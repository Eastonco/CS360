# CS360 Lab 6 + Final Levels 1,2, and almost 3

The Final Project: A fully ext2 compatable file system

## Running the Program

Compile the project by running `make`. This will compile a clean diskimage and `main.bin` into the `bin` directory.

From there, `cd` into the `bin` directory and run the program with `./main.bin`.

You can create the final demo with `make demo` and remove all bin files with `make clean`.

## Command locations

All primary commands are in `src/cmd/` and are labeled accordingly. If a function refrences a command you don't recognize, it's most likely in `src/util.c`. These are all of the dependencies for reading and writing to the file system.

## Authors

* **Connor Easton**  - [Eastonco](https://github.com/Eastonco)
* **Zach Nett** - [zjnet](https://github.com/zjnett)
* **KC Wang**  - [KC Wang](https://school.eecs.wsu.edu/faculty/profile/?nid=kwang)
