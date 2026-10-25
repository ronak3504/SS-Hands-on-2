
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

void handle_sigfpe(int sig)
{
    const char message[] =
        "\n[HANDLER] SIGFPE caught using sigaction()!\n"
        "[HANDLER] Floating-point exception signal handled.\n"
        "[HANDLER] Exiting safely.\n";

    (void)sig;
    write(STDOUT_FILENO, message, sizeof(message) - 1);
    _exit(EXIT_SUCCESS);
}

int main(void)
{
    struct sigaction sa = {0};

    sa.sa_handler = handle_sigfpe;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGFPE, &sa, NULL) == -1) {
        perror("sigaction");
        return EXIT_FAILURE;
    }

    printf("=== SIGFPE Handling using sigaction() ===\n");
    printf("[1] SIGFPE handler registered successfully.\n");
    printf("[2] Press Enter to trigger SIGFPE.\n");
    fflush(stdout);

    getchar();

    printf("\n[3] Raising SIGFPE now...\n");
    fflush(stdout);

    raise(SIGFPE);

    printf("This line should not execute.\n");
    return EXIT_SUCCESS;
}
