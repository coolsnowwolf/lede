// SPDX-License-Identifier: GPL-2.0
/*
 * Phytium Quad SPI controller driver.
 *
 * Copyright (c) 2022-2023, Phytium Technology Co., Ltd.
 */

#include <linux/clk.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>

#include <linux/spi/spi-mem.h>
#include <linux/mtd/spi-nor.h>


#define QSPI_FLASH_CAP_REG		0x00
#define  QSPI_FLASH_CAP_NUM_SHIFT	3
#define  QSPI_FLASH_CAP_NUM_MASK	(0x3 << QSPI_FLASH_CAP_NUM_SHIFT)
#define  QSPI_FLASH_CAP_CAP_SHIFT	0
#define  QSPI_FLASH_CAP_CAP_MASK	(0x7 << QSPI_FLASH_CAP_CAP_SHIFT)

#define QSPI_RD_CFG_REG			0x04
#define  QSPI_RD_CFG_RD_CMD_SHIFT	24
#define  QSPI_RD_CFG_RD_CMD_MASK	(0xff << QSPI_RD_CFG_RD_CMD_SHIFT)
#define  QSPI_RD_CFG_RD_THROUGH_SHIFT	23
#define  QSPI_RD_CFG_RD_THROUGH_MASK	(0x1 << QSPI_RD_CFG_RD_THROUGH_SHIFT)
#define  QSPI_RD_CFG_RD_TRANSFER_SHIFT	20
#define  QSPI_RD_CFG_RD_TRANSFER_MASK	(0x7 << QSPI_RD_CFG_RD_TRANSFER_SHIFT)
#define  QSPI_RD_CFG_RD_ADDR_SEL_SHIFT	19
#define  QSPI_RD_CFG_RD_ADDR_SEL_MASK	(0x1 << QSPI_RD_CFG_RD_ADDR_SEL_SHIFT)
#define  QSPI_RD_CFG_RD_LATENCY_SHIFT	18
#define  QSPI_RD_CFG_RD_LATENCY_MASK	(0x1 << QSPI_RD_CFG_RD_LATENCY_SHIFT)
#define  QSPI_RD_CFG_MODE_BYTE_SHIFT	17
#define  QSPI_RD_CFG_MODE_BYTE_MASK	(0x1 << QSPI_RD_CFG_MODE_BYTE_SHIFT)
#define  QSPI_RD_CFG_CMD_SIGN_SHIFT	9
#define  QSPI_RD_CFG_CMD_SIGN_MASK	(0xff << QSPI_RD_CFG_CMD_SIGN_SHIFT)
#define  QSPI_RD_CFG_DUMMY_SHIFT	4
#define  QSPI_RD_CFG_DUMMY_MASK		(0x1f << QSPI_RD_CFG_DUMMY_SHIFT)
#define  QSPI_RD_CFG_D_BUFFER_SHIFT	3
#define  QSPI_RD_CFG_D_BUFFER_MASK	(0x1 << QSPI_RD_CFG_D_BUFFER_SHIFT)
#define  QSPI_RD_CFG_RD_SCK_SEL_SHIFT	0
#define  QSPI_RD_CFG_RD_SCK_SEL_MASK	(0x7 << QSPI_RD_CFG_RD_SCK_SEL_SHIFT)

#define QSPI_WR_CFG_REG			0x08
#define  QSPI_WR_CFG_WR_CMD_SHIFT	24
#define  QSPI_WR_CFG_WR_CMD_MASK	(0xff << QSPI_WR_CFG_WR_CMD_SHIFT)
#define  QSPI_WR_CFG_WR_WAIT_SHIFT	9
#define  QSPI_WR_CFG_WR_WAIT_MASK	(0x01 << QSPI_WR_CFG_WR_WAIT_SHIFT)
#define  QSPI_WR_CFG_WR_THROUGH_SHIFT	8
#define  QSPI_WR_CFG_WR_THROUGH_MASK	(0x01 << QSPI_WR_CFG_WR_THROUGH_SHIFT)
#define  QSPI_WR_CFG_WR_TRANSFER_SHIFT	5
#define  QSPI_WR_CFG_WR_TRANSFER_MASK	(0X7 << QSPI_WR_CFG_WR_TRANSFER_SHIFT)
#define  QSPI_WR_CFG_WR_ADDR_SEL_SHIFT	4
#define  QSPI_WR_CFG_WR_ADDR_SEL_MASK	(0x1 << QSPI_WR_CFG_WR_ADDR_SEL_SHIFT)
#define  QSPI_WR_CFG_WR_MODE_SHIFT	3
#define  QSPI_WR_CFG_WR_MODE_MASK	(0x1 << QSPI_WR_CFG_WR_MODE_SHIFT)
#define  QSPI_WR_CFG_WR_SCK_SEL_SHIFT	0
#define  QSPI_WR_CFG_WR_SCK_SEL_MASK	(0x7 << QSPI_WR_CFG_WR_SCK_SEL_SHIFT)

