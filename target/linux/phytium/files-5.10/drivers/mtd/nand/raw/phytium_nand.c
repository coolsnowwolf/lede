// SPDX-License-Identifier: GPL-2.0
/*
 * Core driver for Phytium NAND flash controller
 *
 * Copyright (c) 2020-2023 Phytium Technology Co., Ltd.
 */

#include <linux/module.h>
#include <linux/mtd/rawnand.h>
#include <linux/iopoll.h>
#include <linux/interrupt.h>
#include <linux/mfd/syscon.h>
#include <linux/regmap.h>
#include <linux/dma-mapping.h>
#include <linux/workqueue.h>
#include <linux/time.h>
#include <linux/mutex.h>

#include "phytium_nand.h"

#define NFC_DATA_CMD_DEBUG 0

u16 timing_asy_mode0[TIMING_ASY_NUM] = {	/* x100 pass, sample: 1 */
	0x03, 0x03, 0x28, 0x28, 0x03, 0x03, 0x06, 0x06, 0x28, 0x70, 0x30, 0x50};
u16 timing_asy_mode1[TIMING_ASY_NUM] = {	/* x100 pass, sample: 1 */
	0x03, 0x03, 0x14, 0x14, 0x03, 0x03, 0x06, 0x06, 0x14, 0x70, 0x30, 0x28};
u16 timing_asy_mode2[TIMING_ASY_NUM] = {	/* x100 pass, sample: 7/8 (unlic) */
	0x03, 0x03, 0x0D, 0x0D, 0x03, 0x03, 0x06, 0x06, 0x0D, 0x70, 0x20, 0x1A};
u16 timing_asy_mode3[TIMING_ASY_NUM] = {	/* x100 pass, sample: 4-7 */
	0x03, 0x03, 0x0A, 0x0A, 0x03, 0x03, 0x06, 0x06, 0x0A, 0x70, 0x20, 0x14};
u16 timing_asy_mode4[TIMING_ASY_NUM] = {	/* x100 1.8v pass */
	0x03, 0x03, 0x08, 0x08, 0x03, 0x03, 0x06, 0x06, 0x08, 0x70, 0x15, 0x10};
u16 timing_asy_mode5[TIMING_ASY_NUM] = {	/* x100 1.8v pass */
	0x03, 0x03, 0x07, 0x07, 0x03, 0x03, 0x06, 0x06, 0x07, 0x20, 0x15, 0x0E};
u16 timing_syn_mode0[TIMING_SYN_NUM] = {	/* x100 1.8v pass */
	0x20, 0x41, 0x05, 0x20, 0x10, 0x19, 0x62, 0x40, 0x38, 0x20, 0x00, 0x09,
	0x50, 0x20};
u16 timing_syn_mode1[TIMING_SYN_NUM] = {	/* x100 1.8v pass */
	0x18, 0x32, 0x06, 0x18, 0x0C, 0x10, 0x76, 0x40, 0x2A, 0x1E, 0x00, 0x12,
	0x24, 0x18};
u16 timing_syn_mode2[TIMING_SYN_NUM] = {	/* x100 1.8v pass */
	0x10, 0x0A, 0x04, 0x10, 0x08, 0x0A, 0x6E, 0x50, 0x1D, 0x10, 0x00, 0x0C,
	0x18, 0x10};
u16 timing_syn_mode3[TIMING_SYN_NUM] = {	/* x100 1.8v pass */
	0x0C, 0x1A, 0x02, 0x0C, 0x06, 0x08, 0x78, 0x7C, 0x15, 0x0C, 0x00, 0x08,
	0x12, 0x0C};
u16 timing_syn_mode4[TIMING_SYN_NUM] = {	/* x100 1.8v failed */
	0x08, 0x17, 0x05, 0x08, 0x04, 0x01, 0x73, 0x40, 0x0C, 0x08, 0x00, 0x06,
	0x0C, 0x10};
u16 timing_tog_ddr_mode0[TIMING_TOG_NUM] = {	/* 600M clk */
	0x14, 0x0a, 0x08, 0x08, 0xc8, 0xc8, 0x08, 0x08, 0x20, 0x0a, 0x14, 0x08};

static u32 nfc_ecc_errover;
static u32 nfc_ecc_err;
static u32 nfc_irq_st;
static u32 nfc_irq_en;
static u32 nfc_irq_complete;

static DECLARE_WAIT_QUEUE_HEAD(wait_done);

/*
 * Internal helper to conditionnally apply a delay (from the above structure,
 * most of the time).
 */
static void cond_delay(unsigned int ns)
{
	if (!ns)
		return;

	if (ns < 10000)
		ndelay(ns);
	else
		udelay(DIV_ROUND_UP(ns, 1000));
}

static inline struct phytium_nfc *to_phytium_nfc(struct nand_controller *ctrl)
{
	return container_of(ctrl, struct phytium_nfc, controller);
}

static inline struct phytium_nand_chip *to_phytium_nand(struct nand_chip *chip)
{
	return container_of(chip, struct phytium_nand_chip, chip);
}

static u32 phytium_read(struct phytium_nfc *nfc, u32 reg)
{
	return readl_relaxed(nfc->regs + reg);
}

static void phytium_write(struct phytium_nfc *nfc, u32 reg, u32 value)
{
	return writel_relaxed(value, nfc->regs + reg);
}

static inline int phytium_wait_busy(struct phytium_nfc *nfc)
{
	u32 status;

	if (nfc_ecc_errover) {
		nfc_ecc_errover = 0;
		return 0;
	}

	return readl_relaxed_poll_timeout(nfc->regs + NDSR, status,
			!(status & NDSR_BUSY), 10, 10000);
}

static void phytium_nfc_disable_int(struct phytium_nfc *nfc, u32 int_mask)
{
	u32 reg;

	reg = phytium_read(nfc, NDIR_MASK);
	phytium_write(nfc, NDIR_MASK, reg | int_mask);
}

static void phytium_nfc_enable_int(struct phytium_nfc *nfc, u32 int_mask)
{
	u32 reg;

	reg = phytium_read(nfc, NDIR_MASK);
	phytium_write(nfc, NDIR_MASK, reg & (~int_mask));
}

static void phytium_nfc_clear_int(struct phytium_nfc *nfc, u32 int_mask)
{
	phytium_write(nfc, NDIR_MASK, int_mask);
}

static int phytium_nfc_cmd_correct(struct phytium_nfc_op *nfc_op)
{
	if (!nfc_op)
		return -EINVAL;

	if (nfc_op->cmd_len == 0x01) {
		nfc_op->cmd[1] = nfc_op->cmd[0];
		nfc_op->cmd[0] = 0;
	}

	return 0;
}

static int phytium_nfc_addr_correct(struct phytium_nfc_op *nfc_op)
{
	u32 len;
	int i, j;

	if (!nfc_op)
		return -EINVAL;

	len = nfc_op->addr_len > PHYTIUM_NFC_ADDR_MAX_LEN ?
		  PHYTIUM_NFC_ADDR_MAX_LEN : nfc_op->addr_len;

	if (len == PHYTIUM_NFC_ADDR_MAX_LEN)
		return 0;

	for (i = len-1, j = PHYTIUM_NFC_ADDR_MAX_LEN - 1; i >= 0; i--, j--) {
		nfc_op->addr[j] = nfc_op->addr[i];
		nfc_op->addr[i] = 0;
	}

	return 0;
}

static void phytium_nfc_parse_instructions(struct nand_chip *chip,
					   const struct nand_subop *subop,
					   struct phytium_nfc_op *nfc_op)
{
	struct nand_op_instr *instr = NULL;
	bool first_cmd = true;
	u32 op_id;
	int i;

	/* Reset the input structure as most of its fields will be OR'ed */
	memset(nfc_op, 0, sizeof(struct phytium_nfc_op));

	for (op_id = 0; op_id < subop->ninstrs; op_id++) {
		unsigned int offset, naddrs;
		const u8 *addrs;
		int len;

		instr = (struct nand_op_instr *)&subop->instrs[op_id];

		switch (instr->type) {
		case NAND_OP_CMD_INSTR:
			if (first_cmd) {
				nfc_op->cmd[0] = instr->ctx.cmd.opcode;
			} else {
				nfc_op->cmd[1] = instr->ctx.cmd.opcode;
				nfc_op->cmd_ctrl.nfc_ctrl.dbc = 1;
			}

			nfc_op->cle_ale_delay_ns = instr->delay_ns;
			first_cmd = false;
			nfc_op->cmd_len++;

			break;

		case NAND_OP_ADDR_INSTR:
			offset = nand_subop_get_addr_start_off(subop, op_id);
			naddrs = nand_subop_get_num_addr_cyc(subop, op_id);
			addrs = &instr->ctx.addr.addrs[offset];

			nfc_op->cmd_ctrl.nfc_ctrl.addr_cyc = naddrs;

			for (i = 0; i < min_t(u32, PHYTIUM_NFC_ADDR_MAX_LEN, naddrs); i++)
				nfc_op->addr[i] = addrs[i];

			nfc_op->cle_ale_delay_ns = instr->delay_ns;

			nfc_op->addr_len = naddrs;
			break;

		case NAND_OP_DATA_IN_INSTR:
			nfc_op->data_instr = instr;
			nfc_op->data_instr_idx = op_id;
			nfc_op->cmd_ctrl.nfc_ctrl.dc = 1;
			len = nand_subop_get_data_len(subop, op_id);
			nfc_op->page_cnt = len;
			nfc_op->data_delay_ns = instr->delay_ns;

			break;

		case NAND_OP_DATA_OUT_INSTR:
			nfc_op->data_instr = instr;
			nfc_op->data_instr_idx = op_id;
			nfc_op->cmd_ctrl.nfc_ctrl.dc = 1;
			len = nand_subop_get_data_len(subop, op_id);
			nfc_op->page_cnt = len;
			nfc_op->data_delay_ns = instr->delay_ns;
			break;

		case NAND_OP_WAITRDY_INSTR:
			nfc_op->rdy_timeout_ms = instr->ctx.waitrdy.timeout_ms;
			nfc_op->rdy_delay_ns = instr->delay_ns;
			break;
		}
	}
}

int phytium_nfc_prepare_cmd(struct nand_chip *chip,
			    struct phytium_nfc_op *nfc_op,
			    enum dma_data_direction direction)
{
	struct phytium_nand_chip *phytium_nand = to_phytium_nand(chip);
	struct phytium_nfc *nfc = to_phytium_nfc(chip->controller);
	int i;

	phytium_nfc_cmd_correct(nfc_op);
	phytium_nfc_addr_correct(nfc_op);

	nfc_op->cmd_ctrl.nfc_ctrl.csel = 0x0F ^ (0x01 << phytium_nand->selected_die);

	for (i = 0; i < PHYTIUM_NFC_ADDR_MAX_LEN; i++)
		nfc_op->mem_addr_first[i] = (nfc->dma_phy_addr >> (8 * i)) & 0xFF;

	return 0;
}
EXPORT_SYMBOL_GPL(phytium_nfc_prepare_cmd);

#if NFC_DATA_CMD_DEBUG

static int phytium_nfc_cmd_dump(struct phytium_nfc *nfc,
				struct phytium_nfc_op *nfc_op, u8 *buf)
{
	u8 *p;
	u8 str[1024] = {0};
	int i;

	sprintf(str, "Phytium NFC cmd dump:\n");
	sprintf(str, "%s cmd0:%x, cmd1:%x, ctrl:%x, page_cnt:%d\n",
		str, nfc_op->cmd[0], nfc_op->cmd[1], nfc_op->cmd_ctrl.ctrl, nfc_op->page_cnt);

	p = &nfc_op->addr[0];
	sprintf(str, "%s addr:%02x %02x %02x %02x %02x\n",
		str, p[0], p[1], p[2], p[3], p[4]);

