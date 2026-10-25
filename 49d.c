#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>

union semun {
    int val;
};

int main(void)
{
    int choice;

    // Create a private semaphore for this demonstration.
    int semid = semget(IPC_PRIVATE, 1, IPC_CREAT | 0600);

    if (semid == -1) {
        perror("semget");
        return 1;
    }

    union semun arg;
    arg.val = 1;

    if (semctl(semid, 0, SETVAL, arg) == -1) {
        perror("semctl SETVAL");
        semctl(semid, 0, IPC_RMID);
        return 1;
    }

    printf("Semaphore created successfully.\n");
    printf("Semaphore ID: %d\n", semid);

    while (1) {
        printf("\n===== SEMAPHORE MENU =====\n");
        printf("1. Display semaphore value\n");
        printf("2. Remove semaphore\n");
        printf("Enter choice: ");

        if (scanf("%d", &choice) != 1) {
            printf("Invalid input.\n");
            semctl(semid, 0, IPC_RMID);
            return 1;
        }

        if (choice == 1) {
            int value = semctl(semid, 0, GETVAL);

            if (value == -1) {
                perror("semctl GETVAL");
            } else {
                printf("Current semaphore value: %d\n", value);
            }
        }
        else if (choice == 2) {
            if (semctl(semid, 0, IPC_RMID) == -1) {
                perror("semctl IPC_RMID");
                return 1;
            }

            printf("Semaphore removed successfully.\n");

            // Verify that the semaphore no longer exists.
            if (semctl(semid, 0, GETVAL) == -1) {
                printf("Verification successful: semaphore no longer exists.\n");
            } else {
                printf("Semaphore still exists.\n");
            }

            break;
        }
        else {
            printf("Invalid choice. Try again.\n");
        }
    }

    return 0;
}
