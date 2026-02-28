// Under Linux, use    gcc -m32 t.c ts.s  to generate an a.out
// The -m32 flag compiles to 32-bit x86 code, where stack frames use EBP/ESP
// and integers are 4 bytes, making stack layout inspection straightforward.

/************* t.c file ********************/
#include <stdio.h>
#include <stdlib.h>

// Global pointer to hold the EBP (frame pointer) captured inside C()
// This lets us traverse the linked list of stack frames from any function.
int *FP;

// Forward declarations: required because A, B, C, and getebp are called
// before they are defined. Without these, the compiler (in C89 mode) would
// assume each returns int — which is wrong for getebp returning a pointer.
int A(int x, int y);
int B(int x, int y);
int C(int x, int y);
int getebp();   // defined in ts.s — returns the current EBP register value

int main(int argc, char *argv[], char *env[])
{
  int a, b, c;
  printf("enter main\n");

  // Print the addresses of the parameters and locals to see how the
  // compiler lays them out on the stack. Parameters are above the saved
  // return address; locals are below EBP in the current frame.
  printf("&argc=%x argv=%x env=%x\n", &argc, argv, env);
  printf("&a=%8x &b=%8x &c=%8x\n", &a, &b, &c);

  //(1). Write C code to print values of argc and argv[] entries
  printf("argc=%x\n", argc);

  for (int i = 0; i < argc; i++)
  {
    printf("argv[%d]=%s\n", i, argv[i]);
  }

  a = 1;
  b = 2;
  c = 3;
  A(a, b);
  printf("exit main\n");
}

// Each function call pushes: arguments, return address, then saved EBP.
// EBP then points to the saved EBP of the caller, forming a linked list
// of stack frames that can be walked all the way back to main().
int A(int x, int y)
{
  int d, e, f;
  printf("enter A\n");
  // write C code to PRINT ADDRESS OF d, e, f
  // Note how d, e, f are at lower addresses than A's parameters (x, y)
  printf("d=%x e=%x f=%x\n", &d, &e, &f);

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
  // write C code to PRINT ADDRESS OF g,h,i
  printf("g=%x h=%x i=%x\n", &g, &h, &i);

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
  // write C code to PRINT ADDRESS OF u,v,w,i,p;
  printf("u=%x v=%x w=%x i=%x p=%x\n\n", &u, &v, &w, &i, &p);

  u = 10;
  v = 11;
  w = 12;
  i = 13;

  // getebp() (defined in ts.s) executes: movl %ebp, %eax; ret
  // It returns the value of the EBP register at the moment C() is running.
  // EBP points to the saved EBP of B() — the start of a linked list:
  //   C's frame -> B's frame -> A's frame -> main's frame -> NULL
  FP = (int *)getebp(); // FP = stack frame pointer of the C() function
  printf("FP = ebp = %x\n", FP);

  getchar();

  //(2). Write C code to print the stack frame link list.
  // Each frame's first word is the saved EBP of the caller.
  // Dereferencing p walks us up through every function's frame.
  p = (int *)FP;
  printf("stack frame link list:\n");
  while (p)
  {
    printf("%x -> ", p);
    p = (int *)*p;  // *p holds the saved EBP of the calling frame (cast int->int*)
  }
  printf("NULL\n");

  getchar();

  //(3). Print the stack contents from p to the frame of main()
  //     YOU MAY JUST PRINT 128 entries of the stack contents.

  // Starting from &u (bottom of C's locals), print 128 words upward.
  // You'll be able to spot: locals, saved EBP, return address, parameters
  // for each nested call frame as you move to higher addresses.
  printf("\n128 stack contents\n");

  p = (int *)&u;
  for (int i = 0; i < 128; i++)
  {
    printf("%x\t%x\n", p, *p);
    p++;
  }

  //(4). On a hard copy of the print out, identify the stack contents
  //     as LOCAL VARIABLES, PARAMETERS, stack frame pointer of each function.
}