#define QSPI_FLUSH_REG			0x0c
#define  QSPI_FLUSH_EN			(0x1 << 0)

#define QSPI_CMD_PORT_REG		0x10
#define  QSPI_CMD_PORT_CMD_SHIFT	24
#define  QSPI_CMD_PORT_CMD_MASK		(0xff << QSPI_CMD_PORT_CMD_SHIFT)
#define  QSPI_CMD_PORT_WAIT_SHIFT	22
#define  QSPI_CMD_PORT_WAIT_MASK	(0x1 << QSPI_CMD_PORT_WAIT_SHIFT)
#define  QSPI_CMD_PORT_THROUGH_SHIFT	21
#define  QSPI_CMD_PORT_THROUGH_MASK	(0x1 << QSPI_CMD_PORT_THROUGH_SHIFT)
#define  QSPI_CMD_PORT_CS_SHIFT		19
#define  QSPI_CMD_PORT_CS_MASK		(0x3 << QSPI_CMD_PORT_CS_SHIFT)
#define  QSPI_CMD_PORT_TRANSFER_SHIFT	16
#define  QSPI_CMD_PORT_TRANSFER_MASK	(0x7 << QSPI_CMD_PORT_TRANSFER_SHIFT)
#define  QSPI_CMD_PORT_CMD_ADDR_SHIFT	15
#define  QSPI_CMD_PORT_CMD_ADDR_MASK	(0x1 << QSPI_CMD_PORT_CMD_ADDR_SHIFT)
#define  QSPI_CMD_PORT_LATENCY_SHIFT	14
#define  QSPI_CMD_PORT_LATENCY_MASK	(0x1 << QSPI_CMD_PORT_LATENCY_SHIFT)
#define  QSPI_CMD_PORT_DATA_XFER_SHIFT	13
#define  QSPI_CMD_PORT_DATA_XFER_MASK	(0x1 << QSPI_CMD_PORT_DATA_XFER_SHIFT)
#define  QSPI_CMD_PORT_ADDR_SEL_SHIFT	12
#define  QSPI_CMD_PORT_ADDR_SEL_MASK	(0x1 << QSPI_CMD_PORT_ADDR_SEL_SHIFT)
#define  QSPI_CMD_PORT_DUMMY_SHIFT	7
#define  QSPI_CMD_PORT_DUMMY_MASK	(0x1f << QSPI_CMD_PORT_DUMMY_SHIFT)
#define  QSPI_CMD_PORT_P_BUFFER_SHIFT	6
#define  QSPI_CMD_PORT_P_BUFFER_MASK	(0x1 << QSPI_CMD_PORT_P_BUFFER_SHIFT)
#define  QSPI_CMD_PORT_RW_NUM_SHIFT	3
#define  QSPI_CMD_PORT_RW_NUM_MASK	(0x7 << QSPI_CMD_PORT_RW_NUM_SHIFT)
#define  QSPI_CMD_PORT_SCK_SEL_SHIFT	0
#define  QSPI_CMD_PORT_SCK_SEL_MASK	(0x7 << QSPI_CMD_PORT_SCK_SEL_SHIFT)

#define QSPI_ADDR_PORT_REG		0x14
#define QSPI_HD_PORT_REG		0x18
#define QSPI_LD_PORT_REG		0x1c

