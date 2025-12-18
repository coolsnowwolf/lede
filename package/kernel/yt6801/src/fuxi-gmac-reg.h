/* SPDX-License-Identifier: GPL-2.0+ */
/* Copyright (c) 2021 Motor-comm Corporation. All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef __FXGMAC_GMAC_REG_H__
#define __FXGMAC_GMAC_REG_H__

#define  AISC_MODE

#define FXGMAC_REV_01                             0x01    // The first NTO version.
#define FXGMAC_REV_03                             0x03    // ECO back on 07/2023.

/* MAC register offsets */
#define MAC_OFFSET                              0x2000
#define MAC_CR                                  0x0000  //The MAC Configuration Register
#define MAC_ECR                                 0x0004
#define MAC_PFR                                 0x0008
#define MAC_WTR                                 0x000c
#define MAC_HTR0                                0x0010
#define MAC_VLANTR                              0x0050
#define MAC_VLANHTR                             0x0058
#define MAC_VLANIR                              0x0060
#define MAC_IVLANIR                             0x0064
#define MAC_Q0TFCR                              0x0070
#define MAC_RFCR                                0x0090
#define MAC_RQC0R                               0x00a0
#define MAC_RQC1R                               0x00a4
#define MAC_RQC2R                               0x00a8
#define MAC_RQC3R                               0x00ac
#define MAC_ISR                                 0x00b0
#define MAC_IER                                 0x00b4
#define MAC_TX_RX_STA                           0x00b8
#define MAC_PMT_STA                             0x00c0
#define MAC_RWK_PAC                             0x00c4 // This is the FIFO address, the pointer will be increased automatically after writing.
#define MAC_LPI_STA                             0x00d0
#define MAC_LPI_CONTROL                         0x00d4
#define MAC_LPI_TIMER                           0x00d8
#define MAC_MS_TIC_COUNTER                      0x00dc
#define MAC_AN_CR                               0x00e0
#define MAC_AN_SR                               0x00e4
#define MAC_AN_ADV                              0x00e8
#define MAC_AN_LPA                              0x00ec
#define MAC_AN_EXP                              0x00f0
#define MAC_PHYIF_STA                           0x00f8
#define MAC_VR                                  0x0110
#define MAC_DBG_STA                             0x0114
#define MAC_HWF0R                               0x011c
#define MAC_HWF1R                               0x0120
#define MAC_HWF2R                               0x0124
#define MAC_HWF3R                               0x0128
#define MAC_MDIO_ADDRESS                        0x0200
#define MAC_MDIO_DATA                           0x0204
#define MAC_GPIOCR                              0x0208
#define MAC_GPIO_SR                             0x020c
#define MAC_ARP_PROTO_ADDR                      0x0210
#define MAC_CSR_SW_CTRL                         0x0230

#define MAC_MACA0HR                             0x0300  // mac[5]->bit15:8, mac[4]->bit7:0
#define MAC_MACA0LR                             0x0304  // mac[0]->bit7:0, mac[1]->bit15:8, mac[2]->bit23:16, mac[3]->bit31:24

#define MAC_MACA1HR                             0x0308
#define MAC_MACA1HR_AE_POS                      31
#define MAC_MACA1HR_AE_LEN                      1

#define MAC_MACA1LR                             0x030c


#define MAC_RSSCR                               0x3c80//TODO
#define MAC_RSSAR                               0x3c88//TODO
#define MAC_RSSDR                               0x3c8c//TODO


#define MAC_QTFCR_INC               4
#define MAC_MACA_INC                4
#define MAC_HTR_INC                 4
#define MAC_RQC2_INC                4
#define MAC_RQC2_Q_PER_REG          4

/* MAC register entry bit positions and sizes */
#define MAC_HWF0R_ADDMACADRSEL_POS  18
#define MAC_HWF0R_ADDMACADRSEL_LEN  5
#define MAC_HWF0R_ARPOFFSEL_POS     9
#define MAC_HWF0R_ARPOFFSEL_LEN     1
#define MAC_HWF0R_EEESEL_POS        13
#define MAC_HWF0R_EEESEL_LEN        1
#define MAC_HWF0R_ACTPHYIFSEL_POS   28
#define MAC_HWF0R_ACTPHYIFSEL_LEN   3
#define MAC_HWF0R_MGKSEL_POS        7
#define MAC_HWF0R_MGKSEL_LEN        1
#define MAC_HWF0R_MMCSEL_POS        8
#define MAC_HWF0R_MMCSEL_LEN        1
#define MAC_HWF0R_RWKSEL_POS        6
#define MAC_HWF0R_RWKSEL_LEN        1
#define MAC_HWF0R_RXCOESEL_POS      16
#define MAC_HWF0R_RXCOESEL_LEN      1
#define MAC_HWF0R_SAVLANINS_POS     27
#define MAC_HWF0R_SAVLANINS_LEN     1
#define MAC_HWF0R_SMASEL_POS        5
#define MAC_HWF0R_SMASEL_LEN        1
#define MAC_HWF0R_TSSEL_POS         12
#define MAC_HWF0R_TSSEL_LEN         1
#define MAC_HWF0R_TSSTSSEL_POS      25
#define MAC_HWF0R_TSSTSSEL_LEN      2
#define MAC_HWF0R_TXCOESEL_POS      14
#define MAC_HWF0R_TXCOESEL_LEN      1
#define MAC_HWF0R_VLHASH_POS        4
#define MAC_HWF0R_VLHASH_LEN        1
#define MAC_HWF1R_ADDR64_POS        14
#define MAC_HWF1R_ADDR64_LEN        2
#define MAC_HWF1R_ADVTHWORD_POS     13
#define MAC_HWF1R_ADVTHWORD_LEN     1
#define MAC_HWF1R_DBGMEMA_POS       19
#define MAC_HWF1R_DBGMEMA_LEN       1
#define MAC_HWF1R_DCBEN_POS         16
#define MAC_HWF1R_DCBEN_LEN         1
#define MAC_HWF1R_HASHTBLSZ_POS     24
#define MAC_HWF1R_HASHTBLSZ_LEN     2
#define MAC_HWF1R_L3L4FNUM_POS      27
#define MAC_HWF1R_L3L4FNUM_LEN      4
//#define MAC_HWF1R_NUMTC_POS       21
//#define MAC_HWF1R_NUMTC_LEN       3
//#define MAC_HWF1R_RSSEN_POS       20
//#define MAC_HWF1R_RSSEN_LEN       1
#define MAC_HWF1R_RAVSEL_POS        21
#define MAC_HWF1R_RAVSEL_LEN        1
#define MAC_HWF1R_AVSEL_POS         20
#define MAC_HWF1R_AVSEL_LEN         1
#define MAC_HWF1R_RXFIFOSIZE_POS    0
#define MAC_HWF1R_RXFIFOSIZE_LEN    5
#define MAC_HWF1R_SPHEN_POS         17
#define MAC_HWF1R_SPHEN_LEN         1
#define MAC_HWF1R_TSOEN_POS         18
#define MAC_HWF1R_TSOEN_LEN         1
#define MAC_HWF1R_TXFIFOSIZE_POS    6
#define MAC_HWF1R_TXFIFOSIZE_LEN    5
#define MAC_HWF2R_AUXSNAPNUM_POS    28
#define MAC_HWF2R_AUXSNAPNUM_LEN    3
#define MAC_HWF2R_PPSOUTNUM_POS     24
#define MAC_HWF2R_PPSOUTNUM_LEN     3
#define MAC_HWF2R_RXCHCNT_POS       12
#define MAC_HWF2R_RXCHCNT_LEN       4
#define MAC_HWF2R_RXQCNT_POS        0
#define MAC_HWF2R_RXQCNT_LEN        4
#define MAC_HWF2R_TXCHCNT_POS       18
#define MAC_HWF2R_TXCHCNT_LEN       4
#define MAC_HWF2R_TXQCNT_POS        6
#define MAC_HWF2R_TXQCNT_LEN        4
#define MAC_IER_TSIE_POS            12
#define MAC_IER_TSIE_LEN            1
#define MAC_ISR_MMCRXIS_POS         9
#define MAC_ISR_MMCRXIS_LEN         1
#define MAC_ISR_MMCTXIS_POS         10
#define MAC_ISR_MMCTXIS_LEN         1
#define MAC_ISR_PMTIS_POS           4
#define MAC_ISR_PMTIS_LEN           1
#define MAC_ISR_TSIS_POS            12
#define MAC_ISR_TSIS_LEN            1
#define MAC_MACA1HR_AE_POS          31
#define MAC_MACA1HR_AE_LEN          1
#define MAC_PFR_HMC_POS             2
#define MAC_PFR_HMC_LEN             1
#define MAC_PFR_HPF_POS             10
#define MAC_PFR_HPF_LEN             1
#define MAC_PFR_PM_POS              4  // Pass all Multicast.
#define MAC_PFR_PM_LEN              1
#define MAC_PFR_DBF_POS             5  // Disable Broadcast Packets.
#define MAC_PFR_DBF_LEN             1
#define MAC_PFR_HUC_POS             1 // Hash Unicast. 0x0 (DISABLE). compares the DA field with the values programmed in DA registers.
#define MAC_PFR_HUC_LEN             1
#define MAC_PFR_PR_POS              0 // Enable Promiscuous Mode.
#define MAC_PFR_PR_LEN              1
#define MAC_PFR_VTFE_POS            16
#define MAC_PFR_VTFE_LEN            1
#define MAC_Q0TFCR_PT_POS           16
#define MAC_Q0TFCR_PT_LEN           16
#define MAC_Q0TFCR_TFE_POS          1
#define MAC_Q0TFCR_TFE_LEN          1
#define MAC_CR_ARPEN_POS            31
#define MAC_CR_ARPEN_LEN            1
#define MAC_CR_ACS_POS              20
#define MAC_CR_ACS_LEN              1
#define MAC_CR_CST_POS              21
#define MAC_CR_CST_LEN              1
#define MAC_CR_IPC_POS              27
#define MAC_CR_IPC_LEN              1
#define MAC_CR_JE_POS               16
#define MAC_CR_JE_LEN               1
#define MAC_CR_LM_POS               12
#define MAC_CR_LM_LEN               1
#define MAC_CR_RE_POS               0
#define MAC_CR_RE_LEN               1
#define MAC_CR_PS_POS               15
#define MAC_CR_PS_LEN               1
#define MAC_CR_FES_POS              14
#define MAC_CR_FES_LEN              1
#define MAC_CR_DM_POS               13
#define MAC_CR_DM_LEN               1
#define MAC_CR_TE_POS               1
#define MAC_CR_TE_LEN               1
#define MAC_ECR_DCRCC_POS           16
#define MAC_ECR_DCRCC_LEN           1
#define MAC_ECR_HDSMS_POS           20
#define MAC_ECR_HDSMS_LEN           3
#define MAC_RFCR_PFCE_POS           8
#define MAC_RFCR_PFCE_LEN           1
#define MAC_RFCR_RFE_POS            0
#define MAC_RFCR_RFE_LEN            1
#define MAC_RFCR_UP_POS             1
#define MAC_RFCR_UP_LEN             1
#define MAC_RQC0R_RXQ0EN_POS        0
#define MAC_RQC0R_RXQ0EN_LEN        2
#define MAC_LPIIE_POS               5
#define MAC_LPIIE_LEN               1
#define MAC_LPIATE_POS              20
#define MAC_LPIATE_LEN              1
#define MAC_LPITXA_POS              19
#define MAC_LPITXA_LEN              1
#define MAC_PLS_POS                 17
#define MAC_PLS_LEN                 1
#define MAC_LPIEN_POS               16
#define MAC_LPIEN_LEN               1
#define MAC_LPI_ENTRY_TIMER         8
#define MAC_LPIET_POS               3
#define MAC_LPIET_LEN               17
#define MAC_TWT_TIMER               0x10
#define MAC_TWT_POS                 0
#define MAC_TWT_LEN                 16
#define MAC_LST_TIMER               2
#define MAC_LST_POS                 16
#define MAC_LST_LEN                 10
#define MAC_MS_TIC                  24
#define MAC_MS_TIC_POS              0
#define MAC_MS_TIC_LEN              12

/* RSS table */
#define MAC_RSSAR_ADDRT_POS         2
#define MAC_RSSAR_ADDRT_LEN         1
#define MAC_RSSAR_CT_POS            1
#define MAC_RSSAR_CT_LEN            1
#define MAC_RSSAR_OB_POS            0
#define MAC_RSSAR_OB_LEN            1
#define MAC_RSSAR_RSSIA_POS         8
#define MAC_RSSAR_RSSIA_LEN         8

/* RSS control and options */
/* note, below options definitions are used only for pdata->options,
 * not for register, so the position is not consistent with register.
 * [0] ipv4
 * [1] tcpv4
 * [2] udpv4
 * [3] ipv6
 * [4] tcpv6
 * [5] udpv6
 */
#define MAC_RSSCR_IP4TE_POS         0
#define MAC_RSSCR_IP4TE_LEN         1
#define MAC_RSSCR_IP6TE_POS         3
#define MAC_RSSCR_IP6TE_LEN         1
#define MAC_RSSCR_TCP4TE_POS        1
#define MAC_RSSCR_TCP4TE_LEN        1
#define MAC_RSSCR_UDP4TE_POS        2
#define MAC_RSSCR_UDP4TE_LEN        1
#define MAC_RSSCR_TCP6TE_POS        4
#define MAC_RSSCR_TCP6TE_LEN        1
#define MAC_RSSCR_UDP6TE_POS        5
#define MAC_RSSCR_UDP6TE_LEN        1

/* RSS indirection table */
#define MAC_RSSDR_DMCH_POS          0
#define MAC_RSSDR_DMCH_LEN          2

#define MAC_VLANHTR_VLHT_POS        0
#define MAC_VLANHTR_VLHT_LEN        16
#define MAC_VLANIR_VLTI_POS         20
#define MAC_VLANIR_VLTI_LEN         1
#define MAC_VLANIR_CSVL_POS         19
#define MAC_VLANIR_CSVL_LEN         1
#define MAC_VLANIR_VLP_POS          18
#define MAC_VLANIR_VLP_LEN          1
#define MAC_VLANIR_VLC_POS          16
#define MAC_VLANIR_VLC_LEN          2
#define MAC_VLANIR_VLT_POS          0
#define MAC_VLANIR_VLT_LEN          16
#define MAC_VLANTR_DOVLTC_POS       20
#define MAC_VLANTR_DOVLTC_LEN       1
#define MAC_VLANTR_ERSVLM_POS       19
#define MAC_VLANTR_ERSVLM_LEN       1
#define MAC_VLANTR_ESVL_POS         18
#define MAC_VLANTR_ESVL_LEN         1
#define MAC_VLANTR_ETV_POS          16
#define MAC_VLANTR_ETV_LEN          1
#define MAC_VLANTR_EVLS_POS         21
#define MAC_VLANTR_EVLS_LEN         2
#define MAC_VLANTR_EVLRXS_POS       24
#define MAC_VLANTR_EVLRXS_LEN       1
#define MAC_VLANTR_VL_POS           0
#define MAC_VLANTR_VL_LEN           16
#define MAC_VLANTR_VTHM_POS         25
#define MAC_VLANTR_VTHM_LEN         1
#define MAC_VLANTR_VTIM_POS         17
#define MAC_VLANTR_VTIM_LEN         1
#define MAC_VR_DEVID_POS            16
#define MAC_VR_DEVID_LEN            16
#define MAC_VR_SVER_POS             0
#define MAC_VR_SVER_LEN             8
#define MAC_VR_USERVER_POS          8
#define MAC_VR_USERVER_LEN          8

