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

int MBR = 0x1BE;

void main(int argc, char* argv[]){
	int disk;
	char buf[512];
	Partition *p;

    disk = open("vdisk", O_RDONLY);
	if (disk == -1){
		printf("ERROR: failed to open disk, Aborting...\n");
		return;
	}
	printf("Disk opened successfully\nReading...\n");
	read_sector(disk, 0, buf);


	p = (Partition *) &buf[MBR];
	printPartition(p, disk);
	p += 3;
	printExtendedPartition(p, disk);

};

void printPartition(Partition *p, int disk){
	printf("\e[1mDevice\tBoot Start\tEnd\tSectors\tId\n\e[0m");

	for(int i = 0; i<4; i++){
		printf("vdisk%d\t", i+1);
		printf("%d\t\t", p->start_sector);
		printf("%d\t", calculateEndAddress(p,0));
		printf("%d\t", p->nr_sectors);
		printf("%d\n", p->sys_type);
		p++;
	}
   // p->localMBR
}

void printExtendedPartition(Partition *p, int disk){
	char buf[512];
	int p4StartSector = p->start_sector;
	int partitionCount = 4;
	int offset = p4StartSector;

	while(p->nr_sectors != 0){
		read_sector(disk, offset, buf);
		p = (Partition *)&buf[MBR]; 
		printf("vdisk%d\t", partitionCount);
		printf("%d\t\t", p->start_sector + offset);
		printf("%d\t", calculateEndAddress(p,offset));
		printf("%d\t", p->nr_sectors);
		printf("%d\n", p->sys_type);
		partitionCount++;
		p++;
		offset = p4StartSector + p->start_sector;
	}
}

int calculateEndAddress(Partition *p, int offset){
	return p->start_sector + p->nr_sectors - 1 + offset;
}


void read_sector(int disk, int sector, char *buf){
	lseek(disk, sector*512, SEEK_SET);
	read(disk, buf, 512);
};