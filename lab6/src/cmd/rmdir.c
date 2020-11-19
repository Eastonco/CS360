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

int myrmdir(char *pathname)
{
    int ino = getino(pathname);

    if (!ino)
    {
        printf("ERROR: ino does not exist\n");
        return -1;
    }

    MINODE *mip = iget(dev, ino);

    if (!S_ISDIR(mip->INODE.i_mode))
    {
        printf("ERROR: node is not a directory\n");
        return -1;
    }

    if (!is_empty(mip))
    {
        printf("ERROR: Dir is not empty\n");
        return -1;
    }

    if (mip->refCount > 2) // refcount is incrementing when we cd into it but not decrementing when cding out of it
    {
        printf("ERROR: node is busy, refcount > 2, refcount = %d\n", mip->refCount);
        return -1;
    }



    for (int i = 0; i < 12; i++)
    {
        if (mip->INODE.i_block[i] == 0)
            continue;
        bdealloc(mip->dev, mip->INODE.i_block[i]); 
    }
    idealloc(mip->dev, mip->ino); 

    iput(mip); //which clears mip->refCount = 0

    char cp1[256], cp2[256], *parent, *child;
    strcpy(cp1, pathname);
    strcpy(cp2, pathname);

    parent = dirname(cp1);
    child = basename(cp2);

    int pino = getino(parent);

    MINODE *pip = iget(mip->dev, pino);
    rm_child(pip, child);

    pip->INODE.i_links_count--;
    pip->INODE.i_atime = time(0L);
    pip->INODE.i_ctime = time(0L);
    pip->dirty = 1;

    iput(pip);

    return 0;
}

int is_empty(MINODE *mip)
{
    char buf[BLKSIZE], *cp, temp[256];
    DIR *dp;
    INODE *ip = &mip->INODE;

    if (ip->i_links_count > 2)
    { //links count only looks at dirs, still could have files
        return 0;
    }
    else if (ip->i_links_count == 2)
    {
        for (int i = 0; i < 12; i++)
        { //Search DIR direct blocks only
            if (ip->i_block[i] == 0)
                break;
            get_block(mip->dev, mip->INODE.i_block[i], buf);
            dp = (DIR *)buf;
            cp = buf;

            while (cp < buf + BLKSIZE)
            {
                strncpy(temp, dp->name, dp->name_len);
                temp[dp->name_len] = 0;
                printf("%8d%8d%8u %s\n",
                       dp->inode, dp->rec_len, dp->name_len, temp);
                if (strcmp(temp, ".") && strcmp(temp, "..")) // if neither match, there's another file
                {
                    return 0;
                }
                cp += dp->rec_len;
                dp = (DIR *)cp;
            }
        }
    }
    return 1;
}