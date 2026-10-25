#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/sem.h>

union semun {
    int val;
};

int main(int argc, char *argv[])
{
    key_t key = ftok("/tmp", 'X');

    if (key == -1) {
        perror("ftok");
        return 1;
    }

    /* Initialize the counting semaphore to 2. */
    if (argc == 2 && strcmp(argv[1], "init") == 0) {
        int semid = semget(key, 1, IPC_CREAT | IPC_EXCL | 0600);

        if (semid == -1) {
            perror("Semaphore may already exist");
            return 1;
        }

        union semun arg;
        arg.val = 2;

        if (semctl(semid, 0, SETVAL, arg) == -1) {
            perror("semctl");
            semctl(semid, 0, IPC_RMID);
            return 1;
        }

        printf("Counting semaphore initialized to 2.\n");
        printf("Two resources are available.\n");
        return 0;
    }

    /* Remove the semaphore after all processes finish. */
    if (argc == 2 && strcmp(argv[1], "cleanup") == 0) {
        int semid = semget(key, 1, 0600);

        if (semid == -1) {
            perror("semget");
            return 1;
        }

        if (semctl(semid, 0, IPC_RMID) == -1) {
            perror("semctl IPC_RMID");
            return 1;
        }

        printf("Counting semaphore removed.\n");
        return 0;
    }

    int semid = semget(key, 1, 0600);

    if (semid == -1) {
        printf("Initialize first using ./49c init\n");
        return 1;
    }

    printf("\nPID %ld: Resource Request\n", (long)getpid());
    printf("Press ENTER to request a resource: ");
    fflush(stdout);

    getchar();

    printf("PID %ld: Waiting for a resource...\n",
           (long)getpid());
    fflush(stdout);

    /* Wait for one available resource. */
    struct sembuf acquire = {0, -1, 0};

    while (semop(semid, &acquire, 1) == -1) {
        if (errno == EINTR)
            continue;

        perror("semop acquire");
        return 1;
    }

    printf("PID %ld: Resource ACQUIRED!\n", (long)getpid());
    printf("PID %ld: Using resource for 10 seconds...\n",
           (long)getpid());

    sleep(10);

    /* Release the resource. */
    struct sembuf release = {0, 1, 0};

    if (semop(semid, &release, 1) == -1) {
        perror("semop release");
        return 1;
    }

    printf("PID %ld: Resource RELEASED!\n", (long)getpid());

    return 0;
}
