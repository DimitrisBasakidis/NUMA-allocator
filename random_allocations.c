#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "allocator.h"

#define NUM_OPERATIONS 10000
#define ALLOC_SIZES_COUNT 5

// Allocation sizes for the random test
size_t alloc_sizes[ALLOC_SIZES_COUNT] = {64, 256, 1024, 4096, 16384};

// Function to get the current time in nanoseconds
double get_time_ns() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1e9 + ts.tv_nsec;
}

// Random Allocation and Reallocation
void benchmark_random_alloc() {
    printf("Benchmarking Random Allocation and Reallocation:\n");

    void **numa_local_blocks = malloc(NUM_OPERATIONS * sizeof(void *));
    void **numa_interleaved_blocks = malloc(NUM_OPERATIONS * sizeof(void *));
    void **malloc_blocks = malloc(NUM_OPERATIONS * sizeof(void *));

    double start_time, elapsed_time;

    // NUMA Local Allocation
    start_time = get_time_ns();
    for (int i = 0; i < NUM_OPERATIONS; i++) {
        size_t size = alloc_sizes[rand() % ALLOC_SIZES_COUNT];
        numa_local_blocks[i] = allocate_localy(size);
        if (rand() % 2 && numa_local_blocks[i]) {
            deallocate(numa_local_blocks[i]);
            numa_local_blocks[i] = NULL;
        }
    }
    elapsed_time = get_time_ns() - start_time;
    printf("NUMA Local Random Alloc/Free: %.2f ns\n", elapsed_time / NUM_OPERATIONS);

    // NUMA Interleaved Allocation
    start_time = get_time_ns();
    for (int i = 0; i < NUM_OPERATIONS; i++) {
        size_t size = alloc_sizes[rand() % ALLOC_SIZES_COUNT];
        numa_interleaved_blocks[i] = allocate_interleaved(size);
        if (rand() % 2 && numa_interleaved_blocks[i]) {
            deallocate(numa_interleaved_blocks[i]);
            numa_interleaved_blocks[i] = NULL;
        }
    }
    elapsed_time = get_time_ns() - start_time;
    printf("NUMA Interleaved Random Alloc/Free: %.2f ns\n", elapsed_time / NUM_OPERATIONS);

    // Standard Malloc
    start_time = get_time_ns();
    for (int i = 0; i < NUM_OPERATIONS; i++) {
        size_t size = alloc_sizes[rand() % ALLOC_SIZES_COUNT];
        malloc_blocks[i] = malloc(size);
        if (rand() % 2 && malloc_blocks[i]) {
            free(malloc_blocks[i]);
            malloc_blocks[i] = NULL;
        }
    }
    elapsed_time = get_time_ns() - start_time;
    printf("Malloc Random Alloc/Free: %.2f ns\n", elapsed_time / NUM_OPERATIONS);

    // Clean up any remaining allocations
    for (int i = 0; i < NUM_OPERATIONS; i++) {
        if (numa_local_blocks[i]) deallocate(numa_local_blocks[i]);
        if (numa_interleaved_blocks[i]) deallocate(numa_interleaved_blocks[i]);
        if (malloc_blocks[i]) free(malloc_blocks[i]);
    }

    free(numa_local_blocks);
    free(numa_interleaved_blocks);
    free(malloc_blocks);
}


int main() {
    init_allocator(1024 * 1024 * 24);


    // Random Allocation Benchmark
    benchmark_random_alloc();

    free_allocator();
    return 0;
}


