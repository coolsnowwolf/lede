// SPDX-License-Identifier: GPL-2.0-only
/* Realtek RTL838X Ethernet MDIO interface driver
 *
 * Copyright (C) 2020 B. Koblitz
 */

#include <linux/module.h>
#include <linux/delay.h>
#include <linux/of.h>
#include <linux/phy.h>
#include <linux/netdevice.h>
#include <linux/firmware.h>
#include <linux/crc32.h>
#include <linux/sfp.h>
#include <linux/mii.h>
#include <linux/mdio.h>

#include <asm/mach-rtl838x/mach-rtl83xx.h>
#include "rtl83xx-phy.h"

extern struct rtl83xx_soc_info soc_info;
extern struct mutex smi_lock;
extern int phy_package_port_write_paged(struct phy_device *phydev, int port, int page, u32 regnum, u16 val);
extern int phy_package_write_paged(struct phy_device *phydev, int page, u32 regnum, u16 val);
extern int phy_port_write_paged(struct phy_device *phydev, int port, int page, u32 regnum, u16 val);
extern int phy_package_port_read_paged(struct phy_device *phydev, int port, int page, u32 regnum);
extern int phy_package_read_paged(struct phy_device *phydev, int page, u32 regnum);
extern int phy_port_read_paged(struct phy_device *phydev, int port, int page, u32 regnum);

#define PHY_PAGE_2	2
#define PHY_PAGE_4	4

/* all Clause-22 RealTek MDIO PHYs use register 0x1f for page select */
#define RTL8XXX_PAGE_SELECT		0x1f

#define RTL8XXX_PAGE_MAIN		0x0000
#define RTL821X_PAGE_PORT		0x0266
#define RTL821X_PAGE_POWER		0x0a40
#define RTL821X_PAGE_GPHY		0x0a42
#define RTL821X_PAGE_MAC		0x0a43
#define RTL821X_PAGE_STATE		0x0b80
#define RTL821X_PAGE_PATCH		0x0b82

/* Using the special page 0xfff with the MDIO controller found in
 * RealTek SoCs allows to access the PHY in RAW mode, ie. bypassing
 * the cache and paging engine of the MDIO controller.
 */
#define RTL838X_PAGE_RAW		0x0fff
#define RTL839X_PAGE_RAW		0x1fff

/* internal RTL821X PHY uses register 0x1d to select media page */
#define RTL821XINT_MEDIA_PAGE_SELECT	0x1d
/* external RTL821X PHY uses register 0x1e to select media page */
#define RTL821XEXT_MEDIA_PAGE_SELECT	0x1e

#define RTL821X_CHIP_ID			0x6276

#define RTL821X_MEDIA_PAGE_AUTO		0
#define RTL821X_MEDIA_PAGE_COPPER	1
#define RTL821X_MEDIA_PAGE_FIBRE	3
#define RTL821X_MEDIA_PAGE_INTERNAL	8

#define RTL9300_PHY_ID_MASK 0xf0ffffff

/* RTL930X SerDes supports the following modes:
 * 0x02: SGMII		0x04: 1000BX_FIBER	0x05: FIBER100
 * 0x06: QSGMII		0x09: RSGMII		0x0d: USXGMII
 * 0x10: XSGMII		0x12: HISGMII		0x16: 2500Base_X
 * 0x17: RXAUI_LITE	0x19: RXAUI_PLUS	0x1a: 10G Base-R
 * 0x1b: 10GR1000BX_AUTO			0x1f: OFF
 */
#define RTL930X_SDS_MODE_SGMII		0x02
#define RTL930X_SDS_MODE_1000BASEX	0x04
#define RTL930X_SDS_MODE_USXGMII	0x0d
#define RTL930X_SDS_MODE_XGMII		0x10
#define RTL930X_SDS_MODE_HSGMII		0x12
#define RTL930X_SDS_MODE_2500BASEX	0x16
#define RTL930X_SDS_MODE_10GBASER	0x1a
#define RTL930X_SDS_OFF			0x1f
#define RTL930X_SDS_MASK		0x1f

/* This lock protects the state of the SoC automatically polling the PHYs over the SMI
 * bus to detect e.g. link and media changes. For operations on the PHYs such as
 * patching or other configuration changes such as EEE, polling needs to be disabled
 * since otherwise these operations may fails or lead to unpredictable results.
 */
DEFINE_MUTEX(poll_lock);

static const struct firmware rtl838x_8380_fw;
static const struct firmware rtl838x_8214fc_fw;
static const struct firmware rtl838x_8218b_fw;

static u64 disable_polling(int port)
{
	u64 saved_state;

	mutex_lock(&poll_lock);

	switch (soc_info.family) {
	case RTL8380_FAMILY_ID:
		saved_state = sw_r32(RTL838X_SMI_POLL_CTRL);
		sw_w32_mask(BIT(port), 0, RTL838X_SMI_POLL_CTRL);
		break;
	case RTL8390_FAMILY_ID:
		saved_state = sw_r32(RTL839X_SMI_PORT_POLLING_CTRL + 4);
		saved_state <<= 32;
		saved_state |= sw_r32(RTL839X_SMI_PORT_POLLING_CTRL);
		sw_w32_mask(BIT(port % 32), 0,
		            RTL839X_SMI_PORT_POLLING_CTRL + ((port >> 5) << 2));
		break;
	case RTL9300_FAMILY_ID:
		saved_state = sw_r32(RTL930X_SMI_POLL_CTRL);
		sw_w32_mask(BIT(port), 0, RTL930X_SMI_POLL_CTRL);
		break;
	case RTL9310_FAMILY_ID:
		pr_warn("%s not implemented for RTL931X\n", __func__);
		break;
	}

	mutex_unlock(&poll_lock);

	return saved_state;
}

static int resume_polling(u64 saved_state)
{
	mutex_lock(&poll_lock);

	switch (soc_info.family) {
	case RTL8380_FAMILY_ID:
		sw_w32(saved_state, RTL838X_SMI_POLL_CTRL);
		break;
	case RTL8390_FAMILY_ID:
		sw_w32(saved_state >> 32, RTL839X_SMI_PORT_POLLING_CTRL + 4);
		sw_w32(saved_state, RTL839X_SMI_PORT_POLLING_CTRL);
		break;
	case RTL9300_FAMILY_ID:
		sw_w32(saved_state, RTL930X_SMI_POLL_CTRL);
		break;
	case RTL9310_FAMILY_ID:
		pr_warn("%s not implemented for RTL931X\n", __func__);
		break;
	}

	mutex_unlock(&poll_lock);

	return 0;
}

int rtl821x_match_phy_device(struct phy_device *phydev)
{
	u64 poll_state;
	int rawpage, port = phydev->mdio.addr & ~3;
	int oldpage, chip_mode, chip_cfg_mode;

	if (phydev->phy_id == PHY_ID_RTL8218B_E)
		return PHY_IS_RTL8218B_E;

	if (phydev->phy_id != PHY_ID_RTL8214_OR_8218)
		return PHY_IS_NOT_RTL821X;

	if (soc_info.family == RTL8380_FAMILY_ID)
		rawpage = RTL838X_PAGE_RAW;
	else if (soc_info.family == RTL8390_FAMILY_ID)
		rawpage = RTL839X_PAGE_RAW;
	else
		return PHY_IS_NOT_RTL821X;

	poll_state = disable_polling(port);
	/*
	 * At this stage the write_page()/read_page() PHY functions are not yet
	 * registered and normal paged access is not possible. The following
	 * detection routine works because our MDIO bus has all the Realtek
	 * PHY page handling (register 31) integrated into the port functions.
	 */
	oldpage = phy_port_read_paged(phydev, port, rawpage, 31);
	phy_port_write_paged(phydev, port, rawpage, 31, 0xa42);
	phy_port_write_paged(phydev, port, rawpage, 29, 0x008);
	phy_port_write_paged(phydev, port, rawpage, 31, 0x278);
	phy_port_write_paged(phydev, port, rawpage, 18, 0x455);
	phy_port_write_paged(phydev, port, rawpage, 31, 0x260);
	chip_mode = phy_port_read_paged(phydev, port, rawpage, 18);
	phy_port_write_paged(phydev, port, rawpage, 31, 0xa42);
	phy_port_write_paged(phydev, port, rawpage, 29, 0x000);
	phy_port_write_paged(phydev, port, rawpage, 31, oldpage);

	resume_polling(poll_state);

	pr_debug("%s(%d): got chip mode %x\n", __func__, phydev->mdio.addr, chip_mode);

	/* we checked the 4th port of a RTL8218B and got no config values */
	if (!chip_mode)
		return PHY_IS_RTL8218B_E;

	chip_cfg_mode = (chip_mode >> 4) & 0xf;
	chip_mode &= 0xf;

	if (chip_mode == 0xd || chip_mode == 0xf)
		return PHY_IS_RTL8218B_E;

	if (chip_mode == 0x4 || chip_mode == 0x6)
		return PHY_IS_RTL8214FC;

	if (chip_mode != 0xc && chip_mode != 0xe)
		return PHY_IS_NOT_RTL821X;

	if (chip_cfg_mode == 0x4 || chip_cfg_mode == 0x6)
		return PHY_IS_RTL8214FC;

	return PHY_IS_RTL8214FB;
}

static int rtl8218b_ext_match_phy_device(struct phy_device *phydev)
{
	return rtl821x_match_phy_device(phydev) == PHY_IS_RTL8218B_E;
}

static int rtl8214fc_match_phy_device(struct phy_device *phydev)
{
	return rtl821x_match_phy_device(phydev) == PHY_IS_RTL8214FC;
}

static void rtl8380_int_phy_on_off(struct phy_device *phydev, bool on)
{
	phy_modify(phydev, 0, BMCR_PDOWN, on ? 0 : BMCR_PDOWN);
}

static void rtl8380_rtl8214fc_on_off(struct phy_device *phydev, bool on)
{
	/* fiber ports */
	phy_write_paged(phydev, RTL838X_PAGE_RAW, RTL821XEXT_MEDIA_PAGE_SELECT, RTL821X_MEDIA_PAGE_FIBRE);
	phy_modify(phydev, 0x10, BMCR_PDOWN, on ? 0 : BMCR_PDOWN);

	/* copper ports */
	phy_write_paged(phydev, RTL838X_PAGE_RAW, RTL821XEXT_MEDIA_PAGE_SELECT, RTL821X_MEDIA_PAGE_COPPER);
	phy_modify_paged(phydev, RTL821X_PAGE_POWER, 0x10, BMCR_PDOWN, on ? 0 : BMCR_PDOWN);
}

static void rtl8380_phy_reset(struct phy_device *phydev)
{
	phy_modify(phydev, 0, BMCR_RESET, BMCR_RESET);
}

/* The access registers for SDS_MODE_SEL and the LSB for each SDS within */
u16 rtl9300_sds_regs[] = { 0x0194, 0x0194, 0x0194, 0x0194, 0x02a0, 0x02a0, 0x02a0, 0x02a0,
			   0x02A4, 0x02A4, 0x0198, 0x0198 };
u8  rtl9300_sds_lsb[]  = { 0, 6, 12, 18, 0, 6, 12, 18, 0, 6, 0, 6};

/* Reset the SerDes by powering it off and set a new operation mode
 * of the SerDes.
 */
void rtl9300_sds_rst(int sds_num, u32 mode)
{
	pr_info("%s %d\n", __func__, mode);
	if (sds_num < 0 || sds_num > 11) {
		pr_err("Wrong SerDes number: %d\n", sds_num);
		return;
	}

	sw_w32_mask(RTL930X_SDS_MASK << rtl9300_sds_lsb[sds_num],
		    RTL930X_SDS_OFF << rtl9300_sds_lsb[sds_num],
		    rtl9300_sds_regs[sds_num]);
	mdelay(10);

	sw_w32_mask(RTL930X_SDS_MASK << rtl9300_sds_lsb[sds_num], mode << rtl9300_sds_lsb[sds_num],
		    rtl9300_sds_regs[sds_num]);
	mdelay(10);

	pr_debug("%s: 194:%08x 198:%08x 2a0:%08x 2a4:%08x\n", __func__,
	         sw_r32(0x194), sw_r32(0x198), sw_r32(0x2a0), sw_r32(0x2a4));
}

void rtl9300_sds_set(int sds_num, u32 mode)
{
	pr_info("%s %d\n", __func__, mode);
	if (sds_num < 0 || sds_num > 11) {
		pr_err("Wrong SerDes number: %d\n", sds_num);
		return;
	}

	sw_w32_mask(RTL930X_SDS_MASK << rtl9300_sds_lsb[sds_num], mode << rtl9300_sds_lsb[sds_num],
		    rtl9300_sds_regs[sds_num]);
	mdelay(10);

	pr_debug("%s: 194:%08x 198:%08x 2a0:%08x 2a4:%08x\n", __func__,
	         sw_r32(0x194), sw_r32(0x198), sw_r32(0x2a0), sw_r32(0x2a4));
}

u32 rtl9300_sds_mode_get(int sds_num)
{
	u32 v;

	if (sds_num < 0 || sds_num > 11) {
		pr_err("Wrong SerDes number: %d\n", sds_num);
		return 0;
	}

	v = sw_r32(rtl9300_sds_regs[sds_num]);
	v >>= rtl9300_sds_lsb[sds_num];

	return v & RTL930X_SDS_MASK;
}

/* On the RTL839x family of SoCs with inbuilt SerDes, these SerDes are accessed through
 * a 2048 bit register that holds the contents of the PHY being simulated by the SoC.
 */
int rtl839x_read_sds_phy(int phy_addr, int phy_reg)
{
	int offset = 0;
	int reg;
	u32 val;

	if (phy_addr == 49)
		offset = 0x100;

	/* For the RTL8393 internal SerDes, we simulate a PHY ID in registers 2/3
	 * which would otherwise read as 0.
	 */
	if (soc_info.id == 0x8393) {
		if (phy_reg == MII_PHYSID1)
			return 0x1c;
		if (phy_reg == MII_PHYSID2)
			return 0x8393;
	}

	/* Register RTL839X_SDS12_13_XSG0 is 2048 bit broad, the MSB (bit 15) of the
	 * 0th PHY register is bit 1023 (in byte 0x80). Because PHY-registers are 16
	 * bit broad, we offset by reg << 1. In the SoC 2 registers are stored in
	 * one 32 bit register.
	 */
	reg = (phy_reg << 1) & 0xfc;
	val = sw_r32(RTL839X_SDS12_13_XSG0 + offset + 0x80 + reg);

	if (phy_reg & 1)
		val = (val >> 16) & 0xffff;
	else
		val &= 0xffff;

	return val;
}

/* On the RTL930x family of SoCs, the internal SerDes are accessed through an IO
 * register which simulates commands to an internal MDIO bus.
 */
int rtl930x_read_sds_phy(int phy_addr, int page, int phy_reg)
{
	int i;
	u32 cmd = phy_addr << 2 | page << 7 | phy_reg << 13 | 1;

	sw_w32(cmd, RTL930X_SDS_INDACS_CMD);

	for (i = 0; i < 100; i++) {
		if (!(sw_r32(RTL930X_SDS_INDACS_CMD) & 0x1))
			break;
		mdelay(1);
	}

	if (i >= 100)
		return -EIO;

	return sw_r32(RTL930X_SDS_INDACS_DATA) & 0xffff;
}

int rtl930x_write_sds_phy(int phy_addr, int page, int phy_reg, u16 v)
{
	int i;
	u32 cmd;

	sw_w32(v, RTL930X_SDS_INDACS_DATA);
	cmd = phy_addr << 2 | page << 7 | phy_reg << 13 | 0x3;

	sw_w32(cmd, RTL930X_SDS_INDACS_CMD);

	for (i = 0; i < 100; i++) {
		if (!(sw_r32(RTL930X_SDS_INDACS_CMD) & 0x1))
			break;
		mdelay(1);
	}


	if (i >= 100) {
		pr_info("%s ERROR !!!!!!!!!!!!!!!!!!!!\n", __func__);
		return -EIO;
	}

	return 0;
}

int rtl931x_read_sds_phy(int phy_addr, int page, int phy_reg)
{
	int i;
	u32 cmd = phy_addr << 2 | page << 7 | phy_reg << 13 | 1;

	pr_debug("%s: phy_addr(SDS-ID) %d, phy_reg: %d\n", __func__, phy_addr, phy_reg);
	sw_w32(cmd, RTL931X_SERDES_INDRT_ACCESS_CTRL);

	for (i = 0; i < 100; i++) {
		if (!(sw_r32(RTL931X_SERDES_INDRT_ACCESS_CTRL) & 0x1))
			break;
		mdelay(1);
	}

	if (i >= 100)
		return -EIO;

	pr_debug("%s: returning %04x\n", __func__, sw_r32(RTL931X_SERDES_INDRT_DATA_CTRL) & 0xffff);

	return sw_r32(RTL931X_SERDES_INDRT_DATA_CTRL) & 0xffff;
}

int rtl931x_write_sds_phy(int phy_addr, int page, int phy_reg, u16 v)
{
	int i;
	u32 cmd;

	cmd = phy_addr << 2 | page << 7 | phy_reg << 13;
	sw_w32(cmd, RTL931X_SERDES_INDRT_ACCESS_CTRL);

	sw_w32(v, RTL931X_SERDES_INDRT_DATA_CTRL);

	cmd =  sw_r32(RTL931X_SERDES_INDRT_ACCESS_CTRL) | 0x3;
	sw_w32(cmd, RTL931X_SERDES_INDRT_ACCESS_CTRL);

	for (i = 0; i < 100; i++) {
		if (!(sw_r32(RTL931X_SERDES_INDRT_ACCESS_CTRL) & 0x1))
			break;
		mdelay(1);
	}

	if (i >= 100)
		return -EIO;

	return 0;
}

/* On the RTL838x SoCs, the internal SerDes is accessed through direct access to
 * standard PHY registers, where a 32 bit register holds a 16 bit word as found
 * in a standard page 0 of a PHY
 */
int rtl838x_read_sds_phy(int phy_addr, int phy_reg)
{
	int offset = 0;
	u32 val;

	if (phy_addr == 26)
		offset = 0x100;
	val = sw_r32(RTL838X_SDS4_FIB_REG0 + offset + (phy_reg << 2)) & 0xffff;

	return val;
}

int rtl839x_write_sds_phy(int phy_addr, int phy_reg, u16 v)
{
	int offset = 0;
	int reg;
	u32 val;

	if (phy_addr == 49)
		offset = 0x100;

	reg = (phy_reg << 1) & 0xfc;
	val = v;
	if (phy_reg & 1) {
		val = val << 16;
		sw_w32_mask(0xffff0000, val,
			    RTL839X_SDS12_13_XSG0 + offset + 0x80 + reg);
	} else {
		sw_w32_mask(0xffff, val,
			    RTL839X_SDS12_13_XSG0 + offset + 0x80 + reg);
	}

	return 0;
}

/* Read the link and speed status of the 2 internal SGMII/1000Base-X
 * ports of the RTL838x SoCs
 */
static int rtl8380_read_status(struct phy_device *phydev)
{
	int err;

	err = genphy_read_status(phydev);

	if (phydev->link) {
		phydev->speed = SPEED_1000;
		phydev->duplex = DUPLEX_FULL;
	}

	return err;
}

/* Read the link and speed status of the 2 internal SGMII/1000Base-X
 * ports of the RTL8393 SoC
 */
static int rtl8393_read_status(struct phy_device *phydev)
{
	int offset = 0;
	int err;
	int phy_addr = phydev->mdio.addr;
	u32 v;

	err = genphy_read_status(phydev);
	if (phy_addr == 49)
		offset = 0x100;

	if (phydev->link) {
		phydev->speed = SPEED_100;
		/* Read SPD_RD_00 (bit 13) and SPD_RD_01 (bit 6) out of the internal
		 * PHY registers
		 */
		v = sw_r32(RTL839X_SDS12_13_XSG0 + offset + 0x80);
		if (!(v & (1 << 13)) && (v & (1 << 6)))
			phydev->speed = SPEED_1000;
		phydev->duplex = DUPLEX_FULL;
	}

	return err;
}

static int rtl821x_read_page(struct phy_device *phydev)
{
	return __phy_read(phydev, RTL8XXX_PAGE_SELECT);
}

static int rtl821x_write_page(struct phy_device *phydev, int page)
{
	return __phy_write(phydev, RTL8XXX_PAGE_SELECT, page);
}

static int rtl8226_read_status(struct phy_device *phydev)
{
	int ret = 0;
	u32 val;

/* TODO: ret = genphy_read_status(phydev);
 * 	if (ret < 0) {
 * 		pr_info("%s: genphy_read_status failed\n", __func__);
 * 		return ret;
 * 	}
 */

	/* Link status must be read twice */
	for (int i = 0; i < 2; i++)
		val = phy_read_mmd(phydev, MDIO_MMD_VEND2, 0xA402);

	phydev->link = val & BIT(2) ? 1 : 0;
	if (!phydev->link)
		goto out;

	/* Read duplex status */
	val = phy_read_mmd(phydev, MDIO_MMD_VEND2, 0xA434);
	if (val < 0)
		goto out;
	phydev->duplex = !!(val & BIT(3));

	/* Read speed */
	val = phy_read_mmd(phydev, MDIO_MMD_VEND2, 0xA434);
	switch (val & 0x0630) {
	case 0x0000:
		phydev->speed = SPEED_10;
		break;
	case 0x0010:
		phydev->speed = SPEED_100;
		break;
	case 0x0020:
		phydev->speed = SPEED_1000;
		break;
	case 0x0200:
		phydev->speed = SPEED_10000;
		break;
	case 0x0210:
		phydev->speed = SPEED_2500;
		break;
	case 0x0220:
		phydev->speed = SPEED_5000;
		break;
	default:
		break;
	}

out:
	return ret;
}

