import pandas as pd
import matplotlib.pyplot as plt
import os
import seaborn as sns

def gerar_grafico_tempo_execucao():
    """
    Gera e salva um gráfico de linha comparando o tempo de execução
    das implementações Serial, OpenMP (com diferentes schedules) e MPI.
    """
    print("Iniciando a geração do gráfico...")

    # --- 1. Definição dos Caminhos e Criação do Diretório ---
    caminho_base_dados = 'data'
    caminho_graficos = 'graficos'
    
    if not os.path.exists(caminho_graficos):
        print(f"Criando diretório '{caminho_graficos}'...")
        os.makedirs(caminho_graficos)

    arquivo_serial = os.path.join(caminho_base_dados, 'serial.csv')
    arquivo_openmp = os.path.join(caminho_base_dados, 'openmp.csv')
    arquivo_mpi = os.path.join(caminho_base_dados, 'mpi.csv')

    # --- 2. Carregamento e Processamento dos Dados ---
    try:
        df_serial = pd.read_csv(arquivo_serial)
        df_openmp = pd.read_csv(arquivo_openmp)
        df_mpi = pd.read_csv(arquivo_mpi)
        print("Arquivos CSV carregados com sucesso.")
    except FileNotFoundError as e:
        print(f"Erro: Arquivo não encontrado - {e}")
        print("Verifique se os arquivos CSV estão no diretório 'data/'.")
        return

    # --- Processamento Serial ---
    media_serial = df_serial.groupby('Tamanho')['Tempo(s)'].mean().reset_index()
    print("\nDados da execução Serial (Média):")
    print(media_serial)

    # --- Processamento OpenMP ---
    # Filtra para usar apenas os dados com 8 threads
    df_openmp_8_threads = df_openmp[df_openmp['Threads'] == 8].copy()
    # Calcula a média de tempo para cada schedule e tamanho
    media_openmp = df_openmp_8_threads.groupby(['Tamanho', 'Schedule'])['Tempo(s)'].mean().reset_index()
    
    # Separa os dados por schedule para plotagem
    media_openmp_static = media_openmp[media_openmp['Schedule'] == 'static']
    media_openmp_dynamic = media_openmp[media_openmp['Schedule'] == 'dynamic']
    media_openmp_guided = media_openmp[media_openmp['Schedule'] == 'guided']
    
    print("\nDados da execução OpenMP (Média, 8 Threads):")
    print(media_openmp)

    # --- Processamento MPI ---
    # Filtra para usar apenas os dados com 8 processos
    df_mpi_8_procs = df_mpi[df_mpi['Processos'] == 8].copy()
    # Calcula a média de tempo para cada tamanho
    media_mpi = df_mpi_8_procs.groupby('Tamanho')['TempoTotal(max)'].mean().reset_index()
    media_mpi = media_mpi.rename(columns={'TempoTotal(max)': 'Tempo(s)'})
    
    print("\nDados da execução MPI (Média, 8 Processos):")
    print(media_mpi)

    # --- 3. Geração do Gráfico ---
    plt.style.use('seaborn-v0_8-whitegrid')
    plt.figure(figsize=(14, 8))

    # Plota os dados
    plt.plot(media_serial['Tamanho'], media_serial['Tempo(s)'], marker='o', linestyle='-', label='Serial')
    
    plt.plot(media_openmp_static['Tamanho'], media_openmp_static['Tempo(s)'], marker='s', linestyle='--', label='OpenMP (static, 8 threads)')
    plt.plot(media_openmp_dynamic['Tamanho'], media_openmp_dynamic['Tempo(s)'], marker='x', linestyle=':', label='OpenMP (dynamic, 8 threads)')
    plt.plot(media_openmp_guided['Tamanho'], media_openmp_guided['Tempo(s)'], marker='d', linestyle='-.', label='OpenMP (guided, 8 threads)')
    
    plt.plot(media_mpi['Tamanho'], media_mpi['Tempo(s)'], marker='^', linestyle='-', label='MPI (8 processos)')

    # Configurações do gráfico
    plt.title('Tempo de Execução vs. Tamanho da Entrada (8 Threads/Processos)', fontsize=16, fontweight='bold')
    plt.xlabel('Tamanho da Entrada (N)', fontsize=12)
    plt.ylabel('Tempo Médio de Execução (s)', fontsize=12)
    
    plt.yscale('log')
    plt.xscale('log')
    
    plt.legend(fontsize=11)
    plt.xticks(media_serial['Tamanho'])
    plt.gca().get_xaxis().set_major_formatter(plt.FuncFormatter(lambda x, p: format(int(x), ',')))
    plt.grid(True, which="both", ls="--")

    # --- 4. Salvando o Gráfico ---
    caminho_saida = os.path.join(caminho_graficos, 'tempo_vs_tamanho_8_threads.png')
    plt.savefig(caminho_saida, dpi=300, bbox_inches='tight')
    
    print(f"\nGráfico salvo com sucesso em: '{caminho_saida}'")

if __name__ == '__main__':
    gerar_grafico_tempo_execucao()
