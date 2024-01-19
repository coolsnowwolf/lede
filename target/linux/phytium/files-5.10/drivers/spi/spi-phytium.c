 // SPDX-License-Identifier: GPL-2.0
/*
 * Phytium SPI core controller driver.
 *
 * Copyright (c) 2019-2023 Phytium Technology Co., Ltd.
 */
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/gpio.h>
#include <linux/highmem.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/spi/spi.h>
#include <linux/scatterlist.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/of_platform.h>
#include <linux/property.h>
#include <linux/acpi.h>
#include "spi-phytium.h"

struct phytium_spi_chip {
	u8 poll_mode;
	u8 type;
	void (*cs_control)(u32 command);
};

struct chip_data {
	u8 cs;
	u8 tmode;
	u8 type;

	u8 poll_mode;

	u16 clk_div;
	u32 speed_hz;
	void (*cs_control)(u32 command);
};

static void phytium_spi_set_cs(struct spi_device *spi, bool enable)
{
	struct phytium_spi *fts = spi_master_get_devdata(spi->master);
	struct chip_data *chip = spi_get_ctldata(spi);
	u32 origin;

	if (chip && chip->cs_control)
		chip->cs_control(!enable);

	if (!enable) {
		phytium_writel(fts, SER, BIT(spi->chip_select));
		if (fts->global_cs) {
			origin = phytium_readl(fts, GCSR);
			phytium_writel(fts, GCSR, origin | (1 << spi->chip_select));
		}
	} else {
		if (fts->global_cs) {
			origin = phytium_readl(fts, GCSR);
			phytium_writel(fts, GCSR, origin & ~(1 << spi->chip_select));
		}
	}
}

static inline u32 tx_max(struct phytium_spi *fts)
{
	u32 tx_left, tx_room, rxtx_gap;

	tx_left = (fts->tx_end - fts->tx) / fts->n_bytes;
	tx_room = fts->fifo_len - phytium_readl(fts, TXFLR);

	rxtx_gap =  ((fts->rx_end - fts->rx) - (fts->tx_end - fts->tx))
			/ fts->n_bytes;

	return min3(tx_left, tx_room, (u32) (fts->fifo_len - rxtx_gap));
}

static inline u32 rx_max(struct phytium_spi *fts)
{
	u32 rx_left = (fts->rx_end - fts->rx) / fts->n_bytes;

	return min_t(u32, rx_left, phytium_readl(fts, RXFLR));
}

static void phytium_writer(struct phytium_spi *fts)
{
	u32 max = tx_max(fts);
	u16 txw = 0;

	while (max--) {
		if (fts->tx_end - fts->len) {
			if (fts->n_bytes == 1)
				txw = *(u8 *)(fts->tx);
			else
				txw = *(u16 *)(fts->tx);
		}
		phytium_write_io_reg(fts, DR, txw);
		fts->tx += fts->n_bytes;
	}
}

static void phytium_reader(struct phytium_spi *fts)
{
	u32 max = rx_max(fts);
	u16 rxw;

	while (max--) {
		rxw = phytium_read_io_reg(fts, DR);
		if (fts->rx_end - fts->len) {
			if (fts->n_bytes == 1)
				*(u8 *)(fts->rx) = rxw;
			else
				*(u16 *)(fts->rx) = rxw;
		}
		fts->rx += fts->n_bytes;
	}
}

int phytium_spi_check_status(struct phytium_spi *fts, bool raw)
{
	u32 irq_status;
	int ret = 0;

	if (raw)
		irq_status = phytium_readl(fts, RISR);
	else
		irq_status = phytium_readl(fts, ISR);

	if (irq_status & INT_RXOI) {
		dev_err(&fts->master->dev, "RX FIFO overflow detected\n");
		ret = -EIO;
	}

	if (irq_status & INT_RXUI) {
		dev_err(&fts->master->dev, "RX FIFO underflow detected\n");
		ret = -EIO;
	}

	if (irq_status & INT_TXOI) {
		dev_err(&fts->master->dev, "TX FIFO overflow detected\n");
		ret = -EIO;
	}

	/* Generically handle the erroneous situation */
	if (ret) {
		spi_reset_chip(fts);
		if (fts->master->cur_msg)
			fts->master->cur_msg->status = ret;
	}

	return ret;
}
EXPORT_SYMBOL_GPL(phytium_spi_check_status);

