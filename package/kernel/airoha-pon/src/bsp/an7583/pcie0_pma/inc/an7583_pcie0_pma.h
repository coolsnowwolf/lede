#ifndef _AN7583_PCIE0_PMA_H_
#define _AN7583_PCIE0_PMA_H_




#define PCIE0_ANA_RG_BASE  (0x1FC7F000)  // only for asic
#define PCIE0_PMA_RG_BASE  (0x1FC7E000)  // only for asic

#define IO_GPHYREG(reg)						get_pcie0_phya_data(reg)//ioread32((void __iomem *)(reg)) /* GetReg((uint)reg) */
#define IO_SPHYREG(reg, val)				set_pcie0_phya_data(reg, val) /* SetReg((uint)reg, val) */


#define PXP_PMA_BASE                          (0x1fc70000)




//PMA_DIG_CODA
#define AN7583_PXP_PMA_LCPLL_PWCTL_SETTING_0		              PXP_PMA_BASE+0xE000
#define AN7583_PXP_PMA_LCPLL_PWCTL_SETTING_1		              PXP_PMA_BASE+0xE004
#define AN7583_PXP_PMA_LCPLL_PWCTL_SETTING_2		              PXP_PMA_BASE+0xE008
#define AN7583_PXP_PMA_LCPLL_PWCTL_DBG_SETTING		              PXP_PMA_BASE+0xE00C
#define AN7583_PXP_PMA_LCPLL_TDC_PW_0		                      PXP_PMA_BASE+0xE010
#define AN7583_PXP_PMA_LCPLL_TDC_PW_1		                      PXP_PMA_BASE+0xE014
#define AN7583_PXP_PMA_LCPLL_TDC_PW_2		                      PXP_PMA_BASE+0xE018
#define AN7583_PXP_PMA_LCPLL_TDC_PW_3		                      PXP_PMA_BASE+0xE01C
#define AN7583_PXP_PMA_LCPLL_TDC_PW_4		                      PXP_PMA_BASE+0xE020
#define AN7583_PXP_PMA_LCPLL_TDC_PW_5		                      PXP_PMA_BASE+0xE024
#define AN7583_PXP_PMA_LCPLL_TDC_FLT_0		                      PXP_PMA_BASE+0xE028
#define AN7583_PXP_PMA_LCPLL_TDC_FLT_1		                      PXP_PMA_BASE+0xE02C
#define AN7583_PXP_PMA_LCPLL_TDC_FLT_2		                      PXP_PMA_BASE+0xE030
#define AN7583_PXP_PMA_LCPLL_TDC_FLT_3		                      PXP_PMA_BASE+0xE034
#define AN7583_PXP_PMA_LCPLL_TDC_FLT_4		                      PXP_PMA_BASE+0xE038
#define AN7583_PXP_PMA_LCPLL_TDC_FLT_5		                      PXP_PMA_BASE+0xE03C
#define AN7583_PXP_PMA_LCPLL_TDC_FLT_6		                      PXP_PMA_BASE+0xE040
#define AN7583_PXP_PMA_LCPLL_TDC_FLT_7		                      PXP_PMA_BASE+0xE044
#define AN7583_PXP_PMA_LCPLL_TDC_PCW_1		                      PXP_PMA_BASE+0xE048
#define AN7583_PXP_PMA_LCPLL_TDC_PCW_2		                      PXP_PMA_BASE+0xE04C
#define AN7583_PXP_PMA_LCPLL_TDC_RO_1		                      PXP_PMA_BASE+0xE050
#define AN7583_PXP_PMA_LCPLL_TDC_RO_2		                      PXP_PMA_BASE+0xE054
#define AN7583_PXP_PMA_LCPLL_TDC_RO_3		                      PXP_PMA_BASE+0xE058
#define AN7583_PXP_PMA_LCPLL_TDC_RO_4		                      PXP_PMA_BASE+0xE05C
#define AN7583_PXP_PMA_LCPLL_TDC_RO_5		                      PXP_PMA_BASE+0xE060
#define AN7583_PXP_PMA_LCPLL_TDC_CTRL_0		                      PXP_PMA_BASE+0xE064
#define AN7583_PXP_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0		          PXP_PMA_BASE+0xE068
#define AN7583_PXP_PMA_RX_EYE_TOP_EYEINDEX_CTRL_1		          PXP_PMA_BASE+0xE06C
#define AN7583_PXP_PMA_RX_EYE_TOP_EYEINDEX_CTRL_2		          PXP_PMA_BASE+0xE070
#define AN7583_PXP_PMA_RX_EYE_TOP_EYEINDEX_CTRL_3		          PXP_PMA_BASE+0xE074
#define AN7583_PXP_PMA_RX_EYE_TOP_EYEOPENING_CTRL_0		          PXP_PMA_BASE+0xE078
#define AN7583_PXP_PMA_RX_EYE_TOP_EYEOPENING_CTRL_1		          PXP_PMA_BASE+0xE07C
#define AN7583_PXP_PMA_RX_EYE_TOP_EYECNT_CTRL_0		              PXP_PMA_BASE+0xE080
#define AN7583_PXP_PMA_RX_EYE_TOP_EYECNT_CTRL_1		              PXP_PMA_BASE+0xE084
#define AN7583_PXP_PMA_RX_EYE_TOP_EYECNT_CTRL_2		              PXP_PMA_BASE+0xE088
#define AN7583_PXP_PMA_RX_CTRL_SEQUENCE_CTRL_0		              PXP_PMA_BASE+0xE08C
#define AN7583_PXP_PMA_RX_CTRL_SEQUENCE_CTRL_1		              PXP_PMA_BASE+0xE090
#define AN7583_PXP_PMA_RX_CTRL_SEQUENCE_CTRL_2		              PXP_PMA_BASE+0xE094
#define AN7583_PXP_PMA_RX_CTRL_SEQUENCE_CTRL_3		              PXP_PMA_BASE+0xE098
#define AN7583_PXP_PMA_RX_CTRL_SEQUENCE_CTRL_4		              PXP_PMA_BASE+0xE09C
#define AN7583_PXP_PMA_RX_CTRL_SEQUENCE_CTRL_5		              PXP_PMA_BASE+0xE100
#define AN7583_PXP_PMA_RX_CTRL_SEQUENCE_CTRL_6		              PXP_PMA_BASE+0xE104
#define AN7583_PXP_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0		          PXP_PMA_BASE+0xE108
#define AN7583_PXP_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1		          PXP_PMA_BASE+0xE10C
#define AN7583_PXP_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0		      PXP_PMA_BASE+0xE110
#define AN7583_PXP_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1		      PXP_PMA_BASE+0xE114
#define AN7583_PXP_PMA_PHY_EQ_CTRL_0		                      PXP_PMA_BASE+0xE118
#define AN7583_PXP_PMA_PHY_EQ_CTRL_1		                      PXP_PMA_BASE+0xE11C
#define AN7583_PXP_PMA_PHY_EQ_CTRL_2		                      PXP_PMA_BASE+0xE120
#define AN7583_PXP_PMA_PHY_EQ_CTRL_3		                      PXP_PMA_BASE+0xE124
#define AN7583_PXP_PMA_PHY_EQ_CTRL_4		                      PXP_PMA_BASE+0xE128
#define AN7583_PXP_PMA_PHY_EQ_CTRL_5		                      PXP_PMA_BASE+0xE12C
#define AN7583_PXP_PMA_PHY_EQ_CTRL_6		                      PXP_PMA_BASE+0xE130
#define AN7583_PXP_PMA_PHY_EQ_CTRL_7		                      PXP_PMA_BASE+0xE134
#define AN7583_PXP_PMA_PHY_EQ_CTRL_8		                      PXP_PMA_BASE+0xE138
#define AN7583_PXP_PMA_PHY_EQ_CTRL_9		                      PXP_PMA_BASE+0xE13C
#define AN7583_PXP_PMA_PHY_EQ_CTRL_10		                      PXP_PMA_BASE+0xE140
#define AN7583_PXP_PMA_RX_FEOS		                              PXP_PMA_BASE+0xE144
#define AN7583_PXP_PMA_RX_BLWC		                              PXP_PMA_BASE+0xE148
#define AN7583_PXP_PMA_RX_FREQ_DET_1		                      PXP_PMA_BASE+0xE14C
#define AN7583_PXP_PMA_RX_FREQ_DET_2		                      PXP_PMA_BASE+0xE150
#define AN7583_PXP_PMA_RX_FREQ_DET_3		                      PXP_PMA_BASE+0xE154
#define AN7583_PXP_PMA_RX_FREQ_DET_4		                      PXP_PMA_BASE+0xE158
#define AN7583_PXP_PMA_RX_PI_CAL		                          PXP_PMA_BASE+0xE15C
#define AN7583_PXP_PMA_RX_CAL_1		                              PXP_PMA_BASE+0xE160
#define AN7583_PXP_PMA_RX_CAL_2		                              PXP_PMA_BASE+0xE164
#define AN7583_PXP_PMA_RX_SIGDET_0		                          PXP_PMA_BASE+0xE168
#define AN7583_PXP_PMA_RX_SIGDET_1		                          PXP_PMA_BASE+0xE16C
#define AN7583_PXP_PMA_RX_FLL_0		                              PXP_PMA_BASE+0xE170
#define AN7583_PXP_PMA_RX_FLL_1		                              PXP_PMA_BASE+0xE174
#define AN7583_PXP_PMA_RX_FLL_2		                              PXP_PMA_BASE+0xE178
#define AN7583_PXP_PMA_RX_FLL_3		                              PXP_PMA_BASE+0xE17C
#define AN7583_PXP_PMA_RX_FLL_4		                              PXP_PMA_BASE+0xE180
#define AN7583_PXP_PMA_RX_FLL_5		                              PXP_PMA_BASE+0xE184
#define AN7583_PXP_PMA_RX_FLL_6		                              PXP_PMA_BASE+0xE188
#define AN7583_PXP_PMA_RX_FLL_7		                              PXP_PMA_BASE+0xE18C
#define AN7583_PXP_PMA_RX_FLL_8		                              PXP_PMA_BASE+0xE190
#define AN7583_PXP_PMA_RX_FLL_9		                              PXP_PMA_BASE+0xE194
#define AN7583_PXP_PMA_RX_FLL_a		                              PXP_PMA_BASE+0xE198
#define AN7583_PXP_PMA_RX_FLL_b		                              PXP_PMA_BASE+0xE19C
#define AN7583_PXP_PMA_RX_PDOS_CTRL_0		                      PXP_PMA_BASE+0xE200
#define AN7583_PXP_PMA_RX_RESET_0		                          PXP_PMA_BASE+0xE204
#define AN7583_PXP_PMA_RX_RESET_1		                          PXP_PMA_BASE+0xE208
#define AN7583_PXP_PMA_RX_DEBUG_0		                          PXP_PMA_BASE+0xE20C
#define AN7583_PXP_PMA_BISTCTL_CONTROL		                      PXP_PMA_BASE+0xE210
#define AN7583_PXP_PMA_BISTCTL_ALIGN_PAT		                  PXP_PMA_BASE+0xE214
#define AN7583_PXP_PMA_BISTCTL_PROGRAM_PAT_0		              PXP_PMA_BASE+0xE218
#define AN7583_PXP_PMA_BISTCTL_PROGRAM_PAT_1		              PXP_PMA_BASE+0xE21C
#define AN7583_PXP_PMA_BISTCTL_POLLUTION		                  PXP_PMA_BASE+0xE220
#define AN7583_PXP_PMA_BISTCTL_PRBS_INITIAL_SEED		          PXP_PMA_BASE+0xE224
#define AN7583_PXP_PMA_BISTCTL_PRBS_EVENT		                  PXP_PMA_BASE+0xE228
#define AN7583_PXP_PMA_BISTCTL_PRBS_ERRCNT		                  PXP_PMA_BASE+0xE22C
#define AN7583_PXP_PMA_BISTCTL_PRBS_FAIL_THRESHOLD		          PXP_PMA_BASE+0xE230
#define AN7583_PXP_PMA_RX_TORGS_DEBUG_0		                      PXP_PMA_BASE+0xE234
#define AN7583_PXP_PMA_RX_TORGS_DEBUG_1		                      PXP_PMA_BASE+0xE238
#define AN7583_PXP_PMA_RX_TORGS_DEBUG_2		                      PXP_PMA_BASE+0xE23C
#define AN7583_PXP_PMA_RX_TORGS_DEBUG_3		                      PXP_PMA_BASE+0xE240
#define AN7583_PXP_PMA_RX_TORGS_DEBUG_4		                      PXP_PMA_BASE+0xE244
#define AN7583_PXP_PMA_RX_TORGS_DEBUG_5		                      PXP_PMA_BASE+0xE248
#define AN7583_PXP_PMA_RX_TORGS_DEBUG_6		                      PXP_PMA_BASE+0xE24C
#define AN7583_PXP_PMA_RX_TORGS_DEBUG_7		                      PXP_PMA_BASE+0xE250
#define AN7583_PXP_PMA_RX_TORGS_DEBUG_8		                      PXP_PMA_BASE+0xE254
#define AN7583_PXP_PMA_RX_TORGS_DEBUG_9		                      PXP_PMA_BASE+0xE258
#define AN7583_PXP_PMA_RX_TORGS_DEBUG_10		                  PXP_PMA_BASE+0xE25C
#define AN7583_PXP_PMA_TX_RST_B		                              PXP_PMA_BASE+0xE260
#define AN7583_PXP_PMA_TX_CALIB_0		                          PXP_PMA_BASE+0xE264
#define AN7583_PXP_PMA_TX_CALIB_1		                          PXP_PMA_BASE+0xE268
#define AN7583_PXP_PMA_TX_CALIB_2		                          PXP_PMA_BASE+0xE26C
#define AN7583_PXP_PMA_XPON_SETTING_0		                      PXP_PMA_BASE+0xE270
#define AN7583_PXP_PMA_XPON_SETTING_1		                      PXP_PMA_BASE+0xE274
#define AN7583_PXP_PMA_XPON_SETTING_2		                      PXP_PMA_BASE+0xE278
#define AN7583_PXP_PMA_XPON_STA		                              PXP_PMA_BASE+0xE27C
#define AN7583_PXP_PMA_XPON_INT_EN_0		                      PXP_PMA_BASE+0xE280
#define AN7583_PXP_PMA_XPON_INT_EN_1		                      PXP_PMA_BASE+0xE284
#define AN7583_PXP_PMA_XPON_INT_STA_0		                      PXP_PMA_BASE+0xE288
#define AN7583_PXP_PMA_XPON_INT_STA_1		                      PXP_PMA_BASE+0xE28C
#define AN7583_PXP_PMA_RX_TORGS_DEBUG_11		                  PXP_PMA_BASE+0xE290
#define AN7583_PXP_PMA_RX_FORCE_MODE_0		                      PXP_PMA_BASE+0xE294
#define AN7583_PXP_PMA_RX_FORCE_MODE_1		                      PXP_PMA_BASE+0xE298
#define AN7583_PXP_PMA_RX_FORCE_MODE_2		                      PXP_PMA_BASE+0xE29C
#define AN7583_PXP_PMA_RX_DISB_MODE_0		                      PXP_PMA_BASE+0xE300
#define AN7583_PXP_PMA_RX_DISB_MODE_1		                      PXP_PMA_BASE+0xE304
#define AN7583_PXP_PMA_RX_DISB_MODE_2		                      PXP_PMA_BASE+0xE308
#define AN7583_PXP_PMA_RX_FORCE_MODE_3		                      PXP_PMA_BASE+0xE30C
#define AN7583_PXP_PMA_RX_FORCE_MODE_4		                      PXP_PMA_BASE+0xE310
#define AN7583_PXP_PMA_RX_FORCE_MODE_5		                      PXP_PMA_BASE+0xE314
#define AN7583_PXP_PMA_RX_FORCE_MODE_6		                      PXP_PMA_BASE+0xE318
#define AN7583_PXP_PMA_RX_DISB_MODE_3		                      PXP_PMA_BASE+0xE31C
#define AN7583_PXP_PMA_RX_DISB_MODE_4		                      PXP_PMA_BASE+0xE320
#define AN7583_PXP_PMA_RX_DISB_MODE_5		                      PXP_PMA_BASE+0xE324
#define AN7583_PXP_PMA_RX_FORCE_MODE_7		                      PXP_PMA_BASE+0xE328
#define AN7583_PXP_PMA_RX_FORCE_MODE_8		                      PXP_PMA_BASE+0xE32C
#define AN7583_PXP_PMA_RX_FORCE_MODE_9		                      PXP_PMA_BASE+0xE330
#define AN7583_PXP_PMA_RX_DISB_MODE_6		                      PXP_PMA_BASE+0xE334
#define AN7583_PXP_PMA_RX_DISB_MODE_7		                      PXP_PMA_BASE+0xE338
#define AN7583_PXP_PMA_RX_DISB_MODE_8		                      PXP_PMA_BASE+0xE33C
#define AN7583_PXP_PMA_BIST_0		                              PXP_PMA_BASE+0xE340
#define AN7583_PXP_PMA_BIST_1		                              PXP_PMA_BASE+0xE344
#define AN7583_PXP_PMA_BIST_2		                              PXP_PMA_BASE+0xE348
#define AN7583_PXP_PMA_DA_XPON_PWDB_0		                      PXP_PMA_BASE+0xE34C
#define AN7583_PXP_PMA_DA_XPON_PWDB_1		                      PXP_PMA_BASE+0xE350
#define AN7583_PXP_PMA_LCPLL_0		                              PXP_PMA_BASE+0xE354
#define AN7583_PXP_PMA_LCPLL_1		                              PXP_PMA_BASE+0xE358
#define AN7583_PXP_PMA_LCPLL_2		                              PXP_PMA_BASE+0xE35C
#define AN7583_PXP_PMA_ADD_DIG_RESERVE_0		                  PXP_PMA_BASE+0xE360
#define AN7583_PXP_PMA_ADD_DIG_RESERVE_1		                  PXP_PMA_BASE+0xE364
#define AN7583_PXP_PMA_ADD_DIG_RESERVE_2		                  PXP_PMA_BASE+0xE368
#define AN7583_PXP_PMA_ADD_DIG_RESERVE_3		                  PXP_PMA_BASE+0xE36C
#define AN7583_PXP_PMA_ADD_DIG_RESERVE_4		                  PXP_PMA_BASE+0xE370
#define AN7583_PXP_PMA_RG_XPON_RX_RESERVED_1		              PXP_PMA_BASE+0xE374
#define AN7583_PXP_PMA_ADD_DIG_RO_RESERVE_0		                  PXP_PMA_BASE+0xE378
#define AN7583_PXP_PMA_ADD_DIG_RO_RESERVE_1		                  PXP_PMA_BASE+0xE37C
#define AN7583_PXP_PMA_ADD_DIG_RO_RESERVE_2		                  PXP_PMA_BASE+0xE380
#define AN7583_PXP_PMA_ADD_DIG_RO_RESERVE_3		                  PXP_PMA_BASE+0xE384
#define AN7583_PXP_PMA_ADD_DIG_RO_RESERVE_4		                  PXP_PMA_BASE+0xE388
#define AN7583_PXP_PMA_ADD_RX_SYS_EN_SEL_0		                  PXP_PMA_BASE+0xE38C
#define AN7583_PXP_PMA_PLL_TDC_FREQDET_0		                  PXP_PMA_BASE+0xE390
#define AN7583_PXP_PMA_PLL_TDC_FREQDET_1		                  PXP_PMA_BASE+0xE394
#define AN7583_PXP_PMA_PLL_TDC_FREQDET_2		                  PXP_PMA_BASE+0xE398
#define AN7583_PXP_PMA_PLL_TDC_FREQDET_3		                  PXP_PMA_BASE+0xE39C
#define AN7583_PXP_PMA_DA_XPON_TX_FORCE_0		                  PXP_PMA_BASE+0xE400
#define AN7583_PXP_PMA_DA_XPON_TX_FORCE_1		                  PXP_PMA_BASE+0xE404
#define AN7583_PXP_PMA_DA_XPON_TX_FORCE_2		                  PXP_PMA_BASE+0xE408
#define AN7583_PXP_PMA_RX_FORCE_MODE_10		                      PXP_PMA_BASE+0xE40C
#define AN7583_PXP_PMA_ADD_CLKPATH_RST_0		                  PXP_PMA_BASE+0xE410
#define AN7583_PXP_PMA_ADD_XPON_MODE_1		                      PXP_PMA_BASE+0xE414
#define AN7583_PXP_PMA_ADD_R2T_MODE_1		                      PXP_PMA_BASE+0xE418
#define AN7583_PXP_PMA_ADD_T2R_MODE_1		                      PXP_PMA_BASE+0xE41C
#define AN7583_PXP_PMA_ADD_LCPLL_RO_1		                      PXP_PMA_BASE+0xE420
#define AN7583_PXP_PMA_ADD_RO_RX2ANA_1		                      PXP_PMA_BASE+0xE424
#define AN7583_PXP_PMA_ADD_RO_RX2ANA_2		                      PXP_PMA_BASE+0xE428
#define AN7583_PXP_PMA_ADD_RO_RX2ANA_3		                      PXP_PMA_BASE+0xE42C
#define AN7583_PXP_PMA_ADD_RO_R2TMODE_1		                      PXP_PMA_BASE+0xE430
#define AN7583_PXP_PMA_RG_LCPLL_XPON_CMN_STB_CTRL_0		          PXP_PMA_BASE+0xE434
#define AN7583_PXP_PMA_RG_LCPLL_XPON_CMN_STB_CTRL_1		          PXP_PMA_BASE+0xE438
#define AN7583_PXP_PMA_RG_TX_HSDATA_EN_EXT_CNT_0		          PXP_PMA_BASE+0xE43C
#define AN7583_PXP_PMA_PON_TX_COUNTER_0		                      PXP_PMA_BASE+0xE440
#define AN7583_PXP_PMA_PON_TX_COUNTER_1		                      PXP_PMA_BASE+0xE444
#define AN7583_PXP_PMA_PON_TX_COUNTER_2		                      PXP_PMA_BASE+0xE448
#define AN7583_PXP_PMA_PON_TX_COUNTER_3		                      PXP_PMA_BASE+0xE44C
#define AN7583_PXP_PMA_PON_CK_SET		                          PXP_PMA_BASE+0xE450
#define AN7583_PXP_PMA_TX_FIFO_MODE_SEL		                      PXP_PMA_BASE+0xE454
#define AN7583_PXP_PMA_XPON_PLL_STB_CNT		                      PXP_PMA_BASE+0xE458
#define AN7583_PXP_PMA_XPON_PLL_STOP_CNT		                  PXP_PMA_BASE+0xE45C
#define AN7583_PXP_PMA_SW_RST_SET		                          PXP_PMA_BASE+0xE460
#define AN7583_PXP_PMA_ADD_RO_TX2ANA_1		                      PXP_PMA_BASE+0xE464
#define AN7583_PXP_PMA_TX_DLY_CTRL		                          PXP_PMA_BASE+0xE468
#define AN7583_PXP_PMA_MEM_WRAPPER_CTRL		                      PXP_PMA_BASE+0xE46C
#define AN7583_PXP_PMA_XPON_INT_EN_2		                      PXP_PMA_BASE+0xE470
#define AN7583_PXP_PMA_XPON_INT_EN_3		                      PXP_PMA_BASE+0xE474
#define AN7583_PXP_PMA_XPON_INT_STA_2		                      PXP_PMA_BASE+0xE478
#define AN7583_PXP_PMA_XPON_INT_STA_3		                      PXP_PMA_BASE+0xE47C
#define AN7583_PXP_PMA_XPON_SD_BEHAVIOR_SETTING_0		          PXP_PMA_BASE+0xE480
#define AN7583_PXP_PMA_XPON_SD_BEHAVIOR_SETTING_1		          PXP_PMA_BASE+0xE484
#define AN7583_PXP_PMA_XPON_SD_BEHAVIOR_STA		                  PXP_PMA_BASE+0xE488
#define AN7583_PXP_PMA_RX_EXTRAL_CTRL		                      PXP_PMA_BASE+0xE48C
#define AN7583_PXP_PMA_RX_LEQ_DISB_CTRL0		                  PXP_PMA_BASE+0xE490
#define AN7583_PXP_PMA_RX_LEQ_DISB_CTRL1		                  PXP_PMA_BASE+0xE494
#define AN7583_PXP_PMA_RX_LEQ_FORCE_CTRL0		                  PXP_PMA_BASE+0xE498
#define AN7583_PXP_PMA_RX_LEQ_FORCE_CTRL1		                  PXP_PMA_BASE+0xE49C
#define AN7583_PXP_PMA_RX_LEQ_ENTIME_CTRL0		                  PXP_PMA_BASE+0xE500
#define AN7583_PXP_PMA_RX_LEQ_ENTIME_CTRL1		                  PXP_PMA_BASE+0xE504
#define AN7583_PXP_PMA_RX_LEQ_ENTIME_CTRL2		                  PXP_PMA_BASE+0xE508
#define AN7583_PXP_PMA_RX_LEQ_CTRL0		                          PXP_PMA_BASE+0xE50C
#define AN7583_PXP_PMA_RX_LEQ_CTRL1		                          PXP_PMA_BASE+0xE510
#define AN7583_PXP_PMA_RX_LEQ_CTRL2		                          PXP_PMA_BASE+0xE514
#define AN7583_PXP_PMA_RX_LEQ_RO0		                          PXP_PMA_BASE+0xE518
#define AN7583_PXP_PMA_RX_LEQ_RO1		                          PXP_PMA_BASE+0xE51C
#define AN7583_PXP_PMA_PLL_FT_FREQDET_0		                      PXP_PMA_BASE+0xE520
#define AN7583_PXP_PMA_PLL_FT_FREQDET_1		                      PXP_PMA_BASE+0xE524
#define AN7583_PXP_PMA_PLL_FT_FREQDET_2		                      PXP_PMA_BASE+0xE528
#define AN7583_PXP_PMA_PLL_FT_FREQDET_3		                      PXP_PMA_BASE+0xE52C
#define AN7583_PXP_PMA_RO_RX_FREQDET		                      PXP_PMA_BASE+0xE530
#define AN7583_PXP_PMA_RO_PLL_FREQDET		                      PXP_PMA_BASE+0xE534
#define AN7583_PXP_PMA_RO_PLL_FT_FREQDET                          PXP_PMA_BASE+0xE538
#define AN7583_PXP_PMA_RO_PMA_FREQDET		                      PXP_PMA_BASE+0xE53C
#define AN7583_PXP_PMA_RG_PMA_FREQDET		                      PXP_PMA_BASE+0xE540
#define AN7583_PXP_PMA_RG_EXT_BEN_DATA		                      PXP_PMA_BASE+0xE544
#define AN7583_PXP_PMA_RG_PRE_BEN_DATA		                      PXP_PMA_BASE+0xE548
#define AN7583_PXP_PMA_RX_TORGS_DEBUG_12		                  PXP_PMA_BASE+0xE54C
#define AN7583_PXP_PMA_RO_FLL_ADC_0		                          PXP_PMA_BASE+0xE550
#define AN7583_PXP_PMA_RO_FLL_ADC_1		                          PXP_PMA_BASE+0xE554
#define AN7583_PXP_PMA_RO_FLL_ADC_2		                          PXP_PMA_BASE+0xE558
#define AN7583_PXP_PMA_RO_FLL_ADC_3		                          PXP_PMA_BASE+0xE55C
#define AN7583_PXP_PMA_RO_FLL_ADC_4		                          PXP_PMA_BASE+0xE560
#define AN7583_PXP_PMA_RG_AD_XPON_PLL_FT_CK_MON_MUX_SEL		      PXP_PMA_BASE+0xE564
#define AN7583_PXP_PMA_XPON_INT_EN_4		                      PXP_PMA_BASE+0xE568
#define AN7583_PXP_PMA_XPON_INT_STA_4		                      PXP_PMA_BASE+0xE56C
#define AN7583_PXP_PMA_XPON_PMA_AFIFO_REACH_TH		              PXP_PMA_BASE+0xE570
#define AN7583_PXP_PMA_XPON_MD32_PBUS_CSR_CTRL		              PXP_PMA_BASE+0xE574
#define AN7583_PXP_PMA_BENOFF_BIST_SEED		                      PXP_PMA_BASE+0xE578
#define AN7583_PXP_PMA_PWR_PLL_CTRL		                          PXP_PMA_BASE+0xE57C
#define AN7583_PXP_PMA_xpon_tx_rate_ctrl		                  PXP_PMA_BASE+0xE580
#define AN7583_PXP_PMA_BENOFF_DATA0		                          PXP_PMA_BASE+0xE584
#define AN7583_PXP_PMA_BENOFF_DATA1		                          PXP_PMA_BASE+0xE588
#define AN7583_PXP_PMA_BENOFF_DATA2		                          PXP_PMA_BASE+0xE58C
#define AN7583_PXP_PMA_BENOFF_DATA3		                          PXP_PMA_BASE+0xE590
#define AN7583_PXP_PMA_BENOFF_CTRL		                          PXP_PMA_BASE+0xE594
#define AN7583_PXP_PMA_HS_DATA_EN_SEL		                      PXP_PMA_BASE+0xE598
#define AN7583_PXP_PMA_FIFO_CK_STATUS		                      PXP_PMA_BASE+0xE59C
#define AN7583_PXP_PMA_TDC_DLF_GAIN_STATUS		                  PXP_PMA_BASE+0xE600
#define AN7583_PXP_PMA_TDC_DLF_MODE_SETTING		                  PXP_PMA_BASE+0xE604
#define AN7583_PXP_PMA_MD32_ISR_CTRL		                      PXP_PMA_BASE+0xE608
#define AN7583_PXP_PMA_MD32_MEM_CLK_CTRL		                  PXP_PMA_BASE+0xE60C
#define AN7583_PXP_PMA_MD32_MEM_CLK_CG_CTRL		                  PXP_PMA_BASE+0xE610
#define AN7583_PXP_PMA_XG_EQD_STA		                          PXP_PMA_BASE+0xE614
#define AN7583_PXP_PMA_XG_EQD_CTRL		                          PXP_PMA_BASE+0xE618
#define AN7583_PXP_PMA_RAWDATA_DUMP_CTRL_0		                  PXP_PMA_BASE+0xE61C
#define AN7583_PXP_PMA_RAWDATA_DUMP_CTRL_1		                  PXP_PMA_BASE+0xE620
#define AN7583_PXP_PMA_RAWDATA_DUMP_CTRL_2		                  PXP_PMA_BASE+0xE624
#define AN7583_PXP_PMA_RAWDATA_DUMP_CTRL_3		                  PXP_PMA_BASE+0xE628
#define AN7583_PXP_PMA_RAWDATA_DUMP_STA_0		                  PXP_PMA_BASE+0xE62C
#define AN7583_PXP_PMA_RAWDATA_DUMP_STA_1		                  PXP_PMA_BASE+0xE630
#define AN7583_PXP_PMA_RAWDATA_DUMP_STA_2		                  PXP_PMA_BASE+0xE634
#define AN7583_PXP_PMA_MD32MEM_DIV2_CNT		                      PXP_PMA_BASE+0xE638
#define AN7583_PXP_PMA_AD_XPON_CDR_LPF_SV		                  PXP_PMA_BASE+0xE63C
#define AN7583_PXP_PMA_K_VALUE_RD		                          PXP_PMA_BASE+0xE640
#define AN7583_PXP_PMA_XPONPLL_CTRL		                          PXP_PMA_BASE+0xE644
#define AN7583_PXP_PMA_MD32_GATE_HALT	                          PXP_PMA_BASE+0xE648
#define AN7583_PXP_PMA_MD32_MONCCNT		                          PXP_PMA_BASE+0xE64C
#define AN7583_PXP_PMA_MD32_MONPCNT0		                      PXP_PMA_BASE+0xE650
#define AN7583_PXP_PMA_MD32_MONPCNT1		                      PXP_PMA_BASE+0xE654
#define AN7583_PXP_PMA_MD32_MONPCNT2		                      PXP_PMA_BASE+0xE658
#define AN7583_PXP_PMA_MD32_MONCONTID		                      PXP_PMA_BASE+0xE65C
#define AN7583_PXP_PMA_MD32_MONPC		                          PXP_PMA_BASE+0xE660
#define AN7583_PXP_PMA_MD32_TBUF		                          PXP_PMA_BASE+0xE664
#define AN7583_PXP_PMA_MD32_TBUFWDATA_L		                      PXP_PMA_BASE+0xE668
#define AN7583_PXP_PMA_MD32_TBUFWDATA_H		                      PXP_PMA_BASE+0xE66C
#define AN7583_PXP_PMA_MD32_TBUFRDATA_L		                      PXP_PMA_BASE+0xE670
#define AN7583_PXP_PMA_MD32_TBUFRDATA_H		                      PXP_PMA_BASE+0xE674
#define AN7583_PXP_PMA_ADDR_MD32_PMEM_ADDR_REV		              PXP_PMA_BASE+0xE678
#define AN7583_PXP_PMA_ADDR_MD32_PMEM_DATA_REV		              PXP_PMA_BASE+0xE67C
#define AN7583_PXP_PMA_ADDR_MD32_DMEM_ADDR_REV		              PXP_PMA_BASE+0xE680
#define AN7583_PXP_PMA_ADDR_MD32_DMEM_DATA_REV		              PXP_PMA_BASE+0xE684
#define AN7583_PXP_PMA_JCPLL_PWCTL_SETTING_0		              PXP_PMA_BASE+0xE688
#define AN7583_PXP_PMA_JCPLL_PWCTL_SETTING_1		              PXP_PMA_BASE+0xE68C
#define AN7583_PXP_PMA_JCPLL_PWCTL_SETTING_2		              PXP_PMA_BASE+0xE690
#define AN7583_PXP_PMA_JCPLL_CHG		                          PXP_PMA_BASE+0xE694
#define AN7583_PXP_PMA_RG_JCPLL_SDM_PCW_CTRL		              PXP_PMA_BASE+0xE698
#define AN7583_PXP_PMA_RG_JCPLL_SDM_PCW_CHG_CTRL		          PXP_PMA_BASE+0xE69C
#define AN7583_PXP_PMA_PLL_JCPLL_FT_FREQDET_0		              PXP_PMA_BASE+0xE700
#define AN7583_PXP_PMA_PLL_JCPLL_FT_FREQDET_1		              PXP_PMA_BASE+0xE704
#define AN7583_PXP_PMA_PLL_JCPLL_FT_FREQDET_2		              PXP_PMA_BASE+0xE708
#define AN7583_PXP_PMA_PLL_JCPLL_FT_FREQDET_3		              PXP_PMA_BASE+0xE70C
#define AN7583_PXP_PMA_JCPLL_FT_CK_MUX		                      PXP_PMA_BASE+0xE710
#define AN7583_PXP_PMA_JCPLL_DA_RG_CTRL_0		                  PXP_PMA_BASE+0xE714
#define AN7583_PXP_PMA_JCPLL_DA_RG_CTRL_1		                  PXP_PMA_BASE+0xE718
#define AN7583_PXP_PMA_JCPLL_DA_RG_CTRL_2		                  PXP_PMA_BASE+0xE71C
#define AN7583_PXP_PMA_XFI_PLL_DA_RG_CTRL_0		                  PXP_PMA_BASE+0xE720
#define AN7583_PXP_PMA_XFI_PLL_DA_RG_CTRL_1		                  PXP_PMA_BASE+0xE724
#define AN7583_PXP_PMA_XFI_PLL_DA_RG_CTRL_2		                  PXP_PMA_BASE+0xE728
#define AN7583_PXP_PMA_AD_DA_PROBE_STS_0		                  PXP_PMA_BASE+0xE72C
#define AN7583_PXP_PMA_AD_DA_PROBE_STS_1		                  PXP_PMA_BASE+0xE730
#define AN7583_PXP_PMA_JCPLL_SCAN_CTRL		                      PXP_PMA_BASE+0xE734
#define AN7583_PXP_PMA_XFI_GPIO_SEL_0		                      PXP_PMA_BASE+0xE738
#define AN7583_PXP_PMA_TX_PAR_PROBE_PLL_CTRL	                  PXP_PMA_BASE+0xE73C
#define AN7583_PXP_PMA_DUMP_RX_PCSRST_CTRL		                  PXP_PMA_BASE+0xE740
#define AN7583_PXP_PMA_XFI_TX_FIFO_STS		                      PXP_PMA_BASE+0xE744
#define AN7583_PXP_PMA_JCPLL_CK_RSTB_CTRL		                  PXP_PMA_BASE+0xE748
#define AN7583_PXP_PMA_RO_TDC_TX_FREQDET		                  PXP_PMA_BASE+0xE74C
#define AN7583_PXP_PMA_RO_JCPLL_FT_FREQDET		                  PXP_PMA_BASE+0xE750
#define AN7583_PXP_PMA_RO_JCPLL_500M_FREQDET		              PXP_PMA_BASE+0xE754
#define AN7583_PXP_PMA_RG_JCPLL_500M_FREQ_DET_1		              PXP_PMA_BASE+0xE758
#define AN7583_PXP_PMA_RG_JCPLL_500M_FREQ_DET_2		              PXP_PMA_BASE+0xE75C
#define AN7583_PXP_PMA_RG_JCPLL_500M_FREQ_DET_3		              PXP_PMA_BASE+0xE760
#define AN7583_PXP_PMA_RG_JCPLL_500M_FREQ_DET_4		              PXP_PMA_BASE+0xE764
#define AN7583_PXP_PMA_rg_da_pxp_jcpll_sdm_scan		              PXP_PMA_BASE+0xE768
#define AN7583_PXP_PMA_rg_force_da_pxp_aeq_speed	              PXP_PMA_BASE+0xE76C
#define AN7583_PXP_PMA_rg_force_da_pxp_aeq_mode		              PXP_PMA_BASE+0xE770
#define AN7583_PXP_PMA_rg_force_da_pxp_tx_data_ben		          PXP_PMA_BASE+0xE774
#define AN7583_PXP_PMA_rg_force_da_pxp_tx_fir_c0b		          PXP_PMA_BASE+0xE778
#define AN7583_PXP_PMA_rg_force_da_pxp_tx_term_sel		          PXP_PMA_BASE+0xE77C
#define AN7583_PXP_PMA_rg_force_da_pxp_tx_fir_c1		          PXP_PMA_BASE+0xE780
#define AN7583_PXP_PMA_rg_force_da_pxp_tx_rate_ctrl		          PXP_PMA_BASE+0xE784
#define AN7583_PXP_PMA_rg_force_da_pxp_rx_dac_d0		          PXP_PMA_BASE+0xE788
#define AN7583_PXP_PMA_rg_force_da_pxp_rx_dac_e0		          PXP_PMA_BASE+0xE78C
#define AN7583_PXP_PMA_rg_force_da_pxp_cdr_pr_fll_cor	          PXP_PMA_BASE+0xE790
#define AN7583_PXP_PMA_rg_force_da_pxp_cdr_pr_idac		          PXP_PMA_BASE+0xE794
#define AN7583_PXP_PMA_rg_force_da_pxp_txpll_sdm_pcw	          PXP_PMA_BASE+0xE798
#define AN7583_PXP_PMA_rg_force_da_pxp_rx_fe_vos		          PXP_PMA_BASE+0xE79C
#define AN7583_PXP_PMA_rg_force_da_pxp_jcpll_sdm_pcw	          PXP_PMA_BASE+0xE800
#define AN7583_PXP_PMA_rg_force_da_pcie_cktx0_en		          PXP_PMA_BASE+0xE804
#define AN7583_PXP_PMA_rg_force_da_pxp_aeq_en		              PXP_PMA_BASE+0xE808
#define AN7583_PXP_PMA_rg_force_da_pxp_aeq_bypass		          PXP_PMA_BASE+0xE80C
#define AN7583_PXP_PMA_rg_force_da_pxp_aeq_inprgrss		          PXP_PMA_BASE+0xE810
#define AN7583_PXP_PMA_rg_force_da_pxp_aeq_rstb		              PXP_PMA_BASE+0xE814
#define AN7583_PXP_PMA_rg_force_da_pxp_cdr_lpf_lck2data	          PXP_PMA_BASE+0xE818
#define AN7583_PXP_PMA_rg_force_da_pxp_cdr_pd_pwdb		          PXP_PMA_BASE+0xE81C
#define AN7583_PXP_PMA_rg_force_da_pxp_cdr_pr_lpf_c_en		      PXP_PMA_BASE+0xE820
#define AN7583_PXP_PMA_rg_force_da_pxp_cdr_pr_pieye_pwdb		  PXP_PMA_BASE+0xE824
#define AN7583_PXP_PMA_rg_force_da_pxp_jcpll_ckout_en		      PXP_PMA_BASE+0xE828
#define AN7583_PXP_PMA_rg_force_da_pxp_jcpll_kband_scan_en		  PXP_PMA_BASE+0xE82C
#define AN7583_PXP_PMA_rg_force_da_pxp_jcpll_kband_scan_in		  PXP_PMA_BASE+0xE830
#define AN7583_PXP_PMA_rg_force_da_pxp_jcpll_sdm_pcw_chg		  PXP_PMA_BASE+0xE834
#define AN7583_PXP_PMA_rg_force_da_pxp_jcpll_sdm_scan_en_cg		  PXP_PMA_BASE+0xE838
#define AN7583_PXP_PMA_rg_force_da_pxp_jcpll_sdm_scan_rstb		  PXP_PMA_BASE+0xE83C
#define AN7583_PXP_PMA_rg_force_da_pxp_rx_oscal_en		          PXP_PMA_BASE+0xE840
#define AN7583_PXP_PMA_rg_force_da_pxp_rx_scan_enable		      PXP_PMA_BASE+0xE844
#define AN7583_PXP_PMA_rg_force_da_pxp_rx_scan_in		          PXP_PMA_BASE+0xE848
#define AN7583_PXP_PMA_rg_force_da_pxp_rx_scan_rst_b		      PXP_PMA_BASE+0xE84C
#define AN7583_PXP_PMA_rg_force_da_pxp_tdc_cal_bw		          PXP_PMA_BASE+0xE850
#define AN7583_PXP_PMA_rg_force_da_pxp_txpll_ckout_en		      PXP_PMA_BASE+0xE854
#define AN7583_PXP_PMA_rg_force_da_pxp_txpll_kband_load_en		  PXP_PMA_BASE+0xE858
#define AN7583_PXP_PMA_rg_force_da_pxp_txpll_kband_scan_en_cg	  PXP_PMA_BASE+0xE85C
#define AN7583_PXP_PMA_rg_da_pxp_txpll_kband_scan		          PXP_PMA_BASE+0xE860
#define AN7583_PXP_PMA_rg_force_da_pxp_txpll_sdm_pcw_chg		  PXP_PMA_BASE+0xE864
#define AN7583_PXP_PMA_rg_force_da_pxp_txpll_sdm_scan_en_cg		  PXP_PMA_BASE+0xE868
#define AN7583_PXP_PMA_rg_da_pxp_txpll_sdm_scan		              PXP_PMA_BASE+0xE86C
#define AN7583_PXP_PMA_rg_force_da_pxp_tx_acjtag_dn		          PXP_PMA_BASE+0xE870
#define AN7583_PXP_PMA_rg_force_da_pxp_tx_acjtag_en		          PXP_PMA_BASE+0xE874
#define AN7583_PXP_PMA_rg_force_da_pxp_tx_ck_en		              PXP_PMA_BASE+0xE878
#define AN7583_PXP_PMA_rg_force_da_pxp_tx_hsdata_en		          PXP_PMA_BASE+0xE87C
#define AN7583_PXP_PMA_rg_force_da_pxp_tx_rxdet_en		          PXP_PMA_BASE+0xE880
#define AN7583_PXP_PMA_scan_mode		                          PXP_PMA_BASE+0xE884
#define AN7583_PXP_PMA_rg_da_pxp_jcpll_kband_scan		          PXP_PMA_BASE+0xE888
#define AN7583_PXP_PMA_rg_force_da_pxp_rx_fe_gain_ctrl		      PXP_PMA_BASE+0xE88C
#define AN7583_PXP_PMA_rg_force_da_pxp_aeq_rmtxskip		          PXP_PMA_BASE+0xE890
#define AN7583_PXP_PMA_rg_force_da_pxp_rx_fe_pwdb		          PXP_PMA_BASE+0xE894
#define AN7583_PXP_PMA_rg_force_da_pxp_rx_sigdet_cal_en		      PXP_PMA_BASE+0xE898
#define AN7583_PXP_PMA_ADD_DIG_RESERVE_5		                  PXP_PMA_BASE+0xE89C
#define AN7583_PXP_PMA_ADD_DIG_RESERVE_6		                  PXP_PMA_BASE+0xE8A0
#define AN7583_PXP_PMA_ADD_DIG_RESERVE_7		                  PXP_PMA_BASE+0xE8A4
#define AN7583_PXP_PMA_ADD_DIG_RESERVE_8		                  PXP_PMA_BASE+0xE8A8
#define AN7583_PXP_PMA_ADD_DIG_RESERVE_9		                  PXP_PMA_BASE+0xE8AC
#define AN7583_PXP_PMA_ADD_DIG_RESERVE_10		                  PXP_PMA_BASE+0xE8B0
#define AN7583_PXP_PMA_ADD_DIG_RESERVE_11		                  PXP_PMA_BASE+0xE8B4
#define AN7583_PXP_PMA_ADD_DIG_RESERVE_12		                  PXP_PMA_BASE+0xE8B8
#define AN7583_PXP_PMA_ADD_DIG_RESERVE_13		                  PXP_PMA_BASE+0xE8BC
#define AN7583_PXP_PMA_ADD_DIG_RESERVE_14		                  PXP_PMA_BASE+0xE8C0
#define AN7583_PXP_PMA_ADD_DIG_RO_RESERVE_5		                  PXP_PMA_BASE+0xE8C4
#define AN7583_PXP_PMA_ADD_DIG_RO_RESERVE_6		                  PXP_PMA_BASE+0xE8C8
#define AN7583_PXP_PMA_ADD_DIG_RO_RESERVE_7		                  PXP_PMA_BASE+0xE8CC
#define AN7583_PXP_PMA_ADD_DIG_RO_RESERVE_8		                  PXP_PMA_BASE+0xE8D0
#define AN7583_PXP_PMA_ADD_DIG_RO_RESERVE_9		                  PXP_PMA_BASE+0xE8D4
#define AN7583_PXP_PMA_ADD_DIG_RESERVE_15		                  PXP_PMA_BASE+0xE8D8
#define AN7583_PXP_PMA_ADD_DIG_RESERVE_16		                  PXP_PMA_BASE+0xE8DC
#define AN7583_PXP_PMA_ADD_DIG_RESERVE_17		                  PXP_PMA_BASE+0xE8E0
#define AN7583_PXP_PMA_ADD_DIG_RESERVE_18		                  PXP_PMA_BASE+0xE8E4
#define AN7583_PXP_PMA_ADD_DIG_RESERVE_19		                  PXP_PMA_BASE+0xE8E8
#define AN7583_PXP_PMA_ADD_DIG_RESERVE_20		                  PXP_PMA_BASE+0xE8EC
#define AN7583_PXP_PMA_ADD_DIG_RESERVE_21		                  PXP_PMA_BASE+0xE8F0
#define AN7583_PXP_PMA_ADD_DIG_RESERVE_22		                  PXP_PMA_BASE+0xE8F4
#define AN7583_PXP_PMA_ADD_DIG_RESERVE_23		                  PXP_PMA_BASE+0xE8F8
#define AN7583_PXP_PMA_ADD_DIG_RESERVE_24		                  PXP_PMA_BASE+0xE8FC
#define AN7583_PXP_PMA_ADD_DIG_RESERVE_25		                  PXP_PMA_BASE+0xE900
#define AN7583_PXP_PMA_ADD_DIG_RESERVE_26		                  PXP_PMA_BASE+0xE904
#define AN7583_PXP_PMA_ADD_DIG_RESERVE_27		                  PXP_PMA_BASE+0xE908
#define AN7583_PXP_PMA_ADD_DIG_RESERVE_28		                  PXP_PMA_BASE+0xE90C
#define AN7583_PXP_PMA_ADD_DIG_RESERVE_29		                  PXP_PMA_BASE+0xE910
#define AN7583_PXP_PMA_ADD_DIG_RESERVE_30		                  PXP_PMA_BASE+0xE914
#define AN7583_PXP_PMA_ADD_DIG_RESERVE_31		                  PXP_PMA_BASE+0xE918
#define AN7583_PXP_PMA_ADD_DIG_RESERVE_32		                  PXP_PMA_BASE+0xE91C
#define AN7583_PXP_PMA_ADD_DIG_RESERVE_33		                  PXP_PMA_BASE+0xE920
#define AN7583_PXP_PMA_ADD_DIG_RESERVE_34		                  PXP_PMA_BASE+0xE924
#define AN7583_PXP_PMA_ADD_DIG_RESERVE_35		                  PXP_PMA_BASE+0xE928
#define AN7583_PXP_PMA_ADD_DIG_RESERVE_36		                  PXP_PMA_BASE+0xE92C
#define AN7583_PXP_PMA_ADD_DIG_RESERVE_37		                  PXP_PMA_BASE+0xE930
#define AN7583_PXP_PMA_ADD_DIG_RESERVE_38		                  PXP_PMA_BASE+0xE934
#define AN7583_PXP_PMA_ADD_DIG_RESERVE_39		                  PXP_PMA_BASE+0xE938
#define AN7583_PXP_PMA_ADD_DIG_RESERVE_40		                  PXP_PMA_BASE+0xE93C
#define AN7583_PXP_PMA_ADD_DIG_RESERVE_41		                  PXP_PMA_BASE+0xE940
#define AN7583_PXP_PMA_ADD_DIG_RESERVE_42		                  PXP_PMA_BASE+0xE944
#define AN7583_PXP_PMA_ADD_DIG_RESERVE_43		                  PXP_PMA_BASE+0xE948
#define AN7583_PXP_PMA_ADD_DIG_RESERVE_44		                  PXP_PMA_BASE+0xE94C
#define AN7583_PXP_PMA_ADD_DIG_RESERVE_45		                  PXP_PMA_BASE+0xE950
#define AN7583_PXP_PMA_ADD_DIG_RESERVE_46		                  PXP_PMA_BASE+0xE954
#define AN7583_PXP_PMA_ADD_DIG_RESERVE_47		                  PXP_PMA_BASE+0xE958
#define AN7583_PXP_PMA_ADD_DIG_RESERVE_48		                  PXP_PMA_BASE+0xE95C
#define AN7583_PXP_PMA_BEN_CTRL_0		                          PXP_PMA_BASE+0xE960
#define AN7583_PXP_PMA_BEN_CTRL_1		                          PXP_PMA_BASE+0xE964
#define AN7583_PXP_PMA_BEN_STS_0		                          PXP_PMA_BASE+0xE968
#define AN7583_PXP_PMA_BEN_STS_1		                          PXP_PMA_BASE+0xE96C
#define AN7583_PXP_PMA_BEN_STS_2		                          PXP_PMA_BASE+0xE970
#define AN7583_PXP_PMA_BEN_STS_3		                          PXP_PMA_BASE+0xE974
#define AN7583_PXP_PMA_TX_SD_CTRL_0		                          PXP_PMA_BASE+0xE978
#define AN7583_PXP_PMA_TX_SD_CTRL_1		                          PXP_PMA_BASE+0xE97C
#define AN7583_PXP_PMA_TX_SD_STS_0		                          PXP_PMA_BASE+0xE980
#define AN7583_PXP_PMA_TX_SD_STS_1		                          PXP_PMA_BASE+0xE984
#define AN7583_PXP_PMA_TX_SD_STS_2		                          PXP_PMA_BASE+0xE988
#define AN7583_PXP_PMA_TX_SD_STS_3		                          PXP_PMA_BASE+0xE98C



