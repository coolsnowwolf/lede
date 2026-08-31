// SPDX-License-Identifier: GPL-2.0-only

/*
 * On all Realtek switch platforms the hardware periodically reads the link status of all PHYs.
 * The result is automatically written into MAC specific registers that can be consumed by a
 * DSA driver.  This is to some degree programmable, so that one can tell the hardware to read
 * specific C22 registers from specific pages, or C45 registers, to determine the current link
 * speed, duplex, flow-control, ...
 *
 * This happens without any need for the driver to do anything at runtime, completely invisible
 * and in a parallel hardware thread, independent of the CPU running Linux. All one needs to do
 * is to set it up once. Having the MAC link settings automatically follow the PHY link status
 * also happens to be the only way to control MAC port status in a meaningful way, or at least
 * it's the only way that is fully understood, as this is what every vendor firmware is doing.
 *
 * The hardware PHY polling unit doesn't care about bus locking. It just assumes that all paged
 * PHY operations are also done via the same hardware unit offering this PHY access abstractions.
 *
 * Additionally the devices are known to have a so called raw mode. Using the special MAX_PAGE-1
 * with the MDIO controller found in Realtek SoCs allows to access the PHY in raw mode, i.e.
 * bypassing the cache and paging engine of the MDIO controller. E.g. for RTL838x this is 4095.
 *
 * On the other hand Realtek PHYs usually make use of select register 0x1f to switch pages. There
 * is no problem to issue separate page and access bus calls to the PHYs when they are not
 * attached to an Realtek SoC. The design should be to keep the PHY code bus independent.
 *
 * To bring all this together one needs a tricky bus design that intercepts select page calls but
 * lets raw page accesses through. And especially knows how to handle raw accesses to the select
 * register.
 *
 * While C45 clause handling is pretty standard the legacy functions basically track the accesses
 * and the state of the bus with the rtmd_port attributes of the control structure. The page
 * selection works as follows:
 *
 * phy_write(phydev, RTMD_PAGE_SELECT, 12)	: store internal page 12 in driver
 * phy_write(phydev, 7, 33)			: write page=12, reg=7, val=33
 *
 * Any Realtek PHY that is connected to this bus must simply provide the standard page functions:
 *
 * define RTL821X_PAGE_SELECT 0x1f
 *
 * static int rtl821x_read_page(struct phy_device *phydev)
 * {
 *   return __phy_read(phydev, RTL821X_PAGE_SELECT);
 * }
 *
 * static int rtl821x_write_page(struct phy_device *phydev, int page)
 * {
 *   return __phy_write(phydev, RTL821X_PAGE_SELECT, page);
 * }
 *
 * In case there are non Realtek PHYs attached to the bus the logic might need to be changed.
 * For now it should be sufficient.
 */

#include <linux/fwnode.h>
#include <linux/fwnode_mdio.h>
#include <linux/mfd/syscon.h>
#include <linux/of.h>
#include <linux/of_mdio.h>
#include <linux/phy.h>
#include <linux/platform_device.h>
#include <linux/regmap.h>
#include <linux/types.h>

#define RTMD_MAX_PORTS				57
#define RTMD_MAX_SMI_BUSES			4

#define RTMD_838X_NUM_BUSES			1
#define RTMD_838X_NUM_PAGES			4096
#define RTMD_838X_NUM_PORTS			28
#define RTMD_839X_NUM_BUSES			2
#define RTMD_839X_NUM_PAGES			8192
#define RTMD_839X_NUM_PORTS			52
#define RTMD_930X_NUM_BUSES			4
#define RTMD_930X_NUM_PAGES			4096
#define RTMD_930X_NUM_PORTS			28
#define RTMD_931X_NUM_BUSES			4
#define RTMD_931X_NUM_PAGES			8192
#define RTMD_931X_NUM_PORTS			56

#define RTMD_PAGE_SELECT			0x1f
#define RTMD_RAW_PAGE(p)			((p) - 1)

#define RTMD_PHY_AQR113C_A			0x31c31c12
#define RTMD_PHY_AQR113C_B			0x31c31c13
#define RTMD_PHY_AQR813				0x31c31cb2
#define RTMD_PHY_RTL8218D			0x001cc983
#define RTMD_PHY_RTL8218E			0x001cc984
#define RTMD_PHY_RTL8221B_VB_CG			0x001cc849
#define RTMD_PHY_RTL8221B_VM_CG			0x001cc84a
#define RTMD_PHY_RTL8224			0x001ccad0
#define RTMD_PHY_RTL8226			0x001cc838
#define RTMD_PHY_RTL8261			0x001ccaf3
#define RTMD_PHY_RTL8264			0x001ccaf2
#define RTMD_PHY_RTL8264B			0x001cc813

#define RTMD_PHY_MAC_1G				3
#define RTMD_PHY_MAC_2G_PLUS			1
#define RTMD_PHY_MAC_SDS			0

#define RTMD_PHY_POLL_MMD(dev, reg, bit)	((((bit) & GENMASK(3, 0)) << 21) | \
						 (((dev) & GENMASK(4, 0)) << 16) | \
						 ((reg) & GENMASK(15, 0)))

/* MDIO bus registers/fields */
#define RTMD_C45_DATA(devnum, regnum)		((((devnum) & GENMASK(4, 0)) << 16) | \
						  ((regnum) & GENMASK(15, 0)))
#define RTMD_DATA_IN_HI_OUT_LOW			16
#define RTMD_DATA_IN_LOW_OUT_HI			0
#define RTMD_RUN				BIT(0)

#define RTMD_838X_SMI_GLB_CTRL			0xa100
#define   RTMD_838X_SMI_GLB_PHY_MAN_24_27	BIT(7)
#define   RTMD_838X_SMI_GLB_PHY_PATCH_DONE	BIT(15)
#define RTMD_838X_SMI_ACCESS_PHY_CTRL_0		0xa1b8
#define RTMD_838X_SMI_ACCESS_PHY_CTRL_1		0xa1bc
#define   RTMD_838X_CMD_FAIL			0 /* No hardware support */
#define   RTMD_838X_CMD_READ_C22		0
#define   RTMD_838X_CMD_READ_C45		BIT(1)
#define   RTMD_838X_CMD_WRITE_C22		BIT(2)
#define   RTMD_838X_CMD_WRITE_C45		(BIT(1) | BIT(2))
#define   RTMD_838X_C22_DATA(page, reg)		((reg) << 20 | RTMD_PAGE_SELECT << 15 | (page) << 3)
#define RTMD_838X_SMI_ACCESS_PHY_CTRL_2		0xa1c0
#define RTMD_838X_SMI_ACCESS_PHY_CTRL_3		0xa1c4
#define RTMD_838X_SMI_POLL_CTRL			0xa17c
#define RTMD_838X_SMI_PORT0_5_ADDR_CTRL		0xa1c8

