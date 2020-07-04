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
#ifdef TXRX_STAT_SUPPORT
#include "hdev/hdev_basic.h"
#endif

const UCHAR altx_filter_list[] = {
	SUBTYPE_ASSOC_REQ,
	SUBTYPE_ASSOC_RSP,
	SUBTYPE_REASSOC_REQ,
	SUBTYPE_REASSOC_RSP,
	SUBTYPE_PROBE_REQ,
	SUBTYPE_PROBE_RSP,
	SUBTYPE_ATIM,
	SUBTYPE_DISASSOC,
	SUBTYPE_AUTH,
	SUBTYPE_DEAUTH,
};

BOOLEAN in_altx_filter_list(UCHAR sub_type)
{
	UCHAR i;

	for (i = 0; i < (ARRAY_SIZE(altx_filter_list)); i++) {
		if (sub_type == altx_filter_list[i])
			return TRUE;
	}

	return FALSE;
}

const UCHAR wmm_aci_2_hw_ac_queue[] = {
	TxQ_IDX_AC1, /* ACI:0 AC_BE */
	TxQ_IDX_AC0, /* ACI:1 AC_BK */
	TxQ_IDX_AC2, /* ACI:2 AC_VI */
	TxQ_IDX_AC3, /* ACI:3 AC_VO */
	TxQ_IDX_AC11,
	TxQ_IDX_AC10,
	TxQ_IDX_AC12,
	TxQ_IDX_AC13,
	TxQ_IDX_AC21,
	TxQ_IDX_AC20,
	TxQ_IDX_AC22,
	TxQ_IDX_AC23,
	TxQ_IDX_AC31,
	TxQ_IDX_AC30,
	TxQ_IDX_AC32,
	TxQ_IDX_AC33,
	TxQ_IDX_ALTX0, /*16*/
	TxQ_IDX_BMC0,
	TxQ_IDX_BCN0,
	TxQ_IDX_PSMP0,
	TxQ_IDX_ALTX1, /*20*/
	TxQ_IDX_BMC1,
	TxQ_IDX_BCN1,
	TxQ_IDX_PSMP1,
};


VOID dump_rxinfo(RTMP_ADAPTER *pAd, RXINFO_STRUC *pRxInfo)
{
	hex_dump("RxInfo Raw Data", (UCHAR *)pRxInfo, sizeof(RXINFO_STRUC));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("RxInfo Fields:\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tBA=%d\n", pRxInfo->BA));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tDATA=%d\n", pRxInfo->DATA));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tNULLDATA=%d\n", pRxInfo->NULLDATA));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tFRAG=%d\n", pRxInfo->FRAG));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tU2M=%d\n", pRxInfo->U2M));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tMcast=%d\n", pRxInfo->Mcast));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tBcast=%d\n", pRxInfo->Bcast));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tMyBss=%d\n", pRxInfo->MyBss));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tCrc=%d\n", pRxInfo->Crc));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tCipherErr=%d\n", pRxInfo->CipherErr));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tAMSDU=%d\n", pRxInfo->AMSDU));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tHTC=%d\n", pRxInfo->HTC));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tRSSI=%d\n", pRxInfo->RSSI));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tL2PAD=%d\n", pRxInfo->L2PAD));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tAMPDU=%d\n", pRxInfo->AMPDU));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tDecrypted=%d\n", pRxInfo->Decrypted));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tBssIdx3=%d\n", pRxInfo->BssIdx3));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\twapi_kidx=%d\n", pRxInfo->wapi_kidx));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tpn_len=%d\n", pRxInfo->pn_len));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tsw_fc_type0=%d\n", pRxInfo->sw_fc_type0));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tsw_fc_type1=%d\n", pRxInfo->sw_fc_type1));
}


static char *hdr_fmt_str[] = {
	"Non-80211-Frame",
	"Command-Frame",
	"Normal-80211-Frame",
	"enhanced-80211-Frame",
};


static char *p_idx_str[] = {"LMAC", "MCU"};
static char *q_idx_lmac_str[] = {"WMM0_AC0", "WMM0_AC1", "WMM0_AC2", "WMM0_AC3",
								 "WMM1_AC0", "WMM1_AC1", "WMM1_AC2", "WMM1_AC3",
								 "WMM2_AC0", "WMM2_AC1", "WMM2_AC2", "WMM2_AC3",
								 "WMM3_AC0", "WMM3_AC1", "WMM3_AC2", "WMM3_AC3",
								 "Band0_ALTX", "Band0_BMC", "Band0_BNC", "Band0_PSMP",
								 "Band1_ALTX", "Band1_BMC", "Band1_BNC", "Band1_PSMP",
								 "Invalid"
								};
static char *q_idx_mcu_str[] = {"RQ0", "RQ1", "RQ2", "RQ3", "PDA", "Invalid"};
static char *pkt_ft_str[] = {"cut_through", "store_forward", "cmd", "PDA_FW_Download"};

VOID dump_tmac_info(RTMP_ADAPTER *pAd, UCHAR *tmac_info)
{
	TMAC_TXD_S *txd_s = (TMAC_TXD_S *)tmac_info;
	TMAC_TXD_0 *txd_0 = (TMAC_TXD_0 *)tmac_info;
	TMAC_TXD_1 *txd_1 = (TMAC_TXD_1 *)(tmac_info + sizeof(TMAC_TXD_0));
	UINT32 *txd_7 = &txd_s->TxD7;
	UCHAR q_idx = 0;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	hex_dump("TMAC_Info Raw Data: ", (UCHAR *)tmac_info, cap->TXWISize);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TMAC_TXD Fields:\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tTMAC_TXD_0:\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tPortID=%d(%s)\n", txd_0->p_idx, p_idx_str[txd_0->p_idx]));

	if (txd_0->p_idx == P_IDX_LMAC)
		q_idx = txd_0->q_idx % 0x18;
	else
		q_idx = ((txd_0->q_idx == TxQ_IDX_MCU_PDA) ? txd_0->q_idx : (txd_0->q_idx % 0x4));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tQueID=0x%x(%s %s)\n", txd_0->q_idx,
			 (txd_0->p_idx == P_IDX_LMAC ? "LMAC" : "MCU"),
			 txd_0->p_idx == P_IDX_LMAC ? q_idx_lmac_str[q_idx] : q_idx_mcu_str[q_idx]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tTxByteCnt=%d\n", txd_0->TxByteCount));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tTMAC_TXD_1:\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\twlan_idx=%d\n", txd_1->wlan_idx));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tHdrFmt=%d(%s)\n",
			 txd_1->hdr_format, hdr_fmt_str[txd_1->hdr_format]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tHdrInfo=0x%x\n", txd_1->hdr_info));

	switch (txd_1->hdr_format) {
	case TMI_HDR_FT_NON_80211:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\t\tMRD=%d, EOSP=%d, RMVL=%d, VLAN=%d, ETYP=%d\n",
				 txd_1->hdr_info & (1 << TMI_HDR_INFO_0_BIT_MRD),
				 txd_1->hdr_info & (1 << TMI_HDR_INFO_0_BIT_EOSP),
				 txd_1->hdr_info & (1 << TMI_HDR_INFO_0_BIT_RMVL),
				 txd_1->hdr_info & (1 << TMI_HDR_INFO_0_BIT_VLAN),
				 txd_1->hdr_info & (1 << TMI_HDR_INFO_0_BIT_ETYP)));
		break;

	case TMI_HDR_FT_CMD:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\t\tRsvd=0x%x\n", txd_1->hdr_info));
		break;

	case TMI_HDR_FT_NOR_80211:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\t\tHeader Len=%d(WORD)\n",
				 txd_1->hdr_info & TMI_HDR_INFO_2_MASK_LEN));
		break;

	case TMI_HDR_FT_ENH_80211:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\t\tEOSP=%d, AMS=%d\n",
				 txd_1->hdr_info & (1 << TMI_HDR_INFO_3_BIT_EOSP),
				 txd_1->hdr_info & (1 << TMI_HDR_INFO_3_BIT_AMS)));
		break;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tTxDFormatType=%d(%s format)\n", txd_1->ft,
			 txd_1->ft == TMI_FT_LONG ? "Long - 8 DWORD" : "Short - 3 DWORD"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\ttxd_len=%d page(%d DW)\n",
			 txd_1->txd_len == 0 ? 1 : 2, (txd_1->txd_len + 1) * 16));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tHdrPad=%d(Padding Mode: %s, padding bytes: %d)\n",
			 txd_1->hdr_pad, ((txd_1->hdr_pad & (TMI_HDR_PAD_MODE_TAIL << 1)) ? "tail" : "head"),
			 ((txd_1->hdr_pad & 0x1) ? 2 : 0)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tUNxV=%d\n", txd_1->UNxV));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tamsdu=%d\n", txd_1->amsdu));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tTID=%d\n", txd_1->tid));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tpkt_ft=%d(%s)\n",
			 txd_1->pkt_ft, pkt_ft_str[txd_1->pkt_ft]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\town_mac=%d\n", txd_1->OwnMacAddr));

	if (txd_s->TxD1.ft == TMI_FT_LONG) {
		TMAC_TXD_L *txd_l = (TMAC_TXD_L *)tmac_info;
		TMAC_TXD_2 *txd_2 = &txd_l->TxD2;
		TMAC_TXD_3 *txd_3 = &txd_l->TxD3;
		TMAC_TXD_4 *txd_4 = &txd_l->TxD4;
		TMAC_TXD_5 *txd_5 = &txd_l->TxD5;
		TMAC_TXD_6 *txd_6 = &txd_l->TxD6;

		txd_7 = &txd_l->TxD7;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tTMAC_TXD_2:\n"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tsub_type=%d\n", txd_2->sub_type));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tfrm_type=%d\n", txd_2->frm_type));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tNDP=%d\n", txd_2->ndp));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tNDPA=%d\n", txd_2->ndpa));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tSounding=%d\n", txd_2->sounding));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tRTS=%d\n", txd_2->rts));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tbc_mc_pkt=%d\n", txd_2->bc_mc_pkt));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tBIP=%d\n", txd_2->bip));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tDuration=%d\n", txd_2->duration));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tHE(HTC Exist)=%d\n", txd_2->htc_vld));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tFRAG=%d\n", txd_2->frag));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tReamingLife/MaxTx time=%d\n", txd_2->max_tx_time));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tpwr_offset=%d\n", txd_2->pwr_offset));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tba_disable=%d\n", txd_2->ba_disable));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\ttiming_measure=%d\n", txd_2->timing_measure));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tfix_rate=%d\n", txd_2->fix_rate));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tTMAC_TXD_3:\n"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tNoAck=%d\n", txd_3->no_ack));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tPF=%d\n", txd_3->protect_frm));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\ttx_cnt=%d\n", txd_3->tx_cnt));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tremain_tx_cnt=%d\n", txd_3->remain_tx_cnt));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tsn=%d\n", txd_3->sn));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tpn_vld=%d\n", txd_3->pn_vld));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tsn_vld=%d\n", txd_3->sn_vld));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tTMAC_TXD_4:\n"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tpn_low=0x%x\n", txd_4->pn_low));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tTMAC_TXD_5:\n"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\ttx_status_2_host=%d\n", txd_5->tx_status_2_host));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\ttx_status_2_mcu=%d\n", txd_5->tx_status_2_mcu));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\ttx_status_fmt=%d\n", txd_5->tx_status_fmt));

		if (txd_5->tx_status_2_host || txd_5->tx_status_2_mcu)
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tpid=%d\n", txd_5->pid));

		if (txd_2->fix_rate)
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tda_select=%d\n", txd_5->da_select));

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tpwr_mgmt=0x%x\n", txd_5->pwr_mgmt));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tpn_high=0x%x\n", txd_5->pn_high));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tTMAC_TXD_6:\n"));

		if (txd_2->fix_rate) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tfix_rate_mode=%d\n", txd_6->fix_rate_mode));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tGI=%d(%s)\n", txd_6->gi, txd_6->gi == 0 ? "LONG" : "SHORT"));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tldpc=%d(%s)\n", txd_6->ldpc, txd_6->ldpc == 0 ? "BCC" : "LDPC"));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tTxBF=%d\n", txd_6->TxBF));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\ttx_rate=0x%x\n", txd_6->tx_rate));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tant_id=%d\n", txd_6->ant_id));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tdyn_bw=%d\n", txd_6->dyn_bw));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tbw=%d\n", txd_6->bw));
		}
	}

	/*  TODO DW7 */

	if (!IS_ASIC_CAP(pAd, fASIC_CAP_MCU_OFFLOAD)) {
		UINT32 i;
		MAC_TX_PKT_T *txp = (MAC_TX_PKT_T *)(tmac_info + cap->tx_hw_hdr_len);

		for (i = 0; i < NUM_OF_MSDU_ID_IN_TXD; i++) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\t\tID:%u, token id: %u\n", i, txp->au2MsduId[i]));

		}

		for (i = 0; i < (TXD_MAX_BUF_NUM / 2); i++) {
			TXD_PTR_LEN_T *txd_ptr_len = &txp->arPtrLen[i];

			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\t\tID:%u, u4Ptr0 = 0x%x, u2Len0 = %u, u4Ptr1 = 0x%x,\
				u2Len1 = %u\n", i, txd_ptr_len->u4Ptr0, txd_ptr_len->u2Len0, txd_ptr_len->u4Ptr1, txd_ptr_len->u2Len1));
		}
	}
}


static char *rmac_info_type_str[] = {
	"TXS",
	"RXV",
	"RxNormal",
	"DupRFB",
	"TMR",
	"Unknown",
};

static inline char *rxd_pkt_type_str(INT pkt_type)
{
	if (pkt_type <= 0x04)
		return rmac_info_type_str[pkt_type];
	else
		return rmac_info_type_str[5];
}


VOID dump_rmac_info_normal(RTMP_ADAPTER *pAd, UCHAR *rmac_info)
{
	RXD_BASE_STRUCT *rxd_base = (RXD_BASE_STRUCT *)rmac_info;

	hex_dump("RMAC_Info Raw Data: ", rmac_info, sizeof(RXD_BASE_STRUCT));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tRxData_BASE:\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tPktType=%d(%s)\n",
			 rxd_base->RxD0.PktType,
			 rxd_pkt_type_str(rxd_base->RxD0.PktType)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tGroupValid=%x\n", rxd_base->RxD0.RfbGroupVld));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tRxByteCnt=%d\n", rxd_base->RxD0.RxByteCnt));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tIP/UT=%d/%d\n", rxd_base->RxD0.IpChkSumOffload, rxd_base->RxD0.UdpTcpChkSumOffload));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tEtherTypeOffset=%d(WORD)\n", rxd_base->RxD0.EthTypeOffset));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tHTC/UC2ME/MC/BC=%d/%d/%d/%d\n",
			 rxd_base->RxD1.HTC,
			 (rxd_base->RxD1.a1_type == 0x1 ? 1 : 0),
			 (rxd_base->RxD1.a1_type == 0x2 ? 1 : 0),
			 rxd_base->RxD1.a1_type == 0x3 ? 1 : 0));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tBeacon with BMCast/Ucast=%d/%d\n",
			 rxd_base->RxD1.BcnWithBMcst, rxd_base->RxD1.BcnWithUCast));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tKeyID=%d\n", rxd_base->RxD1.KeyId));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tChFrequency=%x\n", rxd_base->RxD1.ChFreq));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tHdearLength(MAC)=%d\n", rxd_base->RxD1.MacHdrLen));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tHeaerOffset(HO)=%d\n", rxd_base->RxD1.HdrOffset));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tHeaerTrans(H)=%d\n", rxd_base->RxD1.HdrTranslation));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tPayloadFormat(PF)=%d\n", rxd_base->RxD1.PayloadFmt));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tBSSID=%d\n", rxd_base->RxD1.RxDBssidIdx));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tWlanIndex=%d\n", rxd_base->RxD2.RxDWlanIdx));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tTID=%d\n", rxd_base->RxD2.RxDTid));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tSEC Mode=%d\n", rxd_base->RxD2.SecMode));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tSW BIT=%d\n", rxd_base->RxD2.SwBit));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tFCE Error(FC)=%d\n", rxd_base->RxD2.FcsErr));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tCipher Mismatch(CM)=%d\n", rxd_base->RxD2.CipherMis));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tCipher Length Mismatch(CLM)=%d\n", rxd_base->RxD2.CipherLenMis));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tICV Err(I)=%d\n", rxd_base->RxD2.IcvErr));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tTKIP MIC Err(T)=%d\n", rxd_base->RxD2.TkipMicErr));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tLength Mismatch(LM)=%d\n", rxd_base->RxD2.LenMis));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tDeAMSDU Fail(DAF)=%d\n", rxd_base->RxD2.DeAmsduFail));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tExceedMax Rx Length(EL)=%d\n", rxd_base->RxD2.ExMaxRxLen));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tHdrTransFail(HTF)=%d\n", rxd_base->RxD2.HdrTransFail));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tInterested Frame(INTF)=%d\n", rxd_base->RxD2.InterestedFrm));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tFragment Frame(FRAG)=%d\n", rxd_base->RxD2.FragFrm));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tNull Frame(NULL)=%d\n", rxd_base->RxD2.NullFrm));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tNon Data Frame(NDATA)=%d\n", rxd_base->RxD2.NonDataFrm));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tNon-AMPDU Subframe(NASF)=%d\n", rxd_base->RxD2.NonAmpduSfrm));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tNon AMPDU(NAMP)=%d\n", rxd_base->RxD2.NonAmpduFrm));
}

VOID dump_rmac_info_for_ICVERR(RTMP_ADAPTER *pAd, UCHAR *rmac_info)
{
	RXD_BASE_STRUCT *rxd_base = (RXD_BASE_STRUCT *)rmac_info;
	union _RMAC_RXD_0_UNION *rxd_0;
	UINT32 pkt_type;
	int LogDbgLvl = DBG_LVL_WARN;

	if (!IS_HIF_TYPE(pAd, HIF_MT))
		return;

	rxd_0 = (union _RMAC_RXD_0_UNION *)rmac_info;
	pkt_type = RMAC_RX_PKT_TYPE(rxd_0->word);

	if (pkt_type != RMAC_RX_PKT_TYPE_RX_NORMAL)
		return;


	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, LogDbgLvl, ("\tHTC/UC2ME/MC/BC=%d/%d/%d/%d",
				rxd_base->RxD1.HTC,
				(rxd_base->RxD1.a1_type == 0x1 ? 1 : 0),
				(rxd_base->RxD1.a1_type == 0x2 ? 1 : 0),
				rxd_base->RxD1.a1_type == 0x3 ? 1 : 0));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, LogDbgLvl, (", WlanIndex=%d", rxd_base->RxD2.RxDWlanIdx));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, LogDbgLvl, (", SEC Mode=%d\n", rxd_base->RxD2.SecMode));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, LogDbgLvl, ("\tFCE Error(FC)=%d", rxd_base->RxD2.FcsErr));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, LogDbgLvl, (", CM=%d", rxd_base->RxD2.CipherMis));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, LogDbgLvl, (", CLM=%d", rxd_base->RxD2.CipherLenMis));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, LogDbgLvl, (", I=%d", rxd_base->RxD2.IcvErr));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, LogDbgLvl, (", T=%d", rxd_base->RxD2.TkipMicErr));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, LogDbgLvl, (", LM=%d\n", rxd_base->RxD2.LenMis));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, LogDbgLvl, ("\tFragment Frame(FRAG)=%d\n", rxd_base->RxD2.FragFrm));

}


