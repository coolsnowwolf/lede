// SPDX-License-Identifier: GPL-2.0
/* Platform NPU driver for Phytium NPU controller
 *
 * Copyright (C) 2023 Phytium Technology Co., Ltd.
 */
#include <linux/delay.h>
#include <linux/iopoll.h>
#include "phytium_npu_mmu.h"
#include "phytium_npu.h"
#include "phytium_npu_pd2408_reg.h"

#define POLL_PERIOD	100
#define POLL_TIMEOUT	100000

static void phytium_npu_set_bit(struct phytium_npu_dev *npudev, u32 offset, u32 bit)
{
	u32 value;

	value = REGREAD32(npudev, offset);
	value |= bit;
	REGWRITE32(npudev, offset, value);
}

void phytium_npu_config_clock(struct phytium_npu_dev *npudev, int is_enable)
{
	u32 status;
	int rc;

	if (is_enable) {
		REGWRITE64(npudev, NPU_SYS_CLK_CTRL0, SYS_CLOCK_AUTO);
		REGWRITE64(npudev, NPU_SYS_CLK_CTRL, MAIN_CLOCK_AUTO);
	} else {
		//wait for the clock being idle
		rc = readl_relaxed_poll_timeout(npudev->reg_base + NPU_SYS_CLK_STATUS,
						status, !status, POLL_PERIOD, POLL_TIMEOUT);

		if (rc == -ETIMEDOUT)
			pr_debug("%s %d, timeout sysclk: %#08x\n",
				 __func__, __LINE__, status);

		rc = readl_relaxed_poll_timeout(npudev->reg_base + NPU_MDBG_IDLE,
						status, 0xFFFF, POLL_PERIOD, POLL_TIMEOUT);

		if (rc == -ETIMEDOUT)
			pr_debug("%s %d, timeout mdbg: %#08x\n",
				 __func__, __LINE__, status);
		REGWRITE64(npudev, NPU_SYS_CLK_CTRL0, 0);
		REGWRITE64(npudev, NPU_SYS_CLK_CTRL, 0);
	}
}

static inline void phytium_npu_config_soft_reset(struct phytium_npu_dev *npudev)
{
	u32 status;
	int rc;

	REGWRITE64(npudev, NPU_SYS_RESET_CTRL, NPU_SOFT_RESET_CONFIG);

	rc = readl_relaxed_poll_timeout(npudev->reg_base + NPU_SYS_RESET_CTRL,
					status, !status, POLL_PERIOD, POLL_TIMEOUT);

	if (rc == -ETIMEDOUT)
		pr_debug("%s %d, timeout reset: %#08x\n", __func__, __LINE__, status);
}

int phytium_npu_hw_reset_self(struct phytium_npu_dev *npudev)
{
	u32 status;
	int rc;

	phytium_npu_config_soft_reset(npudev);

	rc = readl_relaxed_poll_timeout(npudev->reg_base + NPU_CH0_VHA_EVENT_STATUS,
					status, status & NPU_HW_BE_READY,
					POLL_PERIOD, POLL_TIMEOUT);

	if (rc == -ETIMEDOUT)
		pr_debug("%s %d, timeout soft reset: %#08x\n", __func__, __LINE__, status);

	phytium_npu_set_bit(npudev, NPU_CH0_VHA_EVENT_CLEAR, NPU_HW_BE_READY);
	return 0;
}

int phytium_npu_config_dma_address(struct phytium_npu_session *sess,
				   struct phytium_npu_stream *nstream)
{
	struct phytium_npu_dev *npudev = sess->npu_dev;
	struct npu_user_submit_stream *nusstream = &nstream->nustream;
	struct npu_excute_stream *nestream = &nusstream->estream;
	struct npu_mctx_map *nmmap;
	int i, address_used = 0;

	nmmap = phytium_npu_find_mmu_ctx_map(sess, nusstream->stream_fd);
	if (!nmmap) {
		pr_err("Not find stream fd %d", nusstream->stream_fd);
		return -EFAULT;
	}
	REGWRITE64(npudev, NPU_CH0_CMD_BASE_ADDRESS, nmmap->virt_addr);

