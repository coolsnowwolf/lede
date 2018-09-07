/*
 *  ADM5120 ethernet switch definitions
 *
 *  This header file defines the hardware registers of the ADM5120 SoC
 *  built-in Ethernet switch.
 *
 *  Copyright (C) 2007-2008 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */

#ifndef _MACH_ADM5120_SWITCH_H
#define _MACH_ADM5120_SWITCH_H

#ifndef BIT
#  define BIT(at)	(1 << (at))
#endif
#define BITMASK(len)	(BIT(len)-1)

#define SW_READ_REG(r)		__raw_readl( \
	(void __iomem *)KSEG1ADDR(ADM5120_SWITCH_BASE) + r)
#define SW_WRITE_REG(r, v)	__raw_writel((v), \
	(void __iomem *)KSEG1ADDR(ADM5120_SWITCH_BASE) + r)

/* Switch register offsets */
#define SWITCH_REG_CODE		0x0000
#define SWITCH_REG_SOFT_RESET	0x0004	/* Soft Reset */
#define SWITCH_REG_BOOT_DONE	0x0008	/* Boot Done */
#define SWITCH_REG_SW_RESET	0x000C	/* Switch Reset */
#define SWITCH_REG_PHY_STATUS	0x0014	/* PHY Status */
#define SWITCH_REG_MEMCTRL	0x001C	/* Memory Control */
#define SWITCH_REG_CPUP_CONF	0x0024	/* CPU Port Configuration */
#define SWITCH_REG_PORT_CONF0	0x0028	/* Port Configuration 0 */
#define SWITCH_REG_PORT_CONF1	0x002C	/* Port Configuration 1 */
#define SWITCH_REG_PORT_CONF2	0x0030	/* Port Configuration 2 */
#define SWITCH_REG_VLAN_G1	0x0040	/* VLAN group 1 */
#define SWITCH_REG_VLAN_G2	0x0044	/* VLAN group 2 */
#define SWITCH_REG_SEND_TRIG	0x0048	/* Send Trigger */
#define SWITCH_REG_MAC_WT0	0x0058	/* MAC Write Address 0 */
#define SWITCH_REG_MAC_WT1	0x005C	/* MAC Write Address 1 */
#define SWITCH_REG_BW_CNTL0	0x0060	/* Bandwidth Control 0 */
#define SWITCH_REG_BW_CNTL1	0x0064	/* Bandwidth Control 1 */
#define SWITCH_REG_PHY_CNTL0	0x0068	/* PHY Control 0 */
#define SWITCH_REG_PHY_CNTL1	0x006C	/* PHY Control 1 */
#define SWITCH_REG_PORT_TH	0x0078	/* Port Threshold */
#define SWITCH_REG_PHY_CNTL2	0x007C	/* PHY Control 2 */
#define SWITCH_REG_PHY_CNTL3	0x0080	/* PHY Control 3 */
#define SWITCH_REG_PRI_CNTL	0x0084	/* Priority Control */
#define SWITCH_REG_PHY_CNTL4	0x00A0	/* PHY Control 4 */
#define SWITCH_REG_EMPTY_CNT	0x00A4	/* Empty Count */
#define SWITCH_REG_PORT_CNTLS	0x00A8	/* Port Control Select */
#define SWITCH_REG_PORT_CNTL	0x00AC	/* Port Control */
#define SWITCH_REG_INT_STATUS	0x00B0	/* Interrupt Status */
#define SWITCH_REG_INT_MASK	0x00B4	/* Interrupt Mask */
#define SWITCH_REG_GPIO_CONF0	0x00B8	/* GPIO Configuration 0 */
#define SWITCH_REG_GPIO_CONF2	0x00BC	/* GPIO Configuration 1 */
#define SWITCH_REG_WDOG0	0x00C0	/* Watchdog 0 */
#define SWITCH_REG_WDOG1	0x00C4	/* Watchdog 1 */

#define SWITCH_REG_SHDA		0x00D0	/* Send High Descriptors Address */
#define SWITCH_REG_SLDA		0x00D4	/* Send Low Descriptors Address */
#define SWITCH_REG_RHDA		0x00D8	/* Receive High Descriptor Address */
#define SWITCH_REG_RLDA		0x00DC	/* Receive Low Descriptor Address */
#define SWITCH_REG_SHWA		0x00E0	/* Send High Working Address */
#define SWITCH_REG_SLWA		0x00E4	/* Send Low Working Address */
#define SWITCH_REG_RHWA		0x00E8	/* Receive High Working Address */
#define SWITCH_REG_RLWA		0x00EC	/* Receive Low Working Address */

