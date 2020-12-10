# **Command algorithm explanations:**

## mkdir  pathname

“Get the parent index node and search in the parent for child to ensure it does not exist yet. Allocate a new block and inode, get the new allocated inode and set directory settings, set the first i_block to the allocated bno. Get the bno block into memory and create . and .. entries, write the block back. Then enter_name to add the name to the parent block.”

Sub-question: how to enter_name?
*   Get the inode of the provided parent and traverse to the last entry of the block-- once you’re at the last entry, ensure there’s enough space for the name, and then strcpy the name and the inode into the block and save it (put_block).


## creat  pathname

Same as mkdir, except;

*   Mode of the index node is a FILE type and permissions (0x81A4) instead of DIR
*   Links count is initially 1 as it’s a file
*   No creation of . and .. necessary, thus get_block and set_block aren’t necessary in creat.


## link   f1 f2

Get the index node of f1 (oldname) into memory, check that it’s not a DIR and is a REG or LNK file, check both f1 and f2 are on the same device (same dev), add an entry to the data block of f2 which has the same inode number as f1 by enter_name.


## Unlink

Get index node into memory, decrement links count by 1 and check if it’s link count is 0, if so then truncate the inode by deallocating all the data blocks of the inode (function truncate_inode). Remove the child name from the base directory with rm_child given the parent inode (MINODE *) and the child basename.


## Symlink

Gets index node into memory of the ‘old' (first argument), creates a file for ‘new’ (second argument) and get ino, set mode of that new ino to link, copy the old string into the new’s i_block (84 bytes available, 60 chars + 24 after), write inode of new back to disk. (iput)


## open  file for R of W

how did you handle I, D blocks in read(), write()

**Open:**

Get index node into memory (MINODE *) for filename, check mode is valid, check the PROC (running)’s OFT table doesn’t already have filename opened with an incompatible mode (if the OFT’s mptr is mip). Otherwise allocate a new OFT struct, set offset based on mode, set the free entry in the OFT table to the new OFT struct and return the new file descriptor (index in the OFT table).

**Indirect blocks:**

Need to “follow the pointer” at i_block[12] or i_block[13] respectively to get to the indirect or doubly indirect blocks. From there mailman’s algorithm can be used once the logical block has been reset to 0 relatively (as the mailman’s algorithm needs to start from 0.) Read in the buffer based on the blk division (logical blk / 256, 256 = how many ints are in BLKSIZE) and then mod by 256 to get the correct blk.

Doubly indirect needs to do that above process twice (so you need to get_block() twice, essentially need to follow two “pointers”).

**Optimization:**

After you’ve gotten the block, optimization of READ or WRITE copies the entire BLKSIZE at a time (either reads n_bytes at a time or remainder at a time). This is done via memcpy(). This is opposed to a single character at a time.

      
      
## mount: what does it do?

how to cross mounting points DOWNward? UPward?

**What does mount do/how does it work?**

Mount mounts another EXT2 filesystem (another device) into an available directory in the filesystem. Crossing mounting points refers to when device numbers need to be changed. It creates an entry in the global mount table where an entry is free (dev=0), and if opens the provided filesystem for READ_WRITE: it gets the superblock and group descriptors (block 1 and 2) of that filesystem, checks if it’s EXT2 (magic number), and otherwise sets the minode of mounting_point as mounted and sets it to point at the table entry (and the table entry to point at it).

**DOWNward:**

See if a given minode in \() is mounted (1), if so follow to the mount table entry, using the device number in the mount table entry iget the root index node (ino=2), then keep traversing using that as your mip

**UPward:**

If the inode number is root (2), but the device number differs from the root device number (recorded at the beginning of the program’s lifetime), we know we hit a mounting point from below. Search the mount table entries for a matching device number, follow the mount directory pointer (mntDirPtr) to a MINODE pointer, set dev to that MINODE’s dev, and keep traversing.

WHAT WORKS IN OUR CODE (level 3)



*   mount; mount disk3.2 /mnt; mount
    *   Displays no mounted filesystems, then disk3.2 as mounted
*   cd /mnt; ls
    *   Downwards, Displays disk3.2 contents
*   cd ..; ls
    *   Upwards, Displays disk3.1 contents, BUT lose track of mounted disk3.2 (not visible in mount)
*   Switch; rmdir dir1; unlink file1
    *   Switches process to P1 from P0 (superuser), cannot rmdir (uid mismatch), cannot unlink (no permissions)

WHAT DOESN’T WORK IN OUR CODE (level 3)



*   Pwd (unmodified, so crashes upon use in a mounted filesystem)
*   Moving upward through a mounting point unmounts the filesystem you moved out of

BE READY TO SHOW CODE:

LEVEL-1:

  mkdir: enter_name()

  rmdir: rm_child()

  link : the link code

LEVEL-2:

read:  Indirect and Double Indirect data blocks; optimization code

write: Indirect and Double Indirect data blocks; optimization code

LEVEL-3:

Cross mounting point code in (YOUR modified) getino(), pwd()