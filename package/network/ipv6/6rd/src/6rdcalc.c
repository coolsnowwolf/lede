/*
 * Utility used to calculate the 6rd subnet.
 *
 * Copyright 2012, Stéphan Kochen <stephan@kochen.nl>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define INET_PREFIXSTRLEN (INET_ADDRSTRLEN+3)
#define INET6_PREFIXSTRLEN (INET6_ADDRSTRLEN+4)

static void print_usage()
{
	fprintf(stderr, "Usage: 6rdcalc <v6 prefix>/<mask> <v4 address>/<mask>\n");
	exit(1);
}

static void print_error()
{
	fprintf(stderr, "%s", strerror(errno));
	exit(1);
}

static void parse_str(int af, char *str, void *addr, unsigned long *mask)
{
	int ret;
	char *slash;

	/* Split the address at the slash. */
	if ((slash = strchr(str, '/')) == NULL)
		print_usage();
	*slash = '\0';

	/* Parse the address. */
	if ((ret = inet_pton(af, str, addr)) != 1) {
		if (ret == 0)
			print_usage();
		else
			print_error();
	}

	/* Parse the mask. */
	*mask = strtoul(slash+1, NULL, 10);
	if ((af == AF_INET  && *mask >  32) ||
		(af == AF_INET6 && *mask > 128))
		print_usage();
}

int main(int argc, const char **argv)
{
	char v6str[INET6_PREFIXSTRLEN], v4str[INET_PREFIXSTRLEN];
	struct in6_addr v6;
	struct in_addr v4;
	unsigned long v6it, v4it, mask;
	unsigned char *byte4, *byte6;
	unsigned char bit4, bit6;

	/* Check parameters. */
	if (argc != 3)
		print_usage();

	/* Parse the v6 address. */
	strncpy(v6str, argv[1], INET6_PREFIXSTRLEN);
	v6str[INET6_PREFIXSTRLEN-1] = '\0';
	parse_str(AF_INET6, v6str, &v6, &v6it);

	/* Parse the v4 address */
	strncpy(v4str, argv[2], INET_PREFIXSTRLEN);
	v6str[INET_PREFIXSTRLEN-1] = '\0';
	parse_str(AF_INET, v4str, &v4, &v4it);

	/* Check if the combined mask is within bounds. */
	mask = (32 - v4it) + v6it;
	if (mask > 128)
		print_usage();

	/* Combine the addresses. */
	while (v4it < 32) {
		byte6 = (unsigned char *)(&v6.s6_addr) + (v6it >> 3);
		byte4 = (unsigned char *)(&v4.s_addr)  + (v4it >> 3);
		bit6 = 128 >> (v6it & 0x07);
		bit4 = 128 >> (v4it & 0x07);

		if (*byte4 & bit4)
			*byte6 |= bit6;
		else
			*byte6 &= ~bit6;

		v4it++; v6it++;
	}

	/* Clear remaining bits. */
	while (v6it < 128) {
		byte6 = (unsigned char *)(&v6.s6_addr) + (v6it >> 3);
		bit6 = 128 >> (v6it & 0x07);

		*byte6 &= ~bit6;

		v6it++;
	}

	/* Print the subnet prefix. */
	if (inet_ntop(AF_INET6, &v6, v6str, sizeof(v6str)) == NULL)
		print_error();
	printf("%s/%lu\n", v6str, mask);
	return 0;
}
