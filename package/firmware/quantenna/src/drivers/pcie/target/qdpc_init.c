/**
 * Copyright (c) 2011-2013 Quantenna Communications, Inc.
 * All rights reserved.
 *
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 **/
#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/spinlock.h>
#include <linux/version.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/kthread.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <net/sock.h>
#include <linux/netlink.h>
#include <asm/board/board_config.h>
#include <common/topaz_platform.h>

#include "qdpc_config.h"
#include "qdpc_debug.h"
#include "qdpc_init.h"

MODULE_AUTHOR("Quantenna");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Virtual ethernet driver for Quantenna Ruby device");
MODULE_VERSION(QDPC_MODULE_VERSION);

char *qdpc_mac0addr = NULL;
module_param(qdpc_mac0addr, charp, 0);
uint8_t qdpc_basemac[ETH_ALEN] = {'\0', 'R', 'U', 'B', 'Y', '%'};

static int qdpc_parse_mac(const char *mac_str, uint8_t *mac)
{
	unsigned int tmparray[ETH_ALEN];

	if (mac_str == NULL)
		return -1;

	if (sscanf(mac_str, "%02x:%02x:%02x:%02x:%02x:%02x",
			&tmparray[0],
			&tmparray[1],
			&tmparray[2],
			&tmparray[3],
			&tmparray[4],
			&tmparray[5]) != ETH_ALEN) {
		return -1;
	}

	mac[0] = tmparray[0];
	mac[1] = tmparray[1];
	mac[2] = tmparray[2];
	mac[3] = tmparray[3];
	mac[4] = tmparray[4];
	mac[5] = tmparray[5];

	return 0;
}

static void qdpc_nl_recv_msg(struct sk_buff *skb);
int qdpc_init_netdev(struct net_device **net_dev);

static void qdpc_get_config(struct  qdpc_priv *priv)
{
	int cfg_var = 0;
	qdpc_config_t *ep_config = &priv->ep_config;
	memset (ep_config, 0, sizeof(qdpc_config_t));
	ep_config->cf_usdma = 1;
	ep_config->cf_dsdma = 0;
	ep_config->cf_msi = 1;
	ep_config->cf_64bit = 0;
	ep_config->cf_usdma_ndesc = QDPC_DESC_RING_SIZE;
	ep_config->cf_dsdma_ndesc = QDPC_DESC_DSRING_SIZE;

	get_board_config(BOARD_CFG_PCIE, &cfg_var);
	if ((cfg_var & PCIE_USE_PHY_LOOPBK) == PCIE_USE_PHY_LOOPBK)
	{
	ep_config->cf_miipllclk = 0x8f8f8f8f;
     } else {
#ifdef CVM_HOST
		ep_config->cf_miipllclk = 0x8f808f8f;
#else
		ep_config->cf_miipllclk = 0x8f808f80;
#endif
        }

}
/* Global net device pointer */
struct  net_device  *qdpc_ndev  = NULL;

int  __init qdpc_init_module(void)
{
	struct  qdpc_priv	*priv;
	struct  net_device  *ndev = NULL;
	int ret  = SUCCESS;

	if (qdpc_mac0addr != NULL) {
		qdpc_parse_mac(qdpc_mac0addr, qdpc_basemac);
	}

	/* Creating net device */
	ret = qdpc_init_netdev(&ndev);
	if (ret) {
		return ret;
	}

	qdpc_ndev = ndev;
	priv = netdev_priv(ndev);

	/* ep2h isr lock */
	spin_lock_init(&priv->lock);

	/* Get board configuration */
	qdpc_get_config(priv);

	/* Initialize INIT WORK */
	INIT_WORK(&priv->init_work, qdpc_init_work);

	/* Create netlink */
	priv->netlink_socket = netlink_kernel_create(&init_net, QDPC_NETLINK_RPC_PCI, 0, qdpc_nl_recv_msg,
			NULL, THIS_MODULE);
	if (!priv->netlink_socket) {
		PRINT_ERROR(KERN_ALERT "Error creating netlink socket.\n");
		ret = FAILURE;
		goto error;
	}

	/* Initialize Interrupts */
    ret = qdpc_pcie_init_intr(ndev);
	if (ret < 0) {
		PRINT_ERROR("Interrupt Initialization failed \n");
		goto out;
	}

	schedule_work(&priv->init_work);

	return ret;

out:
	/* Release netlink */
	netlink_kernel_release(priv->netlink_socket);

error:
	/* Free netdevice */
	unregister_netdev(ndev);
	free_netdev(ndev);

	return ret;
}

static void __exit qdpc_exit_module(void)
{
	struct qdpc_priv *priv = NULL;

	if (qdpc_ndev) {
		priv = netdev_priv(qdpc_ndev);

		/* release netlink socket */
		netlink_kernel_release(priv->netlink_socket);

		/* release scheduled work */
		cancel_work_sync(&priv->init_work);

		/* Free intettupt line */
		qdpc_free_interrupt(qdpc_ndev);

		/* Free net device */
		unregister_netdev(qdpc_ndev);
		free_netdev(qdpc_ndev);

		qdpc_ndev = NULL;
	}

	return;
}

static void qdpc_nl_recv_msg(struct sk_buff *skb)
{
	struct qdpc_priv *priv = netdev_priv(qdpc_ndev);
	struct nlmsghdr *nlh  = (struct nlmsghdr*)skb->data;
	struct sk_buff *skb2;
	/* Parsing the netlink message */

	PRINT_DBG(KERN_INFO "%s line %d Netlink received pid:%d, size:%d, type:%d\n",
		__FUNCTION__, __LINE__, nlh->nlmsg_pid, nlh->nlmsg_len, nlh->nlmsg_type);

	switch (nlh->nlmsg_type) {
		case QDPC_NETLINK_TYPE_SVC_REGISTER:
			priv->netlink_pid = nlh->nlmsg_pid;/*pid of sending process */
			return;

		case QDPC_NETLINK_TYPE_SVC_RESPONSE:
			break;

		default:
			PRINT_DBG(KERN_INFO "%s line %d Netlink Invalid type %d\n",
				__FUNCTION__, __LINE__, nlh->nlmsg_type);
			return;
	}

	/*
	 * make a new skb. The old skb will freed in netlink_unicast_kernel,
	 * but we must hold the skb before DMA transfer done
	 */
	skb2 = alloc_skb(nlh->nlmsg_len+sizeof(qdpc_cmd_hdr_t), GFP_ATOMIC);
	if (skb2) {
		qdpc_cmd_hdr_t *cmd_hdr;
		cmd_hdr = (qdpc_cmd_hdr_t *)skb2->data;
		memcpy(cmd_hdr->dst_magic, QDPC_NETLINK_DST_MAGIC, ETH_ALEN);
		memcpy(cmd_hdr->src_magic, QDPC_NETLINK_SRC_MAGIC, ETH_ALEN);
		cmd_hdr->type = htons(QDPC_APP_NETLINK_TYPE);
		cmd_hdr->len = htons(nlh->nlmsg_len);
		memcpy(skb2->data+sizeof(qdpc_cmd_hdr_t), skb->data+sizeof(struct nlmsghdr), nlh->nlmsg_len);

		skb_put(skb2, nlh->nlmsg_len+sizeof(qdpc_cmd_hdr_t));
		qdpc_send_packet(skb2, qdpc_ndev);
	}
}

module_init(qdpc_init_module);
module_exit(qdpc_exit_module);
