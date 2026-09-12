/* SPDX-License-Identifier: GPL-2.0 */
/* Phytium display drm driver
 *
 * Copyright (C) 2021-2025 Phytium Technology Co., Ltd.
 */

#ifndef __PHYTIUM_DP_IOCTL_H__
#define __PHYTIUM_DP_IOCTL_H__

#include <linux/ioctl.h>
#if KERNEL_VERSION(5, 5, 0) > LINUX_VERSION_CODE
#include <drm/drmP.h>
#endif

#define DEV_FIFO_TYPE 'x'

#define DP_READ_DPCD _IOR(DEV_FIFO_TYPE,0,struct dpcd_ioctl_data*)
#define DP_WRITE_DPCD _IOW(DEV_FIFO_TYPE,1,struct dpcd_ioctl_data*)

struct dpcd_ioctl_data {
	size_t address;
	char *buffer;
	size_t size;
	int w_data;
};

#endif