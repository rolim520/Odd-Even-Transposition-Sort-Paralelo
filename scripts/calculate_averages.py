import pandas as pd
import os

def process_csv(input_path, output_path, group_by_cols):
    """
    Reads a CSV, calculates the mean of numeric columns based on grouping,
    and saves the result to a new CSV.

    Args:
        input_path (str): Path to the source CSV file.
        output_path (str): Path to save the aggregated CSV file.
        group_by_cols (list): List of column names to group by.
    """
    if not os.path.exists(input_path):
        print(f"Arquivo de entrada não encontrado: {input_path}")
        return

    try:
        df = pd.read_csv(input_path)
        
        # Agrupa pelos identificadores e calcula a média para todas as outras colunas numéricas
        # O as_index=False mantém as colunas de agrupamento como colunas normais
        df_avg = df.groupby(group_by_cols, as_index=False).mean()
        
        df_avg.to_csv(output_path, index=False, float_format='%.6f')
        print(f"Arquivo de médias salvo com sucesso em: '{output_path}'")

    except Exception as e:
        print(f"Ocorreu um erro ao processar {input_path}: {e}")

def main():
    """
    Função principal para orquestrar o processamento dos arquivos CSV.
    """
    print("Iniciando o cálculo das médias dos arquivos CSV...")

    # Define o diretório base para os dados
    base_data_dir = os.path.join(os.path.dirname(__file__), '..', 'data')

    # --- Dicionário de Configuração ---
    # Chave: nome do arquivo de entrada
    # Valor: tupla com (nome do arquivo de saída, lista de colunas para agrupar)
    files_to_process = {
        'serial.csv': ('serial_average.csv', ['Tamanho']),
        'openmp.csv': ('openmp_average.csv', ['Tamanho', 'Threads', 'Schedule']),
        'mpi.csv': ('mpi_average.csv', ['Tamanho', 'Processos'])
    }

    for input_name, (output_name, group_cols) in files_to_process.items():
        input_file_path = os.path.join(base_data_dir, input_name)
        output_file_path = os.path.join(base_data_dir, output_name)
        
        print(f"\nProcessando '{input_name}'...")
        process_csv(input_file_path, output_file_path, group_cols)

    print("\nProcessamento de todos os arquivos concluído.")

if __name__ == '__main__':
    main()
