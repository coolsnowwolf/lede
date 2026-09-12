// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2025, Phytium Technology Co., Ltd.
 */

#include <linux/clk.h>
#include <linux/debugfs.h>

#include <drm/drm_atomic.h>
#include <drm/drm_atomic_helper.h>
#include <drm/drm_crtc.h>

#include "ftd330_crtc.h"
#include "ftd330_gem.h"
#include "ftd330_dc.h"
#include "ftd330_dc_hw.h"
#include "ftd330_dc_property.h"
#include "ftd330_dc_drm_property.h"
#include "phytium_dp.h"
#include "phytium_dp_reg.h"

#if KERNEL_VERSION(5, 5, 0) <= LINUX_VERSION_CODE
#include <drm/drm_vblank.h>
#endif
bool ftd330_display_get_crtc_scanoutpos(struct drm_device *dev, unsigned int crtc_id,
				    bool in_vblank_irq, int *vpos, int *hpos, ktime_t *stime,
				    ktime_t *etime, const struct drm_display_mode *mode)
{
	struct drm_crtc *crtc = drm_crtc_from_index(dev, crtc_id);
	struct ftd330_crtc *ftd330_crtc = to_ftd330_crtc(crtc);
	u32 position;
	int vblank_lines;
	bool ret = false;

	/*
	 * While in vblank, position will be negative counting up
	 * towards 0 at vbl_end. And outside vblank, position will
	 * be positive counting up since vbl_end.
	 */
	if (!in_vblank_irq) {
		/* Get optional system timestamp before query. */
		if (stime)
			*stime = ktime_get();

		if (!ftd330_crtc->funcs->get_crtc_scanout_position) {
			/*
			 * Return a vpos of zero, which will cause calling code
			 * to just return the etime timestamp uncorrected.
			 * At least this is no worse than the standard fallback.
			 */
			DRM_DEBUG("[CRTC:%d:%s] get_crtc_scanout_position() isn't implemented!\n",
				  crtc->base.id, crtc->name);
			*hpos = *vpos = 0;
		} else {
			ret = ftd330_crtc->funcs->get_crtc_scanout_position(ftd330_crtc->dev, crtc,
									&position);
			if (ret != 0)
				return false;

			/* Decode into vertical and horizontal scanout position. */
			*hpos = position & 0xffff;
			*vpos = (position >> 16) & 0xffff;
		}

		/* Get optional system timestamp after query. */
		if (etime)
			*etime = ktime_get();
	} else {
		vblank_lines = mode->vtotal - mode->vdisplay;
		/*
		 * Assume the irq handler got called close to first
		 * line of vblank, so HW has about a full vblank
		 * scanlines to go, and as a base timestamp use the
		 * one taken at entry into vblank irq handler, so it
		 * is not affected by random delays due to lock
		 * contention on event_lock or vblank_time lock in
		 * the core.
		 */
		*hpos = 0;
		*vpos = -vblank_lines;

		if (stime)
			*stime = ftd330_crtc->t_vblank;
		if (etime)
			*etime = ftd330_crtc->t_vblank;
	}

	return true;
}

void ftd330_crtc_destroy(struct drm_crtc *crtc)
{
	struct ftd330_crtc *ftd330_crtc = to_ftd330_crtc(crtc);

	if (crtc->port)
		of_node_put(crtc->port);
	drm_crtc_cleanup(crtc);
	if (ftd330_crtc)
		kfree(ftd330_crtc);
}

static void ftd330_crtc_reset(struct drm_crtc *crtc)
{
	struct ftd330_crtc_state *state;
	struct ftd330_crtc *ftd330_crtc = to_ftd330_crtc(crtc);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 18))
	struct drm_crtc_state *crtc_state = NULL;
#endif
	u32 i;

	if (crtc->state) {
		__drm_atomic_helper_crtc_destroy_state(crtc->state);

		state = to_ftd330_crtc_state(crtc->state);
		drm_property_blob_put(state->prior_gamma);
		drm_property_blob_put(state->roi0_gamma);
		drm_property_blob_put(state->roi1_gamma);
		drm_property_blob_put(state->ltm_luma_get);
		drm_property_blob_put(state->ltm_cd_get);
		drm_property_blob_put(state->ltm_hist_get);
		drm_property_blob_put(state->hist_get);
		drm_property_blob_put(state->rgb_hist_get);

		for (i = 0; i < ftd330_crtc->properties.num; i++) {
			if (state->drm_states[i].proto->type == FTD330_DC_PROPERTY_BLOB)
				drm_property_blob_put(state->drm_states[i].value.blob);
		}

		if (state)
			kfree(state);
		crtc->state = NULL;
	}

	state = kzalloc(sizeof(*state), GFP_KERNEL);
	if (!state)
		return;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 18))
	__drm_atomic_helper_crtc_reset(crtc, &state->base);
#else
	crtc_state = &state->base;
	if (crtc_state)
		crtc_state->crtc = crtc;
	crtc->state = crtc_state;
#endif

	state->sync_mode = FTD330_SINGLE_DC;
	state->output_fmt = MEDIA_BUS_FMT_RGB888_1X24;
	state->encoder_type = DRM_MODE_ENCODER_NONE;
#ifdef CONFIG_PHYTIUM_MMU
	state->mmu_prefetch = FTD330_MMU_PREFETCH_DISABLE;
#endif

	for (i = 0; i < ftd330_crtc->properties.num; i++)
		state->drm_states[i].proto = ftd330_crtc->properties.items[i].proto;
}

static void _ftd330_crtc_duplicate_blob(struct ftd330_crtc_state *state, struct ftd330_crtc_state *ori_state)
{
	state->prior_gamma = ori_state->prior_gamma;
	state->roi0_gamma = ori_state->roi0_gamma;
	state->roi1_gamma = ori_state->roi1_gamma;
	state->ltm_luma_get = ori_state->ltm_luma_get;
	state->ltm_cd_get = ori_state->ltm_cd_get;
	state->ltm_hist_get = ori_state->ltm_hist_get;
	state->hist_get = ori_state->hist_get;
	state->rgb_hist_get = ori_state->rgb_hist_get;

	if (state->prior_gamma)
		drm_property_blob_get(state->prior_gamma);
	if (state->roi0_gamma)
		drm_property_blob_get(state->roi0_gamma);
	if (state->roi1_gamma)
		drm_property_blob_get(state->roi1_gamma);
	if (state->ltm_luma_get)
		drm_property_blob_get(state->ltm_luma_get);
	if (state->ltm_cd_get)
		drm_property_blob_get(state->ltm_cd_get);
	if (state->ltm_hist_get)
		drm_property_blob_get(state->ltm_hist_get);
	if (state->hist_get)
		drm_property_blob_get(state->hist_get);
	if (state->rgb_hist_get)
		drm_property_blob_get(state->rgb_hist_get);

	// Skip dc property
}

