#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define MAX_BLOCK 4096
#define MAX_SIZE 1024
#define BIT_MAP_BLOCK 11
#define MAX_BLOCK_PER_FILE 10

int disk_fd;
char bitmap[MAX_SIZE];

struct inode{
    int size;
    int used;
    int blocks[MAX_BLOCK_PER_FILE];
}

#define INODE_PER_FILE (MAX_SIZE / sizeof(struct inode))

struct superblock{
    int total_block;
    int inode_table;
    int data_block;
    int free_block;
}

void disk_init(){
    disk_fd = open("disk.img", O_CREAT | O_RDWR, 0666);
    if(disk_fd <0) exit(1);
    if(ftruncate(disk_fd, MAX_BLOCK*MAX_SIZE) != 0) exit(1);

}

void read_block(int block_no , void * buffer){
    off_t offset = block_no*MAX_SIZE;
    lseek(disk_fd,offset,SEEK_SET);
    read(disk_fd,buffer, MAX_SIZE );
}
void write_block(int block_no , void * buffer){
        off_t offset = block_no*MAX_SIZE;
    lseek(disk_fd,offset,SEEK_SET);
    read(disk_fd,buffer, MAX_SIZE );
}


void set_block_used(int block_no){
     bitmap[block_no/8] |= (1 << (block_no%8)) ;
}

void set_block_free(int block_no){
    bitmap[block_no/8] &= ~(1 << (block_no % 8));
}



int check_block(int block_no){
      return !(bitmap[block_no / 8] & (1 << (block_no % 8)));
}

void load_bitmap(){
    read_block(11, &bitmap);
}
void save_bitmap(){
    write_block(11, &bitmap);
}

void free_block(int block_no){
    set_block_free(block_no);
    save_bitmap();
}
void allocate_block(){
    for(int i=12; i<MAX_BLOCK; i++){
        if(check_block(i)){
            set_block_used(i);
            save_bitmap();
            return i;
        }
    }
    return -1;
}

void init_superblock(){
    char buffer[MAX_SIZE];
    memset(buffer, 0, MAX_SIZE);
    struct superblock* sb = (struct superblock*)buffer;
    sb->total_block = MAX_BLOCK ;
    sb->inode_table = 1 ;
    sb->data_block = 12 ;
    sb->free_block = MAX_BLOCK - sb->data_block ;
    write_block(0 , buffer); 
}

struct superblock get_superblock(){
    char buffer[MAX_SIZE];
    read_block(0 , buffer) ;
    struct superblock* sb = (struct superblock*)buffer;
    return *sb ;
};





void read_inode(int inode_no , struct inode* node){    // basically we have to put block's section data into this node
    int block_no = inode_no/INODE_PER_BLOCK  + 1;
    int offset = inode_no%INODE_PER_BLOCK ;
    char buffer[MAX_SIZE];
    read_block(block_no , buffer) ;
    struct inode* temp = (struct inode*)buffer;
    *node = temp[offset] ;
}
void write_inode(int inode_no , struct inode* node){   // take data from node write block 
    int block_no = inode_no/INODE_PER_BLOCK + 1 ;
    int offset = inode_no%INODE_PER_BLOCK ;
    char buffer[MAX_SIZE];
    read_block(block_no , buffer) ;
    struct inode* temp = (struct inode*)buffer;
    temp[offset] = *node ;
    write_block(block_no , buffer) ;
}

int allocate_inode(){
    for(int i = 0 ; i < INODE_PER_BLOCK*10 ; i++){
        struct inode node;

        read_inode(i , &node) ;
        if(node.used == 0){
          node.used = 1 ;
          for(int j = 0 ; j < MAX_BLOCK_PER_FILE ; j++){
            node.blocks[j] = -1 ;
          } 
          node.size =  0 ;
          write_inode(i , &node) ;
          return  i ;
        }
    }
    return -1 ;
}
void free_inode(int inode_no){
    struct inode node ;
    read_inode(inode_no , &node) ;
    node.used = 0 ;
    write_inode(inode_no , &node) ;
}


void write_file(int inode_no , char* buffer){
       struct inode node ;
       read_inode(inode_no , &node) ;
       int len = strlen(buffer) ;
       node.size = len ;
       int blocks_needed = (len + MAX_SIZE- 1) / MAX_SIZE;
       for(int i = 0 ; i < blocks_needed ; i++){
         int block_no = allocate_block() ;
         node.blocks[i] = block_no ;
         char buffer2[MAX_SIZE] ;
         memset(buffer2 , 0 , MAX_SIZE);
         int copy_size = (len - i * MAX_SIZE > MAX_SIZE) ? MAX_SIZE : (len - i * MAX_SIZE);
         memcpy(buffer2 , buffer + i*MAX_SIZE , copy_size) ;
         write_block(block_no , buffer2) ;
    }
    write_inode(inode_no, &node);
}

void read_file(int inode_no){
    struct inode node ;
    char buffer[MAX_SIZE] ;
    read_inode(inode_no , &node) ;
    for(int i = 0 ; i < MAX_BLOCK_PER_FILE ; i++){
        if(node.blocks[i] == -1) return ;
        read_block(node.blocks[i] , buffer) ;
        printf("%s", buffer);
    }
    printf("\n") ;
}


int main(){

    return 0;
}



