/*
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 *  Copyright (C) 2009 Lei Chuanhua <chuanhua.lei@infineon.com>
 *  Copyright (C) 2013 John Crispin <blogic@openwrt.org>
 */

#include <linux/types.h>
#include <linux/pci.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/mm.h>
#include <asm/paccess.h>
#include <linux/pci.h>
#include <linux/pci_regs.h>
#include <linux/phy/phy.h>
#include <linux/regmap.h>
#include <linux/reset.h>
#include <linux/mfd/syscon.h>
#include <linux/module.h>

#include <linux/gpio/consumer.h>
#include <linux/platform_device.h>

#include "ifxmips_pcie.h"
#include "ifxmips_pcie_reg.h"

/* Enable 32bit io due to its mem mapped io nature */
#define IFX_PCIE_ERROR_INT
#define IFX_PCIE_IO_32BIT

#define MS(_v, _f)  (((_v) & (_f)) >> _f##_S)
#define SM(_v, _f)  (((_v) << _f##_S) & (_f))
#define IFX_REG_SET_BIT(_f, _r) \
	IFX_REG_W32((IFX_REG_R32((_r)) &~ (_f)) | (_f), (_r))

#define IFX_PCIE_LTSSM_ENABLE_TIMEOUT 10

static DEFINE_SPINLOCK(ifx_pcie_lock);

u32 g_pcie_debug_flag = PCIE_MSG_ANY & (~PCIE_MSG_CFG);
static struct gpio_desc *reset_gpio;
static struct phy *ltq_pcie_phy;
static struct reset_control *ltq_pcie_reset;
static struct regmap *ltq_rcu_regmap;
static bool switch_pcie_endianess;

static ifx_pcie_irq_t pcie_irqs[IFX_PCIE_CORE_NR] = {
    {
        .ir_irq = {
            .name = "ifx_pcie_rc0",
        },

        .legacy_irq = {
            {
                .irq_bit = PCIE_IRN_INTA,
            },
            {
                .irq_bit = PCIE_IRN_INTB,
            },
            {
                .irq_bit = PCIE_IRN_INTC,
            },
            {
                .irq_bit = PCIE_IRN_INTD,
            },
        },
    },

};

void ifx_pcie_debug(const char *fmt, ...)
{
	static char buf[256] = {0};      /* XXX */
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);

	printk("%s", buf);
}

static inline void pcie_ep_gpio_rst_init(int pcie_port)
{
	if (reset_gpio)
		gpiod_set_value_cansleep(reset_gpio, 0);
}

static inline void pcie_device_rst_assert(int pcie_port)
{
	if (reset_gpio)
		gpiod_set_value_cansleep(reset_gpio, 1);
}

static inline void pcie_device_rst_deassert(int pcie_port)
{
	mdelay(100);
	if (reset_gpio)
		gpiod_set_value_cansleep(reset_gpio, 0);
}

static inline int pcie_ltssm_enable(int pcie_port)
{
	int i;

	/* Enable LTSSM */
	IFX_REG_W32(PCIE_RC_CCR_LTSSM_ENABLE, PCIE_RC_CCR(pcie_port));

	/* Wait for the link to come up */
	for (i = 0; i < IFX_PCIE_LTSSM_ENABLE_TIMEOUT; i++) {
		if (!(IFX_REG_R32(PCIE_LCTLSTS(pcie_port)) & PCIE_LCTLSTS_RETRAIN_PENDING))
			return 0;
		udelay(10);
	}

	printk("%s link timeout!!!!!\n", __func__);
	return -1;
}

static inline void pcie_status_register_clear(int pcie_port)
{
	IFX_REG_W32(0, PCIE_RC_DR(pcie_port));
	IFX_REG_W32(0, PCIE_PCICMDSTS(pcie_port));
	IFX_REG_W32(0, PCIE_DCTLSTS(pcie_port));
	IFX_REG_W32(0, PCIE_LCTLSTS(pcie_port));
	IFX_REG_W32(0, PCIE_SLCTLSTS(pcie_port));
	IFX_REG_W32(0, PCIE_RSTS(pcie_port));
	IFX_REG_W32(0, PCIE_UES_R(pcie_port));
	IFX_REG_W32(0, PCIE_UEMR(pcie_port));
	IFX_REG_W32(0, PCIE_UESR(pcie_port));
	IFX_REG_W32(0, PCIE_CESR(pcie_port));
	IFX_REG_W32(0, PCIE_CEMR(pcie_port));
	IFX_REG_W32(0, PCIE_RESR(pcie_port));
	IFX_REG_W32(0, PCIE_PVCCRSR(pcie_port));
	IFX_REG_W32(0, PCIE_VC0_RSR0(pcie_port));
	IFX_REG_W32(0, PCIE_TPFCS(pcie_port));
	IFX_REG_W32(0, PCIE_TNPFCS(pcie_port));
	IFX_REG_W32(0, PCIE_TCFCS(pcie_port));
	IFX_REG_W32(0, PCIE_QSR(pcie_port));
	IFX_REG_W32(0, PCIE_IOBLSECS(pcie_port));
}

static inline int ifx_pcie_link_up(int pcie_port)
{
    return (IFX_REG_R32(PCIE_PHY_SR(pcie_port)) & PCIE_PHY_SR_PHY_LINK_UP) ? 1 : 0;
}


