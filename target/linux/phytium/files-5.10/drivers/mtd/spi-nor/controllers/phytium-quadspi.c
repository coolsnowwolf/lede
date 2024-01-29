/*
 * Phytium SPI core controller driver.
 *
 * Copyright (c) 2019-2023 Phytium Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 */
#include <linux/clk.h>
#include <linux/errno.h>
#include <linux/io.h>
#include <linux/iopoll.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/mtd/spi-nor.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/reset.h>
#include <linux/sizes.h>
#include <linux/spinlock.h>
#include <linux/swab.h>
#include <linux/acpi.h>

#define QSPI_FLASH_CAP_REG   0x000
#define QSPI_RD_CFG_REG      0x004
#define QSPI_WR_CFG_REG      0x008
#define QSPI_FLUSH_REG       0x00C
#define QSPI_CMD_PORT_REG    0x010
#define QSPI_ADDR_PORT_REG   0x014
#define QSPI_HD_PORT_REG     0x018
#define QSPI_LD_PORT_REG     0x01C
#define QSPI_FUN_SET_REG     0x020
#define QSPI_WIP_REG         0x024
#define QSPI_WP_REG          0x028
#define QSPI_MODE_REG        0x02C

#define QSPI_FLASH_CAP_NUM_SHIFT	3
#define QSPI_FLASH_CAP_NUM_MASK		(0x3 << QSPI_FLASH_CAP_NUM_SHIFT)
#define QSPI_FLASH_CAP_CAP_SHIFT	0
#define QSPI_FLASH_CAP_CAP_MASK		(0x7 << QSPI_FLASH_CAP_CAP_SHIFT)

#define QSPI_RD_CFG_RD_CMD_SHIFT	24
#define QSPI_RD_CFG_RD_CMD_MASK		(0xFF << QSPI_RD_CFG_RD_CMD_SHIFT)
#define QSPI_RD_CFG_RD_THROUGH_SHIFT	23
#define QSPI_RD_CFG_RD_THROUGH_MASK	(0x01 << QSPI_RD_CFG_RD_THROUGH_SHIFT)
#define QSPI_RD_CFG_RD_TRANSFER_SHIFT	20
#define QSPI_RD_CFG_RD_TRANSFER_MASK	(0x07 << QSPI_RD_CFG_RD_TRANSFER_SHIFT)
#define QSPI_RD_CFG_RD_ADDR_SEL_SHIFT	19
#define QSPI_RD_CFG_RD_ADDR_SEL_MASK	(0x1 << QSPI_RD_CFG_RD_ADDR_SEL_SHIFT)
#define QSPI_RD_CFG_RD_LATENCY_SHIFT	18
#define QSPI_RD_CFG_RD_LATENCY_MASK	(0x1 << QSPI_RD_CFG_RD_LATENCY_SHIFT)
#define QSPI_RD_CFG_MODE_BYTE_SHIFT	17
#define QSPI_RD_CFG_MODE_BYTE_MASK	(0x1 << QSPI_RD_CFG_MODE_BYTE_SHIFT)
#define QSPI_RD_CFG_CMD_SIGN_SHIFT	9
#define QSPI_RD_CFG_CMD_SIGN_MASK	(0xFF << QSPI_RD_CFG_CMD_SIGN_SHIFT)
#define QSPI_RD_CFG_DUMMY_SHIFT		4
#define QSPI_RD_CFG_DUMMY_MASK		(0x1F << QSPI_RD_CFG_DUMMY_SHIFT)
#define QSPI_RD_CFG_D_BUFFER_SHIFT	3
#define QSPI_RD_CFG_D_BUFFER_MASK	(0x1 << QSPI_RD_CFG_D_BUFFER_SHIFT)
#define QSPI_RD_CFG_RD_SCK_SEL_SHIFT	0
#define QSPI_RD_CFG_RD_SCK_SEL_MASK	(0x3 << QSPI_RD_CFG_RD_SCK_SEL_SHIFT)

#define QSPI_WR_CFG_WR_CMD_SHIFT	24
#define QSPI_WR_CFG_WR_CMD_MASK		(0xFF << QSPI_WR_CFG_WR_CMD_SHIFT)
#define QSPI_WR_CFG_WR_WAIT_SHIFT	9
#define QSPI_WR_CFG_WR_WAIT_MASK	(0x01 << QSPI_WR_CFG_WR_WAIT_SHIFT)
#define QSPI_WR_CFG_WR_THROUGH_SHIFT	8
#define QSPI_WR_CFG_WR_THROUGH_MAS	(0x01 << QSPI_WR_CFG_WR_THROUGH_SHIFT)
#define QSPI_WR_CFG_WR_TRANSFER_SHIFT	5
#define QSPI_WR_CFG_WR_TRANSFER_MASK	(0X7 << QSPI_WR_CFG_WR_TRANSFER_SHIFT)
#define QSPI_WR_CFG_WR_ADDR_SEL_SHIFT	4
#define QSPI_WR_CFG_WR_ADDR_SEL_MASK	(0x1 << QSPI_WR_CFG_WR_ADDR_SEL_SHIFT)
#define QSPI_WR_CFG_WR_MODE_SHIFT	3
#define QSPI_WR_CFG_WR_MODE		(0x01 << QSPI_WR_CFG_WR_MODE_SHIFT)
#define QSPI_WR_CFG_WR_SCK_SEL_SHIFT	0
#define QSPI_WR_CFG_WR_SCK_SEL_MASK	(0x7 << QSPI_WR_CFG_WR_SCK_SEL_SHIFT)

