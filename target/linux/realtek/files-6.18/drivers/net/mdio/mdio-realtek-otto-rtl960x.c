// SPDX-License-Identifier: GPL-2.0-only

#include <linux/mfd/syscon.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_mdio.h>
#include <linux/phy.h>
#include <linux/platform_device.h>
#include <linux/regmap.h>
#include <linux/types.h>

#define RTMD_MAX_PORTS				4
#define RTMD_PAGE_SELECT			0x1f
#define RTMD_RAW_PAGE(p)			((p) - 1)
#define RTMD_960X_NUM_INT_PAGES			4096

/* MDIO bus registers/fields */
#define RTMD_960X_GPHY_IND_WD			0x0
#define RTMD_960X_GPHY_IND_CMD			0x4
#define   RTMD_960X_CMD_READ_C22		0
#define   RTMD_960X_CMD_WRITE_C22		BIT(22)
#define   RTMD_960X_CMD_RUN			BIT(21)
#define   RTMD_960X_C22_DATA(pn, addr)		((pn) << 16 | (addr))
#define RTMD_960X_GPHY_IND_RD			0x8
#define   RTMD_960X_GPHY_BUSY			BIT(16)
#define   RTMD_960X_GPHY_READ_MASK		GENMASK(15, 0)
#define RTMD_960X_WRAP_GPHY_MISC		0x114
#define   RTMD_960X_PHY_PATCH_DONE		BIT(0)
#define RTMD_960X_OCP_PHY_BASE			0xa400

struct rtmd_port {
	int page;
	bool raw;
};

struct rtmd_ctrl {
	struct mutex lock;
	struct regmap *map;
	struct rtmd_port port[RTMD_MAX_PORTS];
};

static int rtmd_960x_to_ocp_addr(u32 page, u32 reg, u32 *addr)
{
	/* Translate phy reg to a PHY OCP address
	 *
	 * The phy reg 31 is reserved for page select
	 * The phy regs 16~23 are for paged access to OCP addresses
	 * The phy regs 0~15 and 24~30 are on base 0xa400 ocp address
	 *
	 * With phy registers 16~23, a user can realistically reach any
	 * PHY OCP address, including the ones on base 0xa400.
	 *
	 * Masking page to 12 bits is presumably because it is the max page
	 * of the mdio bus. Since PHY OCP Address is only 15 bits long, the
	 * registers over 7 (exlucing 16-23 because they are substacted by 16)
	 * overlap with page bits because they essentially shifted by 1 and
	 * that is very strange. But that doesn't matter because all 0~15/24~30
	 * phy registers are fixed to a standard phy ocp base of 0xa400 which
	 * seems to be typical to realtek chips, like rtl8365mb, so there is
	 * precedent in upstream linux for that.
	 */
	if (reg > 30)
		return -EINVAL;
	if (reg > 15 && reg < 24)
		*addr = ((page & 0xfff) << 4) + (reg - 16) * 2;
	else
		*addr = RTMD_960X_OCP_PHY_BASE + reg * 2;

	return 0;
}

static int rtmd_960x_internal_run_cmd(struct mii_bus *bus, u32 pn, u32 page, u32 reg, u32 cmd)
{
	struct rtmd_ctrl *ctrl = bus->priv;
	u32 ocp_addr, c22_adr, val;
	int ret;

	ret = rtmd_960x_to_ocp_addr(page, reg, &ocp_addr);
	if (ret)
		return ret;

	c22_adr = RTMD_960X_C22_DATA(pn, ocp_addr);

	ret = regmap_write(ctrl->map, RTMD_960X_GPHY_IND_CMD, cmd | RTMD_960X_CMD_RUN | c22_adr);
	if (ret)
		return ret;

	ret = regmap_read_poll_timeout(ctrl->map, RTMD_960X_GPHY_IND_RD,
				       val, !(val & RTMD_960X_GPHY_BUSY), 20, 500000);
	if (ret)
		dev_warn_once(&bus->dev, "access timed out\n");

	return ret;
}

static int rtmd_960x_write_int_c22(struct mii_bus *bus, u32 pn, u32 page, u32 reg, u32 val)
{
	struct rtmd_ctrl *ctrl = bus->priv;
	int ret;

	ret = regmap_write(ctrl->map, RTMD_960X_GPHY_IND_WD, val);
	if (ret)
		return ret;

	return rtmd_960x_internal_run_cmd(bus, pn, page, reg, RTMD_960X_CMD_WRITE_C22);
}

