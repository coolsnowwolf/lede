// SPDX-License-Identifier: GPL-2.0-or-later

#include <linux/bitfield.h>
#include <linux/delay.h>
#include <linux/mdio.h>
#include <linux/mfd/syscon.h>
#include <linux/of.h>
#include <linux/of_mdio.h>
#include <linux/of_platform.h>
#include <linux/pcs/pcs-provider.h>
#include <linux/phy.h>
#include <linux/phy/phy-common-props.h>
#include <linux/phylink.h>
#include <linux/platform_device.h>
#include <linux/property.h>
#include <linux/regmap.h>
#include <linux/rtnetlink.h>

#define RTPCS_SDS_CNT				14
#define RTPCS_MAX_LINKS_PER_SDS			8

#define RTPCS_SPEED_10				0
#define RTPCS_SPEED_100				1
#define RTPCS_SPEED_1000			2
#define RTPCS_SPEED_10000_LEGACY		3
#define RTPCS_SPEED_10000			4
#define RTPCS_SPEED_2500			5
#define RTPCS_SPEED_5000			6

/* USXGMII-AN opcodes. RTK variant unused but kept for documentation */
#define RTPCS_USXGMII_AN_OPC_STD		0x03
#define RTPCS_USXGMII_AN_OPC_RTK		0xaa

#define RTPCS_838X_CPU_PORT			28
#define RTPCS_838X_SERDES_CNT			6
#define RTPCS_838X_MAC_LINK_DUP_STS		0xa19c
#define RTPCS_838X_MAC_LINK_SPD_STS		0xa190
#define RTPCS_838X_MAC_LINK_STS			0xa188
#define RTPCS_838X_MAC_RX_PAUSE_STS		0xa1a4
#define RTPCS_838X_MAC_TX_PAUSE_STS		0xa1a0

#define RTPCS_839X_CPU_PORT			52
#define RTPCS_839X_SERDES_CNT			14
#define RTPCS_839X_MAC_LINK_DUP_STS		0x03b0
#define RTPCS_839X_MAC_LINK_SPD_STS		0x03a0
#define RTPCS_839X_MAC_LINK_STS			0x0390
#define RTPCS_839X_MAC_RX_PAUSE_STS		0x03c0
#define RTPCS_839X_MAC_TX_PAUSE_STS		0x03b8

#define RTPCS_83XX_MAC_LINK_SPD_BITS		2

#define RTPCS_930X_CPU_PORT			28
#define RTPCS_930X_SERDES_CNT			12
#define RTPCS_930X_MAC_LINK_DUP_STS		0xcb28
#define RTPCS_930X_MAC_LINK_SPD_STS		0xcb18
#define RTPCS_930X_MAC_LINK_STS			0xcb10
#define RTPCS_930X_MAC_RX_PAUSE_STS		0xcb30
#define RTPCS_930X_MAC_TX_PAUSE_STS		0xcb2c

#define RTPCS_931X_CPU_PORT			56
#define RTPCS_931X_SERDES_CNT			14
#define RTPCS_931X_MAC_LINK_DUP_STS		0x0ef0
#define RTPCS_931X_MAC_LINK_SPD_STS		0x0ed0
#define RTPCS_931X_MAC_LINK_STS			0x0ec0
#define RTPCS_931X_MAC_RX_PAUSE_STS		0x0f00
#define RTPCS_931X_MAC_TX_PAUSE_STS		0x0ef8

#define RTPCS_838X_SDS_CFG_REG			0x34
#define RTPCS_838X_RST_GLB_CTRL_0		0x3c
#define RTPCS_838X_SDS_MODE_SEL			0x0028
#define RTPCS_838X_INT_MODE_CTRL		0x005c
#define RTPCS_838X_PLL_CML_CTRL			0x0ff8

#define RTPCS_839X_MAC_SERDES_IF_CTRL		0x0008

#define RTPCS_93XX_MAC_LINK_SPD_BITS		4

#define RTPCS_93XX_MODEL_NAME_INFO		(0x0004)
#define RTPCS_93XX_CHIP_INFO			(0x0008)

#define RTPCS_930X_SDS_MODE_SEL_0		0x0194
#define RTPCS_930X_SDS_MODE_SEL_1		0x02a0
#define RTPCS_930X_SDS_MODE_SEL_2		0x02a4
#define RTPCS_930X_SDS_MODE_SEL_3		0x0198
#define RTPCS_930X_SDS_SUBMODE_CTRL_0		0x01cc
#define RTPCS_930X_SDS_SUBMODE_CTRL_1		0x02d8

#define RTPCS_93XX_SDS_MODE_SGMII		0x02
#define RTPCS_93XX_SDS_MODE_1000BASEX		0x04
#define RTPCS_93XX_SDS_MODE_QSGMII		0x06
#define RTPCS_93XX_SDS_MODE_USXGMII		0x0d
#define RTPCS_93XX_SDS_MODE_XSGMII		0x10
#define RTPCS_93XX_SDS_MODE_2500BASEX		0x16
#define RTPCS_93XX_SDS_MODE_10GBASER		0x1a
#define RTPCS_93XX_SDS_MODE_OFF			0x1f

#define RTPCS_93XX_SDS_USXGMII_SUBMODE_10GSX	0x00
#define RTPCS_93XX_SDS_USXGMII_SUBMODE_10GDX	0x01
#define RTPCS_93XX_SDS_USXGMII_SUBMODE_10GQX	0x02
#define RTPCS_93XX_SDS_USXGMII_SUBMODE_5GSX	0x03
#define RTPCS_93XX_SDS_USXGMII_SUBMODE_5GDX	0x04
#define RTPCS_93XX_SDS_USXGMII_SUBMODE_2_5GSX	0x05

/* Registers of the internal SerDes of the 9310 */
#define RTPCS_931X_MAC_GROUP0_1_CTRL		(0x13a4)
#define RTPCS_931X_MAC_GROUP2_3_CTRL		(0x13a8)
#define RTPCS_931X_MAC_GROUP4_CTRL		(0x13ac)
#define RTPCS_931X_MAC_GROUP5_CTRL		(0x13b0)
#define RTPCS_931X_MAC_GROUP6_7_CTRL		(0x13b4)
#define RTPCS_931X_MAC_GROUP8_11_CTRL		(0x13b8)
#define RTPCS_931X_SERDES_MODE_CTRL		(0x13cc)
#define RTPCS_931X_SDS_USXGMII_SUBMODE		(0x13e8)
#define RTPCS_931X_PS_SERDES_OFF_MODE_CTRL_ADDR	(0x13F4)
#define RTPCS_931X_MAC_SERDES_MODE_CTRL(sds)	(0x136C + (((sds) << 2)))
#define RTPCS_931X_ISR_SERDES_RXIDLE		(0x12f8)

#define RTPCS_931X_SDS_PRE_AMP_MASK		GENMASK(4, 0)
#define RTPCS_931X_SDS_MAIN_AMP_MASK		GENMASK(9, 5)
#define RTPCS_931X_SDS_POST_AMP_MASK		GENMASK(14, 10)

/*
 * A SerDes has a register space separated into several pages. Each page
 * serves a different purpose and is the home of common settings. E.g.,
 * there are dedicated pages for each operating speed of a SerDes.
 */
enum rtpcs_page {
	PAGE_SDS		= 0x00,
	PAGE_SDS_EXT		= 0x01,
	PAGE_FIB		= 0x02,
	PAGE_FIB_EXT		= 0x03,
	PAGE_TGR_STD_0		= 0x04,
	PAGE_TGR_STD_1		= 0x05,
	PAGE_TGR_PRO_0		= 0x06,
	PAGE_TGR_PRO_1		= 0x07,
	PAGE_TGX_STD_0		= 0x08,
	PAGE_TGX_STD_1		= 0x09,
	PAGE_TGX_PRO_0		= 0x0a,
	PAGE_TGX_PRO_1		= 0x0b,
	PAGE_WDIG		= 0x1f,
	PAGE_ANA_MISC		= 0x20,
	PAGE_ANA_COM		= 0x21,
	PAGE_ANA_SPD		= 0x22,
	PAGE_ANA_SPD_EXT	= 0x23,
	PAGE_ANA_1G2		= 0x24,
	PAGE_ANA_1G2_EXT	= 0x25,
	PAGE_ANA_2G5		= 0x26,
	PAGE_ANA_2G5_EXT	= 0x27,
	PAGE_ANA_3G1		= 0x28,
	PAGE_ANA_3G1_EXT	= 0x29,
	PAGE_ANA_5G0		= 0x2a,
	PAGE_ANA_5G0_EXT	= 0x2b,
	PAGE_ANA_6G2		= 0x2c,
	PAGE_ANA_6G2_EXT	= 0x2d,
	PAGE_ANA_10G		= 0x2e,
	PAGE_ANA_10G_EXT	= 0x2f,
	PAGE_GPON_SP		= 0x30,
	PAGE_GPON_SP_EXT	= 0x31,
	PAGE_EPON_SP		= 0x32,
	PAGE_EPON_SP_EXT	= 0x33,
	PAGE_ANA_6G0		= 0x34,
	PAGE_ANA_6G0_EXT	= 0x35,
};

/*
 * RTL931X only: the digital SDS 1/2 register pages mirror an analog page at a
 * fixed +0x40 / +0x80 region offset. See rtpcs_931x_sds_op_xsg_write().
 */
#define DIGI_1(page)	((page) + 0x40)
#define DIGI_2(page)	((page) + 0x80)

/* TGR_PRO_0, reg 0x0d */
#define RTL93XX_LINKDW_SEL		BIT(6)
#define RTL93XX_LINKDW_SEL_DAC		0x0
#define RTL93XX_LINKDW_SEL_NON_DAC	BIT(6)

/* ANA_MISC, reg 0x00 */
#define RTL93XX_FRC_CMU_EN_MASK		GENMASK(11, 10)
#define RTL93XX_FRC_CMU_EN_UNFORCED	FIELD_PREP(RTL93XX_FRC_CMU_EN_MASK, 0x0)
#define RTL93XX_FRC_CMU_EN_FORCE_OFF	FIELD_PREP(RTL93XX_FRC_CMU_EN_MASK, 0x1)
#define RTL93XX_FRC_CMU_EN_FORCE_ON	FIELD_PREP(RTL93XX_FRC_CMU_EN_MASK, 0x3)
#define RTL93XX_FRC_PDOWN_MASK		GENMASK(7, 6)
#define RTL93XX_FRC_PDOWN_DOWN		FIELD_PREP(RTL93XX_FRC_PDOWN_MASK, 0x3)
#define RTL93XX_FRC_PDOWN_UNFORCED	FIELD_PREP(RTL93XX_FRC_PDOWN_MASK, 0x0)
#define RTL93XX_FRC_RX_EN_MASK		GENMASK(5, 4)
#define RTL93XX_FRC_RX_EN_ON		FIELD_PREP(RTL93XX_FRC_RX_EN_MASK, 0x3)
#define RTL93XX_FRC_RX_EN_OFF		FIELD_PREP(RTL93XX_FRC_RX_EN_MASK, 0x1)
#define RTL93XX_FRC_V2ANALOG_MASK	GENMASK(1, 0)
#define RTL93XX_FRC_V2ANALOG_UNFORCED	FIELD_PREP(RTL93XX_FRC_V2ANALOG_MASK, 0x0)
#define RTL93XX_FRC_V2ANALOG_FORCE_OFF	FIELD_PREP(RTL93XX_FRC_V2ANALOG_MASK, 0x1)

/* DIGI_1(WDIG), reg 0x01 */
/*
 * Gates a digital clock inside the SerDes. The exact block is unknown — it
 * appears to be used by all modes except USXGMII and 10GBASE-R. The bit name
 * is inherited from an earlier SerDes generation and has not been verified
 * on RTL931x. GLI could mean "GMII Line Interface" or "Gigabit Line Interface".
 */
#define RTL931X_STOP_GLI_CLK		BIT(0)

enum rtpcs_sds_type {
	RTPCS_SDS_TYPE_UNKNOWN,
	RTPCS_SDS_TYPE_5G,
	RTPCS_SDS_TYPE_10G,
};

enum rtpcs_sds_mode {
	RTPCS_SDS_MODE_OFF = 0,

	/* fiber modes */
	RTPCS_SDS_MODE_100BASEX,
	RTPCS_SDS_MODE_1000BASEX,
	RTPCS_SDS_MODE_2500BASEX,
	RTPCS_SDS_MODE_10GBASER,

	/* mii modes */
	RTPCS_SDS_MODE_SGMII,
	RTPCS_SDS_MODE_QSGMII,
	RTPCS_SDS_MODE_XSGMII,
	RTPCS_SDS_MODE_USXGMII,

	RTPCS_SDS_MODE_MAX,
};

enum rtpcs_sds_usxgmii_submode {
	RTPCS_SDS_USXGMII_SM_NONE = 0,
	RTPCS_SDS_USXGMII_SM_10GSXGMII,
	RTPCS_SDS_USXGMII_SM_10GDXGMII,
	RTPCS_SDS_USXGMII_SM_10GQXGMII,
	RTPCS_SDS_USXGMII_SM_5GSXGMII,
	RTPCS_SDS_USXGMII_SM_5GDXGMII,
	RTPCS_SDS_USXGMII_SM_2_5GSXGMII,

	RTPCS_SDS_USXGMII_SM_MAX,
};

enum rtpcs_sds_attachment {
	RTPCS_SDS_ATTACH_NONE,
	RTPCS_SDS_ATTACH_FIBER,
	RTPCS_SDS_ATTACH_DAC_SHORT,	/*  < 3m */
	RTPCS_SDS_ATTACH_DAC_LONG,	/* >= 3m */
	RTPCS_SDS_ATTACH_PHY,
};

enum rtpcs_sds_pll_type {
	RTPCS_SDS_PLL_TYPE_RING = 0,
	RTPCS_SDS_PLL_TYPE_LC = 1,
	RTPCS_SDS_PLL_TYPE_END,
};

enum rtpcs_sds_pll_speed {
	RTPCS_SDS_PLL_SPD_1000 = 0,
	RTPCS_SDS_PLL_SPD_2500 = 1,
	RTPCS_SDS_PLL_SPD_10000 = 2,
	RTPCS_SDS_PLL_SPD_END,
};

enum rtpcs_chip_version {
	RTPCS_CHIP_V1 = 0,
	RTPCS_CHIP_V2,
};

struct rtpcs_ctrl;
struct rtpcs_serdes;

struct rtpcs_sds_ops {
	int (*read)(struct rtpcs_serdes *sds, enum rtpcs_page page, int regnum,
		    int bithigh, int bitlow);
	int (*write)(struct rtpcs_serdes *sds, enum rtpcs_page page, int regnum,
		     int bithigh, int bitlow, u16 value);
	/* write an arbitrary, possibly non-contiguous bitmask in a single call */
	int (*write_mask)(struct rtpcs_serdes *sds, enum rtpcs_page page, int regnum,
			  u16 mask, u16 value);

	/* optional */
	int (*xsg_write)(struct rtpcs_serdes *sds, enum rtpcs_page page, int regnum,
			 int bithigh, int bitlow, u16 value);

	int (*set_autoneg)(struct rtpcs_serdes *sds, unsigned int neg_mode,
			   const unsigned long *advertising);
	void (*restart_autoneg)(struct rtpcs_serdes *sds);

	/* CMU management */
	int (*get_pll_select)(struct rtpcs_serdes *sds, enum rtpcs_sds_pll_type *pll);
	int (*set_pll_select)(struct rtpcs_serdes *sds, enum rtpcs_sds_mode hw_mode,
			      enum rtpcs_sds_pll_type pll);
	int (*reset_cmu)(struct rtpcs_serdes *sds, enum rtpcs_sds_pll_type pll);
	/* online reconfiguration of a running SerDes to another PLL */
	int (*reconfigure_to_pll)(struct rtpcs_serdes *sds, enum rtpcs_sds_pll_type pll);

	int (*config_polarity)(struct rtpcs_serdes *sds, unsigned int tx_pol,
			       unsigned int rx_pol);

	/* required: power down before reconfiguration */
	int (*deactivate)(struct rtpcs_serdes *sds);
	/* required: power back up */
	int (*activate)(struct rtpcs_serdes *sds);
	/* required: configure SerDes for hardware mode */
	int (*config_hw_mode)(struct rtpcs_serdes *sds, enum rtpcs_sds_mode hw_mode);
	/* required: set hardware mode */
	int (*set_hw_mode)(struct rtpcs_serdes *sds, enum rtpcs_sds_mode hw_mode,
			   enum rtpcs_sds_usxgmii_submode submode);
	/* optional: configure attachment-specific parameters */
	int (*config_attachment)(struct rtpcs_serdes *sds, enum rtpcs_sds_attachment attachment,
				 enum rtpcs_sds_mode hw_mode);
	/* optional: finalization that must follow power-up, e.g. RX calibration */
	int (*post_config)(struct rtpcs_serdes *sds, enum rtpcs_sds_mode hw_mode);
};

struct rtpcs_serdes {
	struct rtpcs_ctrl *ctrl;
	struct fwnode_handle *fwnode;
	const struct rtpcs_sds_ops *ops;
	enum rtpcs_sds_type type;
	DECLARE_BITMAP(supported_modes, RTPCS_SDS_MODE_MAX);
	struct {
		struct regmap_field *mac_mode;
		struct regmap_field *mac_mode_force;	/* nullable, 931x only */
		struct regmap_field *usxgmii_submode;	/* nullable, 93xx only */
	} swcore_regs;
	struct rtpcs_link *link[RTPCS_MAX_LINKS_PER_SDS];
	s16 link_port[RTPCS_MAX_LINKS_PER_SDS];

	enum rtpcs_sds_mode hw_mode;
	enum rtpcs_sds_usxgmii_submode usxgmii_submode;	/* only valid if hw_mode == RTPCS_SDS_MODE_USXGMII */
	enum rtpcs_sds_attachment attachment;
	u8 id;
	u8 num_of_links;
	bool first_start;
};

struct rtpcs_ctrl {
	struct device *dev;
	struct regmap *map;
	struct mii_bus *bus;
	const struct rtpcs_config *cfg;
	struct rtpcs_serdes serdes[RTPCS_SDS_CNT];
	struct mutex lock;

	/* meaning and source may be family-specific */
	enum rtpcs_chip_version chip_version;
};

struct rtpcs_link {
	struct rtpcs_ctrl *ctrl;
	struct phylink_pcs pcs;
	struct rtpcs_serdes *sds;
	int port;
};

struct rtpcs_config {
	int cpu_port;
	int mac_link_dup_sts;
	int mac_link_spd_bits;
	int mac_link_spd_sts;
	int mac_link_sts;
	int mac_rx_pause_sts;
	int mac_tx_pause_sts;
	u8 serdes_count;

	const struct phylink_pcs_ops *pcs_ops;
	const struct rtpcs_sds_ops *sds_ops;
	enum rtpcs_page phy_page;	/* page mirroring standard PHY registers (BMCR, ...) */
	const s16 *sds_hw_mode_vals;   /* enum rtpcs_sds_mode, -1 = unsupported */

	int (*init)(struct rtpcs_ctrl *ctrl);
	int (*sds_probe)(struct rtpcs_serdes *sds);
};

struct rtpcs_sds_config {
	enum rtpcs_page page;
	u8 reg;
	u16 data;
};

struct rtpcs_sds_tx_config {
	u8 pre_amp;
	u8 main_amp;
	u8 post_amp;
	u8 impedance;
};

/* Calculation helpers */

static int rtpcs_sds_to_mmd(enum rtpcs_page sds_page, int sds_regnum)
{
	return (sds_page << 8) + sds_regnum;
}

static int rtpcs_sign_mag_decode(unsigned int val, unsigned int sign_bit)
{
	int mag = val & GENMASK(sign_bit - 1, 0);

	return (val & BIT(sign_bit)) ? -mag : mag;
}

static unsigned int rtpcs_sign_mag_encode(int val, unsigned int sign_bit)
{
	return (val < 0 ? BIT(sign_bit) : 0) | (abs(val) & GENMASK(sign_bit - 1, 0));
}

static u32 rtpcs_gray_to_binary(u32 gray_code)
{
	u32 binary = gray_code;

	gray_code &= 0x1f; /* only lower 5 bits */
	while (gray_code >>= 1)
		binary ^= gray_code;

	return binary;
}

/*
 * Basic helpers
 *
 * These work on the plain SerDes ID. They shouldn't be used except for
 * implementing the SerDes read/write ops.
 */

static int __rtpcs_sds_read_raw(struct rtpcs_ctrl *ctrl, int sds_id, enum rtpcs_page page,
				int regnum, int bithigh, int bitlow)
{
	int mmd_regnum = rtpcs_sds_to_mmd(page, regnum);
	u16 mask;
	int val;

	if (WARN_ON(bithigh < bitlow))
		return -EINVAL;

	mask = GENMASK(bithigh, bitlow);
	val = mdiobus_c45_read(ctrl->bus, sds_id, MDIO_MMD_VEND1, mmd_regnum);
	if (val < 0)
		return val;

	return (val & mask) >> bitlow;
}

static int __rtpcs_sds_write_mask(struct rtpcs_ctrl *ctrl, int sds_id, enum rtpcs_page page,
				  int regnum, u16 mask, u16 set)
{
	int mmd_regnum = rtpcs_sds_to_mmd(page, regnum);

	if (mask == 0xffff)
		return mdiobus_c45_write(ctrl->bus, sds_id, MDIO_MMD_VEND1, mmd_regnum, set);

	return mdiobus_c45_modify(ctrl->bus, sds_id, MDIO_MMD_VEND1, mmd_regnum, mask, set);
}

static int __rtpcs_sds_write_raw(struct rtpcs_ctrl *ctrl, int sds_id, enum rtpcs_page page,
				 int regnum, int bithigh, int bitlow, u16 value)
{
	u16 mask, set;

	if (WARN_ON(bithigh < bitlow))
		return -EINVAL;

	if (bithigh == 15 && bitlow == 0)
		return __rtpcs_sds_write_mask(ctrl, sds_id, page, regnum, 0xffff, value);

	mask = GENMASK(bithigh, bitlow);
	set = (value << bitlow) & mask;
	return __rtpcs_sds_write_mask(ctrl, sds_id, page, regnum, mask, set);
}

/* Generic implementations, if no special behavior is needed */

static int rtpcs_generic_sds_op_read(struct rtpcs_serdes *sds, enum rtpcs_page page, int regnum,
				     int bithigh, int bitlow)
{
	return __rtpcs_sds_read_raw(sds->ctrl, sds->id, page, regnum, bithigh, bitlow);
}

static int rtpcs_generic_sds_op_write(struct rtpcs_serdes *sds, enum rtpcs_page page, int regnum,
				      int bithigh, int bitlow, u16 value)
{
	return __rtpcs_sds_write_raw(sds->ctrl, sds->id, page, regnum, bithigh, bitlow, value);
}

static int rtpcs_generic_sds_op_write_mask(struct rtpcs_serdes *sds, enum rtpcs_page page,
					   int regnum, u16 mask, u16 value)
{
	return __rtpcs_sds_write_mask(sds->ctrl, sds->id, page, regnum, mask, value);
}

/* Convenience helpers */

static int rtpcs_sds_read_bits(struct rtpcs_serdes *sds, enum rtpcs_page page, int regnum,
			       int bithigh, int bitlow)
{
	return sds->ops->read(sds, page, regnum, bithigh, bitlow);
}

static int rtpcs_sds_write_bits(struct rtpcs_serdes *sds, enum rtpcs_page page, int regnum,
				int bithigh, int bitlow, u16 value)
{
	return sds->ops->write(sds, page, regnum, bithigh, bitlow, value);
}

static int rtpcs_sds_write_mask(struct rtpcs_serdes *sds, enum rtpcs_page page, int regnum,
				u16 mask, u16 value)
{
	return sds->ops->write_mask(sds, page, regnum, mask, value);
}

static int rtpcs_sds_read(struct rtpcs_serdes *sds, enum rtpcs_page page, int regnum)
{
	return sds->ops->read(sds, page, regnum, 15, 0);
}

static int rtpcs_sds_write(struct rtpcs_serdes *sds, enum rtpcs_page page, int regnum, u16 value)
{
	return sds->ops->write(sds, page, regnum, 15, 0, value);
}

static int rtpcs_sds_xsg_write_bits(struct rtpcs_serdes *sds, enum rtpcs_page page, int regnum,
				    int bithigh, int bitlow, u16 value)
{
	if (!sds->ops->xsg_write)
		return -ENOTSUPP;

	return sds->ops->xsg_write(sds, page, regnum, bithigh, bitlow, value);
}

static int rtpcs_sds_xsg_write(struct rtpcs_serdes *sds, enum rtpcs_page page, int regnum,
			       u16 value)
{
	if (!sds->ops->xsg_write)
		return -ENOTSUPP;

	return sds->ops->xsg_write(sds, page, regnum, 15, 0, value);
}

/* Other helpers */

static struct rtpcs_serdes *rtpcs_sds_get_even(struct rtpcs_serdes *sds)
{
	u32 even_sds = sds->id & ~1;
	return &sds->ctrl->serdes[even_sds];
}

static struct rtpcs_serdes *rtpcs_sds_get_odd(struct rtpcs_serdes *sds)
{
	u32 odd_sds = sds->id | 1;
	return &sds->ctrl->serdes[odd_sds];
}

static struct rtpcs_serdes *rtpcs_sds_get_neighbor(struct rtpcs_serdes *sds)
{
	u32 nb_sds = sds->id ^ 1;
	return &sds->ctrl->serdes[nb_sds];
}

static int rtpcs_regmap_read_bits(struct rtpcs_ctrl *ctrl, int base, int bithigh, int bitlow)
{
	int offset = base + (bitlow / 32) * 4;
	int bits = bithigh + 1 - bitlow;
	int shift = bitlow % 32;
	int value;

	regmap_read(ctrl->map, offset, &value);
	value = (value >> shift) & (BIT(bits) - 1);

	return value;
}

static struct rtpcs_link *rtpcs_phylink_pcs_to_link(struct phylink_pcs *pcs)
{
	return container_of(pcs, struct rtpcs_link, pcs);
}

static int rtpcs_sds_select_hw_mode(struct rtpcs_serdes *sds, phy_interface_t if_mode,
				    enum rtpcs_sds_mode *hw_mode,
				    enum rtpcs_sds_usxgmii_submode *submode)
{
	u8 n_links = sds->num_of_links;

	*submode = RTPCS_SDS_USXGMII_SM_NONE;

	/* turn off SerDes when there are no links */
	if (!n_links) {
		*hw_mode = RTPCS_SDS_MODE_OFF;
		return 0;
	}

	switch (if_mode) {
	case PHY_INTERFACE_MODE_NA:
		*hw_mode = RTPCS_SDS_MODE_OFF;
		break;
	case PHY_INTERFACE_MODE_100BASEX:
		*hw_mode = RTPCS_SDS_MODE_100BASEX;
		break;
	case PHY_INTERFACE_MODE_1000BASEX:
		*hw_mode = RTPCS_SDS_MODE_1000BASEX;
		break;
	case PHY_INTERFACE_MODE_2500BASEX:
		*hw_mode = RTPCS_SDS_MODE_2500BASEX;
		break;
	case PHY_INTERFACE_MODE_10GBASER:
		*hw_mode = RTPCS_SDS_MODE_10GBASER;
		break;
	case PHY_INTERFACE_MODE_SGMII:
		*hw_mode = RTPCS_SDS_MODE_SGMII;
		break;
	case PHY_INTERFACE_MODE_QSGMII:
		*hw_mode = RTPCS_SDS_MODE_QSGMII;
		break;
	case PHY_INTERFACE_MODE_USXGMII:
		if (n_links == 1) {
			*hw_mode = RTPCS_SDS_MODE_USXGMII;
			*submode = RTPCS_SDS_USXGMII_SM_10GSXGMII;
		} else if (n_links == 2) {
			*hw_mode = RTPCS_SDS_MODE_USXGMII;
			*submode = RTPCS_SDS_USXGMII_SM_10GDXGMII;
		} else if (n_links <= 4) {
			*hw_mode = RTPCS_SDS_MODE_USXGMII;
			*submode = RTPCS_SDS_USXGMII_SM_10GQXGMII;
		} else if (n_links <= 8) {
			*hw_mode = RTPCS_SDS_MODE_XSGMII;
		}

		break;
	case PHY_INTERFACE_MODE_10G_QXGMII:
		*hw_mode = RTPCS_SDS_MODE_USXGMII;
		*submode = RTPCS_SDS_USXGMII_SM_10GQXGMII;
		break;
	default:
		return -EOPNOTSUPP;
	}

