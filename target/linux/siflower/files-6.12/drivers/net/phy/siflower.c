// SPDX-License-Identifier: GPL-2.0+
/*
 * drivers/net/phy/siflower.c
 *
 * Driver for Siflower PHYs
 *
 * Copyright (c) 2023 Siflower, Inc.
 *
 * Support : Siflower Phys:
 *		Giga phys: p1211f, p1240
 */
#include <linux/bitops.h>
#include <linux/of.h>
#include <linux/phy.h>
#include <linux/module.h>
#include <linux/delay.h>

/* for wol feature */
#include <linux/netdevice.h>

/* WOL Enable Flag:
 * disable by default to enable system WOL feature of phy
 * please define this phy to 1 otherwise, define it to 0.
 */
#define SIFLOWER_PHY_WOL_FEATURE_ENABLE                         0
#define SIFLOWER_PHY_WOL_PASSWD_ENABLE                          0

#define SIFLOWER_PHY_MODE_SET_ENABLE                            0
#define SIFLOWER_PHY_RXC_DELAY_SET_ENABLE                       0
#define SIFLOWER_PHY_RXC_DELAY_VAL                              0x40
#define SIFLOWER_PHY_TXC_DELAY_VAL                              0x40
#define SIFLOWER_PHY_CLK_OUT_125M_ENABLE                        1

#define SFPHY_GLB_DISABLE                                      0
#define SFPHY_GLB_ENABLE                                       1
#define SFPHY_LINK_DOWN                                        0
#define SFPHY_LINK_UP                                          1
/* Mask used for ID comparisons */
#define SIFLOWER_PHY_ID_MASK                                    0xffffffff

/* SF1211F PHY IDs */
#define SF1211F_PHY_ID                                          0xADB40412
/* SF1240 PHY IDs */
#define SF1240_PHY_ID                                          0xADB40411

/* SF1211F PHY LED */
#define SF1211F_EXTREG_LED0                                     0x1E33   // 0
#define SF1211F_EXTREG_LED1                                     0x1E34   // 00101111
#define SF1211F_EXTREG_LED2                                     0x1E35   // 0x40
/* SF1240 PHY BX LED */
#define SF1240_EXTREG_LEDCTRL                                  0x0621
#define SF1240_EXTREG_LED0_1                                   0x0700
#define SF1240_EXTREG_LED0_2                                   0x0701
#define SF1240_EXTREG_LED1_1                                   0x0702
#define SF1240_EXTREG_LED1_2                                   0x0703
#define SF1240_EXTREG_LED2_1                                   0x0706
#define SF1240_EXTREG_LED2_2                                   0x0707
#define SF1240_EXTREG_LED3_1                                   0x0708
#define SF1240_EXTREG_LED3_2                                   0x0709
#define SF1240_EXTREG_LED4_1                                   0x070C
#define SF1240_EXTREG_LED4_2                                   0x070D
#define SF1240_EXTREG_LED5_1                                   0x070E
#define SF1240_EXTREG_LED5_2                                   0x070F
#define SF1240_EXTREG_LED6_1                                   0x0712
#define SF1240_EXTREG_LED6_2                                   0x0713
#define SF1240_EXTREG_LED7_1                                   0x0714
#define SF1240_EXTREG_LED7_2                                   0x0715

/* PHY MODE OPSREG*/
#define SF1211F_EXTREG_GET_PORT_PHY_MODE                        0x062B
#define SF1211F_EXTREG_PHY_MODE_MASK                            0x0070
/* Magic Packet MAC address registers */
#define SIFLOWER_MAGIC_PACKET_MAC_ADDR                          0x0229
/* Magic Packet MAC Passwd registers */
#define SIFLOWER_MAGIC_PACKET_PASSWD_ADDR                       0x022F
#define SIFLOWER_PHY_WOL_PULSE_MODE_SET                         0x062a

/* Magic Packet MAC Passwd Val*/
#define SIFLOWER_MAGIC_PACKET_PASSWD1                            0x11
#define SIFLOWER_MAGIC_PACKET_PASSWD2                            0x22
#define SIFLOWER_MAGIC_PACKET_PASSWD3                            0x33
#define SIFLOWER_MAGIC_PACKET_PASSWD4                            0x44
#define SIFLOWER_MAGIC_PACKET_PASSWD5                            0x55
#define SIFLOWER_MAGIC_PACKET_PASSWD6                            0x66

