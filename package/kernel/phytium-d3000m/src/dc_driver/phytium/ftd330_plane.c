// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2025, Phytium Technology Co., Ltd.
 */
#include <linux/file.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 18)
#include <linux/debugfs.h>
#endif
#include <drm/drm_atomic.h>
#include <drm/drm_atomic_helper.h>
#include <drm/drm_plane_helper.h>
#include <drm/drm_drv.h>
#include <drm/drm_framebuffer.h>
#include <drm/drm_blend.h>
#if KERNEL_VERSION(6, 1, 0) > LINUX_VERSION_CODE
#include <drm/drm_fb_cma_helper.h>
#include <drm/drm_gem_cma_helper.h>
#else
#include <drm/drm_fb_dma_helper.h>
#include <drm/drm_gem_dma_helper.h>
#endif
#include <drm/ftd330_drm.h>
#include <drm/ftd330_drm_fourcc.h>
#include "ftd330_crtc.h"
#include "ftd330_fb.h"
#include "ftd330_gem.h"
#include "ftd330_plane.h"
#include "ftd330_type.h"
#define fourcc_mod_ftd330_get_type(val) (((val) & DRM_FORMAT_MOD_FTD330_TYPE_MASK) >> 53)

void ftd330_plane_destory(struct drm_plane *plane)
{
	struct ftd330_plane *ftd330_plane = to_ftd330_plane(plane);

	drm_plane_cleanup(plane);
	kfree(ftd330_plane);
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 18)
static void ftd330_atomic_helper_plane_reset(struct drm_plane *plane,
				     struct drm_plane_state *state)
{
	state->plane = plane;
	state->rotation = DRM_MODE_ROTATE_0;

	state->alpha = DRM_BLEND_ALPHA_OPAQUE;

	plane->state = state;
}
#endif

static void ftd330_plane_reset(struct drm_plane *plane)
{
	struct ftd330_plane_state *state;
	struct ftd330_plane *ftd330_plane = to_ftd330_plane(plane);
	u32 i;

	if (plane->state) {
		__drm_atomic_helper_plane_destroy_state(plane->state);

		state = to_ftd330_plane_state(plane->state);
		drm_property_blob_put(state->y2r_coef);
		drm_property_blob_put(state->lut_3d);
		drm_property_blob_put(state->pvric_clear);
		drm_property_blob_put(state->pvric_const);

		if (state->fb_ext)
			drm_framebuffer_put(state->fb_ext);
		for (i = 0; i < ftd330_plane->properties.num; i++) {
			if (state->drm_states[i].proto->type == FTD330_DC_PROPERTY_BLOB)
				drm_property_blob_put(state->drm_states[i].value.blob);
		}
		kfree(state);
		plane->state = NULL;
	}

	state = kzalloc(sizeof(*state), GFP_KERNEL);
	if (state == NULL)
		return;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 18)
	__drm_atomic_helper_plane_reset(plane, &state->base);
#else
	ftd330_atomic_helper_plane_reset(plane, &state->base);
#endif

	state->base.color_encoding = DRM_COLOR_YCBCR_BT2020;
	state->base.zpos = ftd330_plane->id;

	for (i = 0; i < ftd330_plane->properties.num; i++)
		state->drm_states[i].proto = ftd330_plane->properties.items[i].proto;

	memset(&state->status, 0, sizeof(state->status));
}

static void _ftd330_plane_duplicate_blob(struct ftd330_plane_state *state, struct ftd330_plane_state *ori_state)
{
	state->watermark = ori_state->watermark;
	state->y2r_coef = ori_state->y2r_coef;
	state->lut_3d = ori_state->lut_3d;
	state->pvric_clear = ori_state->pvric_clear;
	state->pvric_const = ori_state->pvric_const;

	if (state->watermark)
		drm_property_blob_get(state->watermark);
	if (state->y2r_coef)
		drm_property_blob_get(state->y2r_coef);
	if (state->lut_3d)
		drm_property_blob_get(state->lut_3d);
	if (state->pvric_clear)
		drm_property_blob_get(state->pvric_clear);
	if (state->pvric_const)
		drm_property_blob_get(state->pvric_const);
}

static int _ftd330_plane_set_property_blob_from_id(struct drm_device *dev,
					       struct drm_property_blob **blob, uint64_t blob_id,
					       size_t expected_size, bool *changed)
{
	struct drm_property_blob *new_blob = NULL;
	bool data_changed = false;

	if (blob_id) {
		new_blob = drm_property_lookup_blob(dev, blob_id);
		if (new_blob == NULL)
			return -EINVAL;

		if (new_blob->length != expected_size) {
			drm_property_blob_put(new_blob);
			return -EINVAL;
		}
	}

	if ((*blob) && blob_id) {
		if (memcmp(new_blob->data, (*blob)->data, expected_size) == 0) {
			drm_property_blob_put(new_blob);
			if (changed)
				*changed = false;
			return 0;
		}
	}
	data_changed = drm_property_replace_blob(blob, new_blob);
	if (changed)
		*changed = data_changed;

	drm_property_blob_put(new_blob);

	return 0;
}

