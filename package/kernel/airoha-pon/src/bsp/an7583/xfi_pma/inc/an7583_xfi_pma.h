#ifndef _AN7583_XFI_PMA_H_
#define _AN7583_XFI_PMA_H_



//#include "olt_phy_def.h"
//#include "olt_phy_reg.h"

#define XFI_ANA_RG_BASE  (0x1fa7F000)  // only for asic
#define XFI_PMA_RG_BASE  (0x1fa7E000)  // only for asic

#define XFI_BCDR_RG_BASE  (0x1fbe5800)  // only for asic
#define XFI_BIST_RG_BASE  (0x1fbe5900)  // only for asic

#define IO_GPHYREG(reg)						get_xfi_phya_data(reg)//ioread32((void __iomem *)(reg)) /* GetReg((uint)reg) */
#define IO_SPHYREG(reg, val)				set_xfi_phya_data(reg, val) /* SetReg((uint)reg, val) */

#define IO_GBCDR_REG(reg)					get_xfi_bcdr_data(reg)//ioread32((void __iomem *)(reg)) /* GetReg((uint)reg) */
#define IO_SBCDR_REG(reg, val)				set_xfi_bcdr_data(reg, val) /* SetReg((uint)reg, val) */



//#define GPON_OLT_BCDR_CSR_BASE                  (0x1fbe5800)
//#define XFI_PMA_BASE                            (0x1fa7e000)  //PMA_DIG_CODA
//#define XFI_ANA_BASE                            (0x1fa7f000)  //PMA_ANA_CODA
#define XFI_PMA_BASE                            (0x1fa70000)  //PMA_DIG_CODA
#define GPON_XFI_OLT_BCDR_BASE                  (0x1fbe0000)  //PMA_BCDR_CODA



