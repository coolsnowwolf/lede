// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * A driver for Siflower SF21A6826/SF21H8898 QSPI controller.
 *
 * Based on the AMBA PL022 driver:
 * Copyright (C) 2008-2012 ST-Ericsson AB
 * Copyright (C) 2006 STMicroelectronics Pvt. Ltd.
 */

#include <linux/bitfield.h>
#include <linux/clk.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/sizes.h>

#include <linux/spi/spi-mem.h>
#include <linux/spi/spi.h>

#define SF_SSP_FIFO_DEPTH		0x100

#define SSP_CR0				0x000
#define SSP_CR1				0x004
#define SSP_DR				0x008
#define SSP_SR				0x00C
#define SSP_CPSR			0x010
#define SSP_IMSC			0x014
#define SSP_RIS				0x018
#define SSP_MIS				0x01C
#define SSP_ICR				0x020
#define SSP_DMACR			0x024
#define SSP_FIFO_LEVEL			0x028
#define SSP_EXSPI_CMD0			0x02C
#define SSP_EXSPI_CMD1			0x030
#define SSP_EXSPI_CMD2			0x034

/* SSP Control Register 0  - SSP_CR0 */
#define SSP_CR0_EXSPI_FRAME	(0x3 << 4)
#define SSP_CR0_SPO		(0x1 << 6)
#define SSP_CR0_SPH		(0x1 << 7)
#define SSP_CR0_BIT_MODE(x)	((x)-1)
#define SSP_SCR_SHFT		8

/* SSP Control Register 1  - SSP_CR1 */
#define SSP_CR1_MASK_SSE	(0x1 << 1)

/* SSP Status Register - SSP_SR */
#define SSP_SR_MASK_TFE		(0x1 << 0) /* Transmit FIFO empty */
#define SSP_SR_MASK_TNF		(0x1 << 1) /* Transmit FIFO not full */
#define SSP_SR_MASK_RNE		(0x1 << 2) /* Receive FIFO not empty */
#define SSP_SR_MASK_RFF		(0x1 << 3) /* Receive FIFO full */
#define SSP_SR_MASK_BSY		(0x1 << 4) /* Busy Flag */

/* SSP FIFO Threshold Register - SSP_FIFO_LEVEL */
#define SSP_FIFO_LEVEL_RX	GENMASK(14, 8) /* Receive FIFO watermark */
#define SSP_FIFO_LEVEL_TX	GENMASK(6, 0) /* Transmit FIFO watermark */
#define DFLT_THRESH_RX		32
#define DFLT_THRESH_TX		32

/* SSP Raw Interrupt Status Register - SSP_RIS */
#define SSP_RIS_MASK_RORRIS	(0x1 << 0) /* Receive Overrun */
#define SSP_RIS_MASK_RTRIS	(0x1 << 1) /* Receive Timeout */
#define SSP_RIS_MASK_RXRIS	(0x1 << 2) /* Receive FIFO Raw Interrupt status */
#define SSP_RIS_MASK_TXRIS	(0x1 << 3) /* Transmit FIFO Raw Interrupt status */

/* EXSPI command register 0 SSP_EXSPI_CMD0 */
#define EXSPI_CMD0_CMD_COUNT	BIT(0)		/* cmd byte, must be set at last */
#define EXSPI_CMD0_ADDR_COUNT	GENMASK(2, 1)	/* addr bytes */
#define EXSPI_CMD0_EHC_COUNT	BIT(3)		/* Set 1 for 4-byte address mode */
#define EXSPI_CMD0_TX_COUNT	GENMASK(14, 4)	/* TX data bytes */
#define EXSPI_CMD0_VALID	BIT(15)		/* Set 1 to make the cmd to be run */

/* EXSPI command register 1 SSP_EXSPI_CMD1 */
#define EXSPI_CMD1_DUMMY_COUNT	GENMASK(3, 0)	/* dummy bytes */
#define EXSPI_CMD1_RX_COUNT	GENMASK(14, 4)	/* RX data bytes */