static struct drm_plane_state *ftd330_plane_atomic_duplicate_state(struct drm_plane *plane)
{
	struct ftd330_plane_state *ori_state;
	struct ftd330_plane_state *state;
	const struct ftd330_plane *ftd330_plane = to_ftd330_plane(plane);

	if (WARN_ON(!plane->state))
		return NULL;

	ori_state = to_ftd330_plane_state(plane->state);
	state = kzalloc(sizeof(*state), GFP_KERNEL);
	if (!state)
		return NULL;

	__drm_atomic_helper_plane_duplicate_state(plane, &state->base);

	state->lut_3d_changed = false;

	_ftd330_plane_duplicate_blob(state, ori_state);
	memcpy(&state->status, &ori_state->status, sizeof(ori_state->status));

#ifdef CONFIG_DEBUG_FS
	state->qos = ori_state->qos;
#endif
	/* dc properties */
	ftd330_dc_duplicate_drm_properties(state->drm_states, ori_state->drm_states,
				       &ftd330_plane->properties);
	return &state->base;
}

static void ftd330_plane_atomic_destroy_state(struct drm_plane *plane, struct drm_plane_state *state)
{
	struct ftd330_plane_state *ftd330_plane_state = to_ftd330_plane_state(state);
	struct ftd330_plane *ftd330_plane = to_ftd330_plane(plane);

	__drm_atomic_helper_plane_destroy_state(state);

	if (ftd330_plane_state->fb_ext)
		drm_framebuffer_put(ftd330_plane_state->fb_ext);
	drm_property_blob_put(ftd330_plane_state->watermark);
	drm_property_blob_put(ftd330_plane_state->y2r_coef);
	drm_property_blob_put(ftd330_plane_state->lut_3d);
	drm_property_blob_put(ftd330_plane_state->pvric_clear);
	drm_property_blob_put(ftd330_plane_state->pvric_const);

	/* dc properties */
	ftd330_dc_destroy_drm_properties(ftd330_plane_state->drm_states, &ftd330_plane->properties);
	kfree(ftd330_plane_state);
}

static int ftd330_plane_atomic_set_property(struct drm_plane *plane, struct drm_plane_state *state,
					struct drm_property *property, uint64_t val)
{
	struct drm_device *dev = plane->dev;
	struct ftd330_plane *ftd330_plane = to_ftd330_plane(plane);
	struct drm_minor *minor = container_of(&dev, struct drm_minor, dev);
	struct drm_file *file_priv = container_of(&minor, struct drm_file, minor);
	struct ftd330_plane_state *ftd330_plane_state = to_ftd330_plane_state(state);
	int ret = 0;


	if (property == ftd330_plane->ext_layer_fb) {
		ftd330_plane_state->fb_ext =
			drm_framebuffer_lookup(plane->dev, file_priv, (u32)(val & 0xFFFFFFFF));
	} else if (property == ftd330_plane->watermark_prop) {
		ret = _ftd330_plane_set_property_blob_from_id(dev, &ftd330_plane_state->watermark, val,
							  sizeof(struct drm_ftd330_watermark), NULL);
	} else if (property == ftd330_plane->y2r_prop) {
		ret = _ftd330_plane_set_property_blob_from_id(dev, &ftd330_plane_state->y2r_coef, val,
							  sizeof(struct drm_ftd330_y2r_config), NULL);
	} else if (property == ftd330_plane->lut_3d_prop) {
		ret = _ftd330_plane_set_property_blob_from_id(dev, &ftd330_plane_state->lut_3d, val,
							  sizeof(struct drm_ftd330_data_block),
							  &ftd330_plane_state->lut_3d_changed);
	} else if (property == ftd330_plane->pvric_clear_prop) {
		ret = _ftd330_plane_set_property_blob_from_id(dev, &ftd330_plane_state->pvric_clear, val,
							  sizeof(struct drm_ftd330_pvric_clear),
							  &ftd330_plane_state->pvric_color_changed);
	} else if (property == ftd330_plane->pvric_const_prop) {
		ret = _ftd330_plane_set_property_blob_from_id(dev, &ftd330_plane_state->pvric_const, val,
							  sizeof(struct drm_ftd330_pvric_const),
							  &ftd330_plane_state->pvric_color_changed);
	} else {
		/* dc property */
		ret = ftd330_dc_set_drm_property(dev, ftd330_plane_state->drm_states,
			&ftd330_plane->properties, property, val);
	}
	/* dc property */
	ret = ftd330_dc_set_drm_property(dev, ftd330_plane_state->drm_states,
			&ftd330_plane->properties, property, val);

