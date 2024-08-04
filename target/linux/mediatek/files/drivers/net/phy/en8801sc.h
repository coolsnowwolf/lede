// SPDX-License-Identifier: GPL-2.0
/* FILE NAME:  en8801sc.h
 * PURPOSE:
 *      Define EN8801SC driver function
 *
 * NOTES:
 *
 */

#ifndef __EN8801SC_H
#define __EN8801SC_H

/* NAMING DECLARATIONS
 */
#define EN8801S_DRIVER_VERSION  "1.1.8_Generic"
#define EN8801S_PBUS_DEFAULT_ADDR 0x1e
#define EN8801S_PHY_DEFAULT_ADDR 0x1d
#define EN8801S_RG_ETHER_PHY_OUI 0x19a4
#define EN8801S_RG_SMI_ADDR      0x19a8
#define EN8801S_RG_BUCK_CTL      0x1a20
#define EN8801S_RG_LTR_CTL       0x0cf8
#define EN8801S_RG_PROD_VER      0x18e0

#define EN8801S_PBUS_OUI        0x17a5
#define EN8801S_PHY_ID1         0x03a2
#define EN8801S_PHY_ID2         0x9461
#define EN8801SC_PHY_ID         0x03a29471

#define LED_ON_CTRL(i)              (0x024 + ((i)*2))
#define LED_ON_EN                   (1 << 15)
#define LED_ON_POL                  (1 << 14)
#define LED_ON_EVT_MASK             (0x7f)
/* LED ON Event Option.B */
#define LED_ON_EVT_FORCE            (1 << 6)
#define LED_ON_EVT_LINK_DOWN        (1 << 3)
#define LED_ON_EVT_LINK_10M         (1 << 2)
#define LED_ON_EVT_LINK_100M        (1 << 1)
#define LED_ON_EVT_LINK_1000M       (1 << 0)
/* LED ON Event Option.E */

#define LED_BLK_CTRL(i)             (0x025 + ((i)*2))
#define LED_BLK_EVT_MASK            (0x3ff)
/* LED Blinking Event Option.B*/
#define LED_BLK_EVT_FORCE           (1 << 9)
#define LED_BLK_EVT_10M_RX_ACT      (1 << 5)
#define LED_BLK_EVT_10M_TX_ACT      (1 << 4)
#define LED_BLK_EVT_100M_RX_ACT     (1 << 3)
#define LED_BLK_EVT_100M_TX_ACT     (1 << 2)
#define LED_BLK_EVT_1000M_RX_ACT    (1 << 1)
#define LED_BLK_EVT_1000M_TX_ACT    (1 << 0)
/* LED Blinking Event Option.E*/
#define LED_ENABLE                  1
#define LED_DISABLE                 0

#define LINK_UP                 1
#define LINK_DOWN               0

/*
SFP Sample for verification
Tx Reverse, Rx Reverse
*/
#define EN8801S_TX_POLARITY_NORMAL   0x0
#define EN8801S_TX_POLARITY_REVERSE  0x1

#define EN8801S_RX_POLARITY_NORMAL   (0x1 << 1)
#define EN8801S_RX_POLARITY_REVERSE  (0x0 << 1)

/*
The following led_cfg example is for reference only.
LED5 1000M/LINK/ACT  (GPIO5)  <-> BASE_T_LED0,
LED6 10/100M/LINK/ACT(GPIO9)  <-> BASE_T_LED1,
LED4 100M/LINK/ACT   (GPIO8)  <-> BASE_T_LED2,
*/
/* User-defined.B */
#define BASE_T_LED0_ON_CFG      (LED_ON_EVT_LINK_1000M)
#define BASE_T_LED0_BLK_CFG \
			(LED_BLK_EVT_1000M_TX_ACT | \
			LED_BLK_EVT_1000M_RX_ACT)
#define BASE_T_LED1_ON_CFG \
			(LED_ON_EVT_LINK_100M | \
			 LED_ON_EVT_LINK_10M)
#define BASE_T_LED1_BLK_CFG \
			(LED_BLK_EVT_100M_TX_ACT | \
			 LED_BLK_EVT_100M_RX_ACT | \
			 LED_BLK_EVT_10M_TX_ACT | \
			 LED_BLK_EVT_10M_RX_ACT)
#define BASE_T_LED2_ON_CFG \
			(LED_ON_EVT_LINK_100M)
#define BASE_T_LED2_BLK_CFG \
			(LED_BLK_EVT_100M_TX_ACT | \
			 LED_BLK_EVT_100M_RX_ACT)
#define BASE_T_LED3_ON_CFG      (0x0)
#define BASE_T_LED3_BLK_CFG     (0x0)
/* User-defined.E */

#define EN8801S_LED_COUNT       4

#define MAX_RETRY               5
#define MAX_OUI_CHECK           2
/* CL45 MDIO control */
#define MII_MMD_ACC_CTL_REG     0x0d
#define MII_MMD_ADDR_DATA_REG   0x0e
#define MMD_OP_MODE_DATA        BIT(14)

#define MAX_TRG_COUNTER         5

