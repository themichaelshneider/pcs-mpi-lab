#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ARRAY_SIZE 200000

// Заполнение массива случайными значениями от 1 до 100
void fill_random(int *array, int size) {
    for (int i = 0; i < size; i++) {
        array[i] = rand() % 100 + 1;
    }
}

// Функция выполнения всех арифметических операций над массивами
void compute_operations(int *a, int *b, int *sum, int *diff, int *prod, double *quot, int size) {
    for (int i = 0; i < size; i++) {
        sum[i]  = a[i] + b[i];
        diff[i] = a[i] - b[i];
        prod[i] = a[i] * b[i];
        quot[i] = b[i] != 0 ? a[i] / b[i] : 0;  // защита от деления на ноль
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
    double *quot = malloc(sizeof(int) * ARRAY_SIZE);

    if (!a || !b || !sum || !diff || !prod || !quot) {
        printf("Ошибка выделения памяти.\n");
        return 1;
    }

    double total_time = 0.0;

    for (int run = 0; run < runs; run++) {
        srand(time(NULL) + run);
        fill_random(a, ARRAY_SIZE);
        fill_random(b, ARRAY_SIZE);

        clock_t start = clock();
        compute_operations(a, b, sum, diff, prod, quot, ARRAY_SIZE);
        clock_t end = clock();

        total_time += (double)(end - start) / CLOCKS_PER_SEC;
    }

    printf("Среднее время выполнения за %d запусков: %f секунд(ы)\n", runs, total_time / runs);

    free(a); free(b); free(sum); free(diff); free(prod); free(quot);
    return 0;
}
