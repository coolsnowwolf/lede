// SPDX-License-Identifier: GPL-2.0-only

#include <linux/module.h>
#include <linux/of_platform.h>
#include "i2c-rtl9300.h"

#define REG(i, x)	(i->base + x + (i->scl_num ? i->mst2_offset : 0))
#define REG_MASK(i, clear, set, reg)	\
			writel((readl(REG(i, reg)) & ~(clear)) | (set), REG(i, reg))

struct i2c_drv_data {
	int scl0_pin;
	int scl1_pin;
	int sda0_pin;
	struct i2c_algorithm *algo;
	int (*read)(struct rtl9300_i2c *i2c, u8 *buf, int len);
	int (*write)(struct rtl9300_i2c *i2c, u8 *buf, int len);
	void (*reg_addr_set)(struct rtl9300_i2c *i2c, u32 reg, u16 len);
	int (*config_xfer)(struct rtl9300_i2c *i2c, u16 addr, u16 len);
	int (*execute_xfer)(struct rtl9300_i2c *i2c, char read_write, int size,
			    union i2c_smbus_data * data, int len);
	void (*writel)(struct rtl9300_i2c *i2c, u32 data);
	void (*config_io)(struct rtl9300_i2c *i2c, int scl_num, int sda_num);
	u32 mst2_offset;
};

DEFINE_MUTEX(i2c_lock);

static void rtl9300_i2c_reg_addr_set(struct rtl9300_i2c *i2c, u32 reg, u16 len)
{
	/* Set register address width */
	REG_MASK(i2c, 0x3 << RTL9300_I2C_CTRL2_MADDR_WIDTH, len << RTL9300_I2C_CTRL2_MADDR_WIDTH,
		 RTL9300_I2C_CTRL2);

	/* Set register address */
	REG_MASK(i2c, 0xffffff << RTL9300_I2C_CTRL1_MEM_ADDR, reg << RTL9300_I2C_CTRL1_MEM_ADDR,
		 RTL9300_I2C_CTRL1);
}

static void rtl9310_i2c_reg_addr_set(struct rtl9300_i2c *i2c, u32 reg, u16 len)
{
	/* Set register address width */
	REG_MASK(i2c, 0x3 << RTL9310_I2C_CTRL_MADDR_WIDTH, len << RTL9310_I2C_CTRL_MADDR_WIDTH,
		 RTL9310_I2C_CTRL);

	/* Set register address */
	writel(reg, REG(i2c, RTL9310_I2C_MEMADDR));
}

static void rtl9300_i2c_config_io(struct rtl9300_i2c *i2c, int scl_num, int sda_num)
{
	u32 v;

	/* Set SCL pin */
	REG_MASK(i2c, 0, BIT(RTL9300_I2C_CTRL1_GPIO8_SCL_SEL), RTL9300_I2C_CTRL1);

	/* Set SDA pin */
	REG_MASK(i2c, 0x7 << RTL9300_I2C_CTRL1_SDA_OUT_SEL,
		 i2c->sda_num << RTL9300_I2C_CTRL1_SDA_OUT_SEL, RTL9300_I2C_CTRL1);

	/* Set SDA pin to I2C functionality */
	v = readl(i2c->base + RTL9300_I2C_MST_GLB_CTRL);
	v |= BIT(i2c->sda_num);
	writel(v, i2c->base + RTL9300_I2C_MST_GLB_CTRL);
}

static void rtl9310_i2c_config_io(struct rtl9300_i2c *i2c, int scl_num, int sda_num)
{
	u32 v;

	/* Set SCL pin */
	REG_MASK(i2c, 0, BIT(RTL9310_I2C_MST_IF_SEL_GPIO_SCL_SEL + scl_num), RTL9310_I2C_MST_IF_SEL);

	/* Set SDA pin */
	REG_MASK(i2c, 0x7 << RTL9310_I2C_CTRL_SDA_OUT_SEL,
		 i2c->sda_num << RTL9310_I2C_CTRL_SDA_OUT_SEL, RTL9310_I2C_CTRL);

	/* Set SDA pin to I2C functionality */
	v = readl(i2c->base + RTL9310_I2C_MST_IF_SEL);
	v |= BIT(i2c->sda_num);
	writel(v, i2c->base + RTL9310_I2C_MST_IF_SEL);
}

