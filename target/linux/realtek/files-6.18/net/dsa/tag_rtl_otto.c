// SPDX-License-Identifier: GPL-2.0+

#include <linux/etherdevice.h>
#include <linux/list.h>
#include <linux/slab.h>

#include "tag.h"

#define RTL_OTTO_NAME		"rtl_otto"
#define RTL_OTTO_TAILROOM	4

static struct sk_buff *rtl_otto_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct dsa_port *dp = dsa_user_to_port(dev);
	u8 *trailer;

	/* Hardware needs space for Layer 2 FCS. Align tag size with that. */
	trailer = skb_put(skb, RTL_OTTO_TAILROOM);
	trailer[0] = dp->index;
	trailer[1] = 0xab;
	trailer[2] = 0xcd;
	trailer[3] = 0xef;

	return skb;
}

static struct sk_buff *rtl_otto_rcv(struct sk_buff *skb, struct net_device *dev)
{
	/* RX path uses METADATA_HW_PORT_MUX. This function just makes netdev_uses_dsa() happy. */
	netdev_err(dev, "ethernet driver did not set METADATA\n");

	return skb;
}

static const struct dsa_device_ops rtl_otto_netdev_ops = {
	.name			= RTL_OTTO_NAME,
	.proto			= DSA_TAG_PROTO_RTL_OTTO,
	.xmit			= rtl_otto_xmit,
	.rcv			= rtl_otto_rcv,
	.needed_tailroom	= RTL_OTTO_TAILROOM,
};

MODULE_DESCRIPTION("DSA tag driver for Realtek Otto switches (RTL83xx/RTL93xx)");
MODULE_LICENSE("GPL");
MODULE_ALIAS_DSA_TAG_DRIVER(DSA_TAG_PROTO_RTL_OTTO, RTL_OTTO_NAME);

module_dsa_tag_driver(rtl_otto_netdev_ops);
