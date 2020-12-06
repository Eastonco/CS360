//#include "../type.h"
#include "../cmd.h"

// globals
extern MINODE minode[NMINODE];
extern MINODE *root;

extern PROC proc[NPROC], *running;

extern char gpath[128]; // global for tokenized components
extern char *name[32];  // assume at most 32 components in pathname
extern int n;           // number of component strings in name[]

extern int fd, dev;
extern int nblocks, ninodes, bmap, imap, inode_start;

/****************************************************************
* Function: int make_dir(char *pathname)                        *
* Date Created: 11/18/2020                                      *
* Date Last Modified:                                           *
* Description: creates a directory at a given path              *
* Input parameters: the pathname/dirname                        *
* Returns: 0 if success, -1 if fail                             *
* Preconditions:                                                *
* Postconditions:                                               *
*****************************************************************/
int make_dir(char *pathname)
{
    MINODE *start;
    char pathcpy1[256], pathcpy2[256];

    strcpy(pathcpy1, pathname);
    strcpy(pathcpy2, pathname);

    if (pathname[0] == '/') // absolute vs relative pathname
    {
        start = root;
        dev = root->dev;
    }
    else
    {
        start = running->cwd;
        dev = running->cwd->dev;
    }

    char *parent = dirname(pathcpy1); //dir and basename are distructive to strings
    char *child = basename(pathcpy2);

    int pino = getino(parent); // get partent inode numbe

    if (pino == -1) // Verrifies ino exists
    {
        printf("ERROR: parent %s doesn't exist\n", parent);
        return -1;
    }

    MINODE *pip = iget(dev, pino); // get inode of parent

    if (!S_ISDIR(pip->INODE.i_mode)) // verrfies parent is a dir
    {
        printf("ERROR: %s is not a directory\n", parent);
        return -1;
    }

    if (!search(pip, child)) // verrifes the child name doesn't already exist
    {
        printf("ERROR: child %s already exists under parent %s", child, parent);
        return -1;
    }

    mymkdir(pip, child);           // makes the dir
    pip->INODE.i_links_count++;    // dirs increment link count
    pip->INODE.i_atime = time(0L); // reset the time
    pip->dirty = 1;                // mark as changed
    iput(pip);                     //writes the changed MINODE to the block
    return 0;
}

/****************************************************************
* Function: int mymkdir(MINODE *pip, char *name)                *
* Date Created: 11/18/2020                                      *
* Date Last Modified:                                           *
* Description: creates the dir and default files inside         *
* Input parameters: minode to parent, new name of child dir     *
* Returns: 0 on success                                         *
* Preconditions: called from make_dir(char *pathname)          *
* Postconditions:                                               *
*****************************************************************/
int mymkdir(MINODE *pip, char *name)
{
    MINODE *mip;
    char *buf[BLKSIZE], *cp;
    DIR *dp;

    int ino = ialloc(dev); // allocate a new inode
    int bno = balloc(dev); // allocate a new block

    printf("ino: %d, bno: %d\n", ino, bno);

    mip = iget(dev, ino); // get the newly allocated inode

    INODE *ip = &mip->INODE;
    ip->i_mode = DIR_MODE;       // OR 040755: DIR type and permissions
    ip->i_uid = running->uid;    // Owner uid
    ip->i_gid = running->gid;    // Group Id
    ip->i_size = BLKSIZE;        // Size in bytes
    ip->i_links_count = 2;       // Links count=2 because of . and ..
    ip->i_atime = time(0L);      // set to current time
    ip->i_ctime = time(0L);      // set to current time
    ip->i_mtime = time(0L);      // set to current time
    ip->i_blocks = 2;            // LINUX: Blocks count in 512-byte chunks
    ip->i_block[0] = bno;        // new DIR has one data block
    for (int i = 1; i < 15; i++) // clears all the block memeory
    {
        ip->i_block[i] = 0;
    }

    mip->dirty = 1; // mark minode dirty
    iput(mip);      // write INODE to disk

    get_block(dev, bno, buf); // get the newly allocated block
    dp = (DIR *)buf;
    cp = buf;

    printf("Create . and .. in %s\n", name);

    // making . entry
    dp->inode = ino;   // child ino
    dp->rec_len = 12;  // 4 * [(8 + name_len + 3) / 4]
    dp->name_len = 1;  // len of name
    dp->name[0] = '.'; // name

    cp += dp->rec_len; // advancing to end of '.' entry
    dp = (DIR *)cp;

    //making .. entry
    dp->inode = pip->ino;       // setting to parent ino
    dp->rec_len = BLKSIZE - 12; // size is rest of block
    dp->name_len = 2;           // size of the name
    dp->name[0] = '.';
    dp->name[1] = '.';

    put_block(dev, bno, buf); // write the block

    enter_name(pip, ino, name); // add the name to the block

    return 0;
}

