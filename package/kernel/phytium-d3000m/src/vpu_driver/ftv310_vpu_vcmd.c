// SPDX-License-Identifier: GPL-2.0
/*
 *    ftv310 vcmd driver.
 *
 *    Copyright (C) 2024-2025, Phytium Technology Co., Ltd.
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License, version 2, as
 *    published by the Free Software Foundation.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License version 2 for more details.
 *
 *    You may obtain a copy of the GNU General Public License
 *    Version 2 at the following locations:
 *    https://opensource.org/licenses/gpl-2.0.php
 */
#include <linux/uaccess.h>
#include <linux/module.h>
#include <linux/vmalloc.h>
#include <linux/interrupt.h>
/* Our header */
#include "ftv310_vpu.h"
#include "ftv310_vpu_device.h"
#include "bidirect_list.h"
#include "vcmdswhwregisters.h"
#include <linux/delay.h>
#include "ftv310_vpu_vcmd.h"
#include "ipoffset/vcmd_offset.h"
#include "ftv310_vpu_power.h"
#include "ftv310_vpu_voltage.h"
#include "ftv310_vpu_trace.h"
#ifdef HAS_MMU
#include "ftv310_vpu_mmu.h"
#endif


#include "ftv310_vpu_fpga_mem.h"


#ifdef PCIE_EN
#include "ftv310_vpu_pcie.h"
#endif

#ifndef USE_DTB_PROBE


#define VCMD_ENCODER_REGISTER_SIZE              (ASIC_SWREG_AMOUNT * 4)
#define VCMD_DECODER_REGISTER_SIZE              (1023 * 4)
#define VCMD_IM_REGISTER_SIZE                   (ASIC_SWREG_AMOUNT * 4)
#define VCMD_JPEG_ENCODER_REGISTER_SIZE         (ASIC_SWREG_AMOUNT * 4)
#define VCMD_JPEG_DECODER_REGISTER_SIZE         (1023 * 4)
#define VCMD_READ_CMD(p, n, off, addr) \
	do { \
		*(p)++ = (OPCODE_RREG) | ((n) << 16) | (off); \
		*(p)++ = (u32)(addr); \
		*(p)++ = (u32)(((u64)(addr)) >> 32); \
		*(p)++ = 0; \
	} while (0)

#ifdef HAS_MMU
#define MMU_IO_SIZE 238*4
#endif


// Please make sure how many POOL, it related with "vcmd_core_array", the rule is:
// 0. one slice with encoder or decoder, the VCMD_POOL_CNT equal 1
// 1. one slice with encoder and decoder both, the VCMD_POOL_CNT equal 2
// 2. slice1 with decoder only, slice 2 with encoder only, the VCMD_POOL_CNT equal 2
// 3. slice1 with decoder only, slice 2 with decoder and encoder both, the VCMD_POOL_CNT equal 3
#define VCMD_POOL_CNT (2)

static u32 vcmd_isr_polling = 0;

#if (KERNEL_VERSION(2, 6, 18) > LINUX_VERSION_CODE)
static irqreturn_t ftv310_vpu_vcmd_isr(int irq, void *dev_id, struct pt_regs *regs);
#else
static irqreturn_t ftv310_vpu_vcmd_isr(int irq, void *dev_id);
#endif

#endif

//#define VCMD_DEBUG_INTERNAL
#ifdef VCMD_DEBUG_INTERNAL
static void printk_vcmd_register_debug(const void *hwregs, char *info);
#endif

static int allocate_cmdbuf(vcmd_dev_str * subsys_dev,
			   struct noncache_mem *new_cmdbuf_addr,
			   struct noncache_mem *new_status_cmdbuf_addr);
static void vcmd_set_base_addr(vcmd_dev_str *subsys_dev, unsigned long base_reg_hw, unsigned long base_ddr_hw);
static void ensure_vcmd_submodule_addr(struct vcmd_config *vcmd_cfg_p);

#ifdef PHY_FPGA_MEM
static int init_vcmd_pool_on_ftv310_vpu_fpgamem(vcmd_dev_str *subsys_dev);
static void release_vcmd_pool_on_ftv310_vpu_fpgamem(vcmd_dev_str *subsys_dev);
static unsigned long get_vcmd_buffer_pool_start_addr(vcmd_dev_str *subsys_dev);

#ifdef HAS_MMU
extern unsigned long get_mmu_pgtbl_buf_size(void);
#endif

#else
static int customized_init_vcmd_mem_pool(vcmd_dev_str *subsys_dev);
static void customized_release_vcmd_mem_pool(vcmd_dev_str *subsys_dev);
#endif

#ifdef HAS_MMU
static void get_vcmd_pool_mmuaddr(vcmd_dev_str *subsys_dev);
#endif

extern int power_enable;
extern struct ftv310_vpu_fpga_t g_fpga;

/*process manager object management*/
static struct process_manager_obj *create_process_manager_obj(void)
{
	struct process_manager_obj *process_manager_obj = NULL;

	process_manager_obj = vmalloc(sizeof(*process_manager_obj));
	if (!process_manager_obj) {
		PDEBUG("%s\n", "vmalloc for process_manager_obj fail!");
		return process_manager_obj;
	}
	memset(process_manager_obj, 0, sizeof(*process_manager_obj));
	return process_manager_obj;
}

static void
free_process_manager_obj(struct process_manager_obj *process_manager_obj)
{
	if (!process_manager_obj) {
		PDEBUG("%s\n", "free_process_manager_obj NULL");
		return;
	}
	//free current cmdbuf_obj
	vfree(process_manager_obj);
}

static bi_list_node *create_process_manager_node(void)
{
	bi_list_node *current_node = NULL;
	struct process_manager_obj *process_manager_obj = NULL;

	process_manager_obj = create_process_manager_obj();
	if (!process_manager_obj) {
		PDEBUG("%s\n", "create_process_manager_obj fail!");
		return NULL;
	}
	process_manager_obj->total_exe_time = 0;
	spin_lock_init(&process_manager_obj->spinlock);
	init_waitqueue_head(&process_manager_obj->wait_queue);
	current_node = bi_list_create_node();
	if (!current_node) {
		PDEBUG("%s\n", "bi_list_create_node fail!");
		free_process_manager_obj(process_manager_obj);
		return NULL;
	}
	current_node->data = (void *)process_manager_obj;
	return current_node;
}

static void free_process_manager_node(bi_list_node *process_node)
{
	struct process_manager_obj *process_manager_obj = NULL;

	if (!process_node) {
		PDEBUG("%s\n", "free_process_manager_node NULL");
		return;
	}
	process_manager_obj = (struct process_manager_obj *)process_node->data;
	//free struct process_manager_obj
	free_process_manager_obj(process_manager_obj);
	//free current process_manager_obj entity.
	bi_list_free_node(process_node);
}

static long release_process_node_cleanup(bi_list *list)
{
	bi_list_node *new_process_node = NULL;

	while (1) {
		new_process_node = list->head;
		if (!new_process_node)
			break;
		//remove node from list
		bi_list_remove_node(list, new_process_node);
		//remove node from list
		free_process_manager_node(new_process_node);
	}
	return 0;
}

static void create_kernel_process_manager(vcmd_dev_str *subsys_dev)
{
	bi_list_node *process_manager_node;
	struct process_manager_obj *process_manager_obj = NULL;

	process_manager_node = create_process_manager_node();
	process_manager_obj =
		(struct process_manager_obj *)process_manager_node->data;
	process_manager_obj->filp = NULL;
	bi_list_insert_node_tail(&subsys_dev->global_process_manager,
				 process_manager_node);
}

static u32
wait_process_resource_rdy(struct process_manager_obj *process_manager_obj)
{
	return process_manager_obj->total_exe_time <=
	       PROCESS_MAX_SUM_OF_IMAGE_SIZE;
}

#define FTV310_VPU_AXIFE_OFFSET 0
#define FTV310_VPU_AXIFE_IOSIZE (64 * 4)
#define AXI_REG10_SW_FRONTEND_EN           (10 * 4) //0x28
#define AXI_REG11_SW_WORK_MODE             (11 * 4) //0x2c

static void AXIFEEnable(volatile unsigned char *hwregs)
{
	if (!hwregs)
		return;

	//AXI FE pass through
	iowrite32(0x0, (void *)(hwregs + FTV310_VPU_AXIFE_OFFSET + AXI_REG11_SW_WORK_MODE));
	pr_info("AXI FE: 0x2C = 0x%x\n", ioread32((void *)(hwregs + AXI_REG11_SW_WORK_MODE)));
	iowrite32(0x2, (void *)(hwregs + FTV310_VPU_AXIFE_OFFSET + AXI_REG10_SW_FRONTEND_EN));
	pr_info("AXI FE: 0x28 = 0x%x\n", ioread32((void *)(hwregs + AXI_REG10_SW_FRONTEND_EN)));
}

static int vcmd_reserve_IO(vcmd_dev_str *subsys_dev)
{
	u32 hwid;
	int i;
	int j;
	u32 found_hw = 0;
	vcmd_core_str *subsys_core;
	u64 base_addr, axife_base_addr;
	u64 size;
	struct vcmd_config *vcmd_cfg;

	volatile u8 *axife_hwregs;

	subsys_core = subsys_dev->vcmd_core;

	for (i = 0; i < subsys_dev->subsys_num; i++) {
		vcmd_cfg = &subsys_core->vcmd_core_cfg;

		base_addr = vcmd_cfg->vcmd_base_addr;
		size = vcmd_cfg->vcmd_iosize;
		if (size != 0) {
			if (!request_mem_region(base_addr, size,
						"ftv310_vpu_vcmd")) {
				pr_info("ftv310_vpu_vcmd: failed to reserve vcmd HW regs\n");
				return -EBUSY;
			}
			subsys_core->hwregs = (u8 *)ioremap(base_addr, size);
		}

		/*read hwid and check validness and store it*/
		hwid = (u32)ioread32((void *)subsys_core->hwregs);
		subsys_core->hw_version_id = hwid;

		/* check for vcmd HW ID */
		if (((hwid >> 16) & 0xFFFF) != VCMD_HW_ID) {
			pr_info("ftv310_vpu_vcmd: HW not found at 0x%llx\n",
				(unsigned long long)subsys_core->vcmd_core_cfg
					.vcmd_base_addr);
			iounmap((void *)subsys_core->hwregs);
			release_mem_region(
				subsys_core->vcmd_core_cfg.vcmd_base_addr,
				subsys_core->vcmd_core_cfg.vcmd_iosize);
			subsys_core->hwregs = NULL;
			subsys_core = subsys_core->core_next;
			continue;
		}
		for (j = 0; j < 2; j++) {
			if (vcmd_cfg->submodule_axife_addr[j] != 0xffff) {
				axife_base_addr =
					base_addr +
					vcmd_cfg->submodule_axife_addr[j];
				pr_info("%s: axife addr is=%llx\n", __func__,
					axife_base_addr);
				size = FTV310_VPU_AXIFE_IOSIZE;
				if (!request_mem_region(axife_base_addr, size,
							"ftv310_vpu_axife")) {
					pr_info("ftv310_vpu_vcmd: failed to reserve axife HW regs\n");
					return -EBUSY;
				}
				axife_hwregs =
					(u8 *)ioremap(axife_base_addr, size);
				AXIFEEnable(axife_hwregs);
				iounmap((void *)axife_hwregs);
				release_mem_region(axife_base_addr, size);
			}
		}

		found_hw = 1;

		pr_info("ftv310_vpu_vcmd: HW at base <0x%llx> with ID <0x%08x>\n",
			(unsigned long long)
				subsys_core->vcmd_core_cfg.vcmd_base_addr,
			hwid);
		subsys_core = subsys_core->core_next;
	}

	if (found_hw == 0) {
		pr_err("ftv310_vpu_vcmd: NO ANY HW found!!\n");
		return -1;
	}

	return 0;
}

static void vcmd_release_IO(vcmd_dev_str *subsys_dev)
{
	u32 i;
	vcmd_core_str *subsys_core;

	subsys_core = subsys_dev->vcmd_core;

	for (i = 0; i < subsys_dev->subsys_num; i++) {
		if (subsys_core->hwregs) {
			iounmap((void *)subsys_core->hwregs);
			release_mem_region(
				subsys_core->vcmd_core_cfg.vcmd_base_addr,
				subsys_core->vcmd_core_cfg.vcmd_iosize);
			subsys_core->hwregs = NULL;
		}
		subsys_core = subsys_core->core_next;
	}
}

static void vcmd_reset_asic(vcmd_dev_str *subsys_dev)
{
	int i, n;
	u32 result;
	vcmd_core_str *subsys_core;

	subsys_core = subsys_dev->vcmd_core;

	for (n = 0; n < subsys_dev->subsys_num; n++) {
		if (subsys_core->hwregs) {
			//disable interrupt at first
			vcmd_write_reg((const void *)subsys_core->hwregs,
				       VCMD_REGISTER_INT_CTL_OFFSET, 0x0000);
			//reset core
			vcmd_write_reg((const void *)subsys_core->hwregs,
				       VCMD_REGISTER_CONTROL_OFFSET, 0x0004);
			//read status register
			result =
				vcmd_read_reg((const void *)subsys_core->hwregs,
					      VCMD_REGISTER_INT_STATUS_OFFSET);
			//clean status register
			vcmd_write_reg((const void *)subsys_core->hwregs,
				       VCMD_REGISTER_INT_STATUS_OFFSET, result);

			//when reset core need clear reg[3]
			vcmd_write_reg((const void *)subsys_core->hwregs,
							VCMD_REGISTER_EXE_CMDBUF_COUNT_OFFSET, 0x0000);

			for (i = VCMD_REGISTER_CONTROL_OFFSET;
			     i < subsys_core->vcmd_core_cfg.vcmd_iosize;
			     i += 4) {
				//set all register 0
				vcmd_write_reg(
					(const void *)subsys_core->hwregs, i,
					0x0000);
			}
			//enable all interrupt
			vcmd_write_reg((const void *)subsys_core->hwregs,
				       VCMD_REGISTER_INT_CTL_OFFSET,
				       0xffffffff);
			// gate all external interrupts
			vcmd_write_reg((const void *)subsys_core->hwregs,
				       VCMD_REGISTER_EXT_INT_GATE_OFFSET,
					   0xffffffff);
			iowrite32(0x0, (void __iomem *)((const void *)subsys_core->hwregs + VCMD_REGISTER_ARB_OFFSET));
			iowrite32(VCMD_ARBITER_PARAMS, (void __iomem *)((const void *)subsys_core->hwregs + VCMD_REGISTER_ARBITER_CONFIG_OFFSET));
		}
		subsys_core = subsys_core->core_next;
	}
}

static void vcmd_reset_current_asic(vcmd_core_str *subsys_core)
{
	u32 result;

	if (subsys_core->hwregs) {
		//disable interrupt at first
		vcmd_write_reg((const void *)subsys_core->hwregs,
			       VCMD_REGISTER_INT_CTL_OFFSET, 0x0000);
		//reset core
		vcmd_write_reg((const void *)subsys_core->hwregs,
			       VCMD_REGISTER_CONTROL_OFFSET, 0x0004);
		//read status register
		result = vcmd_read_reg((const void *)subsys_core->hwregs,
				       VCMD_REGISTER_INT_STATUS_OFFSET);
		//clean status register
		vcmd_write_reg((const void *)subsys_core->hwregs,
			       VCMD_REGISTER_INT_STATUS_OFFSET, result);
		//when reset core need clear reg[3]
		vcmd_write_reg((const void *)subsys_core->hwregs,
						VCMD_REGISTER_EXE_CMDBUF_COUNT_OFFSET, 0x0000);
	}
}

static void vcmd_start(vcmd_core_str *subsys_core,
		       bi_list_node *first_linked_cmdbuf_node)
{
	u64 cmdbuf_bus_addr;
	struct cmdbuf_obj *cmdbuf_obj = NULL;

	struct vcmd_dev *subsys_dev = subsys_core->parent_dev;

	if (subsys_core->working_state == WORKING_STATE_IDLE) {
		if ((first_linked_cmdbuf_node) &&
		    subsys_core->sw_cmdbuf_rdy_num) {
			cmdbuf_obj = (struct cmdbuf_obj *)
					     first_linked_cmdbuf_node->data;
#ifdef VCMD_DEBUG_INTERNAL
			printk_vcmd_register_debug(
				(const void *)subsys_core->hwregs,
				"vcmd start enters");
#endif
			//0x40
			vcmd_set_register_mirror_value(
				subsys_core->reg_mirror,
				HWIF_VCMD_AXI_CLK_GATE_DISABLE, 0);
			vcmd_set_register_mirror_value(
				subsys_core->reg_mirror,
				HWIF_VCMD_MASTER_OUT_CLK_GATE_DISABLE,
				/* this bit should be set 1 only
				 * when need to reset dec400.
				 */
				1);
			vcmd_set_register_mirror_value(
				subsys_core->reg_mirror,
				HWIF_VCMD_CORE_CLK_GATE_DISABLE, 0);
			vcmd_set_register_mirror_value(subsys_core->reg_mirror,
						       HWIF_VCMD_ABORT_MODE, 0);
			vcmd_set_register_mirror_value(subsys_core->reg_mirror,
						       HWIF_VCMD_RESET_CORE, 0);
			vcmd_set_register_mirror_value(subsys_core->reg_mirror,
						       HWIF_VCMD_RESET_ALL, 0);
			vcmd_set_register_mirror_value(subsys_core->reg_mirror,
						       HWIF_VCMD_START_TRIGGER,
						       0);
			//0x48
			if (subsys_core->hw_version_id <= HW_ID_1_0_C) {
				vcmd_set_register_mirror_value(
					subsys_core->reg_mirror,
					HWIF_VCMD_IRQ_INTCMD_EN, 0xffff);
			} else {
				vcmd_set_register_mirror_value(
					subsys_core->reg_mirror,
					HWIF_VCMD_IRQ_JMPP_EN, 1);
				vcmd_set_register_mirror_value(
					subsys_core->reg_mirror,
					HWIF_VCMD_IRQ_JMPD_EN, 1);
			}

			vcmd_set_register_mirror_value(subsys_core->reg_mirror,
						       HWIF_VCMD_IRQ_RESET_EN,
						       1);
			vcmd_set_register_mirror_value(subsys_core->reg_mirror,
						       HWIF_VCMD_IRQ_ABORT_EN,
						       1);
			vcmd_set_register_mirror_value(subsys_core->reg_mirror,
						       HWIF_VCMD_IRQ_CMDERR_EN,
						       1);
			vcmd_set_register_mirror_value(subsys_core->reg_mirror,
						       HWIF_VCMD_IRQ_TIMEOUT_EN,
						       1);
			vcmd_set_register_mirror_value(subsys_core->reg_mirror,
						       HWIF_VCMD_IRQ_BUSERR_EN,
						       1);
			vcmd_set_register_mirror_value(subsys_core->reg_mirror,
						       HWIF_VCMD_IRQ_ENDCMD_EN,
						       1);
			//0x4c
			vcmd_set_register_mirror_value(subsys_core->reg_mirror,
						       HWIF_VCMD_TIMEOUT_EN, 1);
			vcmd_set_register_mirror_value(subsys_core->reg_mirror,
						       HWIF_VCMD_TIMEOUT_CYCLES,
						       0x1dcd6500);

			cmdbuf_bus_addr =
					  (u64)(cmdbuf_obj->cmdbuf_busAddress -
						    subsys_dev->base_ddr_addr);
#ifdef HAS_MMU
			cmdbuf_bus_addr = (u64)(cmdbuf_obj->mmu_cmdbuf_bus_address);
#endif

			vcmd_set_register_mirror_value(
				subsys_core->reg_mirror,
				HWIF_VCMD_EXECUTING_CMD_ADDR,
				(u32)(cmdbuf_bus_addr));
			if (sizeof(size_t) == 8) {
				vcmd_set_register_mirror_value(
					subsys_core->reg_mirror,
					HWIF_VCMD_EXECUTING_CMD_ADDR_MSB,
					(u32)((u64)(cmdbuf_bus_addr) >>
					      32));
			} else {
				vcmd_set_register_mirror_value(
					subsys_core->reg_mirror,
					HWIF_VCMD_EXECUTING_CMD_ADDR_MSB, 0);
			}
			vcmd_set_register_mirror_value(
				subsys_core->reg_mirror,
				HWIF_VCMD_EXE_CMDBUF_LENGTH,
				(u32)((cmdbuf_obj->cmdbuf_size + 7) / 8));
			vcmd_set_register_mirror_value(
				subsys_core->reg_mirror,
				HWIF_VCMD_RDY_CMDBUF_COUNT,
				subsys_core->sw_cmdbuf_rdy_num);
			vcmd_set_register_mirror_value(subsys_core->reg_mirror,
						       HWIF_VCMD_MAX_BURST_LEN,
						       0x10);
			if (subsys_core->hw_version_id > HW_ID_1_0_C) {
				vcmd_write_register_value(
					(const void *)subsys_core->hwregs,
					subsys_core->reg_mirror,
					HWIF_VCMD_CMDBUF_EXECUTING_ID,
					(u32)cmdbuf_obj->cmdbuf_id);
			}
			vcmd_write_reg((const void *)subsys_core->hwregs, 0x40,
				       subsys_core->reg_mirror[0x40 / 4]);
			vcmd_write_reg(
				(const void *)subsys_core->hwregs, 0x44,
				vcmd_read_reg((const void *)subsys_core->hwregs,
					      0x44));
			vcmd_write_reg((const void *)subsys_core->hwregs, 0x48,
				       subsys_core->reg_mirror[0x48 / 4]);
			vcmd_write_reg((const void *)subsys_core->hwregs, 0x4c,
				       subsys_core->reg_mirror[0x4c / 4]);
			vcmd_write_reg((const void *)subsys_core->hwregs, 0x50,
				       subsys_core->reg_mirror[0x50 / 4]);
			vcmd_write_reg((const void *)subsys_core->hwregs, 0x54,
				       subsys_core->reg_mirror[0x54 / 4]);
			vcmd_write_reg((const void *)subsys_core->hwregs, 0x58,
				       subsys_core->reg_mirror[0x58 / 4]);
			vcmd_write_reg((const void *)subsys_core->hwregs, 0x5c,
				       subsys_core->reg_mirror[0x5c / 4]);
			vcmd_write_reg((const void *)subsys_core->hwregs, 0x60,
				       subsys_core->reg_mirror[0x60 / 4]);
			vcmd_write_reg((const void *)subsys_core->hwregs, 0x64,
				       0xffffffff); //not interrupt cpu

			if (vcmd_get_register_value((const void *)subsys_core->hwregs,
					 subsys_core->reg_mirror, HWIF_VCMD_START_TRIGGER) == 0)
					vcmd_write_register_value((const void *)subsys_core->hwregs,
								subsys_core->reg_mirror,
								HWIF_VCMD_EXE_CMDBUF_COUNT, 0);
			subsys_core->working_state = WORKING_STATE_WORKING;
			vcmd_set_register_mirror_value(subsys_core->reg_mirror,
						       HWIF_VCMD_START_TRIGGER,
						       1);
			vcmd_write_reg((const void *)subsys_core->hwregs, 0x40,
				       subsys_core->reg_mirror[0x40 / 4]);
			trace_vcmd_start(cmdbuf_obj->cmdbuf_id);
		}
	}
}

/*cmdbuf object management*/
static struct cmdbuf_obj *create_cmdbuf_obj(void)
{
	struct cmdbuf_obj *cmdbuf_obj = NULL;

