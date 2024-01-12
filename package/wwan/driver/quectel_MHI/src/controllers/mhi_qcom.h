/* Copyright (c) 2018, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#ifndef _MHI_QCOM_
#define _MHI_QCOM_

/* iova cfg bitmask */
#define MHI_SMMU_ATTACH BIT(0)
#define MHI_SMMU_S1_BYPASS BIT(1)
#define MHI_SMMU_FAST BIT(2)
#define MHI_SMMU_ATOMIC BIT(3)
#define MHI_SMMU_FORCE_COHERENT BIT(4)

#define MHI_PCIE_VENDOR_ID (0x17cb)
#define MHI_PCIE_DEBUG_ID (0xffff)
#define MHI_RPM_SUSPEND_TMR_MS (3000)
#define MHI_PCI_BAR_NUM (0)

struct mhi_dev {
	struct pci_dev *pci_dev;
	u32 smmu_cfg;
	int resn;
	void *arch_info;
	bool powered_on;
	bool debug_mode;
};

void mhi_deinit_pci_dev(struct mhi_controller *mhi_cntrl);
int mhi_pci_probe(struct pci_dev *pci_dev,
		  const struct pci_device_id *device_id);

#if (LINUX_VERSION_CODE < KERNEL_VERSION( 3,10,65 ))
static inline int dma_set_mask_and_coherent(struct device *dev, u64 mask)
{
	int rc = dma_set_mask(dev, mask);
	if (rc == 0)
		dma_set_coherent_mask(dev, mask);
	return rc;
}
#endif

static inline int mhi_arch_iommu_init(struct mhi_controller *mhi_cntrl)
{
	struct mhi_dev *mhi_dev = mhi_controller_get_devdata(mhi_cntrl);

	mhi_cntrl->dev = &mhi_dev->pci_dev->dev;

	return dma_set_mask_and_coherent(mhi_cntrl->dev, DMA_BIT_MASK(64));
}

static inline void mhi_arch_iommu_deinit(struct mhi_controller *mhi_cntrl)
{
}

static inline int mhi_arch_pcie_init(struct mhi_controller *mhi_cntrl)
{
	return 0;
}

static inline void mhi_arch_pcie_deinit(struct mhi_controller *mhi_cntrl)
{
}

static inline int mhi_arch_platform_init(struct mhi_dev *mhi_dev)
{
	return 0;
}

static inline void mhi_arch_platform_deinit(struct mhi_dev *mhi_dev)
{
}

static inline int mhi_arch_link_off(struct mhi_controller *mhi_cntrl,
				    bool graceful)
{
	return 0;
}

static inline int mhi_arch_link_on(struct mhi_controller *mhi_cntrl)
{
	return 0;
}

#endif /* _MHI_QCOM_ */
