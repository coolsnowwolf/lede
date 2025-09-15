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
#include <linux/version.h> 
#include <linux/pci.h>
#include "qdpc_platform.h"

int qdpc_platform_init(void) 
{
#ifdef QDPC_CVM_DMA
	if(qdpc_init_dma_func()) {
		printk("Cavium DMA init failed \n");
		return -1;
	}
#endif /* QDPC_CVM_DMA */

	return 0;
}

void qdpc_platform_exit(void)
{
#ifdef QDPC_CVM_DMA
	/* Shutting down DMA engine  */
	cvmx_dma_engine_shutdown();
#endif /* QDPC_CVM_DMA */

	return;
}

void qdpc_platform_xfer(void *dst, void *src, unsigned int len)
{
#ifdef QDPC_CVM_DMA
	qdpc_do_dma_transfer(CVMX_DMA_ENGINE_TRANSFER_OUTBOUND, cvmx_ptr_to_phys(src),
        	        cvmx_ptr_to_phys(dst), len);
#else   /* QDPC_CVM_DMA */
	/* Copying skb data into packet buffer */
	memcpy_toio(dst, src, len);
#endif

	return;
}

