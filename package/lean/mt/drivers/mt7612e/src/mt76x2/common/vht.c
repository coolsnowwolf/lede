/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:

	Abstract:

	Revision History:
	Who 		When			What
	--------	----------		----------------------------------------------
*/


#include "rt_config.h"

/* some buggy BCM clients can not work with 80MHz */
#define BAD_VHT80_WORKAROUND

#ifdef BAD_VHT80_WORKAROUND
static const UCHAR BAD_VHT80_OUI[][3] = {
	{0x3C, 0xFA, 0x43},	// Huawei P9
	{0x7C, 0x11, 0xCB},	// Huawei Honor 8

	/* iPhone 6*/
	{0x74, 0x1B, 0xB2},
	{0x84, 0x89, 0xAD},
	{0xD8, 0x1D, 0x72},
	{0x60, 0xF8, 0x1D},
	{0x60, 0xA3, 0x7D},
	{0x88, 0x66, 0xA5},
	{0x50, 0xA6, 0x7F},
	{0x6C, 0x72, 0xE7},
	{0x2C, 0x61, 0xF6},
	{0x90, 0x8D, 0x6C},
	{0x90, 0x4F, 0xDA},

	/* MACBOOK */
	{0xAC, 0xBC, 0x32},
	{0xB8, 0xE8, 0x56},
	{0x08, 0x6D, 0x41},
	{0x18, 0x65, 0x90},
	{0xA8, 0x66, 0x7F},
	{0x98, 0x01, 0xA7},

	/* HUAWEI */
	{0xF0, 0x43, 0x47},
	{0xA8, 0xC8, 0x3A},
	{0x10, 0xB1, 0xF8},
	{0x5C, 0xC3, 0x07},
	{0x0C, 0x8F, 0xFF},

	/* ONEPLUS */
	{0x94, 0x0E, 0x6B},
};
#endif /* BAD_VHT80_WORKAROUND */

struct vht_ch_layout{
	UCHAR ch_low_bnd;
	UCHAR ch_up_bnd;
	UCHAR cent_freq_idx;
};

static struct vht_ch_layout vht_ch_80M[]={
	{36, 48, 42},
	{52, 64, 58},
	{100,112, 106},
	{116, 128, 122},	
	{132, 144, 138},
	{149, 161, 155},
	{0, 0 ,0},
};



#ifdef DBG
VOID dump_vht_cap(RTMP_ADAPTER *pAd, VHT_CAP_IE *vht_ie)
{
	VHT_CAP_INFO *vht_cap = &vht_ie->vht_cap;
	VHT_MCS_SET *vht_mcs = &vht_ie->mcs_set;

	DBGPRINT(RT_DEBUG_OFF, ("Dump VHT_CAP IE\n"));	
	hex_dump("VHT CAP IE Raw Data", (UCHAR *)vht_ie, sizeof(VHT_CAP_IE));

	DBGPRINT(RT_DEBUG_OFF, ("VHT Capabilities Info Field\n"));
	DBGPRINT(RT_DEBUG_OFF, ("\tMaximum MPDU Length=%d\n", vht_cap->max_mpdu_len));
	DBGPRINT(RT_DEBUG_OFF, ("\tSupported Channel Width=%d\n", vht_cap->ch_width));
	DBGPRINT(RT_DEBUG_OFF, ("\tRxLDPC=%d\n", vht_cap->rx_ldpc));
	DBGPRINT(RT_DEBUG_OFF, ("\tShortGI_80M=%d\n", vht_cap->sgi_80M));
	DBGPRINT(RT_DEBUG_OFF, ("\tShortGI_160M=%d\n", vht_cap->sgi_160M));
	DBGPRINT(RT_DEBUG_OFF, ("\tTxSTBC=%d\n", vht_cap->tx_stbc));
	DBGPRINT(RT_DEBUG_OFF, ("\tRxSTBC=%d\n", vht_cap->rx_stbc));
	DBGPRINT(RT_DEBUG_OFF, ("\tSU BeamformerCap=%d\n", vht_cap->bfer_cap_su));
	DBGPRINT(RT_DEBUG_OFF, ("\tSU BeamformeeCap=%d\n", vht_cap->bfee_cap_su));
	DBGPRINT(RT_DEBUG_OFF, ("\tCompressedSteeringNumOfBeamformerAnt=%d\n", vht_cap->cmp_st_num_bfer));
	DBGPRINT(RT_DEBUG_OFF, ("\tNumber of Sounding Dimensions=%d\n", vht_cap->num_snd_dimension));	
	DBGPRINT(RT_DEBUG_OFF, ("\tMU BeamformerCap=%d\n", vht_cap->bfer_cap_mu));
	DBGPRINT(RT_DEBUG_OFF, ("\tMU BeamformeeCap=%d\n", vht_cap->bfee_cap_mu));
	DBGPRINT(RT_DEBUG_OFF, ("\tVHT TXOP PS=%d\n", vht_cap->vht_txop_ps));
	DBGPRINT(RT_DEBUG_OFF, ("\t+HTC-VHT Capable=%d\n", vht_cap->htc_vht_cap));
	DBGPRINT(RT_DEBUG_OFF, ("\tMaximum A-MPDU Length Exponent=%d\n", vht_cap->max_ampdu_exp));
	DBGPRINT(RT_DEBUG_OFF, ("\tVHT LinkAdaptation Capable=%d\n", vht_cap->vht_link_adapt));

	DBGPRINT(RT_DEBUG_OFF, ("VHT Supported MCS Set Field\n"));
	DBGPRINT(RT_DEBUG_OFF, ("\tRx Highest SupDataRate=%d\n", vht_mcs->rx_high_rate));
	DBGPRINT(RT_DEBUG_OFF, ("\tRxMCS Map_1SS=%d\n", vht_mcs->rx_mcs_map.mcs_ss1));
	DBGPRINT(RT_DEBUG_OFF, ("\tRxMCS Map_2SS=%d\n", vht_mcs->rx_mcs_map.mcs_ss2));
	DBGPRINT(RT_DEBUG_OFF, ("\tTx Highest SupDataRate=%d\n", vht_mcs->tx_high_rate));
	DBGPRINT(RT_DEBUG_OFF, ("\tTxMCS Map_1SS=%d\n", vht_mcs->tx_mcs_map.mcs_ss1));
	DBGPRINT(RT_DEBUG_OFF, ("\tTxMCS Map_2SS=%d\n", vht_mcs->tx_mcs_map.mcs_ss2));
}


