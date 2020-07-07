// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2018 MediaTek Inc.
 * Author: Zhanguo Ju <zhanguo.ju@mediatek.com>
 */

#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/hrtimer.h>

#include "mt753x.h"
#include "mt753x_regs.h"

/* MT7531 registers */
#define SGMII_REG_BASE			0x5000
#define SGMII_REG_PORT_BASE		0x1000
#define SGMII_REG(p, r)			(SGMII_REG_BASE + \
					(p) * SGMII_REG_PORT_BASE + (r))
#define PCS_CONTROL_1(p)		SGMII_REG(p, 0x00)
#define SGMII_MODE(p)			SGMII_REG(p, 0x20)
#define QPHY_PWR_STATE_CTRL(p)		SGMII_REG(p, 0xe8)
#define PHYA_CTRL_SIGNAL3(p)		SGMII_REG(p, 0x128)

/* Fields of PCS_CONTROL_1 */
#define SGMII_LINK_STATUS		BIT(18)
#define SGMII_AN_ENABLE			BIT(12)
#define SGMII_AN_RESTART		BIT(9)

/* Fields of SGMII_MODE */
#define SGMII_REMOTE_FAULT_DIS		BIT(8)
#define SGMII_IF_MODE_FORCE_DUPLEX	BIT(4)
#define SGMII_IF_MODE_FORCE_SPEED_S	0x2
#define SGMII_IF_MODE_FORCE_SPEED_M	0x0c
#define SGMII_IF_MODE_ADVERT_AN		BIT(1)

/* Values of SGMII_IF_MODE_FORCE_SPEED */
#define SGMII_IF_MODE_FORCE_SPEED_10	0
#define SGMII_IF_MODE_FORCE_SPEED_100	1
#define SGMII_IF_MODE_FORCE_SPEED_1000	2

/* Fields of QPHY_PWR_STATE_CTRL */
#define PHYA_PWD			BIT(4)

/* Fields of PHYA_CTRL_SIGNAL3 */
#define RG_TPHY_SPEED_S			2
#define RG_TPHY_SPEED_M			0x0c

/* Values of RG_TPHY_SPEED */
#define RG_TPHY_SPEED_1000		0
#define RG_TPHY_SPEED_2500		1

/* Unique fields of (M)HWSTRAP for MT7531 */
#define XTAL_FSEL_S			7
#define XTAL_FSEL_M			BIT(7)
#define PHY_EN				BIT(6)
#define CHG_STRAP			BIT(8)

/* Efuse Register Define */
#define GBE_EFUSE			0x7bc8
#define GBE_SEL_EFUSE_EN		BIT(0)

/* PHY ENABLE Register bitmap define */
#define PHY_DEV1F			0x1f
#define PHY_DEV1F_REG_44		0x44
#define PHY_DEV1F_REG_104		0x104
#define PHY_DEV1F_REG_10A		0x10a
#define PHY_DEV1F_REG_10B		0x10b
#define PHY_DEV1F_REG_10C		0x10c
#define PHY_DEV1F_REG_10D		0x10d
#define PHY_DEV1F_REG_268		0x268
#define PHY_DEV1F_REG_269		0x269
#define PHY_DEV1F_REG_403		0x403

/* Fields of PHY_DEV1F_REG_403 */
#define GBE_EFUSE_SETTING		BIT(3)
#define PHY_EN_BYPASS_MODE		BIT(4)
#define POWER_ON_OFF			BIT(5)
#define PHY_PLL_M			GENMASK(9, 8)
#define PHY_PLL_SEL(x)			(((x) << 8) & GENMASK(9, 8))

/* PHY EEE Register bitmap of define */
#define PHY_DEV07			0x07
#define PHY_DEV07_REG_03C		0x3c

/* PHY Extend Register 0x14 bitmap of define */
#define PHY_EXT_REG_14			0x14

/* Fields of PHY_EXT_REG_14 */
#define PHY_EN_DOWN_SHFIT		BIT(4)

/* PHY Extend Register 0x17 bitmap of define */
#define PHY_EXT_REG_17			0x17

/* Fields of PHY_EXT_REG_17 */
#define PHY_LINKDOWN_POWER_SAVING_EN	BIT(4)

/* PHY Token Ring Register 0x10 bitmap of define */
#define PHY_TR_REG_10			0x10

/* PHY Token Ring Register 0x12 bitmap of define */
#define PHY_TR_REG_12			0x12

/* PHY DEV 0x1e Register bitmap of define */
#define PHY_DEV1E			0x1e
#define PHY_DEV1E_REG_13		0x13
#define PHY_DEV1E_REG_14		0x14
#define PHY_DEV1E_REG_41		0x41
#define PHY_DEV1E_REG_A6		0xa6
#define PHY_DEV1E_REG_0C6		0x0c6
#define PHY_DEV1E_REG_0FE		0x0fe
#define PHY_DEV1E_REG_123		0x123
#define PHY_DEV1E_REG_189		0x189

