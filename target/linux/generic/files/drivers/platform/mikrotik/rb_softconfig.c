// SPDX-License-Identifier: GPL-2.0-only
/*
 * Driver for MikroTik RouterBoot soft config.
 *
 * Copyright (C) 2020 Thibaut VARÈNE <hacks+kernel@slashdirt.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 *
 * This driver exposes the data encoded in the "soft_config" flash segment of
 * MikroTik RouterBOARDs devices. It presents the data in a sysfs folder
 * named "soft_config". The data is presented in a user/machine-friendly way
 * with just as much parsing as can be generalized across mikrotik platforms
 * (as inferred from reverse-engineering).
 *
 * The known soft_config tags are presented in the "soft_config" sysfs folder,
 * with the addition of one specific file named "commit", which is only
 * available if the driver supports writes to the mtd device: no modifications
 * made to any of the other attributes are actually written back to flash media
 * until a true value is input into this file (e.g. [Yy1]). This is to avoid
 * unnecessary flash wear, and to permit to revert all changes by issuing a
 * false value ([Nn0]). Reading the content of this file shows the current
 * status of the driver: if the data in sysfs matches the content of the
 * soft_config partition, the file will read "clean". Otherwise, it will read
 * "dirty".
 *
 * The writeable sysfs files presented by this driver will accept only inputs
 * which are in a valid range for the given tag. As a design choice, the driver
 * will not assess whether the inputs are identical to the existing data.
 *
 * Note: PAGE_SIZE is assumed to be >= 4K, hence the device attribute show
 * routines need not check for output overflow.
 *
 * Some constant defines extracted from rbcfg.h by Gabor Juhos
 * <juhosg@openwrt.org>
 */

#include <linux/types.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/kobject.h>
#include <linux/string.h>
#include <linux/mtd/mtd.h>
#include <linux/sysfs.h>
#include <linux/version.h>
#include <linux/capability.h>
#include <linux/spinlock.h>
#include <linux/crc32.h>

#ifdef CONFIG_ATH79
 #include <asm/mach-ath79/ath79.h>
#endif

#include "routerboot.h"

#define RB_SOFTCONFIG_VER		"0.03"
#define RB_SC_PR_PFX			"[rb_softconfig] "

/*
 * mtd operations before 4.17 are asynchronous, not handled by this code
 * Also make the driver act read-only if 4K_SECTORS are not enabled, since they
 * are require to handle partial erasing of the small soft_config partition.
 */
#if defined(CONFIG_MTD_SPI_NOR_USE_4K_SECTORS)
 #define RB_SC_HAS_WRITE_SUPPORT	true
 #define RB_SC_WMODE			S_IWUSR
 #define RB_SC_RMODE			S_IRUSR
#else
 #define RB_SC_HAS_WRITE_SUPPORT	false
 #define RB_SC_WMODE			0
 #define RB_SC_RMODE			S_IRUSR
#endif

/* ID values for software settings */
#define RB_SCID_UART_SPEED		0x01	// u32*1
#define RB_SCID_BOOT_DELAY		0x02	// u32*1
#define RB_SCID_BOOT_DEVICE		0x03	// u32*1
#define RB_SCID_BOOT_KEY		0x04	// u32*1
#define RB_SCID_CPU_MODE		0x05	// u32*1
#define RB_SCID_BIOS_VERSION		0x06	// str
#define RB_SCID_BOOT_PROTOCOL		0x09	// u32*1
#define RB_SCID_CPU_FREQ_IDX		0x0C	// u32*1
#define RB_SCID_BOOTER			0x0D	// u32*1
#define RB_SCID_SILENT_BOOT		0x0F	// u32*1
/*
 * protected_routerboot seems to use tag 0x1F. It only works in combination with
 * RouterOS, resulting in a wiped board otherwise, so it's not implemented here.
 * The tag values are as follows:
 * - off: 0x0
 * - on: the lower halfword encodes the max value in s for the reset feature,
 *	 the higher halfword encodes the min value in s for the reset feature.
 * Default value when on: 0x00140258: 0x14 = 20s / 0x258= 600s
 * See details here: https://wiki.mikrotik.com/wiki/Manual:RouterBOARD_settings#Protected_bootloader
 */

/* Tag values */

