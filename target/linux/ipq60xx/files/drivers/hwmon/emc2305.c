// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2021 Sartura Ltd.
 *
 * Driver for the SMSC/Microchip EMC2301/2/3/5 fan controller.
 *
 * Author: Robert Marko <robert.marko@sartura.hr>
 */

#include <linux/bitfield.h>
#include <linux/delay.h>
#include <linux/hwmon-sysfs.h>
#include <linux/hwmon.h>
#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/regmap.h>

#define MANUFACTURER_ID_REG	0xfe
#define SMSC_MANUFACTURER_ID	0x5d

#define PRODUCT_ID_REG		0xfd
#define EMC2305_PRODUCT_ID	0x34
#define EMC2303_PRODUCT_ID	0x35
#define EMC2302_PRODUCT_ID	0x36
#define EMC2301_PRODUCT_ID	0x37

#define TACH1_HIGH_BYTE		0x3e
#define TACH1_LOW_BYTE		0x3f

#define FAN1_CONFIG		0x32
#define FAN_TACH_RANGE_MASK	GENMASK(6, 5)
#define FAN_TACH_MULTIPLIER_8	3
#define FAN_TACH_MULTIPLIER_4	2
#define FAN_TACH_MULTIPLIER_2	1
#define FAN_TACH_MULTIPLIER_1	0
#define FAN_TACH_CONSTANT	3932160
#define FAN_TACH_READING_MASK	GENMASK(15,3)

#define TACH1_TARGET_LOW_BYTE	0x3c
#define TACH1_TARGET_HIGH_BYTE	0x3d
#define TACH_HIGH_MASK		GENMASK(12,5)
#define TACH_LOW_MASK		GENMASK(4,0)

#define FANX_OFFSET		0x10
#define FAN_NUM_MAX		5

struct emc2305_fan_data {
	u32 min_rpm;
	u32 max_rpm;
	u32 target_rpm;
};

struct emc2305_data {
	struct regmap *regmap;
	struct i2c_client *client;
	struct emc2305_fan_data fan_data[FAN_NUM_MAX];
};

static struct regmap_config emc2305_regmap_config = {
	.reg_bits = 8,
	.val_bits = 8,
	.max_register = 0xff,
};

static int emc2305_read_fan(struct emc2305_data *data, int channel,
			    long *val)
{
	unsigned int regval, high_byte, low_byte;
	u8 range, multiplier;
	int ret;

	ret = regmap_read(data->regmap,
			  FAN1_CONFIG + channel * FANX_OFFSET,
			  &regval);
	if (ret < 0)
		return ret;

	range = FIELD_GET(FAN_TACH_RANGE_MASK, regval);

	switch (range) {
	case FAN_TACH_MULTIPLIER_8:
		multiplier = 8;
		break;
	case FAN_TACH_MULTIPLIER_4:
		multiplier = 4;
		break;
	case FAN_TACH_MULTIPLIER_2:
		multiplier = 2;
		break;
	case FAN_TACH_MULTIPLIER_1:
		multiplier = 1;
		break;
	default:
		return -EINVAL;
	}

	ret = regmap_read(data->regmap,
			  TACH1_HIGH_BYTE + channel * FANX_OFFSET,
			  &high_byte);
	if (ret < 0)
		return ret;

	ret = regmap_read(data->regmap,
			  TACH1_LOW_BYTE + channel * FANX_OFFSET,
			  &low_byte);
	if (ret < 0)
		return ret;

	regval = (u8) high_byte << 8 | (u8) low_byte;

	*val = (FAN_TACH_CONSTANT * multiplier) / FIELD_GET(FAN_TACH_READING_MASK, regval);

	return 0;
}

static int emc2305_read_fan_target(struct emc2305_data *data, int channel,
				   long *val)
{
	unsigned int regval;
	int ret;

	ret = regmap_bulk_read(data->regmap,
			       TACH1_TARGET_LOW_BYTE + channel * FANX_OFFSET,
			       &regval,
			       2);
	if (ret < 0)
		return ret;

	*val = regval;

	return 0;
}

static int emc2305_set_fan_target(struct emc2305_data *data, int channel,
				  long val)
{
	int ret;

	ret = regmap_write(data->regmap,
			   TACH1_TARGET_LOW_BYTE + channel * FANX_OFFSET,
			   val & TACH_LOW_MASK);
	if (ret < 0)
		return ret;

	ret = regmap_write(data->regmap,
			   TACH1_TARGET_HIGH_BYTE + channel * FANX_OFFSET,
			   (val & TACH_HIGH_MASK) >> 5);
	if (ret < 0)
		return ret;

	return 0;
}

static int emc2305_write(struct device *dev, enum hwmon_sensor_types type,
			 u32 attr, int channel, long val)
{
	struct emc2305_data *data = dev_get_drvdata(dev);
	int err;

	switch (type) {
	case hwmon_fan:
		switch (attr) {
		case hwmon_fan_target:
			err = emc2305_set_fan_target(data, channel, val);
			break;
		default:
			return -EOPNOTSUPP;
		}
		break;
	default:
		return -EOPNOTSUPP;
	}

	return err;
}

static int emc2305_read(struct device *dev, enum hwmon_sensor_types type,
			u32 attr, int channel, long *val)
{
	struct emc2305_data *data = dev_get_drvdata(dev);
	int err;

	switch (type) {
	case hwmon_fan:
		switch (attr) {
		case hwmon_fan_input:
			err = emc2305_read_fan(data, channel, val);
			break;
		case hwmon_fan_target:
			err = emc2305_read_fan_target(data, channel, val);
			break;
		default:
			return -EOPNOTSUPP;
		}
		break;
	default:
		return -EOPNOTSUPP;
	}

	return err;
}