	p = &nfc_op->mem_addr_first[0];
	sprintf(str, "%s mem_addr_first:%02x %02x %02x %02x %02x\n",
		str, p[0], p[1], p[2], p[3], p[4]);

	for (i = 0; i < PHYTIUM_NFC_DSP_SIZE; i++)
		sprintf(str, "%s %02x", str, buf[i]);

	dev_info(nfc->dev, "%s\n", str);

	return 0;
}

int phytium_nfc_data_dump(struct phytium_nfc *nfc, u8 *buf, u32 len)
{
	u8 str[1024] = {0};
	int i;

	len = len > 512 ? 512 : len;

	sprintf(str, "Phytium NFC data dump: %d\n", len);
	for (i = 0; i < len; i++) {
		if (i && (i%128 == 0)) {
			dev_info(nfc->dev, "next:\n%s\n", str);
			memset(str, 0, 1024);
		}

		if (i && (i%16 == 0))
			sprintf(str, "%s\n", str);
		sprintf(str, "%s %02x", str, buf[i]);
	}

	dev_info(nfc->dev, "%s\n", str);

	return 0;
}
EXPORT_SYMBOL_GPL(phytium_nfc_data_dump);
#else
#define phytium_nfc_cmd_dump(...)
#define phytium_nfc_data_dump(...)
#endif

int phytium_nfc_send_cmd(struct nand_chip *chip,
			 struct phytium_nfc_op *nfc_op)
{
	struct phytium_nfc *nfc = to_phytium_nfc(chip->controller);
	u32 value = 0;

	memset((u8 *)nfc->dsp_addr, 0, PAGE_SIZE);
	memcpy((u8 *)nfc->dsp_addr, (u8 *)nfc_op, PHYTIUM_NFC_DSP_SIZE);

	phytium_nfc_cmd_dump(nfc, nfc_op, (u8 *)nfc->dsp_addr);

	if (phytium_wait_busy(nfc) != 0) {
		dev_err(nfc->dev, "NFC was always busy\n");
		dev_err(nfc->dev, "NFC state: %x\n", phytium_read(nfc, NDSR));
		dev_err(nfc->dev, "NFC debug: %x\n", phytium_read(nfc, ND_DEBUG));
		return 0;
	}

	spin_lock(&nfc->spinlock);
	value = nfc->dsp_phy_addr & 0xFFFFFFFF;
	phytium_write(nfc, NDAR0, value);

	/* Don't modify NDAR1_DMA_RLEN & NDAR1_DMA_WLEN */
	value  = phytium_read(nfc, NDAR1);
	value |= NDAR1_H8((nfc->dsp_phy_addr >> 32) & 0xFF);
	phytium_write(nfc, NDAR1, value);

	phytium_nfc_enable_int(nfc,  NDIR_CMD_FINISH_MASK);

	value |= NDAR1_DMA_EN;
	phytium_write(nfc, NDAR1, value);
	spin_unlock(&nfc->spinlock);

	return 0;
}
EXPORT_SYMBOL_GPL(phytium_nfc_send_cmd);

int phytium_nfc_prepare_cmd2(struct nand_chip *chip,
			     struct phytium_nfc_op *nfc_op,
			     enum dma_data_direction direction,
			     u32 cmd_num)
{
	struct phytium_nand_chip *phytium_nand = to_phytium_nand(chip);
	int i;

	for (i = 0; i < cmd_num; i++) {
		phytium_nfc_cmd_correct(nfc_op);
		phytium_nfc_addr_correct(nfc_op);
		nfc_op->cmd_ctrl.nfc_ctrl.csel = 0x0F ^ (0x01 << phytium_nand->selected_die);
		nfc_op++;
	}

	return 0;
}
EXPORT_SYMBOL_GPL(phytium_nfc_prepare_cmd2);

int phytium_nfc_send_cmd2(struct nand_chip *chip,
			  struct phytium_nfc_op *nfc_op,
			  u32 cmd_num)
{
	struct phytium_nfc *nfc = to_phytium_nfc(chip->controller);
	u32 value = 0;
	int i;

	memset((u8 *)nfc->dsp_addr, 0, PAGE_SIZE);

	for (i = 0; i < cmd_num; i++) {
		memcpy((u8 *)nfc->dsp_addr + i*PHYTIUM_NFC_DSP_SIZE,
			(u8 *)nfc_op, PHYTIUM_NFC_DSP_SIZE);
		phytium_nfc_cmd_dump(nfc, nfc_op, (u8 *)nfc->dsp_addr + i*PHYTIUM_NFC_DSP_SIZE);
		nfc_op++;
	}

	if (phytium_wait_busy(nfc) != 0) {
		dev_err(nfc->dev, "NFC was always busy\n");
		dev_err(nfc->dev, "NFC state: %x\n", phytium_read(nfc, NDSR));
		dev_err(nfc->dev, "NFC debug: %x\n", phytium_read(nfc, ND_DEBUG));
		return 0;
	}

	spin_lock(&nfc->spinlock);
	value = nfc->dsp_phy_addr & 0xFFFFFFFF;
	phytium_write(nfc, NDAR0, value);

	/* Don't modify NDAR1_DMA_RLEN & NDAR1_DMA_WLEN */
	value  = phytium_read(nfc, NDAR1);
	value |= NDAR1_H8((nfc->dsp_phy_addr >> 32) & 0xFF);
	phytium_write(nfc, NDAR1, value);

	phytium_nfc_enable_int(nfc, NDIR_DMA_FINISH_MASK | NDIR_ECC_ERR_MASK);

	value |= NDAR1_DMA_EN;
	phytium_write(nfc, NDAR1, value);
	spin_unlock(&nfc->spinlock);

	return 0;
}
EXPORT_SYMBOL_GPL(phytium_nfc_send_cmd2);

int phytium_nfc_wait_op(struct nand_chip *chip,
			u32 timeout_ms)
{
	struct phytium_nfc *nfc = to_phytium_nfc(chip->controller);
	int ret;

	/* Timeout is expressed in ms */
	if (!timeout_ms)
		timeout_ms = IRQ_TIMEOUT;
	else if (timeout_ms > 1000)
		timeout_ms = 1000;
	else if (timeout_ms < 100)
		timeout_ms = 100;

	ret = wait_event_interruptible_timeout(wait_done, nfc_irq_complete,
					  msecs_to_jiffies(timeout_ms));
	nfc_irq_complete = false;

	if (!ret) {
		dev_err(nfc->dev, "Timeout waiting for RB signal\n");
		dev_err(nfc->dev, "NFC state: %x\n", phytium_read(nfc, NDSR));
		dev_err(nfc->dev, "NFC irq state: %x, irq en:%x\n",
			phytium_read(nfc, NDIR), phytium_read(nfc, NDIR_MASK));
		dev_err(nfc->dev, "NFC debug: %x\n", phytium_read(nfc, ND_DEBUG));

		phytium_nfc_clear_int(nfc, NDIR_ALL_INT(nfc->caps->int_mask_bits));
		return -ETIMEDOUT;
	}

	return 0;
}
EXPORT_SYMBOL_GPL(phytium_nfc_wait_op);

static int phytium_nfc_xfer_data_pio(struct phytium_nfc *nfc,
				     const struct nand_subop *subop,
				     struct phytium_nfc_op *nfc_op)
{
	const struct nand_op_instr *instr = nfc_op->data_instr;
	unsigned int op_id = nfc_op->data_instr_idx;
	unsigned int len = nand_subop_get_data_len(subop, op_id);
	unsigned int offset = nand_subop_get_data_start_off(subop, op_id);
	bool reading = (instr->type == NAND_OP_DATA_IN_INSTR);

	if (reading) {
		u8 *in = instr->ctx.data.buf.in + offset;

		memcpy(in, nfc->dma_buf, len);

		nfc->dma_offset = 0;
	} else {
		const u8 *out = instr->ctx.data.buf.out + offset;

		memcpy(nfc->dma_buf, out, len);
	}

	return 0;
}

static int memcpy_to_reg16(struct phytium_nfc *nfc, u32 reg, u16 *buf, size_t len)
{
	int i;
	u32 val = 0;

	if (!nfc || !buf || (len >= 16))
		return -EINVAL;

	for (i = 0; i < len; i++) {
		val = (val << 16) + buf[i];
		if (i % 2) {
			phytium_write(nfc, reg, val);
			val = 0;
			reg += 4;
		}
	}

	return 0;
}

int phytium_nfc_default_data_interface(struct phytium_nfc *nfc)
{
	int value;

	value = phytium_read(nfc, NDCR0);
	value &= (~NDCR0_IN_MODE(3));
	value |= NDCR0_IN_MODE(nfc->inter_mode);
	phytium_write(nfc, NDCR0, value);

	value = phytium_read(nfc, NDCR1);
	value &= (~NDCR1_SAMPL_PHASE(0xFFFF));

	switch (nfc->inter_mode) {
	case ASYN_SDR:
		if (nfc->timing_mode == ASY_MODE4) {
			memcpy_to_reg16(nfc, NDTR0, timing_asy_mode4, TIMING_ASY_NUM);
			phytium_write(nfc, NDCR1, value | NDCR1_SAMPL_PHASE(4));
		} else if (nfc->timing_mode == ASY_MODE3) {
			memcpy_to_reg16(nfc, NDTR0, timing_asy_mode3, TIMING_ASY_NUM);
			phytium_write(nfc, NDCR1, value | NDCR1_SAMPL_PHASE(5));
		} else if (nfc->timing_mode == ASY_MODE2) {
			memcpy_to_reg16(nfc, NDTR0, timing_asy_mode2, TIMING_ASY_NUM);
			phytium_write(nfc, NDCR1, value | NDCR1_SAMPL_PHASE(3));
		} else if (nfc->timing_mode == ASY_MODE1) {
			memcpy_to_reg16(nfc, NDTR0, timing_asy_mode1, TIMING_ASY_NUM);
			phytium_write(nfc, NDCR1, value | NDCR1_SAMPL_PHASE(2));
		} else {
			memcpy_to_reg16(nfc, NDTR0, timing_asy_mode0, TIMING_ASY_NUM);
			phytium_write(nfc, NDCR1, value | NDCR1_SAMPL_PHASE(1));
		}
		phytium_write(nfc, ND_INTERVAL_TIME, 0x01);
		break;
	case ONFI_DDR:
		if (nfc->timing_mode == SYN_MODE4) {
			memcpy_to_reg16(nfc, NDTR6, timing_syn_mode4, TIMING_SYN_NUM);
			phytium_write(nfc, NDCR1, value | NDCR1_SAMPL_PHASE(0x0D));
			phytium_write(nfc, ND_INTERVAL_TIME, 0x30);
		} else if (nfc->timing_mode == SYN_MODE3) {
			memcpy_to_reg16(nfc, NDTR6, timing_syn_mode3, TIMING_SYN_NUM);
			phytium_write(nfc, NDCR1, value | NDCR1_SAMPL_PHASE(0x05));
			phytium_write(nfc, ND_INTERVAL_TIME, 0x18);
		} else if (nfc->timing_mode == SYN_MODE2) {
			memcpy_to_reg16(nfc, NDTR6, timing_syn_mode2, TIMING_SYN_NUM);
			phytium_write(nfc, NDCR1, value | NDCR1_SAMPL_PHASE(0x08));
			phytium_write(nfc, ND_INTERVAL_TIME, 0x20);
		} else if (nfc->timing_mode == SYN_MODE1) {
			memcpy_to_reg16(nfc, NDTR6, timing_syn_mode1, TIMING_SYN_NUM);
			phytium_write(nfc, NDCR1, value | NDCR1_SAMPL_PHASE(0x12));
			phytium_write(nfc, ND_INTERVAL_TIME, 0x40);
		} else {
			memcpy_to_reg16(nfc, NDTR6, timing_syn_mode0, TIMING_SYN_NUM);
			phytium_write(nfc, NDCR1, value | NDCR1_SAMPL_PHASE(0x12));
			phytium_write(nfc, ND_INTERVAL_TIME, 0x40);
		}
		break;
	case TOG_ASYN_DDR:
		phytium_write(nfc, NDCR1, value | NDCR1_SAMPL_PHASE(8));
		phytium_write(nfc, ND_INTERVAL_TIME, 0xC8);
		memcpy_to_reg16(nfc, NDTR13, timing_tog_ddr_mode0, TIMING_TOG_NUM);
		break;
	}

	return 0;
}
EXPORT_SYMBOL_GPL(phytium_nfc_default_data_interface);

