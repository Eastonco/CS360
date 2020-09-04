#include "header.h"

void initialize(void){
    root = newNode("/", DIRECTORY);
    cwd = root;
    strcpy(pathname, cwd->name);


}

// ***************************************TREE THINGS******************************************************
NODE *newNode(char *name, char type){
    NODE *node = (NODE *)malloc(sizeof(NODE));
    printf("MADE A NEW NODE\n");
    strcpy(node->name, name);
    node->type = type;
    return node;
}

void dbname(char *pathname){
    char temp[128]; // dirname(), basename() destroy original pathname 
    strcpy(temp, pathname);
    strcpy(dname, dirname(temp));
    strcpy(temp, pathname);
    strcpy(bname, basename(temp));
}


