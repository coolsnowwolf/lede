/****************************************************************************
*
*    Copyright (c) 2025, Phytium Technology Co., Ltd.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Phytium Corporation. This is proprietary information owned by
*    Phytium Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Phytium Corporation.
*
*****************************************************************************/


#ifndef __gc_hal_drm_h_
#define __gc_hal_drm_h_

#ifdef __KERNEL__
#include <uapi/drm/drm.h>
#else
#include <drm.h>
#endif

#if defined(__cplusplus)
extern "C" {
#endif

/* creation flag bits. */
#define DRM_VIV_GEM_CONTIGUOUS      (1u << 0)
#define DRM_VIV_GEM_CACHED          (1u << 1)
#define DRM_VIV_GEM_SECURE          (1u << 2)
#define DRM_VIV_GEM_CMA_LIMIT       (1u << 3)

struct drm_viv_gem_create {
    __u64 size;
    __u32 flags;
    __u32 handle;
};

struct drm_viv_gem_lock {
    __u32 handle;
    __u32 cacheable;
    __u64 logical;
};

struct drm_viv_gem_unlock {
    __u32 handle;
};

#define DRM_VIV_GEM_CLEAN_CACHE         0x01
#define DRM_VIV_GEM_INVALIDATE_CACHE    0x02
#define DRM_VIV_GEM_FLUSH_CACHE         0x03
#define DRM_VIV_GEM_MEMORY_BARRIER      0x04

struct drm_viv_gem_cache {
    __u32 handle;
    __u32 op;
    __u64 logical;
    __u64 bytes;
};

#define DRM_VIV_GEM_PARAM_POOL      0x00
#define DRM_VIV_GEM_PARAM_SIZE      0x01

struct drm_viv_gem_query {
    __u32 handle;
    __u32 param;
    __u64 value;
};

struct drm_viv_gem_timestamp {
    __u32 handle;
    /* inc count, 0 for query current. */
    __u32 inc;
    /* output inc'ed timestamp. */
    __u64 timestamp;
};

/* basic tiling mode. */
#define DRM_VIV_GEM_TILING_LINEAR     0x01
#define DRM_VIV_GEM_TILING_TILED      0x02
#define DRM_VIV_GEM_TILING_SUPERTILED 0x04
#define DRM_VIV_GEM_TILING_MINORTILED 0x08

/* tiling mode modifiers. */
#define DRM_VIV_GEM_TILING_SPLIT    0x10
#define DRM_VIV_GEM_TILING_X_MAJOR  0x20
#define DRM_VIV_GEM_TILING_Y_MAJOR  0x40
#define DRM_VIV_GEM_TILING_SWAP     0x80

/* ts mode. */
#define DRM_VIV_GEM_TS_NONE         0x00
#define DRM_VIV_GEM_TS_DISABLED     0x01
#define DRM_VIV_GEM_TS_NORMAL       0x02
#define DRM_VIV_GEM_TS_COMPRESSED   0x03

/* ts cache mode. */
#define DRM_VIV_GEM_TS_CACHE_MODE_64B     0x00
#define DRM_VIV_GEM_TS_CACHE_MODE_128B    0x01
#define DRM_VIV_GEM_TS_CACHE_MODE_256B    0x02

struct drm_viv_gem_set_tiling {
    __u32 handle;
    __u32 tiling_mode;

    __u32 ts_mode;
    __u32 ts_cache_mode;
    __u64 clear_value;
};

struct drm_viv_gem_get_tiling {
    __u32 handle;
    __u32 tiling_mode;

    __u32 ts_mode;
    __u32 ts_cache_mode;
    __u64 clear_value;
};

struct drm_viv_gem_attach_aux {
    __u32 handle;
    __u32 ts_handle;
};

struct drm_viv_gem_ref_node {
    __u32 handle;

    /* output. */
    __u32 node;
    __u32 ts_node;
};

#define DRM_VIV_GEM_CREATE          0x00
#define DRM_VIV_GEM_LOCK            0x01
#define DRM_VIV_GEM_UNLOCK          0x02
#define DRM_VIV_GEM_CACHE           0x03
#define DRM_VIV_GEM_QUERY           0x04
#define DRM_VIV_GEM_TIMESTAMP       0x05
#define DRM_VIV_GEM_SET_TILING      0x06
#define DRM_VIV_GEM_GET_TILING      0x07
#define DRM_VIV_GEM_ATTACH_AUX      0x08
#define DRM_VIV_GEM_REF_NODE        0x09
#define DRM_VIV_NUM_IOCTLS          0x0A

#define DRM_IOCTL_VIV_GEM_CREATE        DRM_IOWR(DRM_COMMAND_BASE + DRM_VIV_GEM_CREATE,     struct drm_viv_gem_create)
#define DRM_IOCTL_VIV_GEM_LOCK          DRM_IOWR(DRM_COMMAND_BASE + DRM_VIV_GEM_LOCK,       struct drm_viv_gem_lock)
#define DRM_IOCTL_VIV_GEM_UNLOCK        DRM_IOWR(DRM_COMMAND_BASE + DRM_VIV_GEM_UNLOCK,     struct drm_viv_gem_unlock)
#define DRM_IOCTL_VIV_GEM_CACHE         DRM_IOWR(DRM_COMMAND_BASE + DRM_VIV_GEM_CACHE,      struct drm_viv_gem_cache)
#define DRM_IOCTL_VIV_GEM_QUERY         DRM_IOWR(DRM_COMMAND_BASE + DRM_VIV_GEM_QUERY,      struct drm_viv_gem_query)
#define DRM_IOCTL_VIV_GEM_TIMESTAMP     DRM_IOWR(DRM_COMMAND_BASE + DRM_VIV_GEM_TIMESTAMP,  struct drm_viv_gem_timestamp)
#define DRM_IOCTL_VIV_GEM_SET_TILING    DRM_IOWR(DRM_COMMAND_BASE + DRM_VIV_GEM_SET_TILING, struct drm_viv_gem_set_tiling)
#define DRM_IOCTL_VIV_GEM_GET_TILING    DRM_IOWR(DRM_COMMAND_BASE + DRM_VIV_GEM_GET_TILING, struct drm_viv_gem_get_tiling)
#define DRM_IOCTL_VIV_GEM_ATTACH_AUX    DRM_IOWR(DRM_COMMAND_BASE + DRM_VIV_GEM_ATTACH_AUX, struct drm_viv_gem_attach_aux)
#define DRM_IOCTL_VIV_GEM_REF_NODE      DRM_IOWR(DRM_COMMAND_BASE + DRM_VIV_GEM_REF_NODE,   struct drm_viv_gem_ref_node)

#ifdef __KERNEL__
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 9, 0)
#  define drm_gem_object_unreference_unlocked drm_gem_object_put
#  define drm_dev_unref                       drm_dev_put
# elif LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0)
#  define drm_gem_object_unreference_unlocked drm_gem_object_put_unlocked
#  define drm_dev_unref                       drm_dev_put
# endif
#endif

#if defined(__cplusplus)
}
#endif

#endif /* __gc_hal_drm_h_ */
