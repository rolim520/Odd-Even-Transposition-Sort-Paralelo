#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

void odd_even_sort_serial(int arr[], int n) {
    int phase, i;
    for (phase = 0; phase < n; phase++) {
        if (phase % 2 == 0) {
            for (i = 1; i < n; i += 2) {
                if (arr[i - 1] > arr[i]) {
                    swap(&arr[i - 1], &arr[i]);
                }
            }
        } else {
            for (i = 1; i < n - 1; i += 2) {
                if (arr[i] > arr[i + 1]) {
                    swap(&arr[i], &arr[i + 1]);
                }
            }
        }
    }
}

void generate_random_array(int arr[], int n, int max_val) {
    srand(time(NULL));
    for (int i = 0; i < n; i++) {
        arr[i] = rand() % max_val;
    }
}

int main(int argc, char *argv[]) {
    FILE *csv_file = fopen("data/serial.csv", "w");
    if (csv_file == NULL) {
        printf("Erro ao abrir o arquivo data/serial.csv\n");
        return 1;
    }

    fprintf(csv_file, "Tamanho,Tempo(s)\n");

    long sizes[] = {1000, 5000, 10000, 50000, 100000};
    int num_sizes = sizeof(sizes) / sizeof(sizes[0]);

    printf("Iniciando benchmark serial...\n");

    for (int i = 0; i < num_sizes; i++) {
        long n = sizes[i];
        int *arr = malloc(n * sizeof(int));
        if (arr == NULL) {
            printf("Falha ao alocar memória para %ld elementos.\n", n);
            continue;
        }

        generate_random_array(arr, n, 1000);

        printf("Testando com N = %ld... ", n);

        struct timespec start, end;
        clock_gettime(CLOCK_MONOTONIC, &start);

        odd_even_sort_serial(arr, n);

        clock_gettime(CLOCK_MONOTONIC, &end);

        double time_taken = (end.tv_sec - start.tv_sec) + (double)(end.tv_nsec - start.tv_nsec) / 1e9;

        printf("concluído em %.6f segundos.\n", time_taken);

        fprintf(csv_file, "%ld,%.6f\n", n, time_taken);

        free(arr);
    }

    fclose(csv_file);
    printf("Benchmark concluído. Resultados salvos em data/serial.csv\n");

    return 0;
}