static void int_error_stop(struct phytium_spi *fts, const char *msg)
{
	spi_reset_chip(fts);

	dev_err(&fts->master->dev, "%s\n", msg);
	fts->master->cur_msg->status = -EIO;
	spi_finalize_current_transfer(fts->master);
}

static irqreturn_t interrupt_transfer(struct phytium_spi *fts)
{
	u16 irq_status = phytium_readl(fts, ISR);

	if (irq_status & (INT_TXOI | INT_RXOI | INT_RXUI)) {
		phytium_readl(fts, ICR);
		int_error_stop(fts, "interrupt_transfer: fifo overrun/underrun");
		return IRQ_HANDLED;
	}

	phytium_reader(fts);
	if (fts->rx_end == fts->rx) {
		spi_mask_intr(fts, INT_TXEI);
		spi_finalize_current_transfer(fts->master);
		return IRQ_HANDLED;
	}
	if (irq_status & INT_TXEI) {
		spi_mask_intr(fts, INT_TXEI);
		phytium_writer(fts);
		spi_umask_intr(fts, INT_TXEI);
	}

	return IRQ_HANDLED;
}

static irqreturn_t phytium_spi_irq(int irq, void *dev_id)
{
	struct spi_master *master = dev_id;
	struct phytium_spi *fts = spi_master_get_devdata(master);
	u16 irq_status = phytium_readl(fts, ISR) & 0x3f;

	if (!irq_status)
		return IRQ_NONE;

	if (!master->cur_msg) {
		spi_mask_intr(fts, INT_TXEI);
		return IRQ_HANDLED;
	}

	if (fts->transfer_handler)
		return fts->transfer_handler(fts);
	else
		return IRQ_HANDLED;
}

static int poll_transfer(struct phytium_spi *fts)
{
	do {
		phytium_writer(fts);
		phytium_reader(fts);
		cpu_relax();
	} while (fts->rx_end > fts->rx);

	return 0;
}

static int phytium_spi_transfer_one(struct spi_master *master,
		struct spi_device *spi, struct spi_transfer *transfer)
{
	struct phytium_spi *fts = spi_master_get_devdata(master);
	struct chip_data *chip = spi_get_ctldata(spi);
	u8 imask = 0;
	u16 txlevel = 0;
	u16 clk_div;
	u32 cr0;
	int ret = 0;

	fts->dma_mapped = 0;
	fts->tx = (void *)transfer->tx_buf;
	fts->tx_end = fts->tx + transfer->len;
	fts->rx = transfer->rx_buf;
	fts->rx_end = fts->rx + transfer->len;
	fts->len = transfer->len;

	spi_enable_chip(fts, 0);

	if (transfer->speed_hz != fts->current_freq) {
		if (transfer->speed_hz != chip->speed_hz) {
			clk_div = (fts->max_freq / transfer->speed_hz + 1) &
				0xfffe;

			chip->speed_hz = transfer->speed_hz;
			chip->clk_div = clk_div;
		}
		fts->current_freq = transfer->speed_hz;
		spi_set_clk(fts, chip->clk_div);
	}

	if (transfer->bits_per_word == 8) {
		fts->n_bytes = 1;
	} else if (transfer->bits_per_word == 16) {
		fts->n_bytes = 2;
	} else {
		return -EINVAL;
	}

	cr0 = (transfer->bits_per_word - 1)
		| (chip->type << FRF_OFFSET)
		| (spi->mode << MODE_OFFSET)
		| (chip->tmode << TMOD_OFFSET);

