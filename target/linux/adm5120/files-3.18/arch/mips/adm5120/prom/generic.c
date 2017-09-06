/*
 *  Generic PROM routines
 *
 *  Copyright (C) 2007-2008 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/string.h>

#include <asm/bootinfo.h>

#include <prom/generic.h>

static int *_prom_argc;
static char **_prom_argv;
static char **_prom_envp;

char *generic_prom_getenv(char *envname)
{
	char **env;
	char *ret;

	ret = NULL;
	for (env = _prom_envp; *env != NULL; env++) {
		if (strcmp(envname, *env++) == 0) {
			ret = *env;
			break;
		}
	}

	return ret;
}

int generic_prom_present(void)
{
	_prom_argc = (int *)fw_arg0;
	_prom_argv = (char **)fw_arg1;
	_prom_envp = (char **)fw_arg2;

	return 1;
}
