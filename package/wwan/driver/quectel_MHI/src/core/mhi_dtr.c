// SPDX-License-Identifier: GPL-2.0-only
/* Copyright (c) 2018-2019, The Linux Foundation. All rights reserved.*/

#include <linux/debugfs.h>
#include <linux/device.h>
#include <linux/dma-direction.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/of.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/termios.h>
#include <linux/wait.h>
#include "mhi.h"
#include "mhi_internal.h"

struct __packed dtr_ctrl_msg {
	u32 preamble;
	u32 msg_id;
	u32 dest_id;
	u32 size;
	u32 msg;
};

#define CTRL_MAGIC (0x4C525443)
#define CTRL_MSG_DTR BIT(0)
#define CTRL_MSG_RTS BIT(1)
#define CTRL_MSG_DCD BIT(0)
#define CTRL_MSG_DSR BIT(1)
#define CTRL_MSG_RI BIT(3)
#define CTRL_HOST_STATE (0x10)
#define CTRL_DEVICE_STATE (0x11)
#define CTRL_GET_CHID(dtr) (dtr->dest_id & 0xFF)

static int mhi_dtr_tiocmset(struct mhi_controller *mhi_cntrl,
			    struct mhi_device *mhi_dev,
			    u32 tiocm)
{
	struct dtr_ctrl_msg *dtr_msg = NULL;
	struct mhi_chan *dtr_chan = mhi_cntrl->dtr_dev->ul_chan;
	spinlock_t *res_lock = &mhi_dev->dev.devres_lock;
	u32 cur_tiocm;
	int ret = 0;

	cur_tiocm = mhi_dev->tiocm & ~(TIOCM_CD | TIOCM_DSR | TIOCM_RI);

	tiocm &= (TIOCM_DTR | TIOCM_RTS);

	/* state did not changed */
	if (cur_tiocm == tiocm)
		return 0;

	mutex_lock(&dtr_chan->mutex);

	dtr_msg = kzalloc(sizeof(*dtr_msg), GFP_KERNEL);
	if (!dtr_msg) {
		ret = -ENOMEM;
		goto tiocm_exit;
	}

	dtr_msg->preamble = CTRL_MAGIC;
	dtr_msg->msg_id = CTRL_HOST_STATE;
	dtr_msg->dest_id = mhi_dev->ul_chan_id;
	dtr_msg->size = sizeof(u32);
	if (tiocm & TIOCM_DTR)
		dtr_msg->msg |= CTRL_MSG_DTR;
	if (tiocm & TIOCM_RTS)
		dtr_msg->msg |= CTRL_MSG_RTS;

/*
* 'minicom -D /dev/mhi_DUN' will send RTS:1 when open, and RTS:0 when exit.
* RTS:0 will prevent modem output AT response.
* But 'busybox microcom' do not send any RTS to modem.
* [75094.969783] mhi_uci_q 0306_00.03.00_DUN: mhi_dtr_tiocmset DTR:0 RTS:1
* [75100.210994] mhi_uci_q 0306_00.03.00_DUN: mhi_dtr_tiocmset DTR:0 RTS:0
*/
	dev_dbg(&mhi_dev->dev, "%s DTR:%d RTS:%d\n", __func__,
				!!(tiocm & TIOCM_DTR), !!(tiocm & TIOCM_RTS));

	reinit_completion(&dtr_chan->completion);
	ret = mhi_queue_transfer(mhi_cntrl->dtr_dev, DMA_TO_DEVICE, dtr_msg,
				 sizeof(*dtr_msg), MHI_EOT);
	if (ret)
		goto tiocm_exit;

	ret = wait_for_completion_timeout(&dtr_chan->completion,
				msecs_to_jiffies(mhi_cntrl->timeout_ms));
	if (!ret) {
		MHI_ERR("Failed to receive transfer callback\n");
		ret = -EIO;
		goto tiocm_exit;
	}

	ret = 0;
	spin_lock_irq(res_lock);
	mhi_dev->tiocm &= ~(TIOCM_DTR | TIOCM_RTS);
	mhi_dev->tiocm |= tiocm;
	spin_unlock_irq(res_lock);

tiocm_exit:
	kfree(dtr_msg);
	mutex_unlock(&dtr_chan->mutex);

	return ret;
}

long mhi_ioctl(struct mhi_device *mhi_dev, unsigned int cmd, unsigned long arg)
{
	struct mhi_controller *mhi_cntrl = mhi_dev->mhi_cntrl;
	int ret;

	/* ioctl not supported by this controller */
	if (!mhi_cntrl->dtr_dev)
		return -EIO;

	switch (cmd) {
	case TIOCMGET:
		return mhi_dev->tiocm;
	case TIOCMSET:
	{
		u32 tiocm;

		ret = get_user(tiocm, (u32 *)arg);
		if (ret)
			return ret;

		return mhi_dtr_tiocmset(mhi_cntrl, mhi_dev, tiocm);
	}
	default:
		break;
	}

	return -EINVAL;
}
EXPORT_SYMBOL(mhi_ioctl);

