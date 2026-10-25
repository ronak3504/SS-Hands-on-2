#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/types.h>

#define SHM_SIZE 1024

int main()
{
    key_t key;
    int shmid;
    char *data;
    int choice;

    key = ftok("/tmp", 'S');

    if (key == -1)
    {
        perror("ftok");
        return 1;
    }

    shmid = shmget(key, SHM_SIZE, IPC_CREAT | 0666);

    if (shmid == -1)
    {
        perror("shmget");
        return 1;
    }

    printf("Shared Memory ID: %d\n", shmid);

    while (1)
    {
        printf("\n========== SHARED MEMORY LAB ==========\n");
        printf("1. Write data to shared memory\n");
        printf("2. Read data using SHM_RDONLY\n");
        printf("3. Attempt overwrite on read-only memory\n");
        printf("4. Detach shared memory\n");
        printf("5. Remove shared memory\n");
        printf("6. Exit (retain shared memory)\n");
        printf("Enter choice: ");

        if (scanf("%d", &choice) != 1)
        {
            printf("Invalid input.\n");
            break;
        }

        getchar();

        if (choice == 1)
        {
            data = (char *)shmat(shmid, NULL, 0);

            if (data == (char *)-1)
            {
                perror("shmat");
                continue;
            }

            char input[SHM_SIZE];

            printf("Enter data to store: ");

            if (fgets(input, sizeof(input), stdin) == NULL)
            {
                shmdt(data);
                continue;
            }

            input[strcspn(input, "\n")] = '\0';

            strcpy(data, input);

            printf("Stored: %s\n", data);

            if (shmdt(data) == -1)
                perror("shmdt");
            else
                printf("Detached after writing.\n");
        }
        else if (choice == 2)
        {
            data = (char *)shmat(shmid, NULL, SHM_RDONLY);

            if (data == (char *)-1)
            {
                perror("shmat");
                continue;
            }

            printf("Read-only attachment successful.\n");
            printf("Shared memory contains: %s\n", data);

            if (shmdt(data) == -1)
                perror("shmdt");
            else
                printf("Detached successfully.\n");
        }
        else if (choice == 3)
        {
            data = (char *)shmat(shmid, NULL, SHM_RDONLY);

            if (data == (char *)-1)
            {
                perror("shmat");
                continue;
            }

            printf("Creating a child process to test writing...\n");

            pid_t pid = fork();

            if (pid == -1)
            {
                perror("fork");
                shmdt(data);
                continue;
            }

            if (pid == 0)
            {
                // Child deliberately attempts an illegal write.
                data[0] = 'X';
                _exit(0);
            }

            int status;

            if (waitpid(pid, &status, 0) == -1)
            {
                perror("waitpid");
            }
            else if (WIFSIGNALED(status))
            {
                printf("Child terminated by signal %d.\n",
                       WTERMSIG(status));
                printf("The read-only write was rejected by the OS.\n");
            }
            else if (WIFEXITED(status))
            {
                printf("Child exited with status %d.\n",
                       WEXITSTATUS(status));
                printf("Check the shared data to confirm its contents.\n");
            }

            shmdt(data);
            printf("Parent process is still running.\n");
        }
        else if (choice == 4)
        {
            printf("Each attachment is detached after its operation.\n");
            printf("No persistent attachment remains in this program.\n");
        }
        else if (choice == 5)
        {
            if (shmctl(shmid, IPC_RMID, NULL) == -1)
            {
                perror("shmctl IPC_RMID");
            }
            else
            {
                printf("Shared memory marked for removal.\n");
                printf("Exiting because this segment is being removed.\n");
                break;
            }
        }
        else if (choice == 6)
        {
            printf("Exiting without removing shared memory.\n");
            break;
        }
        else
        {
            printf("Invalid choice.\n");
        }
    }

    return 0;
}
