#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ARRAY_SIZE 200000

// последовательная сумма
int sequential_sum(int* array, int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += array[i];
    }
    return sum;
}

// заполнение массива случайными числами
void fill_random(int* array, int size) {
    for (int i = 0; i < size; i++) {
        array[i] = rand() % 100;
    }
}

int main(int argc, char* argv[]) {
    // если количество запусков не было передано
    if (argc < 2) {
        printf("Используйте: %s <количество запусков>\n", argv[0]);
        return 1;
    }

    int runs = atoi(argv[1]);
    if (runs <= 0) {
        printf("Количество запусков должно быть больше 0.\n");
        return 1;
    }


    int* array = (int*)malloc(sizeof(int) * ARRAY_SIZE);
    if (!array) {
        printf("Ошибка: не удалось выделить память под массив.\n");
        return 1;
    }
    
    double total_time = 0.0;
    int total_sum = 0;

    for (int run = 0; run < runs; run++) {
        
        srand(time(NULL) + run); 
        fill_random(array, ARRAY_SIZE);
        
        clock_t start = clock();

        int sum = sequential_sum(array, ARRAY_SIZE);

        clock_t end = clock();

        double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
        total_time += elapsed
        total_sum = sum;
    }

    printf("Среднее время за %d запусков: %f секунд(ы)\n", runs, total_time / runs);
    printf("Сумма элементов массива: %d\n", total_sum);

    free(array);
    return 0;
}
