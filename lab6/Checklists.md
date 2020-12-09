# Lab 6 Questions

On top of your zip file: MARK yes or no
1. does your ls work?  ls; ls /dir1; ls /dir1/dir3
      yes
2. does your cd work?  cd /dir1; cd /dir1/dir3
      yes
3. does your pwd work?
      yes

# Level 1 Checklist 

     Commands              Expected Results           Observed Results
--------------------   -------------------------   ------------------------
1. startup (with an EMPTY diskiamge)
   ls:                  Show contents of / DIR      _________________________

2. mkdir dir1; ls:      Show /dir1 exists           _________________________

   mkdir dir2; ls:      Show /dir1, /dir2 exist     _________________________

3. mkdir dir1/dir3 
   ls dir1:             Show dir3 in dir1/          _________________________

4. creat file1          Show /file1 exists          _________________________

5. rmdir dir1           REJECT (dir1 not empty)     _________________________

6. rmdir dir2; ls:      Show dir2 is removed        _________________________

7. link file1 A;ls:     file1,A same ino,  LINK=2   _________________________

8. unlink A; ls:        A deleted, file1's LINK=1   _________________________ 

9. symlink file1 B;ls:  ls must show   B->file1     _________________________

10.unlink file1; ls:    file1 deleted, B->file1     _________________________

# Level 2 Checklist

1. Download ~samples/LEVEL2/mydisk. Use it as diskimage for testing
                              |-file1 : an empty file
			      |-tiny  : a few lines of text, only 1 data block
			      |-small : with 2 direct data blocks
			      |-large : with Indirect data blocks
			      |-huge  : with Double-Indirect data blocks


A. IF YOU can do 2,3,4 below, you are done, skip Part B below

2. Test YOUR cat:
    cat tiny, cat small, cat large, cat huge: SEE OUTPUTS? ________________ 40

3. Test YOUR cp:  '
    cp small newsmall; ls: newsmall exist? SAME SIZE? _____________________ 10
    cp large newlarge; ls: newlarge exist? SAME SIZE? _____________________ 20
    cp huge newhuge;   ls: newhuge  exist? SAME SIZE? _____________________ 30

4. Enter quit to exit YOUR program. Check YOUR cp results under Linux:
		 
	 sudo mount mydisk /mnt           
	 sudo ls -l /mnt                   # should see all files
	 sudo diff /mnt/huge /mnt/newhuge  # diff will show differences, if ANY
	 sudo umount /mnt

===============================================================================

B. IF you can NOT cat, cp correctly, do the following

5. Show your open, pfd, close                                               20

6. Show you can open small for READ; read a few times from the opened fd    20

7. Show you can open file1 for WRITE; write a few times to the opened fd    20

==============================================================================

# Level 3 Checklist
LEVEL-1: Use disk1                                            55 %
   COMMANDS                    EXPECTED RESULTS           OBSERVED & comments
------------------      ------------------------------  ----------------------
startup & menu:          start up, show commands menu   ______________________
ls                       show contents of / directory   ______________________

mkdir /a ;     ls        show DIR /a exits; ls works    _______________________

mkdir /a/b ;   ls /a     make dir with pathname         _______________________

cd    /a/b ;   pwd       cd to a pathname, show CWD     ________________________

cd    ../../ ; pwd       cd upward, show CWD            _______________________ 

creat f1     ; ls        creat file, show f1 is a file  _______________________

link  f1 f2;   ls        hard link, both linkCount=2    ________________________

unlink   f1;   ls        unlink f1; f2 linkCount=1      ________________________

symlink f2 f3; ls        symlink; ls show f3 -> f2      ________________________

rmdir /a/b;    ls        rmdir and show results         ________________________
  
LEVEL-2: Use disk2: (file1,tiny,small,large,huge)             25 %
------------------        ---------------------------   -----------------------
cat large; cat huge       show contents to LAST LINE
                           === END OF huge FILE ===   _________________________

cp  large newlarge; ls    show they are SAME size     _________________________

cp  huge  newhuge ; ls    show they are SAME size     _________________________

              MUST DO THIS: exit YOUR project; 
(In Linux): dif2          MUST not show any lines     _________________________ 

============ IF can not do cat, cp: TRY to do these for LEVEL2 ================
open  small 0;   pfd      show fd=0 opened for R      _________________________

read 0 20;       pfd      show 20 chars read          _________________________

open file1 1;    pfd      show fd=1 opened for W      _________________________

write 1 "abcde"; ls       show file1 size=5           _________________________

close 1; pfd              show fd=1 is closed         _________________________

LEVEL-3: start with disk3.1;   MOUNT disk3.2                  20 %
-------------------   ----------------------------  ----------------------------
mount disk3.2 /mnt;       mount disk3.2 on /mnt      ___________________________

ls /mnt                   contents of disk3.2        ___________________________

cd /mnt/DIR1; pwd         show CWD is /mnt/DIR1      ___________________________

mkdir ABC; ls             show ABC exits             ___________________________

cd ../../;    pwd         show CWD=/                 ___________________________

Switch to P1; rmdir dir1; unlink file1 : not owner   __________________________