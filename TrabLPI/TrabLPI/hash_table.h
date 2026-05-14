#ifndef HASH_TABLE_H
#define HASH_TABLE_H


#define TAMANHO_TABELA     1024  // Número de buckets da tabela hash
#define MAX_TERMO          51    // Tamanho máximo de um termo (50 chars + '\0')  
#define MAX_NOME_ARQUIVO   71    // Tamanho máximo do nome do arquivo             
#define MIN_TAMANHO_TERMO  2     // Termos menores que isso não entram na tabela  

/* 
  Ocorrencia
  Nó de uma lista encadeada que guarda onde um termo aparece.
 */
typedef struct ocorrencia {
    char nome_arquivo[MAX_NOME_ARQUIVO]; // Nome do arquivo onde o termo foi encontrado 
    long offset;                         // Deslocamento em bytes até o início do termo  
    struct ocorrencia *prox;             // Ponteiro para a próxima ocorrência
} Ocorrencia;

/* 
  RegistroIndice
  Entrada da tabela hash: guarda o termo e sua lista de
  ocorrências nos arquivos indexados.
  */
typedef struct registro_indice {
    char termo[MAX_TERMO];       // Termo indexado
    Ocorrencia *lista;           // Cabeça da lista encadeada de ocorrências        
    struct registro_indice *prox;// Encadeamento para colisão (separate chaining)   
} RegistroIndice;

/* 
  TabelaHash
  Estrutura principal: vetor de ponteiros para RegistroIndice.
 */
typedef struct {
    RegistroIndice *buckets[TAMANHO_TABELA]; // Array de buckets (listas encadeadas) 
} TabelaHash;

// Protótipos das funções da tabela hash 

// Aloca e inicializa uma nova tabela hash vazia 
TabelaHash *hash_criar(void);

// Libera toda a memória ocupada pela tabela hash
void hash_destruir(TabelaHash *tabela);

// Calcula o índice hash de uma string (função djb2)
unsigned int hash_calcular(const char *chave);

/*
  Insere uma ocorrência na tabela.
  - Se o termo ainda não existir, cria um novo RegistroIndice.
  - Se já existir, apenas encadeia uma nova Ocorrencia na lista.
  Retorna 1 em caso de sucesso, 0 em caso de erro de alocação.
 */
int hash_inserir(TabelaHash *tabela, const char *termo,
                 const char *nome_arquivo, long offset);

/*
 * Busca um termo na tabela.
 * Retorna o ponteiro para o RegistroIndice se encontrado,
 * ou NULL caso o termo não esteja na tabela.
 */
RegistroIndice *hash_buscar(TabelaHash *tabela, const char *termo);

#endif
