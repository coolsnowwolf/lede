/******************************************************************************
**
** FILE NAME    : ifxmips_pcie_msi.c
** PROJECT      : IFX UEIP for VRX200
** MODULES      : PCI MSI sub module
**
** DATE         : 02 Mar 2009
** AUTHOR       : Lei Chuanhua
** DESCRIPTION  : PCIe MSI Driver
** COPYRIGHT    :       Copyright (c) 2009
**                      Infineon Technologies AG
**                      Am Campeon 1-12, 85579 Neubiberg, Germany
**
**    This program is free software; you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation; either version 2 of the License, or
**    (at your option) any later version.
** HISTORY
** $Date        $Author         $Comment
** 02 Mar,2009  Lei Chuanhua    Initial version
*******************************************************************************/
/*!
 \defgroup IFX_PCIE_MSI MSI OS APIs
 \ingroup IFX_PCIE
 \brief PCIe bus driver OS interface functions
*/

/*!
 \file ifxmips_pcie_msi.c
 \ingroup IFX_PCIE
 \brief PCIe MSI OS interface file
*/

#ifndef AUTOCONF_INCLUDED
#include <linux/config.h>
#endif /* AUTOCONF_INCLUDED */
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/kernel_stat.h>
#include <linux/pci.h>
#include <linux/msi.h>
#include <linux/module.h>
#include <asm/bootinfo.h>
#include <asm/irq.h>
#include <asm/traps.h>

#include <asm/ifx/ifx_types.h>
#include <asm/ifx/ifx_regs.h>
#include <asm/ifx/common_routines.h>
#include <asm/ifx/irq.h>

#include "ifxmips_pcie_reg.h"
#include "ifxmips_pcie.h"

#define IFX_MSI_IRQ_NUM    16

enum {
    IFX_PCIE_MSI_IDX0 = 0,
    IFX_PCIE_MSI_IDX1,
    IFX_PCIE_MSI_IDX2,
    IFX_PCIE_MSI_IDX3,
};

typedef struct ifx_msi_irq_idx {
    const int irq;
    const int idx;
}ifx_msi_irq_idx_t;

struct ifx_msi_pic {
    volatile u32  pic_table[IFX_MSI_IRQ_NUM];
    volatile u32  pic_endian;    /* 0x40  */
};
typedef struct ifx_msi_pic *ifx_msi_pic_t;

typedef struct ifx_msi_irq {
    const volatile ifx_msi_pic_t msi_pic_p;
    const u32 msi_phy_base;
    const ifx_msi_irq_idx_t msi_irq_idx[IFX_MSI_IRQ_NUM];
    /*
     * Each bit in msi_free_irq_bitmask represents a MSI interrupt that is
     * in use.
     */
    u16 msi_free_irq_bitmask;

    /*
     * Each bit in msi_multiple_irq_bitmask tells that the device using
     * this bit in msi_free_irq_bitmask is also using the next bit. This
     * is used so we can disable all of the MSI interrupts when a device
     * uses multiple.
     */
    u16 msi_multiple_irq_bitmask;
}ifx_msi_irq_t;

