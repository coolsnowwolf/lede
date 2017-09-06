/*
 * Driver for the built-in NAND controller of the Atheros AR934x SoCs
 *
 * Copyright (C) 2011-2013 Gabor Juhos <juhosg@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/dma-mapping.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/partitions.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/slab.h>

#include <linux/platform/ar934x_nfc.h>

#define AR934X_NFC_REG_CMD		0x00
#define AR934X_NFC_REG_CTRL		0x04
#define AR934X_NFC_REG_STATUS		0x08
#define AR934X_NFC_REG_INT_MASK		0x0c
#define AR934X_NFC_REG_INT_STATUS	0x10
#define AR934X_NFC_REG_ECC_CTRL		0x14
#define AR934X_NFC_REG_ECC_OFFSET	0x18
#define AR934X_NFC_REG_ADDR0_0		0x1c
#define AR934X_NFC_REG_ADDR0_1		0x24
#define AR934X_NFC_REG_ADDR1_0		0x20
#define AR934X_NFC_REG_ADDR1_1		0x28
#define AR934X_NFC_REG_SPARE_SIZE	0x30
#define AR934X_NFC_REG_PROTECT		0x38
#define AR934X_NFC_REG_LOOKUP_EN	0x40
#define AR934X_NFC_REG_LOOKUP(_x)	(0x44 + (_i) * 4)
#define AR934X_NFC_REG_DMA_ADDR		0x64
#define AR934X_NFC_REG_DMA_COUNT	0x68
#define AR934X_NFC_REG_DMA_CTRL		0x6c
#define AR934X_NFC_REG_MEM_CTRL		0x80
#define AR934X_NFC_REG_DATA_SIZE	0x84
#define AR934X_NFC_REG_READ_STATUS	0x88
#define AR934X_NFC_REG_TIME_SEQ		0x8c
#define AR934X_NFC_REG_TIMINGS_ASYN	0x90
#define AR934X_NFC_REG_TIMINGS_SYN	0x94
#define AR934X_NFC_REG_FIFO_DATA	0x98
#define AR934X_NFC_REG_TIME_MODE	0x9c
#define AR934X_NFC_REG_DMA_ADDR_OFFS	0xa0
#define AR934X_NFC_REG_FIFO_INIT	0xb0
#define AR934X_NFC_REG_GEN_SEQ_CTRL	0xb4

#define AR934X_NFC_CMD_CMD_SEQ_S		0
#define AR934X_NFC_CMD_CMD_SEQ_M		0x3f
#define   AR934X_NFC_CMD_SEQ_1C			0x00
#define   AR934X_NFC_CMD_SEQ_ERASE		0x0e
#define   AR934X_NFC_CMD_SEQ_12			0x0c
#define   AR934X_NFC_CMD_SEQ_1C1AXR		0x21
#define   AR934X_NFC_CMD_SEQ_S			0x24
#define   AR934X_NFC_CMD_SEQ_1C3AXR		0x27
#define   AR934X_NFC_CMD_SEQ_1C5A1CXR		0x2a
#define   AR934X_NFC_CMD_SEQ_18			0x32
#define AR934X_NFC_CMD_INPUT_SEL_SIU		0
#define AR934X_NFC_CMD_INPUT_SEL_DMA		BIT(6)
#define AR934X_NFC_CMD_ADDR_SEL_0		0
#define AR934X_NFC_CMD_ADDR_SEL_1		BIT(7)
#define AR934X_NFC_CMD_CMD0_S			8
#define AR934X_NFC_CMD_CMD0_M			0xff
#define AR934X_NFC_CMD_CMD1_S			16
#define AR934X_NFC_CMD_CMD1_M			0xff
#define AR934X_NFC_CMD_CMD2_S			24
#define AR934X_NFC_CMD_CMD2_M			0xff

#define AR934X_NFC_CTRL_ADDR_CYCLE0_M		0x7
#define AR934X_NFC_CTRL_ADDR_CYCLE0_S		0
#define AR934X_NFC_CTRL_SPARE_EN		BIT(3)
#define AR934X_NFC_CTRL_INT_EN			BIT(4)
#define AR934X_NFC_CTRL_ECC_EN			BIT(5)
#define AR934X_NFC_CTRL_BLOCK_SIZE_S		6
#define AR934X_NFC_CTRL_BLOCK_SIZE_M		0x3
#define   AR934X_NFC_CTRL_BLOCK_SIZE_32		0
#define   AR934X_NFC_CTRL_BLOCK_SIZE_64		1
#define   AR934X_NFC_CTRL_BLOCK_SIZE_128	2
#define   AR934X_NFC_CTRL_BLOCK_SIZE_256	3
#define AR934X_NFC_CTRL_PAGE_SIZE_S		8
#define AR934X_NFC_CTRL_PAGE_SIZE_M		0x7
#define   AR934X_NFC_CTRL_PAGE_SIZE_256		0
#define   AR934X_NFC_CTRL_PAGE_SIZE_512		1
#define   AR934X_NFC_CTRL_PAGE_SIZE_1024	2
#define   AR934X_NFC_CTRL_PAGE_SIZE_2048	3
#define   AR934X_NFC_CTRL_PAGE_SIZE_4096	4
#define   AR934X_NFC_CTRL_PAGE_SIZE_8192	5
#define   AR934X_NFC_CTRL_PAGE_SIZE_16384	6
#define AR934X_NFC_CTRL_CUSTOM_SIZE_EN		BIT(11)
#define AR934X_NFC_CTRL_IO_WIDTH_8BITS		0
#define AR934X_NFC_CTRL_IO_WIDTH_16BITS		BIT(12)
#define AR934X_NFC_CTRL_LOOKUP_EN		BIT(13)
#define AR934X_NFC_CTRL_PROT_EN			BIT(14)
#define AR934X_NFC_CTRL_WORK_MODE_ASYNC		0
#define AR934X_NFC_CTRL_WORK_MODE_SYNC		BIT(15)
#define AR934X_NFC_CTRL_ADDR0_AUTO_INC		BIT(16)
#define AR934X_NFC_CTRL_ADDR1_AUTO_INC		BIT(17)
#define AR934X_NFC_CTRL_ADDR_CYCLE1_M		0x7
#define AR934X_NFC_CTRL_ADDR_CYCLE1_S		18
#define AR934X_NFC_CTRL_SMALL_PAGE		BIT(21)

#define AR934X_NFC_DMA_CTRL_DMA_START		BIT(7)
#define AR934X_NFC_DMA_CTRL_DMA_DIR_WRITE	0
#define AR934X_NFC_DMA_CTRL_DMA_DIR_READ	BIT(6)
#define AR934X_NFC_DMA_CTRL_DMA_MODE_SG		BIT(5)
#define AR934X_NFC_DMA_CTRL_DMA_BURST_S		2
#define AR934X_NFC_DMA_CTRL_DMA_BURST_0		0
#define AR934X_NFC_DMA_CTRL_DMA_BURST_1		1
#define AR934X_NFC_DMA_CTRL_DMA_BURST_2		2
#define AR934X_NFC_DMA_CTRL_DMA_BURST_3		3
#define AR934X_NFC_DMA_CTRL_DMA_BURST_4		4
#define AR934X_NFC_DMA_CTRL_DMA_BURST_5		5
#define AR934X_NFC_DMA_CTRL_ERR_FLAG		BIT(1)
#define AR934X_NFC_DMA_CTRL_DMA_READY		BIT(0)

#define AR934X_NFC_INT_DEV_RDY(_x)		BIT(4 + (_x))
#define AR934X_NFC_INT_CMD_END			BIT(1)

#define AR934X_NFC_ECC_CTRL_ERR_THRES_S		8
#define AR934X_NFC_ECC_CTRL_ERR_THRES_M		0x1f
#define AR934X_NFC_ECC_CTRL_ECC_CAP_S		5
#define AR934X_NFC_ECC_CTRL_ECC_CAP_M		0x7
#define AR934X_NFC_ECC_CTRL_ECC_CAP_2		0
#define AR934X_NFC_ECC_CTRL_ECC_CAP_4		1
#define AR934X_NFC_ECC_CTRL_ECC_CAP_6		2
#define AR934X_NFC_ECC_CTRL_ECC_CAP_8		3
#define AR934X_NFC_ECC_CTRL_ECC_CAP_10		4
#define AR934X_NFC_ECC_CTRL_ECC_CAP_12		5
#define AR934X_NFC_ECC_CTRL_ECC_CAP_14		6
#define AR934X_NFC_ECC_CTRL_ECC_CAP_16		7
#define AR934X_NFC_ECC_CTRL_ERR_OVER		BIT(2)
#define AR934X_NFC_ECC_CTRL_ERR_UNCORRECT	BIT(1)
#define AR934X_NFC_ECC_CTRL_ERR_CORRECT		BIT(0)

#define AR934X_NFC_ECC_OFFS_OFSET_M		0xffff

/* default timing values */
#define AR934X_NFC_TIME_SEQ_DEFAULT	0x7fff
#define AR934X_NFC_TIMINGS_ASYN_DEFAULT	0x22
#define AR934X_NFC_TIMINGS_SYN_DEFAULT	0xf

