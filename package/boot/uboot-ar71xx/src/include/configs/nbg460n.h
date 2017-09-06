/*
 * (C) Copyright 2010
 * Michael Kurz <michi.kurz@googlemail.com>.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/* This file contains the configuration parameters for the zyxel nbg460n board. */

#ifndef _NBG460N_CONFIG_H
#define _NBG460N_CONFIG_H

#define CONFIG_MIPS32		1  /* MIPS32 CPU core */
#define CONFIG_AR71XX		1
#define CONFIG_AR91XX		1
#define CONFIG_SYS_HZ		1000
#define CONFIG_SYS_MIPS_TIMER_FREQ (400000000/2)

/* Cache Configuration */
#define CONFIG_SYS_DCACHE_SIZE		32768
#define CONFIG_SYS_ICACHE_SIZE		65536
#define CONFIG_SYS_CACHELINE_SIZE	32
/* Cache lock for stack */
#define CONFIG_SYS_INIT_SP_OFFSET	0x1000

#define CONFIG_SYS_MONITOR_BASE	(TEXT_BASE)

#define CONFIG_BAUDRATE		115200
#define CONFIG_SYS_BAUDRATE_TABLE  {115200}

#define CONFIG_MISC_INIT_R

/* SPI-Flash support */
#define CONFIG_SPI_FLASH
#define CONFIG_AR71XX_SPI
#define CONFIG_SPI_FLASH_MACRONIX
#define CONFIG_SF_DEFAULT_HZ	25000000

#define CONFIG_ENV_SPI_MAX_HZ	25000000
#define CONFIG_ENV_SPI_BUS		0
#define CONFIG_ENV_SPI_CS		0

#define	CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_ENV_ADDR			0xbfc20000
#define CONFIG_ENV_OFFSET		0x20000
#define CONFIG_ENV_SIZE			0x01000
#define CONFIG_ENV_SECT_SIZE	0x10000
#define CONFIG_SYS_MAX_FLASH_BANKS 1
#define CONFIG_SYS_MAX_FLASH_SECT 64
#define CONFIG_SYS_FLASH_BASE	0xbfc00000

/* Net support */
#define CONFIG_ETHADDR_ADDR     0xbfc0fff8
#define CONFIG_SYS_RX_ETH_BUFFER  	16
#define CONFIG_AG71XX
#define CONFIG_AG71XX_PORTS     { 1, 1 }
#define CONFIG_AG71XX_MII0_IIF  MII0_CTRL_IF_RGMII
#define CONFIG_AG71XX_MII1_IIF  MII1_CTRL_IF_RGMII
#define CONFIG_NET_MULTI
#define CONFIG_IPADDR			192.168.1.254
#define CONFIG_SERVERIP			192.168.1.42

/* Switch support */
#define CONFIG_MII
#define CONFIG_RTL8366_MII
#define RTL8366_PIN_SDA 16
#define RTL8366_PIN_SCK 18
#define MII_GPIOINCLUDE <asm/ar71xx_gpio.h>
#define MII_SETSDA(x)   ar71xx_setpin(RTL8366_PIN_SDA, x)
#define MII_GETSDA      ar71xx_getpin(RTL8366_PIN_SDA)
#define MII_SETSCK(x)   ar71xx_setpin(RTL8366_PIN_SCK, x)
#define MII_SDAINPUT    ar71xx_setpindir(RTL8366_PIN_SDA, 0)
#define MII_SDAOUTPUT   ar71xx_setpindir(RTL8366_PIN_SDA, 1)
#define MII_SCKINPUT    ar71xx_setpindir(RTL8366_PIN_SCK, 0)
#define MII_SCKOUTPUT   ar71xx_setpindir(RTL8366_PIN_SCK, 1)

#define CONFIG_BOOTDELAY	3
#define	CONFIG_BOOTARGS		"console=ttyS0,115200 rootfstype==squashfs,jffs2 noinitrd machtype=NBG460N"
#define CONFIG_BOOTCOMMAND	"bootm 0xbfc70000"
#define CONFIG_LZMA


/* Commands */
#define CONFIG_SYS_NO_FLASH
#include <config_cmd_default.h>
#undef CONFIG_CMD_BDI
#undef CONFIG_CMD_FPGA
#undef CONFIG_CMD_IMI
#undef CONFIG_CMD_IMLS
#undef CONFIG_CMD_LOADS
#define CONFIG_CMD_SF
#define CONFIG_CMD_MII
#define CONFIG_CMD_PING
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_SPI

/* Miscellaneous configurable options */
#define CONFIG_SYS_PROMPT		"U-Boot> "
#define CONFIG_SYS_CBSIZE		256
#define CONFIG_SYS_MAXARGS		16
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_LONGHELP		1
#define CONFIG_CMDLINE_EDITING	1
#define CONFIG_AUTO_COMPLETE
#define CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_PROMPT_HUSH_PS2	"> "

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		ROUND(3 * 0x10000 + 128*1024, 0x1000)
#define CONFIG_SYS_GBL_DATA_SIZE	128	/* 128 bytes for initial data */

#define CONFIG_SYS_BOOTPARAMS_LEN	128*1024

#define CONFIG_SYS_SDRAM_BASE		0x80000000     /* Cached addr */
#define	CONFIG_SYS_LOAD_ADDR		0x80060000     /* default load address	*/

#define CONFIG_SYS_MEMTEST_START	0x80000800
#define CONFIG_SYS_MEMTEST_END		0x81E00000

#endif	/* _NBG460N_CONFIG_H */
