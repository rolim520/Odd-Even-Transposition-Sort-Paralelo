import pandas as pd
import matplotlib.pyplot as plt
import os
import seaborn as sns

def gerar_grafico_tempo_execucao():
    """
    Gera e salva um gráfico de linha comparando o tempo de execução
    das implementações Serial, OpenMP (com diferentes schedules) e MPI.
    O gráfico mostra o tempo de execução em função do tamanho da entrada,
    com as versões paralelas fixadas em 8 threads/processos.
    """
    print("Iniciando a geração do gráfico de tempo de execução...")

    # --- 1. Definição dos Caminhos e Criação do Diretório ---
    caminho_base_dados = os.path.join(os.path.dirname(__file__), '..', 'data')
    caminho_graficos = os.path.join(os.path.dirname(__file__), '..', 'graficos')
    
    # Cria o diretório de gráficos se ele não existir.
    if not os.path.exists(caminho_graficos):
        print(f"Criando diretório '{caminho_graficos}'...")
        os.makedirs(caminho_graficos)

    # Caminhos completos para os arquivos de dados (médias).
    arquivo_serial = os.path.join(caminho_base_dados, 'serial_average.csv')
    arquivo_openmp = os.path.join(caminho_base_dados, 'openmp_average.csv')
    arquivo_mpi = os.path.join(caminho_base_dados, 'mpi_average.csv')

    # --- 2. Carregamento e Processamento dos Dados ---
    try:
        media_serial = pd.read_csv(arquivo_serial)
        media_openmp_full = pd.read_csv(arquivo_openmp)
        media_mpi_full = pd.read_csv(arquivo_mpi)
        print("Arquivos CSV de médias carregados com sucesso.")
    except FileNotFoundError as e:
        print(f"Erro: Arquivo não encontrado - {e}")
        print("Verifique se os arquivos *_average.csv estão no diretório 'data/'.")
        return

    # --- Processamento Serial ---
    print("\nDados da execução Serial (Média):")
    print(media_serial)

    # --- Processamento OpenMP ---
    # Filtra os dados para manter apenas os resultados com 8 threads.
    media_openmp = media_openmp_full[media_openmp_full['Threads'] == 8].copy()
    # Separa os dados por tipo de schedule.
    media_openmp_static = media_openmp[media_openmp['Schedule'] == 'static']
    media_openmp_dynamic = media_openmp[media_openmp['Schedule'] == 'dynamic']
    media_openmp_guided = media_openmp[media_openmp['Schedule'] == 'guided']
    
    print("\nDados da execução OpenMP (Média, 8 Threads):")
    print(media_openmp)

    # --- Processamento MPI ---
    # Filtra os dados para manter apenas os resultados com 8 processos.
    media_mpi = media_mpi_full[media_mpi_full['Processos'] == 8].copy()
    # Renomeia a coluna de tempo para padronizar com os outros dataframes.
    media_mpi = media_mpi.rename(columns={'TempoTotal(max)': 'Tempo(s)'})
    
    print("\nDados da execução MPI (Média, 8 Processos):")
    print(media_mpi)

    # --- 3. Geração do Gráfico ---
    plt.style.use('seaborn-v0_8-whitegrid') # Define um estilo visual agradável.
    plt.figure(figsize=(15, 9)) # Define o tamanho da figura.

    # Plota os dados de cada implementação com marcadores e estilos de linha distintos.
    plt.plot(media_serial['Tamanho'], media_serial['Tempo(s)'], marker='o', linestyle='-', label='Serial', markersize=8, linewidth=2.5)
    
    plt.plot(media_openmp_static['Tamanho'], media_openmp_static['Tempo(s)'], marker='s', linestyle='--', label='OpenMP (static, 8 threads)', markersize=8, linewidth=2.5)
    plt.plot(media_openmp_dynamic['Tamanho'], media_openmp_dynamic['Tempo(s)'], marker='x', linestyle=':', label='OpenMP (dynamic, 8 threads)', markersize=8, linewidth=2.5)
    plt.plot(media_openmp_guided['Tamanho'], media_openmp_guided['Tempo(s)'], marker='d', linestyle='-.', label='OpenMP (guided, 8 threads)', markersize=8, linewidth=2.5)
    
    plt.plot(media_mpi['Tamanho'], media_mpi['Tempo(s)'], marker='^', linestyle='-', label='MPI (8 processos)', markersize=8, linewidth=2.5)

    # Configurações do gráfico (títulos, eixos, legendas).
    plt.title('Tempo de Execução vs. Tamanho da Entrada (8 Threads/Processos)', fontsize=20, fontweight='bold')
    plt.xlabel('Tamanho da Entrada (N)', fontsize=14)
    plt.ylabel('Tempo Médio de Execução (s)', fontsize=14)
    
    # Usa escala logarítmica para ambos os eixos para melhor visualização da tendência.
    plt.yscale('log')
    plt.xscale('log')
    
    plt.legend(fontsize=12)
    # Formata os ticks do eixo X para serem mais legíveis.
    plt.xticks(media_serial['Tamanho'], fontsize=12)
    plt.yticks(fontsize=12)
    plt.gca().get_xaxis().set_major_formatter(plt.FuncFormatter(lambda x, p: format(int(x), ',')))
    plt.grid(True, which="both", ls="--") # Adiciona grade para facilitar a leitura.

    # --- 4. Salvando o Gráfico ---
    caminho_saida = os.path.join(caminho_graficos, 'tempo_vs_tamanho_8_threads.png')
    plt.savefig(caminho_saida, dpi=300, bbox_inches='tight') # Salva em alta resolução.
    
    print(f"\nGráfico de tempo salvo com sucesso em: '{caminho_saida}'")