/* Fields of PHY_DEV1E_REG_0C6 */
#define PHY_POWER_SAVING_S		8
#define PHY_POWER_SAVING_M		0x300
#define PHY_POWER_SAVING_TX		0x0

/* Fields of PHY_DEV1E_REG_189 */
#define DESCRAMBLER_CLEAR_EN		0x1

/* Values of XTAL_FSEL_S */
#define XTAL_40MHZ			0
#define XTAL_25MHZ			1

#define PLLGP_EN			0x7820
#define EN_COREPLL			BIT(2)
#define SW_CLKSW			BIT(1)
#define SW_PLLGP			BIT(0)

#define PLLGP_CR0			0x78a8
#define RG_COREPLL_EN			BIT(22)
#define RG_COREPLL_POSDIV_S		23
#define RG_COREPLL_POSDIV_M		0x3800000
#define RG_COREPLL_SDM_PCW_S		1
#define RG_COREPLL_SDM_PCW_M		0x3ffffe
#define RG_COREPLL_SDM_PCW_CHG		BIT(0)

/* TOP Signals Status Register */
#define TOP_SIG_SR			0x780c
#define PAD_DUAL_SGMII_EN		BIT(1)

/* RGMII and SGMII PLL clock */
#define ANA_PLLGP_CR2			0x78b0
#define ANA_PLLGP_CR5			0x78bc

/* GPIO mode define */
#define GPIO_MODE_REGS(x)		(0x7c0c + (((x) / 8) * 4))
#define GPIO_MODE_S			4

/* GPIO GROUP IOLB SMT0 Control */
#define SMT0_IOLB			0x7f04
#define SMT_IOLB_5_SMI_MDC_EN		BIT(5)

/* Unique fields of PMCR for MT7531 */
#define FORCE_MODE_EEE1G		BIT(25)
#define FORCE_MODE_EEE100		BIT(26)
#define FORCE_MODE_TX_FC		BIT(27)
#define FORCE_MODE_RX_FC		BIT(28)
#define FORCE_MODE_DPX			BIT(29)
#define FORCE_MODE_SPD			BIT(30)
#define FORCE_MODE_LNK			BIT(31)
#define FORCE_MODE			BIT(15)

#define CHIP_REV			0x781C
#define CHIP_NAME_S			16
#define CHIP_NAME_M			0xffff0000
#define CHIP_REV_S			0
#define CHIP_REV_M			0x0f
#define CHIP_REV_E1			0x0

#define CLKGEN_CTRL			0x7500
#define CLK_SKEW_OUT_S			8
#define CLK_SKEW_OUT_M			0x300
#define CLK_SKEW_IN_S			6
#define CLK_SKEW_IN_M			0xc0
#define RXCLK_NO_DELAY			BIT(5)
#define TXCLK_NO_REVERSE		BIT(4)
#define GP_MODE_S			1
#define GP_MODE_M			0x06
#define GP_CLK_EN			BIT(0)

/* Values of GP_MODE */
#define GP_MODE_RGMII			0
#define GP_MODE_MII			1
#define GP_MODE_REV_MII			2

/* Values of CLK_SKEW_IN */
#define CLK_SKEW_IN_NO_CHANGE		0
#define CLK_SKEW_IN_DELAY_100PPS	1
#define CLK_SKEW_IN_DELAY_200PPS	2
#define CLK_SKEW_IN_REVERSE		3

/* Values of CLK_SKEW_OUT */
#define CLK_SKEW_OUT_NO_CHANGE		0
#define CLK_SKEW_OUT_DELAY_100PPS	1
#define CLK_SKEW_OUT_DELAY_200PPS	2
#define CLK_SKEW_OUT_REVERSE		3

/* Proprietory Control Register of Internal Phy device 0x1e */
#define RXADC_CONTROL_3			0xc2
#define RXADC_LDO_CONTROL_2		0xd3

/* Proprietory Control Register of Internal Phy device 0x1f */
#define TXVLD_DA_271			0x271
#define TXVLD_DA_272			0x272
#define TXVLD_DA_273			0x273

/* DSP Channel and NOD_ADDR*/
#define DSP_CH				0x2
#define DSP_NOD_ADDR			0xD

/* gpio pinmux pins and functions define */
static int gpio_int_pins[] = {0};
static int gpio_int_funcs[] = {1};
static int gpio_mdc_pins[] = {11, 20};
static int gpio_mdc_funcs[] = {2, 2};
static int gpio_mdio_pins[] = {12, 21};
static int gpio_mdio_funcs[] = {2, 2};

static int mt7531_set_port_sgmii_force_mode(struct gsw_mt753x *gsw, u32 port,
					    struct mt753x_port_cfg *port_cfg)
{
	u32 speed, port_base, val;
	ktime_t timeout;
	u32 timeout_us;

	if (port < 5 || port >= MT753X_NUM_PORTS) {
		dev_info(gsw->dev, "port %d is not a SGMII port\n", port);
		return -EINVAL;
	}