#define SWITCH_REG_TIMER_INT	0x00F0	/* Timer */
#define SWITCH_REG_TIMER	0x00F4	/* Timer Interrupt */

#define SWITCH_REG_PORT0_LED	0x0100
#define SWITCH_REG_PORT1_LED	0x0104
#define SWITCH_REG_PORT2_LED	0x0108
#define SWITCH_REG_PORT3_LED	0x010C
#define SWITCH_REG_PORT4_LED	0x0110

/* CODE register bits */
#define CODE_PC_MASK		BITMASK(16)	/* Product Code */
#define CODE_REV_SHIFT		16
#define CODE_REV_MASK		BITMASK(4)	/* Product Revision */
#define CODE_CLKS_SHIFT		20
#define CODE_CLKS_MASK		BITMASK(2)	/* Clock Speed */
#define CODE_CLKS_175		0		/* 175 MHz */
#define CODE_CLKS_200		1		/* 200 MHz */
#define CODE_CLKS_225		2		/* 225 MHz */
#define CODE_CLKS_250		3		/* 250 MHz */
#define CODE_NAB		BIT(24)		/* NAND boot */
#define CODE_PK_MASK		BITMASK(1)	/* Package type */
#define CODE_PK_SHIFT		29
#define CODE_PK_BGA		0		/* BGA package */
#define CODE_PK_PQFP		1		/* PQFP package */

/* MEMCTRL register bits */
#define MEMCTRL_SDRS_MASK	BITMASK(3)	/* SDRAM bank size */
#define MEMCTRL_SDRS_4M		0x01
#define MEMCTRL_SDRS_8M		0x02
#define MEMCTRL_SDRS_16M	0x03
#define MEMCTRL_SDRS_64M	0x04
#define MEMCTRL_SDRS_128M	0x05
#define MEMCTRL_SDR1_ENABLE	BIT(5)		/* enable SDRAM bank 1 */

#define MEMCTRL_SRS0_SHIFT	8		/* shift for SRAM0 size */
#define MEMCTRL_SRS1_SHIFT	16		/* shift for SRAM1 size */
#define MEMCTRL_SRS_MASK	BITMASK(3)	/* SRAM size mask */
#define MEMCTRL_SRS_DISABLED	0x00		/* Disabled */
#define MEMCTRL_SRS_512K	0x01		/* 512KB*/
#define MEMCTRL_SRS_1M		0x02		/* 1MB */
#define MEMCTRL_SRS_2M		0x03		/* 2MB */
#define MEMCTRL_SRS_4M		0x04		/* 4MB */

/* Port bits used in various registers */
#define SWITCH_PORT_PHY0	BIT(0)
#define SWITCH_PORT_PHY1	BIT(1)
#define SWITCH_PORT_PHY2	BIT(2)
#define SWITCH_PORT_PHY3	BIT(3)
#define SWITCH_PORT_PHY4	BIT(4)
#define SWITCH_PORT_MII		BIT(5)
#define SWITCH_PORT_CPU		BIT(6)

/* Port bit shorthands */
#define SWITCH_PORTS_PHY	0x1F	/* phy ports */
#define SWITCH_PORTS_NOCPU	0x3F	/* physical ports */
#define SWITCH_PORTS_ALL	0x7F	/* all ports */

/* CPUP_CONF register bits */
#define CPUP_CONF_DCPUP		BIT(0)	/* Disable CPU port */
#define CPUP_CONF_CRCP		BIT(1)	/* CRC padding from CPU */
#define CPUP_CONF_BTM		BIT(2)	/* Bridge Testing Mode */
#define CPUP_CONF_DUNP_SHIFT	9	/* Disable Unknown Packets for portX */
#define CPUP_CONF_DMCP_SHIFT	16	/* Disable Mcast Packets form portX */
#define CPUP_CONF_DBCP_SHIFT	24	/* Disable Bcast Packets form portX */

/* PORT_CONF0 register bits */
#define PORT_CONF0_DP_SHIFT	0	/* Disable Port */
#define PORT_CONF0_EMCP_SHIFT	8	/* Enable All MC Packets */
#define PORT_CONF0_BP_SHIFT	16	/* Enable Back Pressure */

/* PORT_CONF1 register bits */
#define PORT_CONF1_DISL_SHIFT	0	/* Disable Learning */
#define PORT_CONF1_BS_SHIFT	6	/* Blocking State */
#define PORT_CONF1_BM_SHIFT	12	/* Blocking Mode */

/* SEND_TRIG register bits */
#define SEND_TRIG_STL		BIT(0)	/* Send Trigger Low */
#define SEND_TRIG_STH		BIT(1)	/* Send Trigger High */