#define RTMD_839X_BCAST_PHYID_CTRL		0x03ec
#define RTMD_839X_PHYREG_ACCESS_CTRL		0x03dc
#define   RTMD_839X_CMD_FAIL			BIT(1)
#define   RTMD_839X_CMD_READ_C22		0
#define   RTMD_839X_CMD_READ_C45		BIT(2)
#define   RTMD_839X_CMD_WRITE_C22		BIT(3)
#define   RTMD_839X_CMD_WRITE_C45		(BIT(2) | BIT(3))
#define   RTMD_839X_C22_DATA(page, reg)		((reg) << 5 | (page) << 10)
#define RTMD_839X_PHYREG_CTRL			(0x03e0)
#define   RTMD_839X_PHYREG_SKIP_EXT_PAGE	GENMASK(8, 0)
#define RTMD_839X_PHYREG_DATA_CTRL		0x03f0
#define RTMD_839X_PHYREG_MMD_CTRL		0x03f4
#define RTMD_839X_PHYREG_PORT_CTRL(x)		(0x03e4 + (x) * 4)
#define RTMD_839X_SMI_PORT_POLLING_CTRL		0x03fc
#define RTMD_839X_SMI_GLB_CTRL			0x03f8
#define   RTMD_839X_SMI_GLB_MDX_POLLING_EN	BIT(7)

#define RTMD_930X_SMI_GLB_CTRL			0xca00
#define   RTMD_930X_SMI_GLB_INTF_SEL(bus)	BIT(16 + (bus))
#define   RTMD_930X_SMI_GLB_POLL_SEL(bus)	BIT(20 + (bus))
#define RTMD_930X_SMI_ACCESS_PHY_CTRL_0		0xcb70
#define RTMD_930X_SMI_ACCESS_PHY_CTRL_1		0xcb74
#define   RTMD_930X_CMD_FAIL			BIT(25)
#define   RTMD_930X_CMD_READ_C22		0
#define   RTMD_930X_CMD_READ_C45		BIT(1)
#define   RTMD_930X_CMD_WRITE_C22		BIT(2)
#define   RTMD_930X_CMD_WRITE_C45		(BIT(1) | BIT(2))
#define   RTMD_930X_C22_DATA(page, reg)		((reg) << 20 | RTMD_PAGE_SELECT << 15 | (page) << 3)
#define RTMD_930X_SMI_ACCESS_PHY_CTRL_2		0xcb78
#define RTMD_930X_SMI_ACCESS_PHY_CTRL_3		0xcb7c
#define RTMD_930X_SMI_PORT0_15_POLLING_SEL	0xca08
#define RTMD_930X_SMI_PORT16_27_POLLING_SEL	0xca0c
#define RTMD_930X_SMI_MAC_TYPE_CTRL		0xca04
#define   RTMD_930X_SMI_MAC_TYPE_P0_23(pn)	(GENMASK(1, 0) << (((pn) / 4) * 2))
#define   RTMD_930X_SMI_MAC_TYPE_P24_27(pn)	(GENMASK(2, 0) << (((pn) - 24) * 3 + 12))
#define RTMD_930X_SMI_POLL_CTRL			0xca90
#define RTMD_930X_SMI_PRVTE_POLLING_CTRL	0xca10
#define RTMD_930X_SMI_10G_POLLING_REG0_CFG	0xcbb4
#define RTMD_930X_SMI_10G_POLLING_REG9_CFG	0xcbb8
#define RTMD_930X_SMI_10G_POLLING_REG10_CFG	0xcbbc
#define RTMD_930X_SMI_PORT0_5_ADDR_CTRL	0xcb80

#define RTMD_931X_SMI_PORT_POLLING_CTRL		0x0ccc
#define RTMD_931X_SMI_INDRT_ACCESS_BC		0x0c14
#define   RTMD_931X_SMI_INDRT_PORT(pn)		((pn) << 5)
#define RTMD_931X_SMI_GLB_CTRL0			0x0cc0
#define   RTMD_931X_SMI_GLB_PRVTE0_POLL(bus)	BIT(20 + (bus))
#define   RTMD_931X_SMI_GLB_PRVTE1_POLL(bus)	BIT(24 + (bus))
#define RTMD_931X_SMI_GLB_CTRL1			0x0cbc
#define   RTMD_931X_SMI_GLB_FMT_SEL_C45(bus)	BIT((bus) * 2 + 1)
#define   RTMD_931X_SMI_GLB_FMT_SEL_FRC(bus)	BIT((bus) * 2)
#define RTMD_931X_SMI_INDRT_ACCESS_CTRL_0	0x0c00
#define   RTMD_931X_CMD_FAIL			BIT(1)
#define   RTMD_931X_CMD_READ_C22		0
#define   RTMD_931X_CMD_READ_C45		BIT(3)
#define   RTMD_931X_CMD_WRITE_C22		BIT(4)
#define   RTMD_931X_CMD_WRITE_C45		(BIT(3) | BIT(4))
#define   RTMD_931X_C22_DATA(page, reg)		((reg) << 6 | (page) << 11)
#define RTMD_931X_SMI_INDRT_ACCESS_CTRL_1	0x0c04
#define RTMD_931X_SMI_INDRT_ACCESS_CTRL_2(x)	(0x0c08 + (x) * 4)
#define RTMD_931X_SMI_INDRT_ACCESS_CTRL_3	0x0c10
#define RTMD_931X_SMI_INDRT_ACCESS_MMD		0x0c18
#define RTMD_931X_SMI_PHY_ABLTY_GET_SEL(pn)	(0x0cac + ((pn) / 16) * 4)
#define   RTMD_931X_SMI_PHY_ABLTY_MDIO		0x0
#define   RTMD_931X_SMI_PHY_ABLTY_SDS		0x2
#define   RTMD_931X_SMI_PHY_ABLTY_MASK(pn)	(GENMASK(1, 0) << (((pn) % 16) * 2))
#define RTMD_931X_SMI_PORT_POLLING_SEL		0x0c9c
#define RTMD_931X_SMI_PORT_ADDR_CTRL		0x0c74
#define RTMD_931X_SMI_10GPHY_POLLING_SEL0	0x0cf0
#define RTMD_931X_SMI_10GPHY_POLLING_SEL1	0x0cf4
#define RTMD_931X_SMI_10GPHY_POLLING_SEL2	0x0cf8
#define RTMD_931X_SMI_10GPHY_POLLING_SEL3	0x0cfc
#define RTMD_931X_SMI_10GPHY_POLLING_SEL4	0x0d00

#define for_each_phy_port(ctrl, pn) \
	for_each_set_bit(pn, (ctrl)->phy_ports, RTMD_MAX_PORTS)

#define for_each_sds_port(ctrl, pn) \
	for_each_set_bit(pn, (ctrl)->sds_ports, RTMD_MAX_PORTS)

struct rtmd_port {
	int page;
	bool raw;
	u8 smi_addr;
	u8 smi_bus;
};

struct rtmd_bus {
	bool is_c45;
	struct mii_bus *mii_bus;
};

struct rtmd_ctrl {
	struct mutex lock; /* protect HW access */
	struct regmap *map;
	const struct rtmd_config *cfg;
	struct rtmd_port port[RTMD_MAX_PORTS];
	struct rtmd_bus bus[RTMD_MAX_SMI_BUSES];
	DECLARE_BITMAP(phy_ports, RTMD_MAX_PORTS);
	DECLARE_BITMAP(sds_ports, RTMD_MAX_PORTS);
};