VOID dump_rmac_info_txs(RTMP_ADAPTER *pAd, UCHAR *rmac_info)
{
	/* TXS_FRM_STRUC *txs_frm = (TXS_FRM_STRUC *)rmac_info; */
}


VOID dump_rmac_info_rxv(RTMP_ADAPTER *pAd, UCHAR *Data)
{
	RXV_DWORD0 *DW0 = NULL;
	RXV_DWORD1 *DW1 = NULL;

	RX_VECTOR1_1ST_CYCLE *RXV1_1ST_CYCLE = NULL;
	RX_VECTOR1_2ND_CYCLE *RXV1_2ND_CYCLE = NULL;
	RX_VECTOR1_3TH_CYCLE *RXV1_3TH_CYCLE = NULL;
	RX_VECTOR1_4TH_CYCLE *RXV1_4TH_CYCLE = NULL;
	RX_VECTOR1_5TH_CYCLE *RXV1_5TH_CYCLE = NULL;
	RX_VECTOR1_6TH_CYCLE *RXV1_6TH_CYCLE = NULL;

	RX_VECTOR2_1ST_CYCLE *RXV2_1ST_CYCLE = NULL;
	RX_VECTOR2_2ND_CYCLE *RXV2_2ND_CYCLE = NULL;
	RX_VECTOR2_3TH_CYCLE *RXV2_3TH_CYCLE = NULL;

	DW0 = (RXV_DWORD0 *)Data;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", TA_0_31=%d\n", DW0->TA_0_31));

	DW1 = (RXV_DWORD1 *)(Data + 4);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", TA_32_47=%d", DW1->TA_32_47));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", RxvSn=%d", DW1->RxvSn));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", TR=%d\n", DW1->TR));


	RXV1_1ST_CYCLE = (RX_VECTOR1_1ST_CYCLE *)(Data + 8);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", TxRate=%d", RXV1_1ST_CYCLE->TxRate));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", HtStbc=%d", RXV1_1ST_CYCLE->HtStbc));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", HtAdCode=%d", RXV1_1ST_CYCLE->HtAdCode));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", HtExtltf=%d", RXV1_1ST_CYCLE->HtExtltf));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", TxMode=%d", RXV1_1ST_CYCLE->TxMode));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", FrMode=%d\n", RXV1_1ST_CYCLE->FrMode));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", VHTA1_B22=%d", RXV1_1ST_CYCLE->VHTA1_B22));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", HtAggregation=%d", RXV1_1ST_CYCLE->HtAggregation));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", HtShortGi=%d", RXV1_1ST_CYCLE->HtShortGi));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", HtSmooth=%d", RXV1_1ST_CYCLE->HtSmooth));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", HtNoSound=%d", RXV1_1ST_CYCLE->HtNoSound));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", NumRx=%d\n", RXV1_1ST_CYCLE->NumRx));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", VHTA2_B8_B3=%d", RXV1_1ST_CYCLE->VHTA2_B8_B3));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", ACID_DET_LOWER=%d", RXV1_1ST_CYCLE->ACID_DET_LOWER));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", ACID_DET_UPPER=%d\n", RXV1_1ST_CYCLE->ACID_DET_UPPER));

	RXV1_2ND_CYCLE = (RX_VECTOR1_2ND_CYCLE *)(Data + 12);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", Length=%d", RXV1_2ND_CYCLE->Length));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", GroupId=%d", RXV1_2ND_CYCLE->GroupId));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", NstsField=%d", RXV1_2ND_CYCLE->NstsField));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", RxValidIndicator=%d", RXV1_2ND_CYCLE->RxValidIndicator));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", SelAnt=%d\n", RXV1_2ND_CYCLE->SelAnt));

	RXV1_3TH_CYCLE = (RX_VECTOR1_3TH_CYCLE *)(Data + 16);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", VHTA1_B21_B10=%d", RXV1_3TH_CYCLE->VHTA1_B21_B10));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", BFAgcApply=%d", RXV1_3TH_CYCLE->BFAgcApply));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", POPEverTrig=%d", RXV1_3TH_CYCLE->POPEverTrig));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", FgacCalLnaRx=%d", RXV1_3TH_CYCLE->FgacCalLnaRx));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", IBRssiRx=%d", RXV1_3TH_CYCLE->IBRssiRx));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", WBRssiRx=%d\n", RXV1_3TH_CYCLE->WBRssiRx));

	RXV1_4TH_CYCLE = (RX_VECTOR1_4TH_CYCLE *)(Data + 20);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", RCPI0=%d", RXV1_4TH_CYCLE->RCPI0));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", RCPI1=%d", RXV1_4TH_CYCLE->RCPI1));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", RCPI2=%d", RXV1_4TH_CYCLE->RCPI2));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", RCPI3=%d\n", RXV1_4TH_CYCLE->RCPI3));

	RXV1_5TH_CYCLE = (RX_VECTOR1_5TH_CYCLE *)(Data + 24);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", FagcLnaGainx=%d", RXV1_5TH_CYCLE->FagcLnaGainx));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", FagcLpfGainx=%d", RXV1_5TH_CYCLE->FagcLpfGainx));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", NoiseBalanceEnable=%d", RXV1_5TH_CYCLE->NoiseBalanceEnable));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", MISC1=%d\n", RXV1_5TH_CYCLE->MISC1));

	RXV1_6TH_CYCLE = (RX_VECTOR1_6TH_CYCLE *)(Data + 28);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", Nf0=%d", RXV1_6TH_CYCLE->Nf0));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", Nf1=%d", RXV1_6TH_CYCLE->Nf1));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", Nf2=%d", RXV1_6TH_CYCLE->Nf2));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", Nf3=%d\n", RXV1_6TH_CYCLE->Nf3));

	RXV2_1ST_CYCLE = (RX_VECTOR2_1ST_CYCLE *)(Data + 32);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", PrimItfrEnv=%d", RXV2_1ST_CYCLE->PrimItfrEnv));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", SecItfrEnv=%d", RXV2_1ST_CYCLE->SecItfrEnv));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", Sec40ItfrEnv=%d", RXV2_1ST_CYCLE->Sec40ItfrEnv));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", Sec80ItfrEnv=%d", RXV2_1ST_CYCLE->Sec80ItfrEnv));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", RxLQ=%d", RXV2_1ST_CYCLE->RxLQ));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", BtEnv=%d", RXV2_1ST_CYCLE->BtEnv));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", ScrambleSeed=%d\n", RXV2_1ST_CYCLE->ScrambleSeed));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", VHT_A2=%d", RXV2_1ST_CYCLE->VHT_A2));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", RxCERmsd=%d", RXV2_1ST_CYCLE->RxCERmsd));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", FCSErr=%d\n", RXV2_1ST_CYCLE->FCSErr));

	RXV2_2ND_CYCLE = (RX_VECTOR2_2ND_CYCLE *)(Data + 36);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", PostTMD=%d", RXV2_2ND_CYCLE->PostTMD));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", RxBW=%d", RXV2_2ND_CYCLE->RxBW));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", PostBWDSecCh=%d", RXV2_2ND_CYCLE->PostBWDSecCh));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", PostDewSecCh=%d", RXV2_2ND_CYCLE->PostDewSecCh));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", HtSTFDet=%d", RXV2_2ND_CYCLE->HtSTFDet));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", CagcSTFDet=%d", RXV2_2ND_CYCLE->CagcSTFDet));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", IBRssi0=%d", RXV2_2ND_CYCLE->IBRssi0));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", WBRssi0=%d", RXV2_2ND_CYCLE->WBRssi0));

	RXV2_3TH_CYCLE = (RX_VECTOR2_3TH_CYCLE *)(Data + 40);
}


VOID dump_rmac_info_rfb(RTMP_ADAPTER *pAd, UCHAR *rmac_info)
{
	/* RXD_BASE_STRUCT *rfb_frm = (RXD_BASE_STRUCT *)rmac_info; */
}


VOID dump_rmac_info_tmr(RTMP_ADAPTER *pAd, UCHAR *rmac_info)
{
	/* TMR_FRM_STRUC *rxd_base = (TMR_FRM_STRUC *)rmac_info; */
}


VOID dump_rmac_info(RTMP_ADAPTER *pAd, UCHAR *rmac_info)
{
	union _RMAC_RXD_0_UNION *rxd_0;
	UINT32 pkt_type;

	rxd_0 = (union _RMAC_RXD_0_UNION *)rmac_info;
	pkt_type = RMAC_RX_PKT_TYPE(rxd_0->word);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("RMAC_RXD Header Format :%s\n",
			 rxd_pkt_type_str(pkt_type)));

	switch (pkt_type) {
	case RMAC_RX_PKT_TYPE_RX_TXS:
		dump_rmac_info_txs(pAd, rmac_info);
		break;

	case RMAC_RX_PKT_TYPE_RX_TXRXV:
		dump_rmac_info_rxv(pAd, rmac_info);
		break;

	case RMAC_RX_PKT_TYPE_RX_NORMAL:
		dump_rmac_info_normal(pAd, rmac_info);
		break;

	case RMAC_RX_PKT_TYPE_RX_DUP_RFB:
		dump_rmac_info_rfb(pAd, rmac_info);
		break;

	case RMAC_RX_PKT_TYPE_RX_TMR:
		dump_rmac_info_tmr(pAd, rmac_info);
		break;

	default:
		break;
	}
}


VOID DumpTxSFormat(RTMP_ADAPTER *pAd, UINT8 Format, CHAR *Data)
{
	TXS_STRUC *txs_entry = (TXS_STRUC *)Data;
	TXS_D_0 *TxSD0 = &txs_entry->TxSD0;
	TXS_D_1 *TxSD1 = &txs_entry->TxSD1;
	TXS_D_2 *TxSD2 = &txs_entry->TxSD2;
	TXS_D_3 *TxSD3 = &txs_entry->TxSD3;
	/* TXS_D_4 *TxSD4 = &txs_entry->TxSD4; */
	/* TXS_D_5 *TxSD5 = &txs_entry->TxSD5; */
	/* TXS_D_6 *TxSD6 = &txs_entry->TxSD6; */

	if (Format == TXS_FORMAT0) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("\tType=TimeStamp/FrontTime Mode\n"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("\t\tTXSFM=%d, TXS2M/H=%d/%d, FixRate=%d, TxRate/BW=0x%x/%d\n",
				  TxSD0->TxSFmt, TxSD0->TxS2M, TxSD0->TxS2H,
				  TxSD0->TxS_FR, TxSD0->TxRate, TxSD1->TxS_TxBW));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("\t\tME/RE/LE/BE/TxOPLimitErr/BA-Fail=%d/%d/%d/%d/%d/%d, PS=%d, Pid=%d\n",
				  TxSD0->ME, TxSD0->RE, TxSD0->LE, TxSD0->BE, TxSD0->TxOp,
				  TxSD0->BAFail, TxSD0->PSBit, TxSD0->TxS_PId));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("\t\tTid=%d, AntId=%d, ETxBF/ITxBf=%d/%d\n",
				  TxSD1->TxS_Tid, TxSD1->TxS_AntId,
				  TxSD1->TxS_ETxBF, TxSD1->TxS_ITxBF));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("\t\tTxPwrdBm=0x%x, FinalMPDU=0x%x, AMPDU=0x%x\n",
				  TxSD1->TxPwrdBm, TxSD1->TxS_FianlMPDU, TxSD1->TxS_AMPDU));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("\t\tTxDelay=0x%x, RxVSeqNum=0x%x, Wlan Idx=0x%x\n",
				  TxSD2->TxS_TxDelay, TxSD2->TxS_RxVSN, TxSD2->TxS_WlanIdx));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("\t\tSN=0x%x, MPDU TxCnt=%d, LastTxRateIdx=%d\n",
				  TxSD3->type_0.TxS_SN, TxSD3->type_0.TxS_MpduTxCnt,
				  TxSD3->type_0.TxS_LastTxRateIdx));
	} else if (Format == TXS_FORMAT1) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("\tType=Noisy/RCPI Mode\n"));
	} else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("%s: Unknown TxSFormat(%d)\n", __func__, Format));
	}
}

INT dump_dmac_amsdu_info(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
#define MSDU_CNT_CR_NUMBER 8
	UINT32 msdu_cnt[MSDU_CNT_CR_NUMBER] = {0};
	UINT idx = 0;
	PMAC_TABLE_ENTRY pEntry = NULL;
	STA_TR_ENTRY *tr_entry = NULL;

	RTMP_IO_READ32(pAd, AMSDU_PACK_1_MSDU_CNT, &msdu_cnt[0]);
	RTMP_IO_READ32(pAd, AMSDU_PACK_2_MSDU_CNT, &msdu_cnt[1]);
	RTMP_IO_READ32(pAd, AMSDU_PACK_3_MSDU_CNT, &msdu_cnt[2]);
	RTMP_IO_READ32(pAd, AMSDU_PACK_4_MSDU_CNT, &msdu_cnt[3]);
	RTMP_IO_READ32(pAd, AMSDU_PACK_5_MSDU_CNT, &msdu_cnt[4]);
	RTMP_IO_READ32(pAd, AMSDU_PACK_6_MSDU_CNT, &msdu_cnt[5]);
	RTMP_IO_READ32(pAd, AMSDU_PACK_7_MSDU_CNT, &msdu_cnt[6]);
	RTMP_IO_READ32(pAd, AMSDU_PACK_8_MSDU_CNT, &msdu_cnt[7]);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("=== HW_AMSDU INFO.===\n"));
	for (idx = 0; idx < MSDU_CNT_CR_NUMBER; idx++) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("PACK_%d_MSDU_CNT=%d\n", (idx+1), msdu_cnt[idx]));
	}
	for (idx = 0; VALID_UCAST_ENTRY_WCID(pAd, idx); idx++) {
		pEntry = &pAd->MacTab.Content[idx];
		tr_entry = &pAd->MacTab.tr_entry[idx];

		if (IS_ENTRY_NONE(pEntry))
			continue;

		if ((pEntry->Sst == SST_ASSOC) &&
			(tr_entry->PortSecured == WPA_802_1X_PORT_SECURED) &&
			(pEntry->tx_amsdu_bitmap != 0)) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Wcid%03d: %02x", idx, pEntry->tx_amsdu_bitmap));
		}
	}
	return TRUE;
}

#ifdef CUSTOMER_RSG_FEATURE
VOID Read_Mib_TxRx_Counters(RTMP_ADAPTER *pAd)
{
	UINT32 CrValue;
	UINT32 BeaconCount, CtrlPktCount, MgmtRetryCnt, DataFrameRetryCount, TotalRxCount;

	RTMP_IO_READ32(pAd, MIB_M0SDR0, &CrValue);
	BeaconCount = (CrValue & 0xffff);

	RTMP_IO_READ32(pAd, MIB_M0SDR5, &CrValue);
	TotalRxCount = CrValue;

	RTMP_IO_READ32(pAd, MIB_M0SDR38, &CrValue);
	CtrlPktCount = (CrValue & 0xffffff);

	RTMP_IO_READ32(pAd, MIB_M0SDR39, &CrValue);
	MgmtRetryCnt = (CrValue & 0xffffff);

	RTMP_IO_READ32(pAd, MIB_M0SDR40, &CrValue);
	DataFrameRetryCount = (CrValue & 0xffffff);

	pAd->RadioStatsCounter.TotalBeaconSentCount += BeaconCount;
	pAd->RadioStatsCounter.TotalTxCount += BeaconCount + MgmtRetryCnt + CtrlPktCount;
	pAd->RadioStatsCounter.TxDataCount -= DataFrameRetryCount;
	pAd->RadioStatsCounter.TotalRxCount += TotalRxCount;
	pAd->RadioStatsCounter.TxRetryCount += DataFrameRetryCount + MgmtRetryCnt;
	pAd->beacon_cnt += BeaconCount;
}

#endif


VOID Update_Mib_Bucket_One_Sec(RTMP_ADAPTER *pAd)
{
	UCHAR   i = 0, j = 0;
	UCHAR concurrent_bands = HcGetAmountOfBand(pAd);

	for (i = 0 ; i < concurrent_bands ; i++) {
		if (pAd->OneSecMibBucket.Enabled[i] == TRUE) {
			pAd->OneSecMibBucket.ChannelBusyTime[i] = 0;
			pAd->OneSecMibBucket.OBSSAirtime[i] = 0;
			pAd->OneSecMibBucket.MyTxAirtime[i] = 0;
			pAd->OneSecMibBucket.MyRxAirtime[i] = 0;
			pAd->OneSecMibBucket.EDCCAtime[i] =  0;
			pAd->OneSecMibBucket.MdrdyCount[i] = 0;
			pAd->OneSecMibBucket.PdCount[i] = 0;
			for (j = 0 ; j < 2 ; j++) {
				pAd->OneSecMibBucket.ChannelBusyTime[i] += pAd->MsMibBucket.ChannelBusyTime[i][j];
				pAd->OneSecMibBucket.OBSSAirtime[i] += pAd->MsMibBucket.OBSSAirtime[i][j];
				pAd->OneSecMibBucket.MyTxAirtime[i] += pAd->MsMibBucket.MyTxAirtime[i][j];
				pAd->OneSecMibBucket.MyRxAirtime[i] += pAd->MsMibBucket.MyRxAirtime[i][j];
				pAd->OneSecMibBucket.EDCCAtime[i] += pAd->MsMibBucket.EDCCAtime[i][j];
				pAd->OneSecMibBucket.MdrdyCount[i] += pAd->MsMibBucket.MdrdyCount[i][j];
				pAd->OneSecMibBucket.PdCount[i] += pAd->MsMibBucket.PdCount[i][j];
			}
		}
	}
}
#if defined(OFFCHANNEL_SCAN_FEATURE) || defined(TR181_SUPPORT) || defined(TXRX_STAT_SUPPORT)
VOID Reset_MIB_Update_Counters(RTMP_ADAPTER *pAd, UCHAR Idx)
{

	pAd->ChannelStats.MibUpdateOBSSAirtime[Idx] = 0;
	pAd->ChannelStats.MibUpdateMyTxAirtime[Idx] = 0;
	pAd->ChannelStats.MibUpdateMyRxAirtime[Idx] = 0;
#ifdef CUSTOMER_RSG_FEATURE
	pAd->ChannelStats.MibUpdateEDCCAtime[Idx] = 0;
	pAd->ChannelStats.MibUpdatePdCount[Idx] = 0;
	pAd->ChannelStats.MibUpdateMdrdyCount[Idx] = 0;
#endif
}
#endif