/* Siflower wol config register */
#define SIFLOWER_WOL_CFG_REG0                                   0x0220
#define SIFLOWER_WOL_CFG_REG1                                   0x0221
#define SIFLOWER_WOL_CFG_REG2                                   0x0222
#define SIFLOWER_WOL_STA_REG                                    0x0223
/* 8 PHY MODE */
#define SF1211F_EXTREG_PHY_MODE_UTP_TO_RGMII                    0x00
#define SF1211F_EXTREG_PHY_MODE_FIBER_TO_RGMII                  0x10
#define SF1211F_EXTREG_PHY_MODE_UTP_OR_FIBER_TO_RGMII           0x20
#define SF1211F_EXTREG_PHY_MODE_UTP_TO_SGMII                    0x30
#define SF1211F_EXTREG_PHY_MODE_SGMII_PHY_TO_RGMII_MAC          0x40
#define SF1211F_EXTREG_PHY_MODE_SGMII_MAC_TO_RGMII_PHY          0x50
#define SF1211F_EXTREG_PHY_MODE_UTP_TO_FIBER_AUTO               0x60
#define SF1211F_EXTREG_PHY_MODE_UTP_TO_FIBER_FORCE              0x70

/* PHY EXTRW OPSREG */
#define SF1211F_EXTREG_ADDR                                     0x0E
#define SF1211F_EXTREG_DATA                                     0x0D
/* PHY PAGE SPACE */
#define SFPHY_REG_UTP_SPACE                                    0
#define SFPHY_REG_FIBER_SPACE                                  1

/* PHY PAGE SELECT */
#define SF1211F_EXTREG_PHY_MODE_PAGE_SELECT                     0x0016
#define SFPHY_REG_UTP_SPACE_SETADDR                            0x0000
#define SFPHY_REG_FIBER_SPACE_SETADDR                          0x0100
//utp
#define UTP_REG_PAUSE_CAP                                      0x0400    /* Can pause                   */
#define UTP_REG_PAUSE_ASYM                                     0x0800    /* Can pause asymetrically     */
//fiber
#define FIBER_REG_PAUSE_CAP                                    0x0080    /* Can pause                   */
#define FIBER_REG_PAUSE_ASYM                                   0x0100    /* Can pause asymetrically     */

/* specific status register */
#define SIFLOWER_SPEC_REG                                       0x0011

/* Interrupt Enable Register */
#define SIFLOWER_INTR_REG                                       0x0017
/* WOL TYPE */
#define SIFLOWER_WOL_TYPE                                       BIT(0)
/* WOL Pulse Width */
#define SIFLOWER_WOL_WIDTH1                                     BIT(1)
#define SIFLOWER_WOL_WIDTH2                                     BIT(2)
/* WOL dest addr check enable */
#define SIFLOWER_WOL_SECURE_CHECK                               BIT(5)
/* WOL crc check enable */
#define SIFLOWER_WOL_CRC_CHECK                                  BIT(4)
/* WOL dest addr check enable */
#define SIFLOWER_WOL_DESTADDR_CHECK                             BIT(5)
/* WOL Event Interrupt Enable */
#define SIFLOWER_WOL_INTR_EN                                    BIT(2)
/* WOL Enable */
#define SIFLOWER_WOL_EN                                         BIT(7)

#define SIFLOWER_WOL_RESTARTANEG                                BIT(9)
/* GET PHY MODE */
#define SFPHY_MODE_CURR                                        sfphy_get_port_type(phydev)

enum siflower_port_type_e
{
	SFPHY_PORT_TYPE_UTP,
	SFPHY_PORT_TYPE_FIBER,
	SFPHY_PORT_TYPE_COMBO,
	SFPHY_PORT_TYPE_EXT
};
enum siflower_wol_type_e
{
	SFPHY_WOL_TYPE_LEVEL,
	SFPHY_WOL_TYPE_PULSE,
	SFPHY_WOL_TYPE_EXT
};

enum siflower_wol_width_e
{
	SFPHY_WOL_WIDTH_84MS,
	SFPHY_WOL_WIDTH_168MS,
	SFPHY_WOL_WIDTH_336MS,
	SFPHY_WOL_WIDTH_672MS,
	SFPHY_WOL_WIDTH_EXT
};

typedef struct siflower_wol_cfg_s
{
	int wolen;
	int type;
	int width;
	int secure;
	int checkcrc;
	int checkdst;
}siflower_wol_cfg_t;

