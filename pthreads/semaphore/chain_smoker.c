#include <stdio.h>
#include <stdlib.h>
#include "my_pthread.h"
#include <semaphore.h>
#include <unistd.h>

// Ingredients: 0 = tobacco, 1 = paper, 2 = matches

sem_t smoker_sem[3];   // One semaphore per smoker
sem_t agent_sem;       // Controls when agent can place ingredients
pthread_mutex_t table_mutex;

int table_ingredient1 = -1;
int table_ingredient2 = -1;

void place_ingredients(int ing1, int ing2) {
    table_ingredient1 = ing1;
    table_ingredient2 = ing2;
    printf("[Agent] Placed %d and %d on table.\n", ing1, ing2);
}

void clear_table() {
    table_ingredient1 = -1;
    table_ingredient2 = -1;
}

// Each smoker has one ingredient and waits for the other two.
void* smoker(void* arg) {
    int id = (int)(long)arg; // 0: has tobacco, 1: has paper, 2: has matches

    while (1) {
        sem_wait(&smoker_sem[id]);  // Wait until the agent gives the other two ingredients

        pthread_mutex_lock(&table_mutex);
        printf("[Smoker %d] Making and smoking cigarette.\n", id);
        clear_table();
        pthread_mutex_unlock(&table_mutex);

        sleep(1);                   // Smoking time

        sem_post(&agent_sem);       // Notify agent that the table is free
    }
    return NULL;
}

void* agent(void* arg) {
    (void)arg;
    while (1) {
        sem_wait(&agent_sem);   // Wait until table is free

        pthread_mutex_lock(&table_mutex);

        int choice = rand() % 3;
        switch (choice) {
            case 0: // Give paper and matches to smoker with tobacco (id 0)
                place_ingredients(1, 2);
                sem_post(&smoker_sem[0]);
                break;
            case 1: // Give tobacco and matches to smoker with paper (id 1)
                place_ingredients(0, 2);
                sem_post(&smoker_sem[1]);
                break;
            case 2: // Give tobacco and paper to smoker with matches (id 2)
                place_ingredients(0, 1);
                sem_post(&smoker_sem[2]);
                break;
        }

        pthread_mutex_unlock(&table_mutex);

        // Agent will wait for smoker to finish and post agent_sem again
    }
    return NULL;
}

int main() {
    pthread_t smoker_threads[3];
    pthread_t agent_thread;

    pthread_mutex_init(&table_mutex, NULL);
    sem_init(&agent_sem, 0, 1);  // Start with table free

    for (int i = 0; i < 3; i++) {
        sem_init(&smoker_sem[i], 0, 0);
    }

    for (long i = 0; i < 3; i++) {
        pthread_create(&smoker_threads[i], NULL, smoker, (void*)i);
    }

    pthread_create(&agent_thread, NULL, agent, NULL);

    for (int i = 0; i < 3; i++) {
        pthread_join(smoker_threads[i], NULL);
    }
    pthread_join(agent_thread, NULL);

    for (int i = 0; i < 3; i++) {
        sem_destroy(&smoker_sem[i]);
    }
    sem_destroy(&agent_sem);
    pthread_mutex_destroy(&table_mutex);

    return 0;
}