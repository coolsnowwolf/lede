// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Phytium I2C adapter driver.
 *
 * Based on the TI DAVINCI I2C adapter driver.
 *
 * Copyright (c) 2021-2023 Phytium Technology Co., Ltd.
 */
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/export.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/pm_runtime.h>
#include <linux/swab.h>

#include "i2c-phytium-core.h"

static char *abort_sources[] = {
	[ABRT_7B_ADDR_NOACK] =
		"slave address not acknowledged (7bit mode)",
	[ABRT_10ADDR1_NOACK] =
		"first address byte not acknowledged (10bit mode)",
	[ABRT_10ADDR2_NOACK] =
		"second address byte not acknowledged (10bit mode)",
	[ABRT_TXDATA_NOACK] =
		"data not acknowledged",
	[ABRT_GCALL_NOACK] =
		"no acknowledgment for a general call",
	[ABRT_GCALL_READ] =
		"read after general call",
	[ABRT_SBYTE_ACKDET] =
		"start byte acknowledged",
	[ABRT_SBYTE_NORSTRT] =
		"trying to send start byte when restart is disabled",
	[ABRT_10B_RD_NORSTRT] =
		"trying to read when restart is disabled (10bit mode)",
	[ABRT_MASTER_DIS] =
		"trying to use disabled adapter",
	[ARB_LOST] =
		"lost arbitration",
	[ABRT_SLAVE_FLUSH_TXFIFO] =
		"read command so flush old data in the TX FIFO",
	[ABRT_SLAVE_ARBLOST] =
		"slave lost the bus while transmitting data to a remote master",
	[ABRT_SLAVE_RD_INTX] =
		"incorrect slave-transmitter mode configuration",
};

u32 phytium_readl(struct phytium_i2c_dev *dev, int offset)
{
	return readl_relaxed(dev->base + offset);
}

void phytium_writel(struct phytium_i2c_dev *dev, u32 b, int offset)
{
	writel_relaxed(b, dev->base + offset);
}

u32 i2c_phytium_scl_hcnt(u32 ic_clk, u32 tSYMBOL, u32 tf, int cond, int offset)
{
	if (cond)
		return (ic_clk * tSYMBOL + 500000) / 1000000 - 8 + offset;
	else
		return (ic_clk * (tSYMBOL + tf) + 500000) / 1000000 - 3 + offset;
}

u32 i2c_phytium_scl_lcnt(u32 ic_clk, u32 tLOW, u32 tf, int offset)
{
	return ((ic_clk * (tLOW + tf) + 500000) / 1000000) - 1 + offset;
}

int i2c_phytium_set_sda_hold(struct phytium_i2c_dev *dev)
{
	if (!dev->sda_hold_time) {
		/* Keep previous hold time setting if no one set it */
		dev->sda_hold_time = phytium_readl(dev, IC_SDA_HOLD);
	}

	if (!(dev->sda_hold_time & IC_SDA_HOLD_RX_MASK))
		dev->sda_hold_time |= 1 << IC_SDA_HOLD_RX_SHIFT;

	dev_dbg(dev->dev, "SDA Hold Time TX:RX = %d:%d\n",
		dev->sda_hold_time & ~(u32)IC_SDA_HOLD_RX_MASK,
		dev->sda_hold_time >> IC_SDA_HOLD_RX_SHIFT);

	return 0;
}

void __i2c_phytium_disable(struct phytium_i2c_dev *dev)
{
	int timeout = 100;

	do {
		__i2c_phytium_disable_nowait(dev);
		if ((phytium_readl(dev, IC_ENABLE_STATUS) & 1) == 0)
			return;

		/*
		 * Wait 10 times the signaling period of the highest I2C
		 * transfer supported by the driver (for 400KHz this is
		 * 25us).
		 */
		usleep_range(25, 250);
	} while (timeout--);

	dev_warn(dev->dev, "timeout in disabling adapter\n");
}

unsigned long i2c_phytium_clk_rate(struct phytium_i2c_dev *dev)
{
	if (WARN_ON_ONCE(!dev->get_clk_rate_khz))
		return 0;
	return dev->get_clk_rate_khz(dev);
}

int i2c_phytium_prepare_clk(struct phytium_i2c_dev *dev, bool prepare)
{
	if (IS_ERR(dev->clk))
		return PTR_ERR(dev->clk);

	if (prepare)
		return clk_prepare_enable(dev->clk);

	clk_disable_unprepare(dev->clk);
	return 0;
}
EXPORT_SYMBOL_GPL(i2c_phytium_prepare_clk);

int i2c_phytium_wait_bus_not_busy(struct phytium_i2c_dev *dev)
{
	int timeout = 20; /* 20 ms */

	while (phytium_readl(dev, IC_STATUS) & IC_STATUS_ACTIVITY) {
		if (timeout <= 0) {
			dev_warn(dev->dev, "timeout waiting for bus ready\n");
			i2c_recover_bus(&dev->adapter);

			if (phytium_readl(dev, IC_STATUS) & IC_STATUS_ACTIVITY)
				return -ETIMEDOUT;
			return 0;
		}
		timeout--;
		usleep_range(1000, 1100);
	}

	return 0;
}

int i2c_phytium_handle_tx_abort(struct phytium_i2c_dev *dev)
{
	unsigned long abort_source = dev->abort_source;
	int i;

	if (abort_source & IC_TX_ABRT_NOACK) {
		for_each_set_bit(i, &abort_source, ARRAY_SIZE(abort_sources))
			dev_dbg(dev->dev,
				"%s: %s\n", __func__, abort_sources[i]);
		return -EREMOTEIO;
	}

	for_each_set_bit(i, &abort_source, ARRAY_SIZE(abort_sources))
		dev_err(dev->dev, "%s: %s\n", __func__, abort_sources[i]);

	if (abort_source & IC_TX_ARB_LOST)
		return -EAGAIN;
	else if (abort_source & IC_TX_ABRT_GCALL_READ)
		return -EINVAL;
	else
		return -EIO;

	return 0;
}

u32 i2c_phytium_func(struct i2c_adapter *adapter)
{
	struct phytium_i2c_dev *dev = i2c_get_adapdata(adapter);

	return dev->functionality;
}

void i2c_phytium_disable(struct phytium_i2c_dev *dev)
{
	/* Disable controller */
	__i2c_phytium_disable(dev);

	/* Disable all interrupts */
	phytium_writel(dev, 0, IC_INTR_MASK);
	phytium_readl(dev, IC_CLR_INTR);
}

void i2c_phytium_disable_int(struct phytium_i2c_dev *dev)
{
	phytium_writel(dev, 0, IC_INTR_MASK);
}

MODULE_AUTHOR("Cheng Quan <chengquan@phytium.com.cn>");
MODULE_DESCRIPTION("Phytium I2C bus adapter core");
MODULE_LICENSE("GPL");
