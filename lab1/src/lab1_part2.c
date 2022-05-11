#include <stdio.h>
typedef unsigned int u32;

void myprintf(char *format, ...);
void prints(char *s); /* print string*/
void rpu(u32 x);	  /* recursive print unsigned */
void printu(u32 x);	  /* print unsigned */
void rpd(int x);	  /* recursive print decimal */
void printd(int x);	  /* print decimal */
void rpx(u32 x);	  /* recursive print hex */
void printx(u32 x);	  /* print hex */
void rpo(u32 x);	  /* recursive print octal */
void printo(u32 x);	  /* print octal */

char *ctable = "0123456789ABCDEF"; /* character table */
int BASE = 10;					   /* base 10 */

void main(int argc, char *argv[], char *env[])
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
}

void myprintf(char *format, ...)
{

	/*
	myprintf( string, args...)
	cp = format
	ip = args
	*/

	char *cp = format;			  /* character pointer -> points to primary string */
	int *ip = (int *)&format + 1; /* integer pointer -> points to extra arguments */

	while (*cp != '\0')
	{
		if (*cp == '%') /* check for placeholder values */
		{
			cp++;

			switch (*cp) /* check type of placeholder */
			{
			case 'c':
				putchar(*ip);
				ip++;
				break;
			case 's':
				prints((char *)*ip);
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
			};
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

void rpu(u32 x)
{
	char c;
	if (x)
	{
		c = ctable[x % BASE];
		rpu(x / BASE);
		putchar(c);
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
		rpd(x * -1);
		putchar(' ');
	}
	else
	{
		rpd(x);
		putchar(' ');
	}
}

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
	putchar('x');
	if (x == 0)
	{
		putchar('0');
		return;
	}
	rpx(x);
	putchar(' ');
}

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
	putchar('0');
	if (x == 0)
	{
		putchar('0');
		return;
	}
	rpo(x);
	putchar(' ');
}
