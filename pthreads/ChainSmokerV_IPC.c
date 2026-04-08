#include <stdio.h>
#include <stdlib.h>
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
    // sem[0]: agent, sem[1]: smoker0, sem[2]: smoker1, sem[3]: smoker2
    int semid = semget(IPC_PRIVATE, 4, IPC_CREAT | 0666);
    semctl(semid, 0, SETVAL, 1); // Agent starts
    for (int i = 1; i <= 3; i++) semctl(semid, i, SETVAL, 0);

    if (fork() == 0) { // Agent Process
        while (1) {
            sem_wait(semid, 0); 
            int s = (rand() % 3) + 1; 
            printf("Agent signals smoker %d\n", s - 1);
            sem_signal(semid, s); 
            sleep(1);
        }
    }

    for (int i = 1; i <= 3; i++) {
        if (fork() == 0) { // Smoker Processes
            while (1) {
                sem_wait(semid, i);
                printf("Smoker %d is SMOKING\n", i - 1);
                sleep(1);
                sem_signal(semid, 0); // Tell agent to continue
            }
        }
    }

    wait(NULL);
    semctl(semid, 0, IPC_RMID);
    return 0;
}

// Cigarette Smokers Problem
// Problem Statement:
// The Cigarette Smokers problem consists of one agent and three smokers.
// Each smoker has an infinite supply of one ingredient. The agent places
// two ingredients on the table, and the smoker with the third ingredient
// makes a cigarette and smokes.

// Note:
// This problem is NOT known as the Chain Smoker problem in standard
// Operating Systems textbooks. The correct name is Cigarette Smokers Problem.
