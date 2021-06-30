// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * B43 Fallback SPROM Driver
 *
 * Copyright (C) 2020 Álvaro Fernández Rojas <noltari@gmail.com>
 * Copyright (C) 2014 Jonas Gorski <jonas.gorski@gmail.com>
 * Copyright (C) 2008 Maxime Bizon <mbizon@freebox.fr>
 * Copyright (C) 2008 Florian Fainelli <f.fainelli@gmail.com>
 */

#include <linux/bcma/bcma.h>
#include <linux/etherdevice.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/mtd/mtd.h>
#include <linux/of_net.h>
#include <linux/of_platform.h>
#include <linux/ssb/ssb.h>

enum b43_sprom_type {
	B43_SPROM_SSB,
	B43_SPROM_BCMA,
	B43_SPROM_NUM
};

struct b43_sprom_raw {
	u16 *map;
	size_t size;
	u8 type;
};

struct b43_sprom {
	struct device *dev;
	struct ssb_sprom sprom;
	u32 pci_bus;
	u32 pci_dev;
	u8 mac[ETH_ALEN];
	int devid_override;
};

static struct b43_sprom b43_sprom;

#if defined (CONFIG_SSB_PCIHOST)
static u16 bcm4306_sprom_map[] = {
	0x4001, 0x0000, 0x0453, 0x14e4, 0x4320, 0x8000, 0x0002, 0x0002,
	0x1000, 0x1800, 0x0000, 0x0000, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0x0000, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0x0000, 0x0000, 0x0000, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0x3034, 0x14d4,
	0xfa91, 0xfe60, 0xffff, 0xffff, 0x004c, 0xffff, 0xffff, 0xffff,
	0x003e, 0x0a49, 0xff02, 0x0000, 0xff10, 0xffff, 0xffff, 0x0002,
};

struct b43_sprom_raw bcm4306_sprom = {
	.map = bcm4306_sprom_map,
	.size = ARRAY_SIZE(bcm4306_sprom_map),
	.type = B43_SPROM_SSB,
};

static u16 bcm4318_sprom_map[] = {
	0x2001, 0x0000, 0x0449, 0x14e4, 0x4318, 0x8000, 0x0002, 0x0000,
	0x1000, 0x1800, 0x0000, 0x0000, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0x0000, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0x0000, 0x0000, 0x0000, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0x3046, 0x15a7,
	0xfab0, 0xfe97, 0xffff, 0xffff, 0x0048, 0xffff, 0xffff, 0xffff,
	0x003e, 0xea49, 0xff02, 0x0000, 0xff08, 0xffff, 0xffff, 0x0002,
};

struct b43_sprom_raw bcm4318_sprom = {
	.map = bcm4318_sprom_map,
	.size = ARRAY_SIZE(bcm4318_sprom_map),
	.type = B43_SPROM_SSB,
};

static u16 bcm4321_sprom_map[] = {
	0x3001, 0x0000, 0x046c, 0x14e4, 0x4328, 0x8000, 0x0002, 0x0000,
	0x1000, 0x1800, 0x0000, 0x0000, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0x5372, 0x0032, 0x4a01, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0xffff, 0xffff, 0xffff, 0x0303, 0x0202,
	0xffff, 0x2728, 0x5b5b, 0x222b, 0x5b5b, 0x1927, 0x5b5b, 0x1e36,
	0x5b5b, 0x303c, 0x3030, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0x3e4c, 0x0000, 0x0000, 0x0000, 0x0000, 0x7838, 0x3a34, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0xffff, 0xffff, 0xffff, 0xffff, 0x3e4c,
	0x0000, 0x0000, 0x0000, 0x0000, 0x7838, 0x3a34, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0x0008, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0x0004,
};

struct b43_sprom_raw bcm4321_sprom = {
	.map = bcm4321_sprom_map,
	.size = ARRAY_SIZE(bcm4321_sprom_map),
	.type = B43_SPROM_SSB,
};

static u16 bcm4322_sprom_map[] = {
	0x3001, 0x0000, 0x04bc, 0x14e4, 0x432c, 0x8000, 0x0002, 0x0000,
	0x1730, 0x1800, 0x0000, 0x0000, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0x5372, 0x1209, 0x0200, 0x0000, 0x0400, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0xffff, 0xffff, 0xffff, 0x0303, 0x0202,
	0xffff, 0x0033, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0x0301,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0x2048, 0xfe9a, 0x1571, 0xfabd, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0x2048, 0xfeb9, 0x159f, 0xfadd, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x3333, 0x5555, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0x0008,
};

struct b43_sprom_raw bcm4322_sprom = {
	.map = bcm4322_sprom_map,
	.size = ARRAY_SIZE(bcm4322_sprom_map),
	.type = B43_SPROM_SSB,
};

static u16 bcm43222_sprom_map[] = {
	0x2001, 0x0000, 0x04d4, 0x14e4, 0x4351, 0x8000, 0x0002, 0x0000,
	0x1730, 0x1800, 0x0000, 0x0000, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0x5372, 0x2305, 0x0200, 0x0000, 0x2400, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0xffff, 0xffff, 0xffff, 0x0303, 0x0202,
	0xffff, 0x0033, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0x0325,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0x204c, 0xfea6, 0x1717, 0xfa6d, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0x204c, 0xfeb8, 0x167c, 0xfa9e, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0x0000, 0x3333, 0x3333, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x3333, 0x3333, 0x3333, 0x3333, 0x3333, 0x3333, 0x3333,
	0x3333, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0004, 0x0000, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0x0008,
};

struct b43_sprom_raw bcm43222_sprom = {
	.map = bcm43222_sprom_map,
	.size = ARRAY_SIZE(bcm43222_sprom_map),
	.type = B43_SPROM_SSB,
};
#endif /* CONFIG_SSB_PCIHOST */

#if defined(CONFIG_BCMA_HOST_PCI)
static u16 bcm4313_sprom_map[] = {
	0x2801, 0x0000, 0x0510, 0x14e4, 0x0078, 0xedbe, 0x0000, 0x2bc4,
	0x2a64, 0x2964, 0x2c64, 0x3ce7, 0x46ff, 0x47ff, 0x0c00, 0x0820,
	0x0030, 0x1002, 0x9f28, 0x5d44, 0x8080, 0x1d8f, 0x0032, 0x0100,
	0xdf00, 0x71f5, 0x8400, 0x0083, 0x8500, 0x2010, 0x0001, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x1008, 0x0305, 0x0000, 0x0000, 0x0000, 0x0000,
	0x4727, 0x8000, 0x0002, 0x0000, 0x1f30, 0x1800, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x5372, 0x1215, 0x2a00, 0x0800, 0x0800, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0003, 0xffff, 0x88ff, 0xffff, 0x0003, 0x0202,
	0xffff, 0x0011, 0x007a, 0x0000, 0x0000, 0x0000, 0x0000, 0x0201,
	0x0000, 0x7800, 0x7c0a, 0x0398, 0x0008, 0x0000, 0x0000, 0x0000,
	0x0044, 0x1684, 0xfd0d, 0xff35, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0048, 0xfed2, 0x15d9, 0xfac6, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0008,
};

struct b43_sprom_raw bcm4313_sprom = {
	.map = bcm4313_sprom_map,
	.size = ARRAY_SIZE(bcm4313_sprom_map),
	.type = B43_SPROM_BCMA,
};

static u16 bcm4331_sprom_map[] = {
	0x2801, 0x0000, 0x0525, 0x14e4, 0x0078, 0xedbe, 0x0000, 0x2bc4,
	0x2a64, 0x2964, 0x2c64, 0x3ce7, 0x46ff, 0x47ff, 0x0c00, 0x0820,
	0x0030, 0x1002, 0x9f28, 0x5d44, 0x8080, 0x1d8f, 0x0032, 0x0100,
	0xdf00, 0x71f5, 0x8400, 0x0083, 0x8500, 0x2010, 0x0001, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0x1010, 0x0005, 0xffff, 0xffff, 0xffff, 0xffff,
	0x4331, 0x8000, 0x0002, 0x0000, 0x1f30, 0x1800, 0x0000, 0x0000,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0x5372, 0x1104, 0x0200, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0xffff, 0x88ff, 0xffff, 0x0707, 0x0202,
	0xff02, 0x0077, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0x0325,
	0x0325, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0x2048, 0xfe56, 0x16f2, 0xfa44, 0x3e3c, 0x3c3c, 0xfe77, 0x1657,
	0xfa75, 0xffff, 0xffff, 0xffff, 0xfe76, 0x15da, 0xfa85, 0x0000,
	0x2048, 0xfe5c, 0x16b5, 0xfa56, 0x3e3c, 0x3c3c, 0xfe7c, 0x169d,
	0xfa6b, 0xffff, 0xffff, 0xffff, 0xfe7a, 0x1597, 0xfa97, 0x0000,
	0x2048, 0xfe68, 0x1734, 0xfa46, 0x3e3c, 0x3c3c, 0xfe7f, 0x15e4,
	0xfa94, 0xffff, 0xffff, 0xffff, 0xfe7d, 0x1582, 0xfa9f, 0x0000,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0x0009,
};

