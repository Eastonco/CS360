/*******************************************************
 * CS360 Lab5 Client File, client.c
 * Connor Easton, Zach Nett
 *
 * The client connects to the server over TCP and provides two types of commands:
 *   - Local commands (lcat, lls, lcd, lpwd, lmkdir, lrmdir, lrm): executed on
 *     the client machine, no network traffic.
 *   - Remote commands (get, put, ls, cd, pwd, mkdir, rmdir, rm): sent as text
 *     to the server; the client reads the response until it receives EOT.
 *
 * File transfer protocol:
 *   get/put use a size-prefix scheme:
 *     1. Sender writes file size as ASCII string (MAX bytes)
 *     2. Sender writes file data in MAX-byte chunks
 *     3. Receiver reads size, then reads exactly that many bytes
 ********************************************************/
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
#include <libgen.h>
#include <time.h>

#define MAX 256
#define BLK 1024
#define EOT "\\r\\n\\r\\n"

#define DEBUG 0

struct sockaddr_in saddr;
char *serverIP   = "127.0.0.1";
int   serverPORT = 1234;
int   sock;

/* Local command table */
char *cmd[] = {"lcat", "lls", "lcd", "lpwd", "lmkdir", "lrmdir", "lrm", "menu"};
#define NCMDS ((int)(sizeof(cmd) / sizeof(cmd[0])))

int find_cmd_index(char *command);
int lcat(char *filename);
int lls(char *pathname);
int ls_dir(char *pathname);
int ls_file(char *fname);
int lcd(char *pathname);
int is_end_of_tranmission(char *response);
int lpwd(char *unused);    /* BUG FIX: added char* param to match fptr table */
int lmkdir(char *pathname);
int lrmdir(char *pathname);
int lrm(char *pathname);
int menu(char *unused);    /* BUG FIX: added char* param to match fptr table */
int init(void);

/*
 * Function pointer table for local commands.
 * All handlers take (char *) for type consistency with the dispatch mechanism.
 * BUG FIX: original used (int (*)()) cast which masked type mismatches;
 * all functions now properly declared as int (*)(char *).
 */
int (*fptr[])(char *) = {
    lcat, lls, lcd, lpwd, lmkdir, lrmdir, lrm, menu
};

/*
 * init: connect to the server.
 *
 * TCP client lifecycle:
 *   socket()  — create an unbound endpoint
 *   connect() — send SYN to server IP:port; completes the 3-way handshake
 */
int init(void)
{
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
    saddr.sin_family      = AF_INET;
    saddr.sin_addr.s_addr = inet_addr(serverIP);
    saddr.sin_port        = htons(serverPORT);
    printf("Done.\n");

    printf("Connecting to server... ");
    if (connect(sock, (struct sockaddr *)&saddr, sizeof(saddr)) != 0)
    {
        printf("FAIL\n");
        exit(0);
    }

    printf("\nConnected to server OK\n");
    return 0;
}

int main(int argc, char *argv[], char *env[])
{
    int n;
    char line[MAX], response[MAX];
    char command[16], arg[64];

    init();

    while (1)
    {
        memset(command,  '\0', sizeof(command));
        memset(arg,      '\0', sizeof(arg));
        memset(line,     '\0', sizeof(line));
        memset(response, '\0', sizeof(response));

        printf("$ ");
        fgets(line, MAX, stdin);
        line[strlen(line) - 1] = 0;
        if (line[0] == 0 || !strcmp(line, "quit") || !strcmp(line, "exit"))
            exit(0);

        sscanf(line, "%s %s", command, arg);

        if (find_cmd_index(command) != -1) /* local command — run on client */
        {
            int index = find_cmd_index(command);
            fptr[index](arg);
        }
        else /* remote command — send to server */
        {
            n = write(sock, line, MAX);

            char buf[MAX];
            if (!strcmp(command, "get"))
            {
                /*
                 * get protocol: server sends size, then data chunks.
                 * We open/create the local file and write each chunk.
                 */
                int b = read(sock, buf, MAX);
                int file_size = atoi(buf);
                memset(buf, 0, sizeof(buf));

                int fd = open(arg, O_WRONLY | O_CREAT, 0644);
                if (fd >= 0) /* BUG FIX: was fd > 0; fd 0 is a valid descriptor */
                {
                    while (file_size > 0)
                    {
                        read(sock, buf, MAX);
                        if (file_size < MAX)
                        {
                            write(fd, buf, file_size);
                            file_size = 0;
                        }
                        else
                        {
                            write(fd, buf, MAX);
                            file_size -= MAX;
                        }
                    }
                    close(fd);
                }
            }
            else if (!strcmp(command, "put"))
            {
                /*
                 * put protocol: client sends size, then data chunks.
                 * Server mirrors server_put() — reads size then data.
                 */
                char buffer[MAX];
                struct stat finfo, *sp;
                sp = &finfo;
                if (lstat(arg, &finfo) < 0)
                {
                    printf("can't stat %s\n", arg);
                    continue;
                }
                int file_size = sp->st_size;
                sprintf(buffer, "%d", file_size);

                write(sock, buffer, MAX);

                int fp = open(arg, O_RDONLY);
                if (fp >= 0) /* BUG FIX: was fp > 0 */
                {
                    char fbuf[MAX];
                    memset(fbuf, '\0', sizeof(fbuf));
                    int nr = read(fp, fbuf, MAX);
                    while (nr > 0)
                    {
                        write(sock, fbuf, nr);
                        nr = read(fp, fbuf, MAX);
                    }
                    close(fp);
                }

                /* Read server response until EOT */
                bzero(response, sizeof(response));
                n = read(sock, response, sizeof(response));
                while (!is_end_of_tranmission(response))
                {
                    printf("%s", response);
                    bzero(response, sizeof(response));
                    n = read(sock, response, sizeof(response));
                }
            }
            else
            {
                /* Read server response lines until EOT */
                bzero(response, sizeof(response));
                n = read(sock, response, sizeof(response));
                while (!is_end_of_tranmission(response))
                {
                    printf("%s", response);
                    bzero(response, sizeof(response));
                    n = read(sock, response, sizeof(response));
                }
            }
        }
    }
}