//PMA_DIG_CODA
#define AN7583_XFI_PMA_LCPLL_PWCTL_SETTING_0		              XFI_PMA_BASE+0xE000
#define AN7583_XFI_PMA_LCPLL_PWCTL_SETTING_1		              XFI_PMA_BASE+0xE004
#define AN7583_XFI_PMA_LCPLL_PWCTL_SETTING_2		              XFI_PMA_BASE+0xE008
#define AN7583_XFI_PMA_LCPLL_PWCTL_DBG_SETTING		              XFI_PMA_BASE+0xE00C
#define AN7583_XFI_PMA_LCPLL_TDC_PW_0		                      XFI_PMA_BASE+0xE010
#define AN7583_XFI_PMA_LCPLL_TDC_PW_1		                      XFI_PMA_BASE+0xE014
#define AN7583_XFI_PMA_LCPLL_TDC_PW_2		                      XFI_PMA_BASE+0xE018
#define AN7583_XFI_PMA_LCPLL_TDC_PW_3		                      XFI_PMA_BASE+0xE01C
#define AN7583_XFI_PMA_LCPLL_TDC_PW_4		                      XFI_PMA_BASE+0xE020
#define AN7583_XFI_PMA_LCPLL_TDC_PW_5		                      XFI_PMA_BASE+0xE024
#define AN7583_XFI_PMA_LCPLL_TDC_FLT_0		                      XFI_PMA_BASE+0xE028
#define AN7583_XFI_PMA_LCPLL_TDC_FLT_1		                      XFI_PMA_BASE+0xE02C
#define AN7583_XFI_PMA_LCPLL_TDC_FLT_2		                      XFI_PMA_BASE+0xE030
#define AN7583_XFI_PMA_LCPLL_TDC_FLT_3		                      XFI_PMA_BASE+0xE034
#define AN7583_XFI_PMA_LCPLL_TDC_FLT_4		                      XFI_PMA_BASE+0xE038
#define AN7583_XFI_PMA_LCPLL_TDC_FLT_5		                      XFI_PMA_BASE+0xE03C
#define AN7583_XFI_PMA_LCPLL_TDC_FLT_6		                      XFI_PMA_BASE+0xE040
#define AN7583_XFI_PMA_LCPLL_TDC_FLT_7		                      XFI_PMA_BASE+0xE044
#define AN7583_XFI_PMA_LCPLL_TDC_PCW_1		                      XFI_PMA_BASE+0xE048
#define AN7583_XFI_PMA_LCPLL_TDC_PCW_2		                      XFI_PMA_BASE+0xE04C
#define AN7583_XFI_PMA_LCPLL_TDC_RO_1		                      XFI_PMA_BASE+0xE050
#define AN7583_XFI_PMA_LCPLL_TDC_RO_2		                      XFI_PMA_BASE+0xE054
#define AN7583_XFI_PMA_LCPLL_TDC_RO_3		                      XFI_PMA_BASE+0xE058
#define AN7583_XFI_PMA_LCPLL_TDC_RO_4		                      XFI_PMA_BASE+0xE05C
#define AN7583_XFI_PMA_LCPLL_TDC_RO_5		                      XFI_PMA_BASE+0xE060
#define AN7583_XFI_PMA_LCPLL_TDC_CTRL_0		                      XFI_PMA_BASE+0xE064
#define AN7583_XFI_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0		          XFI_PMA_BASE+0xE068
#define AN7583_XFI_PMA_RX_EYE_TOP_EYEINDEX_CTRL_1		          XFI_PMA_BASE+0xE06C
#define AN7583_XFI_PMA_RX_EYE_TOP_EYEINDEX_CTRL_2		          XFI_PMA_BASE+0xE070
#define AN7583_XFI_PMA_RX_EYE_TOP_EYEINDEX_CTRL_3		          XFI_PMA_BASE+0xE074
#define AN7583_XFI_PMA_RX_EYE_TOP_EYEOPENING_CTRL_0		          XFI_PMA_BASE+0xE078
#define AN7583_XFI_PMA_RX_EYE_TOP_EYEOPENING_CTRL_1		          XFI_PMA_BASE+0xE07C
#define AN7583_XFI_PMA_RX_EYE_TOP_EYECNT_CTRL_0		              XFI_PMA_BASE+0xE080
#define AN7583_XFI_PMA_RX_EYE_TOP_EYECNT_CTRL_1		              XFI_PMA_BASE+0xE084
#define AN7583_XFI_PMA_RX_EYE_TOP_EYECNT_CTRL_2		              XFI_PMA_BASE+0xE088
#define AN7583_XFI_PMA_RX_CTRL_SEQUENCE_CTRL_0		              XFI_PMA_BASE+0xE08C
#define AN7583_XFI_PMA_RX_CTRL_SEQUENCE_CTRL_1		              XFI_PMA_BASE+0xE090
#define AN7583_XFI_PMA_RX_CTRL_SEQUENCE_CTRL_2		              XFI_PMA_BASE+0xE094
#define AN7583_XFI_PMA_RX_CTRL_SEQUENCE_CTRL_3		              XFI_PMA_BASE+0xE098
#define AN7583_XFI_PMA_RX_CTRL_SEQUENCE_CTRL_4		              XFI_PMA_BASE+0xE09C
#define AN7583_XFI_PMA_RX_CTRL_SEQUENCE_CTRL_5		              XFI_PMA_BASE+0xE100
#define AN7583_XFI_PMA_RX_CTRL_SEQUENCE_CTRL_6		              XFI_PMA_BASE+0xE104
#define AN7583_XFI_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0		          XFI_PMA_BASE+0xE108
#define AN7583_XFI_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1		          XFI_PMA_BASE+0xE10C
#define AN7583_XFI_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0		      XFI_PMA_BASE+0xE110
#define AN7583_XFI_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1		      XFI_PMA_BASE+0xE114
#define AN7583_XFI_PMA_PHY_EQ_CTRL_0		                      XFI_PMA_BASE+0xE118
#define AN7583_XFI_PMA_PHY_EQ_CTRL_1		                      XFI_PMA_BASE+0xE11C
#define AN7583_XFI_PMA_PHY_EQ_CTRL_2		                      XFI_PMA_BASE+0xE120
#define AN7583_XFI_PMA_PHY_EQ_CTRL_3		                      XFI_PMA_BASE+0xE124
#define AN7583_XFI_PMA_PHY_EQ_CTRL_4		                      XFI_PMA_BASE+0xE128
#define AN7583_XFI_PMA_PHY_EQ_CTRL_5		                      XFI_PMA_BASE+0xE12C
#define AN7583_XFI_PMA_PHY_EQ_CTRL_6		                      XFI_PMA_BASE+0xE130
#define AN7583_XFI_PMA_PHY_EQ_CTRL_7		                      XFI_PMA_BASE+0xE134
#define AN7583_XFI_PMA_PHY_EQ_CTRL_8		                      XFI_PMA_BASE+0xE138
#define AN7583_XFI_PMA_PHY_EQ_CTRL_9		                      XFI_PMA_BASE+0xE13C
#define AN7583_XFI_PMA_PHY_EQ_CTRL_10		                      XFI_PMA_BASE+0xE140
#define AN7583_XFI_PMA_RX_FEOS		                              XFI_PMA_BASE+0xE144
#define AN7583_XFI_PMA_RX_BLWC		                              XFI_PMA_BASE+0xE148
#define AN7583_XFI_PMA_RX_FREQ_DET_1		                      XFI_PMA_BASE+0xE14C
#define AN7583_XFI_PMA_RX_FREQ_DET_2		                      XFI_PMA_BASE+0xE150
#define AN7583_XFI_PMA_RX_FREQ_DET_3		                      XFI_PMA_BASE+0xE154
#define AN7583_XFI_PMA_RX_FREQ_DET_4		                      XFI_PMA_BASE+0xE158
#define AN7583_XFI_PMA_RX_PI_CAL		                          XFI_PMA_BASE+0xE15C
#define AN7583_XFI_PMA_RX_CAL_1		                              XFI_PMA_BASE+0xE160
#define AN7583_XFI_PMA_RX_CAL_2		                              XFI_PMA_BASE+0xE164
#define AN7583_XFI_PMA_RX_SIGDET_0		                          XFI_PMA_BASE+0xE168
#define AN7583_XFI_PMA_RX_SIGDET_1		                          XFI_PMA_BASE+0xE16C
#define AN7583_XFI_PMA_RX_FLL_0		                              XFI_PMA_BASE+0xE170
#define AN7583_XFI_PMA_RX_FLL_1		                              XFI_PMA_BASE+0xE174
#define AN7583_XFI_PMA_RX_FLL_2		                              XFI_PMA_BASE+0xE178
#define AN7583_XFI_PMA_RX_FLL_3		                              XFI_PMA_BASE+0xE17C
#define AN7583_XFI_PMA_RX_FLL_4		                              XFI_PMA_BASE+0xE180
#define AN7583_XFI_PMA_RX_FLL_5		                              XFI_PMA_BASE+0xE184
#define AN7583_XFI_PMA_RX_FLL_6		                              XFI_PMA_BASE+0xE188
#define AN7583_XFI_PMA_RX_FLL_7		                              XFI_PMA_BASE+0xE18C
#define AN7583_XFI_PMA_RX_FLL_8		                              XFI_PMA_BASE+0xE190
#define AN7583_XFI_PMA_RX_FLL_9		                              XFI_PMA_BASE+0xE194
#define AN7583_XFI_PMA_RX_FLL_a		                              XFI_PMA_BASE+0xE198
#define AN7583_XFI_PMA_RX_FLL_b		                              XFI_PMA_BASE+0xE19C
#define AN7583_XFI_PMA_RX_PDOS_CTRL_0		                      XFI_PMA_BASE+0xE200
#define AN7583_XFI_PMA_RX_RESET_0		                          XFI_PMA_BASE+0xE204
#define AN7583_XFI_PMA_RX_RESET_1		                          XFI_PMA_BASE+0xE208
#define AN7583_XFI_PMA_RX_DEBUG_0		                          XFI_PMA_BASE+0xE20C
#define AN7583_XFI_PMA_BISTCTL_CONTROL		                      XFI_PMA_BASE+0xE210
#define AN7583_XFI_PMA_BISTCTL_ALIGN_PAT		                  XFI_PMA_BASE+0xE214
#define AN7583_XFI_PMA_BISTCTL_PROGRAM_PAT_0		              XFI_PMA_BASE+0xE218
#define AN7583_XFI_PMA_BISTCTL_PROGRAM_PAT_1		              XFI_PMA_BASE+0xE21C
#define AN7583_XFI_PMA_BISTCTL_POLLUTION		                  XFI_PMA_BASE+0xE220
#define AN7583_XFI_PMA_BISTCTL_PRBS_INITIAL_SEED		          XFI_PMA_BASE+0xE224
#define AN7583_XFI_PMA_BISTCTL_PRBS_EVENT		                  XFI_PMA_BASE+0xE228
#define AN7583_XFI_PMA_BISTCTL_PRBS_ERRCNT		                  XFI_PMA_BASE+0xE22C
#define AN7583_XFI_PMA_BISTCTL_PRBS_FAIL_THRESHOLD		          XFI_PMA_BASE+0xE230
#define AN7583_XFI_PMA_RX_TORGS_DEBUG_0		                      XFI_PMA_BASE+0xE234
#define AN7583_XFI_PMA_RX_TORGS_DEBUG_1		                      XFI_PMA_BASE+0xE238
#define AN7583_XFI_PMA_RX_TORGS_DEBUG_2		                      XFI_PMA_BASE+0xE23C
#define AN7583_XFI_PMA_RX_TORGS_DEBUG_3		                      XFI_PMA_BASE+0xE240
#define AN7583_XFI_PMA_RX_TORGS_DEBUG_4		                      XFI_PMA_BASE+0xE244
#define AN7583_XFI_PMA_RX_TORGS_DEBUG_5		                      XFI_PMA_BASE+0xE248
#define AN7583_XFI_PMA_RX_TORGS_DEBUG_6		                      XFI_PMA_BASE+0xE24C
#define AN7583_XFI_PMA_RX_TORGS_DEBUG_7		                      XFI_PMA_BASE+0xE250
#define AN7583_XFI_PMA_RX_TORGS_DEBUG_8		                      XFI_PMA_BASE+0xE254
#define AN7583_XFI_PMA_RX_TORGS_DEBUG_9		                      XFI_PMA_BASE+0xE258
#define AN7583_XFI_PMA_RX_TORGS_DEBUG_10		                  XFI_PMA_BASE+0xE25C
#define AN7583_XFI_PMA_TX_RST_B		                              XFI_PMA_BASE+0xE260
#define AN7583_XFI_PMA_TX_CALIB_0		                          XFI_PMA_BASE+0xE264
#define AN7583_XFI_PMA_TX_CALIB_1		                          XFI_PMA_BASE+0xE268
#define AN7583_XFI_PMA_TX_CALIB_2		                          XFI_PMA_BASE+0xE26C
#define AN7583_XFI_PMA_XPON_SETTING_0		                      XFI_PMA_BASE+0xE270
#define AN7583_XFI_PMA_XPON_SETTING_1		                      XFI_PMA_BASE+0xE274
#define AN7583_XFI_PMA_XPON_SETTING_2		                      XFI_PMA_BASE+0xE278
#define AN7583_XFI_PMA_XPON_STA		                              XFI_PMA_BASE+0xE27C
#define AN7583_XFI_PMA_XPON_INT_EN_0		                      XFI_PMA_BASE+0xE280
#define AN7583_XFI_PMA_XPON_INT_EN_1		                      XFI_PMA_BASE+0xE284
#define AN7583_XFI_PMA_XPON_INT_STA_0		                      XFI_PMA_BASE+0xE288
#define AN7583_XFI_PMA_XPON_INT_STA_1		                      XFI_PMA_BASE+0xE28C
#define AN7583_XFI_PMA_RX_TORGS_DEBUG_11		                  XFI_PMA_BASE+0xE290
#define AN7583_XFI_PMA_RX_FORCE_MODE_0		                      XFI_PMA_BASE+0xE294
#define AN7583_XFI_PMA_RX_FORCE_MODE_1		                      XFI_PMA_BASE+0xE298
#define AN7583_XFI_PMA_RX_FORCE_MODE_2		                      XFI_PMA_BASE+0xE29C
#define AN7583_XFI_PMA_RX_DISB_MODE_0		                      XFI_PMA_BASE+0xE300
#define AN7583_XFI_PMA_RX_DISB_MODE_1		                      XFI_PMA_BASE+0xE304
#define AN7583_XFI_PMA_RX_DISB_MODE_2		                      XFI_PMA_BASE+0xE308
#define AN7583_XFI_PMA_RX_FORCE_MODE_3		                      XFI_PMA_BASE+0xE30C
#define AN7583_XFI_PMA_RX_FORCE_MODE_4		                      XFI_PMA_BASE+0xE310
#define AN7583_XFI_PMA_RX_FORCE_MODE_5		                      XFI_PMA_BASE+0xE314
#define AN7583_XFI_PMA_RX_FORCE_MODE_6		                      XFI_PMA_BASE+0xE318
#define AN7583_XFI_PMA_RX_DISB_MODE_3		                      XFI_PMA_BASE+0xE31C
#define AN7583_XFI_PMA_RX_DISB_MODE_4		                      XFI_PMA_BASE+0xE320
#define AN7583_XFI_PMA_RX_DISB_MODE_5		                      XFI_PMA_BASE+0xE324
#define AN7583_XFI_PMA_RX_FORCE_MODE_7		                      XFI_PMA_BASE+0xE328
#define AN7583_XFI_PMA_RX_FORCE_MODE_8		                      XFI_PMA_BASE+0xE32C
#define AN7583_XFI_PMA_RX_FORCE_MODE_9		                      XFI_PMA_BASE+0xE330
#define AN7583_XFI_PMA_RX_DISB_MODE_6		                      XFI_PMA_BASE+0xE334
#define AN7583_XFI_PMA_RX_DISB_MODE_7		                      XFI_PMA_BASE+0xE338
#define AN7583_XFI_PMA_RX_DISB_MODE_8		                      XFI_PMA_BASE+0xE33C
#define AN7583_XFI_PMA_BIST_0		                              XFI_PMA_BASE+0xE340
#define AN7583_XFI_PMA_BIST_1		                              XFI_PMA_BASE+0xE344
#define AN7583_XFI_PMA_BIST_2		                              XFI_PMA_BASE+0xE348
#define AN7583_XFI_PMA_DA_XPON_PWDB_0		                      XFI_PMA_BASE+0xE34C
#define AN7583_XFI_PMA_DA_XPON_PWDB_1		                      XFI_PMA_BASE+0xE350
#define AN7583_XFI_PMA_LCPLL_0		                              XFI_PMA_BASE+0xE354
#define AN7583_XFI_PMA_LCPLL_1		                              XFI_PMA_BASE+0xE358
#define AN7583_XFI_PMA_LCPLL_2		                              XFI_PMA_BASE+0xE35C
#define AN7583_XFI_PMA_ADD_DIG_RESERVE_0		                  XFI_PMA_BASE+0xE360
#define AN7583_XFI_PMA_ADD_DIG_RESERVE_1		                  XFI_PMA_BASE+0xE364
#define AN7583_XFI_PMA_ADD_DIG_RESERVE_2		                  XFI_PMA_BASE+0xE368
#define AN7583_XFI_PMA_ADD_DIG_RESERVE_3		                  XFI_PMA_BASE+0xE36C
#define AN7583_XFI_PMA_ADD_DIG_RESERVE_4		                  XFI_PMA_BASE+0xE370
#define AN7583_XFI_PMA_RG_XPON_RX_RESERVED_1		              XFI_PMA_BASE+0xE374
#define AN7583_XFI_PMA_ADD_DIG_RO_RESERVE_0		                  XFI_PMA_BASE+0xE378
#define AN7583_XFI_PMA_ADD_DIG_RO_RESERVE_1		                  XFI_PMA_BASE+0xE37C
#define AN7583_XFI_PMA_ADD_DIG_RO_RESERVE_2		                  XFI_PMA_BASE+0xE380
#define AN7583_XFI_PMA_ADD_DIG_RO_RESERVE_3		                  XFI_PMA_BASE+0xE384
#define AN7583_XFI_PMA_ADD_DIG_RO_RESERVE_4		                  XFI_PMA_BASE+0xE388
#define AN7583_XFI_PMA_ADD_RX_SYS_EN_SEL_0		                  XFI_PMA_BASE+0xE38C
#define AN7583_XFI_PMA_PLL_TDC_FREQDET_0		                  XFI_PMA_BASE+0xE390
#define AN7583_XFI_PMA_PLL_TDC_FREQDET_1		                  XFI_PMA_BASE+0xE394
#define AN7583_XFI_PMA_PLL_TDC_FREQDET_2		                  XFI_PMA_BASE+0xE398
#define AN7583_XFI_PMA_PLL_TDC_FREQDET_3		                  XFI_PMA_BASE+0xE39C
#define AN7583_XFI_PMA_DA_XPON_TX_FORCE_0		                  XFI_PMA_BASE+0xE400
#define AN7583_XFI_PMA_DA_XPON_TX_FORCE_1		                  XFI_PMA_BASE+0xE404
#define AN7583_XFI_PMA_DA_XPON_TX_FORCE_2		                  XFI_PMA_BASE+0xE408
#define AN7583_XFI_PMA_RX_FORCE_MODE_10		                      XFI_PMA_BASE+0xE40C
#define AN7583_XFI_PMA_ADD_CLKPATH_RST_0		                  XFI_PMA_BASE+0xE410
#define AN7583_XFI_PMA_ADD_XPON_MODE_1		                      XFI_PMA_BASE+0xE414
#define AN7583_XFI_PMA_ADD_R2T_MODE_1		                      XFI_PMA_BASE+0xE418
#define AN7583_XFI_PMA_ADD_T2R_MODE_1		                      XFI_PMA_BASE+0xE41C
#define AN7583_XFI_PMA_ADD_LCPLL_RO_1		                      XFI_PMA_BASE+0xE420
#define AN7583_XFI_PMA_ADD_RO_RX2ANA_1		                      XFI_PMA_BASE+0xE424
#define AN7583_XFI_PMA_ADD_RO_RX2ANA_2		                      XFI_PMA_BASE+0xE428
#define AN7583_XFI_PMA_ADD_RO_RX2ANA_3		                      XFI_PMA_BASE+0xE42C
#define AN7583_XFI_PMA_ADD_RO_R2TMODE_1		                      XFI_PMA_BASE+0xE430
#define AN7583_XFI_PMA_RG_LCPLL_XPON_CMN_STB_CTRL_0		          XFI_PMA_BASE+0xE434
#define AN7583_XFI_PMA_RG_LCPLL_XPON_CMN_STB_CTRL_1		          XFI_PMA_BASE+0xE438
#define AN7583_XFI_PMA_RG_TX_HSDATA_EN_EXT_CNT_0		          XFI_PMA_BASE+0xE43C
#define AN7583_XFI_PMA_PON_TX_COUNTER_0		                      XFI_PMA_BASE+0xE440
#define AN7583_XFI_PMA_PON_TX_COUNTER_1		                      XFI_PMA_BASE+0xE444
#define AN7583_XFI_PMA_PON_TX_COUNTER_2		                      XFI_PMA_BASE+0xE448
#define AN7583_XFI_PMA_PON_TX_COUNTER_3		                      XFI_PMA_BASE+0xE44C
#define AN7583_XFI_PMA_PON_CK_SET		                          XFI_PMA_BASE+0xE450
#define AN7583_XFI_PMA_TX_FIFO_MODE_SEL		                      XFI_PMA_BASE+0xE454
#define AN7583_XFI_PMA_XPON_PLL_STB_CNT		                      XFI_PMA_BASE+0xE458
#define AN7583_XFI_PMA_XPON_PLL_STOP_CNT		                  XFI_PMA_BASE+0xE45C
#define AN7583_XFI_PMA_SW_RST_SET		                          XFI_PMA_BASE+0xE460
#define AN7583_XFI_PMA_ADD_RO_TX2ANA_1		                      XFI_PMA_BASE+0xE464
#define AN7583_XFI_PMA_TX_DLY_CTRL		                          XFI_PMA_BASE+0xE468
#define AN7583_XFI_PMA_MEM_WRAPPER_CTRL		                      XFI_PMA_BASE+0xE46C
#define AN7583_XFI_PMA_XPON_INT_EN_2		                      XFI_PMA_BASE+0xE470
#define AN7583_XFI_PMA_XPON_INT_EN_3		                      XFI_PMA_BASE+0xE474
#define AN7583_XFI_PMA_XPON_INT_STA_2		                      XFI_PMA_BASE+0xE478
#define AN7583_XFI_PMA_XPON_INT_STA_3		                      XFI_PMA_BASE+0xE47C
#define AN7583_XFI_PMA_XPON_SD_BEHAVIOR_SETTING_0		          XFI_PMA_BASE+0xE480
#define AN7583_XFI_PMA_XPON_SD_BEHAVIOR_SETTING_1		          XFI_PMA_BASE+0xE484
#define AN7583_XFI_PMA_XPON_SD_BEHAVIOR_STA		                  XFI_PMA_BASE+0xE488
#define AN7583_XFI_PMA_RX_EXTRAL_CTRL		                      XFI_PMA_BASE+0xE48C
#define AN7583_XFI_PMA_RX_LEQ_DISB_CTRL0		                  XFI_PMA_BASE+0xE490
#define AN7583_XFI_PMA_RX_LEQ_DISB_CTRL1		                  XFI_PMA_BASE+0xE494
#define AN7583_XFI_PMA_RX_LEQ_FORCE_CTRL0		                  XFI_PMA_BASE+0xE498
#define AN7583_XFI_PMA_RX_LEQ_FORCE_CTRL1		                  XFI_PMA_BASE+0xE49C
#define AN7583_XFI_PMA_RX_LEQ_ENTIME_CTRL0		                  XFI_PMA_BASE+0xE500
#define AN7583_XFI_PMA_RX_LEQ_ENTIME_CTRL1		                  XFI_PMA_BASE+0xE504
#define AN7583_XFI_PMA_RX_LEQ_ENTIME_CTRL2		                  XFI_PMA_BASE+0xE508
#define AN7583_XFI_PMA_RX_LEQ_CTRL0		                          XFI_PMA_BASE+0xE50C
#define AN7583_XFI_PMA_RX_LEQ_CTRL1		                          XFI_PMA_BASE+0xE510
#define AN7583_XFI_PMA_RX_LEQ_CTRL2		                          XFI_PMA_BASE+0xE514
#define AN7583_XFI_PMA_RX_LEQ_RO0		                          XFI_PMA_BASE+0xE518
#define AN7583_XFI_PMA_RX_LEQ_RO1		                          XFI_PMA_BASE+0xE51C
#define AN7583_XFI_PMA_PLL_FT_FREQDET_0		                      XFI_PMA_BASE+0xE520
#define AN7583_XFI_PMA_PLL_FT_FREQDET_1		                      XFI_PMA_BASE+0xE524
#define AN7583_XFI_PMA_PLL_FT_FREQDET_2		                      XFI_PMA_BASE+0xE528
#define AN7583_XFI_PMA_PLL_FT_FREQDET_3		                      XFI_PMA_BASE+0xE52C
#define AN7583_XFI_PMA_RO_RX_FREQDET		                      XFI_PMA_BASE+0xE530
#define AN7583_XFI_PMA_RO_PLL_FREQDET		                      XFI_PMA_BASE+0xE534
#define AN7583_XFI_PMA_RO_PLL_FT_FREQDET                          XFI_PMA_BASE+0xE538
#define AN7583_XFI_PMA_RO_PMA_FREQDET		                      XFI_PMA_BASE+0xE53C
#define AN7583_XFI_PMA_RG_PMA_FREQDET		                      XFI_PMA_BASE+0xE540
#define AN7583_XFI_PMA_RG_EXT_BEN_DATA		                      XFI_PMA_BASE+0xE544
#define AN7583_XFI_PMA_RG_PRE_BEN_DATA		                      XFI_PMA_BASE+0xE548
#define AN7583_XFI_PMA_RX_TORGS_DEBUG_12		                  XFI_PMA_BASE+0xE54C
#define AN7583_XFI_PMA_RO_FLL_ADC_0		                          XFI_PMA_BASE+0xE550
#define AN7583_XFI_PMA_RO_FLL_ADC_1		                          XFI_PMA_BASE+0xE554
#define AN7583_XFI_PMA_RO_FLL_ADC_2		                          XFI_PMA_BASE+0xE558
#define AN7583_XFI_PMA_RO_FLL_ADC_3		                          XFI_PMA_BASE+0xE55C
#define AN7583_XFI_PMA_RO_FLL_ADC_4		                          XFI_PMA_BASE+0xE560
#define AN7583_XFI_PMA_RG_AD_XPON_PLL_FT_CK_MON_MUX_SEL		      XFI_PMA_BASE+0xE564
#define AN7583_XFI_PMA_XPON_INT_EN_4		                      XFI_PMA_BASE+0xE568
#define AN7583_XFI_PMA_XPON_INT_STA_4		                      XFI_PMA_BASE+0xE56C
#define AN7583_XFI_PMA_XPON_PMA_AFIFO_REACH_TH		              XFI_PMA_BASE+0xE570
#define AN7583_XFI_PMA_XPON_MD32_PBUS_CSR_CTRL		              XFI_PMA_BASE+0xE574
#define AN7583_XFI_PMA_BENOFF_BIST_SEED		                      XFI_PMA_BASE+0xE578
#define AN7583_XFI_PMA_PWR_PLL_CTRL		                          XFI_PMA_BASE+0xE57C
#define AN7583_XFI_PMA_xpon_tx_rate_ctrl		                  XFI_PMA_BASE+0xE580
#define AN7583_XFI_PMA_BENOFF_DATA0		                          XFI_PMA_BASE+0xE584
#define AN7583_XFI_PMA_BENOFF_DATA1		                          XFI_PMA_BASE+0xE588
#define AN7583_XFI_PMA_BENOFF_DATA2		                          XFI_PMA_BASE+0xE58C
#define AN7583_XFI_PMA_BENOFF_DATA3		                          XFI_PMA_BASE+0xE590
#define AN7583_XFI_PMA_BENOFF_CTRL		                          XFI_PMA_BASE+0xE594
#define AN7583_XFI_PMA_HS_DATA_EN_SEL		                      XFI_PMA_BASE+0xE598
#define AN7583_XFI_PMA_FIFO_CK_STATUS		                      XFI_PMA_BASE+0xE59C
#define AN7583_XFI_PMA_TDC_DLF_GAIN_STATUS		                  XFI_PMA_BASE+0xE600
#define AN7583_XFI_PMA_TDC_DLF_MODE_SETTING		                  XFI_PMA_BASE+0xE604
#define AN7583_XFI_PMA_MD32_ISR_CTRL		                      XFI_PMA_BASE+0xE608
#define AN7583_XFI_PMA_MD32_MEM_CLK_CTRL		                  XFI_PMA_BASE+0xE60C
#define AN7583_XFI_PMA_MD32_MEM_CLK_CG_CTRL		                  XFI_PMA_BASE+0xE610
#define AN7583_XFI_PMA_XG_EQD_STA		                          XFI_PMA_BASE+0xE614
#define AN7583_XFI_PMA_XG_EQD_CTRL		                          XFI_PMA_BASE+0xE618
#define AN7583_XFI_PMA_RAWDATA_DUMP_CTRL_0		                  XFI_PMA_BASE+0xE61C
#define AN7583_XFI_PMA_RAWDATA_DUMP_CTRL_1		                  XFI_PMA_BASE+0xE620
#define AN7583_XFI_PMA_RAWDATA_DUMP_CTRL_2		                  XFI_PMA_BASE+0xE624
#define AN7583_XFI_PMA_RAWDATA_DUMP_CTRL_3		                  XFI_PMA_BASE+0xE628
#define AN7583_XFI_PMA_RAWDATA_DUMP_STA_0		                  XFI_PMA_BASE+0xE62C
#define AN7583_XFI_PMA_RAWDATA_DUMP_STA_1		                  XFI_PMA_BASE+0xE630
#define AN7583_XFI_PMA_RAWDATA_DUMP_STA_2		                  XFI_PMA_BASE+0xE634
#define AN7583_XFI_PMA_MD32MEM_DIV2_CNT		                      XFI_PMA_BASE+0xE638
#define AN7583_XFI_PMA_AD_XPON_CDR_LPF_SV		                  XFI_PMA_BASE+0xE63C
#define AN7583_XFI_PMA_K_VALUE_RD		                          XFI_PMA_BASE+0xE640
#define AN7583_XFI_PMA_XPONPLL_CTRL		                          XFI_PMA_BASE+0xE644
#define AN7583_XFI_PMA_MD32_GATE_HALT	                          XFI_PMA_BASE+0xE648
#define AN7583_XFI_PMA_MD32_MONCCNT		                          XFI_PMA_BASE+0xE64C
#define AN7583_XFI_PMA_MD32_MONPCNT0		                      XFI_PMA_BASE+0xE650
#define AN7583_XFI_PMA_MD32_MONPCNT1		                      XFI_PMA_BASE+0xE654
#define AN7583_XFI_PMA_MD32_MONPCNT2		                      XFI_PMA_BASE+0xE658
#define AN7583_XFI_PMA_MD32_MONCONTID		                      XFI_PMA_BASE+0xE65C
#define AN7583_XFI_PMA_MD32_MONPC		                          XFI_PMA_BASE+0xE660
#define AN7583_XFI_PMA_MD32_TBUF		                          XFI_PMA_BASE+0xE664
#define AN7583_XFI_PMA_MD32_TBUFWDATA_L		                      XFI_PMA_BASE+0xE668
#define AN7583_XFI_PMA_MD32_TBUFWDATA_H		                      XFI_PMA_BASE+0xE66C
#define AN7583_XFI_PMA_MD32_TBUFRDATA_L		                      XFI_PMA_BASE+0xE670
#define AN7583_XFI_PMA_MD32_TBUFRDATA_H		                      XFI_PMA_BASE+0xE674
#define AN7583_XFI_PMA_ADDR_MD32_PMEM_ADDR_REV		              XFI_PMA_BASE+0xE678
#define AN7583_XFI_PMA_ADDR_MD32_PMEM_DATA_REV		              XFI_PMA_BASE+0xE67C
#define AN7583_XFI_PMA_ADDR_MD32_DMEM_ADDR_REV		              XFI_PMA_BASE+0xE680
#define AN7583_XFI_PMA_ADDR_MD32_DMEM_DATA_REV		              XFI_PMA_BASE+0xE684
#define AN7583_XFI_PMA_JCPLL_PWCTL_SETTING_0		              XFI_PMA_BASE+0xE688
#define AN7583_XFI_PMA_JCPLL_PWCTL_SETTING_1		              XFI_PMA_BASE+0xE68C
#define AN7583_XFI_PMA_JCPLL_PWCTL_SETTING_2		              XFI_PMA_BASE+0xE690
#define AN7583_XFI_PMA_JCPLL_CHG		                          XFI_PMA_BASE+0xE694
#define AN7583_XFI_PMA_RG_JCPLL_SDM_PCW_CTRL		              XFI_PMA_BASE+0xE698
#define AN7583_XFI_PMA_RG_JCPLL_SDM_PCW_CHG_CTRL		          XFI_PMA_BASE+0xE69C
#define AN7583_XFI_PMA_PLL_JCPLL_FT_FREQDET_0		              XFI_PMA_BASE+0xE700
#define AN7583_XFI_PMA_PLL_JCPLL_FT_FREQDET_1		              XFI_PMA_BASE+0xE704
#define AN7583_XFI_PMA_PLL_JCPLL_FT_FREQDET_2		              XFI_PMA_BASE+0xE708
#define AN7583_XFI_PMA_PLL_JCPLL_FT_FREQDET_3		              XFI_PMA_BASE+0xE70C
#define AN7583_XFI_PMA_JCPLL_FT_CK_MUX		                      XFI_PMA_BASE+0xE710
#define AN7583_XFI_PMA_JCPLL_DA_RG_CTRL_0		                  XFI_PMA_BASE+0xE714
#define AN7583_XFI_PMA_JCPLL_DA_RG_CTRL_1		                  XFI_PMA_BASE+0xE718
#define AN7583_XFI_PMA_JCPLL_DA_RG_CTRL_2		                  XFI_PMA_BASE+0xE71C
#define AN7583_XFI_PMA_XFI_PLL_DA_RG_CTRL_0		                  XFI_PMA_BASE+0xE720
#define AN7583_XFI_PMA_XFI_PLL_DA_RG_CTRL_1		                  XFI_PMA_BASE+0xE724
#define AN7583_XFI_PMA_XFI_PLL_DA_RG_CTRL_2		                  XFI_PMA_BASE+0xE728
#define AN7583_XFI_PMA_AD_DA_PROBE_STS_0		                  XFI_PMA_BASE+0xE72C
#define AN7583_XFI_PMA_AD_DA_PROBE_STS_1		                  XFI_PMA_BASE+0xE730
#define AN7583_XFI_PMA_JCPLL_SCAN_CTRL		                      XFI_PMA_BASE+0xE734
#define AN7583_XFI_PMA_XFI_GPIO_SEL_0		                      XFI_PMA_BASE+0xE738
#define AN7583_XFI_PMA_TX_PAR_PROBE_PLL_CTRL	                  XFI_PMA_BASE+0xE73C
#define AN7583_XFI_PMA_DUMP_RX_PCSRST_CTRL		                  XFI_PMA_BASE+0xE740
#define AN7583_XFI_PMA_XFI_TX_FIFO_STS		                      XFI_PMA_BASE+0xE744
#define AN7583_XFI_PMA_JCPLL_CK_RSTB_CTRL		                  XFI_PMA_BASE+0xE748
#define AN7583_XFI_PMA_RO_TDC_TX_FREQDET		                  XFI_PMA_BASE+0xE74C
#define AN7583_XFI_PMA_RO_JCPLL_FT_FREQDET		                  XFI_PMA_BASE+0xE750
#define AN7583_XFI_PMA_RO_JCPLL_500M_FREQDET		              XFI_PMA_BASE+0xE754
#define AN7583_XFI_PMA_RG_JCPLL_500M_FREQ_DET_1		              XFI_PMA_BASE+0xE758
#define AN7583_XFI_PMA_RG_JCPLL_500M_FREQ_DET_2		              XFI_PMA_BASE+0xE75C
#define AN7583_XFI_PMA_RG_JCPLL_500M_FREQ_DET_3		              XFI_PMA_BASE+0xE760
#define AN7583_XFI_PMA_RG_JCPLL_500M_FREQ_DET_4		              XFI_PMA_BASE+0xE764
#define AN7583_XFI_PMA_rg_da_pxp_jcpll_sdm_scan		              XFI_PMA_BASE+0xE768
#define AN7583_XFI_PMA_rg_force_da_pxp_aeq_speed	              XFI_PMA_BASE+0xE76C
#define AN7583_XFI_PMA_rg_force_da_pxp_aeq_mode		              XFI_PMA_BASE+0xE770
#define AN7583_XFI_PMA_rg_force_da_pxp_tx_data_ben		          XFI_PMA_BASE+0xE774
#define AN7583_XFI_PMA_rg_force_da_pxp_tx_fir_c0b		          XFI_PMA_BASE+0xE778
#define AN7583_XFI_PMA_rg_force_da_pxp_tx_term_sel		          XFI_PMA_BASE+0xE77C
#define AN7583_XFI_PMA_rg_force_da_pxp_tx_fir_c1		          XFI_PMA_BASE+0xE780
#define AN7583_XFI_PMA_rg_force_da_pxp_tx_rate_ctrl		          XFI_PMA_BASE+0xE784
#define AN7583_XFI_PMA_rg_force_da_pxp_rx_dac_d0		          XFI_PMA_BASE+0xE788
#define AN7583_XFI_PMA_rg_force_da_pxp_rx_dac_e0		          XFI_PMA_BASE+0xE78C
#define AN7583_XFI_PMA_rg_force_da_pxp_cdr_pr_fll_cor	          XFI_PMA_BASE+0xE790
#define AN7583_XFI_PMA_rg_force_da_pxp_cdr_pr_idac		          XFI_PMA_BASE+0xE794
#define AN7583_XFI_PMA_rg_force_da_pxp_txpll_sdm_pcw	          XFI_PMA_BASE+0xE798
#define AN7583_XFI_PMA_rg_force_da_pxp_rx_fe_vos		          XFI_PMA_BASE+0xE79C
#define AN7583_XFI_PMA_rg_force_da_pxp_jcpll_sdm_pcw	          XFI_PMA_BASE+0xE800
#define AN7583_XFI_PMA_rg_force_da_pcie_cktx0_en		          XFI_PMA_BASE+0xE804
#define AN7583_XFI_PMA_rg_force_da_pxp_aeq_en		              XFI_PMA_BASE+0xE808
#define AN7583_XFI_PMA_rg_force_da_pxp_aeq_bypass		          XFI_PMA_BASE+0xE80C
#define AN7583_XFI_PMA_rg_force_da_pxp_aeq_inprgrss		          XFI_PMA_BASE+0xE810
#define AN7583_XFI_PMA_rg_force_da_pxp_aeq_rstb		              XFI_PMA_BASE+0xE814
#define AN7583_XFI_PMA_rg_force_da_pxp_cdr_lpf_lck2data	          XFI_PMA_BASE+0xE818
#define AN7583_XFI_PMA_rg_force_da_pxp_cdr_pd_pwdb		          XFI_PMA_BASE+0xE81C
#define AN7583_XFI_PMA_rg_force_da_pxp_cdr_pr_lpf_c_en		      XFI_PMA_BASE+0xE820
#define AN7583_XFI_PMA_rg_force_da_pxp_cdr_pr_pieye_pwdb		  XFI_PMA_BASE+0xE824
#define AN7583_XFI_PMA_rg_force_da_pxp_jcpll_ckout_en		      XFI_PMA_BASE+0xE828
#define AN7583_XFI_PMA_rg_force_da_pxp_jcpll_kband_scan_en		  XFI_PMA_BASE+0xE82C
#define AN7583_XFI_PMA_rg_force_da_pxp_jcpll_kband_scan_in		  XFI_PMA_BASE+0xE830
#define AN7583_XFI_PMA_rg_force_da_pxp_jcpll_sdm_pcw_chg		  XFI_PMA_BASE+0xE834
#define AN7583_XFI_PMA_rg_force_da_pxp_jcpll_sdm_scan_en_cg		  XFI_PMA_BASE+0xE838
#define AN7583_XFI_PMA_rg_force_da_pxp_jcpll_sdm_scan_rstb		  XFI_PMA_BASE+0xE83C
#define AN7583_XFI_PMA_rg_force_da_pxp_rx_oscal_en		          XFI_PMA_BASE+0xE840
#define AN7583_XFI_PMA_rg_force_da_pxp_rx_scan_enable		      XFI_PMA_BASE+0xE844
#define AN7583_XFI_PMA_rg_force_da_pxp_rx_scan_in		          XFI_PMA_BASE+0xE848
#define AN7583_XFI_PMA_rg_force_da_pxp_rx_scan_rst_b		      XFI_PMA_BASE+0xE84C
#define AN7583_XFI_PMA_rg_force_da_pxp_tdc_cal_bw		          XFI_PMA_BASE+0xE850
#define AN7583_XFI_PMA_rg_force_da_pxp_txpll_ckout_en		      XFI_PMA_BASE+0xE854
#define AN7583_XFI_PMA_rg_force_da_pxp_txpll_kband_load_en		  XFI_PMA_BASE+0xE858
#define AN7583_XFI_PMA_rg_force_da_pxp_txpll_kband_scan_en_cg	  XFI_PMA_BASE+0xE85C
#define AN7583_XFI_PMA_rg_da_pxp_txpll_kband_scan		          XFI_PMA_BASE+0xE860
#define AN7583_XFI_PMA_rg_force_da_pxp_txpll_sdm_pcw_chg		  XFI_PMA_BASE+0xE864
#define AN7583_XFI_PMA_rg_force_da_pxp_txpll_sdm_scan_en_cg		  XFI_PMA_BASE+0xE868
#define AN7583_XFI_PMA_rg_da_pxp_txpll_sdm_scan		              XFI_PMA_BASE+0xE86C
#define AN7583_XFI_PMA_rg_force_da_pxp_tx_acjtag_dn		          XFI_PMA_BASE+0xE870
#define AN7583_XFI_PMA_rg_force_da_pxp_tx_acjtag_en		          XFI_PMA_BASE+0xE874
#define AN7583_XFI_PMA_rg_force_da_pxp_tx_ck_en		              XFI_PMA_BASE+0xE878
#define AN7583_XFI_PMA_rg_force_da_pxp_tx_hsdata_en		          XFI_PMA_BASE+0xE87C
#define AN7583_XFI_PMA_rg_force_da_pxp_tx_rxdet_en		          XFI_PMA_BASE+0xE880
#define AN7583_XFI_PMA_scan_mode		                          XFI_PMA_BASE+0xE884
#define AN7583_XFI_PMA_rg_da_pxp_jcpll_kband_scan		          XFI_PMA_BASE+0xE888
#define AN7583_XFI_PMA_rg_force_da_pxp_rx_fe_gain_ctrl		      XFI_PMA_BASE+0xE88C
#define AN7583_XFI_PMA_rg_force_da_pxp_aeq_rmtxskip		          XFI_PMA_BASE+0xE890
#define AN7583_XFI_PMA_rg_force_da_pxp_rx_fe_pwdb		          XFI_PMA_BASE+0xE894
#define AN7583_XFI_PMA_rg_force_da_pxp_rx_sigdet_cal_en		      XFI_PMA_BASE+0xE898
#define AN7583_XFI_PMA_ADD_DIG_RESERVE_5		                  XFI_PMA_BASE+0xE89C
#define AN7583_XFI_PMA_ADD_DIG_RESERVE_6		                  XFI_PMA_BASE+0xE8A0
#define AN7583_XFI_PMA_ADD_DIG_RESERVE_7		                  XFI_PMA_BASE+0xE8A4
#define AN7583_XFI_PMA_ADD_DIG_RESERVE_8		                  XFI_PMA_BASE+0xE8A8
#define AN7583_XFI_PMA_ADD_DIG_RESERVE_9		                  XFI_PMA_BASE+0xE8AC
#define AN7583_XFI_PMA_ADD_DIG_RESERVE_10		                  XFI_PMA_BASE+0xE8B0
#define AN7583_XFI_PMA_ADD_DIG_RESERVE_11		                  XFI_PMA_BASE+0xE8B4
#define AN7583_XFI_PMA_ADD_DIG_RESERVE_12		                  XFI_PMA_BASE+0xE8B8
#define AN7583_XFI_PMA_ADD_DIG_RESERVE_13		                  XFI_PMA_BASE+0xE8BC
#define AN7583_XFI_PMA_ADD_DIG_RESERVE_14		                  XFI_PMA_BASE+0xE8C0
#define AN7583_XFI_PMA_ADD_DIG_RO_RESERVE_5		                  XFI_PMA_BASE+0xE8C4
#define AN7583_XFI_PMA_ADD_DIG_RO_RESERVE_6		                  XFI_PMA_BASE+0xE8C8
#define AN7583_XFI_PMA_ADD_DIG_RO_RESERVE_7		                  XFI_PMA_BASE+0xE8CC
#define AN7583_XFI_PMA_ADD_DIG_RO_RESERVE_8		                  XFI_PMA_BASE+0xE8D0
#define AN7583_XFI_PMA_ADD_DIG_RO_RESERVE_9		                  XFI_PMA_BASE+0xE8D4
#define AN7583_XFI_PMA_ADD_DIG_RESERVE_15		                  XFI_PMA_BASE+0xE8D8
#define AN7583_XFI_PMA_ADD_DIG_RESERVE_16		                  XFI_PMA_BASE+0xE8DC
#define AN7583_XFI_PMA_ADD_DIG_RESERVE_17		                  XFI_PMA_BASE+0xE8E0
#define AN7583_XFI_PMA_ADD_DIG_RESERVE_18		                  XFI_PMA_BASE+0xE8E4
#define AN7583_XFI_PMA_ADD_DIG_RESERVE_19		                  XFI_PMA_BASE+0xE8E8
#define AN7583_XFI_PMA_ADD_DIG_RESERVE_20		                  XFI_PMA_BASE+0xE8EC
#define AN7583_XFI_PMA_ADD_DIG_RESERVE_21		                  XFI_PMA_BASE+0xE8F0
#define AN7583_XFI_PMA_ADD_DIG_RESERVE_22		                  XFI_PMA_BASE+0xE8F4
#define AN7583_XFI_PMA_ADD_DIG_RESERVE_23		                  XFI_PMA_BASE+0xE8F8
#define AN7583_XFI_PMA_ADD_DIG_RESERVE_24		                  XFI_PMA_BASE+0xE8FC
#define AN7583_XFI_PMA_ADD_DIG_RESERVE_25		                  XFI_PMA_BASE+0xE900
#define AN7583_XFI_PMA_ADD_DIG_RESERVE_26		                  XFI_PMA_BASE+0xE904
#define AN7583_XFI_PMA_ADD_DIG_RESERVE_27		                  XFI_PMA_BASE+0xE908
#define AN7583_XFI_PMA_ADD_DIG_RESERVE_28		                  XFI_PMA_BASE+0xE90C
#define AN7583_XFI_PMA_ADD_DIG_RESERVE_29		                  XFI_PMA_BASE+0xE910
#define AN7583_XFI_PMA_ADD_DIG_RESERVE_30		                  XFI_PMA_BASE+0xE914
#define AN7583_XFI_PMA_ADD_DIG_RESERVE_31		                  XFI_PMA_BASE+0xE918
#define AN7583_XFI_PMA_ADD_DIG_RESERVE_32		                  XFI_PMA_BASE+0xE91C
#define AN7583_XFI_PMA_ADD_DIG_RESERVE_33		                  XFI_PMA_BASE+0xE920
#define AN7583_XFI_PMA_ADD_DIG_RESERVE_34		                  XFI_PMA_BASE+0xE924
#define AN7583_XFI_PMA_ADD_DIG_RESERVE_35		                  XFI_PMA_BASE+0xE928
#define AN7583_XFI_PMA_ADD_DIG_RESERVE_36		                  XFI_PMA_BASE+0xE92C
#define AN7583_XFI_PMA_ADD_DIG_RESERVE_37		                  XFI_PMA_BASE+0xE930
#define AN7583_XFI_PMA_ADD_DIG_RESERVE_38		                  XFI_PMA_BASE+0xE934
#define AN7583_XFI_PMA_ADD_DIG_RESERVE_39		                  XFI_PMA_BASE+0xE938
#define AN7583_XFI_PMA_ADD_DIG_RESERVE_40		                  XFI_PMA_BASE+0xE93C
#define AN7583_XFI_PMA_ADD_DIG_RESERVE_41		                  XFI_PMA_BASE+0xE940
#define AN7583_XFI_PMA_ADD_DIG_RESERVE_42		                  XFI_PMA_BASE+0xE944
#define AN7583_XFI_PMA_ADD_DIG_RESERVE_43		                  XFI_PMA_BASE+0xE948
#define AN7583_XFI_PMA_ADD_DIG_RESERVE_44		                  XFI_PMA_BASE+0xE94C
#define AN7583_XFI_PMA_ADD_DIG_RESERVE_45		                  XFI_PMA_BASE+0xE950
#define AN7583_XFI_PMA_ADD_DIG_RESERVE_46		                  XFI_PMA_BASE+0xE954
#define AN7583_XFI_PMA_ADD_DIG_RESERVE_47		                  XFI_PMA_BASE+0xE958
#define AN7583_XFI_PMA_ADD_DIG_RESERVE_48		                  XFI_PMA_BASE+0xE95C
#define AN7583_XFI_PMA_BEN_CTRL_0		                          XFI_PMA_BASE+0xE960
#define AN7583_XFI_PMA_BEN_CTRL_1		                          XFI_PMA_BASE+0xE964
#define AN7583_XFI_PMA_BEN_STS_0		                          XFI_PMA_BASE+0xE968
#define AN7583_XFI_PMA_BEN_STS_1		                          XFI_PMA_BASE+0xE96C
#define AN7583_XFI_PMA_BEN_STS_2		                          XFI_PMA_BASE+0xE970
#define AN7583_XFI_PMA_BEN_STS_3		                          XFI_PMA_BASE+0xE974
#define AN7583_XFI_PMA_TX_SD_CTRL_0		                          XFI_PMA_BASE+0xE978
#define AN7583_XFI_PMA_TX_SD_CTRL_1		                          XFI_PMA_BASE+0xE97C
#define AN7583_XFI_PMA_TX_SD_STS_0		                          XFI_PMA_BASE+0xE980
#define AN7583_XFI_PMA_TX_SD_STS_1		                          XFI_PMA_BASE+0xE984
#define AN7583_XFI_PMA_TX_SD_STS_2		                          XFI_PMA_BASE+0xE988
#define AN7583_XFI_PMA_TX_SD_STS_3		                          XFI_PMA_BASE+0xE98C