struct rtmd_chan {
	struct rtmd_ctrl *ctrl;
	u8 smi_bus;
	s8 port[PHY_MAX_ADDR];
};

struct rtmd_command_data {
	u32 brdcast;
	u32 c22_adr;
	u32 c45_adr;
	u32 ex_page;
	u32 io_data;
	u32 mask_lo;
	u32 mask_hi;
};

struct rtmd_config {
	u32 cmd_fail;
	u32 cmd_io_shift;
	struct rtmd_command_data cmd_regs;
	int bus_map_base;
	u16 num_buses;
	u16 num_pages;
	u16 num_ports;
	u32 poll_ctrl;
	int port_map_base;
	int (*read_c22)(struct mii_bus *bus, u32 pn, u32 page, u32 reg, u32 *val);
	int (*read_c45)(struct mii_bus *bus, u32 pn, u32 devnum, u32 regnum, u32 *val);
	int (*setup_ctrl)(struct rtmd_ctrl *ctrl);
	int (*setup_polling)(struct rtmd_ctrl *ctrl);
	int (*write_c22)(struct mii_bus *bus, u32 pn, u32 page, u32 reg, u32 val);
	int (*write_c45)(struct mii_bus *bus, u32 pn, u32 devnum, u32 regnum, u32 val);
};

struct rtmd_phy_info {
	int mac_type;
	bool has_giga_lite;
	bool has_res_reg;
	bool force_res;
	unsigned int poll_duplex;
	unsigned int poll_adv_1000;
	unsigned int poll_lpa_1000;
};

static struct rtmd_ctrl *rtmd_bus_to_ctrl(struct mii_bus *bus)
{
	return ((struct rtmd_chan *)bus->priv)->ctrl;
}

static int rtmd_phy_to_port(struct mii_bus *bus, int phy)
{
	struct rtmd_chan *chan = bus->priv;

	return chan->port[phy];
}

static int rtmd_run_cmd(struct mii_bus *bus, u32 cmd,
			struct rtmd_command_data *cmd_data, u32 *val)
{
	struct rtmd_ctrl *ctrl = rtmd_bus_to_ctrl(bus);
	u32 cmdstate;
	int ret;

	if (ctrl->cfg->cmd_regs.mask_hi) {
		/* high port count models have 3 extra command registers */
		ret = regmap_write(ctrl->map, ctrl->cfg->cmd_regs.brdcast, cmd_data->brdcast);
		if (ret)
			return ret;

		ret = regmap_write(ctrl->map, ctrl->cfg->cmd_regs.ex_page, cmd_data->ex_page);
		if (ret)
			return ret;

		ret = regmap_write(ctrl->map, ctrl->cfg->cmd_regs.mask_hi, cmd_data->mask_hi);
		if (ret)
			return ret;
	}

	ret = regmap_write(ctrl->map, ctrl->cfg->cmd_regs.mask_lo, cmd_data->mask_lo);
	if (ret)
		return ret;

	/* depending on model write output data to high or low word in I/O register */
	ret = regmap_write(ctrl->map, ctrl->cfg->cmd_regs.io_data,
			   cmd_data->io_data << ctrl->cfg->cmd_io_shift);
	if (ret)
		return ret;

	ret = regmap_write(ctrl->map, ctrl->cfg->cmd_regs.c45_adr, cmd_data->c45_adr);
	if (ret)
		return ret;

	/* C22 data and command bits share the same register. */
	ret = regmap_write(ctrl->map, ctrl->cfg->cmd_regs.c22_adr,
			   cmd_data->c22_adr | cmd | RTMD_RUN);
	if (ret)
		return ret;

	ret = regmap_read_poll_timeout(ctrl->map, ctrl->cfg->cmd_regs.c22_adr,
				       cmdstate, !(cmdstate & RTMD_RUN), 20, 500000);
	if (ret) {
		dev_warn_once(&bus->dev, "access timed out\n");
		return ret;
	}

	if (cmdstate & ctrl->cfg->cmd_fail) {
		dev_warn_once(&bus->dev, "access failed\n");
		return -EIO;
	}

	if (!val)
		return 0;

	/* read back data from opposite word in I/O register */
	ret = regmap_read(ctrl->map, ctrl->cfg->cmd_regs.io_data, val);
	if (!ret)
		*val = (*val >> (16 - ctrl->cfg->cmd_io_shift)) & GENMASK(15, 0);

	return ret;
}

static int rtmd_838x_read_c22(struct mii_bus *bus, u32 pn, u32 page, u32 reg, u32 *val)
{
	struct rtmd_command_data cmd_data = {
		.c22_adr = RTMD_838X_C22_DATA(page, reg),
		.io_data = pn,
	};

	return rtmd_run_cmd(bus, RTMD_838X_CMD_READ_C22, &cmd_data, val);
}

static int rtmd_838x_write_c22(struct mii_bus *bus, u32 pn, u32 page, u32 reg, u32 val)
{
	struct rtmd_command_data cmd_data = {
		.c22_adr = RTMD_838X_C22_DATA(page, reg),
		.io_data = val,
		.mask_lo = BIT(pn),
	};

	return rtmd_run_cmd(bus, RTMD_838X_CMD_WRITE_C22, &cmd_data, NULL);
}

static int rtmd_838x_read_c45(struct mii_bus *bus, u32 pn, u32 devnum, u32 regnum, u32 *val)
{
	struct rtmd_command_data cmd_data = {
		.c45_adr = RTMD_C45_DATA(devnum, regnum),
		.io_data = pn,
	};

	return rtmd_run_cmd(bus, RTMD_838X_CMD_READ_C45, &cmd_data, val);
}

static int rtmd_838x_write_c45(struct mii_bus *bus, u32 pn, u32 devnum, u32 regnum, u32 val)
{
	struct rtmd_command_data cmd_data = {
		.c45_adr = RTMD_C45_DATA(devnum, regnum),
		.io_data = val,
		.mask_lo = BIT(pn),
	};

	return rtmd_run_cmd(bus, RTMD_838X_CMD_WRITE_C45, &cmd_data, NULL);
}

static int rtmd_839x_read_c22(struct mii_bus *bus, u32 pn, u32 page, u32 reg, u32 *val)
{
	struct rtmd_command_data cmd_data = {
		.c22_adr = RTMD_839X_C22_DATA(page, reg),
		.ex_page = RTMD_839X_PHYREG_SKIP_EXT_PAGE,
		.io_data = pn,
	};

	return rtmd_run_cmd(bus, RTMD_839X_CMD_READ_C22, &cmd_data, val);
}

static int rtmd_839x_write_c22(struct mii_bus *bus, u32 pn, u32 page, u32 reg, u32 val)
{
	struct rtmd_command_data cmd_data = {
		.c22_adr = RTMD_839X_C22_DATA(page, reg),
		.ex_page = RTMD_839X_PHYREG_SKIP_EXT_PAGE,
		.io_data = val,
		.mask_lo = (u32)(BIT_ULL(pn)),
		.mask_hi = (u32)(BIT_ULL(pn) >> 32),
	};

	return rtmd_run_cmd(bus, RTMD_839X_CMD_WRITE_C22, &cmd_data, NULL);
}

