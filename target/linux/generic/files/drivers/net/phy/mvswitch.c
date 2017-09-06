/*
 * Marvell 88E6060 switch driver
 * Copyright (c) 2008 Felix Fietkau <nbd@nbd.name>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation
 */
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/unistd.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/spinlock.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/mii.h>
#include <linux/ethtool.h>
#include <linux/phy.h>
#include <linux/if_vlan.h>

#include <asm/io.h>
#include <asm/irq.h>
#include <asm/uaccess.h>
#include "mvswitch.h"

/* Undefine this to use trailer mode instead.
 * I don't know if header mode works with all chips */
#define HEADER_MODE	1

MODULE_DESCRIPTION("Marvell 88E6060 Switch driver");
MODULE_AUTHOR("Felix Fietkau");
MODULE_LICENSE("GPL");

#define MVSWITCH_MAGIC 0x88E6060

struct mvswitch_priv {
	netdev_features_t orig_features;
	u8 vlans[16];
};

#define to_mvsw(_phy) ((struct mvswitch_priv *) (_phy)->priv)

static inline u16
r16(struct phy_device *phydev, int addr, int reg)
{
	struct mii_bus *bus = phydev->mdio.bus;

	return bus->read(bus, addr, reg);
}

static inline void
w16(struct phy_device *phydev, int addr, int reg, u16 val)
{
	struct mii_bus *bus = phydev->mdio.bus;

	bus->write(bus, addr, reg, val);
}


static struct sk_buff *
mvswitch_mangle_tx(struct net_device *dev, struct sk_buff *skb)
{
	struct mvswitch_priv *priv;
	char *buf = NULL;
	u16 vid;

	priv = dev->phy_ptr;
	if (unlikely(!priv))
		goto error;

	if (unlikely(skb->len < 16))
		goto error;

#ifdef HEADER_MODE
	if (__vlan_hwaccel_get_tag(skb, &vid))
		goto error;

	if (skb_cloned(skb) || (skb->len <= 62) || (skb_headroom(skb) < MV_HEADER_SIZE)) {
		if (pskb_expand_head(skb, MV_HEADER_SIZE, (skb->len < 62 ? 62 - skb->len : 0), GFP_ATOMIC))
			goto error_expand;
		if (skb->len < 62)
			skb->len = 62;
	}
	buf = skb_push(skb, MV_HEADER_SIZE);
#else
	if (__vlan_get_tag(skb, &vid))
		goto error;

	if (unlikely((vid > 15 || !priv->vlans[vid])))
		goto error;

	if (skb->len <= 64) {
		if (pskb_expand_head(skb, 0, 64 + MV_TRAILER_SIZE - skb->len, GFP_ATOMIC))
			goto error_expand;

		buf = skb->data + 64;
		skb->len = 64 + MV_TRAILER_SIZE;
	} else {
		if (skb_cloned(skb) || unlikely(skb_tailroom(skb) < 4)) {
			if (pskb_expand_head(skb, 0, 4, GFP_ATOMIC))
				goto error_expand;
		}
		buf = skb_put(skb, 4);
	}

	/* move the ethernet header 4 bytes forward, overwriting the vlan tag */
	memmove(skb->data + 4, skb->data, 12);
	skb->data += 4;
	skb->len -= 4;
	skb->mac_header += 4;
#endif

	if (!buf)
		goto error;


#ifdef HEADER_MODE
	/* prepend the tag */
	*((__be16 *) buf) = cpu_to_be16(
		((vid << MV_HEADER_VLAN_S) & MV_HEADER_VLAN_M) |
		((priv->vlans[vid] << MV_HEADER_PORTS_S) & MV_HEADER_PORTS_M)
	);
#else
	/* append the tag */
	*((__be32 *) buf) = cpu_to_be32((
		(MV_TRAILER_OVERRIDE << MV_TRAILER_FLAGS_S) |
		((priv->vlans[vid] & MV_TRAILER_PORTS_M) << MV_TRAILER_PORTS_S)
	));
#endif

	return skb;

error_expand:
	if (net_ratelimit())
		printk("%s: failed to expand/update skb for the switch\n", dev->name);

error:
	/* any errors? drop the packet! */
	dev_kfree_skb_any(skb);
	return NULL;
}

