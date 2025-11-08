CC = gcc
CFLAGS = -Wall -Wextra -O2 -g


PARALLEL_SRCS = ../../lab3/src/parallel_min_max.c ../../lab3/src/find_min_max.c ../../lab3/src/utils.c
PROCESS_SRCS  = process_memory.c
PSUM_SRCS     = parallel_sum.c


PARALLEL_OUT = parallel_min_max
PROCESS_OUT  = process_memory
PSUM_OUT     = psum

# Библиотека
SUM_LIB = libsum_array.so
LDFLAGS_PSUM = -L. -lsum_array -lpthread -Wl,-rpath=.

# Аргументы для запуска psum
THREADS = 4
ARRAY_SIZE = 1000000
SEED = 123

.PHONY: all clean run_psum

# Всё собрать
all: $(PARALLEL_OUT) $(PROCESS_OUT) $(PSUM_OUT)

# Сборка parallel_min_max
$(PARALLEL_OUT): $(PARALLEL_SRCS)
	$(CC) $(CFLAGS) $(PARALLEL_SRCS) -o $@

# Сборка process_memory
$(PROCESS_OUT): $(PROCESS_SRCS)
	$(CC) $(CFLAGS) $(PROCESS_SRCS) -o $@

# Сборка psum с динамической библиотекой и pthread
$(PSUM_OUT): $(PSUM_SRCS) $(SUM_LIB)
	$(CC) $(CFLAGS) $(PSUM_SRCS) -o $@ $(LDFLAGS_PSUM)

# Запуск psum с дефолтными аргументами
run_psum: $(PSUM_OUT)
	./$(PSUM_OUT) --threads_num $(THREADS) --array_size $(ARRAY_SIZE) --seed $(SEED)

# Очистка
clean:
	rm -f $(PARALLEL_OUT) $(PROCESS_OUT) $(PSUM_OUT)