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
	Who		When			What
	--------	----------		----------------------------------------------
*/


#include "rt_config.h"


struct vht_ch_layout {
	UCHAR ch_low_bnd;
	UCHAR ch_up_bnd;
	UCHAR cent_freq_idx;
};

static struct vht_ch_layout vht_ch_80M[] = {
	{36, 48, 42},
	{52, 64, 58},
	{100, 112, 106},
	{116, 128, 122},
	{132, 144, 138},
	{149, 161, 155},
	{0, 0, 0},
};

static struct  vht_ch_layout vht_ch_160M[] = {
	{36, 64, 50},
	{100, 128, 114},
	{0, 0, 0},
};

struct vht_ch_layout *get_ch_array(UINT8 bw)
{
	if (bw == 80)
		return vht_ch_80M;
	else
		return vht_ch_160M;
}

VOID dump_vht_cap(RTMP_ADAPTER *pAd, VHT_CAP_IE *vht_ie)
{
	VHT_CAP_INFO *vht_cap = &vht_ie->vht_cap;
	VHT_MCS_SET *vht_mcs = &vht_ie->mcs_set;

	MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Dump VHT_CAP IE\n"));
	hex_dump("VHT CAP IE Raw Data", (UCHAR *)vht_ie, sizeof(VHT_CAP_IE));
	MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("VHT Capabilities Info Field\n"));
	MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tMaximum MPDU Length=%d\n", vht_cap->max_mpdu_len));
	MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tSupported Channel Width=%d\n", vht_cap->ch_width));
	MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tRxLDPC=%d\n", vht_cap->rx_ldpc));
	MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tShortGI_80M=%d\n", vht_cap->sgi_80M));
	MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tShortGI_160M=%d\n", vht_cap->sgi_160M));
	MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tTxSTBC=%d\n", vht_cap->tx_stbc));
	MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tRxSTBC=%d\n", vht_cap->rx_stbc));
	MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tSU BeamformerCap=%d\n", vht_cap->bfer_cap_su));
	MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tSU BeamformeeCap=%d\n", vht_cap->bfee_cap_su));
	MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tCompressedSteeringNumOfBeamformerAnt=%d\n", vht_cap->bfee_sts_cap));
	MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tNumber of Sounding Dimensions=%d\n", vht_cap->num_snd_dimension));
	MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tMU BeamformerCap=%d\n", vht_cap->bfer_cap_mu));
	MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tMU BeamformeeCap=%d\n", vht_cap->bfee_cap_mu));
	MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tVHT TXOP PS=%d\n", vht_cap->vht_txop_ps));
	MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t+HTC-VHT Capable=%d\n", vht_cap->htc_vht_cap));
	MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tMaximum A-MPDU Length Exponent=%d\n", vht_cap->max_ampdu_exp));
	MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tVHT LinkAdaptation Capable=%d\n", vht_cap->vht_link_adapt));
	MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("VHT Supported MCS Set Field\n"));
	MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tRx Highest SupDataRate=%d\n", vht_mcs->rx_high_rate));
	MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tRxMCS Map_1SS=%d\n", vht_mcs->rx_mcs_map.mcs_ss1));
	MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tRxMCS Map_2SS=%d\n", vht_mcs->rx_mcs_map.mcs_ss2));
	MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tTx Highest SupDataRate=%d\n", vht_mcs->tx_high_rate));
	MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tTxMCS Map_1SS=%d\n", vht_mcs->tx_mcs_map.mcs_ss1));
	MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tTxMCS Map_2SS=%d\n", vht_mcs->tx_mcs_map.mcs_ss2));
}


