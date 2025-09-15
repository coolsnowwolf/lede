/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Phytium NAND flash controller driver
 *
 * Copyright (c) 2020-2023 Phytium Technology Co., Ltd.
 */
#ifndef PHYTIUM_NAND_H
#define PHYTIUM_NAND_H

#include <linux/module.h>
#include <linux/clk.h>
#include <linux/mtd/rawnand.h>
#include <linux/of_platform.h>
#include <linux/iopoll.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/mfd/syscon.h>
#include <linux/regmap.h>
#include <asm/unaligned.h>
#include <linux/dma-mapping.h>
#include <linux/workqueue.h>
#include <linux/mtd/partitions.h>

/* NFC does not support transfers of larger chunks at a time */
#define MAX_PAGE_NUM		16
#define MAX_CHUNK_SIZE		((1024 + 76) * 16)

#define POLL_PERIOD		0
#define POLL_TIMEOUT		100000
/* Interrupt maximum wait period in ms */
#define IRQ_TIMEOUT		1000

/* Latency in clock cycles between SoC pins and NFC logic */
#define MIN_RD_DEL_CNT		3

#define PHYTIUM_NFC_ADDR_MAX_LEN 5
#define PHYTIUM_NFC_DSP_SIZE     16

/* NAND controller flash control register */
#define NDCR0			0x00
#define NDCR0_EN		BIT(0)
#define NDCR0_WIDTH		BIT(1)
#define NDCR0_IN_MODE(x)	(min_t(u32, x, 0x3) << 2)
#define NDCR0_ECC_EN		BIT(4)
#define NDCR0_ECC_STREN(x)	(min_t(u32, x, 0x7) << 5)
#define NDCR0_SPARE_EN		BIT(8)
#define NDCR0_SPARE_SIZE(x)	(min_t(u32, x, 0xFFF) << 9)
#define NDCR0_GENERIC_FIELDS_MASK

#define NDCR1			0x04
#define NDCR1_SAMPL_PHASE(x)	min_t(u32, x, 0xFFFF)
#define NDCR1_ECC_DATA_FIRST_EN BIT(16)
#define NDCR1_RB_SHARE_EN       BIT(17)
#define NDCR1_ECC_BYPASS        BIT(18)

#define NDAR0			0x08

#define NDAR1			0x0C
#define NDAR1_H8(x)		min_t(u32, x, 0xFF)
#define NDAR1_DMA_EN		BIT(8)
#define NDAR1_EMPTY(x)		(min_t(u32, x, 0x7F) << 9)
#define NDAR1_DMA_RLEN(x)	(min_t(u32, x, 0xFF) << 9)
#define NDAR1_DMA_WLEN(x)	(min_t(u32, x, 0xFF) << 9)

#define NDTR0			0x10
#define NDTR0_TCS_TCLS(x)	(min_t(u32, x, 0xFFFF) << 0)
#define NDTR0_TCLS_TWP(x)	(min_t(u32, x, 0xFFFF) << 16)

#define NDTR1			0x14
#define NDTR1_TWH(x)		(min_t(u32, x, 0xFFFF) << 0)
#define NDTR1_TWP(x)		(min_t(u32, x, 0xFFFF) << 16)

#define NDTR2			0x18
#define NDTR2_TCH_TCLH(x)	(min_t(u32, x, 0xFFFF) << 0)
#define NDTR2_TCLH_TWH(x)	(min_t(u32, x, 0xFFFF) << 16)

#define NDTR3			0x1c
#define NDTR3_TDQ_EN(x)		(min_t(u32, x, 0xFFFF) << 0)
#define NDTR3_TCH_TWH(x)	(min_t(u32, x, 0xFFFF) << 16)

#define NDTR4			0x20
#define NDTR4_TWHR_SMX(x)	(min_t(u32, x, 0xFFFF) << 0)
#define NDTR4_TREH(x)		(min_t(u32, x, 0xFFFF) << 16)

#define NDTR5			0x24
#define NDTR5_TRC(x)		(min_t(u32, x, 0xFFFF) << 0)
#define NDTR5_TADL_SMX(x)	(min_t(u32, x, 0xFFFF) << 16)

#define NDTR6			0x28
#define NDTR6_TCAD_TCS_SMX(x)	(min_t(u32, x, 0xFFFF) << 0)
#define NDTR6_RES(x)		(min_t(u32, x, 0xFFFF) << 16)

#define NDTR7			0x2c
#define NDTR7_TCK(x)		(min_t(u32, x, 0xFFFF) << 0)
#define NDTR7_TDQ_EN(x)		(min_t(u32, x, 0xFFFF) << 16)

#define NDTR8			0x30
#define NDTR8_TCAD_TCK_SMX(x)	(min_t(u32, x, 0xFFFF) << 0)
#define NDTR8_HF_TCK(x)		(min_t(u32, x, 0xFFFF) << 16)

