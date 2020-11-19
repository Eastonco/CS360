/****************************************************************************
*                             ext2 file system                              *
*****************************************************************************/
#include "type.h"
#include "util.h"
#include "cmd.h"

// globals
MINODE minode[NMINODE]; // in memory INODEs
MINODE *root;           // root of the file system

PROC proc[NPROC]; // PROC structures
PROC *running;    // current executing PROC

char gpath[128]; // global for tokenized components
char *name[32];  // assume at most 32 components in pathname
int n;           // number of component strings

int fd, dev;
int nblocks, ninodes, bmap, imap, inode_start;

int init()
{
    int i, j;
    MINODE *mip;
    PROC *p;

    printf("init()\n");

    for (i = 0; i < NMINODE; i++)
    {
        mip = &minode[i];
        mip->dev = mip->ino = 0;
        mip->refCount = 0;
        mip->mounted = 0;
        mip->mptr = 0;
    }
    for (i = 0; i < NPROC; i++)
    {
        p = &proc[i];
        p->pid = i;
        p->uid = p->gid = 0;
        p->cwd = 0;
        p->status = FREE;
        for (j = 0; j < NFD; j++)
            p->fd[j] = 0;
    }

    root = NULL;
}

// load root INODE and set root pointer to it
int mount_root()
{
    printf("mount_root()\n");
    root = iget(dev, 2);
}

char *disk = "diskimage";
int main(int argc, char *argv[])
{
    int ino;
    char buf[BLKSIZE];
    char line[128], cmd[32], pathname[128], pathname_two[128];

    if (argc > 1)
        disk = argv[1];

    printf("checking EXT2 FS ....");
    if ((fd = open(disk, O_RDWR)) < 0)
    {
        printf("open %s failed\n", disk);
        exit(1);
    }
    dev = fd;

    /********** read super block  ****************/
    get_block(dev, 1, buf);
    sp = (SUPER *)buf;

    /* verify it's an ext2 file system ***********/
    if (sp->s_magic != 0xEF53)
    {
        printf("magic = %x is not an ext2 filesystem\n", sp->s_magic);
        exit(1);
    }
    printf("EXT2 FS OK\n");
    ninodes = sp->s_inodes_count;
    nblocks = sp->s_blocks_count;

    get_block(dev, 2, buf);
    gp = (GD *)buf;

    bmap = gp->bg_block_bitmap;
    imap = gp->bg_inode_bitmap;
    inode_start = gp->bg_inode_table;
    printf("bmp=%d imap=%d inode_start = %d\n", bmap, imap, inode_start);

    init();
    mount_root();
    printf("root refCount = %d\n", root->refCount);

    printf("creating P0 as running process\n");
    running = &proc[0];
    running->status = READY;
    running->cwd = iget(dev, 2);
    printf("root refCount = %d\n", root->refCount);

    while (1)
    {
        printf("input command : [ls|cd|pwd|quit|mkdir|rmdir|creat|link|unlink] ");
        fgets(line, 128, stdin);
        line[strlen(line) - 1] = 0;

        if (line[0] == 0)
            continue;
        pathname[0] = 0;

        sscanf(line, "%s %s", cmd, pathname);
        printf("cmd=%s pathname=%s\n", cmd, pathname);

        if (!strcmp(cmd, "ls"))
            my_ls(pathname);
        if (!strcmp(cmd, "cd"))
            my_chdir(pathname);
        if (!strcmp(cmd, "pwd"))
            my_pwd(running->cwd);
        if (!strcmp(cmd, "quit"))
            quit();
        if (!strcmp(cmd, "mkdir"))
            make_dir(pathname);
        if (!strcmp(cmd, "link")) {
            sscanf(line, "%s %s %s", cmd, pathname, pathname_two);
            link_wrapper(pathname, pathname_two);
        }
        if (!strcmp(cmd, "unlink"))
            my_unlink(pathname);
        if (!strcmp(cmd, "creat"))
            creat_file(pathname);
        if (!strcmp(cmd, "rmdir"))  
            myrmdir(pathname);
    }
}

int quit()
{
    int i;
    MINODE *mip;
    for (i = 0; i < NMINODE; i++)
    {
        mip = &minode[i];
        if (mip->refCount > 0)
            iput(mip);
    }
    exit(0);
}
