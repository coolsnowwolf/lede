/****************************************************************************
                              Copyright (c) 2010
                            Lantiq Deutschland GmbH
                     Am Campeon 3; 85579 Neubiberg, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

 *****************************************************************************/
/*!
  \file ifxmips_pcie_ar10.h
  \ingroup IFX_PCIE
  \brief PCIe RC driver ar10 specific file
*/

#ifndef IFXMIPS_PCIE_AR10_H
#define IFXMIPS_PCIE_AR10_H
#ifndef AUTOCONF_INCLUDED
#include <linux/config.h>
#endif /* AUTOCONF_INCLUDED */
#include <linux/types.h>
#include <linux/delay.h>

/* Project header file */
#include <asm/ifx/ifx_types.h>
#include <asm/ifx/ifx_pmu.h>
#include <asm/ifx/ifx_gpio.h>
#include <asm/ifx/ifx_ebu_led.h>

static inline void pcie_ep_gpio_rst_init(int pcie_port)
{
    ifx_ebu_led_enable();
    if (pcie_port == 0) {
        ifx_ebu_led_set_data(11, 1);
    }
    else {
        ifx_ebu_led_set_data(12, 1);
    }
}

static inline void pcie_ahb_pmu_setup(void)
{
    /* XXX, moved to CGU to control AHBM */
}

static inline void pcie_rcu_endian_setup(int pcie_port)
{
    u32 reg;

    reg = IFX_REG_R32(IFX_RCU_AHB_ENDIAN);
    /* Inbound, big endian */
    reg |= IFX_RCU_BE_AHB4S;
    if (pcie_port == 0) {
        reg |= IFX_RCU_BE_PCIE0M;

    #ifdef CONFIG_IFX_PCIE_HW_SWAP
        /* Outbound, software swap needed */
        reg |= IFX_RCU_BE_AHB3M;
        reg &= ~IFX_RCU_BE_PCIE0S;
    #else
        /* Outbound little endian  */
        reg &= ~IFX_RCU_BE_AHB3M;
        reg &= ~IFX_RCU_BE_PCIE0S;
    #endif
    }
    else {
        reg |= IFX_RCU_BE_PCIE1M;
    #ifdef CONFIG_IFX_PCIE1_HW_SWAP
        /* Outbound, software swap needed */
        reg |= IFX_RCU_BE_AHB3M;
        reg &= ~IFX_RCU_BE_PCIE1S;
    #else
        /* Outbound little endian  */
        reg &= ~IFX_RCU_BE_AHB3M;
        reg &= ~IFX_RCU_BE_PCIE1S;
    #endif
    }

    IFX_REG_W32(reg, IFX_RCU_AHB_ENDIAN);
    IFX_PCIE_PRINT(PCIE_MSG_REG, "%s IFX_RCU_AHB_ENDIAN: 0x%08x\n", __func__, IFX_REG_R32(IFX_RCU_AHB_ENDIAN));
}

static inline void pcie_phy_pmu_enable(int pcie_port)
{
    if (pcie_port == 0) { /* XXX, should use macro*/
        PCIE0_PHY_PMU_SETUP(IFX_PMU_ENABLE);
    }
    else {
        PCIE1_PHY_PMU_SETUP(IFX_PMU_ENABLE);
    }
}

static inline void pcie_phy_pmu_disable(int pcie_port)
{
    if (pcie_port == 0) { /* XXX, should use macro*/
        PCIE0_PHY_PMU_SETUP(IFX_PMU_DISABLE);
    }
    else {
        PCIE1_PHY_PMU_SETUP(IFX_PMU_DISABLE);
    }
}

static inline void pcie_pdi_big_endian(int pcie_port)
{
    u32 reg;

    reg = IFX_REG_R32(IFX_RCU_AHB_ENDIAN);
    if (pcie_port == 0) {
        /* Config AHB->PCIe and PDI endianness */
        reg |= IFX_RCU_BE_PCIE0_PDI;
    }
    else {
        /* Config AHB->PCIe and PDI endianness */
        reg |= IFX_RCU_BE_PCIE1_PDI;
    }
    IFX_REG_W32(reg, IFX_RCU_AHB_ENDIAN);
}

static inline void pcie_pdi_pmu_enable(int pcie_port)
{
    if (pcie_port == 0) {
        /* Enable PDI to access PCIe PHY register */
        PDI0_PMU_SETUP(IFX_PMU_ENABLE);
    }
    else {
        PDI1_PMU_SETUP(IFX_PMU_ENABLE);
    }
}

static inline void pcie_core_rst_assert(int pcie_port)
{
    u32 reg;

    reg = IFX_REG_R32(IFX_RCU_RST_REQ);

    /* Reset Core, bit 22 */
    if (pcie_port == 0) {
        reg |= 0x00400000;
    }
    else {
        reg |= 0x08000000; /* Bit 27 */
    }
    IFX_REG_W32(reg, IFX_RCU_RST_REQ);
}

