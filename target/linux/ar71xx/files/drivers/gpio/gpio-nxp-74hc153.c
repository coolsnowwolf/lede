/*
 *  NXP 74HC153 - Dual 4-input multiplexer GPIO driver
 *
 *  Copyright (C) 2010 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#include <linux/version.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/nxp_74hc153.h>

#define NXP_74HC153_NUM_GPIOS	8
#define NXP_74HC153_S0_MASK	0x1
#define NXP_74HC153_S1_MASK	0x2
#define NXP_74HC153_BANK_MASK	0x4

struct nxp_74hc153_chip {
	struct device		*parent;
	struct gpio_chip	gpio_chip;
	struct mutex		lock;
};

static struct nxp_74hc153_chip *gpio_to_nxp(struct gpio_chip *gc)
{
	return container_of(gc, struct nxp_74hc153_chip, gpio_chip);
}

static int nxp_74hc153_direction_input(struct gpio_chip *gc, unsigned offset)
{
	return 0;
}

static int nxp_74hc153_direction_output(struct gpio_chip *gc,
					unsigned offset, int val)
{
	return -EINVAL;
}

static int nxp_74hc153_get_value(struct gpio_chip *gc, unsigned offset)
{
	struct nxp_74hc153_chip *nxp;
	struct nxp_74hc153_platform_data *pdata;
	unsigned s0;
	unsigned s1;
	unsigned pin;
	int ret;

	nxp = gpio_to_nxp(gc);
	pdata = nxp->parent->platform_data;

	s0 = !!(offset & NXP_74HC153_S0_MASK);
	s1 = !!(offset & NXP_74HC153_S1_MASK);
	pin = (offset & NXP_74HC153_BANK_MASK) ? pdata->gpio_pin_2y
					       : pdata->gpio_pin_1y;

	mutex_lock(&nxp->lock);
	gpio_set_value(pdata->gpio_pin_s0, s0);
	gpio_set_value(pdata->gpio_pin_s1, s1);
	ret = gpio_get_value(pin);
	mutex_unlock(&nxp->lock);

	return ret;
}

static void nxp_74hc153_set_value(struct gpio_chip *gc,
				  unsigned offset, int val)
{
	/* not supported */
}

static int nxp_74hc153_probe(struct platform_device *pdev)
{
	struct nxp_74hc153_platform_data *pdata;
	struct nxp_74hc153_chip *nxp;
	struct gpio_chip *gc;
	int err;

	pdata = pdev->dev.platform_data;
	if (pdata == NULL) {
		dev_dbg(&pdev->dev, "no platform data specified\n");
		return -EINVAL;
	}

	nxp = kzalloc(sizeof(struct nxp_74hc153_chip), GFP_KERNEL);
	if (nxp == NULL) {
		dev_err(&pdev->dev, "no memory for private data\n");
		return -ENOMEM;
	}

	err = gpio_request(pdata->gpio_pin_s0, dev_name(&pdev->dev));
	if (err) {
		dev_err(&pdev->dev, "unable to claim gpio %u, err=%d\n",
			pdata->gpio_pin_s0, err);
		goto err_free_nxp;
	}

	err = gpio_request(pdata->gpio_pin_s1, dev_name(&pdev->dev));
	if (err) {
		dev_err(&pdev->dev, "unable to claim gpio %u, err=%d\n",
			pdata->gpio_pin_s1, err);
		goto err_free_s0;
	}

	err = gpio_request(pdata->gpio_pin_1y, dev_name(&pdev->dev));
	if (err) {
		dev_err(&pdev->dev, "unable to claim gpio %u, err=%d\n",
			pdata->gpio_pin_1y, err);
		goto err_free_s1;
	}

	err = gpio_request(pdata->gpio_pin_2y, dev_name(&pdev->dev));
	if (err) {
		dev_err(&pdev->dev, "unable to claim gpio %u, err=%d\n",
			pdata->gpio_pin_2y, err);
		goto err_free_1y;
	}

	err = gpio_direction_output(pdata->gpio_pin_s0, 0);
	if (err) {
		dev_err(&pdev->dev,
			"unable to set direction of gpio %u, err=%d\n",
			pdata->gpio_pin_s0, err);
		goto err_free_2y;
	}

	err = gpio_direction_output(pdata->gpio_pin_s1, 0);
	if (err) {
		dev_err(&pdev->dev,
			"unable to set direction of gpio %u, err=%d\n",
			pdata->gpio_pin_s1, err);
		goto err_free_2y;
	}

	err = gpio_direction_input(pdata->gpio_pin_1y);
	if (err) {
		dev_err(&pdev->dev,
			"unable to set direction of gpio %u, err=%d\n",
			pdata->gpio_pin_1y, err);
		goto err_free_2y;
	}

	err = gpio_direction_input(pdata->gpio_pin_2y);
	if (err) {
		dev_err(&pdev->dev,
			"unable to set direction of gpio %u, err=%d\n",
			pdata->gpio_pin_2y, err);
		goto err_free_2y;
	}

	nxp->parent = &pdev->dev;
	mutex_init(&nxp->lock);

	gc = &nxp->gpio_chip;

	gc->direction_input  = nxp_74hc153_direction_input;
	gc->direction_output = nxp_74hc153_direction_output;
	gc->get = nxp_74hc153_get_value;
	gc->set = nxp_74hc153_set_value;
	gc->can_sleep = 1;

	gc->base = pdata->gpio_base;
	gc->ngpio = NXP_74HC153_NUM_GPIOS;
	gc->label = dev_name(nxp->parent);
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,5,0)
	gc->dev = nxp->parent;
