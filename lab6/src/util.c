/*********** util.c — ext2 filesystem utility functions ****************/
/*
 * This file implements the core low-level operations for the ext2 filesystem:
 *
 *   Disk I/O:      get_block / put_block   (block-level read/write)
 *   Bitmaps:       tst_bit / set_bit / clr_bit  (inode and block allocation maps)
 *   Allocation:    ialloc / balloc / idealloc / bdealloc
 *   Free counts:   incFreeInodes / decFreeInodes / incFreeBlocks / decFreeBlocks
 *   Inode cache:   mialloc / midalloc / iget / iput
 *   Path traversal: tokenize / search / getino / findmyname / findino
 *   Directory ops: enter_name / rm_child / inode_truncate
 */
#include "type.h"

/* Globals defined in main.c, declared extern in type.h */
extern MINODE minode[NMINODE];
extern MINODE *root;

extern PROC proc[NPROC], *running;
extern MTABLE mount_table[NMOUNT];

extern char gpath[128];
extern char *name[64];
extern int n;

extern int fd, dev, root_dev;
extern int nblocks, ninodes, bmap, imap, inode_start;

/* ── Bitmap Operations ───────────────────────────────────────────── */

/*
 * tst_bit: test whether bit `bit` is set in the bitmap buffer `buf`.
 *
 * The bitmap stores one bit per inode/block. Bit `b` lives in byte `b/8`
 * at position `b%8` within that byte.
 * Returns non-zero if the bit is set (in use), 0 if clear (free).
 */
int tst_bit(char *buf, int bit)
{
    return buf[bit / 8] & (1 << (bit % 8));
}

/*
 * set_bit: mark bit `bitnum` as used in the bitmap buffer `buf`.
 *
 * BUG FIX: original code was: `if (buf[byte] |= (1 << bit)) { return 1; } return 0;`
 * The |= unconditionally modifies buf[byte]. The conditional just tested whether
 * the result was non-zero (which is almost always true, even before setting).
 * The fix: unconditionally set the bit, always return 0 (success).
 */
int set_bit(char *buf, int bitnum)
{
    int byte = bitnum / 8;
    int bit  = bitnum % 8;
    buf[byte] |= (1 << bit); /* set bit: OR in a 1 at position `bit` */
    return 0;
}

/*
 * clr_bit: mark bit `bitnum` as free in the bitmap buffer `buf`.
 *
 * BUG FIX: original code was: `if (buf[byte] &= ~(1 << bit)) { return 1; } return 0;`
 * Same problem as set_bit — the assignment happened inside the conditional.
 * After clearing, a 0 result is expected (bit cleared), but that made the
 * if-body unreachable for the normal case. Fix: unconditional clear, return 0.
 */
int clr_bit(char *buf, int bitnum)
{
    int byte = bitnum / 8;
    int bit  = bitnum % 8;
    buf[byte] &= ~(1 << bit); /* clear bit: AND with all-1s except position `bit` */
    return 0;
}

/* ── Free Count Maintenance ──────────────────────────────────────── */

/*
 * decFreeInodes: decrement the free inode count in both the superblock and
 * the group descriptor, then write both back to disk.
 *
 * Called after allocating an inode (ialloc). The superblock lives at block 1,
 * the group descriptor at block 2. Both maintain redundant free counts.
 */
int decFreeInodes(int dev)
{
    char buf[BLKSIZE];

    get_block(dev, 1, buf);
    sp = (SUPER *)buf;
    sp->s_free_inodes_count--;
    put_block(dev, 1, buf);

    get_block(dev, 2, buf);
    gp = (GD *)buf;
    gp->bg_free_inodes_count--;
    put_block(dev, 2, buf);
    return 0;
}

/*
 * decFreeBlocks: decrement the free block count in superblock and group descriptor.
 * Called after allocating a data block (balloc).
 */
int decFreeBlocks(int dev)
{
    char buf[BLKSIZE];

    get_block(dev, 1, buf);
    sp = (SUPER *)buf;
    sp->s_free_blocks_count--;
    put_block(dev, 1, buf);

    get_block(dev, 2, buf);
    gp = (GD *)buf;
    gp->bg_free_blocks_count--;
    put_block(dev, 2, buf);
    return 0;
}