static int _ftd330_crtc_set_property_blob_from_id(struct drm_device *dev,
					      struct drm_property_blob **blob, uint64_t blob_id,
					      size_t expected_size, bool *changed)
{
	struct drm_property_blob *new_blob = NULL;
	bool data_changed = false;

	if (blob_id) {
		new_blob = drm_property_lookup_blob(dev, blob_id);
		if (!new_blob)
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

static struct drm_crtc_state *ftd330_crtc_atomic_duplicate_state(struct drm_crtc *crtc)
{
	struct ftd330_crtc_state *ori_state;
	struct ftd330_crtc_state *state;
	const struct ftd330_crtc *ftd330_crtc = to_ftd330_crtc_const(crtc);

	if (WARN_ON(!crtc->state))
		return NULL;

	ori_state = to_ftd330_crtc_state(crtc->state);
	state = kzalloc(sizeof(*state), GFP_KERNEL);
	if (!state)
		return NULL;

	__drm_atomic_helper_crtc_duplicate_state(crtc, &state->base);

	state->sync_mode = ori_state->sync_mode;
	state->output_fmt = ori_state->output_fmt;
	state->encoder_type = ori_state->encoder_type;
	state->bpp = ori_state->bpp;
	state->sync_enable = ori_state->sync_enable;
	state->underflow = ori_state->underflow;
	state->out_dp = ori_state->out_dp;
	state->prior_gamma_changed = false;
	state->roi0_gamma_changed = false;
	state->roi1_gamma_changed = false;
	state->ltm_luma_get_changed = false;
	state->ltm_cd_get_changed = false;
	state->ltm_hist_get_changed = false;
	state->hist_get_changed = false;
	state->rgb_hist_get_changed = false;

#ifdef CONFIG_PHYTIUM_MMU
	state->mmu_prefetch = ori_state->mmu_prefetch;
#endif

#ifdef CONFIG_DEBUG_FS
	state->qos = ori_state->qos;
#endif
	_ftd330_crtc_duplicate_blob(state, ori_state);

	/* dc properties */
	ftd330_dc_duplicate_drm_properties(state->drm_states, ori_state->drm_states,
				       &ftd330_crtc->properties);

	return &state->base;
}

static void ftd330_crtc_atomic_destroy_state(struct drm_crtc *crtc, struct drm_crtc_state *state)
{
	struct ftd330_crtc_state *ftd330_crtc_state = to_ftd330_crtc_state(state);
	struct ftd330_crtc *ftd330_crtc = to_ftd330_crtc(crtc);

	__drm_atomic_helper_crtc_destroy_state(state);

	if (ftd330_crtc_state->rcd_mask)
		drm_framebuffer_put(ftd330_crtc_state->rcd_mask);
	if (ftd330_crtc_state->blur_mask)
		drm_framebuffer_put(ftd330_crtc_state->blur_mask);
	if (ftd330_crtc_state->brightness_mask)
		drm_framebuffer_put(ftd330_crtc_state->brightness_mask);
	drm_property_blob_put(ftd330_crtc_state->prior_gamma);
	drm_property_blob_put(ftd330_crtc_state->roi0_gamma);
	drm_property_blob_put(ftd330_crtc_state->roi1_gamma);
	drm_property_blob_put(ftd330_crtc_state->ltm_luma_get);
	drm_property_blob_put(ftd330_crtc_state->ltm_cd_get);
	drm_property_blob_put(ftd330_crtc_state->ltm_hist_get);
	drm_property_blob_put(ftd330_crtc_state->hist_get);
	drm_property_blob_put(ftd330_crtc_state->rgb_hist_get);

	/* dc properties */
	ftd330_dc_destroy_drm_properties(ftd330_crtc_state->drm_states, &ftd330_crtc->properties);
	if (ftd330_crtc_state)
		kfree(ftd330_crtc_state);
}

static int ftd330_crtc_atomic_set_property(struct drm_crtc *crtc, struct drm_crtc_state *state,
				       struct drm_property *property, uint64_t val)
{
	struct drm_device *dev = crtc->dev;
	struct ftd330_crtc *ftd330_crtc = to_ftd330_crtc(crtc);
	struct ftd330_crtc_state *ftd330_crtc_state = to_ftd330_crtc_state(state);
	int ret = 0;
	struct drm_minor *minor = container_of(&dev, struct drm_minor, dev);
	struct drm_file *file_priv = container_of(&minor, struct drm_file, minor);


	if (property == ftd330_crtc->sync_mode) {
		ftd330_crtc_state->sync_mode = val;
	} else if (property == ftd330_crtc->mmu_prefetch) {
		ftd330_crtc_state->mmu_prefetch = val;
	} else if (property == ftd330_crtc->panel_sync) {
		ftd330_crtc_state->sync_enable = val;
	} else if (property == ftd330_crtc->prior_gamma_prop) {
		ret = _ftd330_crtc_set_property_blob_from_id(dev, &ftd330_crtc_state->prior_gamma, val,
							 sizeof(struct drm_ftd330_gamma_lut),
							 &ftd330_crtc_state->prior_gamma_changed);
	} else if (property == ftd330_crtc->roi0_gamma_prop) {
		ret = _ftd330_crtc_set_property_blob_from_id(dev, &ftd330_crtc_state->roi0_gamma, val,
							 sizeof(struct drm_ftd330_gamma_lut),
							 &ftd330_crtc_state->roi0_gamma_changed);
	} else if (property == ftd330_crtc->roi1_gamma_prop) {
		ret = _ftd330_crtc_set_property_blob_from_id(dev, &ftd330_crtc_state->roi1_gamma, val,
							 sizeof(struct drm_ftd330_gamma_lut),
							 &ftd330_crtc_state->roi1_gamma_changed);
	} else if (property == ftd330_crtc->ltm_luma_get_prop) {
		ret = _ftd330_crtc_set_property_blob_from_id(dev, &ftd330_crtc_state->ltm_luma_get, val,
							 sizeof(struct drm_ftd330_ltm_luma_ave),
							 &ftd330_crtc_state->ltm_luma_get_changed);
	} else if (property == ftd330_crtc->ltm_cd_get_prop) {
		ret = _ftd330_crtc_set_property_blob_from_id(dev, &ftd330_crtc_state->ltm_cd_get, val,
							 sizeof(struct drm_ftd330_ltm_cd_get),
							 &ftd330_crtc_state->ltm_cd_get_changed);
	} else if (property == ftd330_crtc->ltm_hist_get_prop) {
		ret = _ftd330_crtc_set_property_blob_from_id(dev, &ftd330_crtc_state->ltm_hist_get, val,
							 sizeof(struct drm_ftd330_ltm_hist_get),
							 &ftd330_crtc_state->ltm_hist_get_changed);
	} else if (property == ftd330_crtc->hist_get_prop) {
		ret = _ftd330_crtc_set_property_blob_from_id(dev, &ftd330_crtc_state->hist_get, val,
							 sizeof(struct drm_ftd330_hist_get),
							 &ftd330_crtc_state->hist_get_changed);
	} else if (property == ftd330_crtc->rgb_hist_get_prop) {
		ret = _ftd330_crtc_set_property_blob_from_id(dev, &ftd330_crtc_state->rgb_hist_get, val,
							 sizeof(struct drm_ftd330_rgb_hist_get),
							 &ftd330_crtc_state->rgb_hist_get_changed);
	} else if (property == ftd330_crtc->rcd_mask_fb) {
		ftd330_crtc_state->rcd_mask =
			drm_framebuffer_lookup(crtc->dev, file_priv, (u32)(val & 0xFFFFFFFF));
	} else if (property == ftd330_crtc->blur_mask_fb) {
		ftd330_crtc_state->blur_mask =
			drm_framebuffer_lookup(crtc->dev, file_priv, (u32)(val & 0xFFFFFFFF));
	} else if (property == ftd330_crtc->brightness_mask_fb) {
		ftd330_crtc_state->brightness_mask =
			drm_framebuffer_lookup(crtc->dev, file_priv, (u32)(val & 0xFFFFFFFF));
	} else {
		/* dc property */
		ret = ftd330_dc_set_drm_property(dev, ftd330_crtc_state->drm_states, &ftd330_crtc->properties,
					     property, val);
	}

	return ret;
}

static int ftd330_crtc_atomic_get_property(struct drm_crtc *crtc, const struct drm_crtc_state *state,
				       struct drm_property *property, uint64_t *val)
{
	struct ftd330_crtc *ftd330_crtc = to_ftd330_crtc(crtc);
	const struct ftd330_crtc_state *ftd330_crtc_state =
		container_of(state, const struct ftd330_crtc_state, base);

	if (property == ftd330_crtc->sync_mode) {
		*val = ftd330_crtc_state->sync_mode;
	} else if (property == ftd330_crtc->mmu_prefetch) {
		*val = ftd330_crtc_state->mmu_prefetch;
	} else if (property == ftd330_crtc->panel_sync) {
		*val = ftd330_crtc_state->sync_enable;
	} else if (property == ftd330_crtc->prior_gamma_prop) {
		*val = (ftd330_crtc_state->prior_gamma) ? ftd330_crtc_state->prior_gamma->base.id : 0;
	} else if (property == ftd330_crtc->roi0_gamma_prop) {
		*val = (ftd330_crtc_state->roi0_gamma) ? ftd330_crtc_state->roi0_gamma->base.id : 0;
	} else if (property == ftd330_crtc->roi1_gamma_prop) {
		*val = (ftd330_crtc_state->roi1_gamma) ? ftd330_crtc_state->roi1_gamma->base.id : 0;
	} else if (property == ftd330_crtc->ltm_luma_get_prop) {
		*val = (ftd330_crtc_state->ltm_luma_get) ? ftd330_crtc_state->ltm_luma_get->base.id : 0;
	} else if (property == ftd330_crtc->ltm_cd_get_prop) {
		*val = (ftd330_crtc_state->ltm_cd_get) ? ftd330_crtc_state->ltm_cd_get->base.id : 0;
	} else if (property == ftd330_crtc->ltm_hist_get_prop) {
		*val = (ftd330_crtc_state->ltm_hist_get) ? ftd330_crtc_state->ltm_hist_get->base.id : 0;
	} else if (property == ftd330_crtc->hist_get_prop) {
		*val = (ftd330_crtc_state->hist_get) ? ftd330_crtc_state->hist_get->base.id : 0;
	} else if (property == ftd330_crtc->rgb_hist_get_prop) {
		*val = (ftd330_crtc_state->rgb_hist_get) ? ftd330_crtc_state->rgb_hist_get->base.id : 0;
	} else if (property == ftd330_crtc->rcd_mask_fb) {
		*val = (ftd330_crtc_state->rcd_mask) ? ftd330_crtc_state->rcd_mask->base.id : 0;
	} else if (property == ftd330_crtc->blur_mask_fb) {
		*val = (ftd330_crtc_state->blur_mask) ? ftd330_crtc_state->blur_mask->base.id : 0;
	} else if (property == ftd330_crtc->brightness_mask_fb) {
		*val = (ftd330_crtc_state->brightness_mask) ? ftd330_crtc_state->brightness_mask->base.id :
								0;
	} else {
		/* dc porperty */
		return ftd330_dc_get_drm_property(ftd330_crtc_state->drm_states, &ftd330_crtc->properties,
					      property, val);
	}
	return 0;
}

#ifdef CONFIG_DEBUG_FS
static int ftd330_crtc_debugfs_show(struct seq_file *s, void *data)
{
	struct drm_crtc *crtc = s->private;
	uint32_t crtc_index = drm_crtc_index(crtc);
	struct drm_device *drm_dev = crtc->dev;
	struct ftd330_drm_private *priv = drm_dev->dev_private;
	struct ftd330_crtc_state *crtc_state = to_ftd330_crtc_state(crtc->state);
	struct drm_display_mode *mode = &crtc->state->adjusted_mode;
	bool power_status = false;

	power_status = phytium_readl_reg(priv, priv->dp_reg_base[crtc_index], PHYTIUM_DP_SINK_HPD_STATE);
	seq_printf(s, "crtc[%u]: %s\n", crtc->base.id, crtc->name);
	seq_printf(s, "\tactive = %d\n", crtc->state->active);
	seq_printf(s, "\tsize = %dx%d\n", mode->hdisplay, mode->vdisplay);
	seq_printf(s, "\tbpp = %u\n", crtc_state->bpp);
	seq_printf(s, "\tunderflow = %d\n", crtc_state->underflow);
	seq_printf(s, "\tpower : %d\n", power_status);


	return 0;
}

static int ftd330_crtc_debugfs_open(struct inode *inode, struct file *file)
{
	return single_open(file, ftd330_crtc_debugfs_show, inode->i_private);
}

static const struct file_operations ftd330_crtc_debugfs_fops = {
	.open = ftd330_crtc_debugfs_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static int ftd330_crtc_pattern_show(struct seq_file *s, void *data)
{
	struct drm_crtc *crtc = s->private;
	struct ftd330_crtc *ftd330_crtc = to_ftd330_crtc(crtc);

	return ftd330_crtc->funcs->show_pattern_config(s);

}

static int ftd330_crtc_pattern_open(struct inode *inode, struct file *file)
{
	return single_open(file, ftd330_crtc_pattern_show, inode->i_private);
}

static ssize_t ftd330_crtc_pattern_write(struct file *file, const char __user *ubuf, size_t len,
				     loff_t *offp)
{
	struct seq_file *s = file->private_data;
	struct drm_crtc *crtc = s->private;
	struct ftd330_crtc *ftd330_crtc = to_ftd330_crtc(crtc);

	if (ftd330_crtc->funcs->set_pattern)
		ftd330_crtc->funcs->set_pattern(crtc, ubuf, len);

	return len;
}

static int ftd330_crtc_crc_show(struct seq_file *s, void *data)
{
	struct drm_crtc *crtc = s->private;
	struct ftd330_crtc *ftd330_crtc = to_ftd330_crtc(crtc);

	if (!ftd330_crtc->funcs->show_crc)
		return -EINVAL;

	return ftd330_crtc->funcs->show_crc(s);
}

static ssize_t ftd330_crtc_crc_write(struct file *file, const char __user *ubuf, size_t len,
				 loff_t *offp)
{
	struct seq_file *s = file->private_data;
	struct drm_crtc *crtc = s->private;
	struct ftd330_crtc *ftd330_crtc = to_ftd330_crtc(crtc);

	if (ftd330_crtc->funcs->set_crc)
		ftd330_crtc->funcs->set_crc(ftd330_crtc->dev, crtc, ubuf, len);

	return len;
}

static int ftd330_crtc_crc_open(struct inode *inode, struct file *file)
{
	return single_open(file, ftd330_crtc_crc_show, inode->i_private);
}

static int ftd330_crtc_qos_show(struct seq_file *s, void *data)
{
	struct drm_crtc *crtc = s->private;
	struct ftd330_crtc *ftd330_crtc = to_ftd330_crtc(crtc);

	if (ftd330_crtc->funcs->show_qos)
		return ftd330_crtc->funcs->show_qos(s);

	return 0;
}

static int ftd330_crtc_qos_open(struct inode *inode, struct file *file)
{
	return single_open(file, ftd330_crtc_qos_show, inode->i_private);
}

static ssize_t ftd330_crtc_qos_write(struct file *file, const char __user *ubuf, size_t len,
				 loff_t *offp)
{
	struct seq_file *s = file->private_data;
	struct drm_crtc *crtc = s->private;
	struct ftd330_crtc *ftd330_crtc = to_ftd330_crtc(crtc);

	if (ftd330_crtc->funcs->set_qos)
		ftd330_crtc->funcs->set_qos(ftd330_crtc->dev, crtc, ubuf, len);

	return len;
}


static int phytium_crtc_log_show(struct seq_file *s, void *data)
{
	struct drm_crtc *crtc = s->private;
	struct drm_device *drm_dev = crtc->dev;
	struct ftd330_drm_private *priv = drm_dev->dev_private;
	struct ftd330_dc *dc = dev_get_drvdata(priv->dc_dev);

	seq_printf(s, "log_enable : %d\n", dc->hw.phytium_log_enable);
	return 0;
}

static int phytium_crtc_log_open(struct inode *inode, struct file *file)
{
	return single_open(file, phytium_crtc_log_show, inode->i_private);
}

static ssize_t phytium_crtc_log_write(struct file *file, const char __user *ubuf, size_t len,
									loff_t *offp)
{
	struct seq_file *s = file->private_data;
	struct drm_crtc *crtc = s->private;
	struct drm_device *drm_dev = crtc->dev;
	struct ftd330_drm_private *priv = drm_dev->dev_private;
	struct ftd330_dc *dc = dev_get_drvdata(priv->dc_dev);
	char buf[5];
	/*we use 0 to represent no log.use 1 to represent
	 * dc->hw.phytium_log_enable.use 2 to represent 
	 * ftd330_phytium_dc_log on.
	 * */
	const char log_level[5] = {'1', '0', '2'};

	if (len > 2) {
		pr_info("Error: input too long\n");
		return -EINVAL;
	}

	if (copy_from_user(buf, ubuf, len)) {
		pr_info("copy from user failed in %s\n", __func__);
		return -EINVAL;
	}

	if (buf[0] == log_level[0]) {
		//	priv->phytium_log_enable = true;
		dc->hw.phytium_log_enable = true;
	} else if (buf[0] == log_level[1]) {
		//	priv->phytium_log_enable = false;
		dc->hw.phytium_log_enable = false;
		ftd330_phytium_dc_log_level = FTD330_PHYTIUM_DC_NO_LOG;
	} else if (buf[0] == log_level[2]) {
		ftd330_phytium_dc_log_level = FTD330_PHYTIUM_DC_LOG_ON;
	} else {
		pr_info("invalid input! 1 for enable log,0 for disable log\n");
		return -EINVAL;
	}

	return len;
}

static int phytium_crtc_low_power_show(struct seq_file *s, void *data)
{
	struct drm_crtc *crtc = s->private;
	struct drm_device *drm_dev = crtc->dev;
	struct ftd330_drm_private *priv = drm_dev->dev_private;
	int crtc_index = drm_crtc_index(crtc);

	seq_printf(s, "low_power_enable DCDP[%d] : %s\n",
				crtc_index, priv->low_power_enable[crtc_index] ?
				"enable":"disable");

	return 0;
}

static int phytium_crtc_low_power_open(struct inode *inode, struct file *file)
{
	return single_open(file, phytium_crtc_low_power_show, inode->i_private);
}

static ssize_t phytium_crtc_low_power_write(struct file *file, const char __user *ubuf, size_t len,
									loff_t *offp)
{
	struct seq_file *s = file->private_data;
	struct drm_crtc *crtc = s->private;
	struct drm_device *drm_dev = crtc->dev;
	struct ftd330_drm_private *priv = drm_dev->dev_private;
	uint32_t crtc_index = drm_crtc_index(crtc);
	char buf[5];
	const char low_power[3] = {'1', '0'};
	bool temp_value = false;

	if (len > 2) {
		pr_info("Error: input too long\n");
		return -EINVAL;
	}

	if (copy_from_user(buf, ubuf, len)) {
		pr_info("copy from user failed in %s\n", __func__);
		return -EINVAL;
	}

	if (buf[0] == low_power[0]) {
		temp_value = true;
	} else if (buf[0] == low_power[1]) {
		temp_value = false;
	} else {
		pr_info("invalid input! 1 for enable low power,0 for disable low power\n");
		return -EINVAL;
	}

	if (priv->info.total_pipes == 2 && 
			(priv->info.pipe_mask & BIT(DISPLAY_1)) &&
			(priv->info.pipe_mask & BIT(DISPLAY_2))) {
		priv->low_power_enable[0] = temp_value;
		priv->low_power_enable[1] = temp_value;
	} else if ((priv->info.total_pipes == 3) &&
								(crtc_index != 0) &&
				(priv->info.pipe_mask & BIT(DISPLAY_1)) &&
				(priv->info.pipe_mask & BIT(DISPLAY_2))) {
		priv->low_power_enable[1] = temp_value;
		priv->low_power_enable[2] = temp_value;
	} else {
		priv->low_power_enable[crtc_index] = temp_value;
	}

	return len;
}

static int phytium_crtc_underflow_status_show(struct seq_file *s, void *data)
{
	struct drm_crtc *crtc = s->private;
	struct drm_device *drm_dev = crtc->dev;
	struct ftd330_drm_private *priv = drm_dev->dev_private;
	int crtc_index = drm_crtc_index(crtc);
	int dc_id = (crtc_index == 0) ? 0 : 1;
	struct ftd330_dc *dc = dev_get_drvdata(priv->dc_dev);

	seq_printf(s, "water_mark : 0x%x\n", priv->info.water_mark[crtc_index]);
	seq_printf(s, "Qos : 0x%x\n", priv->info.qos[dc_id]);
	seq_puts(s, "Qos_max : 0x0f\n");
	seq_puts(s, "Qos_min : 0x0\n");
	seq_printf(s, "underflow_count : %d\n", dc->hw.display[crtc_index].underflow_count);

	return 0;
}


static int phytium_crtc_pageflip_freq_show(struct seq_file *s, void *data)
{
        struct drm_crtc *crtc = s->private;
	struct ftd330_crtc *ftd330_crtc = to_ftd330_crtc(crtc);

        seq_printf(s, "pageflip freq : %d\n", ftd330_crtc->pageflip.fps);

        return 0;
}



static int phytium_crtc_underflow_status_open(struct inode *inode, struct file *file)
{
	return single_open(file, phytium_crtc_underflow_status_show, inode->i_private);
}

static int phytium_crtc_pageflip_freq_open(struct inode *inode, struct file *file)
{
        return single_open(file, phytium_crtc_pageflip_freq_show, inode->i_private);
}

static const struct file_operations ftd330_crtc_pattern_fops = {
	.open = ftd330_crtc_pattern_open,
	.read = seq_read,
	.write = ftd330_crtc_pattern_write,
	.llseek = seq_lseek,
	.release = single_release,
};

static const struct file_operations ftd330_crtc_crc_fops = {
	.open = ftd330_crtc_crc_open,
	.read = seq_read,
	.write = ftd330_crtc_crc_write,
	.llseek = seq_lseek,
	.release = single_release,
};

static const struct file_operations ftd330_crtc_qos_fops = {
	.open = ftd330_crtc_qos_open,
	.read = seq_read,
	.write = ftd330_crtc_qos_write,
	.llseek = seq_lseek,
	.release = single_release,
};

static const struct file_operations phytium_crtc_log_fops = {
	.open = phytium_crtc_log_open,
	.read = seq_read,
	.write = phytium_crtc_log_write,
	.llseek = seq_lseek,
	.release = single_release,
};


static const struct file_operations phytium_crtc_low_power_fops = {
	.open = phytium_crtc_low_power_open,
	.read = seq_read,
	.write = phytium_crtc_low_power_write,
	.llseek = seq_lseek,
	.release = single_release,
};


static const struct file_operations phytium_crtc_pageflip_freq_fops = {
	.open = phytium_crtc_pageflip_freq_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};



static const struct file_operations phytium_crtc_underflow_status_fops = {
        .open = phytium_crtc_underflow_status_open,
        .read = seq_read,
        .llseek = seq_lseek,
        .release = single_release,
};

static int ftd330_crtc_debugfs_init(struct drm_crtc *crtc)
{
	int crtc_index = drm_crtc_index(crtc);

	debugfs_create_file("status", 0444, crtc->debugfs_entry, crtc, &ftd330_crtc_debugfs_fops);

	debugfs_create_file("pattern", 0644, crtc->debugfs_entry, crtc, &ftd330_crtc_pattern_fops);

	debugfs_create_file("CRC", 0644, crtc->debugfs_entry, crtc, &ftd330_crtc_crc_fops);
	debugfs_create_file("QOS", 0644, crtc->debugfs_entry, crtc, &ftd330_crtc_qos_fops);

	debugfs_create_file("low_power", 0644, crtc->debugfs_entry, crtc, &phytium_crtc_low_power_fops);

	debugfs_create_file("underflow_status", 0644, crtc->debugfs_entry, crtc, &phytium_crtc_underflow_status_fops);

	debugfs_create_file("pageflip_freq", 0644, crtc->debugfs_entry, crtc, &phytium_crtc_pageflip_freq_fops);

	if (crtc_index == 0)
		debugfs_create_file("log_enable", 0644, crtc->debugfs_entry, crtc, &phytium_crtc_log_fops);

	return 0;
}
#else
static int ftd330_crtc_debugfs_init(struct drm_crtc *crtc)
{
	return 0;
}
#endif /* CONFIG_DEBUG_FS */

static int ftd330_crtc_late_register(struct drm_crtc *crtc)
{
	return ftd330_crtc_debugfs_init(crtc);
}

static int ftd330_crtc_enable_vblank(struct drm_crtc *crtc)
{
	struct ftd330_crtc *ftd330_crtc = to_ftd330_crtc(crtc);

	ftd330_crtc->funcs->enable_vblank(ftd330_crtc, true);

	return 0;
}

static void ftd330_crtc_disable_vblank(struct drm_crtc *crtc)
{
	struct ftd330_crtc *ftd330_crtc = to_ftd330_crtc(crtc);

	ftd330_crtc->funcs->enable_vblank(ftd330_crtc, false);
}

static uint32_t ftd330_crtc_get_vblank_count(struct drm_crtc *crtc)
{
	struct ftd330_crtc *ftd330_crtc = to_ftd330_crtc(crtc);

	return ftd330_crtc->funcs->get_vblank_count(ftd330_crtc);
}

int ftd330_drm_atomic_helper_page_flip(struct drm_crtc *crtc,
				struct drm_framebuffer *fb,
				struct drm_pending_vblank_event *event,
				uint32_t flags,
				struct drm_modeset_acquire_ctx *ctx)
{
	struct ftd330_crtc *ftd330_crtc = to_ftd330_crtc(crtc);
	ktime_t present_time;
	long time_passed = 0;

	ftd330_crtc->pageflip.count++;
	present_time = ktime_get();
	time_passed = ktime_to_ms(ktime_sub(present_time, ftd330_crtc->pageflip.time));

	if (time_passed > 1000) {
		ftd330_crtc->pageflip.fps = ((((ftd330_crtc->pageflip.count << 20) / time_passed) * 1000) >> 20);
		ftd330_crtc->pageflip.count = 0;
		ftd330_crtc->pageflip.time = present_time;
	}


	return drm_atomic_helper_page_flip(crtc, fb, event, flags, ctx);
}


static const struct drm_crtc_funcs ftd330_crtc_funcs = {
	.set_config = drm_atomic_helper_set_config,
	.destroy = ftd330_crtc_destroy,
	.page_flip = ftd330_drm_atomic_helper_page_flip,
	.reset = ftd330_crtc_reset,
	.atomic_duplicate_state = ftd330_crtc_atomic_duplicate_state,
	.atomic_destroy_state = ftd330_crtc_atomic_destroy_state,
	.atomic_set_property = ftd330_crtc_atomic_set_property,
	.atomic_get_property = ftd330_crtc_atomic_get_property,
#if KERNEL_VERSION(5, 12, 0) > LINUX_VERSION_CODE
	.gamma_set = drm_atomic_helper_legacy_gamma_set,
#endif
	.late_register = ftd330_crtc_late_register,
	.enable_vblank = ftd330_crtc_enable_vblank,
	.disable_vblank = ftd330_crtc_disable_vblank,
	.get_vblank_counter = ftd330_crtc_get_vblank_count,
//#if KERNEL_VERSION(5, 7, 0) <= LINUX_VERSION_CODE
//	.get_vblank_timestamp = drm_crtc_vblank_helper_get_vblank_timestamp,
//#endif
};

static u8 cal_pixel_bits(u32 bus_format)
{
	u8 bpp;

	switch (bus_format) {
	case MEDIA_BUS_FMT_RGB565_1X16:
	case MEDIA_BUS_FMT_UYVY8_1X16:
		bpp = 16;
		break;
	case MEDIA_BUS_FMT_RGB666_1X18:
	case MEDIA_BUS_FMT_RGB666_1X24_CPADHI:
		bpp = 18;
		break;
	case MEDIA_BUS_FMT_UYVY10_1X20:
		bpp = 20;
		break;
	case MEDIA_BUS_FMT_BGR888_1X24:
	case MEDIA_BUS_FMT_UYYVYY8_0_5X24:
	case MEDIA_BUS_FMT_YUV8_1X24:
		bpp = 24;
		break;
	case MEDIA_BUS_FMT_RGB101010_1X30:
	case MEDIA_BUS_FMT_UYYVYY10_0_5X30:
	case MEDIA_BUS_FMT_YUV10_1X30:
		bpp = 30;
		break;
	default:
		bpp = 24;
		break;
	}

	return bpp;
}


static bool ftd330_crtc_mode_fixup(struct drm_crtc *crtc, const struct drm_display_mode *mode,
			       struct drm_display_mode *adjusted_mode)
{
	struct ftd330_crtc *ftd330_crtc = to_ftd330_crtc(crtc);

	return ftd330_crtc->funcs->mode_fixup(ftd330_crtc->dev, mode, adjusted_mode);
}

static void ftd330_crtc_atomic_enable(struct drm_crtc *crtc,
				  #if KERNEL_VERSION(5, 11, 0) > LINUX_VERSION_CODE
				  struct drm_crtc_state *old_state)
#else
				  struct drm_atomic_state *old_state)
#endif
{
	struct ftd330_crtc *ftd330_crtc = to_ftd330_crtc(crtc);
	struct ftd330_crtc_state *ftd330_crtc_state = to_ftd330_crtc_state(crtc->state);

	ftd330_crtc_state->bpp = cal_pixel_bits(ftd330_crtc_state->output_fmt);

	ftd330_crtc->funcs->enable(ftd330_crtc->dev, crtc);

	drm_crtc_vblank_on(crtc);
	/* vblank interrupt enable after crtc enable */
}

static void ftd330_crtc_atomic_disable(struct drm_crtc *crtc,
#if KERNEL_VERSION(5, 11, 0) > LINUX_VERSION_CODE
				   struct drm_crtc_state *old_state)
#else
				   struct drm_atomic_state *old_state)
#endif
{
	struct ftd330_crtc *ftd330_crtc = to_ftd330_crtc(crtc);
	struct drm_crtc_state *crtc_old_state = NULL;

#if KERNEL_VERSION(5, 11, 0) > LINUX_VERSION_CODE
	crtc_old_state = old_state;
#else
	crtc_old_state = drm_atomic_get_old_crtc_state(old_state, crtc);
#endif
	if (crtc->state->mode_changed && !crtc->state->active_changed) {
		drm_crtc_vblank_off(crtc);
		return;
	}

	ftd330_crtc->funcs->disable(ftd330_crtc->dev, crtc);
	drm_atomic_helper_disable_planes_on_crtc(crtc_old_state, true);

	if (!completion_done(&ftd330_crtc->frame_completion))
		wait_for_completion_timeout(&ftd330_crtc->frame_completion, 10 * 1000);

	ftd330_crtc_handle_vblank(crtc);
	ftd330_crtc_handle_flip_done(crtc);
	drm_crtc_vblank_off(crtc);

}

static void ftd330_crtc_atomic_begin(struct drm_crtc *crtc,
#if KERNEL_VERSION(5, 11, 0) > LINUX_VERSION_CODE
				 struct drm_crtc_state *old_crtc_state)
#else
				 struct drm_atomic_state *old_crtc_state)