#define MAC_DBG_STA_TX_BUSY         0x70000
#define MTL_TXQ_DEG_TX_BUSY         0x10

#define MAC_MDIO_ADDRESS_BUSY       1 //bit 0

#define MAC_MDIO_ADDR_GOC_POS       2
#define MAC_MDIO_ADDR_GOC_LEN       2
#define MAC_MDIO_ADDR_GB_POS        0
#define MAC_MDIO_ADDR_GB_LEN        1

#define MAC_MDIO_DATA_RA_POS        16
#define MAC_MDIO_DATA_RA_LEN        16
#define MAC_MDIO_DATA_GD_POS        0
#define MAC_MDIO_DATA_GD_LEN        16

/* bit definitions for PMT and WOL, 20210622 */
#define MAC_PMT_STA_PWRDWN_POS      0
#define MAC_PMT_STA_PWRDWN_LEN      1
#define MAC_PMT_STA_MGKPKTEN_POS    1
#define MAC_PMT_STA_MGKPKTEN_LEN    1
#define MAC_PMT_STA_RWKPKTEN_POS    2
#define MAC_PMT_STA_RWKPKTEN_LEN    1
#define MAC_PMT_STA_MGKPRCVD_POS    5
#define MAC_PMT_STA_MGKPRCVD_LEN    1
#define MAC_PMT_STA_RWKPRCVD_POS    6
#define MAC_PMT_STA_RWKPRCVD_LEN    1
#define MAC_PMT_STA_GLBLUCAST_POS   9
#define MAC_PMT_STA_GLBLUCAST_LEN   1
#define MAC_PMT_STA_RWKPTR_POS      24
#define MAC_PMT_STA_RWKPTR_LEN      4
#define MAC_PMT_STA_RWKFILTERST_POS 31
#define MAC_PMT_STA_RWKFILTERST_LEN 1

/* MMC register offsets */
#define MMC_CR                              0x0700
#define MMC_RISR                            0x0704
#define MMC_TISR                            0x0708
#define MMC_RIER                            0x070c
#define MMC_TIER                            0x0710
#define MMC_TXOCTETCOUNT_GB_LO              0x0714
#define MMC_TXFRAMECOUNT_GB_LO              0x0718
#define MMC_TXBROADCASTFRAMES_G_LO          0x071c
#define MMC_TXMULTICASTFRAMES_G_LO          0x0720
#define MMC_TX64OCTETS_GB_LO                0x0724
#define MMC_TX65TO127OCTETS_GB_LO           0x0728
#define MMC_TX128TO255OCTETS_GB_LO          0x072c
#define MMC_TX256TO511OCTETS_GB_LO          0x0730
#define MMC_TX512TO1023OCTETS_GB_LO         0x0734
#define MMC_TX1024TOMAXOCTETS_GB_LO         0x0738
#define MMC_TXUNICASTFRAMES_GB_LO           0x073c
#define MMC_TXMULTICASTFRAMES_GB_LO         0x0740
#define MMC_TXBROADCASTFRAMES_GB_LO         0x0744
#define MMC_TXUNDERFLOWERROR_LO             0x0748
#define MMC_TXSINGLECOLLISION_G             0x074c
#define MMC_TXMULTIPLECOLLISION_G           0x0750
#define MMC_TXDEFERREDFRAMES                0x0754
#define MMC_TXLATECOLLISIONFRAMES           0x0758
#define MMC_TXEXCESSIVECOLLSIONFRAMES       0x075c
#define MMC_TXCARRIERERRORFRAMES            0x0760
#define MMC_TXOCTETCOUNT_G_LO               0x0764
#define MMC_TXFRAMECOUNT_G_LO               0x0768
#define MMC_TXEXCESSIVEDEFERRALERROR        0x076c
#define MMC_TXPAUSEFRAMES_LO                0x0770
#define MMC_TXVLANFRAMES_G_LO               0x0774
#define MMC_TXOVERSIZEFRAMES                0x0778
#define MMC_RXFRAMECOUNT_GB_LO              0x0780
#define MMC_RXOCTETCOUNT_GB_LO              0x0784
#define MMC_RXOCTETCOUNT_G_LO               0x0788
#define MMC_RXBROADCASTFRAMES_G_LO          0x078c
#define MMC_RXMULTICASTFRAMES_G_LO          0x0790
#define MMC_RXCRCERROR_LO                   0x0794
#define MMC_RXALIGNERROR                    0x0798
#define MMC_RXRUNTERROR                     0x079c
#define MMC_RXJABBERERROR                   0x07a0
#define MMC_RXUNDERSIZE_G                   0x07a4
#define MMC_RXOVERSIZE_G                    0x07a8
#define MMC_RX64OCTETS_GB_LO                0x07ac
#define MMC_RX65TO127OCTETS_GB_LO           0x07b0
#define MMC_RX128TO255OCTETS_GB_LO          0x07b4
#define MMC_RX256TO511OCTETS_GB_LO          0x07b8
#define MMC_RX512TO1023OCTETS_GB_LO         0x07bc
#define MMC_RX1024TOMAXOCTETS_GB_LO         0x07c0
#define MMC_RXUNICASTFRAMES_G_LO            0x07c4
#define MMC_RXLENGTHERROR_LO                0x07c8
#define MMC_RXOUTOFRANGETYPE_LO             0x07cc
#define MMC_RXPAUSEFRAMES_LO                0x07d0
#define MMC_RXFIFOOVERFLOW_LO               0x07d4
#define MMC_RXVLANFRAMES_GB_LO              0x07d8
#define MMC_RXWATCHDOGERROR                 0x07dc
#define MMC_RXRECEIVEERRORFRAME             0x07e0
#define MMC_RXCONTROLFRAME_G                0x07e4

#define MMC_IPCRXINTMASK                    0x800
#define MMC_IPCRXINT                        0x808

/* MMC register entry bit positions and sizes */
#define MMC_CR_CR_POS                           0
#define MMC_CR_CR_LEN                           1
#define MMC_CR_CSR_POS                          1
#define MMC_CR_CSR_LEN                          1
#define MMC_CR_ROR_POS                          2
#define MMC_CR_ROR_LEN                          1
#define MMC_CR_MCF_POS                          3
#define MMC_CR_MCF_LEN                          1
//#define MMC_CR_MCT_POS                        4
//#define MMC_CR_MCT_LEN                        2
#define MMC_RIER_ALL_INTERRUPTS_POS             0
#define MMC_RIER_ALL_INTERRUPTS_LEN             28
#define MMC_RISR_RXFRAMECOUNT_GB_POS            0
#define MMC_RISR_RXFRAMECOUNT_GB_LEN            1
#define MMC_RISR_RXOCTETCOUNT_GB_POS            1
#define MMC_RISR_RXOCTETCOUNT_GB_LEN            1
#define MMC_RISR_RXOCTETCOUNT_G_POS             2
#define MMC_RISR_RXOCTETCOUNT_G_LEN             1
#define MMC_RISR_RXBROADCASTFRAMES_G_POS        3
#define MMC_RISR_RXBROADCASTFRAMES_G_LEN        1
#define MMC_RISR_RXMULTICASTFRAMES_G_POS        4
#define MMC_RISR_RXMULTICASTFRAMES_G_LEN        1
#define MMC_RISR_RXCRCERROR_POS                 5
#define MMC_RISR_RXCRCERROR_LEN                 1
#define MMC_RISR_RXALIGNERROR_POS               6
#define MMC_RISR_RXALIGNERROR_LEN               1
#define MMC_RISR_RXRUNTERROR_POS                7
#define MMC_RISR_RXRUNTERROR_LEN                1
#define MMC_RISR_RXJABBERERROR_POS              8
#define MMC_RISR_RXJABBERERROR_LEN              1
#define MMC_RISR_RXUNDERSIZE_G_POS              9
#define MMC_RISR_RXUNDERSIZE_G_LEN              1
#define MMC_RISR_RXOVERSIZE_G_POS               10
#define MMC_RISR_RXOVERSIZE_G_LEN               1
#define MMC_RISR_RX64OCTETS_GB_POS              11
#define MMC_RISR_RX64OCTETS_GB_LEN              1
#define MMC_RISR_RX65TO127OCTETS_GB_POS         12
#define MMC_RISR_RX65TO127OCTETS_GB_LEN         1
#define MMC_RISR_RX128TO255OCTETS_GB_POS        13
#define MMC_RISR_RX128TO255OCTETS_GB_LEN        1
#define MMC_RISR_RX256TO511OCTETS_GB_POS        14
#define MMC_RISR_RX256TO511OCTETS_GB_LEN        1
#define MMC_RISR_RX512TO1023OCTETS_GB_POS       15
#define MMC_RISR_RX512TO1023OCTETS_GB_LEN       1
#define MMC_RISR_RX1024TOMAXOCTETS_GB_POS       16
#define MMC_RISR_RX1024TOMAXOCTETS_GB_LEN       1
#define MMC_RISR_RXUNICASTFRAMES_G_POS          17
#define MMC_RISR_RXUNICASTFRAMES_G_LEN          1
#define MMC_RISR_RXLENGTHERROR_POS              18
#define MMC_RISR_RXLENGTHERROR_LEN              1
#define MMC_RISR_RXOUTOFRANGETYPE_POS           19
#define MMC_RISR_RXOUTOFRANGETYPE_LEN           1
#define MMC_RISR_RXPAUSEFRAMES_POS              20
#define MMC_RISR_RXPAUSEFRAMES_LEN              1
#define MMC_RISR_RXFIFOOVERFLOW_POS             21
#define MMC_RISR_RXFIFOOVERFLOW_LEN             1
#define MMC_RISR_RXVLANFRAMES_GB_POS            22
#define MMC_RISR_RXVLANFRAMES_GB_LEN            1
#define MMC_RISR_RXWATCHDOGERROR_POS            23
#define MMC_RISR_RXWATCHDOGERROR_LEN            1
#define MMC_RISR_RXERRORFRAMES_POS              24
#define MMC_RISR_RXERRORFRAMES_LEN              1
#define MMC_RISR_RXERRORCONTROLFRAMES_POS       25
#define MMC_RISR_RXERRORCONTROLFRAMES_LEN       1
#define MMC_RISR_RXLPIMICROSECOND_POS           26 //no counter register
#define MMC_RISR_RXLPIMICROSECOND_LEN           1
#define MMC_RISR_RXLPITRANSITION_POS            27 //no counter register
#define MMC_RISR_RXLPITRANSITION_LEN            1

#define MMC_TIER_ALL_INTERRUPTS_POS             0
#define MMC_TIER_ALL_INTERRUPTS_LEN             28
#define MMC_TISR_TXOCTETCOUNT_GB_POS            0
#define MMC_TISR_TXOCTETCOUNT_GB_LEN            1
#define MMC_TISR_TXFRAMECOUNT_GB_POS            1
#define MMC_TISR_TXFRAMECOUNT_GB_LEN            1
#define MMC_TISR_TXBROADCASTFRAMES_G_POS        2
#define MMC_TISR_TXBROADCASTFRAMES_G_LEN        1
#define MMC_TISR_TXMULTICASTFRAMES_G_POS        3
#define MMC_TISR_TXMULTICASTFRAMES_G_LEN        1
#define MMC_TISR_TX64OCTETS_GB_POS              4
#define MMC_TISR_TX64OCTETS_GB_LEN              1
#define MMC_TISR_TX65TO127OCTETS_GB_POS         5
#define MMC_TISR_TX65TO127OCTETS_GB_LEN         1
#define MMC_TISR_TX128TO255OCTETS_GB_POS        6
#define MMC_TISR_TX128TO255OCTETS_GB_LEN        1
#define MMC_TISR_TX256TO511OCTETS_GB_POS        7
#define MMC_TISR_TX256TO511OCTETS_GB_LEN        1
#define MMC_TISR_TX512TO1023OCTETS_GB_POS       8
#define MMC_TISR_TX512TO1023OCTETS_GB_LEN       1
#define MMC_TISR_TX1024TOMAXOCTETS_GB_POS       9
#define MMC_TISR_TX1024TOMAXOCTETS_GB_LEN       1
#define MMC_TISR_TXUNICASTFRAMES_GB_POS         10
#define MMC_TISR_TXUNICASTFRAMES_GB_LEN         1
#define MMC_TISR_TXMULTICASTFRAMES_GB_POS       11
#define MMC_TISR_TXMULTICASTFRAMES_GB_LEN       1
#define MMC_TISR_TXBROADCASTFRAMES_GB_POS       12
#define MMC_TISR_TXBROADCASTFRAMES_GB_LEN       1
#define MMC_TISR_TXUNDERFLOWERROR_POS           13
#define MMC_TISR_TXUNDERFLOWERROR_LEN           1
#define MMC_TISR_TXSINGLECOLLISION_G_POS        14
#define MMC_TISR_TXSINGLECOLLISION_G_LEN        1
#define MMC_TISR_TXMULTIPLECOLLISION_G_POS      15
#define MMC_TISR_TXMULTIPLECOLLISION_G_LEN      1
#define MMC_TISR_TXDEFERREDFRAMES_POS           16
#define MMC_TISR_TXDEFERREDFRAMES_LEN           1
#define MMC_TISR_TXLATECOLLISIONFRAMES_POS      17
#define MMC_TISR_TXLATECOLLISIONFRAMES_LEN      1
#define MMC_TISR_TXEXCESSIVECOLLISIONFRAMES_POS 18
#define MMC_TISR_TXEXCESSIVECOLLISIONFRAMES_LEN 1
#define MMC_TISR_TXCARRIERERRORFRAMES_POS       19
#define MMC_TISR_TXCARRIERERRORFRAMES_LEN       1
#define MMC_TISR_TXOCTETCOUNT_G_POS             20
#define MMC_TISR_TXOCTETCOUNT_G_LEN             1
#define MMC_TISR_TXFRAMECOUNT_G_POS             21
#define MMC_TISR_TXFRAMECOUNT_G_LEN             1
#define MMC_TISR_TXEXCESSIVEDEFERRALFRAMES_POS  22
#define MMC_TISR_TXEXCESSIVEDEFERRALFRAMES_LEN  1
#define MMC_TISR_TXPAUSEFRAMES_POS              23
#define MMC_TISR_TXPAUSEFRAMES_LEN              1
#define MMC_TISR_TXVLANFRAMES_G_POS             24
#define MMC_TISR_TXVLANFRAMES_G_LEN             1
#define MMC_TISR_TXOVERSIZE_G_POS               25
#define MMC_TISR_TXOVERSIZE_G_LEN               1
#define MMC_TISR_TXLPIMICROSECOND_POS           26 //no counter register
#define MMC_TISR_TXLPIMICROSECOND_LEN           1
#define MMC_TISR_TXLPITRANSITION_POS            27 //no counter register
#define MMC_TISR_TXLPITRANSITION_LEN            1

