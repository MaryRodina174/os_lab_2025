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

#include "find_min_max.h"
#include "utils.h"

int main(int argc, char **argv) {
    int seed = -1;
    int array_size = -1;
    int pnum = -1;
    bool with_files = false;

    while (true) {
        static struct option options[] = {
            {"seed", required_argument, 0, 0},
            {"array_size", required_argument, 0, 0},
            {"pnum", required_argument, 0, 0},
            {"by_files", no_argument, 0, 'f'},
            {0, 0, 0, 0}
        };

        int option_index = 0;
        int c = getopt_long(argc, argv, "f", options, &option_index);

        if (c == -1)
            break;

        switch (c) {
            case 0:
                switch (option_index) {
                    case 0:
                        seed = atoi(optarg);
                        break;
                    case 1:
                        array_size = atoi(optarg);
                        break;
                    case 2:
                        pnum = atoi(optarg);
                        break;
                    case 3:
                        with_files = true;
                        break;
                }
                break;
            case 'f':
                with_files = true;
                break;
            default:
                printf("getopt returned character code 0%o?\n", c);
        }
    }

    if (seed <= 0 || array_size <= 0 || pnum <= 0) {
        printf("Usage: %s --seed num --array_size num --pnum num [--by_files]\n", argv[0]);
        return 1;
    }

    int *array = malloc(sizeof(int) * array_size);
    GenerateArray(array, array_size, seed);

    struct timeval start_time;
    gettimeofday(&start_time, NULL);

    int part_size = array_size / pnum;
    int pipefd[2];
    if (!with_files)
        pipe(pipefd);

    for (int i = 0; i < pnum; i++) {
        pid_t pid = fork();
        if (pid == 0) {
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
        }
    }

    struct MinMax min_max;
    min_max.min = INT_MAX;
    min_max.max = INT_MIN;

    for (int i = 0; i < pnum; i++) {
        wait(NULL);
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

    struct timeval finish_time;
    gettimeofday(&finish_time, NULL);
    double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
    elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

    free(array);
    if (!with_files) {
        close(pipefd[0]);
        close(pipefd[1]);
    }

    printf("Min: %d\n", min_max.min);
    printf("Max: %d\n", min_max.max);
    printf("Elapsed time: %fms\n", elapsed_time);
    fflush(NULL);
    return 0;
}