VOID dump_vht_op(RTMP_ADAPTER *pAd, VHT_OP_IE *vht_ie)
{
	VHT_OP_INFO *vht_op = &vht_ie->vht_op_info;
	VHT_MCS_MAP *vht_mcs = &vht_ie->basic_mcs_set;

	MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Dump VHT_OP IE\n"));
	hex_dump("VHT OP IE Raw Data", (UCHAR *)vht_ie, sizeof(VHT_OP_IE));
	MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("VHT Operation Info Field\n"));
	MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tChannelWidth=%d\n", vht_op->ch_width));
	MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tChannelCenterFrequency Seg 1=%d\n", vht_op->center_freq_1));
	MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tChannelCenterFrequency Seg 1=%d\n", vht_op->center_freq_2));
	MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("VHT Basic MCS Set Field\n"));
	MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tRxMCS Map_1SS=%d\n", vht_mcs->mcs_ss1));
	MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tRxMCS Map_2SS=%d\n", vht_mcs->mcs_ss2));
}


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

	NdisZeroMemory(buf, MAX_MGMT_PKT_LEN);
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

		if (frm_len >= (MAX_MGMT_PKT_LEN - sizeof(SNDING_STA_INFO))) {
			MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): len(%d) too large!cnt=%d\n",
					 __func__, frm_len, sta_cnt));
			break;
		}
	}

	if (entry->snd_dialog_token & 0xc0)
		entry->snd_dialog_token = 0;
	else
		entry->snd_dialog_token++;

	vht_ndpa->duration = pAd->CommonCfg.Dsifs +
						 RTMPCalcDuration(pAd, pAd->CommonCfg.MlmeRate, frm_len);
	MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Send VHT NDPA Frame to STA(%02x:%02x:%02x:%02x:%02x:%02x)\n",
			 PRINT_MAC(entry->Addr)));
	hex_dump("VHT NDPA Frame", buf, frm_len);
	MiniportMMRequest(pAd, 0, buf, frm_len);
	MlmeFreeMemory(buf);
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
		MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Send sounding QoSNULL Frame to STA(%02x:%02x:%02x:%02x:%02x:%02x)\n",
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
UCHAR vht_prim_ch_idx(UCHAR vht_cent_ch, UCHAR prim_ch, UINT8 rf_bw)
{
	INT idx = 0;
	UCHAR bbp_idx = 0;

	if (vht_cent_ch == prim_ch)
		goto done;

	if (rf_bw == RF_BW_80) {
		while (vht_ch_80M[idx].ch_up_bnd != 0) {
			if (vht_cent_ch == vht_ch_80M[idx].cent_freq_idx) {
				if (prim_ch == vht_ch_80M[idx].ch_up_bnd)
					bbp_idx = 3;
				else if (prim_ch == vht_ch_80M[idx].ch_low_bnd)
					bbp_idx = 0;
				else
					bbp_idx = prim_ch > vht_cent_ch ? 2 : 1;

				break;
			}

			idx++;
		}
	} else if (rf_bw == RF_BW_160) {
		/* TODO: Shiang-MT7615, fix me!! */
		while (vht_ch_160M[idx].ch_up_bnd != 0) {
			if (vht_cent_ch == vht_ch_160M[idx].cent_freq_idx) {
				if (prim_ch == vht_ch_160M[idx].ch_up_bnd)
					bbp_idx = 3;
				else if (prim_ch == vht_ch_160M[idx].ch_low_bnd)
					bbp_idx = 0;
				else
					bbp_idx = prim_ch > vht_cent_ch ? 2 : 1;

				break;
			}

			idx++;
		}
	}

done:
	MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s():(VhtCentCh=%d, PrimCh=%d) =>BbpChIdx=%d\n",
			  __func__, vht_cent_ch, prim_ch, bbp_idx));
	return bbp_idx;
}


/*
	Currently we only consider about VHT 80MHz!
*/
UCHAR vht_cent_ch_freq(UCHAR prim_ch, UCHAR vht_bw)
{
	INT idx = 0;

	if (vht_bw == VHT_BW_2040 || prim_ch < 36)
		return prim_ch;
	else if ((vht_bw == VHT_BW_80) || (vht_bw == VHT_BW_8080)) {
		while (vht_ch_80M[idx].ch_up_bnd != 0) {
			if (prim_ch >= vht_ch_80M[idx].ch_low_bnd &&
				prim_ch <= vht_ch_80M[idx].ch_up_bnd)
				return vht_ch_80M[idx].cent_freq_idx;

			idx++;
		}
	} else if (vht_bw == VHT_BW_160) {
		while (vht_ch_160M[idx].ch_up_bnd != 0) {
			if (prim_ch >= vht_ch_160M[idx].ch_low_bnd &&
				prim_ch <= vht_ch_160M[idx].ch_up_bnd)
				return vht_ch_160M[idx].cent_freq_idx;

			idx++;
		}
	}

	return prim_ch;
}


UINT16 vht_max_mpdu_size[3] = {3839u, 7935u, 11454u};

