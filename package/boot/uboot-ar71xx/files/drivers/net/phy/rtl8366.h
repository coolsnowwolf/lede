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

#ifndef RTL8366_MII_H
#define RTL8366_MII_H

#define	MII_CONTROL_REG		    0
#define	MII_STATUS_REG	    	1
#define	MII_PHY_ID0		        2
#define	MII_PHY_ID1		        3
#define	MII_LOCAL_CAP	    	4
#define	MII_REMOTE_CAP		    5
#define	MII_EXT_AUTONEG		    6
#define	MII_LOCAL_NEXT_PAGE	    7
#define	MII_REMOTE_NEXT_PAGE	8
#define	MII_GIGA_CONTROL	    9
#define	MII_GIGA_STATUS		    10
#define	MII_EXT_STATUS_REG	    15

/* Control register */
#define	MII_CONTROL_1000MBPS	6
#define	MII_CONTROL_COLL_TEST	7
#define	MII_CONTROL_FULLDUPLEX	8
#define	MII_CONTROL_RENEG	    9
#define	MII_CONTROL_ISOLATE	    10
#define	MII_CONTROL_POWERDOWN	11
#define	MII_CONTROL_AUTONEG	    12
#define	MII_CONTROL_100MBPS	    13
#define	MII_CONTROL_LOOPBACK	14
#define	MII_CONTROL_RESET	    15

/* Status/Extended status register */
/* Basic status */
#define	MII_STATUS_CAPABILITY	0
#define	MII_STATUS_JABBER	    1
#define	MII_STATUS_LINK_UP	    2
#define	MII_STATUS_AUTONEG_ABLE	3
#define	MII_STATUS_REMOTE_FAULT	4
#define	MII_STATUS_AUTONEG_DONE	5
#define	MII_STATUS_NO_PREAMBLE	6
#define	MII_STATUS_RESERVED	    7
#define	MII_STATUS_EXTENDED	    8
#define	MII_STATUS_100_T2_HALF	9
#define	MII_STATUS_100_T2_FULL	10
#define	MII_STATUS_10_TX_HALF	11
#define	MII_STATUS_10_TX_FULL	12
#define	MII_STATUS_100_TX_HALF	13
#define	MII_STATUS_100_TX_FULL	14
#define	MII_STATUS_100_T4	    15

#define	MII_GIGA_CONTROL_HALF	8
#define	MII_GIGA_CONTROL_FULL	9
#define	MII_GIGA_STATUS_HALF	10
#define	MII_GIGA_STATUS_FULL	11

/* Extended status */
#define	MII_STATUS_1000_T_HALF	12
#define	MII_STATUS_1000_T_FULL	13
#define	MII_STATUS_1000_X_HALF	14
#define	MII_STATUS_1000_X_FULL	15

/* Local/Remmote capability register */
#define	MII_CAP_10BASE_TX	    5
#define	MII_CAP_10BASE_TX_FULL	6
#define	MII_CAP_100BASE_TX	    7
#define	MII_CAP_100BASE_TX_FULL	8
#define	MII_CAP_100BASE_T4	    9
#define	MII_CAP_SYMM_PAUSE	    10
#define	MII_CAP_ASYMM_PAUSE	    11
#define	MII_CAP_RESERVED	    12
#define	MII_CAP_REMOTE_FAULT	13
#define	MII_CAP_ACKNOWLEDGE	    14
#define	MII_CAP_NEXT_PAGE	    15
#define	MII_CAP_IEEE_802_3	    0x0001

#define	MII_LINK_MODE_MASK	    0x1f

#define REALTEK_RTL8366_CHIP_ID0    0x001C
#define REALTEK_RTL8366_CHIP_ID1    0xC940
#define REALTEK_RTL8366_CHIP_ID1_MP 0xC960

#define REALTEK_MIN_PORT_ID     0
#define REALTEK_MAX_PORT_ID     5
#define REALTEK_MIN_PHY_ID      REALTEK_MIN_PORT_ID
#define REALTEK_MAX_PHY_ID      4
#define REALTEK_CPU_PORT_ID     REALTEK_MAX_PORT_ID
#define REALTEK_PHY_PORT_MASK   ((1<<(REALTEK_MAX_PHY_ID+1)) - (1<<REALTEK_MIN_PHY_ID))
#define REALTEK_CPU_PORT_MASK   (1<<REALTEK_CPU_PORT_ID)
#define REALTEK_ALL_PORT_MASK   (REALTEK_PHY_PORT_MASK | REALTEK_CPU_PORT_MASK)