static int mhi_dtr_queue_inbound(struct mhi_controller *mhi_cntrl)
{
	struct mhi_device *mhi_dev = mhi_cntrl->dtr_dev;
	int nr_trbs = mhi_get_no_free_descriptors(mhi_dev, DMA_FROM_DEVICE);
	size_t mtu = mhi_dev->mtu;
	void *buf;
	int ret = -EIO, i;

	for (i = 0; i < nr_trbs; i++) {
		buf = kmalloc(mtu, GFP_KERNEL);
		if (!buf)
			return -ENOMEM;

		ret = mhi_queue_transfer(mhi_dev, DMA_FROM_DEVICE, buf, mtu,
					 MHI_EOT);
		if (ret) {
			kfree(buf);
			return ret;
		}
	}

	return ret;
}

static void mhi_dtr_dl_xfer_cb(struct mhi_device *mhi_dev,
			       struct mhi_result *mhi_result)
{
	struct mhi_controller *mhi_cntrl = mhi_dev->mhi_cntrl;
	struct dtr_ctrl_msg *dtr_msg = mhi_result->buf_addr;
	u32 chan;
	spinlock_t *res_lock;

	if (mhi_result->transaction_status == -ENOTCONN) {
		kfree(mhi_result->buf_addr);
		return;
	}

	if (mhi_result->bytes_xferd != sizeof(*dtr_msg)) {
		MHI_ERR("Unexpected length %zu received\n",
			mhi_result->bytes_xferd);
		return;
	}

	MHI_LOG("preamble:0x%x msg_id:%u dest_id:%u msg:0x%x\n",
		 dtr_msg->preamble, dtr_msg->msg_id, dtr_msg->dest_id,
		 dtr_msg->msg);

	chan = CTRL_GET_CHID(dtr_msg);
	if (chan >= mhi_cntrl->max_chan)
		goto auto_queue;

	mhi_dev = mhi_cntrl->mhi_chan[chan].mhi_dev;
	if (!mhi_dev)
		goto auto_queue;

	res_lock = &mhi_dev->dev.devres_lock;
	spin_lock_irq(res_lock);
	mhi_dev->tiocm &= ~(TIOCM_CD | TIOCM_DSR | TIOCM_RI);

	if (dtr_msg->msg & CTRL_MSG_DCD)
		mhi_dev->tiocm |= TIOCM_CD;

	if (dtr_msg->msg & CTRL_MSG_DSR)
		mhi_dev->tiocm |= TIOCM_DSR;

	if (dtr_msg->msg & CTRL_MSG_RI)
		mhi_dev->tiocm |= TIOCM_RI;
	spin_unlock_irq(res_lock);

auto_queue:
	mhi_queue_transfer(mhi_cntrl->dtr_dev, DMA_FROM_DEVICE, mhi_result->buf_addr,
		mhi_cntrl->dtr_dev->mtu, MHI_EOT);
}

static void mhi_dtr_ul_xfer_cb(struct mhi_device *mhi_dev,
			       struct mhi_result *mhi_result)
{
	struct mhi_controller *mhi_cntrl = mhi_dev->mhi_cntrl;
	struct mhi_chan *dtr_chan = mhi_cntrl->dtr_dev->ul_chan;

	MHI_VERB("Received with status:%d\n", mhi_result->transaction_status);
	if (!mhi_result->transaction_status)
		complete(&dtr_chan->completion);
}

static void mhi_dtr_remove(struct mhi_device *mhi_dev)
{
	struct mhi_controller *mhi_cntrl = mhi_dev->mhi_cntrl;

	mhi_cntrl->dtr_dev = NULL;
}

static int mhi_dtr_probe(struct mhi_device *mhi_dev,
			 const struct mhi_device_id *id)
{
	struct mhi_controller *mhi_cntrl = mhi_dev->mhi_cntrl;
	int ret;

	MHI_LOG("Enter for DTR control channel\n");

	mhi_dev->mtu = min_t(size_t, id->driver_data, mhi_dev->mtu);
	ret = mhi_prepare_for_transfer(mhi_dev);
	if (!ret)
		mhi_cntrl->dtr_dev = mhi_dev;

	if (!ret)
		ret = mhi_dtr_queue_inbound(mhi_cntrl);

	MHI_LOG("Exit with ret:%d\n", ret);

	return ret;
}

static const struct mhi_device_id mhi_dtr_table[] = {
	{ .chan = "IP_CTRL", .driver_data = sizeof(struct dtr_ctrl_msg) },
	{},
};

static struct mhi_driver mhi_dtr_driver = {
	.id_table = mhi_dtr_table,
	.remove = mhi_dtr_remove,
	.probe = mhi_dtr_probe,
	.ul_xfer_cb = mhi_dtr_ul_xfer_cb,
	.dl_xfer_cb = mhi_dtr_dl_xfer_cb,
	.driver = {
		.name = "MHI_DTR",
		.owner = THIS_MODULE,
	}
};

int __init mhi_dtr_init(void)
{
	return mhi_driver_register(&mhi_dtr_driver);
}
void mhi_dtr_exit(void) {
	mhi_driver_unregister(&mhi_dtr_driver);
}