static int sf1211f_phy_ext_read(struct phy_device *phydev, u32 regnum)
{
	int ret, val, oldpage = 0, oldval = 0;

	phy_lock_mdio_bus(phydev);

	ret = __phy_read(phydev, SF1211F_EXTREG_ADDR);
	if (ret < 0)
		goto err_handle;
	oldval = ret;

	/* Force change to utp page */
	ret = __phy_read(phydev, SF1211F_EXTREG_PHY_MODE_PAGE_SELECT);//get old page
	if (ret < 0)
		goto err_handle;
	oldpage = ret;

	ret = __phy_write(phydev, SF1211F_EXTREG_PHY_MODE_PAGE_SELECT, SFPHY_REG_UTP_SPACE_SETADDR);
	if (ret < 0)
		goto err_handle;

	/* Default utp ext rw */
	ret = __phy_write(phydev, SF1211F_EXTREG_ADDR, regnum);
	if (ret < 0)
		goto err_handle;

	ret = __phy_read(phydev, SF1211F_EXTREG_DATA);
	if (ret < 0)
		goto err_handle;
	val = ret;

	/* Recover to old page */
	ret = __phy_write(phydev, SF1211F_EXTREG_PHY_MODE_PAGE_SELECT, oldpage);
	if (ret < 0)
		goto err_handle;

	ret = __phy_write(phydev, SF1211F_EXTREG_ADDR, oldval);
	if (ret < 0)
		goto err_handle;
	ret = val;

err_handle:
	phy_unlock_mdio_bus(phydev);
	return ret;
}

static int sf1211f_phy_ext_write(struct phy_device *phydev, u32 regnum, u16 val)
{
	int ret, oldpage = 0, oldval = 0;

	phy_lock_mdio_bus(phydev);

	ret = __phy_read(phydev, SF1211F_EXTREG_ADDR);
	if (ret < 0)
		goto err_handle;
	oldval = ret;

	/* Force change to utp page */
	ret = __phy_read(phydev, SF1211F_EXTREG_PHY_MODE_PAGE_SELECT); //get old page
	if (ret < 0)
		goto err_handle;
	oldpage = ret;

	ret = __phy_write(phydev, SF1211F_EXTREG_PHY_MODE_PAGE_SELECT, SFPHY_REG_UTP_SPACE_SETADDR);
	if (ret < 0)
		goto err_handle;

	/* Default utp ext rw */
	ret = __phy_write(phydev, SF1211F_EXTREG_ADDR, regnum);
	if (ret < 0)
		goto err_handle;

	ret = __phy_write(phydev, SF1211F_EXTREG_DATA, val);
	if (ret < 0)
		goto err_handle;

	/* Recover to old page */
	ret = __phy_write(phydev, SF1211F_EXTREG_PHY_MODE_PAGE_SELECT, oldpage);
	if (ret < 0)
		goto err_handle;

	ret = __phy_write(phydev, SF1211F_EXTREG_ADDR, oldval);
	if (ret < 0)
		goto err_handle;

err_handle:
	phy_unlock_mdio_bus(phydev);
	return ret;

}

static int siflower_phy_select_reg_page(struct phy_device *phydev, int space)
{
	int ret;
	if (space == SFPHY_REG_UTP_SPACE)
		ret = phy_write(phydev, SF1211F_EXTREG_PHY_MODE_PAGE_SELECT, SFPHY_REG_UTP_SPACE_SETADDR);
	else
		ret = phy_write(phydev, SF1211F_EXTREG_PHY_MODE_PAGE_SELECT, SFPHY_REG_FIBER_SPACE_SETADDR);
	return ret;
}

static int siflower_phy_get_reg_page(struct phy_device *phydev)
{
	return phy_read(phydev, SF1211F_EXTREG_PHY_MODE_PAGE_SELECT);
}

static int siflower_phy_ext_read(struct phy_device *phydev, u32 regnum)
{
	return sf1211f_phy_ext_read(phydev, regnum);
}


static int siflower_phy_ext_write(struct phy_device *phydev, u32 regnum, u16 val)
{
	return sf1211f_phy_ext_write(phydev, regnum, val);
}

static int sfphy_page_read(struct phy_device *phydev, int page, u32 regnum)
{
	int ret, val, oldpage = 0, oldval = 0;

	phy_lock_mdio_bus(phydev);

	ret = __phy_read(phydev, SF1211F_EXTREG_ADDR);
	if (ret < 0)
		goto err_handle;
	oldval = ret;

	ret = __phy_read(phydev, SF1211F_EXTREG_PHY_MODE_PAGE_SELECT);
	if (ret < 0)
		goto err_handle;
	oldpage = ret;

	//Select page
	ret = __phy_write(phydev, SF1211F_EXTREG_PHY_MODE_PAGE_SELECT, (page << 8));
	if (ret < 0)
		goto err_handle;

	ret = __phy_read(phydev, regnum);
	if (ret < 0)
		goto err_handle;
	val = ret;

	/* Recover to old page */
	ret = __phy_write(phydev, SF1211F_EXTREG_PHY_MODE_PAGE_SELECT, oldpage);
	if (ret < 0)
		goto err_handle;

	ret = __phy_write(phydev, SF1211F_EXTREG_ADDR, oldval);
	if (ret < 0)
		goto err_handle;
	ret = val;

err_handle:
	phy_unlock_mdio_bus(phydev);
	return ret;
}

