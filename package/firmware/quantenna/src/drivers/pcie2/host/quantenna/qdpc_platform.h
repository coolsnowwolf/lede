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

#ifndef __QDPC_PFDEP_H__
#define __QDPC_PFDEP_H__

#include <linux/version.h>

#include <topaz_vnet.h>

#include <qtn/topaz_tqe_cpuif.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27)
#define IOREMAP     ioremap_nocache
#else   /* LINUX_VERSION_CODE < KERNEL_VERSION(2,6,27) */
#define IOREMAP     ioremap
#endif

/* IO functions */
#ifndef readb
#define readb(addr) (*(volatile unsigned char *) (addr))
#endif

#ifndef readw
#define readw(addr) (*(volatile unsigned short *) (addr))
#endif

#ifndef readl
#define readl(addr) (*(volatile unsigned int *) (addr))
#endif

#ifndef writeb
#define writeb(b,addr) (*(volatile unsigned char *) (addr) = (b))
#endif

#ifndef writew
#define writew(b,addr) (*(volatile unsigned short *) (addr) = (b))
#endif

#ifndef writel
#define writel(b,addr) (*(volatile unsigned int *) (addr) = (b))
#endif

/*
 * Interrupt
 */
/* Interrupt Mask and Status Reigster */
#define QDPC_RC_SYS_CTL_BASE		0xe0000000
#define QDPC_RC_SYS_CTL_PCIE_INT_MASK	(QDPC_RC_SYS_CTL_BASE + 0xC0)
#define QDPC_RC_SYS_CTL_PCIE_INT_STAT	(QDPC_RC_SYS_CTL_BASE + 0x17C)

/* Bit number and mask of MSI in the interrupt mask and status register */
#define	QDPC_INTR_MSI_BIT		12
#define QDPC_INTR_MSI_MASK		(1 << QDPC_INTR_MSI_BIT)

/* Bit number and mask of EP-reset-detect Interrupt in the mask and status register */
#define QDPC_INTR_EP_RST_BIT		3
#define QDPC_INTR_EP_RST_MASK		(1 << QDPC_INTR_EP_RST_BIT)

extern void enable_vmac_ints(struct vmac_priv *vmp);
extern void disable_vmac_ints(struct vmac_priv *vmp);

extern void enable_ep_rst_detection(struct net_device *ndev);
extern void disable_ep_rst_detection(struct net_device *ndev);
extern void handle_ep_rst_int(struct net_device *ndev);

extern int qdpc_wps_button_init(struct net_device *dev);
extern void qdpc_wps_button_exit(void);

/* Allocated buffer size for a packet */
#define SKB_BUF_SIZE		RX_BUF_SIZE

/* Transmit Queue Length */
#if defined(QTN_BYTEALIGN)
#define QDPC_TX_QUEUE_SIZE	180
#else
#define QDPC_TX_QUEUE_SIZE	200
#endif

/* Receive Queue Length */
#define QDPC_RX_QUEUE_SIZE	384

/* SDP requires packets show up at Lhost */
#define QDPC_PLATFORM_IFPORT	TOPAZ_TQE_LHOST_PORT

/* Customer defined function	*/
#define qdpc_platform_init()                  0
#define qdpc_platform_exit()                  do { } while(0)

/* PCIe driver update resource in PCI configure space after EP reset */
#define qdpc_update_hw_bar(pdev, index)       do { } while(0)

#endif /* __QDPC_PFDEP_H__ */

