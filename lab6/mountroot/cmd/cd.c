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

int chdir(char *pathname)
{
  printf("chdir %s\n", pathname);
  //printf("under construction READ textbook HOW TO chdir!!!!\n");
  // READ Chapter 11.7.3 HOW TO chdir

  int ino = getino(pathname);
  if (ino == 0)
  {
    printf("ERROR in chdir\n");
    return 0;
  }
  MINODE *mip = iget(dev, ino); // TODO: issue in iget, the matching ino node->refcout was never incrimented on creation, need to fix
  if (!S_ISDIR(mip->INODE.i_mode))
  {
    printf("Error: mip is not a directory");
    return 0;
  }
  iput(running->cwd);
  running->cwd = mip;
}

/*
(1). int ino = getino(pathname);  // return error if ino=0
(2). MINODE *mip = iget(dev, ino);
(3). Verify mip->INODE is a DIR // return error if not DIR  - verrify with 'S_ISDIR(mip->INODE.i_mode)'
(4). iput(running->cwd); // release old cwd 
(5). running->cwd = mip; // change cwd to mip
*/