	if (!test_bit(*hw_mode, sds->supported_modes))
		return -EOPNOTSUPP;

	return 0;
}

static int rtpcs_sds_select_attachment(enum rtpcs_sds_mode hw_mode,
				       enum rtpcs_sds_attachment *attachment)
{
	switch (hw_mode) {
	case RTPCS_SDS_MODE_OFF:
		*attachment = RTPCS_SDS_ATTACH_NONE;
		break;
	case RTPCS_SDS_MODE_1000BASEX:
	case RTPCS_SDS_MODE_2500BASEX:
	case RTPCS_SDS_MODE_10GBASER:
		*attachment = RTPCS_SDS_ATTACH_FIBER;
		break;
	default:
		/*
		 * SGMII, QSGMII, XSGMII and USXGMII always run into a PHY, be
		 * it on-board or embedded in an SFP copper module; either way
		 * the SerDes-to-PHY trace is short and the PHY equalizes for
		 * itself on the far end.
		 */
		*attachment = RTPCS_SDS_ATTACH_PHY;
		break;
	}

	return 0;
}

/* Generic auto-negotiation config */

static int rtpcs_generic_sds_set_autoneg(struct rtpcs_serdes *sds, unsigned int neg_mode,
					 const unsigned long *advertising)
{
	enum rtpcs_page phy_page = sds->ctrl->cfg->phy_page;
	u16 bmcr, adv, adv_old;
	bool changed = false;
	int ret;

	if ((sds->hw_mode == RTPCS_SDS_MODE_1000BASEX) ||
	    (sds->hw_mode == RTPCS_SDS_MODE_2500BASEX)) {
		adv = ADVERTISE_1000XFULL;
		if (linkmode_test_bit(ETHTOOL_LINK_MODE_Pause_BIT,
				      advertising))
			adv |= ADVERTISE_1000XPAUSE;
		if (linkmode_test_bit(ETHTOOL_LINK_MODE_Asym_Pause_BIT,
				      advertising))
			adv |= ADVERTISE_1000XPSE_ASYM;

		ret = rtpcs_sds_read(sds, phy_page, MII_ADVERTISE);
		if (ret < 0)
			return ret;

		adv_old = ret;
		if (adv != adv_old) {
			changed = true;
			ret = rtpcs_sds_write(sds, phy_page, MII_ADVERTISE, adv);
			if (ret < 0)
				return ret;
		}
	}

	bmcr = neg_mode == PHYLINK_PCS_NEG_INBAND_ENABLED ? 1 : 0;

	ret = rtpcs_sds_write_bits(sds, phy_page, MII_BMCR, 12, 12, bmcr);
	if (ret < 0)
		return ret;

	return changed;
}

static void rtpcs_generic_sds_restart_autoneg(struct rtpcs_serdes *sds)
{
	rtpcs_sds_write_bits(sds, sds->ctrl->cfg->phy_page, MII_BMCR, 9, 9, 0x1);
}

static int rtpcs_sds_select_pll_speed(enum rtpcs_sds_mode hw_mode, enum rtpcs_sds_pll_speed *speed)
{
	switch (hw_mode) {
	case RTPCS_SDS_MODE_1000BASEX:
	case RTPCS_SDS_MODE_SGMII:
	case RTPCS_SDS_MODE_QSGMII:
		*speed = RTPCS_SDS_PLL_SPD_1000;
		break;
	case RTPCS_SDS_MODE_2500BASEX:
		*speed = RTPCS_SDS_PLL_SPD_2500;
		break;
	case RTPCS_SDS_MODE_10GBASER:
	case RTPCS_SDS_MODE_XSGMII:
	case RTPCS_SDS_MODE_USXGMII:
		*speed = RTPCS_SDS_PLL_SPD_10000;
		break;
	default:
		return -ENOTSUPP;
	}

	return 0;
}

static int rtpcs_sds_apply_config(struct rtpcs_serdes *sds,
				  const struct rtpcs_sds_config *config, size_t count)
{
	int ret;

	for (size_t i = 0; i < count; i++) {
		ret = rtpcs_sds_write(sds, config[i].page, config[i].reg, config[i].data);
		if (ret)
			return ret;
	}
	return 0;
}

static int rtpcs_sds_apply_config_xsg(struct rtpcs_serdes *sds,
				      const struct rtpcs_sds_config *config, size_t count)
{
	int ret;

	for (size_t i = 0; i < count; i++) {
		ret = rtpcs_sds_xsg_write(sds, config[i].page, config[i].reg, config[i].data);
		if (ret)
			return ret;
	}
	return 0;
}

/*
 * Allocate a regmap_field on the SoC-side register map for this SerDes and
 * store the resulting pointer in *dst. Convenience helper for per-SerDes
 * register fields computed from the SerDes ID. Taking reg/lsb/msb as
 * integer arguments (rather than a struct reg_field) keeps callers free of
 * either local reg_field declarations or compound-literal casts, since
 * REG_FIELD() is a brace-initializer and not a usable expression.
 */
static int rtpcs_sds_alloc_field(struct rtpcs_serdes *sds, struct regmap_field **dst,
				 u32 reg, u8 lsb, u8 msb)
{
	struct reg_field rf = REG_FIELD(reg, lsb, msb);

	*dst = devm_regmap_field_alloc(sds->ctrl->dev, sds->ctrl->map, rf);
	return PTR_ERR_OR_ZERO(*dst);
}

/*
 * Write the SerDes MAC mode register. This is the common minimum shared by
 * all variants. Variant-specific extras (force bit, companion registers,
 * USXGMII submode, post-write delay) live in per-variant wrappers.
 */
static int rtpcs_sds_set_mac_mode(struct rtpcs_serdes *sds, enum rtpcs_sds_mode hw_mode)
{
	const struct rtpcs_config *cfg = sds->ctrl->cfg;
	int val;

	if (hw_mode >= RTPCS_SDS_MODE_MAX)
		return -EINVAL;

	val = cfg->sds_hw_mode_vals[hw_mode];
	if (val < 0)
		return -EOPNOTSUPP;

	return regmap_field_write(sds->swcore_regs.mac_mode, val);
}

/* Variant-specific functions */

/* RTL838X */

/* RTL838X SDS_MODE_SEL field values */
static const s16 rtpcs_838x_sds_hw_mode_vals[RTPCS_SDS_MODE_MAX] = {
	[0 ... RTPCS_SDS_MODE_MAX - 1]	= -1,
	[RTPCS_SDS_MODE_1000BASEX]	= 0x4,
	[RTPCS_SDS_MODE_SGMII]		= 0x2,
	[RTPCS_SDS_MODE_QSGMII]		= 0x6,
};

static void rtpcs_838x_sds_patch_qsgmii(struct rtpcs_serdes *sds)
{
	bool is_even = (rtpcs_sds_get_even(sds) == sds);

	rtpcs_sds_write(sds, PAGE_SDS_EXT, 0x3, 0xf46d);
	rtpcs_sds_write(sds, PAGE_SDS_EXT, 0x2, 0x85fa);
	rtpcs_sds_write(sds, PAGE_SDS_EXT, 0x6, 0x20d8);

	rtpcs_sds_write(sds, PAGE_SDS_EXT, 10, is_even ? 0x58c7 : 0x80c7);
	rtpcs_sds_write(sds, PAGE_SDS_EXT, 17, is_even ? 0xb7c9 : 0x4208);
	rtpcs_sds_write(sds, PAGE_SDS_EXT, 18, is_even ? 0xab8e : 0xc208);
	rtpcs_sds_write(sds, PAGE_SDS_EXT, 11, 0x482);
	if (is_even)
		rtpcs_sds_write(sds, PAGE_SDS_EXT, 19, 0x24ab);

	rtpcs_sds_write(sds, PAGE_FIB, 25, 0x303);
	rtpcs_sds_write(sds, PAGE_SDS_EXT, 14, 0xfcc2);
}

static void rtpcs_838x_sds_patch_fiber(struct rtpcs_serdes *sds)
{
	bool is_even = rtpcs_sds_get_even(sds) == sds;

	rtpcs_sds_write(sds, PAGE_SDS_EXT, 2, 0x85fa);
	rtpcs_sds_write(sds, PAGE_SDS_EXT, 3, 0x00);
	rtpcs_sds_write(sds, PAGE_SDS_EXT, 4, 0xdccc);
	rtpcs_sds_write(sds, PAGE_SDS_EXT, 5, 0x00);
	rtpcs_sds_write(sds, PAGE_SDS_EXT, 6, is_even ? 0x20d8 : 0x3600);
	rtpcs_sds_write(sds, PAGE_SDS_EXT, 7, 0x03);
	rtpcs_sds_write(sds, PAGE_SDS_EXT, 8, 0x79aa);
	rtpcs_sds_write(sds, PAGE_SDS_EXT, 9, 0x8c64);
	rtpcs_sds_write(sds, PAGE_SDS_EXT, 10, 0xc3);
	rtpcs_sds_write(sds, PAGE_SDS_EXT, 11, 0x1482);
	rtpcs_sds_write(sds, PAGE_SDS_EXT, 17, 0xb7c9);
	rtpcs_sds_write(sds, PAGE_SDS_EXT, 18, 0xab8e);
	rtpcs_sds_write(sds, PAGE_FIB, 24, 0x14aa);
	rtpcs_sds_write(sds, PAGE_FIB, 25, 0x303);
	rtpcs_sds_write(sds, PAGE_SDS_EXT, 14, 0xf002);
	rtpcs_sds_write(sds, PAGE_FIB, 27, 0x4bf);
}

static void rtpcs_838x_sds_reset(struct rtpcs_serdes *sds)
{
	rtpcs_sds_write_bits(sds, PAGE_SDS, 3, 6, 6, 0x1); /* REG3 SOFT_RST */
	rtpcs_sds_write_bits(sds, PAGE_SDS, 3, 6, 6, 0x0); /* REG3 SOFT_RST */
}

static void rtpcs_838x_sds_fill_caps(struct rtpcs_serdes *sds)
{
	__set_bit(RTPCS_SDS_MODE_OFF, sds->supported_modes);

	if (sds->id <= 4)
		__set_bit(RTPCS_SDS_MODE_QSGMII, sds->supported_modes);

	if (sds->id >= 4) {
		__set_bit(RTPCS_SDS_MODE_SGMII, sds->supported_modes);
		__set_bit(RTPCS_SDS_MODE_1000BASEX, sds->supported_modes);
	}
}

static int rtpcs_838x_sds_power(struct rtpcs_serdes *sds, bool power_on)
{
	u8 sds_id = sds->id;
	int ret;
	u8 val;

	val = power_on ? 0 : BIT(sds_id);

	ret = regmap_write_bits(sds->ctrl->map, RTPCS_838X_SDS_CFG_REG, BIT(sds_id), val);
	if (ret)
		return ret;

	if (sds_id >= 4)
		ret = regmap_write_bits(sds->ctrl->map, RTPCS_838X_SDS_CFG_REG,
					BIT(sds_id) << 2, val << 2); /* SDS*_PHY_MODE */

	return ret;
}

static int rtpcs_838x_sds_deactivate(struct rtpcs_serdes *sds)
{
	int ret;

	ret = rtpcs_838x_sds_power(sds, false);
	if (ret)
		return ret;

	/* EN_RX | EN_TX */
	ret = rtpcs_sds_write_bits(sds, PAGE_SDS, 0, 1, 0, 0x0);
	if (ret)
		return ret;

	/* CFG_FIB_PDOWN / BMCR_PDOWN */
	return rtpcs_sds_write_bits(sds, PAGE_FIB, MII_BMCR, 11, 11, 0x1);
}

static int rtpcs_838x_sds_activate(struct rtpcs_serdes *sds)
{
	int ret;

	rtpcs_838x_sds_reset(sds);

	/* CFG_FIB_PDOWN / BMCR_PDOWN */
	ret = rtpcs_sds_write_bits(sds, PAGE_FIB, MII_BMCR, 11, 11, 0x0);
	if (ret)
		return ret;

	/* EN_RX | EN_TX */
	ret = rtpcs_sds_write_bits(sds, PAGE_SDS, 0, 1, 0, 0x3);
	if (ret)
		return ret;

	return rtpcs_838x_sds_power(sds, true);
}

/*
 * RTL838X wrapper: after setting the MAC mode, SerDes 4-5 also need the
 * companion INT_MODE_CTRL field written.
 *
 * RTL838X doesn't support USXGMII, thus no submodes.
 */
static int rtpcs_838x_sds_set_mode(struct rtpcs_serdes *sds, enum rtpcs_sds_mode hw_mode,
				   enum rtpcs_sds_usxgmii_submode submode)
{
	u8 int_mode_shift, int_mode_val;
	int ret;

	ret = rtpcs_sds_set_mac_mode(sds, hw_mode);
	if (ret)
		return ret;

	if (sds->id < 4)
		return 0;

	int_mode_shift = (sds->id == 5) ? 3 : 0;
	switch (hw_mode) {
	case RTPCS_SDS_MODE_1000BASEX:
		int_mode_val = 0x1;
		break;
	case RTPCS_SDS_MODE_SGMII:
		int_mode_val = 0x2;
		break;
	case RTPCS_SDS_MODE_QSGMII:
		int_mode_val = 0x5;
		break;
	default:
		return -EINVAL;
	}

	return regmap_write_bits(sds->ctrl->map, RTPCS_838X_INT_MODE_CTRL,
				 0x7 << int_mode_shift, int_mode_val << int_mode_shift);
}

static int rtpcs_838x_sds_config_hw_mode(struct rtpcs_serdes *sds, enum rtpcs_sds_mode hw_mode)
{
	struct rtpcs_ctrl *ctrl = sds->ctrl;
	u8 sds_id = sds->id;

	rtpcs_sds_write(sds, PAGE_SDS, 1, 0xf00);
	usleep_range(1000, 2000);
	rtpcs_sds_write(sds, PAGE_SDS, 2, 0x7060);
	usleep_range(1000, 2000);

	if (sds_id >= 4) {
		rtpcs_sds_write(sds, PAGE_FIB, 30, 0x71e);
		usleep_range(1000, 2000);
		rtpcs_sds_write(sds, PAGE_SDS, 4, 0x74d);
		usleep_range(1000, 2000);
	}

	switch (hw_mode) {
	case RTPCS_SDS_MODE_1000BASEX:
		rtpcs_838x_sds_patch_fiber(sds);
		break;
	case RTPCS_SDS_MODE_QSGMII:
		rtpcs_838x_sds_patch_qsgmii(sds);
		break;
	default:
		break;
	}

	if (sds_id < 2) {
		/*
		 * These settings have to match to make QSGMII working.
		 * Testing showed that both variants work:
		 *   - CKREFBUF_S0S1 = 0xf + REG_CML_SEL = 0x1
		 *   - CKREFBUF_S0S1 = 0x0 + REG_CML_SEL = 0x0
		 */

		/* CKREFBUF_S0S1 */
		regmap_write_bits(ctrl->map, RTPCS_838X_PLL_CML_CTRL, 0xf, 0xf);
		rtpcs_sds_write_bits(sds, PAGE_SDS_EXT, 0x3, 1, 1, 0x1); /* REG_CML_SEL */
	}

	rtpcs_sds_write(sds, PAGE_SDS_EXT, 0x9, 0x8e64);
	rtpcs_sds_write(sds, PAGE_SDS_EXT, 0x9, 0x8c64);

	return 0;
}

static int rtpcs_838x_sds_probe(struct rtpcs_serdes *sds)
{
	u8 lsb = (5 - sds->id) * 5;

	sds->type = RTPCS_SDS_TYPE_5G;

	rtpcs_838x_sds_fill_caps(sds);

	/*
	 * SDS_MODE_SEL packs 5-bit fields in reverse order: SDS 0 at [25:29],
	 * SDS 5 at [0:4].
	 */
	return rtpcs_sds_alloc_field(sds, &sds->swcore_regs.mac_mode,
				     RTPCS_838X_SDS_MODE_SEL, lsb, lsb + 4);
}

static int rtpcs_838x_init(struct rtpcs_ctrl *ctrl)
{
	/* power off and reset all SerDes */
	regmap_write(ctrl->map, RTPCS_838X_SDS_CFG_REG, 0x3f);
	regmap_write(ctrl->map, RTPCS_838X_RST_GLB_CTRL_0, 0x10); /* SW_SERDES_RST */
	return 0;
}

static int rtpcs_838x_sds_post_config(struct rtpcs_serdes *sds, enum rtpcs_sds_mode hw_mode)
{
	/*
	 * Run a switch queue reset after the first start of a SerDes. This recovers ports that
	 * were already connected during boot and will not pass traffic. Sometimes the bug can
	 * be seen in registers INGR_DBG_REG0-INGR_DBG_REG2 but this is quite erratic. The SDK
	 * seems to have no issues because it starts all SerDes then PHYs and runs a queue reset
	 * finally during NIC start.
	 *
	 * Of course this is totally wrong here and should be part of the DSA driver. But
	 * implementing it over there requires more tricks than this (e.g. delayed work).
	 */
	if (sds->first_start)
		regmap_write(sds->ctrl->map, RTPCS_838X_RST_GLB_CTRL_0, 0x4);

	return 0;
}

/* RTL839X */

/*
 * RTL839X MAC_SERDES_IF_CTRL mode values.
 * From the vendor SDK; 100BASEX (0x8), 1000BASEX/SGMII (0x7) are documented
 * but not yet exercised here.
 */
static const s16 rtpcs_839x_sds_hw_mode_vals[RTPCS_SDS_MODE_MAX] = {
	[0 ... RTPCS_SDS_MODE_MAX - 1]  = -1,
	[RTPCS_SDS_MODE_OFF]            = 0x0,
	/* [RTPCS_SDS_MODE_100BASEX]    = 0x8, */
	/* [RTPCS_SDS_MODE_1000BASEX]   = 0x7, */
	/* [RTPCS_SDS_MODE_SGMII]       = 0x7, */
	[RTPCS_SDS_MODE_QSGMII]         = 0x6,
};

static void rtpcs_839x_sds_reset(struct rtpcs_serdes *sds)
{
	struct rtpcs_serdes *even_sds = rtpcs_sds_get_even(sds);
	struct rtpcs_serdes *odd_sds = rtpcs_sds_get_odd(sds);

	/* FIXME: The reset sequence seems to break some of the 5G SerDes
	 * though the SDK is calling it for all SerDes during init. Until
	 * this is solved, skip reset.
	 */
	if (sds->type == RTPCS_SDS_TYPE_5G)
		return;

	if (sds->type == RTPCS_SDS_TYPE_10G) {
		rtpcs_sds_write_bits(odd_sds, PAGE_ANA_10G_EXT, 0x1d, 3, 0, 0x5);
		msleep(500);
		rtpcs_sds_write_bits(odd_sds, PAGE_ANA_10G_EXT, 0x1d, 3, 0, 0xf);
		rtpcs_sds_write_bits(odd_sds, PAGE_ANA_10G_EXT, 0x1d, 3, 0, 0x0);

		rtpcs_sds_write_bits(even_sds, PAGE_ANA_10G, 0x10, 3, 3, 0x0);
		rtpcs_sds_write_bits(even_sds, PAGE_ANA_10G_EXT, 0x0, 15, 15, 0x1);
		msleep(100);
		rtpcs_sds_write_bits(even_sds, PAGE_ANA_10G_EXT, 0x0, 15, 15, 0x0);
	} else {
		rtpcs_sds_write(odd_sds, PAGE_ANA_1G2_EXT, 0x1, 0x0050);
		rtpcs_sds_write(odd_sds, PAGE_ANA_1G2_EXT, 0x1, 0x00f0);
		rtpcs_sds_write(odd_sds, PAGE_ANA_1G2_EXT, 0x1, 0x0000);

		rtpcs_sds_write_bits(sds, PAGE_ANA_1G2, 0x14, 0, 0, 0x0);
		rtpcs_sds_write_bits(sds, PAGE_ANA_1G2, 0x14, 9, 9, 0x1);
		msleep(100);
		rtpcs_sds_write_bits(sds, PAGE_ANA_1G2, 0x14, 9, 9, 0x0);
	}

	rtpcs_sds_write(even_sds, PAGE_SDS, 0x3, 0x7146);
	msleep(100);
	rtpcs_sds_write(even_sds, PAGE_SDS, 0x3, 0x7106);

	rtpcs_sds_write(odd_sds, PAGE_SDS, 0x3, 0x7146);
	msleep(100);
	rtpcs_sds_write(odd_sds, PAGE_SDS, 0x3, 0x7106);
}

static void rtpcs_839x_sds_fill_caps(struct rtpcs_serdes *sds)
{
	__set_bit(RTPCS_SDS_MODE_OFF, sds->supported_modes);

	if (sds->id <= 12)
		__set_bit(RTPCS_SDS_MODE_QSGMII, sds->supported_modes);

	/* Uncomment this when modes are supported
	if (sds->id >= 12) {
		__set_bit(RTPCS_SDS_MODE_SGMII, sds->supported_modes);
		__set_bit(RTPCS_SDS_MODE_100BASEX, sds->supported_modes);
		__set_bit(RTPCS_SDS_MODE_1000BASEX, sds->supported_modes);
	}
	*/
}

