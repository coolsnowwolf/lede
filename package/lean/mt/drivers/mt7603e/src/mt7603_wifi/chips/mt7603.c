/*
 ***************************************************************************
 * MediaTek Inc. 
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

	Module Name:
	mt7603.c
*/

#include "rt_config.h"
#ifdef MT7603_E1
#include "mcu/mt7603_firmware.h"
#endif /* MT7603_E1 */
#ifdef MT7603_E2
#include "mcu/mt7603_e2_firmware.h"
#endif /* MT7603_E2 */
#include "eeprom/mt7603_e2p.h"
#include "phy/wf_phy_back.h"

static VOID mt7603_bbp_adjust(RTMP_ADAPTER *pAd)
{
	static char *ext_str[]={"extNone", "extAbove", "", "extBelow"};
	UCHAR rf_bw, ext_ch;

#ifdef DOT11_N_SUPPORT
	if (get_ht_cent_ch(pAd, &rf_bw, &ext_ch) == FALSE)
#endif /* DOT11_N_SUPPORT */
	{
		rf_bw = BW_20;
		ext_ch = EXTCHA_NONE;
		pAd->CommonCfg.CentralChannel = pAd->CommonCfg.Channel;
	}

	bbp_set_bw(pAd, rf_bw);
		
#ifdef DOT11_N_SUPPORT
	DBGPRINT(RT_DEBUG_TRACE, ("%s() : %s, ChannelWidth=%d, Channel=%d, ExtChanOffset=%d(%d) \n",
					__FUNCTION__, ext_str[ext_ch],
					pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth,
					pAd->CommonCfg.Channel,
					pAd->CommonCfg.RegTransmitSetting.field.EXTCHA,
					pAd->CommonCfg.AddHTInfo.AddHtInfo.ExtChanOffset));
#endif /* DOT11_N_SUPPORT */
}

/*Nobody uses it currently*/

static void mt7603_switch_channel(RTMP_ADAPTER *pAd, UCHAR channel, BOOLEAN scan)
{


	if (pAd->CommonCfg.BBPCurrentBW == BW_20)
	{
		CmdChannelSwitch(pAd, channel, channel, BW_20,
								pAd->CommonCfg.TxStream, pAd->CommonCfg.RxStream);
		
		CmdSetTxPowerCtrl(pAd, channel); 			
	}
	else
	{
		CmdChannelSwitch(pAd, pAd->CommonCfg.Channel, channel, pAd->CommonCfg.BBPCurrentBW,
								pAd->CommonCfg.TxStream, pAd->CommonCfg.RxStream);
		
		CmdSetTxPowerCtrl(pAd, channel); 			
	}
							
	/* Channel latch */
	pAd->LatchRfRegs.Channel = channel;


	DBGPRINT(RT_DEBUG_TRACE,
			("%s(): Switch to Ch#%d(%dT%dR), BBP_BW=%d\n",
			__FUNCTION__,
			channel,
			pAd->CommonCfg.TxStream,
			pAd->CommonCfg.RxStream,
			pAd->CommonCfg.BBPCurrentBW));
}


/*
    Init TxD short format template which will copy by PSE-Client to LMAC
*/
static INT asic_set_tmac_info_template(RTMP_ADAPTER *pAd)
{
		UINT32 dw[5];
		TMAC_TXD_2 *dw2 = (TMAC_TXD_2 *)(&dw[0]);
		TMAC_TXD_3 *dw3 = (TMAC_TXD_3 *)(&dw[1]);
		//TMAC_TXD_4 *dw4 = (TMAC_TXD_4 *)(&dw[2]);
		TMAC_TXD_5 *dw5 = (TMAC_TXD_5 *)(&dw[3]);
		//TMAC_TXD_6 *dw6 = (TMAC_TXD_6 *)(&dw[4]);

		NdisZeroMemory((UCHAR *)(&dw[0]), sizeof(dw));

		dw2->htc_vld = 0;
		dw2->frag = 0;
		dw2->max_tx_time = 0;
		dw2->fix_rate = 0;

		dw3->remain_tx_cnt = MT_TX_SHORT_RETRY;
		dw3->sn_vld = 0;
		dw3->pn_vld = 0;

		dw5->pid = PID_DATA_AMPDU;
		dw5->tx_status_fmt = 0;
		dw5->tx_status_2_host = 0; // Disable TxS
		dw5->bar_sn_ctrl = 0; //HW
		dw5->pwr_mgmt= TMI_PM_BIT_CFG_BY_SW;

#ifdef RTMP_PCI_SUPPORT
// TODO: shaing, for MT7628, may need to change this as RTMP_MAC_PCI

        RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, 0x80080000);


        /* For short format */
		RTMP_IO_WRITE32(pAd, 0xc0040, dw[0]);
		RTMP_IO_WRITE32(pAd, 0xc0044, dw[1]);
		RTMP_IO_WRITE32(pAd, 0xc0048, dw[2]);
		RTMP_IO_WRITE32(pAd, 0xc004c, dw[3]);
		RTMP_IO_WRITE32(pAd, 0xc0050, dw[4]);


// TODO: shaing, for MT7628, may need to change this as RTMP_MAC_PCI
	// After change the Tx Padding CR of PCI-E Client, we need to re-map for PSE region
	RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, MT_PSE_BASE_ADDR);

#endif /* RTMP_PCI_SUPPORT */


	return TRUE;
}


