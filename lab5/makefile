#******************************************************
# * CS360 Lab5 Server File, server.c
# * Connor Easton, Zach Nett
#******************************************************

GCC        	= gcc
CFLAGS      = -g -m32 
RM          = rm -rf
BINDIR      = bin


default: all

all: build

build: client.c server.c
	mkdir -p $(BINDIR)
	$(GCC) $(CFLAGS) -o $(BINDIR)/client.bin client.c
	$(GCC) $(CFLAGS) -o $(BINDIR)/server.bin server.c 

clean veryclean:
	$(RM) $(BINDIR)