static void
mvswitch_mangle_rx(struct net_device *dev, struct sk_buff *skb)
{
	struct mvswitch_priv *priv;
	unsigned char *buf;
	int vlan = -1;
	int i;

	priv = dev->phy_ptr;
	if (WARN_ON_ONCE(!priv))
		return;

#ifdef HEADER_MODE
	buf = skb->data;
	skb_pull(skb, MV_HEADER_SIZE);
#else
	buf = skb->data + skb->len - MV_TRAILER_SIZE;
	if (buf[0] != 0x80)
		return;
#endif

	/* look for the vlan matching the incoming port */
	for (i = 0; i < ARRAY_SIZE(priv->vlans); i++) {
		if ((1 << buf[1]) & priv->vlans[i])
			vlan = i;
	}

	if (vlan == -1)
		return;

	__vlan_hwaccel_put_tag(skb, htons(ETH_P_8021Q), vlan);
}


static int
mvswitch_wait_mask(struct phy_device *pdev, int addr, int reg, u16 mask, u16 val)
{
	int i = 100;
	u16 r;

	do {
		r = r16(pdev, addr, reg) & mask;
		if (r == val)
			return 0;
	} while(--i > 0);
	return -ETIMEDOUT;
}

static int
mvswitch_config_init(struct phy_device *pdev)
{
	struct mvswitch_priv *priv = to_mvsw(pdev);
	struct net_device *dev = pdev->attached_dev;
	u8 vlmap = 0;
	int i;

	if (!dev)
		return -EINVAL;

	printk("%s: Marvell 88E6060 PHY driver attached.\n", dev->name);
	pdev->supported = ADVERTISED_100baseT_Full;
	pdev->advertising = ADVERTISED_100baseT_Full;
	dev->phy_ptr = priv;
	pdev->irq = PHY_POLL;
#ifdef HEADER_MODE
	dev->flags |= IFF_PROMISC;
#endif

	/* initialize default vlans */
	for (i = 0; i < MV_PORTS; i++)
		priv->vlans[(i == MV_WANPORT ? 2 : 1)] |= (1 << i);

	/* before entering reset, disable all ports */
	for (i = 0; i < MV_PORTS; i++)
		w16(pdev, MV_PORTREG(CONTROL, i), 0x00);

	msleep(2); /* wait for the status change to settle in */

	/* put the ATU in reset */
	w16(pdev, MV_SWITCHREG(ATU_CTRL), MV_ATUCTL_RESET);

	i = mvswitch_wait_mask(pdev, MV_SWITCHREG(ATU_CTRL), MV_ATUCTL_RESET, 0);
	if (i < 0) {
		printk("%s: Timeout waiting for the switch to reset.\n", dev->name);
		return i;
	}

	/* set the ATU flags */
	w16(pdev, MV_SWITCHREG(ATU_CTRL),
		MV_ATUCTL_NO_LEARN |
		MV_ATUCTL_ATU_1K |
		MV_ATUCTL_AGETIME(MV_ATUCTL_AGETIME_MIN) /* minimum without disabling ageing */
	);

	/* initialize the cpu port */
	w16(pdev, MV_PORTREG(CONTROL, MV_CPUPORT),
#ifdef HEADER_MODE
		MV_PORTCTRL_HEADER |
#else
		MV_PORTCTRL_RXTR |
		MV_PORTCTRL_TXTR |
#endif
		MV_PORTCTRL_ENABLED
	);
	/* wait for the phy change to settle in */
	msleep(2);
	for (i = 0; i < MV_PORTS; i++) {
		u8 pvid = 0;
		int j;

		vlmap = 0;

		/* look for the matching vlan */
		for (j = 0; j < ARRAY_SIZE(priv->vlans); j++) {
			if (priv->vlans[j] & (1 << i)) {
				vlmap = priv->vlans[j];
				pvid = j;
			}
		}
		/* leave port unconfigured if it's not part of a vlan */
		if (!vlmap)
			continue;

		/* add the cpu port to the allowed destinations list */
		vlmap |= (1 << MV_CPUPORT);

		/* take port out of its own vlan destination map */
		vlmap &= ~(1 << i);

		/* apply vlan settings */
		w16(pdev, MV_PORTREG(VLANMAP, i),
			MV_PORTVLAN_PORTS(vlmap) |
			MV_PORTVLAN_ID(i)
		);

		/* re-enable port */
		w16(pdev, MV_PORTREG(CONTROL, i),
			MV_PORTCTRL_ENABLED
		);
	}

	w16(pdev, MV_PORTREG(VLANMAP, MV_CPUPORT),
		MV_PORTVLAN_ID(MV_CPUPORT)
	);

	/* set the port association vector */
	for (i = 0; i <= MV_PORTS; i++) {
		w16(pdev, MV_PORTREG(ASSOC, i),
			MV_PORTASSOC_PORTS(1 << i)
		);
	}

	/* init switch control */
	w16(pdev, MV_SWITCHREG(CTRL),
		MV_SWITCHCTL_MSIZE |
		MV_SWITCHCTL_DROP
	);

	dev->eth_mangle_rx = mvswitch_mangle_rx;
	dev->eth_mangle_tx = mvswitch_mangle_tx;
	priv->orig_features = dev->features;

#ifdef HEADER_MODE
	dev->priv_flags |= IFF_NO_IP_ALIGN;
	dev->features |= NETIF_F_HW_VLAN_CTAG_RX | NETIF_F_HW_VLAN_CTAG_TX;
#else
	dev->features |= NETIF_F_HW_VLAN_CTAG_RX;
#endif

	return 0;
}

