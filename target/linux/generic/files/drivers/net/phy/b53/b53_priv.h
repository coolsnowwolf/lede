/*
 * B53 common definitions
 *
 * Copyright (C) 2011-2013 Jonas Gorski <jogo@openwrt.org>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef __B53_PRIV_H
#define __B53_PRIV_H

#include <linux/kernel.h>
#include <linux/mutex.h>
#include <linux/switch.h>

struct b53_device;

struct b53_io_ops {
	int (*read8)(struct b53_device *dev, u8 page, u8 reg, u8 *value);
	int (*read16)(struct b53_device *dev, u8 page, u8 reg, u16 *value);
	int (*read32)(struct b53_device *dev, u8 page, u8 reg, u32 *value);
	int (*read48)(struct b53_device *dev, u8 page, u8 reg, u64 *value);
	int (*read64)(struct b53_device *dev, u8 page, u8 reg, u64 *value);
	int (*write8)(struct b53_device *dev, u8 page, u8 reg, u8 value);
	int (*write16)(struct b53_device *dev, u8 page, u8 reg, u16 value);
	int (*write32)(struct b53_device *dev, u8 page, u8 reg, u32 value);
	int (*write48)(struct b53_device *dev, u8 page, u8 reg, u64 value);
	int (*write64)(struct b53_device *dev, u8 page, u8 reg, u64 value);
	int (*phy_read16)(struct b53_device *dev, int addr, u8 reg, u16 *value);
	int (*phy_write16)(struct b53_device *dev, int addr, u8 reg, u16 value);
};

enum {
	BCM5325_DEVICE_ID = 0x25,
	BCM5365_DEVICE_ID = 0x65,
	BCM5395_DEVICE_ID = 0x95,
	BCM5397_DEVICE_ID = 0x97,
	BCM5398_DEVICE_ID = 0x98,
	BCM53115_DEVICE_ID = 0x53115,
	BCM53125_DEVICE_ID = 0x53125,
	BCM53128_DEVICE_ID = 0x53128,
	BCM63XX_DEVICE_ID = 0x6300,
	BCM53010_DEVICE_ID = 0x53010,
	BCM53011_DEVICE_ID = 0x53011,
	BCM53012_DEVICE_ID = 0x53012,
	BCM53018_DEVICE_ID = 0x53018,
	BCM53019_DEVICE_ID = 0x53019,
};

#define B53_N_PORTS	9
#define B53_N_PORTS_25	6

struct b53_vlan {
	unsigned int	members:B53_N_PORTS;
	unsigned int	untag:B53_N_PORTS;
};

struct b53_port {
	unsigned int	pvid:12;
};

struct b53_device {
	struct switch_dev sw_dev;
	struct b53_platform_data *pdata;

	struct mutex reg_mutex;
	const struct b53_io_ops *ops;

	/* chip specific data */
	u32 chip_id;
	u8 core_rev;
	u8 vta_regs[3];
	u8 duplex_reg;
	u8 jumbo_pm_reg;
	u8 jumbo_size_reg;
	int reset_gpio;

	/* used ports mask */
	u16 enabled_ports;

	/* connect specific data */
	u8 current_page;
	struct device *dev;
	void *priv;

	/* run time configuration */
	unsigned enable_vlan:1;
	unsigned enable_jumbo:1;
	unsigned allow_vid_4095:1;

	struct b53_port *ports;
	struct b53_vlan *vlans;

	char *buf;
};

#define b53_for_each_port(dev, i) \
	for (i = 0; i < B53_N_PORTS; i++) \
		if (dev->enabled_ports & BIT(i))



static inline int is5325(struct b53_device *dev)
{
	return dev->chip_id == BCM5325_DEVICE_ID;
}

static inline int is5365(struct b53_device *dev)
{
#ifdef CONFIG_BCM47XX
	return dev->chip_id == BCM5365_DEVICE_ID;
#else
	return 0;
#endif
}

static inline int is5397_98(struct b53_device *dev)
{
	return dev->chip_id == BCM5397_DEVICE_ID ||
		dev->chip_id == BCM5398_DEVICE_ID;
}

static inline int is539x(struct b53_device *dev)
{
	return dev->chip_id == BCM5395_DEVICE_ID ||
		dev->chip_id == BCM5397_DEVICE_ID ||
		dev->chip_id == BCM5398_DEVICE_ID;
}

static inline int is531x5(struct b53_device *dev)
{
	return dev->chip_id == BCM53115_DEVICE_ID ||
		dev->chip_id == BCM53125_DEVICE_ID ||
		dev->chip_id == BCM53128_DEVICE_ID;
}

static inline int is63xx(struct b53_device *dev)
{
#ifdef CONFIG_BCM63XX
	return dev->chip_id == BCM63XX_DEVICE_ID;
#else
	return 0;
#endif
}

