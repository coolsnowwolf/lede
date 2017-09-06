/*
 * Cavium CNS3xxx I2C Host Controller
 *
 * Copyright 2010 Cavium Network
 * Copyright 2012 Gateworks Corporation
 *		  Chris Lang <clang@gateworks.com>
 *		  Tim Harvey <tharvey@gateworks.com>
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, Version 2, as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <asm/io.h>
#include <linux/wait.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/clk.h>

/*
 * We need the memory map
 */

#define I2C_MEM_MAP_ADDR(x)         (i2c->base + x)
#define I2C_MEM_MAP_VALUE(x)        (*((unsigned int volatile*)I2C_MEM_MAP_ADDR(x)))

#define I2C_CONTROLLER_REG                    I2C_MEM_MAP_VALUE(0x00)
#define I2C_TIME_OUT_REG                      I2C_MEM_MAP_VALUE(0x04)
#define I2C_SLAVE_ADDRESS_REG                 I2C_MEM_MAP_VALUE(0x08)
#define I2C_WRITE_DATA_REG                    I2C_MEM_MAP_VALUE(0x0C)
#define I2C_READ_DATA_REG                     I2C_MEM_MAP_VALUE(0x10)
#define I2C_INTERRUPT_STATUS_REG              I2C_MEM_MAP_VALUE(0x14)
#define I2C_INTERRUPT_ENABLE_REG              I2C_MEM_MAP_VALUE(0x18)
#define I2C_TWI_OUT_DLY_REG			         			I2C_MEM_MAP_VALUE(0x1C)

#define I2C_BUS_ERROR_FLAG     (0x1)
#define I2C_ACTION_DONE_FLAG   (0x2)

#define CNS3xxx_I2C_ENABLE()          (I2C_CONTROLLER_REG) |= ((unsigned int)0x1 << 31)
#define CNS3xxx_I2C_DISABLE()         (I2C_CONTROLLER_REG) &= ~((unsigned int)0x1 << 31)
#define CNS3xxx_I2C_ENABLE_INTR()     (I2C_INTERRUPT_ENABLE_REG) |= 0x03
#define CNS3xxx_I2C_DISABLE_INTR()    (I2C_INTERRUPT_ENABLE_REG) &= 0xfc

#define TWI_TIMEOUT         (10*HZ)
#define I2C_100KHZ          100000
#define I2C_200KHZ          200000
#define I2C_300KHZ          300000
#define I2C_400KHZ          400000

#define CNS3xxx_I2C_CLK     I2C_100KHZ

#define STATE_DONE		1
#define STATE_ERROR		2

struct cns3xxx_i2c {
	struct device		*dev;
	void __iomem		*base;		/* virtual */
	wait_queue_head_t	wait;
	struct i2c_adapter	adap;
	struct i2c_msg		*msg;
	u8			state;		/* see STATE_ */
	u8			error;		/* see TWI_STATUS register */
	int			rd_wr_len;
	u8			*buf;
};

static u32 cns3xxx_i2c_func(struct i2c_adapter *adap)
{
	return I2C_FUNC_I2C | I2C_FUNC_SMBUS_EMUL;
}

