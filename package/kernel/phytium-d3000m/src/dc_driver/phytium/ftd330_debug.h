/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2025, Phytium Technology Co., Ltd.
 */

#ifndef __FTD330_DEBUG_H_
#define __FTD330_DEBUG_H_

#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

int ftd330_debug_file_create(struct file **fp);
void ftd330_debug_file_close(struct file **fp);
int ftd330_debug_reset(struct file **fp);
void ftd330_debug_dump_capture(struct file *fp, u32 reg, u32 value);

#endif /* __FTD330_VIRTUAL_H_ */