/*
 * incFreeBlocks: increment the free block count in superblock and group descriptor.
 * Called when a data block is freed (bdealloc / inode_truncate).
 */
int incFreeBlocks(int dev)
{
    char buf[BLKSIZE];

    get_block(dev, 1, buf);
    sp = (SUPER *)buf;
    sp->s_free_blocks_count++;
    put_block(dev, 1, buf);

    get_block(dev, 2, buf);
    gp = (GD *)buf;
    gp->bg_free_blocks_count++;
    put_block(dev, 2, buf);
    return 0;
}

/*
 * incFreeInodes: increment the free inode count in superblock and group descriptor.
 * Called when an inode is freed (idealloc).
 */
int incFreeInodes(int dev)
{
    char buf[BLKSIZE];

    get_block(dev, 1, buf);
    sp = (SUPER *)buf;
    sp->s_free_inodes_count++;
    put_block(dev, 1, buf);

    get_block(dev, 2, buf);
    gp = (GD *)buf;
    gp->bg_free_inodes_count++;
    put_block(dev, 2, buf);
    return 0;
}

/* ── Inode and Block Allocation ─────────────────────────────────── */

/*
 * ialloc: allocate a free inode number from the inode bitmap.
 *
 * The inode bitmap is a block where each bit represents one inode.
 * Bit 0 = inode 1, bit 1 = inode 2, etc. (ext2 inodes are 1-indexed).
 * We scan for the first 0 bit (free), set it, write back the bitmap,
 * and decrement the free inode count.
 *
 * Returns: inode number (1-based) on success, 0 if no free inodes.
 */
int ialloc(int dev)
{
    int i;
    char buf[BLKSIZE];

    get_block(dev, imap, buf); /* imap = block number of the inode bitmap */

    for (i = 0; i < ninodes; i++)
    {
        if (tst_bit(buf, i) == 0) /* bit clear = inode is free */
        {
            set_bit(buf, i);
            put_block(dev, imap, buf);
            decFreeInodes(dev);
            printf("allocated ino = %d\n", i + 1);
            return i + 1; /* convert 0-based bit index to 1-based inode number */
        }
    }
    return 0; /* no free inodes */
}

/*
 * balloc: allocate a free data block from the block bitmap.
 *
 * Same algorithm as ialloc but for data blocks.
 * Returns: block number (1-based) on success, 0 if disk full.
 */
int balloc(int dev)
{
    int i;
    char buf[BLKSIZE];

    get_block(dev, bmap, buf); /* bmap = block number of the block bitmap */

    for (i = 0; i < nblocks; i++)
    {
        if (tst_bit(buf, i) == 0)
        {
            set_bit(buf, i);
            decFreeBlocks(dev);
            put_block(dev, bmap, buf);
            printf("Free disk block at %d\n", i + 1);
            return i + 1;
        }
    }
    return 0;
}

/*
 * idealloc: free an inode by clearing its bit in the inode bitmap.
 *
 * Converts 1-based inode number to 0-based bitmap index (ino - 1).
 * Then increments the free inode count in superblock/GD.
 */
int idealloc(int dev, int ino)
{
    char buf[BLKSIZE];

    if (ino > ninodes)
    {
        printf("inumber %d out of range\n", ino);
        return -1;
    }
    get_block(dev, imap, buf);
    clr_bit(buf, ino - 1);
    put_block(dev, imap, buf);
    incFreeInodes(dev);
    return 0;
}

/*
 * bdealloc: free a data block by clearing its bit in the block bitmap.
 *
 * Converts 1-based block number to 0-based bitmap index (bno - 1).
 */
int bdealloc(int dev, int bno)
{
    char buf[BLKSIZE];

    get_block(dev, bmap, buf);
    clr_bit(buf, bno - 1);
    put_block(dev, bmap, buf);
    incFreeBlocks(dev);
    return 0;
}

/* ── Disk I/O Primitives ─────────────────────────────────────────── */

