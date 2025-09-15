// SPDX-License-Identifier: GPL-2.0+
/*
 * MFD driver for Airoha AN8855 Switch
 */

#include <linux/mfd/airoha-an8855-mfd.h>
#include <linux/mfd/core.h>
#include <linux/mdio.h>
#include <linux/module.h>
#include <linux/phy.h>
#include <linux/regmap.h>

static const struct mfd_cell an8855_mfd_devs[] = {
	{
		.name = "an8855-efuse",
		.of_compatible = "airoha,an8855-efuse",
	}, {
		.name = "an8855-switch",
		.of_compatible = "airoha,an8855-switch",
	}, {
		.name = "an8855-mdio",
		.of_compatible = "airoha,an8855-mdio",
	}
};

int an8855_mii_set_page(struct an8855_mfd_priv *priv, u8 phy_id,
			u8 page) __must_hold(&priv->bus->mdio_lock)
{
	struct mii_bus *bus = priv->bus;
	int ret;

	ret = __mdiobus_write(bus, phy_id, AN8855_PHY_SELECT_PAGE, page);
	if (ret < 0)
		dev_err_ratelimited(&bus->dev,
				    "failed to set an8855 mii page\n");

	/* Cache current page if next mii read/write is for switch */
	priv->current_page = page;
	return ret < 0 ? ret : 0;
}
EXPORT_SYMBOL_GPL(an8855_mii_set_page);

static int an8855_mii_read32(struct mii_bus *bus, u8 phy_id, u32 reg,
			     u32 *val) __must_hold(&bus->mdio_lock)
{
	int lo, hi, ret;

	ret = __mdiobus_write(bus, phy_id, AN8855_PBUS_MODE,
			      AN8855_PBUS_MODE_ADDR_FIXED);
	if (ret < 0)
		goto err;

	ret = __mdiobus_write(bus, phy_id, AN8855_PBUS_RD_ADDR_HIGH,
			      upper_16_bits(reg));
	if (ret < 0)
		goto err;
	ret = __mdiobus_write(bus, phy_id, AN8855_PBUS_RD_ADDR_LOW,
			      lower_16_bits(reg));
	if (ret < 0)
		goto err;

	hi = __mdiobus_read(bus, phy_id, AN8855_PBUS_RD_DATA_HIGH);
	if (hi < 0) {
		ret = hi;
		goto err;
	}
	lo = __mdiobus_read(bus, phy_id, AN8855_PBUS_RD_DATA_LOW);
	if (lo < 0) {
		ret = lo;
		goto err;
	}

	*val = ((u16)hi << 16) | ((u16)lo & 0xffff);

	return 0;
err:
	dev_err_ratelimited(&bus->dev,
			    "failed to read an8855 register\n");
	return ret;
}

static int an8855_regmap_read(void *ctx, uint32_t reg, uint32_t *val)
{
	struct an8855_mfd_priv *priv = ctx;
	struct mii_bus *bus = priv->bus;
	u16 addr = priv->switch_addr;
	int ret;

	mutex_lock_nested(&bus->mdio_lock, MDIO_MUTEX_NESTED);
	ret = an8855_mii_set_page(priv, addr, AN8855_PHY_PAGE_EXTENDED_4);
	if (ret < 0)
		goto exit;

	ret = an8855_mii_read32(bus, addr, reg, val);

exit:
	mutex_unlock(&bus->mdio_lock);

	return ret < 0 ? ret : 0;
}

static int an8855_mii_write32(struct mii_bus *bus, u8 phy_id, u32 reg,
			      u32 val) __must_hold(&bus->mdio_lock)
{
	int ret;

	ret = __mdiobus_write(bus, phy_id, AN8855_PBUS_MODE,
			      AN8855_PBUS_MODE_ADDR_FIXED);
	if (ret < 0)
		goto err;

	ret = __mdiobus_write(bus, phy_id, AN8855_PBUS_WR_ADDR_HIGH,
			      upper_16_bits(reg));
	if (ret < 0)
		goto err;
	ret = __mdiobus_write(bus, phy_id, AN8855_PBUS_WR_ADDR_LOW,
			      lower_16_bits(reg));
	if (ret < 0)
		goto err;

	ret = __mdiobus_write(bus, phy_id, AN8855_PBUS_WR_DATA_HIGH,
			      upper_16_bits(val));
	if (ret < 0)
		goto err;
	ret = __mdiobus_write(bus, phy_id, AN8855_PBUS_WR_DATA_LOW,
			      lower_16_bits(val));
	if (ret < 0)
		goto err;

	return 0;
err:
	dev_err_ratelimited(&bus->dev,
			    "failed to write an8855 register\n");
	return ret;
}

static int
an8855_regmap_write(void *ctx, uint32_t reg, uint32_t val)
{
	struct an8855_mfd_priv *priv = ctx;
	struct mii_bus *bus = priv->bus;
	u16 addr = priv->switch_addr;
	int ret;

	mutex_lock_nested(&bus->mdio_lock, MDIO_MUTEX_NESTED);
	ret = an8855_mii_set_page(priv, addr, AN8855_PHY_PAGE_EXTENDED_4);
	if (ret < 0)
		goto exit;

	ret = an8855_mii_write32(bus, addr, reg, val);

exit:
	mutex_unlock(&bus->mdio_lock);

	return ret < 0 ? ret : 0;
}