static int sfphy_page_write(struct phy_device *phydev, int page, u32 regnum, u16 value)
{
	int ret, oldpage = 0, oldval = 0;

	phy_lock_mdio_bus(phydev);

	ret = __phy_read(phydev, SF1211F_EXTREG_ADDR);
	if (ret < 0)
		goto err_handle;
	oldval = ret;

	ret = __phy_read(phydev, SF1211F_EXTREG_PHY_MODE_PAGE_SELECT);
	if (ret < 0)
		goto err_handle;
	oldpage = ret;

	//Select page
	ret = __phy_write(phydev, SF1211F_EXTREG_PHY_MODE_PAGE_SELECT, (page << 8));
	if(ret<0)
		goto err_handle;

	ret = __phy_write(phydev, regnum, value);
	if(ret<0)
		goto err_handle;

	/* Recover to old page */
	ret = __phy_write(phydev, SF1211F_EXTREG_PHY_MODE_PAGE_SELECT, oldpage);
	if (ret < 0)
		goto err_handle;

	ret = __phy_write(phydev, SF1211F_EXTREG_ADDR, oldval);
	if (ret < 0)
		goto err_handle;

err_handle:
	phy_unlock_mdio_bus(phydev);
	return ret;
}

//get port type
static int sfphy_get_port_type(struct phy_device *phydev)
{
	int ret, mode;

	ret = siflower_phy_ext_read(phydev, SF1211F_EXTREG_GET_PORT_PHY_MODE);
	if (ret < 0)
		return ret;
	ret &= SF1211F_EXTREG_PHY_MODE_MASK;

	if (ret == SF1211F_EXTREG_PHY_MODE_UTP_TO_RGMII ||
		ret == SF1211F_EXTREG_PHY_MODE_UTP_TO_SGMII) {
		mode = SFPHY_PORT_TYPE_UTP;
	} else if (ret == SF1211F_EXTREG_PHY_MODE_FIBER_TO_RGMII ||
		ret == SF1211F_EXTREG_PHY_MODE_SGMII_PHY_TO_RGMII_MAC ||
		ret == SF1211F_EXTREG_PHY_MODE_SGMII_MAC_TO_RGMII_PHY) {
		mode = SFPHY_PORT_TYPE_FIBER;
	} else {
		mode = SFPHY_PORT_TYPE_COMBO;
	}

	return mode;
}

static int sfphy_restart_aneg(struct phy_device *phydev)
{
	int ret, ctl;

	ctl = sfphy_page_read(phydev, SFPHY_REG_FIBER_SPACE, MII_BMCR);
	if (ctl < 0)
		return ctl;
	ctl |= BMCR_ANENABLE;
	ret = sfphy_page_write(phydev, SFPHY_REG_FIBER_SPACE, MII_BMCR, ctl);
	if (ret < 0)
		return ret;

	return 0;
}

static int sf1211f_config_aneg(struct phy_device *phydev)
{
	int ret, phymode, oldpage = 0;

	phymode = SFPHY_MODE_CURR;

	if (phymode == SFPHY_PORT_TYPE_UTP || phymode == SFPHY_PORT_TYPE_COMBO) {
		oldpage = siflower_phy_get_reg_page(phydev);
		if (oldpage < 0)
			return oldpage;
		ret = siflower_phy_select_reg_page(phydev, SFPHY_REG_UTP_SPACE);
		if (ret < 0)
			return ret;
		ret = genphy_config_aneg(phydev);
		if (ret < 0)
			return ret;
		ret = siflower_phy_select_reg_page(phydev, oldpage);
		if (ret < 0)
			return ret;
	}

	if (phymode == SFPHY_PORT_TYPE_FIBER || phymode == SFPHY_PORT_TYPE_COMBO) {
		oldpage = siflower_phy_get_reg_page(phydev);
		if (oldpage < 0)
			return oldpage;
		ret = siflower_phy_select_reg_page(phydev, SFPHY_REG_FIBER_SPACE);
		if (ret < 0)
			return ret;
		if (AUTONEG_ENABLE != phydev->autoneg)
			return genphy_setup_forced(phydev);
		ret = sfphy_restart_aneg(phydev);
		if (ret < 0)
			return ret;
		ret = siflower_phy_select_reg_page(phydev, oldpage);
		if (ret < 0)
			return ret;
	}
	return 0;
}

