// SPDX-License-Identifier: GPL-2.0-or-later

#include <linux/debugfs.h>
#include <linux/mfd/core.h>
#include <linux/mfd/syscon.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_mdio.h>
#include <linux/mod_devicetable.h>
#include <linux/phy.h>
#include <linux/platform_device.h>
#include <linux/regmap.h>

#define RTSDS_REG_CNT			32
#define RTSDS_VAL_MASK			GENMASK(15, 0)
#define RTSDS_SUBPAGE(page)		(page % 64)

#define RTSDS_MMD_PAGE_MASK		GENMASK(15, 8)
#define RTSDS_MMD_REG_MASK		GENMASK(7, 0)

#define RTSDS_838X_SDS_CNT		6
#define RTSDS_838X_PAGE_CNT		4
#define RTSDS_838X_BASE			0xe780

#define RTSDS_839X_SDS_CNT		14
#define RTSDS_839X_PAGE_CNT		64
#define RTSDS_839X_BASE			0xa000

#define RTSDS_930X_SDS_CNT		12
#define RTSDS_930X_PAGE_CNT		64
#define RTSDS_930X_BASE			0x03b0

#define RTSDS_931X_SDS_CNT		14
#define RTSDS_931X_PAGE_CNT		192
#define RTSDS_931X_BASE			0x5638

#define RTSDS_93XX_CMD_READ		0
#define RTSDS_93XX_CMD_WRITE		BIT(1)
#define RTSDS_93XX_CMD_BUSY		BIT(0)
#define RTSDS_93XX_CMD_SDS_MASK		GENMASK(6, 2)
#define RTSDS_93XX_CMD_PAGE_MASK	GENMASK(12, 7)
#define RTSDS_93XX_CMD_REG_MASK		GENMASK(17, 13)

struct rtsds_ctrl {
	struct device *dev;
	struct regmap *map;
	struct mii_bus *bus;
	const struct rtsds_config *cfg;
};

struct rtsds_config {
	int sds_cnt;
	int page_cnt;
	int base;
	int (*get_backing_sds)(int sds, int page);
	int (*read)(struct rtsds_ctrl *ctrl, int sds, int page, int regnum);
	int (*write)(struct rtsds_ctrl *ctrl, int sds, int page, int regnum, u16 value);
};

static bool rtsds_mmd_to_sds(struct rtsds_ctrl *ctrl, int addr, int devad, int mmd_regnum,
			     int *sds_page, int *sds_regnum)
{
	*sds_page = FIELD_GET(RTSDS_MMD_PAGE_MASK, mmd_regnum);
	*sds_regnum = FIELD_GET(RTSDS_MMD_REG_MASK, mmd_regnum);

	return !(addr < 0 || addr >= ctrl->cfg->sds_cnt ||
		 *sds_page < 0 || *sds_page >= ctrl->cfg->page_cnt ||
		 *sds_regnum < 0 || *sds_regnum >= RTSDS_REG_CNT ||
		 devad != MDIO_MMD_VEND1);
}

#ifdef CONFIG_DEBUG_FS

/*
 * The SerDes offer a lot of magic that sill needs to be uncovered. Getting this info manually
 * with mdio command line tools can be time consuming. Provide a convenient register dump.
 */

#define RTSDS_DBG_PAGE_NAMES	0x36
#define RTSDS_DBG_ROOT_DIR	"realtek_otto_serdes"

struct rtsds_debug_info {
	struct rtsds_ctrl *ctrl;
	int sds;
};