/*
 * get_block: read one 1024-byte disk block into `buf`.
 *
 * All disk access in this filesystem goes through get_block/put_block.
 * lseek positions the file cursor to the start of block `blk`, then
 * read() fills the buffer. This is the "block device abstraction" —
 * the rest of the code never calls lseek/read directly.
 */
int get_block(int dev, int blk, char *buf)
{
    lseek(dev, (long)blk * BLKSIZE, 0);
    read(dev, buf, BLKSIZE);
    return 0;
}

/*
 * put_block: write one 1024-byte disk block from `buf` to disk.
 *
 * The complement of get_block. Used whenever an in-memory buffer
 * containing modified block data needs to be written back to disk.
 */
int put_block(int dev, int blk, char *buf)
{
    lseek(dev, (long)blk * BLKSIZE, 0);
    write(dev, buf, BLKSIZE);
    return 0;
}

/* ── Path Tokenization ───────────────────────────────────────────── */

/*
 * tokenize: split a pathname into component strings.
 *
 * Copies `pathname` into the global gpath[], then uses strtok() to split
 * on '/' delimiters. Each component is stored in name[0..n-1].
 *
 * Example: tokenize("/a/b/c") → name[0]="a", name[1]="b", name[2]="c", n=3
 *
 * Cross-reference: the same tokenization pattern appears in lab6/util.c
 * and lab3/sh_sim.c (tokenizeLine).
 */
int tokenize(char *pathname)
{
    char *s;
    strcpy(gpath, pathname);
    n = 0;
    s = strtok(gpath, "/");
    while (s)
    {
        name[n++] = s;
        s = strtok(0, "/");
    }
    return n;
}

/* ── In-Memory Inode Cache ───────────────────────────────────────── */

/*
 * mialloc: find a free slot in the minode[] cache and claim it.
 *
 * A slot is free when its refCount == 0. Sets refCount to 1 to claim it.
 * Returns NULL if the cache is full (filesystem panic).
 *
 * The minode cache avoids re-reading from disk when the same inode is
 * accessed multiple times (e.g., two processes with the same cwd).
 */
MINODE *mialloc(void)
{
    int i;
    for (i = 0; i < NMINODE; i++)
    {
        MINODE *mp = &minode[i];
        if (mp->refCount == 0)
        {
            mp->refCount = 1;
            return mp;
        }
    }
    printf("FS panic: out of minodes\n");
    return 0;
}

/*
 * midalloc: release a minode slot by zeroing its refCount.
 * The slot is then available for mialloc() to reuse.
 */
int midalloc(MINODE *mip)
{
    mip->refCount = 0;
    return 0;
}

/*
 * iget: get the in-memory minode for inode (dev, ino).
 *
 * Two cases:
 *   1. Already cached: search minode[] for a matching (dev, ino) entry,
 *      increment its refCount, and return it. No disk read needed.
 *   2. Not cached: allocate a fresh minode slot, compute which disk block
 *      holds this inode, read it, and copy the INODE into the slot.
 *
 * Inode location formula:
 *   block  = (ino - 1) / 8 + inode_start   (8 inodes per 128-byte inode, 8 per block if inode is 128B)
 *   offset = (ino - 1) % 8
 *
 * Returns a pointer to the MINODE with refCount already incremented.
 */
MINODE *iget(int dev, int ino)
{
    MINODE *mip;
    int i, block, offset;
    char buf[BLKSIZE];

    /* Search the cache first */
    for (i = 0; i < NMINODE; i++)
    {
        mip = &minode[i];
        if (mip->refCount && (mip->dev == dev) && (mip->ino == ino))
        {
            mip->refCount++;
            return mip;
        }
    }

    /* Not in cache — load from disk */
    mip = mialloc();
    mip->dev = dev;
    mip->ino = ino;
    block  = (ino - 1) / 8 + inode_start;
    offset = (ino - 1) % 8;
    get_block(dev, block, buf);
    INODE *iptr = (INODE *)buf + offset;
    mip->INODE = *iptr; /* copy the 128-byte on-disk inode into the minode */

    mip->refCount = 1;
    mip->mounted  = 0;
    mip->dirty    = 0;
    mip->mptr     = 0;
    return mip;
}