static int
cns3xxx_i2c_xfer_msg(struct i2c_adapter *adap, struct i2c_msg *msg)
{
	struct cns3xxx_i2c *i2c = i2c_get_adapdata(adap);
	int i, j;
	u8 buf[1] = { 0 };

	if (msg->len == 0) {
		/*
		 *	We are probably doing a probe for a device here,
		 *	so set the length to one, and data to 0
		 */
		msg->len = 1;
		i2c->buf = buf;
	} else {
		i2c->buf = msg->buf;
	}

	if (msg->flags & I2C_M_TEN) {
		printk
			("%s:%d: Presently the driver does not handle extended addressing\n",
				__FUNCTION__, __LINE__);
		return -EINVAL;
	}
	i2c->msg = msg;

	for (i = 0; i < msg->len; i++) {
		if (msg->len - i >= 4)
			i2c->rd_wr_len = 3;
		else
			i2c->rd_wr_len = msg->len - i - 1;

		// Set Data Width	and TWI_EN
		I2C_CONTROLLER_REG = 0x80000000 | (i2c->rd_wr_len << 2) | (i2c->rd_wr_len);

		// Clear Write Reg
		I2C_WRITE_DATA_REG = 0;

		// Set the slave address
		I2C_SLAVE_ADDRESS_REG = (msg->addr << 1);

		// Are we Writing
		if (!(msg->flags & I2C_M_RD)) {
			I2C_CONTROLLER_REG |= (1 << 4);
			if (i != 0) {
				/*
				 * We need to set the address in the first byte.
				 * The base address is going to be in buf[0] and then
				 * it needs to be incremented by i - 1.
				 */
				i2c->buf--;
				*i2c->buf = buf[0] + i - 1;

				if (i2c->rd_wr_len < 3) {
					i += i2c->rd_wr_len;
					i2c->rd_wr_len++;
					I2C_CONTROLLER_REG = 0x80000000 | (1 << 4) | (i2c->rd_wr_len << 2) | (i2c->rd_wr_len);
				} else {
					i += i2c->rd_wr_len - 1;
				}
			} else {
				i += i2c->rd_wr_len;
				buf[0] = *i2c->buf;
			}
			for (j = 0; j <= i2c->rd_wr_len; j++) {
				I2C_WRITE_DATA_REG |= ((*i2c->buf++) << (8 * j));
			}
		} else {
			i += i2c->rd_wr_len;
		}

		// Start the Transfer
		i2c->state = 0;		// Clear out the State
		i2c->error = 0;
		I2C_CONTROLLER_REG |= (1 << 6);

		if (wait_event_timeout(i2c->wait, (i2c->state == STATE_ERROR) ||
				(i2c->state == STATE_DONE), TWI_TIMEOUT)) {
			if (i2c->state == STATE_ERROR) {
				dev_dbg(i2c->dev, "controller error: 0x%2x", i2c->error);
				return -EAGAIN; // try again
			}
		} else {
			dev_err(i2c->dev, "controller timed out "
				"waiting for start condition to finish\n");
			return -ETIMEDOUT;
		}
	}
	return 0;
}

static int
cns3xxx_i2c_xfer(struct i2c_adapter *adap, struct i2c_msg *msgs, int num)
{
	int i;
	int ret;
	for (i = 0; i < num; i++)
	{
		ret = cns3xxx_i2c_xfer_msg(adap, &msgs[i]);
		if (ret < 0) {
			return ret;
		}
	}
	return num;
}


static struct i2c_algorithm cns3xxx_i2c_algo = {
	.master_xfer = cns3xxx_i2c_xfer,
	.functionality = cns3xxx_i2c_func,
};

static struct i2c_adapter cns3xxx_i2c_adapter = {
	.owner = THIS_MODULE,
	.algo = &cns3xxx_i2c_algo,
	.algo_data = NULL,
	.nr = 0,
	.name = "CNS3xxx I2C 0",
	.retries = 5,
};

static void cns3xxx_i2c_adapter_init(struct cns3xxx_i2c *i2c)
{
	struct clk *clk;

	clk = devm_clk_get(i2c->dev, "cpu");
	if (WARN_ON(!clk))
		return;

	/* Disable the I2C */
	I2C_CONTROLLER_REG = 0;	/* Disabled the I2C */

	/* Check the Reg Dump when testing */
	I2C_TIME_OUT_REG =
	    (((((clk_get_rate(clk) / (2 * CNS3xxx_I2C_CLK)) -
		1) & 0x3FF) << 8) | (1 << 7) | 0x7F);
	I2C_TWI_OUT_DLY_REG |= 0x3;

	/* Enable The Interrupt */
	CNS3xxx_I2C_ENABLE_INTR();

	/* Clear Interrupt Status (0x2 | 0x1) */
	I2C_INTERRUPT_STATUS_REG |= (I2C_ACTION_DONE_FLAG | I2C_BUS_ERROR_FLAG);

	/* Enable the I2C Controller */
	CNS3xxx_I2C_ENABLE();
}