#define RB_UART_SPEED_115200		0
#define RB_UART_SPEED_57600		1
#define RB_UART_SPEED_38400		2
#define RB_UART_SPEED_19200		3
#define RB_UART_SPEED_9600		4
#define RB_UART_SPEED_4800		5
#define RB_UART_SPEED_2400		6
#define RB_UART_SPEED_1200		7
#define RB_UART_SPEED_OFF		8

/* valid boot delay: 1 - 9s in 1s increment */
#define RB_BOOT_DELAY_MIN		1
#define RB_BOOT_DELAY_MAX		9

#define RB_BOOT_DEVICE_ETHER		0	// "boot over Ethernet"
#define RB_BOOT_DEVICE_NANDETH		1	// "boot from NAND, if fail then Ethernet"
#define RB_BOOT_DEVICE_CFCARD		2	// (not available in rbcfg)
#define RB_BOOT_DEVICE_ETHONCE		3	// "boot Ethernet once, then NAND"
#define RB_BOOT_DEVICE_NANDONLY		5	// "boot from NAND only"
#define RB_BOOT_DEVICE_FLASHCFG		7	// "boot in flash configuration mode"
#define RB_BOOT_DEVICE_FLSHONCE		8	// "boot in flash configuration mode once, then NAND"

/*
 * ATH79 9xxx CPU frequency indices.
 * It is unknown if they apply to all ATH79 RBs, and some do not seem to feature
 * the upper levels (QCA955x), while F is presumably AR9344-only.
 */
#define RB_CPU_FREQ_IDX_ATH79_9X_A	(0 << 3)
#define RB_CPU_FREQ_IDX_ATH79_9X_B	(1 << 3)	// 0x8
#define RB_CPU_FREQ_IDX_ATH79_9X_C	(2 << 3)	// 0x10 - factory freq for many devices
#define RB_CPU_FREQ_IDX_ATH79_9X_D	(3 << 3)	// 0x18
#define RB_CPU_FREQ_IDX_ATH79_9X_E	(4 << 3)	// 0x20
#define RB_CPU_FREQ_IDX_ATH79_9X_F	(5 << 3)	// 0x28

#define RB_CPU_FREQ_IDX_ATH79_9X_MIN		0	// all devices support lowest setting
#define RB_CPU_FREQ_IDX_ATH79_9X_AR9334_MAX	5	// stops at F
#define RB_CPU_FREQ_IDX_ATH79_9X_QCA953X_MAX	4	// stops at E
#define RB_CPU_FREQ_IDX_ATH79_9X_QCA9556_MAX	2	// stops at C
#define RB_CPU_FREQ_IDX_ATH79_9X_QCA9558_MAX	3	// stops at D

/* ATH79 7xxx CPU frequency indices. */
#define RB_CPU_FREQ_IDX_ATH79_7X_A	((0 * 9) << 4)
#define RB_CPU_FREQ_IDX_ATH79_7X_B	((1 * 9) << 4)
#define RB_CPU_FREQ_IDX_ATH79_7X_C	((2 * 9) << 4)
#define RB_CPU_FREQ_IDX_ATH79_7X_D	((3 * 9) << 4)
#define RB_CPU_FREQ_IDX_ATH79_7X_E	((4 * 9) << 4)
#define RB_CPU_FREQ_IDX_ATH79_7X_F	((5 * 9) << 4)
#define RB_CPU_FREQ_IDX_ATH79_7X_G	((6 * 9) << 4)
#define RB_CPU_FREQ_IDX_ATH79_7X_H	((7 * 9) << 4)

#define RB_CPU_FREQ_IDX_ATH79_7X_MIN		0	// all devices support lowest setting
#define RB_CPU_FREQ_IDX_ATH79_7X_AR724X_MAX	3	// stops at D
#define RB_CPU_FREQ_IDX_ATH79_7X_AR7161_MAX	7	// stops at H - check if applies to all AR71xx devices

#define RB_SC_CRC32_OFFSET		4	// located right after magic

static struct kobject *sc_kobj;
static u8 *sc_buf;
static size_t sc_buflen;
static rwlock_t sc_bufrwl;		// rw lock to sc_buf

