// SPDX-License-Identifier: GPL-2.0

#define SYNOPSYS_XPCS_ID		0x7996ced0
#define SYNOPSYS_XPCS_MASK		0xffffffff

/* Vendor regs access */
#define DW_VENDOR			BIT(15)

/* VR_XS_PCS */
#define DW_USXGMII_RST			BIT(10)
#define DW_USXGMII_EN			BIT(9)
#define DW_VR_XS_PCS_DIG_STS		0x0010
#define DW_RXFIFO_ERR			GENMASK(6, 5)

/* SR_MII */
#define DW_USXGMII_FULL			BIT(8)
#define DW_USXGMII_SS_MASK		(BIT(13) | BIT(6) | BIT(5))
#define DW_USXGMII_10000		(BIT(13) | BIT(6))
#define DW_USXGMII_5000			(BIT(13) | BIT(5))
#define DW_USXGMII_2500			(BIT(5))
#define DW_USXGMII_1000			(BIT(6))
#define DW_USXGMII_100			(BIT(13))
#define DW_USXGMII_10			(0)

/* SR_AN */
#define DW_SR_AN_ADV1			0x10
#define DW_SR_AN_ADV2			0x11
#define DW_SR_AN_ADV3			0x12
#define DW_SR_AN_LP_ABL1		0x13
#define DW_SR_AN_LP_ABL2		0x14
#define DW_SR_AN_LP_ABL3		0x15

/* Clause 73 Defines */
/* AN_LP_ABL1 */
#define DW_C73_PAUSE			BIT(10)
#define DW_C73_ASYM_PAUSE		BIT(11)
#define DW_C73_AN_ADV_SF		0x1
/* AN_LP_ABL2 */
#define DW_C73_1000KX			BIT(5)
#define DW_C73_10000KX4			BIT(6)
#define DW_C73_10000KR			BIT(7)
/* AN_LP_ABL3 */
#define DW_C73_2500KX			BIT(0)
#define DW_C73_5000KR			BIT(1)

/* Clause 37 Defines */
/* VR MII MMD registers offsets */
#define DW_VR_MII_MMD_CTRL		0x0000
#define DW_VR_MII_DIG_CTRL1		0x8000
#define DW_VR_MII_AN_CTRL		0x8001
#define DW_VR_MII_AN_INTR_STS		0x8002
#define DW_VR_MII_DBG_CTRL		0x8005
#define DW_VR_MII_LINK_TIMER_CTRL	0x800a
#define DW_VR_MII_DIG_STS       0x8010
#define DW_VR_MII_MP_6G_RXGENCTRL0  0x8058
#define DW_VR_MII_MP_6G_MPLL_CTRL0  0x8078
#define DW_VR_MII_MP_6G_MPLL_CTRL1  0x8079
#define DW_VR_MII_MP_6G_MISC_CTRL1  0x809a
/* Enable 2.5G Mode */
#define DW_VR_MII_DIG_CTRL1_2G5_EN	BIT(2)
/* EEE Mode Control Register */
#define DW_VR_MII_EEE_MCTRL0		0x8006
#define DW_VR_MII_EEE_MCTRL1		0x800b
#define DW_VR_MII_DIG_CTRL2		0x80e1

/* VR_MII_DIG_CTRL1 */
#define DW_VR_MII_DIG_CTRL1_EN_25G_MODE		BIT(2)
#define DW_VR_MII_DIG_CTRL1_CL37_TMR_OVR_RIDE	BIT(3)
#define DW_VR_MII_DIG_CTRL1_MAC_AUTO_SW		BIT(9)

/* VR_MII_DIG_CTRL2 */
#define DW_VR_MII_DIG_CTRL2_TX_POL_INV		BIT(4)
#define DW_VR_MII_DIG_CTRL2_RX_POL_INV		BIT(0)

/* VR_MII_AN_CTRL */
#define DW_VR_MII_AN_CTRL_TX_CONFIG_SHIFT	3
#define DW_VR_MII_TX_CONFIG_MASK		BIT(3)
#define DW_VR_MII_TX_CONFIG_PHY_SIDE_SGMII	0x1
#define DW_VR_MII_TX_CONFIG_MAC_SIDE_SGMII	0x0
#define DW_VR_MII_AN_CTRL_PCS_MODE_SHIFT	1
#define DW_VR_MII_PCS_MODE_MASK			GENMASK(2, 1)
#define DW_VR_MII_PCS_MODE_C37_1000BASEX	0x0
#define DW_VR_MII_PCS_MODE_C37_SGMII		0x2
#define DW_VR_MII_PCS_MODE_C37_QSGMII		0x3
#define DW_VR_MII_AN_INTR_EN			BIT(0)