static irqreturn_t cns3xxx_i2c_isr(int irq, void *dev_id)
{
	struct cns3xxx_i2c *i2c = dev_id;
	int i;
	uint32_t stat = I2C_INTERRUPT_STATUS_REG;

	/* Clear Interrupt */
	I2C_INTERRUPT_STATUS_REG |= 0x1;

	if (stat & I2C_BUS_ERROR_FLAG) {
		i2c->state = STATE_ERROR;
		i2c->error = (I2C_INTERRUPT_STATUS_REG & 0xff00)>>8;
	} else {
		if (i2c->msg->flags & I2C_M_RD) {
			for (i = 0; i <= i2c->rd_wr_len; i++)
			{
				*i2c->buf++ = ((I2C_READ_DATA_REG >> (8 * i)) & 0xff);
			}
		}
		i2c->state = STATE_DONE;
	}
	wake_up(&i2c->wait);
	return IRQ_HANDLED;
}

static int cns3xxx_i2c_probe(struct platform_device *pdev)
{
	struct cns3xxx_i2c *i2c;
	struct resource *res, *res2;
	int ret;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		printk("%s: IORESOURCE_MEM not defined \n", __FUNCTION__);
		return -ENODEV;
	}

	res2 = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if (!res2) {
		printk("%s: IORESOURCE_IRQ not defined \n", __FUNCTION__);
		return -ENODEV;
	}

	i2c = kzalloc(sizeof(*i2c), GFP_KERNEL);
	if (!i2c)
		return -ENOMEM;

	if (!request_mem_region(res->start, res->end - res->start + 1,
				pdev->name)) {
		dev_err(&pdev->dev, "Memory region busy\n");
		ret = -EBUSY;
		goto request_mem_failed;
	}

	i2c->dev = &pdev->dev;
	i2c->base = ioremap(res->start, res->end - res->start + 1);
	if (!i2c->base) {
		dev_err(&pdev->dev, "Unable to map registers\n");
		ret = -EIO;
		goto map_failed;
	}

	cns3xxx_i2c_adapter_init(i2c);

	init_waitqueue_head(&i2c->wait);
	ret = request_irq(res2->start, cns3xxx_i2c_isr, 0, pdev->name, i2c);
	if (ret) {
		dev_err(&pdev->dev, "Cannot claim IRQ\n");
		goto request_irq_failed;
	}

	platform_set_drvdata(pdev, i2c);
	i2c->adap = cns3xxx_i2c_adapter;
	i2c_set_adapdata(&i2c->adap, i2c);
	i2c->adap.dev.parent = &pdev->dev;

	/* add i2c adapter to i2c tree */
	ret = i2c_add_numbered_adapter(&i2c->adap);
	if (ret) {
		dev_err(&pdev->dev, "Failed to add adapter\n");
		goto add_adapter_failed;
	}

	return 0;

      add_adapter_failed:
	free_irq(res2->start, i2c);
      request_irq_failed:
	iounmap(i2c->base);
      map_failed:
	release_mem_region(res->start, res->end - res->start + 1);
      request_mem_failed:
	kfree(i2c);

	return ret;
}

static int cns3xxx_i2c_remove(struct platform_device *pdev)
{
	struct cns3xxx_i2c *i2c = platform_get_drvdata(pdev);
	struct resource *res;

	/* disable i2c logic */
	CNS3xxx_I2C_DISABLE_INTR();
	CNS3xxx_I2C_DISABLE();
	/* remove adapter & data */
	i2c_del_adapter(&i2c->adap);
	platform_set_drvdata(pdev, NULL);

	res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if (res)
		free_irq(res->start, i2c);

	iounmap(i2c->base);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res)
		release_mem_region(res->start, res->end - res->start + 1);

	kfree(i2c);

	return 0;
}

static struct platform_driver cns3xxx_i2c_driver = {
	.probe = cns3xxx_i2c_probe,
	.remove = cns3xxx_i2c_remove,
	.driver = {
		.owner = THIS_MODULE,
		.name = "cns3xxx-i2c",
	},
};

static int __init cns3xxx_i2c_init(void)
{
	return platform_driver_register(&cns3xxx_i2c_driver);
}

static void __exit cns3xxx_i2c_exit(void)
{
	platform_driver_unregister(&cns3xxx_i2c_driver);
}

module_init(cns3xxx_i2c_init);
module_exit(cns3xxx_i2c_exit);

MODULE_AUTHOR("Cavium Networks");
MODULE_DESCRIPTION("Cavium CNS3XXX I2C Controller");
MODULE_LICENSE("GPL");