#define QSPI_FLUSH_EN			(0x1 << 0)

#define QSPI_CMD_PORT_CMD_SHIFT		24
#define QSPI_CMD_PORT_CMD_MASK		(0xFF << QSPI_CMD_PORT_CMD_SHIFT)
#define QSPI_CMD_PORT_WAIT_SHIFT	22
#define QSPI_CMD_PORT_WAIT_MASK		(0x1 << QSPI_CMD_PORT_WAIT_SHIFT)
#define QSPI_CMD_PORT_THROUGH_SHIFT	21
#define QSPI_CMD_PORT_THROUGH_MASK	(0x1 << QSPI_CMD_PORT_THROUGH_SHIFT)
#define QSPI_CMD_PORT_CS_SHIFT		19
#define QSPI_CMD_PORT_CS_MASK		(0x3 << QSPI_CMD_PORT_CS_SHIFT)
#define QSPI_CMD_PORT_TRANSFER_SHIFT	16
#define QSPI_CMD_PORT_TRANSFER_MASK	(0x7 << QSPI_CMD_PORT_TRANSFER_SHIFT)
#define QSPI_CMD_PORT_CMD_ADDR_SHIFT	15
#define QSPI_CMD_PORT_CMD_ADDR_MASK	(0x1 << QSPI_CMD_PORT_CMD_ADDR_SHIFT)
#define QSPI_CMD_PORT_LATENCY_SHIFT	14
#define QSPI_CMD_PORT_LATENCY_MASK	(0x1 << QSPI_CMD_PORT_LATENCY_SHIFT)
#define QSPI_CMD_PORT_DATA_TRANSFER_SHIFT	13
#define QSPI_CMD_PORT_DATA_TRANSFER_MASK	(0x1 << 13)
#define QSPI_CMD_PORT_SEL_SHIFT		12
#define QSPI_CMD_PORT_SEL_MASK		(0x1 << QSPI_CMD_PORT_SEL_SHIFT)
#define QSPI_CMD_PORT_DUMMY_SHIFT	7
#define QSPI_CMD_PORT_DUMMY_MASK	(0x1F << QSPI_CMD_PORT_DUMMY_SHIFT)
#define QSPI_CMD_PORT_DUMMY(x)		(((x) << QSPI_CMD_PORT_DUMMY_SHIFT) & QSPI_CMD_PORT_DUMMY_MASK)
#define QSPI_CMD_PORT_P_BUFFER_SHIFT	6
#define QSPI_CMD_PORT_P_BUFFER_MASK	(0x1 << QSPI_CMD_PORT_P_BUFFER_SHIFT)
#define QSPI_CMD_PORT_RW_NUM_SHIFT	3
#define QSPI_CMD_PORT_RW_NUM_MASK	(0x7 << QSPI_CMD_PORT_RW_NUM_SHIFT)
#define QSPI_CMD_PORT_SCK_SEL_SHIFT	0
#define QSPI_CMD_PORT_SCK_SEL_MASK	(0x7 << QSPI_CMD_PORT_SCK_SEL_SHIFT)

#define QSPI_FUN_SET_HOLD_SHIFT		24
#define QSPI_FUN_SET_HOLD_MASK		(0xFF << QSPI_FUN_SET_HOLD_SHIFT)
#define QSPI_FUN_SET_SETUP_SHIFT	16
#define QSPI_FUN_SET_SETUP_MASK		(0xFF << QSPI_FUN_SET_SETUP_SHIFT)
#define QSPI_FUN_SET_DELAY_SHIFT	0
#define QSPI_FUN_SET_DELAY_MASK		(0xFFFF << QSPI_FUN_SET_DELAY_SHIFT)

#define QSPI_WIP_W_CMD_SHIFT		24
#define QSPI_WIP_W_CMD_MASK		(0xFF << QSPI_WIP_W_CMD_SHIFT)
#define QSPI_WIP_W_TRANSFER_SHIFT	3
#define QSPI_WIP_W_TRANSFER_MASK	(0x3 << QSPI_WIP_W_TRANSFER_SHIFT)
#define QSPI_WIP_W_SCK_SEL_SHIFT      	0
#define QSPI_WIP_W_SCK_SEL_MASK		(0x7 << QSPI_WIP_W_SCK_SEL_SHIFT)

#define QSPI_WP_EN_SHIFT		17
#define QSPI_WP_EN_MASK			(0x1 << QSPI_WP_EN_SHIFT)
#define QSPI_WP_IO2_SHIFT		16
#define QSPI_WP_IO2_MASK		(0x1 << QSPI_WP_IO2_SHIFT)
#define QSPI_WP_HOLD_SHIFT		8
#define QSPI_WP_HOLD_MASK		(0xFF << QSPI_WP_HOLD_SHIFT)
#define QSPI_WP_SETUP_SHIFT		0
#define QSPI_WP_SETUP_MASK		(0xFF << QSPI_WP_SETUP_SHIFT)

#define QSPI_MODE_VALID_SHIFT		8
#define QSPI_MODE_VALID_MASK		(0xFF << QSPI_MODE_VALID_SHIFT)
#define QSPI_MODE_SHIFT			0
#define QSPI_MODE_MASK			(0xFF << QSPI_MODE_SHIFT)

#define FSIZE_VAL(size)			(__fls(size) - 1)

#define PHYTIUM_MAX_MMAP_S		SZ_512M
#define PHYTIUM_MAX_NORCHIP		4