struct b43_sprom_raw bcm4331_sprom = {
	.map = bcm4331_sprom_map,
	.size = ARRAY_SIZE(bcm4331_sprom_map),
	.type = B43_SPROM_BCMA,
};

static u16 bcm43131_sprom_map[] = {
	0x2801, 0x0000, 0x05f7, 0x14e4, 0x0070, 0xedbe, 0x1c00, 0x2bc4,
	0x2a64, 0x2964, 0x2c64, 0x3ce7, 0x46ff, 0x47ff, 0x0c00, 0x0820,
	0x0030, 0x1002, 0x9f28, 0x5d44, 0x8080, 0x1d8f, 0x0032, 0x0100,
	0xdf00, 0x71f5, 0x8400, 0x0083, 0x8500, 0x2010, 0x0001, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x1008, 0x0305, 0x0000, 0x0000, 0x0000, 0x0000,
	0x43aa, 0x8000, 0x0002, 0x0000, 0x1f30, 0x1800, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x5372, 0x1280, 0x0200, 0x0000, 0x8800, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0003, 0xffff, 0x88ff, 0xffff, 0x0002, 0x0202,
	0xffff, 0x0022, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0415,
	0x0000, 0x7800, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x204c, 0xfe96, 0x192c, 0xfa15, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x204c, 0xfe91, 0x1950, 0xfa0a, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0x0000, 0x4444, 0x4444, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x4444, 0x4444, 0x4444, 0x4444, 0x6666, 0x6666, 0x6666,
	0x6666, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0x0008,
};

struct b43_sprom_raw bcm43131_sprom = {
	.map = bcm43131_sprom_map,
	.size = ARRAY_SIZE(bcm43131_sprom_map),
	.type = B43_SPROM_BCMA,
};

static u16 bcm43217_sprom_map[] = {
	0x2801, 0x0000, 0x05e9, 0x14e4, 0x0070, 0xedbe, 0x0000, 0x2bc4,
	0x2a64, 0x2964, 0x2c64, 0x3ce7, 0x46ff, 0x47ff, 0x0c00, 0x0820,
	0x0030, 0x1002, 0x9f28, 0x5d44, 0x8080, 0x1d8f, 0x0032, 0x0100,
	0xdf00, 0x71f5, 0x8400, 0x0083, 0x8500, 0x2010, 0x0001, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x1008, 0x0305, 0x0000, 0x0000, 0x0000, 0x0000,
	0x43a9, 0x8000, 0x0002, 0x0000, 0x1f30, 0x1800, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x5372, 0x1252, 0x0200, 0x0000, 0x9800, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0003, 0xffff, 0x88ff, 0xffff, 0x0003, 0x0202,
	0xffff, 0x0033, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0415,
	0x0000, 0x7800, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x204c, 0xfe96, 0x192c, 0xfa15, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x204c, 0xfe91, 0x1950, 0xfa0a, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0x0000, 0x4444, 0x4444, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x4444, 0x4444, 0x4444, 0x4444, 0x6666, 0x6666, 0x6666,
	0x6666, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0x7a08,
};

struct b43_sprom_raw bcm43217_sprom = {
	.map = bcm43217_sprom_map,
	.size = ARRAY_SIZE(bcm43217_sprom_map),
	.type = B43_SPROM_BCMA,
};

static u16 bcm43225_sprom_map[] = {
	0x2801, 0x0000, 0x04da, 0x14e4, 0x0078, 0xedbe, 0x0000, 0x2bc4,
	0x2a64, 0x2964, 0x2c64, 0x3ce7, 0x46ff, 0x47ff, 0x0c00, 0x0820,
	0x0030, 0x1002, 0x9f28, 0x5d44, 0x8080, 0x1d8f, 0x0032, 0x0100,
	0xdf00, 0x71f5, 0x8400, 0x0083, 0x8500, 0x2010, 0x0001, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0x1008, 0x0005, 0xffff, 0xffff, 0xffff, 0xffff,
	0x4357, 0x8000, 0x0002, 0x0000, 0x1f30, 0x1800, 0x0000, 0x0000,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0x5372, 0x1200, 0x0200, 0x0000, 0x1000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x88ff, 0xffff, 0xffff, 0x0303, 0x0202,
	0xffff, 0x0033, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0x0325,
	0xffff, 0x7800, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0x204e, 0xfead, 0x1611, 0xfa9a, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0x204e, 0xfec1, 0x1674, 0xfab2, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0x0000, 0x5555, 0x5555, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x5555, 0x7555, 0x5555, 0x7555, 0x5555, 0x7555, 0x5555,
	0x7555, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0002, 0x0000, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0x0008,
};

struct b43_sprom_raw bcm43225_sprom = {
	.map = bcm43225_sprom_map,
	.size = ARRAY_SIZE(bcm43225_sprom_map),
	.type = B43_SPROM_BCMA,
};

static u16 bcm43227_sprom_map[] = {
	0x2801, 0x0000, 0x0543, 0x14e4, 0x0070, 0xedbe, 0x0000, 0x2bc4,
	0x2a64, 0x2964, 0x2c64, 0x3ce7, 0x46ff, 0x47ff, 0x0c00, 0x0820,
	0x0030, 0x1002, 0x9f28, 0x5d44, 0x8080, 0x1d8f, 0x0032, 0x0100,
	0xdf00, 0x71f5, 0x8400, 0x0083, 0x8500, 0x2010, 0x0001, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x1008, 0x0305, 0x0000, 0x0000, 0x0000, 0x0000,
	0x4358, 0x8000, 0x0002, 0x0000, 0x1f30, 0x1800, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x5372, 0x1402, 0x0200, 0x0000, 0x0800, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0003, 0xffff, 0x88ff, 0xffff, 0x0003, 0x0202,
	0xffff, 0x0033, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0415,
	0x0000, 0x7800, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x204c, 0xff36, 0x16d2, 0xfaae, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x204c, 0xfeca, 0x159b, 0xfa80, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0x0000, 0x4444, 0x4444, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x4444, 0x4444, 0x4444, 0x4444, 0x6666, 0x6666, 0x6666,
	0x6666, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0x0008,
};

struct b43_sprom_raw bcm43227_sprom = {
	.map = bcm43227_sprom_map,
	.size = ARRAY_SIZE(bcm43227_sprom_map),
	.type = B43_SPROM_BCMA,
};

static u16 bcm43228_sprom_map[] = {
	0x2801, 0x0000, 0x0011, 0x1028, 0x0070, 0xedbe, 0x0000, 0x2bc4,
	0x2a64, 0x2964, 0x2c64, 0x3ce7, 0x46ff, 0x47ff, 0x0c00, 0x0820,
	0x0030, 0x1002, 0x9f28, 0x5d44, 0x8080, 0x1d8f, 0x0032, 0x0100,
	0xdf00, 0x71f5, 0x8400, 0x0083, 0x8500, 0x2010, 0x0001, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x1008, 0x0305, 0x0000, 0x0000, 0x0000, 0x0000,
	0x4359, 0x8000, 0x0002, 0x0000, 0x1f30, 0x1800, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x5372, 0x1203, 0x0200, 0x0000, 0x0800, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0003, 0xffff, 0x88ff, 0xffff, 0x0303, 0x0202,
	0xffff, 0x0033, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0215,
	0x0215, 0x7800, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x204c, 0xff73, 0x1762, 0xfaa4, 0x3e34, 0x3434, 0xfea1, 0x154c,
	0xfad0, 0xfea1, 0x144c, 0xfafb, 0xfe7b, 0x13fe, 0xfafc, 0x0000,
	0x204c, 0xff41, 0x16a3, 0xfa8f, 0x3e34, 0x3434, 0xfe97, 0x1446,
	0xfb05, 0xfe97, 0x1346, 0xfb32, 0xfeb9, 0x1516, 0xfaee, 0x0000,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0x0000, 0x4444, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x4444, 0x4444, 0x4444, 0x4444, 0x8888, 0x8888, 0x8888,
	0x8888, 0x0000, 0x0000, 0x0000, 0x0000, 0x3333, 0x3333, 0x3333,
	0x3333, 0x0000, 0x0000, 0x0000, 0x0000, 0x3333, 0x3333, 0x3333,
	0x3333, 0x0000, 0x0000, 0x0000, 0x0000, 0x3333, 0x3333, 0x3333,
	0x3333, 0x0000, 0x0000, 0x0000, 0x0000, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xf008,
};