static ifx_msi_irq_t msi_irqs[IFX_PCIE_CORE_NR] = {
    {
        .msi_pic_p = (const volatile ifx_msi_pic_t)IFX_MSI_PIC_REG_BASE,
        .msi_phy_base = PCIE_MSI_PHY_BASE,
        .msi_irq_idx = {
            {IFX_PCIE_MSI_IR0, IFX_PCIE_MSI_IDX0}, {IFX_PCIE_MSI_IR1, IFX_PCIE_MSI_IDX1},
            {IFX_PCIE_MSI_IR2, IFX_PCIE_MSI_IDX2}, {IFX_PCIE_MSI_IR3, IFX_PCIE_MSI_IDX3},
            {IFX_PCIE_MSI_IR0, IFX_PCIE_MSI_IDX0}, {IFX_PCIE_MSI_IR1, IFX_PCIE_MSI_IDX1},
            {IFX_PCIE_MSI_IR2, IFX_PCIE_MSI_IDX2}, {IFX_PCIE_MSI_IR3, IFX_PCIE_MSI_IDX3},
            {IFX_PCIE_MSI_IR0, IFX_PCIE_MSI_IDX0}, {IFX_PCIE_MSI_IR1, IFX_PCIE_MSI_IDX1},
            {IFX_PCIE_MSI_IR2, IFX_PCIE_MSI_IDX2}, {IFX_PCIE_MSI_IR3, IFX_PCIE_MSI_IDX3},
            {IFX_PCIE_MSI_IR0, IFX_PCIE_MSI_IDX0}, {IFX_PCIE_MSI_IR1, IFX_PCIE_MSI_IDX1},
            {IFX_PCIE_MSI_IR2, IFX_PCIE_MSI_IDX2}, {IFX_PCIE_MSI_IR3, IFX_PCIE_MSI_IDX3},
        },
        .msi_free_irq_bitmask = 0,
        .msi_multiple_irq_bitmask= 0,
    },
#ifdef CONFIG_IFX_PCIE_2ND_CORE
    {
        .msi_pic_p = (const volatile ifx_msi_pic_t)IFX_MSI1_PIC_REG_BASE,
        .msi_phy_base = PCIE1_MSI_PHY_BASE,
        .msi_irq_idx = {
            {IFX_PCIE1_MSI_IR0, IFX_PCIE_MSI_IDX0}, {IFX_PCIE1_MSI_IR1, IFX_PCIE_MSI_IDX1},
            {IFX_PCIE1_MSI_IR2, IFX_PCIE_MSI_IDX2}, {IFX_PCIE1_MSI_IR3, IFX_PCIE_MSI_IDX3},
            {IFX_PCIE1_MSI_IR0, IFX_PCIE_MSI_IDX0}, {IFX_PCIE1_MSI_IR1, IFX_PCIE_MSI_IDX1},
            {IFX_PCIE1_MSI_IR2, IFX_PCIE_MSI_IDX2}, {IFX_PCIE1_MSI_IR3, IFX_PCIE_MSI_IDX3},
            {IFX_PCIE1_MSI_IR0, IFX_PCIE_MSI_IDX0}, {IFX_PCIE1_MSI_IR1, IFX_PCIE_MSI_IDX1},
            {IFX_PCIE1_MSI_IR2, IFX_PCIE_MSI_IDX2}, {IFX_PCIE1_MSI_IR3, IFX_PCIE_MSI_IDX3},
            {IFX_PCIE1_MSI_IR0, IFX_PCIE_MSI_IDX0}, {IFX_PCIE1_MSI_IR1, IFX_PCIE_MSI_IDX1},
            {IFX_PCIE1_MSI_IR2, IFX_PCIE_MSI_IDX2}, {IFX_PCIE1_MSI_IR3, IFX_PCIE_MSI_IDX3},
        },
        .msi_free_irq_bitmask = 0,
        .msi_multiple_irq_bitmask= 0,

    },
#endif /* CONFIG_IFX_PCIE_2ND_CORE */
};

/*
 * This lock controls updates to msi_free_irq_bitmask,
 * msi_multiple_irq_bitmask and pic register settting
 */
static DEFINE_SPINLOCK(ifx_pcie_msi_lock);

void pcie_msi_pic_init(int pcie_port)
{
    spin_lock(&ifx_pcie_msi_lock);
    msi_irqs[pcie_port].msi_pic_p->pic_endian = IFX_MSI_PIC_BIG_ENDIAN;
    spin_unlock(&ifx_pcie_msi_lock);
}

/**
 * \fn int arch_setup_msi_irq(struct pci_dev *pdev, struct msi_desc *desc)
 * \brief Called when a driver request MSI interrupts instead of the
 * legacy INT A-D. This routine will allocate multiple interrupts
 * for MSI devices that support them. A device can override this by
 * programming the MSI control bits [6:4] before calling
 * pci_enable_msi().
 *
 * \param[in] pdev   Device requesting MSI interrupts
 * \param[in] desc   MSI descriptor
 *
 * \return   -EINVAL Invalid pcie root port or invalid msi bit
 * \return    0        OK
 * \ingroup IFX_PCIE_MSI
 */
