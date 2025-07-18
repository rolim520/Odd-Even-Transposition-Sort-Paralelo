#define _POSIX_C_SOURCE 199309L // Define a versão do POSIX para ter acesso a clock_gettime
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "utils.h"
#include "csv_utils.h"

// Função para trocar dois elementos de posição em um array.
void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

// Implementação do algoritmo Odd-Even Transposition Sort de forma serial.
void odd_even_sort_serial(int arr[], int n) {
    int phase, i;
    // O algoritmo precisa de 'n' fases para garantir a ordenação completa.
    for (phase = 0; phase < n; phase++) {
        if (phase % 2 == 0) {
            // Fase par: compara e troca elementos em posições (arr[i-1], arr[i]) para i ímpar.
            // Isso cobre os pares (0,1), (2,3), (4,5), ...
            for (i = 1; i < n; i += 2) {
                if (arr[i - 1] > arr[i]) {
                    swap(&arr[i - 1], &arr[i]);
                }
            }
        } else {
            // Fase ímpar: compara e troca elementos em posições (arr[i], arr[i+1]) para i ímpar.
            // Isso cobre os pares (1,2), (3,4), (5,6), ...
            for (i = 1; i < n - 1; i += 2) {
                if (arr[i] > arr[i + 1]) {
                    swap(&arr[i], &arr[i + 1]);
                }
            }
        }
    }
}



int main(int argc, char *argv[]) {
    // Validação dos argumentos de linha de comando.
    if (argc != 2) {
        printf("Uso: %s <tamanho_array>\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[1]); // Converte o tamanho do array de string para inteiro.
    int *arr = malloc(n * sizeof(int)); // Aloca memória para o array.

    // Gera um array com números aleatórios.
    generate_random_array(arr, n, 1000);

    printf("--- Configuração ---\n");
    printf("Tamanho do array: %d\n\n", n);

    printf("--- Array Original ---\n");
    print_array(arr, n > 20 ? 20 : n); // Imprime os 20 primeiros elementos para visualização.
    if (n > 20) printf("(exibindo apenas os 20 primeiros elementos)\n");
    printf("\n");

    // Medição de tempo usando clock_gettime para alta precisão.
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    odd_even_sort_serial(arr, n); // Executa a ordenação.

    clock_gettime(CLOCK_MONOTONIC, &end);

    // Calcula o tempo total de execução.
    double time_taken = (end.tv_sec - start.tv_sec) + (double)(end.tv_nsec - start.tv_nsec) / 1e9;

    printf("--- Resultados ---\n");
    printf("Tempo de execução: %.6f segundos\n", time_taken);

    printf("Array ordenado: ");
    print_array(arr, n > 20 ? 20 : n);
    if (n > 20) printf("(exibindo apenas os 20 primeiros elementos)\n");

    // Verifica se o array está de fato ordenado.
    printf("Array está ordenado: %s\n", is_sorted(arr, n) ? "Sim" : "Não");

    // Salva o resultado (tamanho do array e tempo) no arquivo CSV.
    save_serial_result("data/serial.csv", n, time_taken);

    free(arr); // Libera a memória alocada.
    return 0;
}