static const char * const rtsds_page_name[RTSDS_DBG_PAGE_NAMES] = {
	[0x00] = "SDS",		[0x01] = "SDS_EXT",
	[0x02] = "FIB",		[0x03] = "FIB_EXT",
	[0x04] = "TGR_STD_0",	[0x05] = "TGR_STD_1",
	[0x06] = "TGR_PRO_0",	[0x07] = "TGR_PRO_1",
	[0x08] = "TGX_STD_0",	[0x09] = "TGX_STD_1",
	[0x0a] = "TGX_PRO_0",	[0x0b] = "TGX_PRO_1",
	[0x1f] = "WDIG",
	[0x20] = "ANA_MISC",	[0x21] = "ANA_COM",
	[0x22] = "ANA_SPD",	[0x23] = "ANA_SPD_EXT",
	[0x24] = "ANA_1G2",	[0x25] = "ANA_1G2_EXT",
	[0x26] = "ANA_2G5",	[0x27] = "ANA_2G5_EXT",
	[0x28] = "ANA_3G1",	[0x29] = "ANA_3G1_EXT",
	[0x2a] = "ANA_5G0",	[0x2b] = "ANA_5G0_EXT",
	[0x2c] = "ANA_6G2",	[0x2d] = "ANA_6G2_EXT",
	[0x2e] = "ANA_10G",	[0x2f] = "ANA_10G_EXT",
	[0x30] = "GPON_SP",	[0x31] = "GPON_SP_EXT",
	[0x32] = "EPON_SP",	[0x33] = "EPON_SP_EXT",
	[0x34] = "ANA_6G0",	[0x35] = "ANA_6G0_EXT",
};

static int rtsds_sds_to_mmd(int sds_page, int sds_regnum)
{
	return FIELD_PREP(RTSDS_MMD_PAGE_MASK, sds_page) |
	       FIELD_PREP(RTSDS_MMD_REG_MASK, sds_regnum);
}

static int rtsds_dbg_registers_show(struct seq_file *seqf, void *unused)
{
	struct rtsds_debug_info *dbg_info = seqf->private;
	struct rtsds_ctrl *ctrl = dbg_info->ctrl;
	struct mii_bus *bus = ctrl->bus;
	int sds = dbg_info->sds;
	int regnum, page = 0;
	int subpage;

	do {
		subpage = RTSDS_SUBPAGE(page);
		if (!subpage) {
			seq_printf(seqf, "Back SDS %02d:", ctrl->cfg->get_backing_sds(sds, page));
			for (regnum = 0; regnum < RTSDS_REG_CNT; regnum++)
				seq_printf(seqf, "   %02X", regnum);
			seq_puts(seqf, "\n");
		}

		if (subpage < RTSDS_DBG_PAGE_NAMES && rtsds_page_name[subpage])
			seq_printf(seqf, "%*s: ", -12, rtsds_page_name[subpage]);
		else
			seq_printf(seqf, "PAGE %02X     : ", page);

		for (regnum = 0; regnum < RTSDS_REG_CNT; regnum++)
			seq_printf(seqf, "%04X ",
				   mdiobus_c45_read(bus, sds, MDIO_MMD_VEND1,
						    rtsds_sds_to_mmd(page, regnum)));
		seq_puts(seqf, "\n");
	} while (++page < ctrl->cfg->page_cnt);

	return 0;
}
DEFINE_SHOW_ATTRIBUTE(rtsds_dbg_registers);

static int rtsds_debug_init(struct rtsds_ctrl *ctrl)
{
	struct rtsds_debug_info *dbg_info;
	struct dentry *dir, *root;
	char dirname[32];

	root = debugfs_create_dir(RTSDS_DBG_ROOT_DIR, NULL);
	if (IS_ERR(root))
		return PTR_ERR(root);

	for (int sds = 0; sds < ctrl->cfg->sds_cnt; sds++) {
		dbg_info = devm_kzalloc(ctrl->dev, sizeof(*dbg_info), GFP_KERNEL);
		if (!dbg_info)
			return -ENOMEM;

		dbg_info->ctrl = ctrl;
		dbg_info->sds = sds;

		snprintf(dirname, sizeof(dirname), "serdes.%d", sds);
		dir = debugfs_create_dir(dirname, root);

		debugfs_create_file("registers", 0600, dir, dbg_info,
				    &rtsds_dbg_registers_fops);
	}

	return 0;
}

#endif /* CONFIG_DEBUG_FS */

/*
 * The RTL838x has 6 SerDes. The 16 bit registers start at 0xbb00e780 and are mapped directly into
 * 32 bit memory addresses. High 16 bits are always empty. A "lower" memory block serves pages 0/3
 * a "higher" memory block pages 1/2.
 */

