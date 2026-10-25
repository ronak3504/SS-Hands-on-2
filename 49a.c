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
            perror("semop");
            exit(1);
        }
    }
}

void unlock(int semid)
{
    struct sembuf op = {0, 1, 0};

    if (semop(semid, &op, 1) == -1) {
        perror("semop");
        exit(1);
    }
}

int main(int argc, char *argv[])
{
    key_t shmkey = ftok("/tmp", 'T');
    key_t semkey = ftok("/tmp", 'U');

    if (shmkey == -1 || semkey == -1) {
        perror("ftok");
        return 1;
    }

    /* Initialize shared resources once. */
    if (argc == 2 && strcmp(argv[1], "init") == 0) {
        int shmid = shmget(shmkey, sizeof(int),
                           IPC_CREAT | IPC_EXCL | 0600);

        if (shmid == -1) {
            perror("shmget: resources may already exist");
            return 1;
        }

        int *counter = shmat(shmid, NULL, 0);

        if (counter == (void *)-1) {
            perror("shmat");
            shmctl(shmid, IPC_RMID, NULL);
            return 1;
        }

        *counter = 0;
        shmdt(counter);

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

        printf("Shared counter initialized to 0.\n");
        printf("Semaphore initialized to 1.\n");
        printf("Resources ready for both terminals.\n");
        return 0;
    }

    /* Remove resources after both terminals have exited. */
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

    /* Connect to the existing shared resources. */
    int shmid = shmget(shmkey, sizeof(int), 0600);
    int semid = semget(semkey, 1, 0600);

    if (shmid == -1 || semid == -1) {
        printf("Resources not initialized. Run ./49a init first.\n");
        return 1;
    }

    int *counter = shmat(shmid, NULL, 0);

    if (counter == (void *)-1) {
        perror("shmat");
        return 1;
    }

    printf("\n=== Ticket Generation ===\n");
    printf("Process ID: %ld\n", (long)getpid());
    printf("Press ENTER to request a ticket; type q to quit.\n");

    char input[32];

    while (1) {
        printf("\nYour choice: ");
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL)
            break;

        if (input[0] == 'q' || input[0] == 'Q')
            break;

        printf("PID %ld waiting for semaphore...\n",
               (long)getpid());

        lock(semid);

        printf("PID %ld acquired semaphore.\n",
               (long)getpid());

        (*counter)++;

        printf("Ticket number generated: %d\n", *counter);
        printf("Holding critical section for 10 seconds...\n");

        sleep(10);

        unlock(semid);

        printf("PID %ld released semaphore.\n",
               (long)getpid());
    }

    shmdt(counter);
    printf("Process exiting.\n");

    return 0;
}
