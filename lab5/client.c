/*******************************************************
 * CS360 Lab5 Client File, client.c
 * Connor Easton, Zach Nett
********************************************************/
//#include "lab5.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <libgen.h> // for dirname()/basename()
#include <time.h>

#define MAX 256
#define BLK 1024
#define EOT "\\r\\n\\r\\n"

struct sockaddr_in saddr;
char *serverIP = "127.0.0.1";
int serverPORT = 1234;
int sock;
int debug = 1;

// Function prototypes
int find_cmd_index(char *command);
int lcat(char *filename);
int lls(char * pathname);
int ls_dir(char *pathname);
int ls_file(char *fname);
int lcd(char *pathname);
int is_end_of_tranmission(char * response);
int lpwd();
int lmkdir(char *pathname);
int lrmdir(char *pathname);
int lrm(char *pathname);
int init();

// Command table (for function pointers)
char *cmd[] = {"lcat", "lls", "lcd", "lpwd", "lmkdir", "lrmdir", "lrm"};

int (*fptr[])(char *) = {(int (*)())lcat, lls, lcd, lpwd, lmkdir, lrmdir, lrm};

int init()
{
    int n;

    printf("Creating a socket... ");
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        printf("FAIL\n");
        exit(0);
    }
    printf("Done.\n");

    printf("Filling server IP=%s, port number=%d... ", serverIP, serverPORT);
    bzero(&saddr, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = inet_addr(serverIP);
    saddr.sin_port = htons(serverPORT);
    printf("Done.\n");

    printf("Connecting to server... ");
    if (connect(sock, (struct sockaddr *)&saddr, sizeof(saddr)) != 0)
    {
        printf("FAIL\n");
        exit(0);
    }

    printf("\nConnected to server OK\n");
}

int main(int argc, char *argv[], char *env[])
{

    int n;
    char line[MAX], response[MAX];
    char command[16], arg[64];

    init();

    while (1)
    {
        memset(command, '\0', sizeof(command));
        memset(arg, '\0', sizeof(arg));
        memset(line,'\0', sizeof(line));
        memset(response,'\0', sizeof(response));



        printf("$ ");
        fgets(line, MAX, stdin);
        line[strlen(line) - 1] = 0; // kill <CR> at end
        if (line[0] == 0)           // exit if NULL line
            exit(0);

        // CHeck here if the command should be executed locally
        sscanf(line, "%s %s", command, arg);
        if (find_cmd_index(command) != -1)
        { // local command-- run on client only
            (debug) ? printf("%s\n", command) : NULL;
            int index = find_cmd_index(command);
            if (index != -1)
            {
                fptr[index](arg);
            }
            else
            {
                printf("invalid command %s\n", line);
            }
        }
        else
        { // else send to server
            // Send ENTIRE line to server
            n = write(sock, line, MAX);
            (debug) ? printf("Sent: %s\n", line) : NULL;
            char command[16], arg[64];
            sscanf(line, "%s %s", command, arg);
            char buf[MAX];
            if (!strcmp(command, "get")) {
                // get the # of bytes that the file is
                read(sock, buf, MAX);
                int file_size = atoi(buf);
                memset(buf, 0, sizeof(buf));
                // synchronize data for get, arg is filename
                int fd = open(arg, O_WRONLY|O_CREAT, 0644);
                if (fd != NULL) {
                    int bytes_read = 0;
                    while (bytes_read < file_size) {
                        bytes_read += read(sock, buf, MAX);
                        write(fd, buf, MAX);
                    }
                }
            } else if (!strcmp(command, "put")) {
                // synchronize data for put, arg is filename
            } else {
                // Read a line from sock and show it
                bzero(response, sizeof(response));
                n = read(sock, response, sizeof(response));
                while(!is_end_of_tranmission(response)){
                    (debug) ? printf("client read: %s", response) : printf("%s", response);

                    bzero(response, sizeof(response));
                    n = read(sock, response, sizeof(response));
                }
                // transmission ended
            }
        }
    }
}

