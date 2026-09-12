// SPDX-License-Identifier: GPL-2.0
/*
 * Phytium I2C adapter driver.
 *
 * Copyright (c) 2021-2024 Phytium Technology Co., Ltd.
 */
#include <linux/acpi.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/export.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/pm_runtime.h>
#include <linux/reset.h>

#include "i2c-phytium-core.h"

#define I2C_TRANS_READ_CMD			BIT(8)
#define I2C_TRANS_STOP_CMD			BIT(9)
#define I2C_TRANS_RESTART_CMD			BIT(10)
#define I2C_QUICK_CMD_BIT_SET			BIT(13)
#define DEFAULT_TIMEOUT				(DEFAULT_CLOCK_FREQUENCY / 1000 * 35)

static int i2c_phytium_init_master(struct phytium_i2c_dev *dev);
static void i2c_phytium_change_mode(int target_mode, struct phytium_i2c_dev *dev);

static int i2c_phytium_recover_controller(struct phytium_i2c_dev *dev)
{
	unsigned long flags;

	spin_lock_irqsave(&dev->i2c_lock, flags);

	reset_control_reset(dev->rst);

	if (!dev->slave) {
		i2c_phytium_init_master(dev);
#if IS_ENABLED(CONFIG_I2C_SLAVE)
	} else {
		dev->slave_status = SLAVE_STATE_IDLE;
		phytium_writel(dev, dev->slave->addr, IC_SAR);
		i2c_phytium_change_mode(PHYTIUM_IC_SLAVE, dev);
#endif
	}

	dev->status = STATUS_IDLE;
	spin_unlock_irqrestore(&dev->i2c_lock, flags);

	return 0;
}

static int i2c_phytium_init_slave(struct phytium_i2c_dev *dev)
{
	/* Disable the adapter. */
	__i2c_phytium_disable(dev);

	/* Write SDA hold time if supported */
	if (dev->sda_hold_time)
		phytium_writel(dev, dev->sda_hold_time, IC_SDA_HOLD);

	/* Configure Tx/Rx FIFO threshold levels. */
	phytium_writel(dev, 0, IC_TX_TL);
	phytium_writel(dev, 0, IC_RX_TL);
	dev->mode = PHYTIUM_IC_SLAVE;

	/* Configure the I2C slave. */
	dev->slave_cfg = IC_CON_RX_FIFO_FULL_HLD_CTRL | IC_CON_RESTART_EN |
			 IC_CON_STOP_DET_IFADDRESSED;
	phytium_writel(dev, dev->slave_cfg, IC_CON);
	phytium_writel(dev, IC_INTR_SLAVE_MASK, IC_INTR_MASK);

	return 0;
}

void i2c_phytium_configure_master(struct phytium_i2c_dev *dev)
{
	struct i2c_timings *t = &dev->timings;

	dev->functionality = I2C_FUNC_10BIT_ADDR | IC_DEFAULT_FUNCTIONALITY |
			     I2C_FUNC_SLAVE;

	dev->master_cfg =
		IC_CON_MASTER | IC_CON_SLAVE_DISABLE | IC_CON_RESTART_EN;

	dev->mode = PHYTIUM_IC_MASTER;

	switch (t->bus_freq_hz) {
	case 100000:
		dev->master_cfg |= IC_CON_SPEED_STD;
		break;
	case 3400000:
		dev->master_cfg |= IC_CON_SPEED_HIGH;
		break;
	default:
		dev->master_cfg |= IC_CON_SPEED_FAST;
	}
}
EXPORT_SYMBOL_GPL(i2c_phytium_configure_master);

