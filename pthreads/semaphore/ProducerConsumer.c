#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define BUFFER_SIZE 5
#define NUM_PRODUCERS 3
#define NUM_CONSUMERS 3

int buffer[BUFFER_SIZE];
int in_index = 0;
int out_index = 0;

sem_t empty_slots;   // Counts empty slots in buffer
sem_t full_slots;    // Counts full slots in buffer
pthread_mutex_t buf_mutex; // Protects buffer indices

void* producer(void* arg) {
    long id = (long)arg;
    while (1) {
        int item = rand() % 100;

        sem_wait(&empty_slots);          // Wait for an empty slot
        pthread_mutex_lock(&buf_mutex);  // Enter critical section

        buffer[in_index] = item;
        printf("[Producer %ld] Produced %d at %d\n", id, item, in_index);
        in_index = (in_index + 1) % BUFFER_SIZE;

        pthread_mutex_unlock(&buf_mutex); // Exit critical section
        sem_post(&full_slots);           // Signal that a new item is available

        sleep(1);
    }
    return NULL;
}

void* consumer(void* arg) {
    long id = (long)arg;
    while (1) {
        sem_wait(&full_slots);           // Wait for at least one full slot
        pthread_mutex_lock(&buf_mutex);  // Enter critical section

        int item = buffer[out_index];
        printf("[Consumer %ld] Consumed %d from %d\n", id, item, out_index);
        out_index = (out_index + 1) % BUFFER_SIZE;

        pthread_mutex_unlock(&buf_mutex); // Exit critical section
        sem_post(&empty_slots);          // Signal an empty slot

        sleep(2);
    }
    return NULL;
}

int main() {
    pthread_t prod[NUM_PRODUCERS], cons[NUM_CONSUMERS];

    sem_init(&empty_slots, 0, BUFFER_SIZE);
    sem_init(&full_slots, 0, 0);
    pthread_mutex_init(&buf_mutex, NULL);

    for (long i = 0; i < NUM_PRODUCERS; i++) {
        pthread_create(&prod[i], NULL, producer, (void*)i);
    }

    for (long i = 0; i < NUM_CONSUMERS; i++) {
        pthread_create(&cons[i], NULL, consumer, (void*)i);
    }

    for (int i = 0; i < NUM_PRODUCERS; i++) {
        pthread_join(prod[i], NULL);
    }
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        pthread_join(cons[i], NULL);
    }

    sem_destroy(&empty_slots);
    sem_destroy(&full_slots);
    pthread_mutex_destroy(&buf_mutex);

    return 0;
}
