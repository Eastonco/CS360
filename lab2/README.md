# CS360 Lab 2

Super simple Unix/Linux File System Tree Simulator

## Running the Program

Compile the project by running `make`. This will compile the program into the `bin` directory.

From there, `cd` into the `bin` directory and run `./main.bin`.

For help: run the `menu` command in the sim.

## Key Concepts

### Left-Child / Right-Sibling Tree (LC-RS)

This lab represents a filesystem tree using a two-pointer structure where each node has at most two pointers instead of a variable-length array of children:

```
typedef struct node {
    char name[64];
    struct node *childPtr;    // first child
    struct node *siblingPtr;  // next sibling (same parent)
    struct node *parentPtr;   // back-pointer for pwd()
} NODE;
```

Example: root with children a, b, c where a has children x, y:

```
root ──childPtr──> a ──siblingPtr──> b ──> c ──> NULL
                   |
               childPtr
                   |
                   x ──siblingPtr──> y ──> NULL
```

To list all children of a node: `node->childPtr`, then follow `siblingPtr`.

### parse_pathname: Absolute vs Relative Paths

`parse_pathname()` splits a path into `dname` (parent directory) and `bname` (target name), then navigates to the parent:

- **Absolute path** (`/a/b/c`): start from `root`, walk each component
- **Relative path** (`foo/bar`): start from `cwd`
- Special cases: `.` → cwd, `..` → cwd->parentPtr

### Save / Reload Format

The filesystem is serialized as a tab-separated text file:
```
type    pathname

D   /dir1
F   /dir1/file1
D   /dir2
```
`reload()` reconstructs the tree by calling `mkdir()` or `create()` for each line.

## Checklist

```text
    Commands                Expected Results                      Comments
---------------------  -------------------------------------  -----------------
1. pwd; ls;            cwd = /; directory empty

2. mkdir d1; ls        show [D d1] exists

3. creat f1; ls        show [D d1] [F f1] exist

4. mkdir d2; creat f2;
   mkdir d3; ls        show [D d1] [F f1] [D d2] [F f2] [D d3]

5. rmdir d1; rm f2; ls show [D f1] [D d2] [D d3]

6. mkdir d2/d4; ls d2  show [D d4] in DIR d2

7. cd /d2/d4; pwd      cwd = /d2/d4

8. cd ../../; pwd      cwd = /

9. mkdir /d2/d5/d6     Invalid path /d2/d5; fail

10. cd f1              f1 NOT DIR; fail

11. exit               save tree as a FILE; exit

12. start; reload filesystem.txt     MUST BE THE SAME TREE as before. make sure the filesytem doc is in the same dir
```

## Authors

* **Connor Easton**  - [Eastonco](https://github.com/Eastonco)
* **KC Wang**  - [KC Wang](https://school.eecs.wsu.edu/faculty/profile/?nid=kwang)