/* EXSPI command register 2 SSP_EXSPI_CMD2 */
/* Set 1 for 1-wire, 2 for 2-wire, 3 for 4-wire */
#define EXSPI_CMD2_CMD_IO_MODE	GENMASK(1, 0)	/* cmd IO mode */
#define EXSPI_CMD2_ADDR_IO_MODE	GENMASK(3, 2)	/* addr IO mode */
#define EXSPI_CMD2_DATA_IO_MODE	GENMASK(5, 4)	/* data IO mode */

/* SSP Clock Defaults */
#define SSP_DEFAULT_CLKRATE 0x2
#define SSP_DEFAULT_PRESCALE 0x40

/* SSP Clock Parameter ranges */
#define CPSDVR_MIN 0x02
#define CPSDVR_MAX 0xFE
#define SCR_MIN 0x00
#define SCR_MAX 0xFF

#define SF_READ_TIMEOUT		(10 * HZ)
#define MAX_S_BUF			100

struct sf_qspi {
	void __iomem *base;
	struct clk *clk, *apbclk;
	struct device *dev;
};

struct ssp_clock_params {
	u32 freq;
	u8 cpsdvsr; /* value from 2 to 254 (even only!) */
	u8 scr;	    /* value from 0 to 255 */
};

struct chip_data {
	u32 freq;
	u32 cr0;
	u16 cpsr;
};

static void sf_qspi_flush_rxfifo(struct sf_qspi *s)
{
	while (readw(s->base + SSP_SR) & SSP_SR_MASK_RNE)
		readw(s->base + SSP_DR);
}

static int sf_qspi_wait_not_busy(struct sf_qspi *s)
{
	unsigned long timeout = jiffies + SF_READ_TIMEOUT;

	do {
		if (!(readw(s->base + SSP_SR) & SSP_SR_MASK_BSY))
			return 0;

		cond_resched();
	} while (time_after(timeout, jiffies));

	dev_err(s->dev, "I/O timed out\n");
	return -ETIMEDOUT;
}

static int sf_qspi_wait_rx_not_empty(struct sf_qspi *s)
{
	unsigned long timeout = jiffies + SF_READ_TIMEOUT;

	do {
		if (readw(s->base + SSP_SR) & SSP_SR_MASK_RNE)
			return 0;

		cond_resched();
	} while (time_after(timeout, jiffies));

	dev_err(s->dev, "read timed out\n");
	return -ETIMEDOUT;
}

static int sf_qspi_wait_rxfifo(struct sf_qspi *s)
{
	unsigned long timeout = jiffies + SF_READ_TIMEOUT;

	do {
		if (readw(s->base + SSP_RIS) & SSP_RIS_MASK_RXRIS)
			return 0;

		cond_resched();
	} while (time_after(timeout, jiffies));

	dev_err(s->dev, "read timed out\n");
	return -ETIMEDOUT;
}

static void sf_qspi_enable(struct sf_qspi *s)
{
	/* Enable the SPI hardware */
	writew(SSP_CR1_MASK_SSE, s->base + SSP_CR1);
}

static void sf_qspi_disable(struct sf_qspi *s)
{
	/* Disable the SPI hardware */
	writew(0, s->base + SSP_CR1);
}

static void sf_qspi_xmit(struct sf_qspi *s, unsigned int nbytes, const u8 *out)
{
	while (nbytes--)
		writew(*out++, s->base + SSP_DR);
}

static int sf_qspi_rcv(struct sf_qspi *s, unsigned int nbytes, u8 *in)
{
	int ret, i;

	while (nbytes >= DFLT_THRESH_RX) {
		/* wait for RX FIFO to reach the threshold */
		ret = sf_qspi_wait_rxfifo(s);
		if (ret)
			return ret;

		for (i = 0; i < DFLT_THRESH_RX; i++)
			*in++ = readw(s->base + SSP_DR);

		nbytes -= DFLT_THRESH_RX;
	}

	/* read the remaining data */
	while (nbytes) {
		ret = sf_qspi_wait_rx_not_empty(s);
		if (ret)
			return ret;

		*in++ = readw(s->base + SSP_DR);
		nbytes--;
	}

	return 0;
}

