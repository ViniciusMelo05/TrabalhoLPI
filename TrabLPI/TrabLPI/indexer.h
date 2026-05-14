#ifndef INDEXER_H
#define INDEXER_H

// ============================================================
// indexer.h
// Definição das funções responsáveis por:
//  - Varrer arquivos .txt e popular a tabela hash (indexação).
//  - Buscar termos e exibir os trechos com contexto (recuperação).
// ============================================================

#include "hash_table.h"

// Número de caracteres de contexto exibidos ao redor do termo
#define CONTEXTO_TAMANHO 50

// Quantidade de ocorrências exibidas por página antes de pausar
#define OCORRENCIAS_POR_PAGINA 10


// indexar_pasta: abre a pasta indicada por 'caminho_pasta', percorre todos os
// arquivos .txt encontrados e indexa cada palavra deles na tabela.
// Retorna o número de arquivos indexados, ou -1 em caso de erro.
int indexar_pasta(TabelaHash *tabela, const char *caminho_pasta);

// indexar_arquivo: lê o arquivo palavra por palavra, armazenando o offset de
// cada termo na tabela. Termos com menos de MIN_TAMANHO_TERMO caracteres
// são ignorados. Retorna 1 em sucesso, 0 em falha.
int indexar_arquivo(TabelaHash *tabela, const char *caminho_arquivo,
                    const char *nome_arquivo);

// normalizar_termo: converte o termo para minúsculas e remove pontuação das
// bordas, garantindo comparações consistentes na tabela hash.
void normalizar_termo(char *termo);

// buscar_e_exibir: busca 'termo' na tabela hash (se >= MIN_TAMANHO_TERMO) ou
// faz varredura linear nos arquivos (se termo pequeno). Exibe os trechos de
// contexto páginados; Enter avança, Esc encerra.
void buscar_e_exibir(TabelaHash *tabela, const char *termo,
                     const char *caminho_pasta);

// exibir_contexto: abre 'nome_arquivo', posiciona-se em 'offset' com fseek e
// imprime até CONTEXTO_TAMANHO caracteres ao redor do termo.
void exibir_contexto(const char *caminho_arquivo,
                     const char *nome_arquivo, long offset,
                     const char *termo);

// aguardar_navegacao: lê uma tecla em modo raw; retorna 1 para Enter (avança)
// ou 0 para Esc (para).
int aguardar_navegacao(void);

#endif /* INDEXER_H */