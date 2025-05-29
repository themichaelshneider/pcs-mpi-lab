#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>

#define ARRAY_SIZE 200000  

// Подсчёт суммы элементов локального массива
int local_sum(int* array, int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += array[i];
    }
    return sum;
}

// Заполнение массива случайными числами от 0 до 99
void fill_random(int* array, int size) {
    for (int i = 0; i < size; i++) {
        array[i] = rand() % 100;
    }
}

// Функция параллельного суммирования с N запусков
double parallel_sum(int runs, int rank, int size, int* total_sum_out) {
    // Проверка делимости размера массива
    if (ARRAY_SIZE % size != 0) {
        if (rank == 0) {
            printf("Ошибка: размер массива (%d) не делится на количество процессов (%d) без остатка.\n",
                   ARRAY_SIZE, size);
        }
        return -1.0;
    }

    int local_size = ARRAY_SIZE / size;
    int* local_array = (int*)malloc(sizeof(int) * local_size);
    int* full_array = NULL;

    if (rank == 0) {
        full_array = (int*)malloc(sizeof(int) * ARRAY_SIZE);
        if (!full_array) {
            printf("Ошибка: не удалось выделить память под массив.\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }

    double total_time = 0.0;
    int final_sum = 0;

    for (int run = 0; run < runs; run++) {
        // Только главный процесс заполняет массив случайными числами
        if (rank == 0) {
            srand(time(NULL) + run);  // Новый seed
            fill_random(full_array, ARRAY_SIZE);
        }

        // синхронизация всех процессов перед началом замера времени
        MPI_Barrier(MPI_COMM_WORLD);
        double start_time = MPI_Wtime();

        // Распределение данных
        MPI_Scatter(full_array, local_size, MPI_INT,
                    local_array, local_size, MPI_INT,
                    0, MPI_COMM_WORLD);

        // Локальная сумма
        int local_part_sum = local_sum(local_array, local_size);

        // Сбор всех локальных сумм
        int global_sum = 0;
        MPI_Reduce(&local_part_sum, &global_sum, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

        // синхронизация всех процессов перед окончанием замера времени
        MPI_Barrier(MPI_COMM_WORLD);
        double end_time = MPI_Wtime();

        if (rank == 0) {
            total_time += (end_time - start_time);
            final_sum = global_sum;  // Последняя сумма
        }
    }

    free(local_array);
    if (rank == 0) free(full_array);

    if (rank == 0) *total_sum_out = final_sum;
    return total_time / runs;
}

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    // Получаем номер текущего процесса (rank) и общее количество процессов (size)
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc < 2) {
        if (rank == 0)
            printf("Использование: mpirun -np <процессы> %s <число запусков>\n", argv[0]);
        MPI_Finalize();
        return 1;
    }

    int runs = atoi(argv[1]);
    if (runs <= 0) {
        if (rank == 0)
            printf("Ошибка: количество запусков должно быть больше 0.\n");
        MPI_Finalize();
        return 1;
    }

    int final_sum = 0;
    double avg_time = parallel_sum(runs, rank, size, &final_sum);

    if (rank == 0) {
        printf("Среднее время за %d запусков: %f секунд(ы)\n", runs, avg_time);
        printf("Сумма элементов массива (последний запуск): %d\n", final_sum);
    }

    MPI_Finalize();
    return 0;
}