#define PHYTIUM_QSPI_FIFO_SZ		32
#define PHYTIUM_QSPI_FIFO_TIMEOUT_US	50000
#define PHYTIUM_QSPI_BUSY_TIMEOUT_US	100000

#define PHYTIUM_SCK_SEL			0x05
#define PHYTIUM_CMD_SCK_SEL		0x07

#define PHYTIUM_FMODE_MM		0x01
#define PHYTIUM_FMODE_IN		0x02

/*
 * the codes of the different commands
 */
#define CMD_WRDI	  0x04
#define CMD_RDID	  0x9F
#define CMD_RDSR	  0x05
#define CMD_WREN	  0x06
#define CMD_RDAR	  0x65
#define CMD_P4E           0x20
#define CMD_4P4E          0x21
#define CMD_BE      	  0x60
#define CMD_4BE           0xC7
#define	CMD_READ          0x03
#define	CMD_FAST_READ     0x0B
#define	CMD_QOR           0x6B
#define	CMD_QIOR          0xEB
#define CMD_DDRFR         0x0D
#define	CMD_DDRQIOQ       0xED
#define	CMD_PP            0x02
#define	CMD_QPP           0x32
#define	CMD_SE            0xD8
#define	CMD_4FAST_READ    0x0C
#define	CMD_4READ         0x13
#define	CMD_4QOR          0x6C
#define	CMD_4QIOR         0xEC
#define	CMD_4DDRFR        0x0E
#define	CMD_4DDRQIOR      0xEE
#define	CMD_4PP           0x12
#define	CMD_4QPP          0x34
#define	CMD_4SE           0xDC

#define PHYTIUM_QSPI_1_1_1	0
#define PHYTIUM_QSPI_1_1_2	1
#define PHYTIUM_QSPI_1_1_4	2
#define PHYTIUM_QSPI_1_2_2	3
#define PHYTIUM_QSPI_1_4_4	4
#define PHYTIUM_QSPI_2_2_2	5
#define PHYTIUM_QSPI_4_4_4	6

struct phytium_qspi_flash {
	struct spi_nor nor;
	struct phytium_qspi *qspi;
	u32 cs;
	u32 fsize;
	u32 presc;
	u32 clk_div;
	u32 read_mode;
	bool registered;
	u32 prefetch_limit;
	u32 addr_width;
	u32 read_cmd;
};

struct phytium_qspi {
	struct device *dev;
	void __iomem *io_base;
	void __iomem *mm_base;
	resource_size_t mm_size;
	u32 nor_num;
	struct clk *clk;
	u32 clk_rate;
	struct phytium_qspi_flash flash[PHYTIUM_MAX_NORCHIP];

	spinlock_t spinlock;

	/*
	 * to protect device configuration, could be different between
	 * 2 flash access (bk1, bk2)
	 */
	struct mutex lock;
};

/* Need to enable p_buffer */
static int memcpy_from_ftreg(struct phytium_qspi *qspi, u_char *buf, size_t len)
{
	int i;
	u32 val = 0;

	if (!qspi || !buf)
		return -EINVAL;

	for (i = 0; i < len; i++) {
		if (0 == i % 4)
			val = readl_relaxed(qspi->io_base + QSPI_LD_PORT_REG);

		buf[i] = (u_char) (val >> (i % 4) * 8) & 0xFF;
	}

	return 0;
}

/* Not to enable p_buffer */
static int memcpy_to_ftreg(struct phytium_qspi *qspi, const u8 *buf, size_t len)
{
	u32 val = 0;

	if (!qspi || !buf || (len >= 8))
		return -EINVAL;

	if (1 == len) {
		val = buf[0];
	} else if (2 == len) {
		val = buf[1];
		val = (val << 8) + buf[0];
	} else if (3 == len) {
		val = buf[2];
		val = (val << 8) + buf[1];
		val = (val << 8) + buf[0];
	} else if (4 == len) {
		val = buf[3];
		val = (val << 8) + buf[2];
		val = (val << 8) + buf[1];
		val = (val << 8) + buf[0];
	}

	writel_relaxed(val, qspi->io_base + QSPI_LD_PORT_REG);

	return 0;
}

static int phytium_qspi_wait_cmd(struct phytium_qspi *qspi,
				struct phytium_qspi_flash *flash)
{
	u32 cmd = 0;
	u32 cnt = 0;

	cmd |= CMD_RDSR << QSPI_CMD_PORT_CMD_SHIFT;
	cmd |= BIT(QSPI_CMD_PORT_DATA_TRANSFER_SHIFT);
	cmd |= flash->cs << QSPI_CMD_PORT_CS_SHIFT;

	writel_relaxed(cmd, qspi->io_base + QSPI_CMD_PORT_REG);

	cnt = PHYTIUM_QSPI_BUSY_TIMEOUT_US / 10;
	while (readl_relaxed(qspi->io_base + QSPI_LD_PORT_REG) & 0x01) {
		udelay(10);
		cnt--;
		if (!cnt) {
			dev_err(qspi->dev, "wait command process timeout\n");
			break;
		}
	}

	return !cnt;
}

static int phytium_qspi_cmd_enable(struct phytium_qspi *qspi)
{
	u32 val = 0;

	writel_relaxed(val, qspi->io_base + QSPI_LD_PORT_REG);

	return 0;
}

static int phytium_qspi_write_enable(struct phytium_qspi *qspi,
				struct phytium_qspi_flash *flash)
{
	u32 cmd = 0;

