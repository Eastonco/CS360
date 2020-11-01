/*********** util.c file ****************/
#include "type.h"

int get_block(int dev, int blk, char *buf)
{
  lseek(dev, (long)blk * BLKSIZE, 0);
  read(dev, buf, BLKSIZE);
}
int put_block(int dev, int blk, char *buf)
{
  lseek(dev, (long)blk * BLKSIZE, 0);
  write(dev, buf, BLKSIZE);
}

int tokenize(char *pathname)
{
  // copy pathname into gpath[]; tokenize it into name[0] to name[n-1]
  char *name[64];  // token string pointers
  char gline[256]; // holds token strings, each pointed by a name[i]
  int nname;       // number of token stringsP
  char *s;
  strcpy(gline, pathname);
  nname = 0;
  s = strtok(gline, "/");
  while (s)
  {
    name[nname++] = s;
    s = strtok(0, "/");
  }
}

MINODE *iget(int dev, int ino)
{
  // return minode pointer of loaded INODE=(dev, ino)
  MINODE *mip;
  MTABLE *mp;
  INODE *ip;
  int i, block, offset;
  char buf[BLKSIZE];
  // serach in-memory minodes first
  for (i = 0; i < NMINODES; i++)
  {
    MINODE *mip = &MINODE[i];
    if (mip->refCount && (mip->dev == dev) && (mip->ino == ino))
    {
      mip->refCount++;
      return mip;
    }
  }
  mip = mialloc(); // allocate a FREE minode
  mip->dev = dev;
  mip->ino = ino; // assign to (dev, ino)
  block = (ino - 1) / 8 + iblock;
  offset = (ino - 1) % 8;
  get_block(dev, block, buf);
  ip = (INODE *)buf + offset;
  mip->INODE = *ip;
  // initialize minode
  mip->refCount = 1;
  mip->mounted = 0;
  mip->dirty = 0;
  mip->mountptr = 0;
  return mip;
}

void iput(MINODE *mip)
{
  // dispose of minode pointed by mip
  // Code in Chapter 11.7.2
}

int search(MINODE *mip, char *name)
{
  // search for name in (DIRECT) data blocks of mip->INODE
  // return its ino
  // Code in Chapter 11.7.2
}

int getino(char *pathname)
{
  // return ino of pathname
  // Code in Chapter 11.7.2
}

int findmyname(MINODE *parent, u32 myino, char *myname)
{
  // WRITE YOUR code here:
  // search parent's data block for myino;
  // copy its name STRING to myname[ ];
}

int findino(MINODE *mip, u32 *myino) // myino = ino of . return ino of ..
{
  // mip->a DIR minode. Write YOUR code to get mino=ino of .
  //                                         return ino of ..
}