static int phytium_nfc_naked_waitrdy_exec(struct nand_chip *chip,
					  const struct nand_subop *subop)
{
	struct phytium_nfc *nfc = to_phytium_nfc(chip->controller);
	struct phytium_nfc_op nfc_op;
	enum dma_data_direction direction;
	int ret = 0;

	phytium_nfc_parse_instructions(chip, subop, &nfc_op);

	dev_info(nfc->dev, "Phytium nand command 0x%02x 0x%02x.\n",
		 nfc_op.cmd[0], nfc_op.cmd[1]);

	switch (nfc_op.cmd[0]) {
	case NAND_CMD_PARAM:
		memset(nfc->dma_buf, 0, PAGE_SIZE);
		direction = DMA_FROM_DEVICE;
		nfc_op.cmd_ctrl.nfc_ctrl.cmd_type = TYPE_READ_PARAM;
		nfc_op.cmd_ctrl.nfc_ctrl.auto_rs  = 1;
		if (nfc->inter_pro == NAND_ONFI)
			nfc_op.page_cnt = 3 * sizeof(struct nand_onfi_params);
		else if (nfc->inter_pro == NAND_JEDEC)
			nfc_op.page_cnt = 3 * sizeof(struct nand_jedec_params);
		if (nfc_op.page_cnt)
			nfc_op.cmd_ctrl.nfc_ctrl.dc = 1;
		nfc->dma_offset = 0;
		break;
	case NAND_CMD_SET_FEATURES:
		direction = DMA_TO_DEVICE;
		nfc_op.cmd_ctrl.nfc_ctrl.cmd_type = TYPE_SET_FTR;
		nfc_op.cmd_ctrl.nfc_ctrl.auto_rs  = 1;
		nfc_op.cmd_ctrl.nfc_ctrl.dc = 1;
		if (nfc->inter_mode != ASYN_SDR) {
			dev_err(nfc->dev, "Not support SET_FEATURES command!\n");
			return 0;
		}
		break;
	case NAND_CMD_GET_FEATURES:
		direction = DMA_FROM_DEVICE;
		nfc_op.cmd_ctrl.nfc_ctrl.cmd_type = TYPE_GET_FTR;
		nfc_op.cmd_ctrl.nfc_ctrl.auto_rs  = 1;
		nfc_op.cmd_ctrl.nfc_ctrl.dc = 1;
		break;
	case NAND_CMD_READ0:
		if (nfc_op.cmd[1] == NAND_CMD_READSTART) { /* large page */
			direction = DMA_FROM_DEVICE;
			nfc_op.cmd_ctrl.nfc_ctrl.cmd_type = TYPE_READ;
			nfc_op.cmd_ctrl.nfc_ctrl.auto_rs  = 1;
		} else if (nfc_op.cmd[1] == NAND_CMD_SEQIN) { /* program page begin */
			nfc_op.cmd[0] = NAND_CMD_SEQIN;
			nfc_op.cmd[1] = NAND_CMD_PAGEPROG;
			direction = DMA_TO_DEVICE;
			nfc_op.cmd_ctrl.nfc_ctrl.cmd_type = TYPE_PAGE_PRO;
			nfc_op.cmd_ctrl.nfc_ctrl.auto_rs  = 1;
		} else {   /* small page */
			direction = DMA_FROM_DEVICE;
			nfc_op.cmd_ctrl.nfc_ctrl.cmd_type = TYPE_READ;
			nfc_op.cmd_ctrl.nfc_ctrl.auto_rs  = 1;
		}
		break;
	case NAND_CMD_RNDOUT: /* change read column  */
		direction = DMA_FROM_DEVICE;
		nfc_op.cmd_ctrl.nfc_ctrl.cmd_type = TYPE_CH_READ_COL;
		nfc_op.cmd_ctrl.nfc_ctrl.dc = 1;
		break;
	case NAND_CMD_READSTART:   /* large page */
		direction = DMA_FROM_DEVICE;
		nfc_op.cmd_ctrl.nfc_ctrl.cmd_type = TYPE_READ;
		nfc_op.cmd_ctrl.nfc_ctrl.auto_rs  = 1;
		break;
	case NAND_CMD_RNDOUTSTART:  /* change read column  */
		direction = DMA_FROM_DEVICE;
		nfc_op.cmd_ctrl.nfc_ctrl.cmd_type = TYPE_CH_READ_COL;
		nfc_op.cmd_ctrl.nfc_ctrl.auto_rs  = 1;
		nfc->dma_offset = nfc_op.addr[1];
		nfc->dma_offset = (nfc->dma_offset << 8) + nfc_op.addr[0];
		break;
	case NAND_CMD_SEQIN:  /* program begin */
		if (nfc_op.cmd[0] == NAND_CMD_READ0) {
			nfc_op.cmd[0] = NAND_CMD_SEQIN;
			nfc_op.cmd[1] = NAND_CMD_PAGEPROG;
		}
		direction = DMA_TO_DEVICE;
		nfc_op.cmd_ctrl.nfc_ctrl.cmd_type = TYPE_PAGE_PRO;
		nfc_op.cmd_ctrl.nfc_ctrl.auto_rs  = 1;
		break;
	case NAND_CMD_RNDIN:  /* change write column  */
		direction = DMA_TO_DEVICE;
		nfc_op.cmd_ctrl.nfc_ctrl.cmd_type = TYPE_CH_WR_COL;
		nfc_op.cmd_ctrl.nfc_ctrl.auto_rs  = 1;
		break;
	case NAND_CMD_PAGEPROG: /* program end */
		nfc_op.cmd[0] = NAND_CMD_RNDIN;
		nfc_op.cmd[1] = NAND_CMD_PAGEPROG;
		direction = DMA_TO_DEVICE;
		nfc_op.cmd_ctrl.nfc_ctrl.cmd_type = TYPE_PAGE_PRO;
		nfc_op.cmd_ctrl.nfc_ctrl.auto_rs  = 1;
		break;
	default:
		dev_err(nfc->dev, "Not support cmd %d.\n", nfc_op.cmd[1]);
		ret = -EINVAL;
		goto out;
	}

	if ((nfc_op.data_instr) && (direction == DMA_TO_DEVICE))
		phytium_nfc_xfer_data_pio(nfc, subop, &nfc_op);

	phytium_nfc_prepare_cmd(chip, &nfc_op, direction);
	phytium_nfc_send_cmd(chip, &nfc_op);
	cond_delay(nfc_op.cle_ale_delay_ns);
	nfc_op.rdy_timeout_ms = nfc_op.rdy_timeout_ms;
	ret = phytium_nfc_wait_op(chip, nfc_op.rdy_timeout_ms);
	if (ret)
		goto out;

	cond_delay(nfc_op.rdy_delay_ns);

	if ((nfc_op.data_instr) && (direction == DMA_FROM_DEVICE))
		phytium_nfc_xfer_data_pio(nfc, subop, &nfc_op);

out:
	return ret;
}

static int phytium_nfc_read_id_type_exec(struct nand_chip *chip,
					 const struct nand_subop *subop)
{
	struct phytium_nfc *nfc = to_phytium_nfc(chip->controller);
	struct phytium_nfc_op nfc_op;
	enum dma_data_direction direction;
	u16 read_len = 0;
	int ret;
	u8 *buf = nfc->dma_buf;

	memset(nfc->dma_buf, 0, PAGE_SIZE);
	direction = DMA_FROM_DEVICE;

	phytium_nfc_parse_instructions(chip, subop, &nfc_op);
	read_len = nfc_op.page_cnt;
	nfc_op.page_cnt = (read_len & 0x03) ? ((read_len & 0xFFFC) + 4) : read_len;

	nfc_op.cmd_ctrl.nfc_ctrl.cmd_type = TYPE_READ_ID;
	nfc_op.cmd_ctrl.nfc_ctrl.auto_rs  = 0;

	phytium_nfc_prepare_cmd(chip, &nfc_op, direction);
	phytium_nfc_send_cmd(chip, &nfc_op);
	cond_delay(nfc_op.cle_ale_delay_ns);
	ret = phytium_nfc_wait_op(chip, nfc_op.rdy_timeout_ms);
	if (ret)
		return ret;

	cond_delay(nfc_op.rdy_delay_ns);

	if (!strncmp(nfc->dma_buf, "ONFI", 4)) {
		nfc->inter_pro = NAND_ONFI;
	} else if (!strncmp(nfc->dma_buf, "JEDEC", 5)) {
		nfc->inter_pro = NAND_JEDEC;
		if (buf[5] == 1)
			nfc->inter_mode = ASYN_SDR;
		else if (buf[5] == 2)
			nfc->inter_mode = TOG_ASYN_DDR;
		else if (buf[5] == 4)
			nfc->inter_mode = ASYN_SDR;
	} else {
		nfc->inter_pro = NAND_OTHER;
	}

	dev_info(nfc->dev, "Nand protocol: %d, interface mode: %d\n",
		 nfc->inter_pro, nfc->inter_mode);

	phytium_nfc_xfer_data_pio(nfc, subop, &nfc_op);

	return 0;
}

static int phytium_nfc_read_status_exec(struct nand_chip *chip,
					const struct nand_subop *subop)
{
	struct phytium_nfc *nfc = to_phytium_nfc(chip->controller);
	struct phytium_nfc_op nfc_op;
	enum dma_data_direction direction;
	u16 read_len = 0;
	u32 timeout, count = 0;
	int ret = 0;

	direction = DMA_FROM_DEVICE;

	phytium_nfc_parse_instructions(chip, subop, &nfc_op);
	read_len = nfc_op.page_cnt;
	nfc_op.page_cnt = (read_len & 0x03) ? ((read_len & 0xFFFC) + 4) : read_len;
	nfc_op.cmd_ctrl.nfc_ctrl.cmd_type = TYPE_READ_STATUS;
	phytium_nfc_prepare_cmd(chip, &nfc_op, direction);

read_status_retry:
	count++;
	phytium_nfc_send_cmd(chip, &nfc_op);
	cond_delay(nfc_op.cle_ale_delay_ns);
	timeout = nfc_op.rdy_timeout_ms ? nfc_op.rdy_timeout_ms : 10;
	ret = phytium_nfc_wait_op(chip, nfc_op.rdy_timeout_ms);
	if (ret)
		goto out;

	phytium_nfc_xfer_data_pio(nfc, subop, &nfc_op);

	if (0xE0 != *(u8 *)(nfc->dma_buf)) {
		dev_info(nfc->dev, "Retry to read status (%x)\n", *(u8 *)(nfc->dma_buf));

		if (count < 10)
			goto read_status_retry;
	}

out:
	return ret;
}

static int phytium_nfc_reset_cmd_type_exec(struct nand_chip *chip,
					   const struct nand_subop *subop)
{
	struct phytium_nfc_op nfc_op;
	enum dma_data_direction direction;

	phytium_nfc_parse_instructions(chip, subop, &nfc_op);

	direction = DMA_NONE;
	nfc_op.cmd_ctrl.nfc_ctrl.cmd_type = TYPE_RESET;
	nfc_op.cmd_ctrl.nfc_ctrl.auto_rs  = 1;
	phytium_nfc_prepare_cmd(chip, &nfc_op, direction);
	phytium_nfc_send_cmd(chip, &nfc_op);
	cond_delay(nfc_op.cle_ale_delay_ns);

	return phytium_nfc_wait_op(chip, nfc_op.rdy_timeout_ms);
}

