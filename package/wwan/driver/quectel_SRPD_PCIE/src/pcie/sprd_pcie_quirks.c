/*
 *  This file contains work-arounds for many known PCI hardware
 *  bugs.  Devices present only on certain architectures (host
 *  bridges et cetera) should be handled in arch-specific code.
 *
 *  Note: any quirks for hotpluggable devices must _NOT_ be declared __init.
 *
 *  Copyright (c) 1999 Martin Mares <mj@ucw.cz>
 *
 *  Init/reset quirks for USB host controllers should be in the
 *  USB quirks file, where their drivers can access reuse it.
 */

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/export.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/acpi.h>
#include <linux/kallsyms.h>
#include <linux/dmi.h>
#include <linux/version.h>
#if (LINUX_VERSION_CODE < KERNEL_VERSION( 5,4,0 ))
#include <linux/pci-aspm.h>
#endif
#include <linux/ioport.h>
#include <linux/sched.h>
#include <linux/ktime.h>
#include <linux/mm.h>
#include <asm/dma.h>	/* isa_dma_bridge_buggy */

#ifndef PCI_VENDOR_ID_SYNOPSYS
#define PCI_VENDOR_ID_SYNOPSYS 0x16c3
#endif


 /*
 * It's possible that ep bar size is larger than rc allocated
 * memory, so need to resize ep bar to small size.
 * Original ep bar size:bar0:256MB, bar1:64kb, bar2:256MB,
 * bar3: 64kb, bar4:256MB, bar5:64kb.
 * resize to bar0:8MB, bar1:64kb, bar2:2MB, bar3: 64kb,
 * bar4:2MB, bar5:64kb.
 */
#define SPRD_PCI_BAR0	0x10
#define SPRD_BAR_NUM		0x6
#define SPRD_PCI_MISC_CTRL1_OFF	0x8bc
#define SPRD_PCI_DBI_RO_WR_EN	(0x1 << 0)
#define SPRD_PCI_RESIZABLE_BAR_EXTENDED_CAP_HEADER	0x260
#define SPRD_PCI_RESIZABLE_BAR_EXTENDED_CAPID		0x15
/* Resizable BAR Capability Register */
#define SPRD_PCI_RESIZABLE_BAR0		0x264
#define SPRD_PCI_RESIZABLE_BAR2		0x26c
#define SPRD_PCI_RESIZABLE_BAR4		0x274
#define SPRD_BAR_SUPPORT_2MB (0x1 << 5)
#define SPRD_BAR_SUPPORT_4MB (0x1 << 6)
#define SPRD_BAR_SUPPORT_8MB (0x1 << 7)
/* Resizable BAR Control Register */
#define SPRD_PCI_RESIZABLE_BAR0_CTL		0x268
#define SPRD_PCI_RESIZABLE_BAR2_CTL		0x270
#define SPRD_PCI_RESIZABLE_BAR4_CTL		0x278
/* bit[13:8] is bar size */
#define SPRD_PCI_RESIZABLE_BAR_SIZE_MASK 0x3F00
#define SPRD_PCI_RESIZABLE_2MB		(0x1 << 8)
#define SPRD_PCI_RESIZABLE_4MB		(0x2 << 8)
#define SPRD_PCI_RESIZABLE_8MB		(0x3 << 8)
#define SIZE(val) ((~(val & 0xFFFFFFF0)) + 1)

static void quirk_sprd_pci_resizebar(struct pci_dev *dev)
{
	u32 val, i, backup;

	pci_read_config_dword(dev,
		SPRD_PCI_RESIZABLE_BAR_EXTENDED_CAP_HEADER, &val);
	if ((val & SPRD_PCI_RESIZABLE_BAR_EXTENDED_CAPID) !=
	     SPRD_PCI_RESIZABLE_BAR_EXTENDED_CAPID) {
		dev_info(&dev->dev, "%s: not support resize bar\n", __func__);
		return;
	}

	pci_read_config_dword(dev, SPRD_PCI_MISC_CTRL1_OFF, &val);
	val |= SPRD_PCI_DBI_RO_WR_EN;
	pci_write_config_dword(dev, SPRD_PCI_MISC_CTRL1_OFF, val);

	pci_read_config_dword(dev,  SPRD_PCI_RESIZABLE_BAR0, &val);
	pci_write_config_dword(dev, SPRD_PCI_RESIZABLE_BAR0,
				 val | SPRD_BAR_SUPPORT_4MB |
				 SPRD_BAR_SUPPORT_8MB);
	pci_read_config_dword(dev, SPRD_PCI_RESIZABLE_BAR2, &val);
	pci_write_config_dword(dev, SPRD_PCI_RESIZABLE_BAR2,
				 val | SPRD_BAR_SUPPORT_4MB |
				 SPRD_BAR_SUPPORT_8MB);
	pci_read_config_dword(dev, SPRD_PCI_RESIZABLE_BAR4, &val);
	pci_write_config_dword(dev, SPRD_PCI_RESIZABLE_BAR4,
				 val | SPRD_BAR_SUPPORT_4MB |
				 SPRD_BAR_SUPPORT_8MB);

	pci_read_config_dword(dev, SPRD_PCI_MISC_CTRL1_OFF, &val);
	val &= ~SPRD_PCI_DBI_RO_WR_EN;
	pci_write_config_dword(dev, SPRD_PCI_MISC_CTRL1_OFF, val);

	pci_read_config_dword(dev, SPRD_PCI_RESIZABLE_BAR0_CTL, &val);
	pci_write_config_dword(dev, SPRD_PCI_RESIZABLE_BAR0_CTL,
				 (val & (~SPRD_PCI_RESIZABLE_BAR_SIZE_MASK)) |
				 SPRD_PCI_RESIZABLE_4MB);
	pci_read_config_dword(dev, SPRD_PCI_RESIZABLE_BAR2_CTL, &val);
	pci_write_config_dword(dev, SPRD_PCI_RESIZABLE_BAR2_CTL,
				 (val & (~SPRD_PCI_RESIZABLE_BAR_SIZE_MASK)) |
				 SPRD_PCI_RESIZABLE_4MB);
	pci_read_config_dword(dev, SPRD_PCI_RESIZABLE_BAR4_CTL, &val);
	pci_write_config_dword(dev, SPRD_PCI_RESIZABLE_BAR4_CTL,
				 (val & (~SPRD_PCI_RESIZABLE_BAR_SIZE_MASK)) |
				 SPRD_PCI_RESIZABLE_4MB);

	for (i = 0; i < SPRD_BAR_NUM; i++) {
		pci_read_config_dword(dev, SPRD_PCI_BAR0 + i * 4, &backup);
		pci_write_config_dword(dev, SPRD_PCI_BAR0 + i * 4, 0xFFFFFFFF);
		pci_read_config_dword(dev, SPRD_PCI_BAR0 + i * 4, &val);
		pci_write_config_dword(dev, SPRD_PCI_BAR0 + i * 4, backup);

		dev_info(&dev->dev, "%s: bar%d size 0x%x\n",
			__func__, i, SIZE(val));
	}
}
DECLARE_PCI_FIXUP_EARLY(PCI_VENDOR_ID_SYNOPSYS,	0xabcd,	quirk_sprd_pci_resizebar);