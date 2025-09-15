// SPDX-License-Identifier: GPL-2.0
/*
 * drivers/w1/masters/phytium_w1m.c
 *
 * Copyright (c) 2021-2023 Phytium Technology Co., Ltd.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/sched.h>
#include <linux/pm_runtime.h>
#include <linux/of.h>
#include <linux/delay.h>
#include <linux/acpi.h>
#include <linux/w1.h>

#define PHY_W1M_CTL                             0x08

/* Simplify mode */
#define PHY_W1M_CMD                             0x04
#define PHY_W1M_PWM0_START_B                    0x30
#define PHY_W1M_PWM0_END_B                      0x34
#define PHY_W1M_PWM1_START_B                    0x38
#define PHY_W1M_PWM1_END_B                      0x3c
#define PHY_W1M_SAMPLE_B                        0x40
#define PHY_W1M_INT_EN_B                        0x64
#define PHY_W1M_INT_STATUS_B                    0x74
#define PHY_W1M_DATA_REG                        0x70

#define PHY_W1M_CMD_ROM_SEARCH                  0xF0
#define PHY_W1M_CMD_WRITE_BIT                   0x35
#define PHY_W1M_CMD_WRITE_BYTE                  0x36
#define PHY_W1M_CMD_RESET_BUS                   0x37
#define PHY_W1M_CMD_READ_BIT                    0x3A
#define PHY_W1M_CMD_READ_BYTE                   0x3B
#define PHY_W1M_SLAVE_ROM_ID                    0x160

#define PHY_W1M_INT_EN_TXCOMPLETE               BIT(6)
#define PHY_W1M_INT_EN_RXCOMPLETE               BIT(7)

#define PHY_W1M_INT_STATUS_TXCOMPLETE           BIT(6)
#define PHY_W1M_INT_STATUS_RXCOMPLETE           BIT(7)

#define W1M_MOD_W1                              1
#define W1M_MOD_PECI                            0

#define PHY_W1M_FLAG_CLEAR                      0
#define PHY_W1M_FLAG_SET                        1
#define PHY_W1M_TIMEOUT                         (HZ/5)

#define PHY_W1M_MAX_USER			4

static DECLARE_WAIT_QUEUE_HEAD(w1m_wait_queue);

struct w1m_data {
	struct device           *dev;
	void __iomem            *w1m_base;
	/* lock status update */
	struct  mutex           w1m_mutex;
	int                     w1m_usecount;
	u8                      w1m_irqstatus;
	/* device lock */
	spinlock_t              w1m_spinlock;
	/*
	 * Used to control the call to phytium_w1m_get and phytium_w1m_put.
	 * Non-w1 Protocol: Write the CMD|REG_address first, followed by
	 * the data wrire or read.
	 */
	int                     init_trans;
};

/* W1 register I/O routines */
static inline u8 phytium_w1m_read(struct w1m_data *w1m_data, u32 offset)
{
	return readl(w1m_data->w1m_base + offset);
}

static inline void phytium_w1m_write(struct w1m_data *w1m_data, u32 offset, u8 val)
{
	writel(val, w1m_data->w1m_base + offset);
}

static inline u8 phytium_w1m_merge(struct w1m_data *w1m_data, u32 offset,
				   u8 val, u8 mask)
{
	u8 new_val = (__raw_readl(w1m_data->w1m_base + offset) & ~mask)
			| (val & mask);
	writel(new_val, w1m_data->w1m_base + offset);

	return new_val;
}

static void w1m_disable_interrupt(struct w1m_data *w1m_data, u32 offset,
				  u32 mask)
{
	u32 ie;

	ie = readl(w1m_data->w1m_base + offset);
	writel(ie & mask, w1m_data->w1m_base + offset);
}

