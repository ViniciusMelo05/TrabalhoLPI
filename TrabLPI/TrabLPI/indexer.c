#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>   // opendir, readdir, closedir 
#include <termios.h>  // leitura de tecla sem pressionar Enter 
#include <unistd.h>

#include "indexer.h"
#include "hash_table.h"

/* 
  normalizar_termo
  Converte todos os caracteres para minúsculo e remove
  pontuação e caracteres não-alfanuméricos das extremidades.
  */
void normalizar_termo(char *termo) {
    if (!termo) return;

    // Converte para minúsculo 
    for (int i = 0; termo[i]; i++) {
        termo[i] = (char)tolower((unsigned char)termo[i]);
    }

    // Remove pontuação do início 
    int inicio = 0;
    while (termo[inicio] && !isalnum((unsigned char)termo[inicio])) {
        inicio++;
    }
    if (inicio > 0) {
        memmove(termo, termo + inicio, strlen(termo) - inicio + 1);
    }

    // Remove pontuação do fim
    int fim = (int)strlen(termo) - 1;
    while (fim >= 0 && !isalnum((unsigned char)termo[fim])) {
        termo[fim--] = '\0';
    }
}

/* 
  aguardar_navegacao
  Lê uma tecla sem eco e sem precisar pressionar Enter.
  Retorna 1 para Enter (avanço) ou 0 para Esc (parar).
  */
int aguardar_navegacao(void) {
    printf("\n  Enter para prosseguir ou Esc para parar.\n");
    fflush(stdout);

    // Configura terminal para leitura byte-a-byte sem eco
    struct termios modo_original, modo_raw;
    tcgetattr(STDIN_FILENO, &modo_original);
    modo_raw = modo_original;
    modo_raw.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &modo_raw);

    int tecla = getchar();

    // Restaura o terminal ao modo original 
    tcsetattr(STDIN_FILENO, TCSANOW, &modo_original);

    if (tecla == 27) {  /* Código ASCII do Esc */
        return 0;
    }
    return 1; /* Qualquer outra tecla (incluindo Enter = 10 ou 13) continua */
}

/* 
  exibir_contexto
  Posiciona o cursor no offset com fseek e exibe até
  CONTEXTO_TAMANHO caracteres ao redor do termo encontrado.
  */
void exibir_contexto(const char *caminho_arquivo,
                     const char *nome_arquivo, long offset,
                     const char *termo) {
    (void)termo; // Parâmetro reservado para uso futuro (destacar o termo no contexto)
    FILE *fp = fopen(caminho_arquivo, "rb"); // Modo binário para fseek preciso
    if (!fp) {
        fprintf(stderr, "  Aviso: não foi possível abrir '%s'.\n", caminho_arquivo);
        return;
    }

    // Calcula o início do trecho de contexto (recua CONTEXTO_TAMANHO/2 bytes)
    long inicio_contexto = offset - (CONTEXTO_TAMANHO / 2);
    if (inicio_contexto < 0) inicio_contexto = 0;

    fseek(fp, inicio_contexto, SEEK_SET);

    // Lê os bytes do contexto
    char buffer[CONTEXTO_TAMANHO * 2 + 1];
    int lidos = 0;
    int c;
    while (lidos < CONTEXTO_TAMANHO && (c = fgetc(fp)) != EOF) {
        // Substitui quebras de linha por espaço para exibição em linha única
        buffer[lidos++] = (c == '\n' || c == '\r') ? ' ' : (char)c;
    }
    buffer[lidos] = '\0';

    // Exibe no formato especificado pelo trabalho
    printf("  %s – \"...%s...\"\n", nome_arquivo, buffer);

    fclose(fp);
}

/*
  indexar_arquivo
  Percorre o arquivo palavra por palavra usando fscanf.
  Para cada palavra:
   1. Registra o offset ANTES de ler a palavra.
   2. Normaliza o termo.
   3. Se tiver >= MIN_TAMANHO_TERMO chars, insere na tabela.
    */