static int rtl9300_i2c_config_xfer(struct rtl9300_i2c *i2c, u16 addr, u16 len)
{
	/* Set bus frequency */
	REG_MASK(i2c, 0x3 << RTL9300_I2C_CTRL2_SCL_FREQ,
		 i2c->bus_freq << RTL9300_I2C_CTRL2_SCL_FREQ, RTL9300_I2C_CTRL2);

	/* Set slave device address */
	REG_MASK(i2c, 0x7f << RTL9300_I2C_CTRL2_DEV_ADDR,
		 addr << RTL9300_I2C_CTRL2_DEV_ADDR, RTL9300_I2C_CTRL2);

	/* Set data length */
	REG_MASK(i2c, 0xf << RTL9300_I2C_CTRL2_DATA_WIDTH,
		 ((len - 1) & 0xf) << RTL9300_I2C_CTRL2_DATA_WIDTH, RTL9300_I2C_CTRL2);

	/* Set read mode to random */
	REG_MASK(i2c, 0x1 << RTL9300_I2C_CTRL2_READ_MODE, 0, RTL9300_I2C_CTRL2);

	return 0;
}

static int rtl9310_i2c_config_xfer(struct rtl9300_i2c *i2c, u16 addr, u16 len)
{
	/* Set bus frequency */
	REG_MASK(i2c, 0x3 << RTL9310_I2C_CTRL_SCL_FREQ,
		 i2c->bus_freq << RTL9310_I2C_CTRL_SCL_FREQ, RTL9310_I2C_CTRL);

	/* Set slave device address */
	REG_MASK(i2c, 0x7f << RTL9310_I2C_CTRL_DEV_ADDR,
		 addr << RTL9310_I2C_CTRL_DEV_ADDR, RTL9310_I2C_CTRL);

	/* Set data length */
	REG_MASK(i2c, 0xf << RTL9310_I2C_CTRL_DATA_WIDTH,
		 ((len - 1) & 0xf) << RTL9310_I2C_CTRL_DATA_WIDTH, RTL9310_I2C_CTRL);

	/* Set read mode to random */
	REG_MASK(i2c, 0x1 << RTL9310_I2C_CTRL_READ_MODE, 0, RTL9310_I2C_CTRL);

	return 0;
}

static int i2c_read(void __iomem *r0, u8 *buf, int len)
{
	if (len > 16)
		return -EIO;

	for (int i = 0; i < len; i++) {
		u32 v;

		if (i % 4 == 0)
			v = readl(r0 + i);
		buf[i] = v;
		v >>= 8;
	}

	return len;
}

static int i2c_write(void __iomem *r0, u8 *buf, int len)
{
	if (len > 16)
		return -EIO;

	for (int i = 0; i < len; i++) {
		u32 v;

		if (! (i % 4))
			v = 0;
		v <<= 8;
		v |= buf[i];
		if (i % 4 == 3 || i == len - 1)
			writel(v, r0 + (i / 4) * 4);
	}

	return len;
}

static int rtl9300_i2c_read(struct rtl9300_i2c *i2c, u8 *buf, int len)
{
	return i2c_read(REG(i2c, RTL9300_I2C_DATA_WORD0), buf, len);
}

static int rtl9300_i2c_write(struct rtl9300_i2c *i2c, u8 *buf, int len)
{
	return i2c_write(REG(i2c, RTL9300_I2C_DATA_WORD0), buf, len);
}

static int rtl9310_i2c_read(struct rtl9300_i2c *i2c, u8 *buf, int len)
{
	return i2c_read(REG(i2c, RTL9310_I2C_DATA), buf, len);
}

static int rtl9310_i2c_write(struct rtl9300_i2c *i2c, u8 *buf, int len)
{
	return i2c_write(REG(i2c, RTL9310_I2C_DATA), buf, len);
}

static void rtl9300_writel(struct rtl9300_i2c *i2c, u32 data)
{
	writel(data, REG(i2c, RTL9300_I2C_DATA_WORD0));
}

