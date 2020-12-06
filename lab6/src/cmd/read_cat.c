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

int read_file() {
    int fd = 0, n_bytes = 0;
    printf("Enter a file descriptor: ");
    scanf("%d", &fd);
    printf("Enter the number of bytes to read: ");
    scanf("%d", &n_bytes);

    if (is_invalid_fd(fd)) {
        printf("error: invalid provided fd\n");
        return -1;
    }

    // verify fd is open for RD or RW
    if (running->fd[fd]->mode != READ && running->fd[fd]->mode != READ_WRITE) {
        printf("error: provided fd is not open for read or write\n");
        return -1;
    }

    char buf[BLKSIZE];
    return myread(fd, buf, n_bytes);
}

int myread(int fd, char *buf, int n_bytes) {
    OFT *oftp = running->fd[fd];
    MINODE *mip = oftp->mptr;
    int count = 0, lbk, startByte, blk;
    int ibuf[BLKSIZE] = { 0 };
    int doubly_ibuf[BLKSIZE] = { 0 };

    int avil = mip->INODE.i_size - oftp->offset;
    char *cq = buf;

    if (n_bytes > avil)
        n_bytes = avil;

    while (n_bytes && avil) {
        lbk = oftp->offset / BLKSIZE;
        startByte = oftp->offset % BLKSIZE;

        if (lbk < 12) {                             // direct blocks
            blk = mip->INODE.i_block[lbk];
        } else if (lbk >= 12 && lbk < 256 + 12) {   // indirect blocks
            // read block from i_block[12], pp. 348
            get_block(mip->dev, mip->INODE.i_block[12], ibuf);
            blk = ibuf[lbk - 12]; // offset of 12
        } else {                                    // doubly indirect blocks
            // read block from i_block[13]
            get_block(mip->dev, mip->INODE.i_block[13], (char *)ibuf);
            // use mailman's algorithm to reset blk to the correct doubly indirect block
            int chunk_size = (BLKSIZE / sizeof(int));
            lbk = lbk - chunk_size - 12; // reset lbk to 0 relatively
            blk = ibuf[lbk / chunk_size]; // divide 'addresses'/indices by 256
            get_block(mip->dev, blk, doubly_ibuf);
            // now modulus to get the correct mapping
            blk = doubly_ibuf[lbk % chunk_size];
        }

        char readbuf[BLKSIZE];
        // get data block into readbuf[BLKSIZE]
        get_block(mip->dev, blk, readbuf);

        // copy from startByte to buf[]
        char *cp = readbuf + startByte;
        int remainder = BLKSIZE - startByte;

        // copy entire BLKSIZE at a time
        if (n_bytes <= remainder) {
            memcpy(cq, cp, n_bytes);
            cq += n_bytes;
            cp += n_bytes;
            count += n_bytes;
            oftp->offset += n_bytes;
            avil -= n_bytes; 
            remainder -= n_bytes;
            n_bytes = 0;
        } else {
            memcpy(cq, cp, remainder);
            cq += remainder;
            cp += remainder;
            count += remainder;
            oftp->offset += remainder;
            avil -= remainder;
            n_bytes -= remainder; 
            remainder = 0;
        }
    }
    // printf("myread: read %d char from file descriptor %d, offset: %x\n", count, fd, oftp->offset); // FOR DEBUG
    return count;
}

int my_cat(char *filename) {
    printf("CAT: running->cwd->ino, address: %d\t%x\n", running->cwd->ino, running->cwd);
    int n;
    char mybuf[BLKSIZE];
    int fd = open_file(filename, READ);
    if (is_invalid_fd(fd)) {
        printf("error, invalid fd to cat\n");
        close_file(fd);
        return -1;
    }
    while (n = myread(fd, mybuf, BLKSIZE)) {
        mybuf[n] = 0;
        char *cp = mybuf;
        while (*cp != '\0') {
            if (*cp == '\n') {
                printf("\n");
            } else {
                printf("%c", *cp);
            }
            cp++;
        }
        //printf("%s", mybuf); // to be fixed
    }
    //putchar('\n');
    close_file(fd);

    printf("END OF CAT: running->cwd->ino, address: %d\t%x\n", running->cwd->ino, running->cwd);
    return 0;
}