	if (chip->cs_control) {
		if (fts->rx && fts->tx)
			chip->tmode = TMOD_TR;
		else if (fts->rx)
			chip->tmode = TMOD_RO;
		else
			chip->tmode = TMOD_TO;

		cr0 &= ~TMOD_MASK;
		cr0 |= (chip->tmode << TMOD_OFFSET);
	}

	phytium_writel(fts, CTRLR0, cr0);

	/* check if current transfer is a DMA transcation */
	if (master->can_dma && master->can_dma(master, spi, transfer))
		fts->dma_mapped = master->cur_msg_mapped;

	spi_mask_intr(fts, 0xff);

	/* DMA setup */
	if (fts->dma_mapped) {
		ret = fts->dma_ops->dma_setup(fts, transfer);
		if (ret)
			return ret;
	}

	/* interrupt transfer mode setup */
	if (!chip->poll_mode && !fts->dma_mapped) {
		txlevel = min_t(u16, fts->fifo_len / 2, fts->len / fts->n_bytes);
		phytium_writel(fts, TXFLTR, txlevel);

		imask |= INT_TXEI | INT_TXOI |
			 INT_RXUI | INT_RXOI;
		spi_umask_intr(fts, imask);

		fts->transfer_handler = interrupt_transfer;
	}

	spi_enable_chip(fts, 1);

	if (fts->dma_mapped)
		return fts->dma_ops->dma_transfer(fts, transfer);

	if (chip->poll_mode)
		return poll_transfer(fts);

	return 1;
}

static void phytium_spi_handle_err(struct spi_master *master,
		struct spi_message *msg)
{
	struct phytium_spi *fts = spi_master_get_devdata(master);

	if (fts->dma_mapped)
		fts->dma_ops->dma_stop(fts);

	spi_reset_chip(fts);
}

static int phytium_spi_setup(struct spi_device *spi)
{
	struct phytium_spi_chip *chip_info = NULL;
	struct chip_data *chip;
	struct spi_master *master = spi->master;
	struct phytium_spi *fts = spi_master_get_devdata(master);
	int ret;
	u32 cr0;

	spi_enable_chip(fts, 0);

	chip = spi_get_ctldata(spi);
	if (!chip) {
		chip = kzalloc(sizeof(struct chip_data), GFP_KERNEL);
		if (!chip)
			return -ENOMEM;
		spi_set_ctldata(spi, chip);
	}

	chip_info = spi->controller_data;

	if (chip_info) {
		if (chip_info->cs_control)
			chip->cs_control = chip_info->cs_control;

		chip->poll_mode = chip_info->poll_mode;
		chip->type = chip_info->type;
	}

	chip->tmode = 0;

	cr0 = (spi->bits_per_word - 1) | (chip->type << FRF_OFFSET) |
	      (spi->mode << MODE_OFFSET) | (chip->tmode << TMOD_OFFSET);

	phytium_writel(fts, CTRLR0, cr0);

	if (gpio_is_valid(spi->cs_gpio)) {
		ret = gpio_direction_output(spi->cs_gpio,
				!(spi->mode & SPI_CS_HIGH));
		if (ret)
			return ret;
	}

	spi_enable_chip(fts, 1);

	return 0;
}

static void phytium_spi_cleanup(struct spi_device *spi)
{
	struct chip_data *chip = spi_get_ctldata(spi);

	kfree(chip);
	spi_set_ctldata(spi, NULL);
}

static void spi_hw_init(struct device *dev, struct phytium_spi *fts)
{
	spi_reset_chip(fts);

	if (!fts->fifo_len) {
		u32 fifo;

		for (fifo = 1; fifo < 256; fifo++) {
			phytium_writel(fts, TXFLTR, fifo);
			if (fifo != phytium_readl(fts, TXFLTR))
				break;
		}
		phytium_writel(fts, TXFLTR, 0);

		fts->fifo_len = (fifo == 1) ? 0 : fifo;
		dev_dbg(dev, "Detected FIFO size: %u bytes\n", fts->fifo_len);
	}
}

