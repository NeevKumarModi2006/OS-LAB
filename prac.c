#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>   
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <dirent.h>
#include <ctype.h>

#define MAX_TOKENS 50
#define MAX_LINE 1024


// void ls(int argc , char* argv[]){
//     const char* path = (argc < 2) ? "." : argv[1];
//     DIR* dir = open(path);
//     if(!dir){
//         perror("ls open error");
//         return;
//     }
//     struct dirent* entry;
//     while((entry = readdir(dir)) != NULL){
//         if(entry->d_name[0] == '.') continue;
//         printf("%s\n", entry->d_name);

//     }
//     closedir(dir);
// }

// void cat(int argc, char* argv[]){
//     if(argc < 2){
//         char buf[1024];
//         while(fgets(buf, sizeof(buf), stdin)) printf("%s", buf);
//     }else{
//         for(int i=1; i<argc; i++){
//             FILE *file = fopen(argv[i], "r");
//             if(!file){
//                 perror("cat %s error",argv[i]);
//                 continue;
//             }
//             int c;
//             while((c = fgetc(file)) != EOF) putchar(c);
//             fclose(file);
//         }
//     }
// }



int main(){
    char line[MAX_LINE];
    
        char *cwd = getcwd(NULL, 0);
    while(1){
        printf("%s $ ", cwd);
        fflush(stdout);
        if( !fgets(line, sizeof(line), cin)) break;
        line[strcspn(line, "\n")] = 0;
        if(strcmp(line, "exit") == 0) break;
        char* tokens[MAX_TOKENS];
        char* token = strtok(line, " ");
        int count = 0;
        while(token != NULL && count < MAX_TOKENS){
           tokens[count++] = token;
           token =  strtok(NULL, " ");
        }
        

        
    }


}