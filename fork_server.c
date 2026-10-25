#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/wait.h>

#define PORT 5052
#define BUFFER_SIZE 1024

int main(void)
{
    int server_fd, client_fd;
    int opt = 1;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    char buffer[BUFFER_SIZE];

    // Reap child processes automatically
    signal(SIGCHLD, SIG_IGN);

    // Create TCP socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket");
        return 1;
    }

    // Allow port reuse
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

    // Listen for connections
    if (listen(server_fd, 10) == -1) {
        perror("listen");
        close(server_fd);
        return 1;
    }

    printf("====================================\n");
    printf("   CONCURRENT SERVER USING fork()\n");
    printf("====================================\n");
    printf("Listening on port %d...\n", PORT);
    printf("Waiting for clients...\n");
    fflush(stdout);

    while (1) {
        client_len = sizeof(client_addr);

        // Accept a client
        client_fd = accept(server_fd,
                           (struct sockaddr *)&client_addr,
                           &client_len);

        if (client_fd == -1) {
            if (errno == EINTR)
                continue;

            perror("accept");
            continue;
        }

        printf("\nClient connected: %s:%d\n",
               inet_ntoa(client_addr.sin_addr),
               ntohs(client_addr.sin_port));
        fflush(stdout);

        // Create a child process for the client
        pid_t pid = fork();

        if (pid < 0) {
            perror("fork");
            close(client_fd);
            continue;
        }

        if (pid == 0) {
            // CHILD PROCESS: handles this client
            close(server_fd);

            printf("[Child PID %d] Handling client.\n", getpid());
            fflush(stdout);

            while (1) {
                ssize_t n = recv(client_fd, buffer,
                                 BUFFER_SIZE - 1, 0);

                if (n == 0) {
                    printf("[Child PID %d] Client disconnected.\n",
                           getpid());
                    break;
                }

                if (n < 0) {
                    perror("recv");
                    break;
                }

                buffer[n] = '\0';

                printf("[Child PID %d] Received: %s\n",
                       getpid(), buffer);
                fflush(stdout);

                if (strcmp(buffer, "exit") == 0) {
                    printf("[Child PID %d] Client sent exit.\n",
                           getpid());
                    break;
                }
            }

            close(client_fd);
            _exit(0);
        }

        // PARENT PROCESS: continues accepting clients
        close(client_fd);

        printf("[Parent PID %d] Created child PID %d.\n",
               getpid(), pid);
        printf("Waiting for the next client...\n");
        fflush(stdout);
    }

    close(server_fd);
    return 0;
}
