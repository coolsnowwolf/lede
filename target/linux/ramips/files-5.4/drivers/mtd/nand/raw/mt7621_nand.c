// SPDX-License-Identifier: GPL-2.0
/*
 * MediaTek MT7621 NAND Flash Controller driver
 *
 * Copyright (C) 2020 MediaTek Inc. All Rights Reserved.
 *
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 */

#include <linux/io.h>
#include <linux/clk.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/sizes.h>
#include <linux/iopoll.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/rawnand.h>
#include <linux/mtd/partitions.h>
#include <linux/mtd/mtk_bmt.h>
#include <linux/platform_device.h>
#include <asm/addrspace.h>

/* NFI core registers */
#define NFI_CNFG			0x000
#define   CNFG_OP_MODE_S		12
#define   CNFG_OP_MODE_M		GENMASK(14, 12)
#define     CNFG_OP_CUSTOM		6
#define   CNFG_AUTO_FMT_EN		BIT(9)
#define   CNFG_HW_ECC_EN		BIT(8)
#define   CNFG_BYTE_RW			BIT(6)
#define   CNFG_READ_MODE		BIT(1)

#define NFI_PAGEFMT			0x004
#define   PAGEFMT_FDM_ECC_S		12
#define   PAGEFMT_FDM_ECC_M		GENMASK(15, 12)
#define   PAGEFMT_FDM_S			8
#define   PAGEFMT_FDM_M			GENMASK(11, 8)
#define   PAGEFMT_SPARE_S		4
#define   PAGEFMT_SPARE_M		GENMASK(5, 4)
#define   PAGEFMT_PAGE_S		0
#define   PAGEFMT_PAGE_M		GENMASK(1, 0)

#define NFI_CON				0x008
#define   CON_NFI_SEC_S			12
#define   CON_NFI_SEC_M			GENMASK(15, 12)
#define   CON_NFI_BWR			BIT(9)
#define   CON_NFI_BRD			BIT(8)
#define   CON_NFI_RST			BIT(1)
#define   CON_FIFO_FLUSH		BIT(0)

#define NFI_ACCCON			0x00c
#define   ACCCON_POECS_S		28
#define   ACCCON_POECS_MAX		0x0f
#define   ACCCON_POECS_DEF		3
#define   ACCCON_PRECS_S		22
#define   ACCCON_PRECS_MAX		0x3f
#define   ACCCON_PRECS_DEF		3
#define   ACCCON_C2R_S			16
#define   ACCCON_C2R_MAX		0x3f
#define   ACCCON_C2R_DEF		7
#define   ACCCON_W2R_S			12
#define   ACCCON_W2R_MAX		0x0f
#define   ACCCON_W2R_DEF		7
#define   ACCCON_WH_S			8
#define   ACCCON_WH_MAX			0x0f
#define   ACCCON_WH_DEF			15
#define   ACCCON_WST_S			4
#define   ACCCON_WST_MAX		0x0f
#define   ACCCON_WST_DEF		15
#define   ACCCON_WST_MIN		3
#define   ACCCON_RLT_S			0
#define   ACCCON_RLT_MAX		0x0f
#define   ACCCON_RLT_DEF		15
#define   ACCCON_RLT_MIN		3

#define NFI_CMD				0x020

#define NFI_ADDRNOB			0x030
#define   ADDR_ROW_NOB_S		4
#define   ADDR_ROW_NOB_M		GENMASK(6, 4)
#define   ADDR_COL_NOB_S		0
#define   ADDR_COL_NOB_M		GENMASK(2, 0)

#define NFI_COLADDR			0x034
#define NFI_ROWADDR			0x038

#define NFI_STRDATA			0x040
#define   STR_DATA			BIT(0)

#define NFI_CNRNB			0x044
#define   CB2R_TIME_S			4
#define   CB2R_TIME_M			GENMASK(7, 4)
#define   STR_CNRNB			BIT(0)

#define NFI_DATAW			0x050
#define NFI_DATAR			0x054

#define NFI_PIO_DIRDY			0x058
#define   PIO_DIRDY			BIT(0)

#define NFI_STA				0x060
#define   STA_NFI_FSM_S			16
#define   STA_NFI_FSM_M			GENMASK(19, 16)
#define     STA_FSM_CUSTOM_DATA		14
#define   STA_BUSY			BIT(8)
#define   STA_ADDR			BIT(1)
#define   STA_CMD			BIT(0)

#define NFI_ADDRCNTR			0x070
#define   SEC_CNTR_S			12
#define   SEC_CNTR_M			GENMASK(15, 12)
#define   SEC_ADDR_S			0
#define   SEC_ADDR_M			GENMASK(9, 0)

#define NFI_CSEL			0x090
#define   CSEL_S			0
#define   CSEL_M			GENMASK(1, 0)

#define NFI_FDM0L			0x0a0
#define NFI_FDML(n)			(0x0a0 + ((n) << 3))

#define NFI_FDM0M			0x0a4
#define NFI_FDMM(n)			(0x0a4 + ((n) << 3))

#define NFI_MASTER_STA			0x210
#define   MAS_ADDR			GENMASK(11, 9)
#define   MAS_RD			GENMASK(8, 6)
#define   MAS_WR			GENMASK(5, 3)
#define   MAS_RDDLY			GENMASK(2, 0)

/* ECC engine registers */
#define ECC_ENCCON			0x000
#define   ENC_EN			BIT(0)

#define ECC_ENCCNFG			0x004
#define   ENC_CNFG_MSG_S		16
#define   ENC_CNFG_MSG_M		GENMASK(28, 16)
#define   ENC_MODE_S			4
#define   ENC_MODE_M			GENMASK(5, 4)
#define     ENC_MODE_NFI		1
#define   ENC_TNUM_S			0
#define   ENC_TNUM_M			GENMASK(2, 0)

#define ECC_ENCIDLE			0x00c
#define   ENC_IDLE			BIT(0)

#define ECC_DECCON			0x100
#define   DEC_EN			BIT(0)

