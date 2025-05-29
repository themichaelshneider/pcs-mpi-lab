#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

#define ROWS 1000
#define COLS 1000
#define SIZE (ROWS * COLS)

// Заполнение массива случайными числами от 1 до 100
void fill_random(int* matrix, int size) {
    for (int i = 0; i < size; ++i)
        matrix[i] = rand() % 100 + 1;
}

// Операции над частями массивов (локальными блоками)
void add_op(int* A, int* B, int* C, int n) {
    for (int i = 0; i < n; ++i)
        C[i] = A[i] + B[i];
}

void sub_op(int* A, int* B, int* C, int n) {
    for (int i = 0; i < n; ++i)
        C[i] = A[i] - B[i];
}

void mul_op(int* A, int* B, int* C, int n) {
    for (int i = 0; i < n; ++i)
        C[i] = A[i] * B[i];
}

void div_op(int* A, int* B, double* C, int n) {
    for (int i = 0; i < n; ++i)
        C[i] = (double)A[i] / B[i]; // безопасно, т.к. B[i] >= 1
}

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int rank, nproc;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nproc);

    if (argc < 2) {
        if (rank == 0)
            printf("Использование: mpirun -np <процессы> %s <запуски>\n", argv[0]);
        MPI_Finalize();
        return 1;
    }

    int runs = atoi(argv[1]);
    if (runs <= 0) {
        if (rank == 0)
            printf("Количество запусков должно быть больше 0\n");
        MPI_Finalize();
        return 1;
    }

    if (SIZE % nproc != 0) {
        if (rank == 0)
            printf("Ошибка: размер массива %d не делится на %d процессов\n", SIZE, nproc);
        MPI_Finalize();
        return 1;
    }

    int local_size = SIZE / nproc;

    // Выделение локальной памяти
    int* local_A = (int*)malloc(local_size * sizeof(int));
    int* local_B = (int*)malloc(local_size * sizeof(int));
    int* local_add = (int*)malloc(local_size * sizeof(int));
    int* local_sub = (int*)malloc(local_size * sizeof(int));
    int* local_mul = (int*)malloc(local_size * sizeof(int));
    double* local_div = (double*)malloc(local_size * sizeof(double));

    // Только у корневого процесса будут полные массивы
    int* A = NULL;
    int* B = NULL;
    if (rank == 0) {
        A = (int*)malloc(SIZE * sizeof(int));
        B = (int*)malloc(SIZE * sizeof(int));
    }

    double total_add_time = 0.0;
    double total_sub_time = 0.0;
    double total_mul_time = 0.0;
    double total_div_time = 0.0;

    for (int r = 0; r < runs; r++) {
        if (rank == 0) {
            srand(time(NULL) + r);
            fill_random(A, SIZE);
            fill_random(B, SIZE);
        }

        // Рассылка данных частям процессов
        MPI_Scatter(A, local_size, MPI_INT, local_A, local_size, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Scatter(B, local_size, MPI_INT, local_B, local_size, MPI_INT, 0, MPI_COMM_WORLD);

        // СЛОЖЕНИЕ 
        MPI_Barrier(MPI_COMM_WORLD);
        double start = MPI_Wtime();
        add_op(local_A, local_B, local_add, local_size);
        double end = MPI_Wtime();
        total_add_time += (end - start);

        // ВЫЧИТАНИЕ
        MPI_Barrier(MPI_COMM_WORLD);
        start = MPI_Wtime();
        sub_op(local_A, local_B, local_sub, local_size);
        end = MPI_Wtime();
        total_sub_time += (end - start);

        // УМНОЖЕНИЕ
        MPI_Barrier(MPI_COMM_WORLD);
        start = MPI_Wtime();
        mul_op(local_A, local_B, local_mul, local_size);
        end = MPI_Wtime();
        total_mul_time += (end - start);

        // ДЕЛЕНИЕ 
        MPI_Barrier(MPI_COMM_WORLD);
        start = MPI_Wtime();
        div_op(local_A, local_B, local_div, local_size);
        end = MPI_Wtime();
        total_div_time += (end - start);
    }

    // Сбор суммарного времени со всех процессов на rank 0
    double global_add_time = 0.0, global_sub_time = 0.0;
    double global_mul_time = 0.0, global_div_time = 0.0;

    MPI_Reduce(&total_add_time, &global_add_time, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&total_sub_time, &global_sub_time, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&total_mul_time, &global_mul_time, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&total_div_time, &global_div_time, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("Среднее время выполнения операций за %d запусков (в секундах):\n", runs);
        printf("Сложение:    %f\n", global_add_time / (runs * nproc));
        printf("Вычитание:   %f\n", global_sub_time / (runs * nproc));
        printf("Умножение:   %f\n", global_mul_time / (runs * nproc));
        printf("Деление:     %f\n", global_div_time / (runs * nproc));
    }

    // Очистка памяти
    free(local_A);
    free(local_B);
    free(local_add);
    free(local_sub);
    free(local_mul);
    free(local_div);
    if (rank == 0) {
        free(A);
        free(B);
    }

    MPI_Finalize();
    return 0;
}
