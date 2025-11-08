#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <string.h>
#include <sys/time.h>
#include "sum_array.h"

struct SumArgs {
    int *array;
    int begin;
    int end;
};

void *ThreadSum(void *args) {
    struct SumArgs *sum_args = (struct SumArgs *)args;
    return (void *)(size_t)SumArrayPart(sum_args->array, sum_args->begin, sum_args->end);
}

int main(int argc, char **argv) {
    if (argc != 7) {
        printf("Usage: %s --threads_num N --array_size N --seed N\n", argv[0]);
        return 1;
    }

    int threads_num = 0, array_size = 0, seed = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--threads_num") == 0) threads_num = atoi(argv[++i]);
        if (strcmp(argv[i], "--array_size") == 0) array_size = atoi(argv[++i]);
        if (strcmp(argv[i], "--seed") == 0) seed = atoi(argv[++i]);
    }

    int *array = malloc(sizeof(int) * array_size);

    // Генерация массива (не включается в замер)
    srand(seed);
    for (int i = 0; i < array_size; i++) array[i] = rand();

    pthread_t threads[threads_num];
    struct SumArgs args[threads_num];

    int part_size = array_size / threads_num;

    struct timeval start, finish;
    gettimeofday(&start, NULL); // старт замера

    // Создание потоков POSIX
    for (int i = 0; i < threads_num; i++) {
        args[i].array = array;
        args[i].begin = i * part_size;
        args[i].end = (i == threads_num - 1) ? array_size : (i + 1) * part_size;
        pthread_create(&threads[i], NULL, ThreadSum, &args[i]);
    }

    // Сбор результатов
    long long total_sum = 0;
    for (int i = 0; i < threads_num; i++) {
        void *ret;
        pthread_join(threads[i], &ret);
        total_sum += (long long)(size_t)ret;
    }

    gettimeofday(&finish, NULL); // конец замера

    double elapsed_time = (finish.tv_sec - start.tv_sec) * 1000.0;
    elapsed_time += (finish.tv_usec - start.tv_usec) / 1000.0;

    printf("Total sum: %lld\n", total_sum);
    printf("Elapsed time (ms): %.3f\n", elapsed_time);

    free(array);
    return 0;
}
