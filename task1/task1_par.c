#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>

#define ARRAY_SIZE 200000  // Размер всего массива

// Подсчёт суммы элементов локального массива
int local_sum(int* array, int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += array[i];
    }
    return sum;
}

// Заполнения массива случайными числами от 0 до 99
void fill_random(int* array, int size) {
    for (int i = 0; i < size; i++) {
        array[i] = rand() % 100;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Использование: mpirun -np <кол-во процессов> %s <количество запусков>\n", argv[0]);
        return 1;
    }

    int runs = atoi(argv[1]);
    if (runs <= 0) {
        printf("Количество запусков должно быть больше 0.\n");
        return 1;
    }

    MPI_Init(&argc, &argv);

    // Получаем номер текущего процесса (rank) и общее количество процессов (size)
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Проверка: ARRAY_SIZE должен делиться на size без остатка
    if (ARRAY_SIZE % size != 0) {
        if (rank == 0) {
            printf("Ошибка: размер массива (%d) не делится на количество процессов (%d) без остатка.\n",
                   ARRAY_SIZE, size);
        }
        MPI_Finalize();
        return 1;
    }

    // Вычисляем размер локального подмассива (должен быть кратен количеству процессов)
    int local_size = ARRAY_SIZE / size;
    int* local_array = (int*)malloc(sizeof(int) * local_size);



    double total_time = 0.0;
    int total_sum = 0;

    int* full_array = NULL;
    for (int run = 0; run < runs; run++) {
        
        // Только главный процесс (rank == 0) выделяет и заполняет весь массив
        if (rank == 0) {
            full_array = (int*)malloc(sizeof(int) * ARRAY_SIZE);
            if (!full_array) {
                printf("Ошибка: не удалось выделить память под массив.\n");
                return 1;
            }
            srand(time(NULL) + run);
            fill_random(full_array, ARRAY_SIZE);
        }
        
        // Синхронизация всех процессов перед началом измерения времени
        MPI_Barrier(MPI_COMM_WORLD);
        double start_time = MPI_Wtime();

        // Распределение частей массива
        MPI_Scatter(
            full_array, local_size, MPI_INT,
            local_array, local_size, MPI_INT,
            0, MPI_COMM_WORLD
        );

        // Каждый процесс вычисляет сумму своих элементов
        int local_part_sum = local_sum(local_array, local_size);

        // Сбор всех локальных сумм в одну итоговую сумму у главного процесса (rank 0)
        int global_sum = 0;
        MPI_Reduce(
            &local_part_sum, &global_sum, 1,
            MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD
        );

        // Синхронизация всех процессов перед окончанием измерения времени
        MPI_Barrier(MPI_COMM_WORLD);
        double end_time = MPI_Wtime();

        // Только главный процесс собирает статистику и перезаполняет массив
        if (rank == 0) {
            total_time += (end_time - start_time);
            total_sum = global_sum;
            free(full_array);  // Освобождаем память после каждого запуска
        }
    }

    // Только главный процесс выводит итоговую информацию
    if (rank == 0) {
        printf("Среднее время за %d запусков: %f секунд(ы)\n", runs, total_time / runs);
        printf("Сумма элементов массива: %d\n", total_sum);
        free(full_array);
    }

    free(local_array);
    MPI_Finalize();
    return 0;
}
