#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
    struct message msg;
    int choice;

    // Generate a consistent key shared by Q43 and Q44
    key = ftok("/tmp", 'R');

    if (key == -1)
    {
        perror("ftok");
        return 1;
    }

    // Create the queue if it does not exist
    msgid = msgget(key, IPC_CREAT | 0666);

    if (msgid == -1)
    {
        perror("msgget");
        return 1;
    }

    printf("Message Queue ID: %d\n", msgid);

    while (1)
    {
        printf("\n--- Q43: Send Messages ---\n");
        printf("1. Send a message\n");
        printf("2. Exit\n");
        printf("Enter choice: ");

        if (scanf("%d", &choice) != 1)
        {
            printf("Invalid input.\n");
            break;
        }

        getchar();

        if (choice == 1)
        {
            printf("Enter message type (positive integer): ");

            if (scanf("%ld", &msg.type) != 1 || msg.type <= 0)
            {
                printf("Message type must be a positive integer.\n");
                break;
            }

            getchar();

            printf("Enter message text: ");

            if (fgets(msg.text, SIZE, stdin) == NULL)
            {
                printf("Unable to read message.\n");
                break;
            }

            msg.text[strcspn(msg.text, "\n")] = '\0';

            if (msgsnd(msgid, &msg, strlen(msg.text) + 1, 0) == -1)
            {
                perror("msgsnd");
            }
            else
            {
                printf("Message sent successfully!\n");
            }
        }
        else if (choice == 2)
        {
            printf("Exiting. Message queue has been retained.\n");
            break;
        }
        else
        {
            printf("Invalid choice.\n");
        }
    }

    return 0;
}