/* is_end_of_tranmission: return 1 if response matches the EOT sentinel string */
int is_end_of_tranmission(char *response)
{
    if (!strcmp(response, EOT))
    {
#if DEBUG
        printf("End of transmission\n");
#endif
        return 1;
    }
    return 0;
}

/*
 * find_cmd_index: return the index of command in cmd[], or -1 if not found.
 *
 * BUG FIX: original loop was `while(cmd[i])` — no NULL sentinel.
 * Fixed to use NCMDS (computed from array size).
 */
int find_cmd_index(char *command)
{
    for (int i = 0; i < NCMDS; i++)
    {
        if (!strcmp(command, cmd[i]))
        {
#if DEBUG
            printf("Found %s, index %d\n", command, i);
#endif
            return i;
        }
    }
    return -1;
}

int menu(char *unused)
{
    puts("********************** menu ***********************");
    puts("*  get  put  ls   cd   pwd   mkdir   rmdir   rm   * ");
    puts("*  lcat     lls  lcd  lpwd  lmkdir  lrmdir  lrm   *");
    puts("***************************************************");
    return 0;
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
        fclose(fd);
    }
    else
    {
        printf("fopen failed, aborting\n");
        return 1;
    }
    return 0;
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
        return 1;
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
    return 0;
}

int lls(char *pathname)
{
    if (!strcmp(pathname, ""))
    {
        ls_dir("./");
        return 1;
    }
    ls_dir(pathname);
    return 0;
}

/*
 * ls_file: print file metadata (like `ls -l`) for a local file.
 *
 * Permission bit decoding:
 *   st_mode bits 8..0 correspond to owner-rwx, group-rwx, other-rwx.
 *   t1[] maps bit index to the character to print when the bit is set.
 *   The top 4 bits of st_mode encode the file type (regular, dir, symlink).
 */
int ls_file(char *fname)
{
    char linkname[MAX];
    char *t1 = "xwrxwrxwr-------";
    char *t2 = "----------------";

    struct stat finfo, *sp;
    int r, i;
    char ftime[64];
    sp = &finfo;

    if ((r = lstat(fname, &finfo)) < 0)
    {
        printf("can't stat %s\n", fname);
        exit(1);
    }

    if ((sp->st_mode & 0xF000) == 0x8000)      printf("%c", '-');
    else if ((sp->st_mode & 0xF000) == 0x4000)  printf("%c", 'd');
    else if ((sp->st_mode & 0xF000) == 0xA000)  printf("%c", 'l');

    for (i = 8; i >= 0; i--)
    {
        if (sp->st_mode & (1 << i))
            printf("%c", t1[i]);
        else
            printf("%c", t2[i]);
    }

    printf("%4d ", sp->st_nlink);
    printf("%4d ", sp->st_gid);
    printf("%4d ", sp->st_uid);
    printf("%8ld ", sp->st_size);

    strcpy(ftime, ctime(&sp->st_ctime));
    ftime[strlen(ftime) - 1] = 0;
    printf("%s ", ftime);

    printf("%s", basename(fname));
    if ((sp->st_mode & 0xF000) == 0xA000)
    {
        readlink(fname, linkname, MAX);
        printf(" -> %s", linkname);
    }

    printf("\n");
    return 0;
}

int lcd(char *pathname)
{
    return chdir(pathname);
}

/* BUG FIX: added char *unused to match fptr table type int (*)(char *) */
int lpwd(char *unused)
{
    char buf[MAX];
    getcwd(buf, MAX);
    printf("%s\n", buf);
    return 0;
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
