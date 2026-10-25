#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 5053
#define BUFFER_SIZE 1024

void *handle_client(void *arg)
{
    int client_fd = *(int *)arg;
    free(arg);

    char buffer[BUFFER_SIZE];

    printf("[Thread %lu] Client connected.\n",
           (unsigned long)pthread_self());

    while (1) {
        ssize_t n = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);

        if (n == 0) {
            printf("[Thread %lu] Client disconnected.\n",
                   (unsigned long)pthread_self());
            break;
        }

        if (n < 0) {
            perror("recv");
            break;
        }

        buffer[n] = '\0';

        printf("[Thread %lu] Received: %s\n",
               (unsigned long)pthread_self(), buffer);
        fflush(stdout);

        if (strcmp(buffer, "exit") == 0) {
            printf("[Thread %lu] Client sent exit.\n",
                   (unsigned long)pthread_self());
            break;
        }
    }

    close(client_fd);
    return NULL;
}

int main(void)
{
    int server_fd;
    int opt = 1;
    struct sockaddr_in server_addr, client_addr;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd == -1) {
        perror("socket");
        return 1;
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR,
                   &opt, sizeof(opt)) == -1) {
        perror("setsockopt");
        close(server_fd);
        return 1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&server_addr,
             sizeof(server_addr)) == -1) {
        perror("bind");
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, 10) == -1) {
        perror("listen");
        close(server_fd);
        return 1;
    }

    printf("====================================\n");
    printf(" CONCURRENT SERVER USING pthreads\n");
    printf("====================================\n");
    printf("Listening on port %d...\n", PORT);
    printf("Waiting for clients...\n");

    while (1) {
        socklen_t client_len = sizeof(client_addr);

        int client_fd = accept(server_fd,
                               (struct sockaddr *)&client_addr,
                               &client_len);

        if (client_fd == -1) {
            perror("accept");
            continue;
        }

        printf("\nClient connected: %s:%d\n",
               inet_ntoa(client_addr.sin_addr),
               ntohs(client_addr.sin_port));

        int *client_ptr = malloc(sizeof(int));

        if (client_ptr == NULL) {
            perror("malloc");
            close(client_fd);
            continue;
        }

        *client_ptr = client_fd;

        pthread_t thread;

        if (pthread_create(&thread, NULL, handle_client,
                           client_ptr) != 0) {
            fprintf(stderr, "Failed to create thread.\n");
            close(client_fd);
            free(client_ptr);
            continue;
        }

        // Release thread resources automatically when it finishes
        pthread_detach(thread);
    }

    close(server_fd);
    return 0;
}
