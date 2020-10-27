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
#include <libgen.h>     // for dirname()/basename()
#include <time.h> 

#define MAX   256
#define BLK  1024

int server_sock, client_sock;
char *serverIP = "127.0.0.1";      // hardcoded server IP address
int serverPORT = 1234;             // hardcoded server port number

struct sockaddr_in saddr, caddr;   // socket addr structs


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
    int n, length;
    char line[MAX];
    
    init();  

    while(1){
       printf("server: trying to accept a new connection...\n");
       length = sizeof(caddr);
       client_sock = accept(server_sock, (struct sockaddr *)&caddr, &length);
       if (client_sock < 0){
          printf("server: accept error\n");
          exit(1);
       }
 
       printf("server: accepted a client connection from\n");
       printf("-----------------------------------------------\n");
       printf("    IP=%s  port=%d\n", "127.0.0.1", ntohs(caddr.sin_port));
       printf("-----------------------------------------------\n");

       // Processing loop
       while(1){
         printf("server ready for next request ....\n");
         n = read(client_sock, line, MAX);
         if (n==0){
           printf("server: client died, server loops\n");
           close(client_sock);
           break;
         }
         line[n]=0;
         printf("server: read  n=%d bytes; line=[%s]\n", n, line);

         strcat(line, " ECHO");
         // send the echo line to client 
         n = write(client_sock, line, MAX);

         printf("server: wrote n=%d bytes; ECHO=[%s]\n", n, line);
       }
    }
}