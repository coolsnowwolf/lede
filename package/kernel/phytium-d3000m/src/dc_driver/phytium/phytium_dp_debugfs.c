// SPDX-License-Identifier: GPL-2.0
/* Phytium display drm driver
 *
 * Copyright (C) 2021-2025, Phytium Technology Co., Ltd.
 */

#include <linux/version.h>
#include <linux/debugfs.h>
#include <linux/fs.h>
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 18))
#include <linux/idr.h>
#endif
#include <drm/drm_file.h>
#include <drm/drm_atomic.h>
#include "ftd330_drv.h"
#include "ftd330_dc_hw.h"
#include "ftd330_dc.h"
#include "ftd330_crtc.h"
#include "phytium_vrr.h"
#include "phytium_psr.h"
#include "phytium_dp.h"
#include "phytium_dp_reg.h"
#include "phytium_dp_debugfs.h"
#include "phytium_dp_ioctl.h" 

#define SHOW_MEM_STATE 0
#if SHOW_MEM_STATE
const char *const mem_state[PHYTIUM_MEM_STATE_TYPE_COUNT] = {
	"Memory_Vram_Total",
	"Memory_Vram_Alloc",
	"Memory_System_Carveout_Total",
	"Memory_System_Carveout_Alloc",
	"Memory_System_Alloc",
};
#endif
struct StatusMapping {
	int status;
	const char *status_info;
};

struct StatusMapping sink_psr_status_map[] = {
	{DP_PSR_SINK_INACTIVE, "Sink is inactive"},
	{DP_PSR_SINK_ACTIVE_SRC_SYNCED, "Sink is active and synchronized with source"},
	{DP_PSR_SINK_ACTIVE_RFB, "Sink is active and receiving frame buffer"},
	{DP_PSR_SINK_ACTIVE_SINK_SYNCED, "Sink is active and synchronized with sink"},
	{DP_PSR_SINK_ACTIVE_RESYNC, "Sink is active and resynchronizing"},
	{DP_PSR_SINK_INTERNAL_ERROR, "Internal error in sink"},
};

struct StatusMapping source_psr_status_map[] = {
	{PSR_STATE_DISABLE, "Source psr is disable"},
	{PSR_STATE_INACTIVE, "Source is inactive"},
	{PSR_STATE_TRANS_TO_ACTIVE, "Source is transitioning to active state"},
	{PSR_STATE_ACTIVE, "Source is active"},
	{PSR_STATE_ACTIVE_UPDATE, "Source is active and updating single frame"},
	{PSR_STATE_EXIT, "Source exits psr state"},
};

struct StatusMapping dplp_frame_comp_status_map[] = {
	{0x0, "dplp is idle "},
	{DPLP_LOWFPS_AVAILABLE, "dplp is lowfps available"},
	{DPLP_PSR_AVAILABLE, "dplp is psr available"},
	{DPLP_LOWFPS_ACTIVE, "dplp is lowfps active"},
	{DPLP_PSR_ACTIVE, "dplp is psr active"},
};

static ssize_t
phytium_dp_register_write(struct file *filp,
				 const char __user *ubuf,
				 size_t len,
				 loff_t *ppos)
{
	char tmp[16];

	if (len >= sizeof(tmp))
		return -EINVAL;

	memset(tmp, 0, sizeof(tmp));
	if (copy_from_user(tmp, ubuf, len))
		return -EFAULT;
	tmp[len] = '\0';

	return len;
}