static int an8855_regmap_update_bits(void *ctx, uint32_t reg, uint32_t mask,
				     uint32_t write_val)
{
	struct an8855_mfd_priv *priv = ctx;
	struct mii_bus *bus = priv->bus;
	u16 addr = priv->switch_addr;
	u32 val;
	int ret;

	mutex_lock_nested(&bus->mdio_lock, MDIO_MUTEX_NESTED);
	ret = an8855_mii_set_page(priv, addr, AN8855_PHY_PAGE_EXTENDED_4);
	if (ret < 0)
		goto exit;

	ret = an8855_mii_read32(bus, addr, reg, &val);
	if (ret < 0)
		goto exit;

	val &= ~mask;
	val |= write_val;
	ret = an8855_mii_write32(bus, addr, reg, val);

exit:
	mutex_unlock(&bus->mdio_lock);

	return ret < 0 ? ret : 0;
}

static const struct regmap_range an8855_readable_ranges[] = {
	regmap_reg_range(0x10000000, 0x10000fff), /* SCU */
	regmap_reg_range(0x10001000, 0x10001fff), /* RBUS */
	regmap_reg_range(0x10002000, 0x10002fff), /* MCU */
	regmap_reg_range(0x10005000, 0x10005fff), /* SYS SCU */
	regmap_reg_range(0x10007000, 0x10007fff), /* I2C Slave */
	regmap_reg_range(0x10008000, 0x10008fff), /* I2C Master */
	regmap_reg_range(0x10009000, 0x10009fff), /* PDMA */
	regmap_reg_range(0x1000a100, 0x1000a2ff), /* General Purpose Timer */
	regmap_reg_range(0x1000a200, 0x1000a2ff), /* GPU timer */
	regmap_reg_range(0x1000a300, 0x1000a3ff), /* GPIO */
	regmap_reg_range(0x1000a400, 0x1000a5ff), /* EFUSE */
	regmap_reg_range(0x1000c000, 0x1000cfff), /* GDMP CSR */
	regmap_reg_range(0x10010000, 0x1001ffff), /* GDMP SRAM */
	regmap_reg_range(0x10200000, 0x10203fff), /* Switch - ARL Global */
	regmap_reg_range(0x10204000, 0x10207fff), /* Switch - BMU */
	regmap_reg_range(0x10208000, 0x1020bfff), /* Switch - ARL Port */
	regmap_reg_range(0x1020c000, 0x1020cfff), /* Switch - SCH */
	regmap_reg_range(0x10210000, 0x10213fff), /* Switch - MAC */
	regmap_reg_range(0x10214000, 0x10217fff), /* Switch - MIB */
	regmap_reg_range(0x10218000, 0x1021bfff), /* Switch - Port Control */
	regmap_reg_range(0x1021c000, 0x1021ffff), /* Switch - TOP */
	regmap_reg_range(0x10220000, 0x1022ffff), /* SerDes */
	regmap_reg_range(0x10286000, 0x10286fff), /* RG Batcher */
	regmap_reg_range(0x1028c000, 0x1028ffff), /* ETHER_SYS */
	regmap_reg_range(0x30000000, 0x37ffffff), /* I2C EEPROM */
	regmap_reg_range(0x38000000, 0x3fffffff), /* BOOT_ROM */
	regmap_reg_range(0xa0000000, 0xbfffffff), /* GPHY */
};

static const struct regmap_access_table an8855_readable_table = {
	.yes_ranges = an8855_readable_ranges,
	.n_yes_ranges = ARRAY_SIZE(an8855_readable_ranges),
};

static const struct regmap_config an8855_regmap_config = {
	.reg_bits = 32,
	.val_bits = 32,
	.reg_stride = 4,
	.max_register = 0xbfffffff,
	.reg_read = an8855_regmap_read,
	.reg_write = an8855_regmap_write,
	.reg_update_bits = an8855_regmap_update_bits,
	.disable_locking = true,
	.rd_table = &an8855_readable_table,
};

static int an8855_mfd_probe(struct mdio_device *mdiodev)
{
	struct an8855_mfd_priv *priv;
	struct regmap *regmap;

	priv = devm_kzalloc(&mdiodev->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	priv->bus = mdiodev->bus;
	priv->dev = &mdiodev->dev;
	priv->switch_addr = mdiodev->addr;
	/* no DMA for mdiobus, mute warning for DMA mask not set */
	priv->dev->dma_mask = &priv->dev->coherent_dma_mask;

	regmap = devm_regmap_init(priv->dev, NULL, priv,
				  &an8855_regmap_config);
	if (IS_ERR(regmap))
		dev_err_probe(priv->dev, PTR_ERR(priv->dev),
			      "regmap initialization failed\n");

	dev_set_drvdata(&mdiodev->dev, priv);

	return devm_mfd_add_devices(priv->dev, PLATFORM_DEVID_AUTO, an8855_mfd_devs,
				    ARRAY_SIZE(an8855_mfd_devs), NULL, 0,
				    NULL);
}

static const struct of_device_id an8855_mfd_of_match[] = {
	{ .compatible = "airoha,an8855-mfd" },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, an8855_mfd_of_match);

static struct mdio_driver an8855_mfd_driver = {
	.probe = an8855_mfd_probe,
	.mdiodrv.driver = {
		.name = "an8855",
		.of_match_table = an8855_mfd_of_match,
	},
};
mdio_module_driver(an8855_mfd_driver);

MODULE_AUTHOR("Christian Marangi <ansuelsmth@gmail.com>");
MODULE_DESCRIPTION("Driver for Airoha AN8855 MFD");
MODULE_LICENSE("GPL");