struct b43_sprom_raw bcm43228_sprom = {
	.map = bcm43228_sprom_map,
	.size = ARRAY_SIZE(bcm43228_sprom_map),
	.type = B43_SPROM_BCMA,
};
#endif /* CONFIG_BCMA_HOST_PCI */

/* Get the word-offset for a SSB_SPROM_XXX define. */
#define SPOFF(offset)	((offset) / sizeof(u16))
/* Helper to extract some _offset, which is one of the SSB_SPROM_XXX defines. */
#define SPEX16(_outvar, _offset, _mask, _shift)	\
	out->_outvar = ((in[SPOFF(_offset)] & (_mask)) >> (_shift))
#define SPEX32(_outvar, _offset, _mask, _shift)	\
	out->_outvar = ((((u32)in[SPOFF((_offset)+2)] << 16 | \
			   in[SPOFF(_offset)]) & (_mask)) >> (_shift))
#define SPEX(_outvar, _offset, _mask, _shift) \
	SPEX16(_outvar, _offset, _mask, _shift)

#define SPEX_ARRAY8(_field, _offset, _mask, _shift)	\
	do {	\
		SPEX(_field[0], _offset +  0, _mask, _shift);	\
		SPEX(_field[1], _offset +  2, _mask, _shift);	\
		SPEX(_field[2], _offset +  4, _mask, _shift);	\
		SPEX(_field[3], _offset +  6, _mask, _shift);	\
		SPEX(_field[4], _offset +  8, _mask, _shift);	\
		SPEX(_field[5], _offset + 10, _mask, _shift);	\
		SPEX(_field[6], _offset + 12, _mask, _shift);	\
		SPEX(_field[7], _offset + 14, _mask, _shift);	\
	} while (0)


static s8 sprom_extract_antgain(u8 sprom_revision, const u16 *in, u16 offset,
				u16 mask, u16 shift)
{
	u16 v;
	u8 gain;

	v = in[SPOFF(offset)];
	gain = (v & mask) >> shift;
	if (gain == 0xFF)
		gain = 2; /* If unset use 2dBm */
	if (sprom_revision == 1) {
		/* Convert to Q5.2 */
		gain <<= 2;
	} else {
		/* Q5.2 Fractional part is stored in 0xC0 */
		gain = ((gain & 0xC0) >> 6) | ((gain & 0x3F) << 2);
	}

	return (s8)gain;
}

static void sprom_extract_r23(struct ssb_sprom *out, const u16 *in)
{
	SPEX(boardflags_hi, SSB_SPROM2_BFLHI, 0xFFFF, 0);
	SPEX(opo, SSB_SPROM2_OPO, SSB_SPROM2_OPO_VALUE, 0);
	SPEX(pa1lob0, SSB_SPROM2_PA1LOB0, 0xFFFF, 0);
	SPEX(pa1lob1, SSB_SPROM2_PA1LOB1, 0xFFFF, 0);
	SPEX(pa1lob2, SSB_SPROM2_PA1LOB2, 0xFFFF, 0);
	SPEX(pa1hib0, SSB_SPROM2_PA1HIB0, 0xFFFF, 0);
	SPEX(pa1hib1, SSB_SPROM2_PA1HIB1, 0xFFFF, 0);
	SPEX(pa1hib2, SSB_SPROM2_PA1HIB2, 0xFFFF, 0);
	SPEX(maxpwr_ah, SSB_SPROM2_MAXP_A, SSB_SPROM2_MAXP_A_HI, 0);
	SPEX(maxpwr_al, SSB_SPROM2_MAXP_A, SSB_SPROM2_MAXP_A_LO,
	     SSB_SPROM2_MAXP_A_LO_SHIFT);
}

static void sprom_extract_r123(struct ssb_sprom *out, const u16 *in)
{
	u16 loc[3];

	if (out->revision == 3)			/* rev 3 moved MAC */
		loc[0] = SSB_SPROM3_IL0MAC;
	else {
		loc[0] = SSB_SPROM1_IL0MAC;
		loc[1] = SSB_SPROM1_ET0MAC;
		loc[2] = SSB_SPROM1_ET1MAC;
	}

	SPEX(et0phyaddr, SSB_SPROM1_ETHPHY, SSB_SPROM1_ETHPHY_ET0A, 0);
	SPEX(et1phyaddr, SSB_SPROM1_ETHPHY, SSB_SPROM1_ETHPHY_ET1A,
	     SSB_SPROM1_ETHPHY_ET1A_SHIFT);
	SPEX(et0mdcport, SSB_SPROM1_ETHPHY, SSB_SPROM1_ETHPHY_ET0M, 14);
	SPEX(et1mdcport, SSB_SPROM1_ETHPHY, SSB_SPROM1_ETHPHY_ET1M, 15);
	SPEX(board_rev, SSB_SPROM1_BINF, SSB_SPROM1_BINF_BREV, 0);
	SPEX(board_type, SSB_SPROM1_SPID, 0xFFFF, 0);
	if (out->revision == 1)
		SPEX(country_code, SSB_SPROM1_BINF, SSB_SPROM1_BINF_CCODE,
		     SSB_SPROM1_BINF_CCODE_SHIFT);
	SPEX(ant_available_a, SSB_SPROM1_BINF, SSB_SPROM1_BINF_ANTA,
	     SSB_SPROM1_BINF_ANTA_SHIFT);
	SPEX(ant_available_bg, SSB_SPROM1_BINF, SSB_SPROM1_BINF_ANTBG,
	     SSB_SPROM1_BINF_ANTBG_SHIFT);
	SPEX(pa0b0, SSB_SPROM1_PA0B0, 0xFFFF, 0);
	SPEX(pa0b1, SSB_SPROM1_PA0B1, 0xFFFF, 0);
	SPEX(pa0b2, SSB_SPROM1_PA0B2, 0xFFFF, 0);
	SPEX(pa1b0, SSB_SPROM1_PA1B0, 0xFFFF, 0);
	SPEX(pa1b1, SSB_SPROM1_PA1B1, 0xFFFF, 0);
	SPEX(pa1b2, SSB_SPROM1_PA1B2, 0xFFFF, 0);
	SPEX(gpio0, SSB_SPROM1_GPIOA, SSB_SPROM1_GPIOA_P0, 0);
	SPEX(gpio1, SSB_SPROM1_GPIOA, SSB_SPROM1_GPIOA_P1,
	     SSB_SPROM1_GPIOA_P1_SHIFT);
	SPEX(gpio2, SSB_SPROM1_GPIOB, SSB_SPROM1_GPIOB_P2, 0);
	SPEX(gpio3, SSB_SPROM1_GPIOB, SSB_SPROM1_GPIOB_P3,
	     SSB_SPROM1_GPIOB_P3_SHIFT);
	SPEX(maxpwr_a, SSB_SPROM1_MAXPWR, SSB_SPROM1_MAXPWR_A,
	     SSB_SPROM1_MAXPWR_A_SHIFT);
	SPEX(maxpwr_bg, SSB_SPROM1_MAXPWR, SSB_SPROM1_MAXPWR_BG, 0);
	SPEX(itssi_a, SSB_SPROM1_ITSSI, SSB_SPROM1_ITSSI_A,
	     SSB_SPROM1_ITSSI_A_SHIFT);
	SPEX(itssi_bg, SSB_SPROM1_ITSSI, SSB_SPROM1_ITSSI_BG, 0);
	SPEX(boardflags_lo, SSB_SPROM1_BFLLO, 0xFFFF, 0);

	SPEX(alpha2[0], SSB_SPROM1_CCODE, 0xff00, 8);
	SPEX(alpha2[1], SSB_SPROM1_CCODE, 0x00ff, 0);

	/* Extract the antenna gain values. */
	out->antenna_gain.a0 = sprom_extract_antgain(out->revision, in,
						     SSB_SPROM1_AGAIN,
						     SSB_SPROM1_AGAIN_BG,
						     SSB_SPROM1_AGAIN_BG_SHIFT);
	out->antenna_gain.a1 = sprom_extract_antgain(out->revision, in,
						     SSB_SPROM1_AGAIN,
						     SSB_SPROM1_AGAIN_A,
						     SSB_SPROM1_AGAIN_A_SHIFT);
	if (out->revision >= 2)
		sprom_extract_r23(out, in);
}

