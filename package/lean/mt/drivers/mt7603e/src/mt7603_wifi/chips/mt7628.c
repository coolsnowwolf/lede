#include "rt_config.h"

#include "mcu/mt7628_firmware.h"
#include "eeprom/mt7628_e2p.h"
#include "phy/wf_phy_back.h"


static VOID mt7628_bbp_adjust(RTMP_ADAPTER *pAd)
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

static void mt7603_tx_pwr_gain(RTMP_ADAPTER *pAd, UINT8 channel)
{
	UINT32 value;
	CHAR tx_0_pwr;
	struct MT_TX_PWR_CAP *cap = &pAd->chipCap.MTTxPwrCap;


	return;//Carter add for first day link.

	tx_0_pwr = cap->tx_0_target_pwr_g_band;
	tx_0_pwr += cap->tx_0_chl_pwr_delta_g_band[get_low_mid_hi_index(channel)];

	RTMP_IO_READ32(pAd, TMAC_FP0R0, &value);

	value &= ~LG_OFDM0_FRAME_POWER0_DBM_MASK;
	value |= LG_OFDM0_FRAME_POWER0_DBM(tx_0_pwr + cap->tx_pwr_g_band_ofdm_6_9);

	value &= ~LG_OFDM1_FRAME_POWER0_DBM_MASK;
	value |= LG_OFDM1_FRAME_POWER0_DBM(tx_0_pwr + cap->tx_pwr_g_band_ofdm_12_18);

	value &= ~LG_OFDM2_FRAME_POWER0_DBM_MASK;
	value |= LG_OFDM2_FRAME_POWER0_DBM(tx_0_pwr + cap->tx_pwr_g_band_ofdm_24_36);

	value &= ~LG_OFDM3_FRAME_POWER0_DBM_MASK;
	value |= LG_OFDM3_FRAME_POWER0_DBM(tx_0_pwr + cap->tx_pwr_g_band_ofdm_48);

	RTMP_IO_WRITE32(pAd, TMAC_FP0R0, value);

	RTMP_IO_READ32(pAd, TMAC_FP0R1, &value);

	value &= ~HT20_0_FRAME_POWER0_DBM_MASK;
	value |= HT20_0_FRAME_POWER0_DBM(tx_0_pwr + cap->tx_pwr_ht_bpsk_mcs_0_8);

	value &= ~HT20_1_FRAME_POWER0_DBM_MASK;
	value |= HT20_1_FRAME_POWER0_DBM(tx_0_pwr + cap->tx_pwr_ht_qpsk_mcs_1_2_9_10);

	value &= ~HT20_2_FRAME_POWER0_DBM_MASK;
	value |= HT20_2_FRAME_POWER0_DBM(tx_0_pwr + cap->tx_pwr_ht_16qam_mcs_3_4_11_12);

	value &= ~HT20_3_FRAME_POWER0_DBM_MASK;
	value |= HT20_3_FRAME_POWER0_DBM(tx_0_pwr + cap->tx_pwr_ht_64qam_mcs_5_13);

	RTMP_IO_WRITE32(pAd, TMAC_FP0R1, value);

	RTMP_IO_READ32(pAd, TMAC_FP0R2, &value);

	value &= ~HT40_0_FRAME_POWER0_DBM_MASK;
	value |= HT40_0_FRAME_POWER0_DBM(tx_0_pwr + cap->tx_pwr_ht_bpsk_mcs_0_8
											+ cap->delta_tx_pwr_bw40_g_band);

	value &= ~HT40_1_FRAME_POWER0_DBM_MASK;
	value |= HT40_1_FRAME_POWER0_DBM(tx_0_pwr + cap->tx_pwr_ht_qpsk_mcs_1_2_9_10
											+ cap->delta_tx_pwr_bw40_g_band);

	value &= ~HT40_2_FRAME_POWER0_DBM_MASK;
	value |= HT40_2_FRAME_POWER0_DBM(tx_0_pwr + cap->tx_pwr_ht_16qam_mcs_3_4_11_12
											+ cap->delta_tx_pwr_bw40_g_band);

	value &= ~HT40_3_FRAME_POWER0_DBM_MASK;
	value |= HT40_3_FRAME_POWER0_DBM(tx_0_pwr + cap->tx_pwr_ht_64qam_mcs_5_13
											+ cap->delta_tx_pwr_bw40_g_band);

	RTMP_IO_WRITE32(pAd, TMAC_FP0R2, value);

	RTMP_IO_READ32(pAd, TMAC_FP0R3, &value);

	value &= ~CCK0_FRAME_POWER0_DBM_MASK;
	value |= CCK0_FRAME_POWER0_DBM(tx_0_pwr + cap->tx_pwr_cck_1_2);

	value &= ~LG_OFDM4_FRAME_POWER0_DBM_MASK;
	value |= LG_OFDM4_FRAME_POWER0_DBM(tx_0_pwr + cap->tx_pwr_g_band_ofdm_54);

	value &= ~CCK1_FRAME_POWER0_DBM_MASK;
	value |= CCK1_FRAME_POWER0_DBM(tx_0_pwr + cap->tx_pwr_cck_5_11);

	value &= ~HT40_6_FRAME_POWER0_DBM_MASK;
	value |= HT40_6_FRAME_POWER0_DBM(tx_0_pwr + cap->tx_pwr_ht_bpsk_mcs_32 + cap->delta_tx_pwr_bw40_g_band);

	RTMP_IO_WRITE32(pAd, TMAC_FP0R3, value);

	RTMP_IO_READ32(pAd, TMAC_FP0R4, &value);

	value &= ~HT20_4_FRAME_POWER0_DBM_MASK;
	value |= HT20_4_FRAME_POWER0_DBM(tx_0_pwr + cap->tx_pwr_ht_64qam_mcs_6_14);

	value &= ~HT20_5_FRAME_POWER0_DBM_MASK;
	value |= HT20_5_FRAME_POWER0_DBM(tx_0_pwr + cap->tx_pwr_ht_64qam_mcs_7_15);

	value &= ~HT40_4_FRAME_POWER0_DBM_MASK;
	value |= HT40_4_FRAME_POWER0_DBM(tx_0_pwr + cap->tx_pwr_ht_64qam_mcs_6_14
												+ cap->delta_tx_pwr_bw40_g_band);

	value &= ~HT40_5_FRAME_POWER0_DBM_MASK;
	value |= HT40_5_FRAME_POWER0_DBM(tx_0_pwr + cap->tx_pwr_ht_64qam_mcs_7_15
												+ cap->delta_tx_pwr_bw40_g_band);

	RTMP_IO_WRITE32(pAd, TMAC_FP0R4, value);
}