static void rtl9310_writel(struct rtl9300_i2c *i2c, u32 data)
{
	writel(data, REG(i2c, RTL9310_I2C_DATA));
}


static int rtl9300_execute_xfer(struct rtl9300_i2c *i2c, char read_write,
				int size, union i2c_smbus_data * data, int len)
{
	u32 v;

	if (read_write == I2C_SMBUS_READ)
		REG_MASK(i2c, BIT(RTL9300_I2C_CTRL1_RWOP), 0, RTL9300_I2C_CTRL1);
	else
		REG_MASK(i2c, 0, BIT(RTL9300_I2C_CTRL1_RWOP), RTL9300_I2C_CTRL1);

	REG_MASK(i2c, 0, BIT(RTL9300_I2C_CTRL1_I2C_TRIG), RTL9300_I2C_CTRL1);
	do {
		v = readl(REG(i2c, RTL9300_I2C_CTRL1));
	} while (v & BIT(RTL9300_I2C_CTRL1_I2C_TRIG));

	if (v & BIT(RTL9300_I2C_CTRL1_I2C_FAIL))
		return -EIO;

	if (read_write == I2C_SMBUS_READ) {
		if (size == I2C_SMBUS_BYTE || size == I2C_SMBUS_BYTE_DATA){
			data->byte = readl(REG(i2c, RTL9300_I2C_DATA_WORD0));
		} else if (size == I2C_SMBUS_WORD_DATA) {
			data->word = readl(REG(i2c, RTL9300_I2C_DATA_WORD0));
		} else if (len > 0) {
			rtl9300_i2c_read(i2c, &data->block[0], len);
		}
	}

	return 0;
}

static int rtl9310_execute_xfer(struct rtl9300_i2c *i2c, char read_write,
				int size, union i2c_smbus_data * data, int len)
{
	u32 v;

	if (read_write == I2C_SMBUS_READ)
		REG_MASK(i2c, BIT(RTL9310_I2C_CTRL_RWOP), 0, RTL9310_I2C_CTRL);
	else
		REG_MASK(i2c, 0, BIT(RTL9310_I2C_CTRL_RWOP), RTL9310_I2C_CTRL);

	REG_MASK(i2c, 0, BIT(RTL9310_I2C_CTRL_I2C_TRIG), RTL9310_I2C_CTRL);
	do {
		v = readl(REG(i2c, RTL9310_I2C_CTRL));
	} while (v & BIT(RTL9310_I2C_CTRL_I2C_TRIG));

	if (v & BIT(RTL9310_I2C_CTRL_I2C_FAIL))
		return -EIO;

	if (read_write == I2C_SMBUS_READ) {
		if (size == I2C_SMBUS_BYTE || size == I2C_SMBUS_BYTE_DATA){
			data->byte = readl(REG(i2c, RTL9310_I2C_DATA));
		} else if (size == I2C_SMBUS_WORD_DATA) {
			data->word = readl(REG(i2c, RTL9310_I2C_DATA));
		} else if (len > 0) {
			rtl9310_i2c_read(i2c, &data->block[0], len);
		}
	}

	return 0;
}