static int rtl8226_advertise_aneg(struct phy_device *phydev)
{
	int ret = 0;
	u32 v;

	pr_info("In %s\n", __func__);

	v = phy_read_mmd(phydev, MDIO_MMD_AN, MDIO_AN_ADVERTISE);
	if (v < 0)
		goto out;

	v |= ADVERTISE_10HALF;
	v |= ADVERTISE_10FULL;
	v |= ADVERTISE_100HALF;
	v |= ADVERTISE_100FULL;

	ret = phy_write_mmd(phydev, MDIO_MMD_AN, MDIO_AN_ADVERTISE, v);

	/* Allow 1GBit */
	v = phy_read_mmd(phydev, MDIO_MMD_VEND2, 0xA412);
	if (v < 0)
		goto out;
	v |= ADVERTISE_1000FULL;

	ret = phy_write_mmd(phydev, MDIO_MMD_VEND2, 0xA412, v);
	if (ret < 0)
		goto out;

	/* Allow 2.5G */
	v = phy_read_mmd(phydev, MDIO_MMD_AN, MDIO_AN_10GBT_CTRL);
	if (v < 0)
		goto out;

	v |= MDIO_AN_10GBT_CTRL_ADV2_5G;
	ret = phy_write_mmd(phydev, MDIO_MMD_AN, MDIO_AN_10GBT_CTRL, v);

out:
	return ret;
}

static int rtl8226_config_aneg(struct phy_device *phydev)
{
	int ret = 0;
	u32 v;

	pr_debug("In %s\n", __func__);
	if (phydev->autoneg == AUTONEG_ENABLE) {
		ret = rtl8226_advertise_aneg(phydev);
		if (ret)
			goto out;
		/* AutoNegotiationEnable */
		v = phy_read_mmd(phydev, MDIO_MMD_AN, MDIO_CTRL1);
		if (v < 0)
			goto out;

		v |= MDIO_AN_CTRL1_ENABLE; /* Enable AN */
		ret = phy_write_mmd(phydev, MDIO_MMD_AN, MDIO_CTRL1, v);
		if (ret < 0)
			goto out;

		/* RestartAutoNegotiation */
		v = phy_read_mmd(phydev, MDIO_MMD_VEND2, 0xA400);
		if (v < 0)
			goto out;
		v |= MDIO_AN_CTRL1_RESTART;

		ret = phy_write_mmd(phydev, MDIO_MMD_VEND2, 0xA400, v);
	}

/*	TODO: ret = __genphy_config_aneg(phydev, ret); */

out:
	return ret;
}

static int rtl8226_get_eee(struct phy_device *phydev,
			   struct ethtool_eee *e)
{
	u32 val;
	int addr = phydev->mdio.addr;

	pr_debug("In %s, port %d, was enabled: %d\n", __func__, addr, e->eee_enabled);

	val = phy_read_mmd(phydev, MDIO_MMD_AN, MDIO_AN_EEE_ADV);
	if (e->eee_enabled) {
		e->eee_enabled = !!(val & MDIO_EEE_100TX);
		if (!e->eee_enabled) {
			val = phy_read_mmd(phydev, MDIO_MMD_AN, MDIO_AN_EEE_ADV2);
			e->eee_enabled = !!(val & MDIO_EEE_2_5GT);
		}
	}
	pr_debug("%s: enabled: %d\n", __func__, e->eee_enabled);

	return 0;
}

static int rtl8226_set_eee(struct phy_device *phydev, struct ethtool_eee *e)
{
	int port = phydev->mdio.addr;
	u64 poll_state;
	bool an_enabled;
	u32 val;

	pr_info("In %s, port %d, enabled %d\n", __func__, port, e->eee_enabled);

	poll_state = disable_polling(port);

	/* Remember aneg state */
	val = phy_read_mmd(phydev, MDIO_MMD_AN, MDIO_CTRL1);
	an_enabled = !!(val & MDIO_AN_CTRL1_ENABLE);

	/* Setup 100/1000MBit */
	val = phy_read_mmd(phydev, MDIO_MMD_AN, MDIO_AN_EEE_ADV);
	if (e->eee_enabled)
		val |= (MDIO_EEE_100TX | MDIO_EEE_1000T);
	else
		val &= (MDIO_EEE_100TX | MDIO_EEE_1000T);
	phy_write_mmd(phydev, MDIO_MMD_AN, MDIO_AN_EEE_ADV, val);

	/* Setup 2.5GBit */
	val = phy_read_mmd(phydev, MDIO_MMD_AN, MDIO_AN_EEE_ADV2);
	if (e->eee_enabled)
		val |= MDIO_EEE_2_5GT;
	else
		val &= MDIO_EEE_2_5GT;
	phy_write_mmd(phydev, MDIO_MMD_AN, MDIO_AN_EEE_ADV2, val);

	/* RestartAutoNegotiation */
	val = phy_read_mmd(phydev, MDIO_MMD_VEND2, 0xA400);
	val |= MDIO_AN_CTRL1_RESTART;
	phy_write_mmd(phydev, MDIO_MMD_VEND2, 0xA400, val);

	resume_polling(poll_state);

	return 0;
}

static struct fw_header *rtl838x_request_fw(struct phy_device *phydev,
					    const struct firmware *fw,
					    const char *name)
{
	struct device *dev = &phydev->mdio.dev;
	int err;
	struct fw_header *h;
	uint32_t checksum, my_checksum;

	err = request_firmware(&fw, name, dev);
	if (err < 0)
		goto out;

	if (fw->size < sizeof(struct fw_header)) {
		pr_err("Firmware size too small.\n");
		err = -EINVAL;
		goto out;
	}

	h = (struct fw_header *) fw->data;
	pr_info("Firmware loaded. Size %d, magic: %08x\n", fw->size, h->magic);

	if (h->magic != 0x83808380) {
		pr_err("Wrong firmware file: MAGIC mismatch.\n");
		goto out;
	}

	checksum = h->checksum;
	h->checksum = 0;
	my_checksum = ~crc32(0xFFFFFFFFU, fw->data, fw->size);
	if (checksum != my_checksum) {
		pr_err("Firmware checksum mismatch.\n");
		err = -EINVAL;
		goto out;
	}
	h->checksum = checksum;

	return h;
out:
	dev_err(dev, "Unable to load firmware %s (%d)\n", name, err);
	return NULL;
}

static void rtl821x_phy_setup_package_broadcast(struct phy_device *phydev, bool enable)
{
	int mac = phydev->mdio.addr;

	/* select main page 0 */
	phy_write_paged(phydev, RTL838X_PAGE_RAW, RTL8XXX_PAGE_SELECT, RTL8XXX_PAGE_MAIN);
	/* write to 0x8 to register 0x1d on main page 0 */
	phy_write_paged(phydev, RTL838X_PAGE_RAW, RTL821XINT_MEDIA_PAGE_SELECT, RTL821X_MEDIA_PAGE_INTERNAL);
	/* select page 0x266 */
	phy_write_paged(phydev, RTL838X_PAGE_RAW, RTL8XXX_PAGE_SELECT, RTL821X_PAGE_PORT);
	/* set phy id and target broadcast bitmap in register 0x16 on page 0x266 */
	phy_write_paged(phydev, RTL838X_PAGE_RAW, 0x16, (enable?0xff00:0x00) | mac);
	/* return to main page 0 */
	phy_write_paged(phydev, RTL838X_PAGE_RAW, RTL8XXX_PAGE_SELECT, RTL8XXX_PAGE_MAIN);
	/* write to 0x0 to register 0x1d on main page 0 */
	phy_write_paged(phydev, RTL838X_PAGE_RAW, RTL821XINT_MEDIA_PAGE_SELECT, RTL821X_MEDIA_PAGE_AUTO);
	mdelay(1);
}

static int rtl8390_configure_generic(struct phy_device *phydev)
{
	int mac = phydev->mdio.addr;
	u32 val, phy_id;

	val = phy_read(phydev, 2);
	phy_id = val << 16;
	val = phy_read(phydev, 3);
	phy_id |= val;
	pr_debug("Phy on MAC %d: %x\n", mac, phy_id);

	/* Read internal PHY ID */
	phy_write_paged(phydev, 31, 27, 0x0002);
	val = phy_read_paged(phydev, 31, 28);

	/* Internal RTL8218B, version 2 */
	phydev_info(phydev, "Detected unknown %x\n", val);

	return 0;
}

static int rtl8380_configure_int_rtl8218b(struct phy_device *phydev)
{
	u32 val, phy_id;
	int mac = phydev->mdio.addr;
	struct fw_header *h;
	u32 *rtl838x_6275B_intPhy_perport;
	u32 *rtl8218b_6276B_hwEsd_perport;

	val = phy_read(phydev, 2);
	phy_id = val << 16;
	val = phy_read(phydev, 3);
	phy_id |= val;
	pr_debug("Phy on MAC %d: %x\n", mac, phy_id);

	/* Read internal PHY ID */
	phy_write_paged(phydev, 31, 27, 0x0002);
	val = phy_read_paged(phydev, 31, 28);
	if (val != 0x6275) {
		phydev_err(phydev, "Expected internal RTL8218B, found PHY-ID %x\n", val);
		return -1;
	}

	/* Internal RTL8218B, version 2 */
	phydev_info(phydev, "Detected internal RTL8218B\n");

	h = rtl838x_request_fw(phydev, &rtl838x_8380_fw, FIRMWARE_838X_8380_1);
	if (!h)
		return -1;

	if (h->phy != 0x83800000) {
		phydev_err(phydev, "Wrong firmware file: PHY mismatch.\n");
		return -1;
	}

	rtl838x_6275B_intPhy_perport = (void *)h + sizeof(struct fw_header) + h->parts[8].start;
	rtl8218b_6276B_hwEsd_perport = (void *)h + sizeof(struct fw_header) + h->parts[9].start;

	// Currently not used
	// if (sw_r32(RTL838X_DMY_REG31) == 0x1) {
	// 	int ipd_flag = 1;
	// }

	val = phy_read(phydev, MII_BMCR);
	if (val & BMCR_PDOWN)
		rtl8380_int_phy_on_off(phydev, true);
	else
		rtl8380_phy_reset(phydev);
	msleep(100);

	/* Ready PHY for patch */
	for (int p = 0; p < 8; p++) {
		phy_package_port_write_paged(phydev, p, RTL838X_PAGE_RAW, RTL8XXX_PAGE_SELECT, RTL821X_PAGE_PATCH);
		phy_package_port_write_paged(phydev, p, RTL838X_PAGE_RAW, 0x10, 0x0010);
	}
	msleep(500);
	for (int p = 0; p < 8; p++) {
		int i;

		for (i = 0; i < 100 ; i++) {
			val = phy_package_port_read_paged(phydev, p, RTL821X_PAGE_STATE, 0x10);
			if (val & 0x40)
				break;
		}
		if (i >= 100) {
			phydev_err(phydev,
			           "ERROR: Port %d not ready for patch.\n",
			           mac + p);
			return -1;
		}
	}
	for (int p = 0; p < 8; p++) {
		int i;

		i = 0;
		while (rtl838x_6275B_intPhy_perport[i * 2]) {
			phy_package_port_write_paged(phydev, p, RTL838X_PAGE_RAW,
			                             rtl838x_6275B_intPhy_perport[i * 2],
			                             rtl838x_6275B_intPhy_perport[i * 2 + 1]);
			i++;
		}
		i = 0;
		while (rtl8218b_6276B_hwEsd_perport[i * 2]) {
			phy_package_port_write_paged(phydev, p, RTL838X_PAGE_RAW,
			                             rtl8218b_6276B_hwEsd_perport[i * 2],
			                             rtl8218b_6276B_hwEsd_perport[i * 2 + 1]);
			i++;
		}
	}

	return 0;
}

static int rtl8380_configure_ext_rtl8218b(struct phy_device *phydev)
{
	u32 val, ipd, phy_id;
	int mac = phydev->mdio.addr;
	struct fw_header *h;
	u32 *rtl8380_rtl8218b_perchip;
	u32 *rtl8218B_6276B_rtl8380_perport;
	u32 *rtl8380_rtl8218b_perport;

	if (soc_info.family == RTL8380_FAMILY_ID && mac != 0 && mac != 16) {
		phydev_err(phydev, "External RTL8218B must have PHY-IDs 0 or 16!\n");
		return -1;
	}
	val = phy_read(phydev, 2);
	phy_id = val << 16;
	val = phy_read(phydev, 3);
	phy_id |= val;
	pr_info("Phy on MAC %d: %x\n", mac, phy_id);

	/* Read internal PHY ID */
	phy_write_paged(phydev, 31, 27, 0x0002);
	val = phy_read_paged(phydev, 31, 28);
	if (val != RTL821X_CHIP_ID) {
		phydev_err(phydev, "Expected external RTL8218B, found PHY-ID %x\n", val);
		return -1;
	}
	phydev_info(phydev, "Detected external RTL8218B\n");

	h = rtl838x_request_fw(phydev, &rtl838x_8218b_fw, FIRMWARE_838X_8218b_1);
	if (!h)
		return -1;

	if (h->phy != 0x8218b000) {
		phydev_err(phydev, "Wrong firmware file: PHY mismatch.\n");
		return -1;
	}

	rtl8380_rtl8218b_perchip = (void *)h + sizeof(struct fw_header) + h->parts[0].start;
	rtl8218B_6276B_rtl8380_perport = (void *)h + sizeof(struct fw_header) + h->parts[1].start;
	rtl8380_rtl8218b_perport = (void *)h + sizeof(struct fw_header) + h->parts[2].start;

	val = phy_read(phydev, MII_BMCR);
	if (val & BMCR_PDOWN)
		rtl8380_int_phy_on_off(phydev, true);
	else
		rtl8380_phy_reset(phydev);

	msleep(100);

	/* Get Chip revision */
	phy_write_paged(phydev, RTL838X_PAGE_RAW, RTL8XXX_PAGE_SELECT, RTL8XXX_PAGE_MAIN);
	phy_write_paged(phydev, RTL838X_PAGE_RAW, 0x1b, 0x4);
	val = phy_read_paged(phydev, RTL838X_PAGE_RAW, 0x1c);

	phydev_info(phydev, "Detected chip revision %04x\n", val);

	for (int i = 0; rtl8380_rtl8218b_perchip[i * 3] &&
	                rtl8380_rtl8218b_perchip[i * 3 + 1]; i++) {
		phy_package_port_write_paged(phydev, rtl8380_rtl8218b_perchip[i * 3],
					     RTL838X_PAGE_RAW, rtl8380_rtl8218b_perchip[i * 3 + 1],
					     rtl8380_rtl8218b_perchip[i * 3 + 2]);
	}

	/* Enable PHY */
	for (int i = 0; i < 8; i++) {
		phy_package_port_write_paged(phydev, i, RTL838X_PAGE_RAW, RTL8XXX_PAGE_SELECT, RTL8XXX_PAGE_MAIN);
		phy_package_port_write_paged(phydev, i, RTL838X_PAGE_RAW, 0x00, 0x1140);
	}
	mdelay(100);

	/* Request patch */
	for (int i = 0; i < 8; i++) {
		phy_package_port_write_paged(phydev, i, RTL838X_PAGE_RAW, RTL8XXX_PAGE_SELECT, RTL821X_PAGE_PATCH);
		phy_package_port_write_paged(phydev, i, RTL838X_PAGE_RAW, 0x10, 0x0010);
	}

	mdelay(300);

	/* Verify patch readiness */
	for (int i = 0; i < 8; i++) {
		int l;

		for (l = 0; l < 100; l++) {
			val = phy_package_port_read_paged(phydev, i, RTL821X_PAGE_STATE, 0x10);
			if (val & 0x40)
				break;
		}
		if (l >= 100) {
			phydev_err(phydev, "Could not patch PHY\n");
			return -1;
		}
	}

	/* Use Broadcast ID method for patching */
	rtl821x_phy_setup_package_broadcast(phydev, true);

	phy_write_paged(phydev, RTL838X_PAGE_RAW, 30, 8);
	phy_write_paged(phydev, 0x26e, 17, 0xb);
	phy_write_paged(phydev, 0x26e, 16, 0x2);
	mdelay(1);
	ipd = phy_read_paged(phydev, 0x26e, 19);
	phy_write_paged(phydev, 0, 30, 0);
	ipd = (ipd >> 4) & 0xf; /* unused ? */

	for (int i = 0; rtl8218B_6276B_rtl8380_perport[i * 2]; i++) {
		phy_write_paged(phydev, RTL838X_PAGE_RAW, rtl8218B_6276B_rtl8380_perport[i * 2],
		                rtl8218B_6276B_rtl8380_perport[i * 2 + 1]);
	}

	/* Disable broadcast ID */
	rtl821x_phy_setup_package_broadcast(phydev, false);

	return 0;
}

static bool rtl8214fc_media_is_fibre(struct phy_device *phydev)
{
	int mac = phydev->mdio.addr;

	static int reg[] = {16, 19, 20, 21};
	u32 val;

	phy_package_write_paged(phydev, RTL838X_PAGE_RAW, RTL821XINT_MEDIA_PAGE_SELECT, RTL821X_MEDIA_PAGE_INTERNAL);
	val = phy_package_read_paged(phydev, RTL821X_PAGE_PORT, reg[mac % 4]);
	phy_package_write_paged(phydev, RTL838X_PAGE_RAW, RTL821XINT_MEDIA_PAGE_SELECT, RTL821X_MEDIA_PAGE_AUTO);

	if (val & BMCR_PDOWN)
		return false;

	return true;
}

static void rtl8214fc_power_set(struct phy_device *phydev, int port, bool on)
{
	char *state = on ? "on" : "off";

	if (port == PORT_FIBRE) {
		pr_info("%s: Powering %s FIBRE (port %d)\n", __func__, state, phydev->mdio.addr);
		phy_write_paged(phydev, RTL838X_PAGE_RAW, RTL821XINT_MEDIA_PAGE_SELECT, RTL821X_MEDIA_PAGE_FIBRE);
	} else {
		pr_info("%s: Powering %s COPPER (port %d)\n", __func__, state, phydev->mdio.addr);
		phy_write_paged(phydev, RTL838X_PAGE_RAW, RTL821XINT_MEDIA_PAGE_SELECT, RTL821X_MEDIA_PAGE_COPPER);
	}

	if (on) {
		phy_modify_paged(phydev, RTL821X_PAGE_POWER, 0x10, BMCR_PDOWN, 0);
	} else {
		phy_modify_paged(phydev, RTL821X_PAGE_POWER, 0x10, 0, BMCR_PDOWN);
	}

	phy_write_paged(phydev, RTL838X_PAGE_RAW, RTL821XINT_MEDIA_PAGE_SELECT, RTL821X_MEDIA_PAGE_AUTO);
}

static int rtl8214fc_suspend(struct phy_device *phydev)
{
	rtl8214fc_power_set(phydev, PORT_MII, false);
	rtl8214fc_power_set(phydev, PORT_FIBRE, false);

	return 0;
}

static int rtl8214fc_resume(struct phy_device *phydev)
{
	if (rtl8214fc_media_is_fibre(phydev)) {
		rtl8214fc_power_set(phydev, PORT_MII, false);
		rtl8214fc_power_set(phydev, PORT_FIBRE, true);
	} else {
		rtl8214fc_power_set(phydev, PORT_FIBRE, false);
		rtl8214fc_power_set(phydev, PORT_MII, true);
	}

	return 0;
}

static void rtl8214fc_media_set(struct phy_device *phydev, bool set_fibre)
{
	int mac = phydev->mdio.addr;

	static int reg[] = {16, 19, 20, 21};
	int val;

	pr_info("%s: port %d, set_fibre: %d\n", __func__, mac, set_fibre);
	phy_package_write_paged(phydev, RTL838X_PAGE_RAW, RTL821XINT_MEDIA_PAGE_SELECT, RTL821X_MEDIA_PAGE_INTERNAL);
	val = phy_package_read_paged(phydev, RTL821X_PAGE_PORT, reg[mac % 4]);

	val |= BIT(10);
	if (set_fibre) {
		val &= ~BMCR_PDOWN;
	} else {
		val |= BMCR_PDOWN;
	}

	phy_package_write_paged(phydev, RTL838X_PAGE_RAW, RTL821XINT_MEDIA_PAGE_SELECT, RTL821X_MEDIA_PAGE_INTERNAL);
	phy_package_write_paged(phydev, RTL821X_PAGE_PORT, reg[mac % 4], val);
	phy_package_write_paged(phydev, RTL838X_PAGE_RAW, RTL821XINT_MEDIA_PAGE_SELECT, RTL821X_MEDIA_PAGE_AUTO);

	if (!phydev->suspended) {
		if (set_fibre) {
			rtl8214fc_power_set(phydev, PORT_MII, false);
			rtl8214fc_power_set(phydev, PORT_FIBRE, true);
		} else {
			rtl8214fc_power_set(phydev, PORT_FIBRE, false);
			rtl8214fc_power_set(phydev, PORT_MII, true);
		}
	}
}

static int rtl8214fc_set_port(struct phy_device *phydev, int port)
{
	bool is_fibre = (port == PORT_FIBRE ? true : false);
	int addr = phydev->mdio.addr;

	pr_debug("%s port %d to %d\n", __func__, addr, port);

	rtl8214fc_media_set(phydev, is_fibre);

	return 0;
}

static int rtl8214fc_get_port(struct phy_device *phydev)
{
	int addr = phydev->mdio.addr;

	pr_debug("%s: port %d\n", __func__, addr);
	if (rtl8214fc_media_is_fibre(phydev))
		return PORT_FIBRE;

	return PORT_MII;
}

static int rtl8214fc_get_features(struct phy_device *phydev)
{
	int ret = 0;

	ret = genphy_read_abilities(phydev);
	if (ret)
		return ret;
	/*
	 * The RTL8214FC only advertises TP capabilities in the standard registers. This is
	 * independent from what fibre/copper combination is currently activated. For now just
	 * announce the superset of all possible features.
	 */
	linkmode_set_bit(ETHTOOL_LINK_MODE_1000baseX_Full_BIT, phydev->supported);
	linkmode_set_bit(ETHTOOL_LINK_MODE_FIBRE_BIT, phydev->supported);

	return 0;
}

static int rtl8214fc_read_status(struct phy_device *phydev)
{
	bool changed;
	int ret;

	if (rtl8214fc_media_is_fibre(phydev)) {
		phydev->port = PORT_FIBRE;
		ret = genphy_c37_read_status(phydev, &changed);
	} else {
		phydev->port = PORT_MII; /* for now aligend with rest of code */
		ret = genphy_read_status(phydev);
	}

	return ret;
}

static int rtl8214fc_config_aneg(struct phy_device *phydev)
{
	int ret;

	if (rtl8214fc_media_is_fibre(phydev))
		ret = genphy_c37_config_aneg(phydev);
	else
		ret = genphy_config_aneg(phydev);

	return ret;
}

