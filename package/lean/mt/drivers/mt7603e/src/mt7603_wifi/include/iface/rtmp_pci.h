/****************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ****************************************************************************

    Module Name:
	rtmp_pci.h
 
    Abstract:
 
    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
*/

#ifndef __RTMP_PCI_H__
#define __RTMP_PCI_H__

#include "iface/rtmp_inf_pcirbs.h"

/*
	Device ID & Vendor ID related definitions,
	NOTE: you should not add the new VendorID/DeviceID here unless you 
		not sure it belongs to what chip.
*/

#define PCIBUS_INTEL_VENDOR	0x8086

#if !defined(PCI_CAP_ID_EXP)
#define PCI_CAP_ID_EXP			    0x10
#endif
#if !defined(PCI_EXP_LNKCTL)
#define PCI_EXP_LNKCTL			    0x10
#endif
#if !defined(PCI_CLASS_BRIDGE_PCI)
#define PCI_CLASS_BRIDGE_PCI		0x0604
#endif


#define SYSCFG0			0x10

/*
	PCI registers - base address 0x0000
*/
#define PCI_CFG			0x0000
#define PCI_EECTRL			0x0004
#define PCI_MCUCTRL			0x0008
#define AUX_CTRL		0x10c

#define	OPT_14			0x114


#define RT28XX_HANDLE_DEV_ASSIGN(handle, dev_p)				\
	((POS_COOKIE)handle)->pci_dev = dev_p;


#ifdef LINUX
/* set driver data */
#define RT28XX_DRVDATA_SET(_a)			pci_set_drvdata(_a, net_dev);

#define RT28XX_PUT_DEVICE(dev_p)


#define RTMP_PCI_DEV_UNMAP()						\
{	if (net_dev->base_addr)	{						\
		iounmap((void *)(net_dev->base_addr));			\
		release_mem_region(pci_resource_start(dev_p, 0),	\
							pci_resource_len(dev_p, 0)); }	\
	if (net_dev->irq) pci_release_regions(dev_p); }


#define PCI_REG_READ_WORD(pci_dev, offset, Configuration)   \
    if (pci_read_config_word(pci_dev, offset, &reg16) == 0)     \
        Configuration = le2cpu16(reg16);                        \
    else                                                        \
        Configuration = 0;

#define PCI_REG_WIRTE_WORD(pci_dev, offset, Configuration)  \
    reg16 = cpu2le16(Configuration);                        \
    pci_write_config_word(pci_dev, offset, reg16);
#endif /* LINUX */

#endif /* __RTMP_PCI_H__ */