/* MAC_WT0 register bits */
#define MAC_WT0_MAWC		BIT(0)	/* MAC address write command */
#define MAC_WT0_MWD_SHIFT	1
#define MAC_WT0_MWD		BIT(1)	/* MAC write done */
#define MAC_WT0_WFB		BIT(2)	/* Write Filter Bit */
#define MAC_WT0_WVN_SHIFT	3	/* Write Vlan Number shift */
#define MAC_WT0_WVE		BIT(6)	/* Write VLAN enable */
#define MAC_WT0_WPMN_SHIFT	7
#define MAC_WT0_WAF_SHIFT	13	/* Write Age Field shift */
#define MAC_WT0_WAF_EMPTY	0
#define MAC_WT0_WAF_STATIC	7	/* age: static */
#define MAC_WT0_MAC0_SHIFT	16
#define MAC_WT0_MAC1_SHIFT	24

/* MAC_WT1 register bits */
#define MAC_WT1_MAC2_SHIFT	0
#define MAC_WT1_MAC3_SHIFT	8
#define MAC_WT1_MAC4_SHIFT	16
#define MAC_WT1_MAC5_SHIFT	24

/* BW_CNTL0/BW_CNTL1 register bits */
#define BW_CNTL_DISABLE		0x00
#define BW_CNTL_64K		0x01
#define BW_CNTL_128K		0x02
#define BW_CNTL_256K		0x03
#define BW_CNTL_512K		0x04
#define BW_CNTL_1M		0x05
#define BW_CNTL_4M		0x06
#define BW_CNTL_10M		0x07

#define P4TBC_SHIFT		0
#define P4RBC_SHIFT		4
#define P5TBC_SHIFT		8
#define P5RBC_SHIFT		12

#define BW_CNTL1_NAND_ENABLE	0x100

/* PHY_CNTL0 register bits */
#define PHY_CNTL0_PHYA_MASK	BITMASK(5)
#define PHY_CNTL0_PHYR_MASK	BITMASK(5)
#define PHY_CNTL0_PHYR_SHIFT	8
#define PHY_CNTL0_WC		BIT(13)		/* Write Command */
#define PHY_CNTL0_RC		BIT(14)		/* Read Command */
#define PHY_CNTL0_WTD_MASK	BIT(16)		/* Read Command */
#define PHY_CNTL0_WTD_SHIFT	16

/* PHY_CNTL1 register bits */
#define PHY_CNTL1_WOD		BIT(0)		/* Write Operation Done */
#define PHY_CNTL1_ROD		BIT(1)		/* Read Operation Done */
#define PHY_CNTL1_RD_MASK	BITMASK(16)
#define PHY_CNTL1_RD_SHIFT	16

/* PHY_CNTL2 register bits */
#define PHY_CNTL2_ANE_SHIFT	0	/* Auto Negotiation Enable */
#define PHY_CNTL2_SC_SHIFT	5	/* Speed Control */
#define PHY_CNTL2_DC_SHIFT	10	/* Duplex Control */
#define PHY_CNTL2_FNCV_SHIFT	15	/* Recommended FC Value */
#define PHY_CNTL2_PHYR_SHIFT	20	/* PHY reset */
#define PHY_CNTL2_AMDIX_SHIFT	25	/* Auto MDIX enable */
/* PHY_CNTL2_RMAE is bad in datasheet */
#define PHY_CNTL2_RMAE		BIT(31)	/* Recommended MCC Average enable */

/* PHY_CNTL3 register bits */
#define PHY_CNTL3_RNT		BIT(10) /* Recommend Normal Threshold */

/* PORT_TH register bits */
#define PORT_TH_PPT_MASK	BITMASK(8)	/* Per Port Threshold */
#define PORT_TH_CPUT_SHIFT	8		/* CPU Port Buffer Threshold */
#define PORT_TH_CPUT_MASK	BITMASK(8)
#define PORT_TH_CPUHT_SHIFT	16		/* CPU Hold Threshold */
#define PORT_TH_CPUHT_MASK	BITMASK(8)
#define PORT_TH_CPURT_SHIFT	24		/* CPU Release Threshold */
#define PORT_TH_CPURT_MASK	BITMASK(8)

/* EMPTY_CNT register bits */
#define EMPTY_CNT_EBGB_MASK	BITMASK(9) /* Empty Blocks in the Global Buffer */

