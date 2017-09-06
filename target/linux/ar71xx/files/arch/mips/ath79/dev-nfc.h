/*
 *  Atheros AR934X SoCs built-in NAND Flash Controller support
 *
 *  Copyright (C) 2011-2012 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#ifndef _ATH79_DEV_NFC_H
#define _ATH79_DEV_NFC_H

struct mtd_partition;
enum ar934x_nfc_ecc_mode;

#ifdef CONFIG_ATH79_DEV_NFC
void ath79_nfc_set_parts(struct mtd_partition *parts, int nr_parts);
void ath79_nfc_set_select_chip(void (*f)(int chip_no));
void ath79_nfc_set_scan_fixup(int (*f)(struct mtd_info *mtd));
void ath79_nfc_set_swap_dma(bool enable);
void ath79_nfc_set_ecc_mode(enum ar934x_nfc_ecc_mode mode);
void ath79_register_nfc(void);
#else
static inline void ath79_nfc_set_parts(struct mtd_partition *parts,
				       int nr_parts) {}
static inline void ath79_nfc_set_select_chip(void (*f)(int chip_no)) {}
static inline void ath79_nfc_set_scan_fixup(int (*f)(struct mtd_info *mtd)) {}
static inline void ath79_nfc_set_swap_dma(bool enable) {}
static inline void ath79_nfc_set_ecc_mode(enum ar934x_nfc_ecc_mode mode) {}
static inline void ath79_register_nfc(void) {}
#endif

#endif /* _ATH79_DEV_NFC_H */
