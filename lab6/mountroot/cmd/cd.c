//#include "../type.h"
#include "../cmd.h"

// globals
extern MINODE minode[NMINODE];
extern MINODE *root;

extern PROC proc[NPROC], *running;

extern char gpath[128]; // global for tokenized components
extern char *name[32];  // assume at most 32 components in pathname
extern int n;           // number of component strings

extern int fd, dev;
extern int nblocks, ninodes, bmap, imap, inode_start;
// Headers
int my_chdir(char *pathname);

/************************************************************
* Function: my_chdir(char *pathname)                        *
* Date Created: 11/4/2020                                   *
* Date Last Modified:                                       *
* Description: changes cwd to new pathname                  *
* Input parameters: path to new cwd                         *
* Returns: 1 if success, 0 if fail                          *
* Preconditions: must have initialized system               *
* Postconditions:                                           *
*************************************************************/
int my_chdir(char *pathname)
{
  printf("chdir %s\n", pathname);

  int ino = getino(pathname);
  if (ino == 0)
  {
    printf("ERROR: Chdir() - ino can't be found\n");
    return 0;
  }

  MINODE *mip = iget(dev, ino);

  if (!S_ISDIR(mip->INODE.i_mode))
  {
    printf("Error: Chdir() - mip is not a directory");
    return 0;
  }

  iput(running->cwd);
  running->cwd = mip;
  return 1;
}