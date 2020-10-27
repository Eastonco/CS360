/*******************************************************
 * CS360 Lab5 Server File, server.c
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

int server_sock, client_sock;
char *serverIP = "127.0.0.1"; // hardcoded server IP address
int serverPORT = 1234;        // hardcoded server port number

struct sockaddr_in saddr, caddr; // socket addr structs

int find_cmd_index(char *command);
int has_argument(char *line);
int server_get();
int server_put();
int server_ls();
int server_cd(char *pathname);
int server_pwd();
int server_mkdir();
int server_rmdir();
int server_rm();

// Command table (for function pointers)
char *cmd[] = {"get", "put", "ls", "cd", "pwd", "mkdir", "rmdir", "rm"};

int (*fptr[])(char *) = {(int (*)())server_get, server_put, server_ls, server_cd, server_pwd, server_mkdir, server_rmdir, server_rm};

char data[MAX];

int init()
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
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = inet_addr(serverIP);
    saddr.sin_port = htons(serverPORT);
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
}

int main(int argc, char *argv[], char *env[])
{
    // set virtual root to CWD
    chdir("/");
    chroot("/");

    int n, length;
    char line[MAX];
    char command[16], arg[64];

    init();

    while (1)
    {
        printf("server: trying to accept a new connection...\n");
        length = sizeof(caddr);
        client_sock = accept(server_sock, (struct sockaddr *)&caddr, &length);
        if (client_sock < 0)
        {
            printf("server: accept error\n");
            exit(1);
        }

        printf("server: accepted a client connection from\n");
        printf("-----------------------------------------------\n");
        printf("    IP=%s  port=%d\n", "127.0.0.1", ntohs(caddr.sin_port));
        printf("-----------------------------------------------\n");

        // Processing loop
        while (1)
        {
            printf("server ready for next request ....\n");
            n = read(client_sock, line, MAX);
            if (n == 0)
            {
                printf("server: client died, server loops\n");
                close(client_sock);
                break;
            }
            line[n] = 0;
            printf("server: read  n=%d bytes; line=[%s]\n", n, line);

            memset(data, 0, MAX);

            if(has_argument(line)) {
                sscanf(line, "%s %s", command, arg);
            } else {
                strcpy(command, line);
                strcpy(arg, "");
            }
            
            int index = find_cmd_index(command);
            if (index != -1)
            {
                int r = fptr[index](arg);
                strcat(data, line);
                if (r != -1) {
                    strcat(data, " OK");
                } else {
                    strcat(data, " FAILED");
                }
            }
            else
            {
                printf("invalid command %s\n", line);
            }

            n = write(client_sock, data, MAX);

            printf("server: wrote n=%d bytes; ECHO=[%s]\n", n, data);

        }
    }
}

int has_argument(char *line) {
    int size = strlen(line);
    for (int i = 0; i < size; i++) {
        if (line[i] == ' ') {
            if (line[i+1] != ' ' || line[i+1] != '\0') {
                return 1;
            }
        }
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
            printf("Found %s, index %d\n", command, i);
            return i;
        }
        i++;
    }
    return -1;
}

int server_get()
{
    return 0;
}

int server_put()
{
    return 0;
}

int server_ls()
{
    return 0;
}

int server_cd(char *pathname)
{
    return chdir(pathname);
}

int server_pwd()
{
    char buf[MAX];
    getcwd(buf, MAX);
    printf("SERVER LOCAL: %s\n", buf);
    strcpy(data, buf);
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