/* port ability */
#define RTL8366S_PORT_ABILITY_BASE			0x0011

/* port vlan control register */
#define RTL8366S_PORT_VLAN_CTRL_BASE			0x0058

/* port linking status */
#define RTL8366S_PORT_LINK_STATUS_BASE			0x0060
#define RTL8366S_PORT_STATUS_SPEED_BIT			0
#define RTL8366S_PORT_STATUS_SPEED_MSK			0x0003
#define RTL8366S_PORT_STATUS_DUPLEX_BIT			2
#define RTL8366S_PORT_STATUS_DUPLEX_MSK			0x0004
#define RTL8366S_PORT_STATUS_LINK_BIT			4
#define RTL8366S_PORT_STATUS_LINK_MSK			0x0010
#define RTL8366S_PORT_STATUS_TXPAUSE_BIT		5
#define RTL8366S_PORT_STATUS_TXPAUSE_MSK		0x0020
#define RTL8366S_PORT_STATUS_RXPAUSE_BIT		6
#define RTL8366S_PORT_STATUS_RXPAUSE_MSK		0x0040
#define RTL8366S_PORT_STATUS_AN_BIT			7
#define RTL8366S_PORT_STATUS_AN_MSK			0x0080

/* internal control */
#define RTL8366S_RESET_CONTROL_REG			0x0100
#define RTL8366S_RESET_QUEUE_BIT			2

#define RTL8366S_CHIP_ID_REG				0x0105

/* MAC control */
#define RTL8366S_MAC_FORCE_CTRL0_REG			0x0F04
#define RTL8366S_MAC_FORCE_CTRL1_REG			0x0F05


/* PHY registers control */
#define RTL8366S_PHY_ACCESS_CTRL_REG			0x8028
#define RTL8366S_PHY_ACCESS_DATA_REG			0x8029

#define RTL8366S_PHY_CTRL_READ				1
#define RTL8366S_PHY_CTRL_WRITE				0

#define RTL8366S_PHY_REG_MASK				0x1F
#define RTL8366S_PHY_PAGE_OFFSET			5
#define RTL8366S_PHY_PAGE_MASK				(0x7<<5)
#define RTL8366S_PHY_NO_OFFSET				9
#define RTL8366S_PHY_NO_MASK				(0x1F<<9)

#define RTL8366S_PHY_NO_MAX				4
#define RTL8366S_PHY_PAGE_MAX				7
#define RTL8366S_PHY_ADDR_MAX				31

/* cpu port control reg */
#define RTL8366S_CPU_CTRL_REG				0x004F
#define RTL8366S_CPU_DRP_BIT				14
#define RTL8366S_CPU_DRP_MSK				0x4000
#define RTL8366S_CPU_INSTAG_BIT				15
#define RTL8366S_CPU_INSTAG_MSK				0x8000

/* LED registers*/
#define RTL8366S_LED_BLINK_REG				0x420
#define RTL8366S_LED_BLINKRATE_BIT			0
#define RTL8366S_LED_BLINKRATE_MSK			0x0007
#define RTL8366S_LED_INDICATED_CONF_REG			0x421
#define RTL8366S_LED_0_1_FORCE_REG			0x422
#define RTL8366S_LED_2_3_FORCE_REG			0x423
#define RTL8366S_LEDCONF_LEDFORCE			0x1F
#define RTL8366S_LED_GROUP_MAX				4

#define RTL8366S_GREEN_FEATURE_REG			0x000A
#define RTL8366S_GREEN_FEATURE_TX_BIT			3
#define RTL8366S_GREEN_FEATURE_TX_MSK			0x0008
#define RTL8366S_GREEN_FEATURE_RX_BIT			4
#define RTL8366S_GREEN_FEATURE_RX_MSK			0x0010

#define	RTL8366S_MODEL_ID_REG	0x5C
#define	RTL8366S_REV_ID_REG	0x5D
#define	RTL8366S_MODEL_8366SR	0x6027
#define	RTL8366S_MODEL_8366RB	0x5937

#endif
