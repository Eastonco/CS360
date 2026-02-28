/****************************************************************************
 *                             ext2 file system                              *
 *****************************************************************************/
#include "type.h"
#include "util.h"
#include "cmd.h"

/*
 * Global definitions: these are declared extern in type.h and util.h,
 * and defined exactly once here in main.c.
 *
 * BUG FIX: type.h originally defined sp/gp/ip/dp directly (not extern),
 * causing multiple-definition errors without -fcommon. Now they are extern
 * in the header and defined here.
 *
 * BUG FIX: name[] was char *name[32] here but declared extern char *name[64]
 * in util.c. Fixed to 64 everywhere for consistency.
 */

/* In-memory inode cache and root pointer */
MINODE minode[NMINODE];
MINODE *root;

/* Process table */
PROC proc[NPROC];
PROC *running;

/* Mount table */
MTABLE mount_table[NMOUNT];

/* Path tokenization globals */
char  gpath[128]; /* storage for strtok to carve up pathnames */
char *name[64];   /* BUG FIX: was [32]; util.c declared extern name[64] */
int   n;          /* number of path components after tokenize() */

/* On-disk structure pointers (defined here, extern in type.h) */
SUPER *sp;
GD    *gp;
INODE *ip;
DIR   *dp;

int fd, dev, root_dev;
int nblocks, ninodes, bmap, imap, inode_start;

/*
 * init: zero-initialize all in-memory filesystem data structures.
 *
 * Called once at startup before mounting. Sets all minode refCounts to 0
 * (marking them free), clears all PROC fd tables, and zeroes mount table devs.
 */
int init()
{
    int i, j;
    MINODE *mip;
    PROC *p;
    MTABLE *mtptr;

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
        p->uid = p->gid = i;
        p->cwd = 0;
        p->status = FREE;
        for (j = 0; j < NFD; j++)
            p->fd[j] = 0;
    }
    for (i = 0; i < NMOUNT; i++)
    {
        mtptr = &mount_table[i];
        mtptr->dev = 0;
    }

    /* Circular process list for round-robin scheduling simulation */
    proc[0].next = &proc[1];
    proc[1].next = &proc[0];
    root = NULL;
    return 0;
}

/*
 * mount_root: load the root inode (inode 2) into the minode cache.
 *
 * In ext2, inode 2 is always the root directory. We iget() it so it
 * stays pinned in the cache (refCount > 0) for the lifetime of the process.
 * All absolute path traversals start here.
 */
int mount_root(void)
{
    printf("mount_root()\n");
    root = iget(dev, 2);
    return 0;
}

char *disk = "disk2";

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
    dev = root_dev = fd;

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
    // set all entries in running's fd table to null
    for (int i = 0; i < NFD; i++)
        running->fd[i] = NULL;
    printf("root refCount = %d\n", root->refCount);

    while (1)
    {
        printf("running->cwd->ino, address: %d\t%x\n", running->cwd->ino, running->cwd);
        memset(cmd, 0, sizeof(cmd));
        memset(pathname, 0, sizeof(pathname));
        memset(pathname_two, 0, sizeof(pathname_two));

        printf("input command : [ls|cd|pwd|quit|mkdir|rmdir|creat|link|unlink|symlink]\n\t\t[chmod|cat|cp|open|read|write|close|pfd|mount|umount|switch] ");
        fgets(line, 128, stdin);
        line[strlen(line) - 1] = 0;

        if (line[0] == 0)
            continue;
        pathname[0] = 0;

        sscanf(line, "%s %s", cmd, pathname);
        printf("cmd=%s pathname=%s\n", cmd, pathname);

        /*
         * BUG FIX: was a chain of independent `if` statements — every branch
         * was evaluated even after a match was found. Changed to `else if` so
         * we stop checking once the first match is found. This is both more
         * correct (avoids accidentally triggering multiple handlers if cmd
         * somehow matched multiple strings) and more efficient.
         */
        if (!strcmp(cmd, "ls"))
            my_ls(pathname);
        else if (!strcmp(cmd, "cd"))
            my_chdir(pathname);
        else if (!strcmp(cmd, "pwd"))
            my_pwd(running->cwd);
        else if (!strcmp(cmd, "quit"))
            quit();
        else if (!strcmp(cmd, "mkdir"))
            make_dir(pathname);
        else if (!strcmp(cmd, "link"))
        {
            sscanf(line, "%s %s %s", cmd, pathname, pathname_two);
            link_wrapper(pathname, pathname_two);
        }
        else if (!strcmp(cmd, "unlink"))
            my_unlink(pathname);
        else if (!strcmp(cmd, "symlink"))
        {
            sscanf(line, "%s %s %s", cmd, pathname, pathname_two);
            my_symlink(pathname, pathname_two);
        }
        else if (!strcmp(cmd, "creat"))
            creat_file(pathname);
        else if (!strcmp(cmd, "rmdir"))
            myrmdir(pathname);
        else if (!strcmp(cmd, "chmod"))
            mychmod(pathname);
        else if (!strcmp(cmd, "cat"))
            my_cat(pathname);
        else if (!strcmp(cmd, "open"))
        {
            int mode = -1;
            sscanf(line, "%s %s %d", cmd, pathname, &mode);
            open_file(pathname, mode);
        }
        else if (!strcmp(cmd, "read"))
        {
            int fd = -1;
            sscanf(line, "%s %d %s", cmd, &fd, pathname);
            myread(fd, pathname, sizeof(pathname));
        }
        else if (!strcmp(cmd, "write"))
        {
            int fd = -1;
            sscanf(line, "%s %d %s", cmd, &fd, pathname);
            mywrite(fd, pathname, sizeof(pathname));
        }
        else if (!strcmp(cmd, "close"))
        {
            int fd = -1;
            sscanf(line, "%s %d", cmd, &fd);
            close_file(fd);
        }
        else if (!strcmp(cmd, "cp"))
        {
            sscanf(line, "%s %s %s", cmd, pathname, pathname_two);
            my_cp(pathname, pathname_two);
        }
        else if (!strcmp(cmd, "mount"))
        {
            sscanf(line, "%s %s %s", cmd, pathname, pathname_two);
            if (!strcmp(pathname, "") || !strcmp(pathname_two, ""))
                list_mount();
            else
                my_mount(pathname, pathname_two);
        }
        else if (!strcmp(cmd, "umount"))
            my_umount(pathname);
        else if (!strcmp(cmd, "pfd"))
            pfd();
        else if (!strcmp(cmd, "switch"))
            my_switch();
        else
            printf("unknown command: %s\n", cmd);
    }
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
