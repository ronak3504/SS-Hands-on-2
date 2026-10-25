#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

#define SIZE 256

#define FIFO_OUT "parent_to_child"
#define FIFO_IN  "child_to_parent"

int send_message(int fd, const char *message)
{
    size_t len = strlen(message) + 1;
    size_t sent = 0;

    while (sent < len)
    {
        ssize_t n = write(fd, message + sent, len - sent);

        if (n < 0)
        {
            if (errno == EINTR)
                continue;

            perror("write");
            return -1;
        }

        if (n == 0)
            return -1;

        sent += n;
    }

    return 0;
}

int receive_message(int fd, char *message, size_t size)
{
    size_t i = 0;

    while (i < size - 1)
    {
        char ch;
        ssize_t n = read(fd, &ch, 1);

        if (n == 0)
        {
            printf("[PARENT] FIFO closed by the other process.\n");
            return 0;
        }

        if (n < 0)
        {
            if (errno == EINTR)
                continue;

            perror("read");
            return -1;
        }

        if (ch == '\0')
        {
            message[i] = '\0';
            return 1;
        }

        message[i++] = ch;
    }

    message[i] = '\0';
    fprintf(stderr, "[PARENT] Received message was too long.\n");
    return -1;
}

int main(void)
{
    int write_fd, read_fd;
    char message[SIZE];

    if (mkfifo(FIFO_OUT, 0666) == -1 && errno != EEXIST)
    {
        perror("mkfifo parent_to_child");
        return 1;
    }

    if (mkfifo(FIFO_IN, 0666) == -1 && errno != EEXIST)
    {
        perror("mkfifo child_to_parent");
        return 1;
    }

    struct stat st;

    if (lstat(FIFO_OUT, &st) == -1 || !S_ISFIFO(st.st_mode))
    {
        fprintf(stderr, "parent_to_child is not a FIFO.\n");
        return 1;
    }

    if (lstat(FIFO_IN, &st) == -1 || !S_ISFIFO(st.st_mode))
    {
        fprintf(stderr, "child_to_parent is not a FIFO.\n");
        return 1;
    }

    printf("[PARENT] Waiting for child to connect...\n");

    write_fd = open(FIFO_OUT, O_WRONLY);

    if (write_fd == -1)
    {
        perror("open parent_to_child");
        return 1;
    }

    read_fd = open(FIFO_IN, O_RDONLY);

    if (read_fd == -1)
    {
        perror("open child_to_parent");
        close(write_fd);
        return 1;
    }

    printf("[PARENT] Connected successfully!\n");
    printf("[PARENT] Type 'exit' to end communication.\n");

    while (1)
    {
        printf("\nParent -> Child: ");
        fflush(stdout);

        if (fgets(message, sizeof(message), stdin) == NULL)
            break;

        message[strcspn(message, "\n")] = '\0';

        if (send_message(write_fd, message) == -1)
            break;

        if (strcmp(message, "exit") == 0)
            break;

        printf("[PARENT] Waiting for child's reply...\n");

        int result = receive_message(read_fd, message, sizeof(message));

        if (result <= 0)
            break;

        printf("Child -> Parent: %s\n", message);

        if (strcmp(message, "exit") == 0)
            break;
    }

    close(write_fd);
    close(read_fd);

    printf("[PARENT] Communication ended.\n");

    return 0;
}
