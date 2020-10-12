/*******************************************************
 * CS360 Lab5 Header File, lab5.h
 * Connor Easton, Zach Nett
********************************************************/

#ifndef __LAB_5_H__
#define __LAB_5_H__

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

// defines
#define MAX 256
#define PORT 1234
#define BLK 1024

// globals
struct sockaddr_in saddr; 
int sock, r, n;
char line[MAX], ans[MAX];

#endif /* __LAB_5_H__ */