//AN7583_ANA_CODA
#define  AN7583_PXP_ANA_RG_PXP_CMN_EN                             PXP_PMA_BASE+0xF000
#define  AN7583_PXP_ANA_RG_PXP_JCPLL_IB_EXT_EN                    PXP_PMA_BASE+0xF004
#define  AN7583_PXP_ANA_RG_PXP_JCPLL_LPF_BR                       PXP_PMA_BASE+0xF008
#define  AN7583_PXP_ANA_RG_PXP_JCPLL_LPF_BWC                      PXP_PMA_BASE+0xF00C
#define  AN7583_PXP_ANA_RG_PXP_JCPLL_KBAND_KFC                    PXP_PMA_BASE+0xF010
#define  AN7583_PXP_ANA_RG_PXP_JCPLL_MMD_PREDIV_MODE              PXP_PMA_BASE+0xF014
#define  AN7583_PXP_ANA_RG_PXP_JCPLL_MONCK_EN                     PXP_PMA_BASE+0xF018
#define  AN7583_PXP_ANA_RG_PXP_JCPLL_RST_DLY                      PXP_PMA_BASE+0xF01C
#define  AN7583_PXP_ANA_RG_PXP_JCPLL_SDM_IFM                      PXP_PMA_BASE+0xF020
#define  AN7583_PXP_ANA_RG_PXP_JCPLL_SDM_HREN                     PXP_PMA_BASE+0xF024
#define  AN7583_PXP_ANA_RG_PXP_JCPLL_TCL_CMP_EN                   PXP_PMA_BASE+0xF028
#define  AN7583_PXP_ANA_RG_PXP_JCPLL_VCODIV                       PXP_PMA_BASE+0xF02C
#define  AN7583_PXP_ANA_RG_PXP_JCPLL_VCO_TCLVAR                   PXP_PMA_BASE+0xF030
#define  AN7583_PXP_ANA_RG_PXP_JCPLL_SSC_TRI_EN                   PXP_PMA_BASE+0xF034
#define  AN7583_PXP_ANA_RG_PXP_JCPLL_SSC_DELTA                    PXP_PMA_BASE+0xF038
#define  AN7583_PXP_ANA_RG_PXP_JCPLL_LDO_OUT                      PXP_PMA_BASE+0xF03C
#define  AN7583_PXP_ANA_RG_PXP_JCPLL_LDO_VCO_VTP_EN               PXP_PMA_BASE+0xF040
#define  AN7583_PXP_ANA_RG_PXP_JCPLL_LDO_VTP_EN                   PXP_PMA_BASE+0xF044
#define  AN7583_PXP_ANA_RG_PXP_JCPLL_SPARE_H                      PXP_PMA_BASE+0xF048
#define  AN7583_PXP_ANA_RG_PXP_JCPLL_FREQ_MEAS_EN                 PXP_PMA_BASE+0xF04C
#define  AN7583_PXP_ANA_RG_PXP_TXPLL_CHP_IBIAS                    PXP_PMA_BASE+0xF050
#define  AN7583_PXP_ANA_RG_PXP_TXPLL_LPF_BP                       PXP_PMA_BASE+0xF054
#define  AN7583_PXP_ANA_RG_PXP_TXPLL_KBAND_CODE                   PXP_PMA_BASE+0xF058
#define  AN7583_PXP_ANA_RG_PXP_TXPLL_KBAND_KS                     PXP_PMA_BASE+0xF05C
#define  AN7583_PXP_ANA_RG_PXP_TXPLL_PHY_CK1_EN                   PXP_PMA_BASE+0xF060
#define  AN7583_PXP_ANA_RG_PXP_TXPLL_REFIN_INTERNAL               PXP_PMA_BASE+0xF064
#define  AN7583_PXP_ANA_RG_PXP_TXPLL_SDM_DI_EN                    PXP_PMA_BASE+0xF068
#define  AN7583_PXP_ANA_RG_PXP_TXPLL_SDM_ORD                      PXP_PMA_BASE+0xF06C
#define  AN7583_PXP_ANA_RG_PXP_TXPLL_TCL_AMP_GAIN                 PXP_PMA_BASE+0xF070
#define  AN7583_PXP_ANA_RG_PXP_TXPLL_TCL_LPF_EN                   PXP_PMA_BASE+0xF074
#define  AN7583_PXP_ANA_RG_PXP_TXPLL_VCO_HALFLSB_EN               PXP_PMA_BASE+0xF078
#define  AN7583_PXP_ANA_RG_PXP_TXPLL_SSC_EN                       PXP_PMA_BASE+0xF07C
#define  AN7583_PXP_ANA_RG_PXP_TXPLL_SSC_DELTA1                   PXP_PMA_BASE+0xF080
#define  AN7583_PXP_ANA_RG_PXP_TXPLL_SSC_PERIOD                   PXP_PMA_BASE+0xF084
#define  AN7583_PXP_ANA_RG_PXP_TXPLL_VTP_EN                       PXP_PMA_BASE+0xF088
#define  AN7583_PXP_ANA_RG_PXP_TXPLL_VCO_VTP_EN                   PXP_PMA_BASE+0xF08C
#define  AN7583_PXP_ANA_RG_PXP_TXPLL_TCL_VTP_EN                   PXP_PMA_BASE+0xF090
#define  AN7583_PXP_ANA_RG_PXP_TXPLL_TCL_KBAND_VREF               PXP_PMA_BASE+0xF094
#define  AN7583_PXP_ANA_RG_PCIE_CLKTX0_AMP                        PXP_PMA_BASE+0xF098
#define  AN7583_PXP_ANA_RG_PCIE_CLKTX1_AMP                        PXP_PMA_BASE+0xF09C
#define  AN7583_PXP_ANA_RG_PXP_PLL_MONCLK_SEL                     PXP_PMA_BASE+0xF0A0
#define  AN7583_PXP_ANA_RG_PXP_TDC_TXCK_SEL                       PXP_PMA_BASE+0xF0A4
#define  AN7583_PXP_ANA_RG_PXP_TDC_SYNC_CK_SEL                    PXP_PMA_BASE+0xF0A8
#define  AN7583_PXP_ANA_RGS_PXP_JCPLL_KBAND_CODE                  PXP_PMA_BASE+0xF0AC
#define  AN7583_PXP_ANA_RGS_PXP_TXPLL_KBAND_CODE                  PXP_PMA_BASE+0xF0B0
#define  AN7583_PXP_ANA_RG_PXP_TX_SER_LOADSEL                     PXP_PMA_BASE+0xF0B4
#define  AN7583_PXP_ANA_RG_PXP_TX_CKMON_SEL                       PXP_PMA_BASE+0xF0B8
#define  AN7583_PXP_ANA_RG_PXP_TX_FIR_CHGPLR_C1                   PXP_PMA_BASE+0xF0BC
#define  AN7583_PXP_ANA_RG_PXP_TX_TXLBRX_EN                       PXP_PMA_BASE+0xF0C0
#define  AN7583_PXP_ANA_RG_PXP_TX_CKLDO_EN                        PXP_PMA_BASE+0xF0C4
#define  AN7583_PXP_ANA_RG_PXP_TX_MULTLANE_EN                     PXP_PMA_BASE+0xF0C8
#define  AN7583_PXP_ANA_RG_PXP_RX_BUSBIT_SEL                      PXP_PMA_BASE+0xF0CC
#define  AN7583_PXP_ANA_RG_PXP_RX_MPXSEL                          PXP_PMA_BASE+0xF0D0
#define  AN7583_PXP_ANA_RG_PXP_RX_REV_0                           PXP_PMA_BASE+0xF0D4
#define  AN7583_PXP_ANA_RG_PXP_RX_PHYCK_DIV                       PXP_PMA_BASE+0xF0D8
#define  AN7583_PXP_ANA_RG_PXP_CDR_PD_PICAL_CKD8_INV              PXP_PMA_BASE+0xF0DC
#define  AN7583_PXP_ANA_RG_PXP_CDR_LPF_BOT_LIM                    PXP_PMA_BASE+0xF0E0
#define  AN7583_PXP_ANA_RG_PXP_CDR_LPF_KP_GAIN                    PXP_PMA_BASE+0xF0E4
#define  AN7583_PXP_ANA_RG_PXP_CDR_LPF_RATIO                      PXP_PMA_BASE+0xF0E8
#define  AN7583_PXP_ANA_RG_PXP_CDR_LPF_SNAPSHOT                   PXP_PMA_BASE+0xF0EC
#define  AN7583_PXP_ANA_RG_PXP_CDR_LPF_SETVALUE                   PXP_PMA_BASE+0xF0F0
#define  AN7583_PXP_ANA_RG_PXP_CDR_PR_INJ_MODE                    PXP_PMA_BASE+0xF0F4
#define  AN7583_PXP_ANA_RG_PXP_CDR_PR_BETA_DAC                    PXP_PMA_BASE+0xF0F8
#define  AN7583_PXP_ANA_RG_PXP_CDR_PR_VREG_IBAND_VAL              PXP_PMA_BASE+0xF0FC
#define  AN7583_PXP_ANA_RG_PXP_CDR_PR_CKREF_DIV                   PXP_PMA_BASE+0xF100
#define  AN7583_PXP_ANA_RG_PXP_CDR_PR_MONCK_EN                    PXP_PMA_BASE+0xF104
#define  AN7583_PXP_ANA_RG_PXP_CDR_PR_TDC_REF_SEL                 PXP_PMA_BASE+0xF108
#define  AN7583_PXP_ANA_RG_PXP_CDR_PR_MONPR_EN                    PXP_PMA_BASE+0xF10C
#define  AN7583_PXP_ANA_RG_PXP_RX_DAC_RANGE                       PXP_PMA_BASE+0xF110
#define  AN7583_PXP_ANA_RG_PXP_RX_SIGDET_NOVTH                    PXP_PMA_BASE+0xF114
#define  AN7583_PXP_ANA_RG_PXP_RX_FE_EQ_HZEN                      PXP_PMA_BASE+0xF118
#define  AN7583_PXP_ANA_RG_PXP_RX_FE_VCM_GEN_PWDB                 PXP_PMA_BASE+0xF11C
#define  AN7583_PXP_ANA_RG_PXP_RX_OSCAL_WATCH_WNDW                PXP_PMA_BASE+0xF120
#define  AN7583_PXP_ANA_RG_PXP_RX_OSCAL_OPTION                    PXP_PMA_BASE+0xF124
#define  AN7583_PXP_ANA_RG_PXP_RX_OSCAL_CTLE2IOS                  PXP_PMA_BASE+0xF128
#define  AN7583_PXP_ANA_RG_PXP_RX_OSCAL_VGA2IOS                   PXP_PMA_BASE+0xF12C
#define  AN7583_PXP_ANA_RG_PXP_AEQ_WATCH_WNDW                     PXP_PMA_BASE+0xF130
#define  AN7583_PXP_ANA_RG_PXP_AEQ_FLAG_TXFIR_REQ_RDY             PXP_PMA_BASE+0xF134
#define  AN7583_PXP_ANA_RG_PXP_AEQ_OPTION                         PXP_PMA_BASE+0xF138
#define  AN7583_PXP_ANA_RG_PXP_AEQ_CFORCE                         PXP_PMA_BASE+0xF13C
#define  AN7583_PXP_ANA_RG_PXP_AEQ_CTLE_MAX                       PXP_PMA_BASE+0xF140
#define  AN7583_PXP_ANA_RG_PXP_RX_FE_PEAKING_CTRL_MSB             PXP_PMA_BASE+0xF144
#define  AN7583_PXP_ANA_RG_PXP_RX_DAC_D1_BYPASS_AEQ               PXP_PMA_BASE+0xF148
#define  AN7583_PXP_ANA_RG_PXP_RX_DAC_ERR0_BYPASS_AEQ             PXP_PMA_BASE+0xF14C
#define  AN7583_PXP_ANA_RG_PXP_AEQ_CM1_WNDW                       PXP_PMA_BASE+0xF150
#define  AN7583_PXP_ANA_RG_PXP_AEQ_DFETP2_WNDW                    PXP_PMA_BASE+0xF154
#define  AN7583_PXP_ANA_RG_PXP_AEQ_DFETP6_WNDW                    PXP_PMA_BASE+0xF158
#define  AN7583_PXP_ANA_RGS_PXP_RX_OSCAL_FE_VOS                   PXP_PMA_BASE+0xF15C
#define  AN7583_PXP_ANA_RGS_PXP_RX_OSCAL_CTLE2IOS                 PXP_PMA_BASE+0xF160
#define  AN7583_PXP_ANA_RGS_PXP_RX_OSCAL_VGA2IOS                  PXP_PMA_BASE+0xF164
#define  AN7583_PXP_ANA_RGS_PXP_AEQ_CTLE                          PXP_PMA_BASE+0xF168
#define  AN7583_PXP_ANA_RGS_PXP_AEQ_SAOSC_EN                      PXP_PMA_BASE+0xF16C
#define  AN7583_PXP_ANA_RGS_PXP_AEQ_E1_OS                         PXP_PMA_BASE+0xF170




