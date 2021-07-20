// SPDX-License-Identifier: GPL-2.0-or-later

/*
 * This driver is adapted from the Linux /drivers/net/ifb.c file.
 *
 * Redirect QCA NSS physical interface ingress traffic to this driver's
 * virtual interface.  This will allow ingress traffic shaping using the
 * QCA NSS shaper.
 */

#include <nss_api_if.h>

#define TX_Q_LIMIT    32

struct nss_ifb_dev_private {
	struct nss_virt_if_handle *nssctx;
	struct net_device *nss_src_dev;
	uint32_t nss_src_if_num;
	char nss_src_dev_name[32];
};

char nss_dev_name_array[32] = "eth0";
char *nss_dev_name = nss_dev_name_array;
module_param(nss_dev_name, charp, 0644);
MODULE_PARM_DESC(nss_dev_name, "NSS physical interface source device name");

/*
 * Virtual interface egress packet callback.
 *
 * We send it back to the Linux network stack.
 */
static void nss_ifb_data_cb(struct net_device *netdev, struct sk_buff *skb, struct napi_struct *napi)
{
	struct nss_ifb_dev_private *dp = netdev_priv(netdev);

	skb->protocol = eth_type_trans(skb, dp->nss_src_dev);
	skb->ip_summed = CHECKSUM_UNNECESSARY;

	napi_gro_receive(napi, skb);
}

/*
 * Virtual interface ingress packet callback.
 *
 * We just send it back to the NSS firmware to let the shaper work on it.
 */
static void nss_ifb_xmit_cb(struct net_device *netdev, struct sk_buff *skb)
{
	struct nss_ifb_dev_private *dp = netdev_priv(netdev);
	int ret;

	ret = nss_virt_if_tx_buf(dp->nssctx, skb);
	if (unlikely(ret)) {
		pr_warn("Failed [%d] to send skb [len: %d, protocol: 0x%X] to NSS!\n",
			ret, skb->len, ntohs(skb->protocol));
	}
}

static void nss_ifb_stats64(struct net_device *dev,
			struct rtnl_link_stats64 *stats)
{

}

static int nss_ifb_dev_init(struct net_device *dev)
{
	struct nss_ifb_dev_private *dp =  netdev_priv(dev);

	dp->nssctx = nss_virt_if_create_sync_nexthop(dev, NSS_ETH_RX_INTERFACE, NSS_ETH_RX_INTERFACE);
	if (!dp->nssctx) {
		dp->nssctx = NULL;
		pr_warn("Could not create a NSS virtual interface for dev [%s]\n",
			dev->name);

		return -ENODEV;
	}
	pr_info("Created a NSS virtual interface for dev [%s]\n", dev->name);

	nss_virt_if_register(dp->nssctx, nss_ifb_data_cb, dev);
	pr_info("NSS IFB data callback registered\n");

	nss_virt_if_xmit_callback_register(dp->nssctx, nss_ifb_xmit_cb);
	pr_info("NSS IFB transmit callback registered\n");

	return 0;
}

static void nss_ifb_dev_uninit(struct net_device *dev)
{
	struct nss_ifb_dev_private *dp = netdev_priv(dev);
	int ret;

	nss_virt_if_xmit_callback_unregister(dp->nssctx);
	pr_info("NSS IFB transmit callback unregistered\n");

	ret = nss_virt_if_destroy_sync(dp->nssctx);
	if (ret == NSS_TX_SUCCESS) {
		pr_info("NSS virtual interface destroyed for dev [%s]\n", dev->name);
	}
	else {
		pr_warn("Unable to destroy NSS virtual interface for dev [%s], error[%d]\n",
		dev->name, ret);
	}
	dp->nssctx = NULL;
}

static netdev_tx_t nss_ifb_xmit(struct sk_buff *skb, struct net_device *dev)
{
	return NETDEV_TX_OK;
}

static int nss_ifb_close(struct net_device *dev)
{
	struct nss_ifb_dev_private *dp = netdev_priv(dev);
	struct nss_ctx_instance *nss_ctx;
	struct net_device *src_dev;
	uint32_t src_if_num;
	int ret;

	nss_ctx = dp->nssctx->nss_ctx;
	src_dev = dp->nss_src_dev;
	src_if_num = dp->nss_src_if_num;

	ret = nss_phys_if_set_nexthop(nss_ctx, src_if_num, NSS_ETH_RX_INTERFACE);
	if (ret != NSS_TX_SUCCESS) {
		pr_warn("%p: Failed to reset next hop for net device [%s].\n",
			nss_ctx, src_dev->name);
	}
	else {
		pr_info("%p: Reset nexthop successful for net device [%s].\n",
			nss_ctx, src_dev->name);
	}

	dev_put(src_dev);
	dp->nss_src_dev = NULL;
	dp->nss_src_if_num = -1;

	return 0;
}