/*
 * iput: release a reference to a minode, writing it back to disk if needed.
 *
 * Decrements refCount. If refCount drops to 0 AND dirty=1, writes the INODE
 * back to the disk block it came from (writeback caching).
 *
 * This is the "put" in the get/put pair — every iget() must have a matching iput().
 */
int iput(MINODE *mip)
{
    INODE *iptr;
    int block, offset;
    char buf[BLKSIZE];

    if (mip == 0)
        return -1;

    mip->refCount--;

    if (mip->refCount > 0)
        return -1; /* still has other users; don't write back yet */

    if (mip->dirty == 0)
        return -1; /* not modified; no need to write back */

    /* Write INODE back to the same block it was read from */
    block  = (mip->ino - 1) / 8 + inode_start;
    offset = (mip->ino - 1) % 8;
    get_block(mip->dev, block, buf);
    iptr  = (INODE *)buf + offset;
    *iptr = mip->INODE; /* copy modified in-memory INODE back into block buffer */
    put_block(mip->dev, block, buf);
    return 0;
}

/* ── Directory Search ────────────────────────────────────────────── */

/*
 * search: look for a filename in a directory's direct data blocks.
 *
 * Walks through the directory's i_block[0..11] (up to 12 direct blocks).
 * Each block is formatted as a sequence of variable-length DIR entries:
 *
 *   [inode|rec_len|name_len|file_type|name...]  <- entry 1
 *   [inode|rec_len|name_len|file_type|name...]  <- entry 2 (starts at entry1 + rec_len)
 *   ...
 *
 * rec_len is the actual space taken by this entry (padded to 4-byte alignment).
 * The last entry's rec_len extends to the end of the block.
 *
 * Returns: the inode number of the matching entry, or 0 if not found.
 */
int search(MINODE *mip, char *lname)
{
    int i;
    char *cp, temp[256], sbuf[BLKSIZE];
    DIR *dp;

    for (i = 0; i < 12; i++)
    {
        if (mip->INODE.i_block[i] == 0)
            return -1; /* no more blocks to search */

        get_block(mip->dev, mip->INODE.i_block[i], sbuf);
        dp = (DIR *)sbuf;
        cp = sbuf;

        while (cp < sbuf + BLKSIZE)
        {
            strncpy(temp, dp->name, dp->name_len);
            temp[dp->name_len] = 0;
            printf("%8d%8d%8u %s\n", dp->inode, dp->rec_len, dp->name_len, temp);
            if (strcmp(lname, temp) == 0)
            {
                printf("found %s : inumber = %d\n", lname, dp->inode);
                return dp->inode;
            }
            cp += dp->rec_len; /* advance by this entry's actual (padded) size */
            dp = (DIR *)cp;
        }
    }
    return 0;
}

/*
 * getino: resolve a pathname to its inode number.
 *
 * This is the core path traversal algorithm — the equivalent of the kernel's
 * namei() function. It works component by component:
 *
 *   1. Start at root (absolute path) or cwd (relative path)
 *   2. For each name component: search() the current directory for it
 *   3. iget() the found inode, iput() the previous one, repeat
 *   4. Handle mount points (upward and downward traversal)
 *
 * BUG FIX: on error paths (not a directory, component not found), the original
 * set mip->dirty = 1 before calling iput(). This would cause the unchanged
 * INODE to be unnecessarily written back to disk. Fixed to dirty = 0.
 */
