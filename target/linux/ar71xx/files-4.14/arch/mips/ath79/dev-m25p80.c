/*
 *  Copyright (C) 2009-2012 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/spi/spi.h>
#include <linux/spi/flash.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/mtd/concat.h>

#include "dev-spi.h"
#include "dev-m25p80.h"

static struct spi_board_info ath79_spi_info[] = {
	{
		.bus_num	= 0,
		.chip_select	= 0,
		.max_speed_hz	= 25000000,
		.modalias	= "m25p80",
	},
	{
		.bus_num	= 0,
		.chip_select	= 1,
		.max_speed_hz   = 25000000,
		.modalias	= "m25p80",
	}
};

static struct ath79_spi_platform_data ath79_spi_data;

void __init ath79_register_m25p80(struct flash_platform_data *pdata)
{
	ath79_spi_data.bus_num = 0;
	ath79_spi_data.num_chipselect = 1;
	ath79_spi_info[0].platform_data = pdata;
	ath79_register_spi(&ath79_spi_data, ath79_spi_info, 1);
}

static struct flash_platform_data *multi_pdata;

static struct mtd_info *concat_devs[2] = { NULL, NULL };
static struct work_struct mtd_concat_work;

static void mtd_concat_add_work(struct work_struct *work)
{
	struct mtd_info *mtd;

	mtd = mtd_concat_create(concat_devs, ARRAY_SIZE(concat_devs), "flash");

	mtd_device_register(mtd, multi_pdata->parts, multi_pdata->nr_parts);
}

static void mtd_concat_add(struct mtd_info *mtd)
{
	static bool registered = false;

	if (registered)
		return;

	if (!strcmp(mtd->name, "spi0.0"))
		concat_devs[0] = mtd;
	else if (!strcmp(mtd->name, "spi0.1"))
		concat_devs[1] = mtd;
	else
		return;

	if (!concat_devs[0] || !concat_devs[1])
		return;

	registered = true;
	INIT_WORK(&mtd_concat_work, mtd_concat_add_work);
	schedule_work(&mtd_concat_work);
}

static void mtd_concat_remove(struct mtd_info *mtd)
{
}

static void add_mtd_concat_notifier(void)
{
	static struct mtd_notifier not = {
		.add = mtd_concat_add,
		.remove = mtd_concat_remove,
	};

	register_mtd_user(&not);
}

void __init ath79_register_m25p80_multi(struct flash_platform_data *pdata)
{
	multi_pdata = pdata;
	add_mtd_concat_notifier();
	ath79_spi_data.bus_num = 0;
	ath79_spi_data.num_chipselect = 2;
	ath79_register_spi(&ath79_spi_data, ath79_spi_info, 2);
}
