#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define MAX 100
#define FRAMES 3

struct shared_data {
    int pages[MAX];
    int n;
    int result;
};


void fifo(int page[] , int n){
    int frame[FRAMES] ; int front = 0 ; 
    for(int i = 0 ; i < FRAMES ; i++) frame[i] = -1 ;
    printf("\nFrame Status:\n");
    for(int i = 0 ; i < n ; i++){
        int found = 0 ; 
        for(int j = 0 ; j < FRAMES  ; j++){
            if(frame[j] == page[i]) {
                found = 1 ;
                break ;
            }
        }
        if(!found) {
            frame[front] = page[i] ;
            front = (front+1)%FRAMES ; 
        }
        for(int j = 0 ; j < FRAMES  ; j++){
            if (frame[j] == -1)
                printf("- ");
            else
                printf("%d ", frame[j]);
        }
        printf("\n") ;
    }
}

void lru(int page[] , int n){
    int frame[FRAMES] , time[FRAMES] ;
    int count = 0 ;
    for (int i = 0; i < FRAMES; i++) {
        frame[i] = -1;
        time[i] = 0;
    }
    printf("\nFrame Status:\n");
    for(int i = 0 ; i < n ; i++){
        int found = 0 ;
        for (int j = 0; j < FRAMES; j++) {
            if (frame[j] == page[i]) {
                found = 1;
                time[j] = ++count ;
                break;
            }
        }
        if (!found) {
            int lru_index = 0;
            for (int j = 1; j < FRAMES; j++) {
                if (time[j] < time[lru_index])
                    lru_index = j;
            }

            frame[lru_index] = page[i];
            time[lru_index] = ++count;
        }
        for (int j = 0; j < FRAMES; j++) {
            if (frame[j] == -1)
                printf("- ");
            else
                printf("%d ", frame[j]);
        }
        printf("\n");
    }
}


int main(){
    int shmid = shmget(1234 , sizeof(struct shared_data) , 0666|IPC_CREAT);
    if (shmid == -1) {
        perror("shmget failed");
        exit(1);
    }
    struct shared_data* data = (struct shared_data* ) shmat(shmid , NULL , 0) ;
    if (data == (void *)-1) {
        perror("shmat failed");
        exit(1);
    }
    int choice;
    printf("Choose Algorithm:\n1. FIFO\n2. LRU\nEnter choice: ");
    scanf("%d", &choice);
    if (choice == 1) {
        printf("Using FIFO\n");
        fifo(data->pages, data->n);
    } else {
        printf("Using LRU\n");
        lru(data->pages, data->n);
    }
    shmdt(data);
    // shmctl(shmid, IPC_RMID, NULL); // to clear the shared memory space 
    return 0;
}
