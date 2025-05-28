#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>

#define ARRAY_SIZE 200000  

// Последовательная пузырьковая сортировка
void bubble_sort(int *arr, int n) {
    int i, j, temp;
    for (i = 0; i < n-1; i++) {
        for (j = 0; j < n-i-1; j++) {
            if (arr[j] > arr[j+1]) {
                temp = arr[j];
                arr[j] = arr[j+1];
                arr[j+1] = temp;
            }
        }
    }
}

// Проверка отсортированности массива
int is_sorted(int *arr, int n) {
    for (int i=0; i<n-1; i++) {
        if (arr[i] > arr[i+1]) return 0;
    }
    return 1;
}

// Параллельная пузырьковая сортировка с обменом граничными элементами
void parallel_bubble_sort(int *local_arr, int local_n, int rank, int size, MPI_Comm comm) {
    int temp;
    int left_neighbor = rank - 1;
    int right_neighbor = rank + 1;
    MPI_Status status;

    for (int phase = 0; phase < size; phase++) {
        // Локальная сортировка пузырьком
        for (int i = 0; i < local_n - 1; i++) {
            for (int j = 0; j < local_n - i - 1; j++) {
                if (local_arr[j] > local_arr[j+1]) {
                    temp = local_arr[j];
                    local_arr[j] = local_arr[j+1];
                    local_arr[j+1] = temp;
                }
            }
        }

        // Обмен границами между процессами
        if (rank % 2 == 0) {
            if (right_neighbor < size) {
                MPI_Sendrecv(&local_arr[local_n-1], 1, MPI_INT, right_neighbor, 0,
                             &temp, 1, MPI_INT, right_neighbor, 0,
                             comm, &status);
                if (temp < local_arr[local_n-1]) local_arr[local_n-1] = temp;
            }
        } else {
            if (left_neighbor >= 0) {
                MPI_Sendrecv(&local_arr[0], 1, MPI_INT, left_neighbor, 0,
                             &temp, 1, MPI_INT, left_neighbor, 0,
                             comm, &status);
                if (temp > local_arr[0]) local_arr[0] = temp;
            }
        }
        MPI_Barrier(comm);
    }
}

int main(int argc, char *argv[]) {
    int rank, size;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int n = ARRAY_SIZE;

    // Кол-во запусков по умолчанию
    int runs = 100;
    if (argc > 1) {
        int val = atoi(argv[1]);
        if (val > 0) runs = val;
        else if (rank == 0) printf("Ошибка, используем по умолчанию %d запусков\n", runs);
    }

    if (n % size != 0) {
        if (rank == 0) printf("Размер массива должен быть кратен количеству процессов\n");
        MPI_Finalize();
        return -1;
    }

    int local_n = n / size;
    int *array = NULL;
    int *local_arr = malloc(local_n * sizeof(int));

    double total_time_seq = 0.0;
    double local_time_par = 0.0;

    for (int run = 0; run < runs; run++) {
        if (rank == 0) {
            array = malloc(n * sizeof(int));
            srand(time(NULL) + run);
            for (int i = 0; i < n; i++) {
                array[i] = rand() % 1000000;
            }
        }

        MPI_Scatter(array, local_n, MPI_INT, local_arr, local_n, MPI_INT, 0, MPI_COMM_WORLD);

        if (rank == 0) {
            int *copy_seq = malloc(n * sizeof(int));
            for (int i = 0; i < n; i++) copy_seq[i] = array[i];

            double start_time = MPI_Wtime();
            bubble_sort(copy_seq, n);
            double end_time = MPI_Wtime();
            total_time_seq += (end_time - start_time);

            if (!is_sorted(copy_seq, n)) {
                printf("Не удалось выполнить последовательную сортировку\n");
            }
            free(copy_seq);
        }

        MPI_Barrier(MPI_COMM_WORLD);
        double start_time = MPI_Wtime();

        parallel_bubble_sort(local_arr, local_n, rank, size, MPI_COMM_WORLD);

        MPI_Barrier(MPI_COMM_WORLD);
        double end_time = MPI_Wtime();

        local_time_par += (end_time - start_time);

        MPI_Gather(local_arr, local_n, MPI_INT, array, local_n, MPI_INT, 0, MPI_COMM_WORLD);

        if (rank == 0) {
            if (!is_sorted(array, n)) {
                printf("Сбой параллельной сортировки при запуске %d\n", run);
            }
            free(array);
        }
    }

    double max_time_par = 0.0;
    MPI_Reduce(&local_time_par, &max_time_par, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("Программа запущена %d раз\n", runs);
        printf("Среднее время последовательной сортировки: %f сек\n", total_time_seq / runs);
        printf("Среднее время параллельной сортировки с %d процессами: %f сек\n", size, max_time_par / runs);
    }

    free(local_arr);
    MPI_Finalize();
    return 0;
}
