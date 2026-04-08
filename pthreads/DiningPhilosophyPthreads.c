#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_PHILOSOPHERS 5
#define THINKING 0
#define HUNGRY 1
#define EATING 2

int state[NUM_PHILOSOPHERS];
pthread_mutex_t mutex;
pthread_cond_t cond[NUM_PHILOSOPHERS];

void test(int i) {
    if (state[i] == HUNGRY && 
        state[(i + 4) % NUM_PHILOSOPHERS] != EATING &&
        state[(i + 1) % NUM_PHILOSOPHERS] != EATING) {
        state[i] = EATING;
        pthread_cond_signal(&cond[i]);
    }
}

void pickup_forks(int i) {
    pthread_mutex_lock(&mutex);
    state[i] = HUNGRY;
    printf("Philosopher %d is HUNGRY\n", i);
    test(i);
    while (state[i] != EATING) {
        pthread_cond_wait(&cond[i], &mutex);
    }
    pthread_mutex_unlock(&mutex);
}

void return_forks(int i) {
    pthread_mutex_lock(&mutex);
    state[i] = THINKING;
    printf("Philosopher %d is THINKING\n", i);
    test((i + 4) % NUM_PHILOSOPHERS);
    test((i + 1) % NUM_PHILOSOPHERS);
    pthread_mutex_unlock(&mutex);
}

void* philosopher(void* arg) {
    int id = *((int*)arg);
    
    for (int i = 0; i < 3; i++) {
        printf("Philosopher %d is THINKING\n", id);
        sleep(rand() % 3);
        
        pickup_forks(id);
        printf("Philosopher %d is EATING\n", id);
        sleep(rand() % 3);
        
        return_forks(id);
    }
    
    return NULL;
}

int main() {
    pthread_t philosophers[NUM_PHILOSOPHERS];
    int ids[NUM_PHILOSOPHERS];
    
    pthread_mutex_init(&mutex, NULL);
    
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        state[i] = THINKING;
        pthread_cond_init(&cond[i], NULL);
        ids[i] = i;
    }
    
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        pthread_create(&philosophers[i], NULL, philosopher, &ids[i]);
    }
    
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        pthread_join(philosophers[i], NULL);
    }
    
    pthread_mutex_destroy(&mutex);
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        pthread_cond_destroy(&cond[i]);
    }
    
    printf("All philosophers finished dining\n");
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