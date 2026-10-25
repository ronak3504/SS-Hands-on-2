#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

void handle_sigalrm(int sig)
{
    const char message[] =
        "\n[HANDLER] SIGALRM received!\n"
        "[HANDLER] The alarm timer has expired.\n";

    (void)sig;
    write(STDOUT_FILENO, message, sizeof(message) - 1);
}

int main(void)
{
    if (signal(SIGALRM, handle_sigalrm) == SIG_ERR) {
        perror("signal");
        return EXIT_FAILURE;
    }

    printf("=== SIGALRM using alarm() ===\n");
    printf("Setting an alarm for 10 seconds.\n");

    alarm(10);

    for (int i = 10; i > 0; i--) {
        printf("Waiting... %d seconds remaining (approximately)\n", i);
        fflush(stdout);
        sleep(1);
    }

    printf("\nMain program continues after the alarm.\n");
    printf("Press Enter to exit.\n");
    getchar();

    return EXIT_SUCCESS;
}