/* Enable EEE on the RTL8218B PHYs
 * The method used is not the preferred way (which would be based on the MAC-EEE state,
 * but the only way that works since the kernel first enables EEE in the MAC
 * and then sets up the PHY. The MAC-based approach would require the oppsite.
 */
void rtl8218d_eee_set(struct phy_device *phydev, bool enable)
{
	u32 val;
	bool an_enabled;

	pr_debug("In %s %d, enable %d\n", __func__, phydev->mdio.addr, enable);
	/* Set GPHY page to copper */
	phy_write_paged(phydev, RTL821X_PAGE_GPHY, RTL821XEXT_MEDIA_PAGE_SELECT, RTL821X_MEDIA_PAGE_COPPER);

	val = phy_read(phydev, MII_BMCR);
	an_enabled = val & BMCR_ANENABLE;

	val = phy_read_mmd(phydev, MDIO_MMD_AN, MDIO_AN_EEE_ADV);
	val |= MDIO_EEE_1000T | MDIO_EEE_100TX;
	phy_write_mmd(phydev, MDIO_MMD_AN, MDIO_AN_EEE_ADV, enable ? (MDIO_EEE_100TX | MDIO_EEE_1000T) : 0);

	/* 500M EEE ability */
	val = phy_read_paged(phydev, RTL821X_PAGE_GPHY, 20);
	if (enable)
		val |= BIT(7);
	else
		val &= ~BIT(7);
	phy_write_paged(phydev, RTL821X_PAGE_GPHY, 20, val);

	/* Restart AN if enabled */
	if (an_enabled) {
		val = phy_read(phydev, MII_BMCR);
		val |= BMCR_ANRESTART;
		phy_write(phydev, MII_BMCR, val);
	}

	/* GPHY page back to auto */
	phy_write_paged(phydev, RTL821X_PAGE_GPHY, RTL821XEXT_MEDIA_PAGE_SELECT, RTL821X_MEDIA_PAGE_AUTO);
}

static int rtl8218b_get_eee(struct phy_device *phydev,
			    struct ethtool_eee *e)
{
	u32 val;
	int addr = phydev->mdio.addr;

	pr_debug("In %s, port %d, was enabled: %d\n", __func__, addr, e->eee_enabled);

	/* Set GPHY page to copper */
	phy_write_paged(phydev, RTL821X_PAGE_GPHY, RTL821XINT_MEDIA_PAGE_SELECT, RTL821X_MEDIA_PAGE_COPPER);

	val = phy_read_paged(phydev, 7, MDIO_AN_EEE_ADV);
	if (e->eee_enabled) {
		/* Verify vs MAC-based EEE */
		e->eee_enabled = !!(val & BIT(7));
		if (!e->eee_enabled) {
			val = phy_read_paged(phydev, RTL821X_PAGE_MAC, 25);
			e->eee_enabled = !!(val & BIT(4));
		}
	}
	pr_debug("%s: enabled: %d\n", __func__, e->eee_enabled);

	/* GPHY page to auto */
	phy_write_paged(phydev, RTL821X_PAGE_GPHY, RTL821XINT_MEDIA_PAGE_SELECT, RTL821X_MEDIA_PAGE_AUTO);

	return 0;
}

static int rtl8218d_get_eee(struct phy_device *phydev,
			    struct ethtool_eee *e)
{
	u32 val;
	int addr = phydev->mdio.addr;

	pr_debug("In %s, port %d, was enabled: %d\n", __func__, addr, e->eee_enabled);

	/* Set GPHY page to copper */
	phy_write_paged(phydev, RTL821X_PAGE_GPHY, RTL821XEXT_MEDIA_PAGE_SELECT, RTL821X_MEDIA_PAGE_COPPER);

	val = phy_read_paged(phydev, 7, MDIO_AN_EEE_ADV);
	if (e->eee_enabled)
		e->eee_enabled = !!(val & BIT(7));
	pr_debug("%s: enabled: %d\n", __func__, e->eee_enabled);

	/* GPHY page to auto */
	phy_write_paged(phydev, RTL821X_PAGE_GPHY, RTL821XEXT_MEDIA_PAGE_SELECT, RTL821X_MEDIA_PAGE_AUTO);

	return 0;
}

static int rtl8214fc_set_eee(struct phy_device *phydev,
			     struct ethtool_eee *e)
{
	u32 poll_state;
	int port = phydev->mdio.addr;
	bool an_enabled;
	u32 val;

	pr_debug("In %s port %d, enabled %d\n", __func__, port, e->eee_enabled);

	if (rtl8214fc_media_is_fibre(phydev)) {
		netdev_err(phydev->attached_dev, "Port %d configured for FIBRE", port);
		return -ENOTSUPP;
	}

	poll_state = disable_polling(port);

	/* Set GPHY page to copper */
	phy_write_paged(phydev, RTL821X_PAGE_GPHY, RTL821XINT_MEDIA_PAGE_SELECT, RTL821X_MEDIA_PAGE_COPPER);

	/* Get auto-negotiation status */
	val = phy_read(phydev, MII_BMCR);
	an_enabled = val & BMCR_ANENABLE;

	pr_info("%s: aneg: %d\n", __func__, an_enabled);
	val = phy_read_paged(phydev, RTL821X_PAGE_MAC, 25);
	val &= ~BIT(5);  /* Use MAC-based EEE */
	phy_write_paged(phydev, RTL821X_PAGE_MAC, 25, val);

	/* Enable 100M (bit 1) / 1000M (bit 2) EEE */
	phy_write_paged(phydev, 7, MDIO_AN_EEE_ADV, e->eee_enabled ? (MDIO_EEE_100TX | MDIO_EEE_1000T) : 0);

	/* 500M EEE ability */
	val = phy_read_paged(phydev, RTL821X_PAGE_GPHY, 20);
	if (e->eee_enabled)
		val |= BIT(7);
	else
		val &= ~BIT(7);

	phy_write_paged(phydev, RTL821X_PAGE_GPHY, 20, val);

	/* Restart AN if enabled */
	if (an_enabled) {
		pr_info("%s: doing aneg\n", __func__);
		val = phy_read(phydev, MII_BMCR);
		val |= BMCR_ANRESTART;
		phy_write(phydev, MII_BMCR, val);
	}

	/* GPHY page back to auto */
	phy_write_paged(phydev, RTL821X_PAGE_GPHY, RTL821XINT_MEDIA_PAGE_SELECT, RTL821X_MEDIA_PAGE_AUTO);

	resume_polling(poll_state);

	return 0;
}

static int rtl8214fc_get_eee(struct phy_device *phydev,
			     struct ethtool_eee *e)
{
	int addr = phydev->mdio.addr;

	pr_debug("In %s port %d, enabled %d\n", __func__, addr, e->eee_enabled);
	if (rtl8214fc_media_is_fibre(phydev)) {
		netdev_err(phydev->attached_dev, "Port %d configured for FIBRE", addr);
		return -ENOTSUPP;
	}

	return rtl8218b_get_eee(phydev, e);
}

static int rtl8218b_set_eee(struct phy_device *phydev, struct ethtool_eee *e)
{
	int port = phydev->mdio.addr;
	u64 poll_state;
	u32 val;
	bool an_enabled;

	pr_info("In %s, port %d, enabled %d\n", __func__, port, e->eee_enabled);

	poll_state = disable_polling(port);

	/* Set GPHY page to copper */
	phy_write(phydev, RTL821XEXT_MEDIA_PAGE_SELECT, RTL821X_MEDIA_PAGE_COPPER);
	val = phy_read(phydev, MII_BMCR);
	an_enabled = val & BMCR_ANENABLE;

	if (e->eee_enabled) {
		/* 100/1000M EEE Capability */
		phy_write(phydev, 13, 0x0007);
		phy_write(phydev, 14, 0x003C);
		phy_write(phydev, 13, 0x4007);
		phy_write(phydev, 14, 0x0006);

		val = phy_read_paged(phydev, RTL821X_PAGE_MAC, 25);
		val |= BIT(4);
		phy_write_paged(phydev, RTL821X_PAGE_MAC, 25, val);
	} else {
		/* 100/1000M EEE Capability */
		phy_write(phydev, 13, 0x0007);
		phy_write(phydev, 14, 0x003C);
		phy_write(phydev, 13, 0x0007);
		phy_write(phydev, 14, 0x0000);

		val = phy_read_paged(phydev, RTL821X_PAGE_MAC, 25);
		val &= ~BIT(4);
		phy_write_paged(phydev, RTL821X_PAGE_MAC, 25, val);
	}

	/* Restart AN if enabled */
	if (an_enabled) {
		val = phy_read(phydev, MII_BMCR);
		val |= BMCR_ANRESTART;
		phy_write(phydev, MII_BMCR, val);
	}

	/* GPHY page back to auto */
	phy_write_paged(phydev, RTL821X_PAGE_GPHY, RTL821XEXT_MEDIA_PAGE_SELECT, RTL821X_MEDIA_PAGE_AUTO);

	pr_info("%s done\n", __func__);
	resume_polling(poll_state);

	return 0;
}

static int rtl8218d_set_eee(struct phy_device *phydev, struct ethtool_eee *e)
{
	int addr = phydev->mdio.addr;
	u64 poll_state;

	pr_info("In %s, port %d, enabled %d\n", __func__, addr, e->eee_enabled);

	poll_state = disable_polling(addr);

	rtl8218d_eee_set(phydev, (bool) e->eee_enabled);

	resume_polling(poll_state);

	return 0;
}

static int rtl8380_configure_rtl8214c(struct phy_device *phydev)
{
	u32 phy_id, val;
	int mac = phydev->mdio.addr;

	val = phy_read(phydev, 2);
	phy_id = val << 16;
	val = phy_read(phydev, 3);
	phy_id |= val;
	pr_debug("Phy on MAC %d: %x\n", mac, phy_id);

	phydev_info(phydev, "Detected external RTL8214C\n");

	/* GPHY auto conf */
	phy_write_paged(phydev, RTL821X_PAGE_GPHY, RTL821XINT_MEDIA_PAGE_SELECT, RTL821X_MEDIA_PAGE_AUTO);

	return 0;
}

static int rtl8380_configure_rtl8214fc(struct phy_device *phydev)
{
	int mac = phydev->mdio.addr;
	struct fw_header *h;
	u32 *rtl8380_rtl8214fc_perchip;
	u32 *rtl8380_rtl8214fc_perport;
	u32 phy_id;
	u32 val;

	val = phy_read(phydev, 2);
	phy_id = val << 16;
	val = phy_read(phydev, 3);
	phy_id |= val;
	pr_debug("Phy on MAC %d: %x\n", mac, phy_id);

	/* Read internal PHY id */
	phy_write_paged(phydev, 0, RTL821XEXT_MEDIA_PAGE_SELECT, RTL821X_MEDIA_PAGE_COPPER);
	phy_write_paged(phydev, 0x1f, 0x1b, 0x0002);
	val = phy_read_paged(phydev, 0x1f, 0x1c);
	if (val != RTL821X_CHIP_ID) {
		phydev_err(phydev, "Expected external RTL8214FC, found PHY-ID %x\n", val);
		return -1;
	}
	phydev_info(phydev, "Detected external RTL8214FC\n");

	h = rtl838x_request_fw(phydev, &rtl838x_8214fc_fw, FIRMWARE_838X_8214FC_1);
	if (!h)
		return -1;

	if (h->phy != 0x8214fc00) {
		phydev_err(phydev, "Wrong firmware file: PHY mismatch.\n");
		return -1;
	}

	rtl8380_rtl8214fc_perchip = (void *)h + sizeof(struct fw_header) + h->parts[0].start;

	rtl8380_rtl8214fc_perport = (void *)h + sizeof(struct fw_header) + h->parts[1].start;

	/* detect phy version */
	phy_write_paged(phydev, RTL838X_PAGE_RAW, 27, 0x0004);
	val = phy_read_paged(phydev, RTL838X_PAGE_RAW, 28);

	val = phy_read(phydev, 16);
	if (val & BMCR_PDOWN)
		rtl8380_rtl8214fc_on_off(phydev, true);
	else
		rtl8380_phy_reset(phydev);

	msleep(100);
	phy_write_paged(phydev, 0, RTL821XEXT_MEDIA_PAGE_SELECT, RTL821X_MEDIA_PAGE_COPPER);

	for (int i = 0; rtl8380_rtl8214fc_perchip[i * 3] &&
	                rtl8380_rtl8214fc_perchip[i * 3 + 1]; i++) {
		u32 page = 0;

		if (rtl8380_rtl8214fc_perchip[i * 3 + 1] == 0x1f)
			page = rtl8380_rtl8214fc_perchip[i * 3 + 2];
		if (rtl8380_rtl8214fc_perchip[i * 3 + 1] == 0x13 && page == 0x260) {
			val = phy_read_paged(phydev, 0x260, 13);
			val = (val & 0x1f00) | (rtl8380_rtl8214fc_perchip[i * 3 + 2] & 0xe0ff);
			phy_write_paged(phydev, RTL838X_PAGE_RAW,
					rtl8380_rtl8214fc_perchip[i * 3 + 1], val);
		} else {
			phy_write_paged(phydev, RTL838X_PAGE_RAW,
					rtl8380_rtl8214fc_perchip[i * 3 + 1],
					rtl8380_rtl8214fc_perchip[i * 3 + 2]);
		}
	}

	/* Force copper medium */
	for (int i = 0; i < 4; i++) {
		phy_package_port_write_paged(phydev, i, RTL838X_PAGE_RAW, RTL8XXX_PAGE_SELECT, RTL8XXX_PAGE_MAIN);
		phy_package_port_write_paged(phydev, i, RTL838X_PAGE_RAW, RTL821XEXT_MEDIA_PAGE_SELECT, RTL821X_MEDIA_PAGE_COPPER);
	}

	/* Enable PHY */
	for (int i = 0; i < 4; i++) {
		phy_package_port_write_paged(phydev, i, RTL838X_PAGE_RAW, RTL8XXX_PAGE_SELECT, RTL8XXX_PAGE_MAIN);
		phy_package_port_write_paged(phydev, i, RTL838X_PAGE_RAW, 0x00, 0x1140);
	}
	mdelay(100);

	/* Disable Autosensing */
	for (int i = 0; i < 4; i++) {
		int l;

		for (l = 0; l < 100; l++) {
			val = phy_package_port_read_paged(phydev, i, RTL821X_PAGE_GPHY, 0x10);
			if ((val & 0x7) >= 3)
				break;
		}
		if (l >= 100) {
			phydev_err(phydev, "Could not disable autosensing\n");
			return -1;
		}
	}

	/* Request patch */
	for (int i = 0; i < 4; i++) {
		phy_package_port_write_paged(phydev, i, RTL838X_PAGE_RAW, RTL8XXX_PAGE_SELECT, RTL821X_PAGE_PATCH);
		phy_package_port_write_paged(phydev, i, RTL838X_PAGE_RAW, 0x10, 0x0010);
	}
	mdelay(300);

	/* Verify patch readiness */
	for (int i = 0; i < 4; i++) {
		int l;

		for (l = 0; l < 100; l++) {
			val = phy_package_port_read_paged(phydev, i, RTL821X_PAGE_STATE, 0x10);
			if (val & 0x40)
				break;
		}
		if (l >= 100) {
			phydev_err(phydev, "Could not patch PHY\n");
			return -1;
		}
	}
	/* Use Broadcast ID method for patching */
	rtl821x_phy_setup_package_broadcast(phydev, true);

	for (int i = 0; rtl8380_rtl8214fc_perport[i * 2]; i++) {
		phy_write_paged(phydev, RTL838X_PAGE_RAW, rtl8380_rtl8214fc_perport[i * 2],
		                rtl8380_rtl8214fc_perport[i * 2 + 1]);
	}

	/* Disable broadcast ID */
	rtl821x_phy_setup_package_broadcast(phydev, false);

	/* Auto medium selection */
	for (int i = 0; i < 4; i++) {
		phy_write_paged(phydev, RTL838X_PAGE_RAW, RTL8XXX_PAGE_SELECT, RTL8XXX_PAGE_MAIN);
		phy_write_paged(phydev, RTL838X_PAGE_RAW, RTL821XEXT_MEDIA_PAGE_SELECT, RTL821X_MEDIA_PAGE_AUTO);
	}

	return 0;
}

static int rtl8380_configure_serdes(struct phy_device *phydev)
{
	u32 v;
	u32 sds_conf_value;
	int i;
	struct fw_header *h;
	u32 *rtl8380_sds_take_reset;
	u32 *rtl8380_sds_common;
	u32 *rtl8380_sds01_qsgmii_6275b;
	u32 *rtl8380_sds23_qsgmii_6275b;
	u32 *rtl8380_sds4_fiber_6275b;
	u32 *rtl8380_sds5_fiber_6275b;
	u32 *rtl8380_sds_reset;
	u32 *rtl8380_sds_release_reset;

	phydev_info(phydev, "Detected internal RTL8380 SERDES\n");

	h = rtl838x_request_fw(phydev, &rtl838x_8218b_fw, FIRMWARE_838X_8380_1);
	if (!h)
		return -1;

	if (h->magic != 0x83808380) {
		phydev_err(phydev, "Wrong firmware file: magic number mismatch.\n");
		return -1;
	}

	rtl8380_sds_take_reset = (void *)h + sizeof(struct fw_header) + h->parts[0].start;

	rtl8380_sds_common = (void *)h + sizeof(struct fw_header) + h->parts[1].start;

	rtl8380_sds01_qsgmii_6275b = (void *)h + sizeof(struct fw_header) + h->parts[2].start;

	rtl8380_sds23_qsgmii_6275b = (void *)h + sizeof(struct fw_header) + h->parts[3].start;

	rtl8380_sds4_fiber_6275b = (void *)h + sizeof(struct fw_header) + h->parts[4].start;

	rtl8380_sds5_fiber_6275b = (void *)h + sizeof(struct fw_header) + h->parts[5].start;

	rtl8380_sds_reset = (void *)h + sizeof(struct fw_header) + h->parts[6].start;

	rtl8380_sds_release_reset = (void *)h + sizeof(struct fw_header) + h->parts[7].start;

	/* Back up serdes power off value */
	sds_conf_value = sw_r32(RTL838X_SDS_CFG_REG);
	pr_info("SDS power down value: %x\n", sds_conf_value);

	/* take serdes into reset */
	i = 0;
	while (rtl8380_sds_take_reset[2 * i]) {
		sw_w32(rtl8380_sds_take_reset[2 * i + 1], rtl8380_sds_take_reset[2 * i]);
		i++;
		udelay(1000);
	}

	/* apply common serdes patch */
	i = 0;
	while (rtl8380_sds_common[2 * i]) {
		sw_w32(rtl8380_sds_common[2 * i + 1], rtl8380_sds_common[2 * i]);
		i++;
		udelay(1000);
	}

	/* internal R/W enable */
	sw_w32(3, RTL838X_INT_RW_CTRL);

	/* SerDes ports 4 and 5 are FIBRE ports */
	sw_w32_mask(0x7 | 0x38, 1 | (1 << 3), RTL838X_INT_MODE_CTRL);

	/* SerDes module settings, SerDes 0-3 are QSGMII */
	v = 0x6 << 25 | 0x6 << 20 | 0x6 << 15 | 0x6 << 10;
	/* SerDes 4 and 5 are 1000BX FIBRE */
	v |= 0x4 << 5 | 0x4;
	sw_w32(v, RTL838X_SDS_MODE_SEL);

	pr_info("PLL control register: %x\n", sw_r32(RTL838X_PLL_CML_CTRL));
	sw_w32_mask(0xfffffff0, 0xaaaaaaaf & 0xf, RTL838X_PLL_CML_CTRL);
	i = 0;
	while (rtl8380_sds01_qsgmii_6275b[2 * i]) {
		sw_w32(rtl8380_sds01_qsgmii_6275b[2 * i + 1],
		       rtl8380_sds01_qsgmii_6275b[2 * i]);
		i++;
	}

	i = 0;
	while (rtl8380_sds23_qsgmii_6275b[2 * i]) {
		sw_w32(rtl8380_sds23_qsgmii_6275b[2 * i + 1], rtl8380_sds23_qsgmii_6275b[2 * i]);
		i++;
	}

	i = 0;
	while (rtl8380_sds4_fiber_6275b[2 * i]) {
		sw_w32(rtl8380_sds4_fiber_6275b[2 * i + 1], rtl8380_sds4_fiber_6275b[2 * i]);
		i++;
	}

	i = 0;
	while (rtl8380_sds5_fiber_6275b[2 * i]) {
		sw_w32(rtl8380_sds5_fiber_6275b[2 * i + 1], rtl8380_sds5_fiber_6275b[2 * i]);
		i++;
	}

	i = 0;
	while (rtl8380_sds_reset[2 * i]) {
		sw_w32(rtl8380_sds_reset[2 * i + 1], rtl8380_sds_reset[2 * i]);
		i++;
	}

	i = 0;
	while (rtl8380_sds_release_reset[2 * i]) {
		sw_w32(rtl8380_sds_release_reset[2 * i + 1], rtl8380_sds_release_reset[2 * i]);
		i++;
	}

	pr_info("SDS power down value now: %x\n", sw_r32(RTL838X_SDS_CFG_REG));
	sw_w32(sds_conf_value, RTL838X_SDS_CFG_REG);

	pr_info("Configuration of SERDES done\n");

	return 0;
}

static int rtl8390_configure_serdes(struct phy_device *phydev)
{
	phydev_info(phydev, "Detected internal RTL8390 SERDES\n");

	/* In autoneg state, force link, set SR4_CFG_EN_LINK_FIB1G */
	sw_w32_mask(0, 1 << 18, RTL839X_SDS12_13_XSG0 + 0x0a);

	/* Disable EEE: Clear FRE16_EEE_RSG_FIB1G, FRE16_EEE_STD_FIB1G,
	 * FRE16_C1_PWRSAV_EN_FIB1G, FRE16_C2_PWRSAV_EN_FIB1G
	 * and FRE16_EEE_QUIET_FIB1G
	 */
	sw_w32_mask(0x1f << 10, 0, RTL839X_SDS12_13_XSG0 + 0xe0);

	return 0;
}

