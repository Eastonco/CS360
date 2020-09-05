#ifndef FILE_SYSTEM
#define FILE_SYSTEM

#include <libgen.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>


#define DIRECTORY_TYPE 'D'
#define FILE_TYPE 'F'
#define SET_TEXT_BLUE "\033[1;36m"
#define RESET_TEXT "\033[0m;"

typedef struct node {

    char * name[64];
    char type;
    struct node *parentPtr;
    struct node *siblingPtr;
    struct node *childPtr;

}NODE;

NODE *root, *cwd;
char line[128], command[16], pathname[64],dname[64], bname[64];

void initialize(void);
int find_command(char *command);
NODE *new_node(char *name, char type);
void dbname(char *pathname);
void print_dir(char *dirname);
void save(FILE *fname);

#endif