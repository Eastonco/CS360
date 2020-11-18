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

// todo: probably more checks to do
int unlink(char *pathname) {
    int inode;
    MINODE *mip;

    if (pathname[0] == '/')
    {
        dev = root->dev;
    }
    else
    {
        dev = running->cwd->dev;
    }

    inode = getino(pathname);
    mip = iget(dev, inode);
    if (S_ISDIR(mip->INODE.i_mode))) {
        printf("dir cannot be link; cannot unlink %s\n", pathname);
        return -1;
    }

    mip->INODE.i_links_count--;
    if (mip->INODE.i_links_count == 0) {
        // deallocate data blocks with truncate() function
        truncate(mip);
    }

    char child[256];
    strcpy(child, basename(pathname));
    // now remove child - same function as rm (to be implemented)
    // rm_child(mip, child);

    mip->dirty = 1;
    iput(mip);
}

// use inodes in block, go to address, free them (memset), 
int truncate(MINODE *mip) {
    char buf[BLKSIZE];
    INODE *ip = &mip->INODE;
    // 12 direct blocks
    for (int i = 0; i < 12; i++) {
        if (ip->iblock[i] == 0)
            break;
        // now deallocate block
        get_block(dev, bmap, buf);
        clr_bit(buf, (ip->i_block[i])-1);
        put_block(dev, bmap, buf);
        // increment # of free blocks
        incFreeBlocks(dev);
        ip->iblock[i] = 0;
    }
    mip->i_blocks = 0;
    mip->i_size = 0;
    mip->dirty = 1;
    iput(mip);
}