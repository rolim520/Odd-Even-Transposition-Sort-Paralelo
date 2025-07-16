#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h> // MPI Header

// --- Funções Auxiliares ---

void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

// Executa uma ÚNICA fase do Odd-Even Sort no array local
void single_phase_odd_even(int arr[], int n, int phase) {
    if (phase % 2 == 0) { // Fase Par
        for (int i = 1; i < n; i += 2) {
            if (arr[i - 1] > arr[i]) {
                swap(&arr[i - 1], &arr[i]);
            }
        }
    } else { // Fase Ímpar
        for (int i = 1; i < n - 1; i += 2) {
            if (arr[i] > arr[i + 1]) {
                swap(&arr[i], &arr[i + 1]);
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
    // Garante que a semente seja diferente para o rank 0, evitando arrays idênticos
    // em execuções muito próximas.
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
        printf("--- Configuração ---\n");
        printf("Tamanho do array: %d\n", n);
        printf("Processos: %d\n\n", size);

        printf("--- Array Original ---\n");
        print_array(arr, n > 20 ? 20 : n);
        if (n > 20) printf("(exibindo apenas os 20 primeiros elementos)\n");
        printf("\n");
    }

    // Distribuição de dados para todos os processos
    MPI_Scatter(arr, local_n, MPI_INT, local_arr, local_n, MPI_INT, 0, MPI_COMM_WORLD);

    // Sincronização e medição de tempo, conforme especificado no PDF
    MPI_Barrier(MPI_COMM_WORLD);
    double total_start = MPI_Wtime();
    double comm_time = 0.0;

    // Loop principal, executa n fases 
    for (int phase = 0; phase < n; phase++) {
        // 1. Computação local: uma fase de ordenação interna
        single_phase_odd_even(local_arr, local_n, phase);

        // 2. Determina o parceiro de comunicação para a fase atual
        int partner;
        if ((phase % 2) == 0) { // Fase Par Global
            partner = (rank % 2 == 0) ? rank + 1 : rank - 1;
        } else { // Fase Ímpar Global
            partner = (rank % 2 != 0) ? rank + 1 : rank - 1;
        }

        // 3. Comunicação nas fronteiras
        if (partner >= 0 && partner < size) {
            int send_val, recv_val;

            if (rank < partner) { // Processo da esquerda envia seu último elemento
                send_val = local_arr[local_n - 1];
            } else { // Processo da direita envia seu primeiro elemento
                send_val = local_arr[0];
            }
            
            // Medição específica do tempo de comunicação
            double comm_start = MPI_Wtime();
            MPI_Sendrecv(&send_val, 1, MPI_INT, partner, 0,
                         &recv_val, 1, MPI_INT, partner, 0,
                         MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            comm_time += (MPI_Wtime() - comm_start); // Acumula tempo

            // Compara os valores trocados e atualiza se necessário
            if (rank < partner) { // Esquerda fica com o menor
                if (send_val > recv_val) local_arr[local_n - 1] = recv_val;
            } else { // Direita fica com o maior
                if (recv_val > send_val) local_arr[0] = recv_val;
            }
        }
    }

    // Sincronização final e coleta de resultados
    MPI_Barrier(MPI_COMM_WORLD);
    double total_end = MPI_Wtime();
    double total_time = total_end - total_start;

    // Usa MPI_Allgather para que todos os processos recebam o resultado final 
    if (arr == NULL) arr = (int*)malloc(n * sizeof(int));
    MPI_Allgather(local_arr, local_n, MPI_INT, arr, local_n, MPI_INT, MPI_COMM_WORLD);

    // Reduz os tempos para o processo 0 fazer a análise
    double total_time_global, comm_time_global;
    MPI_Reduce(&total_time, &total_time_global, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Reduce(&comm_time, &comm_time_global, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    // Processo 0 imprime os resultados e a análise de performance
    if (rank == 0) {
        printf("Array ordenado: ");
        print_array(arr, n > 20 ? 20 : n);
        if (n > 20) printf("(exibindo apenas os 20 primeiros elementos)\n");
        printf("Array está ordenado: %s\n\n", is_sorted(arr, n) ? "Sim" : "Não");

        printf("--- Análise de Performance ---\n");
        printf("Tempo Total (max): %.6f s\n", total_time_global);
        printf("Tempo Comunicação (soma total): %.6f s\n", comm_time_global);

        double overhead_percentage = (comm_time_global / (total_time_global * size)) * 100;
        printf("Overhead de Comunicação (aprox): %.2f%%\n", overhead_percentage);
    }

    free(arr);
    free(local_arr);
    MPI_Finalize();
    return 0;
}