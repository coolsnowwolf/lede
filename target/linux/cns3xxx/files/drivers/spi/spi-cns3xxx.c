/*******************************************************************************
 *
 *  CNS3XXX SPI controller driver (master mode only)
 *
 *  Copyright (c) 2008 Cavium Networks
 *  Copyright 2011 Gateworks Corporation
 *		   Chris Lang <clang@gateworks.com>
 *
 *  This file is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License, Version 2, as
 *  published by the Free Software Foundation.
 *
 *  This file is distributed in the hope that it will be useful,
 *  but AS-IS and WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, TITLE, or
 *  NONINFRINGEMENT.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this file; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA or
 *  visit http://www.gnu.org/licenses/.
 *
 *  This file may also be available under a different license from Cavium.
 *  Contact Cavium Networks for more information
 *
 ******************************************************************************/

#include <linux/init.h>
#include <linux/spinlock.h>
#include <linux/workqueue.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/platform_device.h>

#include <linux/spi/spi.h>
#include <linux/spi/spi_bitbang.h>
#include <linux/mtd/partitions.h>
#include <linux/dma-mapping.h>
#include <linux/slab.h>

#include <asm/io.h>
#include <asm/memory.h>
#include <asm/dma.h>
#include <asm/delay.h>
#include <linux/module.h>

/*
 * define access macros
 */
#define SPI_MEM_MAP_VALUE(reg_offset)		(*((u32 volatile *)(hw->base + reg_offset)))

#define SPI_CONFIGURATION_REG			SPI_MEM_MAP_VALUE(0x00)
#define SPI_SERVICE_STATUS_REG			SPI_MEM_MAP_VALUE(0x04)
#define SPI_BIT_RATE_CONTROL_REG		SPI_MEM_MAP_VALUE(0x08)
#define SPI_TRANSMIT_CONTROL_REG		SPI_MEM_MAP_VALUE(0x0C)
#define SPI_TRANSMIT_BUFFER_REG			SPI_MEM_MAP_VALUE(0x10)
#define SPI_RECEIVE_CONTROL_REG			SPI_MEM_MAP_VALUE(0x14)
#define SPI_RECEIVE_BUFFER_REG			SPI_MEM_MAP_VALUE(0x18)
#define SPI_FIFO_TRANSMIT_CONFIG_REG		SPI_MEM_MAP_VALUE(0x1C)
#define SPI_FIFO_TRANSMIT_CONTROL_REG		SPI_MEM_MAP_VALUE(0x20)
#define SPI_FIFO_RECEIVE_CONFIG_REG		SPI_MEM_MAP_VALUE(0x24)
#define SPI_INTERRUPT_STATUS_REG		SPI_MEM_MAP_VALUE(0x28)
#define SPI_INTERRUPT_ENABLE_REG		SPI_MEM_MAP_VALUE(0x2C)

#define SPI_TRANSMIT_BUFFER_REG_ADDR		(CNS3XXX_SSP_BASE +0x10)
#define SPI_RECEIVE_BUFFER_REG_ADDR		(CNS3XXX_SSP_BASE +0x18)

/* Structure for SPI controller of CNS3XXX SOCs */
struct cns3xxx_spi {
	/* bitbang has to be first */
	struct spi_bitbang bitbang;
	struct completion done;
	wait_queue_head_t wait;

	int len;
	int count;
	int last_in_message_list;

	/* data buffers */
	const unsigned char *tx;
	unsigned char *rx;

	void __iomem *base;
	struct spi_master *master;
	struct platform_device *pdev;
	struct device *dev;
};

static inline u8 cns3xxx_spi_bus_idle(struct cns3xxx_spi *hw)
{
	return ((SPI_SERVICE_STATUS_REG & 0x1) ? 0 : 1);
}

static inline u8 cns3xxx_spi_tx_buffer_empty(struct cns3xxx_spi *hw)
{
	return ((SPI_INTERRUPT_STATUS_REG & (0x1 << 3)) ? 1 : 0);
}

