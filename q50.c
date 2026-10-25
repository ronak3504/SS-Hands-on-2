#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/sem.h>

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

int main(int argc, char *argv[])
{
    key_t key = ftok("/tmp", 'D');

    if (key == -1) {
        perror("ftok");
        return 1;
    }

    /* Initialize two semaphores. */
    if (argc == 2 && strcmp(argv[1], "init") == 0) {
        int semid = semget(key, 2, IPC_CREAT | IPC_EXCL | 0600);

        if (semid == -1) {
            perror("Semaphores may already exist");
            return 1;
        }

        unsigned short values[2] = {1, 1};
        union semun arg;
        arg.array = values;

        if (semctl(semid, 0, SETALL, arg) == -1) {
            perror("semctl SETALL");
            semctl(semid, 0, IPC_RMID);
            return 1;
        }

        printf("S1 and S2 initialized to 1.\n");
        printf("Open two terminals and run ./50 in each.\n");
        return 0;
    }

    /* Remove the semaphore set to end the demonstration. */
    if (argc == 2 && strcmp(argv[1], "cleanup") == 0) {
        int semid = semget(key, 2, 0600);

        if (semid == -1) {
            perror("semget");
            return 1;
        }

        if (semctl(semid, 0, IPC_RMID) == -1) {
            perror("semctl IPC_RMID");
            return 1;
        }

        printf("Semaphore set removed. Demonstration terminated.\n");
        return 0;
    }

    int semid = semget(key, 2, 0600);

    if (semid == -1) {
        printf("Initialize first using ./50 init\n");
        return 1;
    }

    int first, second;

    printf("\n=== INTERACTIVE DEADLOCK DEMONSTRATION ===\n");
    printf("Process ID: %ld\n", (long)getpid());

    printf("\nWhich semaphore do you want to acquire first (1 or 2)? ");
    if (scanf("%d", &first) != 1 || (first != 1 && first != 2)) {
        printf("Invalid choice.\n");
        return 1;
    }

    struct sembuf acquire = {first - 1, -1, 0};

    printf("Requesting S%d...\n", first);
    fflush(stdout);

    if (semop(semid, &acquire, 1) == -1) {
        perror("semop");
        return 1;
    }

    printf("S%d acquired successfully!\n", first);

    second = 3 - first;

    printf("\nYou currently hold S%d.\n", first);
    printf("Which semaphore do you want next (S%d)? ", second);
    fflush(stdout);

    int requested;

    if (scanf("%d", &requested) != 1 ||
        requested != second) {
        printf("Please request the remaining semaphore, S%d.\n",
               second);
        printf("Restart this demonstration after cleanup.\n");
        return 1;
    }

    printf("Requesting S%d... Process will wait if it is held.\n",
           second);
    fflush(stdout);

    /* This blocks when the other terminal holds the semaphore. */
    acquire.sem_num = second - 1;

    if (semop(semid, &acquire, 1) == -1) {
        perror("semop");
        return 1;
    }

    printf("S%d acquired successfully!\n", second);
    printf("Both semaphores acquired. No deadlock occurred.\n");

    return 0;
}