/* MUST be used with lock held */
#define RB_SC_CLRCRC()		*(u32 *)(sc_buf + RB_SC_CRC32_OFFSET) = 0
#define RB_SC_GETCRC()		*(u32 *)(sc_buf + RB_SC_CRC32_OFFSET)
#define RB_SC_SETCRC(_crc)	*(u32 *)(sc_buf + RB_SC_CRC32_OFFSET) = (_crc)

struct sc_u32tvs {
	const u32 val;
	const char *str;
};

#define RB_SC_TVS(_val, _str) {		\
	.val = (_val),			\
	.str = (_str),			\
}

static ssize_t sc_tag_show_u32tvs(const u8 *pld, u16 pld_len, char *buf,
				  const struct sc_u32tvs tvs[], const int tvselmts)
{
	const char *fmt;
	char *out = buf;
	u32 data;	// cpu-endian
	int i;

	// fallback to raw hex output if we can't handle the input
	if (tvselmts < 0)
		return routerboot_tag_show_u32s(pld, pld_len, buf);

	if (sizeof(data) != pld_len)
		return -EINVAL;

	read_lock(&sc_bufrwl);
	data = *(u32 *)pld;		// pld aliases sc_buf
	read_unlock(&sc_bufrwl);

	for (i = 0; i < tvselmts; i++) {
		fmt = (tvs[i].val == data) ? "[%s] " : "%s ";
		out += sprintf(out, fmt, tvs[i].str);
	}

	out += sprintf(out, "\n");
	return out - buf;
}

static ssize_t sc_tag_store_u32tvs(const u8 *pld, u16 pld_len, const char *buf, size_t count,
				   const struct sc_u32tvs tvs[], const int tvselmts)
{
	int i;

	if (tvselmts < 0)
		return tvselmts;

	if (sizeof(u32) != pld_len)
		return -EINVAL;

	for (i = 0; i < tvselmts; i++) {
		if (sysfs_streq(buf, tvs[i].str)) {
			write_lock(&sc_bufrwl);
			*(u32 *)pld = tvs[i].val;	// pld aliases sc_buf
			RB_SC_CLRCRC();
			write_unlock(&sc_bufrwl);
			return count;
		}
	}

	return -EINVAL;
}

struct sc_boolts {
	const char *strfalse;
	const char *strtrue;
};

static ssize_t sc_tag_show_boolts(const u8 *pld, u16 pld_len, char *buf,
				  const struct sc_boolts *bts)
{
	const char *fmt;
	char *out = buf;
	u32 data;	// cpu-endian

	if (sizeof(data) != pld_len)
		return -EINVAL;

	read_lock(&sc_bufrwl);
	data = *(u32 *)pld;		// pld aliases sc_buf
	read_unlock(&sc_bufrwl);

	fmt = (data) ? "%s [%s]\n" : "[%s] %s\n";
	out += sprintf(out, fmt, bts->strfalse, bts->strtrue);

	return out - buf;
}

static ssize_t sc_tag_store_boolts(const u8 *pld, u16 pld_len, const char *buf, size_t count,
				   const struct sc_boolts *bts)
{
	u32 data;	// cpu-endian

	if (sizeof(data) != pld_len)
		return -EINVAL;

	if (sysfs_streq(buf, bts->strfalse))
		data = 0;
	else if (sysfs_streq(buf, bts->strtrue))
		data = 1;
	else
		return -EINVAL;

	write_lock(&sc_bufrwl);
	*(u32 *)pld = data;		// pld aliases sc_buf
	RB_SC_CLRCRC();
	write_unlock(&sc_bufrwl);

	return count;
}
static struct sc_u32tvs const sc_uartspeeds[] = {
	RB_SC_TVS(RB_UART_SPEED_OFF,	"off"),
	RB_SC_TVS(RB_UART_SPEED_1200,	"1200"),
	RB_SC_TVS(RB_UART_SPEED_2400,	"2400"),
	RB_SC_TVS(RB_UART_SPEED_4800,	"4800"),
	RB_SC_TVS(RB_UART_SPEED_9600,	"9600"),
	RB_SC_TVS(RB_UART_SPEED_19200,	"19200"),
	RB_SC_TVS(RB_UART_SPEED_38400,	"38400"),
	RB_SC_TVS(RB_UART_SPEED_57600,	"57600"),
	RB_SC_TVS(RB_UART_SPEED_115200,	"115200"),
};

