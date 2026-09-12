// SPDX-License-Identifier: GPL-2.0
/*
 *    ftv310 decoder hardware driver.
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

#include "ftv310_vpu_dec.h"
#include "dwl_defs.h"
#include "ftv310_vpu_axife.h"
#include "ftv310_vpu_cache.h"
#include <linux/io.h>
#include <linux/uaccess.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/pci.h>
#include <linux/sched.h>
#include <linux/semaphore.h>
#include <linux/spinlock.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/wait.h>
#include <linux/timer.h>
#include <linux/clk.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include "ipoffset/vcd_offset.h"
/*ftv310_vpu G1 regs config including dec and pp*/
#define FTV310_VPU_DEC_ORG_REGS 60
#define FTV310_VPU_PP_ORG_REGS 41

#define FTV310_VPU_DEC_EXT_REGS 27
#define FTV310_VPU_PP_EXT_REGS 9

#define FTV310_VPU_G1_DEC_TOTAL_REGS (FTV310_VPU_DEC_ORG_REGS + FTV310_VPU_DEC_EXT_REGS)
#define FTV310_VPU_PP_TOTAL_REGS (FTV310_VPU_PP_ORG_REGS + FTV310_VPU_PP_EXT_REGS)

#define FTV310_VPU_DEC_ORG_FIRST_REG 0
#define FTV310_VPU_DEC_ORG_LAST_REG 59
#define FTV310_VPU_DEC_EXT_FIRST_REG 119
#define FTV310_VPU_DEC_EXT_LAST_REG 145

#define FTV310_VPU_PP_ORG_FIRST_REG 60
#define FTV310_VPU_PP_ORG_LAST_REG 100
#define FTV310_VPU_PP_EXT_FIRST_REG 146
#define FTV310_VPU_PP_EXT_LAST_REG 154

/*ftv310_vpu G2 reg config*/

#define FTV310_VPU_G2_DEC_FIRST_REG 0
#define FTV310_VPU_G2_DEC_LAST_REG (FTV310_VPU_G2_DEC_REGS - 1)

/* ftv310_vpu FTV310D reg config */
#define FTV310_VPU_VCD_FIRST_REG 0
#define FTV310_VPU_VCD_LAST_REG (FTV310_VPU_FTV310D_REGS - 1)
#define FTV310_VPUDEC_HWBUILD_ID_OFF (309 * 4)

#define FTV310_VPU_FTV310D_REG_BWREAD 300
#define FTV310_VPU_FTV310D_REG_BWWRITE 301
#define FTV310D_BURSTWIDTH 16

/********************************************************************
 *                                              PORTING SEGMENT
 * NOTES: customer should modify these configuration if do porting to own
 * platform. Please guarantee the base_addr, io_size,dec_irq belong to
 * same core.
 ********************************************************************/

#define FTV310D_MAX_CORES 8

#define DEC_IO_SIZE_0 DEC_IO_SIZE_MAX /* bytes */
#define DEC_IO_SIZE_1 DEC_IO_SIZE_MAX /* bytes */

#define DEC_IRQ_0 -1
#define DEC_IRQ_1 -1

/***********************************************************************/

#define IS_G1(hw_id) (((hw_id) == 0x6731) ? 1 : 0)
#define IS_G2(hw_id) (((hw_id) == 0x6732) ? 1 : 0)
#define IS_FTV310D(hw_id) (((hw_id) == 0x8001) ? 1 : 0)
#define IS_VCD(hw_id)                                                          \
	((((hw_id) == 0x8001) ? 1 : 0) || (((hw_id) == 0x9001) ? 1 : 0))
#define IS_VCD_1_3(hw_id) (((hw_id) == 0x90011030) ? 1 : 0)
#define IS_VCD_1_1(hw_id) (((hw_id) == 0x90011010) ? 1 : 0)
static const int DecHwId[] = { 0x6731, /* G1 */
			       0x6732, /* G2 */
			       0x8001, 0x9001 };

#ifndef USE_DTB_PROBE

static int irq[FTV310D_MAX_CORES] = { DEC_IRQ_0, DEC_IRQ_1, DEC_IRQ_0, DEC_IRQ_1,
			    DEC_IRQ_0, DEC_IRQ_1, DEC_IRQ_0, DEC_IRQ_1 };

static unsigned int iosize[FTV310D_MAX_CORES] = { DEC_IO_SIZE_0, DEC_IO_SIZE_1,
						DEC_IO_SIZE_0, DEC_IO_SIZE_1,
						DEC_IO_SIZE_0, DEC_IO_SIZE_1,
						DEC_IO_SIZE_0, DEC_IO_SIZE_1 };

/*slice idx must be in sequence, else add dec node
 *will fail: only for no DTB probe mode
 */
static int sliceidxtable[FTV310D_MAX_CORES] = { 0, 0, 1, 1, 2, 2, 3, 3 };

#ifdef PHY_CONFIG_PM
/* shadow_reg used to restore or store regs */
static u32 shadow_dec_regs[FTV310D_MAX_CORES][DEC_IO_SIZE_MAX / 4];
#endif

#endif
//static int elements = 2;
static int bdecprobed;

