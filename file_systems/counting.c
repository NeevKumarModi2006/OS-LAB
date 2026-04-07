#include <stdio.h>
#include <stdlib.h>

struct countnode{
    int start_block ;
    int count ;
    struct countnode* next ;
} ;

struct countnode* head_count = NULL ;



void add_free_blocks(int start , int count){
    struct countnode* newnode = (struct countnode*)malloc(sizeof(struct countnode)) ;
    newnode->start_block = start ;
    newnode->count = count ;
    newnode->next = head_count ;
    head_count = newnode ;
}

int allocate(int need){
    struct countnode* temp = head_count , *prev = NULL ;
    while(temp){
         if(temp->count >= need){
            int allocated_start = temp->start_block;
            temp->start_block += need ;
            temp->count -= need ;
            if(temp->count == 0){
                if(prev) prev->next = temp->next ;
                else head_count = temp->next ;
                free(temp) ;
            }
            return allocated_start ;
         }
         
         prev = temp ;
         temp = temp->next ;
    }
    return -1 ;
}

void display_counting() {
    struct countnode* temp = head_count;
    printf("\n--- Counting Free Space List ---\n");
    while (temp != NULL) {
        printf("[Start: %d, Count: %d] -> ", temp->start_block, temp->count);
        temp = temp->next;
    }
    printf("NULL\n");
}


int main() {
    int choice, start, count, need, allocated;
    
    while(1) {
        printf("\n--- Counting Free Space Allocation ---\n");
        printf("1. Add Free Blocks\n");
        printf("2. Allocate Blocks\n");
        printf("3. Display Free List\n");
        printf("4. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        
        switch(choice) {
            case 1:
                printf("Enter starting block: ");
                scanf("%d", &start);
                printf("Enter count of blocks: ");
                scanf("%d", &count);
                add_free_blocks(start, count);
                printf("Free blocks added.\n");
                break;
            case 2:
                printf("Enter number of blocks needed: ");
                scanf("%d", &need);
                allocated = allocate(need);
                if(allocated != -1) {
                    printf("Allocated successfully starting at block %d.\n", allocated);
                } else {
                    printf("Allocation failed! Not enough contiguous free blocks.\n");
                }
                break;
            case 3:
                display_counting();
                break;
            case 4:
                printf("Exiting...\n");
                return 0;
            default:
                printf("Invalid choice. Please try again.\n");
        }
    }
    return 0;
}