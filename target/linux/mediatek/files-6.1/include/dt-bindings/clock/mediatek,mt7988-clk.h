/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-2-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
 * Author: Sam Shih <sam.shih@mediatek.com>
 * Author: Xiufeng Li <Xiufeng.Li@mediatek.com>
 */

#ifndef _DT_BINDINGS_CLK_MT7988_H
#define _DT_BINDINGS_CLK_MT7988_H

/* APMIXEDSYS */

#define CLK_APMIXED_NETSYSPLL  0
#define CLK_APMIXED_MPLL       1
#define CLK_APMIXED_MMPLL      2
#define CLK_APMIXED_APLL2      3
#define CLK_APMIXED_NET1PLL    4
#define CLK_APMIXED_NET2PLL    5
#define CLK_APMIXED_WEDMCUPLL  6
#define CLK_APMIXED_SGMPLL     7
#define CLK_APMIXED_ARM_B      8
#define CLK_APMIXED_CCIPLL2_B  9
#define CLK_APMIXED_USXGMIIPLL 10
#define CLK_APMIXED_MSDCPLL    11

/* TOPCKGEN */

#define CLK_TOP_XTAL		       0
#define CLK_TOP_XTAL_D2		       1
#define CLK_TOP_RTC_32K		       2
#define CLK_TOP_RTC_32P7K	       3
#define CLK_TOP_MPLL_D2		       4
#define CLK_TOP_MPLL_D3_D2	       5
#define CLK_TOP_MPLL_D4		       6
#define CLK_TOP_MPLL_D8		       7
#define CLK_TOP_MPLL_D8_D2	       8
#define CLK_TOP_MMPLL_D2	       9
#define CLK_TOP_MMPLL_D3_D5	       10
#define CLK_TOP_MMPLL_D4	       11
#define CLK_TOP_MMPLL_D6_D2	       12
#define CLK_TOP_MMPLL_D8	       13
#define CLK_TOP_APLL2_D4	       14
#define CLK_TOP_NET1PLL_D4	       15
#define CLK_TOP_NET1PLL_D5	       16
#define CLK_TOP_NET1PLL_D5_D2	       17
#define CLK_TOP_NET1PLL_D5_D4	       18
#define CLK_TOP_NET1PLL_D8	       19
#define CLK_TOP_NET1PLL_D8_D2	       20
#define CLK_TOP_NET1PLL_D8_D4	       21
#define CLK_TOP_NET1PLL_D8_D8	       22
#define CLK_TOP_NET1PLL_D8_D16	       23
#define CLK_TOP_NET2PLL_D2	       24
#define CLK_TOP_NET2PLL_D4	       25
#define CLK_TOP_NET2PLL_D4_D4	       26
#define CLK_TOP_NET2PLL_D4_D8	       27
#define CLK_TOP_NET2PLL_D6	       28
#define CLK_TOP_NET2PLL_D8	       29
#define CLK_TOP_NETSYS_SEL	       30
#define CLK_TOP_NETSYS_500M_SEL	       31
#define CLK_TOP_NETSYS_2X_SEL	       32
#define CLK_TOP_NETSYS_GSW_SEL	       33
#define CLK_TOP_ETH_GMII_SEL	       34
#define CLK_TOP_NETSYS_MCU_SEL	       35
#define CLK_TOP_NETSYS_PAO_2X_SEL      36
#define CLK_TOP_EIP197_SEL	       37
#define CLK_TOP_AXI_INFRA_SEL	       38
#define CLK_TOP_UART_SEL	       39
#define CLK_TOP_EMMC_250M_SEL	       40
#define CLK_TOP_EMMC_400M_SEL	       41
#define CLK_TOP_SPI_SEL		       42
#define CLK_TOP_SPIM_MST_SEL	       43
#define CLK_TOP_NFI1X_SEL	       44
#define CLK_TOP_SPINFI_SEL	       45
#define CLK_TOP_PWM_SEL		       46
#define CLK_TOP_I2C_SEL		       47
#define CLK_TOP_PCIE_MBIST_250M_SEL    48
#define CLK_TOP_PEXTP_TL_SEL	       49
#define CLK_TOP_PEXTP_TL_P1_SEL	       50
#define CLK_TOP_PEXTP_TL_P2_SEL	       51
#define CLK_TOP_PEXTP_TL_P3_SEL	       52
#define CLK_TOP_USB_SYS_SEL	       53
#define CLK_TOP_USB_SYS_P1_SEL	       54
#define CLK_TOP_USB_XHCI_SEL	       55
#define CLK_TOP_USB_XHCI_P1_SEL	       56
#define CLK_TOP_USB_FRMCNT_SEL	       57
#define CLK_TOP_USB_FRMCNT_P1_SEL      58
#define CLK_TOP_AUD_SEL		       59
#define CLK_TOP_A1SYS_SEL	       60
#define CLK_TOP_AUD_L_SEL	       61
#define CLK_TOP_A_TUNER_SEL	       62
#define CLK_TOP_SSPXTP_SEL	       63
#define CLK_TOP_USB_PHY_SEL	       64
#define CLK_TOP_USXGMII_SBUS_0_SEL     65
#define CLK_TOP_USXGMII_SBUS_1_SEL     66
#define CLK_TOP_SGM_0_SEL	       67
#define CLK_TOP_SGM_SBUS_0_SEL	       68
#define CLK_TOP_SGM_1_SEL	       69
#define CLK_TOP_SGM_SBUS_1_SEL	       70
#define CLK_TOP_XFI_PHY_0_XTAL_SEL     71
#define CLK_TOP_XFI_PHY_1_XTAL_SEL     72
#define CLK_TOP_SYSAXI_SEL	       73
#define CLK_TOP_SYSAPB_SEL	       74
#define CLK_TOP_ETH_REFCK_50M_SEL      75
#define CLK_TOP_ETH_SYS_200M_SEL       76
#define CLK_TOP_ETH_SYS_SEL	       77
#define CLK_TOP_ETH_XGMII_SEL	       78
#define CLK_TOP_BUS_TOPS_SEL	       79
#define CLK_TOP_NPU_TOPS_SEL	       80
#define CLK_TOP_DRAMC_SEL	       81
#define CLK_TOP_DRAMC_MD32_SEL	       82
#define CLK_TOP_INFRA_F26M_SEL	       83
#define CLK_TOP_PEXTP_P0_SEL	       84
#define CLK_TOP_PEXTP_P1_SEL	       85
#define CLK_TOP_PEXTP_P2_SEL	       86
#define CLK_TOP_PEXTP_P3_SEL	       87
#define CLK_TOP_DA_XTP_GLB_P0_SEL      88
#define CLK_TOP_DA_XTP_GLB_P1_SEL      89
#define CLK_TOP_DA_XTP_GLB_P2_SEL      90
#define CLK_TOP_DA_XTP_GLB_P3_SEL      91
#define CLK_TOP_CKM_SEL		       92
#define CLK_TOP_DA_SEL		       93
#define CLK_TOP_PEXTP_SEL	       94
#define CLK_TOP_TOPS_P2_26M_SEL	       95
#define CLK_TOP_MCUSYS_BACKUP_625M_SEL 96
#define CLK_TOP_NETSYS_SYNC_250M_SEL   97
#define CLK_TOP_MACSEC_SEL	       98
#define CLK_TOP_NETSYS_TOPS_400M_SEL   99
#define CLK_TOP_NETSYS_PPEFB_250M_SEL  100
#define CLK_TOP_NETSYS_WARP_SEL	       101
#define CLK_TOP_ETH_MII_SEL	       102
#define CLK_TOP_NPU_SEL		       103
#define CLK_TOP_AUD_I2S_M	       104