	port_base = port - 5;

	switch (port_cfg->speed) {
	case MAC_SPD_1000:
		speed = RG_TPHY_SPEED_1000;
		break;
	case MAC_SPD_2500:
		speed = RG_TPHY_SPEED_2500;
		break;
	default:
		dev_info(gsw->dev, "invalid SGMII speed idx %d for port %d\n",
			 port_cfg->speed, port);

		speed = RG_TPHY_SPEED_1000;
	}

	/* Step 1: Speed select register setting */
	val = mt753x_reg_read(gsw, PHYA_CTRL_SIGNAL3(port_base));
	val &= ~RG_TPHY_SPEED_M;
	val |= speed << RG_TPHY_SPEED_S;
	mt753x_reg_write(gsw, PHYA_CTRL_SIGNAL3(port_base), val);

	/* Step 2 : Disable AN */
	val = mt753x_reg_read(gsw, PCS_CONTROL_1(port_base));
	val &= ~SGMII_AN_ENABLE;
	mt753x_reg_write(gsw, PCS_CONTROL_1(port_base), val);

	/* Step 3: SGMII force mode setting */
	val = mt753x_reg_read(gsw, SGMII_MODE(port_base));
	val &= ~SGMII_IF_MODE_ADVERT_AN;
	val &= ~SGMII_IF_MODE_FORCE_SPEED_M;
	val |= SGMII_IF_MODE_FORCE_SPEED_1000 << SGMII_IF_MODE_FORCE_SPEED_S;
	val |= SGMII_IF_MODE_FORCE_DUPLEX;
	/* For sgmii force mode, 0 is full duplex and 1 is half duplex */
	if (port_cfg->duplex)
		val &= ~SGMII_IF_MODE_FORCE_DUPLEX;

	mt753x_reg_write(gsw, SGMII_MODE(port_base), val);

	/* Step 4: XXX: Disable Link partner's AN and set force mode */

	/* Step 5: XXX: Special setting for PHYA ==> reserved for flexible */

	/* Step 6 : Release PHYA power down state */
	val = mt753x_reg_read(gsw, QPHY_PWR_STATE_CTRL(port_base));
	val &= ~PHYA_PWD;
	mt753x_reg_write(gsw, QPHY_PWR_STATE_CTRL(port_base), val);

	/* Step 7 : Polling SGMII_LINK_STATUS */
	timeout_us = 2000000;
	timeout = ktime_add_us(ktime_get(), timeout_us);
	while (1) {
		val = mt753x_reg_read(gsw, PCS_CONTROL_1(port_base));
		val &= SGMII_LINK_STATUS;

		if (val)
			break;

		if (ktime_compare(ktime_get(), timeout) > 0)
			return -ETIMEDOUT;
	}

	return 0;
}

static int mt7531_set_port_sgmii_an_mode(struct gsw_mt753x *gsw, u32 port,
					 struct mt753x_port_cfg *port_cfg)
{
	u32 speed, port_base, val;
	ktime_t timeout;
	u32 timeout_us;

	if (port < 5 || port >= MT753X_NUM_PORTS) {
		dev_info(gsw->dev, "port %d is not a SGMII port\n", port);
		return -EINVAL;
	}

	port_base = port - 5;

	switch (port_cfg->speed) {
	case MAC_SPD_1000:
		speed = RG_TPHY_SPEED_1000;
		break;
	case MAC_SPD_2500:
		speed = RG_TPHY_SPEED_2500;
		break;
	default:
		dev_info(gsw->dev, "invalid SGMII speed idx %d for port %d\n",
			 port_cfg->speed, port);

		speed = RG_TPHY_SPEED_1000;
	}

	/* Step 1: Speed select register setting */
	val = mt753x_reg_read(gsw, PHYA_CTRL_SIGNAL3(port_base));
	val &= ~RG_TPHY_SPEED_M;
	val |= speed << RG_TPHY_SPEED_S;
	mt753x_reg_write(gsw, PHYA_CTRL_SIGNAL3(port_base), val);

	/* Step 2: Remote fault disable */
	val = mt753x_reg_read(gsw, SGMII_MODE(port));
	val |= SGMII_REMOTE_FAULT_DIS;
	mt753x_reg_write(gsw, SGMII_MODE(port), val);

	/* Step 3: Setting Link partner's AN enable = 1 */

	/* Step 4: Setting Link partner's device ability for speed/duplex */

	/* Step 5: AN re-start */
	val = mt753x_reg_read(gsw, PCS_CONTROL_1(port));
	val |= SGMII_AN_RESTART;
	mt753x_reg_write(gsw, PCS_CONTROL_1(port), val);

	/* Step 6: Special setting for PHYA ==> reserved for flexible */