static VOID mt7603_init_mac_cr(RTMP_ADAPTER *pAd)
{
	UINT32 mac_val;

	DBGPRINT(RT_DEBUG_OFF, ("%s()-->\n", __FUNCTION__));

	/* Preparation of TxD DW2~DW6 when we need run 3DW format */
	asic_set_tmac_info_template(pAd);
	
	/* A-MPDU BA WinSize control */
	RTMP_IO_READ32(pAd, AGG_AWSCR, &mac_val);
	mac_val &= ~WINSIZE0_MASK;
	mac_val |= WINSIZE0(4);
	mac_val &= ~WINSIZE1_MASK;
	mac_val |= WINSIZE1(5);
	mac_val &= ~WINSIZE2_MASK;
	mac_val |= WINSIZE2(8);
	mac_val &= ~WINSIZE3_MASK;
	mac_val |= WINSIZE3(10);
	RTMP_IO_WRITE32(pAd, AGG_AWSCR, mac_val);

	RTMP_IO_READ32(pAd, AGG_AWSCR1, &mac_val);
	mac_val &= ~WINSIZE4_MASK;
	mac_val |= WINSIZE4(16);
	mac_val &= ~WINSIZE5_MASK;
	mac_val |= WINSIZE5(20);
	mac_val &= ~WINSIZE6_MASK;
	mac_val |= WINSIZE6(21);
	mac_val &= ~WINSIZE7_MASK;
	mac_val |= WINSIZE7(42);
 	RTMP_IO_WRITE32(pAd, AGG_AWSCR1, mac_val);
	
	/* A-MPDU Agg limit control */
	RTMP_IO_READ32(pAd, AGG_AALCR, &mac_val);
	mac_val &= ~AC0_AGG_LIMIT_MASK;
	mac_val |= AC0_AGG_LIMIT(24); //modify from 21, 20171207
	mac_val &= ~AC1_AGG_LIMIT_MASK;
	mac_val |= AC1_AGG_LIMIT(24);
	mac_val &= ~AC2_AGG_LIMIT_MASK;
	mac_val |= AC2_AGG_LIMIT(24);
	mac_val &= ~AC3_AGG_LIMIT_MASK;
	mac_val |= AC3_AGG_LIMIT(24);
	RTMP_IO_WRITE32(pAd, AGG_AALCR, mac_val);
	
	RTMP_IO_READ32(pAd, AGG_AALCR1, &mac_val);
	mac_val &= ~AC10_AGG_LIMIT_MASK;
	mac_val |= AC10_AGG_LIMIT(24);
	mac_val &= ~AC11_AGG_LIMIT_MASK;
	mac_val |= AC11_AGG_LIMIT(24);
	mac_val &= ~AC12_AGG_LIMIT_MASK;
	mac_val |= AC12_AGG_LIMIT(24);
	mac_val &= ~AC13_AGG_LIMIT_MASK;
	mac_val |= AC13_AGG_LIMIT(24);
	RTMP_IO_WRITE32(pAd, AGG_AALCR1, mac_val);
	
	/* Vector report queue setting */
	RTMP_IO_READ32(pAd, DMA_VCFR0, &mac_val);
	mac_val |= BIT13;
	RTMP_IO_WRITE32(pAd, DMA_VCFR0, mac_val);

	/* TMR report queue setting */
	RTMP_IO_READ32(pAd, DMA_TMCFR0, &mac_val);
	mac_val |= BIT13;//TMR report send to HIF q1.
        mac_val = mac_val & ~(BIT0);
        mac_val = mac_val & ~(BIT1);
	RTMP_IO_WRITE32(pAd, DMA_TMCFR0, mac_val);

    RTMP_IO_READ32(pAd, RMAC_TMR_PA, &mac_val);
    mac_val = mac_val & ~BIT31;
    RTMP_IO_WRITE32(pAd, RMAC_TMR_PA, mac_val);
	/* Configure all rx packets to HIF, except WOL2M packet */
	RTMP_IO_READ32(pAd, DMA_RCFR0, &mac_val);
	mac_val = 0x00010000; // drop duplicate
	mac_val |= 0xc0200000; // receive BA/CF_End/Ack/RTS/CTS/CTRL_RSVED
	if (pAd->rx_pspoll_filter)
		mac_val |= 0x00000008; //Non-BAR Control frame to MCU
	RTMP_IO_WRITE32(pAd, DMA_RCFR0, mac_val);

	/* Configure Rx Vectors report to HIF */
	RTMP_IO_READ32(pAd, DMA_VCFR0, &mac_val);
	mac_val &= (~0x1); // To HIF
	mac_val |= 0x2000; // RxRing 1
	RTMP_IO_WRITE32(pAd, DMA_VCFR0, mac_val);

    /* RMAC dropping criteria for max/min recv. packet length */
    RTMP_IO_READ32(pAd, RMAC_RMACDR, &mac_val);
    mac_val |= SELECT_RXMAXLEN_20BIT;
    RTMP_IO_WRITE32(pAd, RMAC_RMACDR, mac_val);
	RTMP_IO_READ32(pAd, RMAC_MAXMINLEN, &mac_val);
    mac_val &= ~RMAC_DROP_MAX_LEN_MASK;
    mac_val |= RMAC_DROP_MAX_LEN;
	RTMP_IO_WRITE32(pAd, RMAC_MAXMINLEN, mac_val);



	/* Enable RX Group to HIF */
	AsicSetRxGroup(pAd, HIF_PORT, RXS_GROUP1|RXS_GROUP2|RXS_GROUP3, TRUE);
	AsicSetRxGroup(pAd, MCU_PORT, RXS_GROUP1|RXS_GROUP2|RXS_GROUP3, TRUE);
	
	/* AMPDU BAR setting */
	/* Enable HW BAR feature */
	AsicSetBARTxCntLimit(pAd, TRUE, 1); 

	/* RTS Retry setting */
	AsicSetRTSTxCntLimit(pAd, TRUE, MT_RTS_RETRY); 

	/* Configure the BAR rate setting */
	RTMP_IO_READ32(pAd, AGG_ACR, &mac_val);
	mac_val &= (~0xfff00000);
	mac_val &= ~(AGG_ACR_AMPDU_NO_BA_AR_RULE_MASK|AMPDU_NO_BA_RULE);
	mac_val |= AGG_ACR_AMPDU_NO_BA_AR_RULE_MASK;
	RTMP_IO_WRITE32(pAd, AGG_ACR, mac_val);

	/* AMPDU Statistics Range Control setting
		0 < agg_cnt - 1 <= range_cr(0),				=> 1
		range_cr(0) < agg_cnt - 1 <= range_cr(4),		=> 2~5
		range_cr(4) < agg_cnt - 1 <= range_cr(14),	=> 6~15
		range_cr(14) < agg_cnt - 1,					=> 16~
	*/
	RTMP_IO_READ32(pAd, AGG_ASRCR, &mac_val);
	mac_val =  (0 << 0) | (4 << 8) | (14 << 16);
	RTMP_IO_WRITE32(pAd, AGG_ASRCR, mac_val);

	// Enable MIB counters
	RTMP_IO_WRITE32(pAd, MIB_MSCR, 0x7fffffff);
	RTMP_IO_WRITE32(pAd, MIB_MPBSCR, 0xffffffff);
	

	/* Throughput patch */
	RTMP_IO_READ32(pAd, TMAC_TRCR, &mac_val);
	mac_val &= 0x0fffffff;
	mac_val |= 0x80000000;
	RTMP_IO_WRITE32(pAd, TMAC_TRCR, mac_val);

	RTMP_IO_READ32(pAd, WTBL_OFF_RMVTCR, &mac_val);
	/* RCPI include ACK and Data */
	mac_val |= RX_MV_MODE;
	RTMP_IO_WRITE32(pAd, WTBL_OFF_RMVTCR, mac_val);

#ifdef MT7603_FPGA
	// enable MAC2MAC mode
	RTMP_IO_READ32(pAd, RMAC_MISC, &mac_val);
	mac_val |= BIT18;
	RTMP_IO_WRITE32(pAd, RMAC_MISC, mac_val);
#endif /* MT7603_FPGA */

	/* Turn on RX RIFS Mode */
	RTMP_IO_READ32(pAd, TMAC_TCR, &mac_val);
	mac_val |= RX_RIFS_MODE;
	RTMP_IO_WRITE32(pAd, TMAC_TCR, mac_val);
	    
	/* IOT issue with Realtek at CCK mode */
	mac_val = 0x003000E7;
	RTMP_IO_WRITE32(pAd, TMAC_CDTR, mac_val);

	/* IOT issue with Linksys WUSB6300. Cannot receive BA after TX finish */
	mac_val = 0x4;
	RTMP_IO_WRITE32(pAd, TMAC_RRCR, mac_val);

	/* send RTS/CTS if agg size >= 2 */
	RTMP_IO_READ32(pAd, AGG_PCR1, &mac_val);
	mac_val &= ~RTS_PKT_NUM_THRESHOLD_MASK;
	mac_val |= RTS_PKT_NUM_THRESHOLD(3);
	RTMP_IO_WRITE32(pAd, AGG_PCR1, mac_val);

	/* When WAPI + RDG, don't mask ORDER bit  */
	RTMP_IO_READ32(pAd, SEC_SCR, &mac_val);
	mac_val &= 0xfffbfffc; //zero bit [18] for ICV error issue.
	RTMP_IO_WRITE32(pAd, SEC_SCR, mac_val);

	/* Enable Spatial Extension for RTS/CTS  */
	RTMP_IO_READ32(pAd, TMAC_PCR, &mac_val);
	mac_val |= PTEC_SPE_EN;
	RTMP_IO_WRITE32(pAd, TMAC_PCR, mac_val);

	/* Enable Spatial Extension for ACK/BA/CTS */
	RTMP_IO_READ32(pAd, TMAC_B0BRR0, &mac_val);
	mac_val |= BSSID00_RESP_SPE_EN;
	RTMP_IO_WRITE32(pAd, TMAC_B0BRR0, mac_val);


	AsicSetTxSClassifyFilter(pAd, TXS2HOST, TXS2H_QID1, TXS2HOST_AGGNUMS, 0x00); 
	AsicSetTxSClassifyFilter(pAd, TXS2MCU, TXS2M_QID0, TXS2MCU_AGGNUMS, 0x00);
}


static VOID MT7603BBPInit(RTMP_ADAPTER *pAd)
{
    return;
}

static void mt7603_init_rf_cr(RTMP_ADAPTER *ad)
{
	return;
}

static UINT8 mt7603_txpwr_chlist[] = {
	1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
};


int mt7603_read_chl_pwr(RTMP_ADAPTER *pAd)
{
	UINT32 i, choffset;
	struct MT_TX_PWR_CAP *cap = &pAd->chipCap.MTTxPwrCap;
	USHORT Value;
	
	mt7603_get_tx_pwr_info(pAd);

	DBGPRINT(RT_DEBUG_TRACE, ("%s()--->\n", __FUNCTION__));
	
	for (i = 0; i < sizeof(mt7603_txpwr_chlist); i++) {
		pAd->TxPower[i].Channel = mt7603_txpwr_chlist[i];
		pAd->TxPower[i].Power = TX_TARGET_PWR_DEFAULT_VALUE;
		pAd->TxPower[i].Power2 = TX_TARGET_PWR_DEFAULT_VALUE;
	}

	for (i = 0; i < 14; i++) {
		pAd->TxPower[i].Power = cap->tx_0_target_pwr_g_band;
		pAd->TxPower[i].Power2 = cap->tx_1_target_pwr_g_band;
	}

	choffset = 14;

	/* 4. Print and Debug*/
	for (i = 0; i < choffset; i++)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("E2PROM: TxPower[%03d], Channel=%d, Power[Tx0:%d, Tx1:%d]\n",
					i, pAd->TxPower[i].Channel, pAd->TxPower[i].Power, pAd->TxPower[i].Power2 ));
	}
	
	/* check PA type combination */
	RT28xx_EEPROM_READ16(pAd, EEPROM_NIC1_OFFSET, Value);
	cap->pa_type = GET_PA_TYPE(Value);

	return TRUE;
}


