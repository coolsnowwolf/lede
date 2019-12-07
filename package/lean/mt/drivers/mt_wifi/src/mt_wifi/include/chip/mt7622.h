#ifndef __MT7622_H__
#define __MT7622_H__

#include "mcu/andes_core.h"
#include "phy/mt_rf.h"

struct _RTMP_ADAPTER;
struct _RTMP_CHIP_DBG;

#define MAX_RF_ID	127
#define MAC_RF_BANK 7

#define MT7622_MT_WTBL_SIZE	128
#define MT7622_MT_WMM_SIZE	4
#define MT7622_PDA_PORT		0xf800

#define MT7622_CT_PARSE_LEN	0

#define MT7622_RAM_BIN_FILE_NAME_E2 "WIFI_RAM_CODE_MT7622_E2.bin"

#define MT7622_ROM_PATCH_BIN_FILE_NAME_E2 "mt7622_patch_e2_hdr.bin"

#define MT7622_ROM_PATCH_START_ADDRESS	0x9C000

/* wilsonl */
/* */
/* Device ID & Vendor ID, these values should match EEPROM value */
/* */

#define OF_WBSYS_NAME "mediatek,wbsys"
#define RTMP_MAC_CSR_ADDR       0x18000000
#define RTMP_IRQ_NUM			244
#define RTMP_MAC_CSR_LEN		0x100000
#define RTMP_FLASH_BASE_ADDR    0xbc000000 /* wilsonl */

#define HIF_INTR_BASE	0x10000700
#define HIF_INTR_VALUE	0x2
#define HIF_REMAP_SIZE	0x10

#define EFUSE_ANTENNA_POLARITY_POS1         0x3C
#define EFUSE_ANTENNA_POLARITY_POS2         0x3D
#define EFUSE_ANTENNA_CONFIG_POS1           0x3E
#define EFUSE_ANTENNA_CONFIG_POS2           0x49
#define EFUSE_ANTENNA_COEX_TYPE_POS         0x48
#define EFUSE_ANTENNA_STREAMS_POS           0x42
#define ANT_STREAM_OFFSET                   0
#define FEM_PWR_SAV_OFFSET                  1
#define PA_LNA_CFG_OFFSET					2
#define COEX_TYPE_OFFSET                    4
#define SMART_ANT_OFFSET                    6
#define BT_PIP_OFFSET                       7
#define POLARITY_TRSW_N_OFFSET              0
#define POLARITY_TRSW_P_OFFSET              1
#define POLARITY_LNA_OFFSET					2
#define POLARITY_PA_OFFSET                  3
#define SMART_ANT_POLARITY_OFFSET           4
#define BT_PIP_POLARITY_OFFSET              5
#define VLIN_LINEARITY_OFFSET               6
#define SHARED_PIN_OFFSET                   7
#define GetTddMode(mode)                    ((mode & BITS(4, 5)) >> 4)
#define GetRxConcur(mode)                   ((mode & BITS(4, 5)) >> 4)
#define GetSharePinSupport(mode)           ((mode & BIT(7)) >> 7)
#define GetPIPMode(mode)                   ((mode & BIT(7)) >> 7)
#define GetSmartAntMode(mode)              ((mode & BIT(6)) >> 6)
#define GetGbandPaLna(mode)                ((mode & BITS(2, 3)) >> 2)
#define GetFEMPowerSave(mode)              ((mode & BIT(1)) >> 1)
#define GetStreams(mode)                   ((mode & BIT(0)) >> 0)
#define GetCoexMode(mode)                  ((mode & BITS(4, 5)) >> 4)
#define GetPIP(mode)                       ((mode & BIT(4)) >> 4)
#define GetCoex(mode)                      ((mode & BITS(0, 1)) >> 0)
#define GetType(mode)                      ((mode & BITS(2, 3)) >> 2)
#define GetSharedPin(mode)                 ((mode & BIT(3)) >> 3)
#define GetVLINPol(mode)		   ((mode & BIT(2)) >> 2)
#define GetPIPPol(mode)                    ((mode & BIT(1)) >> 1)
#define GetSAntpol(mode)                   ((mode & BIT(0)) >> 0)
#define GetPolPA(mode)                     ((mode & BIT(5)) >> 5)
#define GetPolLNA(mode)                    ((mode & BIT(4)) >> 4)
#define GetPolTRSWP(mode)                  ((mode & BIT(1)) >> 1)
#define GetPolTRSWN(mode)                  ((mode & BIT(0)) >> 0)

