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

int creat_file(char *pathname) 
{
    MINODE *start;
    char pathcpy1[256], pathcpy2[256];

    strcpy(pathcpy1, pathname);
    strcpy(pathcpy2, pathname);

    if (pathname[0] == '/')
    {
        start = root;
        dev = root->dev;
    }
    else
    {
        start = running->cwd;
        dev = running->cwd->dev;
    }

    char *parent = dirname(pathcpy1); //dir and basename are distructive to strings
    char *child = basename(pathcpy2);

    int pino = getino(parent);

    if (!pino)
    { // Verrifies ino exists
        printf("ERROR: parent %s doesn't exist\n", parent);
        return -1;
    }

    MINODE *pip = iget(dev, pino);

    if (!S_ISDIR(pip->INODE.i_mode)) // checks if parent is dir
    {
        printf("ERROR: %s is not a directory\n", parent);
        return -1;
    }

    if (!search(pip, child))
    {
        printf("ERROR: child %s already exists under parent %s", child, parent);
        return -1;
    }

    my_creat(pip, child);
    pip->INODE.i_atime = time(0L); 
    pip->dirty = 1;
    iput(pip); //writes the changed MINODE to the block
    return 0;
}

int my_creat(MINODE *pip, char *name)
{
    MINODE *mip;
    char *buf[BLKSIZE], *cp;
    DIR *dp;

    int ino = ialloc(dev);
    int bno = balloc(dev);

    printf("ino: %d, bno: %d\n", ino, bno);

    mip = iget(dev, ino);

    INODE *ip = &mip->INODE;
    ip->i_mode = FILE_MODE;    // 0x81A4 OR 0100644: FILE type and permissions
    ip->i_uid = running->uid; // Owner uid
    ip->i_gid = running->gid; // Group Id
    ip->i_size = BLKSIZE;     // Size in bytes
    ip->i_links_count = 1;    // Links count=1 since it's a file
    ip->i_atime = time(0L);   // set to current time
    ip->i_ctime = time(0L);
    ip->i_mtime = time(0L);
    ip->i_blocks = 2;     // LINUX: Blocks count in 512-byte chunks
    ip->i_block[0] = 0; // new File has 0 data blocks
    for (int i = 1; i < 15; i++)
    {
        ip->i_block[i] = 0;
    }

    mip->dirty = 1; // mark minode dirty
    iput(mip);      // write INODE to disk

    enter_name(pip, ino, name);

    return 0;
}