static int sf1211f_aneg_done(struct phy_device *phydev)
{
	int val = 0;

	val = phy_read(phydev, 0x16);

	if (val == SFPHY_REG_FIBER_SPACE_SETADDR) {
		val = phy_read(phydev, 0x1);
		val = phy_read(phydev, 0x1);
		return (val < 0) ? val : (val & BMSR_LSTATUS);
	}

	return genphy_aneg_done(phydev);
}

#if (SIFLOWER_PHY_WOL_FEATURE_ENABLE)
static void siflower_get_wol(struct phy_device *phydev, struct ethtool_wolinfo *wol)
{
	int val = 0;
	wol->supported = WAKE_MAGIC;
	wol->wolopts = 0;

	val = siflower_phy_ext_read(phydev, SIFLOWER_WOL_CFG_REG1);
	if (val < 0)
		return;

	if (val & SIFLOWER_WOL_EN)
		wol->wolopts |= WAKE_MAGIC;

	return;
}

static int siflower_wol_en_cfg(struct phy_device *phydev, siflower_wol_cfg_t wol_cfg)
{
	int ret, val0,val1;

	val0 = siflower_phy_ext_read(phydev, SIFLOWER_WOL_CFG_REG0);
	if (val0 < 0)
		return val0;
	val1 = siflower_phy_ext_read(phydev, SIFLOWER_WOL_CFG_REG1);
	if (val1 < 0)
		return val1;
	if (wol_cfg.wolen) {
		val1 |= SIFLOWER_WOL_EN;
		if (wol_cfg.type == SFPHY_WOL_TYPE_LEVEL) {
			val0 |= SIFLOWER_WOL_TYPE;
		} else if (wol_cfg.type == SFPHY_WOL_TYPE_PULSE) {
			ret = siflower_phy_ext_write(phydev, SIFLOWER_PHY_WOL_PULSE_MODE_SET, 0x04);//set int pin pulse
			if (ret < 0)
				return ret;
			val0 &= ~SIFLOWER_WOL_TYPE;
			if (wol_cfg.width == SFPHY_WOL_WIDTH_84MS) {
				val0 &= ~SIFLOWER_WOL_WIDTH1;
				val0 &= ~SIFLOWER_WOL_WIDTH2;
			} else if (wol_cfg.width == SFPHY_WOL_WIDTH_168MS) {
				val0 |= SIFLOWER_WOL_WIDTH1;
				val0 &= ~SIFLOWER_WOL_WIDTH2;
			} else if (wol_cfg.width == SFPHY_WOL_WIDTH_336MS) {
				val0 &= ~SIFLOWER_WOL_WIDTH1;
				val0 |= SIFLOWER_WOL_WIDTH2;
			} else if (wol_cfg.width == SFPHY_WOL_WIDTH_672MS) {
				val0 |= SIFLOWER_WOL_WIDTH1;
				val0 |= SIFLOWER_WOL_WIDTH2;
			}
		}
		if (wol_cfg.secure == SFPHY_GLB_ENABLE)
			val1 |= SIFLOWER_WOL_SECURE_CHECK;
		else
			val1 &= ~SIFLOWER_WOL_SECURE_CHECK;
		if (wol_cfg.checkcrc == SFPHY_GLB_ENABLE)
			val0 |= SIFLOWER_WOL_CRC_CHECK;
		else
			val0 &= ~SIFLOWER_WOL_CRC_CHECK;
		if (wol_cfg.checkdst == SFPHY_GLB_ENABLE)
			val0 |= SIFLOWER_WOL_DESTADDR_CHECK;
		else
			val0 &= ~SIFLOWER_WOL_DESTADDR_CHECK;
	} else {
		val1 &= ~SIFLOWER_WOL_EN;
	}

	ret = siflower_phy_ext_write(phydev, SIFLOWER_WOL_CFG_REG0, val0);
	if (ret < 0)
		return ret;
	ret = siflower_phy_ext_write(phydev, SIFLOWER_WOL_CFG_REG1, val1);
	if (ret < 0)
		return ret;
	return 0;
}