static int rtmd_839x_read_c45(struct mii_bus *bus, u32 pn, u32 devnum, u32 regnum, u32 *val)
{
	struct rtmd_command_data cmd_data = {
		.c45_adr = RTMD_C45_DATA(devnum, regnum),
		.io_data = pn,
	};

	return rtmd_run_cmd(bus, RTMD_839X_CMD_READ_C45, &cmd_data, val);
}

static int rtmd_839x_write_c45(struct mii_bus *bus, u32 pn, u32 devnum, u32 regnum, u32 val)
{
	struct rtmd_command_data cmd_data = {
		.c45_adr = RTMD_C45_DATA(devnum, regnum),
		.io_data = val,
		.mask_lo = (u32)(BIT_ULL(pn)),
		.mask_hi = (u32)(BIT_ULL(pn) >> 32),
	};

	return rtmd_run_cmd(bus, RTMD_839X_CMD_WRITE_C45, &cmd_data, NULL);
}

static int rtmd_930x_read_c22(struct mii_bus *bus, u32 pn, u32 page, u32 reg, u32 *val)
{
	struct rtmd_command_data cmd_data = {
		.c22_adr = RTMD_930X_C22_DATA(page, reg),
		.io_data = pn,
	};

	return rtmd_run_cmd(bus, RTMD_930X_CMD_READ_C22, &cmd_data, val);
}

static int rtmd_930x_write_c22(struct mii_bus *bus, u32 pn, u32 page, u32 reg, u32 val)
{
	struct rtmd_command_data cmd_data = {
		.c22_adr = RTMD_930X_C22_DATA(page, reg),
		.io_data = val,
		.mask_lo = BIT(pn),
	};

	return rtmd_run_cmd(bus, RTMD_930X_CMD_WRITE_C22, &cmd_data, NULL);
}

static int rtmd_930x_read_c45(struct mii_bus *bus, u32 pn, u32 devnum, u32 regnum, u32 *val)
{
	struct rtmd_command_data cmd_data = {
		.c45_adr = RTMD_C45_DATA(devnum, regnum),
		.io_data = pn,
	};

	return rtmd_run_cmd(bus, RTMD_930X_CMD_READ_C45, &cmd_data, val);
}

static int rtmd_930x_write_c45(struct mii_bus *bus, u32 pn, u32 devnum, u32 regnum, u32 val)
{
	struct rtmd_command_data cmd_data = {
		.c45_adr = RTMD_C45_DATA(devnum, regnum),
		.io_data = val,
		.mask_lo = BIT(pn),
	};

	return rtmd_run_cmd(bus, RTMD_930X_CMD_WRITE_C45, &cmd_data, NULL);
}

static int rtmd_931x_read_c22(struct mii_bus *bus, u32 pn, u32 page, u32 reg, u32 *val)
{
	struct rtmd_command_data cmd_data = {
		.brdcast = RTMD_931X_SMI_INDRT_PORT(pn),
		.c22_adr = RTMD_931X_C22_DATA(page, reg),
	};

	return rtmd_run_cmd(bus, RTMD_931X_CMD_READ_C22, &cmd_data, val);
}

static int rtmd_931x_write_c22(struct mii_bus *bus, u32 pn, u32 page, u32 reg, u32 val)
{
	struct rtmd_command_data cmd_data = {
		.c22_adr = RTMD_931X_C22_DATA(page, reg),
		.mask_lo = (u32)(BIT_ULL(pn)),
		.mask_hi = (u32)(BIT_ULL(pn) >> 32),
		.io_data = val,
	};

	return rtmd_run_cmd(bus, RTMD_931X_CMD_WRITE_C22, &cmd_data, NULL);
}

static int rtmd_931x_read_c45(struct mii_bus *bus, u32 pn, u32 devnum, u32 regnum, u32 *val)
{
	struct rtmd_command_data cmd_data = {
		.brdcast = RTMD_931X_SMI_INDRT_PORT(pn),
		.c45_adr = RTMD_C45_DATA(devnum, regnum),
	};

	return rtmd_run_cmd(bus, RTMD_931X_CMD_READ_C45, &cmd_data, val);
}

static int rtmd_931x_write_c45(struct mii_bus *bus, u32 pn, u32 devnum, u32 regnum, u32 val)
{
	struct rtmd_command_data cmd_data = {
		.c45_adr = RTMD_C45_DATA(devnum, regnum),
		.mask_lo = (u32)(BIT_ULL(pn)),
		.mask_hi = (u32)(BIT_ULL(pn) >> 32),
		.io_data = val,
	};

	return rtmd_run_cmd(bus, RTMD_931X_CMD_WRITE_C45, &cmd_data, NULL);
}

static int rtmd_read_c45(struct mii_bus *bus, int phy, int devnum, int regnum)
{
	struct rtmd_ctrl *ctrl = rtmd_bus_to_ctrl(bus);
	int ret, pn, val = 0;

	pn = rtmd_phy_to_port(bus, phy);
	if (pn < 0)
		return -ENOENT;

	scoped_guard(mutex, &ctrl->lock)
		ret = (*ctrl->cfg->read_c45)(bus, pn, devnum, regnum, &val);

	dev_dbg(&bus->dev, "rd_MMD(phy=0x%02x, dev=0x%04x, reg=0x%04x) = 0x%04x, ret = %d\n",
		phy, devnum, regnum, val, ret);

	return ret ? ret : val;
}

static int rtmd_read_c22(struct mii_bus *bus, int phy, int regnum)
{
	struct rtmd_ctrl *ctrl = rtmd_bus_to_ctrl(bus);
	int ret, pn, page, val = 0;

	pn = rtmd_phy_to_port(bus, phy);
	if (pn < 0)
		return -ENOENT;

	scoped_guard(mutex, &ctrl->lock) {
		page = ctrl->port[pn].page;
		if (regnum == RTMD_PAGE_SELECT &&
		    page != RTMD_RAW_PAGE(ctrl->cfg->num_pages))
			return page;

		ctrl->port[pn].raw = (page == RTMD_RAW_PAGE(ctrl->cfg->num_pages));
		ret = (*ctrl->cfg->read_c22)(bus, pn, page, regnum, &val);
	}

	dev_dbg(&bus->dev, "rd_PHY(phy=0x%02x, pag=0x%04x, reg=0x%04x) = 0x%04x, ret = %d\n",
		phy, page, regnum, val, ret);

	return ret ? ret : val;
}

static int rtmd_write_c45(struct mii_bus *bus, int phy, int devnum, int regnum, u16 val)
{
	struct rtmd_ctrl *ctrl = rtmd_bus_to_ctrl(bus);
	int ret, pn;

	pn = rtmd_phy_to_port(bus, phy);
	if (pn < 0)
		return -ENOENT;

	scoped_guard(mutex, &ctrl->lock)
		ret = (*ctrl->cfg->write_c45)(bus, pn, devnum, regnum, val);
	dev_dbg(&bus->dev, "wr_MMD(phy=0x%02x, dev=0x%04x, reg=0x%04x, val=0x%04x), ret = %d\n",
		phy, devnum, regnum, val, ret);

	return ret;
}