static int i2c_phytium_init_master(struct phytium_i2c_dev *dev)
{
	/* Disable the adapter */
	__i2c_phytium_disable(dev);

	/* Write standard speed timing parameters */
	phytium_writel(dev, dev->ss_hcnt, IC_SS_SCL_HCNT);
	phytium_writel(dev, dev->ss_lcnt, IC_SS_SCL_LCNT);

	/* Write fast mode/fast mode plus timing parameters */
	phytium_writel(dev, dev->fs_hcnt, IC_FS_SCL_HCNT);
	phytium_writel(dev, dev->fs_lcnt, IC_FS_SCL_LCNT);

	/* Write high speed timing parameters if supported */
	if (dev->hs_hcnt && dev->hs_hcnt) {
		phytium_writel(dev, dev->hs_hcnt, IC_HS_SCL_HCNT);
		phytium_writel(dev, dev->hs_lcnt, IC_HS_SCL_LCNT);
	}
	dev->mode = PHYTIUM_IC_MASTER;
	/* Write SDA hold time if supported */
	if (dev->sda_hold_time)
		phytium_writel(dev, dev->sda_hold_time, IC_SDA_HOLD);

	/* Configure Tx/Rx FIFO threshold levels */
	phytium_writel(dev, dev->tx_fifo_depth >> 1, IC_TX_TL);
	phytium_writel(dev, 0, IC_RX_TL);

	/* Configure the I2C master */
	if (dev->first_time_init_master == false) {
		i2c_phytium_configure_master(dev);
		dev->first_time_init_master = true;
	}
	phytium_writel(dev, dev->master_cfg, IC_CON);

	return 0;
}

#if IS_ENABLED(CONFIG_I2C_SLAVE)
static void i2c_phytium_change_mode(int target_mode, struct phytium_i2c_dev *dev)
{
	if (target_mode == PHYTIUM_IC_MASTER) {
		dev->disable(dev);
		i2c_phytium_init_master(dev);
	} else {
		i2c_phytium_init_slave(dev);
		__i2c_phytium_enable(dev);
	}
}
#endif

static void i2c_phytium_xfer_init(struct phytium_i2c_dev *dev)
{
	struct i2c_msg *msgs = dev->msgs;
	u32 ic_con, ic_tar = 0;

	/* Disable the adapter */
	__i2c_phytium_disable(dev);

	if ((dev->capability & I2C_FUNC_SMBUS_QUICK) && (msgs[0].len == 0))
		ic_tar = I2C_QUICK_CMD_BIT_SET;

	/* If the slave address is 10-bit address, enable 10BITADDR */
	ic_con = phytium_readl(dev, IC_CON);
	if (msgs[dev->msg_write_idx].flags & I2C_M_TEN) {
		ic_con |= IC_CON_10BITADDR_MASTER;
		ic_tar = IC_TAR_10BITADDR_MASTER;
	} else {
		ic_con &= ~IC_CON_10BITADDR_MASTER;
	}

	phytium_writel(dev, ic_con, IC_CON);

	/*
	 * Set the slave (target) address and enable 10-bit addressing mode
	 * if applicable.
	 */
	phytium_writel(dev, msgs[dev->msg_write_idx].addr | ic_tar, IC_TAR);

	/* Enforce disabled interrupts */
	i2c_phytium_disable_int(dev);

	/* Enable the adapter */
	__i2c_phytium_enable(dev);

	/* Dummy read */
	phytium_readl(dev, IC_ENABLE_STATUS);

	/* Clear and enable interrupts */
	phytium_readl(dev, IC_CLR_INTR);
	phytium_writel(dev, IC_INTR_SMBUS_MASK, IC_INTR_MASK);
}