	cmdbuf_obj = vmalloc(sizeof(*cmdbuf_obj));
	if (!cmdbuf_obj) {
		PDEBUG("%s\n", "vmalloc for cmdbuf_obj fail!");
		return cmdbuf_obj;
	}
	memset(cmdbuf_obj, 0, sizeof(*cmdbuf_obj));
	return cmdbuf_obj;
}

static void free_cmdbuf_obj(struct cmdbuf_obj *cmdbuf_obj)
{
	if (!cmdbuf_obj) {
		PDEBUG("%s\n", "remove_cmdbuf_obj NULL");
		return;
	}
	//free current cmdbuf_obj
	vfree(cmdbuf_obj);
}

static void free_cmdbuf_mem(vcmd_dev_str *subsys_dev, u16 cmdbuf_id)
{
	unsigned long flags;

	spin_lock_irqsave(&subsys_dev->vcmd_cmdbuf_alloc_lock, flags);
	subsys_dev->cmdbuf_used[cmdbuf_id] = 0;
	subsys_dev->cmdbuf_used_residual += 1;
	spin_unlock_irqrestore(&subsys_dev->vcmd_cmdbuf_alloc_lock, flags);
	wake_up_interruptible_all(&subsys_dev->vcmd_cmdbuf_memory_wait);

	trace_vcmd_free(current->pid, cmdbuf_id);
}

static bi_list_node *create_cmdbuf_node(vcmd_dev_str *subsys_dev)
{
	bi_list_node *current_node = NULL;
	struct cmdbuf_obj *cmdbuf_obj = NULL;
	struct noncache_mem new_cmdbuf_addr;
	struct noncache_mem new_status_cmdbuf_addr;

	if (wait_event_interruptible(subsys_dev->vcmd_cmdbuf_memory_wait,
				     allocate_cmdbuf(subsys_dev,
						     &new_cmdbuf_addr,
						     &new_status_cmdbuf_addr)))
		return NULL;
	cmdbuf_obj = create_cmdbuf_obj();
	if (!cmdbuf_obj) {
		PDEBUG("%s\n", "create_cmdbuf_obj fail!");
		free_cmdbuf_mem(subsys_dev, new_cmdbuf_addr.cmdbuf_id);
		return NULL;
	}
	cmdbuf_obj->cmdbuf_busAddress = new_cmdbuf_addr.busAddress;
	cmdbuf_obj->mmu_cmdbuf_bus_address = new_cmdbuf_addr.mmu_bus_address;
	cmdbuf_obj->cmdbuf_virtualAddress = new_cmdbuf_addr.virtualAddress;
	cmdbuf_obj->cmdbuf_size = new_cmdbuf_addr.size;
	cmdbuf_obj->cmdbuf_id = new_cmdbuf_addr.cmdbuf_id;
	cmdbuf_obj->status_busAddress = new_status_cmdbuf_addr.busAddress;
	cmdbuf_obj->status_virtualAddress =
		new_status_cmdbuf_addr.virtualAddress;
	cmdbuf_obj->status_size = new_status_cmdbuf_addr.size;
	current_node = bi_list_create_node();
	if (!current_node) {
		PDEBUG("%s\n", "bi_list_create_node fail!");
		free_cmdbuf_mem(subsys_dev, new_cmdbuf_addr.cmdbuf_id);
		free_cmdbuf_obj(cmdbuf_obj);
		return NULL;
	}
	current_node->data = (void *)cmdbuf_obj;
	current_node->next = NULL;
	current_node->previous = NULL;
	return current_node;
}

static void free_cmdbuf_node(vcmd_dev_str *subsys_dev,
			     bi_list_node *cmdbuf_node)
{
	struct cmdbuf_obj *cmdbuf_obj = NULL;

	if (!cmdbuf_node) {
		PDEBUG("%s\n", "remove_cmdbuf_node NULL");
		return;
	}
	cmdbuf_obj = (struct cmdbuf_obj *)cmdbuf_node->data;
	//free cmdbuf mem in pool
	free_cmdbuf_mem(subsys_dev, cmdbuf_obj->cmdbuf_id);
	//free struct cmdbuf_obj
	free_cmdbuf_obj(cmdbuf_obj);
	//free current cmdbuf_node entity.
	bi_list_free_node(cmdbuf_node);
}

//just remove, not free the node.
static bi_list_node *remove_cmdbuf_node_from_list(bi_list *list,
						  bi_list_node *cmdbuf_node)
{
	if (!cmdbuf_node) {
		PDEBUG("%s\n", "remove_cmdbuf_node_from_list  NULL");
		return NULL;
	}
	if (cmdbuf_node->next) {
		bi_list_remove_node(list, cmdbuf_node);
		return cmdbuf_node;
	}
	//the last one, should not be removed.
	return NULL;
}

static long release_cmdbuf_node(vcmd_dev_str *subsys_dev, bi_list *list,
				bi_list_node *cmdbuf_node)
{
	bi_list_node *new_cmdbuf_node = NULL;
	struct cmdbuf_obj *cmdbuf_obj = NULL;
	/*get cmdbuf object according to cmdbuf_id*/
	new_cmdbuf_node = cmdbuf_node;
	if (!new_cmdbuf_node)
		return -1;
	//remove node from list
	new_cmdbuf_node = remove_cmdbuf_node_from_list(list, new_cmdbuf_node);
	if (new_cmdbuf_node) {
		//free node
		cmdbuf_obj = (struct cmdbuf_obj *)new_cmdbuf_node->data;
		subsys_dev->global_cmdbuf_node[cmdbuf_obj->cmdbuf_id] = NULL;
		free_cmdbuf_node(subsys_dev, new_cmdbuf_node);
		return 0;
	}
	return 1;
}

//calculate executing_time of each vcmd
static u32 calculate_executing_time_after_node(bi_list_node *exe_cmdbuf_node)
{
	u32 time_run_all = 0;
	struct cmdbuf_obj *cmdbuf_obj_temp = NULL;

	while (1) {
		if (!exe_cmdbuf_node)
			break;
		cmdbuf_obj_temp = (struct cmdbuf_obj *)exe_cmdbuf_node->data;
		time_run_all += cmdbuf_obj_temp->executing_time;
		exe_cmdbuf_node = exe_cmdbuf_node->next;
	}
	return time_run_all;
}

static u32
calculate_executing_time_after_node_high_priority(bi_list_node *exe_cmdbuf_node)
{
	u32 time_run_all = 0;
	struct cmdbuf_obj *cmdbuf_obj_temp = NULL;

	if (!exe_cmdbuf_node)
		return time_run_all;
	cmdbuf_obj_temp = (struct cmdbuf_obj *)exe_cmdbuf_node->data;
	time_run_all += cmdbuf_obj_temp->executing_time;
	exe_cmdbuf_node = exe_cmdbuf_node->next;
	while (1) {
		if (!exe_cmdbuf_node)
			break;
		cmdbuf_obj_temp = (struct cmdbuf_obj *)exe_cmdbuf_node->data;
		if (cmdbuf_obj_temp->priority == CMDBUF_PRIORITY_NORMAL)
			break;
		time_run_all += cmdbuf_obj_temp->executing_time;
		exe_cmdbuf_node = exe_cmdbuf_node->next;
	}
	return time_run_all;
}

/*cmdbuf pool management*/
static int allocate_cmdbuf(vcmd_dev_str *subsys_dev,
			   struct noncache_mem *new_cmdbuf_addr,
			   struct noncache_mem *new_status_cmdbuf_addr)
{
	unsigned long flags;

	spin_lock_irqsave(&subsys_dev->vcmd_cmdbuf_alloc_lock, flags);
	if (subsys_dev->cmdbuf_used_residual == 0) {
		spin_unlock_irqrestore(&subsys_dev->vcmd_cmdbuf_alloc_lock,
				       flags);
		//no empty cmdbuf
		return 0;
	}
	//there is one cmdbuf at least
	while (1) {
		if (subsys_dev->cmdbuf_used[subsys_dev->cmdbuf_used_pos] == 0 &&
		    (subsys_dev->global_cmdbuf_node[subsys_dev->cmdbuf_used_pos] ==
		     NULL)) {
			subsys_dev->cmdbuf_used[subsys_dev->cmdbuf_used_pos] =
				1;
			subsys_dev->cmdbuf_used_residual -= 1;
			new_cmdbuf_addr->virtualAddress =
				subsys_dev->vcmd_buf_mem_pool->virtualAddress +
				subsys_dev->cmdbuf_used_pos * CMDBUF_MAX_SIZE /
					4;
			new_cmdbuf_addr->busAddress =
				subsys_dev->vcmd_buf_mem_pool->busAddress +
				subsys_dev->cmdbuf_used_pos * CMDBUF_MAX_SIZE;

			new_cmdbuf_addr->mmu_bus_address =
				subsys_dev->vcmd_buf_mem_pool->mmu_bus_address +
				subsys_dev->cmdbuf_used_pos * CMDBUF_MAX_SIZE;

			new_cmdbuf_addr->size = CMDBUF_MAX_SIZE;
			new_cmdbuf_addr->cmdbuf_id =
				subsys_dev->cmdbuf_used_pos;
			new_status_cmdbuf_addr->virtualAddress =
				subsys_dev->vcmd_status_buf_mem_pool
					->virtualAddress +
				subsys_dev->cmdbuf_used_pos * CMDBUF_MAX_SIZE /
					4;
			new_status_cmdbuf_addr->busAddress =
				subsys_dev->vcmd_status_buf_mem_pool
					->busAddress +
				subsys_dev->cmdbuf_used_pos * CMDBUF_MAX_SIZE;
			new_status_cmdbuf_addr->mmu_bus_address =
				subsys_dev->vcmd_status_buf_mem_pool
					->mmu_bus_address +
				subsys_dev->cmdbuf_used_pos * CMDBUF_MAX_SIZE;
			new_status_cmdbuf_addr->size = CMDBUF_MAX_SIZE;
			new_status_cmdbuf_addr->cmdbuf_id =
				subsys_dev->cmdbuf_used_pos;
			subsys_dev->cmdbuf_used_pos++;
			if (subsys_dev->cmdbuf_used_pos >=
			    TOTAL_DISCRETE_CMDBUF_NUM)
				subsys_dev->cmdbuf_used_pos = 0;
			spin_unlock_irqrestore(
				&subsys_dev->vcmd_cmdbuf_alloc_lock, flags);
			return 1;
		}
		subsys_dev->cmdbuf_used_pos++;
		if (subsys_dev->cmdbuf_used_pos >=
		    TOTAL_DISCRETE_CMDBUF_NUM)
			subsys_dev->cmdbuf_used_pos = 0;
	}
	return 0;
}

static bi_list_node *get_cmdbuf_node_in_list_by_addr(vcmd_dev_str *subsys_dev,
						     size_t cmdbuf_addr,
						     bi_list *list)
{
	bi_list_node *new_cmdbuf_node = NULL;
	struct cmdbuf_obj *cmdbuf_obj = NULL;

	new_cmdbuf_node = list->head;
	while (1) {
		if (!new_cmdbuf_node)
			return NULL;
		cmdbuf_obj = (struct cmdbuf_obj *)new_cmdbuf_node->data;
		if (((cmdbuf_obj->cmdbuf_busAddress -
		      subsys_dev->base_ddr_addr) <= cmdbuf_addr) &&
		    (((cmdbuf_obj->cmdbuf_busAddress -
		       subsys_dev->base_ddr_addr + cmdbuf_obj->cmdbuf_size) >
		      cmdbuf_addr))) {
			return new_cmdbuf_node;
		}
		new_cmdbuf_node = new_cmdbuf_node->next;
	}
	return NULL;
}

static int wait_abort_rdy(vcmd_core_str *subsys_core)
{
	if (!subsys_core)
		return 0;
	return subsys_core->working_state == WORKING_STATE_IDLE;
}

static int is_vcmd_supported_core(u32 core_mask,  u16 dev_pos)
{
	if (core_mask && ((core_mask >> dev_pos) & 0x01)) {
		/* current core is supported */
		return 0;
	}

	return -1;
}

static int select_vcmd(vcmd_dev_str *subsys_dev, bi_list_node *new_cmdbuf_node)
{
	struct cmdbuf_obj *cmdbuf_obj = NULL;
	bi_list_node *curr_cmdbuf_node = NULL;
	bi_list *list = NULL;
	u32 executing_time = 0xffff;
	int counter = 0;
	unsigned long flags = 0;
	u32 hw_rdy_cmdbuf_num = 0;
	size_t exe_cmdbuf_addr = 0;
	struct cmdbuf_obj *cmdbuf_obj_temp = NULL;
	u32 cmdbuf_id = 0;
	vcmd_core_str *subsys_core = NULL;
	vcmd_core_str *smallest_core = NULL;
	u16 *position;

	cmdbuf_obj = (struct cmdbuf_obj *)new_cmdbuf_node->data;

	position = &subsys_dev->vcmd_position[cmdbuf_obj->module_type];

	//there is an empty vcmd to be used
	while (1) {
		if (is_vcmd_supported_core(cmdbuf_obj->req_cores_info, *position) == 0) {
			subsys_core = subsys_dev->vcmd_manager[cmdbuf_obj->module_type]
							      [*position];
			list = &subsys_core->list_manager;
			spin_lock_irqsave(&subsys_core->spinlock, flags);
			if (!list->tail) {
				bi_list_insert_node_tail(list, new_cmdbuf_node);
				spin_unlock_irqrestore(&subsys_core->spinlock, flags);
				*position = *position + 1;
				if (*position >= subsys_dev->vcmd_type_core_num
							 [cmdbuf_obj->module_type])
					*position = 0;
				cmdbuf_obj->core_id = subsys_core->core_id;
				return 0;
			}
			spin_unlock_irqrestore(&subsys_core->spinlock, flags);
		}
		*position = *position + 1;
		if (*position >= subsys_dev->vcmd_type_core_num
					 [cmdbuf_obj->module_type])
			*position = 0;
		counter++;
		if (counter >=
		    subsys_dev->vcmd_type_core_num[cmdbuf_obj->module_type])
			break;
	}

	/*there is a vcmd which tail node -> cmdbuf_run_done == 1.
	 * It means this vcmd has nothing to do, so we select it.
	 */
	counter = 0;
	while (1) {
		if (is_vcmd_supported_core(cmdbuf_obj->req_cores_info, *position) == 0) {
			subsys_core = subsys_dev->vcmd_manager[cmdbuf_obj->module_type]
							      [*position];
			list = &subsys_core->list_manager;
			spin_lock_irqsave(&subsys_core->spinlock, flags);
			curr_cmdbuf_node = list->tail;
			if (!curr_cmdbuf_node) {
				bi_list_insert_node_tail(list, new_cmdbuf_node);
				spin_unlock_irqrestore(&subsys_core->spinlock, flags);
				*position = *position + 1;
				if (*position >= subsys_dev->vcmd_type_core_num
							 [cmdbuf_obj->module_type])
					*position = 0;
				cmdbuf_obj->core_id = subsys_core->core_id;
				return 0;
			}
			cmdbuf_obj_temp = (struct cmdbuf_obj *)curr_cmdbuf_node->data;
			if (cmdbuf_obj_temp->cmdbuf_run_done == 1) {
				bi_list_insert_node_tail(list, new_cmdbuf_node);
				spin_unlock_irqrestore(&subsys_core->spinlock, flags);
				*position = *position + 1;
				if (*position >= subsys_dev->vcmd_type_core_num
							 [cmdbuf_obj->module_type])
					*position = 0;
				cmdbuf_obj->core_id = subsys_core->core_id;
				return 0;
			}
			spin_unlock_irqrestore(&subsys_core->spinlock, flags);

		}
		*position = *position + 1;
		if (*position >= subsys_dev->vcmd_type_core_num
					 [cmdbuf_obj->module_type])
			*position = 0;
		counter++;
		if (counter >=
		    subsys_dev->vcmd_type_core_num[cmdbuf_obj->module_type])
			break;
	}
	//another case, tail = executing node, and
	//vcmd=pend state (finish but not generate interrupt)
	counter = 0;
	while (1) {
		if (is_vcmd_supported_core(cmdbuf_obj->req_cores_info, *position) == 0) {
			subsys_core = subsys_dev->vcmd_manager[cmdbuf_obj->module_type]
							      [*position];
			list = &subsys_core->list_manager;
			//read executing cmdbuf address
			if (subsys_core->hw_version_id <= HW_ID_1_0_C) {
				hw_rdy_cmdbuf_num = vcmd_get_register_value(
					(const void *)subsys_core->hwregs,
					subsys_core->reg_mirror,
					HWIF_VCMD_EXE_CMDBUF_COUNT);
			} else {
				hw_rdy_cmdbuf_num =
					*(subsys_core->vcmd_reg_mem_virtualAddress +
					  VCMD_EXE_CMDBUF_COUNT);
				if (hw_rdy_cmdbuf_num != subsys_core->sw_cmdbuf_rdy_num)
					hw_rdy_cmdbuf_num += 1;
			}
			spin_lock_irqsave(&subsys_core->spinlock, flags);
			curr_cmdbuf_node = list->tail;
			if (!curr_cmdbuf_node) {
				bi_list_insert_node_tail(list, new_cmdbuf_node);
				spin_unlock_irqrestore(&subsys_core->spinlock, flags);
				*position = *position + 1;
				if (*position >= subsys_dev->vcmd_type_core_num
							 [cmdbuf_obj->module_type])
					*position = 0;
				cmdbuf_obj->core_id = subsys_core->core_id;
				return 0;
			}

			if (subsys_core->sw_cmdbuf_rdy_num == hw_rdy_cmdbuf_num) {
				bi_list_insert_node_tail(list, new_cmdbuf_node);
				spin_unlock_irqrestore(&subsys_core->spinlock, flags);
				*position = *position + 1;
				if (*position >= subsys_dev->vcmd_type_core_num
							 [cmdbuf_obj->module_type])
					*position = 0;
				cmdbuf_obj->core_id = subsys_core->core_id;
				return 0;
			}
			spin_unlock_irqrestore(&subsys_core->spinlock, flags);
		}
		*position = *position + 1;
		if (*position >= subsys_dev->vcmd_type_core_num
					 [cmdbuf_obj->module_type])
			*position = 0;
		counter++;
		if (counter >=
		    subsys_dev->vcmd_type_core_num[cmdbuf_obj->module_type])
			break;
	}
	//there is no idle vcmd,if low priority,calculate
	// exe time, select the least one.
	// or if high priority, calculate the exe time,
	// select the least one and abort it.
	if (cmdbuf_obj->priority == CMDBUF_PRIORITY_NORMAL) {
		counter = 0;
		//calculate total execute time of all devices
		while (1) {
			if (is_vcmd_supported_core(cmdbuf_obj->req_cores_info, *position) == 0) {
				subsys_core =
					subsys_dev->vcmd_manager[cmdbuf_obj->module_type]
								[*position];
				//read executing cmdbuf address
				if (subsys_core->hw_version_id <= HW_ID_1_0_C) {
					exe_cmdbuf_addr = VCMDGetAddrRegisterValue(
						(const void *)subsys_core->hwregs,
						subsys_core->reg_mirror,
						HWIF_VCMD_EXECUTING_CMD_ADDR);
					list = &subsys_core->list_manager;
					spin_lock_irqsave(&subsys_core->spinlock,
							  flags);
					//get the executing cmdbuf node.
					curr_cmdbuf_node =
						get_cmdbuf_node_in_list_by_addr(
							subsys_dev, exe_cmdbuf_addr,
							list);

					//calculate total execute time of this device
					subsys_core->total_exe_time =
						calculate_executing_time_after_node(
							curr_cmdbuf_node);
					spin_unlock_irqrestore(&subsys_core->spinlock,
							       flags);
				} else {
					cmdbuf_id = *(
						subsys_core->vcmd_reg_mem_virtualAddress +
						EXECUTING_CMDBUF_ID_ADDR + 1);
					spin_lock_irqsave(&subsys_core->spinlock,
							  flags);
					if (cmdbuf_id >= TOTAL_DISCRETE_CMDBUF_NUM ||
					    cmdbuf_id == 0) {
						pr_err("cmdbuf_id greater than the ceiling !!\n");
						spin_unlock_irqrestore(
							&subsys_core->spinlock, flags);
						return -1;
					}
					//get the executing cmdbuf node.
					curr_cmdbuf_node =
						subsys_dev
							->global_cmdbuf_node[cmdbuf_id];
					if (!curr_cmdbuf_node) {
						list = &subsys_core->list_manager;
						curr_cmdbuf_node = list->head;
						while (1) {
							if (!curr_cmdbuf_node)
								break;
							cmdbuf_obj_temp =
								(struct cmdbuf_obj *)
									curr_cmdbuf_node
										->data;
							if (cmdbuf_obj_temp
								    ->cmdbuf_data_linked &&
							    cmdbuf_obj_temp->cmdbuf_run_done ==
								    0)
								break;
							curr_cmdbuf_node =
								curr_cmdbuf_node->next;
						}
					}

					//calculate total execute time of this device
					subsys_core->total_exe_time =
						calculate_executing_time_after_node(
							curr_cmdbuf_node);
					spin_unlock_irqrestore(&subsys_core->spinlock,
						       flags);
				}
			}
			*position = *position + 1;
			if (*position >= subsys_dev->vcmd_type_core_num
						 [cmdbuf_obj->module_type])
				*position = 0;
			counter++;
			if (counter >= subsys_dev->vcmd_type_core_num
					       [cmdbuf_obj->module_type])
				break;
		}

		//find the device with the least total_exe_time.
		counter = 0;
		executing_time = 0xffffffff;
		while (1) {
			if (is_vcmd_supported_core(cmdbuf_obj->req_cores_info, *position) == 0) {
				subsys_core =
					subsys_dev->vcmd_manager[cmdbuf_obj->module_type]
								[*position];
				if (subsys_core->total_exe_time <= executing_time) {
					executing_time = subsys_core->total_exe_time;
					smallest_core = subsys_core;
				}
			}
			*position = *position + 1;
			if (*position >= subsys_dev->vcmd_type_core_num
						 [cmdbuf_obj->module_type])
				*position = 0;
			counter++;
			if (counter >= subsys_dev->vcmd_type_core_num
					       [cmdbuf_obj->module_type])
				break;
		}
		//insert list
		list = &smallest_core->list_manager;
		spin_lock_irqsave(&smallest_core->spinlock, flags);
		bi_list_insert_node_tail(list, new_cmdbuf_node);
		spin_unlock_irqrestore(&smallest_core->spinlock, flags);
		cmdbuf_obj->core_id = smallest_core->core_id;
		return 0;
	} else {
		//CMDBUF_PRIORITY_HIGH
		counter = 0;
		//calculate total execute time of all devices
		while (1) {
			if (is_vcmd_supported_core(cmdbuf_obj->req_cores_info, *position) == 0) {
				subsys_core =
					subsys_dev->vcmd_manager[cmdbuf_obj->module_type]
								[*position];
				if (subsys_core->hw_version_id <= HW_ID_1_0_C) {
					//read executing cmdbuf address
					exe_cmdbuf_addr = VCMDGetAddrRegisterValue(
						(const void *)subsys_core->hwregs,
						subsys_core->reg_mirror,
						HWIF_VCMD_EXECUTING_CMD_ADDR);
					list = &subsys_core->list_manager;
					spin_lock_irqsave(&subsys_core->spinlock,
							  flags);
					//get the executing cmdbuf node.
					curr_cmdbuf_node =
						get_cmdbuf_node_in_list_by_addr(
							subsys_dev, exe_cmdbuf_addr,
							list);

					//calculate total execute time of this device
					subsys_core->total_exe_time =
						calculate_executing_time_after_node_high_priority(
							curr_cmdbuf_node);
					spin_unlock_irqrestore(&subsys_core->spinlock,
							       flags);
				} else {
					cmdbuf_id = *(
						subsys_core->vcmd_reg_mem_virtualAddress +
						EXECUTING_CMDBUF_ID_ADDR);
					spin_lock_irqsave(&subsys_core->spinlock,
							  flags);
					if (cmdbuf_id >= TOTAL_DISCRETE_CMDBUF_NUM ||
					    cmdbuf_id == 0) {
						pr_err("cmdbuf_id greater than the ceiling !!\n");
						spin_unlock_irqrestore(
							&subsys_core->spinlock, flags);
						return -1;
					}
					//get the executing cmdbuf node.
					curr_cmdbuf_node =
						subsys_dev
							->global_cmdbuf_node[cmdbuf_id];
					if (!curr_cmdbuf_node) {
						list = &subsys_core->list_manager;
						curr_cmdbuf_node = list->head;
						while (1) {
							if (!curr_cmdbuf_node)
								break;
							cmdbuf_obj_temp =
								(struct cmdbuf_obj *)
									curr_cmdbuf_node
										->data;
							if (cmdbuf_obj_temp
								    ->cmdbuf_data_linked &&
							    cmdbuf_obj_temp->cmdbuf_run_done ==
								    0)
								break;
							curr_cmdbuf_node =
								curr_cmdbuf_node->next;
						}
					}

					//calculate total execute time of this device
					subsys_core->total_exe_time =
						calculate_executing_time_after_node(
							curr_cmdbuf_node);
					spin_unlock_irqrestore(&subsys_core->spinlock,
							       flags);
				}
			}
			*position = *position + 1;
			if (*position >= subsys_dev->vcmd_type_core_num
						 [cmdbuf_obj->module_type])
				*position = 0;
			counter++;
			if (counter >= subsys_dev->vcmd_type_core_num
					       [cmdbuf_obj->module_type])
				break;
		}

		//find the smallest device.
		counter = 0;
		executing_time = 0xffffffff;
		while (1) {
			if (is_vcmd_supported_core(cmdbuf_obj->req_cores_info, *position) == 0) {
				subsys_core =
					subsys_dev->vcmd_manager[cmdbuf_obj->module_type]
								[*position];
				if (subsys_core->total_exe_time <= executing_time) {
					executing_time = subsys_core->total_exe_time;
					smallest_core = subsys_core;
				}
			}
			*position = *position + 1;
			if (*position >= subsys_dev->vcmd_type_core_num
						 [cmdbuf_obj->module_type])
				*position = 0;
			counter++;
			if (counter >= subsys_dev->vcmd_type_core_num
					       [cmdbuf_obj->module_type])
				break;
		}
		//abort the vcmd and wait
		vcmd_write_register_value((const void *)smallest_core->hwregs,
					  smallest_core->reg_mirror,
					  HWIF_VCMD_START_TRIGGER, 0);
		if (subsys_dev->type == FTV310_VPU_ENCODER) {
			subsys_dev->software_triger_abort = 1;
			if (wait_event_interruptible(
				    smallest_core->wait_abort_queue,
				    wait_abort_rdy(subsys_core))) {
				subsys_dev->software_triger_abort = 0;
				return -ERESTARTSYS;
			}
			subsys_dev->software_triger_abort = 0;
		} else {
			if (wait_event_interruptible(
				    smallest_core->wait_abort_queue,
				    wait_abort_rdy(subsys_core))) {
				return -ERESTARTSYS;
			}
		}
		/* need to select inserting position again
		 *because hw maybe have run to the next node.
		 *CMDBUF_PRIORITY_HIGH
		 */
		spin_lock_irqsave(&smallest_core->spinlock, flags);
		curr_cmdbuf_node = smallest_core->list_manager.head;
		while (1) {
			//if list is empty or tail,insert to tail
			if (!curr_cmdbuf_node)
				break;
			cmdbuf_obj_temp =
				(struct cmdbuf_obj *)curr_cmdbuf_node->data;
			/*if find the first node which priority is normal,
			 *insert node prior to  the node
			 */
			if ((cmdbuf_obj_temp->priority ==
			     CMDBUF_PRIORITY_NORMAL) &&
			    (cmdbuf_obj_temp->cmdbuf_run_done == 0))
				break;
			curr_cmdbuf_node = curr_cmdbuf_node->next;
		}
		bi_list_insert_node_before(list, curr_cmdbuf_node,
					   new_cmdbuf_node);
		cmdbuf_obj->core_id = smallest_core->core_id;
		spin_unlock_irqrestore(&smallest_core->spinlock, flags);

		return 0;
	}
	return 0;
}

