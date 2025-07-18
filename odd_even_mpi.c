#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h> 
#include "utils.h" 
#include "csv_utils.h" 

// Função para trocar dois elementos de posição.
void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

// Versão serial do algoritmo para cálculo de speedup no processo raiz.
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

// Executa uma única fase (par ou ímpar) do algoritmo no array local.
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



int main(int argc, char *argv[]) {
    // Inicializa o ambiente MPI.
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // Obtém o rank (ID) do processo atual.
    MPI_Comm_size(MPI_COMM_WORLD, &size); // Obtém o número total de processos.

    // Validação dos argumentos de linha de comando.
    if (argc != 2) {
        if (rank == 0) printf("Uso: mpirun -np <num_procs> %s <tamanho_array>\n", argv[0]);
        MPI_Finalize();
        return 1;
    }

    int n = atoi(argv[1]);
    
    // --- Lógica para Distribuição Desigual de Dados ---
    // Calcula o tamanho base do bloco de dados para cada processo.
    int base_chunk = n / size;
    // Calcula o resto da divisão, que será distribuído entre os primeiros processos.
    int remainder = n % size;
    // Cada processo calcula o tamanho do seu bloco local (local_n).
    // Os 'remainder' primeiros processos recebem um elemento a mais.
    int local_n = (rank < remainder) ? base_chunk + 1 : base_chunk;

    // Aloca memória para o sub-array local de cada processo.
    int *local_arr = (int*)malloc(local_n * sizeof(int));
    // O array completo 'arr' é alocado em todos os processos para receber o resultado final.
    int *arr = (int*)malloc(n * sizeof(int));
    double t_serial = 0.0;
    
    // Prepara os parâmetros para o MPI_Scatterv e MPI_Allgatherv.
    // 'sendcounts': array que diz quantos elementos cada processo envia/recebe.
    // 'displs': array que diz o deslocamento (índice inicial) dos dados de cada processo no array global.
    int *sendcounts = malloc(size * sizeof(int));
    int *displs = malloc(size * sizeof(int));
    int current_displ = 0;
    for (int i = 0; i < size; i++) {
        sendcounts[i] = (i < remainder) ? base_chunk + 1 : base_chunk;
        displs[i] = current_displ;
        current_displ += sendcounts[i];
    }

    // O processo raiz (rank 0) inicializa o array e calcula o tempo serial.
    if (rank == 0) {
        int *arr_serial_copy = (int*)malloc(n * sizeof(int));
        
        generate_random_array(arr, n, 1000);
        memcpy(arr_serial_copy, arr, n * sizeof(int));

        printf("--- Configuração ---\n");
        printf("Tamanho do array: %d\n", n);
        printf("Processos: %d\n\n", size);

        printf("--- Array Original ---\n");
        print_array(arr, n > 20 ? 20 : n);
        if (n > 20) printf("(exibindo apenas os 20 primeiros elementos)\n");
        printf("\n");

        // Mede o tempo de execução da versão serial para usar como base para o speedup.
        double start_serial = MPI_Wtime();
        odd_even_sort_serial(arr_serial_copy, n);
        double end_serial = MPI_Wtime();
        t_serial = end_serial - start_serial;

        printf("--- Serial ---\n");
        printf("Tempo de execução: %.6f segundos\n", t_serial);
        printf("Array está ordenado: %s\n\n", is_sorted(arr_serial_copy, n) ? "Sim" : "Não");
        
        free(arr_serial_copy);
    }

    // O processo raiz envia o tempo serial para todos os outros processos.
    MPI_Bcast(&t_serial, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // Distribui os dados do array 'arr' (no processo 0) para os 'local_arr' de todos os processos.
    // MPI_Scatterv é usado para lidar com a distribuição desigual de dados.
    MPI_Scatterv(arr, sendcounts, displs, MPI_INT,
                 local_arr, local_n, MPI_INT,
                 0, MPI_COMM_WORLD);

    // Sincroniza todos os processos antes de iniciar a medição do tempo paralelo.
    MPI_Barrier(MPI_COMM_WORLD);
    double total_start = MPI_Wtime();
    double comm_time = 0.0; // Variável para acumular o tempo de comunicação.

    // Loop principal do Odd-Even Sort paralelo.
    for (int phase = 0; phase < n; phase++) {
        // 1. Ordenação local: cada processo executa uma fase no seu sub-array.
        single_phase_odd_even(local_arr, local_n, phase);

        // 2. Comunicação de fronteiras: troca de elementos com processos vizinhos.
        int partner;
        // Determina o processo parceiro para a troca nesta fase.
        if ((phase % 2) == 0) { // Fase par
            partner = (rank % 2 == 0) ? rank + 1 : rank - 1;
        } else { // Fase ímpar
            partner = (rank % 2 != 0) ? rank + 1 : rank - 1;
        }

        // Verifica se o parceiro é válido (dentro dos limites de 0 a size-1).
        if (partner >= 0 && partner < size) {
            int send_val, recv_val;
            // Determina qual elemento da fronteira enviar.
            if (rank < partner) {
                send_val = local_arr[local_n - 1]; // Envia o último elemento.
            }
            else {
                send_val = local_arr[0]; // Envia o primeiro elemento.
            }
            
            // Mede o tempo da operação de comunicação.
            double comm_start = MPI_Wtime();
            // MPI_Sendrecv envia e recebe mensagens simultaneamente, evitando deadlocks.
            MPI_Sendrecv(&send_val, 1, MPI_INT, partner, 0,
                         &recv_val, 1, MPI_INT, partner, 0,
                         MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            comm_time += (MPI_Wtime() - comm_start);

            // Compara o elemento da fronteira local com o recebido e atualiza se necessário.
            if (rank < partner) { // Processo de rank menor compara seu último com o primeiro do vizinho.
                if (send_val > recv_val) local_arr[local_n - 1] = recv_val;
            } else { // Processo de rank maior compara seu primeiro com o último do vizinho.
                if (recv_val > send_val) local_arr[0] = recv_val;
            }
        }
    }

    // Sincroniza novamente antes de finalizar a medição e coletar os resultados.
    MPI_Barrier(MPI_COMM_WORLD);
    double total_end = MPI_Wtime();
    double total_time = total_end - total_start;
    double computation_time = total_time - comm_time;

    // Coleta os sub-arrays ordenados de todos os processos e monta o array final em 'arr'.
    // MPI_Allgatherv faz com que todos os processos recebam o resultado completo.
    MPI_Allgatherv(local_arr, local_n, MPI_INT,
                   arr, sendcounts, displs, MPI_INT,
                   MPI_COMM_WORLD);

    // Reduz (agrega) os tempos de todos os processos no processo raiz para análise.
    double t_parallel, comm_time_sum, computation_time_sum;
    // MPI_MAX para o tempo total, pois o tempo da execução é o do processo mais lento.
    MPI_Reduce(&total_time, &t_parallel, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    // MPI_SUM para os tempos de comunicação e computação para calcular o overhead.
    MPI_Reduce(&comm_time, &comm_time_sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&computation_time, &computation_time_sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    // O processo raiz imprime os resultados e salva no CSV.
    if (rank == 0) {
        printf("--- Análise de Performance MPI ---\n");
        printf("Array ordenado: ");
        print_array(arr, n > 20 ? 20 : n);
        if (n > 20) printf("(exibindo apenas os 20 primeiros elementos)\n");
        printf("Array está ordenado: %s\n\n", is_sorted(arr, n) ? "Sim" : "Não");

        // Cálculo das métricas de desempenho.
        double t_computation_avg = computation_time_sum / size;
        double overhead_abs = t_parallel - t_computation_avg;
        double overhead_rel = (comm_time_sum / (computation_time_sum + comm_time_sum)) * 100;
        double comm_efficiency = computation_time_sum / (computation_time_sum + comm_time_sum);
        double speedup = t_serial / t_parallel;
        double efficiency = speedup / size;

        printf("Tempo Total (max): %.6f s\n", t_parallel);
        printf("Tempo de Computação (soma): %.6f s\n", computation_time_sum);
        printf("Tempo de Comunicação (soma): %.6f s\n", comm_time_sum);
        printf("Overhead Absoluto: %.6f s\n", overhead_abs);
        printf("Overhead Relativo: %.2f%%\n", overhead_rel);
        printf("Eficiência de Comunicação: %.4f\n", comm_efficiency);
        printf("Speedup: %.4f\n", speedup);
        printf("Eficiência: %.4f\n", efficiency);

        save_mpi_result("data/mpi.csv", n, size, t_parallel, computation_time_sum, comm_time_sum, overhead_abs, overhead_rel, comm_efficiency, speedup, efficiency);
    }

    // Libera toda a memória alocada.
    free(arr);
    free(local_arr);
    free(sendcounts);
    free(displs);
    
    // Finaliza o ambiente MPI.
    MPI_Finalize();
    return 0;
}