static inline void pcie_mem_io_setup(int pcie_port)
{
    u32 reg;
    /*
     * BAR[0:1] readonly register
     * RC contains only minimal BARs for packets mapped to this device
     * Mem/IO filters defines a range of memory occupied by memory mapped IO devices that
     * reside on the downstream side fo the bridge.
     */
    reg = SM((PCIE_MEM_PHY_PORT_TO_END(pcie_port) >> 20), PCIE_MBML_MEM_LIMIT_ADDR)
        | SM((PCIE_MEM_PHY_PORT_TO_BASE(pcie_port) >> 20), PCIE_MBML_MEM_BASE_ADDR);

    IFX_REG_W32(reg, PCIE_MBML(pcie_port));


#ifdef IFX_PCIE_PREFETCH_MEM_64BIT
    reg = SM((PCIE_MEM_PHY_PORT_TO_END(pcie_port) >> 20), PCIE_PMBL_END_ADDR)
        | SM((PCIE_MEM_PHY_PORT_TO_BASE(pcie_port) >> 20), PCIE_PMBL_UPPER_12BIT)
        | PCIE_PMBL_64BIT_ADDR;
    IFX_REG_W32(reg, PCIE_PMBL(pcie_port));

    /* Must configure upper 32bit */
    IFX_REG_W32(0, PCIE_PMBU32(pcie_port));
    IFX_REG_W32(0, PCIE_PMLU32(pcie_port));
#else
    /* PCIe_PBML, same as MBML */
    IFX_REG_W32(IFX_REG_R32(PCIE_MBML(pcie_port)), PCIE_PMBL(pcie_port));
#endif

    /* IO Address Range */
    reg = SM((PCIE_IO_PHY_PORT_TO_END(pcie_port) >> 12), PCIE_IOBLSECS_IO_LIMIT_ADDR)
        | SM((PCIE_IO_PHY_PORT_TO_BASE(pcie_port) >> 12), PCIE_IOBLSECS_IO_BASE_ADDR);
#ifdef IFX_PCIE_IO_32BIT
    reg |= PCIE_IOBLSECS_32BIT_IO_ADDR;
#endif /* IFX_PCIE_IO_32BIT */
    IFX_REG_W32(reg, PCIE_IOBLSECS(pcie_port));

#ifdef IFX_PCIE_IO_32BIT
    reg = SM((PCIE_IO_PHY_PORT_TO_END(pcie_port) >> 16), PCIE_IO_BANDL_UPPER_16BIT_IO_LIMIT)
        | SM((PCIE_IO_PHY_PORT_TO_BASE(pcie_port) >> 16), PCIE_IO_BANDL_UPPER_16BIT_IO_BASE);
    IFX_REG_W32(reg, PCIE_IO_BANDL(pcie_port));

#endif /* IFX_PCIE_IO_32BIT */
}

static inline void
pcie_device_setup(int pcie_port)
{
    u32 reg;

    /* Device capability register, set up Maximum payload size */
    reg = IFX_REG_R32(PCIE_DCAP(pcie_port));
    reg |= PCIE_DCAP_ROLE_BASE_ERR_REPORT;
    reg |= SM(PCIE_MAX_PAYLOAD_128, PCIE_DCAP_MAX_PAYLOAD_SIZE);

    /* Only available for EP */
    reg &= ~(PCIE_DCAP_EP_L0S_LATENCY | PCIE_DCAP_EP_L1_LATENCY);
    IFX_REG_W32(reg, PCIE_DCAP(pcie_port));

    /* Device control and status register */
    /* Set Maximum Read Request size for the device as a Requestor */
    reg = IFX_REG_R32(PCIE_DCTLSTS(pcie_port));

    /*
     * Request size can be larger than the MPS used, but the completions returned
     * for the read will be bounded by the MPS size.
     * In our system, Max request size depends on AHB burst size. It is 64 bytes.
     * but we set it as 128 as minimum one.
     */
    reg |= SM(PCIE_MAX_PAYLOAD_128, PCIE_DCTLSTS_MAX_READ_SIZE)
            | SM(PCIE_MAX_PAYLOAD_128, PCIE_DCTLSTS_MAX_PAYLOAD_SIZE);

    /* Enable relaxed ordering, no snoop, and all kinds of errors */
    reg |= PCIE_DCTLSTS_RELAXED_ORDERING_EN | PCIE_DCTLSTS_ERR_EN | PCIE_DCTLSTS_NO_SNOOP_EN;

    IFX_REG_W32(reg, PCIE_DCTLSTS(pcie_port));
}

static inline void
pcie_link_setup(int pcie_port)
{
    u32 reg;

    /*
     * XXX, Link capability register, bit 18 for EP CLKREQ# dynamic clock management for L1, L2/3 CPM
     * L0s is reported during link training via TS1 order set by N_FTS
     */
    reg = IFX_REG_R32(PCIE_LCAP(pcie_port));
    reg &= ~PCIE_LCAP_L0S_EIXT_LATENCY;
    reg |= SM(3, PCIE_LCAP_L0S_EIXT_LATENCY);
    IFX_REG_W32(reg, PCIE_LCAP(pcie_port));

    /* Link control and status register */
    reg = IFX_REG_R32(PCIE_LCTLSTS(pcie_port));

    /* Link Enable, ASPM enabled  */
    reg &= ~PCIE_LCTLSTS_LINK_DISABLE;

#ifdef CONFIG_PCIEASPM
    /*
     * We use the same physical reference clock that the platform provides on the connector
     * It paved the way for ASPM to calculate the new exit Latency
     */
    reg |= PCIE_LCTLSTS_SLOT_CLK_CFG;
    reg |= PCIE_LCTLSTS_COM_CLK_CFG;
    /*
     * We should disable ASPM by default except that we have dedicated power management support
     * Enable ASPM will cause the system hangup/instability, performance degration
     */
    reg |= PCIE_LCTLSTS_ASPM_ENABLE;
#else
    reg &= ~PCIE_LCTLSTS_ASPM_ENABLE;
#endif /* CONFIG_PCIEASPM */

    /*
     * The maximum size of any completion with data packet is bounded by the MPS setting
     * in  device control register
     */

    /* RCB may cause multiple split transactions, two options available, we use 64 byte RCB */
    reg &= ~ PCIE_LCTLSTS_RCB128;

    IFX_REG_W32(reg, PCIE_LCTLSTS(pcie_port));
}

static inline void pcie_error_setup(int pcie_port)
{
	u32 reg;

	/*
	* Forward ERR_COR, ERR_NONFATAL, ERR_FATAL to the backbone
	* Poisoned write TLPs and completions indicating poisoned TLPs will set the PCIe_PCICMDSTS.MDPE
	*/
	reg = IFX_REG_R32(PCIE_INTRBCTRL(pcie_port));
	reg |= PCIE_INTRBCTRL_SERR_ENABLE | PCIE_INTRBCTRL_PARITY_ERR_RESP_ENABLE;

	IFX_REG_W32(reg, PCIE_INTRBCTRL(pcie_port));

	/* Uncorrectable Error Mask Register, Unmask <enable> all bits in PCIE_UESR */
	reg = IFX_REG_R32(PCIE_UEMR(pcie_port));
	reg &= ~PCIE_ALL_UNCORRECTABLE_ERR;
	IFX_REG_W32(reg, PCIE_UEMR(pcie_port));

	/* Uncorrectable Error Severity Register, ALL errors are FATAL */
	IFX_REG_W32(PCIE_ALL_UNCORRECTABLE_ERR, PCIE_UESR(pcie_port));

	/* Correctable Error Mask Register, unmask <enable> all bits */
	reg = IFX_REG_R32(PCIE_CEMR(pcie_port));
	reg &= ~PCIE_CORRECTABLE_ERR;
	IFX_REG_W32(reg, PCIE_CEMR(pcie_port));

	/* Advanced Error Capabilities and Control Registr */
	reg = IFX_REG_R32(PCIE_AECCR(pcie_port));
	reg |= PCIE_AECCR_ECRC_CHECK_EN | PCIE_AECCR_ECRC_GEN_EN;
	IFX_REG_W32(reg, PCIE_AECCR(pcie_port));

	/* Root Error Command Register, Report all types of errors */
	reg = IFX_REG_R32(PCIE_RECR(pcie_port));
	reg |= PCIE_RECR_ERR_REPORT_EN;
	IFX_REG_W32(reg, PCIE_RECR(pcie_port));

	/* Clear the Root status register */
	reg = IFX_REG_R32(PCIE_RESR(pcie_port));
	IFX_REG_W32(reg, PCIE_RESR(pcie_port));
}

