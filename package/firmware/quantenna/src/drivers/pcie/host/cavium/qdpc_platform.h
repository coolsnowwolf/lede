/**
 * Copyright (c) 2011-2013 Quantenna Communications, Inc.
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 **/
#ifndef __QDPC_PFDEP_H__
#define __QDPC_PFDEP_H__

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27)
#define IOREMAP     ioremap_wc
#else   /* LINUX_VERSION_CODE < KERNEL_VERSION(2,6,27) */
#define IOREMAP     ioremap
#endif

#ifdef QDPC_CVM_DMA
#include <asm/octeon/cvmx.h>
#include "cvmx-dma-engine.h"

int qdpc_init_dma_func(void);
int qdpc_do_dma_transfer(u_int8_t dma_type, u_int64_t local_addr,
                    u_int64_t host_addr, u_int32_t size);
#endif /* QDPC_CVM_DMA */

#define SRAM_TEXT
#define SRAM_DATA

int qdpc_platform_init(void);
void qdpc_platform_exit(void);
void qdpc_platform_xfer(void *dst, void *src, unsigned int len);

#endif /* __QDPC_PFDEP_H__ */