#endif
{
	struct ftd330_crtc *ftd330_crtc = to_ftd330_crtc(crtc);
	struct device *dev = ftd330_crtc->dev;

	if (ftd330_crtc->funcs->config)
		ftd330_crtc->funcs->config(dev, crtc);
}

static void phytium_display_enable_mismatch_vblank(struct drm_crtc *crtc)
{
	struct ftd330_crtc *ftd330_crtc = NULL;
	struct ftd330_dc *dc = NULL;
	int register_enable = 0;
	int physical_display_id = 0;
	int crtc_index = 0;

	if (!crtc)
		return;

	ftd330_crtc = to_ftd330_crtc(crtc);
	if (!ftd330_crtc)
		return;

	dc = dev_get_drvdata(ftd330_crtc->dev);
	if (!dc)
		return;

	crtc_index = drm_crtc_index(crtc);
	physical_display_id = phytium_display_virtual_to_physical(dc->hw.pipe_mask,crtc_index);

	if (physical_display_id == DISPLAY_0) {
		register_enable = hi_read(&dc->hw, AQ_INTR_ENBL);
	} else if (physical_display_id == DISPLAY_1) {
		register_enable = hi_read(&dc->hw, AQ_INTR_ENBL + DC_REG_OFFSET);
		if (!register_enable || !(register_enable & BIT(0))) {
			register_enable = 0;
		}
        } else if (physical_display_id == DISPLAY_2) {
		register_enable = hi_read(&dc->hw, AQ_INTR_ENBL + DC_REG_OFFSET);
		if (!register_enable || !(register_enable & BIT(1))) {
                        register_enable = 0;
                }
        } else {
		return;
	}

	if (!register_enable && dc->hw.display[physical_display_id].vblank_enable) {
		dc_hw_enable_interrupt(&dc->hw, dc->hw.display[physical_display_id].vblank_enable, crtc_index);
	}

	return;
}


