/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2025, Phytium Technology Co., Ltd.
 */

#ifndef __FTD330_FB_H__
#define __FTD330_FB_H__

struct drm_phytium_display_fb_cmd2 {
	/** @fb_id: Object ID of the frame-buffer. */
	__u32 fb_id;
	/** @width: Width of the frame-buffer. */
	__u32 width;
	/** @height: Height of the frame-buffer. */
	__u32 height;
	/**
	 * @pixel_format: FourCC format code, see ``DRM_FORMAT_*`` constants in
	 * ``drm_fourcc.h``.
	 */
	__u32 pixel_format;
	/**
	 * @flags: Frame-buffer flags (see &DRM_MODE_FB_INTERLACED and
	 * &DRM_MODE_FB_MODIFIERS).
	 */
	__u32 flags;

	/**
	 * @handles: GEM buffer handle, one per plane. Set to 0 if the plane is
	 * unused. The same handle can be used for multiple planes.
	 */
	__u32 handles[4];
	/** @pitches: Pitch (aka. stride) in bytes, one per plane. */
	__u32 pitches[4];
	/** @offsets: Offset into the buffer in bytes, one per plane. */
	__u32 offsets[4];
	/**
	 * @modifier: Format modifier, one per plane. See ``DRM_FORMAT_MOD_*``
	 * constants in ``drm_fourcc.h``. All planes must use the same
	 * modifier. Ignored unless &DRM_MODE_FB_MODIFIERS is set in @flags.
	 */
	__u64 modifier[4];
};

struct ftd330_gem_object *ftd330_fb_get_gem_obj(struct drm_framebuffer *fb, unsigned char plane);

void ftd330_mode_config_init(struct drm_device *dev);

int ftd330_get_fbc_offset_ioctl(struct drm_device *dev, void *data, struct drm_file *file_priv);

struct drm_framebuffer
*ftd330_fb_alloc(struct drm_device *dev,
			const struct drm_mode_fb_cmd2 *mode_cmd,
			struct ftd330_gem_object **obj, unsigned int num_planes);

int ftd330_display_getfb2_ioctl(struct drm_device *dev, void *data, struct drm_file *file_priv);

#ifdef CONFIG_PHYTIUM_PSR
struct phytium_dp_device
*find_phytium_dp_from_fb(struct drm_framebuffer *fb);
#endif
#endif /* __FTD330_FB_H__ */
