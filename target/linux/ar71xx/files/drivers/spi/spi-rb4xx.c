/*
 * SPI controller driver for the Mikrotik RB4xx boards
 *
 * Copyright (C) 2010 Gabor Juhos <juhosg@openwrt.org>
 *
 * This file was based on the patches for Linux 2.6.27.39 published by
 * MikroTik for their RouterBoard 4xx series devices.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/clk.h>
#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/spinlock.h>
#include <linux/workqueue.h>
#include <linux/platform_device.h>
#include <linux/spi/spi.h>

#include <asm/mach-ath79/ar71xx_regs.h>
#include <asm/mach-ath79/ath79.h>

#define DRV_NAME	"rb4xx-spi"
#define DRV_DESC	"Mikrotik RB4xx SPI controller driver"
#define DRV_VERSION	"0.1.0"

#define SPI_CTRL_FASTEST	0x40
#define SPI_FLASH_HZ		33333334
#define SPI_CPLD_HZ		33333334

#define CPLD_CMD_READ_FAST	0x0b

#undef RB4XX_SPI_DEBUG

struct rb4xx_spi {
	void __iomem		*base;
	struct spi_master	*master;

	unsigned		spi_ctrl_flash;
	unsigned		spi_ctrl_fread;

	struct clk		*ahb_clk;
	unsigned long		ahb_freq;

	spinlock_t		lock;
	struct list_head	queue;
	int			busy:1;
	int			cs_wait;
};

static unsigned spi_clk_low = AR71XX_SPI_IOC_CS1;

#ifdef RB4XX_SPI_DEBUG
static inline void do_spi_delay(void)
{
	ndelay(20000);
}
#else
static inline void do_spi_delay(void) { }
#endif

static inline void do_spi_init(struct spi_device *spi)
{
	unsigned cs = AR71XX_SPI_IOC_CS0 | AR71XX_SPI_IOC_CS1;

	if (!(spi->mode & SPI_CS_HIGH))
		cs ^= (spi->chip_select == 2) ? AR71XX_SPI_IOC_CS1 :
						AR71XX_SPI_IOC_CS0;

	spi_clk_low = cs;
}

static inline void do_spi_finish(void __iomem *base)
{
	do_spi_delay();
	__raw_writel(AR71XX_SPI_IOC_CS0 | AR71XX_SPI_IOC_CS1,
		     base + AR71XX_SPI_REG_IOC);
}

static inline void do_spi_clk(void __iomem *base, int bit)
{
	unsigned bval = spi_clk_low | ((bit & 1) ? AR71XX_SPI_IOC_DO : 0);

	do_spi_delay();
	__raw_writel(bval, base + AR71XX_SPI_REG_IOC);
	do_spi_delay();
	__raw_writel(bval | AR71XX_SPI_IOC_CLK, base + AR71XX_SPI_REG_IOC);
}

static void do_spi_byte(void __iomem *base, unsigned char byte)
{
	do_spi_clk(base, byte >> 7);
	do_spi_clk(base, byte >> 6);
	do_spi_clk(base, byte >> 5);
	do_spi_clk(base, byte >> 4);
	do_spi_clk(base, byte >> 3);
	do_spi_clk(base, byte >> 2);
	do_spi_clk(base, byte >> 1);
	do_spi_clk(base, byte);

	pr_debug("spi_byte sent 0x%02x got 0x%02x\n",
	       (unsigned)byte,
	       (unsigned char)__raw_readl(base + AR71XX_SPI_REG_RDS));
}

static inline void do_spi_clk_fast(void __iomem *base, unsigned bit1,
				   unsigned bit2)
{
	unsigned bval = (spi_clk_low |
			 ((bit1 & 1) ? AR71XX_SPI_IOC_DO : 0) |
			 ((bit2 & 1) ? AR71XX_SPI_IOC_CS2 : 0));
	do_spi_delay();
	__raw_writel(bval, base + AR71XX_SPI_REG_IOC);
	do_spi_delay();
	__raw_writel(bval | AR71XX_SPI_IOC_CLK, base + AR71XX_SPI_REG_IOC);
}

static void do_spi_byte_fast(void __iomem *base, unsigned char byte)
{
	do_spi_clk_fast(base, byte >> 7, byte >> 6);
	do_spi_clk_fast(base, byte >> 5, byte >> 4);
	do_spi_clk_fast(base, byte >> 3, byte >> 2);
	do_spi_clk_fast(base, byte >> 1, byte >> 0);

	pr_debug("spi_byte_fast sent 0x%02x got 0x%02x\n",
	       (unsigned)byte,
	       (unsigned char) __raw_readl(base + AR71XX_SPI_REG_RDS));
}

static int rb4xx_spi_txrx(void __iomem *base, struct spi_transfer *t)
{
	const unsigned char *tx_ptr = t->tx_buf;
	unsigned char *rx_ptr = t->rx_buf;
	unsigned i;

	pr_debug("spi_txrx len %u tx %u rx %u\n",
	       t->len,
	       (t->tx_buf ? 1 : 0),
	       (t->rx_buf ? 1 : 0));

	for (i = 0; i < t->len; ++i) {
		unsigned char sdata = tx_ptr ? tx_ptr[i] : 0;

		if (t->tx_nbits == SPI_NBITS_DUAL)
			do_spi_byte_fast(base, sdata);
		else
			do_spi_byte(base, sdata);

		if (rx_ptr)
			rx_ptr[i] = __raw_readl(base + AR71XX_SPI_REG_RDS) & 0xff;
	}

	return i;
}

static int rb4xx_spi_msg(struct rb4xx_spi *rbspi, struct spi_message *m)
{
	struct spi_transfer *t = NULL;
	void __iomem *base = rbspi->base;

	m->status = 0;
	if (list_empty(&m->transfers))
		return -1;

	__raw_writel(AR71XX_SPI_FS_GPIO, base + AR71XX_SPI_REG_FS);
	__raw_writel(SPI_CTRL_FASTEST, base + AR71XX_SPI_REG_CTRL);
	do_spi_init(m->spi);

	list_for_each_entry(t, &m->transfers, transfer_list) {
		int len;

		len = rb4xx_spi_txrx(base, t);
		if (len != t->len) {
			m->status = -EMSGSIZE;
			break;
		}
		m->actual_length += len;

		if (t->cs_change) {
			if (list_is_last(&t->transfer_list, &m->transfers)) {
				/* wait for continuation */
				return m->spi->chip_select;
			}
			do_spi_finish(base);
			ndelay(100);
		}
	}

	do_spi_finish(base);
	__raw_writel(rbspi->spi_ctrl_flash, base + AR71XX_SPI_REG_CTRL);
	__raw_writel(0, base + AR71XX_SPI_REG_FS);
	return -1;
}

