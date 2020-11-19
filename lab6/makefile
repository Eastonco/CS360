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
	$(GCC) $(CFLAGS) -o $(BINDIR)/main.bin $(BINDIR)/util.c $(BINDIR)/main.c $(BINDIR)/type.h $(CMD)/*.c

clean veryclean:
	rm $(BINDIR)/main.bin