static inline void pcie_port_logic_setup(int pcie_port)
{
	u32 reg;

	/* FTS number, default 12, increase to 63, may increase time from/to L0s to L0  */
	reg = IFX_REG_R32(PCIE_AFR(pcie_port));
	reg &= ~(PCIE_AFR_FTS_NUM | PCIE_AFR_COM_FTS_NUM);
	reg |= SM(PCIE_AFR_FTS_NUM_DEFAULT, PCIE_AFR_FTS_NUM)
		| SM(PCIE_AFR_FTS_NUM_DEFAULT, PCIE_AFR_COM_FTS_NUM);
	/* L0s and L1 entry latency */
	reg &= ~(PCIE_AFR_L0S_ENTRY_LATENCY | PCIE_AFR_L1_ENTRY_LATENCY);
	reg |= SM(PCIE_AFR_L0S_ENTRY_LATENCY_DEFAULT, PCIE_AFR_L0S_ENTRY_LATENCY)
		| SM(PCIE_AFR_L1_ENTRY_LATENCY_DEFAULT, PCIE_AFR_L1_ENTRY_LATENCY);
	IFX_REG_W32(reg, PCIE_AFR(pcie_port));


	/* Port Link Control Register */
	reg = IFX_REG_R32(PCIE_PLCR(pcie_port));
	reg |= PCIE_PLCR_DLL_LINK_EN;  /* Enable the DLL link */
	IFX_REG_W32(reg, PCIE_PLCR(pcie_port));

	/* Lane Skew Register */
	reg = IFX_REG_R32(PCIE_LSR(pcie_port));
	/* Enable ACK/NACK and FC */
	reg &= ~(PCIE_LSR_ACKNAK_DISABLE | PCIE_LSR_FC_DISABLE);
	IFX_REG_W32(reg, PCIE_LSR(pcie_port));

	/* Symbol Timer Register and Filter Mask Register 1 */
	reg = IFX_REG_R32(PCIE_STRFMR(pcie_port));

	/* Default SKP interval is very accurate already, 5us */
	/* Enable IO/CFG transaction */
	reg |= PCIE_STRFMR_RX_CFG_TRANS_ENABLE | PCIE_STRFMR_RX_IO_TRANS_ENABLE;
	/* Disable FC WDT */
	reg &= ~PCIE_STRFMR_FC_WDT_DISABLE;
	IFX_REG_W32(reg, PCIE_STRFMR(pcie_port));

	/* Filter Masker Register 2 */
	reg = IFX_REG_R32(PCIE_FMR2(pcie_port));
	reg |= PCIE_FMR2_VENDOR_MSG1_PASSED_TO_TRGT1 | PCIE_FMR2_VENDOR_MSG0_PASSED_TO_TRGT1;
	IFX_REG_W32(reg, PCIE_FMR2(pcie_port));

	/* VC0 Completion Receive Queue Control Register */
	reg = IFX_REG_R32(PCIE_VC0_CRQCR(pcie_port));
	reg &= ~PCIE_VC0_CRQCR_CPL_TLP_QUEUE_MODE;
	reg |= SM(PCIE_VC0_TLP_QUEUE_MODE_BYPASS, PCIE_VC0_CRQCR_CPL_TLP_QUEUE_MODE);
	IFX_REG_W32(reg, PCIE_VC0_CRQCR(pcie_port));
}

static inline void pcie_rc_cfg_reg_setup(int pcie_port)
{
	u32 reg;

	/* Disable LTSSM */
	IFX_REG_W32(0, PCIE_RC_CCR(pcie_port)); /* Disable LTSSM */

	pcie_mem_io_setup(pcie_port);

	/* XXX, MSI stuff should only apply to EP */
	/* MSI Capability: Only enable 32-bit addresses */
	reg = IFX_REG_R32(PCIE_MCAPR(pcie_port));
	reg &= ~PCIE_MCAPR_ADDR64_CAP;

	reg |= PCIE_MCAPR_MSI_ENABLE;

	/* Disable multiple message */
	reg &= ~(PCIE_MCAPR_MULTI_MSG_CAP | PCIE_MCAPR_MULTI_MSG_ENABLE);
	IFX_REG_W32(reg, PCIE_MCAPR(pcie_port));


	/* Enable PME, Soft reset enabled */
	reg = IFX_REG_R32(PCIE_PM_CSR(pcie_port));
	reg |= PCIE_PM_CSR_PME_ENABLE | PCIE_PM_CSR_SW_RST;
	IFX_REG_W32(reg, PCIE_PM_CSR(pcie_port));

	/* setup the bus */
	reg = SM(0, PCIE_BNR_PRIMARY_BUS_NUM) | SM(1, PCIE_PNR_SECONDARY_BUS_NUM) | SM(0xFF, PCIE_PNR_SUB_BUS_NUM);
	IFX_REG_W32(reg, PCIE_BNR(pcie_port));


	pcie_device_setup(pcie_port);
	pcie_link_setup(pcie_port);
	pcie_error_setup(pcie_port);

	/* Root control and capabilities register */
	reg = IFX_REG_R32(PCIE_RCTLCAP(pcie_port));
	reg |= PCIE_RCTLCAP_SERR_ENABLE | PCIE_RCTLCAP_PME_INT_EN;
	IFX_REG_W32(reg, PCIE_RCTLCAP(pcie_port));

	/* Port VC Capability Register 2 */
	reg = IFX_REG_R32(PCIE_PVC2(pcie_port));
	reg &= ~PCIE_PVC2_VC_ARB_WRR;
	reg |= PCIE_PVC2_VC_ARB_16P_FIXED_WRR;
	IFX_REG_W32(reg, PCIE_PVC2(pcie_port));

	/* VC0 Resource Capability Register */
	reg = IFX_REG_R32(PCIE_VC0_RC(pcie_port));
	reg &= ~PCIE_VC0_RC_REJECT_SNOOP;
	IFX_REG_W32(reg, PCIE_VC0_RC(pcie_port));

	pcie_port_logic_setup(pcie_port);
}

