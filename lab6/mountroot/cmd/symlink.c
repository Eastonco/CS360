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

// TODO: finish this sh
int my_symlink(char *old, char *new) {
    MINODE *mip;

    // set dev correctly for getting old inode
    if (old[0] == '/')
    {
        dev = root->dev;
    }
    else
    {
        dev = running->cwd->dev;
    }

    // verify old exists (either dir or regular)
    int old_ino = getino(old);
    if (!old_ino) {
        printf("%s does not exist\n", old);
        return -1;
    }

    // set dev correctly for getting new inode
    if (new[0] == '/')
    {
        dev = root->dev;
    }
    else
    {
        dev = running->cwd->dev;
    }

    // creat file x/y/z/ (new)
    creat_file(new);

    // set type of new to LNK (0xA000)
    int new_ino = getino(new);
    if (!new_ino) {
        printf("%s does not exist\n", new);
        return -1;
    }
    mip = iget(dev, new_ino);
    mip->i_mode = 0xA1FF; // A1FF sets link perm bits correctly (rwx for all users)

    // TODO: everything past here
    // write the string old into the i_block[ ], which has room for 60 chars.
    // i_block[] + 24 after = 84 total for old


    // set /x/y/z file size = number of chars in oldName

    // write the INODE of /x/y/z back to disk.

}