INT vht_mode_adjust(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry, VHT_CAP_IE *cap, VHT_OP_IE *op)
{
	RT_PHY_INFO *ht_phyinfo;
	struct wifi_dev *wdev = pEntry->wdev;
	ADD_HT_INFO_IE *addht;
	UCHAR vht_sgi = 0;

	pEntry->MaxHTPhyMode.field.MODE = MODE_VHT;

	if (!wdev)
		return FALSE;

	addht = wlan_operate_get_addht(wdev);
	pAd->MacTab.fAnyStationNonGF = TRUE;
	vht_sgi = wlan_config_get_vht_sgi(wdev);

	/*
	if (op->vht_op_info.ch_width >= 1 && pEntry->MaxHTPhyMode.field.BW == BW_40)
	{
		pEntry->MaxHTPhyMode.field.BW= BW_80;
		pEntry->MaxHTPhyMode.field.ShortGI = (cap->vht_cap.sgi_80M);
		pEntry->MaxHTPhyMode.field.STBC = (cap->vht_cap.rx_stbc > 1 ? 1 : 0);
	}
	*/
	if ((pEntry->MaxHTPhyMode.field.BW == BW_40) && (pEntry->wdev)) {
		ht_phyinfo = &pEntry->wdev->DesiredHtPhyInfo;

		if (ht_phyinfo) {
			MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: DesiredHtPhyInfo->vht_bw=%d, ch_width=%d\n", __func__,
					 ht_phyinfo->vht_bw, cap->vht_cap.ch_width));

			if ((ht_phyinfo->vht_bw == VHT_BW_2040)) {
				pEntry->MaxHTPhyMode.field.ShortGI = (vht_sgi & (cap->vht_cap.sgi_80M));
				pEntry->MaxHTPhyMode.field.STBC = ((wlan_config_get_vht_stbc(pEntry->wdev) & cap->vht_cap.rx_stbc) > 1 ? 1 : 0);
			} else if ((ht_phyinfo->vht_bw >= VHT_BW_80) && (cap->vht_cap.ch_width == 0)) {
				if (op != NULL) {
					if (op->vht_op_info.ch_width == 0)  /* peer support VHT20,40 */
						pEntry->MaxHTPhyMode.field.BW = BW_40;
					else
						pEntry->MaxHTPhyMode.field.BW = BW_80;
				} else {
					/* can not know peer capability, use it's maximum capability */
					pEntry->MaxHTPhyMode.field.BW = BW_80;
				}
				pEntry->MaxHTPhyMode.field.ShortGI = (vht_sgi & (cap->vht_cap.sgi_80M));
				pEntry->MaxHTPhyMode.field.STBC = ((wlan_config_get_vht_stbc(pEntry->wdev) & cap->vht_cap.rx_stbc) > 1 ? 1 : 0);
			} else if ((ht_phyinfo->vht_bw == VHT_BW_80) && (cap->vht_cap.ch_width != 0)) {
				/* bw80 */
				pEntry->MaxHTPhyMode.field.BW = BW_80;
				pEntry->MaxHTPhyMode.field.ShortGI = (vht_sgi & (cap->vht_cap.sgi_80M));
				pEntry->MaxHTPhyMode.field.STBC = ((wlan_config_get_vht_stbc(pEntry->wdev) & cap->vht_cap.rx_stbc) > 1 ? 1 : 0);
			} else if (((ht_phyinfo->vht_bw == VHT_BW_160) || (ht_phyinfo->vht_bw == VHT_BW_8080)) &&
					   (cap->vht_cap.ch_width != 0)) {
				/* bw160 or bw80+80 */
				pEntry->MaxHTPhyMode.field.BW = BW_160;
				pEntry->MaxHTPhyMode.field.ShortGI = (vht_sgi & (cap->vht_cap.sgi_160M));
				pEntry->MaxHTPhyMode.field.STBC = ((wlan_config_get_vht_stbc(pEntry->wdev) & cap->vht_cap.rx_stbc) > 1 ? 1 : 0);
			}
		}
	}

	return TRUE;
}

VOID set_vht_cap(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *entry, VHT_CAP_IE *vht_cap_ie)
{
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (wlan_config_get_vht_ldpc(entry->wdev) && (cap->phy_caps & fPHY_CAP_LDPC) &&
		(vht_cap_ie->vht_cap.rx_ldpc))
		CLIENT_STATUS_SET_FLAG(entry, fCLIENT_STATUS_VHT_RX_LDPC_CAPABLE);

	CLIENT_STATUS_SET_FLAG(entry, fCLIENT_STATUS_VHT_CAPABLE);

	if (wlan_config_get_vht_sgi(entry->wdev) == GI_400) {
		if (vht_cap_ie->vht_cap.sgi_80M)
			CLIENT_STATUS_SET_FLAG(entry, fCLIENT_STATUS_SGI80_CAPABLE);

		if (vht_cap_ie->vht_cap.sgi_160M)
			CLIENT_STATUS_SET_FLAG(entry, fCLIENT_STATUS_SGI160_CAPABLE);
	}

	if (wlan_config_get_vht_stbc(entry->wdev)) {
		if (vht_cap_ie->vht_cap.tx_stbc)
			CLIENT_STATUS_SET_FLAG(entry, fCLIENT_STATUS_VHT_TXSTBC_CAPABLE);

		if (vht_cap_ie->vht_cap.rx_stbc)
			CLIENT_STATUS_SET_FLAG(entry, fCLIENT_STATUS_VHT_RXSTBC_CAPABLE);
	}

	entry->MaxRAmpduFactor = (vht_cap_ie->vht_cap.max_ampdu_exp > entry->MaxRAmpduFactor) ?
		vht_cap_ie->vht_cap.max_ampdu_exp : entry->MaxRAmpduFactor;
	entry->AMsduSize = vht_cap_ie->vht_cap.max_mpdu_len;

	if (entry->AMsduSize < (sizeof(vht_max_mpdu_size) / sizeof(vht_max_mpdu_size[0])))
		entry->amsdu_limit_len = vht_max_mpdu_size[entry->AMsduSize];
	else
		entry->amsdu_limit_len = 0;
	entry->amsdu_limit_len_adjust = entry->amsdu_limit_len;
}

static UCHAR dot11_vht_mcs_bw_cap[] = {
	8,		8,		9,		8, /* BW20, 1SS~4SS */
	9,		9,		9,		9, /* BW40, 1SS~4SS */
	9,		9,		9,		9, /* BW80, 1SS~4SS */
	9,		9,		8,		9, /* BW160, 1SS~4SS */
};

static UCHAR spec_cap_to_mcs[] = {
	7,  /* VHT_MCS_CAP_7 */
	8, /* VHT_MCS_CAP_8 */
	9, /* VHT_MCS_CAP_9 */
	0, /* VHT_MCS_CAP_NA */
};

