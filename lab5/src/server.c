/*******************************************************
 * CS360 Lab5 Server File, server.c
 * Connor Easton, Zach Nett
 *
 * TCP socket lifecycle:
 *   socket()  — create an endpoint (fd)
 *   bind()    — attach to an IP:port address
 *   listen()  — mark socket as passive (accept incoming connections)
 *   accept()  — block until a client connects; returns a new fd for that client
 *   read()/write() — send/receive data on the client fd
 *   close()   — tear down the connection
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

int server_sock, client_sock;
char *serverIP   = "127.0.0.1";
int   serverPORT = 1234;

struct sockaddr_in saddr, caddr;

/* Command table: server-side commands */
char *cmd[] = {"get", "put", "ls", "cd", "pwd", "mkdir", "rmdir", "rm"};
#define NCMDS ((int)(sizeof(cmd) / sizeof(cmd[0])))

int find_cmd_index(char *command);
int has_argument(char *line);
int server_get(char *filename);
int server_put(char *filename);
int server_ls(char *pathname);
int ls_dir(char *pathname);
int ls_file(char *fname);
int server_cd(char *pathname);
int server_pwd(char *unused);
int server_mkdir(char *pathname);
int server_rmdir(char *pathname);
int server_rm(char *pathname);

/*
 * Function pointer table: maps command index to handler.
 * All handlers take (char *) so the table is type-consistent.
 * BUG FIX: original cast `(int (*)())server_get` mismatched return type;
 * all functions now properly declared as int (*)(char *).
 */
int (*fptr[])(char *) = {
    server_get, server_put, server_ls, server_cd,
    server_pwd, server_mkdir, server_rmdir, server_rm
};

char data[MAX];

/*
 * init: set up the server socket.
 *
 * The TCP server lifecycle:
 *   1. socket()  — create an unbound TCP endpoint
 *   2. bind()    — bind to IP:port so clients know where to connect
 *   3. listen()  — queue up to 5 pending connections
 *   4. (loop) accept() — block until a client connects
 */
int init(void)
{
    printf("Creating socket... ");
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0)
    {
        printf("ERROR\n");
        exit(0);
    }
    printf("Done.\n");

    printf("Filling server IP and port number... ");
    bzero(&saddr, sizeof(saddr));
    saddr.sin_family      = AF_INET;
    saddr.sin_addr.s_addr = inet_addr(serverIP);
    saddr.sin_port        = htons(serverPORT);
    printf("Done.\n");

    printf("Binding socket to server... ");
    if ((bind(server_sock, (struct sockaddr *)&saddr, sizeof(saddr))) != 0)
    {
        printf("ERROR\n");
        exit(0);
    }
    printf("Server listening with queue size = 5... ");
    if ((listen(server_sock, 5)) != 0)
    {
        printf("ERROR\n");
        exit(0);
    }
    printf("Done.\n\n");

    printf("Server at IP=%s port=%d\n", serverIP, serverPORT);
    return 0;
}

int main(int argc, char *argv[], char *env[])
{
    chdir("./");
    chroot("./");

    int n, length;
    char line[MAX];
    char command[16], arg[64];

    init();

    while (1)
    {
        printf("server: trying to accept a new connection...\n");
        length = sizeof(caddr);
        client_sock = accept(server_sock, (struct sockaddr *)&caddr, (socklen_t *)&length);
        if (client_sock < 0)
        {
            printf("server: accept error\n");
            exit(1);
        }

        printf("server: accepted a client connection from\n");
        printf("-----------------------------------------------\n");
        printf("    IP=%s  port=%d\n", "127.0.0.1", ntohs(caddr.sin_port));
        printf("-----------------------------------------------\n");

        while (1)
        {
            memset(command, '\0', sizeof(command));
            memset(arg,     '\0', sizeof(arg));
            memset(line,    '\0', sizeof(line));
            printf("server ready for next request ....\n");

            n = read(client_sock, line, MAX);
            if (n <= 0)
            {
                printf("server: client died, server loops\n");
                close(client_sock);
                break;
            }
            line[n] = 0;
            printf("server: read  n=%d bytes; line=[%s]\n", n, line);

            memset(data, 0, MAX);

            if (has_argument(line))
            {
                sscanf(line, "%s %s", command, arg);
            }
            else
            {
                strcpy(command, line);
                strcpy(arg, "");
            }

            int index = find_cmd_index(command);
            if (index != -1)
            {
                int r = fptr[index](arg);
                strcat(data, line);
                if (r != -1)
                    strcat(data, " OK\n");
                else
                    strcat(data, " FAILED\n");
            }
            else
            {
                printf("invalid command %s\n", line);
            }

#if DEBUG
            n = write(client_sock, data, MAX);
            printf("server: wrote n=%d bytes; ECHO=[%s]\n", n, data);
#endif

            /* Signal end of transmission to client */
            n = write(client_sock, EOT, MAX);
            printf("server: sent EOT to client\n");
        }
    }
}

