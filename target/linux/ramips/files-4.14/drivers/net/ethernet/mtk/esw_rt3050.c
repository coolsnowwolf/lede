/*   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; version 2 of the License
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   Copyright (C) 2009-2015 John Crispin <blogic@openwrt.org>
 *   Copyright (C) 2009-2015 Felix Fietkau <nbd@nbd.name>
 *   Copyright (C) 2013-2015 Michael Lee <igvtee@gmail.com>
 *   Copyright (C) 2016 Vittorio Gambaletta <openwrt@vittgam.net>
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <asm/mach-ralink/ralink_regs.h>
#include <linux/of_irq.h>

#include <linux/switch.h>

#include "mtk_eth_soc.h"

/* HW limitations for this switch:
 * - No large frame support (PKT_MAX_LEN at most 1536)
 * - Can't have untagged vlan and tagged vlan on one port at the same time,
 *   though this might be possible using the undocumented PPE.
 */

#define RT305X_ESW_REG_ISR		0x00
#define RT305X_ESW_REG_IMR		0x04
#define RT305X_ESW_REG_FCT0		0x08
#define RT305X_ESW_REG_PFC1		0x14
#define RT305X_ESW_REG_ATS		0x24
#define RT305X_ESW_REG_ATS0		0x28
#define RT305X_ESW_REG_ATS1		0x2c
#define RT305X_ESW_REG_ATS2		0x30
#define RT305X_ESW_REG_PVIDC(_n)	(0x40 + 4 * (_n))
#define RT305X_ESW_REG_VLANI(_n)	(0x50 + 4 * (_n))
#define RT305X_ESW_REG_VMSC(_n)		(0x70 + 4 * (_n))
#define RT305X_ESW_REG_POA		0x80
#define RT305X_ESW_REG_FPA		0x84
#define RT305X_ESW_REG_SOCPC		0x8c
#define RT305X_ESW_REG_POC0		0x90
#define RT305X_ESW_REG_POC1		0x94
#define RT305X_ESW_REG_POC2		0x98
#define RT305X_ESW_REG_SGC		0x9c
#define RT305X_ESW_REG_STRT		0xa0
#define RT305X_ESW_REG_PCR0		0xc0
#define RT305X_ESW_REG_PCR1		0xc4
#define RT305X_ESW_REG_FPA2		0xc8
#define RT305X_ESW_REG_FCT2		0xcc
#define RT305X_ESW_REG_SGC2		0xe4
#define RT305X_ESW_REG_P0LED		0xa4
#define RT305X_ESW_REG_P1LED		0xa8
#define RT305X_ESW_REG_P2LED		0xac
#define RT305X_ESW_REG_P3LED		0xb0
#define RT305X_ESW_REG_P4LED		0xb4
#define RT305X_ESW_REG_PXPC(_x)		(0xe8 + (4 * _x))
#define RT305X_ESW_REG_P1PC		0xec
#define RT305X_ESW_REG_P2PC		0xf0
#define RT305X_ESW_REG_P3PC		0xf4
#define RT305X_ESW_REG_P4PC		0xf8
#define RT305X_ESW_REG_P5PC		0xfc

#define RT305X_ESW_LED_LINK		0
#define RT305X_ESW_LED_100M		1
#define RT305X_ESW_LED_DUPLEX		2
#define RT305X_ESW_LED_ACTIVITY		3
#define RT305X_ESW_LED_COLLISION	4
#define RT305X_ESW_LED_LINKACT		5
#define RT305X_ESW_LED_DUPLCOLL		6
#define RT305X_ESW_LED_10MACT		7
#define RT305X_ESW_LED_100MACT		8
/* Additional led states not in datasheet: */
#define RT305X_ESW_LED_BLINK		10
#define RT305X_ESW_LED_ON		12

#define RT305X_ESW_LINK_S		25
#define RT305X_ESW_DUPLEX_S		9
#define RT305X_ESW_SPD_S		0

#define RT305X_ESW_PCR0_WT_NWAY_DATA_S	16
#define RT305X_ESW_PCR0_WT_PHY_CMD	BIT(13)
#define RT305X_ESW_PCR0_CPU_PHY_REG_S	8

#define RT305X_ESW_PCR1_WT_DONE		BIT(0)

#define RT305X_ESW_ATS_TIMEOUT		(5 * HZ)
#define RT305X_ESW_PHY_TIMEOUT		(5 * HZ)

#define RT305X_ESW_PVIDC_PVID_M		0xfff
#define RT305X_ESW_PVIDC_PVID_S		12

#define RT305X_ESW_VLANI_VID_M		0xfff
#define RT305X_ESW_VLANI_VID_S		12

#define RT305X_ESW_VMSC_MSC_M		0xff
#define RT305X_ESW_VMSC_MSC_S		8

#define RT305X_ESW_SOCPC_DISUN2CPU_S	0
#define RT305X_ESW_SOCPC_DISMC2CPU_S	8
#define RT305X_ESW_SOCPC_DISBC2CPU_S	16
#define RT305X_ESW_SOCPC_CRC_PADDING	BIT(25)

#define RT305X_ESW_POC0_EN_BP_S		0
#define RT305X_ESW_POC0_EN_FC_S		8
#define RT305X_ESW_POC0_DIS_RMC2CPU_S	16
#define RT305X_ESW_POC0_DIS_PORT_M	0x7f
#define RT305X_ESW_POC0_DIS_PORT_S	23

#define RT305X_ESW_POC2_UNTAG_EN_M	0xff
#define RT305X_ESW_POC2_UNTAG_EN_S	0
#define RT305X_ESW_POC2_ENAGING_S	8
#define RT305X_ESW_POC2_DIS_UC_PAUSE_S	16

#define RT305X_ESW_SGC2_DOUBLE_TAG_M	0x7f
#define RT305X_ESW_SGC2_DOUBLE_TAG_S	0
#define RT305X_ESW_SGC2_LAN_PMAP_M	0x3f
#define RT305X_ESW_SGC2_LAN_PMAP_S	24

#define RT305X_ESW_PFC1_EN_VLAN_M	0xff
#define RT305X_ESW_PFC1_EN_VLAN_S	16
#define RT305X_ESW_PFC1_EN_TOS_S	24

#define RT305X_ESW_VLAN_NONE		0xfff

#define RT305X_ESW_GSC_BC_STROM_MASK	0x3
#define RT305X_ESW_GSC_BC_STROM_SHIFT	4

#define RT305X_ESW_GSC_LED_FREQ_MASK	0x3
#define RT305X_ESW_GSC_LED_FREQ_SHIFT	23

#define RT305X_ESW_POA_LINK_MASK	0x1f
#define RT305X_ESW_POA_LINK_SHIFT	25

#define RT305X_ESW_PORT_ST_CHG		BIT(26)
#define RT305X_ESW_PORT0		0
#define RT305X_ESW_PORT1		1
#define RT305X_ESW_PORT2		2
#define RT305X_ESW_PORT3		3
#define RT305X_ESW_PORT4		4
#define RT305X_ESW_PORT5		5
#define RT305X_ESW_PORT6		6

#define RT305X_ESW_PORTS_NONE		0