#define QSPI_FUN_SET_REG		0x20
#define  QSPI_FUN_SET_HOLD_SHIFT	24
#define  QSPI_FUN_SET_HOLD_MASK		(0xff << QSPI_FUN_SET_HOLD_SHIFT)
#define  QSPI_FUN_SET_SETUP_SHIFT	16
#define  QSPI_FUN_SET_SETUP_MASK	(0xff << QSPI_FUN_SET_SETUP_SHIFT)
#define  QSPI_FUN_SET_DELAY_SHIFT	0
#define  QSPI_FUN_SET_DELAY_MASK	(0xffff << QSPI_FUN_SET_DELAY_SHIFT)

#define QSPI_WIP_REG			0x24
#define  QSPI_WIP_W_CMD_SHIFT		24
#define  QSPI_WIP_W_CMD_MASK		(0xff << QSPI_WIP_W_CMD_SHIFT)
#define  QSPI_WIP_W_TRANSFER_SHIFT	3
#define  QSPI_WIP_W_TRANSFER_MASK	(0x3 << QSPI_WIP_W_TRANSFER_SHIFT)
#define  QSPI_WIP_W_SCK_SEL_SHIFT	0
#define  QSPI_WIP_W_SCK_SEL_MASK	(0x7 << QSPI_WIP_W_SCK_SEL_SHIFT)

#define QSPI_WP_REG			0x28
#define  QSPI_WP_EN_SHIFT		17
#define  QSPI_WP_EN_MASK		(0x1 << QSPI_WP_EN_SHIFT)
#define  QSPI_WP_IO2_SHIFT		16
#define  QSPI_WP_IO2_MASK		(0x1 << QSPI_WP_IO2_SHIFT)
#define  QSPI_WP_HOLD_SHIFT		8
#define  QSPI_WP_HOLD_MASK		(0xff << QSPI_WP_HOLD_SHIFT)
#define  QSPI_WP_SETUP_SHIFT		0
#define  QSPI_WP_SETUP_MASK		(0xff << QSPI_WP_SETUP_SHIFT)

#define QSPI_MODE_REG			0x2c
#define  QSPI_MODE_VALID_SHIFT		8
#define  QSPI_MODE_VALID_MASK		(0xff << QSPI_MODE_VALID_SHIFT)
#define  QSPI_MODE_SHIFT		0
#define  QSPI_MODE_MASK			(0xff << QSPI_MODE_SHIFT)

#define PHYTIUM_QSPI_MAX_NORCHIP	4
#define PHYTIUM_QSPI_MAX_MMAP_SZ	(SZ_256M * PHYTIUM_QSPI_MAX_NORCHIP)
#define PHYTIUM_QSPI_MAX_XFER_SZ	8
#define PHYTIUM_QSPI_DEFAULT_SCK_SEL	5

#define XFER_PROTO_1_1_1		0x0
#define XFER_PROTO_1_1_2		0x1
#define XFER_PROTO_1_1_4		0x2
#define XFER_PROTO_1_2_2		0x3
#define XFER_PROTO_1_4_4		0x4
#define XFER_PROTO_2_2_2		0x5
#define XFER_PROTO_4_4_4		0x6

struct phytium_qspi_flash {
	u32 cs;
	u32 clk_div;

	void __iomem *base;
	resource_size_t size;
	struct spi_device *spi;
};

struct phytium_qspi {
	struct device *dev;
	struct spi_controller *ctrl;

	void __iomem *io_base;
	void __iomem *mm_base;
	resource_size_t mm_size;
	resource_size_t used_size;

	struct clk *clk;
	u32 clk_rate;

	struct phytium_qspi_flash flash[PHYTIUM_QSPI_MAX_NORCHIP];
	u8 fnum;
	bool nodirmap;
};

static bool phytium_qspi_check_buswidth(u8 width)
{
	switch (width) {
	case 1:
	case 2:
	case 4:
		return 0;
	}

	return -ENOTSUPP;
}

static uint phytium_spi_nor_clac_clk_div(int div)
{
	uint clk_div = 0;

	if (div <= 2)
		clk_div = 1;
	else if (div <= 4)
		clk_div = 2;
	else if (div <= 8)
		clk_div = 3;
	else if (div <= 16)
		clk_div = 4;
	else if (div <= 32)
		clk_div = 5;
	else if (div <= 64)
		clk_div = 6;
	else if (div <= 128)
		clk_div = 7;
	else
		clk_div = 65535;

	return clk_div;
}

