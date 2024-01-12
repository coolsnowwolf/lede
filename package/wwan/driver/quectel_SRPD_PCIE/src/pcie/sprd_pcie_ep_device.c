/**
 * SPRD ep device driver in host side for Spreadtrum SoCs
 *
 * Copyright (C) 2018 Spreadtrum Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 of
 * the License as published by the Free Software Foundation.
 *
 * This program is used to control ep device driver in host side for
 * Spreadtrum SoCs.
 */

#include <linux/delay.h>
#include <linux/debugfs.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/bitops.h>
#include <linux/pci.h>
#include <linux/pci_regs.h>
#include <linux/pci_ids.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/workqueue.h>
#include <linux/version.h>

#include <linux/iommu.h>
#include <linux/iova.h>
#include <linux/bitops.h>

#include "../include/pcie-rc-sprd.h"
#include "../include/sprd_pcie_ep_device.h"
#include "../include/sipc_big_to_little.h"
#include "../include/sprd_pcie_resource.h"
#include "../include/sipc.h"
#include "../include/sipa.h"
#include "../sipa/sipa_core.h"

#define DRV_MODULE_NAME		"sprd-pcie-ep-device"
#define CONFIG_SPRD_IPA_PCIE_WORKROUND

enum dev_pci_barno {
	BAR_0 = 0,
	BAR_1,
	BAR_2,
	BAR_3,
	BAR_4,
	BAR_5,
	BAR_CNT
};

#define MAX_SUPPORT_IRQ	32
#define IPA_HW_IRQ_CNT 4
#define IPA_HW_IRQ_BASE 16
#define IPA_HW_IRQ_BASE_DEFECT 0

#define REQUEST_BASE_IRQ (IPA_HW_IRQ_BASE + IPA_HW_IRQ_CNT)
#define REQUEST_BASE_IRQ_DEFECT 16

#ifdef CONFIG_SPRD_IPA_PCIE_WORKROUND
/* the bar0 and the bar1 are used for ipa */
#define IPA_MEM_BAR	BAR_0
#define IPA_REG_BAR	BAR_1
#define BAR_MIN		BAR_2
#else
#define BAR_MIN		BAR_0
#endif

/* the bar4 and the bar5 are specail bars */
#define BAR_MAX BAR_4

#define PCI_VENDOR_ID_SPRD	0x16c3
#define PCI_DEVICE_ID_SPRD_ORCA	0xabcd
#define PCI_CLASS_ID_SPRD_ORCA	0x80d00

/* Parameters for the waiting for iATU enabled routine */
#define LINK_WAIT_MAX_IATU_RETRIES	5
#define LINK_WAIT_IATU_MIN		9000
#define LINK_WAIT_IATU_MAX		10000

/* ep config bar bar4 , can config ep iatu reg and door bell reg */
#define EP_CFG_BAR	BAR_4
#define DOOR_BELL_BASE	0x00000
#define IATU_REG_BASE	0x10000

#define DOOR_BELL_ENABLE	0x10
#define DOOR_BELL_STATUS	0x14

/* used 0x18 & 0x1c to save the smem base & size. */
#define DOOR_BELL_SMEMBASE	0x18
#define DOOR_BELL_SMEMSIZE	0x1C

/* one bit can indicate one irq , if stauts[i] & enable[i] , irq = i */
#define DOOR_BELL_IRQ_VALUE(irq)	BIT((irq))
#define DOOR_BELL_IRQ_CNT		32
#define IATU_MAX_REGION			8
#define PCIE_ATU_VIEWPORT		0x900
#define PCIE_ATU_CR1			0x904
#define PCIE_ATU_CR2			0x908
#define PCIE_ATU_LOWER_BASE		0x90c
#define PCIE_ATU_UPPER_BASE		0x910
#define PCIE_ATU_LIMIT			0x914
#define PCIE_ATU_LOWER_TARGET		0x918
#define PCIE_ATU_UPPER_TARGET		0x91c

#define PCIE_ATU_REGION_INBOUND		BIT(31)
#define PCIE_ATU_ENABLE			BIT(31)
#define PCIE_ATU_BAR_MODE_ENABLE	BIT(30)
#define PCIE_ATU_TYPE_MEM		0x0

#define PCIE_ATU_UNR_REGION_CTRL1	0x00
#define PCIE_ATU_UNR_REGION_CTRL2	0x04
#define PCIE_ATU_UNR_LOWER_BASE		0x08
#define PCIE_ATU_UNR_UPPER_BASE		0x0c
#define PCIE_ATU_UNR_LIMIT		0x10
#define PCIE_ATU_UNR_LOWER_TARGET	0x14
#define PCIE_ATU_UNR_UPPER_TARGET	0x18

/* bar4 + 0x10000 has map to ep base + 0x18000 ((0x3 << 15)) */
#define PCIE_ATU_IB_REGION(region) (((region) << 9) | (0x1 << 8))
#define PCIE_ATU_OB_REGION(region) ((region) << 9)

#define PCIE_SAVE_REGION_NUM	(IATU_MAX_REGION * 2)
#define PCIE_SAVE_REG_NUM	8

#define PCIE_LEGACY_CLEAR_BASE	0x29000000
#define PCIE_LEGACY_CLEAR_REG	0x2c
#define PCIE_LEGACY_CLEAR_MASK	BIT(0)
#define BIT_SET_OFFSET		0x1000
#define BIT_CLR_OFFSET		0x2000

struct sprd_ep_dev_notify {
	void  (*notify)(int event, void *data);
	void *data;
};

struct sprd_pci_ep_dev {
	struct pci_dev	*pdev;
	void __iomem	*cfg_base;	/* ep config bar base in rc */
	spinlock_t	irq_lock;	/* irq spinlock */
	spinlock_t	bar_lock;	/* bar spinlock */
	spinlock_t	set_irq_lock;	/* set irq spinlock */
	spinlock_t	set_bar_lock;	/* set bar spinlock */
	unsigned long	bar_res;

	u32	base_irq;
	u32	ipa_base_irq;
	u32	bak_irq_status;
	u8	iatu_unroll_enabled;
	u8	ep;
	u8	irq_cnt;
	bool	no_msi;
	bool	need_backup;
	void __iomem	*legacy_addr;

	struct resource	*bar[BAR_CNT];
	void __iomem	*bar_vir[BAR_MAX];
	void __iomem	*cpu_vir[BAR_MAX];
	dma_addr_t		src_addr[BAR_MAX];
	dma_addr_t		target_addr[BAR_MAX];
	size_t		map_size[BAR_MAX];
	int	event;
	struct work_struct	notify_work;
};

struct sprd_pci_ep_dev_save {
	bool		save_succ;
	unsigned long	bar_res;
	void __iomem	*bar_vir[BAR_MAX];
	void __iomem	*cpu_vir[BAR_MAX];
	dma_addr_t		src_addr[BAR_MAX];
	dma_addr_t		target_addr[BAR_MAX];
	size_t		map_size[BAR_MAX];
	u32		doorbell_enable;
	u32		doorbell_status;
	void __iomem	*cfg_base;
	u32	save_reg[PCIE_SAVE_REGION_NUM][PCIE_SAVE_REG_NUM];
};

#define VERSION_NUMBER "V1.1.6"
#define QUECTEL_SPRD_PCIE_VERSION "Quectel_Linux&Android_SPRD_PCIE_Driver_"VERSION_NUMBER