static int ifx_pcie_wait_phy_link_up(int pcie_port)
{
#define IFX_PCIE_PHY_LINK_UP_TIMEOUT  1000 /* XXX, tunable */
    int i;

    /* Wait for PHY link is up */
    for (i = 0; i < IFX_PCIE_PHY_LINK_UP_TIMEOUT; i++) {
        if (ifx_pcie_link_up(pcie_port)) {
            break;
        }
        udelay(100);
    }
    if (i >= IFX_PCIE_PHY_LINK_UP_TIMEOUT) {
        printk(KERN_ERR "%s timeout\n", __func__);
        return -1;
    }

    /* Check data link up or not */
    if (!(IFX_REG_R32(PCIE_RC_DR(pcie_port)) & PCIE_RC_DR_DLL_UP)) {
        printk(KERN_ERR "%s DLL link is still down\n", __func__);
        return -1;
    }

    /* Check Data link active or not */
    if (!(IFX_REG_R32(PCIE_LCTLSTS(pcie_port)) & PCIE_LCTLSTS_DLL_ACTIVE)) {
        printk(KERN_ERR "%s DLL is not active\n", __func__);
        return -1;
    }
    return 0;
}

static inline int pcie_app_loigc_setup(int pcie_port)
{
	/* supress ahb bus errrors */
	IFX_REG_W32(PCIE_AHB_CTRL_BUS_ERROR_SUPPRESS, PCIE_AHB_CTRL(pcie_port));

	/* Pull PCIe EP out of reset */
	pcie_device_rst_deassert(pcie_port);

	/* Start LTSSM training between RC and EP */
	pcie_ltssm_enable(pcie_port);

	/* Check PHY status after enabling LTSSM */
	if (ifx_pcie_wait_phy_link_up(pcie_port) != 0)
		return -1;

	return 0;
}

/*
 * The numbers below are directly from the PCIe spec table 3-4/5.
 */
static inline void pcie_replay_time_update(int pcie_port)
{
	u32 reg;
	int nlw;
	int rtl;

	reg = IFX_REG_R32(PCIE_LCTLSTS(pcie_port));

	nlw = MS(reg, PCIE_LCTLSTS_NEGOTIATED_LINK_WIDTH);
	switch (nlw) {
	case PCIE_MAX_LENGTH_WIDTH_X1:
		rtl = 1677;
		break;
	case PCIE_MAX_LENGTH_WIDTH_X2:
		rtl = 867;
		break;
	case PCIE_MAX_LENGTH_WIDTH_X4:
		rtl = 462;
		break;
	case PCIE_MAX_LENGTH_WIDTH_X8:
		rtl = 258;
		break;
	default:
		rtl = 1677;
		break;
	}
	reg = IFX_REG_R32(PCIE_ALTRT(pcie_port));
	reg &= ~PCIE_ALTRT_REPLAY_TIME_LIMIT;
	reg |= SM(rtl, PCIE_ALTRT_REPLAY_TIME_LIMIT);
	IFX_REG_W32(reg, PCIE_ALTRT(pcie_port));
}

/*
 * Table 359 Enhanced Configuration Address Mapping1)
 * 1) This table is defined in Table 7-1, page 341, PCI Express Base Specification v1.1
 * Memory Address PCI Express Configuration Space
 * A[(20+n-1):20] Bus Number 1 < n < 8
 * A[19:15] Device Number
 * A[14:12] Function Number
 * A[11:8] Extended Register Number
 * A[7:2] Register Number
 * A[1:0] Along with size of the access, used to generate Byte Enables
 * For VR9, only the address bits [22:0] are mapped to the configuration space:
 * . Address bits [22:20] select the target bus (1-of-8)1)
 * . Address bits [19:15] select the target device (1-of-32) on the bus
 * . Address bits [14:12] select the target function (1-of-8) within the device.
 * . Address bits [11:2] selects the target dword (1-of-1024) within the selected function.s configuration space
 * . Address bits [1:0] define the start byte location within the selected dword.
 */
static inline u32 pcie_bus_addr(u8 bus_num, u16 devfn, int where)
{
	u32 addr;
	u8  bus;

	if (!bus_num) {
		/* type 0 */
		addr = ((PCI_SLOT(devfn) & 0x1F) << 15) | ((PCI_FUNC(devfn) & 0x7) << 12) | ((where & 0xFFF)& ~3);
	} else {
		bus = bus_num;
		/* type 1, only support 8 buses  */
		addr = ((bus & 0x7) << 20) | ((PCI_SLOT(devfn) & 0x1F) << 15) |
			((PCI_FUNC(devfn) & 0x7) << 12) | ((where & 0xFFF) & ~3);
	}
	return addr;
}

static int pcie_valid_config(int pcie_port, int bus, int dev)
{
	/* RC itself */
	if ((bus == 0) && (dev == 0)) {
		return 1;
	}

	/* No physical link */
	if (!ifx_pcie_link_up(pcie_port)) {
		return 0;
	}

	/* Bus zero only has RC itself
	* XXX, check if EP will be integrated
	*/
	if ((bus == 0) && (dev != 0)) {
		return 0;
	}

	/* Maximum 8 buses supported for VRX */
	if (bus > 9) {
		return 0;
	}

	/*
	 * PCIe is PtP link, one bus only supports only one device
	 * except bus zero and PCIe switch which is virtual bus device
	 * The following two conditions really depends on the system design
	 * and attached the device.
	 * XXX, how about more new switch
	*/
	if ((bus == 1) && (dev != 0)) {
		return 0;
	}

	if ((bus >= 3) && (dev != 0)) {
		return 0;
	}
	return 1;
}

static inline u32 ifx_pcie_cfg_rd(int pcie_port, u32 reg)
{
    return IFX_REG_R32((volatile u32 *)(PCIE_CFG_PORT_TO_BASE(pcie_port) + reg));
}

