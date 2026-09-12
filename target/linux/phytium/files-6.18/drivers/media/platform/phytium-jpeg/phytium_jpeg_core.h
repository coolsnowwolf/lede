/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2021-2023 Phytium Technology Co., Ltd.
 */

#ifndef _PHYTIUM_JPEG_CORE_H
#define _PHYTIUM_JPEG_CORE_H

#include <linux/atomic.h>
#include <linux/bitfield.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/jiffies.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_reserved_mem.h>
#include <linux/platform_device.h>
#include <linux/sched.h>
#include <linux/spinlock.h>
#include <linux/string.h>
#include <linux/v4l2-controls.h>
#include <linux/videodev2.h>
#include <linux/wait.h>
#include <linux/workqueue.h>
#include <media/v4l2-ctrls.h>
#include <media/v4l2-dev.h>
#include <media/v4l2-device.h>
#include <media/v4l2-dv-timings.h>
#include <media/v4l2-event.h>
#include <media/v4l2-ioctl.h>
#include <media/videobuf2-dma-contig.h>

#define PHYTIUM_JPEG_NAME       "phytium-jpeg"
#define MAX_FRAME_RATE          60
#define MAX_HEIGHT              1080
#define MAX_WIDTH               1920
#define MIN_HEIGHT              480
#define MIN_WIDTH               640
#define MIN_PIXEL_CLOCK         (640 * 480 * 60)     /* 640 x 480 x 60Hz */
#define MAX_PIXEL_CLOCK         (1920 * 1080 * 60)   /* 1920 x 1080 x 60Hz */

#define SOURCE_RESOLUTION_DETECT_TIMEOUT    msecs_to_jiffies(500)
#define RESOLUTION_CHANGE_DELAY             msecs_to_jiffies(0)
#define INVALID_RESOLUTION_DELAY            msecs_to_jiffies(250)
#define STOP_TIMEOUT                        msecs_to_jiffies(1000)

#define INVALID_RESOLUTION_RETRIES   2
#define CAPTURE_BUF_NUMBER           3  /* using how many buffers */
#define VB_BUF_NO                    0  /* there are 16 buffer, use which one */

/* The below macros are defined for the JPEG header of the phytium JPEG Engine */
#define PHYTIUM_JPEG_HEADER_LEN		(256 * 3)
#define PHYTIUM_JPEG_HEADER_SIZE        (PHYTIUM_JPEG_HEADER_LEN / sizeof(u32))
#define PHYTIUM_JPEG_HEADER_H_INDEX     40
#define PHYTIUM_JPEG_HEADER_W_INDEX     41

/* There are two ocm buffers that are used for storaging the inputing video data */
#define OCM_BUF_NUM			2

enum phytium_jpeg_status {
	VIDEO_MODE_DETECT_DONE,
	VIDEO_RES_CHANGE,
	VIDEO_RES_DETECT,
	VIDEO_STREAMING,
	VIDEO_FRAME_INPRG,
	VIDEO_STOPPED,
	VIDEO_CLOCKS_ON,
	VIDEO_POWEROFF,
};

struct phytium_jpeg_addr {
	unsigned int size;
	dma_addr_t  dma_addr;
	void    *virt_addr;
};

struct phytium_jpeg_buffer {
	struct vb2_v4l2_buffer vb;
	struct list_head link;
};

/**
 * struct phytium_jpeg - JPEG IP abstraction
 * @lock:	  the mutex protecting this structure
 * @hw_lock:	  spinlock protecting the hw device resource
 * @workqueue:	  decode work queue
 * @dev:	  JPEG device
 * @v4l2_dev:	  v4l2 device for mem2mem mode
 * @m2m_dev:	  v4l2 mem2mem device data
 * @alloc_ctx:	  videobuf2 memory allocator's context
 * @dec_vdev:	  video device node for decoder mem2mem mode
 * @dec_reg_base: JPEG registers mapping
 * @clk_jdec:	  JPEG hw working clock
 * @clk_jdec_smi: JPEG SMI bus clock
 * @larb:	  SMI device
 */
struct phytium_jpeg_dev {
	void __iomem *base_addr;
	struct device *dev;
	struct v4l2_device v4l2_dev;
	struct v4l2_pix_format pix_fmt;
	struct v4l2_bt_timings active_timings;
	struct v4l2_bt_timings detected_timings;
	u32    v4l2_input_status;
	struct vb2_queue queue;
	struct video_device vdev;
	/* v4l2 and videobuf2 lock, protect the structure*/
	struct mutex video_lock;
	u32 jpeg_mode;
	u32 comp_size_read;
	wait_queue_head_t wait;
	/* buffer list lock, protecting the hw device resource */
	spinlock_t hw_lock;
	struct delayed_work res_work;
	struct list_head buffers;
	unsigned long status;
	unsigned int sequence;
	unsigned int max_compressed_size;
	struct phytium_jpeg_addr src_addrs[OCM_BUF_NUM];
	struct phytium_jpeg_addr dst_addrs[16];

	bool yuv420;
	unsigned int frame_rate;
	void __iomem *timer30_addr;
	void __iomem *timer31_addr;
};

struct phytium_jpeg_config {
	u32 jpeg_mode;
	u32 comp_size_read;
};

#define		YUV_MODE_STR_LEN	8
#define		YUVID			42

enum jpeg_yuv_mode {
	YUV444 = 0x0,
	YUV422 = 0x1,
	YUV420 = 0x2
};

#endif  /* _PHYTIUM_JPEG_CORE_H */
