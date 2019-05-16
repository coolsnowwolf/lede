#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <byteswap.h>
#include <endian.h>

#if !defined(__BYTE_ORDER)
#error "Unknown byte order"
#endif

#if __BYTE_ORDER == __BIG_ENDIAN
#define cpu_to_be32(x)  (x)
#define be32_to_cpu(x)  (x)
#define cpu_to_be16(x)  (x)
#define be16_to_cpu(x)  (x)
#elif __BYTE_ORDER == __LITTLE_ENDIAN
#define cpu_to_be32(x)  bswap_32(x)
#define be32_to_cpu(x)  bswap_32(x)
#define cpu_to_be16(x)  bswap_16(x)
#define be16_to_cpu(x)  bswap_16(x)
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
#define ALIGN(a,b) ((a) + ((b) - ((a) % (b))))
#define ROOTFS_ALIGN 128
#define HEADER_SIZE 71

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

/* 7-11: version control/download control ? */
static const unsigned char version[4] = { 0x00, 0x01, 0x00, 0x00 };


/* 512 onwards -> ZIP containing rootfs with the same Header */

struct file_info {
	char* file_name; /* name of the file */
	char* file_data; /* data of the file in memory */
	u_int32_t file_size; /* length of the file */
};

static u_int8_t getCheckSum(char* data, int len) {
	u_int8_t new = 0;

	if (!data) {
		ERR("Invalid pointer provided!\n");
		return 0;
	}

	for (int i = 0; i < len; i++) {
		new += data[i];
	}

	return new;
}