void rtl9300_sds_field_w(int sds, u32 page, u32 reg, int end_bit, int start_bit, u32 v)
{
	int l = end_bit - start_bit + 1;
	u32 data = v;

	if (l < 32) {
		u32 mask = BIT(l) - 1;

		data = rtl930x_read_sds_phy(sds, page, reg);
		data &= ~(mask << start_bit);
		data |= (v & mask) << start_bit;
	}

	rtl930x_write_sds_phy(sds, page, reg, data);
}

u32 rtl9300_sds_field_r(int sds, u32 page, u32 reg, int end_bit, int start_bit)
{
	int l = end_bit - start_bit + 1;
	u32 v = rtl930x_read_sds_phy(sds, page, reg);

	if (l >= 32)
		return v;

	return (v >> start_bit) & (BIT(l) - 1);
}

/* Read the link and speed status of the internal SerDes of the RTL9300
 */
static int rtl9300_read_status(struct phy_device *phydev)
{
	struct device *dev = &phydev->mdio.dev;
	int phy_addr = phydev->mdio.addr;
	struct device_node *dn;
	u32 sds_num = 0, status, latch_status, mode;

	if (dev->of_node) {
		dn = dev->of_node;

		if (of_property_read_u32(dn, "sds", &sds_num))
			sds_num = -1;
		pr_info("%s: Port %d, SerDes is %d\n", __func__, phy_addr, sds_num);
	} else {
		dev_err(dev, "No DT node.\n");
		return -EINVAL;
	}

	if (sds_num < 0)
		return 0;

	mode = rtl9300_sds_mode_get(sds_num);
	pr_info("%s got SDS mode %02x\n", __func__, mode);
	if (mode == RTL930X_SDS_OFF)
		mode = rtl9300_sds_field_r(sds_num, 0x1f, 9, 11, 7);
	if (mode == RTL930X_SDS_MODE_10GBASER) { /* 10GR mode */
		status = rtl9300_sds_field_r(sds_num, 0x5, 0, 12, 12);
		latch_status = rtl9300_sds_field_r(sds_num, 0x4, 1, 2, 2);
		status |= rtl9300_sds_field_r(sds_num, 0x5, 0, 12, 12);
		latch_status |= rtl9300_sds_field_r(sds_num, 0x4, 1, 2, 2);
	} else {
		status = rtl9300_sds_field_r(sds_num, 0x1, 29, 8, 0);
		latch_status = rtl9300_sds_field_r(sds_num, 0x1, 30, 8, 0);
		status |= rtl9300_sds_field_r(sds_num, 0x1, 29, 8, 0);
		latch_status |= rtl9300_sds_field_r(sds_num, 0x1, 30, 8, 0);
	}

	pr_info("%s link status: status: %d, latch %d\n", __func__, status, latch_status);

	if (latch_status) {
		phydev->link = true;
		if (mode == RTL930X_SDS_MODE_10GBASER) {
			phydev->speed = SPEED_10000;
			phydev->interface = PHY_INTERFACE_MODE_10GBASER;
		} else {
			phydev->speed = SPEED_1000;
			phydev->interface = PHY_INTERFACE_MODE_1000BASEX;
		}

		phydev->duplex = DUPLEX_FULL;
	}

	return 0;
}

void rtl930x_sds_rx_rst(int sds_num, phy_interface_t phy_if)
{
	int page = 0x2e; /* 10GR and USXGMII */

	if (phy_if == PHY_INTERFACE_MODE_1000BASEX)
		page = 0x24;

	rtl9300_sds_field_w(sds_num, page, 0x15, 4, 4, 0x1);
	mdelay(5);
	rtl9300_sds_field_w(sds_num, page, 0x15, 4, 4, 0x0);
}

/* Force PHY modes on 10GBit Serdes
 */
void rtl9300_force_sds_mode(int sds, phy_interface_t phy_if)
{
	int lc_value;
	int sds_mode;
	bool lc_on;
	int lane_0 = (sds % 2) ? sds - 1 : sds;
	u32 v;

	pr_info("%s: SDS: %d, mode %d\n", __func__, sds, phy_if);
	switch (phy_if) {
	case PHY_INTERFACE_MODE_SGMII:
		sds_mode = RTL930X_SDS_MODE_SGMII;
		lc_on = false;
		lc_value = 0x1;
		break;

	case PHY_INTERFACE_MODE_HSGMII:
		sds_mode = RTL930X_SDS_MODE_HSGMII;
		lc_value = 0x3;
		/* Configure LC */
		break;

	case PHY_INTERFACE_MODE_1000BASEX:
		sds_mode = RTL930X_SDS_MODE_1000BASEX;
		lc_on = false;
		break;

	case PHY_INTERFACE_MODE_2500BASEX:
		sds_mode = RTL930X_SDS_MODE_2500BASEX;
		lc_value = 0x3;
		/* Configure LC */
		break;

	case PHY_INTERFACE_MODE_10GBASER:
		sds_mode = RTL930X_SDS_MODE_10GBASER;
		lc_on = true;
		lc_value = 0x5;
		break;

	case PHY_INTERFACE_MODE_NA:
		/* This will disable SerDes */
		sds_mode = RTL930X_SDS_OFF;
		break;

	default:
		pr_err("%s: unknown serdes mode: %s\n",
		       __func__, phy_modes(phy_if));
		return;
	}

	pr_info("%s --------------------- serdes %d forcing to %x ...\n", __func__, sds, sds_mode);
	/* Power down SerDes */
	rtl9300_sds_field_w(sds, 0x20, 0, 7, 6, 0x3);
	if (sds == 5) pr_info("%s after %x\n", __func__, rtl930x_read_sds_phy(sds, 0x20, 0));

	if (sds == 5) pr_info("%s a %x\n", __func__, rtl930x_read_sds_phy(sds, 0x1f, 9));
	/* Force mode enable */
	rtl9300_sds_field_w(sds, 0x1f, 9, 6, 6, 0x1);
	if (sds == 5) pr_info("%s b %x\n", __func__, rtl930x_read_sds_phy(sds, 0x1f, 9));

	/* SerDes off */
	rtl9300_sds_field_w(sds, 0x1f, 9, 11, 7, RTL930X_SDS_OFF);

	if (phy_if == PHY_INTERFACE_MODE_NA)
		return;

	if (sds == 5) pr_info("%s c %x\n", __func__, rtl930x_read_sds_phy(sds, 0x20, 18));
	/* Enable LC and ring */
	rtl9300_sds_field_w(lane_0, 0x20, 18, 3, 0, 0xf);

	if (sds == lane_0)
		rtl9300_sds_field_w(lane_0, 0x20, 18, 5, 4, 0x1);
	else
		rtl9300_sds_field_w(lane_0, 0x20, 18, 7, 6, 0x1);

	rtl9300_sds_field_w(sds, 0x20, 0, 5, 4, 0x3);

	if (lc_on)
		rtl9300_sds_field_w(lane_0, 0x20, 18, 11, 8, lc_value);
	else
		rtl9300_sds_field_w(lane_0, 0x20, 18, 15, 12, lc_value);

	/* Force analog LC & ring on */
	rtl9300_sds_field_w(lane_0, 0x21, 11, 3, 0, 0xf);

	v = lc_on ? 0x3 : 0x1;

	if (sds == lane_0)
		rtl9300_sds_field_w(lane_0, 0x20, 18, 5, 4, v);
	else
		rtl9300_sds_field_w(lane_0, 0x20, 18, 7, 6, v);

	/* Force SerDes mode */
	rtl9300_sds_field_w(sds, 0x1f, 9, 6, 6, 1);
	rtl9300_sds_field_w(sds, 0x1f, 9, 11, 7, sds_mode);

	/* Toggle LC or Ring */
	for (int i = 0; i < 20; i++) {
		u32 cr_0, cr_1, cr_2;
		u32 m_bit, l_bit;

		mdelay(200);

		rtl930x_write_sds_phy(lane_0, 0x1f, 2, 53);

		m_bit = (lane_0 == sds) ? (4) : (5);
		l_bit = (lane_0 == sds) ? (4) : (5);

		cr_0 = rtl9300_sds_field_r(lane_0, 0x1f, 20, m_bit, l_bit);
		mdelay(10);
		cr_1 = rtl9300_sds_field_r(lane_0, 0x1f, 20, m_bit, l_bit);
		mdelay(10);
		cr_2 = rtl9300_sds_field_r(lane_0, 0x1f, 20, m_bit, l_bit);

		if (cr_0 && cr_1 && cr_2) {
			u32 t;

			if (phy_if != PHY_INTERFACE_MODE_10GBASER)
				break;

			t = rtl9300_sds_field_r(sds, 0x6, 0x1, 2, 2);
			rtl9300_sds_field_w(sds, 0x6, 0x1, 2, 2, 0x1);

			/* Reset FSM */
			rtl9300_sds_field_w(sds, 0x6, 0x2, 12, 12, 0x1);
			mdelay(10);
			rtl9300_sds_field_w(sds, 0x6, 0x2, 12, 12, 0x0);
			mdelay(10);

			/* Need to read this twice */
			v = rtl9300_sds_field_r(sds, 0x5, 0, 12, 12);
			v = rtl9300_sds_field_r(sds, 0x5, 0, 12, 12);

			rtl9300_sds_field_w(sds, 0x6, 0x1, 2, 2, t);

			/* Reset FSM again */
			rtl9300_sds_field_w(sds, 0x6, 0x2, 12, 12, 0x1);
			mdelay(10);
			rtl9300_sds_field_w(sds, 0x6, 0x2, 12, 12, 0x0);
			mdelay(10);

			if (v == 1)
				break;
		}

		m_bit = (phy_if == PHY_INTERFACE_MODE_10GBASER) ? 3 : 1;
		l_bit = (phy_if == PHY_INTERFACE_MODE_10GBASER) ? 2 : 0;

		rtl9300_sds_field_w(lane_0, 0x21, 11, m_bit, l_bit, 0x2);
		mdelay(10);
		rtl9300_sds_field_w(lane_0, 0x21, 11, m_bit, l_bit, 0x3);
	}

	rtl930x_sds_rx_rst(sds, phy_if);

	/* Re-enable power */
	rtl9300_sds_field_w(sds, 0x20, 0, 7, 6, 0);

	pr_info("%s --------------------- serdes %d forced to %x DONE\n", __func__, sds, sds_mode);
}

void rtl9300_sds_tx_config(int sds, phy_interface_t phy_if)
{
	/* parameters: rtl9303_80G_txParam_s2 */
	int impedance = 0x8;
	int pre_amp = 0x2;
	int main_amp = 0x9;
	int post_amp = 0x2;
	int pre_en = 0x1;
	int post_en = 0x1;
	int page;

	switch(phy_if) {
	case PHY_INTERFACE_MODE_1000BASEX:
		pre_amp = 0x1;
		main_amp = 0x9;
		post_amp = 0x1;
		page = 0x25;
		break;
	case PHY_INTERFACE_MODE_HSGMII:
	case PHY_INTERFACE_MODE_2500BASEX:
		pre_amp = 0;
		post_amp = 0x8;
		pre_en = 0;
		page = 0x29;
		break;
	case PHY_INTERFACE_MODE_10GBASER:
	case PHY_INTERFACE_MODE_USXGMII:
	case PHY_INTERFACE_MODE_XGMII:
		pre_en = 0;
		pre_amp = 0;
		main_amp = 0x10;
		post_amp = 0;
		post_en	= 0;
		page = 0x2f;
		break;
	default:
		pr_err("%s: unsupported PHY mode\n", __func__);
		return;
	}

	rtl9300_sds_field_w(sds, page, 0x01, 15, 11, pre_amp);
	rtl9300_sds_field_w(sds, page, 0x06,  4,  0, post_amp);
	rtl9300_sds_field_w(sds, page, 0x07,  0,  0, pre_en);
	rtl9300_sds_field_w(sds, page, 0x07,  3,  3, post_en);
	rtl9300_sds_field_w(sds, page, 0x07,  8,  4, main_amp);
	rtl9300_sds_field_w(sds, page, 0x18, 15, 12, impedance);
}

/* Wait for clock ready, this assumes the SerDes is in XGMII mode
 * timeout is in ms
 */
int rtl9300_sds_clock_wait(int timeout)
{
	u32 v;
	unsigned long start = jiffies;

	do {
		rtl9300_sds_field_w(2, 0x1f, 0x2, 15, 0, 53);
		v = rtl9300_sds_field_r(2, 0x1f, 20, 5, 4);
		if (v == 3)
			return 0;
	} while (jiffies < start + (HZ / 1000) * timeout);

	return 1;
}

void rtl9300_serdes_mac_link_config(int sds, bool tx_normal, bool rx_normal)
{
	u32 v10, v1;

	v10 = rtl930x_read_sds_phy(sds, 6, 2); /* 10GBit, page 6, reg 2 */
	v1 = rtl930x_read_sds_phy(sds, 0, 0); /* 1GBit, page 0, reg 0 */
	pr_info("%s: registers before %08x %08x\n", __func__, v10, v1);

	v10 &= ~(BIT(13) | BIT(14));
	v1 &= ~(BIT(8) | BIT(9));

	v10 |= rx_normal ? 0 : BIT(13);
	v1 |= rx_normal ? 0 : BIT(9);

	v10 |= tx_normal ? 0 : BIT(14);
	v1 |= tx_normal ? 0 : BIT(8);

	rtl930x_write_sds_phy(sds, 6, 2, v10);
	rtl930x_write_sds_phy(sds, 0, 0, v1);

	v10 = rtl930x_read_sds_phy(sds, 6, 2);
	v1 = rtl930x_read_sds_phy(sds, 0, 0);
	pr_info("%s: registers after %08x %08x\n", __func__, v10, v1);
}

void rtl9300_sds_rxcal_dcvs_manual(u32 sds_num, u32 dcvs_id, bool manual, u32 dvcs_list[])
{
	if (manual) {
		switch(dcvs_id) {
		case 0:
			rtl9300_sds_field_w(sds_num, 0x2e, 0x1e, 14, 14, 0x1);
			rtl9300_sds_field_w(sds_num, 0x2f, 0x03,  5,  5, dvcs_list[0]);
			rtl9300_sds_field_w(sds_num, 0x2f, 0x03,  4,  0, dvcs_list[1]);
			break;
		case 1:
			rtl9300_sds_field_w(sds_num, 0x2e, 0x1e, 13, 13, 0x1);
			rtl9300_sds_field_w(sds_num, 0x2e, 0x1d, 15, 15, dvcs_list[0]);
			rtl9300_sds_field_w(sds_num, 0x2e, 0x1d, 14, 11, dvcs_list[1]);
			break;
		case 2:
			rtl9300_sds_field_w(sds_num, 0x2e, 0x1e, 12, 12, 0x1);
			rtl9300_sds_field_w(sds_num, 0x2e, 0x1d, 10, 10, dvcs_list[0]);
			rtl9300_sds_field_w(sds_num, 0x2e, 0x1d,  9,  6, dvcs_list[1]);
			break;
		case 3:
			rtl9300_sds_field_w(sds_num, 0x2e, 0x1e, 11, 11, 0x1);
			rtl9300_sds_field_w(sds_num, 0x2e, 0x1d,  5,  5, dvcs_list[0]);
			rtl9300_sds_field_w(sds_num, 0x2e, 0x1d,  4,  1, dvcs_list[1]);
			break;
		case 4:
			rtl9300_sds_field_w(sds_num, 0x2e, 0x01, 15, 15, 0x1);
			rtl9300_sds_field_w(sds_num, 0x2e, 0x11, 10, 10, dvcs_list[0]);
			rtl9300_sds_field_w(sds_num, 0x2e, 0x11,  9,  6, dvcs_list[1]);
			break;
		case 5:
			rtl9300_sds_field_w(sds_num, 0x2e, 0x02, 11, 11, 0x1);
			rtl9300_sds_field_w(sds_num, 0x2e, 0x11,  4,  4, dvcs_list[0]);
			rtl9300_sds_field_w(sds_num, 0x2e, 0x11,  3,  0, dvcs_list[1]);
			break;
		default:
			break;
		}
	} else {
		switch(dcvs_id) {
		case 0:
			rtl9300_sds_field_w(sds_num, 0x2e, 0x1e, 14, 14, 0x0);
			break;
		case 1:
			rtl9300_sds_field_w(sds_num, 0x2e, 0x1e, 13, 13, 0x0);
			break;
		case 2:
			rtl9300_sds_field_w(sds_num, 0x2e, 0x1e, 12, 12, 0x0);
			break;
		case 3:
			rtl9300_sds_field_w(sds_num, 0x2e, 0x1e, 11, 11, 0x0);
			break;
		case 4:
			rtl9300_sds_field_w(sds_num, 0x2e, 0x01, 15, 15, 0x0);
			break;
		case 5:
			rtl9300_sds_field_w(sds_num, 0x2e, 0x02, 11, 11, 0x0);
			break;
		default:
			break;
		}
		mdelay(1);
	}
}

void rtl9300_sds_rxcal_dcvs_get(u32 sds_num, u32 dcvs_id, u32 dcvs_list[])
{
	u32 dcvs_sign_out = 0, dcvs_coef_bin = 0;
	bool dcvs_manual;

	if (!(sds_num % 2))
		rtl930x_write_sds_phy(sds_num, 0x1f, 0x2, 0x2f);
	else
		rtl930x_write_sds_phy(sds_num - 1, 0x1f, 0x2, 0x31);

	/* ##Page0x2E, Reg0x15[9], REG0_RX_EN_TEST=[1] */
	rtl9300_sds_field_w(sds_num, 0x2e, 0x15, 9, 9, 0x1);

	/* ##Page0x21, Reg0x06[11 6], REG0_RX_DEBUG_SEL=[1 0 x x x x] */
	rtl9300_sds_field_w(sds_num, 0x21, 0x06, 11, 6, 0x20);

	switch(dcvs_id) {
	case 0:
		rtl9300_sds_field_w(sds_num, 0x2f, 0x0c, 5, 0, 0x22);
		mdelay(1);

		/* ##DCVS0 Read Out */
		dcvs_sign_out = rtl9300_sds_field_r(sds_num, 0x1f, 0x14,  4,  4);
		dcvs_coef_bin = rtl9300_sds_field_r(sds_num, 0x1f, 0x14,  3,  0);
		dcvs_manual = !!rtl9300_sds_field_r(sds_num, 0x2e, 0x1e, 14, 14);
		break;

	case 1:
		rtl9300_sds_field_w(sds_num, 0x2f, 0x0c, 5, 0, 0x23);
		mdelay(1);

		/* ##DCVS0 Read Out */
		dcvs_coef_bin = rtl9300_sds_field_r(sds_num, 0x1f, 0x14,  4,  4);
		dcvs_coef_bin = rtl9300_sds_field_r(sds_num, 0x1f, 0x14,  3,  0);
		dcvs_manual = !!rtl9300_sds_field_r(sds_num, 0x2e, 0x1e, 13, 13);
		break;

	case 2:
		rtl9300_sds_field_w(sds_num, 0x2f, 0x0c, 5, 0, 0x24);
		mdelay(1);

		/* ##DCVS0 Read Out */
		dcvs_sign_out = rtl9300_sds_field_r(sds_num, 0x1f, 0x14,  4,  4);
		dcvs_coef_bin = rtl9300_sds_field_r(sds_num, 0x1f, 0x14,  3,  0);
		dcvs_manual = !!rtl9300_sds_field_r(sds_num, 0x2e, 0x1e, 12, 12);
		break;
	case 3:
		rtl9300_sds_field_w(sds_num, 0x2f, 0x0c, 5, 0, 0x25);
		mdelay(1);

		/* ##DCVS0 Read Out */
		dcvs_sign_out = rtl9300_sds_field_r(sds_num, 0x1f, 0x14,  4,  4);
		dcvs_coef_bin = rtl9300_sds_field_r(sds_num, 0x1f, 0x14,  3,  0);
		dcvs_manual   = rtl9300_sds_field_r(sds_num, 0x2e, 0x1e, 11, 11);
		break;

	case 4:
		rtl9300_sds_field_w(sds_num, 0x2f, 0x0c, 5, 0, 0x2c);
		mdelay(1);

		/* ##DCVS0 Read Out */
		dcvs_sign_out = rtl9300_sds_field_r(sds_num, 0x1f, 0x14,  4,  4);
		dcvs_coef_bin = rtl9300_sds_field_r(sds_num, 0x1f, 0x14,  3,  0);
		dcvs_manual = !!rtl9300_sds_field_r(sds_num, 0x2e, 0x01, 15, 15);
		break;

	case 5:
		rtl9300_sds_field_w(sds_num, 0x2f, 0x0c, 5, 0, 0x2d);
		mdelay(1);

		/* ##DCVS0 Read Out */
		dcvs_sign_out = rtl9300_sds_field_r(sds_num, 0x1f, 0x14,  4,  4);
		dcvs_coef_bin = rtl9300_sds_field_r(sds_num, 0x1f, 0x14,  3,  0);
		dcvs_manual   = rtl9300_sds_field_r(sds_num, 0x2e, 0x02, 11, 11);
		break;

	default:
		break;
	}

	if (dcvs_sign_out)
		pr_info("%s DCVS %u Sign: -", __func__, dcvs_id);
	else
		pr_info("%s DCVS %u Sign: +", __func__, dcvs_id);

	pr_info("DCVS %u even coefficient = %u", dcvs_id, dcvs_coef_bin);
	pr_info("DCVS %u manual = %u", dcvs_id, dcvs_manual);

	dcvs_list[0] = dcvs_sign_out;
	dcvs_list[1] = dcvs_coef_bin;
}

void rtl9300_sds_rxcal_leq_manual(u32 sds_num, bool manual, u32 leq_gray)
{
	if (manual) {
		rtl9300_sds_field_w(sds_num, 0x2e, 0x18, 15, 15, 0x1);
		rtl9300_sds_field_w(sds_num, 0x2e, 0x16, 14, 10, leq_gray);
	} else {
		rtl9300_sds_field_w(sds_num, 0x2e, 0x18, 15, 15, 0x0);
		mdelay(100);
	}
}

void rtl9300_sds_rxcal_leq_offset_manual(u32 sds_num, bool manual, u32 offset)
{
	if (manual) {
		rtl9300_sds_field_w(sds_num, 0x2e, 0x17, 6, 2, offset);
	} else {
		rtl9300_sds_field_w(sds_num, 0x2e, 0x17, 6, 2, offset);
		mdelay(1);
	}
}