/* GPIO_CONF0 register bits */
#define GPIO_CONF0_MASK		BITMASK(8)
#define GPIO_CONF0_IM_SHIFT	0
#define GPIO_CONF0_IV_SHIFT	8
#define GPIO_CONF0_OE_SHIFT	16
#define GPIO_CONF0_OV_SHIFT	24
#define GPIO_CONF0_IM_MASK	(0xFF << GPIO_CONF0_IM_SHIFT)
#define GPIO_CONF0_IV_MASK	(0xFF << GPIO_CONF0_IV_SHIFT)
#define GPIO_CONF0_OE_MASK	(0xFF << GPIO_CONF0_OE_SHIFT)
#define GPIO_CONF0_OV_MASK	(0xFF << GPIO_CONF0_OV_SHIFT)

/* GPIO_CONF2 register bits */
#define GPIO_CONF2_CSX0		BIT(4)	/* enable CSX0:INTX0 on GPIO 1:2 */
#define GPIO_CONF2_CSX1		BIT(5)	/* enable CSX1:INTX1 on GPIO 3:4 */
#define GPIO_CONF2_EW		BIT(6)	/* enable wait state pin for CSX0/1 */

/* INT_STATUS/INT_MASK register bits */
#define SWITCH_INT_SHD		BIT(0)	/* Send High Done */
#define SWITCH_INT_SLD		BIT(1)	/* Send Low Done */
#define SWITCH_INT_RHD		BIT(2)	/* Receive High Done */
#define SWITCH_INT_RLD		BIT(3)	/* Receive Low Done */
#define SWITCH_INT_HDF		BIT(4)	/* High Descriptor Full */
#define SWITCH_INT_LDF		BIT(5)	/* Low Descriptor Full */
#define SWITCH_INT_P0QF		BIT(6)	/* Port0 Queue Full */
#define SWITCH_INT_P1QF		BIT(7)	/* Port1 Queue Full */
#define SWITCH_INT_P2QF		BIT(8)	/* Port2 Queue Full */
#define SWITCH_INT_P3QF		BIT(9)	/* Port3 Queue Full */
#define SWITCH_INT_P4QF		BIT(10)	/* Port4 Queue Full */
#define SWITCH_INT_P5QF		BIT(11)	/* Port5 Queue Full */
#define SWITCH_INT_CPQF		BIT(13)	/* CPU Queue Full */
#define SWITCH_INT_GQF		BIT(14)	/* Global Queue Full */
#define SWITCH_INT_MD		BIT(15)	/* Must Drop */
#define SWITCH_INT_BCS		BIT(16)	/* BC Storm */
#define SWITCH_INT_PSC		BIT(18)	/* Port Status Change */
#define SWITCH_INT_ID		BIT(19)	/* Intruder Detected */
#define SWITCH_INT_W0TE		BIT(20)	/* Watchdog 0 Timer Expired */
#define SWITCH_INT_W1TE		BIT(21)	/* Watchdog 1 Timer Expired */
#define SWITCH_INT_RDE		BIT(22)	/* Receive Descriptor Error */
#define SWITCH_INT_SDE		BIT(23)	/* Send Descriptor Error */
#define SWITCH_INT_CPUH		BIT(24)	/* CPU Hold */

/* TIMER_INT register bits */
#define TIMER_INT_TOS		BIT(0)	/* time-out status */
#define TIMER_INT_TOM		BIT(16)	/* mask time-out interrupt */

/* TIMER register bits */
#define TIMER_PERIOD_MASK	BITMASK(16)	/* mask for timer period */
#define TIMER_PERIOD_DEFAULT	0xFFFF		/* default timer period */
#define TIMER_TE		BIT(16)	/* timer enable bit */

/* PORTx_LED register bits */
#define LED_MODE_MASK		BITMASK(4)
#define LED_MODE_INPUT		0
#define LED_MODE_FLASH		1
#define LED_MODE_OUT_HIGH	2
#define LED_MODE_OUT_LOW	3
#define LED_MODE_LINK		4
#define LED_MODE_SPEED		5
#define LED_MODE_DUPLEX		6
#define LED_MODE_ACT		7
#define LED_MODE_COLL		8
#define LED_MODE_LINK_ACT	9
#define LED_MODE_DUPLEX_COLL	10
#define LED_MODE_10M_ACT	11
#define LED_MODE_100M_ACT	12
#define LED0_MODE_SHIFT		0	/* LED0 mode shift */
#define LED1_MODE_SHIFT		4	/* LED1 mode shift */
#define LED2_MODE_SHIFT		8	/* LED2 mode shift */
#define LED0_IV_SHIFT		12	/* LED0 input value shift */
#define LED1_IV_SHIFT		13	/* LED1 input value shift */
#define LED2_IV_SHIFT		14	/* LED2 input value shift */

#endif /* _MACH_ADM5120_SWITCH_H */