/* MCUSYS */

#define CLK_MCU_BUS_DIV_SEL 0
#define CLK_MCU_ARM_DIV_SEL 1

/* INFRACFG_AO */

#define CLK_INFRA_MUX_UART0_SEL		 0
#define CLK_INFRA_MUX_UART1_SEL		 1
#define CLK_INFRA_MUX_UART2_SEL		 2
#define CLK_INFRA_MUX_SPI0_SEL		 3
#define CLK_INFRA_MUX_SPI1_SEL		 4
#define CLK_INFRA_MUX_SPI2_SEL		 5
#define CLK_INFRA_PWM_SEL		 6
#define CLK_INFRA_PWM_CK1_SEL		 7
#define CLK_INFRA_PWM_CK2_SEL		 8
#define CLK_INFRA_PWM_CK3_SEL		 9
#define CLK_INFRA_PWM_CK4_SEL		 10
#define CLK_INFRA_PWM_CK5_SEL		 11
#define CLK_INFRA_PWM_CK6_SEL		 12
#define CLK_INFRA_PWM_CK7_SEL		 13
#define CLK_INFRA_PWM_CK8_SEL		 14
#define CLK_INFRA_PCIE_GFMUX_TL_O_P0_SEL 15
#define CLK_INFRA_PCIE_GFMUX_TL_O_P1_SEL 16
#define CLK_INFRA_PCIE_GFMUX_TL_O_P2_SEL 17
#define CLK_INFRA_PCIE_GFMUX_TL_O_P3_SEL 18