VOID Update_Mib_Bucket_500Ms(RTMP_ADAPTER *pAd)
{
	UINT32  CrValue;
	UCHAR   i = 0;
	UCHAR   CurrIdx = 0;
	UCHAR concurrent_bands = HcGetAmountOfBand(pAd);

	pAd->MsMibBucket.CurIdx++;
	if (pAd->MsMibBucket.CurIdx >= 2)
		pAd->MsMibBucket.CurIdx = 0;
	CurrIdx = pAd->MsMibBucket.CurIdx;
	for (i = 0; i < concurrent_bands; i++) {
		if (pAd->MsMibBucket.Enabled == TRUE) {
			/* Channel Busy Time */
			HW_IO_READ32(pAd, MIB_M0SDR16 + (i * BandOffset), &CrValue);
			pAd->MsMibBucket.ChannelBusyTime[i][CurrIdx] = CrValue;

#if defined(OFFCHANNEL_SCAN_FEATURE) || defined(TR181_SUPPORT) || defined(TXRX_STAT_SUPPORT)
			if (!(ApScanRunning(pAd, NULL))) {
				pAd->MsMibBucket.OBSSAirtime[i][CurrIdx] = pAd->ChannelStats.MibUpdateOBSSAirtime[i];
				pAd->MsMibBucket.MyTxAirtime[i][CurrIdx] = pAd->ChannelStats.MibUpdateMyTxAirtime[i];
				pAd->MsMibBucket.MyRxAirtime[i][CurrIdx] = pAd->ChannelStats.MibUpdateMyRxAirtime[i];
#ifdef CUSTOMER_RSG_FEATURE
				pAd->MsMibBucket.EDCCAtime[i][CurrIdx] = pAd->ChannelStats.MibUpdateEDCCAtime[i];
				pAd->MsMibBucket.PdCount[i][CurrIdx] = pAd->ChannelStats.MibUpdatePdCount[i];
				pAd->MsMibBucket.MdrdyCount[i][CurrIdx] = pAd->ChannelStats.MibUpdateMdrdyCount[i];
#endif
				Reset_MIB_Update_Counters(pAd, i);
			} else
#endif
			{
				/* OBSS Air time */
				HW_IO_READ32(pAd, RMAC_MIBTIME5 + i * 4, &CrValue);
				pAd->MsMibBucket.OBSSAirtime[i][CurrIdx] = CrValue;
				/* My Tx Air time */
				HW_IO_READ32(pAd, MIB_M0SDR36 + (i * BandOffset), &CrValue);
				pAd->MsMibBucket.MyTxAirtime[i][CurrIdx] = CrValue;
				/* My Rx Air time */
				HW_IO_READ32(pAd, MIB_M0SDR37 + (i * BandOffset), &CrValue);
				pAd->MsMibBucket.MyRxAirtime[i][CurrIdx] = CrValue;
				/* EDCCA time */
				HW_IO_READ32(pAd, MIB_M0SDR18 + (i * BandOffset), &CrValue);
				pAd->MsMibBucket.EDCCAtime[i][CurrIdx] = CrValue;
				/* Reset OBSS Air time */
				HW_IO_READ32(pAd, RMAC_MIBTIME0, &CrValue);
				CrValue |= 1 << RX_MIBTIME_CLR_OFFSET;
				CrValue |= 1 << RX_MIBTIME_EN_OFFSET;
				HW_IO_WRITE32(pAd, RMAC_MIBTIME0, CrValue);

				HW_IO_READ32(pAd, RO_BAND0_PHYCTRL_STS0 + (i * BandOffset), &CrValue); /* PD count */
				pAd->MsMibBucket.PdCount[i][CurrIdx] = CrValue;
				HW_IO_READ32(pAd, RO_BAND0_PHYCTRL_STS5 + (i * BandOffset), &CrValue); /* MDRDY count */
				pAd->MsMibBucket.MdrdyCount[i][CurrIdx] = CrValue;
				HW_IO_READ32(pAd, PHY_BAND0_PHYMUX_5 + (i * BandOffset), &CrValue);
				CrValue &= 0xff8fffff;
				HW_IO_WRITE32(pAd, PHY_BAND0_PHYMUX_5 + (i * BandOffset), CrValue); /* Reset */
				CrValue |= 0x500000;
				HW_IO_WRITE32(pAd, PHY_BAND0_PHYMUX_5 + (i * BandOffset), CrValue); /* Enable */
			}
		}
	}
}

static RTMP_REG_PAIR mac_cr_seg[] = {
	{0x20000, 0x20010}, /* WF_CFG */
	{WF_TRB_BASE, 0x21040}, /* WF_CFG */
	{WF_AGG_BASE, 0x21240}, /* WF_CFG */
	{WF_ARB_BASE, 0x21440}, /* WF_CFG */
	{0, 0},
};


VOID dump_mt_mac_cr(RTMP_ADAPTER *pAd)
{
	INT index = 0;
	UINT32 mac_val, mac_addr, seg_s, seg_e;

	while (mac_cr_seg[index].Register != 0) {
		seg_s = mac_cr_seg[index].Register;
		seg_e = mac_cr_seg[index].Value;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Dump WF_CFG Segment(Start=0x%x, End=0x%x)\n",
				 seg_s, seg_e));

		for (mac_addr = seg_s; mac_addr < seg_e; mac_addr += 4) {
			MAC_IO_READ32(pAd, mac_addr, &mac_val);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("MAC[0x%x] = 0x%x\n", mac_addr, mac_val));
		}

		index++;
	};
}


INT mt_mac_fifo_stat_update(RTMP_ADAPTER *pAd)
{
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Not support for HIF_MT yet!\n",
			 __func__, __LINE__));
	return FALSE;
}


VOID NicGetTxRawCounters(
	IN RTMP_ADAPTER *pAd,
	IN TX_STA_CNT0_STRUC * pStaTxCnt0,
	IN TX_STA_CNT1_STRUC * pStaTxCnt1)
{
	/* TODO: shiang-7603 */
	return;
}


/*
	========================================================================

	Routine Description:
		Read statistical counters from hardware registers and record them
		in software variables for later on query

	Arguments:
		pAd					Pointer to our adapter

	Return Value:
		None

	IRQL = DISPATCH_LEVEL

	========================================================================
*/
static VOID NICUpdateAmpduRawCounters(RTMP_ADAPTER *pAd, UCHAR BandIdx)
{
	/* for PER debug */
	UINT32 AmpduTxCount = 0;
	UINT32 AmpduTxSuccessCount = 0;
	COUNTER_802_11 *wlanCounter;
	UINT32 mac_val, ampdu_range_cnt[4];
	UINT32 Offset = 0x200 * BandIdx;

	wlanCounter = &pAd->WlanCounters[BandIdx];
	MAC_IO_READ32(pAd, MIB_M0SDR14 + Offset, &AmpduTxCount);
	AmpduTxCount &= 0xFFFFFF;
	MAC_IO_READ32(pAd, MIB_M0SDR15 + Offset, &AmpduTxSuccessCount);
	AmpduTxSuccessCount &= 0xFFFFFF;
	MAC_IO_READ32(pAd, MIB_M0DR2 + Offset, &mac_val);
	ampdu_range_cnt[0] = mac_val & 0xffff;
	ampdu_range_cnt[1] =  (mac_val >> 16) & 0xffff;
	MAC_IO_READ32(pAd, MIB_M0DR3 + Offset, &mac_val);
	ampdu_range_cnt[2] = mac_val & 0xffff;
	ampdu_range_cnt[3] =  (mac_val >> 16) & 0xffff;
#ifdef STATS_COUNT_SUPPORT
	wlanCounter->AmpduSuccessCount.u.LowPart += AmpduTxSuccessCount;
	wlanCounter->AmpduFailCount.u.LowPart += (AmpduTxCount - AmpduTxSuccessCount);
#endif /* STATS_COUNT_SUPPORT */
	wlanCounter->TxAggRange1Count.u.LowPart += ampdu_range_cnt[0];
	wlanCounter->TxAggRange2Count.u.LowPart += ampdu_range_cnt[1];
	wlanCounter->TxAggRange3Count.u.LowPart += ampdu_range_cnt[2];
	wlanCounter->TxAggRange4Count.u.LowPart += ampdu_range_cnt[3];
}

VOID NICUpdateRawCounters(RTMP_ADAPTER *pAd)
{
	UINT32 OldValue, i;
	UINT32 rx_err_cnt, fcs_err_cnt, mdrdy_cnt = 0, fcs_err_cnt_band1 = 0, mdrdy_cnt_band1 = 0;
	/* UINT32 TxSuccessCount = 0, TxRetryCount = 0; */
#ifdef TXRX_STAT_SUPPORT
	struct hdev_ctrl *ctrl = (struct hdev_ctrl *)pAd->hdev_ctrl;
#endif
#ifdef COMPOS_WIN
	COUNTER_MTK *pPrivCounters;
#else
	COUNTER_RALINK *pPrivCounters;
#endif
	COUNTER_802_11 *wlanCounter;
	COUNTER_802_3 *dot3Counters;
#ifdef ERR_RECOVERY

	if (IsStopingPdma(&pAd->ErrRecoveryCtl))
		return;

#endif /* ERR_RECOVERY */
#ifdef COMPOS_WIN
	wlanCounter = &pAd->Counter.WlanCounters;
	pPrivCounters = &pAd->Counter.MTKCounters;
	dot3Counters = &pAd->Counter.Counters8023;
#else
	pPrivCounters = &pAd->RalinkCounters;
	wlanCounter = &pAd->WlanCounters[0];
	dot3Counters = &pAd->Counters8023;
#endif /* COMPOS_WIN */
	MAC_IO_READ32(pAd, MIB_M0SDR3, &rx_err_cnt);
	fcs_err_cnt = rx_err_cnt & 0xffff;
	MAC_IO_READ32(pAd, MIB_M0SDR4, &rx_err_cnt);

	MAC_IO_READ32(pAd, MIB_M1SDR3, &fcs_err_cnt_band1);
#ifndef TXRX_STAT_SUPPORT
	if (pAd->parse_rxv_stat_enable) {
#endif
#ifdef MT7615
		if (IS_MT7615(pAd))
			fcs_err_cnt_band1 = (fcs_err_cnt_band1 & 0xffff); /* [15:0] FCS ERR */
#endif
#ifndef TXRX_STAT_SUPPORT
	}
#endif

#ifdef TXRX_STAT_SUPPORT
	ctrl->rdev[DBDC_BAND0].pRadioCtrl->RxCRCErrorCount.QuadPart += fcs_err_cnt;
	if (pAd->CommonCfg.dbdc_mode)
		ctrl->rdev[DBDC_BAND1].pRadioCtrl->RxCRCErrorCount.QuadPart += fcs_err_cnt_band1;
#endif
	if (pAd->parse_rxv_stat_enable) {
		MAC_IO_READ32(pAd, MIB_M0SDR10, &mdrdy_cnt);
#if defined(MT7615)
		if (IS_MT7615(pAd))
			mdrdy_cnt = (mdrdy_cnt & 0x3FFFFFF); /* [25:0] Mac Mdrdy*/

#endif
		MAC_IO_READ32(pAd, MIB_M1SDR10, &mdrdy_cnt_band1);
#if defined(MT7615)
		if (IS_MT7615(pAd))
			mdrdy_cnt_band1 = (mdrdy_cnt_band1 & 0x3FFFFFF); /* [25:0] Mac Mdrdy*/

#endif
	}

	pPrivCounters->OneSecRxFcsErrCnt += fcs_err_cnt;

	if (pAd->parse_rxv_stat_enable) {
		pAd->AccuOneSecRxBand0FcsErrCnt += fcs_err_cnt; /*Used for rx_statistic*/
		pAd->AccuOneSecRxBand0MdrdyCnt += mdrdy_cnt; /*Used for rx_statistic*/
		pAd->AccuOneSecRxBand1FcsErrCnt += fcs_err_cnt_band1; /*Used for rx_statistic*/
		pAd->AccuOneSecRxBand1MdrdyCnt += mdrdy_cnt_band1; /*Used for rx_statistic*/
	}

#ifdef STATS_COUNT_SUPPORT
	/* Update FCS counters*/
	OldValue = pAd->WlanCounters[0].FCSErrorCount.u.LowPart;
	pAd->WlanCounters[0].FCSErrorCount.u.LowPart += fcs_err_cnt; /* >> 7);*/

	if (pAd->WlanCounters[0].FCSErrorCount.u.LowPart < OldValue)
		pAd->WlanCounters[0].FCSErrorCount.u.HighPart++;

#ifdef DBDC_MODE
	if (IS_ATE_DBDC(pAd)) {
		/* Update FCS counters of band1 */
		OldValue = pAd->WlanCounters[1].FCSErrorCount.u.LowPart;
		pAd->WlanCounters[1].FCSErrorCount.u.LowPart += fcs_err_cnt_band1;

		if (pAd->WlanCounters[1].FCSErrorCount.u.LowPart < OldValue)
			pAd->WlanCounters[1].FCSErrorCount.u.HighPart++;
	}
#endif /* DBDC_MODE */

#endif /* STATS_COUNT_SUPPORT */
	/* Add FCS error count to private counters*/
	pPrivCounters->OneSecRxFcsErrCnt += fcs_err_cnt;
	OldValue = pPrivCounters->RealFcsErrCount.u.LowPart;
	pPrivCounters->RealFcsErrCount.u.LowPart += fcs_err_cnt;

	if (pPrivCounters->RealFcsErrCount.u.LowPart < OldValue)
		pPrivCounters->RealFcsErrCount.u.HighPart++;

	dot3Counters->RxNoBuffer += (rx_err_cnt & 0xffff);

	for (i = 0; i < DBDC_BAND_NUM; i++)
		NICUpdateAmpduRawCounters(pAd, i);

#ifdef CONFIG_QA

	if (pAd->ATECtrl.bQAEnabled == TRUE) {
		/* Modify Rx stat structure */
		/* pAd->ATECtrl.rx_stat.RxMacFCSErrCount += fcs_err_cnt; */
		MT_ATEUpdateRxStatistic(pAd, 3, wlanCounter);
	}

#endif
	return;
}


/*
	========================================================================

	Routine Description:
		Clean all Tx/Rx statistic raw counters from hardware registers

	Arguments:
		pAd					Pointer to our adapter

	Return Value:
		None

	========================================================================
*/
VOID NicResetRawCounters(RTMP_ADAPTER *pAd)
{
	return;
}


UCHAR tmi_rate_map_cck_lp[] = {
	TMI_TX_RATE_CCK_1M_LP,
	TMI_TX_RATE_CCK_2M_LP,
	TMI_TX_RATE_CCK_5M_LP,
	TMI_TX_RATE_CCK_11M_LP,
};

UCHAR tmi_rate_map_cck_sp[] = {
	TMI_TX_RATE_CCK_2M_SP,
	TMI_TX_RATE_CCK_5M_SP,
	TMI_TX_RATE_CCK_11M_SP,
};

UCHAR tmi_rate_map_ofdm[] = {
	TMI_TX_RATE_OFDM_6M,
	TMI_TX_RATE_OFDM_9M,
	TMI_TX_RATE_OFDM_12M,
	TMI_TX_RATE_OFDM_18M,
	TMI_TX_RATE_OFDM_24M,
	TMI_TX_RATE_OFDM_36M,
	TMI_TX_RATE_OFDM_48M,
	TMI_TX_RATE_OFDM_54M,
};


#define TMI_TX_RATE_CCK_VAL(_mcs) \
	((TMI_TX_RATE_MODE_CCK << TMI_TX_RATE_BIT_MODE) | (_mcs))

#define TMI_TX_RATE_OFDM_VAL(_mcs) \
	((TMI_TX_RATE_MODE_OFDM << TMI_TX_RATE_BIT_MODE) | (_mcs))

#define TMI_TX_RATE_HT_VAL(_mode, _mcs, _stbc) \
	(((_stbc) << TMI_TX_RATE_BIT_STBC) |\
	 ((_mode) << TMI_TX_RATE_BIT_MODE) | \
	 (_mcs))

#define TMI_TX_RATE_VHT_VAL(_nss, _mcs, _stbc) \
	(((_stbc) << TMI_TX_RATE_BIT_STBC) |\
	 (((_nss - 1) & (TMI_TX_RATE_MASK_NSS)) << TMI_TX_RATE_BIT_NSS) | \
	 (TMI_TX_RATE_MODE_VHT << TMI_TX_RATE_BIT_MODE) | \
	 (_mcs))


UINT16 tx_rate_to_tmi_rate(UINT8 mode, UINT8 mcs, UINT8 nss, BOOLEAN stbc, UINT8 preamble)
{
	UINT16 tmi_rate = 0, mcs_id = 0;

	stbc = (stbc == TRUE) ? 1 : 0;

	switch (mode) {
	case MODE_CCK:
		if (preamble) {
			if (mcs < ARRAY_SIZE(tmi_rate_map_cck_lp))
				mcs_id = tmi_rate_map_cck_lp[mcs];
		} else {
			if (mcs < ARRAY_SIZE(tmi_rate_map_cck_sp))
				mcs_id = tmi_rate_map_cck_sp[mcs];
		}

		tmi_rate = (TMI_TX_RATE_MODE_CCK << TMI_TX_RATE_BIT_MODE) | (mcs_id);
		break;

	case MODE_OFDM:
		if (mcs < ARRAY_SIZE(tmi_rate_map_ofdm)) {
			mcs_id = tmi_rate_map_ofdm[mcs];
			tmi_rate = (TMI_TX_RATE_MODE_OFDM << TMI_TX_RATE_BIT_MODE) | (mcs_id);
		}

		break;

	case MODE_HTMIX:
	case MODE_HTGREENFIELD:
		tmi_rate = ((USHORT)(stbc << TMI_TX_RATE_BIT_STBC)) |
				   (((nss - 1) & TMI_TX_RATE_MASK_NSS) << TMI_TX_RATE_BIT_NSS) |
				   ((USHORT)(mode << TMI_TX_RATE_BIT_MODE)) |
				   ((USHORT)(mcs));
		/* MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): mode=%d, mcs=%d, stbc=%d converted tmi_rate=0x%x\n", */
		/* __FUNCTION__, mode, mcs, stbc, tmi_rate)); */
		break;

	case MODE_VHT:
		tmi_rate = TMI_TX_RATE_VHT_VAL(nss, mcs, stbc);
		break;

	default:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():Invalid mode(mode=%d)\n",
				 __func__, mode));
		break;
	}

	return tmi_rate;
}


UCHAR get_nsts_by_mcs(UCHAR phy_mode, UCHAR mcs, BOOLEAN stbc, UCHAR vht_nss)
{
	UINT8 nsts = 1;

	switch (phy_mode) {
	case MODE_VHT:
		if (stbc && (vht_nss == 1))
			nsts++;
		else
			nsts = vht_nss;

		break;

	case MODE_HTMIX:
	case MODE_HTGREENFIELD:
		if (mcs != 32) {
			nsts += (mcs >> 3);

			if (stbc && (nsts == 1))
				nsts++;
		}

		break;

	case MODE_CCK:
	case MODE_OFDM:
	default:
		break;
	}

	return nsts;
}

UCHAR dmac_wmm_swq_2_hw_ac_que[4][4] = {
	{
		TxQ_IDX_AC0, /* 0: QID_AC_BK */
		TxQ_IDX_AC1, /* 1: QID_AC_BE */
		TxQ_IDX_AC2, /* 2: QID_AC_VI */
		TxQ_IDX_AC3, /* 3: QID_AC_VO */
	},
	{
		TxQ_IDX_AC10,
		TxQ_IDX_AC11,
		TxQ_IDX_AC12,
		TxQ_IDX_AC13,
	},
	{
		TxQ_IDX_AC20,
		TxQ_IDX_AC21,
		TxQ_IDX_AC22,
		TxQ_IDX_AC23,
	},
	{
		TxQ_IDX_AC30,
		TxQ_IDX_AC31,
		TxQ_IDX_AC32,
		TxQ_IDX_AC33,
	}
};

UINT8  dmac_ac_queue_to_up[4] = {
    1 /* AC_BK */,   0 /* AC_BE */,   5 /* AC_VI */,   7 /* AC_VO */
};

