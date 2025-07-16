#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <mpi.h> 

// --- Funções Auxiliares ---

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
    double t_serial = 0.0;

    if (rank == 0) {
        arr = (int*)malloc(n * sizeof(int));
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

        // Medir tempo serial
        double start_serial = MPI_Wtime();
        odd_even_sort_serial(arr_serial_copy, n);
        double end_serial = MPI_Wtime();
        t_serial = end_serial - start_serial;

        printf("--- Serial ---\n");
        printf("Tempo de execução: %.6f segundos\n", t_serial);
        printf("Array está ordenado: %s\n\n", is_sorted(arr_serial_copy, n) ? "Sim" : "Não");
        
        free(arr_serial_copy);
    }

    // Broadcast do tempo serial para todos os processos
    MPI_Bcast(&t_serial, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    MPI_Scatter(arr, local_n, MPI_INT, local_arr, local_n, MPI_INT, 0, MPI_COMM_WORLD);

    MPI_Barrier(MPI_COMM_WORLD);
    double total_start = MPI_Wtime();
    double comm_time = 0.0;

    for (int phase = 0; phase < n; phase++) {
        single_phase_odd_even(local_arr, local_n, phase);
        int partner;
        if ((phase % 2) == 0) {
            partner = (rank % 2 == 0) ? rank + 1 : rank - 1;
        } else {
            partner = (rank % 2 != 0) ? rank + 1 : rank - 1;
        }

        if (partner >= 0 && partner < size) {
            int send_val, recv_val;
            if (rank < partner) {
                send_val = local_arr[local_n - 1];
            }
            else {
                send_val = local_arr[0];
            }
            
            double comm_start = MPI_Wtime();
            MPI_Sendrecv(&send_val, 1, MPI_INT, partner, 0,
                         &recv_val, 1, MPI_INT, partner, 0,
                         MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            comm_time += (MPI_Wtime() - comm_start);

            if (rank < partner) {
                if (send_val > recv_val) local_arr[local_n - 1] = recv_val;
            } else {
                if (recv_val > send_val) local_arr[0] = recv_val;
            }
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);
    double total_end = MPI_Wtime();
    double total_time = total_end - total_start;
    double computation_time = total_time - comm_time;

    if (arr == NULL) arr = (int*)malloc(n * sizeof(int));
    MPI_Allgather(local_arr, local_n, MPI_INT, arr, local_n, MPI_INT, MPI_COMM_WORLD);

    double t_parallel, comm_time_sum, computation_time_sum;
    MPI_Reduce(&total_time, &t_parallel, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Reduce(&comm_time, &comm_time_sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&computation_time, &computation_time_sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("--- Análise de Performance MPI ---\n");
        printf("Array ordenado: ");
        print_array(arr, n > 20 ? 20 : n);
        if (n > 20) printf("(exibindo apenas os 20 primeiros elementos)\n");
        printf("Array está ordenado: %s\n\n", is_sorted(arr, n) ? "Sim" : "Não");

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
    }

    free(arr);
    free(local_arr);
    MPI_Finalize();
    return 0;
}