/* CL22 Reg Support Page Select */
#define RgAddr_Reg1Fh        0x1f
#define CL22_Page_Reg        0x0000
#define CL22_Page_ExtReg     0x0001
#define CL22_Page_MiscReg    0x0002
#define CL22_Page_LpiReg     0x0003
#define CL22_Page_tReg       0x02A3
#define CL22_Page_TrReg      0x52B5

/* CL45 Reg Support DEVID */
#define DEVID_03             0x03
#define DEVID_07             0x07
#define DEVID_1E             0x1E
#define DEVID_1F             0x1F

/* TokenRing Reg Access */
#define TrReg_PKT_XMT_STA    0x8000
#define TrReg_WR             0x8000
#define TrReg_RD             0xA000

#define RgAddr_LPI_1Ch       0x1c
#define RgAddr_AUXILIARY_1Dh 0x1d
#define RgAddr_PMA_00h       0x0f80
#define RgAddr_PMA_01h       0x0f82
#define RgAddr_PMA_17h       0x0fae
#define RgAddr_PMA_18h       0x0fb0
#define RgAddr_DSPF_03h      0x1686
#define RgAddr_DSPF_06h      0x168c
#define RgAddr_DSPF_08h      0x1690
#define RgAddr_DSPF_0Ch      0x1698
#define RgAddr_DSPF_0Dh      0x169a
#define RgAddr_DSPF_0Fh      0x169e
#define RgAddr_DSPF_10h      0x16a0
#define RgAddr_DSPF_11h      0x16a2
#define RgAddr_DSPF_13h      0x16a6
#define RgAddr_DSPF_14h      0x16a8
#define RgAddr_DSPF_1Bh      0x16b6
#define RgAddr_DSPF_1Ch      0x16b8
#define RgAddr_TR_26h        0x0ecc
#define RgAddr_R1000DEC_15h  0x03aa
#define RgAddr_R1000DEC_17h  0x03ae

#define LED_BCR                     (0x021)
#define LED_BCR_EXT_CTRL            (1 << 15)
#define LED_BCR_CLK_EN              (1 << 3)
#define LED_BCR_TIME_TEST           (1 << 2)
#define LED_BCR_MODE_MASK           (3)
#define LED_BCR_MODE_DISABLE        (0)

#define LED_ON_DUR                  (0x022)
#define LED_ON_DUR_MASK             (0xffff)

#define LED_BLK_DUR                 (0x023)
#define LED_BLK_DUR_MASK            (0xffff)

#define LED_GPIO_SEL_MASK 0x7FFFFFF

#define UNIT_LED_BLINK_DURATION     1024

/* Invalid data */
#define INVALID_DATA            0xffffffff

#define LED_SET_GPIO_SEL(gpio, led, val)             \
			(val |= (led << (8 * (gpio % 4))))       \

#define GET_BIT(val, bit) ((val & BIT(bit)) >> bit)
/* DATA TYPE DECLARATIONS
 */
struct AIR_BASE_T_LED_CFG_S {
	u16 en;
	u16 gpio;
	u16 pol;
	u16 on_cfg;
	u16 blk_cfg;
};

union gephy_all_REG_LpiReg1Ch {
	struct {
		/* b[15:00] */
		u16 smi_deton_wt                             : 3;
		u16 smi_det_mdi_inv                          : 1;
		u16 smi_detoff_wt                            : 3;
		u16 smi_sigdet_debouncing_en                 : 1;
		u16 smi_deton_th                             : 6;
		u16 rsv_14                                   : 2;
	} DataBitField;
	u16 DATA;
};

union gephy_all_REG_dev1Eh_reg324h {
	struct {
		/* b[15:00] */
		u16 rg_smi_detcnt_max                        : 6;
		u16 rsv_6                                    : 2;
		u16 rg_smi_det_max_en                        : 1;
		u16 smi_det_deglitch_off                     : 1;
		u16 rsv_10                                   : 6;
	} DataBitField;
	u16 DATA;
};

union gephy_all_REG_dev1Eh_reg012h {
	struct {
		/* b[15:00] */
		u16 da_tx_i2mpb_a_tbt                        : 6;
		u16 rsv_6                                    : 4;
		u16 da_tx_i2mpb_a_gbe                        : 6;
	} DataBitField;
	u16 DATA;
};

union gephy_all_REG_dev1Eh_reg017h {
	struct {
		/* b[15:00] */
		u16 da_tx_i2mpb_b_tbt                        : 6;
		u16 rsv_6                                    : 2;
		u16 da_tx_i2mpb_b_gbe                        : 6;
		u16 rsv_14                                   : 2;
	} DataBitField;
	u16 DATA;
};

enum {
	AIR_LED_BLK_DUR_32M,
	AIR_LED_BLK_DUR_64M,
	AIR_LED_BLK_DUR_128M,
	AIR_LED_BLK_DUR_256M,
	AIR_LED_BLK_DUR_512M,
	AIR_LED_BLK_DUR_1024M,
	AIR_LED_BLK_DUR_LAST
};

enum {
	AIR_ACTIVE_LOW,
	AIR_ACTIVE_HIGH,
};

enum {
	AIR_LED_MODE_DISABLE,
	AIR_LED_MODE_USER_DEFINE,
	AIR_LED_MODE_LAST
};

#endif /* End of __EN8801SC_H */