/*
 * has_argument: return 1 if the command line contains a space followed by
 * a non-space, non-null character (i.e., an argument follows the command).
 *
 * BUG FIX: original condition was `||` (always true since a char can't be
 * both ' ' AND '\0' simultaneously). The correct logic is `&&`:
 * both conditions must hold for there to be a real argument.
 */
int has_argument(char *line)
{
    int size = strlen(line);
    for (int i = 0; i < size; i++)
    {
        if (line[i] == ' ')
        {
            if (line[i + 1] != ' ' && line[i + 1] != '\0') /* BUG FIX: || → && */
            {
                return 1;
            }
        }
    }
    return 0;
}

/*
 * find_cmd_index: return the index of command in cmd[], or -1 if not found.
 *
 * BUG FIX: original loop was `while(cmd[i])` — but cmd[] has no NULL sentinel
 * (it's a fixed-size string literal array). Walking past the end is UB.
 * Fix: use a counted loop with NCMDS.
 */
int find_cmd_index(char *command)
{
    for (int i = 0; i < NCMDS; i++)
    {
        if (!strcmp(command, cmd[i]))
        {
            printf("Found %s, index %d\n", command, i);
            return i;
        }
    }
    return -1;
}

/*
 * server_get: send a file to the client.
 *
 * Protocol (size-prefix + chunked data):
 *   1. Server sends file size as ASCII string
 *   2. Server sends file contents in MAX-byte chunks
 * Client reads the size first, then reads exactly that many bytes.
 */
int server_get(char *filename)
{
    int r;
    char buffer[MAX];

    struct stat finfo, *sp;
    sp = &finfo;
    if ((r = lstat(filename, &finfo)) < 0)
    {
        printf("can't stat %s\n", filename);
        return -1;
    }
    int file_size = sp->st_size;
    sprintf(buffer, "%d", file_size);

    write(client_sock, buffer, MAX);

    int fp = open(filename, O_RDONLY);
    if (fp >= 0) /* BUG FIX: was fp > 0; fd 0 is a valid file descriptor */
    {
        char buf[MAX];
        memset(buf, '\0', sizeof(buf));
        int n = read(fp, buf, MAX);
        while (n > 0)
        {
            write(client_sock, buf, n);
            n = read(fp, buf, MAX);
        }
        close(fp);
    }
    return 0;
}

/*
 * server_put: receive a file from the client.
 *
 * Protocol mirrors server_get (client sends size, then data chunks).
 */
