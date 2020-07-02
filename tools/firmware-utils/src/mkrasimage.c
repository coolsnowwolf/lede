/*
 * --- ZyXEL header format ---
 * Original Version by Benjamin Berg <benjamin@sipsolutions.net>
 * C implementation based on generation-script by Christian Lamparter <chunkeey@gmail.com>
 *
 * The firmware image prefixed with a header (which is written into the MTD device).
 * The header is one erase block (~64KiB) in size, but the checksum only convers the
 * first 2KiB. Padding is 0xff. All integers are in big-endian.
 *
 * The checksum is always a 16-Bit System V checksum (sum -s) stored in a 32-Bit integer.
 *
 *   4 bytes:  checksum of the rootfs image
 *   4 bytes:  length of the contained rootfs image file (big endian)
 *  32 bytes:  Firmware Version string (NUL terminated, 0xff padded)
 *   4 bytes:  checksum over the header partition (big endian - see below)
 *  64 bytes:  Model (e.g. "NBG6617", NUL termiated, 0xff padded)
 *   4 bytes:  checksum of the kernel partition
 *   4 bytes:  length of the contained kernel image file (big endian)
 *      rest:  0xff padding (To erase block size)
 *
 * The kernel partition checksum and length is not used for every device.
 * If it's notused, pad those 8 bytes with 0xFF.
 *
 * The checksums are calculated by adding up all bytes and if a 16bit
 * overflow occurs, one is added and the sum is masked to 16 bit:
 *   csum = csum + databyte; if (csum > 0xffff) { csum += 1; csum &= 0xffff };
 * Should the file have an odd number of bytes then the byte len-0x800 is
 * used additionally.
 *
 * The checksum for the header is calculated over the first 2048 bytes with
 * the rootfs image checksum as the placeholder during calculation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 *
 */
#include <fcntl.h>
#include <getopt.h>
#include <libgen.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/mman.h>
#include <sys/stat.h>

#include <arpa/inet.h>

#define VERSION_STRING_LEN 31
#define ROOTFS_HEADER_LEN 40

#define KERNEL_HEADER_LEN 8

#define BOARD_NAME_LEN 64
#define BOARD_HEADER_LEN 68

#define HEADER_PARTITION_CALC_LENGTH 2048
#define HEADER_PARTITION_LENGTH 0x10000

struct file_info {
    char *name;    /* name of the file */
    char *data;    /* file content */
    size_t size;   /* length of the file */
};

static char *progname;

static char *board_name = 0;
static char *version_name = 0;
static unsigned int rootfs_size = 0;
static unsigned int header_length = HEADER_PARTITION_LENGTH;

static struct file_info kernel = { NULL, NULL, 0 };
static struct file_info rootfs = { NULL, NULL, 0 };
static struct file_info rootfs_out = { NULL, NULL, 0 };
static struct file_info out = { NULL, NULL, 0 };

#define ERR(fmt, ...) do { \
    fprintf(stderr, "[%s] *** error: " fmt "\n", \
            progname, ## __VA_ARGS__ ); \
} while (0)

void map_file(struct file_info *finfo)
{
    struct stat file_stat = {0};
    int fd;

    fd = open(finfo->name, O_RDONLY, (mode_t)0600);
    if (fd == -1) {
        ERR("Error while opening file %s.", finfo->name);
        exit(EXIT_FAILURE);
    }

    if (fstat(fd, &file_stat) == -1) {
        ERR("Error getting file size for %s.", finfo->name);
        exit(EXIT_FAILURE);
    }

    finfo->size = file_stat.st_size;
    finfo->data = mmap(0, finfo->size, PROT_READ, MAP_SHARED, fd, 0);

    if (finfo->data == MAP_FAILED) {
        ERR("Error mapping file %s.", finfo->name);
        exit(EXIT_FAILURE);
    }

    close(fd);
}

void unmap_file(struct file_info *finfo)
{
    if(munmap(finfo->data, finfo->size) == -1) {
        ERR("Error unmapping file %s.", finfo->name);
        exit(EXIT_FAILURE);
    }
}

void write_file(struct file_info *finfo)
{
    FILE *fout = fopen(finfo->name, "w");

    fwrite(finfo->data, finfo->size, 1, fout);

    if (ferror(fout)) {
        ERR("Wanted to write, but something went wrong.");
        exit(EXIT_FAILURE);
    }

    fclose(fout);
}

void usage(int status)
{
    FILE *stream = (status != EXIT_SUCCESS) ? stderr : stdout;

    fprintf(stream, "Usage: %s [OPTIONS...]\n", progname);
    fprintf(stream,
            "\n"
            "Options:\n"
            "  -k <kernel>     path for kernel image\n"
            "  -r <rootfs>     path for rootfs image\n"
            "  -s <rfssize>    size of output rootfs\n"
            "  -v <version>    version string\n"
            "  -b <boardname>  name of board to generate image for\n"
            "  -o <out_name>   name of output image\n"
            "  -l <hdr_length> length of header, default 65536\n"
            "  -h              show this screen\n"
    );

    exit(status);
}