#define RT305X_ESW_PMAP_LLLLLL		0x3f
#define RT305X_ESW_PMAP_LLLLWL		0x2f
#define RT305X_ESW_PMAP_WLLLLL		0x3e

#define RT305X_ESW_PORTS_INTERNAL					\
		(BIT(RT305X_ESW_PORT0) | BIT(RT305X_ESW_PORT1) |	\
		 BIT(RT305X_ESW_PORT2) | BIT(RT305X_ESW_PORT3) |	\
		 BIT(RT305X_ESW_PORT4))

#define RT305X_ESW_PORTS_NOCPU						\
		(RT305X_ESW_PORTS_INTERNAL | BIT(RT305X_ESW_PORT5))

#define RT305X_ESW_PORTS_CPU	BIT(RT305X_ESW_PORT6)

#define RT305X_ESW_PORTS_ALL						\
		(RT305X_ESW_PORTS_NOCPU | RT305X_ESW_PORTS_CPU)

#define RT305X_ESW_NUM_VLANS		16
#define RT305X_ESW_NUM_VIDS		4096
#define RT305X_ESW_NUM_PORTS		7
#define RT305X_ESW_NUM_LANWAN		6
#define RT305X_ESW_NUM_LEDS		5

#define RT5350_ESW_REG_PXTPC(_x)	(0x150 + (4 * _x))
#define RT5350_EWS_REG_LED_POLARITY	0x168
#define RT5350_RESET_EPHY		BIT(24)

enum {
	/* Global attributes. */
	RT305X_ESW_ATTR_ENABLE_VLAN,
	RT305X_ESW_ATTR_ALT_VLAN_DISABLE,
	RT305X_ESW_ATTR_BC_STATUS,
	RT305X_ESW_ATTR_LED_FREQ,
	/* Port attributes. */
	RT305X_ESW_ATTR_PORT_DISABLE,
	RT305X_ESW_ATTR_PORT_DOUBLETAG,
	RT305X_ESW_ATTR_PORT_UNTAG,
	RT305X_ESW_ATTR_PORT_LED,
	RT305X_ESW_ATTR_PORT_LAN,
	RT305X_ESW_ATTR_PORT_RECV_BAD,
	RT305X_ESW_ATTR_PORT_RECV_GOOD,
	RT5350_ESW_ATTR_PORT_TR_BAD,
	RT5350_ESW_ATTR_PORT_TR_GOOD,
};

struct esw_port {
	bool	disable;
	bool	doubletag;
	bool	untag;
	u8	led;
	u16	pvid;
};

struct esw_vlan {
	u8	ports;
	u16	vid;
};

enum {
	RT305X_ESW_VLAN_CONFIG_NONE = 0,
	RT305X_ESW_VLAN_CONFIG_LLLLW,
	RT305X_ESW_VLAN_CONFIG_WLLLL,
};

struct rt305x_esw {
	struct device		*dev;
	void __iomem		*base;
	int			irq;

	/* Protects against concurrent register r/w operations. */
	spinlock_t		reg_rw_lock;

	unsigned char		port_map;
	unsigned char		port_disable;
	unsigned int		reg_initval_fct2;
	unsigned int		reg_initval_fpa2;
	unsigned int		reg_led_polarity;

	struct switch_dev	swdev;
	bool			global_vlan_enable;
	bool			alt_vlan_disable;
	int			bc_storm_protect;
	int			led_frequency;
	struct esw_vlan vlans[RT305X_ESW_NUM_VLANS];
	struct esw_port ports[RT305X_ESW_NUM_PORTS];

};

static inline void esw_w32(struct rt305x_esw *esw, u32 val, unsigned reg)
{
	__raw_writel(val, esw->base + reg);
}

static inline u32 esw_r32(struct rt305x_esw *esw, unsigned reg)
{
	return __raw_readl(esw->base + reg);
}

static inline void esw_rmw_raw(struct rt305x_esw *esw, unsigned reg,
			       unsigned long mask, unsigned long val)
{
	unsigned long t;

	t = __raw_readl(esw->base + reg) & ~mask;
	__raw_writel(t | val, esw->base + reg);
}

static void esw_rmw(struct rt305x_esw *esw, unsigned reg,
		    unsigned long mask, unsigned long val)
{
	unsigned long flags;

	spin_lock_irqsave(&esw->reg_rw_lock, flags);
	esw_rmw_raw(esw, reg, mask, val);
	spin_unlock_irqrestore(&esw->reg_rw_lock, flags);
}

static u32 rt305x_mii_write(struct rt305x_esw *esw, u32 phy_addr,
			    u32 phy_register, u32 write_data)
{
	unsigned long t_start = jiffies;
	int ret = 0;

	while (1) {
		if (!(esw_r32(esw, RT305X_ESW_REG_PCR1) &
		      RT305X_ESW_PCR1_WT_DONE))
			break;
		if (time_after(jiffies, t_start + RT305X_ESW_PHY_TIMEOUT)) {
			ret = 1;
			goto out;
		}
	}

	write_data &= 0xffff;
	esw_w32(esw, (write_data << RT305X_ESW_PCR0_WT_NWAY_DATA_S) |
		      (phy_register << RT305X_ESW_PCR0_CPU_PHY_REG_S) |
		      (phy_addr) | RT305X_ESW_PCR0_WT_PHY_CMD,
		RT305X_ESW_REG_PCR0);

	t_start = jiffies;
	while (1) {
		if (esw_r32(esw, RT305X_ESW_REG_PCR1) &
			    RT305X_ESW_PCR1_WT_DONE)
			break;

		if (time_after(jiffies, t_start + RT305X_ESW_PHY_TIMEOUT)) {
			ret = 1;
			break;
		}
	}
out:
	if (ret)
		dev_err(esw->dev, "ramips_eth: MDIO timeout\n");
	return ret;
}

static unsigned esw_get_vlan_id(struct rt305x_esw *esw, unsigned vlan)
{
	unsigned s;
	unsigned val;

	s = RT305X_ESW_VLANI_VID_S * (vlan % 2);
	val = esw_r32(esw, RT305X_ESW_REG_VLANI(vlan / 2));
	val = (val >> s) & RT305X_ESW_VLANI_VID_M;

	return val;
}

static void esw_set_vlan_id(struct rt305x_esw *esw, unsigned vlan, unsigned vid)
{
	unsigned s;

	s = RT305X_ESW_VLANI_VID_S * (vlan % 2);
	esw_rmw(esw,
		       RT305X_ESW_REG_VLANI(vlan / 2),
		       RT305X_ESW_VLANI_VID_M << s,
		       (vid & RT305X_ESW_VLANI_VID_M) << s);
}

static unsigned esw_get_pvid(struct rt305x_esw *esw, unsigned port)
{
	unsigned s, val;

	s = RT305X_ESW_PVIDC_PVID_S * (port % 2);
	val = esw_r32(esw, RT305X_ESW_REG_PVIDC(port / 2));
	return (val >> s) & RT305X_ESW_PVIDC_PVID_M;
}

static void esw_set_pvid(struct rt305x_esw *esw, unsigned port, unsigned pvid)
{
	unsigned s;

	s = RT305X_ESW_PVIDC_PVID_S * (port % 2);
	esw_rmw(esw,
		       RT305X_ESW_REG_PVIDC(port / 2),
		       RT305X_ESW_PVIDC_PVID_M << s,
		       (pvid & RT305X_ESW_PVIDC_PVID_M) << s);
}

