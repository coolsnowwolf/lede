/*
 * trelay.c: Trivial Ethernet Relay
 *
 * Copyright (C) 2012 Felix Fietkau <nbd@nbd.name>
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
 */
#include <linux/module.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/netdevice.h>
#include <linux/rtnetlink.h>
#include <linux/debugfs.h>

static LIST_HEAD(trelay_devs);
static struct dentry *debugfs_dir;

struct trelay {
	struct list_head list;
	struct net_device *dev1, *dev2;
	struct dentry *debugfs;
	char name[];
};

rx_handler_result_t trelay_handle_frame(struct sk_buff **pskb)
{
	struct net_device *dev;
	struct sk_buff *skb = *pskb;

	dev = rcu_dereference(skb->dev->rx_handler_data);
	if (!dev)
		return RX_HANDLER_PASS;

	if (skb->protocol == htons(ETH_P_PAE))
		return RX_HANDLER_PASS;

	skb_push(skb, ETH_HLEN);
	skb->dev = dev;
	skb_forward_csum(skb);
	dev_queue_xmit(skb);

	return RX_HANDLER_CONSUMED;
}

static int trelay_open(struct inode *inode, struct file *file)
{
	file->private_data = inode->i_private;
	return 0;
}

static int trelay_do_remove(struct trelay *tr)
{
	list_del(&tr->list);

	dev_put(tr->dev1);
	dev_put(tr->dev2);

	netdev_rx_handler_unregister(tr->dev1);
	netdev_rx_handler_unregister(tr->dev2);

	debugfs_remove_recursive(tr->debugfs);
	kfree(tr);

	return 0;
}

static struct trelay *trelay_find(struct net_device *dev)
{
	struct trelay *tr;

	list_for_each_entry(tr, &trelay_devs, list) {
		if (tr->dev1 == dev || tr->dev2 == dev)
			return tr;
	}
	return NULL;
}

static int tr_device_event(struct notifier_block *unused, unsigned long event,
			   void *ptr)
{
	struct net_device *dev = ptr;
	struct trelay *tr;

	if (event != NETDEV_UNREGISTER)
		goto out;

	tr = trelay_find(dev);
	if (!tr)
		goto out;

	trelay_do_remove(tr);

out:
	return NOTIFY_DONE;
}

static ssize_t trelay_remove_write(struct file *file, const char __user *ubuf,
				   size_t count, loff_t *ppos)
{
	struct trelay *tr = file->private_data;
	int ret;

	rtnl_lock();
	ret = trelay_do_remove(tr);
	rtnl_unlock();

	if (ret < 0)
		 return ret;

	return count;
}

static const struct file_operations fops_remove = {
	.owner = THIS_MODULE,
	.open = trelay_open,
	.write = trelay_remove_write,
	.llseek = default_llseek,
};


static int trelay_do_add(char *name, char *devn1, char *devn2)
{
	struct net_device *dev1, *dev2;
	struct trelay *tr, *tr1;
	int ret;

	tr = kzalloc(sizeof(*tr) + strlen(name) + 1, GFP_KERNEL);
	if (!tr)
		return -ENOMEM;

	rtnl_lock();
	rcu_read_lock();

	ret = -EEXIST;
	list_for_each_entry(tr1, &trelay_devs, list) {
		if (!strcmp(tr1->name, name))
			goto out;
	}

	ret = -ENOENT;
	dev1 = dev_get_by_name_rcu(&init_net, devn1);
	dev2 = dev_get_by_name_rcu(&init_net, devn2);
	if (!dev1 || !dev2)
		goto out;

	ret = netdev_rx_handler_register(dev1, trelay_handle_frame, dev2);
	if (ret < 0)
		goto out;

	ret = netdev_rx_handler_register(dev2, trelay_handle_frame, dev1);
	if (ret < 0) {
		netdev_rx_handler_unregister(dev1);
		goto out;
	}

	dev_hold(dev1);
	dev_hold(dev2);

	strcpy(tr->name, name);
	tr->dev1 = dev1;
	tr->dev2 = dev2;
	list_add_tail(&tr->list, &trelay_devs);

	tr->debugfs = debugfs_create_dir(name, debugfs_dir);
	debugfs_create_file("remove", S_IWUSR, tr->debugfs, tr, &fops_remove);
	ret = 0;

out:
	rcu_read_unlock();
	rtnl_unlock();
	if (ret < 0)
		kfree(tr);

	return ret;
}

static ssize_t trelay_add_write(struct file *file, const char __user *ubuf,
				size_t count, loff_t *ppos)
{
	char buf[256];
	char *dev1, *dev2, *tmp;
	ssize_t len, ret;

	len = min(count, sizeof(buf) - 1);
	if (copy_from_user(buf, ubuf, len))
		return -EFAULT;

	buf[len] = 0;

	if ((tmp = strchr(buf, '\n')))
		*tmp = 0;

	dev1 = strchr(buf, ',');
	if (!dev1)
		return -EINVAL;

	*(dev1++) = 0;

	dev2 = strchr(dev1, ',');
	if (!dev2)
		return -EINVAL;

	*(dev2++) = 0;
	if (strchr(dev2, ','))
		return -EINVAL;

	if (!strlen(buf) || !strlen(dev1) || !strlen(dev2))
		return -EINVAL;

	ret = trelay_do_add(buf, dev1, dev2);
	if (ret < 0)
		return ret;

	return count;
}

static const struct file_operations fops_add = {
	.owner = THIS_MODULE,
	.write = trelay_add_write,
	.llseek = default_llseek,
};

static struct notifier_block tr_dev_notifier = {
	.notifier_call = tr_device_event
};

static int __init trelay_init(void)
{
	int ret;

	debugfs_dir = debugfs_create_dir("trelay", NULL);
	if (!debugfs_dir)
		return -ENOMEM;

	debugfs_create_file("add", S_IWUSR, debugfs_dir, NULL, &fops_add);

	ret = register_netdevice_notifier(&tr_dev_notifier);
	if (ret < 0)
		goto error;

	return 0;

error:
	debugfs_remove_recursive(debugfs_dir);
	return ret;
}

static void __exit trelay_exit(void)
{
	struct trelay *tr, *tmp;

	unregister_netdevice_notifier(&tr_dev_notifier);

	rtnl_lock();
	list_for_each_entry_safe(tr, tmp, &trelay_devs, list)
		trelay_do_remove(tr);
	rtnl_unlock();

	debugfs_remove_recursive(debugfs_dir);
}

module_init(trelay_init);
module_exit(trelay_exit);
MODULE_LICENSE("GPL");
