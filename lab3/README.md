# CS360 Lab 3

SH simulator

## Running the Program

Compile the project by running `make`. This will compile the program into the `bin` directory.

From there, `cd` into the `bin` directory and run `./sh_sim.bin`. This is a sh simulator with support for simple commands like: `ls -l`, `cat t.c`, IO redirection: `cat < t.c`, `cat t.c >> outfile` as well as piping: `cat t.c | grep print`, `cat t.c | grep print | less`.

**Note:** The output looks disgusting but the results are there and do work properly.

## Authors

* **Connor Easton**  - [Eastonco](https://github.com/Eastonco)
