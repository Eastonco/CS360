#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

typedef struct partition {
	u8 drive;             /* drive number FD=0, HD=0x80, etc. */

	u8  head;             /* starting head */
	u8  sector;           /* starting sector */
	u8  cylinder;         /* starting cylinder */

	u8  sys_type;         /* partition type: NTFS, LINUX, etc. */

	u8  end_head;         /* end head */
	u8  end_sector;       /* end sector */
	u8  end_cylinder;     /* end cylinder */

	u32 start_sector;     /* starting sector counting from 0 */
	u32 nr_sectors;       /* number of of sectors in partition */
}Partition;



void main(int argc, char* argv[]){
	int MBR = 0x1BE;
	int disk;
	char buf[512];
	Partition *p;

    disk = open("vdisk", O_RDONLY);
	read_sector(disk, 0, buf);

	p = (Partition *) &buf[MBR];
	p++;

	int end_sector = p->start_sector + p->nr_sectors - 1;

	printf("\e[1mDevice\tBoot Start\tEnd\tSectors\tId\tType\n\e[0m");
	printf("vdisk\t\t");
	printf("%d\t", p->start_sector);
	printf("%d\t", end_sector);
	printf("%d\t", p->nr_sectors);
	printf("%d", p->sys_type);

	printf("\n\n");
};


int read_sector(int disk, int sector, char *buf){
	lseek(disk, sector*512, SEEK_SET);
	read(disk, buf, 512);
};