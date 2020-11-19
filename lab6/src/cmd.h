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

//creat
int my_creat(MINODE *pip, char *name);
int creat_file(char *pathname);

//rmdir
int rm_child(MINODE *parent, char *name);
int myrmdir(char *pathname);

// unlink
int my_unlink(char *pathname);
int inode_truncate(MINODE *mip);

// symlink
int my_symlink(char *old, char *new);

// rmdir
int myrmdir(char *pathname);
int is_empty(MINODE *mip);


#endif