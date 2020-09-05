#include "header.h"

void initialize(void){
    root = new_node("/", DIRECTORY_TYPE);
    cwd = root;
    strcpy(pathname, cwd->name);
}

void save(FILE *fname){ //TODO: make this iterate
    FILE *fp = fopen(fname, "w+");
    fprintf(fp, "%c %s", 'D', "string\n"); // Type and name string should go here, pre order traversal
    fclose(fp);
}

int find_command(char *command){
    char *cmd[] = {"mkdir", "rmdir", "ls", "cd", "pwd", "creat", "rm", "reload", "save", "menu", "quit", NULL};

    int i = 0;
    while(cmd[i]){
        if (!strcmp(command, cmd[i])){
            return i; // found command: return index i 
        }
        i++;
    }
    return -1; // not found: return -1
}

// ***************************************TREE THINGS******************************************************
NODE *new_node(char *name, char type){
    NODE *node = (NODE *)malloc(sizeof(NODE));
    printf("MADE A NEW NODE\n");
    strcpy(node->name, name);
    node->type = type;
    return node;
}

// NOTE: dirname truncates the string, basename copies to a new variable
void dbname(char *pathname){
    char temp[128]; // dirname(), basename() destroy original pathname 
    strcpy(temp, pathname);
    strcpy(dname, dirname(temp));
    strcpy(temp, pathname);
    strcpy(bname, basename(temp));
}

void print_dir(char *dirname){
    printf(SET_TEXT_BLUE);
    printf("%s\n", dirname);
    printf(RESET_TEXT);
}