VOID dump_vht_op(RTMP_ADAPTER *pAd, VHT_OP_IE *vht_ie)
{
	VHT_OP_INFO *vht_op = &vht_ie->vht_op_info;
	VHT_MCS_MAP *vht_mcs = &vht_ie->basic_mcs_set;
	
	DBGPRINT(RT_DEBUG_OFF, ("Dump VHT_OP IE\n"));	
	hex_dump("VHT OP IE Raw Data", (UCHAR *)vht_ie, sizeof(VHT_OP_IE));

	DBGPRINT(RT_DEBUG_OFF, ("VHT Operation Info Field\n"));
	DBGPRINT(RT_DEBUG_OFF, ("\tChannelWidth=%d\n", vht_op->ch_width));
	DBGPRINT(RT_DEBUG_OFF, ("\tChannelCenterFrequency Seg 1=%d\n", vht_op->center_freq_1));
	DBGPRINT(RT_DEBUG_OFF, ("\tChannelCenterFrequency Seg 1=%d\n", vht_op->center_freq_2));

	DBGPRINT(RT_DEBUG_OFF, ("VHT Basic MCS Set Field\n"));
	DBGPRINT(RT_DEBUG_OFF, ("\tRxMCS Map_1SS=%d\n", vht_mcs->mcs_ss1));
	DBGPRINT(RT_DEBUG_OFF, ("\tRxMCS Map_2SS=%d\n", vht_mcs->mcs_ss2));
}
#endif

