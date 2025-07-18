import pandas as pd
import matplotlib.pyplot as plt
import os
import seaborn as sns

def gerar_grafico_tempo_execucao():
    """
    Gera e salva um gráfico de linha comparando o tempo de execução
    das implementações Serial, OpenMP (com diferentes schedules) e MPI.
    """
    print("Iniciando a geração do gráfico de tempo de execução...")

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
    
    print(f"\nGráfico de tempo salvo com sucesso em: '{caminho_saida}'")


def gerar_grafico_eficiencia_vs_processos():
    """
    Gera e salva um gráfico de linha comparando a eficiência
    das implementações OpenMP (diferentes schedules) e MPI
    para um tamanho de entrada fixo (N=100.000).
    """
    print("\nIniciando a geração do gráfico de eficiência...")

    # --- 1. Definição dos Caminhos e Constantes ---
    caminho_base_dados = 'data'
    caminho_graficos = 'graficos'
    TAMANHO_ENTRADA = 100000

    if not os.path.exists(caminho_graficos):
        os.makedirs(caminho_graficos)

    arquivo_openmp = os.path.join(caminho_base_dados, 'openmp.csv')
    arquivo_mpi = os.path.join(caminho_base_dados, 'mpi.csv')

    # --- 2. Carregamento e Processamento dos Dados ---
    try:
        df_openmp = pd.read_csv(arquivo_openmp)
        df_mpi = pd.read_csv(arquivo_mpi)
        print("Arquivos CSV para o gráfico de eficiência carregados com sucesso.")
    except FileNotFoundError as e:
        print(f"Erro: Arquivo não encontrado - {e}")
        print("Verifique se os arquivos CSV estão no diretório 'data/'.")
        return

    # --- Processamento OpenMP ---
    df_openmp_filtrado = df_openmp[df_openmp['Tamanho'] == TAMANHO_ENTRADA].copy()
    media_openmp = df_openmp_filtrado.groupby(['Threads', 'Schedule'])['Eficiencia'].mean().reset_index()
    
    media_openmp_static = media_openmp[media_openmp['Schedule'] == 'static']
    media_openmp_dynamic = media_openmp[media_openmp['Schedule'] == 'dynamic']
    media_openmp_guided = media_openmp[media_openmp['Schedule'] == 'guided']
    
    print(f"\nDados de Eficiência OpenMP (Média, N={TAMANHO_ENTRADA:,}):")
    print(media_openmp)

    # --- Processamento MPI ---
    df_mpi_filtrado = df_mpi[df_mpi['Tamanho'] == TAMANHO_ENTRADA].copy()
    media_mpi = df_mpi_filtrado.groupby('Processos')['Eficiencia'].mean().reset_index()
    
    print(f"\nDados de Eficiência MPI (Média, N={TAMANHO_ENTRADA:,}):")
    print(media_mpi)

    # --- 3. Geração do Gráfico ---
    plt.style.use('seaborn-v0_8-whitegrid')
    plt.figure(figsize=(12, 7))

    # Plota os dados
    plt.plot(media_openmp_static['Threads'], media_openmp_static['Eficiencia'], marker='s', linestyle='--', label='OpenMP (static)')
    plt.plot(media_openmp_dynamic['Threads'], media_openmp_dynamic['Eficiencia'], marker='x', linestyle=':', label='OpenMP (dynamic)')
    plt.plot(media_openmp_guided['Threads'], media_openmp_guided['Eficiencia'], marker='d', linestyle='-.', label='OpenMP (guided)')
    
    plt.plot(media_mpi['Processos'], media_mpi['Eficiencia'], marker='^', linestyle='-', label='MPI')

    # Configurações do gráfico
    plt.title(f'Eficiência vs. Número de Threads/Processos (N={TAMANHO_ENTRADA:,})', fontsize=16, fontweight='bold')
    plt.xlabel('Número de Threads/Processos', fontsize=12)
    plt.ylabel('Eficiência Média', fontsize=12)
    
    # Definir os ticks do eixo x para serem os valores exatos de threads/processos
    process_counts = sorted(media_mpi['Processos'].unique())
    plt.xticks(process_counts)

    plt.legend(fontsize=11)
    plt.grid(True, which="both", ls="--")
    plt.ylim(bottom=0) # A eficiência não deve ser negativa

    # --- 4. Salvando o Gráfico ---
    caminho_saida = os.path.join(caminho_graficos, f'eficiencia_vs_processos_{TAMANHO_ENTRADA}.png')
    plt.savefig(caminho_saida, dpi=300, bbox_inches='tight')
    
    print(f"\nGráfico de eficiência salvo com sucesso em: '{caminho_saida}'")


