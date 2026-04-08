#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BUFFER_SIZE 5
#define NUM_ITEMS 20

int buffer[BUFFER_SIZE];
int in = 0, out = 0, count = 0;

pthread_mutex_t mutex;
pthread_cond_t not_full;
pthread_cond_t not_empty;

void* producer(void* arg) {
    int id = *((int*)arg);
    for (int i = 0; i < NUM_ITEMS; i++) {
        int item = rand() % 100;
        
        pthread_mutex_lock(&mutex);
        
        while (count == BUFFER_SIZE) {
            pthread_cond_wait(&not_full, &mutex);
        }
        
        buffer[in] = item;
        printf("Producer %d: Produced %d at position %d\n", id, item, in);
        in = (in + 1) % BUFFER_SIZE;
        count++;
        
        pthread_cond_signal(&not_empty);
        pthread_mutex_unlock(&mutex);
        
        sleep(1);
    }
    return NULL;
}

void* consumer(void* arg) {
    int id = *((int*)arg);
    for (int i = 0; i < NUM_ITEMS; i++) {
        pthread_mutex_lock(&mutex);
        
        while (count == 0) {
            pthread_cond_wait(&not_empty, &mutex);
        }
        
        int item = buffer[out];
        printf("Consumer %d: Consumed %d from position %d\n", id, item, out);
        out = (out + 1) % BUFFER_SIZE;
        count--;
        
        pthread_cond_signal(&not_full);
        pthread_mutex_unlock(&mutex);
        
        sleep(1);
    }
    return NULL;
}

int main() {
    pthread_t prod_thread, cons_thread;
    int prod_id = 1, cons_id = 1;
    
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&not_full, NULL);
    pthread_cond_init(&not_empty, NULL);
    
    pthread_create(&prod_thread, NULL, producer, &prod_id);
    pthread_create(&cons_thread, NULL, consumer, &cons_id);
    
    pthread_join(prod_thread, NULL);
    pthread_join(cons_thread, NULL);
    
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&not_full);
    pthread_cond_destroy(&not_empty);
    
    printf("Program completed successfully\n");
    return 0;
}






// Two processes share a fixed-size circular buffer of items
// producers generate data items and insert them into the buffer,
//  while consumers remove and process items from the buffer. The buffer has finite capacity.


// Synchronization Requirements:
// Buffer not full when producer inserts - Producer must wait if buffer is full
// Buffer not empty when consumer removes - Consumer must wait if buffer is empty
// Mutual exclusion - Only one process accesses buffer at a time

// Race Conditions to Prevent:
// Producer writing while consumer reading (corruption)
// Multiple producers overwriting same slot
// Multiple consumers reading same item
// Classic Example: Print spooler where jobs (producers) go to printer queue (buffer) and printer daemon (consumer) prints them.

