// SPDX-License-Identifier: GPL-2.0
/* Platform NPU driver for Phytium NPU controller
 *
 * Copyright (C) 2023 Phytium Technology Co., Ltd.
 */
#include "phytium_npu_mmu.h"
#include "phytium_npu.h"
#include <linux/kernel.h>
#include "phytium_npu_pd2408_reg.h"

int pg_size[6] = {4096, 16384, 65536, 262144, 1048576, 2097152};

static u64 mmu_get_status(u64 status, u64 mask, u64 shift)
{
	return status >> shift & mask;
}

void phytium_npu_mmu_output_status(struct phytium_npu_dev *npudev)
{
	const char levels[][5] = {"PT", "PD", "PC", "BASE"};
	u64 map_addr = 0;

	u64 s1 = REGREAD64(npudev, NPU_CH0_MMU_ERR_S1);
	u64 s2 = REGREAD64(npudev, NPU_CH0_MMU_ERR_S2);
	u32 fault = mmu_get_status(s1, MMU_FS1_FAULT_MSK, MMU_FS1_FAULT_SHIFT);

	if (!fault)
		return;

	u32 level  = mmu_get_status(s1, MMU_FS1_LEVEL_MSK, MMU_FS1_LEVEL_SHIFT);
	u32 req_id = mmu_get_status(s1, MMU_FS1_REQ_ID_MSK, MMU_FS1_REQ_ID_SHIFT);
	u32 ctxid = mmu_get_status(s1, MMU_FS1_CONTEXT_MSK, MMU_FS1_CONTEXT_SHIFT);
	u64 addr = mmu_get_status(s1, MMU_FS1_ADDRESS_MSK, MMU_FS1_ADDRESS_SHIFT);
	u32 rnw = mmu_get_status(s1, MMU_FS1_RNW_MSK, MMU_FS1_RNW_SHIFT);
	u32 type = mmu_get_status(s1, MMU_FS1_TYPE_MSK, MMU_FS1_TYPE_SHIFT);

	u32 wtb = mmu_get_status(s2, MMU_FS2_WRITEBACK_MSK, MMU_FS2_WRITEBACK_SHIFT);
	u32 unique = mmu_get_status(s2, MMU_FS2_CLEANUNIQUE_MSK, MMU_FS2_CLEANUNIQUE_SHIFT);
	u32 bank = mmu_get_status(s2, MMU_FS2_BANK_MSK, MMU_FS2_BANK_SHIFT);
	u32 tlb_entry = mmu_get_status(s2, MMU_FS2_TLB_ENTRY_MSK, MMU_FS2_TLB_ENTRY_SHIFT);
	u32 fbm = mmu_get_status(s2, MMU_FS2_FBM_FAULT_MSK, MMU_FS2_FBM_FAULT_SHIFT);
	u32 bif_id = mmu_get_status(s2, MMU_FS2_BIF_ID_MSK, MMU_FS2_BIF_ID_SHIFT);

	REGWRITE64(npudev, NPU_CH0_MMU_MAPPING_CONTEXT, ctxid);
	map_addr = REGREAD64(npudev, NPU_CH0_MMU_MAPPING_ADDR);

	pr_debug("%s: *NPU MMU fault: status1:%#llx status2:%#llx\n", __func__, s1, s2);

	pr_warn("%s: NPU MMU fault while %s @ %#llx\n", __func__,
		(rnw) ? "reading" : "writing", addr << 4);
	pr_warn("%s: level:%s Requestor:%#x Context:%#x Type:%s\n",
		__func__, levels[level], req_id, ctxid,
		(type == 0) ? "VALID" :	(type == 2) ? "READ-ONLY" :	"UNKNOWN");
	pr_warn("%s: unique:%d bif_id:%#x tlb_entry:%#x slc_bank:%#x\n", __func__,
		unique, bif_id, tlb_entry, bank);
	pr_warn("%s: current mapping@context%d:%#llx\n", __func__, ctxid, map_addr << 12);
	pr_warn("%s: fbm:%u wtb:%u\n", __func__, fbm, wtb);
}

int phytium_npu_mmu_dev_flush_tlb(struct phytium_npu_dev *npudev, int ctxid)
{
	int value = MMU_CTRL_INVAL_PC | MMU_CTRL_INVAL_PD | MMU_CTRL_INVAL_PT;

	if (ctxid)
		value |= ctxid << MMU_CTRL_CTX_SHIFT;

	REGWRITE32(npudev, NPU_CH0_MMU_CTRL_INVAL, value);
	pr_debug("%s flush tlb value %#x", __func__, value);
	return 0;
}

static inline void
phytium_npu_mmu_setup_dev_config(struct phytium_npu_dev *npudev, int ctxid, u32 pc_phys)
{
	REGWRITE64(npudev, NPU_CH0_MMU_CTRL_BS, MMU_CTRL_BYPASS_DISABLE);
	REGWRITE64(npudev, NPU_CH0_MMU_MAPPING_CONTEXT, ctxid);
	REGWRITE64(npudev, NPU_CH0_MMU_MAPPING_ADDR, pc_phys);
}

int phytium_npu_mmu_config_dev_mmu(struct phytium_npu_session *sess)
{
	struct phytium_npu_dev *npudev = sess->npu_dev;
	size_t i;

	for (i = 0; i < ARRAY_SIZE(sess->mmu_ctx); i++) {
		phytium_npu_mmu_setup_dev_config(npudev, sess->mmu_ctx[i].context_id,
						 sess->mmu_ctx[i].pc_base_phys_addr);
		pr_debug("%s ctx %ld hw context id %d  phys %x", __func__, i,
			 sess->mmu_ctx[i].context_id,
			sess->mmu_ctx[i].pc_base_phys_addr);
		phytium_npu_mmu_dev_flush_tlb(npudev, sess->mmu_ctx[i].context_id);
	}
	return 0;
}

static int phytium_npu_get_page_size_config(int page_size)
{
	int i;

	for (i = 0; i < sizeof(pg_size); i++) {
		if (page_size == pg_size[i])
			return i;
	}

	return 0;//default 4K
}

int phytium_npu_mmu_dev_init(struct phytium_npu_dev *npudev, int page_size)
{
	u64 ps_config = phytium_npu_get_page_size_config(page_size);
	u64 value = 0;

	/* range 2M - 3G */
	pr_debug("%s NPU MMU set NPU va range0:%#llx-%#llx\n", __func__,
		 NPU_VA_BASE_ADDR, NPU_VA_SIZE);
	value = ps_config << MMU_PS_CONFIG_SHIFT;
	value |= NPU_VA_BASE_ADDR >> MMU_PS_ADDR_ALIGNSHIFT;
	value |= (NPU_VA_BASE_ADDR + NPU_VA_SIZE) >> MMU_PS_END_ADDR_DELT_SHIFT;
	REGWRITE64(npudev, NPU_SYS_MMU_PSIZE_RONE, value);
	pr_debug("%s NPU_SYS_MMU_PSIZE_RONE:%#llx\n", __func__, value);
	return 0;
}
