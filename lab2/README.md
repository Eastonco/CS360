# CS360 Lab 2
Super simple Unix/Linux File System Tree Simulator

## Dependencies
* gcc
* make

## Running the Program
To get started, first compile the project by running `make`. This will compile the program into the bin directory.

From there, `cd` into the bin directory and run `main.bin`. This is a super simple filetree simulator with support for a few simple commands.

For help: run the `menu` command in the sim.

## Checklist
```
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