/* MTL register offsets */
#define MTL_OMR                                  0x0c00
#define MTL_FDCR                                 0x0c08
#define MTL_FDSR                                 0x0c0c
#define MTL_FDDR                                 0x0c10
#define MTL_INT_SR                               0x0c20
#define MTL_RQDCM0R                              0x0c30
#define MTL_ECC_INT_SR                           0x0ccc

#define MTL_RQDCM_INC                             4
#define MTL_RQDCM_Q_PER_REG                       4

/* MTL register entry bit positions and sizes */
#define MTL_OMR_ETSALG_POS                        5
#define MTL_OMR_ETSALG_LEN                        2
#define MTL_OMR_RAA_POS                           2
#define MTL_OMR_RAA_LEN                           1

/* MTL queue register offsets
 *   Multiple queues can be active.  The first queue has registers
 *   that begin at 0x0d00.  Each subsequent queue has registers that
 *   are accessed using an offset of 0x40 from the previous queue.
 */
#define MTL_Q_BASE          0x0d00
#define MTL_Q_INC           0x40
#define MTL_Q_INT_CTL_SR    0x0d2c

#define MTL_Q_TQOMR         0x00
#define MTL_Q_TQUR          0x04
#define MTL_Q_RQOMR         0x30
#define MTL_Q_RQMPOCR       0x34
#define MTL_Q_RQDR          0x38
#define MTL_Q_RQCR          0x3c
#define MTL_Q_IER           0x2c
#define MTL_Q_ISR           0x2c //no isr register
#define MTL_TXQ_DEG         0x08 //transmit  debug

/* MTL queue register entry bit positions and sizes */
#define MTL_Q_RQDR_PRXQ_POS     16
#define MTL_Q_RQDR_PRXQ_LEN     14
#define MTL_Q_RQDR_RXQSTS_POS   4
#define MTL_Q_RQDR_RXQSTS_LEN   2
#define MTL_Q_RQOMR_RFA_POS     8
#define MTL_Q_RQOMR_RFA_LEN     6
#define MTL_Q_RQOMR_RFD_POS     14
#define MTL_Q_RQOMR_RFD_LEN     6
#define MTL_Q_RQOMR_EHFC_POS    7
#define MTL_Q_RQOMR_EHFC_LEN    1
#define MTL_Q_RQOMR_RQS_POS     20
#define MTL_Q_RQOMR_RQS_LEN     9
#define MTL_Q_RQOMR_RSF_POS     5
#define MTL_Q_RQOMR_RSF_LEN     1
#define MTL_Q_RQOMR_FEP_POS     4
#define MTL_Q_RQOMR_FEP_LEN     1
#define MTL_Q_RQOMR_FUP_POS     3
#define MTL_Q_RQOMR_FUP_LEN     1
#define MTL_Q_RQOMR_RTC_POS     0
#define MTL_Q_RQOMR_RTC_LEN     2
#define MTL_Q_TQOMR_FTQ_POS     0
#define MTL_Q_TQOMR_FTQ_LEN     1
//#define MTL_Q_TQOMR_Q2TCMAP_POS   8 // no register
//#define MTL_Q_TQOMR_Q2TCMAP_LEN   3 // no register
#define MTL_Q_TQOMR_TQS_POS     16
#define MTL_Q_TQOMR_TQS_LEN     7
#define MTL_Q_TQOMR_TSF_POS     1
#define MTL_Q_TQOMR_TSF_LEN     1
#define MTL_Q_TQOMR_TTC_POS     4
#define MTL_Q_TQOMR_TTC_LEN     3
#define MTL_Q_TQOMR_TXQEN_POS   2
#define MTL_Q_TQOMR_TXQEN_LEN   2

/* MTL queue register value */
#define MTL_RSF_DISABLE         0x00
#define MTL_RSF_ENABLE          0x01
#define MTL_TSF_DISABLE         0x00
#define MTL_TSF_ENABLE          0x01
#define MTL_FEP_DISABLE         0x00
#define MTL_FEP_ENABLE          0x01

#define MTL_RX_THRESHOLD_64     0x00
#define MTL_RX_THRESHOLD_32     0x01
#define MTL_RX_THRESHOLD_96     0x02
#define MTL_RX_THRESHOLD_128    0x03
#define MTL_TX_THRESHOLD_32     0x00
#define MTL_TX_THRESHOLD_64     0x01
#define MTL_TX_THRESHOLD_96     0x02
#define MTL_TX_THRESHOLD_128    0x03
#define MTL_TX_THRESHOLD_192    0x04
#define MTL_TX_THRESHOLD_256    0x05
#define MTL_TX_THRESHOLD_384    0x06
#define MTL_TX_THRESHOLD_512    0x07

#define MTL_ETSALG_WRR          0x00
#define MTL_ETSALG_WFQ          0x01
#define MTL_ETSALG_DWRR         0x02
#define MTL_ETSALG_SP           0x03

#define MTL_RAA_SP              0x00
#define MTL_RAA_WSP             0x01

#define MTL_Q_DISABLED          0x00
#define MTL_Q_EN_IF_AV          0x01
#define MTL_Q_ENABLED           0x02

#define MTL_RQDCM0R_Q0MDMACH        0x0
#define MTL_RQDCM0R_Q1MDMACH        0x00000100
#define MTL_RQDCM0R_Q2MDMACH        0x00020000
#define MTL_RQDCM0R_Q3MDMACH        0x03000000
#define MTL_RQDCM1R_Q4MDMACH        0x00000004
#define MTL_RQDCM1R_Q5MDMACH        0x00000500
#define MTL_RQDCM1R_Q6MDMACH        0x00060000
#define MTL_RQDCM1R_Q7MDMACH        0x07000000
#define MTL_RQDCM2R_Q8MDMACH        0x00000008
#define MTL_RQDCM2R_Q9MDMACH        0x00000900
#define MTL_RQDCM2R_Q10MDMACH       0x000A0000
#define MTL_RQDCM2R_Q11MDMACH       0x0B000000

#define MTL_RQDCM0R_Q0DDMACH        0x10
#define MTL_RQDCM0R_Q1DDMACH        0x00001000
#define MTL_RQDCM0R_Q2DDMACH        0x00100000
#define MTL_RQDCM0R_Q3DDMACH        0x10000000
#define MTL_RQDCM1R_Q4DDMACH        0x00000010
#define MTL_RQDCM1R_Q5DDMACH        0x00001000
#define MTL_RQDCM1R_Q6DDMACH        0x00100000
#define MTL_RQDCM1R_Q7DDMACH        0x10000000


/* MTL traffic class register offsets
 *   Multiple traffic classes can be active.  The first class has registers
 *   that begin at 0x1100.  Each subsequent queue has registers that
 *   are accessed using an offset of 0x80 from the previous queue.
 */
/* NO TRAFFIC CLASS REGISTER DESCRIPTION */
#if 1
#define MTL_TC_BASE         MTL_Q_BASE
#define MTL_TC_INC          MTL_Q_INC

#define MTL_TC_TQDR         0x08
#define MTL_TC_ETSCR        0x10
#define MTL_TC_ETSSR        0x14
#define MTL_TC_QWR          0x18

/* The Queue 0 Transmit Debug register gives the debug status of various blocks
 * related to the Transmit queue
 */
#define MTL_TC_TQDR_TRCSTS_POS      1
#define MTL_TC_TQDR_TRCSTS_LEN      2
#define MTL_TC_TQDR_TXQSTS_POS      4
#define MTL_TC_TQDR_TXQSTS_LEN      1

/* MTL traffic class register entry bit positions and sizes */
#define MTL_TC_ETSCR_TSA_POS        0
#define MTL_TC_ETSCR_TSA_LEN        2
#define MTL_TC_QWR_QW_POS           0
#define MTL_TC_QWR_QW_LEN           21

/* MTL traffic class register value */
#define MTL_TSA_SP          0x00
#define MTL_TSA_ETS         0x02
#endif

/* DMA register offsets */
#define DMA_MR              0x1000
#define DMA_SBMR            0x1004
#define DMA_ISR             0x1008
#define DMA_DSR0            0x100c
#define DMA_DSR1            0x1010
#define DMA_DSR2            0x1014
#define DMA_AXIARCR         0x1020
#define DMA_AXIAWCR         0x1024
#define DMA_AXIAWRCR        0x1028
#define DMA_SAFE_ISR        0x1080
#define DMA_ECC_IE          0x1084
#define DMA_ECC_INT_SR      0x1088

/* DMA register entry bit positions and sizes */
#define DMA_ISR_MACIS_POS       17
#define DMA_ISR_MACIS_LEN       1
#define DMA_ISR_MTLIS_POS       16
#define DMA_ISR_MTLIS_LEN       1
#define DMA_MR_SWR_POS          0
#define DMA_MR_SWR_LEN          1
#define DMA_MR_TXPR_POS         11
#define DMA_MR_TXPR_LEN         1
#define DMA_MR_INTM_POS         16
#define DMA_MR_INTM_LEN         2
#define DMA_MA_INTM_EDGE             0
#define DMA_MA_INTM_LEVEL            1
#define DMA_MA_INTM_LEVLE_ENHANCE    2
#define DMA_MR_QUREAD_POS       19
#define DMA_MR_QUREAD_LEN       1
#define DMA_MR_QUREAD_EN        1
#define DMA_MR_TNDF_POS         20
#define DMA_MR_TNDF_LEN         2
#define DMA_MR_RNDF_POS         22
#define DMA_MR_RNDF_LEN         2

#define DMA_SBMR_EN_LPI_POS         31
#define DMA_SBMR_EN_LPI_LEN         1
#define DMA_SBMR_LPI_XIT_PKT_POS    30
#define DMA_SBMR_LPI_XIT_PKT_LEN    1
#define DMA_SBMR_WR_OSR_LMT_POS     24
#define DMA_SBMR_WR_OSR_LMT_LEN     6
#define DMA_SBMR_RD_OSR_LMT_POS     16
#define DMA_SBMR_RD_OSR_LMT_LEN     8
#define DMA_SBMR_AAL_POS            12
#define DMA_SBMR_AAL_LEN            1
#define DMA_SBMR_EAME_POS           11
#define DMA_SBMR_EAME_LEN           1
#define DMA_SBMR_AALE_POS           10
#define DMA_SBMR_AALE_LEN           1
#define DMA_SBMR_BLEN_4_POS         1
#define DMA_SBMR_BLEN_4_LEN         1
#define DMA_SBMR_BLEN_8_POS         2
#define DMA_SBMR_BLEN_8_LEN         1
#define DMA_SBMR_BLEN_16_POS        3
#define DMA_SBMR_BLEN_16_LEN        1
#define DMA_SBMR_BLEN_32_POS        4
#define DMA_SBMR_BLEN_32_LEN        1
#define DMA_SBMR_BLEN_64_POS        5
#define DMA_SBMR_BLEN_64_LEN        1
#define DMA_SBMR_BLEN_128_POS       6
#define DMA_SBMR_BLEN_128_LEN       1
#define DMA_SBMR_BLEN_256_POS       7
#define DMA_SBMR_BLEN_256_LEN       1
#define DMA_SBMR_FB_POS             0
#define DMA_SBMR_FB_LEN             1

/* DMA register values */
#define DMA_DSR_RPS_LEN         4
#define DMA_DSR_TPS_LEN         4
#define DMA_DSR_Q_LEN           (DMA_DSR_RPS_LEN + DMA_DSR_TPS_LEN)
#define DMA_DSR0_TPS_START      12
#define DMA_DSRX_FIRST_QUEUE    3
#define DMA_DSRX_INC            4
#define DMA_DSRX_QPR            4 // no definition
#define DMA_DSRX_TPS_START      4
#define DMA_TPS_STOPPED         0x00
#define DMA_TPS_SUSPENDED       0x06

/* DMA channel register offsets
 *   Multiple channels can be active.  The first channel has registers
 *   that begin at 0x1100.  Each subsequent channel has registers that
 *   are accessed using an offset of 0x80 from the previous channel.
 */
#define DMA_CH_BASE             0x1100
#define DMA_CH_INC              0x80

#define DMA_CH_CR               0x00
#define DMA_CH_TCR              0x04
#define DMA_CH_RCR              0x08
#define DMA_CH_TDLR_HI          0x10
#define DMA_CH_TDLR_LO          0x14
#define DMA_CH_RDLR_HI          0x18
#define DMA_CH_RDLR_LO          0x1c
#define DMA_CH_TDTR_LO          0x20
#define DMA_CH_RDTR_LO          0x28
#define DMA_CH_TDRLR            0x2c
#define DMA_CH_RDRLR            0x30
#define DMA_CH_IER              0x34
#define DMA_CH_RIWT             0x38
#define DMA_CH_CATDR_LO         0x44
#define DMA_CH_CARDR_LO         0x4c
#define DMA_CH_CATBR_HI         0x50
#define DMA_CH_CATBR_LO         0x54
#define DMA_CH_CARBR_HI         0x58
#define DMA_CH_CARBR_LO         0x5c
#define DMA_CH_SR               0x60

