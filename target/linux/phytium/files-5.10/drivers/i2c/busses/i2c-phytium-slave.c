// SPDX-License-Identifier: GPL-2.0
/*
 * Phytium I2C adapter driver (slave only).
 *
 * Copyright (c) 2021-2023 Phytium Technology Co., Ltd.
 */

#include <linux/delay.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/pm_runtime.h>

#include "i2c-phytium-core.h"

static void i2c_phytium_configure_fifo_slave(struct phytium_i2c_dev *dev)
{
	/* Configure Tx/Rx FIFO threshold levels. */
	phytium_writel(dev, 0, IC_TX_TL);
	phytium_writel(dev, 0, IC_RX_TL);

	/* Configure the I2C slave. */
	phytium_writel(dev, dev->slave_cfg, IC_CON);
	phytium_writel(dev, IC_INTR_SLAVE_MASK, IC_INTR_MASK);
}

static int i2c_phytium_init_slave(struct phytium_i2c_dev *dev)
{
	/* Disable the adapter. */
	__i2c_phytium_disable(dev);

	/* Write SDA hold time if supported */
	if (dev->sda_hold_time)
		phytium_writel(dev, dev->sda_hold_time, IC_SDA_HOLD);

	i2c_phytium_configure_fifo_slave(dev);

	return 0;
}

static int i2c_phytium_reg_slave(struct i2c_client *slave)
{
	struct phytium_i2c_dev *dev = i2c_get_adapdata(slave->adapter);

	if (dev->slave)
		return -EBUSY;
	if (slave->flags & I2C_CLIENT_TEN)
		return -EAFNOSUPPORT;
	pm_runtime_get_sync(dev->dev);

	/*
	 * Set slave address in the IC_SAR register,
	 * the address to which the i2c responds.
	 */
	__i2c_phytium_disable_nowait(dev);
	phytium_writel(dev, slave->addr, IC_SAR);
	dev->slave = slave;

	__i2c_phytium_enable(dev);

	dev->cmd_err = 0;
	dev->msg_write_idx = 0;
	dev->msg_read_idx = 0;
	dev->msg_err = 0;
	dev->status = STATUS_IDLE;
	dev->abort_source = 0;
	dev->rx_outstanding = 0;

	return 0;
}

static int i2c_phytium_unreg_slave(struct i2c_client *slave)
{
	struct phytium_i2c_dev *dev = i2c_get_adapdata(slave->adapter);

	dev->disable_int(dev);
	dev->disable(dev);
	dev->slave = NULL;
	pm_runtime_put(dev->dev);

	return 0;
}

static u32 i2c_phytium_read_clear_intrbits_slave(struct phytium_i2c_dev *dev)
{
	u32 stat;

	/*
	 * The IC_INTR_STAT register just indicates "enabled" interrupts.
	 * Ths unmasked raw version of interrupt status bits are available
	 * in the IC_RAW_INTR_STAT register.
	 *
	 * That is,
	 *   stat = phytium_readl(IC_INTR_STAT);
	 * equals to,
	 *   stat = phytium_readl(IC_RAW_INTR_STAT) & phytium_readl(IC_INTR_MASK);
	 *
	 * The raw version might be useful for debugging purposes.
	 */
	stat = phytium_readl(dev, IC_INTR_STAT);

	/*
	 * Do not use the IC_CLR_INTR register to clear interrupts, or
	 * you'll miss some interrupts, triggered during the period from
	 * phytium_readl(IC_INTR_STAT) to phytium_readl(IC_CLR_INTR).
	 *
	 * Instead, use the separately-prepared IC_CLR_* registers.
	 */
	if (stat & IC_INTR_TX_ABRT)
		phytium_readl(dev, IC_CLR_TX_ABRT);
	if (stat & IC_INTR_RX_UNDER)
		phytium_readl(dev, IC_CLR_RX_UNDER);
	if (stat & IC_INTR_RX_OVER)
		phytium_readl(dev, IC_CLR_RX_OVER);
	if (stat & IC_INTR_TX_OVER)
		phytium_readl(dev, IC_CLR_TX_OVER);
	if (stat & IC_INTR_RX_DONE)
		phytium_readl(dev, IC_CLR_RX_DONE);
	if (stat & IC_INTR_ACTIVITY)
		phytium_readl(dev, IC_CLR_ACTIVITY);
	if (stat & IC_INTR_STOP_DET)
		phytium_readl(dev, IC_CLR_STOP_DET);
	if (stat & IC_INTR_START_DET)
		phytium_readl(dev, IC_CLR_START_DET);
	if (stat & IC_INTR_GEN_CALL)
		phytium_readl(dev, IC_CLR_GEN_CALL);

	return stat;
}

/*
 * Interrupt service routine. This gets called whenever an I2C slave interrupt
 * occurs.
 */
