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

1. Fill in NAMEs, IDs. Send to TA before 12-8-2020.
2. DEMO with TA at scheduled time AND submit a ZIP file of YOUR project to TA
3. Immediately after demo to TA, get on KCW's ZOOM session for Interview.

4. Download disk1, disk2, disk3.1, disk3.2, dif2 from samples/PROJECT directory 
LEVEL-1: Use disk1                                            55 %
   COMMANDS                    EXPECTED RESULTS           OBSERVED & comments
------------------      ------------------------------  ----------------------
startup & menu:          start up, show commands menu   _____works as expected______________
ls                       show contents of / directory   _____works as expected______________

mkdir /a ;     ls        show DIR /a exits; ls works    _____works as expected______________

mkdir /a/b ;   ls /a     make dir with pathname         _____works as expected______________

cd    /a/b ;   pwd       cd to a pathname, show CWD     _____works as expected______________

cd    ../../ ; pwd       cd upward, show CWD            _____works as expected______________

creat f1     ; ls        creat file, show f1 is a file  _____works as expected______________

link  f1 f2;   ls        hard link, both linkCount=2    _____works as expected______________

unlink   f1;   ls        unlink f1; f2 linkCount=1      _____works as expected______________

symlink f2 f3; ls        symlink; ls show f3 -> f2      _____works as expected______________

rmdir /a/b;    ls        rmdir and show results         _____works as expected______________
  
LEVEL-2: Use disk2: (file1,tiny,small,large,huge)             25 %
------------------        ---------------------------   -----------------------
cat large; cat huge       show contents to LAST LINE
                           === END OF huge FILE ===   _____works as expected______________

cp  large newlarge; ls    show they are SAME size     _____works as expected______________

cp  huge  newhuge ; ls    show they are SAME size     _____works as expected______________

              MUST DO THIS: exit YOUR project; 
(In Linux): dif2          MUST not show any lines     _____works as expected______________ 

============ IF can not do cat, cp: TRY to do these for LEVEL2 ================
open  small 0;   pfd      show fd=0 opened for R      ________N/A_________________

read 0 20;       pfd      show 20 chars read          ________N/A_________________

open file1 1;    pfd      show fd=1 opened for W      ________N/A_________________

write 1 "abcde"; ls       show file1 size=5           ________N/A_________________

close 1; pfd              show fd=1 is closed         ________N/A_________________

LEVEL-3: start with disk3.1;   MOUNT disk3.2                  20 %
-------------------   ----------------------------  ----------------------------
mount disk3.2 /mnt;       mount disk3.2 on /mnt      _____works as expected______________ 

ls /mnt                   contents of disk3.2        unable to ls, however can cd into /mnt and then ls successfully

cd /mnt/DIR1; pwd         show CWD is /mnt/DIR1      _____cd successful but can't pwd______________________

mkdir ABC; ls             show ABC exits             _______works as expected____________________

cd ../../;    pwd         show CWD=/                 ________works as expected___________________

Switch to P1; rmdir dir1; unlink file1 : not owner   _______works as expected__________________