def gerar_grafico_eficiencia_vs_processos():
    """
    Gera um gráfico comparando a eficiência das implementações OpenMP e MPI
    para um tamanho de entrada fixo (N=100.000), variando o número de threads/processos.
    """
    print("\nIniciando a geração do gráfico de eficiência...")

    # --- 1. Definição dos Caminhos e Constantes ---
    caminho_base_dados = os.path.join(os.path.dirname(__file__), '..', 'data')
    caminho_graficos = os.path.join(os.path.dirname(__file__), '..', 'graficos')
    TAMANHO_ENTRADA = 100000 # Fixo para esta análise.

    if not os.path.exists(caminho_graficos):
        os.makedirs(caminho_graficos)

    arquivo_openmp = os.path.join(caminho_base_dados, 'openmp_average.csv')
    arquivo_mpi = os.path.join(caminho_base_dados, 'mpi_average.csv')

    # --- 2. Carregamento e Processamento dos Dados ---
    try:
        df_openmp = pd.read_csv(arquivo_openmp)
        df_mpi = pd.read_csv(arquivo_mpi)
        print("Arquivos CSV de médias para o gráfico de eficiência carregados com sucesso.")
    except FileNotFoundError as e:
        print(f"Erro: Arquivo não encontrado - {e}")
        return

    # --- Processamento OpenMP ---
    # Filtra os dados para o tamanho de entrada desejado.
    media_openmp = df_openmp[df_openmp['Tamanho'] == TAMANHO_ENTRADA].copy()
    media_openmp_static = media_openmp[media_openmp['Schedule'] == 'static']
    media_openmp_dynamic = media_openmp[media_openmp['Schedule'] == 'dynamic']
    media_openmp_guided = media_openmp[media_openmp['Schedule'] == 'guided']
    
    print(f"\nDados de Eficiência OpenMP (Média, N={TAMANHO_ENTRADA:,}):")
    print(media_openmp)

    # --- Processamento MPI ---
    media_mpi = df_mpi[df_mpi['Tamanho'] == TAMANHO_ENTRADA].copy()
    
    print(f"\nDados de Eficiência MPI (Média, N={TAMANHO_ENTRADA:,}):")
    print(media_mpi)

    # --- 3. Geração do Gráfico ---
    plt.style.use('seaborn-v0_8-whitegrid')
    plt.figure(figsize=(13, 8))

    # Plota a eficiência em função do número de threads/processos.
    plt.plot(media_openmp_static['Threads'], media_openmp_static['Eficiencia'], marker='s', linestyle='--', label='OpenMP (static)', markersize=8, linewidth=2.5)
    plt.plot(media_openmp_dynamic['Threads'], media_openmp_dynamic['Eficiencia'], marker='x', linestyle=':', label='OpenMP (dynamic)', markersize=8, linewidth=2.5)
    plt.plot(media_openmp_guided['Threads'], media_openmp_guided['Eficiencia'], marker='d', linestyle='-.', label='OpenMP (guided)', markersize=8, linewidth=2.5)
    
    plt.plot(media_mpi['Processos'], media_mpi['Eficiencia'], marker='^', linestyle='-', label='MPI', markersize=8, linewidth=2.5)

    # Configurações do gráfico.
    plt.title(f'Eficiência vs. Número de Threads/Processos (N={TAMANHO_ENTRADA:,})', fontsize=20, fontweight='bold')
    plt.xlabel('Número de Threads/Processos', fontsize=14)
    plt.ylabel('Eficiência Média', fontsize=14)
    
    process_counts = sorted(media_mpi['Processos'].unique())
    plt.xticks(process_counts, fontsize=12)
    plt.yticks(fontsize=12)

    plt.legend(fontsize=12)
    plt.grid(True, which="both", ls="--")
    plt.ylim(bottom=0) # Garante que o eixo Y comece em 0.

    # --- 4. Salvando o Gráfico ---
    caminho_saida = os.path.join(caminho_graficos, f'eficiencia_vs_processos_{TAMANHO_ENTRADA}.png')
    plt.savefig(caminho_saida, dpi=300, bbox_inches='tight')
    
    print(f"\nGráfico de eficiência salvo com sucesso em: '{caminho_saida}'")


