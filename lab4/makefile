
GCC        	= gcc
CFLAGS      = -g -m32 -pthread -fcommon
RM          = rm -rf
BINDIR      = bin
SRCDIR 		= src

default: all

all: build

build: $(SRCDIR)/lu.c $(SRCDIR)/pivot.c
	mkdir -p $(BINDIR)
	$(GCC) $(CFLAGS) -o $(BINDIR)/pivot.bin $(SRCDIR)/pivot.c
	$(GCC) $(CFLAGS) -o $(BINDIR)/lu.bin $(SRCDIR)/lu.c 

clean veryclean:
	$(RM) $(BINDIR)