static void ftd330_crtc_atomic_flush(struct drm_crtc *crtc,
#if KERNEL_VERSION(5, 11, 0) > LINUX_VERSION_CODE
				 struct drm_crtc_state *old_crtc_state)
#else
				 struct drm_atomic_state *old_crtc_state)
#endif
{
	struct ftd330_crtc *ftd330_crtc = to_ftd330_crtc(crtc);
	unsigned long flags;

	/*enable vblank here so that commit tail will get vblank interrupt
	 *and put it imediately so that vblank can be off after no vblank 
	 * request 5s.
	 * */
	drm_crtc_vblank_get(crtc);
	phytium_display_enable_mismatch_vblank(crtc);
	drm_crtc_vblank_put(crtc);
	/* in the unexpected case that event wasn't signaled before, signal it now */
	ftd330_crtc->commit_hw_done = false;
	spin_lock_irqsave(&crtc->dev->event_lock, flags);
	ftd330_crtc->event = crtc->state->event;
	crtc->state->event = NULL;
	spin_unlock_irqrestore(&crtc->dev->event_lock, flags);

	ftd330_crtc->funcs->commit(ftd330_crtc->dev, crtc);
}

#if KERNEL_VERSION(5, 11, 0) > LINUX_VERSION_CODE
static int ftd330_crtc_atomic_check(struct drm_crtc *crtc, struct drm_crtc_state *state)
{
	struct ftd330_crtc *ftd330_crtc = to_ftd330_crtc(crtc);

	if (!crtc)
		return -EINVAL;

	if (!state->enable || !state->active)
		return 0;
	return ftd330_crtc->funcs->check(ftd330_crtc->dev, crtc, state);
}

