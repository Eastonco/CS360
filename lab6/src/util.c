/*********** util.c file ****************/
#include "type.h"

// globals
extern MINODE minode[NMINODE];
extern MINODE *root;

extern PROC proc[NPROC], *running;
extern MTABLE mount_table[NMOUNT];

extern char gpath[128]; // global for tokenized components
extern char *name[64];  // assume at most 32 components in pathname
extern int n;           // number of component strings

extern int fd, dev, root_dev;
extern int nblocks, ninodes, bmap, imap, inode_start;

/****************************************************************
* Function:                                                     *
* Date Created:                                                 *
* Date Last Modified:                                           *
* Description:                                                  *
* Input parameters:                                             *
* Returns:                                                      *
* Preconditions:                                                *
* Postconditions:                                               *
*****************************************************************/
int tst_bit(char *buf, int bit)
{
    return buf[bit / 8] & (1 << (bit % 8));
}

/****************************************************************
* Function:                                                     *
* Date Created:                                                 *
* Date Last Modified:                                           *
* Description:                                                  *
* Input parameters:                                             *
* Returns:                                                      *
* Preconditions:                                                *
* Postconditions:                                               *
*****************************************************************/
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

/****************************************************************
* Function:                                                     *
* Date Created:                                                 *
* Date Last Modified:                                           *
* Description:                                                  *
* Input parameters:                                             *
* Returns:                                                      *
* Preconditions:                                                *
* Postconditions:                                               *
*****************************************************************/
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

/****************************************************************
* Function:                                                     *
* Date Created:                                                 *
* Date Last Modified:                                           *
* Description:                                                  *
* Input parameters:                                             *
* Returns:                                                      *
* Preconditions:                                                *
* Postconditions:                                               *
*****************************************************************/
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

/****************************************************************
* Function:                                                     *
* Date Created:                                                 *
* Date Last Modified:                                           *
* Description:                                                  *
* Input parameters:                                             *
* Returns:                                                      *
* Preconditions:                                                *
* Postconditions:                                               *
*****************************************************************/
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
}

/****************************************************************
* Function:                                                     *
* Date Created:                                                 *
* Date Last Modified:                                           *
* Description:                                                  *
* Input parameters:                                             *
* Returns:                                                      *
* Preconditions:                                                *
* Postconditions:                                               *
*****************************************************************/
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

/****************************************************************
* Function:                                                     *
* Date Created:                                                 *
* Date Last Modified:                                           *
* Description:                                                  *
* Input parameters:                                             *
* Returns:                                                      *
* Preconditions:                                                *
* Postconditions:                                               *
*****************************************************************/
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

/****************************************************************
* Function:                                                     *
* Date Created:                                                 *
* Date Last Modified:                                           *
* Description:                                                  *
* Input parameters:                                             *
* Returns:                                                      *
* Preconditions:                                                *
* Postconditions:                                               *
*****************************************************************/
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

/****************************************************************
* Function:                                                     *
* Date Created:                                                 *
* Date Last Modified:                                           *
* Description:                                                  *
* Input parameters:                                             *
* Returns:                                                      *
* Preconditions:                                                *
* Postconditions:                                               *
*****************************************************************/
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

/****************************************************************
* Function:                                                     *
* Date Created:                                                 *
* Date Last Modified:                                           *
* Description:                                                  *
* Input parameters:                                             *
* Returns:                                                      *
* Preconditions:                                                *
* Postconditions:                                               *
*****************************************************************/
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

/****************************************************************
* Function: bdealloc(int dev, int bno)                          *
* Date Created: 11/18/2020                                      *
* Date Last Modified:                                           *
* Description: deallocates a disk block number                  *
* Input parameters: device id, block numbre                     *
* Returns: 0 on success                                         *
* Preconditions:                                                *
* Postconditions:                                               *
*****************************************************************/
int bdealloc(int dev, int bno)
{
    char buf[BLKSIZE]; // a sweet buffer

    get_block(dev, bmap, buf); // get the block
    clr_bit(buf, bno - 1);     // clear the bits to 0
    put_block(dev, bmap, buf); // write the block back
    incFreeBlocks(dev);        // increment the free block count
    return 0;
}

