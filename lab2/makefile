GCC        	= gcc
CFLAGS      = -g -m32 -fcommon
RM          = rm -rf
BINDIR      = bin
SRCDIR 		= src

default: all

all: build

build: $(SRCDIR)/main.c $(SRCDIR)/functions.c $(SRCDIR)/header.h
	mkdir -p $(BINDIR)
	$(GCC) $(CFLAGS) -o $(BINDIR)/main.bin $(SRCDIR)/main.c $(SRCDIR)/functions.c $(SRCDIR)/header.h

clean veryclean:
	$(RM) $(BINDIR)