#ifdef ENABLE_FTV310_VPU_CLK
static struct clk *ftv310_vpu_clk_g1;
static struct clk *ftv310_vpu_clk_g2;
static struct clk *ftv310_vpu_clk_bus;
#endif
static int ftv310_vpu_dbg = -1;
#undef PDEBUG
#define PDEBUG(fmt, arg...)                                                    \
	do {                                                                   \
		if (ftv310_vpu_dbg > 0)                                            \
			pr_info(fmt, ##arg);                                   \
	} while (0)

/* here's all the must remember stuff */
struct ftv310_vpu_dec_ctrl {
	int cores;
};

static int ReserveIO(struct ftv310_vpu_dec_t *core, struct ftv310_vpu_dec_t **auxcore);
static void ReleaseIO(struct ftv310_vpu_dec_t *);

static void ResetAsic(struct ftv310_vpu_dec_t *dev);

#ifdef FTV310_VPUDEC_DEBUG
static void dump_regs(struct ftv310_vpu_dec_t *dev);
#endif

/* IRQ handler */
static irqreturn_t ftv310_vpu_dec_isr(int irq, void *dev_id);

atomic_t irq_rx = ATOMIC_INIT(0);
atomic_t irq_tx = ATOMIC_INIT(0);
/* spinlock_t owner_lock = SPIN_LOCK_UNLOCKED; */

#define DWL_CLIENT_TYPE_H264_DEC 1U
#define DWL_CLIENT_TYPE_MPEG4_DEC 2U
#define DWL_CLIENT_TYPE_JPEG_DEC 3U
#define DWL_CLIENT_TYPE_PP 4U
#define DWL_CLIENT_TYPE_VC1_DEC 5U
#define DWL_CLIENT_TYPE_MPEG2_DEC 6U
#define DWL_CLIENT_TYPE_VP6_DEC 7U
#define DWL_CLIENT_TYPE_AVS_DEC 8U
#define DWL_CLIENT_TYPE_RV_DEC 9U
#define DWL_CLIENT_TYPE_VP8_DEC 10U
#define DWL_CLIENT_TYPE_VP9_DEC 11U
#define DWL_CLIENT_TYPE_HEVC_DEC 12U
#define DWL_CLIENT_TYPE_H264_MAIN10 15U

static u32 timeout;

#ifdef ENABLE_FTV310_VPU_CLK
static int ftv310_vpu_clk_enable(void)
{
	clk_prepare(ftv310_vpu_clk_g1);
	clk_enable(ftv310_vpu_clk_g1);
	clk_prepare(ftv310_vpu_clk_g2);
	clk_enable(ftv310_vpu_clk_g2);
	clk_prepare(ftv310_vpu_clk_bus);
	clk_enable(ftv310_vpu_clk_bus);
	return 0;
}

static int ftv310_vpu_clk_disable(void)
{
	if (ftv310_vpu_clk_g1) {
		clk_disable(ftv310_vpu_clk_g1);
		clk_unprepare(ftv310_vpu_clk_g1);
	}
	if (ftv310_vpu_clk_g2) {
		clk_disable(ftv310_vpu_clk_g2);
		clk_unprepare(ftv310_vpu_clk_g2);
	}
	if (ftv310_vpu_clk_bus) {
		clk_disable(ftv310_vpu_clk_bus);
		clk_unprepare(ftv310_vpu_clk_bus);
	}
	return 0;
}

static int ftv310_vpu_ctrlblk_reset(void)
{
	u8 *iobase;
	//config G1/G2
	ftv310_vpu_clk_enable();
	iobase = (u8 *)ioremap(BLK_CTL_BASE, 0x10000);
	iowrite32(0x3, (void *)iobase); //VPUMIX G1/G2 block soft reset
	iowrite32(0x3, (void *)iobase + 4); //VPUMIX G1/G2 clock enable
	iowrite32(0xFFFFFFFF, (void *)iobase + 0x8); //all G1 fuse dec enable
	iowrite32(0xFFFFFFFF, (void *)iobase + 0xC); //all G1 fuse pp enable
	iowrite32(0xFFFFFFFF, (void *)iobase + 0x10); //all G2 fuse dec enable
	iounmap(iobase);
	ftv310_vpu_clk_disable();

	return 0;
}
#endif

#ifdef PHY_CONFIG_PM
static long DecRestoreRegs(struct ftv310_vpu_dec_t *dev, u32 id)
{
	long i;

	/* write all regs to hardware */
	for (i = 1; i < DEC_IO_SIZE_MAX / 4; i++)
		iowrite32(shadow_dec_regs[id][i],
			(void __iomem *)(dev->hwregs + i * 4));

	return 0;
}

static long DecStoreRegs(struct ftv310_vpu_dec_t *dev, u32 id)
{
	long i;

	/* read all registers from hardware */
	for (i = 0; i < DEC_IO_SIZE_MAX / 4; i++)
		shadow_dec_regs[id][i] =
			ioread32((void __iomem *)(dev->hwregs + i * 4));

	return 0;
}

int dec_pm_suspend(void *_dev)
{
	struct ftv310_vpu_dec_t *dev = (struct ftv310_vpu_dec_t *)_dev;

	while (dev) {
		if (dev->dec_owner) {
			/* polling until hw is idle */
			while (dev->hw_active)
				usleep_range(5000, 10000);

			/* let's backup all registers from H/W to shadow register to support suspend */
			DecStoreRegs(dev, dev->core_id);
		}
		dev = dev->next;
	}

	return 0;
}

int dec_pm_resume(void *_dev)
{
	int i = 0;
	struct ftv310_vpu_dec_t *dev = (struct ftv310_vpu_dec_t *)_dev;

	while (dev) {
		if (dev->dec_owner) {
			/* let's restore registers from shadow register to H/W to support resume */
			DecRestoreRegs(dev, i);
		}
		dev = dev->next;
	}

	return 0;
}
#endif

static struct ftv310_vpu_dec_t *getcoreCtrl(u32 id)
{
	struct ftv310_vpu_dec_t *pcore;
	u32 slice = SLICE(id);
	u32 node = KCORE(id);

	pcore = get_decnodes(slice, node);
	return pcore;
}

u32 ftv310_vpu_dec_readbandwidth(int sliceidx, int isreadBW)
{
	int i, slicen = get_slicenumber();
	u32 bandwidth = 0;
	struct ftv310_vpu_dec_t *dev;
	u8 *rreg;
	u8 *wreg;

	if (sliceidx < 0) {
		for (i = 0; i < slicen; i++) {
			dev = get_decnodes(i, 0);
			while (dev) {
				rreg = dev->hwregs +
				       FTV310_VPU_FTV310D_REG_BWREAD * 4;
				wreg = dev->hwregs +
				       FTV310_VPU_FTV310D_REG_BWWRITE * 4;
				if (isreadBW)
					bandwidth += ioread32((void *)rreg);
				else
					bandwidth += ioread32((void *)wreg);
				dev = dev->next;
			}
		}
	} else {
		dev = get_decnodes(sliceidx, 0);
		while (dev) {
			rreg = dev->hwregs + FTV310_VPU_FTV310D_REG_BWREAD * 4;
			wreg = dev->hwregs + FTV310_VPU_FTV310D_REG_BWWRITE * 4;
			if (isreadBW)
				bandwidth += ioread32((void *)rreg);
			else
				bandwidth += ioread32((void *)wreg);
			dev = dev->next;
		}
	}
	return bandwidth * FTV310D_BURSTWIDTH;
}

static void ReadCoreConfig(struct ftv310_vpu_dec_t *dev)
{
	int c = dev->core_id;
	u32 reg, tmp, mask;
	struct ftv310_vpu_dec_t *next;
	u8 *pregs = dev->hwregs;
	u32 vcd_hw_id;

	dev->cfg = 0;

	/* Decoder configuration */
	if (IS_G1(dev->hw_id)) {
		reg = ioread32((void *)(pregs + FTV310_VPUDEC_SYNTH_CFG * 4));

		tmp = (reg >> DWL_H264_E) & 0x3U;
		if (tmp)
			pr_info("ftv310_vpu_dec: regs[%d] bit %d high, core[%d] has H264\n", FTV310_VPUDEC_SYNTH_CFG, DWL_H264_E, c);
		dev->cfg |= tmp ? 1 << DWL_CLIENT_TYPE_H264_DEC : 0;

		tmp = (reg >> DWL_JPEG_E) & 0x01U;
		if (tmp)
			pr_info("ftv310_vpu_dec: regs[%d] bit %d high, core[%d] has JPEG\n", FTV310_VPUDEC_SYNTH_CFG, DWL_JPEG_E, c);
		dev->cfg |= tmp ? 1 << DWL_CLIENT_TYPE_JPEG_DEC : 0;
		tmp = (reg >> DWL_HJPEG_E) & 0x01U;
		if (tmp)
			pr_info("ftv310_vpu_dec: regs[%d] bit %d high, core[%d] has HJPEG\n", FTV310_VPUDEC_SYNTH_CFG, DWL_HJPEG_E, c);
		dev->cfg |= tmp ? 1 << DWL_CLIENT_TYPE_JPEG_DEC : 0;
		tmp = (reg >> DWL_MPEG4_E) & 0x3U;
		if (tmp)
			pr_info("ftv310_vpu_dec: regs[%d] bit %d high, core[%d] has MPEG4\n", FTV310_VPUDEC_SYNTH_CFG, DWL_MPEG4_E, c);
		dev->cfg |= tmp ? 1 << DWL_CLIENT_TYPE_MPEG4_DEC : 0;

		tmp = (reg >> DWL_VC1_E) & 0x3U;
		if (tmp)
			pr_info("ftv310_vpu_dec: regs[%d] bit %d high, core[%d] has VC1\n", FTV310_VPUDEC_SYNTH_CFG, DWL_VC1_E, c);
		dev->cfg |= tmp ? 1 << DWL_CLIENT_TYPE_VC1_DEC : 0;

		tmp = (reg >> DWL_MPEG2_E) & 0x01U;
		if (tmp)
			pr_info("ftv310_vpu_dec: regs[%d] bit %d high, core[%d] has MPEG2\n", FTV310_VPUDEC_SYNTH_CFG, DWL_MPEG2_E, c);
		dev->cfg |= tmp ? 1 << DWL_CLIENT_TYPE_MPEG2_DEC : 0;

		tmp = (reg >> DWL_VP6_E) & 0x01U;
		if (tmp)
			pr_info("ftv310_vpu_dec: regs[%d] bit %d high, core[%d] has VP6\n", FTV310_VPUDEC_SYNTH_CFG, DWL_VP6_E, c);
		dev->cfg |= tmp ? 1 << DWL_CLIENT_TYPE_VP6_DEC : 0;

		reg = ioread32((void *)(pregs + FTV310_VPUDEC_SYNTH_CFG_2 * 4));

		/* VP7 and WEBP is part of VP8 */
		mask = (1 << DWL_VP8_E) | (1 << DWL_VP7_E) | (1 << DWL_WEBP_E);
		tmp = (reg & mask);
		if (tmp & (1 << DWL_VP8_E))
			pr_info("ftv310_vpu_dec: regs[%d] bit %d high, core[%d] has VP8\n", FTV310_VPUDEC_SYNTH_CFG_2, DWL_VP8_E, c);
		if (tmp & (1 << DWL_VP7_E))
			pr_info("ftv310_vpu_dec: regs[%d] bit %d high, core[%d] has VP7\n", FTV310_VPUDEC_SYNTH_CFG_2, DWL_VP7_E, c);
		if (tmp & (1 << DWL_WEBP_E))
			pr_info("ftv310_vpu_dec: regs[%d] bit %d high, core[%d] has WebP\n", FTV310_VPUDEC_SYNTH_CFG_2, DWL_WEBP_E, c);
		dev->cfg |= tmp ? 1 << DWL_CLIENT_TYPE_VP8_DEC : 0;

		tmp = (reg >> DWL_AVS_E) & 0x01U;
		if (tmp)
			pr_info("ftv310_vpu_dec: regs[%d] bit %d high, core[%d] has AVS\n", FTV310_VPUDEC_SYNTH_CFG_2, DWL_AVS_E, c);
		dev->cfg |= tmp ? 1 << DWL_CLIENT_TYPE_AVS_DEC : 0;

		tmp = (reg >> DWL_RV_E) & 0x03U;
		if (tmp)
			pr_info("ftv310_vpu_dec: regs[%d] bit %d high, core[%d] has RV\n", FTV310_VPUDEC_SYNTH_CFG_2, DWL_RV_E, c);
		dev->cfg |= tmp ? 1 << DWL_CLIENT_TYPE_RV_DEC : 0;

		/* Post-processor configuration */
		reg = ioread32((void *)(pregs + FTV310_VPUPP_SYNTH_CFG * 4));

		tmp = (reg >> DWL_G1_PP_E) & 0x01U;
		if (tmp)
			pr_info("ftv310_vpu_dec: regs[%d] bit %d high, core[%d] has PP\n", FTV310_VPUPP_SYNTH_CFG, DWL_G1_PP_E, c);
		dev->cfg |= tmp ? 1 << DWL_CLIENT_TYPE_PP : 0;
	} else if ((IS_G2(dev->hw_id))) {
		reg = ioread32((void *)(pregs + FTV310_VPUDEC_CFG_STAT * 4));

		tmp = (reg >> DWL_G2_HEVC_E) & 0x01U;
		if (tmp)
			pr_info("ftv310_vpu_dec: regs[%d] bit %d high, core[%d] has HEVC\n", FTV310_VPUDEC_CFG_STAT, DWL_G2_HEVC_E, c);
		dev->cfg |= tmp ? 1 << DWL_CLIENT_TYPE_HEVC_DEC : 0;

		tmp = (reg >> DWL_G2_VP9_E) & 0x01U;
		if (tmp)
			pr_info("ftv310_vpu_dec: regs[%d] bit %d high, core[%d] has VP9\n", FTV310_VPUDEC_CFG_STAT, DWL_G2_VP9_E, c);
		dev->cfg |= tmp ? 1 << DWL_CLIENT_TYPE_VP9_DEC : 0;

		/* Post-processor configuration */
		reg = ioread32((void *)(pregs + FTV310_VPUDECPP_SYNTH_CFG * 4));

		tmp = (reg >> DWL_G2_PP_E) & 0x01U;
		if (tmp)
			pr_info("ftv310_vpu_dec: regs[%d] bit %d high, core[%d] has PP\n", FTV310_VPUDECPP_SYNTH_CFG, DWL_G2_PP_E, c);
		dev->cfg |= tmp ? 1 << DWL_CLIENT_TYPE_PP : 0;
	} else if ((IS_VCD(dev->hw_id)) && !dev->its_main_core_id) {
		reg = ioread32((void *)(pregs + FTV310_VPUDEC_SYNTH_CFG * 4));
		vcd_hw_id = ioread32((void *)(pregs + FTV310_VPUDEC_HW_ID * 4));

		tmp = (reg >> DWL_H264_E) & 0x3U;
		if (tmp)
			pr_info("ftv310_vpu_dec: regs[%d] bit %d high, core[%d] has H264\n", FTV310_VPUDEC_SYNTH_CFG, DWL_H264_E, c);
		dev->cfg |= tmp ? 1 << DWL_CLIENT_TYPE_H264_DEC : 0;

		tmp = (reg >> DWL_H264HIGH10_E) & 0x01U;
		if (tmp)
			pr_info("ftv310_vpu_dec: regs[%d] bit %d high, core[%d] has H264HIGH10\n", FTV310_VPUDEC_SYNTH_CFG, DWL_H264HIGH10_E, c);
		dev->cfg |= tmp ? 1 << DWL_CLIENT_TYPE_H264_MAIN10 : 0;

		if ((IS_VCD_1_1(vcd_hw_id))) {
			tmp = (reg >> DWL_JPEG_E_1_1) & 0x01U;
			if (tmp)
				pr_info("ftv310_vpu_dec: regs[%d] bit %d high, core[%d] has JPEG\n", FTV310_VPUDEC_SYNTH_CFG, DWL_JPEG_E_1_1, c);
			tmp = (reg >> DWL_HJPEG_E_1_1) & 0x01U;
			if (tmp)
				pr_info("ftv310_vpu_dec: regs[%d] bit %d high, core[%d] has HJPEG\n", FTV310_VPUDEC_SYNTH_CFG, DWL_HJPEG_E_1_1, c);
		} else {
			tmp = (reg >> DWL_JPEG_E_1_3) & 0x01U;
			if (tmp)
				pr_info("ftv310_vpu_dec: regs[%d] bit %d high, core[%d] has JPEG\n", FTV310_VPUDEC_SYNTH_CFG, DWL_JPEG_E_1_3, c);
			tmp = (reg >> DWL_HJPEG_E_1_3) & 0x01U;
			if (tmp)
				pr_info("ftv310_vpu_dec: regs[%d] bit %d high, core[%d] has HJPEG\n", FTV310_VPUDEC_SYNTH_CFG, DWL_HJPEG_E_1_3, c);
		}
		dev->cfg |= tmp ? 1 << DWL_CLIENT_TYPE_JPEG_DEC : 0;
		tmp = (reg >> DWL_MPEG4_E) & 0x3U;
		if (tmp)
			pr_info("ftv310_vpu_dec: regs[%d] bit %d high, core[%d] has MPEG4\n", FTV310_VPUDEC_SYNTH_CFG, DWL_MPEG4_E, c);
		dev->cfg |= tmp ? 1 << DWL_CLIENT_TYPE_MPEG4_DEC : 0;

		tmp = (reg >> DWL_VC1_E) & 0x3U;
		if (tmp)
			pr_info("ftv310_vpu_dec: regs[%d] bit %d high, core[%d] has VC1\n", FTV310_VPUDEC_SYNTH_CFG, DWL_VC1_E, c);
		dev->cfg |= tmp ? 1 << DWL_CLIENT_TYPE_VC1_DEC : 0;

		tmp = (reg >> DWL_MPEG2_E) & 0x01U;
		if (tmp)
			pr_info("ftv310_vpu_dec: regs[%d] bit %d high, core[%d] has MPEG2\n", FTV310_VPUDEC_SYNTH_CFG, DWL_MPEG2_E, c);
		dev->cfg |= tmp ? 1 << DWL_CLIENT_TYPE_MPEG2_DEC : 0;

		tmp = (reg >> DWL_VP6_E) & 0x01U;
		if (tmp)
			pr_info("ftv310_vpu_dec: regs[%d] bit %d high, core[%d] has VP6\n", FTV310_VPUDEC_SYNTH_CFG, DWL_VP6_E, c);
		dev->cfg |= tmp ? 1 << DWL_CLIENT_TYPE_VP6_DEC : 0;

		reg = ioread32((void *)(pregs + FTV310_VPUDEC_SYNTH_CFG_2 * 4));

		/* VP7 and WEBP is part of VP8 */
		mask = (1 << DWL_VP8_E) | (1 << DWL_VP7_E) | (1 << DWL_WEBP_E);
		tmp = (reg & mask);
		if (tmp & (1 << DWL_VP8_E))
			pr_info("ftv310_vpu_dec: regs[%d] bit %d high, core[%d] has VP8\n", FTV310_VPUDEC_SYNTH_CFG_2, DWL_VP8_E, c);
		if (tmp & (1 << DWL_VP7_E))
			pr_info("ftv310_vpu_dec: regs[%d] bit %d high, core[%d] has VP7\n", FTV310_VPUDEC_SYNTH_CFG_2, DWL_VP7_E, c);
		if (tmp & (1 << DWL_WEBP_E))
			pr_info("ftv310_vpu_dec: regs[%d] bit %d high, core[%d] has WebP\n", FTV310_VPUDEC_SYNTH_CFG_2, DWL_WEBP_E, c);
		dev->cfg |= tmp ? 1 << DWL_CLIENT_TYPE_VP8_DEC : 0;

		tmp = (reg >> DWL_AVS_E) & 0x01U;
		if (tmp)
			pr_info("ftv310_vpu_dec: regs[%d] bit %d high, core[%d] has AVS\n", FTV310_VPUDEC_SYNTH_CFG_2, DWL_AVS_E, c);
		dev->cfg |= tmp ? 1 << DWL_CLIENT_TYPE_AVS_DEC : 0;

		tmp = (reg >> DWL_RV_E) & 0x03U;
		if (tmp)
			pr_info("ftv310_vpu_dec: regs[%d] bit %d high, core[%d] has RV\n", FTV310_VPUDEC_SYNTH_CFG_2, DWL_RV_E, c);
		dev->cfg |= tmp ? 1 << DWL_CLIENT_TYPE_RV_DEC : 0;

		reg = ioread32((void *)(pregs + FTV310_VPUDEC_SYNTH_CFG_3 * 4));

		tmp = (reg >> DWL_HEVC_E) & 0x07U;
		if (tmp)
			pr_info("ftv310_vpu_dec: regs[%d] bit %d high, core[%d] has HEVC\n", FTV310_VPUDEC_SYNTH_CFG_3, DWL_HEVC_E, c);
		dev->cfg |= tmp ? 1 << DWL_CLIENT_TYPE_HEVC_DEC : 0;

		tmp = (reg >> DWL_VP9_E) & 0x07U;
		if (tmp)
			pr_info("ftv310_vpu_dec: regs[%d] bit %d high, core[%d] has VP9\n", FTV310_VPUDEC_SYNTH_CFG_3, DWL_VP9_E, c);
		dev->cfg |= tmp ? 1 << DWL_CLIENT_TYPE_VP9_DEC : 0;

		/* Post-processor configuration */
		reg = ioread32((void *)(pregs + FTV310_VPUDECPP_CFG_STAT * 4));

		tmp = (reg >> DWL_PP_E) & 0x01U;
		if (tmp)
			pr_info("ftv310_vpu_dec: regs[%d] bit %d high, core[%d] has PP\n", FTV310_VPUDECPP_CFG_STAT, DWL_PP_E, c);
		dev->cfg |= tmp ? 1 << DWL_CLIENT_TYPE_PP : 0;

		if (dev->its_aux_core_id) {
			/* set main_core_id and aux_core_id */
			next = dev->its_aux_core_id;
			reg = ioread32((void *)(next->hwregs +
						FTV310_VPUDEC_SYNTH_CFG_2 * 4));

			tmp = (reg >> DWL_H264_PIPELINE_E) & 0x01U;
			if (tmp)
				pr_info("ftv310_vpu_dec: regs[%d] bit %d high, core[%d] has H264\n", FTV310_VPUDEC_SYNTH_CFG_2, DWL_H264_PIPELINE_E, c);
			next->cfg |= tmp ? 1 << DWL_CLIENT_TYPE_H264_DEC : 0;

			tmp = (reg >> DWL_JPEG_PIPELINE_E) & 0x01U;
			if (tmp)
				pr_info("ftv310_vpu_dec: regs[%d] bit %d high, core[%d] has JPEG\n", FTV310_VPUDEC_SYNTH_CFG_2, DWL_JPEG_PIPELINE_E, c);
			next->cfg |= tmp ? 1 << DWL_CLIENT_TYPE_JPEG_DEC : 0;
		}
	}
	dev->cfg_backup = dev->cfg;
}

static int CoreHasFormat(const u32 cfg, u32 format)
{
	return (cfg & (1 << format)) ? 1 : 0;
}

static int IsValideCore(long core, unsigned long core_mask)
{
	if ((core_mask >> core) & 0x01)
		return 1;
	return 0;
}

static int GetDecCore(long core, struct ftv310_vpu_dec_t *dev, struct file *filp,
		      unsigned long core_mask)
{
	int success = 0;
	unsigned long flags;
	struct slice_info *parentslice = getparentslice(dev, FTV310_VPU_CORE_DEC);

	spin_lock_irqsave(&parentslice->owner_lock, flags);

	if (IsValideCore(core, core_mask) && !dev->dec_owner) {
		((struct slice_info *)(dev->parentslice))->dec_irq &=
			~(1 << core);
		dev->dec_owner = filp;
		success = 1;
	}


	spin_unlock_irqrestore(&parentslice->owner_lock, flags);

	return success;
}

static int GetDecCoreAny(long *core, struct ftv310_vpu_dec_t *dev, struct file *filp,
			 unsigned long core_mask)
{
	int success = 0;
	long c = 0;

	*core = -1;

	while (dev) {
		/* a free core that has format */
		if (GetDecCore(c, dev, filp, core_mask)) {
			success = 1;
			*core = c;
			break;
		}
		c++;
		dev = dev->next;
	}

	return success;
}

static int GetDecCoreID(struct ftv310_vpu_dec_t *dev, struct file *filp,
			unsigned long format)
{
	long c = 0;
	unsigned long flags;
	int core_id = -1;
	struct slice_info *parentslice = getparentslice(dev, FTV310_VPU_CORE_DEC);

	while (dev) {
		/* a core that has format */
		spin_lock_irqsave(&parentslice->owner_lock, flags);
		if (CoreHasFormat(dev->cfg_backup, format)) {
			core_id = c;
			spin_unlock_irqrestore(&parentslice->owner_lock, flags);
			break;
		}
		spin_unlock_irqrestore(&parentslice->owner_lock, flags);
		dev = dev->next;
		c++;
	}
	return core_id;
}

static long ReserveDecoder(struct ftv310_vpu_dec_t *dev, struct file *filp,
			   unsigned long core_mask)
{
	long core = -1;
	struct slice_info *parentslice = getparentslice(dev, FTV310_VPU_CORE_DEC);

	/* reserve a core */
	if (down_interruptible(&parentslice->dec_core_sem))
		return -ERESTARTSYS;

	/* lock a core that has specific format*/
	if (wait_event_interruptible(parentslice->hw_queue,
				     GetDecCoreAny(&core, dev, filp, core_mask) !=
					     0))
		return -ERESTARTSYS;

	return core;
}

static void ReleaseDecoder(struct ftv310_vpu_dec_t *dev, long core)
{
	u32 status;
	unsigned long flags;
	struct slice_info *parentslice = getparentslice(dev, FTV310_VPU_CORE_DEC);

	status = ioread32((void *)(dev->hwregs + FTV310_VPUDEC_IRQ_STAT_DEC_OFF));

	/* make sure HW is disabled */
	if (status & FTV310_VPUDEC_DEC_E) {
		pr_info("ftv310_vpu_dec: DEC[%li] still enabled -> reset %x\n", core,
			status);

		/* abort decoder */
		status |= FTV310_VPUDEC_DEC_ABORT | FTV310_VPUDEC_DEC_IRQ_DISABLE;
		iowrite32(status,
			  (void *)(dev->hwregs + FTV310_VPUDEC_IRQ_STAT_DEC_OFF));
	}

	spin_lock_irqsave(&parentslice->owner_lock, flags);

	/* If aux core released, revert main core's config back */
	if (dev->its_main_core_id)
		dev->its_main_core_id->cfg = dev->its_main_core_id->cfg_backup;

	/* If main core released, revert aux core's config back */
	if (dev->its_aux_core_id)
		dev->its_aux_core_id->cfg = dev->its_aux_core_id->cfg_backup;

	dev->dec_owner = NULL;

	spin_unlock_irqrestore(&parentslice->owner_lock, flags);

	up(&parentslice->dec_core_sem);

	wake_up_interruptible_all(&parentslice->hw_queue);
}

static long ReservePostProcessor(struct ftv310_vpu_dec_t *dev, struct file *filp)
{
	unsigned long flags;
	long core = 0;
	struct slice_info *parentslice = getparentslice(dev, FTV310_VPU_CORE_DEC);

	/* single core PP only */
	if (down_interruptible(&parentslice->pp_core_sem))
		return -ERESTARTSYS;

	spin_lock_irqsave(&parentslice->owner_lock, flags);
	if (dev)
		dev->pp_owner = filp;

	spin_unlock_irqrestore(&parentslice->owner_lock, flags);

	return core;
}

static void ReleasePostProcessor(struct ftv310_vpu_dec_t *dev, long core)
{
	unsigned long flags;
	struct slice_info *parentslice = getparentslice(dev, FTV310_VPU_CORE_DEC);

	u32 status = ioread32((void *)(dev->hwregs + FTV310_VPU_IRQ_STAT_PP_OFF));

	/* make sure HW is disabled */
	if (status & FTV310_VPU_PP_E) {
		pr_info("ftv310_vpu_dec: PP[%li] still enabled -> reset\n", core);

		/* disable IRQ */
		status |= FTV310_VPU_PP_IRQ_DISABLE;

		/* disable postprocessor */
		status &= (~FTV310_VPU_PP_E);
		iowrite32(0x10, (void *)(dev->hwregs + FTV310_VPU_IRQ_STAT_PP_OFF));
	}

	spin_lock_irqsave(&parentslice->owner_lock, flags);

	dev->pp_owner = NULL;

	spin_unlock_irqrestore(&parentslice->owner_lock, flags);

	up(&parentslice->pp_core_sem);
}

static long DecFlushRegs(struct ftv310_vpu_dec_t *dev, struct core_desc *core)
{
	long ret = 0, i;

	ret = copy_from_user(dev->dec_regs, core->regs,
			     FTV310_VPU_FTV310D_REGS * 4);
	if (ret) {
		PDEBUG("copy_from_user failed, returned %li\n", ret);
		return -EFAULT;
	}

	/* write all regs but the status reg[1] to hardware */
	iowrite32(0x0, (void *)(dev->hwregs + 4));
	for (i = 2; i <= FTV310_VPU_VCD_LAST_REG; i++)
		iowrite32(dev->dec_regs[i], (void *)(dev->hwregs + i * 4));

#ifdef PHY_CONFIG_PM
	if (dev->dec_regs[1] & 0x1)
		dev->hw_active = 1;
#endif

	/* write the status register, which may start the decoder */
	iowrite32(dev->dec_regs[1], (void *)(dev->hwregs + 4));
	return 0;
}

static long DecRefreshRegs(struct ftv310_vpu_dec_t *dev, struct core_desc *core)
{
	long ret, i;

	for (i = 0; i <= FTV310_VPU_VCD_LAST_REG; i++)
		dev->dec_regs[i] = ioread32((void *)(dev->hwregs + i * 4));

	ret = copy_to_user(core->regs, dev->dec_regs, FTV310_VPU_VCD_LAST_REG * 4);
	if (ret) {
		PDEBUG("copy_to_user failed, returned %li\n", ret);
		return -EFAULT;
	}
	return 0;
}

static int CheckDecIrq(struct ftv310_vpu_dec_t *dev, int id)
{
	unsigned long flags;
	int rdy = 0;
	struct slice_info *parentslice = getparentslice(dev, FTV310_VPU_CORE_DEC);
	const u32 irq_mask = (1 << id);

	spin_lock_irqsave(&parentslice->owner_lock, flags);

	if (parentslice->dec_irq & irq_mask) {
		/* reset the wait condition(s) */
		parentslice->dec_irq &= ~irq_mask;
		rdy = 1;
	}

	spin_unlock_irqrestore(&parentslice->owner_lock, flags);

	return rdy;
}

static long WaitDecReadyAndRefreshRegs(struct ftv310_vpu_dec_t *dev,
				       struct core_desc *Core)
{
	u32 id = KCORE(Core->id);
	long ret;
	struct slice_info *parentslice = getparentslice(dev, FTV310_VPU_CORE_DEC);

	PDEBUG("wait_event_interruptible DEC[%d]\n", id);

	ret = wait_event_interruptible_timeout(parentslice->dec_wait_queue,
					       CheckDecIrq(dev, id),
					       msecs_to_jiffies(10));
	if (ret == -ERESTARTSYS) {
		pr_err("DEC[%d]  failed to wait_event_interruptible interrupted\n",
		       id);
		return -ERESTARTSYS;
	} else if (ret == 0) {
		pr_err("DEC[%d]  wait_event_interruptible timeout\n", id);
		timeout = 1;
		return -EBUSY;
	}
	atomic_inc(&irq_tx);

	/* refresh registers */
	return DecRefreshRegs(dev, Core);
}

static long DecWriteRegs(struct ftv310_vpu_dec_t *dev, struct core_desc *core)
{
	long ret = 0, i;

	if (core->type == HW_AXIFE) {
#ifdef HAS_AXIFE
		u32 slice = SLICE(core->id);
		u32 node = KCORE(core->id);
		struct axife_t *axife_dev;

		axife_dev = get_axifenodebytype(slice, NODE_TYPE_DEC, node);
		return AxifeWriteRegs(axife_dev, core);
#else
		return 0;
#endif

	} else if (core->type == HW_L2CACHE) {
#ifdef HAS_CACHECORE
		u32 slice = SLICE(core->id);
		u32 node = KCORE(core->id);
		struct cache_dev_t *L2Cache_dev;

		L2Cache_dev = get_cachenodebytype(slice, NODE_TYPE_DEC, node);
		return CacheWriteRegs(L2Cache_dev, core);
#else
		return 0;
#endif
	} else {
		i = core->reg_id;
		ret = copy_from_user(dev->dec_regs + core->reg_id, core->regs,
				     core->size);

		if (ret) {
			PDEBUG("copy_from_user failed, returned %li\n", ret);
			return -EFAULT;
		}
		for (i = core->reg_id; i < core->reg_id + core->size / 4; i++)
			iowrite32(dev->dec_regs[i],
				  (void *)dev->hwregs + i * 4);
	}
	return 0;
}

u32 *ftv310_vpu_dec_getRegAddr(u32 coreid, u32 regid)
{
	int i;
	struct ftv310_vpu_dec_t *dev = get_decnodes(SLICE(coreid), KCORE(coreid));

	if (!dev)
		return NULL;

	for (i = 0; i < coreid; i++)
		dev = dev->next;
	if (regid * 4 >= dev->iosize)
		return NULL;
	return (u32 *)(dev->hwregs + regid * 4);
}

static long DecReadRegs(struct ftv310_vpu_dec_t *dev, struct core_desc *core)
{
	long ret, i;

	if (core->type == HW_AXIFE) {
#ifdef HAS_AXIFE
		u32 slice = SLICE(core->id);
		u32 node = KCORE(core->id);
		struct axife_t *axife_dev;

		axife_dev = get_axifenodebytype(slice, NODE_TYPE_DEC, node);
		return AxifeReadRegs(axife_dev, core);
#else
		return 0;
#endif
	} else if (core->type == HW_L2CACHE) {
#ifdef HAS_CACHECORE
		u32 slice = SLICE(core->id);
		u32 node = KCORE(core->id);
		struct cache_dev_t *L2Cache_dev;

		L2Cache_dev = get_cachenodebytype(slice, NODE_TYPE_DEC, node);
		return CacheReadRegs(L2Cache_dev, core);
#else
		return 0;
#endif
	} else {
		i = core->reg_id;

		/* read specific registers from hardware */
		for (i = core->reg_id; i < core->reg_id + core->size / 4; i++)
			dev->dec_regs[i] =
				ioread32((void *)dev->hwregs + i * 4);

		/* put registers to user space*/
		ret = copy_to_user(core->regs + core->reg_id, dev->dec_regs + core->reg_id,
				   core->size);
		if (ret) {
			PDEBUG("copy_to_user failed, returned %li\n", ret);
			return -EFAULT;
		}
	}
	return 0;
}

static long PPFlushRegs(struct ftv310_vpu_dec_t *dev, struct core_desc *Core)
{
	long ret = 0;
	u32 i;

	/* copy original dec regs to kernal space*/
	ret = copy_from_user(dev->dec_regs + FTV310_VPU_PP_ORG_FIRST_REG,
			     Core->regs + FTV310_VPU_PP_ORG_FIRST_REG,
			     FTV310_VPU_PP_ORG_REGS * 4);
#ifdef USE_64BIT_ENV
	/* copy extended dec regs to kernal space*/
	ret = copy_from_user(dev->dec_regs + FTV310_VPU_PP_EXT_FIRST_REG,
			     Core->regs + FTV310_VPU_PP_EXT_FIRST_REG,
			     FTV310_VPU_PP_EXT_REGS * 4);
#endif
	if (ret) {
		pr_err("copy_from_user failed, returned %li\n", ret);
		return -EFAULT;
	}

	/* write all regs but the status reg[1] to hardware */
	/* both original and extended regs need to be written */
	for (i = FTV310_VPU_PP_ORG_FIRST_REG + 1; i <= FTV310_VPU_PP_ORG_LAST_REG; i++)
		iowrite32(dev->dec_regs[i], (void *)dev->hwregs + i * 4);
#ifdef USE_64BIT_ENV
	for (i = FTV310_VPU_PP_EXT_FIRST_REG; i <= FTV310_VPU_PP_EXT_LAST_REG; i++)
		iowrite32(dev->dec_regs[i], (void *)dev->hwregs + i * 4);
#endif
	/* write the stat reg, which may start the PP */
	iowrite32(dev->dec_regs[FTV310_VPU_PP_ORG_FIRST_REG],
		  (void *)dev->hwregs + FTV310_VPU_PP_ORG_FIRST_REG * 4);

	return 0;
}

static long PPRefreshRegs(struct ftv310_vpu_dec_t *dev, struct core_desc *Core)
{
	long i, ret;
#ifdef USE_64BIT_ENV
	/* user has to know exactly what they are asking for */
	if (Core->size != (FTV310_VPU_PP_TOTAL_REGS * 4))
		return -EFAULT;
#else
	/* user has to know exactly what they are asking for */
	if (Core->size != (FTV310_VPU_PP_ORG_REGS * 4))
		return -EFAULT;
#endif

	/* read all registers from hardware */
	/* both original and extended regs need to be read */
	for (i = FTV310_VPU_PP_ORG_FIRST_REG; i <= FTV310_VPU_PP_ORG_LAST_REG; i++)
		dev->dec_regs[i] = ioread32((void *)dev->hwregs + i * 4);
#ifdef USE_64BIT_ENV
	for (i = FTV310_VPU_PP_EXT_FIRST_REG; i <= FTV310_VPU_PP_EXT_LAST_REG; i++)
		dev->dec_regs[i] = ioread32((void *)dev->hwregs + i * 4);
#endif
	/* put registers to user space*/
	/* put original registers to user space*/
	ret = copy_to_user(Core->regs + FTV310_VPU_PP_ORG_FIRST_REG,
			   dev->dec_regs + FTV310_VPU_PP_ORG_FIRST_REG,
			   FTV310_VPU_PP_ORG_REGS * 4);
#ifdef USE_64BIT_ENV
	/* put extended registers to user space*/
	ret = copy_to_user(Core->regs + FTV310_VPU_PP_EXT_FIRST_REG,
			   dev->dec_regs + FTV310_VPU_PP_EXT_FIRST_REG,
			   FTV310_VPU_PP_EXT_REGS * 4);
#endif
	if (ret) {
		pr_err("copy_to_user failed, returned %li\n", ret);
		return -EFAULT;
	}

	return 0;
}

static int CheckPPIrq(struct ftv310_vpu_dec_t *dev, int id)
{
	unsigned long flags;
	int rdy = 0;
	struct slice_info *parentslice = getparentslice(dev, FTV310_VPU_CORE_DEC);

	const u32 irq_mask = (1 << id);

	spin_lock_irqsave(&parentslice->owner_lock, flags);

	if (parentslice->pp_irq & irq_mask) {
		/* reset the wait condition(s) */
		parentslice->pp_irq &= ~irq_mask;
		rdy = 1;
	}

	spin_unlock_irqrestore(&parentslice->owner_lock, flags);

	return rdy;
}

static long WaitPPReadyAndRefreshRegs(struct ftv310_vpu_dec_t *dev,
				      struct core_desc *Core)
{
	u32 id = Core->id;
	struct slice_info *parentslice = getparentslice(dev, FTV310_VPU_CORE_DEC);

	PDEBUG("wait_event_interruptible PP[%d]\n", id);

	if (wait_event_interruptible(parentslice->pp_wait_queue,
				     CheckPPIrq(dev, id))) {
		pr_err("PP[%d]  failed to wait_event_interruptible interrupted\n",
		       id);
		return -ERESTARTSYS;
	}

	atomic_inc(&irq_tx);

	/* refresh registers */
	return PPRefreshRegs(dev, Core);
}

static int CheckCoreIrq(struct ftv310_vpu_dec_t *dev, const struct file *filp,
			u32 *id)
{
	unsigned long flags;
	int rdy = 0, n = 0;
	struct slice_info *parentslice = getparentslice(dev, FTV310_VPU_CORE_DEC);

	while (dev) {
		u32 irq_mask = (1 << n);

		spin_lock_irqsave(&parentslice->owner_lock, flags);

		if (parentslice->dec_irq & irq_mask) {
			if (*id == n) { //if (pcore->dec_owner == filp)
				/* we have an IRQ for our client */

				/* reset the wait condition(s) */
				parentslice->dec_irq &= ~irq_mask;

				/* signal ready Core no. for our client */
				rdy = 1;
				spin_unlock_irqrestore(&parentslice->owner_lock,
						       flags);
				break;
			} else if (!dev->dec_owner) {
				/* zombie IRQ */
				pr_info("IRQ on Core[%d], but no owner!!!\n",
					n);

				/* reset the wait condition(s) */
				parentslice->dec_irq &= ~irq_mask;
			}
		}

		spin_unlock_irqrestore(&parentslice->owner_lock, flags);

		n++; /* next Core */
		dev = dev->next;
	}

	return rdy;
}

static long WaitCoreReady(struct ftv310_vpu_dec_t *dev, const struct file *filp,
			  u32 *id)
{
	struct slice_info *parentslice = getparentslice(dev, FTV310_VPU_CORE_DEC);

	PDEBUG("wait_event_interruptible CORE\n");
	if (wait_event_interruptible(parentslice->dec_wait_queue,
				     CheckCoreIrq(dev, filp, id))) {
		pr_err("CORE  failed to wait_event_interruptible interrupted\n");
		return -EINTR;
	}

	atomic_inc(&irq_tx);

	return 0;
}

/*-------------------------------------------------------------------------
 *Function name   : ftv310_vpu_dec_ioctl
 *Description     : communication method to/from the user space
 *
 *Return type     : long
 *-------------------------------------------------------------------------
 */

long ftv310_vpu_dec_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int i, ret;
	u32 hw_id, id;
	u32 slice, node;
	long tmp = 0;
	struct nor32_parameter core_info;
	struct core_desc core;
	struct ftv310_vpu_dec_t *pcore;
	ftv310_vpu_ioctl_id ioctl_id_par;

	switch (_IOC_NR(cmd)) {
	case _IOC_NR(FTV310_VPUDEC_IOC_CLI): {
		id = arg;
		pcore = getcoreCtrl(id);
		if (!pcore)
			return -EFAULT;
		for (i = 0; i < 4; i++)
			if (pcore->irqlist[i] > 0)
				disable_irq(pcore->irqlist[i]);
		break;
	}
	case _IOC_NR(FTV310_VPUDEC_IOC_STI): {
		id = arg;
		pcore = getcoreCtrl(id);
		if (!pcore)
			return -EFAULT;
		for (i = 0; i < 4; i++)
			if (pcore->irqlist[i] > 0)
				enable_irq(pcore->irqlist[i]);
		break;
	}
	case _IOC_NR(FTV310_VPUDEC_IOCGHWOFFSET): {
		__get_user(id, (unsigned long *)arg);
		pcore = getcoreCtrl(id);
		if (!pcore)
			return -EFAULT;

		__put_user(pcore->multicorebase_actual,
			   (unsigned long long *)arg);
		break;
	}
	case _IOC_NR(FTV310_VPUDEC_IOCGHWIOSIZE): {
		__u32 io_size;

		__get_user(ioctl_id_par.data, (__u32 *)arg);
		pcore = get_decnodes(ioctl_id_par.ID_PAR.node_idx,
				     ioctl_id_par.ID_PAR.codec_idx);
		if (!pcore)
			return -EFAULT;
		io_size = pcore->iosize;
		__put_user(io_size, (u32 *)arg);

		return 0;
	}
	case _IOC_NR(FTV310_VPUDEC_IOC_MC_OFFSETS): {
		__get_user(slice, (__u32 *)arg);
		pcore = get_decnodes(slice, 0);
		if (!pcore)
			return -EFAULT;
		i = 0;
		while (pcore) {
			tmp = copy_to_user(((unsigned long long *)arg) + i,
					   &pcore->multicorebase_actual,
					   sizeof(pcore->multicorebase_actual));
			if (tmp) {
				pr_err("copy_to_user failed, returned %li\n",
				       tmp);
				return -EFAULT;
			}
			pcore = pcore->next;
			i++;
		}
		break;
	}
	case _IOC_NR(FTV310_VPUDEC_IOC_MC_CORES):
		id = (u32)arg;
		id = get_slicecorenum(id, FTV310_VPU_CORE_DEC);
		PDEBUG("cores=%d\n", id);
		return id;
	case _IOC_NR(FTV310_VPUDEC_IOCS_DEC_PUSH_REG): {
		/* get registers from user space*/
		tmp = copy_from_user(&core, (void *)arg,
				     sizeof(struct core_desc));
		if (tmp) {
			pr_err("copy_from_user failed, returned %li\n", tmp);
			return -EFAULT;
		}
		pcore = getcoreCtrl(core.id);
		if (!pcore)
			return -EFAULT;
		DecFlushRegs(pcore, &core);
		break;
	}

	case _IOC_NR(FTV310_VPUDEC_IOCS_DEC_WRITE_REG): {
		/* get registers from user space*/
		tmp = copy_from_user(&core, (void *)arg,
				     sizeof(struct core_desc));
		if (tmp) {
			PDEBUG("copy_from_user failed, returned %li\n", tmp);
			return -EFAULT;
		}
		pcore = getcoreCtrl(core.id);
		if (!pcore)
			return -EFAULT;

		DecWriteRegs(pcore, &core);
		break;
	}
	case _IOC_NR(FTV310_VPUDEC_IOCS_PP_PUSH_REG): {
		/* get registers from user space*/
		tmp = copy_from_user(&core, (void *)arg,
				     sizeof(struct core_desc));
		if (tmp) {
			pr_err("copy_from_user failed, returned %li\n", tmp);
			return -EFAULT;
		}
		pcore = getcoreCtrl(core.id);
		if (!pcore)
			return -EFAULT;
		PPFlushRegs(pcore, &core);
		break;
	}
	case _IOC_NR(FTV310_VPUDEC_IOCS_DEC_PULL_REG): {
		tmp = copy_from_user(&core, (void *)arg,
				     sizeof(struct core_desc));
		if (tmp) {
			pr_err("copy_from_user failed, returned %li\n", tmp);
			return -EFAULT;
		}
		pcore = getcoreCtrl(core.id);
		if (!pcore)
			return -EFAULT;
		return DecRefreshRegs(pcore, &core);
	}
	case _IOC_NR(FTV310_VPUDEC_IOCS_DEC_READ_REG): {
		tmp = copy_from_user(&core, (void *)arg,
				     sizeof(struct core_desc));
		if (tmp) {
			PDEBUG("copy_from_user failed, returned %li\n", tmp);
			return -EFAULT;
		}
		pcore = getcoreCtrl(core.id);
		if (!pcore)
			return -EFAULT;
		return DecReadRegs(pcore, &core);
	}
	case _IOC_NR(FTV310_VPUDEC_IOCS_PP_PULL_REG): {
		tmp = copy_from_user(&core, (void *)arg,
				     sizeof(struct core_desc));
		if (tmp) {
			pr_err("copy_from_user failed, returned %li\n", tmp);
			return -EFAULT;
		}
		pcore = getcoreCtrl(core.id);
		if (!pcore)
			return -EFAULT;
		return PPRefreshRegs(pcore, &core);
	}
	case _IOC_NR(FTV310_VPUDEC_IOCH_DEC_RESERVE): {
		ret = copy_from_user(&core_info, (void *)arg,
				     sizeof(struct nor32_parameter));
		if (ret)
			return ret;
		ioctl_id_par.data = core_info.id;
		PDEBUG("Reserve DEC core, core mask = %i\n", (u32)core_info.data);

		pcore = get_decnodes(ioctl_id_par.ID_PAR.node_idx,
				     ioctl_id_par.ID_PAR.codec_idx);
		if (!pcore)
			return -EFAULT;
		ret = ReserveDecoder(pcore, filp, core_info.data & 0xffffffff);
		if (ret < 0)
			return -EFAULT;
		else
			return ret | (ioctl_id_par.ID_PAR.codec_idx);
	}
	case _IOC_NR(FTV310_VPUDEC_IOCT_DEC_RELEASE): {
		pcore = getcoreCtrl((u32)arg);
		if (!pcore)
			return -EFAULT;
		if (pcore->dec_owner != filp) {
			pr_err("bogus DEC release, Core = %li\n", arg);
			return -EFAULT;
		}

		PDEBUG("Release DEC, core = %li\n", arg);

		ReleaseDecoder(pcore, arg);

		break;
	}
	case _IOC_NR(FTV310_VPUDEC_IOCQ_PP_RESERVE):
		id = (u32)arg;
		pcore = get_decnodes(SLICE(id), 0);
		if (!pcore)
			return -EFAULT;
		return ReservePostProcessor(pcore, filp);
	case _IOC_NR(FTV310_VPUDEC_IOCT_PP_RELEASE): {
		pcore = getcoreCtrl(arg);
		if (!pcore)
			return -EFAULT;
		if (arg != 0 || pcore->pp_owner != filp) {
			pr_err("bogus PP release %li\n", arg);
			return -EFAULT;
		}
		ReleasePostProcessor(pcore, arg);

		break;
	}
	case _IOC_NR(FTV310_VPUDEC_IOCX_DEC_WAIT): {
		tmp = copy_from_user(&core, (void *)arg,
				     sizeof(struct core_desc));
		if (tmp) {
			pr_err("copy_from_user failed, returned %li\n", tmp);
			return -EFAULT;
		}
		pcore = getcoreCtrl(core.id);
		if (!pcore)
			return -EFAULT;

		return WaitDecReadyAndRefreshRegs(pcore, &core);
	}
	case _IOC_NR(FTV310_VPUDEC_IOCX_PP_WAIT): {
		tmp = copy_from_user(&core, (void *)arg,
				     sizeof(struct core_desc));
		if (tmp) {
			pr_err("copy_from_user failed, returned %li\n", tmp);
			return -EFAULT;
		}
		pcore = getcoreCtrl(core.id);
		if (!pcore)
			return -EFAULT;

		return WaitPPReadyAndRefreshRegs(pcore, &core);
	}
	case _IOC_NR(FTV310_VPUDEC_IOCG_CORE_WAIT): {
		id = (u32)arg;
		slice = SLICE(id);
		node = KCORE(id);
		pcore = get_decnodes(slice, 0);
		if (!pcore)
			return -EFAULT;
		tmp = WaitCoreReady(pcore, filp, &node);
		return tmp;
	}
	case _IOC_NR(FTV310_VPUDEC_IOX_ASIC_ID): {
		ioctl_id_par.data = (u32)arg;
		pcore = get_decnodes(ioctl_id_par.ID_PAR.node_idx,
				     ioctl_id_par.ID_PAR.codec_idx);
		if (!pcore)
			return 0;
		id = ioread32((void *)pcore->hwregs);
		return id;
	}
	case _IOC_NR(FTV310_VPUDEC_IOCG_CORE_ID): {
		PDEBUG("Get DEC Core_id, format = %li\n", arg);
		ret = copy_from_user(&core_info, (void *)arg,
				     sizeof(struct nor32_parameter));
		if (ret)
			return ret;
		ioctl_id_par.data = core_info.id;
		pcore = get_decnodes(ioctl_id_par.ID_PAR.node_idx, 0);
		if (!pcore)
			return -EFAULT;
		tmp = GetDecCoreID(pcore, filp, core_info.data);
		return tmp;
	}
	case _IOC_NR(FTV310_VPUDEC_IOX_ASIC_BUILD_ID): {
		__get_user(id, (int *)arg);
		pcore = getcoreCtrl(id);
		if (!pcore)
			return -EFAULT;
		hw_id = ioread32((void *)(pcore->hwregs));
		if (IS_G1(hw_id >> 16) || IS_G2(hw_id >> 16)) {
			__put_user(hw_id, (u32 *)arg);
		} else {
			hw_id = ioread32((void *)(pcore->hwregs +
						  FTV310_VPUDEC_HW_BUILD_ID_OFF));
			__put_user(hw_id, (u32 *)arg);
		}
		return 0;
	}
	case _IOC_NR(FTV310_VPUDEC_IOX_IOCX_POLL): {
		__get_user(ioctl_id_par.data, (int *)arg);
		pcore = get_decnodes(ioctl_id_par.ID_PAR.node_idx,
				     ioctl_id_par.ID_PAR.codec_idx);
		if (!pcore)
			return -1;

		ftv310_vpu_dec_isr(0, pcore);

		return 0;
	}
	case _IOC_NR(FTV310_VPUDEC_DEBUG_STATUS): {
		struct slice_info *parentslice;

		PDEBUG("ftv310_vpu_dec: IRQs received/sent2user = %d / %d\n",
		       atomic_read(&irq_rx), atomic_read(&irq_tx));
		slice = get_slicenumber();
		for (i = 0; i < (int)slice; i++) {
			pcore = get_decnodes(i, 0);
			if (!pcore)
				continue;
			parentslice = getparentslice(pcore, FTV310_VPU_CORE_DEC);
			PDEBUG("ftv310_vpu_dec: slice %d dec_irq     = 0x%08x\n", i,
			       parentslice->dec_irq);
			PDEBUG("ftv310_vpu_dec: slice %d pp_irq      = 0x%08x\n", i,
			       parentslice->pp_irq);

			id = 0;
			while (pcore) {
				PDEBUG("ftv310_vpu_dec: slice %d dec_core[%i] %s\n",
				       i, id,
				       !pcore->dec_owner ? "FREE" : "RESERVED");
				PDEBUG("ftv310_vpu_dec: slice %d pp_core[%i]  %s\n",
				       i, id,
				       !pcore->pp_owner ? "FREE" : "RESERVED");
				pcore = pcore->next;
				id++;
			}
		}
		break;
	}
	default:
		return -ENOTTY;
	}

	return 0;
}

