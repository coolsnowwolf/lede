/*
 *  RouterBOOT configuration utility
 *
 *  Copyright (C) 2010 Gabor Juhos <juhosg@openwrt.org>
 *  Copyright (C) 2017 Thibaut VARENE <varenet@parisc-linux.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <linux/limits.h>

#include "rbcfg.h"
#include "cyg_crc.h"

#define RBCFG_TMP_FILE	"/tmp/.rbcfg"
#define RBCFG_MTD_NAME	"soft_config"

#define RB_ERR_NOTFOUND		1
#define RB_ERR_INVALID		2
#define RB_ERR_NOMEM		3
#define RB_ERR_IO		4
#define RB_ERR_NOTWANTED	5

#define ARRAY_SIZE(_a)	(sizeof((_a)) / sizeof((_a)[0]))

struct rbcfg_ctx {
	char		*mtd_device;
	char		*tmp_file;
	char		*buf;
	unsigned	buflen;
};

struct rbcfg_value {
	const char		*name;
	const char		*desc;
	union {
		uint32_t	u32;
		const char	*raw;
	} val;
};

#define RBCFG_ENV_TYPE_U32	0

struct rbcfg_env {
	const char			*name;
	int				type;
	uint16_t			id;
	const struct rbcfg_value	*values;
	int				num_values;
};

#define CMD_FLAG_USES_CFG	0x01

struct rbcfg_command {
	const char	*command;
	const char	*usage;
	int		flags;
	int		(*exec)(int argc, const char *argv[]);
};

struct rbcfg_soc {
	const char	*needle;
	const int	type;
};

static void usage(void);

/* Globals */

static struct rbcfg_ctx *rbcfg_ctx;
static char *rbcfg_name;

#define CFG_U32(_name, _desc, _val) { 	\
	.name		= (_name),	\
	.desc		= (_desc),	\
	.val.u32	= (_val),	\
}

static const struct rbcfg_value rbcfg_boot_delay[] = {
	CFG_U32("1", "1 second", RB_BOOT_DELAY_1SEC),
	CFG_U32("2", "2 seconds", RB_BOOT_DELAY_2SEC),
	CFG_U32("3", "3 seconds", RB_BOOT_DELAY_3SEC),
	CFG_U32("4", "4 seconds", RB_BOOT_DELAY_4SEC),
	CFG_U32("5", "5 seconds", RB_BOOT_DELAY_5SEC),
	CFG_U32("6", "6 seconds", RB_BOOT_DELAY_6SEC),
	CFG_U32("7", "7 seconds", RB_BOOT_DELAY_7SEC),
	CFG_U32("8", "8 seconds", RB_BOOT_DELAY_8SEC),
	CFG_U32("9", "9 seconds", RB_BOOT_DELAY_9SEC),
};

static const struct rbcfg_value rbcfg_boot_device[] = {
	CFG_U32("eth", "boot over Ethernet",
		RB_BOOT_DEVICE_ETHER),
	CFG_U32("nandeth", "boot from NAND, if fail then Ethernet",
		RB_BOOT_DEVICE_NANDETH),
	CFG_U32("ethnand", "boot Ethernet once, then NAND",
		RB_BOOT_DEVICE_ETHONCE),
	CFG_U32("nand", "boot from NAND only",
		RB_BOOT_DEVICE_NANDONLY),
	CFG_U32("flash", "boot in flash configuration mode",
		RB_BOOT_DEVICE_FLASHCFG),
	CFG_U32("flashnand", "boot in flash configuration mode once, then NAND",
		RB_BOOT_DEVICE_FLSHONCE),
};

static const struct rbcfg_value rbcfg_boot_key[] = {
	CFG_U32("any", "any key", RB_BOOT_KEY_ANY),
	CFG_U32("del", "<Delete> key only", RB_BOOT_KEY_DEL),
};

static const struct rbcfg_value rbcfg_boot_protocol[] = {
	CFG_U32("bootp", "BOOTP protocol", RB_BOOT_PROTOCOL_BOOTP),
	CFG_U32("dhcp", "DHCP protocol", RB_BOOT_PROTOCOL_DHCP),
};

