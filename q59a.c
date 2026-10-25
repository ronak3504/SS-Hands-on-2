#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

void handle_sigsegv(int sig)
{
    const char message[] =
        "\n[HANDLER] SIGSEGV caught!\n"
        "[HANDLER] Invalid memory access detected.\n"
        "[HANDLER] Exiting safely.\n";

    (void)sig;
    write(STDOUT_FILENO, message, sizeof(message) - 1);
    _exit(EXIT_SUCCESS);
}

int main(void)
{
    printf("=== SIGSEGV Signal Handling Demo ===\n\n");

    if (signal(SIGSEGV, handle_sigsegv) == SIG_ERR) {
        perror("signal");
        return EXIT_FAILURE;
    }

    printf("[1] SIGSEGV handler registered successfully.\n");
    printf("[2] Program is ready to trigger a real segmentation fault.\n");
    printf("[3] Press Enter to continue...");
    fflush(stdout);

    getchar();

    printf("\n[4] Attempting invalid memory access now.\n");
    fflush(stdout);

    volatile int *ptr = (volatile int *)0;
    *ptr = 100;

    printf("This line should not execute.\n");
    return EXIT_SUCCESS;
}