static void i2c_phytium_xfer_msg(struct phytium_i2c_dev *dev)
{
	struct i2c_msg *msgs = dev->msgs;
	u32 intr_mask;
	int tx_limit, rx_limit;
	u32 addr = msgs[dev->msg_write_idx].addr;
	u32 buf_len = dev->tx_buf_len;
	u8 *buf = dev->tx_buf;
	bool need_restart = false;

	intr_mask = IC_INTR_SMBUS_MASK;

	for (; dev->msg_write_idx < dev->msgs_num; dev->msg_write_idx++) {
		u32 flags = msgs[dev->msg_write_idx].flags;

		if (msgs[dev->msg_write_idx].addr != addr) {
			dev->msg_err = -EINVAL;
			break;
		}
		/* If the register dose not support smbus block read function,
		 * then use the operation of program code
		 */
		if (!(dev->capability & I2C_FUNC_SMBUS_BLOCK_DATA) &&
			(flags & I2C_M_RECV_LEN))
			msgs[dev->msg_write_idx].len = 1 + I2C_SMBUS_BLOCK_MAX;

		if (!(dev->status & STATUS_WRITE_IN_PROGRESS)) {
			/* new i2c_msg */
			buf = msgs[dev->msg_write_idx].buf;
			buf_len = msgs[dev->msg_write_idx].len;
			if ((dev->master_cfg & IC_CON_RESTART_EN) &&
			    (dev->msg_write_idx > 0))
				need_restart = true;
		}

		tx_limit = dev->tx_fifo_depth - phytium_readl(dev, IC_TXFLR);
		rx_limit = dev->tx_fifo_depth - phytium_readl(dev, IC_RXFLR);
		while (buf_len > 0 && tx_limit > 0 && rx_limit > 0) {
			u32 cmd = 0;

			if (dev->msg_write_idx == dev->msgs_num - 1 && buf_len == 1)
				cmd |= I2C_TRANS_STOP_CMD;

			if (need_restart) {
				cmd |= I2C_TRANS_RESTART_CMD;
				need_restart = false;
			}

			if (msgs[dev->msg_write_idx].flags & I2C_M_RD) {
				/* avoid rx buffer overrun */
				if (dev->rx_outstanding >= dev->rx_fifo_depth)
					break;

				phytium_writel(dev, cmd | I2C_TRANS_READ_CMD, IC_DATA_CMD);
				rx_limit--;
				dev->rx_outstanding++;
			} else {
				phytium_writel(dev, cmd | *buf++, IC_DATA_CMD);
			}
			tx_limit--;
			buf_len--;
		}

		dev->tx_buf = buf;
		dev->tx_buf_len = buf_len;

		/*
		 * Because we don't know the buffer length in the
		 * I2C_FUNC_SMBUS_BLOCK_DATA case, we can't stop
		 * the transaction here.
		 */
		if (buf_len > 0) {
			/* more bytes to be written */
			dev->status |= STATUS_WRITE_IN_PROGRESS;
			break;
		}
		dev->status &= ~STATUS_WRITE_IN_PROGRESS;
	}

	/*This is use for quick cmd*/
	if ((dev->capability & I2C_FUNC_SMBUS_QUICK) && (msgs[0].len == 0)) {
		if (msgs[0].flags == I2C_M_RD)
			phytium_writel(dev, I2C_TRANS_STOP_CMD | I2C_TRANS_READ_CMD,
					IC_DATA_CMD);
		else
			phytium_writel(dev, I2C_TRANS_STOP_CMD, IC_DATA_CMD);
	}

	if (dev->msg_write_idx == dev->msgs_num)
		intr_mask &= ~IC_INTR_TX_EMPTY;

	if (dev->msg_err)
		intr_mask = 0;

	phytium_writel(dev, intr_mask, IC_INTR_MASK);
}

static u8 i2c_phytium_recv_len(struct phytium_i2c_dev *dev, u8 len)
{
	struct i2c_msg *msgs = dev->msgs;
	u32 flags = msgs[dev->msg_read_idx].flags;

	/*
	 * Adjust the buffer length and mask the flag
	 * after receiving the first byte.
	 */
	if (len > I2C_SMBUS_BLOCK_MAX)
		len = 0;

	len += (flags & I2C_CLIENT_PEC) ? 2 : 1;
	dev->tx_buf_len = len - min_t(u8, len, dev->rx_outstanding);

	/* require adding one byte to trigger the i2c_phytium_xfer finishing the transaction. */
	if (dev->tx_buf_len == 0) {
		dev->tx_buf_len = 1;
		len = dev->rx_outstanding + 1;
	}

	return len;
}

static void i2c_phytium_read(struct phytium_i2c_dev *dev)
{
	struct i2c_msg *msgs = dev->msgs;
	int rx_valid;

	for (; dev->msg_read_idx < dev->msgs_num; dev->msg_read_idx++) {
		u32 len;
		u8 *buf;

		if (!(msgs[dev->msg_read_idx].flags & I2C_M_RD))
			continue;

		if (!(dev->status & STATUS_READ_IN_PROGRESS)) {
			len = msgs[dev->msg_read_idx].len;
			buf = msgs[dev->msg_read_idx].buf;
		} else {
			len = dev->rx_buf_len;
			buf = dev->rx_buf;
		}

		rx_valid = phytium_readl(dev, IC_RXFLR);

		for (; len > 0 && rx_valid > 0; len--, rx_valid--) {
			u32 flags = msgs[dev->msg_read_idx].flags;

			*buf = phytium_readl(dev, IC_DATA_CMD);
			/* Ensure length byte is a valid value */
			if (flags & I2C_M_RECV_LEN) {
				msgs[dev->msg_read_idx].flags &= ~I2C_M_RECV_LEN;
				len = i2c_phytium_recv_len(dev, *buf);
				if (*buf > I2C_SMBUS_BLOCK_MAX || *buf == 0) {
					dev_err(dev->dev,
						"The value %d is out of the SMBus range [1~32]\n",
						*buf);
					*buf = 0;
					dev->cmd_err = IC_ERR_SMBLK_READ_ZERO;
				} else {
					msgs[dev->msg_read_idx].len = *buf +
								((flags & I2C_CLIENT_PEC) ? 2 : 1);
				}
			}
			buf++;
			dev->rx_outstanding--;
		}

		if (len > 0) {
			dev->status |= STATUS_READ_IN_PROGRESS;
			dev->rx_buf_len = len;
			dev->rx_buf = buf;
			return;
		}
		dev->status &= ~STATUS_READ_IN_PROGRESS;
	}
}

