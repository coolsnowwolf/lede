/* SPDX-License-Identifier: GPL-2.0 */
/* Phytium CAN controller driver
 *
 * Copyright (c) 2021-2023 Phytium Technology Co., Ltd.
 */

#ifndef _PHYTIUM_CAN_H_
#define _PHYTIUM_CAN_H_

#include <linux/can/core.h>
#include <linux/can/dev.h>

#include <linux/device.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/pm_runtime.h>
#include <linux/iopoll.h>

#define KEEP_CAN_FIFO_MIN_LEN 16
#define KEEP_CANFD_FIFO_MIN_LEN	128
#define CAN_FIFO_BYTE_LEN 256
#define STOP_QUEUE_TRUE		1
#define STOP_QUEUE_FALSE	0

enum phytium_can_ip_type {
	PHYTIUM_CAN = 0,
	PHYTIUM_CANFD,
};

struct phytium_can_devtype {
	enum phytium_can_ip_type cantype;
	const struct can_bittiming_const *bittiming_const;
};

struct phytium_can_dev {
	struct can_priv can;
	unsigned int tx_head;
	unsigned int tx_tail;
	unsigned int tx_max;
	struct napi_struct napi;
	struct net_device *net;
	struct device *dev;
	struct clk *clk;

	struct sk_buff *tx_skb;

	const struct can_bittiming_const *bit_timing;
	spinlock_t lock;
	int fdmode;
	u32 isr;
	u32 tx_fifo_depth;
	unsigned int is_stop_queue_flag;
	struct completion comp;
	void __iomem *base;
};

struct phytium_can_dev *phytium_can_allocate_dev(struct device *dev, int sizeof_priv,
						 int tx_fifo_depth);
void phytium_can_free_dev(struct net_device *net);

int phytium_can_register(struct phytium_can_dev *cdev);
void phytium_can_unregister(struct phytium_can_dev *cdev);

int phytium_can_suspend(struct device *dev);
int phytium_can_resume(struct device *dev);
#endif /* _PHYTIUM_CAN_H_ */
