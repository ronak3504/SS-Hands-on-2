#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>

void print_time(const char *label, time_t value)
{
    printf("%s: ", label);

    if (value == 0)
    {
        printf("No operation recorded yet\n");
        return;
    }

    struct tm *tm_info = localtime(&value);

    if (tm_info == NULL)
    {
        printf("Unable to convert time\n");
        return;
    }

    char buffer[100];

    if (strftime(buffer, sizeof(buffer),
                 "%d-%m-%Y %H:%M:%S", tm_info) == 0)
    {
        printf("Unable to format time\n");
        return;
    }

    printf("%s\n", buffer);
}

void display_information(int msgid, int choice)
{
    struct msqid_ds info;

    if (msgctl(msgid, IPC_STAT, &info) == -1)
    {
        perror("msgctl IPC_STAT");
        return;
    }

    if (choice == 0 || choice == 1)
    {
        printf("\n--- Access Permissions ---\n");
        printf("Permission bits: %04o\n",
               (unsigned int)(info.msg_perm.mode & 0777));
        printf("Owner UID: %u\n", (unsigned int)info.msg_perm.uid);
        printf("Owner GID: %u\n", (unsigned int)info.msg_perm.gid);
    }

    if (choice == 0 || choice == 2)
    {
        printf("\n--- Last Message Operations ---\n");
        print_time("Last message sent", info.msg_stime);
        print_time("Last message received", info.msg_rtime);
    }

    if (choice == 0 || choice == 3)
    {
        printf("\n--- Queue Change Time ---\n");
        print_time("Last change", info.msg_ctime);
    }

    if (choice == 0 || choice == 4)
    {
        printf("\n--- Queue Size ---\n");
        printf("Current queued data: %lu bytes\n",
               (unsigned long)info.msg_cbytes);
        printf("Number of messages: %lu\n",
               (unsigned long)info.msg_qnum);
        printf("Maximum bytes allowed: %lu\n",
               (unsigned long)info.msg_qbytes);
    }

    if (choice == 0 || choice == 5)
    {
        printf("\n--- Process IDs ---\n");
        printf("PID of last msgsnd(): %d\n", info.msg_lspid);
        printf("PID of last msgrcv(): %d\n", info.msg_lrpid);
    }
}

int main(void)
{
    key_t key;
    int msgid;
    int choice;

    printf("=== Q42: Message Queue Information ===\n");

    key = ftok(".", 'R');

    if (key == (key_t)-1)
    {
        perror("ftok");
        return 1;
    }

    msgid = msgget(key, IPC_CREAT | 0666);

    if (msgid == -1)
    {
        perror("msgget");
        return 1;
    }

    printf("Message Queue Key: %d\n", (int)key);
    printf("Message Queue ID: %d\n", msgid);

    while (1)
    {
        printf("\n========== MENU ==========\n");
        printf("1. Access permissions and owner IDs\n");
        printf("2. Last message sent/received times\n");
        printf("3. Last queue change time\n");
        printf("4. Queue size and message count\n");
        printf("5. PIDs of msgsnd() and msgrcv()\n");
        printf("6. Display ALL information\n");
        printf("7. Exit\n");
        printf("Enter choice: ");

        if (scanf("%d", &choice) != 1)
        {
            printf("Invalid input. Exiting.\n");
            break;
        }

        if (choice == 7)
        {
            printf("Exiting. Queue is retained in the system.\n");
            break;
        }

        if (choice >= 1 && choice <= 5)
        {
            display_information(msgid, choice);
        }
        else if (choice == 6)
        {
            display_information(msgid, 0);
        }
        else
        {
            printf("Invalid choice. Select 1-7.\n");
        }
    }

    return 0;
}