	return ret;
}

static int ftd330_plane_atomic_get_property(struct drm_plane *plane,
					const struct drm_plane_state *state,
					struct drm_property *property, uint64_t *val)
{
	struct ftd330_plane *ftd330_plane = to_ftd330_plane(plane);
	const struct ftd330_plane_state *ftd330_plane_state =
		container_of(state, const struct ftd330_plane_state, base);

	if (property == ftd330_plane->ext_layer_fb)
		*val = (ftd330_plane_state->fb_ext) ? ftd330_plane_state->fb_ext->base.id : 0;
	else if (property == ftd330_plane->watermark_prop)
		*val = (ftd330_plane_state->watermark) ? ftd330_plane_state->watermark->base.id : 0;
	else if (property == ftd330_plane->y2r_prop)
		*val = (ftd330_plane_state->y2r_coef) ? ftd330_plane_state->y2r_coef->base.id : 0;
	else if (property == ftd330_plane->lut_3d_prop)
		*val = (ftd330_plane_state->lut_3d) ? ftd330_plane_state->lut_3d->base.id : 0;
	else if (property == ftd330_plane->pvric_clear_prop)
		*val = (ftd330_plane_state->pvric_clear) ? ftd330_plane_state->pvric_clear->base.id : 0;
	else if (property == ftd330_plane->pvric_const_prop)
		*val = (ftd330_plane_state->pvric_const) ? ftd330_plane_state->pvric_const->base.id : 0;
	else
		return ftd330_dc_get_drm_property(ftd330_plane_state->drm_states, &ftd330_plane->properties,
					property, val);
	return 0;
}

static bool ftd330_plane_format_mod_supported(struct drm_plane *plane, u32 format, u64 modifier)
{
	struct ftd330_plane *ftd330_plane = to_ftd330_plane(plane);

	if (WARN_ON(modifier == DRM_FORMAT_MOD_INVALID))
		return false;

	if (ftd330_plane->funcs && ftd330_plane->funcs->format_mod_support)
		return ftd330_plane->funcs->format_mod_support(ftd330_plane, format, modifier);
	else
		return true;
}

#ifdef CONFIG_DEBUG_FS
static int ftd330_plane_pattern_show(struct seq_file *s, void *data)
{
	struct drm_plane *plane = s->private;
	struct ftd330_plane_state *plane_state = to_ftd330_plane_state(plane->state);

	seq_printf(s, "plane[%u]: %s\n", plane->base.id, plane->name);

	seq_printf(s, "\tenable: %d\n", plane_state->pattern.enable);
	seq_printf(s, "\tmode setting instructions:\n"
				"\t\tmode\t\tid\n"
				"\t\tPURE_CLR\t0\n"
				"\t\tCLR_BAR_H\t1\n"
				"\t\tCLR_BAR_V\t2\n"
				"\t\tRMAP_H\t\t3\n"
				"\t\tRMAP_V\t\t4\n"
				"\t\tBLK_WHT_H\t5\n"
				"\t\tBLK_WHT_V\t6\n"
				"\t\tBLK_WHT_S\t7\n"
				"\t\tBORDER\t\t8\n"
				"\t\tCURSOR\t\t9\n");
	seq_printf(s, "\tmode-id = %d\n", plane_state->pattern.mode);
	seq_printf(s, "\tcurser = {%d,%d}\n", plane_state->pattern.rect.x,
		   plane_state->pattern.rect.y);
	seq_printf(s, "\twidth = %d\n", plane_state->pattern.rect.w);
	seq_printf(s, "\theight = %d\n", plane_state->pattern.rect.h);
	seq_printf(s, "\tcolor = 0x%llx\n", plane_state->pattern.color);

	return 0;
}

static int ftd330_plane_pattern_open(struct inode *inode, struct file *file)
{
	return single_open(file, ftd330_plane_pattern_show, inode->i_private);
}

