/*
 * Small pcap precompiler
 * Copyright (C) 2008 Felix Fietkau <nbd@nbd.name>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <sys/types.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>
#include <pcap.h>

int main (int argc, char ** argv)
{
	struct bpf_program filter;
	pcap_t *pc;
	int i;

	if (argc != 2)
	{
		printf ("Usage: %s <expression>\n", argv[0]);
		return 1;
	}

	pc = pcap_open_dead(DLT_EN10MB, 1500);
	if (pcap_compile(pc, &filter, argv[1], 1, 0) != 0) {
		printf("error in active-filter expression: %s\n", pcap_geterr(pc));
		return 1;
	}

	printf("/* precompiled expression: %s */\n\n"
		"static struct bpf_insn pktfilter_insns[] = {\n",
		argv[1]);

	for (i = 0; i < filter.bf_len; i++) {
		struct bpf_insn *in = &filter.bf_insns[i];
		printf("\t{ .code = 0x%04x, .jt = 0x%02x, .jf = 0x%02x, .k = 0x%08x },\n", in->code, in->jt, in->jf, in->k);
	}
	printf("};\n\n"
		"static struct bpf_program pktfilter = {\n"
		"\t.bf_len = %d,\n"
		"\t.bf_insns = pktfilter_insns,\n"
		"};\n", filter.bf_len);
	return 0;

}