#define ECC_DECCNFG			0x104
#define   DEC_EMPTY_EN			BIT(31)
#define   DEC_CS_S			16
#define   DEC_CS_M			GENMASK(28, 16)
#define   DEC_CON_S			12
#define   DEC_CON_M			GENMASK(13, 12)
#define     DEC_CON_EL			2
#define   DEC_MODE_S			4
#define   DEC_MODE_M			GENMASK(5, 4)
#define     DEC_MODE_NFI		1
#define   DEC_TNUM_S			0
#define   DEC_TNUM_M			GENMASK(2, 0)

#define ECC_DECIDLE			0x10c
#define   DEC_IDLE			BIT(1)

#define ECC_DECENUM			0x114
#define   ERRNUM_S			2
#define   ERRNUM_M			GENMASK(3, 0)

#define ECC_DECDONE			0x118
#define   DEC_DONE7			BIT(7)
#define   DEC_DONE6			BIT(6)
#define   DEC_DONE5			BIT(5)
#define   DEC_DONE4			BIT(4)
#define   DEC_DONE3			BIT(3)
#define   DEC_DONE2			BIT(2)
#define   DEC_DONE1			BIT(1)
#define   DEC_DONE0			BIT(0)

#define ECC_DECEL(n)			(0x11c + (n) * 4)
#define   DEC_EL_ODD_S			16
#define   DEC_EL_EVEN_S			0
#define   DEC_EL_M			0x1fff
#define   DEC_EL_BYTE_POS_S		3
#define   DEC_EL_BIT_POS_M		GENMASK(2, 0)

#define ECC_FDMADDR			0x13c

/* ENCIDLE and DECIDLE */
#define   ECC_IDLE			BIT(0)

#define ACCTIMING(tpoecs, tprecs, tc2r, tw2r, twh, twst, trlt) \
	((tpoecs) << ACCCON_POECS_S | (tprecs) << ACCCON_PRECS_S | \
	(tc2r) << ACCCON_C2R_S | (tw2r) << ACCCON_W2R_S | \
	(twh) << ACCCON_WH_S | (twst) << ACCCON_WST_S | (trlt))

#define MASTER_STA_MASK			(MAS_ADDR | MAS_RD | MAS_WR | \
					 MAS_RDDLY)
#define NFI_RESET_TIMEOUT		1000000
#define NFI_CORE_TIMEOUT		500000
#define ECC_ENGINE_TIMEOUT		500000

#define ECC_SECTOR_SIZE			512
#define ECC_PARITY_BITS			13

#define NFI_FDM_SIZE		8

#define MT7621_NFC_NAME			"mt7621-nand"

struct mt7621_nfc {
	struct nand_controller controller;
	struct nand_chip nand;
	struct clk *nfi_clk;
	struct device *dev;

	u32 nfi_base;
	void __iomem *nfi_regs;
	void __iomem *ecc_regs;

	u32 spare_per_sector;
};

static const u16 mt7621_nfi_page_size[] = { SZ_512, SZ_2K, SZ_4K };
static const u8 mt7621_nfi_spare_size[] = { 16, 26, 27, 28 };
static const u8 mt7621_ecc_strength[] = { 4, 6, 8, 10, 12 };

static inline u32 nfi_read32(struct mt7621_nfc *nfc, u32 reg)
{
	return readl(nfc->nfi_regs + reg);
}

static inline void nfi_write32(struct mt7621_nfc *nfc, u32 reg, u32 val)
{
	writel(val, nfc->nfi_regs + reg);
}

static inline u16 nfi_read16(struct mt7621_nfc *nfc, u32 reg)
{
	return readw(nfc->nfi_regs + reg);
}

static inline void nfi_write16(struct mt7621_nfc *nfc, u32 reg, u16 val)
{
	writew(val, nfc->nfi_regs + reg);
}

static inline void ecc_write16(struct mt7621_nfc *nfc, u32 reg, u16 val)
{
	writew(val, nfc->ecc_regs + reg);
}

static inline u32 ecc_read32(struct mt7621_nfc *nfc, u32 reg)
{
	return readl(nfc->ecc_regs + reg);
}

static inline void ecc_write32(struct mt7621_nfc *nfc, u32 reg, u32 val)
{
	return writel(val, nfc->ecc_regs + reg);
}

static inline u8 *oob_fdm_ptr(struct nand_chip *nand, int sect)
{
	return nand->oob_poi + sect * NFI_FDM_SIZE;
}

static inline u8 *oob_ecc_ptr(struct mt7621_nfc *nfc, int sect)
{
	struct nand_chip *nand = &nfc->nand;

	return nand->oob_poi + nand->ecc.steps * NFI_FDM_SIZE +
		sect * (nfc->spare_per_sector - NFI_FDM_SIZE);
}

static inline u8 *page_data_ptr(struct nand_chip *nand, const u8 *buf,
				int sect)
{
	return (u8 *)buf + sect * nand->ecc.size;
}

static int mt7621_ecc_wait_idle(struct mt7621_nfc *nfc, u32 reg)
{
	struct device *dev = nfc->dev;
	u32 val;
	int ret;

	ret = readw_poll_timeout_atomic(nfc->ecc_regs + reg, val,
					val & ECC_IDLE, 10,
					ECC_ENGINE_TIMEOUT);
	if (ret) {
		dev_warn(dev, "ECC engine timed out entering idle mode\n");
		return -EIO;
	}

	return 0;
}

static int mt7621_ecc_decoder_wait_done(struct mt7621_nfc *nfc, u32 sect)
{
	struct device *dev = nfc->dev;
	u32 val;
	int ret;

	ret = readw_poll_timeout_atomic(nfc->ecc_regs + ECC_DECDONE, val,
					val & (1 << sect), 10,
					ECC_ENGINE_TIMEOUT);

	if (ret) {
		dev_warn(dev, "ECC decoder for sector %d timed out\n",
			 sect);
		return -ETIMEDOUT;
	}

	return 0;
}

static void mt7621_ecc_encoder_op(struct mt7621_nfc *nfc, bool enable)
{
	mt7621_ecc_wait_idle(nfc, ECC_ENCIDLE);
	ecc_write16(nfc, ECC_ENCCON, enable ? ENC_EN : 0);
}

