/* SPDX-License-Identifier: GPL-2.0 */

/*
 * ftv310 driver debugfs interface to control/query vpu state.
 *
 * Copyright (C) 2024-2025, Phytium Technology Co., Ltd.
 */

#include "ftv310_vpu_debugfs.h"

extern unsigned long vram_used_size;

static int ftv310_vpu_power_state_show(struct seq_file *m, void *data)
{
	struct drm_device *drm_dev = m->private;
	struct device *dev = drm_dev->dev;
	vcmd_slice_str *slice = dev_get_drvdata(dev);

	seq_printf(m, "decoder power status: [%d]\n", slice->dec_power_status);
	seq_printf(m, "encoder power status: [%d]\n", slice->enc_power_status);

	return 0;
}

static int phy_vpu_power_debugfs_open(struct inode *inode, struct file *file)
{
	return single_open(file, ftv310_vpu_power_state_show, inode->i_private);
}

static const struct file_operations phy_vpu_power_status_debugfs_fops = {
	.open = phy_vpu_power_debugfs_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static int ftv310_vpu_voltage_state_show(struct seq_file *m, void *data)
{
	struct drm_device *drm_dev = m->private;
	struct device *dev = drm_dev->dev;
	vcmd_slice_str *slice = dev_get_drvdata(dev);

	seq_printf(m, "decoder voltage: [%lu]\n", slice->dec_voltage);
	seq_printf(m, "encoder voltage: [%lu]\n", slice->enc_voltage);
	return 0;
}

static int phy_vpu_voltage_debugfs_open(struct inode *inode, struct file *file)
{
	return single_open(file, ftv310_vpu_voltage_state_show, inode->i_private);
}

static const struct file_operations phy_vpu_voltage_debugfs_fops = {
	.open = phy_vpu_voltage_debugfs_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static int ftv310_vpu_mem_used_show(struct seq_file *m, void *data)
{
	unsigned long mem_used;
	
	mem_used = vram_used_size;
	seq_printf(m, "Memory has used: [%luMB]\n", mem_used / (0x100000));
	seq_printf(m, "Memory has used: [%luBytes]\n", mem_used);
	return 0;
}

static int phy_vpu_mem_used_debugfs_open(struct inode *inode, struct file *file)
{
	return single_open(file, ftv310_vpu_mem_used_show, inode->i_private);
}

static const struct file_operations phy_vpu_mem_used_debugfs_fops = {
	.open = phy_vpu_mem_used_debugfs_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

#if KERNEL_VERSION(5, 8, 0) <= LINUX_VERSION_CODE
void ftv310_vpu_debugfs_init(struct drm_minor *minor)
#else
int ftv310_vpu_debugfs_init(struct drm_minor *minor)
#endif
{
	struct dentry *root = minor->debugfs_root;
	struct drm_device *drm_dev = minor->dev;
	struct device *dev = drm_dev->dev;

	debugfs_create_file("power_status", 0444, root,
		drm_dev, &phy_vpu_power_status_debugfs_fops);

	if (has_acpi_companion(dev))
		debugfs_create_file("voltage", 0444, root, drm_dev, &phy_vpu_voltage_debugfs_fops);

	debugfs_create_file("mem_used", 0444, root,
		drm_dev, &phy_vpu_mem_used_debugfs_fops);

#if KERNEL_VERSION(5, 8, 0) > LINUX_VERSION_CODE
	return 0;
#endif
}
