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

int link_wrapper(char *old, char *new) {
    // tokenize pathname into 'old' and 'new' delimited by a space
    printf("old = %s\nnew = %s\n", old, new);
    my_link(old, new);
}

// TODO: I don't think this does all of the checks it needs to regarding links, but basic functionality is there
// (i.e. "link file1 link1" creates a link called link1 and ups file1's # of links by 1)
int my_link(char *oldname, char *newname)
{
    int inode_old, inode_new;
    MINODE *mip, *mip_new;

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
        printf("error - file linking to doesn't exist\n");
        return -1;
    }

    mip = iget(dev, inode_old);

    if (S_ISDIR(mip->INODE.i_mode)) {
        printf("is directory, not allowed\n");
        return -1;
    }

    int olddev = dev;

    // set dev correctly for getting new inode
    if (newname[0] == '/')
    {
        dev = root->dev;
    }
    else
    {
        dev = running->cwd->dev;
    }

    if (olddev != dev) {
        printf("cannot link two files on different devices\n");
        return -1;
    }

    printf("%s NEWNAME\n", newname);
    int fileino = getino(newname);
    // ino can be -1? which shouldn't exist, might need refactor
    if (fileino > 0) {
        printf("link or file already exists, %s, ino=%d\n", newname, fileino);
        return -1;
    }

    // look for that directory newname exists but the file does not exist yet in the directory
    char parent[256], child[256];
    strcpy(child, basename(newname));
    strcpy(parent, dirname(newname));

    inode_new = getino(parent);
    if (!inode_new) {
        printf("can't create link in parent dir %s\n", parent);
        return -1;
    }

    mip_new = iget(dev, inode_new);

    enter_name(mip_new, mip->ino, child);

    mip->INODE.i_links_count++;
    mip->dirty = 1;

    iput(mip);
    iput(mip_new);
}