static void mt7621_ecc_decoder_op(struct mt7621_nfc *nfc, bool enable)
{
	mt7621_ecc_wait_idle(nfc, ECC_DECIDLE);
	ecc_write16(nfc, ECC_DECCON, enable ? DEC_EN : 0);
}

static int mt7621_ecc_correct_check(struct mt7621_nfc *nfc, u8 *sector_buf,
				   u8 *fdm_buf, u32 sect)
{
	struct nand_chip *nand = &nfc->nand;
	u32 decnum, num_error_bits, fdm_end_bits;
	u32 error_locations, error_bit_loc;
	u32 error_byte_pos, error_bit_pos;
	int bitflips = 0;
	u32 i;

	decnum = ecc_read32(nfc, ECC_DECENUM);
	num_error_bits = (decnum >> (sect << ERRNUM_S)) & ERRNUM_M;
	fdm_end_bits = (nand->ecc.size + NFI_FDM_SIZE) << 3;

	if (!num_error_bits)
		return 0;

	if (num_error_bits == ERRNUM_M)
		return -1;

	for (i = 0; i < num_error_bits; i++) {
		error_locations = ecc_read32(nfc, ECC_DECEL(i / 2));
		error_bit_loc = (error_locations >> ((i % 2) * DEC_EL_ODD_S)) &
				DEC_EL_M;
		error_byte_pos = error_bit_loc >> DEC_EL_BYTE_POS_S;
		error_bit_pos = error_bit_loc & DEC_EL_BIT_POS_M;

		if (error_bit_loc < (nand->ecc.size << 3)) {
			if (sector_buf) {
				sector_buf[error_byte_pos] ^=
					(1 << error_bit_pos);
			}
		} else if (error_bit_loc < fdm_end_bits) {
			if (fdm_buf) {
				fdm_buf[error_byte_pos - nand->ecc.size] ^=
					(1 << error_bit_pos);
			}
		}

		bitflips++;
	}

	return bitflips;
}

static int mt7621_nfc_wait_write_completion(struct mt7621_nfc *nfc,
					    struct nand_chip *nand)
{
	struct device *dev = nfc->dev;
	u16 val;
	int ret;

	ret = readw_poll_timeout_atomic(nfc->nfi_regs + NFI_ADDRCNTR, val,
		((val & SEC_CNTR_M) >> SEC_CNTR_S) >= nand->ecc.steps, 10,
		NFI_CORE_TIMEOUT);

	if (ret) {
		dev_warn(dev, "NFI core write operation timed out\n");
		return -ETIMEDOUT;
	}

	return ret;
}

static void mt7621_nfc_hw_reset(struct mt7621_nfc *nfc)
{
	u32 val;
	int ret;

	/* reset all registers and force the NFI master to terminate */
	nfi_write16(nfc, NFI_CON, CON_FIFO_FLUSH | CON_NFI_RST);

	/* wait for the master to finish the last transaction */
	ret = readw_poll_timeout(nfc->nfi_regs + NFI_MASTER_STA, val,
				 !(val & MASTER_STA_MASK), 50,
				 NFI_RESET_TIMEOUT);
	if (ret) {
		dev_warn(nfc->dev, "Failed to reset NFI master in %dms\n",
			 NFI_RESET_TIMEOUT);
	}

	/* ensure any status register affected by the NFI master is reset */
	nfi_write16(nfc, NFI_CON, CON_FIFO_FLUSH | CON_NFI_RST);
	nfi_write16(nfc, NFI_STRDATA, 0);
}

static inline void mt7621_nfc_hw_init(struct mt7621_nfc *nfc)
{
	u32 acccon;

	/*
	 * CNRNB: nand ready/busy register
	 * -------------------------------
	 * 7:4: timeout register for polling the NAND busy/ready signal
	 * 0  : poll the status of the busy/ready signal after [7:4]*16 cycles.
	 */
	nfi_write16(nfc, NFI_CNRNB, CB2R_TIME_M | STR_CNRNB);

	mt7621_nfc_hw_reset(nfc);

	/* Apply default access timing */
	acccon = ACCTIMING(ACCCON_POECS_DEF, ACCCON_PRECS_DEF, ACCCON_C2R_DEF,
			   ACCCON_W2R_DEF, ACCCON_WH_DEF, ACCCON_WST_DEF,
			   ACCCON_RLT_DEF);

	nfi_write32(nfc, NFI_ACCCON, acccon);
}

static int mt7621_nfc_send_command(struct mt7621_nfc *nfc, u8 command)
{
	struct device *dev = nfc->dev;
	u32 val;
	int ret;

	nfi_write32(nfc, NFI_CMD, command);

	ret = readl_poll_timeout_atomic(nfc->nfi_regs + NFI_STA, val,
					!(val & STA_CMD), 10,
					NFI_CORE_TIMEOUT);
	if (ret) {
		dev_warn(dev, "NFI core timed out entering command mode\n");
		return -EIO;
	}

	return 0;
}

static int mt7621_nfc_send_address_byte(struct mt7621_nfc *nfc, int addr)
{
	struct device *dev = nfc->dev;
	u32 val;
	int ret;

	nfi_write32(nfc, NFI_COLADDR, addr);
	nfi_write32(nfc, NFI_ROWADDR, 0);
	nfi_write16(nfc, NFI_ADDRNOB, 1);

	ret = readl_poll_timeout_atomic(nfc->nfi_regs + NFI_STA, val,
					!(val & STA_ADDR), 10,
					NFI_CORE_TIMEOUT);
	if (ret) {
		dev_warn(dev, "NFI core timed out entering address mode\n");
		return -EIO;
	}

	return 0;
}

static int mt7621_nfc_send_address(struct mt7621_nfc *nfc, const u8 *addr,
				   unsigned int naddrs)
{
	int ret;

	while (naddrs) {
		ret = mt7621_nfc_send_address_byte(nfc, *addr);
		if (ret)
			return ret;

		addr++;
		naddrs--;
	}

	return 0;
}

static void mt7621_nfc_wait_pio_ready(struct mt7621_nfc *nfc)
{
	struct device *dev = nfc->dev;
	int ret;
	u16 val;

	ret = readw_poll_timeout_atomic(nfc->nfi_regs + NFI_PIO_DIRDY, val,
					val & PIO_DIRDY, 10,
					NFI_CORE_TIMEOUT);
	if (ret < 0)
		dev_err(dev, "NFI core PIO mode not ready\n");
}

