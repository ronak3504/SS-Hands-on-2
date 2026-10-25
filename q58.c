#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void *thread_function(void *arg)
{
    int thread_number = *(int *)arg;

    printf("Thread %d started | Thread ID: %lu\n",
           thread_number,
           (unsigned long)pthread_self());

    return NULL;
}

int main(void)
{
    pthread_t threads[3];
    int thread_numbers[3] = {1, 2, 3};

    printf("Creating three threads...\n\n");

    for (int i = 0; i < 3; i++) {
        int result = pthread_create(
            &threads[i],
            NULL,
            thread_function,
            &thread_numbers[i]
        );

        if (result != 0) {
            fprintf(stderr, "Failed to create thread %d\n", i + 1);

            for (int j = 0; j < i; j++)
                pthread_join(threads[j], NULL);

            return EXIT_FAILURE;
        }
    }

    for (int i = 0; i < 3; i++)
        pthread_join(threads[i], NULL);

    printf("\nAll three threads have completed.\n");

    return 0;
}
