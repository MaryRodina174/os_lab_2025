#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <getopt.h>
#include <signal.h>             
#include "find_min_max.h"
#include "utils.h"


pid_t *child_pids = NULL; // массив PID
int pnum_global = 0;      // кол во дочерних процессов


// Обработчик. Срабатывает, если таймаут.
// Завершает все дочерние процессы.
void kill_children(int signum) {
    printf("Таймаут! Завершение всех дочерних процессов...\n");
    for (int i = 0; i < pnum_global; i++) {
        if (child_pids[i] > 0)
            kill(child_pids[i], SIGKILL); // посылаем каждому дочернему SIGKILL
    }
}

int main(int argc, char **argv) {
    int seed = -1;
    int array_size = -1;
    int pnum = -1;
    bool with_files = false;
    int timeout = 0; // определение таймаут

    
    while (true) {
        static struct option options[] = {
            {"seed", required_argument, 0, 0},
            {"array_size", required_argument, 0, 0},
            {"pnum", required_argument, 0, 0},
            {"by_files", no_argument, 0, 'f'},
            {"timeout", required_argument, 0, 0}, 
            {0, 0, 0, 0}
        };
        int option_index = 0;
        int c = getopt_long(argc, argv, "f", options, &option_index);
        if (c == -1) break;

        switch (c) {
            case 0:
                switch (option_index) {
                    case 0: seed = atoi(optarg); break;
                    case 1: array_size = atoi(optarg); break;
                    case 2: pnum = atoi(optarg); break;
                    case 3: with_files = true; break;
                    case 4: timeout = atoi(optarg); break; 
                }
                break;
            case 'f':
                with_files = true;
                break;
            default:
                printf("getopt returned character code 0%o?\n", c);
        }
    }

    // проверка обязательных параметров
    if (seed <= 0 || array_size <= 0 || pnum <= 0) {
        printf("Usage: %s --seed num --array_size num --pnum num [--by_files] [--timeout sec]\n", argv[0]);
        return 1;
    }

    int *array = malloc(sizeof(int) * array_size);
    GenerateArray(array, array_size, seed);

    struct timeval start_time;
    gettimeofday(&start_time, NULL);

    int part_size = array_size / pnum;
    int pipefd[2];
    if (!with_files) pipe(pipefd);

    // создание массив PID 
    child_pids = malloc(sizeof(pid_t) * pnum);
    pnum_global = pnum; // число процессов для обработчика

    // устанавка обработчика и таймера, если задан таймаут 
    if (timeout > 0) {
        signal(SIGALRM, kill_children); // при срабатывании сигнала вызывается kill_children()
        alarm(timeout);                 // через timeout секунд сработает SIGALRM
    }

    // запуск дочерних процессов 
    for (int i = 0; i < pnum; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            // Дочерний процесс
            unsigned int start = i * part_size;
            unsigned int end = (i == pnum - 1) ? array_size : (i + 1) * part_size;
            struct MinMax local_minmax = GetMinMax(array, start, end);

            if (with_files) {
                char filename[256];
                sprintf(filename, "child_%d.txt", i);
                FILE *f = fopen(filename, "w");
                fprintf(f, "%d %d\n", local_minmax.min, local_minmax.max);
                fclose(f);
            } else {
                write(pipefd[1], &local_minmax, sizeof(struct MinMax));
            }

            free(array);
            return 0;
        } else if (pid > 0) {
            // Родитель: сохранение PID дочернего процесса
            child_pids[i] = pid;
        } else {
            perror("fork");
            return 1;
        }
    }

    struct MinMax min_max;
    min_max.min = INT_MAX;
    min_max.max = INT_MIN;

    int status;
    int finished = 0;

    // Неблокирующий wait c WNOHANG
    // Если процесс завершился — waitpid вернёт его PID
    // Если нет — возвращает 0, ждёт и снова проверяем
    while (finished < pnum) {
        pid_t w = waitpid(-1, &status, WNOHANG);
        if (w > 0) finished++;
        else usleep(100000); 
    }

    // Чтение результатов от дочерних процессов
    for (int i = 0; i < pnum; i++) {
        struct MinMax local_minmax;
        if (with_files) {
            char filename[256];
            sprintf(filename, "child_%d.txt", i);
            FILE *f = fopen(filename, "r");
            fscanf(f, "%d %d", &local_minmax.min, &local_minmax.max);
            fclose(f);
        } else {
            read(pipefd[0], &local_minmax, sizeof(struct MinMax));
        }

        if (local_minmax.min < min_max.min) min_max.min = local_minmax.min;
        if (local_minmax.max > min_max.max) min_max.max = local_minmax.max;
    }

    // трекинг время выполнения
    struct timeval finish_time;
    gettimeofday(&finish_time, NULL);
    double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
    elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

    free(array);
    free(child_pids);

    if (!with_files) {
        close(pipefd[0]);
        close(pipefd[1]);
    }

    printf("Min: %d\n", min_max.min);
    printf("Max: %d\n", min_max.max);
    printf("Elapsed time: %fms\n", elapsed_time);

    sleep(5);
    fflush(NULL);
    return 0;

}