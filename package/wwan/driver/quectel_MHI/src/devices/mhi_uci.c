// SPDX-License-Identifier: GPL-2.0-only
/* Copyright (c) 2018-2019, The Linux Foundation. All rights reserved.*/

#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/dma-direction.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/delay.h>
#if 1
static inline void *ipc_log_context_create(int max_num_pages,
        const char *modname, uint16_t user_version)
{ return NULL; }
static inline int ipc_log_string(void *ilctxt, const char *fmt, ...)
{ return -EINVAL; }
#endif
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/poll.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/uaccess.h>
#include <linux/tty.h>
#include "../core/mhi.h"

#define DEVICE_NAME "mhi"
#define MHI_UCI_DRIVER_NAME "mhi_uci_q"

struct uci_chan {
	wait_queue_head_t wq;
	spinlock_t lock;
	struct list_head pending; /* user space waiting to read */
	struct uci_buf *cur_buf; /* current buffer user space reading */
	size_t rx_size;
};

struct uci_buf {
	struct page *page;
	void *data;
	size_t len;
	unsigned nr_trb;
	struct list_head node;
};

struct uci_dev {
	struct list_head node;
	dev_t devt;
	struct device *dev;
	struct mhi_device *mhi_dev;
	const char *chan;
	struct mutex mutex; /* sync open and close */
	struct mutex r_mutex;
	struct mutex w_mutex;
	struct uci_chan ul_chan;
	struct uci_chan dl_chan;
	size_t mtu;
	int ref_count;
	bool enabled;
	unsigned rx_error;
	unsigned nr_trb;
	unsigned nr_trbs;
	struct uci_buf *uci_buf;
	struct ktermios	termios;
	size_t bytes_xferd;
};

struct mhi_uci_drv {
	struct list_head head;
	struct mutex lock;
	struct class *class;
	int major;
	dev_t dev_t;
};

static int uci_msg_lvl = MHI_MSG_LVL_ERROR;
module_param( uci_msg_lvl, uint, S_IRUGO | S_IWUSR);

#define MSG_VERB(fmt, ...) do { \
		if (uci_msg_lvl <= MHI_MSG_LVL_VERBOSE) \
			pr_err("[D][%s] " fmt, __func__, ##__VA_ARGS__); \
	} while (0)

#define MSG_LOG(fmt, ...) do { \
		if (uci_msg_lvl <= MHI_MSG_LVL_INFO) \
			pr_err("[I][%s] " fmt, __func__, ##__VA_ARGS__); \
	} while (0)

#define MSG_ERR(fmt, ...) do { \
		if (uci_msg_lvl <= MHI_MSG_LVL_ERROR) \
			pr_err("[E][%s] " fmt, __func__, ##__VA_ARGS__); \
	} while (0)

#define MAX_UCI_DEVICES (64)
#define QUEC_MHI_UCI_ALWAYS_OPEN //by now, sdx20 can not handle "start-reset-start" operation, so the simply solution is keep start state

static DECLARE_BITMAP(uci_minors, MAX_UCI_DEVICES);
static struct mhi_uci_drv mhi_uci_drv;

