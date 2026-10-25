#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#define ITERATIONS 100
#define RUNS 5

static inline uint64_t read_tsc(void)
{
    unsigned int low, high;

    __asm__ volatile (
        "lfence\n\t"
        "rdtsc"
        : "=a"(low), "=d"(high)
        :
        : "memory"
    );

    return ((uint64_t)high << 32) | low;
}

int main(void)
{
    uint64_t total_ticks = 0;

    printf("=== Execution Time Measurement ===\n");
    printf("System call: getppid()\n");
    printf("Calls per run: %d\n", ITERATIONS);
    printf("Measurement runs: %d\n\n", RUNS);

    for (int run = 1; run <= RUNS; run++) {
        volatile pid_t parent_pid = 0;

        uint64_t start = read_tsc();

        for (int i = 0; i < ITERATIONS; i++) {
            parent_pid = getppid();
        }

        uint64_t end = read_tsc();

        uint64_t elapsed = end - start;
        total_ticks += elapsed;

        printf("Run %d:\n", run);
        printf("  Start TSC: %llu\n",
               (unsigned long long)start);
        printf("  End TSC:   %llu\n",
               (unsigned long long)end);
        printf("  Total TSC ticks: %llu\n",
               (unsigned long long)elapsed);
        printf("  Average ticks per call: %.2f\n\n",
               (double)elapsed / ITERATIONS);

        (void)parent_pid;
    }

    printf("=== Summary ===\n");
    printf("Total getppid() calls: %d\n", ITERATIONS * RUNS);
    printf("Average TSC ticks per 100-call run: %.2f\n",
           (double)total_ticks / RUNS);

    return 0;
}
