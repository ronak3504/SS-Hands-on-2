#include <stdio.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>

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

    if (sigaction(SIGPROF, &sa, NULL) == -1) {
        perror("sigaction");
        return 1;
    }

    timer.it_value.tv_sec = 10;
    timer.it_value.tv_usec = 0;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 0;

    if (setitimer(ITIMER_PROF, &timer, NULL) == -1) {
        perror("setitimer");
        return 1;
    }

    printf("ITIMER_PROF started: 10 seconds of CPU time.\n");
    printf("Working in batches; press Ctrl+C to stop if needed.\n");
    fflush(stdout);

    volatile unsigned long long count = 0;

    while (!timer_expired) {
        for (volatile int i = 0; i < 1000000; i++)
            count++;

        struct timespec delay = {0, 100000L};
        while (nanosleep(&delay, &delay) == -1 && errno == EINTR) {
            if (timer_expired)
                break;
        }
    }

    printf("\nSIGPROF received! Timer expired.\n");
    printf("Counter: %llu\n", count);

    return 0;
}