	cmd  = CMD_WREN << QSPI_CMD_PORT_CMD_SHIFT;
	cmd |= PHYTIUM_SCK_SEL << QSPI_CMD_PORT_SCK_SEL_SHIFT;
	cmd |= flash->cs << QSPI_CMD_PORT_CS_SHIFT;

	writel_relaxed(cmd, qspi->io_base + QSPI_CMD_PORT_REG);
	phytium_qspi_cmd_enable(qspi);

	return 0;
}

static int phytium_qspi_write_disable(struct phytium_qspi *qspi,
				struct phytium_qspi_flash *flash)
{
	u32 cmd = 0;

	cmd  = CMD_WRDI << QSPI_CMD_PORT_CMD_SHIFT;
	cmd |= PHYTIUM_SCK_SEL << QSPI_CMD_PORT_SCK_SEL_SHIFT;
	cmd |= flash->cs << QSPI_CMD_PORT_CS_SHIFT;

	writel_relaxed(cmd, qspi->io_base + QSPI_CMD_PORT_REG);
	phytium_qspi_cmd_enable(qspi);

	return 0;
}

static int phytium_qspi_read_flash_id(struct phytium_qspi *qspi,
		struct phytium_qspi_flash *flash, u8 opcode, u8 *buf, int len)
{
	u32 cmd = 0;
	unsigned long iflags;

	cmd  = opcode << QSPI_CMD_PORT_CMD_SHIFT;
	cmd |= BIT(QSPI_CMD_PORT_DATA_TRANSFER_SHIFT);
	cmd |= BIT(QSPI_CMD_PORT_P_BUFFER_SHIFT);
	cmd |= PHYTIUM_CMD_SCK_SEL << QSPI_CMD_PORT_SCK_SEL_SHIFT;
	cmd |= flash->cs << QSPI_CMD_PORT_CS_SHIFT;

	writel_relaxed(cmd, qspi->io_base + QSPI_CMD_PORT_REG);
	phytium_qspi_cmd_enable(qspi);

	spin_lock_irqsave(&qspi->spinlock, iflags);
	memcpy_from_ftreg(qspi, buf, len);
	spin_unlock_irqrestore(&qspi->spinlock, iflags);

	dev_dbg(qspi->dev, "read flash id:%x\n", *(u32 *)buf);
	return 0;
}

static int phytium_qspi_read_flash_sfdp(struct phytium_qspi *qspi,
	struct phytium_qspi_flash *flash, struct spi_nor *nor, loff_t from, u8 *buf, int len)
{
	unsigned long iflags;
	u32 cmd = 0;
	u8 opcode = nor->read_opcode;

	cmd  = opcode << QSPI_CMD_PORT_CMD_SHIFT;
	cmd |= BIT(QSPI_CMD_PORT_DATA_TRANSFER_SHIFT);
	cmd |= BIT(QSPI_CMD_PORT_P_BUFFER_SHIFT);
	cmd |= BIT(QSPI_CMD_PORT_CMD_ADDR_SHIFT);
	cmd |= PHYTIUM_SCK_SEL << QSPI_CMD_PORT_SCK_SEL_SHIFT;
	cmd |= flash->cs << QSPI_CMD_PORT_CS_SHIFT;
	cmd |= BIT(QSPI_CMD_PORT_LATENCY_SHIFT);
	cmd |= QSPI_CMD_PORT_DUMMY(nor->read_dummy - 1);

	writel_relaxed(cmd, qspi->io_base + QSPI_CMD_PORT_REG);
	writel_relaxed(from, qspi->io_base + QSPI_ADDR_PORT_REG);
	phytium_qspi_cmd_enable(qspi);

	spin_lock_irqsave(&qspi->spinlock, iflags);
	memcpy_from_ftreg(qspi, buf, len);
	spin_unlock_irqrestore(&qspi->spinlock, iflags);

	dev_dbg(qspi->dev, "read flash sfdp:0x%llx 0x%llx\n",
			*(u64 *)buf, *(u64 *)(buf + 8));
	return len;
}

static int phytium_qspi_read_flash_sr1(struct phytium_qspi *qspi,
		struct phytium_qspi_flash *flash,  u8 opcode, u8 *buf, int len)
{
	u32 cmd = 0;
	u32 val;

	cmd  = opcode << QSPI_CMD_PORT_CMD_SHIFT;
	cmd |= BIT(QSPI_CMD_PORT_DATA_TRANSFER_SHIFT);
	cmd |= (len << QSPI_CMD_PORT_RW_NUM_SHIFT) & QSPI_CMD_PORT_RW_NUM_MASK;
	cmd |= PHYTIUM_CMD_SCK_SEL << QSPI_CMD_PORT_SCK_SEL_SHIFT;
	cmd |= flash->cs << QSPI_CMD_PORT_CS_SHIFT;

	writel_relaxed(cmd, qspi->io_base + QSPI_CMD_PORT_REG);
	phytium_qspi_cmd_enable(qspi);

	val = readl_relaxed(qspi->io_base + QSPI_LD_PORT_REG);
	buf[0] = (u8)val;

	return 0;
}

