#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>

int main()
{
    int pipe1[2], pipe2[2];
    pid_t pid1, pid2, pid3;
    int status;
    char choice;

    while (1)
    {
        printf("\n========== Q35: DIRECTORY COUNTER ==========\n");
        printf("1. Count directories in the current directory\n");
        printf("0. Exit\n");
        printf("Enter choice: ");
        fflush(stdout);

        if (scanf(" %c", &choice) != 1)
            break;

        if (choice == '0')
        {
            printf("Exiting program.\n");
            break;
        }

        if (choice != '1')
        {
            printf("Invalid choice. Try again.\n");
            continue;
        }

        /*
         * First pipe:
         * ls -l  ->  grep ^d
         */
        if (pipe(pipe1) == -1)
        {
            perror("pipe1");
            continue;
        }

        /*
         * Second pipe:
         * grep ^d  ->  wc -l
         */
        if (pipe(pipe2) == -1)
        {
            perror("pipe2");
            close(pipe1[0]);
            close(pipe1[1]);
            continue;
        }

        /* Process 1: ls -l */
        pid1 = fork();

        if (pid1 == -1)
        {
            perror("fork ls");
            close(pipe1[0]);
            close(pipe1[1]);
            close(pipe2[0]);
            close(pipe2[1]);
            continue;
        }

        if (pid1 == 0)
        {
            if (dup2(pipe1[1], STDOUT_FILENO) == -1)
            {
                perror("dup2 ls");
                _exit(1);
            }

            close(pipe1[0]);
            close(pipe1[1]);
            close(pipe2[0]);
            close(pipe2[1]);

            execlp("ls", "ls", "-l", (char *)NULL);

            perror("execlp ls");
            _exit(127);
        }

        /* Process 2: grep ^d */
        pid2 = fork();

        if (pid2 == -1)
        {
            perror("fork grep");
            close(pipe1[0]);
            close(pipe1[1]);
            close(pipe2[0]);
            close(pipe2[1]);

            waitpid(pid1, NULL, 0);
            continue;
        }

        if (pid2 == 0)
        {
            if (dup2(pipe1[0], STDIN_FILENO) == -1)
            {
                perror("dup2 grep input");
                _exit(1);
            }

            if (dup2(pipe2[1], STDOUT_FILENO) == -1)
            {
                perror("dup2 grep output");
                _exit(1);
            }

            close(pipe1[0]);
            close(pipe1[1]);
            close(pipe2[0]);
            close(pipe2[1]);

            execlp("grep", "grep", "^d", (char *)NULL);

            perror("execlp grep");
            _exit(127);
        }

        /* Process 3: wc -l */
        pid3 = fork();

        if (pid3 == -1)
        {
            perror("fork wc");
            close(pipe1[0]);
            close(pipe1[1]);
            close(pipe2[0]);
            close(pipe2[1]);

            waitpid(pid1, NULL, 0);
            waitpid(pid2, NULL, 0);
            continue;
        }

        if (pid3 == 0)
        {
            if (dup2(pipe2[0], STDIN_FILENO) == -1)
            {
                perror("dup2 wc");
                _exit(1);
            }

            close(pipe1[0]);
            close(pipe1[1]);
            close(pipe2[0]);
            close(pipe2[1]);

            execlp("wc", "wc", "-l", (char *)NULL);

            perror("execlp wc");
            _exit(127);
        }

        /* Parent closes all pipe descriptors */
        close(pipe1[0]);
        close(pipe1[1]);
        close(pipe2[0]);
        close(pipe2[1]);

        /* Wait for all three processes */
        waitpid(pid1, &status, 0);
        waitpid(pid2, &status, 0);
        waitpid(pid3, &status, 0);

        printf("Directory counting completed.\n");
    }

    return 0;
}