/*
 * While the defines are carried over from rbcfg, use strings that more clearly
 * show the actual setting purpose (especially since the NAND* settings apply
 * to both nand- and nor-based devices). "cfcard" was disabled in rbcfg: disable
 * it here too.
 */
static struct sc_u32tvs const sc_bootdevices[] = {
	RB_SC_TVS(RB_BOOT_DEVICE_ETHER,		"eth"),
	RB_SC_TVS(RB_BOOT_DEVICE_NANDETH,	"flasheth"),
	//RB_SC_TVS(RB_BOOT_DEVICE_CFCARD,	"cfcard"),
	RB_SC_TVS(RB_BOOT_DEVICE_ETHONCE,	"ethonce"),
	RB_SC_TVS(RB_BOOT_DEVICE_NANDONLY,	"flash"),
	RB_SC_TVS(RB_BOOT_DEVICE_FLASHCFG,	"cfg"),
	RB_SC_TVS(RB_BOOT_DEVICE_FLSHONCE,	"cfgonce"),
};

static struct sc_boolts const sc_bootkey = {
	.strfalse = "any",
	.strtrue = "del",
};

static struct sc_boolts const sc_cpumode = {
	.strfalse = "powersave",
	.strtrue = "regular",
};

static struct sc_boolts const sc_bootproto = {
	.strfalse = "bootp",
	.strtrue = "dhcp",
};

static struct sc_boolts const sc_booter = {
	.strfalse = "regular",
	.strtrue = "backup",
};

static struct sc_boolts const sc_silent_boot = {
	.strfalse = "off",
	.strtrue = "on",
};

