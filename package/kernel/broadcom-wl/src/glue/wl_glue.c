/*
 * wl_glue.c: Broadcom WL support module providing a unified SSB/BCMA handling.
 * Copyright (C) 2011 Jo-Philipp Wich <jo@mein.io>
 */

#include "wl_glue.h"

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>

#ifdef CONFIG_BCM47XX
#include <bcm47xx.h>
#endif

#ifdef CONFIG_SSB
#include <linux/ssb/ssb.h>
#endif

#ifdef CONFIG_BCMA
#include <linux/bcma/bcma.h>
#endif

MODULE_AUTHOR("Jo-Philipp Wich (jo@mein.io)");
MODULE_DESCRIPTION("Broadcom WL SSB/BCMA compatibility layer");
MODULE_LICENSE("GPL");

static wl_glue_attach_cb_t attach_cb = NULL;
static wl_glue_remove_cb_t remove_cb = NULL;
static enum wl_glue_bus_type active_bus_type = WL_GLUE_BUS_TYPE_UNSPEC;
static int wl_glue_attached = 0;


#ifdef CONFIG_SSB
static int wl_glue_ssb_probe(struct ssb_device *dev, const struct ssb_device_id *id)
{
	void *mmio;
	void *wldev;

	if (!attach_cb)
	{
		pr_err("No attach callback registered\n");
		return -ENOSYS;
	}

	if (dev->bus->bustype != SSB_BUSTYPE_SSB)
	{
		pr_err("Attaching to SSB behind PCI is not supported. Please remove the b43 ssb bridge\n");
		return -EINVAL;
	}

	mmio = (void *) 0x18000000 + dev->core_index * 0x1000;
	wldev = attach_cb(id->vendor, id->coreid, (ulong)mmio, dev, dev->irq);

	if (!wldev)
	{
		pr_err("The attach callback failed, SSB probe aborted\n");
		return -ENODEV;
	}

	ssb_set_drvdata(dev, wldev);
	return 0;
}

static void wl_glue_ssb_remove(struct ssb_device *dev)
{
	void *wldev = ssb_get_drvdata(dev);

	if (remove_cb)
		remove_cb(wldev);

	ssb_set_drvdata(dev, NULL);
}

static const struct ssb_device_id wl_glue_ssb_tbl[] = {
	SSB_DEVICE(SSB_VENDOR_BROADCOM, SSB_DEV_80211, SSB_ANY_REV),
	{},
};

static struct ssb_driver wl_glue_ssb_driver = {
	.name     = KBUILD_MODNAME,
	.id_table = wl_glue_ssb_tbl,
	.probe    = wl_glue_ssb_probe,
	.remove   = wl_glue_ssb_remove,
};
#endif /* CONFIG_SSB */

#ifdef CONFIG_BCMA
static int wl_glue_bcma_probe(struct bcma_device *dev)
{
	void *wldev;

	if (!attach_cb)
	{
		pr_err("No attach callback registered\n");
		return -ENOSYS;
	}

	if (dev->bus->hosttype != BCMA_HOSTTYPE_SOC)
	{
		pr_err("Unsupported BCMA bus type %d\n", dev->bus->hosttype);
		return -EINVAL;
	}

	/*
	 * NB:
	 * 0x18000000 = BCMA_ADDR_BASE
	 * 0x1000     = BCMA_CORE_SIZE
	 */

	wldev = attach_cb(dev->id.manuf, dev->id.id, (ulong)dev->addr, dev, dev->irq);

	if (!wldev)
	{
		pr_err("The attach callback failed, BCMA probe aborted\n");
		return -ENODEV;
	}

	bcma_set_drvdata(dev, wldev);
	return 0;
}

static void wl_glue_bcma_remove(struct bcma_device *dev)
{
	void *wldev = bcma_get_drvdata(dev);

	if (remove_cb)
		remove_cb(wldev);

	bcma_set_drvdata(dev, NULL);
}

static const struct bcma_device_id wl_glue_bcma_tbl[] = {
	BCMA_CORE(BCMA_MANUF_BCM, BCMA_CORE_80211, BCMA_ANY_REV, BCMA_ANY_CLASS),
	{},
};

static struct bcma_driver wl_glue_bcma_driver = {
	.name     = KBUILD_MODNAME,
	.id_table = wl_glue_bcma_tbl,
	.probe    = wl_glue_bcma_probe,
	.remove   = wl_glue_bcma_remove,
};
#endif /* CONFIG_BCMA */


void wl_glue_set_attach_callback(wl_glue_attach_cb_t cb)
{
	attach_cb = cb;
}
EXPORT_SYMBOL(wl_glue_set_attach_callback);

