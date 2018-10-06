#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>

/* #define DEBUG 1 */

/*
 * Fw Header Layout for Netgear / Sercomm devices
 * */
static const char *magic = "sErCoMm"; /* 7 */
/* 7-11: version control/download control ? */
unsigned char version[4] = {0x00, 0x01, 0x00, 0x00};
char *hwID = ""; /* 11-43 , ASCII/HEX */
char *hwVer = ""; /* 44-57 , ASCII/HEX */
char *swVer = ""; /* 58-62 , ASCII/HEX */
/* magic again. */

#define HEADER_SIZE 71

/* null bytes until 511 */
u_int32_t checksum = 0xFF; /* checksum */
/* 512 onwards -> ZIP containing rootfs with the same Header */


/* appended on rootfs for the Header. */
const int footer_size = 128;

struct file_info {
	char		*file_name;	/* name of the file */
	char		*file_data;	/* data of the file in memory */
	u_int32_t	 file_size;	/* length of the file */
};

u_int8_t getCheckSum(char *data, int len)
{

	int32_t previous = 0;
	u_int32_t new = 0;

	for (u_int32_t i = 0; i < len; i++) {
		new = (data[i] + previous) % 256;
		previous = new | previous & -256;
	}
	return (u_int8_t) new;
}

void *bufferFile(struct file_info *finfo, int dontload)
{
	int fs = 0;
	FILE *f = NULL;

#ifdef DEBUG
	printf("Opening file: %s\n", finfo->file_name);
#endif
	f = fopen(finfo->file_name, "rb");
	if (f == NULL) {
		perror("Error");
		exit(1);
	}

	fseek(f, 0L, SEEK_END);
	fs = ftell(f);
	rewind(f);

#ifdef DEBUG
	printf("Filesize: %i .\n", fs);
#endif

	finfo->file_size = fs;

	if (dontload) {
		return 0;
	}

	char *data = malloc(fs);
	finfo->file_data = data;

	int read = fread(data, fs, 1, f);

	if (read != 1) {
		printf("Error reading file %s.", finfo->file_name);
		exit(1);
	}

#ifdef DEBUG
	printf("File: read successfully %i bytes.\n", read*fs);
#endif
	fclose(f);
}

void *writeFile(struct file_info *finfo)
{

#ifdef DEBUG
	printf("Writing file: %s.\n", finfo->file_name);
#endif

	FILE *fout = fopen(finfo->file_name, "w");

	if (!fwrite(finfo->file_data, finfo->file_size, 1, fout)) {
		printf("Wanted to write, but something went wrong.\n");
		fclose(fout);
		exit(1);
	}
	fclose(fout);
}

void *rmFile(struct file_info *finfo)
{
	remove(finfo->file_name);
	free(finfo->file_data);
	finfo->file_size = 0;
}

void *usage(char *argv[])
{
	printf("Usage: %s <sysupgradefile> <kernel_offset> <HWID> <HWVER> <SWID>\n"
		"All are positional arguments ...	\n"
		"	sysupgradefile:		File with the kernel uimage at 0\n"
		"	kernel_offset:		Offset in Hex where the kernel is located\n"
		"	HWID:			Hardware ID, ASCII\n"
		"	HWVER:			Hardware Version, ASCII\n"
		"	SWID:			Software Version, Hex\n"
		"	\n"
		"	", argv[0]);
}

