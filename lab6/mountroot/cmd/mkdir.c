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

int make_dir(char *pathname) //TODO: debug mkdir (currently can't make 2 dirs inside one another
{
    MINODE *start;
    char pathcpy1[256], pathcpy2[256];

    strcpy(pathcpy1, pathname);
    strcpy(pathcpy2, pathname);

    if (pathname[0] == '/')
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

    int pino = getino(parent);

    if (!pino)
    { // Verrifies ino exists
        printf("ERROR: parent %s doesn't exist\n", parent);
        return -1;
    }

    MINODE *pip = iget(dev, pino);

    if (!S_ISDIR(pip->INODE.i_mode)) // checks if parent is dir
    {
        printf("ERROR: %s is not a directory\n", parent);
        return -1;
    }

    if (!search(pip, child))
    {
        printf("ERROR: child %s already exists under parent %s", child, parent);
        return -1;
    }

    mymkdir(pip, child);
    pip->INODE.i_links_count++;
    pip->INODE.i_atime = time(0L); // TODO: idk what this is
    pip->dirty = 1;
    iput(pip); //writes the changed MINODE to the block
    return 0;
}

int mymkdir(MINODE *pip, char *name)
{
    MINODE *mip;
    char *buf[BLKSIZE], *cp;
    DIR *dp;

    int ino = ialloc(dev);
    int bno = balloc(dev);

    printf("ino: %d, bno: %d\n", ino, bno);

    mip = iget(dev, ino);

    INODE *ip = &mip->INODE;
    ip->i_mode = DIR_MODE;    // OR 040755: DIR type and permissions
    ip->i_uid = running->uid; // Owner uid
    ip->i_gid = running->gid; // Group Id
    ip->i_size = BLKSIZE;     // Size in bytes
    ip->i_links_count = 2;    // Links count=2 because of . and ..
    ip->i_atime = time(0L);   // set to current time
    ip->i_ctime = time(0L);
    ip->i_mtime = time(0L);
    ip->i_blocks = 2;     // LINUX: Blocks count in 512-byte chunks
    ip->i_block[0] = bno; // new DIR has one data block
    for (int i = 1; i < 15; i++)
    {
        ip->i_block[i] = 0;
    }

    mip->dirty = 1; // mark minode dirty
    iput(mip);      // write INODE to disk

    get_block(dev, bno, buf);
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
    dp->name_len = 2;
    dp->name[0] = '.';
    dp->name[1] = '.';

    put_block(dev, bno, buf);

    enter_name(pip, ino, name);

    return 0;
}

int enter_name(MINODE *pip, int myino, char *myname)
{
    char buf[BLKSIZE], *cp;
    int bno;
    INODE *ip;
    DIR *dp;

    int need_len = 4 * ((8 + strlen(myname) + 3) / 4); //ideal length of entry

    ip = &pip->INODE;

    for (int i = 0; i < 12; i++)
    {

        if (ip->i_block[i] == 0)
        {
            break;
        } 

        bno = ip->i_block[i];
        get_block(pip->dev, ip->i_block[i], buf);
        dp = (DIR *)buf;
        cp = buf;

        // Going to last entry of the block
        while (cp + dp->rec_len < buf + BLKSIZE)
        {
            printf("%s\n", dp->name);
            cp += dp->rec_len;
            dp = (DIR *)cp;
        }

        // at last entry
        int ideal_len = 4 * ((8 + dp->name_len + 3) / 4);
        int remainder = dp->rec_len - ideal_len;

        if (remainder >= need_len)
        {                            // space available for new netry
            dp->rec_len = ideal_len; //trim current entry to ideal len
            cp += dp->rec_len;       // advance to end
            dp = (DIR *)cp;          // point to new open entry space

            dp->inode = myino;
            strcpy(dp->name, myname);
            dp->name_len = strlen(myname);
            dp->rec_len = remainder;

            put_block(dev, bno, buf);
            return 0;
        }
        else
        { // not enough space in block
            ip->i_size = BLKSIZE;
            bno = balloc(dev); // allocate new block
            ip->i_block[i] = bno;
            pip->dirty = 1; // ino is changed so make dirty

            get_block(dev, bno, buf);
            dp = (DIR *)buf;
            cp = buf;

            dp->name_len = strlen(myname);
            strcpy(dp->name, myname);
            dp->inode = myino;
            dp->rec_len = BLKSIZE; // only entry so full size

            put_block(dev, bno, buf); //save
            return 1;
        }
    }
}