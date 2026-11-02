#include <stdio.h>
#include <stdlib.h>
#include "mymemory.h"

static strategy_t estrategia_atual = FIRST_FIT;

void mymemory_set_strategy(strategy_t strategy)
{
    estrategia_atual = strategy;
}

mymemory_t *mymemory_init(size_t size)
{
    if (size == 0)
        return NULL;

    mymemory_t *memory = malloc(sizeof(mymemory_t));
    if (memory == NULL)
        return NULL;

    memory->pool = malloc(size);
    if (memory->pool == NULL) {
        free(memory);
        return NULL;
    }

    memory->total_size = size;
    memory->head = NULL;
    return memory;
}

void *mymemory_alloc(mymemory_t *memory, size_t size)
{
    if (memory == NULL || size == 0)
        return NULL;

    char *fim_pool = (char *)memory->pool + memory->total_size;
    allocation_t *anterior = NULL;
    allocation_t *atual = memory->head;

    char *melhor_inicio = NULL;
    size_t melhor_tamanho = 0;
    allocation_t *melhor_anterior = NULL;

    while (1) {
        char *inicio_lacuna;
        char *fim_lacuna;

        if (anterior == NULL)
            inicio_lacuna = (char *)memory->pool;
        else
            inicio_lacuna = (char *)anterior->start + anterior->size;

        if (atual == NULL)
            fim_lacuna = fim_pool;
        else
            fim_lacuna = (char *)atual->start;

        size_t tamanho_lacuna = (size_t)(fim_lacuna - inicio_lacuna);

        if (tamanho_lacuna >= size) {
            int escolher = 0;

            if (melhor_inicio == NULL)
                escolher = 1;
            else if (estrategia_atual == BEST_FIT && tamanho_lacuna < melhor_tamanho)
                escolher = 1;
            else if (estrategia_atual == WORST_FIT && tamanho_lacuna > melhor_tamanho)
                escolher = 1;

            if (escolher) {
                melhor_inicio = inicio_lacuna;
                melhor_tamanho = tamanho_lacuna;
                melhor_anterior = anterior;
            }

            if (estrategia_atual == FIRST_FIT)
                break;
        }

        if (atual == NULL)
            break;

        anterior = atual;
        atual = atual->next;
    }

    if (melhor_inicio == NULL)
        return NULL;

    allocation_t *novo = malloc(sizeof(allocation_t));
    if (novo == NULL)
        return NULL;

    novo->start = melhor_inicio;
    novo->size = size;

    if (melhor_anterior == NULL) {
        novo->next = memory->head;
        memory->head = novo;
    } else {
        novo->next = melhor_anterior->next;
        melhor_anterior->next = novo;
    }

    return novo->start;
}

void mymemory_free(mymemory_t *memory, void *ptr)
{
    if (memory == NULL || ptr == NULL)
        return;

    allocation_t *anterior = NULL;
    allocation_t *atual = memory->head;

    while (atual != NULL && atual->start != ptr) {
        anterior = atual;
        atual = atual->next;
    }

    if (atual == NULL)
        return;

    if (anterior == NULL)
        memory->head = atual->next;
    else
        anterior->next = atual->next;

    free(atual);
}

void mymemory_display(mymemory_t *memory)
{
    if (memory == NULL)
        return;

    printf("   Alocacoes atuais:\n");

    if (memory->head == NULL) {
        printf("     (nenhuma alocacao)\n");
        return;
    }

    allocation_t *atual = memory->head;
    while (atual != NULL) {
        printf("     endereco %p | deslocamento %5td | tamanho %5zu bytes\n",
               atual->start,
               (char *)atual->start - (char *)memory->pool,
               atual->size);
        atual = atual->next;
    }
}

void mymemory_stats(mymemory_t *memory)
{
    if (memory == NULL)
        return;

    int num_alocacoes = 0;
    size_t total_alocado = 0;
    allocation_t *atual;

    for (atual = memory->head; atual != NULL; atual = atual->next) {
        num_alocacoes++;
        total_alocado += atual->size;
    }

    size_t total_livre = memory->total_size - total_alocado;

    char *fim_pool = (char *)memory->pool + memory->total_size;
    char *inicio_lacuna = (char *)memory->pool;
    size_t maior_lacuna = 0;
    int num_fragmentos = 0;

    atual = memory->head;
    while (1) {
        char *fim_lacuna;

        if (atual == NULL)
            fim_lacuna = fim_pool;
        else
            fim_lacuna = (char *)atual->start;

        size_t tamanho_lacuna = (size_t)(fim_lacuna - inicio_lacuna);

        if (tamanho_lacuna > 0) {
            num_fragmentos++;
            if (tamanho_lacuna > maior_lacuna)
                maior_lacuna = tamanho_lacuna;
        }

        if (atual == NULL)
            break;

        inicio_lacuna = (char *)atual->start + atual->size;
        atual = atual->next;
    }

    printf("   Estatisticas:\n");
    printf("     numero de alocacoes     : %d\n", num_alocacoes);
    printf("     memoria total alocada   : %zu bytes\n", total_alocado);
    printf("     memoria total livre     : %zu bytes\n", total_livre);
    printf("     maior bloco livre       : %zu bytes\n", maior_lacuna);
    printf("     fragmentos livres       : %d\n", num_fragmentos);
}

void mymemory_cleanup(mymemory_t *memory)
{
    if (memory == NULL)
        return;

    allocation_t *atual = memory->head;
    while (atual != NULL) {
        allocation_t *proximo = atual->next;
        free(atual);
        atual = proximo;
    }

    free(memory->pool);
    free(memory);
}