#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct node{
    int block;
    struct node* next;
};

struct node* head = NULL;

void freeBlock(int block_no){
    struct node* newNode = (struct node* ) malloc(sizeof(struct node));
    newNode->block = block_no;
    newNode->next = head;
    head = newNode;
}

void display(){
    struct node * N = head;
    printf("Free Blocks");
    while(N != NULL){
        printf(" %d ", N->block);
        N = N->next;
    }   
    printf("\n");
}

void allocate_block(){
    if(head==NULL){
        printf("no free blocks.\n");
        return;
    }
    struct node* N = head;
    printf("we got a free block: %d\n" , N->block ) ;
    head = head->next ;
    free(N) ;
}



int main(){
    int choice, block;

    while (1) {
        printf("\n1. Free block\n2. Allocate block\n3. Display\n4. Exit\n");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                printf("Enter block: ");
                scanf("%d", &block);
                freeBlock(block);
                break;

            case 2:
                allocate_block();
                break;

            case 3:
                display();
                break;

            case 4:
                return 0;
        }
    }

    return 0;
}