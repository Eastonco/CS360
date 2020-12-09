//#include "../type.h"
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
* Function: int myrmdir(char *pathname)                         *
* Date Created: 11/19/2020                                      *
* Date Last Modified:                                           *
* Description: Removes a directory pathname                     *
* Input parameters:  pathname to the dir                        *
* Returns: 0 if success -1 if fail                              *
* Preconditions: must be an empty, free dir to remove           *
* Postconditions:                                               *
*****************************************************************/
int myrmdir(char *pathname)
{
    int ino = getino(pathname); // get the inode number from the name

    if (ino == -1) // make sure it's vaild
    {
        printf("ERROR: ino does not exist\n");
        return -1;
    }

    MINODE *mip = iget(dev, ino); // get the inode itself

    if (!S_ISDIR(mip->INODE.i_mode)) // verrify  it's a dir
    {
        printf("ERROR: node is not a directory\n");
        return -1;
    }

    if (!is_empty(mip)) // verrify it's empty
    {
        printf("ERROR: Dir is not empty\n");
        return -1;
    }

    if (mip->refCount > 2) // verrify it's not being used
    {
        printf("ERROR: node is busy, refcount > 2, refcount = %d\n", mip->refCount);
        return -1;
    }

    // mask out upper bits to just lower 9 bits (AND with 0x1FF to get lower 9 bits)
    uint other = mip->INODE.i_mode & 0x7;          // low 3 bits
    uint group = (mip->INODE.i_mode & 0x38) >> 3;  // mid 3 bits shifted right 3 spaces (bitmasked with 0b111000)
    uint owner = (mip->INODE.i_mode & 0x1C0) >> 6; // upper 3 bits shifted right 6 spaces (bitmasked with 0b111000000)
    if (!mip->INODE.i_mode & 0x1FF)
    {
        printf("ERROR: no permission\n");
        return -1;
    }

    if (running->uid != mip->INODE.i_uid && running->uid != SUPER_USER) {
        printf("ERROR: uid mismatch, no permission\n");
        return -1;
    }

    for (int i = 0; i < 12; i++)
    {
        if (mip->INODE.i_block[i] == 0)
            continue;
        bdealloc(mip->dev, mip->INODE.i_block[i]);
    }
    idealloc(mip->dev, mip->ino);

    mip->dirty = 1;
    iput(mip); //which clears mip->refCount = 0

    char cp1[256], cp2[256], *parent, *child; // temp character buffers since basename and dirname are destructive
    strcpy(cp1, pathname);
    strcpy(cp2, pathname);

    parent = dirname(cp1);
    child = basename(cp2);

    int pino = getino(parent); // get the parent inode number
    if (pino == -1)
    {
        printf("error finding parent inode, rmdir\n");
        return -1;
    }

    MINODE *pip = iget(mip->dev, pino); // get the parent inode
    rm_child(pip, child);               // remove the dir from the parent children list

    pip->INODE.i_links_count--;
    pip->INODE.i_atime = time(0L);
    pip->INODE.i_ctime = time(0L);
    pip->dirty = 1;

    iput(pip);

    return 0;
}

/****************************************************************
* Function: int is_empty(MINODE *mip)                           *
* Date Created: 11/19/2020                                      *
* Date Last Modified:                                           *
* Description: verrifies a dir is empty                         *
* Input parameters: MINODE to the dir                           *
* Returns: 1 if empty 0 if not                                  *
* Preconditions: must be a dir minode                           *
* Postconditions:                                               *
*****************************************************************/
int is_empty(MINODE *mip)
{
    char buf[BLKSIZE], *cp, temp[256];
    DIR *dp;
    INODE *ip = &mip->INODE;

    if (ip->i_links_count > 2) // make sure there aren't any dirs inside -- links count only looks at dirs, still could have files
    {
        return 0;
    }
    else if (ip->i_links_count == 2) // only the 2 dirs '.' & '..' -> check files
    {
        for (int i = 0; i < 12; i++) // Search DIR direct blocks only
        {
            if (ip->i_block[i] == 0)
                break;
            get_block(mip->dev, mip->INODE.i_block[i], buf); // read the blocks
            dp = (DIR *)buf;
            cp = buf;

            while (cp < buf + BLKSIZE) // while not at the end of the block
            {
                strncpy(temp, dp->name, dp->name_len);
                temp[dp->name_len] = 0;
                printf("%8d%8d%8u %s\n", dp->inode, dp->rec_len, dp->name_len, temp); // print the name of the files
                if (strcmp(temp, ".") && strcmp(temp, ".."))                          // if neither match, there's another file
                {
                    return 0; // fail
                }
                cp += dp->rec_len; // go to next entry in block
                dp = (DIR *)cp;
            }
        }
    }
    return 1; // is empty
}