/* DMA channel register entry bit positions and sizes */
#define DMA_CH_CR_PBLX8_POS     16
#define DMA_CH_CR_PBLX8_LEN     1
#define DMA_CH_CR_SPH_POS       24
#define DMA_CH_CR_SPH_LEN       1
#define DMA_CH_IER_AIE_POS      14
#define DMA_CH_IER_AIE_LEN      1
#define DMA_CH_IER_FBEE_POS     12
#define DMA_CH_IER_FBEE_LEN     1
#define DMA_CH_IER_NIE_POS      15
#define DMA_CH_IER_NIE_LEN      1
#define DMA_CH_IER_RBUE_POS     7
#define DMA_CH_IER_RBUE_LEN     1
#define DMA_CH_IER_RIE_POS      6
#define DMA_CH_IER_RIE_LEN      1
#define DMA_CH_IER_RSE_POS      8
#define DMA_CH_IER_RSE_LEN      1
#define DMA_CH_IER_TBUE_POS     2
#define DMA_CH_IER_TBUE_LEN     1
#define DMA_CH_IER_TIE_POS      0
#define DMA_CH_IER_TIE_LEN      1
#define DMA_CH_IER_TXSE_POS     1
#define DMA_CH_IER_TXSE_LEN     1
#define DMA_CH_RCR_PBL_POS      16
#define DMA_CH_RCR_PBL_LEN      6
#define DMA_CH_RCR_RBSZ_POS     1
#define DMA_CH_RCR_RBSZ_LEN     14
#define DMA_CH_RCR_SR_POS       0
#define DMA_CH_RCR_SR_LEN       1
#define DMA_CH_RIWT_RWT_POS     0
#define DMA_CH_RIWT_RWT_LEN     8
#define DMA_CH_SR_FBE_POS       12
#define DMA_CH_SR_FBE_LEN       1
#define DMA_CH_SR_RBU_POS       7
#define DMA_CH_SR_RBU_LEN       1
#define DMA_CH_SR_RI_POS        6
#define DMA_CH_SR_RI_LEN        1
#define DMA_CH_SR_RPS_POS       8
#define DMA_CH_SR_RPS_LEN       1
#define DMA_CH_SR_TBU_POS       2
#define DMA_CH_SR_TBU_LEN       1
#define DMA_CH_SR_TI_POS        0
#define DMA_CH_SR_TI_LEN        1
#define DMA_CH_SR_TPS_POS       1
#define DMA_CH_SR_TPS_LEN       1
#define DMA_CH_TCR_OSP_POS      4
#define DMA_CH_TCR_OSP_LEN      1
#define DMA_CH_TCR_PBL_POS      16
#define DMA_CH_TCR_PBL_LEN      6
#define DMA_CH_TCR_ST_POS       0
#define DMA_CH_TCR_ST_LEN       1
#define DMA_CH_TCR_TSE_POS      12
#define DMA_CH_TCR_TSE_LEN      1

/* DMA channel register values */
#define DMA_OSP_DISABLE     0x00
#define DMA_OSP_ENABLE      0x01
#define DMA_PBL_1           1
#define DMA_PBL_2           2
#define DMA_PBL_4           4
#define DMA_PBL_8           8
#define DMA_PBL_16          16
#define DMA_PBL_32          32
#define DMA_PBL_64          64
#define DMA_PBL_128         128
#define DMA_PBL_256         256
#define DMA_PBL_X8_DISABLE  0x00
#define DMA_PBL_X8_ENABLE   0x01

/* Descriptor/Packet entry bit positions and sizes */
#define RX_PACKET_ERRORS_CRC_POS        2
#define RX_PACKET_ERRORS_CRC_LEN        1
#define RX_PACKET_ERRORS_FRAME_POS      3
#define RX_PACKET_ERRORS_FRAME_LEN      1
#define RX_PACKET_ERRORS_LENGTH_POS     0
#define RX_PACKET_ERRORS_LENGTH_LEN     1
#define RX_PACKET_ERRORS_OVERRUN_POS    1
#define RX_PACKET_ERRORS_OVERRUN_LEN    1

#define RX_PACKET_ATTRIBUTES_CSUM_DONE_POS      0
#define RX_PACKET_ATTRIBUTES_CSUM_DONE_LEN      1
#define RX_PACKET_ATTRIBUTES_VLAN_CTAG_POS      1
#define RX_PACKET_ATTRIBUTES_VLAN_CTAG_LEN      1
#define RX_PACKET_ATTRIBUTES_INCOMPLETE_POS     2
#define RX_PACKET_ATTRIBUTES_INCOMPLETE_LEN     1
#define RX_PACKET_ATTRIBUTES_CONTEXT_NEXT_POS   3
#define RX_PACKET_ATTRIBUTES_CONTEXT_NEXT_LEN   1
#define RX_PACKET_ATTRIBUTES_CONTEXT_POS        4
#define RX_PACKET_ATTRIBUTES_CONTEXT_LEN        1
#define RX_PACKET_ATTRIBUTES_RX_TSTAMP_POS      5
#define RX_PACKET_ATTRIBUTES_RX_TSTAMP_LEN      1
#define RX_PACKET_ATTRIBUTES_RSS_HASH_POS       6
#define RX_PACKET_ATTRIBUTES_RSS_HASH_LEN       1

#define RX_NORMAL_DESC0_OVT_POS         0
#define RX_NORMAL_DESC0_OVT_LEN         16
#define RX_NORMAL_DESC2_HL_POS          0
#define RX_NORMAL_DESC2_HL_LEN          10
//#define RX_NORMAL_DESC3_CDA_POS           27//
#define RX_NORMAL_DESC3_CDA_LEN         1
#define RX_NORMAL_DESC3_CTXT_POS        30
#define RX_NORMAL_DESC3_CTXT_LEN        1
#define RX_NORMAL_DESC3_ES_POS          15
#define RX_NORMAL_DESC3_ES_LEN          1
#define RX_NORMAL_DESC3_ETLT_POS        16
#define RX_NORMAL_DESC3_ETLT_LEN        3
#define RX_NORMAL_DESC3_FD_POS          29
#define RX_NORMAL_DESC3_FD_LEN          1
#define RX_NORMAL_DESC3_INTE_POS        30
#define RX_NORMAL_DESC3_INTE_LEN        1
//#define RX_NORMAL_DESC3_L34T_POS      20//
#define RX_NORMAL_DESC3_L34T_LEN        4
#define RX_NORMAL_DESC3_RSV_POS         26
#define RX_NORMAL_DESC3_RSV_LEN         1
#define RX_NORMAL_DESC3_LD_POS          28
#define RX_NORMAL_DESC3_LD_LEN          1
#define RX_NORMAL_DESC3_OWN_POS         31
#define RX_NORMAL_DESC3_OWN_LEN         1
#define RX_NORMAL_DESC3_BUF2V_POS       25
#define RX_NORMAL_DESC3_BUF2V_LEN       1
#define RX_NORMAL_DESC3_BUF1V_POS       24
#define RX_NORMAL_DESC3_BUF1V_LEN       1
#define RX_NORMAL_DESC3_PL_POS          0
#define RX_NORMAL_DESC3_PL_LEN          15

#define RX_NORMAL_DESC0_WB_IVT_POS          16 // Inner VLAN Tag. Valid only when Double VLAN tag processing and VLAN tag stripping are enabled.
#define RX_NORMAL_DESC0_WB_IVT_LEN          16
#define RX_NORMAL_DESC0_WB_OVT_POS          0  // Outer VLAN Tag.
#define RX_NORMAL_DESC0_WB_OVT_LEN          16
#define RX_NORMAL_DESC0_WB_OVT_VLANID_POS   0  // Outer VLAN ID.
#define RX_NORMAL_DESC0_WB_OVT_VLANID_LEN   12
#define RX_NORMAL_DESC0_WB_OVT_CFI_POS      12  // Outer VLAN CFI.
#define RX_NORMAL_DESC0_WB_OVT_CFI_LEN      1
#define RX_NORMAL_DESC0_WB_OVT_PRIO_POS     13  // Outer VLAN Priority.
#define RX_NORMAL_DESC0_WB_OVT_PRIO_LEN     3

#define RX_NORMAL_DESC1_WB_IPCE_POS         7  // IP Payload Error.
#define RX_NORMAL_DESC1_WB_IPCE_LEN         1
#define RX_NORMAL_DESC1_WB_IPV6_POS         5  // IPV6 Header Present.
#define RX_NORMAL_DESC1_WB_IPV6_LEN         1
#define RX_NORMAL_DESC1_WB_IPV4_POS         4  // IPV4 Header Present.
#define RX_NORMAL_DESC1_WB_IPV4_LEN         1
#define RX_NORMAL_DESC1_WB_IPHE_POS         3  // IP Header Error.
#define RX_NORMAL_DESC1_WB_IPHE_LEN         1
#define RX_NORMAL_DESC1_WB_PT_POS           0 //
#define RX_NORMAL_DESC1_WB_PT_LEN           3

#define RX_NORMAL_DESC2_WB_HF_POS           18 // Hash Filter Status. When this bit is set, it indicates that the packet passed the MAC address hash filter
#define RX_NORMAL_DESC2_WB_HF_LEN           1

/* Destination Address Filter Fail. When Flexible RX Parser is disabled,
 * and this bit is set, it indicates that the packet failed
 * the DA Filter in the MAC.
 */
#define RX_NORMAL_DESC2_WB_DAF_POS           17
#define RX_NORMAL_DESC2_WB_DAF_LEN           1
#define RX_NORMAL_DESC2_WB_RAPARSER_POS      11
#define RX_NORMAL_DESC2_WB_RAPARSER_LEN      3

#define RX_NORMAL_DESC3_WB_LD_POS                28
#define RX_NORMAL_DESC3_WB_LD_LEN                1
#define RX_NORMAL_DESC3_WB_RS0V_POS              25 // When this bit is set, it indicates that the status in RDES0 is valid and it is written by the DMA.
#define RX_NORMAL_DESC3_WB_RS0V_LEN              1

/* When this bit is set, it indicates that a Cyclic Redundancy Check (CRC)
 * Error occurred on the received packet.This field is valid only when the
 * LD bit of RDES3 is set.
 */
#define RX_NORMAL_DESC3_WB_CE_POS                24
#define RX_NORMAL_DESC3_WB_CE_LEN                1

/*
 * When this bit is set, it indicates that the packet length exceeds the specified maximum
 * Ethernet size of 1518, 1522, or 2000 bytes (9018 or 9022 bytes if jumbo packet enable is set).
 * Note: Giant packet indicates only the packet length. It does not cause any packet truncation.
 */
#define RX_NORMAL_DESC3_WB_GP_POS                23
#define RX_NORMAL_DESC3_WB_GP_LEN                1

/*
 * When this bit is set, it indicates that the Receive Watchdog Timer has expired while receiving
 * the current packet. The current packet is truncated after watchdog timeout.
 */
#define RX_NORMAL_DESC3_WB_RWT_POS                22
#define RX_NORMAL_DESC3_WB_RWT_LEN                1

/*
 * When this bit is set, it indicates that the received packet is damaged because of buffer
 * overflow in Rx FIFO.
 * Note: This bit is set only when the DMA transfers a partial packet to the application. This
 * happens only when the Rx FIFO is operating in the threshold mode. In the store-and-forward
 * mode, all partial packets are dropped completely in Rx FIFO.
 */
#define RX_NORMAL_DESC3_WB_OE_POS                21
#define RX_NORMAL_DESC3_WB_OE_LEN                1

/*
 * When this bit is set, it indicates that the gmii_rxer_i signal is asserted while the gmii_rxdv_i
 * signal is asserted during packet reception. This error also includes carrier extension error in
 * the GMII and half-duplex mode. Error can be of less or no extension, or error (rxd!= 0f) during
 * extension
 */
#define RX_NORMAL_DESC3_WB_RE_POS                20
#define RX_NORMAL_DESC3_WB_RE_LEN                1

/*
 * When this bit is set, it indicates that the received packet has a non-integer multiple of bytes
 * (odd nibbles). This bit is valid only in the MII Mode
 */
#define RX_NORMAL_DESC3_WB_DE_POS                19
#define RX_NORMAL_DESC3_WB_DE_LEN                1

/*
 * When this bit is set, it indicates the logical OR of the following bits:
 *  RDES3[24]: CRC Error
 *  RDES3[19]: Dribble Error
 *  RDES3[20]: Receive Error
 *  RDES3[22]: Watchdog Timeout
 *  RDES3[21]: Overflow Error
 *  RDES3[23]: Giant Packet
 *  RDES2[17]: Destination Address Filter Fail, when Flexible RX Parser is enabled
 *  RDES2[16]: SA Address Filter Fail, when Flexible RX Parser is enabled
 * This field is valid only when the LD bit of RDES3 is set
 */
#define RX_NORMAL_DESC3_WB_ES_POS                15
#define RX_NORMAL_DESC3_WB_ES_LEN                1

#define RX_DESC3_L34T_IPV4_TCP                   1
#define RX_DESC3_L34T_IPV4_UDP                   2
#define RX_DESC3_L34T_IPV4_ICMP                  3
#define RX_DESC3_L34T_IPV6_TCP                   9
#define RX_DESC3_L34T_IPV6_UDP                   10
#define RX_DESC3_L34T_IPV6_ICMP                  11

#define RX_DESC1_PT_UDP                          1
#define RX_DESC1_PT_TCP                          2
#define RX_DESC1_PT_ICMP                         3
#define RX_DESC1_PT_AV_TAG_DATA                  6
#define RX_DESC1_PT_AV_TAG_CTRL                  7
#define RX_DESC1_PT_AV_NOTAG_CTRL                5


//#define RX_CONTEXT_DESC3_TSA_POS              4//
#define RX_CONTEXT_DESC3_TSA_LEN                1
//#define RX_CONTEXT_DESC3_TSD_POS              6//
#define RX_CONTEXT_DESC3_TSD_LEN                1

#define TX_PACKET_ATTRIBUTES_CSUM_ENABLE_POS    0
#define TX_PACKET_ATTRIBUTES_CSUM_ENABLE_LEN    1
#define TX_PACKET_ATTRIBUTES_TSO_ENABLE_POS     1
#define TX_PACKET_ATTRIBUTES_TSO_ENABLE_LEN     1
#define TX_PACKET_ATTRIBUTES_VLAN_CTAG_POS      2
#define TX_PACKET_ATTRIBUTES_VLAN_CTAG_LEN      1
#define TX_PACKET_ATTRIBUTES_PTP_POS            3
#define TX_PACKET_ATTRIBUTES_PTP_LEN            1

#define TX_CONTEXT_DESC2_MSS_POS                0
#define TX_CONTEXT_DESC2_MSS_LEN                14
#define TX_CONTEXT_DESC2_IVLTV_POS              16  // Inner VLAN Tag.
#define TX_CONTEXT_DESC2_IVLTV_LEN              16

#define TX_CONTEXT_DESC3_CTXT_POS               30
#define TX_CONTEXT_DESC3_CTXT_LEN               1
#define TX_CONTEXT_DESC3_TCMSSV_POS             26
#define TX_CONTEXT_DESC3_TCMSSV_LEN             1
#define TX_CONTEXT_DESC3_IVTIR_POS              18
#define TX_CONTEXT_DESC3_IVTIR_LEN              2
#define TX_CONTEXT_DESC3_IVTIR_INSERT           2   // Insert an inner VLAN tag with the tag value programmed in the MAC_Inner_VLAN_Incl register or context descriptor.
#define TX_CONTEXT_DESC3_IVLTV_POS              17  // Indicates that the Inner VLAN TAG, IVLTV field of context TDES2 is valid.
#define TX_CONTEXT_DESC3_IVLTV_LEN              1
#define TX_CONTEXT_DESC3_VLTV_POS               16  // Indicates that the VT field of context TDES3 is valid.
#define TX_CONTEXT_DESC3_VLTV_LEN               1
#define TX_CONTEXT_DESC3_VT_POS                 0
#define TX_CONTEXT_DESC3_VT_LEN                 16