static int rtsds_838x_reg_offset(int sds, int page, int regnum)
{
	if (page == 0 || page == 3)
		return (sds << 9) + (page << 7) + (regnum << 2);

	/* (page == 1 || page == 2) */
	return 0xb80 + (sds << 8) + (page << 7) + (regnum << 2);
}

static int rtsds_838x_read(struct rtsds_ctrl *ctrl, int sds, int page, int regnum)
{
	int offset = rtsds_838x_reg_offset(sds, page, regnum);
	int ret, value;

	ret = regmap_read(ctrl->map, ctrl->cfg->base + offset, &value);

	return ret ? ret : value & RTSDS_VAL_MASK;
}

static int rtsds_838x_write(struct rtsds_ctrl *ctrl, int sds, int page, int regnum, u16 value)
{
	int offset = rtsds_838x_reg_offset(sds, page, regnum);

	return regmap_write(ctrl->map, ctrl->cfg->base + offset, value);
}

/*
 * The RTL839x has 14 SerDes starting at 0xbb00a000. 0-7, 10, 11 are 5GBit, 8, 9, 12, 13 are
 * 10 GBit. Two adjacent SerDes are tightly coupled and share a 1024 bytes register area. Per 32
 * bit address two registers are stored. The first register is stored in the lower 2 bytes ("on
 * the right" due to big endian) and the second register in the upper 2 bytes. The following
 * register areas are known:
 *
 * - XSG0	(4 pages @ offset 0x000): for even SerDes
 * - XSG1	(4 pages @ offset 0x100): for odd SerDes
 * - TGRX	(4 pages @ offset 0x200): for even 10G SerDes
 * - ANA_RG	(2 pages @ offset 0x300): for even 5G SerDes
 * - ANA_RG	(2 pages @ offset 0x380): for odd 5G SerDes
 * - ANA_TG	(2 pages @ offset 0x300): for even 10G SerDes
 * - ANA_TG	(2 pages @ offset 0x380): for odd 10G SerDes
 *
 * The most consistent mapping that aligns to the RTL93xx devices is:
 *
 * RTL93xx page		even 5G SerDes	odd 5G SerDes	even 10G SerDes	odd 10G SerDes
 * 0x00 (SDS)		XSG0/0		XSG1/0		XSG0/0		XSG1/0
 * 0x01 (SDS_EXT)	XSG0/1		XSG1/1		XSG0/1		XSG1/1
 * 0x02 (FIB)		XSG0/2		XSG1/2		XSG0/2		XSG1/2
 * 0x03 (FIB_EXT)	XSG0/3		XSG1/3		XSG0/3		XSG1/3
 * ...			<zero>		<zero>		<zero>		<zero>
 * 0x06 (TGR_PRO_0)	<zero>		<zero>		TGRX/2		<zero>
 * 0x07 (TGR_PRO_1)	<zero>		<zero>		TGRX/3		<zero>
 * 0x08 (TGX_STD_0)	<zero>		<zero>		TGRX/0		<zero>
 * 0x09 (TGX_STD_1)	<zero>		<zero>		TGRX/1		<zero>
 * ...			<zero>		<zero>		<zero>		<zero>
 * 0x24 (ANA_1G2)	ANA_RG		ANA_RG		<zero>		<zero>
 * 0x25 (ANA_1G2_EXT)	ANA_RG_EXT	ANA_RG_EXT	<zero>		<zero>
 * ...			<zero>		<zero>		<zero>		<zero>
 * 0x2e (ANA_10G)	<zero>		<zero>		ANA_TG		ANA_TG
 * 0x2f (ANA_10G_EXT)	<zero>		<zero>		ANA_TG_EXT	ANA_TG_EXT
 */