#else
static int ftd330_crtc_atomic_check(struct drm_crtc *crtc, struct drm_atomic_state *state)
{
	struct ftd330_crtc *ftd330_crtc = to_ftd330_crtc(crtc);
	struct drm_crtc_state *crtc_state = NULL;

	if (!crtc)
		return -EINVAL;

	crtc_state = drm_atomic_get_new_crtc_state(state, crtc);

	if (!crtc_state->enable || !crtc_state->active)
		return 0;

	return ftd330_crtc->funcs->check(ftd330_crtc->dev, crtc, crtc_state);
}
#endif
#if KERNEL_VERSION(5, 7, 0) <= LINUX_VERSION_CODE
#if 0
static bool ftd330_crtc_get_scanout_position(struct drm_crtc *crtc, bool in_vblank_irq, int *vpos,
					 int *hpos, ktime_t *stime, ktime_t *etime,
					 const struct drm_display_mode *mode)
{
	struct drm_device *dev = crtc->dev;
	unsigned int pipe = crtc->index;

	return ftd330_display_get_crtc_scanoutpos(dev, pipe, in_vblank_irq, vpos, hpos, stime, etime,
							mode);
}
#endif
#endif

static const struct drm_crtc_helper_funcs ftd330_crtc_helper_funcs = {
	.mode_fixup = ftd330_crtc_mode_fixup,
	.atomic_enable = ftd330_crtc_atomic_enable,
	.atomic_disable = ftd330_crtc_atomic_disable,
	.atomic_begin = ftd330_crtc_atomic_begin,
	.atomic_flush = ftd330_crtc_atomic_flush,
	.atomic_check = ftd330_crtc_atomic_check,
//#if KERNEL_VERSION(5, 7, 0) <= LINUX_VERSION_CODE
//	.get_scanout_position = ftd330_crtc_get_scanout_position,
//#endif
};