#define SC_TAG_SHOW_STORE_U32TVS_FUNCS(_name)		\
static ssize_t sc_tag_show_##_name(const u8 *pld, u16 pld_len, char *buf)	\
{										\
	return sc_tag_show_u32tvs(pld, pld_len, buf, sc_##_name, ARRAY_SIZE(sc_##_name));	\
}										\
static ssize_t sc_tag_store_##_name(const u8 *pld, u16 pld_len, const char *buf, size_t count)	\
{										\
	return sc_tag_store_u32tvs(pld, pld_len, buf, count, sc_##_name, ARRAY_SIZE(sc_##_name));	\
}

#define SC_TAG_SHOW_STORE_BOOLTS_FUNCS(_name)		\
static ssize_t sc_tag_show_##_name(const u8 *pld, u16 pld_len, char *buf)	\
{										\
	return sc_tag_show_boolts(pld, pld_len, buf, &sc_##_name);	\
}										\
static ssize_t sc_tag_store_##_name(const u8 *pld, u16 pld_len, const char *buf, size_t count)	\
{										\
	return sc_tag_store_boolts(pld, pld_len, buf, count, &sc_##_name);	\
}

SC_TAG_SHOW_STORE_U32TVS_FUNCS(uartspeeds)
SC_TAG_SHOW_STORE_U32TVS_FUNCS(bootdevices)
SC_TAG_SHOW_STORE_BOOLTS_FUNCS(bootkey)
SC_TAG_SHOW_STORE_BOOLTS_FUNCS(cpumode)
SC_TAG_SHOW_STORE_BOOLTS_FUNCS(bootproto)
SC_TAG_SHOW_STORE_BOOLTS_FUNCS(booter)
SC_TAG_SHOW_STORE_BOOLTS_FUNCS(silent_boot)

static ssize_t sc_tag_show_bootdelays(const u8 *pld, u16 pld_len, char *buf)
{
	const char *fmt;
	char *out = buf;
	u32 data;	// cpu-endian
	int i;

	if (sizeof(data) != pld_len)
		return -EINVAL;

	read_lock(&sc_bufrwl);
	data = *(u32 *)pld;		// pld aliases sc_buf
	read_unlock(&sc_bufrwl);

	for (i = RB_BOOT_DELAY_MIN; i <= RB_BOOT_DELAY_MAX; i++) {
		fmt = (i == data) ? "[%d] " : "%d ";
		out += sprintf(out, fmt, i);
	}

	out += sprintf(out, "\n");
	return out - buf;
}

static ssize_t sc_tag_store_bootdelays(const u8 *pld, u16 pld_len, const char *buf, size_t count)
{
	u32 data;	// cpu-endian
	int ret;

	if (sizeof(data) != pld_len)
		return -EINVAL;

	ret = kstrtou32(buf, 10, &data);
	if (ret)
		return ret;

	if ((data < RB_BOOT_DELAY_MIN) || (RB_BOOT_DELAY_MAX < data))
		return -EINVAL;

	write_lock(&sc_bufrwl);
	*(u32 *)pld = data;		// pld aliases sc_buf
	RB_SC_CLRCRC();
	write_unlock(&sc_bufrwl);

	return count;
}

/* Support CPU frequency accessors only when the tag format has been asserted */
#if defined(CONFIG_ATH79)
/* Use the same letter-based nomenclature as RouterBOOT */
static struct sc_u32tvs const sc_cpufreq_indexes_ath79_9x[] = {
	RB_SC_TVS(RB_CPU_FREQ_IDX_ATH79_9X_A,	"a"),
	RB_SC_TVS(RB_CPU_FREQ_IDX_ATH79_9X_B,	"b"),
	RB_SC_TVS(RB_CPU_FREQ_IDX_ATH79_9X_C,	"c"),
	RB_SC_TVS(RB_CPU_FREQ_IDX_ATH79_9X_D,	"d"),
	RB_SC_TVS(RB_CPU_FREQ_IDX_ATH79_9X_E,	"e"),
	RB_SC_TVS(RB_CPU_FREQ_IDX_ATH79_9X_F,	"f"),
};

static struct sc_u32tvs const sc_cpufreq_indexes_ath79_7x[] = {
	RB_SC_TVS(RB_CPU_FREQ_IDX_ATH79_7X_A,	"a"),
	RB_SC_TVS(RB_CPU_FREQ_IDX_ATH79_7X_B,	"b"),
	RB_SC_TVS(RB_CPU_FREQ_IDX_ATH79_7X_C,	"c"),
	RB_SC_TVS(RB_CPU_FREQ_IDX_ATH79_7X_D,	"d"),
	RB_SC_TVS(RB_CPU_FREQ_IDX_ATH79_7X_E,	"e"),
	RB_SC_TVS(RB_CPU_FREQ_IDX_ATH79_7X_F,	"f"),
	RB_SC_TVS(RB_CPU_FREQ_IDX_ATH79_7X_G,	"g"),
	RB_SC_TVS(RB_CPU_FREQ_IDX_ATH79_7X_H,	"h"),
};

static int sc_tag_cpufreq_ath79_arraysize(void)
{
	int idx_max;

	if (ATH79_SOC_AR7161 == ath79_soc)
		idx_max = RB_CPU_FREQ_IDX_ATH79_7X_AR7161_MAX+1;
	else if (soc_is_ar724x())
		idx_max = RB_CPU_FREQ_IDX_ATH79_7X_AR724X_MAX+1;
	else if (soc_is_ar9344())
		idx_max = RB_CPU_FREQ_IDX_ATH79_9X_AR9334_MAX+1;
	else if (soc_is_qca953x())
		idx_max = RB_CPU_FREQ_IDX_ATH79_9X_QCA953X_MAX+1;
	else if (soc_is_qca9556())
		idx_max = RB_CPU_FREQ_IDX_ATH79_9X_QCA9556_MAX+1;
	else if (soc_is_qca9558())
		idx_max = RB_CPU_FREQ_IDX_ATH79_9X_QCA9558_MAX+1;
	else
		idx_max = -EOPNOTSUPP;

	return idx_max;
}

static ssize_t sc_tag_show_cpufreq_indexes(const u8 *pld, u16 pld_len, char *buf)
{
	const struct sc_u32tvs *tvs;

	if (soc_is_ar71xx() || soc_is_ar724x())
		tvs = sc_cpufreq_indexes_ath79_7x;
	else
		tvs = sc_cpufreq_indexes_ath79_9x;

	return sc_tag_show_u32tvs(pld, pld_len, buf, tvs, sc_tag_cpufreq_ath79_arraysize());
}

static ssize_t sc_tag_store_cpufreq_indexes(const u8 *pld, u16 pld_len, const char *buf, size_t count)
{
	const struct sc_u32tvs *tvs;

	if (soc_is_ar71xx() || soc_is_ar724x())
		tvs = sc_cpufreq_indexes_ath79_7x;
	else
		tvs = sc_cpufreq_indexes_ath79_9x;

	return sc_tag_store_u32tvs(pld, pld_len, buf, count, tvs, sc_tag_cpufreq_ath79_arraysize());
}
#else
 /* By default we only show the raw value to help with reverse-engineering */
 #define sc_tag_show_cpufreq_indexes	routerboot_tag_show_u32s
 #define sc_tag_store_cpufreq_indexes	NULL
#endif

static ssize_t sc_attr_show(struct kobject *kobj, struct kobj_attribute *attr,
			    char *buf);
static ssize_t sc_attr_store(struct kobject *kobj, struct kobj_attribute *attr,
			     const char *buf, size_t count);

/* Array of known tags to publish in sysfs */
static struct sc_attr {
	const u16 tag_id;
	/* sysfs tag show attribute. Must lock sc_buf when dereferencing pld */
	ssize_t (* const tshow)(const u8 *pld, u16 pld_len, char *buf);
	/* sysfs tag store attribute. Must lock sc_buf when dereferencing pld */
	ssize_t (* const tstore)(const u8 *pld, u16 pld_len, const char *buf, size_t count);
	struct kobj_attribute kattr;
	u16 pld_ofs;
	u16 pld_len;
} sc_attrs[] = {
	{
		.tag_id = RB_SCID_UART_SPEED,
		.tshow = sc_tag_show_uartspeeds,
		.tstore = sc_tag_store_uartspeeds,
		.kattr = __ATTR(uart_speed, RB_SC_RMODE|RB_SC_WMODE, sc_attr_show, sc_attr_store),
	}, {
		.tag_id = RB_SCID_BOOT_DELAY,
		.tshow = sc_tag_show_bootdelays,
		.tstore = sc_tag_store_bootdelays,
		.kattr = __ATTR(boot_delay, RB_SC_RMODE|RB_SC_WMODE, sc_attr_show, sc_attr_store),
	}, {
		.tag_id = RB_SCID_BOOT_DEVICE,
		.tshow = sc_tag_show_bootdevices,
		.tstore = sc_tag_store_bootdevices,
		.kattr = __ATTR(boot_device, RB_SC_RMODE|RB_SC_WMODE, sc_attr_show, sc_attr_store),
	}, {
		.tag_id = RB_SCID_BOOT_KEY,
		.tshow = sc_tag_show_bootkey,
		.tstore = sc_tag_store_bootkey,
		.kattr = __ATTR(boot_key, RB_SC_RMODE|RB_SC_WMODE, sc_attr_show, sc_attr_store),
	}, {
		.tag_id = RB_SCID_CPU_MODE,
		.tshow = sc_tag_show_cpumode,
		.tstore = sc_tag_store_cpumode,
		.kattr = __ATTR(cpu_mode, RB_SC_RMODE|RB_SC_WMODE, sc_attr_show, sc_attr_store),
	}, {
		.tag_id = RB_SCID_BIOS_VERSION,
		.tshow = routerboot_tag_show_string,
		.tstore = NULL,
		.kattr = __ATTR(bios_version, RB_SC_RMODE, sc_attr_show, NULL),
	}, {
		.tag_id = RB_SCID_BOOT_PROTOCOL,
		.tshow = sc_tag_show_bootproto,
		.tstore = sc_tag_store_bootproto,
		.kattr = __ATTR(boot_proto, RB_SC_RMODE|RB_SC_WMODE, sc_attr_show, sc_attr_store),
	}, {
		.tag_id = RB_SCID_CPU_FREQ_IDX,
		.tshow = sc_tag_show_cpufreq_indexes,
		.tstore = sc_tag_store_cpufreq_indexes,
		.kattr = __ATTR(cpufreq_index, RB_SC_RMODE|RB_SC_WMODE, sc_attr_show, sc_attr_store),
	}, {
		.tag_id = RB_SCID_BOOTER,
		.tshow = sc_tag_show_booter,
		.tstore = sc_tag_store_booter,
		.kattr = __ATTR(booter, RB_SC_RMODE|RB_SC_WMODE, sc_attr_show, sc_attr_store),
	}, {
		.tag_id = RB_SCID_SILENT_BOOT,
		.tshow = sc_tag_show_silent_boot,
		.tstore = sc_tag_store_silent_boot,
		.kattr = __ATTR(silent_boot, RB_SC_RMODE|RB_SC_WMODE, sc_attr_show, sc_attr_store),
	},
};

static ssize_t sc_attr_show(struct kobject *kobj, struct kobj_attribute *attr,
			    char *buf)
{
	const struct sc_attr *sc_attr;
	const u8 *pld;
	u16 pld_len;

	sc_attr = container_of(attr, typeof(*sc_attr), kattr);

	if (!sc_attr->pld_len)
		return -ENOENT;

	pld = sc_buf + sc_attr->pld_ofs;	// pld aliases sc_buf -> lock!
	pld_len = sc_attr->pld_len;

	return sc_attr->tshow(pld, pld_len, buf);
}

static ssize_t sc_attr_store(struct kobject *kobj, struct kobj_attribute *attr,
			     const char *buf, size_t count)
{
	const struct sc_attr *sc_attr;
	const u8 *pld;
	u16 pld_len;

	if (!RB_SC_HAS_WRITE_SUPPORT)
		return -EOPNOTSUPP;

	if (!capable(CAP_SYS_ADMIN))
		return -EACCES;

	sc_attr = container_of(attr, typeof(*sc_attr), kattr);

	if (!sc_attr->tstore)
		return -EOPNOTSUPP;

	if (!sc_attr->pld_len)
		return -ENOENT;

	pld = sc_buf + sc_attr->pld_ofs;	// pld aliases sc_buf -> lock!
	pld_len = sc_attr->pld_len;

	return sc_attr->tstore(pld, pld_len, buf, count);
}

/*
 * Shows the current buffer status:
 * "clean": the buffer is in sync with the mtd data
 * "dirty": the buffer is out of sync with the mtd data
 */
static ssize_t sc_commit_show(struct kobject *kobj, struct kobj_attribute *attr,
			      char *buf)
{
	const char *str;
	char *out = buf;
	u32 crc;

	read_lock(&sc_bufrwl);
	crc = RB_SC_GETCRC();
	read_unlock(&sc_bufrwl);

	str = (crc) ? "clean" : "dirty";
	out += sprintf(out, "%s\n", str);

	return out - buf;
}

/*
 * Performs buffer flushing:
 * This routine expects an input compatible with kstrtobool().
 * - a "false" input discards the current changes and reads data back from mtd.
 * - a "true" input commits the current changes to mtd.
 * If there is no pending changes, this routine is a no-op.
 * Handling failures is left as an exercise to userspace.
 */
static ssize_t sc_commit_store(struct kobject *kobj, struct kobj_attribute *attr,
			      const char *buf, size_t count)
{
	struct mtd_info *mtd;
	struct erase_info ei;
	size_t bytes_rw, ret = count;
	bool flush;
	u32 crc;

	if (!RB_SC_HAS_WRITE_SUPPORT)
		return -EOPNOTSUPP;

	read_lock(&sc_bufrwl);
	crc = RB_SC_GETCRC();
	read_unlock(&sc_bufrwl);

	if (crc)
		return count;	// NO-OP

	ret = kstrtobool(buf, &flush);
	if (ret)
		return ret;

	mtd = get_mtd_device_nm(RB_MTD_SOFT_CONFIG);	// TODO allow override
	if (IS_ERR(mtd))
		return -ENODEV;

	write_lock(&sc_bufrwl);
	if (!flush)	// reread
		ret = mtd_read(mtd, 0, mtd->size, &bytes_rw, sc_buf);
	else {	// crc32 + commit
		/*
		 * CRC32 is computed on the entire buffer, excluding the CRC
		 * value itself. CRC is already null when we reach this point,
		 * so we can compute the CRC32 on the buffer as is.
		 * The expected CRC32 is Ethernet FCS style, meaning the seed is
		 * ~0 and the final result is also bitflipped.
		 */

		crc = ~crc32(~0, sc_buf, sc_buflen);
		RB_SC_SETCRC(crc);

		/*
		 * The soft_config partition is assumed to be entirely contained
		 * in a single eraseblock.
		 */

		ei.addr = 0;
		ei.len = mtd->size;
		ret = mtd_erase(mtd, &ei);
		if (!ret)
			ret = mtd_write(mtd, 0, mtd->size, &bytes_rw, sc_buf);

		/*
		 * Handling mtd_write() failure here is a tricky situation. The
		 * proposed approach is to let userspace deal with retrying,
		 * with the caveat that it must try to flush the buffer again as
		 * rereading the mtd contents could potentially read garbage.
		 * The rationale is: even if we keep a shadow buffer of the
		 * original content, there is no guarantee that we will ever be
		 * able to write it anyway.
		 * Regardless, it appears that RouterBOOT will ignore an invalid
		 * soft_config (including a completely wiped segment) and will
		 * write back factory defaults when it happens.
		 */
	}
	write_unlock(&sc_bufrwl);

	put_mtd_device(mtd);

	if (ret)
		goto mtdfail;

	if (bytes_rw != sc_buflen) {
		ret = -EIO;
		goto mtdfail;
	}

	return count;

mtdfail:
	RB_SC_CLRCRC();	// mark buffer content as dirty/invalid
	return ret;
}

static struct kobj_attribute sc_kattrcommit = __ATTR(commit, RB_SC_RMODE|RB_SC_WMODE, sc_commit_show, sc_commit_store);

int __init rb_softconfig_init(struct kobject *rb_kobj)
{
	struct mtd_info *mtd;
	size_t bytes_read, buflen;
	const u8 *buf;
	int i, ret;
	u32 magic;

	sc_buf = NULL;
	sc_kobj = NULL;

	// TODO allow override
	mtd = get_mtd_device_nm(RB_MTD_SOFT_CONFIG);
	if (IS_ERR(mtd))
		return -ENODEV;

	sc_buflen = mtd->size;
	sc_buf = kmalloc(sc_buflen, GFP_KERNEL);
	if (!sc_buf) {
		put_mtd_device(mtd);
		return -ENOMEM;
	}

	ret = mtd_read(mtd, 0, sc_buflen, &bytes_read, sc_buf);
	put_mtd_device(mtd);

	if (ret)
		goto fail;

	if (bytes_read != sc_buflen) {
		ret = -EIO;
		goto fail;
	}

	/* Check we have what we expect */
	magic = *(const u32 *)sc_buf;
	if (RB_MAGIC_SOFT != magic) {
		ret = -EINVAL;
		goto fail;
	}

	/* Skip magic and 32bit CRC located immediately after */
	buf = sc_buf + (sizeof(magic) + sizeof(u32));
	buflen = sc_buflen - (sizeof(magic) + sizeof(u32));

	/* Populate sysfs */
	ret = -ENOMEM;
	sc_kobj = kobject_create_and_add(RB_MTD_SOFT_CONFIG, rb_kobj);
	if (!sc_kobj)
		goto fail;

	rwlock_init(&sc_bufrwl);

	/* Locate and publish all known tags */
	for (i = 0; i < ARRAY_SIZE(sc_attrs); i++) {
		ret = routerboot_tag_find(buf, buflen, sc_attrs[i].tag_id,
					  &sc_attrs[i].pld_ofs, &sc_attrs[i].pld_len);
		if (ret) {
			sc_attrs[i].pld_ofs = sc_attrs[i].pld_len = 0;
			continue;
		}

		/* Account for skipped magic and crc32 */
		sc_attrs[i].pld_ofs += sizeof(magic) + sizeof(u32);

		ret = sysfs_create_file(sc_kobj, &sc_attrs[i].kattr.attr);
		if (ret)
			pr_warn(RB_SC_PR_PFX "Could not create %s sysfs entry (%d)\n",
			       sc_attrs[i].kattr.attr.name, ret);
	}

	/* Finally add the 'commit' attribute */
	if (RB_SC_HAS_WRITE_SUPPORT) {
		ret = sysfs_create_file(sc_kobj, &sc_kattrcommit.attr);
		if (ret) {
			pr_err(RB_SC_PR_PFX "Could not create %s sysfs entry (%d), aborting!\n",
			       sc_kattrcommit.attr.name, ret);
			goto sysfsfail;	// required attribute
		}
	}

	pr_info("MikroTik RouterBOARD software configuration sysfs driver v" RB_SOFTCONFIG_VER "\n");

	return 0;

sysfsfail:
	kobject_put(sc_kobj);
	sc_kobj = NULL;
fail:
	kfree(sc_buf);
	sc_buf = NULL;
	return ret;
}

void __exit rb_softconfig_exit(void)
{
	kobject_put(sc_kobj);
	kfree(sc_buf);
}
