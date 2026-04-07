#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define addr_per_block 4

struct groupNode{
    int free_blocks[addr_per_block-1];
    int num_free;
    struct groupNode* next;
};


struct groupNode* head = NULL;

void free_block(int block_addr){
    if(!head || head->num_free == addr_per_block - 1 ){
        struct groupNode* N = (struct groupNode*) malloc(sizeof(struct groupNode));
        N->num_free = 0;
        N->free_blocks[N->num_free++] = block_addr;
        N->next = head;
        head = N; 
        return;       
    }
    head->free_blocks[head->num_free++] = block_addr;
}

void display(){
    struct groupNode* n = head;
    printf("head ->");
    while(n){
        printf(" [");
        for(int i= n->num_free -1 ;i>=0 ;i--){
          printf(" %d " , n->free_blocks[i]);
        }
        printf("] ->"); 
        n = n->next;
    }
    printf("NULL\n");
}

void allocate_block(){
    struct groupNode* n = head;

    while(n && n->num_free == 0){
        n = n->next;
    }
    if(n){
        n->num_free--;
        printf("free block at address %d is allocated.\n", n->free_blocks[n->num_free]);
                if(n->num_free == 0){
                    head = n->next;
                    free(n);
                }
    }else{
        printf("No free space available to allocate.\n");
    }
}

int main(){
    int choice , block;

    while(1){
        printf("\n1. Free block\n2. Allocate block\n3. Display\n4. Exit\n");
        scanf("%d", &choice);
        switch(choice){
              case 1:
                printf("Enter block: ");
                scanf("%d", &block);
                free_block(block);
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