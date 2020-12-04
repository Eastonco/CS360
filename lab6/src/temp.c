int mywrite(int fd, char buf[], int nbytes) {
	printf("sw_kl_write: ECHO=%s\n", buf);
	int logical_block, startByte, blk, remain, block12arr, block13arr, doubleblk;
	char ibuf[BLKSIZE] = {0}, doubleibuf[BLKSIZE] = {0}, writebuf[BLKSIZE] = {0};
	OFT *oftp = running->fd[fd];
	MINODE *mip = oftp->mptr;

	// use cq to iterate over buf
	char *cq = buf;

	while (nbytes > 0 ) {
		// compute LOGICAL BLOCK (lbk) and the startByte in that lbk:
		// this part is necessary because we might be appending...
		// 0-11 for direct, 12-(256+12) for indirect, 268-(256^2 + 12) for double indirect
		logical_block = oftp->offset / BLKSIZE;  
		startByte = oftp->offset % BLKSIZE;  // where we start writing in the block
		
		/* This part of the code gets the correct block number to write to AND it allocates
		any new disc blocks if needed.
		*/
		if (logical_block < 12){                         // direct block
			// "If a direct block does not exist, it must be allocated and recorded in the INODE" -- KC Wang
			if (mip->INODE.i_block[logical_block] == 0) {   // if no data block yet
				mip->INODE.i_block[logical_block] = balloc(mip->dev);// MUST ALLOCATE a block
			}
			blk = mip->INODE.i_block[logical_block];      // blk should be a disk block now
		}
		else if (logical_block >= 12 && logical_block < 256 + 12){ // INDIRECT blocks 
			// First check if we even have the ptr array to keep track of indirect blocks; if not, allocate block
			// "If the indirect block i_block[12] does not exist, it must be allocated and initialized to 0." -- KC Wang
			if (mip->INODE.i_block[12] == 0) {  // FIRST INDIRECT BLOCK ALLOCATION
				// allocate a block for it;
				block12arr = mip->INODE.i_block[12] = balloc(mip->dev);
				if (block12arr == 0) return 0; // this means there aren't any more dblocks :'(
				// zero out the block on disk !!!!
				get_block(mip->dev, mip->INODE.i_block[12], ibuf);
				int *ip = (int*)ibuf, p=0;  // step thru block in chunks of sizeof(int), set each ptr to 0
				for (p=0; p<(BLKSIZE/sizeof(int));p++) ip[p] = 0;
				put_block(mip->dev, mip->INODE.i_block[12], ibuf); // write back to disc
				// increment iblock count
				mip->INODE.i_blocks++;
			}
			// get i_block[12] into an int int_buf[256];
			int int_buf[BLKSIZE/sizeof(int)] = {0};
			get_block(mip->dev, mip->INODE.i_block[12], (char*)int_buf);
			blk = int_buf[logical_block - 12];
			// "If an indirect data block does not exist, it must be allocated and recorded in the indirect block." -- KC Wang
			if (blk==0) {
				// allocate a disk block;
				blk = int_buf[logical_block - 12] = balloc(mip->dev);
				// increment iblock count
				mip->INODE.i_blocks++;
				// record it in i_block[12];
				put_block(mip->dev, mip->INODE.i_block[12], (char*)int_buf); // write back to disc
			}
		}
		else { // double indirect blocks
			/* First check if we even have the ptr array to keep track of double indirect blocks; if not, allocate block.
			i_block[13] points to an array A of size 256, each element in this array A points to another array B_i of size 256
			each B_i points to a data block!
			"if the double indirect block i_block[13] does not exist, it must be allocated and initialized to 0" -- KC Wang
			*/
			/* update logical_block for convenience. essentially subtract off all direct blocks and indirect blocks,
			   so that double indirect block 0 is technically logical block 256+12;*/
			logical_block = logical_block - (BLKSIZE/sizeof(int)) - 12;
			if (mip->INODE.i_block[13] == 0) {  // FIRST DOUBLE INDIRECT BLOCK ALLOCATION
				// allocate a block for it;
				block13arr = mip->INODE.i_block[13] = balloc(mip->dev);
				if (block13arr == 0) return 0; // no more dblocks :'(
				// zero out the block on disk !!!!
				get_block(mip->dev, mip->INODE.i_block[13], ibuf);
				int *ip = (int*)ibuf, p=0;  // step thru block in chunks of sizeof(int), set each ptr to 0
				for (p=0; p<BLKSIZE/sizeof(int);p++) ip[p] = 0;
				put_block(mip->dev, mip->INODE.i_block[13], ibuf); // write back to disc
				// increment iblock count
				mip->INODE.i_blocks++;
			}
			// get i_block[13] into an int buf[256];
			int double_int_buf[BLKSIZE/sizeof(int)] = {0};
			get_block(mip->dev, mip->INODE.i_block[13], (char*)double_int_buf);
			// get block number within first indirect array (i_block[13]); divide by 256 to get correct index
			doubleblk = double_int_buf[logical_block/(BLKSIZE/sizeof(int))];
			// if this is 0, it's an unclaimed entry in i_block[13]'s array, so balloc a data block for it
			if (doubleblk==0){
				// allocate a disk block;
				doubleblk = double_int_buf[logical_block/(BLKSIZE/sizeof(int))] = balloc(mip->dev);
				if (doubleblk == 0) return 0;
				// zero out the block on disk !!!!
				get_block(mip->dev, doubleblk, doubleibuf);
				int *ip = (int*)doubleibuf, p=0;  // step thru block in chunks of sizeof(int), set each ptr to 0
				for (p=0; p<BLKSIZE/sizeof(int);p++) ip[p] = 0;
				put_block(mip->dev, doubleblk, doubleibuf); // write back to disc
				// increment iblock count
				mip->INODE.i_blocks++;
				// record it in i_block[13];
				put_block(mip->dev, mip->INODE.i_block[13], (char*)double_int_buf); // write back to disc
			}
			/* Now blk is an address in i_block[13] table that points to the next table of pointers (this final table
			contains the pointers to data blocks).*/
			// NOW, get THAT blk into an int buf
			memset(double_int_buf, 0, BLKSIZE/sizeof(int));
			get_block(mip->dev, doubleblk, (char*)double_int_buf);
			/* MOD because logical block is num between 0 and 256^2; MOD gives us the entry in the final
			array of pointer. this blk is officially the blk we need... but we need to check if it's been allocated */
			// blk = double_int_buf[logical_block%(BLKSIZE/sizeof(int))];
			if (double_int_buf[logical_block%(BLKSIZE/sizeof(int))]==0){
				// allocate a disk block;
				blk = double_int_buf[logical_block%(BLKSIZE/sizeof(int))] = balloc(mip->dev);
				if (blk == 0) return 0;
				// increment iblock count
				mip->INODE.i_blocks++;
				// record it
				put_block(mip->dev, doubleblk, (char*)double_int_buf); // write back to disc
			}
		}

		char wbuf[BLKSIZE] = {0};
		/* all cases come to here : write to the data block */
		get_block(mip->dev, blk, wbuf);   // read disk block into wbuf[ ]  
		char *cp = wbuf + startByte;      // cp points at startByte in wbuf[]
		remain = BLKSIZE - startByte;     // number of BYTEs remain in this block

		// Optimized write code to write full chunk w/o loop.
		// write 'remain' bytes to block if remain <= nbytes; of course, if nbytes < remain, simply write nbytes
		int amount_to_write = (remain <= nbytes) ? remain : nbytes;
		// write!!! recall cp points out where we start write (wbuf+startByte); cq points at buf
		memcpy(cp, cq, amount_to_write);
		// Update control vars
		cp = cq = cp+amount_to_write;
		oftp->offset += amount_to_write;
		if (oftp->offset > mip->INODE.i_size)  // especially for RW|APPEND mode
			mip->INODE.i_size = oftp->offset;    // update file size to offset since offset points to after what was just written
		nbytes -= amount_to_write;
		put_block(mip->dev, blk, wbuf);   // write wbuf[ ] to disk
		
		// loop back to outer while to write more .... until nbytes are written
	}

	mip->dirty = 1;       // mark mip dirty for iput() 
	printf("my_write: wrote %d char into file descriptor fd=%d\n", strlen(buf), fd);           
	return nbytes;
}