#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

volatile sig_atomic_t interrupt_count = 0;

void handle_sigint(int sig)
{
    const char message[] =
        "\n[HANDLER] SIGINT caught using sigaction()!\n"
        "[HANDLER] Program is still running.\n";

    (void)sig;
    interrupt_count++;
    write(STDOUT_FILENO, message, sizeof(message) - 1);
}

int main(void)
{
    struct sigaction sa = {0};

    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction");
        return EXIT_FAILURE;
    }

    printf("=== SIGINT Handling using sigaction() ===\n");
    printf("Handler registered successfully.\n");
    printf("Press Ctrl+C repeatedly to trigger the handler.\n");
    printf("Press Ctrl+\\ to terminate the program.\n\n");
    fflush(stdout);

    while (1) {
        pause();

        printf("[MAIN] SIGINT handled %d time(s).\n",
               (int)interrupt_count);
        fflush(stdout);
    }

    return EXIT_SUCCESS;
}