#define GRAY_BITS 5
u32 rtl9300_sds_rxcal_gray_to_binary(u32 gray_code)
{
	int i, j, m;
	u32 g[GRAY_BITS];
	u32 c[GRAY_BITS];
	u32 leq_binary = 0;

	for(i = 0; i < GRAY_BITS; i++)
		g[i] = (gray_code & BIT(i)) >> i;

	m = GRAY_BITS - 1;

	c[m] = g[m];

	for(i = 0; i < m; i++) {
		c[i] = g[i];
		for(j  = i + 1; j < GRAY_BITS; j++)
			c[i] = c[i] ^ g[j];
	}

	for(i = 0; i < GRAY_BITS; i++)
		leq_binary += c[i] << i;

	return leq_binary;
}

u32 rtl9300_sds_rxcal_leq_read(int sds_num)
{
	u32 leq_gray, leq_bin;
	bool leq_manual;

	if (!(sds_num % 2))
		rtl930x_write_sds_phy(sds_num, 0x1f, 0x2, 0x2f);
	else
		rtl930x_write_sds_phy(sds_num - 1, 0x1f, 0x2, 0x31);

	/* ##Page0x2E, Reg0x15[9], REG0_RX_EN_TEST=[1] */
	rtl9300_sds_field_w(sds_num, 0x2e, 0x15, 9, 9, 0x1);

	/* ##Page0x21, Reg0x06[11 6], REG0_RX_DEBUG_SEL=[0 1 x x x x] */
	rtl9300_sds_field_w(sds_num, 0x21, 0x06, 11, 6, 0x10);
	mdelay(1);

	/* ##LEQ Read Out */
	leq_gray = rtl9300_sds_field_r(sds_num, 0x1f, 0x14, 7, 3);
	leq_manual = !!rtl9300_sds_field_r(sds_num, 0x2e, 0x18, 15, 15);
	leq_bin = rtl9300_sds_rxcal_gray_to_binary(leq_gray);

	pr_info("LEQ_gray: %u, LEQ_bin: %u", leq_gray, leq_bin);
	pr_info("LEQ manual: %u", leq_manual);

	return leq_bin;
}

void rtl9300_sds_rxcal_vth_manual(u32 sds_num, bool manual, u32 vth_list[])
{
	if (manual) {
		rtl9300_sds_field_w(sds_num, 0x2e, 0x0f, 13, 13, 0x1);
		rtl9300_sds_field_w(sds_num, 0x2e, 0x13,  5,  3, vth_list[0]);
		rtl9300_sds_field_w(sds_num, 0x2e, 0x13,  2,  0, vth_list[1]);
	} else {
		rtl9300_sds_field_w(sds_num, 0x2e, 0x0f, 13, 13, 0x0);
		mdelay(10);
	}
}

void rtl9300_sds_rxcal_vth_get(u32  sds_num, u32 vth_list[])
{
	u32 vth_manual;

	/* ##Page0x1F, Reg0x02[15 0], REG_DBGO_SEL=[0x002F]; */ /* Lane0 */
	/* ##Page0x1F, Reg0x02[15 0], REG_DBGO_SEL=[0x0031]; */ /* Lane1 */
	if (!(sds_num % 2))
		rtl930x_write_sds_phy(sds_num, 0x1f, 0x2, 0x2f);
	else
		rtl930x_write_sds_phy(sds_num - 1, 0x1f, 0x2, 0x31);

	/* ##Page0x2E, Reg0x15[9], REG0_RX_EN_TEST=[1] */
	rtl9300_sds_field_w(sds_num, 0x2e, 0x15, 9, 9, 0x1);
	/* ##Page0x21, Reg0x06[11 6], REG0_RX_DEBUG_SEL=[1 0 x x x x] */
	rtl9300_sds_field_w(sds_num, 0x21, 0x06, 11, 6, 0x20);
	/* ##Page0x2F, Reg0x0C[5 0], REG0_COEF_SEL=[0 0 1 1 0 0] */
	rtl9300_sds_field_w(sds_num, 0x2f, 0x0c, 5, 0, 0xc);

	mdelay(1);

	/* ##VthP & VthN Read Out */
	vth_list[0] = rtl9300_sds_field_r(sds_num, 0x1f, 0x14, 2, 0); /* v_thp set bin */
	vth_list[1] = rtl9300_sds_field_r(sds_num, 0x1f, 0x14, 5, 3); /* v_thn set bin */

	pr_info("vth_set_bin = %d", vth_list[0]);
	pr_info("vth_set_bin = %d", vth_list[1]);

	vth_manual = !!rtl9300_sds_field_r(sds_num, 0x2e, 0x0f, 13, 13);
	pr_info("Vth Maunal = %d", vth_manual);
}

void rtl9300_sds_rxcal_tap_manual(u32 sds_num, int tap_id, bool manual, u32 tap_list[])
{
	if (manual) {
		switch(tap_id) {
		case 0:
			/* ##REG0_LOAD_IN_INIT[0]=1; REG0_TAP0_INIT[5:0]=Tap0_Value */
			rtl9300_sds_field_w(sds_num, 0x2e, 0x0f, tap_id + 7, tap_id + 7, 0x1);
			rtl9300_sds_field_w(sds_num, 0x2f, 0x03, 5, 5, tap_list[0]);
			rtl9300_sds_field_w(sds_num, 0x2f, 0x03, 4, 0, tap_list[1]);
			break;
		case 1:
			rtl9300_sds_field_w(sds_num, 0x2e, 0x0f, tap_id + 7, tap_id + 7, 0x1);
			rtl9300_sds_field_w(sds_num, 0x21, 0x07, 6, 6, tap_list[0]);
			rtl9300_sds_field_w(sds_num, 0x2e, 0x09, 11, 6, tap_list[1]);
			rtl9300_sds_field_w(sds_num, 0x21, 0x07, 5, 5, tap_list[2]);
			rtl9300_sds_field_w(sds_num, 0x2f, 0x12, 5, 0, tap_list[3]);
			break;
		case 2:
			rtl9300_sds_field_w(sds_num, 0x2e, 0x0f, tap_id + 7, tap_id + 7, 0x1);
			rtl9300_sds_field_w(sds_num, 0x2e, 0x09, 5, 5, tap_list[0]);
			rtl9300_sds_field_w(sds_num, 0x2e, 0x09, 4, 0, tap_list[1]);
			rtl9300_sds_field_w(sds_num, 0x2e, 0x0a, 11, 11, tap_list[2]);
			rtl9300_sds_field_w(sds_num, 0x2e, 0x0a, 10, 6, tap_list[3]);
			break;
		case 3:
			rtl9300_sds_field_w(sds_num, 0x2e, 0x0f, tap_id + 7, tap_id + 7, 0x1);
			rtl9300_sds_field_w(sds_num, 0x2e, 0x0a, 5, 5, tap_list[0]);
			rtl9300_sds_field_w(sds_num, 0x2e, 0x0a, 4, 0, tap_list[1]);
			rtl9300_sds_field_w(sds_num, 0x2e, 0x06, 5, 5, tap_list[2]);
			rtl9300_sds_field_w(sds_num, 0x2e, 0x06, 4, 0, tap_list[3]);
			break;
		case 4:
			rtl9300_sds_field_w(sds_num, 0x2e, 0x0f, tap_id + 7, tap_id + 7, 0x1);
			rtl9300_sds_field_w(sds_num, 0x2f, 0x01, 5, 5, tap_list[0]);
			rtl9300_sds_field_w(sds_num, 0x2f, 0x01, 4, 0, tap_list[1]);
			rtl9300_sds_field_w(sds_num, 0x2e, 0x06, 11, 11, tap_list[2]);
			rtl9300_sds_field_w(sds_num, 0x2e, 0x06, 10, 6, tap_list[3]);
			break;
		default:
			break;
		}
	} else {
		rtl9300_sds_field_w(sds_num, 0x2e, 0x0f, tap_id + 7, tap_id + 7, 0x0);
		mdelay(10);
	}
}

void rtl9300_sds_rxcal_tap_get(u32 sds_num, u32 tap_id, u32 tap_list[])
{
	u32 tap0_sign_out;
	u32 tap0_coef_bin;
	u32 tap_sign_out_even;
	u32 tap_coef_bin_even;
	u32 tap_sign_out_odd;
	u32 tap_coef_bin_odd;
	bool tap_manual;

	if (!(sds_num % 2))
		rtl930x_write_sds_phy(sds_num, 0x1f, 0x2, 0x2f);
	else
		rtl930x_write_sds_phy(sds_num - 1, 0x1f, 0x2, 0x31);

	/* ##Page0x2E, Reg0x15[9], REG0_RX_EN_TEST=[1] */
	rtl9300_sds_field_w(sds_num, 0x2e, 0x15, 9, 9, 0x1);
	/* ##Page0x21, Reg0x06[11 6], REG0_RX_DEBUG_SEL=[1 0 x x x x] */
	rtl9300_sds_field_w(sds_num, 0x21, 0x06, 11, 6, 0x20);

	if (!tap_id) {
		/* ##Page0x2F, Reg0x0C[5 0], REG0_COEF_SEL=[0 0 0 0 0 1] */
		rtl9300_sds_field_w(sds_num, 0x2f, 0x0c, 5, 0, 0);
		/* ##Tap1 Even Read Out */
		mdelay(1);
		tap0_sign_out = rtl9300_sds_field_r(sds_num, 0x1f, 0x14, 5, 5);
		tap0_coef_bin = rtl9300_sds_field_r(sds_num, 0x1f, 0x14, 4, 0);

		if (tap0_sign_out == 1)
			pr_info("Tap0 Sign : -");
		else
			pr_info("Tap0 Sign : +");

		pr_info("tap0_coef_bin = %d", tap0_coef_bin);

		tap_list[0] = tap0_sign_out;
		tap_list[1] = tap0_coef_bin;

		tap_manual = !!rtl9300_sds_field_r(sds_num, 0x2e, 0x0f, 7, 7);
		pr_info("tap0 manual = %u",tap_manual);
	} else {
		/* ##Page0x2F, Reg0x0C[5 0], REG0_COEF_SEL=[0 0 0 0 0 1] */
		rtl9300_sds_field_w(sds_num, 0x2f, 0x0c, 5, 0, tap_id);
		mdelay(1);
		/* ##Tap1 Even Read Out */
		tap_sign_out_even = rtl9300_sds_field_r(sds_num, 0x1f, 0x14, 5, 5);
		tap_coef_bin_even = rtl9300_sds_field_r(sds_num, 0x1f, 0x14, 4, 0);

		/* ##Page0x2F, Reg0x0C[5 0], REG0_COEF_SEL=[0 0 0 1 1 0] */
		rtl9300_sds_field_w(sds_num, 0x2f, 0x0c, 5, 0, (tap_id + 5));
		/* ##Tap1 Odd Read Out */
		tap_sign_out_odd = rtl9300_sds_field_r(sds_num, 0x1f, 0x14, 5, 5);
		tap_coef_bin_odd = rtl9300_sds_field_r(sds_num, 0x1f, 0x14, 4, 0);

		if (tap_sign_out_even == 1)
			pr_info("Tap %u even sign: -", tap_id);
		else
			pr_info("Tap %u even sign: +", tap_id);

		pr_info("Tap %u even coefficient = %u", tap_id, tap_coef_bin_even);

		if (tap_sign_out_odd == 1)
			pr_info("Tap %u odd sign: -", tap_id);
		else
			pr_info("Tap %u odd sign: +", tap_id);

		pr_info("Tap %u odd coefficient = %u", tap_id,tap_coef_bin_odd);

		tap_list[0] = tap_sign_out_even;
		tap_list[1] = tap_coef_bin_even;
		tap_list[2] = tap_sign_out_odd;
		tap_list[3] = tap_coef_bin_odd;

		tap_manual = rtl9300_sds_field_r(sds_num, 0x2e, 0x0f, tap_id + 7, tap_id + 7);
		pr_info("tap %u manual = %d",tap_id, tap_manual);
	}
}

void rtl9300_do_rx_calibration_1(int sds, phy_interface_t phy_mode)
{
	/* From both rtl9300_rxCaliConf_serdes_myParam and rtl9300_rxCaliConf_phy_myParam */
	int tap0_init_val = 0x1f; /* Initial Decision Fed Equalizer 0 tap */
	int vth_min       = 0x0;

	pr_info("start_1.1.1 initial value for sds %d\n", sds);
	rtl930x_write_sds_phy(sds, 6,  0, 0);

	/* FGCAL */
	rtl9300_sds_field_w(sds, 0x2e, 0x01, 14, 14, 0x00);
	rtl9300_sds_field_w(sds, 0x2e, 0x1c, 10,  5, 0x20);
	rtl9300_sds_field_w(sds, 0x2f, 0x02,  0,  0, 0x01);

	/* DCVS */
	rtl9300_sds_field_w(sds, 0x2e, 0x1e, 14, 11, 0x00);
	rtl9300_sds_field_w(sds, 0x2e, 0x01, 15, 15, 0x00);
	rtl9300_sds_field_w(sds, 0x2e, 0x02, 11, 11, 0x00);
	rtl9300_sds_field_w(sds, 0x2e, 0x1c,  4,  0, 0x00);
	rtl9300_sds_field_w(sds, 0x2e, 0x1d, 15, 11, 0x00);
	rtl9300_sds_field_w(sds, 0x2e, 0x1d, 10,  6, 0x00);
	rtl9300_sds_field_w(sds, 0x2e, 0x1d,  5,  1, 0x00);
	rtl9300_sds_field_w(sds, 0x2e, 0x02, 10,  6, 0x00);
	rtl9300_sds_field_w(sds, 0x2e, 0x11,  4,  0, 0x00);
	rtl9300_sds_field_w(sds, 0x2f, 0x00,  3,  0, 0x0f);
	rtl9300_sds_field_w(sds, 0x2e, 0x04,  6,  6, 0x01);
	rtl9300_sds_field_w(sds, 0x2e, 0x04,  7,  7, 0x01);

	/* LEQ (Long Term Equivalent signal level) */
	rtl9300_sds_field_w(sds, 0x2e, 0x16, 14,  8, 0x00);

	/* DFE (Decision Fed Equalizer) */
	rtl9300_sds_field_w(sds, 0x2f, 0x03,  5,  0, tap0_init_val);
	rtl9300_sds_field_w(sds, 0x2e, 0x09, 11,  6, 0x00);
	rtl9300_sds_field_w(sds, 0x2e, 0x09,  5,  0, 0x00);
	rtl9300_sds_field_w(sds, 0x2e, 0x0a,  5,  0, 0x00);
	rtl9300_sds_field_w(sds, 0x2f, 0x01,  5,  0, 0x00);
	rtl9300_sds_field_w(sds, 0x2f, 0x12,  5,  0, 0x00);
	rtl9300_sds_field_w(sds, 0x2e, 0x0a, 11,  6, 0x00);
	rtl9300_sds_field_w(sds, 0x2e, 0x06,  5,  0, 0x00);
	rtl9300_sds_field_w(sds, 0x2f, 0x01,  5,  0, 0x00);

	/* Vth */
	rtl9300_sds_field_w(sds, 0x2e, 0x13,  5,  3, 0x07);
	rtl9300_sds_field_w(sds, 0x2e, 0x13,  2,  0, 0x07);
	rtl9300_sds_field_w(sds, 0x2f, 0x0b,  5,  3, vth_min);

	pr_info("end_1.1.1 --\n");

	pr_info("start_1.1.2 Load DFE init. value\n");

	rtl9300_sds_field_w(sds, 0x2e, 0x0f, 13,  7, 0x7f);

	pr_info("end_1.1.2\n");

	pr_info("start_1.1.3 disable LEQ training,enable DFE clock\n");

	rtl9300_sds_field_w(sds, 0x2e, 0x17,  7,  7, 0x00);
	rtl9300_sds_field_w(sds, 0x2e, 0x17,  6,  2, 0x00);
	rtl9300_sds_field_w(sds, 0x2e, 0x0c,  8,  8, 0x00);
	rtl9300_sds_field_w(sds, 0x2e, 0x0b,  4,  4, 0x01);
	rtl9300_sds_field_w(sds, 0x2e, 0x12, 14, 14, 0x00);
	rtl9300_sds_field_w(sds, 0x2f, 0x02, 15, 15, 0x00);

	pr_info("end_1.1.3 --\n");

	pr_info("start_1.1.4 offset cali setting\n");

	rtl9300_sds_field_w(sds, 0x2e, 0x0f, 15, 14, 0x03);

	pr_info("end_1.1.4\n");

	pr_info("start_1.1.5 LEQ and DFE setting\n");

	/* TODO: make this work for DAC cables of different lengths */
	/* For a 10GBit serdes wit Fibre, SDS 8 or 9 */
	if (phy_mode == PHY_INTERFACE_MODE_10GBASER || PHY_INTERFACE_MODE_1000BASEX)
		rtl9300_sds_field_w(sds, 0x2e, 0x16,  3,  2, 0x02);
	else
		pr_err("%s not PHY-based or SerDes, implement DAC!\n", __func__);

	/* No serdes, check for Aquantia PHYs */
	rtl9300_sds_field_w(sds, 0x2e, 0x16,  3,  2, 0x02);

	rtl9300_sds_field_w(sds, 0x2e, 0x0f,  6,  0, 0x5f);
	rtl9300_sds_field_w(sds, 0x2f, 0x05,  7,  2, 0x1f);
	rtl9300_sds_field_w(sds, 0x2e, 0x19,  9,  5, 0x1f);
	rtl9300_sds_field_w(sds, 0x2f, 0x0b, 15,  9, 0x3c);
	rtl9300_sds_field_w(sds, 0x2e, 0x0b,  1,  0, 0x03);

	pr_info("end_1.1.5\n");
}

void rtl9300_do_rx_calibration_2_1(u32 sds_num)
{
	pr_info("start_1.2.1 ForegroundOffsetCal_Manual\n");

	/* Gray config endis to 1 */
	rtl9300_sds_field_w(sds_num, 0x2f, 0x02,  2,  2, 0x01);

	/* ForegroundOffsetCal_Manual(auto mode) */
	rtl9300_sds_field_w(sds_num, 0x2e, 0x01, 14, 14, 0x00);

	pr_info("end_1.2.1");
}

void rtl9300_do_rx_calibration_2_2(int sds_num)
{
	/* Force Rx-Run = 0 */
	rtl9300_sds_field_w(sds_num, 0x2e, 0x15, 8, 8, 0x0);

	rtl930x_sds_rx_rst(sds_num, PHY_INTERFACE_MODE_10GBASER);
}

void rtl9300_do_rx_calibration_2_3(int sds_num)
{
	u32 fgcal_binary, fgcal_gray;
	u32 offset_range;

	pr_info("start_1.2.3 Foreground Calibration\n");

	while(1) {
		if (!(sds_num % 2))
			rtl930x_write_sds_phy(sds_num, 0x1f, 0x2, 0x2f);
		else
			rtl930x_write_sds_phy(sds_num -1 , 0x1f, 0x2, 0x31);

		/* ##Page0x2E, Reg0x15[9], REG0_RX_EN_TEST=[1] */
		rtl9300_sds_field_w(sds_num, 0x2e, 0x15, 9, 9, 0x1);
		/* ##Page0x21, Reg0x06[11 6], REG0_RX_DEBUG_SEL=[1 0 x x x x] */
		rtl9300_sds_field_w(sds_num, 0x21, 0x06, 11, 6, 0x20);
		/* ##Page0x2F, Reg0x0C[5 0], REG0_COEF_SEL=[0 0 1 1 1 1] */
		rtl9300_sds_field_w(sds_num, 0x2f, 0x0c, 5, 0, 0xf);
		/* ##FGCAL read gray */
		fgcal_gray = rtl9300_sds_field_r(sds_num, 0x1f, 0x14, 5, 0);
		/* ##Page0x2F, Reg0x0C[5 0], REG0_COEF_SEL=[0 0 1 1 1 0] */
		rtl9300_sds_field_w(sds_num, 0x2f, 0x0c, 5, 0, 0xe);
		/* ##FGCAL read binary */
		fgcal_binary = rtl9300_sds_field_r(sds_num, 0x1f, 0x14, 5, 0);

		pr_info("%s: fgcal_gray: %d, fgcal_binary %d\n",
		        __func__, fgcal_gray, fgcal_binary);

		offset_range = rtl9300_sds_field_r(sds_num, 0x2e, 0x15, 15, 14);

		if (fgcal_binary > 60 || fgcal_binary < 3) {
			if (offset_range == 3) {
				pr_info("%s: Foreground Calibration result marginal!", __func__);
				break;
			} else {
				offset_range++;
				rtl9300_sds_field_w(sds_num, 0x2e, 0x15, 15, 14, offset_range);
				rtl9300_do_rx_calibration_2_2(sds_num);
			}
		} else {
			break;
		}
	}
	pr_info("%s: end_1.2.3\n", __func__);
}

void rtl9300_do_rx_calibration_2(int sds)
{
	rtl930x_sds_rx_rst(sds, PHY_INTERFACE_MODE_10GBASER);
	rtl9300_do_rx_calibration_2_1(sds);
	rtl9300_do_rx_calibration_2_2(sds);
	rtl9300_do_rx_calibration_2_3(sds);
}

void rtl9300_sds_rxcal_3_1(int sds_num, phy_interface_t phy_mode)
{
	pr_info("start_1.3.1");

	/* ##1.3.1 */
	if (phy_mode != PHY_INTERFACE_MODE_10GBASER && phy_mode != PHY_INTERFACE_MODE_1000BASEX)
		rtl9300_sds_field_w(sds_num, 0x2e, 0xc, 8, 8, 0);

	rtl9300_sds_field_w(sds_num, 0x2e, 0x17, 7, 7, 0x0);
	rtl9300_sds_rxcal_leq_manual(sds_num, false, 0);

	pr_info("end_1.3.1");
}

