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
    if (argc != 2) {
        printf("Uso: %s <tamanho_array>\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[1]);
    int *arr = malloc(n * sizeof(int));

    // Gerar array aleatório
    generate_random_array(arr, n, 1000);

    printf("Array original: ");
    if (n <= 20) {
        print_array(arr, n);
    } else {
        print_array(arr, 20);
        printf("(exibindo apenas os 20 primeiros elementos)\n");
    }

    // Medição de tempo seria aqui
    odd_even_sort_serial(arr, n);

    printf("Array ordenado: ");
    if (n <= 20) {
        print_array(arr, n);
    } else {
        print_array(arr, 20);
        printf("(exibindo apenas os 20 primeiros elementos)\n");
    }

    printf("Array está ordenado: %s\n", is_sorted(arr, n) ? "Sim" : "Não");

    free(arr);
    return 0;
}
