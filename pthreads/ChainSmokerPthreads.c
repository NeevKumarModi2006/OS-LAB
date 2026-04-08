#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

pthread_mutex_t mutex;
pthread_cond_t smoker[3], agent;
int table = -1; 

void* agent_func(void* arg) {
    while (1) {
        pthread_mutex_lock(&mutex);
        while (table != -1) pthread_cond_wait(&agent, &mutex); // Wait for table to clear
        
        table = rand() % 3; // 0: Tobacco, 1: Paper, 2: Matches
        printf("Agent places items for smoker %d\n", table);
        
        pthread_cond_signal(&smoker[table]);
        pthread_mutex_unlock(&mutex);
        sleep(1);
    }
}

void* smoker_func(void* arg) {
    int id = *(int*)arg;
    while (1) {
        pthread_mutex_lock(&mutex);
        while (table != id)
            pthread_cond_wait(&smoker[id], &mutex);

        printf("Smoker %d is SMOKING\n", id);
        table = -1; // Clear the table
        pthread_cond_signal(&agent); // Signal agent to put more
        pthread_mutex_unlock(&mutex);
        usleep(500000); 
    }
}

int main() {
    pthread_t a, s[3];
    int ids[3] = {0, 1, 2};
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&agent, NULL);
    for (int i = 0; i < 3; i++) pthread_cond_init(&smoker[i], NULL);

    pthread_create(&a, NULL, agent_func, NULL);
    for (int i = 0; i < 3; i++) pthread_create(&s[i], NULL, smoker_func, &ids[i]);

    pthread_join(a, NULL);
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
