#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

#define ARRAY_SIZE 200000  // должен быть кратен числу процессов

// заполнение массива числами от 0 до 999
void fill_random(int* array, int size) {
    for (int i = 0; i < size; i++) {
        array[i] = rand() % 1000;
    }
}

// Последовательная пузырьковая сортировка для одного подмассива
void bubble_sort(int* arr, int n) {
    for (int i = 0; i < n - 1; ++i) {
        for (int j = 0; j < n - i - 1; ++j) {
            if (arr[j] > arr[j + 1]) {
                int tmp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = tmp;
            }
        }
    }
}

// Параллельная пузырьковая сортировка с четно-нечетными фазами обмена
void parallel_bubble_sort(int* local_arr, int local_size, int rank, int size) {
    int* buffer = (int*)malloc(sizeof(int) * local_size);  // буфер для получения данных от соседа

    for (int phase = 0; phase < size; ++phase) {
        // Локальная сортировка своей части массива
        bubble_sort(local_arr, local_size);

        // Определяем партнёра для обмена
        int partner = (phase % 2 == 0)
            ? (rank % 2 == 0 ? rank + 1 : rank - 1)
            : (rank % 2 == 0 ? rank - 1 : rank + 1);

        // Пропускаем фазу, если партнёр выходит за границы
        if (partner < 0 || partner >= size)
            continue;

        // Обмен массивами с партнёром
        MPI_Sendrecv(local_arr, local_size, MPI_INT, partner, 0,
                     buffer, local_size, MPI_INT, partner, 0,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // Объединяем два подмассива
        int* merged = (int*)malloc(sizeof(int) * 2 * local_size);
        for (int i = 0; i < local_size; ++i) {
            merged[i] = local_arr[i];
            merged[i + local_size] = buffer[i];
        }

        // Сортируем объединённый массив
        bubble_sort(merged, 2 * local_size);

        // Разделяем обратно: младший ранг получает меньшую половину
        if (rank < partner) {
            for (int i = 0; i < local_size; ++i)
                local_arr[i] = merged[i];
        } else {
            for (int i = 0; i < local_size; ++i)
                local_arr[i] = merged[i + local_size];
        }

        free(merged);  // освобождаем временный массив
    }

    free(buffer);  // освобождаем буфер
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Использование: mpirun -np <процессы> %s <кол-во запусков>\n", argv[0]);
        return 1;
    }

    int runs = atoi(argv[1]);  
    if (runs <= 0) {
        printf("Количество запусков должно быть больше 0.\n");
        return 1;
    }

    MPI_Init(&argc, &argv);  

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);  // Получаем ранг процесса
    MPI_Comm_size(MPI_COMM_WORLD, &size);  // Получаем общее число процессов

    if (ARRAY_SIZE % size != 0) {
        if (rank == 0)
            printf("Ошибка: размер массива (%d) не делится на число процессов (%d).\n", ARRAY_SIZE, size);
        MPI_Finalize();
        return 1;
    }

    int local_size = ARRAY_SIZE / size;  // Размер подмассива, обрабатываемого каждым процессом
    int* local_array = (int*)malloc(sizeof(int) * local_size);  
    int* full_array = NULL;

    if (rank == 0) {
        full_array = (int*)malloc(sizeof(int) * ARRAY_SIZE);  // Главный процесс хранит полный массив
    }

    double total_time = 0.0;  

    for (int run = 0; run < runs; ++run) {
        // Главный процесс инициализирует массив случайными числами
        if (rank == 0) {
            srand(time(NULL) + run);  
            fill_random(full_array, ARRAY_SIZE);
        }

        // Распределение массива между процессами
        MPI_Scatter(full_array, local_size, MPI_INT,
                    local_array, local_size, MPI_INT,
                    0, MPI_COMM_WORLD);

        MPI_Barrier(MPI_COMM_WORLD);  // Синхронизация перед замером времени
        double start = MPI_Wtime();   

        // Параллельная сортировка
        parallel_bubble_sort(local_array, local_size, rank, size);

        MPI_Barrier(MPI_COMM_WORLD);  // Синхронизация после сортировки
        double end = MPI_Wtime();     

        if (rank == 0) {
            total_time += (end - start);  
        }
    }

    if (rank == 0) {
        printf("Среднее время за %d запусков: %f секунд(ы)\n", runs, total_time / runs);
        free(full_array);  // Очистка памяти
    }

    free(local_array);  
    MPI_Finalize();     // Завершение работы MPI
    return 0;
}