static int i2c_phytium_irq_handler_slave(struct phytium_i2c_dev *dev)
{
	u32 raw_stat, stat, enabled;
	u8 val, slave_activity;

	stat = phytium_readl(dev, IC_INTR_STAT);
	enabled = phytium_readl(dev, IC_ENABLE);
	raw_stat = phytium_readl(dev, IC_RAW_INTR_STAT);
	slave_activity = ((phytium_readl(dev, IC_STATUS) &
		IC_STATUS_SLAVE_ACTIVITY) >> 6);

	if (!enabled || !(raw_stat & ~IC_INTR_ACTIVITY) || !dev->slave)
		return 0;

	dev_dbg(dev->dev,
		"%#x STATUS SLAVE_ACTIVITY=%#x : RAW_INTR_STAT=%#x : INTR_STAT=%#x\n",
		enabled, slave_activity, raw_stat, stat);

	if ((stat & IC_INTR_RX_FULL) && (stat & IC_INTR_STOP_DET))
		i2c_slave_event(dev->slave, I2C_SLAVE_WRITE_REQUESTED, &val);

	if (stat & IC_INTR_RD_REQ) {
		if (slave_activity) {
			if (stat & IC_INTR_RX_FULL) {
				val = phytium_readl(dev, IC_DATA_CMD);

				if (!i2c_slave_event(dev->slave,
						     I2C_SLAVE_WRITE_RECEIVED,
						     &val)) {
					dev_vdbg(dev->dev, "Byte %X acked!",
						 val);
				}
				phytium_readl(dev, IC_CLR_RD_REQ);
				stat = i2c_phytium_read_clear_intrbits_slave(dev);
			} else {
				phytium_readl(dev, IC_CLR_RD_REQ);
				phytium_readl(dev, IC_CLR_RX_UNDER);
				stat = i2c_phytium_read_clear_intrbits_slave(dev);
			}
			if (!i2c_slave_event(dev->slave,
					     I2C_SLAVE_READ_REQUESTED,
					     &val))
				phytium_writel(dev, val, IC_DATA_CMD);
		}
	}

	if (stat & IC_INTR_RX_DONE) {
		if (!i2c_slave_event(dev->slave, I2C_SLAVE_READ_PROCESSED,
				     &val))
			phytium_readl(dev, IC_CLR_RX_DONE);

		i2c_slave_event(dev->slave, I2C_SLAVE_STOP, &val);
		stat = i2c_phytium_read_clear_intrbits_slave(dev);
		return 1;
	}

	if (stat & IC_INTR_RX_FULL) {
		val = phytium_readl(dev, IC_DATA_CMD);
		if (!i2c_slave_event(dev->slave, I2C_SLAVE_WRITE_RECEIVED,
				     &val))
			dev_vdbg(dev->dev, "Byte %X acked!", val);
	} else {
		i2c_slave_event(dev->slave, I2C_SLAVE_STOP, &val);
		stat = i2c_phytium_read_clear_intrbits_slave(dev);
	}

	return 1;
}

static irqreturn_t i2c_phytium_isr_slave(int this_irq, void *dev_id)
{
	struct phytium_i2c_dev *dev = dev_id;
	int ret;

	i2c_phytium_read_clear_intrbits_slave(dev);
	ret = i2c_phytium_irq_handler_slave(dev);
	if (ret > 0)
		complete(&dev->cmd_complete);

	return IRQ_RETVAL(ret);
}

static const struct i2c_algorithm i2c_phytium_algo = {
	.functionality = i2c_phytium_func,
	.reg_slave = i2c_phytium_reg_slave,
	.unreg_slave = i2c_phytium_unreg_slave,
};

int i2c_phytium_probe_slave(struct phytium_i2c_dev *dev)
{
	struct i2c_adapter *adap = &dev->adapter;
	int ret;

	init_completion(&dev->cmd_complete);

	dev->init = i2c_phytium_init_slave;
	dev->disable = i2c_phytium_disable;
	dev->disable_int = i2c_phytium_disable_int;

	ret = dev->init(dev);
	if (ret)
		return ret;

	snprintf(adap->name, sizeof(adap->name),
		 "Phytium I2C Slave adapter");
	adap->retries = 3;
	adap->algo = &i2c_phytium_algo;
	adap->dev.parent = dev->dev;
	i2c_set_adapdata(adap, dev);

	ret = devm_request_irq(dev->dev, dev->irq, i2c_phytium_isr_slave,
			       IRQF_SHARED, dev_name(dev->dev), dev);
	if (ret) {
		dev_err(dev->dev, "failure requesting irq %i: %d\n",
			dev->irq, ret);
		return ret;
	}

	ret = i2c_add_numbered_adapter(adap);
	if (ret)
		dev_err(dev->dev, "failure adding adapter: %d\n", ret);

	return ret;
}
EXPORT_SYMBOL_GPL(i2c_phytium_probe_slave);
