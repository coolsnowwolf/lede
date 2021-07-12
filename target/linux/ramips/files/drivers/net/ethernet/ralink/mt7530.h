/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * Copyright (C) 2013 John Crispin <blogic@openwrt.org>
 * Copyright (C) 2016 Vitaly Chekryzhev <13hakta@gmail.com>
 */

#ifndef _MT7530_H__
#define _MT7530_H__

#define MT7620_MIB_COUNTER_BASE_PORT	0x4000
#define MT7620_MIB_COUNTER_PORT_OFFSET	0x100
#define MT7620_MIB_COUNTER_BASE	0x1010

/* PPE Accounting Group #0 Byte Counter */
#define MT7620_MIB_STATS_PPE_AC_BCNT0	0x000

/* PPE Accounting Group #0 Packet Counter */
#define MT7620_MIB_STATS_PPE_AC_PCNT0	0x004

/* PPE Accounting Group #63 Byte Counter */
#define MT7620_MIB_STATS_PPE_AC_BCNT63	0x1F8

/* PPE Accounting Group #63 Packet Counter */
#define MT7620_MIB_STATS_PPE_AC_PCNT63	0x1FC

/* PPE Meter Group #0 */
#define MT7620_MIB_STATS_PPE_MTR_CNT0	0x200

/* PPE Meter Group #63 */
#define MT7620_MIB_STATS_PPE_MTR_CNT63	0x2FC

/* Transmit good byte count for CPU GDM */
#define MT7620_MIB_STATS_GDM1_TX_GBCNT	0x300

/* Transmit good packet count for CPU GDM (exclude flow control frames) */
#define MT7620_MIB_STATS_GDM1_TX_GPCNT	0x304

/* Transmit abort count for CPU GDM */
#define MT7620_MIB_STATS_GDM1_TX_SKIPCNT	0x308

/* Transmit collision count for CPU GDM */
#define MT7620_MIB_STATS_GDM1_TX_COLCNT	0x30C

/* Received good byte count for CPU GDM */
#define MT7620_MIB_STATS_GDM1_RX_GBCNT1	0x320

/* Received good packet count for CPU GDM (exclude flow control frame) */
#define MT7620_MIB_STATS_GDM1_RX_GPCNT1	0x324

/* Received overflow error packet count for CPU GDM */
#define MT7620_MIB_STATS_GDM1_RX_OERCNT	0x328

/* Received FCS error packet count for CPU GDM */
#define MT7620_MIB_STATS_GDM1_RX_FERCNT	0x32C

/* Received too short error packet count for CPU GDM */
#define MT7620_MIB_STATS_GDM1_RX_SERCNT	0x330

/* Received too long error packet count for CPU GDM */
#define MT7620_MIB_STATS_GDM1_RX_LERCNT	0x334

/* Received IP/TCP/UDP checksum error packet count for CPU GDM */
#define MT7620_MIB_STATS_GDM1_RX_CERCNT	0x338

/* Received flow control pkt count for CPU GDM */
#define MT7620_MIB_STATS_GDM1_RX_FCCNT	0x33C

/* Transmit good byte count for PPE GDM */
#define MT7620_MIB_STATS_GDM2_TX_GBCNT	0x340

/* Transmit good packet count for PPE GDM (exclude flow control frames) */
#define MT7620_MIB_STATS_GDM2_TX_GPCNT	0x344

/* Transmit abort count for PPE GDM */
#define MT7620_MIB_STATS_GDM2_TX_SKIPCNT	0x348

/* Transmit collision count for PPE GDM */
#define MT7620_MIB_STATS_GDM2_TX_COLCNT	0x34C

/* Received good byte count for PPE GDM */
#define MT7620_MIB_STATS_GDM2_RX_GBCNT	0x360

/* Received good packet count for PPE GDM (exclude flow control frame) */
#define MT7620_MIB_STATS_GDM2_RX_GPCNT	0x364

/* Received overflow error packet count for PPE GDM */
#define MT7620_MIB_STATS_GDM2_RX_OERCNT	0x368

/* Received FCS error packet count for PPE GDM */
#define MT7620_MIB_STATS_GDM2_RX_FERCNT	0x36C

/* Received too short error packet count for PPE GDM */
#define MT7620_MIB_STATS_GDM2_RX_SERCNT	0x370

/* Received too long error packet count for PPE GDM */
#define MT7620_MIB_STATS_GDM2_RX_LERCNT	0x374

/* Received IP/TCP/UDP checksum error packet count for PPE GDM */
#define MT7620_MIB_STATS_GDM2_RX_CERCNT	0x378

/* Received flow control pkt count for PPE GDM */
#define MT7620_MIB_STATS_GDM2_RX_FCCNT	0x37C

/* Tx Packet Counter of Port n */
#define MT7620_MIB_STATS_PORT_TGPCN	0x10

/* Tx Bad Octet Counter of Port n */
#define MT7620_MIB_STATS_PORT_TBOCN	0x14

/* Tx Good Octet Counter of Port n */
#define MT7620_MIB_STATS_PORT_TGOCN	0x18

/* Tx Event Packet Counter of Port n */
#define MT7620_MIB_STATS_PORT_TEPCN	0x1C

/* Rx Packet Counter of Port n */
#define MT7620_MIB_STATS_PORT_RGPCN	0x20

/* Rx Bad Octet Counter of Port n */
#define MT7620_MIB_STATS_PORT_RBOCN	0x24

/* Rx Good Octet Counter of Port n */
#define MT7620_MIB_STATS_PORT_RGOCN	0x28

/* Rx Event Packet Counter of Port n */
#define MT7620_MIB_STATS_PORT_REPC1N	0x2C

/* Rx Event Packet Counter of Port n */
#define MT7620_MIB_STATS_PORT_REPC2N	0x30

int mt7530_probe(struct device *dev, void __iomem *base, struct mii_bus *bus, int vlan);

#endif