static int phytium_spi_nor_protocol_encode(const struct spi_mem_op *op, u32 *code)
{
	int ret = 0;

	if (op->cmd.buswidth == 1 &&
	    op->addr.buswidth == 1 &&
	    op->data.buswidth == 1)
		*code = XFER_PROTO_1_1_1;
	else if (op->cmd.buswidth == 1 &&
		 op->addr.buswidth == 1 &&
		 op->data.buswidth == 2)
		*code = XFER_PROTO_1_1_2;
	else if (op->cmd.buswidth == 1 &&
		 op->addr.buswidth == 1 &&
		 op->data.buswidth == 4)
		*code = XFER_PROTO_1_1_4;
	else if (op->cmd.buswidth == 1 &&
		 op->addr.buswidth == 2 &&
		 op->data.buswidth == 2)
		*code = XFER_PROTO_1_2_2;
	else if (op->cmd.buswidth == 1 &&
		 op->addr.buswidth == 4 &&
		 op->data.buswidth == 4)
		*code = XFER_PROTO_1_4_4;
	else if (op->cmd.buswidth == 2 &&
		 op->addr.buswidth == 2 &&
		 op->data.buswidth == 2)
		*code = XFER_PROTO_2_2_2;
	else if (op->cmd.buswidth == 4 &&
		 op->addr.buswidth == 4 &&
		 op->data.buswidth == 4)
		*code = XFER_PROTO_4_4_4;
	else
		*code = XFER_PROTO_1_1_1;

	return ret;
}

