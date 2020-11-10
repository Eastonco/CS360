#******************************************************
# * CS360 Lab5 Server File, server.c
# * Connor Easton, Zach Nett
#******************************************************

GCC        	= gcc
CFLAGS      = -g -m32 
RM          = rm -rf
BINDIR      = mountroot


default: all

all: build

build: mountroot
	mkdir -p $(BINDIR)
	$(GCC) $(CFLAGS) -o $(BINDIR)/main.bin mountroot/util.c mountroot/main.c mountroot/type.h mountroot/cmd/cd.c mountroot/cmd/ls.c mountroot/cmd/pwd.c mountroot/cmd/mkdir.c

clean veryclean:
	rm $(BINDIR)/main.bin