static int rtpcs_839x_sds_probe(struct rtpcs_serdes *sds)
{
	u8 id = sds->id;
	bool is_even = id % 2 == 0;
	u8 lsb = (id % 8) * 4;
	int ret;

	ret = rtpcs_sds_alloc_field(sds, &sds->swcore_regs.mac_mode,
				    RTPCS_839X_MAC_SERDES_IF_CTRL + (id / 8) * 4,
				    lsb, lsb + 3);
	if (ret)
		return ret;

	if (id == 8 || id == 9 || id == 12 || id == 13)
		sds->type = RTPCS_SDS_TYPE_10G;
	else
		sds->type = RTPCS_SDS_TYPE_5G;

	rtpcs_839x_sds_fill_caps(sds);

	/*
	 * This function is quite "mystic". It has been taken over from the vendor SDK function
	 * rtl839x_serdes_patch_init(). There is not much documentation about it but one could
	 * lookup the fields from the field headers. The 5G SerDes seem to work out of the box
	 * so only setup the 10G SerDes for now.
	 */
	if (sds->type == RTPCS_SDS_TYPE_5G)
		return 0;

	/* Part 1: register setup */
	rtpcs_sds_write(sds, PAGE_ANA_10G, 0x0, 0x5800);
	rtpcs_sds_write(sds, PAGE_ANA_10G, 0x1, 0x4000);
	rtpcs_sds_write(sds, PAGE_ANA_10G, 0x2, is_even ? 0x5400 : 0x5000);
	rtpcs_sds_write(sds, PAGE_ANA_10G, 0x3, 0x0000);
	rtpcs_sds_write(sds, PAGE_ANA_10G, 0x4, 0x0000);
	rtpcs_sds_write(sds, PAGE_ANA_10G, 0x5, 0x4000);
	rtpcs_sds_write(sds, PAGE_ANA_10G, 0x6, 0x4000);
	rtpcs_sds_write(sds, PAGE_ANA_10G, 0x7, 0xffff);
	rtpcs_sds_write(sds, PAGE_ANA_10G, 0x8, 0xffff);
	rtpcs_sds_write(sds, PAGE_ANA_10G, 0x9, 0x806f);
	rtpcs_sds_write(sds, PAGE_ANA_10G, 0xa, 0x0004);
	rtpcs_sds_write(sds, PAGE_ANA_10G, 0xb, 0x0000);
	rtpcs_sds_write(sds, PAGE_ANA_10G, 0xc, 0x0000);
	rtpcs_sds_write(sds, PAGE_ANA_10G, 0xd, 0x0000);
	rtpcs_sds_write(sds, PAGE_ANA_10G, 0xe, 0x0a00);
	rtpcs_sds_write(sds, PAGE_ANA_10G, 0xf, 0x2000);
	rtpcs_sds_write(sds, PAGE_ANA_10G, 0x10, 0xf00e);
	rtpcs_sds_write(sds, PAGE_ANA_10G, 0x11, is_even ? 0xf04a : 0xfdab);
	rtpcs_sds_write(sds, PAGE_ANA_10G, 0x12, is_even ? 0x97b3 : 0x96ea);
	rtpcs_sds_write(sds, PAGE_ANA_10G, 0x13, 0x5318);
	rtpcs_sds_write(sds, PAGE_ANA_10G, 0x14, 0x0f03);
	rtpcs_sds_write(sds, PAGE_ANA_10G, 0x15, 0x0000);
	rtpcs_sds_write(sds, PAGE_ANA_10G, 0x16, 0x0000);
	rtpcs_sds_write(sds, PAGE_ANA_10G, 0x17, 0x0000);
	rtpcs_sds_write(sds, PAGE_ANA_10G, 0x18, 0x0000);
	rtpcs_sds_write(sds, PAGE_ANA_10G, 0x19, 0x0000);
	rtpcs_sds_write(sds, PAGE_ANA_10G, 0x1a, 0xffff);
	rtpcs_sds_write(sds, PAGE_ANA_10G, 0x1b, 0x0000);
	rtpcs_sds_write(sds, PAGE_ANA_10G, 0x1c, 0x1203);
	rtpcs_sds_write(sds, PAGE_ANA_10G, 0x1d, 0x0000);
	rtpcs_sds_write(sds, PAGE_ANA_10G, 0x1e, 0xa052);
	rtpcs_sds_write(sds, PAGE_ANA_10G, 0x1f, 0x9a00);
	rtpcs_sds_write(sds, PAGE_ANA_10G_EXT, 0x0, 0x00f5);
	rtpcs_sds_write(sds, PAGE_ANA_10G_EXT, 0x1, 0xf000);
	rtpcs_sds_write(sds, PAGE_ANA_10G_EXT, 0x2, is_even ? 0x41ff : 0x4079);
	rtpcs_sds_write(sds, PAGE_ANA_10G_EXT, 0x3, 0x0000);
	rtpcs_sds_write(sds, PAGE_ANA_10G_EXT, 0x4, is_even ? 0x39ff : 0x93fa);
	rtpcs_sds_write(sds, PAGE_ANA_10G_EXT, 0x5, 0x3340);
	rtpcs_sds_write(sds, PAGE_ANA_10G_EXT, 0x6, is_even ? 0x40aa : 0x4280);
	rtpcs_sds_write(sds, PAGE_ANA_10G_EXT, 0x7, 0x0000);
	rtpcs_sds_write(sds, PAGE_ANA_10G_EXT, 0x8, 0x801f);
	rtpcs_sds_write(sds, PAGE_ANA_10G_EXT, 0x9, 0x0000);
	rtpcs_sds_write(sds, PAGE_ANA_10G_EXT, 0xa, 0x619c);
	rtpcs_sds_write(sds, PAGE_ANA_10G_EXT, 0xb, 0xffed);
	rtpcs_sds_write(sds, PAGE_ANA_10G_EXT, 0xc, 0x29ff);
	rtpcs_sds_write(sds, PAGE_ANA_10G_EXT, 0xd, 0x29ff);
	rtpcs_sds_write(sds, PAGE_ANA_10G_EXT, 0xe, is_even ? 0x4e10 : 0x4c50);
	rtpcs_sds_write(sds, PAGE_ANA_10G_EXT, 0xf, is_even ? 0x4e10 : 0x4c50);
	rtpcs_sds_write(sds, PAGE_ANA_10G_EXT, 0x10, 0x0000);
	rtpcs_sds_write(sds, PAGE_ANA_10G_EXT, 0x11, 0x0000);
	rtpcs_sds_write(sds, PAGE_SDS, 0xc, 0x08ec);
	if (!is_even)
		rtpcs_sds_write(sds, PAGE_ANA_10G_EXT, 0x1f, 0x003f);

	/* Part 2: register bit patching (contains some "reset flips") */
	rtpcs_sds_write_bits(sds, PAGE_SDS, 0x7, 14, 14, 0x0001);
	rtpcs_sds_write_bits(sds, PAGE_ANA_10G_EXT, 0x9, 15, 0, 0x417f);
	rtpcs_sds_write_bits(sds, PAGE_ANA_10G, 0x1c, 9, 9, 0x0000);
	rtpcs_sds_write_bits(sds, PAGE_ANA_10G, 0x1c, 12, 10, 0x0000);
	rtpcs_sds_write_bits(sds, PAGE_ANA_10G, 0x1c, 5, 3, 0x0005);
	rtpcs_sds_write_bits(sds, PAGE_ANA_10G, 0x1c, 8, 6, 0x0000);
	rtpcs_sds_write_bits(sds, PAGE_ANA_10G, 0x1c, 2, 0, 0x0002);
	rtpcs_sds_write_bits(sds, PAGE_ANA_10G_EXT, 0x1, 15, 0, 0xc440);
	if (is_even)
		rtpcs_sds_write_bits(sds, PAGE_ANA_10G_EXT, 0x6, 3, 3, 0x0000);
	rtpcs_sds_write_bits(sds, PAGE_ANA_10G, 0x5, 15, 0, 0x8000);
	rtpcs_sds_write_bits(sds, PAGE_ANA_10G, 0x6, 15, 0, 0x8000);
	rtpcs_sds_write_bits(sds, PAGE_ANA_10G, 0xa, 15, 0, 0x0000);
	rtpcs_sds_write_bits(sds, PAGE_ANA_10G, 0x1e, 15, 0, 0x0002);
	rtpcs_sds_write_bits(sds, PAGE_ANA_10G, 0x1f, 15, 0, 0xbe00);
	if (is_even) {
		rtpcs_sds_write_bits(sds, PAGE_ANA_10G_EXT, 0xe, 10, 10, 0x0000);
		rtpcs_sds_write_bits(sds, PAGE_ANA_10G_EXT, 0xf, 10, 10, 0x0000);
		rtpcs_sds_write_bits(sds, PAGE_ANA_10G_EXT, 0xe, 14, 14, 0x0000);
		rtpcs_sds_write_bits(sds, PAGE_ANA_10G_EXT, 0xf, 14, 14, 0x0000);
	}
	rtpcs_sds_write_bits(sds, PAGE_ANA_10G, 0x10, 5, 5, 0x0000);
	rtpcs_sds_write_bits(sds, PAGE_ANA_10G_EXT, 0x9, 8, 8, 0x0000);
	rtpcs_sds_write_bits(sds, PAGE_ANA_10G, 0x3, 15, 12, 0x000f);
	rtpcs_sds_write_bits(sds, PAGE_ANA_10G, 0x1f, 13, 12, 0x0003);
	rtpcs_sds_write_bits(sds, PAGE_ANA_10G, 0x1f, 11, 9, 0x0007);
	rtpcs_sds_write_bits(sds, PAGE_ANA_10G_EXT, 0x1, 15, 15, 0x0001);
	rtpcs_sds_write_bits(sds, PAGE_ANA_10G_EXT, 0x1, 14, 14, 0x0001);
	rtpcs_sds_write_bits(sds, PAGE_ANA_10G_EXT, 0x1, 13, 13, 0x0000);
	rtpcs_sds_write_bits(sds, PAGE_ANA_10G_EXT, 0x1, 12, 12, 0x0000);
	rtpcs_sds_write_bits(sds, PAGE_ANA_10G_EXT, 0x1, 11, 9, 0x0002);
	rtpcs_sds_write_bits(sds, PAGE_ANA_10G_EXT, 0x1, 8, 6, 0x0002);
	rtpcs_sds_write_bits(sds, PAGE_ANA_10G_EXT, 0x1, 5, 3, 0x0000);
	rtpcs_sds_write_bits(sds, PAGE_ANA_10G_EXT, 0x1, 2, 0, 0x0000);
	rtpcs_sds_write_bits(sds, PAGE_ANA_10G_EXT, 0xc, 9, 9, 0x0001);
	rtpcs_sds_write_bits(sds, PAGE_ANA_10G_EXT, 0xd, 9, 9, 0x0001);
	rtpcs_sds_write_bits(sds, PAGE_ANA_10G_EXT, 0x8, 5, 5, 0x0001);
	rtpcs_sds_write_bits(sds, PAGE_ANA_10G_EXT, 0x8, 6, 6, 0x0000);
	rtpcs_sds_write_bits(sds, PAGE_ANA_10G, 0x1c, 15, 15, 0x0000);
	rtpcs_sds_write_bits(sds, PAGE_ANA_10G, 0x10, 15, 12, 0x0000);
	rtpcs_sds_write_bits(sds, PAGE_ANA_10G, 0x13, 4, 4, 0x0000);
	rtpcs_sds_write_bits(sds, PAGE_ANA_10G, 0x13, 9, 9, 0x0000);
	rtpcs_sds_write_bits(sds, PAGE_ANA_10G, 0x13, 3, 0, 0x0008);
	rtpcs_sds_write_bits(sds, PAGE_ANA_10G, 0x13, 8, 5, 0x0008);

	return 0;
}

static int rtpcs_839x_init(struct rtpcs_ctrl *ctrl)
{
	/* reset all SerDes once after patching has been applied before */
	for (int sds_id = 0; sds_id < ctrl->cfg->serdes_count; sds_id++)
		rtpcs_839x_sds_reset(&ctrl->serdes[sds_id]);

	return 0;
}

/*
 * These no-op stubs satisfy the mandatory activate/deactivate contract until
 * real power sequencing is implemented.
 */
static int rtpcs_839x_sds_deactivate(struct rtpcs_serdes *sds)
{
	return 0;
}

static int rtpcs_839x_sds_activate(struct rtpcs_serdes *sds)
{
	rtpcs_839x_sds_reset(sds);
	return 0;
}

/*
 * Keep this as a no-op stub until RTL839x is extended to do proper configuration
 * here. E.g., the still missing SGMII, 100BASEX and 1000BASEX setup should go here.
 */
static int rtpcs_839x_sds_config_hw_mode(struct rtpcs_serdes *sds, enum rtpcs_sds_mode hw_mode)
{
	return 0;
}

static int rtpcs_839x_sds_set_mode(struct rtpcs_serdes *sds, enum rtpcs_sds_mode hw_mode,
				   enum rtpcs_sds_usxgmii_submode submode)
{
	/* USXGMII not supported, thus no submode handling */
	return rtpcs_sds_set_mac_mode(sds, hw_mode);
}

/* RTL93XX */

/* forward mapping: enum rtpcs_sds_mode, -1 = unsupported */
static const s16 rtpcs_93xx_sds_hw_mode_vals[RTPCS_SDS_MODE_MAX] = {
	[0 ... RTPCS_SDS_MODE_MAX - 1]		= -1,
	[RTPCS_SDS_MODE_OFF]			= RTPCS_93XX_SDS_MODE_OFF,
	[RTPCS_SDS_MODE_SGMII]			= RTPCS_93XX_SDS_MODE_SGMII,
	[RTPCS_SDS_MODE_1000BASEX]		= RTPCS_93XX_SDS_MODE_1000BASEX,
	[RTPCS_SDS_MODE_2500BASEX]		= RTPCS_93XX_SDS_MODE_2500BASEX,
	[RTPCS_SDS_MODE_10GBASER]		= RTPCS_93XX_SDS_MODE_10GBASER,
	[RTPCS_SDS_MODE_QSGMII]			= RTPCS_93XX_SDS_MODE_QSGMII,
	[RTPCS_SDS_MODE_XSGMII]			= RTPCS_93XX_SDS_MODE_XSGMII,
	[RTPCS_SDS_MODE_USXGMII]		= RTPCS_93XX_SDS_MODE_USXGMII,
};

static int rtpcs_93xx_sds_set_autoneg(struct rtpcs_serdes *sds, unsigned int neg_mode,
				      const unsigned long *advertising)
{
	u16 en_val;

	switch (sds->hw_mode) {
	case RTPCS_SDS_MODE_XSGMII:
		en_val = neg_mode == PHYLINK_PCS_NEG_INBAND_ENABLED ? 0x0 : 0x1;

		return rtpcs_sds_xsg_write_bits(sds, PAGE_SDS, 0x2, 9, 8, en_val);

	case RTPCS_SDS_MODE_USXGMII:
		/*
		 * QHSG_AN_EN_CHX: bits [3:0] enable AN on channels 3..0
		 *
		 * forced USXGMII link not supported yet, always activate USXGMII-AN
		 */
		return rtpcs_sds_write_bits(sds, PAGE_TGR_PRO_1, 0x11, 3, 0, 0xf);

	default:
		return rtpcs_generic_sds_set_autoneg(sds, neg_mode, advertising);
	}
}

static void rtpcs_93xx_sds_usxgmii_config(struct rtpcs_serdes *sds)
{
	/*
	 * Some USXGMII settings are labelled QHSG, presumably because USXGMII and
	 * (Realtek-proprietary) QHSGMII share a functional block.
	 */

	/* undocumented; part of USXGMII patch sequences in the SDK */
	rtpcs_sds_write(sds, PAGE_TGR_PRO_0, 0x00, 0x0000);
	rtpcs_sds_write(sds, PAGE_TGR_PRO_0, 0x0D, 0x0F00);
	rtpcs_sds_write(sds, PAGE_TGR_PRO_0, 0x1D, 0x0600);

	rtpcs_sds_write(sds, PAGE_TGR_PRO_1, 0x06, 0x1401); /* QHSG_TXCFG_MAC_CH0 */
	rtpcs_sds_write(sds, PAGE_TGR_PRO_1, 0x08, 0x1401); /* QHSG_TXCFG_MAC_CH1 */
	rtpcs_sds_write(sds, PAGE_TGR_PRO_1, 0x0a, 0x1401); /* QHSG_TXCFG_MAC_CH2 */
	rtpcs_sds_write(sds, PAGE_TGR_PRO_1, 0x0c, 0x1401); /* QHSG_TXCFG_MAC_CH3 */

	/* USXGMII AN mode */
	rtpcs_sds_write_bits(sds, PAGE_TGR_PRO_1, 0x10, 7, 0, RTPCS_USXGMII_AN_OPC_STD); /* QHSG_AN_OPC */
	rtpcs_sds_write_bits(sds, PAGE_TGR_PRO_0, 0x12, 15, 0, 0xa4); /* am_period */

	/* clear alignment markers */
	rtpcs_sds_write(sds, PAGE_TGR_PRO_0, 0x13, 0x0000); /* AM0_M1 | AM0_M0 */
	rtpcs_sds_write(sds, PAGE_TGR_PRO_0, 0x14, 0x0000); /* AM1_M0 | AM0_M2 */
	rtpcs_sds_write(sds, PAGE_TGR_PRO_0, 0x15, 0x0000); /* AM1_M2 | AM1_M1 */
	rtpcs_sds_write(sds, PAGE_TGR_PRO_0, 0x16, 0x0000); /* AM2_M1 | AM2_M0 */
	rtpcs_sds_write(sds, PAGE_TGR_PRO_0, 0x17, 0x0000); /* AM3_M0 | AM2_M2 */
	rtpcs_sds_write(sds, PAGE_TGR_PRO_0, 0x18, 0x0000); /* AM3_M2 | AM3_M1 */

	rtpcs_sds_write_bits(sds, PAGE_TGR_PRO_0, 0xe, 10, 10, 0x1);  /* an_table */
	rtpcs_sds_write_bits(sds, PAGE_TGR_PRO_0, 0x1d, 11, 10, 0x1); /* sync_bit */

	rtpcs_sds_write_bits(sds, PAGE_TGR_PRO_0, 0x03, 15, 15, 0x1); /* EEE_EN */
}

static int rtpcs_93xx_init(struct rtpcs_ctrl *ctrl)
{
	u32 model_info = 0;
	int rl_vid, val;

	regmap_read(ctrl->map, RTPCS_93XX_MODEL_NAME_INFO, &model_info);
	if (model_info & BIT(4))
		dev_warn(ctrl->dev, "ES chip variants may not work properly!\n");

	val = 0xa0000; /* CHIP_INFO_EN */
	regmap_write(ctrl->map, RTPCS_93XX_CHIP_INFO, val);
	regmap_read(ctrl->map, RTPCS_93XX_CHIP_INFO, &val);
	rl_vid = FIELD_GET(GENMASK(31, 28), val);

	if (rl_vid & BIT(0))
		ctrl->chip_version = RTPCS_CHIP_V2;

	val = 0;
	regmap_write(ctrl->map, RTPCS_93XX_CHIP_INFO, val);

	dev_dbg(ctrl->dev, "chip_version %u\n", ctrl->chip_version + 1);
	return 0;
}

static int rtpcs_93xx_sds_get_pll_config(struct rtpcs_serdes *sds, enum rtpcs_sds_pll_type pll,
					 enum rtpcs_sds_pll_speed *speed)
{
	struct rtpcs_serdes *even_sds = rtpcs_sds_get_even(sds);
	int sbit, speed_val;

	/*
	 * PLL config is shared between adjacent SerDes in the even lane. Each SerDes defines
	 * what PLL it needs (ring or LC) while the PLL itself stores the current speed.
	 */

	sbit = pll == RTPCS_SDS_PLL_TYPE_LC ? 8 : 12;
	speed_val = rtpcs_sds_read_bits(even_sds, PAGE_ANA_MISC, 0x12, sbit + 3, sbit);
	if (speed_val < 0)
		return speed_val;

	/* bit 0 is force-bit, bits [3:1] are speed selector */
	*speed = (enum rtpcs_sds_pll_speed)(speed_val >> 1);
	return 0;
}

static int rtpcs_93xx_sds_set_pll_config(struct rtpcs_serdes *sds, enum rtpcs_sds_pll_type pll,
					 enum rtpcs_sds_pll_speed speed)
{
	struct rtpcs_serdes *even_sds = rtpcs_sds_get_even(sds);
	int sbit = pll == RTPCS_SDS_PLL_TYPE_LC ? 8 : 12;
	int ret;

	if (speed >= RTPCS_SDS_PLL_SPD_END)
		return -EINVAL;

	if (pll >= RTPCS_SDS_PLL_TYPE_END)
		return -EINVAL;

	if ((pll == RTPCS_SDS_PLL_TYPE_RING) && (speed == RTPCS_SDS_PLL_SPD_10000))
		return -EINVAL;

	/*
	 * A SerDes clock can either be taken from the low speed ring PLL or the high speed
	 * LC PLL. As it is unclear if disabling PLLs has any positive or negative effect,
	 * always activate both.
	 */
	ret = rtpcs_sds_write_bits(even_sds, PAGE_ANA_MISC, 0x12, 3, 0, 0xf);
	if (ret < 0)
		return ret;

	/* bit 0 is force-bit, bits [3:1] are speed selector */
	ret = rtpcs_sds_write_bits(even_sds, PAGE_ANA_MISC, 0x12, sbit + 3, sbit,
				   (speed << 1) | BIT(0));
	if (ret < 0)
		return ret;

	if (sds->ops->reset_cmu)
		ret = sds->ops->reset_cmu(sds, pll);

	return ret;
}

static int rtpcs_93xx_sds_config_cmu(struct rtpcs_serdes *sds, enum rtpcs_sds_mode hw_mode)
{
	struct rtpcs_serdes *nb_sds = rtpcs_sds_get_neighbor(sds);
	enum rtpcs_sds_pll_speed speed, neighbor_speed;
	enum rtpcs_sds_pll_type pll, neighbor_pll;
	bool speed_changed = true;
	int ret;

	/*
	 * A SerDes pair on RTL93xx is driven by a shared CMU with two PLLs:
	 *
	 * - a low speed ring PLL which can generate signals of 1.25G and 3.125G for link
	 *   speeds of 1G/2.5G
	 * - a high speed LC PLL which can additionally generate a 10.3125G signal for
	 *   10G link speeds
	 *
	 * To drive the pair at different speeds, each SerDes must use its own PLL and we
	 * must wisely assign the PLLs to the SerDes based on their needs. The logic boils
	 * down to the following rules:
	 *
	 * - use ring PLL for slow 1G speeds
	 * - use LC PLL for fast 10G speeds
	 * - for 2.5G prefer ring over LC PLL
	 *
	 * For the case that we want to configure 10G speed but the LC PLL is already used
	 * by the neighbor SerDes and running with a slower speed, there's no way to avoid
	 * reconfiguration. The neighbor SerDes is reconfigured online to the ring PLL.
	 */

	if (hw_mode == RTPCS_SDS_MODE_OFF)
		return 0;

	ret = rtpcs_sds_select_pll_speed(hw_mode, &speed);
	if (ret < 0)
		return ret;

	if (nb_sds->hw_mode == RTPCS_SDS_MODE_OFF) {
		pll = (speed == RTPCS_SDS_PLL_SPD_10000) ? RTPCS_SDS_PLL_TYPE_LC
							 : RTPCS_SDS_PLL_TYPE_RING;
		goto pll_setup;
	}

	ret = nb_sds->ops->get_pll_select(nb_sds, &neighbor_pll);
	if (ret < 0)
		return ret;

	ret = rtpcs_93xx_sds_get_pll_config(nb_sds, neighbor_pll, &neighbor_speed);
	if (ret < 0)
		return ret;

	if (speed == neighbor_speed) {
		speed_changed = false;
		pll = neighbor_pll;
	} else if (neighbor_pll == RTPCS_SDS_PLL_TYPE_RING)
		pll = RTPCS_SDS_PLL_TYPE_LC;
	else if (speed == RTPCS_SDS_PLL_SPD_10000) {
		dev_info(sds->ctrl->dev,
			 "SerDes %d needs LC PLL, reconfigure SDS %d to use ring PLL\n",
			 sds->id, nb_sds->id);

		ret = nb_sds->ops->reconfigure_to_pll(nb_sds, RTPCS_SDS_PLL_TYPE_RING);
		if (ret < 0)
			return ret;

		pll = RTPCS_SDS_PLL_TYPE_LC;
	} else
		pll = RTPCS_SDS_PLL_TYPE_RING;

pll_setup:
	if (speed_changed) {
		ret = rtpcs_93xx_sds_set_pll_config(sds, pll, speed);
		if (ret < 0)
			return ret;
	}

	ret = sds->ops->set_pll_select(sds, hw_mode, pll);
	if (ret < 0)
		return ret;

	dev_dbg(sds->ctrl->dev, "SerDes %d using %s PLL for mode %d\n", sds->id,
		pll == RTPCS_SDS_PLL_TYPE_LC ? "LC" : "ring", hw_mode);
	return ret;
}

static const s16 rtpcs_93xx_sds_usxgmii_submodes[RTPCS_SDS_USXGMII_SM_MAX] = {
	[RTPCS_SDS_USXGMII_SM_NONE]       = -1,
	[RTPCS_SDS_USXGMII_SM_10GSXGMII]  = RTPCS_93XX_SDS_USXGMII_SUBMODE_10GSX,
	[RTPCS_SDS_USXGMII_SM_10GDXGMII]  = RTPCS_93XX_SDS_USXGMII_SUBMODE_10GDX,
	[RTPCS_SDS_USXGMII_SM_10GQXGMII]  = RTPCS_93XX_SDS_USXGMII_SUBMODE_10GQX,
	[RTPCS_SDS_USXGMII_SM_5GSXGMII]   = RTPCS_93XX_SDS_USXGMII_SUBMODE_5GSX,
	[RTPCS_SDS_USXGMII_SM_5GDXGMII]   = RTPCS_93XX_SDS_USXGMII_SUBMODE_5GDX,
	[RTPCS_SDS_USXGMII_SM_2_5GSXGMII] = RTPCS_93XX_SDS_USXGMII_SUBMODE_2_5GSX,
};

static int rtpcs_93xx_sds_apply_usxgmii_submode(struct rtpcs_serdes *sds,
						enum rtpcs_sds_usxgmii_submode submode)
{
	s16 val = rtpcs_93xx_sds_usxgmii_submodes[submode];

	if (val < 0)
		return 0;

	if (!sds->swcore_regs.usxgmii_submode)
		return -EOPNOTSUPP;

	return regmap_field_write(sds->swcore_regs.usxgmii_submode, val);
}

/*
 * RTL93XX wrapper: set MAC mode, then handle variant-specific extras:
 * - post-write delay (required on 930x)
 * - force-mode bit (931x only; nullable field)
 *
 * Each extra no-ops on the variant that doesn't need it — either because
 * the corresponding regmap_field is NULL, or because the mode doesn't match.
 */
static int rtpcs_93xx_sds_set_mac_mode(struct rtpcs_serdes *sds, enum rtpcs_sds_mode hw_mode)
{
	int ret;

	ret = rtpcs_sds_set_mac_mode(sds, hw_mode);
	if (ret)
		return ret;
	msleep(10);

	if (sds->swcore_regs.mac_mode_force) {
		ret = regmap_field_write(sds->swcore_regs.mac_mode_force, 1);
		if (ret)
			return ret;
	}

	return 0;
}

/*
 * Bring up a MAC-driven mode: release the IP mode force-lock so the MAC
 * side takes over (deactivate forces IP=OFF; this undoes that), set the
 * MAC mode, then apply the USXGMII submode if the mode needs one.
 */
static int rtpcs_93xx_sds_set_mac_driven_mode(struct rtpcs_serdes *sds,
					      enum rtpcs_sds_mode hw_mode,
					      enum rtpcs_sds_usxgmii_submode submode)
{
	int ret;

	ret = rtpcs_sds_write_bits(sds, PAGE_WDIG, 0x09, 6, 6, 0);
	if (ret)
		return ret;

	ret = rtpcs_93xx_sds_set_mac_mode(sds, hw_mode);
	if (ret)
		return ret;

	return rtpcs_93xx_sds_apply_usxgmii_submode(sds, submode);
}

/*
 * Read/write the SerDes IP mode register: page 0x1f reg 0x09, bits 11:7
 * hold the 5-bit mode value, bit 6 is the "force mode" enable. The same
 * physical field is used on RTL930x and RTL931x.
 */
static int rtpcs_93xx_sds_get_ip_mode(struct rtpcs_serdes *sds)
{
	const s16 *vals = sds->ctrl->cfg->sds_hw_mode_vals;
	int raw;

	raw = rtpcs_sds_read_bits(sds, PAGE_WDIG, 0x09, 11, 7);
	if (raw < 0)
		return raw;

	for (int i = 0; i < RTPCS_SDS_MODE_MAX; i++)
		if (vals[i] == raw)
			return i;

	return -ENOENT;
}

static int rtpcs_93xx_sds_set_ip_mode(struct rtpcs_serdes *sds, enum rtpcs_sds_mode hw_mode)
{
	int raw;

	if (hw_mode >= RTPCS_SDS_MODE_MAX)
		return -EINVAL;

	raw = sds->ctrl->cfg->sds_hw_mode_vals[hw_mode];
	if (raw < 0)
		return -EOPNOTSUPP;

	/* BIT(0) is force mode enable bit */
	return rtpcs_sds_write_bits(sds, PAGE_WDIG, 0x09, 11, 6, raw << 1 | BIT(0));
}

static void rtpcs_93xx_sds_fill_caps(struct rtpcs_serdes *sds)
{
	__set_bit(RTPCS_SDS_MODE_OFF, sds->supported_modes);

	switch (sds->type) {
	case RTPCS_SDS_TYPE_5G:
		__set_bit(RTPCS_SDS_MODE_QSGMII, sds->supported_modes);
		break;
	case RTPCS_SDS_TYPE_10G:
		__set_bit(RTPCS_SDS_MODE_SGMII, sds->supported_modes);
		__set_bit(RTPCS_SDS_MODE_XSGMII, sds->supported_modes);
		__set_bit(RTPCS_SDS_MODE_USXGMII, sds->supported_modes);

		__set_bit(RTPCS_SDS_MODE_1000BASEX, sds->supported_modes);
		__set_bit(RTPCS_SDS_MODE_2500BASEX, sds->supported_modes);
		__set_bit(RTPCS_SDS_MODE_10GBASER, sds->supported_modes);
		break;
	case RTPCS_SDS_TYPE_UNKNOWN:
	default:
		break;
	}
}

static int rtpcs_93xx_sds_get_cmu_page(enum rtpcs_sds_mode hw_mode)
{
	switch (hw_mode) {
	case RTPCS_SDS_MODE_SGMII:
	case RTPCS_SDS_MODE_1000BASEX:
		return PAGE_ANA_1G2;
	case RTPCS_SDS_MODE_2500BASEX:
		return PAGE_ANA_3G1;
	case RTPCS_SDS_MODE_QSGMII:
		return PAGE_ANA_5G0;
	//	return PAGE_ANA_6G0;
	case RTPCS_SDS_MODE_XSGMII:
	case RTPCS_SDS_MODE_USXGMII:
	case RTPCS_SDS_MODE_10GBASER:
		return PAGE_ANA_10G;
	default:
		return -EOPNOTSUPP;
	}
}

/* RTL930X */

/* This mapping is not coherent so it cannot be expressed arithmetically */
static const struct reg_field rtpcs_930x_mac_mode_fields[RTPCS_930X_SERDES_CNT] = {
	[0]  = REG_FIELD(RTPCS_930X_SDS_MODE_SEL_0,  0,  4),
	[1]  = REG_FIELD(RTPCS_930X_SDS_MODE_SEL_0,  6, 10),
	[2]  = REG_FIELD(RTPCS_930X_SDS_MODE_SEL_0, 12, 16),
	[3]  = REG_FIELD(RTPCS_930X_SDS_MODE_SEL_0, 18, 22),
	[4]  = REG_FIELD(RTPCS_930X_SDS_MODE_SEL_1,  0,  4),
	[5]  = REG_FIELD(RTPCS_930X_SDS_MODE_SEL_1,  6, 10),
	[6]  = REG_FIELD(RTPCS_930X_SDS_MODE_SEL_1, 12, 16),
	[7]  = REG_FIELD(RTPCS_930X_SDS_MODE_SEL_1, 18, 22),
	[8]  = REG_FIELD(RTPCS_930X_SDS_MODE_SEL_2,  0,  4),
	[9]  = REG_FIELD(RTPCS_930X_SDS_MODE_SEL_2,  6, 10),
	[10] = REG_FIELD(RTPCS_930X_SDS_MODE_SEL_3,  0,  4),
	[11] = REG_FIELD(RTPCS_930X_SDS_MODE_SEL_3,  6, 10),
};

