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

int tst_bit(char *buf, int bit)
{
    return buf[bit / 8] & (1 << (bit % 8));
}

int set_bit(char *buf, int bitnum)
{
    int bit, byte;
    byte = bitnum / 8;
    bit = bitnum % 8;
    if (buf[byte] |= (1 << bit))
    {
        return 1;
    }
    return 0;
}

int clr_bit(char *buf, int bitnum)
{
    int bit, byte;
    byte = bitnum / 8;
    bit = bitnum % 8;
    if (buf[byte] &= ~(1 << bit))
    {
        return 1;
    }
    return 0;
}

int decFreeInodes(int dev)
{
    char buf[BLKSIZE];

    get_block(dev, 1, buf);
    sp = (SUPER *)buf;
    sp->s_free_inodes_count--;
    put_block(dev, 1, buf);

    get_block(dev, 2, buf);
    gp = (GD *)buf;
    gp->bg_free_inodes_count--;
    put_block(dev, 2, buf);
}

int decFreeBlocks(int dev)
{
    char buf[BLKSIZE];

    get_block(dev, 1, buf);
    sp = (SUPER *)buf;
    sp->s_free_blocks_count--;
    put_block(dev, 1, buf);

    get_block(dev, 2, buf);
    gp = (GD *)buf;
    gp->bg_free_blocks_count--;
    put_block(dev, 2, buf);

    //nblocks--;
}

int incFreeBlocks(int dev)
{
    char buf[BLKSIZE];

    get_block(dev, 1, buf);
    sp = (SUPER *)buf;
    sp->s_free_blocks_count++;
    put_block(dev, 1, buf);

    get_block(dev, 2, buf);
    gp = (GD *)buf;
    gp->bg_free_blocks_count++;
    put_block(dev, 2, buf);
}

int incFreeInodes(int dev)
{
    char buf[BLKSIZE];
    // inc free inodes count in SUPER and GD
    get_block(dev, 1, buf);
    sp = (SUPER *)buf;
    sp->s_free_inodes_count++;
    put_block(dev, 1, buf);
    get_block(dev, 2, buf);
    gp = (GD *)buf;
    gp->bg_free_inodes_count++;
    put_block(dev, 2, buf);
}

int ialloc(int dev) // allocate an inode number from inode_bitmap
{
    int i;
    char buf[BLKSIZE];

    get_block(dev, imap, buf); // read inode_bitmap block

    for (i = 0; i < ninodes; i++)
    {
        if (tst_bit(buf, i) == 0)
        {
            set_bit(buf, i);
            put_block(dev, imap, buf);
            decFreeInodes(dev);
            printf("allocated ino = %d\n", i + 1); // bits count from 0; ino from 1
            return i + 1;
        }
    }
    return 0;
}

int balloc(int dev)
{ //returns a FREE disk block number  NOTE: Not 100% sure if this works
    int i;
    char buf[BLKSIZE];

    get_block(dev, bmap, buf);

    for (i = 0; i < nblocks; i++)
    {
        if (tst_bit(buf, i) == 0)
        {
            set_bit(buf, i);
            decFreeBlocks(dev);
            put_block(dev, bmap, buf);
            printf("Free disk block at %d\n", i + 1); // bits count from 0; ino from 1
            return i + 1;
        }
    }
    return 0;
}

int idealloc(int dev, int ino)
{ // deallocating inode (number)
    int i;
    char buf[BLKSIZE];

    if (ino > ninodes)
    {
        printf("inumber %d out of range\n", ino);
        return;
    }
    get_block(dev, imap, buf);
    clr_bit(buf, ino - 1);
    // write buf back
    put_block(dev, imap, buf);
    // update free inode count in SUPER and GD
    incFreeInodes(dev);
}