static int rtsds_839x_reg_offset(int sds, int page, int regnum)
{
	int offset = ((sds & 0xfe) << 9) + ((regnum & 0xfe) << 1);
	int sds5g = (GENMASK(11, 10) | GENMASK(7, 0)) & BIT(sds);

	if (page < 0x4)
		return offset + (page << 6) + ((sds & 1) << 8);
	else if (page >= 0x6 && page <= 0x7 && (sds == 8 || sds == 12))
		return offset + (page << 6) + 0x100;
	else if (page >= 0x8 && page <= 0x9 && (sds == 8 || sds == 12))
		return offset + ((page - 0x2) << 6) + 0x80;
	else if (page >= 0x24 && page <= 0x25 && sds5g)
		return offset + ((page - 0x1c) << 6) + 0x100 + ((sds & 1) << 7);
	else if (page >= 0x2e && page <= 0x2f && !sds5g)
		return offset + ((page - 0x24) << 6) + 0x80 + ((sds & 1) << 7);

	return -EINVAL; /* hole */
}

static int rtsds_839x_read(struct rtsds_ctrl *ctrl, int sds, int page, int regnum)
{
	int offset = rtsds_839x_reg_offset(sds, page, regnum);
	int shift = regnum & 1 ? 16 : 0;
	int ret, value;

	if (offset < 0)
		return 0;

	ret = regmap_read(ctrl->map, ctrl->cfg->base + offset, &value);

	return ret ? ret : (value >> shift) & RTSDS_VAL_MASK;
}

static int rtsds_839x_write(struct rtsds_ctrl *ctrl, int sds, int page, int regnum, u16 value)
{
	int offset = rtsds_839x_reg_offset(sds, page, regnum);
	int write_value = value;
	int neigh_value;

	if (offset < 0)
		return 0;

	neigh_value = rtsds_839x_read(ctrl, sds, page, regnum ^ 1);
	if (neigh_value < 0)
		return neigh_value;

	if (regnum & 1)
		write_value = (write_value << 16) + neigh_value;
	else
		write_value = (neigh_value << 16) + write_value;

	return regmap_write(ctrl->map, ctrl->cfg->base + offset, write_value);
}

static int rtsds_83xx_get_backing_sds(int sds, int page)
{
	return sds;
}

static int rtsds_rt93xx_io(struct rtsds_ctrl *ctrl, int sds, int page, int regnum, int cmd)
{
	int ret, op, value;

	op = FIELD_PREP(RTSDS_93XX_CMD_SDS_MASK, sds) |
	     FIELD_PREP(RTSDS_93XX_CMD_PAGE_MASK, page) |
	     FIELD_PREP(RTSDS_93XX_CMD_REG_MASK, regnum) |
	     RTSDS_93XX_CMD_BUSY | cmd;

	regmap_write(ctrl->map, ctrl->cfg->base, op);
	ret = regmap_read_poll_timeout(ctrl->map, ctrl->cfg->base, value,
				       !(value & RTSDS_93XX_CMD_BUSY), 30, 1000000);

	if (ret < 0) {
		dev_err(ctrl->dev, "SerDes I/O timed out\n");
		return -ETIMEDOUT;
	}

	return 0;
}

/*
 * RTL93xx targets use a shared implementation. Their SerDes data is accessed through two IO
 * registers which simulate commands to an internal MDIO bus.
 *
 * The RTL930x family has 12 SerDes of three types.
 *
 * - SerDes 0-1 exist on the RTL9301 and 9302B and are QSGMII capable
 * - SerDes 2-9 are USXGMII capabable with either quad or single configuration
 * - SerDes 10-11 are 10GBase-R capable
 */
static int rtsds_930x_get_backing_sds(int sds, int page)
{
	return sds;
}