static int phytium_qspi_read_reg(struct spi_nor *nor, u8 opcode, u8 *buf, size_t len)
{
	struct phytium_qspi_flash *flash = nor->priv;
	struct device *dev = flash->qspi->dev;
	struct phytium_qspi *qspi = flash->qspi;
	unsigned long iflags;
	u32 cmd = 0;

	dev_dbg(dev, "read_reg: cmd:%#.2x buf:%pK len:%#lx\n", opcode, buf, len);

	switch (opcode) {
	case CMD_RDID:
		phytium_qspi_read_flash_id(qspi, flash, opcode, buf, len);
		return 0;
	case CMD_RDSR:
		phytium_qspi_read_flash_sr1(qspi, flash, opcode, buf, len);
		return 0;
	default:
		break;
	}

	cmd = opcode << QSPI_CMD_PORT_CMD_SHIFT;
	cmd |= BIT(QSPI_CMD_PORT_DATA_TRANSFER_SHIFT);
	cmd |= BIT(QSPI_CMD_PORT_P_BUFFER_SHIFT);
	cmd |= PHYTIUM_CMD_SCK_SEL << QSPI_CMD_PORT_SCK_SEL_SHIFT;
	cmd |= flash->cs << QSPI_CMD_PORT_CS_SHIFT;

	writel_relaxed(cmd, qspi->io_base + QSPI_CMD_PORT_REG);
	phytium_qspi_cmd_enable(qspi);

	spin_lock_irqsave(&qspi->spinlock, iflags);
	memcpy_from_ftreg(qspi, buf, len);
	spin_unlock_irqrestore(&qspi->spinlock, iflags);

	return 0;
}

static int phytium_qspi_write_reg(struct spi_nor *nor, u8 opcode, const u8 *buf, size_t len)
{
	struct phytium_qspi_flash *flash = nor->priv;
	struct device *dev = flash->qspi->dev;
	struct phytium_qspi *qspi = flash->qspi;
	u32 cmd = 0;

	dev_dbg(dev, "write_reg: cmd:%#.2x buf:%pK len:%#lx\n", opcode, buf, len);

	switch (opcode) {
	case CMD_WREN:
		phytium_qspi_write_enable(qspi, flash);
		return 0;
	case CMD_WRDI:
		phytium_qspi_write_disable(qspi, flash);
		return 0;
	default:
		break;
	}

	cmd = opcode << QSPI_CMD_PORT_CMD_SHIFT;
	cmd |= PHYTIUM_CMD_SCK_SEL << QSPI_CMD_PORT_SCK_SEL_SHIFT;
	cmd |= flash->cs << QSPI_CMD_PORT_CS_SHIFT;

	if ((len > 8) || (NULL == buf)) {
		dev_err(dev, "data length exceed. commad %x, len:%ld \n", opcode, len);
		return -EINVAL;
	} else if (len > 0) {
		cmd |= ((len - 1) << QSPI_CMD_PORT_RW_NUM_SHIFT) & QSPI_CMD_PORT_RW_NUM_MASK;
		cmd |= BIT(QSPI_CMD_PORT_DATA_TRANSFER_SHIFT);
	}

	writel_relaxed(cmd, qspi->io_base + QSPI_CMD_PORT_REG);
	memcpy_to_ftreg(qspi, buf, len);

	return 0;
}

static ssize_t phytium_qspi_read_tmp(struct phytium_qspi *qspi, u32 read_cmd, loff_t from, size_t len, u_char *buf)
{
	u32 addr = (u32)from;
	u64 val = 0;

	if (!qspi)
		return -1;

	dev_dbg(qspi->dev, "read cmd:%x, addr:%x len:%zx\n", read_cmd, addr, len);
	writel_relaxed(read_cmd, qspi->io_base + QSPI_RD_CFG_REG);

	memcpy_fromio(buf, qspi->mm_base + addr, len);

	val = *(u64 *)(buf);
	dev_dbg(qspi->dev, "read val:%llx\n", val);

	return len;
}

static ssize_t phytium_qspi_read(struct spi_nor *nor, loff_t from, size_t len, u8 *buf)
{
	struct phytium_qspi_flash *flash = nor->priv;
	struct phytium_qspi *qspi = flash->qspi;
	u32 cmd = nor->read_opcode;
	u32 addr = (u32)from;

	addr = addr + flash->cs * flash->fsize;

	dev_dbg(qspi->dev, "read(%#.2x): buf:%pK from:%#.8x len:%#zx\n", nor->read_opcode, buf, addr, len);

	cmd = cmd << QSPI_RD_CFG_RD_CMD_SHIFT;
	cmd |= BIT(QSPI_RD_CFG_D_BUFFER_SHIFT);
	cmd |= flash->clk_div << QSPI_CMD_PORT_SCK_SEL_SHIFT;

	cmd &= ~QSPI_RD_CFG_RD_TRANSFER_MASK;
	cmd |= (flash->addr_width << QSPI_RD_CFG_RD_TRANSFER_SHIFT);

	switch (nor->read_opcode) {
	case CMD_READ:
	case CMD_FAST_READ:
	case CMD_QIOR:
	case CMD_QOR:
		cmd &= ~QSPI_RD_CFG_RD_ADDR_SEL_MASK;
		break;
	case CMD_4READ:
	case CMD_4FAST_READ:
	case CMD_4QOR:
	case CMD_4QIOR:
		cmd |= BIT(QSPI_RD_CFG_RD_ADDR_SEL_SHIFT);
		break;
	case 0x5A:
		cmd &= ~QSPI_RD_CFG_RD_ADDR_SEL_MASK;
		return phytium_qspi_read_flash_sfdp(qspi, flash, nor, from, buf, len);
		break;
	default:
		break;
	}

	if ((PHYTIUM_QSPI_1_1_4 == flash->addr_width) || (PHYTIUM_QSPI_1_4_4 == flash->addr_width)) {
		cmd |= BIT(QSPI_RD_CFG_RD_LATENCY_SHIFT);
		cmd &= ~QSPI_RD_CFG_DUMMY_MASK;
		cmd |= (0x07 << QSPI_RD_CFG_DUMMY_SHIFT);
	}

	dev_dbg(qspi->dev, "read(%#.2x): cmd:%#x\n", nor->read_opcode, cmd);

	if (cmd != flash->read_cmd)
		flash->read_cmd = cmd;

	writel_relaxed(cmd, qspi->io_base + QSPI_RD_CFG_REG);

	memcpy_fromio(buf, qspi->mm_base + addr, len);

	return len;
}