void rtl9300_sds_rxcal_3_2(int sds_num, phy_interface_t phy_mode)
{
	u32 sum10 = 0, avg10, int10;
	int dac_long_cable_offset;
	bool eq_hold_enabled;
	int i;

	if (phy_mode == PHY_INTERFACE_MODE_10GBASER || phy_mode == PHY_INTERFACE_MODE_1000BASEX) {
		/* rtl9300_rxCaliConf_serdes_myParam */
		dac_long_cable_offset = 3;
		eq_hold_enabled = true;
	} else {
		/* rtl9300_rxCaliConf_phy_myParam */
		dac_long_cable_offset = 0;
		eq_hold_enabled = false;
	}

	if (phy_mode == PHY_INTERFACE_MODE_1000BASEX)
		pr_warn("%s: LEQ only valid for 10GR!\n", __func__);

	pr_info("start_1.3.2");

	for(i = 0; i < 10; i++) {
		sum10 += rtl9300_sds_rxcal_leq_read(sds_num);
		mdelay(10);
	}

	avg10 = (sum10 / 10) + (((sum10 % 10) >= 5) ? 1 : 0);
	int10 = sum10 / 10;

	pr_info("sum10:%u, avg10:%u, int10:%u", sum10, avg10, int10);

	if (phy_mode == PHY_INTERFACE_MODE_10GBASER || phy_mode == PHY_INTERFACE_MODE_1000BASEX) {
		if (dac_long_cable_offset) {
			rtl9300_sds_rxcal_leq_offset_manual(sds_num, 1, dac_long_cable_offset);
			rtl9300_sds_field_w(sds_num, 0x2e, 0x17, 7, 7, eq_hold_enabled);
			if (phy_mode == PHY_INTERFACE_MODE_10GBASER)
				rtl9300_sds_rxcal_leq_manual(sds_num, true, avg10);
		} else {
			if (sum10 >= 5) {
				rtl9300_sds_rxcal_leq_offset_manual(sds_num, 1, 3);
				rtl9300_sds_field_w(sds_num, 0x2e, 0x17, 7, 7, 0x1);
				if (phy_mode == PHY_INTERFACE_MODE_10GBASER)
					rtl9300_sds_rxcal_leq_manual(sds_num, true, avg10);
			} else {
				rtl9300_sds_rxcal_leq_offset_manual(sds_num, 1, 0);
				rtl9300_sds_field_w(sds_num, 0x2e, 0x17, 7, 7, 0x1);
				if (phy_mode == PHY_INTERFACE_MODE_10GBASER)
					rtl9300_sds_rxcal_leq_manual(sds_num, true, avg10);
			}
		}
	}

	pr_info("Sds:%u LEQ = %u",sds_num, rtl9300_sds_rxcal_leq_read(sds_num));

	pr_info("end_1.3.2");
}

void rtl9300_do_rx_calibration_3(int sds_num, phy_interface_t phy_mode)
{
	rtl9300_sds_rxcal_3_1(sds_num, phy_mode);

	if (phy_mode == PHY_INTERFACE_MODE_10GBASER || phy_mode == PHY_INTERFACE_MODE_1000BASEX)
		rtl9300_sds_rxcal_3_2(sds_num, phy_mode);
}

void rtl9300_do_rx_calibration_4_1(int sds_num)
{
	u32 vth_list[2] = {0, 0};
	u32 tap0_list[4] = {0, 0, 0, 0};

	pr_info("start_1.4.1");

	/* ##1.4.1 */
	rtl9300_sds_rxcal_vth_manual(sds_num, false, vth_list);
	rtl9300_sds_rxcal_tap_manual(sds_num, 0, false, tap0_list);
	mdelay(200);

	pr_info("end_1.4.1");
}

void rtl9300_do_rx_calibration_4_2(u32 sds_num)
{
	u32 vth_list[2];
	u32 tap_list[4];

	pr_info("start_1.4.2");

	rtl9300_sds_rxcal_vth_get(sds_num, vth_list);
	rtl9300_sds_rxcal_vth_manual(sds_num, true, vth_list);

	mdelay(100);

	rtl9300_sds_rxcal_tap_get(sds_num, 0, tap_list);
	rtl9300_sds_rxcal_tap_manual(sds_num, 0, true, tap_list);

	pr_info("end_1.4.2");
}

void rtl9300_do_rx_calibration_4(u32 sds_num)
{
	rtl9300_do_rx_calibration_4_1(sds_num);
	rtl9300_do_rx_calibration_4_2(sds_num);
}

void rtl9300_do_rx_calibration_5_2(u32 sds_num)
{
	u32 tap1_list[4] = {0};
	u32 tap2_list[4] = {0};
	u32 tap3_list[4] = {0};
	u32 tap4_list[4] = {0};

	pr_info("start_1.5.2");

	rtl9300_sds_rxcal_tap_manual(sds_num, 1, false, tap1_list);
	rtl9300_sds_rxcal_tap_manual(sds_num, 2, false, tap2_list);
	rtl9300_sds_rxcal_tap_manual(sds_num, 3, false, tap3_list);
	rtl9300_sds_rxcal_tap_manual(sds_num, 4, false, tap4_list);

	mdelay(30);

	pr_info("end_1.5.2");
}

void rtl9300_do_rx_calibration_5(u32 sds_num, phy_interface_t phy_mode)
{
	if (phy_mode == PHY_INTERFACE_MODE_10GBASER) /* dfeTap1_4Enable true */
		rtl9300_do_rx_calibration_5_2(sds_num);
}


void rtl9300_do_rx_calibration_dfe_disable(u32 sds_num)
{
	u32 tap1_list[4] = {0};
	u32 tap2_list[4] = {0};
	u32 tap3_list[4] = {0};
	u32 tap4_list[4] = {0};

	rtl9300_sds_rxcal_tap_manual(sds_num, 1, true, tap1_list);
	rtl9300_sds_rxcal_tap_manual(sds_num, 2, true, tap2_list);
	rtl9300_sds_rxcal_tap_manual(sds_num, 3, true, tap3_list);
	rtl9300_sds_rxcal_tap_manual(sds_num, 4, true, tap4_list);

	mdelay(10);
}

void rtl9300_do_rx_calibration(int sds, phy_interface_t phy_mode)
{
	u32 latch_sts;

	rtl9300_do_rx_calibration_1(sds, phy_mode);
	rtl9300_do_rx_calibration_2(sds);
	rtl9300_do_rx_calibration_4(sds);
	rtl9300_do_rx_calibration_5(sds, phy_mode);
	mdelay(20);

	/* Do this only for 10GR mode, SDS active in mode 0x1a */
	if (rtl9300_sds_field_r(sds, 0x1f, 9, 11, 7) == RTL930X_SDS_MODE_10GBASER) {
		pr_info("%s: SDS enabled\n", __func__);
		latch_sts = rtl9300_sds_field_r(sds, 0x4, 1, 2, 2);
		mdelay(1);
		latch_sts = rtl9300_sds_field_r(sds, 0x4, 1, 2, 2);
		if (latch_sts) {
			rtl9300_do_rx_calibration_dfe_disable(sds);
			rtl9300_do_rx_calibration_4(sds);
			rtl9300_do_rx_calibration_5(sds, phy_mode);
		}
	}
}

int rtl9300_sds_sym_err_reset(int sds_num, phy_interface_t phy_mode)
{
	switch (phy_mode) {
	case PHY_INTERFACE_MODE_XGMII:
		break;

	case PHY_INTERFACE_MODE_10GBASER:
		/* Read twice to clear */
		rtl930x_read_sds_phy(sds_num, 5, 1);
		rtl930x_read_sds_phy(sds_num, 5, 1);
		break;

	case PHY_INTERFACE_MODE_1000BASEX:
		rtl9300_sds_field_w(sds_num, 0x1, 24, 2, 0, 0);
		rtl9300_sds_field_w(sds_num, 0x1, 3, 15, 8, 0);
		rtl9300_sds_field_w(sds_num, 0x1, 2, 15, 0, 0);
		break;

	default:
		pr_info("%s unsupported phy mode\n", __func__);
		return -1;
	}

	return 0;
}

u32 rtl9300_sds_sym_err_get(int sds_num, phy_interface_t phy_mode)
{
	u32 v = 0;

	switch (phy_mode) {
	case PHY_INTERFACE_MODE_XGMII:
		break;

	case PHY_INTERFACE_MODE_1000BASEX:
	case PHY_INTERFACE_MODE_10GBASER:
		v = rtl930x_read_sds_phy(sds_num, 5, 1);
		return v & 0xff;

	default:
		pr_info("%s unsupported PHY-mode\n", __func__);
	}

	return v;
}

int rtl9300_sds_check_calibration(int sds_num, phy_interface_t phy_mode)
{
	u32 errors1, errors2;

	rtl9300_sds_sym_err_reset(sds_num, phy_mode);
	rtl9300_sds_sym_err_reset(sds_num, phy_mode);

	/* Count errors during 1ms */
	errors1 = rtl9300_sds_sym_err_get(sds_num, phy_mode);
	mdelay(1);
	errors2 = rtl9300_sds_sym_err_get(sds_num, phy_mode);

	switch (phy_mode) {
	case PHY_INTERFACE_MODE_1000BASEX:
	case PHY_INTERFACE_MODE_XGMII:
		if ((errors2 - errors1 > 100) ||
		    (errors1 >= 0xffff00) || (errors2 >= 0xffff00)) {
			pr_info("%s XSGMII error rate too high\n", __func__);
			return 1;
		}
		break;
	case PHY_INTERFACE_MODE_10GBASER:
		if (errors2 > 0) {
			pr_info("%s 10GBASER error rate too high\n", __func__);
			return 1;
		}
		break;
	default:
		return 1;
	}

	return 0;
}

void rtl9300_phy_enable_10g_1g(int sds_num)
{
	u32 v;

	/* Enable 1GBit PHY */
	v = rtl930x_read_sds_phy(sds_num, PHY_PAGE_2, MII_BMCR);
	pr_info("%s 1gbit phy: %08x\n", __func__, v);
	v &= ~BMCR_PDOWN;
	rtl930x_write_sds_phy(sds_num, PHY_PAGE_2, MII_BMCR, v);
	pr_info("%s 1gbit phy enabled: %08x\n", __func__, v);

	/* Enable 10GBit PHY */
	v = rtl930x_read_sds_phy(sds_num, PHY_PAGE_4, MII_BMCR);
	pr_info("%s 10gbit phy: %08x\n", __func__, v);
	v &= ~BMCR_PDOWN;
	rtl930x_write_sds_phy(sds_num, PHY_PAGE_4, MII_BMCR, v);
	pr_info("%s 10gbit phy after: %08x\n", __func__, v);

	/* dal_longan_construct_mac_default_10gmedia_fiber */
	v = rtl930x_read_sds_phy(sds_num, 0x1f, 11);
	pr_info("%s set medium: %08x\n", __func__, v);
	v |= BIT(1);
	rtl930x_write_sds_phy(sds_num, 0x1f, 11, v);
	pr_info("%s set medium after: %08x\n", __func__, v);
}

static int rtl9300_sds_10g_idle(int sds_num);
static void rtl9300_serdes_patch(int sds_num);

#define RTL930X_MAC_FORCE_MODE_CTRL		(0xCA1C)
int rtl9300_serdes_setup(int port, int sds_num, phy_interface_t phy_mode)
{
	int calib_tries = 0;

	/* Turn Off Serdes */
	rtl9300_sds_rst(sds_num, RTL930X_SDS_OFF);

	/* Apply serdes patches */
	rtl9300_serdes_patch(sds_num);

	/* Maybe use dal_longan_sds_init */

	/* dal_longan_construct_serdesConfig_init */ /* Serdes Construct */
	rtl9300_phy_enable_10g_1g(sds_num);

	/* Disable MAC */
	sw_w32_mask(0, 1, RTL930X_MAC_FORCE_MODE_CTRL + 4 * port);
	mdelay(20);

	/* ----> dal_longan_sds_mode_set */
	pr_info("%s: Configuring RTL9300 SERDES %d\n", __func__, sds_num);

	/* Configure link to MAC */
	rtl9300_serdes_mac_link_config(sds_num, true, true);	/* MAC Construct */

	/* Re-Enable MAC */
	sw_w32_mask(1, 0, RTL930X_MAC_FORCE_MODE_CTRL + 4 * port);

	/* Enable SDS in desired mode */
	rtl9300_force_sds_mode(sds_num, phy_mode);

	/* Enable Fiber RX */
	rtl9300_sds_field_w(sds_num, 0x20, 2, 12, 12, 0);

	/* Calibrate SerDes receiver in loopback mode */
	rtl9300_sds_10g_idle(sds_num);
	do {
		rtl9300_do_rx_calibration(sds_num, phy_mode);
		calib_tries++;
		mdelay(50);
	} while (rtl9300_sds_check_calibration(sds_num, phy_mode) && calib_tries < 3);
	if (calib_tries >= 3)
		pr_warn("%s: SerDes RX calibration failed\n", __func__);

	/* Leave loopback mode */
	rtl9300_sds_tx_config(sds_num, phy_mode);

	return 0;
}

static int rtl9300_sds_10g_idle(int sds_num)
{
	bool busy;
	int i = 0;

	do {
		if (sds_num % 2) {
			rtl9300_sds_field_w(sds_num - 1, 0x1f, 0x2, 15, 0, 53);
			busy = !!rtl9300_sds_field_r(sds_num - 1, 0x1f, 0x14, 1, 1);
		} else {
			rtl9300_sds_field_w(sds_num, 0x1f, 0x2, 15, 0, 53);
			busy = !!rtl9300_sds_field_r(sds_num, 0x1f, 0x14, 0, 0);
		}
		i++;
	} while (busy && i < 100);

	if (i < 100)
		return 0;

	pr_warn("%s WARNING: Waiting for RX idle timed out, SDS %d\n", __func__, sds_num);
	return -EIO;
}

typedef struct {
	u8 page;
	u8 reg;
	u16 data;
} sds_config;

sds_config rtl9300_a_sds_10gr_lane0[] =
{
	/* 1G */
	{0x00, 0x0E, 0x3053}, {0x01, 0x14, 0x0100}, {0x21, 0x03, 0x8206},
	{0x21, 0x05, 0x40B0}, {0x21, 0x06, 0x0010}, {0x21, 0x07, 0xF09F},
	{0x21, 0x0C, 0x0007}, {0x21, 0x0D, 0x6009}, {0x21, 0x0E, 0x0000},
	{0x21, 0x0F, 0x0008}, {0x24, 0x00, 0x0668}, {0x24, 0x02, 0xD020},
	{0x24, 0x06, 0xC000}, {0x24, 0x0B, 0x1892}, {0x24, 0x0F, 0xFFDF},
	{0x24, 0x12, 0x03C4}, {0x24, 0x13, 0x027F}, {0x24, 0x14, 0x1311},
	{0x24, 0x16, 0x00C9}, {0x24, 0x17, 0xA100}, {0x24, 0x1A, 0x0001},
	{0x24, 0x1C, 0x0400}, {0x25, 0x01, 0x0300}, {0x25, 0x02, 0x1017},
	{0x25, 0x03, 0xFFDF}, {0x25, 0x05, 0x7F7C}, {0x25, 0x07, 0x8100},
	{0x25, 0x08, 0x0001}, {0x25, 0x09, 0xFFD4}, {0x25, 0x0A, 0x7C2F},
	{0x25, 0x0E, 0x003F}, {0x25, 0x0F, 0x0121}, {0x25, 0x10, 0x0020},
	{0x25, 0x11, 0x8840}, {0x2B, 0x13, 0x0050}, {0x2B, 0x18, 0x8E88},
	{0x2B, 0x19, 0x4902}, {0x2B, 0x1D, 0x2501}, {0x2D, 0x13, 0x0050},
	{0x2D, 0x18, 0x8E88}, {0x2D, 0x19, 0x4902}, {0x2D, 0x1D, 0x2641},
	{0x2F, 0x13, 0x0050}, {0x2F, 0x18, 0x8E88}, {0x2F, 0x19, 0x4902},
	{0x2F, 0x1D, 0x66E1},
	/* 3.125G */
	{0x28, 0x00, 0x0668}, {0x28, 0x02, 0xD020}, {0x28, 0x06, 0xC000},
	{0x28, 0x0B, 0x1892}, {0x28, 0x0F, 0xFFDF}, {0x28, 0x12, 0x01C4},
	{0x28, 0x13, 0x027F}, {0x28, 0x14, 0x1311}, {0x28, 0x16, 0x00C9},
	{0x28, 0x17, 0xA100}, {0x28, 0x1A, 0x0001}, {0x28, 0x1C, 0x0400},
	{0x29, 0x01, 0x0300}, {0x29, 0x02, 0x1017}, {0x29, 0x03, 0xFFDF},
	{0x29, 0x05, 0x7F7C}, {0x29, 0x07, 0x8100}, {0x29, 0x08, 0x0001},
	{0x29, 0x09, 0xFFD4}, {0x29, 0x0A, 0x7C2F}, {0x29, 0x0E, 0x003F},
	{0x29, 0x0F, 0x0121}, {0x29, 0x10, 0x0020}, {0x29, 0x11, 0x8840},
	/* 10G */
	{0x06, 0x0D, 0x0F00}, {0x06, 0x00, 0x0000}, {0x06, 0x01, 0xC800},
	{0x21, 0x03, 0x8206}, {0x21, 0x05, 0x40B0}, {0x21, 0x06, 0x0010},
	{0x21, 0x07, 0xF09F}, {0x21, 0x0C, 0x0007}, {0x21, 0x0D, 0x6009},
	{0x21, 0x0E, 0x0000}, {0x21, 0x0F, 0x0008}, {0x2E, 0x00, 0xA668},
	{0x2E, 0x02, 0xD020}, {0x2E, 0x06, 0xC000}, {0x2E, 0x0B, 0x1892},
	{0x2E, 0x0F, 0xFFDF}, {0x2E, 0x11, 0x8280}, {0x2E, 0x12, 0x0044},
	{0x2E, 0x13, 0x027F}, {0x2E, 0x14, 0x1311}, {0x2E, 0x17, 0xA100},
	{0x2E, 0x1A, 0x0001}, {0x2E, 0x1C, 0x0400}, {0x2F, 0x01, 0x0300},
	{0x2F, 0x02, 0x1217}, {0x2F, 0x03, 0xFFDF}, {0x2F, 0x05, 0x7F7C},
	{0x2F, 0x07, 0x80C4}, {0x2F, 0x08, 0x0001}, {0x2F, 0x09, 0xFFD4},
	{0x2F, 0x0A, 0x7C2F}, {0x2F, 0x0E, 0x003F}, {0x2F, 0x0F, 0x0121},
	{0x2F, 0x10, 0x0020}, {0x2F, 0x11, 0x8840}, {0x2F, 0x14, 0xE008},
	{0x2B, 0x13, 0x0050}, {0x2B, 0x18, 0x8E88}, {0x2B, 0x19, 0x4902},
	{0x2B, 0x1D, 0x2501}, {0x2D, 0x13, 0x0050}, {0x2D, 0x17, 0x4109},
	{0x2D, 0x18, 0x8E88}, {0x2D, 0x19, 0x4902}, {0x2D, 0x1C, 0x1109},
	{0x2D, 0x1D, 0x2641}, {0x2F, 0x13, 0x0050}, {0x2F, 0x18, 0x8E88},
	{0x2F, 0x19, 0x4902}, {0x2F, 0x1D, 0x76E1},
};

sds_config rtl9300_a_sds_10gr_lane1[] =
{
	/* 1G */
	{0x00, 0x0E, 0x3053}, {0x01, 0x14, 0x0100}, {0x21, 0x03, 0x8206},
	{0x21, 0x06, 0x0010}, {0x21, 0x07, 0xF09F}, {0x21, 0x0A, 0x0003},
	{0x21, 0x0B, 0x0005}, {0x21, 0x0C, 0x0007}, {0x21, 0x0D, 0x6009},
	{0x21, 0x0E, 0x0000}, {0x21, 0x0F, 0x0008}, {0x24, 0x00, 0x0668},
	{0x24, 0x02, 0xD020}, {0x24, 0x06, 0xC000}, {0x24, 0x0B, 0x1892},
	{0x24, 0x0F, 0xFFDF}, {0x24, 0x12, 0x03C4}, {0x24, 0x13, 0x027F},
	{0x24, 0x14, 0x1311}, {0x24, 0x16, 0x00C9}, {0x24, 0x17, 0xA100},
	{0x24, 0x1A, 0x0001}, {0x24, 0x1C, 0x0400}, {0x25, 0x00, 0x820F},
	{0x25, 0x01, 0x0300}, {0x25, 0x02, 0x1017}, {0x25, 0x03, 0xFFDF},
	{0x25, 0x05, 0x7F7C}, {0x25, 0x07, 0x8100}, {0x25, 0x08, 0x0001},
	{0x25, 0x09, 0xFFD4}, {0x25, 0x0A, 0x7C2F}, {0x25, 0x0E, 0x003F},
	{0x25, 0x0F, 0x0121}, {0x25, 0x10, 0x0020}, {0x25, 0x11, 0x8840},
	{0x2B, 0x13, 0x3D87}, {0x2B, 0x14, 0x3108}, {0x2D, 0x13, 0x3C87},
	{0x2D, 0x14, 0x1808},
	/* 3.125G */
	{0x28, 0x00, 0x0668}, {0x28, 0x02, 0xD020}, {0x28, 0x06, 0xC000},
	{0x28, 0x0B, 0x1892}, {0x28, 0x0F, 0xFFDF}, {0x28, 0x12, 0x01C4},
	{0x28, 0x13, 0x027F}, {0x28, 0x14, 0x1311}, {0x28, 0x16, 0x00C9},
	{0x28, 0x17, 0xA100}, {0x28, 0x1A, 0x0001}, {0x28, 0x1C, 0x0400},
	{0x29, 0x00, 0x820F}, {0x29, 0x01, 0x0300}, {0x29, 0x02, 0x1017},
	{0x29, 0x03, 0xFFDF}, {0x29, 0x05, 0x7F7C}, {0x29, 0x07, 0x8100},
	{0x29, 0x08, 0x0001}, {0x29, 0x0A, 0x7C2F}, {0x29, 0x0E, 0x003F},
	{0x29, 0x0F, 0x0121}, {0x29, 0x10, 0x0020}, {0x29, 0x11, 0x8840},
	/* 10G */
	{0x06, 0x0D, 0x0F00}, {0x06, 0x00, 0x0000}, {0x06, 0x01, 0xC800},
	{0x21, 0x03, 0x8206}, {0x21, 0x05, 0x40B0}, {0x21, 0x06, 0x0010},
	{0x21, 0x07, 0xF09F}, {0x21, 0x0A, 0x0003}, {0x21, 0x0B, 0x0005},
	{0x21, 0x0C, 0x0007}, {0x21, 0x0D, 0x6009}, {0x21, 0x0E, 0x0000},
	{0x21, 0x0F, 0x0008}, {0x2E, 0x00, 0xA668}, {0x2E, 0x02, 0xD020},
	{0x2E, 0x06, 0xC000}, {0x2E, 0x0B, 0x1892}, {0x2E, 0x0F, 0xFFDF},
	{0x2E, 0x11, 0x8280}, {0x2E, 0x12, 0x0044}, {0x2E, 0x13, 0x027F},
	{0x2E, 0x14, 0x1311}, {0x2E, 0x17, 0xA100}, {0x2E, 0x1A, 0x0001},
	{0x2E, 0x1C, 0x0400}, {0x2F, 0x00, 0x820F}, {0x2F, 0x01, 0x0300},
	{0x2F, 0x02, 0x1217}, {0x2F, 0x03, 0xFFDF}, {0x2F, 0x05, 0x7F7C},
	{0x2F, 0x07, 0x80C4}, {0x2F, 0x08, 0x0001}, {0x2F, 0x09, 0xFFD4},
	{0x2F, 0x0A, 0x7C2F}, {0x2F, 0x0E, 0x003F}, {0x2F, 0x0F, 0x0121},
	{0x2F, 0x10, 0x0020}, {0x2F, 0x11, 0x8840}, {0x2B, 0x13, 0x3D87},
	{0x2B, 0x14, 0x3108}, {0x2D, 0x13, 0x3C87}, {0x2D, 0x14, 0x1808},
};