static void rb4xx_spi_process_queue_locked(struct rb4xx_spi *rbspi,
					   unsigned long *flags)
{
	int cs = rbspi->cs_wait;

	rbspi->busy = 1;
	while (!list_empty(&rbspi->queue)) {
		struct spi_message *m;

		list_for_each_entry(m, &rbspi->queue, queue)
			if (cs < 0 || cs == m->spi->chip_select)
				break;

		if (&m->queue == &rbspi->queue)
			break;

		list_del_init(&m->queue);
		spin_unlock_irqrestore(&rbspi->lock, *flags);

		cs = rb4xx_spi_msg(rbspi, m);
		m->complete(m->context);

		spin_lock_irqsave(&rbspi->lock, *flags);
	}

	rbspi->cs_wait = cs;
	rbspi->busy = 0;

	if (cs >= 0) {
		/* TODO: add timer to unlock cs after 1s inactivity */
	}
}

static int rb4xx_spi_transfer(struct spi_device *spi,
			      struct spi_message *m)
{
	struct rb4xx_spi *rbspi = spi_master_get_devdata(spi->master);
	unsigned long flags;

	m->actual_length = 0;
	m->status = -EINPROGRESS;

	spin_lock_irqsave(&rbspi->lock, flags);
	list_add_tail(&m->queue, &rbspi->queue);
	if (rbspi->busy ||
	    (rbspi->cs_wait >= 0 && rbspi->cs_wait != m->spi->chip_select)) {
		/* job will be done later */
		spin_unlock_irqrestore(&rbspi->lock, flags);
		return 0;
	}

	/* process job in current context */
	rb4xx_spi_process_queue_locked(rbspi, &flags);
	spin_unlock_irqrestore(&rbspi->lock, flags);

	return 0;
}

static int rb4xx_spi_setup(struct spi_device *spi)
{
	struct rb4xx_spi *rbspi = spi_master_get_devdata(spi->master);
	unsigned long flags;

	if (spi->mode & ~(SPI_CS_HIGH | SPI_TX_DUAL)) {
		dev_err(&spi->dev, "mode %x not supported\n",
			(unsigned) spi->mode);
		return -EINVAL;
	}

	if (spi->bits_per_word != 8 && spi->bits_per_word != 0) {
		dev_err(&spi->dev, "bits_per_word %u not supported\n",
			(unsigned) spi->bits_per_word);
		return -EINVAL;
	}

	spin_lock_irqsave(&rbspi->lock, flags);
	if (rbspi->cs_wait == spi->chip_select && !rbspi->busy) {
		rbspi->cs_wait = -1;
		rb4xx_spi_process_queue_locked(rbspi, &flags);
	}
	spin_unlock_irqrestore(&rbspi->lock, flags);

	return 0;
}