/****************************************************************
* Function: get_block(int dev, int blk, char *buf)              *
* Date Created: 10/?/2020                                       *
* Date Last Modified:                                           *
* Description: gets memory block from the device                *
* Input parameters: device id, block number, a buffer to read   *
*                   into                                        *
* Returns: a filled buffer via pointer                          *
* Preconditions:                                                *
* Postconditions:                                               *
*****************************************************************/
int get_block(int dev, int blk, char *buf)
{
    lseek(dev, (long)blk * BLKSIZE, 0); // seek to block number
    read(dev, buf, BLKSIZE);            // read into buffer
}

/****************************************************************
* Function: put_block(int dev, int blk, char *buf)              *
* Date Created: 10/?/2020                                       *
* Date Last Modified:                                           *
* Description: wries memory block to device                     *
* Input parameters: device id, block number, a buffer to write  *
*                   from                                        *
* Returns: n/a                                                  *
* Preconditions:                                                *
* Postconditions:                                               *
*****************************************************************/
int put_block(int dev, int blk, char *buf)
{
    lseek(dev, (long)blk * BLKSIZE, 0); // seek to block number
    write(dev, buf, BLKSIZE);           // write from buffer
}

/****************************************************************
* Function: tokenize(char *pathname)                            *
* Date Created: 10/?/2020                                       *
* Date Last Modified:                                           *
* Description: copy pathname into gpath[],                      *
*               tokenize it into name[0] to name[n-1]           *
* Input parameters: pathname                                    *
* Returns: number of tokens -> writes to global gpath[]         *
*                                           and name[]          *
* Preconditions:                                                *
* Postconditions:                                               *
*****************************************************************/
int tokenize(char *pathname)
{

    char *s;
    strcpy(gpath, pathname); // copy pathname to global gpath[]
    n = 0;
    s = strtok(gpath, "/"); // tokenize the path
    while (s)               // while there is a token
    {
        name[n++] = s;      // write the token to name[n]
        s = strtok(0, "/"); // tokenize again
    }
    return n;
}

/****************************************************************
* Function: mialloc()                                           *
* Date Created: 10/?/2020                                       *
* Date Last Modified:                                           *
* Description: allocate a FREE minode for use                   *
* Input parameters: n/a                                         *
* Returns: newly allocated minode, 0 if fail                    *
* Preconditions:                                                *
* Postconditions:                                               *
*****************************************************************/
MINODE *mialloc()
{
    int i;
    for (i = 0; i < NMINODE; i++) // loop through minode limit
    {
        MINODE *mp = &minode[i]; // pointer to current minode
        if (mp->refCount == 0)   // if refcount == 0, it's unallocated
        {
            mp->refCount = 1; // claim the mindoe
            return mp;        // return minode
        }
    }
    printf("FS panic: out of minodes\n"); // if it gets this far then we're out of minodes to allocate -> fail
    return 0;
}

/****************************************************************
* Function:  midalloc(MINODE *mip)                              *
* Date Created: 10/?/2020                                       *
* Date Last Modified:                                           *
* Description: release a used minode                            *
* Input parameters: minode to be deallocated                    *
* Returns: 0 on success                                         *
* Preconditions:                                                *
* Postconditions:                                               *
*****************************************************************/
int midalloc(MINODE *mip)
{
    mip->refCount = 0; // by setting refcount to 0 -> becomes viewed as unclaimed
    return 0;
}

