#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 5053
#define BUFFER_SIZE 1024

int main(void)
{
    int sock_fd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    // Create TCP socket
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (sock_fd == -1) {
        perror("socket");
        return 1;
    }

    // Configure server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1",
                  &server_addr.sin_addr) != 1) {
        fprintf(stderr, "Invalid server IP address.\n");
        close(sock_fd);
        return 1;
    }

    // Connect to pthread server
    if (connect(sock_fd, (struct sockaddr *)&server_addr,
                sizeof(server_addr)) == -1) {
        perror("connect");
        close(sock_fd);
        return 1;
    }

    printf("Connected to pthread server on port %d.\n", PORT);
    printf("Type messages to send. Type 'exit' to disconnect.\n");

    while (1) {
        printf("\nYou: ");
        fflush(stdout);

        if (fgets(buffer, sizeof(buffer), stdin) == NULL)
            break;

        // Remove newline
        buffer[strcspn(buffer, "\n")] = '\0';

        // Send message to server
        if (send(sock_fd, buffer, strlen(buffer), 0) == -1) {
            perror("send");
            break;
        }

        if (strcmp(buffer, "exit") == 0)
            break;
    }

    close(sock_fd);
    printf("Disconnected from server.\n");

    return 0;
}