/*---------------------------------------------------------------------------
 *Function name   : ftv310_vpu_dec_release
 *Description     : Release driver
 *
 *Return type     : int
 *----------------------------------------------------------------------------
 */
int ftv310_vpu_dec_release(struct file *filp)
{
	int n, i, slicen = get_slicenumber();
	struct ftv310_vpu_dec_t *pcore;

	for (i = 0; i < slicen; i++) {
		pcore = get_decnodes(i, 0);
		n = 0;
		while (pcore) {
			if (pcore->dec_owner == filp) {
				PDEBUG("releasing slice %d dec Core %i lock\n",
				       i, n);
				ReleaseDecoder(pcore, n);
			}
			n++;
			pcore = pcore->next;
		}

		pcore = get_decnodes(i, 0);
		if (pcore && pcore->pp_owner == filp) {
			PDEBUG("releasing slice %d pp Core %i lock\n", i, 0);
			ReleasePostProcessor(pcore, n);
		}
	}

	PDEBUG("closed\n");
	return 0;
}

/*---------------------------------------------------------------------------
 *Function name   : ftv310_vpu_dec_init
 *Description     : Initialize the driver
 *
 *Return type     : int
 *---------------------------------------------------------------------------
 */

int ftv310_vpu_dec_open(struct inode *inode, struct file *filp)
{
	return 0;
}