int main(int argc, char *argv[])
{
	printf("Building fw image for sercomm devices.\n");

	if (argc == 2) {
		struct file_info myfile = {argv[1], 0, 0};
		bufferFile(&myfile, 0);
		char chksum = getCheckSum(myfile.file_data, myfile.file_size);
		printf("Checksum for File: %X.\n", chksum);
		return 0;
	}

	if (argc != 6) {
		usage(argv);
		return 1;
	}

	/* Args */

	struct file_info sysupgrade = {argv[1], 0, 0};
	bufferFile(&sysupgrade, 0);

	int kernel_offset = 0x90000; /* offset for the kernel inside the rootfs, default val */
	sscanf(argv[2], "%X", &kernel_offset);
#ifdef DEBUG
	printf("Kernel_offset: at %X/%i bytes.\n", kernel_offset, kernel_offset);
#endif
	char *hwID = argv[3];
	char *hwVer = argv[4];
	u_int32_t swVer = 0;
	sscanf(argv[5],"%4X",&swVer);
	swVer = bswap_32(swVer);

	char *rootfsname = malloc(2*strlen(sysupgrade.file_name) + 8);
	sprintf(rootfsname, "%s.rootfs", sysupgrade.file_name);

	char *zipfsname = malloc(2*strlen(rootfsname) + 5);
	sprintf(zipfsname, "%s.zip", rootfsname);
	/* / Args */

#ifdef DEBUG
	printf("Building header: %s %s %2X %s.\n", hwID , hwVer, swVer, magic);
#endif
	/* Construct the firmware header/magic */
	struct file_info header = {0, 0, 0};
	header.file_size = HEADER_SIZE;
	header.file_data = malloc(HEADER_SIZE);
	bzero(header.file_data, header.file_size);

	char *tg = header.file_data;
	strcpy(tg, magic);
	memcpy(tg+7, version, 4*sizeof(char));
	strcpy(tg+11, hwID);
	strcpy(tg+45, hwVer);
	memcpy(tg+55, &swVer,sizeof(u_int32_t));
	strcpy(tg+63, magic);

#ifdef DEBUG
	printf("Header done, now creating rootfs.");
#endif
	/* Construct a rootfs */
	struct file_info rootfs = {0, 0, 0};
	rootfs.file_size = sysupgrade.file_size + kernel_offset + footer_size;
	rootfs.file_data =  malloc(rootfs.file_size);
	bzero(rootfs.file_data, rootfs.file_size);
	rootfs.file_name = rootfsname;

	/* copy Owrt image to Kernel location */
	memcpy(rootfs.file_data+kernel_offset, sysupgrade.file_data, sysupgrade.file_size);

	/* 22 added to get away from sysup image, no other reason.
	 *  updater searches for magic anyway */
	tg = rootfs.file_data + kernel_offset + sysupgrade.file_size+22;

	memcpy(tg, header.file_data, header.file_size);
	writeFile(&rootfs);

#ifdef DEBUG
	printf("Preparing to zip.\n");
#endif
	/* now that we got the rootfs, repeat the whole thing again(sorta):
	 * 1. zip the rootfs */
	char *zipper = malloc(5 + 2*strlen(rootfs.file_name) + 4);
	sprintf(zipper, "%s %s %s", "zip ", zipfsname, rootfs.file_name);
	int ret = system(zipper);

	/* clear rootfs file */
	rmFile(&rootfs);

	/* and load zipped fs */
	struct file_info zippedfs = {zipfsname, 0, 0};
	bufferFile(&zippedfs, 0);

#ifdef DEBUG
	printf("Creating Image.\n");
#endif

	/* 2. create new file 512+rootfs size */
	struct file_info image = {argv[1], 0, 0};
	image.file_data = malloc(zippedfs.file_size + 512);
	image.file_size = zippedfs.file_size + 512;

	/* 3. copy zipfile at loc 512 */
	memcpy(image.file_data+512, zippedfs.file_data, zippedfs.file_size);
	rmFile(&zippedfs);

	/* 4. add header to file */
	memcpy(image.file_data, header.file_data, header.file_size);

	/* 5. do a checksum run, and compute checksum */
	char chksum = getCheckSum(image.file_data, image.file_size);
#ifdef DEBUG
	printf("Checksum for Image: %X.\n", chksum);
#endif

	/* 6. write the checksum invert into byte 511 to bring it to 0 */
	chksum = (chksum ^ 0xFF) + 1;
	memcpy(image.file_data+511, &chksum, 1);

	chksum = getCheckSum(image.file_data, image.file_size);
#ifdef DEBUG
	printf("Checksum for after fix: %X.\n", chksum);
#endif
	/* 7. pray that the updater will accept the file */
	writeFile(&image);
	return 0;
}