int bdealloc(int dev, int bno)
{ // deallocating disk block number
    char buf[BLKSIZE];

    get_block(dev, bmap, buf);
    clr_bit(buf, bno-1); // I (Zach) wrote this same exact code for unlink's truncate and I was off by 1, so updated this here
    put_block(dev, bmap, buf);
    incFreeBlocks(dev);
    /*
    get_block(dev, 1, buf);
    sp = (SUPER *)buf;
    sp->s_free_blocks_count++;
    put_block(dev, 1, buf);

    get_block(dev, 2, buf);
    gp = (GD *)buf;
    gp->bg_free_blocks_count++;
    put_block(dev, 2, buf);
    */
}

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
        return -1;
    mip->refCount--;
    if (mip->refCount > 0)
        return -1;
    if (mip->dirty == 0)
        return -1;
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
    return 0;
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
            return -1;
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
    if (pathname[0] == '/')
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

/****************************************************************
* Function: findmyname(MINODE *parent, u32 myino, char *myname) *
* Date Created: 11/4/2020                                       *
* Date Last Modified:                                           *
* Description: Searches for an inode and returns its name.      *
* Input parameters: MINODE ptr parent, u32 current dir          *
*                   myino, return parameter string myname.      *
* Returns: Name of inode indirectly through myname.             *
* Preconditions: parent must point to a valid MINODE, myino     *
*               must be a declared u32 variable, myname is a    *
*               valid string.                                   *
* Postconditions:                                               *
*****************************************************************/
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

/************************************************************
* Function: findino(MINODE *mip, u32 *myino)                *
* Date Created: 11/4/2020                                   *
* Date Last Modified:                                       *
* Description: Finds the inode of current dir (.) and       *
*                parent dir (..).                           *
* Input parameters:  myino output parameter, pointer to     *
*                   MINODE mip.                             *
* Returns: inode of . indirectly, inode of .. directly.     *
* Preconditions: mip must point to a valid MINODE, myino    *
*               must be a declared u32 variable pointer.    *
* Postconditions:                                           *
*************************************************************/
// reads a block of memory in i_block[0] and copies it to a buffer.
// Uses that buffer to get the inode of '.' (after casting to a DIR *)
// Then iterates that pointer to the buffer by the length of the directory entry (dp->rec_len)
// repeat, now use the DIR * pointer to get the inode of '..' and return
int findino(MINODE *mip, u32 *myino) // myino = ino of . return ino of ..
{
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

int enter_name(MINODE *pip, int myino, char *myname)
{
    char buf[BLKSIZE], *cp;
    int bno;
    INODE *ip;
    DIR *dp;

    int need_len = 4 * ((8 + strlen(myname) + 3) / 4); //ideal length of entry

    ip = &pip->INODE;

    for (int i = 0; i < 12; i++)
    {

        if (ip->i_block[i] == 0)
        {
            break;
        }

        bno = ip->i_block[i];
        get_block(pip->dev, ip->i_block[i], buf);
        dp = (DIR *)buf;
        cp = buf;

        // Going to last entry of the block
        while (cp + dp->rec_len < buf + BLKSIZE)
        {
            printf("%s\n", dp->name);
            cp += dp->rec_len;
            dp = (DIR *)cp;
        }

        // at last entry
        int ideal_len = 4 * ((8 + dp->name_len + 3) / 4);
        int remainder = dp->rec_len - ideal_len;

        if (remainder >= need_len)
        {                            // space available for new netry
            dp->rec_len = ideal_len; //trim current entry to ideal len
            cp += dp->rec_len;       // advance to end
            dp = (DIR *)cp;          // point to new open entry space

            dp->inode = myino;
            strcpy(dp->name, myname);
            dp->name_len = strlen(myname);
            dp->rec_len = remainder;

            put_block(dev, bno, buf);
            return 0;
        }
        else
        { // not enough space in block
            ip->i_size = BLKSIZE;
            bno = balloc(dev); // allocate new block
            ip->i_block[i] = bno;
            pip->dirty = 1; // ino is changed so make dirty

            get_block(dev, bno, buf);
            dp = (DIR *)buf;
            cp = buf;

            dp->name_len = strlen(myname);
            strcpy(dp->name, myname);
            dp->inode = myino;
            dp->rec_len = BLKSIZE; // only entry so full size

            put_block(dev, bno, buf); //save
            return 1;
        }
    }
}