int ftv310_vpu_dec_init(void)
{
	bdecprobed = 0;

	return 0;
}

int ftv310_vpu_dec_probe(dtbnode *pnode, int useirq, int loop,
		    struct ftv310_vpu_dec_t *pdeccore)
{
	int i, k, result = 0;
	struct ftv310_vpu_dec_t *pcore, *auxcore;

	if (loop == 0) {
#ifndef USE_DTB_PROBE /*simulate and compatible with old code*/
		for (i = 0; i < ARRAY_SIZE(multicorebase); i++) {
			if (multicorebase[i] == 0)
				break;
			pcore = vmalloc(sizeof(*pcore));
			if (!pcore)
				break;

			memset(pcore, 0, sizeof(*pcore));
			pcore->multicorebase = multicorebase[i];
			pcore->multicorebase_actual = multicorebase[i];
			pcore->iosize = iosize[i];
			pcore->dec_owner = NULL;
			pcore->pp_owner = NULL;
			pcore->sliceidx = sliceidxtable[i];

			for (k = 0; k < 4; k++)
				pcore->irqlist[k] = -1;
			pcore->irqlist[0] = irq[i];
			pr_info("add dec core %d;%d", pcore->sliceidx, i);
			add_decnode(pcore->sliceidx, pcore);
		}
#else /*USE_DTB_PROBE*/
		{
			pcore = vmalloc(sizeof(*pcore));
			if (!pcore)
				return -ENOMEM;

			memset(pcore, 0, sizeof(struct ftv310_vpu_dec_t));
			pcore->multicorebase = pnode->ioaddr;
			pcore->multicorebase_actual = pnode->ioaddr;
			pcore->iosize = pnode->iosize;
			pcore->dec_owner = NULL;
			pcore->pp_owner = NULL;
			pcore->sliceidx = pnode->sliceidx;
			for (i = 0; i < 4; i++)
				pcore->irqlist[i] = -1;
			pcore->irqlist[0] = pnode->irq[0];
			add_decnode(pnode->sliceidx, pcore);
		}
#endif /*USE_DTB_PROBE*/
	} else {
		auxcore = NULL;
		result = ReserveIO(pdeccore, &auxcore);
		if (result < 0)
			return -ENODEV;
		pr_debug("reserveIO success\n");

		ReadCoreConfig(pdeccore);
		ResetAsic(pdeccore);

		if (auxcore) {
			ReadCoreConfig(auxcore);
			ResetAsic(auxcore);
			auxcore->dec_owner = NULL;
			auxcore->pp_owner = NULL;
			auxcore->sliceidx = pdeccore->sliceidx;
		}
		if (useirq && pdeccore->irqlist[0] > 0) {
			result = request_irq(pdeccore->irqlist[0],
					     ftv310_vpu_dec_isr, IRQF_SHARED,
					     "irq_ftv310_vpu_c1", (void *)pdeccore);
			if (result != 0) {
				pr_err("dec can't reserve irq %d\n",
				       pdeccore->irqlist[0]);
				ReleaseIO(pdeccore);
				if (auxcore) {
					ReleaseIO(auxcore);
					vfree(auxcore);
				}
				return -ENODEV;
			}
		}
		if (auxcore)
			add_decnode(pdeccore->sliceidx, auxcore);
	}
	return 0;
}