static int phytium_dp_register_show(struct seq_file *m, void *data)
{
	struct drm_connector *connector = m->private;
	struct phytium_dp_device *phytium_dp = connector_to_dp_device(connector);
	struct drm_device *dev =  phytium_dp->dev;
	struct ftd330_drm_private *priv = dev->dev_private;
	int port = phytium_dp->port;
	uint32_t group_offset = priv->dp_reg_base[port];

	seq_printf(m, "addr:h0x%08x	h0x%08x\n", PHYTIUM_DP_M_VID,
		   phytium_readl_reg(priv, group_offset, PHYTIUM_DP_M_VID));
	seq_printf(m, "addr:h0x%08x	h0x%08x\n", PHYTIUM_DP_N_VID,
		   phytium_readl_reg(priv, group_offset, PHYTIUM_DP_N_VID));
	seq_printf(m, "addr:h0x%08x	h0x%08x\n", PHYTIUM_DP_TRANSFER_UNIT_SIZE,
		   phytium_readl_reg(priv, group_offset, PHYTIUM_DP_TRANSFER_UNIT_SIZE));
	seq_printf(m, "addr:h0x%08x	h0x%08x\n", PHYTIUM_DP_DATA_COUNT,
		   phytium_readl_reg(priv, group_offset, PHYTIUM_DP_DATA_COUNT));
	seq_printf(m, "addr:h0x%08x	h0x%08x\n", PHYTIUM_DP_MAIN_LINK_HTOTAL,
		   phytium_readl_reg(priv, group_offset, PHYTIUM_DP_MAIN_LINK_HTOTAL));
	seq_printf(m, "addr:h0x%08x	h0x%08x\n", PHYTIUM_DP_MAIN_LINK_HRES,
		   phytium_readl_reg(priv, group_offset, PHYTIUM_DP_MAIN_LINK_HRES));
	seq_printf(m, "addr:h0x%08x	h0x%08x\n", PHYTIUM_DP_MAIN_LINK_HSWIDTH,
		   phytium_readl_reg(priv, group_offset, PHYTIUM_DP_MAIN_LINK_HSWIDTH));
	seq_printf(m, "addr:h0x%08x	h0x%08x\n", PHYTIUM_DP_MAIN_LINK_HSTART,
		   phytium_readl_reg(priv, group_offset, PHYTIUM_DP_MAIN_LINK_HSTART));
	seq_printf(m, "addr:h'0x%08x	h'0x%08x\n", PHYTIUM_DP_MAIN_LINK_VTOTAL,
		   phytium_readl_reg(priv, group_offset, PHYTIUM_DP_MAIN_LINK_VTOTAL));
	seq_printf(m, "addr:h'0x%08x	h'0x%08x\n", PHYTIUM_DP_MAIN_LINK_VRES,
		   phytium_readl_reg(priv, group_offset, PHYTIUM_DP_MAIN_LINK_VRES));
	seq_printf(m, "addr:h'0x%08x	h'0x%08x\n", PHYTIUM_DP_MAIN_LINK_VSWIDTH,
		   phytium_readl_reg(priv, group_offset, PHYTIUM_DP_MAIN_LINK_VSWIDTH));
	seq_printf(m, "addr:h'0x%08x	h'0x%08x\n", PHYTIUM_DP_MAIN_LINK_VSTART,
		   phytium_readl_reg(priv, group_offset, PHYTIUM_DP_MAIN_LINK_VSTART));
	seq_printf(m, "addr:h'0x%08x	h'0x%08x\n", PHYTIUM_DP_MAIN_LINK_POLARITY,
		   phytium_readl_reg(priv, group_offset, PHYTIUM_DP_MAIN_LINK_POLARITY));
	seq_printf(m, "addr:h'0x%08x	h'0x%08x\n", PHYTIUM_DP_MAIN_LINK_MISC0,
		   phytium_readl_reg(priv, group_offset, PHYTIUM_DP_MAIN_LINK_MISC0));
	seq_printf(m, "addr:h'0x%08x	h'0x%08x\n", PHYTIUM_DP_MAIN_LINK_MISC1,
		   phytium_readl_reg(priv, group_offset, PHYTIUM_DP_MAIN_LINK_MISC1));
	seq_printf(m, "addr:h'0x%08x	h'0x%08x\n", PHYTIUM_DP_USER_SYNC_POLARITY,
		   phytium_readl_reg(priv, group_offset, PHYTIUM_DP_USER_SYNC_POLARITY));
	seq_printf(m, "addr:h'0x%08x	h'0x%08x\n", PHYTIUM_DP_VIDEO_STREAM_ENABLE,
		   phytium_readl_reg(priv, group_offset, PHYTIUM_DP_VIDEO_STREAM_ENABLE));
	seq_printf(m, "addr:h'0x%08x	h'0x%08x\n", PHYTIUM_DP_SECONDARY_STREAM_ENABLE,
		   phytium_readl_reg(priv, group_offset, PHYTIUM_DP_SECONDARY_STREAM_ENABLE));
	seq_puts(m, "audio:\n");
	seq_printf(m, "addr:h'0x%08x	h'0x%08x\n", PHYTIUM_DP_SEC_INPUT_SELECT,
		   phytium_readl_reg(priv, group_offset, PHYTIUM_DP_SEC_INPUT_SELECT));
	seq_printf(m, "addr:h'0x%08x	h'0x%08x\n", PHYTIUM_DP_SEC_DIRECT_CLKDIV,
		   phytium_readl_reg(priv, group_offset, PHYTIUM_DP_SEC_DIRECT_CLKDIV));
	seq_printf(m, "addr:h'0x%08x	h'0x%08x\n", PHYTIUM_DP_SEC_CHANNEL_COUNT,
		   phytium_readl_reg(priv, group_offset, PHYTIUM_DP_SEC_CHANNEL_COUNT));
	seq_printf(m, "addr:h'0x%08x	h'0x%08x\n", PHYTIUM_DP_SEC_CHANNEL_MAP,
		   phytium_readl_reg(priv, group_offset, PHYTIUM_DP_SEC_CHANNEL_MAP));
	seq_printf(m, "addr:h'0x%08x	h'0x%08x\n", PHYTIUM_DP_SEC_DATA_WINDOW,
		   phytium_readl_reg(priv, group_offset, PHYTIUM_DP_SEC_DATA_WINDOW));
	seq_printf(m, "addr:h'0x%08x	h'0x%08x\n", PHYTIUM_DP_SEC_CS_CATEGORY_CODE,
		   phytium_readl_reg(priv, group_offset, PHYTIUM_DP_SEC_CS_CATEGORY_CODE));
	seq_printf(m, "addr:h'0x%08x	h'0x%08x\n", PHYTIUM_DP_SEC_MAUD,
		   phytium_readl_reg(priv, group_offset, PHYTIUM_DP_SEC_MAUD));
	seq_printf(m, "addr:h'0x%08x	h'0x%08x\n", PHYTIUM_DP_SEC_NAUD,
		   phytium_readl_reg(priv, group_offset, PHYTIUM_DP_SEC_NAUD));
	seq_printf(m, "addr:h'0x%08x	h'0x%08x\n", PHYTIUM_DP_SEC_CLOCK_MODE,
		   phytium_readl_reg(priv, group_offset, PHYTIUM_DP_SEC_CLOCK_MODE));
	seq_printf(m, "addr:h'0x%08x	h'0x%08x\n", PHYTIUM_DP_SEC_CS_SOURCE_FORMAT,
		   phytium_readl_reg(priv, group_offset, PHYTIUM_DP_SEC_CS_SOURCE_FORMAT));
	seq_printf(m, "addr:h'0x%08x	h'0x%08x\n", PHYTIUM_DP_SEC_CS_LENGTH_ORIG_FREQ,
		   phytium_readl_reg(priv, group_offset, PHYTIUM_DP_SEC_CS_LENGTH_ORIG_FREQ));
	seq_printf(m, "addr:h'0x%08x	h'0x%08x\n", PHYTIUM_DP_SEC_CS_FREQ_CLOCK_ACCURACY,
		   phytium_readl_reg(priv, group_offset, PHYTIUM_DP_SEC_CS_FREQ_CLOCK_ACCURACY));
	seq_printf(m, "addr:h'0x%08x	h'0x%08x\n", PHYTIUM_DP_SEC_AUDIO_ENABLE,
		   phytium_readl_reg(priv, group_offset, PHYTIUM_DP_SEC_AUDIO_ENABLE));

	return 0;
}

static int phytium_dp_register_open(struct inode *inode, struct file *file)
{
	return single_open(file, phytium_dp_register_show, inode->i_private);
}

static const struct file_operations phytium_dp_register_fops = {
	.owner = THIS_MODULE,
	.open = phytium_dp_register_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
	.write = phytium_dp_register_write,
};

static ssize_t
phytium_dp_trigger_train_fail_write(struct file *filp,
				 const char __user *ubuf,
				 size_t len,
				 loff_t *ppos)
{
	struct seq_file *m = filp->private_data;
	struct drm_connector *connector = m->private;
	struct phytium_dp_device *phytium_dp = connector_to_dp_device(connector);
	char tmp[16];

	if (len >= sizeof(tmp))
		return -EINVAL;

	memset(tmp, 0, sizeof(tmp));
	if (copy_from_user(tmp, ubuf, len))
		return -EFAULT;
	tmp[len] = '\0';

	if (kstrtouint(tmp, 10, &phytium_dp->trigger_train_fail) != 0)
		return -EINVAL;

	return len;
}