/* Revs 4 5 and 8 have partially shared layout */
static void sprom_extract_r458(struct ssb_sprom *out, const u16 *in)
{
	SPEX(txpid2g[0], SSB_SPROM4_TXPID2G01,
	     SSB_SPROM4_TXPID2G0, SSB_SPROM4_TXPID2G0_SHIFT);
	SPEX(txpid2g[1], SSB_SPROM4_TXPID2G01,
	     SSB_SPROM4_TXPID2G1, SSB_SPROM4_TXPID2G1_SHIFT);
	SPEX(txpid2g[2], SSB_SPROM4_TXPID2G23,
	     SSB_SPROM4_TXPID2G2, SSB_SPROM4_TXPID2G2_SHIFT);
	SPEX(txpid2g[3], SSB_SPROM4_TXPID2G23,
	     SSB_SPROM4_TXPID2G3, SSB_SPROM4_TXPID2G3_SHIFT);

	SPEX(txpid5gl[0], SSB_SPROM4_TXPID5GL01,
	     SSB_SPROM4_TXPID5GL0, SSB_SPROM4_TXPID5GL0_SHIFT);
	SPEX(txpid5gl[1], SSB_SPROM4_TXPID5GL01,
	     SSB_SPROM4_TXPID5GL1, SSB_SPROM4_TXPID5GL1_SHIFT);
	SPEX(txpid5gl[2], SSB_SPROM4_TXPID5GL23,
	     SSB_SPROM4_TXPID5GL2, SSB_SPROM4_TXPID5GL2_SHIFT);
	SPEX(txpid5gl[3], SSB_SPROM4_TXPID5GL23,
	     SSB_SPROM4_TXPID5GL3, SSB_SPROM4_TXPID5GL3_SHIFT);

	SPEX(txpid5g[0], SSB_SPROM4_TXPID5G01,
	     SSB_SPROM4_TXPID5G0, SSB_SPROM4_TXPID5G0_SHIFT);
	SPEX(txpid5g[1], SSB_SPROM4_TXPID5G01,
	     SSB_SPROM4_TXPID5G1, SSB_SPROM4_TXPID5G1_SHIFT);
	SPEX(txpid5g[2], SSB_SPROM4_TXPID5G23,
	     SSB_SPROM4_TXPID5G2, SSB_SPROM4_TXPID5G2_SHIFT);
	SPEX(txpid5g[3], SSB_SPROM4_TXPID5G23,
	     SSB_SPROM4_TXPID5G3, SSB_SPROM4_TXPID5G3_SHIFT);

	SPEX(txpid5gh[0], SSB_SPROM4_TXPID5GH01,
	     SSB_SPROM4_TXPID5GH0, SSB_SPROM4_TXPID5GH0_SHIFT);
	SPEX(txpid5gh[1], SSB_SPROM4_TXPID5GH01,
	     SSB_SPROM4_TXPID5GH1, SSB_SPROM4_TXPID5GH1_SHIFT);
	SPEX(txpid5gh[2], SSB_SPROM4_TXPID5GH23,
	     SSB_SPROM4_TXPID5GH2, SSB_SPROM4_TXPID5GH2_SHIFT);
	SPEX(txpid5gh[3], SSB_SPROM4_TXPID5GH23,
	     SSB_SPROM4_TXPID5GH3, SSB_SPROM4_TXPID5GH3_SHIFT);
}