static int i2c_phytium_xfer(struct i2c_adapter *adapter, struct i2c_msg msgs[],
		     int num)
{
	struct phytium_i2c_dev *dev = i2c_get_adapdata(adapter);
	int ret;
	unsigned long flags;

	dev_dbg(dev->dev, "%s: msgs: %d\n", __func__, num);

	ret = pm_runtime_get_sync(dev->dev);
	if (ret < 0) {
		dev_err(dev->dev, "pm runtime get sync err.\n");
		goto done;
	}

	spin_lock_irqsave(&dev->i2c_lock, flags);

#if IS_ENABLED(CONFIG_I2C_SLAVE)
	if (dev->slave && dev->slave_status != SLAVE_STATE_IDLE) {
		reinit_completion(&dev->slave_complete);
		spin_unlock_irqrestore(&dev->i2c_lock, flags);

		/* Waiting for slave to complete. */
		if (!wait_for_completion_timeout(&dev->slave_complete,
				adapter->timeout)) {
			dev_err(dev->dev, "Slave is timeout, recover\n");
			i2c_phytium_recover_controller(dev);
			ret = -ETIMEDOUT;
			goto done;
		}
		spin_lock_irqsave(&dev->i2c_lock, flags);
	}

	if (dev->mode == PHYTIUM_IC_SLAVE)
		i2c_phytium_change_mode(PHYTIUM_IC_MASTER, dev);
#endif

	reinit_completion(&dev->cmd_complete);
	dev->msgs = msgs;
	dev->msgs_num = num;
	dev->cmd_err = 0;
	dev->msg_write_idx = 0;
	dev->msg_read_idx = 0;
	dev->msg_err = 0;
	dev->abort_source = 0;
	dev->rx_outstanding = 0;

	/* Start the transfers */
	i2c_phytium_xfer_init(dev);
	spin_unlock_irqrestore(&dev->i2c_lock, flags);
	/* Wait for tx to complete */
	if (!wait_for_completion_timeout(&dev->cmd_complete,
					 adapter->timeout)) {
		dev_err(dev->dev, "controller timed out\n");
		i2c_phytium_recover_controller(dev);
		ret = -ETIMEDOUT;
		goto done;
	}

	if (!dev->slave)
		__i2c_phytium_disable_nowait(dev);
#if IS_ENABLED(CONFIG_I2C_SLAVE)
	/* Do nothing. */
	if (dev->slave && dev->msgs->len >= REQ_MIN_LEN) {
		reinit_completion(&dev->slave_complete);
		wait_for_completion_timeout(&dev->slave_complete, adapter->timeout);
	}
#endif
	if (dev->msg_err) {
		ret = dev->msg_err;
		goto done;
	}

	if (likely(!dev->cmd_err && !dev->status)) {
		ret = num;
		goto done;
	}

	/* We have got an error */
	if (dev->cmd_err == IC_ERR_TX_ABRT) {
		ret = i2c_phytium_handle_tx_abort(dev);
		goto done;
	}

	ret = -EIO;

done:
	pm_runtime_mark_last_busy(dev->dev);
	pm_runtime_put_autosuspend(dev->dev);

	return ret;
}

