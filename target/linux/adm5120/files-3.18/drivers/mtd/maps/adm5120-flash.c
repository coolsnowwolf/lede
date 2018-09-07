/*
 *  Platform driver for NOR flash devices on ADM5120 based boards
 *
 *  Copyright (C) 2007-2008 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This file was derived from: drivers/mtd/map/physmap.c
 *	Copyright (C) 2003 MontaVista Software Inc.
 *	Author: Jun Sun, jsun@mvista.com or jsun@junsun.net
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */

#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/device.h>
#include <linux/platform_device.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/map.h>
#include <linux/mtd/partitions.h>

#include <asm/mach-adm5120/adm5120_defs.h>
#include <asm/mach-adm5120/adm5120_switch.h>
#include <asm/mach-adm5120/adm5120_mpmc.h>
#include <asm/mach-adm5120/adm5120_platform.h>

#define DRV_NAME	"adm5120-flash"
#define DRV_DESC	"ADM5120 flash MAP driver"
#define MAX_PARSED_PARTS 8

#ifdef ADM5120_FLASH_DEBUG
#define MAP_DBG(m, f, a...)	printk(KERN_INFO "%s: " f, (m->name) , ## a)
#else
#define MAP_DBG(m, f, a...)	do {} while (0)
#endif
#define MAP_ERR(m, f, a...)	printk(KERN_ERR "%s: " f, (m->name) , ## a)
#define MAP_INFO(m, f, a...)	printk(KERN_INFO "%s: " f, (m->name) , ## a)

struct adm5120_map_info {
	struct map_info	map;
	void		(*switch_bank)(unsigned);
	unsigned long	window_size;
};

struct adm5120_flash_info {
	struct mtd_info		*mtd;
	struct resource		*res;
	struct platform_device	*dev;
	struct adm5120_map_info	amap;
};

struct flash_desc {
	u32	phys;
	u32	srs_shift;
};

/*
 * Globals
 */
static DEFINE_SPINLOCK(adm5120_flash_spin);
#define FLASH_LOCK()	spin_lock(&adm5120_flash_spin)
#define FLASH_UNLOCK()	spin_unlock(&adm5120_flash_spin)

static u32 flash_bankwidths[4] = { 1, 2, 4, 0 };

static u32 flash_sizes[8] = {
	0, 512*1024, 1024*1024, 2*1024*1024,
	4*1024*1024, 0, 0, 0
};

static struct flash_desc flash_descs[2] = {
	{
		.phys		= ADM5120_SRAM0_BASE,
		.srs_shift	= MEMCTRL_SRS0_SHIFT,
	}, {
		.phys		= ADM5120_SRAM1_BASE,
		.srs_shift	= MEMCTRL_SRS1_SHIFT,
	}
};

static const char const *probe_types[] = {
	"cfi_probe",
	"jedec_probe",
	"map_rom",
	NULL
};

static const char const *parse_types[] = {
	"cmdlinepart",
#ifdef CONFIG_MTD_REDBOOT_PARTS
	"RedBoot",
#endif
#ifdef CONFIG_MTD_MYLOADER_PARTS
	"MyLoader",
#endif
	NULL,
};

#define BANK_SIZE	(2<<20)
#define BANK_SIZE_MAX	(4<<20)
#define BANK_OFFS_MASK	(BANK_SIZE-1)
#define BANK_START_MASK	(~BANK_OFFS_MASK)

static inline struct adm5120_map_info *map_to_amap(struct map_info *map)
{
	return (struct adm5120_map_info *)map;
}

static void adm5120_flash_switchbank(struct map_info *map,
		unsigned long ofs)
{
	struct adm5120_map_info *amap = map_to_amap(map);
	unsigned bank;

	if (amap->switch_bank == NULL)
		return;

	bank = (ofs & BANK_START_MASK) >> 21;
	if (bank > 1)
		BUG();

	MAP_DBG(map, "switching to bank %u, ofs=%lX\n", bank, ofs);
	amap->switch_bank(bank);
}

static map_word adm5120_flash_read(struct map_info *map, unsigned long ofs)
{
	struct adm5120_map_info *amap = map_to_amap(map);
	map_word ret;

	MAP_DBG(map, "reading from ofs %lX\n", ofs);

	if (ofs >= amap->window_size)
		return map_word_ff(map);

	FLASH_LOCK();
	adm5120_flash_switchbank(map, ofs);
	ret = inline_map_read(map, (ofs & (amap->window_size-1)));
	FLASH_UNLOCK();

	return ret;
}

