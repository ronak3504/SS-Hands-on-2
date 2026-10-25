#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

int main(void)
{
    printf("[RECEIVER] PID = %ld\n", (long)getpid());
    printf("[RECEIVER] Waiting for signals...\n");
    fflush(stdout);

    while (1) {
        pause();
        printf("[RECEIVER] A caught signal interrupted pause().\n");
        fflush(stdout);
    }

    return EXIT_SUCCESS;
}