#if IS_ENABLED(CONFIG_I2C_SLAVE)
static int i2c_phytium_reg_slave(struct i2c_client *slave)
{
	int ret = 0;
	unsigned long flags;
	struct phytium_i2c_dev *dev = i2c_get_adapdata(slave->adapter);

	spin_lock_irqsave(&dev->i2c_lock, flags);

	if (dev->slave) {
		ret = -EBUSY;
		goto abort;
	}
	if (slave->flags & I2C_CLIENT_TEN) {
		ret = -EAFNOSUPPORT;
		goto abort;
	}
	pm_runtime_get_sync(dev->dev);

	/*
	 * Set slave address in the IC_SAR register,
	 * the address to which the i2c responds.
	 */
	i2c_phytium_init_slave(dev);
	phytium_writel(dev, slave->addr, IC_SAR);
	dev->slave = slave;
	dev->mode = PHYTIUM_IC_SLAVE;
	__i2c_phytium_enable(dev);

	dev->cmd_err = 0;
	dev->msg_write_idx = 0;
	dev->msg_read_idx = 0;
	dev->msg_err = 0;
	dev->slave_status = SLAVE_STATE_IDLE;
	dev->abort_source = 0;
	dev->rx_outstanding = 0;

abort:
	spin_unlock_irqrestore(&dev->i2c_lock, flags);
	return ret;
}

static int i2c_phytium_unreg_slave(struct i2c_client *slave)
{
	unsigned long flags;
	struct phytium_i2c_dev *dev = i2c_get_adapdata(slave->adapter);

	spin_lock_irqsave(&dev->i2c_lock, flags);

	dev->disable_int(dev);
	dev->disable(dev);
	dev->slave = NULL;
	pm_runtime_put(dev->dev);

	dev->mode = PHYTIUM_IC_MASTER;
	i2c_phytium_init_master(dev);
	spin_unlock_irqrestore(&dev->i2c_lock, flags);
	return 0;
}
#endif

static const struct i2c_algorithm i2c_phytium_algo = {
	.master_xfer = i2c_phytium_xfer,
	.functionality = i2c_phytium_func,
#if IS_ENABLED(CONFIG_I2C_SLAVE)
	.reg_slave = i2c_phytium_reg_slave,
	.unreg_slave = i2c_phytium_unreg_slave,
#endif
};

static u32 i2c_phytium_read_clear_intrbits(struct phytium_i2c_dev *dev)
{
	u32 stat;

	stat = phytium_readl(dev, IC_INTR_STAT);

	if (stat & IC_INTR_RX_UNDER)
		phytium_readl(dev, IC_CLR_RX_UNDER);
	if (stat & IC_INTR_RX_OVER)
		phytium_readl(dev, IC_CLR_RX_OVER);
	if (stat & IC_INTR_TX_OVER)
		phytium_readl(dev, IC_CLR_TX_OVER);
	if (stat & IC_INTR_RD_REQ)
		phytium_readl(dev, IC_CLR_RD_REQ);
	if (stat & IC_INTR_TX_ABRT) {
		if (dev->mode == PHYTIUM_IC_MASTER)
			dev->abort_source = phytium_readl(dev, IC_TX_ABRT_SOURCE);
		phytium_readl(dev, IC_CLR_TX_ABRT);
	}
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
	if (stat & IC_INTR_SMBCLK_EXT_LOW_TIMEOUT)
		phytium_readl(dev, IC_CLR_SMBCLK_EXT_LOW_TIMEOUT);
	if (stat & IC_INTR_SMBCLK_TMO_LOW_TIMEOUT)
		phytium_readl(dev, IC_CLR_SMBCLK_TMO_LOW_TIMEOUT);
	if (stat & IC_INTR_SMBSDA_LOW_TIMEOUT)
		phytium_readl(dev, IC_CLR_SMBDAT_LOW_TIMEOUT);
	if (stat & IC_INTR_SMBALERT_IN_N)
		phytium_readl(dev, IC_CLR_SMBALERT_IN_N);

	return stat;
}

