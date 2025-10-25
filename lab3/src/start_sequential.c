#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    pid_t pid = fork(); // дочерний процесс

    if (pid < 0) {
        perror("fork failed");
        return 1;
    }

    if (pid == 0) {
        printf("Child process: running sequential_min_max...\n");

        // запуск программы sequential_min_max с аргументами
        char *args[] = {"./sequential_min_max", "42", "100", NULL};
        execv(args[0], args); // замена процесса на другую программу

        // если execv не сработал
        perror("execv failed");
        exit(1);
    } else {
        // родительский процесс
        int status;
        waitpid(pid, &status, 0); // ожидание завершения дочернего процесса
        printf("Parent process: sequential_min_max finished with status %d\n", status);
    }

    return 0;
}
