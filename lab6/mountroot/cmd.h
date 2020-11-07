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

#endif