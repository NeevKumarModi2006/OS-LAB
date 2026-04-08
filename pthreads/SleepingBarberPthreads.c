#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

#define CHAIRS 3

int waiting = 0;
pthread_mutex_t mutex;
pthread_cond_t barber_cond, customer_cond;

void* barber(void* arg) {
    while (1) {
        pthread_mutex_lock(&mutex);
        while (waiting == 0) {
            printf("Barber is SLEEPING\n");
            pthread_cond_wait(&customer_cond, &mutex);
        }
        waiting--;
        pthread_cond_signal(&barber_cond);
        pthread_mutex_unlock(&mutex);

        printf("Barber is CUTTING hair\n");
        sleep(2);
    }
}

void* customer(void* arg) {
    pthread_mutex_lock(&mutex);
    if (waiting < CHAIRS) {
        waiting++;
        printf("Customer is WAITING\n");
        pthread_cond_signal(&customer_cond);
        pthread_cond_wait(&barber_cond, &mutex);
    } else {
        printf("Customer LEFT (no chair)\n");
    }
    pthread_mutex_unlock(&mutex);
    return NULL;
}

int main() {
    pthread_t barber_t, cust[5];

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&barber_cond, NULL);
    pthread_cond_init(&customer_cond, NULL);

    pthread_create(&barber_t, NULL, barber, NULL);

    for (int i = 0; i < 5; i++) {
        sleep(1);
        pthread_create(&cust[i], NULL, customer, NULL);
    }

    pthread_join(barber_t, NULL);
    return 0;
}
// Sleeping Barber Problem
// Problem Statement:
// The Sleeping Barber problem describes a classic synchronization scenario in which
// a barber shop has one barber, one barber chair, and a waiting room with a limited
// number of chairs. The barber spends his time either cutting hair or sleeping when
// there are no customers. When a customer arrives, if the barber is sleeping, the
// customer wakes him up. If the barber is busy and waiting chairs are available, the
// customer waits; otherwise, the customer leaves the shop.

// Synchronization Requirements:
// Only one customer can be served by the barber at a time
// The barber sleeps when there are no customers
// Customers must wait if chairs are available
// Customers leave if no waiting chairs are available
// Proper synchronization is required to avoid race conditions
// Mutual exclusion must be ensured while accessing shared variable