#define TX_NORMAL_DESC2_HL_B1L_POS              0   // Header Length or Buffer 1 Length.
#define TX_NORMAL_DESC2_HL_B1L_LEN              14
#define TX_NORMAL_DESC2_IC_POS                  31  // Interrupt on Completion.
#define TX_NORMAL_DESC2_IC_LEN                  1
#define TX_NORMAL_DESC2_TTSE_POS                30  // Transmit Timestamp Enable or External TSO Memory Write Enable.
#define TX_NORMAL_DESC2_TTSE_LEN                1
#define TX_NORMAL_DESC2_VTIR_POS                14  //LAN Tag Insertion or Replacement.
#define TX_NORMAL_DESC2_VTIR_LEN                2
#define TX_NORMAL_DESC2_VLAN_INSERT             0x2

#define TX_NORMAL_DESC3_TCPPL_POS               0
#define TX_NORMAL_DESC3_TCPPL_LEN               18
#define TX_NORMAL_DESC3_FL_POS                  0   // Frame Length or TCP Payload Length.
#define TX_NORMAL_DESC3_FL_LEN                  15
#define TX_NORMAL_DESC3_CIC_POS                 16  /* Checksum Insertion Control or TCP Payload Length.
                                                        2'b00: Checksum Insertion Disabled.
                                                        2'b01: Only IP header checksum calculation and insertion are enabled.
                                                        2'b10: IP header checksum and payload checksum calculation and insertion are
                                                            enabled, but pseudo-header checksum is not calculated in hardware.
                                                        2'b11: IP Header checksum and payload checksum calculation and insertion are
                                                        enabled, and pseudo - header checksum is calculated in hardware. */
#define TX_NORMAL_DESC3_CIC_LEN                 2
#define TX_NORMAL_DESC3_TSE_POS                 18  // TCP Segmentation Enable.
#define TX_NORMAL_DESC3_TSE_LEN                 1
#define TX_NORMAL_DESC3_TCPHDRLEN_POS           19  /* THL: TCP/UDP Header Length.If the TSE bit is set, this field contains
                                                       the length of the TCP / UDP header.The minimum value of this field must
                                                       be 5 for TCP header.The value must be equal to 2 for UDP header. This
                                                       field is valid only for the first descriptor.*/
#define TX_NORMAL_DESC3_TCPHDRLEN_LEN           4
#define TX_NORMAL_DESC3_CPC_POS                 26  // CRC Pad Control.
#define TX_NORMAL_DESC3_CPC_LEN                 2
#define TX_NORMAL_DESC3_LD_POS                  28  // Last Descriptor.
#define TX_NORMAL_DESC3_LD_LEN                  1
#define TX_NORMAL_DESC3_FD_POS                  29  // First Descriptor.
#define TX_NORMAL_DESC3_FD_LEN                  1
#define TX_NORMAL_DESC3_CTXT_POS                30  // Context Type.This bit should be set to 1'b0 for normal descriptor.
#define TX_NORMAL_DESC3_CTXT_LEN                1
#define TX_NORMAL_DESC3_OWN_POS                 31  // Own Bit.
#define TX_NORMAL_DESC3_OWN_LEN                 1

/* for ephy generic register definitions */
#define FXGMAC_EPHY_REGS_LEN               32  //32 ethernet phy registers under spec

#define REG_MII_BMCR                       0x00    /* Basic mode control register */
#define PHY_CR_RESET_POS                   15
#define PHY_CR_RESET_LEN                   1
#define PHY_CR_SPEED_SEL_H_POS             6
#define PHY_CR_SPEED_SEL_H_LEN             1
#define PHY_CR_SPEED_SEL_L_POS             13
#define PHY_CR_SPEED_SEL_L_LEN             1
#define PHY_CR_AUTOENG_POS                 12
#define PHY_CR_AUTOENG_LEN                 1
#define PHY_CR_POWER_POS                   11
#define PHY_CR_POWER_LEN                   1
#define PHY_CR_RE_AUTOENG_POS              9
#define PHY_CR_RE_AUTOENG_LEN              1
#define PHY_CR_DUPLEX_POS                  8
#define PHY_CR_DUPLEX_LEN                  1
#define REG_MII_BMCR_ENABLE_LOOPBACK      0x8140
#define REG_MII_BMCR_DISABLE_LOOPBACK     0x9140
#define REG_MII_BMSR                      0x01    /* Basic mode status register  */
#define REG_MII_PHYSID1                   0x02    /* PHYS ID 1                   */
#define REG_MII_PHYSID2                   0x03    /* PHYS ID 2                   */
#define REG_MII_ADVERTISE                 0x04    /* Advertisement control reg   */
#define PHY_MII_ADVERTISE_ASYPAUSE_POS    11
#define PHY_MII_ADVERTISE_ASYPAUSE_LEN    1
#define PHY_MII_ADVERTISE_PAUSE_POS       10
#define PHY_MII_ADVERTISE_PAUSE_LEN       1
#define PHY_MII_ADVERTISE_100FULL_POS     8
#define PHY_MII_ADVERTISE_100FULL_LEN     1
#define PHY_MII_ADVERTISE_100HALF_POS     7
#define PHY_MII_ADVERTISE_100HALF_LEN     1
#define PHY_MII_ADVERTISE_10FULL_POS      6
#define PHY_MII_ADVERTISE_10FULL_LEN      1
#define PHY_MII_ADVERTISE_10HALF_POS      5
#define PHY_MII_ADVERTISE_10HALF_LEN      1
#define REG_MII_LPA                       0x05    /* Link partner ability reg    */
#define PHY_MII_LINK_PARNTNER_10FULL_POS  6
#define PHY_MII_LINK_PARNTNER_10FULL_LEN  1
#define PHY_MII_LINK_PARNTNER_10HALF_POS  5
#define PHY_MII_LINK_PARNTNER_10HALF_LEN  1
#define REG_MII_EXPANSION                 0x06    /* Expansion register          */
#define REG_MII_NEXT_PAGE                 0x07    /* Next page register          */
#define REG_MII_LPR_NEXT_PAGE             0x08    /* LPR next page register      */
#define REG_MII_CTRL1000                  0x09    /* 1000BASE-T control          */
#define PHY_MII_CTRL1000_1000FULL_POS     9
#define PHY_MII_CTRL1000_1000FULL_LEN     1
#define PHY_MII_CTRL1000_1000HALF_POS     8
#define PHY_MII_CTRL1000_1000HALF_LEN     1
#define REG_MII_STAT1000                  0x0a    /* 1000BASE-T status           */
#define PHY_MII_STAT1000_CFG_ERROR_POS    15
#define PHY_MII_STAT1000_CFG_ERROR_LEN    1

#define REG_MII_MMD_CTRL                  0x0d    /* MMD access control register */
#define REG_MII_MMD_DATA                  0x0e    /* MMD access data register    */

#define REG_MII_ESTATUS                   0x0f    /* Extended Status             */

#define REG_MII_SPEC_CTRL                 0x10    /* PHY specific func control   */
#define PHY_MII_SPEC_CTRL_CRS_ON_POS      3
#define PHY_MII_SPEC_CTRL_CRS_ON_LEN      1
#define REG_MII_SPEC_STATUS               0x11    /* PHY specific status         */
#define PHY_MII_SPEC_DUPLEX_POS           13
#define PHY_MII_SPEC_DUPLEX_LEN           1
#define REG_MII_INT_MASK                  0x12    /* Interrupt mask register     */

#ifdef   AISC_MODE
#define PHY_INT_MASK_LINK_UP_POS        10
#define PHY_INT_MASK_LINK_UP_LEN        1
#define PHY_INT_MASK_LINK_DOWN_POS      11
#define PHY_INT_MASK_LINK_DOWN_LEN      1
#else  //FPGA_MODE
#define PHY_INT_MASK_LINK_UP_POS        1
#define PHY_INT_MASK_LINK_UP_LEN        1
#define PHY_INT_MASK_LINK_DOWN_POS      0
#define PHY_INT_MASK_LINK_DOWN_LEN      1
#endif
#define REG_MII_INT_STATUS              0x13    /* Interrupt status register   */
#ifdef AISC_MODE
#define PHY_INT_STAT_LINK_UP_POS        10
#define PHY_INT_STAT_LINK_UP_LEN        1
#define PHY_INT_STAT_LINK_DOWN_POS      11
#define PHY_INT_STAT_LINK_DOWN_LEN      1
#else
#define PHY_INT_STAT_LINK_UP_POS        1
#define PHY_INT_STAT_LINK_UP_LEN        1
#define PHY_INT_STAT_LINK_DOWN_POS      0
#define PHY_INT_STAT_LINK_DOWN_LEN      1
#endif
#define REG_MII_DOWNG_CTRL              0x14    /* Speed auto downgrade control*/
#define REG_MII_RERRCOUNTER             0x15    /* Receive error counter       */

#define REG_MII_EXT_ADDR                0x1e    /* Extended reg's address      */
#define REG_MII_EXT_DATA                0x1f    /* Extended reg's date         */

#define FXGMAC_EPHY_ID_MASK             0x0000ffff

/* for ephy link capability
 * Advertisement control register(0x04)
 */
#define FXGMAC_ADVERTISE_SLCT           0x001f  /* Selector bits               */
#define FXGMAC_ADVERTISE_CSMA           0x0001  /* Only selector supported     */
//#define FXGMAC_ADVERTISE_1000FULL       0x0004  /* Try for 1000BASE-T full duplex */
//#define FXGMAC_ADVERTISE_1000HALF       0x0008  /* Try for 1000BASE-T half duplex */
#define FXGMAC_ADVERTISE_10HALF         0x0020  /* Try for 10mbps half-duplex  */
#define FXGMAC_ADVERTISE_10FULL         0x0040  /* Try for 10mbps full-duplex  */
#define FXGMAC_ADVERTISE_100HALF        0x0080  /* Try for 100mbps half-duplex */
#define FXGMAC_ADVERTISE_100FULL        0x0100  /* Try for 100mbps full-duplex */
#define FXGMAC_ADVERTISE_100BASE4       0x0200  /* Try for 100mbps 4k packets  */
#define FXGMAC_ADVERTISE_PAUSE_CAP      0x0400  /* Try for pause               */
#define FXGMAC_ADVERTISE_PAUSE_ASYM     0x0800  /* Try for asymmetric pause    */
#define FXGMAC_ADVERTISE_RESV           0x1000  /* Unused...                   */
#define FXGMAC_ADVERTISE_RFAULT         0x2000  /* Say we can detect faults    */
#define FXGMAC_ADVERTISE_LPACK          0x4000  /* Ack link partners response  */
#define FXGMAC_ADVERTISE_NPAGE          0x8000  /* Next page bit               */

/* 1000BASE-T Control register(0x09) */
#define FXGMAC_ADVERTISE_1000FULL      0x0200  /* Advertise 1000BASE-T full duplex */
#define FXGMAC_ADVERTISE_1000HALF      0x0100  /* Advertise 1000BASE-T half duplex */

#define REG_BIT_ADVERTISE_1000_CAP      (FXGMAC_ADVERTISE_1000FULL | FXGMAC_ADVERTISE_1000HALF)
#define REG_BIT_ADVERTISE_100_10_CAP    (FXGMAC_ADVERTISE_100FULL | FXGMAC_ADVERTISE_100HALF | FXGMAC_ADVERTISE_10FULL | FXGMAC_ADVERTISE_10HALF )

#ifndef SPEED_1000M
#define SPEED_1000M     1000
#endif
#ifndef SPEED_100M
#define SPEED_100M      100
#endif
#ifndef SPEED_10M
#define SPEED_10M       10
#endif

#ifndef SPEED_UNKNOWN
#define SPEED_UNKNOWN   0xffff
#endif

#ifndef DUPLEX_FULL
#define DUPLEX_FULL     1
#endif
#ifndef DUPLEX_HALF
#define DUPLEX_HALF     0
#endif

#ifndef BIT
#define BIT(n) (0x1<<(n))
#endif

#ifndef FXGMAC_EPHY_SPEED_MODE_BIT
#define FXGMAC_EPHY_SPEED_MODE                             0xc000
#define FXGMAC_EPHY_DUPLEX                                 0x2000
#define FXGMAC_EPHY_SPEED_MODE_BIT                         14
#define FXGMAC_EPHY_DUPLEX_BIT                             13
#define FXGMAC_EPHY_LINK_STATUS_BIT                        10

#endif

#define FXGMAC_EPHY_SMI_SEL_PHY                            0x0
#define FXGMAC_EPHY_SMI_SEL_SDS_QSGMII                     0x02
#define FXGMAC_EPHY_SMI_SEL_SDS_SGMII                      0x03

#define REG_MII_EXT_AFE_CONTROL_REGISTER3                0x12
#define REG_MII_EXT_AFE_CONTROL_CLKDAC_AON_POS           13
#define REG_MII_EXT_AFE_CONTROL_CLKDAC_AON_LEN           1
#define REG_MII_EXT_AFE_CONTROL_CLKDAC_AON_ON            1
#define REG_MII_EXT_ANALOG_CFG3                          0x52
#define MII_EXT_ANALOG_CFG3_ADC_START_CFG_POS            14
#define MII_EXT_ANALOG_CFG3_ADC_START_CFG_LEN            2
// VGA bandwidth, default is 2 after reset. Set to 0 to mitigate unstable issue in 130m.
#define MII_EXT_ANALOG_CFG3_ADC_START_CFG_DEFAULT        0x0
#define MII_EXT_ANALOG_CFG3_ON_TIME_CFG_POS              12
#define MII_EXT_ANALOG_CFG3_ON_TIME_CFG_LEN              2
#define MII_EXT_ANALOG_CFG3_VGA_AMP_GAIN_CFG_POS         8
#define MII_EXT_ANALOG_CFG3_VGA_AMP_GAIN_CFG_LEN         4
#define MII_EXT_ANALOG_CFG3_VGA_IBIAS_CFG_POS            4
#define MII_EXT_ANALOG_CFG3_VGA_IBIAS_CFG_LEN            3
#define MII_EXT_ANALOG_CFG3_OCP_CFG_POS                  2
#define MII_EXT_ANALOG_CFG3_OCP_CFG_LEN                  2
#define MII_EXT_ANALOG_CFG3_VGA_LPF_CFG_POS              0
#define MII_EXT_ANALOG_CFG3_VGA_LPF_CFG_LEN              2

#define REG_MII_EXT_PMA_DEBUG_KCOEF                      0x78
#define MII_EXT_PMA_DEBUG_KCOEF_IPR_KCOEF_GE_LNG_POS     8
#define MII_EXT_PMA_DEBUG_KCOEF_IPR_KCOEF_GE_LNG_LEN     6
// After reset, it's 0x10. We need change it to 0x20 to make it easier to linkup in gigabit mode with long cable.
#define MII_EXT_PMA_DEBUG_KCOEF_IPR_KCOEF_GE_LNG_DEFAULT 0x20
#define MII_EXT_PMA_DEBUG_KCOEF_IPR_KCOEF_DEFAULT_POS    0
#define MII_EXT_PMA_DEBUG_KCOEF_IPR_KCOEF_DEFAULT_LEN    6