static int
mvswitch_read_status(struct phy_device *pdev)
{
	pdev->speed = SPEED_100;
	pdev->duplex = DUPLEX_FULL;
	pdev->link = 1;

	/* XXX ugly workaround: we can't force the switch
	 * to gracefully handle hosts moving from one port to another,
	 * so we have to regularly clear the ATU database */

	/* wait for the ATU to become available */
	mvswitch_wait_mask(pdev, MV_SWITCHREG(ATU_OP), MV_ATUOP_INPROGRESS, 0);

	/* flush the ATU */
	w16(pdev, MV_SWITCHREG(ATU_OP),
		MV_ATUOP_INPROGRESS |
		MV_ATUOP_FLUSH_ALL
	);

	/* wait for operation to complete */
	mvswitch_wait_mask(pdev, MV_SWITCHREG(ATU_OP), MV_ATUOP_INPROGRESS, 0);

	return 0;
}

static int
mvswitch_aneg_done(struct phy_device *phydev)
{
	return 1;	/* Return any positive value */
}

static int
mvswitch_config_aneg(struct phy_device *phydev)
{
	return 0;
}

static void
mvswitch_detach(struct phy_device *pdev)
{
	struct mvswitch_priv *priv = to_mvsw(pdev);
	struct net_device *dev = pdev->attached_dev;

	if (!dev)
		return;

	dev->phy_ptr = NULL;
	dev->eth_mangle_rx = NULL;
	dev->eth_mangle_tx = NULL;
	dev->features = priv->orig_features;
	dev->priv_flags &= ~IFF_NO_IP_ALIGN;
}

static void
mvswitch_remove(struct phy_device *pdev)
{
	struct mvswitch_priv *priv = to_mvsw(pdev);

	kfree(priv);
}

static int
mvswitch_probe(struct phy_device *pdev)
{
	struct mvswitch_priv *priv;

	priv = kzalloc(sizeof(struct mvswitch_priv), GFP_KERNEL);
	if (priv == NULL)
		return -ENOMEM;

	pdev->priv = priv;

	return 0;
}

static int
mvswitch_fixup(struct phy_device *dev)
{
	struct mii_bus *bus = dev->mdio.bus;
	u16 reg;

	if (dev->mdio.addr != 0x10)
		return 0;

	reg = bus->read(bus, MV_PORTREG(IDENT, 0)) & MV_IDENT_MASK;
	if (reg != MV_IDENT_VALUE)
		return 0;

	dev->phy_id = MVSWITCH_MAGIC;
	return 0;
}


static struct phy_driver mvswitch_driver = {
	.name		= "Marvell 88E6060",
	.phy_id		= MVSWITCH_MAGIC,
	.phy_id_mask	= 0xffffffff,
	.features	= PHY_BASIC_FEATURES,
	.probe		= &mvswitch_probe,
	.remove		= &mvswitch_remove,
	.detach		= &mvswitch_detach,
	.config_init	= &mvswitch_config_init,
	.config_aneg	= &mvswitch_config_aneg,
	.aneg_done	= &mvswitch_aneg_done,
	.read_status	= &mvswitch_read_status,
};

static int __init
mvswitch_init(void)
{
	phy_register_fixup_for_id(PHY_ANY_ID, mvswitch_fixup);
	return phy_driver_register(&mvswitch_driver, THIS_MODULE);
}

static void __exit
mvswitch_exit(void)
{
	phy_driver_unregister(&mvswitch_driver);
}

module_init(mvswitch_init);
module_exit(mvswitch_exit);
