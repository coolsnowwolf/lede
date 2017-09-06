/*
 * mkbrncmdline.c - partially based on OpenWrt's wndr3700.c
 *
 * Copyright (C) 2011 Tobias Diedrich <ranma+openwrt@tdiedrich.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License,
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <netinet/in.h>
#include <inttypes.h>

static void usage(const char *) __attribute__ (( __noreturn__ ));

static void usage(const char *mess)
{
	fprintf(stderr, "Error: %s\n", mess);
	fprintf(stderr, "Usage: mkbrncmdline -i input_file -o output_file [-a loadaddress] arg1 [argx ...]\n");
	fprintf(stderr, "\n");
	exit(1);
}

static char *input_file = NULL;
static char *output_file = NULL;
static unsigned loadaddr = 0x80002000;

static void parseopts(int *argc, char ***argv)
{
	char *endptr;
	int res;

	while ((res = getopt(*argc, *argv, "a:i:o:")) != -1) {
		switch (res) {
		default:
			usage("Unknown option");
			break;
		case 'a':
			loadaddr = strtoul(optarg, &endptr, 0);
			if (endptr == optarg || *endptr != 0)
				usage("loadaddress must be a decimal or hexadecimal 32-bit value");
			break;
		case 'i':
			input_file = optarg;
			break;
		case 'o':
			output_file = optarg;
			break;
		}
	}
	*argc -= optind;
	*argv += optind;
}

static void emitload(int outfd, int reg, unsigned value)
{
	char buf[8] = {
		0x3c, 0x04 + reg,
		value >> 24, value >> 16,
		0x34, 0x84 + reg + (reg << 5),
		value >> 8, value,
	};
	if (write(outfd, buf, sizeof(buf)) != sizeof(buf)) {
		fprintf(stderr, "write: %s\n", strerror(errno));
		exit(1);
	}
}

int main(int argc, char **argv)
{
	int outfd;
	int i;
	int fd;
	size_t len, skip, buf_len;
	unsigned cmdline_addr;
	unsigned s_ofs;
	char *buf;

	parseopts(&argc, &argv);

	if (argc < 1)
		usage("must specify at least one kernel cmdline argument");

	if (input_file == NULL || output_file == NULL)
		usage("must specify input and output file");

	if ((outfd = open(output_file, O_WRONLY|O_CREAT|O_TRUNC, 0644)) == -1)
	{
		fprintf(stderr, "Error opening '%s' for writing: %s\n", output_file, strerror(errno));
		exit(1);
	}

	// mmap input_file
	if ((fd = open(input_file, O_RDONLY))  < 0
	|| (len = lseek(fd, 0, SEEK_END)) < 0
	|| (input_file = mmap(0, len, PROT_READ, MAP_SHARED, fd, 0)) == (void *) (-1)
	|| close(fd) < 0)
	{
		fprintf(stderr, "Error mapping file '%s': %s\n", input_file, strerror(errno));
		exit(1);
	}

	cmdline_addr = loadaddr + len;

	// Kernel args are passed in registers a0,a1,a2 and a3
	emitload(outfd, 0, 0);              /* a0 = 0 */
	emitload(outfd, 1, 0);              /* a1 = 0 */
	emitload(outfd, 2, cmdline_addr);   /* a2 = &cmdline */
	emitload(outfd, 3, 0);              /* a3 = 0 */
	skip = lseek(outfd, 0, SEEK_END);

	// write the kernel
	if (write(outfd, input_file + skip, len - skip) != len -skip) {
		fprintf(stderr, "write: %s\n", strerror(errno));
		exit(1);
	}

	// write cmdline structure
	buf_len = (argc + 1) * 4;
	for (i=0; i<argc; i++) {
		buf_len += strlen(argv[i]) + 1;
	}
	buf = malloc(buf_len + 16);
	if (!buf) {
		fprintf(stderr, "Could not allocate memory for cmdline buffer\n");
		exit(1);
	}
	memset(buf, 0, buf_len);

	s_ofs = 4 * (argc + 1);
	for (i=0; i<argc; i++) {
		unsigned s_ptr = cmdline_addr + s_ofs;
		buf[i * 4 + 0] = s_ptr >> 24;
		buf[i * 4 + 1] = s_ptr >> 16;
		buf[i * 4 + 2] = s_ptr >>  8;
		buf[i * 4 + 3] = s_ptr >>  0;
		memcpy(&buf[s_ofs], argv[i], strlen(argv[i]));
		s_ofs += strlen(argv[i]) + 1;
	}
	if (write(outfd, buf, buf_len) != buf_len) {
		fprintf(stderr, "write: %s\n", strerror(errno));
		exit(1);
	}


	munmap(input_file, len);
	close(outfd);
	free(buf);

	return 0;
}
