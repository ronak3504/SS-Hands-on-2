#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <string.h>
#include <errno.h>

#define FIFO_NAME "myfifo"
#define SIZE 256

int main(void)
{
    int fd;
    char buffer[SIZE];

    /* Create FIFO if it does not already exist */
    if (mkfifo(FIFO_NAME, 0666) == -1 && errno != EEXIST)
    {
        perror("mkfifo");
        return 1;
    }

    /* Make sure the existing path is actually a FIFO */
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

    /*
       O_RDWR prevents the FIFO from reporting EOF just because
       no external writer is currently connected.
       O_NONBLOCK prevents open() from waiting for another process.
    */
    fd = open(FIFO_NAME, O_RDWR | O_NONBLOCK);

    if (fd == -1)
    {
        perror("open");
        return 1;
    }

    printf("=== Q39: Waiting for FIFO Data ===\n");
    printf("FIFO: %s\n", FIFO_NAME);

    while (1)
    {
        fd_set readfds;
        struct timeval timeout;

        printf("\nWaiting up to 10 seconds for data...\n");
        printf("Write to the FIFO from another terminal.\n");

        FD_ZERO(&readfds);
        FD_SET(fd, &readfds);

        timeout.tv_sec = 10;
        timeout.tv_usec = 0;

        int result = select(fd + 1, &readfds, NULL, NULL, &timeout);

        if (result == -1)
        {
            if (errno == EINTR)
            {
                printf("select() interrupted. Try again.\n");
                continue;
            }

            perror("select");
            break;
        }
        else if (result == 0)
        {
            printf("TIMEOUT: No data received within 10 seconds.\n");
        }
        else
        {
            if (FD_ISSET(fd, &readfds))
            {
                ssize_t n = read(fd, buffer, sizeof(buffer) - 1);

                if (n > 0)
                {
                    buffer[n] = '\0';
                    printf("SUCCESS: Data received: %s\n", buffer);
                }
                else if (n == -1 && errno != EAGAIN &&
                         errno != EWOULDBLOCK && errno != EINTR)
                {
                    perror("read");
                    break;
                }
                else
                {
                    printf("No readable data. Try again.\n");
                }
            }
        }

        printf("\n1. Wait again\n");
        printf("2. Exit\n");
        printf("Choice: ");
        fflush(stdout);

        char choice[16];

        if (fgets(choice, sizeof(choice), stdin) == NULL)
            break;

        if (choice[0] == '2')
            break;

        if (choice[0] != '1')
            printf("Invalid choice. Waiting again.\n");
    }

    close(fd);

    printf("Program terminated.\n");

    return 0;
}
