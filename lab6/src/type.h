#ifndef TYPE_H
#define TYPE_H

/*************** type.h file ************************/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <linux/fs.h>
#include <ext2fs/ext2_fs.h> /* install: sudo apt-get install e2fslibs-dev */
#include <libgen.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

/*
 * Shorter type aliases for the ext2 on-disk structures (from ext2_fs.h):
 *
 *   SUPER  — superblock: filesystem metadata (block size, inode count, magic number, etc.)
 *   GD     — group descriptor: per-block-group metadata (bitmap locations, inode table block)
 *   INODE  — on-disk inode: file metadata (size, permissions, block pointers)
 *   DIR    — directory entry: (inode, rec_len, name_len, name[]) for one entry in a directory
 */
typedef struct ext2_super_block  SUPER;
typedef struct ext2_group_desc   GD;
typedef struct ext2_inode        INODE;
typedef struct ext2_dir_entry_2  DIR;

/*
 * Global pointers to on-disk structures currently loaded in memory.
 * Declared extern here; defined once in main.c.
 *
 * BUG FIX: original code defined these directly in the header (no extern).
 * Including type.h in multiple .c files would create multiple definitions,
 * which is only tolerated with -fcommon (old C behavior). The correct approach
 * is to declare them extern here and define them in exactly one .c file.
 */
extern SUPER *sp;
extern GD    *gp;
extern INODE *ip;
extern DIR   *dp;

/* Well-known ext2 block numbers */
#define SUPERBLOCK  1  /* superblock is always at block 1 (byte offset 1024) */
#define GDBLOCK     2  /* group descriptor table starts at block 2 */
#define ROOT_INODE  2  /* root directory always has inode number 2 in ext2 */

/* Default permission modes for new directories and files */
#define DIR_MODE   0x41ED  /* directory: type bits | 0755 */
#define FILE_MODE  0x81A4  /* regular file: type bits | 0644 */
#define SUPER_MAGIC 0xEF53 /* ext2 magic number — validated at mount time */
#define SUPER_USER  0

/* Process status values */
#define FREE  0
#define READY 1

/* Filesystem table sizes */
#define BLKSIZE  1024  /* bytes per disk block (for 1K block size filesystems) */
#define NMINODE  128   /* size of the in-memory inode cache */
#define NFD      16    /* max open files per process */
#define NPROC    2     /* number of simulated processes */
#define NMOUNT   4     /* max simultaneously mounted filesystems */

/* File open modes */
#define READ       0
#define WRITE      1
#define READ_WRITE 2
#define APPEND     3

/*
 * MINODE: in-memory inode (the "minode" cache entry).
 *
 * ext2 inodes live on disk. When a file is accessed, its INODE is loaded into
 * an MINODE and cached here. The refCount tracks how many users hold this entry.
 * When dirty=1, the in-memory INODE differs from the disk copy and must be
 * written back (by iput()) when the last reference is dropped.
 *
 * This is the same concept as the Linux kernel's struct inode + inode cache.
 */
typedef struct minode
{
    INODE INODE;          /* the disk inode, copied into memory */
    int dev;              /* device (fd) this inode belongs to */
    int ino;              /* inode number on that device */
    int refCount;         /* number of active users; 0 = this slot is free */
    int dirty;            /* 1 = INODE modified in memory, needs writeback */
    int mounted;          /* 1 = another filesystem is mounted here */
    struct mntable *mptr; /* if mounted: pointer to that filesystem's mount table entry */
} MINODE;

/*
 * OFT: Open File Table entry.
 *
 * Created when a process calls open(). Tracks the current byte offset
 * and the mode (read/write/append). Multiple processes can share one OFT
 * entry (e.g., after fork()), tracked by refCount.
 */
typedef struct oft
{
    int mode;      /* how the file was opened: READ, WRITE, READ_WRITE, APPEND */
    int refCount;  /* number of PROC fd slots pointing to this OFT entry */
    MINODE *mptr;  /* the minode of the open file */
    int offset;    /* current read/write position in bytes */
} OFT;

/*
 * PROC: simulated process structure.
 *
 * Each PROC has its own current working directory (cwd) and open file table
 * (fd[]), mirroring how the Linux kernel tracks per-process file state.
 */
typedef struct proc
{
    struct proc *next;  /* circular linked list for round-robin scheduling */
    int pid;
    int ppid;
    int status;         /* FREE or READY */
    int uid, gid;
    MINODE *cwd;        /* current working directory minode */
    OFT *fd[NFD];       /* open file descriptors (NULL = unused slot) */
} PROC;

/*
 * MTABLE: mount table entry.
 *
 * When a filesystem is mounted, its superblock/GD data is cached here
 * so we don't re-read the disk for every path traversal. Also records
 * the device name and the directory where it's mounted (mntDirPtr).
 */
typedef struct mtable
{
    int dev;            /* device fd; 0 = this mount slot is FREE */
    int ninodes;        /* total inodes in this filesystem (from superblock) */
    int nblocks;        /* total blocks */
    int free_blocks;    /* free block count (from superblock + GD) */
    int free_inodes;    /* free inode count */
    int bmap;           /* block number of the block bitmap (from GD) */
    int imap;           /* block number of the inode bitmap (from GD) */
    int iblock;         /* first block of the inode table */
    MINODE *mntDirPtr;  /* minode of the directory where this FS is mounted */
    char devName[64];   /* device file path (e.g., "/dev/sdb1") */
    char mntName[64];   /* mount point path (e.g., "/mnt/disk") */
} MTABLE;

#endif
