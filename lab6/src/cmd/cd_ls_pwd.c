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
// Headers
int my_chdir(char *pathname);

/************************************************************
* Function: my_chdir(char *pathname)                        *
* Date Created: 11/4/2020                                   *
* Date Last Modified:                                       *
* Description: changes cwd to new pathname                  *
* Input parameters: path to new cwd                         *
* Returns: 1 if success, 0 if fail                          *
* Preconditions: must have initialized system               *
* Postconditions:                                           *
*************************************************************/
int my_chdir(char *pathname)
{
  printf("chdir %s\n", pathname);

  int ino = getino(pathname);
  if (ino == -1)
  {
    printf("ERROR: Chdir() - ino can't be found\n");
    return 0;
  }

  MINODE *mip = iget(dev, ino);

  if (!S_ISDIR(mip->INODE.i_mode))
  {
    printf("Error: Chdir() - mip is not a directory");
    return 0;
  }

  iput(running->cwd);
  running->cwd = mip;
  return 1;
}


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
    MINODE *temp_mip;

    u16 mode = mip->INODE.i_mode;

    char buf[BLKSIZE], temp[BLKSIZE];
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

        temp_mip = iget(dev, dp->inode);
        ls_file(temp_mip, temp);
        temp_mip->dirty = 1;
        iput(temp_mip); //----------------------- FIXME: this causes the loop

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
        if (ino == -1)
        {
            printf("inode DNE\n");
            return -1;
        }
        else
        {
            dev = root->dev;
            MINODE *mip = iget(dev, ino);
            mode = mip->INODE.i_mode;
            if (S_ISDIR(mode))
            {
                ls_dir(mip);
            }
            else
            {
                ls_file(mip, basename(pathname));
            }
            iput(mip); //-----------FIXME: this also causes the loop
        }
    }
    return 0;
}


/************************************************************
* Function: my_pwd(MINODE *wd)                              *
* Date Created: 11/4/2020                                   *
* Date Last Modified:                                       *
* Description: Prints the working directory to console      *
* Input parameters:  working directory MINODE               *
* Returns: NULL - Prints out the working dir                *
* Preconditions: wd and root must bet set to a node         *
* Postconditions:                                           *
*************************************************************/
char *my_pwd(MINODE *wd)
{
    if (wd == root)
    {
        printf("/\n");
    }
    else
    {
        rpwd(wd);
        printf("\n");
    }
}

/************************************************************
* Function:rpwd(MINODE *wd)                                 *
* Date Created: 11/4/2020                                   *
* Date Last Modified:                                       *
* Description: Recursive helper to pwd                      *
* Input parameters: MINODE of directory to be printed       *
* Returns: NULL - Prints out the working dir                *
* Preconditions: wd and root must bet set to a node         *
* Postconditions:                                           *
*************************************************************/
void rpwd(MINODE *wd)
{
    if (wd == root)
    {
        return;
    }
    char buf[BLKSIZE], lname[256];
    int ino;
    get_block(dev, wd->INODE.i_block[0], buf);
    int parent_ino = findino(wd, &ino);
    MINODE *pip = iget(dev, parent_ino);
    
    findmyname(pip, ino, lname);
    rpwd(pip);
    pip->dirty = 1;
    iput(pip);
    printf("/%s", lname);
    return;
}