static int siflower_set_wol(struct phy_device *phydev, struct ethtool_wolinfo *wol)
{
	int ret, val, i, phymode;
	siflower_wol_cfg_t wol_cfg;

	phymode = SFPHY_MODE_CURR;
	memset(&wol_cfg,0,sizeof(siflower_wol_cfg_t));

	if (wol->wolopts & WAKE_MAGIC) {
		if (phymode == SFPHY_PORT_TYPE_UTP || phymode == SFPHY_PORT_TYPE_COMBO) {
		/* Enable the WOL interrupt */
		val = sfphy_page_read(phydev, SFPHY_REG_UTP_SPACE, SIFLOWER_INTR_REG);
		val |= SIFLOWER_WOL_INTR_EN;
		ret = sfphy_page_write(phydev, SFPHY_REG_UTP_SPACE, SIFLOWER_INTR_REG, val);
		if (ret < 0)
			return ret;
		}
		if (phymode == SFPHY_PORT_TYPE_FIBER || phymode == SFPHY_PORT_TYPE_COMBO) {
			/* Enable the WOL interrupt */
			val = sfphy_page_read(phydev, SFPHY_REG_FIBER_SPACE, SIFLOWER_INTR_REG);
			val |= SIFLOWER_WOL_INTR_EN;
			ret = sfphy_page_write(phydev, SFPHY_REG_FIBER_SPACE, SIFLOWER_INTR_REG, val);
			if (ret < 0)
				return ret;
		}
		/* Set the WOL config */
		wol_cfg.wolen = SFPHY_GLB_ENABLE;
		wol_cfg.type  = SFPHY_WOL_TYPE_PULSE;
		wol_cfg.width = SFPHY_WOL_WIDTH_672MS;
		wol_cfg.checkdst  = SFPHY_GLB_ENABLE;
		wol_cfg.checkcrc = SFPHY_GLB_ENABLE;
		ret = siflower_wol_en_cfg(phydev, wol_cfg);
		if (ret < 0)
			return ret;

		/* Store the device address for the magic packet */
		for(i = 0; i < 6; ++i) {
			ret = siflower_phy_ext_write(phydev, SIFLOWER_MAGIC_PACKET_MAC_ADDR - i,
				((phydev->attached_dev->dev_addr[i])));
			if (ret < 0)
				return ret;
		}
#if SIFLOWER_PHY_WOL_PASSWD_ENABLE
		/* Set passwd for the magic packet */
		ret = siflower_phy_ext_write(phydev, SIFLOWER_MAGIC_PACKET_PASSWD_ADDR, SIFLOWER_MAGIC_PACKET_PASSWD1);
		if (ret < 0)
			return ret;
		ret = siflower_phy_ext_write(phydev, SIFLOWER_MAGIC_PACKET_PASSWD_ADDR - 1, SIFLOWER_MAGIC_PACKET_PASSWD2);
		if (ret < 0)
			return ret;
		ret = siflower_phy_ext_write(phydev, SIFLOWER_MAGIC_PACKET_PASSWD_ADDR - 2, SIFLOWER_MAGIC_PACKET_PASSWD3);
		if (ret < 0)
			return ret;
		ret = siflower_phy_ext_write(phydev, SIFLOWER_MAGIC_PACKET_PASSWD_ADDR - 3, SIFLOWER_MAGIC_PACKET_PASSWD4);
		if (ret < 0)
			return ret;
		ret = siflower_phy_ext_write(phydev, SIFLOWER_MAGIC_PACKET_PASSWD_ADDR - 4, SIFLOWER_MAGIC_PACKET_PASSWD5);
		if (ret < 0)
			return ret;
		ret = siflower_phy_ext_write(phydev, SIFLOWER_MAGIC_PACKET_PASSWD_ADDR - 5, SIFLOWER_MAGIC_PACKET_PASSWD6);
		if (ret < 0)
			return ret;
#endif
	} else {
		wol_cfg.wolen = SFPHY_GLB_DISABLE;
		wol_cfg.type  = SFPHY_WOL_TYPE_EXT;
		wol_cfg.width = SFPHY_WOL_WIDTH_EXT;
		wol_cfg.checkdst  = SFPHY_GLB_DISABLE;
		wol_cfg.checkcrc  = SFPHY_GLB_DISABLE;
		ret = siflower_wol_en_cfg(phydev, wol_cfg);
		if (ret < 0)
			return ret;
	}

	if (val == SF1211F_EXTREG_PHY_MODE_UTP_TO_SGMII) {
		val = sfphy_page_read(phydev, SFPHY_REG_UTP_SPACE, MII_BMCR);
		val |= SIFLOWER_WOL_RESTARTANEG;
		ret = sfphy_page_write(phydev, SFPHY_REG_UTP_SPACE, MII_BMCR, val);
		if (ret < 0)
			return ret;
	}

	return 0;
}
#endif
static int sf1211f_rxc_txc_init(struct phy_device *phydev)
{
	int ret;

	ret = (siflower_phy_ext_read(phydev, SF1211F_EXTREG_GET_PORT_PHY_MODE) &
		SF1211F_EXTREG_PHY_MODE_MASK);
	if (ret < 0)
		return ret;

	if ((ret == SF1211F_EXTREG_PHY_MODE_UTP_TO_SGMII) ||
		(ret == SF1211F_EXTREG_PHY_MODE_UTP_TO_FIBER_AUTO) ||
		(ret == SF1211F_EXTREG_PHY_MODE_UTP_TO_FIBER_FORCE))
		return 0;

	// Init rxc and enable rxc
	if (ret == SF1211F_EXTREG_PHY_MODE_UTP_TO_RGMII) {
		ret = phy_read(phydev, 0x11);
		if ((ret & 0x4) == 0x0) {
			ret = siflower_phy_ext_write(phydev,0x1E0C, 0x17);
			if (ret < 0)
				return ret;
			ret = siflower_phy_ext_write(phydev,0x1E58, 0x00);
			if (ret < 0)
				return ret;
		}
	}

	if (phydev->interface == PHY_INTERFACE_MODE_RGMII_RXID){
		// Init rxc delay
		ret = siflower_phy_ext_write(phydev,0x0282, SIFLOWER_PHY_RXC_DELAY_VAL);
		if (ret < 0)
			return ret;
	}
	else if (phydev->interface == PHY_INTERFACE_MODE_RGMII_TXID){
		// Init txc delay
		ret = siflower_phy_ext_write(phydev,0x0281, SIFLOWER_PHY_TXC_DELAY_VAL);
		if (ret < 0)
			return ret;
	}
	else if(phydev->interface == PHY_INTERFACE_MODE_RGMII_ID){
		ret = siflower_phy_ext_write(phydev,0x0282, SIFLOWER_PHY_RXC_DELAY_VAL);
		if (ret < 0)
			return ret;
		ret = siflower_phy_ext_write(phydev,0x0281, SIFLOWER_PHY_TXC_DELAY_VAL);
		if (ret < 0)
			return ret;
	}

	return ret;
}

