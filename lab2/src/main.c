#include "header.h"


int main(int argc, char* argv[], char* env[]){
    int index;
    debug = false;

    initialize(); //initialize root node of the file system tree

    while(1){
        memset(pathname, 0, sizeof pathname);
        memset(command, 0, sizeof command);

        printf("$ ");
        fgets(line,128,stdin); 
        line[strlen(line)-1] = 0;
        sscanf(line, "%s %s", command, pathname); 
        index = find_command(command);
        switch(index){
            case 0: 
                mkdir(pathname);
                break;
            case 1: 
                removedir(pathname);
                break;
            case 2: 
                ls(pathname);
                break; 
            case 3:
                cd(pathname);
                break;
            case 4:
                pwd();
                break;
            case 5:
                create(pathname);
                break;
            case 6:
                rm(pathname);
                break;
            case 7:
                reload(pathname);
                break;
            case 8:
                save(pathname);
                break;
            case 9:
                menu();
                break;
            case 10:
                quit();
                return;
            default: 
                printf("invalid command %s\n", command);
        }
    }
    
    
    return 1;

}
