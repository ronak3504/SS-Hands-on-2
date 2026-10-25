#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>

int main(void)
{
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return EXIT_FAILURE;
    }

    if (pid == 0) {
        /* Child process */
        pid_t ppid = getppid();

        printf("[CHILD] My PID = %ld, Parent PID = %ld\n",
               (long)getpid(), (long)ppid);
        fflush(stdout);

        sleep(2);

        printf("[CHILD] Sending SIGKILL to parent...\n");
        fflush(stdout);

        if (kill(ppid, SIGKILL) == -1) {
            perror("kill");
            return EXIT_FAILURE;
        }

        /* Wait until the parent dies and the child is reparented. */
        while (getppid() == ppid)
            sleep(1);

        printf("[CHILD] Parent terminated!\n");
        printf("[CHILD] My PID = %ld, New PPID = %ld\n",
               (long)getpid(), (long)getppid());
        printf("[CHILD] I am now an orphan.\n");
        fflush(stdout);

        sleep(5);
        return EXIT_SUCCESS;
    }

    /* Parent process */
    printf("[PARENT] My PID = %ld, Child PID = %ld\n",
           (long)getpid(), (long)pid);
    printf("[PARENT] Waiting for the child to kill me...\n");
    fflush(stdout);

    while (1)
        pause();
}
