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
    char buffer[MAX_SIZE];
    memset(buffer, 0, MAX_SIZE);
    struct superblock* sb = (struct superblock*)buffer;
    sb->total_block = MAX_BLOCK ;
    sb->inode_table = 1 ;
    sb->data_block = 12 ;
    sb->free_block = MAX_BLOCK - sb->data_block ;
    write_block(0 , buffer);  // block 0 me sb stored 
}

struct superblock get_superblock(){
    char buffer[MAX_SIZE];
    read_block(0 , buffer) ;
    struct superblock* sb = (struct superblock*)buffer;
    return *sb ;
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

#define FILENAME 28 //max length of filename 

struct dir_entry{
    int inode_no ; 
    char filename[FILENAME] ;
} ;

#define ENTRIES_PER_BLOCK  MAX_SIZE/sizeof(struct dir_entry)


// now we need to setup the root directory 

int init_root_directory(){
    //get a inode number for root 
    int root_inode_no = allocate_inode() ;
    // now we have to fill the space at that block where root_inode is present with appropriate data
    struct inode root_inode ;
    // pass the address of our local struct so read_inode can fill it
    read_inode(root_inode_no , &root_inode) ;
    
    // now it has the correct data, let's update the entry
    root_inode.blocks[0] = allocate_block() ; // some block b/w 12 to 1024 
    root_inode.size = MAX_SIZE ;
    root_inode.used = 1 ; 
    
    // now write this value back to the inode table
    write_inode(root_inode_no , &root_inode) ;
    
    // now we know root directory manage 10 blocks and block 0 we will use to store filenames 
    char buffer[MAX_SIZE];
    memset(buffer, 0, MAX_SIZE);
    struct dir_entry* entries = (struct dir_entry*)buffer;   // this much entry i can store in one block
    for (int i = 0; i < ENTRIES_PER_BLOCK; i++) {
        entries[i].inode_no = -1;
        memset(entries[i].filename, 0, FILENAME);
    }
    // write the empty directory block to disk
    write_block(root_inode.blocks[0] , buffer ) ;
    return root_inode_no ;
}

void add_file_to_dir(int dir_inode_no , int file_inode_no , const char* filename){
    struct inode directory ;  
    read_inode(dir_inode_no , &directory) ;  
    
    int block_no = directory.blocks[0] ;  // get the block number where entries are present
    if(block_no == -1) {
        printf("Error: Directory has no data blocks.\n");
        return;
    }
    
    char buffer[MAX_SIZE];
    struct dir_entry* entries = (struct dir_entry*)buffer;
    read_block(block_no , buffer) ; // got all the entries  
    
    // look for any empty slots 
    for(int i = 0 ; i < ENTRIES_PER_BLOCK ; i++){
        if(entries[i].inode_no == -1){
            // empty block found
            entries[i].inode_no = file_inode_no ; // BUG FIX: changed entries->inode_no to entries[i]
            strncpy(entries[i].filename , filename , FILENAME-1);
            // write the updated entry to block 
            write_block(block_no , buffer) ;
            return ;
        }
    }
    printf("Error: Directory is full!\n");
}

int find_file_inode(int dir_inode_no , const char* filename){
    struct inode dir ;
    read_inode(dir_inode_no , &dir) ;
    
    int block_no = dir.blocks[0] ;
    if(block_no == -1) {
        printf("Error: Directory has no data blocks.\n");
        return -1;
    }
    
    char buffer[MAX_SIZE];
    struct dir_entry* entries = (struct dir_entry*)buffer;
    read_block(block_no , buffer ) ;
    
    for(int i = 0 ; i < ENTRIES_PER_BLOCK ; i++){
        if (entries[i].inode_no != -1 && strcmp(entries[i].filename, filename) == 0) {
            return entries[i].inode_no; // Found it!
        }
    }
    return -1 ;
}

int main() {
        memset(bitmap , 0 , MAX_BLOCK) ;
    for(int i = 0 ; i < 12 ; i++){
        set_block_used(i) ;
    }
    save_bitmap() ;

    // 1. Setup the file system
init_disk();
init_superblock();
int root_dir = init_root_directory(); // Create root

// 2. Create a new file "hello.txt"
int new_file_inode = allocate_inode();
add_file_to_dir(root_dir, new_file_inode, "hello.txt");
write_file(new_file_inode, "hello my name is shreyash sharma");

// 3. To read it later, just search by name!
int target_inode = find_file_inode(root_dir, "hello.txt");
if (target_inode != -1) {
    printf("Found 'hello.txt' at Inode %d\n", target_inode);
    read_file(target_inode);
} else {
    printf("File not found.\n");
}

    // memset(bitmap , 0 , MAX_BLOCK) ;
    // for(int i = 0 ; i < 12 ; i++){
    //     set_block_used(i) ;
    // }
    // save_bitmap() ;
    // init_disk();
    // init_superblock();

    // int inode_no = allocate_inode() ;
    // char* data = "hello my name is shreyash sharma and i am studying in nitwarangal" ;
    // write_file(inode_no , data) ;
    // printf("Reading file:\n");
    // read_file(inode_no);

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