/* INFRACFG */

#define CLK_INFRA_PCIE_PERI_26M_CK_P0 19
#define CLK_INFRA_PCIE_PERI_26M_CK_P1 20
#define CLK_INFRA_PCIE_PERI_26M_CK_P2 21
#define CLK_INFRA_PCIE_PERI_26M_CK_P3 22
#define CLK_INFRA_66M_GPT_BCK	      23
#define CLK_INFRA_66M_PWM_HCK	      24
#define CLK_INFRA_66M_PWM_BCK	      25
#define CLK_INFRA_66M_PWM_CK1	      26
#define CLK_INFRA_66M_PWM_CK2	      27
#define CLK_INFRA_66M_PWM_CK3	      28
#define CLK_INFRA_66M_PWM_CK4	      29
#define CLK_INFRA_66M_PWM_CK5	      30
#define CLK_INFRA_66M_PWM_CK6	      31
#define CLK_INFRA_66M_PWM_CK7	      32
#define CLK_INFRA_66M_PWM_CK8	      33
#define CLK_INFRA_133M_CQDMA_BCK      34
#define CLK_INFRA_66M_AUD_SLV_BCK     35
#define CLK_INFRA_AUD_26M	      36
#define CLK_INFRA_AUD_L		      37
#define CLK_INFRA_AUD_AUD	      38
#define CLK_INFRA_AUD_EG2	      39
#define CLK_INFRA_DRAMC_F26M	      40
#define CLK_INFRA_133M_DBG_ACKM	      41
#define CLK_INFRA_66M_AP_DMA_BCK      42
#define CLK_INFRA_66M_SEJ_BCK	      43
#define CLK_INFRA_PRE_CK_SEJ_F13M     44
#define CLK_INFRA_26M_THERM_SYSTEM    45
#define CLK_INFRA_I2C_BCK	      46
#define CLK_INFRA_52M_UART0_CK	      47
#define CLK_INFRA_52M_UART1_CK	      48
#define CLK_INFRA_52M_UART2_CK	      49
#define CLK_INFRA_NFI		      50
#define CLK_INFRA_SPINFI	      51
#define CLK_INFRA_66M_NFI_HCK	      52
#define CLK_INFRA_104M_SPI0	      53
#define CLK_INFRA_104M_SPI1	      54
#define CLK_INFRA_104M_SPI2_BCK	      55
#define CLK_INFRA_66M_SPI0_HCK	      56
#define CLK_INFRA_66M_SPI1_HCK	      57
#define CLK_INFRA_66M_SPI2_HCK	      58
#define CLK_INFRA_66M_FLASHIF_AXI     59
#define CLK_INFRA_RTC		      60
#define CLK_INFRA_26M_ADC_BCK	      61
#define CLK_INFRA_RC_ADC	      62
#define CLK_INFRA_MSDC400	      63
#define CLK_INFRA_MSDC2_HCK	      64
#define CLK_INFRA_133M_MSDC_0_HCK     65
#define CLK_INFRA_66M_MSDC_0_HCK      66
#define CLK_INFRA_133M_CPUM_BCK	      67
#define CLK_INFRA_BIST2FPC	      68
#define CLK_INFRA_I2C_X16W_MCK_CK_P1  69
#define CLK_INFRA_I2C_X16W_PCK_CK_P1  70
#define CLK_INFRA_133M_USB_HCK	      71
#define CLK_INFRA_133M_USB_HCK_CK_P1  72
#define CLK_INFRA_66M_USB_HCK	      73
#define CLK_INFRA_66M_USB_HCK_CK_P1   74
#define CLK_INFRA_USB_SYS	      75
#define CLK_INFRA_USB_SYS_CK_P1	      76
#define CLK_INFRA_USB_REF	      77
#define CLK_INFRA_USB_CK_P1	      78
#define CLK_INFRA_USB_FRMCNT	      79
#define CLK_INFRA_USB_FRMCNT_CK_P1    80
#define CLK_INFRA_USB_PIPE	      81
#define CLK_INFRA_USB_PIPE_CK_P1      82
#define CLK_INFRA_USB_UTMI	      83
#define CLK_INFRA_USB_UTMI_CK_P1      84
#define CLK_INFRA_USB_XHCI	      85
#define CLK_INFRA_USB_XHCI_CK_P1      86
#define CLK_INFRA_PCIE_GFMUX_TL_P0    87
#define CLK_INFRA_PCIE_GFMUX_TL_P1    88
#define CLK_INFRA_PCIE_GFMUX_TL_P2    89
#define CLK_INFRA_PCIE_GFMUX_TL_P3    90
#define CLK_INFRA_PCIE_PIPE_P0	      91
#define CLK_INFRA_PCIE_PIPE_P1	      92
#define CLK_INFRA_PCIE_PIPE_P2	      93
#define CLK_INFRA_PCIE_PIPE_P3	      94
#define CLK_INFRA_133M_PCIE_CK_P0     95
#define CLK_INFRA_133M_PCIE_CK_P1     96
#define CLK_INFRA_133M_PCIE_CK_P2     97
#define CLK_INFRA_133M_PCIE_CK_P3     98