static void vcmd_link_cmdbuf(vcmd_core_str *subsys_core,
			     bi_list_node *last_linked_cmdbuf_node)
{
	bi_list_node *new_cmdbuf_node = NULL;
	bi_list_node *next_cmdbuf_node = NULL;
	struct cmdbuf_obj *cmdbuf_obj = NULL;
	struct cmdbuf_obj *next_cmdbuf_obj = NULL;
	u32 *jmp_addr = NULL;
	u32 operation_code;
	u64 next_cmd_buf_addr;

	struct vcmd_dev *subsys_dev = subsys_core->parent_dev;

	new_cmdbuf_node = last_linked_cmdbuf_node;
	//for the first cmdbuf.
	if (new_cmdbuf_node) {
		cmdbuf_obj = (struct cmdbuf_obj *)new_cmdbuf_node->data;
		if (cmdbuf_obj->cmdbuf_data_linked == 0) {
			subsys_core->sw_cmdbuf_rdy_num++;
			cmdbuf_obj->cmdbuf_data_linked = 1;
			subsys_core->duration_without_int = 0;
			if (cmdbuf_obj->has_end_cmdbuf == 0) {
				if (cmdbuf_obj->no_normal_int_cmdbuf == 1) {
					subsys_core->duration_without_int =
						cmdbuf_obj->executing_time;
					//maybe nop is modified, so write back.
					if (subsys_core->duration_without_int >=
					    INT_MIN_SUM_OF_IMAGE_SIZE) {
						jmp_addr =
							cmdbuf_obj
								->cmdbuf_virtualAddress +
							(cmdbuf_obj->cmdbuf_size /
							 4);
						operation_code =
							*(jmp_addr - 4);
						operation_code = JMP_IE_1 |
								 operation_code;
						*(jmp_addr - 4) =
							operation_code;
						subsys_core
							->duration_without_int =
							0;
					}
				}
			}
		}
	}
	while (1) {
		if (!new_cmdbuf_node)
			break;
		if (!new_cmdbuf_node->next)
			break;
		next_cmdbuf_node = new_cmdbuf_node->next;
		cmdbuf_obj = (struct cmdbuf_obj *)new_cmdbuf_node->data;
		next_cmdbuf_obj = (struct cmdbuf_obj *)next_cmdbuf_node->data;
		if (cmdbuf_obj->has_end_cmdbuf == 0 && !next_cmdbuf_obj->cmdbuf_run_done) {
			//need to link, current cmdbuf link to next cmdbuf
			jmp_addr = cmdbuf_obj->cmdbuf_virtualAddress +
				   (cmdbuf_obj->cmdbuf_size / 4);
			if (subsys_core->hw_version_id > HW_ID_1_0_C) {
				//set next cmdbuf id
				*(jmp_addr - 1) = next_cmdbuf_obj->cmdbuf_id;
			}
			next_cmd_buf_addr = (u64)(next_cmdbuf_obj->cmdbuf_busAddress -
					subsys_dev->base_ddr_addr);

#ifdef HAS_MMU
			next_cmd_buf_addr = (u64)(next_cmdbuf_obj->mmu_cmdbuf_bus_address);
#endif

			if (sizeof(size_t) == 8) {
				*(jmp_addr - 2) = (u32)(
					(u64)(next_cmd_buf_addr) >>	32);
			} else {
				*(jmp_addr - 2) = 0;
			}
			*(jmp_addr - 3) = (u32)(next_cmd_buf_addr);
			operation_code = *(jmp_addr - 4);
			operation_code >>= 16;
			operation_code <<= 16;
			*(jmp_addr - 4) =
				(u32)(operation_code | JMP_RDY_1 |
				      ((next_cmdbuf_obj->cmdbuf_size + 7) / 8));
			next_cmdbuf_obj->cmdbuf_data_linked = 1;
			subsys_core->sw_cmdbuf_rdy_num++;
			//modify nop code of next cmdbuf
			if (next_cmdbuf_obj->has_end_cmdbuf == 0) {
				if (next_cmdbuf_obj->no_normal_int_cmdbuf ==
				    1) {
					subsys_core->duration_without_int +=
						next_cmdbuf_obj->executing_time;

					//maybe we see the modified nop before abort,
					//so need to write back.
					if (subsys_core->duration_without_int >=
					    INT_MIN_SUM_OF_IMAGE_SIZE) {
						jmp_addr =
							next_cmdbuf_obj
								->cmdbuf_virtualAddress +
							(next_cmdbuf_obj
								 ->cmdbuf_size /
							 4);
						operation_code =
							*(jmp_addr - 4);
						operation_code = JMP_IE_1 |
								 operation_code;
						*(jmp_addr - 4) =
							operation_code;
						subsys_core
							->duration_without_int =
							0;
					}
				}
			} else {
				subsys_core->duration_without_int = 0;
			}
#ifdef VCMD_DEBUG_INTERNAL
		{
		u32 i;

		pr_info("vcmd link, last cmdbuf content\n");
		for (i = cmdbuf_obj->cmdbuf_size / 4 - 8; i < cmdbuf_obj->cmdbuf_size / 4; i++)
			pr_info("current linked cmdbuf data %d =0x%x\n",
				i, *(cmdbuf_obj->cmdbuf_virtualAddress + i));
		}
#endif
		}
		new_cmdbuf_node = new_cmdbuf_node->next;
	}
}

/* delink all the cmd buffers from the cmdbuf in front of last_linked_cmdbuf_node
 * to head of the list. All the cmd bufs marked as X will be delinked.
 * E.g.,
 * L->L->...->L->L->X->        ...        ->X
 * ^             ^                          ^
 * head          last_linked_cmdbuf_node    tail
 */
static void vcmd_delink_cmdbuf(vcmd_core_str *subsys_core,
			       bi_list_node *last_linked_cmdbuf_node)
{
	bi_list_node *new_cmdbuf_node = NULL;
	struct cmdbuf_obj *cmdbuf_obj = NULL;

	new_cmdbuf_node = last_linked_cmdbuf_node;
	while (1) {
		if (!new_cmdbuf_node)
			break;
		cmdbuf_obj = (struct cmdbuf_obj *)new_cmdbuf_node->data;
		if (cmdbuf_obj->cmdbuf_data_linked)
			cmdbuf_obj->cmdbuf_data_linked = 0;
		else
			break;
		new_cmdbuf_node = new_cmdbuf_node->next;
	}
	subsys_core->sw_cmdbuf_rdy_num = 0;
}

static struct bi_list_node *get_process_manager_node(struct file *filp,
	vcmd_dev_str *subsys_dev)
{
	unsigned long flags;
	bi_list_node *process_manager_node = NULL;
	struct process_manager_obj *pmo = NULL;

	spin_lock_irqsave(&subsys_dev->vcmd_process_manager_lock, flags);
	process_manager_node = subsys_dev->global_process_manager.head;
	while (1) {
		if (!process_manager_node) {
			pr_err("ftv310_vpu_vcmd: ERROR process_manager_node, current progress is %s!!\n", current->comm);
			spin_unlock_irqrestore(
				&subsys_dev->vcmd_process_manager_lock, flags);
			return NULL;
		}
		pmo = (struct process_manager_obj *)process_manager_node->data;
		PDEBUG("reserve process loop: node %p for filp %p\n",
						(void *)process_manager_node, (void *)pmo->filp);
		if (filp == pmo->filp)
			break;
		process_manager_node = process_manager_node->next;
	}
	spin_unlock_irqrestore(&subsys_dev->vcmd_process_manager_lock, flags);

	return process_manager_node;
}

static struct process_manager_obj *get_process_manager_obj(struct file *filp,
	vcmd_dev_str *subsys_dev)
{
	bi_list_node *process_manager_node = NULL;

	process_manager_node = get_process_manager_node(filp, subsys_dev);
	if (!process_manager_node) {
		pr_err("ftv310_vpu_vcmd: ERROR process_manager_node, current progress is %s!!\n", current->comm);
		return NULL;
	}

	return (struct process_manager_obj *)process_manager_node->data;
}

static long reserve_cmdbuf(struct file *filp, vcmd_dev_str *subsys_dev,
			   struct exchange_parameter *input_para)
{
	bi_list_node *new_cmdbuf_node = NULL;
	struct cmdbuf_obj *cmdbuf_obj = NULL;
	struct process_manager_obj *pmo = NULL;
	unsigned long flags;

	input_para->cmdbuf_id = 0;
	if (input_para->cmdbuf_size > CMDBUF_MAX_SIZE)
		return -1;
	pmo = get_process_manager_obj(filp, subsys_dev);
	if (pmo == NULL) {
		pr_err("ftv310_vpu_vcmd: ERROR process_manager_node, current progress is %s!!\n", current->comm);
		return -1;
	}
	spin_lock_irqsave(&pmo->spinlock, flags);
	pmo->total_exe_time += input_para->interrupt_ctrl;
	spin_unlock_irqrestore(&pmo->spinlock, flags);

	if (wait_event_interruptible(
		    pmo->wait_queue,
		    wait_process_resource_rdy(pmo)))
		return -1;

	if (down_interruptible(&subsys_dev->vcmd_reserve_cmdbuf_sem[input_para->module_type])) 
		return -ERESTARTSYS;

	new_cmdbuf_node = create_cmdbuf_node(subsys_dev);
	if (!new_cmdbuf_node)
		return -1;

	cmdbuf_obj = (struct cmdbuf_obj *)new_cmdbuf_node->data;
	cmdbuf_obj->module_type = input_para->module_type;
	cmdbuf_obj->priority = input_para->priority;
	cmdbuf_obj->executing_time = input_para->interrupt_ctrl;
	cmdbuf_obj->cmdbuf_size = CMDBUF_MAX_SIZE;
	input_para->cmdbuf_size = CMDBUF_MAX_SIZE;
	cmdbuf_obj->filp = filp;
	cmdbuf_obj->req_cores_info = input_para->req_cores_info;

	cmdbuf_obj->process_manager_obj = pmo;

	input_para->cmdbuf_id = cmdbuf_obj->cmdbuf_id;
	subsys_dev->global_cmdbuf_node[input_para->cmdbuf_id] = new_cmdbuf_node;
	up(&subsys_dev->vcmd_reserve_cmdbuf_sem[input_para->module_type]);
	spin_lock_irqsave(&pmo->spinlock, flags);
	pmo->cmdbuf_nodes[input_para->cmdbuf_id] = new_cmdbuf_node;
	PDEBUG("%s, process obj %p for filp %p, new_cmdbuf_node %p\n", __func__,
										(void *)pmo, (void *)pmo->filp, (void *)new_cmdbuf_node);
	spin_unlock_irqrestore(&pmo->spinlock, flags);
	trace_vcmd_reserve(current->pid, cmdbuf_obj->cmdbuf_id);
	return 0;
}

static long release_cmdbuf_notlinked(struct file *filp, vcmd_dev_str *subsys_dev,
			   u16 cmdbuf_id)
{
	struct cmdbuf_obj *cmdbuf_obj = NULL;
	struct process_manager_obj *pmo = NULL;
	bi_list_node *new_cmdbuf_node = NULL;
	u32 module_type;
	unsigned long flags;
	/*get cmdbuf object according to cmdbuf_id*/
	new_cmdbuf_node = subsys_dev->global_cmdbuf_node[cmdbuf_id];
	if (!new_cmdbuf_node) {
		//should not happen
		pr_err("ftv310_vpu_vcmd: ERROR cmdbuf_id !!\n");
		return -1;
	}
	cmdbuf_obj = (struct cmdbuf_obj *)new_cmdbuf_node->data;
	if (cmdbuf_obj->filp != filp) {
		pr_info("ftv310_vpu_vcmd: WARN cmdbuf_id %d not belong to this progress!!\n", cmdbuf_id);
		return -1;
	}
	module_type = cmdbuf_obj->module_type;
	cmdbuf_obj->cmdbuf_need_remove = 1;

	do {
		//remove current node
		cmdbuf_obj = (struct cmdbuf_obj *)new_cmdbuf_node->data;
		if (cmdbuf_obj->cmdbuf_need_remove == 1) {
			if (new_cmdbuf_node) {
				//free node
				subsys_dev->global_cmdbuf_node[cmdbuf_obj->cmdbuf_id] = NULL;
				if (cmdbuf_obj->process_manager_obj) {
					spin_lock_irqsave(&cmdbuf_obj->process_manager_obj->spinlock,
														flags);
					cmdbuf_obj->process_manager_obj->total_exe_time -=
									cmdbuf_obj->executing_time;
					spin_unlock_irqrestore(&cmdbuf_obj->process_manager_obj->spinlock,
																	flags);
					wake_up_interruptible_all(&cmdbuf_obj->process_manager_obj->wait_queue);
				}
				pmo = cmdbuf_obj->process_manager_obj;
				if (pmo) {
					spin_lock_irqsave(&pmo->spinlock, flags);
					pmo->total_exe_time -= cmdbuf_obj->executing_time;
					free_cmdbuf_node(subsys_dev, new_cmdbuf_node);
					spin_unlock_irqrestore(&pmo->spinlock, flags);
					wake_up_interruptible_all(&pmo->wait_queue);
				} else {
					free_cmdbuf_node(subsys_dev, new_cmdbuf_node);
				}
			}
		}
	} while (0);

	return 0;
}


static long release_cmdbuf(struct file *filp, vcmd_dev_str *subsys_dev,
			   u16 cmdbuf_id)
{
	struct cmdbuf_obj *cmdbuf_obj = NULL;
	bi_list_node *last_cmdbuf_node = NULL;
	bi_list_node *new_cmdbuf_node = NULL;
	bi_list *list = NULL;
	u32 module_type;
	struct process_manager_obj *pmo = NULL;
	unsigned long flags;
	vcmd_core_str *subsys_core;

	/*get cmdbuf object according to cmdbuf_id*/
	new_cmdbuf_node = subsys_dev->global_cmdbuf_node[cmdbuf_id];
	if (!new_cmdbuf_node) {
		//should not happen
		pr_err("ftv310_vpu_vcmd: ERROR cmdbuf_id !!\n");
		return -1;
	}
	cmdbuf_obj = (struct cmdbuf_obj *)new_cmdbuf_node->data;
	if (cmdbuf_obj->filp != filp) {
		pr_err("ftv310_vpu_vcmd: ERROR cmdbuf_id with mismatch progress filp !!\n");
		return -1;
	}
	module_type = cmdbuf_obj->module_type;
	pmo = cmdbuf_obj->process_manager_obj;
	spin_lock_irqsave(&pmo->spinlock, flags);
	pmo->cmdbuf_nodes[cmdbuf_id] = NULL;
	spin_unlock_irqrestore(&pmo->spinlock, flags);
	//TODO
	if (down_interruptible(
		    &subsys_dev->vcmd_reserve_cmdbuf_sem[module_type]))
		return -ERESTARTSYS;

	subsys_core = get_dev_of_core(subsys_dev, cmdbuf_obj->core_id);

	list = &subsys_core->list_manager;
	cmdbuf_obj->cmdbuf_need_remove = 1;
	last_cmdbuf_node = new_cmdbuf_node->previous;
	while (1) {
		//remove current node
		cmdbuf_obj = (struct cmdbuf_obj *)new_cmdbuf_node->data;
		if (cmdbuf_obj->cmdbuf_need_remove == 1) {
			new_cmdbuf_node = remove_cmdbuf_node_from_list(
				list, new_cmdbuf_node);

			if (new_cmdbuf_node) {
				cmdbuf_obj = (struct cmdbuf_obj *)new_cmdbuf_node->data;
				//free node
				subsys_dev->global_cmdbuf_node
					[cmdbuf_obj->cmdbuf_id] = NULL;
				pmo = cmdbuf_obj->process_manager_obj;
				if (pmo) {
					spin_lock_irqsave(&pmo->spinlock, flags);
					pmo->total_exe_time -= cmdbuf_obj->executing_time;
					free_cmdbuf_node(subsys_dev, new_cmdbuf_node);
					spin_unlock_irqrestore(&pmo->spinlock, flags);
					wake_up_interruptible_all(&pmo->wait_queue);
				} else {
					free_cmdbuf_node(subsys_dev, new_cmdbuf_node);
				}
			}
		}
		if (!last_cmdbuf_node)
			break;
		new_cmdbuf_node = last_cmdbuf_node;
		last_cmdbuf_node = new_cmdbuf_node->previous;
	}
	up(&subsys_dev->vcmd_reserve_cmdbuf_sem[module_type]);
	return 0;
}

static long release_cmdbuf_node_cleanup(vcmd_dev_str *subsys_dev, bi_list *list)
{
	bi_list_node *new_cmdbuf_node = NULL;
	struct cmdbuf_obj *cmdbuf_obj = NULL;

	while (1) {
		new_cmdbuf_node = list->head;
		if (!new_cmdbuf_node)
			return 0;
		//remove node from list
		bi_list_remove_node(list, new_cmdbuf_node);
		//free node
		cmdbuf_obj = (struct cmdbuf_obj *)new_cmdbuf_node->data;
		subsys_dev->global_cmdbuf_node[cmdbuf_obj->cmdbuf_id] = NULL;
		free_cmdbuf_node(subsys_dev, new_cmdbuf_node);
	}
	return 0;
}

static bi_list_node *find_last_linked_cmdbuf(bi_list_node *current_node)
{
	bi_list_node *new_cmdbuf_node = current_node;
	bi_list_node *last_cmdbuf_node;
	struct cmdbuf_obj *cmdbuf_obj = NULL;

	if (!current_node)
		return NULL;
	last_cmdbuf_node = new_cmdbuf_node;
	new_cmdbuf_node = new_cmdbuf_node->previous;
	while (1) {
		if (!new_cmdbuf_node)
			return last_cmdbuf_node;
		cmdbuf_obj = (struct cmdbuf_obj *)new_cmdbuf_node->data;
		if (cmdbuf_obj->cmdbuf_data_linked)
			return new_cmdbuf_node;
		last_cmdbuf_node = new_cmdbuf_node;
		new_cmdbuf_node = new_cmdbuf_node->previous;
	}
	return NULL;
}

static long link_and_run_cmdbuf(struct file *filp, vcmd_dev_str *subsys_dev,
				struct exchange_parameter *input_para)
{
	struct cmdbuf_obj *cmdbuf_obj = NULL;
	bi_list_node *new_cmdbuf_node = NULL;
	bi_list_node *last_cmdbuf_node;
	u32 *jmp_addr = NULL;
	u32 op_code;
	u32 temp_opcode;
	u32 record_last_cmdbuf_rdy_num;
	unsigned long flags;
	int return_value;
	u16 cmdbuf_id = input_para->cmdbuf_id;
	vcmd_core_str *subsys_core = NULL;

