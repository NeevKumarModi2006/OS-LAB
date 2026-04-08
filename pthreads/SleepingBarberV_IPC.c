#include <stdio.h>
#include <unistd.h>
#include <sys/sem.h>
#include <sys/wait.h>

void sem_wait(int id, int n) {
    struct sembuf sb = {n, -1, 0};
    semop(id, &sb, 1);
}

void sem_signal(int id, int n) {
    struct sembuf sb = {n, 1, 0};
    semop(id, &sb, 1);
}

int main() {
    int semid = semget(IPC_PRIVATE, 3, IPC_CREAT | 0666);

    // sem[0] → mutex
    // sem[1] → customers
    // sem[2] → barber
    semctl(semid, 0, SETVAL, 1);
    semctl(semid, 1, SETVAL, 0);
    semctl(semid, 2, SETVAL, 0);

    if (fork() == 0) {
        while (1) {
            sem_wait(semid, 1);
            sem_wait(semid, 0);
            printf("Barber is CUTTING hair\n");
            sem_signal(semid, 2);
            sem_signal(semid, 0);
            sleep(2);
        }
    }

    for (int i = 0; i < 3; i++) {
        if (fork() == 0) {
            sem_wait(semid, 0);
            printf("Customer ARRIVED\n");
            sem_signal(semid, 1);
            sem_signal(semid, 0);
            sem_wait(semid, 2);
            exit(0);
        }
    }

    for (int i = 0; i < 3; i++)
        wait(NULL);

    semctl(semid, 0, IPC_RMID);
    return 0;
}
// Sleeping Barber Problem
// Problem Statement:
// The Sleeping Barber problem describes a classic synchronization scenario in which
// a barber shop has one barber, one barber chair, and a waiting room with a limited
// number of chairs. The barber spends his time either cutting hair or sleeping when
// there are no customers. When a customer arrives, if the barber is sleeping, the
// customer wakes him up. If the barber is busy and waiting chairs are available, the
// customer waits; otherwise, the customer leaves the shop.

// Synchronization Requirements:
// Only one customer can be served by the barber at a time
// The barber sleeps when there are no customers
// Customers must wait if chairs are available
// Customers leave if no waiting chairs are available
// Proper synchronization is required to avoid race conditions
// Mutual exclusion must be ensured while accessing shared variable
