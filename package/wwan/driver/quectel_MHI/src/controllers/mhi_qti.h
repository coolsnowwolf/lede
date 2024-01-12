/* SPDX-License-Identifier: GPL-2.0-only */
/* Copyright (c) 2018-2019, The Linux Foundation. All rights reserved.*/

#ifndef _MHI_QTI_
#define _MHI_QTI_

/* iova cfg bitmask */
#define MHI_SMMU_ATTACH BIT(0)
#define MHI_SMMU_S1_BYPASS BIT(1)
#define MHI_SMMU_FAST BIT(2)
#define MHI_SMMU_ATOMIC BIT(3)
#define MHI_SMMU_FORCE_COHERENT BIT(4)

#define MHI_PCIE_VENDOR_ID (0x17cb)
#define MHI_PCIE_DEBUG_ID (0xffff)

/* runtime suspend timer */
#define MHI_RPM_SUSPEND_TMR_MS (2000)
#define MHI_PCI_BAR_NUM (0)

struct mhi_dev {
	struct pci_dev *pci_dev;
	u32 smmu_cfg;
	int resn;
	void *arch_info;
	bool powered_on;
	dma_addr_t iova_start;
	dma_addr_t iova_stop;
	bool lpm_disabled;
};

void mhi_deinit_pci_dev(struct mhi_controller *mhi_cntrl);
int mhi_pci_probe(struct pci_dev *pci_dev,
		  const struct pci_device_id *device_id);

void mhi_pci_device_removed(struct pci_dev *pci_dev);
int mhi_arch_pcie_init(struct mhi_controller *mhi_cntrl);
void mhi_arch_pcie_deinit(struct mhi_controller *mhi_cntrl);
int mhi_arch_iommu_init(struct mhi_controller *mhi_cntrl);
void mhi_arch_iommu_deinit(struct mhi_controller *mhi_cntrl);
int mhi_arch_link_off(struct mhi_controller *mhi_cntrl, bool graceful);
int mhi_arch_link_on(struct mhi_controller *mhi_cntrl);

#endif /* _MHI_QTI_ */
