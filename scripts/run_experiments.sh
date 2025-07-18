#!/bin/bash

# Script para executar os experimentos de ordenação com diferentes parâmetros.
# Este script automatiza a coleta de dados para as versões serial, OpenMP e MPI do algoritmo.

# --- Navegação e Preparação ---

# Obtém o diretório onde o script está localizado.
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# Navega para o diretório raiz do projeto (um nível acima do diretório 'scripts').
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
cd "$PROJECT_DIR"

# Garante que todos os executáveis estão compilados e atualizados.
echo "--- Compilando todos os programas... ---"
make all
echo "--- Compilação concluída. ---"
echo

# Cria o diretório 'data' para salvar os arquivos CSV de resultados, se ele não existir.
mkdir -p data

# --- Definição dos Parâmetros dos Experimentos ---

# Define os diferentes tamanhos de array a serem testados.
SIZES=(1000 5000 10000 50000 100000)
# Define o número de processos (para MPI) ou threads (para OpenMP) a serem testados.
PROCS=(1 2 4 8)
# Define o número de vezes que cada combinação de parâmetros será executada para obter uma média.
NUM_RUNS=3 

# --- Execução dos Experimentos ---

# --- 1. Execução Serial ---
echo "============================================================"
echo "--- Iniciando Experimentos: Serial ---"
echo "============================================================"
for SIZE in "${SIZES[@]}"; do
    echo "Executando Serial com Tamanho do Array: $SIZE ($NUM_RUNS rodadas)"
    for i in $(seq 1 $NUM_RUNS); do
        echo "  Rodada $i de $NUM_RUNS"
        # Executa a versão serial e os resultados são salvos em CSV pela própria aplicação.
        ./build/odd_even_serial "$SIZE"
    done
    echo "------------------------------------------------------------"
done
echo "--- Experimentos com Serial concluídos. ---"
echo

# --- 2. Execução OpenMP ---
echo "============================================================"
echo "--- Iniciando Experimentos: OpenMP ---"
echo "============================================================"
for SIZE in "${SIZES[@]}"; do
    for PROC in "${PROCS[@]}"; do
        echo "Executando OpenMP com Tamanho: $SIZE, Threads: $PROC ($NUM_RUNS rodadas)"
        for i in $(seq 1 $NUM_RUNS); do
            echo "  Rodada $i de $NUM_RUNS"
            # Executa a versão OpenMP com os parâmetros de tamanho e número de threads.
            ./build/odd_even_openmp "$SIZE" "$PROC"
        done
        echo "------------------------------------------------------------"
    done
done
echo "--- Experimentos com OpenMP concluídos. ---"
echo

# --- 3. Execução MPI ---
echo "============================================================"
echo "--- Iniciando Experimentos: MPI ---"
echo "============================================================"
for SIZE in "${SIZES[@]}"; do
    for PROC in "${PROCS[@]}"; do
        echo "Executando MPI com Tamanho: $SIZE, Processos: $PROC ($NUM_RUNS rodadas)"
        for i in $(seq 1 $NUM_RUNS); do
            echo "  Rodada $i de $NUM_RUNS"
            # Executa a versão MPI usando 'mpirun' com o número de processos especificado.
            # A flag --oversubscribe permite rodar mais processos que o número de cores físicas, útil para testes.
            mpirun --oversubscribe -np "$PROC" ./build/odd_even_mpi "$SIZE"
        done
        echo "------------------------------------------------------------"
    done
done
echo "--- Experimentos com MPI concluídos. ---"
echo
echo "Todos os experimentos foram finalizados."