#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hash_table.h"

/*
  hash_criar
  Aloca dinamicamente uma TabelaHash e inicializa todos os
  buckets com NULL (tabela vazia).
 */
TabelaHash *hash_criar(void) {
    TabelaHash *tabela = (TabelaHash *)malloc(sizeof(TabelaHash));
    if (!tabela) {
        fprintf(stderr, "Erro: falha ao alocar memória para a tabela hash.\n");
        return NULL;
    }

    // Inicializa todos os buckets como NULL
    for (int i = 0; i < TAMANHO_TABELA; i++) {
        tabela->buckets[i] = NULL;
    }

    return tabela;
}

/*
  hash_calcular
  Função de dispersão djb2 — distribui bem strings de tamanho
  variado no espaço de TAMANHO_TABELA buckets. 
  */
unsigned int hash_calcular(const char *chave) {
    unsigned long hash = 5381;
    int c;

    while ((c = (unsigned char)*chave++)) {
        // hash = hash * 33 + c  (operação bit-a-bit equivalente)
        hash = ((hash << 5) + hash) + c;
    }

    return (unsigned int)(hash % TAMANHO_TABELA);
}

/*
 hash_inserir
 Insere o par (termo, ocorrência) na tabela.

 Fluxo:
  1. Calcula o índice hash do termo.
  2. Percorre o bucket à procura do termo.
  3a. Se encontrado: apenas acrescenta uma Ocorrencia no início
      da lista encadeada do RegistroIndice existente.
  3b. Se não encontrado: cria um novo RegistroIndice e o insere
      no início do bucket (inserção no início da lista de colisão).
  */
int hash_inserir(TabelaHash *tabela, const char *termo,
                 const char *nome_arquivo, long offset) {
    unsigned int indice = hash_calcular(termo);

    //Cria a nova Ocorrencia que será adicionada
    Ocorrencia *nova_ocorrencia = (Ocorrencia *)malloc(sizeof(Ocorrencia));
    if (!nova_ocorrencia) {
        fprintf(stderr, "Erro: falha ao alocar Ocorrencia.\n");
        return 0;
    }
    strncpy(nova_ocorrencia->nome_arquivo, nome_arquivo, MAX_NOME_ARQUIVO - 1);
    nova_ocorrencia->nome_arquivo[MAX_NOME_ARQUIVO - 1] = '\0';
    nova_ocorrencia->offset = offset;
    nova_ocorrencia->prox   = NULL;

    // Verifica se o termo já existe no bucket 
    RegistroIndice *atual = tabela->buckets[indice];
    while (atual != NULL) {
        if (strcmp(atual->termo, termo) == 0) {
            // Termo encontrado: encadeia a nova ocorrência no início da lista
            nova_ocorrencia->prox = atual->lista;
            atual->lista = nova_ocorrencia;
            return 1;
        }
        atual = atual->prox;
    }

    // Termo não existe: cria novo RegistroIndice
    RegistroIndice *novo_registro = (RegistroIndice *)malloc(sizeof(RegistroIndice));
    if (!novo_registro) {
        fprintf(stderr, "Erro: falha ao alocar RegistroIndice.\n");
        free(nova_ocorrencia);
        return 0;
    }
    strncpy(novo_registro->termo, termo, MAX_TERMO - 1);
    novo_registro->termo[MAX_TERMO - 1] = '\0';
    novo_registro->lista = nova_ocorrencia;

    // Insere o novo registro no início do bucket (encadeamento de colisão)
    novo_registro->prox      = tabela->buckets[indice];
    tabela->buckets[indice]  = novo_registro;

    return 1;
}

/* 
 hash_buscar
 Procura um termo na tabela e retorna seu RegistroIndice.
 Retorna NULL se o termo não estiver indexado.
*/
RegistroIndice *hash_buscar(TabelaHash *tabela, const char *termo) {
    unsigned int indice = hash_calcular(termo);

    RegistroIndice *atual = tabela->buckets[indice];
    while (atual != NULL) {
        if (strcmp(atual->termo, termo) == 0) {
            return atual; // Termo encontrado 
        }
        atual = atual->prox;
    }

    return NULL; // Termo não encontrado
}

/* 
 hash_destruir
 Libera recursivamente toda a memória da tabela:
  - Cada Ocorrencia de cada RegistroIndice.
  - Cada RegistroIndice de cada bucket.
  - A própria TabelaHash.
*/
void hash_destruir(TabelaHash *tabela) {
    if (!tabela) return;

    for (int i = 0; i < TAMANHO_TABELA; i++) {
        RegistroIndice *registro = tabela->buckets[i];

        while (registro != NULL) {
            RegistroIndice *prox_registro = registro->prox;

            // Libera a lista de ocorrências deste registro
            Ocorrencia *ocorrencia = registro->lista;
            while (ocorrencia != NULL) {
                Ocorrencia *prox_ocorrencia = ocorrencia->prox;
                free(ocorrencia);
                ocorrencia = prox_ocorrencia;
            }

            free(registro);
            registro = prox_registro;
        }

        tabela->buckets[i] = NULL;
    }

    free(tabela);
}