static void __iomem	*g_irq_addr[PCIE_EP_NR];
static struct sprd_pci_ep_dev_save g_ep_save[PCIE_EP_NR];
static struct sprd_pci_ep_dev *g_ep_dev[PCIE_EP_NR];
static irq_handler_t ep_dev_handler[PCIE_EP_NR][PCIE_MSI_MAX_IRQ];
static void *ep_dev_handler_data[PCIE_EP_NR][PCIE_MSI_MAX_IRQ];
static struct sprd_ep_dev_notify g_ep_dev_notify[PCIE_EP_NR];

static int sprd_ep_dev_get_bar(int ep);
static int sprd_ep_dev_put_bar(int ep, int bar);
static int sprd_ep_dev_adjust_region(struct sprd_pci_ep_dev *ep_dev,
				     int bar, dma_addr_t *cpu_addr_ptr,
				     size_t *size_ptr, dma_addr_t *offset_ptr);
static int sprd_ep_dev_just_map_bar(struct sprd_pci_ep_dev *ep_dev, int bar,
				    dma_addr_t cpu_addr, size_t size);
static int sprd_ep_dev_just_unmap_bar(struct sprd_pci_ep_dev *ep_dev, int bar);
static void __iomem *sprd_ep_dev_map_bar(int ep, int bar,
					 dma_addr_t cpu_addr,
					 size_t size);
static int sprd_ep_dev_unmap_bar(int ep, int bar);
static void sprd_pci_ep_dev_backup(struct sprd_pci_ep_dev *ep_dev);

static void sprd_pcie_iommu_init(struct device *dev)
{
	struct iommu_domain *domain = NULL;
	unsigned long pg_size;
	dma_addr_t start, end, addr;
	u32 base, size;
	int ret = 0;
	
	domain = iommu_get_domain_for_dev(dev);
	if(!domain) {
		dev_info(dev, "sprd_pcie_iommu_init domian null");
		return;
	}
	
	pg_size = 1UL << __ffs(domain->pgsize_bitmap);
	smem_get_area(SIPC_ID_MINIAP, &base, &size);
	start = ALIGN(base, pg_size);
	end   = ALIGN(base + size, pg_size);

	for (addr = start; addr < end; addr += pg_size) {
		phys_addr_t phys_addr;
	
		phys_addr = iommu_iova_to_phys(domain, addr);
		if (phys_addr) {
			dev_info(dev, "sprd_pcie_iommu_init iova:%d have been used", (u32)addr);
			continue;
		}
		ret = iommu_map(domain, addr, addr, pg_size, IOMMU_READ | IOMMU_WRITE);
		if (ret) {
			dev_info(dev, "sprd_pcie_iommu_init iommu_map failed");
			return;
		}
	}
}



#if (LINUX_VERSION_CODE < KERNEL_VERSION( 3,11,0 ))
int pci_enable_msi_range(struct pci_dev *dev, int minvec, int maxvec)
{
	int nvec = maxvec;
	int rc;

	if (maxvec < minvec)
		return -ERANGE;

	do {
		rc = pci_enable_msi_block(dev, nvec);
		if (rc < 0) {
			return rc;
		} else if (rc > 0) {
			if (rc < minvec)
				return -ENOSPC;
			nvec = rc;
		}
	} while (rc);

	return nvec;
}
#endif

int sprd_ep_dev_register_notify(int ep,
				void (*notify)(int event, void *data),
				void *data)
{
	struct sprd_ep_dev_notify *dev_notify;

	if (ep >= PCIE_EP_NR)
		return -EINVAL;

	dev_notify = &g_ep_dev_notify[ep];
	dev_notify->notify = notify;
	dev_notify->data = data;

	return 0;
}
EXPORT_SYMBOL_GPL(sprd_ep_dev_register_notify);

int sprd_ep_dev_unregister_notify(int ep)
{
	struct sprd_ep_dev_notify *notify;

	if (ep >= PCIE_EP_NR)
		return -EINVAL;

	notify = &g_ep_dev_notify[ep];
	notify->notify = NULL;
	notify->data = NULL;

	return 0;
}
EXPORT_SYMBOL_GPL(sprd_ep_dev_unregister_notify);

int sprd_ep_dev_set_irq_addr(int ep, void __iomem *irq_addr)
{
	if (ep >= PCIE_EP_NR)
		return -EINVAL;

	g_irq_addr[ep] = irq_addr;

	return 0;
}
EXPORT_SYMBOL_GPL(sprd_ep_dev_set_irq_addr);

int sprd_ep_dev_register_irq_handler(int ep, int irq,
				     irq_handler_t handler, void *data)
{
	struct sprd_pci_ep_dev *ep_dev;

	if (ep >= PCIE_EP_NR || irq >= PCIE_MSI_MAX_IRQ)
		return -EINVAL;

	ep_dev_handler[ep][irq] = handler;
	ep_dev_handler_data[ep][irq] = data;
	ep_dev = g_ep_dev[ep];

	if (handler && ep_dev &&
	    (BIT(irq) & ep_dev->bak_irq_status)) {
		ep_dev->bak_irq_status &= ~BIT(irq);
		handler(irq, data);
	}

	return 0;
}
EXPORT_SYMBOL_GPL(sprd_ep_dev_register_irq_handler);

int sprd_ep_dev_unregister_irq_handler(int ep, int irq)
{
	if (ep < PCIE_EP_NR && irq < PCIE_MSI_MAX_IRQ) {
		ep_dev_handler[ep][irq] = NULL;
		ep_dev_handler_data[ep][irq] = NULL;
		return 0;
	}

	return -EINVAL;
}
EXPORT_SYMBOL_GPL(sprd_ep_dev_unregister_irq_handler);

int sprd_ep_dev_register_irq_handler_ex(int ep,
					int from_irq,
					int to_irq,
					irq_handler_t handler,
					void *data)
{
	int i, ret;

	for (i = from_irq; i < to_irq + 1; i++) {
		ret = sprd_ep_dev_register_irq_handler(ep,
							i, handler, data);
		if (ret)
			return ret;
	}

	return 0;
}

int sprd_ep_dev_unregister_irq_handler_ex(int ep,
					  int from_irq,
					  int to_irq)
{
	int i, ret;

	for (i = from_irq; i < to_irq + 1; i++) {
		ret = sprd_ep_dev_unregister_irq_handler(ep, i);
		if (ret)
			return ret;
	}

	return 0;
}

void __iomem *sprd_ep_map_memory(int ep,
				 phys_addr_t cpu_addr,
				 size_t size)
{
	int bar;
	void __iomem *bar_addr;

	bar = sprd_ep_dev_get_bar(ep);
	if (bar < 0) {
		pr_err("%s: get bar err = %d\n", __func__, bar);
		return NULL;
	}

	bar_addr = sprd_ep_dev_map_bar(ep, bar, cpu_addr, size);
	if (!bar_addr) {
		pr_err("%s: map bar = %d err!\n", __func__, bar);
		sprd_ep_dev_put_bar(ep, bar);
		return NULL;
	}

	return bar_addr;
}
EXPORT_SYMBOL_GPL(sprd_ep_map_memory);

void sprd_ep_unmap_memory(int ep, const void __iomem *bar_addr)
{
	int bar;
	struct sprd_pci_ep_dev *ep_dev;

	if (ep >= PCIE_EP_NR || !g_ep_dev[ep])
		return;

	ep_dev = g_ep_dev[ep];

	for (bar = 0; bar < BAR_MAX; bar++) {
		if (bar_addr == ep_dev->cpu_vir[bar]) {
			sprd_ep_dev_unmap_bar(ep, bar);
			sprd_ep_dev_put_bar(ep, bar);
			break;
		}
	}
}
EXPORT_SYMBOL_GPL(sprd_ep_unmap_memory);

