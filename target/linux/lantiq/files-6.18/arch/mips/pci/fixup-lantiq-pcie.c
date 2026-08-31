/******************************************************************************
**
** FILE NAME    : ifxmips_fixup_pcie.c
** PROJECT      : IFX UEIP for VRX200
** MODULES      : PCIe
**
** DATE         : 02 Mar 2009
** AUTHOR       : Lei Chuanhua
** DESCRIPTION  : PCIe Root Complex Driver
** COPYRIGHT    :       Copyright (c) 2009
**                      Infineon Technologies AG
**                      Am Campeon 1-12, 85579 Neubiberg, Germany
**
**    This program is free software; you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation; either version 2 of the License, or
**    (at your option) any later version.
** HISTORY
** $Version $Date        $Author         $Comment
** 0.0.1    17 Mar,2009  Lei Chuanhua    Initial version
*******************************************************************************/
/*!
 \file ifxmips_fixup_pcie.c
 \ingroup IFX_PCIE
 \brief PCIe Fixup functions source file
*/
#include <linux/pci.h>
#include <linux/pci_regs.h>
#include <linux/pci_ids.h>

#include <lantiq_soc.h>

#include "pcie-lantiq.h"

static void
ifx_pcie_fixup_resource(struct pci_dev *dev)
{
    u32 reg;

    IFX_PCIE_PRINT(PCIE_MSG_FIXUP, "%s dev %s: enter\n", __func__, pci_name(dev));

    printk("%s: fixup host controller %s (%04x:%04x)\n",
        __func__, pci_name(dev), dev->vendor, dev->device);

   /* Setup COMMAND register */
    reg = PCI_COMMAND_IO | PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER /* |
          PCI_COMMAND_INTX_DISABLE */| PCI_COMMAND_SERR;
    pci_write_config_word(dev, PCI_COMMAND, reg);
    IFX_PCIE_PRINT(PCIE_MSG_FIXUP, "%s dev %s: exit\n", __func__, pci_name(dev));
}
DECLARE_PCI_FIXUP_EARLY(PCI_VENDOR_ID_INFINEON, PCI_DEVICE_ID_INFINEON_PCIE, ifx_pcie_fixup_resource);
DECLARE_PCI_FIXUP_EARLY(PCI_VENDOR_ID_LANTIQ, PCI_VENDOR_ID_LANTIQ, ifx_pcie_fixup_resource);

static void
ifx_pcie_rc_class_early_fixup(struct pci_dev *dev)
{
    IFX_PCIE_PRINT(PCIE_MSG_FIXUP, "%s dev %s: enter\n", __func__, pci_name(dev));

    if (dev->devfn == PCI_DEVFN(0, 0) &&
        (dev->class >> 8) == PCI_CLASS_BRIDGE_HOST) {

        dev->class = (PCI_CLASS_BRIDGE_PCI << 8) | (dev->class & 0xff);

        printk(KERN_INFO "%s: fixed pcie host bridge to pci-pci bridge\n", __func__);
    }
    IFX_PCIE_PRINT(PCIE_MSG_FIXUP, "%s dev %s: exit\n", __func__, pci_name(dev));
    mdelay(10);
}

DECLARE_PCI_FIXUP_EARLY(PCI_VENDOR_ID_INFINEON, PCI_DEVICE_ID_INFINEON_PCIE,
     ifx_pcie_rc_class_early_fixup);

DECLARE_PCI_FIXUP_EARLY(PCI_VENDOR_ID_LANTIQ, PCI_DEVICE_ID_LANTIQ_PCIE,
     ifx_pcie_rc_class_early_fixup);