/*---------------------------------------------------------------------------
 *Function name   : ftv310_vpu_dec_cleanup
 *Description     : clean up
 *
 *Return type     : int
 *---------------------------------------------------------------------------
 */
void ftv310_vpu_dec_cleanup(void)
{
#ifndef SIMICS_TEST
	int slicen = get_slicenumber();
	struct ftv310_vpu_dec_t *dev, *next;
	int n, i;

	for (n = 0; n < slicen; n++) {
		/* free the IRQ */
		dev = get_decnodes(n, 0);
		while (dev) {
			/* reset hardware */
			ResetAsic(dev);
			for (i = 0; i < 4; i++)
				if (dev->irqlist[i] > 0)
					free_irq(dev->irqlist[i], (void *)dev);
			ReleaseIO(dev);
			next = dev->next;
			vfree(dev);
			dev = next;
		}
	}
#endif
	bdecprobed = 0;
}

/*---------------------------------------------------------------------------
 *Function name   : CheckHwId
 *Return type     : int
 *---------------------------------------------------------------------------
 */
static int CheckHwId(struct ftv310_vpu_dec_t *dev)
{
	long hwid;
	size_t num_hw = sizeof(DecHwId) / sizeof(*DecHwId);
	int found = 0;

	hwid = readl(dev->hwregs);
	hwid = (hwid >> 16) & 0xFFFF; /* product version only */

	while (num_hw--) {
		if (hwid == DecHwId[num_hw]) {
			pr_info("ftv310_vpu_dec: Supported HW found at 0x%16llx\n",
				dev->multicorebase_actual);
			found = 1;
			dev->hw_id = hwid;
			break;
		}
	}
	if (!found) {
		pr_info("ftv310_vpu_dec: Unknown HW found at 0x%16llx\n",
			dev->multicorebase_actual);
		return 0;
	}
	return 1;
}