static inline u8 cns3xxx_spi_rx_buffer_full(struct cns3xxx_spi *hw)
{
	return ((SPI_INTERRUPT_STATUS_REG & (0x1 << 2)) ? 1 : 0);
}

u8 cns3xxx_spi_tx_rx(struct cns3xxx_spi *hw, u8 tx_channel, u8 tx_eof,
		     u32 tx_data, u32 * rx_data)
{
	u8 rx_channel;
	u8 rx_eof;

	while (!cns3xxx_spi_bus_idle(hw)) ;	// do nothing

	while (!cns3xxx_spi_tx_buffer_empty(hw)) ;	// do nothing

	SPI_TRANSMIT_CONTROL_REG &= ~(0x7);
	SPI_TRANSMIT_CONTROL_REG |= (tx_channel & 0x3) | ((tx_eof & 0x1) << 2);

	SPI_TRANSMIT_BUFFER_REG = tx_data;

	while (!cns3xxx_spi_rx_buffer_full(hw)) ;	// do nothing

	rx_channel = SPI_RECEIVE_CONTROL_REG & 0x3;
	rx_eof = (SPI_RECEIVE_CONTROL_REG & (0x1 << 2)) ? 1 : 0;

	*rx_data = SPI_RECEIVE_BUFFER_REG;

	if ((tx_channel != rx_channel) || (tx_eof != rx_eof)) {
		return 0;
	} else {
		return 1;
	}
}

u8 cns3xxx_spi_tx(struct cns3xxx_spi *hw, u8 tx_channel, u8 tx_eof, u32 tx_data)
{

        while (!cns3xxx_spi_bus_idle(hw)) ;       // do nothing

        while (!cns3xxx_spi_tx_buffer_empty(hw)) ;        // do nothing

        SPI_TRANSMIT_CONTROL_REG &= ~(0x7);
        SPI_TRANSMIT_CONTROL_REG |= (tx_channel & 0x3) | ((tx_eof & 0x1) << 2);

        SPI_TRANSMIT_BUFFER_REG = tx_data;

        return 1;
}

static inline struct cns3xxx_spi *to_hw(struct spi_device *sdev)
{
	return spi_master_get_devdata(sdev->master);
}

static int cns3xxx_spi_setup_transfer(struct spi_device *spi,
				      struct spi_transfer *t)
{
	return 0;
}

static void cns3xxx_spi_chipselect(struct spi_device *spi, int value)
{
	struct cns3xxx_spi *hw = to_hw(spi);
	unsigned int spi_config;

	switch (value) {
	case BITBANG_CS_INACTIVE:
		break;

	case BITBANG_CS_ACTIVE:
		spi_config = SPI_CONFIGURATION_REG;

		if (spi->mode & SPI_CPHA)
			spi_config |= (0x1 << 13);
		else
			spi_config &= ~(0x1 << 13);

		if (spi->mode & SPI_CPOL)
			spi_config |= (0x1 << 14);
		else
			spi_config &= ~(0x1 << 14);

		/* write new configration */
		SPI_CONFIGURATION_REG = spi_config;

		SPI_TRANSMIT_CONTROL_REG &= ~(0x7);
		SPI_TRANSMIT_CONTROL_REG |= (spi->chip_select & 0x3);

		break;
	}
}

static int cns3xxx_spi_setup(struct spi_device *spi)
{
	if (!spi->bits_per_word)
		spi->bits_per_word = 8;

	return 0;
}

