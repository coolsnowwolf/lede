/*
 * MAC address manupulation utility
 *
 * Copyright (C) 2011 Gabor Juhos <juhosg@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 *
 */

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#define MAC_ADDRESS_LEN		6

#define ERR_INVALID		1
#define ERR_IO			2

static void usage(void);

char *maccalc_name;

static int parse_mac(const char *mac_str, unsigned char *buf)
{
	int t;

	t = sscanf(mac_str, "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx",
		   &buf[0], &buf[1], &buf[2], &buf[3], &buf[4], &buf[5]);

	if (t != MAC_ADDRESS_LEN)
		return ERR_INVALID;

	return 0;
}

static void print_mac(unsigned char *buf)
{
	printf("%02x:%02x:%02x:%02x:%02x:%02x\n",
	       buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
}

static int maccalc_do_add(int argc, const char *argv[])
{
	unsigned char mac[MAC_ADDRESS_LEN];
	uint32_t t;
	int err;
	int i;

	if (argc != 2) {
		usage();
		return ERR_INVALID;
	}

	err = parse_mac(argv[0], mac);
	if (err)
		return err;

	i = atoi(argv[1]);

	t = (mac[3] << 16) | (mac[4] << 8) | mac[5];
	t += i;
	mac[3] = (t >> 16) & 0xff;
	mac[4] = (t >> 8) & 0xff;
	mac[5] = t & 0xff;

	print_mac(mac);
	return 0;
}

static int maccalc_do_logical(int argc, const char *argv[],
			      unsigned char (*op)(unsigned char n1,
						  unsigned char n2))
{
	unsigned char mac1[MAC_ADDRESS_LEN];
	unsigned char mac2[MAC_ADDRESS_LEN];
	int err;
	int i;

	if (argc != 2) {
		usage();
		return ERR_INVALID;
	}

	err = parse_mac(argv[0], mac1);
	if (err)
		return err;

	err = parse_mac(argv[1], mac2);
	if (err)
		return err;

	for (i = 0; i < MAC_ADDRESS_LEN; i++)
		mac1[i] = op(mac1[i],mac2[i]);

	print_mac(mac1);
	return 0;
}

static int maccalc_do_mac2bin(int argc, const char *argv[])
{
	unsigned char mac[MAC_ADDRESS_LEN];
	ssize_t c;
	int err;

	if (argc != 1) {
		usage();
		return ERR_INVALID;
	}

	err = parse_mac(argv[0], mac);
	if (err)
		return err;

	c = write(STDOUT_FILENO, mac, sizeof(mac));
	if (c != sizeof(mac)) {
		fprintf(stderr, "failed to write to stdout\n");
		return ERR_IO;
	}

	return 0;
}

static ssize_t read_safe(int fd, void *buf, size_t count)
{
	ssize_t total = 0;
	ssize_t r;

	while(count > 0) {
		r = read(fd, buf, count);
		if (r == 0)
			/* EOF */
			break;
		if (r < 0) {
			if (errno == EINTR)
				/* interrupted by a signal, restart */
				continue;
			/* error */
			total = -1;
			break;
		}

		/* ok */
		total += r;
		count -= r;
		buf += r;
	}

	return total;
}

static int maccalc_do_bin2mac(int argc, const char *argv[])
{
	unsigned char mac[MAC_ADDRESS_LEN];
	ssize_t c;

	if (argc != 0) {
		usage();
		return ERR_INVALID;
	}

	c = read_safe(STDIN_FILENO, mac, sizeof(mac));
	if (c != sizeof(mac)) {
		fprintf(stderr, "failed to read from stdin\n");
		return ERR_IO;
	}

	print_mac(mac);
	return 0;
}

static unsigned char op_or(unsigned char n1, unsigned char n2)
{
	return n1 | n2;
}

static int maccalc_do_or(int argc, const char *argv[])
{
	return maccalc_do_logical(argc, argv, op_or);
}

static unsigned char op_and(unsigned char n1, unsigned char n2)
{
	return n1 & n2;
}

static int maccalc_do_and(int argc, const char *argv[])
{
	return maccalc_do_logical(argc, argv, op_and);
}

static unsigned char op_xor(unsigned char n1, unsigned char n2)
{
	return n1 ^ n2;
}

static int maccalc_do_xor(int argc, const char *argv[])
{
	return maccalc_do_logical(argc, argv, op_xor);
}

static void usage(void)
{
	fprintf(stderr,
		"Usage: %s <command>\n"
		"valid commands:\n"
		"  add <mac> <number>\n"
		"  and|or|xor <mac1> <mac2>\n"
		"  mac2bin <mac>\n"
		"  bin2mac\n",
		maccalc_name);
}

int main(int argc, const char *argv[])
{
	int (*op)(int argc, const char *argv[]);
	int ret;

	maccalc_name = (char *) argv[0];

	if (argc < 2) {
		usage();
		return EXIT_FAILURE;
	}

	if (strcmp(argv[1], "add") == 0) {
		op = maccalc_do_add;
	} else if (strcmp(argv[1], "and") == 0) {
		op = maccalc_do_and;
	} else if (strcmp(argv[1], "or") == 0) {
		op = maccalc_do_or;
	} else if (strcmp(argv[1], "xor") == 0) {
		op = maccalc_do_xor;
	} else if (strcmp(argv[1], "mac2bin") == 0) {
		op = maccalc_do_mac2bin;
	} else if (strcmp(argv[1], "bin2mac") == 0) {
		op = maccalc_do_bin2mac;
	} else {
		fprintf(stderr, "unknown command '%s'\n", argv[1]);
		usage();
		return EXIT_FAILURE;
	}

	argc -= 2;
	argv += 2;

	ret = op(argc, argv);
	if (ret)
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}