INT dot11_vht_mcs_to_internal_mcs(
	struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	VHT_CAP_IE *vht_cap,
	HTTRANSMIT_SETTING *tx)
{
	HTTRANSMIT_SETTING *tx_mode = tx;
	UCHAR spec_cap, peer_cap, cap_offset = 0, nss;
	/* TODO: implement get_vht_max_mcs to get peer max MCS */

	switch (tx_mode->field.BW) {
	case BW_20:
		cap_offset = 0;
		break;

	case BW_40:
		cap_offset = 4;
		break;

	case BW_80:
		cap_offset = 8;
		break;

	case BW_160:
		cap_offset = 12;
		break;

	default:
		cap_offset = 0;
		break;
	};

	if ((vht_cap->mcs_set.rx_mcs_map.mcs_ss1 != VHT_MCS_CAP_NA) && (wlan_operate_get_tx_stream(wdev) >= 1)) {
		nss = 1;
		spec_cap = dot11_vht_mcs_bw_cap[cap_offset + (nss - 1)];
		peer_cap = spec_cap_to_mcs[vht_cap->mcs_set.rx_mcs_map.mcs_ss1];

		if (peer_cap < spec_cap)
			tx_mode->field.MCS = ((nss - 1) << 4) | peer_cap;
		else
			tx_mode->field.MCS = ((nss - 1) << 4) | spec_cap;
	}

	if ((vht_cap->mcs_set.rx_mcs_map.mcs_ss2 != VHT_MCS_CAP_NA) && (wlan_operate_get_tx_stream(wdev) >= 2)) {
		nss = 2;
		spec_cap = dot11_vht_mcs_bw_cap[cap_offset + (nss - 1)];
		peer_cap = spec_cap_to_mcs[vht_cap->mcs_set.rx_mcs_map.mcs_ss2];

		if (peer_cap < spec_cap)
			tx_mode->field.MCS = ((nss - 1) << 4) | peer_cap;
		else
			tx_mode->field.MCS = ((nss - 1) << 4) | spec_cap;
	}

	if ((vht_cap->mcs_set.rx_mcs_map.mcs_ss3 != VHT_MCS_CAP_NA) && (wlan_operate_get_tx_stream(wdev) >= 3)) {
		nss = 3;
		spec_cap = dot11_vht_mcs_bw_cap[cap_offset + (nss - 1)];
		peer_cap = spec_cap_to_mcs[vht_cap->mcs_set.rx_mcs_map.mcs_ss3];

		if (peer_cap < spec_cap)
			tx_mode->field.MCS = ((nss - 1) << 4) | peer_cap;
		else
			tx_mode->field.MCS = ((nss - 1) << 4) | spec_cap;
	}

	if ((vht_cap->mcs_set.rx_mcs_map.mcs_ss4 != VHT_MCS_CAP_NA) && (wlan_operate_get_tx_stream(wdev) >= 4)) {
		nss = 4;
		spec_cap = dot11_vht_mcs_bw_cap[cap_offset + (nss - 1)];
		peer_cap = spec_cap_to_mcs[vht_cap->mcs_set.rx_mcs_map.mcs_ss4];

		if (peer_cap < spec_cap)
			tx_mode->field.MCS = ((nss - 1) << 4) | peer_cap;
		else
			tx_mode->field.MCS = ((nss - 1) << 4) | spec_cap;
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
INT build_vht_txpwr_envelope(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *buf)
{
	INT len = 0, pwr_cnt;
	VHT_TXPWR_ENV_IE txpwr_env;
	UCHAR vht_bw = wlan_operate_get_vht_bw(wdev);
	UCHAR ht_bw = wlan_operate_get_ht_bw(wdev);

	NdisZeroMemory(&txpwr_env, sizeof(txpwr_env));

	if ((vht_bw == VHT_BW_160)
		|| (vht_bw == VHT_BW_8080))
		pwr_cnt = 3;
	else if (vht_bw == VHT_BW_80)
		pwr_cnt = 2;
	else {
		if (ht_bw == HT_BW_40)
			pwr_cnt = 1;
		else
			pwr_cnt = 0;
	}

	txpwr_env.tx_pwr_info.max_tx_pwr_cnt = pwr_cnt;
	txpwr_env.tx_pwr_info.max_tx_pwr_interpretation = TX_PWR_INTERPRET_EIRP;

	/* TODO: fixme, we need the real tx_pwr value for each port. */
	for (len = 0; len <= pwr_cnt; len++)
		txpwr_env.tx_pwr_bw[len] = 30; /* 15dB */

	len = 2 + pwr_cnt;
	NdisMoveMemory(buf, &txpwr_env, len);
	return len;
}


/*
	Defined in IEEE 802.11AC

	Appeared in Beacon, (Re)AssocResp, ProbResp frames
*/
INT build_vht_op_ie(RTMP_ADAPTER *pAd, UCHAR bw, UCHAR Channel, struct wifi_dev *wdev, UCHAR *buf)
{
	VHT_OP_IE vht_op;
	UCHAR cent_ch;
#ifdef RT_BIG_ENDIAN
	UINT16 tmp;
#endif /* RT_BIG_ENDIAN */
	UCHAR cen_ch_2;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 max_vht_mcs = cap->max_vht_mcs;
#ifdef CONFIG_AP_SUPPORT
	struct DOT11_H *pDot11h = NULL;
#endif

	if (wdev == NULL)
		return FALSE;

	cen_ch_2 = wlan_operate_get_cen_ch_2(wdev);

	NdisZeroMemory((UCHAR *)&vht_op, sizeof(VHT_OP_IE));
#ifdef CONFIG_AP_SUPPORT
	pDot11h = wdev->pDot11_H;
	if (pDot11h == NULL)
		return FALSE;

	if (Channel > 14 &&
		(pAd->CommonCfg.bIEEE80211H == 1) &&
		(pDot11h->RDMode == RD_SWITCHING_MODE))
		cent_ch = vht_cent_ch_freq(pDot11h->org_ch, bw);
	else
#endif /* CONFIG_AP_SUPPORT */
		cent_ch = vht_cent_ch_freq(Channel, bw);

	switch (bw) {
	case VHT_BW_2040:
		vht_op.vht_op_info.ch_width = 0;
		vht_op.vht_op_info.center_freq_1 = 0;
		vht_op.vht_op_info.center_freq_2 = 0;
		break;

	case VHT_BW_80:
		vht_op.vht_op_info.ch_width = 1;
		vht_op.vht_op_info.center_freq_1 = cent_ch;
		vht_op.vht_op_info.center_freq_2 = 0;
		break;

	case VHT_BW_160:
		vht_op.vht_op_info.ch_width = 1;
		vht_op.vht_op_info.center_freq_1 = (cent_ch - 8);
		vht_op.vht_op_info.center_freq_2 = cent_ch;
		break;

	case VHT_BW_8080:
		vht_op.vht_op_info.ch_width = 1;
		vht_op.vht_op_info.center_freq_1 = cent_ch;
		vht_op.vht_op_info.center_freq_2 = cen_ch_2;
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

	switch  (wlan_operate_get_rx_stream(wdev)) {
	case 4:
		vht_op.basic_mcs_set.mcs_ss4 = max_vht_mcs;
	case 3:
		vht_op.basic_mcs_set.mcs_ss3 = max_vht_mcs;
	case 2:
		vht_op.basic_mcs_set.mcs_ss2 = max_vht_mcs;
	case 1:
		vht_op.basic_mcs_set.mcs_ss1 = max_vht_mcs;
		break;
	}

#ifdef RT_BIG_ENDIAN
	/* SWAP16((UINT16)vht_op.basic_mcs_set); */
	NdisCopyMemory(&tmp, &vht_op.basic_mcs_set, sizeof(VHT_MCS_MAP));
	tmp = SWAP16(tmp);
	NdisCopyMemory(&vht_op.basic_mcs_set, &tmp, sizeof(VHT_MCS_MAP));
#endif /* RT_BIG_ENDIAN */
	NdisMoveMemory((UCHAR *)buf, (UCHAR *)&vht_op, sizeof(VHT_OP_IE));
	/* hex_dump("vht op info", (UCHAR *)&vht_op, sizeof(VHT_OP_IE)); */
	return sizeof(VHT_OP_IE);
}


/*
	Defined in IEEE 802.11AC

	Appeared in Beacon, (Re)AssocReq, (Re)AssocResp, ProbReq/Resp frames
*/
static UINT16 VHT_HIGH_RATE_BW80[3][4] = {
	{292, 585, 877, 1170},
	{351, 702, 1053, 1404},
	{390, 780, 1170, 1560},
};


INT build_vht_cap_ie(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *buf)
{
	VHT_CAP_IE vht_cap_ie;
	INT rx_nss, tx_nss, mcs_cap;
	UCHAR cap_vht_bw = wlan_config_get_vht_bw(wdev);
#ifdef RT_BIG_ENDIAN
	UINT32 tmp_1;
	UINT64 tmp_2;
#endif /*RT_BIG_ENDIAN*/
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	NdisZeroMemory((UCHAR *)&vht_cap_ie,  sizeof(VHT_CAP_IE));
	vht_cap_ie.vht_cap.max_mpdu_len = cap->max_mpdu_len; /* TODO: Ask Jerry about hardware limitation. */

	if (cap_vht_bw == VHT_BW_160) {
		vht_cap_ie.vht_cap.ch_width = 1;
		vht_cap_ie.vht_cap.sgi_160M = wlan_config_get_vht_sgi(wdev);
	} else if (cap_vht_bw == VHT_BW_8080) {
		vht_cap_ie.vht_cap.ch_width = 2;
		vht_cap_ie.vht_cap.sgi_160M = wlan_config_get_vht_sgi(wdev);
	} else
		vht_cap_ie.vht_cap.ch_width = 0;

	if (wlan_config_get_vht_ldpc(wdev) && (cap->phy_caps & fPHY_CAP_LDPC))
		vht_cap_ie.vht_cap.rx_ldpc = 1;
	else
		vht_cap_ie.vht_cap.rx_ldpc = 0;

	vht_cap_ie.vht_cap.sgi_80M = wlan_config_get_vht_sgi(wdev);
	vht_cap_ie.vht_cap.htc_vht_cap = 1;
	vht_cap_ie.vht_cap.max_ampdu_exp = cap->vht_max_ampdu_len_exp;
	vht_cap_ie.vht_cap.tx_stbc = 0;
	vht_cap_ie.vht_cap.rx_stbc = 0;

	if (wlan_config_get_vht_stbc(wdev)) {
		UINT8   ucTxPath = pAd->Antenna.field.TxPath;

#ifdef DBDC_MODE
		if (pAd->CommonCfg.dbdc_mode) {
			UINT8 band_idx = HcGetBandByWdev(wdev);

			if (band_idx == DBDC_BAND0)
				ucTxPath = pAd->dbdc_band0_tx_path;
			else
				ucTxPath = pAd->dbdc_band1_tx_path;
		}
#endif

		if (ucTxPath >= 2)
			vht_cap_ie.vht_cap.tx_stbc = 1;
		else
			vht_cap_ie.vht_cap.tx_stbc = 0;

		vht_cap_ie.vht_cap.rx_stbc = 1;
	}

	vht_cap_ie.vht_cap.tx_ant_consistency = 1;
	vht_cap_ie.vht_cap.rx_ant_consistency = 1;
#ifdef VHT_TXBF_SUPPORT

	if (cap->FlgHwTxBfCap) {
		VHT_CAP_INFO vht_cap;

		NdisCopyMemory(&vht_cap, &pAd->CommonCfg.vht_cap_ie.vht_cap, sizeof(VHT_CAP_INFO));
		mt_WrapSetVHTETxBFCap(pAd, wdev, &vht_cap);
		vht_cap_ie.vht_cap.num_snd_dimension = vht_cap.num_snd_dimension;
		vht_cap_ie.vht_cap.bfee_sts_cap      = vht_cap.bfee_sts_cap;
		vht_cap_ie.vht_cap.bfee_cap_su       = vht_cap.bfee_cap_su;
		vht_cap_ie.vht_cap.bfer_cap_su       = vht_cap.bfer_cap_su;
#ifdef MT_MAC
		vht_cap_ie.vht_cap.bfee_cap_mu       = vht_cap.bfee_cap_mu;
		vht_cap_ie.vht_cap.bfer_cap_mu       = vht_cap.bfer_cap_mu;
#endif
	}

#endif
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
	mcs_cap = cap->max_vht_mcs;
	rx_nss = wlan_operate_get_rx_stream(wdev);
	tx_nss = wlan_operate_get_tx_stream(wdev);
#ifdef WFA_VHT_PF

	if ((pAd->CommonCfg.vht_nss_cap > 0) &&
		(pAd->CommonCfg.vht_nss_cap < wlan_operate_get_rx_stream(wdev)))
		rx_nss = pAd->CommonCfg.vht_nss_cap;

	if ((pAd->CommonCfg.vht_nss_cap > 0) &&
		(pAd->CommonCfg.vht_nss_cap < wlan_operate_get_tx_stream(wdev)))
		tx_nss = pAd->CommonCfg.vht_nss_cap;

	if (pAd->CommonCfg.vht_mcs_cap < cap->max_vht_mcs)
		mcs_cap = pAd->CommonCfg.vht_mcs_cap;

#endif /* WFA_VHT_PF */

	if ((mcs_cap <= VHT_MCS_CAP_9)
		&& ((rx_nss > 0) && (rx_nss <= 4))
		&& ((tx_nss > 0) && (tx_nss <= 4))) {
		vht_cap_ie.mcs_set.rx_high_rate = VHT_HIGH_RATE_BW80[mcs_cap][rx_nss - 1];
		vht_cap_ie.mcs_set.tx_high_rate = VHT_HIGH_RATE_BW80[mcs_cap][tx_nss - 1];

		if (rx_nss >= 1)
			vht_cap_ie.mcs_set.rx_mcs_map.mcs_ss1 = mcs_cap;

		if (rx_nss >= 2)
			vht_cap_ie.mcs_set.rx_mcs_map.mcs_ss2 = mcs_cap;

		if (rx_nss >= 3)
			vht_cap_ie.mcs_set.rx_mcs_map.mcs_ss3 = mcs_cap;

		if (rx_nss >= 4)
			vht_cap_ie.mcs_set.rx_mcs_map.mcs_ss4 = mcs_cap;

		if (tx_nss >= 1)
			vht_cap_ie.mcs_set.tx_mcs_map.mcs_ss1 = mcs_cap;

		if (tx_nss >= 2)
			vht_cap_ie.mcs_set.tx_mcs_map.mcs_ss2 = mcs_cap;

		if (tx_nss >= 3)
			vht_cap_ie.mcs_set.tx_mcs_map.mcs_ss3 = mcs_cap;

		if (tx_nss >= 4)
			vht_cap_ie.mcs_set.tx_mcs_map.mcs_ss4 = mcs_cap;
	} else {
		vht_cap_ie.mcs_set.rx_high_rate = 0;
		vht_cap_ie.mcs_set.tx_high_rate = 0;
	}

#ifdef RT_BIG_ENDIAN
	NdisCopyMemory(&tmp_1, &vht_cap_ie.vht_cap, sizeof(VHT_CAP_INFO));
	tmp_1 = SWAP32(tmp_1);
	NdisCopyMemory(&vht_cap_ie.vht_cap, &tmp_1, sizeof(VHT_CAP_INFO));
	NdisCopyMemory(&tmp_2, &vht_cap_ie.mcs_set, sizeof(VHT_MCS_SET));
	tmp_2 = SWAP64(tmp_2);
	NdisCopyMemory(&vht_cap_ie.mcs_set, &tmp_2, sizeof(VHT_MCS_SET));
	/* hex_dump("&vht_cap_ie", &vht_cap_ie,  sizeof(VHT_CAP_IE)); */
	/* SWAP32((UINT32)vht_cap_ie.vht_cap); */
	/* SWAP32((UINT32)vht_cap_ie.mcs_set); */
#endif
	NdisMoveMemory(buf, (UCHAR *)&vht_cap_ie, sizeof(VHT_CAP_IE));
	return sizeof(VHT_CAP_IE);
}

static INT build_vht_op_mode_ie(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *buf)
{
	INT len = 0;
	EID_STRUCT eid_hdr;
	OPERATING_MODE operating_mode_ie;
	UCHAR op_vht_bw = wlan_operate_get_vht_bw(wdev);

	NdisZeroMemory(&eid_hdr, sizeof(EID_STRUCT));
	NdisZeroMemory((UCHAR *)&operating_mode_ie,  sizeof(OPERATING_MODE));
	eid_hdr.Eid = IE_OPERATING_MODE_NOTIFY;
	eid_hdr.Len = sizeof(OPERATING_MODE);
	NdisMoveMemory(buf, (UCHAR *)&eid_hdr, 2);
	len = 2;
	operating_mode_ie.rx_nss_type = 0;
	operating_mode_ie.rx_nss = (wlan_operate_get_rx_stream(wdev) - 1);

	if (op_vht_bw == VHT_BW_2040) {
		if (wlan_operate_get_ht_bw(wdev) == HT_BW_40)
			operating_mode_ie.ch_width = 1;
		else
			operating_mode_ie.ch_width = 0;
	} else if (op_vht_bw == VHT_BW_80)
		operating_mode_ie.ch_width = 2;
	else if ((op_vht_bw == VHT_BW_160) ||
			 (op_vht_bw == VHT_BW_8080))
		operating_mode_ie.ch_width = 3;

	buf += len;
	NdisMoveMemory(buf, (UCHAR *)&operating_mode_ie, sizeof(OPERATING_MODE));
	len += eid_hdr.Len;
	return len;
}

#ifdef G_BAND_256QAM
static BOOLEAN g_band_256_qam_enable(
	struct _RTMP_ADAPTER *pAd, struct _build_ie_info *info)
{
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if ((pAd->CommonCfg.g_band_256_qam) &&
		(cap->g_band_256_qam) &&
		(WMODE_CAP(info->phy_mode, WMODE_GN)) &&
		(info->channel < 14))
		info->g_band_256_qam = TRUE;
	else
		info->g_band_256_qam = FALSE;

	return info->g_band_256_qam;
}
#endif /* G_BAND_256QAM */


INT build_vht_ies(RTMP_ADAPTER *pAd, struct _build_ie_info *info)
{
	INT len = 0;
	EID_STRUCT eid_hdr;
	UCHAR vht_bw = wlan_operate_get_vht_bw(info->wdev);
	UCHAR ht_bw = wlan_operate_get_ht_bw(info->wdev);

	if (
#if defined(G_BAND_256QAM)
		g_band_256_qam_enable(pAd, info) ||
#endif /* G_BAND_256QAM */
		(WMODE_CAP_AC(info->phy_mode) &&
		 (info->channel > 14))
	) {
		NdisZeroMemory(&eid_hdr, sizeof(EID_STRUCT));
		eid_hdr.Eid = IE_VHT_CAP;
		eid_hdr.Len = sizeof(VHT_CAP_IE);
		NdisMoveMemory(info->frame_buf, (UCHAR *)&eid_hdr, 2);
		len = 2;
		len += build_vht_cap_ie(pAd, info->wdev, (UCHAR *)(info->frame_buf + len));

		if ((info->frame_subtype == SUBTYPE_BEACON) ||
			(info->frame_subtype == SUBTYPE_PROBE_RSP) ||
			(info->frame_subtype == SUBTYPE_ASSOC_RSP) ||
			(info->frame_subtype == SUBTYPE_REASSOC_RSP)) {
			eid_hdr.Eid = IE_VHT_OP;
			eid_hdr.Len = sizeof(VHT_OP_IE);
			NdisMoveMemory((UCHAR *)(info->frame_buf + len), (UCHAR *)&eid_hdr, 2);
			len += 2;
#if defined(G_BAND_256QAM)
			vht_bw = (g_band_256_qam_enable(pAd, info)) ? VHT_BW_2040 : vht_bw;
#endif /* G_BAND_256QAM */
			len += build_vht_op_ie(pAd, vht_bw, info->channel, info->wdev, (UCHAR *)(info->frame_buf + len));
		} else if  ((info->frame_subtype == SUBTYPE_ASSOC_REQ) ||
					(info->frame_subtype == SUBTYPE_REASSOC_REQ)) {
			/* optional IE, Now only FOR VHT40 STA/ApClient with op_mode ie to notify AP
			   and avoid the BW info not sync.
			 */
			if ((vht_bw == VHT_BW_2040) &&
				(ht_bw == HT_BW_40))
				len += build_vht_op_mode_ie(pAd, info->wdev, (UCHAR *)(info->frame_buf + len));
		}
	}

	return len;
}


static UINT8 ch_offset_abs(UINT8 x, UINT8 y)
{
	if (x > y)
		return x - y;
	else
		return y - x;
}

void update_vht_op_info(UINT8 cap_bw, VHT_OP_INFO *vht_op_info, struct _op_info *op_info)
{
	UINT8 p80ccf = vht_op_info->center_freq_1;
	UINT8 s80160ccf = vht_op_info->center_freq_2;

	if (op_info == NULL)
		return;

	/*check op bw should below or equal the cap*/
	if (vht_op_info->ch_width > cap_bw)
		op_info->bw = cap_bw;
	else
		op_info->bw = vht_op_info->ch_width;

	switch (vht_op_info->ch_width) {
	case VHT_BW_2040:
		break;

	case VHT_BW_80:
		if (cap_bw == VHT_BW_80)
			op_info->cent_ch = p80ccf;
		else if (cap_bw > VHT_BW_80) {
			if (s80160ccf == 0)
				op_info->cent_ch = p80ccf;
			else if (ch_offset_abs(s80160ccf, p80ccf) == 8) {
				op_info->bw = VHT_BW_160;
				op_info->cent_ch = s80160ccf;
			} else if (ch_offset_abs(s80160ccf, p80ccf) > 16) {
				op_info->bw = VHT_BW_8080;
				op_info->cent_ch = p80ccf;
			}
		}

		break;

	case VHT_BW_160:
		if (cap_bw == VHT_BW_80)
			op_info->bw = VHT_BW_80;

		op_info->cent_ch = p80ccf;
		break;

	case VHT_BW_8080:
		if (cap_bw == VHT_BW_80)
			op_info->bw = VHT_BW_80;

		op_info->cent_ch = p80ccf;
		break;

	default:
		op_info->bw = VHT_BW_80;
		op_info->cent_ch = p80ccf;
		break;
	}

	MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s, bw=%u, cent_ch=%u\n", __func__, op_info->bw, op_info->cent_ch));
	return;
}

BOOLEAN vht80_channel_group(RTMP_ADAPTER *pAd, UCHAR channel)
{
	INT idx = 0;
	UCHAR region = GetCountryRegionFromCountryCode(pAd->CommonCfg.CountryCode);

	if (channel <= 14)
		return FALSE;

	while (vht_ch_80M[idx].ch_up_bnd != 0) {
		if (channel >= vht_ch_80M[idx].ch_low_bnd &&
			channel <= vht_ch_80M[idx].ch_up_bnd) {
			if (
				((pAd->CommonCfg.RDDurRegion == JAP ||
				  pAd->CommonCfg.RDDurRegion == JAP_W53 ||
				  pAd->CommonCfg.RDDurRegion == JAP_W56) &&
				 vht_ch_80M[idx].cent_freq_idx == 138)
				||
				((region == JAP || region == CE) &&
				 vht_ch_80M[idx].cent_freq_idx == 138)
			) {
				/* prevent using 132~144 while Region is JAP or CE */
				idx++;
				continue;
			}

			return TRUE;
		}

		idx++;
	}

	return FALSE;
}

BOOLEAN vht160_channel_group(RTMP_ADAPTER *pAd, UCHAR channel)
{
	INT idx = 0;
	/* UCHAR region = GetCountryRegionFromCountryCode(pAd->CommonCfg.CountryCode); */

	if (channel <= 14)
		return FALSE;

	while (vht_ch_160M[idx].ch_up_bnd != 0) {
		if (channel >= vht_ch_160M[idx].ch_low_bnd &&
			channel <= vht_ch_160M[idx].ch_up_bnd)
			return TRUE;

		idx++;
	}

	return FALSE;
}

void print_vht_op_info(VHT_OP_INFO *vht_op)
{
	char *ch_with[] = {"20/40M", "80M", "160M", "80+80M"};

	MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("VHT Operation Infomation: 0x%02X%02X%02X\n",
			  vht_op->ch_width, vht_op->center_freq_1, vht_op->center_freq_2));
	MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("     - Channel Width: %u (%s)\n",
			  vht_op->ch_width, ch_with[vht_op->ch_width]));
	MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("     - Channel Center Frequency Segment 0: %u\n",
			  vht_op->center_freq_1));
	MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("     - Channel Center Frequency Segment 1: %u\n",
			  vht_op->center_freq_2));
}

/*sta rec vht features decision*/
UINT32 starec_vht_feature_decision(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry, UINT32 *feature)
{
	/* struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER*)wdev->sys_handle; */
	UINT32 features = 0;

	if (CLIENT_STATUS_TEST_FLAG(entry, fCLIENT_STATUS_VHT_CAPABLE))
		features |= STA_REC_BASIC_VHT_INFO_FEATURE;

	/*return value, must use or operation*/
	*feature |= features;
	return TRUE;
}

UCHAR rf_bw_2_vht_bw(UCHAR rf_bw)
{
	UCHAR vht_bw = VHT_BW_2040;

	if (rf_bw == BW_80)
		vht_bw = VHT_BW_80;
	else if (rf_bw == BW_160)
		vht_bw = VHT_BW_160;
	else if (rf_bw == BW_8080)
		vht_bw = VHT_BW_8080;
	else
		vht_bw = VHT_BW_2040;

	return vht_bw;
}
