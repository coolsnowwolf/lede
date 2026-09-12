// SPDX-License-Identifier: GPL-2.0
/* Platform NPU driver for Phytium NPU controller
 *
 * Copyright (C) 2023 Phytium Technology Co., Ltd.
 */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/miscdevice.h>
#include <linux/poll.h>
#include <linux/fs.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/pm_runtime.h>
#include "phytium_npu.h"
#include "phytium_npu_uapi.h"

static int phytium_npu_open(struct inode *inode, struct file *file)
{
	struct miscdevice  *miscdev = (struct miscdevice *)file->private_data;
	struct phytium_npu_dev *npudev = container_of(miscdev, struct phytium_npu_dev, miscdev);

	pr_debug("open device here!, npudev addr:%p, miscdev:%p", npudev, miscdev);
	if (!npudev->dev)
		return -1;

	struct phytium_npu_session *sess = phytium_npu_session_create(npudev->dev);

	if (!sess) {
		dev_err(npudev->dev, "No memory for creating session.");
		return -ENOMEM;
	}

	phytium_npu_session_init(npudev, sess);

	phytium_npu_create_new_mmu_context(npudev, sess);
	mutex_lock(&npudev->mutex_lock);
	phytium_npu_try_resume_work(npudev);
	mutex_unlock(&npudev->mutex_lock);
	file->private_data = sess;
	return 0;
}

static int phytium_npu_release(struct inode *inode, struct file *file)
{
	struct phytium_npu_session *sess = file->private_data;
	struct phytium_npu_dev *npu;
	int ret;

	if (!sess)
		return -EINVAL;
	npu = sess->npu_dev;
	pr_debug("npu close here!, npudev addr:%p, sess:%p", npu, sess);
	ret = mutex_lock_interruptible(&npu->mutex_lock);
	if (ret)
		return ret;

	phytium_npu_session_release(npu, sess);
	file->private_data = NULL;
	mutex_unlock(&npu->mutex_lock);
	return 0;
}

static ssize_t phytium_npu_read(struct file *file, char __user *buf, size_t len, loff_t *ppos)
{
	struct phytium_npu_session *sess = file->private_data;
	struct phytium_npu_dev *npu = sess->npu_dev;
	int ret, ret_len;

	pr_debug("%s:user reads the response", __func__);
	if (!sess)
		return -EINVAL;
	pr_debug("%s: with sess :%p, id:%d", __func__, sess, sess->id);
	ret = mutex_lock_interruptible(&npu->mutex_lock);
	if (ret)
		return ret;
	while (list_empty(&sess->response_list)) {
		if (file->f_flags & O_NONBLOCK) {
			dev_dbg(npu->dev, "%s: no block!", __func__);
			mutex_unlock(&npu->mutex_lock);
			return -EAGAIN;
		}
		dev_dbg(npu->dev, "%s: going to sleep\n", __func__);
		if (wait_event_interruptible(sess->response_wq,
					     !list_empty(&sess->response_list))) {
			dev_dbg(npu->dev, "%s: signal\n", __func__);
			mutex_unlock(&npu->mutex_lock);
			return -ERESTARTSYS;
		}

		dev_dbg(npu->dev, "%s: woken up\n", __func__);
	}

	if (list_empty(&sess->response_list)) {
		ret = 0;
		goto out;
	}

	struct npu_user_stream_rsp *rsp;

	rsp = list_first_entry(&sess->response_list,
			       struct npu_user_stream_rsp,
								stream_rsp_list_entry);
	if (!rsp) {
		pr_debug("%s:get response NULL", __func__);
		ret = 0;
		goto out;
	}
	if (len >= rsp->rsp_size)
		ret_len = rsp->rsp_size;
	else
		ret_len = len;
	ret = copy_to_user(buf, &rsp->ursp, ret_len);
	if (ret) {
		ret = -EFAULT;
		goto out;
	}

	list_del(&rsp->stream_rsp_list_entry);
	ret = ret_len;

	if (!rsp->session)
		kfree(rsp);
out:
	mutex_unlock(&npu->mutex_lock);
	return ret;
}

static ssize_t phytium_npu_write(struct file *file, const char __user *buf,
				 size_t count, loff_t *pposn)
{
	struct phytium_npu_session *sess = file->private_data;
	struct phytium_npu_dev *npu = sess->npu_dev;

	if (!sess)
		return -EINVAL;
	phytium_npu_write_stream(npu, sess, buf, count);
	return count;
}