/* write out a byte or bit and fill *status with W1M_INT_STATUS */
static int phytium_write_data(struct w1m_data *w1m_data, u8 val, u8 *status, bool is_bit)
{
	int ret;
	unsigned long irqflags;

	*status = 0;

	spin_lock_irqsave(&w1m_data->w1m_spinlock, irqflags);
	/* clear interrupt flags via a dummy read */
	phytium_w1m_read(w1m_data, PHY_W1M_INT_STATUS_B);
	/* ISR loads it with new INT_STATUS */
	w1m_data->w1m_irqstatus = 0;
	spin_unlock_irqrestore(&w1m_data->w1m_spinlock, irqflags);

	phytium_w1m_merge(w1m_data, PHY_W1M_INT_EN_B,
			  PHY_W1M_INT_EN_TXCOMPLETE,
			  PHY_W1M_INT_EN_TXCOMPLETE);

	phytium_w1m_write(w1m_data, PHY_W1M_DATA_REG, val);
	phytium_w1m_write(w1m_data, PHY_W1M_CMD, is_bit ? PHY_W1M_CMD_WRITE_BIT : PHY_W1M_CMD_WRITE_BYTE);

	/* wait for the TXCOMPLETE bit */
	ret = wait_event_timeout(w1m_wait_queue,
		w1m_data->w1m_irqstatus, PHY_W1M_TIMEOUT);
	if (ret == 0) {
		dev_err(w1m_data->dev, "TX wait elapsed\n");
		ret = -ETIMEDOUT;
		goto out;
	}

	*status = w1m_data->w1m_irqstatus;
	/* check irqstatus */
	if (!(*status & PHY_W1M_INT_STATUS_TXCOMPLETE)) {
		dev_err(w1m_data->dev,
			"timeout waiting for TXCOMPLETE/RXCOMPLETE, %x", *status);
		ret = -ETIMEDOUT;
	}

out:
	return ret;
}

static irqreturn_t w1m_isr(int irq, void *_w1m)
{
	struct w1m_data *w1m_data = _w1m;
	unsigned long irqflags;

	spin_lock_irqsave(&w1m_data->w1m_spinlock, irqflags);
	w1m_data->w1m_irqstatus = phytium_w1m_read(w1m_data, PHY_W1M_INT_STATUS_B);
	spin_unlock_irqrestore(&w1m_data->w1m_spinlock, irqflags);

	phytium_w1m_write(w1m_data, PHY_W1M_INT_STATUS_B, 0x00);
	if (w1m_data->w1m_irqstatus &
		(PHY_W1M_INT_STATUS_TXCOMPLETE | PHY_W1M_INT_STATUS_RXCOMPLETE)) {
		/* wake up sleeping process */
		wake_up(&w1m_wait_queue);
	}

	return IRQ_HANDLED;
}


static int phytium_read_data(struct w1m_data *w1m_data, u8 *val, bool is_bit)
{
	int ret = 0;
	u8 status;
	unsigned long irqflags;

	ret = mutex_lock_interruptible(&w1m_data->w1m_mutex);
	if (ret < 0) {
		ret = -EINTR;
		goto rtn;
	}

	if (!w1m_data->w1m_usecount) {
		ret = -EINVAL;
		goto out;
	}

	spin_lock_irqsave(&w1m_data->w1m_spinlock, irqflags);
	/* clear interrupt flags via a dummy read */
	phytium_w1m_read(w1m_data, PHY_W1M_INT_STATUS_B);
	/* ISR loads it with new INT_STATUS */
	w1m_data->w1m_irqstatus = 0;
	spin_unlock_irqrestore(&w1m_data->w1m_spinlock, irqflags);

	phytium_w1m_merge(w1m_data, PHY_W1M_INT_EN_B,
			  PHY_W1M_INT_EN_RXCOMPLETE,
			  PHY_W1M_INT_EN_RXCOMPLETE);

	phytium_w1m_write(w1m_data, PHY_W1M_CMD, is_bit ? PHY_W1M_CMD_READ_BIT : PHY_W1M_CMD_READ_BYTE);

	wait_event_timeout(w1m_wait_queue,
			   (w1m_data->w1m_irqstatus & PHY_W1M_INT_STATUS_RXCOMPLETE),
			   PHY_W1M_TIMEOUT);

	status = w1m_data->w1m_irqstatus;
	/* check irqstatus */
	if (!(status & PHY_W1M_INT_STATUS_RXCOMPLETE)) {
		dev_err(w1m_data->dev, "timeout waiting for RXCOMPLETE, %x", status);
		ret = -ETIMEDOUT;
		goto out;
	}

	/* the data is ready. Read it in! */
	*val = phytium_w1m_read(w1m_data, PHY_W1M_DATA_REG);
out:
	mutex_unlock(&w1m_data->w1m_mutex);
rtn:
	return ret;
}


static int phytium_w1m_get(struct w1m_data *w1m_data)
{
	int ret = 0;

	ret = mutex_lock_interruptible(&w1m_data->w1m_mutex);
	if (ret < 0) {
		ret = -EINTR;
		goto rtn;
	}

	if (w1m_data->w1m_usecount == PHY_W1M_MAX_USER) {
		dev_warn(w1m_data->dev, "attempt to exceed the max use count");
		ret = -EINVAL;
		goto out;
	} else {
		w1m_data->w1m_usecount++;
		try_module_get(THIS_MODULE);
		if (w1m_data->w1m_usecount == 1)
			pm_runtime_get_sync(w1m_data->dev);
	}

out:
	mutex_unlock(&w1m_data->w1m_mutex);
rtn:
	return ret;
}