//#define PXP_Eye_Scan_Result  0
#define LAB_PRINT_PXP 0
#define LAB_PXP_Test  0
#define PXP_PR_WK 0
#define PXP_AEQ 0





//define_Bit Rate
#define PXP_USXGMII                    0xB    //DS(RX)_10.31252G  /  US(TX)_10.3125G
#define PXP_HSGMII                     0x11   //DS(RX)_3.125G      /  US(TX)_3.125G





//define_Pcie_TXPLL
#define PCIE_10p3125G_BYP_JCPLL		       0x1     //TXPLL_10p3125G
#define PCIE_12p5G_BYP_JCPLL		       0x2     //TXPLL_12p5G

//define_Pcie_TX_Spd
#define TX_Spd_USXGMII                     0x1    //DS(RX)_10.31252G  /  US(TX)_10.3125G
#define TX_Spd_HSGMII                      0x2    //DS(RX)_9.95328G   /  US(TX)_2.48832G

//define_Pcie_RX_Spd
#define RX_Spd_USXGMII                     0x1    //DS(RX)_10.31252G  /  US(TX)_10.3125G
#define RX_Spd_HSGMII                      0x2    //DS(RX)_9.95328G   /  US(TX)_2.48832G

#define PXP_FIRST_PLUG_IN     1
#define PXP_PLUG_IN           2
#define PXP_PLUG_OUT          3	

