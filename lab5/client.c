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

struct sockaddr_in saddr;
char *serverIP = "127.0.0.1";
int serverPORT = 1234;
int sock;

// Function prototypes
int find_cmd_index(char *command);
int lcat(char *filename);
int lls();
int lcd(char *pathname);
int lpwd();
int lmkdir(char *pathname);
int lrmdir(char *pathname);
int lrm(char *pathname);

// Command table (for function pointers)
char *cmd[] = {"lcat", "lls", "lcd", "lpwd", "lmkdir", "lrmdir", "lrm"};

int (*fptr[])(char *) = { (int (*)())lcat, lls, lcd, lpwd, lmkdir, lrmdir, lrm };

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
    char line[MAX], ans[MAX];
    char command[16], arg[64];

    init();

    while (1)
    {
        printf("input a line : ");
        fgets(line, MAX, stdin);
        line[strlen(line) - 1] = 0; // kill <CR> at end
        if (line[0] == 0)           // exit if NULL line
            exit(0);

        // CHeck here if the command should be executed locally
        if (line[0] == 'l') { // local command-- run on client only
            sscanf(line, "%s %s", command, arg);
            int index = find_cmd_index(command);
            if (index != -1) {
                fptr[index](arg);
            } else {
                printf("invalid command %s\n", line);
            }
        } else { // else send to server
            // Send ENTIRE line to server
            n = write(sock, line, MAX);
            printf("client: wrote n=%d bytes; line=(%s)\n", n, line);

            // Read a line from sock and show it
            bzero(ans, MAX);
            n = read(sock, ans, MAX);
            printf("client: read  n=%d bytes; echo=(%s)\n", n, ans);
        } 
    }
}

int find_cmd_index(char *command) {
    int i = 0;
    while (cmd[i]) {
        if (!strcmp(command, cmd[i])) {
            printf("Found %s, index %d\n", command, i);
            return i;
        }
        i++;
    }
    return -1;
}

int lcat(char *filename) {
    char buf[512];
    FILE *fd = fopen(filename, "r");
    if (fd != NULL) {
        while (!feof(fd)) {
            fgets(buf, 512, fd);
            puts(buf);
        }
    } else {
        printf("fopen failed, aborting\n");
    }
    return 1;
}

int lls() {
    // to be written
}

int lcd(char *pathname) {
    return chdir(pathname);
}

int lpwd() {
    char buf[MAX];
    getcwd(buf, MAX);
    printf("%s\n", buf);
}

int lmkdir(char *pathname) {
    return mkdir(pathname, 0755);
}

int lrmdir(char *pathname) {
    return rmdir(pathname);
} 

int lrm(char *pathname) {
    return unlink(pathname);
}