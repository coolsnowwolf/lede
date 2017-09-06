/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License,
 * version 2 as published by the Free Software Foundation.
 *
 * (C) John Crispin <blogic@openwrt.org>
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>

int main(int argc, char **argv)
{
	uint32_t t = 0, sum = 0x55aa55aa;
	int fd;

	if ((argc != 2) || ((fd = open(argv[1], O_RDWR)) == -1)) {
		fprintf(stderr, "Usage: %s input_file\n", *argv);
		return -EINVAL;
	}

	lseek(fd, -4, SEEK_END);
	write(fd, &t, 4);
	lseek(fd, 0, SEEK_SET);

	while (read(fd, &t, 4) > 0)
		sum -= t;

	lseek(fd, -4, SEEK_END);
	write(fd, &sum, 4);
	close(fd);

	return 0;
}