int is_end_of_tranmission(char * response){
    if(!strcmp(response, EOT)){
        (debug) ? printf("End of transmission\n") : NULL;
        return 1;
    }
    return 0;
}

int find_cmd_index(char *command)
{
    int i = 0;
    while (cmd[i])
    {
        if (!strcmp(command, cmd[i]))
        {
            (debug) ?printf("Found %s, index %d\n", command, i) : NULL;
            return i;
        }
        i++;
    }
    return -1;
}

int lcat(char *filename)
{
    char buf[512];
    FILE *fd = fopen(filename, "r");
    if (fd != NULL)
    {
        while (fgets(buf, 512, fd) != NULL)
        {
            buf[strlen(buf) - 1] = '\0';
            puts(buf);
        }
    }
    else
    {
        printf("fopen failed, aborting\n");
    }
    return 1;
}

int ls_dir(char *pathname)
{
    struct dirent *dp;
    DIR *mydir;
    char fullPath[MAX * 2];
    memset(fullPath, '\0', sizeof(fullPath));

    if ((mydir = opendir(pathname)) == NULL)
    {
        perror("couldn't open pathname");
        return;
    }

    do
    {
        if ((dp = readdir(mydir)) != NULL)
        {
            memset(fullPath, '\0', sizeof(fullPath));
            strcpy(fullPath, pathname);
            strcat(fullPath, "/");
            strcat(fullPath, dp->d_name);
            ls_file(fullPath);
        }

    } while (dp != NULL);

    closedir(mydir);
    return;
}

int lls(char * pathname)
{
    if (!strcmp(pathname, "")){
        ls_dir("./");
        return;
    }
    ls_dir(pathname);
    return;
}

int ls_file(char *fname)
{
    char linkname[MAX];
    char *t1 = "xwrxwrxwr-------";
    char *t2 = "----------------";

    struct stat fstat, *sp;
    int r, i;
    char ftime[64];
    sp = &fstat;
    if ((r = lstat(fname, &fstat)) < 0)
    {
        printf("can’t stat %s\n", fname);
        exit(1);
    }
    if ((sp->st_mode & 0xF000) == 0x8000) // if (S_ISREG())
        printf("%c", '-');
    if ((sp->st_mode & 0xF000) == 0x4000) // if (S_ISDIR())
        printf("%c", 'd');
    if ((sp->st_mode & 0xF000) == 0xA000) // if (S_ISLNK())
        printf("%c", 'l');
    for (i = 8; i >= 0; i--)
    {
        if (sp->st_mode & (1 << i))
            printf("%c", t1[i]); // print r|w|x printf("%c", t1[i]);
        else
            printf("%c", t2[i]); // or print -
    }
    printf("%4d ", sp->st_nlink); // link count
    printf("%4d ", sp->st_gid);   // gid
    printf("%4d ", sp->st_uid);   // uid
    printf("%8d ", sp->st_size);  // file size

    strcpy(ftime, ctime(&sp->st_ctime)); // print time in calendar form ftime[strlen(ftime)-1] = 0; // kill \n at end
    ftime[strlen(ftime) - 1] = 0;        // removes the \n
    printf("%s ", ftime);                // prints the time

    printf("%s", basename(fname)); // print file basename // print -> linkname if symbolic file
    if ((sp->st_mode & 0xF000) == 0xA000)
    {
        readlink(fname, linkname, MAX);
        printf(" -> %s", linkname); // print linked name }
    }

    printf("\n");
}

int lcd(char *pathname)
{
    return chdir(pathname);
}

int lpwd()
{
    char buf[MAX];
    getcwd(buf, MAX);
    printf("%s\n", buf);
}

int lmkdir(char *pathname)
{
    return mkdir(pathname, 0755);
}

int lrmdir(char *pathname)
{
    return rmdir(pathname);
}

int lrm(char *pathname)
{
    return unlink(pathname);
}