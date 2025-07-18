#ifndef CSV_UTILS_H
#define CSV_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h> // Para stat() e mkdir()
#include <errno.h>    // Para verificar erros como EEXIST

/**
 * @brief Verifica de forma simples se um arquivo existe no caminho especificado.
 * 
 * @param path O caminho para o arquivo.
 * @return int Retorna 1 se o arquivo existe, 0 caso contrário.
 */
static inline int file_exists(const char *path) {
    struct stat buffer;
    return (stat(path, &buffer) == 0);
}

/**
 * @brief Garante que o diretório para um determinado caminho de arquivo exista.
 * Se o diretório não existir, ele será criado.
 * 
 * @param filepath O caminho completo do arquivo cujo diretório deve ser verificado/criado.
 */
static inline void ensure_dir_exists(const char *filepath) {
    char *dir_path = strdup(filepath); // Duplica a string para poder modificá-la.
    if (dir_path == NULL) {
        perror("Falha ao duplicar o caminho do arquivo (strdup)");
        return;
    }

    // Encontra a última barra '/' para isolar o caminho do diretório.
    char *last_slash = strrchr(dir_path, '/');
    if (last_slash != NULL) {
        *last_slash = '\0'; // Termina a string na barra, deixando apenas o caminho do diretório.

        struct stat st = {0};
        // Verifica se o diretório existe.
        if (stat(dir_path, &st) == -1) {
            // Se não existir, tenta criá-lo com permissões de leitura/escrita/execução para dono/grupo e leitura para outros.
            if (mkdir(dir_path, 0775) != 0 && errno != EEXIST) {
                fprintf(stderr, "Falha ao criar diretório '%s': %s\n", dir_path, strerror(errno));
            }
        }
    }
    free(dir_path); // Libera a memória da string duplicada.
}

/**
 * @brief Anexa o resultado de uma execução serial a um arquivo CSV.
 * Se o arquivo não existir, cria-o e adiciona um cabeçalho.
 * 
 * @param filepath O caminho para o arquivo CSV.
 * @param n O tamanho da entrada (array).
 * @param time_taken O tempo de execução gasto.
 */
static inline void save_serial_result(const char *filepath, int n, double time_taken) {
    ensure_dir_exists(filepath); // Garante que o diretório 'data/' existe.
    FILE *fp;
    int needs_header = !file_exists(filepath); // Verifica se o cabeçalho é necessário.

    fp = fopen(filepath, "a"); // Abre o arquivo em modo "append" (anexar).
    if (fp == NULL) {
        perror("Falha ao abrir CSV para anexar dados");
        return;
    }

    if (needs_header) {
        fprintf(fp, "Tamanho,Tempo(s)\n");
    }

    fprintf(fp, "%d,%.6f\n", n, time_taken);
    fclose(fp);
}

/**
 * @brief Anexa o resultado de uma execução OpenMP a um arquivo CSV.
 * 
 * @param filepath Caminho para o arquivo CSV.
 * @param n Tamanho da entrada.
 * @param num_threads Número de threads usadas.
 * @param schedule Política de escalonamento (schedule) usada.
 * @param time_taken Tempo de execução.
 * @param speedup Speedup calculado.
 * @param efficiency Eficiência calculada.
 */
static inline void save_openmp_result(const char *filepath, int n, int num_threads, const char *schedule, double time_taken, double speedup, double efficiency) {
    ensure_dir_exists(filepath);
    FILE *fp;
    int needs_header = !file_exists(filepath);

    fp = fopen(filepath, "a");
    if (fp == NULL) {
        perror("Falha ao abrir CSV para anexar dados");
        return;
    }

    if (needs_header) {
        fprintf(fp, "Tamanho,Threads,Schedule,Tempo(s),Speedup,Eficiencia\n");
    }

    fprintf(fp, "%d,%d,%s,%.6f,%.4f,%.4f\n", n, num_threads, schedule, time_taken, speedup, efficiency);
    fclose(fp);
}

/**
 * @brief Anexa o resultado de uma execução MPI a um arquivo CSV.
 * 
 * @param filepath Caminho para o arquivo CSV.
 * @param n Tamanho da entrada global.
 * @param size Número de processos.
 * @param t_parallel Tempo de execução paralelo (máximo entre os processos).
 * @param computation_time_sum Soma do tempo de computação de todos os processos.
 * @param comm_time_sum Soma do tempo de comunicação de todos os processos.
 * @param overhead_abs Overhead absoluto.
 * @param overhead_rel Overhead relativo (percentual).
 * @param comm_efficiency Eficiência da comunicação.
 * @param speedup Speedup calculado.
 * @param efficiency Eficiência calculada.
 */
static inline void save_mpi_result(const char *filepath, int n, int size, double t_parallel, double computation_time_sum, double comm_time_sum, double overhead_abs, double overhead_rel, double comm_efficiency, double speedup, double efficiency) {
    ensure_dir_exists(filepath);
    FILE *fp;
    int needs_header = !file_exists(filepath);

    fp = fopen(filepath, "a");
    if (fp == NULL) {
        perror("Falha ao abrir CSV para anexar dados");
        return;
    }

    if (needs_header) {
        fprintf(fp, "Tamanho,Processos,TempoTotal(max),TempoComputacao(soma),TempoComunicacao(soma),OverheadAbsoluto,OverheadRelativo,EficienciaComunicacao,Speedup,Eficiencia\n");
    }

    fprintf(fp, "%d,%d,%.6f,%.6f,%.6f,%.6f,%.2f,%.4f,%.4f,%.4f\n", n, size, t_parallel, computation_time_sum, comm_time_sum, overhead_abs, overhead_rel, comm_efficiency, speedup, efficiency);
    fclose(fp);
}

#endif // CSV_UTILS_H