	new_cmdbuf_node = subsys_dev->global_cmdbuf_node[cmdbuf_id];
	if (!new_cmdbuf_node) {
		//should not happen
		pr_err("ftv310_vpu_vcmd: ERROR cmdbuf_id !!\n");
		return -1;
	}
	cmdbuf_obj = (struct cmdbuf_obj *)new_cmdbuf_node->data;
	if (cmdbuf_obj->filp != filp) {
		//should not happen
		pr_err("ftv310_vpu_vcmd: ERROR cmdbuf_id with mismatch progress filp !!\n");
		return -1;
	}
	cmdbuf_obj->cmdbuf_data_loaded = 1;
	cmdbuf_obj->cmdbuf_size = input_para->cmdbuf_size;
	cmdbuf_obj->waited = 0;

	//test nop and end opcode, then assign value.
	cmdbuf_obj->has_end_cmdbuf = 0; //0: has jmp opcode,1 has end code
	cmdbuf_obj->no_normal_int_cmdbuf = 0; //0: interrupt when JMP,1 not interrupt when JMP
	jmp_addr = cmdbuf_obj->cmdbuf_virtualAddress + (cmdbuf_obj->cmdbuf_size / 4);
	temp_opcode = *(jmp_addr - 4);
	op_code = temp_opcode;
	op_code >>= 27;
	op_code <<= 27;
	/*we can't identify END opcode or JMP opcode,
	 *so we don't support END opcode in control sw and driver.
	 */
	if (op_code == OPCODE_JMP) {
		op_code = temp_opcode;
		op_code &= 0x02000000;
		if (op_code == JMP_IE_1)
			cmdbuf_obj->no_normal_int_cmdbuf = 0;
		else
			cmdbuf_obj->no_normal_int_cmdbuf = 1;
	} else {
		pr_info("didn't support op_code %d in %s, re -1\n", op_code, __func__);
		return -1;
	}

	if (down_interruptible(&subsys_dev->vcmd_reserve_cmdbuf_sem[cmdbuf_obj->module_type]))
		return -ERESTARTSYS;

	return_value = select_vcmd(subsys_dev, new_cmdbuf_node);

	if (return_value) {
		up(&subsys_dev->vcmd_reserve_cmdbuf_sem[cmdbuf_obj->module_type]);
		return return_value;
	}

	subsys_core = get_dev_of_core(subsys_dev, cmdbuf_obj->core_id);

	input_para->core_id = cmdbuf_obj->core_id;
	//set ddr address for vcmd registers copy.
	if (subsys_core->hw_version_id > HW_ID_1_0_C) {
		//This logic depend UMD call CWLCollectReadRegData to read vcmd register
		//read vcmd executing register into ddr memory.
		//now core id is got and output ddr address of
		//vcmd register can be filled in.
		//each core has its own fixed output ddr address
		//of vcmd registers.
		u32 *vcmd_rreg_data = NULL;
		u32 jmp_cmd_size = 4;
		u32 rreg_cmd_size = 4;

		vcmd_rreg_data = cmdbuf_obj->cmdbuf_virtualAddress;

		if (sizeof(size_t) == 8) {
			*(vcmd_rreg_data + 2) = (u32)(
				(u64)(subsys_core->vcmd_reg_mem_busAddress +
				      (EXECUTING_CMDBUF_ID_ADDR + 1) * 4) >>
				32);
		} else {
			*(vcmd_rreg_data + 2) = 0;
		}
		*(vcmd_rreg_data + 1) = (u32)((subsys_core->vcmd_reg_mem_busAddress +
					 (EXECUTING_CMDBUF_ID_ADDR + 1) * 4));
#ifdef HAS_MMU
		*(vcmd_rreg_data + 1) = (u32)((subsys_core->mmu_vcmd_reg_mem_bus_address +
					(EXECUTING_CMDBUF_ID_ADDR + 1) * 4));
		*(vcmd_rreg_data + 2) = 0;
#endif

		vcmd_rreg_data = cmdbuf_obj->cmdbuf_virtualAddress +
			(cmdbuf_obj->cmdbuf_size / 4) - jmp_cmd_size - rreg_cmd_size;

		//read vcmd all registers into ddr memory.
		//now core id is got and output ddr address
		//of vcmd registers can be filled in.
		//each core has its own fixed output ddr
		//address of vcmd registers.
		if (sizeof(size_t) == 8) {
			*(vcmd_rreg_data + 2) = (u32)(
				(u64)subsys_core->vcmd_reg_mem_busAddress >>
				32);
		} else {
			*(vcmd_rreg_data + 2) = 0;
		}
		*(vcmd_rreg_data + 1) = (u32)(subsys_core->vcmd_reg_mem_busAddress);

#ifdef HAS_MMU
		*(vcmd_rreg_data + 1) = (u32)(subsys_core->mmu_vcmd_reg_mem_bus_address);
		*(vcmd_rreg_data + 2) = 0;
#endif
	}
	//start to link and/or run
	spin_lock_irqsave(&subsys_core->spinlock, flags);
	last_cmdbuf_node = find_last_linked_cmdbuf(new_cmdbuf_node);
	record_last_cmdbuf_rdy_num = subsys_core->sw_cmdbuf_rdy_num;
	vcmd_link_cmdbuf(subsys_core, last_cmdbuf_node);
	trace_vcmd_link(cmdbuf_obj->cmdbuf_id);
	if (subsys_core->working_state == WORKING_STATE_IDLE) {
		//run
		while (last_cmdbuf_node &&
			  ((struct cmdbuf_obj *)last_cmdbuf_node->data)->cmdbuf_run_done)
			last_cmdbuf_node = last_cmdbuf_node->next;

		if (last_cmdbuf_node && last_cmdbuf_node->data) {
			PDEBUG("vcmd start for cmdbuf id %d, cmdbuf_run_done = %d\n",
			((struct cmdbuf_obj *)last_cmdbuf_node->data)->cmdbuf_id,
			((struct cmdbuf_obj *)last_cmdbuf_node->data)->cmdbuf_run_done);
		}
		vcmd_start(subsys_core, last_cmdbuf_node);
	} else {
		//just update cmdbuf ready number
		PDEBUG("subsys_core state = %d, cmdbuf rdy num updated %d -> %d\n",
			subsys_core->working_state, record_last_cmdbuf_rdy_num,
			subsys_core->sw_cmdbuf_rdy_num);
		if (record_last_cmdbuf_rdy_num !=
		    subsys_core->sw_cmdbuf_rdy_num)
			vcmd_write_register_value(
				(const void *)subsys_core->hwregs,
				subsys_core->reg_mirror,
				HWIF_VCMD_RDY_CMDBUF_COUNT,
				subsys_core->sw_cmdbuf_rdy_num);
	}
	spin_unlock_irqrestore(&subsys_core->spinlock, flags);
	up(&subsys_dev->vcmd_reserve_cmdbuf_sem[cmdbuf_obj->module_type]);

	return 0;
}

/******************************************************************************/
static int check_cmdbuf_irq(vcmd_core_str *subsys_core,
			    struct cmdbuf_obj *cmdbuf_obj, u32 *irq_status_ret)
{
	int rdy = 0;
	unsigned long flags;

	spin_lock_irqsave(&subsys_core->spinlock, flags);
	if (cmdbuf_obj->cmdbuf_run_done) {
		rdy = 1;
		//need to decide how to assign this variable
		*irq_status_ret = cmdbuf_obj->executing_status;
	}
	spin_unlock_irqrestore(&subsys_core->spinlock, flags);
	return rdy;
}

static int check_mc_cmdbuf_irq(vcmd_dev_str *subsys_dev,
			       struct process_manager_obj *pmo,
			       u32 *irq_status_ret)
{
	int k;
	unsigned long flags;
	bi_list_node *new_cmdbuf_node = NULL;
	vcmd_core_str *subsys_core;
	struct cmdbuf_obj *cmdbuf_obj = NULL;

	spin_lock_irqsave(&pmo->spinlock, flags);
	for (k = 0; k < TOTAL_DISCRETE_CMDBUF_NUM; k++) {
		new_cmdbuf_node = pmo->cmdbuf_nodes[k];
		if (!new_cmdbuf_node)
			continue;

		cmdbuf_obj = (struct cmdbuf_obj *)new_cmdbuf_node->data;
		if (!cmdbuf_obj)
			continue;

		subsys_core = get_dev_of_core(subsys_dev, cmdbuf_obj->core_id);

		if (!subsys_core)
			continue;
		if (subsys_core->vcmd_core_cfg.vcmd_irq == -1) {
			if (!cmdbuf_obj->cmdbuf_run_done) {
				ftv310_vpu_vcmd_isr(subsys_core->core_id,
								subsys_core);
			}
		}

		if (check_cmdbuf_irq(subsys_core, cmdbuf_obj, irq_status_ret) ==
		    1) {
			/* Return cmdbuf_id when ANY_CMDBUF_ID is used. */
			if (!cmdbuf_obj->waited) {
				*irq_status_ret = cmdbuf_obj->cmdbuf_id;
				cmdbuf_obj->waited = 1;
				spin_unlock_irqrestore(&pmo->spinlock, flags);
				return 1;
			}
		}
	}
	spin_unlock_irqrestore(&pmo->spinlock, flags);

	return 0;
}

static unsigned int wait_cmdbuf_ready(struct file *filp,
				      vcmd_dev_str *subsys_dev, u16 cmdbuf_id,
				      u32 *irq_status_ret)
{
	struct cmdbuf_obj *cmdbuf_obj = NULL;
	bi_list_node *new_cmdbuf_node = NULL;
	vcmd_core_str *subsys_core;
	struct process_manager_obj *pmo = NULL;

	PDEBUG("%s, %d\n", __func__, __LINE__);
	if (cmdbuf_id != ANY_CMDBUF_ID) {
		new_cmdbuf_node = subsys_dev->global_cmdbuf_node[cmdbuf_id];
		if (!new_cmdbuf_node) {
			//should not happen
			pr_err("ftv310_vpu_vcmd: ERROR cmdbuf_id !!\n");
			return -1;
		}
		cmdbuf_obj = (struct cmdbuf_obj *)new_cmdbuf_node->data;
		if (cmdbuf_obj->filp != filp) {
			//should not happen
			pr_err("ftv310_vpu_vcmd: ERROR cmdbuf_id with mismatch progess filp!!\n");
			return -1;
		}
		subsys_core = get_dev_of_core(subsys_dev, cmdbuf_obj->core_id);

		if (subsys_core->vcmd_core_cfg.vcmd_irq == -1) {
			while (!cmdbuf_obj->cmdbuf_run_done) {
				ftv310_vpu_vcmd_isr(subsys_core->core_id,
								subsys_core);
				msleep(20);
			}
		}

		if (wait_event_interruptible(subsys_core->wait_queue, check_cmdbuf_irq(subsys_core, cmdbuf_obj, irq_status_ret))) {
			PDEBUG("vcmd_wait_queue_0 interrupted\n");
			return -ERESTARTSYS;
		}
	} else {
		pmo = get_process_manager_obj(filp, subsys_dev);
		if (pmo == NULL) {
			pr_err("ftv310_vpu_vcmd: ERROR process_manager_node, current progress is %s!!\n", current->comm);
			return -ERESTARTSYS;
		}

		subsys_core = get_dev_of_core(subsys_dev, 0); //get whether use irq info based core 0
		if (subsys_core->vcmd_core_cfg.vcmd_irq == -1) { //simulate polling
			while (!check_mc_cmdbuf_irq(subsys_dev, pmo,
						irq_status_ret)) {
				msleep(20);
			}
			return 0;
		} else {
			if (check_mc_cmdbuf_irq(subsys_dev, pmo,
						irq_status_ret))
				return 0;

			if (wait_event_interruptible(
					subsys_dev->mc_wait_queue,
					check_mc_cmdbuf_irq(subsys_dev, pmo,
							irq_status_ret))) {
				PDEBUG("multicore wait queue interrupted\n");
				return -ERESTARTSYS;
			}
		}
	}
	return 0;
}