def gerar_grafico_speedup_vs_processos():
    """
    Gera e salva um gráfico de linha comparando o speedup
    das implementações OpenMP (diferentes schedules) e MPI
    para um tamanho de entrada fixo (N=100.000).
    """
    print("\nIniciando a geração do gráfico de speedup...")

    # --- 1. Definição dos Caminhos e Constantes ---
    caminho_base_dados = 'data'
    caminho_graficos = 'graficos'
    TAMANHO_ENTRADA = 100000

    if not os.path.exists(caminho_graficos):
        os.makedirs(caminho_graficos)

    arquivo_openmp = os.path.join(caminho_base_dados, 'openmp.csv')
    arquivo_mpi = os.path.join(caminho_base_dados, 'mpi.csv')

    # --- 2. Carregamento e Processamento dos Dados ---
    try:
        df_openmp = pd.read_csv(arquivo_openmp)
        df_mpi = pd.read_csv(arquivo_mpi)
        print("Arquivos CSV para o gráfico de speedup carregados com sucesso.")
    except FileNotFoundError as e:
        print(f"Erro: Arquivo não encontrado - {e}")
        print("Verifique se os arquivos CSV estão no diretório 'data/'.")
        return

    # --- Processamento OpenMP ---
    df_openmp_filtrado = df_openmp[df_openmp['Tamanho'] == TAMANHO_ENTRADA].copy()
    media_openmp = df_openmp_filtrado.groupby(['Threads', 'Schedule'])['Speedup'].mean().reset_index()
    
    media_openmp_static = media_openmp[media_openmp['Schedule'] == 'static']
    media_openmp_dynamic = media_openmp[media_openmp['Schedule'] == 'dynamic']
    media_openmp_guided = media_openmp[media_openmp['Schedule'] == 'guided']
    
    print(f"\nDados de Speedup OpenMP (Média, N={TAMANHO_ENTRADA:,}):")
    print(media_openmp)

    # --- Processamento MPI ---
    df_mpi_filtrado = df_mpi[df_mpi['Tamanho'] == TAMANHO_ENTRADA].copy()
    media_mpi = df_mpi_filtrado.groupby('Processos')['Speedup'].mean().reset_index()
    
    print(f"\nDados de Speedup MPI (Média, N={TAMANHO_ENTRADA:,}):")
    print(media_mpi)

    # --- 3. Geração do Gráfico ---
    plt.style.use('seaborn-v0_8-whitegrid')
    plt.figure(figsize=(12, 7))

    # Plota os dados
    plt.plot(media_openmp_static['Threads'], media_openmp_static['Speedup'], marker='s', linestyle='--', label='OpenMP (static)')
    plt.plot(media_openmp_dynamic['Threads'], media_openmp_dynamic['Speedup'], marker='x', linestyle=':', label='OpenMP (dynamic)')
    plt.plot(media_openmp_guided['Threads'], media_openmp_guided['Speedup'], marker='d', linestyle='-.', label='OpenMP (guided)')
    
    plt.plot(media_mpi['Processos'], media_mpi['Speedup'], marker='^', linestyle='-', label='MPI')

    # Configurações do gráfico
    plt.title(f'Speedup vs. Número de Threads/Processos (N={TAMANHO_ENTRADA:,})', fontsize=16, fontweight='bold')
    plt.xlabel('Número de Threads/Processos', fontsize=12)
    plt.ylabel('Speedup Médio', fontsize=12)
    
    # Definir os ticks do eixo x para serem os valores exatos de threads/processos
    process_counts = sorted(media_mpi['Processos'].unique())
    plt.xticks(process_counts)

    plt.legend(fontsize=11)
    plt.grid(True, which="both", ls="--")
    plt.ylim(bottom=0) # Speedup não deve ser negativo

    # --- 4. Salvando o Gráfico ---
    caminho_saida = os.path.join(caminho_graficos, f'speedup_vs_processos_{TAMANHO_ENTRADA}.png')
    plt.savefig(caminho_saida, dpi=300, bbox_inches='tight')
    
    print(f"\nGráfico de speedup salvo com sucesso em: '{caminho_saida}'")


if __name__ == '__main__':
    gerar_grafico_tempo_execucao()
    gerar_grafico_eficiencia_vs_processos()
    gerar_grafico_speedup_vs_processos()
