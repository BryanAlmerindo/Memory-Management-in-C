/*
 * mymemory.h
 *
 * Interface (a "vitrine") da biblioteca de gerenciamento de memória.
 * Aqui ficam só as DECLARAÇÕES: os tipos e as assinaturas das funções.
 * As IMPLEMENTAÇÕES ficam em mymemory.c.
 */

/*
 * "Include guard": evita que este arquivo seja incluído duas vezes no
 * mesmo .c (o que causaria erro de "tipo redefinido").
 * Na 1ª vez, MYMEMORY_H não existe -> define e lê o arquivo.
 * Na 2ª vez, MYMEMORY_H já existe -> pula tudo até o #endif.
 */
#ifndef MYMEMORY_H
#define MYMEMORY_H

#include <stddef.h> /* size_t */

/*
 * Um bloco alocado dentro do pool (um nó da lista encadeada).
 * Estrutura definida pelo enunciado.
 */
typedef struct allocation {
    void *start;             /* onde o bloco começa (endereço dentro do pool) */
    size_t size;             /* quantos bytes o bloco tem */
    struct allocation *next; /* próximo bloco da lista (NULL = fim da lista) */
} allocation_t;

/*
 * O gerenciador: o pool inteiro + a lista de blocos alocados.
 * Estrutura definida pelo enunciado.
 *
 * DECISÃO DE PROJETO: a lista é mantida ORDENADA PELO ENDEREÇO (start).
 * Assim, os espaços livres são simplesmente as "lacunas" entre um bloco
 * e o próximo (e antes do primeiro / depois do último).
 */
typedef struct {
    void *pool;          /* ponteiro para o bloco de memória real */
    size_t total_size;   /* tamanho total do pool, em bytes */
    allocation_t *head;  /* primeiro bloco alocado (NULL = nenhum) */
} mymemory_t;

/*
 * Estratégias de alocação.
 * Um enum é só uma lista de constantes com nome:
 * FIRST_FIT vale 0, BEST_FIT vale 1, WORST_FIT vale 2.
 */
typedef enum {
    FIRST_FIT, /* primeira lacuna onde o bloco cabe */
    BEST_FIT,  /* menor lacuna onde o bloco cabe */
    WORST_FIT  /* maior lacuna */
} strategy_t;

/* Funções pedidas pelo enunciado */
mymemory_t *mymemory_init(size_t size);
void *mymemory_alloc(mymemory_t *memory, size_t size);
void mymemory_free(mymemory_t *memory, void *ptr);
void mymemory_display(mymemory_t *memory);
void mymemory_stats(mymemory_t *memory);
void mymemory_cleanup(mymemory_t *memory);

/*
 * Função extra (não está no enunciado): escolhe a estratégia usada por
 * mymemory_alloc. Foi necessária porque a assinatura de mymemory_alloc
 * é fixa e não tem um parâmetro para a estratégia.
 */
void mymemory_set_strategy(strategy_t strategy);

#endif /* MYMEMORY_H */
