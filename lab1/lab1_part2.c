typedef unsigned int u32;

char *ctable = "0123456789ABCDEF";
int  BASE = 10; 

void main(int argc, char* argv[], char* env[]){
    
    myprintf("char=%c string=%s      dec=%d hex=%x oct=%o neg=%d\n", 'A', "this is a test", 100,    100,   100,  -100);
    myprintf("argc=%d", argc);
    for(int i = 0; i < argc; i++){
        myprintf("argv[%d]=%s", i, argv[i]);
    }
    for(int i = 0; env[i] != 0; i++){
        myprintf("env[%d]=%s", i, env[i]);
    }
}


void myprintf(char *format, ...){
    char *cp = format;
    int *ip = &format+1;

    while (*cp != '\0'){

        if(*cp == '%'){
            cp++;

            switch (*cp)
            {
            case 'c' :
                putchar(*ip);
                ip++;
                break;
            case 's' :
                prints(*ip);
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
            };
            cp++;
            
        }

        else{
            putchar(*cp);
            cp++;
        }
    }

    putchar('\n');

}

void prints(char * s){
    for(int i = 0; s[i] != '\0'; i++){
        putchar(s[i]);
    }
}


void rpu(u32 x)
{  
    char c;
    if (x){
       c = ctable[x % BASE];
       rpu(x / BASE);
       putchar(c);
    }
}

void printu(u32 x)
{
   (x==0)? putchar('0') : rpu(x);
   putchar(' ');
}

void rpd(int x){
    char c;
    if(x){
        c = ctable[x % BASE];
        rpd(x / BASE);
        putchar(c);
    }

}

void printd(int x){
    if (x == 0){
        putchar('0');
        putchar(' ');
        return 0;
    }
    if (x < 0)
    {
        putchar('-');
        rpd(x * -1);
        putchar(' ');
    }
    else{
    rpd(x);
    putchar(' ');
    }

}

void rpx(u32 x){
    char c;
    if(x){
        c = ctable[x % 16];
        rpx(x / 16);
        putchar(c);
    }

}

void printx(u32 x){
    putchar('0');
    putchar('x');
    if (x == 0){
        putchar('0');
        return;
    }
    rpx(x);
    putchar(' ');
}

void rpo(u32 x){
    char c;
    if(x){
        c = ctable[x % 8];
        rpo(x / 8);
        putchar(c);
    }
}

void printo(u32 x){
    putchar('0');
    if (x == 0){
        putchar('0');
        return;
    }
    rpo(x);
    putchar(' ');
}

