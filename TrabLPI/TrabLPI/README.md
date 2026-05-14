# Indexador de Alta Performance
**Avaliação AP1 N2 — Linguagem de Programação I — 2026.1**
**IFCE Campus Aracati**
**Professor Raimundo valter**

---

## Visão Geral

Este programa implementa um **sistema de indexação de texto** baseado em **Tabela Hash com encadeamento** (separate chaining). Ele percorre arquivos `.txt` de uma pasta, indexa todas as palavras com seus *offsets* exatos e, na fase de busca, usa `fseek` para recuperar e exibir o trecho de contexto de cada ocorrência de forma praticamente instantânea.

---

## Estrutura de Arquivos

```
indexador/
├── hash_table.h   — Definição das structs e protótipos da tabela hash
├── hash_table.c   — Implementação da tabela hash (criar, inserir, buscar, destruir)
├── indexer.h      — Protótipos do scanner de arquivos e do motor de busca
├── indexer.c      — Implementação da indexação e da recuperação com fseek
├── main.c         — Ponto de entrada: argumentos, loop de busca interativa
├── Makefile       — Compilação automática com gcc
└── README.md      — Este documento
```

---

## Compilação

Certifique-se de ter o `gcc` instalado. Na pasta do projeto:

```bash
gcc -Wall -std=c11 -o indexador main.c hash_table.c indexer.c

Depois execute:

./indexador /sua_pasta_com_txts
```



---

## Uso

```bash
./indexador <caminho_da_pasta>
```

**Exemplo:**

```bash
./indexador ./textos
```

O programa indexa todos os `.txt` da pasta e exibe o prompt de busca:

```
Pesquise o termo: _
```

Digite o termo desejado e pressione **Enter**. Os resultados aparecem no formato:

```
  arquivo.txt – "...trecho de 50 caracteres com o termo..."
```

Pressione **Enter** para avançar para a próxima página de resultados ou **Esc** para voltar ao prompt de busca. Digite `sair` para encerrar o programa.

---

## Descrição Detalhada dos Módulos

### `hash_table.h` / `hash_table.c` — Tabela de Dispersão

#### Estruturas

| Struct | Descrição |
|---|---|
| `Ocorrencia` | Nó de lista encadeada que guarda o nome do arquivo e o `offset` (posição em bytes) de uma ocorrência de um termo. |
| `RegistroIndice` | Entrada da tabela: guarda o `termo` e a lista de `Ocorrencia`. Também possui um ponteiro `prox` para resolver colisões por encadeamento. |
| `TabelaHash` | Vetor de `TAMANHO_TABELA` (1024) ponteiros para `RegistroIndice`. |

#### Funções

| Função | Assinatura | Descrição |
|---|---|---|
| `hash_criar` | `TabelaHash *hash_criar(void)` | Aloca e inicializa uma tabela hash vazia, com todos os buckets apontando para `NULL`. |
| `hash_calcular` | `unsigned int hash_calcular(const char *chave)` | Implementa a função de dispersão **djb2** (`hash = hash * 33 + c`), conhecida por boa distribuição com strings. Retorna um índice entre 0 e `TAMANHO_TABELA - 1`. |
| `hash_inserir` | `int hash_inserir(TabelaHash *, const char *termo, const char *nome_arquivo, long offset)` | Calcula o bucket, percorre a lista do bucket procurando o termo. Se encontrado, acrescenta uma `Ocorrencia` no início da lista do `RegistroIndice`. Se não encontrado, cria um novo `RegistroIndice` e o insere no bucket. |
| `hash_buscar` | `RegistroIndice *hash_buscar(TabelaHash *, const char *termo)` | Calcula o bucket do termo e percorre a lista procurando uma entrada com `strcmp`. Retorna o `RegistroIndice` ou `NULL`. Complexidade média **O(1)**. |
| `hash_destruir` | `void hash_destruir(TabelaHash *)` | Percorre todos os buckets, libera cada `Ocorrencia`, cada `RegistroIndice` e por fim a própria `TabelaHash`. Evita vazamentos de memória. |

---

### `indexer.h` / `indexer.c` — Indexação e Recuperação

#### Constantes

| Constante | Valor | Descrição |
|---|---|---|
| `CONTEXTO_TAMANHO` | 50 | Número de caracteres exibidos em torno do termo na saída. |
| `OCORRENCIAS_POR_PAGINA` | 10 | Quantidade de resultados exibidos antes de pausar para navegação. |