static void sf_qspi_set_param(struct sf_qspi *s, const struct spi_mem_op *op)
{
	unsigned int tx_count = 0, rx_count = 0;
	u8 cmd_io, addr_io, data_io;
	u8 cmd_count, addr_count, ehc_count;

	cmd_io = op->cmd.buswidth == 4 ? 3 : op->cmd.buswidth;
	addr_io = op->addr.buswidth == 4 ? 3 : op->addr.buswidth;
	data_io = op->data.buswidth == 4 ? 3 : op->data.buswidth;

	if (op->data.nbytes) {
		if (op->data.dir == SPI_MEM_DATA_IN)
			rx_count = op->data.nbytes;
		else
			tx_count = op->data.nbytes;
	}
	if (op->addr.nbytes > 3) {
		addr_count = 3;
		ehc_count = 1;
	} else {
		addr_count = op->addr.nbytes;
		ehc_count = 0;
	}
	cmd_count = op->cmd.nbytes;

	writew(FIELD_PREP(EXSPI_CMD2_CMD_IO_MODE, cmd_io) |
		       FIELD_PREP(EXSPI_CMD2_ADDR_IO_MODE, addr_io) |
		       FIELD_PREP(EXSPI_CMD2_DATA_IO_MODE, data_io),
	       s->base + SSP_EXSPI_CMD2);
	writew(FIELD_PREP(EXSPI_CMD1_DUMMY_COUNT, op->dummy.nbytes) |
		       FIELD_PREP(EXSPI_CMD1_RX_COUNT, rx_count),
	       s->base + SSP_EXSPI_CMD1);
	writew(EXSPI_CMD0_VALID |
		       FIELD_PREP(EXSPI_CMD0_CMD_COUNT, op->cmd.nbytes) |
		       FIELD_PREP(EXSPI_CMD0_ADDR_COUNT, addr_count) |
		       FIELD_PREP(EXSPI_CMD0_EHC_COUNT, ehc_count) |
		       FIELD_PREP(EXSPI_CMD0_TX_COUNT, tx_count),
	       s->base + SSP_EXSPI_CMD0);
}

static int sf_qspi_exec_op(struct spi_mem *mem, const struct spi_mem_op *op)
{
	struct sf_qspi *s = spi_controller_get_devdata(mem->spi->controller);
	struct chip_data *chip = spi_get_ctldata(mem->spi);
	unsigned int pops = 0;
	int ret, i, op_len;
	const u8 *tx_buf = NULL;
	u8 *rx_buf = NULL, op_buf[MAX_S_BUF];

	writew(chip->cr0, s->base + SSP_CR0);
	writew(chip->cpsr, s->base + SSP_CPSR);

	if (op->data.nbytes) {
		if (op->data.dir == SPI_MEM_DATA_IN)
			rx_buf = op->data.buf.in;
		else
			tx_buf = op->data.buf.out;
	}
	op_len = op->cmd.nbytes + op->addr.nbytes + op->dummy.nbytes;
	sf_qspi_set_param(s, op);

	op_buf[pops++] = op->cmd.opcode;
	if (op->addr.nbytes) {
		for (i = 0; i < op->addr.nbytes; i++)
			op_buf[pops + i] = op->addr.val >>
					   (8 * (op->addr.nbytes - i - 1));
		pops += op->addr.nbytes;
	}

	sf_qspi_flush_rxfifo(s);
	memset(op_buf + pops, 0xff, op->dummy.nbytes);
	sf_qspi_xmit(s, op_len, op_buf);
	if (tx_buf) {
		sf_qspi_xmit(s, op->data.nbytes, tx_buf);
	}
	sf_qspi_enable(s);
	if (rx_buf)
		ret = sf_qspi_rcv(s, op->data.nbytes, rx_buf);
	else
		ret = sf_qspi_wait_not_busy(s);

	sf_qspi_disable(s);

	return ret;
}