static int sysv_chksm(const unsigned char *data, int size)
{
    int r;
    int checksum;
    unsigned int s = 0; /* The sum of all the input bytes, modulo (UINT_MAX + 1).  */


    for (int i = 0; i < size; i++) {
        s += data[i];
    }

    r = (s & 0xffff) + ((s & 0xffffffff) >> 16);
    checksum = (r & 0xffff) + (r >> 16);

    return checksum;
}

static int zyxel_chksm(const unsigned char *data, int size)
{
     return htonl(sysv_chksm(data, size));
}

char *generate_rootfs_header(struct file_info filesystem, char *version)
{
    size_t version_string_length;
    unsigned int chksm, size;
    char *rootfs_header;
    size_t ptr = 0;

    rootfs_header = malloc(ROOTFS_HEADER_LEN);
    if (!rootfs_header) {
        ERR("Couldn't allocate memory for rootfs header!");
        exit(EXIT_FAILURE);
    }

    /* Prepare padding for firmware-version string here */
    memset(rootfs_header, 0xff, ROOTFS_HEADER_LEN);

    chksm = zyxel_chksm((const unsigned char *)filesystem.data, filesystem.size);
    size = htonl(filesystem.size);

    /* 4 bytes:  checksum of the rootfs image */
    memcpy(rootfs_header + ptr, &chksm, 4);
    ptr += 4;

    /* 4 bytes:  length of the contained rootfs image file (big endian) */
    memcpy(rootfs_header + ptr, &size, 4);
    ptr += 4;

    /* 32 bytes:  Firmware Version string (NUL terminated, 0xff padded) */
    version_string_length = strlen(version) <= VERSION_STRING_LEN ? strlen(version) : VERSION_STRING_LEN;
    memcpy(rootfs_header + ptr, version, version_string_length);
    ptr += version_string_length;
    /* Add null-terminator */
    rootfs_header[ptr] = 0x0;

    return rootfs_header;
}

char *generate_kernel_header(struct file_info kernel)
{
    unsigned int chksm, size;
    char *kernel_header;
    size_t ptr = 0;

    kernel_header = malloc(KERNEL_HEADER_LEN);
    if (!kernel_header) {
        ERR("Couldn't allocate memory for kernel header!");
        exit(EXIT_FAILURE);
    }

    chksm = zyxel_chksm((const unsigned char *)kernel.data, kernel.size);
    size = htonl(kernel.size);

    /* 4 bytes:  checksum of the kernel image */
    memcpy(kernel_header + ptr, &chksm, 4);
    ptr += 4;

    /* 4 bytes:  length of the contained kernel image file (big endian) */
    memcpy(kernel_header + ptr, &size, 4);

    return kernel_header;
}

unsigned int generate_board_header_checksum(char *kernel_hdr, char *rootfs_hdr, char *boardname)
{
    char *board_hdr_tmp;
    unsigned int sum;
    size_t ptr = 0;

    /*
     * The checksum of the board header is calculated over the first 2048 bytes of
     * the header partition with the rootfs checksum used as a placeholder for then
     * board checksum we calculate in this step. The checksum gained from this step
     * is then used for the final board header partition.
     */

    board_hdr_tmp = malloc(HEADER_PARTITION_CALC_LENGTH);
    if (!board_hdr_tmp) {
        ERR("Couldn't allocate memory for temporary board header!");
        exit(EXIT_FAILURE);
    }
    memset(board_hdr_tmp, 0xff, HEADER_PARTITION_CALC_LENGTH);

    /* 40 bytes:  RootFS header */
    memcpy(board_hdr_tmp, rootfs_hdr, ROOTFS_HEADER_LEN);
    ptr += ROOTFS_HEADER_LEN;

    /* 4 bytes:  RootFS checksum (BE) as placeholder for board-header checksum */
    memcpy(board_hdr_tmp + ptr, rootfs_hdr, 4);
    ptr += 4;

    /* 32 bytes:  Model (e.g. "NBG6617", NUL termiated, 0xff padded) */
    memcpy(board_hdr_tmp + ptr, boardname, strlen(boardname));
    ptr += strlen(boardname);
    /* Add null-terminator */
    board_hdr_tmp[ptr] = 0x0;
    ptr = ROOTFS_HEADER_LEN + 4 + BOARD_NAME_LEN;

    /* 8 bytes:  Kernel header */
    if (kernel_hdr)
        memcpy(board_hdr_tmp + ptr, kernel_hdr, 8);

    /* Calculate the checksum over the first 2048 bytes */
    sum = zyxel_chksm((const unsigned char *)board_hdr_tmp, HEADER_PARTITION_CALC_LENGTH);
    free(board_hdr_tmp);
    return sum;
}