static void rtl9300_serdes_patch(int sds_num)
{
	if (sds_num % 2) {
		for (int i = 0; i < sizeof(rtl9300_a_sds_10gr_lane1) / sizeof(sds_config); ++i) {
			rtl930x_write_sds_phy(sds_num, rtl9300_a_sds_10gr_lane1[i].page,
					      rtl9300_a_sds_10gr_lane1[i].reg,
					      rtl9300_a_sds_10gr_lane1[i].data);
		}
	} else {
		for (int i = 0; i < sizeof(rtl9300_a_sds_10gr_lane0) / sizeof(sds_config); ++i) {
			rtl930x_write_sds_phy(sds_num, rtl9300_a_sds_10gr_lane0[i].page,
					      rtl9300_a_sds_10gr_lane0[i].reg,
					      rtl9300_a_sds_10gr_lane0[i].data);
		}
	}
}

int rtl9300_sds_cmu_band_get(int sds)
{
	u32 page;
	u32 en;
	u32 cmu_band;

/*	page = rtl9300_sds_cmu_page_get(sds); */
	page = 0x25; /* 10GR and 1000BX */
	sds = (sds % 2) ? (sds - 1) : (sds);

	rtl9300_sds_field_w(sds, page, 0x1c, 15, 15, 1);
	rtl9300_sds_field_w(sds + 1, page, 0x1c, 15, 15, 1);

	en = rtl9300_sds_field_r(sds, page, 27, 1, 1);
	if(!en) { /* Auto mode */
		rtl930x_write_sds_phy(sds, 0x1f, 0x02, 31);

		cmu_band = rtl9300_sds_field_r(sds, 0x1f, 0x15, 5, 1);
	} else {
		cmu_band = rtl9300_sds_field_r(sds, page, 30, 4, 0);
	}

	return cmu_band;
}

void rtl9310_sds_field_w(int sds, u32 page, u32 reg, int end_bit, int start_bit, u32 v)
{
	int l = end_bit - start_bit + 1;
	u32 data = v;

	if (l < 32) {
		u32 mask = BIT(l) - 1;

		data = rtl930x_read_sds_phy(sds, page, reg);
		data &= ~(mask << start_bit);
		data |= (v & mask) << start_bit;
	}

	rtl931x_write_sds_phy(sds, page, reg, data);
}

u32 rtl9310_sds_field_r(int sds, u32 page, u32 reg, int end_bit, int start_bit)
{
	int l = end_bit - start_bit + 1;
	u32 v = rtl931x_read_sds_phy(sds, page, reg);

	if (l >= 32)
		return v;

	return (v >> start_bit) & (BIT(l) - 1);
}

static void rtl931x_sds_rst(u32 sds)
{
	u32 o, v, o_mode;
	int shift = ((sds & 0x3) << 3);

	/* TODO: We need to lock this! */

	o = sw_r32(RTL931X_PS_SERDES_OFF_MODE_CTRL_ADDR);
	v = o | BIT(sds);
	sw_w32(v, RTL931X_PS_SERDES_OFF_MODE_CTRL_ADDR);

	o_mode = sw_r32(RTL931X_SERDES_MODE_CTRL + 4 * (sds >> 2));
	v = BIT(7) | 0x1F;
	sw_w32_mask(0xff << shift, v << shift, RTL931X_SERDES_MODE_CTRL + 4 * (sds >> 2));
	sw_w32(o_mode, RTL931X_SERDES_MODE_CTRL + 4 * (sds >> 2));

	sw_w32(o, RTL931X_PS_SERDES_OFF_MODE_CTRL_ADDR);
}

static void rtl931x_symerr_clear(u32 sds, phy_interface_t mode)
{

	switch (mode) {
	case PHY_INTERFACE_MODE_NA:
		break;
	case PHY_INTERFACE_MODE_XGMII:
		u32 xsg_sdsid_0, xsg_sdsid_1;

		if (sds < 2)
			xsg_sdsid_0 = sds;
		else
			xsg_sdsid_0 = (sds - 1) * 2;
		xsg_sdsid_1 = xsg_sdsid_0 + 1;

		for (int i = 0; i < 4; ++i) {
			rtl9310_sds_field_w(xsg_sdsid_0, 0x1, 24,  2, 0, i);
			rtl9310_sds_field_w(xsg_sdsid_0, 0x1,  3, 15, 8, 0x0);
			rtl9310_sds_field_w(xsg_sdsid_0, 0x1,  2, 15, 0, 0x0);
		}

		for (int i = 0; i < 4; ++i) {
			rtl9310_sds_field_w(xsg_sdsid_1, 0x1, 24,  2, 0, i);
			rtl9310_sds_field_w(xsg_sdsid_1, 0x1,  3, 15, 8, 0x0);
			rtl9310_sds_field_w(xsg_sdsid_1, 0x1,  2, 15, 0, 0x0);
		}

		rtl9310_sds_field_w(xsg_sdsid_0, 0x1, 0, 15, 0, 0x0);
		rtl9310_sds_field_w(xsg_sdsid_0, 0x1, 1, 15, 8, 0x0);
		rtl9310_sds_field_w(xsg_sdsid_1, 0x1, 0, 15, 0, 0x0);
		rtl9310_sds_field_w(xsg_sdsid_1, 0x1, 1, 15, 8, 0x0);
		break;
	default:
		break;
	}

	return;
}

static u32 rtl931x_get_analog_sds(u32 sds)
{
	u32 sds_map[] = { 0, 1, 2, 3, 6, 7, 10, 11, 14, 15, 18, 19, 22, 23 };

	if (sds < 14)
		return sds_map[sds];

	return sds;
}

void rtl931x_sds_fiber_disable(u32 sds)
{
	u32 v = 0x3F;
	u32 asds = rtl931x_get_analog_sds(sds);

	rtl9310_sds_field_w(asds, 0x1F, 0x9, 11, 6, v);
}

static void rtl931x_sds_fiber_mode_set(u32 sds, phy_interface_t mode)
{
	u32 val, asds = rtl931x_get_analog_sds(sds);

	/* clear symbol error count before changing mode */
	rtl931x_symerr_clear(sds, mode);

	val = 0x9F;
	sw_w32(val, RTL931X_SERDES_MODE_CTRL + 4 * (sds >> 2));

	switch (mode) {
	case PHY_INTERFACE_MODE_SGMII:
		val = 0x5;
		break;

	case PHY_INTERFACE_MODE_1000BASEX:
		/* serdes mode FIBER1G */
		val = 0x9;
		break;

	case PHY_INTERFACE_MODE_10GBASER:
	case PHY_INTERFACE_MODE_10GKR:
		val = 0x35;
		break;
/*	case MII_10GR1000BX_AUTO:
		val = 0x39;
		break; */


	case PHY_INTERFACE_MODE_USXGMII:
		val = 0x1B;
		break;
	default:
		val = 0x25;
	}

	pr_info("%s writing analog SerDes Mode value %02x\n", __func__, val);
	rtl9310_sds_field_w(asds, 0x1F, 0x9, 11, 6, val);

	return;
}

static int rtl931x_sds_cmu_page_get(phy_interface_t mode)
{
	switch (mode) {
	case PHY_INTERFACE_MODE_SGMII:
	case PHY_INTERFACE_MODE_1000BASEX:	/* MII_1000BX_FIBER / 100BX_FIBER / 1000BX100BX_AUTO */
		return 0x24;
	case PHY_INTERFACE_MODE_HSGMII:
	case PHY_INTERFACE_MODE_2500BASEX:	/* MII_2500Base_X: */
		return 0x28;
/*	case MII_HISGMII_5G: */
/*		return 0x2a; */
	case PHY_INTERFACE_MODE_QSGMII:
		return 0x2a;			/* Code also has 0x34 */
	case PHY_INTERFACE_MODE_XAUI:		/* MII_RXAUI_LITE: */
		return 0x2c;
	case PHY_INTERFACE_MODE_XGMII:		/* MII_XSGMII */
	case PHY_INTERFACE_MODE_10GKR:
	case PHY_INTERFACE_MODE_10GBASER:	/* MII_10GR */
		return 0x2e;
	default:
		return -1;
	}

	return -1;
}

static void rtl931x_cmu_type_set(u32 asds, phy_interface_t mode, int chiptype)
{
	int cmu_type = 0; /* Clock Management Unit */
	u32 cmu_page = 0;
	u32 frc_cmu_spd;
	u32 evenSds;
	u32 lane, frc_lc_mode_bitnum, frc_lc_mode_val_bitnum;

	switch (mode) {
	case PHY_INTERFACE_MODE_NA:
	case PHY_INTERFACE_MODE_10GKR:
	case PHY_INTERFACE_MODE_XGMII:
	case PHY_INTERFACE_MODE_10GBASER:
	case PHY_INTERFACE_MODE_USXGMII:
		return;

/*	case MII_10GR1000BX_AUTO:
		if (chiptype)
			rtl9310_sds_field_w(asds, 0x24, 0xd, 14, 14, 0);
		return; */

	case PHY_INTERFACE_MODE_QSGMII:
		cmu_type = 1;
		frc_cmu_spd = 0;
		break;

	case PHY_INTERFACE_MODE_HSGMII:
		cmu_type = 1;
		frc_cmu_spd = 1;
		break;

	case PHY_INTERFACE_MODE_1000BASEX:
		cmu_type = 1;
		frc_cmu_spd = 0;
		break;

/*	case MII_1000BX100BX_AUTO:
		cmu_type = 1;
		frc_cmu_spd = 0;
		break; */

	case PHY_INTERFACE_MODE_SGMII:
		cmu_type = 1;
		frc_cmu_spd = 0;
		break;

	case PHY_INTERFACE_MODE_2500BASEX:
		cmu_type = 1;
		frc_cmu_spd = 1;
		break;

	default:
		pr_info("SerDes %d mode is invalid\n", asds);
		return;
	}

	if (cmu_type == 1)
		cmu_page = rtl931x_sds_cmu_page_get(mode);

	lane = asds % 2;

	if (!lane) {
		frc_lc_mode_bitnum = 4;
		frc_lc_mode_val_bitnum = 5;
	} else {
		frc_lc_mode_bitnum = 6;
		frc_lc_mode_val_bitnum = 7;
	}

	evenSds = asds - lane;

	pr_info("%s: cmu_type %0d cmu_page %x frc_cmu_spd %d lane %d asds %d\n",
	        __func__, cmu_type, cmu_page, frc_cmu_spd, lane, asds);

	if (cmu_type == 1) {
		pr_info("%s A CMU page 0x28 0x7 %08x\n", __func__, rtl931x_read_sds_phy(asds, 0x28, 0x7));
		rtl9310_sds_field_w(asds, cmu_page, 0x7, 15, 15, 0);
		pr_info("%s B CMU page 0x28 0x7 %08x\n", __func__, rtl931x_read_sds_phy(asds, 0x28, 0x7));
		if (chiptype) {
			rtl9310_sds_field_w(asds, cmu_page, 0xd, 14, 14, 0);
		}

		rtl9310_sds_field_w(evenSds, 0x20, 0x12, 3, 2, 0x3);
		rtl9310_sds_field_w(evenSds, 0x20, 0x12, frc_lc_mode_bitnum, frc_lc_mode_bitnum, 1);
		rtl9310_sds_field_w(evenSds, 0x20, 0x12, frc_lc_mode_val_bitnum, frc_lc_mode_val_bitnum, 0);
		rtl9310_sds_field_w(evenSds, 0x20, 0x12, 12, 12, 1);
		rtl9310_sds_field_w(evenSds, 0x20, 0x12, 15, 13, frc_cmu_spd);
	}

	pr_info("%s CMU page 0x28 0x7 %08x\n", __func__, rtl931x_read_sds_phy(asds, 0x28, 0x7));
	return;
}

static void rtl931x_sds_rx_rst(u32 sds)
{
	u32 asds = rtl931x_get_analog_sds(sds);

	if (sds < 2)
		return;

	rtl931x_write_sds_phy(asds, 0x2e, 0x12, 0x2740);
	rtl931x_write_sds_phy(asds, 0x2f, 0x0, 0x0);
	rtl931x_write_sds_phy(asds, 0x2f, 0x2, 0x2010);
	rtl931x_write_sds_phy(asds, 0x20, 0x0, 0xc10);

	rtl931x_write_sds_phy(asds, 0x2e, 0x12, 0x27c0);
	rtl931x_write_sds_phy(asds, 0x2f, 0x0, 0xc000);
	rtl931x_write_sds_phy(asds, 0x2f, 0x2, 0x6010);
	rtl931x_write_sds_phy(asds, 0x20, 0x0, 0xc30);

	mdelay(50);
}

// Currently not used
// static void rtl931x_sds_disable(u32 sds)
// {
// 	u32 v = 0x1f;

// 	v |= BIT(7);
// 	sw_w32(v, RTL931X_SERDES_MODE_CTRL + (sds >> 2) * 4);
// }

static void rtl931x_sds_mii_mode_set(u32 sds, phy_interface_t mode)
{
	u32 val;

	switch (mode) {
	case PHY_INTERFACE_MODE_QSGMII:
		val = 0x6;
		break;
	case PHY_INTERFACE_MODE_XGMII:
		val = 0x10; /* serdes mode XSGMII */
		break;
	case PHY_INTERFACE_MODE_USXGMII:
	case PHY_INTERFACE_MODE_2500BASEX:
		val = 0xD;
		break;
	case PHY_INTERFACE_MODE_HSGMII:
		val = 0x12;
		break;
	case PHY_INTERFACE_MODE_SGMII:
		val = 0x2;
		break;
	default:
		return;
	}

	val |= (1 << 7);

	sw_w32(val, RTL931X_SERDES_MODE_CTRL + 4 * (sds >> 2));
}

static sds_config sds_config_10p3125g_type1[] = {
	{ 0x2E, 0x00, 0x0107 }, { 0x2E, 0x01, 0x01A3 }, { 0x2E, 0x02, 0x6A24 },
	{ 0x2E, 0x03, 0xD10D }, { 0x2E, 0x04, 0x8000 }, { 0x2E, 0x05, 0xA17E },
	{ 0x2E, 0x06, 0xE31D }, { 0x2E, 0x07, 0x800E }, { 0x2E, 0x08, 0x0294 },
	{ 0x2E, 0x09, 0x0CE4 }, { 0x2E, 0x0A, 0x7FC8 }, { 0x2E, 0x0B, 0xE0E7 },
	{ 0x2E, 0x0C, 0x0200 }, { 0x2E, 0x0D, 0xDF80 }, { 0x2E, 0x0E, 0x0000 },
	{ 0x2E, 0x0F, 0x1FC2 }, { 0x2E, 0x10, 0x0C3F }, { 0x2E, 0x11, 0x0000 },
	{ 0x2E, 0x12, 0x27C0 }, { 0x2E, 0x13, 0x7E1D }, { 0x2E, 0x14, 0x1300 },
	{ 0x2E, 0x15, 0x003F }, { 0x2E, 0x16, 0xBE7F }, { 0x2E, 0x17, 0x0090 },
	{ 0x2E, 0x18, 0x0000 }, { 0x2E, 0x19, 0x4000 }, { 0x2E, 0x1A, 0x0000 },
	{ 0x2E, 0x1B, 0x8000 }, { 0x2E, 0x1C, 0x011F }, { 0x2E, 0x1D, 0x0000 },
	{ 0x2E, 0x1E, 0xC8FF }, { 0x2E, 0x1F, 0x0000 }, { 0x2F, 0x00, 0xC000 },
	{ 0x2F, 0x01, 0xF000 }, { 0x2F, 0x02, 0x6010 }, { 0x2F, 0x12, 0x0EE7 },
	{ 0x2F, 0x13, 0x0000 }
};

static sds_config sds_config_10p3125g_cmu_type1[] = {
	{ 0x2F, 0x03, 0x4210 }, { 0x2F, 0x04, 0x0000 }, { 0x2F, 0x05, 0x0019 },
	{ 0x2F, 0x06, 0x18A6 }, { 0x2F, 0x07, 0x2990 }, { 0x2F, 0x08, 0xFFF4 },
	{ 0x2F, 0x09, 0x1F08 }, { 0x2F, 0x0A, 0x0000 }, { 0x2F, 0x0B, 0x8000 },
	{ 0x2F, 0x0C, 0x4224 }, { 0x2F, 0x0D, 0x0000 }, { 0x2F, 0x0E, 0x0000 },
	{ 0x2F, 0x0F, 0xA470 }, { 0x2F, 0x10, 0x8000 }, { 0x2F, 0x11, 0x037B }
};