#ifdef CONFIG_SPRD_SIPA
phys_addr_t sprd_ep_ipa_map(int type, phys_addr_t target_addr, size_t size)
{
	int bar, ep = PCIE_EP_MODEM;
	dma_addr_t offset;
	struct sprd_pci_ep_dev *ep_dev;
	struct pci_dev *pdev;
	struct device *dev;
	struct resource *res;

	ep_dev = g_ep_dev[ep];
	if (!ep_dev)
		return 0;

	ep_dev = g_ep_dev[ep];
	pdev = ep_dev->pdev;
	dev = &pdev->dev;
#ifdef CONFIG_SPRD_IPA_PCIE_WORKROUND
	bar = type == PCIE_IPA_TYPE_MEM ? IPA_MEM_BAR : IPA_REG_BAR;
#else
	bar = sprd_ep_dev_get_bar(ep);
	if (bar < 0) {
		dev_err(dev, "ep: ipa map, get bar err = %d\n", bar);
		return 0;
	}
#endif
	res = &pdev->resource[bar];

	dev_info(dev, "ep: ipa map type=%d, addr=0x%lx, size=0x%lx\n",
		type,
		(unsigned long)target_addr,
		(unsigned long)size);

	/* 1st, adjust the map region */
	if (sprd_ep_dev_adjust_region(ep_dev, bar, &target_addr,
				      &size, &offset))
		return 0;

	/* than, map bar */
	if (sprd_ep_dev_just_map_bar(ep_dev, bar, target_addr, size))
		return 0;

	/* save for unmap */
	ep_dev->src_addr[bar] = res->start + offset;
	ep_dev->target_addr[bar] = target_addr;
	ep_dev->map_size[bar] = size;

	/*  return the cpu phy address */
	return res->start + offset;
}

int sprd_ep_ipa_unmap(int type, const phys_addr_t cpu_addr)
{
	int bar, ep = PCIE_EP_MODEM;
	bool find_bar = false;
	struct sprd_pci_ep_dev *ep_dev;
	struct pci_dev *pdev;
	struct resource *res;

	ep_dev = g_ep_dev[ep];
	if (!ep_dev)
		return -EINVAL;

	pdev = ep_dev->pdev;
	res = &pdev->resource[bar];

	dev_info(&pdev->dev, "ep: ipa unmap cpu_addr=0x%lx\n",
		(unsigned long)cpu_addr);

#ifdef CONFIG_SPRD_IPA_PCIE_WORKROUND
	bar = type == PCIE_IPA_TYPE_MEM ? IPA_MEM_BAR : IPA_REG_BAR;
	if (ep_dev->src_addr[bar] == cpu_addr)
		find_bar = true;
#else
	for (bar = 0; bar < BAR_MAX; bar++) {
		if (cpu_addr == ep_dev->src_addr[bar]) {
			find_bar = true;
			break;
		}
	}
#endif

	if (!find_bar) {
		dev_err(&pdev->dev, "ep: ipa unmap can't find bar!");
		return -EINVAL;
	}

	ep_dev->target_addr[bar] = 0;
	ep_dev->target_addr[bar] = 0;
	ep_dev->map_size[bar] = 0;
	return sprd_ep_dev_just_unmap_bar(ep_dev, bar);
}
#endif

int sprd_ep_dev_raise_irq(int ep, int irq)
{
	struct pci_dev *pdev;
	struct device *dev;
	struct sprd_pci_ep_dev *ep_dev;
	void __iomem	*base;
	u32 value;

	if (ep >= PCIE_EP_NR || !g_ep_dev[ep])
		return -ENODEV;

	ep_dev = g_ep_dev[ep];
	pdev = ep_dev->pdev;
	dev = &pdev->dev;

	dev_dbg(dev, "ep: raise, ep=%d, irq=%d\n", ep, irq);

	if (irq >= DOOR_BELL_IRQ_CNT) {
		dev_err(&pdev->dev, "raise err, irq=%d\n", irq);
		return -EINVAL;
	}

	spin_lock(&ep_dev->set_irq_lock);
	base = ep_dev->cfg_base + DOOR_BELL_BASE;
	value = readl_relaxed(base + DOOR_BELL_STATUS);
	writel_relaxed(value | DOOR_BELL_IRQ_VALUE(irq),
		       base + DOOR_BELL_STATUS);
	spin_unlock(&ep_dev->set_irq_lock);

	return 0;
}
EXPORT_SYMBOL_GPL(sprd_ep_dev_raise_irq);

int sprd_ep_dev_clear_doolbell_irq(int ep, int irq)
{
	struct pci_dev *pdev;
	struct device *dev;
	struct sprd_pci_ep_dev *ep_dev;
	void __iomem	*base;
	u32 value;

	if (ep >= PCIE_EP_NR || !g_ep_dev[ep])
		return -ENODEV;

	ep_dev = g_ep_dev[ep];
	pdev = ep_dev->pdev;
	dev = &pdev->dev;

	dev_dbg(dev, "ep: clear doorbell, ep=%d, irq=%d\n", ep, irq);

	if (irq >= DOOR_BELL_IRQ_CNT)
		return -EINVAL;

	spin_lock(&ep_dev->set_irq_lock);
	base = ep_dev->cfg_base + DOOR_BELL_BASE;
	value = readl_relaxed(base + DOOR_BELL_STATUS);
	if (value & DOOR_BELL_IRQ_VALUE(irq))
		writel_relaxed(value & (~DOOR_BELL_IRQ_VALUE(irq)),
			       base + DOOR_BELL_STATUS);
	spin_unlock(&ep_dev->set_irq_lock);

	return 0;
}

int sprd_ep_dev_set_backup(int ep)
{
	struct sprd_pci_ep_dev *ep_dev;

	if (ep >= PCIE_EP_NR || !g_ep_dev[ep])
		return -ENODEV;

	ep_dev = g_ep_dev[ep];
	ep_dev->need_backup = true;

	/* backup once immediately. */
	sprd_pci_ep_dev_backup(ep_dev);

	return 0;
}

int sprd_ep_dev_clear_backup(int ep)
{
	struct sprd_pci_ep_dev *ep_dev;

	if (ep >= PCIE_EP_NR || !g_ep_dev[ep])
		return -ENODEV;

	ep_dev = g_ep_dev[ep];
	ep_dev->need_backup = false;

	return 0;
}

int sprd_ep_dev_pass_smem(int ep, u32 base, u32 size)
{
	struct sprd_pci_ep_dev *ep_dev;
	void __iomem *reg_base;
	struct pci_dev *pdev;
	struct device *dev;

	if (ep >= PCIE_EP_NR || !g_ep_dev[ep])
		return -ENODEV;

	ep_dev = g_ep_dev[ep];
	pdev = ep_dev->pdev;
	dev = &pdev->dev;
	dev_info(&pdev->dev,
		"pass_smem, base=0x%x,size=0x%x\n",
		base, size);

	reg_base = ep_dev->cfg_base + DOOR_BELL_BASE;

	writel_relaxed(base, reg_base + DOOR_BELL_SMEMBASE);
	writel_relaxed(size, reg_base + DOOR_BELL_SMEMSIZE);

	return 0;
}

static inline u32 sprd_pci_ep_iatu_readl(struct sprd_pci_ep_dev *ep_dev,
					 u32 offset)
{
	return readl_relaxed(ep_dev->cfg_base + IATU_REG_BASE + offset);
}

