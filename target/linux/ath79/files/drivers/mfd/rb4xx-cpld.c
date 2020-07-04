// SPDX-License-Identifier: GPL-2.0-only
/*
 * CPLD driver for the MikroTik RouterBoard 4xx series
 *
 * This driver provides access to a CPLD that interfaces between the SoC SPI bus
 * and other devices. Behind the CPLD there is a NAND flash chip and five LEDs.
 *
 * The CPLD supports SPI two-wire mode, in which two bits are transferred per
 * SPI clock cycle. The second bit is transmitted with the SoC's CS2 pin.
 *
 * The CPLD also acts as a GPIO expander.
 *
 * Copyright (C) 2008-2011 Gabor Juhos <juhosg@openwrt.org>
 * Copyright (C) 2008 Imre Kaloz <kaloz@openwrt.org>
 * Copyright (C) 2015 Bert Vermeulen <bert@biot.com>
 * Copyright (C) 2020 Christopher Hill <ch6574@gmail.com>
 *
 * This file was based on the driver for Linux 2.6.22 published by
 * MikroTik for their RouterBoard 4xx series devices.
*/
#include <linux/mfd/core.h>
#include <linux/spi/spi.h>
#include <linux/module.h>
#include <linux/of_platform.h>

#include <mfd/rb4xx-cpld.h>

/* CPLD commands */
#define CPLD_CMD_WRITE_NAND	0x08 /* send cmd, n x send data, send idle */
#define CPLD_CMD_WRITE_CFG	0x09 /* send cmd, n x send cfg */
#define CPLD_CMD_READ_NAND	0x0a /* send cmd, send idle, n x read data */
#define CPLD_CMD_READ_FAST	0x0b /* send cmd, 4 x idle, n x read data */
#define CPLD_CMD_GPIO8_HIGH	0x0c /* send cmd */
#define CPLD_CMD_GPIO8_LOW	0x0d /* send cmd */

static int rb4xx_cpld_write_nand(struct rb4xx_cpld *cpld, const void *tx_buf,
				 unsigned int len)
{
	struct spi_message m;
	static const u8 cmd = CPLD_CMD_WRITE_NAND;
	struct spi_transfer t[3] = {
		{
			.tx_buf = &cmd,
			.len = sizeof(cmd),
		}, {
			.tx_buf = tx_buf,
			.len = len,
			.tx_nbits = SPI_NBITS_DUAL,
		}, {
			.len = 1,
			.tx_nbits = SPI_NBITS_DUAL,
		},
	};

	spi_message_init(&m);
	spi_message_add_tail(&t[0], &m);
	spi_message_add_tail(&t[1], &m);
	spi_message_add_tail(&t[2], &m);
	return spi_sync(cpld->spi, &m);
}

static int rb4xx_cpld_read_nand(struct rb4xx_cpld *cpld, void *rx_buf,
				unsigned int len)
{
	struct spi_message m;
	static const u8 cmd[2] = {
		CPLD_CMD_READ_NAND, 0
	};
	struct spi_transfer t[2] = {
		{
			.tx_buf = &cmd,
			.len = sizeof(cmd),
		}, {
			.rx_buf = rx_buf,
			.len = len,
		},
	};

	spi_message_init(&m);
	spi_message_add_tail(&t[0], &m);
	spi_message_add_tail(&t[1], &m);
	return spi_sync(cpld->spi, &m);
}

static int rb4xx_cpld_cmd(struct rb4xx_cpld *cpld, const void *tx_buf,
			  unsigned int len)
{
	struct spi_message m;
	struct spi_transfer t = {
		.tx_buf = tx_buf,
		.len = len,
	};

	spi_message_init(&m);
	spi_message_add_tail(&t, &m);
	return spi_sync(cpld->spi, &m);
}

static int rb4xx_cpld_gpio_set_0_7(struct rb4xx_cpld *cpld, u8 values)
{
	/* GPIO 0-7 change can be sent via command + bitfield */
	u8 cmd[2] = {
		CPLD_CMD_WRITE_CFG, values
	};
	return rb4xx_cpld_cmd(cpld, &cmd, 2);
}

static int rb4xx_cpld_gpio_set_8(struct rb4xx_cpld *cpld, u8 value)
{
	/* GPIO 8 uses dedicated high/low commands */
	u8 cmd = CPLD_CMD_GPIO8_HIGH | !!(value);
	return rb4xx_cpld_cmd(cpld, &cmd, 1);
}

static const struct mfd_cell rb4xx_cpld_cells[] = {
	{
		.name = "mikrotik,rb4xx-gpio",
		.of_compatible = "mikrotik,rb4xx-gpio",
	}, {
		.name = "mikrotik,rb4xx-nand",
		.of_compatible = "mikrotik,rb4xx-nand",
	},
};

static int rb4xx_cpld_probe(struct spi_device *spi)
{
	struct device *dev = &spi->dev;
	struct rb4xx_cpld *cpld;
	int ret;

	cpld = devm_kzalloc(dev, sizeof(*cpld), GFP_KERNEL);
	if (!cpld)
		return -ENOMEM;

	dev_set_drvdata(dev, cpld);

	cpld->spi		= spi;
	cpld->write_nand	= rb4xx_cpld_write_nand;
	cpld->read_nand		= rb4xx_cpld_read_nand;
	cpld->gpio_set_0_7	= rb4xx_cpld_gpio_set_0_7;
	cpld->gpio_set_8	= rb4xx_cpld_gpio_set_8;

	spi->mode = SPI_MODE_0 | SPI_TX_DUAL;
	ret = spi_setup(spi);
	if (ret)
		return ret;

	return devm_mfd_add_devices(dev, PLATFORM_DEVID_NONE,
				    rb4xx_cpld_cells,
				    ARRAY_SIZE(rb4xx_cpld_cells),
				    NULL, 0, NULL);
}

static int rb4xx_cpld_remove(struct spi_device *spi)
{
	return 0;
}

static const struct of_device_id rb4xx_cpld_dt_match[] = {
	{ .compatible = "mikrotik,rb4xx-cpld", },
	{ },
};
MODULE_DEVICE_TABLE(of, rb4xx_cpld_dt_match);

static struct spi_driver rb4xx_cpld_driver = {
	.probe = rb4xx_cpld_probe,
	.remove = rb4xx_cpld_remove,
	.driver = {
		.name = "rb4xx-cpld",
		.bus = &spi_bus_type,
		.of_match_table = of_match_ptr(rb4xx_cpld_dt_match),
	},
};

module_spi_driver(rb4xx_cpld_driver);

MODULE_DESCRIPTION("Mikrotik RB4xx CPLD driver");
MODULE_AUTHOR("Gabor Juhos <juhosg@openwrt.org>");
MODULE_AUTHOR("Imre Kaloz <kaloz@openwrt.org>");
MODULE_AUTHOR("Bert Vermeulen <bert@biot.com>");
MODULE_AUTHOR("Christopher Hill <ch6574@gmail.com");
MODULE_LICENSE("GPL v2");
