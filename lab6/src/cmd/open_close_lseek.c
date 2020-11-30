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
    MINODE *mip = iget(dev, ino);

    if (!S_ISREG(mip->INODE.i_mode)) {
        printf("error: not a regular file\n");
        return -1;
    }

    // TODO: check file permissions are OK (uid, gid?)

    // go through all open files-- check if anything is open with incompatible mode
    for (int i = 0; i < NFD; i++) {
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
    running->fd[fd] = 0;
    oftp->refCount--;
    if (oftp->refCount > 0) // minode not ready to be disposed
        return 0;
    
    MINODE *mip = oftp->mptr;
    iput(mip);
    
    free(oftp); // no memory leaks allowed!

    return 0;
}

int is_invalid_fd(int fd) {
    return (fd < 0 || fd > (NFD-1));
}

int my_lseek(int fd, int position) {

}

int pfd() {

}

int dup(int fd) {

}

int dup2(int fd, int gd) {

}