int getino(char *pathname)
{
    MINODE *mip;
    int i, ino;

    if (strcmp(pathname, "/") == 0)
        return 2; /* root inode is always 2 in ext2 */

    if (pathname[0] == '/')
    {
        /* Absolute path: start traversal from root */
        dev = root->dev;
        ino = root->ino;
    }
    else
    {
        /* Relative path: start from current working directory */
        dev = running->cwd->dev;
        ino = running->cwd->ino;
    }

    mip = iget(dev, ino);
    tokenize(pathname);

    for (i = 0; i < n; i++)
    {
        if (!S_ISDIR(mip->INODE.i_mode))
        {
            printf("%s is not a directory\n", name[i]);
            mip->dirty = 0; /* BUG FIX: was dirty=1; no modification made */
            iput(mip);
            return -1;
        }

        printf("inode #: %d\n", mip->ino);
        ino = search(mip, name[i]);

        if (!ino)
        {
            printf("no such component name %s\n", name[i]);
            mip->dirty = 0; /* BUG FIX: was dirty=1 */
            iput(mip);
            return -1;
        }
        else if (ino == 2 && dev != root_dev)
        {
            /* Upward traversal: we've reached the root of a mounted FS.
             * Switch to the mount point directory on the parent FS. */
            printf("UPWARDS\n");
            for (int j = 0; j < NMOUNT; j++)
            {
                if (mount_table[j].dev == dev)
                {
                    iput(mip);
                    mip = mount_table[j].mntDirPtr;
                    dev = mip->dev;
                    break;
                }
            }
        }
        else
        {
            mip->dirty = 0;
            iput(mip);
            mip = iget(dev, ino);

            /* Downward traversal: if this inode is a mount point,
             * switch to the root of the mounted filesystem instead. */
            if (mip->mounted)
            {
                MTABLE *mtptr = mip->mptr;
                dev = mtptr->dev;
                ino = 2; /* root of mounted filesystem */
                printf("get dev of %d\n", dev);
                iput(mip);
                mip = iget(dev, ino);
            }
        }
    }

    mip->dirty = 0;
    iput(mip);
    return ino;
}

/*
 * findmyname: given a parent directory minode and a child inode number,
 * return the child's filename by scanning the parent's directory entries.
 *
 * Used by pwd() to reconstruct the path by walking up via parentPtr and
 * looking up the name at each level.
 */
int findmyname(MINODE *parent, u32 myino, char *myname)
{
    int i;
    char *cp, temp[256], sbuf[BLKSIZE];
    DIR *dp;
    MINODE *mip = parent;

    for (i = 0; i < 12; i++)
    {
        if (mip->INODE.i_block[i] == 0)
            return -1;

        get_block(mip->dev, mip->INODE.i_block[i], sbuf);
        dp = (DIR *)sbuf;
        cp = sbuf;

        while (cp < sbuf + BLKSIZE)
        {
            strncpy(temp, dp->name, dp->name_len);
            temp[dp->name_len] = 0;

            if (dp->inode == myino)
            {
                strncpy(myname, dp->name, dp->name_len);
                myname[dp->name_len] = 0;
                return 0;
            }
            cp += dp->rec_len;
            dp = (DIR *)cp;
        }
    }
    return -1;
}

/*
 * findino: return the inode numbers of '.' and '..' from a directory block.
 *
 * Every directory's first block starts with two fixed entries:
 *   entry 0: "."  (current directory) — inode = this directory's inode
 *   entry 1: ".." (parent directory)  — inode = parent's inode
 *
 * Used by pwd() and cd("..") to navigate upward in the tree.
 * Returns the '..' inode number directly; sets *myino to the '.' inode number.
 */
int findino(MINODE *mip, u32 *myino)
{
    char buf[BLKSIZE], *temp_ptr;
    DIR *dp;

    get_block(mip->dev, mip->INODE.i_block[0], buf);
    temp_ptr = buf;
    dp = (DIR *)buf;
    *myino = dp->inode;     /* '.' entry: inode of current directory */
    temp_ptr += dp->rec_len;
    dp = (DIR *)temp_ptr;
    return dp->inode;       /* '..' entry: inode of parent directory */
}

/* ── Directory Entry Management ─────────────────────────────────── */

/*
 * enter_name: add a new directory entry (myino, myname) to parent directory pip.
 *
 * Directory entries are variable-length and packed sequentially in data blocks.
 * Each entry's rec_len covers its own data PLUS any padding between it and the
 * next entry. The last entry's rec_len extends to the end of the block.
 *
 * To insert a new entry: walk to the LAST entry in the last block, then check
 * if there's enough "slack" (rec_len - ideal_len) for the new entry. If yes,
 * trim the last entry to its ideal size and append the new entry in the gap.
 * If no, allocate a new block and put the new entry there as the sole entry.
 *
 * ideal_len = round up to 4-byte boundary: 4 * ((8 + name_len + 3) / 4)
 *             (8 = fixed fields: inode + rec_len + name_len + file_type)
 */