static inline int is5301x(struct b53_device *dev)
{
	return dev->chip_id == BCM53010_DEVICE_ID ||
		dev->chip_id == BCM53011_DEVICE_ID ||
		dev->chip_id == BCM53012_DEVICE_ID ||
		dev->chip_id == BCM53018_DEVICE_ID ||
		dev->chip_id == BCM53019_DEVICE_ID;
}

#define B53_CPU_PORT_25	5
#define B53_CPU_PORT	8

static inline int is_cpu_port(struct b53_device *dev, int port)
{
	return dev->sw_dev.cpu_port == port;
}

static inline int is_imp_port(struct b53_device *dev, int port)
{
	if (is5325(dev) || is5365(dev))
		return port == B53_CPU_PORT_25;
	else
		return port == B53_CPU_PORT;
}

static inline struct b53_device *sw_to_b53(struct switch_dev *sw)
{
	return container_of(sw, struct b53_device, sw_dev);
}

struct b53_device *b53_swconfig_switch_alloc(struct device *base, struct b53_io_ops *ops,
					     void *priv);

int b53_swconfig_switch_detect(struct b53_device *dev);

int b53_swconfig_switch_register(struct b53_device *dev);

static inline void b53_switch_remove(struct b53_device *dev)
{
	unregister_switch(&dev->sw_dev);
}

static inline int b53_read8(struct b53_device *dev, u8 page, u8 reg, u8 *val)
{
	int ret;

	mutex_lock(&dev->reg_mutex);
	ret = dev->ops->read8(dev, page, reg, val);
	mutex_unlock(&dev->reg_mutex);

	return ret;
}

static inline int b53_read16(struct b53_device *dev, u8 page, u8 reg, u16 *val)
{
	int ret;

	mutex_lock(&dev->reg_mutex);
	ret = dev->ops->read16(dev, page, reg, val);
	mutex_unlock(&dev->reg_mutex);

	return ret;
}

static inline int b53_read32(struct b53_device *dev, u8 page, u8 reg, u32 *val)
{
	int ret;

	mutex_lock(&dev->reg_mutex);
	ret = dev->ops->read32(dev, page, reg, val);
	mutex_unlock(&dev->reg_mutex);

	return ret;
}

static inline int b53_read48(struct b53_device *dev, u8 page, u8 reg, u64 *val)
{
	int ret;

	mutex_lock(&dev->reg_mutex);
	ret = dev->ops->read48(dev, page, reg, val);
	mutex_unlock(&dev->reg_mutex);

	return ret;
}

static inline int b53_read64(struct b53_device *dev, u8 page, u8 reg, u64 *val)
{
	int ret;

	mutex_lock(&dev->reg_mutex);
	ret = dev->ops->read64(dev, page, reg, val);
	mutex_unlock(&dev->reg_mutex);

	return ret;
}

static inline int b53_write8(struct b53_device *dev, u8 page, u8 reg, u8 value)
{
	int ret;

	mutex_lock(&dev->reg_mutex);
	ret = dev->ops->write8(dev, page, reg, value);
	mutex_unlock(&dev->reg_mutex);

	return ret;
}

static inline int b53_write16(struct b53_device *dev, u8 page, u8 reg,
			      u16 value)
{
	int ret;

	mutex_lock(&dev->reg_mutex);
	ret = dev->ops->write16(dev, page, reg, value);
	mutex_unlock(&dev->reg_mutex);

	return ret;
}

static inline int b53_write32(struct b53_device *dev, u8 page, u8 reg,
			      u32 value)
{
	int ret;

	mutex_lock(&dev->reg_mutex);
	ret = dev->ops->write32(dev, page, reg, value);
	mutex_unlock(&dev->reg_mutex);

	return ret;
}

static inline int b53_write48(struct b53_device *dev, u8 page, u8 reg,
			      u64 value)
{
	int ret;

	mutex_lock(&dev->reg_mutex);
	ret = dev->ops->write48(dev, page, reg, value);
	mutex_unlock(&dev->reg_mutex);

	return ret;
}

static inline int b53_write64(struct b53_device *dev, u8 page, u8 reg,
			       u64 value)
{
	int ret;

	mutex_lock(&dev->reg_mutex);
	ret = dev->ops->write64(dev, page, reg, value);
	mutex_unlock(&dev->reg_mutex);

	return ret;
}

#ifdef CONFIG_BCM47XX
#include <bcm47xx_board.h>
#endif

#include <linux/version.h>
#include <linux/bcm47xx_nvram.h>

static inline int b53_switch_get_reset_gpio(struct b53_device *dev)
{
#ifdef CONFIG_BCM47XX
	enum bcm47xx_board board = bcm47xx_board_get();

	switch (board) {
	case BCM47XX_BOARD_LINKSYS_WRT300NV11:
	case BCM47XX_BOARD_LINKSYS_WRT310NV1:
		return 8;
	default:
		break;
	}
#endif

	return bcm47xx_nvram_gpio_pin("robo_reset");
}

#endif
