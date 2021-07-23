/**
 * Copyright (c) 2012-2012 Quantenna Communications, Inc.
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

#ifndef __QDPC_DSP_H__
#define __QDPC_DSP_H__

#include <linux/version.h>

#include <topaz_vnet.h>

#include <qtn/topaz_tqe_cpuif.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27)
#define IOREMAP     ioremap_nocache
#else   /* LINUX_VERSION_CODE < KERNEL_VERSION(2,6,27) */
#define IOREMAP     ioremap
#endif

#define QTN_TXBF_MUC_TO_DSP_MBOX_INT            (0)
#define QTN_TXBF_DSP_TO_HOST_MBOX_INT           (0)

#define QDCP_DSP_FILE_NAME "/etc/firmware/rdsp_driver.0.bin"

RUBY_INLINE void
qtn_txbf_lhost_init(void)
{
#if CONFIG_USE_SPI1_FOR_IPC
        /* Initialize SPI controller, keep IRQ disabled */
        qtn_mproc_sync_mem_write(RUBY_SPI1_SPCR,
                RUBY_SPI1_SPCR_SPE | RUBY_SPI1_SPCR_MSTR |
                RUBY_SPI1_SPCR_SPR(0));
        qtn_mproc_sync_mem_write(RUBY_SPI1_SPER,
                RUBY_SPI1_SPER_ESPR(0));
#else
        /* Ack, and keep IRQ disabled */
        qtn_mproc_sync_mem_write(RUBY_SYS_CTL_D2L_INT,
                qtn_mproc_sync_mem_read(RUBY_SYS_CTL_D2L_INT));
        qtn_mproc_sync_mem_write(RUBY_SYS_CTL_D2L_INT_MASK,
                ~(1 << QTN_TXBF_DSP_TO_HOST_MBOX_INT));
#endif
}

extern int qdpc_dsp_open(void);

#endif /* __QDPC_PFDEP_H__ */