static u32 mt7621_nfc_pio_read(struct mt7621_nfc *nfc, bool br)
{
	u32 reg;

	/* after each byte read, the NFI_STA reg is reset by the hardware */
	reg = (nfi_read32(nfc, NFI_STA) & STA_NFI_FSM_M) >> STA_NFI_FSM_S;
	if (reg != STA_FSM_CUSTOM_DATA) {
		reg = nfi_read16(nfc, NFI_CNFG);
		reg |= CNFG_READ_MODE | CNFG_BYTE_RW;
		if (!br)
			reg &= ~CNFG_BYTE_RW;
		nfi_write16(nfc, NFI_CNFG, reg);

		/*
		 * set to max sector to allow the HW to continue reading over
		 * unaligned accesses
		 */
		nfi_write16(nfc, NFI_CON, CON_NFI_SEC_M | CON_NFI_BRD);

		/* trigger to fetch data */
		nfi_write16(nfc, NFI_STRDATA, STR_DATA);
	}

	mt7621_nfc_wait_pio_ready(nfc);

	return nfi_read32(nfc, NFI_DATAR);
}

static void mt7621_nfc_read_data(struct mt7621_nfc *nfc, u8 *buf, u32 len)
{
	while (((uintptr_t)buf & 3) && len) {
		*buf = mt7621_nfc_pio_read(nfc, true);
		buf++;
		len--;
	}

	while (len >= 4) {
		*(u32 *)buf = mt7621_nfc_pio_read(nfc, false);
		buf += 4;
		len -= 4;
	}

	while (len) {
		*buf = mt7621_nfc_pio_read(nfc, true);
		buf++;
		len--;
	}
}

static void mt7621_nfc_read_data_discard(struct mt7621_nfc *nfc, u32 len)
{
	while (len >= 4) {
		mt7621_nfc_pio_read(nfc, false);
		len -= 4;
	}

	while (len) {
		mt7621_nfc_pio_read(nfc, true);
		len--;
	}
}

static void mt7621_nfc_pio_write(struct mt7621_nfc *nfc, u32 val, bool bw)
{
	u32 reg;

	reg = (nfi_read32(nfc, NFI_STA) & STA_NFI_FSM_M) >> STA_NFI_FSM_S;
	if (reg != STA_FSM_CUSTOM_DATA) {
		reg = nfi_read16(nfc, NFI_CNFG);
		reg &= ~(CNFG_READ_MODE | CNFG_BYTE_RW);
		if (bw)
			reg |= CNFG_BYTE_RW;
		nfi_write16(nfc, NFI_CNFG, reg);

		nfi_write16(nfc, NFI_CON, CON_NFI_SEC_M | CON_NFI_BWR);
		nfi_write16(nfc, NFI_STRDATA, STR_DATA);
	}

	mt7621_nfc_wait_pio_ready(nfc);
	nfi_write32(nfc, NFI_DATAW, val);
}

static void mt7621_nfc_write_data(struct mt7621_nfc *nfc, const u8 *buf,
				  u32 len)
{
	while (((uintptr_t)buf & 3) && len) {
		mt7621_nfc_pio_write(nfc, *buf, true);
		buf++;
		len--;
	}

	while (len >= 4) {
		mt7621_nfc_pio_write(nfc, *(const u32 *)buf, false);
		buf += 4;
		len -= 4;
	}

	while (len) {
		mt7621_nfc_pio_write(nfc, *buf, true);
		buf++;
		len--;
	}
}

static void mt7621_nfc_write_data_empty(struct mt7621_nfc *nfc, u32 len)
{
	while (len >= 4) {
		mt7621_nfc_pio_write(nfc, 0xffffffff, false);
		len -= 4;
	}

	while (len) {
		mt7621_nfc_pio_write(nfc, 0xff, true);
		len--;
	}
}

static int mt7621_nfc_dev_ready(struct mt7621_nfc *nfc,
				unsigned int timeout_ms)
{
	u32 val;

	return readl_poll_timeout_atomic(nfc->nfi_regs + NFI_STA, val,
					 !(val & STA_BUSY), 10,
					 timeout_ms * 1000);
}

static int mt7621_nfc_exec_instr(struct nand_chip *nand,
				 const struct nand_op_instr *instr)
{
	struct mt7621_nfc *nfc = nand_get_controller_data(nand);

	switch (instr->type) {
	case NAND_OP_CMD_INSTR:
		mt7621_nfc_hw_reset(nfc);
		nfi_write16(nfc, NFI_CNFG, CNFG_OP_CUSTOM << CNFG_OP_MODE_S);
		return mt7621_nfc_send_command(nfc, instr->ctx.cmd.opcode);
	case NAND_OP_ADDR_INSTR:
		return mt7621_nfc_send_address(nfc, instr->ctx.addr.addrs,
					       instr->ctx.addr.naddrs);
	case NAND_OP_DATA_IN_INSTR:
		mt7621_nfc_read_data(nfc, instr->ctx.data.buf.in,
				     instr->ctx.data.len);
		return 0;
	case NAND_OP_DATA_OUT_INSTR:
		mt7621_nfc_write_data(nfc, instr->ctx.data.buf.out,
				      instr->ctx.data.len);
		return 0;
	case NAND_OP_WAITRDY_INSTR:
		return mt7621_nfc_dev_ready(nfc,
					    instr->ctx.waitrdy.timeout_ms);
	default:
		WARN_ONCE(1, "unsupported NAND instruction type: %d\n",
			  instr->type);

		return -EINVAL;
	}
}

static int mt7621_nfc_exec_op(struct nand_chip *nand,
			      const struct nand_operation *op, bool check_only)
{
	struct mt7621_nfc *nfc = nand_get_controller_data(nand);
	int i, ret;

	if (check_only)
		return 0;

	/* Only CS0 available */
	nfi_write16(nfc, NFI_CSEL, 0);

	for (i = 0; i < op->ninstrs; i++) {
		ret = mt7621_nfc_exec_instr(nand, &op->instrs[i]);
		if (ret)
			return ret;
	}

	return 0;
}

