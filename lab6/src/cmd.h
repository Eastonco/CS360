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

//misclvl1
int mychmod(char *pathname);

//open_close_lseek
int open_file(char *pathname, int mode);
int mode_is_invalid(int mode);
int close_file(int fd);
int is_invalid_fd(int fd);
int my_lseek(int fd, int position);
int pfd(void);
int dup(int fd);
int dup2(int fd, int gd);

//write cp
int my_cp(char *src, char *dest);
int mywrite(int fd, char *buf, int n_bytes);
int write_file();


#endif