static int bufferFile(struct file_info* finfo) {
	int fs = 0;
	FILE* fp = NULL;

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
	fs = ftell(fp);
	rewind(fp);

	if (fs < 0) {
		ERR("Error getting filesize: %s\n", finfo->file_name);
		fclose(fp);
		return -1;
	}

	DBG("Filesize: %i\n", fs);
	finfo->file_size = fs;

	if (!(finfo->file_data = malloc(fs))) {
		ERR("Out of memory!\n");
		fclose(fp);
		return -1;
	}

	if (fread(finfo->file_data, 1, fs, fp) != fs) {
		ERR("Error reading file %s\n", finfo->file_name);
		fclose(fp);
		return -1;
	}

	DBG("File: read successful\n");
	fclose(fp);

	return 0;
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

static void fi_clean(struct file_info* finfo) {
	if (!finfo)
		return;

	if (finfo->file_name) {
		finfo->file_name = NULL;
	}

	if (finfo->file_data) {
		free(finfo->file_data);
		finfo->file_data = NULL;
	}

	finfo->file_size = 0;
}

static void usage(char* argv[]) {
    printf("Usage: %s <sysupgradefile> <kernel_offset> <HWID> <HWVER> <SWID>\n"
           "All are positional arguments ...	\n"
           "	sysupgradefile:     File with the kernel uimage at 0\n"
           "	kernel_offset:      Offset where the kernel is located (decimal, hex or octal notation)\n"
           "	HWID:               Hardware ID, ASCII\n"
           "	HWVER:              Hardware Version, ASCII\n"
           "	SWID:               Software Version (decimal, hex or octal notation)\n"
           "	\n"
           , argv[0]);
}

int main(int argc, char* argv[]) {
	int ret = 1;
	int rootfsname_sz;
	int zipfsname_sz;
	int zipcmd_sz;
	u_int32_t kernel_offset = 0x90000; /* offset for the kernel inside the rootfs, default val */
	u_int32_t swVer = 0;
	struct file_info sysupgrade = { 0 };
	struct file_info header = { 0 };
	struct file_info rootfs = { 0 };
	struct file_info zippedfs = { 0 };
	struct file_info image = { 0 };
	char* hwID = NULL;
	char* hwVer = NULL;
	char* rootfsname = NULL;
	char* zipfsname = NULL;
	char* zipcmd = NULL;
	u_int8_t chkSum;

	if (argc == 2) {
		struct file_info myfile = { argv[1], 0, 0 };

		if (bufferFile(&myfile))
			return 1;

		chkSum = getCheckSum(myfile.file_data, myfile.file_size);
		printf("Checksum for File: 0x%hhX\n", chkSum);

		return 0;
	}

	if (argc != 6) {
		usage(argv);
		return 1;
	}

	printf("Building fw image for sercomm devices ..\n");

	/* process args */
	hwID = argv[3];
	hwVer = argv[4];

	sysupgrade.file_name = argv[1];
	image.file_name = argv[1];
	kernel_offset = (u_int32_t) strtol(argv[2], NULL, 0);
	swVer = (u_int32_t) strtol(argv[5], NULL, 0);
	swVer = cpu_to_be32(swVer);

	/* Check if files actually exist */
	if (access(sysupgrade.file_name, (F_OK | R_OK))) {
		/* Error */
		ERR("File not found: %s\n", sysupgrade.file_name);
		goto cleanup;
	}

	/* Calculate amount of required memory (incl. 0-term) */
	rootfsname_sz = strlen(sysupgrade.file_name) + 7 + 1;
	zipfsname_sz = strlen(sysupgrade.file_name) + 7 + 4 + 1;

	/* Allocate required memory */
	if (!(rootfsname = (char*) malloc(rootfsname_sz)) || !(zipfsname =
			(char*) malloc(zipfsname_sz))) {
		/* Error */
		ERR("Out of memory!\n");
		goto cleanup;
	}

	/* Create filenames */
	if (snprintf(rootfsname, rootfsname_sz, "%s.rootfs", sysupgrade.file_name)
			>= rootfsname_sz
			|| snprintf(zipfsname, zipfsname_sz, "%s.rootfs.zip",
					sysupgrade.file_name) >= zipfsname_sz) {
		/* Error */
		ERR("Buffer too small!\n");
		goto cleanup;
	}

	/* Buffer all files */
	if (bufferFile(&sysupgrade)) {
		/* Error */
		goto cleanup;
	}

	DBG("Building header: %s %s %2X %s\n", hwID, hwVer, swVer, magic);

	/* Construct the firmware header/magic */
	header.file_name = NULL;
	header.file_size = HEADER_SIZE;

	if (!(header.file_data = (char*) calloc(1, HEADER_SIZE))) {
		/* Error */
		ERR("Out of memory!\n");
		goto cleanup;
	}

	strncpy(header.file_data + 0, magic, 7);
	memcpy(header.file_data + 7, version, sizeof(version));
	strncpy(header.file_data + 11, hwID, 34);
	strncpy(header.file_data + 45, hwVer, 10);
	memcpy(header.file_data + 55, &swVer, sizeof(swVer));
	strncpy(header.file_data + 63, magic, 7);

	DBG("Creating rootfs ..\n");

	/* Construct a rootfs */
	rootfs.file_name = rootfsname;
	rootfs.file_size = ALIGN(
			sysupgrade.file_size + kernel_offset + header.file_size,
			ROOTFS_ALIGN);

	if (!(rootfs.file_data = calloc(1, rootfs.file_size))) {
		/* Error */
		ERR("Out of memory!\n");
		goto cleanup;
	}

	/* copy Owrt image to kernel location */
	memcpy(rootfs.file_data + kernel_offset, sysupgrade.file_data,
			sysupgrade.file_size);

	/* Append header after the owrt image.  The updater searches for it */
	memcpy(rootfs.file_data + kernel_offset + sysupgrade.file_size,
			header.file_data, header.file_size);

	/* Write to file */
	if (writeFile(&rootfs)) {
		/* Error */
		goto cleanup;
	}

	/* Construct a zip */
	DBG("Preparing to zip ..\n");

	/* now that we got the rootfs, repeat the whole thing again(sorta):
	 * 1. zip the rootfs */
	zipcmd_sz = 3 + 1 + strlen(zipfsname) + 1 + strlen(rootfs.file_name) + 1;

	if (!(zipcmd = malloc(zipcmd_sz))) {
		/* Error */
		ERR("Out of memory!\n");
		goto cleanup;
	}

	if (snprintf(zipcmd, zipcmd_sz, "%s %s %s", "zip", zipfsname,
			rootfs.file_name) >= zipcmd_sz) {
		/* Error */
		ERR("Buffer too small!\n");
		goto cleanup;
	}

	if (system(zipcmd)) {
		/* Error */
		ERR("Error creating a zip file!\n");
		goto cleanup;
	}

	/* and load zipped fs */
	zippedfs.file_name = zipfsname;

	if (bufferFile(&zippedfs)) {
		/* Error */
		goto cleanup;
	}

	DBG("Creating Image.\n");

	/* 2. create new file 512 + rootfs size */
	image.file_size = zippedfs.file_size + 512;
	if (!(image.file_data = malloc(zippedfs.file_size + 512))) {
		/* Error */
		ERR("Out of memory!\n");
		goto cleanup;
	}

	/* 3. add header to file */
	memcpy(image.file_data, header.file_data, header.file_size);

	/* 4. clear remaining space */
	if (header.file_size < 512)
		memset(image.file_data + header.file_size, 0, 512 - header.file_size);

	/* 5. copy zipfile at loc 512 */
	memcpy(image.file_data + 512, zippedfs.file_data, zippedfs.file_size);

	/* 6. do a checksum run, and compute checksum */
	chkSum = getCheckSum(image.file_data, image.file_size);

	DBG("Checksum for Image: %hhX\n", chkSum);

	/* 7. write the checksum inverted into byte 511 to bring it to 0 on verification */
	chkSum = (chkSum ^ 0xFF) + 1;
	image.file_data[511] = (char) chkSum;

	chkSum = getCheckSum(image.file_data, image.file_size);
	DBG("Checksum for after fix: %hhX\n", chkSum);

	if (chkSum != 0) {
		ERR("Invalid checksum!\n")
		goto cleanup;
	}

	/* 8. pray that the updater will accept the file */
	if (writeFile(&image)) {
		/* Error */
		goto cleanup;
	}

	/* All seems OK */
	ret = 0;

	cleanup:

	if (rootfs.file_name && !access(rootfs.file_name, F_OK | W_OK))
		remove(rootfs.file_name);

	if (zippedfs.file_name && !access(zippedfs.file_name, F_OK | W_OK))
		remove(zippedfs.file_name);

	fi_clean(&sysupgrade);
	fi_clean(&header);
	fi_clean(&rootfs);
	fi_clean(&zippedfs);
	fi_clean(&image);

	if (rootfsname)
		free(rootfsname);

	if (zipfsname)
		free(zipfsname);

	if (zipcmd)
		free(zipcmd);

	return ret;
}