static const struct drm_prop_enum_list ftd330_sync_mode_enum_list[] = {
	{ FTD330_SINGLE_DC, "single dc mode" },
	{ FTD330_MULTI_DC_PRIMARY, "primary dc for multi dc mode" },
	{ FTD330_MULTI_DC_SECONDARY, "secondary dc for multi dc mode" },
};

#ifdef CONFIG_PHYTIUM_MMU
static const struct drm_prop_enum_list ftd330_mmu_prefetch_enum_list[] = {
	{ FTD330_MMU_PREFETCH_DISABLE, "disable mmu prefetch" },
	{ FTD330_MMU_PREFETCH_ENABLE, "enable mmu prefetch" },
};
#endif

struct ftd330_crtc *ftd330_crtc_create(const struct dc_hw_display *display, struct drm_device *drm_dev,
			       const struct ftd330_dc_info *info, u8 index)
{
	struct ftd330_crtc *crtc;
	struct ftd330_display_info *display_info = NULL;
	int ret;

	if (!info)
		return NULL;

	display_info = (struct ftd330_display_info *)&info->displays[index];
	if (!display_info)
		return NULL;

	crtc = kzalloc(sizeof(*crtc), GFP_KERNEL);
	if (!crtc)
		return NULL;

	ret = drm_crtc_init_with_planes(drm_dev, &crtc->base, NULL, NULL, &ftd330_crtc_funcs,
					display_info->name ? display_info->name : NULL);
	if (ret)
		goto err_free_crtc;


	drm_crtc_helper_add(&crtc->base, &ftd330_crtc_helper_funcs);

	/* Set up the crtc properties */
	if (info->pipe_sync) {
		crtc->sync_mode = drm_property_create_enum(drm_dev, 0, "SYNC_MODE",
							   ftd330_sync_mode_enum_list,
							   ARRAY_SIZE(ftd330_sync_mode_enum_list));

		if (!crtc->sync_mode)
			goto err_cleanup_crts;

		drm_object_attach_property(&crtc->base.base, crtc->sync_mode, FTD330_SINGLE_DC);
	}

	if (display_info->gamma) {
		if (info->std_color_lut) {
			ret = drm_mode_crtc_set_gamma_size(&crtc->base, info->max_gamma_size);
			if (ret)
				goto err_cleanup_crts;

			drm_crtc_enable_color_mgmt(&crtc->base, 0, display_info->ccm_linear,
						   info->max_gamma_size);
		} else {
			crtc->prior_gamma_prop =
				drm_property_create(drm_dev, DRM_MODE_PROP_BLOB, "PRIOR_GAMMA", 0);

			if (!crtc->prior_gamma_prop)
				goto err_cleanup_crts;

			drm_object_attach_property(&crtc->base.base, crtc->prior_gamma_prop, 0);
		}

		if (display_info->lut_roi) {
			crtc->roi0_gamma_prop =
				drm_property_create(drm_dev, DRM_MODE_PROP_BLOB, "ROI0_GAMMA", 0);

			if (!crtc->roi0_gamma_prop)
				goto err_cleanup_crts;

			drm_object_attach_property(&crtc->base.base, crtc->roi0_gamma_prop, 0);

			crtc->roi1_gamma_prop =
				drm_property_create(drm_dev, DRM_MODE_PROP_BLOB, "ROI1_GAMMA", 0);

			if (!crtc->roi1_gamma_prop)
				goto err_cleanup_crts;

			drm_object_attach_property(&crtc->base.base, crtc->roi1_gamma_prop, 0);
		}
	}


	if (info->panel_sync) {
		crtc->panel_sync = drm_property_create_bool(drm_dev, 0, "SYNC_ENABLED");

		if (!crtc->panel_sync)
			goto err_cleanup_crts;

		drm_object_attach_property(&crtc->base.base, crtc->panel_sync, 0);
	}

	/* for FTD330 dither module */
	if (display_info->ltm && (display_info->id == 0)) {
		crtc->ltm_luma_get_prop =
			drm_property_create(drm_dev, DRM_MODE_PROP_BLOB, "LTM_LUMA_AVE_GET", 0);

		if (!crtc->ltm_luma_get_prop)
			goto err_cleanup_crts;

		drm_object_attach_property(&crtc->base.base, crtc->ltm_luma_get_prop, 0);

		crtc->ltm_cd_get_prop =
			drm_property_create(drm_dev, DRM_MODE_PROP_BLOB, "LTM_HIST_CD_GET", 0);

		if (!crtc->ltm_cd_get_prop)
			goto err_cleanup_crts;

		drm_object_attach_property(&crtc->base.base, crtc->ltm_cd_get_prop, 0);

		crtc->ltm_hist_get_prop =
			drm_property_create(drm_dev, DRM_MODE_PROP_BLOB, "LTM_LOCAL_HIST_GET", 0);

		if (!crtc->ltm_hist_get_prop)
			goto err_cleanup_crts;

		drm_object_attach_property(&crtc->base.base, crtc->ltm_hist_get_prop, 0);
	}

	if (display_info->histogram) {
		crtc->hist_get_prop =
			drm_property_create(drm_dev, DRM_MODE_PROP_BLOB, "HIST_GET", 0);

		if (!crtc->hist_get_prop)
			goto err_cleanup_crts;

		drm_object_attach_property(&crtc->base.base, crtc->hist_get_prop, 0);
	}

	if (display_info->rgb_hist && (display_info->id == 0 || ((info->display_num > 2) && display_info->id == 1))) {
		crtc->rgb_hist_get_prop =
			drm_property_create(drm_dev, DRM_MODE_PROP_BLOB, "RGB_HIST_GET", 0);

		if (!crtc->rgb_hist_get_prop)
			goto err_cleanup_crts;

		drm_object_attach_property(&crtc->base.base, crtc->rgb_hist_get_prop, 0);
	}

	if (display_info->rcd) {
		crtc->rcd_mask_fb = drm_property_create_object(drm_dev, DRM_MODE_PROP_ATOMIC,
			"RCD_MASK", DRM_MODE_OBJECT_FB);
		if (!crtc->rcd_mask_fb)
			goto err_cleanup_crts;

		drm_object_attach_property(&crtc->base.base, crtc->rcd_mask_fb, 0);
	}

	if (display_info->blur) {
		crtc->blur_mask_fb = drm_property_create_object(drm_dev, DRM_MODE_PROP_ATOMIC,
			"BLUR_MASK", DRM_MODE_OBJECT_FB);
		if (!crtc->blur_mask_fb)
			goto err_cleanup_crts;

		drm_object_attach_property(&crtc->base.base, crtc->blur_mask_fb, 0);
	}

	if (display_info->brightness) {
		crtc->brightness_mask_fb = drm_property_create_object(
			drm_dev, DRM_MODE_PROP_ATOMIC, "BRIGHTNESS_MASK", DRM_MODE_OBJECT_FB);
		if (!crtc->brightness_mask_fb)
			goto err_cleanup_crts;

		drm_object_attach_property(&crtc->base.base, crtc->brightness_mask_fb, 0);
	}

	if (display != NULL && ftd330_dc_create_drm_properties(drm_dev, &crtc->base.base,
		&display->states, &crtc->properties)) {
		goto err_cleanup_crts;
	}

#ifdef CONFIG_PHYTIUM_MMU
	if (info->mmu_prefetch) {
		crtc->mmu_prefetch = drm_property_create_enum(
			drm_dev, 0, "MMU_PREFETCH", ftd330_mmu_prefetch_enum_list,
			ARRAY_SIZE(ftd330_mmu_prefetch_enum_list));
		if (!crtc->mmu_prefetch)
			goto err_cleanup_crts;

		drm_object_attach_property(&crtc->base.base, crtc->mmu_prefetch,
					   FTD330_MMU_PREFETCH_DISABLE);
	}
#endif
	spin_lock_init(&crtc->slock);

	crtc->max_bpc = info->max_bpc;
	crtc->color_formats = display_info->color_formats;
	crtc->id = index;


	return crtc;

err_cleanup_crts:
	drm_crtc_cleanup(&crtc->base);

err_free_crtc:
	if (crtc)
		kfree(crtc);
	return NULL;
}