	/* Step 7 : Polling SGMII_LINK_STATUS */
	timeout_us = 2000000;
	timeout = ktime_add_us(ktime_get(), timeout_us);
	while (1) {
		val = mt753x_reg_read(gsw, PCS_CONTROL_1(port_base));
		val &= SGMII_LINK_STATUS;

		if (val)
			break;

		if (ktime_compare(ktime_get(), timeout) > 0)
			return -ETIMEDOUT;
	}

	return 0;
}

static int mt7531_set_port_rgmii(struct gsw_mt753x *gsw, u32 port)
{
	u32 val;

	if (port != 5) {
		dev_info(gsw->dev, "RGMII mode is not available for port %d\n",
			 port);
		return -EINVAL;
	}

	val = mt753x_reg_read(gsw, CLKGEN_CTRL);
	val |= GP_CLK_EN;
	val &= ~GP_MODE_M;
	val |= GP_MODE_RGMII << GP_MODE_S;
	val |= TXCLK_NO_REVERSE;
	val |= RXCLK_NO_DELAY;
	val &= ~CLK_SKEW_IN_M;
	val |= CLK_SKEW_IN_NO_CHANGE << CLK_SKEW_IN_S;
	val &= ~CLK_SKEW_OUT_M;
	val |= CLK_SKEW_OUT_NO_CHANGE << CLK_SKEW_OUT_S;
	mt753x_reg_write(gsw, CLKGEN_CTRL, val);

	return 0;
}

static int mt7531_mac_port_setup(struct gsw_mt753x *gsw, u32 port,
				 struct mt753x_port_cfg *port_cfg)
{
	u32 pmcr;
	u32 speed;

	if (port < 5 || port >= MT753X_NUM_PORTS) {
		dev_info(gsw->dev, "port %d is not a MAC port\n", port);
		return -EINVAL;
	}

	if (port_cfg->enabled) {
		pmcr = (IPG_96BIT_WITH_SHORT_IPG << IPG_CFG_S) |
		       MAC_MODE | MAC_TX_EN | MAC_RX_EN |
		       BKOFF_EN | BACKPR_EN;

		if (port_cfg->force_link) {
			/* PMCR's speed field 0x11 is reserved,
			 * sw should set 0x10
			 */
			speed = port_cfg->speed;
			if (port_cfg->speed == MAC_SPD_2500)
				speed = MAC_SPD_1000;

			pmcr |= FORCE_MODE_LNK | FORCE_LINK |
				FORCE_MODE_SPD | FORCE_MODE_DPX |
				FORCE_MODE_RX_FC | FORCE_MODE_TX_FC |
				FORCE_RX_FC | FORCE_TX_FC |
				(speed << FORCE_SPD_S);

			if (port_cfg->duplex)
				pmcr |= FORCE_DPX;
		}
	} else {
		pmcr = FORCE_MODE_LNK;
	}

	switch (port_cfg->phy_mode) {
	case PHY_INTERFACE_MODE_RGMII:
		mt7531_set_port_rgmii(gsw, port);
		break;
	case PHY_INTERFACE_MODE_SGMII:
		if (port_cfg->force_link)
			mt7531_set_port_sgmii_force_mode(gsw, port, port_cfg);
		else
			mt7531_set_port_sgmii_an_mode(gsw, port, port_cfg);
		break;
	default:
		if (port_cfg->enabled)
			dev_info(gsw->dev, "%s is not supported by port %d\n",
				 phy_modes(port_cfg->phy_mode), port);

		pmcr = FORCE_MODE_LNK;
	}

	mt753x_reg_write(gsw, PMCR(port), pmcr);

	return 0;
}

