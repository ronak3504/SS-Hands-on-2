#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define SIZE 100

struct message
{
    long type;
    char text[SIZE];
};

int main()
{
    key_t key;
    int msgid;
    int choice;
    long type;
    struct message msg;
    ssize_t bytes;

    // Use the same key as Q43
    key = ftok("/tmp", 'R');

    if (key == -1)
    {
        perror("ftok");
        return 1;
    }

    // Access the existing queue or create it if absent
    msgid = msgget(key, IPC_CREAT | 0666);

    if (msgid == -1)
    {
        perror("msgget");
        return 1;
    }

    printf("Message Queue ID: %d\n", msgid);

    while (1)
    {
        printf("\n--- Q44: Receive Messages ---\n");
        printf("1. Receive with flag 0 (blocking)\n");
        printf("2. Receive with IPC_NOWAIT (non-blocking)\n");
        printf("3. Exit\n");
        printf("Enter choice: ");

        if (scanf("%d", &choice) != 1)
        {
            printf("Invalid input.\n");
            break;
        }

        if (choice == 3)
        {
            printf("Exiting. Message queue retained.\n");
            break;
        }

        if (choice != 1 && choice != 2)
        {
            printf("Invalid choice.\n");
            continue;
        }

        printf("Enter message type (0 = any type): ");

        if (scanf("%ld", &type) != 1 || type < 0)
        {
            printf("Invalid message type.\n");
            break;
        }

        int flag;

        if (choice == 1)
            flag = 0;
        else
            flag = IPC_NOWAIT;

        printf("\nReceiving message...\n");

        bytes = msgrcv(msgid, &msg, sizeof(msg.text),
                       type, flag);

        if (bytes == -1)
        {
            if (errno == ENOMSG)
                printf("No matching message available. Try again later.\n");
            else
                perror("msgrcv");
        }
        else
        {
            msg.text[SIZE - 1] = '\0';

            printf("Message received successfully!\n");
            printf("Message type: %ld\n", msg.type);
            printf("Message text: %s\n", msg.text);
        }
    }

    return 0;
}
