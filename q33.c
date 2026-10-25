#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>

#define SIZE 256

/*
   Read one line from standard input.
   Returns 1 on success, 0 on EOF, and -1 on error.
*/
int get_input(char *buffer, size_t size)
{
    if (fgets(buffer, size, stdin) == NULL)
    {
        if (ferror(stdin))
            return -1;

        return 0;
    }

    size_t len = strlen(buffer);

    if (len > 0 && buffer[len - 1] == '\n')
    {
        buffer[len - 1] = '\0';
    }
    else
    {
        int ch;

        while ((ch = getchar()) != '\n' && ch != EOF)
            ;

        if (len == size - 1)
        {
            fprintf(stderr, "Message too long. Try again.\n");
            return -1;
        }
    }

    return 1;
}

/* Send a complete message through a pipe. */
int send_message(int fd, const char *message)
{
    size_t len = strlen(message) + 1;
    size_t sent = 0;

    while (sent < len)
    {
        ssize_t n = write(fd, message + sent, len - sent);

        if (n == -1)
        {
            if (errno == EINTR)
                continue;

            return -1;
        }

        if (n == 0)
            return -1;

        sent += (size_t)n;
    }

    return 0;
}

/* Receive one null-terminated message from a pipe. */
int receive_message(int fd, char *buffer, size_t size)
{
    size_t received = 0;

    while (received < size)
    {
        char ch;
        ssize_t n = read(fd, &ch, 1);

        if (n == -1)
        {
            if (errno == EINTR)
                continue;

            return -1;
        }

        if (n == 0)
            return 0;

        buffer[received++] = ch;

        if (ch == '\0')
            return 1;
    }

    fprintf(stderr, "Received message is too long.\n");
    return -1;
}

int main(void)
{
    int p_to_c[2];
    int c_to_p[2];

    pid_t pid;

    char message[SIZE];
    char reply[SIZE];

    /* Pipe 1: Parent -> Child */
    if (pipe(p_to_c) == -1)
    {
        perror("pipe");
        return 1;
    }

    /* Pipe 2: Child -> Parent */
    if (pipe(c_to_p) == -1)
    {
        perror("pipe");

        close(p_to_c[0]);
        close(p_to_c[1]);

        return 1;
    }

    pid = fork();

    if (pid == -1)
    {
        perror("fork");

        close(p_to_c[0]);
        close(p_to_c[1]);
        close(c_to_p[0]);
        close(c_to_p[1]);

        return 1;
    }

    if (pid > 0)
    {
        /* ============== PARENT PROCESS ============== */

        close(p_to_c[0]);
        close(c_to_p[1]);

        printf("========== PARENT PROCESS ==========\n");
        printf("Parent PID: %d\n", getpid());
        printf("Child PID : %d\n", pid);
        printf("Type 'exit' to end communication.\n\n");

        while (1)
        {
            printf("[Parent] Enter message: ");
            fflush(stdout);

            int input_status = get_input(message, sizeof(message));

            if (input_status != 1)
            {
                strcpy(message, "exit");
            }

            /* Send message to child. */
            if (send_message(p_to_c[1], message) == -1)
            {
                perror("[Parent] write");
                break;
            }

            /* Parent requested termination. */
            if (strcmp(message, "exit") == 0)
            {
                printf("[Parent] Ending communication.\n");
                break;
            }

            /* Wait for child's reply. */
            int status = receive_message(c_to_p[0], reply, sizeof(reply));

            if (status != 1)
            {
                if (status == -1)
                    perror("[Parent] read");
                else
                    fprintf(stderr, "[Parent] Child closed the pipe.\n");

                break;
            }

            /* Child requested termination. */
            if (strcmp(reply, "exit") == 0)
            {
                printf("[Parent] Child requested termination.\n");
                break;
            }

            printf("[Parent] Reply received: %s\n\n", reply);
        }

        close(p_to_c[1]);
        close(c_to_p[0]);

        int child_status;

        if (waitpid(pid, &child_status, 0) == -1)
        {
            perror("waitpid");
            return 1;
        }

        printf("[Parent] Child process terminated.\n");
    }
    else
    {
        /* ============== CHILD PROCESS ============== */

        close(p_to_c[1]);
        close(c_to_p[0]);

        printf("\n========== CHILD PROCESS ==========\n");
        printf("Child PID: %d\n", getpid());
        fflush(stdout);

        while (1)
        {
            /* Receive parent's message. */
            int status = receive_message(p_to_c[0], message, sizeof(message));

            if (status != 1)
            {
                if (status == -1)
                    perror("[Child] read");

                break;
            }

            printf("[Child] Message received: %s\n", message);

            /* Parent requested termination. */
            if (strcmp(message, "exit") == 0)
            {
                printf("[Child] Parent requested termination.\n");
                break;
            }

            printf("[Child] Enter reply (type 'exit' to quit): ");
            fflush(stdout);

            int input_status = get_input(reply, sizeof(reply));

            if (input_status != 1)
            {
                strcpy(reply, "exit");
            }

            /* Send reply to parent. */
            if (send_message(c_to_p[1], reply) == -1)
            {
                perror("[Child] write");
                break;
            }

            if (strcmp(reply, "exit") == 0)
            {
                printf("[Child] Ending communication.\n");
                break;
            }

            printf("[Child] Reply sent.\n\n");
        }

        close(p_to_c[0]);
        close(c_to_p[1]);

        _exit(0);
    }

    return 0;
}