static ssize_t ftd330_plane_pattern_write(struct file *file, const char __user *ubuf, size_t len,
				      loff_t *offp)
{
	struct seq_file *s = file->private_data;
	struct drm_plane *plane = s->private;
	struct drm_device *drm_dev = plane->dev;
	struct ftd330_plane *ftd330_plane = to_ftd330_plane(plane);
	struct ftd330_plane_state *plane_state = to_ftd330_plane_state(plane->state);
	struct ftd330_drm_private *priv = NULL;
	char buf[96], *cur = buf;
	unsigned long value;
	unsigned long long color_val;

	if (!drm_dev)
		return -ENXIO;

	priv = drm_dev->dev_private;
	if (!priv || !priv->dc_dev)
		return -ENXIO;

	if (!ftd330_plane->funcs->set_pattern)
		return -EINVAL;

	if (len > sizeof(buf) - 1)
		return -EINVAL;

	if (copy_from_user(buf, ubuf, len))
		return -EFAULT;

	buf[len] = '\0';

	cur = strstr(buf, "enable:");
	if (cur) {
		cur += 7;
		if (kstrtoul(cur, 10, &value))
			return -EINVAL;
		plane_state->pattern.enable = value;
	} else {
		return -EINVAL;
	}

	cur = strstr(buf, "mode:");
	if (cur) {
		cur += 5;
		if (kstrtoul(cur, 10, &value))
			return -EINVAL;
		plane_state->pattern.mode = value;
	}

	cur = strstr(buf, "size.x:");
	if (cur) {
		cur += 7;
		if (kstrtoul(cur, 10, &value))
			return -EINVAL;
		plane_state->pattern.rect.x = value;
	}

	cur = strstr(buf, "size.y:");
	if (cur) {
		cur += 7;
		if (kstrtoul(cur, 10, &value))
			return -EINVAL;
		plane_state->pattern.rect.y = value;
	}

	cur = strstr(buf, "size.w:");
	if (cur) {
		cur += 7;
		if (kstrtoul(cur, 10, &value))
			return -EINVAL;
		plane_state->pattern.rect.w = value;
	}

	cur = strstr(buf, "size.h:");
	if (cur) {
		cur += 7;
		if (kstrtoul(cur, 10, &value))
			return -EINVAL;
		plane_state->pattern.rect.h = value;
	}

	cur = strstr(buf, "color:0x");
	if (cur) {
		cur += 8;
		if (kstrtoull(cur, 16, &color_val))
			return -EINVAL;

		plane_state->pattern.color = color_val;
	}

	ftd330_plane->funcs->set_pattern(priv->dc_dev, ftd330_plane);

	return len;
}

static int ftd330_plane_crc_show(struct seq_file *s, void *data)
{
	struct drm_plane *plane = s->private;
	struct ftd330_plane_state *plane_state = to_ftd330_plane_state(plane->state);

	seq_printf(s, "plane[%u]: %s\n", plane->base.id, plane->name);

	seq_printf(s, "\tenable: %d\n", plane_state->crc.enable);
	seq_printf(s, "\tpos setting instructions:\n"
				"\t\tpos\t\tid\n"
				"\t\tDFC\t\t0\n"
				"\t\tHDR\t\t1\n");
	seq_printf(s, "\tpos-id = %d\n", plane_state->crc.pos);
	seq_printf(s, "\talpha-seed= 0x%x\n", plane_state->crc.seed.a);
	seq_printf(s, "\tred-seed= 0x%x\n", plane_state->crc.seed.r);
	seq_printf(s, "\tgreen-seed= 0x%x\n", plane_state->crc.seed.g);
	seq_printf(s, "\tblue-seed= 0x%x\n", plane_state->crc.seed.b);

	seq_printf(s, "\talpha-crc= 0x%x\n", plane_state->crc.result.a);
	seq_printf(s, "\tred-crc= 0x%x\n", plane_state->crc.result.r);
	seq_printf(s, "\tgreen-crc= 0x%x\n", plane_state->crc.result.g);
	seq_printf(s, "\tblue-crc= 0x%x\n", plane_state->crc.result.b);

	return 0;
}

static int ftd330_plane_crc_open(struct inode *inode, struct file *file)
{
	return single_open(file, ftd330_plane_crc_show, inode->i_private);
}