static int phytium_nfc_erase_cmd_type_exec(struct nand_chip *chip,
					   const struct nand_subop *subop)
{
	struct phytium_nfc_op nfc_op;
	enum dma_data_direction direction;

	phytium_nfc_parse_instructions(chip, subop, &nfc_op);
	direction = DMA_NONE;
	nfc_op.cmd_ctrl.nfc_ctrl.cmd_type = TYPE_ERASE;
	nfc_op.cmd_ctrl.nfc_ctrl.auto_rs  = 1;
	phytium_nfc_prepare_cmd(chip, &nfc_op, direction);
	phytium_nfc_send_cmd(chip, &nfc_op);
	cond_delay(nfc_op.cle_ale_delay_ns);

	return phytium_nfc_wait_op(chip, nfc_op.rdy_timeout_ms);
}

static int phytium_nfc_data_in_type_exec(struct nand_chip *chip,
					 const struct nand_subop *subop)
{
	struct phytium_nfc *nfc = to_phytium_nfc(chip->controller);
	struct phytium_nfc_op nfc_op;
	struct nand_op_instr *instr;
	unsigned int op_id;
	unsigned int len;
	unsigned int offset;
	u8 *in = NULL;

	phytium_nfc_parse_instructions(chip, subop, &nfc_op);
	if (nfc_op.data_instr->type != NAND_OP_DATA_IN_INSTR) {
		dev_err(nfc->dev, "Phytium nfc instrs parser failed!\n");
		return -EINVAL;
	}

	instr = nfc_op.data_instr;
	op_id = nfc_op.data_instr_idx;
	len = nand_subop_get_data_len(subop, op_id);
	offset = nand_subop_get_data_start_off(subop, op_id);
	in = instr->ctx.data.buf.in + offset;

	phytium_nfc_cmd_dump(nfc, &nfc_op, (u8 *)nfc->dsp_addr);

	memcpy(in, nfc->dma_buf + nfc->dma_offset, len);
	nfc->dma_offset += len;
	phytium_nfc_data_dump(nfc, in, len);

	return 0;
}

static const struct nand_op_parser phytium_nfc_op_parser = NAND_OP_PARSER(
	/* Naked commands not supported, use a function for each pattern */
	NAND_OP_PARSER_PATTERN(
		phytium_nfc_read_id_type_exec,
		NAND_OP_PARSER_PAT_CMD_ELEM(false),
		NAND_OP_PARSER_PAT_ADDR_ELEM(false, PHYTIUM_NFC_ADDR_MAX_LEN),
		NAND_OP_PARSER_PAT_DATA_IN_ELEM(false, 8)),
	NAND_OP_PARSER_PATTERN(
		phytium_nfc_erase_cmd_type_exec,
		NAND_OP_PARSER_PAT_CMD_ELEM(false),
		NAND_OP_PARSER_PAT_ADDR_ELEM(false, PHYTIUM_NFC_ADDR_MAX_LEN),
		NAND_OP_PARSER_PAT_CMD_ELEM(false),
		NAND_OP_PARSER_PAT_WAITRDY_ELEM(false)),
	NAND_OP_PARSER_PATTERN(
		phytium_nfc_read_status_exec,
		NAND_OP_PARSER_PAT_CMD_ELEM(false),
		NAND_OP_PARSER_PAT_DATA_IN_ELEM(false, 1)),
	NAND_OP_PARSER_PATTERN(
		phytium_nfc_reset_cmd_type_exec,
		NAND_OP_PARSER_PAT_CMD_ELEM(false),
		NAND_OP_PARSER_PAT_WAITRDY_ELEM(false)),
	NAND_OP_PARSER_PATTERN(
		phytium_nfc_naked_waitrdy_exec,
		NAND_OP_PARSER_PAT_CMD_ELEM(false),
		NAND_OP_PARSER_PAT_ADDR_ELEM(false, PHYTIUM_NFC_ADDR_MAX_LEN),
		NAND_OP_PARSER_PAT_WAITRDY_ELEM(false),
		NAND_OP_PARSER_PAT_DATA_IN_ELEM(false, MAX_CHUNK_SIZE)),
	NAND_OP_PARSER_PATTERN(
		phytium_nfc_naked_waitrdy_exec,
		NAND_OP_PARSER_PAT_CMD_ELEM(false),
		NAND_OP_PARSER_PAT_ADDR_ELEM(false, PHYTIUM_NFC_ADDR_MAX_LEN),
		NAND_OP_PARSER_PAT_WAITRDY_ELEM(false)),
	NAND_OP_PARSER_PATTERN(
		phytium_nfc_naked_waitrdy_exec,
		NAND_OP_PARSER_PAT_CMD_ELEM(false),
		NAND_OP_PARSER_PAT_ADDR_ELEM(false, PHYTIUM_NFC_ADDR_MAX_LEN),
		NAND_OP_PARSER_PAT_DATA_OUT_ELEM(false, 8),
		NAND_OP_PARSER_PAT_WAITRDY_ELEM(false)),
	NAND_OP_PARSER_PATTERN(
		phytium_nfc_naked_waitrdy_exec,
		NAND_OP_PARSER_PAT_CMD_ELEM(false),
		NAND_OP_PARSER_PAT_ADDR_ELEM(false, PHYTIUM_NFC_ADDR_MAX_LEN),
		NAND_OP_PARSER_PAT_CMD_ELEM(false),
		NAND_OP_PARSER_PAT_DATA_IN_ELEM(false, MAX_CHUNK_SIZE)),
	NAND_OP_PARSER_PATTERN(
		phytium_nfc_data_in_type_exec,
		NAND_OP_PARSER_PAT_DATA_IN_ELEM(false, MAX_CHUNK_SIZE)),
	);

static int phytium_nfc_exec_op(struct nand_chip *chip,
			       const struct nand_operation *op,
			       bool check_only)
{
	return nand_op_parser_exec_op(chip, &phytium_nfc_op_parser,
				      op, check_only);
}

static int phytium_nfc_reset(struct phytium_nfc *nfc)
{
	u32 value;

	phytium_write(nfc, NDIR_MASK, NDIR_ALL_INT(nfc->caps->int_mask_bits));
	phytium_write(nfc, NDSR, NDIR_ALL_INT(nfc->caps->int_mask_bits));

	phytium_write(nfc, ND_ERR_CLR, 0x0F);
	phytium_write(nfc, NDFIFO_CLR, 1);

	value = phytium_read(nfc, NDCR0);
	phytium_write(nfc, NDCR0, value & ~(NDCR0_ECC_EN | NDCR0_SPARE_EN));

	return 0;
}

static void phytium_nfc_select_chip(struct nand_chip *chip, int die_nr)
{
	struct phytium_nand_chip *phytium_nand = to_phytium_nand(chip);
	struct phytium_nfc *nfc = to_phytium_nfc(chip->controller);

	dev_dbg(nfc->dev, "Phytium nand selected chip %d\n", die_nr);

	if (chip == nfc->selected_chip && die_nr == phytium_nand->selected_die)
		return;

	if (die_nr < 0 || die_nr >= phytium_nand->nsels) {
		nfc->selected_chip = NULL;
		phytium_nand->selected_die = -1;
		return;
	}

	phytium_nfc_reset(nfc);

	nfc->selected_chip = chip;
	phytium_nand->selected_die = die_nr;
}

static int phytium_nand_ooblayout_ecc(struct mtd_info *mtd, int section,
				      struct mtd_oob_region *oobregion)
{
	struct nand_chip *chip = mtd_to_nand(mtd);

	if (section)
		return -ERANGE;

	oobregion->length = chip->ecc.total;
	oobregion->offset = mtd->oobsize - oobregion->length;

	return 0;
}

static int phytium_nand_ooblayout_free(struct mtd_info *mtd, int section,
				       struct mtd_oob_region *oobregion)
{
	struct nand_chip *chip = mtd_to_nand(mtd);

	if (section)
		return -ERANGE;

	/*
	 * Bootrom looks in bytes 0 & 5 for bad blocks for the
	 * 4KB page / 4bit BCH combination.
	 */
	if (mtd->writesize >= SZ_4K)
		oobregion->offset = 6;
	else
		oobregion->offset = 2;

	oobregion->length = mtd->oobsize - chip->ecc.total - oobregion->offset;

	return 0;
}

static const struct mtd_ooblayout_ops phytium_nand_ooblayout_ops = {
	.ecc = phytium_nand_ooblayout_ecc,
	.free = phytium_nand_ooblayout_free,
};

static void phytium_nfc_enable_hw_ecc(struct nand_chip *chip)
{
	struct phytium_nfc *nfc = to_phytium_nfc(chip->controller);
	u32 ndcr0 = phytium_read(nfc, NDCR0);

	if (!(ndcr0 & NDCR0_ECC_EN))
		phytium_write(nfc, NDCR0, ndcr0 | NDCR0_ECC_EN);
}

static void phytium_nfc_disable_hw_ecc(struct nand_chip *chip)
{
	struct phytium_nfc *nfc = to_phytium_nfc(chip->controller);
	u32 ndcr0 = phytium_read(nfc, NDCR0);

	if (ndcr0 & NDCR0_ECC_EN)
		phytium_write(nfc, NDCR0, ndcr0 & ~NDCR0_ECC_EN);
}


irqreturn_t phytium_nfc_isr(int irq, void *dev_id)
{
	struct phytium_nfc *nfc = dev_id;
	u32 st = phytium_read(nfc, NDIR);
	u32 ien = (~phytium_read(nfc, NDIR_MASK)) & NDIR_ALL_INT(nfc->caps->int_mask_bits);

	if (!(st & ien))
		return IRQ_NONE;

	nfc_irq_st = st;
	nfc_irq_en = ien;
	phytium_nfc_disable_int(nfc, st & NDIR_ALL_INT(nfc->caps->int_mask_bits));
	phytium_write(nfc, 0xFD0, 0);

	if (st & (NDIR_CMD_FINISH | NDIR_DMA_FINISH)) {
		if (st & NDIR_ECC_ERR)
			nfc_ecc_err = 1;
		phytium_write(nfc, NDIR, st);
		nfc_irq_complete = 1;
	} else if (st & (NDIR_FIFO_TIMEOUT | NDIR_PGFINISH)) {
		phytium_write(nfc, NDIR, st);
		phytium_nfc_enable_int(nfc, (~st) & (NDIR_DMA_FINISH_MASK |
						     NDIR_PGFINISH_MASK |
						     NDIR_FIFO_TIMEOUT_MASK |
						     NDIR_CMD_FINISH_MASK));
		nfc_irq_complete = 0;
	} else if (st & NDIR_ECC_ERR) {
		phytium_write(nfc, ND_ERR_CLR, 0x08);
		phytium_write(nfc, NDIR, st);
		phytium_write(nfc, NDFIFO_CLR, 0x01);
		nfc_irq_complete = 1;
		nfc_ecc_errover = 1;
	} else {
		phytium_write(nfc, NDIR, st);
		nfc_irq_complete = 1;
	}

	wake_up(&wait_done);

	return IRQ_HANDLED;
}
EXPORT_SYMBOL(phytium_nfc_isr);

