/*******************************************************
 * CS360 Lab5 Client File, client.c
 * Connor Easton, Zach Nett
********************************************************/
#include "lab5.h"

int init()
{

    int n;

    printf("Creating socket... ");
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        printf("ERROR CREATING SOCKET");
        exit(0);
    }
    printf("Done.\n");

    printf("Filling server IP=%s, port number=%s... ", serverIP, serverPORT);
    bzero(&saddr, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = inet_addr(serverIP);
    saddr.sin_port = htons(serverPORT);
    printf("Done.\n");

    printf("Connecting to server...");
    if (connect(sock, (struct sockaddr *)&saddr, sizeof(saddr)) != 0)
    {
        {
            printf("FAILED\n");
            exit(0);
        };
        printf("Done.\n");
        printf("Connection to server OK\n\n");
    }
}

int main(int argc, char *argv[], char *env[])
{

    int n;
    char line[MAX], ans[MAX];

    init();

    while (1)
    {
        printf("input a line : ");
        fgets(line, MAX, stdin);
        line[strlen(line) - 1] = 0; // kill <CR> at end
        if (line[0] == 0)           // exit if NULL line
            exit(0);

        // Send ENTIRE line to server
        n = write(sock, line, MAX);
        printf("client: wrote n=%d bytes; line=(%s)\n", n, line);

        // Read a line from sock and show it
        bzero(ans, MAX);
        n = read(sock, ans, MAX);
        printf("client: read  n=%d bytes; echo=(%s)\n", n, ans);
    }
}