static unsigned get_spi_ctrl(struct rb4xx_spi *rbspi, unsigned hz_max,
			     const char *name)
{
	unsigned div;

	div = (rbspi->ahb_freq - 1) / (2 * hz_max);

	/*
	 * CPU has a bug at (div == 0) - first bit read is random
	 */
	if (div == 0)
		++div;

	if (name) {
		unsigned ahb_khz = (rbspi->ahb_freq + 500) / 1000;
		unsigned div_real = 2 * (div + 1);
		pr_debug("rb4xx: %s SPI clock %u kHz (AHB %u kHz / %u)\n",
		       name,
		       ahb_khz / div_real,
		       ahb_khz, div_real);
	}

	return SPI_CTRL_FASTEST + div;
}

static int rb4xx_spi_probe(struct platform_device *pdev)
{
	struct spi_master *master;
	struct rb4xx_spi *rbspi;
	struct resource *r;
	int err = 0;

	master = spi_alloc_master(&pdev->dev, sizeof(*rbspi));
	if (master == NULL) {
		dev_err(&pdev->dev, "no memory for spi_master\n");
		err = -ENOMEM;
		goto err_out;
	}

	master->bus_num = 0;
	master->num_chipselect = 3;
	master->mode_bits = SPI_TX_DUAL;
	master->setup = rb4xx_spi_setup;
	master->transfer = rb4xx_spi_transfer;

	rbspi = spi_master_get_devdata(master);

	rbspi->ahb_clk = clk_get(&pdev->dev, "ahb");
	if (IS_ERR(rbspi->ahb_clk)) {
		err = PTR_ERR(rbspi->ahb_clk);
		goto err_put_master;
	}

	err = clk_prepare_enable(rbspi->ahb_clk);
	if (err)
		goto err_clk_put;

	rbspi->ahb_freq = clk_get_rate(rbspi->ahb_clk);
	if (!rbspi->ahb_freq) {
		err = -EINVAL;
		goto err_clk_disable;
	}

	platform_set_drvdata(pdev, rbspi);

	r = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (r == NULL) {
		err = -ENOENT;
		goto err_clk_disable;
	}

	rbspi->base = ioremap(r->start, r->end - r->start + 1);
	if (!rbspi->base) {
		err = -ENXIO;
		goto err_clk_disable;
	}

	rbspi->master = master;
	rbspi->spi_ctrl_flash = get_spi_ctrl(rbspi, SPI_FLASH_HZ, "FLASH");
	rbspi->spi_ctrl_fread = get_spi_ctrl(rbspi, SPI_CPLD_HZ, "CPLD");
	rbspi->cs_wait = -1;

	spin_lock_init(&rbspi->lock);
	INIT_LIST_HEAD(&rbspi->queue);

	err = spi_register_master(master);
	if (err) {
		dev_err(&pdev->dev, "failed to register SPI master\n");
		goto err_iounmap;
	}

	return 0;

err_iounmap:
	iounmap(rbspi->base);
err_clk_disable:
	clk_disable(rbspi->ahb_clk);
err_clk_put:
	clk_put(rbspi->ahb_clk);
err_put_master:
	platform_set_drvdata(pdev, NULL);
	spi_master_put(master);
err_out:
	return err;
}

static int rb4xx_spi_remove(struct platform_device *pdev)
{
	struct rb4xx_spi *rbspi = platform_get_drvdata(pdev);

	iounmap(rbspi->base);
	clk_disable(rbspi->ahb_clk);
	clk_put(rbspi->ahb_clk);
	platform_set_drvdata(pdev, NULL);
	spi_master_put(rbspi->master);

	return 0;
}

static struct platform_driver rb4xx_spi_drv = {
	.probe		= rb4xx_spi_probe,
	.remove		= rb4xx_spi_remove,
	.driver		= {
		.name	= DRV_NAME,
		.owner	= THIS_MODULE,
	},
};

static int __init rb4xx_spi_init(void)
{
	return platform_driver_register(&rb4xx_spi_drv);
}
subsys_initcall(rb4xx_spi_init);

static void __exit rb4xx_spi_exit(void)
{
	platform_driver_unregister(&rb4xx_spi_drv);
}

module_exit(rb4xx_spi_exit);

MODULE_DESCRIPTION(DRV_DESC);
MODULE_VERSION(DRV_VERSION);
MODULE_AUTHOR("Gabor Juhos <juhosg@openwrt.org>");
MODULE_LICENSE("GPL v2");
