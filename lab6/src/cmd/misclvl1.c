#include "../cmd.h"

// globals
extern MINODE minode[NMINODE];
extern MINODE *root;

extern PROC proc[NPROC], *running;

extern char gpath[128]; // global for tokenized components
extern char *name[32];  // assume at most 32 components in pathname
extern int n;           // number of component strings in name[]

extern int fd, dev;
extern int nblocks, ninodes, bmap, imap, inode_start;

int mychmod(char *pathname)
{
    int ino = getino(pathname);

    if(!ino){
        printf("ERROR: Inode does not exist\n");
        return -1;
    }

    MINODE *mip = iget(dev, ino);
    mip->INODE.i_mode |= 7; // not sure if this number is correct but does make file rwx
    mip->dirty = 1;
    iput(mip);
}