static int mhi_queue_inbound(struct uci_dev *uci_dev)
{
	struct mhi_device *mhi_dev = uci_dev->mhi_dev;
	int nr_trbs = mhi_get_no_free_descriptors(mhi_dev, DMA_FROM_DEVICE);
	size_t mtu = uci_dev->mtu;
	void *buf;
	struct uci_buf *uci_buf;
	int ret = -EIO, i;

	if (uci_dev->uci_buf == NULL) {
		uci_dev->nr_trb = 0;
		uci_dev->nr_trbs = (nr_trbs + 1);
		uci_dev->uci_buf = kmalloc_array(uci_dev->nr_trbs, sizeof(*uci_buf), GFP_KERNEL);
		if (!uci_dev->uci_buf)
			return -ENOMEM;

		uci_buf = uci_dev->uci_buf;
		for (i = 0; i < uci_dev->nr_trbs; i++, uci_buf++) {
			uci_buf->page = alloc_pages(GFP_KERNEL, get_order(mtu));
			if (!uci_buf->page)
				return -ENOMEM;
			uci_buf->data = page_address(uci_buf->page);
			uci_buf->len = 0;
			uci_buf->nr_trb = i;
			if (mhi_dev->dl_chan_id == MHI_CLIENT_DUN_IN) {
				//MSG_ERR("[%d] = %p\n", i, uci_buf->data);
			}
		}
	}

	for (i = 0; i < nr_trbs; i++) {
		#if 0
		buf = kmalloc(mtu + sizeof(*uci_buf), GFP_KERNEL);
		if (!buf)
			return -ENOMEM;

		uci_buf = buf + mtu;
		uci_buf->data = buf;
		#else
		uci_buf = &uci_dev->uci_buf[i];
		buf = uci_buf->data;
		#endif

		MSG_VERB("Allocated buf %d of %d size %zu\n", i, nr_trbs, mtu);

		ret = mhi_queue_transfer(mhi_dev, DMA_FROM_DEVICE, buf, mtu,
					 MHI_EOT);
		if (ret) {
			#if 0
			kfree(buf);
			#endif
			MSG_ERR("Failed to queue buffer %d\n", i);
			return ret;
		}
	}

	return ret;
}

static long mhi_uci_ioctl(struct file *file,
			  unsigned int cmd,
			  unsigned long arg)
{
	struct uci_dev *uci_dev = file->private_data;
	struct mhi_device *mhi_dev = uci_dev->mhi_dev;
	long ret = -ERESTARTSYS;

	mutex_lock(&uci_dev->mutex);
	if (uci_dev->enabled)
		ret = mhi_ioctl(mhi_dev, cmd, arg);

	if (uci_dev->enabled) {
		switch (cmd) {
		case TCGETS:
#ifndef TCGETS2
			ret = kernel_termios_to_user_termios((struct termios __user *)arg, &uci_dev->termios);
#else
			ret = kernel_termios_to_user_termios_1((struct termios __user *)arg, &uci_dev->termios);
#endif
		break;

		case TCSETSF:
		case TCSETS:
#ifndef TCGETS2
			ret = user_termios_to_kernel_termios(&uci_dev->termios, (struct termios __user *)arg);
#else
			ret = user_termios_to_kernel_termios_1(&uci_dev->termios, (struct termios __user *)arg);
#endif
		break;

		case TCFLSH:
			ret = 0;
		break;

		default:
		break;
		}
	}
	mutex_unlock(&uci_dev->mutex);

	return ret;
}

static int mhi_uci_release(struct inode *inode, struct file *file)
{
	struct uci_dev *uci_dev = file->private_data;

	mutex_lock(&uci_dev->mutex);
	uci_dev->ref_count--;
	if (!uci_dev->ref_count) {
		struct uci_chan *uci_chan;

		MSG_LOG("Last client left, closing node\n");

		if (uci_dev->enabled)
			mhi_unprepare_from_transfer(uci_dev->mhi_dev);

		/* clean inbound channel */
		uci_chan = &uci_dev->dl_chan;
		if (uci_dev->uci_buf) {
			unsigned nr_trb = 0;

			for (nr_trb = 0; nr_trb < uci_dev->nr_trbs; nr_trb++) {
				if (uci_dev->uci_buf[nr_trb].page)
					__free_pages(uci_dev->uci_buf[nr_trb].page, get_order(uci_dev->mtu));
			}
			kfree(uci_dev->uci_buf);
		}

		uci_chan->cur_buf = NULL;

		if (!uci_dev->enabled) {
			MSG_LOG("Node is deleted, freeing dev node\n");
			mutex_unlock(&uci_dev->mutex);
			mutex_destroy(&uci_dev->mutex);
			clear_bit(MINOR(uci_dev->devt), uci_minors);
			kfree(uci_dev);
			return 0;
		}
	}

	MSG_LOG("exit: ref_count:%d\n", uci_dev->ref_count);

	mutex_unlock(&uci_dev->mutex);

	return 0;
}

