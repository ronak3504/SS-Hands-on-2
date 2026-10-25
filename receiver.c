#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

#define FIFO_NAME "myfifo"
#define BUFFER_SIZE 512

int main(void)
{
    int fd;
    char message[BUFFER_SIZE];

    /* Create the FIFO if it does not already exist. */
    if (mkfifo(FIFO_NAME, 0666) == -1 && errno != EEXIST)
    {
        perror("mkfifo");
        return 1;
    }

    struct stat st;

    if (lstat(FIFO_NAME, &st) == -1)
    {
        perror("lstat");
        return 1;
    }

    if (!S_ISFIFO(st.st_mode))
    {
        fprintf(stderr, "%s exists but is not a FIFO.\n", FIFO_NAME);
        return 1;
    }

    printf("=== FIFO RECEIVER ===\n");
    printf("FIFO: %s\n", FIFO_NAME);
    printf("Waiting for the sender to connect...\n");

    /* Blocks until a sender opens the FIFO for writing. */
    fd = open(FIFO_NAME, O_RDONLY);

    if (fd == -1)
    {
        perror("open");
        return 1;
    }

    printf("Sender connected! Waiting for messages...\n\n");

    while (1)
    {
        size_t used = 0;
        int complete = 0;

        /*
         * Read one complete message, including its terminating
         * null byte. Handle partial reads correctly.
         */
        while (used < sizeof(message))
        {
            ssize_t n = read(fd, message + used, sizeof(message) - used);

            if (n == -1)
            {
                if (errno == EINTR)
                    continue;

                perror("read");
                close(fd);
                return 1;
            }

            if (n == 0)
            {
                printf("\nSender disconnected.\n");
                close(fd);
                return 0;
            }

            char *terminator = memchr(message + used, '\0', (size_t)n);

            if (terminator != NULL)
            {
                used = (size_t)(terminator - message);
                complete = 1;
                break;
            }

            used += (size_t)n;
        }

        if (!complete)
        {
            fprintf(stderr, "Invalid or oversized message received.\n");
            close(fd);
            return 1;
        }

        printf("Received: %s\n", message);
        fflush(stdout);

        if (strcmp(message, "exit") == 0)
        {
            printf("Exit message received. Closing receiver.\n");
            break;
        }
    }

    close(fd);
    return 0;
}
