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

int ls_file(MINODE *mip, char *name);
int ls_dir(MINODE *mip);
int ls(char *pathname);

/*************************************************************
* Function: ls_file(MINODE *mip, char *name)                *
* Date Created: 11/4/2020                                   *
* Date Last Modified:                                       *
* Description: "List" command specifically for a file.      *
* Input parameters: Pointer to MINODE mip, name of file.    *
* Returns: Returns success (redundant).                     *
* Preconditions: mip points to a valid MINODE, name         *
*                describes the file being listed.           *
* Postconditions:                                           *
*************************************************************/
int ls_file(MINODE *mip, char *name)
{
    // READ Chapter 11.7.3 HOW TO ls

    char *t1 = "xwrxwrxwr-------";
    char *t2 = "----------------";

    char ftime[256];
    u16 mode = mip->INODE.i_mode;

    if (S_ISREG(mode)) 
        printf("%c", '-');
    if (S_ISDIR(mode))
        printf("%c", 'd');
    if (S_ISLNK(mode))
        printf("%c", 'l');
    for (int i = 8; i >= 0; i--)
    {
        if (mode & (1 << i))
            printf("%c", t1[i]); // print r|w|x printf("%c", t1[i]);
        else
            printf("%c", t2[i]); // or print -
    }
    printf("%4d ", mip->INODE.i_links_count); // link count
    printf("%4d ", mip->INODE.i_gid);         // gid
    printf("%4d ", mip->INODE.i_uid);         // uid
    printf("%8d ", mip->INODE.i_size);       // file size

    strcpy(ftime, ctime((time_t *)&(mip->INODE.i_mtime))); // print time in calendar form ftime[strlen(ftime)-1] = 0; // kill \n at end
    ftime[strlen(ftime) - 1] = 0;                // removes the \n
    printf("%s ", ftime);                        // prints the time

    printf("%s", name);
    if (S_ISLNK(mode))
    {
        printf(" -> %s", (char *)mip->INODE.i_block); // print linked name 
    }

    printf("\n");
    return 0;
}

/*************************************************************
* Function: ls_dir(MINODE *mip)                             *
* Date Created: 11/4/2020                                   *
* Date Last Modified:                                       *
* Description: "List" command for a directory.              *
* Input parameters: MINODE pointer mip.                     *
* Returns: Returns success (redundant).                     *
* Preconditions: mip points to a valid MINODE.              *
* Postconditions:                                           *
*************************************************************/
int ls_dir(MINODE *mip)
{
    char *t1 = "xwrxwrxwr-------";
    char *t2 = "----------------";

    char ftime[256];
    u16 mode = mip->INODE.i_mode;

    char buf[BLKSIZE], temp[256];
    DIR *dp;
    char *cp;

    // Assume DIR has only one data block i_block[0]
    get_block(dev, mip->INODE.i_block[0], buf);
    dp = (DIR *)buf;
    cp = buf;

    while (cp < buf + BLKSIZE)
    {
        strncpy(temp, dp->name, dp->name_len);
        temp[dp->name_len] = 0;

        MINODE *temp_mip = iget(dev, dp->inode);
        ls_file(temp_mip, temp);

        //printf("[%d %s]  ", dp->inode, temp); // print [inode# name]

        cp += dp->rec_len;
        dp = (DIR *)cp;
    }
    printf("\n");
    return 0;
}

/*************************************************************
* Function: my_ls(char *pathname)                           *
* Date Created: 11/4/2020                                   *
* Date Last Modified:                                       *
* Description: "List" parent command to call ls_file or     *
*               ls_dir as necessary.                        *
* Input parameters: Pathname string.                        *
* Returns: Returns success.                                 *
* Preconditions: Pathname string refers to a valid path.    *
* Postconditions:                                           *
*************************************************************/
int my_ls(char *pathname)
{
    int mode;
    printf("ls %s\n", pathname);
    if (!strcmp(pathname, ""))
    {
        ls_dir(running->cwd);
    }
    else
    {
        int ino = getino(pathname);
        if (ino == 0)
        {
            printf("inode DNE\n");
            return -1;
        }
        else
        {
            int dev = root->dev;
            MINODE *mip = iget(dev, ino);
            mode = mip->INODE.i_mode;
            if (S_ISDIR(mode))
            {
                ls_dir(mip);
            }
            else
            {
                ls_file(mip, pathname);
            }
        }
    }
    return 0;
}