#define REG_MII_EXT_LPBK_REG                             0x0a
#define REG_MII_EXT_LPBK_REG_ENABLE_LOOPBACK             0x3a18
#define REG_MII_EXT_LPBK_REG_CLEAN_LOOPBACK              0x3a08
#define REG_MII_EXT_SLEEP_CONTROL_REG                    0x27
#define REG_MII_EXT_SLEEP_REG_ENABLE_LOOPBACK            0x6812
#define REG_MII_EXT_SLEEP_REG_CLEAN_LOOPBACK             0xe812

#define REG_MII_EXT_ANALOG_CFG2                                 0x51
#define REG_MII_EXT_ANALOG_CFG2_VALUE                           0x4a9
#define REG_MII_EXT_ANALOG_CFG8                                 0x57
#define REG_MII_EXT_ANALOG_CFG8_VALUE                           0x274c
#define REG_MII_EXT_ANALOG_CFG8_137D1D05_VALUE                  0x264c

#define REG_MII_EXT_COMMON_LED_CFG                              0xa00b
#define REG_MII_EXT_COMMON_LED0_CFG                             0xa00c
#define REG_MII_EXT_COMMON_LED0_CFG_VALUE_SOLUTION0             0x2600
#define REG_MII_EXT_COMMON_LED0_CFG_VALUE_SOLUTION1             0x00
#define REG_MII_EXT_COMMON_LED0_CFG_VALUE_SOLUTION2             0x20
#define REG_MII_EXT_COMMON_LED0_CFG_VALUE_SOLUTION3             0x2600
#define REG_MII_EXT_COMMON_LED1_CFG                             0xa00d
#define REG_MII_EXT_COMMON_LED1_CFG_VALUE_SOLUTION0             0x1800
#define REG_MII_EXT_COMMON_LED1_CFG_VALUE_SOLUTION1             0x00
#define REG_MII_EXT_COMMON_LED1_CFG_VALUE_SOLUTION2             0x40
#define REG_MII_EXT_COMMON_LED2_CFG                             0xa00e
#define REG_MII_EXT_COMMON_LED2_CFG_VALUE_SOLUTION0             0x00
#define REG_MII_EXT_COMMON_LED2_CFG_VALUE_SOLUTION2             0x07
#define REG_MII_EXT_COMMON_LED2_CFG_VALUE_SOLUTION3             0x20
#define REG_MII_EXT_COMMON_LED2_CFG_VALUE_SOLUTION4             0x1800
#define REG_MII_EXT_COMMON_LED_BLINK_CFG                        0xa00f
#define REG_MII_EXT_COMMON_LED_BLINK_CFG_SOLUTION2              0x0f

#define REG_MII_EXT_COMMON_LED0_CFG_VALUE_SLEEP_SOLUTION3       0x2600

#define REG_MII_EXT_PKG_CFG0                                    0xa0
#define REG_MII_EXT_PKG_CHECK_POS                               14
#define REG_MII_EXT_PKG_CHECK_LEN                               2
#define REG_MII_EXT_PKG_ENABLE_CHECK                            0x2
#define REG_MII_EXT_PKG_DISABLE_CHECK                           0x1
#define REG_MII_EXT_SLEEP_CONTROL1                              0x27
#define MII_EXT_SLEEP_CONTROL1_EN_POS                           15
#define MII_EXT_SLEEP_CONTROL1_EN_LEN                           1
#define MII_EXT_SLEEP_CONTROL1_PLLON_IN_SLP_POS                 14
#define MII_EXT_SLEEP_CONTROL1_PLLON_IN_SLP_LEN                 1
#define REG_MII_EXT_PKG_RX_VALID0                               0xa3
#define REG_MII_EXT_REG_RX_VALID1                               0xa4
#define REG_MII_EXT_REG_RX_OS0                                  0xa5
#define REG_MII_EXT_REG_RX_OS1                                  0xa6
#define REG_MII_EXT_REG_RX_US0                                  0xa7
#define REG_MII_EXT_REG_RX_US1                                  0xa8
#define REG_MII_EXT_REG_RX_ERR                                  0xa9
#define REG_MII_EXT_REG_RX_0S_BAD                               0xaa
#define REG_MII_EXT_REG_RX_FRAGMENT                             0xab
#define REG_MII_EXT_REG_RX_NOSFD                                0xac
#define REG_MII_EXT_REG_TX_VALID0                               0xad
#define REG_MII_EXT_REG_TX_VALID1                               0xae
#define REG_MII_EXT_REG_TX_OS0                                  0xaf
#define REG_MII_EXT_REG_TX_OS1                                  0xb0
#define REG_MII_EXT_REG_TX_US0                                  0xb1
#define REG_MII_EXT_REG_TX_US1                                  0xb2
#define REG_MII_EXT_REG_TX_ERR                                  0xb3
#define REG_MII_EXT_REG_TX_OS_BAD                               0xb4
#define REG_MII_EXT_REG_TX_FRAGMENT                             0xb5
#define REG_MII_EXT_REG_TX_NOSFD                                0xb6
#define REG_MII_EXT_REG_PMA_DBG0_ADC                            0x13
#define REG_MII_EXT_ENABLE_GIGA_POWER_SAVING_FOR_SHORT_CABLE    0x3538
#define REG_MII_EXT_REG_CLD_REG0                                0x3a0
#define REG_MII_EXT_ENABLE_CLD_NP_WP                            0xeb24
#define REG_MII_EXT_REG_CLD_REG1                                0x3cc
#define REG_MII_EXT_ENABLE_CLD_GT_HT_BT                         0x7001
#define REG_MMD_EEE_ABILITY_REG                                 0x3c
#define REG_MMD_EEE_ABILITY_VALUE                               0x06

/* Below registers don't belong to GMAC, it has zero offset, not 0x2000 offset. mem_base + REG_XXX. */
/* When issue happens, driver write this register to trigger pcie sniffer.  */
#define REG_PCIE_TRIGGER                        0x1000
#define PCIE_TRIGGER_CODE_TX_HANG               0x00000002
#define PCIE_TRIGGER_CODE_LINKDOWN              0x00000003


#define  MGMT_EPHY_CTRL                         0x1004
/* check register address 0x1004
* b[6:5]       ephy_pause
* b[4:3]       ephy_speed 0b10 1000m 0b01 100m
* b[2]         ephy_duplex
* b[1]         ephy_link
* b[0]         ephy_reset.0-reset, 1-unreset. Should be set to 1 before use phy.
*/
#define MGMT_EPHY_CTRL_RESET_POS                0
#define MGMT_EPHY_CTRL_RESET_LEN                1
#define MGMT_EPHY_CTRL_STA_EPHY_RESET           0 // 0: reset state.
#define MGMT_EPHY_CTRL_STA_EPHY_RELEASE         1 // 1: release state.
#define MGMT_EPHY_CTRL_STA_EPHY_LINKUP          2 // 1: link up; 0: link down.
#define MGMT_EPHY_CTRL_STA_EPHY_LINKUP_POS      1
#define MGMT_EPHY_CTRL_STA_EPHY_LINKUP_LEN      1
#define MGMT_EPHY_CTRL_STA_EPHY_DUPLEX_POS      2 // ephy duplex
#define MGMT_EPHY_CTRL_STA_EPHY_DUPLEX_LEN      1

#define MGMT_EPHY_CTRL_STA_SPEED_POS            3
#define MGMT_EPHY_CTRL_STA_SPEED_LEN            2
#define MGMT_EPHY_CTRL_STA_SPEED_MASK           0x18

#define MGMT_EPHY_CTRL_ERROR_VALUE              0xffffffff

#define MGMT_PCIE_EP_CTRL                       0x1008

#define MGMT_PCIE_EP_CTRL_DBI_CS_EN_POS         0
#define MGMT_PCIE_EP_CTRL_DBI_CS_EN_LEN         1

#define MGMT_PCIE_CFG_CTRL                      0x8bc
#define PCIE_CFG_CTRL_DEFAULT_VAL               0x7ff40

#define MGMT_PCIE_CFG_CTRL_CS_EN_POS            0
#define MGMT_PCIE_CFG_CTRL_CS_EN_LEN            1

/* power management */
#define WOL_CTL                                 0x100c
#define WOL_PKT_EN_POS                          1 //set means magic and remote packet wakeup enable
#define WOL_PKT_EN_LEN                          1
#define WOL_LINKCHG_EN_POS                      0 //set means link change wakeup enable
#define WOL_LINKCHG_EN_LEN                      1
#define WOL_WAIT_TIME_POS                       2
#define WOL_WAIT_TIME_LEN                       13

#define OOB_WOL_CTRL                            0x1010
#define OOB_WOL_CTRL_DIS_POS                    0
#define OOB_WOL_CTRL_DIS_LEN                    1

#define MGMT_INT_CTRL0                          0x1100
/* b3:0 per rx ch interrupt
  * b7:4 per tx ch interrupt
  * b8  Safety interrupt signal for un-correctable error
  * b9  Safety interrupt signal for correctable error
  * b10 Interrupt signal to host system
  * b11 Magic Packet Received or Remote Wake-up Packet Received
  * b12 ethernet phy interrupt
  */

/* MAC management registers bit positions and sizes */
#define MGMT_INT_CTRL0_INT_MASK_POS             16
#define MGMT_INT_CTRL0_INT_MASK_LEN             16
#define MGMT_INT_CTRL0_INT_MASK_MASK            0xffff
#define MGMT_INT_CTRL0_INT_MASK_RXCH            0xf
#define MGMT_INT_CTRL0_INT_MASK_TXCH            0x10
#define MGMT_INT_CTRL0_INT_MASK_EX_PMT          0xf7ff
#define MGMT_INT_CTRL0_INT_MASK_DISABLE         0xf000

#define MGMT_INT_CTRL0_INT_STATUS_POS           0
#define MGMT_INT_CTRL0_INT_STATUS_LEN           16
#define MGMT_INT_CTRL0_INT_STATUS_MASK          0xffff
#define MGMT_INT_CTRL0_INT_STATUS_RX            0x0001
#define MGMT_INT_CTRL0_INT_STATUS_TX            0x0010
#define MGMT_INT_CTRL0_INT_STATUS_TX_INVERSE    0xffef
#define MGMT_INT_CTRL0_INT_STATUS_MISC_INVERSE  0xffdf
#define MGMT_INT_CTRL0_INT_STATUS_MISC          0x0020

#define MGMT_INT_CTRL0_INT_MASK_RXCH_POS        16
#define MGMT_INT_CTRL0_INT_STATUS_RXCH_POS      0
#define MGMT_INT_CTRL0_INT_STATUS_RXCH_LEN      4
#define MGMT_INT_CTRL0_INT_STATUS_RXCH_MASK     0xf
#define MGMT_INT_CTRL0_INT_STATUS_RXTX_LEN      5
#define MGMT_INT_CTRL0_INT_STATUS_RXTX_MASK     0x1f
#define MGMT_INT_CTRL0_INT_STATUS_RXTXMISC_MASK 0x3f

#define MGMT_INT_CTRL0_INT_MASK_TXCH_POS        20
#define MGMT_INT_CTRL0_INT_STATUS_TXCH_POS      4
#define MGMT_INT_CTRL0_INT_STATUS_TXCH_LEN      1
#define MGMT_INT_CTRL0_INT_STATUS_TXCH_MASK     0x1

#define MGMT_MAC_PHYIF_STA_POS                  0
#define MGMT_MAC_AN_SR0_POS                     1
#define MGMT_MAC_AN_SR1_POS                     2
#define MGMT_MAC_AN_SR2_POS                     3
#define MGMT_MAC_PMT_STA_POS                    4
#define MGMT_MAC_LPI_STA_POS                    5
#define MGMT_MAC_MMC_STA_POS                    8
#define MGMT_MAC_RX_MMC_STA_POS                 9
#define MGMT_MAC_TX_MMC_STA_POS                 10
#define MGMT_MMC_IPCRXINT_POS                   11
#define MGMT_MAC_TX_RX_STA0_POS                 13
#define MGMT_MAC_TX_RX_STA1_POS                 14
#define MGMT_MAC_GPIO_SR_POS                    15

/* Interrupt Ctrl1 */
#define INT_CTRL1                               0x1104
#define INT_CTRL1_TMR_CNT_CFG_MAX_POS           0        /* Timer counter cfg max. Default 0x19, 1us. */
#define INT_CTRL1_TMR_CNT_CFG_MAX_LEN           10
#define INT_CTRL1_TMR_CNT_CFG_DEF_VAL           0x19
#define INT_CTRL1_MSI_AIO_EN_POS                16
#define INT_CTRL1_MSI_AIO_EN_LEN                1

/* Interrupt Moderation */
#define INT_MOD                                 0x1108
#define INT_MOD_TX_POS                          16
#define INT_MOD_TX_LEN                          12
#define INT_MOD_RX_POS                          0
#define INT_MOD_RX_LEN                          12
#define INT_MOD_IN_US                           200     /*in us*/

/* PCIE LTR 2 working modes:
 * Two working mode:
 * 1. SW trigger
 * LTR idle threshold timer set as 0, enable LTR enable will trigger one LTR message
 * Note: PCIe cfg enable should set in initialization before enable LTR.
 * 2. HW auto trigger
 * LTR idle threshold timer set as one non-zero value, HW monitor system status,
 * when system idle timer over threshold, HW send out LTR message
 * system exit idle state, send out one LTR exit message.
 */
#define  LTR_CTRL                               0x1130
#define  LTR_CTRL_IDLE_THRE_TIMER_POS           16
#define  LTR_CTRL_IDLE_THRE_TIMER_LEN           14      /* in 8ns units*/
#define  LTR_CTRL_IDLE_THRE_TIMER_VAL           0x3fff
#define  LTR_CTRL_EN_POS                        0
#define  LTR_CTRL_EN_LEN                        1

#define  LTR_CTRL1                              0x1134  /* LTR latency message, only for SW enable. */
#define  LTR_CTRL1_LTR_MSG_POS                  0
#define  LTR_CTRL1_LTR_MSG_LEN                  32

#define  LTR_CTRL2                              0x1138
#define  LTR_CTRL2_DBG_DATA_POS                 0
#define  LTR_CTRL2_DBG_DATA_LEN                 32

#define  LTR_IDLE_ENTER                         0x113c  /* LTR_CTRL3, LTR latency message, only for System IDLE Start. */
#define  LTR_IDLE_ENTER_POS                     0
#define  LTR_IDLE_ENTER_LEN                     10
#define  LTR_IDLE_ENTER_USVAL                   900
#define  LTR_IDLE_ENTER_SCALE_POS               10
#define  LTR_IDLE_ENTER_SCALE_LEN               5
#define  LTR_IDLE_ENTER_SCALE                   2       /* 0-1ns, 1-32ns, 2-1024ns, 3-32,768ns, 4-1,048,576ns, 5-33,554,432ns, 110-111-Not Permitted.*/
#define  LTR_IDLE_ENTER_REQUIRE_POS             15
#define  LTR_IDLE_ENTER_REQUIRE_LEN             1
#define  LTR_IDLE_ENTER_REQUIRE                 1