static ssize_t phytium_qspi_write(struct spi_nor *nor, loff_t to, size_t len, const u8 *buf)
{
	struct phytium_qspi_flash *flash = nor->priv;
	struct device *dev = flash->qspi->dev;
	struct phytium_qspi *qspi = flash->qspi;
	u32 cmd = nor->program_opcode;
	u32 addr = (u32)to;
	int i;
	u_char tmp[8] = {0};
	size_t mask = 0x03;

	addr = addr + flash->cs * flash->fsize;

	dev_dbg(dev, "write(%#.2x): buf:%p to:%#.8x len:%#zx\n", nor->program_opcode, buf, addr, len);

	if (addr & 0x03) {
		dev_err(dev, "Addr not four-byte aligned!\n");
		return -EINVAL;
	}

	cmd = cmd << QSPI_WR_CFG_WR_CMD_SHIFT;
	cmd |= BIT(QSPI_WR_CFG_WR_MODE_SHIFT);
	cmd |= PHYTIUM_CMD_SCK_SEL << QSPI_CMD_PORT_SCK_SEL_SHIFT;

	switch (nor->program_opcode) {
	case CMD_PP:
	case CMD_QPP:
		cmd &= ~QSPI_WR_CFG_WR_ADDR_SEL_MASK;
		break;
	case CMD_4PP:
	case CMD_4QPP:
		cmd |= BIT(QSPI_WR_CFG_WR_ADDR_SEL_SHIFT);
		break;
	default:
		dev_err(qspi->dev, "Not support program command:%#x\n", nor->erase_opcode);
		return -EINVAL;
	}

	dev_dbg(qspi->dev, "write cmd:%x\n", cmd);

	writel_relaxed(cmd, qspi->io_base + QSPI_WR_CFG_REG);

	for (i = 0; i < len/4; i++)
		writel_relaxed(*(u32 *)(buf + 4*i), qspi->mm_base + addr + 4*i);

	if (len & mask) {
		addr = addr + (len & ~mask);
		phytium_qspi_read_tmp(qspi, flash->read_cmd, addr, 4, &tmp[0]);
		memcpy(tmp, buf + (len & ~mask), len & mask);
		writel_relaxed(*(u32 *)(tmp), qspi->mm_base + addr);
	}

	writel_relaxed(QSPI_FLUSH_EN, qspi->io_base + QSPI_FLUSH_REG);

	phytium_qspi_wait_cmd(qspi, flash);

	return len;
}

static int phytium_qspi_erase(struct spi_nor *nor, loff_t offs)
{
	struct phytium_qspi_flash *flash = nor->priv;
	struct device *dev = flash->qspi->dev;
	struct phytium_qspi *qspi = flash->qspi;
	u32 cmd = nor->erase_opcode;
	u32 addr = (u32)offs;

	dev_dbg(dev, "erase(%#.2x):offs:%#x\n", nor->erase_opcode, (u32)offs);

	phytium_qspi_write_enable(qspi, flash);

	cmd = cmd << QSPI_CMD_PORT_CMD_SHIFT;
	cmd |= PHYTIUM_SCK_SEL << QSPI_CMD_PORT_SCK_SEL_SHIFT;
	cmd |= flash->cs << QSPI_CMD_PORT_CS_SHIFT;

	/* s25fl256s1 not supoort D8, DC, 20, 21 */
	switch (nor->erase_opcode) {
	case CMD_SE:
		cmd &= ~QSPI_CMD_PORT_SEL_MASK;
		cmd |= BIT(QSPI_CMD_PORT_CMD_ADDR_SHIFT);
		writel_relaxed(addr, qspi->io_base + QSPI_ADDR_PORT_REG);
		break;
	case CMD_4SE:
		cmd |= BIT(QSPI_CMD_PORT_SEL_SHIFT);
		cmd |= BIT(QSPI_CMD_PORT_CMD_ADDR_SHIFT);
		writel_relaxed(addr, qspi->io_base + QSPI_ADDR_PORT_REG);
		break;
	case CMD_P4E:
		cmd &= ~QSPI_CMD_PORT_SEL_MASK;
		cmd |= BIT(QSPI_CMD_PORT_CMD_ADDR_SHIFT);
		writel_relaxed(addr, qspi->io_base + QSPI_ADDR_PORT_REG);
		break;
	case CMD_4P4E:
		cmd |= BIT(QSPI_CMD_PORT_SEL_SHIFT);
		cmd |= BIT(QSPI_CMD_PORT_CMD_ADDR_SHIFT);
		writel_relaxed(addr, qspi->io_base + QSPI_ADDR_PORT_REG);
		break;
	case CMD_BE:
		cmd &= ~QSPI_CMD_PORT_SEL_MASK;
		break;
	case CMD_4BE:
		cmd |= BIT(QSPI_CMD_PORT_SEL_SHIFT);
		break;
	default:
		dev_err(qspi->dev, "Not support erase command:%#x\n", nor->erase_opcode);
		return -EINVAL;
	}

	writel_relaxed(cmd, qspi->io_base + QSPI_CMD_PORT_REG);
	phytium_qspi_cmd_enable(qspi);
	phytium_qspi_wait_cmd(qspi, flash);

	return 0;
}

