#ifndef CMD_H
#define CMD_H

#include "util.h"


// Function prototypes
//  ls
int ls_file(MINODE *mip, char *name);
int ls_dir(MINODE *mip);
int my_ls(char *pathname);

//  pwd
char *my_pwd(MINODE *wd);
void rpwd(MINODE *wd);

//  cd
int my_chdir(char *pathname);

// mkdir
int make_dir(char *pathname);
int mymkdir(MINODE *pip, char *name);
int enter_name(MINODE *pip, int myino, char *myname);

// link
int link_wrapper(char *old, char *new);
int my_link(char *oldname, char *newname);

#endif