#define NDTR9			0x34
#define NDTR9_TWHR(x)		(min_t(u32, x, 0xFFFF) << 0)
#define NDTR9_TCCS_TCALS_SMX(x)	(min_t(u32, x, 0xFFFF) << 16)

#define NDTR10			0x38
#define NDTR10_TCK(x)		(min_t(u32, x, 0xFFFF) << 0)
#define NDTR10_MTCK(x)		(min_t(u32, x, 0xFFFF) << 16)

#define NDTR11			0x3c
#define NDTR11_TCK_TCALS(x)	(min_t(u32, x, 0xFFFF) << 16)
#define NDTR11_RES(x)		(min_t(u32, x, 0xFFFF) << 16)

#define NDTR12			0x40
#define NDTR12_TWRCK(x)		(min_t(u32, x, 0xFFFF) << 0)
#define NDTR12_RES(x)		(min_t(u32, x, 0xFFFF) << 16)

#define NDTR13			0x44
#define NDTR13_TWRHCA(x)	(min_t(u32, x, 0xFFFF) << 0)
#define NDTR13_TRLCA(x)		(min_t(u32, x, 0xFFFF) << 16)

#define NDTR14			0x48
#define NDTR14_TWRHCE(x)	(min_t(u32, x, 0xFFFF) << 0)
#define NDTR14_RES(x)		(min_t(u32, x, 0xFFFF) << 16)

#define NDTR15			0x4c
#define NDTR15_TCDQSS_TWPRE_TDS(x) (min_t(u32, x, 0xFFFF) << 0)
#define NDTR15_HFTDSC(x)	(min_t(u32, x, 0xFFFF) << 16)

#define NDTR16			0x50
#define NDTR16_TWPST_TDH(x)	(min_t(u32, x, 0xFFFF) << 0)
#define NDTR16_TWPSTH(x)	(min_t(u32, x, 0xFFFF) << 16)

#define NDTR17			0x54
#define NDTR17_TCS_TRPRE(x)	(min_t(u32, x, 0xFFFF) << 0)
#define NDTR17_TRELDQS(x)	(min_t(u32, x, 0xFFFF) << 16)

#define NDTR18			0x58
#define NDTR18_TRPST_TDQSRE(x)	(min_t(u32, x, 0xFFFF) << 0)
#define NDTR18_RES(x)		(min_t(u32, x, 0xFFFF) << 16)

#define NDFIFO			0x5c
#define NDFIFO_REV		(min_t(u32, x, 0) << 12)
#define NDFIFO_FULL		BIT(11)
#define NDFIFO_EMP		BIT(10)
#define NDFIFO_CNT(x)		(min_t(u32, x, 0x3F) << 0)

#define ND_INTERVAL_TIME	0x60
#define NDCMD_INTERVAL_TIME	0x64
#define NDFIFO_TIMEOUT		0x68
#define NDFIFO_LEVEL0		0x6c
#define NDFIFO_LEVEL1		0x70
#define NDWP			0x74
#define NDFIFO_CLR		0x78

#define NDSR			0x7c
#define NDSR_BUSY		BIT(0)
#define NDSR_DMA_BUSY		BIT(1)
#define NDSR_DMA_PGFINISH	BIT(2)
#define NDSR_DMA_FINISH		BIT(3)
#define NDSR_FIFO_EMP		BIT(4)
#define NDSR_FIFO_FULL		BIT(5)
#define NDSR_FIFO_TIMEOUT	BIT(6)
#define NDSR_CS(x)		(min_t(u32, x, 0xF) << 7)
#define NDSR_CMD_PGFINISH	BIT(11)
#define NDSR_PG_PGFINISH	BIT(12)
#define NDSR_RE			BIT(13)
#define NDSR_DQS		BIT(14)
#define NDSR_RB			BIT(15)
#define NDSR_ECC_BUSY		BIT(16)
#define NDSR_ECC_FINISH		BIT(17)
#define NDSR_ECC_RIGHT		BIT(18)
#define NDSR_ECC_ERR		BIT(19)
#define NDSR_ECC_ERROVER	BIT(20)
#define NDSR_AXI_DSP_ERR	BIT(21)
#define NDSR_AXI_RD_ERR		BIT(22)
#define NDSR_AXI_WR_ERR		BIT(23)
#define NDSR_RB_STATUS(x)       (min_t(u32, x, 0xF) << 24)
#define NDSR_PROT_ERR           BIT(28)
#define NDSR_ECC_BYPASS         BIT(29)

