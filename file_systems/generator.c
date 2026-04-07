#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

#define MAX_PAGES 100

struct shared_data {
    int page[MAX_PAGES];
    int n;
    int result;
};

int main() {
    int shmid = shmget(1234, sizeof(struct shared_data), 0666 | IPC_CREAT);
    struct shared_data *data = (struct shared_data *)shmat(shmid, NULL, 0);
    
    printf("enter the number of pages\n");
    scanf("%d", &data->n);
    
    printf("enter the reference string\n");
    for (int i = 0; i < data->n; i++) {
        scanf("%d", &data->page[i]);
    }
    
    shmdt(data);
    return 0;
}
