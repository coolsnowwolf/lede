/******************************************************************************
**
** FILE NAME    : ifxmips_pcie.h
** PROJECT      : IFX UEIP for VRX200
** MODULES      : PCIe module
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
#ifndef IFXMIPS_PCIE_H
#define IFXMIPS_PCIE_H
#include <linux/version.h>
#include <linux/types.h>
#include <linux/pci.h>
#include <linux/interrupt.h>
#include "ifxmips_pci_common.h"
#include "ifxmips_pcie_reg.h"

/*!
 \defgroup IFX_PCIE  PCI Express bus driver module
 \brief  PCI Express IP module support VRX200
*/

/*!
 \defgroup IFX_PCIE_OS OS APIs
 \ingroup IFX_PCIE
 \brief PCIe bus driver OS interface functions
*/

/*!
 \file ifxmips_pcie.h
 \ingroup IFX_PCIE
 \brief header file for PCIe module common header file
*/
#define PCIE_IRQ_LOCK(lock) do {             \
    unsigned long flags;                     \
    spin_lock_irqsave(&(lock), flags);
#define PCIE_IRQ_UNLOCK(lock)                \
    spin_unlock_irqrestore(&(lock), flags);  \
} while (0)

#define PCIE_MSG_MSI        0x00000001
#define PCIE_MSG_ISR        0x00000002
#define PCIE_MSG_FIXUP      0x00000004
#define PCIE_MSG_READ_CFG   0x00000008
#define PCIE_MSG_WRITE_CFG  0x00000010
#define PCIE_MSG_CFG        (PCIE_MSG_READ_CFG | PCIE_MSG_WRITE_CFG)
#define PCIE_MSG_REG        0x00000020
#define PCIE_MSG_INIT       0x00000040
#define PCIE_MSG_ERR        0x00000080
#define PCIE_MSG_PHY        0x00000100
#define PCIE_MSG_ANY        0x000001ff

#define IFX_PCIE_PORT0      0
#define IFX_PCIE_PORT1      1

#ifdef CONFIG_IFX_PCIE_2ND_CORE
#define IFX_PCIE_CORE_NR    2
#else
#define IFX_PCIE_CORE_NR    1
#endif

#define IFX_PCIE_ERROR_INT

//#define IFX_PCIE_DBG

#if defined(IFX_PCIE_DBG)
#define IFX_PCIE_PRINT(_m, _fmt, args...) do {   \
        ifx_pcie_debug((_fmt), ##args);          \
} while (0)

#define INLINE
#else
#define IFX_PCIE_PRINT(_m, _fmt, args...)   \
    do {} while(0)
#define INLINE inline
#endif

struct ifx_pci_controller {
	struct pci_controller   pcic;

	/* RC specific, per host bus information */
	u32   port;  /* Port index, 0 -- 1st core, 1 -- 2nd core */
};

typedef struct ifx_pcie_ir_irq {
    unsigned int irq;
    const char name[16];
}ifx_pcie_ir_irq_t;

typedef struct ifx_pcie_legacy_irq{
    const u32 irq_bit;
    int irq;
}ifx_pcie_legacy_irq_t;

typedef struct ifx_pcie_irq {
    ifx_pcie_ir_irq_t ir_irq;
    ifx_pcie_legacy_irq_t legacy_irq[PCIE_LEGACY_INT_MAX];
}ifx_pcie_irq_t;

extern u32 g_pcie_debug_flag;
extern void ifx_pcie_debug(const char *fmt, ...);
extern void pcie_phy_clock_mode_setup(int pcie_port);
extern void pcie_msi_pic_init(int pcie_port);
extern u32 ifx_pcie_bus_enum_read_hack(int where, u32 value);
extern u32 ifx_pcie_bus_enum_write_hack(int where, u32 value);

#define CONFIG_VR9

#ifdef CONFIG_VR9
#include "ifxmips_pcie_vr9.h"
#elif defined (CONFIG_AR10)
#include "ifxmips_pcie_ar10.h"
#else
#error "PCIE: platform not defined"
#endif /* CONFIG_VR9 */

#endif  /* IFXMIPS_PCIE_H */