int
arch_setup_msi_irq(struct pci_dev *pdev, struct msi_desc *desc)
{
    int  irq, pos;
    u16  control;
    int  irq_idx;
    int  irq_step;
    int configured_private_bits;
    int request_private_bits;
    struct msi_msg msg;
    u16 search_mask;
    struct ifx_pci_controller *ctrl = pdev->bus->sysdata;
    int pcie_port = ctrl->port;

    IFX_PCIE_PRINT(PCIE_MSG_MSI, "%s %s enter\n", __func__, pci_name(pdev));

    /* XXX, skip RC MSI itself */
    if (pdev->pcie_type == PCI_EXP_TYPE_ROOT_PORT) {
        IFX_PCIE_PRINT(PCIE_MSG_MSI, "%s RC itself doesn't use MSI interrupt\n", __func__);
        return -EINVAL;
    }

    /*
     * Read the MSI config to figure out how many IRQs this device
     * wants.  Most devices only want 1, which will give
     * configured_private_bits and request_private_bits equal 0.
     */
    pci_read_config_word(pdev, desc->msi_attrib.pos + PCI_MSI_FLAGS, &control);

    /*
     * If the number of private bits has been configured then use
     * that value instead of the requested number. This gives the
     * driver the chance to override the number of interrupts
     * before calling pci_enable_msi().
     */
    configured_private_bits = (control & PCI_MSI_FLAGS_QSIZE) >> 4;
    if (configured_private_bits == 0) {
        /* Nothing is configured, so use the hardware requested size */
        request_private_bits = (control & PCI_MSI_FLAGS_QMASK) >> 1;
    }
    else {
        /*
         * Use the number of configured bits, assuming the
         * driver wanted to override the hardware request
         * value.
         */
        request_private_bits = configured_private_bits;
    }

    /*
     * The PCI 2.3 spec mandates that there are at most 32
     * interrupts. If this device asks for more, only give it one.
     */
    if (request_private_bits > 5) {
        request_private_bits = 0;
    }
again:
    /*
     * The IRQs have to be aligned on a power of two based on the
     * number being requested.
     */
    irq_step = (1 << request_private_bits);

    /* Mask with one bit for each IRQ */
    search_mask = (1 << irq_step) - 1;

    /*
     * We're going to search msi_free_irq_bitmask_lock for zero
     * bits. This represents an MSI interrupt number that isn't in
     * use.
     */
    spin_lock(&ifx_pcie_msi_lock);
    for (pos = 0; pos < IFX_MSI_IRQ_NUM; pos += irq_step) {
        if ((msi_irqs[pcie_port].msi_free_irq_bitmask & (search_mask << pos)) == 0) {
            msi_irqs[pcie_port].msi_free_irq_bitmask |= search_mask << pos;
            msi_irqs[pcie_port].msi_multiple_irq_bitmask |= (search_mask >> 1) << pos;
            break;
        }
    }
    spin_unlock(&ifx_pcie_msi_lock);

    /* Make sure the search for available interrupts didn't fail */
    if (pos >= IFX_MSI_IRQ_NUM) {
        if (request_private_bits) {
            IFX_PCIE_PRINT(PCIE_MSG_MSI, "%s: Unable to find %d free "
                  "interrupts, trying just one", __func__, 1 << request_private_bits);
            request_private_bits = 0;
            goto again;
        }
        else {
            printk(KERN_ERR "%s: Unable to find a free MSI interrupt\n", __func__);
            return -EINVAL;
        }
    }
    irq = msi_irqs[pcie_port].msi_irq_idx[pos].irq;
    irq_idx = msi_irqs[pcie_port].msi_irq_idx[pos].idx;

    IFX_PCIE_PRINT(PCIE_MSG_MSI, "pos %d, irq %d irq_idx %d\n", pos, irq, irq_idx);

    /*
     * Initialize MSI. This has to match the memory-write endianess from the device
     * Address bits [23:12]
     */
    spin_lock(&ifx_pcie_msi_lock);
    msi_irqs[pcie_port].msi_pic_p->pic_table[pos] = SM(irq_idx, IFX_MSI_PIC_INT_LINE) |
                    SM((msi_irqs[pcie_port].msi_phy_base >> 12), IFX_MSI_PIC_MSG_ADDR) |
                    SM((1 << pos), IFX_MSI_PIC_MSG_DATA);

    /* Enable this entry */
    msi_irqs[pcie_port].msi_pic_p->pic_table[pos] &= ~IFX_MSI_PCI_INT_DISABLE;
    spin_unlock(&ifx_pcie_msi_lock);

    IFX_PCIE_PRINT(PCIE_MSG_MSI, "pic_table[%d]: 0x%08x\n",
        pos, msi_irqs[pcie_port].msi_pic_p->pic_table[pos]);

    /* Update the number of IRQs the device has available to it */
    control &= ~PCI_MSI_FLAGS_QSIZE;
    control |= (request_private_bits << 4);
    pci_write_config_word(pdev, desc->msi_attrib.pos + PCI_MSI_FLAGS, control);

    set_irq_msi(irq, desc);
    msg.address_hi = 0x0;
    msg.address_lo = msi_irqs[pcie_port].msi_phy_base;
    msg.data = SM((1 << pos), IFX_MSI_PIC_MSG_DATA);
    IFX_PCIE_PRINT(PCIE_MSG_MSI, "msi_data: pos %d 0x%08x\n", pos, msg.data);

    write_msi_msg(irq, &msg);
    IFX_PCIE_PRINT(PCIE_MSG_MSI, "%s exit\n", __func__);
    return 0;
}