#ifdef COEX_SUPPORT
#define COEX_TYPE_FDD		                  0
#define COEX_TYPE_TDD_WITH_SWITCH             1
#define COEX_TYPE_TDD_WITHOUT_SWITCH          2
#define COEX_TYPE_RX_CONCURRENT               3
#define CONFIG_IPA_ILNA        0
#define CONFIG_IPA_ELNA        1
#define CONFIG_EPA_ELNA        2
#define CONFIG_EPA_ILNA        3
#define CONFIG_4x4             0x44
#define CONFIG_3x3             0x33
#define CONFIG_2x2             0x22
#define CONFIG_1x1             0x11
#define USED_PIN_WF0_PAPE        BIT(5)
#define USED_PIN_WF0_LNA_EN      BIT(8)
#define USED_PIN_WF0_TRSW_P      BIT(7)
#define USED_PIN_WF0_TRSW_N      BIT(4)
#define USED_PIN_WF1_PAPE        BIT(9)
#define USED_PIN_WF1_LNA_EN      BIT(6)
#define USED_PIN_WF1_TRSW_P      BIT(15)
#define USED_PIN_WF1_TRSW_N      BIT(17)
#define USED_PIN_WF2_PAPE        BIT(3)
#define USED_PIN_WF2_LNA_EN      BIT(2)
#define USED_PIN_WF2_TRSW_P      BIT(13)
#define USED_PIN_WF2_TRSW_N      BIT(12)
#define USED_PIN_WF3_PAPE        BIT(1)
#define USED_PIN_WF3_LNA_EN      BIT(0)
#define USED_PIN_WF3_TRSW_P      BIT(16)
#define USED_PIN_WF3_TRSW_N      BIT(14)
#define USED_PIN_BTWIFI_SW_P     BIT(18)
#define USED_PIN_BTWIFI_SW_N     BIT(19)
#define USED_PIN_SANT0           BIT(20)
#define USED_PIN_SANT1           BIT(21)
#define USED_PIN_SANT2           BIT(22)
#define USED_PIN_SANT3           BIT(23)
#define USED_PIN_SANT4           BIT(24)
#define USED_PIN_SANT5           BIT(25)
#define USED_PIN_SANT6           BIT(26)
#define USED_PIN_SANT7           BIT(27)
#define USED_PIN_SANT8           BIT(28)
#define USED_PIN_SANT9           BIT(29)
#define USED_PIN_SANT10          BIT(30)
#define USED_PIN_SANT11          BIT(31)
#define USED_PIN_SANT      (USED_PIN_SANT0 | USED_PIN_SANT1 | USED_PIN_SANT2 | USED_PIN_SANT3 | \
				USED_PIN_SANT4 | USED_PIN_SANT5 | \
				USED_PIN_SANT6 | USED_PIN_SANT7 | USED_PIN_SANT8 | \
				USED_PIN_SANT9 | USED_PIN_SANT10 | USED_PIN_SANT11)
#define USED_PIN_BTPIP0           BIT(18)
#define USED_PIN_BTPIP1           BIT(19)
#define USED_PIN_BTPIP2           BIT(20)
#define USED_PIN_BTPIP3           BIT(21)
#define USED_PIN_BTPIP4           BIT(22)
#define USED_PIN_BTPIP5           BIT(23)
#define USED_PIN_BTPIP6           BIT(24)
#define USED_PIN_BTPIP7           BIT(25)
#define USED_PIN_BT_PIP   (USED_PIN_BTPIP0 | USED_PIN_BTPIP1 | USED_PIN_BTPIP2 | USED_PIN_BTPIP3 | \
				USED_PIN_BTPIP4 | USED_PIN_BTPIP5 \
				| USED_PIN_BTPIP6 | USED_PIN_BTPIP7)
