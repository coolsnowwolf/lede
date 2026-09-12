/* SPDX-License-Identifier: GPL-2.0 */
/*
 * header file for pcie ftv310_vpu fpga.
 *
 * Copyright (C) 2024-2025, Phytium Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License version 2 for more details.
 *
 * You may obtain a copy of the GNU General Public License
 * Version 2 at the following locations:
 * https://opensource.org/licenses/gpl-2.0.php
 */

#ifndef __FTV310_VPU_FPGA_PCIE__
#define __FTV310_VPU_FPGA_PCIE__

struct ftv310_vpu_pci_t {
	struct pci_dev *dev;
	/* PCI base register address (Hardware address) */
	unsigned long pci_base_reg_hw;
	/* PCI base register address (memalloc) */
	unsigned long pci_base_ddr_hw;
	u32 pci_base_reg_len; /* Base register address Length */
	u32 pci_base_ddr_len; /* Base register address Length */
};

/* API */
int pcie_init(struct ftv310_vpu_pci_t *pci_par);
int pcie_exit(struct pci_dev *dev);

#endif /* __FTV310_VPU_FPGA_PCIE__ */