static void mt7628_switch_channel(RTMP_ADAPTER *pAd, UCHAR channel, BOOLEAN scan)
{


	/* tx pwr gain setting */
	mt7603_tx_pwr_gain(pAd, channel);

	if (pAd->CommonCfg.BBPCurrentBW == BW_20)
		CmdChannelSwitch(pAd, channel, channel, BW_20,
								pAd->CommonCfg.TxStream, pAd->CommonCfg.RxStream);
	else
		CmdChannelSwitch(pAd, pAd->CommonCfg.Channel, channel, pAd->CommonCfg.BBPCurrentBW,
							pAd->CommonCfg.TxStream, pAd->CommonCfg.RxStream);

	/* Channel latch */
	pAd->LatchRfRegs.Channel = channel;


	DBGPRINT(RT_DEBUG_OFF,
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
		TMAC_TXD_4 *dw4 = (TMAC_TXD_4 *)(&dw[2]);
		TMAC_TXD_5 *dw5 = (TMAC_TXD_5 *)(&dw[3]);
		TMAC_TXD_6 *dw6 = (TMAC_TXD_6 *)(&dw[4]);

		NdisZeroMemory((UCHAR *)(&dw[0]), sizeof(dw));

		dw2->htc_vld = 0;
		dw2->frag = 0;
		dw2->max_tx_time = 0;
		dw2->fix_rate = 0;

		dw3->remain_tx_cnt = MT_TX_RETRY_CNT;
		dw3->sn_vld = 0;
		dw3->pn_vld = 0;

		dw5->pid = PID_DATA_AMPDU;
		dw5->tx_status_fmt = 0;
		dw5->tx_status_2_host = 0; // Disable TxS
		dw5->bar_sn_ctrl = 0; //HW
		dw5->pwr_mgmt = TMI_PM_BIT_CFG_BY_HW; // HW

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


static VOID mt7628_init_mac_cr(RTMP_ADAPTER *pAd)
{
	UINT32 mac_val;

	DBGPRINT(RT_DEBUG_OFF, ("%s()-->\n", __FUNCTION__));

	/* Preparation of TxD DW2~DW6 when we need run 3DW format */
	asic_set_tmac_info_template(pAd);

	/* A-MPDU Agg limit control in range 1/2/4/8/10/12/14/16 */
	RTMP_IO_READ32(pAd, AGG_AWSCR, &mac_val);
	mac_val = ((1 << 0) |(2<<8) | (4 << 16) | (8 << 24));
	RTMP_IO_WRITE32(pAd, AGG_AWSCR, mac_val);

	RTMP_IO_READ32(pAd, AGG_AWSCR1, &mac_val);
	//mac_val = ((10 << 0) |(12<<8) | (14 << 16) | (16 << 24));
    mac_val &= 0x0;
	mac_val = ((10 << 0) |(12<<8) | (14 << 16) | (0 << 24));
 	RTMP_IO_WRITE32(pAd, AGG_AWSCR1, mac_val);

	RTMP_IO_WRITE32(pAd, AGG_AALCR, 0);
	RTMP_IO_WRITE32(pAd, AGG_AALCR1, 0);

	/* Vector report queue setting */
	RTMP_IO_READ32(pAd, DMA_VCFR0, &mac_val);
	mac_val |= BIT13;
	RTMP_IO_WRITE32(pAd, DMA_VCFR0, mac_val);

	/* TxStatus report queue setting */
	RTMP_IO_READ32(pAd, DMA_TCFR1, &mac_val);
	mac_val |= BIT14;
	RTMP_IO_WRITE32(pAd, DMA_TCFR1, mac_val);

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
	// TODO: shiang-MT7603, remove me after FPGA verification done
	mac_val |= 0xc0200000; // receive BA/CF_End/Ack/RTS/CTS/CTRL_RSVED
	RTMP_IO_WRITE32(pAd, DMA_RCFR0, mac_val);

	/* Configure Rx Vectors report to HIF */
	RTMP_IO_READ32(pAd, DMA_VCFR0, &mac_val);
	mac_val &= (~0x1); // To HIF
	mac_val |= 0x2000; // RxRing 1
	RTMP_IO_WRITE32(pAd, DMA_VCFR0, mac_val);

	/* Enable RX Group to HIF */
	AsicSetRxGroup(pAd, HIF_PORT, RXS_GROUP3, TRUE);
	AsicSetRxGroup(pAd, MCU_PORT, RXS_GROUP3, TRUE);

	/* AMPDU BAR setting */
	/* Enable HW BAR feature */
	AsicSetBARTxCntLimit(pAd, TRUE, 1);

	/* Configure the BAR rate setting */
	RTMP_IO_READ32(pAd, AGG_ACR, &mac_val);
	mac_val &= (~0xfff00000);
	mac_val &= ~(AGG_ACR_AMPDU_NO_BA_AR_RULE_MASK|AMPDU_NO_BA_RULE);
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


	/* CCA Workaround */
	RTMP_IO_READ32(pAd, TMAC_TRCR, &mac_val);
	mac_val &= ~CCA_SRC_SEL_MASK;
	mac_val |= CCA_SRC_SEL(0x3);
	mac_val &= ~CCA_SEC_SRC_SEL_MASK;
	mac_val |= CCA_SEC_SRC_SEL(0x3);
	RTMP_IO_WRITE32(pAd, TMAC_TRCR, mac_val);

	RTMP_IO_READ32(pAd, WTBL_OFF_RMVTCR, &mac_val);
	/* RCPI include ACK and Data */
	mac_val |= RX_MV_MODE;
	RTMP_IO_WRITE32(pAd, WTBL_OFF_RMVTCR, mac_val);

#ifdef MT7628_FPGA
	// enable MAC2MAC mode
	RTMP_IO_READ32(pAd, RMAC_MISC, &mac_val);
	mac_val |= BIT18;
	RTMP_IO_WRITE32(pAd, RMAC_MISC, mac_val);

    mac_val = 0x00d700d7;
    RTMP_IO_WRITE32(pAd, TMAC_CDTR, mac_val);
    mac_val = 0x00d700d7;
    RTMP_IO_WRITE32(pAd, TMAC_ODTR, mac_val);
#endif /* MT7628_FPGA */



	/* Turn on RX RIFS Mode */
	RTMP_IO_READ32(pAd, TMAC_TCR, &mac_val);
	mac_val |= RX_RIFS_MODE;
	RTMP_IO_WRITE32(pAd, TMAC_TCR, mac_val);
	/*enable RDG setting*/
	if(pAd->CommonCfg.bRdg)
	{
		RTMP_IO_READ32(pAd, TMAC_TCR, &mac_val);
		mac_val |= (RDG_RA_MODE | RDG_RESP_EN);
		RTMP_IO_WRITE32(pAd, TMAC_TCR, mac_val);

		/*enable long NAV*/
		RTMP_IO_READ32(pAd, AGG_PCR, &mac_val);
		mac_val &= ~(PROTECTION_MODE);
		RTMP_IO_WRITE32(pAd, TMAC_TCR, mac_val);
	}
}


static VOID MT7628BBPInit(RTMP_ADAPTER *pAd)
{
	UINT32 Value;

    RTMP_IO_READ32(pAd, 0x106c8, &Value);
    Value &= ~(1 << 30);
    RTMP_IO_WRITE32(pAd, 0x106c8, Value);

    RTMP_IO_READ32(pAd, 0x106cc, &Value);
    Value &= ~(1 << 0);
    RTMP_IO_WRITE32(pAd, 0x106cc, Value);
}

static void mt7628_init_rf_cr(RTMP_ADAPTER *ad)
{
	return;
}

static UINT8 mt7628_txpwr_chlist[] = {
	1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
};


int mt7628_read_chl_pwr(RTMP_ADAPTER *pAd)
{
	UINT32 i, choffset;
	struct MT_TX_PWR_CAP *cap = &pAd->chipCap.MTTxPwrCap;

	mt7628_get_tx_pwr_info(pAd);

	DBGPRINT(RT_DEBUG_TRACE, ("%s()--->\n", __FUNCTION__));

	for (i = 0; i < sizeof(mt7628_txpwr_chlist); i++) {
		pAd->TxPower[i].Channel = mt7628_txpwr_chlist[i];
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

	return TRUE;
}


/* Read power per rate */
void mt7628_get_tx_pwr_per_rate(RTMP_ADAPTER *pAd)
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


void mt7628_get_tx_pwr_info(RTMP_ADAPTER *pAd)
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


static VOID mt7628_show_pwr_info(RTMP_ADAPTER *pAd)
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


static void mt7628_antenna_default_reset(
	struct _RTMP_ADAPTER *pAd,
	EEPROM_ANTENNA_STRUC *pAntenna)
{
	pAntenna->word = 0;
	pAd->RfIcType = RFIC_7628;
	pAntenna->field.TxPath = 2;
	pAntenna->field.RxPath = 2;
}

static const RTMP_CHIP_CAP MT7628_ChipCap = {
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
#ifdef NEW_MBSSID_MODE
#ifdef ENHANCE_NEW_MBSSID_MODE
	.MBSSIDMode = MBSSID_MODE4,
#else
	.MBSSIDMode = MBSSID_MODE1,
#endif /* ENHANCE_NEW_MBSSID_MODE */
#else
	.MBSSIDMode = MBSSID_MODE0,
#endif /* NEW_MBSSID_MODE */
#ifdef RTMP_EFUSE_SUPPORT
	.EFUSE_USAGE_MAP_START = 0x1e0,
	.EFUSE_USAGE_MAP_END = 0x1fc,
	.EFUSE_USAGE_MAP_SIZE = 29,
	.EFUSE_RESERVED_SIZE = 22,	// Cal-Free is 22 free block
#endif
	.EEPROM_DEFAULT_BIN = MT7628_E2PImage,
	.EEPROM_DEFAULT_BIN_SIZE = sizeof(MT7628_E2PImage),
#ifdef CONFIG_ANDES_SUPPORT
	.CmdRspRxRing = RX_RING1,
	.need_load_fw = TRUE,
//	.DownLoadType = DownLoadTypeA,
#endif
	.MCUType = ANDES,
	.cmd_header_len = 12,
#ifdef RTMP_PCI_SUPPORT
	.cmd_padding_len = 0,
#endif
	.fw_header_image = MT7628_FirmwareImage,
	.fw_bin_file_name = "mtk/MT7628.bin",
	.fw_len = sizeof(MT7628_FirmwareImage),
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
	.RxBAWinSize = 21,
	.AMPDUFactor = 2,
};


static const RTMP_CHIP_OP MT7628_ChipOp = {
	.ChipBBPAdjust = mt7628_bbp_adjust,
	.ChipSwitchChannel = mt7628_switch_channel,
	.AsicMacInit = mt7628_init_mac_cr,
	.AsicBbpInit = MT7628BBPInit,
	.AsicRfInit = mt7628_init_rf_cr,
	.AsicAntennaDefaultReset = mt7628_antenna_default_reset,
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
	.show_pwr_info = mt7628_show_pwr_info,
};


VOID mt7628_init(RTMP_ADAPTER *pAd)
{
	RTMP_CHIP_CAP *pChipCap = &pAd->chipCap;

	DBGPRINT(RT_DEBUG_OFF, ("%s()-->\n", __FUNCTION__));

	memcpy(&pAd->chipCap, &MT7628_ChipCap, sizeof(RTMP_CHIP_CAP));
	memcpy(&pAd->chipOps, &MT7628_ChipOp, sizeof(RTMP_CHIP_OP));

	pAd->chipCap.hif_type = HIF_MT;
	pAd->chipCap.mac_type = MAC_MT;

	mt_phy_probe(pAd);

	pChipCap->tx_hw_hdr_len = pChipCap->TXWISize;
	pChipCap->rx_hw_hdr_len = pChipCap->RXWISize;




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