static int sf1211f_config_opt(struct phy_device *phydev)
{
	int ret;
	//100M utp optimise
	ret = siflower_phy_ext_write(phydev, 0x0149, 0x84);
	if (ret < 0)
		return ret;

	ret = siflower_phy_ext_write(phydev, 0x014A, 0x86);
	if (ret < 0)
		return ret;

	ret = siflower_phy_ext_write(phydev, 0x023C, 0x81);
	if (ret < 0)
		return ret;

	//1000M utp optimise
	ret = siflower_phy_ext_write(phydev, 0x0184, 0x85);
	if (ret < 0)
		return ret;

	ret = siflower_phy_ext_write(phydev, 0x0185, 0x86);
	if (ret < 0)
		return ret;

	ret = siflower_phy_ext_write(phydev, 0x0186, 0x85);
	if (ret < 0)
		return ret;

	ret = siflower_phy_ext_write(phydev, 0x0187, 0x86);
	if (ret < 0)
		return ret;
	return ret;
}
#if SIFLOWER_PHY_CLK_OUT_125M_ENABLE
static int sf1211f_clkout_init(struct phy_device *phydev)
{
	int ret;

	ret = siflower_phy_ext_write(phydev, 0x0272 , 0x09);

	return ret;
}
#endif

#if SIFLOWER_PHY_MODE_SET_ENABLE
//set mode
static int phy_mode_set(struct phy_device *phydev, u16 phyMode)
{
	int ret, num = 0;

	ret = siflower_phy_ext_read(phydev, 0xC417);
	if (ret < 0)
		return ret;

	ret = (ret & 0xF0) | (0x8 | phyMode);

	ret = siflower_phy_ext_write(phydev, 0xC417, ret);
	if (ret < 0)
		return ret;

	while ((siflower_phy_ext_read(phydev, 0xC415) & 0x07) != phyMode) {
		msleep(10);
		if(++num == 5) {
			printk("Phy Mode Set Time Out!\r\n");
			break;
		}
	}

	while (siflower_phy_ext_read(phydev, 0xC413) != 0) {
		msleep(10);
		if(++num == 10) {
			printk("Phy Mode Set Time Out!\r\n");
			break;
		}
	}

	return 0;
}
#endif

