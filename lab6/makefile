#******************************************************
# * CS360 Lab5 Server File, server.c
# * Connor Easton, Zach Nett
#******************************************************

GCC        	= gcc
CFLAGS      = -g -m32 
RM          = rm -rf
SRCDIR      = src
CMD 		= src/cmd
BINDIR 		= bin

default: all

all: clean build

build: disk
	mkdir -p $(BINDIR)
	$(GCC) $(CFLAGS) -o $(BINDIR)/main.bin $(SRCDIR)/util.c $(SRCDIR)/main.c $(SRCDIR)/type.h $(CMD)/*.c

clean veryclean:
	$(RM) $(BINDIR)

disk:
	dd if=/dev/zero of=diskimage bs=1024 count=1440
	mke2fs -b 1024 diskimage 1440

	sudo mount diskimage /mnt
	(cd /mnt; sudo rm -r *;sudo mkdir dir1 dir2; sudo mkdir dir1/dir3; sudo touch file1 file2; ls -l)
	cd /home/eastonco/dev/360/lab6
	sudo umount /mnt
	mkdir -p $(BINDIR)
	mv ./diskimage $(BINDIR)