static const struct rbcfg_value rbcfg_uart_speed[] = {
	CFG_U32("115200", "", RB_UART_SPEED_115200),
	CFG_U32("57600", "", RB_UART_SPEED_57600),
	CFG_U32("38400", "", RB_UART_SPEED_38400),
	CFG_U32("19200", "", RB_UART_SPEED_19200),
	CFG_U32("9600", "", RB_UART_SPEED_9600),
	CFG_U32("4800", "", RB_UART_SPEED_4800),
	CFG_U32("2400", "", RB_UART_SPEED_2400),
	CFG_U32("1200", "", RB_UART_SPEED_1200),
	CFG_U32("off", "disable console output", RB_UART_SPEED_OFF),
};

static const struct rbcfg_value rbcfg_cpu_mode[] = {
	CFG_U32("powersave", "power save", RB_CPU_MODE_POWERSAVE),
	CFG_U32("regular", "regular (better for -0c environment)",
		RB_CPU_MODE_REGULAR),
};

static const struct rbcfg_value rbcfg_cpu_freq_dummy[] = {
};

static const struct rbcfg_value rbcfg_cpu_freq_qca953x[] = {
	CFG_U32("-2", "-100MHz", RB_CPU_FREQ_L2),
	CFG_U32("-1", "- 50MHz", RB_CPU_FREQ_L1),
	CFG_U32("0", "Factory", RB_CPU_FREQ_N0),
	CFG_U32("+1", "+ 50MHz", RB_CPU_FREQ_H1),
	CFG_U32("+2", "+100MHz", RB_CPU_FREQ_H2),
};

static const struct rbcfg_value rbcfg_cpu_freq_ar9344[] = {
	CFG_U32("-2", "-100MHz", RB_CPU_FREQ_L2),
	CFG_U32("-1", "- 50MHz", RB_CPU_FREQ_L1),
	CFG_U32("0", "Factory", RB_CPU_FREQ_N0),
	CFG_U32("+1", "+ 50MHz", RB_CPU_FREQ_H1),
	CFG_U32("+2", "+100MHz", RB_CPU_FREQ_H2),
	CFG_U32("+3", "+150MHz", RB_CPU_FREQ_H3),
};

static const struct rbcfg_value rbcfg_booter[] = {
	CFG_U32("regular", "load regular booter", RB_BOOTER_REGULAR),
	CFG_U32("backup", "force backup-booter loading", RB_BOOTER_BACKUP),
};

static struct rbcfg_env rbcfg_envs[] = {
	{
		.name		= "boot_delay",
		.id		= RB_ID_BOOT_DELAY,
		.type		= RBCFG_ENV_TYPE_U32,
		.values		= rbcfg_boot_delay,
		.num_values	= ARRAY_SIZE(rbcfg_boot_delay),
	}, {
		.name		= "boot_device",
		.id		= RB_ID_BOOT_DEVICE,
		.type		= RBCFG_ENV_TYPE_U32,
		.values		= rbcfg_boot_device,
		.num_values	= ARRAY_SIZE(rbcfg_boot_device),
	}, {
		.name		= "boot_key",
		.id		= RB_ID_BOOT_KEY,
		.type		= RBCFG_ENV_TYPE_U32,
		.values		= rbcfg_boot_key,
		.num_values	= ARRAY_SIZE(rbcfg_boot_key),
	}, {
		.name		= "boot_protocol",
		.id		= RB_ID_BOOT_PROTOCOL,
		.type		= RBCFG_ENV_TYPE_U32,
		.values		= rbcfg_boot_protocol,
		.num_values	= ARRAY_SIZE(rbcfg_boot_protocol),
	}, {
		.name		= "booter",
		.id		= RB_ID_BOOTER,
		.type		= RBCFG_ENV_TYPE_U32,
		.values		= rbcfg_booter,
		.num_values	= ARRAY_SIZE(rbcfg_booter),
	}, {
		.name		= "cpu_mode",
		.id		= RB_ID_CPU_MODE,
		.type		= RBCFG_ENV_TYPE_U32,
		.values		= rbcfg_cpu_mode,
		.num_values	= ARRAY_SIZE(rbcfg_cpu_mode),
	}, {
		.name		= "cpu_freq",
		.id		= RB_ID_CPU_FREQ,
		.type		= RBCFG_ENV_TYPE_U32,
		.values		= rbcfg_cpu_freq_dummy,
		.num_values	= ARRAY_SIZE(rbcfg_cpu_freq_dummy),
	}, {
		.name		= "uart_speed",
		.id		= RB_ID_UART_SPEED,
		.type		= RBCFG_ENV_TYPE_U32,
		.values		= rbcfg_uart_speed,
		.num_values	= ARRAY_SIZE(rbcfg_uart_speed),
	}
};

