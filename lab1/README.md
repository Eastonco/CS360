# CS360 Lab 1

Partition Table, myprintf Function

## Running the Program

Compile the project by running `make`. This compiles the programs into the `bin` directory.

Next, `cd` into the `bin` directory and run `./part1.bin` or `./part2.bin`.

- **Part 1** (`lab1_part1.c`): Reads a virtual disk image (`vdisk`) and prints its partition table, including logical partitions inside the extended partition.
- **Part 2** (`lab1_part2.c`): Implements a custom `myprintf` without `<stdarg.h>`, then uses it to print `argv[]` and `env[]`.

## Key Concepts

### Part 1 — MBR and Partition Tables

A disk's first 512-byte sector is the **Master Boot Record (MBR)**. At byte offset `0x1BE` (446), there are four 16-byte **partition table entries** describing the primary partitions.

```
Byte 0     Byte 446 (0x1BE)              Byte 510
|  Boot code (446 bytes)  |  4 × 16-byte entries  | 55 AA |
```

- **sys_type 0x05**: Extended partition — a container for logical partitions.
- **Logical partitions** are stored in a linked list of **EBRs (Extended Boot Records)**, one per logical partition. Each EBR looks like a mini-MBR:
  - Entry 0: the logical partition (offset relative to this EBR's sector)
  - Entry 1: pointer to the next EBR (offset relative to the extended partition start), or zero if last

### Part 2 — Manual Varargs (32-bit only)

Standard C uses `<stdarg.h>` to handle variadic functions portably. This lab implements the same idea manually, exploiting how 32-bit x86 passes arguments:

```
Stack layout for: myprintf("fmt", arg1, arg2):
    [ &format ]   <- &format (address of format pointer on stack)
    [ arg1    ]   <- (int *)&format + 1
    [ arg2    ]   <- (int *)&format + 2
```

By casting `&format` to `int *` and incrementing, we step through each argument. **This only works with `-m32`** — on 64-bit x86, the first 6 arguments go in registers.

### Recursive Digit Printing

`rpu`/`rpd`/`rpx`/`rpo` all use the same recursive trick to print digits most-significant-first:

1. Compute the least-significant digit (`x % base`) and save it
2. Recurse on `x / base` (which handles more-significant digits first)
3. Print the saved digit after the recursion returns

This reverses the natural "right-to-left" order of digit extraction.

## Authors

* **Connor Easton**  - [Eastonco](https://github.com/Eastonco)
* **KC Wang**  - [KC Wang](https://school.eecs.wsu.edu/faculty/profile/?nid=kwang)
