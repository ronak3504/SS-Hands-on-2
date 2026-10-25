#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 5051
#define BUFFER_SIZE 1024

int main(void)
{
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    char buffer[BUFFER_SIZE];

    // Create TCP socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket");
        return 1;
    }

    // Allow port reuse
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR,
                   &opt, sizeof(opt)) == -1) {
        perror("setsockopt");
        close(server_fd);
        return 1;
    }

    // Configure server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind socket
    if (bind(server_fd, (struct sockaddr *)&server_addr,
             sizeof(server_addr)) == -1) {
        perror("bind");
        close(server_fd);
        return 1;
    }

    // Listen for clients
    if (listen(server_fd, 5) == -1) {
        perror("listen");
        close(server_fd);
        return 1;
    }

    printf("====================================\n");
    printf("       TCP SERVER STARTED\n");
    printf("====================================\n");
    printf("Port: %d\n", PORT);
    printf("Server is running continuously.\n");

    int stop_server = 0;

    while (!stop_server) {

        printf("\nWaiting for a client to connect...\n");
        fflush(stdout);

        client_len = sizeof(client_addr);

        // Wait for a client
        client_fd = accept(server_fd,
                           (struct sockaddr *)&client_addr,
                           &client_len);

        if (client_fd == -1) {
            perror("accept");
            continue;
        }

        printf("\n[CONNECTED] Client: %s:%d\n",
               inet_ntoa(client_addr.sin_addr),
               ntohs(client_addr.sin_port));

        // Communicate with this client
        while (1) {
            ssize_t n = recv(client_fd, buffer,
                             BUFFER_SIZE - 1, 0);

            if (n == 0) {
                printf("[DISCONNECTED] Client left without sending exit.\n");
                break;
            }

            if (n < 0) {
                perror("recv");
                printf("[DISCONNECTED] Connection ended unexpectedly.\n");
                break;
            }

            buffer[n] = '\0';

            printf("Client says: %s\n", buffer);
            fflush(stdout);

            if (strcmp(buffer, "exit") == 0) {
                printf("[DISCONNECTED] Client sent exit.\n");
                printf("Server is still running.\n");
                break;
            }
        }

        // Close only the client connection
        close(client_fd);

        printf("\n------------------------------------\n");
        printf("1. Wait for another client\n");
        printf("2. Stop the server\n");
        printf("------------------------------------\n");

        int choice;

        while (1) {
            printf("Choose an option: ");
            fflush(stdout);

            if (scanf("%d", &choice) != 1) {
                printf("Invalid input. Enter 1 or 2.\n");

                int ch;
                while ((ch = getchar()) != '\n' && ch != EOF)
                    ;

                continue;
            }

            if (choice == 1 || choice == 2)
                break;

            printf("Please enter 1 or 2.\n");
        }

        if (choice == 2) {
            stop_server = 1;
        } else {
            printf("\nServer remains active.\n");
        }
    }

    close(server_fd);

    printf("\n[SERVER STOPPED] Server socket closed.\n");

    return 0;
}