static int phytium_qspi_flash_capacity_encode(u32 size, u32 *cap)
{
	int ret = 0;

	switch (size) {
	case SZ_4M:
		*cap = 0x0;
		break;
	case SZ_8M:
		*cap = 0x1;
		break;
	case SZ_16M:
		*cap = 0x2;
		break;
	case SZ_32M:
		*cap = 0x3;
		break;
	case SZ_64M:
		*cap = 0x4;
		break;
	case SZ_128M:
		*cap = 0x5;
		break;
	case SZ_256M:
		*cap = 0x6;
		break;
	case SZ_512M:
		*cap = 0x7;
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

static int phytium_qspi_write_port(struct phytium_qspi *qspi,
				   const u8 *buf, const size_t len)
{
	u32 bouncebuf[2] = { 0 };

	if (len > PHYTIUM_QSPI_MAX_XFER_SZ) {
		dev_err(qspi->dev, "WRITE data exceeds 8 bytes.\n");
		return -EINVAL;
	}

	memcpy(bouncebuf, buf, len);

	if (len > 4)
		writel_relaxed(bouncebuf[1], qspi->io_base + QSPI_HD_PORT_REG);
	writel_relaxed(bouncebuf[0], qspi->io_base + QSPI_LD_PORT_REG);

	return 0;
}

static int phytium_qspi_read_port(struct phytium_qspi *qspi,
				  u8 *buf, size_t len)
{
	u32 bouncebuf[2] = { 0 };

	if (len > PHYTIUM_QSPI_MAX_XFER_SZ) {
		dev_err(qspi->dev, "READ data exceeds 8 bytes.\n");
		return -EINVAL;
	}

	/* Dummy write to LD_PORT register and issue READ ops*/
	writel_relaxed(0, qspi->io_base + QSPI_LD_PORT_REG);

	/* Read data */
	bouncebuf[0] = readl_relaxed(qspi->io_base + QSPI_LD_PORT_REG);
	if (len > 4)
		bouncebuf[1] = readl_relaxed(qspi->io_base + QSPI_HD_PORT_REG);

	memcpy(buf, bouncebuf, len);

	return 0;
}

static int phytium_qspi_adjust_op_size(struct spi_mem *mem,
					  struct spi_mem_op *op)
{
	if (op->data.nbytes > PHYTIUM_QSPI_MAX_XFER_SZ)
		op->data.nbytes = PHYTIUM_QSPI_MAX_XFER_SZ;

	return 0;
}

static bool phytium_qspi_supports_op(struct spi_mem *mem,
				     const struct spi_mem_op *op)
{
	int ret;

	ret = phytium_qspi_check_buswidth(op->cmd.buswidth);

	if (op->addr.nbytes)
		ret |= phytium_qspi_check_buswidth(op->addr.buswidth);

	if (op->dummy.nbytes)
		ret |= phytium_qspi_check_buswidth(op->dummy.buswidth);

	if (op->data.nbytes)
		ret |= phytium_qspi_check_buswidth(op->data.buswidth);

	if (ret)
		return false;

	/* Max 32 dummy clock cycles supported */
	if (op->dummy.nbytes &&
	    (op->dummy.nbytes * 8 / op->dummy.buswidth > 32))
		return false;

	return spi_mem_default_supports_op(mem, op);
}

static int phytium_qspi_exec_op(struct spi_mem *mem,
				const struct spi_mem_op *op)
{
	struct phytium_qspi *qspi = spi_controller_get_devdata(mem->spi->master);
	struct phytium_qspi_flash *flash = &qspi->flash[mem->spi->chip_select];
	u32 cmd, transfer;
	int ret;

	dev_dbg(qspi->dev, "cmd:%#x mode: %d.%d.%d.%d addr:%#llx len:%#x\n",
		op->cmd.opcode, op->cmd.buswidth, op->addr.buswidth,
		op->dummy.buswidth, op->data.buswidth, op->addr.val,
		op->data.nbytes);

	cmd = op->cmd.opcode << QSPI_CMD_PORT_CMD_SHIFT;
	cmd |= flash->cs << QSPI_CMD_PORT_CS_SHIFT;

	ret = phytium_spi_nor_protocol_encode(op, &transfer);
	if (ret) {
		dev_err(qspi->dev, "Unsupported SPI NOR protocol.\n");
		goto out;
	}
	cmd |= transfer << QSPI_CMD_PORT_TRANSFER_SHIFT;

	if (op->addr.nbytes) {
		cmd |= QSPI_CMD_PORT_CMD_ADDR_MASK;
		if (op->addr.nbytes == 4)
			cmd |= QSPI_CMD_PORT_ADDR_SEL_MASK;

		/* Write target address to ADDR_PORT register */
		writel_relaxed(op->addr.val, qspi->io_base + QSPI_ADDR_PORT_REG);
	}

	if (op->dummy.nbytes) {
		cmd |= QSPI_CMD_PORT_LATENCY_MASK;
		cmd |= ((op->dummy.nbytes * 8) / op->dummy.buswidth) <<
			QSPI_CMD_PORT_LATENCY_SHIFT;
	}

	if (op->data.nbytes) {
		cmd |= QSPI_CMD_PORT_DATA_XFER_MASK;
		cmd &= ~QSPI_CMD_PORT_P_BUFFER_MASK;
		cmd |= (op->data.nbytes-1) << QSPI_CMD_PORT_RW_NUM_SHIFT;
	}

	cmd |= flash->clk_div;
	writel_relaxed(cmd, qspi->io_base + QSPI_CMD_PORT_REG);

	if (op->data.dir == SPI_MEM_DATA_IN) {
		ret = phytium_qspi_read_port(qspi, op->data.buf.in, op->data.nbytes);
		if (ret) {
			dev_err(qspi->dev, "Failed to read data from the port.\n");
			goto out;
		}
	} else if (op->data.dir == SPI_MEM_DATA_OUT) {
		ret = phytium_qspi_write_port(qspi, op->data.buf.out, op->data.nbytes);
		if (ret) {
			dev_err(qspi->dev, "Failed to write data to the port.\n");
			goto out;
		}
	} else {
		/* Dummy write to LD_PORT register and issue the command */
		writel_relaxed(0, qspi->io_base + QSPI_LD_PORT_REG);
	}

out:
	return ret;
}

static int phytium_qspi_dirmap_create(struct spi_mem_dirmap_desc *desc)
{
	struct spi_device *spi = desc->mem->spi;
	struct phytium_qspi *qspi = spi_controller_get_devdata(spi->master);
	struct phytium_qspi_flash *flash = &qspi->flash[spi->chip_select];
	struct spi_nor *nor = spi_mem_get_drvdata(desc->mem);
	u32 cmd, transfer;
	int ret = 0;

	if (!qspi->mm_base || !qspi->mm_size) {
		ret = -EOPNOTSUPP;
		goto out;
	}

	if (!flash->base) {
		flash->base = qspi->mm_base + qspi->used_size;
		qspi->used_size += nor->mtd.size;
	}

	/* Setup RD/WR_CFG register */
	if (desc->info.op_tmpl.data.dir == SPI_MEM_DATA_IN) {
		cmd = desc->info.op_tmpl.cmd.opcode << QSPI_RD_CFG_RD_CMD_SHIFT;
		ret = phytium_spi_nor_protocol_encode(&desc->info.op_tmpl, &transfer);
		if (ret) {
			dev_err(qspi->dev, "Unsupported SPI NOR protocol.\n");
			goto out;
		}
		cmd |= transfer << QSPI_RD_CFG_RD_TRANSFER_SHIFT;

		if (desc->info.op_tmpl.addr.nbytes == 4)
			cmd |= QSPI_RD_CFG_RD_ADDR_SEL_MASK;

		if (nor->read_dummy) {
			cmd |= QSPI_RD_CFG_RD_LATENCY_MASK;
			cmd |= (nor->read_dummy - 1) << QSPI_RD_CFG_DUMMY_SHIFT;
		}

		cmd |= QSPI_RD_CFG_D_BUFFER_MASK;
		cmd |= flash->clk_div & QSPI_RD_CFG_RD_SCK_SEL_MASK;

		writel_relaxed(cmd, qspi->io_base + QSPI_RD_CFG_REG);

		dev_dbg(qspi->dev, "Create read dirmap and setup RD_CFG_REG [%#x].\n", cmd);
	} else if (desc->info.op_tmpl.data.dir == SPI_MEM_DATA_OUT) {
		cmd = desc->info.op_tmpl.cmd.opcode << QSPI_WR_CFG_WR_CMD_SHIFT;
		ret = phytium_spi_nor_protocol_encode(&desc->info.op_tmpl, &transfer);
		if (ret) {
			dev_err(qspi->dev, "Unsupported SPI NOR protocol.\n");
			goto out;
		}
		cmd |= transfer << QSPI_WR_CFG_WR_TRANSFER_SHIFT;

		if (desc->info.op_tmpl.addr.nbytes == 4)
			cmd |= QSPI_WR_CFG_WR_ADDR_SEL_MASK;

		cmd |= QSPI_WR_CFG_WR_MODE_MASK;
		cmd |= flash->clk_div & QSPI_WR_CFG_WR_SCK_SEL_MASK;

		writel_relaxed(cmd, qspi->io_base + QSPI_WR_CFG_REG);

		dev_dbg(qspi->dev, "Create write dirmap and setup WR_CFG_REG [%#x].\n", cmd);
	} else {
		ret = -EINVAL;
	}

out:
	return ret;
}

static ssize_t phytium_qspi_dirmap_read(struct spi_mem_dirmap_desc *desc,
					u64 offs, size_t len, void *buf)
{
	struct spi_device *spi = desc->mem->spi;
	struct phytium_qspi *qspi = spi_controller_get_devdata(spi->master);
	struct phytium_qspi_flash *flash = &qspi->flash[spi->chip_select];

	void __iomem *src = flash->base + offs;
	u8 *buf_rx = buf;

	memcpy_fromio(buf_rx, src, len);

	return len;
}

static ssize_t phytium_qspi_dirmap_write(struct spi_mem_dirmap_desc *desc,
					 u64 offs, size_t len, const void *buf)
{
	struct spi_device *spi = desc->mem->spi;
	struct phytium_qspi *qspi = spi_controller_get_devdata(spi->master);
	struct phytium_qspi_flash *flash = &qspi->flash[spi->chip_select];

	void __iomem *dst = flash->base + offs;
	void __iomem *addr;
	int i;
	size_t mask = 0x03;
	u_char tmp[4] = {0};

	if (offs & 0x03) {
		dev_err(qspi->dev, "Addr not four-byte aligned!\n");
		return -EINVAL;
	}

	for (i = 0; i < len / 4; i++)
		writel_relaxed(*(u32 *)(buf + 4 * i), dst + 4 * i);

	if (len & mask) {
		addr =  dst + (len & ~mask);
		memcpy(tmp, buf + (len & ~mask), len & mask);
		writel_relaxed(*(u32 *)(tmp), addr);
	}

	//write cache data to flash
	writel_relaxed(QSPI_FLUSH_EN, qspi->io_base + QSPI_FLUSH_REG);

	return len;
}

static int phytium_qspi_setup(struct spi_device *spi)
{
	struct spi_controller *ctrl = spi->master;
	struct phytium_qspi *qspi = spi_controller_get_devdata(ctrl);
	struct phytium_qspi_flash *flash;
	uint clk_div;

	if (ctrl->busy)
		return -EBUSY;

	flash = &qspi->flash[spi->chip_select];

	flash->cs = spi->chip_select;
	flash->spi = spi;
	if (flash->cs >= PHYTIUM_QSPI_MAX_NORCHIP) {
		dev_err(qspi->dev, "Flash CS is out of range.\n");
		return -EINVAL;
	}
	qspi->fnum++;


	if (spi->max_speed_hz) {
		clk_div = DIV_ROUND_UP(qspi->clk_rate, spi->max_speed_hz);
		flash->clk_div = phytium_spi_nor_clac_clk_div(clk_div);
		if (flash->clk_div == 65535) {
			dev_err(qspi->dev, "qspi maximum frequency setting is error.\n");
			return -EINVAL;
		}
	} else
		flash->clk_div = PHYTIUM_QSPI_DEFAULT_SCK_SEL;

	return 0;
}

static struct spi_controller_mem_ops phytium_qspi_mem_ops = {
	.adjust_op_size = phytium_qspi_adjust_op_size,
	.supports_op	= phytium_qspi_supports_op,
	.exec_op	= phytium_qspi_exec_op,
	.dirmap_create	= phytium_qspi_dirmap_create,
	.dirmap_read	= phytium_qspi_dirmap_read,
	.dirmap_write	= phytium_qspi_dirmap_write,
};

/**
 * Direct mapping is supported only when all flashes under the controller
 * are of the same size and the mapping address is continuous. For those
 * cases which flashes are of different sizes, the driver offered a non-dirmap
 * mem_ops with which read/write ops is executed through command port.
 */
static struct spi_controller_mem_ops phytium_qspi_mem_ops_nodirmap = {
	.adjust_op_size = phytium_qspi_adjust_op_size,
	.supports_op	= phytium_qspi_supports_op,
	.exec_op	= phytium_qspi_exec_op,
};

/**
 * phytium_qspi_probe - Probe method for the QSPI driver
 * @pdev:	Pointer to the platform_device structure
 *
 * This function initializes the driver data structures and the hardware.
 *
 * Return:	0 on success and error value on failure
 */
static int phytium_qspi_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct spi_controller *ctrl;
	struct resource *res;
	struct phytium_qspi *qspi;
	int i, ret;
	u32 flash_cap;
	struct spi_mem *mem;
	struct spi_nor *nor;

	ctrl = spi_alloc_master(dev, sizeof(*qspi));
	if (!ctrl)
		return -ENOMEM;

	ctrl->mode_bits = SPI_CPOL | SPI_CPHA |
			  SPI_RX_DUAL | SPI_RX_QUAD |
			  SPI_TX_DUAL | SPI_TX_QUAD;
	ctrl->setup = phytium_qspi_setup;
	ctrl->num_chipselect = PHYTIUM_QSPI_MAX_NORCHIP;
	ctrl->dev.of_node = dev->of_node;

	qspi = spi_controller_get_devdata(ctrl);
	qspi->ctrl = ctrl;

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "qspi");
	qspi->io_base = devm_ioremap_resource(dev, res);
	if (IS_ERR(qspi->io_base)) {
		ret = PTR_ERR(qspi->io_base);
		goto probe_master_put;
	}

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "qspi_mm");
	qspi->mm_base = devm_ioremap_resource(dev, res);
	if (IS_ERR(qspi->mm_base)) {
		ret = PTR_ERR(qspi->mm_base);
		goto probe_master_put;
	}

	qspi->mm_size = resource_size(res);
	if (qspi->mm_size > PHYTIUM_QSPI_MAX_MMAP_SZ) {
		ret = -EINVAL;
		goto probe_master_put;
	}
	qspi->used_size = 0;

	qspi->clk = devm_clk_get(dev, NULL);
	if (IS_ERR(qspi->clk)) {
		ret = PTR_ERR(qspi->clk);
		goto probe_master_put;
	}

	qspi->clk_rate = clk_get_rate(qspi->clk);
	if (!qspi->clk_rate) {
		ret = -EINVAL;
		goto probe_master_put;
	}

	pm_runtime_enable(dev);
	ret = pm_runtime_get_sync(dev);
	if (ret < 0) {
		pm_runtime_put_noidle(dev);
		goto probe_master_put;
	}

	ret = clk_prepare_enable(qspi->clk);
	if (ret) {
		dev_err(dev, "Failed to enable PCLK of the controller.\n");
		goto probe_clk_failed;
	}

	qspi->nodirmap = device_property_present(dev, "no-direct-mapping");
	ctrl->mem_ops = qspi->nodirmap ?
			&phytium_qspi_mem_ops_nodirmap :
			&phytium_qspi_mem_ops;

	qspi->dev = dev;
	platform_set_drvdata(pdev, qspi);

	ret = devm_spi_register_controller(dev, ctrl);
	if (ret) {
		dev_err(dev, "failed to register SPI controller: %d\n", ret);
		goto probe_setup_failed;
	}

	if (!qspi->nodirmap) {
		/*
		 * The controller supports direct mapping access only if all
		 * flashes are of same size.
		 */

		i = 0;
		for (i = 0; qspi->fnum > i && i < PHYTIUM_QSPI_MAX_NORCHIP; i++) {
			if (qspi->flash[i].spi) {
				mem = spi_get_drvdata(qspi->flash[i].spi);
				if (mem) {
					nor = spi_mem_get_drvdata(mem);
					if (nor)
						qspi->flash[i].size = nor->mtd.size;
				}
			}
		}

		for (i = 1; qspi->fnum > i && i < PHYTIUM_QSPI_MAX_NORCHIP; i++) {
			if (qspi->flash[i].size != qspi->flash[0].size) {
				dev_err(dev, "Flashes are of different sizes.\n");
				ret = -EINVAL;
				goto probe_setup_failed;
			}
		}

		ret = phytium_qspi_flash_capacity_encode(qspi->flash[0].size,
							 &flash_cap);
		if (ret) {
			dev_err(dev, "Flash size is invalid.\n");
			goto probe_setup_failed;
		}

		flash_cap |= qspi->fnum << QSPI_FLASH_CAP_NUM_SHIFT;

		writel_relaxed(flash_cap, qspi->io_base + QSPI_FLASH_CAP_REG);
	}

	return 0;