static int cns3xxx_spi_txrx(struct spi_device *spi, struct spi_transfer *t)
{
	struct cns3xxx_spi *hw = to_hw(spi);

	dev_dbg(&spi->dev, "txrx: tx %p, rx %p, len %d\n", t->tx_buf, t->rx_buf,
		t->len);

	hw->tx = t->tx_buf;
	hw->rx = t->rx_buf;
	hw->len = t->len;
	hw->count = 0;
	hw->last_in_message_list = t->last_in_message_list;

	init_completion(&hw->done);

	if (hw->tx) {
		int i;
		u32 rx_data;
		for (i = 0; i < (hw->len - 1); i++) {
			dev_dbg(&spi->dev,
				"[SPI_CNS3XXX_DEBUG] hw->tx[%02d]: 0x%02x\n", i,
				hw->tx[i]);
			cns3xxx_spi_tx_rx(hw, spi->chip_select, 0, hw->tx[i],
					  &rx_data);
			if (hw->rx) {
				hw->rx[i] = rx_data;
				dev_dbg(&spi->dev,
					"[SPI_CNS3XXX_DEBUG] hw->rx[%02d]: 0x%02x\n",
					i, hw->rx[i]);
			}
		}

		if (t->last_in_message_list) {
			cns3xxx_spi_tx_rx(hw, spi->chip_select, 1, hw->tx[i],
					  &rx_data);
			if (hw->rx) {
				hw->rx[i] = rx_data;
				dev_dbg(&spi->dev,
					"[SPI_CNS3XXX_DEBUG] hw->rx[%02d]: 0x%02x\n",
					i, hw->rx[i]);
			}
		} else {
			cns3xxx_spi_tx_rx(hw, spi->chip_select, 0, hw->tx[i],
					  &rx_data);
		}
		goto done;
	}

	if (hw->rx) {
		int i;
		u32 rx_data;
		for (i = 0; i < (hw->len - 1); i++) {
			cns3xxx_spi_tx_rx(hw, spi->chip_select, 0, 0xff, &rx_data);
			hw->rx[i] = rx_data;
			dev_dbg(&spi->dev,
				"[SPI_CNS3XXX_DEBUG] hw->rx[%02d]: 0x%02x\n", i,
				hw->rx[i]);
		}

		if (t->last_in_message_list) {
			cns3xxx_spi_tx_rx(hw, spi->chip_select, 1, 0xff, &rx_data);
		} else {
			cns3xxx_spi_tx_rx(hw, spi->chip_select, 0, 0xff, &rx_data);
		}
		hw->rx[i] = rx_data;
		dev_dbg(&spi->dev, "[SPI_CNS3XXX_DEBUG] hw->rx[%02d]: 0x%02x\n",
			i, hw->rx[i]);
	}
done:
	return hw->len;
}

static void __init cns3xxx_spi_initial(struct cns3xxx_spi *hw)
{
	SPI_CONFIGURATION_REG = (((0x0 & 0x3) << 0) |	/* 8bits shift length */
				 (0x0 << 9) |	/* SPI mode */
				 (0x0 << 10) |	/* disable FIFO */
				 (0x1 << 11) |	/* SPI master mode */
				 (0x0 << 12) |	/* disable SPI loopback mode */
				 (0x1 << 13) |	/* clock phase */
				 (0x1 << 14) |	/* clock polarity */
				 (0x0 << 24) |	/* disable - SPI data swap */
				 (0x1 << 29) |	/* enable - 2IO Read mode */
				 (0x0 << 30) |	/* disable - SPI high speed read for system boot up */
				 (0x0 << 31));	/* disable - SPI */

	/* Set SPI bit rate PCLK/2 */
	SPI_BIT_RATE_CONTROL_REG = 0x1;

	/* Set SPI Tx channel 0 */
	SPI_TRANSMIT_CONTROL_REG = 0x0;

	/* Set Tx FIFO Threshold, Tx FIFO has 2 words */
	SPI_FIFO_TRANSMIT_CONFIG_REG &= ~(0x03 << 4);
	SPI_FIFO_TRANSMIT_CONFIG_REG |= ((0x0 & 0x03) << 4);

	/* Set Rx FIFO Threshold, Rx FIFO has 2 words */
	SPI_FIFO_RECEIVE_CONFIG_REG &= ~(0x03 << 4);
	SPI_FIFO_RECEIVE_CONFIG_REG |= ((0x0 & 0x03) << 4);

	/* Disable all interrupt */
	SPI_INTERRUPT_ENABLE_REG = 0x0;

	/* Clear spurious interrupt sources */
	SPI_INTERRUPT_STATUS_REG = (0x0F << 4);

	/* Enable SPI */
	SPI_CONFIGURATION_REG |= (0x1 << 31);

	return;
}

