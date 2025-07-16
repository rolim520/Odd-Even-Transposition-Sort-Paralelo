#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "utils.h"

void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

void odd_even_sort_serial(int arr[], int n) {
    int phase, i;
    for (phase = 0; phase < n; phase++) {
        if (phase % 2 == 0) {
            // Fase par: compara (i-1, i) para i ímpar
            for (i = 1; i < n; i += 2) {
                if (arr[i - 1] > arr[i]) {
                    swap(&arr[i - 1], &arr[i]);
                }
            }
        } else {
            // Fase ímpar: compara (i, i+1) para i ímpar
            for (i = 1; i < n - 1; i += 2) {
                if (arr[i] > arr[i + 1]) {
                    swap(&arr[i], &arr[i + 1]);
                }
            }
        }
    }
}



int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Uso: %s <tamanho_array>\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[1]);
    int *arr = malloc(n * sizeof(int));

    // Gerar array aleatório
    generate_random_array(arr, n, 1000);

    printf("--- Configuração ---\n");
    printf("Tamanho do array: %d\n\n", n);

    printf("--- Array Original ---\n");
    print_array(arr, n > 20 ? 20 : n);
    if (n > 20) printf("(exibindo apenas os 20 primeiros elementos)\n");
    printf("\n");

    // Medição de tempo
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    odd_even_sort_serial(arr, n);

    clock_gettime(CLOCK_MONOTONIC, &end);

    double time_taken = (end.tv_sec - start.tv_sec) + (double)(end.tv_nsec - start.tv_nsec) / 1e9;

    printf("--- Resultados ---\n");
    printf("Tempo de execução: %.6f segundos\n", time_taken);

    printf("Array ordenado: ");
    print_array(arr, n > 20 ? 20 : n);
    if (n > 20) printf("(exibindo apenas os 20 primeiros elementos)\n");

    printf("Array está ordenado: %s\n", is_sorted(arr, n) ? "Sim" : "Não");

    free(arr);
    return 0;
}