#if (KERNEL_VERSION(2, 6, 18) > LINUX_VERSION_CODE)
static irqreturn_t ftv310_vpu_vcmd_isr(int irq, void *dev_id, struct pt_regs *regs)
#else
static irqreturn_t ftv310_vpu_vcmd_isr(int irq, void *dev_id)
#endif
{
	unsigned int handled = 0;
	vcmd_core_str *subsys_core = (vcmd_core_str *)dev_id;
	u32 irq_status = 0;
	unsigned long flags;
	bi_list_node *new_cmdbuf_node = NULL;
	bi_list_node *base_cmdbuf_node = NULL;
	struct cmdbuf_obj *cmdbuf_obj = NULL;
	size_t exe_cmdbuf_busAddress;
	u32 cmdbuf_processed_num = 0;
	u32 cmdbuf_id = 0;
	vcmd_dev_str *subsys_dev;

	subsys_dev = subsys_core->parent_dev;

	/*If core is not reserved by any user,
	 *but irq is received, just clean it
	 */
	spin_lock_irqsave(&subsys_core->spinlock, flags);
	if (!subsys_core->list_manager.head) {
		PDEBUG("ftv310_vpu_vcmd_isr:received IRQ but core has nothing to do.\n");
		irq_status = vcmd_read_reg((const void *)subsys_core->hwregs,
					   VCMD_REGISTER_INT_STATUS_OFFSET);
		vcmd_write_reg((const void *)subsys_core->hwregs,
			       VCMD_REGISTER_INT_STATUS_OFFSET, irq_status);
		spin_unlock_irqrestore(&subsys_core->spinlock, flags);
		return IRQ_HANDLED;
	}

	if (subsys_core->vcmd_core_cfg.vcmd_irq == -1)
		PDEBUG("ftv310_vpu_vcmd_isr:received fake IRQ!\n");
	else
		PDEBUG("ftv310_vpu_vcmd_isr:received IRQ!\n");

	irq_status = vcmd_read_reg((const void *)subsys_core->hwregs,
				   VCMD_REGISTER_INT_STATUS_OFFSET);

	if (!irq_status) {
		spin_unlock_irqrestore(&subsys_core->spinlock, flags);
		return IRQ_HANDLED;
	}
#ifdef VCMD_DEBUG_INTERNAL
	{
		u32 i, fordebug;

		for (i = 0; i < ASIC_VCMD_SWREG_AMOUNT; i++) {
			fordebug =
				vcmd_read_reg((const void *)subsys_core->hwregs, i * 4);
			pr_info("vcmd register %d:0x%x\n", i,
				fordebug);
		}
	}
#endif
	PDEBUG("irq_status of %d is:%x\n", subsys_core->core_id, irq_status);
	vcmd_write_reg((const void *)subsys_core->hwregs,
		       VCMD_REGISTER_INT_STATUS_OFFSET, irq_status);
	subsys_core->reg_mirror[VCMD_REGISTER_INT_STATUS_OFFSET / 4] =
		irq_status;

	if (subsys_core->hw_version_id > HW_ID_1_0_C && (irq_status & 0x3f)) {
		//if error,read from register directly.
		cmdbuf_id = vcmd_get_register_value(
			(const void *)subsys_core->hwregs,
			subsys_core->reg_mirror, HWIF_VCMD_CMDBUF_EXECUTING_ID);
		if (cmdbuf_id >= TOTAL_DISCRETE_CMDBUF_NUM) {
			pr_err("ftv310_vpu_vcmd_isr error cmdbuf_id greater than the ceiling !!\n");
			spin_unlock_irqrestore(&subsys_core->spinlock, flags);
			return IRQ_HANDLED;
		}
	} else if (subsys_core->hw_version_id > HW_ID_1_0_C) {
		//read cmdbuf id from ddr
#ifdef VCMD_DEBUG_INTERNAL
		{
			u32 i, fordebug;

			pr_info("ddr vcmd register phy_addr=0x%x\n",
				subsys_core->vcmd_reg_mem_busAddress);
			pr_info("ddr vcmd register virt_addr=0x%x\n",
				subsys_core->vcmd_reg_mem_virtualAddress);
			for (i = 0; i < ASIC_VCMD_SWREG_AMOUNT; i++) {
				fordebug = *(subsys_core->vcmd_reg_mem_virtualAddress +
						 i);
				pr_info("ddr vcmd register %d:0x%x\n",
					i, fordebug);
			}
		}
#endif
		cmdbuf_id = *(subsys_core->vcmd_reg_mem_virtualAddress +
			      EXECUTING_CMDBUF_ID_ADDR);
		if (cmdbuf_id >= TOTAL_DISCRETE_CMDBUF_NUM) {
			pr_err("ftv310_vpu_vcmd_isr error cmdbuf_id greater than the ceiling !!\n");
			spin_unlock_irqrestore(&subsys_core->spinlock, flags);
			return IRQ_HANDLED;
		}
	}

    trace_vcmd_isr_event(cmdbuf_id, irq_status, subsys_core->sw_cmdbuf_rdy_num, 
            *(subsys_core->vcmd_reg_mem_virtualAddress +VCMD_EXE_CMDBUF_COUNT));

	if (vcmd_get_register_mirror_value(subsys_core->reg_mirror,
					   HWIF_VCMD_IRQ_RESET)) {
		//reset error,all cmdbuf that is not  done will be run again.
		new_cmdbuf_node = subsys_core->list_manager.head;
		subsys_core->working_state = WORKING_STATE_IDLE;
		//find the first run_done=0
		while (1) {
			if (!new_cmdbuf_node)
				break;
			cmdbuf_obj = (struct cmdbuf_obj *)new_cmdbuf_node->data;
			if (cmdbuf_obj->cmdbuf_run_done == 0)
				break;
			new_cmdbuf_node = new_cmdbuf_node->next;
		}
		base_cmdbuf_node = new_cmdbuf_node;
		vcmd_delink_cmdbuf(subsys_core, base_cmdbuf_node);
		vcmd_link_cmdbuf(subsys_core, base_cmdbuf_node);
		if (subsys_core->sw_cmdbuf_rdy_num != 0) {
			//restart new command
			vcmd_start(subsys_core, base_cmdbuf_node);
		}
		handled++;
		spin_unlock_irqrestore(&subsys_core->spinlock, flags);
		return IRQ_HANDLED;
	}
	if (vcmd_get_register_mirror_value(subsys_core->reg_mirror,
					   HWIF_VCMD_IRQ_ABORT)) {
		//abort error,don't need to reset
		new_cmdbuf_node = subsys_core->list_manager.head;
		subsys_core->working_state = WORKING_STATE_IDLE;

		if (subsys_core->hw_version_id > HW_ID_1_0_C) {
			new_cmdbuf_node =
				subsys_dev->global_cmdbuf_node[cmdbuf_id];
			if (!new_cmdbuf_node) {
				pr_err("ftv310_vpu_vcmd_isr error cmdbuf_id !!\n");
				spin_unlock_irqrestore(&subsys_core->spinlock,
						       flags);
				return IRQ_HANDLED;
			}
		} else {
			exe_cmdbuf_busAddress = VCMDGetAddrRegisterValue(
				(const void *)subsys_core->hwregs,
				subsys_core->reg_mirror,
				HWIF_VCMD_EXECUTING_CMD_ADDR);
			//find the cmderror cmdbuf
			while (1) {
				if (!new_cmdbuf_node) {
					spin_unlock_irqrestore(
						&subsys_core->spinlock, flags);
					return IRQ_HANDLED;
				}
				cmdbuf_obj = (struct cmdbuf_obj *)
						     new_cmdbuf_node->data;
				if ((((cmdbuf_obj->cmdbuf_busAddress -
				       subsys_dev->base_ddr_addr) <=
				      exe_cmdbuf_busAddress) &&
				     (((cmdbuf_obj->cmdbuf_busAddress -
					subsys_dev->base_ddr_addr +
					cmdbuf_obj->cmdbuf_size) >
				       exe_cmdbuf_busAddress))) &&
				    (cmdbuf_obj->cmdbuf_run_done == 0))
					break;
				new_cmdbuf_node = new_cmdbuf_node->next;
			}
		}
		base_cmdbuf_node = new_cmdbuf_node;
		// this cmdbuf and cmdbufs prior to itself, run_done = 1
		while (1) {
			if (!new_cmdbuf_node)
				break;
			cmdbuf_obj = (struct cmdbuf_obj *)new_cmdbuf_node->data;
			if (cmdbuf_obj->cmdbuf_run_done == 0) {
				cmdbuf_obj->cmdbuf_run_done = 1;
				cmdbuf_obj->executing_status =
					CMDBUF_EXE_STATUS_OK;
				cmdbuf_processed_num++;
			} else {
				break;
			}
			new_cmdbuf_node = new_cmdbuf_node->previous;
		}
		base_cmdbuf_node = base_cmdbuf_node->next;
		vcmd_delink_cmdbuf(subsys_core, base_cmdbuf_node);
		if (subsys_dev->type == FTV310_VPU_ENCODER) {
			if (subsys_dev->software_triger_abort == 0) {
				//for QCFE
				vcmd_link_cmdbuf(subsys_core, base_cmdbuf_node);
				if (subsys_core->sw_cmdbuf_rdy_num != 0) {
					//restart new command
					vcmd_start(subsys_core,
						   base_cmdbuf_node);
				}
			}
		}
		spin_unlock_irqrestore(&subsys_core->spinlock, flags);
		if (cmdbuf_processed_num)
			wake_up_interruptible_all(&subsys_core->wait_queue);
		//to let high priority cmdbuf be inserted
		wake_up_interruptible_all(&subsys_core->wait_abort_queue);
		wake_up_interruptible_all(&subsys_dev->mc_wait_queue);
		handled++;
		return IRQ_HANDLED;
	}
	if (vcmd_get_register_mirror_value(subsys_core->reg_mirror,
					   HWIF_VCMD_IRQ_BUSERR)) {
		//bus error ,don't need to reset, where to record status?
		new_cmdbuf_node = subsys_core->list_manager.head;
		subsys_core->working_state = WORKING_STATE_IDLE;
		if (subsys_core->hw_version_id > HW_ID_1_0_C) {
			new_cmdbuf_node =
				subsys_dev->global_cmdbuf_node[cmdbuf_id];
			if (!new_cmdbuf_node) {
				pr_err("ftv310_vpu_vcmd_isr error cmdbuf_id !!\n");
				spin_unlock_irqrestore(&subsys_core->spinlock,
						       flags);
				return IRQ_HANDLED;
			}
		} else {
			exe_cmdbuf_busAddress = VCMDGetAddrRegisterValue(
				(const void *)subsys_core->hwregs,
				subsys_core->reg_mirror,
				HWIF_VCMD_EXECUTING_CMD_ADDR);
			//find the buserr cmdbuf
			while (1) {
				if (!new_cmdbuf_node) {
					spin_unlock_irqrestore(
						&subsys_core->spinlock, flags);
					return IRQ_HANDLED;
				}
				cmdbuf_obj = (struct cmdbuf_obj *)
						     new_cmdbuf_node->data;
				if ((((cmdbuf_obj->cmdbuf_busAddress -
				       subsys_dev->base_ddr_addr) <=
				      exe_cmdbuf_busAddress) &&
				     (((cmdbuf_obj->cmdbuf_busAddress -
					subsys_dev->base_ddr_addr +
					cmdbuf_obj->cmdbuf_size) >
				       exe_cmdbuf_busAddress))) &&
				    (cmdbuf_obj->cmdbuf_run_done == 0))
					break;
				new_cmdbuf_node = new_cmdbuf_node->next;
			}
		}
		base_cmdbuf_node = new_cmdbuf_node;
		// this cmdbuf and cmdbufs prior to itself, run_done = 1
		while (1) {
			if (!new_cmdbuf_node)
				break;
			cmdbuf_obj = (struct cmdbuf_obj *)new_cmdbuf_node->data;
			if ((cmdbuf_obj->cmdbuf_run_done == 0)) {
				cmdbuf_obj->cmdbuf_run_done = 1;
				cmdbuf_obj->executing_status =
					CMDBUF_EXE_STATUS_OK;
				cmdbuf_processed_num++;
			} else {
				break;
			}
			new_cmdbuf_node = new_cmdbuf_node->previous;
		}
		new_cmdbuf_node = base_cmdbuf_node;
		if (new_cmdbuf_node) {
			cmdbuf_obj = (struct cmdbuf_obj *)new_cmdbuf_node->data;
			cmdbuf_obj->executing_status = CMDBUF_EXE_STATUS_BUSERR;
		}
		base_cmdbuf_node = base_cmdbuf_node->next;
		vcmd_delink_cmdbuf(subsys_core, base_cmdbuf_node);
		vcmd_link_cmdbuf(subsys_core, base_cmdbuf_node);
		if (subsys_core->sw_cmdbuf_rdy_num != 0) {
			//restart new command
			vcmd_start(subsys_core, base_cmdbuf_node);
		}
		spin_unlock_irqrestore(&subsys_core->spinlock, flags);
		if (cmdbuf_processed_num)
			wake_up_interruptible_all(&subsys_core->wait_queue);
		handled++;
		wake_up_interruptible_all(&subsys_dev->mc_wait_queue);
		return IRQ_HANDLED;
	}
	if (vcmd_get_register_mirror_value(subsys_core->reg_mirror,
					   HWIF_VCMD_IRQ_TIMEOUT)) {
		//time out,need to reset
		new_cmdbuf_node = subsys_core->list_manager.head;
		subsys_core->working_state = WORKING_STATE_IDLE;
		if (subsys_core->hw_version_id > HW_ID_1_0_C) {
			new_cmdbuf_node =
				subsys_dev->global_cmdbuf_node[cmdbuf_id];
			if (!new_cmdbuf_node) {
				pr_err("ftv310_vpu_vcmd_isr error cmdbuf_id !!\n");
				spin_unlock_irqrestore(&subsys_core->spinlock,
						       flags);
				return IRQ_HANDLED;
			}
		} else {
			exe_cmdbuf_busAddress = VCMDGetAddrRegisterValue(
				(const void *)subsys_core->hwregs,
				subsys_core->reg_mirror,
				HWIF_VCMD_EXECUTING_CMD_ADDR);
			//find the timeout cmdbuf
			while (1) {
				if (!new_cmdbuf_node) {
					spin_unlock_irqrestore(
						&subsys_core->spinlock, flags);
					return IRQ_HANDLED;
				}
				cmdbuf_obj = (struct cmdbuf_obj *)
						     new_cmdbuf_node->data;
				if ((((cmdbuf_obj->cmdbuf_busAddress -
				       subsys_dev->base_ddr_addr) <=
				      exe_cmdbuf_busAddress) &&
				     (((cmdbuf_obj->cmdbuf_busAddress -
					subsys_dev->base_ddr_addr +
					cmdbuf_obj->cmdbuf_size) >
				       exe_cmdbuf_busAddress))) &&
				    (cmdbuf_obj->cmdbuf_run_done == 0))
					break;
				new_cmdbuf_node = new_cmdbuf_node->next;
			}
		}
		base_cmdbuf_node = new_cmdbuf_node;
		new_cmdbuf_node = new_cmdbuf_node->previous;
		// this cmdbuf and cmdbufs prior to itself, run_done = 1
		while (1) {
			if (!new_cmdbuf_node)
				break;
			cmdbuf_obj = (struct cmdbuf_obj *)new_cmdbuf_node->data;
			if (cmdbuf_obj->cmdbuf_run_done == 0) {
				cmdbuf_obj->cmdbuf_run_done = 1;
				cmdbuf_obj->executing_status =
					CMDBUF_EXE_STATUS_OK;
				cmdbuf_processed_num++;
			} else {
				break;
			}
			new_cmdbuf_node = new_cmdbuf_node->previous;
		}
		vcmd_delink_cmdbuf(subsys_core, base_cmdbuf_node);
		vcmd_link_cmdbuf(subsys_core, base_cmdbuf_node);
		if (subsys_core->sw_cmdbuf_rdy_num != 0) {
			//reset
			vcmd_reset_current_asic(subsys_core);
			//restart new command
			vcmd_start(subsys_core, base_cmdbuf_node);
		}
		spin_unlock_irqrestore(&subsys_core->spinlock, flags);
		if (cmdbuf_processed_num)
			wake_up_interruptible_all(&subsys_core->wait_queue);
		handled++;
		wake_up_interruptible_all(&subsys_dev->mc_wait_queue);
		return IRQ_HANDLED;
	}
	if (vcmd_get_register_mirror_value(subsys_core->reg_mirror,
					   HWIF_VCMD_IRQ_CMDERR)) {
		//command error,don't need to reset
		new_cmdbuf_node = subsys_core->list_manager.head;
		subsys_core->working_state = WORKING_STATE_IDLE;
		if (subsys_core->hw_version_id > HW_ID_1_0_C) {
			new_cmdbuf_node =
				subsys_dev->global_cmdbuf_node[cmdbuf_id];
			if (!new_cmdbuf_node) {
				pr_err("ftv310_vpu_vcmd_isr error cmdbuf_id !!\n");
				spin_unlock_irqrestore(&subsys_core->spinlock,
						       flags);
				return IRQ_HANDLED;
			}
		} else {
			exe_cmdbuf_busAddress = VCMDGetAddrRegisterValue(
				(const void *)subsys_core->hwregs,
				subsys_core->reg_mirror,
				HWIF_VCMD_EXECUTING_CMD_ADDR);
			//find the cmderror cmdbuf
			while (1) {
				if (!new_cmdbuf_node) {
					spin_unlock_irqrestore(
						&subsys_core->spinlock, flags);
					return IRQ_HANDLED;
				}
				cmdbuf_obj = (struct cmdbuf_obj *)
						     new_cmdbuf_node->data;
				if ((((cmdbuf_obj->cmdbuf_busAddress -
				       subsys_dev->base_ddr_addr) <=
				      exe_cmdbuf_busAddress) &&
				     (((cmdbuf_obj->cmdbuf_busAddress -
					subsys_dev->base_ddr_addr +
					cmdbuf_obj->cmdbuf_size) >
				       exe_cmdbuf_busAddress))) &&
				    (cmdbuf_obj->cmdbuf_run_done == 0))
					break;
				new_cmdbuf_node = new_cmdbuf_node->next;
			}
		}
		base_cmdbuf_node = new_cmdbuf_node;
		// this cmdbuf and cmdbufs prior to itself, run_done = 1
		while (1) {
			if (!new_cmdbuf_node)
				break;
			cmdbuf_obj = (struct cmdbuf_obj *)new_cmdbuf_node->data;
			if (cmdbuf_obj->cmdbuf_run_done == 0) {
				cmdbuf_obj->cmdbuf_run_done = 1;
				cmdbuf_obj->executing_status =
					CMDBUF_EXE_STATUS_OK;
				cmdbuf_processed_num++;
			} else {
				break;
			}
			new_cmdbuf_node = new_cmdbuf_node->previous;
		}
		new_cmdbuf_node = base_cmdbuf_node;
		if (new_cmdbuf_node) {
			cmdbuf_obj = (struct cmdbuf_obj *)new_cmdbuf_node->data;
			cmdbuf_obj->executing_status =
				CMDBUF_EXE_STATUS_CMDERR; //cmderr
		}
		base_cmdbuf_node = base_cmdbuf_node->next;
		vcmd_delink_cmdbuf(subsys_core, base_cmdbuf_node);
		vcmd_link_cmdbuf(subsys_core, base_cmdbuf_node);
		if (subsys_core->sw_cmdbuf_rdy_num != 0) {
			//restart new command
			vcmd_start(subsys_core, base_cmdbuf_node);
		}
		spin_unlock_irqrestore(&subsys_core->spinlock, flags);
		if (cmdbuf_processed_num)
			wake_up_interruptible_all(&subsys_core->wait_queue);
		handled++;
		wake_up_interruptible_all(&subsys_dev->mc_wait_queue);
		return IRQ_HANDLED;
	}

	if (vcmd_get_register_mirror_value(subsys_core->reg_mirror,
					   HWIF_VCMD_IRQ_ENDCMD)) {
		//end command interrupt
		new_cmdbuf_node = subsys_core->list_manager.head;
		subsys_core->working_state = WORKING_STATE_IDLE;
		if (subsys_core->hw_version_id > HW_ID_1_0_C) {
			new_cmdbuf_node =
				subsys_dev->global_cmdbuf_node[cmdbuf_id];
			if (!new_cmdbuf_node) {
				pr_err("ftv310_vpu_vcmd_isr error cmdbuf_id !!\n");
				spin_unlock_irqrestore(&subsys_core->spinlock,
						       flags);
				return IRQ_HANDLED;
			}
		} else {
			//find the end cmdbuf
			while (1) {
				if (!new_cmdbuf_node) {
					spin_unlock_irqrestore(
						&subsys_core->spinlock, flags);
					return IRQ_HANDLED;
				}
				cmdbuf_obj = (struct cmdbuf_obj *)
						     new_cmdbuf_node->data;
				if ((cmdbuf_obj->has_end_cmdbuf == 1) &&
				    (cmdbuf_obj->cmdbuf_run_done == 0))
					break;
				new_cmdbuf_node = new_cmdbuf_node->next;
			}
		}
		base_cmdbuf_node = new_cmdbuf_node;
		// this cmdbuf and cmdbufs prior to itself, run_done = 1
		while (1) {
			if (!new_cmdbuf_node)
				break;
			cmdbuf_obj = (struct cmdbuf_obj *)new_cmdbuf_node->data;
			if (cmdbuf_obj->cmdbuf_run_done == 0) {
				cmdbuf_obj->cmdbuf_run_done = 1;
				cmdbuf_obj->executing_status =
					CMDBUF_EXE_STATUS_OK;
				cmdbuf_processed_num++;
			} else {
				break;
			}
			new_cmdbuf_node = new_cmdbuf_node->previous;
		}
		base_cmdbuf_node = base_cmdbuf_node->next;
		vcmd_delink_cmdbuf(subsys_core, base_cmdbuf_node);
		vcmd_link_cmdbuf(subsys_core, base_cmdbuf_node);
		if (subsys_core->sw_cmdbuf_rdy_num != 0) {
			//restart new command
			vcmd_start(subsys_core, base_cmdbuf_node);
		}
		spin_unlock_irqrestore(&subsys_core->spinlock, flags);
		if (cmdbuf_processed_num)
			wake_up_interruptible_all(&subsys_core->wait_queue);
		handled++;
		wake_up_interruptible_all(&subsys_dev->mc_wait_queue);
		return IRQ_HANDLED;
	}

	if (subsys_core->hw_version_id <= HW_ID_1_0_C)
		cmdbuf_id = vcmd_get_register_mirror_value(
			subsys_core->reg_mirror, HWIF_VCMD_IRQ_INTCMD);
	if (cmdbuf_id) {
		if (subsys_core->hw_version_id <= HW_ID_1_0_C) {
			if (cmdbuf_id >= TOTAL_DISCRETE_CMDBUF_NUM) {
				pr_err("ftv310_vpu_vcmd_isr error cmdbuf_id greater than the ceiling !!\n");
				spin_unlock_irqrestore(&subsys_core->spinlock,
						       flags);
				return IRQ_HANDLED;
			}
		}
		new_cmdbuf_node = subsys_dev->global_cmdbuf_node[cmdbuf_id];
		if (!new_cmdbuf_node) {
			pr_err("ftv310_vpu_vcmd_isr error cmdbuf_id !!\n");
			spin_unlock_irqrestore(&subsys_core->spinlock, flags);
			return IRQ_HANDLED;
		}
		// interrupt cmdbuf and cmdbufs prior to itself, run_done = 1
		while (1) {
			if (!new_cmdbuf_node)
				break;
			cmdbuf_obj = (struct cmdbuf_obj *)new_cmdbuf_node->data;
			if (cmdbuf_obj->cmdbuf_run_done == 0) {
				cmdbuf_obj->cmdbuf_run_done = 1;
				cmdbuf_obj->executing_status =
					CMDBUF_EXE_STATUS_OK;
				cmdbuf_processed_num++;
			} else {
				break;
			}
			new_cmdbuf_node = new_cmdbuf_node->previous;
		}
		handled++;
	}

	spin_unlock_irqrestore(&subsys_core->spinlock, flags);
	if (cmdbuf_processed_num)
		wake_up_interruptible_all(&subsys_core->wait_queue);
	if (!handled)
		PDEBUG("IRQ received, but not ftv310_vpu's!\n");

	wake_up_interruptible_all(&subsys_dev->mc_wait_queue);
	return IRQ_HANDLED;
}

long vcmd_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	vcmd_dev_str *subsys_dev;
	vcmd_core_str *subsys_core;
	ftv310_vpu_ioctl_id ioctl_id_par;
	u32 i = 0;

	PDEBUG("ioctl cmd 0x%08x\n", cmd);

	switch (cmd) {
	case VCMD_IOCH_GET_CMDBUF_PARAMETER: {
		struct cmdbuf_mem_parameter local_cmdbuf_mem_data;
	PDEBUG(" VCMD GET_CMDBUF_PARAMETER\n");
		ret = copy_from_user(&local_cmdbuf_mem_data,
				     (struct cmdbuf_mem_parameter *)arg,
				     sizeof(struct cmdbuf_mem_parameter));
		if (ret)
			return ret;
		ioctl_id_par.data = local_cmdbuf_mem_data.id;
		subsys_dev = get_dev_by_sliceidx(ioctl_id_par.ID_PAR.node_idx,
						 ioctl_id_par.ID_PAR.group_idx);
		local_cmdbuf_mem_data.cmdbuf_unit_size = CMDBUF_MAX_SIZE;
		local_cmdbuf_mem_data.status_cmdbuf_unit_size = CMDBUF_MAX_SIZE;
		local_cmdbuf_mem_data.cmdbuf_total_size =
			CMDBUF_POOL_TOTAL_SIZE;
		local_cmdbuf_mem_data.status_cmdbuf_total_size =
			CMDBUF_POOL_TOTAL_SIZE;
		local_cmdbuf_mem_data.phy_status_cmdbuf_addr =
			subsys_dev->vcmd_status_buf_mem_pool->busAddress;
		local_cmdbuf_mem_data.phy_cmdbuf_addr =
			subsys_dev->vcmd_buf_mem_pool->busAddress;
#ifdef HAS_MMU
		local_cmdbuf_mem_data.mmu_phy_status_cmdbuf_addr =
			subsys_dev->vcmd_status_buf_mem_pool->mmu_bus_address;
		local_cmdbuf_mem_data.mmu_phy_cmdbuf_addr =
			subsys_dev->vcmd_buf_mem_pool->mmu_bus_address;
#else
		local_cmdbuf_mem_data.mmu_phy_status_cmdbuf_addr =
			subsys_dev->vcmd_status_buf_mem_pool->busAddress - subsys_dev->base_ddr_addr;
		local_cmdbuf_mem_data.mmu_phy_cmdbuf_addr =
			subsys_dev->vcmd_buf_mem_pool->busAddress - subsys_dev->base_ddr_addr;
#endif
		local_cmdbuf_mem_data.base_ddr_addr = subsys_dev->base_ddr_addr;
		ret = copy_to_user((struct cmdbuf_mem_parameter *)arg,
				   &local_cmdbuf_mem_data,
				   sizeof(struct cmdbuf_mem_parameter));
		trace_vcmd_params(TOTAL_DISCRETE_CMDBUF_NUM, CMDBUF_MAX_SIZE,
			subsys_dev->vcmd_buf_mem_pool->busAddress);
		break;
	}
	case VCMD_IOCH_GET_VCMD_PARAMETER: {
		struct vcmd_cfg_par input_para;

		PDEBUG(" VCMD get vcmd config parameter\n");
		ret = copy_from_user(&input_para, (struct vcmd_cfg_par *)arg,
				     sizeof(struct vcmd_cfg_par));
		if (ret)
			return ret;
		ioctl_id_par.data = input_para.id;
		subsys_dev = get_dev_by_sliceidx(ioctl_id_par.ID_PAR.node_idx,
						 ioctl_id_par.ID_PAR.group_idx);
		if (subsys_dev->vcmd_type_core_num[input_para.module_type]) {
			input_para.submodule_main_addr =
				subsys_dev
					->vcmd_manager[input_para.module_type][0]
					->vcmd_core_cfg.submodule_main_addr;
			input_para.submodule_dec400_addr =
				subsys_dev
					->vcmd_manager[input_para.module_type][0]
					->vcmd_core_cfg.submodule_dec400_addr;
			input_para.submodule_L2Cache_addr =
				subsys_dev
					->vcmd_manager[input_para.module_type][0]
					->vcmd_core_cfg.submodule_L2Cache_addr;
			input_para.submodule_MMU_addr =
				subsys_dev
					->vcmd_manager[input_para.module_type][0]
					->vcmd_core_cfg.submodule_MMU_addr;
			input_para.submodule_MMUWrite_addr =
				subsys_dev
					->vcmd_manager[input_para.module_type][0]
					->vcmd_core_cfg.submodule_MMUWrite_addr;
			for (i = 0; i < 2; i++)
				input_para.submodule_axife_addr[i] =
					subsys_dev
						->vcmd_manager
							[input_para.module_type]
							[0]
						->vcmd_core_cfg
						.submodule_axife_addr[i];
			input_para.config_status_cmdbuf_id =
				subsys_dev
					->vcmd_manager[input_para.module_type][0]
					->status_cmdbuf_id;
			input_para.vcmd_hw_version_id =
				subsys_dev
					->vcmd_manager[input_para.module_type][0]
					->hw_version_id;
			input_para.vcmd_core_num =
				subsys_dev->vcmd_type_core_num
					[input_para.module_type];
		} else {
			input_para.submodule_main_addr = 0xffff;
			input_para.submodule_dec400_addr = 0xffff;
			input_para.submodule_L2Cache_addr = 0xffff;
			input_para.submodule_MMU_addr = 0xffff;
			input_para.submodule_MMUWrite_addr = 0xffff;
			input_para.config_status_cmdbuf_id = 0;
			input_para.vcmd_core_num = 0;
			input_para.vcmd_hw_version_id = HW_ID_1_0_C;
		}
		ret = copy_to_user((struct vcmd_cfg_par *)arg, &input_para,
				   sizeof(struct vcmd_cfg_par));
		break;
	}
	case VCMD_IOCH_RESERVE_CMDBUF: {
		struct exchange_parameter input_para;

		PDEBUG(" VCMD Reserve CMDBUF\n");
		ret = copy_from_user(&input_para,
				     (struct exchange_parameter *)arg,
				     sizeof(struct exchange_parameter));
		if (ret)
			return ret;
		ioctl_id_par.data = input_para.id;
		subsys_dev = get_dev_by_sliceidx(ioctl_id_par.ID_PAR.node_idx,
						 ioctl_id_par.ID_PAR.group_idx);
		ret = reserve_cmdbuf(filp, subsys_dev, &input_para);
		if (ret == 0)
			ret = copy_to_user((struct exchange_parameter *)arg,
					   &input_para,
					   sizeof(struct exchange_parameter));
		return ret;
	}

	case VCMD_IOCH_LINK_RUN_CMDBUF: {
		struct exchange_parameter input_para;
		long ret_val;

		PDEBUG(" VCMD Reserve CMDBUF\n");
		ret = copy_from_user(&input_para,
				     (struct exchange_parameter *)arg,
				     sizeof(struct exchange_parameter));
		if (ret)
			return ret;
		ioctl_id_par.data = input_para.id;
		subsys_dev = get_dev_by_sliceidx(ioctl_id_par.ID_PAR.node_idx,
						 ioctl_id_par.ID_PAR.group_idx);
		PDEBUG("VCMD link and run cmdbuf\n");
		ret_val = link_and_run_cmdbuf(filp, subsys_dev, &input_para);
		ret_val = copy_to_user((struct exchange_parameter *)arg,
				       &input_para,
				       sizeof(struct exchange_parameter));
		return ret_val;
	}

	case VCMD_IOCH_WAIT_CMDBUF: {
		struct cmdbuf_id_parameter cmdbuf_par;
		unsigned int tmp;
		u32 irq_status_ret = 0;

		ret = copy_from_user(&cmdbuf_par,
				     (struct cmdbuf_id_parameter *)arg,
				     sizeof(struct cmdbuf_id_parameter));
		if (ret)
			return ret;

		ioctl_id_par.data = cmdbuf_par.id;
		subsys_dev = get_dev_by_sliceidx(ioctl_id_par.ID_PAR.node_idx,
						 ioctl_id_par.ID_PAR.group_idx);
		tmp = wait_cmdbuf_ready(filp, subsys_dev, cmdbuf_par.cmdbuf_id,
					&irq_status_ret);
		cmdbuf_par.cmdbuf_id = (u16)irq_status_ret;
		PDEBUG("VCMD wait for CMDBUF finishing re tmp %d.\n", tmp);

		if (tmp == 0) {
			tmp = copy_to_user((struct cmdbuf_id_parameter *)arg,
					   &cmdbuf_par,
					   sizeof(struct cmdbuf_id_parameter));
			return tmp; //return core_id
		} else if(tmp == -ERESTARTSYS){
			return tmp;
		} else {
			cmdbuf_par.cmdbuf_id = 0;
			ret = copy_to_user((struct cmdbuf_id_parameter *)arg,
					   &cmdbuf_par,
					   sizeof(struct cmdbuf_id_parameter));
			return -1;
		}
	}
	case VCMD_IOCH_RELEASE_CMDBUF: {
		struct cmdbuf_id_parameter cmdbuf_par;

		PDEBUG("VCMD release CMDBUF\n");
		ret = copy_from_user(&cmdbuf_par,
				     (struct cmdbuf_id_parameter *)arg,
				     sizeof(struct cmdbuf_id_parameter));
		if (ret)
			return ret;
		ioctl_id_par.data = cmdbuf_par.id;
		subsys_dev = get_dev_by_sliceidx(ioctl_id_par.ID_PAR.node_idx,
						 ioctl_id_par.ID_PAR.group_idx);
		release_cmdbuf(filp, subsys_dev, cmdbuf_par.cmdbuf_id);
		break;
	}
	case VCMD_IOCH_POLLING_CMDBUF: {
		unsigned int id;

		ret = copy_from_user(&id, (unsigned int *)arg,
				     sizeof(unsigned int));
		if (ret)
			return ret;
		ioctl_id_par.data = id;
		subsys_dev = get_dev_by_sliceidx(ioctl_id_par.ID_PAR.node_idx,
						 ioctl_id_par.ID_PAR.group_idx);
		subsys_core = get_dev_of_core(subsys_dev,
					      ioctl_id_par.ID_PAR.codec_idx);
		ftv310_vpu_vcmd_isr(id, subsys_core);
		break;
	}
	default: {
		break;
	}
	}
	return ret;
}