static const char * const emc2305_fan_label[] = {
	"Fan1",
	"Fan2",
	"Fan3",
	"Fan4",
	"Fan5",
};

static int emc2305_read_string(struct device *dev,
			       enum hwmon_sensor_types type,
			       u32 attr, int channel, const char **str)
{
	switch (type) {
	case hwmon_fan:
		*str = emc2305_fan_label[channel];
		break;
	default:
		return -EOPNOTSUPP;
	}

	return 0;
}

static umode_t emc2305_is_visible(const void *data, enum hwmon_sensor_types type,
				  u32 attr, int channel)
{
	switch (type) {
	case hwmon_fan:
		switch (attr) {
		case hwmon_fan_input:
		case hwmon_fan_label:
			return 0444;
		case hwmon_fan_target:
			return 0644;
		default:
			return 0;
		}
	default:
		return 0;
	}
}

static const struct hwmon_channel_info *emc2301_info[] = {
	HWMON_CHANNEL_INFO(fan,
			   HWMON_F_INPUT | HWMON_F_TARGET | HWMON_F_LABEL),
	NULL
};

static const struct hwmon_channel_info *emc2302_info[] = {
	HWMON_CHANNEL_INFO(fan,
			   HWMON_F_INPUT | HWMON_F_TARGET | HWMON_F_LABEL,
			   HWMON_F_INPUT | HWMON_F_TARGET | HWMON_F_LABEL),
	NULL
};

static const struct hwmon_channel_info *emc2303_info[] = {
	HWMON_CHANNEL_INFO(fan,
			   HWMON_F_INPUT | HWMON_F_TARGET | HWMON_F_LABEL,
			   HWMON_F_INPUT | HWMON_F_TARGET | HWMON_F_LABEL,
			   HWMON_F_INPUT | HWMON_F_TARGET | HWMON_F_LABEL),
	NULL
};

static const struct hwmon_channel_info *emc2305_info[] = {
	HWMON_CHANNEL_INFO(fan,
			   HWMON_F_INPUT | HWMON_F_TARGET | HWMON_F_LABEL,
			   HWMON_F_INPUT | HWMON_F_TARGET | HWMON_F_LABEL,
			   HWMON_F_INPUT | HWMON_F_TARGET | HWMON_F_LABEL,
			   HWMON_F_INPUT | HWMON_F_TARGET | HWMON_F_LABEL,
			   HWMON_F_INPUT | HWMON_F_TARGET | HWMON_F_LABEL),
	NULL
};

static const struct hwmon_ops emc2305_hwmon_ops = {
	.is_visible = emc2305_is_visible,
	.write = emc2305_write,
	.read = emc2305_read,
	.read_string = emc2305_read_string,
};

static struct hwmon_chip_info emc2305_chip_info = {
	.ops = &emc2305_hwmon_ops,
};

static int emc2305_probe(struct i2c_client *client)
{
	struct device *dev = &client->dev;
	struct emc2305_data *data;
	struct device *hwmon_dev;
	const char *model_name;
	unsigned int regval;
	int ret;

	data = devm_kzalloc(dev, sizeof(*data), GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	data->client = client;
	i2c_set_clientdata(client, data);

	data->regmap = devm_regmap_init_i2c(client, &emc2305_regmap_config);
	if (IS_ERR(data->regmap)) {
		dev_err(dev, "failed to allocate register map\n");
		return PTR_ERR(data->regmap);
	}

	ret = regmap_read(data->regmap, MANUFACTURER_ID_REG, &regval);
	if (ret < 0)
		return ret;

	if (regval != SMSC_MANUFACTURER_ID) {
		dev_err(dev, "Invalid manufacturer id: 0x%x\n", regval);
		return -ENODEV;
	}

	ret = regmap_read(data->regmap, PRODUCT_ID_REG, &regval);
	if (ret < 0)
		return ret;

	switch (regval) {
	case EMC2305_PRODUCT_ID:
		model_name = "emc2305";
		emc2305_chip_info.info = emc2305_info;
		break;
	case EMC2303_PRODUCT_ID:
		model_name = "emc2303";
		emc2305_chip_info.info = emc2303_info;
		break;
	case EMC2302_PRODUCT_ID:
		model_name = "emc2302";
		emc2305_chip_info.info = emc2302_info;
		break;
	case EMC2301_PRODUCT_ID:
		model_name = "emc2301";
		emc2305_chip_info.info = emc2301_info;
		break;
	default:
		dev_err(dev, "Unknown ID detected: 0x%x\n", regval);
		return -ENODEV;
	}

	dev_info(dev, "%s detected\n", model_name);

	hwmon_dev = devm_hwmon_device_register_with_info(dev, model_name,
							 data, &emc2305_chip_info,
							 NULL);
	if (IS_ERR(hwmon_dev))
		return PTR_ERR(hwmon_dev);

	return 0;
}

static const struct of_device_id emc2305_of_match[] = {
	{ .compatible = "smsc,emc2301", },
	{ .compatible = "smsc,emc2302", },
	{ .compatible = "smsc,emc2303", },
	{ .compatible = "smsc,emc2305", },
	{ },
};
MODULE_DEVICE_TABLE(of, emc2305_of_match);

static struct i2c_driver emc2305_driver = {
	.probe_new		= emc2305_probe,
	.driver = {
		.name		= "emc2305",
		.of_match_table	= of_match_ptr(emc2305_of_match),
	},
};
module_i2c_driver(emc2305_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Robert Marko <robert.marko@sartura.hr>");
MODULE_DESCRIPTION("SMSC EMC2301/2/3/5 fan controller");