static ssize_t ftd330_plane_crc_write(struct file *file, const char __user *ubuf, size_t len,
				  loff_t *offp)
{
	struct seq_file *s = file->private_data;
	struct drm_plane *plane = s->private;
	struct drm_device *drm_dev = plane->dev;
	struct ftd330_plane *ftd330_plane = to_ftd330_plane(plane);
	struct ftd330_plane_state *plane_state = to_ftd330_plane_state(plane->state);
	struct ftd330_drm_private *priv = NULL;
	char buf[120], *cur = buf;
	unsigned long value;

	if (!drm_dev)
		return -ENXIO;

	priv = drm_dev->dev_private;
	if (!priv || !priv->dc_dev)
		return -ENXIO;

	if (!ftd330_plane->funcs->set_crc)
		return -EINVAL;

	if (len > sizeof(buf) - 1)
		return -EINVAL;

	if (copy_from_user(buf, ubuf, len))
		return -EFAULT;

	buf[len] = '\0';

	cur = strstr(buf, "enable:");
	if (cur) {
		cur += 7;
		if (kstrtoul(cur, 10, &value))
			return -EINVAL;
		plane_state->crc.enable = value;
	} else {
		return -EINVAL;
	}

	cur = strstr(buf, "pos:");
	if (cur) {
		cur += 4;
		if (kstrtoul(cur, 10, &value))
			return -EINVAL;
		plane_state->crc.pos = value;
	}

	cur = strstr(buf, "a-seed:0x");
	if (cur) {
		cur += 9;
		if (kstrtoul(cur, 16, &value))
			return -EINVAL;
		plane_state->crc.seed.a = value;
	}

	cur = strstr(buf, "r-seed:0x");
	if (cur) {
		cur += 9;
		if (kstrtoul(cur, 16, &value))
			return -EINVAL;
		plane_state->crc.seed.r = value;
	}

	cur = strstr(buf, "g-seed:0x");
	if (cur) {
		cur += 9;
		if (kstrtoul(cur, 16, &value))
			return -EINVAL;
		plane_state->crc.seed.g = value;
	}

	cur = strstr(buf, "b-seed:0x");
	if (cur) {
		cur += 9;
		if (kstrtoul(cur, 16, &value))
			return -EINVAL;
		plane_state->crc.seed.b = value;
	}

	cur = strstr(buf, "initMode:");
	if (cur) {
		cur += 9;
		if (kstrtoul(cur, 10, &value))
			return -EINVAL;
		plane_state->crc.init_mode = value;
	}
	cur = strstr(buf, "initValue:0x");
	if (cur) {
		cur += 12;
		if (kstrtoul(cur, 16, &value))
			return -EINVAL;
		plane_state->crc.init_value = value;
	}
	cur = strstr(buf, "xorValue:0x");
	if (cur) {
		cur += 11;
		if (kstrtoul(cur, 16, &value))
			return -EINVAL;
		plane_state->crc.xor_value = value;
	}
	cur = strstr(buf, "golden0:0x");
	if (cur) {
		cur += 10;
		if (kstrtoul(cur, 16, &value))
			return -EINVAL;
		plane_state->crc.golden[0] = value;
	}
	cur = strstr(buf, "golden1:0x");
	if (cur) {
		cur += 10;
		if (kstrtoul(cur, 16, &value))
			return -EINVAL;
		plane_state->crc.golden[1] = value;
	}
	cur = strstr(buf, "golden2:0x");
	if (cur) {
		cur += 10;
		if (kstrtoul(cur, 16, &value))
			return -EINVAL;
		plane_state->crc.golden[2] = value;
	}
	ftd330_plane->funcs->set_crc(priv->dc_dev, ftd330_plane);

	return len;
}
static int ftd330_plane_qos_show(struct seq_file *s, void *data)
{
	struct drm_plane *plane = s->private;
	struct ftd330_plane *ftd330_plane = to_ftd330_plane(plane);

	if (ftd330_plane->funcs->show_qos)
		return ftd330_plane->funcs->show_qos(s);
	return 0;
}
static int ftd330_plane_qos_open(struct inode *inode, struct file *file)
{
	return single_open(file, ftd330_plane_qos_show, inode->i_private);
}
static ssize_t ftd330_plane_qos_write(struct file *file, const char __user *ubuf, size_t len,
				  loff_t *offp)
{
	struct seq_file *s = file->private_data;
	struct drm_plane *plane = s->private;
	struct ftd330_plane *ftd330_plane = to_ftd330_plane(plane);

	if (ftd330_plane->funcs->set_qos)
		ftd330_plane->funcs->set_qos(ftd330_plane->base.dev->dev, ftd330_plane, ubuf, len);
	return len;
}
static const struct file_operations ftd330_plane_qos_fops = {
	.open = ftd330_plane_qos_open,
	.read = seq_read,
	.write = ftd330_plane_qos_write,
	.llseek = seq_lseek,
	.release = single_release,
};

static const struct file_operations ftd330_plane_pattern_fops = {
	.open = ftd330_plane_pattern_open,
	.read = seq_read,
	.write = ftd330_plane_pattern_write,
	.llseek = seq_lseek,
	.release = single_release,
};

