/******************************************************************************
**
** FILE NAME    : ifxmips_deu_dma.h
** DESCRIPTION  : Data Encryption Unit Driver
** COPYRIGHT    :       Copyright (c) 2009
**                      Infineon Technologies AG
**                      Am Campeon 1-12, 85579 Neubiberg, Germany
**
**    This program is free software; you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation; either version 2 of the License, or
**    (at your option) any later version.
**
** HISTORY
** $Date        $Author             $Comment
** 08,Sept 2009 Mohammad Firdaus    Initial UEIP release
*******************************************************************************/

/*!
  \addtogroup    IFX_DEU IFX_DEU_DRIVERS
  \ingroup	 API
  \brief 	 ifx deu driver module
*/

/*!
  \file		ifxmips_deu_dma.h
  \ingroup 	IFX_DEU
  \brief 	DMA deu driver header file
*/

#ifndef IFXMIPS_DEU_DMA_H
#define IFXMIPS_DEU_DMA_H

#include <linux/init.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/crypto.h>
#include <linux/scatterlist.h>
#include <asm/byteorder.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/version.h>

// must match the size of memory block allocated for g_dma_block and g_dma_block2
#define DEU_MAX_PACKET_SIZE    (PAGE_SIZE >> 1)

typedef struct ifx_deu_device {
	struct dma_device_info *dma_device;
	u8 *dst;
	u8 *src;
	int len;
	int dst_count;
	int src_count;
	int recv_count;
	int packet_size;
	int packet_num;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,13,0))
	wait_queue_entry_t wait;
#else
	wait_queue_t wait;
#endif
} _ifx_deu_device;

extern _ifx_deu_device ifx_deu[1];

extern int deu_dma_intr_handler (struct dma_device_info *, int);
extern u8 *deu_dma_buffer_alloc (int, int *, void **);
extern int deu_dma_buffer_free (u8 *, void *);
extern void deu_dma_inactivate_poll(struct dma_device_info* dma_dev);
extern void deu_dma_activate_poll (struct dma_device_info* dma_dev);
extern struct dma_device_info* deu_dma_reserve(struct dma_device_info** dma_device);
extern int deu_dma_release(struct dma_device_info** dma_device);

#endif	/* IFMIPS_DEU_DMA_H */
