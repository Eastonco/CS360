GCC        	= gcc
CFLAGS      = -g -m32 -fcommon
RM          = rm -rf
BINDIR      = bin
SRCDIR 		= src

default: all

all: build

build: $(SRCDIR)/sh_sim.c
	mkdir -p $(BINDIR)
	$(GCC) $(CFLAGS) -o $(BINDIR)/pivot.bin $(SRCDIR)/sh_sim.c

clean veryclean:
	$(RM) $(BINDIR)