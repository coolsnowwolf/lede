/*
 * A hwmon driver for the Gateworks System Controller 
 * Copyright (C) 2009 Gateworks Corporation
 *
 * Author: Chris Lang <clang@gateworks.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,
 * as published by the Free Software Foundation - version 2.
 */

#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/err.h>
#include <linux/slab.h>

#define DRV_VERSION "0.2"

enum chips { gsp };

/* AD7418 registers */
#define GSP_REG_TEMP_IN		0x00
#define GSP_REG_VIN		0x02
#define GSP_REG_3P3		0x05
#define GSP_REG_BAT		0x08
#define GSP_REG_5P0		0x0b
#define GSP_REG_CORE		0x0e
#define GSP_REG_CPU1		0x11
#define GSP_REG_CPU2		0x14
#define GSP_REG_DRAM		0x17
#define GSP_REG_EXT_BAT		0x1a
#define GSP_REG_IO1		0x1d
#define GSP_REG_IO2 		0x20
#define GSP_REG_PCIE		0x23
#define GSP_REG_CURRENT		0x26
#define GSP_FAN_0		0x2C
#define GSP_FAN_1		0x2E
#define GSP_FAN_2		0x30
#define GSP_FAN_3		0x32
#define GSP_FAN_4		0x34
#define GSP_FAN_5		0x36

struct gsp_sensor_info {
	const char* name;
	int reg;
};

static const struct gsp_sensor_info gsp_sensors[] = {
	{"temp", GSP_REG_TEMP_IN},
	{"vin", GSP_REG_VIN},
	{"3p3", GSP_REG_3P3},
	{"bat", GSP_REG_BAT},
	{"5p0", GSP_REG_5P0},
	{"core", GSP_REG_CORE},
	{"cpu1", GSP_REG_CPU1},
	{"cpu2", GSP_REG_CPU2},
	{"dram", GSP_REG_DRAM},
	{"ext_bat", GSP_REG_EXT_BAT},
	{"io1", GSP_REG_IO1},
	{"io2", GSP_REG_IO2},
	{"pci2", GSP_REG_PCIE},
	{"current", GSP_REG_CURRENT},
	{"fan_point0", GSP_FAN_0},
	{"fan_point1", GSP_FAN_1},
	{"fan_point2", GSP_FAN_2},
	{"fan_point3", GSP_FAN_3},
	{"fan_point4", GSP_FAN_4},
	{"fan_point5", GSP_FAN_5},
};

struct gsp_data {
	struct device		*hwmon_dev;
	struct attribute_group	attrs;
	enum chips		type;
};

static int gsp_probe(struct i2c_client *client,
			const struct i2c_device_id *id);
static int gsp_remove(struct i2c_client *client);

static const struct i2c_device_id gsp_id[] = {
	{ "gsp", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, gsp_id);

static struct i2c_driver gsp_driver = {
	.driver = {
		.name	= "gsp",
	},
	.probe		= gsp_probe,
	.remove		= gsp_remove,
	.id_table	= gsp_id,
};

/* All registers are word-sized, except for the configuration registers.
 * AD7418 uses a high-byte first convention. Do NOT use those functions to
 * access the configuration registers CONF and CONF2, as they are byte-sized.
 */
static inline int gsp_read(struct i2c_client *client, u8 reg)
{
	unsigned int adc = 0;
	if (reg == GSP_REG_TEMP_IN || reg > GSP_REG_CURRENT)
	{
		adc |= i2c_smbus_read_byte_data(client, reg);
		adc |= i2c_smbus_read_byte_data(client, reg + 1) << 8;
		return adc;
	}
	else
	{
		adc |= i2c_smbus_read_byte_data(client, reg);
		adc |= i2c_smbus_read_byte_data(client, reg + 1) << 8;
		adc |= i2c_smbus_read_byte_data(client, reg + 2) << 16;
		return adc;
	}
}

static inline int gsp_write(struct i2c_client *client, u8 reg, u16 value)
{
	i2c_smbus_write_byte_data(client, reg, value & 0xff);
	i2c_smbus_write_byte_data(client, reg + 1, ((value >> 8) & 0xff));
	return 1;
}

static ssize_t show_adc(struct device *dev, struct device_attribute *devattr,
			char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
	struct i2c_client *client = to_i2c_client(dev);
	return sprintf(buf, "%d\n", gsp_read(client, gsp_sensors[attr->index].reg));
}

static ssize_t show_label(struct device *dev,
			struct device_attribute *devattr, char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);

	return sprintf(buf, "%s\n", gsp_sensors[attr->index].name);
}