static int phytium_nfc_hw_ecc_correct(struct nand_chip *chip,
				char *buf, int len)
{
	struct phytium_nfc *nfc = to_phytium_nfc(chip->controller);
	u32 i, j, value, tmp;
	u32 ecc_err_reg_nums;
	int stat = 0;

	if (!buf)
		return -EINVAL;

	if (nfc->caps->hw_ver == 1)
		ecc_err_reg_nums = 2;
	else
		ecc_err_reg_nums = 4;

	for (i = 0; i < chip->ecc.steps; i++) {
		for (j = 0; j < ecc_err_reg_nums; j++) {
			value = phytium_read(nfc, 0xB8 + 4 * (ecc_err_reg_nums * i + j));
			dev_info(nfc->dev, "ECC_FLAG: offset:%x value:0x%08x\n",
				 0xB8 + 4 * (2 * i + j), value);

			tmp = value & 0xFFFF;
			if (tmp && (tmp <= 4096)) {
				tmp--;
				stat++;
				buf[chip->ecc.size * i + (tmp >> 3)] ^= (0x01 << tmp % 8);
				dev_info(nfc->dev, "ECC_CORRECT %x %02x\n",
					 chip->ecc.size * i + (tmp >> 3),
					 buf[chip->ecc.size * i + (tmp >> 3)]);
				dev_info(nfc->dev, "ECC_CORRECT xor %x %02x\n",
					 0x01 << (tmp % 8), buf[chip->ecc.size * i + (tmp >> 3)]);
			} else if (tmp > 4096) {
				dev_info(nfc->dev, "ECC_CORRECT offset > 4096!\n");
			}

			tmp = (value >> 16) & 0xFFFF;
			if (tmp && (tmp <= 4096)) {
				tmp--;
				stat++;
				buf[chip->ecc.size * i + (tmp >> 3)] ^= (0x01 << tmp % 8);
				dev_info(nfc->dev, "ECC_CORRECT %x %02x\n",
					 chip->ecc.size * i + (tmp >> 3),
					 buf[chip->ecc.size * i + (tmp >> 3)]);
				dev_info(nfc->dev, "ECC_CORRECT xor %x %02x\n",
					 chip->ecc.size * i + (tmp >> 3),
					 buf[chip->ecc.size * i + (tmp >> 3)]);
			} else if (tmp > 4096) {
				dev_info(nfc->dev, "ECC_CORRECT offset > 4096!\n");
			}
		}
	}

	return stat;
}

static int phytium_nand_page_read(struct mtd_info *mtd, struct nand_chip *chip,
				  u8 *buf, u8 *oob_buf, int oob_len, int page,
				  bool read)
{
	struct phytium_nfc *nfc = to_phytium_nfc(chip->controller);
	struct phytium_nand_chip *phytium_nand = NULL;
	const struct nand_sdr_timings *sdr = NULL;
	struct phytium_nfc_op nfc_op;
	enum dma_data_direction direction;
	int ret = 0;

	memset(&nfc_op, 0, sizeof(nfc_op));
	phytium_nand = to_phytium_nand(chip);
	sdr = nand_get_sdr_timings(nand_get_interface_config(chip));

	memset(nfc->dma_buf, 0x0, mtd->writesize + mtd->oobsize);
	direction  = DMA_FROM_DEVICE;
	nfc_op.cmd_ctrl.nfc_ctrl.cmd_type = TYPE_READ;
	nfc_op.cmd[0] = NAND_CMD_READ0;
	nfc_op.cmd[1] = NAND_CMD_READSTART;
	nfc_op.cmd_len  = 2;
	nfc_op.cle_ale_delay_ns = PSEC_TO_NSEC(sdr->tWB_max);
	nfc_op.rdy_timeout_ms = PSEC_TO_MSEC(sdr->tR_max);
	nfc_op.rdy_delay_ns = PSEC_TO_NSEC(sdr->tRR_min);

	nfc_op.cmd_ctrl.nfc_ctrl.dbc = 1;
	nfc_op.addr[2] = page;
	nfc_op.addr[3] = page >> 8;
	nfc_op.addr[4] = page >> 16;
	nfc_op.addr_len = 5;
	nfc_op.cmd_ctrl.nfc_ctrl.addr_cyc = 0x05;
	nfc_op.cmd_ctrl.nfc_ctrl.auto_rs = 1;

	nfc_op.page_cnt = mtd->writesize;
	nfc_op.cmd_ctrl.nfc_ctrl.dc = 1;
	nfc_op.cmd_ctrl.nfc_ctrl.ecc_en = 0;

	/* For data read/program */
	phytium_nfc_prepare_cmd(chip, &nfc_op, direction);
	phytium_nfc_send_cmd(chip, &nfc_op);
	cond_delay(nfc_op.cle_ale_delay_ns);

	ret = phytium_nfc_wait_op(chip, nfc_op.rdy_timeout_ms);
	if (ret)
		return ret;

	if ((direction == DMA_FROM_DEVICE) && buf)
		memcpy(buf, nfc->dma_buf, mtd->writesize);

	return ret;
}

static int phytium_nand_oob_read(struct mtd_info *mtd, struct nand_chip *chip,
				 u8 *buf, u8 *oob_buf, int oob_len, int page,
				 bool read)
{
	struct phytium_nfc *nfc = to_phytium_nfc(chip->controller);
	struct phytium_nand_chip *phytium_nand = NULL;
	const struct nand_sdr_timings *sdr = NULL;
	struct phytium_nfc_op nfc_op;
	enum dma_data_direction direction;
	int ret = 0;

	memset(&nfc_op, 0, sizeof(nfc_op));
	phytium_nand = to_phytium_nand(chip);
	sdr = nand_get_sdr_timings(nand_get_interface_config(chip));

	memset(nfc->dma_buf, 0x00, mtd->writesize + mtd->oobsize);
	direction  = DMA_FROM_DEVICE;
	nfc_op.cmd_ctrl.nfc_ctrl.cmd_type = TYPE_READ;
	nfc_op.cmd[0] = NAND_CMD_READ0;
	nfc_op.cmd[1] = NAND_CMD_READSTART;
	nfc_op.cmd_len  = 2;
	nfc_op.cle_ale_delay_ns = PSEC_TO_NSEC(sdr->tWB_max);
	nfc_op.rdy_timeout_ms = PSEC_TO_MSEC(sdr->tR_max);
	nfc_op.rdy_delay_ns = PSEC_TO_NSEC(sdr->tRR_min);

	nfc_op.cmd_ctrl.nfc_ctrl.dbc = 1;
	nfc_op.addr[2] = page;
	nfc_op.addr[3] = page >> 8;
	nfc_op.addr[4] = page >> 16;
	nfc_op.addr_len = 5;
	nfc_op.cmd_ctrl.nfc_ctrl.addr_cyc = 0x05;
	nfc_op.cmd_ctrl.nfc_ctrl.auto_rs = 1;

	nfc_op.page_cnt = oob_len;
	nfc_op.cmd_ctrl.nfc_ctrl.dc = 1;
	nfc_op.cmd_ctrl.nfc_ctrl.ecc_en = 0;
	nfc_op.addr[0] = mtd->writesize & 0xFF;
	nfc_op.addr[1] = (mtd->writesize >> 8) & 0xFF;

	/* For data read/program */
	phytium_nfc_prepare_cmd(chip, &nfc_op, direction);
	phytium_nfc_send_cmd(chip, &nfc_op);
	cond_delay(nfc_op.cle_ale_delay_ns);

	ret = phytium_nfc_wait_op(chip, nfc_op.rdy_timeout_ms);
	if (ret)
		return ret;

	cond_delay(nfc_op.rdy_delay_ns);

	if (direction == DMA_FROM_DEVICE)
		memcpy(oob_buf, nfc->dma_buf, oob_len);

	return ret;
}

static int phytium_nand_get_ecc_total(struct mtd_info *mtd,
				 struct nand_ecc_ctrl *ecc)
{
	int ecc_total = 0;

	switch (mtd->writesize) {
	case 0x200:
		if (ecc->strength == 8)
			ecc_total = 0x0D;
		else if (ecc->strength == 4)
			ecc_total = 7;
		else if (ecc->strength == 2)
			ecc_total = 4;
		else
			ecc_total = 0;
		break;
	case 0x800:
		if (ecc->strength == 8)
			ecc_total = 0x34;
		else if (ecc->strength == 4)
			ecc_total = 0x1a;
		else if (ecc->strength == 2)
			ecc_total = 0xd;
		else
			ecc_total = 0;
	break;
	case 0x1000:
		if (ecc->strength == 8)
			ecc_total = 0x68;
		else if (ecc->strength == 4)
			ecc_total = 0x34;
		else if (ecc->strength == 2)
			ecc_total = 0x1a;
		else
			ecc_total = 0;
		break;
	case 0x2000:
		if (ecc->strength == 8)
			ecc_total = 0xD0;
		else if (ecc->strength == 4)
			ecc_total = 0x68;
		else if (ecc->strength == 2)
			ecc_total = 0x34;
		else
			ecc_total = 0;
		break;
	case 0x4000:
		if (ecc->strength == 8)
			ecc_total = 0x1A0;
		if (ecc->strength == 4)
			ecc_total = 0xD0;
		else if (ecc->strength == 2)
			ecc_total = 0x68;
		else
			ecc_total = 0;
		break;
	default:
			ecc_total = 0;
		break;
	}

	return ecc_total;
}

static int phytium_nand_page_read_hwecc(struct mtd_info *mtd, struct nand_chip *chip,
					u8 *buf, u8 *oob_buf, int oob_len, int page,
					bool read)
{
	struct phytium_nfc *nfc = to_phytium_nfc(chip->controller);
	struct phytium_nand_chip *phytium_nand = NULL;
	const struct nand_sdr_timings *sdr = NULL;
	struct phytium_nfc_op *nfc_op = NULL;
	enum dma_data_direction direction;
	u32 ecc_offset;
	int max_bitflips = 0;
	u32 nfc_state = 0;
	int ret = 0;
	int i;

	phytium_nand = to_phytium_nand(chip);
	sdr = nand_get_sdr_timings(nand_get_interface_config(chip));

	ecc_offset = phytium_nand->ecc.offset;
	memset(nfc->dma_buf, 0x00, mtd->writesize + mtd->oobsize);
	nfc_op = kzalloc(2 * sizeof(struct phytium_nfc_op), GFP_KERNEL);
	if (!nfc_op) {
		dev_err(nfc->dev, "Can't malloc space for phytium_nfc_op\n");
		return 0;
	}

	nfc_op->cle_ale_delay_ns = PSEC_TO_NSEC(sdr->tWB_max);
	nfc_op->rdy_timeout_ms = PSEC_TO_MSEC(sdr->tR_max);
	nfc_op->rdy_delay_ns = PSEC_TO_NSEC(sdr->tRR_min);

	direction  = DMA_FROM_DEVICE;
	nfc_op->cmd_ctrl.nfc_ctrl.cmd_type = TYPE_READ;
	nfc_op->cmd[0] = NAND_CMD_READ0;
	nfc_op->cmd[1] = NAND_CMD_READSTART;
	nfc_op->cmd_len  = 2;
	nfc_op->addr_len = 5;
	nfc_op->cmd_ctrl.nfc_ctrl.dbc = 1;
	nfc_op->addr[2] = page;
	nfc_op->addr[3] = page >> 8;
	nfc_op->addr[4] = page >> 16;
	nfc_op->cmd_ctrl.nfc_ctrl.addr_cyc = 0x05;
	nfc_op->cmd_ctrl.nfc_ctrl.dc = 1;
	nfc_op->cmd_ctrl.nfc_ctrl.auto_rs = 1;
	nfc_op->page_cnt = mtd->writesize;
	nfc_op->cmd_ctrl.nfc_ctrl.nc = 1;
	for (i = 0; i < PHYTIUM_NFC_ADDR_MAX_LEN; i++)
		nfc_op->mem_addr_first[i] = (nfc->dma_phy_addr >> (8 * i)) & 0xFF;

	nfc_op++;
	memcpy(nfc_op, nfc_op - 1, sizeof(struct phytium_nfc_op));
	nfc_op->cmd_ctrl.nfc_ctrl.cmd_type = TYPE_CH_READ_COL;
	nfc_op->cmd[0] = NAND_CMD_RNDOUT;
	nfc_op->cmd[1] = NAND_CMD_RNDOUTSTART;
	memset(&nfc_op->addr, 0, PHYTIUM_NFC_ADDR_MAX_LEN);
	nfc_op->addr_len = 2;
	nfc_op->addr[0] = mtd->writesize + phytium_nand->ecc.offset;
	nfc_op->addr[1] = (mtd->writesize + phytium_nand->ecc.offset) >> 8;
	nfc_op->cmd_ctrl.nfc_ctrl.addr_cyc = 0x02;
	nfc_op->page_cnt = phytium_nand_get_ecc_total(mtd, &chip->ecc);
	nfc_op->cmd_ctrl.nfc_ctrl.nc = 0;
	nfc_op->cmd_ctrl.nfc_ctrl.auto_rs = 0;
	nfc_op->cmd_ctrl.nfc_ctrl.ecc_en = 1;
	for (i = 0; i < PHYTIUM_NFC_ADDR_MAX_LEN; i++)
		nfc_op->mem_addr_first[i] =
			((nfc->dma_phy_addr + mtd->writesize) >> (8 * i)) & 0xFF;

	nfc_op--;
	phytium_nfc_prepare_cmd2(chip, nfc_op, direction, 2);
	phytium_nfc_send_cmd2(chip, nfc_op, 2);
	cond_delay(nfc_op->cle_ale_delay_ns);

	ret = phytium_nfc_wait_op(chip, nfc_op->rdy_timeout_ms);
	if (ret){
		kfree(nfc_op);
		return ret;
	}

	cond_delay(nfc_op->rdy_delay_ns*1000);

	if ((direction == DMA_FROM_DEVICE) && buf) {
		nfc_state = phytium_read(nfc, NDSR);
		if ((nfc_state & NDSR_ECC_ERROVER) || (nfc_ecc_errover == 1)) {
			for (i = 0; i < mtd->writesize/16; i++) {
				if (0xFF != *(u8 *)(nfc->dma_buf + i)) {
					dev_info(nfc->dev, "NFC: NDSR_ECC_ERROVER %x\n", page);
					mtd->ecc_stats.failed++;
					mtd->ecc_stats.corrected += max_bitflips;
					break;
				}
			}
		} else if (nfc_state & NDSR_ECC_ERR) {
			max_bitflips = phytium_nfc_hw_ecc_correct(chip,
						nfc->dma_buf, mtd->writesize);
			mtd->ecc_stats.corrected += max_bitflips;
			dev_info(nfc->dev, "NFC: NDSR_ECC_ERR page:%x, bit:%d\n",
						page, max_bitflips);
		}

		memcpy(buf, nfc->dma_buf, mtd->writesize);
	}

	kfree(nfc_op);
	return max_bitflips;
}