static void sprom_extract_r45(struct ssb_sprom *out, const u16 *in)
{
	static const u16 pwr_info_offset[] = {
		SSB_SPROM4_PWR_INFO_CORE0, SSB_SPROM4_PWR_INFO_CORE1,
		SSB_SPROM4_PWR_INFO_CORE2, SSB_SPROM4_PWR_INFO_CORE3
	};
	int i;

	BUILD_BUG_ON(ARRAY_SIZE(pwr_info_offset) !=
		     ARRAY_SIZE(out->core_pwr_info));

	SPEX(et0phyaddr, SSB_SPROM4_ETHPHY, SSB_SPROM4_ETHPHY_ET0A, 0);
	SPEX(et1phyaddr, SSB_SPROM4_ETHPHY, SSB_SPROM4_ETHPHY_ET1A,
	     SSB_SPROM4_ETHPHY_ET1A_SHIFT);
	SPEX(board_rev, SSB_SPROM4_BOARDREV, 0xFFFF, 0);
	SPEX(board_type, SSB_SPROM1_SPID, 0xFFFF, 0);
	if (out->revision == 4) {
		SPEX(alpha2[0], SSB_SPROM4_CCODE, 0xff00, 8);
		SPEX(alpha2[1], SSB_SPROM4_CCODE, 0x00ff, 0);
		SPEX(boardflags_lo, SSB_SPROM4_BFLLO, 0xFFFF, 0);
		SPEX(boardflags_hi, SSB_SPROM4_BFLHI, 0xFFFF, 0);
		SPEX(boardflags2_lo, SSB_SPROM4_BFL2LO, 0xFFFF, 0);
		SPEX(boardflags2_hi, SSB_SPROM4_BFL2HI, 0xFFFF, 0);
	} else {
		SPEX(alpha2[0], SSB_SPROM5_CCODE, 0xff00, 8);
		SPEX(alpha2[1], SSB_SPROM5_CCODE, 0x00ff, 0);
		SPEX(boardflags_lo, SSB_SPROM5_BFLLO, 0xFFFF, 0);
		SPEX(boardflags_hi, SSB_SPROM5_BFLHI, 0xFFFF, 0);
		SPEX(boardflags2_lo, SSB_SPROM5_BFL2LO, 0xFFFF, 0);
		SPEX(boardflags2_hi, SSB_SPROM5_BFL2HI, 0xFFFF, 0);
	}
	SPEX(ant_available_a, SSB_SPROM4_ANTAVAIL, SSB_SPROM4_ANTAVAIL_A,
	     SSB_SPROM4_ANTAVAIL_A_SHIFT);
	SPEX(ant_available_bg, SSB_SPROM4_ANTAVAIL, SSB_SPROM4_ANTAVAIL_BG,
	     SSB_SPROM4_ANTAVAIL_BG_SHIFT);
	SPEX(maxpwr_bg, SSB_SPROM4_MAXP_BG, SSB_SPROM4_MAXP_BG_MASK, 0);
	SPEX(itssi_bg, SSB_SPROM4_MAXP_BG, SSB_SPROM4_ITSSI_BG,
	     SSB_SPROM4_ITSSI_BG_SHIFT);
	SPEX(maxpwr_a, SSB_SPROM4_MAXP_A, SSB_SPROM4_MAXP_A_MASK, 0);
	SPEX(itssi_a, SSB_SPROM4_MAXP_A, SSB_SPROM4_ITSSI_A,
	     SSB_SPROM4_ITSSI_A_SHIFT);
	if (out->revision == 4) {
		SPEX(gpio0, SSB_SPROM4_GPIOA, SSB_SPROM4_GPIOA_P0, 0);
		SPEX(gpio1, SSB_SPROM4_GPIOA, SSB_SPROM4_GPIOA_P1,
		     SSB_SPROM4_GPIOA_P1_SHIFT);
		SPEX(gpio2, SSB_SPROM4_GPIOB, SSB_SPROM4_GPIOB_P2, 0);
		SPEX(gpio3, SSB_SPROM4_GPIOB, SSB_SPROM4_GPIOB_P3,
		     SSB_SPROM4_GPIOB_P3_SHIFT);
	} else {
		SPEX(gpio0, SSB_SPROM5_GPIOA, SSB_SPROM5_GPIOA_P0, 0);
		SPEX(gpio1, SSB_SPROM5_GPIOA, SSB_SPROM5_GPIOA_P1,
		     SSB_SPROM5_GPIOA_P1_SHIFT);
		SPEX(gpio2, SSB_SPROM5_GPIOB, SSB_SPROM5_GPIOB_P2, 0);
		SPEX(gpio3, SSB_SPROM5_GPIOB, SSB_SPROM5_GPIOB_P3,
		     SSB_SPROM5_GPIOB_P3_SHIFT);
	}

	/* Extract the antenna gain values. */
	out->antenna_gain.a0 = sprom_extract_antgain(out->revision, in,
						     SSB_SPROM4_AGAIN01,
						     SSB_SPROM4_AGAIN0,
						     SSB_SPROM4_AGAIN0_SHIFT);
	out->antenna_gain.a1 = sprom_extract_antgain(out->revision, in,
						     SSB_SPROM4_AGAIN01,
						     SSB_SPROM4_AGAIN1,
						     SSB_SPROM4_AGAIN1_SHIFT);
	out->antenna_gain.a2 = sprom_extract_antgain(out->revision, in,
						     SSB_SPROM4_AGAIN23,
						     SSB_SPROM4_AGAIN2,
						     SSB_SPROM4_AGAIN2_SHIFT);
	out->antenna_gain.a3 = sprom_extract_antgain(out->revision, in,
						     SSB_SPROM4_AGAIN23,
						     SSB_SPROM4_AGAIN3,
						     SSB_SPROM4_AGAIN3_SHIFT);

	/* Extract cores power info info */
	for (i = 0; i < ARRAY_SIZE(pwr_info_offset); i++) {
		u16 o = pwr_info_offset[i];

		SPEX(core_pwr_info[i].itssi_2g, o + SSB_SPROM4_2G_MAXP_ITSSI,
			SSB_SPROM4_2G_ITSSI, SSB_SPROM4_2G_ITSSI_SHIFT);
		SPEX(core_pwr_info[i].maxpwr_2g, o + SSB_SPROM4_2G_MAXP_ITSSI,
			SSB_SPROM4_2G_MAXP, 0);

		SPEX(core_pwr_info[i].pa_2g[0], o + SSB_SPROM4_2G_PA_0, ~0, 0);
		SPEX(core_pwr_info[i].pa_2g[1], o + SSB_SPROM4_2G_PA_1, ~0, 0);
		SPEX(core_pwr_info[i].pa_2g[2], o + SSB_SPROM4_2G_PA_2, ~0, 0);
		SPEX(core_pwr_info[i].pa_2g[3], o + SSB_SPROM4_2G_PA_3, ~0, 0);

		SPEX(core_pwr_info[i].itssi_5g, o + SSB_SPROM4_5G_MAXP_ITSSI,
			SSB_SPROM4_5G_ITSSI, SSB_SPROM4_5G_ITSSI_SHIFT);
		SPEX(core_pwr_info[i].maxpwr_5g, o + SSB_SPROM4_5G_MAXP_ITSSI,
			SSB_SPROM4_5G_MAXP, 0);
		SPEX(core_pwr_info[i].maxpwr_5gh, o + SSB_SPROM4_5GHL_MAXP,
			SSB_SPROM4_5GH_MAXP, 0);
		SPEX(core_pwr_info[i].maxpwr_5gl, o + SSB_SPROM4_5GHL_MAXP,
			SSB_SPROM4_5GL_MAXP, SSB_SPROM4_5GL_MAXP_SHIFT);

		SPEX(core_pwr_info[i].pa_5gl[0], o + SSB_SPROM4_5GL_PA_0, ~0, 0);
		SPEX(core_pwr_info[i].pa_5gl[1], o + SSB_SPROM4_5GL_PA_1, ~0, 0);
		SPEX(core_pwr_info[i].pa_5gl[2], o + SSB_SPROM4_5GL_PA_2, ~0, 0);
		SPEX(core_pwr_info[i].pa_5gl[3], o + SSB_SPROM4_5GL_PA_3, ~0, 0);
		SPEX(core_pwr_info[i].pa_5g[0], o + SSB_SPROM4_5G_PA_0, ~0, 0);
		SPEX(core_pwr_info[i].pa_5g[1], o + SSB_SPROM4_5G_PA_1, ~0, 0);
		SPEX(core_pwr_info[i].pa_5g[2], o + SSB_SPROM4_5G_PA_2, ~0, 0);
		SPEX(core_pwr_info[i].pa_5g[3], o + SSB_SPROM4_5G_PA_3, ~0, 0);
		SPEX(core_pwr_info[i].pa_5gh[0], o + SSB_SPROM4_5GH_PA_0, ~0, 0);
		SPEX(core_pwr_info[i].pa_5gh[1], o + SSB_SPROM4_5GH_PA_1, ~0, 0);
		SPEX(core_pwr_info[i].pa_5gh[2], o + SSB_SPROM4_5GH_PA_2, ~0, 0);
		SPEX(core_pwr_info[i].pa_5gh[3], o + SSB_SPROM4_5GH_PA_3, ~0, 0);
	}

	sprom_extract_r458(out, in);

	/* TODO - get remaining rev 4 stuff needed */
}