/* VR_MII_AN_INTR_STS */
#define DW_VR_MII_AN_STS_C37_ANSGM_FD		BIT(1)
#define DW_VR_MII_AN_STS_C37_ANSGM_SP_SHIFT	2
#define DW_VR_MII_AN_STS_C37_ANSGM_SP		GENMASK(3, 2)
#define DW_VR_MII_C37_ANSGM_SP_10		0x0
#define DW_VR_MII_C37_ANSGM_SP_100		0x1
#define DW_VR_MII_C37_ANSGM_SP_1000		0x2
#define DW_VR_MII_C37_ANSGM_SP_LNKSTS		BIT(4)
#define DW_VR_MII_C37_ANCMPLT_INTR		BIT(0)

/* VR_MII_LINK_TIMER_CTRL */
#define DW_VR_MII_LINK_TIMER_2500BASEX		0x2faf

/* VR_MII_DIG_STS */
#define PSEQ_STATE              GENMASK(4, 2)
#define PSEQ_STATE_GOOD             4
#define PSEQ_STATE_DOWN             6

/* VR_MII_MP_6G_MPLL_CTRL0 */
#define LANE_10BIT_SEL              BIT(1)

/* VR_MII_MP_6G_RXGENCTRL0 */
#define RX_ALIGN_EN_0               BIT(4)

/* SR MII MMD Control defines */
#define AN_CL37_EN			BIT(12)	/* Enable Clause 37 auto-nego */
#define SGMII_SPEED_SS13		BIT(13)	/* SGMII speed along with SS6 */
#define SGMII_SPEED_SS6			BIT(6)	/* SGMII speed along with SS13 */

/* VR MII EEE Control 0 defines */
#define DW_VR_MII_EEE_LTX_EN			BIT(0)  /* LPI Tx Enable */
#define DW_VR_MII_EEE_LRX_EN			BIT(1)  /* LPI Rx Enable */
#define DW_VR_MII_EEE_TX_QUIET_EN		BIT(2)  /* Tx Quiet Enable */
#define DW_VR_MII_EEE_RX_QUIET_EN		BIT(3)  /* Rx Quiet Enable */
#define DW_VR_MII_EEE_TX_EN_CTRL		BIT(4)  /* Tx Control Enable */
#define DW_VR_MII_EEE_RX_EN_CTRL		BIT(7)  /* Rx Control Enable */

#define DW_VR_MII_EEE_MULT_FACT_100NS_SHIFT	8
#define DW_VR_MII_EEE_MULT_FACT_100NS		GENMASK(11, 8)

/* VR MII EEE Control 1 defines */
#define DW_VR_MII_EEE_TRN_LPI		BIT(0)	/* Transparent Mode Enable */

/* Additional MMDs for QSGMII */
#define DW_QSGMII_MMD1			0x1a
#define DW_QSGMII_MMD2			0x1b
#define DW_QSGMII_MMD3			0x1c