/* Disable clocks to the module */
static int phytium_w1m_put(struct w1m_data *w1m_data)
{
	int ret = 0;

	ret = mutex_lock_interruptible(&w1m_data->w1m_mutex);
	if (ret < 0)
		return -EINTR;

	if (w1m_data->w1m_usecount == 0) {
		dev_warn(w1m_data->dev,
			"attempt to decrement use count when it is zero");
		ret = -EINVAL;
	} else {
		w1m_data->w1m_usecount--;
		module_put(THIS_MODULE);
		if (w1m_data->w1m_usecount == 0)
			pm_runtime_put_sync(w1m_data->dev);
	}
	mutex_unlock(&w1m_data->w1m_mutex);

	return ret;
}

/*
 * W1 triplet callback function - used for searching ROM addresses.
 * Registered only when controller is in 1-wire mode.
 */
static u8 phytium_w1_triplet(void *_w1m, u8 bdir)
{
	u8 id_bit, comp_bit;
	int err;
	u8 ret = 0x3; /* no slaves responded */
	struct w1m_data *w1m_data = _w1m;

	phytium_w1m_get(_w1m);

	err = mutex_lock_interruptible(&w1m_data->w1m_mutex);
	if (err < 0) {
		dev_err(w1m_data->dev, "Could not acquire mutex\n");
		goto rtn;
	}

	w1m_data->w1m_irqstatus = 0;
	/* read id_bit */
	phytium_w1m_merge(w1m_data, PHY_W1M_INT_EN_B,
			  PHY_W1M_INT_EN_RXCOMPLETE,
			  PHY_W1M_INT_EN_RXCOMPLETE);
	phytium_w1m_write(w1m_data, PHY_W1M_CMD, 0x3a);

	err = wait_event_timeout(w1m_wait_queue,
				 (w1m_data->w1m_irqstatus
				  & PHY_W1M_INT_STATUS_RXCOMPLETE),
				 PHY_W1M_TIMEOUT);
	if (err == 0) {
		dev_err(w1m_data->dev, "RX wait elapsed\n");
		goto out;
	}
	id_bit = (phytium_w1m_read(_w1m, PHY_W1M_DATA_REG) & 0x01);

	w1m_data->w1m_irqstatus = 0;
	/* read comp_bit */
	phytium_w1m_merge(w1m_data, PHY_W1M_INT_EN_B,
				PHY_W1M_INT_EN_RXCOMPLETE,
				PHY_W1M_INT_EN_RXCOMPLETE);
	phytium_w1m_write(w1m_data, PHY_W1M_CMD, 0x3a);
	err = wait_event_timeout(w1m_wait_queue,
				 (w1m_data->w1m_irqstatus
				  & PHY_W1M_INT_STATUS_RXCOMPLETE),
				 PHY_W1M_TIMEOUT);
	if (err == 0) {
		dev_err(w1m_data->dev, "RX wait elapsed\n");
		goto out;
	}
	comp_bit = (phytium_w1m_read(_w1m, PHY_W1M_DATA_REG) & 0x01);

	if (id_bit && comp_bit) {
		ret = 0x03;  /* no slaves responded */
		goto out;
	}
	if (!id_bit && !comp_bit) {
		/* Both bits are valid, take the direction given */
		ret = bdir ? 0x04 : 0;
	} else {
		/* Only one bit is valid, take that direction */
		bdir = id_bit;
		ret = id_bit ? 0x05 : 0x02;
	}

	w1m_data->w1m_irqstatus = 0;
	/* write bdir bit */
	phytium_w1m_merge(w1m_data, PHY_W1M_INT_EN_B,
				PHY_W1M_INT_EN_TXCOMPLETE,
				PHY_W1M_INT_EN_TXCOMPLETE);

	phytium_w1m_write(w1m_data, PHY_W1M_DATA_REG, bdir);
	phytium_w1m_write(w1m_data, PHY_W1M_CMD, 0x35);

	err = wait_event_timeout(w1m_wait_queue,
				 (w1m_data->w1m_irqstatus
				  & PHY_W1M_INT_STATUS_TXCOMPLETE),
				 PHY_W1M_TIMEOUT);
	if (err == 0) {
		dev_err(w1m_data->dev, "TX wait elapsed\n");
		goto out;
	}

out:
	mutex_unlock(&w1m_data->w1m_mutex);
rtn:
	phytium_w1m_put(_w1m);
	return ret;
}

