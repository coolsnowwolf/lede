//##################################################################
//
// COPYRIGHT 2010, TRENDCHIP TECHNOLOGIES CORP.
// ALL RIGHTS RESERVED.
//
// File_ modulename_test.c
//   modulename firmware simulation file
//   
// DESCRIPTION_
// 1. Module test function, used to set register values and control
//    test flow.
// 
// AUTHOR _ xx 
// DATE   _ yy/mm/dd
// Ver. 20200825
//##################################################################


#ifndef PHY_EFUSE_DEF_H
#define PHY_EFUSE_DEF_H

//==============================================
//7583 efuse
//efuse_LSB_MSB
#define	SerDes_PCIe_efuse_valid_LSB                                 31 
#define	SerDes_PCIe_efuse_valid_MSB                                 33 
#define	SerDes_USB_efuse_valid_LSB                                  34 
#define	SerDes_USB_efuse_valid_MSB                                  36 
#define	PON_XFI_efuse_valid_LSB                                     37 
#define	PON_XFI_efuse_valid_MSB                                     37 
#define	GPHY_efuse_valid_LSB                                        38 
#define	GPHY_efuse_valid_MSB                                        38 
#define	AVS_thermal_sensor_efuse_valid_LSB                          39 
#define	AVS_thermal_sensor_efuse_valid_MSB                          39 
#define	Reserved_1_LSB                                              40 
#define	Reserved_1_MSB                                              46 
#define	DA_QP_SSUSB_TX_TERM_SEL_LSB /*[1:0]*/                       47 
#define	DA_QP_SSUSB_TX_TERM_SEL_MSB /*[1:0]*/                       48 
#define	DA_QP_SSUSB_RX_IMP_SEL_LSB /*[4:0]*/                        49 
#define	DA_QP_SSUSB_RX_IMP_SEL_MSB /*[4:0]*/                        53 
#define	Reserved_2_LSB                                              54 
#define	Reserved_2_MSB                                              59 
#define	Reserved_3_LSB                                              60 
#define	Reserved_3_MSB                                              66 
#define	DA_QP_PCIE_TX_TERM_SEL_LSB /*[1:0]*/                        67 
#define	DA_QP_PCIE_TX_TERM_SEL_MSB /*[1:0]*/                        68 
#define	DA_QP_PCIE_RX_IMP_SEL_LSB /*[4:0]*/                         69 
#define	DA_QP_PCIE_RX_IMP_SEL_MSB /*[4:0]*/                         73 
#define	Reserved_4_LSB                                              74 
#define	Reserved_4_MSB                                              79 
#define	Reserved_5_LSB                                              80 
#define	Reserved_5_MSB                                              84 
#define	Reserved_6_LSB                                              85 
#define	Reserved_6_MSB                                              88 
#define	RG_USB20_P0_INTR_CTRL_LSB /*[4:0]*/                         89 
#define	RG_USB20_P0_INTR_CTRL_MSB /*[4:0]*/                         93 
#define	RG_USB20_P1_INTR_CTRL_LSB /*[4:0]*/                         94 
#define	RG_USB20_P1_INTR_CTRL_MSB /*[4:0]*/                         98 
#define	Reserved_7_LSB                                              99 
#define	Reserved_7_MSB                                              102
#define	RG_PXP_PCIE_CMN_TRIM_LSB /*[4:0]*/                          103
#define	RG_PXP_PCIE_CMN_TRIM_MSB /*[4:0]*/                          107
#define	RG_PXP_PCIE_TX0_TERM_SEL_LSB /*[1:0]*/                      108
#define	RG_PXP_PCIE_TX0_TERM_SEL_MSB /*[1:0]*/                      109
#define	RG_PXP_PCIE_RX0_FE_50OHMS_SEL_LSB /*[1:0]*/                 110
#define	RG_PXP_PCIE_RX0_FE_50OHMS_SEL_MSB /*[1:0]*/                 111
#define	RG_PXP_PCIE_TX1_TERM_SEL_LSB /*[1:0]*/                      112
#define	RG_PXP_PCIE_TX1_TERM_SEL_MSB /*[1:0]*/                      113
#define	RG_PXP_PCIE_RX1_FE_50OHMS_SEL_LSB /*[1:0]*/                 114
#define	RG_PXP_PCIE_RX1_FE_50OHMS_SEL_MSB /*[1:0]*/                 115
#define	Reserved_8_LSB                                              116
#define	Reserved_8_MSB                                              119
#define	RG_PXP_XPON_CMN_TRIM_LSB /*[4:0]*/                          120
#define	RG_PXP_XPON_CMN_TRIM_MSB /*[4:0]*/                          124
#define	RG_PXP_XPON_TX_TERM_SEL_LSB /*[1:0]*/                       125
#define	RG_PXP_XPON_TX_TERM_SEL_MSB /*[1:0]*/                       126
#define	RG_PXP_XPON_RX_FE_50OHMS_SEL_LSB /*[1:0]*/                  127
#define	RG_PXP_XPON_RX_FE_50OHMS_SEL_MSB /*[1:0]*/                  128
#define	Reserved_9_LSB                                             	129
#define	Reserved_9_MSB                                             	132
#define	RG_PXP_XFI_CMN_TRIM_LSB /*[4:0]*/                           133
#define	RG_PXP_XFI_CMN_TRIM_MSB /*[4:0]*/                           137
#define	RG_PXP_XFI_TX_TERM_SEL_LSB /*[1:0]*/                        138
#define	RG_PXP_XFI_TX_TERM_SEL_MSB /*[1:0]*/                        139
#define	RG_PXP_XFI_RX_FE_50OHMS_SEL_LSB /*[1:0]*/                   140
#define	RG_PXP_XFI_RX_FE_50OHMS_SEL_MSB /*[1:0]*/                   141
#define	Reserved_10_LSB                                             142
#define	Reserved_10_MSB                                             145
#define	Reserved_for_GBE_P0_RG_COMP_CTL_P0_LSB /*[2:0]*/            146
#define	Reserved_for_GBE_P0_RG_COMP_CTL_P0_MSB /*[2:0]*/            148
#define	DA_TX_I2MPB_A_gbe_P0_LSB /*[5:0]*/                          149
#define	DA_TX_I2MPB_A_gbe_P0_MSB /*[5:0]*/                          154
#define	DA_TX_I2MPB_B_gbe_P0_LSB /*[5:0]*/                          155
#define	DA_TX_I2MPB_B_gbe_P0_MSB /*[5:0]*/                          160
#define	DA_TX_I2MPB_C_gbe_P0_LSB /*[5:0]*/                          161
#define	DA_TX_I2MPB_C_gbe_P0_MSB /*[5:0]*/                          166
#define	DA_TX_I2MPB_D_gbe_P0_LSB /*[5:0]*/                          167
#define	DA_TX_I2MPB_D_gbe_P0_MSB /*[5:0]*/                          172
#define	DA_TX_I2MPB_A_tst_P0_LSB /*[5:0]*/                          173
#define	DA_TX_I2MPB_A_tst_P0_MSB /*[5:0]*/                          178
#define	DA_TX_I2MPB_B_tst_P0_LSB /*[5:0]*/                          179
#define	DA_TX_I2MPB_B_tst_P0_MSB /*[5:0]*/                          184
#define	DA_TX_I2MPB_C_tst_P0_LSB /*[5:0]*/                          185
#define	DA_TX_I2MPB_C_tst_P0_MSB /*[5:0]*/                          190
#define	DA_TX_I2MPB_D_tst_P0_LSB /*[5:0]*/                          191
#define	DA_TX_I2MPB_D_tst_P0_MSB /*[5:0]*/                          196
#define	DA_TX_I2MPB_A_hbt_P0_LSB /*[5:0]*/                          197
#define	DA_TX_I2MPB_A_hbt_P0_MSB /*[5:0]*/                          202
#define	DA_TX_I2MPB_B_hbt_P0_LSB /*[5:0]*/                          203
#define	DA_TX_I2MPB_B_hbt_P0_MSB /*[5:0]*/                          208
#define	DA_TX_I2MPB_A_tbt_P0_LSB /*[5:0]*/                          209
#define	DA_TX_I2MPB_A_tbt_P0_MSB /*[5:0]*/                          214
#define	DA_TX_I2MPB_B_tbt_P0_LSB /*[5:0]*/                          215
#define	DA_TX_I2MPB_B_tbt_P0_MSB /*[5:0]*/                          220
#define	DA_TX_AMP_OFFSET_A_P0_LSB /*[5:0]*/                         221
#define	DA_TX_AMP_OFFSET_A_P0_MSB /*[5:0]*/                         226
#define	DA_TX_AMP_OFFSET_B_P0_LSB /*[5:0]*/                         227
#define	DA_TX_AMP_OFFSET_B_P0_MSB /*[5:0]*/                         232
#define	DA_TX_AMP_OFFSET_C_P0_LSB /*[5:0]*/                         233
#define	DA_TX_AMP_OFFSET_C_P0_MSB /*[5:0]*/                         238
#define	DA_TX_AMP_OFFSET_D_P0_LSB /*[5:0]*/                         239
#define	DA_TX_AMP_OFFSET_D_P0_MSB /*[5:0]*/                         244
#define	DA_R50OHM_RSEL_TX_A_P0_LSB /*[6:0]*/                        245
#define	DA_R50OHM_RSEL_TX_A_P0_MSB /*[6:0]*/                        251
#define	DA_R50OHM_RSEL_TX_B_P0_LSB /*[6:0]*/                        252
#define	DA_R50OHM_RSEL_TX_B_P0_MSB /*[6:0]*/                        258
#define	DA_R50OHM_RSEL_TX_C_P0_LSB /*[6:0]*/                        259
#define	DA_R50OHM_RSEL_TX_C_P0_MSB /*[6:0]*/                        265
#define	DA_R50OHM_RSEL_TX_D_P0_LSB /*[6:0]*/                        266
#define	DA_R50OHM_RSEL_TX_D_P0_MSB /*[6:0]*/                        272
#define	Reserved_for_GBE_P1_RG_COMP_CTL_P1_LSB /*[2:0]*/           	273
#define	Reserved_for_GBE_P1_RG_COMP_CTL_P1_MSB /*[2:0]*/           	275
#define	DA_TX_I2MPB_A_gbe_P1_LSB /*[5:0]*/                          276
#define	DA_TX_I2MPB_A_gbe_P1_MSB /*[5:0]*/                          281
#define	DA_TX_I2MPB_B_gbe_P1_LSB /*[5:0]*/                          282
#define	DA_TX_I2MPB_B_gbe_P1_MSB /*[5:0]*/                          287
#define	DA_TX_I2MPB_C_gbe_P1_LSB /*[5:0]*/                          288
#define	DA_TX_I2MPB_C_gbe_P1_MSB /*[5:0]*/                          293
#define	DA_TX_I2MPB_D_gbe_P1_LSB /*[5:0]*/                          294
#define	DA_TX_I2MPB_D_gbe_P1_MSB /*[5:0]*/                          299
#define	DA_TX_I2MPB_A_tst_P1_LSB /*[5:0]*/                          300
#define	DA_TX_I2MPB_A_tst_P1_MSB /*[5:0]*/                          305
#define	DA_TX_I2MPB_B_tst_P1_LSB /*[5:0]*/                          306
#define	DA_TX_I2MPB_B_tst_P1_MSB /*[5:0]*/                          311
#define	DA_TX_I2MPB_C_tst_P1_LSB /*[5:0]*/                          312
#define	DA_TX_I2MPB_C_tst_P1_MSB /*[5:0]*/                          317
#define	DA_TX_I2MPB_D_tst_P1_LSB /*[5:0]*/                          318
#define	DA_TX_I2MPB_D_tst_P1_MSB /*[5:0]*/                          323
#define	DA_TX_I2MPB_A_hbt_P1_LSB /*[5:0]*/                          324
#define	DA_TX_I2MPB_A_hbt_P1_MSB /*[5:0]*/                          329
#define	DA_TX_I2MPB_B_hbt_P1_LSB /*[5:0]*/                          330
#define	DA_TX_I2MPB_B_hbt_P1_MSB /*[5:0]*/                          335
#define	DA_TX_I2MPB_A_tbt_P1_LSB /*[5:0]*/                          336
#define	DA_TX_I2MPB_A_tbt_P1_MSB /*[5:0]*/                          341
#define	DA_TX_I2MPB_B_tbt_P1_LSB /*[5:0]*/                          342
#define	DA_TX_I2MPB_B_tbt_P1_MSB /*[5:0]*/                          347
#define	DA_TX_AMP_OFFSET_A_P1_LSB /*[5:0]*/                         348
#define	DA_TX_AMP_OFFSET_A_P1_MSB /*[5:0]*/                         353
#define	DA_TX_AMP_OFFSET_B_P1_LSB /*[5:0]*/                         354
#define	DA_TX_AMP_OFFSET_B_P1_MSB /*[5:0]*/                         359
#define	DA_TX_AMP_OFFSET_C_P1_LSB /*[5:0]*/                         360
#define	DA_TX_AMP_OFFSET_C_P1_MSB /*[5:0]*/                         365
#define	DA_TX_AMP_OFFSET_D_P1_LSB /*[5:0]*/                         366
#define	DA_TX_AMP_OFFSET_D_P1_MSB /*[5:0]*/                         371
#define	DA_R50OHM_RSEL_TX_A_P1_LSB /*[6:0]*/                        372
#define	DA_R50OHM_RSEL_TX_A_P1_MSB /*[6:0]*/                        378
#define	DA_R50OHM_RSEL_TX_B_P1_LSB /*[6:0]*/                        379
#define	DA_R50OHM_RSEL_TX_B_P1_MSB /*[6:0]*/                        385
#define	DA_R50OHM_RSEL_TX_C_P1_LSB /*[6:0]*/                        386
#define	DA_R50OHM_RSEL_TX_C_P1_MSB /*[6:0]*/                        392
#define	DA_R50OHM_RSEL_TX_D_P1_LSB /*[6:0]*/                        393
#define	DA_R50OHM_RSEL_TX_D_P1_MSB /*[6:0]*/                        399
#define	RG_BG_RASEL_LSB /*[2:0]*/                                   400
#define	RG_BG_RASEL_MSB /*[2:0]*/                                   402
#define	RG_REXT_TRIM_LSB /*[5:0]*/                                  403
#define	RG_REXT_TRIM_MSB /*[5:0]*/                                  408
#define	DA_TX_PS_DRIR0_hbt_tbt_LSB /*[3:0]*/                        409
#define	DA_TX_PS_DRIR0_hbt_tbt_MSB /*[3:0]*/                        412
#define	DA_TX_PS_DRIR0_B_hbt_tbt_LSB /*[3:0]*/                      413
#define	DA_TX_PS_DRIR0_B_hbt_tbt_MSB /*[3:0]*/                      416
#define	DA_TX_PS_OP_hbt_LSB /*[4:0]*/                               417
#define	DA_TX_PS_OP_hbt_MSB /*[4:0]*/                               421
#define	DA_TX_PS_OP_B_hbt_LSB /*[4:0]*/                             422
#define	DA_TX_PS_OP_B_hbt_MSB /*[4:0]*/                             426
#define	DA_TX_PS_OP_tbt_LSB /*[4:0]*/                               427
#define	DA_TX_PS_OP_tbt_MSB /*[4:0]*/                               431
#define	DA_TX_PS_OP_B_tbt_LSB /*[4:0]*/                             432
#define	DA_TX_PS_OP_B_tbt_MSB /*[4:0]*/                             436
#define	DA_TX_RM2P_OP_LSB /*[2:0]*/                                 437
#define	DA_TX_RM2P_OP_MSB /*[2:0]*/                                 439
#define	DA_TX_RM2P_OP_B_LSB /*[2:0]*/                               440
#define	DA_TX_RM2P_OP_B_MSB /*[2:0]*/                               442
#define	DA_TX_CM1_OP_LSB /*[3:0]*/                                  443
#define	DA_TX_CM1_OP_MSB /*[3:0]*/                                  446
#define	RG_HVGA_ECHO_GAIN_LSB /*[2:0]*/                             447
#define	RG_HVGA_ECHO_GAIN_MSB /*[2:0]*/                             449
#define	RG_HVGA_ECHO_RES_LSB /*[0]*/                                450
#define	RG_HVGA_ECHO_RES_MSB /*[0]*/                                450
#define	DA_ADBUF_BIAS_gbe_LSB /*[1:0]*/                             451
#define	DA_ADBUF_BIAS_gbe_MSB /*[1:0]*/                             452
#define	DA_ADBUF_BIAS_lp_LSB /*[1:0]*/                              453
#define	DA_ADBUF_BIAS_lp_MSB /*[1:0]*/                              454
#define	Reserved_for_GBE_P2_RG_COMP_CTL_P2_LSB /*[2:0]*/            455
#define	Reserved_for_GBE_P2_RG_COMP_CTL_P2_MSB /*[2:0]*/            457
#define	DA_TX_I2MPB_A_gbe_P2_LSB /*[5:0]*/                          458
#define	DA_TX_I2MPB_A_gbe_P2_MSB /*[5:0]*/                          463
#define	DA_TX_I2MPB_B_gbe_P2_LSB /*[5:0]*/                          464
#define	DA_TX_I2MPB_B_gbe_P2_MSB /*[5:0]*/                          469
#define	DA_TX_I2MPB_C_gbe_P2_LSB /*[5:0]*/                          470
#define	DA_TX_I2MPB_C_gbe_P2_MSB /*[5:0]*/                          475
#define	DA_TX_I2MPB_D_gbe_P2_LSB /*[5:0]*/                          476
#define	DA_TX_I2MPB_D_gbe_P2_MSB /*[5:0]*/                          481
#define	DA_TX_I2MPB_A_tst_P2_LSB /*[5:0]*/                          482
#define	DA_TX_I2MPB_A_tst_P2_MSB /*[5:0]*/                          487
#define	DA_TX_I2MPB_B_tst_P2_LSB /*[5:0]*/                          488
#define	DA_TX_I2MPB_B_tst_P2_MSB /*[5:0]*/                          493
#define	DA_TX_I2MPB_C_tst_P2_LSB /*[5:0]*/                          494
#define	DA_TX_I2MPB_C_tst_P2_MSB /*[5:0]*/                          499
#define	DA_TX_I2MPB_D_tst_P2_LSB /*[5:0]*/                          500
#define	DA_TX_I2MPB_D_tst_P2_MSB /*[5:0]*/                          505
#define	DA_TX_I2MPB_A_hbt_P2_LSB /*[5:0]*/                          506
#define	DA_TX_I2MPB_A_hbt_P2_MSB /*[5:0]*/                          511
#define	DA_TX_I2MPB_B_hbt_P2_LSB /*[5:0]*/                          512
#define	DA_TX_I2MPB_B_hbt_P2_MSB /*[5:0]*/                          517
#define	DA_TX_I2MPB_A_tbt_P2_LSB /*[5:0]*/                          518
#define	DA_TX_I2MPB_A_tbt_P2_MSB /*[5:0]*/                          523
#define	DA_TX_I2MPB_B_tbt_P2_LSB /*[5:0]*/                          524
#define	DA_TX_I2MPB_B_tbt_P2_MSB /*[5:0]*/                          529
#define	DA_TX_AMP_OFFSET_A_P2_LSB /*[5:0]*/                         530
#define	DA_TX_AMP_OFFSET_A_P2_MSB /*[5:0]*/                         535
#define	DA_TX_AMP_OFFSET_B_P2_LSB /*[5:0]*/                         536
#define	DA_TX_AMP_OFFSET_B_P2_MSB /*[5:0]*/                         541
#define	DA_TX_AMP_OFFSET_C_P2_LSB /*[5:0]*/                         542
#define	DA_TX_AMP_OFFSET_C_P2_MSB /*[5:0]*/                         547
#define	DA_TX_AMP_OFFSET_D_P2_LSB /*[5:0]*/                         548
#define	DA_TX_AMP_OFFSET_D_P2_MSB /*[5:0]*/                         553
#define	DA_R50OHM_RSEL_TX_A_P2_LSB /*[6:0]*/                        554
#define	DA_R50OHM_RSEL_TX_A_P2_MSB /*[6:0]*/                        560
#define	DA_R50OHM_RSEL_TX_B_P2_LSB /*[6:0]*/                        561
#define	DA_R50OHM_RSEL_TX_B_P2_MSB /*[6:0]*/                        567
#define	DA_R50OHM_RSEL_TX_C_P2_LSB /*[6:0]*/                        568
#define	DA_R50OHM_RSEL_TX_C_P2_MSB /*[6:0]*/                        574
#define	DA_R50OHM_RSEL_TX_D_P2_LSB /*[6:0]*/                        575
#define	DA_R50OHM_RSEL_TX_D_P2_MSB /*[6:0]*/                        581
#define	Reserved_for_GBE_P3_RG_COMP_CTL_P3_LSB /*[2:0]*/            582
#define	Reserved_for_GBE_P3_RG_COMP_CTL_P3_MSB /*[2:0]*/            584
#define	DA_TX_I2MPB_A_gbe_P3_LSB /*[5:0]*/                          585
#define	DA_TX_I2MPB_A_gbe_P3_MSB /*[5:0]*/                          590
#define	DA_TX_I2MPB_B_gbe_P3_LSB /*[5:0]*/                          591
#define	DA_TX_I2MPB_B_gbe_P3_MSB /*[5:0]*/                          596
#define	DA_TX_I2MPB_C_gbe_P3_LSB /*[5:0]*/                          597
#define	DA_TX_I2MPB_C_gbe_P3_MSB /*[5:0]*/                          602
#define	DA_TX_I2MPB_D_gbe_P3_LSB /*[5:0]*/                          603
#define	DA_TX_I2MPB_D_gbe_P3_MSB /*[5:0]*/                          608
#define	DA_TX_I2MPB_A_tst_P3_LSB /*[5:0]*/                          609
#define	DA_TX_I2MPB_A_tst_P3_MSB /*[5:0]*/                          614
#define	DA_TX_I2MPB_B_tst_P3_LSB /*[5:0]*/                          615
#define	DA_TX_I2MPB_B_tst_P3_MSB /*[5:0]*/                          620
#define	DA_TX_I2MPB_C_tst_P3_LSB /*[5:0]*/                          621
#define	DA_TX_I2MPB_C_tst_P3_MSB /*[5:0]*/                          626
#define	DA_TX_I2MPB_D_tst_P3_LSB /*[5:0]*/                          627
#define	DA_TX_I2MPB_D_tst_P3_MSB /*[5:0]*/                          632
#define	DA_TX_I2MPB_A_hbt_P3_LSB /*[5:0]*/                          633
#define	DA_TX_I2MPB_A_hbt_P3_MSB /*[5:0]*/                          638
#define	DA_TX_I2MPB_B_hbt_P3_LSB /*[5:0]*/                          639
#define	DA_TX_I2MPB_B_hbt_P3_MSB /*[5:0]*/                          644
#define	DA_TX_I2MPB_A_tbt_P3_LSB /*[5:0]*/                          645
#define	DA_TX_I2MPB_A_tbt_P3_MSB /*[5:0]*/                          650
#define	DA_TX_I2MPB_B_tbt_P3_LSB /*[5:0]*/                          651
#define	DA_TX_I2MPB_B_tbt_P3_MSB /*[5:0]*/                          656
#define	DA_TX_AMP_OFFSET_A_P3_LSB /*[5:0]*/                         657
#define	DA_TX_AMP_OFFSET_A_P3_MSB /*[5:0]*/                         662
#define	DA_TX_AMP_OFFSET_B_P3_LSB /*[5:0]*/                         663
#define	DA_TX_AMP_OFFSET_B_P3_MSB /*[5:0]*/                         668
#define	DA_TX_AMP_OFFSET_C_P3_LSB /*[5:0]*/                         669
#define	DA_TX_AMP_OFFSET_C_P3_MSB /*[5:0]*/                         674
#define	DA_TX_AMP_OFFSET_D_P3_LSB /*[5:0]*/                         675
#define	DA_TX_AMP_OFFSET_D_P3_MSB /*[5:0]*/                         680
#define	DA_R50OHM_RSEL_TX_A_P3_LSB /*[6:0]*/                        681
#define	DA_R50OHM_RSEL_TX_A_P3_MSB /*[6:0]*/                        687
#define	DA_R50OHM_RSEL_TX_B_P3_LSB /*[6:0]*/                        688
#define	DA_R50OHM_RSEL_TX_B_P3_MSB /*[6:0]*/                        694
#define	DA_R50OHM_RSEL_TX_C_P3_LSB /*[6:0]*/                        695
#define	DA_R50OHM_RSEL_TX_C_P3_MSB /*[6:0]*/                        701
#define	DA_R50OHM_RSEL_TX_D_P3_LSB /*[6:0]*/                        702
#define	DA_R50OHM_RSEL_TX_D_P3_MSB /*[6:0]*/                        708
#define	Reserved_11_LSB                                             709
#define	Reserved_11_MSB                                             712
#define	DDR3_DRVN_imp_cal_LSB                                       713
#define	DDR3_DRVN_imp_cal_MSB                                       719
#define	DDR3_DRVP_imp_cal_LSB                                       720
#define	DDR3_DRVP_imp_cal_MSB                                       726
#define	DDR4_DRVN_imp_cal_LSB                                       727
#define	DDR4_DRVN_imp_cal_MSB                                       733
#define	DDR4_DRVP_imp_cal_LSB                                       734
#define	DDR4_DRVP_imp_cal_MSB                                       740
#define	Reserved_12_LSB                                             741
#define	Reserved_12_MSB                                             744
#define	AVS_TADC_AVS_DIO_CP_LSB /*[15:0]*/                          745
#define	AVS_TADC_AVS_DIO_CP_MSB /*[15:0]*/                          760
#define	AVS_TADC_EXT_BJT_CP_LSB /*[15:0]*/                          761
#define	AVS_TADC_EXT_BJT_CP_MSB /*[15:0]*/                          776
#define	STAL_TADC_STAL_DIO_CP_LSB /*[15:0]*/                        777
#define	STAL_TADC_STAL_DIO_CP_MSB /*[15:0]*/                        792
#define	AVS_TADC_AVS_RES_CP_LSB /*[15:0]*/                          793
#define	AVS_TADC_AVS_RES_CP_MSB /*[15:0]*/                          808
#define	STAL_TADC_STAL_RES_CP_LSB /*[15:0]*/                        809
#define	STAL_TADC_STAL_RES_CP_MSB /*[15:0]*/                        824
#define	Reserved_for_thermal_LSB /*[15:0]*/                     	825
#define	Reserved_for_thermal_MSB /*[15:0]*/                     	840
#define	AVS_Voltage_AVS_Voltage_Cal_1_ADC_LSB /*[15:0]*/            841
#define	AVS_Voltage_AVS_Voltage_Cal_1_ADC_MSB /*[15:0]*/            856
#define	AVS_Voltage_AVS_Voltage_Cal_2_ADC_LSB /*[15:0]*/            857
#define	AVS_Voltage_AVS_Voltage_Cal_2_ADC_MSB /*[15:0]*/            872
#define	Reserved_13_LSB                                             873
#define	Reserved_13_MSB                                             876
#define	IDDQ_LSB                                                    877
#define	IDDQ_MSB                                                    884
#define	CPU_Ring_LSB                                               	885
#define	CPU_Ring_MSB                                               	916
#define	Reserved_14_LSB                                             917
#define	Reserved_14_MSB                                             920