static inline void ifx_pcie_cfg_wr(int pcie_port, unsigned int reg, u32 val)
{
    IFX_REG_W32( val, (volatile u32 *)(PCIE_CFG_PORT_TO_BASE(pcie_port) + reg));
}

static inline u32 ifx_pcie_rc_cfg_rd(int pcie_port, u32 reg)
{
    return IFX_REG_R32((volatile u32 *)(PCIE_RC_PORT_TO_BASE(pcie_port) + reg));
}

static inline void ifx_pcie_rc_cfg_wr(int pcie_port, unsigned int reg, u32 val)
{
	IFX_REG_W32(val, (volatile u32 *)(PCIE_RC_PORT_TO_BASE(pcie_port) + reg));
}

u32 ifx_pcie_bus_enum_read_hack(int where, u32 value)
{
	u32 tvalue = value;

	if (where == PCI_PRIMARY_BUS) {
		u8 primary, secondary, subordinate;

		primary = tvalue & 0xFF;
		secondary = (tvalue >> 8) & 0xFF;
		subordinate = (tvalue >> 16) & 0xFF;
		primary += pcibios_1st_host_bus_nr();
		secondary += pcibios_1st_host_bus_nr();
		subordinate += pcibios_1st_host_bus_nr();
		tvalue = (tvalue & 0xFF000000) | (u32)primary | (u32)(secondary << 8) | (u32)(subordinate << 16);
	}
	return tvalue;
}

u32 ifx_pcie_bus_enum_write_hack(int where, u32 value)
{
    u32 tvalue = value;

    if (where == PCI_PRIMARY_BUS) {
        u8 primary, secondary, subordinate;

        primary = tvalue & 0xFF;
        secondary = (tvalue >> 8) & 0xFF;
        subordinate = (tvalue >> 16) & 0xFF;
        if (primary > 0 && primary != 0xFF) {
            primary -= pcibios_1st_host_bus_nr();
        }

        if (secondary > 0 && secondary != 0xFF) {
            secondary -= pcibios_1st_host_bus_nr();
        }
        if (subordinate > 0 && subordinate != 0xFF) {
            subordinate -= pcibios_1st_host_bus_nr();
        }
        tvalue = (tvalue & 0xFF000000) | (u32)primary | (u32)(secondary << 8) | (u32)(subordinate << 16);
    }
    else if (where == PCI_SUBORDINATE_BUS) {
        u8 subordinate = tvalue & 0xFF;

        subordinate = subordinate > 0 ? subordinate - pcibios_1st_host_bus_nr() : 0;
        tvalue = subordinate;
    }
    return tvalue;
}

static int ifx_pcie_read_config(struct pci_bus *bus, u32 devfn,
				int where, int size, u32 *value)
{
    u32 data = 0;
    int bus_number = bus->number;
    static const u32 mask[8] = {0, 0xff, 0xffff, 0, 0xffffffff, 0, 0, 0};
    int ret = PCIBIOS_SUCCESSFUL;
    struct ifx_pci_controller *ctrl = bus->sysdata;
    int pcie_port = ctrl->port;

    if (unlikely(size != 1 && size != 2 && size != 4)){
        ret = PCIBIOS_BAD_REGISTER_NUMBER;
        goto out;
    }

    /* Make sure the address is aligned to natural boundary */
    if (unlikely(((size - 1) & where))) {
        ret = PCIBIOS_BAD_REGISTER_NUMBER;
        goto out;
    }

    /*
     * If we are second controller, we have to cheat OS so that it assume
     * its bus number starts from 0 in host controller
     */
    bus_number = ifx_pcie_bus_nr_deduct(bus_number, pcie_port);

    /*
     * We need to force the bus number to be zero on the root
     * bus. Linux numbers the 2nd root bus to start after all
     * busses on root 0.
     */
    if (bus->parent == NULL) {
        bus_number = 0;
    }

    /*
     * PCIe only has a single device connected to it. It is
     * always device ID 0. Don't bother doing reads for other
     * device IDs on the first segment.
     */
    if ((bus_number == 0) && (PCI_SLOT(devfn) != 0)) {
        ret = PCIBIOS_FUNC_NOT_SUPPORTED;
        goto out;
    }

    if (pcie_valid_config(pcie_port, bus_number, PCI_SLOT(devfn)) == 0) {
        *value = 0xffffffff;
        ret = PCIBIOS_DEVICE_NOT_FOUND;
        goto out;
    }

    PCIE_IRQ_LOCK(ifx_pcie_lock);
    if (bus_number == 0) { /* RC itself */
        u32 t;

        t = (where & ~3);
        data = ifx_pcie_rc_cfg_rd(pcie_port, t);
    } else {
        u32 addr = pcie_bus_addr(bus_number, devfn, where);

        data = ifx_pcie_cfg_rd(pcie_port, addr);
    #ifdef CONFIG_IFX_PCIE_HW_SWAP
            data = le32_to_cpu(data);
    #endif /* CONFIG_IFX_PCIE_HW_SWAP */
    }
    /* To get a correct PCI topology, we have to restore the bus number to OS */
    data = ifx_pcie_bus_enum_hack(bus, devfn, where, data, pcie_port, 1);

    PCIE_IRQ_UNLOCK(ifx_pcie_lock);

    *value = (data >> (8 * (where & 3))) & mask[size & 7];
out:
    return ret;
}

static u32 ifx_pcie_size_to_value(int where, int size, u32 data, u32 value)
{
	u32 shift;
	u32 tdata = data;

	switch (size) {
	case 1:
		shift = (where & 0x3) << 3;
		tdata &= ~(0xffU << shift);
		tdata |= ((value & 0xffU) << shift);
		break;
	case 2:
		shift = (where & 3) << 3;
		tdata &= ~(0xffffU << shift);
		tdata |= ((value & 0xffffU) << shift);
		break;
	case 4:
		tdata = value;
		break;
	}
	return tdata;
}