static int rtl9300_i2c_smbus_xfer(struct i2c_adapter * adap, u16 addr,
		  unsigned short flags, char read_write,
		  u8 command, int size, union i2c_smbus_data * data)
{
	struct rtl9300_i2c *i2c = i2c_get_adapdata(adap);
	struct i2c_drv_data *drv_data = (struct i2c_drv_data *)device_get_match_data(i2c->dev);
	int len = 0, ret;

	mutex_lock(&i2c_lock);
	switch (size) {
	case I2C_SMBUS_QUICK:
		drv_data->config_xfer(i2c, addr, 0);
		drv_data->reg_addr_set(i2c, 0, 0);
		break;

	case I2C_SMBUS_BYTE:
		if (read_write == I2C_SMBUS_WRITE) {
			drv_data->config_xfer(i2c, addr, 0);
			drv_data->reg_addr_set(i2c, command, 1);
		} else {
			drv_data->config_xfer(i2c, addr, 1);
			drv_data->reg_addr_set(i2c, 0, 0);
		}
		break;

	case I2C_SMBUS_BYTE_DATA:
		pr_debug("I2C_SMBUS_BYTE_DATA %02x, read %d cmd %02x\n", addr, read_write, command);
		drv_data->reg_addr_set(i2c, command, 1);
		drv_data->config_xfer(i2c, addr, 1);

		if (read_write == I2C_SMBUS_WRITE) {
			pr_debug("--> data %02x\n", data->byte);
			drv_data->writel(i2c, data->byte);
		}
		break;

	case I2C_SMBUS_WORD_DATA:
		pr_debug("I2C_SMBUS_WORD %02x, read %d\n", addr, read_write);
		drv_data->reg_addr_set(i2c, command, 1);
		drv_data->config_xfer(i2c, addr, 2);
		if (read_write == I2C_SMBUS_WRITE)
			drv_data->writel(i2c, data->word);
		break;

	case I2C_SMBUS_BLOCK_DATA:
		pr_debug("I2C_SMBUS_BLOCK_DATA %02x, read %d, len %d\n",
			addr, read_write, data->block[0]);
		drv_data->reg_addr_set(i2c, command, 1);
		drv_data->config_xfer(i2c, addr, data->block[0]);
		if (read_write == I2C_SMBUS_WRITE)
			drv_data->write(i2c, &data->block[1], data->block[0]);
		len = data->block[0];
		break;

	default:
		dev_warn(&adap->dev, "Unsupported transaction %d\n", size);
		return -EOPNOTSUPP;
	}

	ret = drv_data->execute_xfer(i2c, read_write, size, data, len);

	mutex_unlock(&i2c_lock);

	return ret;
}

static u32 rtl9300_i2c_func(struct i2c_adapter *a)
{
	return I2C_FUNC_SMBUS_QUICK | I2C_FUNC_SMBUS_BYTE |
	       I2C_FUNC_SMBUS_BYTE_DATA | I2C_FUNC_SMBUS_WORD_DATA |
	       I2C_FUNC_SMBUS_BLOCK_DATA;
}

static const struct i2c_algorithm rtl9300_i2c_algo = {
	.smbus_xfer	= rtl9300_i2c_smbus_xfer,
	.functionality	= rtl9300_i2c_func,
};

struct i2c_adapter_quirks rtl9300_i2c_quirks = {
	.flags		= I2C_AQ_NO_CLK_STRETCH,
	.max_read_len	= 16,
	.max_write_len	= 16,
};