static int phytium_nand_page_write(struct mtd_info *mtd, struct nand_chip *chip,
				   const u8 *buf, u8 *oob_buf, int oob_len, int page,
				   bool read)
{
	struct phytium_nfc *nfc = to_phytium_nfc(chip->controller);
	struct phytium_nand_chip *phytium_nand = NULL;
	const struct nand_sdr_timings *sdr = NULL;
	struct phytium_nfc_op nfc_op;
	enum dma_data_direction direction;
	int ret = 0;

	memset(&nfc_op, 0, sizeof(nfc_op));
	phytium_nand = to_phytium_nand(chip);
	sdr = nand_get_sdr_timings(nand_get_interface_config(chip));

	memcpy(nfc->dma_buf, buf, mtd->writesize);
	direction  = DMA_TO_DEVICE;
	nfc_op.cmd_ctrl.nfc_ctrl.cmd_type = TYPE_PAGE_PRO;
	nfc_op.cmd[0] = NAND_CMD_SEQIN;
	nfc_op.cmd[1] = NAND_CMD_PAGEPROG;
	nfc_op.cmd_len  = 2;
	nfc_op.addr_len = 5;
	nfc_op.cle_ale_delay_ns = PSEC_TO_NSEC(sdr->tWB_max);
	nfc_op.rdy_timeout_ms = PSEC_TO_MSEC(sdr->tPROG_max);
	nfc_op.rdy_delay_ns = 0;

	nfc_op.cmd_ctrl.nfc_ctrl.dbc = 1;
	nfc_op.addr[2] = page;
	nfc_op.addr[3] = page >> 8;
	nfc_op.addr[4] = page >> 16;
	nfc_op.cmd_ctrl.nfc_ctrl.addr_cyc = 0x05;
	nfc_op.cmd_ctrl.nfc_ctrl.dc = 1;
	nfc_op.cmd_ctrl.nfc_ctrl.auto_rs = 1;
	nfc_op.page_cnt = mtd->writesize;

	/* For data read/program */
	phytium_nfc_prepare_cmd(chip, &nfc_op, direction);
	phytium_nfc_send_cmd(chip, &nfc_op);
	cond_delay(nfc_op.cle_ale_delay_ns);

	ret = phytium_nfc_wait_op(chip, nfc_op.rdy_timeout_ms);
	if (ret)
		goto out;

	cond_delay(nfc_op.rdy_delay_ns);
out:
	return ret;
}

static int phytium_nand_oob_write(struct mtd_info *mtd, struct nand_chip *chip,
				  u8 *buf, u8 *oob_buf, int oob_len, int page,
				  bool read)
{
	struct phytium_nfc *nfc = to_phytium_nfc(chip->controller);
	struct phytium_nand_chip *phytium_nand = NULL;
	const struct nand_sdr_timings *sdr = NULL;
	struct phytium_nfc_op nfc_op;
	enum dma_data_direction direction;
	int ret = 0;

	memset(&nfc_op, 0, sizeof(nfc_op));
	phytium_nand = to_phytium_nand(chip);
	sdr = nand_get_sdr_timings(nand_get_interface_config(chip));

	direction  = DMA_TO_DEVICE;
	nfc_op.cmd_ctrl.nfc_ctrl.cmd_type = TYPE_PAGE_PRO;
	nfc_op.cmd[0] = NAND_CMD_SEQIN;
	nfc_op.cmd[1] = NAND_CMD_PAGEPROG;
	nfc_op.cmd_len  = 2;
	nfc_op.addr_len = 5;
	nfc_op.cle_ale_delay_ns = PSEC_TO_NSEC(sdr->tWB_max);
	nfc_op.rdy_timeout_ms = PSEC_TO_MSEC(sdr->tPROG_max);
	nfc_op.rdy_delay_ns = 0;

	nfc_op.cmd_ctrl.nfc_ctrl.dbc = 1;
	nfc_op.addr[2] = page;
	nfc_op.addr[3] = page >> 8;
	nfc_op.addr[4] = page >> 16;
	nfc_op.cmd_ctrl.nfc_ctrl.addr_cyc = 0x05;
	nfc_op.cmd_ctrl.nfc_ctrl.dc = 1;
	nfc_op.cmd_ctrl.nfc_ctrl.auto_rs = 1;

	nfc_op.page_cnt = oob_len;
	nfc_op.cmd_ctrl.nfc_ctrl.ecc_en = 0;
	nfc_op.addr[0] = mtd->writesize & 0xFF;
	nfc_op.addr[1] = (mtd->writesize >> 8) & 0xFF;
	nfc_op.cmd_ctrl.nfc_ctrl.ecc_en = 0;
	memcpy(nfc->dma_buf, oob_buf, mtd->oobsize);

	/* For data read/program */
	phytium_nfc_prepare_cmd(chip, &nfc_op, direction);
	phytium_nfc_send_cmd(chip, &nfc_op);
	cond_delay(nfc_op.cle_ale_delay_ns);

	ret = phytium_nfc_wait_op(chip, nfc_op.rdy_timeout_ms);
	if (ret)
		goto out;

	cond_delay(nfc_op.rdy_delay_ns);
out:
	return ret;
}

static int phytium_nand_page_write_hwecc(struct mtd_info *mtd, struct nand_chip *chip,
					 const u8 *buf, u8 *oob_buf, int oob_len, int page,
					 bool read)
{
	struct phytium_nfc *nfc = to_phytium_nfc(chip->controller);
	struct phytium_nand_chip *phytium_nand = NULL;
	const struct nand_sdr_timings *sdr = NULL;
	struct phytium_nfc_op *nfc_op;
	enum dma_data_direction direction;
	u32 ecc_offset;
	int ret = 0;
	int i;

	phytium_nand = to_phytium_nand(chip);
	sdr = nand_get_sdr_timings(nand_get_interface_config(chip));
	ecc_offset = phytium_nand->ecc.offset;

	nfc_op = kzalloc(2 * sizeof(struct phytium_nfc_op), GFP_KERNEL);
	if (!nfc_op) {
		dev_err(nfc->dev, "Can't malloc space for phytium_nfc_op\n");
		return 0;
	}

	nfc_op->cle_ale_delay_ns = PSEC_TO_NSEC(sdr->tWB_max);
	nfc_op->rdy_timeout_ms = PSEC_TO_MSEC(sdr->tR_max);
	nfc_op->rdy_delay_ns = PSEC_TO_NSEC(sdr->tRR_min);

	direction = DMA_TO_DEVICE;
	nfc_op->cmd_ctrl.nfc_ctrl.cmd_type = TYPE_CH_ROW_ADDR;
	nfc_op->cmd[0] = NAND_CMD_SEQIN;
	nfc_op->cmd_len  = 1;
	nfc_op->addr_len = 5;
	nfc_op->cmd_ctrl.nfc_ctrl.dbc = 0;
	nfc_op->addr[2] = page;
	nfc_op->addr[3] = page >> 8;
	nfc_op->addr[4] = page >> 16;
	nfc_op->cmd_ctrl.nfc_ctrl.addr_cyc = 0x05;
	nfc_op->cmd_ctrl.nfc_ctrl.auto_rs = 0;
	nfc_op->cmd_ctrl.nfc_ctrl.nc = 1;
	for (i = 0; i < PHYTIUM_NFC_ADDR_MAX_LEN; i++)
		nfc_op->mem_addr_first[i] = (nfc->dma_phy_addr >> (8 * i)) & 0xFF;

	/* The first dsp must have data to transfer */
	memcpy(nfc->dma_buf, buf, mtd->writesize);
	nfc_op->page_cnt = mtd->writesize;
	nfc_op->cmd_ctrl.nfc_ctrl.dc = 1;

	nfc_op++;
	memcpy(nfc_op, nfc_op - 1, sizeof(struct phytium_nfc_op));
	nfc_op->cmd_ctrl.nfc_ctrl.cmd_type = TYPE_PAGE_PRO;
	nfc_op->cmd_ctrl.nfc_ctrl.dbc = 1;
	nfc_op->cmd_ctrl.nfc_ctrl.auto_rs = 1;
	nfc_op->cmd[0] = NAND_CMD_RNDIN;
	nfc_op->cmd[1] = NAND_CMD_PAGEPROG;
	memset(&nfc_op->addr, 0, PHYTIUM_NFC_ADDR_MAX_LEN);
	nfc_op->addr_len = 2;
	nfc_op->cmd_len = 2;
	nfc_op->addr[0] = mtd->writesize + ecc_offset;
	nfc_op->addr[1] = (mtd->writesize + ecc_offset) >> 8;
	nfc_op->cmd_ctrl.nfc_ctrl.addr_cyc = 0x02;
	nfc_op->page_cnt = phytium_nand_get_ecc_total(mtd, &chip->ecc);
	nfc_op->cmd_ctrl.nfc_ctrl.nc = 0;
	nfc_op->cmd_ctrl.nfc_ctrl.dc = 1;
	nfc_op->cmd_ctrl.nfc_ctrl.ecc_en = 1;
	for (i = 0; i < PHYTIUM_NFC_ADDR_MAX_LEN; i++)
		nfc_op->mem_addr_first[i] =
			((nfc->dma_phy_addr + mtd->writesize + ecc_offset) >> (8 * i)) & 0xFF;

	/* when enable ECC, must offer ecc_offset of oob, but no oobdata */
	nfc_op--;
	phytium_nfc_prepare_cmd2(chip, nfc_op, direction, 2);
	phytium_nfc_send_cmd2(chip, nfc_op, 2);
	cond_delay(nfc_op->cle_ale_delay_ns);

	ret = phytium_nfc_wait_op(chip, nfc_op->rdy_timeout_ms);
	if (ret)
		goto out;

	cond_delay(nfc_op->rdy_delay_ns*1000);
out:
	kfree(nfc_op);
	return ret;
}