static int ifx_pcie_write_config(struct pci_bus *bus, u32 devfn,
				int where, int size, u32 value)
{
	int bus_number = bus->number;
	int ret = PCIBIOS_SUCCESSFUL;
	struct ifx_pci_controller *ctrl = bus->sysdata;
	int pcie_port = ctrl->port;
	u32 tvalue = value;
	u32 data;

	/* Make sure the address is aligned to natural boundary */
	if (unlikely(((size - 1) & where))) {
		ret = PCIBIOS_BAD_REGISTER_NUMBER;
		goto out;
	}
	/*
	* If we are second controller, we have to cheat OS so that it assume
	* its bus number starts from 0 in host controller
	*/
	bus_number = ifx_pcie_bus_nr_deduct(bus_number, pcie_port);

	/*
	* We need to force the bus number to be zero on the root
	* bus. Linux numbers the 2nd root bus to start after all
	* busses on root 0.
	*/
	if (bus->parent == NULL) {
		bus_number = 0;
	}

	if (pcie_valid_config(pcie_port, bus_number, PCI_SLOT(devfn)) == 0) {
		ret = PCIBIOS_DEVICE_NOT_FOUND;
		goto out;
	}

	/* XXX, some PCIe device may need some delay */
	PCIE_IRQ_LOCK(ifx_pcie_lock);

	/*
	* To configure the correct bus topology using native way, we have to cheat Os so that
	* it can configure the PCIe hardware correctly.
	*/
	tvalue = ifx_pcie_bus_enum_hack(bus, devfn, where, value, pcie_port, 0);

	if (bus_number == 0) { /* RC itself */
		u32 t;

		t = (where & ~3);
		data = ifx_pcie_rc_cfg_rd(pcie_port, t);

		data = ifx_pcie_size_to_value(where, size, data, tvalue);

		ifx_pcie_rc_cfg_wr(pcie_port, t, data);
	} else {
		u32 addr = pcie_bus_addr(bus_number, devfn, where);

		data = ifx_pcie_cfg_rd(pcie_port, addr);
#ifdef CONFIG_IFX_PCIE_HW_SWAP
		data = le32_to_cpu(data);
#endif

		data = ifx_pcie_size_to_value(where, size, data, tvalue);
#ifdef CONFIG_IFX_PCIE_HW_SWAP
		data = cpu_to_le32(data);
#endif
		ifx_pcie_cfg_wr(pcie_port, addr, data);
	}
	PCIE_IRQ_UNLOCK(ifx_pcie_lock);
out:
	return ret;
}

static struct resource ifx_pcie_io_resource = {
	.name	= "PCIe0 I/O space",
	.start	= PCIE_IO_PHY_BASE,
	.end	= PCIE_IO_PHY_END,
	.flags	= IORESOURCE_IO,
};

static struct resource ifx_pcie_mem_resource = {
	.name	= "PCIe0 Memory space",
	.start	= PCIE_MEM_PHY_BASE,
	.end	= PCIE_MEM_PHY_END,
	.flags	= IORESOURCE_MEM,
};

static struct pci_ops ifx_pcie_ops = {
	.read	= ifx_pcie_read_config,
	.write	= ifx_pcie_write_config,
};

static struct ifx_pci_controller ifx_pcie_controller[IFX_PCIE_CORE_NR] = {
    {
        .pcic = {
            .pci_ops      = &ifx_pcie_ops,
            .mem_resource = &ifx_pcie_mem_resource,
            .io_resource  = &ifx_pcie_io_resource,
         },
         .port = IFX_PCIE_PORT0,
    },
};

#ifdef IFX_PCIE_ERROR_INT

static irqreturn_t pcie_rc_core_isr(int irq, void *dev_id)
{
	struct ifx_pci_controller *ctrl = (struct ifx_pci_controller *)dev_id;
	int pcie_port = ctrl->port;
	u32 reg;

	pr_debug("PCIe RC error intr %d\n", irq);
	reg = IFX_REG_R32(PCIE_IRNCR(pcie_port));
	reg &= PCIE_RC_CORE_COMBINED_INT;
	IFX_REG_W32(reg, PCIE_IRNCR(pcie_port));

	return IRQ_HANDLED;
}

static int
pcie_rc_core_int_init(int pcie_port)
{
	int ret;

	/* Enable core interrupt */
	IFX_REG_SET_BIT(PCIE_RC_CORE_COMBINED_INT, PCIE_IRNEN(pcie_port));

	/* Clear it first */
	IFX_REG_SET_BIT(PCIE_RC_CORE_COMBINED_INT, PCIE_IRNCR(pcie_port));
	ret = request_irq(pcie_irqs[pcie_port].ir_irq.irq, pcie_rc_core_isr, 0,
		pcie_irqs[pcie_port].ir_irq.name, &ifx_pcie_controller[pcie_port]);
	if (ret)
		printk(KERN_ERR "%s request irq %d failed\n", __func__,
		       pcie_irqs[pcie_port].ir_irq.irq);

	return ret;
}
#endif

int ifx_pcie_bios_map_irq(IFX_PCI_CONST struct pci_dev *dev, u8 slot, u8 pin)
{
	u32 irq_bit = 0;
	int irq = 0;
	struct ifx_pci_controller *ctrl = dev->bus->sysdata;
	int pcie_port = ctrl->port;

	printk("%s port %d dev %s slot %d pin %d \n", __func__, pcie_port, pci_name(dev), slot, pin);

	if ((pin == PCIE_LEGACY_DISABLE) || (pin > PCIE_LEGACY_INT_MAX)) {
		printk(KERN_WARNING "WARNING: dev %s: invalid interrupt pin %d\n", pci_name(dev), pin);
		return -1;
	}

	/* Pin index so minus one */
	irq_bit = pcie_irqs[pcie_port].legacy_irq[pin - 1].irq_bit;
	irq = pcie_irqs[pcie_port].legacy_irq[pin - 1].irq;
	IFX_REG_SET_BIT(irq_bit, PCIE_IRNEN(pcie_port));
	IFX_REG_SET_BIT(irq_bit, PCIE_IRNCR(pcie_port));
	printk("%s dev %s irq %d assigned\n", __func__, pci_name(dev), irq);
	return irq;
}

