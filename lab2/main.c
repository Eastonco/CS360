#include "header.h"


int main(int argc, char* argv[], char* env[]){
    int index;


    
    

    initialize(); //initialize root node of the file system tree

    while(1){
        printf("input a commad line : ");
        fgets(line,128,stdin); 
        line[strlen(line)-1] = 0;
        sscanf(line, "%s %s", command, pathname); 
        index = find_command(command);
        switch(index){
            case 0: 
                //mkdir(pathname);
                printf("\nMAKEDIR CALLED\n");
                break;
            case 1: 
                //rmdir(pathname);
                printf("\nRMDIR CALLED\n");
                break;
            case 2: 
                printf("\nLS CALLED\n"); 
                //ls(pathname);
                break; 
            case 3:
                //cd();
                break;
            case 4:
                //pwd();
                break;
            case 5:
                //creat();
                break;
            case 6:
                //rm();
                break;
            case 7:
                //reload();
                break;
            case 8:
                //save();
                break;
            case 9:
                //menu();
                break;
            case 10:
                //quit();
                break;
            default: 
                printf("invalid command %s\n", command);
        }
    }
    
    
    return 1;

}