#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/wait.h>

#define N 5

void sem_wait(int semid, int num) {
    struct sembuf sb = {num, -1, 0};
    semop(semid, &sb, 1);
}

void sem_signal(int semid, int num) {
    struct sembuf sb = {num, 1, 0};
    semop(semid, &sb, 1);
}

int main() {
    int semid = semget(IPC_PRIVATE, N, IPC_CREAT | 0666);
    for (int i = 0; i < N; i++) semctl(semid, i, SETVAL, 1);

    for (int i = 0; i < N; i++) {
        if (fork() == 0) {
            int left = i;
            int right = (i + 1) % N;
            
            // Deadlock prevention: Odd philosophers pick right then left
            if (i % 2 == 0) {
                sem_wait(semid, left);
                sem_wait(semid, right);
            } else {
                sem_wait(semid, right);
                sem_wait(semid, left);
            }

            printf("Philosopher %d is eating\n", i);
            sleep(1);

            sem_signal(semid, left);
            sem_signal(semid, right);
            printf("Philosopher %d finished\n", i);
            exit(0);
        }
    }

    for (int i = 0; i < N; i++) wait(NULL);
    semctl(semid, 0, IPC_RMID);
    return 0;
}


// 2. Dining Philosophers Problem
// Problem Statement:
// Five philosophers sit around a circular table with five plates of spaghetti and 
// five forks (one between each pair). A philosopher alternates between thinking and eating.
//  To eat, a philosopher needs both adjacent forks (left and right).

// Synchronization Requirements:
// Acquire both forks before eating
// Release both forks after eating
// Avoid deadlock - All philosophers cannot simultaneously hold one fork and wait for the other
// Avoid starvation - No philosopher waits indefinitely