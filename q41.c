#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>

int main(void)
{
    key_t key;
    int msgid;
    char choice;

    printf("=== Q41: Message Queue Creation ===\n\n");

    /* Generate a key using ftok() */
    key = ftok(".", 'R');

    if (key == (key_t)-1)
    {
        perror("ftok");
        return 1;
    }

    printf("Generated Key: %d (0x%08x)\n",
           (int)key, (unsigned int)key);

    /* Create a message queue or access an existing one */
    msgid = msgget(key, IPC_CREAT | IPC_EXCL | 0666);

    if (msgid == -1)
    {
        if (errno == EEXIST)
        {
            printf("Queue already exists. Accessing existing queue...\n");

            msgid = msgget(key, 0666);

            if (msgid == -1)
            {
                perror("msgget");
                return 1;
            }
        }
        else
        {
            perror("msgget");
            return 1;
        }
    }
    else
    {
        printf("New message queue created successfully!\n");
    }

    printf("Message Queue ID: %d\n", msgid);

    printf("\n1. Remove the message queue\n");
    printf("2. Keep the message queue\n");
    printf("Choose an option (1/2): ");
    fflush(stdout);

    if (scanf(" %c", &choice) != 1)
    {
        fprintf(stderr, "Invalid input.\n");
        return 1;
    }

    if (choice == '1')
    {
        if (msgctl(msgid, IPC_RMID, NULL) == -1)
        {
            perror("msgctl");
            return 1;
        }

        printf("Message queue removed successfully.\n");
    }
    else if (choice == '2')
    {
        printf("Message queue retained in the system.\n");
        printf("It can be accessed again using the same key.\n");
    }
    else
    {
        printf("Invalid choice. Message queue retained.\n");
    }

    printf("Program completed.\n");

    return 0;
}