#define USED_PIN_TRSW_N     (BIT(4)|BIT(17)|BIT(12)|BIT(14))
#define USED_PIN_TRSW_P     (BIT(7)|BIT(15)|BIT(13)|BIT(16))
#define USED_PIN_LNA	    (BIT(8)|BIT(6)|BIT(2)|BIT(0))
#define USED_PIN_EPA	    (BIT(5)|BIT(9)|BIT(3)|BIT(1))
#define USED_PIN_BTWIFI_SW  (USED_PIN_BTWIFI_SW_P | USED_PIN_BTWIFI_SW_N)
#define USED_PIN_EPA_ELNA_4x4	(USED_PIN_WF3_PAPE | USED_PIN_WF3_LNA_EN)
#define USED_PIN_EPA_ILNA_4x4	(USED_PIN_WF3_PAPE)
#define USED_PIN_IPA_ELNA_4x4	(USED_PIN_WF3_LNA_EN)
#define USED_PIN_EPA_ELNA_3x3	(USED_PIN_WF2_PAPE | USED_PIN_WF2_LNA_EN)
#define USED_PIN_EPA_ILNA_3x3   (USED_PIN_WF2_PAPE)
#define USED_PIN_IPA_ELNA_3x3   (USED_PIN_WF2_LNA_EN)
#define USED_PIN_EPA_ELNA_2x2	(USED_PIN_WF1_PAPE | USED_PIN_WF1_LNA_EN)
#define USED_PIN_EPA_ILNA_2x2   (USED_PIN_WF1_PAPE)
#define USED_PIN_IPA_ELNA_2x2   (USED_PIN_WF1_LNA_EN)
#define USED_PIN_EPA_ELNA_1x1	(USED_PIN_WF0_PAPE | USED_PIN_WF0_LNA_EN)
#define USED_PIN_EPA_ILNA_1x1   (USED_PIN_WF0_PAPE)
#define USED_PIN_IPA_ELNA_1x1   (USED_PIN_WF0_LNA_EN)
#define WF0_PAPE      0x5
#define WF0_LNA_EN    0x4
#define WF0_TRSW_P    0x2
#define WF0_TRSW_N    0x3
#define WF1_PAPE      0x9
#define WF1_LNA_EN    0x8
#define WF1_TRSW_P    0x6
#define WF1_TRSW_N    0x7
#define WF2_PAPE      0xD
#define WF2_LNA_EN    0xC
#define WF2_TRSW_P    0xA
#define WF2_TRSW_N    0xB
#define WF3_PAPE      0x11
#define WF3_LNA_EN    0x10
#define WF3_TRSW_P    0xE
#define WF3_TRSW_N    0xF
#define WF3_LNA_EN_POL    0x25
#define WF2_LNA_EN_POL    0x24
#define WF1_LNA_EN_POL    0x23
#define WF0_LNA_EN_POL    0x22
#define GPIO_BASE_ADDR  0x10211000
#define PINMUX_FUCTION_ANTSEL               5
typedef enum _ENUM_ANT_SEL_PIN_T {
	ANT_SEL_0 = 0,
	ANT_SEL_1,
	ANT_SEL_2,
	ANT_SEL_3,
	ANT_SEL_4,
	ANT_SEL_5,
	ANT_SEL_6,
	ANT_SEL_7,
	ANT_SEL_8,
	ANT_SEL_9,
	ANT_SEL_10,
	ANT_SEL_11,
	ANT_SEL_12,
	ANT_SEL_13,
	ANT_SEL_14,
	ANT_SEL_15,
	ANT_SEL_16,
	ANT_SEL_17,
	ANT_SEL_18,
	ANT_SEL_19,
	ANT_SEL_20,
	ANT_SEL_21,
	ANT_SEL_22,
	ANT_SEL_23,
	ANT_SEL_24,
	ANT_SEL_25,
	ANT_SEL_26,
	ANT_SEL_27,
	ANT_SEL_28,
	ANT_SEL_29,
	ANT_SEL_PIN_NUM
} ENUM_ANT_SEL_PIN_T;
#define GPIO_GPIO_MODE1                    (0x00000310)
    #define GPIO21_MODE                     BITS(28, 31)   /* ANTSEL29 */
    #define GPIO21_MODE_OFFSET              28
    #define GPIO20_MODE                     BITS(24, 27)   /* ANTSEL28 */
    #define GPIO20_MODE_OFFSET              24
    #define GPIO19_MODE                     BITS(20, 23)   /* ANTSEL27 */
    #define GPIO19_MODE_OFFSET              20
    #define GPIO18_MODE                     BITS(16, 19)   /* ANTSEL26 */
    #define GPIO18_MODE_OFFSET              16
    #define GPIO76_MODE                     BITS(12, 15)   /* ANTSEL15 */
    #define GPIO76_MODE_OFFSET              12
    #define GPIO75_MODE                     BITS(8, 11)   /* ANTSEL14 */
    #define GPIO75_MODE_OFFSET              8
    #define GPIO74_MODE                     BITS(4,  7)   /* ANTSEL13 */
    #define GPIO74_MODE_OFFSET              4
    #define GPIO73_MODE                     BITS(0,  3)   /* ANTSEL12 */
    #define GPIO73_MODE_OFFSET              0