static void sprom_extract_r8(struct ssb_sprom *out, const u16 *in)
{
	int i;
	u16 o;
	static const u16 pwr_info_offset[] = {
		SSB_SROM8_PWR_INFO_CORE0, SSB_SROM8_PWR_INFO_CORE1,
		SSB_SROM8_PWR_INFO_CORE2, SSB_SROM8_PWR_INFO_CORE3
	};
	BUILD_BUG_ON(ARRAY_SIZE(pwr_info_offset) !=
			ARRAY_SIZE(out->core_pwr_info));

	SPEX(board_rev, SSB_SPROM8_BOARDREV, 0xFFFF, 0);
	SPEX(board_type, SSB_SPROM1_SPID, 0xFFFF, 0);
	SPEX(alpha2[0], SSB_SPROM8_CCODE, 0xff00, 8);
	SPEX(alpha2[1], SSB_SPROM8_CCODE, 0x00ff, 0);
	SPEX(boardflags_lo, SSB_SPROM8_BFLLO, 0xFFFF, 0);
	SPEX(boardflags_hi, SSB_SPROM8_BFLHI, 0xFFFF, 0);
	SPEX(boardflags2_lo, SSB_SPROM8_BFL2LO, 0xFFFF, 0);
	SPEX(boardflags2_hi, SSB_SPROM8_BFL2HI, 0xFFFF, 0);
	SPEX(ant_available_a, SSB_SPROM8_ANTAVAIL, SSB_SPROM8_ANTAVAIL_A,
	     SSB_SPROM8_ANTAVAIL_A_SHIFT);
	SPEX(ant_available_bg, SSB_SPROM8_ANTAVAIL, SSB_SPROM8_ANTAVAIL_BG,
	     SSB_SPROM8_ANTAVAIL_BG_SHIFT);
	SPEX(maxpwr_bg, SSB_SPROM8_MAXP_BG, SSB_SPROM8_MAXP_BG_MASK, 0);
	SPEX(itssi_bg, SSB_SPROM8_MAXP_BG, SSB_SPROM8_ITSSI_BG,
	     SSB_SPROM8_ITSSI_BG_SHIFT);
	SPEX(maxpwr_a, SSB_SPROM8_MAXP_A, SSB_SPROM8_MAXP_A_MASK, 0);
	SPEX(itssi_a, SSB_SPROM8_MAXP_A, SSB_SPROM8_ITSSI_A,
	     SSB_SPROM8_ITSSI_A_SHIFT);
	SPEX(maxpwr_ah, SSB_SPROM8_MAXP_AHL, SSB_SPROM8_MAXP_AH_MASK, 0);
	SPEX(maxpwr_al, SSB_SPROM8_MAXP_AHL, SSB_SPROM8_MAXP_AL_MASK,
	     SSB_SPROM8_MAXP_AL_SHIFT);
	SPEX(gpio0, SSB_SPROM8_GPIOA, SSB_SPROM8_GPIOA_P0, 0);
	SPEX(gpio1, SSB_SPROM8_GPIOA, SSB_SPROM8_GPIOA_P1,
	     SSB_SPROM8_GPIOA_P1_SHIFT);
	SPEX(gpio2, SSB_SPROM8_GPIOB, SSB_SPROM8_GPIOB_P2, 0);
	SPEX(gpio3, SSB_SPROM8_GPIOB, SSB_SPROM8_GPIOB_P3,
	     SSB_SPROM8_GPIOB_P3_SHIFT);
	SPEX(tri2g, SSB_SPROM8_TRI25G, SSB_SPROM8_TRI2G, 0);
	SPEX(tri5g, SSB_SPROM8_TRI25G, SSB_SPROM8_TRI5G,
	     SSB_SPROM8_TRI5G_SHIFT);
	SPEX(tri5gl, SSB_SPROM8_TRI5GHL, SSB_SPROM8_TRI5GL, 0);
	SPEX(tri5gh, SSB_SPROM8_TRI5GHL, SSB_SPROM8_TRI5GH,
	     SSB_SPROM8_TRI5GH_SHIFT);
	SPEX(rxpo2g, SSB_SPROM8_RXPO, SSB_SPROM8_RXPO2G, 0);
	SPEX(rxpo5g, SSB_SPROM8_RXPO, SSB_SPROM8_RXPO5G,
	     SSB_SPROM8_RXPO5G_SHIFT);
	SPEX(rssismf2g, SSB_SPROM8_RSSIPARM2G, SSB_SPROM8_RSSISMF2G, 0);
	SPEX(rssismc2g, SSB_SPROM8_RSSIPARM2G, SSB_SPROM8_RSSISMC2G,
	     SSB_SPROM8_RSSISMC2G_SHIFT);
	SPEX(rssisav2g, SSB_SPROM8_RSSIPARM2G, SSB_SPROM8_RSSISAV2G,
	     SSB_SPROM8_RSSISAV2G_SHIFT);
	SPEX(bxa2g, SSB_SPROM8_RSSIPARM2G, SSB_SPROM8_BXA2G,
	     SSB_SPROM8_BXA2G_SHIFT);
	SPEX(rssismf5g, SSB_SPROM8_RSSIPARM5G, SSB_SPROM8_RSSISMF5G, 0);
	SPEX(rssismc5g, SSB_SPROM8_RSSIPARM5G, SSB_SPROM8_RSSISMC5G,
	     SSB_SPROM8_RSSISMC5G_SHIFT);
	SPEX(rssisav5g, SSB_SPROM8_RSSIPARM5G, SSB_SPROM8_RSSISAV5G,
	     SSB_SPROM8_RSSISAV5G_SHIFT);
	SPEX(bxa5g, SSB_SPROM8_RSSIPARM5G, SSB_SPROM8_BXA5G,
	     SSB_SPROM8_BXA5G_SHIFT);
	SPEX(pa0b0, SSB_SPROM8_PA0B0, 0xFFFF, 0);
	SPEX(pa0b1, SSB_SPROM8_PA0B1, 0xFFFF, 0);
	SPEX(pa0b2, SSB_SPROM8_PA0B2, 0xFFFF, 0);
	SPEX(pa1b0, SSB_SPROM8_PA1B0, 0xFFFF, 0);
	SPEX(pa1b1, SSB_SPROM8_PA1B1, 0xFFFF, 0);
	SPEX(pa1b2, SSB_SPROM8_PA1B2, 0xFFFF, 0);
	SPEX(pa1lob0, SSB_SPROM8_PA1LOB0, 0xFFFF, 0);
	SPEX(pa1lob1, SSB_SPROM8_PA1LOB1, 0xFFFF, 0);
	SPEX(pa1lob2, SSB_SPROM8_PA1LOB2, 0xFFFF, 0);
	SPEX(pa1hib0, SSB_SPROM8_PA1HIB0, 0xFFFF, 0);
	SPEX(pa1hib1, SSB_SPROM8_PA1HIB1, 0xFFFF, 0);
	SPEX(pa1hib2, SSB_SPROM8_PA1HIB2, 0xFFFF, 0);
	SPEX(cck2gpo, SSB_SPROM8_CCK2GPO, 0xFFFF, 0);
	SPEX32(ofdm2gpo, SSB_SPROM8_OFDM2GPO, 0xFFFFFFFF, 0);
	SPEX32(ofdm5glpo, SSB_SPROM8_OFDM5GLPO, 0xFFFFFFFF, 0);
	SPEX32(ofdm5gpo, SSB_SPROM8_OFDM5GPO, 0xFFFFFFFF, 0);
	SPEX32(ofdm5ghpo, SSB_SPROM8_OFDM5GHPO, 0xFFFFFFFF, 0);

	/* Extract the antenna gain values. */
	out->antenna_gain.a0 = sprom_extract_antgain(out->revision, in,
						     SSB_SPROM8_AGAIN01,
						     SSB_SPROM8_AGAIN0,
						     SSB_SPROM8_AGAIN0_SHIFT);
	out->antenna_gain.a1 = sprom_extract_antgain(out->revision, in,
						     SSB_SPROM8_AGAIN01,
						     SSB_SPROM8_AGAIN1,
						     SSB_SPROM8_AGAIN1_SHIFT);
	out->antenna_gain.a2 = sprom_extract_antgain(out->revision, in,
						     SSB_SPROM8_AGAIN23,
						     SSB_SPROM8_AGAIN2,
						     SSB_SPROM8_AGAIN2_SHIFT);
	out->antenna_gain.a3 = sprom_extract_antgain(out->revision, in,
						     SSB_SPROM8_AGAIN23,
						     SSB_SPROM8_AGAIN3,
						     SSB_SPROM8_AGAIN3_SHIFT);

	/* Extract cores power info info */
	for (i = 0; i < ARRAY_SIZE(pwr_info_offset); i++) {
		o = pwr_info_offset[i];
		SPEX(core_pwr_info[i].itssi_2g, o + SSB_SROM8_2G_MAXP_ITSSI,
			SSB_SPROM8_2G_ITSSI, SSB_SPROM8_2G_ITSSI_SHIFT);
		SPEX(core_pwr_info[i].maxpwr_2g, o + SSB_SROM8_2G_MAXP_ITSSI,
			SSB_SPROM8_2G_MAXP, 0);

		SPEX(core_pwr_info[i].pa_2g[0], o + SSB_SROM8_2G_PA_0, ~0, 0);
		SPEX(core_pwr_info[i].pa_2g[1], o + SSB_SROM8_2G_PA_1, ~0, 0);
		SPEX(core_pwr_info[i].pa_2g[2], o + SSB_SROM8_2G_PA_2, ~0, 0);

		SPEX(core_pwr_info[i].itssi_5g, o + SSB_SROM8_5G_MAXP_ITSSI,
			SSB_SPROM8_5G_ITSSI, SSB_SPROM8_5G_ITSSI_SHIFT);
		SPEX(core_pwr_info[i].maxpwr_5g, o + SSB_SROM8_5G_MAXP_ITSSI,
			SSB_SPROM8_5G_MAXP, 0);
		SPEX(core_pwr_info[i].maxpwr_5gh, o + SSB_SPROM8_5GHL_MAXP,
			SSB_SPROM8_5GH_MAXP, 0);
		SPEX(core_pwr_info[i].maxpwr_5gl, o + SSB_SPROM8_5GHL_MAXP,
			SSB_SPROM8_5GL_MAXP, SSB_SPROM8_5GL_MAXP_SHIFT);

		SPEX(core_pwr_info[i].pa_5gl[0], o + SSB_SROM8_5GL_PA_0, ~0, 0);
		SPEX(core_pwr_info[i].pa_5gl[1], o + SSB_SROM8_5GL_PA_1, ~0, 0);
		SPEX(core_pwr_info[i].pa_5gl[2], o + SSB_SROM8_5GL_PA_2, ~0, 0);
		SPEX(core_pwr_info[i].pa_5g[0], o + SSB_SROM8_5G_PA_0, ~0, 0);
		SPEX(core_pwr_info[i].pa_5g[1], o + SSB_SROM8_5G_PA_1, ~0, 0);
		SPEX(core_pwr_info[i].pa_5g[2], o + SSB_SROM8_5G_PA_2, ~0, 0);
		SPEX(core_pwr_info[i].pa_5gh[0], o + SSB_SROM8_5GH_PA_0, ~0, 0);
		SPEX(core_pwr_info[i].pa_5gh[1], o + SSB_SROM8_5GH_PA_1, ~0, 0);
		SPEX(core_pwr_info[i].pa_5gh[2], o + SSB_SROM8_5GH_PA_2, ~0, 0);
	}

	/* Extract FEM info */
	SPEX(fem.ghz2.tssipos, SSB_SPROM8_FEM2G,
		SSB_SROM8_FEM_TSSIPOS, SSB_SROM8_FEM_TSSIPOS_SHIFT);
	SPEX(fem.ghz2.extpa_gain, SSB_SPROM8_FEM2G,
		SSB_SROM8_FEM_EXTPA_GAIN, SSB_SROM8_FEM_EXTPA_GAIN_SHIFT);
	SPEX(fem.ghz2.pdet_range, SSB_SPROM8_FEM2G,
		SSB_SROM8_FEM_PDET_RANGE, SSB_SROM8_FEM_PDET_RANGE_SHIFT);
	SPEX(fem.ghz2.tr_iso, SSB_SPROM8_FEM2G,
		SSB_SROM8_FEM_TR_ISO, SSB_SROM8_FEM_TR_ISO_SHIFT);
	SPEX(fem.ghz2.antswlut, SSB_SPROM8_FEM2G,
		SSB_SROM8_FEM_ANTSWLUT, SSB_SROM8_FEM_ANTSWLUT_SHIFT);

	SPEX(fem.ghz5.tssipos, SSB_SPROM8_FEM5G,
		SSB_SROM8_FEM_TSSIPOS, SSB_SROM8_FEM_TSSIPOS_SHIFT);
	SPEX(fem.ghz5.extpa_gain, SSB_SPROM8_FEM5G,
		SSB_SROM8_FEM_EXTPA_GAIN, SSB_SROM8_FEM_EXTPA_GAIN_SHIFT);
	SPEX(fem.ghz5.pdet_range, SSB_SPROM8_FEM5G,
		SSB_SROM8_FEM_PDET_RANGE, SSB_SROM8_FEM_PDET_RANGE_SHIFT);
	SPEX(fem.ghz5.tr_iso, SSB_SPROM8_FEM5G,
		SSB_SROM8_FEM_TR_ISO, SSB_SROM8_FEM_TR_ISO_SHIFT);
	SPEX(fem.ghz5.antswlut, SSB_SPROM8_FEM5G,
		SSB_SROM8_FEM_ANTSWLUT, SSB_SROM8_FEM_ANTSWLUT_SHIFT);

	SPEX(leddc_on_time, SSB_SPROM8_LEDDC, SSB_SPROM8_LEDDC_ON,
	     SSB_SPROM8_LEDDC_ON_SHIFT);
	SPEX(leddc_off_time, SSB_SPROM8_LEDDC, SSB_SPROM8_LEDDC_OFF,
	     SSB_SPROM8_LEDDC_OFF_SHIFT);

	SPEX(txchain, SSB_SPROM8_TXRXC, SSB_SPROM8_TXRXC_TXCHAIN,
	     SSB_SPROM8_TXRXC_TXCHAIN_SHIFT);
	SPEX(rxchain, SSB_SPROM8_TXRXC, SSB_SPROM8_TXRXC_RXCHAIN,
	     SSB_SPROM8_TXRXC_RXCHAIN_SHIFT);
	SPEX(antswitch, SSB_SPROM8_TXRXC, SSB_SPROM8_TXRXC_SWITCH,
	     SSB_SPROM8_TXRXC_SWITCH_SHIFT);

	SPEX(opo, SSB_SPROM8_OFDM2GPO, 0x00ff, 0);

	SPEX_ARRAY8(mcs2gpo, SSB_SPROM8_2G_MCSPO, ~0, 0);
	SPEX_ARRAY8(mcs5gpo, SSB_SPROM8_5G_MCSPO, ~0, 0);
	SPEX_ARRAY8(mcs5glpo, SSB_SPROM8_5GL_MCSPO, ~0, 0);
	SPEX_ARRAY8(mcs5ghpo, SSB_SPROM8_5GH_MCSPO, ~0, 0);

	SPEX(rawtempsense, SSB_SPROM8_RAWTS, SSB_SPROM8_RAWTS_RAWTEMP,
	     SSB_SPROM8_RAWTS_RAWTEMP_SHIFT);
	SPEX(measpower, SSB_SPROM8_RAWTS, SSB_SPROM8_RAWTS_MEASPOWER,
	     SSB_SPROM8_RAWTS_MEASPOWER_SHIFT);
	SPEX(tempsense_slope, SSB_SPROM8_OPT_CORRX,
	     SSB_SPROM8_OPT_CORRX_TEMP_SLOPE,
	     SSB_SPROM8_OPT_CORRX_TEMP_SLOPE_SHIFT);
	SPEX(tempcorrx, SSB_SPROM8_OPT_CORRX, SSB_SPROM8_OPT_CORRX_TEMPCORRX,
	     SSB_SPROM8_OPT_CORRX_TEMPCORRX_SHIFT);
	SPEX(tempsense_option, SSB_SPROM8_OPT_CORRX,
	     SSB_SPROM8_OPT_CORRX_TEMP_OPTION,
	     SSB_SPROM8_OPT_CORRX_TEMP_OPTION_SHIFT);
	SPEX(freqoffset_corr, SSB_SPROM8_HWIQ_IQSWP,
	     SSB_SPROM8_HWIQ_IQSWP_FREQ_CORR,
	     SSB_SPROM8_HWIQ_IQSWP_FREQ_CORR_SHIFT);
	SPEX(iqcal_swp_dis, SSB_SPROM8_HWIQ_IQSWP,
	     SSB_SPROM8_HWIQ_IQSWP_IQCAL_SWP,
	     SSB_SPROM8_HWIQ_IQSWP_IQCAL_SWP_SHIFT);
	SPEX(hw_iqcal_en, SSB_SPROM8_HWIQ_IQSWP, SSB_SPROM8_HWIQ_IQSWP_HW_IQCAL,
	     SSB_SPROM8_HWIQ_IQSWP_HW_IQCAL_SHIFT);

	SPEX(bw40po, SSB_SPROM8_BW40PO, ~0, 0);
	SPEX(cddpo, SSB_SPROM8_CDDPO, ~0, 0);
	SPEX(stbcpo, SSB_SPROM8_STBCPO, ~0, 0);
	SPEX(bwduppo, SSB_SPROM8_BWDUPPO, ~0, 0);

	SPEX(tempthresh, SSB_SPROM8_THERMAL, SSB_SPROM8_THERMAL_TRESH,
	     SSB_SPROM8_THERMAL_TRESH_SHIFT);
	SPEX(tempoffset, SSB_SPROM8_THERMAL, SSB_SPROM8_THERMAL_OFFSET,
	     SSB_SPROM8_THERMAL_OFFSET_SHIFT);
	SPEX(phycal_tempdelta, SSB_SPROM8_TEMPDELTA,
	     SSB_SPROM8_TEMPDELTA_PHYCAL,
	     SSB_SPROM8_TEMPDELTA_PHYCAL_SHIFT);
	SPEX(temps_period, SSB_SPROM8_TEMPDELTA, SSB_SPROM8_TEMPDELTA_PERIOD,
	     SSB_SPROM8_TEMPDELTA_PERIOD_SHIFT);
	SPEX(temps_hysteresis, SSB_SPROM8_TEMPDELTA,
	     SSB_SPROM8_TEMPDELTA_HYSTERESIS,
	     SSB_SPROM8_TEMPDELTA_HYSTERESIS_SHIFT);
	sprom_extract_r458(out, in);

	/* TODO - get remaining rev 8 stuff needed */
}