//AN7583_ANA_CODA
#define AN7583_XFI_ANA_RG_XPON_CMN_EN                              XFI_PMA_BASE+0xF000
#define AN7583_XFI_ANA_RG_XPON_CMN_MODE_SEL                        XFI_PMA_BASE+0xF004
#define AN7583_XFI_ANA_RG_DUMMY1                                   XFI_PMA_BASE+0xF008
#define AN7583_XFI_ANA_RG_DUMMY2                                   XFI_PMA_BASE+0xF00C
#define AN7583_XFI_ANA_RG_DUMMY3                                   XFI_PMA_BASE+0xF010
#define AN7583_XFI_ANA_RG_DUMMY4                                   XFI_PMA_BASE+0xF014
#define AN7583_XFI_ANA_RG_DUMMY5                                   XFI_PMA_BASE+0xF018
#define AN7583_XFI_ANA_RG_DUMMY6                                   XFI_PMA_BASE+0xF01C
#define AN7583_XFI_ANA_RG_DUMMY7                                   XFI_PMA_BASE+0xF020
#define AN7583_XFI_ANA_RG_DUMMY8                                   XFI_PMA_BASE+0xF024
#define AN7583_XFI_ANA_RG_DUMMY9                                   XFI_PMA_BASE+0xF028
#define AN7583_XFI_ANA_RG_DUMMY10                                  XFI_PMA_BASE+0xF02C
#define AN7583_XFI_ANA_RG_DUMMY11                                  XFI_PMA_BASE+0xF030
#define AN7583_XFI_ANA_RG_DUMMY12                                  XFI_PMA_BASE+0xF034
#define AN7583_XFI_ANA_RG_DUMMY13                                  XFI_PMA_BASE+0xF038
#define AN7583_XFI_ANA_RG_DUMMY14                                  XFI_PMA_BASE+0xF03C
#define AN7583_XFI_ANA_RG_DUMMY15                                  XFI_PMA_BASE+0xF040
#define AN7583_XFI_ANA_RG_DUMMY16                                  XFI_PMA_BASE+0xF044
#define AN7583_XFI_ANA_RG_DUMMY17                                  XFI_PMA_BASE+0xF048
#define AN7583_XFI_ANA_RG_XPON_TXPLL_IB_EXT_EN                     XFI_PMA_BASE+0xF04C
#define AN7583_XFI_ANA_RG_XPON_TXPLL_CHP_IBIAS                     XFI_PMA_BASE+0xF050
#define AN7583_XFI_ANA_RG_XPON_TXPLL_LPF_BP                        XFI_PMA_BASE+0xF054
#define AN7583_XFI_ANA_RG_XPON_TXPLL_KBAND_CODE                    XFI_PMA_BASE+0xF058
#define AN7583_XFI_ANA_RG_XPON_TXPLL_KBAND_KS                      XFI_PMA_BASE+0xF05C
#define AN7583_XFI_ANA_RG_XPON_TXPLL_PHY_CK1_EN                    XFI_PMA_BASE+0xF060
#define AN7583_XFI_ANA_RG_XPON_TXPLL_REFIN_INTERNAL                XFI_PMA_BASE+0xF064
#define AN7583_XFI_ANA_RG_XPON_TXPLL_SDM_DI_EN                     XFI_PMA_BASE+0xF068
#define AN7583_XFI_ANA_RG_XPON_TXPLL_SDM_ORD                       XFI_PMA_BASE+0xF06C
#define AN7583_XFI_ANA_RG_XPON_TXPLL_TCL_AMP_GAIN                  XFI_PMA_BASE+0xF070
#define AN7583_XFI_ANA_RG_XPON_TXPLL_TCL_LPF_EN                    XFI_PMA_BASE+0xF074
#define AN7583_XFI_ANA_RG_XPON_TXPLL_VCO_HALFLSB_EN                XFI_PMA_BASE+0xF078
#define AN7583_XFI_ANA_RG_XPON_TXPLL_SSC_EN                        XFI_PMA_BASE+0xF07C
#define AN7583_XFI_ANA_RG_XPON_TXPLL_SSC_DELTA1                    XFI_PMA_BASE+0xF080
#define AN7583_XFI_ANA_RG_XPON_TXPLL_SSC_PERIOD                    XFI_PMA_BASE+0xF084
#define AN7583_XFI_ANA_RG_XPON_TXPLL_VTP_EN                        XFI_PMA_BASE+0xF088
#define AN7583_XFI_ANA_RG_XPON_TXPLL_VCO_VTP_EN                    XFI_PMA_BASE+0xF08C
#define AN7583_XFI_ANA_RG_XPON_TXPLL_TCL_VTP_EN                    XFI_PMA_BASE+0xF090
#define AN7583_XFI_ANA_RG_XPON_TXPLL_TCL_KBAND_VREF                XFI_PMA_BASE+0xF094
#define AN7583_XFI_ANA_RG_XPON_TXPLL_CHP_DOUBLE_EN                 XFI_PMA_BASE+0xF098
#define AN7583_XFI_ANA_RG_DUMMY18                                  XFI_PMA_BASE+0xF09C
#define AN7583_XFI_ANA_RG_XPON_PLL_MONCLK_SEL                      XFI_PMA_BASE+0xF0A0
#define AN7583_XFI_ANA_RG_XPON_TDC_TXCK_SEL                        XFI_PMA_BASE+0xF0A4
#define AN7583_XFI_ANA_RG_XPON_TDC_SYNC_CK_SEL                     XFI_PMA_BASE+0xF0A8
#define AN7583_XFI_ANA_RG_XPON_DUMMY                               XFI_PMA_BASE+0xF0AC
#define AN7583_XFI_ANA_RG_XPON_RO_TXPLL_KBAND_CODE                 XFI_PMA_BASE+0xF0B0
#define AN7583_XFI_ANA_RG_XPON_TX_SER_LOADSEL                      XFI_PMA_BASE+0xF0B4
#define AN7583_XFI_ANA_RG_XPON_TX_CKMON_SEL                        XFI_PMA_BASE+0xF0B8
#define AN7583_XFI_ANA_RG_XPON_TX_FIR_CHGPLR_C1                    XFI_PMA_BASE+0xF0BC
#define AN7583_XFI_ANA_RG_XPON_TX_TXLBRX_EN                        XFI_PMA_BASE+0xF0C0
#define AN7583_XFI_ANA_RG_XPON_TX_CKLDO_EN                         XFI_PMA_BASE+0xF0C4
#define AN7583_XFI_ANA_RG_XPON_TX_TERMCAL_SELPN                    XFI_PMA_BASE+0xF0C8
#define AN7583_XFI_ANA_RG_XPON_RX_BUSBIT_SEL                       XFI_PMA_BASE+0xF0CC
#define AN7583_XFI_ANA_RG_XPON_RX_MPXSEL                           XFI_PMA_BASE+0xF0D0
#define AN7583_XFI_ANA_RG_XPON_RX_REV_0                            XFI_PMA_BASE+0xF0D4
#define AN7583_XFI_ANA_RG_XPON_RX_PHYCK_DIV                        XFI_PMA_BASE+0xF0D8
#define AN7583_XFI_ANA_RG_XPON_CDR_PD_PICAL_CKD8_INV               XFI_PMA_BASE+0xF0DC
#define AN7583_XFI_ANA_RG_XPON_CDR_LPF_BOT_LIM                     XFI_PMA_BASE+0xF0E0
#define AN7583_XFI_ANA_RG_XPON_CDR_LPF_KP_GAIN                     XFI_PMA_BASE+0xF0E4
#define AN7583_XFI_ANA_RG_XPON_CDR_LPF_RATIO                       XFI_PMA_BASE+0xF0E8
#define AN7583_XFI_ANA_RG_XPON_CDR_LPF_SNAPSHOT                    XFI_PMA_BASE+0xF0EC
#define AN7583_XFI_ANA_RG_XPON_CDR_LPF_SETVALUE                    XFI_PMA_BASE+0xF0F0
#define AN7583_XFI_ANA_RG_XPON_CDR_PR_INJ_MODE                     XFI_PMA_BASE+0xF0F4
#define AN7583_XFI_ANA_RG_XPON_CDR_PR_BETA_DAC                     XFI_PMA_BASE+0xF0F8
#define AN7583_XFI_ANA_RG_XPON_CDR_PR_VREG_IBAND_VAL               XFI_PMA_BASE+0xF0FC
#define AN7583_XFI_ANA_RG_XPON_CDR_PR_CKREF_DIV                    XFI_PMA_BASE+0xF100
#define AN7583_XFI_ANA_RG_XPON_CDR_PR_MONCK_EN                     XFI_PMA_BASE+0xF104
#define AN7583_XFI_ANA_RG_XPON_CDR_PR_TDC_REF_SEL                  XFI_PMA_BASE+0xF108
#define AN7583_XFI_ANA_RG_XPON_CDR_PR_MONPR_EN                     XFI_PMA_BASE+0xF10C
#define AN7583_XFI_ANA_RG_XPON_RX_DAC_RANGE                        XFI_PMA_BASE+0xF110
#define AN7583_XFI_ANA_RG_XPON_RX_SIGDET_NOVTH                     XFI_PMA_BASE+0xF114
#define AN7583_XFI_ANA_RG_XPON_RX_FE_EQ_HZEN                       XFI_PMA_BASE+0xF118
#define AN7583_XFI_ANA_RG_XPON_RX_FE_VCM_GEN_PWDB                  XFI_PMA_BASE+0xF11C
#define AN7583_XFI_ANA_RG_XPON_RX_OSCAL_FORCE                      XFI_PMA_BASE+0xF120
#define AN7583_XFI_ANA_RG_DUMMY19                                  XFI_PMA_BASE+0xF124
#define AN7583_XFI_ANA_RG_XPON_RX_OSCAL_VGA1IOS                    XFI_PMA_BASE+0xF128
#define AN7583_XFI_ANA_RG_XPON_RX_OSCAL_VGA2IOS                    XFI_PMA_BASE+0xF12C
#define AN7583_XFI_ANA_RG_DUMMY20                                  XFI_PMA_BASE+0xF130
#define AN7583_XFI_ANA_RG_DUMMY21                                  XFI_PMA_BASE+0xF134
#define AN7583_XFI_ANA_RG_XPON_AEQ_OPTION3                         XFI_PMA_BASE+0xF138
#define AN7583_XFI_ANA_RG_DUMMY22                                  XFI_PMA_BASE+0xF13C
#define AN7583_XFI_ANA_RG_DUMMY23                                  XFI_PMA_BASE+0xF140
#define AN7583_XFI_ANA_RG_XPON_RX_FE_PEAKING_CTRL_MSB              XFI_PMA_BASE+0xF144




