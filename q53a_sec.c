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

    // Register signal handler
    if (signal(SIGALRM, handle_alarm) == SIG_ERR) {
        perror("signal");
        return 1;
    }

    // Initial expiration: 10 seconds
    timer.it_value.tv_sec = 10;
    timer.it_value.tv_usec = 0;

    // No repeated interval
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 0;

    // Start ITIMER_REAL
    if (setitimer(ITIMER_REAL, &timer, NULL) == -1) {
        perror("setitimer");
        return 1;
    }

    printf("ITIMER_REAL started: 10 seconds.\n");
    printf("Waiting for the timer to expire...\n");
    fflush(stdout);

    while (!timer_expired) {
        pause();
    }

    printf("\nSIGALRM received!\n");
    printf("The 10-second real-time timer has expired.\n");

    return 0;
}