static u8 phytium_w1_touch_bit(void *_w1m, u8 bit)
{
	u8 result = 0;
	int err;
	struct w1m_data *w1m_data = _w1m;

	phytium_w1m_get(w1m_data);

	w1m_data->init_trans++;

	err = mutex_lock_interruptible(&w1m_data->w1m_mutex);
	if (err < 0) {
		dev_err(w1m_data->dev, "Could not acquired mutex\n");
		goto rtn;
	}

	w1m_data->w1m_irqstatus = 0;

	phytium_w1m_merge(w1m_data, PHY_W1M_INT_EN_B,
			PHY_W1M_INT_EN_RXCOMPLETE,
			PHY_W1M_INT_EN_RXCOMPLETE);
	phytium_w1m_write(w1m_data, PHY_W1M_CMD, 0x3a);

	err = wait_event_timeout(w1m_wait_queue,
			(w1m_data->w1m_irqstatus
			 & PHY_W1M_INT_STATUS_RXCOMPLETE),
			PHY_W1M_TIMEOUT);
	if (err == 0) {
		dev_info(w1m_data->dev, "Rx wait elapsed\n");
		goto out;
	}

	result = (phytium_w1m_read(w1m_data, PHY_W1M_DATA_REG) & 0x01);

	w1m_data->w1m_irqstatus = 0;

out:
	mutex_unlock(&w1m_data->w1m_mutex);

rtn:
	phytium_w1m_put(w1m_data);

	return result;
}

/* reset callback */
static u8 phytium_w1_reset_bus(void *_w1m)
{
	phytium_w1m_get(_w1m);
	phytium_w1m_write(_w1m, PHY_W1M_CMD, 0x37);
	mdelay(1);
	phytium_w1m_put(_w1m);
	return 0;
}

/* Read a byte of data from the device */
static u8 phytium_w1_read_data(void *_w1m, bool is_bit)
{
	struct w1m_data *w1m_data = _w1m;
	u8 val = 0;
	int ret;

	/* First write to initialize the transfer */
	if (w1m_data->init_trans == 0)
		phytium_w1m_get(w1m_data);

	w1m_data->init_trans++;
	ret = phytium_read_data(w1m_data, &val, is_bit);
	if (ret) {
		ret = mutex_lock_interruptible(&w1m_data->w1m_mutex);
		if (ret < 0) {
			dev_err(w1m_data->dev, "Could not acquire mutex\n");
			return -EINTR;
		}
		w1m_data->init_trans = 0;
		mutex_unlock(&w1m_data->w1m_mutex);
		phytium_w1m_put(w1m_data);
		return -1;
	}

	w1m_disable_interrupt(w1m_data, PHY_W1M_INT_EN_B,
			      ~((u32)PHY_W1M_INT_EN_RXCOMPLETE));

	/* Write followed by a read, release the module */
	if (w1m_data->init_trans) {
		ret = mutex_lock_interruptible(&w1m_data->w1m_mutex);
		if (ret < 0) {
			dev_err(w1m_data->dev, "Could not acquire mutex\n");
			return -EINTR;
		}
		w1m_data->init_trans = 0;
		mutex_unlock(&w1m_data->w1m_mutex);
		phytium_w1m_put(w1m_data);
	}

	return val;
}

static u8 phytium_w1_read_bit(void *_w1m)
{
	return phytium_w1_read_data(_w1m, true);
}

static u8 phytium_w1_read_byte(void *_w1m)
{
	return phytium_w1_read_data(_w1m, false);
}

/* Write a byte of data to the device */
static void phytium_w1_write_data(void *_w1m, u8 byte, bool is_bit)
{
	struct w1m_data *w1m_data = _w1m;
	int ret;
	u8 status;

	/* First write to initialize the transfer */
	if (w1m_data->init_trans == 0)
		phytium_w1m_get(w1m_data);

	ret = mutex_lock_interruptible(&w1m_data->w1m_mutex);
	if (ret < 0) {
		dev_err(w1m_data->dev, "Could not acquire mutex\n");
		return;
	}
	mutex_unlock(&w1m_data->w1m_mutex);

	ret = phytium_write_data(w1m_data, byte, &status, is_bit);
	if (ret < 0) {
		dev_err(w1m_data->dev, "TX failure:Ctrl status %x\n", status);
		return;
	}

	w1m_data->init_trans++;
	w1m_disable_interrupt(w1m_data, PHY_W1M_INT_EN_B,
			      ~((u32)PHY_W1M_INT_EN_TXCOMPLETE));
	/* Second write, data transferred. Release the module */
	if (w1m_data->init_trans > 1) {
		phytium_w1m_put(w1m_data);
		ret = mutex_lock_interruptible(&w1m_data->w1m_mutex);
		if (ret < 0) {
			dev_err(w1m_data->dev, "Could not acquire mutex\n");
			return;
		}
		w1m_data->init_trans = 0;
		mutex_unlock(&w1m_data->w1m_mutex);
	}
}

