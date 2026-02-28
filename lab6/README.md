# CS360 Lab 6 — Full ext2 Filesystem (Capstone)

A fully ext2-compatible filesystem simulator: reads and writes a real ext2 disk image using the same on-disk structures as the Linux kernel.

## Running the Program

Compile the project by running `make`. This compiles a clean disk image and `main.bin` into the `bin` directory.

```bash
cd bin && ./main.bin          # run with default disk image (disk2)
./main.bin /dev/sdb1          # run with a specific disk image
make demo                     # run a scripted demonstration
make clean                    # remove all compiled files
```

## Key Concepts

### ext2 On-Disk Structure

An ext2 filesystem is organized as:

```
Block 0:  boot block (unused here)
Block 1:  Superblock — filesystem metadata (magic=0xEF53, inode count, block count, ...)
Block 2:  Group Descriptor — per-block-group info (bitmap block numbers, inode table start)
Block 3:  Block bitmap — 1 bit per block; 0=free, 1=in-use
Block 4:  Inode bitmap — 1 bit per inode; 0=free, 1=in-use
Block 5+: Inode table — array of 128-byte INODE structs (inode 2 = root directory)
...rest:  Data blocks
```

The **magic number** `0xEF53` in the superblock identifies a valid ext2 filesystem. Verified at startup.

### Inodes and Block Pointers

Each INODE contains 15 block pointers (`i_block[0..14]`):

```
i_block[0..11]  — direct: point to data blocks (12 × 1KB = 12KB max direct)
i_block[12]     — indirect: points to a block of 256 block numbers (256KB)
i_block[13]     — double indirect: points to 256 indirect blocks (64MB)
i_block[14]     — triple indirect (not implemented here)
```

### The In-Memory Inode Cache (minode[])

Reading from disk on every operation would be slow. Instead, inodes are cached in an array of `MINODE` structs:

- `iget(dev, ino)` — load an inode into the cache (or find it already there), increment refCount
- `iput(mip)` — decrement refCount; if it reaches 0 and dirty=1, write back to disk
- `dirty` flag — set when the in-memory INODE is modified; cleared after writeback

This is the same write-back caching strategy used by the Linux kernel's inode cache.

### Directory Entry Format (Variable-Length Records)

Each directory data block contains packed `DIR` entries:

```
[inode(4)|rec_len(2)|name_len(1)|file_type(1)|name(name_len)...(padding)]
```

`rec_len` is padded to a 4-byte boundary and the last entry's `rec_len` extends to the end of the block. This allows efficient in-place insertion and deletion.

### Pathname Resolution (getino)

`getino(pathname)` is the core path traversal function (equivalent to kernel `namei()`):

1. Start at root (absolute `/...`) or cwd (relative)
2. For each component: `search()` the current directory's data blocks for the name
3. `iget()` the found inode, `iput()` the previous one
4. Handle mount points: switch devices when crossing mount boundaries

### Allocation: Bitmaps

`ialloc()` and `balloc()` scan the inode/block bitmaps for the first free bit (`tst_bit == 0`), set it (`set_bit`), and update the free counts in both the superblock and group descriptor.

## Commands

| Command | Description |
|---------|-------------|
| `ls [path]` | List directory contents |
| `cd <path>` | Change directory |
| `pwd` | Print working directory |
| `mkdir <path>` | Create directory |
| `rmdir <path>` | Remove empty directory |
| `creat <path>` | Create a regular file |
| `link <src> <dst>` | Hard link |
| `unlink <path>` | Remove hard link (delete if count reaches 0) |
| `symlink <src> <dst>` | Symbolic link |
| `chmod <path>` | Change permissions |
| `cat <path>` | Print file contents |
| `open <path> <mode>` | Open file (0=R, 1=W, 2=RW, 3=APPEND) |
| `read <fd> <buf>` | Read from open file |
| `write <fd> <buf>` | Write to open file |
| `close <fd>` | Close open file |
| `cp <src> <dst>` | Copy file |
| `mount <dev> <dir>` | Mount filesystem |
| `umount <dir>` | Unmount filesystem |
| `pfd` | Print open file descriptors |
| `switch` | Switch between simulated processes |
| `quit` | Flush dirty inodes and exit |

## Build Requirements

Requires Linux with ext2 header libraries:
```bash
sudo apt-get install e2fslibs-dev
```

## Authors

* **Connor Easton**  - [Eastonco](https://github.com/Eastonco)
* **Zach Nett** - [zjnet](https://github.com/zjnett)
* **KC Wang**  - [KC Wang](https://school.eecs.wsu.edu/faculty/profile/?nid=kwang)