#define fm_USXGMII            0       //fm_DS_10.3125G  /  US_10.3125G
#define fm_HSGMII             1        //fm_DS_3.125G     /  US_3.125G


#define PXP_bist_data             1
#define PXP_r2t_data              2
#define PXP_nor_pma_data          3




#define EO_PXP_USXGMII                    0xB     // DS(RX)_10.31252G  /  US(TX)_10.3125G








/*


//R2T
#define  Eth_Ser_bist_data         1
#define  Eth_Ser_r2t_data          2
#define  Eth_Ser_nor_pma_data      3




#define  EO_Async_GPON     0    //DS(RX)_2.48832G  /  US(TX)_1.24416G
		 
#define  EO_Sync_EPON_1    1    //DS_1.25G       /  US_1.25G
#define  EO_Sync_EPON_2    8    //DS_1.25G       /  US_1.25G
		 
#define  EO_Async_XEPON    6    //DS_10.3125G   /  US_1.25G
		 
#define  EO_Sync_XEPON     7    //DS_10.3125G   /  US_10.3125G
		 
#define  EO_Async_XGPON_1  9    //DS_9.95328G   /  US_2.48832G
#define  EO_Async_XGPON_2  12   //DS_9.95328G   /  US_2.48832G
		 
#define  EO_Sync_XGSPON_1  10   //DS_9.95328G   /  US_9.95328G
#define  EO_Sync_XGSPON_2  11   //DS_9.95328G   /  US_9.95328G
		 
#define  EO_Sync_GPON      99   //DS_2.48832G   /  US_2.48832G




*/


