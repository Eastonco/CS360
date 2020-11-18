#******************************************************
# * CS360 Lab5 Server File, server.c
# * Connor Easton, Zach Nett
#******************************************************

GCC        	= gcc
CFLAGS      = -g -m32 
RM          = rm -rf
BINDIR      = mountroot
CMD 		= mountroot/cmd


default: all

all: build

build: mountroot
	mkdir -p $(BINDIR)
	$(GCC) $(CFLAGS) -o $(BINDIR)/main.bin $(BINDIR)/util.c $(BINDIR)/main.c $(BINDIR)/type.h $(CMD)/cd.c $(CMD)/ls.c $(CMD)/pwd.c $(CMD)/mkdir.c $(CMD)/link.c $(CMD)/creat.c

clean veryclean:
	rm $(BINDIR)/main.bin