VOID MtWriteTMacInfo(RTMP_ADAPTER *pAd, UCHAR *buf, TMAC_INFO *TxInfo)
{
	TMAC_TXD_S *txd_s = (TMAC_TXD_S *)buf;
	TMAC_TXD_L *txd_l = (TMAC_TXD_L *)buf;
	TMAC_TXD_0 *txd_0 = &txd_s->TxD0;
	TMAC_TXD_1 *txd_1 = &txd_s->TxD1;
	TMAC_TXD_2 *txd_2 = NULL;
	TMAC_TXD_3 *txd_3 = NULL;
	TMAC_TXD_5 *txd_5 = NULL;
	TMAC_TXD_6 *txd_6 = NULL;
	UINT32 *txd_7 = NULL;
	UCHAR txd_size;
	UCHAR stbc = 0, bw = BW_20, mcs = 0, nss = 1, sgi = 0, phy_mode = 0, preamble = 1, ldpc = 0, expBf = 0, impBf = 0, vht_nss = 1;
	TX_RADIO_SET_T *pTxRadioSet = NULL;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	/*DWORD 0*/
	txd_0->p_idx = TxInfo->PortIdx;
	txd_0->q_idx = TxInfo->QueIdx;

	if ((TxInfo->QueIdx < 4) && (TxInfo->WmmSet < 4))
		txd_0->q_idx = dmac_wmm_swq_2_hw_ac_que[TxInfo->WmmSet][TxInfo->QueIdx];
	else {
		/* TODO: shiang-usw, consider about MCC case! */
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			("%s: Non-WMM Q, TxInfo->WmmSet/QueIdx(%d/%d)!\n",
			__func__, TxInfo->WmmSet, TxInfo->QueIdx));
		txd_0->q_idx = TxInfo->QueIdx;
	}

#ifdef CONFIG_CSO_SUPPORT
	txd_0->UdpTcpChkSumOffload = 0;
	txd_0->IpChkSumOffload = 0;

	if (txd_0->IpChkSumOffload || txd_0->UdpTcpChkSumOffload)
		txd_0->EthTypeOffset = 8; /* 8 is (14+2)/2, it is  7615 setting just for 802.3 frame not for 802.11 */

#endif /* CONFIG_CSO_SUPPORT */
	/*DWORD 1*/
	txd_1->wlan_idx = TxInfo->Wcid;

	if (txd_1->wlan_idx >= cap->WtblHwNum) {
		pAd->wrong_wlan_idx_num++;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("wrong wlan index = %d\n", txd_1->wlan_idx));
		dump_stack();
	}

	if (!TxInfo->NeedTrans) {
		txd_1->hdr_format = TMI_HDR_FT_NOR_80211;
		TMI_HDR_INFO_VAL(txd_1->hdr_format, 0, 0, 0, 0, 0, TxInfo->WifiHdrLen, 0, txd_1->hdr_info);

		/* TODO: depends on QoS to decide if need to padding */
		if (TxInfo->HdrPad)
			txd_1->hdr_pad = (TMI_HDR_PAD_MODE_TAIL << TMI_HDR_PAD_BIT_MODE) | TxInfo->HdrPad;
	} else {
		txd_1->hdr_format = TMI_HDR_FT_NON_80211;
		TMI_HDR_INFO_VAL(txd_1->hdr_format, TxInfo->MoreData, TxInfo->Eosp, 1, TxInfo->VlanFrame, TxInfo->EtherFrame,
						 TxInfo->WifiHdrLen, 0, txd_1->hdr_info);

		if (TxInfo->HdrPad)
			txd_1->hdr_pad = (TMI_HDR_PAD_MODE_HEAD << TMI_HDR_PAD_BIT_MODE) | TxInfo->HdrPad;
	}

	txd_1->tid = TxInfo->UserPriority;
	txd_1->OwnMacAddr = TxInfo->OwnMacIdx;

	if (TxInfo->LongFmt == FALSE) {
		txd_1->ft = TMI_FT_SHORT;
		txd_size = sizeof(TMAC_TXD_S);
		txd_7 = &txd_s->TxD7;
	} else {
		txd_2 = &txd_l->TxD2;
		txd_3 = &txd_l->TxD3;
		txd_5 = &txd_l->TxD5;
		txd_6 = &txd_l->TxD6;
		txd_7 = &txd_l->TxD7;
		txd_1->ft = TMI_FT_LONG;
		txd_size = sizeof(TMAC_TXD_L);
		pTxRadioSet = &TxInfo->TxRadioSet;
		ldpc = pTxRadioSet->Ldpc;
		mcs = pTxRadioSet->RateCode;
		sgi = pTxRadioSet->ShortGI;
		stbc = pTxRadioSet->Stbc;
		phy_mode = pTxRadioSet->PhyMode;
		bw = pTxRadioSet->CurrentPerPktBW;
		expBf = pTxRadioSet->EtxBFEnable;
		impBf = pTxRadioSet->ItxBFEnable;
		vht_nss = TxInfo->VhtNss ? TxInfo->VhtNss : 1;
		nss = get_nsts_by_mcs(phy_mode, mcs, stbc, vht_nss);
		/*DW2*/
		txd_2->max_tx_time = TxInfo->MaxTxTime;
		txd_2->bc_mc_pkt = TxInfo->BmcPkt;
		txd_2->fix_rate = TxInfo->FixRate;
		txd_2->frm_type = TxInfo->FrmType;
		txd_2->sub_type = TxInfo->SubType;

		if (TxInfo->NeedTrans)
			txd_2->htc_vld = 0;

		txd_2->frag = TxInfo->FragIdx;
		txd_2->timing_measure = TxInfo->TimingMeasure;
		txd_2->ba_disable = TxInfo->BaDisable;
		txd_2->pwr_offset = TxInfo->PowerOffset;
		/*DW3*/
		txd_3->remain_tx_cnt = TxInfo->RemainTxCnt;
		txd_3->sn = TxInfo->Sn;
		txd_3->no_ack = (TxInfo->bAckRequired ? 0 : 1);
		txd_3->protect_frm = (TxInfo->CipherAlg != CIPHER_NONE) ? 1 : 0;
		/*DW5*/
		txd_5->pid = TxInfo->Pid;
		txd_5->tx_status_fmt = TxInfo->TxSFmt;
		txd_5->tx_status_2_host = TxInfo->TxS2Host;
		txd_5->tx_status_2_mcu = TxInfo->TxS2Mcu;

		if (TxInfo->NeedTrans)
			txd_5->da_select = TMI_DAS_FROM_MPDU;

		/* txd_5->BarSsnCtrl = TxInfo->BarSsnCtrl; */
		/* For  MT STA LP control, use H/W control mode for PM bit */
#if defined(CONFIG_STA_SUPPORT) && defined(CONFIG_PM_BIT_HW_MODE)
		txd_5->pwr_mgmt = TMI_PM_BIT_CFG_BY_HW;
#else
		txd_5->pwr_mgmt = TMI_PM_BIT_CFG_BY_SW;
#endif /* CONFIG_STA_SUPPORT && CONFIG_PM_BIT_HW_MODE */

		/* DW6 */
		if (txd_2->fix_rate == 1) {
			txd_6->fix_rate_mode = TMI_FIX_RATE_BY_TXD;
			/* TODO: MT7615 fix me! the AntPri is 8 bits, but currently hardware "ant_id" can support up to 12 bits!! */
			txd_6->ant_id = TxInfo->AntPri;
			/* TODO: MT7615, fix me! how to support SpeExtEnable in MT7615?? */
			/* txd_6->spe_en = TxInfo->SpeEn; */
			txd_6->bw = ((1 << 2) | bw);
			txd_6->dyn_bw = 0;
			txd_6->TxBF = ((impBf | expBf) ? 1 : 0);
			txd_6->ldpc = ldpc;
			txd_6->gi = sgi;

			if (txd_6->fix_rate_mode == TMI_FIX_RATE_BY_TXD) {
				preamble = TxInfo->TxRadioSet.Premable;
				txd_6->tx_rate = tx_rate_to_tmi_rate(phy_mode, mcs, nss, stbc, preamble);
			}
		}
	}

	/* DWORD 7 */
	if (cap->txd_type == TXD_V1) {
		*txd_7 &= ~SPE_IDX_MASK;
		*txd_7 |= SPE_IDX(TxInfo->AntPri);
	} else {
		*txd_7 &= ~CON_SPE_IDX_MASK;
		*txd_7 |= CON_SPE_IDX(TxInfo->AntPri);
	}
	*txd_7 |= PP_REF_SUBTYPE(TxInfo->SubType);
	*txd_7 |= PP_REF_TYPE(TxInfo->FrmType);

	txd_0->TxByteCount = txd_size + TxInfo->PktLen;
}


/*
	========================================================================

	Routine Description:
		Calculates the duration which is required to transmit out frames
	with given size and specified rate.

	Arguments:
		pTxWI		Pointer to head of each MPDU to HW.
		Ack		Setting for Ack requirement bit
		Fragment	Setting for Fragment bit
		RetryMode	Setting for retry mode
		Ifs		Setting for IFS gap
		Rate		Setting for transmit rate
		Service	Setting for service
		Length		Frame length
		TxPreamble	Short or Long preamble when using CCK rates
		QueIdx - 0-3, according to 802.11e/d4.4 June/2003

	Return Value:
		None

	See also : BASmartHardTransmit()    !!!

	========================================================================
*/

VOID mtd_write_tmac_info_fixed_rate(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR *tmac_info,
	IN MAC_TX_INFO * info,
	IN HTTRANSMIT_SETTING * pTransmit)
{
	MAC_TABLE_ENTRY *mac_entry = NULL;
	UCHAR stbc, bw, mcs, nss = 1, sgi, phy_mode, ldpc = 0, preamble = LONG_PREAMBLE;
	UCHAR q_idx = info->q_idx;
	TMAC_TXD_L txd;
	TMAC_TXD_0 *txd_0 = &txd.TxD0;
	TMAC_TXD_1 *txd_1 = &txd.TxD1;
	TMAC_TXD_2 *txd_2 = &txd.TxD2;
	TMAC_TXD_3 *txd_3 = &txd.TxD3;
	TMAC_TXD_5 *txd_5 = &txd.TxD5;
	TMAC_TXD_6 *txd_6 = &txd.TxD6;
	UINT32 *txd_7 = &txd.TxD7;
	INT txd_size = sizeof(TMAC_TXD_S);
	STA_TR_ENTRY *tr_entry = NULL;
#ifdef CONFIG_AP_SUPPORT
	struct wifi_dev *wdev = NULL;
#endif
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (VALID_UCAST_ENTRY_WCID(pAd, info->WCID))
		mac_entry = &pAd->MacTab.Content[info->WCID];

	NdisZeroMemory(&txd, sizeof(TMAC_TXD_L));
	ldpc = pTransmit->field.ldpc;
	mcs = pTransmit->field.MCS;
	sgi = pTransmit->field.ShortGI;
	stbc = pTransmit->field.STBC;
	phy_mode = pTransmit->field.MODE;
	bw = (phy_mode <= MODE_OFDM) ? (BW_20) : (pTransmit->field.BW);
#ifdef DOT11_N_SUPPORT
#ifdef CONFIG_ATE

	if (!ATE_ON(pAd))
#endif
	{
		if (mac_entry && !IS_ENTRY_NONE(mac_entry)) {
			UCHAR MaxMcs_1ss;
#ifdef DOT11_VHT_AC

			if (IS_VHT_STA(mac_entry))
				MaxMcs_1ss = 9;
			else
#endif /* DOT11_VHT_AC */
				MaxMcs_1ss = 7;

			if ((pAd->CommonCfg.bMIMOPSEnable) && (mac_entry->MmpsMode == MMPS_STATIC)
				&& (pTransmit->field.MODE >= MODE_HTMIX && pTransmit->field.MCS > MaxMcs_1ss))
				mcs = MaxMcs_1ss;
		}
	}

#endif /* DOT11_N_SUPPORT */
#ifdef DOT11K_RRM_SUPPORT

	if (pAd->CommonCfg.VoPwrConstraintTest == TRUE) {
		info->AMPDU = 0;
		mcs = 0;
		ldpc = 0;
		bw = 0;
		sgi = 0;
		stbc = 0;
		phy_mode = MODE_OFDM;
	}

#endif /* DOT11K_RRM_SUPPORT */
	/* DWORD 0 */
	txd_0->p_idx = P_IDX_LMAC;

	if (q_idx < WMM_QUE_NUM)
		txd_0->q_idx = dmac_wmm_swq_2_hw_ac_que[info->wmm_set][q_idx];
	else
		txd_0->q_idx = q_idx;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s(%d): TxBlk->wmm_set= %d, QID = %d\n",
			  __func__, __LINE__, info->wmm_set, txd_0->q_idx));
#ifdef CONFIG_CSO_SUPPORT
#if defined(MT7615) || defined(MT7622) || defined(P18) || defined(MT7663)

	if (IS_MT7615(pAd) || IS_MT7622(pAd) || IS_P18(pAd) || IS_MT7663(pAd)) {
		/* Leonardo: checksumm offload is not necessary for management frame */
		txd_0->UdpTcpChkSumOffload = 0;
		txd_0->IpChkSumOffload = 0;
	}

#endif /* defined(MT7615) || defined(MT7622) || defined(P18) || defined(MT7663) */
#endif /* CONFIG_CSO_SUPPORT */
	/* DWORD 1 */
	txd_1->wlan_idx = info->WCID;

	if (txd_1->wlan_idx >= cap->WtblHwNum) {
		pAd->wrong_wlan_idx_num++;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("wrong wlan index = %d\n", txd_1->wlan_idx));
		dump_stack();
	}

	txd_1->ft = TMI_FT_LONG;
	txd_1->txd_len = 0;
	txd_1->hdr_format = TMI_HDR_FT_NOR_80211;
	TMI_HDR_INFO_VAL(txd_1->hdr_format, 0, 0, 0, 0, 0, info->hdr_len, 0, txd_1->hdr_info);

	if (info->hdr_pad)  /* TODO: depends on QoS to decide if need to padding */
		txd_1->hdr_pad = (TMI_HDR_PAD_MODE_TAIL << TMI_HDR_PAD_BIT_MODE) | 0x1;

#if defined(MT7622) || defined(P18) || defined(MT7663)

	if (IS_MT7622(pAd) || IS_P18(pAd) || IS_MT7663(pAd))
		txd_1->UNxV = 0;	/* MT7622: Chagne UNxV to TXDEXTLEN */

#endif
	txd_1->tid = info->TID;

	if (info->IsOffloadPkt == TRUE)
		txd_1->pkt_ft = TMI_PKT_FT_MCU_FW;
	else
		txd_1->pkt_ft = TMI_PKT_FT_HIF_CT; /* cut-through */

	txd_1->OwnMacAddr = info->OmacIdx;

	/*
	  repeater entry doesn't have real wdev could bind.
	  so reference the OmacIdx which is stored in tr_entry.

	  other types entry could reference the OmacIdx of wdev which is connected to the pEntry.
	*/
	if (mac_entry && IS_ENTRY_REPEATER(mac_entry)) {
		tr_entry = &pAd->MacTab.tr_entry[mac_entry->wcid];
		txd_1->OwnMacAddr = tr_entry->OmacIdx;
	} else if (mac_entry && !IS_ENTRY_NONE(mac_entry) && !IS_ENTRY_MCAST(mac_entry))
		txd_1->OwnMacAddr = mac_entry->wdev->OmacIdx;

	if (txd_1->ft == TMI_FT_LONG) {
		txd_size = sizeof(TMAC_TXD_L);

		/* DWORD 2 */
		if (info->IsOffloadPkt == TRUE) {
			txd_2->sub_type = info->SubType;
			txd_2->frm_type = info->Type;
		}

		txd_2->ndp = 0;
		txd_2->ndpa = 0;
		txd_2->sounding = 0;
		txd_2->rts = 0;
		txd_2->bc_mc_pkt = info->BM;
		txd_2->bip = 0;
		txd_2->fix_rate = 1;
		txd_2->max_tx_time = 0;
		txd_2->duration = 0;
		txd_2->htc_vld = 0;
		txd_2->frag = info->FRAG; /* 0: no frag, 1: 1st frag, 2: mid frag, 3: last frag */
		txd_2->max_tx_time = 0;
		txd_2->pwr_offset = 0;
		txd_2->ba_disable = 1;
		txd_2->timing_measure = 0;

		if (info->IsAutoRate)
			txd_2->fix_rate = 0;
		else
			txd_2->fix_rate = 1;

		if ((pAd->pTmrCtrlStruct != NULL)
			&& (pAd->pTmrCtrlStruct->TmrEnable == TMR_INITIATOR)) {
			if ((info->Ack == 1) && (txd_2->bc_mc_pkt == 0))
				txd_2->timing_measure = 1;
		}

#ifdef WIFI_EAP_FEATURE
		if (pAd->CommonCfg.mgmt_txpwr_force_on == TRUE) {
			if (info->Type == FC_TYPE_MGMT)
				txd_2->pwr_offset = pAd->CommonCfg.txd_txpwr_offset;
		}
#endif

#ifndef FTM_SUPPORT

		if ((pAd->pTmrCtrlStruct != NULL)
			&& (pAd->pTmrCtrlStruct->TmrEnable == TMR_INITIATOR))
#endif /* FTM_SUPPORT */
		{
			if (info->IsTmr)
				txd_2->timing_measure = 1;
		}

		/* DWORD 3 */
		if (txd_0->q_idx == TxQ_IDX_BCN0 || txd_0->q_idx == TxQ_IDX_BCN1)
			txd_3->remain_tx_cnt = MT_TX_RETRY_UNLIMIT;
		else {
#ifdef FTM_SUPPORT

			if (info->IsTmr)
				txd_3->remain_tx_cnt = 1;
			else
#endif /* FTM_SUPPORT */
				txd_3->remain_tx_cnt = MT_TX_SHORT_RETRY;
		}

		txd_3->no_ack = (info->Ack ? 0 : 1);

		if (0 /* bar_sn_ctrl */)
			txd_3->sn_vld = 1;

		/* DWORD 4 */
		/* DWORD 5 */
#ifdef FTM_SUPPORT

		if (info->IsTmr) {
			txd_5->pid = info->PID;
			txd_5->tx_status_fmt = TXS_FORMAT0;
			txd_5->tx_status_2_mcu = 0;
			txd_5->tx_status_2_host = 1;
		}

#endif /* FTM_SUPPORT */


#ifdef HDR_TRANS_TX_SUPPORT
		/* txd_5->da_select = TMI_DAS_FROM_MPDU; */
#endif /* HDR_TRANS_TX_SUPPORT */
		/* TODO: shiang-MT7615, fix me! bar_sn_ctrl = Write SSN to SN field and set sn_vld bit to 1 */
		/* txd_5->bar_sn_ctrl = 1; */

		if (info->PsmBySw)
			txd_5->pwr_mgmt = TMI_PM_BIT_CFG_BY_SW;
		else
			txd_5->pwr_mgmt = TMI_PM_BIT_CFG_BY_HW;

		/* txd_5->pn_high = 0; */

		/* DWORD 6 */
		if (txd_2->fix_rate == 1) {
			txd_6->fix_rate_mode = TMI_FIX_RATE_BY_TXD;
			txd_6->bw = ((1 << 2) | bw);
			txd_6->dyn_bw = 0;
			txd_6->ant_id = 0; /* Smart Antenna indicator */
			txd_6->TxBF = 0;
			txd_6->ldpc = ldpc;
			txd_6->gi = sgi;

			if (txd_6->fix_rate_mode == TMI_FIX_RATE_BY_TXD) {
				if (phy_mode == MODE_CCK)
					preamble = info->Preamble;

				txd_6->tx_rate = tx_rate_to_tmi_rate(phy_mode, mcs, nss, stbc, preamble);
			}
		}

#ifdef FTM_SUPPORT

		if (txd_2->timing_measure) {
			txd_2->fix_rate = 1;
			txd_6->fix_rate_mode = TMI_FIX_RATE_BY_TXD;

			switch (info->ftm_bw) {
			case FTM_BW_VHT_BW80_80:
			case FTM_BW_VHT_BW80:
			case FTM_BW_VHT_BW40:
			case FTM_BW_VHT_BW20:
				txd_6->tx_rate = tx_rate_to_tmi_rate(MODE_VHT, 0, nss, stbc, preamble);
				break;

			case FTM_BW_HT_BW40:
			case FTM_BW_HT_BW20:
				txd_6->tx_rate = tx_rate_to_tmi_rate(MODE_HTMIX, 0, nss, stbc, preamble);
				break;

			case FTM_BW_NONHT_BW20:
			default:
				txd_6->tx_rate = tx_rate_to_tmi_rate(MODE_OFDM, 0, nss, stbc, preamble);
				break;
			}
		}

#endif /* FTM_SUPPORT */

		if (info->prot == 1)
			txd_3->protect_frm = 1;
		else if (info->prot == 2 || info->prot == 3) {
#ifdef CONFIG_AP_SUPPORT

			if (mac_entry) {
				wdev = mac_entry->wdev;
				GET_GroupKey_WCID(wdev, info->WCID);
			}

#ifdef DOT11W_PMF_SUPPORT
			if (info->prot == 2)
				txd_2->bip = 1;
#endif /* DOT11W_PMF_SUPPORT */
#else
			{
				MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[info->WCID];

				info->WCID = pEntry->wdev->bss_info_argument.ucBcMcWlanIdx;
			}
#endif
			txd_1->wlan_idx = info->WCID;

			if (txd_1->wlan_idx > 127) {
				pAd->wrong_wlan_idx_num++;
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("wrong wlan index = %d\n", txd_1->wlan_idx));
				dump_stack();
			}
		} else
			txd_3->protect_frm = 0;
	}

	/* DWORD 7 */
	if (cap->txd_type == TXD_V1) {
		*txd_7 &= ~SPE_IDX_MASK;
		*txd_7 |= SPE_IDX(info->AntPri);

		*txd_7 &= ~PP_REF_SUBTYPE_MASK;
		*txd_7 |= PP_REF_SUBTYPE(info->SubType);

		*txd_7 &= ~PP_REF_TYPE_MASK;
		*txd_7 |= PP_REF_TYPE(info->Type);
	} else {
		*txd_7 &= ~CON_SPE_IDX_MASK;
		*txd_7 |= CON_SPE_IDX(info->AntPri);
	}

	txd_0->TxByteCount = txd_size + info->Length;

	NdisMoveMemory(tmac_info, &txd, sizeof(TMAC_TXD_L));

	if (0 /*(DebugSubCategory[9] & CATTX_TMAC) == CATTX_TMAC*/)
		dump_tmac_info(pAd, tmac_info);