//BCDR_CODA
#define AN7583_XFI_OLT_BCDR_CORE_CTRL                           GPON_XFI_OLT_BCDR_BASE+0x5800
#define AN7583_XFI_OLT_BCDR_MISC_CTRL1                          GPON_XFI_OLT_BCDR_BASE+0x5804
#define AN7583_XFI_OLT_BCDR_MISC_CTRL2                          GPON_XFI_OLT_BCDR_BASE+0x5808
#define AN7583_XFI_OLT_BCDR_INT_ENABLE                          GPON_XFI_OLT_BCDR_BASE+0x580C
#define AN7583_XFI_OLT_BCDR_INT_STS                             GPON_XFI_OLT_BCDR_BASE+0x5810
#define AN7583_XFI_OLT_BCDR_PROBE_SEL                           GPON_XFI_OLT_BCDR_BASE+0x5814
#define AN7583_XFI_OLT_BCDR_PROBE_L32                           GPON_XFI_OLT_BCDR_BASE+0x5818
#define AN7583_XFI_OLT_BCDR_PROBE_H32                           GPON_XFI_OLT_BCDR_BASE+0x581C
#define AN7583_XFI_OLT_BCDR_DUMP_TRIG                           GPON_XFI_OLT_BCDR_BASE+0x5820
#define AN7583_XFI_OLT_BCDR_DBG_CTRL                            GPON_XFI_OLT_BCDR_BASE+0x5824
#define AN7583_XFI_OLT_BCDR_BCDRERR_CNT                         GPON_XFI_OLT_BCDR_BASE+0x5828
#define AN7583_XFI_OLT_BCDR_DOUTERR_CNT                         GPON_XFI_OLT_BCDR_BASE+0x582C
#define AN7583_XFI_OLT_BCDR_DOUT79_CNT                          GPON_XFI_OLT_BCDR_BASE+0x5830
#define AN7583_XFI_OLT_BCDR_DATAIN_L32                          GPON_XFI_OLT_BCDR_BASE+0x5834
#define AN7583_XFI_OLT_BCDR_DATAIN_H32                          GPON_XFI_OLT_BCDR_BASE+0x5838
#define AN7583_XFI_OLT_BCDR_DBG_MAP                             GPON_XFI_OLT_BCDR_BASE+0x583C
#define AN7583_XFI_OLT_BCDR_PRBS_FIFOST                         GPON_XFI_OLT_BCDR_BASE+0x5840
#define AN7583_XFI_OLT_BCDR_PRBS_DATA                           GPON_XFI_OLT_BCDR_BASE+0x5844
#define AN7583_XFI_OLT_BCDR_SRAM_MISC                           GPON_XFI_OLT_BCDR_BASE+0x584C
#define AN7583_XFI_OLT_BCDR_DMP_TRIG_CTRL1                      GPON_XFI_OLT_BCDR_BASE+0x5850
#define AN7583_XFI_OLT_BCDR_DMP_TRIG_CTRL2                      GPON_XFI_OLT_BCDR_BASE+0x5854
#define AN7583_XFI_OLT_BCDR_DMP_TRIG_CTRL3                      GPON_XFI_OLT_BCDR_BASE+0x5858
#define AN7583_XFI_OLT_BCDR_DUMP_WR                             GPON_XFI_OLT_BCDR_BASE+0x585C
#define AN7583_XFI_OLT_BCDR_DUMP_WR_ST                          GPON_XFI_OLT_BCDR_BASE+0x5860
#define AN7583_XFI_OLT_BCDR_DUMP_RD                             GPON_XFI_OLT_BCDR_BASE+0x5864
#define AN7583_XFI_OLT_BCDR_DUMP_RDATA0                         GPON_XFI_OLT_BCDR_BASE+0x5868
#define AN7583_XFI_OLT_BCDR_DUMP_RDATA1                         GPON_XFI_OLT_BCDR_BASE+0x586C
#define AN7583_XFI_OLT_BCDR_DUMP_RDATA2                         GPON_XFI_OLT_BCDR_BASE+0x5874
#define AN7583_XFI_OLT_BCDR_SS_SAMPLE_PNT                       GPON_XFI_OLT_BCDR_BASE+0x5880
#define AN7583_XFI_OLT_BCDR_SS_SHIFT_CNT                        GPON_XFI_OLT_BCDR_BASE+0x5884
#define AN7583_XFI_OLT_BCDR_SS_ZEROTWO_CNT                      GPON_XFI_OLT_BCDR_BASE+0x5888
#define AN7583_XFI_OLT_BCDR_SS_SHORT_CNT                        GPON_XFI_OLT_BCDR_BASE+0x588C
#define AN7583_XFI_OLT_BCDR_SS_ERR_CNT                          GPON_XFI_OLT_BCDR_BASE+0x5890
#define AN7583_XFI_OLT_FPGA_SERDES_STATUS                       GPON_XFI_OLT_BCDR_BASE+0x5894
#define AN7583_XFI_OLT_GDMP_TRG_PATN0_L                         GPON_XFI_OLT_BCDR_BASE+0x58A0
#define AN7583_XFI_OLT_GDMP_TRG_PATN0_MSK_L                     GPON_XFI_OLT_BCDR_BASE+0x58A4
#define AN7583_XFI_OLT_GDMP_TRG_PATN0_H                         GPON_XFI_OLT_BCDR_BASE+0x58A8
#define AN7583_XFI_OLT_GDMP_TRG_PATN0_MSK_H                     GPON_XFI_OLT_BCDR_BASE+0x58AC
#define AN7583_XFI_OLT_GDMP_TRG_PATN1_L                         GPON_XFI_OLT_BCDR_BASE+0x58B0
#define AN7583_XFI_OLT_GDMP_TRG_PATN1_MSK_L                     GPON_XFI_OLT_BCDR_BASE+0x58B4
#define AN7583_XFI_OLT_GDMP_TRG_PATN1_H                         GPON_XFI_OLT_BCDR_BASE+0x58B8
#define AN7583_XFI_OLT_GDMP_TRG_PATN1_MSK_H                     GPON_XFI_OLT_BCDR_BASE+0x58BC
#define AN7583_XFI_OLT_GDMP_TRG_MODE                            GPON_XFI_OLT_BCDR_BASE+0x58C0