/* Read power per rate */
void mt7603_get_tx_pwr_per_rate(RTMP_ADAPTER *pAd)
{
    BOOLEAN is_empty = 0;
    UINT16 value = 0;
	struct MT_TX_PWR_CAP *cap = &pAd->chipCap.MTTxPwrCap;

    /* G Band tx power for CCK 1M/2M, 5.5M/11M */
    is_empty = RT28xx_EEPROM_READ16(pAd, TX_PWR_CCK_1_2M, value);
    if (is_empty) {
        cap->tx_pwr_cck_1_2 = 0;
        cap->tx_pwr_cck_5_11 = 0;
    } else {
        /* CCK 1M/2M */
        if (value & TX_PWR_CCK_1_2M_EN) {
            if (value & TX_PWR_CCK_1_2M_SIGN) {
                cap->tx_pwr_cck_1_2 = (value & TX_PWR_CCK_1_2M_MASK);
            } else {
                cap->tx_pwr_cck_1_2 = -(value & TX_PWR_CCK_1_2M_MASK);
            }
        } else {
            cap->tx_pwr_cck_1_2 = 0;
        }

        /* CCK 5.5M/11M */
        if (value & TX_PWR_CCK_5_11M_EN) {
            if (value & TX_PWR_CCK_5_11M_SIGN) {
                cap->tx_pwr_cck_5_11 = ((value & TX_PWR_CCK_5_11M_MASK) >> 8);
            } else {
                cap->tx_pwr_cck_5_11 = -((value & TX_PWR_CCK_5_11M_MASK) >> 8);
            }
        } else {
            cap->tx_pwr_cck_5_11 = 0;
        }
    }

    DBGPRINT(RT_DEBUG_TRACE, ("tx_pwr_cck_1_2 = %d\n", cap->tx_pwr_cck_1_2));
    DBGPRINT(RT_DEBUG_TRACE, ("tx_pwr_cck_5_11 = %d\n", cap->tx_pwr_cck_5_11));

    /* G Band tx power for OFDM 6M/9M, 12M/18M, 24M/36M, 48M/54M */
    is_empty = RT28xx_EEPROM_READ16(pAd, TX_PWR_G_BAND_OFDM_6_9M, value);
    if (is_empty) {
        cap->tx_pwr_g_band_ofdm_6_9 = 0;
        cap->tx_pwr_g_band_ofdm_12_18 = 0;
    } else {
        /* OFDM 6M/9M */
        if (value & TX_PWR_G_BAND_OFDM_6_9M_EN) {
            if (value & TX_PWR_G_BAND_OFDM_6_9M_SIGN) {
                cap->tx_pwr_g_band_ofdm_6_9 = (value & TX_PWR_G_BAND_OFDM_6_9M_MASK);
            } else {
                cap->tx_pwr_g_band_ofdm_6_9 = -(value & TX_PWR_G_BAND_OFDM_6_9M_MASK);
            }
        } else {
            cap->tx_pwr_g_band_ofdm_6_9 = 0;
        }

        /* OFDM 12M/18M */
        if (value & TX_PWR_G_BAND_OFDM_12_18M_EN) {
            if (value & TX_PWR_G_BAND_OFDM_12_18M_SIGN) {
                cap->tx_pwr_g_band_ofdm_12_18 = ((value & TX_PWR_G_BAND_OFDM_12_18M_MASK) >> 8);
            } else {
                cap->tx_pwr_g_band_ofdm_12_18 = -((value & TX_PWR_G_BAND_OFDM_12_18M_MASK) >> 8);
            }
        } else {
            cap->tx_pwr_g_band_ofdm_12_18 = 0;
        }
    }
    DBGPRINT(RT_DEBUG_TRACE, ("tx_pwr_g_band_ofdm_6_9 = %d\n", cap->tx_pwr_g_band_ofdm_6_9));
    DBGPRINT(RT_DEBUG_TRACE, ("tx_pwr_g_band_ofdm_12_18 = %d\n", cap->tx_pwr_g_band_ofdm_12_18));

    is_empty = RT28xx_EEPROM_READ16(pAd, TX_PWR_G_BAND_OFDM_24_36M, value);
    if (is_empty) {
        cap->tx_pwr_g_band_ofdm_24_36 = 0;
        cap->tx_pwr_g_band_ofdm_48= 0;
    } else {
        /* OFDM 24M/36M */
        if (value & TX_PWR_G_BAND_OFDM_24_36M_EN) {
            if (value & TX_PWR_G_BAND_OFDM_24_36M_SIGN) {
                cap->tx_pwr_g_band_ofdm_24_36 = (value & TX_PWR_G_BAND_OFDM_24_36M_MASK);
            } else {
                cap->tx_pwr_g_band_ofdm_24_36 = -(value & TX_PWR_G_BAND_OFDM_24_36M_MASK);
            }
        } else {
            cap->tx_pwr_g_band_ofdm_24_36 = 0;
        }

        /* OFDM 48M */
        if (value & TX_PWR_G_BAND_OFDM_48M_EN) {
            if (value & TX_PWR_G_BAND_OFDM_48M_SIGN) {
                cap->tx_pwr_g_band_ofdm_48 = ((value & TX_PWR_G_BAND_OFDM_48M_MASK) >> 8);
            } else {
                cap->tx_pwr_g_band_ofdm_48 = -((value & TX_PWR_G_BAND_OFDM_48M_MASK) >> 8);
            }
        } else {
            cap->tx_pwr_g_band_ofdm_48 = 0;
        }
    }
    DBGPRINT(RT_DEBUG_TRACE, ("tx_pwr_g_band_ofdm_24_36 = %d\n", cap->tx_pwr_g_band_ofdm_24_36));
    DBGPRINT(RT_DEBUG_TRACE, ("tx_pwr_g_band_ofdm_48 = %d\n", cap->tx_pwr_g_band_ofdm_48));

    is_empty = RT28xx_EEPROM_READ16(pAd, TX_PWR_G_BAND_OFDM_54M, value);
    if (is_empty) {
        cap->tx_pwr_g_band_ofdm_54 = 0;
        cap->tx_pwr_ht_bpsk_mcs_0_8 = 0;
    } else {
        /* OFDM 54M */
        if (value & TX_PWR_G_BAND_OFDM_54M_EN) {
            if (value & TX_PWR_G_BAND_OFDM_54M_SIGN) {
                cap->tx_pwr_g_band_ofdm_54 = (value & TX_PWR_G_BAND_OFDM_54M_MASK);
            } else {
                cap->tx_pwr_g_band_ofdm_54 = -(value & TX_PWR_G_BAND_OFDM_54M_MASK);
            }
        } else {
            cap->tx_pwr_g_band_ofdm_54 = 0;
        }

        /* HT MCS_0, MCS_8 */
        if (value & TX_PWR_HT_BPSK_MCS_0_8_EN) {
            if (value & TX_PWR_HT_BPSK_MCS_0_8_SIGN) {
                cap->tx_pwr_ht_bpsk_mcs_0_8 = ((value & TX_PWR_HT_BPSK_MCS_0_8_MASK) >> 8);
            } else {
                cap->tx_pwr_ht_bpsk_mcs_0_8 = -((value & TX_PWR_HT_BPSK_MCS_0_8_MASK) >> 8);
            }
        } else {
            cap->tx_pwr_ht_bpsk_mcs_0_8 = 0;
        }    }
    DBGPRINT(RT_DEBUG_TRACE, ("tx_pwr_g_band_ofdm_54 = %d\n", cap->tx_pwr_g_band_ofdm_54));
    DBGPRINT(RT_DEBUG_TRACE, ("tx_pwr_ht_bpsk_mcs_0_8 = %d\n", cap->tx_pwr_ht_bpsk_mcs_0_8));

    is_empty = RT28xx_EEPROM_READ16(pAd, TX_PWR_HT_BPSK_MCS_32, value);
    if (is_empty) {
        cap->tx_pwr_ht_bpsk_mcs_32 = 0;
        cap->tx_pwr_ht_qpsk_mcs_1_2_9_10 = 0;
    } else {
        /* HT MCS_0, MCS_8 */
        if (value & TX_PWR_HT_BPSK_MCS_32_EN) {
            if (value & TX_PWR_HT_BPSK_MCS_32_SIGN) {
                cap->tx_pwr_ht_bpsk_mcs_32 = (value & TX_PWR_HT_BPSK_MCS_32_MASK);
            } else {
                cap->tx_pwr_ht_bpsk_mcs_32 = -(value & TX_PWR_HT_BPSK_MCS_32_MASK);
            }
        } else {
            cap->tx_pwr_ht_bpsk_mcs_32 = 0;
        }

        /* HT MCS_1, MCS_2, MCS_9, MCS_10 */
        if (value & TX_PWR_HT_QPSK_MCS_1_2_9_10_EN) {
            if (value & TX_PWR_HT_QPSK_MCS_1_2_9_10_SIGN) {
                cap->tx_pwr_ht_qpsk_mcs_1_2_9_10 = ((value & TX_PWR_HT_QPSK_MCS_1_2_9_10_MASK) >> 8);
            } else {
                cap->tx_pwr_ht_qpsk_mcs_1_2_9_10 = -((value & TX_PWR_HT_QPSK_MCS_1_2_9_10_MASK) >> 8);
            }
        } else {
            cap->tx_pwr_ht_qpsk_mcs_1_2_9_10 = 0;
        }
    }

    DBGPRINT(RT_DEBUG_TRACE, ("tx_pwr_ht_bpsk_mcs_32 = %d\n", cap->tx_pwr_ht_bpsk_mcs_32));
    DBGPRINT(RT_DEBUG_TRACE, ("tx_pwr_ht_qpsk_mcs_1_2_9_10 = %d\n", cap->tx_pwr_ht_qpsk_mcs_1_2_9_10));

    is_empty = RT28xx_EEPROM_READ16(pAd, TX_PWR_HT_16QAM_MCS_3_4_11_12, value);
    if (is_empty) {
        cap->tx_pwr_ht_16qam_mcs_3_4_11_12 = 0;
        cap->tx_pwr_ht_64qam_mcs_5_13 = 0;
    } else {
        /* HT MCS_3, MCS_4, MCS_11, MCS_12 */
        if (value & TX_PWR_HT_16QAM_MCS_3_4_11_12_EN) {
            if (value & TX_PWR_HT_16QAM_MCS_3_4_11_12_SIGN) {
                cap->tx_pwr_ht_16qam_mcs_3_4_11_12 = (value & TX_PWR_HT_16QAM_MCS_3_4_11_12_MASK);
            } else {
                cap->tx_pwr_ht_16qam_mcs_3_4_11_12 = -(value & TX_PWR_HT_16QAM_MCS_3_4_11_12_MASK);
            }
        } else {
            cap->tx_pwr_ht_16qam_mcs_3_4_11_12 = 0;
        }

        /* HT MCS_5, MCS_13 */
        if (value & TX_PWR_HT_64QAM_MCS_5_13_EN) {
            if (value & TX_PWR_HT_64QAM_MCS_5_13_SIGN) {
                cap->tx_pwr_ht_64qam_mcs_5_13 = ((value & TX_PWR_HT_64QAM_MCS_5_13_MASK) >> 8);
            } else {
                cap->tx_pwr_ht_64qam_mcs_5_13 = -((value & TX_PWR_HT_64QAM_MCS_5_13_MASK) >> 8);
            }
        } else {
            cap->tx_pwr_ht_64qam_mcs_5_13 = 0;
        }
    }
    DBGPRINT(RT_DEBUG_TRACE, ("tx_pwr_ht_16qam_mcs_3_4_11_12 = %d\n", cap->tx_pwr_ht_16qam_mcs_3_4_11_12));
    DBGPRINT(RT_DEBUG_TRACE, ("tx_pwr_ht_64qam_mcs_5_13 = %d\n", cap->tx_pwr_ht_64qam_mcs_5_13));

    is_empty = RT28xx_EEPROM_READ16(pAd, TX_PWR_HT_64QAM_MCS_6_14, value);
    if (is_empty) {
        cap->tx_pwr_ht_64qam_mcs_6_14 = 0;
        cap->tx_pwr_ht_64qam_mcs_7_15 = 0;
    } else {
        /* HT MCS_6, MCS_14 */
        if (value & TX_PWR_HT_64QAM_MCS_6_14_EN) {
            if (value & TX_PWR_HT_64QAM_MCS_6_14_SIGN) {
                cap->tx_pwr_ht_64qam_mcs_6_14 = (value & TX_PWR_HT_64QAM_MCS_6_14_MASK);
            } else {
                cap->tx_pwr_ht_64qam_mcs_6_14 = -(value & TX_PWR_HT_64QAM_MCS_6_14_MASK);
            }
        } else {
            cap->tx_pwr_ht_64qam_mcs_6_14 = 0;
        }

        /* HT MCS_7, MCS_15 */
        if (value & TX_PWR_HT_64QAM_MCS_7_15_EN) {
            if (value & TX_PWR_HT_64QAM_MCS_7_15_SIGN) {
                cap->tx_pwr_ht_64qam_mcs_7_15 = ((value & TX_PWR_HT_64QAM_MCS_7_15_MASK) >> 8);
            } else {
                cap->tx_pwr_ht_64qam_mcs_7_15 = -((value & TX_PWR_HT_64QAM_MCS_7_15_MASK) >> 8);
            }
        } else {
            cap->tx_pwr_ht_64qam_mcs_7_15 = 0;
        }
    }
    DBGPRINT(RT_DEBUG_TRACE, ("tx_pwr_ht_64qam_mcs_6_14 = %d\n", cap->tx_pwr_ht_64qam_mcs_6_14));
    DBGPRINT(RT_DEBUG_TRACE, ("tx_pwr_ht_64qam_mcs_7_15 = %d\n", cap->tx_pwr_ht_64qam_mcs_7_15));

	return;
}


