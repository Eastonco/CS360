#include "../type.h"

// globals
extern MINODE minode[NMINODE];
extern MINODE *root;

extern PROC   proc[NPROC], *running;

extern char gpath[128]; // global for tokenized components
extern char *name[32];  // assume at most 32 components in pathname
extern int   n;         // number of component strings

extern int fd, dev;
extern int nblocks, ninodes, bmap, imap, inode_start;

int chdir(char *pathname)   
{
  printf("chdir %s\n", pathname);
  printf("under construction READ textbook HOW TO chdir!!!!\n");
  // READ Chapter 11.7.3 HOW TO chdir
}