static void adm5120_flash_write(struct map_info *map, const map_word datum,
		unsigned long ofs)
{
	struct adm5120_map_info *amap = map_to_amap(map);

	MAP_DBG(map, "writing to ofs %lX\n", ofs);

	if (ofs > amap->window_size)
		return;

	FLASH_LOCK();
	adm5120_flash_switchbank(map, ofs);
	inline_map_write(map, datum, (ofs & (amap->window_size-1)));
	FLASH_UNLOCK();
}

static void adm5120_flash_copy_from(struct map_info *map, void *to,
		unsigned long from, ssize_t len)
{
	struct adm5120_map_info *amap = map_to_amap(map);
	char *p;
	ssize_t t;

	MAP_DBG(map, "copy_from, to=%lX, from=%lX, len=%lX\n",
		(unsigned long)to, from, (unsigned long)len);

	if (from > amap->window_size)
		return;

	p = (char *)to;
	while (len > 0) {
		t = len;
		if ((from < BANK_SIZE) && ((from+len) > BANK_SIZE))
			t = BANK_SIZE-from;

		FLASH_LOCK();
		MAP_DBG(map, "copying %lu byte(s) from %lX to %lX\n",
			(unsigned long)t, (from & (amap->window_size-1)),
			(unsigned long)p);
		adm5120_flash_switchbank(map, from);
		inline_map_copy_from(map, p, (from & (amap->window_size-1)), t);
		FLASH_UNLOCK();
		p += t;
		from += t;
		len -= t;
	}
}

static int adm5120_flash_initres(struct adm5120_flash_info *info)
{
	struct map_info *map = &info->amap.map;
	int err = 0;

	info->res = request_mem_region(map->phys, info->amap.window_size,
			map->name);
	if (info->res == NULL) {
		MAP_ERR(map, "could not reserve memory region\n");
		err = -ENOMEM;
		goto out;
	}

	map->virt = ioremap_nocache(map->phys, info->amap.window_size);
	if (map->virt == NULL) {
		MAP_ERR(map, "failed to ioremap flash region\n");
		err = -ENOMEM;
		goto out;
	}

out:
	return err;
}

static int adm5120_flash_initinfo(struct adm5120_flash_info *info,
		struct platform_device *dev)
{
	struct map_info *map = &info->amap.map;
	struct adm5120_flash_platform_data *pdata = dev->dev.platform_data;
	struct flash_desc *fdesc;
	u32 t = 0;

	map->name = dev_name(&dev->dev);

	if (dev->id > 1) {
		MAP_ERR(map, "invalid flash id\n");
		goto err_out;
	}

	fdesc = &flash_descs[dev->id];

	if (pdata)
		info->amap.window_size = pdata->window_size;

	if (info->amap.window_size == 0) {
		/* get memory window size */
		t = SW_READ_REG(SWITCH_REG_MEMCTRL) >> fdesc->srs_shift;
		t &= MEMCTRL_SRS_MASK;
		info->amap.window_size = flash_sizes[t];
	}

	if (info->amap.window_size == 0) {
		MAP_ERR(map, "unable to determine window size\n");
		goto err_out;
	}

	/* get flash bus width */
	switch (dev->id) {
	case 0:
		t = MPMC_READ_REG(SC1) & SC_MW_MASK;
		break;
	case 1:
		t = MPMC_READ_REG(SC0) & SC_MW_MASK;
		break;
	}
	map->bankwidth = flash_bankwidths[t];
	if (map->bankwidth == 0) {
		MAP_ERR(map, "invalid bus width detected\n");
		goto err_out;
	}

	map->phys = fdesc->phys;
	map->size = BANK_SIZE_MAX;

	simple_map_init(map);
	map->read = adm5120_flash_read;
	map->write = adm5120_flash_write;
	map->copy_from = adm5120_flash_copy_from;

	if (pdata) {
		map->set_vpp = pdata->set_vpp;
		info->amap.switch_bank = pdata->switch_bank;
	}

	info->dev = dev;

	MAP_INFO(map, "probing at 0x%lX, size:%ldKiB, width:%d bits\n",
		(unsigned long)map->phys,
		(unsigned long)info->amap.window_size >> 10,
		map->bankwidth*8);

	return 0;

err_out:
	return -ENODEV;
}

static void adm5120_flash_initbanks(struct adm5120_flash_info *info)
{
	struct map_info *map = &info->amap.map;

	if (info->mtd->size <= BANK_SIZE)
		/* no bank switching needed */
		return;

	if (info->amap.switch_bank) {
		info->amap.window_size = info->mtd->size;
		return;
	}

	MAP_ERR(map, "reduce visibility from %ldKiB to %ldKiB\n",
		(unsigned long)map->size >> 10,
		(unsigned long)info->mtd->size >> 10);

	info->mtd->size = info->amap.window_size;
}

