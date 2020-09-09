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
#include <stdbool.h>



#define DIRECTORY_TYPE 'D'
#define FILE_TYPE 'F'
#define SET_TEXT_BLUE "\033[1;36m"
#define RESET_TEXT "\033[0m"


typedef struct node {

    char * name[64];
    char type;
    struct node *parentPtr;
    struct node *siblingPtr;
    struct node *childPtr;

}NODE;

NODE *root, *cwd;
char line[128], command[16], pathname[64],dname[64], bname[64], savefile[64];

bool debug;

// pathname = "/this/that/hello"
// dname = "/this/that"
// bname = "hello"

void initialize(void);
int find_command(char *command);
NODE *new_node(char *name, char type);
void dbname(char *pathname);
void print_node(NODE *pcur);
void save(char *filename);
void pwd();
void pwdhelper(NODE * pcur);
void quit();

void mkdir(char *pathname);
NODE *insert_node(NODE *parent, char *name, char type);
NODE *find_node(NODE *pcur, char *pathname);
NODE *find_helper(NODE *pcur, char *target, char file_type);

void rprint(NODE * pcur, FILE *fd);
void print_filesystem(FILE * fd);
void fpwd(NODE *pcur, FILE * fd);

void ls(char * pathname);
void rm(char * pathname);

NODE * parse_pathname(char *pathname);

void create(char * pathname);
void cd(char *pathname);

void delete_node(NODE *pcur);
void removedir(char *pathname);


#endif