static int sf1240_config_init(struct phy_device *phydev)
{
	int ret;
		ret = genphy_read_abilities(phydev);
	if (ret < 0)
		return ret;

	linkmode_mod_bit(ETHTOOL_LINK_MODE_1000baseT_Full_BIT,
			phydev->supported, ESTATUS_1000_TFULL);
	linkmode_mod_bit(ETHTOOL_LINK_MODE_1000baseT_Full_BIT,
			phydev->advertising, ESTATUS_1000_TFULL);
	return 0;
}

static int sf1211f_config_init(struct phy_device *phydev)
{
	int ret, phymode;

#if SIFLOWER_PHY_WOL_FEATURE_ENABLE
	struct ethtool_wolinfo wol;
#endif

#if SIFLOWER_PHY_MODE_SET_ENABLE
	ret = phy_mode_set(phydev, 0x0);
	if (ret < 0)
		return ret;
#endif
	phymode = SFPHY_MODE_CURR;

	if (phymode == SFPHY_PORT_TYPE_UTP || phymode == SFPHY_PORT_TYPE_COMBO) {
		siflower_phy_select_reg_page(phydev, SFPHY_REG_UTP_SPACE);
		ret = genphy_read_abilities(phydev);
		if (ret < 0)
			return ret;
	} else {
		siflower_phy_select_reg_page(phydev, SFPHY_REG_FIBER_SPACE);
		ret = genphy_read_abilities(phydev);
		if (ret < 0)
			return ret;

		linkmode_mod_bit(ETHTOOL_LINK_MODE_1000baseT_Full_BIT,
				phydev->supported, ESTATUS_1000_TFULL);
		linkmode_mod_bit(ETHTOOL_LINK_MODE_1000baseT_Full_BIT,
				phydev->advertising, ESTATUS_1000_TFULL);
	}

	ret = sf1211f_rxc_txc_init(phydev);
	if (ret < 0)
		return ret;

	ret = sf1211f_config_opt(phydev);
	if (ret < 0)
		return ret;

#if SIFLOWER_PHY_CLK_OUT_125M_ENABLE
	ret = sf1211f_clkout_init(phydev);
	if (ret < 0)
		return ret;
#endif

#if SIFLOWER_PHY_WOL_FEATURE_ENABLE
	wol.wolopts = 0;
	wol.supported = WAKE_MAGIC;
	wol.wolopts |= WAKE_MAGIC;
	siflower_set_wol(phydev, &wol);
#endif

	return 0;
}

static struct phy_driver sf_phy_drivers[] = {
	{
		.phy_id             = SF1211F_PHY_ID,
		.phy_id_mask        = SIFLOWER_PHY_ID_MASK,
		.name               = "SF1211F Gigabit Ethernet",
		.features           = PHY_GBIT_FEATURES,
		.flags              = PHY_POLL,
		.config_init        = sf1211f_config_init,
		.config_aneg        = sf1211f_config_aneg,
		.aneg_done          = sf1211f_aneg_done,
		.write_mmd          = genphy_write_mmd_unsupported,
		.read_mmd           = genphy_read_mmd_unsupported,
		.suspend            = genphy_suspend,
		.resume             = genphy_resume,
#if SIFLOWER_PHY_WOL_FEATURE_ENABLE
		.get_wol            = &siflower_get_wol,
		.set_wol            = &siflower_set_wol,
#endif
	},

	{
		.phy_id             = SF1240_PHY_ID,
		.phy_id_mask        = SIFLOWER_PHY_ID_MASK,
		.name               = "SF1240 Gigabit Ethernet",
		.features           = PHY_GBIT_FEATURES,
		.flags              = PHY_POLL,
		.config_init        = sf1240_config_init,
		.config_aneg        = genphy_config_aneg,
		.write_mmd          = genphy_write_mmd_unsupported,
		.read_mmd           = genphy_read_mmd_unsupported,
		.suspend            = genphy_suspend,
		.resume             = genphy_resume,
	},
};

/* for linux 4.x */
module_phy_driver(sf_phy_drivers);

static struct mdio_device_id __maybe_unused siflower_phy_tbl[] = {
	{ SF1211F_PHY_ID, SIFLOWER_PHY_ID_MASK },
	{ SF1240_PHY_ID, SIFLOWER_PHY_ID_MASK },
	{},
};

MODULE_DEVICE_TABLE(mdio, siflower_phy_tbl);

MODULE_DESCRIPTION("Siflower PHY driver");
MODULE_LICENSE("GPL");