static inline uint16_t
get_u16(const void *buf)
{
	const uint8_t *p = buf;

	return ((uint16_t) p[1] + ((uint16_t) p[0] << 8));
}

static inline uint32_t
get_u32(const void *buf)
{
	const uint8_t *p = buf;

	return ((uint32_t) p[3] + ((uint32_t) p[2] << 8) +
	       ((uint32_t) p[1] << 16) + ((uint32_t) p[0] << 24));
}

static inline void
put_u32(void *buf, uint32_t val)
{
	uint8_t *p = buf;

	p[3] = val & 0xff;
	p[2] = (val >> 8) & 0xff;
	p[1] = (val >> 16) & 0xff;
	p[0] = (val >> 24) & 0xff;
}

static int
rbcfg_find_tag(struct rbcfg_ctx *ctx, uint16_t tag_id, uint16_t *tag_len,
	       void **tag_data)
{
	uint16_t id;
	uint16_t len;
	char *buf = ctx->buf;
	unsigned int buflen = ctx->buflen;
	int ret = RB_ERR_NOTFOUND;

	/* skip magic and CRC value */
	buf += 8;
	buflen -= 8;

	while (buflen > 2) {
		len = get_u16(buf);
		buf += 2;
		buflen -= 2;

		if (buflen < 2)
			break;

		id = get_u16(buf);
		buf += 2;
		buflen -= 2;

		if (id == RB_ID_TERMINATOR) {
			ret = RB_ERR_NOTWANTED;
			break;
		}

		if (buflen < len)
			break;

		if (id == tag_id) {
			*tag_len = len;
			*tag_data = buf;
			ret = 0;
			break;
		}

		buf += len;
		buflen -= len;
	}

	if (RB_ERR_NOTFOUND == ret)
		fprintf(stderr, "no tag found with id=%u\n", tag_id);

	return ret;
}

static int
rbcfg_get_u32(struct rbcfg_ctx *ctx, uint16_t id, uint32_t *val)
{
	void *tag_data;
	uint16_t tag_len;
	int err;

	err = rbcfg_find_tag(ctx, id, &tag_len, &tag_data);
	if (err)
		return err;

	*val = get_u32(tag_data);
	return 0;
}

static int
rbcfg_set_u32(struct rbcfg_ctx *ctx, uint16_t id, uint32_t val)
{
	void *tag_data;
	uint16_t tag_len;
	int err;

	err = rbcfg_find_tag(ctx, id, &tag_len, &tag_data);
	if (err)
		return err;

	put_u32(tag_data, val);
	return 0;
}

char *rbcfg_find_mtd(const char *name, int *erase_size)
{
	FILE *f;
	int mtd_num;
	char dev[PATH_MAX];
	char *ret = NULL;
	struct stat s;
	int err;

	f = fopen("/proc/mtd", "r");
	if (!f)
		return NULL;

	while (1) {
		char *p;
		p = fgets(dev, sizeof(dev), f);
		if (!p)
			break;

		if (!strstr(dev, name))
			continue;

		err = sscanf(dev, "mtd%d: %08x", &mtd_num, erase_size);
		if (err != 2)
			break;

		sprintf(dev, "/dev/mtdblock%d", mtd_num);
		err = stat(dev, &s);
		if (err < 0)
			break;

		if ((s.st_mode & S_IFBLK) == 0)
			break;

		ret = malloc(strlen(dev) + 1);
		if (ret == NULL)
			break;

		strncpy(ret, dev, strlen(dev) + 1);
		break;
	}

	fclose(f);
	return ret;
}

static int
rbcfg_check_tmp(struct rbcfg_ctx *ctx)
{
	struct stat s;
	int err;

	err = stat(ctx->tmp_file, &s);
	if (err < 0)
		return 0;

	if ((s.st_mode & S_IFREG) == 0)
		return 0;

	if (s.st_size != ctx->buflen)
		return 0;

	return 1;
}

