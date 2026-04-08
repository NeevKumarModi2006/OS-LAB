#include <stdio.h>
#include <sys/types.h>
#include "windows_compat.h"

sem_t forks[5];

void* philosopher(void* arg){
    int id = *((int *)arg);
    
    // Define Left and Right fork indices
    int left = id;
    int right = (id + 1) % 5;

    while(1){ 
        printf("Philosopher %d is thinking...\n", id);
        
        // --- DEADLOCK PREVENTION (Odd/Even Strategy) ---
        if (id % 2 == 0) {
            // Even philosophers pick LEFT first, then RIGHT
            sem_wait(&forks[left]);
            sem_wait(&forks[right]);
        } else {
            // Odd philosophers pick RIGHT first, then LEFT
            sem_wait(&forks[right]);
            sem_wait(&forks[left]);
        }
        // -----------------------------------------------

        printf("Philosopher %d is EATING with forks %d and %d\n", id, left, right);
        Sleep(1); // Eat for 1 second

        // Put down forks (Order doesn't matter here)
        sem_post(&forks[left]);
        sem_post(&forks[right]);
        
        Sleep(1000); // Think for a bit before eating again
    }
}

int main(){
    pthread_t p[5];
    int ids[5] = {0, 1, 2, 3, 4};

    // Initialize semaphores
    for(int i=0; i<5; i++){
        sem_init(&forks[i], 0, 1);
    }

    // Create threads
    for(int i=0; i<5; i++){
        pthread_create(&p[i], NULL, philosopher, &ids[i]);
    }

    // Join threads (Infinite wait)
    for(int i=0; i<5; i++){
        pthread_join(p[i], NULL);
    }
    
    return 0;
}