static unsigned int mhi_uci_poll(struct file *file, poll_table *wait)
{
	struct uci_dev *uci_dev = file->private_data;
	struct mhi_device *mhi_dev = uci_dev->mhi_dev;
	struct uci_chan *uci_chan;
	unsigned int mask = 0;

	poll_wait(file, &uci_dev->dl_chan.wq, wait);
	poll_wait(file, &uci_dev->ul_chan.wq, wait);

	uci_chan = &uci_dev->dl_chan;
	spin_lock_bh(&uci_chan->lock);
	if (!uci_dev->enabled) {
		mask = POLLERR;
	} else if (!list_empty(&uci_chan->pending) || uci_chan->cur_buf) {
		MSG_VERB("Client can read from node\n");
		mask |= POLLIN | POLLRDNORM;
	}
	spin_unlock_bh(&uci_chan->lock);

	uci_chan = &uci_dev->ul_chan;
	spin_lock_bh(&uci_chan->lock);
	if (!uci_dev->enabled) {
		mask |= POLLERR;
	} else if (mhi_get_no_free_descriptors(mhi_dev, DMA_TO_DEVICE) > 0) {
		MSG_VERB("Client can write to node\n");
		mask |= POLLOUT | POLLWRNORM;
	}

	if (!uci_dev->enabled)
		mask |= POLLHUP;
	if (uci_dev->rx_error)
		mask |= POLLERR;

	spin_unlock_bh(&uci_chan->lock);

	MSG_LOG("Client attempted to poll, returning mask 0x%x\n", mask);

	return mask;
}

static ssize_t mhi_uci_write(struct file *file,
			     const char __user *buf,
			     size_t count,
			     loff_t *offp)
{
	struct uci_dev *uci_dev = file->private_data;
	struct mhi_device *mhi_dev = uci_dev->mhi_dev;
	struct uci_chan *uci_chan = &uci_dev->ul_chan;
	size_t bytes_xfered = 0;
	int ret, nr_avail;

	if (!buf || !count || uci_dev->rx_error)
		return -EINVAL;

	/* confirm channel is active */
	spin_lock_bh(&uci_chan->lock);
	if (!uci_dev->enabled) {
		spin_unlock_bh(&uci_chan->lock);
		return -ERESTARTSYS;
	}

	MSG_VERB("Enter: to xfer:%zu bytes\n", count);

	while (count) {
		size_t xfer_size;
		void *kbuf;
		enum MHI_FLAGS flags;

		spin_unlock_bh(&uci_chan->lock);

		nr_avail = mhi_get_no_free_descriptors(mhi_dev, DMA_TO_DEVICE);
 		if ((nr_avail == 0) && (file->f_flags & O_NONBLOCK))
			return -EAGAIN;

		/* wait for free descriptors */
		ret = wait_event_interruptible(uci_chan->wq,
			(!uci_dev->enabled) ||
			(nr_avail = mhi_get_no_free_descriptors(mhi_dev,
							DMA_TO_DEVICE)) > 0);

		if (ret == -ERESTARTSYS || !uci_dev->enabled) {
			MSG_LOG("Exit signal caught for node or not enabled\n");
			return -ERESTARTSYS;
		}

		xfer_size = min_t(size_t, count, uci_dev->mtu);
		kbuf = kmalloc(xfer_size, GFP_KERNEL);
		if (!kbuf) {
			MSG_ERR("Failed to allocate memory %zu\n", xfer_size);
			return -ENOMEM;
		}

		ret = copy_from_user(kbuf, buf, xfer_size);
		if (unlikely(ret)) {
			kfree(kbuf);
			return ret;
		}

		spin_lock_bh(&uci_chan->lock);

		/* if ring is full after this force EOT */
		if (nr_avail > 1 && (count - xfer_size))
			flags = MHI_CHAIN;
		else
			flags = MHI_EOT;

		if (uci_dev->enabled)
			ret = mhi_queue_transfer(mhi_dev, DMA_TO_DEVICE, kbuf,
						 xfer_size, flags);
		else
			ret = -ERESTARTSYS;

		if (ret) {
			kfree(kbuf);
			goto sys_interrupt;
		}

		bytes_xfered += xfer_size;
		count -= xfer_size;
		buf += xfer_size;
	}

	spin_unlock_bh(&uci_chan->lock);
	MSG_VERB("Exit: Number of bytes xferred:%zu\n", bytes_xfered);

	return bytes_xfered;

sys_interrupt:
	spin_unlock_bh(&uci_chan->lock);

	return ret;
}