void mt7603_get_tx_pwr_info(RTMP_ADAPTER *pAd)
{
    bool is_empty = 0;
    UINT16 value = 0;
	struct MT_TX_PWR_CAP *cap = &pAd->chipCap.MTTxPwrCap;

    /* Read 20/40 BW delta */
    is_empty = RT28xx_EEPROM_READ16(pAd, G_BAND_20_40_BW_PWR_DELTA, value);

	if (is_empty) {
        cap->delta_tx_pwr_bw40_g_band = 0x0;
    } else {
        /* G Band */
        if (value & G_BAND_20_40_BW_PWR_DELTA_EN) {
            if (value & G_BAND_20_40_BW_PWR_DELTA_SIGN) {
                /* bit[0..5] tx power delta value */
                cap->delta_tx_pwr_bw40_g_band = (value & G_BAND_20_40_BW_PWR_DELTA_MASK);
            } else {
                cap->delta_tx_pwr_bw40_g_band = -(value & G_BAND_20_40_BW_PWR_DELTA_MASK);
            }
        } else {
            cap->delta_tx_pwr_bw40_g_band = 0x0;
        }
    }
    DBGPRINT(RT_DEBUG_TRACE, ("delta_tx_pwr_bw40_g_band = %d\n", cap->delta_tx_pwr_bw40_g_band));

    /////////////////// Tx0 //////////////////////////
    /* Read TSSI slope/offset for TSSI compensation */
    is_empty = RT28xx_EEPROM_READ16(pAd, TX0_G_BAND_TSSI_SLOPE, value);

    cap->tssi_0_slope_g_band =
        (is_empty) ? TSSI_0_SLOPE_G_BAND_DEFAULT_VALUE : (value & TX0_G_BAND_TSSI_SLOPE_MASK);

    cap->tssi_0_offset_g_band =
        (is_empty) ? TSSI_0_OFFSET_G_BAND_DEFAULT_VALUE : ((value & TX0_G_BAND_TSSI_OFFSET_MASK) >> 8);

    DBGPRINT(RT_DEBUG_TRACE, ("tssi_0_slope_g_band = %d\n", cap->tssi_0_slope_g_band));
    DBGPRINT(RT_DEBUG_TRACE, ("tssi_0_offset_g_band = %d\n", cap->tssi_0_offset_g_band));
    /* Read 54M target power */
    is_empty = RT28xx_EEPROM_READ16(pAd, TX0_G_BAND_TARGET_PWR, value);

    cap->tx_0_target_pwr_g_band =
        (is_empty) ? TX_TARGET_PWR_DEFAULT_VALUE : (value & TX0_G_BAND_TARGET_PWR_MASK);

    DBGPRINT(RT_DEBUG_TRACE, ("tssi_0_target_pwr_g_band = %d\n", cap->tx_0_target_pwr_g_band));

    /* Read power offset (channel delta) */
    if (is_empty) {
        cap->tx_0_chl_pwr_delta_g_band[G_BAND_LOW] = 0x0;
    } else {
        /* tx power offset LOW */
        if (value & TX0_G_BAND_CHL_PWR_DELTA_LOW_EN) {
            if (value & TX0_G_BAND_CHL_PWR_DELTA_LOW_SIGN) {
                cap->tx_0_chl_pwr_delta_g_band[G_BAND_LOW] = ((value & TX0_G_BAND_CHL_PWR_DELTA_LOW_MASK) >> 8);
            } else {
                cap->tx_0_chl_pwr_delta_g_band[G_BAND_LOW] = -((value & TX0_G_BAND_CHL_PWR_DELTA_LOW_MASK) >> 8);
            }
        } else {
            cap->tx_0_chl_pwr_delta_g_band[G_BAND_LOW] = 0x0;
        }
    }
    DBGPRINT(RT_DEBUG_TRACE, ("tx_0_chl_pwr_delta_g_band[G_BAND_LOW] = %d\n", cap->tx_0_chl_pwr_delta_g_band[G_BAND_LOW]));

    is_empty = RT28xx_EEPROM_READ16(pAd, TX0_G_BAND_CHL_PWR_DELTA_MID, value);

    if (is_empty) {
        cap->tx_0_chl_pwr_delta_g_band[G_BAND_MID] = 0x0;
        cap->tx_0_chl_pwr_delta_g_band[G_BAND_HI] = 0x0;
    } else {
        /* tx power offset MID */
        if (value & TX0_G_BAND_CHL_PWR_DELTA_MID_EN) {
            if (value & TX0_G_BAND_CHL_PWR_DELTA_MID_SIGN)
                cap->tx_0_chl_pwr_delta_g_band[G_BAND_MID] = (value & TX0_G_BAND_CHL_PWR_DELTA_MID_MASK);
            else
                cap->tx_0_chl_pwr_delta_g_band[G_BAND_MID] = -(value & TX0_G_BAND_CHL_PWR_DELTA_MID_MASK);
        } else {
            cap->tx_0_chl_pwr_delta_g_band[G_BAND_MID] = 0x0;
        }
        /* tx power offset HIGH */
        if (value & TX0_G_BAND_CHL_PWR_DELTA_HI_EN) {
            if (value & TX0_G_BAND_CHL_PWR_DELTA_HI_SIGN)
                cap->tx_0_chl_pwr_delta_g_band[G_BAND_HI] = ((value & TX0_G_BAND_CHL_PWR_DELTA_HI_MASK) >> 8);
            else
                cap->tx_0_chl_pwr_delta_g_band[G_BAND_HI] = -((value & TX0_G_BAND_CHL_PWR_DELTA_HI_MASK) >> 8);
        } else {
            cap->tx_0_chl_pwr_delta_g_band[G_BAND_HI] = 0x0;
        }
    }
    DBGPRINT(RT_DEBUG_TRACE, ("tx_0_chl_pwr_delta_g_band[G_BAND_MID] = %d\n", cap->tx_0_chl_pwr_delta_g_band[G_BAND_MID]));
    DBGPRINT(RT_DEBUG_TRACE, ("tx_0_chl_pwr_delta_g_band[G_BAND_HI] = %d\n", cap->tx_0_chl_pwr_delta_g_band[G_BAND_HI]));

    /////////////////// Tx1 //////////////////////////
    /* Read TSSI slope/offset for TSSI compensation */
    is_empty = RT28xx_EEPROM_READ16(pAd, TX1_G_BAND_TSSI_SLOPE, value);

    cap->tssi_1_slope_g_band = (is_empty) ? TSSI_1_SLOPE_G_BAND_DEFAULT_VALUE : (value & TX1_G_BAND_TSSI_SLOPE_MASK);

    cap->tssi_1_offset_g_band = (is_empty) ? TSSI_1_OFFSET_G_BAND_DEFAULT_VALUE : ((value & TX1_G_BAND_TSSI_OFFSET_MASK) >> 8);

    DBGPRINT(RT_DEBUG_TRACE, ("tssi_1_slope_g_band = %d\n", cap->tssi_1_slope_g_band));
    DBGPRINT(RT_DEBUG_TRACE, ("tssi_1_offset_g_band = %d\n", cap->tssi_1_offset_g_band));

    /* Read 54M target power */
    is_empty = RT28xx_EEPROM_READ16(pAd, TX1_G_BAND_TARGET_PWR, value);

    cap->tx_1_target_pwr_g_band = (is_empty) ? TX_TARGET_PWR_DEFAULT_VALUE : (value & TX1_G_BAND_TARGET_PWR_MASK);

    printk ("tssi_1_target_pwr_g_band = %d\n", cap->tx_1_target_pwr_g_band);

    /* Read power offset (channel delta) */
    if (is_empty) {
        cap->tx_1_chl_pwr_delta_g_band[G_BAND_LOW] =  0;
    } else {
        /* tx power offset LOW */
        if (value & TX1_G_BAND_CHL_PWR_DELTA_LOW_EN) {
            if (value & TX1_G_BAND_CHL_PWR_DELTA_LOW_SIGN) {
                cap->tx_1_chl_pwr_delta_g_band[G_BAND_LOW] = ((value & TX1_G_BAND_CHL_PWR_DELTA_LOW_MASK) >> 8);
            } else {
                cap->tx_1_chl_pwr_delta_g_band[G_BAND_LOW] = -((value & TX1_G_BAND_CHL_PWR_DELTA_LOW_MASK) >> 8);
            }
        } else {
            cap->tx_1_chl_pwr_delta_g_band[G_BAND_LOW] = 0;
        }
    }
    DBGPRINT(RT_DEBUG_TRACE, ("tx_1_chl_pwr_delta_g_band[G_BAND_LOW] = %d\n", cap->tx_1_chl_pwr_delta_g_band[G_BAND_LOW]));

    is_empty = RT28xx_EEPROM_READ16(pAd, TX1_G_BAND_CHL_PWR_DELTA_MID, value);
    if (is_empty) {
        cap->tx_1_chl_pwr_delta_g_band[G_BAND_MID] = 0;
        cap->tx_1_chl_pwr_delta_g_band[G_BAND_HI] = 0;
    } else {
        /* tx power offset MID */
        if (value & TX1_G_BAND_CHL_PWR_DELTA_MID_EN) {
            if (value & TX1_G_BAND_CHL_PWR_DELTA_MID_SIGN) {
                cap->tx_1_chl_pwr_delta_g_band[G_BAND_MID] = (value & TX1_G_BAND_CHL_PWR_DELTA_MID_MASK);
            } else {
                cap->tx_1_chl_pwr_delta_g_band[G_BAND_MID] = -(value & TX1_G_BAND_CHL_PWR_DELTA_MID_MASK);
            }
        } else {
            cap->tx_1_chl_pwr_delta_g_band[G_BAND_MID] = 0;
        }
        /* tx power offset HIGH */
        if (value & TX1_G_BAND_CHL_PWR_DELTA_HI_EN) {
            if (value & TX1_G_BAND_CHL_PWR_DELTA_HI_SIGN) {
                cap->tx_1_chl_pwr_delta_g_band[G_BAND_HI] = ((value & TX1_G_BAND_CHL_PWR_DELTA_HI_MASK) >> 8);
            } else {
                cap->tx_1_chl_pwr_delta_g_band[G_BAND_HI] = -((value & TX1_G_BAND_CHL_PWR_DELTA_HI_MASK) >> 8);
            }
        } else {
            cap->tx_1_chl_pwr_delta_g_band[G_BAND_HI] = 0;
        }
    }
    DBGPRINT(RT_DEBUG_TRACE, ("tx_1_chl_pwr_delta_g_band[G_BAND_MID] = %d\n", cap->tx_1_chl_pwr_delta_g_band[G_BAND_MID]));
    DBGPRINT(RT_DEBUG_TRACE, ("tx_1_chl_pwr_delta_g_band[G_BAND_HI] = %d\n", cap->tx_1_chl_pwr_delta_g_band[G_BAND_HI]));

    return ;
}


