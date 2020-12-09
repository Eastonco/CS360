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
    OFT *oftp = running->fd[fd];
    MINODE *mip = oftp->mptr;
    INODE *ip = &mip->INODE;
    int count = 0, lblk, startByte, blk, remainder, doubleblk;
    char ibuf[BLKSIZE] = { 0 }, doubly_ibuf[BLKSIZE] = { 0 };

    char *cq = buf;

    while (n_bytes > 0)
    {
        lblk = oftp->offset / BLKSIZE;
        startByte = oftp->offset % BLKSIZE;


        if (lblk < 12) // direct blocks
        {
            if(ip->i_block[lblk] == 0)
            {
                ip->i_block[lblk] = balloc(mip->dev);
            }
            blk = ip->i_block[lblk];
        }

        else if (lblk >= 12 && lblk < 256 + 12 )
        {
            char tbuf[BLKSIZE] = { 0 };

            if(ip->i_block[12] == 0)
            {
                int block_12 = ip->i_block[12] = balloc(mip->dev);

                if (block_12 == 0) 
                    return 0;

                get_block(mip->dev, ip->i_block[12], ibuf);
                int *ptr = (int *)ibuf;
                for(int i = 0; i < (BLKSIZE / sizeof(int)); i++)
                {
                    ptr[i] = 0; 
                }

                put_block(mip->dev, ip->i_block[12], ibuf);
                mip->INODE.i_blocks++;
            }
            int indir_buf[BLKSIZE / sizeof(int)] = { 0 };
            get_block(mip->dev, ip->i_block[12], (char *)indir_buf);
            blk = indir_buf[lblk - 12];

            if(blk == 0){
                blk = indir_buf[lblk - 12] = balloc(mip->dev);
                ip->i_blocks++;
                put_block(mip->dev, ip->i_block[12], (char *)indir_buf);

            }
        }

        else
        {
            lblk = lblk - (BLKSIZE/sizeof(int)) - 12;
            //printf("%d\n", mip->INODE.i_block[13]);
            if(mip->INODE.i_block[13] == 0)
            {
                int block_13 = ip->i_block[13] = balloc(mip->dev);

                if (block_13 == 0) 
                    return 0;

                get_block(mip->dev, ip->i_block[13], ibuf);
                int *ptr = (int *)ibuf;
                for(int i = 0; i < (BLKSIZE / sizeof(int)); i++){
                    ptr[i] = 0;
                }
                put_block(mip->dev, ip->i_block[13], ibuf);
                ip->i_blocks++;
            }
            int doublebuf[BLKSIZE/sizeof(int)] = {0};
            get_block(mip->dev, ip->i_block[13], (char *)doublebuf);
            doubleblk = doublebuf[lblk/(BLKSIZE / sizeof(int))];

            if(doubleblk == 0){
                doubleblk = doublebuf[lblk/(BLKSIZE / sizeof(int))] = balloc(mip->dev);
                if (doubleblk == 0) 
                    return 0;
                get_block(mip->dev, doubleblk, doubly_ibuf);
                int *ptr = (int *)doubly_ibuf;
                for(int i = 0; i < (BLKSIZE / sizeof(int)); i++){
                    ptr[i] = 0;
                }
                put_block(mip->dev, doubleblk, doubly_ibuf);
                ip->i_blocks++;
                put_block(mip->dev, mip->INODE.i_block[13], (char *)doublebuf);
            }

            memset(doublebuf, 0, BLKSIZE / sizeof(int));
            get_block(mip->dev, doubleblk, (char *)doublebuf);
            if (doublebuf[lblk % (BLKSIZE / sizeof(int))] == 0) {
                blk = doublebuf[lblk % (BLKSIZE / sizeof(int))] = balloc(mip->dev);
                if (blk == 0)
                    return 0;
                ip->i_blocks++;
                put_block(mip->dev, doubleblk, (char *)doublebuf);
            }
        }
        
        char writebuf[BLKSIZE] = { 0 };

        get_block(mip->dev, blk, writebuf);

        char *cp = writebuf + startByte;
        remainder = BLKSIZE - startByte;

        if(remainder <= n_bytes)
        {
            memcpy(cp, cq, remainder);
            cq += remainder;
            cp += remainder;
            oftp->offset += remainder;
            n_bytes -= remainder;
        } else {
            memcpy(cp, cq, n_bytes); 
            cq += n_bytes;
            cp += n_bytes;
            oftp->offset += n_bytes;
            n_bytes -= n_bytes;
        }
        if(oftp->offset > mip->INODE.i_size)
            mip->INODE.i_size = oftp->offset;

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
        memset(mybuf, '\0', BLKSIZE);
    }
    close_file(fdsrc);
    close_file(fddest);
    return 0;
}

