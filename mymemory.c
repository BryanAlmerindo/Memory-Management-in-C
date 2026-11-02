/*
 * mymemory.c
 *
 * Implementação da biblioteca de gerenciamento de memória.
 *
 * Regra do enunciado: malloc/free SÓ podem ser usados para
 *   - o pool (o bloco grande),
 *   - a struct mymemory_t,
 *   - os nós da lista (allocation_t).
 * As alocações do usuário são "pedaços" do pool, calculados com
 * aritmética de ponteiros usando (char *), que anda de 1 em 1 byte.
 *
 * IDEIA CENTRAL (lista ordenada por endereço -> lacunas):
 *
 *   pool
 *   v
 *   +--------+==========+--------+======================+
 *   | bloco A|  lacuna  | bloco B|        lacuna        |
 *   +--------+==========+--------+======================+
 *            ^          ^        ^                      ^
 *     fim de A      início de B  fim de B          fim do pool
 *
 *   início da lacuna = fim do bloco anterior
 *                      (ou o início do pool, se não há bloco anterior)
 *   fim da lacuna    = início do bloco seguinte
 *                      (ou o fim do pool, se não há bloco seguinte)
 *   fim de um bloco  = (char *)bloco->start + bloco->size
 *   fim do pool      = (char *)memory->pool + memory->total_size
 */

#include <stdio.h>
#include <stdlib.h>
#include "mymemory.h"

/*
 * Estratégia atual. "static" aqui significa que esta variável só é
 * visível dentro deste arquivo (ninguém de fora mexe nela diretamente).
 */
static strategy_t estrategia_atual = FIRST_FIT;

void mymemory_set_strategy(strategy_t strategy)
{
    estrategia_atual = strategy;
}

/*
 * Cria o gerenciador e o pool de 'size' bytes.
 * Retorna NULL se não for possível.
 *
 * PASSO A PASSO:
 *   1. Se size == 0, retorne NULL (não faz sentido um pool vazio).
 *   2. Aloque (malloc) uma mymemory_t. Se der NULL, retorne NULL.
 *   3. Aloque (malloc) o pool de 'size' bytes.
 *      Se der NULL: libere a mymemory_t do passo 2 e retorne NULL
 *      (senão ela vazaria!).
 *   4. Preencha os campos: pool, total_size = size, head = NULL
 *      (nenhuma alocação ainda).
 *   5. Retorne o ponteiro para a mymemory_t.
 */
mymemory_t *mymemory_init(size_t size)
{
    /* TODO: implementar */
    (void)size; /* evita warning de "parâmetro não usado"; apague ao implementar */
    return NULL;
}

/*
 * Tenta alocar 'size' bytes dentro do pool, usando a estratégia atual.
 * Retorna o endereço do início do bloco, ou NULL se não couber.
 *
 * PASSO A PASSO:
 *   1. Se memory == NULL ou size == 0, retorne NULL.
 *   2. Percorra TODAS as lacunas, da esquerda para a direita, usando dois
 *      ponteiros: 'anterior' (o bloco antes da lacuna; NULL no início) e
 *      'atual' (o bloco depois da lacuna; NULL depois do último bloco).
 *      Atenção: são (número de blocos + 1) lacunas, então não esqueça a
 *      última, entre o último bloco e o fim do pool.
 *   3. Para cada lacuna, calcule o início, o fim e o tamanho (fim - início).
 *      Se o bloco couber (tamanho >= size), decida conforme a estratégia:
 *        FIRST_FIT: escolha esta lacuna e pare de procurar.
 *        BEST_FIT:  escolha esta se for MENOR que a melhor achada até agora.
 *        WORST_FIT: escolha esta se for MAIOR que a melhor achada até agora.
 *      Guarde o início da lacuna escolhida e o bloco 'anterior' a ela.
 *   4. Se nenhuma lacuna serviu, retorne NULL.
 *   5. Crie (malloc) um novo allocation_t e preencha start e size.
 *      Se o malloc falhar, retorne NULL.
 *   6. Insira o novo nó na lista LOGO DEPOIS do 'anterior' escolhido
 *      (se o 'anterior' for NULL, o novo nó vira o head).
 *      Assim a lista continua ordenada por endereço.
 *   7. Retorne o start do novo bloco.
 */
void *mymemory_alloc(mymemory_t *memory, size_t size)
{
    /* TODO: implementar */
    (void)memory;
    (void)size;
    return NULL;
}

/*
 * Libera o bloco que começa em 'ptr'.
 * Se ptr não for o início de um bloco alocado, não faz nada.
 *
 * PASSO A PASSO:
 *   1. Se memory == NULL ou ptr == NULL, retorne.
 *   2. Percorra a lista com 'anterior' e 'atual' procurando o nó em que
 *      atual->start == ptr.
 *   3. Se não achou, retorne (ptr inválido: não faz nada).
 *   4. Tire o nó da lista:
 *        se o 'anterior' existe -> anterior->next = atual->next;
 *        se não (era o head)    -> memory->head = atual->next;
 *   5. free(atual): libera só o NÓ de controle. Os bytes do pool não
 *      precisam de nada: como nenhum nó os cobre mais, eles passam
 *      automaticamente a fazer parte de uma lacuna.
 */
void mymemory_free(mymemory_t *memory, void *ptr)
{
    /* TODO: implementar */
    (void)memory;
    (void)ptr;
}

/*
 * Mostra todas as alocações atuais (início e tamanho de cada uma).
 *
 * PASSO A PASSO:
 *   1. Se memory == NULL, retorne.
 *   2. Se a lista estiver vazia, imprima algo como "nenhuma alocação".
 *   3. Percorra a lista imprimindo, para cada bloco:
 *        - o endereço (formato %p, com cast para (void *))
 *        - o deslocamento desde o início do pool, que é mais fácil de ler:
 *          (char *)bloco->start - (char *)memory->pool   (formato %td)
 *        - o tamanho (formato %zu)
 */
void mymemory_display(mymemory_t *memory)
{
    /* TODO: implementar */
    (void)memory;
}

/*
 * Mostra estatísticas gerais da memória.
 *
 * PASSO A PASSO:
 *   1. Se memory == NULL, retorne.
 *   2. Percorra a lista contando:
 *        - o número de alocações
 *        - o total alocado (soma dos size)
 *   3. Memória livre = total_size - total alocado.
 *   4. Percorra as lacunas (igual ao mymemory_alloc) para achar:
 *        - o maior bloco livre contíguo (a maior lacuna)
 *        - o número de fragmentos livres (lacunas com tamanho > 0)
 *   5. Imprima tudo.
 */
void mymemory_stats(mymemory_t *memory)
{
    /* TODO: implementar */
    (void)memory;
}

/*
 * Libera TUDO: todos os nós da lista, o pool e a própria mymemory_t.
 *
 * PASSO A PASSO:
 *   1. Se memory == NULL, retorne.
 *   2. Percorra a lista liberando cada nó.
 *      CUIDADO: guarde o 'next' ANTES de dar free no nó atual
 *      (depois do free, ler atual->next é use-after-free!).
 *   3. free(memory->pool).
 *   4. free(memory).
 */
void mymemory_cleanup(mymemory_t *memory)
{
    /* TODO: implementar */
    (void)memory;
}
