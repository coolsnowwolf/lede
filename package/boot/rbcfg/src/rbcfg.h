/*
 *  Mikrotik's RouterBOOT configuration defines
 *
 *  Copyright (C) 2010 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */

#ifndef _RBCFG_H
#define _RBCFG_H

/*
 * Magic numbers
 */
#define RB_MAGIC_SOFT	0x74666f53 /* 'Soft' */

/*
 * ID values for Software settings
 */
#define RB_ID_TERMINATOR	0
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
#define RB_ID_CPU_FREQ		12
#define RB_ID_BOOTER		13

#define RB_UART_SPEED_115200	0
#define RB_UART_SPEED_57600	1
#define RB_UART_SPEED_38400	2
#define RB_UART_SPEED_19200	3
#define RB_UART_SPEED_9600	4
#define RB_UART_SPEED_4800	5
#define RB_UART_SPEED_2400	6
#define RB_UART_SPEED_1200	7
#define RB_UART_SPEED_OFF	8

#define RB_BOOT_DELAY_1SEC	1
#define RB_BOOT_DELAY_2SEC	2
#define RB_BOOT_DELAY_3SEC	3
#define RB_BOOT_DELAY_4SEC	4
#define RB_BOOT_DELAY_5SEC	5
#define RB_BOOT_DELAY_6SEC	6
#define RB_BOOT_DELAY_7SEC	7
#define RB_BOOT_DELAY_8SEC	8
#define RB_BOOT_DELAY_9SEC	9

#define RB_BOOT_DEVICE_ETHER	0
#define RB_BOOT_DEVICE_NANDETH	1
#define RB_BOOT_DEVICE_CFCARD	2
#define RB_BOOT_DEVICE_ETHONCE	3
#define RB_BOOT_DEVICE_NANDONLY	5
#define RB_BOOT_DEVICE_FLASHCFG	7
#define RB_BOOT_DEVICE_FLSHONCE	8

#define RB_BOOT_KEY_ANY		0
#define RB_BOOT_KEY_DEL		1

#define RB_CPU_MODE_POWERSAVE	0
#define RB_CPU_MODE_REGULAR	1

#define RB_BOOT_PROTOCOL_BOOTP	0
#define RB_BOOT_PROTOCOL_DHCP	1

#define RB_CPU_FREQ_L2		(0 << 3)
#define RB_CPU_FREQ_L1		(1 << 3)
#define RB_CPU_FREQ_N0		(2 << 3)
#define RB_CPU_FREQ_H1		(3 << 3)
#define RB_CPU_FREQ_H2		(4 << 3)
#define RB_CPU_FREQ_H3		(5 << 3)

#define RB_BOOTER_REGULAR	0
#define RB_BOOTER_BACKUP	1

#endif /* _RBCFG_H */
