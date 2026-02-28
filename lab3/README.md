# CS360 Lab 3

SH simulator

## Running the Program

Compile the project by running `make`. This will compile the program into the `bin` directory.

From there, `cd` into the `bin` directory and run `./sh_sim.bin`. This is a sh simulator with support for simple commands like: `ls -l`, `cat t.c`, IO redirection: `cat < t.c`, `cat t.c >> outfile` as well as piping: `cat t.c | grep print`, `cat t.c | grep print | less`.

**Note:** The output looks disgusting but the results are there and do work properly.

## Key Concepts

### Fork/Exec Model

Every non-builtin command runs in a child process:

```
shell (parent)
  └── fork()
        ├── parent: wait() until child exits
        └── child:  doPipe() → execve()
                    (child process image replaced by the program)
```

The critical insight: `execve()` replaces the current process image entirely. It only returns on failure. So the child never needs to clean up — it *becomes* the target program.

### I/O Redirection: close + open + dup2

Unix file descriptors 0/1/2 are stdin/stdout/stderr. Redirection repoints these:

```c
close(1);                                  // free fd slot 1
int fd = open("out.txt", O_WRONLY|O_CREAT, 0644); // open gets fd 1 (lowest free)
dup2(fd, 1);                               // force-copy to fd 1 (in case open got fd 3+)
```

When `execve()` runs the child, it inherits fd 1 pointing to `out.txt` — the child program writes to "stdout" without knowing it's a file.

### Pipe Mechanics: Recursive doPipe

`doPipe()` handles `cmd1 | cmd2 | cmd3` by scanning right-to-left for the **rightmost** `|`:

```
"a | b | c"
  scan() → head = "a | b", tail = "c"
  fork:
    parent (reader): dup2(pipe_read, 0) → doCommand("c")
    child  (writer): doPipe("a | b", pipe_write)
      scan() → head = "a", tail = "b"
      fork:
        parent: doCommand("b")
        child:  doPipe("a") → doCommand("a")
```

Each level connects via a fresh `pipe()`, so data flows `a → b → c` through two pipe buffers.

### PATH Search

When you type `ls`, the shell doesn't know where `ls` lives. It tries each directory in PATH:

```c
for each dir in PATH:
    execve("/usr/bin/ls", name, env)   // try it; continues only if it fails
```

`execve()` fails silently with ENOENT if the file doesn't exist, so the loop naturally finds the right directory.

## Authors

* **Connor Easton**  - [Eastonco](https://github.com/Eastonco)
* **KC Wang**  - [KC Wang](https://school.eecs.wsu.edu/faculty/profile/?nid=kwang)
