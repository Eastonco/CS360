//#include "../type.h"
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

int write_file() 
{
    int fd = 0, n_bytes = 0;
    char wbuf[BLKSIZE] = {0};
    printf("Enter a file descriptor: ");
    scanf("%d", &fd);
    printf("Enter what you want to write: ");
    scanf("%s", &wbuf);

    if (is_invalid_fd(fd)) {
        printf("error: invalid provided fd\n");
        return -1;
    }

    // verify fd is open for RD or RW
    if (running->fd[fd]->mode != WRITE && running->fd[fd]->mode != READ_WRITE) {
        printf("error: provided fd is not open for write\n");
        return -1;
    }

    n_bytes = sizeof(wbuf);
    return mywrite(fd, wbuf, n_bytes);
}

int mywrite(int fd, char *buf, int n_bytes)
{
    printf("fd = %d\n", fd);
    OFT *oftp = running->fd[fd];
    MINODE *mip = oftp->mptr;
    INODE *ip = &mip->INODE;
    int count = 0, lblk, startByte, blk;
    int ibuf[BLKSIZE] = { 0 };
    int doubly_ibuf[BLKSIZE] = { 0 };

    char *cq = buf;

    printf("write!\n");

    while (n_bytes)
    {
        printf("loop\n");
        lblk = oftp->offset / BLKSIZE;
        startByte = oftp->offset % BLKSIZE;

        printf("lblk: %d\n", lblk);

        if (lblk < 12) // direct blocks
        {
            printf("direct blocks\n");
            if(ip->i_block[lblk] == 0)
            {
                ip->i_block[lblk] = balloc(mip->dev);
            }
            blk = ip->i_block[lblk];
            printf("survived 1\n");
        }

        else if (lblk >= 12 && lblk < 256 + 12 )
        {
            printf("indirect blocks\n");
            int tbuf[BLKSIZE];

            if(ip->i_block[12] == 0)
            {
                ip->i_block[12] = balloc(mip->dev);

                get_block(mip->dev, ip->i_block[12], tbuf);

                for(int i = 0; i < BLKSIZE; i++)
                {
                    tbuf[i] = 0; 
                }

                put_block(mip->dev, ip->i_block[12], tbuf);
                mip->INODE.i_blocks++;
            }
            get_block(mip->dev, ip->i_block[12], (char *)ibuf);
            blk = ibuf[lblk - 12];

            if(blk == 0){
                blk = balloc(mip->dev);
                ibuf[lblk - 12] = blk;
                put_block(mip->dev, ip->i_block[12], (char *)ibuf);

            }
            printf("survived 2\n");
        }

        else
        {
            printf("doubly indirect blocks\n");
            lblk = lblk - ((BLKSIZE/sizeof(int)) - 12);
            printf("%d\n", mip->INODE.i_block[13]);
            if(mip->INODE.i_block[13] == 0)
            {
                printf("allocate new block\n");
                ip->i_block[13] = balloc(mip->dev);
                get_block(mip->dev, ip->i_block[13], (char *)ibuf);

                for(int i = 0; i < 256; i++){
                    ibuf[i] = 0;
                }
                put_block(mip->dev, ip->i_block[13], ibuf);
                ip->i_blocks++;
            }
            printf("get block from the 13th place\n");
            get_block(mip->dev, ip->i_block[13], (char *)doubly_ibuf);
            int doubleblk = doubly_ibuf[lblk/256];

            if(doubleblk == 0){
                doubly_ibuf[lblk/256] = balloc(mip->dev);
                doubleblk = doubly_ibuf[lblk/256];
                get_block(mip->dev, doubleblk, (char *)doubly_ibuf);
                for(int i = 0; i < 256; i++){
                    doubly_ibuf[i] = 0;
                }
                put_block(mip->dev, ip->i_block[13], (char *)doubly_ibuf);
                ip->i_blocks++;
                put_block(mip->dev, mip->INODE.i_block[13], (char *)doubly_ibuf);
            }

            memset(doubly_ibuf, 0, 256);
            get_block(mip->dev, doubleblk, (char *)doubly_ibuf);

            if (doubly_ibuf[lblk % 256] == 0) {
                blk = doubly_ibuf[lblk % 256] = balloc(mip->dev);
                if (blk == 0)
                    return 0;
                ip->i_blocks++;
                put_block(mip->dev, doubleblk, (char *)doubly_ibuf);
            }

            /*// use mailman's algorithm to reset blk to the correct doubly indirect block
            int chunk_size = BLKSIZE / sizeof(int);
            lblk -= chunk_size - 12; // reset lbk to 0 relatively
            blk = ibuf[lblk / chunk_size]; // divide 'addresses'/indices by 256
            get_block(mip->dev, blk, doubly_ibuf);
            // now modulus to get the correct mapping
            blk = doubly_ibuf[lblk % chunk_size];
            //fixme*/
        }
        
        char writebuf[BLKSIZE];

        get_block(mip->dev, blk, writebuf);

        char *cp = writebuf + startByte;
        int remainder = BLKSIZE - startByte;

        if(remainder > n_bytes)
        {
            memcpy(cp, cq, n_bytes); 
            cq += n_bytes;
            cp += n_bytes;
            oftp->offset += n_bytes;
            remainder -= n_bytes;
            n_bytes -= n_bytes;
        } else {
            memcpy(cp, cq, remainder);
            cq += remainder;
            cp += remainder;
            oftp->offset += remainder;
            n_bytes -= remainder; 
        }

        put_block(mip->dev, blk, writebuf);
    }

    mip->dirty = 1;
    return n_bytes;
}

int my_cp(char *src, char *dest){
    int n = 0;
    char mybuf[BLKSIZE] = {0};
    int fdsrc = open_file(src, READ);
    int fddest = open_file(dest, READ_WRITE);

    printf("fdsrc %d\n", fdsrc);
    printf("fddest %d\n", fddest);

    if (fdsrc == -1 || fddest == -1) {
		if (fddest == -1) close_file(fddest);
		if (fdsrc == -1) close_file(fdsrc);
		return -1;
	}

    memset(mybuf, '\0', BLKSIZE);
    while ( n = myread(fdsrc, mybuf, BLKSIZE)){
        mybuf[n] = 0;
        mywrite(fddest, mybuf, n);
        printf("debug zach\n");
        memset(mybuf, '\0', BLKSIZE);
    }
    close_file(fdsrc);
    close_file(fddest);
    return 0;
}