static ssize_t mhi_uci_read(struct file *file,
			    char __user *buf,
			    size_t count,
			    loff_t *ppos)
{
	struct uci_dev *uci_dev = file->private_data;
	struct mhi_device *mhi_dev = uci_dev->mhi_dev;
	struct uci_chan *uci_chan = &uci_dev->dl_chan;
	struct uci_buf *uci_buf;
	char *ptr;
	size_t to_copy;
	int ret = 0;

	if (!buf || uci_dev->rx_error)
		return -EINVAL;

	MSG_VERB("Client provided buf len:%zu\n", count);

	/* confirm channel is active */
	spin_lock_bh(&uci_chan->lock);
	if (!uci_dev->enabled) {
		spin_unlock_bh(&uci_chan->lock);
		return -ERESTARTSYS;
	}

	/* No data available to read, wait */
	if (!uci_chan->cur_buf && list_empty(&uci_chan->pending)) {
		MSG_VERB("No data available to read waiting\n");

		spin_unlock_bh(&uci_chan->lock);

		if (file->f_flags & O_NONBLOCK)
			return -EAGAIN;

		ret = wait_event_interruptible(uci_chan->wq,
				(!uci_dev->enabled ||
				 !list_empty(&uci_chan->pending)));
		if (ret == -ERESTARTSYS) {
			MSG_LOG("Exit signal caught for node\n");
			return -ERESTARTSYS;
		}

		spin_lock_bh(&uci_chan->lock);
		if (!uci_dev->enabled) {
			MSG_LOG("node is disabled\n");
			ret = -ERESTARTSYS;
			goto read_error;
		}
	}

	/* new read, get the next descriptor from the list */
	if (!uci_chan->cur_buf) {
		uci_buf = list_first_entry_or_null(&uci_chan->pending,
						   struct uci_buf, node);
		if (unlikely(!uci_buf)) {
			ret = -EIO;
			goto read_error;
		}

		if (uci_buf->node.next == LIST_POISON1 || uci_buf->node.prev == LIST_POISON1) {
			dump_stack();
			ret = -EIO;
			MSG_ERR("chan[%d] data=%p, len=%zd, nr_trb=%d\n",
				mhi_dev->dl_chan_id, uci_buf->data, uci_buf->len, uci_buf->nr_trb);
			goto read_error;
		}

		list_del(&uci_buf->node);
		uci_chan->cur_buf = uci_buf;
		uci_chan->rx_size = uci_buf->len;
		MSG_VERB("Got pkt of size:%zu\n", uci_chan->rx_size);
	}

	uci_buf = uci_chan->cur_buf;
	spin_unlock_bh(&uci_chan->lock);

	/* Copy the buffer to user space */
	to_copy = min_t(size_t, count, uci_chan->rx_size);
	ptr = uci_buf->data + (uci_buf->len - uci_chan->rx_size);
	ret = copy_to_user(buf, ptr, to_copy);
	if (ret)
		return ret;

	MSG_VERB("Copied %zu of %zu bytes\n", to_copy, uci_chan->rx_size);
	uci_chan->rx_size -= to_copy;

	/* we finished with this buffer, queue it back to hardware */
	if (!uci_chan->rx_size) {
		spin_lock_bh(&uci_chan->lock);
		uci_chan->cur_buf = NULL;

		if (uci_dev->enabled)
#if 1 //this can make the address in ring do not change
		{
			if (uci_buf->page) {
				unsigned nr_trb = uci_buf->nr_trb ? (uci_buf->nr_trb - 1) : (uci_dev->nr_trbs - 1);

				uci_buf = &uci_dev->uci_buf[nr_trb];
				ret = mhi_queue_transfer(mhi_dev, DMA_FROM_DEVICE,
							 uci_buf->data, uci_dev->mtu,
							 MHI_EOT);
			} else {
				kfree(uci_buf);
				ret = 0;
			}
		}
#endif
		else
			ret = -ERESTARTSYS;

		if (ret) {
			MSG_ERR("Failed to recycle element for chan:%d , ret=%d\n", mhi_dev->ul_chan_id, ret);
#if 0
			kfree(uci_buf->data);
#endif
			goto read_error;
		}

		spin_unlock_bh(&uci_chan->lock);
	}

	MSG_VERB("Returning %zu bytes\n", to_copy);

	return to_copy;

read_error:
	spin_unlock_bh(&uci_chan->lock);

	return ret;
}