static int rtmd_write_c22(struct mii_bus *bus, int phy, int regnum, u16 val)
{
	struct rtmd_ctrl *ctrl = rtmd_bus_to_ctrl(bus);
	bool do_write = false;
	int ret = 0, page, pn;

	pn = rtmd_phy_to_port(bus, phy);
	if (pn < 0)
		return -ENOENT;

	scoped_guard(mutex, &ctrl->lock) {
		page = ctrl->port[pn].page;
		if (regnum == RTMD_PAGE_SELECT)
			ctrl->port[pn].page = val;

		do_write = !ctrl->port[pn].raw &&
			   (regnum != RTMD_PAGE_SELECT ||
			    page == RTMD_RAW_PAGE(ctrl->cfg->num_pages));

		if (do_write) {
			ctrl->port[pn].raw = (page == RTMD_RAW_PAGE(ctrl->cfg->num_pages));
			ret = (*ctrl->cfg->write_c22)(bus, pn, page, regnum, val);
		} else {
			ctrl->port[pn].raw = false;
		}
	}

	if (do_write)
		dev_dbg(&bus->dev,
			"wr_PHY(phy=0x%02x, pag=0x%04x, reg=0x%04x, val=0x%04x), ret = %d\n",
			phy, page, regnum, val, ret);

	return ret;
}

static int rtmd_poll_port(struct rtmd_ctrl *ctrl, int pn, bool active)
{
	return regmap_assign_bits(ctrl->map, ctrl->cfg->poll_ctrl + (pn / 32) * 4,
				  BIT(pn % 32), active);
}

static int rtmd_disable_polling(struct rtmd_ctrl *ctrl)
{
	int pn, ret;

	for (pn = 0; pn < ctrl->cfg->num_ports; pn++) {
		ret = rtmd_poll_port(ctrl, pn, false);
		if (ret)
			return ret;
	}

	return 0;
}

static int rtmd_enable_polling(struct rtmd_ctrl *ctrl)
{
	int pn, ret;

	for_each_phy_port(ctrl, pn) {
		ret = rtmd_poll_port(ctrl, pn, true);
		if (ret)
			return ret;
	}

	for_each_sds_port(ctrl, pn) {
		ret = rtmd_poll_port(ctrl, pn, true);
		if (ret)
			return ret;
	}

	return 0;
}

static int rtmd_setup_smi_topology(struct rtmd_ctrl *ctrl)
{
	u32 reg, mask, val, pn;
	int ret;

	for_each_phy_port(ctrl, pn) {
		if (ctrl->cfg->bus_map_base) {
			reg = ctrl->cfg->bus_map_base + (pn / 16) * 4;
			mask = GENMASK(1, 0) << ((pn % 16) * 2);
			val = (u32)ctrl->port[pn].smi_bus << __ffs(mask);
			ret = regmap_update_bits(ctrl->map, reg, mask, val);
			if (ret)
				return ret;
		}

		if (ctrl->cfg->port_map_base) {
			reg = ctrl->cfg->port_map_base + (pn / 6) * 4;
			mask = GENMASK(4, 0) << ((pn % 6) * 5);
			val = (u32)ctrl->port[pn].smi_addr << __ffs(mask);
			ret = regmap_update_bits(ctrl->map, reg, mask, val);
			if (ret)
				return ret;
		}
	}

	return 0;
}

static u32 rtmd_get_phy_id(struct phy_device *phydev)
{
	if (!phydev)
		return 0;

	if (phydev->is_c45) {
		for (int devad = 0; devad < MDIO_MMD_NUM; devad++) {
			u32 phyid = phydev->c45_ids.device_ids[devad];

			if (phyid && phyid != 0xffffffff)
				return phyid;
		}
	}

	return phydev->phy_id;
}

static int rtmd_get_phy_info(struct rtmd_ctrl *ctrl, int pn, struct rtmd_phy_info *phyinfo)
{
	struct mii_bus *bus;
	u32 smi_addr, phyid;

	if (!test_bit(pn, ctrl->phy_ports))
		return -EINVAL;

	bus = ctrl->bus[ctrl->port[pn].smi_bus].mii_bus;
	smi_addr = ctrl->port[pn].smi_addr;
	phyid = rtmd_get_phy_id(mdiobus_get_phy(bus, smi_addr));
	*phyinfo = (struct rtmd_phy_info){};

	/* Determine PHY specific characteristics for polling fine tuning */
	switch (phyid) {
	case RTMD_PHY_AQR113C_A:
	case RTMD_PHY_AQR113C_B:
	case RTMD_PHY_AQR813:
		phyinfo->mac_type = RTMD_PHY_MAC_2G_PLUS;
		phyinfo->poll_duplex = RTMD_PHY_POLL_MMD(1, 0x0000, 8);
		phyinfo->poll_adv_1000 = RTMD_PHY_POLL_MMD(7, 0xc400, 15);
		phyinfo->poll_lpa_1000 = RTMD_PHY_POLL_MMD(7, 0xe820, 15);
		break;
	case RTMD_PHY_RTL8218D:
	case RTMD_PHY_RTL8218E:
		phyinfo->mac_type = RTMD_PHY_MAC_1G;
		phyinfo->has_giga_lite = true;
		break;
	case RTMD_PHY_RTL8226:
	case RTMD_PHY_RTL8221B_VB_CG:
	case RTMD_PHY_RTL8221B_VM_CG:
	case RTMD_PHY_RTL8224:
		phyinfo->mac_type = RTMD_PHY_MAC_2G_PLUS;
		phyinfo->has_giga_lite = true;
		phyinfo->poll_duplex = RTMD_PHY_POLL_MMD(31, 0xa400, 8);
		phyinfo->poll_adv_1000 = RTMD_PHY_POLL_MMD(31, 0xa412, 9);
		phyinfo->poll_lpa_1000 = RTMD_PHY_POLL_MMD(31, 0xa414, 11);
		break;
	case RTMD_PHY_RTL8261:
	case RTMD_PHY_RTL8264:
	case RTMD_PHY_RTL8264B:
		phyinfo->mac_type = RTMD_PHY_MAC_2G_PLUS;
		phyinfo->has_giga_lite = true;
		phyinfo->has_res_reg = true;
		break;
	default:
		dev_warn(&bus->dev, "skip polling setup for phy 0x%08x on port %d\n", phyid, pn);
		return -EINVAL;
	}

	return 0;
}

static struct fwnode_handle *rtmd_get_bus_node(struct fwnode_handle *phynode)
{
	struct fwnode_handle *parent = fwnode_get_parent(phynode);

	if (parent && fwnode_name_eq(parent, "ethernet-phy-package")) {
		struct fwnode_handle *grandparent = fwnode_get_parent(parent);
		fwnode_handle_put(parent);
		return grandparent;
	}
	return parent;
}

static int rtmd_838x_setup_ctrl(struct rtmd_ctrl *ctrl)
{
	/*
	 * PHY_PATCH_DONE enables phy control via SoC. This is required for phy access,
	 * including patching. Must always be set before the phys are probed.
	 */
	return regmap_set_bits(ctrl->map, RTMD_838X_SMI_GLB_CTRL,
			       RTMD_838X_SMI_GLB_PHY_PATCH_DONE);
}