static void
create_read_all_registers_cmdbuf(vcmd_core_str *subsys_core,
				 struct exchange_parameter *input_para)
{
	u32 register_num[] = { VCMD_ENCODER_REGISTER_SIZE / 4,
				 VCMD_IM_REGISTER_SIZE / 4,
				 VCMD_DECODER_REGISTER_SIZE / 4,
				 VCMD_JPEG_ENCODER_REGISTER_SIZE / 4,
				 VCMD_JPEG_DECODER_REGISTER_SIZE / 4};
	u32 *cmd_base = NULL;
	u32 *ptr = NULL;
	ptr_t status_base_addr;
	u32 i = 0;
	vcmd_dev_str *subsys_dev;
	struct vcmd_config *core_cfg = NULL;
	u16 module = input_para->module_type;

	subsys_dev = subsys_core->parent_dev;
	cmd_base = subsys_dev->vcmd_buf_mem_pool->virtualAddress +
			input_para->cmdbuf_id * CMDBUF_MAX_SIZE / 4;
	ptr = cmd_base;

#ifdef HAS_MMU
	status_base_addr =
		subsys_dev->vcmd_status_buf_mem_pool->mmu_bus_address;
#else
	status_base_addr =
			subsys_dev->vcmd_status_buf_mem_pool->busAddress - subsys_dev->base_ddr_addr;
#endif
	status_base_addr += input_para->cmdbuf_id * CMDBUF_MAX_SIZE;

	core_cfg = &subsys_dev
					->vcmd_manager[module]
						[input_para->core_id]
					->vcmd_core_cfg;
	if (subsys_dev
		    ->vcmd_manager[module][input_para->core_id]
		    ->hw_version_id > HW_ID_1_0_C) {
		pr_info("vcmd_driver:create cmdbuf data when hw_version_id = 0x%x\n",
			subsys_dev
				->vcmd_manager[module]
					      [input_para->core_id]
				->hw_version_id);

		VCMD_READ_CMD(ptr, 1, EXECUTING_CMDBUF_ID_ADDR * 4, 0);
		//read main IP all registers
		VCMD_READ_CMD(ptr,
					  register_num[module],
					  core_cfg->submodule_main_addr + 0,
					  status_base_addr + core_cfg->submodule_main_addr / 2 + 0);

		if (core_cfg->submodule_L2Cache_addr != 0xffff) {
			//read L2cache IP register 0
			VCMD_READ_CMD(ptr,
						  1,
						  core_cfg->submodule_L2Cache_addr + 0,
						  status_base_addr + core_cfg->submodule_L2Cache_addr / 2 + 0);
		}

		if (core_cfg->submodule_dec400_addr != 0xffff) {
			//read dec400 register
			VCMD_READ_CMD(ptr,
						  0x2b,
						  core_cfg->submodule_dec400_addr + 0,
						  status_base_addr + core_cfg->submodule_dec400_addr / 2 + 0);
		}

		for (i = 0; i < 2; i++) {
			if (core_cfg->submodule_axife_addr[i] != 0xffff) {
				//read AXIFE register 0
				VCMD_READ_CMD(ptr,
							  1,
							  core_cfg->submodule_axife_addr[i] + 0,
							  status_base_addr + core_cfg->submodule_axife_addr[i] / 2 + 0);
			}
		}

		VCMD_READ_CMD(ptr, 27, 0, 0);
	} else {
		pr_info("vcmd_driver:create cmdbuf data when hw_version_id = 0x%x\n",
			subsys_dev
				->vcmd_manager[module]
					      [input_para->core_id]
				->hw_version_id);
		//read all registers
				//read all registers
		VCMD_READ_CMD(ptr,
					  register_num[module],
					  core_cfg->submodule_main_addr + 0,
					  status_base_addr + core_cfg->submodule_main_addr / 2 + 0);

	}
	//JMP RDY = 0
	*ptr++ = OPCODE_JMP_RDY0 | 0 | JMP_IE_1 | 0;
	*ptr++ = 0;
	*ptr++ = 0;
	*ptr++ = input_para->cmdbuf_id;
	input_para->cmdbuf_size = (ptr - cmd_base) * sizeof(u32);
}

static void read_main_module_all_registers(vcmd_core_str *subsys_core)
{
	int count = 0;
	int ret;
	struct exchange_parameter input_para;
	u32 irq_status_ret = 0;
	u32 *status_base_virt_addr;
	u32 main_module_type;
	u32 manager_id;
	u32 core_id;
	u32 tmp_result = 0;
	u32 result = 0;
	vcmd_dev_str *subsys_dev;

	if (!subsys_core)
		return;
	subsys_dev = subsys_core->parent_dev;
	core_id = subsys_core->core_id;

	main_module_type = subsys_core->vcmd_core_cfg.sub_module_type;
	manager_id = subsys_core->id_in_type;
	input_para.interrupt_ctrl = 0;
	input_para.priority = CMDBUF_PRIORITY_NORMAL;
	input_para.module_type = main_module_type;
	input_para.cmdbuf_size = 0;
	input_para.core_id = manager_id;
	input_para.req_cores_info = 1<<core_id;

	tmp_result =  vcmd_read_reg((const void *)subsys_core->hwregs,
						VCMD_REGISTER_EXE_CMDBUF_COUNT_OFFSET);
	ret = reserve_cmdbuf(NULL, subsys_dev, &input_para);
	(void)(ret);
	subsys_dev->vcmd_manager[main_module_type][manager_id]
		->status_cmdbuf_id = input_para.cmdbuf_id;
	create_read_all_registers_cmdbuf(subsys_core, &input_para);
	link_and_run_cmdbuf(NULL, subsys_dev, &input_para);
	while(count < 10){
		msleep(5);
		result =  vcmd_read_reg((const void *)subsys_core->hwregs,
						VCMD_REGISTER_EXE_CMDBUF_COUNT_OFFSET);
		count++;
		if (result != tmp_result)
			break;
	}

	subsys_core = get_dev_of_core(subsys_dev, input_para.core_id);
	ftv310_vpu_vcmd_isr(input_para.core_id, subsys_core);
	wait_cmdbuf_ready(NULL, subsys_dev, input_para.cmdbuf_id,
			  &irq_status_ret);
	status_base_virt_addr =
		subsys_dev->vcmd_status_buf_mem_pool->virtualAddress +
		input_para.cmdbuf_id * CMDBUF_MAX_SIZE / 4 +
		(subsys_dev->vcmd_manager[main_module_type][manager_id]
				 ->vcmd_core_cfg.submodule_main_addr / 2 / 4 +
		 0);
	if (subsys_dev->type == FTV310_VPU_ENCODER) {
		pr_info("vcmd_driver: main module register 0:0x%x\n",
			*status_base_virt_addr);
		pr_info("vcmd_driver: main module register 80:0x%x\n",
			*(status_base_virt_addr + 80));
		pr_info("vcmd_driver: main module register 214:0x%x\n",
			*(status_base_virt_addr + 214));
		pr_info("vcmd_driver: main module register 226:0x%x\n",
			*(status_base_virt_addr + 226));
		pr_info("vcmd_driver: main module register 287:0x%x\n",
			*(status_base_virt_addr + 287));
		pr_info("vcmd_driver: main module register 509:0x%x\n",
			*(status_base_virt_addr + 509));
	} else if (subsys_dev->type == FTV310_VPU_DECODER) {
		pr_info("vcmd_driver: main module register 0:0x%x\n",
			*status_base_virt_addr);
		pr_info("vcmd_driver: main module register 50:0x%08x\n",
			*(status_base_virt_addr + 50));
		pr_info("vcmd_driver: main module register 54:0x%08x\n",
			*(status_base_virt_addr + 54));
		pr_info("vcmd_driver: main module register 56:0x%08x\n",
			*(status_base_virt_addr + 56));
		pr_info("vcmd_driver: main module register 309:0x%x\n",
			*(status_base_virt_addr + 309));
	} else {
		pr_err("%s,%d unkonwn subsys_dev->type %d\n\n", __func__, __LINE__, subsys_dev->type);
	}
}

static int ftv310_vpu_vcmd_mem_pool_init(vcmd_dev_str *subsys_dev)
{
	int ret;

#ifndef PHY_FPGA_MEM
	ret = customized_init_vcmd_mem_pool(subsys_dev);
	if (ret != 0)
		return -1;
#else
	ret = init_vcmd_pool_on_ftv310_vpu_fpgamem(subsys_dev);
	if (ret != 0)
		return -1;
#endif

#ifdef HAS_MMU
	get_vcmd_pool_mmuaddr(subsys_dev);
#endif
	return 0;
}

int ftv310_vpu_vcmd_subsys_probe(vcmd_dev_str *subsys_dev)
{
	vcmd_core_str *subsys_core;
	u32 i, k;
	int result;

	for (k = 0; k < MAX_VCMD_TYPE; k++) {
		subsys_dev->vcmd_type_core_num[k] = 0;
		subsys_dev->vcmd_position[k] = 0;
		for (i = 0; i < MAX_VCMD_NUMBER; i++)
			subsys_dev->vcmd_manager[k][i] = NULL;
	}

	init_bi_list(&subsys_dev->global_process_manager);

	init_waitqueue_head(&subsys_dev->vcmd_cmdbuf_memory_wait);
	spin_lock_init(&subsys_dev->vcmd_cmdbuf_alloc_lock);
	spin_lock_init(&subsys_dev->vcmd_process_manager_lock);

	subsys_core = subsys_dev->vcmd_core;
	for (i = 0; i < subsys_dev->subsys_num; i++) {
		subsys_core->working_state = WORKING_STATE_IDLE;
		subsys_core->sw_cmdbuf_rdy_num = 0;
		spin_lock_init(&subsys_core->spinlock);
		init_waitqueue_head(&subsys_core->wait_queue);
		init_waitqueue_head(&subsys_core->wait_abort_queue);
		init_bi_list(&subsys_core->list_manager);
		subsys_core->duration_without_int = 0;
		subsys_core->id_in_type =
			subsys_dev->vcmd_type_core_num
				[subsys_core->vcmd_core_cfg.sub_module_type];
		subsys_dev->vcmd_manager
			[subsys_core->vcmd_core_cfg.sub_module_type]
			[subsys_dev->vcmd_type_core_num
				 [subsys_core->vcmd_core_cfg.sub_module_type]] =
			subsys_core;
		subsys_dev->vcmd_type_core_num[subsys_core->vcmd_core_cfg
						       .sub_module_type]++;
		subsys_core->vcmd_reg_mem_busAddress =
			subsys_dev->vcmd_registers_mem_pool->busAddress +
			i * VCMD_REGISTER_SIZE - subsys_dev->base_ddr_addr;
#ifdef HAS_MMU
		subsys_core->mmu_vcmd_reg_mem_bus_address =
			subsys_dev->vcmd_registers_mem_pool->mmu_bus_address +
			i * VCMD_REGISTER_SIZE;
#endif
		subsys_core->vcmd_reg_mem_virtualAddress =
			subsys_dev->vcmd_registers_mem_pool->virtualAddress +
			i * VCMD_REGISTER_SIZE / 4;
		subsys_core->vcmd_reg_mem_size = VCMD_REGISTER_SIZE;
		memset(subsys_core->vcmd_reg_mem_virtualAddress, 0,
		       VCMD_REGISTER_SIZE);
		subsys_core = subsys_core->core_next;
	}
	init_waitqueue_head(&subsys_dev->mc_wait_queue);
	result = vcmd_reserve_IO(subsys_dev);
	if (result < 0)
		goto err;
	vcmd_reset_asic(subsys_dev);
	/* get the IRQ line */

	subsys_core = subsys_dev->vcmd_core;
	for (i = 0; i < subsys_dev->subsys_num; i++) {
		if (!subsys_core->hwregs)
			continue;
		if (subsys_core->useirq &&
			subsys_core->vcmd_core_cfg.vcmd_irq != -1) {
			result =
				request_irq(subsys_core->vcmd_core_cfg.vcmd_irq,
					    ftv310_vpu_vcmd_isr,
#if (KERNEL_VERSION(2, 6, 18) > LINUX_VERSION_CODE)
					    SA_INTERRUPT | SA_SHIRQ,
#else
					    IRQF_SHARED,
#endif
					    "ftv310_vpu_vcmd", (void *)subsys_core);
			if (result == -EINVAL) {
				pr_err("vcmd_driver: Bad vcmd_irq number or handler core_id=%d\n",
				       i);
				vcmd_release_IO(subsys_dev);
				goto err;
			} else if (result == -EBUSY) {
				pr_err("vcmd_driver: IRQ <%d> busy, change your config core_id=%d\n",
				       subsys_core->vcmd_core_cfg.vcmd_irq, i);
				vcmd_release_IO(subsys_dev);
				goto err;
			}
		} else {
			pr_info("vcmd_driver: IRQ not in use, global useirq = %d, vmcd_irq = %d\n",
				subsys_core->useirq, subsys_core->vcmd_core_cfg.vcmd_irq);
			subsys_core->vcmd_core_cfg.vcmd_irq = -1;
		}
		subsys_core = subsys_core->core_next;
	}
	subsys_dev->cmdbuf_used_pos = 0;
	for (k = 0; k < TOTAL_DISCRETE_CMDBUF_NUM; k++) {
		subsys_dev->cmdbuf_used[k] = 0;
		subsys_dev->global_cmdbuf_node[k] = NULL;
	}
	//cmdbuf_used[0] not be used, because int vector must non-zero
	subsys_dev->cmdbuf_used_residual = TOTAL_DISCRETE_CMDBUF_NUM;
	subsys_dev->cmdbuf_used_pos = 1;
	subsys_dev->cmdbuf_used[0] = 1;
	subsys_dev->cmdbuf_used_residual -= 1;

	create_kernel_process_manager(subsys_dev);
	/* read all registers for each type of module
	 *for analyzing configuration in cwl
	 */
	subsys_core = subsys_dev->vcmd_core;
	if (subsys_core == NULL)
		goto err;
	for (i = 0; i < subsys_dev->subsys_num; i++) {
		read_main_module_all_registers(subsys_core);
		subsys_core = subsys_core->core_next;
		if (subsys_core == NULL)
			break;
	}
	return 0;
err:
	return result;
}

static void ftv310_vpu_vcmd_mem_pool_release(vcmd_dev_str *subsys_dev)
{
#ifndef PHY_FPGA_MEM
	customized_release_vcmd_mem_pool(subsys_dev);
#else
	release_vcmd_pool_on_ftv310_vpu_fpgamem(subsys_dev);
#endif

	kfree(subsys_dev->vcmd_buf_mem_pool);
	kfree(subsys_dev->vcmd_status_buf_mem_pool);
	kfree(subsys_dev->vcmd_registers_mem_pool);
}

static int ftv310_vpu_vcmd_subsys_cleanup(vcmd_dev_str *subsys_dev)
{
	u32 i;
	u32 result;
	vcmd_core_str *subsys_core;
	vcmd_core_str *back_core;

	subsys_core = subsys_dev->vcmd_core;

	for (i = 0; i < subsys_dev->subsys_num; i++) {
		vcmd_write_reg((const void *)subsys_core->hwregs,
			       VCMD_REGISTER_INT_CTL_OFFSET, 0x0000);
		//disable HW
		vcmd_write_reg((const void *)subsys_core->hwregs,
			       VCMD_REGISTER_CONTROL_OFFSET, 0x0000);
		//read status register
		result = vcmd_read_reg((const void *)subsys_core->hwregs,
				       VCMD_REGISTER_INT_STATUS_OFFSET);
		//clean status register
		vcmd_write_reg((const void *)subsys_core->hwregs,
			       VCMD_REGISTER_INT_STATUS_OFFSET, result);

		/* free the vcmd IRQ */
		if (subsys_core->vcmd_core_cfg.vcmd_irq != -1)
			free_irq(subsys_core->vcmd_core_cfg.vcmd_irq,
				 (void *)subsys_core);
		release_cmdbuf_node_cleanup(subsys_dev,
					    &subsys_core->list_manager);
		subsys_core = subsys_core->core_next;
	}
	release_process_node_cleanup(&subsys_dev->global_process_manager);

	vcmd_release_IO(subsys_dev);

	subsys_core = subsys_dev->vcmd_core;

	for (i = 0; i < subsys_dev->subsys_num; i++) {
		back_core = subsys_core;
		subsys_core = subsys_core->core_next;
		kfree(back_core);
	}

	subsys_dev->subsys_num = 0;
	subsys_dev->vcmd_core = NULL;

	return 0;
}

#ifdef USE_DTB_PROBE
int ftv310_vpu_vcmd_init(u32 sliceidx)
{
	vcmd_slice_str *cur_slice;

	cur_slice = get_slice_by_sliceidx(sliceidx);
	if (cur_slice->dec_vcmd.subsys_num != 0)
		ftv310_vpu_vcmd_subsys_probe(&cur_slice->dec_vcmd);
	if (cur_slice->enc_vcmd.subsys_num != 0)
		ftv310_vpu_vcmd_subsys_probe(&cur_slice->enc_vcmd);
	return 0;
}

int ftv310_vpu_vcmd_probe(struct platform_device *pdev, int useirq, struct device_node *slice, unsigned long ddr_base, unsigned long reg_base)
{
	vcmd_slice_str *cur_slice;

	cur_slice = ftv310_vpu_vcmd_analyze_subnode(pdev, useirq, slice);
	return cur_slice->sliceidx;
}
#else

int ftv310_vpu_vcmd_init(void *p)
{
	int i;
	int slice_num = 0;
	vcmd_slice_str *cur_slice;
	unsigned long base_reg_hw;
	unsigned long base_ddr_hw;
	struct ftv310_vpu_base_addr *base_addr = p;

	base_reg_hw = base_addr->reg_base;
	base_ddr_hw = base_addr->ddr_base;
	slice_num = get_vcmd_slice_num();
	for (i = 0; i < slice_num; i++) {
		cur_slice = get_slice_by_sliceidx(i);

		PDEBUG("%s dec num %d, enc num %d\n", __func__,
			cur_slice->dec_vcmd.subsys_num, cur_slice->enc_vcmd.subsys_num);

		if (cur_slice->dec_vcmd.subsys_num != 0) {
			cur_slice->dec_vcmd.sliceidx = i;
			vcmd_set_base_addr(&cur_slice->dec_vcmd, base_reg_hw, base_ddr_hw);
			if (ftv310_vpu_vcmd_mem_pool_init(&cur_slice->dec_vcmd) < 0) {
				pr_err("%s,%d ftv310_vpu_vcmd_mem_pool_init failed\n", __func__, __LINE__);
				return -1;
			}
			if (ftv310_vpu_vcmd_subsys_probe(&cur_slice->dec_vcmd) < 0) {
				pr_err("%s,%d ftv310_vpu_vcmd_subsys_probe failed\n", __func__, __LINE__);
				return -1;
			}
		}

		if (cur_slice->enc_vcmd.subsys_num != 0) {
			cur_slice->enc_vcmd.sliceidx = i;
			vcmd_set_base_addr(&cur_slice->enc_vcmd, base_reg_hw, base_ddr_hw);

			if (ftv310_vpu_vcmd_mem_pool_init(&cur_slice->enc_vcmd) < 0) {
				pr_err("%s,%d ftv310_vpu_vcmd_mem_pool_init failed\n", __func__, __LINE__);
				return -1;
			}
			if (ftv310_vpu_vcmd_subsys_probe(&cur_slice->enc_vcmd) < 0) {
				pr_err("%s,%d ftv310_vpu_vcmd_subsys_probe failed\n", __func__, __LINE__);
				return -1;
			}
		}
	}
	return 0;
}

int ftv310_vpu_get_core_type(u32 sub_module_type)
{
	int core_type;

	switch (sub_module_type) {
#ifdef HAS_VCE
	case VCMD_TYPE_ENCODER:
	case VCMD_TYPE_CUTREE:
		core_type = FTV310_VPU_CORE_ENC;
		break;
	case VCMD_TYPE_JPEG_ENCODER:
		core_type = FTV310_VPU_CORE_ENCJPG;
		break;
#endif
#ifdef HAS_VCD
	case VCMD_TYPE_DECODER:
		core_type = FTV310_VPU_CORE_DEC;
		break;
	case VCMD_TYPE_JPEG_DECODER:
		core_type = FTV310_VPU_CORE_DECJPG;
		break;
#endif
	default:
		core_type = -1;
		break;
	}
	return core_type;
}

int ftv310_vpu_vcmd_probe(struct platform_device *pdev, int useirq, struct device_node *slice,
	unsigned long ddr_base, unsigned long reg_base)
{
	int i, j;
	int ret = 0;
	int slice_num = 0;
	int core_num = 0;
	vcmd_slice_str *cur_slice;
	vcmd_core_str *vcmd_core;
	struct vcmd_config *vcmd_cfg_p;
	struct device *dev = &pdev->dev;
	struct mmu_core_cfg tmp;

	slice_num = ARRAY_SIZE(vcmd_core_array);
	for (i = 0; i < slice_num; i++) {
		cur_slice = kzalloc(sizeof(*cur_slice), GFP_KERNEL);
		add_vcmd_slice(dev, cur_slice);
		add_norslice(dev, i);

		ftv310_vpu_power_init(pdev, cur_slice);

		if (has_acpi_companion(dev)) {
			ret = ftv310_vpu_voltage_request(dev, true, &cur_slice->dec_voltage);
			if (ret)
				pr_err("Get the voltage of decoder failed!\n");
			ret = 0;
			ret = ftv310_vpu_voltage_request(dev, false, &cur_slice->enc_voltage);
			if (ret)
				pr_err("Get the voltage of encoder failed!\n");
		}
		
		core_num = ARRAY_SIZE(vcmd_core_array[i]);

		for (j = 0; j < core_num; j++) {
			if (vcmd_core_array[i][j].submodule_main_addr == 0 ||
				vcmd_core_array[i][j].submodule_main_addr == 0xffff)
				continue;

			vcmd_core = kzalloc(sizeof(*vcmd_core), GFP_KERNEL);
			vcmd_core->useirq = useirq;
			vcmd_cfg_p = &vcmd_core->vcmd_core_cfg;
			memcpy(vcmd_cfg_p, &vcmd_core_array[i][j],
			       sizeof(struct vcmd_config));
			ensure_vcmd_submodule_addr(vcmd_cfg_p);
			set_vcmd_slice_config(cur_slice, vcmd_cfg_p);

			vcmd_core->type = ftv310_vpu_get_core_type(
				vcmd_cfg_p->sub_module_type);
			if (vcmd_core->type == -1) {
				pr_err("error! %s,%d sub_module_type is not correct type %d\n",
				       __func__, __LINE__, vcmd_cfg_p->sub_module_type);
				kfree(vcmd_core);
				continue;
			}

			add_vcmd_core(cur_slice, vcmd_core);
#ifdef HAS_MMU
			tmp.iosize = MMU_IO_SIZE;
			tmp.mmucorebase = reg_base + vcmd_cfg_p->vcmd_base_addr + vcmd_cfg_p->submodule_MMU_addr;
			tmp.sliceidx = i;
			if (vcmd_core->type == FTV310_VPU_CORE_DEC)
				tmp.parenttype = FTV310_VPU_CORE_DEC;
			else if (vcmd_core->type == FTV310_VPU_CORE_ENC)
				tmp.parenttype = FTV310_VPU_CORE_ENC;

			if (vcmd_cfg_p->submodule_MMU_addr != 0xffff)
				ftv310_vpu_MMUprobe(NULL, ALL_RESOURCE, NULL, pdev, ddr_base, &tmp);
#endif
		}
		transfer_vcmdslice_to_norslice(i);
	}

	return slice_num;
}


