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

#define MAX 256
#define BLK 1024

struct sockaddr_in saddr; 
char *serverIP   = "127.0.0.1";
int   serverPORT = 1234;
int   sock;

int init()
{
    int n; 

    printf("1. create a socket\n");
    sock = socket(AF_INET, SOCK_STREAM, 0); 
    if (sock < 0) { 
        printf("socket creation failed\n"); 
        exit(0); 
    }
    
    printf("2. fill in server IP=%s, port number=%d\n", serverIP, serverPORT);
    bzero(&saddr, sizeof(saddr)); 
    saddr.sin_family = AF_INET; 
    saddr.sin_addr.s_addr = inet_addr(serverIP); 
    saddr.sin_port = htons(serverPORT); 
  
    printf("3. connect to server\n");
    if (connect(sock, (struct sockaddr *)&saddr, sizeof(saddr)) != 0) { 
        printf("connection with the server failed...\n"); 
        exit(0); 
    }
    printf("4. connected to server OK\n");
}
  
int main(int argc, char *argv[], char *env[]) 
{ 
    int  n;
    char line[MAX], ans[MAX];

    init();
  
    while (1){
      printf("input a line : ");
      fgets(line, MAX, stdin);
      line[strlen(line)-1] = 0;        // kill <CR> at end
      if (line[0]==0)                  // exit if NULL line
         exit(0);

      // Send ENTIRE line to server
      n = write(sock, line, MAX);
      printf("client: wrote n=%d bytes; line=(%s)\n", n, line);

      // Read a line from sock and show it
      bzero(ans, MAX);
      n = read(sock, ans, MAX);
      printf("client: read  n=%d bytes; echo=(%s)\n",n, ans);
    }
}