void ftd330_crtc_handle_vblank(struct drm_crtc *crtc)
{
	struct ftd330_crtc *ftd330_crtc = to_ftd330_crtc(crtc);

	ftd330_crtc->t_cut_vblank = ktime_get() - ftd330_crtc->t_vblank;
	ftd330_crtc->t_vblank = ktime_get();
	drm_crtc_handle_vblank(crtc);
}

void ftd330_crtc_handle_frame_done(struct drm_crtc *crtc)
{
	struct ftd330_crtc *ftd330_crtc = to_ftd330_crtc(crtc);

	complete(&ftd330_crtc->frame_completion);
}

void ftd330_crtc_handle_flip_done_while_hw_done(struct drm_crtc *crtc)
{
	struct ftd330_crtc *ftd330_crtc = to_ftd330_crtc(crtc);
	unsigned long flags;

	/* Init the frame completion */
	init_completion(&ftd330_crtc->frame_completion);

	if ((!ftd330_crtc->commit_hw_done) || (!ftd330_crtc->event))
		return;

	spin_lock_irqsave(&crtc->dev->event_lock, flags);
	drm_crtc_send_vblank_event(crtc, ftd330_crtc->event);
	ftd330_crtc->event = NULL;
	spin_unlock_irqrestore(&crtc->dev->event_lock, flags);
}

void ftd330_crtc_handle_flip_done(struct drm_crtc *crtc)
{
	struct ftd330_crtc *ftd330_crtc = to_ftd330_crtc(crtc);
	unsigned long flags;

	/* Init the frame completion */
	init_completion(&ftd330_crtc->frame_completion);

	if (!ftd330_crtc->event)
		return;

	spin_lock_irqsave(&crtc->dev->event_lock, flags);
	drm_crtc_send_vblank_event(crtc, ftd330_crtc->event);
	ftd330_crtc->event = NULL;
	spin_unlock_irqrestore(&crtc->dev->event_lock, flags);
}


