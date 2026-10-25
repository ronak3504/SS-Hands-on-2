#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>

int main()
{
    key_t key;
    int msgid;
    struct msqid_ds info;
    int new_perm;

    // Access the same message queue used by Q43 and Q44
    key = ftok("/tmp", 'R');

    if (key == -1)
    {
        perror("ftok");
        return 1;
    }

    msgid = msgget(key, 0666);

    if (msgid == -1)
    {
        perror("msgget");
        return 1;
    }

    printf("Message Queue ID: %d\n", msgid);

    // Get current queue information
    if (msgctl(msgid, IPC_STAT, &info) == -1)
    {
        perror("msgctl IPC_STAT");
        return 1;
    }

    printf("Current permissions: %04o\n",
           info.msg_perm.mode & 0777);

    printf("\nEnter new permissions in octal (e.g. 0600): ");

    if (scanf("%o", &new_perm) != 1 ||
        new_perm < 0 || new_perm > 0777)
    {
        printf("Invalid permissions.\n");
        return 1;
    }

    // Set the new permissions
    info.msg_perm.mode =
        (info.msg_perm.mode & ~0777) | new_perm;

    if (msgctl(msgid, IPC_SET, &info) == -1)
    {
        perror("msgctl IPC_SET");
        return 1;
    }

    printf("Permissions changed successfully!\n");

    // Verify the change
    if (msgctl(msgid, IPC_STAT, &info) == -1)
    {
        perror("msgctl IPC_STAT");
        return 1;
    }

    printf("Updated permissions: %04o\n",
           info.msg_perm.mode & 0777);

    return 0;
}