#ifdef VHT_TXBF_SUPPORT
VOID trigger_vht_ndpa(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *entry)
{
	UCHAR *buf;
	VHT_NDPA_FRAME *vht_ndpa;
	struct wifi_dev *wdev = entry->wdev;
	UINT frm_len, sta_cnt;
	SNDING_STA_INFO *sta_info;

	if (MlmeAllocateMemory(pAd, &buf) != NDIS_STATUS_SUCCESS)
		return;

	NdisZeroMemory(buf, MGMT_DMA_BUFFER_SIZE);

	vht_ndpa = (VHT_NDPA_FRAME *)buf;
	frm_len = sizeof(VHT_NDPA_FRAME);
	vht_ndpa->fc.Type = FC_TYPE_CNTL;
	vht_ndpa->fc.SubType = SUBTYPE_VHT_NDPA;
	COPY_MAC_ADDR(vht_ndpa->ra, entry->Addr);
	COPY_MAC_ADDR(vht_ndpa->ta, wdev->if_addr);

	/* Currnetly we only support 1 STA for a VHT DNPA */
	sta_info = vht_ndpa->sta_info;
	for (sta_cnt = 0; sta_cnt < 1; sta_cnt++) {
		sta_info->aid12 = entry->Aid;
		sta_info->fb_type = SNDING_FB_SU;
		sta_info->nc_idx = 0;
		vht_ndpa->token.token_num = entry->snd_dialog_token;
		frm_len += sizeof(SNDING_STA_INFO);
		sta_info++;
		if (frm_len >= (MGMT_DMA_BUFFER_SIZE - sizeof(SNDING_STA_INFO))) {
			DBGPRINT(RT_DEBUG_ERROR, ("%s(): len(%d) too large!cnt=%d\n",
						__FUNCTION__, frm_len, sta_cnt));
			break;
		}
	}
	if (entry->snd_dialog_token & 0xc0)
		entry->snd_dialog_token = 0;
	else
		entry->snd_dialog_token++;

	vht_ndpa->duration = pAd->CommonCfg.Dsifs + 
						RTMPCalcDuration(pAd, pAd->CommonCfg.MlmeRate, frm_len);

	//DBGPRINT(RT_DEBUG_OFF, ("Send VHT NDPA Frame to STA(%02x:%02x:%02x:%02x:%02x:%02x)\n",
	//			PRINT_MAC(entry->Addr)));
	//hex_dump("VHT NDPA Frame", buf, frm_len);
	MiniportMMRequest(pAd, 0, buf, frm_len);
	MlmeFreeMemory(pAd, buf);
		
#ifdef SOFT_SOUNDING
	if (1) {
		HEADER_802_11 *pNullFr;
		UCHAR *qos_p;
		UCHAR NullFrame[48];
		
		NdisZeroMemory(NullFrame, 48);
		pNullFr = (PHEADER_802_11)&NullFrame[0];
		frm_len = sizeof(HEADER_802_11);

		pNullFr->FC.Type = FC_TYPE_DATA;
		pNullFr->FC.SubType = SUBTYPE_QOS_NULL;
		pNullFr->FC.FrDs = 1;
		pNullFr->FC.ToDs = 0;
		COPY_MAC_ADDR(pNullFr->Addr1, entry->Addr);
		COPY_MAC_ADDR(pNullFr->Addr2, wdev->if_addr);
		COPY_MAC_ADDR(pNullFr->Addr3, wdev->bssid);
		
		qos_p = ((UCHAR *)pNullFr) + frm_len;
		qos_p[0] = 0;
		qos_p[1] = 0;
		frm_len += 2;

		entry->snd_reqired = TRUE;
		DBGPRINT(RT_DEBUG_OFF, ("Send sounding QoSNULL Frame to STA(%02x:%02x:%02x:%02x:%02x:%02x)\n",
					PRINT_MAC(entry->Addr)));

		hex_dump("VHT NDP Frame(QoSNull)", NullFrame, frm_len);
		
		HAL_KickOutNullFrameTx(pAd, 0, NullFrame, frm_len);
	}
#endif /* SOFT_SOUNDING */

}
#endif /* VHT_TXBF_SUPPORT */


/*
	Get BBP Channel Index by RF channel info
	return value: 0~3
*/
UCHAR vht_prim_ch_idx(UCHAR vht_cent_ch, UCHAR prim_ch)
{
	INT idx = 0;
	UCHAR bbp_idx = 0;

	if (vht_cent_ch == prim_ch)
		goto done;

	while (vht_ch_80M[idx].ch_up_bnd != 0)
	{
		if (vht_cent_ch == vht_ch_80M[idx].cent_freq_idx)
		{
			if (prim_ch == vht_ch_80M[idx].ch_up_bnd)
				bbp_idx = 3;
			else if (prim_ch == vht_ch_80M[idx].ch_low_bnd)
				bbp_idx = 0;
			else {
				bbp_idx = prim_ch > vht_cent_ch ? 2 : 1;
			}
			break;
		}
		idx++;
	}

done:
	DBGPRINT(RT_DEBUG_INFO, ("%s():(VhtCentCh=%d, PrimCh=%d) =>BbpChIdx=%d\n",
				__FUNCTION__, vht_cent_ch, prim_ch, bbp_idx));
	return bbp_idx;
}