static int rtmd_960x_read_int_c22(struct mii_bus *bus, u32 pn, u32 page, u32 reg, u32 *val)
{
	struct rtmd_ctrl *ctrl = bus->priv;
	int ret;

	ret = rtmd_960x_internal_run_cmd(bus, pn, page, reg, RTMD_960X_CMD_READ_C22);
	if (ret)
		return ret;

	ret = regmap_read(ctrl->map, RTMD_960X_GPHY_IND_RD, val);
	if (ret)
		return ret;

	*val &= RTMD_960X_GPHY_READ_MASK;

	return ret;
}

static int rtmd_read_c22(struct mii_bus *bus, int phy, int regnum)
{
	struct rtmd_ctrl *ctrl = bus->priv;
	int ret, page, val = 0;

	if (phy >= RTMD_MAX_PORTS)
		return 0xffff;

	scoped_guard(mutex, &ctrl->lock) {
		page = ctrl->port[phy].page;
		if (regnum == RTMD_PAGE_SELECT &&
		    page != RTMD_RAW_PAGE(RTMD_960X_NUM_INT_PAGES))
			return page;

		ctrl->port[phy].raw = (page == RTMD_RAW_PAGE(RTMD_960X_NUM_INT_PAGES));
		ret = rtmd_960x_read_int_c22(bus, phy, page, regnum, &val);
	}

	dev_dbg(&bus->dev, "rd_PHY(phy=0x%02x, pag=0x%04x, reg=0x%04x) = 0x%04x, ret = %d\n",
		phy, page, regnum, val, ret);

	return ret ? ret : val;
}

static int rtmd_write_c22(struct mii_bus *bus, int phy, int regnum, u16 val)
{
	struct rtmd_ctrl *ctrl = bus->priv;
	bool do_write = false;
	int ret = 0, page;

	if (phy >= RTMD_MAX_PORTS)
		return -ENODEV;

	scoped_guard(mutex, &ctrl->lock) {
		page = ctrl->port[phy].page;
		if (regnum == RTMD_PAGE_SELECT)
			ctrl->port[phy].page = val;

		do_write = !ctrl->port[phy].raw &&
			   (regnum != RTMD_PAGE_SELECT ||
			    page == RTMD_RAW_PAGE(RTMD_960X_NUM_INT_PAGES));

		if (do_write) {
			ctrl->port[phy].raw = (page == RTMD_RAW_PAGE(RTMD_960X_NUM_INT_PAGES));
			ret = rtmd_960x_write_int_c22(bus, phy, page, regnum, val);
		} else {
			ctrl->port[phy].raw = false;
		}
	}

	if (do_write)
		dev_dbg(&bus->dev,
			"wr_PHY(phy=0x%02x, pag=0x%04x, reg=0x%04x, val=0x%04x), ret = %d\n",
			phy, page, regnum, val, ret);

	return ret;
}

static int rtmd_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct rtmd_ctrl *ctrl;
	struct mii_bus *bus;
	int ret;

	bus = devm_mdiobus_alloc_size(dev, sizeof(*ctrl));
	if (!bus)
		return -ENOMEM;

	ctrl = bus->priv;

	ret = devm_mutex_init(dev, &ctrl->lock);
	if (ret)
		return ret;

	ctrl->map = syscon_node_to_regmap(pdev->dev.of_node->parent);
	if (IS_ERR(ctrl->map))
		return PTR_ERR(ctrl->map);

	/*
	 * PHY_PATCH_DONE enables phy control via SoC. This is required for phy access,
	 * including patching. Must always be set before the phys are probed.
	 */
	ret = regmap_update_bits(ctrl->map, RTMD_960X_WRAP_GPHY_MISC,
				 RTMD_960X_PHY_PATCH_DONE, RTMD_960X_PHY_PATCH_DONE);
	if (ret)
		return ret;

	bus->name = "Realtek MDIO bus";
	bus->read = rtmd_read_c22;
	bus->write = rtmd_write_c22;
	bus->parent = dev;
	snprintf(bus->id, MII_BUS_ID_SIZE, "realtek-mdio-int");

	ret = devm_of_mdiobus_register(dev, bus, dev->of_node);
	if (ret)
		return dev_err_probe(dev, ret, "cannot register MDIO bus\n");

	platform_set_drvdata(pdev, bus);

	return 0;
}

static const struct of_device_id rtmd_ids[] = {
	{ .compatible = "realtek,rtl9607-int-mdio" },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, rtmd_ids);

static struct platform_driver rtmd_driver = {
	.probe = rtmd_probe,
	.driver = {
		.name = "mdio-rtl960x-otto",
		.of_match_table = rtmd_ids,
	},
};

module_platform_driver(rtmd_driver);

MODULE_DESCRIPTION("Realtek Otto RTL960X MDIO driver");
MODULE_LICENSE("GPL");
