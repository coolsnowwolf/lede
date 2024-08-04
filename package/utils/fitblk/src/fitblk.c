// SPDX-License-Identifier: GPL-2.0-only
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/fitblk.h>

static int fitblk_release(char *device)
{
	int fd, ret;

	fd = open(device, O_RDONLY);
	if (fd == -1)
		return errno;

	ret = ioctl(fd, FITBLK_RELEASE, NULL);
	close(fd);

	if (ret == -1)
		return errno;

	return 0;
}

int main(int argc, char *argp[])
{
	int ret;

	if (argc != 2) {
		fprintf(stderr, "Release uImage.FIT sub-image block device\n");
		fprintf(stderr, "Syntax: %s /dev/fitXXX\n", argp[0]);
		return -EINVAL;
	}

	ret = fitblk_release(argp[1]);
	if (ret)
		fprintf(stderr, "fitblk: error releasing %s: %s\n", argp[1],
			strerror(ret));
	else
		fprintf(stderr, "fitblk: %s released\n", argp[1]);

	return ret;
}