probe_setup_failed:
	clk_disable_unprepare(qspi->clk);
probe_clk_failed:
	pm_runtime_put_sync(dev);
	pm_runtime_disable(dev);
probe_master_put:

	return ret;
}

/**
 * phytium_qspi_remove - Remove method for the QSPI driver
 * @pdev:	Pointer to the platform_device structure
 *
 * This function is called if a device is physically removed from the system
 * or if the driver module is being unloaded. It free all resources allocated
 * to the device.
 *
 * Return:	0 on success and error value on failure
 */
static int phytium_qspi_remove(struct platform_device *pdev)
{
	struct phytium_qspi *qspi = platform_get_drvdata(pdev);

	clk_disable_unprepare(qspi->clk);

	pm_runtime_put_sync(&pdev->dev);
	pm_runtime_disable(&pdev->dev);

	return 0;
}

static int __maybe_unused phytium_qspi_suspend(struct device *dev)
{
	return pm_runtime_force_suspend(dev);
}

static int __maybe_unused phytium_qspi_resume(struct device *dev)
{
	return pm_runtime_force_resume(dev);
}

static const struct dev_pm_ops phytium_qspi_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(phytium_qspi_suspend,
				phytium_qspi_resume)
};

static const struct of_device_id phytium_qspi_of_match[] = {
	{ .compatible = "phytium,qspi-nor" },
	{ }
};
MODULE_DEVICE_TABLE(of, phytium_qspi_of_match);

static struct platform_driver phytium_qspi_driver = {
	.probe = phytium_qspi_probe,
	.remove = phytium_qspi_remove,
	.driver = {
		.name = "phytium-qspi",
		.of_match_table = of_match_ptr(phytium_qspi_of_match),
		.pm = &phytium_qspi_pm_ops,
	},
};
module_platform_driver(phytium_qspi_driver);

MODULE_AUTHOR("Chen Baozi <chenbaozi@phytium.com.cn>");
MODULE_DESCRIPTION("Phytium Quad SPI driver");
MODULE_LICENSE("GPL v2");