static int rtmd_838x_setup_polling(struct rtmd_ctrl *ctrl)
{
	/*
	 * Control bits EX_PHY_MAN_xxx have an important effect on the detection of the media
	 * status (fibre/copper) of a PHY. Once activated, register MAC_LINK_MEDIA_STS can
	 * give the real media status (0=copper, 1=fibre). For now assume that if address 24 is
	 * PHY driven, it must be a combo PHY and media detection is needed.
	 */
	return regmap_assign_bits(ctrl->map, RTMD_838X_SMI_GLB_CTRL,
				  RTMD_838X_SMI_GLB_PHY_MAN_24_27,
				  test_bit(24, ctrl->phy_ports));
}

static int rtmd_839x_setup_polling(struct rtmd_ctrl *ctrl)
{
	/* This is the only device that has a global polling enable bit */
	return regmap_set_bits(ctrl->map, RTMD_839X_SMI_GLB_CTRL,
			       RTMD_839X_SMI_GLB_MDX_POLLING_EN);
}

static int rtmd_930x_setup_ctrl(struct rtmd_ctrl *ctrl)
{
	int ret;

	/* Define C22/C45 bus feature set */
	for (int smi_bus = 0; smi_bus < ctrl->cfg->num_buses; smi_bus++) {
		ret = regmap_assign_bits(ctrl->map, RTMD_930X_SMI_GLB_CTRL,
					 RTMD_930X_SMI_GLB_INTF_SEL(smi_bus),
					 ctrl->bus[smi_bus].is_c45);
		if (ret)
			return ret;
	}

	return 0;
}

static int rtmd_930x_set_port_ability(struct rtmd_ctrl *ctrl, u32 pn, u32 ability)
{
	u32 mask;

	if (pn < 24)
		mask = RTMD_930X_SMI_MAC_TYPE_P0_23(pn);
	else if (pn < 28)
		mask = RTMD_930X_SMI_MAC_TYPE_P24_27(pn);
	else
		return -EINVAL;

	return regmap_update_bits(ctrl->map, RTMD_930X_SMI_MAC_TYPE_CTRL,
				  mask, ability << __ffs(mask));
}

static int rtmd_930x_setup_polling(struct rtmd_ctrl *ctrl)
{
	struct rtmd_phy_info phyinfo;
	int ret;
	u32 pn;

	/* set all ports to "SerDes driven" */
	for (pn = 0; pn < ctrl->cfg->num_ports; pn++) {
		ret = rtmd_930x_set_port_ability(ctrl, pn, RTMD_PHY_MAC_SDS);
		if (ret)
			return ret;
	}

	/* Define PHY specific polling parameters */
	for_each_phy_port(ctrl, pn) {
		if (rtmd_get_phy_info(ctrl, pn, &phyinfo))
			continue;

		/* set port to "PHY driven" */
		ret = rtmd_930x_set_port_ability(ctrl, pn, phyinfo.mac_type);
		if (ret)
			return ret;

		/* polling via standard or resolution register */
		ret = regmap_assign_bits(ctrl->map, RTMD_930X_SMI_GLB_CTRL,
					 RTMD_930X_SMI_GLB_POLL_SEL(ctrl->port[pn].smi_bus),
					 phyinfo.has_res_reg);
		if (ret)
			return ret;

		/* proprietary Realtek 1G/2.5 lite polling */
		ret = regmap_assign_bits(ctrl->map, RTMD_930X_SMI_PRVTE_POLLING_CTRL,
					 BIT(pn), phyinfo.has_giga_lite);
		if (ret)
			return ret;

		if (!phyinfo.poll_duplex && !phyinfo.poll_adv_1000 && !phyinfo.poll_lpa_1000)
			continue;

		/* Unique 10G polling setup enforced by hardware design. Always same 10G PHYs. */
		ret = regmap_write(ctrl->map, RTMD_930X_SMI_10G_POLLING_REG0_CFG,
				   phyinfo.poll_duplex);
		if (ret)
			return ret;

		ret = regmap_write(ctrl->map, RTMD_930X_SMI_10G_POLLING_REG9_CFG,
				   phyinfo.poll_adv_1000);
		if (ret)
			return ret;

		ret = regmap_write(ctrl->map, RTMD_930X_SMI_10G_POLLING_REG10_CFG,
				   phyinfo.poll_lpa_1000);
		if (ret)
			return ret;
	}

	return 0;
}

static int rtmd_931x_setup_ctrl(struct rtmd_ctrl *ctrl)
{
	int ret;

	/* Define C22/C45 bus feature set (bit 1 of SMI_SETx_FMT_SEL) */
	for (int smi_bus = 0; smi_bus < ctrl->cfg->num_buses; smi_bus++) {
		ret = regmap_assign_bits(ctrl->map, RTMD_931X_SMI_GLB_CTRL1,
					 RTMD_931X_SMI_GLB_FMT_SEL_C45(smi_bus),
					 ctrl->bus[smi_bus].is_c45);
		if (ret)
			return ret;
	}

	return 0;
}

static int rtmd_931x_set_port_ability(struct rtmd_ctrl *ctrl, u32 pn, u32 ability)
{
	u32 mask;

	if (pn < ctrl->cfg->num_ports)
		mask = RTMD_931X_SMI_PHY_ABLTY_MASK(pn);
	else
		return -EINVAL;

	return regmap_update_bits(ctrl->map, RTMD_931X_SMI_PHY_ABLTY_GET_SEL(pn),
				  mask, ability << __ffs(mask));
}

static int rtmd_931x_setup_polling(struct rtmd_ctrl *ctrl)
{
	struct rtmd_phy_info phyinfo;
	int ret;
	u32 pn;

	/* set all ports to "SerDes driven" */
	for (pn = 0; pn < ctrl->cfg->num_ports; pn++) {
		ret = rtmd_931x_set_port_ability(ctrl, pn, RTMD_931X_SMI_PHY_ABLTY_SDS);
		if (ret)
			return ret;
	}

	/* Define PHY specific polling parameters */
	for_each_phy_port(ctrl, pn) {
		u8 smi_bus = ctrl->port[pn].smi_bus;

		if (rtmd_get_phy_info(ctrl, pn, &phyinfo))
			continue;

		/* set port to "PHY driven" */
		ret = rtmd_931x_set_port_ability(ctrl, pn, RTMD_931X_SMI_PHY_ABLTY_MDIO);
		if (ret)
			return ret;

		ret = regmap_assign_bits(ctrl->map, RTMD_931X_SMI_GLB_CTRL0,
					 RTMD_931X_SMI_GLB_PRVTE0_POLL(smi_bus),
					 phyinfo.has_res_reg);
		if (ret)
			return ret;

		ret = regmap_assign_bits(ctrl->map, RTMD_931X_SMI_GLB_CTRL0,
					 RTMD_931X_SMI_GLB_PRVTE1_POLL(smi_bus),
					 phyinfo.force_res);
		if (ret)
			return ret;

		/* polling std. or proprietary format (bit 0 of SMI_SETx_FMT_SEL) */
		ret = regmap_assign_bits(ctrl->map, RTMD_931X_SMI_GLB_CTRL1,
					 RTMD_931X_SMI_GLB_FMT_SEL_FRC(smi_bus),
					 phyinfo.force_res);
		if (ret)
			return ret;

		if (!phyinfo.poll_duplex && !phyinfo.poll_adv_1000 && !phyinfo.poll_lpa_1000)
			continue;

		/* Unique 10G polling setup enforced by hardware design. Always same 10G PHYs. */
		ret = regmap_write(ctrl->map, RTMD_931X_SMI_10GPHY_POLLING_SEL2,
				   phyinfo.poll_duplex);
		if (ret)
			return ret;

		ret = regmap_write(ctrl->map, RTMD_931X_SMI_10GPHY_POLLING_SEL3,
				   phyinfo.poll_adv_1000);
		if (ret)
			return ret;

		ret = regmap_write(ctrl->map, RTMD_931X_SMI_10GPHY_POLLING_SEL4,
				   phyinfo.poll_lpa_1000);
		if (ret)
			return ret;
	}

	return 0;
}