/*---------------------------------------------------------------------------
 *Function name   : ReserveIO
 *Description     : IO reserve
 *
 *Return type     : int
 *---------------------------------------------------------------------------
 */
static int ReserveIO(struct ftv310_vpu_dec_t *core, struct ftv310_vpu_dec_t **auxcore)
{
	int result;
	long hwid;
	u32 reg;

	if (!request_mem_region(core->multicorebase_actual, core->iosize,
				"ftv310_vpu_dec0")) {
		pr_info("ftv310_vpu_dec: failed to reserve HW regs\n");
		return -EBUSY;
	}

	core->hwregs = (u8 *)ioremap(core->multicorebase_actual, core->iosize);

	if (!core->hwregs) {
		pr_info("ftv310_vpu_dec: failed to ioremap HW regs\n");
		release_mem_region(core->multicorebase_actual, core->iosize);
		return -EBUSY;
	}
	core->its_main_core_id = NULL;
	core->its_aux_core_id = NULL;
	/* check for correct HW */
	result = CheckHwId(core);
	if (!result) {
		result = -ENXIO;
		goto error;
	}

	/* product version only */
	hwid = ((readl(core->hwregs)) >> 16) & 0xFFFF;

	if (IS_VCD(hwid)) {
		reg = readl(core->hwregs + FTV310_VPUDEC_SYNTH_CFG_2_OFF);
		if (((reg >> DWL_H264_PIPELINE_E) & 0x01U) ||
		    ((reg >> DWL_JPEG_PIPELINE_E) & 0x01U)) {
			*auxcore = vmalloc(sizeof(**auxcore));
			if (!(*auxcore)) {
				result = -ENOMEM;
				goto error;
			}
			memset(*auxcore, 0, sizeof(struct ftv310_vpu_dec_t));
			(*auxcore)->multicorebase_actual =
				core->multicorebase_actual + 0x800;
			(*auxcore)->multicorebase =
				(*auxcore)->multicorebase_actual;
			(*auxcore)->iosize = core->iosize;
			if (!request_mem_region(
				    (*auxcore)->multicorebase_actual,
				    (*auxcore)->iosize, "ftv310_vpu_dec0")) {
				pr_info("ftv310_vpu_dec: failed to reserve HW regs\n");
				result = -EBUSY;
				vfree(*auxcore);
				*auxcore = NULL;
				goto error;
			}

			(*auxcore)->hwregs =
				(u8 *)ioremap((*auxcore)->multicorebase_actual,
					      (*auxcore)->iosize);

			if (!((*auxcore)->hwregs)) {
				pr_info("ftv310_vpu_dec: failed to ioremap HW regs\n");
				release_mem_region(
					(*auxcore)->multicorebase_actual,
					(*auxcore)->iosize);
				vfree(*auxcore);
				*auxcore = NULL;
				result = -EBUSY;
				goto error;
			}

			core->its_aux_core_id = *auxcore;
			(*auxcore)->its_main_core_id = core;
			(*auxcore)->its_aux_core_id = NULL;
		}
	}

	pr_info("ftv310_vpu_dec: VCD has build id 0x%08x\n",
	       readl(core->hwregs + FTV310_VPUDEC_HWBUILD_ID_OFF));

	if (*auxcore) {
		result = CheckHwId(*auxcore);
		if (!result) {
			result = -ENXIO;
			goto error;
		}
	}

	return 0;

error:
	ReleaseIO(core);
	if (*auxcore) {
		ReleaseIO(*auxcore);
		vfree(*auxcore);
		*auxcore = NULL;
	}
	return result;
}