static VOID mt7603_show_pwr_info(RTMP_ADAPTER *pAd)
{
	struct MT_TX_PWR_CAP *cap = &pAd->chipCap.MTTxPwrCap;
	UINT32 value;

	DBGPRINT(RT_DEBUG_OFF, ("\n===================================\n"));
	DBGPRINT(RT_DEBUG_OFF, ("channel info related to power\n"));
	DBGPRINT(RT_DEBUG_OFF, ("===================================\n"));

	if (pAd->LatchRfRegs.Channel < 14) {
		DBGPRINT(RT_DEBUG_OFF, ("central channel = %d, low_mid_hi = %d\n", pAd->LatchRfRegs.Channel,
							get_low_mid_hi_index(pAd->LatchRfRegs.Channel)));
	}

	DBGPRINT(RT_DEBUG_OFF, ("\n===================================\n"));
	DBGPRINT(RT_DEBUG_OFF, ("channel power(unit: 0.5dbm)\n"));
	DBGPRINT(RT_DEBUG_OFF, ("===================================\n"));
	DBGPRINT(RT_DEBUG_OFF, ("tx_0_target_pwr_g_band = 0x%x\n", cap->tx_0_target_pwr_g_band));
	DBGPRINT(RT_DEBUG_OFF, ("tx_1_target_pwr_g_band = 0x%x\n", cap->tx_1_target_pwr_g_band));

	/* channel power delta */
	DBGPRINT(RT_DEBUG_OFF, ("\n===================================\n"));
	DBGPRINT(RT_DEBUG_OFF, ("channel power delta(unit: 0.5db)\n"));
	DBGPRINT(RT_DEBUG_OFF, ("===================================\n"));
	DBGPRINT(RT_DEBUG_OFF, ("tx_0_chl_pwr_delta_g_band[G_BAND_LOW] = 0x%x\n", cap->tx_0_chl_pwr_delta_g_band[G_BAND_LOW]));
	DBGPRINT(RT_DEBUG_OFF, ("tx_0_chl_pwr_delta_g_band[G_BAND_MID] = 0x%x\n", cap->tx_0_chl_pwr_delta_g_band[G_BAND_MID]));
	DBGPRINT(RT_DEBUG_OFF, ("tx_0_chl_pwr_delta_g_band[G_BAND_HI] = 0x%x\n", cap->tx_0_chl_pwr_delta_g_band[G_BAND_HI]));
	DBGPRINT(RT_DEBUG_OFF, ("tx_1_chl_pwr_delta_g_band[G_BAND_LOW] = 0x%x\n", cap->tx_1_chl_pwr_delta_g_band[G_BAND_LOW]));
	DBGPRINT(RT_DEBUG_OFF, ("tx_1_chl_pwr_delta_g_band[G_BAND_MID] = 0x%x\n", cap->tx_1_chl_pwr_delta_g_band[G_BAND_MID]));
	DBGPRINT(RT_DEBUG_OFF, ("tx_1_chl_pwr_delta_g_band[G_BAND_HI] = 0x%x\n", cap->tx_1_chl_pwr_delta_g_band[G_BAND_HI]));

	/* bw power delta */
	DBGPRINT(RT_DEBUG_OFF, ("\n===================================\n"));
	DBGPRINT(RT_DEBUG_OFF, ("bw power delta(unit: 0.5db)\n"));
	DBGPRINT(RT_DEBUG_OFF, ("===================================\n"));
	DBGPRINT(RT_DEBUG_OFF, ("delta_tx_pwr_bw40_g_band = %d\n", cap->delta_tx_pwr_bw40_g_band));

	/* per-rate power delta */
	DBGPRINT(RT_DEBUG_OFF, ("\n===================================\n"));
	DBGPRINT(RT_DEBUG_OFF, ("per-rate power delta\n"));
	DBGPRINT(RT_DEBUG_OFF, ("===================================\n"));
	DBGPRINT(RT_DEBUG_OFF, ("tx_pwr_cck_1_2 = %d\n", cap->tx_pwr_cck_1_2));
	DBGPRINT(RT_DEBUG_OFF, ("tx_pwr_cck_5_11 = %d\n", cap->tx_pwr_cck_5_11));
	DBGPRINT(RT_DEBUG_OFF, ("tx_pwr_g_band_ofdm_6_9 = %d\n", cap->tx_pwr_g_band_ofdm_6_9));
	DBGPRINT(RT_DEBUG_OFF, ("tx_pwr_g_band_ofdm_12_18 = %d\n", cap->tx_pwr_g_band_ofdm_12_18));
	DBGPRINT(RT_DEBUG_OFF, ("tx_pwr_g_band_ofdm_24_36 = %d\n", cap->tx_pwr_g_band_ofdm_24_36));
	DBGPRINT(RT_DEBUG_OFF, ("tx_pwr_g_band_ofdm_48 = %d\n", cap->tx_pwr_g_band_ofdm_48));
	DBGPRINT(RT_DEBUG_OFF, ("tx_pwr_g_band_ofdm_54 = %d\n", cap->tx_pwr_g_band_ofdm_54));
	DBGPRINT(RT_DEBUG_OFF, ("tx_pwr_ht_bpsk_mcs_32 = %d\n", cap->tx_pwr_ht_bpsk_mcs_32));
	DBGPRINT(RT_DEBUG_OFF, ("tx_pwr_ht_bpsk_mcs_0_8 = %d\n", cap->tx_pwr_ht_bpsk_mcs_0_8));
	DBGPRINT(RT_DEBUG_OFF, ("tx_pwr_ht_qpsk_mcs_1_2_9_10 = %d\n", cap->tx_pwr_ht_qpsk_mcs_1_2_9_10));
	DBGPRINT(RT_DEBUG_OFF, ("tx_pwr_ht_16qam_mcs_3_4_11_12 = %d\n", cap->tx_pwr_ht_16qam_mcs_3_4_11_12));
	DBGPRINT(RT_DEBUG_OFF, ("tx_pwr_ht_64qam_mcs_5_13 = %d\n", cap->tx_pwr_ht_64qam_mcs_5_13));
	DBGPRINT(RT_DEBUG_OFF, ("tx_pwr_ht_64qam_mcs_6_14 = %d\n", cap->tx_pwr_ht_64qam_mcs_6_14));
	DBGPRINT(RT_DEBUG_OFF, ("tx_pwr_ht_64qam_mcs_7_15 = %d\n", cap->tx_pwr_ht_64qam_mcs_7_15));

	/* TMAC POWER INFO */
	DBGPRINT(RT_DEBUG_OFF, ("\n===================================\n"));
	DBGPRINT(RT_DEBUG_OFF, ("per-rate power delta in MAC 0x60130020 ~ 0x60130030\n"));
	DBGPRINT(RT_DEBUG_OFF, ("===================================\n"));
	RTMP_IO_READ32(pAd, TMAC_FP0R0, &value);
	DBGPRINT(RT_DEBUG_OFF, ("TMAC_FP0R0 = 0x%x\n", value));
	RTMP_IO_READ32(pAd, TMAC_FP0R1, &value);
	DBGPRINT(RT_DEBUG_OFF, ("TMAC_FP0R1 = 0x%x\n", value));
	RTMP_IO_READ32(pAd, TMAC_FP0R2, &value);
	DBGPRINT(RT_DEBUG_OFF, ("TMAC_FP0R2 = 0x%x\n", value));
	RTMP_IO_READ32(pAd, TMAC_FP0R3, &value);
	DBGPRINT(RT_DEBUG_OFF, ("TMAC_FP0R3 = 0x%x\n", value));
	RTMP_IO_READ32(pAd, TMAC_FP0R4, &value);
	DBGPRINT(RT_DEBUG_OFF, ("TMAC_FP0R4 = 0x%x\n", value));

	/* TSSI info */
	DBGPRINT(RT_DEBUG_OFF, ("\n===================================\n"));
	DBGPRINT(RT_DEBUG_OFF, ("TSSI info\n"));
	DBGPRINT(RT_DEBUG_OFF, ("===================================\n"));
	DBGPRINT(RT_DEBUG_OFF, ("TSSI enable = %d\n", pAd->chipCap.tssi_enable));
	DBGPRINT(RT_DEBUG_OFF, ("tssi_0_slope_g_band = 0x%x\n", cap->tssi_0_slope_g_band));
	DBGPRINT(RT_DEBUG_OFF, ("tssi_1_slope_g_band = 0x%x\n", cap->tssi_1_slope_g_band));
	DBGPRINT(RT_DEBUG_OFF, ("tssi_0_offset_g_band = 0x%x\n", cap->tssi_0_offset_g_band));
	DBGPRINT(RT_DEBUG_OFF, ("tssi_1_offset_g_band = 0x%x\n", cap->tssi_1_offset_g_band));
}