/* PMA MMD registers */
#define XS_PMA_MMD_BaseAddress 0x8020
#define VR_XS_PMA_RX_LSTS (XS_PMA_MMD_BaseAddress + 0x0)
#define VR_XS_PMA_RX_LSTS_RegisterResetValue 0x0
#define VR_XS_PMA_MP_12G_16G_25G_TX_GENCTRL0 (XS_PMA_MMD_BaseAddress + 0x10)
#define VR_XS_PMA_MP_12G_16G_25G_TX_GENCTRL0_RegisterResetValue 0x1000
#define VR_XS_PMA_MP_12G_16G_25G_TX_GENCTRL1 (XS_PMA_MMD_BaseAddress + 0x11)
#define VR_XS_PMA_MP_12G_16G_25G_TX_GENCTRL1_RegisterResetValue 0x1510
#define VR_XS_PMA_MP_12G_16G_TX_GENCTRL2 (XS_PMA_MMD_BaseAddress + 0x12)
#define VR_XS_PMA_MP_12G_16G_TX_GENCTRL2_RegisterResetValue 0x300
#define VR_XS_PMA_MP_12G_16G_25G_TX_BOOST_CTRL (XS_PMA_MMD_BaseAddress + 0x13)
#define VR_XS_PMA_MP_12G_16G_25G_TX_BOOST_CTRL_RegisterResetValue 0xf
#define VR_XS_PMA_MP_12G_16G_25G_TX_RATE_CTRL (XS_PMA_MMD_BaseAddress + 0x14)
#define VR_XS_PMA_MP_12G_16G_25G_TX_RATE_CTRL_RegisterResetValue 0x0
#define VR_XS_PMA_MP_12G_16G_25G_TX_POWER_STATE_CTRL (XS_PMA_MMD_BaseAddress + 0x15)
#define VR_XS_PMA_MP_12G_16G_25G_TX_POWER_STATE_CTRL_RegisterResetValue 0x0
#define VR_XS_PMA_MP_12G_16G_25G_TX_EQ_CTRL0 (XS_PMA_MMD_BaseAddress + 0x16)
#define VR_XS_PMA_MP_12G_16G_25G_TX_EQ_CTRL0_RegisterResetValue 0x2800
#define VR_XS_PMA_MP_12G_16G_25G_TX_EQ_CTRL1 (XS_PMA_MMD_BaseAddress + 0x17)
#define VR_XS_PMA_MP_12G_16G_25G_TX_EQ_CTRL1_RegisterResetValue 0x0
#define VR_XS_PMA_MP_16G_25G_TX_GENCTRL3 (XS_PMA_MMD_BaseAddress + 0x1c)
#define VR_XS_PMA_MP_16G_25G_TX_GENCTRL3_RegisterResetValue 0x0
#define VR_XS_PMA_MP_16G_25G_TX_GENCTRL4 (XS_PMA_MMD_BaseAddress + 0x1d)
#define VR_XS_PMA_MP_16G_25G_TX_GENCTRL4_RegisterResetValue 0x0
#define VR_XS_PMA_MP_16G_25G_TX_MISC_CTRL0 (XS_PMA_MMD_BaseAddress + 0x1e)
#define VR_XS_PMA_MP_16G_25G_TX_MISC_CTRL0_RegisterResetValue 0x0
#define VR_XS_PMA_MP_12G_16G_25G_TX_STS (XS_PMA_MMD_BaseAddress + 0x20)
#define VR_XS_PMA_MP_12G_16G_25G_TX_STS_RegisterResetValue 0x0
#define VR_XS_PMA_MP_12G_16G_25G_RX_GENCTRL0 (XS_PMA_MMD_BaseAddress + 0x30)
#define VR_XS_PMA_MP_12G_16G_25G_RX_GENCTRL0_RegisterResetValue 0x101
#define VR_XS_PMA_MP_12G_16G_25G_RX_GENCTRL1 (XS_PMA_MMD_BaseAddress + 0x31)
#define VR_XS_PMA_MP_12G_16G_25G_RX_GENCTRL1_RegisterResetValue 0x1100
#define VR_XS_PMA_MP_12G_16G_RX_GENCTRL2 (XS_PMA_MMD_BaseAddress + 0x32)
#define VR_XS_PMA_MP_12G_16G_RX_GENCTRL2_RegisterResetValue 0x300
#define VR_XS_PMA_MP_12G_16G_RX_GENCTRL3 (XS_PMA_MMD_BaseAddress + 0x33)
#define VR_XS_PMA_MP_12G_16G_RX_GENCTRL3_RegisterResetValue 0x1
#define VR_XS_PMA_MP_12G_16G_25G_RX_RATE_CTRL (XS_PMA_MMD_BaseAddress + 0x34)
#define VR_XS_PMA_MP_12G_16G_25G_RX_RATE_CTRL_RegisterResetValue 0x0
#define VR_XS_PMA_MP_12G_16G_25G_RX_POWER_STATE_CTRL (XS_PMA_MMD_BaseAddress + 0x35)
#define VR_XS_PMA_MP_12G_16G_25G_RX_POWER_STATE_CTRL_RegisterResetValue 0x0
#define VR_XS_PMA_MP_12G_16G_25G_RX_CDR_CTRL (XS_PMA_MMD_BaseAddress + 0x36)
#define VR_XS_PMA_MP_12G_16G_25G_RX_CDR_CTRL_RegisterResetValue 0x0
#define VR_XS_PMA_MP_12G_16G_25G_RX_ATTN_CTRL (XS_PMA_MMD_BaseAddress + 0x37)
#define VR_XS_PMA_MP_12G_16G_25G_RX_ATTN_CTRL_RegisterResetValue 0x0
#define VR_XS_PMA_MP_16G_25G_RX_EQ_CTRL0 (XS_PMA_MMD_BaseAddress + 0x38)
#define VR_XS_PMA_MP_16G_25G_RX_EQ_CTRL0_RegisterResetValue 0x5550
#define VR_XS_PMA_MP_12G_16G_25G_RX_EQ_CTRL4 (XS_PMA_MMD_BaseAddress + 0x3c)
#define VR_XS_PMA_MP_12G_16G_25G_RX_EQ_CTRL4_RegisterResetValue 0x11
#define VR_XS_PMA_MP_16G_25G_RX_EQ_CTRL5 (XS_PMA_MMD_BaseAddress + 0x3d)
#define VR_XS_PMA_MP_16G_25G_RX_EQ_CTRL5_RegisterResetValue 0x30
#define VR_XS_PMA_MP_12G_16G_25G_DFE_TAP_CTRL0 (XS_PMA_MMD_BaseAddress + 0x3e)
#define VR_XS_PMA_MP_12G_16G_25G_DFE_TAP_CTRL0_RegisterResetValue 0x0
#define VR_XS_PMA_MP_12G_16G_25G_RX_STS (XS_PMA_MMD_BaseAddress + 0x40)
#define VR_XS_PMA_MP_12G_16G_25G_RX_STS_RegisterResetValue 0x0
#define VR_XS_PMA_MP_16G_25G_RX_PPM_STS0 (XS_PMA_MMD_BaseAddress + 0x41)
#define VR_XS_PMA_MP_16G_25G_RX_PPM_STS0_RegisterResetValue 0x0
#define VR_XS_PMA_MP_16G_RX_CDR_CTRL1 (XS_PMA_MMD_BaseAddress + 0x44)
#define VR_XS_PMA_MP_16G_RX_CDR_CTRL1_RegisterResetValue 0x111
#define VR_XS_PMA_MP_16G_25G_RX_PPM_CTRL0 (XS_PMA_MMD_BaseAddress + 0x45)
#define VR_XS_PMA_MP_16G_25G_RX_PPM_CTRL0_RegisterResetValue 0x12
#define VR_XS_PMA_MP_16G_25G_RX_GENCTRL4 (XS_PMA_MMD_BaseAddress + 0x48)
#define VR_XS_PMA_MP_16G_25G_RX_GENCTRL4_RegisterResetValue 0x0
#define VR_XS_PMA_MP_16G_25G_RX_MISC_CTRL0 (XS_PMA_MMD_BaseAddress + 0x49)
#define VR_XS_PMA_MP_16G_25G_RX_MISC_CTRL0_RegisterResetValue 0x12
#define VR_XS_PMA_MP_16G_25G_RX_IQ_CTRL0 (XS_PMA_MMD_BaseAddress + 0x4b)
#define VR_XS_PMA_MP_16G_25G_RX_IQ_CTRL0_RegisterResetValue 0x0
#define VR_XS_PMA_MP_12G_16G_25G_MPLL_CMN_CTRL (XS_PMA_MMD_BaseAddress + 0x50)
#define VR_XS_PMA_MP_12G_16G_25G_MPLL_CMN_CTRL_RegisterResetValue 0x1
#define VR_XS_PMA_MP_12G_16G_MPLLA_CTRL0 (XS_PMA_MMD_BaseAddress + 0x51)
#define VR_XS_PMA_MP_12G_16G_MPLLA_CTRL0_RegisterResetValue 0x21
#define VR_XS_PMA_MP_16G_MPLLA_CTRL1 (XS_PMA_MMD_BaseAddress + 0x52)
#define VR_XS_PMA_MP_16G_MPLLA_CTRL1_RegisterResetValue 0x0
#define VR_XS_PMA_MP_12G_16G_MPLLA_CTRL2 (XS_PMA_MMD_BaseAddress + 0x53)
#define VR_XS_PMA_MP_12G_16G_MPLLA_CTRL2_RegisterResetValue 0x600
#define VR_XS_PMA_MP_12G_16G_MPLLB_CTRL0 (XS_PMA_MMD_BaseAddress + 0x54)
#define VR_XS_PMA_MP_12G_16G_MPLLB_CTRL0_RegisterResetValue 0x8000
#define VR_XS_PMA_MP_16G_MPLLB_CTRL1 (XS_PMA_MMD_BaseAddress + 0x55)
#define VR_XS_PMA_MP_16G_MPLLB_CTRL1_RegisterResetValue 0x0
#define VR_XS_PMA_MP_12G_16G_MPLLB_CTRL2 (XS_PMA_MMD_BaseAddress + 0x56)
#define VR_XS_PMA_MP_12G_16G_MPLLB_CTRL2_RegisterResetValue 0x0
#define VR_XS_PMA_MP_16G_MPLLA_CTRL3 (XS_PMA_MMD_BaseAddress + 0x57)
#define VR_XS_PMA_MP_16G_MPLLA_CTRL3_RegisterResetValue 0xa016
#define VR_XS_PMA_MP_16G_MPLLB_CTRL3 (XS_PMA_MMD_BaseAddress + 0x58)
#define VR_XS_PMA_MP_16G_MPLLB_CTRL3_RegisterResetValue 0x0
#define VR_XS_PMA_MP_16G_MPLLA_CTRL4 (XS_PMA_MMD_BaseAddress + 0x59)
#define VR_XS_PMA_MP_16G_MPLLA_CTRL4_RegisterResetValue 0x0
#define VR_XS_PMA_MP_16G_MPLLA_CTRL5 (XS_PMA_MMD_BaseAddress + 0x5a)
#define VR_XS_PMA_MP_16G_MPLLA_CTRL5_RegisterResetValue 0x0
#define VR_XS_PMA_MP_16G_MPLLB_CTRL4 (XS_PMA_MMD_BaseAddress + 0x5b)
#define VR_XS_PMA_MP_16G_MPLLB_CTRL4_RegisterResetValue 0x0
#define VR_XS_PMA_MP_16G_MPLLB_CTRL5 (XS_PMA_MMD_BaseAddress + 0x5c)
#define VR_XS_PMA_MP_16G_MPLLB_CTRL5_RegisterResetValue 0x0
#define VR_XS_PMA_MP_12G_16G_25G_MISC_CTRL0 (XS_PMA_MMD_BaseAddress + 0x70)
#define VR_XS_PMA_MP_12G_16G_25G_MISC_CTRL0_RegisterResetValue 0x5100
#define VR_XS_PMA_MP_12G_16G_25G_REF_CLK_CTRL (XS_PMA_MMD_BaseAddress + 0x71)
#define VR_XS_PMA_MP_12G_16G_25G_REF_CLK_CTRL_RegisterResetValue 0x71
#define VR_XS_PMA_MP_12G_16G_25G_REF_CLK_CTRL_REF_RPT_CLK_EN BIT(8)
#define VR_XS_PMA_MP_12G_16G_25G_VCO_CAL_LD0 (XS_PMA_MMD_BaseAddress + 0x72)
#define VR_XS_PMA_MP_12G_16G_25G_VCO_CAL_LD0_RegisterResetValue 0x549
#define VR_XS_PMA_MP_16G_25G_VCO_CAL_REF0 (XS_PMA_MMD_BaseAddress + 0x76)
#define VR_XS_PMA_MP_16G_25G_VCO_CAL_REF0_RegisterResetValue 0x29
#define VR_XS_PMA_MP_12G_16G_25G_MISC_STS (XS_PMA_MMD_BaseAddress + 0x78)
#define VR_XS_PMA_MP_12G_16G_25G_MISC_STS_RegisterResetValue 0x200
#define VR_XS_PMA_MP_12G_16G_25G_MISC_CTRL1 (XS_PMA_MMD_BaseAddress + 0x79)
#define VR_XS_PMA_MP_12G_16G_25G_MISC_CTRL1_RegisterResetValue 0xffff
#define VR_XS_PMA_MP_12G_16G_25G_EEE_CTRL (XS_PMA_MMD_BaseAddress + 0x7a)
#define VR_XS_PMA_MP_12G_16G_25G_EEE_CTRL_RegisterResetValue 0x4f
#define VR_XS_PMA_MP_12G_16G_25G_SRAM (XS_PMA_MMD_BaseAddress + 0x7b)
#define VR_XS_PMA_MP_12G_16G_25G_SRAM_INIT_DN BIT(0)
#define VR_XS_PMA_MP_12G_16G_25G_SRAM_EXT_LD_DN BIT(1)
#define VR_XS_PMA_MP_16G_25G_MISC_CTRL2 (XS_PMA_MMD_BaseAddress + 0x7c)
#define VR_XS_PMA_MP_16G_25G_MISC_CTRL2_RegisterResetValue 0x1
#define VR_XS_PMA_SNPS_CR_CTRL (XS_PMA_MMD_BaseAddress + 0x80)
#define VR_XS_PMA_SNPS_CR_CTRL_START_BUSY BIT(0)
#define VR_XS_PMA_SNPS_CR_CTRL_WR_RDN	BIT(1)
#define VR_XS_PMA_SNPS_CR_ADDR (XS_PMA_MMD_BaseAddress + 0x81)
#define VR_XS_PMA_SNPS_CR_DATA (XS_PMA_MMD_BaseAddress + 0x82)