static int sf_qspi_adjust_op_size(struct spi_mem *mem, struct spi_mem_op *op)
{
	u32 nbytes;

	nbytes = op->cmd.nbytes + op->addr.nbytes + op->dummy.nbytes;
	if (nbytes >= SF_SSP_FIFO_DEPTH)
		return -ENOTSUPP;

	if (op->data.dir == SPI_MEM_DATA_IN)
		op->data.nbytes =
			min_t(unsigned int, op->data.nbytes, SF_SSP_FIFO_DEPTH);
	else
		op->data.nbytes = min_t(unsigned int, op->data.nbytes,
					SF_SSP_FIFO_DEPTH - nbytes);

	return 0;
}

static bool sf_qspi_supports_op(struct spi_mem *mem,
				const struct spi_mem_op *op)
{
	if (!spi_mem_default_supports_op(mem, op))
		return false;

	/* dummy buswidth must be the same as addr */
	if (op->addr.nbytes && op->dummy.nbytes &&
	    op->addr.buswidth != op->dummy.buswidth)
		return false;

	return true;
}

static inline u32 spi_rate(u32 rate, u16 cpsdvsr, u16 scr)
{
	return rate / (cpsdvsr * (1 + scr));
}

static int calculate_effective_freq(struct sf_qspi *s, int freq,
				    struct ssp_clock_params *clk_freq)
{
	/* Lets calculate the frequency parameters */
	u16 cpsdvsr = CPSDVR_MIN;
	u32 rate, rate_scaled, max_tclk, min_tclk, scr;
	u32 best_freq = 0, best_cpsdvsr = 0, best_scr = 0, tmp, found = 0;

	rate = clk_get_rate(s->clk);
	/* cpsdvscr = 2 & scr 0 */
	max_tclk = spi_rate(rate, CPSDVR_MIN, SCR_MIN);
	if (freq > max_tclk) {
		dev_warn(
			s->dev,
			"Requested SPI frequency %d Hz is more than maximum: %d Hz\n",
			freq, max_tclk);
		clk_freq->freq = max_tclk;
		clk_freq->cpsdvsr = CPSDVR_MIN;
		clk_freq->scr = SCR_MIN;
		return 0;
	}

	/* cpsdvsr = 254 & scr = 255 */
	min_tclk = spi_rate(rate, CPSDVR_MAX, SCR_MAX);
	if (freq < min_tclk) {
		dev_err(s->dev,
			"Requested SPI frequency %d Hz is less than minimum: %d Hz\n",
			freq, min_tclk);
		return -EINVAL;
	}

	/*
	 * best_freq will give closest possible available rate (<= requested
	 * freq) for all values of scr & cpsdvsr.
	 */
	while ((cpsdvsr <= CPSDVR_MAX) && !found) {
		rate_scaled = rate / cpsdvsr;

		if (rate_scaled < freq)
			break;

		scr = DIV_ROUND_UP(rate_scaled, freq) - 1;
		if (scr > SCR_MAX)
			continue;

		tmp = spi_rate(rate, cpsdvsr, scr);

		/*
		 * If found exact value, mark found and break.
		 * If found more closer value, update and break.
		 */
		if (tmp > best_freq) {
			best_freq = tmp;
			best_cpsdvsr = cpsdvsr;
			best_scr = scr;

			if (tmp == freq)
				found = 1;
		}

		cpsdvsr += 2;
	}

	clk_freq->freq = best_freq;
	clk_freq->cpsdvsr = (u8) (best_cpsdvsr & 0xFF);
	clk_freq->scr = (u8) (best_scr & 0xFF);
	dev_dbg(s->dev,
		"SSP Target Frequency is: %u, Effective Frequency is %u\n",
		freq, best_freq);
	dev_dbg(s->dev, "SSP cpsdvsr = %d, scr = %d\n",
		clk_freq->cpsdvsr, clk_freq->scr);

	return 0;
}

