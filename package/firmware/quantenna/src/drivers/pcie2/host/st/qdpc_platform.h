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

/*
 * Platform dependant implement. Customer needs to modify this file.
 */
#ifndef __QDPC_PFDEP_H__
#define __QDPC_PFDEP_H__

#include <linux/version.h>

#include <topaz_vnet.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27)
#define IOREMAP      ioremap_wc
#else    /* LINUX_VERSION_CODE < KERNEL_VERSION(2,6,27) */
#define IOREMAP      ioremap
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

#ifndef virt_to_bus
#define virt_to_bus virt_to_phys
#endif

/* Bit number and mask of MSI in the interrupt mask and status register */
#define	QDPC_INTR_MSI_BIT		0
#define QDPC_INTR_MSI_MASK		(1 << QDPC_INTR_MSI_BIT)

/* Enable MSI interrupt of PCIe */
extern void enable_vmac_ints(struct vmac_priv *vmp);
/* Disable MSI interrupt of PCIe */
extern void disable_vmac_ints(struct vmac_priv *vmp);

/* Enable interrupt for detecting EP reset */
extern void enable_ep_rst_detection(struct net_device *ndev);
/* Disable interrupt for detecting EP reset */
extern void disable_ep_rst_detection(struct net_device *ndev);
/* Interrupt context for detecting EP reset */
extern void handle_ep_rst_int(struct net_device *ndev);

/* Allocated buffer size for a packet */
#define SKB_BUF_SIZE		2048

/* Transmit Queue Length */
#define QDPC_TX_QUEUE_SIZE	180

/* Receive Queue Length */
#define QDPC_RX_QUEUE_SIZE	384

/* Customer defined function	*/
#define qdpc_platform_init()                  0
#define qdpc_platform_exit()                  do { } while(0)

/* PCIe driver update resource in PCI configure space after EP reset */
#define qdpc_update_hw_bar(pdev, index)       do { } while(0)

/* TODO: If IRQ-loss issue can be fixed, remove macro below */
#define QDPC_PLATFORM_IRQ_FIXUP

#endif /* __QDPC_PFDEP_H__ */