#define  LTR_IDLE_EXIT                          0x1140  /* LTR_CTRL4, LTR latency message, only for System IDLE End. */
#define  LTR_IDLE_EXIT_POS                      0
#define  LTR_IDLE_EXIT_LEN                      10
#define  LTR_IDLE_EXIT_USVAL                    171
#define  LTR_IDLE_EXIT_SCALE_POS                10
#define  LTR_IDLE_EXIT_SCALE_LEN                5
#define  LTR_IDLE_EXIT_SCALE                    2
#define  LTR_IDLE_EXIT_REQUIRE_POS              15
#define  LTR_IDLE_EXIT_REQUIRE_LEN              1
#define  LTR_IDLE_EXIT_REQUIRE                  1

#define  LPW_CTRL                               0x1188
#define  LPW_CTRL_L1SS_EN_POS                   22
#define  LPW_CTRL_L1SS_EN_LEN                   1
#define  LPW_CTRL_L1SS_SEL_POS                  21  /* 0 - up to both CFG0x158 and reg1188 L1ss setting. 1 - up to CFG0x158 L1ss setting. */
#define  LPW_CTRL_L1SS_SEL_LEN                  1
#define  LPW_CTRL_L1SS_SEL_CFG                  1
#define  LPW_CTRL_ASPM_L1_CPM_POS               19  /*L1.CPM mode enable bit. Default 0,set as 1 enable this mode. clkreq pin need to connect RC*/
#define  LPW_CTRL_ASPM_L1_CPM_LEN               1
#define  LPW_CTRL_ASPM_L0S_EN_POS               17
#define  LPW_CTRL_ASPM_L0S_EN_LEN               1
#define  LPW_CTRL_ASPM_L1_EN_POS                16
#define  LPW_CTRL_ASPM_L1_EN_LEN                1
#define  LPW_CTRL_ASPM_LPW_EN_POS               9  /* application ready to enter L23. */
#define  LPW_CTRL_ASPM_LPW_EN_LEN               1
#define  LPW_CTRL_SYS_CLK_125_SEL_POS           8  /* system 125M select: 125M or 62.5MHz. Default: 125MHz.*/
#define  LPW_CTRL_SYS_CLK_125_SEL_LEN           1
#define  LPW_CTRL_PCIE_RADM_CG_EN_POS           5  /* clock gating enable bit of PCIe Radm clock. Default 1; set as 1, enable gating.*/
#define  LPW_CTRL_PCIE_RADM_CG_EN_LEN           1
#define  LPW_CTRL_PCIE_CORE_CG_EN_POS           4  /* clock gating enable bit of PCIe Core clock. Default 1; set as 1, enable gating.*/
#define  LPW_CTRL_PCIE_CORE_CG_EN_LEN           1
#define  LPW_CTRL_PCIE_AXI_CG_EN_POS            3  /* clock gating enable bit of PCIe AXI clock.Default 1; set as 1, enable gating.*/
#define  LPW_CTRL_PCIE_AXI_CG_EN_LEN            1
#define  LPW_CTRL_GMAC_AXI_CG_EN_POS            2  /* clock gating enable bit of GMAC AXI clock. Default 1; set as 1, enable gating.*/
#define  LPW_CTRL_GMAC_AXI_CG_EN_LEN            1
#define  LPW_CTRL_MDIO2APB_CG_EN_POS            1  /* clock gating enable bit of MDIO2APB, default 1. Set as 1, enable clock gating feature. */
#define  LPW_CTRL_MDIO2APB_CG_EN_LEN            1
#define  LPW_CTRL_OTP_CLK_ON_POS                0  /* Turn on before SW OTP operation, default 1. */
#define  LPW_CTRL_OTP_CLK_ON_LEN                1

#define  MSI_PBA_REG                            0x1300
#define  SYS_RESET_REG                          0x152c
#define  SYS_RESET_POS                          31
#define  SYS_RESET_LEN                          1

#define  REG_PCIE_PSM_STATE                     0x1994  /* PCIe PHY power state. */
#define  PCIE_PSM_STATE_POS                     0
#define  PCIE_PSM_STATE_LEN                     4
#define  PCIE_PSM_STATE_P0                      2
#define  PCIE_PSM_STATE_P0s                     3
#define  PCIE_PSM_STATE_P1                      4
#define  PCIE_PSM_STATE_P1_CPM                  5
#define  PCIE_PSM_STATE_P1_1                    6
#define  PCIE_PSM_STATE_P1_2                    7
#define  PCIE_PSM_STATE_P2                      8

#define  REG_PCIE_SERDES_STATUS                 0x1998
#define  PCIE_SERDES_STATUS_DRV_ON_POS          11
#define  PCIE_SERDES_STATUS_DRV_ON_LEN          1
#define  PCIE_SERDES_STATUS_RX_PD_POS           10
#define  PCIE_SERDES_STATUS_RX_PD_LEN           1
#define  PCIE_SERDES_STATUS_PI_PD_POS           9
#define  PCIE_SERDES_STATUS_PI_PD_LEN           1
#define  PCIE_SERDES_STATUS_SIGDET_ON_POS       8
#define  PCIE_SERDES_STATUS_SIGDET_ON_LEN       1
#define  PCIE_SERDES_STATUS_TX_VCM_POS          7
#define  PCIE_SERDES_STATUS_TX_VCM_LEN          1
#define  PCIE_SERDES_STATUS_RX_RT50_POS         6
#define  PCIE_SERDES_STATUS_RX_RT50_LEN         1
#define  PCIE_SERDES_STATUS_BEACON_ON_POS       5
#define  PCIE_SERDES_STATUS_BEACON_ON_LEN       1
#define  PCIE_SERDES_STATUS_PLL_ON_POS          4
#define  PCIE_SERDES_STATUS_PLL_ON_LEN          1
#define  PCIE_SERDES_STATUS_REFCLK_ON_POS       3
#define  PCIE_SERDES_STATUS_REFCLK_ON_LEN       1
#define  PCIE_SERDES_STATUS_LDO_ON_POS          2
#define  PCIE_SERDES_STATUS_LDO_ON_LEN          1
#define  PCIE_SERDES_STATUS_HW_EN_SDS_BIAS_POS  1
#define  PCIE_SERDES_STATUS_HW_EN_SDS_BIAS_LEN  1
#define  PCIE_SERDES_STATUS_HW_BIAS_ON_POS      0
#define  PCIE_SERDES_STATUS_HW_BIAS_ON_LEN      1

#define  REG_PCIE_SERDES_PLL                    0x199c
#define  PCIE_SERDES_PLL_AUTOOFF_POS            0
#define  PCIE_SERDES_PLL_AUTOOFF_LEN            1

#define  NS_OF_GLB_CTL                  0x1B00
#define  NS_TPID_PRO                    0x1B04
#define  NS_LUT_ROMOTE0                 0x1B08
#define  NS_LUT_ROMOTE1                 0X1B0C
#define  NS_LUT_ROMOTE2                 0X1B10
#define  NS_LUT_ROMOTE3                 0X1B14
#define  NS_LUT_TARGET0                 0X1B18
#define  NS_LUT_TARGET1                 0X1B1C
#define  NS_LUT_TARGET2                 0X1B20
#define  NS_LUT_TARGET3                 0X1B24
#define  NS_LUT_SOLICITED0              0X1B28
#define  NS_LUT_SOLICITED1              0X1B2C
#define  NS_LUT_SOLICITED2              0X1B30
#define  NS_LUT_SOLICITED3              0X1B34
#define  NS_LUT_MAC_ADDR                0X1B38
#define  NS_LUT_MAC_ADDR_CTL            0X1B3C
#define  NS_LUT_TARGET4                 0X1B78
#define  NS_LUT_TARGET5                 0X1B7c
#define  NS_LUT_TARGET6                 0X1B80
#define  NS_LUT_TARGET7                 0X1B84

#define NS_OF_GLB_CTL_TX_CLK_EN_POS     2
#define NS_OF_GLB_CTL_TX_CLK_EN_LEN     1
#define NS_OF_GLB_CTL_RX_CLK_EN_POS     1
#define NS_OF_GLB_CTL_RX_CLK_EN_LEN     1
#define NS_OF_GLB_CTL_EN_POS            0
#define NS_OF_GLB_CTL_EN_ELN            1
#define NS_TPID_PRO_STPID_POS           16
#define NS_TPID_PRO_STPID_LEN           16
#define NS_TPID_PRO_CTPID_POS           0
#define NS_TPID_PRO_CTPID_LEN           16
#define NS_LUT_DST_CMP_TYPE_POS         19
#define NS_LUT_DST_CMP_TYPE_LEN         1
#define NS_LUT_DST_IGNORED_POS          18
#define NS_LUT_DST_IGNORED_LEN          1
#define NS_LUT_REMOTE_AWARED_POS        17
#define NS_LUT_REMOTE_AWARED_LEN        1
#define NS_LUT_TARGET_ISANY_POS         16
#define NS_LUT_TARGET_ISANY_LEN         1
#define NS_LUT_MAC_ADDR_LOW_POS         0
#define NS_LUT_MAC_ADDR_LOW_LEN         16

/* RSS implementation registers, 20210817 */

/* 10 RSS key registers */
#define MGMT_RSS_KEY0                   0x1020
#define MGMT_RSS_KEY9                   0x1044
#define MGMT_RSS_KEY_REG_INC            0x4

/* RSS control register */
#define MGMT_RSS_CTRL                   0x1048
/* b31  enable
 * b12:10   indirection table size. 2^(val+1)
 * b9:8 default Queue NO.
 * b7:0 hash type or options
 */

/* RSS ctrl register bit definitions.
 * [0] ipv4
 * [1] tcpv4
 * [2] udpv4
 * [3] ipv6
 * [4] tcpv6
 * [5] udpv6
 * [6] only ipv4 udp check IP hash
 * [7] only ipv6 udp check IP hash
 */
#define MGMT_RSS_CTRL_OPT_POS           0
#define MGMT_RSS_CTRL_OPT_LEN           8
#define MGMT_RSS_CTRL_OPT_MASK          0xff
#define MGMT_RSS_CTRL_IPV4_EN           0x01
#define MGMT_RSS_CTRL_TCPV4_EN          0x02
#define MGMT_RSS_CTRL_UDPV4_EN          0x04
#define MGMT_RSS_CTRL_IPV6_EN           0x08
#define MGMT_RSS_CTRL_TCPV6_EN          0x10
#define MGMT_RSS_CTRL_UDPV6_EN          0x20
#define MGMT_RSS_CTRL_IPV4              0x0
#define MGMT_RSS_CTRL_IPV4              0x0

#define MGMT_RSS_CTRL_DEFAULT_Q_POS     8
#define MGMT_RSS_CTRL_DEFAULT_Q_LEN     2
#define MGMT_RSS_CTRL_DEFAULT_Q_MASK    0x3

#define MGMT_RSS_CTRL_TBL_SIZE_POS      10
#define MGMT_RSS_CTRL_TBL_SIZE_LEN      3
#define MGMT_RSS_CTRL_TBL_SIZE_MASK     0x7

#define MAC_RSSCR_IP2TE_POS             1
#define MAC_RSSCR_IP2TE_LEN             1
#define MAC_RSSCR_RSSE_POS              31
#define MAC_RSSCR_RSSE_LEN              1

/* rss indirection table (IDT) */
#define MGMT_RSS_IDT                    0x1050
/* b0:1 entry0
 * b2:3 entry1
 * ...
 */
#define MGMT_RSS_IDT_REG_INC            4
#define MGMT_RSS_IDT_ENTRY_PER_REG      16
#define MGMT_RSS_IDT_ENTRY_MASK         0x3
#define MAC_CRC_LENGTH                  4

 /* osc_ctrl */
#define MGMT_XST_OSC_CTRL                    0x1158
#define MGMT_XST_OSC_CTRL_XST_OSC_SEL_POS    2
#define MGMT_XST_OSC_CTRL_XST_OSC_SEL_LEN    1
#define MGMT_XST_OSC_CTRL_EN_OSC_POS         1
#define MGMT_XST_OSC_CTRL_EN_OSC_LEN         1
#define MGMT_XST_OSC_CTRL_EN_XST_POS         0
#define MGMT_XST_OSC_CTRL_EN_XST_LEN         1

#define MGMT_WPI_CTRL0                       0x1160
/* b1:0  wpi_mode      "2b00: normal working mode; 2b01: WPI write mode, work in sleep mode; 2b10: WPI read mode, work after sleep before normal working mode;"
 * b2    ram_op_done   Each row ram read done, SW can start read after done;
 * b3    wpi_op_done   WPI read done for the total packet;
 * b17:4 wpi_pkt_len   WOL packet length, unit byte;
 * b31   wpi_fail      Error status in Sleep mode;
 */
#define MGMT_WPI_CTRL0_WPI_MODE_POS     0
#define MGMT_WPI_CTRL0_WPI_MODE_LEN     2
#define MGMT_WPI_CTRL0_WPI_MODE_NORMAL  0x00 // normal working mode.
#define MGMT_WPI_CTRL0_WPI_MODE_WR      0x01 // WPI write mode, work in sleep mode.
#define MGMT_WPI_CTRL0_WPI_MODE_RD      0x02 // WPI read mode, work after sleep before normal working mode.
#define MGMT_WPI_CTRL0_RAM_OP_DONE      0x4
#define MGMT_WPI_CTRL0_WPI_OP_DONE      0x8
#define MGMT_WPI_CTRL0_WPI_PKT_LEN_POS  4
#define MGMT_WPI_CTRL0_WPI_PKT_LEN_LEN  14
#define MGMT_WPI_CTRL0_WPI_FAIL         0x80000000

#define MGMT_WPI_CTRL1_DATA             0x1164

#define MGMT_WOL_CTRL                   0x1530
/* b0    link_chg_status   1: waken by link-change
 * b1    mgk_pkt_status    1: waken by magic-packet
 * b2    rwk_pkt_status    1: waken by remote patten packet
 */
#define MGMT_WOL_CTRL_WPI_LINK_CHG                   1
#define MGMT_WOL_CTRL_WPI_MGC_PKT                    2
#define MGMT_WOL_CTRL_WPI_RWK_PKT                    4
#define MGMT_WOL_CTRL_WPI_RWK_PKT_NUMBER             0x010000

#define MGMT_RMK_CTRL                                0x1400

#define MGMT_SIGDET_DEGLITCH                         0x17f0
#define MGMT_SIGDET_DEGLITCH_DISABLE_POS             2   //sigdet deglitch disable ,active low
#define MGMT_SIGDET_DEGLITCH_DISABLE_LEN             1
#define MGMT_SIGDET_DEGLITCH_TIME_WIN_POS            3   //sigdet deglitch time windows filter seltion
#define MGMT_SIGDET_DEGLITCH_TIME_WIN_LEN            2
#define MGMT_SIGDET_DEGLITCH_TIME_WIN_10ns           0
#define MGMT_SIGDET_DEGLITCH_TIME_WIN_20ns           1
#define MGMT_SIGDET_DEGLITCH_TIME_WIN_30ns           2
#define MGMT_SIGDET_DEGLITCH_TIME_WIN_40ns           3

