#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

union semun {
    int val;
};

void lock(int semid)
{
    struct sembuf op = {0, -1, 0};

    while (semop(semid, &op, 1) == -1) {
        if (errno != EINTR) {
            perror("semop lock");
            exit(1);
        }
    }
}

void unlock(int semid)
{
    struct sembuf op = {0, 1, 0};

    if (semop(semid, &op, 1) == -1) {
        perror("semop unlock");
        exit(1);
    }
}

int main(int argc, char *argv[])
{
    key_t shmkey = ftok("/tmp", 'V');
    key_t semkey = ftok("/tmp", 'W');

    if (shmkey == -1 || semkey == -1) {
        perror("ftok");
        return 1;
    }

    /* Initialize shared resources once. */
    if (argc == 2 && strcmp(argv[1], "init") == 0) {
        int shmid = shmget(shmkey, sizeof(int),
                           IPC_CREAT | IPC_EXCL | 0600);

        if (shmid == -1) {
            perror("Shared memory may already exist");
            return 1;
        }

        int *shared = shmat(shmid, NULL, 0);

        if (shared == (void *)-1) {
            perror("shmat");
            shmctl(shmid, IPC_RMID, NULL);
            return 1;
        }

        *shared = 0;
        shmdt(shared);

        int semid = semget(semkey, 1, IPC_CREAT | IPC_EXCL | 0600);

        if (semid == -1) {
            perror("semget");
            shmctl(shmid, IPC_RMID, NULL);
            return 1;
        }

        union semun arg;
        arg.val = 1;

        if (semctl(semid, 0, SETVAL, arg) == -1) {
            perror("semctl");
            semctl(semid, 0, IPC_RMID);
            shmctl(shmid, IPC_RMID, NULL);
            return 1;
        }

        printf("Shared variable initialized to 0.\n");
        printf("Semaphore initialized to 1.\n");
        printf("Resources ready for both terminals.\n");
        return 0;
    }

    /* Cleanup after both terminal instances have exited. */
    if (argc == 2 && strcmp(argv[1], "cleanup") == 0) {
        int shmid = shmget(shmkey, sizeof(int), 0600);
        int semid = semget(semkey, 1, 0600);

        if (shmid != -1)
            shmctl(shmid, IPC_RMID, NULL);

        if (semid != -1)
            semctl(semid, 0, IPC_RMID);

        printf("Shared memory and semaphore removed.\n");
        return 0;
    }

    int shmid = shmget(shmkey, sizeof(int), 0600);
    int semid = semget(semkey, 1, 0600);

    if (shmid == -1 || semid == -1) {
        printf("Run ./49b init first.\n");
        return 1;
    }

    int *shared = shmat(shmid, NULL, 0);

    if (shared == (void *)-1) {
        perror("shmat");
        return 1;
    }

    int value;

    printf("\nPID: %ld\n", (long)getpid());
    printf("Current shared value: %d\n", *shared);
    printf("Enter the value you want to write: ");

    if (scanf("%d", &value) != 1) {
        printf("Invalid input.\n");
        shmdt(shared);
        return 1;
    }

    printf("\nPID %ld waiting for semaphore...\n", (long)getpid());
    fflush(stdout);

    lock(semid);

    printf("PID %ld acquired semaphore.\n", (long)getpid());

    /* Critical section: protected shared-memory write. */
    printf("Writing %d to shared memory...\n", value);
    *shared = value;

    printf("Shared value is now: %d\n", *shared);
    printf("Holding the critical section for 10 seconds...\n");

    sleep(10);

    unlock(semid);

    printf("PID %ld released semaphore.\n", (long)getpid());

    shmdt(shared);
    return 0;
}
