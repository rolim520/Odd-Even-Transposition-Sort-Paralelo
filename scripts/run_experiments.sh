#!/bin/bash

# Script para executar os experimentos de ordenação com diferentes parâmetros.

# Obter o diretório do script
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# Voltar para o diretório raiz do projeto
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
cd "$PROJECT_DIR"

# Garante que todos os executáveis estão compilados
echo "--- Compilando todos os programas... ---"
make all
echo "--- Compilação concluída. ---"
echo

# Cria o diretório para salvar os resultados, se não existir
mkdir -p data

# Define os parâmetros para os experimentos
SIZES=(1000 5000 10000 50000 100000)
PROCS=(1 2 4 8)
NUM_RUNS=3 # Define o número de vezes que cada experimento será executado

# --- Execução Serial ---
echo "============================================================"
echo "--- Iniciando Experimentos: Serial ---"
echo "============================================================"
for SIZE in "${SIZES[@]}"; do
    echo "Executando Serial com Tamanho do Array: $SIZE ($NUM_RUNS rodadas)"
    for i in $(seq 1 $NUM_RUNS); do
        echo "  Rodada $i de $NUM_RUNS"
        ./build/odd_even_serial "$SIZE"
    done
    echo "------------------------------------------------------------"
done
echo "--- Experimentos com Serial concluídos. ---"
echo

# --- Execução OpenMP ---
echo "============================================================"
echo "--- Iniciando Experimentos: OpenMP ---"
echo "============================================================"
for SIZE in "${SIZES[@]}"; do
    for PROC in "${PROCS[@]}"; do
        echo "Executando OpenMP com Tamanho: $SIZE, Threads: $PROC ($NUM_RUNS rodadas)"
        for i in $(seq 1 $NUM_RUNS); do
            echo "  Rodada $i de $NUM_RUNS"
            ./build/odd_even_openmp "$SIZE" "$PROC"
        done
        echo "------------------------------------------------------------"
    done
done
echo "--- Experimentos com OpenMP concluídos. ---"
echo

# --- Execução MPI ---
echo "============================================================"
echo "--- Iniciando Experimentos: MPI ---"
echo "============================================================"
for SIZE in "${SIZES[@]}"; do
    for PROC in "${PROCS[@]}"; do
        echo "Executando MPI com Tamanho: $SIZE, Processos: $PROC ($NUM_RUNS rodadas)"
        for i in $(seq 1 $NUM_RUNS); do
            echo "  Rodada $i de $NUM_RUNS"
            mpirun --oversubscribe -np "$PROC" ./build/odd_even_mpi "$SIZE"
        done
        echo "------------------------------------------------------------"
    done
done
echo "--- Experimentos com MPI concluídos. ---"
echo
echo "Todos os experimentos foram finalizados."
