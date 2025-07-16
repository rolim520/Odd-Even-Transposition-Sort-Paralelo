# Compilador C padrão (para serial e OpenMP)
CC = gcc
# Compilador wrapper do MPI
MPICC = mpicc

# Flags de compilação para todos
CFLAGS = -O2 -Wall -D_POSIX_C_SOURCE=199309L

# Flags de vinculação específicas
LDFLAGS_OPENMP = -fopenmp

# Nomes dos arquivos de saída
TARGET_SERIAL = build/odd_even_serial
TARGET_OPENMP = build/odd_even_openmp
TARGET_MPI = build/odd_even_mpi
TARGET_BENCHMARK = build/odd_even_serial_benchmark

# Regra padrão: compila todos os alvos
all: $(TARGET_SERIAL) $(TARGET_OPENMP) $(TARGET_MPI) $(TARGET_BENCHMARK)

# Regra para o código Serial
# $@ é o nome do alvo (build/odd_even_serial)
# $^ são as dependências (odd_even_serial.c)
$(TARGET_SERIAL): odd_even_serial.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ $^

# Regra para o código OpenMP
$(TARGET_OPENMP): odd_even_openmp.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(LDFLAGS_OPENMP) -o $@ $^

# Regra para o código MPI <-- MUDANÇA PRINCIPAL AQUI
$(TARGET_MPI): odd_even_mpi.c
	@mkdir -p $(dir $@)
	# Usa o compilador MPI: $(MPICC)
	$(MPICC) $(CFLAGS) -o $@ $^

# Regra para compilar o benchmark serial
$(TARGET_BENCHMARK): odd_even_serial_benchmark.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ $^

# Regra de limpeza
clean:
	rm -rf build

# Regra para testes básicos, conforme o documento do projeto
test: all
	@echo "--- Testando Serial (1K elementos) ---"
	@./$(TARGET_SERIAL) 1000
	@echo
	@echo "============================================================"
	@echo
	@echo "--- Testando OpenMP (1K elementos, 4 threads) ---"
	@./$(TARGET_OPENMP) 1000 4
	@echo
	@echo "============================================================"
	@echo
	@echo "--- Testando MPI (1K elementos, 4 processos) ---"
	@mpirun -np 4 ./$(TARGET_MPI) 1000

# Regra para o benchmark serial
benchmark: $(TARGET_BENCHMARK)
	@echo "Executando benchmark serial..."
	@./$(TARGET_BENCHMARK)