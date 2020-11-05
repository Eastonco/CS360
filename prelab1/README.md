# CS360 Pre-LAB1 Assignment

- DUE: 9-1-2020
- email YOUR output files with ID, NAME to TA

## Table Of Contents

- [CS360 Pre-LAB1 Assignment](#cs360-pre-lab1-assignment)
  - [Table Of Contents](#table-of-contents)
  - [Intro](#intro)
  - [Part 1:](#part-1)
    - [Part A](#part-a)
      - [Answer These Questions:](#answer-these-questions)
    - [Part B](#part-b)
      - [Answer These Questions:](#answer-these-questions-1)
  - [Part 2](#part-2)
    - [Do the requirements 1 to 7 as specified below:](#do-the-requirements-1-to-7-as-specified-below)

## Intro

To compile C programs into 32-bit code in 64-bit Ubuntu Linux:

```bash
sudo apt-get install gcc-multilib
```

to get and install

```bash
gcc-multilib
```

## Part 1:

A binary executable file, `a.out`, consists of

```
| header | Code | Data | <-BSS-> |
```

where BSS is for uninitialized globals and uninitialized static locals. The Unix command

```bash
size a.out
```

shows the size of `TEXT`, `DATA`, `BSS` of `a.out`. Use the following C program, `t1.c`, to generate `t2.c`, `t3.c`,..., `t6.c` as specified
below.

```c
//********** t1.c file ************
#include <stdio.h>
int g;
main()
{
   int a,b,c;
   a = 1; b = 2;
   c = a + b;
   printf("c=%d\n", c);
}
```

`t2.c`: Change the global variable `g` to `int g=3;`
`t3.c`: Change the global variable `g` to `int g[10000];`
`t4.c`: Change the global variable `g` to ` int g[10000] = {4};`
`t5.c`: Change the local variables `of main()` to

```c
int a,b,c, d[10000];
```

`t6.c`. Change the local variables of main() to

```c
static int a,b,c, d[10000];
```

### Part A

For each case, use

```bash
cc -m32 t<number>.c
```

to generate `a.out`. Then, use

```bash
ls -l a.out
```

to get `a.out` size and run

```bash
size a.out
```

to get its section sizes. After that, record the observed sizes in a table:

| Case   | a.out | TEXT | DATA | BSS |
| :----- | :---- | :--- | :--- | :-- |
| `t1.c` |       |      |      |     |
| `t2.c` |       |      |      |     |
| `t3.c` |       |      |      |     |
| `t4.c` |       |      |      |     |
| `t5.c` |       |      |      |     |
| `t6.c` |       |      |      |     |

---

#### Answer These Questions:

1.  Variables in C may be classified as

```
globals ---|--- UNINITIALIZED  globals;
           |--- INITIALIZED    globals;
locals  ---|--- AUTOMATIC locals;
           |--- STATIC    locals;
```

2. In terms of the above classification and the variables `g`, `a`, `b`, `c`, `d`,

- Which variables are in DATA?
- Which variables are in BSS ?

1. In terms of the `TEXT`, `DATA` and `BSS` sections,

- Which sections are in a.out, which section is NOT in a.out?
- Why?

### Part B

For each case, use

```bash
cc -m32 -static t.c
```

to generate `a.out`.

#### Answer These Questions:

- Record the sizes again and compare them with the sizes in (A).
- What do you see?
- Why?

## Part 2

Given the following `t.c` and `ts.s` files
Under Linux, use

```bash
gcc -m32 t.c ts.s
```

to generate an `a.out`. Then, run a.out with

```bash
a.out one two three > outfile
```

### Do the requirements 1 to 7 as specified below:

```
# ts.s file:
       .global getebp
getebp:
        movl %ebp, %eax
        ret
```

```c
/************* t.c file ********************/
#include <stdio.h>
#include <stdlib.h>

int *FP;

int main(int argc, char *argv[], char *env[])
{
  int a, b, c;
  printf("enter main\n");

  printf("&argc=%x argv=%x env=%x\n", &argc, argv, env);
  printf("&a=%8x &b=%8x &c=%8x\n", &a, &b, &c);

  // (1). Write C code to print values of argc and argv[] entries

  a = 1;
  b = 2;
  c = 3;
  A(a, b);
  printf("exit main\n");
}

int A(int x, int y)
{
  int d, e, f;
  printf("enter A\n");
  // (2). write C code to PRINT ADDRESS OF d, e, f
  d = 4;
  e = 5;
  f = 6;
  B(d, e);
  printf("exit A\n");
}

int B(int x, int y)
{
  int g, h, i;
  printf("enter B\n");
  // (3). write C code to PRINT ADDRESS OF g,h,i
  g = 7;
  h = 8;
  i = 9;
  C(g, h);
  printf("exit B\n");
}

int C(int x, int y)
{
  int u, v, w, i, *p;

  printf("enter C\n");
  // (4).write C code to PRINT ADDRESS OF u,v,w,i,p;
  u = 10;
  v = 11;
  w = 12;
  i = 13;

  FP = (int *)getebp();  // FP = stack frame pointer of the C()
                         //      function

  // (5). Write C code to print the stack frame link list.

  p = (int *)&p;

  // (6). Print the stack contents from p to the frame of main()
  // YOU MAY JUST PRINT 128 entries of the stack contents.

  // (7). On a hard copy of the print out, identify the stack contents
  //      as LOCAL VARIABLES, PARAMETERS, stack frame pointer of each function.
}
```