static int sprom_extract(struct ssb_sprom *out, const u16 *in, u16 size)
{
	memset(out, 0, sizeof(*out));

	out->revision = in[size - 1] & 0x00FF;
	memset(out->et0mac, 0xFF, 6);		/* preset et0 and et1 mac */
	memset(out->et1mac, 0xFF, 6);

	switch (out->revision) {
	case 1:
	case 2:
	case 3:
		sprom_extract_r123(out, in);
		break;
	case 4:
	case 5:
		sprom_extract_r45(out, in);
		break;
	case 8:
		sprom_extract_r8(out, in);
		break;
	default:
		pr_warn("Unsupported SPROM revision %d detected. Will extract v1\n",
			out->revision);
		out->revision = 1;
		sprom_extract_r123(out, in);
	}

	if (out->boardflags_lo == 0xFFFF)
		out->boardflags_lo = 0;  /* per specs */
	if (out->boardflags_hi == 0xFFFF)
		out->boardflags_hi = 0;  /* per specs */

	return 0;
}

static void b43_sprom_fixup(struct b43_sprom *priv, u16 *sprom)
{
	struct device_node *node = priv->dev->of_node;
	u32 fixups, off, val;
	int i = 0;

	if (!of_get_property(node, "brcm,sprom-fixups", &fixups))
		return;

	fixups /= sizeof(u32);

	dev_info(priv->dev, "patching SPROM with %u fixups...\n", fixups >> 1);

	while (i < fixups) {
		if (of_property_read_u32_index(node, "brcm,sprom-fixups",
					       i++, &off)) {
			dev_err(priv->dev, "error reading fixup[%u] offset\n",
				i - 1);
			return;
		}

		if (of_property_read_u32_index(node, "brcm,sprom-fixups",
					       i++, &val)) {
			dev_err(priv->dev, "error reading fixup[%u] value\n",
				i - 1);
			return;
		}

		dev_dbg(priv->dev, "fixup[%d]=0x%04x\n", off, val);

		sprom[off] = val;
	}
}