static const struct reg_field rtpcs_930x_usxgmii_submode_fields[] = {
	[0] = REG_FIELD(RTPCS_930X_SDS_SUBMODE_CTRL_0,  0,  4),
	[1] = REG_FIELD(RTPCS_930X_SDS_SUBMODE_CTRL_0,  5,  9),
	[2] = REG_FIELD(RTPCS_930X_SDS_SUBMODE_CTRL_1,  0,  4),
	[3] = REG_FIELD(RTPCS_930X_SDS_SUBMODE_CTRL_1,  5,  9),
	[4] = REG_FIELD(RTPCS_930X_SDS_SUBMODE_CTRL_1, 10, 14),
	[5] = REG_FIELD(RTPCS_930X_SDS_SUBMODE_CTRL_1, 15, 19),
	[6] = REG_FIELD(RTPCS_930X_SDS_SUBMODE_CTRL_1, 20, 24),
	[7] = REG_FIELD(RTPCS_930X_SDS_SUBMODE_CTRL_1, 25, 29),
};

static const struct rtpcs_sds_tx_config rtpcs_930x_sds_tx_config_1g = {
	.pre_amp = 0x1, .main_amp = 0x9, .post_amp = 0x1, .impedance = 0x8
};

static const struct rtpcs_sds_tx_config rtpcs_930x_sds_tx_config_2g5 = {
	.main_amp = 0x9, .post_amp = 0x8, .impedance = 0x8
};

static const struct rtpcs_sds_tx_config rtpcs_930x_sds_tx_config_10g = {
	.main_amp = 0x10, .impedance = 0x8
};

/*
 * PHY-attached 10G-class links (XSGMII, USXGMII): weaker than the fiber/DAC
 * value above since the far-end PHY equalizes on its own. main_amp reuses
 * the 1G/2.5G fiber override's value, also the vendor SDK's genuine-fiber
 * (non-DAC) 10G value; a non-adaptive receiver needing no more than this
 * should be a safe upper bound for a PHY that equalizes on top.
 */
static const struct rtpcs_sds_tx_config rtpcs_930x_sds_tx_config_phy = {
	.main_amp = 0x9, .impedance = 0x8
};

/*
 * RTL930X needs a special mapping from logic SerDes ID to physical SerDes ID,
 * which takes the page into account. This applies to most of read/write calls.
 */
static int rtpcs_930x_sds_get_phys_sds_id(int sds_id, int page)
{
        if (sds_id == 3 && page < 4)
                return 10;

        return sds_id;
}

static int rtpcs_930x_sds_op_read(struct rtpcs_serdes *sds, enum rtpcs_page page, int regnum,
				  int bithigh, int bitlow)
{
	int sds_id = rtpcs_930x_sds_get_phys_sds_id(sds->id, page);

	return __rtpcs_sds_read_raw(sds->ctrl, sds_id, page, regnum, bithigh, bitlow);
}

static int rtpcs_930x_sds_op_write(struct rtpcs_serdes *sds, enum rtpcs_page page, int regnum,
				   int bithigh, int bitlow, u16 value)
{
	int sds_id = rtpcs_930x_sds_get_phys_sds_id(sds->id, page);

	return __rtpcs_sds_write_raw(sds->ctrl, sds_id, page, regnum, bithigh, bitlow, value);
}

static int rtpcs_930x_sds_op_write_mask(struct rtpcs_serdes *sds, enum rtpcs_page page,
					int regnum, u16 mask, u16 value)
{
	int sds_id = rtpcs_930x_sds_get_phys_sds_id(sds->id, page);

	return __rtpcs_sds_write_mask(sds->ctrl, sds_id, page, regnum, mask, value);
}

/*
 * Realtek uses some nasty logic for digital parts of SerDes 2 and 3.
 *
 * This implements 'dal_longan_sds_xsg_field_write' and a combination of
 * '_rtl9300_serdes_index_to_physical' and '_rtl9300_serdes_reg_write' from the SDK.
 */
static int rtpcs_930x_sds_op_xsg_write(struct rtpcs_serdes *sds, enum rtpcs_page page, int regnum,
				       int bithigh, int bitlow, u16 value)
{
	int phys_sds_id, ret;

	switch (sds->id) {
	case 2:
		phys_sds_id = 2;
		break;
	case 3:
		phys_sds_id = 10;
		break;
	default:
		return -ENOTSUPP;
	}

	if (page >= 4)
		return sds->ops->write(sds, page, regnum, bithigh, bitlow, value);

	ret = __rtpcs_sds_write_raw(sds->ctrl, phys_sds_id, page, regnum, bithigh, bitlow, value);
	if (ret)
		return ret;

	return __rtpcs_sds_write_raw(sds->ctrl, phys_sds_id + 1, page, regnum, bithigh, bitlow,
				     value);
}

static void rtpcs_930x_sds_rx_reset(struct rtpcs_serdes *sds,
				    enum rtpcs_sds_mode hw_mode)
{
	enum rtpcs_page page = PAGE_ANA_10G; /* 10GR and USXGMII */

	if (hw_mode == RTPCS_SDS_MODE_1000BASEX)
		page = PAGE_ANA_1G2;

	rtpcs_sds_write_bits(sds, page, 0x15, 4, 4, 0x1);
	usleep_range(5000, 6000);
	rtpcs_sds_write_bits(sds, page, 0x15, 4, 4, 0x0);
}

static int rtpcs_930x_sds_get_pll_select(struct rtpcs_serdes *sds, enum rtpcs_sds_pll_type *pll)
{
	struct rtpcs_serdes *even_sds = rtpcs_sds_get_even(sds);
	int pbit = (sds == even_sds) ? 4 : 6;
	int pll_sel;

	pll_sel = rtpcs_sds_read_bits(even_sds, PAGE_ANA_MISC, 0x12, pbit + 1, pbit);
	if (pll_sel < 0)
		return pll_sel;

	/* bit 0 is force-bit, bit 1 is PLL selector */
	*pll = (enum rtpcs_sds_pll_type)(pll_sel >> 1);
	return 0;
}

static int rtpcs_930x_sds_set_pll_select(struct rtpcs_serdes *sds, enum rtpcs_sds_mode hw_mode,
					 enum rtpcs_sds_pll_type pll)
{
	struct rtpcs_serdes *even_sds = rtpcs_sds_get_even(sds);
	int pbit = (sds == even_sds) ? 4 : 6;

	/* PLL selection in even lane register. bit 0 is force-bit, bit 1 is PLL selector */
	return rtpcs_sds_write_bits(even_sds, PAGE_ANA_MISC, 0x12, pbit + 1, pbit,
				    (pll << 1) | BIT(0));
}

static int rtpcs_930x_sds_reset_cmu(struct rtpcs_serdes *sds, enum rtpcs_sds_pll_type pll)
{
	struct rtpcs_serdes *even_sds = rtpcs_sds_get_even(sds);
	int reset_sequence[4] = { 3, 2, 3, 1 };
	int bit, i, ret;

	/*
	 * After the PLL speed has changed, the CMU must take over the new values. The models
	 * of the Otto platform have different reset sequences. Luckily it always boils down
	 * to flipping two bits in a special sequence.
	 */

	bit = pll == RTPCS_SDS_PLL_TYPE_LC ? 2 : 0;

	for (i = 0; i < ARRAY_SIZE(reset_sequence); i++) {
		ret = rtpcs_sds_write_bits(even_sds, PAGE_ANA_COM, 0x0b, bit + 1, bit,
					   reset_sequence[i]);
		if (ret < 0)
			return ret;
	}
	return 0;
}

static int rtpcs_930x_sds_wait_clock_ready(struct rtpcs_serdes *sds)
{
	struct rtpcs_serdes *even_sds = rtpcs_sds_get_even(sds);
	int i, ready, ready_cnt = 0, bit = (sds == even_sds) ? 4 : 5;

	/*
	 * While reconfiguring a SerDes it might take some time until its clock is in sync with
	 * the PLL. During that timespan the ready signal might toggle randomly. According to
	 * GPL sources it is enough to verify that 3 consecutive clock ready checks say "ok".
	 */

	for (i = 0; i < 20; i++) {
		usleep_range(10000, 15000);

		rtpcs_sds_write(even_sds, PAGE_WDIG, 0x02, 53);
		ready = rtpcs_sds_read_bits(even_sds, PAGE_WDIG, 0x14, bit, bit);

		ready_cnt = ready ? ready_cnt + 1 : 0;
		if (ready_cnt >= 3)
			return 0;
	}

	return -EBUSY;
}

static void rtpcs_930x_sds_set_power(struct rtpcs_serdes *sds, bool on)
{
	int power_down, rx_enable;

	power_down = on ? RTL93XX_FRC_PDOWN_UNFORCED : RTL93XX_FRC_PDOWN_DOWN;
	rx_enable = on ? RTL93XX_FRC_RX_EN_ON : RTL93XX_FRC_RX_EN_OFF;

	rtpcs_sds_write_mask(sds, PAGE_ANA_MISC, 0x00,
			     RTL93XX_FRC_PDOWN_MASK | RTL93XX_FRC_RX_EN_MASK,
			     power_down | rx_enable);
}

static int rtpcs_930x_sds_reconfigure_to_pll(struct rtpcs_serdes *sds, enum rtpcs_sds_pll_type pll)
{
	enum rtpcs_sds_pll_speed speed;
	enum rtpcs_sds_pll_type old_pll;
	int hw_mode, ret;

	hw_mode = rtpcs_93xx_sds_get_ip_mode(sds);
	if (hw_mode < 0)
		return hw_mode;

	ret = rtpcs_930x_sds_get_pll_select(sds, &old_pll);
	if (ret < 0)
		return ret;

	ret = rtpcs_93xx_sds_get_pll_config(sds, old_pll, &speed);
	if (ret < 0)
		return ret;

	rtpcs_930x_sds_set_power(sds, false);
	rtpcs_93xx_sds_set_ip_mode(sds, RTPCS_SDS_MODE_OFF);

	ret = rtpcs_93xx_sds_set_pll_config(sds, pll, speed);
	if (ret < 0)
		return ret;

	ret = rtpcs_930x_sds_set_pll_select(sds, sds->hw_mode, pll);
	if (ret < 0)
		return ret;

	rtpcs_93xx_sds_set_ip_mode(sds, hw_mode);
	if (rtpcs_930x_sds_wait_clock_ready(sds))
		dev_err(sds->ctrl->dev, "SerDes %d could not sync clock\n", sds->id);

	rtpcs_930x_sds_set_power(sds, true);
	return 0;
}

static void rtpcs_930x_sds_reset_state_machine(struct rtpcs_serdes *sds)
{
	rtpcs_sds_write_bits(sds, PAGE_TGR_PRO_0, 0x02, 12, 12, 0x01); /* SM_RESET */
	usleep_range(10000, 20000);
	rtpcs_sds_write_bits(sds, PAGE_TGR_PRO_0, 0x02, 12, 12, 0x00);
	usleep_range(10000, 20000);
}

static int rtpcs_930x_sds_init_state_machine(struct rtpcs_serdes *sds,
					     enum rtpcs_sds_mode hw_mode)
{
	int loopback, link, cnt = 20, ret = -EBUSY;

	if (hw_mode != RTPCS_SDS_MODE_10GBASER)
		return 0;
	/*
	 * After a SerDes mode change it takes some time until the frontend state machine
	 * works properly for 10G. To verify operation readyness run a connection check via
	 * loopback.
	 */
	loopback = rtpcs_sds_read_bits(sds, PAGE_TGR_PRO_0, 0x01, 2, 2); /* AFE_LPK */
	rtpcs_sds_write_bits(sds, PAGE_TGR_PRO_0, 0x01, 2, 2, 0x01);

	while (cnt-- && ret) {
		rtpcs_930x_sds_reset_state_machine(sds);

		/* 10G link state (latched) */
		link = rtpcs_sds_read_bits(sds, PAGE_TGR_STD_1, 0x00, 12, 12);
		link = rtpcs_sds_read_bits(sds, PAGE_TGR_STD_1, 0x00, 12, 12);
		if (link)
			ret = 0;
	}

	rtpcs_sds_write_bits(sds, PAGE_TGR_PRO_0, 0x01, 2, 2, loopback);
	rtpcs_930x_sds_reset_state_machine(sds);

	return ret;
}

static int rtpcs_930x_sds_apply_ip_mode(struct rtpcs_serdes *sds,
					enum rtpcs_sds_mode hw_mode)
{
	struct device *dev = sds->ctrl->dev;
	int ret;

	/*
	 * TODO: Usually one would expect that it is enough to modify the SDS_MODE_SEL_*
	 * registers (lets call it MAC setup). It seems as if this complex sequence is only
	 * needed for modes that cannot be set by the SoC itself. Additionally it is unclear
	 * if this sequence should quit early in case of errors.
	 */

	ret = rtpcs_93xx_sds_set_ip_mode(sds, RTPCS_SDS_MODE_OFF);
	if (ret < 0)
		return ret;

	if (hw_mode == RTPCS_SDS_MODE_OFF)
		return 0;

	ret = rtpcs_93xx_sds_config_cmu(sds, hw_mode);
	if (ret < 0)
		dev_err(dev, "SerDes %d could not configure PLL for mode %d: %d\n",
			sds->id, hw_mode, ret);

	ret = rtpcs_93xx_sds_set_ip_mode(sds, hw_mode);
	if (ret < 0)
		return ret;

	if (rtpcs_930x_sds_wait_clock_ready(sds))
		dev_err(dev, "SerDes %d could not sync clock\n", sds->id);

	if (rtpcs_930x_sds_init_state_machine(sds, hw_mode))
		dev_err(dev, "SerDes %d could not reset state machine\n", sds->id);

	return 0;
}

static int rtpcs_930x_sds_set_mode(struct rtpcs_serdes *sds, enum rtpcs_sds_mode hw_mode,
				   enum rtpcs_sds_usxgmii_submode submode)
{
	/*
	 * Several modes can be configured via MAC setup, just by setting
	 * a register to a specific value and the MAC will configure
	 * "everything" as needed. For some modes, this seems incomplete and
	 * we need to do manual configuration in the SerDes IP core itself.
	 */

	switch (hw_mode) {
	case RTPCS_SDS_MODE_SGMII:
	case RTPCS_SDS_MODE_1000BASEX:
	case RTPCS_SDS_MODE_2500BASEX:
	case RTPCS_SDS_MODE_10GBASER:
		return rtpcs_930x_sds_apply_ip_mode(sds, hw_mode);

	default:
		break;
	}

	return rtpcs_93xx_sds_set_mac_driven_mode(sds, hw_mode, submode);
}

static int rtpcs_930x_sds_deactivate(struct rtpcs_serdes *sds)
{
	int ret;

	/* Power down the SerDes core analog block. */
	rtpcs_930x_sds_set_power(sds, false);

	/* Force MAC and IP mode registers to OFF, leaving the SerDes inert. */
	ret = rtpcs_93xx_sds_set_mac_mode(sds, RTPCS_SDS_MODE_OFF);
	if (ret)
		return ret;

	ret = rtpcs_93xx_sds_set_ip_mode(sds, RTPCS_SDS_MODE_OFF);
	if (ret)
		return ret;

	/* Disable fiber RX. */
	ret = rtpcs_sds_write_bits(sds, PAGE_ANA_MISC, 2, 12, 12, 1);
	if (ret)
		return ret;

	/* Power down the 1G PHY block. */
	ret = rtpcs_sds_write_bits(sds, PAGE_FIB, MII_BMCR, 11, 11, 1); /* BMCR_PDOWN */
	if (ret)
		return ret;

	/* Power down the 10G PHY block. */
	return rtpcs_sds_write_bits(sds, PAGE_TGR_STD_0, MII_BMCR, 11, 11, 1); /* BMCR_PDOWN */
}

static int rtpcs_930x_sds_activate(struct rtpcs_serdes *sds)
{
	int ret;

	/* Power up the SerDes core analog block and reset its RX path. */
	rtpcs_930x_sds_set_power(sds, true);
	rtpcs_930x_sds_rx_reset(sds, sds->hw_mode);

	/* Enable fiber RX. */
	ret = rtpcs_sds_write_bits(sds, PAGE_ANA_MISC, 2, 12, 12, 0);
	if (ret)
		return ret;

	/* Power up the 1G PHY block. */
	ret = rtpcs_sds_write_bits(sds, PAGE_FIB, MII_BMCR, 11, 11, 0); /* BMCR_PDOWN */
	if (ret)
		return ret;

	/* Power up the 10G PHY block. */
	return rtpcs_sds_write_bits(sds, PAGE_TGR_STD_0, MII_BMCR, 11, 11, 0); /* BMCR_PDOWN */
}

static int rtpcs_930x_sds_tx_config(struct rtpcs_serdes *sds, enum rtpcs_sds_mode hw_mode,
				    const struct rtpcs_sds_tx_config *tx_conf)
{
	enum rtpcs_page page;
	int ret;

	ret = rtpcs_93xx_sds_get_cmu_page(hw_mode);
	if (ret < 0)
		return ret;

	/* TX config happens on extended CMU page */
	page = ret + 1;

	ret = rtpcs_sds_write_bits(sds, page, 0x18, 15, 12, tx_conf->impedance);
	if (!ret)
		ret = rtpcs_sds_write_bits(sds, page, 0x07, 8, 4, tx_conf->main_amp);

	/* pre-amp */
	if (!ret)
		ret = rtpcs_sds_write_bits(sds, page, 0x01, 15, 11, tx_conf->pre_amp);
	if (!ret)
		ret = rtpcs_sds_write_bits(sds, page, 0x07, 0, 0, tx_conf->pre_amp ? 1 : 0);

	/* post-amp */
	if (!ret)
		ret = rtpcs_sds_write_bits(sds, page, 0x06, 4, 0, tx_conf->post_amp);
	if (!ret)
		ret = rtpcs_sds_write_bits(sds, page, 0x07, 3, 3, tx_conf->post_amp ? 1 : 0);

	return ret;
}

static int rtpcs_930x_sds_set_debug(struct rtpcs_serdes *sds, unsigned int debug_sel)
{
	struct rtpcs_serdes *even_sds = rtpcs_sds_get_even(sds);
	int ret;

	ret = rtpcs_sds_write(even_sds, PAGE_WDIG, 0x2, (sds == even_sds) ? 0x2f : 0x31);
	if (ret < 0)
		return ret;

	ret = rtpcs_sds_write_bits(sds, PAGE_ANA_10G, 0x15, 9, 9, 0x1);	/* RX_EN_TEST */
	if (ret < 0)
		return ret;

	return rtpcs_sds_write_bits(sds, PAGE_ANA_COM, 0x06, 11, 6, debug_sel); /* RX_DEBUG_SEL */
}

static int rtpcs_930x_sds_rxeq_dcvs_set_adapt(struct rtpcs_serdes *sds, unsigned int dcvs_id,
					      bool enable)
{
	u8 reg[6] = { 0x1e, 0x1e, 0x1e, 0x1e, 0x01, 0x02 };
	u8 bit[6] = { 14, 13, 12, 11, 15, 11 };

	if (dcvs_id > 5)
		return -EINVAL;

	return rtpcs_sds_write_bits(sds, PAGE_ANA_10G, reg[dcvs_id], bit[dcvs_id],
				    bit[dcvs_id], enable ? 0x0 : 0x1);
}

static int rtpcs_930x_sds_rxeq_dcvs_set_coef(struct rtpcs_serdes *sds, unsigned int dcvs_id,
					     int dcvs_coef)
{
	u8 reg[6] = { 0x1c, 0x1d, 0x1d, 0x1d, 0x02, 0x11 };
	u8 lbit[6] = { 0, 11, 6, 1, 6, 0 };
	if (dcvs_id > 5)
		return -EINVAL;

	return rtpcs_sds_write_bits(sds, PAGE_ANA_10G, reg[dcvs_id], lbit[dcvs_id] + 4,
				    lbit[dcvs_id], rtpcs_sign_mag_encode(dcvs_coef, 4));
}

__maybe_unused
static int rtpcs_930x_sds_rxeq_dcvs_get_coef(struct rtpcs_serdes *sds, unsigned int dcvs_id,
					     int *dcvs_coef)
{
	u8 manual_reg[6] = { 0x1e, 0x1e, 0x1e, 0x1e, 0x01, 0x02 };
	u8 coeff_sel[6] = { 0x22, 0x23, 0x24, 0x25, 0x2c, 0x2d };
	u8 manual_bit[6] = { 14, 13, 12, 11, 15, 11 };
	int ret, val;

	if (dcvs_id > 5)
		return -EINVAL;

	ret = rtpcs_930x_sds_set_debug(sds, 0x20);
	if (ret < 0)
		return ret;

	ret = rtpcs_sds_write_bits(sds, PAGE_ANA_10G_EXT, 0x0c, 5, 0, coeff_sel[dcvs_id]);
	if (ret < 0)
		return ret;
	usleep_range(1000, 2000);

	/* ## DCVSX Read Out */
	val = rtpcs_sds_read_bits(sds, PAGE_WDIG, 0x14, 4, 0);
	if (val < 0)
		return val;

	*dcvs_coef = rtpcs_sign_mag_decode(val, 4);
	val = rtpcs_sds_read_bits(sds, PAGE_ANA_10G, manual_reg[dcvs_id], manual_bit[dcvs_id],
				  manual_bit[dcvs_id]);
	if (val < 0)
		return val;

	dev_dbg(sds->ctrl->dev, "SerDes %u: DCVS %u, manual = %u, coefficient = %d\n",
		sds->id, dcvs_id, val, *dcvs_coef);

	return 0;
}

static int rtpcs_930x_sds_rxeq_leq_set_adapt(struct rtpcs_serdes *sds, bool enable)
{
	int ret;

	ret = rtpcs_sds_write_bits(sds, PAGE_ANA_10G, 0x18, 15, 15, enable ? 0x0 : 0x1);
	if (!ret && enable)
		msleep(100);

	return ret;
}

static int rtpcs_930x_sds_rxeq_leq_set_coef(struct rtpcs_serdes *sds, unsigned int leq_gray,
					    unsigned int offset)
{
	int ret;

	ret = rtpcs_sds_write_bits(sds, PAGE_ANA_10G, 0x17, 6, 2, offset);
	if (ret < 0)
		return ret;

	return rtpcs_sds_write_bits(sds, PAGE_ANA_10G, 0x16, 14, 10, leq_gray);
}

static int rtpcs_930x_sds_rxeq_leq_get_coef(struct rtpcs_serdes *sds)
{
	int bin, gray, manual, ret;

	ret = rtpcs_930x_sds_set_debug(sds, 0x10);
	if (ret < 0)
		return ret;
	usleep_range(1000, 2000);

	/* ##LEQ Read Out */
	gray = rtpcs_sds_read_bits(sds, PAGE_WDIG, 0x14, 7, 3);
	if (gray < 0)
		return gray;

	bin = rtpcs_gray_to_binary(gray);

	manual = rtpcs_sds_read_bits(sds, PAGE_ANA_10G, 0x18, 15, 15);
	if (manual < 0)
		return manual;

	dev_dbg(sds->ctrl->dev, "SerDes %u: LEQ gray: %d, LEQ bin: %d, LEQ manual: %u\n",
		sds->id, gray, bin, manual);
	return bin;
}

static int rtpcs_930x_sds_rxeq_vth_set_adapt(struct rtpcs_serdes *sds, bool enable)
{
	return rtpcs_sds_write_bits(sds, PAGE_ANA_10G, 0x0f, 13, 13, enable ? 0 : 1);
}

static int rtpcs_930x_sds_rxeq_vth_set_value(struct rtpcs_serdes *sds, unsigned int vth_p,
					     unsigned int vth_n)
{
	int ret;

	ret = rtpcs_sds_write_bits(sds, PAGE_ANA_10G, 0x13,  5,  3, vth_p);
	if (ret < 0)
		return ret;

	ret = rtpcs_sds_write_bits(sds, PAGE_ANA_10G, 0x13,  2,  0, vth_n);
	if (ret < 0)
		return ret;

	return 0;
}

static int rtpcs_930x_sds_rxeq_vth_get(struct rtpcs_serdes *sds, unsigned int *vth_p,
				       unsigned int *vth_n)
{
	int manual, ret, val;

	ret = rtpcs_930x_sds_set_debug(sds, 0x20);
	if (ret < 0)
		return ret;

	ret = rtpcs_sds_write_bits(sds, PAGE_ANA_10G_EXT, 0x0c, 5, 0, 0xc); /* COEF_SEL */
	if (ret < 0)
		return ret;
	usleep_range(1000, 2000);

	/* ##VthP & VthN Read Out */
	val = rtpcs_sds_read_bits(sds, PAGE_WDIG, 0x14, 5, 0);
	if (val < 0)
		return val;

	*vth_p = FIELD_GET(GENMASK(2, 0), val);
	*vth_n = FIELD_GET(GENMASK(5, 3), val);
	manual = rtpcs_sds_read_bits(sds, PAGE_ANA_10G, 0x0f, 13, 13);

	dev_dbg(sds->ctrl->dev, "SerDes %u: vth_p = %d, vth_n = %d, manual = %d\n",
		sds->id, *vth_p, *vth_n, manual);
	return 0;
}

static int rtpcs_930x_sds_rxeq_tap_set_adapt(struct rtpcs_serdes *sds, unsigned int tap_id,
					     bool enable)
{
	if (tap_id > 4)
		return -EINVAL;

	/* ##LOAD_IN_INIT[0], [11:7] = TAP0-TAP4 */
	return rtpcs_sds_write_bits(sds, PAGE_ANA_10G, 0xf, tap_id + 7, tap_id + 7,
				    enable ? 0x0 : 0x1);
}

static int rtpcs_930x_sds_rxeq_tap_set_value(struct rtpcs_serdes *sds, unsigned int tap_id,
					     int tap_even, int tap_odd)
{
	int ret = 0;

	if (tap_id > 4)
		return -EINVAL;

	switch (tap_id) {
	case 0:
		ret = rtpcs_sds_write_bits(sds, PAGE_ANA_10G_EXT, 0x03, 5, 0,
					   rtpcs_sign_mag_encode(tap_even, 5));
		break;
	case 1:
		ret = rtpcs_sds_write_bits(sds, PAGE_ANA_COM, 0x07, 6, 5,
					   (tap_even < 0) << 1 | (tap_odd < 0));
		if (!ret)
			ret = rtpcs_sds_write_bits(sds, PAGE_ANA_10G, 0x09, 11, 6,
						   abs(tap_even) & GENMASK(4, 0));
		if (!ret)
			ret = rtpcs_sds_write_bits(sds, PAGE_ANA_10G_EXT, 0x12, 5, 0,
						   abs(tap_odd) & GENMASK(4, 0));
		break;
	case 2:
		ret = rtpcs_sds_write_bits(sds, PAGE_ANA_10G, 0x09, 5, 0,
					   rtpcs_sign_mag_encode(tap_even, 5));
		if (!ret)
			ret = rtpcs_sds_write_bits(sds, PAGE_ANA_10G, 0x0a, 11, 6,
						   rtpcs_sign_mag_encode(tap_odd, 5));
		break;
	case 3:
		ret = rtpcs_sds_write_bits(sds, PAGE_ANA_10G, 0x0a, 5, 0,
					   rtpcs_sign_mag_encode(tap_even, 5));
		if (!ret)
			ret = rtpcs_sds_write_bits(sds, PAGE_ANA_10G, 0x06, 5, 0,
						   rtpcs_sign_mag_encode(tap_odd, 5));
		break;
	case 4:
		ret = rtpcs_sds_write_bits(sds, PAGE_ANA_10G_EXT, 0x01, 5, 0,
					   rtpcs_sign_mag_encode(tap_even, 5));
		if (!ret)
			ret = rtpcs_sds_write_bits(sds, PAGE_ANA_10G, 0x06, 11, 6,
						   rtpcs_sign_mag_encode(tap_odd, 5));
		break;
	default:
		break;
	}

	return ret;
}

