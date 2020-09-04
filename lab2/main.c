#include "header.h"


int main(int argc, char* argv[], char* env[]){
    int index;
    char line[128], command[16], pathname[64];

    initialize(); //initialize root node of the file system tree 

    printf("%s\n", dirname("hello/world/this/test\0"));
    
    
    /*
    while(1){
        printf("input a commad line : "); fgets(line,128,stdin); line[strlen(line)-1] = 0;
        sscanf(line, "%s %s", command, pathname); index = fidnCmd(command);
        switch(index){
            case 0 : 
                mkdir(pathname); 
                break;
            case 1 : 
                rmdir(pathname);
                break;
            case 2 : 
                ls(pathname);
                break; 
            default: 
                printf("invalid command %s\n", command);
        }
    }
    */
    return;

}