static int
rbcfg_load(struct rbcfg_ctx *ctx)
{
	uint32_t magic;
	uint32_t crc_orig, crc;
	char *name;
	int tmp;
	int fd;
	int err;

	tmp = rbcfg_check_tmp(ctx);
	name = (tmp) ? ctx->tmp_file : ctx->mtd_device;

	fd = open(name, O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "unable to open %s\n", name);
		err = RB_ERR_IO;
		goto err;
	}

	err = read(fd, ctx->buf, ctx->buflen);
	if (err != ctx->buflen) {
		fprintf(stderr, "unable to read from %s\n", name);
		err = RB_ERR_IO;
		goto err_close;
	}

	magic = get_u32(ctx->buf);
	if (magic != RB_MAGIC_SOFT) {
		fprintf(stderr, "invalid configuration\n");
		err = RB_ERR_INVALID;
		goto err_close;
	}

	crc_orig = get_u32(ctx->buf + 4);
	put_u32(ctx->buf + 4, 0);
	crc = cyg_ether_crc32((unsigned char *) ctx->buf, ctx->buflen);
	if (crc != crc_orig) {
		fprintf(stderr, "configuration has CRC error\n");
		err = RB_ERR_INVALID;
		goto err_close;
	}

	err = 0;

 err_close:
	close(fd);
 err:
	return err;
}

static int
rbcfg_open()
{
	char *mtd_device;
	struct rbcfg_ctx *ctx;
	int buflen;
	int err;

	mtd_device = rbcfg_find_mtd(RBCFG_MTD_NAME, &buflen);
	if (!mtd_device) {
		fprintf(stderr, "unable to find configuration\n");
		return RB_ERR_NOTFOUND;
	}

	ctx = malloc(sizeof(struct rbcfg_ctx) + buflen);
	if (ctx == NULL) {
		err = RB_ERR_NOMEM;
		goto err_free_mtd;
	}

	ctx->mtd_device = mtd_device;
	ctx->tmp_file = RBCFG_TMP_FILE;
	ctx->buflen = buflen;
	ctx->buf = (char *) &ctx[1];

	err = rbcfg_load(ctx);
	if (err)
		goto err_free_ctx;

	rbcfg_ctx = ctx;
	return 0;

 err_free_ctx:
	free(ctx);
 err_free_mtd:
	free(mtd_device);
	return err;
}

static int
rbcfg_update(int tmp)
{
	struct rbcfg_ctx *ctx = rbcfg_ctx;
	char *name;
	uint32_t crc;
	int fd;
	int err;

	put_u32(ctx->buf, RB_MAGIC_SOFT);
	put_u32(ctx->buf + 4, 0);
	crc = cyg_ether_crc32((unsigned char *) ctx->buf, ctx->buflen);
	put_u32(ctx->buf + 4, crc);

	name = (tmp) ? ctx->tmp_file : ctx->mtd_device;
	fd = open(name, O_WRONLY | O_CREAT);
	if (fd < 0) {
		fprintf(stderr, "unable to open %s for writing\n", name);
		err = RB_ERR_IO;
		goto out;
	}

	err = write(fd, ctx->buf, ctx->buflen);
	if (err != ctx->buflen) {
		err = RB_ERR_IO;
		goto out_close;
	}

	fsync(fd);
	err = 0;

 out_close:
	close(fd);
 out:
	return err;
}

static void
rbcfg_close(void)
{
	struct rbcfg_ctx *ctx;

	ctx = rbcfg_ctx;
	free(ctx->mtd_device);
	free(ctx);
}

static const struct rbcfg_value *
rbcfg_env_find(const struct rbcfg_env *env, const char *name)
{
	unsigned i;

	for (i = 0; i < env->num_values; i++) {
		const struct rbcfg_value *v = &env->values[i];

		if (strcmp(v->name, name) == 0)
			return v;
	}

	return NULL;
}

static const struct rbcfg_value *
rbcfg_env_find_u32(const struct rbcfg_env *env, uint32_t val)
{
	unsigned i;

	for (i = 0; i < env->num_values; i++) {
		const struct rbcfg_value *v = &env->values[i];

		if (v->val.u32 == val)
			return v;
	}

	return NULL;
}

static const char *
rbcfg_env_get_u32(const struct rbcfg_env *env)
{
	const struct rbcfg_value *v;
	uint32_t val;
	int err;

	err = rbcfg_get_u32(rbcfg_ctx, env->id, &val);
	if (err)
		return NULL;

	v = rbcfg_env_find_u32(env, val);
	if (v == NULL) {
		fprintf(stderr, "unknown value %08x found for %s\n",
			val, env->name);
		return NULL;
	}

	return v->name;
}

