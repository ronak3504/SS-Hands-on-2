#include <stdio.h>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>

volatile sig_atomic_t timer_expired = 0;

void handle_timer(int sig)
{
    (void)sig;
    timer_expired = 1;
}

int main(void)
{
    struct sigaction sa = {0};
    struct itimerval timer;

    /* Register signal handler */
    sa.sa_handler = handle_timer;
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGVTALRM, &sa, NULL) == -1) {
        perror("sigaction");
        return 1;
    }

    /* Set ITIMER_VIRTUAL to 10 microseconds */
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 10;

    /* One-shot timer */
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 0;

    if (setitimer(ITIMER_VIRTUAL, &timer, NULL) == -1) {
        perror("setitimer");
        return 1;
    }

    printf("ITIMER_VIRTUAL set to 10 microseconds.\n");
    printf("Waiting for SIGVTALRM...\n");
    fflush(stdout);

    /* Consume user-mode CPU time until the signal arrives */
    volatile unsigned long long count = 0;

    while (!timer_expired) {
        count++;
    }

    printf("SIGVTALRM received!\n");
    printf("Virtual timer expired.\n");
    printf("Counter: %llu\n", count);

    return 0;
}
