/*
 *  Mikrotik's RouterBOOT definitions
 *
 *  Copyright (C) 2007-2008 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */

#ifndef _ROUTERBOOT_H
#define _ROUTERBOOT_H

#define RB_MAC_SIZE		6

/*
 * Magic numbers
 */
#define RB_MAGIC_HARD	0x64726148 /* "Hard" */
#define RB_MAGIC_SOFT	0x74666F53 /* "Soft" */
#define RB_MAGIC_DAWN	0x6E776144 /* "Dawn" */

#define RB_ID_TERMINATOR	0

/*
 * ID values for Hardware settings
 */
#define RB_ID_HARD_01		1
#define RB_ID_HARD_02		2
#define RB_ID_FLASH_INFO	3
#define RB_ID_MAC_ADDRESS_PACK	4
#define RB_ID_BOARD_NAME	5
#define RB_ID_BIOS_VERSION	6
#define RB_ID_HARD_07		7
#define RB_ID_SDRAM_TIMINGS	8
#define RB_ID_DEVICE_TIMINGS	9
#define RB_ID_SOFTWARE_ID	10
#define RB_ID_SERIAL_NUMBER	11
#define RB_ID_HARD_12		12
#define RB_ID_MEMORY_SIZE	13
#define RB_ID_MAC_ADDRESS_COUNT	14
#define RB_ID_HW_OPTIONS	21
#define RB_ID_WLAN_DATA		22

/*
 * ID values for Software settings
 */
#define RB_ID_UART_SPEED	1
#define RB_ID_BOOT_DELAY	2
#define RB_ID_BOOT_DEVICE	3
#define RB_ID_BOOT_KEY		4
#define RB_ID_CPU_MODE		5
#define RB_ID_FW_VERSION	6
#define RB_ID_SOFT_07		7
#define RB_ID_SOFT_08		8
#define RB_ID_BOOT_PROTOCOL	9
#define RB_ID_SOFT_10		10
#define RB_ID_SOFT_11		11

/*
 * UART_SPEED values
 */
#define RB_UART_SPEED_115200	0
#define RB_UART_SPEED_57600	1
#define RB_UART_SPEED_38400	2
#define RB_UART_SPEED_19200	3
#define RB_UART_SPEED_9600	4
#define RB_UART_SPEED_4800	5
#define RB_UART_SPEED_2400	6
#define RB_UART_SPEED_1200	7

/*
 * BOOT_DELAY values
 */
#define RB_BOOT_DELAY_0SEC	0
#define RB_BOOT_DELAY_1SEC	1
#define RB_BOOT_DELAY_2SEC	2

/*
 * BOOT_DEVICE values
 */
#define RB_BOOT_DEVICE_ETHER	0
#define RB_BOOT_DEVICE_NANDETH	1
#define RB_BOOT_DEVICE_ETHONCE	2
#define RB_BOOT_DEVICE_NANDONLY	3

/*
 * BOOT_KEY values
 */
#define RB_BOOT_KEY_ANY		0
#define RB_BOOT_KEY_DEL		1

/*
 * CPU_MODE values
 */
#define RB_CPU_MODE_POWERSAVE	0
#define RB_CPU_MODE_REGULAR	1

/*
 * BOOT_PROTOCOL values
 */
#define RB_BOOT_PROTOCOL_BOOTP	0
#define RB_BOOT_PROTOCOL_DHCP	1

#endif /* _ROUTERBOOT_H */
