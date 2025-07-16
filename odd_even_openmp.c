#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>

void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
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
    int *arr = malloc(n * sizeof(int));

    // Gerar array aleatório
    generate_random_array(arr, n, 1000);

    printf("--- Configuração ---\n");
    printf("Tamanho do array: %d\n", n);
    printf("Threads: %d\n\n", num_threads);

    printf("--- Array Original ---\n");
    print_array(arr, n > 20 ? 20 : n);
    if (n > 20) printf("(exibindo apenas os 20 primeiros elementos)\n");
    printf("\n");

    // Medição de tempo
    double start_time = omp_get_wtime();
    odd_even_sort_openmp_static(arr, n, num_threads);
    double end_time = omp_get_wtime();

    printf("--- Resultados ---\n");
    printf("Tempo de execução: %.6f segundos\n", end_time - start_time);

    printf("Array ordenado: ");
    print_array(arr, n > 20 ? 20 : n);
    if (n > 20) printf("(exibindo apenas os 20 primeiros elementos)\n");

    printf("Array está ordenado: %s\n", is_sorted(arr, n) ? "Sim" : "Não");

    free(arr);
    return 0;
}