/* ETHDMA */

#define CLK_ETHDMA_XGP1_EN   0
#define CLK_ETHDMA_XGP2_EN   1
#define CLK_ETHDMA_XGP3_EN   2
#define CLK_ETHDMA_FE_EN     3
#define CLK_ETHDMA_GP2_EN    4
#define CLK_ETHDMA_GP1_EN    5
#define CLK_ETHDMA_GP3_EN    6
#define CLK_ETHDMA_ESW_EN    7
#define CLK_ETHDMA_CRYPT0_EN 8
#define CLK_ETHDMA_NR_CLK    9

/* SGMIISYS_0 */

#define CLK_SGM0_TX_EN	  0
#define CLK_SGM0_RX_EN	  1
#define CLK_SGMII0_NR_CLK 2

/* SGMIISYS_1 */

#define CLK_SGM1_TX_EN	  0
#define CLK_SGM1_RX_EN	  1
#define CLK_SGMII1_NR_CLK 2

/* ETHWARP */

#define CLK_ETHWARP_WOCPU2_EN 0
#define CLK_ETHWARP_WOCPU1_EN 1
#define CLK_ETHWARP_WOCPU0_EN 2
#define CLK_ETHWARP_NR_CLK    3

#endif /* _DT_BINDINGS_CLK_MT7988_H */