static int i2c_phytium_irq_handler_master(struct phytium_i2c_dev *dev)
{
	u32 stat, raw_stat;

	raw_stat = phytium_readl(dev, IC_RAW_INTR_STAT);
	stat = i2c_phytium_read_clear_intrbits(dev);

	/* SMBus interrupt */
	if (stat &
	    (IC_INTR_SMBCLK_EXT_LOW_TIMEOUT | IC_INTR_SMBCLK_TMO_LOW_TIMEOUT)) {
		phytium_writel(dev, phytium_readl(dev, IC_ENABLE) & (~BIT(6)),
			       IC_ENABLE);
		phytium_writel(dev, phytium_readl(dev, IC_ENABLE) | BIT(4),
			       IC_ENABLE);
		goto abort;
	}

	if (stat & IC_INTR_SMBSDA_LOW_TIMEOUT) {
		phytium_writel(dev, phytium_readl(dev, IC_ENABLE) | BIT(6),
			       IC_ENABLE);
		goto abort;
	}

	if (stat & IC_INTR_SMBALERT_IN_N && dev->ara)
		i2c_handle_smbus_alert(dev->ara);

	if (raw_stat & IC_INTR_TX_ABRT) {
		if (stat & IC_INTR_TX_ABRT) {
			dev->cmd_err |= IC_ERR_TX_ABRT;
			dev->status = STATUS_IDLE;

			/* Anytime TX_ABRT is set, the contents of the tx/rx
			 * buffers are flushed. Make sure to skip them.
			 */
			phytium_writel(dev, 0, IC_INTR_MASK);
			goto abort;
		} else if (stat & IC_INTR_STOP_DET) {
			dev->cmd_err |= IC_ERR_TX_ABRT;
			dev->status = STATUS_IDLE;
			phytium_readl(dev, IC_CLR_TX_ABRT);
			phytium_writel(dev, 0, IC_INTR_MASK);

			goto abort;
		}
	}

	if (stat & IC_INTR_RX_FULL)
		i2c_phytium_read(dev);

	if (stat & IC_INTR_TX_EMPTY)
		i2c_phytium_xfer_msg(dev);

abort:
	if ((stat & IC_INTR_TX_ABRT) || (stat & IC_INTR_STOP_DET) || dev->msg_err) {
		complete(&dev->cmd_complete);
#if IS_ENABLED(CONFIG_I2C_SLAVE)
		if (dev->slave)
			i2c_phytium_change_mode(PHYTIUM_IC_SLAVE, dev);
#endif
	} else if (unlikely(dev->flags & ACCESS_INTR_MASK)) {
		/* Workaround to trigger pending interrupt */
		stat = phytium_readl(dev, IC_INTR_MASK);
		i2c_phytium_disable_int(dev);
		phytium_writel(dev, stat, IC_INTR_MASK);
	}

	return 0;
}

static int i2c_phytium_irq_handler_slave(struct phytium_i2c_dev *dev)
{
	u32 raw_stat, stat, enabled;
	u8 val, slave_activity;

	enabled = phytium_readl(dev, IC_ENABLE);
	raw_stat = phytium_readl(dev, IC_RAW_INTR_STAT);
	slave_activity =
		((phytium_readl(dev, IC_STATUS) & IC_STATUS_SLAVE_ACTIVITY) >>
		 6);

	if (!enabled || !(raw_stat & ~IC_INTR_ACTIVITY) || !dev->slave)
		return 0;

	stat = i2c_phytium_read_clear_intrbits(dev);

	if (stat & IC_INTR_RX_FULL) {
		if (dev->slave_status != SLAVE_WRITE_IN_PROGRESS) {
			dev->slave_status = SLAVE_WRITE_IN_PROGRESS;
			i2c_slave_event(dev->slave, I2C_SLAVE_WRITE_REQUESTED,
					&val);
		}
		do {
			val = phytium_readl(dev, IC_DATA_CMD);
			i2c_slave_event(dev->slave, I2C_SLAVE_WRITE_RECEIVED,
					&val);
			val = phytium_readl(dev, IC_STATUS);
		} while (val & BIT(3));
	}

	if (stat & IC_INTR_RD_REQ) {
		if (slave_activity) {
			phytium_readl(dev, IC_CLR_RD_REQ);

			if (!(dev->slave_status & SLAVE_READ_IN_PROGRESS)) {
				i2c_slave_event(dev->slave,
						I2C_SLAVE_READ_REQUESTED, &val);
				dev->slave_status |= SLAVE_READ_IN_PROGRESS;
				dev->slave_status &= ~SLAVE_WRITE_IN_PROGRESS;
			} else {
				i2c_slave_event(dev->slave,
						I2C_SLAVE_READ_PROCESSED, &val);
			}
			phytium_writel(dev, val, IC_DATA_CMD);
		}
	}

	if (stat & IC_INTR_STOP_DET) {
		i2c_slave_event(dev->slave, I2C_SLAVE_STOP, &val);
#if IS_ENABLED(CONFIG_I2C_SLAVE)
		dev->slave_status = SLAVE_STATE_IDLE;
		complete(&dev->slave_complete);
#endif
	}

	return 1;
}