#else
	gc->parent = nxp->parent;
#endif
	gc->owner = THIS_MODULE;

	err = gpiochip_add(&nxp->gpio_chip);
	if (err) {
		dev_err(&pdev->dev, "unable to add gpio chip, err=%d\n", err);
		goto err_free_2y;
	}

	platform_set_drvdata(pdev, nxp);
	return 0;

err_free_2y:
	gpio_free(pdata->gpio_pin_2y);
err_free_1y:
	gpio_free(pdata->gpio_pin_1y);
err_free_s1:
	gpio_free(pdata->gpio_pin_s1);
err_free_s0:
	gpio_free(pdata->gpio_pin_s0);
err_free_nxp:
	kfree(nxp);
	return err;
}

static int nxp_74hc153_remove(struct platform_device *pdev)
{
	struct nxp_74hc153_chip *nxp = platform_get_drvdata(pdev);
	struct nxp_74hc153_platform_data *pdata = pdev->dev.platform_data;

	if (nxp) {
		gpiochip_remove(&nxp->gpio_chip);
		gpio_free(pdata->gpio_pin_2y);
		gpio_free(pdata->gpio_pin_1y);
		gpio_free(pdata->gpio_pin_s1);
		gpio_free(pdata->gpio_pin_s0);

		kfree(nxp);
		platform_set_drvdata(pdev, NULL);
	}

	return 0;
}

static struct platform_driver nxp_74hc153_driver = {
	.probe		= nxp_74hc153_probe,
	.remove		= nxp_74hc153_remove,
	.driver = {
		.name	= NXP_74HC153_DRIVER_NAME,
		.owner	= THIS_MODULE,
	},
};

static int __init nxp_74hc153_init(void)
{
	return platform_driver_register(&nxp_74hc153_driver);
}
subsys_initcall(nxp_74hc153_init);

static void __exit nxp_74hc153_exit(void)
{
	platform_driver_unregister(&nxp_74hc153_driver);
}
module_exit(nxp_74hc153_exit);

MODULE_AUTHOR("Gabor Juhos <juhosg@openwrt.org>");
MODULE_DESCRIPTION("GPIO expander driver for NXP 74HC153");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:" NXP_74HC153_DRIVER_NAME);