#define GPIO_GPIO_MODE2                    (0x00000320)
    #define GPIO77_MODE                     BITS(28, 31)   /* ANTSEL16 */
    #define GPIO77_MODE_OFFSET              28
    #define GPIO37_MODE                     BITS(24, 27)   /* ANTSEL25 */
    #define GPIO37_MODE_OFFSET              24
    #define GPIO36_MODE                     BITS(20, 23)   /* ANTSEL24 */
    #define GPIO36_MODE_OFFSET              20
    #define GPIO0_MODE                      BITS(16, 19)
    #define GPIO0_MODE_OFFSET               16
    #define GPIO78_MODE                     BITS(12, 15)   /* ANTSEL17 */
    #define GPIO78_MODE_OFFSET              12
    #define GPIO35_MODE                     BITS(8, 11)   /* ANTSEL23 */
    #define GPIO35_MODE_OFFSET              8
    #define GPIO34_MODE                     BITS(4,  7)   /* ANTSEL22 */
    #define GPIO34_MODE_OFFSET              4
    #define GPIO5_MODE                      BITS(0,  3)
    #define GPIO5_MODE_OFFSET               0
#define GPIO_GPIO_MODE8                    (0x00000380)
    #define GPIO97_MODE                     BITS(28, 31)   /* ANTSEL6 */
    #define GPIO97_MODE_OFFSET              28
    #define GPIO96_MODE                     BITS(24, 27)   /* ANTSEL5 */
    #define GPIO96_MODE_OFFSET              24
    #define GPIO95_MODE                     BITS(20, 23)   /* ANTSEL4 */
    #define GPIO95_MODE_OFFSET              20
    #define GPIO42_MODE                     BITS(16, 19)
    #define GPIO42_MODE_OFFSET              16
#define GPIO_GPIO_MODE9                    (0x00000390)
    #define GPIO94_MODE                     BITS(28, 31)   /* ANTSEL3 */
    #define GPIO94_MODE_OFFSET              28
    #define GPIO93_MODE                     BITS(24, 27)   /* ANTSEL2 */
    #define GPIO93_MODE_OFFSET              24
    #define GPIO92_MODE                     BITS(20, 23)   /* ANTSEL1 */
    #define GPIO92_MODE_OFFSET              20
    #define GPIO91_MODE                     BITS(16, 19)   /* ANTSEL0 */
    #define GPIO91_MODE_OFFSET              16
    #define GPIO101_MODE                    BITS(12, 15)   /* ANTSEL10 */
    #define GPIO101_MODE_OFFSET             12
    #define GPIO100_MODE                    BITS(8, 11)   /* ANTSEL9 */
    #define GPIO100_MODE_OFFSET             8
    #define GPIO99_MODE                     BITS(4,  7)   /* ANTSEL8 */
    #define GPIO99_MODE_OFFSET              4
    #define GPIO98_MODE                     BITS(0,  3)   /* ANTSEL7 */
    #define GPIO98_MODE_OFFSET              0
