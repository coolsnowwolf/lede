/*
 ***************************************************************************
 * MediaTek Inc.
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

	Module Name: mt_wifi
	wbsys_of.h
*/

#ifndef _WBSYS_RES_H
#define _WBSYS_RES_H

#include "rtmp_chip.h"

/*platform device & platform driver match name*/
static char wbsys_string[] = "wb_sys";

#if defined(CONFIG_OF)
#include <linux/of_irq.h>
#include <linux/of_address.h>

static const struct of_device_id wbsys_of_ids[] = {
	{   .compatible = OF_WBSYS_NAME, },
	{ },
};

#define wbsys_dev_alloc(res)
#define wbsys_dev_release(res)

#else
static struct resource wbsys_res[] = {
	[0] = {
		.start = (RTMP_MAC_CSR_ADDR),
		.end = (RTMP_MAC_CSR_ADDR + RTMP_MAC_CSR_LEN-1),
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = RTMP_IRQ_NUM,
		.end = RTMP_IRQ_NUM,
		.flags = IORESOURCE_IRQ,
	}
};

static void platform_wbsys_release(struct device *dev)
{
	return;
}

struct platform_device wbsys_dev = {
	.name = wbsys_string,
	.id = -1,
	.num_resources	= ARRAY_SIZE(wbsys_res),
	.resource		= wbsys_res,
	.dev = {
		.release = platform_wbsys_release,
	}
};
#define wbsys_dev_alloc(dev)\
	{\
		platform_device_register(dev);\
	}

#define wbsys_dev_release(dev)\
	{\
		platform_device_unregister(dev);\
	}
#endif /*CONFIG_OF*/

#endif /*_WBSYS_RES_H*/