int  ifx_pcie_bios_plat_dev_init(struct pci_dev *dev)
{
    u16 config;
#ifdef IFX_PCIE_ERROR_INT
    u32 dconfig;
    int pos;
#endif

    /* Enable reporting System errors and parity errors on all devices */
    /* Enable parity checking and error reporting */
    pci_read_config_word(dev, PCI_COMMAND, &config);
    config |= PCI_COMMAND_PARITY | PCI_COMMAND_SERR /*| PCI_COMMAND_INVALIDATE |
          PCI_COMMAND_FAST_BACK*/;
    pci_write_config_word(dev, PCI_COMMAND, config);

    if (dev->subordinate) {
        /* Set latency timers on sub bridges */
        pci_write_config_byte(dev, PCI_SEC_LATENCY_TIMER, 0x40); /* XXX, */
        /* More bridge error detection */
        pci_read_config_word(dev, PCI_BRIDGE_CONTROL, &config);
        config |= PCI_BRIDGE_CTL_PARITY | PCI_BRIDGE_CTL_SERR;
        pci_write_config_word(dev, PCI_BRIDGE_CONTROL, config);
    }
#ifdef IFX_PCIE_ERROR_INT
    /* Enable the PCIe normal error reporting */
    pos = pci_find_capability(dev, PCI_CAP_ID_EXP);
    if (pos) {

        /* Disable system error generation in response to error messages */
        pci_read_config_word(dev, pos + PCI_EXP_RTCTL, &config);
        config &= ~(PCI_EXP_RTCTL_SECEE | PCI_EXP_RTCTL_SENFEE | PCI_EXP_RTCTL_SEFEE);
        pci_write_config_word(dev, pos + PCI_EXP_RTCTL, config);

        /* Clear PCIE Capability's Device Status */
        pci_read_config_word(dev, pos + PCI_EXP_DEVSTA, &config);
        pci_write_config_word(dev, pos + PCI_EXP_DEVSTA, config);

        /* Update Device Control */
        pci_read_config_word(dev, pos + PCI_EXP_DEVCTL, &config);
        /* Correctable Error Reporting */
        config |= PCI_EXP_DEVCTL_CERE;
        /* Non-Fatal Error Reporting */
        config |= PCI_EXP_DEVCTL_NFERE;
        /* Fatal Error Reporting */
        config |= PCI_EXP_DEVCTL_FERE;
        /* Unsupported Request */
        config |= PCI_EXP_DEVCTL_URRE;
        pci_write_config_word(dev, pos + PCI_EXP_DEVCTL, config);
    }

    /* Find the Advanced Error Reporting capability */
    pos = pci_find_ext_capability(dev, PCI_EXT_CAP_ID_ERR);
    if (pos) {
        /* Clear Uncorrectable Error Status */
        pci_read_config_dword(dev, pos + PCI_ERR_UNCOR_STATUS, &dconfig);
        pci_write_config_dword(dev, pos + PCI_ERR_UNCOR_STATUS, dconfig);
        /* Enable reporting of all uncorrectable errors */
        /* Uncorrectable Error Mask - turned on bits disable errors */
        pci_write_config_dword(dev, pos + PCI_ERR_UNCOR_MASK, 0);
        /*
        * Leave severity at HW default. This only controls if
        * errors are reported as uncorrectable or
        * correctable, not if the error is reported.
        */
        /* PCI_ERR_UNCOR_SEVER - Uncorrectable Error Severity */
        /* Clear Correctable Error Status */
        pci_read_config_dword(dev, pos + PCI_ERR_COR_STATUS, &dconfig);
        pci_write_config_dword(dev, pos + PCI_ERR_COR_STATUS, dconfig);
        /* Enable reporting of all correctable errors */
        /* Correctable Error Mask - turned on bits disable errors */
        pci_write_config_dword(dev, pos + PCI_ERR_COR_MASK, 0);
        /* Advanced Error Capabilities */
        pci_read_config_dword(dev, pos + PCI_ERR_CAP, &dconfig);
        /* ECRC Generation Enable */
        if (dconfig & PCI_ERR_CAP_ECRC_GENC) {
            dconfig |= PCI_ERR_CAP_ECRC_GENE;
        }
        /* ECRC Check Enable */
        if (dconfig & PCI_ERR_CAP_ECRC_CHKC) {
            dconfig |= PCI_ERR_CAP_ECRC_CHKE;
        }
        pci_write_config_dword(dev, pos + PCI_ERR_CAP, dconfig);

        /* PCI_ERR_HEADER_LOG - Header Log Register (16 bytes) */
        /* Enable Root Port's interrupt in response to error messages */
        pci_write_config_dword(dev, pos + PCI_ERR_ROOT_COMMAND,
              PCI_ERR_ROOT_CMD_COR_EN |
              PCI_ERR_ROOT_CMD_NONFATAL_EN |
              PCI_ERR_ROOT_CMD_FATAL_EN);
        /* Clear the Root status register */
        pci_read_config_dword(dev, pos + PCI_ERR_ROOT_STATUS, &dconfig);
        pci_write_config_dword(dev, pos + PCI_ERR_ROOT_STATUS, dconfig);
    }
#endif /* IFX_PCIE_ERROR_INT */
    /* WAR, only 128 MRRS is supported, force all EPs to support this value */
    pcie_set_readrq(dev, 128);
    return 0;
}

static int
pcie_rc_initialize(int pcie_port)
{
	int i, ret;
#define IFX_PCIE_PHY_LOOP_CNT  5

	regmap_update_bits(ltq_rcu_regmap, 0x4c, IFX_RCU_AHB_BE_PCIE_M,
			   IFX_RCU_AHB_BE_PCIE_M);

#ifdef CONFIG_IFX_PCIE_HW_SWAP
	regmap_update_bits(ltq_rcu_regmap, 0x4c, IFX_RCU_AHB_BE_PCIE_S,
			   IFX_RCU_AHB_BE_PCIE_S);
       if (switch_pcie_endianess) {
	        regmap_update_bits(ltq_rcu_regmap, 0x4c, IFX_RCU_AHB_BE_XBAR_S,
			           IFX_RCU_AHB_BE_XBAR_S);
       }
#else
	regmap_update_bits(ltq_rcu_regmap, 0x4c, IFX_RCU_AHB_BE_PCIE_S,
			   0x0);
#endif

	regmap_update_bits(ltq_rcu_regmap, 0x4c, IFX_RCU_AHB_BE_XBAR_M,
			   0x0);

	pcie_ep_gpio_rst_init(pcie_port);

	/*
	* XXX, PCIe elastic buffer bug will cause not to be detected. One more
	* reset PCIe PHY will solve this issue
	*/
	for (i = 0; i < IFX_PCIE_PHY_LOOP_CNT; i++) {
		ret = phy_init(ltq_pcie_phy);
		if (ret)
			continue;

		/* Put PCIe EP in reset status */
		pcie_device_rst_assert(pcie_port);

		udelay(1);
		reset_control_deassert(ltq_pcie_reset);

		ret = phy_power_on(ltq_pcie_phy);
		if (ret) {
			phy_exit(ltq_pcie_phy);
			continue;
		}

		/* Enable PCIe PHY and Clock */
		pcie_core_pmu_setup(pcie_port);

		/* Clear status registers */
		pcie_status_register_clear(pcie_port);

#ifdef CONFIG_PCI_MSI
		pcie_msi_init(pcie_port);
#endif /* CONFIG_PCI_MSI */
		pcie_rc_cfg_reg_setup(pcie_port);

		/* Once link is up, break out */
		if (pcie_app_loigc_setup(pcie_port) == 0)
			break;

		phy_power_off(ltq_pcie_phy);
		reset_control_assert(ltq_pcie_reset);
		phy_exit(ltq_pcie_phy);
	}
	if (i >= IFX_PCIE_PHY_LOOP_CNT) {
		printk(KERN_ERR "%s link up failed!!!!!\n", __func__);
		return -EIO;
	}
	/* NB, don't increase ACK/NACK timer timeout value, which will cause a lot of COR errors */
	pcie_replay_time_update(pcie_port);
	return 0;
}

