#ifndef TYPE_H
#define TYPE_H

/*************** type.h file ************************/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <linux/fs.h>
#include <ext2fs/ext2_fs.h> // install with 'sudo apt-get install e2fslibs-dev' **note: Linux/ext2_fs.h is depreciated
#include <libgen.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

// define shorter TYPES for convenience
typedef struct ext2_super_block SUPER;
typedef struct ext2_group_desc GD;
typedef struct ext2_inode INODE;
typedef struct ext2_dir_entry_2 DIR;

SUPER *sp;
GD *gp;
INODE *ip;
DIR *dp;

// Block number of EXT2 FS on FD
#define SUPERBLOCK 1
#define GDBLOCK 2
#define ROOT_INODE 2

// Default dir and regular file modes
#define DIR_MODE 0x41ED
#define FILE_MODE 0x81A4
#define SUPER_MAGIC 0xEF53
#define SUPER_USER 0

// Proc status
#define FREE 0
#define READY 1

// file system table sizes
#define BLKSIZE 1024
#define NMINODE 128
#define NFD 16
#define NPROC 2
#define NMOUNT 4

// read/write/rw/append modes
#define READ 0
#define WRITE 1
#define READ_WRITE 2
#define APPEND 3

// In-memory inode structure
typedef struct minode
{
  INODE INODE;          // disk Inode
  int dev;              
  int ino;              
  int refCount;         // use count
  int dirty;            // modified flag
  int mounted;          // mounted flag
  struct mntable *mptr; // mount table pointer
} MINODE;

// Open file table
typedef struct oft
{
  int mode;     // mode of opened file
  int refCount; // number of PROCs sharing this instance
  MINODE *mptr; // pointer to minode of file
  int offset;   // byte offset for R|W
} OFT;

// Proc structure
typedef struct proc
{
  struct proc *next;
  int pid;
  int ppid;
  int status;
  int uid, gid;
  MINODE *cwd;
  OFT *fd[NFD];
} PROC;

// Mount Table structure
typedef struct mtable
{
  int dev;           // device number - 0 for FREE
  int ninodes;       // from superblock
  int nblocks;       
  int free_blocks;   // from superblock and GD
  int free_inodes;   
  int bmap;          // from group descriptor
  int imap;          
  int iblock;        // inodes start block
  MINODE *mntDirPtr; // mount point DIR pointer
  char devName[64];  // device name
  char mntName[64];  // mount point DIR name
} MTABLE;

#endif