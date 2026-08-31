/****************************************************************************
                              Copyright (c) 2010
                            Lantiq Deutschland GmbH
                     Am Campeon 3; 85579 Neubiberg, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

 *****************************************************************************/
/*!
  \file ifxmips_pcie_vr9.h
  \ingroup IFX_PCIE
  \brief PCIe RC driver vr9 specific file
*/

#ifndef IFXMIPS_PCIE_VR9_H
#define IFXMIPS_PCIE_VR9_H

#include <linux/types.h>
#include <linux/delay.h>

#include <linux/gpio.h>
#include <lantiq_soc.h>

#define IFX_REG_R32    ltq_r32
#define IFX_REG_W32    ltq_w32
#define CONFIG_IFX_PCIE_HW_SWAP
#define IFX_RCU_AHB_ENDIAN                      ((volatile u32*)(IFX_RCU + 0x004C))
#define IFX_RCU_RST_REQ                         ((volatile u32*)(IFX_RCU + 0x0010))
#define IFX_RCU_AHB_BE_PCIE_PDI                  0x00000080  /* Configure PCIE PDI module in big endian*/

#define IFX_RCU                                 (KSEG1 | 0x1F203000)
#define IFX_RCU_AHB_BE_PCIE_M                    0x00000001  /* Configure AHB master port that connects to PCIe RC in big endian */
#define IFX_RCU_AHB_BE_PCIE_S                    0x00000010  /* Configure AHB slave port that connects to PCIe RC in little endian */
#define IFX_RCU_AHB_BE_XBAR_M                    0x00000002  /* Configure AHB master port that connects to XBAR in big endian */
#define IFX_RCU_AHB_BE_XBAR_S                    0x00000008  /* Configure AHB slave port that connects to XBAR in big endian */
#define CONFIG_IFX_PCIE_PHY_36MHZ_MODE

#define IFX_PMU1_MODULE_PCIE_PHY   (0)
#define IFX_PMU1_MODULE_PCIE_CTRL  (1)
#define IFX_PMU1_MODULE_PDI        (4)
#define IFX_PMU1_MODULE_MSI        (5)

#define IFX_PMU_MODULE_PCIE_L0_CLK (31)


#define IFX_GPIO				(KSEG1 | 0x1E100B00)
#define ALT0			((volatile u32*)(IFX_GPIO + 0x007c))
#define ALT1			((volatile u32*)(IFX_GPIO + 0x0080))
#define OD			((volatile u32*)(IFX_GPIO + 0x0084))
#define DIR			((volatile u32*)(IFX_GPIO + 0x0078))
#define OUT			((volatile u32*)(IFX_GPIO + 0x0070))


static inline void pcie_ahb_pmu_setup(void)
{
	/* Enable AHB bus master/slave */
	struct clk *clk;
	clk = clk_get_sys("1d900000.pcie", "ahb");
	clk_enable(clk);

    //AHBM_PMU_SETUP(IFX_PMU_ENABLE);
    //AHBS_PMU_SETUP(IFX_PMU_ENABLE);
}

static inline void pcie_phy_pmu_enable(int pcie_port)
{
	struct clk *clk;
	clk = clk_get_sys("1d900000.pcie", "phy");
	clk_enable(clk);

	//PCIE_PHY_PMU_SETUP(IFX_PMU_ENABLE);
}

static inline void pcie_phy_pmu_disable(int pcie_port)
{
	struct clk *clk;
	clk = clk_get_sys("1d900000.pcie", "phy");
	clk_disable(clk);

//    PCIE_PHY_PMU_SETUP(IFX_PMU_DISABLE);
}

static inline void pcie_pdi_pmu_enable(int pcie_port)
{
    /* Enable PDI to access PCIe PHY register */
	struct clk *clk;
	clk = clk_get_sys("1d900000.pcie", "pdi");
	clk_enable(clk);
    //PDI_PMU_SETUP(IFX_PMU_ENABLE);
}

static inline void pcie_core_pmu_setup(int pcie_port)
{
	struct clk *clk;
	clk = clk_get_sys("1d900000.pcie", "ctl");
	clk_enable(clk);
	clk = clk_get_sys("1d900000.pcie", "bus");
	clk_enable(clk);

    /* PCIe Core controller enabled */
//    PCIE_CTRL_PMU_SETUP(IFX_PMU_ENABLE);

    /* Enable PCIe L0 Clock */
//  PCIE_L0_CLK_PMU_SETUP(IFX_PMU_ENABLE);
}

static inline void pcie_msi_init(int pcie_port)
{
	struct clk *clk;
	pcie_msi_pic_init(pcie_port);
	clk = clk_get_sys("ltq_pcie", "msi");
	clk_enable(clk);
//    MSI_PMU_SETUP(IFX_PMU_ENABLE);
}

static inline u32
ifx_pcie_bus_nr_deduct(u32 bus_number, int pcie_port)
{
    u32 tbus_number = bus_number;

#ifdef CONFIG_PCI_LANTIQ
    if (pcibios_host_nr() > 1) {
        tbus_number -= pcibios_1st_host_bus_nr();
    }
#endif /* CONFIG_PCI_LANTIQ */
    return tbus_number;
}

static inline struct pci_dev *ifx_pci_get_slot(struct pci_bus *bus, unsigned int devfn)
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
    #ifdef CONFIG_PCI_LANTIQ
        if (pcibios_host_nr() > 1) {
            tvalue = ifx_pcie_bus_enum_read_hack(where, tvalue);
        }
    #endif /* CONFIG_PCI_LANTIQ */
    }
    else { /* Write hack */
    #ifdef CONFIG_PCI_LANTIQ
        if (pcibios_host_nr() > 1) {
            tvalue = ifx_pcie_bus_enum_write_hack(where, tvalue);
        }
    #endif
    }
    return tvalue;
}

#endif /* IFXMIPS_PCIE_VR9_H */