/****************************************************************
* Function: int creat_file(char *pathname)                      *
* Date Created: 11/12/2020                                      *
* Date Last Modified:                                           *
* Description: creates a file with name pathname                *
* Input parameters: pathname/name of file                       *
* Returns: 0 if success, -1 if fail                             *
* Preconditions:                                                *
* Postconditions:                                               *
*****************************************************************/
int creat_file(char *pathname)
{
    MINODE *start;
    char pathcpy1[256], pathcpy2[256];

    strcpy(pathcpy1, pathname);
    strcpy(pathcpy2, pathname);

    if (pathname[0] == '/') // relative or absolute pathname
    {
        start = root;
        dev = root->dev;
    }
    else
    {
        start = running->cwd;
        dev = running->cwd->dev;
    }

    char *parent = dirname(pathcpy1); //dir and basename are distructive to strings
    char *child = basename(pathcpy2);

    int pino = getino(parent); // get the parent inode number

    if (pino == -1) // Verrifies ino exists
    {
        printf("ERROR: parent %s doesn't exist\n", parent);
        return -1;
    }

    MINODE *pip = iget(dev, pino); // get the minode from pino

    if (!S_ISDIR(pip->INODE.i_mode)) // verrifies parent is dir
    {
        printf("ERROR: %s is not a directory\n", parent);
        return -1;
    }

    if (!search(pip, child)) // verrifes name doesn't already exist in the parent dir
    {
        printf("ERROR: child %s already exists under parent %s", child, parent);
        return -1;
    }

    my_creat(pip, child);          // creates the file
    pip->INODE.i_atime = time(0L); // sets time
    pip->dirty = 1;                // mark as changed
    iput(pip);                     //writes the changed MINODE to the block
    return 0;
}

/****************************************************************
* Function: my_creat(MINODE *pip, char *name)                   *
* Date Created: 11/12/2020                                      *
* Date Last Modified:                                           *
* Description: creates a file with stadard permissions          *
* Input parameters: parent minode, name of file                 *
* Returns: 0 if success                                         *
* Preconditions: called from creat_file(char *pathname)         *
* Postconditions:                                               *
*****************************************************************/
int my_creat(MINODE *pip, char *name)
{
    MINODE *mip;
    char *buf[BLKSIZE], *cp;
    DIR *dp;

    int ino = ialloc(dev); // allocates new inode
    int bno = balloc(dev); // allocdes new block

    printf("ino: %d, bno: %d\n", ino, bno);

    mip = iget(dev, ino); // get the minode from memory

    INODE *ip = &mip->INODE;
    ip->i_mode = FILE_MODE;   // 0x81A4 OR 0100644: FILE type and permissions
    ip->i_uid = running->uid; // Owner uid
    ip->i_gid = running->gid; // Group Id
    ip->i_size = BLKSIZE;     // Size in bytes
    ip->i_links_count = 1;    // Links count=1 since it's a file
    ip->i_atime = time(0L);   // set to current time
    ip->i_ctime = time(0L);
    ip->i_mtime = time(0L);
    ip->i_blocks = 2;            // LINUX: Blocks count in 512-byte chunks
    ip->i_block[0] = 0;          // new File has 0 data blocks
    for (int i = 1; i < 15; i++) // clear block memory
    {
        ip->i_block[i] = 0;
    }

    mip->dirty = 1; // mark minode dirty
    iput(mip);      // write INODE to disk

    enter_name(pip, ino, name); // add the name to the block

    return 0;
}