static unsigned esw_get_vmsc(struct rt305x_esw *esw, unsigned vlan)
{
	unsigned s, val;

	s = RT305X_ESW_VMSC_MSC_S * (vlan % 4);
	val = esw_r32(esw, RT305X_ESW_REG_VMSC(vlan / 4));
	val = (val >> s) & RT305X_ESW_VMSC_MSC_M;

	return val;
}

static void esw_set_vmsc(struct rt305x_esw *esw, unsigned vlan, unsigned msc)
{
	unsigned s;

	s = RT305X_ESW_VMSC_MSC_S * (vlan % 4);
	esw_rmw(esw,
		       RT305X_ESW_REG_VMSC(vlan / 4),
		       RT305X_ESW_VMSC_MSC_M << s,
		       (msc & RT305X_ESW_VMSC_MSC_M) << s);
}

static unsigned esw_get_port_disable(struct rt305x_esw *esw)
{
	unsigned reg;

	reg = esw_r32(esw, RT305X_ESW_REG_POC0);
	return (reg >> RT305X_ESW_POC0_DIS_PORT_S) &
	       RT305X_ESW_POC0_DIS_PORT_M;
}

static void esw_set_port_disable(struct rt305x_esw *esw, unsigned disable_mask)
{
	unsigned old_mask;
	unsigned enable_mask;
	unsigned changed;
	int i;

	old_mask = esw_get_port_disable(esw);
	changed = old_mask ^ disable_mask;
	enable_mask = old_mask & disable_mask;

	/* enable before writing to MII */
	esw_rmw(esw, RT305X_ESW_REG_POC0,
		       (RT305X_ESW_POC0_DIS_PORT_M <<
			RT305X_ESW_POC0_DIS_PORT_S),
		       enable_mask << RT305X_ESW_POC0_DIS_PORT_S);

	for (i = 0; i < RT305X_ESW_NUM_LEDS; i++) {
		if (!(changed & (1 << i)))
			continue;
		if (disable_mask & (1 << i)) {
			/* disable */
			rt305x_mii_write(esw, i, MII_BMCR,
					 BMCR_PDOWN);
		} else {
			/* enable */
			rt305x_mii_write(esw, i, MII_BMCR,
					 BMCR_FULLDPLX |
					 BMCR_ANENABLE |
					 BMCR_ANRESTART |
					 BMCR_SPEED100);
		}
	}

	/* disable after writing to MII */
	esw_rmw(esw, RT305X_ESW_REG_POC0,
		       (RT305X_ESW_POC0_DIS_PORT_M <<
			RT305X_ESW_POC0_DIS_PORT_S),
		       disable_mask << RT305X_ESW_POC0_DIS_PORT_S);
}

static void esw_set_gsc(struct rt305x_esw *esw)
{
	esw_rmw(esw, RT305X_ESW_REG_SGC,
		RT305X_ESW_GSC_BC_STROM_MASK << RT305X_ESW_GSC_BC_STROM_SHIFT,
		esw->bc_storm_protect << RT305X_ESW_GSC_BC_STROM_SHIFT);
	esw_rmw(esw, RT305X_ESW_REG_SGC,
		RT305X_ESW_GSC_LED_FREQ_MASK << RT305X_ESW_GSC_LED_FREQ_SHIFT,
		esw->led_frequency << RT305X_ESW_GSC_LED_FREQ_SHIFT);
}

static int esw_apply_config(struct switch_dev *dev);

