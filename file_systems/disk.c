#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define MAX_BLOCK 1024 
#define MAX_SIZE 4096  
#define BIT_MAP_BLOCK 11 
#define MAX_BLOCK_PER_FILE 10 

char bitmap[MAX_SIZE] ;

int disk_fd ;

void init_disk(){
    disk_fd = open("disk.img" , O_CREAT|O_RDWR , 0666) ;  // here its normal file creation 
    if(disk_fd < 0) {perror("failed to initialize disk") ; 
        exit(1); }
    if(ftruncate(disk_fd, MAX_BLOCK*MAX_SIZE) !=0) {    // here we are fixing the size
          perror("failed to specify size of the disk") ;
          exit(1) ;
    }
    printf("Disk initialized: %d blocks\n", MAX_BLOCK);
}

void read_block(int block_no , void* buffer){
   off_t offset = block_no*MAX_SIZE ;
   lseek(disk_fd , offset , SEEK_SET) ;
   read(disk_fd , buffer , MAX_SIZE) ;
}

void write_block(int block_no , void* buffer ){
   off_t offset = block_no*MAX_SIZE ;
   lseek(disk_fd , offset , SEEK_SET) ;
   write(disk_fd , buffer , MAX_SIZE) ;
}
struct superblock{
    int total_block ;
    int inode_table ;
    int data_block ;
    int free_block ; 
} ;

void save_bitmap(){
    write_block(11 , &bitmap) ;
}

void load_bitmap(){
    read_block(11 , &bitmap) ;
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

int allocate_block(){
    for(int i = 12 ; i < MAX_BLOCK ; i++){
        if(check_block(i)){
            set_block_used(i) ;
            save_bitmap() ;
            return i ;
        }
    }
    return -1;
}

void free_block(int block_no) {
    set_block_free(block_no);
    save_bitmap();
}
void init_superblock(){
    struct superblock sb ;
    sb.total_block = MAX_BLOCK ;
    sb.inode_table = 1 ;
    sb.data_block = 12 ;
    sb.free_block = MAX_BLOCK - sb.data_block ;
    write_block(0 , &sb);  // block 0 me sb stored 
}

struct superblock get_superblock(){
    struct superblock sb ;
    read_block(0 , &sb) ;
    return sb ;
};

struct inode{
    int used ;
    int size ;
    int blocks[MAX_BLOCK_PER_FILE] ;
};

#define INODE_PER_BLOCK (MAX_SIZE / sizeof(struct inode))      // here roughly 32 inodes constitue one block 

void read_inode(int inode_no , struct inode* node){    // basically we have to put block's section data into this node
    int block_no = inode_no/INODE_PER_BLOCK  + 1;
    int offset = inode_no%INODE_PER_BLOCK ;
    struct inode temp[INODE_PER_BLOCK];
    read_block(block_no , temp) ;
    *node = temp[offset] ;
}
void write_inode(int inode_no , struct inode* node){   // take data from node write block 
    int block_no = inode_no/INODE_PER_BLOCK + 1 ;
    int offset = inode_no%INODE_PER_BLOCK ;
    struct inode temp[INODE_PER_BLOCK] ;
    read_block(block_no , temp) ;
    temp[offset] = *node ;
    write_block(block_no , temp) ;
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


int main() {

    memset(bitmap , 0 , MAX_BLOCK) ;
    for(int i = 0 ; i < 12 ; i++){
        set_block_used(i) ;
    }
    save_bitmap() ;
    init_disk();
    init_superblock();

    int inode_no = allocate_inode() ;
    char* data = "hello my name is shreyash sharma and i am studying in nitwarangal" ;
    write_file(inode_no , data) ;
    printf("Reading file:\n");
    read_file(inode_no);

    // load_bitmap();

    // int b1 = allocate_block();
    // int b2 = allocate_block();

    // printf("Allocated blocks: %d %d\n", b1, b2);

    // free_block(b1);

    // printf("Freed block: %d\n", b1);

    // save_bitmap();

    // struct inode node;

    // int inode_no = allocate_inode();
    // printf("Allocated inode: %d\n", inode_no);

    // read_inode(inode_no, &node);
    // printf("Inode used: %d, size: %d\n", node.used, node.size);

    // free_inode(inode_no);
    // printf("Freed inode: %d\n", inode_no);

    close(disk_fd);
}






// int main(){
//      init() ;
//      // make buffer using character array 
//     //  char read_buff[MAX_SIZE] ;
//     //  char write_buff[MAX_SIZE] ;

//     //  memset(read_buff , 0 , MAX_SIZE);
//     //  strcpy(write_buff, "hello i am shreyash") ; 
//     //  write_block(5 , write_buff) ;
     
//     //      // Read from block 5
//     // read_block(5, read_buff);
//     // printf("Read from block 5: %s\n", read_buff);
//     init_superblock();

//     struct superblock sb = get_superblock();

//     printf("Total blocks: %d\n", sb.total_block);
//     printf("Inode start: %d\n", sb.inode_table);
//     printf("Data start: %d\n", sb.data_block);
//     printf("Free blocks: %d\n", sb.free_block);
//     close(disk_fd);
//     return 0;
// }