#ifdef RED_SUPPORT_BY_HOST
	if (!pAd->red_have_cr4)
		RedRecordForceRateFromDriver(pAd, info->WCID);
#endif
}

VOID mtd_write_tmac_info(RTMP_ADAPTER *pAd, UCHAR *buf, TX_BLK *pTxBlk)
{
	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_MCU_OFFLOAD))
		mtd_write_tmac_info_by_cr4(pAd, buf, pTxBlk);
	else
		mtd_write_tmac_info_by_host(pAd, buf, pTxBlk);
}

VOID mtd_write_tmac_info_by_cr4(RTMP_ADAPTER *pAd, UCHAR *buf, TX_BLK *pTxBlk)
{
	TMAC_TXD_L *txd_l = (TMAC_TXD_L *)buf;
	TMAC_TXD_0 *txd_0 = &txd_l->TxD0;
	TMAC_TXD_1 *txd_1 = &txd_l->TxD1;

	txd_0->p_idx = P_IDX_LMAC;
	txd_0->q_idx = 0;
	txd_0->TxByteCount = sizeof(TMAC_TXD_L) +
						 pTxBlk->MpduHeaderLen +
						 pTxBlk->HdrPadLen +
						 pTxBlk->SrcBufLen;
	txd_1->ft = TMI_FT_LONG;
	txd_1->txd_len = 0;
	txd_1->pkt_ft = TMI_PKT_FT_HIF_CT;
	txd_1->hdr_format = TMI_HDR_FT_NON_80211;
	TMI_HDR_INFO_VAL(TMI_HDR_FT_NON_80211, 0, 0, 0, 0, 0, 0, 0, txd_1->hdr_info);

	if (pTxBlk->HdrPadLen)
		txd_1->hdr_pad = (TMI_HDR_PAD_MODE_HEAD << TMI_HDR_PAD_BIT_MODE) | 0x1;

}


VOID mtd_write_tmac_info_by_host(RTMP_ADAPTER *pAd, UCHAR *buf, TX_BLK *pTxBlk)
{
	MAC_TABLE_ENTRY *pMacEntry = pTxBlk->pMacEntry;
	UCHAR stbc = 0, bw = BW_20, mcs = 0, nss = 1, sgi = 0, phy_mode = 0, preamble = 1, ldpc = 0;
	UCHAR wcid;
	TMAC_TXD_S *txd_s = (TMAC_TXD_S *)buf;
	TMAC_TXD_L *txd_l = (TMAC_TXD_L *)buf;
	TMAC_TXD_0 *txd_0 = &txd_s->TxD0;
	TMAC_TXD_1 *txd_1 = &txd_s->TxD1;
	UINT32 *txd_7 = &txd_s->TxD7;
	STA_TR_ENTRY *tr_entry = NULL;
	TMAC_TXD_2 *txd_2 = &txd_l->TxD2;
	TMAC_TXD_3 *txd_3 = &txd_l->TxD3;
	TMAC_TXD_5 *txd_5 = &txd_l->TxD5;
	TMAC_TXD_6 *txd_6 = &txd_l->TxD6;
	HTTRANSMIT_SETTING *pTransmit = pTxBlk->pTransmit;
	struct wifi_dev *wdev = NULL;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#if defined(VOW_SUPPORT) && defined(VOW_DVT)
	UCHAR cloned_wcid;
	static UCHAR ucPid;
#endif /* defined(VOW_SUPPORT) && defined(VOW_DVT) */

	wcid = pTxBlk->Wcid;
	wdev = pTxBlk->wdev;

#if defined(VOW_SUPPORT) && defined(VOW_DVT)
	if (pAd->CommonCfg.dbdc_mode)
		cloned_wcid = 2;
	else
		cloned_wcid = 1;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: wcid %d, pTxBlk->Wcid %d\n", __func__, wcid, pTxBlk->Wcid));

	if ((pTxBlk->Wcid > cloned_wcid) && (pTxBlk->Wcid <= pAd->vow_cloned_wtbl_max))
		wcid = pTxBlk->Wcid;
#endif /* defined(VOW_SUPPORT) && defined(VOW_DVT) */

	NdisZeroMemory(txd_l, sizeof(TMAC_TXD_L));

	txd_0 = &txd_s->TxD0;
	txd_1 = &txd_s->TxD1;

	/* DWORD 0 */
	txd_0->p_idx = P_IDX_LMAC;
#if defined(VOW_SUPPORT) && defined(VOW_DVT)
	if ((pTxBlk->Wcid > 0)
		&& (pTxBlk->Wcid <= (MAX_LEN_OF_MAC_TABLE - HW_BEACON_MAX_NUM))
		&& (((pAd->vow_cloned_wtbl_max != 0) && (pTxBlk->Wcid <= pAd->vow_cloned_wtbl_max))
		|| (pAd->vow_cloned_wtbl_max == 0))) {
		pTxBlk->wmm_set = pAd->vow_sta_wmm[wcid];
		pTxBlk->QueIdx = pAd->vow_sta_ac[wcid];
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: wmm set %d, QueIdx %d\n", __func__, pTxBlk->wmm_set, pTxBlk->QueIdx));
	}

#endif /* defined(VOW_SUPPORT) && defined(VOW_DVT) */

	if (pTxBlk->QueIdx < 4) {
		txd_0->q_idx = dmac_wmm_swq_2_hw_ac_que[pTxBlk->wmm_set][pTxBlk->QueIdx];
	} else {
		/* TODO: shiang-usw, consider about MCC case! */
		txd_0->q_idx = pTxBlk->QueIdx;
	}

#ifdef RANDOM_PKT_GEN
	random_write_qidx(pAd, buf, pTxBlk);
#endif

#if (defined(VOW_SUPPORT) && defined(VOW_DVT))
    if ((pTxBlk->Wcid > 0)
		&& (pTxBlk->Wcid <= (MAX_LEN_OF_MAC_TABLE - HW_BEACON_MAX_NUM))
		&& (((pAd->vow_cloned_wtbl_max != 0) && (pTxBlk->Wcid <= pAd->vow_cloned_wtbl_max))
		|| (pAd->vow_cloned_wtbl_max == 0))) {
		txd_0->q_idx = dmac_wmm_swq_2_hw_ac_que[pTxBlk->wmm_set][pTxBlk->QueIdx];
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\x1b[31m%s: q_idx %d, resource_idx %d\x1b[m\n",
		__func__, txd_0->q_idx, pTxBlk->resource_idx));

	if (pTxBlk->wmm_set == 0) {/* the same band */
		if (pAd->vow_bcmc_en == 1)
			txd_0->q_idx = TxQ_IDX_BMC0;
		else if (pAd->vow_bcmc_en == 2)
			txd_0->q_idx = TxQ_IDX_BMC1;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(%d): TxBlk->wmm_set= %d, QID = %d\n",
		__func__, __LINE__, pTxBlk->wmm_set, txd_0->q_idx));
#endif /* defined(VOW_SUPPORT) && defined(VOW_DVT) */

	txd_0->TxByteCount = pTxBlk->tx_bytes_len;

	txd_1->wlan_idx = wcid;

	if (txd_1->wlan_idx >= cap->WtblHwNum) {
		pAd->wrong_wlan_idx_num++;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: 2 wrong wlan index = %d (%d)\n", __func__, txd_1->wlan_idx, HcGetMaxStaNum(pAd)));
		dump_stack();
	}

	if (pTxBlk->TxFrameType & TX_AMSDU_FRAME)
		txd_1->amsdu = 1;


	txd_1->pkt_ft = TMI_PKT_FT_HIF_CT;

	if (!TX_BLK_TEST_FLAG(pTxBlk, fTX_HDR_TRANS)) {
		txd_1->hdr_format = TMI_HDR_FT_NOR_80211;
		TMI_HDR_INFO_VAL(txd_1->hdr_format, 0, 0, 0, 0, 0, pTxBlk->wifi_hdr_len, 0, txd_1->hdr_info);

		if (pTxBlk->HdrPadLen)  /* TODO: depends on QoS to decide if need to padding */
			txd_1->hdr_pad = (TMI_HDR_PAD_MODE_TAIL << TMI_HDR_PAD_BIT_MODE) | 0x1;

	} else {
		BOOLEAN is_vlan = RTMP_GET_PACKET_VLAN(pTxBlk->pPacket);
		BOOLEAN is_etype = ((RTMP_GET_PACKET_PROTOCOL(pTxBlk->pPacket) <= 1500) ? 0 : 1);
		BOOLEAN is_rmvl = 1;

		txd_1->hdr_format = TMI_HDR_FT_NON_80211;
		TMI_HDR_INFO_VAL(txd_1->hdr_format,
						 TX_BLK_TEST_FLAG(pTxBlk, fTX_bMoreData),
						 TX_BLK_TEST_FLAG(pTxBlk, fTX_bWMM_UAPSD_EOSP),
						 is_rmvl,
						 is_vlan,
						 is_etype,
						 pTxBlk->wifi_hdr_len, 0, txd_1->hdr_info);

		if (pTxBlk->HdrPadLen)
			txd_1->hdr_pad = (TMI_HDR_PAD_MODE_HEAD << TMI_HDR_PAD_BIT_MODE) | 0x1;
	}

	txd_1->tid = pTxBlk->UserPriority;

#ifdef CONFIG_CSO_SUPPORT
	if (IS_ASIC_CAP(pAd, fASIC_CAP_CSO)) {
		txd_0->UdpTcpChkSumOffload = 1;
		txd_0->IpChkSumOffload = 1;

		if (txd_0->IpChkSumOffload || txd_0->UdpTcpChkSumOffload) {
			if (txd_1->hdr_format == TMI_HDR_FT_NOR_80211)
				txd_0->EthTypeOffset = (pTxBlk->MpduHeaderLen + 2)>>1;
			else if (txd_1->hdr_format == TMI_HDR_FT_NON_80211)
				txd_0->EthTypeOffset = (LENGTH_802_3 + 2)>>1; /* 8 is (14+2)/2, it is	7615 setting just for 802.3 frame not for 802.11 */
			else
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: txd_1->hdr_format is unexpected\n", __func__));
		}
	}
#endif /* CONFIG_CSO_SUPPORT */

	/*
	  repeater entry doesn't have real wdev could bind.
	  so reference the OmacIdx which is stored in tr_entry.
	  other types entry could reference the OmacIdx of wdev which is connected to the pEntry.
	*/
	if (pMacEntry && IS_ENTRY_REPEATER(pMacEntry)) {
		if (pTxBlk->tr_entry) {
			tr_entry = pTxBlk->tr_entry;
			txd_1->OwnMacAddr = tr_entry->OmacIdx;
		}
	} else if (pMacEntry) {
		txd_1->OwnMacAddr = wdev->OmacIdx;

#if defined(VOW_SUPPORT) && defined(VOW_DVT)
		if (pAd->vow_sta_mbss[pTxBlk->Wcid]) {
			if (wcid <= (MAX_LEN_OF_MAC_TABLE - HW_BEACON_MAX_NUM))
				txd_1->OwnMacAddr = pAd->vow_sta_mbss[wcid]+16;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("\x1b[31m%s: wcid %d, OM %d\x1b[m\n", __func__, pTxBlk->Wcid, txd_1->OwnMacAddr));
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("\x1b[31m%s: wcid %d, OM %d\x1b[m\n", __func__, pTxBlk->Wcid, txd_1->OwnMacAddr));
#endif /* defined(VOW_SUPPORT) && defined(VOW_DVT) */
	} else {
		wdev = pTxBlk->wdev;
		txd_1->OwnMacAddr = wdev->OmacIdx;
	}

	txd_7 = &txd_l->TxD7;
	txd_1->ft = TMI_FT_LONG;

	if (pTransmit) {
		ldpc = pTransmit->field.ldpc;
		phy_mode = pTransmit->field.MODE;
		mcs = phy_mode == MODE_VHT ? pTransmit->field.MCS & 0xf : pTransmit->field.MCS;
		sgi = pTransmit->field.ShortGI;
		stbc = pTransmit->field.STBC;
		bw = (phy_mode <= MODE_OFDM) ? (BW_20) : (pTransmit->field.BW);
		nss = get_nsts_by_mcs(phy_mode, mcs, stbc,
							  phy_mode == MODE_VHT ? ((pTransmit->field.MCS & (0x3 << 4)) >> 4) + 1 : 0);
	}

	txd_l->TxD2.max_tx_time = 0;

#if defined(VOW_SUPPORT) && defined(VOW_DVT)
	if (wcid <= (MAX_LEN_OF_MAC_TABLE - HW_BEACON_MAX_NUM))
		txd_l->TxD2.max_tx_time = pAd->vow_life_time;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		("%s: wcid %d, lifetime %d\n", __func__, wcid, txd_l->TxD2.max_tx_time));
#endif /* defined(VOW_SUPPORT) && defined(VOW_DVT) */

	txd_l->TxD2.bc_mc_pkt = (pTxBlk->TxFrameType == TX_MCAST_FRAME ? 1 : 0);

	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_ForceRate)) {
		txd_l->TxD2.fix_rate = 1;
		if (pAd->CommonCfg.bSeOff != TRUE) {
			if (HcGetBandByWdev(pTxBlk->wdev) == BAND0) {
				if (cap->txd_type == TXD_V1) {
					*txd_7 &= ~SPE_IDX_MASK;
					*txd_7 |= SPE_IDX(BAND0_SPE_IDX);
				} else {
					*txd_7 &= ~CON_SPE_IDX_MASK;
					*txd_7 |= CON_SPE_IDX(BAND0_SPE_IDX);
				}
			} else if (HcGetBandByWdev(pTxBlk->wdev) == BAND1) {
				if (cap->txd_type == TXD_V1) {
					*txd_7 &= ~SPE_IDX_MASK;
					*txd_7 |= SPE_IDX(BAND1_SPE_IDX);
				} else {
					*txd_7 &= ~CON_SPE_IDX_MASK;
					*txd_7 |= CON_SPE_IDX(BAND1_SPE_IDX);
				}
			}
		}
	}

	txd_l->TxD2.frag = pTxBlk->FragIdx;

#ifdef WIFI_EAP_FEATURE
	if (pAd->CommonCfg.mgmt_txpwr_force_on == TRUE) {
		if (txd_2->frm_type == FC_TYPE_MGMT)
			txd_2->pwr_offset = pAd->CommonCfg.txd_txpwr_offset;
	}
#endif

	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bAckRequired)) {
		txd_3->no_ack = 0;
		txd_l->TxD3.remain_tx_cnt = MT_TX_LONG_RETRY;
	} else {
		txd_3->no_ack = 1;
		txd_l->TxD3.remain_tx_cnt = MT_TX_RETRY_UNLIMIT;
	}

#if defined(VOW_SUPPORT) && defined(VOW_DVT)
	if (wcid <= (MAX_LEN_OF_MAC_TABLE - HW_BEACON_MAX_NUM))
		txd_3->no_ack = (pAd->vow_sta_ack[wcid] ? 1 : 0);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: wcid %d, no ack %d\n", __func__, wcid, txd_3->no_ack));
#endif /* defined(VOW_SUPPORT) && defined(VOW_DVT) */

	if (cap->TmrEnable == 1) {
		if (txd_3->no_ack == 0)
			txd_l->TxD2.timing_measure = 1;
	}

	if (IS_CIPHER_NONE(pTxBlk->CipherAlg))
		txd_3->protect_frm = 0;
	else
		txd_3->protect_frm = 1;

	txd_l->TxD5.pid = pTxBlk->Pid;

	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_HDR_TRANS)) {
		if (!TX_BLK_TEST_FLAG(pTxBlk, fTX_MCAST_CLONE))
			txd_5->da_select = TMI_DAS_FROM_MPDU;
		else
			txd_5->da_select = TMI_DAS_FROM_WTBL;

#if (defined(VOW_SUPPORT) && defined(VOW_DVT))
	if (wcid <= (MAX_LEN_OF_MAC_TABLE - HW_BEACON_MAX_NUM))
		txd_5->da_select = TMI_DAS_FROM_WTBL;
#endif /* (defined(VOW_SUPPORT) && defined(VOW_DVT)) */
	}

	if (0 /* bar_sn_ctrl */)
		txd_3->sn_vld = 1;

#if defined(CONFIG_STA_SUPPORT) && defined(CONFIG_PM_BIT_HW_MODE)
	txd_5->pwr_mgmt = TMI_PM_BIT_CFG_BY_HW;
#else
	txd_5->pwr_mgmt = TMI_PM_BIT_CFG_BY_SW;
#endif

