/*
 * (c) Copyright 2011 by Tigris Elektronik GmbH
 *
 * Author:
 *  Maximilian Schwerin <mvs@tigris.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>

#include <command.h>
#include <environment.h>
#include <linux/stddef.h>
#include <malloc.h>
#include <search.h>
#include <errno.h>
#include <ext4fs.h>

char *env_name_spec = "EXT4";

env_t *env_ptr;

DECLARE_GLOBAL_DATA_PTR;

int env_init(void)
{
	/* use default */
	gd->env_addr = (ulong)&default_environment[0];
	gd->env_valid = 1;

	return 0;
}

#ifdef CONFIG_CMD_SAVEENV
int saveenv(void)
{
	env_t	env_new;
	ssize_t	len;
	char	*res;
	block_dev_desc_t *dev_desc = NULL;
	int dev = EXT4_ENV_DEVICE;
	int part = EXT4_ENV_PART;
	int err;

	res = (char *)&env_new.data;
	len = hexport_r(&env_htab, '\0', 0, &res, ENV_SIZE, 0, NULL);
	if (len < 0) {
		error("Cannot export environment: errno = %d\n", errno);
		return 1;
	}

	dev_desc = get_dev(EXT4_ENV_INTERFACE, dev);
	if (dev_desc == NULL) {
		printf("Failed to find %s%d\n",
			EXT4_ENV_INTERFACE, dev);
		return 1;
	}

	err = ext4_register_device(dev_desc, part);
	if (err) {
		printf("Failed to register %s%d:%d\n",
			EXT4_ENV_INTERFACE, dev, part);
		return 1;
	}

	env_new.crc = crc32(0, env_new.data, ENV_SIZE);
	err = ext4fs_write(EXT4_ENV_FILE, (void *)&env_new, sizeof(env_t));
	ext4fs_close();
	if (err == -1) {
		printf("\n** Unable to write \"%s\" from %s%d:%d **\n",
			EXT4_ENV_FILE, EXT4_ENV_INTERFACE, dev, part);
		return 1;
	}

	puts("done\n");
	return 0;
}
#endif /* CONFIG_CMD_SAVEENV */

void env_relocate_spec(void)
{
	char buf[CONFIG_ENV_SIZE];
	block_dev_desc_t *dev_desc = NULL;
	int dev = EXT4_ENV_DEVICE;
	int part = EXT4_ENV_PART;
	int err;

	dev_desc = get_dev(EXT4_ENV_INTERFACE, dev);
	if (dev_desc == NULL) {
		printf("Failed to find %s%d\n",
			EXT4_ENV_INTERFACE, dev);
		set_default_env(NULL);
		return;
	}

	err = ext4_register_device(dev_desc, part);
	if (err) {
		printf("Failed to register %s%d:%d\n",
			EXT4_ENV_INTERFACE, dev, part);
		set_default_env(NULL);
		return;
	}

	err = ext4_read_file(EXT4_ENV_FILE, (uchar *)&buf, 0, CONFIG_ENV_SIZE);
	ext4fs_close();

	if (err == -1) {
		printf("\n** Unable to read \"%s\" from %s%d:%d **\n",
			EXT4_ENV_FILE, EXT4_ENV_INTERFACE, dev, part);
		set_default_env(NULL);
		return;
	}

	env_import(buf, 1);
}