static void esw_hw_init(struct rt305x_esw *esw)
{
	int i;
	u8 port_disable = 0;
	u8 port_map = RT305X_ESW_PMAP_LLLLLL;

	/* vodoo from original driver */
	esw_w32(esw, 0xC8A07850, RT305X_ESW_REG_FCT0);
	esw_w32(esw, 0x00000000, RT305X_ESW_REG_SGC2);
	/* Port priority 1 for all ports, vlan enabled. */
	esw_w32(esw, 0x00005555 |
		     (RT305X_ESW_PORTS_ALL << RT305X_ESW_PFC1_EN_VLAN_S),
		RT305X_ESW_REG_PFC1);

	/* Enable all ports, Back Pressure and Flow Control */
	esw_w32(esw, ((RT305X_ESW_PORTS_ALL << RT305X_ESW_POC0_EN_BP_S) |
		      (RT305X_ESW_PORTS_ALL << RT305X_ESW_POC0_EN_FC_S)),
		RT305X_ESW_REG_POC0);

	/* Enable Aging, and VLAN TAG removal */
	esw_w32(esw, ((RT305X_ESW_PORTS_ALL << RT305X_ESW_POC2_ENAGING_S) |
		      (RT305X_ESW_PORTS_NOCPU << RT305X_ESW_POC2_UNTAG_EN_S)),
		RT305X_ESW_REG_POC2);

	if (esw->reg_initval_fct2)
		esw_w32(esw, esw->reg_initval_fct2, RT305X_ESW_REG_FCT2);
	else
		esw_w32(esw, 0x0002500c, RT305X_ESW_REG_FCT2);

	/* 300s aging timer, max packet len 1536, broadcast storm prevention
	 * disabled, disable collision abort, mac xor48 hash, 10 packet back
	 * pressure jam, GMII disable was_transmit, back pressure disabled,
	 * 30ms led flash, unmatched IGMP as broadcast, rmc tb fault to all
	 * ports.
	 */
	esw_w32(esw, 0x0008a301, RT305X_ESW_REG_SGC);

	/* Setup SoC Port control register */
	esw_w32(esw,
		(RT305X_ESW_SOCPC_CRC_PADDING |
		(RT305X_ESW_PORTS_CPU << RT305X_ESW_SOCPC_DISUN2CPU_S) |
		(RT305X_ESW_PORTS_CPU << RT305X_ESW_SOCPC_DISMC2CPU_S) |
		(RT305X_ESW_PORTS_CPU << RT305X_ESW_SOCPC_DISBC2CPU_S)),
		RT305X_ESW_REG_SOCPC);

	/* ext phy base addr 31, enable port 5 polling, rx/tx clock skew 1,
	 * turbo mii off, rgmi 3.3v off
	 * port5: disabled
	 * port6: enabled, gige, full-duplex, rx/tx-flow-control
	 */
	if (esw->reg_initval_fpa2)
		esw_w32(esw, esw->reg_initval_fpa2, RT305X_ESW_REG_FPA2);
	else
		esw_w32(esw, 0x3f502b28, RT305X_ESW_REG_FPA2);
	esw_w32(esw, 0x00000000, RT305X_ESW_REG_FPA);

	/* Force Link/Activity on ports */
	esw_w32(esw, 0x00000005, RT305X_ESW_REG_P0LED);
	esw_w32(esw, 0x00000005, RT305X_ESW_REG_P1LED);
	esw_w32(esw, 0x00000005, RT305X_ESW_REG_P2LED);
	esw_w32(esw, 0x00000005, RT305X_ESW_REG_P3LED);
	esw_w32(esw, 0x00000005, RT305X_ESW_REG_P4LED);

	/* Copy disabled port configuration from device tree setup */
	port_disable = esw->port_disable;

	/* Disable nonexistent ports by reading the switch config
	 * after having enabled all possible ports above
	 */
	port_disable |= esw_get_port_disable(esw);

	for (i = 0; i < 6; i++)
		esw->ports[i].disable = (port_disable & (1 << i)) != 0;

	if (ralink_soc == RT305X_SOC_RT3352) {
		/* reset EPHY */
		fe_reset(RT5350_RESET_EPHY);

		rt305x_mii_write(esw, 0, 31, 0x8000);
		for (i = 0; i < 5; i++) {
			if (esw->ports[i].disable) {
				rt305x_mii_write(esw, i, MII_BMCR, BMCR_PDOWN);
			} else {
				rt305x_mii_write(esw, i, MII_BMCR,
						 BMCR_FULLDPLX |
						 BMCR_ANENABLE |
						 BMCR_SPEED100);
			}
			/* TX10 waveform coefficient LSB=0 disable PHY */
			rt305x_mii_write(esw, i, 26, 0x1601);
			/* TX100/TX10 AD/DA current bias */
			rt305x_mii_write(esw, i, 29, 0x7016);
			/* TX100 slew rate control */
			rt305x_mii_write(esw, i, 30, 0x0038);
		}

		/* select global register */
		rt305x_mii_write(esw, 0, 31, 0x0);
		/* enlarge agcsel threshold 3 and threshold 2 */
		rt305x_mii_write(esw, 0, 1, 0x4a40);
		/* enlarge agcsel threshold 5 and threshold 4 */
		rt305x_mii_write(esw, 0, 2, 0x6254);
		/* enlarge agcsel threshold  */
		rt305x_mii_write(esw, 0, 3, 0xa17f);
		rt305x_mii_write(esw, 0, 12, 0x7eaa);
		/* longer TP_IDL tail length */
		rt305x_mii_write(esw, 0, 14, 0x65);
		/* increased squelch pulse count threshold. */
		rt305x_mii_write(esw, 0, 16, 0x0684);
		/* set TX10 signal amplitude threshold to minimum */
		rt305x_mii_write(esw, 0, 17, 0x0fe0);
		/* set squelch amplitude to higher threshold */
		rt305x_mii_write(esw, 0, 18, 0x40ba);
		/* tune TP_IDL tail and head waveform, enable power
		 * down slew rate control
		 */
		rt305x_mii_write(esw, 0, 22, 0x253f);
		/* set PLL/Receive bias current are calibrated */
		rt305x_mii_write(esw, 0, 27, 0x2fda);
		/* change PLL/Receive bias current to internal(RT3350) */
		rt305x_mii_write(esw, 0, 28, 0xc410);
		/* change PLL bias current to internal(RT3052_MP3) */
		rt305x_mii_write(esw, 0, 29, 0x598b);
		/* select local register */
		rt305x_mii_write(esw, 0, 31, 0x8000);
	} else if (ralink_soc == RT305X_SOC_RT5350) {
		/* reset EPHY */
		fe_reset(RT5350_RESET_EPHY);

		/* set the led polarity */
		esw_w32(esw, esw->reg_led_polarity & 0x1F,
			RT5350_EWS_REG_LED_POLARITY);

		/* local registers */
		rt305x_mii_write(esw, 0, 31, 0x8000);
		for (i = 0; i < 5; i++) {
			if (esw->ports[i].disable) {
				rt305x_mii_write(esw, i, MII_BMCR, BMCR_PDOWN);
			} else {
				rt305x_mii_write(esw, i, MII_BMCR,
						 BMCR_FULLDPLX |
						 BMCR_ANENABLE |
						 BMCR_SPEED100);
			}
			/* TX10 waveform coefficient LSB=0 disable PHY */
			rt305x_mii_write(esw, i, 26, 0x1601);
			/* TX100/TX10 AD/DA current bias */
			rt305x_mii_write(esw, i, 29, 0x7015);
			/* TX100 slew rate control */
			rt305x_mii_write(esw, i, 30, 0x0038);
		}

		/* global registers */
		rt305x_mii_write(esw, 0, 31, 0x0);
		/* enlarge agcsel threshold 3 and threshold 2 */
		rt305x_mii_write(esw, 0, 1, 0x4a40);
		/* enlarge agcsel threshold 5 and threshold 4 */
		rt305x_mii_write(esw, 0, 2, 0x6254);
		/* enlarge agcsel threshold 6 */
		rt305x_mii_write(esw, 0, 3, 0xa17f);
		rt305x_mii_write(esw, 0, 12, 0x7eaa);
		/* longer TP_IDL tail length */
		rt305x_mii_write(esw, 0, 14, 0x65);
		/* increased squelch pulse count threshold. */
		rt305x_mii_write(esw, 0, 16, 0x0684);
		/* set TX10 signal amplitude threshold to minimum */
		rt305x_mii_write(esw, 0, 17, 0x0fe0);
		/* set squelch amplitude to higher threshold */
		rt305x_mii_write(esw, 0, 18, 0x40ba);
		/* tune TP_IDL tail and head waveform, enable power
		 * down slew rate control
		 */
		rt305x_mii_write(esw, 0, 22, 0x253f);
		/* set PLL/Receive bias current are calibrated */
		rt305x_mii_write(esw, 0, 27, 0x2fda);
		/* change PLL/Receive bias current to internal(RT3350) */
		rt305x_mii_write(esw, 0, 28, 0xc410);
		/* change PLL bias current to internal(RT3052_MP3) */
		rt305x_mii_write(esw, 0, 29, 0x598b);
		/* select local register */
		rt305x_mii_write(esw, 0, 31, 0x8000);
	} else if (ralink_soc == MT762X_SOC_MT7628AN || ralink_soc == MT762X_SOC_MT7688) {
		int i;

		/* reset EPHY */
		fe_reset(RT5350_RESET_EPHY);

		rt305x_mii_write(esw, 0, 31, 0x2000); /* change G2 page */
		rt305x_mii_write(esw, 0, 26, 0x0020);

		for (i = 0; i < 5; i++) {
			rt305x_mii_write(esw, i, 31, 0x8000);
			rt305x_mii_write(esw, i,  0, 0x3100);
			rt305x_mii_write(esw, i, 30, 0xa000);
			rt305x_mii_write(esw, i, 31, 0xa000);
			rt305x_mii_write(esw, i, 16, 0x0606);
			rt305x_mii_write(esw, i, 23, 0x0f0e);
			rt305x_mii_write(esw, i, 24, 0x1610);
			rt305x_mii_write(esw, i, 30, 0x1f15);
			rt305x_mii_write(esw, i, 28, 0x6111);
			rt305x_mii_write(esw, i, 31, 0x2000);
			rt305x_mii_write(esw, i, 26, 0x0000);
		}

		/* 100Base AOI setting */
		rt305x_mii_write(esw, 0, 31, 0x5000);
		rt305x_mii_write(esw, 0, 19, 0x004a);
		rt305x_mii_write(esw, 0, 20, 0x015a);
		rt305x_mii_write(esw, 0, 21, 0x00ee);
		rt305x_mii_write(esw, 0, 22, 0x0033);
		rt305x_mii_write(esw, 0, 23, 0x020a);
		rt305x_mii_write(esw, 0, 24, 0x0000);
		rt305x_mii_write(esw, 0, 25, 0x024a);
		rt305x_mii_write(esw, 0, 26, 0x035a);
		rt305x_mii_write(esw, 0, 27, 0x02ee);
		rt305x_mii_write(esw, 0, 28, 0x0233);
		rt305x_mii_write(esw, 0, 29, 0x000a);
		rt305x_mii_write(esw, 0, 30, 0x0000);
	} else {
		rt305x_mii_write(esw, 0, 31, 0x8000);
		for (i = 0; i < 5; i++) {
			if (esw->ports[i].disable) {
				rt305x_mii_write(esw, i, MII_BMCR, BMCR_PDOWN);
			} else {
				rt305x_mii_write(esw, i, MII_BMCR,
						 BMCR_FULLDPLX |
						 BMCR_ANENABLE |
						 BMCR_SPEED100);
			}
			/* TX10 waveform coefficient */
			rt305x_mii_write(esw, i, 26, 0x1601);
			/* TX100/TX10 AD/DA current bias */
			rt305x_mii_write(esw, i, 29, 0x7058);
			/* TX100 slew rate control */
			rt305x_mii_write(esw, i, 30, 0x0018);
		}

		/* PHY IOT */
		/* select global register */
		rt305x_mii_write(esw, 0, 31, 0x0);
		/* tune TP_IDL tail and head waveform */
		rt305x_mii_write(esw, 0, 22, 0x052f);
		/* set TX10 signal amplitude threshold to minimum */
		rt305x_mii_write(esw, 0, 17, 0x0fe0);
		/* set squelch amplitude to higher threshold */
		rt305x_mii_write(esw, 0, 18, 0x40ba);
		/* longer TP_IDL tail length */
		rt305x_mii_write(esw, 0, 14, 0x65);
		/* select local register */
		rt305x_mii_write(esw, 0, 31, 0x8000);
	}

	if (esw->port_map)
		port_map = esw->port_map;
	else
		port_map = RT305X_ESW_PMAP_LLLLLL;

	/* Unused HW feature, but still nice to be consistent here...
	 * This is also exported to userspace ('lan' attribute) so it's
	 * conveniently usable to decide which ports go into the wan vlan by
	 * default.
	 */
	esw_rmw(esw, RT305X_ESW_REG_SGC2,
		RT305X_ESW_SGC2_LAN_PMAP_M << RT305X_ESW_SGC2_LAN_PMAP_S,
		port_map << RT305X_ESW_SGC2_LAN_PMAP_S);

	/* make the switch leds blink */
	for (i = 0; i < RT305X_ESW_NUM_LEDS; i++)
		esw->ports[i].led = 0x05;

	/* Apply the empty config. */
	esw_apply_config(&esw->swdev);

	/* Only unmask the port change interrupt */
	esw_w32(esw, ~RT305X_ESW_PORT_ST_CHG, RT305X_ESW_REG_IMR);
}

