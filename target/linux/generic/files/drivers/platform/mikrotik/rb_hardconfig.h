// SPDX-License-Identifier: GPL-2.0-only
/*
 * Common definitions for MikroTik RouterBoot hard config data.
 *
 * Copyright (C) 2020 Thibaut VARÈNE <hacks+kernel@slashdirt.org>
 *
 * Some constant defines extracted from routerboot.{c,h} by Gabor Juhos
 * <juhosg@openwrt.org>
 */

#ifndef _ROUTERBOOT_HARD_CONFIG_H_
#define _ROUTERBOOT_HARD_CONFIG_H_

/* ID values for hardware settings */
#define RB_ID_FLASH_INFO		0x03
#define RB_ID_MAC_ADDRESS_PACK		0x04
#define RB_ID_BOARD_PRODUCT_CODE	0x05
#define RB_ID_BIOS_VERSION		0x06
#define RB_ID_SDRAM_TIMINGS		0x08
#define RB_ID_DEVICE_TIMINGS		0x09
#define RB_ID_SOFTWARE_ID		0x0A
#define RB_ID_SERIAL_NUMBER		0x0B
#define RB_ID_MEMORY_SIZE		0x0D
#define RB_ID_MAC_ADDRESS_COUNT		0x0E
#define RB_ID_HW_OPTIONS		0x15
#define RB_ID_WLAN_DATA			0x16
#define RB_ID_BOARD_IDENTIFIER		0x17
#define RB_ID_PRODUCT_NAME		0x21
#define RB_ID_DEFCONF			0x26
#define RB_ID_BOARD_REVISION		0x27

#endif /* _ROUTERBOOT_HARD_CONFIG_H_ */