int enter_name(MINODE *pip, int myino, char *myname)
{
    char buf[BLKSIZE], *cp;
    int bno;
    INODE *iptr;
    DIR *dp;

    int need_len = 4 * ((8 + strlen(myname) + 3) / 4);

    iptr = &pip->INODE;

    for (int i = 0; i < 12; i++)
    {
        if (iptr->i_block[i] == 0)
            break;

        bno = iptr->i_block[i];
        get_block(pip->dev, iptr->i_block[i], buf);
        dp = (DIR *)buf;
        cp = buf;

        /* Walk to the last directory entry in this block */
        while (cp + dp->rec_len < buf + BLKSIZE)
        {
            printf("%s\n", dp->name);
            cp += dp->rec_len;
            dp = (DIR *)cp;
        }

        /* Check if the last entry has enough slack space for the new entry */
        int ideal_len = 4 * ((8 + dp->name_len + 3) / 4);
        int remainder = dp->rec_len - ideal_len;

        if (remainder >= need_len)
        {
            /* Trim last entry, append new entry in the freed space */
            dp->rec_len = ideal_len;
            cp += dp->rec_len;
            dp = (DIR *)cp;

            dp->inode    = myino;
            strcpy(dp->name, myname);
            dp->name_len = strlen(myname);
            dp->rec_len  = remainder; /* new entry inherits the slack */

            put_block(pip->dev, bno, buf);
            return 0;
        }
        else
        {
            /* No room in this block — allocate a new data block */
            iptr->i_size   = BLKSIZE;
            bno            = balloc(dev);
            iptr->i_block[i] = bno;
            pip->dirty     = 1;

            get_block(pip->dev, bno, buf);
            dp = (DIR *)buf;
            cp = buf;

            dp->name_len = strlen(myname);
            strcpy(dp->name, myname);
            dp->inode   = myino;
            dp->rec_len = BLKSIZE; /* sole entry spans the whole block */

            put_block(pip->dev, bno, buf);
            return 1;
        }
    }
    return -1;
}

/*
 * rm_child: remove a named entry from a parent directory.
 *
 * Three cases based on the entry's position:
 *   1. First and only entry in a block → bdealloc the whole block,
 *      shift remaining i_block[] entries down to fill the hole.
 *   2. Last entry in a block (but not the only one) → the previous entry
 *      absorbs its rec_len, effectively extending to cover the gap.
 *   3. Middle entry → add its rec_len to the LAST entry in the block
 *      (ext2 convention), then memmove remaining entries left to close gap.
 */
int rm_child(MINODE *parent, char *name)
{
    DIR *dp, *prevdp, *lastdp;
    char *cp, *lastcp, buf[BLKSIZE], tmp[256], *startptr, *endptr;
    INODE *iptr = &parent->INODE;

    for (int i = 0; i < 12; i++)
    {
        if (iptr->i_block[i] == 0)
            continue;

        get_block(parent->dev, iptr->i_block[i], buf);
        dp = (DIR *)buf;
        cp = buf;

        while (cp < buf + BLKSIZE)
        {
            strncpy(tmp, dp->name, dp->name_len);
            tmp[dp->name_len] = 0;

            if (!strcmp(tmp, name))
            {
                if (cp == buf && cp + dp->rec_len == buf + BLKSIZE)
                {
                    /* Case 1: only entry in block — free the block */
                    bdealloc(parent->dev, iptr->i_block[i]);
                    iptr->i_size -= BLKSIZE;

                    /* Shift remaining i_block[] entries to fill the hole */
                    while (i + 1 < 12 && iptr->i_block[i + 1] != 0)
                    {
                        i++;
                        get_block(parent->dev, iptr->i_block[i], buf);
                        put_block(parent->dev, iptr->i_block[i - 1], buf);
                    }
                }
                else if (cp + dp->rec_len == buf + BLKSIZE)
                {
                    /* Case 2: last entry — previous entry absorbs its space */
                    prevdp->rec_len += dp->rec_len;
                    put_block(parent->dev, iptr->i_block[i], buf);
                }
                else
                {
                    /* Case 3: middle entry — give space to last entry, shift left */
                    lastdp = (DIR *)buf;
                    lastcp = buf;
                    while (lastcp + lastdp->rec_len < buf + BLKSIZE)
                    {
                        lastcp += lastdp->rec_len;
                        lastdp = (DIR *)lastcp;
                    }
                    lastdp->rec_len += dp->rec_len;

                    startptr = cp + dp->rec_len;
                    endptr   = buf + BLKSIZE;
                    memmove(cp, startptr, endptr - startptr);
                    put_block(parent->dev, iptr->i_block[i], buf);
                }

                parent->dirty = 1;
                iput(parent);
                return 0;
            }

            prevdp = dp;
            cp += dp->rec_len;
            dp = (DIR *)cp;
        }
    }
    printf("ERROR: child not found\n");
    return -1;
}