static inline void sprd_pci_ep_iatu_writel(struct sprd_pci_ep_dev *ep_dev,
					   u32 offset, u32 value)
{
	writel_relaxed(value, ep_dev->cfg_base + IATU_REG_BASE + offset);
}

static int sprd_ep_dev_get_bar(int ep)
{
	int bar;
	int ret = -EBUSY;
	struct sprd_pci_ep_dev *ep_dev;

	if (ep >= PCIE_EP_NR || !g_ep_dev[ep])
		return -ENODEV;

	ep_dev = g_ep_dev[ep];
	spin_lock(&ep_dev->bar_lock);
	for (bar = BAR_MIN; bar < BAR_MAX; bar++) {
		if (ep_dev->bar[bar] && !test_bit(bar, &ep_dev->bar_res)) {
			set_bit(bar, &ep_dev->bar_res);
			ret = bar;
			break;
		}
	}
	spin_unlock(&ep_dev->bar_lock);

	return ret;
}

static int sprd_ep_dev_put_bar(int ep, int bar)
{
	int ret = -ENODEV;
	struct sprd_pci_ep_dev *ep_dev;

	if (ep >= PCIE_EP_NR || !g_ep_dev[ep])
		return -ENODEV;

	ep_dev = g_ep_dev[ep];
	spin_lock(&ep_dev->bar_lock);
	if (test_and_clear_bit(bar, &ep_dev->bar_res))
		ret = bar;
	spin_unlock(&ep_dev->bar_lock);

	return ret;
}

static int sprd_ep_dev_unr_set_bar(struct sprd_pci_ep_dev *ep_dev,
				   int bar,
				   dma_addr_t cpu_addr, size_t size)
{
	u32 retries, val;
	struct pci_dev *pdev = ep_dev->pdev;

	spin_lock(&ep_dev->set_bar_lock);

	/* bar n use region n to map, map to bar match mode */
	sprd_pci_ep_iatu_writel(ep_dev,
				PCIE_ATU_IB_REGION(bar) +
				PCIE_ATU_UNR_LOWER_TARGET,
				lower_32_bits(cpu_addr));
	sprd_pci_ep_iatu_writel(ep_dev,
				PCIE_ATU_IB_REGION(bar) +
				PCIE_ATU_UNR_UPPER_TARGET,
				upper_32_bits(cpu_addr));

	sprd_pci_ep_iatu_writel(ep_dev,
				PCIE_ATU_IB_REGION(bar) +
				PCIE_ATU_UNR_REGION_CTRL1,
				PCIE_ATU_TYPE_MEM);
	sprd_pci_ep_iatu_writel(ep_dev,
				PCIE_ATU_IB_REGION(bar) +
				PCIE_ATU_UNR_REGION_CTRL2,
				PCIE_ATU_ENABLE |
				PCIE_ATU_BAR_MODE_ENABLE | (bar << 8));

	spin_unlock(&ep_dev->set_bar_lock);

	/*
	 * Make sure ATU enable takes effect before any subsequent config
	 * and I/O accesses.
	 */
	for (retries = 0; retries < LINK_WAIT_MAX_IATU_RETRIES; retries++) {
		val = sprd_pci_ep_iatu_readl(ep_dev,
					     PCIE_ATU_IB_REGION(bar) +
					     PCIE_ATU_UNR_REGION_CTRL2);
		if (val & PCIE_ATU_ENABLE)
			return 0;

		dev_dbg(&pdev->dev,
			"ep: unr set bar[%d],  var = 0x%x\n",
			bar,
			val);
		/* wait a moment for polling ep atu enable bit */
		usleep_range(LINK_WAIT_IATU_MIN, LINK_WAIT_IATU_MAX);
	}

	return -EINVAL;
}

static int sprd_ep_dev_unr_clear_bar(struct sprd_pci_ep_dev *ep_dev, int bar)
{
	struct pci_dev *pdev = ep_dev->pdev;

	dev_dbg(&pdev->dev, "ep: unr clear map bar=%d\n", bar);

	spin_lock(&ep_dev->set_bar_lock);

	sprd_pci_ep_iatu_writel(ep_dev,
				PCIE_ATU_IB_REGION(bar) +
				PCIE_ATU_UNR_REGION_CTRL2,
				(u32)(~PCIE_ATU_ENABLE));
	spin_unlock(&ep_dev->set_bar_lock);

	return 0;
}

static int sprd_ep_dev_adjust_region(struct sprd_pci_ep_dev *ep_dev, int bar,
					     dma_addr_t *cpu_addr_ptr,
					     size_t *size_ptr,
					     dma_addr_t *offset_ptr)
{
	dma_addr_t cpu_addr, base, offset;
	resource_size_t bar_size, size;
	struct pci_dev *pdev = ep_dev->pdev;
	struct resource *res = &pdev->resource[bar];

	size = (resource_size_t)*size_ptr;
	cpu_addr = *cpu_addr_ptr;
	bar_size = resource_size(res);

	/* size must align with page */
	size = PAGE_ALIGN(size);

	/* base must be divisible by bar size for bar match mode */
	base = cpu_addr / bar_size * bar_size;
	offset = cpu_addr - base;
	size += PAGE_ALIGN(offset);

	/* size must < bar size  */
	if (size > bar_size) {
		dev_err(&pdev->dev,
			"bar[%d]:size=0x%lx > 0x%lx\n",
			bar,
			(unsigned long)size,
			(unsigned long)bar_size);
		return -EINVAL;
	}

	dev_dbg(&pdev->dev,
		"bar[%d]: base=0x%lx,size=0x%lx,offset=0x%lx\n",
		bar, (unsigned long)base,
		(unsigned long)size,
		(unsigned long)offset);

	*size_ptr = (size_t)size;
	*offset_ptr = offset;
	*cpu_addr_ptr = base;

	return 0;
}

static int sprd_ep_dev_just_map_bar(struct sprd_pci_ep_dev *ep_dev, int bar,
			 dma_addr_t cpu_addr, size_t size)
{
	u32 retries, val;
	struct pci_dev *pdev;
	struct device *dev;

	pdev = ep_dev->pdev;
	dev = &pdev->dev;

	dev_dbg(dev, "ep: map bar=%d, addr=0x%lx, size=0x%lx\n",
		bar,
		(unsigned long)cpu_addr,
		(unsigned long)size);

	if (ep_dev->iatu_unroll_enabled)
		return sprd_ep_dev_unr_set_bar(ep_dev, bar, cpu_addr, size);

	spin_lock(&ep_dev->set_bar_lock);

	/* bar n use region n to map, map to bar match mode */
	sprd_pci_ep_iatu_writel(ep_dev,
				PCIE_ATU_VIEWPORT,
				PCIE_ATU_REGION_INBOUND | bar);
	sprd_pci_ep_iatu_writel(ep_dev, PCIE_ATU_LOWER_TARGET,
				lower_32_bits(cpu_addr));
	sprd_pci_ep_iatu_writel(ep_dev, PCIE_ATU_UPPER_TARGET,
				upper_32_bits(cpu_addr));
	sprd_pci_ep_iatu_writel(ep_dev,
				PCIE_ATU_CR1,
				PCIE_ATU_TYPE_MEM);
	sprd_pci_ep_iatu_writel(ep_dev,
				PCIE_ATU_CR2,
				PCIE_ATU_ENABLE |
				PCIE_ATU_BAR_MODE_ENABLE | (bar << 8));

	spin_unlock(&ep_dev->set_bar_lock);

	/*
	 * Make sure ATU enable takes effect
	 * before any subsequent config  and I/O accesses.
	 */
	for (retries = 0;
	     retries < LINK_WAIT_MAX_IATU_RETRIES;
	     retries++) {
		val = sprd_pci_ep_iatu_readl(ep_dev, PCIE_ATU_CR2);
		if (val & PCIE_ATU_ENABLE)
			return 0;

		/* wait a moment for polling ep atu enable bit */
		usleep_range(LINK_WAIT_IATU_MIN, LINK_WAIT_IATU_MAX);
	}

	return -EINVAL;
}

