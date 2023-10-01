/* Copyright (c) 2013-2019, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * RMNET configuration engine
 *
 */

#include <net/sock.h>
#include <linux/module.h>
#include <linux/netlink.h>
#include <linux/netdevice.h>
#include <linux/hashtable.h>
#include "rmnet_config.h"
#include "rmnet_handlers.h"
#include "rmnet_vnd.h"
#include "rmnet_private.h"
#include "rmnet_map.h"
#include "rmnet_descriptor.h"

/* Locking scheme -
 * The shared resource which needs to be protected is realdev->rx_handler_data.
 * For the writer path, this is using rtnl_lock(). The writer paths are
 * rmnet_newlink(), rmnet_dellink() and rmnet_force_unassociate_device(). These
 * paths are already called with rtnl_lock() acquired in. There is also an
 * ASSERT_RTNL() to ensure that we are calling with rtnl acquired. For
 * dereference here, we will need to use rtnl_dereference(). Dev list writing
 * needs to happen with rtnl_lock() acquired for netdev_master_upper_dev_link().
 * For the reader path, the real_dev->rx_handler_data is called in the TX / RX
 * path. We only need rcu_read_lock() for these scenarios. In these cases,
 * the rcu_read_lock() is held in __dev_queue_xmit() and
 * netif_receive_skb_internal(), so readers need to use rcu_dereference_rtnl()
 * to get the relevant information. For dev list reading, we again acquire
 * rcu_read_lock() in rmnet_dellink() for netdev_master_upper_dev_get_rcu().
 * We also use unregister_netdevice_many() to free all rmnet devices in
 * rmnet_force_unassociate_device() so we dont lose the rtnl_lock() and free in
 * same context.
 */

/* Local Definitions and Declarations */

static int rmnet_is_real_dev_registered(const struct net_device *real_dev)
{
	return rcu_access_pointer(real_dev->rx_handler) == rmnet_rx_handler;
}

/* Needs rtnl lock */
static struct rmnet_port*
rmnet_get_port_rtnl(const struct net_device *real_dev)
{
	return rtnl_dereference(real_dev->rx_handler_data);
}

static int rmnet_unregister_real_device(struct net_device *real_dev,
					struct rmnet_port *port)
{
	if (port->nr_rmnet_devs)
		return -EINVAL;

	rmnet_map_cmd_exit(port);
	rmnet_map_tx_aggregate_exit(port);

	rmnet_descriptor_deinit(port);

	kfree(port);

	netdev_rx_handler_unregister(real_dev);

	/* release reference on real_dev */
	dev_put(real_dev);

	netdev_dbg(real_dev, "Removed from rmnet\n");
	return 0;
}

static int rmnet_register_real_device(struct net_device *real_dev)
{
	struct rmnet_port *port;
	int rc, entry;

	ASSERT_RTNL();

	if (rmnet_is_real_dev_registered(real_dev))
		return 0;

	port = kzalloc(sizeof(*port), GFP_ATOMIC);
	if (!port)
		return -ENOMEM;

	port->dev = real_dev;
	rc = netdev_rx_handler_register(real_dev, rmnet_rx_handler, port);
	if (rc) {
		kfree(port);
		return -EBUSY;
	}
	/* hold on to real dev for MAP data */
	dev_hold(real_dev);

	for (entry = 0; entry < RMNET_MAX_LOGICAL_EP; entry++)
		INIT_HLIST_HEAD(&port->muxed_ep[entry]);

	rc = rmnet_descriptor_init(port);
	if (rc) {
		rmnet_descriptor_deinit(port);
		return rc;
	}

	rmnet_map_tx_aggregate_init(port);
	rmnet_map_cmd_init(port);

	netdev_dbg(real_dev, "registered with rmnet\n");
	return 0;
}

/* Needs either rcu_read_lock() or rtnl lock */
static struct rmnet_port *rmnet_get_port(struct net_device *real_dev)
{
	if (rmnet_is_real_dev_registered(real_dev))
		return rcu_dereference_rtnl(real_dev->rx_handler_data);
	else
		return NULL;
}

static struct rmnet_endpoint *rmnet_get_endpoint(struct rmnet_port *port, u8 mux_id)
{
	struct rmnet_endpoint *ep;

	hlist_for_each_entry_rcu(ep, &port->muxed_ep[mux_id], hlnode) {
		if (ep->mux_id == mux_id)
			return ep;
	}

	return NULL;
}