static int rtpcs_930x_sds_rxeq_tap_get(struct rtpcs_serdes *sds, unsigned int tap_id,
				       int *tap_even, int *tap_odd)
{
	struct device *dev = sds->ctrl->dev;
	int ret, val;

	ret = rtpcs_930x_sds_set_debug(sds, 0x20);
	if (ret < 0)
		return ret;

	ret = rtpcs_sds_write_bits(sds, PAGE_ANA_10G_EXT, 0x0c, 5, 0, tap_id); /* COEF_SEL */
	if (ret < 0)
		return ret;
	usleep_range(1000, 2000);

	val = rtpcs_sds_read_bits(sds, PAGE_WDIG, 0x14, 5, 0);
	if (val < 0)
		return val;

	*tap_even = rtpcs_sign_mag_decode(val, 5);
	dev_dbg(dev, "SerDes %u: tap%d even coefficient = %d\n", sds->id, tap_id, *tap_even);

	if (tap_id > 0) {
		/* COEF_SEL */
		ret = rtpcs_sds_write_bits(sds, PAGE_ANA_10G_EXT, 0x0c, 5, 0, (tap_id + 5));
		if (ret < 0)
			return ret;

		val = rtpcs_sds_read_bits(sds, PAGE_WDIG, 0x14, 5, 0);
		if (val < 0)
			return val;

		*tap_odd = rtpcs_sign_mag_decode(val, 5);
		dev_dbg(dev, "SerDes %u: tap%u odd coefficient = %d\n", sds->id, tap_id, *tap_odd);
	}

	val = rtpcs_sds_read_bits(sds, PAGE_ANA_10G, 0x0f, tap_id + 7, tap_id + 7);
	if (val < 0)
		return val;

	dev_dbg(dev, "SerDes %u: tap%u manual = %d\n", sds->id, tap_id, val);
	return 0;
}

static void rtpcs_930x_sds_rxcal_init(struct rtpcs_serdes *sds, enum rtpcs_sds_mode hw_mode)
{
	int tap0_init_val = 0x1f; /* initial DFE TAP0 */
	int vth_min = 0x1;

	/* Clear some seeds and bits */
	rtpcs_sds_write(sds, PAGE_TGR_PRO_0, 0, 0);

	/* FGCAL */
	rtpcs_sds_write_bits(sds, PAGE_ANA_10G, 0x01, 14, 14, 0x00);
	rtpcs_sds_write_bits(sds, PAGE_ANA_10G, 0x1c, 10, 5, 0x20); /* offset_ini */
	rtpcs_sds_write_bits(sds, PAGE_ANA_10G_EXT, 0x02, 0, 0, 0x01); /* z0_ok */

	/* DCVS */
	for (int i = 0; i <= 5; i++) {
		rtpcs_930x_sds_rxeq_dcvs_set_coef(sds, i, 0);
		rtpcs_930x_sds_rxeq_dcvs_set_adapt(sds, i, true);
	}

	rtpcs_sds_write_bits(sds, PAGE_ANA_10G_EXT, 0x00, 3, 0, 0x0f); /* z0_ok_X */
	rtpcs_sds_write_bits(sds, PAGE_ANA_10G, 0x04, 7, 6, 0x03);

	/* LEQ (Linear Equalization) */
	rtpcs_sds_write_bits(sds, PAGE_ANA_10G, 0x16, 14, 8, 0x00); /* FILTER_OUT */

	/* DFE (Decision Feedback Equalizer) TAPs */
	rtpcs_930x_sds_rxeq_tap_set_value(sds, 0, tap0_init_val, 0);
	rtpcs_930x_sds_rxeq_tap_set_value(sds, 1, 0, 0);
	rtpcs_930x_sds_rxeq_tap_set_value(sds, 2, 0, 0);
	rtpcs_930x_sds_rxeq_tap_set_value(sds, 3, 0, 0);
	rtpcs_930x_sds_rxeq_tap_set_value(sds, 4, 0, 0);

	/* VTH (Voltage Threshold) */
	rtpcs_930x_sds_rxeq_vth_set_value(sds, 0x07, 0x07);
	rtpcs_sds_write_bits(sds, PAGE_ANA_10G_EXT, 0x0b, 5, 3, vth_min);

	/* load DFE initial value */
	rtpcs_sds_write_bits(sds, PAGE_ANA_10G, 0x0f, 13, 7, 0x7f); /* load_in_init */

	/* disable LEQ training, enable DFE clock */
	rtpcs_sds_write_bits(sds, PAGE_ANA_10G, 0x17,  7,  2, 0x00); /* [7] = EQHOLD, [6:2] = EQOUT */
	rtpcs_sds_write_bits(sds, PAGE_ANA_10G, 0x0c,  8,  8, 0x00); /* MAXHOLD_EN */
	rtpcs_sds_write_bits(sds, PAGE_ANA_10G, 0x0b,  4,  4, 0x01); /* dfe_adapt_eqen */
	rtpcs_sds_write_bits(sds, PAGE_ANA_10G, 0x12, 14, 14, 0x00); /* start_timer_en */
	rtpcs_sds_write_bits(sds, PAGE_ANA_10G_EXT, 0x02, 15, 15, 0x00); /* hold_timer_en */

	/* offset cali setting */
	rtpcs_sds_write_bits(sds, PAGE_ANA_10G, 0x0f, 15, 14, 0x03); /* cali_en */

	/* LEQ and DFE setting */

	/* assume this is equivalent with (PHY_TYPE == SERDES && MEDIA == FIBER_10G) for now */
	if (hw_mode == RTPCS_SDS_MODE_10GBASER) {
		rtpcs_sds_write_bits(sds, PAGE_ANA_10G, 0x03, 13, 8, 0x1f);
		rtpcs_sds_write_bits(sds, PAGE_ANA_10G, 0x00, 13, 13, 0x01);
		rtpcs_sds_write_bits(sds, PAGE_ANA_10G, 0x16, 14, 8, 0x00); /* FILTER_OUT */
	}

	/* REG0_LEQ_DC_GAIN, 0x01 for short DACs */
	rtpcs_sds_write_bits(sds, PAGE_ANA_10G, 0x16, 3, 2, 0x02); /* LEQ_DC_GAIN */
	rtpcs_sds_write_bits(sds, PAGE_ANA_10G, 0x0f, 6, 0, 0x5f); /* dfe_adapt_en */
	rtpcs_sds_write_bits(sds, PAGE_ANA_10G_EXT, 0x05, 7, 2, 0x1f); /* dfe_adapt_en2 */
	rtpcs_sds_write_bits(sds, PAGE_ANA_10G, 0x19, 9, 5, 0x1f); /* leq_min */
	rtpcs_sds_write_bits(sds, PAGE_ANA_10G_EXT, 0x0b, 15, 9, 0x3c); /* gray_en */
	rtpcs_sds_write_bits(sds, PAGE_ANA_10G, 0x0b, 1, 0, 0x03); /* dfe_adapt_mode */
}

static void rtpcs_930x_sds_rxcal_fgcal(struct rtpcs_serdes *sds)
{
	u32 fgcal_binary, fgcal_gray;
	u32 offset_range;

	rtpcs_930x_sds_rx_reset(sds, RTPCS_SDS_MODE_10GBASER);

	/* ForegroundOffsetCal_Manual */

	/* Gray config endis to 1 */
	rtpcs_sds_write_bits(sds, PAGE_ANA_10G_EXT, 0x02, 2, 2, 0x01);

	/* ForegroundOffsetCal_Manual(auto mode) */
	rtpcs_sds_write_bits(sds, PAGE_ANA_10G, 0x01, 14, 14, 0x00);

	/* Force Rx-Run = 0 */
	rtpcs_sds_write_bits(sds, PAGE_ANA_10G, 0x15, 8, 8, 0x0);
	rtpcs_930x_sds_rx_reset(sds, RTPCS_SDS_MODE_10GBASER);

	/* Foreground Calibration --- */

	for (int run = 0; run < 10; run++) {
		rtpcs_930x_sds_set_debug(sds, 0x20);
		rtpcs_sds_write_bits(sds, PAGE_ANA_10G_EXT, 0x0c, 5, 0, 0xf); /* COEF_SEL */
		/* ##FGCAL read gray */
		fgcal_gray = rtpcs_sds_read_bits(sds, PAGE_WDIG, 0x14, 5, 0);
		rtpcs_sds_write_bits(sds, PAGE_ANA_10G_EXT, 0x0c, 5, 0, 0xe); /* COEF_SEL */
		/* ##FGCAL read binary */
		fgcal_binary = rtpcs_sds_read_bits(sds, PAGE_WDIG, 0x14, 5, 0);

		if (fgcal_binary <= 60 && fgcal_binary >= 3)
			break;

		dev_dbg(sds->ctrl->dev, "SerDes %u: fgcal_gray = %d, fgcal_binary = %d\n",
			sds->id, fgcal_gray, fgcal_binary);

		offset_range = rtpcs_sds_read_bits(sds, PAGE_ANA_10G, 0x15, 15, 14);
		if (offset_range == 3) {
			dev_dbg(sds->ctrl->dev,
				"SerDes %u: Foreground Calibration result marginal!\n", sds->id);
			break;
		}

		offset_range++;
		rtpcs_sds_write_bits(sds, PAGE_ANA_10G, 0x15, 15, 14, offset_range);

		/* Force Rx-Run = 0 */
		rtpcs_sds_write_bits(sds, PAGE_ANA_10G, 0x15, 8, 8, 0x0);
		rtpcs_930x_sds_rx_reset(sds, RTPCS_SDS_MODE_10GBASER);
	}
}

__always_unused
static void rtpcs_930x_sds_rxcal_leq_adapt_lock(struct rtpcs_serdes *sds)
{
	/*
	 * SDK dacLongCableOffset / eqHoldEnable from rtl9300_rxCaliConf_serdes/phy_myParam.
	 * These distinguish direct SerDes connections (DAC, fiber SFP — no external PHY in
	 * the signal path) from PHY-attached ports (PCB traces to an external PHY). On
	 * PHY-attached ports the PHY handles its own equalization, so the SerDes LEQ is left
	 * in auto-adapt and no correction offset is needed.
	 */
	bool direct_serdes = sds->attachment == RTPCS_SDS_ATTACH_FIBER ||
			     sds->attachment == RTPCS_SDS_ATTACH_DAC_SHORT ||
			     sds->attachment == RTPCS_SDS_ATTACH_DAC_LONG;
	u32 sum10 = 0, avg10;
	int i, val;

	/* 1.3.1: release LEQ auto-adapt, let it settle from zero */
	if (!direct_serdes)
		rtpcs_sds_write_bits(sds, PAGE_ANA_10G, 0xc, 8, 8, 0x0);
	rtpcs_sds_write_bits(sds, PAGE_ANA_10G, 0x17, 7, 7, 0x0);
	rtpcs_930x_sds_rxeq_leq_set_adapt(sds, true);

	/* 1.3.2: sample the auto-adapted LEQ value 10 times over ~100ms */
	for (i = 0; i < 10; i++) {
		val = rtpcs_930x_sds_rxeq_leq_get_coef(sds);
		if (val < 0)
			return;

		sum10 += val;
		usleep_range(10000, 11000);
	}

	/* rounded average of where auto-adapt settled */
	avg10 = (sum10 / 10) + (((sum10 % 10) >= 5) ? 1 : 0);

	/*
	 * Empirical correction based on attachment type.
	 * Direct SerDes connections get a base offset of +3; DAC cables add further
	 * correction for their attenuation. PHY-attached needs none.
	 */
	switch (sds->attachment) {
	case RTPCS_SDS_ATTACH_FIBER:
		avg10 += 3;
		break;
	case RTPCS_SDS_ATTACH_DAC_SHORT:
		avg10 += 4;	/* base 3 + 1 for short DAC */
		break;
	case RTPCS_SDS_ATTACH_DAC_LONG:
		avg10 += 6;	/* base 3 + 3 for long DAC */
		break;
	default:
		break;
	}

	dev_dbg(sds->ctrl->dev, "SerDes %u: LEQ adapt sum10=%u, avg10=%u\n", sds->id,
		sum10, avg10);

	/* lock LEQ at corrected value for direct SerDes; PHY-attached stays in auto-adapt */
	if (direct_serdes) {
		rtpcs_sds_write_bits(sds, PAGE_ANA_10G, 0x17, 7, 7, 0x1);
		rtpcs_930x_sds_rxeq_leq_set_adapt(sds, false);
		rtpcs_930x_sds_rxeq_leq_set_coef(sds, avg10, 0);
	}

	dev_dbg(sds->ctrl->dev, "SerDes %u: LEQ = %u\n", sds->id,
		rtpcs_930x_sds_rxeq_leq_get_coef(sds));
}

static void rtpcs_930x_sds_rxcal_vth_tap0_adapt_lock(struct rtpcs_serdes *sds)
{
	unsigned int vth_p, vth_n;
	int tap0;

	/* run VTH/TAP auto-adapt */
	rtpcs_930x_sds_rxeq_vth_set_adapt(sds, true);
	rtpcs_930x_sds_rxeq_tap_set_adapt(sds, 0, true);
	msleep(200);

	/* manually set learned VTH */
	if (rtpcs_930x_sds_rxeq_vth_get(sds, &vth_p, &vth_n) < 0)
		return;
	rtpcs_930x_sds_rxeq_vth_set_value(sds, vth_p, vth_n);
	rtpcs_930x_sds_rxeq_vth_set_adapt(sds, false);

	msleep(100);

	/* manually set learned TAP0 */
	if (rtpcs_930x_sds_rxeq_tap_get(sds, 0, &tap0, NULL) < 0)
		return;
	rtpcs_930x_sds_rxeq_tap_set_value(sds, 0, tap0, 0);
	rtpcs_930x_sds_rxeq_tap_set_adapt(sds, 0, false);
}

static void rtpcs_930x_sds_rxeq_dfe_taps_adapt(struct rtpcs_serdes *sds)
{
	/* dfeTap1_4Enable true */
	rtpcs_930x_sds_rxeq_tap_set_adapt(sds, 1, true);
	rtpcs_930x_sds_rxeq_tap_set_adapt(sds, 2, true);
	rtpcs_930x_sds_rxeq_tap_set_adapt(sds, 3, true);
	rtpcs_930x_sds_rxeq_tap_set_adapt(sds, 4, true);

	msleep(30);
}

static void rtpcs_930x_sds_rxeq_dfe_disable(struct rtpcs_serdes *sds)
{
	int tap_even = 0, tap_odd = 0;

	for (int i = 1; i <= 4; i++) {
		rtpcs_930x_sds_rxeq_tap_set_value(sds, i, tap_even, tap_odd);
		rtpcs_930x_sds_rxeq_tap_set_adapt(sds, i, false);
	}

	usleep_range(10000, 11000);
}

static void rtpcs_930x_sds_do_rx_calibration(struct rtpcs_serdes *sds,
					     enum rtpcs_sds_mode hw_mode)
{
	u32 latch_sts;

	rtpcs_930x_sds_rxcal_init(sds, hw_mode);
	rtpcs_930x_sds_rxcal_fgcal(sds);
	rtpcs_930x_sds_rxcal_vth_tap0_adapt_lock(sds);

	/* Do this only for 10GR mode */
	if (hw_mode == RTPCS_SDS_MODE_10GBASER) {
		rtpcs_930x_sds_rxeq_dfe_taps_adapt(sds);
		msleep(20);

		latch_sts = rtpcs_sds_read_bits(sds, PAGE_TGR_STD_0, 1, 2, 2);
		usleep_range(1000, 2000);
		latch_sts = rtpcs_sds_read_bits(sds, PAGE_TGR_STD_0, 1, 2, 2);
		if (latch_sts) {
			rtpcs_930x_sds_rxeq_dfe_disable(sds);
			rtpcs_930x_sds_rxcal_vth_tap0_adapt_lock(sds);
			rtpcs_930x_sds_rxeq_dfe_taps_adapt(sds);
		}
	}
}

static int rtpcs_930x_sds_sym_err_reset(struct rtpcs_serdes *sds,
					enum rtpcs_sds_mode hw_mode)
{
	int channel, channels;

	switch (hw_mode) {
	case RTPCS_SDS_MODE_USXGMII:
		if (sds->usxgmii_submode != RTPCS_SDS_USXGMII_SM_10GSXGMII) {
			channels = 1;
			break;
		}
		fallthrough;
	case RTPCS_SDS_MODE_10GBASER:
		/* Read twice to clear */
		rtpcs_sds_read(sds, PAGE_TGR_STD_1, 0x1);
		rtpcs_sds_read(sds, PAGE_TGR_STD_1, 0x1);
		return 0;

	case RTPCS_SDS_MODE_XSGMII:
	case RTPCS_SDS_MODE_QSGMII:
		channels = 4;
		break;

	default:
		channels = 1;
		break;
	}

	for (channel = 0; channel < channels; channel++) {
		if (hw_mode == RTPCS_SDS_MODE_XSGMII) {
			rtpcs_sds_xsg_write_bits(sds, PAGE_SDS_EXT, 0x18, 2, 0, channel);
			rtpcs_sds_xsg_write_bits(sds, PAGE_SDS_EXT, 0x3, 15, 8, 0x0);
			rtpcs_sds_xsg_write_bits(sds, PAGE_SDS_EXT, 0x2, 15, 0, 0x0);
		} else {
			rtpcs_sds_write_bits(sds, PAGE_SDS_EXT, 0x18, 2, 0, channel);
			rtpcs_sds_write_bits(sds, PAGE_SDS_EXT, 0x3, 15, 8, 0x0);
			rtpcs_sds_write_bits(sds, PAGE_SDS_EXT, 0x2, 15, 0, 0x0);
		}
	}

	if (channels > 1) {
		if (hw_mode == RTPCS_SDS_MODE_XSGMII) {
			rtpcs_sds_xsg_write_bits(sds, PAGE_SDS_EXT, 0x0, 15, 0, 0x0);
			rtpcs_sds_xsg_write_bits(sds, PAGE_SDS_EXT, 0x1, 15, 8, 0x0);
		} else {
			rtpcs_sds_write_bits(sds, PAGE_SDS_EXT, 0x0, 15, 0, 0x0);
			rtpcs_sds_write_bits(sds, PAGE_SDS_EXT, 0x1, 15, 8, 0x0);
		}
	}

	return 0;
}

static u32 rtpcs_930x_sds_sym_err_get(struct rtpcs_serdes *sds,
				      enum rtpcs_sds_mode hw_mode)
{
	u32 v = 0;

	if (hw_mode == RTPCS_SDS_MODE_QSGMII || hw_mode == RTPCS_SDS_MODE_XSGMII) {
		v = rtpcs_sds_read_bits(sds, PAGE_SDS_EXT, 0x1, 15, 8) << 16; /* ALL_SYMBOLERR_CNT_NEW_23_16 */
		v |= rtpcs_sds_read_bits(sds, PAGE_SDS_EXT, 0x0, 15, 0); /* ALL_SYMBOLERR_CNT_NEW_15_0 */
	} else if (hw_mode == RTPCS_SDS_MODE_USXGMII &&
		   sds->usxgmii_submode == RTPCS_SDS_USXGMII_SM_10GQXGMII) {
		/* no known symbol error count for USXGMII QXGMII */
	} else if (hw_mode == RTPCS_SDS_MODE_1000BASEX || hw_mode == RTPCS_SDS_MODE_SGMII ||
		   hw_mode == RTPCS_SDS_MODE_10GBASER ||
		   (hw_mode == RTPCS_SDS_MODE_USXGMII &&
		    sds->usxgmii_submode == RTPCS_SDS_USXGMII_SM_10GSXGMII)) {
		v = rtpcs_sds_read(sds, PAGE_TGR_STD_1, 0x1);
		v &= 0xff;
	} else {
		rtpcs_sds_write_bits(sds, PAGE_SDS_EXT, 24, 2, 0, 0);

		v = rtpcs_sds_read_bits(sds, PAGE_SDS_EXT, 0x3, 15, 8) << 16; /* MUX_SYMBOLERR_CNT_NEW_23_16 */
		v |= rtpcs_sds_read_bits(sds, PAGE_SDS_EXT, 0x2, 15, 0); /* MUX_SYMBOLERR_CNT_NEW_15_0 */
	}

	return v;
}

static int rtpcs_930x_sds_check_calibration(struct rtpcs_serdes *sds,
					    enum rtpcs_sds_mode hw_mode)
{
	u32 errors1, errors2;

	rtpcs_930x_sds_sym_err_reset(sds, hw_mode);
	rtpcs_930x_sds_sym_err_reset(sds, hw_mode);

	/* Count errors during 1ms */
	errors1 = rtpcs_930x_sds_sym_err_get(sds, hw_mode);
	usleep_range(1000, 2000);
	errors2 = rtpcs_930x_sds_sym_err_get(sds, hw_mode);

	switch (hw_mode) {
	case RTPCS_SDS_MODE_XSGMII:
		if ((errors2 - errors1 > 100) || (errors1 >= 0xffff00) || (errors2 >= 0xffff00)) {
			dev_err(sds->ctrl->dev, "SerDes %u: XSGMII error rate too high\n",
				sds->id);
			return 1;
		}
		break;
	default:
		if (errors2 > 0) {
			dev_err(sds->ctrl->dev, "SerDes %u: symbol error rate too high\n",
				sds->id);
			return 1;
		}
		break;
	}

	return 0;
}

static int rtpcs_930x_sds_10g_idle(struct rtpcs_serdes *sds)
{
	struct rtpcs_serdes *even_sds = rtpcs_sds_get_even(sds);
	ktime_t timeout;
	int bit, busy;

	bit = (sds == even_sds) ? 0 : 1;
	timeout = ktime_add_us(ktime_get(), 10000); /* timeout after 10 msecs */

	do {
		rtpcs_sds_write(even_sds, PAGE_WDIG, 0x2, 53);
		busy = rtpcs_sds_read_bits(even_sds, PAGE_WDIG, 0x14, bit, bit);
		if (busy < 0)
			return busy;

		if (!busy)
			return 0;

		usleep_range(100, 200); /* wait ~100 usecs before retry */
	} while (ktime_before(ktime_get(), timeout));

	dev_warn(sds->ctrl->dev, "SerDes %u: waiting for RX idle timed out\n", sds->id);
	return -ETIMEDOUT;
}

static int rtpcs_930x_sds_config_polarity(struct rtpcs_serdes *sds, unsigned int tx_pol,
					  unsigned int rx_pol)
{
	u8 rx_val = (rx_pol == PHY_POL_INVERT) ? 1 : 0;
	u8 tx_val = (tx_pol == PHY_POL_INVERT) ? 1 : 0;
	u32 val;
	int ret;

	/* 10GR */
	val = (tx_val << 1) | rx_val;
	ret = rtpcs_sds_write_bits(sds, PAGE_TGR_PRO_0, 0x2, 14, 13, val);
	if (ret)
		return ret;

	/* 1G */
	val = (rx_val << 1) | tx_val;
	return rtpcs_sds_write_bits(sds, PAGE_SDS, 0x0, 9, 8, val);
}

static const struct rtpcs_sds_config rtpcs_930x_sds_cfg_ana_com[] = {
	{PAGE_ANA_COM, 0x03, 0x8206}, {PAGE_ANA_COM, 0x05, 0x40B0},
	{PAGE_ANA_COM, 0x06, 0x0010}, {PAGE_ANA_COM, 0x07, 0xF09F},
	{PAGE_ANA_COM, 0x0A, 0x0003}, {PAGE_ANA_COM, 0x0B, 0x0005},
	{PAGE_ANA_COM, 0x0C, 0x0007}, {PAGE_ANA_COM, 0x0D, 0x6009},
	{PAGE_ANA_COM, 0x0E, 0x0000}, {PAGE_ANA_COM, 0x0F, 0x0008}
};

static const struct rtpcs_sds_config rtpcs_930x_sds_cfg_ana_1g[] = {
	{PAGE_ANA_1G2, 0x00, 0x0668}, {PAGE_ANA_1G2, 0x02, 0xD020},
	{PAGE_ANA_1G2, 0x06, 0xC000}, {PAGE_ANA_1G2, 0x0B, 0x1892},
	{PAGE_ANA_1G2, 0x0F, 0xFFDF}, {PAGE_ANA_1G2, 0x12, 0x03C4},
	{PAGE_ANA_1G2, 0x13, 0x027F}, {PAGE_ANA_1G2, 0x14, 0x1311},
	{PAGE_ANA_1G2, 0x16, 0x00C9}, {PAGE_ANA_1G2, 0x17, 0xA100},
	{PAGE_ANA_1G2, 0x1A, 0x0001}, {PAGE_ANA_1G2, 0x1C, 0x0400},
	{PAGE_ANA_1G2_EXT, 0x00, 0x820F}, {PAGE_ANA_1G2_EXT, 0x01, 0x0300},
	{PAGE_ANA_1G2_EXT, 0x02, 0x1017}, {PAGE_ANA_1G2_EXT, 0x03, 0xFFDF},
	{PAGE_ANA_1G2_EXT, 0x05, 0x7F7C}, {PAGE_ANA_1G2_EXT, 0x07, 0x8100},
	{PAGE_ANA_1G2_EXT, 0x08, 0x0001}, {PAGE_ANA_1G2_EXT, 0x09, 0xFFD4},
	{PAGE_ANA_1G2_EXT, 0x0A, 0x7C2F}, {PAGE_ANA_1G2_EXT, 0x0E, 0x003F},
	{PAGE_ANA_1G2_EXT, 0x0F, 0x0121}, {PAGE_ANA_1G2_EXT, 0x10, 0x0020},
	{PAGE_ANA_1G2_EXT, 0x11, 0x8840}
};

static const struct rtpcs_sds_config rtpcs_930x_sds_cfg_ana_3g[] = {
	{PAGE_ANA_3G1, 0x00, 0x0668}, {PAGE_ANA_3G1, 0x02, 0xD020},
	{PAGE_ANA_3G1, 0x06, 0xC000}, {PAGE_ANA_3G1, 0x0B, 0x1892},
	{PAGE_ANA_3G1, 0x0F, 0xFFDF}, {PAGE_ANA_3G1, 0x12, 0x01C4},
	{PAGE_ANA_3G1, 0x13, 0x027F}, {PAGE_ANA_3G1, 0x14, 0x1311},
	{PAGE_ANA_3G1, 0x16, 0x00C9}, {PAGE_ANA_3G1, 0x17, 0xA100},
	{PAGE_ANA_3G1, 0x1A, 0x0001}, {PAGE_ANA_3G1, 0x1C, 0x0400},
	{PAGE_ANA_3G1_EXT, 0x00, 0x820F}, {PAGE_ANA_3G1_EXT, 0x01, 0x0300},
	{PAGE_ANA_3G1_EXT, 0x02, 0x1017}, {PAGE_ANA_3G1_EXT, 0x03, 0xFFDF},
	{PAGE_ANA_3G1_EXT, 0x05, 0x7F7C}, {PAGE_ANA_3G1_EXT, 0x07, 0x8100},
	{PAGE_ANA_3G1_EXT, 0x08, 0x0001}, {PAGE_ANA_3G1_EXT, 0x09, 0xFFD4},
	{PAGE_ANA_3G1_EXT, 0x0A, 0x7C2F}, {PAGE_ANA_3G1_EXT, 0x0E, 0x003F},
	{PAGE_ANA_3G1_EXT, 0x0F, 0x0121}, {PAGE_ANA_3G1_EXT, 0x10, 0x0020},
	{PAGE_ANA_3G1_EXT, 0x11, 0x8840},
};