static int sprd_ep_dev_just_unmap_bar(struct sprd_pci_ep_dev *ep_dev, int bar)
{
	struct pci_dev *pdev;
	struct device *dev;

	pdev = ep_dev->pdev;
	dev = &pdev->dev;

	dev_dbg(dev, "ep: unmap bar = %d\n", bar);

	if (ep_dev->iatu_unroll_enabled)
		return sprd_ep_dev_unr_clear_bar(ep_dev, bar);

	spin_lock(&ep_dev->set_bar_lock);

	sprd_pci_ep_iatu_writel(ep_dev, PCIE_ATU_VIEWPORT,
				PCIE_ATU_REGION_INBOUND | bar);
	sprd_pci_ep_iatu_writel(ep_dev, PCIE_ATU_CR2,
				(u32)(~PCIE_ATU_ENABLE));

	spin_unlock(&ep_dev->set_bar_lock);

	return 0;
}

static void __iomem *sprd_ep_dev_map_bar(int ep, int bar,
			 dma_addr_t cpu_addr, size_t size)
{
	resource_size_t offset;
	struct pci_dev *pdev;
	struct device *dev;
	struct sprd_pci_ep_dev *ep_dev;
	void __iomem *bar_vir;
	struct resource *res;

	if (ep >= PCIE_EP_NR || !g_ep_dev[ep])
		return NULL;

	ep_dev = g_ep_dev[ep];
	pdev = ep_dev->pdev;
	dev = &pdev->dev;

	/* bar is be used */
	if (ep_dev->bar_vir[bar]) {
		dev_err(dev, "ep: bar[%d] is used!", bar);
		return NULL;
	}

	/* 1st, adjust the map region */
	if (sprd_ep_dev_adjust_region(ep_dev, bar, &cpu_addr, &size, &offset))
		return NULL;

	/* than, ioremap, if map failed, no need to set bar */
	res = &pdev->resource[bar];
#ifndef ioremap_nocache
#define ioremap_nocache ioremap
#endif
	bar_vir = ioremap_nocache(res->start, size);
	if (!bar_vir) {
		dev_err(dev, "ep: map error, bar=%d, addr=0x%lx, size=0x%lx\n",
			bar,
			(unsigned long)cpu_addr,
			(unsigned long)size);
		return NULL;
	}

	if (sprd_ep_dev_just_map_bar(ep_dev, bar, cpu_addr, size)) {
		dev_err(dev, "ep: map bar =%d\n", bar);
		iounmap(ep_dev->bar_vir[bar]);
		return NULL;
	}

	ep_dev->bar_vir[bar] = (void __iomem *)bar_vir;
	ep_dev->cpu_vir[bar] = (void __iomem *)(bar_vir + offset);
	ep_dev->src_addr[bar] = res->start + offset;
	ep_dev->target_addr[bar] = cpu_addr;
	ep_dev->map_size[bar] = size;

	return ep_dev->cpu_vir[bar];
}

static int sprd_ep_dev_unmap_bar(int ep, int bar)
{
	struct pci_dev *pdev;
	struct device *dev;
	struct sprd_pci_ep_dev *ep_dev;

	if (ep >= PCIE_EP_NR || !g_ep_dev[ep])
		return -ENODEV;

	ep_dev = g_ep_dev[ep];
	pdev = ep_dev->pdev;
	dev = &pdev->dev;

	dev_info(dev, "ep: unmap bar = %d\n", bar);

	if (!ep_dev->bar_vir[bar])
		return -ENODEV;

	sprd_ep_dev_just_unmap_bar(ep_dev, bar);

	iounmap(ep_dev->bar_vir[bar]);
	ep_dev->bar_vir[bar] = NULL;
	ep_dev->cpu_vir[bar] = NULL;
	ep_dev->src_addr[bar] = 0;
	ep_dev->target_addr[bar] = 0;
	ep_dev->map_size[bar] = 0;

	return 0;
}

static void sprd_pci_ep_dev_clear_legacy_irq(struct sprd_pci_ep_dev *ep_dev)
{
	if (!ep_dev->legacy_addr)
		return;

	writel_relaxed(PCIE_LEGACY_CLEAR_MASK,
		       ep_dev->legacy_addr + BIT_CLR_OFFSET + PCIE_LEGACY_CLEAR_REG);
}

static irqreturn_t sprd_pci_ep_dev_irqhandler(int irq, void *dev_ptr)
{
	struct sprd_pci_ep_dev *ep_dev = dev_ptr;
	struct pci_dev *pdev = ep_dev->pdev;
	struct device *dev = &pdev->dev;
	irq_handler_t handler;
	u32 i, j, value;
	int ipa_irq;

	if (ep_dev->no_msi) {
		/* clear irq */
		sprd_pci_ep_dev_clear_legacy_irq(ep_dev);

		/* workaroud for IPA */
		handler = ep_dev_handler[ep_dev->ep][PCIE_MSI_IPA];
		if (handler)
			handler(irq, ep_dev_handler_data[ep_dev->ep][PCIE_MSI_IPA]);


		value = BL_READL(g_irq_addr[ep_dev->ep]);
		dev_dbg(dev, "ep: irq handler. irq = 0x%x, base=%d\n", value, ep_dev->base_irq);
		for (i = 0; i < 32; i++) {
			if (value & BIT(i)) {
				/* clear iqr bit*/
				value = BL_READL(g_irq_addr[ep_dev->ep]);
				value &= ~(BIT(i));
				BL_WRITEL(value,g_irq_addr[ep_dev->ep]);
				j = i - ep_dev->base_irq;
				if (j >= PCIE_MSI_MAX_IRQ)
					continue;

				handler = ep_dev_handler[ep_dev->ep][j];
				if (handler)
					handler(irq, ep_dev_handler_data[ep_dev->ep][j]);
				else
					ep_dev->bak_irq_status |= BIT(j);
			}
		}
		return IRQ_HANDLED;
	}

	dev_dbg(dev, "ep: irq handler. irq = %d\n",  irq);
	/* for ipa hw irq. */
	ipa_irq = irq - (pdev->irq + ep_dev->ipa_base_irq);
	if (ipa_irq >= 0 && ipa_irq < IPA_HW_IRQ_CNT) {
		handler = ep_dev_handler[ep_dev->ep][PCIE_MSI_IPA];
		if (handler)
			handler(ipa_irq, ep_dev_handler_data[ep_dev->ep][PCIE_MSI_IPA]);
		else
			ep_dev->bak_irq_status |= BIT(PCIE_MSI_IPA);

		return IRQ_HANDLED;
	}


	irq -= (pdev->irq + ep_dev->base_irq);
	if (irq >= PCIE_MSI_MAX_IRQ || irq < 0) {
		dev_err(dev, "ep: error, irq = %d", irq);
		return IRQ_HANDLED;
	}

	handler = ep_dev_handler[ep_dev->ep][irq];
	if (handler)
		handler(irq, ep_dev_handler_data[ep_dev->ep][irq]);
	else
		ep_dev->bak_irq_status |= BIT(irq);

	return IRQ_HANDLED;
}

