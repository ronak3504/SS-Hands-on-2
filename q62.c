
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

int main(void)
{
    struct sigaction sa = {0};

    /* Phase 1: Ignore SIGINT */
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction: SIG_IGN");
        return EXIT_FAILURE;
    }

    printf("=== Ignore and Reset SIGINT ===\n");
    printf("[PHASE 1] SIGINT is now ignored.\n");
    printf("Press Ctrl+C to test. Press Enter to continue.\n");
    fflush(stdout);

    getchar();

    /* Phase 2: Restore the default action */
    sa.sa_handler = SIG_DFL;

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction: SIG_DFL");
        return EXIT_FAILURE;
    }

    printf("\n[PHASE 2] Default SIGINT action restored.\n");
    printf("Press Ctrl+C now. The program should terminate.\n");
    fflush(stdout);

    while (1) {
        pause();
    }

    return EXIT_SUCCESS;
}

