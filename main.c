#include <stdio.h>
#include <string.h>
#include "mymemory.h"

#define NUM_BLOCOS 50
#define POOL_ESTRATEGIAS 12000

static const size_t tamanhos[NUM_BLOCOS] = {
    10, 25, 50, 100, 250, 500, 1000, 15, 30, 60,
    120, 300, 600, 40, 80, 200, 400, 800, 20, 45,
    90, 150, 350, 700, 12, 35, 70, 140, 280, 560,
    18, 55, 110, 220, 440, 900, 22, 65, 130, 260,
    520, 33, 66, 125, 375, 750, 8, 27, 81, 243};

#define NUM_SEGUNDA_RODADA 10

static const size_t segunda_rodada[NUM_SEGUNDA_RODADA] = {
    300, 45, 700, 120, 1000, 60, 250, 15, 500, 90};

enum
{
    CRESCENTE,
    DECRESCENTE,
    INTERCALADA
};

static const char *nome_ordem[] = {
    "crescente", "decrescente", "intercalada"};

static int indice_na_ordem(int i, int ordem)
{
    if (ordem == DECRESCENTE)
        return NUM_BLOCOS - 1 - i;

    if (ordem == INTERCALADA)
        return (i * 7) % NUM_BLOCOS;

    return i;
}

static int verificar(mymemory_t *memory)
{
    char *inicio_pool = (char *)memory->pool;
    char *fim_pool = inicio_pool + memory->total_size;
    char *fim_anterior = inicio_pool;

    allocation_t *atual;

    for (atual = memory->head; atual != NULL; atual = atual->next)
    {
        char *inicio = (char *)atual->start;
        char *fim = inicio + atual->size;

        if (inicio < fim_anterior || fim > fim_pool)
        {
            printf("  [ERRO] bloco em %p (tamanho %zu) fora do pool, "
                   "fora de ordem ou sobreposto!\n",
                   atual->start, atual->size);

            return 0;
        }

        fim_anterior = fim;
    }

    return 1;
}

static void usar_bloco(void *p, size_t size)
{
    if (p != NULL)
        memset(p, 0xAB, size);
}

static void teste_basico(void)
{
    mymemory_t *mem;
    void *a, *b, *c, *d;
    int x;

    printf("==================================================\n");
    printf(" TESTE BASICO (pool de 1000 bytes, FIRST FIT)\n");
    printf("==================================================\n");

    mem = mymemory_init(1000);

    if (mem == NULL)
    {
        printf("mymemory_init retornou NULL\n\n");
        return;
    }

    printf("\n-> alloc 100, 200 e 300 bytes\n");

    a = mymemory_alloc(mem, 100);
    b = mymemory_alloc(mem, 200);
    c = mymemory_alloc(mem, 300);

    usar_bloco(a, 100);
    usar_bloco(b, 200);
    usar_bloco(c, 300);

    mymemory_display(mem);

    printf("\n-> free do bloco do meio (200 bytes)\n");

    mymemory_free(mem, b);

    mymemory_display(mem);
    mymemory_stats(mem);

    printf("\n-> alloc 150 bytes (deve ocupar a lacuna do meio)\n");

    d = mymemory_alloc(mem, 150);
    usar_bloco(d, 150);

    mymemory_display(mem);

    printf("\n-> casos especiais (nada deve quebrar):\n");

    printf("   alloc de 0 bytes        -> %s\n",
           mymemory_alloc(mem, 0) == NULL ? "NULL (ok)" : "NAO NULL (errado)");

    printf("   alloc de 5000 bytes     -> %s\n",
           mymemory_alloc(mem, 5000) == NULL ? "NULL (ok)" : "NAO NULL (errado)");

    mymemory_free(mem, NULL);
    printf("   free(NULL)              -> ok\n");

    mymemory_free(mem, &x);
    printf("   free de ptr fora do pool -> ok\n");

    mymemory_free(mem, (char *)a + 10);
    printf("   free do meio de um bloco -> ok\n");

    mymemory_free(mem, b);
    printf("   free duplo do mesmo ptr  -> ok\n");

    printf("\n-> estado final:\n");

    mymemory_display(mem);
    mymemory_stats(mem);

    printf("   verificacao da lista: %s\n",
           verificar(mem) ? "ok" : "FALHOU");

    mymemory_cleanup(mem);

    printf("\n");
}

static void executar_cenario(int ordem)
{
    mymemory_t *mem;
    void *ptrs[NUM_BLOCOS];

    int i, ok;

    printf("--------------------------------------------------\n");
    printf(" FIRST FIT | ordem %s | pool de %d bytes\n",
           nome_ordem[ordem], POOL_ESTRATEGIAS);
    printf("--------------------------------------------------\n");

    mem = mymemory_init(POOL_ESTRATEGIAS);

    if (mem == NULL)
    {
        printf("mymemory_init retornou NULL\n\n");
        return;
    }

    ok = 0;

    for (i = 0; i < NUM_BLOCOS; i++)
    {
        int k = indice_na_ordem(i, ordem);

        ptrs[k] = mymemory_alloc(mem, tamanhos[k]);

        usar_bloco(ptrs[k], tamanhos[k]);

        if (ptrs[k] != NULL)
            ok++;
    }

    printf("1) alocados %d de %d blocos\n", ok, NUM_BLOCOS);

    for (i = 1; i < NUM_BLOCOS; i += 2)
        mymemory_free(mem, ptrs[i]);

    printf("2) liberados os blocos de indice impar\n");

    mymemory_stats(mem);

    printf("3) segunda rodada de pedidos:\n");

    for (i = 0; i < NUM_SEGUNDA_RODADA; i++)
    {
        void *p = mymemory_alloc(mem, segunda_rodada[i]);

        usar_bloco(p, segunda_rodada[i]);

        if (p != NULL)
        {
            printf("   %4zu bytes -> deslocamento %td\n",
                   segunda_rodada[i],
                   (char *)p - (char *)mem->pool);
        }
        else
        {
            printf("   %4zu bytes -> FALHOU (sem lacuna suficiente)\n",
                   segunda_rodada[i]);
        }
    }

    mymemory_stats(mem);

    printf("   verificacao da lista: %s\n\n",
           verificar(mem) ? "ok" : "FALHOU");

    mymemory_cleanup(mem);
}

static void teste_first_fit(void)
{
    int ordem;

    printf("==================================================\n");
    printf(" TESTE FIRST FIT (%d blocos)\n", NUM_BLOCOS);
    printf("==================================================\n");

    for (ordem = CRESCENTE; ordem <= INTERCALADA; ordem++)
        executar_cenario(ordem);
}

int main(void)
{
    teste_basico();
    teste_first_fit();

    return 0;
}