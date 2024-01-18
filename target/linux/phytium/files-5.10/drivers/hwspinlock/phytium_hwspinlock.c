// SPDX-License-Identifier: GPL-2.0
/*
 * Phytium hardware spinlock driver
 *
 * Copyright (c) 2021-2023 Phytium Technology Co., Ltd.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/bitops.h>
#include <linux/pm_runtime.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/hwspinlock.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/acpi.h>

#include "hwspinlock_internal.h"

/* Spinlock register offsets */
#define LOCK_BASE	0x10

#define SEMA_NOTTAKEN	(0)	/* free */
#define SEMA_TAKEN	(1)	/* locked */

static int phytium_hwspinlock_trylock(struct hwspinlock *lock)
{
	void __iomem *lock_addr = lock->priv;

	/* attempt to acquire the lock by reading its value */
	return (readl(lock_addr) == SEMA_NOTTAKEN);
}

static void phytium_hwspinlock_unlock(struct hwspinlock *lock)
{
	void __iomem *lock_addr = lock->priv;

	/* release the lock by writing 0 to it */
	writel(SEMA_NOTTAKEN, lock_addr);
}

static void phytium_hwspinlock_relax(struct hwspinlock *lock)
{
	ndelay(50);
}

static const struct hwspinlock_ops phytium_hwspinlock_ops = {
	.trylock = phytium_hwspinlock_trylock,
	.unlock = phytium_hwspinlock_unlock,
	.relax = phytium_hwspinlock_relax,
};

static ssize_t phytium_hwlock_test_store(struct device *dev,
                        struct device_attribute *attr, const char *buf, size_t count)
{
    struct hwspinlock *hwlock = NULL;
    int ret;

    /* dynamically assign a hwspinlock */
    hwlock = hwspin_lock_request();
    if (!hwlock) {
        pr_err("%s can't request hwlock\n", __func__);
        return -ENODEV;
    }
    ret = hwspin_lock_timeout(hwlock, 1000);
    if (ret < 0) {
        pr_err("do lock fail\n");
        goto err;
    }
    pr_info("do lock success\n");

    ret = hwspin_trylock(hwlock);
    if (ret == -EINVAL) {
        pr_err("lock invalid\n");
        goto err;
    }
    else
        pr_info("lock status is %s before, than unlock it\n", ret ? "locked" : "no use");

    pr_info("do lock test pass\n");
err:
    hwspin_unlock(hwlock);
    hwspin_lock_free(hwlock);
    return count;
}

static DEVICE_ATTR_WO(phytium_hwlock_test);


static int phytium_hwspinlock_probe(struct platform_device *pdev)
{
	struct fwnode_handle *np = dev_fwnode(&(pdev->dev));
	struct hwspinlock_device *bank;
	struct hwspinlock *hwlock;
	struct resource *res;
	void __iomem *io_base;
	int num_locks, i, ret;

	if (!np)
		return -ENODEV;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res)
		return -ENODEV;

	io_base = ioremap(res->start, resource_size(res));
	if (!io_base)
		return -ENOMEM;

	/*
	 * make sure the module is enabled and clocked before reading
	 * the module SYSSTATUS register
	 */
	pm_runtime_enable(&pdev->dev);
	ret = pm_runtime_get_sync(&pdev->dev);
	if (ret < 0) {
		pm_runtime_put_noidle(&pdev->dev);
		goto iounmap_base;
	}

	/* Determine number of locks */
	if (fwnode_property_read_u32(np, "nr-locks", &num_locks)) {
		dev_err(&pdev->dev, "missing/invalid number of locks\n");
		ret = -EINVAL;
		goto iounmap_base;
	}

	/*
	 * runtime PM will make sure the clock of this module is
	 * enabled again iff at least one lock is requested
	 */
	ret = pm_runtime_put(&pdev->dev);
	if (ret < 0)
		goto iounmap_base;

	bank = kzalloc(struct_size(bank, lock, num_locks), GFP_KERNEL);
	if (!bank) {
		ret = -ENOMEM;
		goto iounmap_base;
	}

	platform_set_drvdata(pdev, bank);

	for (i = 0, hwlock = &bank->lock[0]; i < num_locks; i++, hwlock++) {
		/* Set register address of each lock */
		hwlock->priv = io_base + LOCK_BASE + sizeof(u32) * i;
	}

	ret = hwspin_lock_register(bank, &pdev->dev, &phytium_hwspinlock_ops,
				   0, num_locks);
	if (ret)
		goto reg_fail;

	ret = device_create_file(&pdev->dev, &dev_attr_phytium_hwlock_test);
	return ret;
reg_fail:
	kfree(bank);
iounmap_base:
	iounmap(io_base);
	return ret;
}

static int phytium_hwspinlock_remove(struct platform_device *pdev)
{
	struct hwspinlock_device *bank = platform_get_drvdata(pdev);
	void __iomem *io_base = bank->lock[0].priv - LOCK_BASE;
	int ret;

	ret = hwspin_lock_unregister(bank);
	if (ret) {
		dev_err(&pdev->dev, "%s failed: %d\n", __func__, ret);
		return ret;
	}

	iounmap(io_base);
	kfree(bank);

	return 0;
}

#ifdef CONFIG_ACPI
static const struct acpi_device_id phytium_hwspinlock_acpi_ids[] = {
       { "PHYT0027", 0 },
       { /* sentinel */ },
};
MODULE_DEVICE_TABLE(acpi, phytium_hwspinlock_acpi_ids);
#endif

static const struct of_device_id phytium_hwspinlock_of_match[] = {
	{ .compatible = "phytium,hwspinlock", },
	{ /* end */ },
};
MODULE_DEVICE_TABLE(of, phytium_hwspinlock_of_match);

static struct platform_driver phytium_hwspinlock_driver = {
	.probe		= phytium_hwspinlock_probe,
	.remove		= phytium_hwspinlock_remove,
	.driver		= {
		.name	= "phytium_hwspinlock",
		.of_match_table = of_match_ptr(phytium_hwspinlock_of_match),
		.acpi_match_table = ACPI_PTR(phytium_hwspinlock_acpi_ids),
	},
};

static int __init phytium_hwspinlock_init(void)
{
	return platform_driver_register(&phytium_hwspinlock_driver);
}
postcore_initcall(phytium_hwspinlock_init);

static void __exit phytium_hwspinlock_exit(void)
{
	platform_driver_unregister(&phytium_hwspinlock_driver);
}
module_exit(phytium_hwspinlock_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Hardware spinlock driver for Phytium");
MODULE_AUTHOR("Chen Baozi <chenbaozi@phytium.com.cn>");
