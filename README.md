![](https://img.shields.io/badge/Difficulty-Hard-informational?style=flat&color=2bbc8a)

# CPTS 360 — Systems Programming

Taken Fall 2020 @ WSU · Refactored and improved Spring 2022

A lab series building from first principles up through a complete ext2 filesystem implementation. Each lab introduces a core systems programming concept and builds on the previous.

---

## Lab Progression

| Lab | Topic | Key Concept |
|-----|-------|-------------|
| [prelab1](prelab1/) | Stack Frame Analysis | EBP register, frame pointer linked list, BSS/DATA/TEXT sections |
| [lab1](lab1/) | Disk Partitioning + Custom Printf | MBR structure, extended partitions, manual varargs (`-m32` only) |
| [lab2](lab2/) | In-Memory Filesystem | Left-child/right-sibling tree, path parsing, save/reload serialization |
| [lab3](lab3/) | Shell Simulator | fork/exec model, pipe chaining (recursive), I/O redirection via dup2 |
| [lab4](lab4/) | Multi-threaded LU Decomposition | pthreads, barrier synchronization, LU factorization, forward/backward substitution |
| [lab5](lab5/) | Client-Server File Transfer | TCP socket lifecycle, size-prefix protocol, permission bit decoding |
| [lab6](lab6/) | Full ext2 Filesystem *(capstone)* | On-disk structures, inode cache, bitmap allocation, path traversal, directory entries |

## Concept Progression

```
prelab1: Stack frames, registers, memory layout
   ↓
lab1:    Disk I/O (raw sectors), MBR, custom printf
   ↓
lab2:    Tree data structures, path resolution in memory
   ↓
lab3:    Process creation (fork/exec), pipes, file descriptors
   ↓
lab4:    Threads, shared memory, synchronization primitives
   ↓
lab5:    Network sockets, client-server protocols
   ↓
lab6:    Full filesystem: inodes, bitmaps, directory entries, mount/umount
```

## Build Requirements

| Requirement | Purpose |
|-------------|---------|
| `gcc` (or `clang`) | C compiler |
| `make` | Build system |
| `gcc-multilib` | 32-bit compilation for prelab1 and lab1 (`-m32`) |
| `e2fslibs-dev` | ext2 header files for lab6 (`sudo apt-get install e2fslibs-dev`) |
| Linux | Required for lab4 (pthread barriers), lab6 (ext2 headers) |

Each lab compiles independently with `make` from within its directory.

## Usage

```bash
cd lab3 && make && cd bin && ./sh_sim.bin   # shell simulator
cd lab5 && make && cd bin                   # client-server
  sudo ./server.bin  # terminal 1
  sudo ./client.bin  # terminal 2
cd lab6 && make && cd bin && ./main.bin     # ext2 filesystem
```

## Note

This repo is for educational reference only. Not for copying or submitting as your own work.

If this repo helped you, a star is appreciated!
