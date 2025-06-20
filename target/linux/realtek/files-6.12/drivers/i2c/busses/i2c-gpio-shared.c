// SPDX-License-Identifier: GPL-2.0
/*
 * Bitbanging driver for multiple I2C busses with shared SCL pin using the GPIO API
 * Copyright (c) 2025 Markus Stockhausen <markus.stockhausen at gmx.de>
 */

#include <linux/i2c-algo-bit.h>
#include <linux/gpio/consumer.h>
#include <linux/mod_devicetable.h>
#include <linux/mutex.h>
#include <linux/platform_device.h>

struct gpio_shared_ctx;

struct gpio_shared_bus {
	int num;
	struct gpio_desc *sda;
	struct i2c_adapter adap;
	struct i2c_algo_bit_data bit_data;
	struct gpio_shared_ctx *ctx;
};

#define GPIO_SHARED_MAX_BUS	4

struct gpio_shared_ctx {
	struct device *dev;
	struct gpio_desc *scl;
	struct mutex lock;
	struct gpio_shared_bus bus[GPIO_SHARED_MAX_BUS];
};

static void gpio_shared_setsda(void *data, int state)
{
	struct gpio_shared_bus *bus = data;

	gpiod_set_value_cansleep(bus->sda, state);
}

static void gpio_shared_setscl(void *data, int state)
{
	struct gpio_shared_bus *bus = data;
	struct gpio_shared_ctx *ctx = bus->ctx;

	gpiod_set_value_cansleep(ctx->scl, state);
}

static int gpio_shared_getsda(void *data)
{
	struct gpio_shared_bus *bus = data;

	return gpiod_get_value_cansleep(bus->sda);
}

static int gpio_shared_getscl(void *data)
{
	struct gpio_shared_bus *bus = data;
	struct gpio_shared_ctx *ctx = bus->ctx;

	return gpiod_get_value_cansleep(ctx->scl);
}

static int gpio_shared_pre_xfer(struct i2c_adapter *adap)
{
	struct gpio_shared_bus *bus = container_of(adap, typeof(*bus), adap);
	struct gpio_shared_ctx *ctx = bus->ctx;

	mutex_lock(&ctx->lock);
	dev_dbg(ctx->dev, "lock before transfer to bus %d\n", bus->num);

	return 0;
}

static void gpio_shared_post_xfer(struct i2c_adapter *adap)
{
	struct gpio_shared_bus *bus = container_of(adap, typeof(*bus), adap);
	struct gpio_shared_ctx *ctx = bus->ctx;

	dev_dbg(ctx->dev, "unlock after transfer to bus %d\n", bus->num);
	mutex_unlock(&ctx->lock);
}

static int gpio_shared_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct fwnode_handle *child;
	struct gpio_shared_ctx *ctx;
	int msecs, ret, bus_num = -1;

	ctx = devm_kzalloc(dev, sizeof(*ctx), GFP_KERNEL);
	if (!ctx)
		return -ENOMEM;

	ctx->dev = dev;
	mutex_init(&ctx->lock);

	ctx->scl =  devm_gpiod_get(dev, "scl", GPIOD_OUT_HIGH_OPEN_DRAIN);
	if (IS_ERR(ctx->scl))
		return dev_err_probe(dev, PTR_ERR(ctx->scl), "shared SCL node not found\n");

	if (device_get_child_node_count(dev) >= GPIO_SHARED_MAX_BUS)
		return dev_err_probe(dev, -EINVAL, "Too many channels\n");

	device_for_each_child_node(dev, child) {
		struct gpio_shared_bus *bus = &ctx->bus[++bus_num];
		struct i2c_adapter *adap = &bus->adap;
		struct i2c_algo_bit_data *bit_data = &bus->bit_data;

		bus->sda = devm_fwnode_gpiod_get(dev, child, "sda", GPIOD_OUT_HIGH_OPEN_DRAIN,
						 fwnode_get_name(child));
		if (IS_ERR(bus->sda)) {
			fwnode_handle_put(child);
			dev_err(dev, "SDA node for bus %d not found\n", bus_num);
			continue;
		}

		bus->num = bus_num;
		bus->ctx = ctx;

		bit_data->data = bus;
		bit_data->setsda = gpio_shared_setsda;
		bit_data->setscl = gpio_shared_setscl;
		bit_data->pre_xfer = gpio_shared_pre_xfer;
		bit_data->post_xfer = gpio_shared_post_xfer;

		if (fwnode_property_read_u32(child, "i2c-gpio,delay-us", &bit_data->udelay))
			bit_data->udelay = 5;
		if (!fwnode_property_read_bool(child, "i2c-gpio,sda-output-only"))
			bit_data->getsda = gpio_shared_getsda;
		if (!device_property_read_bool(dev, "i2c-gpio,scl-output-only"))
			bit_data->getscl = gpio_shared_getscl;

		if (!device_property_read_u32(dev, "i2c-gpio,timeout-ms", &msecs))
			bit_data->timeout = msecs_to_jiffies(msecs);
		else
			bit_data->timeout = HZ / 10; /* 100ms */

		if (gpiod_cansleep(bus->sda) || gpiod_cansleep(ctx->scl))
			dev_warn(dev, "Slow GPIO pins might wreak havoc into I2C/SMBus bus timing");
		else
			bit_data->can_do_atomic = true;

		adap->owner = THIS_MODULE;
		strscpy(adap->name, KBUILD_MODNAME, sizeof(adap->name));
		adap->dev.parent = dev;
		device_set_node(&adap->dev, child);
		adap->algo_data = &bus->bit_data;
		adap->class = I2C_CLASS_HWMON;

		ret = i2c_bit_add_bus(adap);
		if (ret)
			return ret;

		dev_info(dev, "shared I2C bus %u using lines %u (SDA) and %u (SCL) delay=%d\n",
			 bus_num, desc_to_gpio(bus->sda), desc_to_gpio(ctx->scl),
			 bit_data->udelay);
	}

	return 0;
}

static const struct of_device_id gpio_shared_of_match[] = {
	{ .compatible = "i2c-gpio-shared" },
	{}
};
MODULE_DEVICE_TABLE(of, gpio_shared_of_match);

static struct platform_driver gpio_shared_driver = {
	.probe = gpio_shared_probe,
	.driver = {
		.name = "i2c-gpio-shared",
		.of_match_table = gpio_shared_of_match,
	},
};

module_platform_driver(gpio_shared_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Markus Stockhausen <markus.stockhausen at gmx.de>");
MODULE_DESCRIPTION("bitbanging multi I2C driver for shared SCL");