/*
	Currently we only consider about VHT 80MHz!
*/
UCHAR vht_cent_ch_freq(RTMP_ADAPTER *pAd, UCHAR prim_ch)
{
	INT idx = 0, ch_idx = 0;
	BOOLEAN ch_support_bw_80 = FALSE;

	if (pAd->CommonCfg.vht_bw < VHT_BW_80 || prim_ch < 36)
	{
		pAd->CommonCfg.vht_cent_ch = 0;
		pAd->CommonCfg.vht_cent_ch2 = 0;
		return prim_ch;
	}

#ifdef RT_CFG80211_SUPPORT
#else
	/* 
	sanity check , if this channel has no BW80 capability, use first channel in channel list 
	ex: when CH144 is not availble , group CH 132~140 won't have BW80 cap , shouldn't be used.
	*/
	for ( ch_idx = 0; ch_idx <  pAd->ChannelListNum; ch_idx++)
	{
		if ((pAd->ChannelList[ch_idx].Channel == prim_ch) && (pAd->ChannelList[ch_idx].Flags & CHANNEL_80M_CAP))
			ch_support_bw_80 = TRUE;
	}
			
	if(ch_support_bw_80 == FALSE)
	{
		pAd->CommonCfg.Channel = FirstChannel(pAd);
		DBGPRINT(RT_DEBUG_OFF, ("vht_cent_ch_freq: channel(%d) don't have BW80 capability, use first channel in channel list=%d \n"
		, prim_ch, pAd->CommonCfg.Channel));
		prim_ch = FirstChannel(pAd);
	}
#endif /* RT_CFG80211_SUPPORT */

	/* choose cent_freq by prim_ch */
	
	while (vht_ch_80M[idx].ch_up_bnd != 0)
	{
		if (prim_ch >= vht_ch_80M[idx].ch_low_bnd &&
			prim_ch <= vht_ch_80M[idx].ch_up_bnd)
		{
			pAd->CommonCfg.vht_cent_ch = vht_ch_80M[idx].cent_freq_idx;
			return vht_ch_80M[idx].cent_freq_idx;
		}
		idx++;
	}

	return prim_ch;
}


INT vht_mode_adjust(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry, VHT_CAP_IE *cap, VHT_OP_IE *op)
{
	struct wifi_dev *wdev;
	INT vht_bw = VHT_BW_80;

	pEntry->MaxHTPhyMode.field.MODE = MODE_VHT;
	pAd->CommonCfg.AddHTInfo.AddHtInfo2.NonGfPresent = 1;
	pAd->MacTab.fAnyStationNonGF = TRUE;

	wdev = pEntry->wdev;
	if (wdev)
		vht_bw = wdev->DesiredHtPhyInfo.vht_bw;

	DBGPRINT(RT_DEBUG_TRACE, ("%s: DesiredHtPhyInfo->vht_bw=%d, ch_width=%d\n", __FUNCTION__,
		vht_bw, cap->vht_cap.ch_width));

	if (pEntry->MaxHTPhyMode.field.BW == BW_40)
	{
		if (vht_bw == VHT_BW_80)
		{
			if (cap->vht_cap.ch_width == 0)
			{
				if (op != NULL)
				{
					if (op->vht_op_info.ch_width != 0)
					{
						pEntry->MaxHTPhyMode.field.BW = BW_80;
					}
				}
				else
				{
					/* can not know peer capability, use it's maximum capability */
					pEntry->MaxHTPhyMode.field.BW = BW_80;
#ifdef BAD_VHT80_WORKAROUND
					/* skip DB region */
					if ((pAd->CommonCfg.CountryRegionForABand & 0x7f) != 7)
					{
						INT i;

						/* some buggy BCM clients can not work with 80MHz */
						for (i = 0; i < sizeof(BAD_VHT80_OUI) / 3; i++)
						{
							if (NdisEqualMemory(pEntry->Addr, &BAD_VHT80_OUI[i][0], 3))
							{
								pEntry->MaxHTPhyMode.field.BW = BW_40;
								printk("%s: drop buggy OUI: %02X-%02X-%02X to VHT40!\n",
									"mt7612",
									BAD_VHT80_OUI[i][0],
									BAD_VHT80_OUI[i][1],
									BAD_VHT80_OUI[i][2]);
								break;
							}
						}
					}
#endif /* BAD_VHT80_WORKAROUND */
				}
			}
			else
			{
				pEntry->MaxHTPhyMode.field.BW = BW_80;
			}
		}
	}

	pEntry->MaxHTPhyMode.field.STBC = (pAd->CommonCfg.vht_stbc && cap->vht_cap.rx_stbc > 1) ? 1 : 0;

	if (pEntry->MaxHTPhyMode.field.BW == BW_80)
	{
		pEntry->MaxHTPhyMode.field.ShortGI = (pAd->CommonCfg.vht_sgi_80 && cap->vht_cap.sgi_80M) ? 1 : 0;
	}

	return TRUE;
}