static int mt7621_nfc_setup_interface(struct nand_chip *nand, int csline,
				      const struct nand_interface_config *conf)
{
	struct mt7621_nfc *nfc = nand_get_controller_data(nand);
	const struct nand_sdr_timings *timings;
	u32 acccon, temp, rate, tpoecs, tprecs, tc2r, tw2r, twh, twst, trlt;

	if (!nfc->nfi_clk)
		return -ENOTSUPP;

	timings = nand_get_sdr_timings(conf);
	if (IS_ERR(timings))
		return -ENOTSUPP;

	rate = clk_get_rate(nfc->nfi_clk);

	/* turn clock rate into KHZ */
	rate /= 1000;

	tpoecs = max(timings->tALH_min, timings->tCLH_min) / 1000;
	tpoecs = DIV_ROUND_UP(tpoecs * rate, 1000000);
	tpoecs = min_t(u32, tpoecs, ACCCON_POECS_MAX);

	tprecs = max(timings->tCLS_min, timings->tALS_min) / 1000;
	tprecs = DIV_ROUND_UP(tprecs * rate, 1000000);
	tprecs = min_t(u32, tprecs, ACCCON_PRECS_MAX);

	/* sdr interface has no tCR which means CE# low to RE# low */
	tc2r = 0;

	tw2r = timings->tWHR_min / 1000;
	tw2r = DIV_ROUND_UP(tw2r * rate, 1000000);
	tw2r = DIV_ROUND_UP(tw2r - 1, 2);
	tw2r = min_t(u32, tw2r, ACCCON_W2R_MAX);

	twh = max(timings->tREH_min, timings->tWH_min) / 1000;
	twh = DIV_ROUND_UP(twh * rate, 1000000) - 1;
	twh = min_t(u32, twh, ACCCON_WH_MAX);

	/* Calculate real WE#/RE# hold time in nanosecond */
	temp = (twh + 1) * 1000000 / rate;
	/* nanosecond to picosecond */
	temp *= 1000;

	/*
	 * WE# low level time should be expaned to meet WE# pulse time
	 * and WE# cycle time at the same time.
	 */
	if (temp < timings->tWC_min)
		twst = timings->tWC_min - temp;
	else
		twst = 0;
	twst = max(timings->tWP_min, twst) / 1000;
	twst = DIV_ROUND_UP(twst * rate, 1000000) - 1;
	twst = min_t(u32, twst, ACCCON_WST_MAX);

	/*
	 * RE# low level time should be expaned to meet RE# pulse time
	 * and RE# cycle time at the same time.
	 */
	if (temp < timings->tRC_min)
		trlt = timings->tRC_min - temp;
	else
		trlt = 0;
	trlt = max(trlt, timings->tRP_min) / 1000;
	trlt = DIV_ROUND_UP(trlt * rate, 1000000) - 1;
	trlt = min_t(u32, trlt, ACCCON_RLT_MAX);

	if (csline == NAND_DATA_IFACE_CHECK_ONLY) {
		if (twst < ACCCON_WST_MIN || trlt < ACCCON_RLT_MIN)
			return -ENOTSUPP;
	}

	acccon = ACCTIMING(tpoecs, tprecs, tc2r, tw2r, twh, twst, trlt);

	dev_dbg(nfc->dev, "Using programmed access timing: %08x\n", acccon);

	nfi_write32(nfc, NFI_ACCCON, acccon);

	return 0;
}

static int mt7621_nfc_calc_ecc_strength(struct mt7621_nfc *nfc,
					u32 avail_ecc_bytes)
{
	struct nand_chip *nand = &nfc->nand;
	struct mtd_info *mtd = nand_to_mtd(nand);
	u32 strength;
	int i;

	strength = avail_ecc_bytes * 8 / ECC_PARITY_BITS;

	/* Find the closest supported ecc strength */
	for (i = ARRAY_SIZE(mt7621_ecc_strength) - 1; i >= 0; i--) {
		if (mt7621_ecc_strength[i] <= strength)
			break;
	}

	if (unlikely(i < 0)) {
		dev_err(nfc->dev, "OOB size (%u) is not supported\n",
			mtd->oobsize);
		return -EINVAL;
	}

	nand->ecc.strength = mt7621_ecc_strength[i];
	nand->ecc.bytes =
		DIV_ROUND_UP(nand->ecc.strength * ECC_PARITY_BITS, 8);

	dev_info(nfc->dev, "ECC strength adjusted to %u bits\n",
		 nand->ecc.strength);

	return i;
}

static int mt7621_nfc_set_spare_per_sector(struct mt7621_nfc *nfc)
{
	struct nand_chip *nand = &nfc->nand;
	struct mtd_info *mtd = nand_to_mtd(nand);
	u32 size;
	int i;

	size = nand->ecc.bytes + NFI_FDM_SIZE;

	/* Find the closest supported spare size */
	for (i = 0; i < ARRAY_SIZE(mt7621_nfi_spare_size); i++) {
		if (mt7621_nfi_spare_size[i] >= size)
			break;
	}

	if (unlikely(i >= ARRAY_SIZE(mt7621_nfi_spare_size))) {
		dev_err(nfc->dev, "OOB size (%u) is not supported\n",
			mtd->oobsize);
		return -EINVAL;
	}

	nfc->spare_per_sector = mt7621_nfi_spare_size[i];

	return i;
}

