#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

int main(void)
{
    if (signal(SIGINT, SIG_IGN) == SIG_ERR) {
        perror("signal");
        return EXIT_FAILURE;
    }

    printf("=== Ignore and Reset SIGINT ===\n\n");
    printf("PHASE 1: SIGINT is being ignored.\n");
    printf("Press Ctrl+C during the next 10 seconds.\n");
    fflush(stdout);

    for (int i = 10; i > 0; i--) {
        printf("Ignored phase: %d seconds remaining\n", i);
        fflush(stdout);
        sleep(1);
    }

    if (signal(SIGINT, SIG_DFL) == SIG_ERR) {
        perror("signal");
        return EXIT_FAILURE;
    }

    printf("\nPHASE 2: SIGINT has been reset to its default action.\n");
    printf("Press Ctrl+C now to terminate this program.\n");
    fflush(stdout);

    while (1) {
        pause();
    }

    return EXIT_SUCCESS;
}