static int phytium_nfc_hw_ecc_bch_read_page_raw(struct nand_chip *chip,
						u8 *buf, int oob_required, int page)
{
	struct mtd_info *mtd = nand_to_mtd(chip);
	u32 oob_len = oob_required ? mtd->oobsize : 0;
	int ret;

	ret = phytium_nand_page_read(mtd, chip, buf, NULL, 0, page, true);
	if (oob_required)
		ret = phytium_nand_oob_read(mtd, chip, NULL, chip->oob_poi,
					    oob_len, page, true);

	phytium_nfc_data_dump(to_phytium_nfc(chip->controller), buf, mtd->writesize);

	return ret;
}

static int phytium_nfc_hw_ecc_bch_read_oob_raw(struct nand_chip *chip, int page)
{
	int ret;
	struct mtd_info *mtd = nand_to_mtd(chip);

	/* Invalidate page cache */
	chip->pagecache.page = -1;
	memset(chip->oob_poi, 0xFF, mtd->oobsize);

	ret = phytium_nand_oob_read(mtd, chip, NULL, chip->oob_poi,
				    mtd->oobsize, page, true);

	phytium_nfc_data_dump(to_phytium_nfc(chip->controller), chip->oob_poi, mtd->oobsize);

	return ret;
}

static int phytium_nfc_hw_ecc_bch_read_page(struct nand_chip *chip,
					    u8 *buf, int oob_required, int page)
{
	int ret;
	struct mtd_info *mtd = nand_to_mtd(chip);
	u32  oob_len = oob_required ? mtd->oobsize : 0;
	struct phytium_nand_chip *phytium_nand = NULL;

	phytium_nand = to_phytium_nand(chip);

	phytium_nfc_enable_hw_ecc(chip);
	cond_delay(20*1000);

	ret = phytium_nand_page_read_hwecc(mtd, chip, buf, NULL,
					   0, page, true);

	phytium_nfc_disable_hw_ecc(chip);

	if (oob_required) {
		oob_len = mtd->oobsize;
		ret = phytium_nand_oob_read(mtd, chip, NULL, chip->oob_poi,
					    oob_len, page, true);
	}

	phytium_nfc_data_dump(to_phytium_nfc(chip->controller), buf, mtd->writesize);

	return ret;
}

static int phytium_nfc_hw_ecc_bch_read_oob(struct nand_chip *chip,
					    int page)
{
	struct mtd_info *mtd = nand_to_mtd(chip);
	u32 oob_len = mtd->oobsize;
	int ret;

	ret = phytium_nand_oob_read(mtd, chip, NULL, chip->oob_poi,
				    oob_len, page, true);

	phytium_nfc_data_dump(to_phytium_nfc(chip->controller), chip->oob_poi, oob_len);

	return ret;
}

static int phytium_nfc_hw_ecc_bch_write_page_raw(struct nand_chip *chip,
						 const u8 *buf, int oob_required, int page)
{
	void *oob_buf = oob_required ? chip->oob_poi : NULL;
	struct mtd_info *mtd = nand_to_mtd(chip);

	if (oob_required)
		phytium_nand_oob_write(mtd, chip, NULL, oob_buf,
				       mtd->oobsize, page, false);

	return phytium_nand_page_write(mtd, chip, buf, NULL,
				       0, page, false);
}

static int phytium_nfc_hw_ecc_bch_write_page(struct nand_chip *chip,
					     const u8 *buf, int oob_required, int page)
{
	int ret;
	void *oob_buf = oob_required ? chip->oob_poi : NULL;
	u32 oob_len;
	struct mtd_info *mtd = nand_to_mtd(chip);

	if (oob_required) {
		oob_len = mtd->oobsize;
		phytium_nand_oob_write(mtd, chip, NULL, oob_buf,
				       oob_len, page, false);
	}

	phytium_nfc_enable_hw_ecc(chip);

	cond_delay(20*1000);

	ret = phytium_nand_page_write_hwecc(mtd, chip, buf, NULL,
					    0, page, false);

	phytium_nfc_disable_hw_ecc(chip);

	return ret;
}

static int phytium_nfc_hw_ecc_bch_write_oob_raw(struct nand_chip *chip, int page)
{
	struct mtd_info *mtd = nand_to_mtd(chip);
	return phytium_nand_oob_write(mtd, chip, NULL, chip->oob_poi,
				      mtd->oobsize, page, false);
}

static int phytium_nfc_hw_ecc_bch_write_oob(struct nand_chip *chip, int page)
{
	struct phytium_nand_chip *phytium_nand = to_phytium_nand(chip);
	struct mtd_info *mtd = nand_to_mtd(chip);
	u32 oob_len = mtd->oobsize - phytium_nand->ecc.length;

	return phytium_nand_oob_write(mtd, chip, NULL, chip->oob_poi,
				      oob_len, page, false);
}

static int phytium_nand_hw_ecc_ctrl_init(struct mtd_info *mtd,
					 struct nand_ecc_ctrl *ecc)
{
	struct nand_chip *chip = mtd_to_nand(mtd);

	if ((mtd->writesize + mtd->oobsize > MAX_CHUNK_SIZE))
		return -ENOTSUPP;

	chip->ecc.algo = NAND_ECC_ALGO_BCH;
	ecc->read_page_raw = phytium_nfc_hw_ecc_bch_read_page_raw;
	ecc->read_page = phytium_nfc_hw_ecc_bch_read_page;
	ecc->read_oob_raw = phytium_nfc_hw_ecc_bch_read_oob_raw;
	ecc->read_oob = phytium_nfc_hw_ecc_bch_read_oob;
	ecc->write_page_raw = phytium_nfc_hw_ecc_bch_write_page_raw;
	ecc->write_page = phytium_nfc_hw_ecc_bch_write_page;
	ecc->write_oob_raw = phytium_nfc_hw_ecc_bch_write_oob_raw;
	ecc->write_oob = phytium_nfc_hw_ecc_bch_write_oob;

	return 0;
}

static int phytium_nand_ecc_init(struct mtd_info *mtd,
				 struct nand_ecc_ctrl *ecc)
{
	int ret = 0;

	mtd_set_ooblayout(mtd, &phytium_nand_ooblayout_ops);

