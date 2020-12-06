#include "../cmd.h"

// globals
extern MINODE minode[NMINODE];
extern MINODE *root;

extern PROC proc[NPROC], *running;

extern char gpath[128]; // global for tokenized components
extern char *name[32];  // assume at most 32 components in pathname
extern int n;           // number of component strings

extern int fd, dev;
extern int nblocks, ninodes, bmap, imap, inode_start;

int open_file(char *pathname, int mode) {
    // mode, 0 = read, 1 = write, 2 = read/write, 3 = append
    if (mode_is_invalid(mode)) {
        printf("error: invalid mode\n");
        return -1;
    }
        
    
    if (pathname[0] == '/')
    {
        dev = root->dev;
    }
    else
    {
        dev = running->cwd->dev;
    }
    
    int ino = getino(pathname);

    if (ino == -1) {
        // ino must be created, does not exist

        // find parent ino of file to be created
        char parent[BLKSIZE], buf[BLKSIZE];
        strcpy(buf, pathname);
        strcpy(parent, dirname(buf));
        printf("PARENT: %s\n", parent);
        int pino = getino(parent);
        if (pino == -1) {
            printf("error finding parent inode (open file)\n");
            return -1;
        }
        MINODE *pmip = iget(dev, pino);

        int r = my_creat(pmip, pathname);
        ino = getino(pathname);

        if (ino == -1) {
            // if ino still failed, we probably have bigger problems
            printf("error: new ino allocation failed for open\n");
            return -1;
        }
    }
    printf("MIDDLE OPEN: running->cwd->ino, address: %d\t%x\n", running->cwd->ino, running->cwd);

    MINODE *mip = iget(dev, ino);

    printf("debug mode: %d\n", mip->ino);

    if (!S_ISREG(mip->INODE.i_mode)) {
        printf("error: not a regular file\n");
        return -1;
    }

    if (!(mip->INODE.i_uid == running->uid || running->uid)) {
        printf("permissions error: uid\n");
        return -1;
    }

    if (!(mip->INODE.i_gid == running->gid || running->gid)) {
        printf("permissions error: gid\n");
        return -1;
    }

    // go through all open files-- check if anything is open with incompatible mode
    for (int i = 0; i < NFD; i++) {
        if (running->fd[i] == NULL)
            break;
        if (running->fd[i]->mptr == mip) {
            if (mode != 0) {
                printf("error: already open with incompatible mode\n");
                return -1;
            }
        }
    }

    OFT *oftp = (OFT *)malloc(sizeof(OFT));
    oftp->mode = mode;
    oftp->refCount = 1;
    oftp->mptr = mip;

    switch(mode) {
        case 0:                 // read, offset = 0
            oftp->offset = 0;
            break;
        case 1:                 // write, truncate file to 0 size
            inode_truncate(mip);
            oftp->offset = 0;
            break;
        case 2:                 // read/write, don't truncate file
            oftp->offset = 0;
            break;
        case 3:                 // append
            oftp->offset = mip->INODE.i_size;
            break;
        default:                // shouldn't ever get here based on first check, but just in case
            printf("error: invalid mode\n");
            return -1;
    }

    int returned_fd = -1;
    // might be redundant-- same loop earlier, could be refactored to find NULL fd[i] earlier
    for (int i = 0; i < NFD; i++) {
        if (running->fd[i] == NULL) {
            running->fd[i] = oftp;
            returned_fd = i;
            break;
        }
    }

    if (mode != 0) { // not read, mtime
        mip->INODE.i_mtime = time(NULL);
    }
    mip->INODE.i_atime = time(NULL);
    mip->dirty = 1;
    iput(mip);

    return returned_fd;
}

int mode_is_invalid(int mode) {
    return !(mode == 0 || mode == 1 || mode == 2 || mode == 3);
}

int close_file(int fd) {
    /*if (is_invalid_fd(fd)) {
        printf("error: fd not in range\n");
        return -1;
    }*/

    // check if pointing at OFT entry
    if (running->fd[fd] == NULL) {
        printf("error: not OFT entry\n");
        return -1;
    }

    OFT *oftp = running->fd[fd];
    running->fd[fd] = 0;
    oftp->refCount--;
    if (oftp->refCount > 0) // minode not ready to be disposed
        return 0;
    
    MINODE *mip = oftp->mptr;
    mip->dirty = 1;
    iput(mip);
    
    free(oftp); // no memory leaks allowed!

    return 0;
}

int is_invalid_fd(int fd) {
    return (fd < 0 || fd > (NFD-1));
}

int my_lseek(int fd, int position) {
    if (is_invalid_fd(fd)) {
        printf("error: fd not in range\n");
        return -1;
    }

    // check if pointing at OFT entry
    if (running->fd[fd] == NULL) {
        printf("error: not OFT entry\n");
        return -1;
    }

    OFT *oftp = running->fd[fd];
    if (position > oftp->mptr->INODE.i_size || position < 0) {
        printf("error: file size overrun\n");
    }

    int original_offset = oftp->offset;
    oftp->offset = position;

    return original_offset;
}

int pfd(void) {
    printf("fd\tmode\toffset\tINODE [dev, ino]\n");
    for (int i = 0; i < NFD; i++) {
        if (running->fd[i] == NULL)
            break;
        printf("%d\t%s\t%d\t[%d, %d]\n", i, running->fd[i]->mode, running->fd[i]->offset, running->fd[i]->mptr->dev, running->fd[i]->mptr->ino);
    }
}

int dup(int fd) {
    if (is_invalid_fd(fd)) {
        printf("error: fd not in range\n");
        return -1;
    }

    // check if pointing at OFT entry
    if (running->fd[fd] == NULL) {
        printf("error: not OFT entry\n");
        return -1;
    }

    OFT *oftp = running->fd[fd];
    for (int i = 0; i < NFD; i++) {
        if (running->fd[i] == NULL) {
            running->fd[i] = oftp;
            oftp->refCount++;
            return 0;
        }
    }

    printf("error: no free fd to dupe into (allocated FDs = NFD)\n");
    return -1;
}

int dup2(int fd, int gd) {
    if (is_invalid_fd(fd)) {
        printf("error: fd not in range\n");
        return -1;
    }

    if (is_invalid_fd(gd)) {
        printf("error: gd not in range\n");
        return -1;
    }

    // check if gd is already opened
    if (running->fd[gd] != NULL) {
        // close if so
        int r = close_file(gd);
        if (r == -1) {
            printf("error closing file gd\n");
            return -1;
        }
    }
    // dupe fd[fd] into fd[gd]
    OFT *oftp = running->fd[fd];
    running->fd[gd] = oftp;
    oftp->refCount++;
    return 0;
}