/*
 * The RTL931x family has 14 "frontend" SerDes that are cascaded. All operations (e.g. reset) work
 * on this frontend view while their registers are distributed over a total of least 26 background
 * SerDes with 64 pages and 32 registers. Three types of SerDes exist:
 *
 * - Serdes 0,1 are "simple" and work on one background serdes.
 * - "Even" SerDes with numbers 2, 4, 6, 8, 10, 12 work on two background SerDes. One analog and
 *   one digital.
 * - "Odd" SerDes with numbers 3, 5, 7, 9, 11, 13 work on a total of 3 background SerDes (one
 *   analog and two digital)
 *
 * This maps to:
 *
 * Frontend SerDes  |  0  1  2  3  4  5  6  7  8  9 10 11 12 13
 * -----------------+------------------------------------------
 * Backend SerDes 1 |  0  1  2  3  6  7 10 11 14 15 18 19 22 23
 * Backend SerDes 2 |  0  1  2  4  6  8 10 12 14 16 18 20 22 24
 * Backend SerDes 3 |  0  1  3  5  7  9 11 13 15 17 19 21 23 25
 *
 * Note: In Realtek proprietary XSGMII mode (10G pumped SGMII) the frontend SerDes works on the
 * two digital SerDes while in all other modes it works on the analog and the first digital
 * SerDes. Overlapping (e.g. backend SerDes 7 can be analog or digital 2) is avoided by the
 * existing hardware designs.
 *
 * Align this for readability by simulating a total of 192 pages and mix them as follows.
 *
 * frontend page		"even" frontend SerDes		"odd" frontend SerDes
 * page 0x00-0x3f (analog):	page 0x00-0x3f back SDS		page 0x00-0x3f back SDS
 * page 0x40-0x7f (digi 1):	page 0x00-0x3f back SDS		page 0x00-0x3f back SDS+1
 * page 0x80-0xbf (digi 2):	page 0x00-0x3f back SDS+1	page 0x00-0x3f back SDS+2
 */
static int rtsds_931x_get_backing_sds(int sds, int page)
{
	int map[] = { 0, 1, 2, 3, 6, 7, 10, 11, 14, 15, 18, 19, 22, 23 };
	int backsds;

	/* First two RTL931x SerDes have 1:1 frontend/backend mapping */
	if (sds < 2)
		return sds;

	backsds = map[sds];
	if (sds & 1)
		backsds += (page >> 6); /* distribute "odd" to 3 background SerDes */
	else
		backsds += (page >> 7); /* distribute "even" to 2 background SerDes */

	return backsds;
}

static int rtsds_93xx_read(struct rtsds_ctrl *ctrl, int sds, int page, int regnum)
{
	int subpage = RTSDS_SUBPAGE(page);
	int ret, backsds, value;

	backsds = ctrl->cfg->get_backing_sds(sds, page);
	ret = rtsds_rt93xx_io(ctrl, backsds, subpage, regnum, RTSDS_93XX_CMD_READ);
	if (ret)
		return ret;

	ret = regmap_read(ctrl->map, ctrl->cfg->base + 4, &value);

	return ret ? ret : value & RTSDS_VAL_MASK;
}

static int rtsds_93xx_write(struct rtsds_ctrl *ctrl, int sds, int page, int regnum, u16 value)
{
	int subpage = RTSDS_SUBPAGE(page);
	int ret, backsds;

	backsds = ctrl->cfg->get_backing_sds(sds, page);
	ret = regmap_write(ctrl->map, ctrl->cfg->base + 4, value);
	if (ret)
		return ret;

	return rtsds_rt93xx_io(ctrl, backsds, subpage, regnum, RTSDS_93XX_CMD_WRITE);
}

static int rtsds_read(struct mii_bus *bus, int addr, int devad, int regnum)
{
	struct rtsds_ctrl *ctrl = bus->priv;
	int sds_page, sds_regnum;

	if (!rtsds_mmd_to_sds(ctrl, addr, devad, regnum, &sds_page, &sds_regnum))
		return -EINVAL;

	return ctrl->cfg->read(ctrl, addr, sds_page, sds_regnum);
}

static int rtsds_write(struct mii_bus *bus, int addr, int devad, int regnum, u16 value)
{
	struct rtsds_ctrl *ctrl = bus->priv;
	int sds_page, sds_regnum;

	if (!rtsds_mmd_to_sds(ctrl, addr, devad, regnum, &sds_page, &sds_regnum))
		return -EINVAL;

	return ctrl->cfg->write(ctrl, addr, sds_page, sds_regnum, value);
}