//BCDR_BIST
#define AN7583_XFI_OLT_BCDR_csr_clear_en                        GPON_XFI_OLT_BCDR_BASE+0x5900
#define AN7583_XFI_OLT_BCDR_csr_prbs_align_pattern              GPON_XFI_OLT_BCDR_BASE+0x5904
#define AN7583_XFI_OLT_BCDR_csr_align_mask                      GPON_XFI_OLT_BCDR_BASE+0x5908
#define AN7583_XFI_OLT_BCDR_csr_bistctl_sel                     GPON_XFI_OLT_BCDR_BASE+0x590C
#define AN7583_XFI_OLT_BCDR_csr_cid_length                      GPON_XFI_OLT_BCDR_BASE+0x5910
#define AN7583_XFI_OLT_BCDR_csr_timer_set_low                   GPON_XFI_OLT_BCDR_BASE+0x5914
#define AN7583_XFI_OLT_BCDR_csr_timer_set_high                  GPON_XFI_OLT_BCDR_BASE+0x5918
#define AN7583_XFI_OLT_BCDR_csr_check_cnt_low                   GPON_XFI_OLT_BCDR_BASE+0x591C
#define AN7583_XFI_OLT_BCDR_csr_check_cnt_high                  GPON_XFI_OLT_BCDR_BASE+0x5920
#define AN7583_XFI_OLT_BCDR_csr_err_tran_cnt                    GPON_XFI_OLT_BCDR_BASE+0x5924
#define AN7583_XFI_OLT_BCDR_csr_err_bit_low                     GPON_XFI_OLT_BCDR_BASE+0x5928
#define AN7583_XFI_OLT_BCDR_csr_err_bit_high                    GPON_XFI_OLT_BCDR_BASE+0x592C
#define AN7583_XFI_OLT_BCDR_csr_prbs_sts                        GPON_XFI_OLT_BCDR_BASE+0x5930
#define AN7583_XFI_OLT_BCDR_csr_prbs_probe_l                    GPON_XFI_OLT_BCDR_BASE+0x5934
#define AN7583_XFI_OLT_BCDR_csr_prbs_probe_h                    GPON_XFI_OLT_BCDR_BASE+0x5938




