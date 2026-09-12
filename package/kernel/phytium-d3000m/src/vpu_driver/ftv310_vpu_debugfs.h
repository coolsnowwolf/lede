/* SPDX-License-Identifier: GPL-2.0 */

/*
 * ftv310 driver debugfs interface to control/query vpu state.
 *
 * Copyright (C) 2024-2025, Phytium Technology Co., Ltd.
 */

#ifndef __PHYTIUM_VPU_DEBUGFS_H_
#define __PHYTIUM_VPU_DEBUGFS_H_

#include <linux/debugfs.h>
#include "ftv310_vpu_priv.h"
#include "ftv310_vpu_power.h"
#include "ftv310_vpu_vcmd.h"
#include "ftv310_vpu_voltage.h"

#if KERNEL_VERSION(5, 8, 0) <= LINUX_VERSION_CODE
void ftv310_vpu_debugfs_init(struct drm_minor *minor);
#else
int ftv310_vpu_debugfs_init(struct drm_minor *minor);
#endif

#endif
