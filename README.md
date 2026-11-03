# Gerenciador de Memória em C

Trabalho 2 da disciplina de Programação de Software Básico.

Biblioteca simples de gerenciamento de memória que aloca e libera blocos dentro de um **pool pré-alocado**, funcionando como um `malloc`/`free` próprio. O pool é controlado por uma lista encadeada que registra as regiões ocupadas.

## Sumário

- [Estrutura do projeto](#estrutura-do-projeto)
- [Como compilar e executar](#como-compilar-e-executar)
- [Estruturas de dados](#estruturas-de-dados)
- [Organização da memória](#organização-da-memória)
- [Decisão de projeto: lista ordenada por endereço](#decisão-de-projeto-lista-ordenada-por-endereço)
- [Funções da biblioteca](#funções-da-biblioteca)
- [Estratégia de alocação: First Fit](#estratégia-de-alocação-first-fit)
- [Testes](#testes)
- [Restrições atendidas](#restrições-atendidas)
- [Uso de ferramentas de IA](#uso-de-ferramentas-de-ia)

## Estrutura do projeto

| Arquivo | Conteúdo |
|---|---|
| `mymemory.h` | Definição das estruturas e protótipos das funções |
| `mymemory.c` | Implementação da biblioteca |
| `main.c` | Programa de testes |
| `README.md` | Este documento |

## Como compilar e executar

Compilar com todos os avisos habilitados:

```bash
gcc -Wall -Wextra -pedantic -std=c99 -g -o teste main.c mymemory.c
```

Executar os testes:

```bash
./teste
```

Verificar vazamentos de memória com o Valgrind:

```bash
valgrind --leak-check=full --show-leak-kinds=all ./teste
```

Gerar o pacote de entrega:

```bash
tar -czf trabalho2.tar.gz mymemory.h mymemory.c main.c README.md
```

O código compila sem erros e sem avisos, e o Valgrind não aponta vazamentos nem acessos inválidos.

## Estruturas de dados

### `allocation_t`

Representa **um bloco alocado**. Cada nó funciona como uma ficha que descreve onde o bloco está no pool.

```c
typedef struct allocation {
    void *start;              // início do bloco dentro do pool
    size_t size;              // tamanho do bloco em bytes
    struct allocation *next;  // próximo nó da lista
} allocation_t;
```

### `mymemory_t`

Representa **o gerenciador**: o pool inteiro e a lista de alocações.

```c
typedef struct {
    void *pool;               // início do bloco de memória real
    size_t total_size;        // tamanho total do pool
    allocation_t *head;       // primeiro nó da lista encadeada
} mymemory_t;
```

## Organização da memória

Existem dois tipos de memória envolvidos, e eles ficam em lugares diferentes:

| O quê | Onde fica | Quem cria |
|---|---|---|
| Dados do usuário (os blocos) | **dentro** do pool | a própria biblioteca, sem `malloc` |
| Nós da lista (`allocation_t`) | heap, **fora** do pool | `malloc` a cada alocação |
| Estrutura `mymemory_t` | heap, fora do pool | `malloc` no `mymemory_init` |
| O pool | heap | `malloc` no `mymemory_init` |

Exemplo com um pool de 100 bytes e dois blocos alocados:

```
POOL
+----------+-----------+-----------------+-----------------------+
|  A (20)  | livre(10) |     B (30)      |       livre (40)      |
+----------+-----------+-----------------+-----------------------+
0          20          30                60                     100

LISTA (fora do pool)
head -> [A: start=0, size=20] -> [B: start=30, size=30] -> NULL
```

O pool guarda apenas os dados do usuário. Toda a informação de controle fica nos nós da lista.

## Decisão de projeto: lista ordenada por endereço

A lista guarda **somente os blocos ocupados**, sempre em ordem crescente de endereço. Os espaços livres **não são armazenados**: eles são calculados a partir da lista.

Como os nós estão em ordem, cada lacuna livre é o espaço entre o fim de um bloco e o início do próximo:

| Lacuna | Início | Fim |
|---|---|---|
| Antes do primeiro bloco | início do pool | `head->start` |
| Entre dois blocos | `anterior->start + anterior->size` | `atual->start` |
| Depois do último bloco | fim do último bloco | `pool + total_size` |

Vantagens dessa escolha:

- **Sem lista de livres para manter sincronizada.** Não há risco de as duas listas divergirem.
- **Liberação simples.** O `free` só remove o nó da lista. Lacunas vizinhas se unem automaticamente, porque passam a ser calculadas como um único espaço entre os nós que restaram. Não é preciso fazer fusão (coalescência) manual.
- **Ordem mantida sem ordenação.** O nó novo é inserido entre os mesmos vizinhos que o bloco tem no pool, então a lista continua ordenada.

O custo é percorrer a lista a cada alocação, o que é aceitável para o tamanho do problema.

## Funções da biblioteca

### `mymemory_t *mymemory_init(size_t size)`

Cria o gerenciador. Aloca a estrutura `mymemory_t` e o pool de `size` bytes, e inicia a lista vazia (`head = NULL`).

Retorna `NULL` se `size` for 0 ou se alguma alocação falhar. Se o pool não puder ser alocado, a estrutura já criada é liberada antes de retornar, evitando vazamento.

### `void *mymemory_alloc(mymemory_t *memory, size_t size)`

Reserva `size` bytes dentro do pool e retorna o endereço de início do bloco.

Funcionamento:

1. Percorre a lista com dois ponteiros, `anterior` e `atual`. Cada volta corresponde a uma lacuna: a que fica entre `anterior` e `atual`.
2. Calcula o início, o fim e o tamanho da lacuna.
3. Se a lacuna comporta o pedido, para o laço (First Fit).
4. Se a lista acabou e nenhuma lacuna serviu, retorna `NULL`.
5. Cria um nó com `malloc`, preenche `start` e `size` e insere na lista:
   - se `anterior == NULL`, a lacuna é a do início do pool e o nó novo vira o `head`
   - senão, o nó novo entra logo depois de `anterior`

O laço usa `while (1)` para que a volta com `atual == NULL` também seja executada. É nela que se testa a lacuna do fim do pool, inclusive quando a lista está vazia.

A inserção segue esta ordem para não perder a referência do próximo nó:

```c
novo->next = anterior->next;   // 1. o novo aponta para o seguinte
anterior->next = novo;         // 2. o anterior aponta para o novo
```

Retorna `NULL` se `size` for 0, se não houver lacuna suficiente ou se o `malloc` do nó falhar.

### `void mymemory_free(mymemory_t *memory, void *ptr)`

Libera o bloco que começa em `ptr`.

Funcionamento:

1. Procura na lista o nó cujo `start` é igual a `ptr`.
2. Se não encontrar, não faz nada.
3. Se encontrar, remove o nó religando o anterior ao próximo (ou atualizando o `head`) e libera o nó com `free`.

O pool não é alterado: o espaço volta a ser livre porque deixa de haver um nó descrevendo aquela região.

Como só remove nós que existem, a função ignora com segurança:

- `ptr == NULL`
- ponteiros fora do pool
- ponteiros para o meio de um bloco
- liberação duplicada do mesmo ponteiro

### `void mymemory_display(mymemory_t *memory)`

Mostra todas as alocações atuais, com endereço, deslocamento em relação ao início do pool e tamanho.

### `void mymemory_stats(mymemory_t *memory)`

Mostra as estatísticas do pool:

- número total de alocações
- memória total alocada, em bytes
- memória total livre, em bytes
- maior bloco contíguo livre
- número de fragmentos livres

As lacunas são calculadas da mesma forma que no `mymemory_alloc`. Lacunas de tamanho 0 (blocos encostados) não são contadas como fragmentos.

### `void mymemory_cleanup(mymemory_t *memory)`

Libera todos os recursos: cada nó da lista, o pool e a estrutura `mymemory_t`. O ponteiro para o próximo nó é guardado antes de liberar o atual, para não acessar memória já liberada.

## Estratégia de alocação: First Fit

A implementação usa **First Fit**: a lista é percorrida em ordem de endereço e o bloco é colocado na **primeira lacuna grande o suficiente**.

Exemplo com lacunas de 40, 12, 90 e 20 bytes e pedido de 15 bytes: o First Fit escolhe a lacuna de **40**, sem olhar as demais.

É a estratégia mais simples e geralmente a mais rápida, pois a busca para assim que encontra uma lacuna adequada.

## Testes

O `main.c` contém dois grupos de testes.

### Teste básico

Pool de 1000 bytes:

1. Aloca blocos de 100, 200 e 300 bytes
2. Libera o bloco do meio, criando uma lacuna
3. Aloca 150 bytes e confere que a lacuna do meio foi reaproveitada
4. Testa casos especiais, que não devem quebrar o programa:
   - alocação de 0 bytes (retorna `NULL`)
   - alocação maior que o pool (retorna `NULL`)
   - `free(NULL)`
   - `free` de ponteiro fora do pool
   - `free` do meio de um bloco
   - `free` duplo do mesmo ponteiro

### Teste com 50 blocos

Conforme o enunciado, usa um conjunto de **50 blocos de tamanhos variados** (10, 25, 50, 100, 250, 500, 1000...) em um pool de 12000 bytes, e **varia a ordem das alocações**:

| Ordem | Como funciona |
|---|---|
| Crescente | índices 0, 1, 2, ..., 49 |
| Decrescente | índices 49, 48, ..., 0 |
| Intercalada | índice `(i * 7) % 50`, que visita todos os 50 sem repetir |

Para cada ordem:

1. Aloca os 50 blocos
2. Libera os de índice ímpar, criando lacunas espalhadas
3. Faz uma segunda rodada de 10 pedidos de tamanhos variados
4. Mostra as estatísticas e verifica a lista

### Verificação automática

A função `verificar` percorre a lista e confere que todo bloco está dentro do pool, em ordem crescente e sem sobreposição com o anterior.

A função `usar_bloco` escreve em todos os bytes de cada bloco alocado. Assim, se algum endereço devolvido fosse inválido, o Valgrind apontaria o erro.

### Fragmentação observada

Na segunda rodada, o pedido de 1000 bytes falha nos três cenários. As estatísticas antes dele mostram o motivo:

```
memoria total livre: 6868 bytes
maior bloco livre: 900 bytes
fragmentos livres: 25
```

Há memória livre suficiente no total, mas ela está dividida em 25 pedaços e o maior tem 900 bytes. Esse é o efeito da **fragmentação externa**, causado pela liberação intercalada dos blocos.

## Restrições atendidas

- `malloc` e `free` são usados **apenas** para o pool, a estrutura `mymemory_t` e os nós da lista. Os blocos do usuário são posicionados dentro do pool pela própria biblioteca.
- Os endereços retornados são sempre válidos e ficam dentro do pool.
- Não há vazamentos de memória (verificado com Valgrind).
- O código compila sem erros e sem avisos com `-Wall -Wextra -pedantic -std=c99`.

## Uso de ferramentas de IA

<!-- Preencham com a descrição real de como a IA foi usada no trabalho. -->

## Integrantes

- Bryan Leandro Almerindo Brasil
- Júnior Fernando Stahl
