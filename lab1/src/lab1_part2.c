#include <stdio.h>
typedef unsigned int u32;

void myprintf(char *format, ...);
void prints(char *s); /* print string */
void rpu(u32 x);      /* recursive print unsigned */
void printu(u32 x);   /* print unsigned */
void rpd(int x);      /* recursive print decimal (signed) */
void printd(int x);   /* print decimal */
void rpx(u32 x);      /* recursive print hex */
void printx(u32 x);   /* print hex */
void rpo(u32 x);      /* recursive print octal */
void printo(u32 x);   /* print octal */

char *ctable = "0123456789ABCDEF"; /* digit character table — works for bases up to 16 */
int BASE = 10;                     /* default base for rpu/rpd */

int main(int argc, char *argv[], char *env[])
{
	myprintf("char = %c\nstring = %s\ndec = %d\nhex = %x\noct = %o\nneg = %d\n",
			 'A', "this is a test", 100, 100, 100, -100);
	myprintf("argc = %d", argc);

	for (int i = 0; i < argc; i++)
	{
		myprintf("argv[ %d] = %s", i, argv[i]);
	}

	printf("\n");

	for (int i = 0; env[i] != 0; i++)
	{
		myprintf("env[ %d] = %s", i, env[i]);
	}

	return 0;
}

/*
 * myprintf: a manual implementation of printf (subset: %c %s %u %d %o %x).
 *
 * IMPORTANT: This ONLY works compiled with -m32 (32-bit x86).
 *
 * How manual varargs works here (the "stack hack"):
 *   In 32-bit x86 calling convention, ALL arguments (including variadic ones)
 *   are pushed onto the stack in right-to-left order before the CALL instruction.
 *   So in memory, the stack looks like:
 *
 *       [ format ]  <- &format (what we take the address of)
 *       [ arg1   ]  <- (int *)&format + 1
 *       [ arg2   ]  <- (int *)&format + 2
 *       ...
 *
 *   By treating &format as an int* and adding 1, we step past the format
 *   string pointer to the first variadic argument, then walk ip++ for each.
 *
 *   In 64-bit mode, the first 6 arguments go in registers (not on the stack),
 *   so this trick fails — which is why we must use -m32.
 *
 *   The standard library solves this portably with <stdarg.h> / va_list.
 */
void myprintf(char *format, ...)
{
	char *cp = format;            /* walk through the format string character by character */
	int *ip = (int *)&format + 1; /* ip points just past 'format' on the stack = first vararg */

	while (*cp != '\0')
	{
		if (*cp == '%')
		{
			cp++; /* move past '%' to the type specifier */

			switch (*cp)
			{
			case 'c':
				putchar(*ip); /* print character (int promoted to char) */
				ip++;
				break;
			case 's':
				prints((char *)*ip); /* *ip is the pointer to the string */
				ip++;
				break;
			case 'u':
				printu(*ip);
				ip++;
				break;
			case 'd':
				printd(*ip);
				ip++;
				break;
			case 'o':
				printo(*ip);
				ip++;
				break;
			case 'x':
				printx(*ip);
				ip++;
				break;
			default:
				printf("ERROR");
				break;
			}
			cp++;
		}
		else
		{
			putchar(*cp);
			cp++;
		}
	}

	putchar('\n');
}

void prints(char *s)
{
	for (int i = 0; s[i] != '\0'; i++)
	{
		putchar(s[i]);
	}
}

/*
 * Recursive digit printing: rpu/rpd/rpx/rpo all use the same trick.
 *
 * To print a number digit by digit (most significant first), we can't
 * simply extract digits left-to-right — we only know the least significant
 * digit (x % base) at each step. The solution: recurse first, then print.
 *
 *   rpu(123):
 *     rpu(12) — recurse
 *       rpu(1) — recurse
 *         rpu(0) — base case, returns immediately
 *         putchar('1')
 *       putchar('2')
 *     putchar('3')
 *
 * The recursion naturally reverses the digit order so output is correct.
 */
void rpu(u32 x)
{
	char c;
	if (x)
	{
		c = ctable[x % BASE]; /* extract least-significant digit */
		rpu(x / BASE);        /* recurse on remaining digits */
		putchar(c);           /* print THIS digit after the recursion returns */
	}
}

void printu(u32 x)
{
	if (x == 0)
		putchar('0');
	else
		rpu(x);
	putchar(' ');
}

void rpd(int x)
{
	char c;
	if (x)
	{
		c = ctable[x % BASE];
		rpd(x / BASE);
		putchar(c);
	}
}

void printd(int x)
{
	if (x == 0)
	{
		putchar('0');
		putchar(' ');
		return;
	}
	if (x < 0)
	{
		putchar('-');
		rpd(x * -1); /* negate before passing to rpd (rpd handles positive only) */
		putchar(' ');
	}
	else
	{
		rpd(x);
		putchar(' ');
	}
}

/* rpx: same recursive technique but always uses base 16 */
void rpx(u32 x)
{
	char c;
	if (x)
	{
		c = ctable[x % 16];
		rpx(x / 16);
		putchar(c);
	}
}

void printx(u32 x)
{
	putchar('0');
	putchar('x'); /* hex values conventionally prefixed with 0x */
	if (x == 0)
	{
		putchar('0');
		return;
	}
	rpx(x);
	putchar(' ');
}

/* rpo: same recursive technique but always uses base 8 */
void rpo(u32 x)
{
	char c;
	if (x)
	{
		c = ctable[x % 8];
		rpo(x / 8);
		putchar(c);
	}
}

void printo(u32 x)
{
	putchar('0'); /* octal values conventionally prefixed with 0 */
	if (x == 0)
	{
		putchar('0');
		return;
	}
	rpo(x);
	putchar(' ');
}
