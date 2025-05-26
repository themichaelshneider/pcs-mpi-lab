#include <stdio.h>      
#include <stdlib.h>     
#include <mpi.h>       
#include <time.h> 

#define ARRAY_SIZE 1000000 

// Заполнение массива случайными числами от 0 до 99
void fill_array(int *array, int size) {
    for (int i = 0; i < size; i++) {
        array[i] = rand() % 100;
    }
}

// Последовательный вариант вычисления суммы
double sequential_sum(int *array, int size, int launches) {
    double total_time = 0.0; 
    long long result = 0; // Сумма элементов 

    for (int k = 0; k < launches; k++) {
        result = 0; // Обнуляем результат перед каждым запуском
        double start = MPI_Wtime(); // Засекаем начало времени
        for (int i = 0; i < size; i++) {
            result += array[i];
        }
        double end = MPI_Wtime(); // Засекаем конец времени
        total_time += end - start; // Добавляем к общему времени
    }

    // Выводим результат суммы только один раз
    printf("Последовательно: %lld\n", result);
    return total_time / launches; // Среднее время выполнения 
}

//Параллельный вариант с MPI
double parallel_sum(int *array, int size, int launches, int rank, int size_proc) {
    double total_time = 0.0; 
    int local_size = size / size_proc; // Размер подмассива для одного процесса
    int *sub_array = (int *)malloc(local_size * sizeof(int)); // Выделение памяти под подмассив
    long long global_sum = 0, local_sum = 0;

    for (int k = 0; k < launches; k++) {
        local_sum = 0;
        double start = MPI_Wtime(); 

        // Распределение массива по всем процессам
        MPI_Scatter(array, local_size, MPI_INT,
                    sub_array, local_size, MPI_INT,
                    0, MPI_COMM_WORLD);

        // Каждый процесс считает свою локальную сумму
        for (int i = 0; i < local_size; i++) {
            local_sum += sub_array[i];
        }

        // Суммируем все локальные суммы в один итог 
        MPI_Reduce(&local_sum, &global_sum, 1, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

        double end = MPI_Wtime();   
        total_time += end - start;
    }

    // Итоговая сумма
    if (rank == 0) {
        printf("Parallel sum result: %lld\n", global_sum);
    }

    free(sub_array);  
    return total_time / launches; 
}

int main(int argc, char *argv[]) {
    int rank, size_proc;     // Номер процесса и общее число процессов
    int *array = NULL;       // Указатель на массив

    //Число запусков считываем из параметров командной строки
    int launches = (argc > 1) ? atoi(argv[1]) : 100;

    //Инициализация MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);     // Узнаём номер текущего процесса
    MPI_Comm_size(MPI_COMM_WORLD, &size_proc); // Узнаём общее количество процессов

    //Только процесс 0 создаёт и заполняет массив
    if (rank == 0) {
        array = (int *)malloc(ARRAY_SIZE * sizeof(int));
        srand(time(NULL));            
        fill_array(array, ARRAY_SIZE); // Заполняем массив
    }

    // Параллельное суммирование
    double avg_time = parallel_sum(array, ARRAY_SIZE, launches, rank, size_proc);

    // Если это главный процесс, дополнительно считаем последовательную сумму
    if (rank == 0) {
        printf("Average parallel time over %d runs: %f seconds\n", launches, avg_time);

        double seq_time = sequential_sum(array, ARRAY_SIZE, launches);
        printf("Average sequential time over %d runs: %f seconds\n", launches, seq_time);

        free(array);  
    }

    //Завершаем работу MPI
    MPI_Finalize();
    return 0;
}
