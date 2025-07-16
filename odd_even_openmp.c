#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>
#include <string.h>

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

void odd_even_sort_openmp_static(int arr[], int n, int num_threads) {
    int phase, i;
    #pragma omp parallel num_threads(num_threads) default(none) shared(arr, n) private(phase, i)
    {
        for (phase = 0; phase < n; phase++) {
            if (phase % 2 == 0) { // Fase Par
                #pragma omp for schedule(static)
                for (i = 1; i < n; i += 2) {
                    if (arr[i - 1] > arr[i]) {
                        swap(&arr[i - 1], &arr[i]);
                    }
                }
            } else { // Fase Ímpar
                #pragma omp for schedule(static)
                for (i = 1; i < n - 1; i += 2) {
                    if (arr[i] > arr[i + 1]) {
                        swap(&arr[i], &arr[i + 1]);
                    }
                }
            }
        }
    }
}

void odd_even_sort_openmp_dynamic(int arr[], int n, int num_threads) {
    int phase, i;
    #pragma omp parallel num_threads(num_threads) default(none) shared(arr, n) private(phase, i)
    {
        for (phase = 0; phase < n; phase++) {
            if (phase % 2 == 0) { // Fase Par
                #pragma omp for schedule(dynamic)
                for (i = 1; i < n; i += 2) {
                    if (arr[i - 1] > arr[i]) {
                        swap(&arr[i - 1], &arr[i]);
                    }
                }
            } else { // Fase Ímpar
                #pragma omp for schedule(dynamic)
                for (i = 1; i < n - 1; i += 2) {
                    if (arr[i] > arr[i + 1]) {
                        swap(&arr[i], &arr[i + 1]);
                    }
                }
            }
        }
    }
}

void odd_even_sort_openmp_guided(int arr[], int n, int num_threads) {
    int phase, i;
    #pragma omp parallel num_threads(num_threads) default(none) shared(arr, n) private(phase, i)
    {
        for (phase = 0; phase < n; phase++) {
            if (phase % 2 == 0) { // Fase Par
                #pragma omp for schedule(guided)
                for (i = 1; i < n; i += 2) {
                    if (arr[i - 1] > arr[i]) {
                        swap(&arr[i - 1], &arr[i]);
                    }
                }
            } else { // Fase Ímpar
                #pragma omp for schedule(guided)
                for (i = 1; i < n - 1; i += 2) {
                    if (arr[i] > arr[i + 1]) {
                        swap(&arr[i], &arr[i + 1]);
                    }
                }
            }
        }
    }
}

void print_array(int arr[], int n) {
    for (int i = 0; i < n; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");
}

void generate_random_array(int arr[], int n, int max_val) {
    srand(time(NULL));
    for (int i = 0; i < n; i++) {
        arr[i] = rand() % max_val;
    }
}

int is_sorted(int arr[], int n) {
    for (int i = 0; i < n - 1; i++) {
        if (arr[i] > arr[i + 1]) {
            return 0;
        }
    }
    return 1;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Uso: %s <tamanho_array> <num_threads>\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[1]);
    int num_threads = atoi(argv[2]);
    int *arr_base = malloc(n * sizeof(int));
    int *arr_temp = malloc(n * sizeof(int));

    generate_random_array(arr_base, n, 1000);

    printf("--- Configuração ---\n");
    printf("Tamanho do array: %d\n", n);
    printf("Threads: %d\n\n", num_threads);

    // --- Execução Serial ---
    memcpy(arr_temp, arr_base, n * sizeof(int));
    double start_time_serial = omp_get_wtime();
    odd_even_sort_serial(arr_temp, n);
    double end_time_serial = omp_get_wtime();
    double t_serial = end_time_serial - start_time_serial;
    printf("--- Serial ---\n");
    printf("Tempo de execução: %.6f segundos\n", t_serial);
    printf("Array está ordenado: %s\n\n", is_sorted(arr_temp, n) ? "Sim" : "Não");

    // --- Análise de Performance OpenMP ---
    printf("--- Análise de Performance OpenMP ---\n");

    // Static
    memcpy(arr_temp, arr_base, n * sizeof(int));
    double start_time_static = omp_get_wtime();
    odd_even_sort_openmp_static(arr_temp, n, num_threads);
    double end_time_static = omp_get_wtime();
    double t_parallel_static = end_time_static - start_time_static;
    double speedup_static = t_serial / t_parallel_static;
    double efficiency_static = speedup_static / num_threads;
    printf("Schedule: static\n");
    printf("  Tempo: %.6f s\n", t_parallel_static);
    printf("  Speedup: %.4f\n", speedup_static);
    printf("  Eficiência: %.4f\n", efficiency_static);
    printf("  Array está ordenado: %s\n\n", is_sorted(arr_temp, n) ? "Sim" : "Não");

    // Dynamic
    memcpy(arr_temp, arr_base, n * sizeof(int));
    double start_time_dynamic = omp_get_wtime();
    odd_even_sort_openmp_dynamic(arr_temp, n, num_threads);
    double end_time_dynamic = omp_get_wtime();
    double t_parallel_dynamic = end_time_dynamic - start_time_dynamic;
    double speedup_dynamic = t_serial / t_parallel_dynamic;
    double efficiency_dynamic = speedup_dynamic / num_threads;
    printf("Schedule: dynamic\n");
    printf("  Tempo: %.6f s\n", t_parallel_dynamic);
    printf("  Speedup: %.4f\n", speedup_dynamic);
    printf("  Eficiência: %.4f\n", efficiency_dynamic);
    printf("  Array está ordenado: %s\n\n", is_sorted(arr_temp, n) ? "Sim" : "Não");

    // Guided
    memcpy(arr_temp, arr_base, n * sizeof(int));
    double start_time_guided = omp_get_wtime();
    odd_even_sort_openmp_guided(arr_temp, n, num_threads);
    double end_time_guided = omp_get_wtime();
    double t_parallel_guided = end_time_guided - start_time_guided;
    double speedup_guided = t_serial / t_parallel_guided;
    double efficiency_guided = speedup_guided / num_threads;
    printf("Schedule: guided\n");
    printf("  Tempo: %.6f s\n", t_parallel_guided);
    printf("  Speedup: %.4f\n", speedup_guided);
    printf("  Eficiência: %.4f\n", efficiency_guided);
    printf("  Array está ordenado: %s\n", is_sorted(arr_temp, n) ? "Sim" : "Não");

    free(arr_base);
    free(arr_temp);
    return 0;
}