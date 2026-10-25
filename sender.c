#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>

int main(void)
{
    pid_t pid;

    printf("Enter receiver PID: ");

    if (scanf("%d", &pid) != 1 || pid <= 0) {
        fprintf(stderr, "Invalid PID.\n");
        return EXIT_FAILURE;
    }

    if (kill(pid, SIGSTOP) == -1) {
        perror("kill");
        return EXIT_FAILURE;
    }

    printf("[SENDER] SIGSTOP sent to PID %ld.\n", (long)pid);
    return EXIT_SUCCESS;
}
