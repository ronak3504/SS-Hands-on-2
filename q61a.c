#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

void handle_sigsegv(int sig)
{
    const char message[] =
        "\n[HANDLER] SIGSEGV caught using sigaction()!\n"
        "[HANDLER] Invalid memory access detected.\n"
        "[HANDLER] Exiting safely.\n";

    (void)sig;
    write(STDOUT_FILENO, message, sizeof(message) - 1);
    _exit(EXIT_SUCCESS);
}

int main(void)
{
    struct sigaction sa = {0};

    sa.sa_handler = handle_sigsegv;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGSEGV, &sa, NULL) == -1) {
        perror("sigaction");
        return EXIT_FAILURE;
    }

    printf("=== SIGSEGV Handling using sigaction() ===\n");
    printf("[1] SIGSEGV handler registered successfully.\n");
    printf("[2] Press Enter to trigger an invalid memory access...");
    fflush(stdout);

    getchar();

    printf("\n[3] Attempting to write to a null pointer.\n");
    fflush(stdout);

    volatile int *ptr = NULL;
    *ptr = 100;

    printf("This line should not execute.\n");
    return EXIT_SUCCESS;
}