static irqreturn_t i2c_phytium_isr(int this_irq, void *dev_id)
{
	struct phytium_i2c_dev *dev = dev_id;
	u32 stat, enabled;

	spin_lock(&dev->i2c_lock);

#if IS_ENABLED(CONFIG_I2C_SLAVE)
	if (dev->mode == PHYTIUM_IC_SLAVE) {
		if (dev->slave_status == SLAVE_STATE_IDLE)
			dev->slave_status = SLAVE_STATE_ACTIVE;
		i2c_phytium_irq_handler_slave(dev);
		spin_unlock(&dev->i2c_lock);
		return IRQ_HANDLED;
	}
#endif
	enabled = phytium_readl(dev, IC_ENABLE);
	stat = phytium_readl(dev, IC_RAW_INTR_STAT);
	if (!enabled || !(stat & ~IC_INTR_ACTIVITY)) {
		spin_unlock(&dev->i2c_lock);
		return IRQ_NONE;
	}

	i2c_phytium_irq_handler_master(dev);
	spin_unlock(&dev->i2c_lock);
	return IRQ_HANDLED;
}

static int i2c_phytium_set_timings_master(struct phytium_i2c_dev *dev)
{
	const char *mode_str, *fp_str = "";
	u32 sda_falling_time, scl_falling_time;
	struct i2c_timings *t = &dev->timings;
	u32 ic_clk;
	int ret;

	/* Set standard and fast speed dividers for high/low periods */
	sda_falling_time = t->sda_fall_ns ?: 300; /* ns */
	scl_falling_time = t->scl_fall_ns ?: 300; /* ns */

	/* Calculate SCL timing parameters for standard mode if not set */
	if (!dev->ss_hcnt || !dev->ss_lcnt) {
		ic_clk = i2c_phytium_clk_rate(dev);
		dev->ss_hcnt = i2c_phytium_scl_hcnt(
			ic_clk, 4000, /* tHD;STA = tHIGH = 4.0 us */
			sda_falling_time, 0, /* 0: DW default, 1: Ideal */
			0); /* No offset */
		dev->ss_lcnt =
			i2c_phytium_scl_lcnt(ic_clk, 4700, /* tLOW = 4.7 us */
					     scl_falling_time,
					     0); /* No offset */
	}
	dev_dbg(dev->dev, "Standard Mode HCNT:LCNT = %d:%d\n", dev->ss_hcnt,
		dev->ss_lcnt);
	/*
	 * Set SCL timing parameters for fast mode or fast mode plus. Only
	 * difference is the timing parameter values since the registers
	 * are the same.
	 */
	if (t->bus_freq_hz == 1000000) {
		/*
		 * Check are fast mode plus parameters available and use
		 * fast mode if not.
		 */
		if (dev->fp_hcnt && dev->fp_lcnt) {
			dev->fs_hcnt = dev->fp_hcnt;
			dev->fs_lcnt = dev->fp_lcnt;
			fp_str = " Plus";
		}
	}
	/*
	 * Calculate SCL timing parameters for fast mode if not set. They are
	 * needed also in high speed mode.
	 */
	if (!dev->fs_hcnt || !dev->fs_lcnt) {
		ic_clk = i2c_phytium_clk_rate(dev);
		dev->fs_hcnt = i2c_phytium_scl_hcnt(
			ic_clk, 600, /* tHD;STA = tHIGH = 0.6 us */
			sda_falling_time, 0, /* 0: DW default, 1: Ideal */
			0); /* No offset */
		dev->fs_lcnt =
			i2c_phytium_scl_lcnt(ic_clk, 1300, /* tLOW = 1.3 us */
					     scl_falling_time,
					     0); /* No offset */
	}
	dev_dbg(dev->dev, "Fast Mode%s HCNT:LCNT = %d:%d\n", fp_str,
		dev->fs_hcnt, dev->fs_lcnt);

	if (dev->hs_hcnt && dev->hs_lcnt)
		dev_dbg(dev->dev, "High Speed Mode HCNT:LCNT = %d:%d\n",
			dev->hs_hcnt, dev->hs_lcnt);

	ret = i2c_phytium_set_sda_hold(dev);
	if (ret)
		goto out;

	switch (dev->master_cfg & IC_CON_SPEED_MASK) {
	case IC_CON_SPEED_STD:
		mode_str = "Standard Mode";
		break;
	case IC_CON_SPEED_HIGH:
		mode_str = "High Speed Mode";
		break;
	default:
		mode_str = "Fast Mode";
	}
	dev_dbg(dev->dev, "Bus speed: %s%s\n", mode_str, fp_str);

out:
	return ret;
}

