#include "../cmd.h"

// globals
extern MINODE minode[NMINODE];
extern MINODE *root;

extern PROC proc[NPROC], *running;
extern MTABLE mount_table[NMOUNT];

extern char gpath[128]; // global for tokenized components
extern char *name[32];  // assume at most 32 components in pathname
extern int n;           // number of component strings in name[]

extern int fd, dev;
extern int nblocks, ninodes, bmap, imap, inode_start;

// mount (no parameters)
int list_mount(void) {
    // print currently mounted file systems
    printf("Currently mounted filesystems:\n");
    printf("dev\tname\tmount dir\n");
    for (int i = 0; i < NMOUNT; i++) {
        if (mount_table[i].dev != 0) {
            printf("%d\t%s\t%s\n", mount_table[i].dev, mount_table[i].devName, mount_table[i].mntName);
        }
    }
    return 0;
}

// mount filesys mount_point
// might want to make mount table an array of pointers to structs? that might be more elegant
int my_mount(char *filesys, char *mount_dest) {
    int ino, mount_index = -1;
    MINODE *mip;
    MTABLE *mtptr;
    // Check whether filesystem is already mounted
    // if so, reject
    for (int i = 0; i < NMOUNT; i++) {
        if (mount_table[i].dev != 0) {
            if (!strcmp(filesys, mount_table[i].devName)) {
                printf("mount rejected, %s is already mounted\n", filesys);
                return -1;
            }
        }
    }

    // find first free mnt_table index
    for (int i = 0; i < NMOUNT; i++) {
        if (mount_table[i].dev == 0) {
            mount_index = i;
            break;
        }
    }

    if (mount_index == -1) {
        printf("PANIC: device mount limit reached\n");
        return -1;
    }

    // if not, allocate free mount table entry (dev=0 is FREE)
    mtptr = &mount_table[mount_index]; // no malloc --> mtptr = (MTABLE *)malloc(sizeof(MTABLE));
    mtptr->dev = 0;
    strcpy(mtptr->devName, filesys);
    strcpy(mtptr->mntName, mount_dest);
    // TODO: set mount table entry to mtptr (not currently array of ptrs)
    // mount_table[mount_index] = mtptr;

    // open filesystem for RW, use FD # as new dev
    // check whether ext2 filesystem or not; if not reject
    //  |
    //  |-> read superblock, check if s_magic is 0xEF53
    int fd = open_file(mount_dest, READ_WRITE);
    if (is_invalid_fd(fd)) {
        printf("invalid fd for filesys\n");
        return -1;
    }

    char buf[BLKSIZE];
    get_block(fd, 1, buf);
    SUPER *sp = (SUPER *)buf;

    if (sp->s_magic != SUPER_MAGIC) {
        printf("error, magic = %x is not an ext2 filesystem\n", sp->s_magic);
        return -1;
    }

    // mount_point, get ino and then minode
    ino = getino(mount_dest);
    mip = iget(running->cwd->dev, ino);

    // check m_p is a dir and is not busy (not someone else's CWD)
    if (!S_ISDIR(mip->INODE.i_mode)) {
        printf("error, %s is not a directory, cannot be mounted\n", mount_dest);
        return -1;
    }

    if (mip->refCount > 0) {
        printf("cannot mount: directory is busy (refcount > 0)\n");
        return -1;
    }

    // record new dev in mount table entry (fd is new DEV)
    mtptr->dev = fd;
    // for convenience, mark other information as well (TODO)
    mtptr->ninodes = sp->s_inodes_count;
    mtptr->nblocks = sp->s_blocks_count;

    // mark mount_point's minode as being mounted on and let it point at the MOUNT table entry, which points back to the
    // m_p minode
    mip->mounted = 1;
    mip->mptr = mtptr;
    mtptr->mntDirPtr = mip;

    return 0;
}

int my_umount(char *filesys) {
    // 1. Search the MOUNT table to check filesys is indeed mounted.

    // 2. Check whether any file is still active in the mounted filesys;
    // HOW to check?      ANS: by checking all minode[].dev

    // 3. Find the mount_point's inode (which should be in memory while it's mounted on).  Reset it to "not mounted"; then 
    // iput()   the minode.  (because it was iget()ed during mounting)


    return 0;
}