static ssize_t store_fan(struct device *dev,
			struct device_attribute *devattr, const char *buf, size_t count)
{
	u16 val;
	struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
	struct i2c_client *client = to_i2c_client(dev);
	val = simple_strtoul(buf, NULL, 10);
	gsp_write(client, gsp_sensors[attr->index].reg, val);
	return count;
}

static SENSOR_DEVICE_ATTR(temp0_input, S_IRUGO, show_adc, NULL, 0);
static SENSOR_DEVICE_ATTR(temp0_label, S_IRUGO, show_label, NULL, 0);

static SENSOR_DEVICE_ATTR(in0_input, S_IRUGO, show_adc, NULL, 1);
static SENSOR_DEVICE_ATTR(in0_label, S_IRUGO, show_label, NULL, 1);
static SENSOR_DEVICE_ATTR(in1_input, S_IRUGO, show_adc, NULL, 2);
static SENSOR_DEVICE_ATTR(in1_label, S_IRUGO, show_label, NULL, 2);
static SENSOR_DEVICE_ATTR(in2_input, S_IRUGO, show_adc, NULL, 3);
static SENSOR_DEVICE_ATTR(in2_label, S_IRUGO, show_label, NULL, 3);
static SENSOR_DEVICE_ATTR(in3_input, S_IRUGO, show_adc, NULL, 4);
static SENSOR_DEVICE_ATTR(in3_label, S_IRUGO, show_label, NULL, 4);
static SENSOR_DEVICE_ATTR(in4_input, S_IRUGO, show_adc, NULL, 5);
static SENSOR_DEVICE_ATTR(in4_label, S_IRUGO, show_label, NULL, 5);
static SENSOR_DEVICE_ATTR(in5_input, S_IRUGO, show_adc, NULL, 6);
static SENSOR_DEVICE_ATTR(in5_label, S_IRUGO, show_label, NULL, 6);
static SENSOR_DEVICE_ATTR(in6_input, S_IRUGO, show_adc, NULL, 7);
static SENSOR_DEVICE_ATTR(in6_label, S_IRUGO, show_label, NULL, 7);
static SENSOR_DEVICE_ATTR(in7_input, S_IRUGO, show_adc, NULL, 8);
static SENSOR_DEVICE_ATTR(in7_label, S_IRUGO, show_label, NULL, 8);
static SENSOR_DEVICE_ATTR(in8_input, S_IRUGO, show_adc, NULL, 9);
static SENSOR_DEVICE_ATTR(in8_label, S_IRUGO, show_label, NULL, 9);
static SENSOR_DEVICE_ATTR(in9_input, S_IRUGO, show_adc, NULL, 10);
static SENSOR_DEVICE_ATTR(in9_label, S_IRUGO, show_label, NULL, 10);
static SENSOR_DEVICE_ATTR(in10_input, S_IRUGO, show_adc, NULL, 11);
static SENSOR_DEVICE_ATTR(in10_label, S_IRUGO, show_label, NULL, 11);
static SENSOR_DEVICE_ATTR(in11_input, S_IRUGO, show_adc, NULL, 12);
static SENSOR_DEVICE_ATTR(in11_label, S_IRUGO, show_label, NULL, 12);
static SENSOR_DEVICE_ATTR(in12_input, S_IRUGO, show_adc, NULL, 13);
static SENSOR_DEVICE_ATTR(in12_label, S_IRUGO, show_label, NULL, 13);

static SENSOR_DEVICE_ATTR(fan0_point0, S_IRUGO | S_IWUSR, show_adc, store_fan, 14);
static SENSOR_DEVICE_ATTR(fan0_point1, S_IRUGO | S_IWUSR, show_adc, store_fan, 15);
static SENSOR_DEVICE_ATTR(fan0_point2, S_IRUGO | S_IWUSR, show_adc, store_fan, 16);
static SENSOR_DEVICE_ATTR(fan0_point3, S_IRUGO | S_IWUSR, show_adc, store_fan, 17);
static SENSOR_DEVICE_ATTR(fan0_point4, S_IRUGO | S_IWUSR, show_adc, store_fan, 18);
static SENSOR_DEVICE_ATTR(fan0_point5, S_IRUGO | S_IWUSR, show_adc, store_fan, 19);