static int mt7621_nfc_ecc_init(struct mt7621_nfc *nfc)
{
	struct nand_chip *nand = &nfc->nand;
	struct mtd_info *mtd = nand_to_mtd(nand);
	u32 spare_per_sector, encode_block_size, decode_block_size;
	u32 ecc_enccfg, ecc_deccfg;
	int ecc_cap;

	/* Only hardware ECC mode is supported */
	if (nand->ecc.engine_type != NAND_ECC_ENGINE_TYPE_ON_HOST) {
		dev_err(nfc->dev, "Only hardware ECC mode is supported\n");
		return -EINVAL;
	}

	nand->ecc.size = ECC_SECTOR_SIZE;
	nand->ecc.steps = mtd->writesize / nand->ecc.size;

	spare_per_sector = mtd->oobsize / nand->ecc.steps;

	ecc_cap = mt7621_nfc_calc_ecc_strength(nfc,
		spare_per_sector - NFI_FDM_SIZE);
	if (ecc_cap < 0)
		return ecc_cap;

	/* Sector + FDM */
	encode_block_size = (nand->ecc.size + NFI_FDM_SIZE) * 8;
	ecc_enccfg = ecc_cap | (ENC_MODE_NFI << ENC_MODE_S) |
		     (encode_block_size << ENC_CNFG_MSG_S);

	/* Sector + FDM + ECC parity bits */
	decode_block_size = ((nand->ecc.size + NFI_FDM_SIZE) * 8) +
			    nand->ecc.strength * ECC_PARITY_BITS;
	ecc_deccfg = ecc_cap | (DEC_MODE_NFI << DEC_MODE_S) |
		     (decode_block_size << DEC_CS_S) |
		     (DEC_CON_EL << DEC_CON_S) | DEC_EMPTY_EN;

	ecc_write32(nfc, ECC_FDMADDR, nfc->nfi_base + NFI_FDML(0));

	mt7621_ecc_encoder_op(nfc, false);
	ecc_write32(nfc, ECC_ENCCNFG, ecc_enccfg);

	mt7621_ecc_decoder_op(nfc, false);
	ecc_write32(nfc, ECC_DECCNFG, ecc_deccfg);

	return 0;
}

static int mt7621_nfc_set_page_format(struct mt7621_nfc *nfc)
{
	struct nand_chip *nand = &nfc->nand;
	struct mtd_info *mtd = nand_to_mtd(nand);
	int i, spare_size;
	u32 pagefmt;

	spare_size = mt7621_nfc_set_spare_per_sector(nfc);
	if (spare_size < 0)
		return spare_size;

	for (i = 0; i < ARRAY_SIZE(mt7621_nfi_page_size); i++) {
		if (mt7621_nfi_page_size[i] == mtd->writesize)
			break;
	}

	if (unlikely(i >= ARRAY_SIZE(mt7621_nfi_page_size))) {
		dev_err(nfc->dev, "Page size (%u) is not supported\n",
			mtd->writesize);
		return -EINVAL;
	}

	pagefmt = i | (spare_size << PAGEFMT_SPARE_S) |
		  (NFI_FDM_SIZE << PAGEFMT_FDM_S) |
		  (NFI_FDM_SIZE << PAGEFMT_FDM_ECC_S);

	nfi_write16(nfc, NFI_PAGEFMT, pagefmt);

	return 0;
}

static int mt7621_nfc_attach_chip(struct nand_chip *nand)
{
	struct mt7621_nfc *nfc = nand_get_controller_data(nand);
	int ret;

	if (nand->options & NAND_BUSWIDTH_16) {
		dev_err(nfc->dev, "16-bit buswidth is not supported");
		return -EINVAL;
	}

	ret = mt7621_nfc_ecc_init(nfc);
	if (ret)
		return ret;

	return mt7621_nfc_set_page_format(nfc);
}

static const struct nand_controller_ops mt7621_nfc_controller_ops = {
	.attach_chip = mt7621_nfc_attach_chip,
	.exec_op = mt7621_nfc_exec_op,
	.setup_interface = mt7621_nfc_setup_interface,
};

static int mt7621_nfc_ooblayout_free(struct mtd_info *mtd, int section,
				     struct mtd_oob_region *oob_region)
{
	struct nand_chip *nand = mtd_to_nand(mtd);

	if (section >= nand->ecc.steps)
		return -ERANGE;

	oob_region->length = NFI_FDM_SIZE - 1;
	oob_region->offset = section * NFI_FDM_SIZE + 1;

	return 0;
}

static int mt7621_nfc_ooblayout_ecc(struct mtd_info *mtd, int section,
				    struct mtd_oob_region *oob_region)
{
	struct nand_chip *nand = mtd_to_nand(mtd);

	if (section)
		return -ERANGE;

	oob_region->offset = NFI_FDM_SIZE * nand->ecc.steps;
	oob_region->length = mtd->oobsize - oob_region->offset;

	return 0;
}

static const struct mtd_ooblayout_ops mt7621_nfc_ooblayout_ops = {
	.free = mt7621_nfc_ooblayout_free,
	.ecc = mt7621_nfc_ooblayout_ecc,
};

static void mt7621_nfc_write_fdm(struct mt7621_nfc *nfc)
{
	struct nand_chip *nand = &nfc->nand;
	u32 vall, valm;
	u8 *oobptr;
	int i, j;

	for (i = 0; i < nand->ecc.steps; i++) {
		vall = 0;
		valm = 0;
		oobptr = oob_fdm_ptr(nand, i);

		for (j = 0; j < 4; j++)
			vall |= (u32)oobptr[j] << (j * 8);

		for (j = 0; j < 4; j++)
			valm |= (u32)oobptr[j + 4] << (j * 8);

		nfi_write32(nfc, NFI_FDML(i), vall);
		nfi_write32(nfc, NFI_FDMM(i), valm);
	}
}

static void mt7621_nfc_read_sector_fdm(struct mt7621_nfc *nfc, u32 sect)
{
	struct nand_chip *nand = &nfc->nand;
	u32 vall, valm;
	u8 *oobptr;
	int i;

	vall = nfi_read32(nfc, NFI_FDML(sect));
	valm = nfi_read32(nfc, NFI_FDMM(sect));
	oobptr = oob_fdm_ptr(nand, sect);

	for (i = 0; i < 4; i++)
		oobptr[i] = (vall >> (i * 8)) & 0xff;

	for (i = 0; i < 4; i++)
		oobptr[i + 4] = (valm >> (i * 8)) & 0xff;
}