static const struct rtpcs_sds_config rtpcs_930x_sds_cfg_ana_10g[] = {
	{PAGE_ANA_10G, 0x00, 0xA668}, {PAGE_ANA_10G, 0x01, 0x2088},
	{PAGE_ANA_10G, 0x02, 0xD020}, {PAGE_ANA_10G, 0x06, 0xC000},
	{PAGE_ANA_10G, 0x0B, 0x1892}, {PAGE_ANA_10G, 0x0F, 0xFFDF},
	{PAGE_ANA_10G, 0x11, 0x8280}, {PAGE_ANA_10G, 0x12, 0x0044},
	{PAGE_ANA_10G, 0x13, 0x027F}, {PAGE_ANA_10G, 0x14, 0x1311},
	{PAGE_ANA_10G, 0x17, 0xA100}, {PAGE_ANA_10G, 0x1A, 0x0001},
	{PAGE_ANA_10G, 0x1C, 0x0400},
	{PAGE_ANA_10G_EXT, 0x00, 0x820F}, {PAGE_ANA_10G_EXT, 0x01, 0x0300},
	{PAGE_ANA_10G_EXT, 0x02, 0x1217}, {PAGE_ANA_10G_EXT, 0x03, 0xFFDF},
	{PAGE_ANA_10G_EXT, 0x05, 0x7F7C}, {PAGE_ANA_10G_EXT, 0x07, 0x80C4},
	{PAGE_ANA_10G_EXT, 0x08, 0x0001}, {PAGE_ANA_10G_EXT, 0x09, 0xFFD4},
	{PAGE_ANA_10G_EXT, 0x0A, 0x7C2F}, {PAGE_ANA_10G_EXT, 0x0E, 0x003F},
	{PAGE_ANA_10G_EXT, 0x0F, 0x0121}, {PAGE_ANA_10G_EXT, 0x10, 0x0020},
	{PAGE_ANA_10G_EXT, 0x11, 0x8840},
};

static const struct rtpcs_sds_config rtpcs_930x_sds_cfg_usxgmii_xsgmii[] = {
	{PAGE_ANA_10G, 0x12, 0x0484},
	{PAGE_ANA_10G_EXT, 0x02, 0x1017}, {PAGE_ANA_10G_EXT, 0x07, 0x8104}
};

static const struct rtpcs_sds_config rtpcs_930x_sds_cfg_5g_qsgmii[] =
{
	{PAGE_ANA_COM, 0x00, 0x3C91}, {PAGE_ANA_COM, 0x02, 0xB602},
	{PAGE_ANA_COM, 0x07, 0xFA66}, {PAGE_ANA_COM, 0x0A, 0xDF40},
	{PAGE_ANA_5G0, 0x02, 0x35A1}, {PAGE_ANA_5G0, 0x03, 0x6960},
};

static const struct rtpcs_sds_config rtpcs_930x_sds_cfg_final_even[] =
{
	{PAGE_ANA_5G0_EXT, 0x13, 0x0050}, {PAGE_ANA_5G0_EXT, 0x18, 0x8E88},
	{PAGE_ANA_5G0_EXT, 0x19, 0x4902}, {PAGE_ANA_5G0_EXT, 0x1D, 0x2501},

	{PAGE_ANA_6G2_EXT, 0x13, 0x0050}, {PAGE_ANA_6G2_EXT, 0x17, 0x4109},
	{PAGE_ANA_6G2_EXT, 0x18, 0x8E88}, {PAGE_ANA_6G2_EXT, 0x19, 0x4902},
	{PAGE_ANA_6G2_EXT, 0x1C, 0x1109}, {PAGE_ANA_6G2_EXT, 0x1D, 0x2641},

	{PAGE_ANA_10G_EXT, 0x13, 0x0050}, {PAGE_ANA_10G_EXT, 0x18, 0x8E88},
	{PAGE_ANA_10G_EXT, 0x19, 0x4902}, {PAGE_ANA_10G_EXT, 0x1D, 0x66E1},
};

static const struct rtpcs_sds_config rtpcs_930x_sds_cfg_final_odd[] =
{
	{PAGE_ANA_5G0_EXT, 0x13, 0x3D87}, {PAGE_ANA_5G0_EXT, 0x14, 0x3108},
	{PAGE_ANA_6G2_EXT, 0x13, 0x3C87}, {PAGE_ANA_6G2_EXT, 0x14, 0x1808}
};

static int rtpcs_930x_sds_config_hw_mode(struct rtpcs_serdes *sds, enum rtpcs_sds_mode hw_mode)
{
	int (*apply_fn)(struct rtpcs_serdes *, const struct rtpcs_sds_config *, size_t);
	bool is_xsgmii = (hw_mode == RTPCS_SDS_MODE_XSGMII);
	bool is_even_sds = (sds == rtpcs_sds_get_even(sds));
	int ret;

	apply_fn = is_xsgmii ? rtpcs_sds_apply_config_xsg : rtpcs_sds_apply_config;

	if (hw_mode == RTPCS_SDS_MODE_QSGMII) {
		if (sds->type != RTPCS_SDS_TYPE_5G)
			return -ENOTSUPP;

		return rtpcs_sds_apply_config(sds, rtpcs_930x_sds_cfg_5g_qsgmii,
					      ARRAY_SIZE(rtpcs_930x_sds_cfg_5g_qsgmii));
	}

	if (hw_mode != RTPCS_SDS_MODE_USXGMII) {
		if (is_xsgmii)
			rtpcs_sds_xsg_write(sds, PAGE_SDS, 0x0E, 0x3053);
		else {
			rtpcs_sds_write(sds, PAGE_SDS, 0x0E, 0x3053);
			rtpcs_sds_write(sds, PAGE_SDS_EXT, 0x14, 0x0100);
		}
	}

	ret = apply_fn(sds, rtpcs_930x_sds_cfg_ana_com, ARRAY_SIZE(rtpcs_930x_sds_cfg_ana_com));
	if (ret < 0)
		return ret;

	switch (hw_mode) {
	case RTPCS_SDS_MODE_1000BASEX:
	case RTPCS_SDS_MODE_SGMII:
		ret = rtpcs_sds_apply_config(sds, rtpcs_930x_sds_cfg_ana_1g,
					     ARRAY_SIZE(rtpcs_930x_sds_cfg_ana_1g));
		if (ret < 0)
			return ret;

		break;

	case RTPCS_SDS_MODE_10GBASER:
		rtpcs_sds_write(sds, PAGE_TGR_PRO_0, 0x0D, 0x0F00);
		rtpcs_sds_write(sds, PAGE_TGR_PRO_0, 0x00, 0x0000);
		rtpcs_sds_write(sds, PAGE_TGR_PRO_0, 0x01, 0xC800);

		ret = rtpcs_sds_apply_config(sds, rtpcs_930x_sds_cfg_ana_10g,
					     ARRAY_SIZE(rtpcs_930x_sds_cfg_ana_10g));
		if (ret < 0)
			return ret;

		rtpcs_sds_write(sds, PAGE_ANA_10G_EXT, 0x14, 0xE008);
		break;

	case RTPCS_SDS_MODE_2500BASEX:
		ret = rtpcs_sds_apply_config(sds, rtpcs_930x_sds_cfg_ana_3g,
					     ARRAY_SIZE(rtpcs_930x_sds_cfg_ana_3g));
		if (ret < 0)
			return ret;

		break;

	case RTPCS_SDS_MODE_XSGMII:
	case RTPCS_SDS_MODE_USXGMII:
		ret = apply_fn(sds, rtpcs_930x_sds_cfg_ana_10g,
			       ARRAY_SIZE(rtpcs_930x_sds_cfg_ana_10g));
		if (ret < 0)
			return ret;

		ret = apply_fn(sds, rtpcs_930x_sds_cfg_usxgmii_xsgmii,
			       ARRAY_SIZE(rtpcs_930x_sds_cfg_usxgmii_xsgmii));
		if (ret < 0)
			return ret;

		if (!is_xsgmii)
			rtpcs_93xx_sds_usxgmii_config(sds);
		break;

	default:
		return 0;
	}

	if (is_even_sds)
		ret = apply_fn(sds, rtpcs_930x_sds_cfg_final_even,
			       ARRAY_SIZE(rtpcs_930x_sds_cfg_final_even));
	else
		ret = apply_fn(sds, rtpcs_930x_sds_cfg_final_odd,
			       ARRAY_SIZE(rtpcs_930x_sds_cfg_final_odd));

	if (ret < 0)
		return ret;

	if (hw_mode == RTPCS_SDS_MODE_10GBASER && is_even_sds)
		rtpcs_sds_write(sds, PAGE_ANA_10G_EXT, 0x1D, 0x76E1);

	return 0;
}

static int rtpcs_930x_sds_config_attachment(struct rtpcs_serdes *sds,
					    enum rtpcs_sds_attachment attachment,
					    enum rtpcs_sds_mode hw_mode)
{
	const struct rtpcs_sds_tx_config *tx_cfg;
	enum rtpcs_sds_pll_speed speed;
	int ret;

	if (sds->type != RTPCS_SDS_TYPE_10G)
		return 0;

	ret = rtpcs_sds_select_pll_speed(hw_mode, &speed);
	if (ret < 0)
		return ret;

	/*
	 * dal_longan_construct_mac_default_10gmedia_fiber: vendor sets this
	 * for any 10G-class port regardless of attachment, despite the
	 * name; 0 matches this register's hardware reset default.
	 */
	ret = rtpcs_sds_write_bits(sds, PAGE_WDIG, 11, 1, 1, speed == RTPCS_SDS_PLL_SPD_10000);
	if (ret < 0)
		return ret;

	switch (speed) {
	case RTPCS_SDS_PLL_SPD_1000:
		tx_cfg = &rtpcs_930x_sds_tx_config_1g;
		break;
	case RTPCS_SDS_PLL_SPD_2500:
		tx_cfg = &rtpcs_930x_sds_tx_config_2g5;
		break;
	case RTPCS_SDS_PLL_SPD_10000:
		/*
		 * PHY-attached links equalize on the PHY's own far-end
		 * receiver, less amp drive needed. Real DAC-vs-fiber
		 * detection doesn't exist yet, so both of those share
		 * the same config for now.
		 */
		tx_cfg = (attachment == RTPCS_SDS_ATTACH_PHY) ? &rtpcs_930x_sds_tx_config_phy
							      : &rtpcs_930x_sds_tx_config_10g;
		break;
	default:
		return -EOPNOTSUPP;
	}

	return rtpcs_930x_sds_tx_config(sds, hw_mode, tx_cfg);
}

static int rtpcs_930x_sds_post_config(struct rtpcs_serdes *sds, enum rtpcs_sds_mode hw_mode)
{
	int calib_tries = 0;

	if (hw_mode == RTPCS_SDS_MODE_QSGMII)
		return 0;

	rtpcs_930x_sds_10g_idle(sds);
	do {
		rtpcs_930x_sds_do_rx_calibration(sds, hw_mode);
		calib_tries++;
		msleep(50);
	} while (rtpcs_930x_sds_check_calibration(sds, hw_mode) && calib_tries < 3);
	if (calib_tries >= 3)
		dev_warn(sds->ctrl->dev, "SerDes %u: RX calibration failed\n", sds->id);

	return 0;
}

static int rtpcs_930x_sds_probe(struct rtpcs_serdes *sds)
{
	struct device *dev = sds->ctrl->dev;
	struct regmap *map = sds->ctrl->map;
	u8 id = sds->id;

	if (id < 2)
		sds->type = RTPCS_SDS_TYPE_5G;
	else if (id <= 9)
		sds->type = RTPCS_SDS_TYPE_10G;
	else
		sds->type = RTPCS_SDS_TYPE_UNKNOWN;

	rtpcs_93xx_sds_fill_caps(sds);

	sds->swcore_regs.mac_mode = devm_regmap_field_alloc(dev, map,
							    rtpcs_930x_mac_mode_fields[id]);
	if (IS_ERR(sds->swcore_regs.mac_mode))
		return PTR_ERR(sds->swcore_regs.mac_mode);

	/* submode only for 10G SerDes (id 2-9) */
	if (sds->type == RTPCS_SDS_TYPE_10G) {
		sds->swcore_regs.usxgmii_submode = devm_regmap_field_alloc(dev, map,
							rtpcs_930x_usxgmii_submode_fields[id - 2]);
		if (IS_ERR(sds->swcore_regs.usxgmii_submode))
			return PTR_ERR(sds->swcore_regs.usxgmii_submode);
	}

	return 0;
}

/* RTL931X */

/*
 * The SerDes MDIO driver maps page regions to different background SerDes.
 * 0x00 - 0x3f	analog SDS
 * 0x40 - 0x7f	digital SDS 1
 * 0x80 - 0xbf	digital SDS 2
 *
 * An XSG write operates on digital SDS 1 and digital SDS 2. Map that to the
 * page ranges accordingly.
 */
static int rtpcs_931x_sds_op_xsg_write(struct rtpcs_serdes *sds, enum rtpcs_page page, int regnum,
                                       int bithigh, int bitlow, u16 value)
{
        int ret;

        ret = __rtpcs_sds_write_raw(sds->ctrl, sds->id, DIGI_1(page), regnum, bithigh, bitlow,
				    value);
        if (ret)
                return ret;

        return __rtpcs_sds_write_raw(sds->ctrl, sds->id, DIGI_2(page), regnum, bithigh, bitlow,
				     value);
}

static int rtpcs_931x_sds_fiber_get_symerr(struct rtpcs_serdes *sds,
					   enum rtpcs_sds_mode hw_mode)
{
	int symerr, val, val2;

	switch (hw_mode) {
	case RTPCS_SDS_MODE_10GBASER:
		symerr = rtpcs_sds_read_bits(sds, PAGE_TGR_STD_1, 0x1, 7, 0);
		break;
	case RTPCS_SDS_MODE_1000BASEX:
		rtpcs_sds_write_bits(sds, DIGI_1(PAGE_SDS_EXT), 0x18, 2, 0, 0x0);

		val = rtpcs_sds_read_bits(sds, DIGI_1(PAGE_SDS_EXT), 0x3, 15, 8);
		val2 = rtpcs_sds_read_bits(sds, DIGI_1(PAGE_SDS_EXT), 0x2, 15, 0);
		symerr = (val << 16) | val2;
		break;
	default:
		symerr = -EINVAL;
	}

	return symerr;
}

static bool rtpcs_931x_sds_10gr_link_up(struct rtpcs_serdes *sds)
{
	return rtpcs_sds_read_bits(sds, PAGE_TGR_STD_1, 0x0, 12, 12) == 1;
}

static void rtpcs_931x_sds_clear_symerr(struct rtpcs_serdes *sds,
					enum rtpcs_sds_mode hw_mode)
{
	switch (hw_mode) {
	case RTPCS_SDS_MODE_SGMII:
	case RTPCS_SDS_MODE_XSGMII:
		for (int i = 0; i < 4; ++i) {
			rtpcs_sds_xsg_write_bits(sds, PAGE_SDS_EXT, 0x18, 2, 0, i);
			rtpcs_sds_xsg_write_bits(sds, PAGE_SDS_EXT, 0x3, 15, 8, 0x0);
			rtpcs_sds_xsg_write(sds, PAGE_SDS_EXT, 0x2, 0x0);
		}

		rtpcs_sds_xsg_write(sds, PAGE_SDS_EXT, 0x0, 0x0);
		rtpcs_sds_xsg_write_bits(sds, PAGE_SDS_EXT, 0x1, 15, 8, 0x0);
		break;
	case RTPCS_SDS_MODE_1000BASEX:
		rtpcs_sds_write_bits(sds, DIGI_1(PAGE_SDS_EXT), 0x18, 2, 0, 0x0);
		rtpcs_sds_write_bits(sds, DIGI_1(PAGE_SDS_EXT), 0x3, 15, 8, 0x0);
		rtpcs_sds_write_bits(sds, DIGI_1(PAGE_SDS_EXT), 0x2, 15, 0, 0x0);
		break;
	case RTPCS_SDS_MODE_10GBASER:
		/* to be verified: clear on read? */
		rtpcs_sds_read_bits(sds, PAGE_TGR_STD_1, 0x1, 7, 0);
		break;
	case RTPCS_SDS_MODE_OFF:
	default:
		break;
	}
}

/*
 * rtpcs_931x_sds_set_debug() - Route a coefficient's debug readback.
 *
 * Vendor SDK: _phy_rtl9310_dbg_set(). Selects which lane of the even/odd
 * pair feeds the shared WDIG debug readback register, then selects
 * dbg_sel within that lane. Must run before reading any rxeq_*_get().
 *
 * Note: This needs to be locked to avoid adjacent SerDes interfering with
 * 	 those settings and produce inconsistent results. Currently, this is
 * 	 achieved by the global PCS lock.
 */
static int rtpcs_931x_sds_set_debug(struct rtpcs_serdes *sds, unsigned int dbg_sel)
{
	struct rtpcs_serdes *even_sds = rtpcs_sds_get_even(sds);
	int ret;

	ret = rtpcs_sds_write(even_sds, PAGE_WDIG, 0x2, (sds == even_sds) ? 0x4b : 0x4c);
	if (ret < 0)
		return ret;

	ret = rtpcs_sds_write_bits(sds, PAGE_ANA_COM, 0x0, 2, 2, 0x1);
	if (ret < 0)
		return ret;

	return rtpcs_sds_write_bits(sds, PAGE_ANA_10G, 0x15, 11, 10, dbg_sel);
}

static int rtpcs_931x_sds_rxeq_leq_set_adapt(struct rtpcs_serdes *sds, bool enable)
{
	return rtpcs_sds_write_bits(sds, PAGE_ANA_10G, 0xd, 7, 7, enable ? 0x0 : 0x1);
}

static int rtpcs_931x_sds_rxeq_leq_set_coef(struct rtpcs_serdes *sds, unsigned int gain)
{
	return rtpcs_sds_write_bits(sds, PAGE_ANA_10G, 0xd, 6, 2, gain);
}

static int rtpcs_931x_sds_rxeq_leq_get_coef(struct rtpcs_serdes *sds)
{
	int ret, gray;

	ret = rtpcs_931x_sds_set_debug(sds, 0x1);
	if (ret < 0)
		return ret;

	gray = rtpcs_sds_read_bits(sds, PAGE_WDIG, 0x14, 7, 3);
	if (gray < 0)
		return gray;

	return rtpcs_gray_to_binary(gray);
}

static int rtpcs_931x_sds_rxeq_tap_set_value(struct rtpcs_serdes *sds, unsigned int tap_id,
					     int tap_even, int tap_odd)
{
	int ret;

	switch (tap_id) {
	case 0:
		return rtpcs_sds_write_bits(sds, PAGE_ANA_10G, 0x1c, 5, 0,
					    rtpcs_sign_mag_encode(tap_even, 5));
	case 1:
		ret = rtpcs_sds_write_bits(sds, PAGE_ANA_10G, 0x1d, 5, 0,
					   rtpcs_sign_mag_encode(tap_even, 5));
		if (!ret)
			ret = rtpcs_sds_write_bits(sds, PAGE_ANA_10G, 0x1d, 11, 6,
						   rtpcs_sign_mag_encode(tap_odd, 5));
		return ret;
	case 2:
		ret = rtpcs_sds_write_bits(sds, PAGE_ANA_10G, 0x1f, 5, 0,
					   rtpcs_sign_mag_encode(tap_even, 5));
		if (!ret)
			ret = rtpcs_sds_write_bits(sds, PAGE_ANA_10G, 0x1f, 11, 6,
						   rtpcs_sign_mag_encode(tap_odd, 5));
		return ret;
	case 3:
		ret = rtpcs_sds_write_bits(sds, PAGE_ANA_10G_EXT, 0x0, 5, 0,
					   rtpcs_sign_mag_encode(tap_even, 5));
		if (!ret)
			ret = rtpcs_sds_write_bits(sds, PAGE_ANA_10G_EXT, 0x0, 11, 6,
						   rtpcs_sign_mag_encode(tap_odd, 5));
		return ret;
	case 4:
		ret = rtpcs_sds_write_bits(sds, PAGE_ANA_10G_EXT, 0x1, 5, 0,
					   rtpcs_sign_mag_encode(tap_even, 5));
		if (!ret)
			ret = rtpcs_sds_write_bits(sds, PAGE_ANA_10G_EXT, 0x1, 11, 6,
						   rtpcs_sign_mag_encode(tap_odd, 5));
		return ret;
	default:
		return -EINVAL;
	}
}

static int rtpcs_931x_sds_rxeq_tap_set_adapt(struct rtpcs_serdes *sds, unsigned int tap_id,
					     bool enable)
{
	if (tap_id > 4)
		return -EINVAL;

	/* manual-mode enable bits, [10:6] = TAP0-TAP4 */
	return rtpcs_sds_write_bits(sds, PAGE_ANA_10G, 0xf, tap_id + 6, tap_id + 6,
				    enable ? 0x0 : 0x1);
}

static int rtpcs_931x_sds_rxeq_vth_set_value(struct rtpcs_serdes *sds, unsigned int vth_p,
					     unsigned int vth_n)
{
	return rtpcs_sds_write_bits(sds, PAGE_ANA_10G_EXT, 0x12, 11, 4,
				    FIELD_PREP(GENMASK(3, 0), vth_p) |
				    FIELD_PREP(GENMASK(7, 4), vth_n));
}

static int rtpcs_931x_sds_rxeq_vth_set_adapt(struct rtpcs_serdes *sds, bool enable)
{
	return rtpcs_sds_write_bits(sds, PAGE_ANA_10G, 0xf, 12, 12, enable ? 0x0 : 0x1);
}

static int rtpcs_931x_sds_rxeq_vth_get(struct rtpcs_serdes *sds, unsigned int *vth_p,
				       unsigned int *vth_n)
{
	int ret, val;

	ret = rtpcs_931x_sds_set_debug(sds, 0x2);
	if (ret < 0)
		return ret;

	ret = rtpcs_sds_write_bits(sds, PAGE_ANA_10G, 0x14, 10, 5, 0x0c); /* COEF_SEL = VTH */
	if (ret < 0)
		return ret;
	usleep_range(1000, 2000);

	val = rtpcs_sds_read_bits(sds, PAGE_WDIG, 0x14, 7, 0);
	if (val < 0)
		return val;

	*vth_p = FIELD_GET(GENMASK(3, 0), val);
	*vth_n = FIELD_GET(GENMASK(7, 4), val);

	return 0;
}

/**
 * rtpcs_931x_sds_reset_leq_dfe() - Reset LEQ + DFE to a baseline.
 *
 * @sds: Reference to SerDes instance
 *
 * Reset both LEQ and DFE in the RX path to baseline configuration. I.e.
 * sets LEQ and DFE to manual mode and sets certain values (mostly 0) for
 * LEQ and DFE coefficients/parameters.
 *
 * LEQ and DFE can run in two modes:
 * * manual: specific values are set and used
 * * auto: both adapt their parameters automatically
 *
 */
static int rtpcs_931x_sds_reset_leq_dfe(struct rtpcs_serdes *sds)
{
	rtpcs_931x_sds_rxeq_leq_set_adapt(sds, false);
	rtpcs_931x_sds_rxeq_leq_set_coef(sds, 0);
	/* bits [1:0] are undocumented but part of the known-good reset value */
	rtpcs_sds_write_bits(sds, PAGE_ANA_10G, 0xd, 1, 0, 0x0);

	/*
	 * Force manual mode before writing values - not after like the vendor
	 * SDK does - to prevent the adapt engine from overwriting '0' in the
	 * short timeframe.
	 */
	rtpcs_sds_write_bits(sds, PAGE_ANA_10G, 0xf, 12, 6, 0x7f);

	rtpcs_931x_sds_rxeq_tap_set_value(sds, 0, 0x1e, 0);
	rtpcs_931x_sds_rxeq_tap_set_value(sds, 1, 0, 0);
	rtpcs_931x_sds_rxeq_tap_set_value(sds, 2, 0, 0);
	rtpcs_931x_sds_rxeq_tap_set_value(sds, 3, 0, 0);
	rtpcs_931x_sds_rxeq_tap_set_value(sds, 4, 0, 0);

	rtpcs_931x_sds_rxeq_vth_set_value(sds, 0xa, 0xa);
	/* bits [15:12] and [3:0] are undocumented but part of the known-good reset value */
	rtpcs_sds_write_bits(sds, PAGE_ANA_10G_EXT, 0x12, 15, 12, 0x0);
	rtpcs_sds_write_bits(sds, PAGE_ANA_10G_EXT, 0x12, 3, 0, 0xa);

	return 0;
}

/*
 * Disable DFE auto-adapt on the companion 5G analog block before
 * calibrating this lane's 10G LEQ/DFE. Used by the vendor SDK's
 * PHY-attached and PCB-adapt calibration paths.
 */
static int rtpcs_931x_sds_rxeq_dfe_disable_5g(struct rtpcs_serdes *sds)
{
	return rtpcs_sds_write_bits(sds, PAGE_ANA_5G0, 0xf, 12, 6, 0x7f);
}

static int rtpcs_931x_sds_power(struct rtpcs_serdes *sds, bool power_on)
{
	u32 en_val = power_on ? 0 : BIT(sds->id);

	return regmap_write_bits(sds->ctrl->map,
				 RTPCS_931X_PS_SERDES_OFF_MODE_CTRL_ADDR,
				 BIT(sds->id), en_val);
}

/*
 * Set the SerDes mode in the SerDes IP block's registers, after clearing
 * the symbol error counter and forcing the MAC mode off.
 */
static int rtpcs_931x_sds_apply_ip_mode(struct rtpcs_serdes *sds,
					enum rtpcs_sds_mode hw_mode)
{
	int ret;

	/* clear symbol error count before changing mode */
	rtpcs_931x_sds_clear_symerr(sds, hw_mode);
	ret = rtpcs_93xx_sds_set_mac_mode(sds, RTPCS_SDS_MODE_OFF);
	if (ret)
		return ret;

	return rtpcs_93xx_sds_set_ip_mode(sds, hw_mode);
}

static int rtpcs_931x_sds_set_mode(struct rtpcs_serdes *sds,
				   enum rtpcs_sds_mode hw_mode,
				   enum rtpcs_sds_usxgmii_submode submode)
{
	int ret;

	if (hw_mode == RTPCS_SDS_MODE_XSGMII)
		return rtpcs_93xx_sds_set_mac_driven_mode(sds, hw_mode, submode);

	ret = rtpcs_931x_sds_apply_ip_mode(sds, hw_mode);
	if (ret)
		return ret;

	return rtpcs_93xx_sds_apply_usxgmii_submode(sds, submode);
}

static int rtpcs_931x_sds_deactivate(struct rtpcs_serdes *sds)
{
	int ret;

	ret = rtpcs_sds_write_mask(sds, PAGE_ANA_MISC, 0x0,
				   RTL93XX_FRC_PDOWN_MASK | RTL93XX_FRC_RX_EN_MASK,
				   RTL93XX_FRC_PDOWN_DOWN | RTL93XX_FRC_RX_EN_OFF);
	if (ret)
		return ret;

	ret = rtpcs_sds_write_mask(sds, DIGI_1(PAGE_WDIG), 0x1, RTL931X_STOP_GLI_CLK,
				   RTL931X_STOP_GLI_CLK);
	if (ret)
		return ret;

	ret = rtpcs_931x_sds_power(sds, false);
	if (ret)
		return ret;

	return rtpcs_931x_sds_set_mode(sds, RTPCS_SDS_MODE_OFF, RTPCS_SDS_USXGMII_SM_NONE);
}