/****************************************************************
* Function: iget(int dev, int ino)                              *
* Date Created: 10/?/2020                                       *
* Date Last Modified:                                           *
* Description: gets the minode from memory via inode nunber     *
* Input parameters: device id, inode nunber                     *
* Returns: the minode                                           *
* Preconditions:                                                *
* Postconditions:                                               *
*****************************************************************/
MINODE *iget(int dev, int ino)
{
    MINODE *mip;
    MTABLE *mp;
    INODE *ip;
    int i, block, offset;
    char buf[BLKSIZE]; // a siiiiick buffer

    // serach in-memory minodes first
    for (i = 0; i < NMINODE; i++)
    {
        mip = &minode[i];                                            // set minode to minode at this position
        if (mip->refCount && (mip->dev == dev) && (mip->ino == ino)) // if it has a refcount and it's in the device,
        {                                                            //and matches our inode numnber
            mip->refCount++;                                         // increment refcount
            return mip;                                              // return the minode
        }
    }

    // needed INODE=(dev,ino) not in memory
    mip = mialloc();                     // allocate a FREE minode
    mip->dev = dev;                      // set the device to current
    mip->ino = ino;                      // assign to (dev, ino)
    block = (ino - 1) / 8 + inode_start; // find the block to read from
    offset = (ino - 1) % 8;
    get_block(dev, block, buf); // read the block
    ip = (INODE *)buf + offset;
    mip->INODE = *ip; // set pointer to block

    // initialize minode
    mip->refCount = 1;
    mip->mounted = 0;
    mip->dirty = 0;
    mip->mptr = 0;
    return mip;
}

/****************************************************************
* Function:                                                     *
* Date Created:                                                 *
* Date Last Modified:                                           *
* Description:                                                  *
* Input parameters:                                             *
* Returns:                                                      *
* Preconditions:                                                *
* Postconditions:                                               *
*****************************************************************/
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
    //midalloc(mip);                   // mip->refCount = 0;
    return 0;
}

/****************************************************************
* Function:                                                     *
* Date Created:                                                 *
* Date Last Modified:                                           *
* Description:                                                  *
* Input parameters:                                             *
* Returns:                                                      *
* Preconditions:                                                *
* Postconditions:                                               *
*****************************************************************/
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

