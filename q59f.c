#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>

volatile sig_atomic_t timer_fired = 0;

void handle_sigvtalrm(int sig)
{
    const char message[] =
        "\n[HANDLER] SIGVTALRM received!\n"
        "[HANDLER] One second of user-mode CPU time accumulated.\n";

    (void)sig;
    timer_fired = 1;
    write(STDOUT_FILENO, message, sizeof(message) - 1);
}

int main(void)
{
    if (signal(SIGVTALRM, handle_sigvtalrm) == SIG_ERR) {
        perror("signal");
        return EXIT_FAILURE;
    }

    printf("=== ITIMER_VIRTUAL Demonstration ===\n");
    printf("This timer measures user-mode CPU time.\n");
    printf("Press Enter to start the experiment...");
    fflush(stdout);

    getchar();

    struct itimerval timer = {0};
    timer.it_value.tv_sec = 1;

    if (setitimer(ITIMER_VIRTUAL, &timer, NULL) == -1) {
        perror("setitimer");
        return EXIT_FAILURE;
    }

    printf("\n[PHASE A] Process sleeping for 3 seconds...\n");
    fflush(stdout);

    sleep(3);

    if (timer_fired) {
        printf("[PHASE A] Timer expired unexpectedly during sleep.\n");
        return EXIT_FAILURE;
    }

    printf("[PHASE A] Three seconds passed, but SIGVTALRM has NOT arrived.\n");
    printf("[PHASE A] The timer measures CPU time, not elapsed time.\n");

    printf("\n[PHASE B] Now consuming user-mode CPU time...\n");
    fflush(stdout);

    volatile unsigned long long counter = 0;

    while (!timer_fired) {
        counter++;
    }

    printf("\n[PHASE B] Computation stopped after the signal.\n");
    printf("Signal caught successfully: Yes\n");
    printf("Counter value: %llu\n", counter);

    return EXIT_SUCCESS;
}
