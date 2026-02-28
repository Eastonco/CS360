#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

/*
 * Partition table entry structure (16 bytes each).
 * The MBR (Master Boot Record) lives in the first 512 bytes of a disk.
 * At offset 0x1BE (446 decimal), there are four 16-byte partition entries
 * describing the primary partitions. The last two bytes (0x1FE–0x1FF)
 * are the boot signature: 0x55AA.
 */
typedef struct partition {
	u8 drive;        /* drive number: FD=0, HD=0x80, etc. */

	u8 head;         /* CHS starting head */
	u8 sector;       /* CHS starting sector */
	u8 cylinder;     /* CHS starting cylinder */

	u8 sys_type;     /* partition type: 0x83=Linux, 0x05=Extended, 0x07=NTFS, etc. */

	u8 end_head;     /* CHS end head */
	u8 end_sector;   /* CHS end sector */
	u8 end_cylinder; /* CHS end cylinder */

	u32 start_sector; /* LBA start sector (counting from 0, absolute) */
	u32 nr_sectors;   /* total sectors in this partition */
} Partition;

/*
 * 0x1BE = 446: the byte offset within the 512-byte MBR where the
 * partition table begins. The layout before it is the boot code.
 */
int MBR = 0x1BE;

void printPartition(Partition *p, int disk);
void printExtendedPartition(Partition *p, int disk);
int calculateEndAddress(Partition *p, int offset);
void read_sector(int disk, int sector, char *buf);

int main(int argc, char *argv[])
{
	int disk;
	char buf[512];
	Partition *p;

	disk = open("vdisk", O_RDONLY);

	if (disk == -1) {
		printf("ERROR: failed to open disk, Aborting...\n\n");
		return 1;
	}
	printf("Disk opened successfully\nReading...\n\n");

	/* Sector 0 is the MBR — read it into buf, then cast to Partition* */
	read_sector(disk, 0, buf);

	/*
	 * The first four partition entries start at buf[0x1BE].
	 * We print all four, then follow the extended partition chain.
	 * The extended partition (type 0x05) acts as a container for
	 * logical partitions stored in a linked list of EBRs (Extended Boot Records).
	 */
	p = (Partition *)&buf[MBR];
	printPartition(p, disk);
	p += 3; /* jump to entry 4 (0-indexed: entry 3) — the extended partition */
	printExtendedPartition(p, disk);

	close(disk);
	return 0;
}

void printPartition(Partition *p, int disk)
{
	printf("\e[4mDevice\tBoot Start\tEnd\tSectors\tId\n\e[0m");

	for (int i = 0; i < 4; i++) {
		printf("vdisk%d\t", i + 1);
		printf("%d\t\t", p->start_sector);
		printf("%d\t", calculateEndAddress(p, 0));
		printf("%d\t", p->nr_sectors);
		printf("%d\n", p->sys_type);
		p++;
	}
}

/*
 * Extended partitions use a linked list of EBRs (Extended Boot Records).
 * Each EBR is a 512-byte sector (like a mini-MBR) containing two entries:
 *   - Entry 0: the logical partition described by this EBR (relative to this EBR)
 *   - Entry 1: pointer to the NEXT EBR in the chain (relative to the extended
 *              partition start), or zero if this is the last logical partition.
 *
 * We follow the chain by reading each EBR sector and examining entry 1.
 */
void printExtendedPartition(Partition *p, int disk)
{
	char buf[512];
	int p4StartSector = p->start_sector; /* absolute LBA of the extended partition */
	int partitionCount = 5;              /* logical partitions start at vdisk5 */
	int offset = p4StartSector;

	while (p->nr_sectors != 0) {
		/* Read the EBR at the current offset */
		read_sector(disk, offset, buf);
		p = (Partition *)&buf[MBR];

		/* Entry 0: the logical partition (start is relative to this EBR) */
		printf("vdisk%d\t", partitionCount);
		printf("%d\t\t", p->start_sector + offset);
		printf("%d\t", calculateEndAddress(p, offset));
		printf("%d\t", p->nr_sectors);
		printf("%d\n", p->sys_type);
		partitionCount++;

		/* Entry 1: next EBR pointer (relative to extended partition start) */
		p++;
		offset = p4StartSector + p->start_sector;
	}
}

int calculateEndAddress(Partition *p, int offset)
{
	return p->start_sector + p->nr_sectors - 1 + offset;
}

/*
 * read_sector: reads one 512-byte sector from the disk file.
 *
 * The seek+read pattern:
 *   lseek(fd, byte_offset, SEEK_SET) — move the file cursor to the right byte
 *   read(fd, buf, 512)               — read exactly 512 bytes into buf
 *
 * Disks are block devices; every sector is exactly 512 bytes, so
 * sector N starts at byte N * 512.
 */
void read_sector(int disk, int sector, char *buf)
{
	if (lseek(disk, (off_t)sector * 512, SEEK_SET) == (off_t)-1) {
		printf("ERROR: lseek to sector %d failed\n", sector);
		return;
	}
	if (read(disk, buf, 512) != 512) {
		printf("ERROR: read of sector %d failed\n", sector);
	}
}
