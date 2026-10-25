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

    printf("=== FIFO SENDER ===\n");
    printf("FIFO: %s\n", FIFO_NAME);
    printf("Waiting for the receiver to connect...\n");

    /* Blocks until a receiver opens the FIFO for reading. */
    fd = open(FIFO_NAME, O_WRONLY);

    if (fd == -1)
    {
        perror("open");
        return 1;
    }

    printf("Receiver connected!\n");
    printf("Enter messages. Type 'exit' to stop.\n\n");

    while (1)
    {
        printf("You: ");
        fflush(stdout);

        if (fgets(message, sizeof(message), stdin) == NULL)
        {
            printf("\nInput closed. Stopping sender.\n");
            break;
        }

        /* Ensure the entire input line fits in one message. */
        if (strchr(message, '\n') == NULL && !feof(stdin))
        {
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF)
                ;

            printf("Message too long. Please use fewer than %d characters.\n",
                   BUFFER_SIZE);
            continue;
        }

        size_t length = strlen(message);

        if (length > 0 && message[length - 1] == '\n')
            message[--length] = '\0';

        if (length == 0)
        {
            printf("Empty messages are not sent.\n");
            continue;
        }

        /* Include the terminating null byte in the transmission. */
        if (write(fd, message, length + 1) != (ssize_t)(length + 1))
        {
            perror("write");
            break;
        }

        if (strcmp(message, "exit") == 0)
        {
            printf("Exit message sent. Closing sender.\n");
            break;
        }
    }

    close(fd);
    return 0;
}