static ssize_t mhi_uci_write_mutex(struct file *file,
			     const char __user *buf,
			     size_t count,
			     loff_t *offp)
{
	struct uci_dev *uci_dev = file->private_data;
	int ret;

	ret = mutex_lock_interruptible(&uci_dev->w_mutex); /*concurrent writes */
	if (ret < 0)
		return -ERESTARTSYS;

	ret = mhi_uci_write(file, buf, count, offp);
	mutex_unlock(&uci_dev->w_mutex);

	return ret;
}

static ssize_t mhi_uci_read_mutex(struct file *file,
			    char __user *buf,
			    size_t count,
			    loff_t *ppos)
{
	struct uci_dev *uci_dev = file->private_data;
	int ret;

	ret = mutex_lock_interruptible(&uci_dev->r_mutex); /*concurrent reads */
	if (ret < 0)
		return -ERESTARTSYS;

	ret = mhi_uci_read(file, buf, count, ppos);
	mutex_unlock(&uci_dev->r_mutex);

	return ret;
}

static int mhi_uci_open(struct inode *inode, struct file *filp)
{
	struct uci_dev *uci_dev = NULL, *tmp_dev;
	int ret = -EIO;
	struct uci_chan *dl_chan;

	mutex_lock(&mhi_uci_drv.lock);
	list_for_each_entry(tmp_dev, &mhi_uci_drv.head, node) {
		if (tmp_dev->devt == inode->i_rdev) {
			uci_dev = tmp_dev;
			break;
		}
	}

	/* could not find a minor node */
	if (!uci_dev)
		goto error_exit;

	mutex_lock(&uci_dev->mutex);
	if (!uci_dev->enabled) {
		MSG_ERR("Node exist, but not in active state!\n");
		goto error_open_chan;
	}

	uci_dev->ref_count++;

	MSG_LOG("Node open, ref counts %u\n", uci_dev->ref_count);

	if (uci_dev->ref_count == 1) {
		MSG_LOG("Starting channel\n");
		ret = mhi_prepare_for_transfer(uci_dev->mhi_dev);
		if (ret) {
			MSG_ERR("Error starting transfer channels\n");
			uci_dev->ref_count--;
			goto error_open_chan;
		}

		ret = mhi_queue_inbound(uci_dev);
		if (ret)
			goto error_rx_queue;

#ifdef QUEC_MHI_UCI_ALWAYS_OPEN
		uci_dev->ref_count++;
#endif
	}

	filp->private_data = uci_dev;
	mutex_unlock(&uci_dev->mutex);
	mutex_unlock(&mhi_uci_drv.lock);

	return 0;

 error_rx_queue:
	dl_chan = &uci_dev->dl_chan;
	mhi_unprepare_from_transfer(uci_dev->mhi_dev);
	if (uci_dev->uci_buf) {
		unsigned nr_trb = 0;

		for (nr_trb = 0; nr_trb < uci_dev->nr_trbs; nr_trb++) {
			if (uci_dev->uci_buf[nr_trb].page)
				__free_pages(uci_dev->uci_buf[nr_trb].page, get_order(uci_dev->mtu));
		}
		kfree(uci_dev->uci_buf);
	}

 error_open_chan:
	mutex_unlock(&uci_dev->mutex);

error_exit:
	mutex_unlock(&mhi_uci_drv.lock);

	return ret;
}

