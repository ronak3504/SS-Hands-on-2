#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>

int main()
{
    key_t key;
    int msgid;
    char confirm[20];

    // Generate the same key used by Q43 and Q44
    key = ftok("/tmp", 'R');

    if (key == -1)
    {
        perror("ftok");
        return 1;
    }

    // Access the existing queue; do not create a new one
    msgid = msgget(key, 0);

    if (msgid == -1)
    {
        perror("msgget");
        printf("Message queue does not exist.\n");
        return 1;
    }

    printf("Message Queue ID: %d\n", msgid);

    printf("Are you sure you want to remove this queue? (yes/no): ");

    if (scanf("%19s", confirm) != 1)
    {
        printf("Input error.\n");
        return 1;
    }

    if (strcmp(confirm, "yes") != 0)
    {
        printf("Queue removal cancelled.\n");
        return 0;
    }

    // Remove the message queue
    if (msgctl(msgid, IPC_RMID, NULL) == -1)
    {
        perror("msgctl IPC_RMID");
        return 1;
    }

    printf("Message queue removed successfully!\n");

    return 0;
}
