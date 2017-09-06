/*
 * Copyright (C) 2010 Scott Nicholas <neutronscott@scottn.us>
 *
 * based on work of rb532 prom.c
 *  Copyright (C) 2003, Peter Sadik <peter.sadik@idt.com>
 *  Copyright (C) 2005-2006, P.Christeas <p_christ@hol.gr>
 *  Copyright (C) 2007, Gabor Juhos <juhosg@openwrt.org>
 *			Felix Fietkau <nbd@nbd.name>
 *			Florian Fainelli <florian@openwrt.org>
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/console.h>
#include <linux/string.h>
#include <linux/serial_core.h>
#include <asm/bootinfo.h>
#include <adm8668.h>
#include "u-boot.h"

register volatile struct global_data *gd asm ("k0");

void __init prom_free_prom_memory(void)
{
	/* No prom memory to free */
}

static inline int match_tag(char *arg, const char *tag)
{
	return strncmp(arg, tag, strlen(tag)) == 0;
}

static inline unsigned long tag2ul(char *arg, const char *tag)
{
	char *num;

	num = arg + strlen(tag);
	return simple_strtoul(num, 0, 10);
}

void __init prom_setup_cmdline(void)
{
	char *cp;
	int prom_argc;
	char **prom_argv;
	int i;

	prom_argc = fw_arg0;
	prom_argv = (char **)KSEG0ADDR(fw_arg1);

	cp = &(arcs_cmdline[0]);
	for (i = 1; i < prom_argc; i++) {
		prom_argv[i] = (char *)KSEG0ADDR(prom_argv[i]);

		/* default bootargs has "console=/dev/ttyS0" yet console won't
		 * show up at all if you include the '/dev/' nowadays ... */
		if (match_tag(prom_argv[i], "console=/dev/")) {
			char *ptr = prom_argv[i] + strlen("console=/dev/");

			strcpy(cp, "console=");
			cp += strlen("console=");
			strcpy(cp, ptr);
			cp += strlen(ptr);
			*cp++ = ' ';
			continue;
		}
		strcpy(cp, prom_argv[i]);
		cp += strlen(prom_argv[i]);
		*cp++ = ' ';
	}
	if (prom_argc > 1)
		--cp; /* trailing space */

	*cp = '\0';
}

void __init prom_init(void)
{
	bd_t *bd = gd->bd;
	int memsize;

	memsize = bd->bi_memsize;
	printk("Board info:\n");
	printk("  RAM size: %d MB\n", (int)memsize/(1024*1024));
	printk("  NOR start: %#lx\n", bd->bi_flashstart);
	printk("  NOR size: %#lx\n", bd->bi_flashsize);

	prom_setup_cmdline();
	add_memory_region(0, memsize, BOOT_MEM_RAM);
}