static int nss_ifb_open(struct net_device *dev)
{
	struct nss_ifb_dev_private *dp = netdev_priv(dev);
	struct net_device *src_dev;
	uint32_t src_if_num;
	uint32_t nh_if_num;
	nss_tx_status_t nss_tx_status;
	struct nss_ctx_instance *nss_ctx;

	nss_ctx = dp->nssctx->nss_ctx;
	nh_if_num = dp->nssctx->if_num_n2h;

	strcpy(dp->nss_src_dev_name, nss_dev_name);

	src_dev = dev_get_by_name(&init_net, dp->nss_src_dev_name);
	if (!src_dev) {
		pr_warn("%p: Cannot find the net device [%s]\n",
			nss_ctx, dp->nss_src_dev_name);

		return -ENODEV;
	}
	pr_info("%p: Found net device [%s]\n", nss_ctx, dp->nss_src_dev_name);

	src_if_num = nss_cmn_get_interface_number_by_dev(src_dev);
	if (src_if_num < 0) {
		pr_warn("%p: Invalid interface number:%d\n", nss_ctx, src_if_num);
		dev_put(src_dev);

		return -ENODEV;
	}
	pr_info("%p: Net device [%s] has NSS intf_num [%d]\n",
		nss_ctx, dp->nss_src_dev_name, src_if_num);

	nss_tx_status = nss_phys_if_set_nexthop(nss_ctx, src_if_num, nh_if_num);
	if (nss_tx_status != NSS_TX_SUCCESS) {
		pr_warn("%p: Sending message failed, cannot change nexthop for [%s]\n",
			nss_ctx, dp->nss_src_dev_name);
	}
	else {
		pr_info("Nexthop successfully set for [%s] to [%s]\n",
			dp->nss_src_dev_name, dev->name);
	}

	dp->nss_src_dev = src_dev;
	dp->nss_src_if_num = src_if_num;

	return 0;
}

static const struct net_device_ops nss_ifb_netdev_ops = {
	.ndo_open	= nss_ifb_open,
	.ndo_stop	= nss_ifb_close,
	.ndo_get_stats64 = nss_ifb_stats64,
	.ndo_start_xmit	= nss_ifb_xmit,
	.ndo_validate_addr = eth_validate_addr,
	.ndo_init	= nss_ifb_dev_init,
	.ndo_uninit	= nss_ifb_dev_uninit,
};

#define IFB_FEATURES (NETIF_F_HW_CSUM | NETIF_F_SG  | NETIF_F_FRAGLIST	| \
		      NETIF_F_TSO_ECN | NETIF_F_TSO | NETIF_F_TSO6	| \
		      NETIF_F_GSO_ENCAP_ALL 				| \
		      NETIF_F_HIGHDMA | NETIF_F_HW_VLAN_CTAG_TX		| \
		      NETIF_F_HW_VLAN_STAG_TX)

static void nss_ifb_dev_free(struct net_device *dev)
{

}

static void nss_ifb_setup(struct net_device *dev)
{
	/* Initialize the device structure. */
	dev->netdev_ops = &nss_ifb_netdev_ops;

	/* Fill in device structure with ethernet-generic values. */
	ether_setup(dev);
	dev->tx_queue_len = TX_Q_LIMIT;

	dev->features |= IFB_FEATURES;
	dev->hw_features |= dev->features;
	dev->hw_enc_features |= dev->features;
	dev->vlan_features |= IFB_FEATURES & ~(NETIF_F_HW_VLAN_CTAG_TX |
					       NETIF_F_HW_VLAN_STAG_TX);

	dev->flags |= IFF_NOARP;
	dev->flags &= ~IFF_MULTICAST;
	dev->priv_flags &= ~IFF_TX_SKB_SHARING;
	netif_keep_dst(dev);
	eth_hw_addr_random(dev);
	dev->needs_free_netdev = true;
	dev->priv_destructor = nss_ifb_dev_free;

	dev->min_mtu = 0;
	dev->max_mtu = 0;
}

static int nss_ifb_validate(struct nlattr *tb[], struct nlattr *data[],
			struct netlink_ext_ack *extack)
{
	if (tb[IFLA_ADDRESS]) {
		if (nla_len(tb[IFLA_ADDRESS]) != ETH_ALEN)
			return -EINVAL;
		if (!is_valid_ether_addr(nla_data(tb[IFLA_ADDRESS])))
			return -EADDRNOTAVAIL;
	}
	return 0;
}

static struct rtnl_link_ops nss_ifb_link_ops __read_mostly = {
	.kind		= "nss_ifb",
	.priv_size	= sizeof(struct nss_ifb_dev_private),
	.setup		= nss_ifb_setup,
	.validate	= nss_ifb_validate,
};

static int __init nss_ifb_init_module(void)
{
	struct net_device *dev;
	int err;

	down_write(&pernet_ops_rwsem);
	rtnl_lock();
	err = __rtnl_link_register(&nss_ifb_link_ops);
	if (err < 0)
		goto out;

	dev = alloc_netdev(sizeof(struct nss_ifb_dev_private), "nssifb",
			NET_NAME_UNKNOWN, nss_ifb_setup);

	if (dev) {
		dev->rtnl_link_ops = &nss_ifb_link_ops;
		err = register_netdevice(dev);
	}
	else {
		err = -ENOMEM;
	}

	if (err)
		__rtnl_link_unregister(&nss_ifb_link_ops);

out:
	rtnl_unlock();
	up_write(&pernet_ops_rwsem);

	if (!err)
		pr_info("NSS IFB module loaded.\n");
	else
		pr_warn("Failed to load NSS IFB module.\n");

	return err;
}

static void __exit nss_ifb_cleanup_module(void)
{
	rtnl_link_unregister(&nss_ifb_link_ops);

	pr_info("NSS IFB module unloaded.\n");
}

module_init(nss_ifb_init_module);
module_exit(nss_ifb_cleanup_module);
MODULE_LICENSE("GPL");
MODULE_ALIAS_RTNL_LINK("nss_ifb");
