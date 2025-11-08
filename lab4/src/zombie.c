#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    pid_t pid = fork();  // создание дочернего процесса

    if (pid > 0) {
        printf("Родительский процесс. PID = %d, PID дочерний = %d\n", getpid(), pid);
        printf("Родитель ожидает 60 секунд, чтобы появился зомби процесс\n");

        //int status;
        //wait(&status); 

        // Родитель ожидает и не вызывает wait()
        for (int i = 0; i < 5; i++) {
            sleep(1);
            system("ps -el | grep Z");  // проверяем наличие зомби 
        }

        printf("Родитель процесс. Время sleep вышло.\n");
    } else if (pid == 0) {
        printf("Дочерний процесс. PID = %d, PID родительский = %d\n", getpid(), getppid());
        exit(0);  // завершается сразу
    } else {
        perror("fork");
        return 1;
    }

    return 0;
}