static int mt7621_nfc_read_page_hwecc(struct nand_chip *nand, uint8_t *buf,
				      int oob_required, int page)
{
	struct mt7621_nfc *nfc = nand_get_controller_data(nand);
	struct mtd_info *mtd = nand_to_mtd(nand);
	int bitflips = 0;
	int rc, i;

	nand_read_page_op(nand, page, 0, NULL, 0);

	nfi_write16(nfc, NFI_CNFG, (CNFG_OP_CUSTOM << CNFG_OP_MODE_S) |
		    CNFG_READ_MODE | CNFG_AUTO_FMT_EN | CNFG_HW_ECC_EN);

	mt7621_ecc_decoder_op(nfc, true);

	nfi_write16(nfc, NFI_CON,
		    CON_NFI_BRD | (nand->ecc.steps << CON_NFI_SEC_S));

	for (i = 0; i < nand->ecc.steps; i++) {
		if (buf)
			mt7621_nfc_read_data(nfc, page_data_ptr(nand, buf, i),
					     nand->ecc.size);
		else
			mt7621_nfc_read_data_discard(nfc, nand->ecc.size);

		rc = mt7621_ecc_decoder_wait_done(nfc, i);

		mt7621_nfc_read_sector_fdm(nfc, i);

		if (rc < 0) {
			bitflips = -EIO;
			continue;
		}

		rc = mt7621_ecc_correct_check(nfc,
			buf ? page_data_ptr(nand, buf, i) : NULL,
			oob_fdm_ptr(nand, i), i);

		if (rc < 0) {
			dev_dbg(nfc->dev,
				 "Uncorrectable ECC error at page %d.%d\n",
				 page, i);
			bitflips = -EBADMSG;
			mtd->ecc_stats.failed++;
		} else if (bitflips >= 0) {
			bitflips += rc;
			mtd->ecc_stats.corrected += rc;
		}
	}

	mt7621_ecc_decoder_op(nfc, false);

	nfi_write16(nfc, NFI_CON, 0);

	return bitflips;
}

static int mt7621_nfc_read_page_raw(struct nand_chip *nand, uint8_t *buf,
				    int oob_required, int page)
{
	struct mt7621_nfc *nfc = nand_get_controller_data(nand);
	int i;

	nand_read_page_op(nand, page, 0, NULL, 0);

	nfi_write16(nfc, NFI_CNFG, (CNFG_OP_CUSTOM << CNFG_OP_MODE_S) |
		    CNFG_READ_MODE);

	nfi_write16(nfc, NFI_CON,
		    CON_NFI_BRD | (nand->ecc.steps << CON_NFI_SEC_S));

	for (i = 0; i < nand->ecc.steps; i++) {
		/* Read data */
		if (buf)
			mt7621_nfc_read_data(nfc, page_data_ptr(nand, buf, i),
					     nand->ecc.size);
		else
			mt7621_nfc_read_data_discard(nfc, nand->ecc.size);

		/* Read FDM */
		mt7621_nfc_read_data(nfc, oob_fdm_ptr(nand, i), NFI_FDM_SIZE);

		/* Read ECC parity data */
		mt7621_nfc_read_data(nfc, oob_ecc_ptr(nfc, i),
				     nfc->spare_per_sector - NFI_FDM_SIZE);
	}

	nfi_write16(nfc, NFI_CON, 0);

	return 0;
}

static int mt7621_nfc_read_oob_hwecc(struct nand_chip *nand, int page)
{
	return mt7621_nfc_read_page_hwecc(nand, NULL, 1, page);
}

static int mt7621_nfc_read_oob_raw(struct nand_chip *nand, int page)
{
	return mt7621_nfc_read_page_raw(nand, NULL, 1, page);
}

static int mt7621_nfc_check_empty_page(struct nand_chip *nand, const u8 *buf)
{
	struct mtd_info *mtd = nand_to_mtd(nand);
	uint32_t i, j;
	u8 *oobptr;

	if (buf) {
		for (i = 0; i < mtd->writesize; i++)
			if (buf[i] != 0xff)
				return 0;
	}

	for (i = 0; i < nand->ecc.steps; i++) {
		oobptr = oob_fdm_ptr(nand, i);
		for (j = 0; j < NFI_FDM_SIZE; j++)
			if (oobptr[j] != 0xff)
				return 0;
	}

	return 1;
}

static int mt7621_nfc_write_page_hwecc(struct nand_chip *nand,
				       const uint8_t *buf, int oob_required,
				       int page)
{
	struct mt7621_nfc *nfc = nand_get_controller_data(nand);
	struct mtd_info *mtd = nand_to_mtd(nand);

	if (mt7621_nfc_check_empty_page(nand, buf)) {
		/*
		 * MT7621 ECC engine always generates parity code for input
		 * pages, even for empty pages. Doing so will write back ECC
		 * parity code to the oob region, which means such pages will
		 * no longer be empty pages.
		 *
		 * To avoid this, stop write operation if current page is an
		 * empty page.
		 */
		return 0;
	}

	nand_prog_page_begin_op(nand, page, 0, NULL, 0);

	nfi_write16(nfc, NFI_CNFG, (CNFG_OP_CUSTOM << CNFG_OP_MODE_S) |
		   CNFG_AUTO_FMT_EN | CNFG_HW_ECC_EN);

	mt7621_ecc_encoder_op(nfc, true);

	mt7621_nfc_write_fdm(nfc);

	nfi_write16(nfc, NFI_CON,
		    CON_NFI_BWR | (nand->ecc.steps << CON_NFI_SEC_S));

	if (buf)
		mt7621_nfc_write_data(nfc, buf, mtd->writesize);
	else
		mt7621_nfc_write_data_empty(nfc, mtd->writesize);

	mt7621_nfc_wait_write_completion(nfc, nand);

	mt7621_ecc_encoder_op(nfc, false);

	nfi_write16(nfc, NFI_CON, 0);

	return nand_prog_page_end_op(nand);
}

