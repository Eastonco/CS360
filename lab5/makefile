#******************************************************
# * CS360 Lab5 Server File, server.c
# * Connor Easton, Zach Nett
#******************************************************

GCC        	= gcc
CFLAGS      = -g -m32 
RM          = rm -rf
BINDIR      = bin
SRCDIR 		= src

default: all

all: build

build: $(SRCDIR)/client.c $(SRCDIR)/server.c
	mkdir -p $(BINDIR)
	$(GCC) $(CFLAGS) -o $(BINDIR)/client.bin $(SRCDIR)/client.c
	$(GCC) $(CFLAGS) -o $(BINDIR)/server.bin $(SRCDIR)/server.c 

clean veryclean:
	$(RM) $(BINDIR)