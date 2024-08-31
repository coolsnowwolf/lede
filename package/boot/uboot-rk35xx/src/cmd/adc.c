// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 BayLibre, SAS
 * Author: Neil Armstrong <narmstrong@baylibre.com>
 */
#include <common.h>
#include <command.h>
#include <dm.h>
#include <adc.h>

static int do_adc_list(cmd_tbl_t *cmdtp, int flag, int argc,
		       char *const argv[])
{
	struct udevice *dev;
	int ret, err;

	ret = err = uclass_first_device_check(UCLASS_ADC, &dev);

	while (dev) {
		printf("- %s status: %i\n", dev->name, ret);

		ret = uclass_next_device_check(&dev);
		if (ret)
			err = ret;
	}

	return err ? CMD_RET_FAILURE : CMD_RET_SUCCESS;
}

static int do_adc_info(cmd_tbl_t *cmdtp, int flag, int argc,
		       char *const argv[])
{
	struct udevice *dev;
	unsigned int data_mask, ch_mask;
	int ret, vss, vdd;

	if (argc < 2)
		return CMD_RET_USAGE;

	ret = uclass_get_device_by_name(UCLASS_ADC, argv[1], &dev);
	if (ret) {
		printf("Unknown ADC device %s\n", argv[1]);
		return CMD_RET_FAILURE;
	}

	printf("ADC Device '%s' :\n", argv[1]);

	ret = adc_channel_mask(dev, &ch_mask);
	if (!ret)
		printf("channel mask: %x\n", ch_mask);

	ret = adc_data_mask(dev, &data_mask);
	if (!ret)
		printf("data mask: %x\n", data_mask);

	ret = adc_vdd_value(dev, &vdd);
	if (!ret)
		printf("vdd: %duV\n", vdd);

	ret = adc_vss_value(dev, &vss);
	if (!ret)
		printf("vss: %duV\n", vss);

	return CMD_RET_SUCCESS;
}

static int do_adc_single(cmd_tbl_t *cmdtp, int flag, int argc,
			 char *const argv[])
{
	char *varname = NULL;
	struct udevice *dev;
	unsigned int data;
	int ret, uV, val;

	if (argc < 3)
		return CMD_RET_USAGE;

	if (argc >= 4)
		varname = argv[3];

	ret = adc_channel_single_shot(argv[1], simple_strtol(argv[2], NULL, 0),
				      &data);
	if (ret) {
		printf("Error getting single shot for device %s channel %s (ret=%d)\n",
		       argv[1], argv[2], ret);
		return CMD_RET_FAILURE;
	}

	ret = uclass_get_device_by_name(UCLASS_ADC, argv[1], &dev);
	if (!ret && !adc_raw_to_uV(dev, data, &uV)) {
		val = uV;
		printf("%u, %d uV\n", data, uV);
	} else {
		val = data;
		printf("%u\n", data);
	}

	if (varname)
		env_set_ulong(varname, val);

	return CMD_RET_SUCCESS;
}

static int do_adc_scan(cmd_tbl_t *cmdtp, int flag, int argc,
		       char *const argv[])
{
	struct adc_channel ch[ADC_MAX_CHANNEL];
	struct udevice *dev;
	unsigned int ch_mask;
	int i, chan, ret, uV;

	if (argc < 2)
		return CMD_RET_USAGE;

	ret = uclass_get_device_by_name(UCLASS_ADC, argv[1], &dev);
	if (ret) {
		pr_err("Can't get the ADC %s: %d\n", argv[1], ret);
		return CMD_RET_FAILURE;
	}

	switch (argc) {
	case 3:
		ch_mask = simple_strtoul(argv[2], NULL, 0);
		if (ch_mask)
			break;
	case 2:
		ret = adc_channel_mask(dev, &ch_mask);
		if (ret) {
			pr_err("Can't get mask for %s: %d\n", dev->name, ret);
			return CMD_RET_FAILURE;
		}
		break;
	}

	ret = adc_channels_single_shot(dev->name, ch_mask, ch);
	if (ret) {
		pr_err("Can't get single shot for %s (chans mask: 0x%x): %d\n",
		       dev->name, ch_mask, ret);
		return CMD_RET_FAILURE;
	}

	for (chan = 0, i = 0; chan < ADC_MAX_CHANNEL; chan++) {
		if (!(ch_mask & ADC_CHANNEL(chan)))
			continue;
		if (!adc_raw_to_uV(dev, ch[i].data, &uV))
			printf("[%02d]: %u, %d uV\n", ch[i].id, ch[i].data, uV);
		else
			printf("[%02d]: %u\n", ch[i].id, ch[i].data);
		i++;
	}

	return CMD_RET_SUCCESS;
}

static cmd_tbl_t cmd_adc_sub[] = {
	U_BOOT_CMD_MKENT(list, 1, 1, do_adc_list, "", ""),
	U_BOOT_CMD_MKENT(info, 2, 1, do_adc_info, "", ""),
	U_BOOT_CMD_MKENT(single, 4, 1, do_adc_single, "", ""),
	U_BOOT_CMD_MKENT(scan, 3, 1, do_adc_scan, "", ""),
};

static int do_adc(cmd_tbl_t *cmdtp, int flag, int argc,
		  char *const argv[])
{
	cmd_tbl_t *c;

	if (argc < 2)
		return CMD_RET_USAGE;

	/* Strip off leading 'adc' command argument */
	argc--;
	argv++;

	c = find_cmd_tbl(argv[0], &cmd_adc_sub[0], ARRAY_SIZE(cmd_adc_sub));

	if (c)
		return c->cmd(cmdtp, flag, argc, argv);
	else
		return CMD_RET_USAGE;
}

static char adc_help_text[] =
	"list - list ADC devices\n"
	"adc info <name> - Get ADC device info\n"
	"adc single <name> <channel> [varname] - Get Single data of ADC device channel\n"
	"adc scan <name> [channel mask] - Scan all [or masked] ADC channels";

U_BOOT_CMD(adc, 5, 1, do_adc, "ADC sub-system", adc_help_text);