/*
struct {
     uint PON_RX_Align_t		Align_Pat;
	 //PON_TX_Pat			TX_Pat   ;	

} AN7583_XPON_PMA_Param_T ;
*/


typedef enum {
	 Align_PRBS7  = 0x6081fd53,
	 Align_PRBS9  = 0xd97b0d53,
	 Align_PRBS11 = 0xa1903d53,
	 Align_PRBS15 = 0xe1fd7d53,
	 Align_PRBS23 = 0x5cf1fd53,
	 Align_PRBS31 = 0x8FF1FD53
	
}PON_RX_Align;

/*
typedef enum{
	PRBS7   = 0x1, 
	PRBS9   = 0x2,	
	PRBS11  = 0x13,	
	PRBS15  = 0x3,
	PRBS23  = 0x4,
    PRBS31  = 0x5,
    HFTP    = 0x6,
    MFTP    = 0x7,
    SWP_4   = 0x8,  //Square_wave_pattern (n = 4)
    SWP_5   = 0x9,  //Square_wave_pattern (n = 5) LFTP
    SWP_6   = 0xA,  //Square_wave_pattern (n = 6) 
    SWP_7   = 0xB,  //Square_wave_pattern (n = 7) 
    SWP_8   = 0xC,  //Square_wave_pattern (n = 8) LFTP
    SWP_9   = 0xD,  //Square_wave_pattern (n = 9) 
    SWP_10  = 0xE,  //Square_wave_pattern (n = 10)
    SWP_11  = 0xF,  //Square_wave_pattern (n = 11)
    UDP     = 0x10, //User define Pattern
    All_1   = 0x11, //All_1
    All_0   = 0x12  //All_0
    
}PON_TX_Pat;

*/






