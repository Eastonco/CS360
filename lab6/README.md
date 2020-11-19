# CS360 Lab 6 + Final
The Final Project

## Dependencies
* gcc
* make
* ext2_fs.h --> install with `sudo apt-get install e2fslibs-dev` **note: Linux/ext2_fs.h is depreciated

## Running the Program
To get started, first compile the project by running `make`. This will compile a clean diskimage and main.bin programs into the bin directory.

From there, `cd` into the bin directory and run main with `./main.bin`

## Command locations
all primary commands are in `src/cmd/` and are labeled accordingly. If a function refrences a command you don't recognize, it's most likely in `src/util.c`. These are all of the dependencies for reading and writing to the file system.

## Authors 
* **Connor Easton**  - [Eastonco](https://github.com/Eastonco)
* **Zach Nett** - [zjnet](https://github.com/zjnett)
* **KC Wang**  - [KC Wang](https://school.eecs.wsu.edu/faculty/profile/?nid=kwang)