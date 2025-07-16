#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h> // MPI Header

// --- Funções Auxiliares (a maioria inalterada) ---

void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

// Realiza uma única fase da ordenação Odd-Even.
// is_odd_phase = 0 para fase par, 1 para fase ímpar.
void single_phase_odd_even(int arr[], int n, int is_odd_phase) {
    if (is_odd_phase) { // Fase Ímpar: compara (arr[1], arr[2]), (arr[3], arr[4]), ...
        for (int i = 1; i < n - 1; i += 2) {
            if (arr[i] > arr[i + 1]) {
                swap(&arr[i], &arr[i + 1]);
            }
        }
    } else { // Fase Par: compara (arr[0], arr[1]), (arr[2], arr[3]), ...
        for (int i = 1; i < n; i += 2) {
            if (arr[i - 1] > arr[i]) {
                swap(&arr[i - 1], &arr[i]);
            }
        }
    }
}

// Renomeada para clareza, será usada para ordenar os blocos locais
void odd_even_sort_local(int arr[], int n) {
    for (int phase = 0; phase < n; phase++) {
        if (phase % 2 == 0) {
            for (int i = 1; i < n; i += 2) {
                if (arr[i - 1] > arr[i]) {
                    swap(&arr[i - 1], &arr[i]);
                }
            }
        } else {
            for (int i = 1; i < n - 1; i += 2) {
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
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc != 2) {
        if (rank == 0) printf("Uso: mpirun -np <num_procs> %s <tamanho_array>\n", argv[0]);
        MPI_Finalize();
        return 1;
    }

    int n = atoi(argv[1]);
    if (n % size != 0) {
        if (rank == 0) printf("O tamanho do array (%d) deve ser divisível pelo número de processos (%d).\n", n, size);
        MPI_Finalize();
        return 1;
    }

    int local_n = n / size;
    int *local_arr = (int*)malloc(local_n * sizeof(int));
    int *arr = NULL;

    if (rank == 0) {
        arr = (int*)malloc(n * sizeof(int));
        generate_random_array(arr, n, 1000);
        printf("Array original: ");
        print_array(arr, n > 20 ? 20 : n);
        if(n > 20) printf("...\n");
    }

    MPI_Barrier(MPI_COMM_WORLD);
    double start_time = MPI_Wtime();

    MPI_Scatter(arr, local_n, MPI_INT, local_arr, local_n, MPI_INT, 0, MPI_COMM_WORLD);

    // --- Loop Principal da Ordenação Otimizada ---
    for (int phase = 0; phase < size; phase++) {
        // 1. Ordena o bloco local. CRUCIAL para ter os elementos de fronteira corretos.
        odd_even_sort_local(local_arr, local_n);

        int partner;
        // 2. Determina o parceiro de comunicação para a fase atual
        if (phase % 2 == 0) { // Fase PAR: pares (0,1), (2,3)...
            partner = (rank % 2 == 0) ? rank + 1 : rank - 1;
        } else { // Fase ÍMPAR: pares (1,2), (3,4)...
            partner = (rank % 2 == 0) ? rank - 1 : rank + 1;
        }

        // Se o parceiro é válido (não está fora dos limites)
        if (partner >= 0 && partner < size) {
            // Aloca um buffer temporário para receber os dados do parceiro
            int *partner_arr = (int*)malloc(local_n * sizeof(int));
            
            // Troca os blocos de dados com o parceiro
            MPI_Sendrecv(local_arr, local_n, MPI_INT, partner, 0,
                         partner_arr, local_n, MPI_INT, partner, 0,
                         MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            // Aloca um buffer para mesclar os dois blocos
            int *merged_arr = (int*)malloc(local_n * 2 * sizeof(int));
            
            // Mescla os dados dependendo se é o processo da esquerda ou da direita
            if (rank < partner) {
                for (int i = 0; i < local_n; i++) {
                    merged_arr[i] = local_arr[i];
                    merged_arr[i + local_n] = partner_arr[i];
                }
            } else {
                for (int i = 0; i < local_n; i++) {
                    merged_arr[i] = partner_arr[i];
                    merged_arr[i + local_n] = local_arr[i];
                }
            }

            // Ordena o bloco mesclado
            odd_even_sort_local(merged_arr, local_n * 2);

            // Mantém a metade correta dos dados ordenados
            if (rank < partner) {
                for (int i = 0; i < local_n; i++) {
                    local_arr[i] = merged_arr[i];
                }
            } else {
                for (int i = 0; i < local_n; i++) {
                    local_arr[i] = merged_arr[i + local_n];
                }
            }

            // Libera a memória alocada
            free(partner_arr);
            free(merged_arr);
        }
    }

    // Ordenação final para garantir a ordem local após a última troca
    odd_even_sort_local(local_arr, local_n);

    MPI_Gather(local_arr, local_n, MPI_INT, arr, local_n, MPI_INT, 0, MPI_COMM_WORLD);

    double end_time = MPI_Wtime();

    if (rank == 0) {
        printf("Tempo de execução MPI (otimizado): %f segundos\n", end_time - start_time);
        printf("Array ordenado: ");
        print_array(arr, n > 20 ? 20 : n);
        if(n > 20) printf("...\n");
        printf("Array está ordenado: %s\n", is_sorted(arr, n) ? "Sim" : "Não");
        free(arr);
    }

    free(local_arr);
    MPI_Finalize();
    return 0;
}