static int
rbcfg_env_set_u32(const struct rbcfg_env *env, const char *data)
{
	const struct rbcfg_value *v;
	int err;

	v = rbcfg_env_find(env, data);
	if (v == NULL) {
		fprintf(stderr, "invalid value '%s'\n", data);
		return RB_ERR_INVALID;
	}

	err = rbcfg_set_u32(rbcfg_ctx, env->id, v->val.u32);
	return err;
}

static const char *
rbcfg_env_get(const struct rbcfg_env *env)
{
	const char *ret = NULL;

	switch (env->type) {
	case RBCFG_ENV_TYPE_U32:
		ret = rbcfg_env_get_u32(env);
		break;
	}

	return ret;
}

static int
rbcfg_env_set(const struct rbcfg_env *env, const char *data)
{
	int ret = 0;

	switch (env->type) {
	case RBCFG_ENV_TYPE_U32:
		ret = rbcfg_env_set_u32(env, data);
		break;
	}

	return ret;
}

static int
rbcfg_cmd_apply(int argc, const char *argv[])
{
	return rbcfg_update(0);
}

static int
rbcfg_cmd_help(int argc, const char *argv[])
{
	usage();
	return 0;
}

static int
rbcfg_cmd_get(int argc, const char *argv[])
{
	int err = RB_ERR_NOTFOUND;
	int i;

	if (argc != 1) {
		usage();
		return RB_ERR_INVALID;
	}

	for (i = 0; i < ARRAY_SIZE(rbcfg_envs); i++) {
		const struct rbcfg_env *env = &rbcfg_envs[i];
		const char *value;

		if (strcmp(env->name, argv[0]))
			continue;

		value = rbcfg_env_get(env);
		if (value) {
			fprintf(stdout, "%s\n", value);
			err = 0;
		}
		break;
	}

	return err;
}

static int
rbcfg_cmd_set(int argc, const char *argv[])
{
	int err = RB_ERR_INVALID;
	int i;

	if (argc != 2) {
		/* not enough parameters */
		usage();
		return RB_ERR_INVALID;
	}

	for (i = 0; i < ARRAY_SIZE(rbcfg_envs); i++) {
		const struct rbcfg_env *env = &rbcfg_envs[i];

		if (strcmp(env->name, argv[0]))
			continue;

		err = rbcfg_env_set(env, argv[1]);
		if (err == 0)
			err = rbcfg_update(1);
		break;
	}

	return err;
}

static int
rbcfg_cmd_show(int argc, const char *argv[])
{
	int i;

	if (argc != 0) {
		usage();
		return RB_ERR_INVALID;
	}

	for (i = 0; i < ARRAY_SIZE(rbcfg_envs); i++) {
		const struct rbcfg_env *env = &rbcfg_envs[i];
		const char *value;

		value = rbcfg_env_get(env);
		if (value)
			fprintf(stdout, "%s=%s\n", env->name, value);
	}

	return 0;
}

static const struct rbcfg_command rbcfg_commands[] = {
	{
		.command	= "apply",
		.usage		= "apply\n"
				  "\t- write configuration to the mtd device",
		.flags		= CMD_FLAG_USES_CFG,
		.exec		= rbcfg_cmd_apply,
	}, {
		.command	= "help",
		.usage		= "help\n"
				  "\t- show this screen",
		.exec		= rbcfg_cmd_help,
	}, {
		.command	= "get",
		.usage		= "get <name>\n"
				  "\t- get value of the configuration option <name>",
		.flags		= CMD_FLAG_USES_CFG,
		.exec		= rbcfg_cmd_get,
	}, {
		.command	= "set",
		.usage		= "set <name> <value>\n"
				  "\t- set value of the configuration option <name> to <value>",
		.flags		= CMD_FLAG_USES_CFG,
		.exec		= rbcfg_cmd_set,
	}, {
		.command	= "show",
		.usage		= "show\n"
				  "\t- show value of all configuration options",
		.flags		= CMD_FLAG_USES_CFG,
		.exec		= rbcfg_cmd_show,
	}
};

