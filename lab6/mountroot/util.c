/*********** util.c file ****************/
#include "type.h"

// globals
extern MINODE minode[NMINODE];
extern MINODE *root;

extern PROC proc[NPROC], *running;

extern char gpath[128]; // global for tokenized components
extern char *name[64];  // assume at most 32 components in pathname
extern int n;           // number of component strings

extern int fd, dev;
extern int nblocks, ninodes, bmap, imap, inode_start;

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
    char *s;
    strcpy(gpath, pathname);
    n = 0;
    s = strtok(gpath, "/");
    while (s)
    {
        name[n++] = s;
        s = strtok(0, "/");
    }
    return n;
}

MINODE *mialloc() // allocate a FREE minode for use
{
    int i;
    for (i = 0; i < NMINODE; i++)
    {
        MINODE *mp = &minode[i];
        if (mp->refCount == 0)
        {
            mp->refCount = 1;
            return mp;
        }
    }
    printf("FS panic: out of minodes\n");
    return 0;
}

int midalloc(MINODE *mip) // release a used minode
{
    mip->refCount = 0;
}

MINODE *iget(int dev, int ino)
{
    MINODE *mip;
    MTABLE *mp;
    INODE *ip;
    int i, block, offset;
    char buf[BLKSIZE];
    // serach in-memory minodes first
    for (i = 0; i < NMINODE; i++)
    {
        mip = &minode[i];
        if (mip->refCount && (mip->dev == dev) && (mip->ino == ino))
        {
            mip->refCount++;
            return mip;
        }
    }
    // needed INODE=(dev,ino) not in memory
    mip = mialloc(); // allocate a FREE minode
    mip->dev = dev;
    mip->ino = ino; // assign to (dev, ino)
    block = (ino - 1) / 8 + inode_start;
    offset = (ino - 1) % 8;
    get_block(dev, block, buf);
    ip = (INODE *)buf + offset;
    mip->INODE = *ip;
    // initialize minode
    mip->refCount = 1;
    mip->mounted = 0;
    mip->dirty = 0;
    mip->mptr = 0;
    return mip;
}

int iput(MINODE *mip)
{
    INODE *ip;
    int i, block, offset;
    char buf[BLKSIZE];
    if (mip == 0)
        return;
    mip->refCount--;
    if (mip->refCount > 0)
        return;
    if (mip->dirty == 0)
        return;
    // dec refCount by 1
    // still has user
    // no need to write back
    // write INODE back to disk
    block = (mip->ino - 1) / 8 + inode_start;
    offset = (mip->ino - 1) % 8;
    // get block containing this inode
    get_block(mip->dev, block, buf);
    ip = (INODE *)buf + offset;      // ip points at INODE
    *ip = mip->INODE;                // copy INODE to inode in block
    put_block(mip->dev, block, buf); // write back to disk
    midalloc(mip);                   // mip->refCount = 0;
}

int search(MINODE *mip, char *lname)
{
    // search for name in (DIRECT) data blocks of mip->INODE
    // return its ino

    int i;
    char *cp, temp[256], sbuf[BLKSIZE];
    DIR *dp;
    for (i = 0; i < 12; i++)
    { //Search DIR direct blcoks only
        if (mip->INODE.i_block[i] == 0)
            return 0;
        get_block(mip->dev, mip->INODE.i_block[i], sbuf);
        dp = (DIR *)sbuf;
        cp = sbuf;
        while (cp < sbuf + BLKSIZE)
        {
            strncpy(temp, dp->name, dp->name_len);
            temp[dp->name_len] = 0;
            printf("%8d%8d%8u %s\n",
                   dp->inode, dp->rec_len, dp->name_len, temp);
            if (strcmp(lname, temp) == 0)
            {
                printf("found %s : inumber = %d\n", lname, dp->inode);
                return dp->inode;
            }
            cp += dp->rec_len;
            dp = (DIR *)cp;
        }
    }
    return 0;
}

int getino(char *pathname)
{
    // return ino of pathname
    MINODE *mip;
    int i, ino;
    if (strcmp(pathname, "/") == 0)
        return 2; // return root ino = 2
    if (pathname[0] == "/")
        mip = root; // if absolute pathname: start from root
    else
    {
        mip = running->cwd; // if relative pathname: start from cwd
    }

    mip->refCount++;

    tokenize(pathname); // assume: name[], nname are globals

    for (i = 0; i < n; i++)
    { //search for each component string
        if (!S_ISDIR(mip->INODE.i_mode))
        {
            printf("%s is not a directory\n", name[i]);
            iput(mip);
            return 0;
        }
        ino = search(mip, name[i]);
        if (!ino)
        {
            printf("no such component name %s\n", name[i]);
            iput(mip);
            return 0;
        }
        iput(mip);
        mip = iget(dev, ino);
    }
    iput(mip);
    return ino;
}

// code is *very* similar to search-- just copies to myname instead of returning inode
int findmyname(MINODE *parent, u32 myino, char *myname)
{
    // WRITE YOUR code here:
    // search parent's data block for myino;
    // copy its name STRING to myname[ ];
    int i;
    char *cp, temp[256], sbuf[BLKSIZE];
    DIR *dp;
    MINODE *mip = parent;

    for (i = 0; i < 12; i++)
    { //Search DIR direct blcoks only
        if (mip->INODE.i_block[i] == 0)
            return -1;
        get_block(mip->dev, mip->INODE.i_block[i], sbuf);
        dp = (DIR *)sbuf;
        cp = sbuf;
        while (cp < sbuf + BLKSIZE)
        {
            strncpy(temp, dp->name, dp->name_len);
            temp[dp->name_len] = 0;
            
            if (dp->inode == myino)
            {
                strncpy(myname, dp->name, dp->name_len);
                myname[dp->name_len] = 0;
                return 0;
            }
            cp += dp->rec_len;
            dp = (DIR *)cp;
        }
    }
    return -1;
}

// reads a block of memory in i_block[0] and copies it to a buffer.
// Uses that buffer to get the inode of '.' (after casting to a DIR *)
// Then iterates that pointer to the buffer by the length of the directory entry (dp->rec_len)
// repeat, now use the DIR * pointer to get the inode of '..' and return
int findino(MINODE *mip, u32 *myino) // myino = ino of . return ino of ..
{
    // mip->a DIR minode. Write YOUR code to get mino=ino of .
    //                                         return ino of ..
    char buf[BLKSIZE], *temp_ptr;
    DIR *dp;

    get_block(mip->dev, mip->INODE.i_block[0], buf);
    temp_ptr = buf;
    dp = (DIR *)buf;
    *myino = dp->inode;
    temp_ptr += dp->rec_len;
    dp = (DIR *)temp_ptr;
    return dp->inode;
}
