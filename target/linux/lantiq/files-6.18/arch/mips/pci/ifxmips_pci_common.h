/******************************************************************************
**
** FILE NAME    : ifxmips_pci_common.h
** PROJECT      : IFX UEIP
** MODULES      : PCI subsystem
**
** DATE         : 30 June 2009
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
** 0.0.1    30 June,2009  Lei Chuanhua    Initial version
*******************************************************************************/

#ifndef IFXMIPS_PCI_COMMON_H
#define IFXMIPS_PCI_COMMON_H
#include <linux/version.h>
/*!
 \defgroup IFX_PCI_COM  IFX PCI/PCIe common parts for OS integration
 \brief  PCI/PCIe common parts
*/

/*!
 \defgroup IFX_PCI_COM_OS OS APIs
 \ingroup IFX_PCI_COM
 \brief PCI/PCIe bus driver OS interface functions
*/
/*!
  \file ifxmips_pci_common.h
  \ingroup IFX_PCI_COM
  \brief PCI/PCIe bus driver common OS header file
*/
#define IFX_PCI_CONST const
#ifdef CONFIG_IFX_PCI
extern int ifx_pci_bios_map_irq(IFX_PCI_CONST struct pci_dev *dev, u8 slot, u8 pin);
extern int ifx_pci_bios_plat_dev_init(struct pci_dev *dev);
#endif /* COFNIG_IFX_PCI */

#ifdef CONFIG_PCIE_LANTIQ
extern int ifx_pcie_bios_map_irq(IFX_PCI_CONST struct pci_dev *dev, u8 slot, u8 pin);
extern int ifx_pcie_bios_plat_dev_init(struct pci_dev *dev);
#endif

#endif /* IFXMIPS_PCI_COMMON_H */