static unsigned int phytium_npu_poll(struct file *file, poll_table *wait)
{
	struct phytium_npu_session *sess = file->private_data;
	unsigned long event = poll_requested_events(wait);
	unsigned int mask = 0, ret;

	if (!sess)
		return -EINVAL;
	pr_debug("%s: PID: %d, sess id: %d, link: %p\n", __func__,
		 task_pid_nr(current), sess->id, sess);
	ret = mutex_lock_interruptible(&sess->npu_dev->mutex_lock);
	if (ret)
		return POLLERR;
	if (event & (POLLIN | POLLRDNORM)) {
		/* Register for event */
		poll_wait(file, &sess->response_wq, wait);

		if (!list_empty(&sess->response_list))
			mask = POLLIN | POLLRDNORM;
	}
	mutex_unlock(&sess->npu_dev->mutex_lock);
	pr_debug("return mask %x, POLLIN:%x, POLLRDNORM:%x", mask, POLLIN, POLLRDNORM);
	return mask;
}

static int phytium_npu_mm_debug_perf(struct phytium_npu_dev *npu,
				     struct phytium_npu_session *sess,
							void *arg)
{
	struct npu_debug_perf *dbg_cfg = (struct npu_debug_perf *)arg;
	struct phytium_npu_debugfs *dbgfs = &sess->dbgfs;
	int ret;

	if (copy_from_user(&sess->dbgfs, dbg_cfg, sizeof(*dbg_cfg)))
		return -EFAULT;
	pr_debug("[%s]:config debug info:%d,type:%#x", __func__,
		 dbgfs->debug_mode, dbgfs->debug_type);

	ret = mutex_lock_interruptible(&npu->mutex_lock);
	if (ret)
		return ret;

	ret = phytiun_npu_check_debug_fs_cfg(sess);
	mutex_unlock(&npu->mutex_lock);
	return ret;
}

static int phytium_npu_mm_repeat_stream(struct phytium_npu_dev *npu,
					struct phytium_npu_session *sess,
							void *arg)
{
	struct npu_repeat_stream stream;
	struct npu_repeat_stream *rstream = &stream;
	int ret;

	if (copy_from_user(rstream, arg, sizeof(*rstream)))
		return -EFAULT;
	ret = mutex_lock_interruptible(&npu->mutex_lock);
	if (ret)
		return ret;

	phytium_npu_repeat_stream(sess, rstream->stream_id & rstream->stream_id_mask,
				  rstream->is_repeat);
	mutex_unlock(&npu->mutex_lock);
	return 0;
}

static int phytium_npu_mm_delete_stream(struct phytium_npu_dev *npu,
					struct phytium_npu_session *sess,
							void *arg)
{
	struct npu_delete_stream stream;
	struct npu_delete_stream *dstream = &stream;
	int ret;

	if (copy_from_user(dstream, arg, sizeof(*dstream)))
		return -EFAULT;
	pr_info("%s: dstream id :%x, mask:%x, response:%d", __func__, dstream->stream_id,
		dstream->stream_id_mask, dstream->is_res);
	ret = mutex_lock_interruptible(&npu->mutex_lock);
	if (ret)
		return ret;

	phytium_npu_delete_stream(npu, sess, dstream);
	mutex_unlock(&npu->mutex_lock);
	return 0;
}

static int phytium_npu_mm_sync_buf(struct phytium_npu_dev *npu,
				   struct phytium_npu_session *sess, void *arg)
{//TODO.
	return 0;
}

static int phytium_npu_mm_set_buf(struct phytium_npu_dev *npu,
				  struct phytium_npu_session *sess,
						void *arg)
{
	struct npu_set_buffer buf;
	struct npu_set_buffer *sbuf = &buf;
	int ret;

	if (copy_from_user(sbuf, arg, sizeof(*sbuf)))
		return -EFAULT;
	ret = mutex_lock_interruptible(&npu->mutex_lock);
	if (ret)
		return ret;
	phytium_npu_set_stream_buf_status_with_fd(sess, sbuf->fd, sbuf->set_state);
	mutex_unlock(&npu->mutex_lock);
	return 0;
}

static int phytium_npu_mm_unmap(struct phytium_npu_dev *npu,
				struct phytium_npu_session *sess,
					void *arg)
{
	int ret;
	struct npu_memory_unmap usr_unmap;

	if (copy_from_user(&usr_unmap, arg, sizeof(usr_unmap)))
		return -EFAULT;
	ret = mutex_lock_interruptible(&npu->mutex_lock);
	if (ret)
		return ret;

	phytium_npu_mmu_unmap(npu, sess, &usr_unmap);
	pr_debug("------unmap done");
	mutex_unlock(&npu->mutex_lock);
	return 0;
}

static int phytium_npu_mm_import(struct phytium_npu_dev *npu,
				 struct phytium_npu_session *sess,
							void *arg)
{
	struct npu_memory_map tmp;
	struct npu_memory_map *map = &tmp;
	int ret;

	if (copy_from_user(map, arg, sizeof(*map)))
		return -EFAULT;
	pr_debug("map type:%x, fd:%d, vaddr:%llx", map->map_type, map->fd, map->vaddr);
	ret = mutex_lock_interruptible(&npu->mutex_lock);
	if (ret)
		return ret;

	ret = phytium_npu_mmu_map_init(npu, sess, map);
	if (ret)
		return ret;
	phytium_npu_import_dmabuf(npu, sess, map);
	mutex_unlock(&npu->mutex_lock);
	return 0;
}