static const struct file_operations ftd330_plane_crc_fops = {
	.open = ftd330_plane_crc_open,
	.read = seq_read,
	.write = ftd330_plane_crc_write,
	.llseek = seq_lseek,
	.release = single_release,
};
static int ftd330_plane_debugfs_init(struct drm_plane *plane)
{
	struct ftd330_plane *ftd330_plane = to_ftd330_plane(plane);
	char *name;

	name = kasprintf(GFP_KERNEL, "plane-%d", plane->index);
	if (!name)
		return 0;

	ftd330_plane->debugfs_entry = debugfs_create_dir(name, plane->dev->primary->debugfs_root);

	kfree(name);

	debugfs_create_file("pattern", 0644, ftd330_plane->debugfs_entry, plane,
			    &ftd330_plane_pattern_fops);
	debugfs_create_file("CRC", 0644, ftd330_plane->debugfs_entry, plane, &ftd330_plane_crc_fops);
	debugfs_create_file("QOS", 0644, ftd330_plane->debugfs_entry, plane, &ftd330_plane_qos_fops);

	return 0;
}
#else
static int ftd330_plane_debugfs_init(struct drm_plane *plane)
{
	return 0;
}
#endif /* CONFIG_DEBUG_FS */

static int ftd330_plane_late_register(struct drm_plane *plane)
{
	return ftd330_plane_debugfs_init(plane);
}

static void ftd330_plane_early_unregister(struct drm_plane *plane)
{
	struct ftd330_plane *ftd330_plane = to_ftd330_plane(plane);

	debugfs_remove_recursive(ftd330_plane->debugfs_entry);
}

const struct drm_plane_funcs ftd330_plane_funcs = {
	.update_plane = drm_atomic_helper_update_plane,
	.disable_plane = drm_atomic_helper_disable_plane,
	.late_register = ftd330_plane_late_register,
	.early_unregister = ftd330_plane_early_unregister,
	.destroy = ftd330_plane_destory,
	.reset = ftd330_plane_reset,
	.atomic_duplicate_state = ftd330_plane_atomic_duplicate_state,
	.atomic_destroy_state = ftd330_plane_atomic_destroy_state,
	.atomic_set_property = ftd330_plane_atomic_set_property,
	.atomic_get_property = ftd330_plane_atomic_get_property,
	.format_mod_supported = ftd330_plane_format_mod_supported,
};

static int ftd330_plane_atomic_check(struct drm_plane *plane,
#if KERNEL_VERSION(5, 13, 0) > LINUX_VERSION_CODE
				 struct drm_plane_state *state)
#else
				 struct drm_atomic_state *atomic_state)
#endif
{
	struct ftd330_plane *ftd330_plane = to_ftd330_plane(plane);
#if KERNEL_VERSION(5, 13, 0) <= LINUX_VERSION_CODE
	struct drm_plane_state *state = drm_atomic_get_new_plane_state(atomic_state, plane);
#endif
	struct drm_framebuffer *fb = state->fb;
	struct drm_crtc *crtc = state->crtc;
	struct ftd330_crtc *ftd330_crtc = to_ftd330_crtc(crtc);

	if (!crtc || !fb)
		return 0;

	return ftd330_plane->funcs->check(ftd330_crtc->dev, ftd330_plane, state);
}

static void ftd330_plane_atomic_update(struct drm_plane *plane,
#if KERNEL_VERSION(5, 13, 0) > LINUX_VERSION_CODE
				   struct drm_plane_state *old_state)
#else
				   struct drm_atomic_state *atomic_state)
#endif
{
	unsigned char i, num_planes;
	struct drm_framebuffer *fb;
	struct ftd330_plane *ftd330_plane = to_ftd330_plane(plane);
	struct drm_plane_state *state = plane->state;
	struct ftd330_crtc *ftd330_crtc = to_ftd330_crtc(state->crtc);
	struct ftd330_plane_state *plane_state = to_ftd330_plane_state(state);
#if KERNEL_VERSION(5, 14, 0) > LINUX_VERSION_CODE
	struct drm_format_name_buf *name = &plane_state->status.format_name;
#endif
	if (!state->fb || !state->crtc || !state->visible)
		return;

	fb = state->fb;

	num_planes = fb->format->num_planes;

	for (i = 0; i < num_planes; i++) {
		struct ftd330_gem_object *ftd330_obj;

		ftd330_obj = ftd330_fb_get_gem_obj(fb, i);
		ftd330_plane->dma_addr[i] = ftd330_obj->iova + fb->offsets[i];
		if ((fourcc_mod_ftd330_get_type(fb->modifier) == DRM_FORMAT_MOD_FTD330_TYPE_COMPRESSED) ||
		    fb->modifier == DRM_FORMAT_MOD_PHYTIUM_SUPER_TILED_FC)
			ftd330_plane_get_dec_tile_status(plane->dev, ftd330_obj, i, &ftd330_plane->ts_addr[i],
						     &ftd330_plane->ts_dma_buf[i]);
	}

	plane_state->status.src = drm_plane_state_src(state);
	plane_state->status.dest = drm_plane_state_dest(state);
#if KERNEL_VERSION(5, 14, 0) > LINUX_VERSION_CODE
	drm_get_format_name(fb->format->format, name);
#endif
	ftd330_plane->funcs->update(ftd330_crtc->dev, ftd330_plane);
}