static irqreturn_t esw_interrupt(int irq, void *_esw)
{
	struct rt305x_esw *esw = (struct rt305x_esw *)_esw;
	u32 status;

	status = esw_r32(esw, RT305X_ESW_REG_ISR);
	if (status & RT305X_ESW_PORT_ST_CHG) {
		u32 link = esw_r32(esw, RT305X_ESW_REG_POA);

		link >>= RT305X_ESW_POA_LINK_SHIFT;
		link &= RT305X_ESW_POA_LINK_MASK;
		dev_info(esw->dev, "link changed 0x%02X\n", link);
	}
	esw_w32(esw, status, RT305X_ESW_REG_ISR);

	return IRQ_HANDLED;
}

static int esw_apply_config(struct switch_dev *dev)
{
	struct rt305x_esw *esw = container_of(dev, struct rt305x_esw, swdev);
	int i;
	u8 disable = 0;
	u8 doubletag = 0;
	u8 en_vlan = 0;
	u8 untag = 0;

	for (i = 0; i < RT305X_ESW_NUM_VLANS; i++) {
		u32 vid, vmsc;
		if (esw->global_vlan_enable) {
			vid = esw->vlans[i].vid;
			vmsc = esw->vlans[i].ports;
		} else {
			vid = RT305X_ESW_VLAN_NONE;
			vmsc = RT305X_ESW_PORTS_NONE;
		}
		esw_set_vlan_id(esw, i, vid);
		esw_set_vmsc(esw, i, vmsc);
	}

	for (i = 0; i < RT305X_ESW_NUM_PORTS; i++) {
		u32 pvid;
		disable |= esw->ports[i].disable << i;
		if (esw->global_vlan_enable) {
			doubletag |= esw->ports[i].doubletag << i;
			en_vlan   |= 1                       << i;
			untag     |= esw->ports[i].untag     << i;
			pvid       = esw->ports[i].pvid;
		} else {
			int x = esw->alt_vlan_disable ? 0 : 1;
			doubletag |= x << i;
			en_vlan   |= x << i;
			untag     |= x << i;
			pvid       = 0;
		}
		esw_set_pvid(esw, i, pvid);
		if (i < RT305X_ESW_NUM_LEDS)
			esw_w32(esw, esw->ports[i].led,
				      RT305X_ESW_REG_P0LED + 4*i);
	}

	esw_set_gsc(esw);
	esw_set_port_disable(esw, disable);
	esw_rmw(esw, RT305X_ESW_REG_SGC2,
		       (RT305X_ESW_SGC2_DOUBLE_TAG_M <<
			RT305X_ESW_SGC2_DOUBLE_TAG_S),
		       doubletag << RT305X_ESW_SGC2_DOUBLE_TAG_S);
	esw_rmw(esw, RT305X_ESW_REG_PFC1,
		       RT305X_ESW_PFC1_EN_VLAN_M << RT305X_ESW_PFC1_EN_VLAN_S,
		       en_vlan << RT305X_ESW_PFC1_EN_VLAN_S);
	esw_rmw(esw, RT305X_ESW_REG_POC2,
		       RT305X_ESW_POC2_UNTAG_EN_M << RT305X_ESW_POC2_UNTAG_EN_S,
		       untag << RT305X_ESW_POC2_UNTAG_EN_S);

	if (!esw->global_vlan_enable) {
		/*
		 * Still need to put all ports into vlan 0 or they'll be
		 * isolated.
		 * NOTE: vlan 0 is special, no vlan tag is prepended
		 */
		esw_set_vlan_id(esw, 0, 0);
		esw_set_vmsc(esw, 0, RT305X_ESW_PORTS_ALL);
	}

	return 0;
}

static int esw_reset_switch(struct switch_dev *dev)
{
	struct rt305x_esw *esw = container_of(dev, struct rt305x_esw, swdev);

	esw->global_vlan_enable = 0;
	memset(esw->ports, 0, sizeof(esw->ports));
	memset(esw->vlans, 0, sizeof(esw->vlans));
	esw_hw_init(esw);

	return 0;
}

static int esw_get_vlan_enable(struct switch_dev *dev,
			   const struct switch_attr *attr,
			   struct switch_val *val)
{
	struct rt305x_esw *esw = container_of(dev, struct rt305x_esw, swdev);

