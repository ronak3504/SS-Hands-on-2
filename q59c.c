#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

void handle_sigfpe(int sig)
{
    const char message[] =
        "\n[HANDLER] SIGFPE caught successfully!\n"
        "[HANDLER] Floating-point exception signal received.\n"
        "[HANDLER] Exiting safely.\n";

    (void)sig;
    write(STDOUT_FILENO, message, sizeof(message) - 1);
    _exit(EXIT_SUCCESS);
}

int main(void)
{
    if (signal(SIGFPE, handle_sigfpe) == SIG_ERR) {
        perror("signal");
        return EXIT_FAILURE;
    }

    printf("=== SIGFPE Handling Demonstration ===\n");
    printf("SIGFPE handler registered.\n");
    printf("Press Enter to raise SIGFPE...\n");
    fflush(stdout);

    getchar();

    printf("\nRaising SIGFPE now...\n");
    fflush(stdout);

    if (raise(SIGFPE) != 0) {
        perror("raise");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
