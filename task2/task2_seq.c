#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ARRAY_SIZE 200000

void fill_random(int *array, int size) {
    for (int i = 0; i < size; i++) {
        array[i] = rand() % 1000;
    }
}

// СОортировка пузырьком
void bubble_sort(int *array, int size) {
    for (int i = 0; i < size - 1; i++) {
        for (int j = 0; j < size - i - 1; j++) {
            if (array[j] > array[j + 1]) {
                int tmp = array[j];
                array[j] = array[j + 1];
                array[j + 1] = tmp;
            }
        }
    }
}

int main(int argc, char* argv[]) {
  
    // Если параметр не передали
    if (argc < 2) {
        printf("Используйте: %s <количество запусков>\n", argv[0]);
        return 1;
    }

    
    int runs = atoi(argv[1]);
    if (runs <= 0) {
        printf("Количество запусков должно быть больше 0.\n");
        return 1;
    }

    int *array = malloc(sizeof(int) * ARRAY_SIZE);
    if (!array) {
        printf("Ошибка выделения памяти.\n");
        return 1;
    }

    double total_time = 0.0;

    for (int run = 0; run < runs; run++) {
        srand(time(NULL) + run);
        // Каждый раз перезаполняется
        fill_random(array, ARRAY_SIZE);

        clock_t start = clock();
        bubble_sort(array, ARRAY_SIZE);
        clock_t end = clock();

        total_time += (double)(end - start) / CLOCKS_PER_SEC;
    }

    printf("Среднее время сортировки за %d запусков: %f секунд(ы)\n", runs, total_time / runs);

    free(array);
    return 0;
}