static int phytium_qspi_prep(struct spi_nor *nor)
{
	struct phytium_qspi_flash *flash = nor->priv;
	struct phytium_qspi *qspi = flash->qspi;

	mutex_lock(&qspi->lock);
	return 0;
}

static void phytium_qspi_unprep(struct spi_nor *nor)
{
	struct phytium_qspi_flash *flash = nor->priv;
	struct phytium_qspi *qspi = flash->qspi;

	mutex_unlock(&qspi->lock);
}

static int phytium_qspi_get_flash_size(struct phytium_qspi *qspi, u32 size)
{
	int ret = 0;
	u32 value;

	switch (size) {
	case SZ_4M:
		value = 0;
		break;
	case SZ_8M:
		value = 1;
		break;
	case SZ_16M:
		value = 2;
		break;
	case SZ_32M:
		value = 3;
		break;
	case SZ_64M:
		value = 4;
		break;
	case SZ_128M:
		value = 5;
		break;
	case SZ_256M:
		value = 6;
		break;
	case SZ_512M:
		value = 7;
		break;
	default:
		value = 0;
		ret = -EINVAL;
		return ret;
	}

	return value;
}

static const struct spi_nor_controller_ops phytium_controller_ops = {
	.prepare = phytium_qspi_prep,
	.unprepare = phytium_qspi_unprep,
	.read_reg = phytium_qspi_read_reg,
	.write_reg = phytium_qspi_write_reg,
	.read = phytium_qspi_read,
	.write = phytium_qspi_write,
	.erase = phytium_qspi_erase,
};

static int phytium_qspi_flash_setup(struct phytium_qspi *qspi, struct fwnode_handle *np)
{
	struct spi_nor_hwcaps hwcaps = { .mask = SNOR_HWCAPS_READ | SNOR_HWCAPS_READ_FAST | SNOR_HWCAPS_PP, };
	u32 width, presc;
	u32 cs_num = 0;
	u32 max_rate = 0;
	u32 clk_div = 0;
	u32 flash_cap = 0;
	u32 addr_width = PHYTIUM_QSPI_1_1_1;
	struct phytium_qspi_flash *flash;
	struct mtd_info *mtd;
	int ret;

	fwnode_property_read_u32(np, "reg", &cs_num);
	if (cs_num >= PHYTIUM_MAX_NORCHIP)
		return -EINVAL;

	fwnode_property_read_u32(np, "spi-max-frequency", &max_rate);
	if (!max_rate)
		return -EINVAL;

	fwnode_property_read_u32(np, "spi-clk-div", &clk_div);
	if (!clk_div)
		clk_div = PHYTIUM_SCK_SEL;

	if (clk_div < 4)
		return -EINVAL;

	presc = DIV_ROUND_UP(qspi->clk_rate, max_rate) - 1;

	fwnode_property_read_u32(np, "spi-rx-bus-width", &width);
	if (!width)
		width = 1;

	if (width == 4) {
		hwcaps.mask |= SNOR_HWCAPS_READ_1_1_4;
		addr_width   = PHYTIUM_QSPI_1_1_4;
	} else if (width == 2) {
		hwcaps.mask |= SNOR_HWCAPS_READ_1_1_2;
		addr_width   = PHYTIUM_QSPI_1_1_2;
	} else if (width != 1)
		return -EINVAL;

	flash = &qspi->flash[cs_num];

	flash->qspi = qspi;
	flash->cs = cs_num;
	flash->presc = presc;
	flash->clk_div = clk_div;
	flash->addr_width = addr_width;
	flash->nor.dev = qspi->dev;
	if (qspi->dev->of_node)
		spi_nor_set_flash_node(&flash->nor, qspi->dev->of_node);
	flash->nor.priv = flash;
	flash->nor.controller_ops = &phytium_controller_ops;

	ret = spi_nor_scan(&flash->nor, NULL, &hwcaps);
	if (ret) {
		dev_err(qspi->dev, "device scan failed\n");
		return ret;
	}

	mtd = &flash->nor.mtd;

	flash->fsize = mtd->size;
	flash->prefetch_limit = mtd->size - PHYTIUM_QSPI_FIFO_SZ;

	ret = phytium_qspi_get_flash_size(flash->qspi, mtd->size);
	if (ret < 0) {
		dev_err(qspi->dev, "flash size invalid\n");
		return ret;
	}

	flash_cap = cs_num << QSPI_FLASH_CAP_NUM_SHIFT;
	flash_cap |= ret;
	writel_relaxed(flash_cap, qspi->io_base + QSPI_FLASH_CAP_REG);

	flash->read_mode = PHYTIUM_FMODE_MM;

	ret = mtd_device_register(mtd, NULL, 0);
	if (ret) {
		dev_err(qspi->dev, "mtd device parse failed\n");
		return ret;
	}

	flash->registered = true;

	dev_dbg(qspi->dev, "read mm:%s %px cs:%d bus:%d clk-div:%d\n",
		flash->read_mode == PHYTIUM_FMODE_MM ? "yes" : "no",
		qspi->mm_base, cs_num, width, clk_div);

	dev_dbg(qspi->dev, "mtd->size:%llx, mtd->erasesize:%x, fsize:%x\n",
			mtd->size, mtd->erasesize, flash->fsize);

	return 0;
}

