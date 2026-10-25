#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>

int main()
{
    int fd[2];
    pid_t pid;
    char message[200];
    char buffer[200];

    if (pipe(fd) == -1)
    {
        perror("pipe");
        return 1;
    }

    printf("========== PARENT TO CHILD IPC ==========\n");

    pid = fork();

    if (pid == -1)
    {
        perror("fork");
        close(fd[0]);
        close(fd[1]);
        return 1;
    }

    if (pid > 0)
    {
        // Parent process
        close(fd[0]);

        printf("[Parent] PID: %d\n", getpid());
        printf("[Parent] Child PID: %d\n", pid);

        printf("\nEnter a message for the child: ");
        fflush(stdout);

        if (fgets(message, sizeof(message), stdin) == NULL)
        {
            fprintf(stderr, "Error reading input.\n");
            close(fd[1]);
            waitpid(pid, NULL, 0);
            return 1;
        }

        size_t length = strlen(message);

        if (length > 0 && message[length - 1] == '\n')
            message[--length] = '\0';

        size_t total = length + 1;
        size_t sent = 0;

        while (sent < total)
        {
            ssize_t n = write(fd[1], message + sent, total - sent);

            if (n == -1)
            {
                if (errno == EINTR)
                    continue;

                perror("write");
                close(fd[1]);
                waitpid(pid, NULL, 0);
                return 1;
            }

            if (n == 0)
            {
                fprintf(stderr, "No data written.\n");
                close(fd[1]);
                waitpid(pid, NULL, 0);
                return 1;
            }

            sent += (size_t)n;
        }

        printf("[Parent] Sent %zu bytes through the pipe.\n", sent);

        close(fd[1]);

        int status;

        if (waitpid(pid, &status, 0) == -1)
        {
            perror("waitpid");
            return 1;
        }

        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
        {
            fprintf(stderr, "[Parent] Child did not finish successfully.\n");
            return 1;
        }

        printf("[Parent] Child completed successfully.\n");
    }
    else
    {
        // Child process
        close(fd[1]);

        printf("[Child] PID: %d\n", getpid());
        printf("[Child] Waiting for the parent's message...\n");
        fflush(stdout);

        size_t received = 0;

        while (received < sizeof(buffer))
        {
            ssize_t n = read(fd[0], buffer + received,
                             sizeof(buffer) - received);

            if (n == -1)
            {
                if (errno == EINTR)
                    continue;

                perror("read");
                close(fd[0]);
                _exit(1);
            }

            if (n == 0)
                break;

            received += (size_t)n;

            if (memchr(buffer, '\0', received) != NULL)
                break;
        }

        close(fd[0]);

        if (received == 0 ||
            memchr(buffer, '\0', received) == NULL)
        {
            fprintf(stderr, "[Child] No complete message received.\n");
            _exit(1);
        }

        printf("\n[Child] Message received: %s\n", buffer);
        printf("[Child] Received %zu bytes.\n", received);
        fflush(stdout);

        _exit(0);
    }

    return 0;
}