int olt_phy_pma_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data);
int olt_phy_pma_write_proc(struct file *file, const char *buffer, unsigned long count, void *data);

void olt_bcdr_dump(void);
int olt_phy_analog_init(void);
void olt_bcdr_sim_setting_dump(void);
void bcdr_dbg_dump(uint mode, uint trig_set, uint dump_set);
void bcdr_int_status (uint mode);






//#define XFI_EOScan  0
#define LAB_PRINT_XFI 0
#define LAB_PON_Test  0
#define Pon_PR_WK 0
#define XFI_PCS_PRINT 0



//define_Bit Rate
#define OLT_GPON_ASYM_MODE		           0x1    //DS(RX)_1.24416G  /  US(TX)_2.48832G
#define OLT_GPON_SYM_MODE		           0x3    //DS(RX)_2.48832G  /  US(TX)_2.48832G
#define OLT_GPON_SYM_MODE_7552	           0x7    //DS(RX)_2.48832G  /  US(TX)_2.48832G
#define OLT_GPON_MIX_MODE		           0x5    //DS(RX)_2.48832G  /  US(TX)_2.48832G
#define OLT_MODE_DEFAULT		           0x0    //DS(RX)_2.48832G  /  US(TX)_2.48832G

#define Eth_Ser_USXGMII                    0xB    //DS(RX)_10.31252G  /  US(TX)_10.3125G
#define Eth_Ser_5GBaseR                    0x55   //DS(RX)_5.15625G   /  US(TX)_5.15625G
#define Eth_Ser_HSGMII                     0x11   //DS(RX)_3.125G      /  US(TX)_3.125G
#define Eth_Ser_SGMII                      0x10   //DS(RX)_1.25G       /  US(TX)_1.25G