static void mt7531_core_pll_setup(struct gsw_mt753x *gsw)
{
	u32 hwstrap;
	u32 val;

	val = mt753x_reg_read(gsw, TOP_SIG_SR);
	if (val & PAD_DUAL_SGMII_EN)
		return;

	hwstrap = mt753x_reg_read(gsw, HWSTRAP);

	switch ((hwstrap & XTAL_FSEL_M) >> XTAL_FSEL_S) {
	case XTAL_25MHZ:
		/* Step 1 : Disable MT7531 COREPLL */
		val = mt753x_reg_read(gsw, PLLGP_EN);
		val &= ~EN_COREPLL;
		mt753x_reg_write(gsw, PLLGP_EN, val);

		/* Step 2: switch to XTAL output */
		val = mt753x_reg_read(gsw, PLLGP_EN);
		val |= SW_CLKSW;
		mt753x_reg_write(gsw, PLLGP_EN, val);

		val = mt753x_reg_read(gsw, PLLGP_CR0);
		val &= ~RG_COREPLL_EN;
		mt753x_reg_write(gsw, PLLGP_CR0, val);

		/* Step 3: disable PLLGP and enable program PLLGP */
		val = mt753x_reg_read(gsw, PLLGP_EN);
		val |= SW_PLLGP;
		mt753x_reg_write(gsw, PLLGP_EN, val);

		/* Step 4: program COREPLL output frequency to 500MHz */
		val = mt753x_reg_read(gsw, PLLGP_CR0);
		val &= ~RG_COREPLL_POSDIV_M;
		val |= 2 << RG_COREPLL_POSDIV_S;
		mt753x_reg_write(gsw, PLLGP_CR0, val);
		usleep_range(25, 35);

		val = mt753x_reg_read(gsw, PLLGP_CR0);
		val &= ~RG_COREPLL_SDM_PCW_M;
		val |= 0x140000 << RG_COREPLL_SDM_PCW_S;
		mt753x_reg_write(gsw, PLLGP_CR0, val);

		/* Set feedback divide ratio update signal to high */
		val = mt753x_reg_read(gsw, PLLGP_CR0);
		val |= RG_COREPLL_SDM_PCW_CHG;
		mt753x_reg_write(gsw, PLLGP_CR0, val);
		/* Wait for at least 16 XTAL clocks */
		usleep_range(10, 20);

		/* Step 5: set feedback divide ratio update signal to low */
		val = mt753x_reg_read(gsw, PLLGP_CR0);
		val &= ~RG_COREPLL_SDM_PCW_CHG;
		mt753x_reg_write(gsw, PLLGP_CR0, val);

		/* Enable 325M clock for SGMII */
		mt753x_reg_write(gsw, ANA_PLLGP_CR5, 0xad0000);

		/* Enable 250SSC clock for RGMII */
		mt753x_reg_write(gsw, ANA_PLLGP_CR2, 0x4f40000);

		/* Step 6: Enable MT7531 PLL */
		val = mt753x_reg_read(gsw, PLLGP_CR0);
		val |= RG_COREPLL_EN;
		mt753x_reg_write(gsw, PLLGP_CR0, val);

		val = mt753x_reg_read(gsw, PLLGP_EN);
		val |= EN_COREPLL;
		mt753x_reg_write(gsw, PLLGP_EN, val);
		usleep_range(25, 35);

		break;
	case XTAL_40MHZ:
		/* Step 1 : Disable MT7531 COREPLL */
		val = mt753x_reg_read(gsw, PLLGP_EN);
		val &= ~EN_COREPLL;
		mt753x_reg_write(gsw, PLLGP_EN, val);

		/* Step 2: switch to XTAL output */
		val = mt753x_reg_read(gsw, PLLGP_EN);
		val |= SW_CLKSW;
		mt753x_reg_write(gsw, PLLGP_EN, val);

		val = mt753x_reg_read(gsw, PLLGP_CR0);
		val &= ~RG_COREPLL_EN;
		mt753x_reg_write(gsw, PLLGP_CR0, val);

		/* Step 3: disable PLLGP and enable program PLLGP */
		val = mt753x_reg_read(gsw, PLLGP_EN);
		val |= SW_PLLGP;
		mt753x_reg_write(gsw, PLLGP_EN, val);

		/* Step 4: program COREPLL output frequency to 500MHz */
		val = mt753x_reg_read(gsw, PLLGP_CR0);
		val &= ~RG_COREPLL_POSDIV_M;
		val |= 2 << RG_COREPLL_POSDIV_S;
		mt753x_reg_write(gsw, PLLGP_CR0, val);
		usleep_range(25, 35);

		val = mt753x_reg_read(gsw, PLLGP_CR0);
		val &= ~RG_COREPLL_SDM_PCW_M;
		val |= 0x190000 << RG_COREPLL_SDM_PCW_S;
		mt753x_reg_write(gsw, PLLGP_CR0, val);

		/* Set feedback divide ratio update signal to high */
		val = mt753x_reg_read(gsw, PLLGP_CR0);
		val |= RG_COREPLL_SDM_PCW_CHG;
		mt753x_reg_write(gsw, PLLGP_CR0, val);
		/* Wait for at least 16 XTAL clocks */
		usleep_range(10, 20);

		/* Step 5: set feedback divide ratio update signal to low */
		val = mt753x_reg_read(gsw, PLLGP_CR0);
		val &= ~RG_COREPLL_SDM_PCW_CHG;
		mt753x_reg_write(gsw, PLLGP_CR0, val);

		/* Enable 325M clock for SGMII */
		mt753x_reg_write(gsw, ANA_PLLGP_CR5, 0xad0000);

		/* Enable 250SSC clock for RGMII */
		mt753x_reg_write(gsw, ANA_PLLGP_CR2, 0x4f40000);

		/* Step 6: Enable MT7531 PLL */
		val = mt753x_reg_read(gsw, PLLGP_CR0);
		val |= RG_COREPLL_EN;
		mt753x_reg_write(gsw, PLLGP_CR0, val);

		val = mt753x_reg_read(gsw, PLLGP_EN);
		val |= EN_COREPLL;
		mt753x_reg_write(gsw, PLLGP_EN, val);
		usleep_range(25, 35);
		break;
	}
}

static int mt7531_internal_phy_calibration(struct gsw_mt753x *gsw)
{
	return 0;
}

