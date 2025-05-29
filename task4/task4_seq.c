#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ROWS 1000
#define COLS 1000

// Заполнение матрицы случайными числами от 1 до 100
void fill_random(int matrix[ROWS][COLS]) {
    for (int i = 0; i < ROWS; ++i)
        for (int j = 0; j < COLS; ++j)
            matrix[i][j] = rand() % 100 + 1;
}

// Операции над элементами матриц
void add_matrices(int A[ROWS][COLS], int B[ROWS][COLS], int C[ROWS][COLS]) {
    for (int i = 0; i < ROWS; ++i)
        for (int j = 0; j < COLS; ++j)
            C[i][j] = A[i][j] + B[i][j];
}

void sub_matrices(int A[ROWS][COLS], int B[ROWS][COLS], int C[ROWS][COLS]) {
    for (int i = 0; i < ROWS; ++i)
        for (int j = 0; j < COLS; ++j)
            C[i][j] = A[i][j] - B[i][j];
}

void mul_matrices(int A[ROWS][COLS], int B[ROWS][COLS], int C[ROWS][COLS]) {
    for (int i = 0; i < ROWS; ++i)
        for (int j = 0; j < COLS; ++j)
            C[i][j] = A[i][j] * B[i][j];
}

void div_matrices(int A[ROWS][COLS], int B[ROWS][COLS], double C[ROWS][COLS]) {
    for (int i = 0; i < ROWS; ++i)
        for (int j = 0; j < COLS; ++j)
            C[i][j] = (double)A[i][j] / B[i][j]; // B[i][j] от 1 до 100, деление безопасно
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Использование: %s <количество запусков>\n", argv[0]);
        return 1;
    }

    int runs = atoi(argv[1]);
    if (runs <= 0) {
        printf("Количество запусков должно быть больше 0\n");
        return 1;
    }

    int A[ROWS][COLS], B[ROWS][COLS];
    int C_add[ROWS][COLS], C_sub[ROWS][COLS], C_mul[ROWS][COLS];
    double C_div[ROWS][COLS];

    double total_time_add = 0.0;
    double total_time_sub = 0.0;
    double total_time_mul = 0.0;
    double total_time_div = 0.0;

    for (int r = 0; r < runs; r++) {
        srand(time(NULL) + r);
        fill_random(A);
        fill_random(B);

        clock_t start, end;

        start = clock();
        add_matrices(A, B, C_add);
        end = clock();
        total_time_add += (double)(end - start) / CLOCKS_PER_SEC;

        start = clock();
        sub_matrices(A, B, C_sub);
        end = clock();
        total_time_sub += (double)(end - start) / CLOCKS_PER_SEC;

        start = clock();
        mul_matrices(A, B, C_mul);
        end = clock();
        total_time_mul += (double)(end - start) / CLOCKS_PER_SEC;

        start = clock();
        div_matrices(A, B, C_div);
        end = clock();
        total_time_div += (double)(end - start) / CLOCKS_PER_SEC;
    }

    printf("Среднее время выполнения операций за %d запусков:\n", runs);
    printf("Сложение:    %f секунд\n", total_time_add / runs);
    printf("Вычитание:   %f секунд\n", total_time_sub / runs);
    printf("Умножение:   %f секунд\n", total_time_mul / runs);
    printf("Деление:     %f секунд\n", total_time_div / runs);

    return 0;
}