//define_TXPLL
#define XFI_12p5G_BYP_JCPLL		           0x1     //TXPLL_12p5G
#define XFI_12p4G_BYP_JCPLL		           0x2     //TXPLL_12p4G
#define XFI_10p3125G_BYP_JCPLL		       0x3     //TXPLL_10p3125G
#define XFI_10G_BYP_JCPLL		           0x4     //TXPLL_10G
#define XFI_6p25G_BYP_JCPLL		           0x5     //TXPLL_6p25G


//define_TX_Spd
#define TX_Spd_USXGMII                     0x1    //DS(RX)_10.31252G  /  US(TX)_10.3125G
#define TX_Spd_5GBaseR                     0x2    //DS(RX)_5.15625G   /  US(TX)_5.15625G
#define TX_Spd_HSGMII                      0x3    //DS(RX)_3.125G      /  US(TX)_3.125G
#define TX_Spd_SGMII                       0x4    //DS(RX)_1.25G       /  US(TX)_1.25G
#define TX_Spd_OLT                         0x5    //DS(RX)_1.24416G  /  US(TX)_2.48832G & DS(RX)_2.48832G  /  US(TX)_2.48832G


//define_RX_Spd
#define RX_Spd_USXGMII                     0x1    //DS(RX)_10.31252G  /  US(TX)_10.3125G
#define RX_Spd_5GBaseR                     0x2    //DS(RX)_5.15625G   /  US(TX)_5.15625G
#define RX_Spd_HSGMII                      0x3    //DS(RX)_3.125G      /  US(TX)_3.125G
#define RX_Spd_SGMII                       0x4    //DS(RX)_1.25G       /  US(TX)_1.25G
#define RX_Spd_Asym_OLT                    0x5    //DS(RX)_1.24416G  /  US(TX)_2.48832G
#define RX_Spd_Sym_OLT                     0x6    //DS(RX)_2.48832G  /  US(TX)_2.48832G




