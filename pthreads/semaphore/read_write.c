#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>


sem_t mutex , wrt  ;
int read_count = 0 ;

void* reader(void* arg) {
    while(1){
    int i = (int)(long) arg ;
    sem_wait(&mutex) ;
    read_count++ ;
    if(read_count == 1) {
        sem_wait(&wrt) ;
    }
    printf("reader %d has started reading \n" , i) ;
    

    
    sem_post(&mutex) ;
    
    sem_wait(&mutex) ;
    printf("reader %d has finished reading \n" , i) ;
    read_count-- ; 
    sem_post(&mutex) ;
    if(read_count == 0) sem_post(&wrt) ;
	sleep(1);
}
}


void* writer(void* arg){
    while(1){
    int i = (int)(long) arg ;
    sem_wait(&wrt) ;
    printf("			writer %d has started writing\n" , i) ;
     printf("			writer %d has finished writing\n" , i) ;
    sem_post(&wrt) ;
    sleep(1) ;
}
}

int main(){
    pthread_t wr[2] , rd[4] ;
    sem_init(&wrt , 0 , 1) ;
    sem_init(&mutex , 0 , 1) ;
    for(int i = 0 ; i < 4 ; i++){
        pthread_create(&rd[i] , NULL , reader , (void*) (long) i ) ;
    }
    for(int i = 0 ; i < 2 ; i++){
        pthread_create(&wr[i] , NULL , writer , (void*) (long) i ) ;
    }
    for(int i = 0; i < 4; i++) pthread_join(rd[i], NULL);
    for(int i = 0; i < 2; i++) pthread_join(wr[i], NULL);

    sem_destroy(&wrt);
    sem_destroy(&mutex);
    return 0;
    
}


#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

// Fair (no-starvation) Readers–Writers using a service queue
double shared_data = 0.0;
int read_count_fair = 0;

sem_t resource;      // Controls access to shared_data (readers as a group OR one writer)
sem_t rmutex;        // Protects read_count_fair
sem_t serviceQueue;  // Fairness: all readers and writers queue here first

void* fair_reader(void* arg) {
    long id = (long)arg;

    while (1) {
        // --- ENTRY SECTION (fair) ---
        sem_wait(&serviceQueue);       // Get in line with writers and other readers
        sem_wait(&rmutex);             // Safely update read_count_fair

        read_count_fair++;
        if (read_count_fair == 1) {
            // First reader locks the resource so writers cannot enter
            sem_wait(&resource);
        }

        sem_post(&rmutex);
        sem_post(&serviceQueue);       // Let next thread in the queue proceed

        // --- CRITICAL SECTION (read) ---
        printf("[Reader %ld] reading shared_data = %.2f (active readers = %d)\n",
               id, shared_data, read_count_fair);
        sleep(1);

        // --- EXIT SECTION ---
        sem_wait(&rmutex);
        read_count_fair--;
        if (read_count_fair == 0) {
            // Last reader releases the resource for writers
            sem_post(&resource);
        }
        sem_post(&rmutex);

        // Simulate time between reads
        sleep(1);
    }

    return NULL;
}

void* fair_writer(void* arg) {
    long id = (long)arg;

    while (1) {
        // --- ENTRY SECTION (fair) ---
        sem_wait(&serviceQueue);   // Get in line (no one can bypass this)
        sem_wait(&resource);       // Exclusive access to the shared_data
        sem_post(&serviceQueue);   // Allow next thread in line to start its entry protocol

        // --- CRITICAL SECTION (write) ---
        shared_data += 1.0;
        printf("[Writer %ld] writing shared_data, new value = %.2f\n", id, shared_data);
        sleep(1);

        // --- EXIT SECTION ---
        sem_post(&resource);

        // Simulate time between writes
        sleep(2);
    }

    return NULL;
}

int main() {
    pthread_t readers[4], writers[2];

    // Initialize semaphores
    sem_init(&resource, 0, 1);      // Resource is initially free
    sem_init(&rmutex, 0, 1);        // Protects read_count_fair
    sem_init(&serviceQueue, 0, 1);  // Fair queue entry point

    // Create reader threads
    for (long i = 0; i < 4; i++) {
        pthread_create(&readers[i], NULL, fair_reader, (void*)i);
    }

    // Create writer threads
    for (long i = 0; i < 2; i++) {
        pthread_create(&writers[i], NULL, fair_writer, (void*)i);
    }

    // Join threads (in practice this never returns because of infinite loops)
    for (int i = 0; i < 4; i++) {
        pthread_join(readers[i], NULL);
    }
    for (int i = 0; i < 2; i++) {
        pthread_join(writers[i], NULL);
    }

    sem_destroy(&resource);
    sem_destroy(&rmutex);
    sem_destroy(&serviceQueue);

    return 0;
}



sem_t resource;
sem_t rmutex, wmutex, readTry;
int read_count = 0;
int write_count = 0;

void* reader(void* arg) {
    sem_wait(&readTry);      
    sem_wait(&rmutex);
    read_count++;
    if (read_count == 1)
        sem_wait(&resource);
    sem_post(&rmutex);
    sem_post(&readTry);

    printf("Reader reading\n");
    sleep(1);

    sem_wait(&rmutex);
    read_count--;
    if (read_count == 0)
        sem_post(&resource);
    sem_post(&rmutex);
}

void* writer(void* arg) {
    sem_wait(&wmutex);
    write_count++;
    if (write_count == 1)
        sem_wait(&readTry);   // block readers
    sem_post(&wmutex);

    sem_wait(&resource);
    printf("Writer writing\n");
    sleep(2);
    sem_post(&resource);

    sem_wait(&wmutex);
    write_count--;
    if (write_count == 0)
        sem_post(&readTry);
    sem_post(&wmutex);
}