//efuse_LEN                                                 
#define	SerDes_PCIe_efuse_valid_LEN                                 3 
#define	SerDes_USB_efuse_valid_LEN                                  3 
#define	PON_XFI_efuse_valid_LEN                                     1 
#define	GPHY_efuse_valid_LEN                                        1 
#define	AVS_thermal_sensor_efuse_valid_LEN                          1 
#define	Reserved_1_LEN                                              7 
#define	DA_QP_SSUSB_TX_TERM_SEL_LEN /*[1:0]*/                       2 
#define	DA_QP_SSUSB_RX_IMP_SEL_LEN /*[4:0]*/                        5 
#define	Reserved_2_LEN                                              6 
#define	Reserved_3_LEN                                              7 
#define	DA_QP_PCIE_TX_TERM_SEL_LEN /*[1:0]*/                        2 
#define	DA_QP_PCIE_RX_IMP_SEL_LEN /*[4:0]*/                         5 
#define	Reserved_4_LEN                                              6 
#define	Reserved_5_LEN                                              5 
#define	Reserved_6_LEN                                              4 
#define	RG_USB20_P0_INTR_CTRL_LEN /*[4:0]*/                         5 
#define	RG_USB20_P1_INTR_CTRL_LEN /*[4:0]*/                         5 
#define	Reserved_7_LEN                                              4 
#define	RG_PXP_PCIE_CMN_TRIM_LEN /*[4:0]*/                          5 
#define	RG_PXP_PCIE_TX0_TERM_SEL_LEN /*[1:0]*/                      2 
#define	RG_PXP_PCIE_RX0_FE_50OHMS_SEL_LEN /*[1:0]*/                 2 
#define	RG_PXP_PCIE_TX1_TERM_SEL_LEN /*[1:0]*/                      2 
#define	RG_PXP_PCIE_RX1_FE_50OHMS_SEL_LEN /*[1:0]*/                 2 
#define	Reserved_8_LEN                                              4 
#define	RG_PXP_XPON_CMN_TRIM_LEN /*[4:0]*/                          5 
#define	RG_PXP_XPON_TX_TERM_SEL_LEN /*[1:0]*/                       2 
#define	RG_PXP_XPON_RX_FE_50OHMS_SEL_LEN /*[1:0]*/                  2 
#define	Reserved_9_LEN                                             	4 
#define	RG_PXP_XFI_CMN_TRIM_LEN /*[4:0]*/                           5 
#define	RG_PXP_XFI_TX_TERM_SEL_LEN /*[1:0]*/                        2 
#define	RG_PXP_XFI_RX_FE_50OHMS_SEL_LEN /*[1:0]*/                   2 
#define	Reserved_10_LEN                                             4 
#define	Reserved_for_GBE_P0_RG_COMP_CTL_P0_LEN /*[2:0]*/            3 
#define	DA_TX_I2MPB_A_gbe_P0_LEN /*[5:0]*/                          6 
#define	DA_TX_I2MPB_B_gbe_P0_LEN /*[5:0]*/                          6 
#define	DA_TX_I2MPB_C_gbe_P0_LEN /*[5:0]*/                          6 
#define	DA_TX_I2MPB_D_gbe_P0_LEN /*[5:0]*/                          6 
#define	DA_TX_I2MPB_A_tst_P0_LEN /*[5:0]*/                          6 
#define	DA_TX_I2MPB_B_tst_P0_LEN /*[5:0]*/                          6 
#define	DA_TX_I2MPB_C_tst_P0_LEN /*[5:0]*/                          6 
#define	DA_TX_I2MPB_D_tst_P0_LEN /*[5:0]*/                          6 
#define	DA_TX_I2MPB_A_hbt_P0_LEN /*[5:0]*/                          6 
#define	DA_TX_I2MPB_B_hbt_P0_LEN /*[5:0]*/                          6 
#define	DA_TX_I2MPB_A_tbt_P0_LEN /*[5:0]*/                          6 
#define	DA_TX_I2MPB_B_tbt_P0_LEN /*[5:0]*/                          6 
#define	DA_TX_AMP_OFFSET_A_P0_LEN /*[5:0]*/                         6 
#define	DA_TX_AMP_OFFSET_B_P0_LEN /*[5:0]*/                         6 
#define	DA_TX_AMP_OFFSET_C_P0_LEN /*[5:0]*/                         6 
#define	DA_TX_AMP_OFFSET_D_P0_LEN /*[5:0]*/                         6 
#define	DA_R50OHM_RSEL_TX_A_P0_LEN /*[6:0]*/                        7 
#define	DA_R50OHM_RSEL_TX_B_P0_LEN /*[6:0]*/                        7 
#define	DA_R50OHM_RSEL_TX_C_P0_LEN /*[6:0]*/                        7 
#define	DA_R50OHM_RSEL_TX_D_P0_LEN /*[6:0]*/                        7 
#define	Reserved_for_GBE_P1_RG_COMP_CTL_P1_LEN /*[2:0]*/           	3 
#define	DA_TX_I2MPB_A_gbe_P1_LEN /*[5:0]*/                          6 
#define	DA_TX_I2MPB_B_gbe_P1_LEN /*[5:0]*/                          6 
#define	DA_TX_I2MPB_C_gbe_P1_LEN /*[5:0]*/                          6 
#define	DA_TX_I2MPB_D_gbe_P1_LEN /*[5:0]*/                          6 
#define	DA_TX_I2MPB_A_tst_P1_LEN /*[5:0]*/                          6 
#define	DA_TX_I2MPB_B_tst_P1_LEN /*[5:0]*/                          6 
#define	DA_TX_I2MPB_C_tst_P1_LEN /*[5:0]*/                          6 
#define	DA_TX_I2MPB_D_tst_P1_LEN /*[5:0]*/                          6 
#define	DA_TX_I2MPB_A_hbt_P1_LEN /*[5:0]*/                          6 
#define	DA_TX_I2MPB_B_hbt_P1_LEN /*[5:0]*/                          6 
#define	DA_TX_I2MPB_A_tbt_P1_LEN /*[5:0]*/                          6 
#define	DA_TX_I2MPB_B_tbt_P1_LEN /*[5:0]*/                          6 
#define	DA_TX_AMP_OFFSET_A_P1_LEN /*[5:0]*/                         6 
#define	DA_TX_AMP_OFFSET_B_P1_LEN /*[5:0]*/                         6 
#define	DA_TX_AMP_OFFSET_C_P1_LEN /*[5:0]*/                         6 
#define	DA_TX_AMP_OFFSET_D_P1_LEN /*[5:0]*/                         6 
#define	DA_R50OHM_RSEL_TX_A_P1_LEN /*[6:0]*/                        7 
#define	DA_R50OHM_RSEL_TX_B_P1_LEN /*[6:0]*/                        7 
#define	DA_R50OHM_RSEL_TX_C_P1_LEN /*[6:0]*/                        7 
#define	DA_R50OHM_RSEL_TX_D_P1_LEN /*[6:0]*/                        7 
#define	RG_BG_RASEL_LEN /*[2:0]*/                                   3 
#define	RG_REXT_TRIM_LEN /*[5:0]*/                                  6 
#define	DA_TX_PS_DRIR0_hbt_tbt_LEN /*[3:0]*/                        4 
#define	DA_TX_PS_DRIR0_B_hbt_tbt_LEN /*[3:0]*/                      4 
#define	DA_TX_PS_OP_hbt_LEN /*[4:0]*/                               5 
#define	DA_TX_PS_OP_B_hbt_LEN /*[4:0]*/                             5 
#define	DA_TX_PS_OP_tbt_LEN /*[4:0]*/                               5 
#define	DA_TX_PS_OP_B_tbt_LEN /*[4:0]*/                             5 
#define	DA_TX_RM2P_OP_LEN /*[2:0]*/                                 3 
#define	DA_TX_RM2P_OP_B_LEN /*[2:0]*/                               3 
#define	DA_TX_CM1_OP_LEN /*[3:0]*/                                  4 
#define	RG_HVGA_ECHO_GAIN_LEN /*[2:0]*/                             3 
#define	RG_HVGA_ECHO_RES_LEN /*[0]*/                                1 
#define	DA_ADBUF_BIAS_gbe_LEN /*[1:0]*/                             2 
#define	DA_ADBUF_BIAS_lp_LEN /*[1:0]*/                              2 
#define	Reserved_for_GBE_P2_RG_COMP_CTL_P2_LEN /*[2:0]*/            3 
#define	DA_TX_I2MPB_A_gbe_P2_LEN /*[5:0]*/                          6 
#define	DA_TX_I2MPB_B_gbe_P2_LEN /*[5:0]*/                          6 
#define	DA_TX_I2MPB_C_gbe_P2_LEN /*[5:0]*/                          6 
#define	DA_TX_I2MPB_D_gbe_P2_LEN /*[5:0]*/                          6 
#define	DA_TX_I2MPB_A_tst_P2_LEN /*[5:0]*/                          6 
#define	DA_TX_I2MPB_B_tst_P2_LEN /*[5:0]*/                          6 
#define	DA_TX_I2MPB_C_tst_P2_LEN /*[5:0]*/                          6 
#define	DA_TX_I2MPB_D_tst_P2_LEN /*[5:0]*/                          6 
#define	DA_TX_I2MPB_A_hbt_P2_LEN /*[5:0]*/                          6 
#define	DA_TX_I2MPB_B_hbt_P2_LEN /*[5:0]*/                          6 
#define	DA_TX_I2MPB_A_tbt_P2_LEN /*[5:0]*/                          6 
#define	DA_TX_I2MPB_B_tbt_P2_LEN /*[5:0]*/                          6 
#define	DA_TX_AMP_OFFSET_A_P2_LEN /*[5:0]*/                         6 
#define	DA_TX_AMP_OFFSET_B_P2_LEN /*[5:0]*/                         6 
#define	DA_TX_AMP_OFFSET_C_P2_LEN /*[5:0]*/                         6 
#define	DA_TX_AMP_OFFSET_D_P2_LEN /*[5:0]*/                         6 
#define	DA_R50OHM_RSEL_TX_A_P2_LEN /*[6:0]*/                        7 
#define	DA_R50OHM_RSEL_TX_B_P2_LEN /*[6:0]*/                        7 
#define	DA_R50OHM_RSEL_TX_C_P2_LEN /*[6:0]*/                        7 
#define	DA_R50OHM_RSEL_TX_D_P2_LEN /*[6:0]*/                        7 
#define	Reserved_for_GBE_P3_RG_COMP_CTL_P3_LEN /*[2:0]*/            3 
#define	DA_TX_I2MPB_A_gbe_P3_LEN /*[5:0]*/                          6 
#define	DA_TX_I2MPB_B_gbe_P3_LEN /*[5:0]*/                          6 
#define	DA_TX_I2MPB_C_gbe_P3_LEN /*[5:0]*/                          6 
#define	DA_TX_I2MPB_D_gbe_P3_LEN /*[5:0]*/                          6 
#define	DA_TX_I2MPB_A_tst_P3_LEN /*[5:0]*/                          6 
#define	DA_TX_I2MPB_B_tst_P3_LEN /*[5:0]*/                          6 
#define	DA_TX_I2MPB_C_tst_P3_LEN /*[5:0]*/                          6 
#define	DA_TX_I2MPB_D_tst_P3_LEN /*[5:0]*/                          6 
#define	DA_TX_I2MPB_A_hbt_P3_LEN /*[5:0]*/                          6 
#define	DA_TX_I2MPB_B_hbt_P3_LEN /*[5:0]*/                          6 
#define	DA_TX_I2MPB_A_tbt_P3_LEN /*[5:0]*/                          6 
#define	DA_TX_I2MPB_B_tbt_P3_LEN /*[5:0]*/                          6 
#define	DA_TX_AMP_OFFSET_A_P3_LEN /*[5:0]*/                         6 
#define	DA_TX_AMP_OFFSET_B_P3_LEN /*[5:0]*/                         6 
#define	DA_TX_AMP_OFFSET_C_P3_LEN /*[5:0]*/                         6 
#define	DA_TX_AMP_OFFSET_D_P3_LEN /*[5:0]*/                         6 
#define	DA_R50OHM_RSEL_TX_A_P3_LEN /*[6:0]*/                        7 
#define	DA_R50OHM_RSEL_TX_B_P3_LEN /*[6:0]*/                        7 
#define	DA_R50OHM_RSEL_TX_C_P3_LEN /*[6:0]*/                        7 
#define	DA_R50OHM_RSEL_TX_D_P3_LEN /*[6:0]*/                        7 
#define	Reserved_11_LEN                                             4 
#define	DDR3_DRVN_imp_cal_LEN                                       7 
#define	DDR3_DRVP_imp_cal_LEN                                       7 
#define	DDR4_DRVN_imp_cal_LEN                                       7 
#define	DDR4_DRVP_imp_cal_LEN                                       7 
#define	Reserved_12_LEN                                             4 
#define	AVS_TADC_AVS_DIO_CP_LEN /*[15:0]*/                          16
#define	AVS_TADC_EXT_BJT_CP_LEN /*[15:0]*/                          16
#define	STAL_TADC_STAL_DIO_CP_LEN /*[15:0]*/                        16
#define	AVS_TADC_AVS_RES_CP_LEN /*[15:0]*/                          16
#define	STAL_TADC_STAL_RES_CP_LEN /*[15:0]*/                        16
#define	Reserved_for_thermal_LEN /*[15:0]*/                     	16
#define	AVS_Voltage_AVS_Voltage_Cal_1_ADC_LEN /*[15:0]*/            16
#define	AVS_Voltage_AVS_Voltage_Cal_2_ADC_LEN /*[15:0]*/            16
#define	Reserved_13_LEN                                             4 
#define	IDDQ_LEN                                                    8 
#define	CPU_Ring_LEN                                               	32
#define	Reserved_14_LEN                                             4


#endif /*PHY_EFUSE_DEF_H*/