INT get_vht_op_ch_width(RTMP_ADAPTER *pAd)
{
	
	return TRUE;
}


/********************************************************************
	Procedures for 802.11 AC Information elements
********************************************************************/
/*
	Defined in IEEE 802.11AC

	Appeared in Beacon, ProbResp frames
*/
INT build_quiet_channel(RTMP_ADAPTER *pAd, UCHAR *buf)
{
	INT len = 0;


	return len;
}


/*
	Defined in IEEE 802.11AC

	Appeared in Beacon, ProbResp frames
*/
INT build_ext_bss_load(RTMP_ADAPTER *pAd, UCHAR *buf)
{
	INT len = 0;


	return len;
}




/*
	Defined in IEEE 802.11AC

	Appeared in Beacon, ProbResp frames
*/
INT build_ext_pwr_constraint(RTMP_ADAPTER *pAd, UCHAR *buf)
{
	INT len = 0;


	return len;
}


/*
	Defined in IEEE 802.11AC

	Appeared in Beacon, ProbResp frames
*/
INT build_vht_txpwr_envelope(RTMP_ADAPTER *pAd, UCHAR *buf)
{
	INT len = 0, pwr_cnt;
	VHT_TXPWR_ENV_IE txpwr_env;

	NdisZeroMemory(&txpwr_env, sizeof(txpwr_env));

	if (pAd->CommonCfg.vht_bw == VHT_BW_80) {
		pwr_cnt = 2;
	} else {
		if (pAd->CommonCfg.AddHTInfo.AddHtInfo.RecomWidth == 1)
			pwr_cnt = 1;
		else
			pwr_cnt = 0;
	}
	txpwr_env.tx_pwr_info.max_tx_pwr_cnt = pwr_cnt;
	txpwr_env.tx_pwr_info.max_tx_pwr_interpretation = TX_PWR_INTERPRET_EIRP;

// TODO: fixme, we need the real tx_pwr value for each port.
	for (len = 0; len < pwr_cnt; len++)
		txpwr_env.tx_pwr_bw[len] = 15;

	len = 2 + pwr_cnt;
	NdisMoveMemory(buf, &txpwr_env, len);
	
	return len;
}