	for (i = 0; i < nestream->all; i++) {
		if (!nestream->fd[i])
			continue;
		nmmap = phytium_npu_find_mmu_ctx_map(sess, nestream->fd[i]);
		if (!nmmap) {
			pr_err("Not find stream buf fd %d", nestream->fd[i]);
			return -EFAULT;
		}
		if (nusstream->stream_fd == nestream->fd[i])
			continue;
		if (i < 8)
			REGWRITE64(npudev, NPU_CH0_ADDR0 + nusstream->idx[i] * 8, nmmap->virt_addr);
		else
			REGWRITE64(npudev, NPU_CH0_ADDR8 + nusstream->idx[i] * 8, nmmap->virt_addr);
		address_used |= 0x1 << nusstream->idx[i];
		if (!(nmmap->map_type & NPU_MAP_TYPE_INFERENCE))
			address_used |= 0x1 << (nusstream->idx[i] + 16);
	}
	REGWRITE64(npudev, NPU_CH0_ADDR_USED, address_used);
	pr_debug("DEV set channel used value:%#x", address_used);
	return 0;
}

u32 phytium_npu_get_irq_status(struct phytium_npu_dev *npudev)
{
	return REGREAD32(npudev, NPU_CH0_VHA_EVENT_STATUS);
}

u32 phytium_npu_get_irq_enable_status(struct phytium_npu_dev *npudev)
{
	return REGREAD32(npudev, NPU_CH0_VHA_EVENT_ENABLE);
}

void phytium_npu_config_hl_wdt(struct phytium_npu_dev *npudev)
{
	//monitor the event for pass layer (1), layer group (2)
	REGWRITE64(npudev, NPU_SYS_HWDT_CM, NPU_HL_WDT_CYCLES);
	REGWRITE64(npudev, NPU_SYS_HWDT_CTRL, 0x1);
	REGWRITE64(npudev, NPU_SYS_HWDT, 0);
}

void phytium_npu_config_mem_wdt(struct phytium_npu_dev *npudev)
{
	//monitor the event for memory access. same to hl wdt mode
	REGWRITE64(npudev, NPU_SYS_MEM_WDT_CM, NPU_MEM_WDT_CYCLES);
	REGWRITE64(npudev, NPU_SYS_MEM_WDT_CTRL, 0x1);
	REGWRITE64(npudev, NPU_SYS_MWDT, 0);
}

void phytium_npu_config_preload(struct phytium_npu_dev *npudev)
{
	REGWRITE32(npudev, NPU_CH0_PRELOAD_CONTROL, NPU_PRELOAD_CFG);
}

void phytium_npu_config_start_inference(struct phytium_npu_dev *npudev,
					struct phytium_npu_session *sess,
							struct phytium_npu_stream *nstream)
{
	u32 ctrl, priority = 0;

	ctrl = min(2048, nstream->nustream.stream_size);
	ctrl = (ctrl / 32 - 1) << 1;
	ctrl |= NPU_HW_START_EN;
	ctrl |= (priority << 8);
	ctrl |= (sess->mmu_ctx[NPU_MMU_CONTEXT_MODULE_ID].context_id << 12);
	REGWRITE32(npudev, NPU_CH0_CONTROL, ctrl);
	pr_debug("%s start npu :%#x", __func__, ctrl);
}

void phytium_npu_config_event(struct phytium_npu_dev *npudev, u32 events, int is_enable)
{
	if (is_enable)
		REGWRITE32(npudev, NPU_CH0_VHA_EVENT_ENABLE, events);
	else
		REGWRITE32(npudev, NPU_CH0_VHA_EVENT_ENABLE, 0);
	pr_debug("enable irq :%#x", is_enable ? events : 0);
}

void phytium_npu_clear_msi_irq(struct phytium_npu_dev *npudev)
{
	REGWRITE32(npudev, 0x100000 + 0x420, 0x1);
}

void phytium_npu_clear_irq_status(struct phytium_npu_dev *npudev, u32 event)
{
	REGWRITE32(npudev, NPU_CH0_VHA_EVENT_CLEAR, event);
}

u32 phytium_npu_get_axi_err_status(struct phytium_npu_dev *npudev)
{
	return REGREAD32(npudev, NPU_SYS_ACE_STATUS);
}

void phytium_npu_output_mem_wdt_err(struct phytium_npu_dev *npudev)
{
	dev_dbg(npudev->dev, "mem wdt compare match %#x", REGREAD32(npudev, NPU_SYS_MEM_WDT_CM));
	dev_dbg(npudev->dev, "mem wdt time %#x", REGREAD32(npudev, NPU_SYS_MWDT));
	dev_dbg(npudev->dev, "mem bif outstanding %#x", REGREAD32(npudev, NPU_OUTSTANDING_READ));
}
