#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>

volatile sig_atomic_t timer_fired = 0;

void handle_sigprof(int sig)
{
    const char message[] =
        "\n[HANDLER] SIGPROF received!\n"
        "[HANDLER] ITIMER_PROF CPU-time timer expired.\n";

    (void)sig;
    timer_fired = 1;
    write(STDOUT_FILENO, message, sizeof(message) - 1);
}

int main(void)
{
    if (signal(SIGPROF, handle_sigprof) == SIG_ERR) {
        perror("signal");
        return EXIT_FAILURE;
    }

    printf("=== ITIMER_PROF Demonstration ===\n");
    printf("This timer measures user-mode + kernel-mode CPU time.\n");
    printf("Press Enter to start...");
    fflush(stdout);

    getchar();

    struct itimerval timer = {0};
    timer.it_value.tv_usec = 200000;  /* 0.2 seconds of CPU time */

    if (setitimer(ITIMER_PROF, &timer, NULL) == -1) {
        perror("setitimer");
        return EXIT_FAILURE;
    }

    printf("\n[PHASE A] Sleeping for 2 seconds...\n");
    fflush(stdout);

    sleep(2);

    if (timer_fired) {
        printf("[PHASE A] Timer expired during sleep unexpectedly.\n");
        return EXIT_FAILURE;
    }

    printf("[PHASE A] Two seconds passed; SIGPROF has not arrived.\n");
    printf("[PHASE A] Wall-clock time alone did not expire the timer.\n");

    printf("\n[PHASE B] Repeatedly calling getppid()...\n");
    fflush(stdout);

    unsigned long long calls = 0;
    volatile pid_t parent_pid = 0;

    while (!timer_fired) {
        parent_pid = getppid();
        calls++;
    }

    printf("\n[PHASE B] Timer expired during CPU execution.\n");
    printf("SIGPROF handler executed: Yes\n");
    printf("getppid() calls completed: %llu\n", calls);
    printf("Last parent PID returned: %ld\n", (long)parent_pid);

    return EXIT_SUCCESS;
}