static int rtsds_probe(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	struct device *dev = &pdev->dev;
	struct rtsds_ctrl *ctrl;
	struct mii_bus *bus;
	int ret;

	bus = devm_mdiobus_alloc_size(&pdev->dev, sizeof(*ctrl));
	if (!bus)
		return -ENOMEM;

	ctrl = bus->priv;
	ctrl->map = syscon_node_to_regmap(np->parent);
	if (IS_ERR(ctrl->map))
		return PTR_ERR(ctrl->map);

	ctrl->dev = dev;
	ctrl->cfg = (const struct rtsds_config *)device_get_match_data(ctrl->dev);
	ctrl->bus = bus;

	snprintf(bus->id, MII_BUS_ID_SIZE, "realtek-serdes-mdio");
	bus->name = "Realtek SerDes MDIO bus";
	bus->parent = dev;
	bus->read_c45 = rtsds_read;
	bus->write_c45 = rtsds_write;
	bus->phy_mask = ~0;

	ret = devm_of_mdiobus_register(dev, bus, dev->of_node);
	if (ret)
		return ret;

#ifdef CONFIG_DEBUG_FS
	rtsds_debug_init(ctrl);
#endif

	dev_info(dev, "Realtek SerDes mdio bus initialized, %d SerDes, %d pages, %d registers\n",
		 ctrl->cfg->sds_cnt, ctrl->cfg->page_cnt, RTSDS_REG_CNT);

	return 0;
}

static const struct rtsds_config rtsds_838x_cfg = {
	.sds_cnt		= RTSDS_838X_SDS_CNT,
	.page_cnt		= RTSDS_838X_PAGE_CNT,
	.base			= RTSDS_838X_BASE,
	.get_backing_sds	= rtsds_83xx_get_backing_sds,
	.read			= rtsds_838x_read,
	.write			= rtsds_838x_write,
};

static const struct rtsds_config rtsds_839x_cfg = {
	.sds_cnt		= RTSDS_839X_SDS_CNT,
	.page_cnt		= RTSDS_839X_PAGE_CNT,
	.base			= RTSDS_839X_BASE,
	.get_backing_sds	= rtsds_83xx_get_backing_sds,
	.read			= rtsds_839x_read,
	.write			= rtsds_839x_write,
};

static const struct rtsds_config rtsds_930x_cfg = {
	.sds_cnt		= RTSDS_930X_SDS_CNT,
	.page_cnt		= RTSDS_930X_PAGE_CNT,
	.base			= RTSDS_930X_BASE,
	.get_backing_sds	= rtsds_930x_get_backing_sds,
	.read			= rtsds_93xx_read,
	.write			= rtsds_93xx_write,
};

static const struct rtsds_config rtsds_931x_cfg = {
	.sds_cnt		= RTSDS_931X_SDS_CNT,
	.page_cnt		= RTSDS_931X_PAGE_CNT,
	.base			= RTSDS_931X_BASE,
	.get_backing_sds	= rtsds_931x_get_backing_sds,
	.read			= rtsds_93xx_read,
	.write			= rtsds_93xx_write,
};

static const struct of_device_id rtsds_of_match[] = {
	{
		.compatible = "realtek,rtl8380-serdes-mdio",
		.data = &rtsds_838x_cfg,
	},
	{
		.compatible = "realtek,rtl8392-serdes-mdio",
		.data = &rtsds_839x_cfg,
	},
	{
		.compatible = "realtek,rtl9301-serdes-mdio",
		.data = &rtsds_930x_cfg,
	},
	{
		.compatible = "realtek,rtl9311-serdes-mdio",
		.data = &rtsds_931x_cfg,
	},
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, rtsds_of_match);

static struct platform_driver rtsds_mdio_driver = {
	.driver = {
		.name = "realtek-otto-serdes-mdio",
		.of_match_table = rtsds_of_match
	},
	.probe = rtsds_probe,
};
module_platform_driver(rtsds_mdio_driver);

MODULE_AUTHOR("Markus Stockhausen <markus.stockhausen@gmx.de>");
MODULE_DESCRIPTION("Realtek Otto SerDes MDIO bus");
MODULE_LICENSE("GPL v2");
