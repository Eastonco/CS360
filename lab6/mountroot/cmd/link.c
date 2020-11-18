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

int link_wrapper(char *pathname) {
    // tokenize pathname into 'old' and 'new' delimited by a space
    char dup[256];
    strcpy(dup, pathname);
    char old[256], new[256];
    old = strtok(dup, " ");
    new = strtok(NULL, " ");
    printf("old = %s\nnew = %s\n", old, new);
    //link(old, new);
}

int my_link(char *oldname, char *newname)
{
    int inode_old, inode_new;
    MINODE *mip, mip_new;

    // set dev correctly for getting old inode
    if (oldname[0] == '/')
    {
        dev = root->dev;
    }
    else
    {
        dev = running->cwd->dev;
    }

    inode_old = getino(oldname);
    if (!inode_old) {
        // file doesn't exist
    }

    mip = iget(dev, inode_old);

    if (S_ISDIR(mip->INODE.i_mode)) {
        printf("is directory, not allowed\n");
        return -1;
    }

    // set dev correctly for getting new inode
    if (newname[0] == '/')
    {
        dev = root->dev;
    }
    else
    {
        dev = running->cwd->dev;
    }

    // look for that directory newname exists but the file does not exist yet in the directory
    char parent[256], child[256];
    strcpy(child, basename(newfile));
    strcpy(parent, dirname(newfile));

    inode_new = getino(parent);
    if (inode_new) {
        // if inode_new exists already, then there's already a link
        return -1;
    }

    mip_new = iget(dev, inode_new);

    enter_name(mip_new, mip->ino, child);

    mip->INODE.i_links_count++;
    mip->dirty = 1;

    iput(mip);
    iput(mip_new);
}