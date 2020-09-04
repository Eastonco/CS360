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


#define DIRECTORY 'D'
#define FILE 'F'

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
NODE *newNode(char *name, char type);
void dbname(char *pathname);

#endif