/*
 * inode_truncate: free all data blocks belonging to an inode.
 *
 * ext2 inodes have three levels of block pointers:
 *   i_block[0..11]  — direct blocks (12 blocks of data, up to 12KB)
 *   i_block[12]     — single indirect: points to a block of block numbers
 *   i_block[13]     — double indirect: points to a block of indirect blocks
 *   (i_block[14]    — triple indirect, not handled here)
 *
 * We deallocate all three levels by following the pointer chain, then
 * freeing the pointer block itself at each level.
 *
 * BUG FIX: original comparisons were `ip->i_block[12] != NULL` and
 * `ip->i_block[13] != NULL`. i_block[] contains int values (block numbers),
 * not pointers — comparing int to NULL is invalid. Fixed to `!= 0`.
 */
int inode_truncate(MINODE *mip)
{
    char buf[BLKSIZE];
    INODE *iptr = &mip->INODE;

    /* Level 1: free the 12 direct data blocks */
    for (int i = 0; i < 12; i++)
    {
        if (iptr->i_block[i] == 0)
            break;
        bdealloc(dev, iptr->i_block[i]);
        iptr->i_block[i] = 0;
    }

    /* Level 2: single indirect block — one block of pointers to data blocks */
    if (iptr->i_block[12] != 0) /* BUG FIX: was != NULL */
    {
        get_block(dev, iptr->i_block[12], buf);
        int *bp = (int *)buf; /* treat block contents as an array of block numbers */
        for (int i = 0; i < (int)(BLKSIZE / sizeof(int)); i++)
        {
            if (bp[i] == 0) break;
            bdealloc(dev, bp[i]);
            bp[i] = 0;
        }
        bdealloc(dev, iptr->i_block[12]); /* free the indirect pointer block itself */
        iptr->i_block[12] = 0;
    }

    /* Level 3: double indirect block — one block of pointers to indirect blocks */
    if (iptr->i_block[13] != 0) /* BUG FIX: was != NULL */
    {
        get_block(dev, iptr->i_block[13], buf);
        int *bp = (int *)buf;
        for (int i = 0; i < (int)(BLKSIZE / sizeof(int)); i++)
        {
            if (bp[i] == 0) break;
            /* Each bp[i] is itself an indirect block; free its data blocks first */
            char ibuf[BLKSIZE];
            get_block(dev, bp[i], ibuf);
            int *ibp = (int *)ibuf;
            for (int j = 0; j < (int)(BLKSIZE / sizeof(int)); j++)
            {
                if (ibp[j] == 0) break;
                bdealloc(dev, ibp[j]);
            }
            bdealloc(dev, bp[i]); /* free the indirect pointer block */
            bp[i] = 0;
        }
        bdealloc(dev, iptr->i_block[13]); /* free the double-indirect block */
        iptr->i_block[13] = 0;
    }

    mip->INODE.i_blocks = 0;
    mip->INODE.i_size   = 0;
    mip->dirty = 1;
    iput(mip);
    return 0;
}