static int adm5120_flash_remove(struct platform_device *dev)
{
	struct adm5120_flash_info *info;

	info = platform_get_drvdata(dev);
	if (info == NULL)
		return 0;

	platform_set_drvdata(dev, NULL);

	if (info->mtd != NULL) {
		mtd_device_unregister(info->mtd);
		map_destroy(info->mtd);
	}

	if (info->amap.map.virt != NULL)
		iounmap(info->amap.map.virt);

	if (info->res != NULL) {
		release_resource(info->res);
		kfree(info->res);
	}

	return 0;
}

static int adm5120_flash_probe(struct platform_device *dev)
{
	struct adm5120_flash_platform_data *pdata;
	struct adm5120_flash_info *info;
	struct map_info *map;
	const char **probe_type;
	int err;

	pdata = dev->dev.platform_data;
	if (!pdata) {
		dev_err(&dev->dev, "no platform data\n");
		return -EINVAL;
	}

	info = kzalloc(sizeof(*info), GFP_KERNEL);
	if (info == NULL) {
		err = -ENOMEM;
		goto err_out;
	}

	platform_set_drvdata(dev, info);

	err = adm5120_flash_initinfo(info, dev);
	if (err)
		goto err_out;

	err = adm5120_flash_initres(info);
	if (err)
		goto err_out;

	map = &info->amap.map;
	for (probe_type = probe_types; info->mtd == NULL && *probe_type != NULL;
		probe_type++)
		info->mtd = do_map_probe(*probe_type, map);

	if (info->mtd == NULL) {
		MAP_ERR(map, "map_probe failed\n");
		err = -ENXIO;
		goto err_out;
	}

	adm5120_flash_initbanks(info);

	if (info->mtd->size < info->amap.window_size) {
		/* readjust resources */
		iounmap(map->virt);
		release_resource(info->res);
		kfree(info->res);

		info->amap.window_size = info->mtd->size;
		map->size = info->mtd->size;
		MAP_INFO(map, "reducing map size to %ldKiB\n",
			(unsigned long)map->size >> 10);
		err = adm5120_flash_initres(info);
		if (err)
			goto err_out;
	}

	MAP_INFO(map, "found at 0x%lX, size:%ldKiB, width:%d bits\n",
		(unsigned long)map->phys, (unsigned long)info->mtd->size >> 10,
		map->bankwidth*8);

	info->mtd->owner = THIS_MODULE;

	err = mtd_device_parse_register(info->mtd, parse_types, 0,
				  	pdata->parts, pdata->nr_parts);
	if (err)
		goto err_out;

	return 0;

err_out:
	adm5120_flash_remove(dev);
	return err;
}

#ifdef CONFIG_PM
static int adm5120_flash_suspend(struct platform_device *dev,
		pm_message_t state)
{
	struct adm5120_flash_info *info = platform_get_drvdata(dev);
	int ret = 0;

	if (info)
		ret = info->mtd->suspend(info->mtd);

	return ret;
}

static int adm5120_flash_resume(struct platform_device *dev)
{
	struct adm5120_flash_info *info = platform_get_drvdata(dev);

	if (info)
		info->mtd->resume(info->mtd);

	return 0;
}

static void adm5120_flash_shutdown(struct platform_device *dev)
{
	struct adm5120_flash_info *info = platform_get_drvdata(dev);

	if (info && info->mtd->suspend(info->mtd) == 0)
		info->mtd->resume(info->mtd);
}
#endif

static struct platform_driver adm5120_flash_driver = {
	.probe		= adm5120_flash_probe,
	.remove		= adm5120_flash_remove,
#ifdef CONFIG_PM
	.suspend	= adm5120_flash_suspend,
	.resume		= adm5120_flash_resume,
	.shutdown	= adm5120_flash_shutdown,
#endif
	.driver		= {
		.name	= DRV_NAME,
	},
};

static int __init adm5120_flash_init(void)
{
	int err;

	err = platform_driver_register(&adm5120_flash_driver);

	return err;
}

static void __exit adm5120_flash_exit(void)
{
	platform_driver_unregister(&adm5120_flash_driver);
}

module_init(adm5120_flash_init);
module_exit(adm5120_flash_exit);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Gabor Juhos <juhosg@openwrt.org>");
MODULE_DESCRIPTION(DRV_DESC);
