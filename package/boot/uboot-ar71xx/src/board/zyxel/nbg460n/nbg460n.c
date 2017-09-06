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

#include <common.h>
#include <netdev.h>
#include <asm/mipsregs.h>
#include <asm/addrspace.h>
#include <asm/reboot.h>
#include <asm/ar71xx.h>
#include <asm/ar71xx_gpio.h>

#define NBG460N_WAN_LED			19

phys_size_t initdram(int board_type)
{
    return (32*1024*1024);
}

int checkboard(void)
{
	// Set pin 19 to 1, to stop WAN LED blinking
    ar71xx_setpindir(NBG460N_WAN_LED, 1);
    ar71xx_setpin(NBG460N_WAN_LED, 1);

    printf("U-boot on Zyxel NBG460N\n");
    return 0;
}

void _machine_restart(void)
{
	for (;;) {
		writel((RESET_MODULE_FULL_CHIP | RESET_MODULE_DDR),
			KSEG1ADDR(AR71XX_RESET_BASE + AR91XX_RESET_REG_RESET_MODULE));
        readl(KSEG1ADDR(AR71XX_RESET_BASE + AR91XX_RESET_REG_RESET_MODULE));
	}
}

int board_eth_init(bd_t *bis)
{
    char *phynames[] = {RTL8366_DEVNAME, RTL8366_DEVNAME};
    u16 phyids[] = {RTL8366_LANPHY_ID, RTL8366_WANPHY_ID};
    u16 phyfixed[] = {1, 0};

    if (ag71xx_register(bis, phynames, phyids, phyfixed) <= 0)
        return -1;

	if (rtl8366s_initialize())
        return -1;

    if (rtl8366_mii_register(bis))
        return -1;
		
    return 0;
}

int misc_init_r(void) {
    uint8_t macaddr[6];
    uint8_t enetaddr[6];

	debug("Testing mac addresses\n");
	
    memcpy(macaddr, (uint8_t *) CONFIG_ETHADDR_ADDR, 6);

    if (!eth_getenv_enetaddr("ethaddr", enetaddr)) {
        debug("Setting eth0 mac addr to %pM\n", macaddr);
        eth_setenv_enetaddr("ethaddr", macaddr);
    }

    if (!eth_getenv_enetaddr("eth1addr", enetaddr)) {
		macaddr[5] += 1;
        debug("Setting eth1 mac addr to %pM\n", macaddr);
        eth_setenv_enetaddr("eth1addr", macaddr);
    }

    return 0;
}