static int rtpcs_931x_sds_activate(struct rtpcs_serdes *sds)
{
	int ret;

	if (sds->hw_mode != RTPCS_SDS_MODE_USXGMII &&
	    sds->hw_mode != RTPCS_SDS_MODE_10GBASER) {
		ret = rtpcs_sds_write_mask(sds, DIGI_1(PAGE_WDIG), 0x1, RTL931X_STOP_GLI_CLK, 0x0);
		if (ret)
			return ret;
	}

	ret = rtpcs_sds_write_mask(sds, PAGE_ANA_MISC, 0x0,
				   RTL93XX_FRC_PDOWN_MASK | RTL93XX_FRC_RX_EN_MASK,
				   RTL93XX_FRC_PDOWN_UNFORCED | RTL93XX_FRC_RX_EN_ON);
	if (ret)
		return ret;

	return rtpcs_931x_sds_power(sds, true);
}

static void rtpcs_931x_sds_10g_ana_pre(struct rtpcs_serdes *sds)
{
	rtpcs_sds_write(sds, PAGE_ANA_10G, 0x12, 0x2740);
	rtpcs_sds_write_bits(sds, PAGE_ANA_10G_EXT, 0x0, 15, 12, 0x0);
	rtpcs_sds_write(sds, PAGE_ANA_10G_EXT, 0x2, 0x2010);
}

static void rtpcs_931x_sds_10g_ana_post(struct rtpcs_serdes *sds)
{
	rtpcs_sds_write(sds, PAGE_ANA_10G, 0x12, 0x27c0);
	rtpcs_sds_write_bits(sds, PAGE_ANA_10G_EXT, 0x0, 15, 12, 0xc);
	rtpcs_sds_write(sds, PAGE_ANA_10G_EXT, 0x2, 0x6010);
}

static void rtpcs_931x_sds_rx_reset(struct rtpcs_serdes *sds)
{
	if (sds->type != RTPCS_SDS_TYPE_10G)
		return;

	rtpcs_931x_sds_10g_ana_pre(sds);
	rtpcs_sds_write_mask(sds, PAGE_ANA_MISC, 0x0, RTL93XX_FRC_RX_EN_MASK,
			     RTL93XX_FRC_RX_EN_OFF);

	rtpcs_931x_sds_10g_ana_post(sds);
	rtpcs_sds_write_mask(sds, PAGE_ANA_MISC, 0x0, RTL93XX_FRC_RX_EN_MASK,
			     RTL93XX_FRC_RX_EN_ON);
	msleep(50);
}

/*
 * rtpcs_931x_sds_rxcal_leq_adapt() - RX calibration for PHY-attached ports.
 *
 * Vendor SDK: _phy_rtl9310_leq_adapt(). Used whenever the port has an
 * external PHY attached, regardless of hw_mode - the PHY performs its own
 * equalization, so this only resets LEQ and releases it back into
 * auto-adapt. No TAP/VTH involvement.
 */
static void rtpcs_931x_sds_rxcal_leq_adapt(struct rtpcs_serdes *sds)
{
	dev_dbg(sds->ctrl->dev, "SerDes %u PHY-attached RX calibration...\n", sds->id);

	rtpcs_931x_sds_rxeq_leq_set_coef(sds, 0);
	rtpcs_sds_write_bits(sds, PAGE_ANA_10G, 0xd, 1, 0, 0x0);   /* undocumented */
	rtpcs_sds_write_bits(sds, PAGE_ANA_10G, 0xd, 13, 13, 0x0); /* undocumented */

	rtpcs_931x_sds_rxeq_dfe_disable_5g(sds);

	rtpcs_931x_sds_rxeq_leq_set_adapt(sds, false);
	rtpcs_931x_sds_rx_reset(sds);
	rtpcs_931x_sds_rxeq_leq_set_adapt(sds, true);
	msleep(100);

	dev_dbg(sds->ctrl->dev, "SerDes %u LEQ = %#x\n", sds->id,
		rtpcs_931x_sds_rxeq_leq_get_coef(sds));
}

/*
 * rtpcs_931x_sds_rxcal_fiber_adapt() - RX calibration for 10G fiber.
 *
 * Only used for 10GBase-R fiber; calibration not needed for fiber running
 * on slower speeds.
 *
 * Deviates from the vendor SDK's retry shape which is considerably tighter
 * (3 symbol error rechecks, 150ms delay, exact-0 target). symErr's field
 * (8 bits wide) has been observed reading a saturated 0xff right after
 * rx_reset(), which looks like "link hasn't relocked yet" rather than a
 * genuine error count. Thus, recheck more times with more patience per
 * check instead (no reset in between, so a settling link isn't interrupted),
 * and tolerate a small nonzero symbol-error count rather than requiring
 * exactly 0.
 */
static void rtpcs_931x_sds_rxcal_fiber_adapt(struct rtpcs_serdes *sds)
{
	unsigned int vth_p = 0, vth_n = 0, sum_p = 0, sum_n = 0;
	struct device *dev = sds->ctrl->dev;
	int i, samples = 0, symerr = -1;
	bool link_up = false;

	dev_dbg(dev, "SerDes %u fiber RX calibration...\n", sds->id);
	/* per-port calibration offset in the SDK, kept 0 here */
	rtpcs_sds_write_bits(sds, PAGE_ANA_10G, 0xc, 14, 10, 0x0);

	rtpcs_931x_sds_reset_leq_dfe(sds);

	/* let VTH + TAP0 auto-adapt run and settle before sampling/forcing values */
	rtpcs_931x_sds_rxeq_tap_set_adapt(sds, 0, true);
	rtpcs_931x_sds_rxeq_vth_set_adapt(sds, true);
	msleep(200);

	/* average several samples instead of trusting one possibly-noisy read */
	for (i = 0; i < 10; i++) {
		unsigned int p, n;

		if (rtpcs_931x_sds_rxeq_vth_get(sds, &p, &n) == 0) {
			sum_p += p;
			sum_n += n;
			samples++;
		}
		usleep_range(10000, 11000);
	}

	if (samples > 0) {
		vth_p = DIV_ROUND_CLOSEST(sum_p, samples);
		vth_n = DIV_ROUND_CLOSEST(sum_n, samples);
	} else {
		/* fallback baseline */
		vth_p = vth_n = 0xa;
		dev_warn(dev, "SerDes %u failed to read auto-adapted VTH\n", sds->id);
	}

	dev_dbg(dev, "SerDes %u VTH = %#x/%#x\n", sds->id, vth_p, vth_n);

	rtpcs_931x_sds_rxeq_tap_set_value(sds, 0, 31, 0);
	rtpcs_931x_sds_rxeq_tap_set_adapt(sds, 0, false);
	rtpcs_931x_sds_rxeq_vth_set_value(sds, vth_p, vth_n);
	rtpcs_931x_sds_rxeq_vth_set_adapt(sds, false);

	rtpcs_931x_sds_rx_reset(sds);

	/* let DFE TAP1-4 auto-adapt continuously */
	for (i = 1; i <= 4; i++)
		rtpcs_931x_sds_rxeq_tap_set_adapt(sds, i, true);

	for (i = 0; i < 8; i++) {
		rtpcs_931x_sds_clear_symerr(sds, RTPCS_SDS_MODE_10GBASER);
		msleep(300);
		symerr = rtpcs_931x_sds_fiber_get_symerr(sds, RTPCS_SDS_MODE_10GBASER);
		link_up = rtpcs_931x_sds_10gr_link_up(sds);
		dev_dbg(dev, "SerDes %u symErr check %d: linkUp=%d symErr=0x%x\n", sds->id,
			i + 1, link_up, symerr);

		/*
		 * symErr also reads 0 with no signal at all, not just a clean
		 * link - don't trust it without link_up confirming there's
		 * actually something being decoded.
		 */
		if (link_up && symerr >= 0 && symerr <= 5) {
			dev_dbg(dev, "SerDes %u fiber RX calibration OK (check %d)\n",
				sds->id, i + 1);
			return;
		}
	}

	if (link_up)
		dev_dbg(dev, "SerDes %u fiber RX calibration: symErr still 0x%x after %d checks, link up anyway\n",
			sds->id, symerr, i);
	else
		dev_warn(dev, "SerDes %u fiber RX calibration failed after %d symErr checks\n",
			 sds->id, i);
}

static int rtpcs_931x_sds_get_pll_select(struct rtpcs_serdes *sds, enum rtpcs_sds_pll_type *pll)
{
	int cmu_page, pll_sel;

	cmu_page = rtpcs_93xx_sds_get_cmu_page(sds->hw_mode);
	if (cmu_page < 0)
		return cmu_page;

	pll_sel = rtpcs_sds_read_bits(sds, cmu_page, 0x7, 15, 15);
	if (pll_sel < 0)
		return pll_sel;

	*pll = (enum rtpcs_sds_pll_type)pll_sel;
	return 0;
}

static int rtpcs_931x_sds_set_pll_select(struct rtpcs_serdes *sds, enum rtpcs_sds_mode hw_mode,
					 enum rtpcs_sds_pll_type pll)
{
	struct rtpcs_serdes *even_sds = rtpcs_sds_get_even(sds);
	int cmu_page, ret, val;
	int frc_lc_mode_bit;

	cmu_page = rtpcs_93xx_sds_get_cmu_page(hw_mode);
	if (cmu_page < 0)
		return cmu_page;

	/*
	 * bits [5:4] (even) / [7:6] (odd) are used by RTL930x as selector. The selector
	 * for RTL931x SerDes is in the CMU page of each SerDes, depending on the hardware
	 * mode.
	 *
	 * Here, the SDK calls them 'frc_lc_mode' and 'frc_lc_mode_val'. However, they don't
	 * seem to have any effect and thus their purpose is unknown. So just set them as
	 * the SDK does.
	 */
	val = (pll == RTPCS_SDS_PLL_TYPE_LC) ? 0x3 : 0x1;
	frc_lc_mode_bit = (sds == even_sds) ? 4 : 6;
	ret = rtpcs_sds_write_bits(even_sds, PAGE_ANA_MISC, 0x12, frc_lc_mode_bit + 1,
				   frc_lc_mode_bit, val);
	if (ret < 0)
		return ret;

	return rtpcs_sds_write_bits(sds, cmu_page, 0x7, 15, 15, pll);
}

static int rtpcs_931x_sds_reconfigure_to_pll(struct rtpcs_serdes *sds, enum rtpcs_sds_pll_type pll)
{
	enum rtpcs_sds_pll_type tmp_pll;
	enum rtpcs_sds_pll_speed speed;
	enum rtpcs_sds_mode tmp_mode;
	int ret;

	/* assume we always reconfigure to the other PLL */
	tmp_pll = (pll == RTPCS_SDS_PLL_TYPE_LC) ? RTPCS_SDS_PLL_TYPE_RING : RTPCS_SDS_PLL_TYPE_LC;

	ret = rtpcs_93xx_sds_get_pll_config(sds, tmp_pll, &speed);
	if (ret < 0)
		return ret;

	tmp_mode = sds->hw_mode;

	/* turn off SerDes for reconfiguration */
	ret = rtpcs_931x_sds_power(sds, false);
	if (ret < 0)
		return ret;

	ret = rtpcs_931x_sds_set_mode(sds, RTPCS_SDS_MODE_OFF, RTPCS_SDS_USXGMII_SM_NONE);
	if (ret < 0)
		return ret;

	/* reconfigure to other PLL */
	ret = rtpcs_93xx_sds_set_pll_config(sds, pll, speed);
	if (ret < 0)
		return ret;

	ret = rtpcs_931x_sds_set_pll_select(sds, sds->hw_mode, pll);
	if (ret < 0)
		return ret;

	/* turn on SerDes again */
	ret = rtpcs_931x_sds_set_mode(sds, tmp_mode, sds->usxgmii_submode);
	if (ret < 0)
		return ret;

	return rtpcs_931x_sds_power(sds, true);
}

__always_unused
static int rtpcs_931x_sds_link_sts_get(struct rtpcs_serdes *sds)
{
	u32 sts, sts1, latch_sts, latch_sts1;

	switch (sds->hw_mode) {
	case RTPCS_SDS_MODE_XSGMII:
		sts = rtpcs_sds_read_bits(sds, DIGI_1(PAGE_SDS_EXT), 29, 8, 0);
		sts1 = rtpcs_sds_read_bits(sds, DIGI_2(PAGE_SDS_EXT), 29, 8, 0);
		latch_sts = rtpcs_sds_read_bits(sds, DIGI_1(PAGE_SDS_EXT), 30, 8, 0);
		latch_sts1 = rtpcs_sds_read_bits(sds, DIGI_2(PAGE_SDS_EXT), 30, 8, 0);
		break;

	case RTPCS_SDS_MODE_SGMII:
	case RTPCS_SDS_MODE_2500BASEX:
		sts = rtpcs_sds_read_bits(sds, DIGI_1(PAGE_SDS_EXT), 29, 8, 0);
		latch_sts = rtpcs_sds_read_bits(sds, DIGI_1(PAGE_SDS_EXT), 30, 8, 0);
		break;

	default:
		sts = rtpcs_sds_read_bits(sds, PAGE_TGR_STD_1, 0, 12, 12);
		latch_sts = rtpcs_sds_read_bits(sds, PAGE_TGR_STD_0, 1, 2, 2);
		latch_sts1 = rtpcs_sds_read_bits(sds, DIGI_1(PAGE_FIB), 1, 2, 2);
		sts1 = rtpcs_sds_read_bits(sds, DIGI_1(PAGE_FIB), 1, 2, 2);
	}

	dev_info(sds->ctrl->dev, "SerDes %d sts %d, sts1 %d, latch_sts %d, latch_sts1 %d\n",
		 sds->id, sts, sts1, latch_sts, latch_sts1);

	return sts1;
}

static int rtpcs_931x_sds_config_polarity(struct rtpcs_serdes *sds, unsigned int tx_pol,
					  unsigned int rx_pol)
{
	u8 rx_val = (rx_pol == PHY_POL_INVERT) ? 1 : 0;
	u8 tx_val = (tx_pol == PHY_POL_INVERT) ? 1 : 0;
	u32 val;
	int ret;

	/* 10gr_*_inv */
	val = (tx_val << 1) | rx_val;
	ret = rtpcs_sds_write_bits(sds, PAGE_TGR_PRO_0, 0x2, 14, 13, val);
	if (ret)
		return ret;

	/* xsg_*_inv */
	val = (rx_val << 1) | tx_val;
	return rtpcs_sds_xsg_write_bits(sds, PAGE_SDS, 0x0, 9, 8, val);
}

static const struct rtpcs_sds_tx_config rtpcs_931x_sds_tx_cfg_v1[] = {
	{ .pre_amp = 0x00, .main_amp = 0x10, .post_amp = 0x06 },
	{ .pre_amp = 0x00, .main_amp = 0x10, .post_amp = 0x06 },
	{ .pre_amp = 0x00, .main_amp = 0x10 },
	{ .pre_amp = 0x00, .main_amp = 0x10 },
	{ .pre_amp = 0x00, .main_amp = 0x10 },
	{ .pre_amp = 0x00, .main_amp = 0x10 },
	{ .pre_amp = 0x03, .main_amp = 0x0d },
	{ .pre_amp = 0x03, .main_amp = 0x0d },
	{ .pre_amp = 0x03, .main_amp = 0x0d },
	{ .pre_amp = 0x03, .main_amp = 0x0d },
	{ .pre_amp = 0x03, .main_amp = 0x0f },
	{ .pre_amp = 0x03, .main_amp = 0x0f },
};
static const struct rtpcs_sds_tx_config rtpcs_931x_sds_tx_cfg_v2[] = {
	{ .pre_amp = 0x00, .main_amp = 0x0e, .post_amp = 0x03 },
	{ .pre_amp = 0x00, .main_amp = 0x0e },
	{ .pre_amp = 0x00, .main_amp = 0x10 },
	{ .pre_amp = 0x00, .main_amp = 0x0c },
	{ .pre_amp = 0x00, .main_amp = 0x0b },
	{ .pre_amp = 0x03, .main_amp = 0x09 },
	{ .pre_amp = 0x03, .main_amp = 0x09 },
	{ .pre_amp = 0x03, .main_amp = 0x0b },
	{ .pre_amp = 0x03, .main_amp = 0x0d },
	{ .pre_amp = 0x00, .main_amp = 0x0d },
	{ .pre_amp = 0x03, .main_amp = 0x0e },
	{ .pre_amp = 0x03, .main_amp = 0x0e, .post_amp = 0x02 },
};
static const struct rtpcs_sds_tx_config rtpcs_931x_sds_tx_cfg_sdac = { /* short DACs */
	.pre_amp = 0x00, .main_amp = 0x1a, .post_amp = 0x04
};
static const struct rtpcs_sds_tx_config rtpcs_931x_sds_tx_cfg_ldac = { /* long DACs */
	.pre_amp = 0x00, .main_amp = 0x10, .post_amp = 0x14
};

/**
 * rtpcs_931x_sds_config_tx_amps - Configure SerDes TX amplifiers
 *
 * A SerDes has three amplifiers (pre, main, post) in the TX path that allow to tune the signal,
 * usually based on eye diagrams. This is needed to account for different tx media, i.e. PCB
 * trace, fiber, DAC. Using the amplifier coefficients, one can precondition the signal in such
 * a way so that it arrives "clean" at the partner.
 */
static int rtpcs_931x_sds_config_tx_amps(struct rtpcs_serdes *sds, u8 pre_amp, u8 main_amp,
					 u8 post_amp)
{
	u16 cfg_val, en_val = 0;
	int ret;

	cfg_val = FIELD_PREP(RTPCS_931X_SDS_PRE_AMP_MASK, pre_amp) |
		  FIELD_PREP(RTPCS_931X_SDS_MAIN_AMP_MASK, main_amp) |
		  FIELD_PREP(RTPCS_931X_SDS_POST_AMP_MASK, post_amp);
	ret = rtpcs_sds_write(sds, PAGE_ANA_10G, 0x1, cfg_val);
	if (ret < 0)
		return ret;

	/* enable/disable pre + post amp, main amp has no enable bit so seems always active */
	if (post_amp)
		en_val |= BIT(0);
	if (pre_amp)
		en_val |= BIT(1);

	return rtpcs_sds_write_bits(sds, PAGE_ANA_10G, 0x0, 1, 0, en_val);
}

static int rtpcs_931x_sds_config_attachment(struct rtpcs_serdes *sds,
					    enum rtpcs_sds_attachment attachment,
					    enum rtpcs_sds_mode hw_mode)
{
	struct rtpcs_serdes *even_sds = rtpcs_sds_get_even(sds);
	const struct rtpcs_sds_tx_config *tx_cfg;
	bool is_dac, is_10g;
	int ret;

	if (sds->type != RTPCS_SDS_TYPE_10G)
		return 0;

	rtpcs_sds_write_bits(sds, PAGE_ANA_10G, 0xe, 13, 11, 0x0);
	if (hw_mode != RTPCS_SDS_MODE_XSGMII)
		rtpcs_931x_sds_reset_leq_dfe(sds);

	/* SDK: media none behavior - baseline applied regardless of attachment */
	rtpcs_931x_sds_10g_ana_pre(sds);

	rtpcs_sds_write_mask(sds, PAGE_ANA_MISC, 0x0, RTL93XX_FRC_CMU_EN_MASK,
			     RTL93XX_FRC_CMU_EN_FORCE_ON);
	rtpcs_sds_write_mask(sds, PAGE_ANA_MISC, 0x0, RTL93XX_FRC_V2ANALOG_MASK,
			     RTL93XX_FRC_V2ANALOG_FORCE_OFF);

	rtpcs_sds_write_bits(sds, PAGE_ANA_10G, 0xf, 5, 0, 0x4);
	rtpcs_sds_write_bits(sds, PAGE_ANA_5G0, 0x12, 7, 6, 0x1);

	if (attachment == RTPCS_SDS_ATTACH_NONE)
		return 0;

	is_dac = (attachment == RTPCS_SDS_ATTACH_DAC_SHORT ||
		  attachment == RTPCS_SDS_ATTACH_DAC_LONG);
	is_10g = (hw_mode == RTPCS_SDS_MODE_10GBASER ||
		  hw_mode == RTPCS_SDS_MODE_XSGMII ||
		  hw_mode == RTPCS_SDS_MODE_USXGMII);

	rtpcs_sds_write_mask(sds, PAGE_ANA_MISC, 0x0, RTL93XX_FRC_CMU_EN_MASK,
			     RTL93XX_FRC_CMU_EN_UNFORCED);
	rtpcs_sds_write_bits(sds, PAGE_ANA_5G0, 0x7, 15, 15, is_dac ? 0x1 : 0x0);
	rtpcs_sds_write_mask(sds, PAGE_ANA_MISC, 0x0, RTL93XX_FRC_CMU_EN_MASK,
			     RTL93XX_FRC_CMU_EN_FORCE_ON);

	switch (attachment) {
	case RTPCS_SDS_ATTACH_DAC_SHORT:
	case RTPCS_SDS_ATTACH_DAC_LONG:
		tx_cfg = (attachment == RTPCS_SDS_ATTACH_DAC_SHORT) ? &rtpcs_931x_sds_tx_cfg_sdac :
								      &rtpcs_931x_sds_tx_cfg_ldac;

		rtpcs_sds_write(sds, PAGE_ANA_COM, 0x19, 0xf0a5);
		rtpcs_sds_write(even_sds, PAGE_ANA_10G, 0x8, 0x02a0); /* [10:7] TX impedance */
		break;

	case RTPCS_SDS_ATTACH_FIBER:
		if (is_10g)
			rtpcs_sds_write_bits(sds, PAGE_ANA_10G, 0xf, 5, 0, 0x2);

		fallthrough;
	default:
		if (sds->ctrl->chip_version == RTPCS_CHIP_V2)
			tx_cfg = &rtpcs_931x_sds_tx_cfg_v2[sds->id - 2];
		else
			tx_cfg = &rtpcs_931x_sds_tx_cfg_v1[sds->id - 2];

		rtpcs_sds_write(sds, PAGE_ANA_COM, 0x19, 0xf0f0);
		rtpcs_sds_write(even_sds, PAGE_ANA_10G, 0x8, 0x0294); /* [10:7] TX impedance */
		break;
	}

	ret = rtpcs_931x_sds_config_tx_amps(sds, tx_cfg->pre_amp, tx_cfg->main_amp,
					    tx_cfg->post_amp);
	if (ret)
		return ret;

	rtpcs_sds_write_mask(sds, PAGE_TGR_PRO_0, 0xd, RTL93XX_LINKDW_SEL,
			     is_dac ? RTL93XX_LINKDW_SEL_DAC : RTL93XX_LINKDW_SEL_NON_DAC);

	if (is_10g)
		rtpcs_931x_sds_10g_ana_post(sds);

	rtpcs_sds_write_mask(sds, PAGE_ANA_MISC, 0x0, RTL93XX_FRC_V2ANALOG_MASK,
			     RTL93XX_FRC_V2ANALOG_UNFORCED);
	rtpcs_sds_write_bits(sds, PAGE_ANA_5G0, 0x12, 7, 6, 0x3);

	rtpcs_sds_write_mask(sds, PAGE_ANA_MISC, 0x0, RTL93XX_FRC_CMU_EN_MASK,
			     RTL93XX_FRC_CMU_EN_FORCE_OFF);
	rtpcs_sds_write_mask(sds, PAGE_ANA_MISC, 0x0, RTL93XX_FRC_CMU_EN_MASK,
			     RTL93XX_FRC_CMU_EN_FORCE_ON);

	/* clear pending SerDes RX idle interrupt flag */
	return regmap_write_bits(sds->ctrl->map, RTPCS_931X_ISR_SERDES_RXIDLE,
				 BIT(sds->id - 2), BIT(sds->id - 2));
}

/*
 * rtpcs_931x_sds_post_config() - RX calibration, run after power-up.
 *
 * Vendor SDK: _phy_rtl9310_rxCali(). Dispatches by attachment, mirroring
 * the SDK's own split into separate calibration paths per attachment.
 */
static int rtpcs_931x_sds_post_config(struct rtpcs_serdes *sds, enum rtpcs_sds_mode hw_mode)
{
	if (sds->type != RTPCS_SDS_TYPE_10G)
		return 0;

	switch (sds->attachment) {
	case RTPCS_SDS_ATTACH_PHY:
		rtpcs_931x_sds_rxcal_leq_adapt(sds);
		break;

	case RTPCS_SDS_ATTACH_FIBER:
		if (hw_mode == RTPCS_SDS_MODE_10GBASER)
			rtpcs_931x_sds_rxcal_fiber_adapt(sds);
		break;

	default:
		/* TODO: DAC RX calibration */
		break;
	}

	return 0;
}

static int rtpcs_931x_sds_config_hw_mode(struct rtpcs_serdes *sds,
					 enum rtpcs_sds_mode hw_mode)
{
	switch (hw_mode) {
	case RTPCS_SDS_MODE_OFF:
		break;

	case RTPCS_SDS_MODE_1000BASEX:
		rtpcs_sds_write_bits(sds, DIGI_1(PAGE_FIB_EXT), 0x13, 15, 14, 0);

		/* BMCR_SPEED1000 */
		rtpcs_sds_write_bits(sds, DIGI_1(PAGE_FIB), MII_BMCR, 6, 6, 1);
		/* BMCR_SPEED100 */
		rtpcs_sds_write_bits(sds, DIGI_1(PAGE_FIB), MII_BMCR, 13, 13, 0);
		/* EN_LINK_FIB1G */
		rtpcs_sds_write_bits(sds, DIGI_1(PAGE_SDS), 0x4, 2, 2, 1);
		break;

	case RTPCS_SDS_MODE_2500BASEX:
		rtpcs_sds_write_bits(sds, DIGI_1(PAGE_SDS_EXT), 0x14, 8, 8, 1);
		break;

	case RTPCS_SDS_MODE_10GBASER:
		/* configure 10GR fiber mode=1 */
		rtpcs_sds_write_bits(sds, PAGE_WDIG, 0xb, 1, 1, 1);
		break;

	case RTPCS_SDS_MODE_SGMII:
		rtpcs_sds_write_bits(sds, PAGE_ANA_1G2, 0x9, 15, 15, 0);
		break;

	case RTPCS_SDS_MODE_XSGMII:
		rtpcs_sds_xsg_write_bits(sds, PAGE_SDS, 0xe, 12, 12, 0x1);
		break;

	case RTPCS_SDS_MODE_USXGMII:
		rtpcs_93xx_sds_usxgmii_config(sds);
		break;

	case RTPCS_SDS_MODE_QSGMII:
	default:
		return -ENOTSUPP;
	}

	return rtpcs_93xx_sds_config_cmu(sds, hw_mode);
}

/**
 * rtpcs_931x_init_mac_groups - Initialize MAC groups
 *
 * RTL931x organizes MACs into 12 groups (one per SerDes) that must be explicitly
 * enabled before link establishment. Without initialization, link may fail or
 * packets may be corrupted, especially in USXGMII/XSGMII modes.
 *
 * Simply enable all MACs by writing 0xffffffff to all group registers. Unused
 * MACs and reserved bits are harmless, avoiding complex per-SerDes logic.
 *
 * This lives in the PCS driver since groups are tied to SerDes, and the DSA
 * driver has no SerDes awareness.
 */
static int rtpcs_931x_init_mac_groups(struct rtpcs_ctrl *ctrl)
{
	static const u32 mac_group_regs[] = {
		RTPCS_931X_MAC_GROUP0_1_CTRL,
		RTPCS_931X_MAC_GROUP2_3_CTRL,
		RTPCS_931X_MAC_GROUP4_CTRL,
		RTPCS_931X_MAC_GROUP5_CTRL,
		RTPCS_931X_MAC_GROUP6_7_CTRL,
		RTPCS_931X_MAC_GROUP8_11_CTRL,
	};
	int ret;

	for (int i = 0; i < ARRAY_SIZE(mac_group_regs); i++) {
		ret = regmap_write(ctrl->map, mac_group_regs[i], 0xffffffff);
		if (ret)
			return ret;
	}

	return 0;
}

