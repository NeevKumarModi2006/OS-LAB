#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define MAX_BLOCK 4096
#define MAX_SIZE 1024
#define BIT_MAP_BLOCK 11
#define MAX_BLOCK_PER_FILE 10

char bitmap[MAX_SIZE];
int disk_fd;

struct superblock{
    int total_block; 
    int table_data;
    int free_block;
    int data_block;
}

struct inode{
    int used;
    int size;
    int blocks[MAX_BLOCK_PER_FILE];
}

#define INODE_PER_BLOCK (MAX_SIZE / sizeof(struct inode))


void init_disk(){
    disk_fd = open( "disk.img", O_CREAT | O_RDWR,0666);
    if(disk_fd < 0){
        perror("init disk failed.");
        exit(1);
    }
    if(ftrucate(disk_fd, MAX_BLOCK*MAX_SIZE) != 0){
        perror("init disk failed.");
        exit(1);
    }
    printf("Disk Initialized successfully.\n");
    close(disk_fd);
}

void set_block_used(int block_no){
    bitmap[block_no/8] |= (1 << (block_no % 8));
}
void set_block_free(){
    bitmap[block_no/8] &= ~(1 << (block_no % 8));
}
int check_block(int block_no){
    return !( bitmap[block_no/8] &  (1 << (block_no % 8)) );
}

void read_block( int block_no, void* buffer){
    off_t offset = block_no*MAX_SIZE;
    lseek(disk_fd, offset, SEEK_SET);
    read(disk_fd, buffer , MAX_SIZE );
}
void write_block(int block_no, void* buffer){
    off_t offset = block_no* MAX_SIZE;
    lseek(disk_fd, offset, SEEK_SET);
    write(disk_fd, buffer, MAX_SIZE ) ;
}
void allocate_block(){
    for(int i=12; i<MAX_BLOCK; i++){
        if(check_block(i)){
            set_block_used(i);
            save_bitmap();
            return i;
        }
    }
}
void free_block(int block_no){
    set_block_free(block_no);
    save_bitmap();
}

void save_bitmap(){
    write_block(11, &bitmap);
}
void load_bitmap(){
    read_bitmap(11, &bitmap);
}



void init_superblock();
struct superblock get_superblock(){
    struct superblock sb ;
    read_block(0 , &sb) ;
    return sb ;
};


void read_inode( int inode_no, struct inode* node){
    int block_no = inode_no/INODE_PER_BLOCK + 1;
    int offset = inode_no % INODE_PER_BLOCK;
    char buffer[MAX_SIZE];
    read_buffer(block_no, buffer);
    struct inode* temp = (struct inode*) buffer;
    *node = temp[offset];
}
void write_inode(int inode_no, struct inode* node){
    int block_no = inode_no/INODE_PER_BLOCK + 1;
    int offset = inode_no % INODE_PER_BLOCK;
    char buffer[MAX_SIZE];
    read_block(block_no,buffer );
    struct inode* temp = (struct inode*) buffer;
    temp[offset] = *node ;  
    write_block(block_no, buffer);  
}
int allocate_inode(){
    for(int i=0; i<12; i++){
        struct inode node;
        read_inode(i, &node);
        if(node.used == 0){
            node.used = 1;
            for(int i=0; i<MAX_BLOCKS_PER_FILE  ; i++){
                    node.blocks[i]  = -1;
            }
            node.size = 0;
            write_inode(i , &node);
            return i;
        }
    }
    return -1;
}
void free_inode(int i){
    struct inode node;
    read_inode(i, &node);
    node.used = 0;
    writr_inode(i, &node);
}

void read_file(){

}
void write_file(){
    
}

int main(){

    return 0;
}



