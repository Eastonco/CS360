/********* h1.c **********/ 
#include <stdio.h>
#include <string.h>

typedef struct person{
  char name[64];
  int  id;
  int  age;
  char gender;
}PERSON;

PERSON kcw, *p;

int main()
{

// Access struct fields by . operator: OK but ugly
   kcw.id = 12345678;
   kcw.age = 83;
   kcw.gender = 'M';
  
   p = &kcw;

// Deference pointer to struct, then use . operator: NOT GOOD either!
   (*p).id = 123;
   (*p).age = 120;

// Use pointer by -> operator is the BEST WAY:
   p->id = 12345678;
   p->age = 83;
   p->gender = 'M';
   strcpy(p->name, "k.c. wang");

   printf("name=%s id=%d age=%d gender=%c\n",
	 p->name, p->id, p->age, p->gender);
}


// creat a disk file of 2880 512-byte sectors OR 1440 1K blocks
   dd if=/dev/zero of=disk bs=512 count=2880