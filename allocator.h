#ifndef NUMA_ALLOCATOR
#define NUMA_ALLOCATOR

#include <stddef.h>
#include <pthread.h>

#define BINS 12

typedef struct {
    void *starting_addr;
    size_t size;
    struct free_block *next;
} free_block;

typedef struct {
    void *start_addr;
    size_t heap_size;
    unsigned numa_node;
    free_block *free_list[BINS];
    pthread_mutex_t lock;
} numa_heap;


void init_allocator(size_t heap_size);
void free_allocator(void);

void *allocate_localy(size_t size);
void *allocate_interleaved(size_t size);

void deallocate(void *ptr);

#endif