#define AR934X_NFC_ID_BUF_SIZE		8
#define AR934X_NFC_DEV_READY_TIMEOUT	25 /* msecs */
#define AR934X_NFC_DMA_READY_TIMEOUT	25 /* msecs */
#define AR934X_NFC_DONE_TIMEOUT		1000
#define AR934X_NFC_DMA_RETRIES		20

#define AR934X_NFC_USE_IRQ		true
#define AR934X_NFC_IRQ_MASK		AR934X_NFC_INT_DEV_RDY(0)

#define  AR934X_NFC_GENSEQ_SMALL_PAGE_READ	0x30043

#undef AR934X_NFC_DEBUG_DATA
#undef AR934X_NFC_DEBUG

struct ar934x_nfc;

static inline  __attribute__ ((format (printf, 2, 3)))
void _nfc_dbg(struct ar934x_nfc *nfc, const char *fmt, ...)
{
}

#ifdef AR934X_NFC_DEBUG
#define nfc_dbg(_nfc, fmt, ...) \
	dev_info((_nfc)->parent, fmt, ##__VA_ARGS__)
#else
#define nfc_dbg(_nfc, fmt, ...) \
	_nfc_dbg((_nfc), fmt, ##__VA_ARGS__)
#endif /* AR934X_NFC_DEBUG */

#ifdef AR934X_NFC_DEBUG_DATA
static void
nfc_debug_data(const char *label, void *data, int len)
{
	print_hex_dump(KERN_WARNING, label, DUMP_PREFIX_OFFSET, 16, 1,
		       data, len, 0);
}
#else
static inline void
nfc_debug_data(const char *label, void *data, int len) {}
#endif /* AR934X_NFC_DEBUG_DATA */

struct ar934x_nfc {
	struct mtd_info mtd;
	struct nand_chip nand_chip;
	struct device *parent;
	void __iomem *base;
	void (*select_chip)(int chip_no);
	bool swap_dma;
	int irq;
	wait_queue_head_t irq_waitq;

	bool spurious_irq_expected;
	u32 irq_status;

	u32 ctrl_reg;
	u32 ecc_ctrl_reg;
	u32 ecc_offset_reg;
	u32 ecc_thres;
	u32 ecc_oob_pos;

	bool small_page;
	unsigned int addr_count0;
	unsigned int addr_count1;

	u8 *buf;
	dma_addr_t buf_dma;
	unsigned int buf_size;
	int buf_index;

	bool read_id;

	int erase1_page_addr;

	int rndout_page_addr;
	int rndout_read_cmd;

	int seqin_page_addr;
	int seqin_column;
	int seqin_read_cmd;
};

static void ar934x_nfc_restart(struct ar934x_nfc *nfc);

static inline bool
is_all_ff(u8 *buf, int len)
{
	while (len--)
		if (buf[len] != 0xff)
			return false;

	return true;
}

static inline void
ar934x_nfc_wr(struct ar934x_nfc *nfc, unsigned reg, u32 val)
{
	__raw_writel(val, nfc->base + reg);
}

static inline u32
ar934x_nfc_rr(struct ar934x_nfc *nfc, unsigned reg)
{
	return __raw_readl(nfc->base + reg);
}

static inline struct ar934x_nfc_platform_data *
ar934x_nfc_get_platform_data(struct ar934x_nfc *nfc)
{
	return nfc->parent->platform_data;
}

static inline struct
ar934x_nfc *mtd_to_ar934x_nfc(struct mtd_info *mtd)
{
	return container_of(mtd, struct ar934x_nfc, mtd);
}

static inline bool ar934x_nfc_use_irq(struct ar934x_nfc *nfc)
{
	return AR934X_NFC_USE_IRQ;
}

static inline void ar934x_nfc_write_cmd_reg(struct ar934x_nfc *nfc, u32 cmd_reg)
{
	wmb();

	ar934x_nfc_wr(nfc, AR934X_NFC_REG_CMD, cmd_reg);
	/* flush write */
	ar934x_nfc_rr(nfc, AR934X_NFC_REG_CMD);
}

static bool
__ar934x_nfc_dev_ready(struct ar934x_nfc *nfc)
{
	u32 status;

	status = ar934x_nfc_rr(nfc, AR934X_NFC_REG_STATUS);
	return (status & 0xff) == 0xff;
}

static inline bool
__ar934x_nfc_is_dma_ready(struct ar934x_nfc *nfc)
{
	u32 status;

	status = ar934x_nfc_rr(nfc, AR934X_NFC_REG_DMA_CTRL);
	return (status & AR934X_NFC_DMA_CTRL_DMA_READY) != 0;
}

static int
ar934x_nfc_wait_dev_ready(struct ar934x_nfc *nfc)
{
	unsigned long timeout;

	timeout = jiffies + msecs_to_jiffies(AR934X_NFC_DEV_READY_TIMEOUT);
	do {
		if (__ar934x_nfc_dev_ready(nfc))
			return 0;
	} while time_before(jiffies, timeout);

	nfc_dbg(nfc, "timeout waiting for device ready, status:%08x int:%08x\n",
		ar934x_nfc_rr(nfc, AR934X_NFC_REG_STATUS),
		ar934x_nfc_rr(nfc, AR934X_NFC_REG_INT_STATUS));
	return -ETIMEDOUT;
}

static int
ar934x_nfc_wait_dma_ready(struct ar934x_nfc *nfc)
{
	unsigned long timeout;

	timeout = jiffies + msecs_to_jiffies(AR934X_NFC_DMA_READY_TIMEOUT);
	do {
		if (__ar934x_nfc_is_dma_ready(nfc))
			return 0;
	} while time_before(jiffies, timeout);

	nfc_dbg(nfc, "timeout waiting for DMA ready, dma_ctrl:%08x\n",
		ar934x_nfc_rr(nfc, AR934X_NFC_REG_DMA_CTRL));
	return -ETIMEDOUT;
}

static int
ar934x_nfc_wait_irq(struct ar934x_nfc *nfc)
{
	long timeout;
	int ret;

	timeout = wait_event_timeout(nfc->irq_waitq,
				(nfc->irq_status & AR934X_NFC_IRQ_MASK) != 0,
				msecs_to_jiffies(AR934X_NFC_DEV_READY_TIMEOUT));

	ret = 0;
	if (!timeout) {
		ar934x_nfc_wr(nfc, AR934X_NFC_REG_INT_MASK, 0);
		ar934x_nfc_wr(nfc, AR934X_NFC_REG_INT_STATUS, 0);
		/* flush write */
		ar934x_nfc_rr(nfc, AR934X_NFC_REG_INT_STATUS);

		nfc_dbg(nfc,
			"timeout waiting for interrupt, status:%08x\n",
			nfc->irq_status);
		ret = -ETIMEDOUT;
	}

	nfc->irq_status = 0;
	return ret;
}

static int
ar934x_nfc_wait_done(struct ar934x_nfc *nfc)
{
	int ret;

	if (ar934x_nfc_use_irq(nfc))
		ret = ar934x_nfc_wait_irq(nfc);
	else
		ret = ar934x_nfc_wait_dev_ready(nfc);

	if (ret)
		return ret;

	return ar934x_nfc_wait_dma_ready(nfc);
}

static int
ar934x_nfc_alloc_buf(struct ar934x_nfc *nfc, unsigned size)
{
	nfc->buf = dma_alloc_coherent(nfc->parent, size,
				      &nfc->buf_dma, GFP_KERNEL);
	if (nfc->buf == NULL) {
		dev_err(nfc->parent, "no memory for DMA buffer\n");
		return -ENOMEM;
	}

	nfc->buf_size = size;
	nfc_dbg(nfc, "buf:%p size:%u\n", nfc->buf, nfc->buf_size);

	return 0;
}

static void
ar934x_nfc_free_buf(struct ar934x_nfc *nfc)
{
	dma_free_coherent(nfc->parent, nfc->buf_size, nfc->buf, nfc->buf_dma);
}

static void
ar934x_nfc_get_addr(struct ar934x_nfc *nfc, int column, int page_addr,
		    u32 *addr0, u32 *addr1)
{
	u32 a0, a1;

	a0 = 0;
	a1 = 0;

	if (column == -1) {
		/* ERASE1 */
		a0 = (page_addr & 0xffff) << 16;
		a1 = (page_addr >> 16) & 0xf;
	} else if (page_addr != -1) {
		/* SEQIN, READ0, etc.. */

		/* TODO: handle 16bit bus width */
		if (nfc->small_page) {
			a0 = column & 0xff;
			a0 |= (page_addr & 0xff) << 8;
			a0 |= ((page_addr >> 8) & 0xff) << 16;
			a0 |= ((page_addr >> 16) & 0xff) << 24;
		} else {
			a0 = column & 0x0FFF;
			a0 |= (page_addr & 0xffff) << 16;

			if (nfc->addr_count0 > 4)
				a1 = (page_addr >> 16) & 0xf;
		}
	}

	*addr0 = a0;
	*addr1 = a1;
}

static void
ar934x_nfc_send_cmd(struct ar934x_nfc *nfc, unsigned command)
{
	u32 cmd_reg;

	cmd_reg = AR934X_NFC_CMD_INPUT_SEL_SIU | AR934X_NFC_CMD_ADDR_SEL_0 |
		  AR934X_NFC_CMD_SEQ_1C;
	cmd_reg |= (command & AR934X_NFC_CMD_CMD0_M) << AR934X_NFC_CMD_CMD0_S;

	ar934x_nfc_wr(nfc, AR934X_NFC_REG_INT_STATUS, 0);
	ar934x_nfc_wr(nfc, AR934X_NFC_REG_CTRL, nfc->ctrl_reg);

	ar934x_nfc_write_cmd_reg(nfc, cmd_reg);
	ar934x_nfc_wait_dev_ready(nfc);
}

static int
ar934x_nfc_do_rw_command(struct ar934x_nfc *nfc, int column, int page_addr,
			 int len, u32 cmd_reg, u32 ctrl_reg, bool write)
{
	u32 addr0, addr1;
	u32 dma_ctrl;
	int dir;
	int err;
	int retries = 0;

	WARN_ON(len & 3);

	if (WARN_ON(len > nfc->buf_size))
		dev_err(nfc->parent, "len=%d > buf_size=%d", len, nfc->buf_size);

	if (write) {
		dma_ctrl = AR934X_NFC_DMA_CTRL_DMA_DIR_WRITE;
		dir = DMA_TO_DEVICE;
	} else {
		dma_ctrl = AR934X_NFC_DMA_CTRL_DMA_DIR_READ;
		dir = DMA_FROM_DEVICE;
	}

	ar934x_nfc_get_addr(nfc, column, page_addr, &addr0, &addr1);

	dma_ctrl |= AR934X_NFC_DMA_CTRL_DMA_START |
		    (AR934X_NFC_DMA_CTRL_DMA_BURST_3 <<
		     AR934X_NFC_DMA_CTRL_DMA_BURST_S);

	cmd_reg |= AR934X_NFC_CMD_INPUT_SEL_DMA | AR934X_NFC_CMD_ADDR_SEL_0;
	ctrl_reg |= AR934X_NFC_CTRL_INT_EN;

	nfc_dbg(nfc, "%s a0:%08x a1:%08x len:%x cmd:%08x dma:%08x ctrl:%08x\n",
		(write) ? "write" : "read",
		addr0, addr1, len, cmd_reg, dma_ctrl, ctrl_reg);

retry:
	ar934x_nfc_wr(nfc, AR934X_NFC_REG_INT_STATUS, 0);
	ar934x_nfc_wr(nfc, AR934X_NFC_REG_ADDR0_0, addr0);
	ar934x_nfc_wr(nfc, AR934X_NFC_REG_ADDR0_1, addr1);
	ar934x_nfc_wr(nfc, AR934X_NFC_REG_DMA_ADDR, nfc->buf_dma);
	ar934x_nfc_wr(nfc, AR934X_NFC_REG_DMA_COUNT, len);
	ar934x_nfc_wr(nfc, AR934X_NFC_REG_DATA_SIZE, len);
	ar934x_nfc_wr(nfc, AR934X_NFC_REG_CTRL, ctrl_reg);
	ar934x_nfc_wr(nfc, AR934X_NFC_REG_DMA_CTRL, dma_ctrl);
	ar934x_nfc_wr(nfc, AR934X_NFC_REG_ECC_CTRL, nfc->ecc_ctrl_reg);
	ar934x_nfc_wr(nfc, AR934X_NFC_REG_ECC_OFFSET, nfc->ecc_offset_reg);

	if (ar934x_nfc_use_irq(nfc)) {
		ar934x_nfc_wr(nfc, AR934X_NFC_REG_INT_MASK, AR934X_NFC_IRQ_MASK);
		/* flush write */
		ar934x_nfc_rr(nfc, AR934X_NFC_REG_INT_MASK);
	}

	ar934x_nfc_write_cmd_reg(nfc, cmd_reg);
	err = ar934x_nfc_wait_done(nfc);
	if (err) {
		dev_dbg(nfc->parent, "%s operation stuck at page %d\n",
			(write) ? "write" : "read", page_addr);

		ar934x_nfc_restart(nfc);
		if (retries++ < AR934X_NFC_DMA_RETRIES)
			goto retry;

		dev_err(nfc->parent, "%s operation failed on page %d\n",
			(write) ? "write" : "read", page_addr);
	}

	return err;
}

static int
ar934x_nfc_send_readid(struct ar934x_nfc *nfc, unsigned command)
{
	u32 cmd_reg;
	int err;

	nfc_dbg(nfc, "readid, cmd:%02x\n", command);

	cmd_reg = AR934X_NFC_CMD_SEQ_1C1AXR;
	cmd_reg |= (command & AR934X_NFC_CMD_CMD0_M) << AR934X_NFC_CMD_CMD0_S;

	err = ar934x_nfc_do_rw_command(nfc, -1, -1, AR934X_NFC_ID_BUF_SIZE,
				       cmd_reg, nfc->ctrl_reg, false);

	nfc_debug_data("[id] ", nfc->buf, AR934X_NFC_ID_BUF_SIZE);

	return err;
}

static int
ar934x_nfc_send_read(struct ar934x_nfc *nfc, unsigned command, int column,
		     int page_addr, int len)
{
	u32 cmd_reg;
	int err;

	nfc_dbg(nfc, "read, column=%d page=%d len=%d\n",
		column, page_addr, len);

	cmd_reg = (command & AR934X_NFC_CMD_CMD0_M) << AR934X_NFC_CMD_CMD0_S;

	if (nfc->small_page) {
		cmd_reg |= AR934X_NFC_CMD_SEQ_18;
	} else {
		cmd_reg |= NAND_CMD_READSTART << AR934X_NFC_CMD_CMD1_S;
		cmd_reg |= AR934X_NFC_CMD_SEQ_1C5A1CXR;
	}

	err = ar934x_nfc_do_rw_command(nfc, column, page_addr, len,
				       cmd_reg, nfc->ctrl_reg, false);

	nfc_debug_data("[data] ", nfc->buf, len);

	return err;
}

static void
ar934x_nfc_send_erase(struct ar934x_nfc *nfc, unsigned command, int column,
		      int page_addr)
{
	u32 addr0, addr1;
	u32 ctrl_reg;
	u32 cmd_reg;

	ar934x_nfc_get_addr(nfc, column, page_addr, &addr0, &addr1);

	ctrl_reg = nfc->ctrl_reg;
	if (nfc->small_page) {
		/* override number of address cycles for the erase command */
		ctrl_reg &= ~(AR934X_NFC_CTRL_ADDR_CYCLE0_M <<
			      AR934X_NFC_CTRL_ADDR_CYCLE0_S);
		ctrl_reg &= ~(AR934X_NFC_CTRL_ADDR_CYCLE1_M <<
			      AR934X_NFC_CTRL_ADDR_CYCLE1_S);
		ctrl_reg &= ~(AR934X_NFC_CTRL_SMALL_PAGE);
		ctrl_reg |= (nfc->addr_count0 + 1) <<
			    AR934X_NFC_CTRL_ADDR_CYCLE0_S;
	}

	cmd_reg = NAND_CMD_ERASE1 << AR934X_NFC_CMD_CMD0_S;
	cmd_reg |= command << AR934X_NFC_CMD_CMD1_S;
	cmd_reg |= AR934X_NFC_CMD_SEQ_ERASE;

	nfc_dbg(nfc, "erase page %d, a0:%08x a1:%08x cmd:%08x ctrl:%08x\n",
		page_addr, addr0, addr1, cmd_reg, ctrl_reg);

	ar934x_nfc_wr(nfc, AR934X_NFC_REG_INT_STATUS, 0);
	ar934x_nfc_wr(nfc, AR934X_NFC_REG_CTRL, ctrl_reg);
	ar934x_nfc_wr(nfc, AR934X_NFC_REG_ADDR0_0, addr0);
	ar934x_nfc_wr(nfc, AR934X_NFC_REG_ADDR0_1, addr1);

	ar934x_nfc_write_cmd_reg(nfc, cmd_reg);
	ar934x_nfc_wait_dev_ready(nfc);
}

static int
ar934x_nfc_send_write(struct ar934x_nfc *nfc, unsigned command, int column,
		     int page_addr, int len)
{
	u32 cmd_reg;

	nfc_dbg(nfc, "write, column=%d page=%d len=%d\n",
		column, page_addr, len);

	nfc_debug_data("[data] ", nfc->buf, len);

	cmd_reg = NAND_CMD_SEQIN << AR934X_NFC_CMD_CMD0_S;
	cmd_reg |= command << AR934X_NFC_CMD_CMD1_S;
	cmd_reg |= AR934X_NFC_CMD_SEQ_12;

	return ar934x_nfc_do_rw_command(nfc, column, page_addr, len,
					cmd_reg, nfc->ctrl_reg, true);
}

static void
ar934x_nfc_read_status(struct ar934x_nfc *nfc)
{
	u32 cmd_reg;
	u32 status;

	cmd_reg = NAND_CMD_STATUS << AR934X_NFC_CMD_CMD0_S;
	cmd_reg |= AR934X_NFC_CMD_SEQ_S;

	ar934x_nfc_wr(nfc, AR934X_NFC_REG_INT_STATUS, 0);
	ar934x_nfc_wr(nfc, AR934X_NFC_REG_CTRL, nfc->ctrl_reg);

	ar934x_nfc_write_cmd_reg(nfc, cmd_reg);
	ar934x_nfc_wait_dev_ready(nfc);

	status = ar934x_nfc_rr(nfc, AR934X_NFC_REG_READ_STATUS);

	nfc_dbg(nfc, "read status, cmd:%08x status:%02x\n",
		cmd_reg, (status & 0xff));

	if (nfc->swap_dma)
		nfc->buf[0 ^ 3] = status;
	else
		nfc->buf[0] = status;
}

static void
ar934x_nfc_cmdfunc(struct mtd_info *mtd, unsigned int command, int column,
		   int page_addr)
{
	struct ar934x_nfc *nfc = mtd_to_ar934x_nfc(mtd);
	struct nand_chip *nand = mtd->priv;

	nfc->read_id = false;
	if (command != NAND_CMD_PAGEPROG)
		nfc->buf_index = 0;

	switch (command) {
	case NAND_CMD_RESET:
		ar934x_nfc_send_cmd(nfc, command);
		break;

	case NAND_CMD_READID:
		nfc->read_id = true;
		ar934x_nfc_send_readid(nfc, command);
		break;

	case NAND_CMD_READ0:
	case NAND_CMD_READ1:
		if (nfc->small_page) {
			ar934x_nfc_send_read(nfc, command, column, page_addr,
					     mtd->writesize + mtd->oobsize);
		} else {
			ar934x_nfc_send_read(nfc, command, 0, page_addr,
					     mtd->writesize + mtd->oobsize);
			nfc->buf_index = column;
			nfc->rndout_page_addr = page_addr;
			nfc->rndout_read_cmd = command;
		}
		break;

	case NAND_CMD_READOOB:
		if (nfc->small_page)
			ar934x_nfc_send_read(nfc, NAND_CMD_READOOB,
					     column, page_addr,
					     mtd->oobsize);
		else
			ar934x_nfc_send_read(nfc, NAND_CMD_READ0,
					     mtd->writesize, page_addr,
					     mtd->oobsize);
		break;

	case NAND_CMD_RNDOUT:
		if (WARN_ON(nfc->small_page))
			break;

		/* emulate subpage read */
		ar934x_nfc_send_read(nfc, nfc->rndout_read_cmd, 0,
				     nfc->rndout_page_addr,
				     mtd->writesize + mtd->oobsize);
		nfc->buf_index = column;
		break;

	case NAND_CMD_ERASE1:
		nfc->erase1_page_addr = page_addr;
		break;

	case NAND_CMD_ERASE2:
		ar934x_nfc_send_erase(nfc, command, -1, nfc->erase1_page_addr);
		break;

	case NAND_CMD_STATUS:
		ar934x_nfc_read_status(nfc);
		break;

	case NAND_CMD_SEQIN:
		if (nfc->small_page) {
			/* output read command */
			if (column >= mtd->writesize) {
				column -= mtd->writesize;
				nfc->seqin_read_cmd = NAND_CMD_READOOB;
			} else if (column < 256) {
				nfc->seqin_read_cmd = NAND_CMD_READ0;
			} else {
				column -= 256;
				nfc->seqin_read_cmd = NAND_CMD_READ1;
			}
		} else {
			nfc->seqin_read_cmd = NAND_CMD_READ0;
		}
		nfc->seqin_column = column;
		nfc->seqin_page_addr = page_addr;
		break;

	case NAND_CMD_PAGEPROG:
		if (nand->ecc.mode == NAND_ECC_HW) {
			/* the data is already written */
			break;
		}

		if (nfc->small_page)
			ar934x_nfc_send_cmd(nfc, nfc->seqin_read_cmd);

		ar934x_nfc_send_write(nfc, command, nfc->seqin_column,
				      nfc->seqin_page_addr,
				      nfc->buf_index);
		break;

	default:
		dev_err(nfc->parent,
			"unsupported command: %x, column:%d page_addr=%d\n",
			command, column, page_addr);
		break;
	}
}

static int
ar934x_nfc_dev_ready(struct mtd_info *mtd)
{
	struct ar934x_nfc *nfc = mtd_to_ar934x_nfc(mtd);

	return __ar934x_nfc_dev_ready(nfc);
}

static void
ar934x_nfc_select_chip(struct mtd_info *mtd, int chip_no)
{
	struct ar934x_nfc *nfc = mtd_to_ar934x_nfc(mtd);

	if (nfc->select_chip)
		nfc->select_chip(chip_no);
}

static u8
ar934x_nfc_read_byte(struct mtd_info *mtd)
{
	struct ar934x_nfc *nfc = mtd_to_ar934x_nfc(mtd);
	u8 data;

	WARN_ON(nfc->buf_index >= nfc->buf_size);

	if (nfc->swap_dma || nfc->read_id)
		data = nfc->buf[nfc->buf_index ^ 3];
	else
		data = nfc->buf[nfc->buf_index];

	nfc->buf_index++;

	return data;
}

static void
ar934x_nfc_write_buf(struct mtd_info *mtd, const u8 *buf, int len)
{
	struct ar934x_nfc *nfc = mtd_to_ar934x_nfc(mtd);
	int i;

	WARN_ON(nfc->buf_index + len > nfc->buf_size);

	if (nfc->swap_dma) {
		for (i = 0; i < len; i++) {
			nfc->buf[nfc->buf_index ^ 3] = buf[i];
			nfc->buf_index++;
		}
	} else {
		for (i = 0; i < len; i++) {
			nfc->buf[nfc->buf_index] = buf[i];
			nfc->buf_index++;
		}
	}
}

static void
ar934x_nfc_read_buf(struct mtd_info *mtd, u8 *buf, int len)
{
	struct ar934x_nfc *nfc = mtd_to_ar934x_nfc(mtd);
	int buf_index;
	int i;

	WARN_ON(nfc->buf_index + len > nfc->buf_size);

	buf_index = nfc->buf_index;

	if (nfc->swap_dma || nfc->read_id) {
		for (i = 0; i < len; i++) {
			buf[i] = nfc->buf[buf_index ^ 3];
			buf_index++;
		}
	} else {
		for (i = 0; i < len; i++) {
			buf[i] = nfc->buf[buf_index];
			buf_index++;
		}
	}

	nfc->buf_index = buf_index;
}

static inline void
ar934x_nfc_enable_hwecc(struct ar934x_nfc *nfc)
{
	nfc->ctrl_reg |= AR934X_NFC_CTRL_ECC_EN;
	nfc->ctrl_reg &= ~AR934X_NFC_CTRL_CUSTOM_SIZE_EN;
}

static inline void
ar934x_nfc_disable_hwecc(struct ar934x_nfc *nfc)
{
	nfc->ctrl_reg &= ~AR934X_NFC_CTRL_ECC_EN;
	nfc->ctrl_reg |= AR934X_NFC_CTRL_CUSTOM_SIZE_EN;
}

static int
ar934x_nfc_read_oob(struct mtd_info *mtd, struct nand_chip *chip,
		    int page)
{
	struct ar934x_nfc *nfc = mtd_to_ar934x_nfc(mtd);
	int err;

	nfc_dbg(nfc, "read_oob: page:%d\n", page);

	err = ar934x_nfc_send_read(nfc, NAND_CMD_READ0, mtd->writesize, page,
				   mtd->oobsize);
	if (err)
		return err;

	memcpy(chip->oob_poi, nfc->buf, mtd->oobsize);

	return 0;
}

static int
ar934x_nfc_write_oob(struct mtd_info *mtd, struct nand_chip *chip,
		     int page)
{
	struct ar934x_nfc *nfc = mtd_to_ar934x_nfc(mtd);

	nfc_dbg(nfc, "write_oob: page:%d\n", page);

	memcpy(nfc->buf, chip->oob_poi, mtd->oobsize);

	return ar934x_nfc_send_write(nfc, NAND_CMD_PAGEPROG, mtd->writesize,
				     page, mtd->oobsize);
}

static int
ar934x_nfc_read_page_raw(struct mtd_info *mtd, struct nand_chip *chip,
			 u8 *buf, int oob_required, int page)
{
	struct ar934x_nfc *nfc = mtd_to_ar934x_nfc(mtd);
	int len;
	int err;

	nfc_dbg(nfc, "read_page_raw: page:%d oob:%d\n", page, oob_required);

	len = mtd->writesize;
	if (oob_required)
		len += mtd->oobsize;

	err = ar934x_nfc_send_read(nfc, NAND_CMD_READ0, 0, page, len);
	if (err)
		return err;

	memcpy(buf, nfc->buf, mtd->writesize);

	if (oob_required)
		memcpy(chip->oob_poi, &nfc->buf[mtd->writesize], mtd->oobsize);

	return 0;
}

static int
ar934x_nfc_read_page(struct mtd_info *mtd, struct nand_chip *chip,
		     u8 *buf, int oob_required, int page)
{
	struct ar934x_nfc *nfc = mtd_to_ar934x_nfc(mtd);
	u32 ecc_ctrl;
	int max_bitflips = 0;
	bool ecc_failed;
	bool ecc_corrected;
	int err;

	nfc_dbg(nfc, "read_page: page:%d oob:%d\n", page, oob_required);

	ar934x_nfc_enable_hwecc(nfc);
	err = ar934x_nfc_send_read(nfc, NAND_CMD_READ0, 0, page,
				   mtd->writesize);
	ar934x_nfc_disable_hwecc(nfc);

	if (err)
		return err;

	/* TODO: optimize to avoid memcpy */
	memcpy(buf, nfc->buf, mtd->writesize);

	/* read the ECC status */
	ecc_ctrl = ar934x_nfc_rr(nfc, AR934X_NFC_REG_ECC_CTRL);
	ecc_failed = ecc_ctrl & AR934X_NFC_ECC_CTRL_ERR_UNCORRECT;
	ecc_corrected = ecc_ctrl & AR934X_NFC_ECC_CTRL_ERR_CORRECT;

	if (oob_required || ecc_failed) {
		err = ar934x_nfc_send_read(nfc, NAND_CMD_READ0, mtd->writesize,
					   page, mtd->oobsize);
		if (err)
			return err;

		if (oob_required)
			memcpy(chip->oob_poi, nfc->buf, mtd->oobsize);
	}

	if (ecc_failed) {
		/*
		 * The hardware ECC engine reports uncorrectable errors
		 * on empty pages. Check the ECC bytes and the data. If
		 * both contains 0xff bytes only, dont report a failure.
		 *
		 * TODO: prebuild a buffer with 0xff bytes and use memcmp
		 * for better performance?
		 */
		if (!is_all_ff(&nfc->buf[nfc->ecc_oob_pos], chip->ecc.total) ||
		    !is_all_ff(buf, mtd->writesize))
				mtd->ecc_stats.failed++;
	} else if (ecc_corrected) {
		/*
		 * The hardware does not report the exact count of the
		 * corrected bitflips, use assumptions based on the
		 * threshold.
		 */
		if (ecc_ctrl & AR934X_NFC_ECC_CTRL_ERR_OVER) {
			/*
			 * The number of corrected bitflips exceeds the
			 * threshold. Assume the maximum.
			 */
			max_bitflips = chip->ecc.strength * chip->ecc.steps;
		} else {
			max_bitflips = nfc->ecc_thres * chip->ecc.steps;
		}

		mtd->ecc_stats.corrected += max_bitflips;
	}

	return max_bitflips;
}

static int
ar934x_nfc_write_page_raw(struct mtd_info *mtd, struct nand_chip *chip,
			  const u8 *buf, int oob_required, int page)
{
	struct ar934x_nfc *nfc = mtd_to_ar934x_nfc(mtd);
	int len;

	nfc_dbg(nfc, "write_page_raw: page:%d oob:%d\n", page, oob_required);

	memcpy(nfc->buf, buf, mtd->writesize);
	len = mtd->writesize;

	if (oob_required) {
		memcpy(&nfc->buf[mtd->writesize], chip->oob_poi, mtd->oobsize);
		len += mtd->oobsize;
	}

	return ar934x_nfc_send_write(nfc, NAND_CMD_PAGEPROG, 0, page, len);
}

static int
ar934x_nfc_write_page(struct mtd_info *mtd, struct nand_chip *chip,
		      const u8 *buf, int oob_required, int page)
{
	struct ar934x_nfc *nfc = mtd_to_ar934x_nfc(mtd);
	int err;

	nfc_dbg(nfc, "write_page: page:%d oob:%d\n", page, oob_required);

	/* write OOB first */
	if (oob_required &&
	    !is_all_ff(chip->oob_poi, mtd->oobsize)) {
		err = ar934x_nfc_write_oob(mtd, chip, page);
		if (err)
			return err;
	}

	/* TODO: optimize to avoid memcopy */
	memcpy(nfc->buf, buf, mtd->writesize);

	ar934x_nfc_enable_hwecc(nfc);
	err = ar934x_nfc_send_write(nfc, NAND_CMD_PAGEPROG, 0, page,
				    mtd->writesize);
	ar934x_nfc_disable_hwecc(nfc);

	return err;
}

static void
ar934x_nfc_hw_init(struct ar934x_nfc *nfc)
{
	struct ar934x_nfc_platform_data *pdata;

	pdata = ar934x_nfc_get_platform_data(nfc);
	if (pdata->hw_reset) {
		pdata->hw_reset(true);
		pdata->hw_reset(false);
	}

	/*
	 * setup timings
	 * TODO: make it configurable via platform data
	 */
	ar934x_nfc_wr(nfc, AR934X_NFC_REG_TIME_SEQ,
		      AR934X_NFC_TIME_SEQ_DEFAULT);
	ar934x_nfc_wr(nfc, AR934X_NFC_REG_TIMINGS_ASYN,
		      AR934X_NFC_TIMINGS_ASYN_DEFAULT);
	ar934x_nfc_wr(nfc, AR934X_NFC_REG_TIMINGS_SYN,
		      AR934X_NFC_TIMINGS_SYN_DEFAULT);

	/* disable WP on all chips, and select chip 0 */
	ar934x_nfc_wr(nfc, AR934X_NFC_REG_MEM_CTRL, 0xff00);

	ar934x_nfc_wr(nfc, AR934X_NFC_REG_DMA_ADDR_OFFS, 0);

	/* initialize Control register */
	nfc->ctrl_reg = AR934X_NFC_CTRL_CUSTOM_SIZE_EN;
	ar934x_nfc_wr(nfc, AR934X_NFC_REG_CTRL, nfc->ctrl_reg);

	if (nfc->small_page) {
		/*  Setup generic sequence register for small page reads. */
		ar934x_nfc_wr(nfc, AR934X_NFC_REG_GEN_SEQ_CTRL,
			      AR934X_NFC_GENSEQ_SMALL_PAGE_READ);
	}
}

static void
ar934x_nfc_restart(struct ar934x_nfc *nfc)
{
	u32 ctrl_reg;

	if (nfc->select_chip)
		nfc->select_chip(-1);

	ctrl_reg = nfc->ctrl_reg;
	ar934x_nfc_hw_init(nfc);
	nfc->ctrl_reg = ctrl_reg;

	if (nfc->select_chip)
		nfc->select_chip(0);

	ar934x_nfc_send_cmd(nfc, NAND_CMD_RESET);
}

static irqreturn_t
ar934x_nfc_irq_handler(int irq, void *data)
{
	struct ar934x_nfc *nfc = data;
	u32 status;

	status = ar934x_nfc_rr(nfc, AR934X_NFC_REG_INT_STATUS);

	ar934x_nfc_wr(nfc, AR934X_NFC_REG_INT_STATUS, 0);
	/* flush write */
	ar934x_nfc_rr(nfc, AR934X_NFC_REG_INT_STATUS);

	status &= ar934x_nfc_rr(nfc, AR934X_NFC_REG_INT_MASK);
	if (status) {
		nfc_dbg(nfc, "got IRQ, status:%08x\n", status);

		nfc->irq_status = status;
		nfc->spurious_irq_expected = true;
		wake_up(&nfc->irq_waitq);
	} else {
		if (nfc->spurious_irq_expected) {
			nfc->spurious_irq_expected = false;
		} else {
			dev_warn(nfc->parent, "spurious interrupt\n");
		}
	}

	return IRQ_HANDLED;
}

static int
ar934x_nfc_init_tail(struct mtd_info *mtd)
{
	struct ar934x_nfc *nfc = mtd_to_ar934x_nfc(mtd);
	struct nand_chip *chip = &nfc->nand_chip;
	u32 ctrl;
	u32 t;
	int err;

	switch (mtd->oobsize) {
	case 16:
	case 64:
	case 128:
		ar934x_nfc_wr(nfc, AR934X_NFC_REG_SPARE_SIZE, mtd->oobsize);
		break;

	default:
		dev_err(nfc->parent, "unsupported OOB size: %d bytes\n",
			mtd->oobsize);
		return -ENXIO;
	}

	ctrl = AR934X_NFC_CTRL_CUSTOM_SIZE_EN;

	switch (mtd->erasesize / mtd->writesize) {
	case 32:
		t = AR934X_NFC_CTRL_BLOCK_SIZE_32;
		break;

	case 64:
		t = AR934X_NFC_CTRL_BLOCK_SIZE_64;
		break;

	case 128:
		t = AR934X_NFC_CTRL_BLOCK_SIZE_128;
		break;

	case 256:
		t = AR934X_NFC_CTRL_BLOCK_SIZE_256;
		break;

	default:
		dev_err(nfc->parent, "unsupported block size: %u\n",
			mtd->erasesize / mtd->writesize);
		return -ENXIO;
	}

	ctrl |= t << AR934X_NFC_CTRL_BLOCK_SIZE_S;

	switch (mtd->writesize) {
	case 256:
		nfc->small_page = 1;
		t = AR934X_NFC_CTRL_PAGE_SIZE_256;
		break;

	case 512:
		nfc->small_page = 1;
		t = AR934X_NFC_CTRL_PAGE_SIZE_512;
		break;

	case 1024:
		t = AR934X_NFC_CTRL_PAGE_SIZE_1024;
		break;

	case 2048:
		t = AR934X_NFC_CTRL_PAGE_SIZE_2048;
		break;

	case 4096:
		t = AR934X_NFC_CTRL_PAGE_SIZE_4096;
		break;

	case 8192:
		t = AR934X_NFC_CTRL_PAGE_SIZE_8192;
		break;

	case 16384:
		t = AR934X_NFC_CTRL_PAGE_SIZE_16384;
		break;

	default:
		dev_err(nfc->parent, "unsupported write size: %d bytes\n",
			mtd->writesize);
		return -ENXIO;
	}

	ctrl |= t << AR934X_NFC_CTRL_PAGE_SIZE_S;

	if (nfc->small_page) {
		ctrl |= AR934X_NFC_CTRL_SMALL_PAGE;

		if (chip->chipsize > (32 << 20)) {
			nfc->addr_count0 = 4;
			nfc->addr_count1 = 3;
		} else if (chip->chipsize > (2 << 16)) {
			nfc->addr_count0 = 3;
			nfc->addr_count1 = 2;
		} else {
			nfc->addr_count0 = 2;
			nfc->addr_count1 = 1;
		}
	} else {
		if (chip->chipsize > (128 << 20)) {
			nfc->addr_count0 = 5;
			nfc->addr_count1 = 3;
		} else if (chip->chipsize > (8 << 16)) {
			nfc->addr_count0 = 4;
			nfc->addr_count1 = 2;
		} else {
			nfc->addr_count0 = 3;
			nfc->addr_count1 = 1;
		}
	}

	ctrl |= nfc->addr_count0 << AR934X_NFC_CTRL_ADDR_CYCLE0_S;
	ctrl |= nfc->addr_count1 << AR934X_NFC_CTRL_ADDR_CYCLE1_S;

	nfc->ctrl_reg = ctrl;
	ar934x_nfc_wr(nfc, AR934X_NFC_REG_CTRL, nfc->ctrl_reg);

	ar934x_nfc_free_buf(nfc);
	err = ar934x_nfc_alloc_buf(nfc, mtd->writesize + mtd->oobsize);

	return err;
}

static struct nand_ecclayout ar934x_nfc_oob_64_hwecc = {
	.eccbytes = 28,
	.eccpos = {
		20, 21, 22, 23, 24, 25, 26,
		27, 28, 29, 30, 31, 32, 33,
		34, 35, 36, 37, 38, 39, 40,
		41, 42, 43, 44, 45, 46, 47,
	},
	.oobfree = {
		{
			.offset = 4,
			.length = 16,
		},
		{
			.offset = 48,
			.length = 16,
		},
	},
};

static int
ar934x_nfc_setup_hwecc(struct ar934x_nfc *nfc)
{
	struct nand_chip *nand = &nfc->nand_chip;
	u32 ecc_cap;
	u32 ecc_thres;

	if (!config_enabled(CONFIG_MTD_NAND_AR934X_HW_ECC)) {
		dev_err(nfc->parent, "hardware ECC support is disabled\n");
		return -EINVAL;
	}

	switch (nfc->mtd.writesize) {
	case 2048:
		/*
		 * Writing a subpage separately is not supported, because
		 * the controller only does ECC on full-page accesses.
		 */
		nand->options = NAND_NO_SUBPAGE_WRITE;

		nand->ecc.size = 512;
		nand->ecc.bytes = 7;
		nand->ecc.strength = 4;
		nand->ecc.layout = &ar934x_nfc_oob_64_hwecc;
		break;

	default:
		dev_err(nfc->parent,
			"hardware ECC is not available for %d byte pages\n",
			nfc->mtd.writesize);
		return -EINVAL;
	}

	BUG_ON(!nand->ecc.layout);

	switch (nand->ecc.strength) {
	case 4:
		ecc_cap = AR934X_NFC_ECC_CTRL_ECC_CAP_4;
		ecc_thres = 4;
		break;

	default:
		dev_err(nfc->parent, "unsupported ECC strength %u\n",
			nand->ecc.strength);
		return -EINVAL;
	}

	nfc->ecc_thres = ecc_thres;
	nfc->ecc_oob_pos = nand->ecc.layout->eccpos[0];

	nfc->ecc_ctrl_reg = ecc_cap << AR934X_NFC_ECC_CTRL_ECC_CAP_S;
	nfc->ecc_ctrl_reg |= ecc_thres << AR934X_NFC_ECC_CTRL_ERR_THRES_S;

	nfc->ecc_offset_reg = nfc->mtd.writesize + nfc->ecc_oob_pos;

	nand->ecc.mode = NAND_ECC_HW;
	nand->ecc.read_page = ar934x_nfc_read_page;
	nand->ecc.read_page_raw = ar934x_nfc_read_page_raw;
	nand->ecc.write_page = ar934x_nfc_write_page;
	nand->ecc.write_page_raw = ar934x_nfc_write_page_raw;
	nand->ecc.read_oob = ar934x_nfc_read_oob;
	nand->ecc.write_oob = ar934x_nfc_write_oob;

	return 0;
}

static int
ar934x_nfc_probe(struct platform_device *pdev)
{
	static const char *part_probes[] = { "cmdlinepart", NULL, };
	struct ar934x_nfc_platform_data *pdata;
	struct ar934x_nfc *nfc;
	struct resource *res;
	struct mtd_info *mtd;
	struct nand_chip *nand;
	struct mtd_part_parser_data ppdata;
	int ret;

	pdata = pdev->dev.platform_data;
	if (pdata == NULL) {
		dev_err(&pdev->dev, "no platform data defined\n");
		return -EINVAL;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(&pdev->dev, "failed to get I/O memory\n");
		return -EINVAL;
	}

	nfc = devm_kzalloc(&pdev->dev, sizeof(struct ar934x_nfc), GFP_KERNEL);
	if (!nfc) {
		dev_err(&pdev->dev, "failed to allocate driver data\n");
		return -ENOMEM;
	}

	nfc->base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(nfc->base)) {
		dev_err(&pdev->dev, "failed to remap I/O memory\n");
		return PTR_ERR(nfc->base);
	}

	nfc->irq = platform_get_irq(pdev, 0);
	if (nfc->irq < 0) {
		dev_err(&pdev->dev, "no IRQ resource specified\n");
		return -EINVAL;
	}

	init_waitqueue_head(&nfc->irq_waitq);
	ret = request_irq(nfc->irq, ar934x_nfc_irq_handler, 0,
			  dev_name(&pdev->dev), nfc);
	if (ret) {
		dev_err(&pdev->dev, "requast_irq failed, err:%d\n", ret);
		return ret;
	}

	nfc->parent = &pdev->dev;
	nfc->select_chip = pdata->select_chip;
	nfc->swap_dma = pdata->swap_dma;

	nand = &nfc->nand_chip;
	mtd = &nfc->mtd;

	mtd->priv = nand;
	mtd->owner = THIS_MODULE;
	if (pdata->name)
		mtd->name = pdata->name;
	else
		mtd->name = dev_name(&pdev->dev);

	nand->chip_delay = 25;

	nand->dev_ready = ar934x_nfc_dev_ready;
	nand->cmdfunc = ar934x_nfc_cmdfunc;
	nand->read_byte = ar934x_nfc_read_byte;
	nand->write_buf = ar934x_nfc_write_buf;
	nand->read_buf = ar934x_nfc_read_buf;
	nand->select_chip = ar934x_nfc_select_chip;

	ret = ar934x_nfc_alloc_buf(nfc, AR934X_NFC_ID_BUF_SIZE);
	if (ret)
		goto err_free_irq;

	platform_set_drvdata(pdev, nfc);

	ar934x_nfc_hw_init(nfc);

	ret = nand_scan_ident(mtd, 1, NULL);
	if (ret) {
		dev_err(&pdev->dev, "nand_scan_ident failed, err:%d\n", ret);
		goto err_free_buf;
	}

	ret = ar934x_nfc_init_tail(mtd);
	if (ret) {
		dev_err(&pdev->dev, "init tail failed, err:%d\n", ret);
		goto err_free_buf;
	}

	if (pdata->scan_fixup) {
		ret = pdata->scan_fixup(mtd);
		if (ret)
			goto err_free_buf;
	}

	switch (pdata->ecc_mode) {
	case AR934X_NFC_ECC_SOFT:
		nand->ecc.mode = NAND_ECC_SOFT;
		break;

	case AR934X_NFC_ECC_SOFT_BCH:
		nand->ecc.mode = NAND_ECC_SOFT_BCH;
		break;

	case AR934X_NFC_ECC_HW:
		ret = ar934x_nfc_setup_hwecc(nfc);
		if (ret)
			goto err_free_buf;

		break;

	default:
		dev_err(nfc->parent, "unknown ECC mode %d\n", pdata->ecc_mode);
		return -EINVAL;
	}

	ret = nand_scan_tail(mtd);
	if (ret) {
		dev_err(&pdev->dev, "scan tail failed, err:%d\n", ret);
		goto err_free_buf;
	}

	memset(&ppdata, '\0', sizeof(ppdata));
	ret = mtd_device_parse_register(mtd, part_probes, &ppdata,
					pdata->parts, pdata->nr_parts);
	if (ret) {
		dev_err(&pdev->dev, "unable to register mtd, err:%d\n", ret);
		goto err_free_buf;
	}

	return 0;

err_free_buf:
	ar934x_nfc_free_buf(nfc);
err_free_irq:
	free_irq(nfc->irq, nfc);
	return ret;
}

static int
ar934x_nfc_remove(struct platform_device *pdev)
{
	struct ar934x_nfc *nfc;

	nfc = platform_get_drvdata(pdev);
	if (nfc) {
		nand_release(&nfc->mtd);
		ar934x_nfc_free_buf(nfc);
		free_irq(nfc->irq, nfc);
	}

	return 0;
}

static struct platform_driver ar934x_nfc_driver = {
	.probe		= ar934x_nfc_probe,
	.remove		= ar934x_nfc_remove,
	.driver = {
		.name	= AR934X_NFC_DRIVER_NAME,
		.owner	= THIS_MODULE,
	},
};

module_platform_driver(ar934x_nfc_driver);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Gabor Juhos <juhosg@openwrt.org>");
MODULE_DESCRIPTION("Atheros AR934x NAND Flash Controller driver");
MODULE_ALIAS("platform:" AR934X_NFC_DRIVER_NAME);