static int rtpcs_931x_sds_probe(struct rtpcs_serdes *sds)
{
	u32 base = RTPCS_931X_SERDES_MODE_CTRL + 4 * (sds->id >> 2);
	u8 lsb = (sds->id & 3) * 8;
	int ret;

	if (sds->id >= 2)
		sds->type = RTPCS_SDS_TYPE_10G;
	else
		sds->type = RTPCS_SDS_TYPE_UNKNOWN;

	rtpcs_93xx_sds_fill_caps(sds);

	/*
	 * Width is 7 bits (lsb..lsb+6) so every MAC mode write also clears
	 * bit 5 (FEC enable) and bit 6 (10G speedup). These are mode-dependent
	 * and not yet programmed here; keeping them cleared matches the
	 * original 8-bit-wide write behaviour.
	 */
	ret = rtpcs_sds_alloc_field(sds, &sds->swcore_regs.mac_mode,
				    base, lsb, lsb + 6);
	if (ret)
		return ret;

	ret = rtpcs_sds_alloc_field(sds, &sds->swcore_regs.mac_mode_force,
				    base, lsb + 7, lsb + 7);
	if (ret)
		return ret;

	/*
	 * USXGMII submode is packed at 5 bits per SerDes for IDs 2..13,
	 * six entries per 32-bit word, non-straddling.
	 */
	if (sds->type == RTPCS_SDS_TYPE_10G) {
		u8 submode_lsb = ((sds->id - 2) % 6) * 5;

		ret = rtpcs_sds_alloc_field(sds, &sds->swcore_regs.usxgmii_submode,
					    RTPCS_931X_SDS_USXGMII_SUBMODE + ((sds->id - 2) / 6) * 4,
					    submode_lsb, submode_lsb + 4);
		if (ret)
			return ret;
	}

	return 0;
}

static int rtpcs_931x_init(struct rtpcs_ctrl *ctrl)
{
	int ret;

	ret = rtpcs_931x_init_mac_groups(ctrl);
	if (ret < 0)
		return ret;

	return rtpcs_93xx_init(ctrl);
}

/* Common functions */

static int rtpcs_sds_config_polarity(struct rtpcs_serdes *sds, phy_interface_t if_mode)
{
	unsigned int rx_pol, tx_pol;
	int ret;

	if (!sds->fwnode)
		return 0;

	ret = phy_get_manual_rx_polarity(sds->fwnode, phy_modes(if_mode), &rx_pol);
	if (ret < 0)
		return ret;

	ret = phy_get_manual_tx_polarity(sds->fwnode, phy_modes(if_mode), &tx_pol);
	if (ret < 0)
		return ret;

	if (!sds->ops->config_polarity) {
		if (tx_pol != PHY_POL_NORMAL || rx_pol != PHY_POL_NORMAL)
			dev_warn(sds->ctrl->dev,
				 "Polarity change requested but not supported\n");
		return 0;
	}

	return sds->ops->config_polarity(sds, tx_pol, rx_pol);
}

static void rtpcs_pcs_get_state(struct phylink_pcs *pcs, unsigned int neg_mode,
				struct phylink_link_state *state)
{
	struct rtpcs_link *link = rtpcs_phylink_pcs_to_link(pcs);
	struct rtpcs_ctrl *ctrl = link->ctrl;
	int port = link->port;
	int linkup, speed;

	state->link = 0;
	state->speed = SPEED_UNKNOWN;
	state->duplex = DUPLEX_UNKNOWN;
	state->pause &= ~(MLO_PAUSE_RX | MLO_PAUSE_TX);

	/* Read MAC side link twice */
	for (int i = 0; i < 2; i++)
		linkup = rtpcs_regmap_read_bits(ctrl, ctrl->cfg->mac_link_sts, port, port);

	if (!linkup)
		return;

	state->link = 1;
	state->duplex = rtpcs_regmap_read_bits(ctrl, ctrl->cfg->mac_link_dup_sts, port, port);

	speed = rtpcs_regmap_read_bits(ctrl, ctrl->cfg->mac_link_spd_sts,
				       ctrl->cfg->mac_link_spd_bits * (port + 1) - 1,
				       ctrl->cfg->mac_link_spd_bits * port);
	switch (speed) {
	case RTPCS_SPEED_10:
		state->speed = SPEED_10;
		break;
	case RTPCS_SPEED_100:
		state->speed = SPEED_100;
		break;
	case RTPCS_SPEED_1000:
		state->speed = SPEED_1000;
		break;
	case RTPCS_SPEED_10000:
	case RTPCS_SPEED_10000_LEGACY:
		/*
		 * The legacy mode is ok so far with minor inconsistencies. On RTL838x this flag
		 * is either 500M or 2G. It might be that MAC_GLITE_STS register tells more. On
		 * RTL839x this is either 500M or 10G. More info might be in MAC_LINK_500M_STS.
		 * Without support for the 500M modes simply resolve to 10G.
		 */
		state->speed = SPEED_10000;
		break;
	case RTPCS_SPEED_2500:
		state->speed = SPEED_2500;
		break;
	case RTPCS_SPEED_5000:
		state->speed = SPEED_5000;
		break;
	default:
		dev_err(ctrl->dev, "unknown speed %d\n", speed);
	}

	if (rtpcs_regmap_read_bits(ctrl, ctrl->cfg->mac_rx_pause_sts, port, port))
		state->pause |= MLO_PAUSE_RX;
	if (rtpcs_regmap_read_bits(ctrl, ctrl->cfg->mac_tx_pause_sts, port, port))
		state->pause |= MLO_PAUSE_TX;
}

static void rtpcs_pcs_an_restart(struct phylink_pcs *pcs)
{
	struct rtpcs_link *link = rtpcs_phylink_pcs_to_link(pcs);
	struct rtpcs_ctrl *ctrl = link->ctrl;
	struct rtpcs_serdes *sds = link->sds;

	mutex_lock(&ctrl->lock);
	sds->ops->restart_autoneg(sds);
	mutex_unlock(&ctrl->lock);
}

static int rtpcs_pcs_config(struct phylink_pcs *pcs, unsigned int neg_mode,
			    phy_interface_t interface, const unsigned long *advertising,
			    bool permit_pause_to_mac)
{
	struct rtpcs_link *link = rtpcs_phylink_pcs_to_link(pcs);
	struct rtpcs_ctrl *ctrl = link->ctrl;
	struct rtpcs_serdes *sds = link->sds;
	enum rtpcs_sds_attachment attachment;
	enum rtpcs_sds_usxgmii_submode submode;
	enum rtpcs_sds_mode hw_mode;
	int ret;

	ret = rtpcs_sds_select_hw_mode(sds, interface, &hw_mode, &submode);
	if (ret < 0) {
		dev_err(ctrl->dev, "SerDes %u doesn't support %s mode\n", sds->id,
			phy_modes(interface));
		return -EOPNOTSUPP;
	}

	scoped_guard(mutex, &ctrl->lock) {
		if (sds->hw_mode != hw_mode || sds->usxgmii_submode != submode) {
			ret = rtpcs_sds_config_polarity(sds, interface);
			if (ret < 0) {
				dev_err(ctrl->dev, "failed to configure polarity of SerDes %u\n",
					sds->id);
				return ret;
			}

			ret = sds->ops->deactivate(sds);
			if (ret < 0)
				return ret;

			ret = sds->ops->config_hw_mode(sds, hw_mode);
			if (ret < 0)
				return ret;

			if (sds->ops->config_attachment) {
				ret = rtpcs_sds_select_attachment(hw_mode, &attachment);
				if (ret < 0)
					return ret;

				ret = sds->ops->config_attachment(sds, attachment, hw_mode);
				if (ret < 0)
					return ret;

				sds->attachment = attachment;
			}

			ret = sds->ops->set_hw_mode(sds, hw_mode, submode);
			if (ret < 0)
				return ret;

			sds->hw_mode = hw_mode;
			sds->usxgmii_submode = submode;

			ret = sds->ops->activate(sds);
			if (ret < 0)
				return ret;

			if (sds->ops->post_config) {
				ret = sds->ops->post_config(sds, hw_mode);
				if (ret < 0)
					return ret;
			}

			sds->first_start = false;

			dev_info(ctrl->dev, "SerDes %u configured for %s mode\n",
				 sds->id, phy_modes(interface));
		} else
			dev_dbg(ctrl->dev, "SerDes %u already in mode %s, no change\n",
				 sds->id, phy_modes(interface));

		ret = sds->ops->set_autoneg(sds, neg_mode, advertising);
	}

	return ret;
}

static void rtpcs_mdio_bus_put(void *data)
{
	struct mii_bus *mdio_bus = data;

	put_device(&mdio_bus->dev);
}

static struct mii_bus *rtpcs_find_mdio_bus(struct rtpcs_ctrl *ctrl)
{
	struct device_node *mdio_np;
	struct mii_bus *mdio_bus;
	int ret;

	mdio_np = of_parse_phandle(ctrl->dev->of_node, "mdio-parent-bus", 0);
	if (!mdio_np)
		return ERR_PTR(dev_err_probe(ctrl->dev, -ENODEV, "MDIO parent bus not found\n"));

	if (!of_device_is_available(mdio_np)) {
		of_node_put(mdio_np);
		return ERR_PTR(dev_err_probe(ctrl->dev, -ENODEV, "MDIO parent bus not usable\n"));
	}

	mdio_bus = of_mdio_find_bus(mdio_np);
	of_node_put(mdio_np);
	if (!mdio_bus)
		return ERR_PTR(dev_err_probe(ctrl->dev, -EPROBE_DEFER,
					     "MDIO parent bus not (yet) active\n"));

	ret = devm_add_action_or_reset(ctrl->dev, rtpcs_mdio_bus_put, mdio_bus);
	if (ret)
		return ERR_PTR(ret);

	return mdio_bus;
}

static void rtpcs_sds_put_fwnode(void *data)
{
	struct rtpcs_serdes *sds = data;

	fwnode_handle_put(sds->fwnode);
}

static void rtpcs_del_provider_action(void *data)
{
	struct rtpcs_serdes *sds = data;

	fwnode_pcs_del_provider(sds->fwnode);

	rtnl_lock();
	for (int i = 0; i < RTPCS_MAX_LINKS_PER_SDS; i++) {
		if (!sds->link[i])
			continue;

		phylink_release_pcs(&sds->link[i]->pcs);
	}
	rtnl_unlock();
}

static struct rtpcs_serdes *rtpcs_find_serdes(struct rtpcs_ctrl *ctrl,
					      struct fwnode_handle *fwnode)
{
	for (int i = 0; i < ctrl->cfg->serdes_count; i++) {
		if (ctrl->serdes[i].fwnode == fwnode)
			return &ctrl->serdes[i];
	}
	return NULL;
}

/*
 * Walk the sibling switch's ethernet-ports subtree to learn which MAC port
 * each (SerDes, link_idx) pair serves. Same "backwards" topology lookup the
 * sibling MDIO driver does for phy-handle: the DT already encodes the
 * mapping via per-port pcs-handle properties, so the driver doesn't need a
 * parallel per-SoC table. pcs_get_state still needs the port number to
 * index MAC-side link status registers; it reads link_port[] populated
 * here.
 */
static int rtpcs_map_links(struct device *dev, struct rtpcs_ctrl *ctrl)
{
	struct fwnode_handle *fw_dev = dev_fwnode(dev);
	struct fwnode_handle *fw_switch, *fw_ports;
	int ret = 0;

	fw_switch = fwnode_get_parent(fw_dev);
	if (!fw_switch)
		return -ENODEV;

	fw_ports = fwnode_get_named_child_node(fw_switch, "ethernet-ports");
	if (!fw_ports) {
		ret = dev_err_probe(dev, -ENODEV, "%pfwP missing ethernet-ports\n",
				    fw_switch);
		goto put_switch;
	}

	fwnode_for_each_child_node_scoped(fw_ports, fw_port) {
		struct fwnode_reference_args args;
		struct rtpcs_serdes *sds;
		int link_idx;
		u32 pn;

		if (fwnode_property_read_u32(fw_port, "reg", &pn))
			continue;

		if (fwnode_property_get_reference_args(fw_port, "pcs-handle", "#pcs-cells",
						       -1, 0, &args))
			continue;

		link_idx = args.args[0];
		sds = rtpcs_find_serdes(ctrl, args.fwnode);
		fwnode_handle_put(args.fwnode);
		if (!sds)
			continue;

		if (link_idx >= RTPCS_MAX_LINKS_PER_SDS) {
			ret = dev_err_probe(dev, -ERANGE,
					    "%pfwP: pcs-handle link %d exceeds max %u\n",
					    fw_port, link_idx, RTPCS_MAX_LINKS_PER_SDS);
			break;
		}

		if (sds->link_port[link_idx] >= 0) {
			ret = dev_err_probe(dev, -EEXIST,
					    "%pfwP: sds%u link %d already assigned to port %d\n",
					    fw_port, sds->id, link_idx,
					    sds->link_port[link_idx]);
			break;
		}

		sds->link_port[link_idx] = pn;
		sds->num_of_links++;
	}

	fwnode_handle_put(fw_ports);
put_switch:
	fwnode_handle_put(fw_switch);
	return ret;
}

static struct phylink_pcs *rtpcs_pcs_get(struct fwnode_reference_args *pcsspec, void *data)
{
	struct rtpcs_serdes *sds = data;
	struct rtpcs_link *link;
	unsigned int link_idx;
	struct device *dev;

	dev = sds->ctrl->dev;
	if (!pcsspec->nargs) {
		dev_err(dev, "invalid number of cells in 'pcs' property\n");
		return ERR_PTR(-EINVAL);
	}

	link_idx = pcsspec->args[0];
	if (link_idx >= RTPCS_MAX_LINKS_PER_SDS)
		return ERR_PTR(-EINVAL);

	if (sds->link_port[link_idx] < 0) {
		dev_err(dev, "sds %u link %d not associated with any port\n",
			sds->id, link_idx);
		return ERR_PTR(-ENODEV);
	}

	if (!sds->link[link_idx]) {
		link = devm_kzalloc(dev, sizeof(*link), GFP_KERNEL);
		if (!link)
			return ERR_PTR(-ENOMEM);

		link->ctrl = sds->ctrl;
		link->port = sds->link_port[link_idx];
		link->sds = sds;
		link->pcs.ops = sds->ctrl->cfg->pcs_ops;

		sds->link[link_idx] = link;
	}

	return &sds->link[link_idx]->pcs;
}

static int rtpcs_probe(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	struct device *dev = &pdev->dev;
	struct rtpcs_serdes *sds;
	struct rtpcs_ctrl *ctrl;
	u32 sds_id;
	int i, ret;

	ctrl = devm_kzalloc(dev, sizeof(*ctrl), GFP_KERNEL);
	if (!ctrl)
		return -ENOMEM;

	mutex_init(&ctrl->lock);

	ctrl->dev = dev;
	ctrl->cfg = device_get_match_data(ctrl->dev);
	ctrl->map = syscon_node_to_regmap(np->parent);
	if (IS_ERR(ctrl->map))
		return PTR_ERR(ctrl->map);

	ctrl->bus = rtpcs_find_mdio_bus(ctrl);
	if (IS_ERR(ctrl->bus))
		return PTR_ERR(ctrl->bus);

	for (i = 0; i < ctrl->cfg->serdes_count; i++) {
		sds = &ctrl->serdes[i];

		sds->ctrl = ctrl;
		sds->first_start = true;
		sds->id = i;
		sds->ops = ctrl->cfg->sds_ops;
		for (int j = 0; j < RTPCS_MAX_LINKS_PER_SDS; j++)
			sds->link_port[j] = -1;

		ret = ctrl->cfg->sds_probe(sds);
		if (ret)
			return ret;
	}

	device_for_each_child_node_scoped(dev, child) {
		ret = fwnode_property_read_u32(child, "reg", &sds_id);
		if (ret)
			return ret;

		if (sds_id >= ctrl->cfg->serdes_count)
			return -EINVAL;

		sds = &ctrl->serdes[sds_id];
		sds->fwnode = fwnode_handle_get(child);
		ret = devm_add_action_or_reset(dev, rtpcs_sds_put_fwnode, sds);
		if (ret)
			return ret;
	}

	ret = rtpcs_map_links(dev, ctrl);
	if (ret)
		return ret;

	if (ctrl->cfg->init) {
		ret = ctrl->cfg->init(ctrl);
		if (ret)
			return ret;
	}

	platform_set_drvdata(pdev, ctrl);

	for (i = 0; i < ctrl->cfg->serdes_count; i++) {
		sds = &ctrl->serdes[i];
		if (!sds->fwnode)
			continue;

		ret = fwnode_pcs_add_provider(sds->fwnode, rtpcs_pcs_get, sds);
		if (ret)
			return ret;
		ret = devm_add_action_or_reset(dev, rtpcs_del_provider_action,
					       sds);
		if (ret)
			return ret;
	}

	dev_info(dev, "Realtek PCS driver initialized\n");
	return 0;
}

static const struct phylink_pcs_ops rtpcs_838x_pcs_ops = {
	.pcs_an_restart		= rtpcs_pcs_an_restart,
	.pcs_config		= rtpcs_pcs_config,
	.pcs_get_state		= rtpcs_pcs_get_state,
};

static const struct rtpcs_sds_ops rtpcs_838x_sds_ops = {
	.read			= rtpcs_generic_sds_op_read,
	.write			= rtpcs_generic_sds_op_write,
	.write_mask		= rtpcs_generic_sds_op_write_mask,
	.set_autoneg		= rtpcs_generic_sds_set_autoneg,
	.restart_autoneg	= rtpcs_generic_sds_restart_autoneg,
	.deactivate		= rtpcs_838x_sds_deactivate,
	.activate		= rtpcs_838x_sds_activate,
	.config_hw_mode		= rtpcs_838x_sds_config_hw_mode,
	.set_hw_mode		= rtpcs_838x_sds_set_mode,
	.post_config		= rtpcs_838x_sds_post_config,
};

static const struct rtpcs_config rtpcs_838x_cfg = {
	.cpu_port		= RTPCS_838X_CPU_PORT,
	.mac_link_dup_sts	= RTPCS_838X_MAC_LINK_DUP_STS,
	.mac_link_spd_sts	= RTPCS_838X_MAC_LINK_SPD_STS,
	.mac_link_spd_bits	= RTPCS_83XX_MAC_LINK_SPD_BITS,
	.mac_link_sts		= RTPCS_838X_MAC_LINK_STS,
	.mac_rx_pause_sts	= RTPCS_838X_MAC_RX_PAUSE_STS,
	.mac_tx_pause_sts	= RTPCS_838X_MAC_TX_PAUSE_STS,
	.serdes_count		= RTPCS_838X_SERDES_CNT,
	.pcs_ops		= &rtpcs_838x_pcs_ops,
	.sds_ops		= &rtpcs_838x_sds_ops,
	.phy_page		= PAGE_FIB,
	.sds_hw_mode_vals	= rtpcs_838x_sds_hw_mode_vals,
	.init			= rtpcs_838x_init,
	.sds_probe		= rtpcs_838x_sds_probe,
};

static const struct phylink_pcs_ops rtpcs_839x_pcs_ops = {
	.pcs_an_restart		= rtpcs_pcs_an_restart,
	.pcs_config		= rtpcs_pcs_config,
	.pcs_get_state		= rtpcs_pcs_get_state,
};

static const struct rtpcs_sds_ops rtpcs_839x_sds_ops = {
	.read			= rtpcs_generic_sds_op_read,
	.write			= rtpcs_generic_sds_op_write,
	.write_mask		= rtpcs_generic_sds_op_write_mask,
	.set_autoneg		= rtpcs_generic_sds_set_autoneg,
	.restart_autoneg	= rtpcs_generic_sds_restart_autoneg,
	.deactivate		= rtpcs_839x_sds_deactivate,
	.activate		= rtpcs_839x_sds_activate,
	.config_hw_mode		= rtpcs_839x_sds_config_hw_mode,
	.set_hw_mode		= rtpcs_839x_sds_set_mode,
};

static const struct rtpcs_config rtpcs_839x_cfg = {
	.cpu_port		= RTPCS_839X_CPU_PORT,
	.mac_link_dup_sts	= RTPCS_839X_MAC_LINK_DUP_STS,
	.mac_link_spd_sts	= RTPCS_839X_MAC_LINK_SPD_STS,
	.mac_link_spd_bits	= RTPCS_83XX_MAC_LINK_SPD_BITS,
	.mac_link_sts		= RTPCS_839X_MAC_LINK_STS,
	.mac_rx_pause_sts	= RTPCS_839X_MAC_RX_PAUSE_STS,
	.mac_tx_pause_sts	= RTPCS_839X_MAC_TX_PAUSE_STS,
	.serdes_count		= RTPCS_839X_SERDES_CNT,
	.pcs_ops		= &rtpcs_839x_pcs_ops,
	.sds_ops		= &rtpcs_839x_sds_ops,
	.phy_page		= PAGE_FIB,
	.sds_hw_mode_vals	= rtpcs_839x_sds_hw_mode_vals,
	.init			= rtpcs_839x_init,
	.sds_probe		= rtpcs_839x_sds_probe,
};

static const struct phylink_pcs_ops rtpcs_930x_pcs_ops = {
	.pcs_an_restart		= rtpcs_pcs_an_restart,
	.pcs_config		= rtpcs_pcs_config,
	.pcs_get_state		= rtpcs_pcs_get_state,
};

static const struct rtpcs_sds_ops rtpcs_930x_sds_ops = {
	.read			= rtpcs_930x_sds_op_read,
	.write			= rtpcs_930x_sds_op_write,
	.write_mask		= rtpcs_930x_sds_op_write_mask,
	.xsg_write		= rtpcs_930x_sds_op_xsg_write,
	.set_autoneg		= rtpcs_93xx_sds_set_autoneg,
	.restart_autoneg	= rtpcs_generic_sds_restart_autoneg,
	.get_pll_select		= rtpcs_930x_sds_get_pll_select,
	.set_pll_select		= rtpcs_930x_sds_set_pll_select,
	.reset_cmu		= rtpcs_930x_sds_reset_cmu,
	.reconfigure_to_pll	= rtpcs_930x_sds_reconfigure_to_pll,
	.config_polarity	= rtpcs_930x_sds_config_polarity,
	.deactivate		= rtpcs_930x_sds_deactivate,
	.activate		= rtpcs_930x_sds_activate,
	.config_hw_mode		= rtpcs_930x_sds_config_hw_mode,
	.set_hw_mode		= rtpcs_930x_sds_set_mode,
	.config_attachment	= rtpcs_930x_sds_config_attachment,
	.post_config		= rtpcs_930x_sds_post_config,
};

static const struct rtpcs_config rtpcs_930x_cfg = {
	.cpu_port		= RTPCS_930X_CPU_PORT,
	.mac_link_dup_sts	= RTPCS_930X_MAC_LINK_DUP_STS,
	.mac_link_spd_sts	= RTPCS_930X_MAC_LINK_SPD_STS,
	.mac_link_spd_bits	= RTPCS_93XX_MAC_LINK_SPD_BITS,
	.mac_link_sts		= RTPCS_930X_MAC_LINK_STS,
	.mac_rx_pause_sts	= RTPCS_930X_MAC_RX_PAUSE_STS,
	.mac_tx_pause_sts	= RTPCS_930X_MAC_TX_PAUSE_STS,
	.serdes_count		= RTPCS_930X_SERDES_CNT,
	.pcs_ops		= &rtpcs_930x_pcs_ops,
	.sds_ops		= &rtpcs_930x_sds_ops,
	.phy_page		= PAGE_FIB,
	.sds_hw_mode_vals	= rtpcs_93xx_sds_hw_mode_vals,
	.init			= rtpcs_93xx_init,
	.sds_probe		= rtpcs_930x_sds_probe,
};

static const struct phylink_pcs_ops rtpcs_931x_pcs_ops = {
	.pcs_an_restart		= rtpcs_pcs_an_restart,
	.pcs_config		= rtpcs_pcs_config,
	.pcs_get_state		= rtpcs_pcs_get_state,
};

static const struct rtpcs_sds_ops rtpcs_931x_sds_ops = {
	.read			= rtpcs_generic_sds_op_read,
	.write			= rtpcs_generic_sds_op_write,
	.write_mask		= rtpcs_generic_sds_op_write_mask,
	.xsg_write		= rtpcs_931x_sds_op_xsg_write,
	.set_autoneg		= rtpcs_93xx_sds_set_autoneg,
	.restart_autoneg	= rtpcs_generic_sds_restart_autoneg,
	.get_pll_select		= rtpcs_931x_sds_get_pll_select,
	.set_pll_select		= rtpcs_931x_sds_set_pll_select,
	.reconfigure_to_pll	= rtpcs_931x_sds_reconfigure_to_pll,
	.config_polarity	= rtpcs_931x_sds_config_polarity,
	.deactivate		= rtpcs_931x_sds_deactivate,
	.activate		= rtpcs_931x_sds_activate,
	.config_hw_mode		= rtpcs_931x_sds_config_hw_mode,
	.set_hw_mode		= rtpcs_931x_sds_set_mode,
	.config_attachment	= rtpcs_931x_sds_config_attachment,
	.post_config		= rtpcs_931x_sds_post_config,
};

static const struct rtpcs_config rtpcs_931x_cfg = {
	.cpu_port		= RTPCS_931X_CPU_PORT,
	.mac_link_dup_sts	= RTPCS_931X_MAC_LINK_DUP_STS,
	.mac_link_spd_sts	= RTPCS_931X_MAC_LINK_SPD_STS,
	.mac_link_spd_bits	= RTPCS_93XX_MAC_LINK_SPD_BITS,
	.mac_link_sts		= RTPCS_931X_MAC_LINK_STS,
	.mac_rx_pause_sts	= RTPCS_931X_MAC_RX_PAUSE_STS,
	.mac_tx_pause_sts	= RTPCS_931X_MAC_TX_PAUSE_STS,
	.serdes_count		= RTPCS_931X_SERDES_CNT,
	.pcs_ops		= &rtpcs_931x_pcs_ops,
	.sds_ops		= &rtpcs_931x_sds_ops,
	.phy_page		= DIGI_1(PAGE_FIB),
	.sds_hw_mode_vals	= rtpcs_93xx_sds_hw_mode_vals,
	.init			= rtpcs_931x_init,
	.sds_probe		= rtpcs_931x_sds_probe,
};

static const struct of_device_id rtpcs_of_match[] = {
	{
		.compatible = "realtek,rtl8380-pcs",
		.data = &rtpcs_838x_cfg,
	},
	{
		.compatible = "realtek,rtl8392-pcs",
		.data = &rtpcs_839x_cfg,
	},
	{
		.compatible = "realtek,rtl9301-pcs",
		.data = &rtpcs_930x_cfg,
	},
	{
		.compatible = "realtek,rtl9311-pcs",
		.data = &rtpcs_931x_cfg,
	},
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, rtpcs_of_match);

static struct platform_driver rtpcs_driver = {
	.driver = {
		.name = "realtek-otto-pcs",
		.of_match_table = rtpcs_of_match
	},
	.probe = rtpcs_probe,
};
module_platform_driver(rtpcs_driver);

MODULE_AUTHOR("Markus Stockhausen <markus.stockhausen@gmx.de>");
MODULE_AUTHOR("Jonas Jelonek <jelonek.jonas@gmail.com>");
MODULE_DESCRIPTION("Realtek Otto SerDes PCS driver");
MODULE_LICENSE("GPL v2");