/*
	Defined in IEEE 802.11AC

	Appeared in Beacon, (Re)AssocResp, ProbResp frames
*/	
INT build_vht_op_ie(RTMP_ADAPTER *pAd, UCHAR *buf)
{
	VHT_OP_IE vht_op;
	UCHAR cent_ch;
#ifdef RT_BIG_ENDIAN
	UINT16 tmp;
#endif /* RT_BIG_ENDIAN */

	NdisZeroMemory((UCHAR *)&vht_op, sizeof(VHT_OP_IE));
	vht_op.vht_op_info.ch_width = (pAd->CommonCfg.vht_bw == VHT_BW_80 ? 1: 0);

#ifdef CONFIG_AP_SUPPORT
	if (pAd->CommonCfg.Channel > 14 && 
		(pAd->CommonCfg.bIEEE80211H == 1) && 
		(pAd->Dot11_H.RDMode == RD_SWITCHING_MODE))
		cent_ch = vht_cent_ch_freq(pAd, pAd->Dot11_H.org_ch);
	else
#endif /* CONFIG_AP_SUPPORT */
		cent_ch = vht_cent_ch_freq(pAd, pAd->CommonCfg.Channel);

	switch (vht_op.vht_op_info.ch_width)
	{
		case 0:
			vht_op.vht_op_info.center_freq_1 = 0;
			vht_op.vht_op_info.center_freq_2 = 0;
			break;
		case 1:
		case 2:
			vht_op.vht_op_info.center_freq_1 = cent_ch;
			vht_op.vht_op_info.center_freq_2 = 0;
			break;
		case 3:
			vht_op.vht_op_info.center_freq_1 = cent_ch;
			vht_op.vht_op_info.center_freq_2 = pAd->CommonCfg.vht_cent_ch2;
			break;
	}

	vht_op.basic_mcs_set.mcs_ss1 = VHT_MCS_CAP_NA;
	vht_op.basic_mcs_set.mcs_ss2 = VHT_MCS_CAP_NA;
	vht_op.basic_mcs_set.mcs_ss3 = VHT_MCS_CAP_NA;
	vht_op.basic_mcs_set.mcs_ss4 = VHT_MCS_CAP_NA;
	vht_op.basic_mcs_set.mcs_ss5 = VHT_MCS_CAP_NA;
	vht_op.basic_mcs_set.mcs_ss6 = VHT_MCS_CAP_NA;
	vht_op.basic_mcs_set.mcs_ss7 = VHT_MCS_CAP_NA;
	vht_op.basic_mcs_set.mcs_ss8 = VHT_MCS_CAP_NA;
	switch  (pAd->CommonCfg.RxStream)
	{
		case 2:
#ifdef MT76x2
			if (IS_MT76x2(pAd)) {
				vht_op.basic_mcs_set.mcs_ss2 = (((pAd->CommonCfg.vht_bw == VHT_BW_2040) 
					&& (pAd->CommonCfg.RegTransmitSetting.field.BW == BW_20)) ? VHT_MCS_CAP_8 : VHT_MCS_CAP_9);
			} else
#endif /* MT76x2 */
				vht_op.basic_mcs_set.mcs_ss2 = VHT_MCS_CAP_7;
		case 1:
#if	defined(MT76x0) || defined(MT76x2)
			if (IS_MT76x0(pAd) || IS_MT76x2(pAd))
				vht_op.basic_mcs_set.mcs_ss1 = (((pAd->CommonCfg.vht_bw == VHT_BW_2040) 
					&& (pAd->CommonCfg.RegTransmitSetting.field.BW == BW_20)) ? VHT_MCS_CAP_8 : VHT_MCS_CAP_9);
			else
#endif
				vht_op.basic_mcs_set.mcs_ss1 = VHT_MCS_CAP_7;
			break;			
	}

#ifdef RT_BIG_ENDIAN
	//SWAP16((UINT16)vht_op.basic_mcs_set);
	NdisCopyMemory(&tmp,&vht_op.basic_mcs_set, 2);
	tmp=SWAP16(tmp);
	NdisCopyMemory(&vht_op.basic_mcs_set,&tmp, 2);
#endif /* RT_BIG_ENDIAN */
	NdisMoveMemory((UCHAR *)buf, (UCHAR *)&vht_op, sizeof(VHT_OP_IE));
	
	return sizeof(VHT_OP_IE);
}