static int i2c_phytium_enable_smbus_alert(struct phytium_i2c_dev *i2c_dev)
{
	struct i2c_adapter *adap = &i2c_dev->adapter;
	struct i2c_smbus_alert_setup setup;

	setup.irq = 0;
	i2c_dev->ara = i2c_new_smbus_alert_device(adap, &setup);

	if (IS_ERR(i2c_dev->ara))
		return PTR_ERR(i2c_dev->ara);

	return 0;
}

int i2c_phytium_probe(struct phytium_i2c_dev *dev)
{
	struct i2c_adapter *adapter = &dev->adapter;
	unsigned long irq_flags;
	int ret;
	u32 alert = 0;

	init_completion(&dev->cmd_complete);
	if (dev->mode == PHYTIUM_IC_MASTER) {
		snprintf(adapter->name, sizeof(adapter->name),
					"Phytium I2C Adapter");
		dev->init = i2c_phytium_init_master;
	} else if (dev->mode == PHYTIUM_IC_SLAVE) {
		snprintf(adapter->name, sizeof(adapter->name),
					"Phytium I2C Slave Adapter");
		dev->init = i2c_phytium_init_slave;
	}
#if IS_ENABLED(CONFIG_I2C_SLAVE)
	init_completion(&dev->slave_complete);
	dev->slave_status = SLAVE_STATE_IDLE;
#endif

	dev->disable = i2c_phytium_disable;
	dev->disable_int = i2c_phytium_disable_int;
	ret = i2c_phytium_set_timings_master(dev);
	if (ret)
		return ret;

	ret = dev->init(dev);
	if (ret)
		return ret;

	if (dev->mode == PHYTIUM_IC_MASTER) {
		phytium_writel(dev, DEFAULT_TIMEOUT, IC_SMBCLK_LOW_MEXT);
		phytium_writel(dev, DEFAULT_TIMEOUT, IC_SMBCLK_LOW_TIMEOUT);
		phytium_writel(dev, DEFAULT_TIMEOUT, IC_SMBDAT_STUCK_TIMEOUT);
	}

	adapter->retries = 3;
	adapter->algo = &i2c_phytium_algo;
	adapter->dev.parent = dev->dev;
	i2c_set_adapdata(adapter, dev);

	irq_flags = IRQF_SHARED | IRQF_COND_SUSPEND;

	i2c_phytium_disable_int(dev);
	ret = devm_request_irq(dev->dev, dev->irq, i2c_phytium_isr, irq_flags,
			       dev_name(dev->dev), dev);
	if (ret) {
		dev_err(dev->dev, "failed to request irq %i: %d\n", dev->irq,
			ret);
		return ret;
	}

	/*
	 * Increment PM usage count during adapter registration in order to
	 * avoid possible spurious runtime suspend when adapter device is
	 * registered to the device core and immediate resume in case bus has
	 * registered I2C slaves that do I2C transfers in their probe.
	 */
	pm_runtime_get_noresume(dev->dev);
	ret = i2c_add_numbered_adapter(adapter);
	if (ret)
		dev_err(dev->dev, "fail to add adapter: %d\n", ret);

	if (of_property_read_bool(dev->dev->of_node, "smbus-alert"))
		alert = 1;
	else if (has_acpi_companion(dev->dev))
		fwnode_property_read_u32_array(dev->dev->fwnode, "smbus_alert", &alert, 1);

	if (alert) {
		ret = i2c_phytium_enable_smbus_alert(dev);
		if (ret) {
			dev_err(dev->dev,
			"failed to enable SMBus alert protocol (%d)\n", ret);
		}
	}

	pm_runtime_put_noidle(dev->dev);

	return ret;
}
EXPORT_SYMBOL_GPL(i2c_phytium_probe);

MODULE_AUTHOR("Wu Jinyong <wujinyong1788@phytium.com.cn>");
MODULE_DESCRIPTION("Phytium I2C bus controller adapter");
MODULE_LICENSE("GPL");
MODULE_VERSION(I2C_PHYTIUM_DRV_VERSION);