static int mt7621_nfc_write_page_raw(struct nand_chip *nand,
				     const uint8_t *buf, int oob_required,
				     int page)
{
	struct mt7621_nfc *nfc = nand_get_controller_data(nand);
	int i;

	nand_prog_page_begin_op(nand, page, 0, NULL, 0);

	nfi_write16(nfc, NFI_CNFG, (CNFG_OP_CUSTOM << CNFG_OP_MODE_S));

	nfi_write16(nfc, NFI_CON,
		    CON_NFI_BWR | (nand->ecc.steps << CON_NFI_SEC_S));

	for (i = 0; i < nand->ecc.steps; i++) {
		/* Write data */
		if (buf)
			mt7621_nfc_write_data(nfc, page_data_ptr(nand, buf, i),
					      nand->ecc.size);
		else
			mt7621_nfc_write_data_empty(nfc, nand->ecc.size);

		/* Write FDM */
		mt7621_nfc_write_data(nfc, oob_fdm_ptr(nand, i),
				      NFI_FDM_SIZE);

		/* Write dummy ECC parity data */
		mt7621_nfc_write_data_empty(nfc, nfc->spare_per_sector -
					    NFI_FDM_SIZE);
	}

	mt7621_nfc_wait_write_completion(nfc, nand);

	nfi_write16(nfc, NFI_CON, 0);

	return nand_prog_page_end_op(nand);
}

static int mt7621_nfc_write_oob_hwecc(struct nand_chip *nand, int page)
{
	return mt7621_nfc_write_page_hwecc(nand, NULL, 1, page);
}

static int mt7621_nfc_write_oob_raw(struct nand_chip *nand, int page)
{
	return mt7621_nfc_write_page_raw(nand, NULL, 1, page);
}

static int mt7621_nfc_init_chip(struct mt7621_nfc *nfc)
{
	struct nand_chip *nand = &nfc->nand;
	struct mtd_info *mtd;
	int ret;

	nand->controller = &nfc->controller;
	nand_set_controller_data(nand, (void *)nfc);
	nand_set_flash_node(nand, nfc->dev->of_node);

	nand->options |= NAND_USES_DMA | NAND_NO_SUBPAGE_WRITE | NAND_SKIP_BBTSCAN;
	if (!nfc->nfi_clk)
		nand->options |= NAND_KEEP_TIMINGS;

	nand->ecc.engine_type = NAND_ECC_ENGINE_TYPE_ON_HOST;
	nand->ecc.read_page = mt7621_nfc_read_page_hwecc;
	nand->ecc.read_page_raw = mt7621_nfc_read_page_raw;
	nand->ecc.write_page = mt7621_nfc_write_page_hwecc;
	nand->ecc.write_page_raw = mt7621_nfc_write_page_raw;
	nand->ecc.read_oob = mt7621_nfc_read_oob_hwecc;
	nand->ecc.read_oob_raw = mt7621_nfc_read_oob_raw;
	nand->ecc.write_oob = mt7621_nfc_write_oob_hwecc;
	nand->ecc.write_oob_raw = mt7621_nfc_write_oob_raw;

	mtd = nand_to_mtd(nand);
	mtd->owner = THIS_MODULE;
	mtd->dev.parent = nfc->dev;
	mtd->name = MT7621_NFC_NAME;
	mtd_set_ooblayout(mtd, &mt7621_nfc_ooblayout_ops);

	mt7621_nfc_hw_init(nfc);

	ret = nand_scan(nand, 1);
	if (ret)
		return ret;

	mtk_bmt_attach(mtd);

	ret = mtd_device_register(mtd, NULL, 0);
	if (ret) {
		dev_err(nfc->dev, "Failed to register MTD: %d\n", ret);
		mtk_bmt_detach(mtd);
		nand_cleanup(nand);
		return ret;
	}

	return 0;
}

static int mt7621_nfc_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct mt7621_nfc *nfc;
	struct resource *res;
	int ret;

	nfc = devm_kzalloc(dev, sizeof(*nfc), GFP_KERNEL);
	if (!nfc)
		return -ENOMEM;

	nand_controller_init(&nfc->controller);
	nfc->controller.ops = &mt7621_nfc_controller_ops;
	nfc->dev = dev;

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "nfi");
	nfc->nfi_base = res->start;
	nfc->nfi_regs = devm_ioremap_resource(dev, res);
	if (IS_ERR(nfc->nfi_regs)) {
		ret = PTR_ERR(nfc->nfi_regs);
		return ret;
	}

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "ecc");
	nfc->ecc_regs = devm_ioremap_resource(dev, res);
	if (IS_ERR(nfc->ecc_regs)) {
		ret = PTR_ERR(nfc->ecc_regs);
		return ret;
	}

	nfc->nfi_clk = devm_clk_get(dev, "nfi_clk");
	if (IS_ERR(nfc->nfi_clk)) {
		dev_warn(dev, "nfi clk not provided\n");
		nfc->nfi_clk = NULL;
	} else {
		ret = clk_prepare_enable(nfc->nfi_clk);
		if (ret) {
			dev_err(dev, "Failed to enable nfi core clock\n");
			return ret;
		}
	}

	platform_set_drvdata(pdev, nfc);

	ret = mt7621_nfc_init_chip(nfc);
	if (ret) {
		dev_err(dev, "Failed to initialize nand chip\n");
		goto clk_disable;
	}

	return 0;

clk_disable:
	clk_disable_unprepare(nfc->nfi_clk);

	return ret;
}

static int mt7621_nfc_remove(struct platform_device *pdev)
{
	struct mt7621_nfc *nfc = platform_get_drvdata(pdev);
	struct nand_chip *nand = &nfc->nand;
	struct mtd_info *mtd = nand_to_mtd(nand);

	mtk_bmt_detach(mtd);
	mtd_device_unregister(mtd);
	nand_cleanup(nand);
	clk_disable_unprepare(nfc->nfi_clk);

	return 0;
}

static const struct of_device_id mt7621_nfc_id_table[] = {
	{ .compatible = "mediatek,mt7621-nfc" },
	{ },
};
MODULE_DEVICE_TABLE(of, match);

static struct platform_driver mt7621_nfc_driver = {
	.probe = mt7621_nfc_probe,
	.remove = mt7621_nfc_remove,
	.driver = {
		.name = MT7621_NFC_NAME,
		.owner = THIS_MODULE,
		.of_match_table = mt7621_nfc_id_table,
	},
};
module_platform_driver(mt7621_nfc_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Weijie Gao <weijie.gao@mediatek.com>");
MODULE_DESCRIPTION("MediaTek MT7621 NAND Flash Controller driver");
