#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

#define M_READERS 5
#define N_WRITERS 3

int read_count = 0;
pthread_mutex_t mutex;   // protects read_count
pthread_mutex_t wrt;     // ensures exclusive writer access

void* reader(void* arg) {
    int id = *(int*)arg;

    /* Entry section */
    pthread_mutex_lock(&mutex);
    read_count++;
    if (read_count == 1)
        pthread_mutex_lock(&wrt);   // first reader blocks writers
    pthread_mutex_unlock(&mutex);

    /* Critical section */
    printf("Reader %d is READING\n", id);
    sleep(1);

    /* Exit section */
    pthread_mutex_lock(&mutex);
    read_count--;
    if (read_count == 0)
        pthread_mutex_unlock(&wrt); // last reader releases writers
    pthread_mutex_unlock(&mutex);

    return NULL;
}

void* writer(void* arg) {
    int id = *(int*)arg;

    /* Entry section */
    pthread_mutex_lock(&wrt);

    /* Critical section */
    printf("Writer %d is WRITING\n", id);
    sleep(2);

    /* Exit section */
    pthread_mutex_unlock(&wrt);

    return NULL;
}

int main() {
    pthread_t readers[M_READERS], writers[N_WRITERS];
    int r_id[M_READERS], w_id[N_WRITERS];

    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&wrt, NULL);

    for (int i = 0; i < M_READERS; i++) {
        r_id[i] = i + 1;
        pthread_create(&readers[i], NULL, reader, &r_id[i]);
    }

    for (int i = 0; i < N_WRITERS; i++) {
        w_id[i] = i + 1;
        pthread_create(&writers[i], NULL, writer, &w_id[i]);
    }

    for (int i = 0; i < M_READERS; i++)
        pthread_join(readers[i], NULL);
    for (int i = 0; i < N_WRITERS; i++)
        pthread_join(writers[i], NULL);

    return 0;
}

// Readers–Writers Problem
// Problem Statement:
// The Readers–Writers problem involves a shared resource that is accessed by multiple
// reader and writer processes. Readers only read the shared data, while writers modify
// it. Multiple readers may read simultaneously, but writers must have exclusive access.

// Synchronization Requirements:
// Supports M reader processes and N writer processes
// Multiple readers can read concurrently
// Only one writer can write at a time
// No reader should read while a writer is writing
// Ensure mutual exclusion and data consistency