/*---------------------------------------------------------------------------
 *Function name   : releaseIO
 *Description     : release
 *
 *Return type     : void
 *---------------------------------------------------------------------------
 */
static void ReleaseIO(struct ftv310_vpu_dec_t *dev)
{
	if (dev->hwregs)
		iounmap((void *)dev->hwregs);
	release_mem_region(dev->multicorebase_actual, dev->iosize);
}

/*---------------------------------------------------------------------------
 *Function name   : ftv310_vpu_dec_isr
 *Description     : interrupt handler
 *
 *Return type     : irqreturn_t
 *---------------------------------------------------------------------------
 */
static irqreturn_t ftv310_vpu_dec_isr(int irq, void *dev_id)
{
	unsigned long flags;
	unsigned int handled = 0;
	int i = 0;

	u8 *hwregs;
	struct ftv310_vpu_dec_t *dev = (struct ftv310_vpu_dec_t *)dev_id;
	u32 irq_status_dec;
	struct slice_info *parentslice = getparentslice(dev, FTV310_VPU_CORE_DEC);

	dev = getfirst_decnodes(parentslice);
	spin_lock_irqsave(&parentslice->owner_lock, flags);

	while (dev) {
		u8 *hwregs = dev->hwregs;

		/* interrupt status register read */
		irq_status_dec =
			ioread32((void *)hwregs + FTV310_VPUDEC_IRQ_STAT_DEC_OFF);
		pr_info("irq = %x\n", irq_status_dec);
		if (irq_status_dec & FTV310_VPUDEC_DEC_IRQ) {
			/* clear dec IRQ */
			irq_status_dec &= (~FTV310_VPUDEC_DEC_IRQ);
			iowrite32(irq_status_dec,
				  (void *)hwregs + FTV310_VPUDEC_IRQ_STAT_DEC_OFF);

			PDEBUG("decoder IRQ received! Core %d\n", i);
#ifdef PHY_CONFIG_PM
			dev->hw_active = 0;
#endif

			atomic_inc(&irq_rx);

			parentslice->dec_irq |= (1 << i);

			wake_up_interruptible_all(&parentslice->dec_wait_queue);
			handled++;
		}
		i++;
		dev = dev->next;
	}

	spin_unlock_irqrestore(&parentslice->owner_lock, flags);

	if (!handled)
		pr_info("IRQ received, but not ftv310_vpu_dec's!\n");

	(void)hwregs;
	return IRQ_RETVAL(handled);
}

/*---------------------------------------------------------------------------
 *Function name   : ResetAsic
 *Description     : reset asic
 *
 *Return type     :
 *---------------------------------------------------------------------------
 */
static void ResetAsic(struct ftv310_vpu_dec_t *dev)
{
#ifndef SIMICS_TEST
	int i;
	u32 status;

	status = ioread32((void *)dev->hwregs + FTV310_VPUDEC_IRQ_STAT_DEC_OFF);

	if (status & FTV310_VPUDEC_DEC_E) {
		/* abort with IRQ disabled */
		status = FTV310_VPUDEC_DEC_ABORT | FTV310_VPUDEC_DEC_IRQ_DISABLE;
		iowrite32(status,
			  (void *)dev->hwregs + FTV310_VPUDEC_IRQ_STAT_DEC_OFF);
	}

	if (IS_G1(dev->hw_id))
		/* reset PP */
		iowrite32(0, (void *)dev->hwregs + FTV310_VPU_IRQ_STAT_PP_OFF);

	for (i = 4; i < dev->iosize; i += 4)
		iowrite32(0, (void *)dev->hwregs + i);
#endif
}
