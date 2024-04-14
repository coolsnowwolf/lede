/**
 * SPRD ep device driver in host side for Spreadtrum SoCs
 *
 * Copyright (C) 2019 Spreadtrum Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 of
 * the License as published by the Free Software Foundation.
 *
 * This program is used to control ep device driver in host side for
 * Spreadtrum SoCs.
 */

#ifndef __SPRD_PCIE_EP_DEVICE_H
#define __SPRD_PCIE_EP_DEVICE_H

#include <linux/interrupt.h>

/* host receive msi irq */
enum {
	PCIE_MSI_SIPC_IRQ = 0,
	PCIE_MSI_REQUEST_RES,
	PCIE_MSI_EP_READY_FOR_RESCAN,
	PCIE_MSI_RELEASE_RES,
	PCIE_MSI_SCANNED_RESPOND,
	PCIE_MSI_REMOVE_RESPOND,
	PCIE_MSI_IPA,
	PCIE_MSI_MAX_IRQ
};

/* host send doorbell irq */
enum {
	PCIE_DBELL_SIPC_IRQ = 0,
	PCIE_DBEL_EP_SCANNED,
	PCIE_DBEL_EP_REMOVING,
	PCIE_DBEL_IRQ_MAX
};

enum {
	PCIE_EP_MODEM = 0,
	/* PCIE_EP_WCN, */
	PCIE_EP_NR
};

enum {
	PCIE_EP_PROBE = 0,
	PCIE_EP_REMOVE,
	PCIE_EP_PROBE_BEFORE_SPLIT_BAR
};

#ifdef CONFIG_SPRD_SIPA
enum {
	PCIE_IPA_TYPE_MEM = 0,
	PCIE_IPA_TYPE_REG
};
#endif

#define MINI_REGION_SIZE 0x10000 /*64 K default */

int sprd_ep_dev_register_notify(int ep,
				void (*notify)(int event, void *data),
				void *data);
int sprd_ep_dev_unregister_notify(int ep);
int sprd_ep_dev_register_irq_handler(int ep,
				     int irq,
				     irq_handler_t handler,
				     void *data);
int sprd_ep_dev_unregister_irq_handler(int ep, int irq);
int sprd_ep_dev_register_irq_handler_ex(int ep,
					int from_irq,
					int to_irq,
					irq_handler_t handler,
					void *data);
int sprd_ep_dev_unregister_irq_handler_ex(int ep,
					  int from_irq,
					  int to_irq);
int sprd_ep_dev_set_irq_addr(int ep, void __iomem *irq_addr);
int sprd_ep_dev_raise_irq(int ep, int irq);
int sprd_ep_dev_clear_doolbell_irq(int ep, int irq);
int sprd_ep_dev_set_backup(int ep);
int sprd_ep_dev_clear_backup(int ep);

void __iomem *sprd_ep_map_memory(int ep,
				 phys_addr_t cpu_addr,
				 size_t size);
void sprd_ep_unmap_memory(int ep, const void __iomem *bar_addr);
int sprd_ep_dev_pass_smem(int ep, u32 base, u32 size);
int sipa_module_init(struct device *dev);
void sipa_module_exit(void);
int sipa_eth_init(void);
void sipa_eth_exit(void);
int sipa_dummy_init(void);
void sipa_dummy_exit(void);

#ifdef CONFIG_SPRD_SIPA
phys_addr_t sprd_ep_ipa_map(int type, phys_addr_t target_addr, size_t size);
int sprd_ep_ipa_unmap(int type, phys_addr_t cpu_addr);
#endif
#endif