static int rtl9300_i2c_probe(struct platform_device *pdev)
{
	struct rtl9300_i2c *i2c;
	struct i2c_adapter *adap;
	struct i2c_drv_data *drv_data;
	struct device_node *node = pdev->dev.of_node;
	u32 clock_freq, pin;
	int ret = 0;

	pr_info("%s probing I2C adapter\n", __func__);

	if (!node) {
		dev_err(i2c->dev, "No DT found\n");
		return -EINVAL;
	}

	drv_data = (struct i2c_drv_data *) device_get_match_data(&pdev->dev);

	i2c = devm_kzalloc(&pdev->dev, sizeof(struct rtl9300_i2c), GFP_KERNEL);
	if (!i2c)
		return -ENOMEM;

	i2c->base = devm_platform_ioremap_resource(pdev, 0);
	i2c->mst2_offset = drv_data->mst2_offset;
	if (IS_ERR(i2c->base))
		return PTR_ERR(i2c->base);

	pr_debug("%s base memory %08x\n", __func__, (u32)i2c->base);
	i2c->dev = &pdev->dev;

	if (of_property_read_u32(node, "clock-frequency", &clock_freq)) {
		clock_freq = I2C_MAX_STANDARD_MODE_FREQ;
	}
	switch(clock_freq) {
	case I2C_MAX_STANDARD_MODE_FREQ:
		i2c->bus_freq = RTL9300_I2C_STD_FREQ;
		break;

	case I2C_MAX_FAST_MODE_FREQ:
		i2c->bus_freq = RTL9300_I2C_FAST_FREQ;
		break;
	default:
		dev_warn(i2c->dev, "clock-frequency %d not supported\n", clock_freq);
		return -EINVAL;
	}

	dev_info(&pdev->dev, "SCL speed %d, mode is %d\n", clock_freq, i2c->bus_freq);

	if (of_property_read_u32(node, "scl-pin", &pin)) {
		dev_warn(i2c->dev, "SCL pin not found in DT, using default\n");
		pin = drv_data->scl0_pin;
	}
	if (!(pin == drv_data->scl0_pin || pin == drv_data->scl1_pin)) {
		dev_warn(i2c->dev, "SCL pin %d not supported\n", pin);
		return -EINVAL;
	}
	i2c->scl_num = pin == drv_data->scl0_pin ? 0 : 1;
	pr_info("%s scl_num %d\n", __func__, i2c->scl_num);

	if (of_property_read_u32(node, "sda-pin", &pin)) {
		dev_warn(i2c->dev, "SDA pin not found in DT, using default \n");
		pin = drv_data->sda0_pin;
	}
	i2c->sda_num = pin - drv_data->sda0_pin;
	if (i2c->sda_num < 0 || i2c->sda_num > 7) {
		dev_warn(i2c->dev, "SDA pin %d not supported\n", pin);
		return -EINVAL;
	}
	pr_info("%s sda_num %d\n", __func__, i2c->sda_num);

	adap = &i2c->adap;
	adap->owner = THIS_MODULE;
	adap->algo = &rtl9300_i2c_algo;
	adap->retries = 3;
	adap->dev.parent = &pdev->dev;
	i2c_set_adapdata(adap, i2c);
	adap->dev.of_node = node;
	strlcpy(adap->name, dev_name(&pdev->dev), sizeof(adap->name));

	platform_set_drvdata(pdev, i2c);

	drv_data->config_io(i2c, i2c->scl_num, i2c->sda_num);

	ret = i2c_add_adapter(adap);

	return ret;
}

static int rtl9300_i2c_remove(struct platform_device *pdev)
{
	struct rtl9300_i2c *i2c = platform_get_drvdata(pdev);

	i2c_del_adapter(&i2c->adap);

	return 0;
}

struct i2c_drv_data rtl9300_i2c_drv_data = {
	.scl0_pin = 8,
	.scl1_pin = 17,
	.sda0_pin = 9,
	.read = rtl9300_i2c_read,
	.read = rtl9300_i2c_write,
	.reg_addr_set = rtl9300_i2c_reg_addr_set,
	.config_xfer = rtl9300_i2c_config_xfer,
	.execute_xfer = rtl9300_execute_xfer,
	.writel = rtl9300_writel,
	.config_io = rtl9300_i2c_config_io,
	.mst2_offset = 0x1c,
};

struct i2c_drv_data rtl9310_i2c_drv_data = {
	.scl0_pin = 13,
	.scl1_pin = 14,
	.sda0_pin = 0,
	.read = rtl9310_i2c_read,
	.read = rtl9310_i2c_write,
	.reg_addr_set = rtl9310_i2c_reg_addr_set,
	.config_xfer = rtl9310_i2c_config_xfer,
	.execute_xfer = rtl9310_execute_xfer,
	.writel = rtl9310_writel,
	.config_io = rtl9310_i2c_config_io,
	.mst2_offset = 0x18,
};

static const struct of_device_id i2c_rtl9300_dt_ids[] = {
	{ .compatible = "realtek,rtl9300-i2c", .data = (void *) &rtl9300_i2c_drv_data },
	{ .compatible = "realtek,rtl9310-i2c", .data = (void *) &rtl9310_i2c_drv_data },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, rtl838x_eth_of_ids);

static struct platform_driver rtl9300_i2c_driver = {
	.probe		= rtl9300_i2c_probe,
	.remove		= rtl9300_i2c_remove,
	.driver		= {
		.name	= "i2c-rtl9300",
		.pm 	= NULL,
		.of_match_table = i2c_rtl9300_dt_ids,
	},
};

module_platform_driver(rtl9300_i2c_driver);

MODULE_AUTHOR("Birger Koblitz");
MODULE_DESCRIPTION("RTL9300 I2C host driver");
MODULE_LICENSE("GPL v2");
