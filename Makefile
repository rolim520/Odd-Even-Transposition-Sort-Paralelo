# Compilador C padrão (para serial e OpenMP)
CC = gcc
# Compilador wrapper do MPI
MPICC = mpicc

# Flags de compilação para todos
# -O2: Nível de otimização
# -Wall: Habilita a maioria dos warnings
# -D_POSIX_C_SOURCE=199309L: Necessário para clock_gettime
CFLAGS = -O2 -Wall -D_POSIX_C_SOURCE=199309L

# Flags de vinculação específicas para OpenMP
# -fopenmp: Habilita o suporte a OpenMP no GCC
LDFLAGS_OPENMP = -fopenmp

# Nomes dos arquivos de saída (executáveis)
TARGET_SERIAL = build/odd_even_serial
TARGET_OPENMP = build/odd_even_openmp
TARGET_MPI = build/odd_even_mpi

# Regra padrão: compila todos os alvos
# Esta é a regra executada quando se digita 'make' sem argumentos.
# Ela depende das regras dos três executáveis.
all: $(TARGET_SERIAL) $(TARGET_OPENMP) $(TARGET_MPI)

# Regra para o código Serial
# $@ é uma variável automática do Make que representa o nome do alvo (build/odd_even_serial)
# $^ é uma variável automática que representa todas as dependências (odd_even_serial.c utils.h csv_utils.h)
$(TARGET_SERIAL): odd_even_serial.c utils.h csv_utils.h
	@mkdir -p $(dir $@) # Cria o diretório 'build/' se não existir. O '@' suprime a exibição do comando.
	# Compila o código
	$(CC) $(CFLAGS) -o $@ odd_even_serial.c

# Regra para o código OpenMP
$(TARGET_OPENMP): odd_even_openmp.c utils.h csv_utils.h
	@mkdir -p $(dir $@)
	# Compila usando as flags do OpenMP
	$(CC) $(CFLAGS) $(LDFLAGS_OPENMP) -o $@ odd_even_openmp.c

# Regra para o código MPI
$(TARGET_MPI): odd_even_mpi.c utils.h csv_utils.h
	@mkdir -p $(dir $@)
	# Compila usando o compilador wrapper do MPI
	$(MPICC) $(CFLAGS) -o $@ odd_even_mpi.c

# Regra de limpeza: remove o diretório de build e seu conteúdo
clean:
	rm -rf build

# Regra para testes básicos, conforme o documento do projeto
# Executa cada versão do programa com um input pequeno para verificação.
test: all
	@mkdir -p data # Garante que o diretório de dados existe
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