static const struct file_operations mhidev_fops = {
	.open = mhi_uci_open,
	.release = mhi_uci_release,
	.read = mhi_uci_read_mutex,
	.write = mhi_uci_write_mutex,
	.poll = mhi_uci_poll,
	.unlocked_ioctl = mhi_uci_ioctl,
};

static void mhi_uci_remove(struct mhi_device *mhi_dev)
{
	struct uci_dev *uci_dev = mhi_device_get_devdata(mhi_dev);

	MSG_LOG("Enter\n");


	mutex_lock(&mhi_uci_drv.lock);
	mutex_lock(&uci_dev->mutex);

	/* disable the node */
	spin_lock_irq(&uci_dev->dl_chan.lock);
	spin_lock_irq(&uci_dev->ul_chan.lock);
	uci_dev->enabled = false;
	spin_unlock_irq(&uci_dev->ul_chan.lock);
	spin_unlock_irq(&uci_dev->dl_chan.lock);
	wake_up(&uci_dev->dl_chan.wq);
	wake_up(&uci_dev->ul_chan.wq);

	/* delete the node to prevent new opens */
	device_destroy(mhi_uci_drv.class, uci_dev->devt);
	uci_dev->dev = NULL;
	list_del(&uci_dev->node);

#ifdef QUEC_MHI_UCI_ALWAYS_OPEN
	if (uci_dev->ref_count > 0)
		uci_dev->ref_count--;
#endif

	/* safe to free memory only if all file nodes are closed */
	if (!uci_dev->ref_count) {
		mutex_unlock(&uci_dev->mutex);
		mutex_destroy(&uci_dev->mutex);
		clear_bit(MINOR(uci_dev->devt), uci_minors);
		kfree(uci_dev);
		mutex_unlock(&mhi_uci_drv.lock);
		return;
	}

	MSG_LOG("Exit\n");
	mutex_unlock(&uci_dev->mutex);
	mutex_unlock(&mhi_uci_drv.lock);

}

static int mhi_uci_probe(struct mhi_device *mhi_dev,
			 const struct mhi_device_id *id)
{
	struct uci_dev *uci_dev;
	int minor;
	char node_name[32];
	int dir;

	uci_dev = kzalloc(sizeof(*uci_dev), GFP_KERNEL);
	if (!uci_dev)
		return -ENOMEM;

	mutex_init(&uci_dev->mutex);
	mutex_init(&uci_dev->r_mutex);
	mutex_init(&uci_dev->w_mutex);
	uci_dev->mhi_dev = mhi_dev;

	minor = find_first_zero_bit(uci_minors, MAX_UCI_DEVICES);
	if (minor >= MAX_UCI_DEVICES) {
		kfree(uci_dev);
		return -ENOSPC;
	}

	mutex_lock(&uci_dev->mutex);
	mutex_lock(&mhi_uci_drv.lock);

	uci_dev->devt = MKDEV(mhi_uci_drv.major, minor);
#if 1
	if (mhi_dev->mhi_cntrl->cntrl_idx)
		uci_dev->dev = device_create(mhi_uci_drv.class, &mhi_dev->dev,
					     uci_dev->devt, uci_dev,
					     DEVICE_NAME "_%s%d",
					     mhi_dev->chan_name, mhi_dev->mhi_cntrl->cntrl_idx);
	else
		uci_dev->dev = device_create(mhi_uci_drv.class, &mhi_dev->dev,
					     uci_dev->devt, uci_dev,
					     DEVICE_NAME "_%s",
					     mhi_dev->chan_name);
#else
	uci_dev->dev = device_create(mhi_uci_drv.class, &mhi_dev->dev,
				     uci_dev->devt, uci_dev,
				     DEVICE_NAME "_%04x_%02u.%02u.%02u%s%d",
				     mhi_dev->dev_id, mhi_dev->domain,
				     mhi_dev->bus, mhi_dev->slot, "_pipe_",
				     mhi_dev->ul_chan_id);
#endif

	set_bit(minor, uci_minors);

	/* create debugging buffer */
	snprintf(node_name, sizeof(node_name), "mhi_uci_%04x_%02u.%02u.%02u_%d",
		 mhi_dev->dev_id, mhi_dev->domain, mhi_dev->bus, mhi_dev->slot,
		 mhi_dev->ul_chan_id);

	for (dir = 0; dir < 2; dir++) {
		struct uci_chan *uci_chan = (dir) ?
			&uci_dev->ul_chan : &uci_dev->dl_chan;
		spin_lock_init(&uci_chan->lock);
		init_waitqueue_head(&uci_chan->wq);
		INIT_LIST_HEAD(&uci_chan->pending);
	}

	uci_dev->termios = tty_std_termios;

	uci_dev->mtu = min_t(size_t, id->driver_data, mhi_dev->mtu);
	mhi_device_set_devdata(mhi_dev, uci_dev);
	uci_dev->enabled = true;

	list_add(&uci_dev->node, &mhi_uci_drv.head);
	mutex_unlock(&mhi_uci_drv.lock);
	mutex_unlock(&uci_dev->mutex);

	MSG_LOG("channel:%s successfully probed\n", mhi_dev->chan_name);

	return 0;
};