static void sprd_pci_ep_save_reg(struct sprd_pci_ep_dev *ep_dev)
{
	int i, j;
	u32 (*save_reg)[PCIE_SAVE_REG_NUM];
	static struct sprd_pci_ep_dev_save *ep_save;

	ep_save = &g_ep_save[ep_dev->ep];
	save_reg = ep_save->save_reg;

	for (i = 0; i < PCIE_SAVE_REGION_NUM; i += 2) {
		for (j = 0; j < PCIE_SAVE_REG_NUM; j++) {
			save_reg[i][j] =
				sprd_pci_ep_iatu_readl(ep_dev,
						       PCIE_ATU_OB_REGION(i) +
						       j * sizeof(u32));
			save_reg[i + 1][j] =
				sprd_pci_ep_iatu_readl(ep_dev,
						       PCIE_ATU_IB_REGION(i) +
						       j * sizeof(u32));
		}
	}

	ep_save->doorbell_enable = sprd_pci_ep_iatu_readl(ep_dev,
							  DOOR_BELL_BASE +
							  DOOR_BELL_ENABLE);
	ep_save->doorbell_status = sprd_pci_ep_iatu_readl(ep_dev,
							  DOOR_BELL_BASE +
							  DOOR_BELL_STATUS);
	ep_save->cfg_base = ep_dev->cfg_base;

	ep_save->save_succ = true;
}

static void sprd_pci_ep_dev_backup(struct sprd_pci_ep_dev *ep_dev)
{
	struct pci_dev *pdev = ep_dev->pdev;
	struct device *dev = &pdev->dev;
	struct sprd_pci_ep_dev_save *ep_save;
	int i;

	ep_save = &g_ep_save[ep_dev->ep];

	/* save some member */
	ep_save->bar_res = ep_dev->bar_res;
	for (i = 0; i < BAR_MAX; i++) {
		if (!ep_dev->src_addr[i])
			continue;

		dev_info(dev, "ep: backup bar=%d, addr=0x%lx, size=0x%lx\n",
			 i,
			 (unsigned long)ep_save->target_addr[i],
			 (unsigned long)ep_save->map_size[i]);

		ep_save->bar_vir[i] =  ep_dev->bar_vir[i];
		ep_save->cpu_vir[i] =  ep_dev->cpu_vir[i];
		ep_save->src_addr[i] =  ep_dev->src_addr[i];
		ep_save->target_addr[i] =  ep_dev->target_addr[i];
		ep_save->map_size[i] =  ep_dev->map_size[i];
	}

	/* save ep reg */
	sprd_pci_ep_save_reg(ep_dev);
}

static void sprd_pci_ep_dev_restore(struct sprd_pci_ep_dev *ep_dev)
{
	struct pci_dev *pdev = ep_dev->pdev;
	struct device *dev = &pdev->dev;
	struct sprd_pci_ep_dev_save *ep_save;
	int i;

	ep_save = &g_ep_save[ep_dev->ep];

	/* save some member */
	ep_dev->bar_res = ep_save->bar_res;
	for (i = 0; i < BAR_MAX; i++) {
		if (!ep_save->src_addr[i])
			continue;

		ep_dev->bar_vir[i] =  ep_save->bar_vir[i];
		ep_dev->cpu_vir[i] =  ep_save->cpu_vir[i];
		ep_dev->src_addr[i] = ep_save->src_addr[i];
		ep_dev->target_addr[i] =  ep_save->target_addr[i];
		ep_dev->map_size[i] = ep_save->map_size[i];

		dev_info(dev, "ep: restore bar=%d, addr=0x%lx, size=0x%lx\n",
			 i,
			 (unsigned long)ep_dev->target_addr[i],
			 (unsigned long)ep_dev->map_size[i]);

		if (sprd_ep_dev_just_map_bar(ep_dev,
					     i,
					     ep_dev->target_addr[i],
					     ep_dev->map_size[i]))
			dev_err(dev, "ep: restore map err i = %d.\n", i);
	}
}

static void sprd_pci_ep_notify_fn(struct work_struct *work)
{
	struct sprd_ep_dev_notify *notify;
	struct sprd_pci_ep_dev *ep_dev = container_of(work,
						      struct sprd_pci_ep_dev,
						      notify_work);

	notify = &g_ep_dev_notify[ep_dev->ep];
	if (notify->notify)
		notify->notify(ep_dev->event, notify->data);
}