//void XPON_DIG_reset_hold(uint spd);
//void XPON_DIG_ref_release(void);
//void XPON_DIG_reset_release(uint spd);
void pcie0_init (int bit_sel);
void pcie_LinkControl (void);
void pcie_CfgPhyType (int pon_Spd);
void pcie_WanSelInit (uint pon_Spd);
void PCIE0_TXPLL(uint rate_sel);
void PCIE0_TX(uint tx_bit_sel);
void PCIE0_RX(uint rx_bit_sel);
void PCIE0_ANA(uint rate_sel);
void PCIE0_TXPLL_on(void);
void PCIE0_TX_on(void);
void PCIE0_RX_preset(void);
void PCIE0_TDC_off(void);
void PCIE0_RX_on(void);
void PCIE0_RX_L2R(void);
void PCIE0_RX_OSCal(void);
void PCIE0_RX_pical(void);
void PCIE0_RX_pdos(void);
void PCIE0_RX_feos(void);
void PCIE0_RX_sdcal(void);
void PCIE0_phy_status(void);
void PCIE0_DIG_reset(void);
void PCIE0_RX_L2D(void);
void PCIE0_RX_rxrdy(void);
void PCIE0_BIST_setting(void);
void PCIE0_R2T_sel(uint r2t_sel);

void PCIE0_BIST_on(void);
void PCIE0_R2T_on(void);

void PCIE0_Version (void);


void pxp_DIG_reset_hold(void);
void pxp_DIG_ref_release(void);
void pxp_DIG_reset_release(void);

void Read_AEQ_Result(void);

void pcie_phy_eyescan_test(int start_p, int sweep_r);
void PCIE_eye_setting(void);
void PCIE_eye_Cal(void);
int pcie_eyescan_countPoint(uint EYE_X_FW, uint EYE_Y_FW );
uint pcie_eyescan_moveX(uint EYE_X_HW, uint EYE_Y_HW);

void pcie_Ext_T2R (int pon_Spd);
void pcie_Ext_R2T (int pon_Spd);
void PCIE_eye_EO(volatile int *fom_num , volatile int *veo, uint PXP_Eye_Scan);
void PCIE_readout_EO(int sweep_direction , volatile int *R_fom_num , volatile int *R_veo, uint PXP_Eye_Scan);
void pcie_EO_Scan(uint bit_sel, uint EO_start, uint EO_end, uint PXP_Eye_Scan);


#endif /* _PCIE0_PMA_H_ */