#### Funções

| Função | Descrição |
|---|---|
| `normalizar_termo` | Converte o termo para letras minúsculas com `tolower` e remove pontuação das bordas (vírgulas, pontos, aspas etc.) com `isalnum`. Garante consistência nas comparações da tabela hash. |
| `indexar_arquivo` | Abre o arquivo em modo binário (`"rb"`) para garantia de `ftell`/`fseek` corretos. Percorre o arquivo caractere a caractere para pular espaços, registra o `offset` com `ftell` antes de cada palavra, lê a palavra com `fscanf("%50s")`, normaliza e insere na tabela se tiver ≥ 5 caracteres. |
| `indexar_pasta` | Usa `opendir`/`readdir` para listar entradas da pasta. Filtra arquivos com extensão `.txt` (comparação case-insensitive) e chama `indexar_arquivo` para cada um. Retorna o total de arquivos indexados. |
| `exibir_contexto` | Abre o arquivo com `fopen`, calcula o início do trecho (`offset - CONTEXTO_TAMANHO/2`), posiciona com `fseek(fp, inicio, SEEK_SET)` e lê os bytes para exibição. Quebras de linha são substituídas por espaço para manter a saída em uma linha. |
| `buscar_e_exibir` | Normaliza o termo buscado. Se o termo tiver < 5 caracteres, delega para `buscar_termo_pequeno` (varredura linear). Caso contrário, chama `hash_buscar` e percorre a lista de `Ocorrencia`, chamando `exibir_contexto` para cada uma. Pausa a cada `OCORRENCIAS_POR_PAGINA` resultados. |
| `aguardar_navegacao` | Coloca o terminal em modo *raw* (`termios`) para ler uma única tecla sem precisar de Enter. Retorna `0` se Esc (código ASCII 27) for pressionado, ou `1` para qualquer outra tecla. |
| `buscar_termo_pequeno` | Varredura linear em todos os `.txt` da pasta para termos com menos de 5 caracteres. Não utiliza a tabela hash. Exibe os resultados e respeita a paginação. |

---

### `main.c` — Ponto de Entrada

| Função | Descrição |
|---|---|
| `exibir_cabecalho` | Exibe o banner do programa. |
| `main` | Valida os argumentos (`argc == 2`), cria a tabela hash, chama `indexar_pasta`, entra no loop de busca com `fgets` e, ao encerrar, chama `hash_destruir`. |

---

## Regras de Negócio Implementadas

1. **Termos curtos (< 5 caracteres):** não são armazenados na tabela. A busca por eles é feita por varredura linear nos arquivos.
2. **Colisões:** resolvidas por encadeamento separado (*separate chaining*) — cada bucket é uma lista encadeada de `RegistroIndice`.
3. **Acesso aleatório:** após indexar, a busca usa `fseek(fp, offset, SEEK_SET)` para ir diretamente ao byte onde o termo começa, sem reler o arquivo inteiro.
4. **Normalização:** todos os termos são comparados em minúsculas e sem pontuação nas bordas.
5. **Paginação:** a cada 10 resultados, o programa pausa e aguarda Enter ou Esc.

---

## Exemplo de Execução

```
==============================================
   INDEXADOR DE ALTA PERFORMANCE
   Tabelas de Dispersão — LPI 2026.1
   IFCE Campus Aracati
==============================================

Indexando arquivos em './textos'...
  Indexando: artigo1.txt
  Indexando: artigo2.txt
Indexação concluída. 2 arquivo(s) processado(s).

Digite "sair" a qualquer momento para encerrar.

Pesquise o termo: machine

Resultados para "machine":

  artigo1.txt – "...n of machine learning has tra..."
  artigo2.txt – "...he virtual machine environment..."

  Total: 2 ocorrência(s) encontrada(s).

Pesquise o termo: sair
Encerrando o indexador. Até logo!
```

---

## Limitações e Possíveis Melhorias

- O programa foi desenvolvido e testado em sistemas **Linux/macOS** (usa `dirent.h` e `termios.h`). Para Windows, seria necessário usar `windows.h` e `conio.h`.
- A normalização atual considera apenas ASCII. Caracteres acentuados não são normalizados.
- O tamanho da tabela (`TAMANHO_TABELA = 1024`) pode ser ajustado em `hash_table.h` para coleções maiores de arquivos.
