#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hash_table.h"
#include "indexer.h"

/*
  exibir_cabecalho
  Exibe o banner de boas-vindas do programa.
  */
static void exibir_cabecalho(void) {
    printf("==============================================\n");
    printf("   INDEXADOR DE ALTA PERFORMANCE\n");
    printf("   Tabelas de Dispersão — LPI 2026.1\n");
    printf("   IFCE Campus Aracati\n");
    printf("==============================================\n\n");
}


int main(int argc, char *argv[]) {
    exibir_cabecalho();

   
    if (argc != 2) {
        fprintf(stderr,
                "Uso: %s <caminho_da_pasta>\n"
                "Exemplo: %s ./textos\n",
                argv[0], argv[0]);
        return EXIT_FAILURE;
    }

    const char *caminho_pasta = argv[1];

    // Cria a tabela hash 
    TabelaHash *tabela = hash_criar();
    if (!tabela) {
        fprintf(stderr, "Erro fatal: não foi possível criar a tabela hash.\n");
        return EXIT_FAILURE;
    }

    // Fase 1: Indexação 
    int arquivos = indexar_pasta(tabela, caminho_pasta);
    if (arquivos <= 0) {
        printf("Nenhum arquivo foi indexado. Encerrando.\n");
        hash_destruir(tabela);
        return EXIT_SUCCESS;
    }

    // Fase 2: Loop de busca interativa 
    char termo[MAX_TERMO + 1]; // +1 para '\n' ou '\0' 

    printf("Digite \"sair\" a qualquer momento para encerrar.\n\n");

    while (1) {
        printf("Pesquise o termo: ");
        fflush(stdout);

        // Lê o termo digitado pelo usuário 
        if (!fgets(termo, sizeof(termo), stdin)) {
            break; 
        }

        // Remove o '\n' que fgets inclui no final
        termo[strcspn(termo, "\n")] = '\0';

        // Verifica se o usuário quer sair
        if (strcmp(termo, "sair") == 0) {
            printf("Encerrando o indexador. Até logo!\n");
            break;
        }

        // Ignora entradas vazias
        if (strlen(termo) == 0) {
            continue;
        }

        // Busca e exibe os resultados
        buscar_e_exibir(tabela, termo, caminho_pasta);

        printf("\n");
    }

    // Libera toda a memória antes de encerrar
    hash_destruir(tabela);

    return EXIT_SUCCESS;
}