static void ftd330_plane_atomic_disable(struct drm_plane *plane,
#if KERNEL_VERSION(5, 13, 0) > LINUX_VERSION_CODE
				    struct drm_plane_state *old_state)
#else
				    struct drm_atomic_state *atomic_state)
#endif
{
	struct ftd330_plane *ftd330_plane = to_ftd330_plane(plane);
	struct ftd330_drm_private *priv = plane->dev->dev_private;
#if KERNEL_VERSION(5, 13, 0) <= LINUX_VERSION_CODE
	struct drm_plane_state *old_state =
		atomic_state ? drm_atomic_get_old_plane_state(atomic_state, plane) : NULL;
#endif

	ftd330_plane->funcs->disable(priv->dc_dev, ftd330_plane, old_state);
}

const struct drm_plane_helper_funcs ftd330_plane_helper_funcs = {
	.atomic_check = ftd330_plane_atomic_check,
	.atomic_update = ftd330_plane_atomic_update,
	.atomic_disable = ftd330_plane_atomic_disable,
};

void ftd330_plane_get_dec_tile_status(struct drm_device *dev, struct ftd330_gem_object *ftd330_gem,
				  u8 plane_id, dma_addr_t *ts_addr, void **ts_dma_buf)
{
	struct drm_gem_object *obj;
	struct ftd330_gem_object *ftd330_obj;
	struct ftd330_ts_metadata *metadata;
	struct dma_buf *dma_buf;
	u64 addr_align = PHYTIUM_TILE_STATUS_BUFFER_ALIGNMENT;

	/* Return while not the import DMA-buf */
	if (!ftd330_gem->base.dma_buf)
		return;

	if (plane_id > 2) {
		pr_err("%s: The plane ID[%d] out of range.\n", __func__, plane_id);
		return;
	}

	metadata = ftd330_gem->base.dma_buf->priv;
	dma_buf = metadata->plane[plane_id].ts_dma_buf;
	if (IS_ERR(dma_buf)) {
		pr_err("%s: Tile status dma buf get fail.\n", __func__);
		return;
	}

	if (ftd330_gem->sub_gem) {
		ftd330_obj = ftd330_gem->sub_gem;
		*ts_addr = ftd330_obj->iova + metadata->plane[plane_id].ts_offset;
		*ts_dma_buf = metadata->plane[plane_id].ts_dma_buf;
		return;
	}

	if (dev->driver->gem_prime_import)
		obj = dev->driver->gem_prime_import(dev, dma_buf);
	else
		obj = drm_gem_prime_import(dev, dma_buf);

	if (IS_ERR(obj)) {
		pr_err("%s: tile status dma buf gem_prime_import fail.\n", __func__);
		return;
	}

	ftd330_obj = container_of(obj, struct ftd330_gem_object, base);
	*ts_addr = ftd330_obj->iova + metadata->plane[plane_id].ts_offset;
	*ts_dma_buf = metadata->plane[plane_id].ts_dma_buf;

	ftd330_gem->sub_gem = ftd330_obj;

	if ((*ts_addr + 128) % addr_align) {
		pr_err("%s: The ts base addr should align with %lld.\n", __func__, addr_align);
		return;
	}
}


struct ftd330_plane *ftd330_plane_create(const struct dc_hw_plane *hw_plane, struct drm_device *drm_dev,
				 const struct ftd330_dc_info *info, u8 index,
				 unsigned int possible_crtcs,
				 const struct ftd330_plane_funcs *dc_plane_funcs)
{
	struct ftd330_plane *plane;
	struct ftd330_plane_info *plane_info = NULL;
	int ret, i = 0, n = 0;
	u64 *supported_modifiers;
	const u64 *modifiers;

	if (!info)
		return NULL;

	plane_info = (struct ftd330_plane_info *)&info->planes[index];
	if (!plane_info)
		return NULL;

	plane = kzalloc(sizeof(struct ftd330_plane), GFP_KERNEL);
	if (!plane)
		return NULL;

	modifiers = plane_info->modifiers;
	supported_modifiers = kcalloc(plane_info->num_modifiers * 2, sizeof(u64), GFP_KERNEL);
	if (!supported_modifiers)
		return NULL;
	for (i = 0; i < plane_info->num_modifiers; i++) {
		supported_modifiers[n++] = *modifiers;
		if (fourcc_mod_is_vendor(*modifiers, PHY)) {
			supported_modifiers[n++] = (*modifiers) |
						   (DRM_FORMAT_MOD_FTD330_CUSTOM_FORMAT_ENABLE);
		}
		modifiers++;
	}
	plane->funcs = dc_plane_funcs;


	ret = drm_universal_plane_init(drm_dev, &plane->base, possible_crtcs, &ftd330_plane_funcs,
				       plane_info->formats, plane_info->num_formats,
				       supported_modifiers, plane_info->type,
				       plane_info->name ? plane_info->name : NULL);
	if (ret)
		goto err_free_plane;

	drm_plane_helper_add(&plane->base, &ftd330_plane_helper_funcs);

	/* Set up the plane properties */

	/* Standard properties */
	if (plane_info->rotation) {
		ret = drm_plane_create_rotation_property(&plane->base, DRM_MODE_ROTATE_0,
							 plane_info->rotation);
		if (ret)
			goto error_cleanup_plane;
	}

	if (plane_info->blend_mode) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 18)
		ret = drm_plane_create_blend_mode_property(&plane->base, plane_info->blend_mode);
		if (ret)
			goto error_cleanup_plane;