static int ifx_pcie_bios_probe(struct platform_device *pdev)
{
    struct device_node *node = pdev->dev.of_node;
    void __iomem *io_map_base;
    int pcie_port;
    int startup_port;
    struct device_node *np;
    struct pci_bus *bus;

    /*
     * In case a PCI device is physical present, the Lantiq PCI driver need
     * to be loaded prior to the Lantiq PCIe driver. Otherwise none of them
     * will work.
     *
     * In case the lantiq PCI driver is enabled in the device tree, check if
     * a PCI bus (hopefully the one of the Lantiq PCI driver one) is already
     * registered.
     *
     * It will fail if there is another PCI controller, this controller is
     * registered before the Lantiq PCIe driver is probe and the lantiq PCI
     */
    np = of_find_compatible_node(NULL, NULL, "lantiq,pci-xway");

    if (of_device_is_available(np)) {
        bus = pci_find_next_bus(bus);

        if (!bus)
	     return -EPROBE_DEFER;
    }

    /* Enable AHB Master/ Slave */
    pcie_ahb_pmu_setup();

    startup_port = IFX_PCIE_PORT0;

    ltq_pcie_phy = devm_phy_get(&pdev->dev, "pcie");
    if (IS_ERR(ltq_pcie_phy))
        return dev_err_probe(&pdev->dev, PTR_ERR(ltq_pcie_phy),
                             "failed to get the PCIe PHY\n");

    ltq_pcie_reset = devm_reset_control_get_shared(&pdev->dev, NULL);
    if (IS_ERR(ltq_pcie_reset)) {
        dev_err(&pdev->dev, "failed to get the PCIe reset line\n");
        return PTR_ERR(ltq_pcie_reset);
    }

    if (of_property_read_bool(node, "lantiq,switch-pcie-endianess")) {
        switch_pcie_endianess = true;
        dev_info(&pdev->dev, "switch pcie endianess requested\n");
    } else {
        switch_pcie_endianess = false;
    }

    ltq_rcu_regmap = syscon_regmap_lookup_by_phandle(node, "lantiq,rcu");
    if (IS_ERR(ltq_rcu_regmap))
        return PTR_ERR(ltq_rcu_regmap);

    reset_gpio = devm_gpiod_get_optional(&pdev->dev, "reset",
                                         GPIOD_OUT_HIGH);
    if (IS_ERR(reset_gpio))
        return dev_err_probe(&pdev->dev, PTR_ERR(reset_gpio),
                             "failed to request reset gpio\n");

    if (reset_gpio)
        gpiod_set_consumer_name(reset_gpio, "pcie-reset");

    for (pcie_port = startup_port; pcie_port < IFX_PCIE_CORE_NR; pcie_port++){
	if (pcie_rc_initialize(pcie_port) == 0) {
	    IFX_PCIE_PRINT(PCIE_MSG_INIT, "%s: ifx_pcie_cfg_base 0x%p\n",
                 __func__, PCIE_CFG_PORT_TO_BASE(pcie_port));
            /* Otherwise, warning will pop up */
            io_map_base = ioremap(PCIE_IO_PHY_PORT_TO_BASE(pcie_port), PCIE_IO_SIZE);
            if (io_map_base == NULL) {
                IFX_PCIE_PRINT(PCIE_MSG_ERR, "%s io space ioremap failed\n", __func__);
                return -ENOMEM;
            }
            pcie_irqs[pcie_port].ir_irq.irq = platform_get_irq(pdev, 0);
            if (pcie_irqs[pcie_port].ir_irq.irq < 0)
                return pcie_irqs[pcie_port].ir_irq.irq;

            for (int i = 0; i <= 3; i++){
                pcie_irqs[pcie_port].legacy_irq[i].irq = platform_get_irq(pdev, i + 1);

                if (pcie_irqs[pcie_port].legacy_irq[i].irq < 0)
                    return pcie_irqs[pcie_port].legacy_irq[i].irq;
            }

            ifx_pcie_controller[pcie_port].pcic.io_map_base = (unsigned long)io_map_base;
            pci_load_of_ranges(&ifx_pcie_controller[pcie_port].pcic, node);

            register_pci_controller(&ifx_pcie_controller[pcie_port].pcic);
            /* XXX, clear error status */

            IFX_PCIE_PRINT(PCIE_MSG_INIT, "%s: mem_resource 0x%p, io_resource 0x%p\n",
                              __func__, &ifx_pcie_controller[pcie_port].pcic.mem_resource,
                              &ifx_pcie_controller[pcie_port].pcic.io_resource);

        #ifdef IFX_PCIE_ERROR_INT
            pcie_rc_core_int_init(pcie_port);
        #endif /* IFX_PCIE_ERROR_INT */
        }
    }

    return 0;
}

static const struct of_device_id ifxmips_pcie_match[] = {
        { .compatible = "lantiq,pcie-xrx200" },
        {},
};
MODULE_DEVICE_TABLE(of, ifxmips_pcie_match);

static struct platform_driver ltq_pci_driver = {
        .probe = ifx_pcie_bios_probe,
        .driver = {
                .name = "pcie-xrx200",
                .of_match_table = ifxmips_pcie_match,
        },
};

static int __init ifx_pcie_bios_init(void)
{
        int ret = platform_driver_register(&ltq_pci_driver);
        if (ret)
                pr_info("pcie-xrx200: Error registering platform driver!");
        return ret;
}

arch_initcall(ifx_pcie_bios_init);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Chuanhua.Lei@infineon.com");
MODULE_DESCRIPTION("Infineon builtin PCIe RC driver");

