GCC        	= gcc
CFLAGS      = -g -m32 
RM          = rm -rf
BINDIR      = bin
SRCDIR 		= src

default: all

all: build

build: $(SRCDIR)/lab1_part1.c $(SRCDIR)/lab1_part2.c
	mkdir -p $(BINDIR)
	$(GCC) $(CFLAGS) -o $(BINDIR)/part1.bin $(SRCDIR)/lab1_part1.c 
	$(GCC) $(CFLAGS) -o $(BINDIR)/part2.bin $(SRCDIR)/lab1_part2.c 

clean veryclean:
	$(RM) $(BINDIR)