	val->value.i = esw->global_vlan_enable;

	return 0;
}

static int esw_set_vlan_enable(struct switch_dev *dev,
			   const struct switch_attr *attr,
			   struct switch_val *val)
{
	struct rt305x_esw *esw = container_of(dev, struct rt305x_esw, swdev);

	esw->global_vlan_enable = val->value.i != 0;

	return 0;
}

static int esw_get_alt_vlan_disable(struct switch_dev *dev,
				const struct switch_attr *attr,
				struct switch_val *val)
{
	struct rt305x_esw *esw = container_of(dev, struct rt305x_esw, swdev);

	val->value.i = esw->alt_vlan_disable;

	return 0;
}

static int esw_set_alt_vlan_disable(struct switch_dev *dev,
				const struct switch_attr *attr,
				struct switch_val *val)
{
	struct rt305x_esw *esw = container_of(dev, struct rt305x_esw, swdev);

	esw->alt_vlan_disable = val->value.i != 0;

	return 0;
}

static int
rt305x_esw_set_bc_status(struct switch_dev *dev,
			const struct switch_attr *attr,
			struct switch_val *val)
{
	struct rt305x_esw *esw = container_of(dev, struct rt305x_esw, swdev);

	esw->bc_storm_protect = val->value.i & RT305X_ESW_GSC_BC_STROM_MASK;

	return 0;
}

static int
rt305x_esw_get_bc_status(struct switch_dev *dev,
			const struct switch_attr *attr,
			struct switch_val *val)
{
	struct rt305x_esw *esw = container_of(dev, struct rt305x_esw, swdev);

	val->value.i = esw->bc_storm_protect;

	return 0;
}

static int
rt305x_esw_set_led_freq(struct switch_dev *dev,
			const struct switch_attr *attr,
			struct switch_val *val)
{
	struct rt305x_esw *esw = container_of(dev, struct rt305x_esw, swdev);

	esw->led_frequency = val->value.i & RT305X_ESW_GSC_LED_FREQ_MASK;

	return 0;
}

static int
rt305x_esw_get_led_freq(struct switch_dev *dev,
			const struct switch_attr *attr,
			struct switch_val *val)
{
	struct rt305x_esw *esw = container_of(dev, struct rt305x_esw, swdev);

	val->value.i = esw->led_frequency;

	return 0;
}

static int esw_get_port_link(struct switch_dev *dev,
			 int port,
			 struct switch_port_link *link)
{
	struct rt305x_esw *esw = container_of(dev, struct rt305x_esw, swdev);
	u32 speed, poa;

	if (port < 0 || port >= RT305X_ESW_NUM_PORTS)
		return -EINVAL;

	poa = esw_r32(esw, RT305X_ESW_REG_POA) >> port;

	link->link = (poa >> RT305X_ESW_LINK_S) & 1;
	link->duplex = (poa >> RT305X_ESW_DUPLEX_S) & 1;
	if (port < RT305X_ESW_NUM_LEDS) {
		speed = (poa >> RT305X_ESW_SPD_S) & 1;
	} else {
		if (port == RT305X_ESW_NUM_PORTS - 1)
			poa >>= 1;
		speed = (poa >> RT305X_ESW_SPD_S) & 3;
	}
	switch (speed) {
	case 0:
		link->speed = SWITCH_PORT_SPEED_10;
		break;
	case 1:
		link->speed = SWITCH_PORT_SPEED_100;
		break;
	case 2:
	case 3: /* forced gige speed can be 2 or 3 */
		link->speed = SWITCH_PORT_SPEED_1000;
		break;
	default:
		link->speed = SWITCH_PORT_SPEED_UNKNOWN;
		break;
	}

	return 0;
}

static int esw_get_port_bool(struct switch_dev *dev,
			 const struct switch_attr *attr,
			 struct switch_val *val)
{
	struct rt305x_esw *esw = container_of(dev, struct rt305x_esw, swdev);
	int idx = val->port_vlan;
	u32 x, reg, shift;

	if (idx < 0 || idx >= RT305X_ESW_NUM_PORTS)
		return -EINVAL;

	switch (attr->id) {
	case RT305X_ESW_ATTR_PORT_DISABLE:
		reg = RT305X_ESW_REG_POC0;
		shift = RT305X_ESW_POC0_DIS_PORT_S;
		break;
	case RT305X_ESW_ATTR_PORT_DOUBLETAG:
		reg = RT305X_ESW_REG_SGC2;
		shift = RT305X_ESW_SGC2_DOUBLE_TAG_S;
		break;
	case RT305X_ESW_ATTR_PORT_UNTAG:
		reg = RT305X_ESW_REG_POC2;
		shift = RT305X_ESW_POC2_UNTAG_EN_S;
		break;
	case RT305X_ESW_ATTR_PORT_LAN:
		reg = RT305X_ESW_REG_SGC2;
		shift = RT305X_ESW_SGC2_LAN_PMAP_S;
		if (idx >= RT305X_ESW_NUM_LANWAN)
			return -EINVAL;
		break;
	default:
		return -EINVAL;
	}

	x = esw_r32(esw, reg);
	val->value.i = (x >> (idx + shift)) & 1;

	return 0;
}

static int esw_set_port_bool(struct switch_dev *dev,
			 const struct switch_attr *attr,
			 struct switch_val *val)
{
	struct rt305x_esw *esw = container_of(dev, struct rt305x_esw, swdev);
	int idx = val->port_vlan;

	if (idx < 0 || idx >= RT305X_ESW_NUM_PORTS ||
	    val->value.i < 0 || val->value.i > 1)
		return -EINVAL;