#if (defined(VOW_SUPPORT) && defined(VOW_DVT))
	if (wcid <= (MAX_LEN_OF_MAC_TABLE - HW_BEACON_MAX_NUM)) {
		txd_l->TxD5.tx_status_fmt = TXS_FORMAT0;
		txd_l->TxD5.tx_status_2_mcu = 0;
		if (pAd->vow_txs_en)
			txd_l->TxD5.tx_status_2_host = 1;
		txd_l->TxD5.pid = ucPid++%255;
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		("\x1b[32m%s: wcid %d, pid %d\x1b[m\n", __func__, wcid, txd_l->TxD5.pid));
#endif /* (defined(VOW_SUPPORT) && defined(VOW_DVT)) */

	/* DWORD 6 */
	if (txd_2->fix_rate == 1) {
		txd_6->fix_rate_mode = TMI_FIX_RATE_BY_TXD;
		txd_6->bw = ((1 << 2) | bw);
		txd_6->dyn_bw = 0;
		txd_6->TxBF = 0;
		txd_6->ldpc = ldpc;
		txd_6->gi = sgi;

		if (txd_6->fix_rate_mode == TMI_FIX_RATE_BY_TXD) {
			if (phy_mode == MODE_CCK) {
				if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED))
					preamble = SHORT_PREAMBLE;
				else
					preamble = LONG_PREAMBLE;
			}

			txd_6->tx_rate = tx_rate_to_tmi_rate(phy_mode, mcs, nss, stbc, preamble);
		}
	}

	/* DWORD 7 */
#if defined(VOW_SUPPORT) && defined(VOW_DVT)
	if (wcid <= (MAX_LEN_OF_MAC_TABLE - HW_BEACON_MAX_NUM)) {
		if (cap->txd_type == TXD_V1) {
			*txd_7 &= ~SW_TX_TIME_MASK;
			*txd_7 |= SW_TX_TIME(0);
		} else {
			*txd_7 &= ~CON_SW_TX_TIME_MASK;
			*txd_7 |= CON_SW_TX_TIME(0);
		}

	}
#endif /* defined(VOW_SUPPORT) && defined(VOW_DVT)*/

	if (cap->txd_type == TXD_V2) {
		if (TX_BLK_TEST_FLAG(pTxBlk, fTX_HW_AMSDU)) {
			*txd_7 &= ~CON_HW_AMSDU_CAP_MASK;
			*txd_7 |= CON_HW_AMSDU_CAP(1);
		}
	}

	/* dump_tmac_info(pAd, buf); */

#ifdef RED_SUPPORT_BY_HOST
	if (!pAd->red_have_cr4) {
		if (TX_BLK_TEST_FLAG(pTxBlk, fTX_ForceRate))
			RedRecordForceRateFromDriver(pAd, pTxBlk->Wcid);
	}
#endif
}


VOID write_tmac_info_Cache(RTMP_ADAPTER *pAd, UCHAR *buf, TX_BLK *pTxBlk)
{
	pAd->archOps.write_tmac_info(pAd, buf, pTxBlk);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Not Finish yet for HT_MT!!!!\n", __func__, __LINE__));
	return;
}

#ifdef RANDOM_PKT_GEN
VOID random_write_qidx(RTMP_ADAPTER *pAd, UCHAR *buf, TX_BLK *pTxBlk)
{
	TMAC_TXD_L *txd_l = (TMAC_TXD_L *)buf;
	TMAC_TXD_0 *txd_0 = &txd_l->TxD0;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (RandomTxCtrl != 0 && txd_0->q_idx < (cap->WmmHwNum * 4))
		txd_0->q_idx = pTxBlk->lmac_qidx;
}
#endif

INT32 mtd_write_txp_info_by_host(RTMP_ADAPTER *pAd, UCHAR *buf, TX_BLK *tx_blk)
{
	MAC_TX_PKT_T *txp = (MAC_TX_PKT_T *)buf;
	TXD_PTR_LEN_T *txp_ptr_len;
	PKT_TOKEN_CB *pktTokenCb = (PKT_TOKEN_CB *)pAd->PktTokenCb;
	UINT16 token;

#if defined(VOW_SUPPORT) && defined(VOW_DVT)
	/* check queue status */
	if (vow_is_queue_full(pAd, tx_blk->Wcid, tx_blk->QueIdx)) {
		pAd->vow_need_drop_cnt[tx_blk->Wcid]++;
		return NDIS_STATUS_SUCCESS;
	}
#endif /* #if defined(VOW_SUPPORT) && defined(VOW_DVT) */

	if ((tx_blk->amsdu_state == TX_AMSDU_ID_NO) ||
		(tx_blk->amsdu_state == TX_AMSDU_ID_FIRST))
		NdisZeroMemory(txp, sizeof(MAC_TX_PKT_T));

	txp_ptr_len = &txp->arPtrLen[tx_blk->frame_idx / 2];

	if ((tx_blk->frame_idx & 0x1) == 0x0) {
		txp_ptr_len->u4Ptr0 = cpu2le32(PCI_MAP_SINGLE(pAd, tx_blk, 0, 1, RTMP_PCI_DMA_TODEVICE));

		if (RTMP_GET_PACKET_MGMT_PKT(tx_blk->pPacket)) {
			token = cut_through_tx_enq(pktTokenCb, tx_blk->pPacket, TOKEN_TX_MGT, tx_blk->Wcid,
									   txp_ptr_len->u4Ptr0, GET_OS_PKT_LEN(tx_blk->pPacket));
		} else {
			token = cut_through_tx_enq(pktTokenCb, tx_blk->pPacket, TOKEN_TX_DATA, tx_blk->Wcid,
									   txp_ptr_len->u4Ptr0, GET_OS_PKT_LEN(tx_blk->pPacket));
		}

		txp_ptr_len->u2Len0 = (tx_blk->SrcBufLen | TXD_LEN_ML);

		if ((tx_blk->amsdu_state == TX_AMSDU_ID_NO) ||
			(tx_blk->amsdu_state == TX_AMSDU_ID_LAST))
			txp_ptr_len->u2Len0 |= TXD_LEN_AL;

		txp_ptr_len->u2Len0 = cpu2le16(txp_ptr_len->u2Len0);
	} else {
		txp_ptr_len->u4Ptr1 = cpu2le32(PCI_MAP_SINGLE(pAd, tx_blk, 0, 1, RTMP_PCI_DMA_TODEVICE));

		if (RTMP_GET_PACKET_MGMT_PKT(tx_blk->pPacket)) {
			token = cut_through_tx_enq(pktTokenCb, tx_blk->pPacket, TOKEN_TX_MGT, tx_blk->Wcid,
									   txp_ptr_len->u4Ptr1, GET_OS_PKT_LEN(tx_blk->pPacket));
		} else {
			token = cut_through_tx_enq(pktTokenCb, tx_blk->pPacket, TOKEN_TX_DATA, tx_blk->Wcid,
									   txp_ptr_len->u4Ptr1, GET_OS_PKT_LEN(tx_blk->pPacket));
		}

		txp_ptr_len->u2Len1 = (tx_blk->SrcBufLen | TXD_LEN_ML);

		if (tx_blk->amsdu_state == TX_AMSDU_ID_LAST)
			txp_ptr_len->u2Len1 |= TXD_LEN_AL;

		txp_ptr_len->u2Len1 = cpu2le16(txp_ptr_len->u2Len1);
	}
#if defined(VOW_SUPPORT) && defined(VOW_DVT)
	if (tx_blk->Wcid != 0) {
		pAd->vow_queue_map[token][0] = tx_blk->Wcid;
		pAd->vow_queue_map[token][1] = tx_blk->QueIdx;
		pAd->vow_queue_len[tx_blk->Wcid][tx_blk->QueIdx]++;
		/*printk("\x1b[31m%s: enqueue wcid %d, qidx %d, queue len %d....\x1b[m\n",
			__FUNCTION__, tx_blk->Wcid, tx_blk->QueIdx, pAd->vow_queue_len[tx_blk->Wcid][tx_blk->QueIdx]);*/
	}
#endif /* #if defined(VOW_SUPPORT) && defined(VOW_DVT) */
	txp->au2MsduId[tx_blk->frame_idx] = cpu2le16(token | TXD_MSDU_ID_VLD);
	tx_blk->txp_len = sizeof(MAC_TX_PKT_T);
	return NDIS_STATUS_SUCCESS;
}

INT32 mtd_write_txp_info_by_host_v2(RTMP_ADAPTER *pAd, UCHAR *buf, TX_BLK *tx_blk)
{
	MAC_TX_PKT_T *txp = (MAC_TX_PKT_T *)buf;
	TXD_PTR_LEN_T *txp_ptr_len;
	PKT_TOKEN_CB *pktTokenCb = (PKT_TOKEN_CB *)pAd->PktTokenCb;
	UINT16 token;

	if ((tx_blk->amsdu_state == TX_AMSDU_ID_NO) ||
		(tx_blk->amsdu_state == TX_AMSDU_ID_FIRST))
		NdisZeroMemory(txp, sizeof(MAC_TX_PKT_T));

	txp_ptr_len = &txp->arPtrLen[tx_blk->frame_idx / 2];

	if ((tx_blk->frame_idx & 0x1) == 0x0) {
		txp_ptr_len->u4Ptr0 = PCI_MAP_SINGLE(pAd, tx_blk, 0, 1, RTMP_PCI_DMA_TODEVICE);
		txp_ptr_len->u4Ptr0 = cpu2le32(txp_ptr_len->u4Ptr0);
		if (RTMP_GET_PACKET_MGMT_PKT(tx_blk->pPacket)) {
			token = cut_through_tx_enq(pktTokenCb, tx_blk->pPacket, TOKEN_TX_MGT, tx_blk->Wcid,
									   txp_ptr_len->u4Ptr0, GET_OS_PKT_LEN(tx_blk->pPacket));
		} else {
			token = cut_through_tx_enq(pktTokenCb, tx_blk->pPacket, TOKEN_TX_DATA, tx_blk->Wcid,
									   txp_ptr_len->u4Ptr0, GET_OS_PKT_LEN(tx_blk->pPacket));
		}

		txp_ptr_len->u2Len0 = ((tx_blk->SrcBufLen & TXD_LEN_MASK_V2) | TXD_LEN_ML_V2);

		txp_ptr_len->u2Len0 = cpu2le16(txp_ptr_len->u2Len0);
	} else {
		txp_ptr_len->u4Ptr1 = PCI_MAP_SINGLE(pAd, tx_blk, 0, 1, RTMP_PCI_DMA_TODEVICE);
		txp_ptr_len->u4Ptr1 = cpu2le32(txp_ptr_len->u4Ptr1);
		if (RTMP_GET_PACKET_MGMT_PKT(tx_blk->pPacket)) {
			token = cut_through_tx_enq(pktTokenCb, tx_blk->pPacket, TOKEN_TX_MGT, tx_blk->Wcid,
									   txp_ptr_len->u4Ptr1, GET_OS_PKT_LEN(tx_blk->pPacket));
		} else {
			token = cut_through_tx_enq(pktTokenCb, tx_blk->pPacket, TOKEN_TX_DATA, tx_blk->Wcid,
									   txp_ptr_len->u4Ptr1, GET_OS_PKT_LEN(tx_blk->pPacket));
		}

		txp_ptr_len->u2Len1 = ((tx_blk->SrcBufLen & TXD_LEN_MASK_V2) | TXD_LEN_ML_V2);

		txp_ptr_len->u2Len1 = cpu2le16(txp_ptr_len->u2Len1);
	}

	txp->au2MsduId[tx_blk->frame_idx] = cpu2le16(token | TXD_MSDU_ID_VLD);
	tx_blk->txp_len = sizeof(MAC_TX_PKT_T);
	return NDIS_STATUS_SUCCESS;
}


INT32 mtd_write_txp_info_by_cr4(RTMP_ADAPTER *pAd, UCHAR *buf, TX_BLK *pTxBlk)
{
	CR4_TXP_MSDU_INFO *cr4_txp_msdu_info = (CR4_TXP_MSDU_INFO *)buf;
	PKT_TOKEN_CB *pktTokenCb = (PKT_TOKEN_CB *)pAd->PktTokenCb;
	UINT16 token;
	struct wifi_dev *wdev = pTxBlk->wdev;
	UCHAR BssInfoIdx = 0;

	NdisZeroMemory(cr4_txp_msdu_info, sizeof(CR4_TXP_MSDU_INFO));
#ifdef CONFIG_HOTSPOT_R2

	/* Inform CR4 to bypass ProxyARP check on this packet */
	if (RTMP_IS_PACKET_DIRECT_TX(pTxBlk->pPacket))
		cr4_txp_msdu_info->type_and_flags |= CT_INFO_HSR2_TX;

#endif /* HOTSPOT_SUPPORT_R2 */

	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_CT_WithTxD))
		cr4_txp_msdu_info->type_and_flags |= CT_INFO_APPLY_TXD;

	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bClearEAPFrame))
		cr4_txp_msdu_info->type_and_flags |= CT_INFO_NONE_CIPHER_FRAME;

	cr4_txp_msdu_info->buf_ptr[0] = PCI_MAP_SINGLE(pAd, pTxBlk, 0, 1, RTMP_PCI_DMA_TODEVICE);

	if (RTMP_GET_PACKET_MGMT_PKT(pTxBlk->pPacket)) {
		cr4_txp_msdu_info->type_and_flags |= CT_INFO_MGN_FRAME;
		token = cut_through_tx_enq(pktTokenCb, pTxBlk->pPacket, TOKEN_TX_MGT, pTxBlk->Wcid,
								   cr4_txp_msdu_info->buf_ptr[0], GET_OS_PKT_LEN(pTxBlk->pPacket));
	} else {
		token = cut_through_tx_enq(pktTokenCb, pTxBlk->pPacket, TOKEN_TX_DATA, pTxBlk->Wcid,
								   cr4_txp_msdu_info->buf_ptr[0], GET_OS_PKT_LEN(pTxBlk->pPacket));
	}

#ifdef MAC_REPEATER_SUPPORT

	if (pTxBlk->pMacEntry && IS_ENTRY_REPEATER(pTxBlk->pMacEntry))
		cr4_txp_msdu_info->rept_wds_wcid = pTxBlk->pMacEntry->wcid;
	else
		/*TODO: WDS case.*/
#endif
#ifdef A4_CONN
	if (pTxBlk->pMacEntry && IS_ENTRY_A4(pTxBlk->pMacEntry))
		cr4_txp_msdu_info->rept_wds_wcid = pTxBlk->pMacEntry->wcid;
	else
#endif /* A4_CONN */
	{
		cr4_txp_msdu_info->rept_wds_wcid = 0xff;
	}

#ifdef MBSS_AS_WDS_AP_SUPPORT
	if (RTMP_GET_PACKET_WCID(pTxBlk->pPacket)) {
		cr4_txp_msdu_info->rept_wds_wcid = RTMP_GET_PACKET_WCID(pTxBlk->pPacket);
	}
#endif

    BssInfoIdx = wdev->bss_info_argument.ucBssIndex;

	if (token == pktTokenCb->pkt_tkid_invalid) {
		RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_FAILURE);
		return NDIS_STATUS_FAILURE;
	}

	cr4_txp_msdu_info->msdu_token = token;
	cr4_txp_msdu_info->bss_index = BssInfoIdx;
	cr4_txp_msdu_info->buf_num = 1; /* os get scatter. */
	cr4_txp_msdu_info->buf_len[0] = pTxBlk->SrcBufLen;
#ifdef VLAN_SUPPORT
	if (RTMP_GET_PACKET_VLAN(pTxBlk->pPacket) != 0) {
		UINT8 VlanPcp;
		VlanPcp = RTMP_GET_VLAN_PCP(pTxBlk->pPacket);

		if ((VlanPcp >= 0) && (VlanPcp <= 7))
			cr4_txp_msdu_info->reserved = VlanPcp;
	}
#endif /*VLAN_SUPPORT*/

#ifdef DSCP_PRI_SUPPORT
	if ((pTxBlk->DscpMappedPri >= 0)  && (pTxBlk->DscpMappedPri <= 7))
		cr4_txp_msdu_info->reserved = pTxBlk->DscpMappedPri;
#endif
	pTxBlk->txp_len = sizeof(CR4_TXP_MSDU_INFO);

	if (cr4_txp_msdu_info->type_and_flags == 0) {
		pTxBlk->dbdc_band = pTxBlk->resource_idx;
		pTxBlk->DropPkt = FALSE;
		WLAN_HOOK_CALL(WLAN_HOOK_TX, pAd, pTxBlk);

		if (pTxBlk->DropPkt == TRUE) {
			UINT8 Type;

			cut_through_tx_deq(pktTokenCb, token, &Type);
			RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_SUCCESS);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s drop keep alive pkt\n", __func__));
			return NDIS_STATUS_FAILURE;
		}
	}
#ifdef RT_BIG_ENDIAN
	cr4_txp_msdu_info->msdu_token = cpu2le16(cr4_txp_msdu_info->msdu_token);
	cr4_txp_msdu_info->buf_ptr[0] = cpu2le32(cr4_txp_msdu_info->buf_ptr[0]);
	cr4_txp_msdu_info->buf_len[0] = cpu2le16(cr4_txp_msdu_info->buf_len[0]);
	cr4_txp_msdu_info->type_and_flags = cpu2le16(cr4_txp_msdu_info->type_and_flags);
#endif

	return NDIS_STATUS_SUCCESS;
}

INT dump_txp_info(RTMP_ADAPTER *pAd, CR4_TXP_MSDU_INFO *txp_info)
{
	INT cnt;

	hex_dump("CT_TxP_Info Raw Data: ", (UCHAR *)txp_info, sizeof(CR4_TXP_MSDU_INFO));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TXP_Fields:\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\ttype_and_flags=0x%x\n", txp_info->type_and_flags));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tmsdu_token=%d\n", txp_info->msdu_token));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tbss_index=%d\n", txp_info->bss_index));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tbuf_num=%d\n", txp_info->buf_num));

	for (cnt = 0; cnt < txp_info->buf_num; cnt++) {
		if (cnt >= MAX_BUF_NUM_PER_PKT)
			break;

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("\t\tbuf_ptr=0x%x\n", txp_info->buf_ptr[cnt]));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("\t\tbuf_len=%d(0x%x)\n", txp_info->buf_len[cnt], txp_info->buf_len[cnt]));
	}

	return 0;
}

VOID mt_write_tmac_info_beacon(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *tmac_buf, HTTRANSMIT_SETTING *BeaconTransmit, ULONG frmLen)
{
	MAC_TX_INFO mac_info;

	NdisZeroMemory((UCHAR *)&mac_info, sizeof(mac_info));
	mac_info.Type = FC_TYPE_MGMT;
	mac_info.SubType = SUBTYPE_BEACON;
	mac_info.FRAG = FALSE;
	mac_info.CFACK = FALSE;
	mac_info.InsTimestamp = TRUE;
	mac_info.AMPDU = FALSE;
	mac_info.BM = 1;
	mac_info.Ack = FALSE;
	mac_info.NSeq = TRUE;
	mac_info.BASize = 0;
	mac_info.WCID = 0;
	mac_info.Length = frmLen;
	mac_info.TID = 0;
	mac_info.TxRate = 0;
	mac_info.Txopmode = IFS_HTTXOP;
	mac_info.hdr_len = 24;
	mac_info.bss_idx = wdev->func_idx;
	mac_info.SpeEn = 1;
	mac_info.q_idx = HcGetBcnQueueIdx(pAd, wdev);
	mac_info.TxSPriv = wdev->func_idx;
	mac_info.OmacIdx = wdev->OmacIdx;

	if (wdev->bcn_buf.BcnUpdateMethod == BCN_GEN_BY_FW)
		mac_info.IsOffloadPkt = TRUE;
	else
		mac_info.IsOffloadPkt = FALSE;

	mac_info.Preamble = LONG_PREAMBLE;
	mac_info.IsAutoRate = FALSE;

	if (pAd->CommonCfg.bSeOff != TRUE) {
		if (HcGetBandByWdev(wdev) == BAND0)
			mac_info.AntPri = BAND0_SPE_IDX;
		else if (HcGetBandByWdev(wdev) == BAND1)
			mac_info.AntPri = BAND1_SPE_IDX;
	}
	NdisZeroMemory(tmac_buf, sizeof(TMAC_TXD_L));
	mtd_write_tmac_info_fixed_rate(pAd, tmac_buf, &mac_info, BeaconTransmit);
#ifdef RT_BIG_ENDIAN

	if (IS_HIF_TYPE(pAd, HIF_MT))
		MTMacInfoEndianChange(pAd, tmac_buf, TYPE_TXWI, sizeof(TMAC_TXD_L));

#endif
}