void wl_glue_set_remove_callback(wl_glue_remove_cb_t cb)
{
	remove_cb = cb;
}
EXPORT_SYMBOL(wl_glue_set_remove_callback);

int wl_glue_register(void)
{
	int err;

	switch(active_bus_type)
	{
#ifdef CONFIG_SSB
	case WL_GLUE_BUS_TYPE_SSB:
		err = ssb_driver_register(&wl_glue_ssb_driver);
		break;
#endif /* CONFIG_SSB */

#ifdef CONFIG_BCMA
	case WL_GLUE_BUS_TYPE_BCMA:
		err = bcma_driver_register(&wl_glue_bcma_driver);
		break;
#endif /* CONFIG_BCMA */

	default:
		pr_err("Not attaching through glue driver due to unsupported bus\n");
		err = -ENOSYS;
		break;
	}

	if (!err)
	{
		pr_info("SSB/BCMA glue driver successfully attached\n");
		wl_glue_attached = 1;
	}

	return err;
}
EXPORT_SYMBOL(wl_glue_register);

int wl_glue_unregister(void)
{
	int err;

	if (!wl_glue_attached)
		return -ENOSYS;

	switch (active_bus_type)
	{
#ifdef CONFIG_SSB
	case WL_GLUE_BUS_TYPE_SSB:
		ssb_driver_unregister(&wl_glue_ssb_driver);
		err = 0;
		break;
#endif /* CONFIG_SSB */

#ifdef CONFIG_BCMA
	case WL_GLUE_BUS_TYPE_BCMA:
		bcma_driver_unregister(&wl_glue_bcma_driver);
		err = 0;
		break;
#endif /* CONFIG_BCMA */

	default:
		pr_err("Not removing glue driver due to unsupported bus\n");
		err = -ENOSYS;
		break;
	}

	if (!err)
	{
		pr_info("SSB/BCMA glue driver successfully detached\n");
		wl_glue_attached = 0;
	}

	return err;
}
EXPORT_SYMBOL(wl_glue_unregister);

struct device * wl_glue_get_dmadev(void *dev)
{
	struct device *dma_dev;

	switch (active_bus_type)
	{
#ifdef CONFIG_SSB
	case WL_GLUE_BUS_TYPE_SSB:
		dma_dev = ((struct ssb_device *)dev)->dma_dev;
		break;
#endif /* CONFIG_SSB */

#ifdef CONFIG_BCMA
	case WL_GLUE_BUS_TYPE_BCMA:
		dma_dev = ((struct bcma_device *)dev)->dma_dev;
		break;
#endif /* CONFIG_BCMA */

	default:
		BUG();
		dma_dev = NULL;
		break;
	}

	return dma_dev;
}
EXPORT_SYMBOL(wl_glue_get_dmadev);


static int __init wl_glue_init(void)
{
#ifdef CONFIG_BCM47XX
	/*
	 * BCM47xx currently supports either SSB or BCMA bus,
	 * determine the used one from the info set by the
	 * platform setup code.
	 */
	switch (bcm47xx_bus_type)
	{
#ifdef CONFIG_BCM47XX_SSB
	case BCM47XX_BUS_TYPE_SSB:
		active_bus_type = WL_GLUE_BUS_TYPE_SSB;
		break;
#endif /* CONFIG_BCM47XX_SSB */

#ifdef CONFIG_BCM47XX_BCMA
	case BCM47XX_BUS_TYPE_BCMA:
		active_bus_type = WL_GLUE_BUS_TYPE_BCMA;
		break;
#endif /* CONFIG_BCM47XX_BCMA */
	}
#endif /* CONFIG_BCM47XX */

#ifdef CONFIG_BCM63XX
#ifdef CONFIG_SSB
	/*
	 * BCM63xx currently only uses SSB, so assume that.
	 */
	active_bus_type = WL_GLUE_BUS_TYPE_SSB;
#endif /* CONFIG_SSB */
#endif /* CONFIG_BCM63XX */

	/* do not fail here, let wl_glue_register() return -ENOSYS later */
	if (active_bus_type == WL_GLUE_BUS_TYPE_UNSPEC)
		pr_err("Unable to determine used system bus type\n");

	return 0;
}

static void __exit wl_glue_exit(void)
{
	if (wl_glue_attached)
	{
		if (wl_glue_unregister())
			pr_err("Failed to unregister glue driver\n");

		wl_glue_attached = 0;
	}

	return;
}

module_init(wl_glue_init);
module_exit(wl_glue_exit);
