#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define NUM_CUSTOMERS 10
#define NUM_CHAIRS 3

sem_t customers;      // Number of waiting customers
sem_t barber;         // Barber is ready
pthread_mutex_t mutex; // Protects waiting count

int waiting = 0;

void* barber_thread(void* arg) {
    (void)arg;
    while (1) {
        sem_wait(&customers);       // Go to sleep if no customers
        pthread_mutex_lock(&mutex);

        waiting--;                  // One customer leaves waiting room
        sem_post(&barber);         // Barber is ready to cut hair
                printf("[Barber] Cutting hair... Remaining waiting: %d\n", waiting);
        pthread_mutex_unlock(&mutex);

        // Cutting hair

        sleep(2);
    }
    return NULL;
}

void* customer_thread(void* arg) {
    long id = (long)arg;

    pthread_mutex_lock(&mutex);
    if (waiting < NUM_CHAIRS) {
        waiting++;
        printf("[Customer %ld] Sitting in waiting room. Waiting = %d\n", id, waiting);
        sem_post(&customers);     // Notify barber that there is a customer
        pthread_mutex_unlock(&mutex);

        sem_wait(&barber);        // Wait until barber is ready
        printf("[Customer %ld] Getting haircut.\n", id);
        // Being served (no extra sleep here; barber simulates work)
    } else {
        // No free chairs; customer leaves
        printf("[Customer %ld] No free chair, leaving.\n", id);
        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

int main() {
    pthread_t barber_t;
    pthread_t customers_t[NUM_CUSTOMERS];

    sem_init(&customers, 0, 0);
    sem_init(&barber, 0, 0);
    pthread_mutex_init(&mutex, NULL);

    pthread_create(&barber_t, NULL, barber_thread, NULL);

    for (long i = 0; i < NUM_CUSTOMERS; i++) {
        sleep(1); // Customers arrive over time
        pthread_create(&customers_t[i], NULL, customer_thread, (void*)i);
    }

    for (int i = 0; i < NUM_CUSTOMERS; i++) {
        pthread_join(customers_t[i], NULL);
    }

    // In this simple demo, barber thread runs forever; normally you would cancel/join it.

    sem_destroy(&customers);
    sem_destroy(&barber);
    pthread_mutex_destroy(&mutex);
    return 0;
}