INT rtmp_mac_set_band(RTMP_ADAPTER *pAd, int  band)
{
	/* TODO: shiang-7603 */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Not support for HIF_MT yet!\n",
			 __func__, __LINE__));
	return FALSE;
}


INT mt_mac_set_ctrlch(RTMP_ADAPTER *pAd, UINT8 extch)
{
	/* TODO: shiang-7603 */
	return FALSE;
}


#ifdef GREENAP_SUPPORT
INT rtmp_mac_set_mmps(RTMP_ADAPTER *pAd, INT ReduceCorePower)
{
	/* TODO: shiang-7603 */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Not support for HIF_MT yet!\n",
			 __func__, __LINE__));
	return FALSE;
}
#endif /* GREENAP_SUPPORT */


#define BCN_TBTT_OFFSET		64	/*defer 64 us*/
VOID ReSyncBeaconTime(RTMP_ADAPTER *pAd)
{
	/* TODO: shiang-7603 */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Not support for HIF_MT yet!\n",
			 __func__, __LINE__));
}


#ifdef RTMP_MAC_PCI
static INT mt_asic_cfg_hif_tx_ring(RTMP_ADAPTER *pAd, RTMP_TX_RING *ring, UINT32 offset, UINT32 phy_addr, UINT32 cnt)
{
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT16 tx_ring_size = GET_TX_RING_SIZE(cap);

	ring->TxSwFreeIdx = 0;
	ring->TxCpuIdx = 0;
	ring->hw_desc_base = MT_TX_RING_BASE + offset;
	ring->hw_cnt_addr = ring->hw_desc_base + 0x04;
	ring->hw_cidx_addr = ring->hw_desc_base + 0x08;
	ring->hw_didx_addr = ring->hw_desc_base + 0x0c;
	HIF_IO_WRITE32(pAd, ring->hw_desc_base, phy_addr);
	HIF_IO_WRITE32(pAd, ring->hw_cidx_addr, ring->TxCpuIdx);
	HIF_IO_WRITE32(pAd, ring->hw_cnt_addr, tx_ring_size);
	return TRUE;
}


VOID mt_asic_init_txrx_ring(RTMP_ADAPTER *pAd)
{
	UINT32 phy_addr, offset;
	INT i, TxHwRingNum;
	RTMP_CTRL_RING *ctrl_ring;
	RTMP_FWDWLO_RING *FwDwlo_ring;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT16 tx_ring_size = GET_TX_RING_SIZE(cap);
	UINT16 rx0_ring_size = GET_RX0_RING_SIZE(cap);
	UINT16 rx1_ring_size = GET_RX1_RING_SIZE(cap);
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	struct tr_delay_control *tr_delay_ctl = &pAd->tr_ctl.tr_delay_ctl;
	UINT32 reg_value = 0;

	/* Set DMA global configuration except TX_DMA_EN and RX_DMA_EN bits */
	AsicSetWPDMA(pAd, PDMA_TX_RX, FALSE);
	AsicWaitPDMAIdle(pAd, 100, 1000);

	if (IS_ASIC_CAP(pAd, fASIC_CAP_RX_DLY)) {
		reg_value = RX_DLY_INT_CFG;
		tr_delay_ctl->rx_delay_en = TRUE;
	}

	HIF_IO_WRITE32(pAd, MT_DELAY_INT_CFG, reg_value);

	/* Reset DMA Index */
	HIF_IO_WRITE32(pAd, WPDMA_RST_PTR, 0xFFFFFFFF);

	TxHwRingNum = GET_NUM_OF_TX_RING(cap);


	for (i = 0; i < TxHwRingNum; i++) {
		offset = i * 0x10;
		phy_addr = RTMP_GetPhysicalAddressLow(hif->TxRing[i].Cell[0].AllocPa);
		mt_asic_cfg_hif_tx_ring(pAd, &hif->TxRing[i], offset, phy_addr, tx_ring_size);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("-->TX_RING_%d[0x%x]: Base=0x%x, Cnt=%d!\n",
				 i, hif->TxRing[i].hw_desc_base, phy_addr, tx_ring_size));
	}

	/* init CTRL ring index pointer */
	ctrl_ring = &hif->CtrlRing;
	phy_addr = RTMP_GetPhysicalAddressLow(ctrl_ring->Cell[0].AllocPa);

	if (IS_MT7615(pAd))
		offset = MT_RINGREG_DIFF * 2;
	else if (IS_MT7622(pAd))
		offset = MT_RINGREG_DIFF * 15;
	else if (IS_P18(pAd))
		offset = MT_RINGREG_DIFF * 15;
	else if (IS_MT7663(pAd))
		offset = MT_RINGREG_DIFF * 15;
	else
		offset = MT_RINGREG_DIFF * 5;

	ctrl_ring->TxSwFreeIdx = 0;
	ctrl_ring->TxCpuIdx = 0;
	ctrl_ring->hw_desc_base = (MT_TX_RING_BASE  + offset);
	ctrl_ring->hw_cnt_addr = (ctrl_ring->hw_desc_base + 0x04);
	ctrl_ring->hw_cidx_addr = (ctrl_ring->hw_desc_base + 0x08);
	ctrl_ring->hw_didx_addr = (ctrl_ring->hw_desc_base + 0x0c);
	HIF_IO_WRITE32(pAd, ctrl_ring->hw_desc_base, phy_addr);
	HIF_IO_WRITE32(pAd, ctrl_ring->hw_cidx_addr, ctrl_ring->TxCpuIdx);
	HIF_IO_WRITE32(pAd, ctrl_ring->hw_cnt_addr, MGMT_RING_SIZE);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("-->TX_RING_CTRL: Base=0x%x, Cnt=%d!\n",
			 phy_addr, MGMT_RING_SIZE));

	/* init Firmware download ring index pointer */
	FwDwlo_ring = &hif->FwDwloRing;
	phy_addr = RTMP_GetPhysicalAddressLow(FwDwlo_ring->Cell[0].AllocPa);
	offset = MT_RINGREG_DIFF * 3;
	FwDwlo_ring->TxSwFreeIdx = 0;
	FwDwlo_ring->TxCpuIdx = 0;
	FwDwlo_ring->hw_desc_base = (MT_TX_RING_BASE  + offset);
	FwDwlo_ring->hw_cnt_addr = (FwDwlo_ring->hw_desc_base + 0x04);
	FwDwlo_ring->hw_cidx_addr = (FwDwlo_ring->hw_desc_base + 0x08);
	FwDwlo_ring->hw_didx_addr = (FwDwlo_ring->hw_desc_base + 0x0c);
	HIF_IO_WRITE32(pAd, FwDwlo_ring->hw_desc_base, phy_addr);
	HIF_IO_WRITE32(pAd, FwDwlo_ring->hw_cidx_addr, FwDwlo_ring->TxCpuIdx);
	HIF_IO_WRITE32(pAd, FwDwlo_ring->hw_cnt_addr, MGMT_RING_SIZE);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("-->TX_RING_FwDwlo: Base=0x%x, Cnt=%d!\n",
			  phy_addr, MGMT_RING_SIZE));

	/* Init RX Ring0 Base/Size/Index pointer CSR */
	for (i = 0; i < GET_NUM_OF_RX_RING(cap); i++) {
		UINT16 RxRingSize = (i == 0) ? rx0_ring_size : rx1_ring_size;
		RTMP_RX_RING *rx_ring;

		rx_ring = &hif->RxRing[i];
		offset = i * 0x10;
		phy_addr = RTMP_GetPhysicalAddressLow(rx_ring->Cell[0].AllocPa);
		rx_ring->RxSwReadIdx = 0;
		rx_ring->RxCpuIdx = RxRingSize - 1;
		rx_ring->hw_desc_base = MT_RX_RING_BASE + offset;
		rx_ring->hw_cidx_addr = MT_RX_RING_CIDX + offset;
		rx_ring->hw_didx_addr = MT_RX_RING_DIDX + offset;
		rx_ring->hw_cnt_addr = MT_RX_RING_CNT + offset;
		HIF_IO_WRITE32(pAd, rx_ring->hw_desc_base, phy_addr);
		HIF_IO_WRITE32(pAd, rx_ring->hw_cidx_addr, rx_ring->RxCpuIdx);
		HIF_IO_WRITE32(pAd, rx_ring->hw_cnt_addr, RxRingSize);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("-->RX_RING%d[0x%x]: Base=0x%x, Cnt=%d\n",
				 i, rx_ring->hw_desc_base, phy_addr, RxRingSize));
	}
}
#endif /* RTMP_MAC_PCI */

INT mt_mac_pse_init(RTMP_ADAPTER *pAd)
{
	/* Don't Support this now! */
	return FALSE;
}


#define HW_TX_RATE_TO_MODE(_x)			(((_x) & (0x7 << 6)) >> 6)
#define HW_TX_RATE_TO_MCS(_x, _mode)		((_x) & (0x3f))
#define HW_TX_RATE_TO_NSS(_x)				(((_x) & (0x3 << 9)) >> 9)
#define HW_TX_RATE_TO_STBC(_x)			(((_x) & (0x1 << 11)) >> 11)

#define MAX_TX_MODE 5
static char *HW_TX_MODE_STR[] = {"CCK", "OFDM", "HT-Mix", "HT-GF", "VHT", "N/A"};
static char *HW_TX_RATE_CCK_STR[] = {"1M", "2M", "5.5M", "11M", "N/A"};
static char *HW_TX_RATE_OFDM_STR[] = {"6M", "9M", "12M", "18M", "24M", "36M", "48M", "54M", "N/A"};

static char *hw_rate_ofdm_str(UINT16 ofdm_idx)
{
	switch (ofdm_idx) {
	case 11: /* 6M */
		return HW_TX_RATE_OFDM_STR[0];

	case 15: /* 9M */
		return HW_TX_RATE_OFDM_STR[1];

	case 10: /* 12M */
		return HW_TX_RATE_OFDM_STR[2];

	case 14: /* 18M */
		return HW_TX_RATE_OFDM_STR[3];

	case 9: /* 24M */
		return HW_TX_RATE_OFDM_STR[4];

	case 13: /* 36M */
		return HW_TX_RATE_OFDM_STR[5];

	case 8: /* 48M */
		return HW_TX_RATE_OFDM_STR[6];

	case 12: /* 54M */
		return HW_TX_RATE_OFDM_STR[7];

	default:
		return HW_TX_RATE_OFDM_STR[8];
	}
}

static char *hw_rate_str(UINT8 mode, UINT16 rate_idx)
{
	if (mode == 0)
		return rate_idx < 4 ? HW_TX_RATE_CCK_STR[rate_idx] : HW_TX_RATE_CCK_STR[4];
	else if (mode == 1)
		return hw_rate_ofdm_str(rate_idx);
	else
		return "MCS";
}


VOID dump_wtbl_basic_info(RTMP_ADAPTER *pAd, struct wtbl_struc *tb)
{
	struct wtbl_basic_info *basic_info = &tb->peer_basic_info;
	struct wtbl_tx_rx_cap *trx_cap = &tb->trx_cap;
	struct wtbl_rate_tb *rate_info = &tb->auto_rate_tb;
	UCHAR addr[MAC_ADDR_LEN];
	UINT16 txrate[8], rate_idx, txmode, mcs, nss, stbc;

	NdisMoveMemory(&addr[0], (UCHAR *)basic_info, 4);
	addr[0] = basic_info->wtbl_d1.field.addr_0 & 0xff;
	addr[1] = ((basic_info->wtbl_d1.field.addr_0 & 0xff00) >> 8);
	addr[2] = ((basic_info->wtbl_d1.field.addr_0 & 0xff0000) >> 16);
	addr[3] = ((basic_info->wtbl_d1.field.addr_0 & 0xff000000) >> 24);
	addr[4] = basic_info->wtbl_d0.field.addr_4 & 0xff;
	addr[5] = basic_info->wtbl_d0.field.addr_5 & 0xff;
	hex_dump("WTBL Raw Data", (UCHAR *)tb, sizeof(struct wtbl_struc));
	/* Basic Info (DW0~DW1) */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("Basic Info(DW0~DW1):\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tAddr: %02x:%02x:%02x:%02x:%02x:%02x(D0[B0~15], D1[B0~31])\n",
			  PRINT_MAC(addr)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tMUAR_Idx(D0[B16~21]):%d\n",
			  basic_info->wtbl_d0.field.muar_idx));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\trc_a1/rc_a2:%d/%d(D0[B22]/D0[B29])\n",
			  basic_info->wtbl_d0.field.rc_a1, basic_info->wtbl_d0.field.rc_a2));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tKID:%d/RCID:%d/RKV:%d/RV:%d/IKV:%d/WPI_FLAG:%d(D0[B23~24], D0[B25], D0[B26], D0[B28], D0[B30])\n",
			  basic_info->wtbl_d0.field.kid, basic_info->wtbl_d0.field.rc_id,
			  basic_info->wtbl_d0.field.rkv, basic_info->wtbl_d0.field.rv,
			  basic_info->wtbl_d0.field.ikv, basic_info->wtbl_d0.field.wpi_flg));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tGID_SU:%d(D0[B31])\n", basic_info->wtbl_d0.field.gid_su));
	/* TRX Cap(DW2~5) */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("TRX Cap(DW2~5):\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tsw/DIS_RHTR:%d/%d\n",
			  trx_cap->wtbl_d2.field.SW, trx_cap->wtbl_d2.field.dis_rhtr));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tHT/VHT/HT-LDPC/VHT-LDPC/DYN_BW/MMSS:%d/%d/%d/%d/%d/%d\n",
			  trx_cap->wtbl_d2.field.ht, trx_cap->wtbl_d2.field.vht,
			  trx_cap->wtbl_d5.field.ldpc, trx_cap->wtbl_d5.field.ldpc_vht,
			  trx_cap->wtbl_d5.field.dyn_bw, trx_cap->wtbl_d5.field.mm));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tTx Power Offset:0x%x(%d)\n",
			  trx_cap->wtbl_d5.field.txpwr_offset, ((trx_cap->wtbl_d5.field.txpwr_offset == 0x0) ? 0 : (trx_cap->wtbl_d5.field.txpwr_offset - 0x20))));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tFCAP/G2/G4/G8/G16/CBRN:%d/%d/%d/%d/%d/%d\n",
			  trx_cap->wtbl_d5.field.fcap, trx_cap->wtbl_d5.field.g2,
			  trx_cap->wtbl_d5.field.g4, trx_cap->wtbl_d5.field.g8,
			  trx_cap->wtbl_d5.field.g16, trx_cap->wtbl_d5.field.cbrn));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tHT-TxBF(tibf/tebf):%d/%d, VHT-TxBF(tibf/tebf):%d/%d, PFMU_IDX=%d\n",
			  trx_cap->wtbl_d2.field.tibf, trx_cap->wtbl_d2.field.tebf,
			  trx_cap->wtbl_d2.field.tibf_vht, trx_cap->wtbl_d2.field.tebf_vht,
			  trx_cap->wtbl_d2.field.pfmu_idx));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tSPE_IDX=%d\n", trx_cap->wtbl_d3.field.spe_idx));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tBA Enable:0x%x, BAFail Enable:%d\n",
			  trx_cap->wtbl_d4.field.ba_en, trx_cap->wtbl_d3.field.baf_en));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tQoS Enable:%d\n",
			  trx_cap->wtbl_d5.field.qos));

	if (trx_cap->wtbl_d4.field.ba_en) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("\t\tBA WinSize: TID 0 - %d, TID 1 - %d\n",
				  trx_cap->wtbl_d4.field.ba_win_size_tid_0,
				  trx_cap->wtbl_d4.field.ba_win_size_tid_1));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("\t\tBA WinSize: TID 2 - %d, TID 3 - %d\n",
				  trx_cap->wtbl_d4.field.ba_win_size_tid_2,
				  trx_cap->wtbl_d4.field.ba_win_size_tid_3));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("\t\tBA WinSize: TID 4 - %d, TID 5 - %d\n",
				  trx_cap->wtbl_d4.field.ba_win_size_tid_4,
				  trx_cap->wtbl_d4.field.ba_win_size_tid_5));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("\t\tBA WinSize: TID 6 - %d, TID 7 - %d\n",
				  trx_cap->wtbl_d4.field.ba_win_size_tid_6,
				  trx_cap->wtbl_d4.field.ba_win_size_tid_7));
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tpartial_aid:%d\n", trx_cap->wtbl_d2.field.partial_aid));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\twpi_even:%d\n", trx_cap->wtbl_d2.field.wpi_even));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tAAD_OM/CipherSuit:%d/%d\n",
			 trx_cap->wtbl_d2.field.AAD_OM, trx_cap->wtbl_d2.field.cipher_suit));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\taf:%d\n", trx_cap->wtbl_d3.field.af));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\trdg_ba:%d/rdg capability:%d\n", trx_cap->wtbl_d3.field.rdg_ba, trx_cap->wtbl_d3.field.r));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tcipher_suit:%d\n", trx_cap->wtbl_d2.field.cipher_suit));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tFromDS:%d\n", trx_cap->wtbl_d5.field.fd));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tToDS:%d\n", trx_cap->wtbl_d5.field.td));
	/* Rate Info (DW6~8) */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Rate Info (DW6~8):\n"));
	txrate[0] = (rate_info->wtbl_d6.word & 0xfff);
	txrate[1] = (rate_info->wtbl_d6.word & (0xfff << 12)) >> 12;
	txrate[2] = ((rate_info->wtbl_d6.word & (0xff << 24)) >> 24) | ((rate_info->wtbl_d7.word & 0xf) << 8);
	txrate[3] = ((rate_info->wtbl_d7.word & (0xfff << 4)) >> 4);
	txrate[4] = ((rate_info->wtbl_d7.word & (0xfff << 16)) >> 16);
	txrate[5] = ((rate_info->wtbl_d7.word & (0xf << 28)) >> 28) | ((rate_info->wtbl_d8.word & (0xff)) << 4);
	txrate[6] = ((rate_info->wtbl_d8.word & (0xfff << 8)) >> 8);
	txrate[7] = ((rate_info->wtbl_d8.word & (0xfff << 20)) >> 20);

	for (rate_idx = 0; rate_idx < 8; rate_idx++) {
		txmode = HW_TX_RATE_TO_MODE(txrate[rate_idx]);
		mcs = HW_TX_RATE_TO_MCS(txrate[rate_idx], txmode);
		nss = HW_TX_RATE_TO_NSS(txrate[rate_idx]);
		stbc = HW_TX_RATE_TO_STBC(txrate[rate_idx]);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("\tRate%d(0x%x):TxMode=%d(%s), TxRate=%d(%s), Nsts=%d, STBC=%d\n",
				  rate_idx + 1, txrate[rate_idx],
				  txmode, (txmode < MAX_TX_MODE ? HW_TX_MODE_STR[txmode] : HW_TX_MODE_STR[MAX_TX_MODE]),
				  mcs, hw_rate_str(txmode, mcs), nss, stbc));
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tpsm:%d\n", trx_cap->wtbl_d3.field.psm));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tskip_tx:%d\n", trx_cap->wtbl_d3.field.skip_tx));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tdu_i_psm:%d\n", trx_cap->wtbl_d3.field.du_i_psm));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\ti_psm:%d\n", trx_cap->wtbl_d3.field.i_psm));
}



