#ifndef UTIL_H
#define UTIL_H

#include "type.h"

// Function prototypes, main.c
int init();
int mount_root();
int quit();

// Function prototypes, util.c
int get_block(int dev, int blk, char *buf);
int put_block(int dev, int blk, char *buf);
int tokenize(char *pathname);
MINODE *mialloc();
MINODE *iget(int dev, int ino);
int iput(MINODE *mip);
int search(MINODE *mip, char *lname);
int getino(char *pathname);
int findmyname(MINODE *parent, u32 myino, char *myname);
int findino(MINODE *mip, u32 *myino);

int tst_bit(char *buf, int bitnum);
int set_bit(char *buf, int bitnum);
int clr_bit(char *buf, int bitnum);
int ialloc(int dev);
int balloc(int dev);
int decFreeBlocks(int dev);
int decFreeInodes(int dev);
int enter_name(MINODE *pip, int myino, char *myname);

int bdealloc(int dev, int bno);
int idealloc(int dev, int ino);
int incFreeInodes(int dev);
int incFreeBlocks(int dev);

int rm_child(MINODE *parent, char *name);
#endif