#endif

int ftv310_vpu_vcmd_cleanup(void)
{
	vcmd_slice_str *cur_slice;
	vcmd_slice_str *back_slice;

	cur_slice = get_vcmd_slice_head();
	while (cur_slice) {
		if (cur_slice->dec_vcmd.subsys_num != 0) {
			ftv310_vpu_vcmd_subsys_cleanup(&cur_slice->dec_vcmd);
			ftv310_vpu_vcmd_mem_pool_release(&cur_slice->dec_vcmd);
		}
		if (cur_slice->enc_vcmd.subsys_num != 0) {
			ftv310_vpu_vcmd_subsys_cleanup(&cur_slice->enc_vcmd);
			ftv310_vpu_vcmd_mem_pool_release(&cur_slice->enc_vcmd);
		}
		back_slice = cur_slice;
		cur_slice = cur_slice->slice_next;
		kfree(back_slice);
	}
	set_vcmd_slice_listnull();
	return 1;
}

/* Update the last JMP cmd in cmdbuf_ojb in order to jump to next_cmdbuf_obj. */
static void cmdbuf_update_jmp_cmd(int hw_version_id,
					struct cmdbuf_obj *cmdbuf_obj,
					struct cmdbuf_obj *next_cmdbuf_obj,
					int jmp_IE_1,
					struct vcmd_dev *subsys_dev)
{
	u32 *jmp_addr;
	u64 next_cmd_buf_addr;
	u32 operation_code;

	if (!cmdbuf_obj)
		return;

	if (cmdbuf_obj->has_end_cmdbuf == 0) {
		//need to link, current cmdbuf link to next cmdbuf
		jmp_addr = cmdbuf_obj->cmdbuf_virtualAddress + (cmdbuf_obj->cmdbuf_size / 4);
		if (!next_cmdbuf_obj) {
		// If next cmdbuf is not available, set the RDY to 0.
			operation_code = *(jmp_addr - 4);
			operation_code >>= 16;
			operation_code <<= 16;
			*(jmp_addr - 4) = (u32)(operation_code & ~JMP_RDY_1);
		} else {
			if (hw_version_id > HW_ID_1_0_C) {
				//set next cmdbuf id
				*(jmp_addr - 1) = next_cmdbuf_obj->cmdbuf_id;
			}
			next_cmd_buf_addr = (u64)(next_cmdbuf_obj->cmdbuf_busAddress -
					subsys_dev->base_ddr_addr);

#ifdef HAS_MMU
			next_cmd_buf_addr = (u64)(next_cmdbuf_obj->mmu_cmdbuf_bus_address);
#endif
			if (sizeof(size_t) == 8)
				*(jmp_addr - 2) = (u32)(next_cmd_buf_addr >> 32);
			else
				*(jmp_addr - 2) = 0;
			*(jmp_addr - 3) = (u32)(next_cmd_buf_addr);

			operation_code = *(jmp_addr - 4);
			operation_code >>= 16;
			operation_code <<= 16;
			*(jmp_addr - 4) = (u32)(operation_code | JMP_RDY_1 |
				(jmp_IE_1 ? JMP_IE_1 : 0) |
				((next_cmdbuf_obj->cmdbuf_size + 7) / 8));
		}

#ifdef VCMD_DEBUG_INTERNAL
		{
		u32 i;

		pr_info("vcmd link, last cmdbuf content\n");
		for (i = cmdbuf_obj->cmdbuf_size / 4 - 8; i < cmdbuf_obj->cmdbuf_size / 4; i++)
			pr_info("current linked cmdbuf data %d =0x%x\n",
				i, *(cmdbuf_obj->cmdbuf_virtualAddress + i));
		}
#endif
	}
}

/* delink given cmd buffer (cmdbuf_node) and remove it from list.
 * Also modify the last JMP of buf P to point to cmdbuf N.
 * Used when a process is terminated but there are pending cmd bufs in vmcd list.
 * E.g.,
 * before:

 *   L->L->...->P->X->N->        ...        ->L
 *   ^             ^                          ^
 *   head          cmdbuf_node                tail

 * end:

 *   L->L->...->P->N->        ...        ->L
 *   ^                                     ^
 *   head                                  tail

 * Return: pointer to N or NULL if N doesn't exist.
 */

void vcmd_delink_rm_cmdbuf(vcmd_core_str *subsys_core, bi_list_node *cmdbuf_node)
{
	struct vcmd_dev *subsys_dev = subsys_core->parent_dev;
	bi_list *list = &subsys_core->list_manager;
	struct cmdbuf_obj *cmdbuf_obj = (struct cmdbuf_obj *)cmdbuf_node->data;
	bi_list_node *prev = cmdbuf_node->previous;
	bi_list_node *next = cmdbuf_node->next;

	PDEBUG("Delink and remove cmdbuf [%d] from vcmd list.\n",
	       cmdbuf_obj->cmdbuf_id);
	//#ifdef FTV310_VPU_VCMD_DRIVER_DEBUG
	if (prev) {
		PDEBUG("prev cmdbuf [%d].\n",
		       ((struct cmdbuf_obj *)prev->data)->cmdbuf_id);
	} else {
		PDEBUG("NO prev cmdbuf.\n");
	}
	if (next) {
		PDEBUG("next cmdbuf [%d].\n",
		       ((struct cmdbuf_obj *)next->data)->cmdbuf_id);
	} else {
		PDEBUG("NO next cmdbuf.\n");
	}
	//#endif

	bi_list_remove_node(list, cmdbuf_node);
	subsys_dev->global_cmdbuf_node[cmdbuf_obj->cmdbuf_id] = NULL;
	free_cmdbuf_node(subsys_dev, cmdbuf_node);

	cmdbuf_update_jmp_cmd(subsys_core->hw_version_id, prev ? prev->data : NULL, next ? next->data : NULL,
			subsys_core->duration_without_int > INT_MIN_SUM_OF_IMAGE_SIZE, subsys_dev);
}
#define SUBS_DEV_COUNT (2)

int ftv310_vpu_vcmd_open(struct inode *inode, struct file *filp)
{
	int result = 0;
	int i = 0;
	vcmd_slice_str *cur_slice = get_vcmd_slice_head();
	bi_list_node *process_manager_node;
	unsigned long flags;
	struct process_manager_obj *process_manager_obj = NULL;

//TODO: refine enc/dec in one platform
	struct vcmd_dev *all_subsys_dev[SUBS_DEV_COUNT] = {NULL};

	if (cur_slice->enc_vcmd.subsys_num > 0)
		all_subsys_dev[0] = &cur_slice->enc_vcmd;
	if (cur_slice->dec_vcmd.subsys_num > 0)
		all_subsys_dev[1] = &cur_slice->dec_vcmd;

	for (i = 0; i < SUBS_DEV_COUNT; i++) {
		struct vcmd_dev *subsys_dev = all_subsys_dev[i];

		if (subsys_dev == NULL)
			continue;

		process_manager_node = create_process_manager_node();
		if (!process_manager_node)
			return -1;
		process_manager_obj = (struct process_manager_obj *)process_manager_node->data;
		process_manager_obj->filp = filp;
		spin_lock_irqsave(&subsys_dev->vcmd_process_manager_lock, flags);
		bi_list_insert_node_tail(&subsys_dev->global_process_manager, process_manager_node);
		spin_unlock_irqrestore(&subsys_dev->vcmd_process_manager_lock, flags);

		PDEBUG("dev opened\n");
		PDEBUG("process node %p for filp opened %p\n",
			(void *)process_manager_node, (void *)filp);
	}

	return result;
}

int ftv310_vpu_vcmd_release(struct inode *inode, struct file *filp)
{
	vcmd_slice_str *cur_slice = NULL;
	u32 core_id = 0;
	u32 i = 0;
	int total_vcmd_core_num = 0;
	u32 release_cmdbuf_num = 0;
	bi_list_node *new_cmdbuf_node = NULL;
	bi_list_node *next_cmdbuf_node = NULL;
	struct cmdbuf_obj *cmdbuf_obj_temp = NULL;
	bi_list_node *process_manager_node = NULL;

	int vcmd_aborted = 0;  // vcmd is aborted in this function
	struct cmdbuf_obj *restart_cmdbuf = NULL;

	unsigned long flags;
	long retVal = 0;
	struct vcmd_dev *all_subsys_dev[SUBS_DEV_COUNT] = {NULL};
	vcmd_core_str *subsys_core;
	int cmdbuf_used_pos;
	int ret = 0;

	if (!filp)
		pr_info("%s,%d\n", __func__, __LINE__);
	cur_slice = get_vcmd_slice_head();
	if (!cur_slice)
		return 0;

	total_vcmd_core_num = sizeof(vcmd_core_array) / sizeof(struct vcmd_config);

	if (cur_slice->enc_vcmd.subsys_num > 0)
		all_subsys_dev[0] = &cur_slice->enc_vcmd;
	if (cur_slice->dec_vcmd.subsys_num > 0)
		all_subsys_dev[1] = &cur_slice->dec_vcmd;

	for (i = 0; i < SUBS_DEV_COUNT; i++) {
		struct vcmd_dev *subsys_dev = all_subsys_dev[i];

		if (subsys_dev == NULL)
			continue;

		 subsys_core = subsys_dev->vcmd_core;

		if (subsys_core == NULL)
			continue;

		if (down_interruptible(&subsys_dev->vcmd_reserve_cmdbuf_sem[subsys_core->vcmd_core_cfg.sub_module_type])) {
			return -ERESTARTSYS;
		}
		for (core_id = 0; core_id < total_vcmd_core_num; core_id++) {
			if (subsys_core == NULL)
				continue;

			//wait cmdbuf done
			{
				bi_list_node *cur = NULL;
				bi_list_node *last = NULL;
				u32 irq_ret = 0;

				spin_lock_irqsave(&subsys_core->spinlock, flags);
				cur = subsys_core->list_manager.head;
				while (cur) {
					cmdbuf_obj_temp = (struct cmdbuf_obj *)cur->data;
					if (cmdbuf_obj_temp->filp == filp) {
						last = cur;
					}
					cur = cur->next;
				}
				spin_unlock_irqrestore(&subsys_core->spinlock, flags);

				if (last) {
					cmdbuf_obj_temp = (struct cmdbuf_obj *)last->data;
					while (!cmdbuf_obj_temp->cmdbuf_run_done) {
						ret = wait_cmdbuf_ready(filp, subsys_dev, cmdbuf_obj_temp->cmdbuf_id, &irq_ret);
						if (!ret) {
							break;
						} else if (ret == -ERESTARTSYS) {
							break;
						}
					}
				}
			}

			spin_lock_irqsave(&subsys_core->spinlock, flags);
			new_cmdbuf_node = subsys_core->list_manager.head;

			while (1) {
				if (!new_cmdbuf_node)
					break;
				next_cmdbuf_node = new_cmdbuf_node->next;

				cmdbuf_obj_temp = (struct cmdbuf_obj *)new_cmdbuf_node->data;
				PDEBUG("Process %p is releasing: checking cmdbuf %d of process %p.\n",
					filp, cmdbuf_obj_temp->cmdbuf_id, cmdbuf_obj_temp->filp);
				if (subsys_core->hwregs && (cmdbuf_obj_temp->filp == filp)) {
					trace_vcmd_clear(current->pid, cmdbuf_obj_temp->cmdbuf_id,
							cmdbuf_obj_temp->cmdbuf_data_linked, cmdbuf_obj_temp->cmdbuf_run_done,
							*(subsys_core->vcmd_reg_mem_virtualAddress +VCMD_EXE_CMDBUF_COUNT));
					if (cmdbuf_obj_temp->cmdbuf_run_done) {
						cmdbuf_obj_temp->cmdbuf_need_remove = 1;
						retVal = release_cmdbuf_node(subsys_dev,
							&subsys_core->list_manager, new_cmdbuf_node);
						if (retVal == 1)
							cmdbuf_obj_temp->process_manager_obj = NULL;
					} else if (cmdbuf_obj_temp->cmdbuf_data_linked == 0) {
						cmdbuf_obj_temp->cmdbuf_data_linked = 1;
						cmdbuf_obj_temp->cmdbuf_run_done    = 1;
						cmdbuf_obj_temp->cmdbuf_need_remove = 1;
						retVal = release_cmdbuf_node(subsys_dev,
							&subsys_core->list_manager, new_cmdbuf_node);
						if (retVal == 1)
							cmdbuf_obj_temp->process_manager_obj = NULL;
					} else if (cmdbuf_obj_temp->cmdbuf_data_linked == 1 &&
						subsys_core->working_state == WORKING_STATE_IDLE) {
						vcmd_delink_rm_cmdbuf(subsys_core, new_cmdbuf_node);
						if (restart_cmdbuf == cmdbuf_obj_temp)
							restart_cmdbuf = next_cmdbuf_node ?
								next_cmdbuf_node->data : NULL;
						if (restart_cmdbuf) {
							PDEBUG("Set restart cmdbuf [%d].\n",
								restart_cmdbuf->cmdbuf_id);
						} else {
							PDEBUG("Set restart cmdbuf to NULL.\n");
						}
					} else if (cmdbuf_obj_temp->cmdbuf_data_linked == 1 &&
						subsys_core->working_state == WORKING_STATE_WORKING) {
						bi_list_node *last_cmdbuf_node = NULL;
						bi_list_node *done_cmdbuf_node = NULL;
						int abort_cmdbuf_id;
						int loop_count = 0;

						//abort the vcmd and wait
						PDEBUG("Abort due to linked cmdbuf %d of current process.\n",
							cmdbuf_obj_temp->cmdbuf_id);

						// disable abort interrupt
						vcmd_write_register_value((const void *)subsys_core->hwregs,
							subsys_core->reg_mirror, HWIF_VCMD_START_TRIGGER, 0);
						vcmd_aborted = 1;
						subsys_dev->software_triger_abort = 1;

						// Wait vcmd core aborted and vcmd enters IDLE mode.
						while (vcmd_get_register_value((const void *)subsys_core->hwregs,
							subsys_core->reg_mirror, HWIF_VCMD_WORK_STATE)) {
							loop_count++;
							if (!(loop_count % 10)) {
								u32 irq_status = vcmd_read_reg((const void *)subsys_core->hwregs,
									VCMD_REGISTER_INT_STATUS_OFFSET);
								pr_err("ftv310_vpu_vcmd: expected idle state, but irq status = 0x%0x\n",
								       irq_status);
								pr_err("ftv310_vpu_vcmd: vcmd current status is %d\n",
										vcmd_get_register_value((const void *)subsys_core->hwregs,
											subsys_core->reg_mirror, HWIF_VCMD_WORK_STATE));
							}
							mdelay(10);  // wait 10ms
							// Expand the loop_count to ensure hardware idle and update sw working status to avoid precess hang next time
							if (loop_count > 300) {
								pr_err("ftv310_vpu_vcmd: too long before vcmd core to IDLE state\n");
								process_manager_node = get_process_manager_node(filp, subsys_dev);
								if (process_manager_node) {
									spin_lock_irqsave(&subsys_dev->vcmd_process_manager_lock, flags);
									bi_list_remove_node(&subsys_dev->global_process_manager, process_manager_node);
									free_process_manager_node(process_manager_node);
									spin_unlock_irqrestore(&subsys_dev->vcmd_process_manager_lock, flags);
								}
								spin_unlock_irqrestore(&subsys_core->spinlock, flags);
								up(&subsys_dev->vcmd_reserve_cmdbuf_sem[subsys_dev->vcmd_core->vcmd_core_cfg.sub_module_type]);
								subsys_core->working_state = WORKING_STATE_IDLE;
								return -ERESTARTSYS;
							}
						}
						subsys_core->working_state = WORKING_STATE_IDLE;
						// clear interrupt & restore
						subsys_core->reg_mirror[VCMD_REGISTER_INT_STATUS_OFFSET / 4] =
								vcmd_read_reg((const void *)subsys_core->hwregs, VCMD_REGISTER_INT_STATUS_OFFSET);
						if (subsys_core->reg_mirror[VCMD_REGISTER_INT_STATUS_OFFSET / 4]) {
							PDEBUG("Abort interrupt triggered, now clear all interrupts to avoid int...\n");

							vcmd_write_reg((const void *)(subsys_core->hwregs),
								VCMD_REGISTER_INT_STATUS_OFFSET,
								subsys_core->reg_mirror[VCMD_REGISTER_INT_STATUS_OFFSET / 4]);

							PDEBUG("Now irq status = 0x%08x.\n",
								vcmd_read_reg((const void *)subsys_core->hwregs, VCMD_REGISTER_INT_STATUS_OFFSET));
						}

						abort_cmdbuf_id = vcmd_get_register_value((const void *)subsys_core->hwregs,
								subsys_core->reg_mirror, HWIF_VCMD_CMDBUF_EXECUTING_ID);
						PDEBUG("Abort when executing cmd buf %d.\n", abort_cmdbuf_id);

						{
							trace_vcmd_trigger_abort(current->pid, cmdbuf_obj_temp->cmdbuf_id, abort_cmdbuf_id, subsys_core->sw_cmdbuf_rdy_num, 
									*(subsys_core->vcmd_reg_mem_virtualAddress +VCMD_EXE_CMDBUF_COUNT));
						}

						subsys_core->sw_cmdbuf_rdy_num = 0;
						subsys_core->duration_without_int = 0;
						vcmd_write_register_value((const void *)subsys_core->hwregs, subsys_core->reg_mirror,
							HWIF_VCMD_EXE_CMDBUF_COUNT, 0);
						vcmd_write_register_value((const void *)subsys_core->hwregs, subsys_core->reg_mirror,
							HWIF_VCMD_RDY_CMDBUF_COUNT, 0);

						/* Mark cmdbuf_run_done to 1 for all the cmd buf executed. */
						done_cmdbuf_node = subsys_core->list_manager.head;
						while (done_cmdbuf_node) {
							if (!((struct cmdbuf_obj *)done_cmdbuf_node->data)->cmdbuf_run_done) {
								((struct cmdbuf_obj *)done_cmdbuf_node->data)->cmdbuf_run_done = 1;
								((struct cmdbuf_obj *)done_cmdbuf_node->data)->cmdbuf_data_linked = 0;
								PDEBUG("Set cmdbuf [%d] cmdbuf_run_done to 1.\n",
									((struct cmdbuf_obj *)done_cmdbuf_node->data)->cmdbuf_id);
							}
							if (((struct cmdbuf_obj *)done_cmdbuf_node->data)->cmdbuf_id == abort_cmdbuf_id)
								break;
							done_cmdbuf_node = done_cmdbuf_node->next;
						}
						if (cmdbuf_obj_temp->cmdbuf_run_done) {
							/* current cmdbuf is in fact has been executed, but due to interrupt is not triggered,
							 * the status is not updated.
							 * Just delink and remove it from the list.
							 */
							if (done_cmdbuf_node && done_cmdbuf_node->data) {
								PDEBUG("done_cmdbuf_node is cmdbuf [%d].\n",
									((struct cmdbuf_obj *)done_cmdbuf_node->data)->cmdbuf_id);
							}
							if (done_cmdbuf_node)
								done_cmdbuf_node = done_cmdbuf_node->next;
							if (done_cmdbuf_node)
								restart_cmdbuf = (struct cmdbuf_obj *)done_cmdbuf_node->data;
							if (restart_cmdbuf)
								PDEBUG("Set restart cmdbuf [%d] via if.\n", restart_cmdbuf->cmdbuf_id);
						} else {
							last_cmdbuf_node = new_cmdbuf_node;
							/* cmd buf num from aborted cmd buf to current cmdbuf_obj_temp */
							if (cmdbuf_obj_temp->cmdbuf_id != abort_cmdbuf_id) {
								last_cmdbuf_node  = new_cmdbuf_node->previous;

								while (last_cmdbuf_node &&
									((struct cmdbuf_obj *)last_cmdbuf_node->data)->cmdbuf_id != abort_cmdbuf_id) {
									restart_cmdbuf = (struct cmdbuf_obj *)last_cmdbuf_node->data;
									last_cmdbuf_node = last_cmdbuf_node->previous;
									subsys_core->sw_cmdbuf_rdy_num++;
									subsys_core->duration_without_int += restart_cmdbuf->executing_time;
									PDEBUG("Keep valid cmdbuf [%d] in the list.\n", restart_cmdbuf->cmdbuf_id);
								}
							}
							if (restart_cmdbuf)
								PDEBUG("Set restart cmdbuf [%d] via else.\n", restart_cmdbuf->cmdbuf_id);
						}
						// remove first linked cmdbuf from list
						vcmd_delink_rm_cmdbuf(subsys_core, new_cmdbuf_node);
					}
					subsys_dev->software_triger_abort = 0;
					release_cmdbuf_num++;
					PDEBUG("release reserved cmdbuf\n");
				} else if (vcmd_aborted && !cmdbuf_obj_temp->cmdbuf_run_done) {
				/* VCMD is aborted, need to re-calculate the duration_without_int */
					if (!restart_cmdbuf)
						restart_cmdbuf = cmdbuf_obj_temp; /* first cmdbuf to be restarted */
					subsys_core->duration_without_int += cmdbuf_obj_temp->executing_time;
					subsys_core->sw_cmdbuf_rdy_num++;
				}
				new_cmdbuf_node = next_cmdbuf_node;
			}

			if (restart_cmdbuf && restart_cmdbuf->core_id == core_id) {
				u32 irq_status1, irq_status2;
				u64 restart_cmd_buf_addr;

				PDEBUG("Restart from cmdbuf [%d] after aborting.\n", restart_cmdbuf->cmdbuf_id);

				irq_status1 = vcmd_read_reg((const void *)subsys_core->hwregs, VCMD_REGISTER_INT_STATUS_OFFSET);

				vcmd_write_reg((const void *)subsys_core->hwregs, VCMD_REGISTER_INT_STATUS_OFFSET, irq_status1);

				irq_status2 = vcmd_read_reg((const void *)subsys_core->hwregs, VCMD_REGISTER_INT_STATUS_OFFSET);

				PDEBUG("Clear irq status from 0x%0x -> 0x%0x\n", irq_status1, irq_status2);

				restart_cmd_buf_addr = (u64)(restart_cmdbuf->cmdbuf_busAddress - subsys_dev->base_ddr_addr);
#ifdef HAS_MMU
				restart_cmd_buf_addr = (u64)(restart_cmdbuf->mmu_cmdbuf_bus_address);
#endif
				vcmd_write_register_value((const void *)subsys_core->hwregs, subsys_core->reg_mirror,
					HWIF_VCMD_EXECUTING_CMD_ADDR, (u32)(restart_cmd_buf_addr));
				if (sizeof(size_t) == 8) {
					vcmd_write_register_value((const void *)subsys_core->hwregs,
						subsys_core->reg_mirror, HWIF_VCMD_EXECUTING_CMD_ADDR_MSB,
						(u32)((u64)(restart_cmd_buf_addr) >> 32));
				} else {
					vcmd_write_register_value((const void *)subsys_core->hwregs,
						subsys_core->reg_mirror, HWIF_VCMD_EXECUTING_CMD_ADDR_MSB, 0);
				}
				vcmd_write_register_value((const void *)subsys_core->hwregs, subsys_core->reg_mirror,
					HWIF_VCMD_EXE_CMDBUF_COUNT, 0);

				vcmd_write_register_value((const void *)subsys_core->hwregs, subsys_core->reg_mirror,
					HWIF_VCMD_EXE_CMDBUF_LENGTH, (u32)((restart_cmdbuf->cmdbuf_size + 7) / 8));

				vcmd_write_register_value((const void *)subsys_core->hwregs, subsys_core->reg_mirror,
					HWIF_VCMD_CMDBUF_EXECUTING_ID, restart_cmdbuf->cmdbuf_id);

				PDEBUG("====subsys_core->sw_cmdbuf_rdy_num is %d\n", subsys_core->sw_cmdbuf_rdy_num);
				vcmd_write_register_value((const void *)subsys_core->hwregs, subsys_core->reg_mirror,
					HWIF_VCMD_RDY_CMDBUF_COUNT, subsys_dev->vcmd_core->sw_cmdbuf_rdy_num);

	#ifdef VCMD_DEBUG_INTERNAL
				printk_vcmd_register_debug((const void *)subsys_core->hwregs, "before restart");
	#endif
				vcmd_write_register_value((const void *)subsys_core->hwregs, subsys_core->reg_mirror,
					HWIF_VCMD_START_TRIGGER, 1);

				PDEBUG("Restart from cmdbuf [%d] after aborting: start trigger = %d.\n", restart_cmdbuf->cmdbuf_id,
				vcmd_get_register_value((const void *)subsys_core->hwregs, subsys_core->reg_mirror,
				HWIF_VCMD_START_TRIGGER));
				PDEBUG("dev state from %d -> WORKING.\n", subsys_core->working_state);
				subsys_core->working_state = WORKING_STATE_WORKING;
	#ifdef VCMD_DEBUG_INTERNAL
				printk_vcmd_register_debug((const void *)subsys_core->hwregs, "after restart");
	#endif
			} else
				PDEBUG("No more command buffer to be restarted!\n");
			spin_unlock_irqrestore(&subsys_core->spinlock, flags);

			// VCMD aborted but not restarted, need to wake up
			if (vcmd_aborted && !restart_cmdbuf)
				wake_up_interruptible_all(&subsys_core->wait_queue);
			subsys_core = subsys_core->core_next;// To next core in loop
		}

		if (release_cmdbuf_num)
			wake_up_interruptible_all(&subsys_dev->vcmd_cmdbuf_memory_wait);
		// remove cmdbuf reserved but not linked
		cmdbuf_used_pos = 2;

		while (1) {
			if (subsys_dev->cmdbuf_used[cmdbuf_used_pos]) {
				cmdbuf_obj_temp = (struct cmdbuf_obj *)subsys_dev->global_cmdbuf_node[cmdbuf_used_pos]->data;
				if (cmdbuf_obj_temp->filp == filp &&
					 cmdbuf_obj_temp->cmdbuf_data_linked == 0 &&
						cmdbuf_obj_temp->cmdbuf_run_done == 0) {
					cmdbuf_obj_temp = (struct cmdbuf_obj *)subsys_dev->global_cmdbuf_node[cmdbuf_used_pos]->data;
					cmdbuf_obj_temp->cmdbuf_data_linked = 1;
					cmdbuf_obj_temp->cmdbuf_run_done = 1;
					cmdbuf_obj_temp->cmdbuf_need_remove = 1;
					ret =  release_cmdbuf_notlinked(filp, subsys_dev, cmdbuf_used_pos);
				if (ret != 0)
					cmdbuf_obj_temp->process_manager_obj = NULL;
				}
			}
			cmdbuf_used_pos++;
			if (cmdbuf_used_pos >= TOTAL_DISCRETE_CMDBUF_NUM)
				break;
		}

		process_manager_node = get_process_manager_node(filp, subsys_dev);
		if (process_manager_node) {
			//remove node from list
			PDEBUG("process node %p for filp to be removed: %p\n",
				(void *)process_manager_node, (void *)filp);
			spin_lock_irqsave(&subsys_dev->vcmd_process_manager_lock, flags);
			bi_list_remove_node(&subsys_dev->global_process_manager, process_manager_node);
			free_process_manager_node(process_manager_node);
			spin_unlock_irqrestore(&subsys_dev->vcmd_process_manager_lock, flags);
		}
		up(&subsys_dev->vcmd_reserve_cmdbuf_sem[subsys_dev->vcmd_core->vcmd_core_cfg.sub_module_type]);
	}
	ftv310_vpu_clear_fence(filp);

	return 0;
}