static int mt7531_sw_detect(struct gsw_mt753x *gsw, struct chip_rev *crev)
{
	u32 rev, topsig;

	rev = mt753x_reg_read(gsw, CHIP_REV);

	if (((rev & CHIP_NAME_M) >> CHIP_NAME_S) == MT7531) {
		if (crev) {
			topsig = mt753x_reg_read(gsw, TOP_SIG_SR);

			crev->rev = rev & CHIP_REV_M;
			crev->name = topsig & PAD_DUAL_SGMII_EN ?
				     "MT7531AE" : "MT7531BE";
		}

		return 0;
	}

	return -ENODEV;
}

static void pinmux_set_mux_7531(struct gsw_mt753x *gsw, u32 pin, u32 mode)
{
	u32 val;

	val = mt753x_reg_read(gsw, GPIO_MODE_REGS(pin));
	val &= ~(0xf << (pin & 7) * GPIO_MODE_S);
	val |= mode << (pin & 7) * GPIO_MODE_S;
	mt753x_reg_write(gsw, GPIO_MODE_REGS(pin), val);
}

static int mt7531_set_gpio_pinmux(struct gsw_mt753x *gsw)
{
	u32 group = 0;
	struct device_node *np = gsw->dev->of_node;

	/* Set GPIO 0 interrupt mode */
	pinmux_set_mux_7531(gsw, gpio_int_pins[0], gpio_int_funcs[0]);

	of_property_read_u32(np, "mediatek,mdio_master_pinmux", &group);

	/* group = 0: do nothing, 1: 1st group (AE), 2: 2nd group (BE) */
	if (group > 0 && group <= 2) {
		group--;
		pinmux_set_mux_7531(gsw, gpio_mdc_pins[group],
				    gpio_mdc_funcs[group]);
		pinmux_set_mux_7531(gsw, gpio_mdio_pins[group],
				    gpio_mdio_funcs[group]);
	}

	return 0;
}

static void mt7531_phy_pll_setup(struct gsw_mt753x *gsw)
{
	u32 hwstrap;
	u32 val;

	hwstrap = mt753x_reg_read(gsw, HWSTRAP);

	switch ((hwstrap & XTAL_FSEL_M) >> XTAL_FSEL_S) {
	case XTAL_25MHZ:
		/* disable pll auto calibration */
		gsw->mmd_write(gsw, 0, PHY_DEV1F, PHY_DEV1F_REG_104, 0x608);

		/* change pll sel */
		val = gsw->mmd_read(gsw, 0, PHY_DEV1F,
				     PHY_DEV1F_REG_403);
		val &= ~(PHY_PLL_M);
		val |= PHY_PLL_SEL(3);
		gsw->mmd_write(gsw, 0, PHY_DEV1F, PHY_DEV1F_REG_403, val);

		/* set divider ratio */
		gsw->mmd_write(gsw, 0, PHY_DEV1F,
			       PHY_DEV1F_REG_10A, 0x1009);

		/* set divider ratio */
		gsw->mmd_write(gsw, 0, PHY_DEV1F, PHY_DEV1F_REG_10B, 0x7c6);

		/* capacitance and resistance adjustment */
		gsw->mmd_write(gsw, 0, PHY_DEV1F,
			       PHY_DEV1F_REG_10C, 0xa8be);

		break;
	case XTAL_40MHZ:
		/* disable pll auto calibration */
		gsw->mmd_write(gsw, 0, PHY_DEV1F, PHY_DEV1F_REG_104, 0x608);

		/* change pll sel */
		val = gsw->mmd_read(gsw, 0, PHY_DEV1F,
				     PHY_DEV1F_REG_403);
		val &= ~(PHY_PLL_M);
		val |= PHY_PLL_SEL(3);
		gsw->mmd_write(gsw, 0, PHY_DEV1F, PHY_DEV1F_REG_403, val);

		/* set divider ratio */
		gsw->mmd_write(gsw, 0, PHY_DEV1F,
			       PHY_DEV1F_REG_10A, 0x1018);

		/* set divider ratio */
		gsw->mmd_write(gsw, 0, PHY_DEV1F, PHY_DEV1F_REG_10B, 0xc676);

		/* capacitance and resistance adjustment */
		gsw->mmd_write(gsw, 0, PHY_DEV1F,
			       PHY_DEV1F_REG_10C, 0xd8be);
		break;
	}

	/* power down pll. additional delay is not required via mdio access */
	gsw->mmd_write(gsw, 0, PHY_DEV1F, PHY_DEV1F_REG_10D, 0x10);

	/* power up pll */
	gsw->mmd_write(gsw, 0, PHY_DEV1F, PHY_DEV1F_REG_10D, 0x14);
}

