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

/* FOR rpwd
(1). if (wd==root) return;
(2). from wd->INODE.i_block[0], get my_ino and parent_ino
(3). pip = iget(dev, parent_ino);
(4). from pip->INODE.i_block[ ]: get my_name string by my_ino as LOCAL (5). rpwd(pip); // recursive call rpwd(pip) with parent minode
(6). print "/%s", my_name;
*/