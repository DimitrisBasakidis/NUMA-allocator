#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#include "allocator.h"

#define NUM_ITERATIONS 10000  // Number of iterations for latency and throughput benchmarks
#define NUM_THREADS 4          // Number of threads for throughput benchmarks
#define ALLOC_SIZE 1024        // Default allocation size (1 KB)

// Timing utilities
double get_time_ns() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1e9 + ts.tv_nsec;
}

// Latency Benchmark
void benchmark_latency(size_t alloc_size) {
    printf("Benchmarking Latency for Allocations of Size %zu Bytes:\n", alloc_size);

    // NUMA Local Allocation
    double start_time = get_time_ns();
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        void *ptr = allocate_localy(alloc_size);
        deallocate(ptr);
    }
    double numa_local_latency = (get_time_ns() - start_time) / NUM_ITERATIONS;

    // NUMA Interleaved Allocation
    start_time = get_time_ns();
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        void *ptr = allocate_interleaved(alloc_size);
        deallocate(ptr);
    }
    double numa_interleaved_latency = (get_time_ns() - start_time) / NUM_ITERATIONS;

    // Standard malloc
    start_time = get_time_ns();
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        void *ptr = malloc(alloc_size);
        free(ptr);
    }
    double malloc_latency = (get_time_ns() - start_time) / NUM_ITERATIONS;

    printf("NUMA Local Latency: %.2f ns\n", numa_local_latency);
    printf("NUMA Interleaved Latency: %.2f ns\n", numa_interleaved_latency);
    printf("Malloc Latency: %.2f ns\n\n", malloc_latency);
}

// Throughput Benchmark
void *thread_alloc_work(void *arg) {
    size_t alloc_size = *(size_t *)arg;

    for (int i = 0; i < NUM_ITERATIONS; i++) {
        void *ptr = allocate_localy(alloc_size);
        deallocate(ptr);
    }
    return NULL;
}

void benchmark_throughput(size_t alloc_size) {
    printf("Benchmarking Throughput with %d Threads and Allocations of Size %zu Bytes:\n", NUM_THREADS, alloc_size);

    pthread_t threads[NUM_THREADS];
    double start_time;

    // NUMA Local Allocation
    start_time = get_time_ns();
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, thread_alloc_work, &alloc_size);
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    double numa_local_throughput = (get_time_ns() - start_time) / (NUM_THREADS * NUM_ITERATIONS);

    // NUMA Interleaved Allocation
    start_time = get_time_ns();
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, thread_alloc_work, &alloc_size);
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    double numa_interleaved_throughput = (get_time_ns() - start_time) / (NUM_THREADS * NUM_ITERATIONS);

    // Standard malloc
    start_time = get_time_ns();
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, thread_alloc_work, &alloc_size);
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    double malloc_throughput = (get_time_ns() - start_time) / (NUM_THREADS * NUM_ITERATIONS);

    printf("NUMA Local Throughput: %.2f ns per allocation\n", numa_local_throughput);
    printf("NUMA Interleaved Throughput: %.2f ns per allocation\n", numa_interleaved_throughput);
    printf("Malloc Throughput: %.2f ns per allocation\n\n", malloc_throughput);
}

// Access Time Benchmark
void benchmark_access_time(size_t alloc_size) {
    printf("Benchmarking Access Time for Allocations of Size %zu Bytes:\n", alloc_size);

    // NUMA Local Allocation
    void *ptr = allocate_localy(alloc_size);
    double start_time = get_time_ns();
    for (size_t i = 0; i < alloc_size; i += 64) {
        ((char *)ptr)[i] = i % 256;
    }
    double numa_local_access_time = get_time_ns() - start_time;
    deallocate(ptr);

    // NUMA Interleaved Allocation
    ptr = allocate_interleaved(alloc_size);
    start_time = get_time_ns();
    for (size_t i = 0; i < alloc_size; i += 64) {
        ((char *)ptr)[i] = i % 256;
    }
    double numa_interleaved_access_time = get_time_ns() - start_time;
    deallocate(ptr);

    // Standard malloc
    ptr = malloc(alloc_size);
    start_time = get_time_ns();
    for (size_t i = 0; i < alloc_size; i += 64) {
        ((char *)ptr)[i] = i % 256;
    }
    double malloc_access_time = get_time_ns() - start_time;
    free(ptr);

    printf("NUMA Local Access Time: %.2f ns\n", numa_local_access_time / alloc_size);
    printf("NUMA Interleaved Access Time: %.2f ns\n", numa_interleaved_access_time / alloc_size);
    printf("Malloc Access Time: %.2f ns\n\n", malloc_access_time / alloc_size);
}

// Main Benchmarking Function
int main() {
	
    init_allocator(1024 * 1024 * 24); // 24 MB allocator initialization

    size_t sizes[] = {64, 256, 1024, 4096, 16384}; // Different allocation sizes
    for (size_t i = 0; i < sizeof(sizes) / sizeof(sizes[0]); i++) {
        benchmark_latency(sizes[i]);
        benchmark_throughput(sizes[i]);
        benchmark_access_time(sizes[i]);
    }

    free_allocator(); // Clean up
    return 0;
}