static int
pcie_msi_irq_to_port(unsigned int irq, int *port)
{
    int ret = 0;

    if (irq == IFX_PCIE_MSI_IR0 || irq == IFX_PCIE_MSI_IR1 ||
        irq == IFX_PCIE_MSI_IR2 || irq == IFX_PCIE_MSI_IR3) {
        *port = IFX_PCIE_PORT0;
    }
#ifdef CONFIG_IFX_PCIE_2ND_CORE
    else if (irq == IFX_PCIE1_MSI_IR0 || irq == IFX_PCIE1_MSI_IR1 ||
        irq == IFX_PCIE1_MSI_IR2 || irq == IFX_PCIE1_MSI_IR3) {
        *port = IFX_PCIE_PORT1;
    }
#endif /* CONFIG_IFX_PCIE_2ND_CORE */
    else {
        printk(KERN_ERR "%s: Attempted to teardown illegal "
            "MSI interrupt (%d)\n", __func__, irq);
        ret = -EINVAL;
    }
    return ret;
}

/**
 * \fn void arch_teardown_msi_irq(unsigned int irq)
 * \brief Called when a device no longer needs its MSI interrupts. All
 * MSI interrupts for the device are freed.
 *
 * \param irq   The devices first irq number. There may be multple in sequence.
 * \return none
 * \ingroup IFX_PCIE_MSI
 */
void
arch_teardown_msi_irq(unsigned int irq)
{
    int pos;
    int number_irqs;
    u16 bitmask;
    int pcie_port;

    IFX_PCIE_PRINT(PCIE_MSG_MSI, "%s enter\n", __func__);

    BUG_ON(irq > INT_NUM_IM4_IRL31);

    if (pcie_msi_irq_to_port(irq, &pcie_port) != 0) {
        return;
    }

    /* Shift the mask to the correct bit location, not always correct
     * Probally, the first match will be chosen.
     */
    for (pos = 0; pos < IFX_MSI_IRQ_NUM; pos++) {
        if ((msi_irqs[pcie_port].msi_irq_idx[pos].irq == irq)
            && (msi_irqs[pcie_port].msi_free_irq_bitmask & ( 1 << pos))) {
            break;
        }
    }
    if (pos >= IFX_MSI_IRQ_NUM) {
        printk(KERN_ERR "%s: Unable to find a matched MSI interrupt\n", __func__);
        return;
    }
    spin_lock(&ifx_pcie_msi_lock);
    /* Disable this entry */
    msi_irqs[pcie_port].msi_pic_p->pic_table[pos] |= IFX_MSI_PCI_INT_DISABLE;
    msi_irqs[pcie_port].msi_pic_p->pic_table[pos] &= ~(IFX_MSI_PIC_INT_LINE | IFX_MSI_PIC_MSG_ADDR | IFX_MSI_PIC_MSG_DATA);
    spin_unlock(&ifx_pcie_msi_lock);
    /*
     * Count the number of IRQs we need to free by looking at the
     * msi_multiple_irq_bitmask. Each bit set means that the next
     * IRQ is also owned by this device.
     */
    number_irqs = 0;
    while (((pos + number_irqs) < IFX_MSI_IRQ_NUM) &&
        (msi_irqs[pcie_port].msi_multiple_irq_bitmask & (1 << (pos + number_irqs)))) {
        number_irqs++;
    }
    number_irqs++;

    /* Mask with one bit for each IRQ */
    bitmask = (1 << number_irqs) - 1;

    bitmask <<= pos;
    if ((msi_irqs[pcie_port].msi_free_irq_bitmask & bitmask) != bitmask) {
        printk(KERN_ERR "%s: Attempted to teardown MSI "
             "interrupt (%d) not in use\n", __func__, irq);
        return;
    }
    /* Checks are done, update the in use bitmask */
    spin_lock(&ifx_pcie_msi_lock);
    msi_irqs[pcie_port].msi_free_irq_bitmask &= ~bitmask;
    msi_irqs[pcie_port].msi_multiple_irq_bitmask &= ~(bitmask >> 1);
    spin_unlock(&ifx_pcie_msi_lock);
    IFX_PCIE_PRINT(PCIE_MSG_MSI, "%s exit\n", __func__);
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Chuanhua.Lei@infineon.com");
MODULE_DESCRIPTION("Infineon PCIe IP builtin MSI PIC driver");