int indexar_arquivo(TabelaHash *tabela, const char *caminho_arquivo,
                    const char *nome_arquivo) {
    FILE *fp = fopen(caminho_arquivo, "rb");
    if (!fp) {
        fprintf(stderr, "  Aviso: não foi possível abrir '%s' para indexação.\n",
                caminho_arquivo);
        return 0;
    }

    char palavra[MAX_TERMO];
    long offset_palavra;

    while (1) {
        // Pula espaços e registra a posição antes da palavra 
        int c;
        do {
            offset_palavra = ftell(fp);
            c = fgetc(fp);
        } while (c != EOF && isspace((unsigned char)c));

        if (c == EOF) break;

        // Volta um byte para que fscanf leia a palavra completa
        fseek(fp, -1L, SEEK_CUR);
        offset_palavra = ftell(fp);

        // Lê a próxima palavra (até MAX_TERMO - 1 caracteres)
        if (fscanf(fp, "%50s", palavra) != 1) break;

        // Normaliza: minúsculas + remove pontuação das bordas
        normalizar_termo(palavra);

        // Ignora termos muito curtos (não entram na tabela)
        if ((int)strlen(palavra) < MIN_TAMANHO_TERMO) continue;

        // Insere o termo com seu offset na tabela hash
        hash_inserir(tabela, palavra, nome_arquivo, offset_palavra);
    }

    fclose(fp);
    return 1;
}

/*
  indexar_pasta
  Abre a pasta, filtra arquivos .txt e chama indexar_arquivo
  para cada um deles.
 */
int indexar_pasta(TabelaHash *tabela, const char *caminho_pasta) {
    DIR *pasta = opendir(caminho_pasta);
    if (!pasta) {
        fprintf(stderr, "Erro: não foi possível abrir a pasta '%s'.\n",
                caminho_pasta);
        return -1;
    }

    int arquivos_indexados = 0;
    struct dirent *entrada;

    printf("Indexando arquivos em '%s'...\n", caminho_pasta);

    while ((entrada = readdir(pasta)) != NULL) {
        // Verifica se a entrada é um arquivo .txt (extensão case-insensitive)
        char *nome = entrada->d_name;
        int tamanho_nome = (int)strlen(nome);

        if (tamanho_nome < 5) continue; /* Muito curto para ter ".txt" */

        // Compara os últimos 4 caracteres com ".txt"
        char extensao[5];
        strncpy(extensao, nome + tamanho_nome - 4, 4);
        extensao[4] = '\0';
        for (int i = 0; extensao[i]; i++) {
            extensao[i] = (char)tolower((unsigned char)extensao[i]);
        }
        if (strcmp(extensao, ".txt") != 0) continue;

        // Monta o caminho completo do arquivo
        char caminho_arquivo[512];
        snprintf(caminho_arquivo, sizeof(caminho_arquivo),
                 "%s/%s", caminho_pasta, nome);

        printf("  Indexando: %s\n", nome);
        if (indexar_arquivo(tabela, caminho_arquivo, nome)) {
            arquivos_indexados++;
        }
    }

    closedir(pasta);

    if (arquivos_indexados == 0) {
        printf("  Nenhum arquivo .txt encontrado na pasta.\n");
    } else {
        printf("Indexação concluída. %d arquivo(s) processado(s).\n\n",
               arquivos_indexados);
    }

    return arquivos_indexados;
}

/*
  buscar_termo_pequeno (função auxiliar interna)
  Para termos com menos de MIN_TAMANHO_TERMO caracteres,
  percorre todos os .txt linearmente e exibe as ocorrências.
  */