#ifdef VCMD_DEBUG_INTERNAL
static void printk_vcmd_register_debug(const void *hwregs, char *info)
{
	u32 i, fordebug;

	for (i = 0; i < ASIC_VCMD_SWREG_AMOUNT; i++) {
		fordebug = vcmd_read_reg((const void *)hwregs, i * 4);
		pr_info("%s vcmd register %d:0x%x\n", info, i, fordebug);
	}
}
#endif

static void vcmd_set_base_addr(vcmd_dev_str *subsys_dev, unsigned long base_reg_hw, unsigned long base_ddr_hw)
{
	int i = 0;
	/* Base register address Length */
	vcmd_core_str *subsys_core;

	subsys_core = subsys_dev->vcmd_core;
	for (i = 0; i < subsys_dev->subsys_num; i++) {
		subsys_core->vcmd_core_cfg.vcmd_base_addr =
			base_reg_hw +
			subsys_core->vcmd_core_cfg.vcmd_base_addr;
		subsys_core = subsys_core->core_next;
	}
	subsys_dev->base_ddr_addr = base_ddr_hw;
	subsys_dev->base_reg_addr = base_reg_hw;
}

#ifdef PHY_FPGA_MEM
static int init_vcmd_pool_on_ftv310_vpu_fpgamem(vcmd_dev_str *subsys_dev)
{
	struct noncache_mem *mem_temp;
	unsigned long vcmd_base_ddr_hw = 0;

	vcmd_base_ddr_hw = get_vcmd_buffer_pool_start_addr(subsys_dev);
	mem_temp = kzalloc(sizeof(*mem_temp), GFP_KERNEL);
	subsys_dev->vcmd_buf_mem_pool = mem_temp;
	subsys_dev->vcmd_buf_mem_pool->busAddress = vcmd_base_ddr_hw;
	subsys_dev->vcmd_buf_mem_pool->size = CMDBUF_POOL_TOTAL_SIZE;
	if (!request_mem_region(subsys_dev->vcmd_buf_mem_pool->busAddress,
				subsys_dev->vcmd_buf_mem_pool->size,
				"ftv310_vpu_vcmd")) {
		pr_info("Init: failed to request hw region.\n");
		return -1;
	}
	pr_info("Init: vcmd_buf_mem_pool.busAddress=0x%llx.\n",
		(unsigned long long)subsys_dev->vcmd_buf_mem_pool->busAddress);

	subsys_dev->vcmd_buf_mem_pool->virtualAddress =
		(u32 *)ioremap(subsys_dev->vcmd_buf_mem_pool->busAddress,
			       subsys_dev->vcmd_buf_mem_pool->size);
	if (!subsys_dev->vcmd_buf_mem_pool->virtualAddress) {
		pr_info("Init: failed to ioremap.\n");
		return -1;
	}
	memset(subsys_dev->vcmd_buf_mem_pool->virtualAddress, 0,
		subsys_dev->vcmd_buf_mem_pool->size);
	pr_info("Init: vcmd_buf_mem_pool.virtualAddress=0x%llx.\n",
		(unsigned long long)
			subsys_dev->vcmd_buf_mem_pool->virtualAddress);

	mem_temp = kzalloc(sizeof(*mem_temp), GFP_KERNEL);
	subsys_dev->vcmd_status_buf_mem_pool = mem_temp;
	subsys_dev->vcmd_status_buf_mem_pool->busAddress =
		vcmd_base_ddr_hw + CMDBUF_POOL_TOTAL_SIZE;
	subsys_dev->vcmd_status_buf_mem_pool->size = CMDBUF_POOL_TOTAL_SIZE;
	if (!request_mem_region(subsys_dev->vcmd_status_buf_mem_pool->busAddress,
				subsys_dev->vcmd_status_buf_mem_pool->size,
				"ftv310_vpu_vcmd")) {
		pr_info("Init: failed to request hw region.\n");
		return -1;
	}
	pr_info("Init: vcmd_status_buf_mem_pool.busAddress=0x%llx.\n",
		(unsigned long long)
			subsys_dev->vcmd_status_buf_mem_pool->busAddress);
	subsys_dev->vcmd_status_buf_mem_pool->virtualAddress =
		(u32 *)ioremap(subsys_dev->vcmd_status_buf_mem_pool->busAddress,
			       subsys_dev->vcmd_status_buf_mem_pool->size);

	if (!subsys_dev->vcmd_status_buf_mem_pool->virtualAddress) {
		pr_info("Init: failed to ioremap.\n");
		return -1;
	}
	pr_info("Init: vcmd_status_buf_mem_pool.virtualAddress=0x%llx.\n",
		(unsigned long long)
			subsys_dev->vcmd_status_buf_mem_pool->virtualAddress);
	memset(subsys_dev->vcmd_status_buf_mem_pool->virtualAddress, 0,
		subsys_dev->vcmd_status_buf_mem_pool->size);

	mem_temp = kzalloc(sizeof(*mem_temp), GFP_KERNEL);
	subsys_dev->vcmd_registers_mem_pool = mem_temp;
	subsys_dev->vcmd_registers_mem_pool->busAddress =
		vcmd_base_ddr_hw + CMDBUF_POOL_TOTAL_SIZE * 2;
	subsys_dev->vcmd_registers_mem_pool->size =
		CMDBUF_VCMD_REGISTER_TOTAL_SIZE;
	if (!request_mem_region(subsys_dev->vcmd_registers_mem_pool->busAddress,
				subsys_dev->vcmd_registers_mem_pool->size,
				"ftv310_vpu_vcmd")) {
		pr_info("Init: failed to request hw region.\n");
		return -1;
	}
	pr_info("Init: vcmd_registers_mem_pool.busAddress=0x%llx.\n",
		(unsigned long long)
			subsys_dev->vcmd_registers_mem_pool->busAddress);
	subsys_dev->vcmd_registers_mem_pool->virtualAddress =
		(u32 *)ioremap(subsys_dev->vcmd_registers_mem_pool->busAddress,
			       subsys_dev->vcmd_registers_mem_pool->size);

	if (!subsys_dev->vcmd_registers_mem_pool->virtualAddress) {
		pr_info("Init: failed to ioremap.\n");
		return -1;
	}
	pr_info("Init: vcmd_registers_mem_pool.virtualAddress=0x%llx.\n",
		(unsigned long long)
			subsys_dev->vcmd_registers_mem_pool->virtualAddress);
	memset(subsys_dev->vcmd_registers_mem_pool->virtualAddress, 0,
		subsys_dev->vcmd_registers_mem_pool->size);
	return 0;
}

static void release_vcmd_pool_on_ftv310_vpu_fpgamem(vcmd_dev_str *subsys_dev)
{
	iounmap((void *)subsys_dev->vcmd_buf_mem_pool->virtualAddress);
	release_mem_region(subsys_dev->vcmd_buf_mem_pool->busAddress,
			   subsys_dev->vcmd_buf_mem_pool->size);
	iounmap((void *)subsys_dev->vcmd_status_buf_mem_pool->virtualAddress);
	release_mem_region(subsys_dev->vcmd_status_buf_mem_pool->busAddress,
			   subsys_dev->vcmd_status_buf_mem_pool->size);
	iounmap((void *)subsys_dev->vcmd_registers_mem_pool->virtualAddress);
	release_mem_region(subsys_dev->vcmd_registers_mem_pool->busAddress,
			   subsys_dev->vcmd_registers_mem_pool->size);
}

static unsigned long get_next_slice_reserved_mem_start_addr(void)
{
	unsigned long vcmd_base_ddr_hw = 0;

	vcmd_base_ddr_hw += get_vcmd_pool_size();

	return vcmd_base_ddr_hw;
}

static unsigned long get_vcmd_buffer_pool_start_addr(vcmd_dev_str *subsys_dev)
{
#define INVALID_SLICE_IDX (0xffffffff)
// different subsys_dev type has itself vcmd pool memory region in one slice
// different slice has different vcmd pool memory
// one slice share same mmu pgtble
	static unsigned long vcmd_base_ddr_hw = 0;
	static u32 cur_sliceid = INVALID_SLICE_IDX;
	static int pool_cont = 0;
	if (cur_sliceid != subsys_dev->sliceidx) {
		if (vcmd_base_ddr_hw != 0)
			vcmd_base_ddr_hw += get_vcmd_pool_size();
		else
			vcmd_base_ddr_hw = subsys_dev->base_ddr_addr;

#ifdef HAS_MMU
		vcmd_base_ddr_hw += get_mmu_pgtbl_buf_size();
#endif
		cur_sliceid = subsys_dev->sliceidx;
		pool_cont++;
	} else {
		vcmd_base_ddr_hw += get_vcmd_pool_size();
		pool_cont++;
	}
	if (pool_cont > VCMD_POOL_CNT)
		pr_err("error: wronly config vcmd pool, init pool count is %d, but preset VCMD_POOL_CNT is %d\n",
			pool_cont, VCMD_POOL_CNT);

	return vcmd_base_ddr_hw;
}

#else
//maybe need customer implment this function based on own environment
static int customized_init_vcmd_mem_pool(vcmd_dev_str *subsys_dev)
{
	dma_addr_t dma_handle;
	struct noncache_mem *mem_temp;
	struct ftv310_vpu_fpga_t *fpga = &g_fpga;

	mem_temp = kzalloc(sizeof(*mem_temp), GFP_KERNEL);
	if (fpga->vcmd_base[0]) {
		if (subsys_dev->vcmd_core->type == FTV310_VPU_CORE_DEC) {
			mem_temp->virtualAddress = fpga->virt_pool_addr + (fpga->vcmd_base[0] - fpga->ddr_base);
			mem_temp->busAddress = (uint64_t)fpga->vcmd_base[0];
		} else {
			mem_temp->virtualAddress = fpga->virt_pool_addr + (fpga->vcmd_base[1] - fpga->ddr_base);
			mem_temp->busAddress = (uint64_t)fpga->vcmd_base[1];
		}
	} else {
		mem_temp->virtualAddress = dma_alloc_coherent(subsys_dev->dev,
			CMDBUF_POOL_TOTAL_SIZE + CMDBUF_POOL_TOTAL_SIZE +
			CMDBUF_VCMD_REGISTER_TOTAL_SIZE,
			&dma_handle, GFP_KERNEL | GFP_DMA);
		if (!mem_temp->virtualAddress)
			return -1;
		mem_temp->busAddress = (uint64_t)dma_handle;
	}

	mem_temp->size = CMDBUF_POOL_TOTAL_SIZE;
	subsys_dev->vcmd_buf_mem_pool = mem_temp;

	mem_temp = kzalloc(sizeof(*mem_temp), GFP_KERNEL);
	mem_temp->virtualAddress =
		subsys_dev->vcmd_buf_mem_pool->virtualAddress +
		CMDBUF_POOL_TOTAL_SIZE / 4;
	mem_temp->busAddress =
		(uint64_t)(subsys_dev->vcmd_buf_mem_pool->busAddress) +
		CMDBUF_POOL_TOTAL_SIZE;
	mem_temp->size = CMDBUF_POOL_TOTAL_SIZE;
	subsys_dev->vcmd_status_buf_mem_pool = mem_temp;

	mem_temp = kzalloc(sizeof(*mem_temp), GFP_KERNEL);
	mem_temp->virtualAddress =
		subsys_dev->vcmd_status_buf_mem_pool->virtualAddress +
		CMDBUF_POOL_TOTAL_SIZE / 4;
	mem_temp->busAddress =
		(uint64_t)(subsys_dev->vcmd_status_buf_mem_pool->busAddress) +
		CMDBUF_POOL_TOTAL_SIZE;
	mem_temp->size = CMDBUF_VCMD_REGISTER_TOTAL_SIZE;
	subsys_dev->vcmd_registers_mem_pool = mem_temp;
	subsys_dev->base_ddr_addr = 0;
	subsys_dev->base_reg_addr = 0;
	return 0;
}

static void customized_release_vcmd_mem_pool(vcmd_dev_str *subsys_dev)
{
	struct ftv310_vpu_fpga_t *fpga = &g_fpga;

	if (!fpga->vcmd_base[0]) {
		dma_addr_t dma_handle;

		dma_handle = (dma_addr_t)subsys_dev->vcmd_buf_mem_pool->busAddress;
		dma_free_coherent(subsys_dev->dev,
			CMDBUF_POOL_TOTAL_SIZE + CMDBUF_POOL_TOTAL_SIZE +
			CMDBUF_VCMD_REGISTER_TOTAL_SIZE,
			(void *)subsys_dev->vcmd_buf_mem_pool->virtualAddress,
			dma_handle);
	}
}

unsigned long get_vcmd_pool_size(void)
{
	return HLINA_TRANSL_VCMD_SIZE;
}

unsigned long get_total_vcmd_pool_size(void)
{
	return get_vcmd_pool_size() * VCMD_POOL_CNT;
}

#endif

#ifdef HAS_MMU

static void get_vcmd_pool_mmuaddr(vcmd_dev_str *subsys_dev)
{
	u32 id = 0;
	struct mmu_addr_desc addr = {0};
	unsigned long base_ddr_addr;
#ifdef PHY_FPGA_MEM
	base_ddr_addr = subsys_dev->base_ddr_addr;
#else
	base_ddr_addr = 0;
#endif

	if (subsys_dev->vcmd_core->type == FTV310_VPU_CORE_DEC)
		id = NODE_TYPE_DEC;
	else if (subsys_dev->vcmd_core->type == FTV310_VPU_CORE_ENC)
		id = NODE_TYPE_ENC;
	addr.id = (subsys_dev->sliceidx << 16) | (id << 8);
	addr.virtual_address = 0;
	addr.bus_address = subsys_dev->vcmd_buf_mem_pool->busAddress - base_ddr_addr;
	addr.size = subsys_dev->vcmd_buf_mem_pool->size;
	ftv310_vpu_mmu_map(&addr, 1);
	subsys_dev->vcmd_buf_mem_pool->mmu_bus_address = addr.bus_address;

	addr.virtual_address = 0;
	addr.bus_address = subsys_dev->vcmd_status_buf_mem_pool->busAddress - base_ddr_addr;
	addr.size = subsys_dev->vcmd_status_buf_mem_pool->size;
	ftv310_vpu_mmu_map(&addr, 1);
	subsys_dev->vcmd_status_buf_mem_pool->mmu_bus_address = addr.bus_address;

	addr.virtual_address = 0;
	addr.bus_address = subsys_dev->vcmd_registers_mem_pool->busAddress - base_ddr_addr;
	addr.size = subsys_dev->vcmd_registers_mem_pool->size;
	ftv310_vpu_mmu_map(&addr, 1);
	subsys_dev->vcmd_registers_mem_pool->mmu_bus_address = addr.bus_address;
}
#endif

#ifdef PHY_CONFIG_PM
/**
 * @brief abort a specified vcmd hw device.
 */
static int vcmd_abort(vcmd_core_str *subsys_core)
{
	unsigned long flags = 0;

	spin_lock_irqsave(&subsys_core->spinlock, flags);
	vcmd_write_register_value((const void *)subsys_core->hwregs,
								subsys_core->reg_mirror,
								HWIF_VCMD_START_TRIGGER, 0);
	spin_unlock_irqrestore(&subsys_core->spinlock, flags);
	if (vcmd_isr_polling == 0) {
		if (wait_event_interruptible(subsys_core->wait_abort_queue,
						(subsys_core->working_state == WORKING_STATE_IDLE))) {
			pr_err("%s: wait_abort_queue is signaled!!!\n", __func__);
			return -ERESTARTSYS;
		}
	} else {
		u32 irq, cnt = 100000;

		irq = (subsys_core->vcmd_core_cfg.vcmd_irq == -1) ?
				subsys_core->core_id : subsys_core->vcmd_core_cfg.vcmd_irq;

		while (cnt--) {
			usleep_range(100, 120);
			ftv310_vpu_vcmd_isr(irq, subsys_core);
			if (subsys_core->working_state == WORKING_STATE_IDLE) {
				cnt += 1;
				break;
			}
		}
		if (cnt == 0) {
			pr_err("%s: can't wait aborted!!!\n", __func__);
			return -ERESTARTSYS;
		}
	}

	return 0;
}

/**
 * @brief suspend for vcmd driver power management
 */
int vcmd_pm_suspend(void *_dev)
{
	vcmd_dev_str *subsys_dev = (vcmd_dev_str *)_dev;
	vcmd_core_str *subsys_core = subsys_dev->vcmd_core;
	int i;
	unsigned long flags;

	for (i = 0; i < subsys_dev->subsys_num; i++) {
		if (!subsys_core)
			continue;
		spin_lock_irqsave(&subsys_core->spinlock, flags);
		subsys_dev->software_triger_abort = 1;
		if (subsys_core->working_state == WORKING_STATE_WORKING) {
			spin_unlock_irqrestore(&subsys_core->spinlock, flags);
			vcmd_abort(subsys_core);
			if (subsys_core->working_state != WORKING_STATE_IDLE) {
				subsys_dev->software_triger_abort = 0;
				pr_err("suspend failed for dev [%d].", subsys_core->core_id);
				return -EBUSY;
			}
		} else {
			spin_unlock_irqrestore(&subsys_core->spinlock, flags);
		}
		subsys_dev->software_triger_abort = 0;
		subsys_core = subsys_core->core_next;
	}
	return 0;
}

/**
 * @brief resume for vcmd driver power management
 */
int vcmd_pm_resume(void *_dev)
{
	vcmd_dev_str *subsys_dev = (vcmd_dev_str *)_dev;
	vcmd_core_str *subsys_core = subsys_dev->vcmd_core;
	int ret = 0;
	bi_list_node *node;
	int i;
	unsigned long flags;

	for (i = 0; i < subsys_dev->subsys_num; i++) {
		if (!subsys_core)
			continue;
		spin_lock_irqsave(&subsys_core->spinlock, flags);
		if (subsys_core->working_state == WORKING_STATE_IDLE) {
			node = subsys_core->list_manager.head;
			while (node && ((struct cmdbuf_obj *)node->data)->cmdbuf_run_done)
				node = node->next;
			vcmd_link_cmdbuf(subsys_core, node);
			if (node && subsys_core->sw_cmdbuf_rdy_num != 0)
				vcmd_start(subsys_core, node);
		}
		spin_unlock_irqrestore(&subsys_core->spinlock, flags);
		subsys_core = subsys_core->core_next;
	}

	ret = mmu_pm_resume(subsys_dev->sliceidx, subsys_dev->vcmd_core->type);

	return ret;
}
#endif

static void ensure_vcmd_submodule_addr(struct vcmd_config *vcmd_cfg_p)
{
#ifndef HAS_AXIFE
	vcmd_cfg_p->submodule_axife_addr[0] = 0xffff;
	vcmd_cfg_p->submodule_axife_addr[1] = 0xffff;
#endif

#ifndef HAS_CACHECORE
	vcmd_cfg_p->submodule_L2Cache_addr = 0xffff;
#endif

#ifndef HAS_DEC400
	vcmd_cfg_p->submodule_dec400_addr = 0xffff;
#endif

#ifndef HAS_MMU
	vcmd_cfg_p->submodule_MMU_addr = 0xffff;
	vcmd_cfg_p->submodule_MMUWrite_addr = 0xffff;
#endif

}
