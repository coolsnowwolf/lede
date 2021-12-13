/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 MediaTek Inc. All Rights Reserved.
 *
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 */

#ifndef _MTK_SNAND_OS_H_
#define _MTK_SNAND_OS_H_

#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/limits.h>
#include <linux/types.h>
#include <linux/bitops.h>
#include <linux/sizes.h>
#include <linux/iopoll.h>
#include <linux/hrtimer.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>
#include <linux/io.h>
#include <asm/div64.h>

struct mtk_snand_plat_dev {
	struct device *dev;
	struct completion done;
};

/* Polling helpers */
#define read16_poll_timeout(addr, val, cond, sleep_us, timeout_us) \
	readw_poll_timeout((addr), (val), (cond), (sleep_us), (timeout_us))

#define read32_poll_timeout(addr, val, cond, sleep_us, timeout_us) \
	readl_poll_timeout((addr), (val), (cond), (sleep_us), (timeout_us))

/* Timer helpers */
#define mtk_snand_time_t ktime_t

static inline mtk_snand_time_t timer_get_ticks(void)
{
	return ktime_get();
}

static inline mtk_snand_time_t timer_time_to_tick(uint32_t timeout_us)
{
	return ktime_add_us(ktime_set(0, 0), timeout_us);
}

static inline bool timer_is_timeout(mtk_snand_time_t start_tick,
				    mtk_snand_time_t timeout_tick)
{
	ktime_t tmo = ktime_add(start_tick, timeout_tick);

	return ktime_compare(ktime_get(), tmo) > 0;
}

/* Memory helpers */
static inline void *generic_mem_alloc(struct mtk_snand_plat_dev *pdev,
				      size_t size)
{
	return devm_kzalloc(pdev->dev, size, GFP_KERNEL);
}
static inline void generic_mem_free(struct mtk_snand_plat_dev *pdev, void *ptr)
{
	devm_kfree(pdev->dev, ptr);
}

static inline void *dma_mem_alloc(struct mtk_snand_plat_dev *pdev, size_t size)
{
	return kzalloc(size, GFP_KERNEL);
}
static inline void dma_mem_free(struct mtk_snand_plat_dev *pdev, void *ptr)
{
	kfree(ptr);
}

static inline int dma_mem_map(struct mtk_snand_plat_dev *pdev, void *vaddr,
			      uintptr_t *dma_addr, size_t size, bool to_device)
{
	dma_addr_t addr;
	int ret;

	addr = dma_map_single(pdev->dev, vaddr, size,
			      to_device ? DMA_TO_DEVICE : DMA_FROM_DEVICE);
	ret = dma_mapping_error(pdev->dev, addr);
	if (ret)
		return ret;

	*dma_addr = (uintptr_t)addr;

	return 0;
}

static inline void dma_mem_unmap(struct mtk_snand_plat_dev *pdev,
				 uintptr_t dma_addr, size_t size,
				 bool to_device)
{
	dma_unmap_single(pdev->dev, dma_addr, size,
			 to_device ? DMA_TO_DEVICE : DMA_FROM_DEVICE);
}

/* Interrupt helpers */
static inline void irq_completion_done(struct mtk_snand_plat_dev *pdev)
{
	complete(&pdev->done);
}

static inline void irq_completion_init(struct mtk_snand_plat_dev *pdev)
{
	init_completion(&pdev->done);
}

static inline int irq_completion_wait(struct mtk_snand_plat_dev *pdev,
				       void __iomem *reg, uint32_t bit,
				       uint32_t timeout_us)
{
	int ret;

	ret = wait_for_completion_timeout(&pdev->done,
					  usecs_to_jiffies(timeout_us));
	if (!ret)
		return -ETIMEDOUT;

	return 0;
}

#endif /* _MTK_SNAND_OS_H_ */