int server_put(char *filename)
{
    char buf[MAX];
    int b = read(client_sock, buf, MAX);
    int file_size = atoi(buf);
    memset(buf, 0, sizeof(buf));

    int fd = open(filename, O_WRONLY | O_CREAT, 0644);
    if (fd >= 0) /* BUG FIX: was fd > 0 */
    {
        while (file_size > 0)
        {
            read(client_sock, buf, MAX);
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
    return 0;
}

int server_ls(char *pathname)
{
    char buf[MAX];
    getcwd(buf, MAX - 1);
    if (!strcmp(pathname, ""))
    {
        ls_dir(buf);
        return 1;
    }
    ls_dir(pathname);
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

/*
 * ls_file: stat a file and send formatted metadata to the client.
 *
 * Permission bit decoding:
 *   st_mode is a bitmask. The top 4 bits encode file type (regular, dir, symlink).
 *   The bottom 9 bits are rwx permissions for owner/group/other.
 *   We index into t1[] ("xwrxwrxwr") to map each bit to its letter.
 *   Note: t1 is indexed 8..0 (bit 8 = owner-read, bit 7 = owner-write, etc.)
 *   which corresponds to the conventional display order.
 *
 * BUG FIX: buff[] was uninitialized — strcat scans for '\0' terminator which
 * may not exist in random stack memory, causing out-of-bounds writes.
 */
int ls_file(char *fname)
{
    int n;
    char buff[MAX];
    char fmt[MAX];
    char linkname[MAX];
    char *t1 = "xwrxwrxwr-------";
    char *t2 = "----------------";

    struct stat finfo, *sp;
    int r, i;
    char ftime[64];
    sp = &finfo;

    buff[0] = '\0'; /* BUG FIX: initialize before strcat */

    if ((r = lstat(fname, &finfo)) < 0)
    {
        sprintf(fmt, "can't stat %s\n", fname);
        strcpy(buff, fmt);
        n = write(client_sock, buff, MAX);
        printf("server: wrote n=%d bytes; ECHO=[%s]\n", n, buff);
        exit(1);
    }

    /* File type character (-, d, or l) */
    if ((sp->st_mode & 0xF000) == 0x8000) sprintf(fmt, "%c", '-');
    else if ((sp->st_mode & 0xF000) == 0x4000) sprintf(fmt, "%c", 'd');
    else if ((sp->st_mode & 0xF000) == 0xA000) sprintf(fmt, "%c", 'l');
    else sprintf(fmt, "%c", '?');
    strcat(buff, fmt);

    /* Permission bits: iterate bit 8 down to 0 */
    for (i = 8; i >= 0; i--)
    {
        if (sp->st_mode & (1 << i))
            sprintf(fmt, "%c", t1[i]);
        else
            sprintf(fmt, "%c", t2[i]);
        strcat(buff, fmt);
    }

    sprintf(fmt, "%4d ",  sp->st_nlink); strcat(buff, fmt); /* link count */
    sprintf(fmt, "%4d ",  sp->st_gid);   strcat(buff, fmt); /* group id   */
    sprintf(fmt, "%4d ",  sp->st_uid);   strcat(buff, fmt); /* user id    */
    sprintf(fmt, "%8ld ", sp->st_size);  strcat(buff, fmt); /* file size  */

    strcpy(ftime, ctime(&sp->st_ctime));
    ftime[strlen(ftime) - 1] = 0; /* strip trailing \n from ctime output */
    sprintf(fmt, "%s ", ftime);
    strcat(buff, fmt);

    sprintf(fmt, "%s", basename(fname));
    strcat(buff, fmt);

    /* For symbolic links, also show the target */
    if ((sp->st_mode & 0xF000) == 0xA000)
    {
        readlink(fname, linkname, MAX);
        sprintf(fmt, " -> %s", linkname);
        strcat(buff, fmt);
    }

    strcat(buff, "\n");
    n = write(client_sock, buff, MAX);
    printf("server: wrote n=%d bytes; ECHO=[%s]\n", n, buff);
    memset(buff, 0, MAX);
    return 0;
}

int server_cd(char *pathname)
{
    return chdir(pathname);
}

/* BUG FIX: added char *unused parameter to match function pointer table type */
int server_pwd(char *unused)
{
    char buf[MAX];
    getcwd(buf, MAX - 1);
    strcat(buf, "\n");
    printf("SERVER LOCAL: %s\n", buf);
    write(client_sock, buf, MAX);
    return 0;
}

int server_mkdir(char *pathname)
{
    return mkdir(pathname, 0755);
}

int server_rmdir(char *pathname)
{
    return rmdir(pathname);
}

int server_rm(char *pathname)
{
    return unlink(pathname);
}