static void phytium_qspi_mtd_free(struct phytium_qspi *qspi)
{
	int i;

	for (i = 0; i < PHYTIUM_MAX_NORCHIP; i++)
		if (qspi->flash[i].registered)
			mtd_device_unregister(&qspi->flash[i].nor.mtd);
}

static ssize_t clk_div_show(struct device *dev, struct device_attribute *attr,
			      char *buf)
{
	struct phytium_qspi *qspi = dev_get_drvdata(dev);
	struct phytium_qspi_flash *flash = &qspi->flash[0];

	return sprintf(buf, "Flash 0 clk-div: %d\n", flash->clk_div);
}

static ssize_t clk_div_store(struct device *dev,
			       struct device_attribute *attr, const char *buf,
			       size_t size)
{
	struct phytium_qspi *qspi = dev_get_drvdata(dev);
	struct phytium_qspi_flash *flash = &qspi->flash[0];
	long value;
	char *token;
	ssize_t status;

	token = strsep ((char **)&buf, " ");
	if (!token)
		return -EINVAL;

	status = kstrtol(token, 0, &value);
	if (status)
		return status;

	flash->clk_div = (u8)value;

	return size;
}
static DEVICE_ATTR_RW(clk_div);

static struct attribute *phytium_qspi_attrs[] = {
	&dev_attr_clk_div.attr,
	NULL,
};

static struct attribute_group phytium_qspi_attr_group = {
	.attrs = phytium_qspi_attrs,
};

static int phytium_qspi_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct fwnode_handle *flash_np;
	struct phytium_qspi *qspi;
	struct resource *res;
	int ret;

	qspi = devm_kzalloc(dev, sizeof(*qspi), GFP_KERNEL);
	if (!qspi)
		return -ENOMEM;

	qspi->nor_num = device_get_child_node_count(dev);
	if (!qspi->nor_num || qspi->nor_num > PHYTIUM_MAX_NORCHIP)
		return -ENODEV;

	if (dev->of_node)
		res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "qspi");
	else if (has_acpi_companion(dev)) {
		res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	}
	qspi->io_base = devm_ioremap_resource(dev, res);
	if (IS_ERR(qspi->io_base))
		return PTR_ERR(qspi->io_base);


	if (dev->of_node)
		res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "qspi_mm");
	else if (has_acpi_companion(dev))
		res = platform_get_resource(pdev, IORESOURCE_MEM, 1);

	qspi->mm_base = devm_ioremap_resource(dev, res);
	if (IS_ERR(qspi->mm_base))
		return PTR_ERR(qspi->mm_base);

	qspi->mm_size = resource_size(res);

	if (dev->of_node) {
		qspi->clk = devm_clk_get(dev, NULL);
		if (IS_ERR(qspi->clk))
			return PTR_ERR(qspi->clk);

		qspi->clk_rate = clk_get_rate(qspi->clk);
		if (!qspi->clk_rate)
			return -EINVAL;

		ret = clk_prepare_enable(qspi->clk);
		if (ret) {
			dev_err(dev, "can not enable the clock\n");
			return ret;
		}
	}
	else if (has_acpi_companion(dev)) { /* ACPI table not pass clk rate */
		qspi->clk_rate = 50000000;
	}

	qspi->dev = dev;
	platform_set_drvdata(pdev, qspi);
	mutex_init(&qspi->lock);
	spin_lock_init(&qspi->spinlock);

	fwnode_for_each_available_child_node(dev_fwnode(dev), flash_np) {
		ret = phytium_qspi_flash_setup(qspi, flash_np);
		if (ret) {
			dev_err(dev, "unable to setup flash chip\n");
			goto err_flash;
		}
	}

	ret = sysfs_create_group(&qspi->dev->kobj, &phytium_qspi_attr_group);
	if (ret) {
		dev_err(dev, "unable to create sysfs\n");
		goto err_flash;
	}

	return 0;

err_flash:
	mutex_destroy(&qspi->lock);
	phytium_qspi_mtd_free(qspi);

	clk_disable_unprepare(qspi->clk);
	return ret;
}

static int phytium_qspi_remove(struct platform_device *pdev)
{
	struct phytium_qspi *qspi = platform_get_drvdata(pdev);

	sysfs_remove_group(&qspi->dev->kobj, &phytium_qspi_attr_group);

	phytium_qspi_mtd_free(qspi);
	mutex_destroy(&qspi->lock);

	clk_disable_unprepare(qspi->clk);
	return 0;
}

#ifdef CONFIG_ACPI
static const struct acpi_device_id phytium_qspi_acpi_ids[] = {
	{ "PHYT0011", 0 },
	{ /* sentinel */ },
};
MODULE_DEVICE_TABLE(acpi, phytium_qspi_acpi_ids);
#endif

static const struct of_device_id phytium_qspi_match[] = {
	{ .compatible = "phytium,qspi" },
	{ }
};
MODULE_DEVICE_TABLE(of, phytium_qspi_match);

static struct platform_driver phytium_qspi_driver = {
	.probe	= phytium_qspi_probe,
	.remove	= phytium_qspi_remove,
	.driver	= {
		.name = "phytium-quadspi",
		.of_match_table = phytium_qspi_match,
		.acpi_match_table = ACPI_PTR(phytium_qspi_acpi_ids),
	},
};
module_platform_driver(phytium_qspi_driver);

MODULE_AUTHOR("Mingshuai Zhu <zhumingshui@phytium.com.cn>");
MODULE_AUTHOR("Shaojun Yang <yangshaojun@phytium.com.cn>");
MODULE_DESCRIPTION("Phytium QuadSPI driver");
MODULE_LICENSE("GPL v2");