#define NDIR_MASK		0x80
#define NDIR_BUSY_MASK		BIT(0)
#define NDIR_DMA_BUSY_MASK	BIT(1)
#define NDIR_DMA_PGFINISH_MASK	BIT(2)
#define NDIR_DMA_FINISH_MASK	BIT(3)
#define NDIR_FIFO_EMP_MASK	BIT(4)
#define NDIR_FIFO_FULL_MASK	BIT(5)
#define NDIR_FIFO_TIMEOUT_MASK	BIT(6)
#define NDIR_CMD_FINISH_MASK	BIT(7)
#define NDIR_PGFINISH_MASK	BIT(8)
#define NDIR_RE_MASK		BIT(9)
#define NDIR_DQS_MASK		BIT(10)
#define NDIR_RB_MASK		BIT(11)
#define NDIR_ECC_FINISH_MASK	BIT(12)
#define NDIR_ECC_ERR_MASK	BIT(13)

#define NDIR			0x84
#define NDIR_ALL_INT(x)		GENMASK(x, 0)
#define NDIR_BUSY		BIT(0)
#define NDIR_DMA_BUSY		BIT(1)
#define NDIR_DMA_PGFINISH	BIT(2)
#define NDIR_DMA_FINISH		BIT(3)
#define NDIR_FIFO_EMP		BIT(4)
#define NDIR_FIFO_FULL		BIT(5)
#define NDIR_FIFO_TIMEOUT	BIT(6)
#define NDIR_CMD_FINISH		BIT(7)
#define NDIR_PGFINISH		BIT(8)
#define NDIR_RE			BIT(9)
#define NDIR_DQS		BIT(10)
#define NDIR_RB			BIT(11)
#define NDIR_ECC_FINISH		BIT(12)
#define NDIR_ECC_ERR		BIT(13)

#define ND_DEBUG		0x88

#define ND_ERR_CLR		0x8c
#define ND_DSP_ERR_CLR		BIT(0)
#define ND_AXI_RD_ERR_CLR	BIT(1)
#define ND_AXI_WR_ERR_CLR	BIT(2)
#define ND_ECC_ERR_CLR		BIT(3)

#define NDCR_PAGE_SZ(x)		(x >= 2048 ? BIT(24) : 0)

enum nand_inter_pro {
	NAND_ONFI,
	NAND_JEDEC,
	NAND_OTHER,
};

enum nand_inter_mode {
	ASYN_SDR,
	ONFI_DDR,
	TOG_ASYN_DDR,
};

enum asy_timing_mode {
	ASY_MODE0,
	ASY_MODE1,
	ASY_MODE2,
	ASY_MODE3,
	ASY_MODE4,
};

enum onfi_syn_timing_mode {
	SYN_MODE0 = 0x10,
	SYN_MODE1,
	SYN_MODE2,
	SYN_MODE3,
	SYN_MODE4,
};

/**
 * NAND controller timings expressed in NAND Controller clock cycles
 *
 * @tRP:	ND_nRE pulse width
 * @tRH:	ND_nRE high duration
 * @tWP:	ND_nWE pulse time
 * @tWH:	ND_nWE high duration
 * @tCS:	Enable signal setup time
 * @tCH:	Enable signal hold time
 * @tADL:	Address to write data delay
 * @tAR:	ND_ALE low to ND_nRE low delay
 * @tWHR:	ND_nWE high to ND_nRE low for status read
 * @tRHW:	ND_nRE high duration, read to write delay
 * @tR:		ND_nWE high to ND_nRE low for read
 */
struct phytium_nfc_timings {
	u16 tRP;
	u16 tRH;
	u16 tWP;       /* NDTR1_TWP */
	u16 tWH;       /* NDTR1_TWH */
	u16 tCS;
	u16 tCH;
	u16 tADL;
	u16 tAR;
	u16 tWHR;
	u16 tRHW;
	u16 tR;
};

/**
 * NAND chip structure: stores NAND chip device related information
 *
 * @chip:		Base NAND chip structure
 * @node:		Used to store NAND chips into a list
 * @ndcr:		Controller register value for this NAND chip
 * @ndtr0:		Timing registers 0 value for this NAND chip
 * @ndtr1:		Timing registers 1 value for this NAND chip
 * @selected_die:	Current active CS
 * @nsels:		Number of CS lines required by the NAND chip
 */
struct phytium_nand_chip {
	struct nand_chip chip;
	struct list_head node;
	u32 ndcr;
	u32 ndtr0;
	u32 ndtr1;
	int addr_cyc;
	int selected_die;
	unsigned int nsels;
	struct mtd_oob_region ecc;
};

/**
 * NAND controller capabilities for distinction between compatible strings
 *
 * @max_cs_nb:		Number of Chip Select lines available
 * @max_rb_nb:		Number of Ready/Busy lines available
 * @legacy_of_bindings:	Indicates if DT parsing must be done using the old
 *			fashion way
 * @flash_bbt:
 * @ecc_strength:
 * @ecc_step_size:
 * @parts:
 * @nr_parts:
 */
