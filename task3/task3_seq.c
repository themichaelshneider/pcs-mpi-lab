#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ARRAY_SIZE 1000000

// Заполнение массива случайными значениями от 1 до 100
void fill_random(int *array, int size) {
    for (int i = 0; i < size; i++) {
        array[i] = rand() % 100 + 1;
    }
}

// Операция сложения массивов
void add_arrays(int *a, int *b, int *sum, int size) {
    for (int i = 0; i < size; i++) {
        sum[i] = a[i] + b[i];
    }
}

// Операция вычитания массивов
void subtract_arrays(int *a, int *b, int *diff, int size) {
    for (int i = 0; i < size; i++) {
        diff[i] = a[i] - b[i];
    }
}

// Операция умножения массивов
void multiply_arrays(int *a, int *b, int *prod, int size) {
    for (int i = 0; i < size; i++) {
        prod[i] = a[i] * b[i];
    }
}

// Операция деления массивов
void divide_arrays(int *a, int *b, double *quot, int size) {
    for (int i = 0; i < size; i++) {
        quot[i] = (b[i] != 0) ? ((double)a[i] / b[i]) : 0.0;
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Используйте: %s <количество запусков>\n", argv[0]);
        return 1;
    }

    int runs = atoi(argv[1]);
    if (runs <= 0) {
        printf("Ошибка: количество запусков должно быть положительным числом.\n");
        return 1;
    }

    int *a = malloc(sizeof(int) * ARRAY_SIZE);
    int *b = malloc(sizeof(int) * ARRAY_SIZE);
    int *sum = malloc(sizeof(int) * ARRAY_SIZE);
    int *diff = malloc(sizeof(int) * ARRAY_SIZE);
    int *prod = malloc(sizeof(int) * ARRAY_SIZE);
    double *quot = malloc(sizeof(double) * ARRAY_SIZE);

    if (!a || !b || !sum || !diff || !prod || !quot) {
        printf("Ошибка выделения памяти.\n");
        return 1;
    }

     // Время для каждой операции
    double time_add = 0.0;
    double time_sub = 0.0;
    double time_mul = 0.0;
    double time_div = 0.0;

    for (int run = 0; run < runs; run++) {
        srand(time(NULL) + run);
        fill_random(a, ARRAY_SIZE);
        fill_random(b, ARRAY_SIZE);

        clock_t start, end;

        // Сложение
        start = clock();
        add_arrays(a, b, sum, ARRAY_SIZE);
        end = clock();
        time_add += (double)(end - start) / CLOCKS_PER_SEC;

        // Вычитание
        start = clock();
        subtract_arrays(a, b, diff, ARRAY_SIZE);
        end = clock();
        time_sub += (double)(end - start) / CLOCKS_PER_SEC;

        // Умножение
        start = clock();
        multiply_arrays(a, b, prod, ARRAY_SIZE);
        end = clock();
        time_mul += (double)(end - start) / CLOCKS_PER_SEC;

        // Деление
        start = clock();
        divide_arrays(a, b, quot, ARRAY_SIZE);
        end = clock();
        time_div += (double)(end - start) / CLOCKS_PER_SEC;
    }


    printf("Среднее время выполнения операций за %d запусков:\n", runs);
    printf("Сложение:    %f секунд(ы)\n", time_add / runs);
    printf("Вычитание:   %f секунд(ы)\n", time_sub / runs);
    printf("Умножение:   %f секунд(ы)\n", time_mul / runs);
    printf("Деление:     %f секунд(ы)\n", time_div / runs);

    free(a); free(b); free(sum); free(diff); free(prod); free(quot);
    return 0;
}
