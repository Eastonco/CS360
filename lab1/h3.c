/*********** h3.c ***********/
// Assume disk sector 1234 contains a PERSON struct at byte offset 256

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>

typedef struct person{
  char name[64];
  int  id;
  int  age;
  char gender;
}PERSON;

int read_sector(int fd, int sector, char *buf)
{
  int n;
  lseek(fd, sector*512, SEEK_SET);
  n = read(fd, buf, 512);
  if (n <= 0){
    printf("read failed\n");
    return -1;
  }
  return n;
}

PERSON kcw, *p;

int fd;
char buf[512];

int main()
{
  fd = open("disk", O_RDONLY);  // open disk for READ
  printf("fd = %d\n", fd);

  read_sector(fd, 1234, buf);   // READ sector 1234 into buf[ ]

  p = (PERSON *)(buf+256);      // OR p=(PERSON *)&buf[256];
  
  printf("name=%s id=%d age=%d gender=%c\n",
	 p->name, p->id, p->age, p->gender);

}