static void
usage(void)
{
	char buf[255];
	int len;
	int i;

	fprintf(stderr, "Usage: %s <command>\n", rbcfg_name);

	fprintf(stderr, "\nCommands:\n");
	for (i = 0; i < ARRAY_SIZE(rbcfg_commands); i++) {
		const struct rbcfg_command *cmd;
		cmd = &rbcfg_commands[i];

		len = snprintf(buf, sizeof(buf), "%s", cmd->usage);
		buf[len] = '\0';
		fprintf(stderr, "%s\n", buf);
	}

	fprintf(stderr, "\nConfiguration options:\n");
	for (i = 0; i < ARRAY_SIZE(rbcfg_envs); i++) {
		const struct rbcfg_env *env;
		int j;

		env = &rbcfg_envs[i];
		fprintf(stderr, "\n%s:\n", env->name);
		for (j = 0; j < env->num_values; j++) {
			const struct rbcfg_value *v = &env->values[j];
			fprintf(stderr, "\t%-12s %s\n", v->name, v->desc);
		}
	}
	fprintf(stderr, "\n");
}

#define RBCFG_SOC_UNKNOWN	0
#define RBCFG_SOC_QCA953X	1
#define RBCFG_SOC_AR9344	2

static const struct rbcfg_soc rbcfg_socs[] = {
	{
		.needle = "QCA953",
		.type = RBCFG_SOC_QCA953X,
	}, {
		.needle = "AR9344",
		.type = RBCFG_SOC_AR9344,
	},
};

#define CPUINFO_BUFSIZE		128	/* lines of interest are < 80 chars */

static int cpuinfo_find_soc(void)
{
	FILE *fp;
	char temp[CPUINFO_BUFSIZE];
	char *haystack, *needle;
	int i, found = 0, soc_type = RBCFG_SOC_UNKNOWN;

	fp = fopen("/proc/cpuinfo", "r");
	if (!fp)
		goto end;

	/* first, extract the system type line */
	needle = "system type";
	while(fgets(temp, CPUINFO_BUFSIZE, fp)) {
		if (!strncmp(temp, needle, strlen(needle))) {
			found = 1;
			break;
		}
	}

	fclose(fp);

	/* failsafe in case cpuinfo format changes */
	if (!found)
		goto end;

	/* skip the field header */
	haystack = strchr(temp, ':');

	/* then, try to identify known SoC, stop at first match */
	for (i = 0; i < ARRAY_SIZE(rbcfg_socs); i++) {
		if ((strstr(haystack, rbcfg_socs[i].needle))) {
			soc_type = rbcfg_socs[i].type;
			break;
		}
	}

end:
	return soc_type;
}

static void fixup_rbcfg_envs(void)
{
	int i, num_val, soc_type;
	const struct rbcfg_value * env_value;

	/* detect SoC */
	soc_type = cpuinfo_find_soc();

	/* update rbcfg_envs */
	switch (soc_type) {
		case RBCFG_SOC_QCA953X:
			env_value = rbcfg_cpu_freq_qca953x;
			num_val = ARRAY_SIZE(rbcfg_cpu_freq_qca953x);
			break;
		case RBCFG_SOC_AR9344:
			env_value = rbcfg_cpu_freq_ar9344;
			num_val = ARRAY_SIZE(rbcfg_cpu_freq_ar9344);
			break;
	}

	for (i = 0; i < ARRAY_SIZE(rbcfg_envs); i++) {
		if (RB_ID_CPU_FREQ == rbcfg_envs[i].id) {
			if (RBCFG_SOC_UNKNOWN == soc_type)
				rbcfg_envs[i].id = RB_ID_TERMINATOR;
			else {
				rbcfg_envs[i].values = env_value;
				rbcfg_envs[i].num_values = num_val;
			}
			break;
		}
	}
}

int main(int argc, const char *argv[])
{
	const struct rbcfg_command *cmd = NULL;
	int ret;
	int i;

	rbcfg_name = (char *) argv[0];

	fixup_rbcfg_envs();

	if (argc < 2) {
		usage();
		return EXIT_FAILURE;
	}

	for (i = 0; i < ARRAY_SIZE(rbcfg_commands); i++) {
		if (strcmp(rbcfg_commands[i].command, argv[1]) == 0) {
			cmd = &rbcfg_commands[i];
			break;
		}
	}

	if (cmd == NULL) {
		fprintf(stderr, "unknown command '%s'\n", argv[1]);
		usage();
		return EXIT_FAILURE;
	}

	argc -= 2;
	argv += 2;

	if (cmd->flags & CMD_FLAG_USES_CFG) {
		ret = rbcfg_open();
		if (ret)
			return EXIT_FAILURE;
	}

	ret = cmd->exec(argc, argv);

	if (cmd->flags & CMD_FLAG_USES_CFG)
		rbcfg_close();

	if (ret)
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}