#define GPIO_GPIO_MODE10                   (0x00000394)
    #define GPIO82_MODE                     BITS(12, 15)   /* ANTSEL21 */
    #define GPIO82_MODE_OFFSET              12
    #define GPIO81_MODE                     BITS(8, 11)   /* ANTSEL20 */
    #define GPIO81_MODE_OFFSET              8
    #define GPIO80_MODE                     BITS(4,  7)   /* ANTSEL19 */
    #define GPIO80_MODE_OFFSET              4
    #define GPIO79_MODE                     BITS(0,  3)   /* ANTSEL18 */
    #define GPIO79_MODE_OFFSET              0
void mt7622_antenna_sel_ctl(struct _RTMP_ADAPTER *pAd);
void mt7622_antenna_sel_cfg(struct _RTMP_ADAPTER *pAd);
UINT32 mt7622_antsel_get_used_pin(struct _RTMP_ADAPTER *pAd, UCHAR polCtrl, UCHAR antCtrl, UCHAR wifiStreams);
UINT32 mt7622_gpio_get(UINT32 offset);
void mt7622_gpio_set(UINT32 offset, UINT32 gpioVal);
void mt7622_pin_mux_control_setup(struct _RTMP_ADAPTER *pAd, UCHAR polCtrl, UCHAR antCtrl, UCHAR wifiStreams);
#endif

void mt7622_init(struct _RTMP_ADAPTER *pAd);
void mt7622_get_tx_pwr_per_rate(struct _RTMP_ADAPTER *pAd);
void mt7622_get_tx_pwr_info(struct _RTMP_ADAPTER *pAd);
void mt7622_antenna_sel_ctl(struct _RTMP_ADAPTER *pAd);
void mt7622_pwrOn(struct _RTMP_ADAPTER *pAd);
void mt7622_calibration(struct _RTMP_ADAPTER *pAd, UCHAR channel);
void mt7622_tssi_compensation(struct _RTMP_ADAPTER *pAd, UCHAR channel);
VOID mt7622_chip_dbg_init(struct _RTMP_CHIP_DBG *dbg_ops);

#ifdef MT7622_FPGA
INT mt7622_chk_top_default_cr_setting(struct _RTMP_ADAPTER *pAd);
INT mt7622_chk_hif_default_cr_setting(struct _RTMP_ADAPTER *pAd);
#endif /* MT7622_FPGA */

VOID mt7622_trigger_intr_to_mcu(BOOLEAN enable);

INT Mt7622AsicArchOpsInit(struct _RTMP_ADAPTER *pAd);

#ifdef ERR_RECOVERY
#define DUMMY_N9_HEART_BEAT                (MCU_CFG_BASE + 0x0300)  /* N9 Heart beat, SW accumulate */
#endif

#ifdef CONFIG_PROPRIETARY_DRIVER
int rbus_tssi_set(struct _RTMP_ADAPTER *ad, UCHAR mode);
int rbus_pa_lna_set(struct _RTMP_ADAPTER *ad, UINT32 mode);
#endif /*CONFIG_PROPRIETARY_DRIVER*/

#ifdef PRE_CAL_MT7622_SUPPORT
#ifdef RTMP_FLASH_SUPPORT
#define CAL_FLASH_OFFSET	1024
#else
#define CAL_FLASH_OFFSET	0
#endif

#define TXLPFG_FLASH_OFFSET (CAL_FLASH_OFFSET)
#define TXDCIQ_FLASH_OFFSET (CAL_FLASH_OFFSET + TXLPFG_CAL_SIZE)
#define TXDPD_FLASH_OFFSET (TXDCIQ_FLASH_OFFSET + TXDCIQ_CAL_SIZE)

#define TXLPFG_CAL_SIZE (4 * sizeof(UINT32))
#define TXDCIQ_CAL_SIZE (48 * sizeof(UINT32))
#endif /*PRE_CAL_MT7622_SUPPORT*/

#endif /* __MT7622_H__ */

