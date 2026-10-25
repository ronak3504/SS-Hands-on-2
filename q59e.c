#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>

void handle_sigalrm(int sig)
{
    const char message[] =
        "\n[HANDLER] SIGALRM received!\n"
        "[HANDLER] ITIMER_REAL timer expired.\n";

    (void)sig;
    write(STDOUT_FILENO, message, sizeof(message) - 1);
}

int main(void)
{
    if (signal(SIGALRM, handle_sigalrm) == SIG_ERR) {
        perror("signal");
        return EXIT_FAILURE;
    }

    struct itimerval timer = {0};

    /* First expiration after 5 seconds; no repeating interval. */
    timer.it_value.tv_sec = 5;
    timer.it_value.tv_usec = 0;

    if (setitimer(ITIMER_REAL, &timer, NULL) == -1) {
        perror("setitimer");
        return EXIT_FAILURE;
    }

    printf("=== SIGALRM using setitimer() ===\n");
    printf("ITIMER_REAL set for 5 seconds.\n\n");
    fflush(stdout);

    for (int i = 5; i > 0; i--) {
        printf("Waiting... %d seconds remaining (approximately)\n", i);
        fflush(stdout);
        sleep(1);
    }

    printf("\nMain program continues after the timer expires.\n");
    printf("Press Enter to exit.\n");
    getchar();

    return EXIT_SUCCESS;
}