#define MGMT_SIGDET                 0x17f8
#define MGMT_SIGDET_POS             13
#define MGMT_SIGDET_LEN             3
#define MGMT_SIGDET_55MV            7
#define MGMT_SIGDET_50MV            6
#define MGMT_SIGDET_45MV            5  //default value
#define MGMT_SIGDET_40MV            4
#define MGMT_SIGDET_35MV            3
#define MGMT_SIGDET_30MV            2
#define MGMT_SIGDET_25MV            1
#define MGMT_SIGDET_20MV            0

#define FXGMAC_MTL_REG(pdata, n, reg)                   \
    ((pdata)->mac_regs + MTL_Q_BASE + ((n) * MTL_Q_INC) + (reg))

#define FXGMAC_DMA_REG(channel, reg)    ((channel)->dma_regs + (reg))

//#define RSS_Q_COUNT                       4
#define MSI_ID_RXQ0                         0
#define MSI_ID_RXQ1                         1
#define MSI_ID_RXQ2                         2
#define MSI_ID_RXQ3                         3
#define MSI_ID_TXQ0                         4

#if  1//msi table  modify  to 6  0~3  rx  4  tx  5  phy/other
#define MSI_ID_PHY_OTHER                    5
//#define MSI_ID_TXQ2                       6
//#define MSI_ID_TXQ3                       7
//#define MSI_ID_SFTUE                      8
//#define MSI_ID_SFTCE                      9
//#define MSI_ID_SBD                        10
//#define MSI_ID_PMT                        11
//#define MSI_ID_PHY                        12

#define MSIX_TBL_MAX_NUM                6
#define MSIX_TBL_RXTX_NUM               5

#else
#define MSI_ID_TXQ1                     5
#define MSI_ID_TXQ2                     6
#define MSI_ID_TXQ3                     7
#define MSI_ID_SFTUE                    8
#define MSI_ID_SFTCE                    9
#define MSI_ID_SBD                      10
#define MSI_ID_PMT                      11
#define MSI_ID_PHY                      12

#define MSIX_TBL_MAX_NUM                16
#define MSIX_TBL_RXTX_NUM                8
#endif
#define MSIX_TBL_BASE_ADDR              0x1200
#define MSIX_TBL_MASK_OFFSET            0xc
#define MSIX_TBL_DATA_OFFSET            0x8
#define MSIX_TBL_ADDR_OFFSET            0x0

/*******************************************************************
        efuse entry. val31:0 -> offset15:0
        offset7:0
        offset15:8
        val7:0
        val15:8
        val23:16
        val31:24
*******************************************************************/
#define EFUSE_OP_CTRL_0                 0x1500
#define EFUSE_OP_WR_DATA_POS            16
#define EFUSE_OP_WR_DATA_LEN            8
#define EFUSE_OP_ADDR_POS               8
#define EFUSE_OP_ADDR_LEN               8
#define EFUSE_OP_START_POS              2
#define EFUSE_OP_START_LEN              1
#define EFUSE_OP_MODE_POS               0
#define EFUSE_OP_MODE_LEN               2
#define EFUSE_OP_MODE_ROW_WRITE         0x0
#define EFUSE_OP_MODE_ROW_READ          0x1
#define EFUSE_OP_MODE_AUTO_LOAD         0x2
#define EFUSE_OP_MODE_READ_BLANK        0x3

#define EFUSE_OP_CTRL_1                 0x1504
#define EFUSE_OP_RD_DATA_POS            24
#define EFUSE_OP_RD_DATA_LEN            8
#define EFUSE_OP_BIST_ERR_ADDR_POS      16
#define EFUSE_OP_BIST_ERR_ADDR_LEN      8
#define EFUSE_OP_BIST_ERR_CNT_POS       8
#define EFUSE_OP_BIST_ERR_CNT_LEN       8
#define EFUSE_OP_PGM_PASS_POS           2
#define EFUSE_OP_PGM_PASS_LEN           1
#define EFUSE_OP_DONE_POS               1
#define EFUSE_OP_DONE_LEN               1

//efuse layout refer to http://redmine.motor-comm.com/issues/3856
#define EFUSE_FISRT_UPDATE_ADDR                 255
#define EFUSE_SECOND_UPDATE_ADDR                209
#define FXGMAC_EFUSE_MAX_ENTRY                    39
#define FXGMAC_EFUSE_MAX_ENTRY_UNDER_LED_COMMON   24
#define EFUSE_PATCH_ADDR_START_BYTE             0
#define EFUSE_PATCH_DATA_START_BYTE             2
#define EFUSE_REGION_A_B_LENGTH                 18
#define EFUSE_EACH_PATH_SIZE                    6

#define EFUSE_REVID_REGISTER            0x0008
#define EFUSE_SUBSYS_REGISTER           0x002c
#define MACA0LR_FROM_EFUSE              0x1520 //mac[5]->bit7:0, mac[4]->bit15:8, mac[3]->bit23:16, mac[2]->bit31:24.
#define MACA0HR_FROM_EFUSE              0x1524 //mac[1]->bit7:0, mac[0]->bit15:8. mac[6] = {00, 01, 02, 03, 04, 05} 00-01-02-03-04-05.

#define EFUSE_LED_ADDR                  0x00
#define EFUSE_LED_POS                   0
#define EFUSE_LED_LEN                   5
#define EFUSE_OOB_ADDR                  0x07
#define EFUSE_OOB_POS                   2
#define EFUSE_OOB_LEN                   1
#define EFUSE_LED_SOLUTION0             0
#define EFUSE_LED_SOLUTION1             1
#define EFUSE_LED_SOLUTION2             2
#define EFUSE_LED_SOLUTION3             3
#define EFUSE_LED_SOLUTION4             4
#define EFUSE_LED_COMMON_SOLUTION       0x1f

/******************** Below for pcie configuration register. *********************/
#define REG_PCI_VENDOR_ID                       0x0     /* WORD reg */
#define REG_PCI_DEVICE_ID                       0x2     /* WORD reg */
#define PCI_DEVICE_ID_FUXI                      0x6801

#define REG_PCI_COMMAND                         0x4
#define PCI_COMMAND_IO_SPACE_POS                0
#define PCI_COMMAND_IO_SPACE_LEN                1
#define PCI_COMAMND_MEM_SPACE_POS               1
#define PCI_COMAMND_MEM_SPACE_LEN               1
#define PCI_COMMAND_MASTER_POS                  2
#define PCI_COMMAND_MASTER_LEN                  1
#define PCI_COMMAND_DIS_INT_POS                 10
#define PCI_COMMAND_DIS_INT_LEN                 1
#define PCI_COMMAND_INTX_STATUS_POS             19
#define PCI_COMMAND_INTX_STATUS_LEN             1

#define REG_PCI_REVID                           0x8     /* BYTE reg */
#define REG_PCI_PROGRAM_INTF                    0x9     /* BYTE reg PCI Class Program Interface */
#define REG_PCI_SUB_CLASS                       0xa     /* BYTE reg */
#define REG_PCI_BASE_CLASS                      0xb     /* BYTE reg */
#define REG_CACHE_LINE_SIZE                     0xc


#define REG_MEM_BASE                            0x10    /* DWORD or QWORD reg */
#define REG_MEM_BASE_HI                         0x14    /* DWORD or QWORD reg */

#define REG_IO_BASE                             0x20    /* DWORD reg */

#define REG_PCI_SUB_VENDOR_ID                   0x2c    /* WORD reg */
#define REG_PCI_SUB_DEVICE_ID                   0x2e    /* WORD reg */

#define REG_INT_LINE                            0x3c    /* BYTE reg */

#define REG_PM_STATCTRL                         0x44    /* WORD reg */
#define PM_STATCTRL_PWR_STAT_POS                0
#define PM_STATCTRL_PWR_STAT_LEN                2
#define PM_STATCTRL_PWR_STAT_D3                 3
#define PM_STATCTRL_PWR_STAT_D0                 0
#define PM_CTRLSTAT_PME_EN_POS                  8
#define PM_CTRLSTAT_PME_EN_LEN                  1
#define PM_CTRLSTAT_DATA_SEL_POS                9
#define PM_CTRLSTAT_DATA_SEL_LEN                4
#define PM_CTRLSTAT_DATA_SCAL_POS               13
#define PM_CTRLSTAT_DATA_SCAL_LEN               2
#define PM_CTRLSTAT_PME_STAT_POS                15
#define PM_CTRLSTAT_PME_STAT_LEN                1

#define REG_DEVICE_CTRL1                        0x78
#define DEVICE_CTRL1_MPS_POS                    5     //MPS: max payload size
#define DEVICE_CTRL1_MPS_LEN                    3
#define DEVICE_CTRL1_MPS_128B                   0
#define DEVICE_CTRL1_MPS_256B                   1
#define DEVICE_CTRL1_MPS_512B                   2
#define DEVICE_CTRL1_MPS_1024B                  3
#define DEVICE_CTRL1_MPS_2048B                  4
#define DEVICE_CTRL1_MPS_4096B                  5
#define DEVICE_CTRL1_CONTROL_POS                0
#define DEVICE_CTRL1_CONTROL_LEN                16
#define DEVICE_CTRL1_STATUS_POS                 16
#define DEVICE_CTRL1_STATUS_LEN                 16

#define REG_PCI_LINK_CTRL                       0x80
#define PCI_LINK_CTRL_CONTROL_POS               0
#define PCI_LINK_CTRL_CONTROL_LEN               16
#define PCI_LINK_CTRL_ASPM_CONTROL_POS          0
#define PCI_LINK_CTRL_ASPM_CONTROL_LEN          2
#define PCI_LINK_CTRL_L1_STATUS                 2
#define PCI_LINK_CTRL_CONTROL_CPM_POS           8       /*L1.CPM mode enable bit. Default 0,set as 1 enable this mode. clkreq pin need to connect RC*/
#define PCI_LINK_CTRL_CONTROL_CPM_LEN           1
#define PCI_LINK_CTRL_STATUS_POS                16
#define PCI_LINK_CTRL_STATUS_LEN                16

#define REG_DEVICE_CTRL2                        0x98    /* WORD reg */
#define DEVICE_CTRL2_LTR_EN_POS                 10      /* Enable from BIOS side. */
#define DEVICE_CTRL2_LTR_EN_LEN                 1

#define REG_MSIX_CAPABILITY                     0xb0

/* ASPM L1ss PM Substates */
#define REG_ASPM_L1SS_CAP                       0x154   /* Capabilities Register */
#define ASPM_L1SS_CAP_PCIPM_L1_2_POS            0       /* PCI-PM L1.2 Supported */
#define ASPM_L1SS_CAP_PCIPM_L1_2_LEN            1
#define ASPM_L1SS_CAP_PCIPM_L1_1_POS            1       /* PCI-PM L1.1 Supported */
#define ASPM_L1SS_CAP_PCIPM_L1_1_LEN            1
#define ASPM_L1SS_CAP_ASPM_L1_2_POS             2       /* ASPM L1.2 Supported */
#define ASPM_L1SS_CAP_ASPM_L1_2_LEN             1
#define ASPM_L1SS_CAP_ASPM_L1_1_POS             3       /* ASPM L1.1 Supported */
#define ASPM_L1SS_CAP_ASPM_L1_1_LEN             1
#define ASPM_L1SS_CAP_L1_PM_SS_POS              4       /* L1 PM Substates Supported */
#define ASPM_L1SS_CAP_L1_PM_SS_LEN              1
#define ASPM_L1SS_CAP_CM_RESTORE_TIME_POS       8       /* Port Common_Mode_Restore_Time */
#define ASPM_L1SS_CAP_CM_RESTORE_TIME_LEN       8
#define ASPM_L1SS_CAP_P_PWR_ON_SCALE_POS        16      /* Port T_POWER_ON scale */
#define ASPM_L1SS_CAP_P_PWR_ON_SCALE_LEN        2
#define ASPM_L1SS_CAP_P_PWR_ON_VALUE_POS        19      /* Port T_POWER_ON value */
#define ASPM_L1SS_CAP_P_PWR_ON_VALUE_LEN        5

#define REG_ASPM_L1SS_CTRL1                     0x158
#define REG_ASPM_L1SS_CTRL1_VALUE               0x405e000f
#define ASPM_L1SS_CTRL1_L12_PCIPM_EN_POS        0       /* L1.2 in D3 state. */
#define ASPM_L1SS_CTRL1_L12_PCIPM_EN_LEN        1
#define ASPM_L1SS_CTRL1_L11_PCIPM_EN_POS        1       /* L1.1 in D3 state. */
#define ASPM_L1SS_CTRL1_L11_PCIPM_EN_LEN        1
#define ASPM_L1SS_CTRL1_L12_EN_POS              2
#define ASPM_L1SS_CTRL1_L12_EN_LEN              1
#define ASPM_L1SS_CTRL1_L11_EN_POS              3
#define ASPM_L1SS_CTRL1_L11_EN_LEN              1
#define ASPM_L1SS_CTRL1_CM_RESTORE_TIME_POS     8       /* Common_Mode_Restore_Time */
#define ASPM_L1SS_CTRL1_CM_RESTORE_TIME_LEN     8
#define ASPM_L1SS_CTRL1_LTR_L12_TH_VALUE_POS    16      /* LTR_L1.2_THRESHOLD_Value */
#define ASPM_L1SS_CTRL1_LTR_L12_TH_VALUE_LEN    10
#define ASPM_L1SS_CTRL1_L12_TH_SCALE_POS        29      /* LTR_L1.2_THRESHOLD_Scale */
#define ASPM_L1SS_CTRL1_L12_TH_SCALE_LEN        3

#define REG_ASPM_L1SS_CTL2                      0x15c   /* Control 2 Register */

#define REG_ASPM_CONTROL                        0x70c
#define ASPM_L1_IDLE_THRESHOLD_POS              27
#define ASPM_L1_IDLE_THRESHOLD_LEN              3
#define ASPM_L1_IDLE_THRESHOLD_1US              0
#define ASPM_L1_IDLE_THRESHOLD_2US              1
#define ASPM_L1_IDLE_THRESHOLD_4US              2
#define ASPM_L1_IDLE_THRESHOLD_8US              3       /* default value after reset. */
#define ASPM_L1_IDLE_THRESHOLD_16US             4
#define ASPM_L1_IDLE_THRESHOLD_32US             5
#define ASPM_L1_IDLE_THRESHOLD_64US             6

#define REG_POWER_EIOS                          0x710
#define POWER_EIOS_POS                          7
#define POWER_EIOS_LEN                          1

#define AISTONEID_137D1D05_ADJUST_SI           0x137d1d05

#endif /* __FXGMAC_GMAC_REG_H__ */
