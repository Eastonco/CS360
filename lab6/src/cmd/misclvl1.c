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

/****************************************************************
* Function: mychmod(char *pathname)                             *
* Date Created: 11/18/2020                                      *
* Date Last Modified:                                           *
* Description: modifes permission of a given file               *
* Input parameters: pathname/file name                          *
* Returns: 0 if succes, 0 if inode doesn't exist                *
* Preconditions: file must exist                                *
* Postconditions:                                               *
*****************************************************************/
int mychmod(char *pathname)
{
    int ino = getino(pathname); // get the inode number from pathname

    if (ino == -1) // verrfies ino exists
    {
        printf("ERROR: Inode does not exist\n");
        return -1;
    }

    MINODE *mip = iget(dev, ino); // get the minode from memory
    mip->INODE.i_mode |= 7;       // not sure if this number is correct but does make file rwx
    mip->dirty = 1;               // mark as changed
    iput(mip);                    // write changes to memory
    return 0;
}