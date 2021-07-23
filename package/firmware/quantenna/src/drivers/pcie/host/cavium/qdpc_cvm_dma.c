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
#ifdef QDPC_CVM_DMA

#include <asm/octeon/cvmx.h>
#include "cvmx-cmd-queue.h"
#include "cvmx-version.h"
#include "cvmx-atomic.h"
#include "cvmx-pip.h"
#include "cvmx-ipd.h"
#include "cvmx-pow.h"
#include "cvmx-spi.h"
#include "cvmx-bootmem.h"
#include "cvmx-app-init.h"
#include "cvmx-helper.h"
#include "cvmx-helper-board.h"
#include "cvmx-npei-defs.h"
#include "cvmx-pexp-defs.h"
#include <asm/octeon/cvmx-fpa.h>
#include "cvmx-dma-engine.h"
#include "cvmx-helper-fpa.h"
#include "cvmx-pcie.h"
#include "cvmx-pciercx-defs.h"
#include "cvmx-pescx-defs.h"
#include "cvmx-pemx-defs.h"
#include <linux/version.h>
#include "qdpc_platform.h"

#define BYTE_SWAP_MODE_64b       1
#define NUM_PACKET_BUFFERS       1024

#define PRINT_ERROR              printk

extern int cvm_oct_mem_fill_fpa(int pool, int size, int elements);

int qdpc_pcie_port = 0;


int qdpc_init_dma_func(void)
{
	cvmx_npei_dma_control_t dma_control;
        int port = 0, value = 0;

	/* Free pool buffers are allocated. These are used by DMA engine. */
	cvmx_fpa_enable();
	cvm_oct_mem_fill_fpa(CVMX_FPA_PACKET_POOL, CVMX_FPA_PACKET_POOL_SIZE, NUM_PACKET_BUFFERS);
	cvm_oct_mem_fill_fpa(CVMX_FPA_WQE_POOL, CVMX_FPA_WQE_POOL_SIZE, NUM_PACKET_BUFFERS);

	if (CVMX_FPA_OUTPUT_BUFFER_POOL != CVMX_FPA_PACKET_POOL) {
	   	cvm_oct_mem_fill_fpa(CVMX_FPA_OUTPUT_BUFFER_POOL, 
                                     CVMX_FPA_OUTPUT_BUFFER_POOL_SIZE, 128);
	}

	cvmx_helper_setup_red(NUM_PACKET_BUFFERS/4, NUM_PACKET_BUFFERS/8);

	/* Initializing DMA engine */
	if(cvmx_dma_engine_initialize()) {
    		PRINT_ERROR(KERN_ERR " cvmx_dma_engine_initialize failed\n");
	    	return -1;
	}

	dma_control.u64 = 0;
	dma_control.u64 = cvmx_read_csr(CVMX_PEXP_NPEI_DMA_CONTROL);
	/* Enable endian-swap mode for DMA. */
	dma_control.s.o_es = BYTE_SWAP_MODE_64b;
	/* 
         * 1 = use pointer values for address and register values for O_RO, O_ES, and O_NS.
	 * 0 = use register values for address and pointer values for O_RO, O_ES, and O_NS.
	 */
	dma_control.s.o_mode = 1;
	cvmx_write_csr(CVMX_PEXP_NPEI_DMA_CONTROL, dma_control.u64);

	for(port = 0 ; port < 2; port ++) {
		value = cvmx_pcie_cfgx_read(port, CVMX_PCIERCX_CFG032(port));
		if(value & ( 1<< 29)) {
			qdpc_pcie_port = port;
			break;   /* Assuming only one pcie card is connected to Cavium board */
		}
	}

	return 0;
}

uint32_t cvmx_pcie_cfgx_read(int port, uint32_t cfg_offset)
{
	if (octeon_has_feature(OCTEON_FEATURE_NPEI)) {
		cvmx_pescx_cfg_rd_t pescx_cfg_rd;
		pescx_cfg_rd.u64 = 0;
		pescx_cfg_rd.s.addr = cfg_offset;
		cvmx_write_csr(CVMX_PESCX_CFG_RD(port), pescx_cfg_rd.u64);
		pescx_cfg_rd.u64 = cvmx_read_csr(CVMX_PESCX_CFG_RD(port));

		return pescx_cfg_rd.s.data;
	}
	else {
		cvmx_pemx_cfg_rd_t pemx_cfg_rd;
		pemx_cfg_rd.u64 = 0;
		pemx_cfg_rd.s.addr = cfg_offset;
		cvmx_write_csr(CVMX_PEMX_CFG_RD(port), pemx_cfg_rd.u64);
		pemx_cfg_rd.u64 = cvmx_read_csr(CVMX_PEMX_CFG_RD(port));

		return pemx_cfg_rd.s.data;
	}
}


int qdpc_do_dma_transfer(u_int8_t dma_type, u_int64_t local_addr, u_int64_t host_addr, u_int32_t size)
{
	cvmx_dma_engine_header_t header;
	volatile u_int8_t  stat;
	int count = 0;
	
	header.u64     = 0;
	header.s.type  = dma_type;
	header.s.lport = qdpc_pcie_port; /* Physical hardware pcie port on which pcie read/write takes place */
	
	/* Initializing status to some value */
	stat = 0x65;
	header.s.addr = cvmx_ptr_to_phys((void *)&stat); /* DMA completion status */
	    	 
	/* DMA transfer. Dma engine number - 1.(0 - 4). 
	 * One engine is assigned for DMAing user data
	 * between host and octeon user mgmt app.
	 */
	if(cvmx_dma_engine_transfer(0 , header, local_addr, host_addr & 0xffffffff, size)) {
		PRINT_ERROR("cvmx_dma_engine_transfer: qdpc_do_dma_transfer failed\n");
		return -1;
	}
	
	while((stat != 0) &&(count < 10000)) { /* Checking DMA completion status */
		CVMX_SYNCW;
		count ++;
	}
	
	if(stat) {
		PRINT_ERROR("qdpc_do_dma_transfer failed \n");
		return -1;
	}
	   
	return 0;
}

#endif /* QDPC_CVM_DMA */
