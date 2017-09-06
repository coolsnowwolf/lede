/* J J Larworthy 27 September 2006 */

/* file to read the boot sector of a dis and the loaded image and report
 * if the boot rom would accept the data as intact and suitable for use
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/errno.h>

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

extern uint32_t crc32(uint32_t, const unsigned char *, unsigned int);

#define NUMBER_VECTORS   12
struct {
	unsigned int start_vector[NUMBER_VECTORS];
	char code[4];
	unsigned int header_length;
	unsigned int reserved[3];
	unsigned int length;
	unsigned int img_CRC;
	unsigned int CRC;
} img_header;

void print_usage(void)
{
	printf("update_header file.bin\n");
}

void print_header(void)
{
	int i;

	printf("vectors in header\n");
	for (i = 0; i < NUMBER_VECTORS; i++) {
		printf("%d:0x%08x\n", i, img_header.start_vector[i]);
	}
	printf("length:%8x\nimg_CRC:0x%08x\nHeader CRC:0x%08x\n",
		img_header.length, img_header.img_CRC, img_header.CRC);
}

int main(int argc, char **argv)
{
	int in_file;
	int status;
	int unsigned crc;
	int file_length;
	int len;

	struct stat file_stat;

	void *executable;

	in_file = open(argv[1], O_RDWR);

	if (in_file < 0) {
		printf("failed to open file:%s\n", argv[optind]);
		return -ENOENT;
	}

	status = fstat(in_file, &file_stat);

	/* read header and obtain size of image */
	status = read(in_file, &img_header, sizeof(img_header));

	file_length = file_stat.st_size - sizeof(img_header);

	if (img_header.length != file_length) {
		printf("size in header:%d, size of file: %d\n",
			img_header.length, file_length);
	}
	img_header.length = file_length;

	/* read working image and CRC */
	executable = malloc(file_length);

	status = read(in_file, executable, file_length);

	if (status != file_length) {
		printf("Failed to load image\n");
		return -ENOENT;
	}

	/* verify image CRC */
	crc = crc32(0, (const unsigned char *) executable, img_header.length);

	if (crc != img_header.img_CRC) {
		printf("New Image CRC:0x%08x, hdr:0x%08x\n", crc,
			img_header.img_CRC);
		img_header.img_CRC = crc;
	}
	memcpy(img_header.code, "BOOT", 4);
	img_header.header_length = sizeof(img_header);

	/* check header CRC */
	crc = crc32(0, (const unsigned char *) &img_header,
			sizeof(img_header) - sizeof(unsigned int));
	if (crc != img_header.CRC) {
		printf("New header CRC - crc:0x%08x hdr:0x%08x\n", crc,
			img_header.CRC);
		img_header.CRC = crc;
	}

	/* re-write the file */
	status = lseek(in_file, 0, SEEK_SET);
	if (status != 0) {
		printf("failed to rewind\n");
		return 1;
	}
	len = write(in_file, &img_header, sizeof(img_header));
	assert(len == sizeof(img_header));
	len = write(in_file, executable, file_length);
	assert(len == file_length);
	close(in_file);

	return 0;
}