static int sprd_pci_ep_dev_probe(struct pci_dev *pdev,
				 const struct pci_device_id *ent)
{
	int i, err, irq_cnt =  0;
	u32 val;
	enum dev_pci_barno bar;
	struct device *dev = &pdev->dev;
	struct sprd_pci_ep_dev *ep_dev;
	struct resource *res;

	dev_info(dev, "ep: probe\n");

	sprd_pcie_iommu_init(dev);
	
	ep_dev = devm_kzalloc(dev, sizeof(*ep_dev), GFP_KERNEL);
	if (!ep_dev)
		return -ENOMEM;

	ep_dev->pdev = pdev;

	if (ent->device == PCI_DEVICE_ID_SPRD_ORCA)
		ep_dev->ep = PCIE_EP_MODEM;
	else {
		dev_err(dev, "ep: Cannot support ep device = 0x%x\n",
			ent->device);
		return -EINVAL;
	}

	err = pci_enable_device(pdev);
	if (err) {
		dev_err(dev, "ep: Cannot enable PCI device\n");
		return err;
	}

	err = pci_request_regions(pdev, DRV_MODULE_NAME);
	if (err) {
		dev_err(dev, "ep: Cannot obtain PCI resources\n");
		goto err_disable_pdev;
	}

	pci_set_master(pdev);

#ifdef PCI_IRQ_MSI
	irq_cnt = pci_alloc_irq_vectors(pdev,
				    1,
				    MAX_SUPPORT_IRQ,
				    PCI_IRQ_MSI);
#else
	irq_cnt = pci_enable_msi_range(pdev, 1, MAX_SUPPORT_IRQ);
#endif

	if (sprd_pcie_is_defective_chip()) {
		ep_dev->base_irq = REQUEST_BASE_IRQ_DEFECT;
		ep_dev->ipa_base_irq = IPA_HW_IRQ_BASE_DEFECT;
	} else {
		ep_dev->base_irq = REQUEST_BASE_IRQ;
		ep_dev->ipa_base_irq = IPA_HW_IRQ_BASE;
	}

	for (bar = BAR_0; bar <= BAR_5; bar++) {
		res = &pdev->resource[bar];
		dev_info(dev, "ep: BAR[%d] %pR\n", bar, res);
		/* only save mem bar */
		if (resource_type(res) == IORESOURCE_MEM)
			ep_dev->bar[bar] = res;
	}

	ep_dev->cfg_base = pci_ioremap_bar(pdev, EP_CFG_BAR);
	if (!ep_dev->cfg_base) {
		dev_err(dev, "ep: failed to map cfg bar.\n");
		err = -ENOMEM;
		goto err_disable_msi;
	}

	/* clear all 32 bit door bell */
	writel_relaxed(0x0,
		       ep_dev->cfg_base + DOOR_BELL_BASE + DOOR_BELL_STATUS);

	pci_set_drvdata(pdev, ep_dev);
	pci_read_config_dword(ep_dev->pdev, PCIE_ATU_VIEWPORT, &val);
	/*
	 * this atu view port reg is 0xffffffff means that the ep device
	 * doesn't support atu view port, we need unroll iatu registers
	 */
	dev_info(dev, "ep: atu_view_port val = 0x%x", val);
	ep_dev->iatu_unroll_enabled = val == 0xffffffff;

	/* default , PCIE_EP_PROBE */
	ep_dev->event = PCIE_EP_PROBE;
	g_ep_dev[ep_dev->ep] = ep_dev;

	if (!ep_dev->bar[BAR_1] || !ep_dev->bar[BAR_3]) {
		/* only 2 bar, set PCIE_EP_PROBE_BEFORE_SPLIT_BAR */
		ep_dev->event = PCIE_EP_PROBE_BEFORE_SPLIT_BAR;
		dev_info(dev, "ep:bar not ready, wait the next probe!");
	}

	/* restore all the config */
	if (ep_dev->event == PCIE_EP_PROBE)
		sprd_pci_ep_dev_restore(ep_dev);

	/* start notify work */
	INIT_WORK(&ep_dev->notify_work, sprd_pci_ep_notify_fn);
	schedule_work(&ep_dev->notify_work);

	if (irq_cnt < MAX_SUPPORT_IRQ) {
		err = irq_cnt < 0 ? irq_cnt : -EINVAL;
		ep_dev->no_msi = true;
		dev_info(dev, "ep: failed to get MSI, err=%d, irq=%d\n", err, pdev->irq);

		ep_dev->legacy_addr = sprd_ep_map_memory(ep_dev->ep, PCIE_LEGACY_CLEAR_BASE, 0x4000);
		/* request legacy irq */
		err = devm_request_irq(dev, pdev->irq, sprd_pci_ep_dev_irqhandler,
					   IRQF_SHARED, DRV_MODULE_NAME, ep_dev);
		if (err)
			dev_warn(dev,
				"ep: failed to request legacy %d\n",
				pdev->irq);

	} else {
		ep_dev->irq_cnt = irq_cnt;
		dev_info(dev, "ep: request IRQ = %d, cnt =%d\n",
			 pdev->irq,
		 ep_dev->irq_cnt);

		/* request msi irq */
		for (i = ep_dev->base_irq;
			 i < ep_dev->base_irq + PCIE_MSI_MAX_IRQ;
			 i++) {
			err = devm_request_irq(dev, pdev->irq + i,
						   sprd_pci_ep_dev_irqhandler,
						   IRQF_SHARED, DRV_MODULE_NAME, ep_dev);
			if (err)
				dev_warn(dev,
					"ep: failed to request IRQ %d for MSI %d\n",
					pdev->irq + i, i + 1);
		}

	#ifdef CONFIG_SPRD_IPA_PCIE_WORKROUND
		for (i = ep_dev->ipa_base_irq;
			 i < ep_dev->ipa_base_irq + IPA_HW_IRQ_CNT;
			 i++) {
			err = devm_request_irq(dev, pdev->irq + i,
						   sprd_pci_ep_dev_irqhandler,
						   IRQF_SHARED, DRV_MODULE_NAME,
						   ep_dev);
			if (!err)
				sprd_pcie_teardown_msi_irq(pdev->irq + i);
		}
	#endif
	}

#ifndef SPRD_PCIE_USE_DTS
	sipa_module_init(dev);
	sipa_eth_init();
    sipa_dummy_init();
#endif 

	return 0;

err_disable_msi:
	pci_disable_msi(pdev);
	pci_release_regions(pdev);

err_disable_pdev:
	pci_disable_device(pdev);

	return err;
}

static void sprd_pci_ep_dev_remove(struct pci_dev *pdev)
{
	u32 i;
	struct sprd_ep_dev_notify *notify;
	struct sprd_pci_ep_dev *ep_dev = pci_get_drvdata(pdev);
    struct sipa_core *ctrl = sipa_get_ctrl_pointer();
	dev_info(&pdev->dev, "ep: remove\n");
    ctrl->remote_ready = false;
    spipe_device_down();
    spool_device_down();

	cancel_work_sync(&ep_dev->notify_work);

	/* first notify PCIE_EP_REMOVE */
	notify = &g_ep_dev_notify[ep_dev->ep];
	if (notify->notify)
		notify->notify(PCIE_EP_REMOVE, notify->data);

	/* back up some config before remove */
	if (ep_dev->need_backup)
		sprd_pci_ep_dev_backup(ep_dev);

	if (ep_dev->no_msi) {
		devm_free_irq(&pdev->dev, pdev->irq, ep_dev);
	} else {
		for (i = ep_dev->base_irq; i < ep_dev->base_irq + PCIE_MSI_MAX_IRQ; i++)
			devm_free_irq(&pdev->dev, pdev->irq + i, ep_dev);

	#ifdef CONFIG_SPRD_IPA_PCIE_WORKROUND
		for (i = ep_dev->ipa_base_irq;
			 i < ep_dev->ipa_base_irq + IPA_HW_IRQ_CNT;
			 i++)
			devm_free_irq(&pdev->dev, pdev->irq + i, ep_dev);
	#endif
	}

	pci_disable_msi(pdev);

#ifndef SPRD_PCIE_USE_DTS
	sipa_dummy_exit();
	sipa_eth_exit();
	sipa_module_exit();
#endif

	if (ep_dev->legacy_addr) {
		sprd_ep_unmap_memory(ep_dev->ep, ep_dev->legacy_addr);
		ep_dev->legacy_addr = NULL;
		dev_info(&ep_dev->pdev->dev, "set ep_dev->legacy_addr = %lx\n", (long)ep_dev->legacy_addr);
	}

	if (ep_dev->cfg_base) {
		iounmap(ep_dev->cfg_base);
		ep_dev->cfg_base = NULL;
	}
		
	pci_release_regions(pdev);
	pci_disable_device(pdev);

	g_ep_dev[ep_dev->ep] = NULL;
	ep_dev->bar_res = 0;
}

static const struct pci_device_id sprd_pci_ep_dev_tbl[] = {
	{ PCI_DEVICE(PCI_VENDOR_ID_SPRD, PCI_DEVICE_ID_SPRD_ORCA) },
	{ }
};
MODULE_DEVICE_TABLE(pci, sprd_pci_ep_dev_tbl);

#ifdef CONFIG_PM_SLEEP
static int sprd_pci_ep_suspend(struct device *dev)
{
	struct pci_dev *pdev = to_pci_dev(dev);
	int rc;

	dev_info(dev, "suspend\n");

	/* Exec pci PCI_D3cold one time */
	if (pdev->current_state != PCI_D0) {
		dev_info(dev, "done for pm %d\n", pdev->current_state);
		return 0;
	}

	/*
	 * TODO: The HAL will ask the shared memory layer whether D3 is allowed.
	 */

	/* Save the PCI configuration space of a device before suspending. */
	rc = pci_save_state(pdev);
	if (rc) {
		dev_err(dev, "pci_save_state error=%d\n", rc);
		return rc;
	}

	/* Set the power state of a PCI device.
	 * Transition a device to a new power state, using the device's PCI PM
	 * registers.
	 */
	rc = pci_set_power_state(pdev, PCI_D3cold);
	if (rc) {
		dev_err(dev, "pci_set_power_state error=%d\n", rc);
		return rc;
	}
	return 0;
}

static int sprd_pci_ep_resume(struct device *dev)
{
	struct pci_dev *pdev = to_pci_dev(dev);
	int rc;

	dev_info(dev, "resume\n");

	/* Set the power state of a PCI device. */
	rc = pci_set_power_state(pdev, PCI_D0);
	if (rc) {
		dev_err(dev, "pci_set_power_state error=%d\n", rc);
		return rc;
	}

	/* Restore the saved state of a PCI device. */
	pci_restore_state(pdev);

	/* TODO: The HAL shall inform that the device is active. */
	return 0;
}
#endif /* CONFIG_PM_SLEEP */

