#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

static uint8_t *block_chain = NULL;
static int blocks;
static int32_t block_size;

/* Initialize an internal memory pool.
 * The allocated memory is <size>*<n> and is a sequence of <n> blocks of size <size>. */
bool mem_initialize(int32_t size, int n)
{
    uint8_t *addr, *ptr;

    if (size <= 0 || n <= 0)
        return 0;

    if ((addr = malloc(size * n)) == NULL)
        return false;

    block_size = size;
    ptr = NULL;
    while (n--)
    {
        block_chain = addr;
        *(uint8_t**)addr = ptr;
        ptr = addr;
        addr += size;
    }

    return true;
}


/*
 * Allocate a block from the internal memory pool.
 */
void *mem_allocate(int32_t size)
{
    if (size > block_size || block_chain == NULL)
        return NULL;

    uint8_t *addr = block_chain;
    block_chain = *(uint8_t**)addr;
    *(int32_t*)addr = block_size;    /* Make size info available */

    return (void*)addr;
}


/*
 * Free a block and return it to the internal memory pool.
 */
void mem_free(void *addr)
{
    *(uint8_t**)addr = block_chain;
    block_chain = addr;
}