#ifdef CAL_FREE_IC_SUPPORT
static BOOLEAN mt7603_is_cal_free_ic(RTMP_ADAPTER *pAd)
{
	UINT16 Value = 0;
	BOOLEAN NotValid;

	NotValid = rtmp_ee_efuse_read16(pAd, 0x54, &Value);

	if (NotValid)
		return FALSE;

	if (((Value >> 8) & 0xff) == 0x00)
		return FALSE;

	NotValid = rtmp_ee_efuse_read16(pAd, 0x56, &Value);

	if (NotValid)
		return FALSE;

	if (Value == 0x00)
		return FALSE;

	NotValid = rtmp_ee_efuse_read16(pAd, 0x5c, &Value);

	if (NotValid)
		return FALSE;

	if (Value == 0x00)
		return FALSE;

	NotValid = rtmp_ee_efuse_read16(pAd, 0xf0, &Value);

	if (NotValid)
		return FALSE;

	if ((Value & 0xff) == 0x00)
		return FALSE;

	NotValid = rtmp_ee_efuse_read16(pAd, 0xf4, &Value);

	if (NotValid)
		return FALSE;

	if ((Value & 0xff) == 0x00)
		return FALSE;

	NotValid = rtmp_ee_efuse_read16(pAd, 0xf6, &Value);

	if (NotValid)
		return FALSE;

	if (((Value >> 8) & 0xff) == 0x00)
		return FALSE;

	return TRUE;
}



static VOID mt7603_cal_free_data_get(RTMP_ADAPTER *ad)

{

	UINT16 value;

	DBGPRINT(RT_DEBUG_TRACE, ("%s\n", __FUNCTION__));



	/* 0x55 0x56 0x57 0x5C 0x5D */
	eFuseReadRegisters(ad, A_BAND_EXT_PA_SETTING, 2, &value);
	/*0x55*/
	ad->EEPROMImage[A_BAND_EXT_PA_SETTING + 1] = (value >> 8) & 0xFF;

	eFuseReadRegisters(ad, TX0_G_BAND_TSSI_SLOPE, 2, &value);
	/*0x56,0x57*/

	*(UINT16 *)(&ad->EEPROMImage[TX0_G_BAND_TSSI_SLOPE]) = le2cpu16(value);



	eFuseReadRegisters(ad, TX1_G_BAND_TSSI_SLOPE, 2, &value);

	/*0x5c,0x5d*/
	*(UINT16 *)(&ad->EEPROMImage[TX1_G_BAND_TSSI_SLOPE]) = le2cpu16(value);
		
	eFuseReadRegisters(ad, TX1_G_BAND_TARGET_PWR, 2, &value);


	/* 0xF0 0xF4 0xF7  */

	eFuseReadRegisters(ad, CP_FT_VERSION, 2, &value);
	
	ad->EEPROMImage[CP_FT_VERSION] = value & 0xFF;

	eFuseReadRegisters(ad, XTAL_CALIB_FREQ_OFFSET, 2, &value);

	ad->EEPROMImage[XTAL_CALIB_FREQ_OFFSET]  = value & 0xFF;

	eFuseReadRegisters(ad, XTAL_TRIM_3_COMP, 2, &value);

	ad->EEPROMImage[XTAL_TRIM_3_COMP+1] = (value >> 8) & 0xFF;


}


#endif /* CAL_FREE_IC_SUPPORT */





static void mt7603_antenna_default_reset(
	struct _RTMP_ADAPTER *pAd,
	EEPROM_ANTENNA_STRUC *pAntenna)
{
	pAntenna->word = 0;
	pAd->RfIcType = RFIC_7603;
	pAntenna->field.TxPath = 2;
	pAntenna->field.RxPath = 2;
}




