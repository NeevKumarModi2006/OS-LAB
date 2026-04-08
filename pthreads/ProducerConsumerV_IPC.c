#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/wait.h>

#define BUFFER_SIZE 5
#define NUM_ITEMS 10

typedef struct {
    int buffer[BUFFER_SIZE];
    int in;
    int out;
} shared_data;

void sem_wait(int semid, int sem_num) {
    struct sembuf sb = {sem_num, -1, 0};
    semop(semid, &sb, 1);
}

void sem_signal(int semid, int sem_num) {
    struct sembuf sb = {sem_num, 1, 0};
    semop(semid, &sb, 1);
}

int main() {
    int shmid = shmget(IPC_PRIVATE, sizeof(shared_data), IPC_CREAT | 0666);
    shared_data *shared = (shared_data*)shmat(shmid, NULL, 0);
    shared->in = 0;
    shared->out = 0;

    // sem[0]: Mutex, sem[1]: Empty slots, sem[2]: Full slots
    int semid = semget(IPC_PRIVATE, 3, IPC_CREAT | 0666);
    semctl(semid, 0, SETVAL, 1);           // Mutex = 1
    semctl(semid, 1, SETVAL, BUFFER_SIZE); // Empty = 5
    semctl(semid, 2, SETVAL, 0);           // Full = 0

    if (fork() == 0) { // Producer
        for (int i = 0; i < NUM_ITEMS; i++) {
            sem_wait(semid, 1); // wait(empty)
            sem_wait(semid, 0); // wait(mutex)
            
            shared->buffer[shared->in] = rand() % 100;
            printf("Producer: Inserted at %d\n", shared->in);
            shared->in = (shared->in + 1) % BUFFER_SIZE;
            
            sem_signal(semid, 0); // signal(mutex)
            sem_signal(semid, 2); // signal(full)
            sleep(1);
        }
        exit(0);
    }

    if (fork() == 0) { // Consumer
        for (int i = 0; i < NUM_ITEMS; i++) {
            sem_wait(semid, 2); // wait(full)
            sem_wait(semid, 0); // wait(mutex)
            
            printf("Consumer: Removed from %d\n", shared->out);
            shared->out = (shared->out + 1) % BUFFER_SIZE;
            
            sem_signal(semid, 0); // signal(mutex)
            sem_signal(semid, 1); // signal(empty)
            sleep(2);
        }
        exit(0);
    }

    wait(NULL); wait(NULL);
    shmctl(shmid, IPC_RMID, NULL);
    semctl(semid, 0, IPC_RMID);
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