def gerar_grafico_speedup_vs_processos():
    """
    Gera um gráfico comparando o speedup das implementações OpenMP e MPI
    para um tamanho de entrada fixo (N=100.000), variando o número de threads/processos.
    """
    print("\nIniciando a geração do gráfico de speedup...")

    # --- 1. Definição dos Caminhos e Constantes ---
    caminho_base_dados = os.path.join(os.path.dirname(__file__), '..', 'data')
    caminho_graficos = os.path.join(os.path.dirname(__file__), '..', 'graficos')
    TAMANHO_ENTRADA = 100000

    if not os.path.exists(caminho_graficos):
        os.makedirs(caminho_graficos)

    arquivo_openmp = os.path.join(caminho_base_dados, 'openmp_average.csv')
    arquivo_mpi = os.path.join(caminho_base_dados, 'mpi_average.csv')

    # --- 2. Carregamento e Processamento dos Dados ---
    try:
        df_openmp = pd.read_csv(arquivo_openmp)
        df_mpi = pd.read_csv(arquivo_mpi)
        print("Arquivos CSV de médias para o gráfico de speedup carregados com sucesso.")
    except FileNotFoundError as e:
        print(f"Erro: Arquivo não encontrado - {e}")
        return

    # --- Processamento OpenMP ---
    media_openmp = df_openmp[df_openmp['Tamanho'] == TAMANHO_ENTRADA].copy()
    media_openmp_static = media_openmp[media_openmp['Schedule'] == 'static']
    media_openmp_dynamic = media_openmp[media_openmp['Schedule'] == 'dynamic']
    media_openmp_guided = media_openmp[media_openmp['Schedule'] == 'guided']
    
    print(f"\nDados de Speedup OpenMP (Média, N={TAMANHO_ENTRADA:,}):")
    print(media_openmp)

    # --- Processamento MPI ---
    media_mpi = df_mpi[df_mpi['Tamanho'] == TAMANHO_ENTRADA].copy()
    
    print(f"\nDados de Speedup MPI (Média, N={TAMANHO_ENTRADA:,}):")
    print(media_mpi)

    # --- 3. Geração do Gráfico ---
    plt.style.use('seaborn-v0_8-whitegrid')
    plt.figure(figsize=(13, 8))

    # Plota o speedup em função do número de threads/processos.
    plt.plot(media_openmp_static['Threads'], media_openmp_static['Speedup'], marker='s', linestyle='--', label='OpenMP (static)', markersize=8, linewidth=2.5)
    plt.plot(media_openmp_dynamic['Threads'], media_openmp_dynamic['Speedup'], marker='x', linestyle=':', label='OpenMP (dynamic)', markersize=8, linewidth=2.5)
    plt.plot(media_openmp_guided['Threads'], media_openmp_guided['Speedup'], marker='d', linestyle='-.', label='OpenMP (guided)', markersize=8, linewidth=2.5)
    
    plt.plot(media_mpi['Processos'], media_mpi['Speedup'], marker='^', linestyle='-', label='MPI', markersize=8, linewidth=2.5)

    # Configurações do gráfico.
    plt.title(f'Speedup vs. Número de Threads/Processos (N={TAMANHO_ENTRADA:,})', fontsize=20, fontweight='bold')
    plt.xlabel('Número de Threads/Processos', fontsize=14)
    plt.ylabel('Speedup Médio', fontsize=14)
    
    process_counts = sorted(media_mpi['Processos'].unique())
    plt.xticks(process_counts, fontsize=12)
    plt.yticks(fontsize=12)

    plt.legend(fontsize=12)
    plt.grid(True, which="both", ls="--")
    plt.ylim(bottom=0)

    # --- 4. Salvando o Gráfico ---
    caminho_saida = os.path.join(caminho_graficos, f'speedup_vs_processos_{TAMANHO_ENTRADA}.png')
    plt.savefig(caminho_saida, dpi=300, bbox_inches='tight')
    
    print(f"\nGráfico de speedup salvo com sucesso em: '{caminho_saida}'")


if __name__ == '__main__':
    gerar_grafico_tempo_execucao()
    gerar_grafico_eficiencia_vs_processos()
    gerar_grafico_speedup_vs_processos()