	switch (attr->id) {
	case RT305X_ESW_ATTR_PORT_DISABLE:
		esw->ports[idx].disable = val->value.i;
		break;
	case RT305X_ESW_ATTR_PORT_DOUBLETAG:
		esw->ports[idx].doubletag = val->value.i;
		break;
	case RT305X_ESW_ATTR_PORT_UNTAG:
		esw->ports[idx].untag = val->value.i;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int esw_get_port_recv_badgood(struct switch_dev *dev,
				 const struct switch_attr *attr,
				 struct switch_val *val)
{
	struct rt305x_esw *esw = container_of(dev, struct rt305x_esw, swdev);
	int idx = val->port_vlan;
	int shift = attr->id == RT305X_ESW_ATTR_PORT_RECV_GOOD ? 0 : 16;
	u32 reg;

	if (idx < 0 || idx >= RT305X_ESW_NUM_LANWAN)
		return -EINVAL;
	reg = esw_r32(esw, RT305X_ESW_REG_PXPC(idx));
	val->value.i = (reg >> shift) & 0xffff;

	return 0;
}

static int
esw_get_port_tr_badgood(struct switch_dev *dev,
				 const struct switch_attr *attr,
				 struct switch_val *val)
{
	struct rt305x_esw *esw = container_of(dev, struct rt305x_esw, swdev);

	int idx = val->port_vlan;
	int shift = attr->id == RT5350_ESW_ATTR_PORT_TR_GOOD ? 0 : 16;
	u32 reg;

	if ((ralink_soc != RT305X_SOC_RT5350) && (ralink_soc != MT762X_SOC_MT7628AN) && (ralink_soc != MT762X_SOC_MT7688))
		return -EINVAL;

	if (idx < 0 || idx >= RT305X_ESW_NUM_LANWAN)
		return -EINVAL;

	reg = esw_r32(esw, RT5350_ESW_REG_PXTPC(idx));
	val->value.i = (reg >> shift) & 0xffff;

	return 0;
}

static int esw_get_port_led(struct switch_dev *dev,
			const struct switch_attr *attr,
			struct switch_val *val)
{
	struct rt305x_esw *esw = container_of(dev, struct rt305x_esw, swdev);
	int idx = val->port_vlan;

	if (idx < 0 || idx >= RT305X_ESW_NUM_PORTS ||
	    idx >= RT305X_ESW_NUM_LEDS)
		return -EINVAL;

	val->value.i = esw_r32(esw, RT305X_ESW_REG_P0LED + 4*idx);

	return 0;
}

static int esw_set_port_led(struct switch_dev *dev,
			const struct switch_attr *attr,
			struct switch_val *val)
{
	struct rt305x_esw *esw = container_of(dev, struct rt305x_esw, swdev);
	int idx = val->port_vlan;

	if (idx < 0 || idx >= RT305X_ESW_NUM_LEDS)
		return -EINVAL;

	esw->ports[idx].led = val->value.i;

	return 0;
}

static int esw_get_port_pvid(struct switch_dev *dev, int port, int *val)
{
	struct rt305x_esw *esw = container_of(dev, struct rt305x_esw, swdev);

	if (port >= RT305X_ESW_NUM_PORTS)
		return -EINVAL;

	*val = esw_get_pvid(esw, port);

	return 0;
}

static int esw_set_port_pvid(struct switch_dev *dev, int port, int val)
{
	struct rt305x_esw *esw = container_of(dev, struct rt305x_esw, swdev);

	if (port >= RT305X_ESW_NUM_PORTS)
		return -EINVAL;

	esw->ports[port].pvid = val;

	return 0;
}

static int esw_get_vlan_ports(struct switch_dev *dev, struct switch_val *val)
{
	struct rt305x_esw *esw = container_of(dev, struct rt305x_esw, swdev);
	u32 vmsc, poc2;
	int vlan_idx = -1;
	int i;

	val->len = 0;

	if (val->port_vlan < 0 || val->port_vlan >= RT305X_ESW_NUM_VIDS)
		return -EINVAL;

	/* valid vlan? */
	for (i = 0; i < RT305X_ESW_NUM_VLANS; i++) {
		if (esw_get_vlan_id(esw, i) == val->port_vlan &&
		    esw_get_vmsc(esw, i) != RT305X_ESW_PORTS_NONE) {
			vlan_idx = i;
			break;
		}
	}

	if (vlan_idx == -1)
		return -EINVAL;

	vmsc = esw_get_vmsc(esw, vlan_idx);
	poc2 = esw_r32(esw, RT305X_ESW_REG_POC2);

	for (i = 0; i < RT305X_ESW_NUM_PORTS; i++) {
		struct switch_port *p;
		int port_mask = 1 << i;

		if (!(vmsc & port_mask))
			continue;

		p = &val->value.ports[val->len++];
		p->id = i;
		if (poc2 & (port_mask << RT305X_ESW_POC2_UNTAG_EN_S))
			p->flags = 0;
		else
			p->flags = 1 << SWITCH_PORT_FLAG_TAGGED;
	}

	return 0;
}

static int esw_set_vlan_ports(struct switch_dev *dev, struct switch_val *val)
{
	struct rt305x_esw *esw = container_of(dev, struct rt305x_esw, swdev);
	int ports;
	int vlan_idx = -1;
	int i;

	if (val->port_vlan < 0 || val->port_vlan >= RT305X_ESW_NUM_VIDS ||
	    val->len > RT305X_ESW_NUM_PORTS)
		return -EINVAL;

	/* one of the already defined vlans? */
	for (i = 0; i < RT305X_ESW_NUM_VLANS; i++) {
		if (esw->vlans[i].vid == val->port_vlan &&
		    esw->vlans[i].ports != RT305X_ESW_PORTS_NONE) {
			vlan_idx = i;
			break;
		}
	}

	/* select a free slot */
	for (i = 0; vlan_idx == -1 && i < RT305X_ESW_NUM_VLANS; i++) {
		if (esw->vlans[i].ports == RT305X_ESW_PORTS_NONE)
			vlan_idx = i;
	}

	/* bail if all slots are in use */
	if (vlan_idx == -1)
		return -EINVAL;

	ports = RT305X_ESW_PORTS_NONE;
	for (i = 0; i < val->len; i++) {
		struct switch_port *p = &val->value.ports[i];
		int port_mask = 1 << p->id;
		bool untagged = !(p->flags & (1 << SWITCH_PORT_FLAG_TAGGED));

		if (p->id >= RT305X_ESW_NUM_PORTS)
			return -EINVAL;

		ports |= port_mask;
		esw->ports[p->id].untag = untagged;
	}
	esw->vlans[vlan_idx].ports = ports;
	if (ports == RT305X_ESW_PORTS_NONE)
		esw->vlans[vlan_idx].vid = RT305X_ESW_VLAN_NONE;
	else
		esw->vlans[vlan_idx].vid = val->port_vlan;

	return 0;
}

static const struct switch_attr esw_global[] = {
	{
		.type = SWITCH_TYPE_INT,
		.name = "enable_vlan",
		.description = "VLAN mode (1:enabled)",
		.max = 1,
		.id = RT305X_ESW_ATTR_ENABLE_VLAN,
		.get = esw_get_vlan_enable,
		.set = esw_set_vlan_enable,
	},
	{
		.type = SWITCH_TYPE_INT,
		.name = "alternate_vlan_disable",
		.description = "Use en_vlan instead of doubletag to disable"
				" VLAN mode",
		.max = 1,
		.id = RT305X_ESW_ATTR_ALT_VLAN_DISABLE,
		.get = esw_get_alt_vlan_disable,
		.set = esw_set_alt_vlan_disable,
	},
	{
		.type = SWITCH_TYPE_INT,
		.name = "bc_storm_protect",
		.description = "Global broadcast storm protection (0:Disable, 1:64 blocks, 2:96 blocks, 3:128 blocks)",
		.max = 3,
		.id = RT305X_ESW_ATTR_BC_STATUS,
		.get = rt305x_esw_get_bc_status,
		.set = rt305x_esw_set_bc_status,
	},
	{
		.type = SWITCH_TYPE_INT,
		.name = "led_frequency",
		.description = "LED Flash frequency (0:30mS, 1:60mS, 2:240mS, 3:480mS)",
		.max = 3,
		.id = RT305X_ESW_ATTR_LED_FREQ,
		.get = rt305x_esw_get_led_freq,
		.set = rt305x_esw_set_led_freq,
	}
};

static const struct switch_attr esw_port[] = {
	{
		.type = SWITCH_TYPE_INT,
		.name = "disable",
		.description = "Port state (1:disabled)",
		.max = 1,
		.id = RT305X_ESW_ATTR_PORT_DISABLE,
		.get = esw_get_port_bool,
		.set = esw_set_port_bool,
	},
	{
		.type = SWITCH_TYPE_INT,
		.name = "doubletag",
		.description = "Double tagging for incoming vlan packets "
				"(1:enabled)",
		.max = 1,
		.id = RT305X_ESW_ATTR_PORT_DOUBLETAG,
		.get = esw_get_port_bool,
		.set = esw_set_port_bool,
	},
	{
		.type = SWITCH_TYPE_INT,
		.name = "untag",
		.description = "Untag (1:strip outgoing vlan tag)",
		.max = 1,
		.id = RT305X_ESW_ATTR_PORT_UNTAG,
		.get = esw_get_port_bool,
		.set = esw_set_port_bool,
	},
	{
		.type = SWITCH_TYPE_INT,
		.name = "led",
		.description = "LED mode (0:link, 1:100m, 2:duplex, 3:activity,"
				" 4:collision, 5:linkact, 6:duplcoll, 7:10mact,"
				" 8:100mact, 10:blink, 11:off, 12:on)",
		.max = 15,
		.id = RT305X_ESW_ATTR_PORT_LED,
		.get = esw_get_port_led,
		.set = esw_set_port_led,
	},
	{
		.type = SWITCH_TYPE_INT,
		.name = "lan",
		.description = "HW port group (0:wan, 1:lan)",
		.max = 1,
		.id = RT305X_ESW_ATTR_PORT_LAN,
		.get = esw_get_port_bool,
	},
	{
		.type = SWITCH_TYPE_INT,
		.name = "recv_bad",
		.description = "Receive bad packet counter",
		.id = RT305X_ESW_ATTR_PORT_RECV_BAD,
		.get = esw_get_port_recv_badgood,
	},
	{
		.type = SWITCH_TYPE_INT,
		.name = "recv_good",
		.description = "Receive good packet counter",
		.id = RT305X_ESW_ATTR_PORT_RECV_GOOD,
		.get = esw_get_port_recv_badgood,
	},
	{
		.type = SWITCH_TYPE_INT,
		.name = "tr_bad",

		.description = "Transmit bad packet counter. rt5350 only",
		.id = RT5350_ESW_ATTR_PORT_TR_BAD,
		.get = esw_get_port_tr_badgood,
	},
	{
		.type = SWITCH_TYPE_INT,
		.name = "tr_good",

		.description = "Transmit good packet counter. rt5350 only",
		.id = RT5350_ESW_ATTR_PORT_TR_GOOD,
		.get = esw_get_port_tr_badgood,
	},
};

static const struct switch_attr esw_vlan[] = {
};

static const struct switch_dev_ops esw_ops = {
	.attr_global = {
		.attr = esw_global,
		.n_attr = ARRAY_SIZE(esw_global),
	},
	.attr_port = {
		.attr = esw_port,
		.n_attr = ARRAY_SIZE(esw_port),
	},
	.attr_vlan = {
		.attr = esw_vlan,
		.n_attr = ARRAY_SIZE(esw_vlan),
	},
	.get_vlan_ports = esw_get_vlan_ports,
	.set_vlan_ports = esw_set_vlan_ports,
	.get_port_pvid = esw_get_port_pvid,
	.set_port_pvid = esw_set_port_pvid,
	.get_port_link = esw_get_port_link,
	.apply_config = esw_apply_config,
	.reset_switch = esw_reset_switch,
};

static int esw_probe(struct platform_device *pdev)
{
	struct resource *res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	struct device_node *np = pdev->dev.of_node;
	const __be32 *port_map, *port_disable, *reg_init;
	struct switch_dev *swdev;
	struct rt305x_esw *esw;
	int ret;

	esw = devm_kzalloc(&pdev->dev, sizeof(*esw), GFP_KERNEL);
	if (!esw)
		return -ENOMEM;

	esw->dev = &pdev->dev;
	esw->irq = irq_of_parse_and_map(np, 0);
	esw->base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(esw->base))
		return PTR_ERR(esw->base);

	port_map = of_get_property(np, "mediatek,portmap", NULL);
	if (port_map)
		esw->port_map = be32_to_cpu(*port_map);

	port_disable = of_get_property(np, "mediatek,portdisable", NULL);
	if (port_disable)
		esw->port_disable = be32_to_cpu(*port_disable);

	reg_init = of_get_property(np, "ralink,fct2", NULL);
	if (reg_init)
		esw->reg_initval_fct2 = be32_to_cpu(*reg_init);

	reg_init = of_get_property(np, "ralink,fpa2", NULL);
	if (reg_init)
		esw->reg_initval_fpa2 = be32_to_cpu(*reg_init);

	reg_init = of_get_property(np, "mediatek,led_polarity", NULL);
	if (reg_init)
		esw->reg_led_polarity = be32_to_cpu(*reg_init);

	swdev = &esw->swdev;
	swdev->of_node = pdev->dev.of_node;
	swdev->name = "rt305x-esw";
	swdev->alias = "rt305x";
	swdev->cpu_port = RT305X_ESW_PORT6;
	swdev->ports = RT305X_ESW_NUM_PORTS;
	swdev->vlans = RT305X_ESW_NUM_VIDS;
	swdev->ops = &esw_ops;

	ret = register_switch(swdev, NULL);
	if (ret < 0) {
		dev_err(&pdev->dev, "register_switch failed\n");
		return ret;
	}

	platform_set_drvdata(pdev, esw);

	spin_lock_init(&esw->reg_rw_lock);

	esw_hw_init(esw);

	reg_init = of_get_property(np, "ralink,rgmii", NULL);
	if (reg_init && be32_to_cpu(*reg_init) == 1) {
		/* 
		 * External switch connected to RGMII interface. 
		 * Unregister the switch device after initialization. 
		 */
		dev_err(&pdev->dev, "RGMII mode, not exporting switch device.\n");
		unregister_switch(&esw->swdev);
		platform_set_drvdata(pdev, NULL);
		return -ENODEV;
	}

	ret = devm_request_irq(&pdev->dev, esw->irq, esw_interrupt, 0, "esw",
			       esw);

	if (!ret) {
		esw_w32(esw, RT305X_ESW_PORT_ST_CHG, RT305X_ESW_REG_ISR);
		esw_w32(esw, ~RT305X_ESW_PORT_ST_CHG, RT305X_ESW_REG_IMR);
	}

	return ret;
}

static int esw_remove(struct platform_device *pdev)
{
	struct rt305x_esw *esw = platform_get_drvdata(pdev);

	if (esw) {
		esw_w32(esw, ~0, RT305X_ESW_REG_IMR);
		platform_set_drvdata(pdev, NULL);
	}

	return 0;
}

static const struct of_device_id ralink_esw_match[] = {
	{ .compatible = "ralink,rt3050-esw" },
	{},
};
MODULE_DEVICE_TABLE(of, ralink_esw_match);

static struct platform_driver esw_driver = {
	.probe = esw_probe,
	.remove = esw_remove,
	.driver = {
		.name = "rt3050-esw",
		.owner = THIS_MODULE,
		.of_match_table = ralink_esw_match,
	},
};

module_platform_driver(esw_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("John Crispin <blogic@openwrt.org>");
MODULE_DESCRIPTION("Switch driver for RT305X SoC");
MODULE_VERSION(MTK_FE_DRV_VERSION);
