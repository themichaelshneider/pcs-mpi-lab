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

//Параллельный вариант с MPI с учётом разных размеров подмассивов
double parallel_sum(int *array, int size, int launches, int rank, int size_proc) {
    double total_time = 0.0; 
    int *sendcounts = NULL; // Массив: сколько элементов отправить каждому процессу
    int *displs = NULL; // Массив смещений: начальный индекс для каждого процесса
    int local_size = 0; // Размер подмассива для одного процесса
    int *sub_array = NULL; // Подмассив на каждый процесс
    long long global_sum = 0, local_sum = 0;

    // Только корневой процесс создаёт sendcounts и displs
    if (rank == 0) {
        sendcounts = (int *)malloc(size_proc * sizeof(int));  
        displs = (int *)malloc(size_proc * sizeof(int));

        if (!sendcounts || !displs) {
            fprintf(stderr, "Ошибка выделения памяти для sendcounts/displs.\n");
            MPI_Abort(MPI_COMM_WORLD, 1);  
        }

        int rem = size % size_proc;  // Остаток, если размер не делится нацело, распределим равномерно по всем процессам
        int offset = 0;
        for (int i = 0; i < size_proc; i++) {
            sendcounts[i] = size / size_proc + (i < rem ? 1 : 0);  // Распределение остатков: первые rem процесов получат по 1 элементу
            displs[i] = offset;        // Запоминаем начальный индекс
            offset += sendcounts[i];   // Увеличиваем смещение
        }
    }

    // Рассылаем каждому процессу его local_size 
    int mpi_status = MPI_Scatter(sendcounts, 1, MPI_INT, &local_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
    if (mpi_status != MPI_SUCCESS) {
        fprintf(stderr, "Ошибка в MPI_Scatter.\n");
        MPI_Abort(MPI_COMM_WORLD, 2);
    }

    sub_array = (int *)malloc(local_size * sizeof(int));  // Память под подмассив
    if (!sub_array) {
        fprintf(stderr, "Ошибка выделения памяти для sub_array в процессе %d\n", rank);
        MPI_Abort(MPI_COMM_WORLD, 3);
    }

    // Основной цикл замеров
    for (int k = 0; k < launches; k++) {
        local_sum = 0;                    // Обнуляем локальную сумму
        double start = MPI_Wtime();      // Засекаем начало времени

        // Рассылаем подмассивы разного размера
        mpi_status = MPI_Scatterv(array, sendcounts, displs, MPI_INT,
                                  sub_array, local_size, MPI_INT,
                                  0, MPI_COMM_WORLD);
        if (mpi_status != MPI_SUCCESS) {
            fprintf(stderr, "Ошибка в MPI_Scatterv.\n");
            MPI_Abort(MPI_COMM_WORLD, 4);
        }

        // Локально считаем сумму
        for (int i = 0; i < local_size; i++) {
            local_sum += sub_array[i];
        }

        // Собираем все локальные суммы в глобальную сумму
        mpi_status = MPI_Reduce(&local_sum, &global_sum, 1, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
        if (mpi_status != MPI_SUCCESS) {
            fprintf(stderr, "Ошибка в MPI_Reduce.\n");
            MPI_Abort(MPI_COMM_WORLD, 5);
        }

        double end = MPI_Wtime();       // Засекаем конец времени
        total_time += end - start;      // Сохраняем время выполнения
    }

    
    // Итоговая сумма
    if (rank == 0) {
        printf("Параллельная сумма: %lld\n", global_sum);
    }

    // Очистка памяти
    free(sub_array);
    if (rank == 0) {
        free(sendcounts);
        free(displs);
    }
    
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
