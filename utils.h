#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/**
 * @brief Imprime os elementos de um array de inteiros.
 * 
 * @param arr O array a ser impresso.
 * @param n O número de elementos no array.
 */
void print_array(int arr[], int n) {
    for (int i = 0; i < n; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");
}

/**
 * @brief Preenche um array com números inteiros aleatórios.
 * 
 * @param arr O array a ser preenchido.
 * @param n O número de elementos no array.
 * @param max_val O valor máximo (exclusive) para os números aleatórios.
 */
void generate_random_array(int arr[], int n, int max_val) {
    // Usa o tempo atual como semente para o gerador de números aleatórios,
    // garantindo que os números sejam diferentes a cada execução.
    srand(time(NULL)); 
    for (int i = 0; i < n; i++) {
        arr[i] = rand() % max_val;
    }
}

/**
 * @brief Verifica se um array de inteiros está ordenado em ordem crescente.
 * 
 * @param arr O array a ser verificado.
 * @param n O número de elementos no array.
 * @return int Retorna 1 se o array estiver ordenado, 0 caso contrário.
 */
int is_sorted(int arr[], int n) {
    for (int i = 0; i < n - 1; i++) {
        if (arr[i] > arr[i + 1]) {
            return 0; // Encontra um par fora de ordem.
        }
    }
    return 1; // Nenhum par fora de ordem foi encontrado.
}

#endif // UTILS_H