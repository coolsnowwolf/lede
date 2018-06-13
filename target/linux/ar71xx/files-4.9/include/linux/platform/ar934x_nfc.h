/*
 * Platform data definition for the built-in NAND controller of the
 * Atheros AR934x SoCs
 *
 * Copyright (C) 2011-2012 Gabor Juhos <juhosg@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */

#ifndef _AR934X_NFC_PLATFORM_H
#define _AR934X_NFC_PLATFORM_H

#define AR934X_NFC_DRIVER_NAME		"ar934x-nfc"

struct mtd_info;
struct mtd_partition;

enum ar934x_nfc_ecc_mode {
	AR934X_NFC_ECC_SOFT = 0,
	AR934X_NFC_ECC_HW,
	AR934X_NFC_ECC_SOFT_BCH,
};

struct ar934x_nfc_platform_data {
	const char *name;
	struct mtd_partition *parts;
	int nr_parts;

	bool swap_dma;
	enum ar934x_nfc_ecc_mode ecc_mode;

	void (*hw_reset)(bool active);
	void (*select_chip)(int chip_no);
	int (*scan_fixup)(struct mtd_info *mtd);
};

#endif /* _AR934X_NFC_PLATFORM_H */