static void mhi_ul_xfer_cb(struct mhi_device *mhi_dev,
			   struct mhi_result *mhi_result)
{
	struct uci_dev *uci_dev = mhi_device_get_devdata(mhi_dev);
	struct uci_chan *uci_chan = &uci_dev->ul_chan;

	MSG_VERB("status:%d xfer_len:%zu\n", mhi_result->transaction_status,
		 mhi_result->bytes_xferd);

	kfree(mhi_result->buf_addr);
	if (!mhi_result->transaction_status)
		wake_up(&uci_chan->wq);
}

static void mhi_dl_xfer_cb(struct mhi_device *mhi_dev,
			   struct mhi_result *mhi_result)
{
	struct uci_dev *uci_dev = mhi_device_get_devdata(mhi_dev);
	struct uci_chan *uci_chan = &uci_dev->dl_chan;
	unsigned long flags;
	struct uci_buf *buf;
	unsigned nr_trb = uci_dev->nr_trb;

	buf = &uci_dev->uci_buf[nr_trb];
	if (buf->nr_trb != nr_trb || buf->data != mhi_result->buf_addr)
	{
		uci_dev->rx_error++;
		MSG_ERR("chan[%d]: uci_buf[%u] = %p , mhi_result[%u] = %p\n",
			mhi_dev->dl_chan_id, buf->nr_trb, buf->data, nr_trb, mhi_result->buf_addr);
		return;
	}

	uci_dev->nr_trb++;
	if (uci_dev->nr_trb == uci_dev->nr_trbs)
		uci_dev->nr_trb = 0;

	if (mhi_result->transaction_status == -ENOTCONN) {
		return;
	}

	if (mhi_result->bytes_xferd > uci_dev->mtu || mhi_result->bytes_xferd <= 0)
	{
		MSG_ERR("chan[%d]: bytes_xferd = %zd , mtu = %zd\n",
			mhi_dev->dl_chan_id, mhi_result->bytes_xferd, uci_dev->mtu);
		return;
	}
	if (mhi_result->bytes_xferd > uci_dev->bytes_xferd)
	{
		uci_dev->bytes_xferd = mhi_result->bytes_xferd;
		//MSG_ERR("chan[%d]: bytes_xferd = %zd , mtu = %zd\n",
		//	mhi_dev->dl_chan_id, mhi_result->bytes_xferd, uci_dev->mtu);
	}

	MSG_VERB("status:%d receive_len:%zu\n", mhi_result->transaction_status,
		 mhi_result->bytes_xferd);