static int phytium_dp_trigger_train_fail_show(struct seq_file *m, void *data)
{
	struct drm_connector *connector = m->private;
	struct phytium_dp_device *phytium_dp = connector_to_dp_device(connector);

	seq_printf(m, "trigger_train_fail: %d\n", phytium_dp->trigger_train_fail);
	seq_printf(m, "train_retry_count: %d\n", phytium_dp->train_retry_count);

	return 0;
}

static int phytium_dp_trigger_train_fail_open(struct inode *inode, struct file *file)
{
	return single_open(file, phytium_dp_trigger_train_fail_show, inode->i_private);
}

static const struct file_operations phytium_dp_trigger_train_fail_fops = {
	.owner = THIS_MODULE,
	.open = phytium_dp_trigger_train_fail_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
	.write = phytium_dp_trigger_train_fail_write,
};

static int phytium_edp_backlight_show(struct seq_file *m, void *data)
{
	struct drm_connector *connector = m->private;
	struct phytium_dp_device *phytium_dp = connector_to_dp_device(connector);

	if (!phytium_dp->is_edp)
		return -ENODEV;

	mutex_lock(&phytium_dp->panel.panel_lock);
	seq_printf(m, "backlight: %s\n", phytium_dp->panel.backlight_enabled?"enabled":"disabled");
	mutex_unlock(&phytium_dp->panel.panel_lock);

	return 0;
}

static int phytium_edp_backlight_open(struct inode *inode, struct file *file)
{
	return single_open(file, phytium_edp_backlight_show, inode->i_private);
}