static const struct dev_pm_ops sprd_pci_ep_pm = {
	SET_SYSTEM_SLEEP_PM_OPS(sprd_pci_ep_suspend,
				sprd_pci_ep_resume)
};

static struct pci_driver sprd_pci_ep_dev_driver = {
	.name		= DRV_MODULE_NAME,
	.id_table	= sprd_pci_ep_dev_tbl,
	.probe		= sprd_pci_ep_dev_probe,
	.remove		= sprd_pci_ep_dev_remove,
	.driver		= {
		.pm = &sprd_pci_ep_pm,
	}
};
//module_pci_driver(sprd_pci_ep_dev_driver);

#if defined(CONFIG_DEBUG_FS)
static void sprd_pci_ep_dev_save_show(struct seq_file *m,
				      struct sprd_pci_ep_dev_save *ep_save,
				      int ep)
{
	u32 i;

	seq_printf(m, "ep-save-%d configs:\n", ep);
	seq_printf(m, "bar_res = 0x%lx\n", ep_save->bar_res);

	for (i = 0; i < BAR_MAX; i++) {
		seq_printf(m, "src_addr[%d] = 0x%lx\n",
			   i,
			   (unsigned long)ep_save->src_addr[i]);
		seq_printf(m, "target_addr[%d] = 0x%lx\n",
			   i,
			   (unsigned long)ep_save->target_addr[i]);
		seq_printf(m, "map_size[%d] = 0x%lx\n",
			   i,
			   (unsigned long)ep_save->map_size[i]);
	}
}

static void sprd_pci_ep_dev_config_show(struct seq_file *m,
					struct sprd_pci_ep_dev *ep_dev)
{
	u32 i;
	void __iomem *base;

	seq_printf(m, "ep-%d configs:\n", ep_dev->ep);

	/* doorbell regs */
	seq_puts(m, "door bell regs:\n");
	base = ep_dev->cfg_base + DOOR_BELL_BASE;

	seq_printf(m, "irq_enable = 0x%08x\n irq_status = 0x%08x\n",
		   readl_relaxed(base + DOOR_BELL_ENABLE),
		   readl_relaxed(base + DOOR_BELL_STATUS));

	/* iatu reg regs */
	seq_puts(m, "iatu regs reg:\n");
	for (i = 0; i < IATU_MAX_REGION * 2; i++) {
		base = ep_dev->cfg_base + IATU_REG_BASE + i * 100;
		seq_printf(m, "IATU[%d]:\n", i);
		seq_printf(m, "0x%p: 0x%08x 0x%08x 0x%08x 0x%08x\n",
			   base,
			   readl_relaxed(base + 0x0),
			   readl_relaxed(base + 0x4),
			   readl_relaxed(base + 0x8),
			   readl_relaxed(base + 0xc));
		base += 0x10;
		seq_printf(m, "0x%p: 0x%08x 0x%08x 0x%08x 0x%08x\n",
			   base,
			   readl_relaxed(base + 0x0),
			   readl_relaxed(base + 0x4),
			   readl_relaxed(base + 0x8),
			   readl_relaxed(base + 0x10));
	}
}

static void sprd_pci_ep_dev_backup_show(struct seq_file *m,
					struct sprd_pci_ep_dev_save *ep_save,
					int ep)
{
	int i;
	u32 (*save_reg)[PCIE_SAVE_REG_NUM];
	void __iomem *base;

	save_reg = ep_save->save_reg;

	seq_printf(m, "ep-%d backup configs:\n", ep);

	/* doorbell regs */
	seq_puts(m, "door bell regs:\n");
	seq_printf(m, "irq_enable = 0x%08x\n irq_status = 0x%08x\n",
		   ep_save->doorbell_enable,
		   ep_save->doorbell_status);

	/* iatu reg regs */
	seq_puts(m, "iatu regs reg:\n");
	for (i = 0; i < PCIE_SAVE_REGION_NUM; i++) {
		seq_printf(m, "IATU[%d]:\n", i);
		base = ep_save->cfg_base + IATU_REG_BASE + i * 100;

		seq_printf(m, "0x%p: 0x%08x 0x%08x 0x%08x 0x%08x\n",
			   base,
			   save_reg[i][0],
			   save_reg[i][1],
			   save_reg[i][2],
			   save_reg[i][3]);
		base += 0x10;
		seq_printf(m, "0x%p: 0x%08x 0x%08x 0x%08x 0x%08x\n",
			   base,
			   save_reg[i][4],
			   save_reg[i][5],
			   save_reg[i][6],
			   save_reg[i][7]);
	}
}

static int sprd_pci_ep_dev_show(struct seq_file *m, void *unused)
{
	u32 i;
	struct sprd_pci_ep_dev *ep_dev;
	struct sprd_pci_ep_dev_save *ep_save;

	for (i = 0; i < PCIE_EP_NR; i++) {
		/* ep_save configus */
		ep_save = &g_ep_save[i];
		ep_dev = g_ep_dev[i];

		if (!ep_dev && !ep_save->save_succ)
			continue;

		if (ep_save)
			sprd_pci_ep_dev_save_show(m, ep_save, i);

		if (ep_dev)
			sprd_pci_ep_dev_config_show(m, ep_dev);
		else
			sprd_pci_ep_dev_backup_show(m, ep_save, i);
	}

	return 0;
}

static int sprd_pci_ep_dev_open(struct inode *inode, struct file *file)
{
	return single_open(file, sprd_pci_ep_dev_show, NULL);
}

static const struct file_operations sprd_pci_ep_dev_fops = {
	.owner = THIS_MODULE,
	.open = sprd_pci_ep_dev_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static struct dentry *g_ep_debugfs_root;

static int sprd_pci_ep_dev_init_debugfs(void)
{
	struct dentry *g_ep_debugfs_root = debugfs_create_dir("ep_dev", NULL);

	if (!g_ep_debugfs_root)
		return -ENXIO;

	debugfs_create_file("ep", 0444,
			    g_ep_debugfs_root,
			    NULL, &sprd_pci_ep_dev_fops);
	return 0;
}

static void sprd_pci_ep_dev_remove_debugfs(void)
{
	debugfs_remove_recursive(g_ep_debugfs_root);
}
#endif

static int __init sprd_pci_ep_dev_init(void)
{
	pr_info("%s %s\n", __func__, QUECTEL_SPRD_PCIE_VERSION);
#if defined(CONFIG_DEBUG_FS)
	sprd_pci_ep_dev_init_debugfs();
    sipc_init_debugfs();
#endif
	sipc_init();
	spipe_init();
	spool_init();
	modem_power_manager_init();
	return pci_register_driver(&sprd_pci_ep_dev_driver);	 
}

static void __exit sprd_pci_ep_dev_exit(void)
{
    spipe_exit();
	spool_exit();
	modem_power_manager_exit();
	pci_unregister_driver(&sprd_pci_ep_dev_driver);
#if defined(CONFIG_DEBUG_FS)
	sprd_pci_ep_dev_remove_debugfs();
#endif
	sipc_exit();           
}

module_init(sprd_pci_ep_dev_init);
module_exit(sprd_pci_ep_dev_exit);


MODULE_DESCRIPTION("SPRD PCI EP DEVICE HOST DRIVER");
MODULE_AUTHOR("Wenping Zhou <wenping.zhou@unisoc.com>");
MODULE_LICENSE("GPL v2");
MODULE_VERSION(QUECTEL_SPRD_PCIE_VERSION);