static int phytium_npu_mm_map2cache(struct phytium_npu_dev *npu,
				    struct phytium_npu_session *sess, void *arg)
{
	return 0;
}

#define PRODUCT_ID 0x0102030405060708
#define KVERSION    0x0001
static int
phytium_npu_get_info(struct phytium_npu_dev *npu, struct phytium_npu_session *sess, void *arg)
{
	struct npu_info sinfo;
	struct npu_info *info = &sinfo;

	info->pid = PRODUCT_ID;
	info->version = 0x0001;
	info->is_have_mmu = 1;
	info->mmu_page_size = npu->nmmu_config.page_size;
	info->mefficiency = 1;
	info->use_debug = 0;
	info->core_num = 1;
	info->l1_size = 0;
	info->l3_size = 0;
	info->l3_percore_size = 0;
	info->clock_freq = npu->clock_freq;
	if (copy_to_user(arg, info, sizeof(*info)))
		return -EFAULT;
	return 0;
}

static long phytium_npu_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	struct phytium_npu_session *sess = file->private_data;
	struct phytium_npu_dev *npu = sess->npu_dev;
	int retval = 0;

	if (!sess)
		return -EINVAL;
	dev_dbg(npu->dev, "%s: cmd: 0x%x\n", __func__, cmd);

	switch (cmd) {
	case NPU_INFO:
		retval =  phytium_npu_get_info(npu, sess, (void __user *)arg);
		break;

	case NPU_MAP2CACHE:
		retval = phytium_npu_mm_map2cache(npu, sess, (void __user *)arg);
		break;

	case NPU_MEMORY_IMPORT:
		retval = phytium_npu_mm_import(npu, sess, (void __user *)arg);
		break;

	case NPU_MEMORY_UNMAP:
		retval = phytium_npu_mm_unmap(npu, sess, (void __user *)arg);
		break;

	case NPU_SET_BUF:
		retval = phytium_npu_mm_set_buf(npu, sess, (void __user *)arg);
		break;

	case NPU_SYNC_BUF:
		retval = phytium_npu_mm_sync_buf(npu, sess, (void __user *)arg);
		break;
	case NPU_DELET_STREAM:
		retval = phytium_npu_mm_delete_stream(npu, sess, (void __user *)arg);
		break;
	case NPU_REPEAT_STREAM:
		retval = phytium_npu_mm_repeat_stream(npu, sess, (void __user *)arg);
		break;
	case NPU_DEBUG_PERF:
		retval = phytium_npu_mm_debug_perf(npu, sess, (void __user *)arg);
		break;
	default:
		dev_err(npu->dev, "No this cmd to execute.");
		break;
	}

	return retval;
}

static const struct file_operations phytium_npu_fops = {
	.owner			= THIS_MODULE,
	.read			= phytium_npu_read,
	.poll			= phytium_npu_poll,
	.write			= phytium_npu_write,
	.open			= phytium_npu_open,
	.unlocked_ioctl		= phytium_npu_ioctl,
	.compat_ioctl		= phytium_npu_ioctl,
	.release		= phytium_npu_release,
};

int phytium_npu_register_misc(struct phytium_npu_dev *npudev)
{
	int ret;
	char *npu_dev_name = NULL;

	if (!npudev || !npudev->dev) {
		pr_err("%s: invalid params!\n", __func__);
		return -EINVAL;
	}

	npu_dev_name = devm_kzalloc(npudev->dev, 8, GFP_KERNEL);
	if (!npu_dev_name)
		return -ENOMEM;

	snprintf(npu_dev_name, 8, "npu%d", 0);

	dev_dbg(npudev->dev, "%s: trying to register NPU misc /dev/%s ...\n",
		__func__, npu_dev_name);

	npudev->miscdev.minor = MISC_DYNAMIC_MINOR;
	npudev->miscdev.fops = &phytium_npu_fops;
	npudev->miscdev.name = npu_dev_name;

	ret = misc_register(&npudev->miscdev);
	if (ret) {
		dev_err(npudev->dev, "%s: Unable to register NPU misc device\n", __func__);
		goto err;
	}

	dev_dbg(npudev->dev, "%s: NPU misc device registered successfully\n", __func__);
	pr_info("%s, misc:%p, npu:%p\n", __func__, &npudev->miscdev, npudev);

	return 0;

err:
	devm_kfree(npudev->dev, npu_dev_name);
	return ret;
}

int phytium_npu_unregister_misc(struct phytium_npu_dev *npudev)
{
	if (!npudev || !npudev->dev) {
		pr_err("%s: invalid params!\n", __func__);
		return -EINVAL;
	}

	misc_deregister(&npudev->miscdev);

	dev_dbg(npudev->dev, "%s: NPU misc device unregistered successfully\n", __func__);

	return 0;
}