void rtl931x_sds_init(u32 sds, phy_interface_t mode)
{
	u32 board_sds_tx_type1[] = {
		0x01c3, 0x01c3, 0x01c3, 0x01a3, 0x01a3, 0x01a3,
		0x0143, 0x0143, 0x0143, 0x0143, 0x0163, 0x0163,
	};
	u32 board_sds_tx[] = {
		0x1a00, 0x1a00, 0x0200, 0x0200, 0x0200, 0x0200,
		0x01a3, 0x01a3, 0x01a3, 0x01a3, 0x01e3, 0x01e3
	};
	u32 board_sds_tx2[] = {
		0x0dc0, 0x01c0, 0x0200, 0x0180, 0x0160, 0x0123,
		0x0123, 0x0163, 0x01a3, 0x01a0, 0x01c3, 0x09c3,
	};
	u32 asds, dSds, ori, model_info, val;
	int chiptype = 0;

	asds = rtl931x_get_analog_sds(sds);

	if (sds > 13)
		return;

	pr_info("%s: set sds %d to mode %d\n", __func__, sds, mode);
	val = rtl9310_sds_field_r(asds, 0x1F, 0x9, 11, 6);

	pr_info("%s: fibermode %08X stored mode 0x%x analog SDS %d", __func__,
			rtl931x_read_sds_phy(asds, 0x1f, 0x9), val, asds);
	pr_info("%s: SGMII mode %08X in 0x24 0x9 analog SDS %d", __func__,
			rtl931x_read_sds_phy(asds, 0x24, 0x9), asds);
	pr_info("%s: CMU mode %08X stored even SDS %d", __func__,
			rtl931x_read_sds_phy(asds & ~1, 0x20, 0x12), asds & ~1);
	pr_info("%s: serdes_mode_ctrl %08X", __func__,  RTL931X_SERDES_MODE_CTRL + 4 * (sds >> 2));
	pr_info("%s CMU page 0x24 0x7 %08x\n", __func__, rtl931x_read_sds_phy(asds, 0x24, 0x7));
	pr_info("%s CMU page 0x26 0x7 %08x\n", __func__, rtl931x_read_sds_phy(asds, 0x26, 0x7));
	pr_info("%s CMU page 0x28 0x7 %08x\n", __func__, rtl931x_read_sds_phy(asds, 0x28, 0x7));
	pr_info("%s XSG page 0x0 0xe %08x\n", __func__, rtl931x_read_sds_phy(dSds, 0x0, 0xe));
	pr_info("%s XSG2 page 0x0 0xe %08x\n", __func__, rtl931x_read_sds_phy(dSds + 1, 0x0, 0xe));

	model_info = sw_r32(RTL93XX_MODEL_NAME_INFO);
	if ((model_info >> 4) & 0x1) {
		pr_info("detected chiptype 1\n");
		chiptype = 1;
	} else {
		pr_info("detected chiptype 0\n");
	}

	if (sds < 2)
		dSds = sds;
	else
		dSds = (sds - 1) * 2;

	pr_info("%s: 2.5gbit %08X dsds %d", __func__,
	        rtl931x_read_sds_phy(dSds, 0x1, 0x14), dSds);

	pr_info("%s: RTL931X_PS_SERDES_OFF_MODE_CTRL_ADDR 0x%08X\n", __func__, sw_r32(RTL931X_PS_SERDES_OFF_MODE_CTRL_ADDR));
	ori = sw_r32(RTL931X_PS_SERDES_OFF_MODE_CTRL_ADDR);
	val = ori | (1 << sds);
	sw_w32(val, RTL931X_PS_SERDES_OFF_MODE_CTRL_ADDR);

	switch (mode) {
	case PHY_INTERFACE_MODE_NA:
		break;

	case PHY_INTERFACE_MODE_XGMII: /* MII_XSGMII */

		if (chiptype) {
			u32 xsg_sdsid_1;
			xsg_sdsid_1 = dSds + 1;
			/* fifo inv clk */
			rtl9310_sds_field_w(dSds, 0x1, 0x1, 7, 4, 0xf);
			rtl9310_sds_field_w(dSds, 0x1, 0x1, 3, 0, 0xf);

			rtl9310_sds_field_w(xsg_sdsid_1, 0x1, 0x1, 7, 4, 0xf);
			rtl9310_sds_field_w(xsg_sdsid_1, 0x1, 0x1, 3, 0, 0xf);

		}

		rtl9310_sds_field_w(dSds, 0x0, 0xE, 12, 12, 1);
		rtl9310_sds_field_w(dSds + 1, 0x0, 0xE, 12, 12, 1);
		break;

	case PHY_INTERFACE_MODE_USXGMII: /* MII_USXGMII_10GSXGMII/10GDXGMII/10GQXGMII: */
		u32 op_code = 0x6003;
		u32 evenSds;

		if (chiptype) {
			rtl9310_sds_field_w(asds, 0x6, 0x2, 12, 12, 1);

			for (int i = 0; i < sizeof(sds_config_10p3125g_type1) / sizeof(sds_config); ++i) {
				rtl931x_write_sds_phy(asds, sds_config_10p3125g_type1[i].page - 0x4, sds_config_10p3125g_type1[i].reg, sds_config_10p3125g_type1[i].data);
			}

			evenSds = asds - (asds % 2);

			for (int i = 0; i < sizeof(sds_config_10p3125g_cmu_type1) / sizeof(sds_config); ++i) {
				rtl931x_write_sds_phy(evenSds,
				                      sds_config_10p3125g_cmu_type1[i].page - 0x4, sds_config_10p3125g_cmu_type1[i].reg, sds_config_10p3125g_cmu_type1[i].data);
			}

			rtl9310_sds_field_w(asds, 0x6, 0x2, 12, 12, 0);
		} else {

			rtl9310_sds_field_w(asds, 0x2e, 0xd, 6, 0, 0x0);
			rtl9310_sds_field_w(asds, 0x2e, 0xd, 7, 7, 0x1);

			rtl9310_sds_field_w(asds, 0x2e, 0x1c, 5, 0, 0x1E);
			rtl9310_sds_field_w(asds, 0x2e, 0x1d, 11, 0, 0x00);
			rtl9310_sds_field_w(asds, 0x2e, 0x1f, 11, 0, 0x00);
			rtl9310_sds_field_w(asds, 0x2f, 0x0, 11, 0, 0x00);
			rtl9310_sds_field_w(asds, 0x2f, 0x1, 11, 0, 0x00);

			rtl9310_sds_field_w(asds, 0x2e, 0xf, 12, 6, 0x7F);
			rtl931x_write_sds_phy(asds, 0x2f, 0x12, 0xaaa);

			rtl931x_sds_rx_rst(sds);

			rtl931x_write_sds_phy(asds, 0x7, 0x10, op_code);
			rtl931x_write_sds_phy(asds, 0x6, 0x1d, 0x0480);
			rtl931x_write_sds_phy(asds, 0x6, 0xe, 0x0400);
		}
		break;

	case PHY_INTERFACE_MODE_10GBASER: /* MII_10GR / MII_10GR1000BX_AUTO: */
	                                  /* configure 10GR fiber mode=1 */
		rtl9310_sds_field_w(asds, 0x1f, 0xb, 1, 1, 1);

		/* init fiber_1g */
		rtl9310_sds_field_w(dSds, 0x3, 0x13, 15, 14, 0);

		rtl9310_sds_field_w(dSds, 0x2, 0x0, 12, 12, 1);
		rtl9310_sds_field_w(dSds, 0x2, 0x0, 6, 6, 1);
		rtl9310_sds_field_w(dSds, 0x2, 0x0, 13, 13, 0);

		/* init auto */
		rtl9310_sds_field_w(asds, 0x1f, 13, 15, 0, 0x109e);
		rtl9310_sds_field_w(asds, 0x1f, 0x6, 14, 10, 0x8);
		rtl9310_sds_field_w(asds, 0x1f, 0x7, 10, 4, 0x7f);
		break;

	case PHY_INTERFACE_MODE_HSGMII:
		rtl9310_sds_field_w(dSds, 0x1, 0x14, 8, 8, 1);
		break;

	case PHY_INTERFACE_MODE_1000BASEX: /* MII_1000BX_FIBER */
		rtl9310_sds_field_w(dSds, 0x3, 0x13, 15, 14, 0);

		rtl9310_sds_field_w(dSds, 0x2, 0x0, 12, 12, 1);
		rtl9310_sds_field_w(dSds, 0x2, 0x0, 6, 6, 1);
		rtl9310_sds_field_w(dSds, 0x2, 0x0, 13, 13, 0);
		break;

	case PHY_INTERFACE_MODE_SGMII:
		rtl9310_sds_field_w(asds, 0x24, 0x9, 15, 15, 0);
		break;

	case PHY_INTERFACE_MODE_2500BASEX:
		rtl9310_sds_field_w(dSds, 0x1, 0x14, 8, 8, 1);
		break;

	case PHY_INTERFACE_MODE_QSGMII:
	default:
		pr_info("%s: PHY mode %s not supported by SerDes %d\n",
		        __func__, phy_modes(mode), sds);
		return;
	}

	rtl931x_cmu_type_set(asds, mode, chiptype);

	if (sds >= 2 && sds <= 13) {
		if (chiptype)
			rtl931x_write_sds_phy(asds, 0x2E, 0x1, board_sds_tx_type1[sds - 2]);
		else {
			val = 0xa0000;
			sw_w32(val, RTL931X_CHIP_INFO_ADDR);
			val = sw_r32(RTL931X_CHIP_INFO_ADDR);
			if (val & BIT(28)) /* consider 9311 etc. RTL9313_CHIP_ID == HWP_CHIP_ID(unit)) */
			{
				rtl931x_write_sds_phy(asds, 0x2E, 0x1, board_sds_tx2[sds - 2]);
			} else {
				rtl931x_write_sds_phy(asds, 0x2E, 0x1, board_sds_tx[sds - 2]);
			}
			val = 0;
			sw_w32(val, RTL931X_CHIP_INFO_ADDR);
		}
	}

	val = ori & ~BIT(sds);
	sw_w32(val, RTL931X_PS_SERDES_OFF_MODE_CTRL_ADDR);
	pr_debug("%s: RTL931X_PS_SERDES_OFF_MODE_CTRL_ADDR 0x%08X\n", __func__, sw_r32(RTL931X_PS_SERDES_OFF_MODE_CTRL_ADDR));

	if (mode == PHY_INTERFACE_MODE_XGMII ||
	    mode == PHY_INTERFACE_MODE_QSGMII ||
	    mode == PHY_INTERFACE_MODE_HSGMII ||
	    mode == PHY_INTERFACE_MODE_SGMII ||
	    mode == PHY_INTERFACE_MODE_USXGMII) {
		if (mode == PHY_INTERFACE_MODE_XGMII)
			rtl931x_sds_mii_mode_set(sds, mode);
		else
			rtl931x_sds_fiber_mode_set(sds, mode);
	}
}

int rtl931x_sds_cmu_band_set(int sds, bool enable, u32 band, phy_interface_t mode)
{
	u32 asds;
	int page = rtl931x_sds_cmu_page_get(mode);

	sds -= (sds % 2);
	sds = sds & ~1;
	asds = rtl931x_get_analog_sds(sds);
	page += 1;

	if (enable) {
		rtl9310_sds_field_w(asds, page, 0x7, 13, 13, 0);
		rtl9310_sds_field_w(asds, page, 0x7, 11, 11, 0);
	} else {
		rtl9310_sds_field_w(asds, page, 0x7, 13, 13, 0);
		rtl9310_sds_field_w(asds, page, 0x7, 11, 11, 0);
	}

	rtl9310_sds_field_w(asds, page, 0x7, 4, 0, band);

	rtl931x_sds_rst(sds);

	return 0;
}

int rtl931x_sds_cmu_band_get(int sds, phy_interface_t mode)
{
	int page = rtl931x_sds_cmu_page_get(mode);
	u32 asds, band;

	sds -= (sds % 2);
	asds = rtl931x_get_analog_sds(sds);
	page += 1;
	rtl931x_write_sds_phy(asds, 0x1f, 0x02, 73);

	rtl9310_sds_field_w(asds, page, 0x5, 15, 15, 1);
	band = rtl9310_sds_field_r(asds, 0x1f, 0x15, 8, 3);
	pr_info("%s band is: %d\n", __func__, band);

	return band;
}


int rtl931x_link_sts_get(u32 sds)
{
	u32 sts, sts1, latch_sts, latch_sts1;
	if (0){
		u32 xsg_sdsid_0, xsg_sdsid_1;

		xsg_sdsid_0 = sds < 2 ? sds : (sds - 1) * 2;
		xsg_sdsid_1 = xsg_sdsid_0 + 1;

		sts = rtl9310_sds_field_r(xsg_sdsid_0, 0x1, 29, 8, 0);
		sts1 = rtl9310_sds_field_r(xsg_sdsid_1, 0x1, 29, 8, 0);
		latch_sts = rtl9310_sds_field_r(xsg_sdsid_0, 0x1, 30, 8, 0);
		latch_sts1 = rtl9310_sds_field_r(xsg_sdsid_1, 0x1, 30, 8, 0);
	} else {
		u32  asds, dsds;

		asds = rtl931x_get_analog_sds(sds);
		sts = rtl9310_sds_field_r(asds, 0x5, 0, 12, 12);
		latch_sts = rtl9310_sds_field_r(asds, 0x4, 1, 2, 2);

		dsds = sds < 2 ? sds : (sds - 1) * 2;
		latch_sts1 = rtl9310_sds_field_r(dsds, 0x2, 1, 2, 2);
		sts1 = rtl9310_sds_field_r(dsds, 0x2, 1, 2, 2);
	}

	pr_info("%s: serdes %d sts %d, sts1 %d, latch_sts %d, latch_sts1 %d\n", __func__,
		sds, sts, sts1, latch_sts, latch_sts1);

	return sts1;
}

static int rtl8214fc_sfp_insert(void *upstream, const struct sfp_eeprom_id *id)
{
	__ETHTOOL_DECLARE_LINK_MODE_MASK(support) = { 0, };
	DECLARE_PHY_INTERFACE_MASK(interfaces);
	struct phy_device *phydev = upstream;
	phy_interface_t iface;

	sfp_parse_support(phydev->sfp_bus, id, support, interfaces);
	iface = sfp_select_interface(phydev->sfp_bus, support);

	dev_info(&phydev->mdio.dev, "%s SFP module inserted\n", phy_modes(iface));

	rtl8214fc_media_set(phydev, true);

	return 0;
}

static void rtl8214fc_sfp_remove(void *upstream)
{
	struct phy_device *phydev = upstream;

	rtl8214fc_media_set(phydev, false);
}

static const struct sfp_upstream_ops rtl8214fc_sfp_ops = {
	.attach = phy_sfp_attach,
	.detach = phy_sfp_detach,
	.module_insert = rtl8214fc_sfp_insert,
	.module_remove = rtl8214fc_sfp_remove,
};

static int rtl8214fc_phy_probe(struct phy_device *phydev)
{
	struct device *dev = &phydev->mdio.dev;
	int addr = phydev->mdio.addr;
	int ret = 0;

	/* 839x has internal SerDes */
	if (soc_info.id == 0x8393)
		return -ENODEV;

	/* All base addresses of the PHYs start at multiples of 8 */
	devm_phy_package_join(dev, phydev, addr & (~7),
				sizeof(struct rtl83xx_shared_private));

	if (!(addr % 8)) {
		struct rtl83xx_shared_private *shared = phydev->shared->priv;
		shared->name = "RTL8214FC";
		/* Configuration must be done while patching still possible */
		ret = rtl8380_configure_rtl8214fc(phydev);
		if (ret)
			return ret;
	}

	return phy_sfp_probe(phydev, &rtl8214fc_sfp_ops);
}

static int rtl8214c_phy_probe(struct phy_device *phydev)
{
	struct device *dev = &phydev->mdio.dev;
	int addr = phydev->mdio.addr;

	/* All base addresses of the PHYs start at multiples of 8 */
	devm_phy_package_join(dev, phydev, addr & (~7),
				sizeof(struct rtl83xx_shared_private));

	if (!(addr % 8)) {
		struct rtl83xx_shared_private *shared = phydev->shared->priv;
		shared->name = "RTL8214C";
		/* Configuration must be done whil patching still possible */
		return rtl8380_configure_rtl8214c(phydev);
	}

	return 0;
}

static int rtl8218b_ext_phy_probe(struct phy_device *phydev)
{
	struct device *dev = &phydev->mdio.dev;
	int addr = phydev->mdio.addr;

	/* All base addresses of the PHYs start at multiples of 8 */
	devm_phy_package_join(dev, phydev, addr & (~7),
				sizeof(struct rtl83xx_shared_private));

	if (!(addr % 8)) {
		struct rtl83xx_shared_private *shared = phydev->shared->priv;
		shared->name = "RTL8218B (external)";
		if (soc_info.family == RTL8380_FAMILY_ID) {
			/* Configuration must be done while patching still possible */
			return rtl8380_configure_ext_rtl8218b(phydev);
		}
	}

	return 0;
}

static int rtl8218b_int_phy_probe(struct phy_device *phydev)
{
	struct device *dev = &phydev->mdio.dev;
	int addr = phydev->mdio.addr;

	if (soc_info.family != RTL8380_FAMILY_ID)
		return -ENODEV;
	if (addr >= 24)
		return -ENODEV;

	pr_debug("%s: id: %d\n", __func__, addr);
	/* All base addresses of the PHYs start at multiples of 8 */
	devm_phy_package_join(dev, phydev, addr & (~7),
			      sizeof(struct rtl83xx_shared_private));

	if (!(addr % 8)) {
		struct rtl83xx_shared_private *shared = phydev->shared->priv;
		shared->name = "RTL8218B (internal)";
		/* Configuration must be done while patching still possible */
		return rtl8380_configure_int_rtl8218b(phydev);
	}

	return 0;
}

static int rtl8218d_phy_probe(struct phy_device *phydev)
{
	struct device *dev = &phydev->mdio.dev;
	int addr = phydev->mdio.addr;

	pr_debug("%s: id: %d\n", __func__, addr);
	/* All base addresses of the PHYs start at multiples of 8 */
	devm_phy_package_join(dev, phydev, addr & (~7),
			      sizeof(struct rtl83xx_shared_private));

	/* All base addresses of the PHYs start at multiples of 8 */
	if (!(addr % 8)) {
		struct rtl83xx_shared_private *shared = phydev->shared->priv;
		shared->name = "RTL8218D";
		/* Configuration must be done while patching still possible */
/* TODO:		return configure_rtl8218d(phydev); */
	}

	return 0;
}

static int rtl838x_serdes_probe(struct phy_device *phydev)
{
	int addr = phydev->mdio.addr;

	if (soc_info.family != RTL8380_FAMILY_ID)
		return -ENODEV;
	if (addr < 24)
		return -ENODEV;

	/* On the RTL8380M, PHYs 24-27 connect to the internal SerDes */
	if (soc_info.id == 0x8380) {
		if (addr == 24)
			return rtl8380_configure_serdes(phydev);
		return 0;
	}

	return -ENODEV;
}

static int rtl8393_serdes_probe(struct phy_device *phydev)
{
	int addr = phydev->mdio.addr;

	pr_info("%s: id: %d\n", __func__, addr);
	if (soc_info.family != RTL8390_FAMILY_ID)
		return -ENODEV;

	if (addr < 24)
		return -ENODEV;

	return rtl8390_configure_serdes(phydev);
}

static int rtl8390_serdes_probe(struct phy_device *phydev)
{
	int addr = phydev->mdio.addr;

	if (soc_info.family != RTL8390_FAMILY_ID)
		return -ENODEV;

	if (addr < 24)
		return -ENODEV;

	return rtl8390_configure_generic(phydev);
}

static int rtl9300_serdes_probe(struct phy_device *phydev)
{
	if (soc_info.family != RTL9300_FAMILY_ID)
		return -ENODEV;

	phydev_info(phydev, "Detected internal RTL9300 Serdes\n");

	return 0;
}

static struct phy_driver rtl83xx_phy_driver[] = {
	{
		PHY_ID_MATCH_EXACT(PHY_ID_RTL8214C),
		.name		= "Realtek RTL8214C",
		.features	= PHY_GBIT_FEATURES,
		.probe		= rtl8214c_phy_probe,
		.read_page	= rtl821x_read_page,
		.write_page	= rtl821x_write_page,
		.suspend	= genphy_suspend,
		.resume		= genphy_resume,
		.set_loopback	= genphy_loopback,
	},
	{
		.match_phy_device = rtl8214fc_match_phy_device,
		.name		= "Realtek RTL8214FC",
		.config_aneg	= rtl8214fc_config_aneg,
		.get_eee	= rtl8214fc_get_eee,
		.get_features	= rtl8214fc_get_features,
		.get_port	= rtl8214fc_get_port,
		.probe		= rtl8214fc_phy_probe,
		.read_page	= rtl821x_read_page,
		.read_status    = rtl8214fc_read_status,
		.resume		= rtl8214fc_resume,
		.set_eee	= rtl8214fc_set_eee,
		.set_port	= rtl8214fc_set_port,
		.suspend	= rtl8214fc_suspend,
		.write_page	= rtl821x_write_page,
	},
	{
		.match_phy_device = rtl8218b_ext_match_phy_device,
		.name		= "Realtek RTL8218B (external)",
		.features	= PHY_GBIT_FEATURES,
		.probe		= rtl8218b_ext_phy_probe,
		.read_page	= rtl821x_read_page,
		.write_page	= rtl821x_write_page,
		.suspend	= genphy_suspend,
		.resume		= genphy_resume,
		.set_loopback	= genphy_loopback,
		.set_eee	= rtl8218b_set_eee,
		.get_eee	= rtl8218b_get_eee,
	},
	{
		PHY_ID_MATCH_EXACT(PHY_ID_RTL8218D),
		.name		= "REALTEK RTL8218D",
		.features	= PHY_GBIT_FEATURES,
		.probe		= rtl8218d_phy_probe,
		.read_page	= rtl821x_read_page,
		.write_page	= rtl821x_write_page,
		.suspend	= genphy_suspend,
		.resume		= genphy_resume,
		.set_loopback	= genphy_loopback,
		.set_eee	= rtl8218d_set_eee,
		.get_eee	= rtl8218d_get_eee,
	},
	{
		PHY_ID_MATCH_MODEL(PHY_ID_RTL8221B),
		.name           = "REALTEK RTL8221B",
		.features       = PHY_GBIT_FEATURES,
		.suspend        = genphy_suspend,
		.resume         = genphy_resume,
		.set_loopback   = genphy_loopback,
		.read_page      = rtl821x_read_page,
		.write_page     = rtl821x_write_page,
		.read_status    = rtl8226_read_status,
		.config_aneg    = rtl8226_config_aneg,
		.set_eee        = rtl8226_set_eee,
		.get_eee        = rtl8226_get_eee,
	},
	{
		PHY_ID_MATCH_MODEL(PHY_ID_RTL8226),
		.name		= "REALTEK RTL8226",
		.features	= PHY_GBIT_FEATURES,
		.suspend	= genphy_suspend,
		.resume		= genphy_resume,
		.set_loopback	= genphy_loopback,
		.read_page	= rtl821x_read_page,
		.write_page	= rtl821x_write_page,
		.read_status	= rtl8226_read_status,
		.config_aneg	= rtl8226_config_aneg,
		.set_eee	= rtl8226_set_eee,
		.get_eee	= rtl8226_get_eee,
	},
	{
		PHY_ID_MATCH_MODEL(PHY_ID_RTL8218B_I),
		.name		= "Realtek RTL8218B (internal)",
		.features	= PHY_GBIT_FEATURES,
		.probe		= rtl8218b_int_phy_probe,
		.read_page	= rtl821x_read_page,
		.write_page	= rtl821x_write_page,
		.suspend	= genphy_suspend,
		.resume		= genphy_resume,
		.set_loopback	= genphy_loopback,
		.set_eee	= rtl8218b_set_eee,
		.get_eee	= rtl8218b_get_eee,
	},
	{
		PHY_ID_MATCH_MODEL(PHY_ID_RTL8218B_I),
		.name		= "Realtek RTL8380 SERDES",
		.features	= PHY_GBIT_FIBRE_FEATURES,
		.probe		= rtl838x_serdes_probe,
		.read_page	= rtl821x_read_page,
		.write_page	= rtl821x_write_page,
		.suspend	= genphy_suspend,
		.resume		= genphy_resume,
		.set_loopback	= genphy_loopback,
		.read_status	= rtl8380_read_status,
	},
	{
		PHY_ID_MATCH_MODEL(PHY_ID_RTL8393_I),
		.name		= "Realtek RTL8393 SERDES",
		.features	= PHY_GBIT_FIBRE_FEATURES,
		.probe		= rtl8393_serdes_probe,
		.read_page	= rtl821x_read_page,
		.write_page	= rtl821x_write_page,
		.suspend	= genphy_suspend,
		.resume		= genphy_resume,
		.set_loopback	= genphy_loopback,
		.read_status	= rtl8393_read_status,
	},
	{
		PHY_ID_MATCH_MODEL(PHY_ID_RTL8390_GENERIC),
		.name		= "Realtek RTL8390 Generic",
		.features	= PHY_GBIT_FIBRE_FEATURES,
		.read_page	= rtl821x_read_page,
		.write_page	= rtl821x_write_page,
		.probe		= rtl8390_serdes_probe,
		.suspend	= genphy_suspend,
		.resume		= genphy_resume,
		.set_loopback	= genphy_loopback,
	},
	{
		PHY_ID_MATCH_MODEL(PHY_ID_RTL9300_I),
		.name		= "REALTEK RTL9300 SERDES",
		.features	= PHY_GBIT_FIBRE_FEATURES,
		.read_page	= rtl821x_read_page,
		.write_page	= rtl821x_write_page,
		.probe		= rtl9300_serdes_probe,
		.suspend	= genphy_suspend,
		.resume		= genphy_resume,
		.set_loopback	= genphy_loopback,
		.read_status	= rtl9300_read_status,
	},
};

module_phy_driver(rtl83xx_phy_driver);

static struct mdio_device_id __maybe_unused rtl83xx_tbl[] = {
	{ PHY_ID_MATCH_MODEL(PHY_ID_RTL8214_OR_8218) },
	{ }
};

MODULE_DEVICE_TABLE(mdio, rtl83xx_tbl);

MODULE_AUTHOR("B. Koblitz");
MODULE_DESCRIPTION("RTL83xx PHY driver");
MODULE_LICENSE("GPL");