static void buscar_termo_pequeno(const char *termo,
                                 const char *caminho_pasta) {
    DIR *pasta = opendir(caminho_pasta);
    if (!pasta) return;

    printf("\nBusca linear (termo pequeno): \"%s\"\n\n", termo);

    struct dirent *entrada;
    int total = 0;
    int continuar = 1;

    while ((entrada = readdir(pasta)) != NULL && continuar) {
        char *nome = entrada->d_name;
        int tamanho_nome = (int)strlen(nome);
        if (tamanho_nome < 5) continue;

        char extensao[5];
        strncpy(extensao, nome + tamanho_nome - 4, 4);
        extensao[4] = '\0';
        for (int i = 0; extensao[i]; i++) {
            extensao[i] = (char)tolower((unsigned char)extensao[i]);
        }
        if (strcmp(extensao, ".txt") != 0) continue;

        char caminho_arquivo[512];
        snprintf(caminho_arquivo, sizeof(caminho_arquivo),
                 "%s/%s", caminho_pasta, nome);

        FILE *fp = fopen(caminho_arquivo, "rb");
        if (!fp) continue;

        char palavra[MAX_TERMO];
        long offset;

        while (continuar) {
            int c;
            do {
                offset = ftell(fp);
                c = fgetc(fp);
            } while (c != EOF && isspace((unsigned char)c));

            if (c == EOF) break;

            fseek(fp, -1L, SEEK_CUR);
            offset = ftell(fp);
            if (fscanf(fp, "%50s", palavra) != 1) break;

            char termo_normalizado[MAX_TERMO];
            strncpy(termo_normalizado, palavra, MAX_TERMO - 1);
            termo_normalizado[MAX_TERMO - 1] = '\0';
            normalizar_termo(termo_normalizado);

            if (strcmp(termo_normalizado, termo) == 0) {
                exibir_contexto(caminho_arquivo, nome, offset, termo);
                total++;

                /* Pausa a cada OCORRENCIAS_POR_PAGINA resultados */
                if (total % OCORRENCIAS_POR_PAGINA == 0) {
                    continuar = aguardar_navegacao();
                }
            }
        }

        fclose(fp);
    }

    closedir(pasta);

    if (total == 0) {
        printf("  Nenhuma ocorrência encontrada para \"%s\".\n", termo);
    } else {
        printf("\n  Total: %d ocorrência(s) encontrada(s).\n", total);
    }
}

/*
  buscar_e_exibir
  Ponto principal de busca:
  - Termos >= MIN_TAMANHO_TERMO: busca instantânea na tabela hash.
  - Termos < MIN_TAMANHO_TERMO: busca linear nos arquivos.
  */
void buscar_e_exibir(TabelaHash *tabela, const char *termo,
                     const char *caminho_pasta) {
    // Cria cópia normalizada do termo para busca 
    char termo_norm[MAX_TERMO];
    strncpy(termo_norm, termo, MAX_TERMO - 1);
    termo_norm[MAX_TERMO - 1] = '\0';
    normalizar_termo(termo_norm);

    // Termo muito curto: busca linear sem usar a tabela hash 
    if ((int)strlen(termo_norm) < MIN_TAMANHO_TERMO) {
        buscar_termo_pequeno(termo_norm, caminho_pasta);
        return;
    }

    // Busca o termo na tabela hash (O(1) esperado) 
    RegistroIndice *registro = hash_buscar(tabela, termo_norm);

    if (!registro) {
        printf("\n  Nenhuma ocorrência encontrada para \"%s\".\n", termo_norm);
        return;
    }

    printf("\nResultados para \"%s\":\n\n", termo_norm);

    Ocorrencia *ocorrencia = registro->lista;
    int total = 0;
    int continuar = 1;

    while (ocorrencia != NULL && continuar) {
        // Monta o caminho completo para abrir o arquivo com fseek 
        char caminho_arquivo[512];
        snprintf(caminho_arquivo, sizeof(caminho_arquivo),
                 "%s/%s", caminho_pasta, ocorrencia->nome_arquivo);

        // Exibe o trecho de contexto usando acesso aleatório 
        exibir_contexto(caminho_arquivo, ocorrencia->nome_arquivo,
                        ocorrencia->offset, termo_norm);
        total++;

        // Pausa a cada OCORRENCIAS_POR_PAGINA resultados
        if (total % OCORRENCIAS_POR_PAGINA == 0 && ocorrencia->prox != NULL) {
            continuar = aguardar_navegacao();
        }

        ocorrencia = ocorrencia->prox;
    }

    printf("\n  Total: %d ocorrência(s) encontrada(s).\n", total);
}