#endif
		ret = drm_plane_create_alpha_property(&plane->base);
		if (ret)
			goto error_cleanup_plane;
	}

	if (plane_info->color_encoding) {
		ret = drm_plane_create_color_properties(&plane->base, plane_info->color_encoding,
							plane_info->color_range,
							DRM_COLOR_YCBCR_BT2020,
							DRM_COLOR_YCBCR_LIMITED_RANGE);
		if (ret)
			goto error_cleanup_plane;

		/* For user-defined Y2R conversion coefficients */
		if (plane_info->program_csc) {
			plane->y2r_prop =
				drm_property_create(drm_dev, DRM_MODE_PROP_BLOB, "Y2R_CONFIG", 0);
			if (!plane->y2r_prop)
				goto error_cleanup_plane;

			drm_object_attach_property(&plane->base.base, plane->y2r_prop, 0);
		}
	}

	if (plane_info->zpos != 255) {
		ret = drm_plane_create_zpos_property(&plane->base, plane_info->zpos, 0,
						     info->max_blend_layer - 1);
		if (ret)
			goto error_cleanup_plane;
	}
#if KERNEL_VERSION(5, 8, 0) <= LINUX_VERSION_CODE
	else {
		ret = drm_plane_create_zpos_immutable_property(&plane->base, plane_info->zpos);
		if (ret)
			goto error_cleanup_plane;
	}
#endif

	/* Private properties */
	if (plane_info->layer_ext || plane_info->layer_ext_ex) {
		plane->ext_layer_fb = drm_property_create_object(
			drm_dev, DRM_MODE_PROP_ATOMIC, "EXT_LAYER_FB", DRM_MODE_OBJECT_FB);
		if (!plane->ext_layer_fb)
			goto error_cleanup_plane;

		drm_object_attach_property(&plane->base.base, plane->ext_layer_fb, 0);
	}

	if (plane_info->watermark) {
		plane->watermark_prop =
			drm_property_create(drm_dev, DRM_MODE_PROP_BLOB, "WATERMARK", 0);
		if (!plane->watermark_prop)
			goto error_cleanup_plane;

		drm_object_attach_property(&plane->base.base, plane->watermark_prop, 0);
	}

	if (plane_info->cgm_lut) {
		plane->lut_3d_prop = drm_property_create(drm_dev, DRM_MODE_PROP_BLOB, "3DLUT", 0);
		if (!plane->lut_3d_prop)
			goto error_cleanup_plane;

		drm_object_attach_property(&plane->base.base, plane->lut_3d_prop, 0);
	}

#ifdef CONFIG_PHYTIUM_PVRIC
	if (info->cap_dec) {
		plane->pvric_clear_prop =
			drm_property_create(drm_dev, DRM_MODE_PROP_BLOB, "PVRIC_CLEAR", 0);
		if (!plane->pvric_clear_prop)
			goto error_cleanup_plane;

		drm_object_attach_property(&plane->base.base, plane->pvric_clear_prop, 0);

		plane->pvric_const_prop =
			drm_property_create(drm_dev, DRM_MODE_PROP_BLOB, "PVRIC_CONST", 0);
		if (!plane->pvric_const_prop)
			goto error_cleanup_plane;

		drm_object_attach_property(&plane->base.base, plane->pvric_const_prop, 0);
	}
#endif
	if (hw_plane != NULL &&
	    ftd330_dc_create_drm_properties(drm_dev, &plane->base.base, &hw_plane->states,
					&plane->properties)) {
		goto error_cleanup_plane;
	}


	kfree(supported_modifiers);
	return plane;

error_cleanup_plane:
	drm_plane_cleanup(&plane->base);
err_free_plane:
	kfree(plane);
	kfree(supported_modifiers);
	return NULL;
}