char *generate_board_header(char *kernel_hdr, char *rootfs_hdr, char *boardname)
{
    unsigned int board_checksum;
    char *board_hdr;

    board_hdr = malloc(BOARD_HEADER_LEN);
    if (!board_hdr) {
        ERR("Couldn't allocate memory for board header!");
        exit(EXIT_FAILURE);
    }
    memset(board_hdr, 0xff, BOARD_HEADER_LEN);

    /* 4 bytes:  checksum over the header partition (big endian) */
    board_checksum = generate_board_header_checksum(kernel_hdr, rootfs_hdr, boardname);
    memcpy(board_hdr, &board_checksum, 4);

    /* 32 bytes:  Model (e.g. "NBG6617", NUL termiated, 0xff padded) */
    memcpy(board_hdr + 4, boardname, strlen(boardname));
    board_hdr[4 + strlen(boardname)] = 0x0;

    return board_hdr;
}

int build_image()
{
    char *rootfs_header = NULL;
    char *kernel_header = NULL;
    char *board_header = NULL;

    size_t ptr;

    /* Load files */
    if (kernel.name)
        map_file(&kernel);
    map_file(&rootfs);

    /* As ZyXEL Web-GUI only accept images with a rootfs equal or larger than the first firmware shipped
     * for the device, we need to pad rootfs partition to this size. To perform further calculations, we
     * decide the size of this part here. In case the rootfs we want to integrate in our image is larger,
     * take it's size, otherwise the supplied size.
     *
     * Be careful! We rely on assertion of correct size to be performed beforehand. It is unknown if images
     * with a to large rootfs are accepted or not.
     */
    rootfs_out.size = rootfs_size < rootfs.size ? rootfs.size : rootfs_size;

    /*
     * Allocate memory and copy input rootfs for temporary output rootfs.
     * This is important as we have to generate the rootfs checksum over the
     * entire rootfs partition. As we might have to pad the partition to allow
     * for flashing via ZyXEL's Web-GUI, we prepare the rootfs partition for the
     * output image here (and also use it for calculating the rootfs checksum).
     *
     * The roofs padding has to be done with 0x00.
     */
    rootfs_out.data = calloc(rootfs_out.size, sizeof(char));
    memcpy(rootfs_out.data, rootfs.data, rootfs.size);

    /* Prepare headers */
    rootfs_header = generate_rootfs_header(rootfs_out, version_name);
    if (kernel.name)
        kernel_header = generate_kernel_header(kernel);
    board_header = generate_board_header(kernel_header, rootfs_header, board_name);

    /* Prepare output file */
    out.size = header_length + rootfs_out.size;
    if (kernel.name)
        out.size += kernel.size;
    out.data = malloc(out.size);
    memset(out.data, 0xFF, out.size);

    /* Build output image */
    memcpy(out.data, rootfs_header, ROOTFS_HEADER_LEN);
    memcpy(out.data + ROOTFS_HEADER_LEN, board_header, BOARD_HEADER_LEN);
    if (kernel.name)
        memcpy(out.data + ROOTFS_HEADER_LEN + BOARD_HEADER_LEN, kernel_header, KERNEL_HEADER_LEN);
    ptr = header_length;
    memcpy(out.data + ptr, rootfs_out.data, rootfs_out.size);
    ptr += rootfs_out.size;
    if (kernel.name)
        memcpy(out.data + ptr, kernel.data, kernel.size);

    /* Write back output image */
    write_file(&out);

    /* Free allocated memory */
    if (kernel.name)
        unmap_file(&kernel);
    unmap_file(&rootfs);
    free(out.data);
    free(rootfs_out.data);

    free(rootfs_header);
    if (kernel.name)
        free(kernel_header);
    free(board_header);

    return 0;
}

int check_options()
{
    if (!rootfs.name) {
        ERR("No rootfs filename supplied");
        return -2;
    }

    if (!out.name) {
        ERR("No output filename supplied");
        return -3;
    }

    if (!board_name) {
        ERR("No board-name supplied");
        return -4;
    }

    if (!version_name) {
        ERR("No version supplied");
        return -5;
    }

    if (rootfs_size <= 0) {
        ERR("Invalid rootfs size supplied");
        return -6;
    }

    if (strlen(board_name) > 31) {
        ERR("Board name is to long");
        return -7;
    }
    return 0;
}

int main(int argc, char *argv[])
{
    int ret;
    progname = basename(argv[0]);
    while (1) {
        int c;

        c = getopt(argc, argv, "b:k:o:r:s:v:l:h");
        if (c == -1)
            break;

        switch (c) {
            case 'b':
                board_name = optarg;
                break;
            case 'h':
                usage(EXIT_SUCCESS);
                break;
            case 'k':
                kernel.name = optarg;
                break;
            case 'o':
                out.name = optarg;
                break;
            case 'r':
                rootfs.name = optarg;
                break;
            case 's':
                sscanf(optarg, "%u", &rootfs_size);
                break;
            case 'v':
                version_name = optarg;
                break;
            case 'l':
                sscanf(optarg, "%u", &header_length);
                break;
            default:
                usage(EXIT_FAILURE);
                break;
        }
    }

    ret = check_options();
    if (ret)
        usage(EXIT_FAILURE);

    return build_image();
}
