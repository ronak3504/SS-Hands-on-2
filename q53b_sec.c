
#include <stdio.h>
#include <signal.h>
#include <sys/time.h>

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

    sa.sa_handler = handle_timer;
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGVTALRM, &sa, NULL) == -1) {
        perror("sigaction");
        return 1;
    }

    timer.it_value.tv_sec = 10;
    timer.it_value.tv_usec = 0;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 0;

    if (setitimer(ITIMER_VIRTUAL, &timer, NULL) == -1) {
        perror("setitimer");
        return 1;
    }

    printf("ITIMER_VIRTUAL started: 10 seconds of user CPU time.\n");
    printf("Running a bounded, periodically checked workload...\n");
    fflush(stdout);

    unsigned long long count = 0;

    while (!timer_expired) {
        // A finite batch of calculations
        for (volatile int i = 0; i < 100000; i++)
            count++;
    }

    printf("\nSIGVTALRM received!\n");
    printf("Virtual timer expired. Counter = %llu\n", count);

    return 0;
}

