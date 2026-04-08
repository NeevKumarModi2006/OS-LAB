#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/wait.h>

#define M_READERS 5
#define N_WRITERS 3

/* Semaphore operations helper functions */
void sem_wait(int semid, int num) {
    struct sembuf sb = {num, -1, 0};
    semop(semid, &sb, 1);
}

void sem_signal(int semid, int num) {
    struct sembuf sb = {num, 1, 0};
    semop(semid, &sb, 1);
}

int main() {
    /* 1. Create shared memory for read_count */
    // Using IPC_PRIVATE to create a unique segment for this parent/child group
    int shmid = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666);
    int *read_count = (int *)shmat(shmid, NULL, 0);
    *read_count = 0;

    /* 2. Create a semaphore set with 2 semaphores */
    // sem[0] -> mutex (protects the read_count variable)
    // sem[1] -> wrt   (provides exclusive access for writers)
    int semid = semget(IPC_PRIVATE, 2, IPC_CREAT | 0666);

    /* 3. Initialize semaphores */
    semctl(semid, 0, SETVAL, 1); // mutex = 1
    semctl(semid, 1, SETVAL, 1); // wrt = 1

    /* 4. Create Reader Processes */
    for (int i = 0; i < M_READERS; i++) {
        if (fork() == 0) {
            // Entry Section
            sem_wait(semid, 0);           // Lock mutex to update read_count
            (*read_count)++;
            if (*read_count == 1)
                sem_wait(semid, 1);       // First reader blocks writers
            sem_signal(semid, 0);         // Release mutex

            // Critical Section
            printf("Reader %d (PID %d) is READING. Active readers: %d\n", i + 1, getpid(), *read_count);
            sleep(1);

            // Exit Section
            sem_wait(semid, 0);           // Lock mutex to update read_count
            (*read_count)--;
            if (*read_count == 0)
                sem_signal(semid, 1);     // Last reader releases writers
            sem_signal(semid, 0);         // Release mutex

            exit(0);
        }
    }

    /* 5. Create Writer Processes */
    for (int i = 0; i < N_WRITERS; i++) {
        if (fork() == 0) {
            // Entry Section
            sem_wait(semid, 1);           // Lock wrt (exclusive access)

            // Critical Section
            printf("Writer %d (PID %d) is WRITING\n", i + 1, getpid());
            sleep(0.01);

            // Exit Section
            sem_signal(semid, 1);         // Release wrt

            exit(0);
        }
    }

    /* 6. Wait for all child processes to finish */
    for (int i = 0; i < M_READERS + N_WRITERS; i++)
        wait(NULL);

    /* 7. Cleanup Resources */
    shmdt(read_count);
    shmctl(shmid, IPC_RMID, NULL);
    semctl(semid, 0, IPC_RMID);

    printf("Main process: Simulation finished and resources cleaned up.\n");
    return 0;
}

// Readers–Writers Problem (System V IPC)
// Problem Statement:
// The Readers–Writers problem involves a shared resource accessed by multiple reader
// and writer processes. Readers only read the data, while writers modify it. Multiple
// readers may read concurrently, but writers must have exclusive access.

// Synchronization Requirements:
// Supports M reader processes and N writer processes
// Multiple readers can read simultaneously
// Writers must have exclusive access to the shared resource
// read_count is maintained in shared memory
// Semaphores ensure mutual exclusion and prevent deadlock
