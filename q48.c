#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>

union semun
{
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

int main()
{
    int semid, type, value, choice;
    union semun arg;

    printf("========== SEMAPHORE LAB ==========\n");
    printf("1. Create Binary Semaphore\n");
    printf("2. Create Counting Semaphore\n");
    printf("Choose semaphore type: ");

    if (scanf("%d", &type) != 1 || (type != 1 && type != 2))
    {
        printf("Invalid choice.\n");
        return 1;
    }

    printf("Enter initial value: ");

    if (scanf("%d", &value) != 1 || value < 0)
    {
        printf("Invalid value.\n");
        return 1;
    }

    if (type == 1 && value > 1)
    {
        printf("Binary semaphore value must be 0 or 1.\n");
        return 1;
    }

    // Create a new semaphore with a unique ID
    semid = semget(IPC_PRIVATE, 1, IPC_CREAT | 0600);

    if (semid == -1)
    {
        perror("semget");
        return 1;
    }

    printf("\nSemaphore created successfully!\n");
    printf("Semaphore ID: %d\n", semid);

    // Initialize semaphore
    arg.val = value;

    if (semctl(semid, 0, SETVAL, arg) == -1)
    {
        perror("semctl SETVAL");
        semctl(semid, 0, IPC_RMID);
        return 1;
    }

    printf("Semaphore initialized to %d.\n", value);

    while (1)
    {
        printf("\n========== MENU ==========\n");
        printf("1. Display current value\n");
        printf("2. Reinitialize semaphore\n");
        printf("3. Remove semaphore\n");
        printf("Enter choice: ");

        if (scanf("%d", &choice) != 1)
        {
            printf("Invalid input.\n");
            semctl(semid, 0, IPC_RMID);
            return 1;
        }

        if (choice == 1)
        {
            value = semctl(semid, 0, GETVAL);

            if (value == -1)
                perror("semctl GETVAL");
            else
                printf("Current semaphore value: %d\n", value);
        }
        else if (choice == 2)
        {
            printf("Enter new initial value: ");

            if (scanf("%d", &value) != 1 || value < 0 ||
                (type == 1 && value > 1))
            {
                printf("Invalid value for this semaphore type.\n");
                return 1;
            }

            arg.val = value;

            if (semctl(semid, 0, SETVAL, arg) == -1)
                perror("semctl SETVAL");
            else
                printf("Semaphore reinitialized to %d.\n", value);
        }
        else if (choice == 3)
        {
            if (semctl(semid, 0, IPC_RMID) == -1)
                perror("semctl IPC_RMID");
            else
                printf("Semaphore removed successfully!\n");

            break;
        }
        else
        {
            printf("Invalid choice.\n");
        }
    }

    return 0;
}