/****************************************************************
* Function:                                                     *
* Date Created:                                                 *
* Date Last Modified:                                           *
* Description:                                                  *
* Input parameters:                                             *
* Returns:                                                      *
* Preconditions:                                                *
* Postconditions:                                               *
*****************************************************************/
int getino(char *pathname)
{
    // return ino of pathname
    MINODE *mip, *mip_two;
    int i, ino;
    if (strcmp(pathname, "/") == 0)
        return 2; // return root ino = 2
    if (pathname[0] == '/') {
        //mip = root; // if absolute pathname: start from root
        dev = root->dev;
        ino = root->ino;
    } 
    else
    {
        //mip = running->cwd; // if relative pathname: start from cwd
        dev = running->cwd->dev;
        ino = running->cwd->ino;
    }

    mip = iget(dev, ino);

    tokenize(pathname); // assume: name[], nname are globals

    for (i = 0; i < n; i++)
    { //search for each component string
        if (!S_ISDIR(mip->INODE.i_mode))
        {
            printf("%s is not a directory\n", name[i]);
            mip->dirty = 1;
            iput(mip);
            return -1;
        }

        printf("inode #: %d\n", mip->ino);

        ino = search(mip, name[i]);

        if (!ino)
        {
            printf("no such component name %s\n", name[i]);
            mip->dirty = 1;
            iput(mip);
            return -1;
        }
        else if (ino == 2 && dev != root_dev)
        { // root of mount, but dev # is not dev # of real root
            // upwards traversal!
            // locate mount table entry via dev #
            printf("UPWARDS\n");
            for (int i = 0; i < NMOUNT; i++)
            {
                if (mount_table[i].dev == dev)
                {
                    // switch to minode pointed to by mount table entry and continue
                    iput(mip);
                    mip = mount_table[i].mntDirPtr;
                    dev = mip->dev;
                    break;
                }
            }
        }
        else 
        {
            mip->dirty = 1;
            iput(mip);
            mip = iget(dev, ino);

            // downward traversal
            if (mip->mounted)
            {
                MTABLE *mtptr = mip->mptr;
                dev = mtptr->dev;
                ino = 2;
                printf("get dev of %d\n", dev);
                iput(mip);
                mip = iget(dev, ino); // get root INODE into memory
                // seach for x under root INODE of mounted device
            }
        }
    }

    mip->dirty = 1;
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

/****************************************************************
* Function: enter_name(MINODE *pip, int myino, char *myname)    *
* Date Created: 11/12/2020                                      *
* Date Last Modified:                                           *
* Description: adds the name of a dir to the block entry        *
* Input parameters: parent minode, inode number, name of file   *
* Returns: 1 if allocating a new block, 0 if adding to block    *
* Preconditions:                                                *
* Postconditions:                                               *
*****************************************************************/
int enter_name(MINODE *pip, int myino, char *myname)
{
    char buf[BLKSIZE], *cp;
    int bno;
    INODE *ip;
    DIR *dp;

    int need_len = 4 * ((8 + strlen(myname) + 3) / 4); //ideal length of entry

    ip = &pip->INODE; // get the inode

    for (int i = 0; i < 12; i++)
    {

        if (ip->i_block[i] == 0)
        {
            break;
        }

        bno = ip->i_block[i];
        get_block(pip->dev, ip->i_block[i], buf); // get the block
        dp = (DIR *)buf;
        cp = buf;

        while (cp + dp->rec_len < buf + BLKSIZE) // Going to last entry of the block
        {
            printf("%s\n", dp->name);
            cp += dp->rec_len;
            dp = (DIR *)cp;
        }

        // at last entry
        int ideal_len = 4 * ((8 + dp->name_len + 3) / 4); // ideal len of the name
        int remainder = dp->rec_len - ideal_len;          // remaining space

        if (remainder >= need_len)
        {                            // space available for new netry
            dp->rec_len = ideal_len; //trim current entry to ideal len
            cp += dp->rec_len;       // advance to end
            dp = (DIR *)cp;          // point to new open entry space

            dp->inode = myino;             // add the inode
            strcpy(dp->name, myname);      // add the name
            dp->name_len = strlen(myname); // len of name
            dp->rec_len = remainder;       // size of the record

            put_block(dev, bno, buf); // save block
            return 0;
        }
        else
        {                         // not enough space in block
            ip->i_size = BLKSIZE; // size is new block
            bno = balloc(dev);    // allocate new block
            ip->i_block[i] = bno; // add the block to the list
            pip->dirty = 1;       // ino is changed so make dirty

            get_block(dev, bno, buf); // get the blcok from memory
            dp = (DIR *)buf;
            cp = buf;

            dp->name_len = strlen(myname); // add name len
            strcpy(dp->name, myname);      // name
            dp->inode = myino;             // inode
            dp->rec_len = BLKSIZE;         // only entry so full size

            put_block(dev, bno, buf); //save
            return 1;
        }
    }
}

/****************************************************************
* Function: rm_child(MINODE *parent, char *name)                *
* Date Created: 11/18/2020                                      *
* Date Last Modified:                                           *
* Description: removes the child from the parents child list    *
* Input parameters: parent Minode, name of child to be removed  *
* Returns: 0 if success, -1 if fail                             *
* Preconditions:                                                *
* Postconditions:                                               *
*****************************************************************/
int rm_child(MINODE *parent, char *name)
{
    DIR *dp, *prevdp, *lastdp;
    char *cp, *lastcp, buf[BLKSIZE], tmp[256], *startptr, *endptr;
    INODE *ip = &parent->INODE;

    for (int i = 0; i < 12; i++) // loop through all 12 blocks of memory
    {
        if (ip->i_block[i] != 0)
        {
            get_block(parent->dev, ip->i_block[i], buf); // get block from file
            dp = (DIR *)buf;
            cp = buf;

            while (cp < buf + BLKSIZE) // while not at the end of the block
            {
                strncpy(tmp, dp->name, dp->name_len); // copy name
                tmp[dp->name_len] = 0;                // add name delimiter

                if (!strcmp(tmp, name)) // name found
                {
                    if (cp == buf && cp + dp->rec_len == buf + BLKSIZE) // first/only record
                    {
                        bdealloc(parent->dev, ip->i_block[i]);
                        ip->i_size -= BLKSIZE;

                        while (ip->i_block[i + 1] != 0 && i + 1 < 12) // filling hole in the i_blocks since we deallocated this one
                        {
                            i++;
                            get_block(parent->dev, ip->i_block[i], buf);
                            put_block(parent->dev, ip->i_block[i - 1], buf);
                        }
                    }

                    else if (cp + dp->rec_len == buf + BLKSIZE) // Last record in the block, previous absorbs size
                    {
                        prevdp->rec_len += dp->rec_len;
                        put_block(parent->dev, ip->i_block[i], buf);
                    }

                    else // Record between others, must shift
                    {
                        lastdp = (DIR *)buf;
                        lastcp = buf;

                        while (lastcp + lastdp->rec_len < buf + BLKSIZE) // finding last record in the block
                        {
                            lastcp += lastdp->rec_len;
                            lastdp = (DIR *)lastcp;
                        }

                        lastdp->rec_len += dp->rec_len; // adding size to last one

                        startptr = cp + dp->rec_len; // start of copy block
                        endptr = buf + BLKSIZE;      // end of copy block

                        memmove(cp, startptr, endptr - startptr); // Shift left
                        put_block(parent->dev, ip->i_block[i], buf);
                    }

                    parent->dirty = 1;
                    iput(parent);
                    return 0;
                }

                prevdp = dp;
                cp += dp->rec_len;
                dp = (DIR *)cp;
            }
        }
    }
    printf("ERROR: child not found\n");
    return -1;
}

// use inodes in block, go to address, free them (memset), 
int inode_truncate(MINODE *mip) {
    char buf[BLKSIZE];
    INODE *ip = &mip->INODE;
    // 12 direct blocks
    for (int i = 0; i < 12; i++) {
        if (ip->i_block[i] == 0)
            break;
        // now deallocate block
        bdealloc(dev, ip->i_block[i]);
        ip->i_block[i] = 0;
    }
    // now worry about indirect blocks and doubly indirect blocks
    // (see pp. 762 in ULK for visualization of data blocks)
    // indirect blocks:
    if (ip->i_block[12] != NULL) {
        get_block(dev, ip->i_block[12], buf); // follow the ptr to the block
        int *ip_indirect = (int *)buf; // reference to indirect block via integer ptr
        int indirect_count = 0;
        while (indirect_count < BLKSIZE / sizeof(int)) { // split blksize into int sized chunks (4 bytes at a time)
            if (ip_indirect[indirect_count] == 0)
                break;
            // deallocate indirect block
            bdealloc(dev, ip_indirect[indirect_count]);
            ip_indirect[indirect_count] = 0;
            indirect_count++;
        }
        // now all indirect blocks have been dealt with, deallocate reference to indirect
        bdealloc(dev, ip->i_block[12]);
        ip->i_block[12] = 0;
    }

    // doubly indirect blocks (same code as above, different variables):
    if (ip->i_block[13] != NULL) {
        get_block(dev, ip->i_block[13], buf);
        int *ip_doubly_indirect = (int *)buf;
        int doubly_indirect_count = 0;
        while (doubly_indirect_count < BLKSIZE / sizeof(int)) {
            if (ip_doubly_indirect[doubly_indirect_count] == 0)
                break;
            // deallocate doubly indirect block
            bdealloc(dev, ip_doubly_indirect[doubly_indirect_count]);
            ip_doubly_indirect[doubly_indirect_count] = 0;
            doubly_indirect_count++;
        }
        bdealloc(dev, ip->i_block[13]);
        ip->i_block[13] = 0;
    }

    mip->INODE.i_blocks = 0;
    mip->INODE.i_size = 0;
    mip->dirty = 1;
    iput(mip);
}