static void phytium_w1_write_bit(void *_w1m, u8 bit)
{
	phytium_w1_write_data(_w1m, bit, true);
}

static void phytium_w1_write_byte(void *_w1m, u8 bit)
{
	phytium_w1_write_data(_w1m, bit, false);
}

static struct w1_bus_master phytium_w1_master = {
	.read_byte	= phytium_w1_read_byte,
	.write_byte	= phytium_w1_write_byte,
	.read_bit	= phytium_w1_read_bit,
	.write_bit	= phytium_w1_write_bit,
	.reset_bus	= phytium_w1_reset_bus,
	.touch_bit	= phytium_w1_touch_bit,
};

static int phytium_w1m_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct w1m_data *w1m_data;
	struct resource *res;
	int ret, irq;

	w1m_data = devm_kzalloc(dev, sizeof(*w1m_data), GFP_KERNEL);
	if (!w1m_data)
		return -ENOMEM;

	w1m_data->dev = dev;
	platform_set_drvdata(pdev, w1m_data);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	w1m_data->w1m_base = devm_ioremap_resource(dev, res);
	if (IS_ERR(w1m_data->w1m_base))
		return PTR_ERR(w1m_data->w1m_base);

	w1m_data->w1m_usecount = 0;
	mutex_init(&w1m_data->w1m_mutex);

	pm_runtime_enable(&pdev->dev);
	ret = pm_runtime_get_sync(&pdev->dev);
	if (ret < 0) {
		dev_err(&pdev->dev, "pm_runtime_get_sync failed\n");
		goto err_w1;
	}

	spin_lock_init(&w1m_data->w1m_spinlock);

	irq = platform_get_irq(pdev, 0);
	if (irq	< 0) {
		dev_err(&pdev->dev, "Failed to get IRQ: %d\n", irq);
		ret = irq;
		goto err_irq;
	}

	ret = devm_request_irq(dev, irq, w1m_isr, 0, "phytium-w1", w1m_data);
	if (ret < 0) {
		dev_err(&pdev->dev, "could not request irq\n");
		goto err_irq;
	}

	pm_runtime_put_sync(&pdev->dev);

	phytium_w1_master.triplet = phytium_w1_triplet;
	phytium_w1m_write(w1m_data, PHY_W1M_CTL, 0x10);
	phytium_w1m_write(w1m_data, PHY_W1M_INT_EN_B, 0x00);

	phytium_w1_master.data = w1m_data;

	ret = w1_add_master_device(&phytium_w1_master);
	if (ret) {
		dev_err(&pdev->dev, "Failure in registering w1 master\n");
		goto err_w1;
	}

	return 0;

err_irq:
	pm_runtime_put_sync(&pdev->dev);
err_w1:
	pm_runtime_disable(&pdev->dev);

	return ret;
}

static int phytium_w1m_remove(struct platform_device *pdev)
{
	struct w1m_data *w1m_data = platform_get_drvdata(pdev);

	mutex_lock(&w1m_data->w1m_mutex);

	if (w1m_data->w1m_usecount) {
		dev_warn(&pdev->dev, "removed when use count is not zero\n");
		mutex_unlock(&w1m_data->w1m_mutex);
		return -EBUSY;
	}

	mutex_unlock(&w1m_data->w1m_mutex);

	/* remove module dependency */
	pm_runtime_disable(&pdev->dev);

	return 0;
}

static const struct of_device_id phytium_w1m_dt_ids[] = {
	{ .compatible = "phytium,w1" },
	{ }
};
MODULE_DEVICE_TABLE(of, phytium_w1m_dt_ids);

#ifdef CONFIG_ACPI
static const struct acpi_device_id phytium_w1m_acpi_ids[] = {
	{ "PHYT0034", 0 },
	{ }
};
#endif

static struct platform_driver phytium_w1m_driver = {
	.probe = phytium_w1m_probe,
	.remove = phytium_w1m_remove,
	.driver = {
		.name =	"phytium-w1",
		.of_match_table = phytium_w1m_dt_ids,
		.acpi_match_table = ACPI_PTR(phytium_w1m_acpi_ids),
	},
};
module_platform_driver(phytium_w1m_driver);

MODULE_AUTHOR("Zhu Mingshuai <zhumingshuai@phytium.com.cn>");
MODULE_DESCRIPTION("Phytium w1 bus master driver");
MODULE_LICENSE("GPL");