static int sf_qspi_setup(struct spi_device *spi)
{
	struct sf_qspi *s = spi_controller_get_devdata(spi->controller);
	struct ssp_clock_params clk_freq = { .cpsdvsr = 0, .scr = 0 };
	struct chip_data *chip;
	int ret = 0;
	u16 cr0 = 0;

	if (!spi->max_speed_hz)
		return -EINVAL;

	ret = calculate_effective_freq(s, spi->max_speed_hz, &clk_freq);
	if (ret < 0)
		return ret;

	chip = kzalloc(sizeof(struct chip_data), GFP_KERNEL);
	if (!chip)
		return -ENOMEM;

	cr0 = SSP_CR0_BIT_MODE(8);
	cr0 |= clk_freq.scr << 8;
	/*set module*/
	cr0 &= ~(SSP_CR0_SPH | SSP_CR0_SPO);
	if (spi->mode & SPI_CPHA)
		cr0 |= SSP_CR0_SPH;
	if (spi->mode & SPI_CPOL)
		cr0 |= SSP_CR0_SPO;
	cr0 |= SSP_CR0_EXSPI_FRAME;

	chip->freq = clk_freq.freq;
	chip->cr0 = cr0;
	chip->cpsr = clk_freq.cpsdvsr;

	spi_set_ctldata(spi, chip);
	return 0;
}

static void sf_qspi_cleanup(struct spi_device *spi)
{
	struct chip_data *chip = spi_get_ctldata(spi);

	spi_set_ctldata(spi, NULL);
	kfree(chip);
}

static const struct spi_controller_mem_ops sf_qspi_mem_ops = {
	.supports_op = sf_qspi_supports_op,
	.adjust_op_size = sf_qspi_adjust_op_size,
	.exec_op = sf_qspi_exec_op,
};

static int sf_qspi_probe(struct platform_device *pdev)
{
	struct spi_controller *controller;
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	struct sf_qspi *s;
	int ret;

	controller = devm_spi_alloc_host(&pdev->dev, sizeof(*s));
	if (!controller)
		return -ENOMEM;
	controller->mode_bits = SPI_RX_DUAL | SPI_RX_QUAD | SPI_TX_DUAL |
			    SPI_TX_QUAD;
	s = spi_controller_get_devdata(controller);
	s->dev = dev;
	platform_set_drvdata(pdev, s);

	s->base = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(s->base))
		return dev_err_probe(dev, PTR_ERR(s->base),
				     "failed to remap memory resources.\n");

	s->clk = devm_clk_get_enabled(dev, "sspclk");
	if (IS_ERR(s->clk))
		return dev_err_probe(dev, PTR_ERR(s->clk),
				     "failed to get and enable sspclk.\n");

	s->apbclk = devm_clk_get_enabled(dev, "apb_pclk");
	if (IS_ERR(s->apbclk))
		return dev_err_probe(dev, PTR_ERR(s->apbclk),
				     "failed to get and enable apb_pclk.\n");

	controller->cleanup = sf_qspi_cleanup;
	controller->setup = sf_qspi_setup;
	controller->use_gpio_descriptors = true;
	controller->mem_ops = &sf_qspi_mem_ops;
	controller->dev.of_node = np;

	writew(FIELD_PREP(SSP_FIFO_LEVEL_RX, DFLT_THRESH_RX) |
	       FIELD_PREP(SSP_FIFO_LEVEL_TX, DFLT_THRESH_TX),
	       s->base + SSP_FIFO_LEVEL);

	ret = devm_spi_register_controller(dev, controller);
	if (ret)
		return dev_err_probe(dev, ret,
				     "failed to register controller.\n");

	return 0;
}

static const struct of_device_id sf_qspi_ids[] = {
	{.compatible = "siflower,sf21-qspi"},
	{},
};
MODULE_DEVICE_TABLE(of, sf_qspi_ids);

static struct platform_driver sf_qspi_driver = {
	.driver = {
		.name = "sf21_qspi",
		.of_match_table = sf_qspi_ids,
	},
	.probe = sf_qspi_probe,
};
module_platform_driver(sf_qspi_driver);