static struct attribute *gsp_attributes[] = {
	&sensor_dev_attr_temp0_input.dev_attr.attr,
	&sensor_dev_attr_in0_input.dev_attr.attr,
	&sensor_dev_attr_in1_input.dev_attr.attr,
	&sensor_dev_attr_in2_input.dev_attr.attr,
	&sensor_dev_attr_in3_input.dev_attr.attr,
	&sensor_dev_attr_in4_input.dev_attr.attr,
	&sensor_dev_attr_in5_input.dev_attr.attr,
	&sensor_dev_attr_in6_input.dev_attr.attr,
	&sensor_dev_attr_in7_input.dev_attr.attr,
	&sensor_dev_attr_in8_input.dev_attr.attr,
	&sensor_dev_attr_in9_input.dev_attr.attr,
	&sensor_dev_attr_in10_input.dev_attr.attr,
	&sensor_dev_attr_in11_input.dev_attr.attr,
	&sensor_dev_attr_in12_input.dev_attr.attr,

	&sensor_dev_attr_temp0_label.dev_attr.attr,
	&sensor_dev_attr_in0_label.dev_attr.attr,
	&sensor_dev_attr_in1_label.dev_attr.attr,
	&sensor_dev_attr_in2_label.dev_attr.attr,
	&sensor_dev_attr_in3_label.dev_attr.attr,
	&sensor_dev_attr_in4_label.dev_attr.attr,
	&sensor_dev_attr_in5_label.dev_attr.attr,
	&sensor_dev_attr_in6_label.dev_attr.attr,
	&sensor_dev_attr_in7_label.dev_attr.attr,
	&sensor_dev_attr_in8_label.dev_attr.attr,
	&sensor_dev_attr_in9_label.dev_attr.attr,
	&sensor_dev_attr_in10_label.dev_attr.attr,
	&sensor_dev_attr_in11_label.dev_attr.attr,
	&sensor_dev_attr_in12_label.dev_attr.attr,

	&sensor_dev_attr_fan0_point0.dev_attr.attr,
	&sensor_dev_attr_fan0_point1.dev_attr.attr,
	&sensor_dev_attr_fan0_point2.dev_attr.attr,
	&sensor_dev_attr_fan0_point3.dev_attr.attr,
	&sensor_dev_attr_fan0_point4.dev_attr.attr,
	&sensor_dev_attr_fan0_point5.dev_attr.attr,
	NULL
};


static int gsp_probe(struct i2c_client *client,
			 const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = client->adapter;
	struct gsp_data *data;
	int err;

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA |
					I2C_FUNC_SMBUS_WORD_DATA)) {
		err = -EOPNOTSUPP;
		goto exit;
	}

	if (!(data = kzalloc(sizeof(struct gsp_data), GFP_KERNEL))) {
		err = -ENOMEM;
		goto exit;
	}

	i2c_set_clientdata(client, data);

	data->type = id->driver_data;

	switch (data->type) {
	case 0:
		data->attrs.attrs = gsp_attributes;
		break;
	}

	dev_info(&client->dev, "%s chip found\n", client->name);

	/* Register sysfs hooks */
	if ((err = sysfs_create_group(&client->dev.kobj, &data->attrs)))
		goto exit_free;

	data->hwmon_dev = hwmon_device_register(&client->dev);
	if (IS_ERR(data->hwmon_dev)) {
		err = PTR_ERR(data->hwmon_dev);
		goto exit_remove;
	}

	return 0;

exit_remove:
	sysfs_remove_group(&client->dev.kobj, &data->attrs);
exit_free:
	kfree(data);
exit:
	return err;
}

static int gsp_remove(struct i2c_client *client)
{
	struct gsp_data *data = i2c_get_clientdata(client);
	hwmon_device_unregister(data->hwmon_dev);
	sysfs_remove_group(&client->dev.kobj, &data->attrs);
	kfree(data);
	return 0;
}

static int __init gsp_init(void)
{
	return i2c_add_driver(&gsp_driver);
}

static void __exit gsp_exit(void)
{
	i2c_del_driver(&gsp_driver);
}

module_init(gsp_init);
module_exit(gsp_exit);

MODULE_AUTHOR("Chris Lang <clang@gateworks.com>");
MODULE_DESCRIPTION("GSC HWMON driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRV_VERSION);