/*
	Defined in IEEE 802.11AC

	Appeared in Beacon, (Re)AssocReq, (Re)AssocResp, ProbReq/Resp frames
*/
INT build_vht_cap_ie(RTMP_ADAPTER *pAd, UCHAR *buf)
{
	VHT_CAP_IE vht_cap_ie;
	INT rx_nss, tx_nss, mcs_cap;
#ifdef RT_BIG_ENDIAN
	UINT32 tmp_1;
	UINT64 tmp_2;
#endif /*RT_BIG_ENDIAN*/

	NdisZeroMemory((UCHAR *)&vht_cap_ie,  sizeof(VHT_CAP_IE));
	vht_cap_ie.vht_cap.max_mpdu_len = 0; // TODO: Ask Jerry about hardware limitation.
	vht_cap_ie.vht_cap.ch_width = 0; /* not support 160 or 80 + 80 MHz */

	if (pAd->CommonCfg.vht_ldpc && (pAd->chipCap.phy_caps & fPHY_CAP_LDPC))
		vht_cap_ie.vht_cap.rx_ldpc = 1;
	else
		vht_cap_ie.vht_cap.rx_ldpc = 0;

	vht_cap_ie.vht_cap.sgi_80M = pAd->CommonCfg.vht_sgi_80;
	vht_cap_ie.vht_cap.htc_vht_cap = 1;
	vht_cap_ie.vht_cap.max_ampdu_exp = 3; // TODO: Ask Jerry about the hardware limitation, currently set as 64K

	vht_cap_ie.vht_cap.tx_stbc = 0;
	vht_cap_ie.vht_cap.rx_stbc = 0;
	if (pAd->CommonCfg.vht_stbc)
	{
		if (pAd->CommonCfg.TxStream >= 2)
			vht_cap_ie.vht_cap.tx_stbc = 1;
		else
			vht_cap_ie.vht_cap.tx_stbc = 0;
		
		if (pAd->CommonCfg.RxStream >= 1)
			vht_cap_ie.vht_cap.rx_stbc = 1; // TODO: is it depends on the number of our antennas?
		else
			vht_cap_ie.vht_cap.rx_stbc = 0;
	}

#ifdef VHT_TXBF_SUPPORT

	if ((pAd->chipCap.FlgHwTxBfCap) && (pAd->BeaconSndDimensionFlag ==0))
	{
		vht_cap_ie.vht_cap.num_snd_dimension = pAd->CommonCfg.vht_cap_ie.vht_cap.num_snd_dimension;
   		vht_cap_ie.vht_cap.cmp_st_num_bfer= pAd->CommonCfg.vht_cap_ie.vht_cap.cmp_st_num_bfer;
		vht_cap_ie.vht_cap.bfee_cap_su=pAd->CommonCfg.vht_cap_ie.vht_cap.bfee_cap_su;
		vht_cap_ie.vht_cap.bfer_cap_su=pAd->CommonCfg.vht_cap_ie.vht_cap.bfer_cap_su;
	} 
    pAd->BeaconSndDimensionFlag =0; 
#endif

	vht_cap_ie.vht_cap.tx_ant_consistency = 1;
	vht_cap_ie.vht_cap.rx_ant_consistency = 1;

	vht_cap_ie.mcs_set.rx_mcs_map.mcs_ss1 = VHT_MCS_CAP_NA;
	vht_cap_ie.mcs_set.rx_mcs_map.mcs_ss2 = VHT_MCS_CAP_NA;
	vht_cap_ie.mcs_set.rx_mcs_map.mcs_ss3 = VHT_MCS_CAP_NA;
	vht_cap_ie.mcs_set.rx_mcs_map.mcs_ss4 = VHT_MCS_CAP_NA;
	vht_cap_ie.mcs_set.rx_mcs_map.mcs_ss5 = VHT_MCS_CAP_NA;
	vht_cap_ie.mcs_set.rx_mcs_map.mcs_ss6 = VHT_MCS_CAP_NA;
	vht_cap_ie.mcs_set.rx_mcs_map.mcs_ss7 = VHT_MCS_CAP_NA;
	vht_cap_ie.mcs_set.rx_mcs_map.mcs_ss8 = VHT_MCS_CAP_NA;

	vht_cap_ie.mcs_set.tx_mcs_map.mcs_ss1 = VHT_MCS_CAP_NA;
	vht_cap_ie.mcs_set.tx_mcs_map.mcs_ss2 = VHT_MCS_CAP_NA;
	vht_cap_ie.mcs_set.tx_mcs_map.mcs_ss3 = VHT_MCS_CAP_NA;
	vht_cap_ie.mcs_set.tx_mcs_map.mcs_ss4 = VHT_MCS_CAP_NA;
	vht_cap_ie.mcs_set.tx_mcs_map.mcs_ss5 = VHT_MCS_CAP_NA;
	vht_cap_ie.mcs_set.tx_mcs_map.mcs_ss6 = VHT_MCS_CAP_NA;
	vht_cap_ie.mcs_set.tx_mcs_map.mcs_ss7 = VHT_MCS_CAP_NA;
	vht_cap_ie.mcs_set.tx_mcs_map.mcs_ss8 = VHT_MCS_CAP_NA;

	mcs_cap = pAd->chipCap.max_vht_mcs;

	rx_nss = pAd->CommonCfg.RxStream;
	tx_nss = pAd->CommonCfg.TxStream;
#ifdef WFA_VHT_PF
	if ((pAd->CommonCfg.vht_nss_cap > 0) &&
		(pAd->CommonCfg.vht_nss_cap < pAd->CommonCfg.RxStream))
		rx_nss = pAd->CommonCfg.vht_nss_cap;

	if ((pAd->CommonCfg.vht_nss_cap > 0) && 
		(pAd->CommonCfg.vht_nss_cap < pAd->CommonCfg.TxStream))
		tx_nss = pAd->CommonCfg.vht_nss_cap;

	if (pAd->CommonCfg.vht_mcs_cap <pAd->chipCap.max_vht_mcs)
		mcs_cap = pAd->CommonCfg.vht_mcs_cap;
#endif /* WFA_VHT_PF */

	switch  (rx_nss)
	{
		case 1:
			vht_cap_ie.mcs_set.rx_high_rate = 292;
			vht_cap_ie.mcs_set.rx_mcs_map.mcs_ss1 = mcs_cap;
			break;
		case 2:
			if (mcs_cap == VHT_MCS_CAP_9)
				vht_cap_ie.mcs_set.rx_high_rate = 780;
			else
				vht_cap_ie.mcs_set.rx_high_rate = 585;

			vht_cap_ie.mcs_set.rx_mcs_map.mcs_ss1 = mcs_cap;
			vht_cap_ie.mcs_set.rx_mcs_map.mcs_ss2 = mcs_cap;
			break;
		default:
			vht_cap_ie.mcs_set.rx_high_rate = 0;
			break;
	}

	switch (tx_nss)
	{
		case 1:
			vht_cap_ie.mcs_set.tx_high_rate = 292;
			vht_cap_ie.mcs_set.tx_mcs_map.mcs_ss1 = mcs_cap;
			break;
		case 2:
			if (mcs_cap == VHT_MCS_CAP_9)
				vht_cap_ie.mcs_set.tx_high_rate = 780;
			else
				vht_cap_ie.mcs_set.tx_high_rate = 585;

			vht_cap_ie.mcs_set.tx_mcs_map.mcs_ss1 = mcs_cap;
			vht_cap_ie.mcs_set.tx_mcs_map.mcs_ss2 = mcs_cap;
			break;
		default:
			vht_cap_ie.mcs_set.tx_high_rate = 0;
			break;
	}

#ifdef RT_BIG_ENDIAN
	NdisCopyMemory(&tmp_1,&vht_cap_ie.vht_cap, 4);
	tmp_1 = SWAP32(tmp_1);
	NdisCopyMemory(&vht_cap_ie.vht_cap,&tmp_1, 4);
	
	NdisCopyMemory(&tmp_2,&vht_cap_ie.mcs_set, 8);	
	tmp_2=SWAP64(tmp_2);
	NdisCopyMemory(&vht_cap_ie.mcs_set,&tmp_2, 8);

	//hex_dump("&vht_cap_ie", &vht_cap_ie,  sizeof(VHT_CAP_IE));
	//SWAP32((UINT32)vht_cap_ie.vht_cap);
	//SWAP32((UINT32)vht_cap_ie.mcs_set);
#endif /* RT_BIG_ENDIAN */

	NdisMoveMemory(buf, (UCHAR *)&vht_cap_ie, sizeof(VHT_CAP_IE));

	return sizeof(VHT_CAP_IE);
}