static void mt7531_phy_setting(struct gsw_mt753x *gsw)
{
	int i;
	u32 val;

	/* Adjust DAC TX Delay */
	gsw->mmd_write(gsw, 0, PHY_DEV1F, PHY_DEV1F_REG_44, 0xc0);

	for (i = 0; i < MT753X_NUM_PHYS; i++) {
		/* Disable EEE */
		gsw->mmd_write(gsw, i, PHY_DEV07, PHY_DEV07_REG_03C, 0);

		/* Enable HW auto downshift */
		gsw->mii_write(gsw, i, 0x1f, 0x1);
		val = gsw->mii_read(gsw, i, PHY_EXT_REG_14);
		val |= PHY_EN_DOWN_SHFIT;
		gsw->mii_write(gsw, i, PHY_EXT_REG_14, val);

		/* Increase SlvDPSready time */
		gsw->mii_write(gsw, i, 0x1f, 0x52b5);
		gsw->mii_write(gsw, i, PHY_TR_REG_10, 0xafae);
		gsw->mii_write(gsw, i, PHY_TR_REG_12, 0x2f);
		gsw->mii_write(gsw, i, PHY_TR_REG_10, 0x8fae);
		gsw->mii_write(gsw, i, 0x1f, 0);

		/* Adjust 100_mse_threshold */
		gsw->mmd_write(gsw, i, PHY_DEV1E, PHY_DEV1E_REG_123, 0xffff);

		/* Disable mcc */
		gsw->mmd_write(gsw, i, PHY_DEV1E, PHY_DEV1E_REG_A6, 0x300);

		/* PHY link down power saving enable */
		val = gsw->mii_read(gsw, i, PHY_EXT_REG_17);
		val |= PHY_LINKDOWN_POWER_SAVING_EN;
		gsw->mii_write(gsw, i, PHY_EXT_REG_17, val);

		val = gsw->mmd_read(gsw, i, PHY_DEV1E, PHY_DEV1E_REG_0C6);
		val &= ~PHY_POWER_SAVING_M;
		val |= PHY_POWER_SAVING_TX << PHY_POWER_SAVING_S;
		gsw->mmd_write(gsw, i, PHY_DEV1E, PHY_DEV1E_REG_0C6, val);

		/* Set TX Pair delay selection */
		gsw->mmd_write(gsw, i, PHY_DEV1E, PHY_DEV1E_REG_13, 0x404);
		gsw->mmd_write(gsw, i, PHY_DEV1E, PHY_DEV1E_REG_14, 0x404);
	}
}

static void mt7531_adjust_line_driving(struct gsw_mt753x *gsw, u32 port)
{
	/* For ADC timing margin window for LDO calibration */
	gsw->mmd_write(gsw, port, PHY_DEV1E, RXADC_LDO_CONTROL_2, 0x2222);

	/* Adjust AD sample timing */
	gsw->mmd_write(gsw, port, PHY_DEV1E, RXADC_CONTROL_3, 0x4444);

	/* Adjust Line driver current for different mode */
	gsw->mmd_write(gsw, port, PHY_DEV1F, TXVLD_DA_271, 0x2ca5);

	/* Adjust Line driver current for different mode */
	gsw->mmd_write(gsw, port, PHY_DEV1F, TXVLD_DA_272, 0xc6b);

	/* Adjust Line driver amplitude for 10BT */
	gsw->mmd_write(gsw, port, PHY_DEV1F, TXVLD_DA_273, 0x3000);

	/* Adjust RX Echo path filter */
	gsw->mmd_write(gsw, port, PHY_DEV1E, PHY_DEV1E_REG_0FE, 0x2);

	/* Adjust RX HVGA bias current */
	gsw->mmd_write(gsw, port, PHY_DEV1E, PHY_DEV1E_REG_41, 0x3333);

	/* Adjust TX class AB driver 1 */
	gsw->mmd_write(gsw, port, PHY_DEV1F, PHY_DEV1F_REG_268, 0x388);

	/* Adjust TX class AB driver 2 */
	gsw->mmd_write(gsw, port, PHY_DEV1F, PHY_DEV1F_REG_269, 0x4448);
}

static void mt7531_eee_setting(struct gsw_mt753x *gsw, u32 port)
{
	u32 tr_reg_control;
	u32 val;

	/* Disable generate signal to clear the scramble_lock when lpi mode */
	val = gsw->mmd_read(gsw, port, PHY_DEV1E, PHY_DEV1E_REG_189);
	val &= ~DESCRAMBLER_CLEAR_EN;
	gsw->mmd_write(gsw, port, PHY_DEV1E, PHY_DEV1E_REG_189, val);

	/* roll back CR*/
	gsw->mii_write(gsw, port, 0x1f, 0x52b5);
	gsw->mmd_write(gsw, port, 0x1e, 0x2d1, 0);
	tr_reg_control = (1 << 15) | (0 << 13) | (DSP_CH << 11) |
			 (DSP_NOD_ADDR << 7) | (0x8 << 1);
	gsw->mii_write(gsw, port, 17, 0x1b);
	gsw->mii_write(gsw, port, 18, 0);
	gsw->mii_write(gsw, port, 16, tr_reg_control);
	tr_reg_control = (1 << 15) | (0 << 13) | (DSP_CH << 11) |
			 (DSP_NOD_ADDR << 7) | (0xf << 1);
	gsw->mii_write(gsw, port, 17, 0);
	gsw->mii_write(gsw, port, 18, 0);
	gsw->mii_write(gsw, port, 16, tr_reg_control);

	tr_reg_control = (1 << 15) | (0 << 13) | (DSP_CH << 11) |
			 (DSP_NOD_ADDR << 7) | (0x10 << 1);
	gsw->mii_write(gsw, port, 17, 0x500);
	gsw->mii_write(gsw, port, 18, 0);
	gsw->mii_write(gsw, port, 16, tr_reg_control);
	gsw->mii_write(gsw, port, 0x1f, 0);
}