int phytium_spi_add_host(struct device *dev, struct phytium_spi *fts)
{
	struct spi_master *master;
	int ret;

	BUG_ON(fts == NULL);

	master = spi_alloc_master(dev, 0);
	if (!master)
		return -ENOMEM;

	fts->master = master;
	fts->dma_addr = (dma_addr_t)(fts->paddr + DR);
	snprintf(fts->name, sizeof(fts->name), "phytium_spi%d", fts->bus_num);

	ret = request_irq(fts->irq, phytium_spi_irq, IRQF_SHARED, fts->name, master);
	if (ret < 0) {
		dev_err(dev, "can not get IRQ\n");
		goto err_free_master;
	}

	master->mode_bits = SPI_CPOL | SPI_CPHA | SPI_LOOP;
	master->bits_per_word_mask = SPI_BPW_MASK(8) | SPI_BPW_MASK(16);
	master->bus_num = fts->bus_num;
	master->num_chipselect = fts->num_cs;
	master->setup = phytium_spi_setup;
	master->cleanup = phytium_spi_cleanup;
	master->set_cs = phytium_spi_set_cs;
	master->transfer_one = phytium_spi_transfer_one;
	master->handle_err = phytium_spi_handle_err;
	master->max_speed_hz = fts->max_freq;
	master->dev.of_node = dev->of_node;
	master->dev.fwnode = dev->fwnode;
	master->flags = SPI_MASTER_GPIO_SS;
	master->cs_gpios = fts->cs;

	spi_hw_init(dev, fts);


	if (fts->dma_ops && fts->dma_ops->dma_init) {
		ret = fts->dma_ops->dma_init(dev, fts);
		if (ret) {
			dev_warn(dev, "DMA init failed\n");
		} else {
			master->can_dma = fts->dma_ops->can_dma;
			master->flags |= SPI_CONTROLLER_MUST_TX;
		}
	}

	spi_master_set_devdata(master, fts);
	ret = devm_spi_register_master(dev, master);
	if (ret) {
		dev_err(&master->dev, "problem registering spi master\n");
		goto err_exit;
	}

	return 0;

err_exit:
	if (fts->dma_ops && fts->dma_ops->dma_exit)
		fts->dma_ops->dma_exit(fts);
	spi_enable_chip(fts, 0);
	free_irq(fts->irq, master);
err_free_master:
	spi_master_put(master);
	return ret;
}
EXPORT_SYMBOL_GPL(phytium_spi_add_host);

void phytium_spi_remove_host(struct phytium_spi *fts)
{
	if (fts->dma_ops && fts->dma_ops->dma_exit)
		fts->dma_ops->dma_exit(fts);
	spi_shutdown_chip(fts);

	free_irq(fts->irq, fts->master);
}
EXPORT_SYMBOL_GPL(phytium_spi_remove_host);

int phytium_spi_suspend_host(struct phytium_spi *fts)
{
	int ret;

	ret = spi_controller_suspend(fts->master);
	if (ret)
		return ret;

	spi_shutdown_chip(fts);
	return 0;
}
EXPORT_SYMBOL_GPL(phytium_spi_suspend_host);

int phytium_spi_resume_host(struct phytium_spi *fts)
{
	int ret;

	spi_hw_init(&fts->master->dev, fts);
	ret = spi_controller_resume(fts->master);
	if (ret)
		dev_err(&fts->master->dev, "fail to start queue (%d)\n", ret);
	return ret;
}
EXPORT_SYMBOL_GPL(phytium_spi_resume_host);

MODULE_AUTHOR("Zhu Mingshuai <zhumingshuai@phytium.com.cn>");
MODULE_AUTHOR("Chen Baozi <chenbaozi@phytium.com.cn>");
MODULE_DESCRIPTION("Driver for Phytium SPI controller core");
MODULE_LICENSE("GPL v2");