VOID dump_wtbl_base_info(RTMP_ADAPTER *pAd)
{
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tWTBL Basic Info:\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tMemBaseAddr:0x%x\n",
			 pAd->mac_ctrl.wtbl_base_addr[0]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tEntrySize/Cnt:%d/%d\n",
			 pAd->mac_ctrl.wtbl_entry_size[0],
			 pAd->mac_ctrl.wtbl_entry_cnt[0]));
}


VOID dump_wtbl_info(RTMP_ADAPTER *pAd, UINT wtbl_idx)
{
	INT idx, start_idx, end_idx, wtbl_len;
	UINT32 wtbl_offset, addr;
	UCHAR *wtbl_raw_dw = NULL;
	struct wtbl_entry wtbl_ent;
	struct wtbl_struc *wtbl = &wtbl_ent.wtbl;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UCHAR wtbl_num = cap->WtblHwNum;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Dump WTBL info of WLAN_IDX:%d\n", wtbl_idx));

	if (wtbl_idx == RESERVED_WCID) {
		start_idx = 0;
		end_idx = (wtbl_num - 1);
	} else if (wtbl_idx < wtbl_num)
		start_idx = end_idx = wtbl_idx;
	else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s():Invalid WTBL index(%d)!\n",
				  __func__, wtbl_idx));
		return;
	}

	wtbl_len = sizeof(WTBL_STRUC);
	os_alloc_mem(pAd, (UCHAR **)&wtbl_raw_dw, wtbl_len);

	if (!wtbl_raw_dw) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s():AllocMem fail(%d)!\n",
				  __func__, wtbl_idx));
		return;
	}

	for (idx = start_idx; idx <= end_idx; idx++) {
		wtbl_ent.wtbl_idx = idx;
		wtbl_ent.wtbl_addr = pAd->mac_ctrl.wtbl_base_addr[0] + idx * pAd->mac_ctrl.wtbl_entry_size[0];

		/* Read WTBL Entries */
		for (wtbl_offset = 0; wtbl_offset <= wtbl_len; wtbl_offset += 4) {
			addr = wtbl_ent.wtbl_addr + wtbl_offset;
			HW_IO_READ32(pAd, addr, (UINT32 *)(&wtbl_raw_dw[wtbl_offset]));
		}

		NdisCopyMemory((UCHAR *)wtbl, &wtbl_raw_dw[0], sizeof(struct wtbl_struc));
		dump_wtbl_entry(pAd, &wtbl_ent);
	}

	os_free_mem(wtbl_raw_dw);
}


VOID dump_wtbl_entry(RTMP_ADAPTER *pAd, struct wtbl_entry *ent)
{
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Dump WTBL SW Entry[%d] info\n", ent->wtbl_idx));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tWTBL info:\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tAddr=0x%x\n", ent->wtbl_addr));
	dump_wtbl_basic_info(pAd, &ent->wtbl);
}


INT mt_wtbl_get_entry234(RTMP_ADAPTER *pAd, UCHAR widx, struct wtbl_entry *ent)
{
	struct rtmp_mac_ctrl *wtbl_ctrl;
	UINT8 wtbl_idx;

	wtbl_ctrl = &pAd->mac_ctrl;

	if (wtbl_ctrl->wtbl_entry_cnt[0] > 0)
		wtbl_idx = (widx < wtbl_ctrl->wtbl_entry_cnt[0] ? widx : wtbl_ctrl->wtbl_entry_cnt[0] - 1);
	else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s():pAd->mac_ctrl not init yet!\n", __func__));
		return FALSE;
	}

	ent->wtbl_idx = wtbl_idx;
	ent->wtbl_addr = wtbl_ctrl->wtbl_base_addr[0] +
					 wtbl_idx * wtbl_ctrl->wtbl_entry_size[0];
	return TRUE;
}

INT mt_wtbl_init_ByFw(struct _RTMP_ADAPTER *pAd)
{
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	pAd->mac_ctrl.wtbl_base_addr[0] = (UINT32)WTBL_BASE_ADDR;
	pAd->mac_ctrl.wtbl_entry_size[0] = (UINT16)WTBL_PER_ENTRY_SIZE;
	pAd->mac_ctrl.wtbl_entry_cnt[0] = (UINT8)cap->WtblHwNum;
	return TRUE;
}

INT mt_wtbl_init_ByDriver(RTMP_ADAPTER *pAd)
{
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	pAd->mac_ctrl.wtbl_base_addr[0] = (UINT32)WTBL_BASE_ADDR;
	pAd->mac_ctrl.wtbl_entry_size[0] = (UINT16)WTBL_PER_ENTRY_SIZE;
	pAd->mac_ctrl.wtbl_entry_cnt[0] = (UINT8)cap->WtblHwNum;
	return TRUE;
}

INT mt_wtbl_init(RTMP_ADAPTER *pAd)
{
#ifdef CONFIG_WTBL_TLV_MODE
	/* We can use mt_wtbl_init_ByWtblTlv() to replace mt_wtbl_init_ByDriver() when there are no */
	/* function using mt_wtbl_get_entry234 and pAd->mac_ctrl */
	mt_wtbl_init_ByFw(pAd);
#else
	mt_wtbl_init_ByDriver(pAd);
#endif /* CONFIG_WTBL_TLV_MODE */
	return TRUE;
}
#define MCAST_WCID_TO_REMOVE 0
INT mt_hw_tb_init(RTMP_ADAPTER *pAd, BOOLEAN bHardReset)
{
	/* TODO: shiang-7603 */
	mt_wtbl_init(pAd);
	return TRUE;
}


/*
	ASIC register initialization sets
*/
INT mt_mac_init(RTMP_ADAPTER *pAd)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s()-->\n", __func__));
	mt_mac_pse_init(pAd);
	MtAsicInitMac(pAd);

	/* re-set specific MAC registers for individual chip */
	/* TODO: Shiang-usw-win, here we need call "mt7603_init_mac_cr" for windows! */
	if (ops->AsicMacInit != NULL)
		ops->AsicMacInit(pAd);

	/* auto-fall back settings */
	MtAsicAutoFallbackInit(pAd);
	MtAsicSetMacMaxLen(pAd);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("<--%s()\n", __func__));
	return TRUE;
}


VOID mt_chip_info_show(RTMP_ADAPTER *pAd)
{
#if defined(MT7615_FPGA) || defined(MT7622_FPGA) || defined(P18_FPGA) || defined(MT7663_FPGA)

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		UINT32 ver, date_code, rev;
		UINT32 mac_val;

		RTMP_IO_READ32(pAd, 0x2700, &ver);
		RTMP_IO_READ32(pAd, 0x2704, &rev);
		RTMP_IO_READ32(pAd, 0x2708, &date_code);
		RTMP_IO_READ32(pAd, 0x201f8, &mac_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("##########################################\n"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("%s(%d): MT Series FPGA Version:\n", __func__, __LINE__));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("\tFGPA1: Code[0x700]:0x%08x, [0x704]:0x%08x, [0x708]:0x%08x\n",
				  ver, rev, date_code));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("\tFPGA2: Version[0x201f8]:0x%08x\n", mac_val));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("##########################################\n"));
	}

#endif /* defined(MT7615_FPGA) || defined(MT7622_FPGA) || defined(P18_FPGA) || defined(MT7663_FPGA) */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("MAC[Ver:Rev/ID=0x%08x : 0x%08x]\n",
			  pAd->MACVersion, pAd->ChipID));
}

INT mt_nic_asic_init(RTMP_ADAPTER *pAd)
{
	INT ret = NDIS_STATUS_SUCCESS;
	MT_DMASCH_CTRL_T DmaSchCtrl;

	if (AsicWaitPDMAIdle(pAd, 100, 1000) != TRUE) {
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)) {
			ret =  NDIS_STATUS_FAILURE;
			return ret;
		}
	}

#if  defined(COMPOS_WIN)  || defined(COMPOS_TESTMODE_WIN)
	DmaSchCtrl.bBeaconSpecificGroup = FALSE;
#else

	if (MTK_REV_GTE(pAd, MT7603, MT7603E1) && MTK_REV_LT(pAd, MT7603, MT7603E2))
		DmaSchCtrl.bBeaconSpecificGroup = FALSE;
	else
		DmaSchCtrl.bBeaconSpecificGroup = TRUE;

#endif
	DmaSchCtrl.mode = DMA_SCH_LMAC;
#ifdef DMA_SCH_SUPPORT
	MtAsicDMASchedulerInit(pAd, DmaSchCtrl);
#endif
#ifdef RTMP_PCI_SUPPORT

	if (IS_PCI_INF(pAd)) {
		pAd->CommonCfg.bPCIeBus = FALSE;
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():device act as PCI%s driver\n",
				 __func__, (pAd->CommonCfg.bPCIeBus ? "-E" : "")));
	}

#endif /* RTMP_PCI_SUPPORT */
	/* TODO: shiang-7603, init MAC setting */
	/* TODO: shiang-7603, init beacon buffer */
	mt_mac_init(pAd);
	mt_hw_tb_init(pAd, TRUE);
#ifdef HDR_TRANS_RX_SUPPORT
#ifdef VLAN_SUPPORT
	AsicRxHeaderTransCtl(pAd, TRUE, FALSE, FALSE, FALSE, FALSE);
#else
	AsicRxHeaderTransCtl(pAd, TRUE, FALSE, FALSE, TRUE, FALSE);
#endif /* VLAN_SUPPORT */
	AsicRxHeaderTaranBLCtl(pAd, 0, TRUE, ETH_TYPE_EAPOL);
	AsicRxHeaderTaranBLCtl(pAd, 1, TRUE, ETH_TYPE_WAI);
	AsicRxHeaderTaranBLCtl(pAd, 2, TRUE, ETH_TYPE_FASTROAMING);
#endif

	if (IS_ASIC_CAP(pAd, fASIC_CAP_MCU_OFFLOAD))
		MtAsicAutoBATrigger(pAd, TRUE, BA_TRIGGER_OFFLOAD_TIMEOUT);

	return ret;
}

INT mtd_check_hw_resource(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR resource_idx)
{
	UINT free_num;
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	RTMP_TX_RING *tx_ring = &hif->TxRing[resource_idx];
	PKT_TOKEN_CB *pktTokenCb;

	free_num = pci_get_tx_resource_free_num_nolock(pAd, resource_idx);

	if (free_num < tx_ring->tx_ring_low_water_mark) {
		pci_inc_resource_full_cnt(pAd, resource_idx);
		pci_set_resource_state(pAd, resource_idx, TX_RING_LOW);
		return NDIS_STATUS_RESOURCES;
	}

	pktTokenCb = (PKT_TOKEN_CB *)pAd->PktTokenCb;
	free_num = pktTokenCb->tx_id_list.list->FreeTokenCnt;

	if (free_num < pktTokenCb->TxTokenLowWaterMark) {
			cut_through_inc_token_full_cnt(pAd);
			cut_through_set_token_state(pAd, TX_TOKEN_LOW);
		return NDIS_STATUS_FAILURE;
	}

	return NDIS_STATUS_SUCCESS;
}

INT mtd_hw_tx(RTMP_ADAPTER *pAd, TX_BLK *tx_blk)
{
	RTMP_ARCH_OP *op = &pAd->archOps;
	USHORT free_cnt = 1;
	INT32 ret = NDIS_STATUS_SUCCESS;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (!TX_BLK_TEST_FLAG(tx_blk, fTX_MCU_OFFLOAD))
		tx_bytes_calculate(pAd, tx_blk);

	if (TX_BLK_TEST_FLAG(tx_blk, fTX_HDR_TRANS)) {

		if ((tx_blk->amsdu_state == TX_AMSDU_ID_NO) ||
			(tx_blk->amsdu_state == TX_AMSDU_ID_LAST))
			op->write_tmac_info(pAd, &tx_blk->HeaderBuf[0], tx_blk);

		ret = op->write_txp_info(pAd, &tx_blk->HeaderBuf[cap->tx_hw_hdr_len], tx_blk);

		if (ret != NDIS_STATUS_SUCCESS)
			return ret;

#if defined(VOW_SUPPORT) && defined(VOW_DVT)
		/* if needr_to_drop_counter > 0 then skip to update PDMA descritpor */
		if (pAd->vow_need_drop_cnt[tx_blk->Wcid]) {
			/*printk("\x1b[31m%s: skip write_tx_resource....\x1b[m\n", __FUNCTION__);*/
			return ret;
		}
#endif /* #if defined(VOW_SUPPORT) && defined(VOW_DVT) */

		if ((tx_blk->amsdu_state == TX_AMSDU_ID_NO) ||
			(tx_blk->amsdu_state == TX_AMSDU_ID_LAST))
			op->write_tx_resource(pAd, tx_blk, TRUE, &free_cnt);
	} else {
		INT dot11_meta_hdr_len, tx_hw_hdr_len;
		PNDIS_PACKET pkt;
		NDIS_STATUS status;
		UCHAR *dot11_hdr;
		TX_BLK new_tx_blk, *p_new_tx_blk = &new_tx_blk;

		NdisCopyMemory(p_new_tx_blk, tx_blk, sizeof(*tx_blk));
		dot11_meta_hdr_len = tx_blk->MpduHeaderLen + tx_blk->HdrPadLen;
		tx_hw_hdr_len = cap->tx_hw_hdr_len;
		dot11_hdr = &tx_blk->HeaderBuf[tx_hw_hdr_len];
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 ("%s():DataFrm, MpduHdrL=%d,WFHdrL=%d,HdrPadL=%d,HwRsvL=%d, NeedCopyHdrLen=%d\n",
				  __func__, tx_blk->MpduHeaderLen, tx_blk->wifi_hdr_len, tx_blk->HdrPadLen,
				  tx_blk->hw_rsv_len, dot11_meta_hdr_len));
		/* original packet only 802.3 payload, so create a new packet including 802.11 header for cut-through transfer */
		status = RTMPAllocateNdisPacket(pAd, &pkt,
										dot11_hdr, dot11_meta_hdr_len,
										tx_blk->pSrcBufData, tx_blk->SrcBufLen);

		if (status != NDIS_STATUS_SUCCESS)
			return NDIS_STATUS_FAILURE;

		if ((tx_blk->FragIdx == TX_FRAG_ID_NO) || (tx_blk->FragIdx == TX_FRAG_ID_LAST))
			RELEASE_NDIS_PACKET(pAd, tx_blk->pPacket, NDIS_STATUS_SUCCESS);

		p_new_tx_blk->HeaderBuf = op->get_hif_buf(pAd, p_new_tx_blk, tx_blk->resource_idx, tx_blk->TxFrameType);
		p_new_tx_blk->pPacket = pkt;
		p_new_tx_blk->pSrcBufData = GET_OS_PKT_DATAPTR(pkt);
		p_new_tx_blk->SrcBufLen = GET_OS_PKT_LEN(pkt);
		TX_BLK_SET_FLAG(p_new_tx_blk, fTX_CT_WithTxD);

		if ((tx_blk->amsdu_state == TX_AMSDU_ID_NO) ||
			(tx_blk->amsdu_state == TX_AMSDU_ID_LAST))
			op->write_tmac_info(pAd, &p_new_tx_blk->HeaderBuf[0], p_new_tx_blk);

		/* because 802.11 header already in new packet, not in HeaderBuf, so set below parameters to 0 */
		p_new_tx_blk->MpduHeaderLen = 0;
		p_new_tx_blk->wifi_hdr_len = 0;
		p_new_tx_blk->HdrPadLen = 0;
		p_new_tx_blk->hw_rsv_len = 0;

		tx_blk->resource_idx = p_new_tx_blk->resource_idx;
		ret = op->write_txp_info(pAd, &p_new_tx_blk->HeaderBuf[cap->tx_hw_hdr_len], p_new_tx_blk);

		if (ret != NDIS_STATUS_SUCCESS)
			return ret;

		if ((tx_blk->amsdu_state == TX_AMSDU_ID_NO) ||
			(tx_blk->amsdu_state == TX_AMSDU_ID_LAST))
			op->write_tx_resource(pAd, p_new_tx_blk, TRUE, &free_cnt);
	}

	return NDIS_STATUS_SUCCESS;
}

INT mtd_mlme_hw_tx(RTMP_ADAPTER *pAd, UCHAR *tmac_info, MAC_TX_INFO *info, HTTRANSMIT_SETTING *transmit, TX_BLK *tx_blk)
{
	RTMP_ARCH_OP *op = &pAd->archOps;
	UINT16 free_cnt = 1;
	INT32 ret = NDIS_STATUS_SUCCESS;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	op->write_tmac_info_fixed_rate(pAd, tmac_info, info, transmit);

	tx_blk->pSrcBufData = tx_blk->pSrcBufHeader;
	NdisCopyMemory(&tx_blk->HeaderBuf[0], tx_blk->pSrcBufData, cap->tx_hw_hdr_len);
	tx_blk->pSrcBufData += cap->tx_hw_hdr_len;
	tx_blk->SrcBufLen -= cap->tx_hw_hdr_len;
	tx_blk->MpduHeaderLen = 0;
	tx_blk->wifi_hdr_len = 0;
	tx_blk->HdrPadLen = 0;
	tx_blk->hw_rsv_len = 0;

	ret = op->write_txp_info(pAd, &tx_blk->HeaderBuf[cap->tx_hw_hdr_len], tx_blk);

	if (ret != NDIS_STATUS_SUCCESS)
		return ret;

	op->write_tx_resource(pAd, tx_blk, TRUE, &free_cnt);
	return NDIS_STATUS_SUCCESS;
}

#ifdef CONFIG_ATE
INT32 mtd_ate_hw_tx(RTMP_ADAPTER *pAd, TMAC_INFO *info, TX_BLK *tx_blk)
{
	RTMP_ARCH_OP *op = &pAd->archOps;
	UINT16 free_cnt = 1;
	INT32 ret = NDIS_STATUS_SUCCESS;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	MtWriteTMacInfo(pAd, tx_blk->pSrcBufHeader, info);

	tx_blk->pSrcBufData = tx_blk->pSrcBufHeader;
	NdisCopyMemory(&tx_blk->HeaderBuf[0], tx_blk->pSrcBufData, cap->tx_hw_hdr_len);
	tx_blk->pSrcBufData += cap->tx_hw_hdr_len;
	tx_blk->SrcBufLen -= cap->tx_hw_hdr_len;
	tx_blk->MpduHeaderLen = 0;
	tx_blk->wifi_hdr_len = 0;
	tx_blk->HdrPadLen = 0;
	tx_blk->hw_rsv_len = 0;
	/* Indicate which Tx ring to be sent by band_idx */
#ifdef MT7615
	if (IS_MT7615(pAd))
		tx_blk->resource_idx = info->band_idx;
#endif
	ret = op->write_txp_info(pAd, &tx_blk->HeaderBuf[cap->tx_hw_hdr_len], tx_blk);

	if (ret != NDIS_STATUS_SUCCESS)
		return ret;

	op->write_tx_resource(pAd, tx_blk, TRUE, &free_cnt);
	return NDIS_STATUS_SUCCESS;
}
#endif