static int rtmd_map_ports(struct device *dev)
{
	struct fwnode_handle *fw_dev = dev_fwnode(dev);
	struct rtmd_ctrl *ctrl = dev_get_drvdata(dev);
	int smi_bus, smi_addr, pn;

	struct fwnode_handle *fw_switch __free(fwnode_handle) = fwnode_get_parent(fw_dev);
	if (!fw_switch)
		return -ENODEV;

	struct fwnode_handle *fw_ports __free(fwnode_handle) =
		fwnode_get_named_child_node(fw_switch, "ethernet-ports");
	if (!fw_ports)
		return dev_err_probe(dev, -ENODEV, "%pfwP missing ethernet-ports\n", fw_switch);

	fwnode_for_each_child_node_scoped(fw_ports, fw_port) {
		if (fwnode_property_read_u32(fw_port, "reg", &pn))
			continue;

		if (pn == ctrl->cfg->num_ports)
			continue;

		if (pn > ctrl->cfg->num_ports)
			return dev_err_probe(dev, -EINVAL, "%pfwP illegal port number\n", fw_port);

		if (test_bit(pn, ctrl->phy_ports) ||
		    test_bit(pn, ctrl->sds_ports))
			return dev_err_probe(dev, -EINVAL, "%pfwP duplicate port number\n",
					     fw_port);

		struct fwnode_handle *fw_phy __free(fwnode_handle) =
			fwnode_find_reference(fw_port, "phy-handle", 0);
		if (IS_ERR(fw_phy)) {
			/* port without a phy-handle is SerDes driven */
			__set_bit(pn, ctrl->sds_ports);
			continue;
		}

		if (fwnode_property_read_u32(fw_phy, "reg", &smi_addr))
			return dev_err_probe(dev, -EINVAL, "%pfwP no phy address\n", fw_phy);

		if (smi_addr >= PHY_MAX_ADDR)
			return dev_err_probe(dev, -EINVAL, "%pfwP illegal phy address\n", fw_phy);

		struct fwnode_handle *fw_bus __free(fwnode_handle) = rtmd_get_bus_node(fw_phy);
		if (!fw_bus || fwnode_property_read_u32(fw_bus, "reg", &smi_bus))
			return dev_err_probe(dev, -EINVAL, "%pfwP no bus number\n",
					     fw_bus ?: fw_phy);

		if (smi_bus >= ctrl->cfg->num_buses)
			return dev_err_probe(dev, -EINVAL, "%pfwP illegal bus number\n", fw_bus);

		if (fwnode_device_is_compatible(fw_phy, "ethernet-phy-ieee802.3-c45"))
			ctrl->bus[smi_bus].is_c45 = true;

		ctrl->port[pn].smi_bus = smi_bus;
		ctrl->port[pn].smi_addr = smi_addr;
		__set_bit(pn, ctrl->phy_ports);
	}

	return 0;
}

static int rtmd_probe_one(struct device *dev, struct rtmd_ctrl *ctrl,
			  struct fwnode_handle *fw_bus)
{
	struct rtmd_chan *chan;
	struct mii_bus *bus;
	int smi_bus, ret;
	u32 pn;

	ret = fwnode_property_read_u32(fw_bus, "reg", &smi_bus);
	if (ret)
		return dev_err_probe(dev, ret, "%pfwP no bus number\n", fw_bus);
	if (smi_bus >= ctrl->cfg->num_buses)
		return dev_err_probe(dev, -EINVAL, "%pfwP illegal bus number\n", fw_bus);

	bus = devm_mdiobus_alloc_size(dev, sizeof(*chan));
	if (!bus)
		return -ENOMEM;

	ctrl->bus[smi_bus].mii_bus = bus;

	chan = bus->priv;
	chan->ctrl = ctrl;
	chan->smi_bus = smi_bus;

	/* setup reverse lookup bus/phy -> port */
	for (int smi_addr = 0; smi_addr < ARRAY_SIZE(chan->port); smi_addr++)
		chan->port[smi_addr] = -1;
	for_each_phy_port(ctrl, pn)
		if (ctrl->port[pn].smi_bus == smi_bus)
			chan->port[ctrl->port[pn].smi_addr] = pn;

	bus->name = "Realtek MDIO bus";
	bus->read = rtmd_read_c22;
	bus->write = rtmd_write_c22;
	bus->read_c45 = rtmd_read_c45;
	bus->write_c45 = rtmd_write_c45;
	bus->parent = dev;
	snprintf(bus->id, MII_BUS_ID_SIZE, "realtek-mdio-%d", smi_bus);

	ret = devm_of_mdiobus_register(dev, bus, to_of_node(fw_bus));
	if (ret)
		return dev_err_probe(dev, ret, "cannot register MDIO %d bus\n", smi_bus);

	return 0;
}

static int rtmd_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct rtmd_ctrl *ctrl;
	int ret;

	ctrl = devm_kzalloc(dev, sizeof(*ctrl), GFP_KERNEL);
	if (!ctrl)
		return -ENOMEM;

	ret = devm_mutex_init(dev, &ctrl->lock);
	if (ret)
		return ret;

	platform_set_drvdata(pdev, ctrl);
	ctrl->cfg = (const struct rtmd_config *)device_get_match_data(dev);
	ctrl->map = syscon_node_to_regmap(pdev->dev.of_node->parent);
	if (IS_ERR(ctrl->map))
		return PTR_ERR(ctrl->map);

	ret = rtmd_disable_polling(ctrl);
	if (ret)
		return ret;

	ret = rtmd_map_ports(dev);
	if (ret)
		return ret;

	ret = rtmd_setup_smi_topology(ctrl);
	if (ret)
		return ret;

	if (ctrl->cfg->setup_ctrl) {
		ret = ctrl->cfg->setup_ctrl(ctrl);
		if (ret)
			return ret;
	}

	device_for_each_child_node_scoped(dev, child) {
		ret = rtmd_probe_one(dev, ctrl, child);
		if (ret)
			return ret;
	}

	if (ctrl->cfg->setup_polling) {
		ret = ctrl->cfg->setup_polling(ctrl);
		if (ret)
			return ret;
	}

	ret = rtmd_enable_polling(ctrl);
	if (ret)
		return ret;

	return 0;
}

