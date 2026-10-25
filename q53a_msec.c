#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>

volatile sig_atomic_t timer_expired = 0;

void handle_alarm(int sig)
{
    (void)sig;
    timer_expired = 1;
}

int main(void)
{
    struct itimerval timer;

    // Register SIGALRM handler
    if (signal(SIGALRM, handle_alarm) == SIG_ERR) {
        perror("signal");
        return 1;
    }

    // Initial expiration: 10 microseconds
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 10;

    // One-shot timer
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 0;

    // Start ITIMER_REAL
    if (setitimer(ITIMER_REAL, &timer, NULL) == -1) {
        perror("setitimer");
        return 1;
    }

    printf("ITIMER_REAL started: 10 microseconds.\n");
    fflush(stdout);

    // Wait for timer expiration
    while (!timer_expired)
        pause();

    printf("SIGALRM received!\n");
    printf("The real-time timer has expired.\n");

    return 0;
}