static inline void pcie_core_rst_deassert(int pcie_port)
{
    u32 reg;

    /* Make sure one micro-second delay */
    udelay(1);

    reg = IFX_REG_R32(IFX_RCU_RST_REQ);
    if (pcie_port == 0) {
        reg &= ~0x00400000; /* bit 22 */
    }
    else {
        reg &= ~0x08000000; /* Bit 27 */
    }
    IFX_REG_W32(reg, IFX_RCU_RST_REQ);
}

static inline void pcie_phy_rst_assert(int pcie_port)
{
    u32 reg;

    reg = IFX_REG_R32(IFX_RCU_RST_REQ);
    if (pcie_port == 0) {
        reg |= 0x00001000; /* Bit 12 */
    }
    else {
        reg |= 0x00002000; /* Bit 13 */
    }
    IFX_REG_W32(reg, IFX_RCU_RST_REQ);
}

static inline void pcie_phy_rst_deassert(int pcie_port)
{
    u32 reg;

    /* Make sure one micro-second delay */
    udelay(1);

    reg = IFX_REG_R32(IFX_RCU_RST_REQ);
    if (pcie_port == 0) {
        reg &= ~0x00001000; /* Bit 12 */
    }
    else {
        reg &= ~0x00002000; /* Bit 13 */
    }
    IFX_REG_W32(reg, IFX_RCU_RST_REQ);
}

static inline void pcie_device_rst_assert(int pcie_port)
{
    if (pcie_port == 0) {
        ifx_ebu_led_set_data(11, 0);
    }
    else {
        ifx_ebu_led_set_data(12, 0);
    }
}

static inline void pcie_device_rst_deassert(int pcie_port)
{
    mdelay(100);
    if (pcie_port == 0) {
        ifx_ebu_led_set_data(11, 1);
    }
    else {
        ifx_ebu_led_set_data(12, 1);
    }
    ifx_ebu_led_disable();
}

static inline void pcie_core_pmu_setup(int pcie_port)
{
    if (pcie_port == 0) {
        PCIE0_CTRL_PMU_SETUP(IFX_PMU_ENABLE);
    }
    else {
        PCIE1_CTRL_PMU_SETUP(IFX_PMU_ENABLE);
    }
}

static inline void pcie_msi_init(int pcie_port)
{
    pcie_msi_pic_init(pcie_port);
    if (pcie_port == 0) {
        MSI0_PMU_SETUP(IFX_PMU_ENABLE);
    }
    else {
        MSI1_PMU_SETUP(IFX_PMU_ENABLE);
    }
}

static inline u32
ifx_pcie_bus_nr_deduct(u32 bus_number, int pcie_port)
{
    u32 tbus_number = bus_number;

#ifdef CONFIG_IFX_PCIE_2ND_CORE
    if (pcie_port == IFX_PCIE_PORT1) { /* Port 1 must check if there are two cores enabled */
        if (pcibios_host_nr() > 1) {
            tbus_number -= pcibios_1st_host_bus_nr();
        }
    }
#endif /* CONFIG_IFX_PCI */
    return tbus_number;
}

static struct pci_dev *ifx_pci_get_slot(struct pci_bus *bus, unsigned int devfn)
{
	struct pci_dev *dev;

	list_for_each_entry(dev, &bus->devices, bus_list) {
		if (dev->devfn == devfn)
			goto out;
	}

	dev = NULL;
 out:
	pci_dev_get(dev);
	return dev;
}

static inline u32
ifx_pcie_bus_enum_hack(struct pci_bus *bus, u32 devfn, int where, u32 value, int pcie_port, int read)
{
    struct pci_dev *pdev;
    u32 tvalue = value;

    /* Sanity check */
    pdev = ifx_pci_get_slot(bus, devfn);
    if (pdev == NULL) {
        return tvalue;
    }

    /* Only care about PCI bridge */
    if (pdev->hdr_type != PCI_HEADER_TYPE_BRIDGE) {
        return tvalue;
    }

    if (read) { /* Read hack */
    #ifdef CONFIG_IFX_PCIE_2ND_CORE
        if (pcie_port == IFX_PCIE_PORT1) { /* Port 1 must check if there are two cores enabled */
            if (pcibios_host_nr() > 1) {
                tvalue = ifx_pcie_bus_enum_read_hack(where, tvalue);
            }
        }
    #endif /* CONFIG_IFX_PCIE_2ND_CORE */
    }
    else { /* Write hack */
    #ifdef CONFIG_IFX_PCIE_2ND_CORE
        if (pcie_port == IFX_PCIE_PORT1) { /* Port 1 must check if there are two cores enabled */
            if (pcibios_host_nr() > 1) {
                tvalue = ifx_pcie_bus_enum_write_hack(where, tvalue);
            }
        }
    #endif
    }
    return tvalue;
}

#endif /* IFXMIPS_PCIE_AR10_H */
