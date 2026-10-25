#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

int main()
{
    int fd[2];
    char message[200];
    char buffer[200];

    // Create the pipe
    if (pipe(fd) == -1)
    {
        perror("pipe");
        return 1;
    }

    printf("========== PIPE COMMUNICATION ==========\n");
    printf("Pipe created successfully.\n");
    printf("Read end descriptor  : %d\n", fd[0]);
    printf("Write end descriptor : %d\n", fd[1]);

    // Get user input
    printf("\nEnter a message to send through the pipe: ");
    fflush(stdout);

    if (fgets(message, sizeof(message), stdin) == NULL)
    {
        fprintf(stderr, "Error: Could not read input.\n");
        close(fd[0]);
        close(fd[1]);
        return 1;
    }

    // Remove trailing newline
    size_t length = strlen(message);

    if (length > 0 && message[length - 1] == '\n')
    {
        message[--length] = '\0';
    }

    // Write the message, including the null terminator
    size_t total = length + 1;
    size_t written = 0;

    while (written < total)
    {
        ssize_t n = write(fd[1], message + written, total - written);

        if (n == -1)
        {
            if (errno == EINTR)
                continue;

            perror("write");
            close(fd[0]);
            close(fd[1]);
            return 1;
        }

        if (n == 0)
        {
            fprintf(stderr, "Error: No data written.\n");
            close(fd[0]);
            close(fd[1]);
            return 1;
        }

        written += (size_t)n;
    }

    printf("\nMessage written successfully.\n");
    printf("Bytes written: %zu\n", written);

    // Read the message from the pipe
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
            close(fd[1]);
            return 1;
        }

        if (n == 0)
            break;

        received += (size_t)n;

        // Stop when the complete C string is received
        if (memchr(buffer, '\0', received) != NULL)
            break;
    }

    if (received == 0 || memchr(buffer, '\0', received) == NULL)
    {
        fprintf(stderr, "Error: No complete message received.\n");
        close(fd[0]);
        close(fd[1]);
        return 1;
    }

    printf("Message read successfully.\n");
    printf("Received message: %s\n", buffer);
    printf("Bytes read: %zu\n", received);

    if (strcmp(message, buffer) == 0)
        printf("Verification: SUCCESS - Messages match.\n");
    else
        printf("Verification: FAILED - Messages differ.\n");

    close(fd[0]);
    close(fd[1]);

    printf("\nBoth pipe descriptors closed.\n");

    return 0;
}