//R2T
#define  Eth_Ser_bist_data         1
#define  Eth_Ser_r2t_data          2
#define  nor_pma_data              3
#define  OLT_bist_data             4
#define  OLT_r2t_data              5



		 
#define  XFI_FIRST_PLUG_IN     1
#define  XFI_PLUG_IN           2
#define  XFI_PLUG_OUT          3


#define  OLT_FIRST_PLUG_IN     1
#define  OLT_PLUG_IN           2
#define  OLT_PLUG_OUT          3



		 
#define  TX_Async_GPON     1   //DS_2.48832G  /  US_1.24416G   
#define  TX_Sync_EPON      2   //DS_1.25G      /  US_1.25G     
#define  TX_Async_XEPON    3   //DS_10.3125G  /  US_1.25G       
#define  TX_Sync_XEPON     4   //DS_10.3125G  /  US_10.3125G   
#define  TX_Async_XGPON    5   //DS_9.95328G  /  US_2.48832G
#define  TX_Sync_XGSPON    6   //DS_9.95328G  /  US_9.95328G   
#define  TX_Sync_GPON      7   //DS_2.48832G  /  US_2.48832G


#define EO_OLT_GPON_ASYM_MODE		          0x1     // DS(RX)_1.24416G   /  US(TX)_2.48832G
#define EO_OLT_GPON_SYM_MODE		          0x3     // DS(RX)_2.48832G   /  US(TX)_2.48832G
#define EO_OLT_GPON_SYM_MODE_7552	          0x7     // DS(RX)_2.48832G   /  US(TX)_2.48832G
#define EO_OLT_GPON_MIX_MODE		          0x5     // DS(RX)_2.48832G   /  US(TX)_2.48832G
#define EO_OLT_MODE_DEFAULT		              0x0     // DS(RX)_2.48832G   /  US(TX)_2.48832G

#define EO_Eth_Ser_USXGMII                    0xB     // DS(RX)_10.31252G  /  US(TX)_10.3125G
#define EO_Eth_Ser_5GBaseR                    0x55    // DS(RX)_5.15625G   /  US(TX)_5.15625G
#define EO_Eth_Ser_HSGMII                     0x11    // DS(RX)_3.125G      /  US(TX)_3.125G
#define EO_Eth_Ser_SGMII                      0x10    // DS(RX)_1.25G       /  US(TX)_1.25G





void XFI_Init (int spd_sel);
void XFI_LinkControl (void);
void OLT_plug_reset (int spd_sel);
void XFI_CfgPhyType(int spd_sel);
void XFI_DIG_reset_hold(uint spd);
void XFI_DIG_ref_release(void);
void XFI_DIG_reset_release(uint spd);
void XFI_WanSelInit (uint xfi_Spd);
void XFI_TXPLL(uint xfi_txpll);
void XFI_TX(uint xfi_tx);
void XFI_RX(uint xfi_rx);
void XFI_ANA(uint xfi_rxana);
void XFI_TXPLL_on(void);
void XFI_TX_on(void);
void XFI_RX_preset(uint xfi_preset);
void XFI_RX_on(uint xfi_rxon);
//void XFI_RX_L2R(void);
void XFI_RX_L2R(uint spd);

void XFI_RX_L2D(void);
void XFI_RX_OSCal(void);
void XFI_RX_pical(void);
void XFI_RX_pdos(void);
void XFI_RX_feos(void);
void XFI_RX_sdcal(void);
void XFI_phy_status(void);
void XFI_OLT_Eth_DIG_reset(void);
void XFI_RX_rxrdy(void);
void XFI_OLT_RX_rxrdy(void);
void XFI_OLT_BIST_setting(void);
void XFI_OLT_BIST_on(void);
void XFI_Eth_OLT_R2T_sel(uint r2t_sel);

//int XFI_readout_EO(int sweep_direction);
void XFI_readout_EO(int sweep_direction , volatile int *R_fom_num , volatile int *R_veo ,uint XFI_Eye_Scan);

void XFI_eye_Cal(void);
void XFI_eye_setting(uint bit_set);
void XFI_EO_Scan(uint bit_sel, uint EO_start, uint EO_end , uint XFI_Eye_Scan);
void XFI_eye_EO(uint bit_rate, volatile int *fom_num , volatile int *veo, uint XFI_Eye_Scan);
uint XFI_eyescan_moveX(uint EYE_X_HW, uint EYE_Y_HW,uint bit_sel );
int XFI_eyescan_countPoint(uint EYE_X_FW, uint EYE_Y_FW, uint bit_sel);

void Eth_Ser_Ext_T2R (int xfi_Spd);








#endif /* _PHY_INIT_H_ */