struct phytium_nfc_caps {
	unsigned int hw_ver;
	unsigned int int_mask_bits;
	unsigned int max_cs_nb;
	unsigned int max_rb_nb;
	bool legacy_of_bindings;
	bool flash_bbt;
	int ecc_strength;
	int ecc_step_size;
	struct mtd_partition *parts;
	unsigned int nr_parts;
};

/**
 * NAND controller structure: stores Marvell NAND controller information
 *
 * @controller:		Base controller structure
 * @dev:		Parent device (used to print error messages)
 * @regs:		NAND controller registers
 * @reg_clk:		Regiters clock
 * @complete:		Completion object to wait for NAND controller events
 * @chips:		List containing all the NAND chips attached to
 *			this NAND controller
 * @caps:		NAND controller capabilities for each compatible string
 * @dma_buf:		32-bit aligned buffer for DMA transfers (NFCv1 only)
 */
struct phytium_nfc {
	struct nand_controller controller;
	struct device *dev;
	void __iomem *regs;
	int irq;
	struct list_head chips;
	struct nand_chip *selected_chip;
	struct phytium_nfc_caps *caps;

	void *dsp_addr;
	dma_addr_t dsp_phy_addr;

	void *dma_buf;
	u32 dma_offset;
	dma_addr_t dma_phy_addr;

	enum nand_inter_pro inter_pro;
	enum nand_inter_mode inter_mode;
	u32 timing_mode;

	spinlock_t spinlock;
};

/**
 * Derives a duration in numbers of clock cycles.
 *
 * @ps: Duration in pico-seconds
 * @period_ns:  Clock period in nano-seconds
 *
 * Convert the duration in nano-seconds, then divide by the period and
 * return the number of clock periods.
 */
#define TO_CYCLES(ps, period_ns) (DIV_ROUND_UP(ps / 1000, period_ns))
#define TO_CYCLES64(ps, period_ns) (DIV_ROUND_UP_ULL(div_u64(ps, 1000), \
						     period_ns))

struct phytium_nfc_cmd_ctrl {
	u16 csel:4;
	u16 dbc:1;
	u16 addr_cyc:3;
	u16 nc:1;
#define TYPE_RESET         0x00
#define TYPE_SET_FTR       0x01
#define TYPE_GET_FTR       0x02
#define TYPE_READ_ID       0x03
#define TYPE_PAGE_PRO      0x04
#define TYPE_ERASE         0x05
#define TYPE_READ          0x06
#define TYPE_TOGGLE        0x07
#define TYPE_READ_PARAM    0x02
#define TYPE_READ_STATUS   0x03
#define TYPE_CH_READ_COL   0x03
#define TYPE_CH_ROW_ADDR   0x01
#define TYPE_CH_WR_COL     0x01
	u16 cmd_type:4;
	u16 dc:1;
	u16 auto_rs:1;
	u16 ecc_en:1;
};

/**
 * NAND driver structure filled during the parsing of the ->exec_op() subop
 * subset of instructions.
 *
 * @cle_ale_delay_ns:	Optional delay after the last CMD or ADDR cycle
 * @rdy_timeout_ms:	Timeout for waits on Ready/Busy pin
 * @rdy_delay_ns:	Optional delay after waiting for the RB pin
 * @data_delay_ns:	Optional delay after the data xfer
 * @data_instr_idx:	Index of the data instruction in the subop
 * @data_instr:		Pointer to the data instruction in the subop
 */
struct phytium_nfc_op {
	u8 cmd[2];
	union {
		u16 ctrl;
		struct phytium_nfc_cmd_ctrl nfc_ctrl;
	} cmd_ctrl;
	u8 addr[PHYTIUM_NFC_ADDR_MAX_LEN];
	u16 page_cnt;
	u8 mem_addr_first[PHYTIUM_NFC_ADDR_MAX_LEN];

	u32 cmd_len;
	u32 addr_len;

	u32 cle_ale_delay_ns;
	u32 rdy_timeout_ms;
	u32 rdy_delay_ns;
	u32 data_delay_ns;
	u32 data_instr_idx;
	struct nand_op_instr *data_instr;
} __attribute__ ((__packed__));

#define TIMING_ASY_NUM 12
#define TIMING_SYN_NUM 14
#define TIMING_TOG_NUM 12

#define TMP_DMA_DEBUG 0    /* Temporary dma space */

int phytium_nand_init(struct phytium_nfc *nfc);
int phytium_nand_remove(struct phytium_nfc *nfc);
int phytium_nand_prepare(struct phytium_nfc *nfc);
int phytium_nand_suspend(struct phytium_nfc *nfc);
int phytium_nand_resume(struct phytium_nfc *nfc);

irqreturn_t phytium_nfc_isr(int irq, void *dev_id);

#endif /* NAND_PHYTIUM_NAND_H */
