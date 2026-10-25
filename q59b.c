#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

volatile sig_atomic_t interrupt_count = 0;

void handle_sigint(int sig)
{
    (void)sig;
    interrupt_count++;

    const char message[] =
        "\nSIGINT caught! The program is still running.\n";

    write(STDOUT_FILENO, message, sizeof(message) - 1);
}

int main(void)
{
    if (signal(SIGINT, handle_sigint) == SIG_ERR) {
        perror("signal");
        return EXIT_FAILURE;
    }

    printf("=== SIGINT Handling Demonstration ===\n");
    printf("Process ID: %ld\n", (long)getpid());
    printf("Press Ctrl+C to send SIGINT.\n");
    printf("Press Ctrl+C repeatedly to demonstrate handling.\n");
    printf("Enter 'q' and press Enter to exit normally.\n\n");
    fflush(stdout);

    while (1) {
        printf("Waiting for input (interrupts caught: %d): ",
               (int)interrupt_count);
        fflush(stdout);

        char input[32];

        if (fgets(input, sizeof(input), stdin) == NULL) {
            if (interrupt_count > 0) {
                clearerr(stdin);
                continue;
            }
            break;
        }

        if (input[0] == 'q' || input[0] == 'Q')
            break;
    }

    printf("\nProgram exiting normally.\n");
    return EXIT_SUCCESS;
}