static const struct file_operations phytium_edp_backlight_fops = {
	.owner = THIS_MODULE,
	.open = phytium_edp_backlight_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static int phytium_edp_power_show(struct seq_file *m, void *data)
{
	struct drm_connector *connector = m->private;
	struct phytium_dp_device *phytium_dp = connector_to_dp_device(connector);

	if (!phytium_dp->is_edp)
		return -ENODEV;

	mutex_lock(&phytium_dp->panel.panel_lock);
	seq_printf(m, "power: %s\n", phytium_dp->panel.power_enabled?"enabled":"disabled");
	mutex_unlock(&phytium_dp->panel.panel_lock);

	return 0;
}

static int phytium_edp_power_open(struct inode *inode, struct file *file)
{
	return single_open(file, phytium_edp_power_show, inode->i_private);
}

static const struct file_operations phytium_edp_power_fops = {
	.owner = THIS_MODULE,
	.open = phytium_edp_power_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

struct dpcd_block {
	/* DPCD dump start address. */
	unsigned int offset;
	/* DPCD dump end address, inclusive. If unset, .size will be used. */
	unsigned int end;
	/* DPCD dump size. Used if .end is unset. If unset, defaults to 1. */
	size_t size;
	/* Only valid for eDP. */
	bool edp;
};

static const struct dpcd_block phytium_dpcd_debug[] = {
	{ .offset = DP_DPCD_REV, .size = DP_RECEIVER_CAP_SIZE },
	{ .offset = DP_PSR_SUPPORT, .end = DP_PSR_CAPS },
	{ .offset = DP_DOWNSTREAM_PORT_0, .size = 16 },
	{ .offset = DP_LINK_BW_SET, .end = DP_EDP_CONFIGURATION_SET },
	{ .offset = DP_SINK_COUNT, .end = DP_ADJUST_REQUEST_LANE2_3 },
	{ .offset = DP_SET_POWER },
	{ .offset = DP_EDP_DPCD_REV },
	{ .offset = DP_EDP_GENERAL_CAP_1, .end = DP_EDP_GENERAL_CAP_3 },
	{ .offset = DP_EDP_DISPLAY_CONTROL_REGISTER, .end = DP_EDP_BACKLIGHT_FREQ_CAP_MAX_LSB },
	{ .offset = DP_EDP_DBC_MINIMUM_BRIGHTNESS_SET, .end = DP_EDP_DBC_MAXIMUM_BRIGHTNESS_SET },
	{ .offset = DP_DEVICE_SERVICE_IRQ_VECTOR, .size = 1 },
	{ .offset = DP_TEST_REQUEST, .end = DP_TEST_PATTERN },
};

static loff_t phytium_dpcd_llseek(struct file *file, loff_t offset, int whence)
{
	loff_t new_pos;

	switch (whence) {
	case SEEK_SET:
		new_pos = offset;
		break;
	case SEEK_CUR:
		new_pos = file->f_pos + offset;
		break;
	case SEEK_END:
		new_pos = 0x100000 + offset;
		break;
	default:
		return -EINVAL;
	}

	if (new_pos < 0 || new_pos > 0x100000)
		return -EINVAL;

	if (new_pos < 0)
		return -EINVAL;

	file->f_pos = new_pos;
	return new_pos;
}	

static ssize_t phytium_dpcd_read(struct file *file, char __user *buf, 
							size_t size, loff_t *ppos)
{
	struct seq_file *m = file->private_data;
	struct drm_connector *connector = m->private;
	struct phytium_dp_device *phytium_dp = connector_to_dp_device(connector);
	unsigned long p = *ppos;

	ssize_t err;
	char *kbuf;

	if (connector->status != connector_status_connected)
		return -ENODEV;

	kbuf = kmalloc(size, GFP_KERNEL);
	if (!kbuf)
		return -ENOMEM;
	if (size > 16) {
		DRM_INFO("Supports a maximum of 16 bytes per read operation\n");
		size = 16;
	}

	err = drm_dp_dpcd_read(&phytium_dp->aux, p, kbuf, size);
	if (err <= 0) {
		DRM_ERROR("dpcd read (%zu bytes at h0x%04lx) failed (%zd)\n",
		size, p, err);
		kfree(kbuf);
		return err;
	}

	if (copy_to_user(buf, kbuf, size)) {
		kfree(kbuf);
		return -EFAULT;
	}

	kfree(kbuf);
	*ppos += size;
	return err;
}

static ssize_t
phytium_dpcd_write(struct file *filp,
				 const char __user *ubuf,
				 size_t len,
				 loff_t *ppos)
{
	struct seq_file *m = filp->private_data;
	struct drm_connector *connector = m->private;
	struct phytium_dp_device *phytium_dp = connector_to_dp_device(connector);
	
	unsigned long p = *ppos;
	char *tmp;
	ssize_t ret;

	if (connector->status != connector_status_connected)
		return -ENODEV;

	tmp = kmalloc(len, GFP_KERNEL);
	if (!tmp)
		return -ENOMEM;

	if (len > 16) {
		DRM_INFO("Supports a maximum of 16 bytes per write operation\n");
		len = 16;
	}

	memset(tmp, 0, len);
	if (copy_from_user(tmp, ubuf, len)) {
		kfree(tmp);
		return -EFAULT;
	}

	ret = drm_dp_dpcd_write(&phytium_dp->aux, p, tmp, len);

	if (ret < 0) 
		DRM_ERROR("write dpcd fail: ret:%ld\n", ret);
	kfree(tmp);
	*ppos += ret;

	return ret;
}

static ssize_t phytium_read_dpcd(struct phytium_dp_device *phytium_dp, struct dpcd_ioctl_data *read)
{
	ssize_t err;
	size_t max_read = 16;
	size_t copied = 0;
	char *kbuf = NULL;

	if (read->size > max_read) {
		kbuf = kmalloc(read->size, GFP_KERNEL);
		if (!kbuf)
			return -ENOMEM;
		}
	else {
		kbuf = kmalloc(max_read, GFP_KERNEL);
		if (!kbuf)
			return -ENOMEM;
		}

	while (copied < read->size) {
		ssize_t read_size = min(read->size - copied, max_read);

		err = drm_dp_dpcd_read(&phytium_dp->aux, read->address, kbuf, read_size);
		if (err <= 0) {
			DRM_ERROR("dpcd read (%zu bytes at 0x%04lx) failed (%zd)\n",
					  read_size, read->address, err);
			goto out;
		}

		if (copy_to_user(read->buffer + copied, kbuf, read_size)) {
			err = -EFAULT;
			goto out;
		}

		copied += read_size;
		read->address += read_size;
	}

	err = copied;

out:
	kfree(kbuf);
	return err;
}

static ssize_t phytium_write_dpcd(struct phytium_dp_device *phytium_dp, struct dpcd_ioctl_data *write)
{
	ssize_t ret;

	ret = drm_dp_dpcd_write(&phytium_dp->aux, write->address, &(write->w_data), write->size);

	if (ret < 0) 
		DRM_ERROR("write dpcd 0x%04lx fail: ret:%ld\n", write->address, ret);

	return ret;
}

static long phytium_dpcd_ioctl (struct file *filep, unsigned int cmd, unsigned long arg)
{
	struct seq_file *m = filep->private_data;
	struct drm_connector *connector = m->private;
	struct phytium_dp_device *phytium_dp = connector_to_dp_device(connector);

	struct dpcd_ioctl_data data;
	long err;
	void __user *argp = (void __user *) arg;

	if (connector->status != connector_status_connected)
		return -ENODEV;

	switch(cmd) {
	case DP_READ_DPCD:
		if (copy_from_user(&data, argp, sizeof(struct dpcd_ioctl_data)))
			return -EFAULT;

		err = phytium_read_dpcd(phytium_dp, &data);

		if (copy_to_user(argp, &data, sizeof(struct dpcd_ioctl_data)))
			return -EFAULT;
		break;

	case DP_WRITE_DPCD:
		if (copy_from_user(&data, argp, sizeof(struct dpcd_ioctl_data)))
			return -EFAULT;

		err = phytium_write_dpcd(phytium_dp, &data);

		if (copy_to_user(argp, &data, sizeof(struct dpcd_ioctl_data)))
			return -EFAULT;
		break;

	default:
		return -EINVAL;
	}

	return err;
}

static int phytium_dpcd_show_null(struct seq_file *m, void *data)
{
	return 0;
}

static int phytium_change_dpcd_open(struct inode *inode, struct file *file)
{
	return single_open(file, phytium_dpcd_show_null, inode->i_private);
}

static const struct file_operations phytium_change_dpcd_fops = {
	.owner = THIS_MODULE,
	.open = phytium_change_dpcd_open,
	.read = phytium_dpcd_read,
	.write = phytium_dpcd_write,
	.llseek = phytium_dpcd_llseek,
	.unlocked_ioctl = phytium_dpcd_ioctl,
};

static int phytium_dpcd_show(struct seq_file *m, void *data)
{
	struct drm_connector *connector = m->private;
	struct phytium_dp_device *phytium_dp = connector_to_dp_device(connector);
	uint8_t buf[16], i;
	ssize_t err;

	if (connector->status != connector_status_connected)
		return -ENODEV;

	for (i = 0; i < ARRAY_SIZE(phytium_dpcd_debug); i++) {
		const struct dpcd_block *b = &phytium_dpcd_debug[i];
		size_t size = b->end ? b->end - b->offset + 1 : (b->size ?: 1);

		if (WARN_ON(size > sizeof(buf)))
			continue;

		err = drm_dp_dpcd_read(&phytium_dp->aux, b->offset, buf, size);
		if (err <= 0) {
			DRM_ERROR("dpcd read (%zu bytes at %u) failed (%zd)\n",
				   size, b->offset, err);
			continue;
		}

		seq_printf(m, "%04x: %*ph\n", b->offset, (int) size, buf);
	}

	return 0;
}

static int phytium_dpcd_open(struct inode *inode, struct file *file)
{
	return single_open(file, phytium_dpcd_show, inode->i_private);
}

static const struct file_operations phytium_dpcd_fops = {
	.owner = THIS_MODULE,
	.open = phytium_dpcd_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static ssize_t
phytium_dp_state_write(struct file *filp,
				 const char __user *ubuf,
				 size_t len,
				 loff_t *ppos)
{
	char tmp[16];

	if (len >= sizeof(tmp))
		return -EINVAL;

	memset(tmp, 0, sizeof(tmp));
	if (copy_from_user(tmp, ubuf, len))
		return -EFAULT;
	tmp[len] = '\0';

	return len;
}

static int phytium_dp_state_show(struct seq_file *m, void *data)
{
	struct drm_connector *connector = m->private;
	struct phytium_dp_device *phytium_dp = connector_to_dp_device(connector);

	seq_printf(m, "port number: %d\n", phytium_dp->port);
	if (!phytium_dp->source_rates) {
		seq_printf(m, "dp-%d hw is not init\n", phytium_dp->port);
		return 0;
	}
	seq_printf(m, "source_max_lane_count: %d\n", phytium_dp->source_max_lane_count);
	seq_printf(m, "max_source_rates: %d\n",
		   phytium_dp->source_rates[phytium_dp->num_source_rates-1]);
	if (connector->status == connector_status_connected) {
		seq_printf(m, "dp-%d connected\n", phytium_dp->port);
		seq_printf(m, "sink_max_lane_count: %d\n", phytium_dp->sink_max_lane_count);
		seq_printf(m, "max_sink_rates: %d\n",
			   phytium_dp->sink_rates[phytium_dp->num_sink_rates-1]);
		seq_printf(m, "max_pass_link_rate: %d\n", phytium_dp->max_pass_link_rate);
		seq_printf(m, "max_pass link_lane_count: %d\n", phytium_dp->max_pass_lane_count);
		seq_printf(m, "link_rate: %d\n", phytium_dp->link_rate);
		seq_printf(m, "link_lane_count: %d\n", phytium_dp->link_lane_count);
		seq_printf(m, "train_set[0]: %d\n", phytium_dp->train_set[0]);
		seq_printf(m, "has_audio: %s\n", phytium_dp->has_audio?"yes":"no");
	} else
		seq_printf(m, "dp-%d disconnected\n", phytium_dp->port);

	return 0;
}

static int phytium_dp_state_open(struct inode *inode, struct file *file)
{
	return single_open(file, phytium_dp_state_show, inode->i_private);
}

static const struct file_operations phytium_dp_state_fops = {
	.owner = THIS_MODULE,
	.open = phytium_dp_state_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
	.write = phytium_dp_state_write,
};

static ssize_t
phytium_vrr_control(struct file *filp,
				 const char __user *ubuf,
				 size_t len,
				 loff_t *ppos)
{
	struct seq_file *m = filp->private_data;
	struct drm_connector *connector = m->private;
	struct phytium_dp_device *phytium_dp = connector_to_dp_device(connector);
	char tmp[16];
	int enable;
	int data;

	if (len >= sizeof(tmp))
		return -EINVAL;

	memset(tmp, 0, sizeof(tmp));
	if (copy_from_user(tmp, ubuf, len))
		return -EFAULT;
	tmp[len] = '\0';

	if (sscanf(tmp, "%d %d", &enable, &data) != 2)
		return -EINVAL;
#if DEBUG_LOG
	pr_info("enable is %d, data is %d\n", enable ? true : false, data);
#endif
	phytium_dp->vrr.sink_support = phytium_sink_supports_vrr(phytium_dp);
	phytium_change_fps(phytium_dp, enable, data);
	return len;
}

static int phytium_vrr_state_show(struct seq_file *m, void *data)
{
	struct drm_connector *connector = m->private;
	struct phytium_dp_device *phytium_dp = connector_to_dp_device(connector);
	int port = phytium_dp->port;
	struct drm_crtc *crtc = connector->state->crtc;
	struct drm_device *dev = phytium_dp->dev;
	struct ftd330_drm_private *priv = dev->dev_private;
	struct ftd330_dc *dc = dev_get_drvdata(priv->dc_dev);
	struct ftd330_crtc *ftd330_crtc = to_ftd330_crtc(crtc);
	struct dc_hw *hw = &dc->hw;
	uint32_t dp_group_offset = priv->dp_reg_base[port];
	bool vrr_support = false;

	u32 dc_group_offset = display_get_addr_offset(hw, ftd330_crtc->id);
	vrr_support = phytium_sink_supports_vrr(phytium_dp);

	if (vrr_support)
		seq_printf(m, "panel supports VRR, Adjustment range %d - %d\n",
					phytium_dp->vrr.range->min_vfreq,
					phytium_dp->vrr.range->max_vfreq);
	else
		seq_puts(m, "panel does not support VRR\n");

	seq_printf(m, "VRR state is %s, current refresh rate is %d\n",
				phytium_dp->vrr.enabled ? "enabled":"disabled",
				phytium_dp->freq);
	seq_puts(m, "dc registers show:\n");

	seq_printf(m, "addr:h0x%08x	h0x%08x\n", DC_DISPLAY_V,
				readl(hw->reg_base + dc_group_offset + DC_DISPLAY_V));
	seq_printf(m, "addr:h0x%08x	h0x%08x\n", DC_DISPLAY_V_SYNC,
				readl(hw->reg_base + dc_group_offset + DC_DISPLAY_V_SYNC));

	seq_puts(m, "dp registers show:\n");
	seq_printf(m, "addr:h0x%08x	h0x%08x\n", PHYTIUM_DP_MAIN_LINK_VTOTAL,
		phytium_readl_reg(priv, dp_group_offset, PHYTIUM_DP_MAIN_LINK_VTOTAL));
	seq_printf(m, "addr:h0x%08x	h0x%08x\n", PHYTIUM_DP_MAIN_LINK_VRES,
		phytium_readl_reg(priv, dp_group_offset, PHYTIUM_DP_MAIN_LINK_VRES));
	seq_printf(m, "addr:h0x%08x	h0x%08x\n", PHYTIUM_DP_MAIN_LINK_VSWIDTH,
		phytium_readl_reg(priv, dp_group_offset, PHYTIUM_DP_MAIN_LINK_VSWIDTH));
	seq_printf(m, "addr:h0x%08x	h0x%08x\n", PHYTIUM_DP_MAIN_LINK_VSTART,
		phytium_readl_reg(priv, dp_group_offset, PHYTIUM_DP_MAIN_LINK_VSTART));


	return 0;
}

static int phytium_vrr_state_open(struct inode *inode, struct file *file)
{
	return single_open(file, phytium_vrr_state_show, inode->i_private);
}

static const struct file_operations phytium_vrr_state_fops = {
	.owner = THIS_MODULE,
	.open = phytium_vrr_state_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
	.write = phytium_vrr_control,
};

static ssize_t
phytium_psr_control(struct file *filp,
				 const char __user *ubuf,
				 size_t len,
				 loff_t *ppos)
{
	struct seq_file *m = filp->private_data;
	struct drm_connector *connector = m->private;
	struct phytium_dp_device *phytium_dp = connector_to_dp_device(connector);
	struct drm_device *dev = phytium_dp->dev;
	struct ftd330_drm_private *priv = dev->dev_private;
	int port = phytium_dp->port;
	uint32_t dplp_offset = priv->dplp_reg_base[port];
	uint32_t val;
	char tmp[16];

	if (len >= sizeof(tmp))
		return -EINVAL;

	memset(tmp, 0, sizeof(tmp));
	if (copy_from_user(tmp, ubuf, len))
		return -EFAULT;
	tmp[len] = '\0';

	if (strcmp(tmp, "enter\n") == 0) {
		/* Exit dplp psr active state */
		val = phytium_dplp_read_reg(priv, dplp_offset, PHYTIUM_DPLP_FRAME_COMP_STATE);
		if (val & DPLP_PSR_ACTIVE)
			phytium_dplp_write_reg(priv, EXIT_PSR_REQ, dplp_offset,
									PHYTIUM_DPLP_FRAME_COMP_REQ);
		/* Disable dplp psr func */
		val = phytium_dplp_read_reg(priv, dplp_offset, PHYTIUM_DPLP_FRAME_COMP_CFG);
		if (val & PSR_FUNC_EN)
			phytium_dplp_write_reg(priv, val&~(PSR_FUNC_EN),
						dplp_offset, PHYTIUM_DPLP_FRAME_COMP_CFG);
		/* Disable dp psr */
		if (phytium_dp->psr.enabled) {
			phytium_psr_disable(phytium_dp);
		}
		phytium_psr_enable(phytium_dp);
	}
	else if (strcmp(tmp, "update\n") == 0)
		phytium_psr_sf_update(phytium_dp);
	else if (strcmp(tmp, "exit\n") == 0)
		phytium_psr_disable(phytium_dp);
	else if (strcmp(tmp, "auto\n") == 0) {
		val = phytium_dplp_read_reg(priv, dplp_offset, PHYTIUM_DPLP_FRAME_COMP_CFG);
		phytium_dplp_write_reg(priv, val|PSR_FUNC_EN,
						dplp_offset, PHYTIUM_DPLP_FRAME_COMP_CFG);
	}

	return len;
}

static int phytium_psr_state_show(struct seq_file *m, void *data)
{
	struct drm_connector *connector = m->private;
	struct phytium_dp_device *phytium_dp = connector_to_dp_device(connector);
	struct drm_device *dev =  phytium_dp->dev;
	struct ftd330_drm_private *priv = dev->dev_private;
	int port = phytium_dp->port;
	uint32_t group_offset = priv->dp_reg_base[port];
	uint32_t dplp_offset = priv->dplp_reg_base[port];
	uint32_t config;
	uint8_t status;
	int i, val, ret;
	uint8_t edp_dpcd[EDP_DISPLAY_CTL_CAP_SIZE], psr_dpcd[EDP_PSR_RECEIVER_CAP_SIZE];

	ret = drm_dp_dpcd_read(&phytium_dp->aux, DP_EDP_DPCD_REV, edp_dpcd, sizeof(edp_dpcd));
		if (ret < 0) {
			DRM_ERROR("port %d get PSR capability fail\n", phytium_dp->port);
			return -1;
		}

	ret = drm_dp_dpcd_read(&phytium_dp->aux, DP_PSR_SUPPORT, psr_dpcd, sizeof(psr_dpcd));
	if (ret < 0) {
		DRM_ERROR("port %d get PSR capability fail\n", phytium_dp->port);
		return -1;
	}

	if (!(edp_dpcd[1] & DP_EDP_SET_POWER_CAP)) {
		seq_puts(m, "Panel lacks power state control, PSR cannot be enabled\n");
		return 0;
	}

	if (!psr_dpcd[0]) {
		seq_puts(m, "eDP panel does not support PSR\n");
		return 0;
	}
	seq_printf(m, "eDP panel supports PSR version %d\n", psr_dpcd[0]);

	val = phytium_dplp_read_reg(priv, dplp_offset, PHYTIUM_DPLP_FRAME_COMP_CFG);
		if (val & PSR_FUNC_EN)
			seq_puts(m, "Currently in psr auto mode\n");
		else
			seq_puts(m, "Currently in psr manual mode\n");

	seq_puts(m, "source state show:\n");
	config = phytium_readl_reg(priv, group_offset, SEC_PSR_STATE_INTERNAL);
	for (i = 0; i < ARRAY_SIZE(source_psr_status_map); i++) {
		if (source_psr_status_map[i].status == config) {
			seq_printf(m, "%s\n", source_psr_status_map[i].status_info);
			break;
		}
	}
	config = phytium_readl_reg(priv, group_offset, PHYTIUM_DP_SEC_PACKET_COUNT);
	seq_printf(m, "Send SDP nums: %d\n", config);

	seq_puts(m, "sink state show:\n");
	val = drm_dp_dpcd_readb(&phytium_dp->aux, DP_PSR_STATUS, &status);
	if (val < 0)
		DRM_ERROR("PSR_STATUS read failed ret=%d", val);
	for (i = 0; i < ARRAY_SIZE(sink_psr_status_map); i++) {
		if (sink_psr_status_map[i].status == status) {
			seq_printf(m, "%s\n", sink_psr_status_map[i].status_info);
			break;
		}
	}
	val = drm_dp_dpcd_readb(&phytium_dp->aux, DP_LAST_RECEIVED_PSR_SDP, &status);
	if (val < 0)
		DRM_ERROR("PSR_STATUS read failed ret=%d", val);
	seq_printf(m, "Sink LAST RECEIVED PSR SDP: 0X%02x \n", status);

	seq_printf(m, "Panel PSR is%s reliable\n", phytium_dp->psr.sink_not_reliable ?
		" not" : "");

	return 0;
}

static int phytium_psr_state_open(struct inode *inode, struct file *file)
{
	return single_open(file, phytium_psr_state_show, inode->i_private);
}

static const struct file_operations phytium_psr_state_fops = {
	.owner = THIS_MODULE,
	.open = phytium_psr_state_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
	.write = phytium_psr_control,
};

static ssize_t
phytium_dplp_cnt_config(struct file *filp,
				 const char __user *ubuf,
				 size_t len,
				 loff_t *ppos)
{
	struct seq_file *m = filp->private_data;
	struct drm_connector *connector = m->private;
	struct phytium_dp_device *phytium_dp = connector_to_dp_device(connector);
	struct drm_device *dev =  phytium_dp->dev;
	struct ftd330_drm_private *priv = dev->dev_private;
	int port = phytium_dp->port;
	uint32_t dplp_offset = priv->dplp_reg_base[port];
	char tmp[16], func[16];
	uint32_t data, val;

	if (len >= sizeof(tmp))
		return -EINVAL;

	memset(tmp, 0, sizeof(tmp));
	if (copy_from_user(tmp, ubuf, len))
		return -EFAULT;
	tmp[len] = '\0';

	if (sscanf(tmp, "%s %d", func, &data) != 2)
		return -EINVAL;

	if (data < 2) {
		DRM_ERROR("The value of CNT must be greater than or equal to 2\n");
		return -EINVAL;
	}

	if (strcmp(func, "lowfps") == 0) {
		val = phytium_dplp_read_reg(priv, dplp_offset, PHYTIUM_DPLP_PSR_CNT);
		if (data > val)
			DRM_ERROR("The value setting of lowfps cnt must be less than psr cnt %d\n", val);
		else
			phytium_dplp_write_reg(priv, data, dplp_offset, PHYTIUM_DPLP_LOWFPS_CNT);
	}
	else if (strcmp(func, "psr") == 0) {
		val = phytium_dplp_read_reg(priv, dplp_offset, PHYTIUM_DPLP_LOWFPS_CNT);
		if (data <= val)
			DRM_ERROR("The value setting of psr cnt must greater than lowfps cnt %d\n", val);
		else
			phytium_dplp_write_reg(priv, data, dplp_offset, PHYTIUM_DPLP_PSR_CNT);
	}
	return len;
}

static int phytium_dplp_cnt_config_show(struct seq_file *m, void *data)
{
	struct drm_connector *connector = m->private;
	struct phytium_dp_device *phytium_dp = connector_to_dp_device(connector);
	struct drm_device *dev =  phytium_dp->dev;
	struct ftd330_drm_private *priv = dev->dev_private;
	int port = phytium_dp->port, i;
	uint32_t dplp_offset = priv->dplp_reg_base[port];
	uint32_t val;

	seq_puts(m, "dplp LOWFPS CNT show:\n");
	val = phytium_dplp_read_reg(priv, dplp_offset, PHYTIUM_DPLP_LOWFPS_CNT);
	seq_printf(m, "Enter lowfps state after %d identical frames\n", val);

	seq_puts(m, "dplp PSR CNT show:\n");
	val = phytium_dplp_read_reg(priv, dplp_offset, PHYTIUM_DPLP_PSR_CNT);
	seq_printf(m, "Enter psr state after %d identical frames\n", val);

	seq_puts(m, "dplp frame comp state show:\n");
	val = phytium_dplp_read_reg(priv, dplp_offset, PHYTIUM_DPLP_FRAME_COMP_STATE);
	for (i = 0; i < ARRAY_SIZE(dplp_frame_comp_status_map); i++) {
		if (dplp_frame_comp_status_map[i].status == val) {
			seq_printf(m, "%s\n", dplp_frame_comp_status_map[i].status_info);
			break;
		}
	}
	return 0;
}

static int phytium_dplp_cnt_config_open(struct inode *inode, struct file *file)
{
	return single_open(file, phytium_dplp_cnt_config_show, inode->i_private);
}

static const struct file_operations phytium_dplp_cnt_config_fops = {
	.owner = THIS_MODULE,
	.open = phytium_dplp_cnt_config_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
	.write = phytium_dplp_cnt_config,
};

static int phytium_dp_current_mode_bpc_show(struct seq_file *m, void *data)
{
	struct drm_connector *connector = m->private;
	struct phytium_dp_device *phytium_dp = connector_to_dp_device(connector);
	struct drm_crtc *crtc = NULL;
	struct drm_display_mode *mode = NULL;
	struct phytium_display_mode *phytium_mode = NULL;
	struct drm_device *dev = phytium_dp->dev;
	struct ftd330_drm_private *priv = dev->dev_private;
	struct ftd330_crtc *ftd330_crtc = NULL;
	struct ftd330_dc *dc = NULL;
	struct dc_hw *hw = NULL;
	u32 dc_group_offset = 0;

	if (!connector->state || !connector->state->crtc)
		return 0;
	crtc = connector->state->crtc;

	if (!priv->dc_dev)
		return 0;
	dc = dev_get_drvdata(priv->dc_dev);
	hw = &dc->hw;
	ftd330_crtc = to_ftd330_crtc(crtc);
	dc_group_offset = display_get_addr_offset(hw, ftd330_crtc->id);

	if (!list_empty(&phytium_dp->phytium_mode.list)) {
		list_for_each_entry(phytium_mode, &phytium_dp->phytium_mode.list, list) {
			if (phytium_mode->reduced_bpc)
				seq_printf(m, "Mode %dx%d-%dHz (clock %d) using reduced bpc %d\n",
							phytium_mode->h_active,
							phytium_mode->v_active,
							phytium_mode->freq,
							phytium_mode->clock,
							phytium_mode->bpc);
		}
	} else {
		seq_puts(m, "No mode using reduced bpc\n");
	}

	if (crtc) {
		mode = &crtc->state->mode;
		if (mode)
			seq_printf(m, "Current mode %dx%d-%dHz (clk %d) bpc is %d\n",
						mode->hdisplay, mode->vdisplay, drm_mode_vrefresh(mode),
						mode->clock, phytium_dp->bpc);
		else
			seq_puts(m, "No mode currently set\n");
	} else
		seq_puts(m, "No mode currently set\n");

	seq_printf(m, "DC_DISPLAY_DPI_CONFIG addr:h0x%08x	h0x%08x\n", DC_DISPLAY_DPI_CONFIG,
			readl(hw->reg_base + dc_group_offset + DC_DISPLAY_DPI_CONFIG));

	return 0;
}

static int phytium_dp_current_mode_bpc_open(struct inode *inode, struct file *file)
{
	return single_open(file, phytium_dp_current_mode_bpc_show, inode->i_private);
}

static const struct file_operations phytium_current_mode_bpc = {
	.owner = THIS_MODULE,
	.open = phytium_dp_current_mode_bpc_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static int phytium_custom_para_info_show(struct seq_file *m, void *data)
{
	struct drm_connector *connector = m->private;
	struct phytium_dp_device *phytium_dp = connector_to_dp_device(connector);
	struct drm_device *dev =  phytium_dp->dev;
	struct ftd330_drm_private *priv = dev->dev_private;
	struct bios_table_info *bios_info = &priv->info.bios_info;
	int i, j;
	int len = 0;
	char buf[256];

	seq_puts(m, "dp_mgnfs_val:\n");
	for (i = 0; i < 4; i++) {
		seq_printf(m, "link rate index %d:\n", i);
		for (j = 0; j < 4; j++) {
			seq_printf(m, "0x%x 0x%x 0x%x 0x%x\n",
					phytium_dp->phy_matrix.dp_mgnfs_val[i][j][0],
					phytium_dp->phy_matrix.dp_mgnfs_val[i][j][1],
					phytium_dp->phy_matrix.dp_mgnfs_val[i][j][2],
					phytium_dp->phy_matrix.dp_mgnfs_val[i][j][3]);
		}
	}
	seq_puts(m, "\n");

	seq_puts(m, "dp_cpost_val:\n");
	for (i = 0; i < 4; i++) {
		seq_printf(m, "link rate index %d:\n", i);
		for (j = 0; j < 4; j++) {
			seq_printf(m, "0x%x 0x%x 0x%x 0x%x\n",
					phytium_dp->phy_matrix.dp_cpost_val[i][j][0],
					phytium_dp->phy_matrix.dp_cpost_val[i][j][1],
					phytium_dp->phy_matrix.dp_cpost_val[i][j][2],
					phytium_dp->phy_matrix.dp_cpost_val[i][j][3]);
		}
	}
	seq_puts(m, "\n");

	seq_puts(m, "edp backlight map:\n");
	seq_puts(m, "edp bright to pwm mapping table\n");
	for (i = 0; i < 101; i++) {
		if (i % 10 == 0)
			len = 0;

		len += snprintf(buf + len, sizeof(buf) - len, "%d ",
						phytium_dp->edp_bl_map.edp_bright_to_pwm[i]);

		if ((i + 1) % 10 == 0) {
			seq_printf(m, "%s\n", buf);
		}
	}
	seq_puts(m, "edp pwm to bright mapping table\n");
	for (i = 0; i < 101; i++) {
		if (i % 10 == 0)
			len = 0;

		len += snprintf(buf + len, sizeof(buf) - len, "%d ",
						phytium_dp->edp_bl_map.edp_pwm_to_bright[i]);

		if ((i + 1) % 10 == 0) {
			seq_printf(m, "%s\n", buf);
		}
	}
	seq_puts(m, "\n");

	seq_puts(m, "edp panel timing:\n");
	seq_printf(m, "panel power up delay is %d ms\n",
					phytium_dp->panel.edp_panel_time.panel_power_up_delay);
	seq_printf(m, "backlight on delay is %d ms\n",
					phytium_dp->panel.edp_panel_time.backlight_on_delay);
	seq_printf(m, "backlight off delay is %d ms\n",
					phytium_dp->panel.edp_panel_time.backlight_off_delay);
	seq_printf(m, "panel power down delay is %d ms\n",
					phytium_dp->panel.edp_panel_time.panel_power_down_delay);
	seq_printf(m, "panel power cycle delay is %d ms\n",
					phytium_dp->panel.edp_panel_time.panel_power_cycle_delay);
	seq_puts(m, "\n");

	if (bios_info->panel_count) {
		seq_puts(m, "panel compatibility information:\n");
		for (i = 0; i < bios_info->panel_count; i++) {
			seq_printf(m, "panel id 0x%x, table valid is %d\n"
				"max link rate is 0x%x, max lane count is %d\n",
				bios_info->panels[i].panel_id, bios_info->panels[i].valid,
				bios_info->panels[i].max_link_rate, bios_info->panels[i].max_lane_count);
		}
	}
	return 0;
}

static int phytium_custom_para_info_open(struct inode *inode, struct file *file)
{
	return single_open(file, phytium_custom_para_info_show, inode->i_private);
}

static const struct file_operations phytium_custom_para_info_fops = {
	.owner = THIS_MODULE,
	.open = phytium_custom_para_info_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static const struct phytium_debugfs_files {
	const char *name;
	const struct file_operations *fops;
} phytium_debugfs_connector_files[] = {
	{"dp_state", &phytium_dp_state_fops},
	{"dpcd", &phytium_dpcd_fops},
	{"change_dpcd", &phytium_change_dpcd_fops},
	{"dp_register", &phytium_dp_register_fops},
	{"dp_trigger_train_fail", &phytium_dp_trigger_train_fail_fops},
	{"vrr_state", &phytium_vrr_state_fops},
	{"dplp_cnt_config", &phytium_dplp_cnt_config_fops},
	{"current_mode_bpc", &phytium_current_mode_bpc},
	{"custom_para_info", &phytium_custom_para_info_fops},
};

static const struct phytium_debugfs_files phytium_edp_debugfs_connector_files[] = {
	{"edp_power", &phytium_edp_power_fops},
	{"edp_backlight", &phytium_edp_backlight_fops},
	{"psr_state", &phytium_psr_state_fops},
};

int phytium_dp_debugfs_connector_add(struct drm_connector *connector)
{
	struct dentry *root = connector->debugfs_entry;
	struct dentry *ent;
	int i;
	struct phytium_dp_device *phytium_dp = connector_to_dp_device(connector);

	if (!root)
		return -ENODEV;

	for (i = 0; i < ARRAY_SIZE(phytium_debugfs_connector_files); i++) {
		ent = debugfs_create_file(phytium_debugfs_connector_files[i].name,
					  0644,
					  root,
					  connector,
					  phytium_debugfs_connector_files[i].fops);
		if (!ent)
			return -ENOMEM;
	}

	if (phytium_dp->is_edp)
		for (i = 0; i < ARRAY_SIZE(phytium_edp_debugfs_connector_files); i++) {
			ent = debugfs_create_file(phytium_edp_debugfs_connector_files[i].name,
						  0644,
						  root,
						  connector,
						  phytium_edp_debugfs_connector_files[i].fops);
			if (!ent)
				return -ENOMEM;
		}

	return 0;
}
#if SHOW_MEM_STATE
static int phytium_mem_state_show(struct seq_file *m, void *data)
{
	struct ftd330_drm_private *priv = m->private;
	uint8_t i;

	for (i = 0; i < ARRAY_SIZE(mem_state); i++)
		seq_printf(m, "%-34s %10lld\n", mem_state[i], priv->mem_state[i]);

	return 0;
}

static int phytium_mem_state_open(struct inode *inode, struct file *file)
{
	return single_open(file, phytium_mem_state_show, inode->i_private);
}

static const struct file_operations phytium_mem_state_fops = {
	.owner = THIS_MODULE,
	.open = phytium_mem_state_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static const struct phytium_debugfs_files phytium_debugfs_display_files[] = {
	{"mem_state", &phytium_mem_state_fops},
};

int phytium_dp_debugfs_display_register(struct ftd330_drm_private *priv)
{
	struct drm_minor *minor = priv->dev->primary;
	struct dentry *root = minor->debugfs_root;
	struct dentry *ent;

	if (!root)
		return -ENODEV;

	ent = debugfs_create_file(phytium_debugfs_display_files[0].name,
				  0644,
				  root,
				  priv,
				  phytium_debugfs_display_files[0].fops);
	if (!ent)
		return -ENOMEM;

	return 0;
}
#endif