static int cns3xxx_spi_probe(struct platform_device *pdev)
{
	struct spi_master *master;
	struct cns3xxx_spi *hw;
	struct resource *res;
	int err = 0;

	printk("%s: setup CNS3XXX SPI Controller\n", __FUNCTION__);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res)
		return -ENODEV;

	/* Allocate master with space for cns3xxx_spi */
	master = spi_alloc_master(&pdev->dev, sizeof(struct cns3xxx_spi));
	if (master == NULL) {
		dev_err(&pdev->dev, "No memory for spi_master\n");
		err = -ENOMEM;
		goto err_nomem;
	}

	hw = spi_master_get_devdata(master);
	memset(hw, 0, sizeof(struct cns3xxx_spi));

	hw->master = spi_master_get(master);
	hw->dev = &pdev->dev;

	hw->base = devm_ioremap_resource(hw->dev, res);
	if (IS_ERR(hw->base)) {
		dev_err(hw->dev, "Unable to map registers\n");
		err = PTR_ERR(hw->base);
		goto err_register;
	}

	platform_set_drvdata(pdev, hw);
	init_completion(&hw->done);

	/* setup the master state. */

	master->num_chipselect = 4;
	master->bus_num = 1;

	/* setup the state for the bitbang driver */

	hw->bitbang.master = hw->master;
	hw->bitbang.setup_transfer = cns3xxx_spi_setup_transfer;
	hw->bitbang.chipselect = cns3xxx_spi_chipselect;
	hw->bitbang.txrx_bufs = cns3xxx_spi_txrx;
	hw->bitbang.master->setup = cns3xxx_spi_setup;

	dev_dbg(hw->dev, "bitbang at %p\n", &hw->bitbang);

	/* SPI controller initializations */
	cns3xxx_spi_initial(hw);

	/* register SPI controller */

	err = spi_bitbang_start(&hw->bitbang);
	if (err) {
		dev_err(&pdev->dev, "Failed to register SPI master\n");
		goto err_register;
	}

	return 0;

err_register:
	spi_master_put(hw->master);;

err_nomem:
	return err;
}

static int cns3xxx_spi_remove(struct platform_device *dev)
{
	struct cns3xxx_spi *hw = platform_get_drvdata(dev);

	platform_set_drvdata(dev, NULL);

	spi_unregister_master(hw->master);

	spi_master_put(hw->master);
	return 0;
}

#ifdef CONFIG_PM

static int cns3xxx_spi_suspend(struct platform_device *pdev, pm_message_t msg)
{
	struct cns3xxx_spi *hw = platform_get_drvdata(pdev);

	return 0;
}

static int cns3xxx_spi_resume(struct platform_device *pdev)
{
	struct cns3xxx_spi *hw = platform_get_drvdata(pdev);

	return 0;
}

#else
#define cns3xxx_spi_suspend	NULL
#define cns3xxx_spi_resume	NULL
#endif

static struct platform_driver cns3xxx_spi_driver = {
	.probe		= cns3xxx_spi_probe,
	.remove		= cns3xxx_spi_remove,
	.suspend	= cns3xxx_spi_suspend,
	.resume		= cns3xxx_spi_resume,
	.driver		= {
		.name = "cns3xxx_spi",
		.owner = THIS_MODULE,
	},
};

static int __init cns3xxx_spi_init(void)
{
	return platform_driver_register(&cns3xxx_spi_driver);
}

static void __exit cns3xxx_spi_exit(void)
{
	platform_driver_unregister(&cns3xxx_spi_driver);
}

module_init(cns3xxx_spi_init);
module_exit(cns3xxx_spi_exit);

MODULE_AUTHOR("Cavium Networks");
MODULE_DESCRIPTION("CNS3XXX SPI Controller Driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:cns3xxx_spi");

EXPORT_SYMBOL_GPL(cns3xxx_spi_tx_rx);
