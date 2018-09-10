/*
 * Moschip MCS814x Watchdog driver
 *
 * Copyright (C) 2012, Florian Fainelli <florian@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/watchdog.h>
#include <linux/io.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/of.h>

#define WDT_COUNT	0x00
#define WDT_CTRL	0x04
#define  WDT_CTRL_EN	0x1

/* watchdog frequency */
#define WDT_MAX_VALUE	(0xffffffff)

struct mcs814x_wdt {
	void __iomem		*regs;
	spinlock_t		lock;
	struct watchdog_device	wdt_dev;
	struct clk		*clk;
};

static int mcs814x_wdt_start(struct watchdog_device *dev)
{
	struct mcs814x_wdt *wdt = watchdog_get_drvdata(dev);
	u32 reg;

	spin_lock(&wdt->lock);
	reg = readl_relaxed(wdt->regs + WDT_CTRL);
	reg |= WDT_CTRL_EN;
	writel_relaxed(reg, wdt->regs + WDT_CTRL);
	spin_unlock(&wdt->lock);

	return 0;
}

static int mcs814x_wdt_stop(struct watchdog_device *dev)
{
	struct mcs814x_wdt *wdt = watchdog_get_drvdata(dev);
	u32 reg;

	spin_lock(&wdt->lock);
	reg = readl_relaxed(wdt->regs + WDT_CTRL);
	reg &= ~WDT_CTRL_EN;
	writel_relaxed(reg, wdt->regs + WDT_CTRL);
	spin_unlock(&wdt->lock);

	return 0;
}

static int mcs814x_wdt_set_timeout(struct watchdog_device *dev,
				unsigned int new_timeout)
{
	struct mcs814x_wdt *wdt = watchdog_get_drvdata(dev);

	spin_lock(&wdt->lock);
	/* watchdog counts upward and rollover (0xfffffff -> 0)
	 * triggers the reboot
	 */
	writel_relaxed(WDT_MAX_VALUE - (new_timeout * clk_get_rate(wdt->clk)),
		wdt->regs + WDT_COUNT);
	spin_unlock(&wdt->lock);

	return 0;
}

static int mcs814x_wdt_ping(struct watchdog_device *dev)
{
	/* restart the watchdog */
	mcs814x_wdt_stop(dev);
	mcs814x_wdt_set_timeout(dev, dev->timeout);
	mcs814x_wdt_start(dev);

	return 0;
}

static const struct watchdog_info mcs814x_wdt_ident = {
	.options	= WDIOF_CARDRESET | WDIOF_SETTIMEOUT |
			  WDIOF_MAGICCLOSE | WDIOF_KEEPALIVEPING,
	.identity	= "MCS814x Watchdog",
};

static struct watchdog_ops mcs814x_wdt_ops = {
	.owner		= THIS_MODULE,
	.start		= mcs814x_wdt_start,
	.stop		= mcs814x_wdt_stop,
	.set_timeout	= mcs814x_wdt_set_timeout,
	.ping		= mcs814x_wdt_ping,
};

static int mcs814x_wdt_probe(struct platform_device *pdev)
{
	struct resource *res;
	struct mcs814x_wdt *wdt;
	int ret;
	struct clk *clk;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res)
		return -ENODEV;

	clk = clk_get(NULL, "wdt");
	if (IS_ERR_OR_NULL(clk)) {
		dev_err(&pdev->dev, "failed to get watchdog clock\n");
		return PTR_ERR(clk);
	}

	wdt = kzalloc(sizeof(*wdt), GFP_KERNEL);
	if (!wdt) {
		ret = -ENOMEM;
		goto out_clk;
	}

	spin_lock_init(&wdt->lock);
	wdt->clk = clk;
	wdt->wdt_dev.info = &mcs814x_wdt_ident;
	wdt->wdt_dev.ops = &mcs814x_wdt_ops;
	wdt->wdt_dev.min_timeout = 1;
	/* approximately 10995 secs */
	wdt->wdt_dev.max_timeout = (WDT_MAX_VALUE / clk_get_rate(clk));

	platform_set_drvdata(pdev, wdt);

	/* only ioremap registers, because the register is shared */
	wdt->regs = devm_ioremap(&pdev->dev, res->start, resource_size(res));
	if (!wdt->regs) {
		ret = -ENOMEM;
		goto out;
	}

	watchdog_set_drvdata(&wdt->wdt_dev, wdt);

	ret = watchdog_register_device(&wdt->wdt_dev);
	if (ret) {
		dev_err(&pdev->dev, "cannot register watchdog: %d\n", ret);
		goto out;
	}

	dev_info(&pdev->dev, "registered\n");
	return 0;

out:
	platform_set_drvdata(pdev, NULL);
	kfree(wdt);
out_clk:
	clk_put(clk);
	return ret;
}

static int mcs814x_wdt_remove(struct platform_device *pdev)
{
	struct mcs814x_wdt *wdt = platform_get_drvdata(pdev);

	clk_put(wdt->clk);
	watchdog_unregister_device(&wdt->wdt_dev);
	watchdog_set_drvdata(&wdt->wdt_dev, NULL);
	kfree(wdt);
	platform_set_drvdata(pdev, NULL);

	return 0;
}

static const struct of_device_id mcs814x_wdt_ids[] = {
	{ .compatible = "moschip,mcs814x-wdt", },
	{ /* sentinel */ },
};

static struct platform_driver mcs814x_wdt_driver = {
	.driver	= {
		.name	= "mcs814x-wdt",
		.owner	= THIS_MODULE,
		.of_match_table = mcs814x_wdt_ids,
	},
	.probe		= mcs814x_wdt_probe,
	.remove		= mcs814x_wdt_remove,
};

module_platform_driver(mcs814x_wdt_driver);

MODULE_AUTHOR("Florian Fainelli <florian@openwrt.org>");
MODULE_DESCRIPTION("Moschip MCS814x Watchdog driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS_MISCDEV(WATCHDOG_MINOR);
MODULE_ALIAS("platform:mcs814x-wdt");