static const RTMP_CHIP_CAP MT7603_ChipCap = {
	.max_nss = 2,
	.TXWISize = sizeof(TMAC_TXD_L), /* 32 */
	.RXWISize = 28,
#ifdef RTMP_MAC_PCI
	.WPDMABurstSIZE = 3,
#endif
	.SnrFormula = SNR_FORMULA4,
	.FlgIsHwWapiSup = TRUE,
	.FlgIsHwAntennaDiversitySup = FALSE,
#ifdef STREAM_MODE_SUPPORT
	.FlgHwStreamMode = FALSE,
#endif
#ifdef FIFO_EXT_SUPPORT
	.FlgHwFifoExtCap = FALSE,
#endif
	.asic_caps = (fASIC_CAP_PMF_ENC | fASIC_CAP_MCS_LUT),
	.phy_caps = (fPHY_CAP_24G | fPHY_CAP_HT),
	.MaxNumOfRfId = MAX_RF_ID,
	.pRFRegTable = NULL,
	.MaxNumOfBbpId = 200,
	.pBBPRegTable = NULL,
	.bbpRegTbSize = 0,
	/* Force MT7603 use MBSSID_MODE2~ MBSSID_MODE6 of ENHANCE_NEW_MBSSID_MODE*/
	.MBSSIDMode = MBSSID_MODE4,
#ifdef RTMP_EFUSE_SUPPORT
	.EFUSE_USAGE_MAP_START = 0x1e0,
	.EFUSE_USAGE_MAP_END = 0x1fc,
	.EFUSE_USAGE_MAP_SIZE = 29,
	.EFUSE_RESERVED_SIZE = 22,	// Cal-Free is 22 free block
#endif
	.EEPROM_DEFAULT_BIN = MT7603_E2PImage,
	.EEPROM_DEFAULT_BIN_SIZE = sizeof(MT7603_E2PImage),
#ifdef CONFIG_ANDES_SUPPORT
	.CmdRspRxRing = RX_RING1,
	.need_load_fw = TRUE,
	.DownLoadType = DownLoadTypeA,
#endif
	.MCUType = ANDES,
	.cmd_header_len = sizeof(FW_TXD),
#ifdef RTMP_PCI_SUPPORT
	.cmd_padding_len = 0,
#endif
#ifdef MT7603_E1
	.fw_header_image = MT7603_FirmwareImage,
	.fw_len = sizeof(MT7603_FirmwareImage),
#endif /* MT7603_E1 */
	.fw_bin_file_name = "mtk/MT7603.bin",
#ifdef CARRIER_DETECTION_SUPPORT
	.carrier_func = TONE_RADAR_V2,
#endif
#ifdef CONFIG_WIFI_TEST
	.MemMapStart = 0x0000,
	.MemMapEnd = 0xffff,
	.MacMemMapOffset = 0x1000,
	.MacStart = 0x0000,
	.MacEnd = 0x0600,
	.BBPMemMapOffset = 0x2000,
	.BBPStart = 0x0000,
	.BBPEnd = 0x0f00,
	.RFIndexNum = 0,
	.RFIndexOffset = 0,
	.E2PStart = 0x0000,
	.E2PEnd = 0x00fe,
#endif /* CONFIG_WIFI_TEST */
	.hif_type = HIF_MT,
	.rf_type = RF_MT,
	.RxBAWinSize = 64,
	.AMPDUFactor = 3,

    .BiTxOpOn = 1,
};


static const RTMP_CHIP_OP MT7603_ChipOp = {
	.ChipBBPAdjust = mt7603_bbp_adjust,
	.ChipSwitchChannel = mt7603_switch_channel,
	.AsicMacInit = mt7603_init_mac_cr,
	.AsicBbpInit = MT7603BBPInit,
	.AsicRfInit = mt7603_init_rf_cr,
	.AsicAntennaDefaultReset = mt7603_antenna_default_reset,
	.ChipAGCInit = NULL,
	.AsicRfTurnOn = NULL,
	.AsicHaltAction = NULL,
	.AsicRfTurnOff = NULL,
	.AsicReverseRfFromSleepMode = NULL,
#ifdef CARRIER_DETECTION_SUPPORT
	.ToneRadarProgram = ToneRadarProgram_v2,
#endif 
	.RxSensitivityTuning = NULL,
	.DisableTxRx = NULL,
#ifdef RTMP_PCI_SUPPORT
	//.AsicRadioOn = RT28xxPciAsicRadioOn,
	//.AsicRadioOff = RT28xxPciAsicRadioOff,
#endif

#ifdef CAL_FREE_IC_SUPPORT	
        .is_cal_free_ic = mt7603_is_cal_free_ic,	
        .cal_free_data_get = mt7603_cal_free_data_get,
#endif /* CAL_FREE_IC_SUPPORT */
	.show_pwr_info = mt7603_show_pwr_info,

#ifdef MT_WOW_SUPPORT
	.AsicWOWEnable = MT76xxAndesWOWEnable,
	.AsicWOWDisable = MT76xxAndesWOWDisable,
	.AsicWOWInit = MT76xxAndesWOWInit,
#endif /* MT_WOW_SUPPORT */
	.ChipSetEDCCA = mt7603_set_ed_cca,
#ifdef GREENAP_SUPPORT
	.EnableAPMIMOPS = EnableAPMIMOPSv2,
	.DisableAPMIMOPS = DisableAPMIMOPSv2,
#endif /* GREENAP_SUPPORT */
};


VOID mt7603_init(RTMP_ADAPTER *pAd)
{
	RTMP_CHIP_CAP *pChipCap = &pAd->chipCap;

	DBGPRINT(RT_DEBUG_OFF, ("%s()-->\n", __FUNCTION__));
	
	memcpy(&pAd->chipCap, &MT7603_ChipCap, sizeof(RTMP_CHIP_CAP));
	memcpy(&pAd->chipOps, &MT7603_ChipOp, sizeof(RTMP_CHIP_OP));

	pAd->chipCap.hif_type = HIF_MT;
	pAd->chipCap.mac_type = MAC_MT;
	
	mt_phy_probe(pAd);
#ifdef MT7603_E1
	if (MTK_REV_GTE(pAd, MT7603, MT7603E1) && MTK_REV_LT(pAd, MT7603, MT7603E2)) {
		pChipCap->fw_header_image = MT7603_FirmwareImage;
		pChipCap->fw_bin_file_name = "mtk/MT7603.bin";
		pChipCap->fw_len = sizeof(MT7603_FirmwareImage);

	}
	else 
#endif /* MT7603_E1 */
#ifdef MT7603_E2
	if(MTK_REV_GTE(pAd, MT7603, MT7603E2))
	{
		pChipCap->fw_header_image = MT7603_e2_FirmwareImage;
		pChipCap->fw_bin_file_name = "mtk/MT7603_e2.bin";
		pChipCap->fw_len = sizeof(MT7603_e2_FirmwareImage);
	}
#endif /* MT7603_E2 */

#ifdef RTMP_MAC_PCI
	if (IS_PCI_INF(pAd)) {
		pChipCap->tx_hw_hdr_len = pChipCap->TXWISize;
		pChipCap->rx_hw_hdr_len = pChipCap->RXWISize;
	}
#endif /* RTMP_MAC_PCI */



	RTMP_DRS_ALG_INIT(pAd, RATE_ALG_GRP);
		
	/*
		Following function configure beacon related parameters
		in pChipCap
			FlgIsSupSpecBcnBuf / BcnMaxHwNum / 
			WcidHwRsvNum / BcnMaxHwSize / BcnBase[]
	*/
	mt_bcn_buf_init(pAd);

#ifdef DOT11W_PMF_SUPPORT
	pChipCap->FlgPMFEncrtptMode = PMF_ENCRYPT_MODE_2;
#endif /* DOT11W_PMF_SUPPORT */

	DBGPRINT(RT_DEBUG_OFF, ("<--%s()\n", __FUNCTION__));
}

#ifdef LED_CONTROL_SUPPORT
INT Set_MT7603LED_Proc(
	IN RTMP_ADAPTER		*pAd,
	IN RTMP_STRING		*arg)
{
	UINT8 cmd = (UINT8)simple_strtol(arg, 0, 10);
	/*
		0x2300[5] Default Antenna:
		0 for WIFI main antenna
		1  for WIFI aux  antenna

	*/
	
	if (cmd < 33)
	{
			AndesLedOP(pAd, 0, cmd);
			DBGPRINT(RT_DEBUG_TRACE, ("%s:cmd:0x%x\n", __FUNCTION__, cmd)); 
	}
	return TRUE;
}

INT Set_MT7603LED_Enhance_Proc(
	IN RTMP_ADAPTER		*pAd,
	IN RTMP_STRING		*arg)
{
	UINT8 time = (UINT8)simple_strtol(arg, 0, 10);
	/*
		0x2300[5] Default Antenna:
		0 for WIFI main antenna
		1  for WIFI aux  antenna

	*/
	
	//if (cmd < 33)
	{
			AndesLedEnhanceOP(pAd, 0, time, time, 31);
			DBGPRINT(RT_DEBUG_TRACE, ("%s:time:%d\n", __FUNCTION__, time)); 
	}
	return TRUE;
}

#define LED_BEHAVIOR_SOLID_ON           0
#define LED_BEHAVIOR_SOLID_OFF          1
#define LED_BEHAVIOR_GENERIC_FIX_BLINK 31

