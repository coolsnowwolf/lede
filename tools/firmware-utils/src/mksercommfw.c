#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <byteswap.h>
#include <endian.h>
#include <getopt.h>

#if !defined(__BYTE_ORDER)
#error "Unknown byte order"
#endif

#if __BYTE_ORDER == __BIG_ENDIAN
#define cpu_to_be32(x)  (x)
#elif __BYTE_ORDER == __LITTLE_ENDIAN
#define cpu_to_be32(x)  bswap_32(x)
#else
#error "Unsupported endianness"
#endif

/* #define DEBUG 1 */

#ifdef DEBUG
#define DBG(...) {printf(__VA_ARGS__); }
#else
#define DBG(...) {}
#endif

#define ERR(...) {printf(__VA_ARGS__); }

/*
 * Fw Header Layout for Netgear / Sercomm devices (bytes)
 *
 * Size : 512 bytes + zipped image size
 *
 * Locations:
 * magic  : 0-6    ASCII
 * version: 7-11   fixed
 * hwID   : 11-44  ASCII
 * hwVer  : 45-54  ASCII
 * swVer  : 55-62  uint32_t in BE
 * magic  : 63-69  ASCII
 * ChkSum : 511    Inverse value of the full image checksum while this location is 0x00
 */
static const char* magic = "sErCoMm"; /* 7 */
static const unsigned char version[4] = { 0x00, 0x01, 0x00, 0x00 };
static const int header_sz = 512;
static const int footer_sz = 71;

static int is_header = 1;

struct file_info {
	char* file_name; /* name of the file */
	char* file_data; /* data of the file in memory */
	u_int32_t file_size; /* length of the file */
};

static u_int8_t getCheckSum(char* data, int len) {
	u_int8_t new = 0;
	int i;

	if (!data) {
		ERR("Invalid pointer provided!\n");
		return 0;
	}

	for (i = 0; i < len; i++) {
		new += data[i];
	}

	return new;
}

/*
 * read file into buffer
 * add space for header/footer
 */
static int copyToOutputBuf(struct file_info* finfo) {
	FILE* fp = NULL;

	int file_sz = 0;
	int extra_sz;
	int hdr_pos;
	int img_pos;

	if (!finfo || !finfo->file_name) {
		ERR("Invalid pointer provided!\n");
		return -1;
	}

	DBG("Opening file: %s\n", finfo->file_name);

	if (!(fp = fopen(finfo->file_name, "rb"))) {
		ERR("Error opening file: %s\n", finfo->file_name);
		return -1;
	}

	/* Get filesize */
	rewind(fp);
	fseek(fp, 0L, SEEK_END);
	file_sz = ftell(fp);
	rewind(fp);

	if (file_sz < 1) {
		ERR("Error getting filesize: %s\n", finfo->file_name);
		fclose(fp);
		return -1;
	}

	if (is_header) {
		extra_sz = header_sz;
		hdr_pos = 0;
		img_pos = header_sz;
	} else {
		extra_sz = footer_sz;
		hdr_pos = file_sz;
		img_pos = 0;
	}

	DBG("Filesize: %i\n", file_sz);
	finfo->file_size = file_sz + extra_sz;

	if (!(finfo->file_data = malloc(finfo->file_size))) {
		ERR("Out of memory!\n");
		fclose(fp);
		return -1;
	}

	/* init header/footer bytes */
	memset(finfo->file_data + hdr_pos, 0, extra_sz);

	/* read file and take care of leading header if exists */
	if (fread(finfo->file_data + img_pos, 1, file_sz, fp) != file_sz) {
		ERR("Error reading file %s\n", finfo->file_name);
		fclose(fp);
		return -1;
	}

	DBG("File: read successful\n");
	fclose(fp);

	return hdr_pos;
}

static int writeFile(struct file_info* finfo) {
	FILE* fp;

	if (!finfo || !finfo->file_name) {
		ERR("Invalid pointer provided!\n");
		return -1;
	}

	DBG("Opening file: %s\n", finfo->file_name);

	if (!(fp = fopen(finfo->file_name, "w"))) {
		ERR("Error opening file: %s\n", finfo->file_name);
		return -1;
	}

	DBG("Writing file: %s\n", finfo->file_name);

	if (fwrite(finfo->file_data, 1, finfo->file_size, fp) != finfo->file_size) {
		ERR("Wanted to write, but something went wrong!\n");
		fclose(fp);
		return -1;
	}

	fclose(fp);
	return 0;
}

static void usage(char* argv[]) {
	printf("Usage: %s [OPTIONS...]\n"
	       "\n"
	       "Options:\n"
	       "  -f            add sercom footer (if absent, header)\n"
	       "  -b <hwid>     use hardware id specified with <hwid> (ASCII)\n"
	       "  -r <hwrev>    use hardware revision specified with <hwrev> (ASCII)\n"
	       "  -v <version>  set image version to <version> (decimal, hex or octal notation)\n"
	       "  -i <file>     input file\n"
	       , argv[0]);
}

int main(int argc, char* argv[]) {
	struct file_info image = { 0 };

	char* hwID = NULL;
	char* hwVer = NULL;
	u_int32_t swVer = 0;
	u_int8_t chkSum;
	int hdr_offset;

	while ( 1 ) {
		int c;

		c = getopt(argc, argv, "b:i:r:v:f");
		if (c == -1)
			break;

		switch (c) {
		case 'b':
			hwID = optarg;
			break;
		case 'f':
			is_header = 0;
			break;
		case 'i':
			image.file_name = optarg;
			break;
		case 'r':
			hwVer = optarg;
			break;
		case 'v':
			swVer = (u_int32_t) strtol(optarg, NULL, 0);
			swVer = cpu_to_be32(swVer);
			break;
		default:
			usage(argv);
			return EXIT_FAILURE;
		}
	}

	if (!hwID || !hwVer || !image.file_name) {
			usage(argv);
			return EXIT_FAILURE;
	}

	/*
	 * copy input to buffer, add extra space for header/footer and return
	 * header position
	 */
	hdr_offset = copyToOutputBuf(&image);
	if (hdr_offset < 0)
		return EXIT_FAILURE;

	DBG("Filling header: %s %s %2X %s\n", hwID, hwVer, swVer, magic);

	strncpy(image.file_data + hdr_offset + 0, magic, 7);
	memcpy(image.file_data + hdr_offset + 7, version, sizeof(version));
	strncpy(image.file_data + hdr_offset + 11, hwID, 34);
	strncpy(image.file_data + hdr_offset + 45, hwVer, 10);
	memcpy(image.file_data + hdr_offset + 55, &swVer, sizeof(swVer));
	strncpy(image.file_data + hdr_offset + 63, magic, 7);

	/* calculate checksum and invert checksum */
	if (is_header) {
		chkSum = getCheckSum(image.file_data, image.file_size);
		chkSum = (chkSum ^ 0xFF) + 1;
		DBG("Checksum for Image: %hhX\n", chkSum);

		/* write checksum to header */
		image.file_data[511] = (char) chkSum;
	}

	/* overwrite input file */
	if (writeFile(&image))
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}
