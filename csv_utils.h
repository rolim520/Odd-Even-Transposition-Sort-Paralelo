#ifndef CSV_UTILS_H
#define CSV_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

// A simple function to check if a file exists.
static inline int file_exists(const char *path) {
    struct stat buffer;
    return (stat(path, &buffer) == 0);
}

// Ensures the directory for a given file path exists.
static inline void ensure_dir_exists(const char *filepath) {
    char *dir_path = strdup(filepath);
    if (dir_path == NULL) {
        perror("strdup failed");
        return;
    }

    char *last_slash = strrchr(dir_path, '/');
    if (last_slash != NULL) {
        *last_slash = '\0'; // Null-terminate to get only the directory path

        struct stat st = {0};
        if (stat(dir_path, &st) == -1) {
            if (mkdir(dir_path, 0775) != 0 && errno != EEXIST) {
                fprintf(stderr, "Failed to create directory '%s': %s\n", dir_path, strerror(errno));
            }
        }
    }
    free(dir_path);
}

// Appends a result from a serial execution to a CSV file.
static inline void save_serial_result(const char *filepath, int n, double time_taken) {
    ensure_dir_exists(filepath);
    FILE *fp;
    int needs_header = !file_exists(filepath);

    fp = fopen(filepath, "a");
    if (fp == NULL) {
        perror("Failed to open CSV for appending");
        return;
    }

    if (needs_header) {
        fprintf(fp, "Tamanho,Tempo(s)\n");
    }

    fprintf(fp, "%d,%.6f\n", n, time_taken);
    fclose(fp);
}

// Appends a result from an OpenMP execution to a CSV file.
static inline void save_openmp_result(const char *filepath, int n, int num_threads, const char *schedule, double time_taken, double speedup, double efficiency) {
    ensure_dir_exists(filepath);
    FILE *fp;
    int needs_header = !file_exists(filepath);

    fp = fopen(filepath, "a");
    if (fp == NULL) {
        perror("Failed to open CSV for appending");
        return;
    }

    if (needs_header) {
        fprintf(fp, "Tamanho,Threads,Schedule,Tempo(s),Speedup,Eficiencia\n");
    }

    fprintf(fp, "%d,%d,%s,%.6f,%.4f,%.4f\n", n, num_threads, schedule, time_taken, speedup, efficiency);
    fclose(fp);
}

// Appends a result from an MPI execution to a CSV file.
static inline void save_mpi_result(const char *filepath, int n, int size, double t_parallel, double computation_time_sum, double comm_time_sum, double overhead_abs, double overhead_rel, double comm_efficiency, double speedup, double efficiency) {
    ensure_dir_exists(filepath);
    FILE *fp;
    int needs_header = !file_exists(filepath);

    fp = fopen(filepath, "a");
    if (fp == NULL) {
        perror("Failed to open CSV for appending");
        return;
    }

    if (needs_header) {
        fprintf(fp, "Tamanho,Processos,TempoTotal(max),TempoComputacao(soma),TempoComunicacao(soma),OverheadAbsoluto,OverheadRelativo,EficienciaComunicacao,Speedup,Eficiencia\n");
    }

    fprintf(fp, "%d,%d,%.6f,%.6f,%.6f,%.6f,%.2f,%.4f,%.4f,%.4f\n", n, size, t_parallel, computation_time_sum, comm_time_sum, overhead_abs, overhead_rel, comm_efficiency, speedup, efficiency);
    fclose(fp);
}

#endif // CSV_UTILS_H