INT build_vht_ies(RTMP_ADAPTER *pAd, UCHAR *buf, UCHAR frm)
{
	INT len = 0;
	EID_STRUCT eid_hdr;


	eid_hdr.Eid = IE_VHT_CAP;
	eid_hdr.Len = sizeof(VHT_CAP_IE);
	NdisMoveMemory(buf, (UCHAR *)&eid_hdr, 2);
	len = 2;

	len += build_vht_cap_ie(pAd, (UCHAR *)(buf + len));
	if (frm == SUBTYPE_BEACON || frm == SUBTYPE_PROBE_RSP ||
		frm == SUBTYPE_ASSOC_RSP || frm == SUBTYPE_REASSOC_RSP)
	{
		eid_hdr.Eid = IE_VHT_OP;
		eid_hdr.Len = sizeof(VHT_OP_IE);
		NdisMoveMemory((UCHAR *)(buf + len), (UCHAR *)&eid_hdr, 2);
		len +=2;

		len += build_vht_op_ie(pAd, (UCHAR *)(buf + len));
	}
	
	return len;
}

BOOLEAN vht80_channel_group( RTMP_ADAPTER *pAd, UCHAR channel)
{
	INT idx = 0;

	if (channel <= 14)
		return FALSE;
	
	while (vht_ch_80M[idx].ch_up_bnd != 0)
	{
		if (channel >= vht_ch_80M[idx].ch_low_bnd &&
			channel <= vht_ch_80M[idx].ch_up_bnd)
		{
			if ( (pAd->CommonCfg.RDDurRegion == JAP ||
				pAd->CommonCfg.RDDurRegion == JAP_W53 ||
				pAd->CommonCfg.RDDurRegion == JAP_W56 ||
				pAd->CommonCfg.RDDurRegion == CE
				) &&
				vht_ch_80M[idx].cent_freq_idx == 138)
			{
				idx++;
				continue;
			}

			return TRUE;
		}
		idx++;
	}

	return FALSE;
}