static int mt7531_sw_init(struct gsw_mt753x *gsw)
{
	int i;
	u32 val;

	gsw->phy_base = (gsw->smi_addr + 1) & MT753X_SMI_ADDR_MASK;

	gsw->mii_read = mt753x_mii_read;
	gsw->mii_write = mt753x_mii_write;
	gsw->mmd_read = mt753x_mmd_read;
	gsw->mmd_write = mt753x_mmd_write;

	for (i = 0; i < MT753X_NUM_PHYS; i++) {
		val = gsw->mii_read(gsw, i, MII_BMCR);
		val |= BMCR_ISOLATE;
		gsw->mii_write(gsw, i, MII_BMCR, val);
	}

	/* Force MAC link down before reset */
	mt753x_reg_write(gsw, PMCR(5), FORCE_MODE_LNK);
	mt753x_reg_write(gsw, PMCR(6), FORCE_MODE_LNK);

	/* Switch soft reset */
	mt753x_reg_write(gsw, SYS_CTRL, SW_SYS_RST | SW_REG_RST);
	usleep_range(10, 20);

	/* Enable MDC input Schmitt Trigger */
	val = mt753x_reg_read(gsw, SMT0_IOLB);
	mt753x_reg_write(gsw, SMT0_IOLB, val | SMT_IOLB_5_SMI_MDC_EN);

	/* Set 7531 gpio pinmux */
	mt7531_set_gpio_pinmux(gsw);

	/* Global mac control settings */
	mt753x_reg_write(gsw, GMACCR,
			 (15 << MTCC_LMT_S) | (11 << MAX_RX_JUMBO_S) |
			 RX_PKT_LEN_MAX_JUMBO);

	mt7531_core_pll_setup(gsw);
	mt7531_mac_port_setup(gsw, 5, &gsw->port5_cfg);
	mt7531_mac_port_setup(gsw, 6, &gsw->port6_cfg);

	return 0;
}

static int mt7531_sw_post_init(struct gsw_mt753x *gsw)
{
	int i;
	u32 val;

	mt7531_phy_pll_setup(gsw);

	/* Internal PHYs are disabled by default. SW should enable them.
	 * Note that this may already be enabled in bootloader stage.
	 */
	val = gsw->mmd_read(gsw, 0, PHY_DEV1F, PHY_DEV1F_REG_403);
	val |= PHY_EN_BYPASS_MODE;
	val &= ~POWER_ON_OFF;
	gsw->mmd_write(gsw, 0, PHY_DEV1F, PHY_DEV1F_REG_403, val);

	mt7531_phy_setting(gsw);

	for (i = 0; i < MT753X_NUM_PHYS; i++) {
		val = gsw->mii_read(gsw, i, MII_BMCR);
		val &= ~BMCR_ISOLATE;
		gsw->mii_write(gsw, i, MII_BMCR, val);
	}

	for (i = 0; i < MT753X_NUM_PHYS; i++)
		mt7531_adjust_line_driving(gsw, i);

	for (i = 0; i < MT753X_NUM_PHYS; i++)
		mt7531_eee_setting(gsw, i);

	val = mt753x_reg_read(gsw, CHIP_REV);
	val &= CHIP_REV_M;
	if (val == CHIP_REV_E1) {
		mt7531_internal_phy_calibration(gsw);
	} else {
		val = mt753x_reg_read(gsw, GBE_EFUSE);
		if (val & GBE_SEL_EFUSE_EN) {
			val = gsw->mmd_read(gsw, 0, PHY_DEV1F,
					    PHY_DEV1F_REG_403);
			val &= ~GBE_EFUSE_SETTING;
			gsw->mmd_write(gsw, 0, PHY_DEV1F, PHY_DEV1F_REG_403,
				       val);
		} else {
			mt7531_internal_phy_calibration(gsw);
		}
	}

	return 0;
}

struct mt753x_sw_id mt7531_id = {
	.model = MT7531,
	.detect = mt7531_sw_detect,
	.init = mt7531_sw_init,
	.post_init = mt7531_sw_post_init
};

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Zhanguo Ju <zhanguo.ju@mediatek.com>");
MODULE_DESCRIPTION("Driver for MediaTek MT753x Gigabit Switch");
