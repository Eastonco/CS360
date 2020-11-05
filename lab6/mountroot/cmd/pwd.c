#include "../type.h"

// globals
extern MINODE minode[NMINODE];
extern MINODE *root;

extern PROC proc[NPROC], *running;

extern char gpath[128]; // global for tokenized components
extern char *name[32];  // assume at most 32 components in pathname
extern int n;           // number of component strings

extern int fd, dev;
extern int nblocks, ninodes, bmap, imap, inode_start;

/************************************************************
* Function: pwd(MINODE *wd)                                 *
* Date Created: 11/4/2020                                   *
* Date Last Modified:                                       *
* Description: Prints the working directory to console      *
* Input parameters:  working directory MINODE               *
* Returns: NULL - Prints out the working dir                *
* Preconditions: wd and root must bet set to a node         *
* Postconditions:                                           *
*************************************************************/
char *pwd(MINODE *wd)
{
    if (wd == root)
    {
        printf("/\n");
    }
    else
    {
        rpwd(wd);
        printf("\n");
    }
}

/************************************************************
* Function:rpwd(MINODE *wd)                                 *
* Date Created: 11/4/2020                                   *
* Date Last Modified:                                       *
* Description: Recursive helper to pwd                      *
* Input parameters: MINODE of directory to be printed       *
* Returns: NULL - Prints out the working dir                *
* Preconditions: wd and root must bet set to a node         *
* Postconditions:                                           *
*************************************************************/
void rpwd(MINODE *wd)
{
    if (wd == root)
    {
        return;
    }
    char buf[BLKSIZE], lname[256];
    int ino;
    get_block(dev, wd->INODE.i_block[0], buf);
    int parent_ino = findino(wd, &ino);
    MINODE *pip = iget(dev, parent_ino);

    findmyname(pip, ino, lname);
    rpwd(pip);
    iput(pip);
    printf("/%s", lname);
    return;
}