INT Set_MT7603LED_Behavor_Proc(
	IN RTMP_ADAPTER		*pAd,
	IN RTMP_STRING		*arg)
{
	UINT8 behavior = (UINT8)simple_strtol(arg, 0, 10);
	DBGPRINT(RT_DEBUG_TRACE, ("-->Set_MT7603LED_Behavor_Proc (%d)\n",behavior)); 
			switch (behavior)
			{
				case 0:
					AndesLedEnhanceOP(pAd, 0, 0, 0, LED_BEHAVIOR_SOLID_OFF);
				break;
				
				case 1:
					AndesLedEnhanceOP(pAd, 0, 0, 0, LED_BEHAVIOR_SOLID_ON);
					break;
				
				case 2:
					AndesLedEnhanceOP(pAd, 0, 27, 27, LED_BEHAVIOR_GENERIC_FIX_BLINK);
					break;
				
				case 3:
					AndesLedEnhanceOP(pAd, 0, 200, 200, LED_BEHAVIOR_GENERIC_FIX_BLINK);
					break;
				
				default:
				{
					DBGPRINT_RAW(RT_DEBUG_ERROR,
						("%s: Unknow LED behavior(%d)\n",
						__FUNCTION__, behavior));
				}
				break;
			}		
	return TRUE;
}
#endif /*LED_CONTROL_SUPPORT*/

#ifdef USB_IOT_WORKAROUND2
void usb_iot_add_padding(struct urb *urb, UINT8 *buf, ra_dma_addr_t dma)
{
	UCHAR pkt2drop[512] = {	0x00, 0x02, 0x00, 0x08, 0x00, 0xcd, 0x02, 0x00, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
							0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
							0x88, 0x01, 0x00, 0x00, 0x48, 0x02, 0x2a, 0x65, 0x44, 0x30, 0x00, 0x0c, 0x43, 0x26, 0x60, 0x40, 
							0x01, 0x00, 0x5e, 0x00, 0x00, 0xfb, 0x60, 0x01, 0x00, 0x00, 0x00, 0x00, 0xaa, 0xaa, 0x03, 0x00, 
							0x00, 0x00, 0x08, 0x00, 0x45, 0x00, 0x00, 0x43, 0x51, 0x0e, 0x40, 0x00, 0xff, 0x11, 0xc0, 0x7b, 
							0xc0, 0xa8, 0xc8, 0x7b, 0xe0, 0x00, 0x00, 0xfb, 0x14, 0xe9, 0x14, 0xe9, 0x00, 0x2f, 0xc8, 0x6f, 
							0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x5f, 0x73, 0x61, 
							0x6e, 0x65, 0x2d, 0x70, 0x6f, 0x72, 0x74, 0x04, 0x5f, 0x74, 0x63, 0x70, 0x05, 0x6c, 0x6f, 0x63, 
							0x61, 0x6c, 0x00, 0x00, 0x0c, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
							0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
							0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
							0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
							0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
							0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
							0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
							0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
							0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
							0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
							0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
							0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
							0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
							0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
							0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
							0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
							0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
							0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
							0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
							0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
							0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
							0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
							0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
							0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
						};

	int padding = 0;
	int sendlen = 0;

	if (!urb || !buf){
		printk("usb_iot_add_padding %x %x\n", urb, buf);
		printk("\ntransfer_buffer_length=%d\n", urb->transfer_buffer_length);
		return;
	}

	padding = 512 - ((urb->transfer_buffer_length)%512);

	// copy data to new place
	NdisCopyMemory(buf,	urb->transfer_buffer, urb->transfer_buffer_length);
	sendlen = urb->transfer_buffer_length;

	// add padding to 512 boundary
	NdisZeroMemory(buf+sendlen, padding);
	sendlen += padding;
	
	NdisCopyMemory(buf+sendlen,	pkt2drop, 512);
	sendlen += 512;

	// add final 4 bytes padding
	NdisZeroMemory(buf+sendlen, 4);
	sendlen += 4;

	// update URB
	urb->transfer_buffer = buf;
	if (dma)
		urb->transfer_dma = dma;

	urb->transfer_buffer_length = sendlen;

}
#endif

void mt7603_set_ed_cca(RTMP_ADAPTER *pAd, BOOLEAN enable)
{
	
	UINT32 macVal = 0, macVal2 = 0; 
	UINT32 NBIDmacVal = 0;
	BOOLEAN bIsCERegion = FALSE;
	UINT32 ed_th = 0;
	bIsCERegion = GetEDCCASupport(pAd);
	ed_th = pAd->ed_th;

	RTMP_IO_READ32(pAd, WF_PHY_BASE + 0x0634, &macVal2);
	
	if (enable)
	{
		macVal = 0xD7C87D0F;  //EDCCA ON , TH - L, USER case  //D7C87D0F
		ed_th = pAd->ed_th;
		pAd->ed_on = TRUE;
		if((!bIsCERegion) && (ed_th < NON_CE_REGION_MAX_ED_TH)) {
			macVal = macVal & (~(0x7F << 8));  /*NON-CE Region, ED_TH set to -40dBm~-50dBm;bit14~bit8 is for ED_TH, 0x00 means -62dBm*/
			macVal = macVal | (ed_th << 8);
			pAd->ed_on = FALSE;
			DBGPRINT(RT_DEBUG_OFF, ("%s: NON-CE Region,60200618=%x,ed_th=%x\n", __FUNCTION__, macVal,ed_th));
		}
		RTMP_IO_WRITE32(pAd, WF_PHY_BASE + 0x0618, macVal);

		macVal2 |= 0x1;
		RTMP_IO_WRITE32(pAd, WF_PHY_BASE + 0x0634, macVal2);
		
#ifdef SMART_CARRIER_SENSE_SUPPORT	
		pAd->SCSCtrl.EDCCA_Status = 1;
		DBGPRINT(RT_DEBUG_ERROR, ("%s: TURN ON EDCCA mac 0x10618 = 0x%x, EDCCA_Status=%d\n", __FUNCTION__, macVal, pAd->SCSCtrl.EDCCA_Status));
#else
		DBGPRINT(RT_DEBUG_ERROR, ("%s: TURN ON EDCCA mac 0x10618 = 0x%x\n", __FUNCTION__, macVal));
#endif /* SMART_CARRIER_SENSE_SUPPORT */

	}
	else	
	{


		/* After 7603 ACI issue task, the edcca could be always opened;
		set ED threshold to -65dBm;
		Disable PSE reset;
		*/
		macVal = 0xD7e87d10;
		pAd->ed_on = TRUE;
		RTMP_IO_WRITE32(pAd, WF_PHY_BASE + 0x0618, macVal);

		macVal2 &= 0xFFFFFFFE;
		RTMP_IO_WRITE32(pAd, WF_PHY_BASE + 0x0634, macVal2);

		/*Adjust PD Threshold to -40dBm*/
		RTMP_IO_READ32(pAd, WF_PHY_BASE + 0x0620 , &macVal);
		macVal &= ~0x3f;
		macVal |= 0x3c;
		RTMP_IO_WRITE32(pAd, WF_PHY_BASE + 0x0620, macVal);
#ifdef SMART_CARRIER_SENSE_SUPPORT
		pAd->SCSCtrl.EDCCA_Status = 0;
		DBGPRINT(RT_DEBUG_ERROR, ("%s: TURN OFF EDCCA  mac 0x10618 = 0x%x, EDCCA_Status=%d\n", __FUNCTION__, macVal, pAd->SCSCtrl.EDCCA_Status));
#else
		DBGPRINT(RT_DEBUG_ERROR, ("%s: TURN OFF EDCCA  mac 0x10618 = 0x%x\n", __FUNCTION__, macVal));
#endif /* SMART_CARRIER_SENSE_SUPPORT */

	}

	if (strncmp(pAd->CommonCfg.CountryCode, "JP", 2) == 0)
	{
		/* disable NBID for JAPAN carrier sense test mac, 0610[24]=0 [31]=0 */		
		RTMP_IO_READ32(pAd, WF_PHY_BASE + 0x0610, &NBIDmacVal);
		
		DBGPRINT(RT_DEBUG_ERROR, ("%s: pAd->CommonCfg.CountryCode = %s \n", __FUNCTION__, pAd->CommonCfg.CountryCode));
		NBIDmacVal &= ~(1<<24); 
		NBIDmacVal &= ~(1<<31); 
		RTMP_IO_WRITE32(pAd, WF_PHY_BASE + 0x0610, NBIDmacVal);
		DBGPRINT(RT_DEBUG_ERROR, ("%s: TURN OFF NBID mac 0x10610 = 0x%x\n", __FUNCTION__, NBIDmacVal));
	}
	else
	{
		RTMP_IO_READ32(pAd, WF_PHY_BASE + 0x0610, &NBIDmacVal);		
		NBIDmacVal |= (1<<24); 
		NBIDmacVal |= (1<<31); 
		RTMP_IO_WRITE32(pAd, WF_PHY_BASE + 0x0610, NBIDmacVal);		
	}
	/*Init Lower_signal_level to -65dBm*/
	RTMP_IO_READ32(pAd, WF_PHY_BASE + 0x0620, &macVal2);
	macVal2 &= 0xFFFFFFC0;
	macVal2 |= 0x00000023;	
	RTMP_IO_WRITE32(pAd, WF_PHY_BASE + 0x0620, macVal2);	
			
}