int sprom_override_devid(struct b43_sprom *priv, struct ssb_sprom *out,
			 const u16 *in)
{
	const struct b43_sprom_raw *raw = of_device_get_match_data(priv->dev);

	if (raw->type == B43_SPROM_SSB) {
		SPEX(dev_id, SSB_SPROM1_PID, 0xFFFF, 0);
		return !!out->dev_id;
	} else if (raw->type == B43_SPROM_BCMA) {
		SPEX(dev_id, 0x0060, 0xFFFF, 0);
		return !!out->dev_id;
	}

	return 0;
}

static void b43_sprom_set(struct b43_sprom *priv)
{
	const struct b43_sprom_raw *raw = of_device_get_match_data(priv->dev);
	struct ssb_sprom *sprom = &priv->sprom;

	if (raw) {
		u16 template_sprom[220];

		memcpy(template_sprom, raw->map, raw->size * sizeof(u16));
		b43_sprom_fixup(priv, template_sprom);
		sprom_extract(sprom, template_sprom, raw->size);

		priv->devid_override = sprom_override_devid(priv, sprom,
							    template_sprom);
	} else {
		sprom->revision = 0x02;
		sprom->board_rev = 0x0017;
		sprom->country_code = 0x00;
		sprom->ant_available_bg = 0x03;
		sprom->pa0b0 = 0x15ae;
		sprom->pa0b1 = 0xfa85;
		sprom->pa0b2 = 0xfe8d;
		sprom->pa1b0 = 0xffff;
		sprom->pa1b1 = 0xffff;
		sprom->pa1b2 = 0xffff;
		sprom->gpio0 = 0xff;
		sprom->gpio1 = 0xff;
		sprom->gpio2 = 0xff;
		sprom->gpio3 = 0xff;
		sprom->maxpwr_bg = 0x4c;
		sprom->itssi_bg = 0x00;
		sprom->boardflags_lo = 0x2848;
		sprom->boardflags_hi = 0x0000;
		priv->devid_override = 0;
	}
}

#if defined(CONFIG_SSB_PCIHOST)
int b43_get_fallback_ssb_sprom(struct ssb_bus *bus, struct ssb_sprom *out)
{
	struct b43_sprom *priv = &b43_sprom;

	if (bus->bustype == SSB_BUSTYPE_PCI) {
		if (bus->host_pci->bus->number != priv->pci_bus ||
		    PCI_SLOT(bus->host_pci->devfn) != priv->pci_dev)
			pr_warn("ssb_fallback_sprom: pci bus/device num "
				"mismatch: expected %i/%i, but got %i/%i\n",
				priv->pci_bus, priv->pci_dev,
				bus->host_pci->bus->number,
				PCI_SLOT(bus->host_pci->devfn));
		if (priv->devid_override)
			bus->host_pci->device = priv->sprom.dev_id;
		memcpy(out, &priv->sprom, sizeof(struct ssb_sprom));
		return 0;
	} else {
		pr_err("%s: unable to fill SPROM for given bustype.\n",
		       __func__);
		return -EINVAL;
	}
}
#endif /* CONFIG_SSB_PCIHOST */

#if defined(CONFIG_BCMA_HOST_PCI)
int b43_get_fallback_bcma_sprom(struct bcma_bus *bus, struct ssb_sprom *out)
{
	struct b43_sprom *priv = &b43_sprom;

	if (bus->hosttype == BCMA_HOSTTYPE_PCI) {
		if (bus->host_pci->bus->number != priv->pci_bus ||
		    PCI_SLOT(bus->host_pci->devfn) != priv->pci_dev)
			pr_warn("bcma_fallback_sprom: pci bus/device num "
				"mismatch: expected %i/%i, but got %i/%i\n",
				priv->pci_bus, priv->pci_dev,
				bus->host_pci->bus->number,
				PCI_SLOT(bus->host_pci->devfn));
		if (priv->devid_override)
			bus->host_pci->device = priv->sprom.dev_id;
		memcpy(out, &priv->sprom, sizeof(struct ssb_sprom));
		return 0;
	} else {
		pr_err("%s: unable to fill SPROM for given bustype.\n",
		       __func__);
		return -EINVAL;
	}
}
#endif /* CONFIG_BCMA_HOST_PCI */

static int b43_sprom_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *node = dev->of_node;
	struct b43_sprom *priv = &b43_sprom;
	const void *mac;
	int ret;

	priv->dev = dev;

	b43_sprom_set(priv);

	of_property_read_u32(node, "pci-bus", &priv->pci_bus);
	of_property_read_u32(node, "pci-dev", &priv->pci_dev);

	mac = of_get_mac_address(node);
	if (!IS_ERR_OR_NULL(mac)) {
		memcpy(priv->mac, mac, ETH_ALEN);
		dev_info(dev, "mtd mac %pM\n", priv->mac);
	} else {
		random_ether_addr(priv->mac);
		dev_info(dev, "random mac %pM\n", priv->mac);
	}

	memcpy(priv->sprom.il0mac, priv->mac, ETH_ALEN);
 	memcpy(priv->sprom.et0mac, priv->mac, ETH_ALEN);
 	memcpy(priv->sprom.et1mac, priv->mac, ETH_ALEN);

#if defined(CONFIG_SSB_PCIHOST)
	ret = ssb_arch_register_fallback_sprom(&b43_get_fallback_ssb_sprom);
	if (ret)
		return ret;
#endif /* CONFIG_SSB_PCIHOST */

#if defined(CONFIG_BCMA_HOST_PCI)
	ret = bcma_arch_register_fallback_sprom(b43_get_fallback_bcma_sprom);
	if (ret)
		return ret;
#endif /* CONFIG_BCMA_HOST_PCI */

	return 0;
}

static const struct of_device_id b43_sprom_of_match[] = {
	{ .compatible = "brcm,bcm43xx-sprom", .data = NULL },
#if defined (CONFIG_SSB_PCIHOST)
	{ .compatible = "brcm,bcm4306-sprom", .data = &bcm4306_sprom, },
	{ .compatible = "brcm,bcm4321-sprom", .data = &bcm4321_sprom, },
	{ .compatible = "brcm,bcm4322-sprom", .data = &bcm4322_sprom, },
	{ .compatible = "brcm,bcm43222-sprom", .data = &bcm43222_sprom, },
#endif /* CONFIG_BCMA_HOST_PCI */
#if defined(CONFIG_BCMA_HOST_PCI)
	{ .compatible = "brcm,bcm4313-sprom", .data = &bcm4313_sprom, },
	{ .compatible = "brcm,bcm4331-sprom", .data = &bcm4331_sprom, },
	{ .compatible = "brcm,bcm43131-sprom", .data = &bcm43131_sprom, },
	{ .compatible = "brcm,bcm43217-sprom", .data = &bcm43217_sprom, },
	{ .compatible = "brcm,bcm43225-sprom", .data = &bcm43225_sprom, },
	{ .compatible = "brcm,bcm43227-sprom", .data = &bcm43227_sprom, },
	{ .compatible = "brcm,bcm43228-sprom", .data = &bcm43228_sprom, },
#endif /* CONFIG_BCMA_HOST_PCI */
	{ /* sentinel */ }
};

static struct platform_driver b43_sprom_driver = {
	.probe = b43_sprom_probe,
	.driver	= {
		.name = "b43-sprom",
		.of_match_table = b43_sprom_of_match,
	},
};

int __init b43_sprom_init(void)
{
	int ret = platform_driver_register(&b43_sprom_driver);
	if (ret)
		pr_err("b43-sprom: Error registering platform driver!\n");
	return ret;
}
late_initcall(b43_sprom_init);
