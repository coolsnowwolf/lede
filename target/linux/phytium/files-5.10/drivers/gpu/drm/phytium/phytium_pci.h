/* SPDX-License-Identifier: GPL-2.0 */
/* Phytium display drm driver
 *
 * Copyright (c) 2021-2023 Phytium Technology Co., Ltd.
 */

#ifndef __PHYTIUM_PCI_H__
#define __PHYTIUM_PCI_H__

#include "phytium_display_drv.h"

struct phytium_pci_private {
	struct phytium_display_private base;
	void (*dc_hw_vram_init)(struct phytium_display_private *priv, resource_size_t vram_addr,
				resource_size_t vram_size);
};

struct phytium_dma_slave {
	struct device *dma_dev;
	u32 chan_id;
};

#define	to_pci_priv(priv)		container_of(priv, struct phytium_pci_private, base)

extern struct pci_driver phytium_pci_driver;
#endif /* __PHYTIUM_PCI_H__ */