	switch (ecc->engine_type) {
	case NAND_ECC_ENGINE_TYPE_ON_HOST:
		ret = phytium_nand_hw_ecc_ctrl_init(mtd, ecc);
		break;
	case NAND_ECC_ENGINE_TYPE_NONE:
		ecc->read_page_raw = phytium_nfc_hw_ecc_bch_read_page_raw;
		ecc->read_oob_raw = phytium_nfc_hw_ecc_bch_read_oob;
		ecc->write_page_raw = phytium_nfc_hw_ecc_bch_write_page_raw;
		ecc->write_oob_raw = phytium_nfc_hw_ecc_bch_write_oob_raw;
		ecc->read_page = ecc->read_page_raw;
		ecc->read_oob = ecc->read_oob_raw;
		ecc->write_page = ecc->write_page_raw;
		ecc->write_oob = ecc->write_oob_raw;
		break;
	case NAND_ECC_ENGINE_TYPE_SOFT:
	case NAND_ECC_ENGINE_TYPE_ON_DIE:
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

static u8 bbt_pattern[] = {'P', 'H', 'Y', 'b', 't', '0' };
static u8 bbt_mirror_pattern[] = {'1', 't', 'b', 'Y', 'H', 'P' };

static struct nand_bbt_descr bbt_main_descr = {
	.options = NAND_BBT_LASTBLOCK | NAND_BBT_CREATE | NAND_BBT_WRITE |
		   NAND_BBT_2BIT | NAND_BBT_VERSION,
	.offs =	8,
	.len = 6,
	.veroffs = 14,
	.maxblocks = 8,	/* Last 8 blocks in each chip */
	.pattern = bbt_pattern
};

static struct nand_bbt_descr bbt_mirror_descr = {
	.options = NAND_BBT_LASTBLOCK | NAND_BBT_CREATE | NAND_BBT_WRITE |
		   NAND_BBT_2BIT | NAND_BBT_VERSION,
	.offs =	8,
	.len = 6,
	.veroffs = 14,
	.maxblocks = 8,	/* Last 8 blocks in each chip */
	.pattern = bbt_mirror_pattern
};

static int phytium_nand_attach_chip(struct nand_chip *chip)
{
	struct mtd_info *mtd = nand_to_mtd(chip);
	struct phytium_nand_chip *phytium_nand = to_phytium_nand(chip);
	struct phytium_nfc *nfc = to_phytium_nfc(chip->controller);
	u32 value;
	int ret = 0;

	if (nfc->caps->flash_bbt)
		chip->bbt_options |= NAND_BBT_USE_FLASH;

	if (chip->bbt_options & NAND_BBT_USE_FLASH) {
		/*
		 * We'll use a bad block table stored in-flash and don't
		 * allow writing the bad block marker to the flash.
		 */
		chip->bbt_options |= NAND_BBT_NO_OOB_BBM;
		chip->bbt_td = &bbt_main_descr;
		chip->bbt_md = &bbt_mirror_descr;
	}

	if (chip->options & NAND_BUSWIDTH_16)
		phytium_nand->ndcr |= NDCR0_WIDTH;

	/*
	 * On small page NANDs, only one cycle is needed to pass the
	 * column address.
	 */
	if (mtd->writesize <= 512)
		phytium_nand->addr_cyc = 1;
	else
		phytium_nand->addr_cyc = 2;

	/*
	 * Now add the number of cycles needed to pass the row
	 * address.
	 *
	 * Addressing a chip using CS 2 or 3 should also need the third row
	 * cycle but due to inconsistance in the documentation and lack of
	 * hardware to test this situation, this case is not supported.
	 */
	if (chip->options & NAND_ROW_ADDR_3)
		phytium_nand->addr_cyc += 3;
	else
		phytium_nand->addr_cyc += 2;

	if (nfc->caps) {
		if (chip->ecc.engine_type == NAND_ECC_ENGINE_TYPE_ON_HOST) {
			chip->ecc.size = nfc->caps->ecc_step_size ?
					 nfc->caps->ecc_step_size :
					 chip->ecc.size;
			chip->ecc.strength = nfc->caps->ecc_strength ?
					     nfc->caps->ecc_strength :
					     chip->ecc.strength;
			chip->ecc.strength = nfc->caps->ecc_strength;
			chip->ecc.bytes = 7;
		} else {
			chip->ecc.size = 512;
			chip->ecc.strength = 1;
			chip->ecc.bytes = 0;
		}
		chip->ecc.engine_type = NAND_ECC_ENGINE_TYPE_ON_HOST;
	}

	if (nfc->caps->hw_ver == 1) {
		if (chip->ecc.strength == 0x04)
			phytium_nand->ndcr |= NDCR0_ECC_STREN(4);
		else if (chip->ecc.strength == 0x02)
			phytium_nand->ndcr |= NDCR0_ECC_STREN(2);
		else
			phytium_nand->ndcr |= NDCR0_ECC_STREN(0);
	} else {
		if (chip->ecc.strength == 0x08)
			phytium_nand->ndcr |= NDCR0_ECC_STREN(7);
		else if (chip->ecc.strength == 0x04)
			phytium_nand->ndcr |= NDCR0_ECC_STREN(3);
		else if (chip->ecc.strength == 0x02)
			phytium_nand->ndcr |= NDCR0_ECC_STREN(1);
		else
			phytium_nand->ndcr |= NDCR0_ECC_STREN(0);
	}

	value = phytium_read(nfc, NDCR0);
	value &= ~NDCR0_EN;
	phytium_write(nfc, NDCR0, value);

	value &= ~NDCR0_ECC_STREN(7);
	value |= phytium_nand->ndcr;
	phytium_write(nfc, NDCR0, value | NDCR0_EN);

	ret = phytium_nand_ecc_init(mtd, &chip->ecc);
	if (ret) {
		dev_err(nfc->dev, "ECC init failed: %d\n", ret);
		goto out;
	}

	/*
	 * Subpage write not available with hardware ECC, prohibit also
	 * subpage read as in userspace subpage access would still be
	 * allowed and subpage write, if used, would lead to numerous
	 * uncorrectable ECC errors.
	 */
	if (chip->ecc.engine_type == NAND_ECC_ENGINE_TYPE_ON_HOST)
		chip->options |= NAND_NO_SUBPAGE_WRITE;

	/*
	 * We keep the MTD name unchanged to avoid breaking platforms
	 * where the MTD cmdline parser is used and the bootloader
	 * has not been updated to use the new naming scheme.
	 */
	if (nfc->caps->legacy_of_bindings)
		mtd->name = "phytium_nand-0";

out:
	return ret;
}

static int phytium_nfc_setup_interface(struct nand_chip *chip, int chipnr,
					    const struct nand_interface_config *conf)
{
	struct phytium_nfc *nfc = to_phytium_nfc(chip->controller);
	unsigned int period_ns = 2;
	const struct nand_sdr_timings *sdr;
	struct phytium_nfc_timings nfc_tmg;
	int read_delay;

	sdr = nand_get_sdr_timings(conf);
	if (IS_ERR(sdr))
		return PTR_ERR(sdr);

	nfc_tmg.tRP = TO_CYCLES(DIV_ROUND_UP(sdr->tRC_min, 2), period_ns) - 1;
	nfc_tmg.tRH = nfc_tmg.tRP;
	nfc_tmg.tWP = TO_CYCLES(DIV_ROUND_UP(sdr->tWC_min, 2), period_ns) - 1;
	nfc_tmg.tWH = nfc_tmg.tWP;
	nfc_tmg.tCS = TO_CYCLES(sdr->tCS_min, period_ns);
	nfc_tmg.tCH = TO_CYCLES(sdr->tCH_min, period_ns) - 1;
	nfc_tmg.tADL = TO_CYCLES(sdr->tADL_min, period_ns);
	dev_info(nfc->dev, "[nfc_tmg]tRP: %d, tRH:%d, tWP:%d tWH:%d\n",
		nfc_tmg.tRP, nfc_tmg.tRH, nfc_tmg.tWP, nfc_tmg.tWH);
	dev_info(nfc->dev, "[nfc_tmg]tCS: %d, tCH:%d, tADL:%d\n",
		nfc_tmg.tCS, nfc_tmg.tCH, nfc_tmg.tADL);

	read_delay = sdr->tRC_min >= 30000 ?
		MIN_RD_DEL_CNT : MIN_RD_DEL_CNT + nfc_tmg.tRH;

	nfc_tmg.tAR = TO_CYCLES(sdr->tAR_min, period_ns);
	nfc_tmg.tWHR = TO_CYCLES(max_t(int, sdr->tWHR_min, sdr->tCCS_min),
				 period_ns) - 2,
	nfc_tmg.tRHW = TO_CYCLES(max_t(int, sdr->tRHW_min, sdr->tCCS_min),
				 period_ns);
	dev_info(nfc->dev, "[nfc_tmg]tAR: %d, tWHR:%d, tRHW:%d\n",
		nfc_tmg.tAR, nfc_tmg.tWHR, nfc_tmg.tRHW);

	nfc_tmg.tR = TO_CYCLES(sdr->tWB_max, period_ns);

	if (chipnr < 0)
		return 0;

	if (nfc_tmg.tWP > 0x10)
		nfc->timing_mode = ASY_MODE1;
	else if (nfc_tmg.tWP < 0x0D)
		nfc->timing_mode = ASY_MODE3;

	if (nfc->inter_mode == ONFI_DDR)
		nfc->timing_mode = SYN_MODE3;

	phytium_nfc_default_data_interface(nfc);

	return 0;
}

static const struct nand_controller_ops phytium_nand_controller_ops = {
	.attach_chip = phytium_nand_attach_chip,
	.exec_op = phytium_nfc_exec_op,
	.setup_interface = phytium_nfc_setup_interface,
};

static void phytium_nand_chips_cleanup(struct phytium_nfc *nfc)
{
	struct phytium_nand_chip *entry, *temp;
	int ret;

	list_for_each_entry_safe(entry, temp, &nfc->chips, node) {
		ret = mtd_device_unregister(nand_to_mtd(&entry->chip));
		WARN_ON(ret);
		nand_cleanup(&entry->chip);
		list_del(&entry->node);
	}
}

static int phytium_nfc_init_dma(struct phytium_nfc *nfc)
{
	int ret;

	ret = dma_set_mask_and_coherent(nfc->dev, DMA_BIT_MASK(64));
	if (ret)
		return ret;

	nfc->dsp_addr = dma_alloc_coherent(nfc->dev, PAGE_SIZE,
					&nfc->dsp_phy_addr, GFP_KERNEL | GFP_DMA);
	if (!nfc->dsp_addr)
		return -ENOMEM;

	nfc->dma_buf = dma_alloc_coherent(nfc->dev, MAX_CHUNK_SIZE,
					&nfc->dma_phy_addr, GFP_KERNEL | GFP_DMA);
	if (!nfc->dma_buf)
		return -ENOMEM;

	dev_info(nfc->dev, "NFC address dsp_phy_addr:%llx, dma_phy_addr:%llx\n",
					nfc->dsp_phy_addr, nfc->dma_phy_addr);

	return 0;
}

int phytium_nfc_init(struct phytium_nfc *nfc)
{
	u32 value;

	nfc->inter_mode = ASYN_SDR;
	nfc->timing_mode = ASY_MODE0;

	value = phytium_read(nfc, NDCR1);
	value &= (~NDCR1_SAMPL_PHASE(0xFFFF));
	value &= ~NDCR1_ECC_DATA_FIRST_EN;
	value |= NDCR1_SAMPL_PHASE(1);
	value |= NDCR1_ECC_BYPASS;
	phytium_write(nfc, NDCR1, value);
	phytium_write(nfc, ND_INTERVAL_TIME, 1);
	phytium_write(nfc, NDFIFO_LEVEL0, 4);
	phytium_write(nfc, NDFIFO_LEVEL1, 4);
	phytium_write(nfc, NDFIFO_CLR, 1);
	phytium_write(nfc, ND_ERR_CLR, 1);

	/* Configure the DMA  */
	phytium_nfc_init_dma(nfc);



	phytium_nfc_reset(nfc);

	value = phytium_read(nfc, NDCR0);
	value &= (~NDCR0_IN_MODE(3));
	value |= NDCR0_IN_MODE(nfc->inter_mode);
	value |= NDCR0_EN;

	phytium_write(nfc, NDCR0, value);

	nfc_ecc_errover = 0;

	return 0;
}
EXPORT_SYMBOL_GPL(phytium_nfc_init);

static int phytium_nand_chip_init(struct phytium_nfc *nfc)
{
	struct device *dev = nfc->dev;
	struct phytium_nand_chip *phytium_nand;
	struct mtd_info *mtd;
	struct nand_chip *chip;
	int ret;

	/* Alloc the nand chip structure */
	phytium_nand = devm_kzalloc(dev, sizeof(*phytium_nand), GFP_KERNEL);
	if (!phytium_nand)
		return -ENOMEM;

	phytium_nand->nsels = 1;
	phytium_nand->selected_die = -1;

	chip = &phytium_nand->chip;
	chip->controller = &nfc->controller;
	chip->legacy.select_chip = phytium_nfc_select_chip;
	phytium_nfc_default_data_interface(nfc);

	mtd = nand_to_mtd(chip);
	mtd->dev.parent = dev;
	mtd->owner = THIS_MODULE;

	/*
	 * Default to HW ECC engine mode. If the nand-ecc-mode property is given
	 * in the DT node, this entry will be overwritten in nand_scan_ident().
	 */
	chip->ecc.engine_type = NAND_ECC_ENGINE_TYPE_ON_HOST;

	chip->options |= NAND_BUSWIDTH_AUTO;
	chip->options |= NAND_SKIP_BBTSCAN;
	chip->bbt_options |= NAND_BBT_NO_OOB;

	ret = nand_scan(chip, phytium_nand->nsels);
	if (ret) {
		dev_err(dev, "could not scan the nand chip\n");
		goto out;
	}

	if (nfc->caps->parts) {
		ret = mtd_device_register(mtd, nfc->caps->parts, nfc->caps->nr_parts - 1);
	} else if (dev->of_node) {
		nand_set_flash_node(chip, dev->of_node);
		ret = mtd_device_register(mtd, NULL, 0);
	} else {
		ret = -EINVAL;
	}

	if (ret) {
		dev_err(dev, "failed to register mtd device: %d\n", ret);
		nand_cleanup(chip);
		return ret;
	}

	phytium_nand->ecc.length = phytium_nand_get_ecc_total(mtd, &chip->ecc);
	phytium_nand->ecc.offset = mtd->oobsize - phytium_nand->ecc.length;
	chip->ecc.total = phytium_nand_get_ecc_total(mtd, &chip->ecc);

	mtd_ooblayout_ecc(mtd, 0, &phytium_nand->ecc);

	dev_info(dev, "ooblayout ecc offset: %x, length: %x\n",
		 phytium_nand->ecc.offset, phytium_nand->ecc.length);

out:
	list_add_tail(&phytium_nand->node, &nfc->chips);
	return 0;
}

int phytium_nand_init(struct phytium_nfc *nfc)
{
	int ret;

	nand_controller_init(&nfc->controller);
	nfc->controller.ops = &phytium_nand_controller_ops;
	INIT_LIST_HEAD(&nfc->chips);

	/* Init the controller and then probe the chips */
	ret = phytium_nfc_init(nfc);
	if (ret)
		goto out;

	ret = phytium_nand_chip_init(nfc);
	if (ret)
		goto out;

	spin_lock_init(&nfc->spinlock);

out:
	return ret;
}
EXPORT_SYMBOL_GPL(phytium_nand_init);

int phytium_nand_remove(struct phytium_nfc *nfc)
{
	phytium_nand_chips_cleanup(nfc);

	return 0;
}
EXPORT_SYMBOL_GPL(phytium_nand_remove);

static int phytium_nfc_wait_ndrun(struct nand_chip *chip)
{
	struct phytium_nfc *nfc = to_phytium_nfc(chip->controller);
	int ret = 0;
	u32 val;

	ret = readl_relaxed_poll_timeout(nfc->regs + NDSR, val,
					 (val & NDSR_RB) == 0,
					 0, 100 * 1000);
	if (ret) {
		dev_err(nfc->dev, "Timeout on NAND controller run mode\n");
		ret = -EAGAIN;
	}

	return ret;
}

int phytium_nand_prepare(struct phytium_nfc *nfc)
{
	struct phytium_nand_chip *chip = NULL;

	list_for_each_entry(chip, &nfc->chips, node)
		phytium_nfc_wait_ndrun(&chip->chip);

	return 0;
}
EXPORT_SYMBOL_GPL(phytium_nand_prepare);

int phytium_nand_resume(struct phytium_nfc *nfc)
{
	nfc->selected_chip = NULL;
	phytium_nfc_init(nfc);
	phytium_nfc_default_data_interface(nfc);

	return 0;
}
EXPORT_SYMBOL_GPL(phytium_nand_resume);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Phytium NAND controller platform driver");
MODULE_AUTHOR("Zhu Mingshuai <zhumingshuai@phytium.com.cn>");