static const struct rtmd_config rtmd_838x_cfg = {
	.cmd_fail	= RTMD_838X_CMD_FAIL,
	.cmd_io_shift	= RTMD_DATA_IN_HI_OUT_LOW,
	.cmd_regs = {
		.c22_adr = RTMD_838X_SMI_ACCESS_PHY_CTRL_1,
		.c45_adr = RTMD_838X_SMI_ACCESS_PHY_CTRL_3,
		.mask_lo = RTMD_838X_SMI_ACCESS_PHY_CTRL_0,
		.io_data = RTMD_838X_SMI_ACCESS_PHY_CTRL_2,
	},
	.num_buses	= RTMD_838X_NUM_BUSES,
	.num_pages	= RTMD_838X_NUM_PAGES,
	.num_ports	= RTMD_838X_NUM_PORTS,
	.poll_ctrl	= RTMD_838X_SMI_POLL_CTRL,
	.port_map_base	= RTMD_838X_SMI_PORT0_5_ADDR_CTRL,
	.read_c22	= rtmd_838x_read_c22,
	.read_c45	= rtmd_838x_read_c45,
	.setup_ctrl	= rtmd_838x_setup_ctrl,
	.setup_polling	= rtmd_838x_setup_polling,
	.write_c22	= rtmd_838x_write_c22,
	.write_c45	= rtmd_838x_write_c45,
};

static const struct rtmd_config rtmd_839x_cfg = {
	.cmd_fail	= RTMD_839X_CMD_FAIL,
	.cmd_io_shift	= RTMD_DATA_IN_HI_OUT_LOW,
	.cmd_regs = {
		.brdcast = RTMD_839X_BCAST_PHYID_CTRL,
		.c22_adr = RTMD_839X_PHYREG_ACCESS_CTRL,
		.c45_adr = RTMD_839X_PHYREG_MMD_CTRL,
		.ex_page = RTMD_839X_PHYREG_CTRL,
		.mask_lo = RTMD_839X_PHYREG_PORT_CTRL(0),
		.mask_hi = RTMD_839X_PHYREG_PORT_CTRL(1),
		.io_data = RTMD_839X_PHYREG_DATA_CTRL,
	},
	.num_buses	= RTMD_839X_NUM_BUSES,
	.num_pages	= RTMD_839X_NUM_PAGES,
	.num_ports	= RTMD_839X_NUM_PORTS,
	.poll_ctrl	= RTMD_839X_SMI_PORT_POLLING_CTRL,
	.read_c22	= rtmd_839x_read_c22,
	.read_c45	= rtmd_839x_read_c45,
	.setup_polling	= rtmd_839x_setup_polling,
	.write_c22	= rtmd_839x_write_c22,
	.write_c45	= rtmd_839x_write_c45,
};

static const struct rtmd_config rtmd_930x_cfg = {
	.cmd_fail	= RTMD_930X_CMD_FAIL,
	.cmd_io_shift	= RTMD_DATA_IN_HI_OUT_LOW,
	.cmd_regs = {
		.c22_adr = RTMD_930X_SMI_ACCESS_PHY_CTRL_1,
		.c45_adr = RTMD_930X_SMI_ACCESS_PHY_CTRL_3,
		.mask_lo = RTMD_930X_SMI_ACCESS_PHY_CTRL_0,
		.io_data = RTMD_930X_SMI_ACCESS_PHY_CTRL_2,
	},
	.bus_map_base	= RTMD_930X_SMI_PORT0_15_POLLING_SEL,
	.num_buses	= RTMD_930X_NUM_BUSES,
	.num_pages	= RTMD_930X_NUM_PAGES,
	.num_ports	= RTMD_930X_NUM_PORTS,
	.poll_ctrl	= RTMD_930X_SMI_POLL_CTRL,
	.port_map_base	= RTMD_930X_SMI_PORT0_5_ADDR_CTRL,
	.read_c22	= rtmd_930x_read_c22,
	.read_c45	= rtmd_930x_read_c45,
	.setup_ctrl	= rtmd_930x_setup_ctrl,
	.setup_polling	= rtmd_930x_setup_polling,
	.write_c22	= rtmd_930x_write_c22,
	.write_c45	= rtmd_930x_write_c45,
};

static const struct rtmd_config rtmd_931x_cfg = {
	.cmd_fail	= RTMD_931X_CMD_FAIL,
	.cmd_io_shift	= RTMD_DATA_IN_LOW_OUT_HI,
	.cmd_regs = {
		.brdcast = RTMD_931X_SMI_INDRT_ACCESS_BC,
		.c22_adr = RTMD_931X_SMI_INDRT_ACCESS_CTRL_0,
		.c45_adr = RTMD_931X_SMI_INDRT_ACCESS_MMD,
		.ex_page = RTMD_931X_SMI_INDRT_ACCESS_CTRL_1,
		.mask_lo = RTMD_931X_SMI_INDRT_ACCESS_CTRL_2(0),
		.mask_hi = RTMD_931X_SMI_INDRT_ACCESS_CTRL_2(1),
		.io_data = RTMD_931X_SMI_INDRT_ACCESS_CTRL_3,
	},
	.bus_map_base	= RTMD_931X_SMI_PORT_POLLING_SEL,
	.num_buses	= RTMD_931X_NUM_BUSES,
	.num_pages	= RTMD_931X_NUM_PAGES,
	.num_ports	= RTMD_931X_NUM_PORTS,
	.poll_ctrl	= RTMD_931X_SMI_PORT_POLLING_CTRL,
	.port_map_base	= RTMD_931X_SMI_PORT_ADDR_CTRL,
	.read_c22	= rtmd_931x_read_c22,
	.read_c45	= rtmd_931x_read_c45,
	.setup_ctrl	= rtmd_931x_setup_ctrl,
	.setup_polling	= rtmd_931x_setup_polling,
	.write_c22	= rtmd_931x_write_c22,
	.write_c45	= rtmd_931x_write_c45,
};

static const struct of_device_id rtmd_ids[] = {
	{ .compatible = "realtek,rtl8380-mdio", .data = &rtmd_838x_cfg, },
	{ .compatible = "realtek,rtl8392-mdio", .data = &rtmd_839x_cfg, },
	{ .compatible = "realtek,rtl9301-mdio", .data = &rtmd_930x_cfg, },
	{ .compatible = "realtek,rtl9311-mdio", .data = &rtmd_931x_cfg, },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, rtmd_ids);

static struct platform_driver rtmd_driver = {
	.probe = rtmd_probe,
	.driver = {
		.name = "mdio-rtl-otto",
		.of_match_table = rtmd_ids,
	},
};

module_platform_driver(rtmd_driver);

MODULE_AUTHOR("Markus Stockhausen <markus.stockhausen@gmx.de>");
MODULE_DESCRIPTION("Realtek Otto MDIO driver");
MODULE_LICENSE("GPL");
