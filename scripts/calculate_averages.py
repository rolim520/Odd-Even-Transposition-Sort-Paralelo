import pandas as pd
import os

def process_csv(input_path, output_path, group_by_cols):
    """
    Lê um arquivo CSV, calcula a média das colunas numéricas com base em um agrupamento
    e salva o resultado em um novo arquivo CSV.

    Esta função é usada para consolidar os resultados das múltiplas execuções
    de cada experimento em um único valor médio, facilitando a análise.

    Args:
        input_path (str): Caminho para o arquivo CSV de origem (dados brutos).
        output_path (str): Caminho para salvar o arquivo CSV agregado (com as médias).
        group_by_cols (list): Lista de nomes de colunas para usar no agrupamento.
                              Por exemplo, ['Tamanho', 'Threads'].
    """
    # Verifica se o arquivo de entrada existe antes de prosseguir.
    if not os.path.exists(input_path):
        print(f"Arquivo de entrada não encontrado: {input_path}")
        return

    try:
        # Lê o arquivo CSV para um DataFrame do pandas.
        df = pd.read_csv(input_path)
        
        # Agrupa o DataFrame pelas colunas especificadas e calcula a média para todas as 
        # outras colunas numéricas. O 'as_index=False' mantém as colunas de agrupamento
        # como colunas normais no DataFrame resultante.
        df_avg = df.groupby(group_by_cols, as_index=False).mean()
        
        # Salva o DataFrame com as médias em um novo arquivo CSV.
        # 'index=False' evita que o pandas escreva o índice do DataFrame no arquivo.
        # 'float_format' garante uma formatação consistente para os números de ponto flutuante.
        df_avg.to_csv(output_path, index=False, float_format='%.6f')
        print(f"Arquivo de médias salvo com sucesso em: '{output_path}'")

    except Exception as e:
        print(f"Ocorreu um erro ao processar {input_path}: {e}")

def main():
    """
    Função principal que orquestra o processamento de todos os arquivos CSV de resultados.
    """
    print("Iniciando o cálculo das médias dos arquivos CSV...")

    # Define o diretório base onde os arquivos de dados estão localizados.
    # O caminho é construído de forma relativa ao local do script.
    base_data_dir = os.path.join(os.path.dirname(__file__), '..', 'data')

    # --- Dicionário de Configuração ---
    # Define os arquivos a serem processados e como eles devem ser agrupados.
    # Chave: nome do arquivo de entrada (dados brutos).
    # Valor: tupla contendo (nome do arquivo de saída, lista de colunas para agrupar).
    files_to_process = {
        'serial.csv': ('serial_average.csv', ['Tamanho']),
        'openmp.csv': ('openmp_average.csv', ['Tamanho', 'Threads', 'Schedule']),
        'mpi.csv': ('mpi_average.csv', ['Tamanho', 'Processos'])
    }

    # Itera sobre o dicionário de configuração para processar cada arquivo.
    for input_name, (output_name, group_cols) in files_to_process.items():
        input_file_path = os.path.join(base_data_dir, input_name)
        output_file_path = os.path.join(base_data_dir, output_name)
        
        print(f"\nProcessando '{input_name}'...")
        process_csv(input_file_path, output_file_path, group_cols)

    print("\nProcessamento de todos os arquivos concluído.")

# Ponto de entrada do script: executa a função main se o arquivo for chamado diretamente.
if __name__ == '__main__':
    main()