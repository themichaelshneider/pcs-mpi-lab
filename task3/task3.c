#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>

#define ARRAY_SIZE 200000  // Размер массивов

// Последовательные операции
void sequential_operations(double *a, double *b, double *sum, double *diff, double *prod, double *div, int n,
                           double *time_sum, double *time_diff, double *time_prod, double *time_div) {
    double start, end;

    start = MPI_Wtime();
    for (int i = 0; i < n; i++) sum[i] = a[i] + b[i];
    end = MPI_Wtime();
    *time_sum += (end - start);

    start = MPI_Wtime();
    for (int i = 0; i < n; i++) diff[i] = a[i] - b[i];
    end = MPI_Wtime();
    *time_diff += (end - start);

    start = MPI_Wtime();
    for (int i = 0; i < n; i++) prod[i] = a[i] * b[i];
    end = MPI_Wtime();
    *time_prod += (end - start);

    start = MPI_Wtime();
    for (int i = 0; i < n; i++) div[i] = (b[i] != 0) ? a[i] / b[i] : 0.0;
    end = MPI_Wtime();
    *time_div += (end - start);
}

// Параллельные операции
void parallel_operations(double *a, double *b, double *sum, double *diff, double *prod, double *div, int n_local,
                         double *time_sum, double *time_diff, double *time_prod, double *time_div) {
    double start, end;

    start = MPI_Wtime();
    for (int i = 0; i < n_local; i++) sum[i] = a[i] + b[i];
    end = MPI_Wtime();
    *time_sum += (end - start);

    start = MPI_Wtime();
    for (int i = 0; i < n_local; i++) diff[i] = a[i] - b[i];
    end = MPI_Wtime();
    *time_diff += (end - start);

    start = MPI_Wtime();
    for (int i = 0; i < n_local; i++) prod[i] = a[i] * b[i];
    end = MPI_Wtime();
    *time_prod += (end - start);

    start = MPI_Wtime();
    for (int i = 0; i < n_local; i++) div[i] = (b[i] != 0) ? a[i] / b[i] : 0.0;
    end = MPI_Wtime();
    *time_div += (end - start);
}

int main(int argc, char *argv[]) {
    int rank, size;
    MPI_Init(&argc, &argv);                   // Инициализация MPI
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);    // Номер процесса
    MPI_Comm_size(MPI_COMM_WORLD, &size);    // Общее число процессов

    int runs = 100;                          // Количество запусков по умолчанию
    if (argc > 1) {
        int val = atoi(argv[1]);
        if (val > 0) runs = val;
        else if (rank == 0)
            printf("Некорректный параметр запусков. Используем %d запусков по умолчанию.\n", runs);
    }

    if (ARRAY_SIZE % size != 0) {
        if (rank == 0)
            printf("Размер массива (%d) должен быть кратен количеству процессов (%d).\n", ARRAY_SIZE, size);
        MPI_Finalize();
        return -1;
    }

    int n_local = ARRAY_SIZE / size;

    double *a_local = malloc(n_local * sizeof(double));
    double *b_local = malloc(n_local * sizeof(double));
    double *sum_local = malloc(n_local * sizeof(double));
    double *diff_local = malloc(n_local * sizeof(double));
    double *prod_local = malloc(n_local * sizeof(double));
    double *div_local = malloc(n_local * sizeof(double));

    double *a = NULL, *b = NULL;
    double *sum = NULL, *diff = NULL, *prod = NULL, *div = NULL;

    if (rank == 0) {
        a = malloc(ARRAY_SIZE * sizeof(double));
        b = malloc(ARRAY_SIZE * sizeof(double));
        sum = malloc(ARRAY_SIZE * sizeof(double));
        diff = malloc(ARRAY_SIZE * sizeof(double));
        prod = malloc(ARRAY_SIZE * sizeof(double));
        div = malloc(ARRAY_SIZE * sizeof(double));
    }

    // Время для операций последовательных
    double total_seq_sum = 0.0, total_seq_diff = 0.0, total_seq_prod = 0.0, total_seq_div = 0.0;
    // Время для операций параллельных
    double total_par_sum = 0.0, total_par_diff = 0.0, total_par_prod = 0.0, total_par_div = 0.0;

    for (int run = 0; run < runs; run++) {
        if (rank == 0) {
            srand(time(NULL) + run);
            for (int i = 0; i < ARRAY_SIZE; i++) {
                a[i] = (double)(rand() % 100000) / 100.0;
                b[i] = (double)(rand() % 100000) / 100.0;
                if (b[i] == 0) b[i] = 1.0;
            }
        }

        MPI_Scatter(a, n_local, MPI_DOUBLE, a_local, n_local, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        MPI_Scatter(b, n_local, MPI_DOUBLE, b_local, n_local, MPI_DOUBLE, 0, MPI_COMM_WORLD);

        // Последовательные операции
        if (rank == 0) {
            sequential_operations(a, b, sum, diff, prod, div, ARRAY_SIZE,
                                  &total_seq_sum, &total_seq_diff, &total_seq_prod, &total_seq_div);
        }

        MPI_Barrier(MPI_COMM_WORLD);

        // Параллельные операции
        parallel_operations(a_local, b_local, sum_local, diff_local, prod_local, div_local, n_local,
                            &total_par_sum, &total_par_diff, &total_par_prod, &total_par_div);

        MPI_Barrier(MPI_COMM_WORLD);

        MPI_Gather(sum_local, n_local, MPI_DOUBLE, sum, n_local, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        MPI_Gather(diff_local, n_local, MPI_DOUBLE, diff, n_local, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        MPI_Gather(prod_local, n_local, MPI_DOUBLE, prod, n_local, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        MPI_Gather(div_local, n_local, MPI_DOUBLE, div, n_local, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    }

    // Усредняем время параллельных операций по всем процессам, берём максимум
    double max_par_sum, max_par_diff, max_par_prod, max_par_div;
    MPI_Reduce(&total_par_sum, &max_par_sum, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Reduce(&total_par_diff, &max_par_diff, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Reduce(&total_par_prod, &max_par_prod, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Reduce(&total_par_div, &max_par_div, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("Запусков: %d\n", runs);
        printf("Среднее время последовательных операций (сек):\n");
        printf("Сложение:     %f\n", total_seq_sum / runs);
        printf("Вычитание:    %f\n", total_seq_diff / runs);
        printf("Умножение:    %f\n", total_seq_prod / runs);
        printf("Деление:      %f\n", total_seq_div / runs);

        printf("Среднее время параллельных операций с %d процессами (сек):\n", size);
        printf("Сложение:     %f\n", max_par_sum / runs);
        printf("Вычитание:    %f\n", max_par_diff / runs);
        printf("Умножение:    %f\n", max_par_prod / runs);
        printf("Деление:      %f\n", max_par_div / runs);
    }

    free(a_local);
    free(b_local);
    free(sum_local);
    free(diff_local);
    free(prod_local);
    free(div_local);

    if (rank == 0) {
        free(a);
        free(b);
        free(sum);
        free(diff);
        free(prod);
        free(div);
    }

    MPI_Finalize();
    return 0;
}