	spin_lock_irqsave(&uci_chan->lock, flags);
#if 0
	buf = mhi_result->buf_addr + uci_dev->mtu;
	buf->data = mhi_result->buf_addr;
#endif
	buf->len = mhi_result->bytes_xferd;
	if (mhi_dev->dl_chan_id ==  MHI_CLIENT_DUN_IN
		|| mhi_dev->dl_chan_id == MHI_CLIENT_QMI_IN
		|| mhi_dev->dl_chan_id ==  MHI_CLIENT_MBIM_IN) 
	{
		struct uci_buf *tmp_buf = NULL;
		int skip_buf = 0;

#ifdef QUEC_MHI_UCI_ALWAYS_OPEN
		if (uci_dev->ref_count == 1)
			skip_buf++;
#endif
		if (!skip_buf)
			tmp_buf = (struct uci_buf *)kmalloc(buf->len + sizeof(struct uci_buf), GFP_ATOMIC);;
		
		if (tmp_buf) {
			tmp_buf->page = NULL;
			tmp_buf->data = ((void *)tmp_buf) + sizeof(struct uci_buf);
			tmp_buf->len = buf->len;
			memcpy(tmp_buf->data, buf->data, buf->len);
		}

		if (buf) {
			struct uci_buf *uci_buf = buf;
			unsigned nr_trb = uci_buf->nr_trb ? (uci_buf->nr_trb - 1) : (uci_dev->nr_trbs - 1);

			uci_buf = &uci_dev->uci_buf[nr_trb];
			mhi_queue_transfer(mhi_dev, DMA_FROM_DEVICE, uci_buf->data, uci_dev->mtu, MHI_EOT);
		}

		buf = tmp_buf;
	}

	if (buf)
		list_add_tail(&buf->node, &uci_chan->pending);
	spin_unlock_irqrestore(&uci_chan->lock, flags);

#ifdef CONFIG_PM_SLEEP
	if (mhi_dev->dev.power.wakeup)
		__pm_wakeup_event(mhi_dev->dev.power.wakeup, 0);
#endif

	wake_up(&uci_chan->wq);
}

#define DIAG_MAX_PCIE_PKT_SZ	2048 //define by module

/* .driver_data stores max mtu */
static const struct mhi_device_id mhi_uci_match_table[] = {
	{ .chan = "LOOPBACK", .driver_data = 0x1000 },
	{ .chan = "SAHARA", .driver_data = 0x4000 },
	{ .chan = "EDL", .driver_data = 0x4000 },
	{ .chan = "DIAG", .driver_data = DIAG_MAX_PCIE_PKT_SZ },
	{ .chan = "MBIM", .driver_data = 0x1000 },
	{ .chan = "QMI0", .driver_data = 0x1000 },
	{ .chan = "QMI1", .driver_data = 0x1000 },
	{ .chan = "DUN", .driver_data = 0x1000 },
	{},
};

static struct mhi_driver mhi_uci_driver = {
	.id_table = mhi_uci_match_table,
	.remove = mhi_uci_remove,
	.probe = mhi_uci_probe,
	.ul_xfer_cb = mhi_ul_xfer_cb,
	.dl_xfer_cb = mhi_dl_xfer_cb,
	.driver = {
		.name = MHI_UCI_DRIVER_NAME,
		.owner = THIS_MODULE,
	},
};

int mhi_device_uci_init(void)
{
	int ret;

	ret = register_chrdev(0, MHI_UCI_DRIVER_NAME, &mhidev_fops);
	if (ret < 0)
		return ret;

	mhi_uci_drv.major = ret;
	mhi_uci_drv.class = class_create(THIS_MODULE, MHI_UCI_DRIVER_NAME);
	if (IS_ERR(mhi_uci_drv.class)) {
		unregister_chrdev(mhi_uci_drv.major, MHI_UCI_DRIVER_NAME);
		return -ENODEV;
       }

	mutex_init(&mhi_uci_drv.lock);
	INIT_LIST_HEAD(&mhi_uci_drv.head);

	ret = mhi_driver_register(&mhi_uci_driver);
	if (ret) {
		class_destroy(mhi_uci_drv.class);
		unregister_chrdev(mhi_uci_drv.major, MHI_UCI_DRIVER_NAME);
	}

	return ret;
}

void mhi_device_uci_exit(void)
{
	mhi_driver_unregister(&mhi_uci_driver);
	class_destroy(mhi_uci_drv.class);
	unregister_chrdev(mhi_uci_drv.major, MHI_UCI_DRIVER_NAME);
}
