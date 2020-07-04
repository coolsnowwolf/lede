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

#ifdef DBG
VOID dump_rxinfo(RTMP_ADAPTER *pAd, RXINFO_STRUC *pRxInfo)
{
	hex_dump("RxInfo Raw Data", (UCHAR *)pRxInfo, sizeof(RXINFO_STRUC));

	DBGPRINT(RT_DEBUG_OFF, ("RxInfo Fields:\n"));

	DBGPRINT(RT_DEBUG_OFF, ("\tBA=%d\n", pRxInfo->BA));
	DBGPRINT(RT_DEBUG_OFF, ("\tDATA=%d\n", pRxInfo->DATA));
	DBGPRINT(RT_DEBUG_OFF, ("\tNULLDATA=%d\n", pRxInfo->NULLDATA));
	DBGPRINT(RT_DEBUG_OFF, ("\tFRAG=%d\n", pRxInfo->FRAG));
	DBGPRINT(RT_DEBUG_OFF, ("\tU2M=%d\n", pRxInfo->U2M));
	DBGPRINT(RT_DEBUG_OFF, ("\tMcast=%d\n", pRxInfo->Mcast));
	DBGPRINT(RT_DEBUG_OFF, ("\tBcast=%d\n", pRxInfo->Bcast));
	DBGPRINT(RT_DEBUG_OFF, ("\tMyBss=%d\n", pRxInfo->MyBss));
	DBGPRINT(RT_DEBUG_OFF, ("\tCrc=%d\n", pRxInfo->Crc));
	DBGPRINT(RT_DEBUG_OFF, ("\tCipherErr=%d\n", pRxInfo->CipherErr));
	DBGPRINT(RT_DEBUG_OFF, ("\tAMSDU=%d\n", pRxInfo->AMSDU));
	DBGPRINT(RT_DEBUG_OFF, ("\tHTC=%d\n", pRxInfo->HTC));
	DBGPRINT(RT_DEBUG_OFF, ("\tRSSI=%d\n", pRxInfo->RSSI));
	DBGPRINT(RT_DEBUG_OFF, ("\tL2PAD=%d\n", pRxInfo->L2PAD));
	DBGPRINT(RT_DEBUG_OFF, ("\tAMPDU=%d\n", pRxInfo->AMPDU));
	DBGPRINT(RT_DEBUG_OFF, ("\tDecrypted=%d\n", pRxInfo->Decrypted));
	DBGPRINT(RT_DEBUG_OFF, ("\tBssIdx3=%d\n", pRxInfo->BssIdx3));
	DBGPRINT(RT_DEBUG_OFF, ("\twapi_kidx=%d\n", pRxInfo->wapi_kidx));
	DBGPRINT(RT_DEBUG_OFF, ("\tpn_len=%d\n", pRxInfo->pn_len));
	DBGPRINT(RT_DEBUG_OFF, ("\tsw_fc_type0=%d\n", pRxInfo->sw_fc_type0));
	DBGPRINT(RT_DEBUG_OFF, ("\tsw_fc_type1=%d\n", pRxInfo->sw_fc_type1));
}


static char *hdr_fmt_str[]={
	"Non-80211-Frame",
	"Command-Frame",
	"Normal-80211-Frame",
	"enhanced-80211-Frame",
};


static char *p_idx_str[]={"LMAC", "MCU"};
static char *q_idx_lmac_str[] = {"AC0", "AC1", "AC2", "AC3", "AC4", "AC5", "AC6",
								"BCN", "BMC",
								"AC10", "AC11", "AC12", "AC13", "AC14",
								"Invalid"};
static char *q_idx_mcu_str[] = {"RQ0", "RQ1", "RQ2", "RQ3", "Invalid"};

VOID dump_tmac_info(RTMP_ADAPTER *pAd, UCHAR *tmac_info)
{
	TMAC_TXD_S *txd_s = (TMAC_TXD_S *)tmac_info;
	TMAC_TXD_0 *txd_0 = (TMAC_TXD_0 *)tmac_info;
	TMAC_TXD_1 *txd_1 = (TMAC_TXD_1 *)(tmac_info + sizeof(TMAC_TXD_0));
	UCHAR q_idx = 0;

	hex_dump("TMAC_Info Raw Data: ", (UCHAR *)tmac_info, pAd->chipCap.TXWISize);

	DBGPRINT(RT_DEBUG_OFF, ("TMAC_TXD Fields:\n"));

	DBGPRINT(RT_DEBUG_OFF, ("\tTMAC_TXD_0:\n"));
	DBGPRINT(RT_DEBUG_OFF, ("\t\tPortID=%d(%s)\n", txd_0->p_idx,
				p_idx_str[txd_0->p_idx]));
	if (txd_0->p_idx == P_IDX_LMAC)
		q_idx = txd_0->q_idx % 0xf;
	else
		q_idx = txd_0->q_idx % 0x5;
	DBGPRINT(RT_DEBUG_OFF, ("\t\tQueID=%d(%s %s)\n", txd_0->q_idx,
				(txd_0->p_idx == P_IDX_LMAC ? "LMAC" : "MCU"),
				txd_0->p_idx == P_IDX_LMAC ? q_idx_lmac_str[q_idx] : q_idx_mcu_str[q_idx]));
	DBGPRINT(RT_DEBUG_OFF, ("\t\tTxByteCnt=%d\n", txd_0->tx_byte_cnt));

	DBGPRINT(RT_DEBUG_OFF, ("\tTMAC_TXD_1:\n"));
	DBGPRINT(RT_DEBUG_OFF, ("\t\tWlan_idx=%d\n", txd_1->wlan_idx));
	DBGPRINT(RT_DEBUG_OFF, ("\t\tHdrInfo=0x%x\n", txd_1->hdr_info));
	DBGPRINT(RT_DEBUG_OFF, ("\t\tHdrFmt=%d(%s)\n",
				txd_1->hdr_format,
				hdr_fmt_str[txd_1->hdr_format] ));
	switch (txd_1->hdr_format) {
		case TMI_HDR_FT_NON_80211:
			DBGPRINT(RT_DEBUG_OFF, ("\t\t\tMRD=%d, EOSP=%d, RMVL=%d, VLAN=%d, ETYP=%d\n",
						txd_1->hdr_info & (1<< TMI_HDR_INFO_0_BIT_MRD),
						txd_1->hdr_info & (1<< TMI_HDR_INFO_0_BIT_EOSP),
						txd_1->hdr_info & (1<< TMI_HDR_INFO_0_BIT_RMVL),
						txd_1->hdr_info & (1<< TMI_HDR_INFO_0_BIT_VLAN),
						txd_1->hdr_info & (1<< TMI_HDR_INFO_0_BIT_ETYP)));
			break;
		case TMI_HDR_FT_CMD:
			DBGPRINT(RT_DEBUG_OFF, ("\t\t\tRsvd=0x%x\n", txd_1->hdr_info));
			break;
		case TMI_HDR_FT_NOR_80211:
			DBGPRINT(RT_DEBUG_OFF, ("\t\t\tHeader Len=%d(WORD)\n",
						txd_1->hdr_info & TMI_HDR_INFO_2_MASK_LEN));
			break;
		case TMI_HDR_FT_ENH_80211:
			DBGPRINT(RT_DEBUG_OFF, ("\t\t\tEOSP=%d, AMS=%d\n",
						txd_1->hdr_info & (1<< TMI_HDR_INFO_3_BIT_EOSP),
						txd_1->hdr_info & (1<< TMI_HDR_INFO_3_BIT_AMS)));
			break;
	}
	DBGPRINT(RT_DEBUG_OFF, ("\t\tFormatType=%d(%s format)\n", txd_1->ft,
				txd_1->ft == TMI_FT_LONG ? "Long - 8 DWORD" : "Short - 3 DWORD"));
	DBGPRINT(RT_DEBUG_OFF, ("\t\tHdrPad=%d\n", txd_1->hdr_pad));
	DBGPRINT(RT_DEBUG_OFF, ("\t\tNoAck=%d\n", txd_1->no_ack));
	DBGPRINT(RT_DEBUG_OFF, ("\t\tTID=%d\n", txd_1->tid));
	DBGPRINT(RT_DEBUG_OFF, ("\t\tProtect Frame=%d\n", txd_1->protect_frm));
	DBGPRINT(RT_DEBUG_OFF, ("\t\town_mac=%d\n", txd_1->own_mac));

	if (txd_s->txd_1.ft == TMI_FT_LONG) {
		TMAC_TXD_L *txd_l = (TMAC_TXD_L *)tmac_info;
		TMAC_TXD_2 *txd_2 = &txd_l->txd_2;
		TMAC_TXD_3 *txd_3 = &txd_l->txd_3;
		TMAC_TXD_4 *txd_4 = &txd_l->txd_4;
		TMAC_TXD_5 *txd_5 = &txd_l->txd_5;
		TMAC_TXD_6 *txd_6 = &txd_l->txd_6;

		DBGPRINT(RT_DEBUG_OFF, ("\tTMAC_TXD_2:\n"));
		DBGPRINT(RT_DEBUG_OFF, ("\t\tReamingLife/MaxTx time=%d\n", txd_2->max_tx_time));
		DBGPRINT(RT_DEBUG_OFF, ("\t\tNDP=%d\n", txd_2->ndp));
		DBGPRINT(RT_DEBUG_OFF, ("\t\tNDPA=%d\n", txd_2->ndpa));
		DBGPRINT(RT_DEBUG_OFF, ("\t\tSounding=%d\n", txd_2->sounding));
		DBGPRINT(RT_DEBUG_OFF, ("\t\tbc_mc_pkt=%d\n", txd_2->bc_mc_pkt));
		DBGPRINT(RT_DEBUG_OFF, ("\t\tBIP=%d\n", txd_2->bip));
		DBGPRINT(RT_DEBUG_OFF, ("\t\tDuration=%d\n", txd_2->duration));
		DBGPRINT(RT_DEBUG_OFF, ("\t\tHE(HTC Exist)=%d\n", txd_2->htc_vld));
		DBGPRINT(RT_DEBUG_OFF, ("\t\tFRAG=%d\n", txd_2->frag));
		DBGPRINT(RT_DEBUG_OFF, ("\t\tpwr_offset=%d\n", txd_2->pwr_offset));
		DBGPRINT(RT_DEBUG_OFF, ("\t\tba_disable=%d\n", txd_2->ba_disable));
		DBGPRINT(RT_DEBUG_OFF, ("\t\ttiming_measure=%d\n", txd_2->timing_measure));
		DBGPRINT(RT_DEBUG_OFF, ("\t\tfix_rate=%d\n", txd_2->fix_rate));

		DBGPRINT(RT_DEBUG_OFF, ("\tTMAC_TXD_3:\n"));
		DBGPRINT(RT_DEBUG_OFF, ("\t\ttx_cnt=%d\n", txd_3->tx_cnt));
		DBGPRINT(RT_DEBUG_OFF, ("\t\tremain_tx_cnt=%d\n", txd_3->remain_tx_cnt));
		DBGPRINT(RT_DEBUG_OFF, ("\t\tsn=%d\n", txd_3->sn));
		DBGPRINT(RT_DEBUG_OFF, ("\t\tpn_vld=%d\n", txd_3->pn_vld));
		DBGPRINT(RT_DEBUG_OFF, ("\t\tsn_vld=%d\n", txd_3->sn_vld));

		DBGPRINT(RT_DEBUG_OFF, ("\tTMAC_TXD_4:\n"));
		DBGPRINT(RT_DEBUG_OFF, ("\t\tpn_low=0x%x\n", txd_4->pn_low));

		DBGPRINT(RT_DEBUG_OFF, ("\tTMAC_TXD_5:\n"));
		DBGPRINT(RT_DEBUG_OFF, ("\t\tpwr_mgmt=%d\n", txd_5->pwr_mgmt));
		DBGPRINT(RT_DEBUG_OFF, ("\t\ttx_status_2_host=%d\n", txd_5->tx_status_2_host));
		DBGPRINT(RT_DEBUG_OFF, ("\t\ttx_status_2_mcu=%d\n", txd_5->tx_status_2_mcu));
		DBGPRINT(RT_DEBUG_OFF, ("\t\ttx_status_fmt=%d\n", txd_5->tx_status_fmt));
		if (txd_5->tx_status_2_host || txd_5->tx_status_2_mcu) {
			DBGPRINT(RT_DEBUG_OFF, ("\t\tpid=%d\n", txd_5->pid));
		}
		if (txd_2->fix_rate) {
			DBGPRINT(RT_DEBUG_OFF, ("\t\tda_select=%d\n", txd_5->da_select));
		}
		DBGPRINT(RT_DEBUG_OFF, ("\t\tpn_high=0x%x\n", txd_5->pn_high));

		DBGPRINT(RT_DEBUG_OFF, ("\tTMAC_TXD_6:\n"));
		DBGPRINT(RT_DEBUG_OFF, ("\t\ttx_rate=0x%x\n", txd_6->tx_rate));
		if (txd_2->fix_rate) {
			DBGPRINT(RT_DEBUG_OFF, ("\t\tfix_rate_mode=%d\n", txd_6->fix_rate_mode));
			DBGPRINT(RT_DEBUG_OFF, ("\t\tant_id=%d\n", txd_6->ant_id));
			DBGPRINT(RT_DEBUG_OFF, ("\t\tbw=%d\n", txd_6->bw));
			DBGPRINT(RT_DEBUG_OFF, ("\t\tspe_en=%d\n", txd_6->spe_en));
			DBGPRINT(RT_DEBUG_OFF, ("\t\tant_pri=%d\n", txd_6->ant_pri));
			DBGPRINT(RT_DEBUG_OFF, ("\t\tdyn_bw=%d\n", txd_6->dyn_bw));
			DBGPRINT(RT_DEBUG_OFF, ("\t\tETxBF=%d\n", txd_6->ETxBF));
			DBGPRINT(RT_DEBUG_OFF, ("\t\tITxBF=%d\n", txd_6->ITxBF));
			DBGPRINT(RT_DEBUG_OFF, ("\t\tldpc=%d(%s)\n", txd_6->ldpc, txd_6->ldpc == 0 ? "BCC" : "LDPC"));
			DBGPRINT(RT_DEBUG_OFF, ("\t\tGI=%d(%s)\n", txd_6->gi, txd_6->gi == 0? "LONG" : "SHORT"));
		}
	}
}


static char *rmac_info_type_str[]={
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
	struct rxd_base_struct *rxd_base = (struct rxd_base_struct *)rmac_info;

	hex_dump("RMAC_Info Raw Data: ", rmac_info, sizeof(struct rxd_base_struct));

	DBGPRINT(RT_DEBUG_TRACE, ("\tRxData_BASE:\n"));
	DBGPRINT(RT_DEBUG_TRACE, ("\t\tPktType=%d(%s)\n",
					rxd_base->rxd_0.pkt_type,
					rxd_pkt_type_str(rxd_base->rxd_0.pkt_type)));
	DBGPRINT(RT_DEBUG_TRACE, ("\t\tGroupValid=%x\n", rxd_base->rxd_0.grp_vld));
	DBGPRINT(RT_DEBUG_TRACE, ("\t\tRxByteCnt=%d\n", rxd_base->rxd_0.rx_byte_cnt));
	DBGPRINT(RT_DEBUG_TRACE, ("\t\tIP/UT=%d/%d\n", rxd_base->rxd_0.ip_sum, rxd_base->rxd_0.ut_sum));
	DBGPRINT(RT_DEBUG_TRACE, ("\t\tEtherTypeOffset=%d(WORD)\n", rxd_base->rxd_0.eth_type_offset));
	DBGPRINT(RT_DEBUG_TRACE, ("\t\tHTC/UC2ME/MC/BC=%d/%d/%d/%d\n",
				rxd_base->rxd_1.htc_vld, rxd_base->rxd_1.u2m,
				rxd_base->rxd_1.mcast, rxd_base->rxd_1.bcast));
	DBGPRINT(RT_DEBUG_TRACE, ("\t\tBeacon with BMCast/Ucast=%d/%d\n",
				rxd_base->rxd_1.beacon_mc, rxd_base->rxd_1.beacon_uc));
	DBGPRINT(RT_DEBUG_TRACE, ("\t\tKeyID=%d\n", rxd_base->rxd_1.key_id));
	DBGPRINT(RT_DEBUG_TRACE, ("\t\tChFrequency=%x\n", rxd_base->rxd_1.ch_freq));
	DBGPRINT(RT_DEBUG_TRACE, ("\t\tHdearLength(MAC)=%d\n", rxd_base->rxd_1.mac_hdr_len));
	DBGPRINT(RT_DEBUG_TRACE, ("\t\tHeaerOffset(HO)=%d\n", rxd_base->rxd_1.hdr_offset));
	DBGPRINT(RT_DEBUG_TRACE, ("\t\tHeaerTrans(H)=%d\n", rxd_base->rxd_1.hdr_trans));
	DBGPRINT(RT_DEBUG_TRACE, ("\t\tPayloadFormat(PF)=%d\n", rxd_base->rxd_1.payload_format));
	DBGPRINT(RT_DEBUG_TRACE, ("\t\tBSSID=%d\n", rxd_base->rxd_1.bssid));

	DBGPRINT(RT_DEBUG_TRACE, ("\t\tWlanIndex=%d\n", rxd_base->rxd_2.wlan_idx));
	DBGPRINT(RT_DEBUG_TRACE, ("\t\tTID=%d\n", rxd_base->rxd_2.tid));
	DBGPRINT(RT_DEBUG_TRACE, ("\t\tSEC Mode=%d\n", rxd_base->rxd_2.sec_mode));
	DBGPRINT(RT_DEBUG_TRACE, ("\t\tSW BIT=%d\n", rxd_base->rxd_2.sw_bit));
	DBGPRINT(RT_DEBUG_TRACE, ("\t\tFCE Error(FC)=%d\n", rxd_base->rxd_2.fcs_err));
	DBGPRINT(RT_DEBUG_TRACE, ("\t\tCipher Mismatch(CM)=%d\n", rxd_base->rxd_2.cm));
	DBGPRINT(RT_DEBUG_TRACE, ("\t\tCipher Length Mismatch(CLM)=%d\n", rxd_base->rxd_2.clm));
	DBGPRINT(RT_DEBUG_TRACE, ("\t\tICV Err(I)=%d\n", rxd_base->rxd_2.icv_err));
	DBGPRINT(RT_DEBUG_TRACE, ("\t\tTKIP MIC Err(T)=%d\n", rxd_base->rxd_2.tkip_mic_err));
	DBGPRINT(RT_DEBUG_TRACE, ("\t\tLength Mismatch(LM)=%d\n", rxd_base->rxd_2.len_mismatch));
	DBGPRINT(RT_DEBUG_TRACE, ("\t\tDeAMSDU Fail(DAF)=%d\n", rxd_base->rxd_2.de_amsdu_err));
	DBGPRINT(RT_DEBUG_TRACE, ("\t\tExceedMax Rx Length(EL)=%d\n", rxd_base->rxd_2.exceed_max_rx_len));
	DBGPRINT(RT_DEBUG_TRACE, ("\t\tLLC-SNAP mismatch(LLC-MIS, for HdrTrans)=%d\n", rxd_base->rxd_2.llc_mis));
	DBGPRINT(RT_DEBUG_TRACE, ("\t\tUndefined VLAN tagged Type(UDF_VLT)=%d\n", rxd_base->rxd_2.UDF_VLT));
	DBGPRINT(RT_DEBUG_TRACE, ("\t\tFragment Frame(FRAG)=%d\n", rxd_base->rxd_2.frag));
	DBGPRINT(RT_DEBUG_TRACE, ("\t\tNull Frame(NULL)=%d\n", rxd_base->rxd_2.null_frm));
	DBGPRINT(RT_DEBUG_TRACE, ("\t\tNon Data Frame(NDATA)=%d\n", rxd_base->rxd_2.ndata));
	DBGPRINT(RT_DEBUG_TRACE, ("\t\tNon-AMPDU Subframe(NASF)=%d\n", rxd_base->rxd_2.non_ampdu_sub_frm));
	DBGPRINT(RT_DEBUG_TRACE, ("\t\tNon AMPDU(NAMP)=%d\n", rxd_base->rxd_2.non_ampdu));

	if (rxd_base->rxd_0.grp_vld) {
		// TODO: dump group info!!
	}
}


VOID dump_rmac_info_txs(RTMP_ADAPTER *pAd, UCHAR *rmac_info)
{
	//TXS_FRM_STRUC *txs_frm = (TXS_FRM_STRUC *)rmac_info;

}


VOID dump_rmac_info_rxv(RTMP_ADAPTER *pAd, UCHAR *rmac_info)
{
	//RXV_FRM_STRUC *rxv_frm = (RXV_FRM_STRUC *)rmac_info;

}


VOID dump_rmac_info_rfb(RTMP_ADAPTER *pAd, UCHAR *rmac_info)
{
	//RXD_BASE_STRUCT *rfb_frm = (RXD_BASE_STRUCT *)rmac_info;

}


VOID dump_rmac_info_tmr(RTMP_ADAPTER *pAd, UCHAR *rmac_info)
{
	//TMR_FRM_STRUC *rxd_base = (TMR_FRM_STRUC *)rmac_info;

}


VOID dump_rmac_info(RTMP_ADAPTER *pAd, UCHAR *rmac_info)
{
	union rmac_rxd_0 *rxd_0 = (union rmac_rxd_0 *)rmac_info;
	INT pkt_type;

	rxd_0 = (union rmac_rxd_0 *)rmac_info;
	pkt_type = RMAC_RX_PKT_TYPE(rxd_0->word);
	DBGPRINT(RT_DEBUG_OFF, ("RMAC_RXD Header Format :%s\n",
				rxd_pkt_type_str(pkt_type)));
	switch (pkt_type)
	{
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


static RTMP_REG_PAIR mac_cr_seg[]={
	{0x20000, 0x20010}, /* WF_CFG */
	{WF_TRB_BASE, 0x21040}, /* WF_CFG */
	{WF_AGG_BASE, 0x21240}, /* WF_CFG */
	{WF_ARB_BASE, 0x21440}, /* WF_CFG */
	{0,0},
};


VOID dump_mt_mac_cr(RTMP_ADAPTER *pAd)
{
	INT index = 0;
	UINT32 mac_val, mac_addr, seg_s, seg_e;

	while (mac_cr_seg[index].Register != 0)
	{
		seg_s = mac_cr_seg[index].Register;
		seg_e = mac_cr_seg[index].Value;
		DBGPRINT(RT_DEBUG_OFF, ("Dump WF_CFG Segment(Start=0x%x, End=0x%x)\n",
					seg_s, seg_e));
		for (mac_addr = seg_s; mac_addr < seg_e; mac_addr += 4)
		{
			RTMP_IO_READ32(pAd, mac_addr, &mac_val);
			DBGPRINT(RT_DEBUG_OFF, ("MAC[0x%x] = 0x%x\n", mac_addr, mac_val));
		}

		index++;
	};
}
#endif /* DBG */


INT mt_mac_fifo_stat_update(RTMP_ADAPTER *pAd)
{
	if (pAd->chipCap.hif_type == HIF_MT) {
		DBGPRINT(RT_DEBUG_OFF, ("%s(%d): Not support for HIF_MT yet!\n",
							__FUNCTION__, __LINE__));
	}

	return FALSE;
}


VOID NicGetTxRawCounters(
	IN RTMP_ADAPTER *pAd,
	IN TX_STA_CNT0_STRUC *pStaTxCnt0,
	IN TX_STA_CNT1_STRUC *pStaTxCnt1)
{
	// TODO: shiang-7603
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
VOID NICUpdateRawCounters(RTMP_ADAPTER *pAd)
{
	RTEnqueueInternalCmd(pAd, CMDTHREAD_PERODIC_CR_ACCESS_NIC_UPDATE_RAW_COUNTERS, NULL, 0);
}

NTSTATUS MtCmdNICUpdateRawCounters(RTMP_ADAPTER *pAd, PCmdQElmt CMDQelmt)
{
	UINT32 OldValue, ampdu_range_cnt[4];
	UINT32 mac_val, rx_err_cnt, fcs_err_cnt;
	//UINT32 TxSuccessCount = 0, bss_tx_cnt;
	/* for PER debug */
	UINT32 AmpduTxCount = 0;
	UINT32 AmpduTxSuccessCount = 0;

	UINT32  OFDM_PD_CNT, CCK_PD_CNT,OFDM_MDRDY_CNT, CCK_MDRDY_CNT;
	UINT32  PDCnt = 0;
       UINT32  MDRDYCnt = 0;
	UINT32  value, CCAcount = 0;

#ifdef DBG_DIAGNOSE
	UINT32 bss_tx_cnt = 0;
	UINT32 TxFailCount = 0;
#endif
	//UINT32 TxRetryCount = 0;
	COUNTER_RALINK		*pRalinkCounters;


	if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE))
	{
		DBGPRINT(RT_DEBUG_INFO, ("%s(%d): H/W in PM4, return\n", __FUNCTION__, __LINE__));
		return NDIS_STATUS_SUCCESS;
	}


	pRalinkCounters = &pAd->RalinkCounters;



	pRalinkCounters = &pAd->RalinkCounters;

	RTMP_IO_READ32(pAd, MIB_MSDR14, &AmpduTxCount);
	AmpduTxCount &= 0xFFFFFF;
	RTMP_IO_READ32(pAd, MIB_MSDR15, &AmpduTxSuccessCount);
	AmpduTxSuccessCount &= 0xFFFFFF;
	RTMP_IO_READ32(pAd, MIB_MSDR4, &rx_err_cnt);
	fcs_err_cnt = (rx_err_cnt >> 16 ) & 0xffff;
	RTMP_IO_READ32(pAd, MIB_MDR2, &mac_val);
	ampdu_range_cnt[0] = mac_val & 0xffff;
	ampdu_range_cnt[1] =  (mac_val >> 16 ) & 0xffff;
	RTMP_IO_READ32(pAd, MIB_MDR3, &mac_val);
	ampdu_range_cnt[2] = mac_val & 0xffff;
	ampdu_range_cnt[3] =  (mac_val >> 16 ) & 0xffff;
#ifdef DBG_DIAGNOSE
	RTMP_IO_READ32(pAd, MIB_MB0SDR1, &mac_val);
	TxFailCount = (mac_val >> 16) & 0xffff;
	// TODO: shiang, now only check BSS0
	RTMP_IO_READ32(pAd, WTBL_BTCRn, &bss_tx_cnt);
	bss_tx_cnt = (bss_tx_cnt >> 16) & 0xffff;
#endif /* DBG_DIAGNOSE */

#ifdef ANTI_INTERFERENCE_SUPPORT
	RTMP_IO_READ32(pAd, MIB_MSDR12, &mac_val);
	pRalinkCounters->OneSecTxAMpduCnt = mac_val & 0xFFFF;
	pRalinkCounters->OneSecTxBACnt = (mac_val >> 16) & 0xFFFF;
#endif /* ANTI_INTERFERENCE_SUPPORT */

#ifdef STATS_COUNT_SUPPORT
	pAd->WlanCounters.AmpduSuccessCount.u.LowPart += AmpduTxSuccessCount;
	pAd->WlanCounters.AmpduFailCount.u.LowPart += (AmpduTxCount - AmpduTxSuccessCount);
#endif /* STATS_COUNT_SUPPORT */

	pAd->RalinkCounters.OneSecRxFcsErrCnt += fcs_err_cnt;

#ifdef STATS_COUNT_SUPPORT
	/* Update FCS counters*/
	OldValue= pAd->WlanCounters.FCSErrorCount.u.LowPart;
	pAd->WlanCounters.FCSErrorCount.u.LowPart += fcs_err_cnt; /* >> 7);*/
	if (pAd->WlanCounters.FCSErrorCount.u.LowPart < OldValue)
		pAd->WlanCounters.FCSErrorCount.u.HighPart++;
#endif /* STATS_COUNT_SUPPORT */
#ifdef CONFIG_QA
    if (pAd->ATECtrl.bQAEnabled == TRUE)
        pAd->ATECtrl.RxMacFCSErrCount += fcs_err_cnt;
#endif
	/* Add FCS error count to private counters*/
	pRalinkCounters->OneSecRxFcsErrCnt += fcs_err_cnt;
	OldValue = pRalinkCounters->RealFcsErrCount.u.LowPart;
	pRalinkCounters->RealFcsErrCount.u.LowPart += fcs_err_cnt;
	if (pRalinkCounters->RealFcsErrCount.u.LowPart < OldValue)
		pRalinkCounters->RealFcsErrCount.u.HighPart++;

	pAd->Counters8023.RxNoBuffer += (rx_err_cnt & 0xffff);

	pAd->WlanCounters.TxAggRange1Count.u.LowPart += ampdu_range_cnt[0];
	pAd->WlanCounters.TxAggRange2Count.u.LowPart += ampdu_range_cnt[1];
	pAd->WlanCounters.TxAggRange3Count.u.LowPart += ampdu_range_cnt[2];
	pAd->WlanCounters.TxAggRange4Count.u.LowPart += ampdu_range_cnt[3];

#ifdef DBG_DIAGNOSE
	{
		RtmpDiagStruct *pDiag;
		UCHAR ArrayCurIdx, i;
		struct dbg_diag_info *diag_info;

		pDiag = &pAd->DiagStruct;
		ArrayCurIdx = pDiag->ArrayCurIdx;

		if (pDiag->inited == 0)
		{
			NdisZeroMemory(pDiag, sizeof(struct _RtmpDiagStrcut_));
			pDiag->ArrayStartIdx = pDiag->ArrayCurIdx = 0;
			pDiag->wcid = 0;
			pDiag->inited = 1;
            pDiag->diag_cond = 0;
		}
		else
		{
			diag_info = &pDiag->diag_info[ArrayCurIdx];

			if ((pDiag->wcid == 0) && (pAd->MacTab.Size > 0)) {
				UCHAR idx;
				MAC_TABLE_ENTRY *pEntry;

				for (idx = 1; idx < MAX_LEN_OF_MAC_TABLE; idx++) {
					pEntry = &pAd->MacTab.Content[idx];
					if (IS_ENTRY_CLIENT(pEntry) && (pEntry->Sst==SST_ASSOC)) {
						pDiag->wcid = idx;
						DBGPRINT(RT_DEBUG_TRACE, ("%s(): DBG_DIAGNOSE Start to monitor the SwQ depth of WCID[%d], ArrayCurIdx=%d\n",
									__FUNCTION__, pDiag->wcid, pDiag->ArrayCurIdx));
						break;
					}
				}
			}

			/* Tx*/
			diag_info->TxFailCnt = TxFailCount;
#ifdef DBG_TX_AGG_CNT
			diag_info->TxAggCnt = AmpduTxCount;
			diag_info->TxNonAggCnt = bss_tx_cnt - AmpduTxCount; // only useful when only one BSS case

			diag_info->TxAMPDUCnt[0] = ampdu_range_cnt[0];
			diag_info->TxAMPDUCnt[1] = ampdu_range_cnt[1];
			diag_info->TxAMPDUCnt[2] = ampdu_range_cnt[2];
			diag_info->TxAMPDUCnt[3] = ampdu_range_cnt[3];
#endif /* DBG_TX_AGG_CNT */

			diag_info->RxCrcErrCnt = fcs_err_cnt;

			INC_RING_INDEX(pDiag->ArrayCurIdx,  DIAGNOSE_TIME);
			ArrayCurIdx = pDiag->ArrayCurIdx;

			NdisZeroMemory(&pDiag->diag_info[ArrayCurIdx], sizeof(pDiag->diag_info[ArrayCurIdx]));

			if (pDiag->ArrayCurIdx == pDiag->ArrayStartIdx)
				INC_RING_INDEX(pDiag->ArrayStartIdx,  DIAGNOSE_TIME);
		}
	}
#endif /* DBG_DIAGNOSE */


	/* update one sec falseCCA (PD CNT - MDRDY CNT) */
	RTMP_IO_READ32(pAd,RO_PHYCTRL_STS0,&PDCnt);
	OFDM_PD_CNT = (PDCnt >> 16);
	CCK_PD_CNT = (PDCnt & 0xFFFF);
	RTMP_IO_READ32(pAd,RO_PHYCTRL_STS5,&MDRDYCnt);
	OFDM_MDRDY_CNT = (MDRDYCnt >> 16);
	CCK_MDRDY_CNT = (MDRDYCnt & 0xFFFF);
	CCAcount = ((OFDM_PD_CNT - OFDM_MDRDY_CNT) + (CCK_PD_CNT - CCK_MDRDY_CNT));
	RTMP_IO_READ32(pAd,CR_PHYCTRL_2,&value);
	value |= (1<<6); /* BIT6: CR_STSCNT_RST */
	RTMP_IO_WRITE32(pAd,CR_PHYCTRL_2,value);
	value &= (~(1<<6));
	RTMP_IO_WRITE32(pAd,CR_PHYCTRL_2,value);
	value |= (1<<7); /* BIT7: CR_STSCNT_EN */
	RTMP_IO_WRITE32(pAd,CR_PHYCTRL_2,value);
	
	//DBGPRINT(RT_DEBUG_WARN, ("one-sec FalseCCACnt %d (CCK %d + OFDM %d)\n"
		//,CCAcount,(CCK_PD_CNT - CCK_MDRDY_CNT),(OFDM_PD_CNT - OFDM_MDRDY_CNT)));

	pAd->RalinkCounters.OneSecFalseCCACnt = CCAcount;
	pAd->RalinkCounters.OneSecCCKFalseCCACnt = (CCK_PD_CNT - CCK_MDRDY_CNT);
	pAd->RalinkCounters.OneSecOFDMFalseCCACnt = (OFDM_PD_CNT - OFDM_MDRDY_CNT);
	pAd->RalinkCounters.FalseCCACnt = CCAcount;
		
#ifdef SMART_CARRIER_SENSE_SUPPORT
        pAd->SCSCtrl.PdCount = PDCnt;
        pAd->SCSCtrl.MdrdyCount = MDRDYCnt;
	DBGPRINT(RT_DEBUG_TRACE,("False CCA (one second) = %ld (CCK %d + OFDM %d)\n" , (ULONG)pAd->RalinkCounters.OneSecFalseCCACnt,pAd->RalinkCounters.OneSecCCKFalseCCACnt,pAd->RalinkCounters.OneSecOFDMFalseCCACnt));
        pAd->SCSCtrl.FalseCCA = pAd->RalinkCounters.FalseCCACnt;
#endif /* SMART_CARRIER_SENSE_SUPPORT */

	return NDIS_STATUS_SUCCESS;
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


UCHAR tmi_rate_map_cck_lp[]={
	TMI_TX_RATE_CCK_1M_LP,
	TMI_TX_RATE_CCK_2M_LP,
	TMI_TX_RATE_CCK_5M_LP,
	TMI_TX_RATE_CCK_11M_LP,
};

UCHAR tmi_rate_map_cck_sp[]={
	TMI_TX_RATE_CCK_2M_SP,
	TMI_TX_RATE_CCK_2M_SP,
	TMI_TX_RATE_CCK_5M_SP,
	TMI_TX_RATE_CCK_11M_SP,
};

UCHAR tmi_rate_map_ofdm[]={
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
		 (((_nss -1) & (~TMI_TX_RATE_MASK_NSS)) << TMI_TX_RATE_BIT_NSS) | \
		 (TMI_TX_RATE_MODE_VHT << TMI_TX_RATE_BIT_MODE) | \
		 (_mcs))


USHORT tx_rate_to_tmi_rate(UCHAR mode, UCHAR mcs, UCHAR nss, BOOLEAN stbc, UCHAR preamble)
{
	USHORT tmi_rate = 0, mcs_id = 0;

	stbc = (stbc== TRUE) ? 1 : 0;
	switch (mode) {
		case MODE_CCK:
			if (preamble && mcs < ARRAY_SIZE(tmi_rate_map_cck_lp))
				mcs_id = tmi_rate_map_cck_lp[mcs];
			else if (mcs < ARRAY_SIZE(tmi_rate_map_cck_sp))
				mcs_id = tmi_rate_map_cck_sp[mcs];
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
					(((nss -1) & TMI_TX_RATE_MASK_NSS) << TMI_TX_RATE_BIT_NSS) |
					((USHORT)(mode << TMI_TX_RATE_BIT_MODE)) |
					((USHORT)(mcs));
//			DBGPRINT(RT_DEBUG_OFF, ("%s(): mode=%d, mcs=%d, stbc=%d converted tmi_rate=0x%x\n",
//						__FUNCTION__, mode, mcs, stbc, tmi_rate));
			break;
		case MODE_VHT:
			tmi_rate = TMI_TX_RATE_VHT_VAL(nss, mcs, stbc);
			break;
		default:
			DBGPRINT(RT_DEBUG_ERROR, ("%s():Invalid mode(mode=%d)\n",
						__FUNCTION__, mode));
			break;
	}

	return tmi_rate;
}


UCHAR get_nss_by_mcs(UCHAR phy_mode, UCHAR mcs, BOOLEAN stbc)
{
	UCHAR nss = 1;

	switch (phy_mode)
	{
		case MODE_HTMIX:
		case MODE_HTGREENFIELD:
			if (mcs != 32) {
				nss += (mcs >> 3);
				if ( stbc && nss == 1 )
					nss ++;
			}
			break;
		case MODE_CCK:
		case MODE_OFDM:
		default:
			break;
	}

	return nss;
}


/*
	========================================================================

	Routine Description:
		Calculates the duration which is required to transmit out frames
	with given size and specified rate.

	Arguments:
		pTxWI		Pointer to head of each MPDU to HW.
		Ack 		Setting for Ack requirement bit
		Fragment	Setting for Fragment bit
		RetryMode	Setting for retry mode
		Ifs 		Setting for IFS gap
		Rate		Setting for transmit rate
		Service 	Setting for service
		Length		Frame length
		TxPreamble	Short or Long preamble when using CCK rates
		QueIdx - 0-3, according to 802.11e/d4.4 June/2003

	Return Value:
		None

	See also : BASmartHardTransmit()    !!!

	========================================================================
*/
const UCHAR wmm_aci_2_hw_ac_queue[18] =
{
		Q_IDX_AC1, /* 0: QID_AC_BE */
		Q_IDX_AC0, /* 1: QID_AC_BK */
		Q_IDX_AC2, /* 2: QID_AC_VI */
		Q_IDX_AC3, /* 3:QID_AC_VO */
		4, /* 4:QID_AC_VO */
		5,
		6,
		7,
		8,
		9,
		10,
		11,
		12,
		13,
		14,
		15,
		16,
		17,
};
VOID write_tmac_info(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR *tmac_info,
	IN MAC_TX_INFO *info,
	IN HTTRANSMIT_SETTING *pTransmit)
{
	MAC_TABLE_ENTRY *mac_entry = NULL;
	UCHAR stbc, bw, mcs, nss = 1, sgi, phy_mode, ldpc = 0, preamble = LONG_PREAMBLE;
	UCHAR to_mcu = FALSE, q_idx = info->q_idx;
	TMAC_TXD_L txd;
	TMAC_TXD_0 *txd_0 = &txd.txd_0;
	TMAC_TXD_1 *txd_1 = &txd.txd_1;
	TMAC_TXD_2 *txd_2 = &txd.txd_2;
	TMAC_TXD_3 *txd_3 = &txd.txd_3;
	TMAC_TXD_5 *txd_5 = &txd.txd_5;
	TMAC_TXD_6 *txd_6 = &txd.txd_6;
	INT txd_size = sizeof(TMAC_TXD_S);
	TXS_CTL *TxSCtl = &pAd->TxSCtl;

	if (info->WCID < MAX_LEN_OF_MAC_TABLE)
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
		if (mac_entry)
		{
			if ((pAd->CommonCfg.bMIMOPSEnable) && (mac_entry->MmpsMode == MMPS_STATIC)
				&& (pTransmit->field.MODE >= MODE_HTMIX && pTransmit->field.MCS > 7))
				mcs = 7;
		}
	}
#endif /* DOT11_N_SUPPORT */

#ifdef DOT11K_RRM_SUPPORT
	if (pAd->CommonCfg.VoPwrConstraintTest == TRUE)
	{
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
	txd_0->p_idx = (to_mcu ? P_IDX_MCU : P_IDX_LMAC);
	if (q_idx < 4) {
		txd_0->q_idx = wmm_aci_2_hw_ac_queue[q_idx];
	} else
	txd_0->q_idx = q_idx;



	/* DWORD 1 */
	txd_1->wlan_idx = info->WCID;
	txd_1->ft = TMI_FT_LONG;
	txd_1->hdr_format = TMI_HDR_FT_NOR_80211;
	TMI_HDR_INFO_VAL(txd_1->hdr_format, 0, 0, 0, 0, 0, info->hdr_len, 0, txd_1->hdr_info);
	if (info->hdr_pad)  // TODO: depends on QoS to decide if need to padding
		txd_1->hdr_pad = (TMI_HDR_PAD_MODE_TAIL << TMI_HDR_PAD_BIT_MODE) | info->hdr_pad;
	txd_1->no_ack = (info->Ack ? 0 : 1);
	txd_1->tid = info->TID;

	txd_1->own_mac = 0 ;
	if (txd_0->q_idx == Q_IDX_BCN) {
		if (info->bss_idx == 0)
			txd_1->own_mac = 0 ;// info->bss_idx;
		else if (info->bss_idx > 0 && info->bss_idx <=15)
			txd_1->own_mac = 0x10 | info->bss_idx;

        //TODO:Carter, for Adhoc Beacon, use own_mac = 0 now,
        //but what if need to consider GO/client case, it shall be refined.
	}

	if (mac_entry && IS_ENTRY_APCLI(mac_entry))
	{
#ifdef MULTI_APCLI_SUPPORT
		txd_1->own_mac = (0x1 + mac_entry->func_tb_idx);
#else /* MULTI_APCLI_SUPPORT */
		txd_1->own_mac = 0x1; //Carter, refine this
#endif /* !MULTI_APCLI_SUPPORT */

	} else if (mac_entry && IS_ENTRY_CLIENT(mac_entry)) {
		/* handle HW_BSSID_0 & Extend from 0 to 15 */
		if (mac_entry->func_tb_idx == 0)
			txd_1->own_mac = 0x0;
		else if (mac_entry->func_tb_idx >= 1 && mac_entry->func_tb_idx <= 15)
			txd_1->own_mac = 0x10 | mac_entry->func_tb_idx;

		/* for concurrent to handle HW_BSSID_1/2/3 */
		if (mac_entry->wdev->hw_bssid_idx != 0)
            txd_1->own_mac = mac_entry->wdev->hw_bssid_idx ;
	}

	if (txd_1->ft == TMI_FT_LONG) {
		txd_size = sizeof(TMAC_TXD_L);

		/* DWORD 2 */
		txd_2->max_tx_time = 0;
		txd_2->bc_mc_pkt = info->BM;
		txd_2->fix_rate = 1;

		txd_2->duration = 0;
		txd_2->htc_vld = 0;
		txd_2->frag = info->FRAG; // 0: no frag, 1: 1st frag, 2: mid frag, 3: last frag
		txd_2->max_tx_time = 0;
		txd_2->ba_disable = 1;
		txd_2->timing_measure = 0;
		txd_2->fix_rate = 1;
        if (pAd->chipCap.TmrEnable == 1) {
            if ((txd_1->no_ack == 0) && (txd_2->bc_mc_pkt == 0)) {
                txd_2->timing_measure = 1;
            }
        }

		txd_2->sub_type = (*((UINT16 *)(tmac_info + sizeof(TMAC_TXD_L))) & (0xf << 4)) >> 4;
		txd_2->frm_type = (*((UINT16 *)(tmac_info + sizeof(TMAC_TXD_L))) & (0x3 << 2)) >> 2;

		/* DWORD 3 */
        if (txd_0->q_idx == Q_IDX_BCN)
            txd_3->remain_tx_cnt = MT_TX_RETRY_UNLIMIT;
        else if ((txd_2->frm_type == FC_TYPE_CNTL) && (txd_2->sub_type == SUBTYPE_BLOCK_ACK_REQ))
            txd_3->remain_tx_cnt = 1;			
        else {
		if (pAd->shortretry != 0)
			txd_3->remain_tx_cnt = pAd->shortretry;
        else
            txd_3->remain_tx_cnt = MT_TX_SHORT_RETRY;
        }

		/* DWORD 4 */

		/* DWORD 6 */
		if (txd_2->fix_rate == 1) {
			txd_6->fix_rate_mode = TMI_FIX_RATE_BY_TXD;
			//txd_6->ant_id = 0;
			txd_6->ant_pri = info->AntPri;
			txd_6->spe_en = info->SpeEn;
			txd_6->bw = ((1 << 2) |bw);
			txd_6->dyn_bw = 0;
			txd_6->ETxBF = 0;
			txd_6->ITxBF = 0;
			txd_6->ldpc = ldpc;
			txd_6->gi = sgi;

#ifdef GREENAP_SUPPORT
			if (pAd->ApCfg.bGreenAPActive == TRUE)
				txd_6->spe_en = 0;
#endif /* GREENAP_SUPPORT */

			if (txd_6->fix_rate_mode == TMI_FIX_RATE_BY_TXD)
			{
				if (phy_mode == MODE_CCK)
					preamble = info->Preamble;

				txd_6->tx_rate = tx_rate_to_tmi_rate(phy_mode, mcs, nss, stbc, preamble);
			}
		}

		/* DWORD 5 */
#ifdef WH_EZ_SETUP	
		if (0)//mac_entry && IS_EZ_SETUP_ENABLED(mac_entry->wdev) && info->PID == PID_EZ_ACTION)
		{
			EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Action Frame TXD initialized\n"));
		
			txd_5->pid = info->PID;
			txd_5->tx_status_fmt = TXS_FORMAT0;
			txd_5->tx_status_2_mcu = 0;
			txd_5->tx_status_2_host = 1;
		}
		else
#endif
		{
			txd_5->pid = 0;
			
			if (info->PID)
			{
				if (TxSCtl->TxSFormatPerPkt & (1 << info->PID))
					txd_5->tx_status_fmt = TXS_FORMAT1;
				else
					txd_5->tx_status_fmt = TXS_FORMAT0;
			
				if (TxSCtl->TxS2McUStatusPerPkt & (1 << info->PID))
					txd_5->tx_status_2_mcu = 1;
				else
				txd_5->tx_status_2_mcu = 0;
	
			if (TxSCtl->TxS2HostStatusPerPkt & (1 << info->PID))
			{
				txd_5->pid = AddTxSStatus(pAd, TXS_TYPE0, info->PID, 0, 0, 
												txd_6->tx_rate, info->TxSPriv);
				txd_5->tx_status_2_host = 1;
			}
			else
			{
				txd_5->tx_status_2_host = 0;
			}
		}
		else
		{
			ULONG TxSStatusPerWlanIdx;
			if (txd_1->wlan_idx < 64)
			{
				TxSStatusPerWlanIdx = TxSCtl->TxSStatusPerWlanIdx[0];
			}
			else
			{
				TxSStatusPerWlanIdx = TxSCtl->TxSStatusPerWlanIdx[1];
			}

			if (TxSStatusPerWlanIdx & (1 << txd_1->wlan_idx))
			{
			
				if (TxSCtl->TxSFormatPerPktType[txd_2->frm_type] & (1 << txd_2->sub_type))
					txd_5->tx_status_fmt = TXS_FORMAT1;
				else
					txd_5->tx_status_fmt = TXS_FORMAT0;
		
				if (TxSCtl->TxS2McUStatusPerPktType[txd_2->frm_type] & (1 << txd_2->sub_type))
					txd_5->tx_status_2_mcu = 1;
				else
					txd_5->tx_status_2_mcu = 0;
	
				if (TxSCtl->TxS2HostStatusPerPktType[txd_2->frm_type] & (1 << txd_2->sub_type))
				{
					txd_5->pid = AddTxSStatus(pAd, TXS_TYPE1, 0, 
								txd_2->frm_type, txd_2->sub_type, txd_6->tx_rate, info->TxSPriv); 
					txd_5->tx_status_2_host = 1;
				}
				else
				{
					txd_5->tx_status_2_host = 0;
				}
			}
		}
}
#ifdef HDR_TRANS_SUPPORT
		//txd_5->da_select = TMI_DAS_FROM_MPDU;
#endif /* HDR_TRANS_SUPPORT */
		txd_5->bar_sn_ctrl = 1;
		if (info->PsmBySw)
			txd_5->pwr_mgmt = TMI_PM_BIT_CFG_BY_SW;
		else
			txd_5->pwr_mgmt = TMI_PM_BIT_CFG_BY_HW;
		//txd_5->pn_high = 0;

		if (info->prot == 1) {
			txd_1->protect_frm = 1;
		}
		else if (info->prot == 2) {
#ifdef DOT11W_PMF_SUPPORT
			GET_PMF_GroupKey_WCID(pAd, info->WCID, info->bss_idx);
#else
			GET_GroupKey_WCID(pAd, info->WCID, info->bss_idx);
#endif /* DOT11W_PMF_SUPPORT */
			txd_1->wlan_idx = info->WCID;
		}
		else {
			txd_1->protect_frm = 0;
		}
	}

	txd_0->tx_byte_cnt = txd_size + info->Length; // TODO: shiang-7603, need to adjust
	//txwi_n->MPDUtotalByteCnt = Length;

	// TODO: shiang-7603, any mapping to following parameters?

	NdisMoveMemory(tmac_info, &txd, sizeof(TMAC_TXD_L));
#ifdef DBG
	if ((RTDebugFunc & DBG_FUNC_TMAC_INFO) == DBG_FUNC_TMAC_INFO) {
		dump_tmac_info(pAd, tmac_info);
	}
#endif /* DBG */
}


VOID write_tmac_info_Data(RTMP_ADAPTER *pAd, UCHAR *buf, TX_BLK *pTxBlk)
{
	MAC_TABLE_ENTRY *pMacEntry = pTxBlk->pMacEntry;
	UCHAR stbc = 0, bw = BW_20, mcs = 0, nss = 1, sgi = 0, phy_mode = 0, preamble = 1, ldpc = 0;
	UCHAR wcid, to_mcu = FALSE;
	TMAC_TXD_S *txd_s = (TMAC_TXD_S *)buf;
	TMAC_TXD_L *txd_l = (TMAC_TXD_L *)buf;
	TMAC_TXD_0 *txd_0 = &txd_s->txd_0;
	TMAC_TXD_1 *txd_1 = &txd_s->txd_1;
	UCHAR txd_size;
	BOOLEAN txd_long = FALSE;
	struct rtmp_mac_ctrl *wtbl_ctrl = &pAd->mac_ctrl;
	//STA_TR_ENTRY *tr_entry;
	struct wtbl_entry tb_entry;
	union WTBL_1_DW0 *dw0 = (union WTBL_1_DW0 *)&tb_entry.wtbl_1.wtbl_1_d0.word;
	TXS_CTL *TxSCtl = &pAd->TxSCtl;

		wcid = pTxBlk->Wcid;

	if (pMacEntry == NULL)
	{
		//printk("##### MacEntry is NULL ##### \n");
		txd_long = TRUE;
		//wcid = 0;	// TODO: wcid
	}
	else
	{
		//printk("##### MacEntry Aid is %d ##### \n", pMacEntry->Aid);
		wcid = pMacEntry->wcid;
		if ((pMacEntry->bAutoTxRateSwitch == FALSE) ||
			(TX_BLK_TEST_FLAG(pTxBlk, fTX_ForceRate)) ||
			(TX_BLK_TEST_FLAG(pTxBlk, fTX_bAllowFrag) && pTxBlk->FragIdx))
			txd_long = TRUE;
	}

	/* E1 workaround of DMAS bug, must use long format */
	txd_long = TRUE;


	if (txd_long == FALSE) {
        NdisZeroMemory(txd_s, sizeof(TMAC_TXD_S));
		pTxBlk->hw_rsv_len = 5 * sizeof(UINT32);
		txd_s = (TMAC_TXD_S *)(buf + pTxBlk->hw_rsv_len);
	} else {
        NdisZeroMemory(txd_l, sizeof(TMAC_TXD_L));
    }
	txd_0 = &txd_s->txd_0;
	txd_1 = &txd_s->txd_1;

	/* DWORD 0 */
	txd_0->p_idx = (to_mcu ? P_IDX_MCU : P_IDX_LMAC);
	if (pTxBlk->QueIdx < 4) {
		txd_0->q_idx = wmm_aci_2_hw_ac_queue[pTxBlk->QueIdx];
	} else {
		// TODO: shiang-usw, consider about MCC case!
		txd_0->q_idx = pTxBlk->QueIdx;
	}

	txd_1->wlan_idx = wcid;


	txd_1->hdr_format = TMI_HDR_FT_NOR_80211;
	TMI_HDR_INFO_VAL(txd_1->hdr_format, 0, 0, 0, 0, 0, pTxBlk->wifi_hdr_len, 0, txd_1->hdr_info);
	if (pTxBlk->HdrPadLen)  // TODO: depends on QoS to decide if need to padding
		txd_1->hdr_pad = (TMI_HDR_PAD_MODE_TAIL << TMI_HDR_PAD_BIT_MODE) | pTxBlk->HdrPadLen;

	txd_1->no_ack = (TX_BLK_TEST_FLAG(pTxBlk, fTX_bAckRequired) ? 0 : 1);
	txd_1->tid = pTxBlk->UserPriority;

	if (pTxBlk->CipherAlg != CIPHER_NONE)
		txd_1->protect_frm = 1;
	else
	txd_1->protect_frm = 0;
	if (pMacEntry && IS_ENTRY_APCLI(pMacEntry)) {
		//tr_entry = &pAd->MacTab.tr_entry[pMacEntry->wcid];
		tb_entry.wtbl_addr[0] = wtbl_ctrl->wtbl_base_addr[0] +
							pMacEntry->wcid * wtbl_ctrl->wtbl_entry_size[0];
		RTMP_IO_READ32(pAd, tb_entry.wtbl_addr[0], &tb_entry.wtbl_1.wtbl_1_d0.word);
		txd_1->own_mac = dw0->field.muar_idx; //Carter, refine this
		//printk("txd_1->own_mac = %x\n", txd_1->own_mac);
	}
	else if (pMacEntry && IS_ENTRY_CLIENT(pMacEntry)) {
		if (pMacEntry->func_tb_idx == 0)
			txd_1->own_mac = 0x0;
		else if (pMacEntry->func_tb_idx >= 1 && pMacEntry->func_tb_idx <= 15)
			txd_1->own_mac = 0x10 | pMacEntry->func_tb_idx;

		/* for concurrent to handle HW_BSSID_1/2/3 */
		if (pTxBlk->wdev->hw_bssid_idx != 0)
            txd_1->own_mac = pTxBlk->wdev->hw_bssid_idx ;

	}
	else if (pMacEntry && IS_ENTRY_WDS(pMacEntry)) {
			txd_1->own_mac = 0x0;
	}
	else {
#ifdef USE_BMC
		if (pTxBlk->wdev_idx == 0)
			txd_1->own_mac = 0x0;
		else if (pTxBlk->wdev_idx >= 1 && pTxBlk->wdev_idx <= 15)
			txd_1->own_mac = 0x10 | pTxBlk->wdev_idx;
#else
		txd_1->own_mac = 0; // TODO: revise this
#endif /* USE_BMC */

		if (pTxBlk->wdev->hw_bssid_idx != 0)
        	txd_1->own_mac = pTxBlk->wdev->hw_bssid_idx ;
	}

	if (txd_long == FALSE)
	{
		txd_1->ft = TMI_FT_SHORT;
		txd_size = sizeof(TMAC_TXD_S);
	} else {
		TMAC_TXD_2 *txd_2 = &txd_l->txd_2;
		//TMAC_TXD_3 *txd_3 = &txd_l->txd_3;
		TMAC_TXD_5 *txd_5 = &txd_l->txd_5;
		TMAC_TXD_6 *txd_6 = &txd_l->txd_6;
		HTTRANSMIT_SETTING *pTransmit = pTxBlk->pTransmit;

		txd_1->ft = TMI_FT_LONG;
		txd_size = sizeof(TMAC_TXD_L);
		if (pTransmit) {
			ldpc = pTransmit->field.ldpc;
			mcs = pTransmit->field.MCS;
			sgi = pTransmit->field.ShortGI;
			stbc = pTransmit->field.STBC;
			phy_mode = pTransmit->field.MODE;
			bw = (phy_mode <= MODE_OFDM) ? (BW_20) : (pTransmit->field.BW);
			nss = get_nss_by_mcs(phy_mode, mcs, stbc);
		}

		txd_l->txd_2.max_tx_time = 0;
		txd_l->txd_2.bc_mc_pkt = (pTxBlk->TxFrameType == TX_MCAST_FRAME ? 1 : 0);

		if (TX_BLK_TEST_FLAG(pTxBlk, fTX_ForceRate))
			txd_l->txd_2.fix_rate = 1;

		txd_l->txd_2.frag = pTxBlk->FragIdx;

        if (pAd->chipCap.TmrEnable == 1) {
            if ((txd_1->no_ack == 0) /*&& (txd_l->txd_2.bc_mc_pkt == 0)*/) {
                //printk("TMR trigger packet.\n");
                txd_l->txd_2.timing_measure = 1;
            }
        }

		txd_2->sub_type = (*((UINT16 *)(pTxBlk->wifi_hdr)) & (0xf << 4)) >> 4;
		txd_2->frm_type = (*((UINT16 *)(pTxBlk->wifi_hdr)) & (0x3 << 2)) >> 2;
		
		/*start: fix ping rate*/
		if ((pMacEntry) && (RTMP_GET_PACKET_ICMP(pTxBlk->pPacket)) && PingFixRate)
        	{
			/*solve ping tx hang when HT40, by dragon, 16.01.29 */
			//txd_l->txd_2.fix_rate = 1;
			/*if((pMacEntry->RssiSample.AvgRssi[0] >= -60) || (pMacEntry->RssiSample.AvgRssi[1] >= -60))
			{
				mcs = 6;
				phy_mode = MODE_OFDM;
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN | DBG_FUNC_UAPSD, ("write_tmac_info_Data,%-3d/%-3d,phy_mode=%d,mcs=%d\n", pMacEntry->RssiSample.AvgRssi[0],pMacEntry->RssiSample.AvgRssi[1],phy_mode,mcs));
			}
			else*/
			{
				mcs = 2;
				phy_mode = MODE_CCK;
			}
		}
		/*end: fix ping rate*/

		if (pMacEntry)
		{
			if (pAd->shortretry != 0)
				txd_l->txd_3.remain_tx_cnt = pAd->shortretry;
			else
				txd_l->txd_3.remain_tx_cnt = pMacEntry->ucMaxTxRetryCnt;
		} else
			txd_l->txd_3.remain_tx_cnt = MT_TX_SHORT_RETRY;

		if ((pTxBlk->TxFrameType == TX_LEGACY_FRAME) 
			|| (pTxBlk->TxFrameType == TX_RALINK_FRAME))
			txd_2->ba_disable = 1; 

#ifdef MT7603
		if (MTK_REV_GTE(pAd, MT7603, MT7603E1))
		{
	        if (txd_0->q_idx == QID_BMC)
    	        txd_l->txd_3.remain_tx_cnt = MT_TX_RETRY_UNLIMIT;
        }
#endif /* MT7603 */
		
		/* DWORD 6 */
		if (txd_2->fix_rate == 1) {
			txd_6->fix_rate_mode = TMI_FIX_RATE_BY_TXD;
			txd_6->bw = ((1 << 2) |bw);
			txd_6->dyn_bw = 0;
			txd_6->ETxBF = 0;
			txd_6->ITxBF = 0;
			txd_6->ldpc = ldpc;
			txd_6->gi = sgi;

			if (txd_6->fix_rate_mode == TMI_FIX_RATE_BY_TXD)
			{
				if (phy_mode == MODE_CCK)
				{
					if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED))
						preamble = SHORT_PREAMBLE;
					else
						preamble = LONG_PREAMBLE;
				}

				txd_6->tx_rate = tx_rate_to_tmi_rate(phy_mode, mcs, nss, stbc, preamble);

			}
		}

		txd_5->pid = 0;
		
		if (pTxBlk->Pid)
		{
			if (TxSCtl->TxSFormatPerPkt & (1 << pTxBlk->Pid))
				txd_5->tx_status_fmt = TXS_FORMAT1;
			else
				txd_5->tx_status_fmt = TXS_FORMAT0;
		
			if (TxSCtl->TxS2McUStatusPerPkt & (1 << pTxBlk->Pid))
				txd_5->tx_status_2_mcu = 1;
			else
				txd_5->tx_status_2_mcu = 0;
	
			if (TxSCtl->TxS2HostStatusPerPkt & (1 << pTxBlk->Pid))
			{
				txd_5->pid = AddTxSStatus(pAd, TXS_TYPE0, pTxBlk->Pid, 0, 0, 
									txd_6->tx_rate, pTxBlk->TxSPriv);
				txd_5->tx_status_2_host = 1;
			}
			else
			{
				txd_5->tx_status_2_host = 0;
			}
		}
		else
		{
			ULONG TxSStatusPerWlanIdx;
			if (txd_1->wlan_idx < 64)
			{
				TxSStatusPerWlanIdx = TxSCtl->TxSStatusPerWlanIdx[0];
			}
			else
			{
				TxSStatusPerWlanIdx = TxSCtl->TxSStatusPerWlanIdx[1];
			}

			if (TxSStatusPerWlanIdx & (1 << txd_1->wlan_idx))
			{
				if (TxSCtl->TxSFormatPerPktType[txd_2->frm_type] & (1 << txd_2->sub_type))
					txd_5->tx_status_fmt = TXS_FORMAT1;
				else
					txd_5->tx_status_fmt = TXS_FORMAT0;
		
				if (TxSCtl->TxS2McUStatusPerPktType[txd_2->frm_type] & (1 << txd_2->sub_type))
					txd_5->tx_status_2_mcu = 1;
				else
					txd_5->tx_status_2_mcu = 0;
	
				if (TxSCtl->TxS2HostStatusPerPktType[txd_2->frm_type] & (1 << txd_2->sub_type))
				{
					txd_5->pid = AddTxSStatus(pAd, TXS_TYPE1, 0, 
							txd_2->frm_type, txd_2->sub_type, txd_6->tx_rate, pTxBlk->TxSPriv); 
					txd_5->tx_status_2_host = 1;
				}
				else
				{
					txd_5->tx_status_2_host = 0;
				}
			}
		}

#ifdef HDR_TRANS_SUPPORT
		//txd_5->da_select = TMI_DAS_FROM_MPDU;
#endif /* HDR_TRANS_SUPPORT */
		txd_5->bar_sn_ctrl = TMI_PM_BIT_CFG_BY_SW;

		/* For  MT STA LP control, use H/W control mode for PM bit */
 		txd_5->pwr_mgmt = TMI_PM_BIT_CFG_BY_SW;
	}

	txd_0->tx_byte_cnt = txd_size +
						pTxBlk->MpduHeaderLen +
						pTxBlk->HdrPadLen +
						pTxBlk->SrcBufLen; // TODO: shiang-7603, need to adjust

#ifdef DBG_DIAGNOSE
	if (pTxBlk->QueIdx== 0)
	{
		HTTRANSMIT_SETTING *pTransmit = pTxBlk->pTransmit;
		UCHAR mcs = pTransmit->field.MCS;

		pAd->DiagStruct.diag_info[pAd->DiagStruct.ArrayCurIdx].TxDataCnt++;
#ifdef DBG_TX_MCS
		if (pTransmit->field.MODE == MODE_HTMIX || pTransmit->field.MODE == MODE_HTGREENFIELD) {
			if (mcs < MAX_MCS_SET)
				pAd->DiagStruct.diag_info[pAd->DiagStruct.ArrayCurIdx].TxMcsCnt_HT[mcs]++;
		}
#ifdef DOT11_VHT_AC
		else if (pTransmit->field.MODE == MODE_VHT) {
			INT mcs_idx = ((mcs >> 4) * 10) +  (mcs & 0xf);
			if (mcs_idx < MAX_VHT_MCS_SET)
				pAd->DiagStruct.diag_info[pAd->DiagStruct.ArrayCurIdx].TxMcsCnt_VHT[mcs_idx]++;
		}
#endif /* DOT11_VHT_AC */
#endif /* DBG_TX_MCS */
	}
#endif /* DBG_DIAGNOSE */
}


VOID write_tmac_info_Cache(RTMP_ADAPTER *pAd, UCHAR *buf, TX_BLK *pTxBlk)
{
	// TODO: shiang-7603, for now, go following function first;
	write_tmac_info_Data(pAd, buf, pTxBlk);
	DBGPRINT(RT_DEBUG_OFF, ("%s(%d): Not Finish yet for HT_MT!!!!\n", __FUNCTION__, __LINE__));
	return;
}


INT rtmp_mac_set_band(RTMP_ADAPTER *pAd, int  band)
{
	// TODO: shiang-7603
	if (pAd->chipCap.hif_type == HIF_MT) {
		DBGPRINT(RT_DEBUG_OFF, ("%s(%d): Not support for HIF_MT yet!\n",
							__FUNCTION__, __LINE__));
		return FALSE;
	}

	return TRUE;
}


INT mt_mac_set_ctrlch(RTMP_ADAPTER *pAd, UINT8 extch)
{
	// TODO: shiang-7603
	if (pAd->chipCap.hif_type == HIF_MT)
		return FALSE;

	return TRUE;
}


INT rtmp_mac_set_mmps(RTMP_ADAPTER *pAd, INT ReduceCorePower)
{
	// TODO: shiang-7603
	if (pAd->chipCap.hif_type == HIF_MT) {
		DBGPRINT(RT_DEBUG_OFF, ("%s(%d): Not support for HIF_MT yet!\n",
							__FUNCTION__, __LINE__));
		return FALSE;
	}
	return TRUE;
}


#define BCN_TBTT_OFFSET		64	/*defer 64 us*/
VOID ReSyncBeaconTime(RTMP_ADAPTER *pAd)
{
	// TODO: shiang-7603
	if (pAd->chipCap.hif_type == HIF_MT) {
		DBGPRINT(RT_DEBUG_OFF, ("%s(%d): Not support for HIF_MT yet!\n",
							__FUNCTION__, __LINE__));
	}
}


#ifdef RTMP_MAC_PCI
VOID mt_asic_init_txrx_ring(RTMP_ADAPTER *pAd)
{
	DELAY_INT_CFG_STRUC IntCfg;
	UINT32 phy_addr, offset;
	INT i;

	/* Reset DMA Index */
	RTMP_IO_WRITE32(pAd, WPDMA_RST_PTR, 0xFFFFFFFF);

	/*
		Write Tx Ring base address registers

		The Tx Ring arrangement:
		RingIdx	SwRingIdx	AsicPriority	WMM QID		LMAC QID	MCU QID
		0 		Grp1_TxSw0		L		QID_AC_BK		AC0
		1		Grp1_TxSw1		L		QID_AC_BE		AC1
		2		Grp1_TxSw2		L		QID_AC_VI		AC2
		3		Grp1_TxSw3		L		QID_AC_VO		AC3
		4		Grp1_MGMT		H			-			AC4

		5		CTRL						-			-
		6		PSMP						-			-
		7		BC/MC									BC/MC
		8		BCN				H						BCN

		9		Grp2_MGMT					-			AC14
		10		Grp2_TxSw0		L		QID_AC_BK		AC10
		11		Grp2_TxSw0		L		QID_AC_BE		AC11
		12		Grp2_TxSw0		L		QID_AC_VI		AC12
		13		Grp2_TxSw0		L		QID_AC_VO		AC13
		14		-
		15		MCU				H			-			MCU				0

		Ring 0~3 for TxChannel 0
		Ring 10~14 for TxChannel 1
	*/
	for (i = 0; i < NUM_OF_TX_RING; i++) {
		if (i == QID_AC_BE)
			offset = 0x10;
		else if (i == QID_AC_BK)
			offset = 0;
		else
			offset = i * 0x10;
		phy_addr = RTMP_GetPhysicalAddressLow(pAd->TxRing[i].Cell[0].AllocPa);
		pAd->TxRing[i].TxSwFreeIdx = 0;
		pAd->TxRing[i].TxCpuIdx = 0;

		pAd->TxRing[i].hw_desc_base = MT_TX_RING_BASE + offset;
		pAd->TxRing[i].hw_cidx_addr = MT_TX_RING_CIDX + offset;
		pAd->TxRing[i].hw_didx_addr = MT_TX_RING_DIDX + offset;
		pAd->TxRing[i].hw_cnt_addr = MT_TX_RING_CNT + offset;
		RTMP_IO_WRITE32(pAd, pAd->TxRing[i].hw_desc_base, phy_addr);
		RTMP_IO_WRITE32(pAd, pAd->TxRing[i].hw_cidx_addr, pAd->TxRing[i].TxCpuIdx);
		RTMP_IO_WRITE32(pAd, pAd->TxRing[i].hw_cnt_addr, TX_RING_SIZE);
		DBGPRINT(RT_DEBUG_TRACE, ("-->TX_RING_%d[0x%x]: Base=0x%x, Cnt=%d!\n",
					i, pAd->TxRing[i].hw_desc_base, phy_addr, TX_RING_SIZE));
	}

	/* init BMC ring */
	offset = QID_BMC * 0x10;
	phy_addr = RTMP_GetPhysicalAddressLow(pAd->TxBmcRing.Cell[0].AllocPa);
	pAd->TxBmcRing.TxSwFreeIdx = 0;
	pAd->TxBmcRing.TxCpuIdx = 0;
	pAd->TxBmcRing.hw_desc_base = MT_TX_RING_BASE + offset;
	pAd->TxBmcRing.hw_cidx_addr = MT_TX_RING_CIDX + offset;
	pAd->TxBmcRing.hw_didx_addr = MT_TX_RING_DIDX + offset;
	pAd->TxBmcRing.hw_cnt_addr = MT_TX_RING_CNT + offset;
	RTMP_IO_WRITE32(pAd, pAd->TxBmcRing.hw_desc_base, phy_addr);
	RTMP_IO_WRITE32(pAd, pAd->TxBmcRing.hw_cidx_addr, pAd->TxBmcRing.TxCpuIdx);
	RTMP_IO_WRITE32(pAd, pAd->TxBmcRing.hw_cnt_addr, TX_RING_SIZE);
	DBGPRINT(RT_DEBUG_TRACE, ("-->TX_BMC_RING [0x%x]: Base=0x%x, Cnt=%d!\n",
				pAd->TxBmcRing.hw_desc_base, phy_addr, TX_RING_SIZE));

	/* init MGMT ring Base/Size/Index pointer CSR */
	phy_addr = RTMP_GetPhysicalAddressLow(pAd->MgmtRing.Cell[0].AllocPa);
	pAd->MgmtRing.TxSwFreeIdx = 0;
	pAd->MgmtRing.TxCpuIdx = 0;

	pAd->MgmtRing.hw_desc_base = (MT_TX_RING_BASE  + MT_RINGREG_DIFF * 4);
	pAd->MgmtRing.hw_cnt_addr = (pAd->MgmtRing.hw_desc_base + 0x04);
	pAd->MgmtRing.hw_cidx_addr = (pAd->MgmtRing.hw_desc_base + 0x08);
	pAd->MgmtRing.hw_didx_addr = (pAd->MgmtRing.hw_desc_base + 0x0c);
	RTMP_IO_WRITE32(pAd, pAd->MgmtRing.hw_desc_base, phy_addr);
	RTMP_IO_WRITE32(pAd, pAd->MgmtRing.hw_cidx_addr, pAd->MgmtRing.TxCpuIdx);
	RTMP_IO_WRITE32(pAd, pAd->MgmtRing.hw_cnt_addr, MGMT_RING_SIZE);
	DBGPRINT(RT_DEBUG_TRACE, ("-->TX_RING_MGMT[0x%x]: Base=0x%x, Cnt=%d!\n",
					pAd->MgmtRing.hw_desc_base, phy_addr, MGMT_RING_SIZE));

#ifdef CONFIG_ANDES_SUPPORT
	/* init CTRL ring index pointer */
	phy_addr = RTMP_GetPhysicalAddressLow(pAd->CtrlRing.Cell[0].AllocPa);
	pAd->CtrlRing.TxSwFreeIdx = 0;
	pAd->CtrlRing.TxCpuIdx = 0;

	pAd->CtrlRing.hw_desc_base = (MT_TX_RING_BASE  + MT_RINGREG_DIFF * 5);
	pAd->CtrlRing.hw_cnt_addr = (pAd->CtrlRing.hw_desc_base + 0x04);
	pAd->CtrlRing.hw_cidx_addr = (pAd->CtrlRing.hw_desc_base + 0x08);
	pAd->CtrlRing.hw_didx_addr = (pAd->CtrlRing.hw_desc_base + 0x0c);

	RTMP_IO_WRITE32(pAd, pAd->CtrlRing.hw_desc_base, phy_addr);
	RTMP_IO_WRITE32(pAd, pAd->CtrlRing.hw_cidx_addr,  pAd->CtrlRing.TxCpuIdx);
	RTMP_IO_WRITE32(pAd, pAd->CtrlRing.hw_cnt_addr, MGMT_RING_SIZE);
	DBGPRINT(RT_DEBUG_TRACE, ("-->TX_RING_CTRL: Base=0x%x, Cnt=%d!\n",
					phy_addr, MGMT_RING_SIZE));
#endif /* CONFIG_ANDES_SUPPORT */

	/* init BCN ring index pointer */
	phy_addr = RTMP_GetPhysicalAddressLow(pAd->BcnRing.Cell[0].AllocPa);
	pAd->BcnRing.TxSwFreeIdx = 0;
	pAd->BcnRing.TxCpuIdx = 0;

	pAd->BcnRing.hw_desc_base = (MT_TX_RING_BASE  + MT_RINGREG_DIFF * MT_TX_RING_BCN_IDX);
	pAd->BcnRing.hw_cnt_addr = (pAd->BcnRing.hw_desc_base + 0x4);
	pAd->BcnRing.hw_cidx_addr = (pAd->BcnRing.hw_desc_base + 0x8);
	pAd->BcnRing.hw_didx_addr = (pAd->BcnRing.hw_desc_base + 0xc);
	RTMP_IO_WRITE32(pAd, pAd->BcnRing.hw_desc_base, phy_addr);
	RTMP_IO_WRITE32(pAd, pAd->BcnRing.hw_cidx_addr,  pAd->BcnRing.TxCpuIdx);
	RTMP_IO_WRITE32(pAd, pAd->BcnRing.hw_cnt_addr, BCN_RING_SIZE);
	DBGPRINT(RT_DEBUG_TRACE, ("-->TX_RING_BCN: Base=0x%x, Cnt=%d!\n",
					phy_addr, BCN_RING_SIZE));

	/* Init RX Ring0 Base/Size/Index pointer CSR */
	for (i = 0; i < NUM_OF_RX_RING; i++) {
		offset = i * 0x10;
		phy_addr = RTMP_GetPhysicalAddressLow(pAd->RxRing[i].Cell[0].AllocPa);
		pAd->RxRing[i].RxSwReadIdx = 0;
		pAd->RxRing[i].RxCpuIdx = RX_RING_SIZE - 1;
		pAd->RxRing[i].hw_desc_base = MT_RX_RING_BASE + offset;
		pAd->RxRing[i].hw_cidx_addr = MT_RX_RING_CIDX + offset;
		pAd->RxRing[i].hw_didx_addr = MT_RX_RING_DIDX + offset;
		pAd->RxRing[i].hw_cnt_addr = MT_RX_RING_CNT + offset;
		RTMP_IO_WRITE32(pAd, pAd->RxRing[i].hw_desc_base, phy_addr);
		RTMP_IO_WRITE32(pAd, pAd->RxRing[i].hw_cidx_addr, pAd->RxRing[i].RxCpuIdx);
		RTMP_IO_WRITE32(pAd, pAd->RxRing[i].hw_cnt_addr, RX_RING_SIZE);
		DBGPRINT(RT_DEBUG_TRACE, ("-->RX_RING%d[0x%x]: Base=0x%x, Cnt=%d\n",
					i, pAd->RxRing[i].hw_desc_base, phy_addr, RX_RING_SIZE));
	}

	/* Set DMA global configuration except TX_DMA_EN and RX_DMA_EN bits */
	AsicWaitPDMAIdle(pAd, 100, 1000);
	AsicSetWPDMA(pAd, PDMA_TX_RX, FALSE);

	IntCfg.word = 0;
	RTMP_IO_WRITE32(pAd, MT_DELAY_INT_CFG, IntCfg.word);
}
#endif /* RTMP_MAC_PCI */


VOID mt_mac_bcn_buf_init(IN RTMP_ADAPTER *pAd, BOOLEAN bHardReset)
{
	int idx;
	RTMP_CHIP_CAP *pChipCap = &pAd->chipCap;


	for (idx = 0; idx < pChipCap->BcnMaxHwNum; idx++)
		pAd->BeaconOffset[idx] = pChipCap->BcnBase[idx];

	DBGPRINT(RT_DEBUG_TRACE, ("< Beacon Information: >\n"));
	DBGPRINT(RT_DEBUG_TRACE, ("\tFlgIsSupSpecBcnBuf = %s\n", pChipCap->FlgIsSupSpecBcnBuf ? "TRUE" : "FALSE"));
	DBGPRINT(RT_DEBUG_TRACE, ("\tBcnMaxHwNum = %d\n", pChipCap->BcnMaxHwNum));
	DBGPRINT(RT_DEBUG_TRACE, ("\tBcnMaxNum = %d\n", pChipCap->BcnMaxNum));
	DBGPRINT(RT_DEBUG_TRACE, ("\tBcnMaxHwSize = 0x%x\n", pChipCap->BcnMaxHwSize));
	DBGPRINT(RT_DEBUG_TRACE, ("\tWcidHwRsvNum = %d\n", pChipCap->WcidHwRsvNum));
	for (idx = 0; idx < pChipCap->BcnMaxHwNum; idx++) {
		DBGPRINT(RT_DEBUG_TRACE, ("\t\tBcnBase[%d] = 0x%x, pAd->BeaconOffset[%d]=0x%x\n",
					idx, pChipCap->BcnBase[idx], idx, pAd->BeaconOffset[idx]));
	}

	// TODO: shiang-7603
	if (pAd->chipCap.hif_type == HIF_MT) {
		DBGPRINT(RT_DEBUG_OFF, ("%s(%d): Not support for HIF_MT yet!\n",
							__FUNCTION__, __LINE__));
		return;
	}

	/* It isn't necessary to clear this space when not hard reset. */
	if (bHardReset == TRUE)
	{
		/* clear all on-chip BEACON frame space */
#ifdef CONFIG_AP_SUPPORT
		INT	apidx;
		for (apidx = 0; apidx < HW_BEACON_MAX_COUNT(pAd); apidx++)
		{
			if (pAd->BeaconOffset[apidx] > 0) {
			}
		}
#endif /* CONFIG_AP_SUPPORT */
	}

}


INT mt_mac_pse_init(RTMP_ADAPTER *pAd)
{
	// TODO: shiang-7603

	/* do PCI-E remap for physical address 0xa5000000 to 0x40000 */
	RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_1, MT_PSE_WTBL_2_ADDR);

// TODO: shaing, for MT7628, may need to change this as RTMP_MAC_PCI
#ifdef RTMP_PCI_SUPPORT
	RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, 0x80080000);
#endif /* RTMP_PCI_SUPPORT */

	return TRUE;
}


#ifdef DBG
VOID dump_wtbl_1_info(RTMP_ADAPTER *pAd, struct wtbl_1_struc *tb)
{
	union WTBL_1_DW0 *wtbl_1_d0 = (union WTBL_1_DW0 *)&tb->wtbl_1_d0.word;
	union WTBL_1_DW1 *wtbl_1_d1 = (union WTBL_1_DW1 *)&tb->wtbl_1_d1.word;
	union WTBL_1_DW2 *wtbl_1_d2 = (union WTBL_1_DW2 *)&tb->wtbl_1_d2.word;
	union WTBL_1_DW3 *wtbl_1_d3 = (union WTBL_1_DW3 *)&tb->wtbl_1_d3.word;
	union WTBL_1_DW4 *wtbl_1_d4 = (union WTBL_1_DW4 *)&tb->wtbl_1_d4.word;
	UCHAR addr[6];

	NdisMoveMemory(&addr[0], &wtbl_1_d1->word, 4);
	addr[0] = wtbl_1_d1->field.addr_0 & 0xff;
	addr[1] = ((wtbl_1_d1->field.addr_0 & 0xff00) >> 8);
	addr[2] = ((wtbl_1_d1->field.addr_0 & 0xff0000) >> 16);
	addr[3] = ((wtbl_1_d1->field.addr_0 & 0xff000000) >> 24);
	//addr[4] = wtbl_1_d0->field.addr_32 & 0xff;
	//addr[5] = (wtbl_1_d0->field.addr_32 & 0xff00 >> 8);
	addr[4] = wtbl_1_d0->field.addr_4 & 0xff;
	addr[5] = wtbl_1_d0->field.addr_5 & 0xff;
	hex_dump("WTBL Segment 1 Raw Data", (UCHAR *)tb, sizeof(struct wtbl_1_struc));

	DBGPRINT(RT_DEBUG_OFF, ("WTBL Segment 1 Fields:\n"));
	DBGPRINT(RT_DEBUG_OFF, ("\tAddr: %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(addr)));
	DBGPRINT(RT_DEBUG_OFF, ("\tMUAR_Idx:%d\n", wtbl_1_d0->field.muar_idx));
	DBGPRINT(RT_DEBUG_OFF, ("\trc_a1/rc_a2:%d/%d\n",
							wtbl_1_d0->field.rc_a1, wtbl_1_d0->field.rc_a2));
	DBGPRINT(RT_DEBUG_OFF, ("\tkid:%d\n", wtbl_1_d0->field.kid));
	DBGPRINT(RT_DEBUG_OFF, ("\trkv/rv:%d/%d\n", wtbl_1_d0->field.rkv, wtbl_1_d0->field.rv));
	DBGPRINT(RT_DEBUG_OFF, ("\tsw:%d\n", wtbl_1_d0->field.sw));
	DBGPRINT(RT_DEBUG_OFF, ("\twm/mm:%d/%d\n", wtbl_1_d0->field.wm, wtbl_1_d2->field.mm));

	DBGPRINT(RT_DEBUG_OFF, ("\tcipher_suit:%d\n", wtbl_1_d2->field.cipher_suit));
	DBGPRINT(RT_DEBUG_OFF, ("\ttd/fd:%d/%d\n", wtbl_1_d2->field.td, wtbl_1_d2->field.fd));
	DBGPRINT(RT_DEBUG_OFF, ("\tdis_rhtr:%d\n", wtbl_1_d2->field.dis_rhtr));
	DBGPRINT(RT_DEBUG_OFF, ("\taf:%d\n", wtbl_1_d2->field.af));
	DBGPRINT(RT_DEBUG_OFF, ("\ttx_ps:%d\n", wtbl_1_d2->field.tx_ps));
	DBGPRINT(RT_DEBUG_OFF, ("\tr:%d\n", wtbl_1_d2->field.r));
	DBGPRINT(RT_DEBUG_OFF, ("\trts:%d\n", wtbl_1_d2->field.rts));
	DBGPRINT(RT_DEBUG_OFF, ("\tcf_ack:%d\n", wtbl_1_d2->field.cf_ack));
	DBGPRINT(RT_DEBUG_OFF, ("\trdg_ba:%d\n", wtbl_1_d2->field.rdg_ba));
	DBGPRINT(RT_DEBUG_OFF, ("\tsmps:%d\n", wtbl_1_d2->field.smps));
	DBGPRINT(RT_DEBUG_OFF, ("\tbaf_en:%d\n", wtbl_1_d2->field.baf_en));
	DBGPRINT(RT_DEBUG_OFF, ("\tht/vht/ldpc/dyn_bw:%d/%d/%d/%d\n",
				wtbl_1_d2->field.ht, wtbl_1_d2->field.vht,
				wtbl_1_d2->field.ldpc, wtbl_1_d2->field.dyn_bw));
	DBGPRINT(RT_DEBUG_OFF, ("\tTxBF(tibf/tebf):%d / %d\n", wtbl_1_d2->field.tibf, wtbl_1_d2->field.tebf));
	DBGPRINT(RT_DEBUG_OFF, ("\ttxop_ps_cap:%d\n", wtbl_1_d2->field.txop_ps_cap));
	DBGPRINT(RT_DEBUG_OFF, ("\tmesh:%d\n", wtbl_1_d2->field.mesh));
	DBGPRINT(RT_DEBUG_OFF, ("\tqos:%d\n", wtbl_1_d2->field.qos));

	DBGPRINT(RT_DEBUG_OFF, ("\tadm:%d\n", wtbl_1_d2->field.adm));
	DBGPRINT(RT_DEBUG_OFF, ("\tgid:%d\n", wtbl_1_d2->field.gid));

	DBGPRINT(RT_DEBUG_OFF, ("\twtbl2_fid:%d\n", wtbl_1_d3->field.wtbl2_fid));
	DBGPRINT(RT_DEBUG_OFF, ("\twtbl2_eid:%d\n", wtbl_1_d3->field.wtbl2_eid));

	DBGPRINT(RT_DEBUG_OFF, ("\twtbl3_fid:%d\n", wtbl_1_d4->field.wtbl3_fid));
	DBGPRINT(RT_DEBUG_OFF, ("\twtbl3_eid:%d\n", wtbl_1_d4->field.wtbl3_eid));

	DBGPRINT(RT_DEBUG_OFF, ("\twtbl4_fid:%d\n", wtbl_1_d3->field.wtbl4_fid));
	DBGPRINT(RT_DEBUG_OFF, ("\twtbl4_eid:%d\n", wtbl_1_d4->field.wtbl4_eid));

	DBGPRINT(RT_DEBUG_OFF, ("\tchk_per:%d\n", wtbl_1_d3->field.chk_per));
	DBGPRINT(RT_DEBUG_OFF, ("\tdu_i_psm:%d\n", wtbl_1_d3->field.du_i_psm));
        DBGPRINT(RT_DEBUG_OFF, ("\ti_psm:%d\n", wtbl_1_d3->field.i_psm));
	DBGPRINT(RT_DEBUG_OFF, ("\tpsm:%d\n", wtbl_1_d3->field.psm));
	DBGPRINT(RT_DEBUG_OFF, ("\tskip_tx:%d\n", wtbl_1_d3->field.skip_tx));

	DBGPRINT(RT_DEBUG_OFF, ("\tpartial_aid:%d\n", wtbl_1_d4->field.partial_aid));
}


static UCHAR ba_range[] = {4, 5, 8, 10, 16, 20, 21, 42};
static UCHAR *bw_str[] = {"20", "40", "80", "160"};
VOID dump_wtbl_2_info(RTMP_ADAPTER *pAd, struct wtbl_2_struc *tb)
{
	union WTBL_2_DW0 *dw_0 = &tb->wtbl_2_d0;
	union WTBL_2_DW1 *dw_1 = &tb->wtbl_2_d1;
	union WTBL_2_DW2 *dw_2 = &tb->wtbl_2_d2;
	union WTBL_2_DW3 *dw_3 = &tb->wtbl_2_d3;
	union WTBL_2_DW4 *dw_4 = &tb->wtbl_2_d4;
	union WTBL_2_DW5 *dw_5 = &tb->wtbl_2_d5;
	union WTBL_2_DW6 *dw_6 = &tb->wtbl_2_d6;
	union WTBL_2_DW7 *dw_7 = &tb->wtbl_2_d7;
	union WTBL_2_DW8 *dw_8 = &tb->wtbl_2_d8;
	union WTBL_2_DW9 *dw_9 = &tb->wtbl_2_d9;
	union WTBL_2_DW10 *dw_10 = &tb->wtbl_2_d10;
	union WTBL_2_DW11 *dw_11 = &tb->wtbl_2_d11;
	union WTBL_2_DW12 *dw_12 = &tb->wtbl_2_d12;
	union WTBL_2_DW13 *dw_13 = &tb->wtbl_2_d13;
	union WTBL_2_DW14 *dw_14 = &tb->wtbl_2_d14;
	union WTBL_2_DW15 *dw_15 = &tb->wtbl_2_d15;
	UINT32 idx, ba_size_idx, ba_size = 0;
	BOOLEAN ba_en;
	UINT32 rate_info[8];

	hex_dump("WTBL Segment 2 Raw Data", (UCHAR *)tb, sizeof(struct wtbl_2_struc));
	DBGPRINT(RT_DEBUG_OFF, ("WTBL Segment 2 Fields:\n"));
	DBGPRINT(RT_DEBUG_OFF, ("\tPN_0-31:0x%x\n", dw_0->pn_0));
	DBGPRINT(RT_DEBUG_OFF, ("\tPN_32-48:0x%x\n", dw_1->field.pn_32));
	DBGPRINT(RT_DEBUG_OFF, ("\tSN(NonQos/Mgmt Frame):%d\n", dw_1->field.com_sn));
	DBGPRINT(RT_DEBUG_OFF, ("\tSN(TID0~7 QoS Frame):%d - %d - %d - %d - %d - %d - %d - %d\n",
				dw_2->field.tid_ac_0_sn, dw_2->field.tid_ac_1_sn,
				dw_2->field.tid_ac_2_sn_0 | (dw_3->field.tid_ac_2_sn_9 << 9),
				dw_3->field.tid_ac_3_sn, dw_3->field.tid_4_sn,
				dw_3->field.tid_5_sn_0 | (dw_4->field.tid_5_sn_5 << 5),
				dw_4->field.tid_6_sn, dw_4->field.tid_7_sn));
	DBGPRINT(RT_DEBUG_OFF, ("\tTxRateCnt(1-5):%d(%d) - %d - %d - %d - %d\n",
				dw_5->field.rate_1_tx_cnt, dw_5->field.rate_1_fail_cnt,
				dw_6->field.rate_2_tx_cnt, dw_6->field.rate_3_tx_cnt,
				dw_6->field.rate_4_tx_cnt, dw_6->field.rate_5_tx_cnt));
	DBGPRINT(RT_DEBUG_OFF, ("\tTxBwCnt(Current-Other):%d(%d) - %d(%d)\n",
				dw_7->field.current_bw_tx_cnt, dw_7->field.current_bw_fail_cnt,
				dw_8->field.other_bw_tx_cnt, dw_8->field.other_bw_fail_cnt));
	DBGPRINT(RT_DEBUG_OFF, ("\tFreqCap:%d(%sMHz)\n",
				dw_9->field.fcap, bw_str[dw_9->field.fcap]));
	DBGPRINT(RT_DEBUG_OFF, ("\tRateIdx/CBRN/CCBW_SEL/SPE_EN: %d/%d/%d/%d\n",
				dw_9->field.rate_idx, dw_9->field.cbrn, dw_9->field.ccbw_sel, dw_9->field.spe_en));
	DBGPRINT(RT_DEBUG_OFF, ("\tMpduCnt(Fail/OK):%d-%d\n",
				dw_9->field.mpdu_fail_cnt, dw_9->field.mpdu_ok_cnt));
	DBGPRINT(RT_DEBUG_OFF, ("\tTxRate Info: G2/G4/G8/G16=%d/%d/%d/%d\n",
				dw_9->field.g2, dw_9->field.g4, dw_9->field.g8, dw_9->field.g16));
	DBGPRINT(RT_DEBUG_OFF, ("\tTxRate Info: RateIdx/STBC/Nsts/PhyMode/TxRate (RawData)\n"));
	rate_info[0] = (dw_10->word & 0xfff);
	rate_info[1] = (dw_10->word & 0xfff000) >> 12;
	rate_info[2] = ((dw_10->word & 0xff000000) >> 24) | ((dw_11->word & 0xf) << 8);
	rate_info[3] = ((dw_11->word & 0xfff0) >> 4);
	rate_info[4] = ((dw_11->word & 0xfff0000) >> 16);
	rate_info[5] = ((dw_11->word & 0xf0000000) >> 28) | ((dw_12->word & 0xff) << 4);
	rate_info[6] = ((dw_12->word & 0xfff00) >> 8);
	rate_info[7] = ((dw_12->word & 0xfff00000) >> 20);
	for (idx = 0; idx <=7; idx++)
	{
		UCHAR stbc, nss, phy_mode, rate;
		UINT32 raw_data;

		raw_data = rate_info[idx] & 0xfff;
		stbc = (raw_data & 0x800) ? 1 : 0;
		nss = (raw_data & 0x600) >> 9;
		phy_mode = (raw_data & 0x1c0) >> 6;
		rate = (raw_data & 0x3f);
		//DBGPRINT(RT_DEBUG_OFF, ("\t\t%d/%d/%d/%d/MCS%d 0x%x\n", idx + 1, stbc, nss,  phy_mode, rate, rate_info[idx]));

		DBGPRINT(RT_DEBUG_OFF, ("\t\t%d/%d/%d/", idx + 1, stbc, nss));
		if ( phy_mode == MODE_CCK )
			DBGPRINT(RT_DEBUG_OFF, ("CCK/"));
		else if ( phy_mode == MODE_OFDM )
			DBGPRINT(RT_DEBUG_OFF, ("OFDM/"));
		else if ( phy_mode == MODE_HTMIX )
			DBGPRINT(RT_DEBUG_OFF, ("HT/"));
		else if ( phy_mode == MODE_HTGREENFIELD )
			DBGPRINT(RT_DEBUG_OFF, ("GF/"));
		else if ( phy_mode == MODE_VHT )
			DBGPRINT(RT_DEBUG_OFF, ("VHT/"));
		else
			DBGPRINT(RT_DEBUG_OFF, ("unkonw/"));

		if ( phy_mode == MODE_CCK ) {
			if ( rate == TMI_TX_RATE_CCK_1M_LP )
				DBGPRINT(RT_DEBUG_OFF, ("1M"));
			else if ( rate == TMI_TX_RATE_CCK_2M_LP )
				DBGPRINT(RT_DEBUG_OFF, ("2M"));
			else if ( rate == TMI_TX_RATE_CCK_5M_LP )
				DBGPRINT(RT_DEBUG_OFF, ("5M"));
			else if ( rate == TMI_TX_RATE_CCK_11M_LP )
				DBGPRINT(RT_DEBUG_OFF, ("11M"));
			else if ( rate == TMI_TX_RATE_CCK_2M_SP )
				DBGPRINT(RT_DEBUG_OFF, ("2M"));
			else if ( rate == TMI_TX_RATE_CCK_5M_SP )
				DBGPRINT(RT_DEBUG_OFF, ("5M"));
			else if ( rate == TMI_TX_RATE_CCK_11M_SP )
				DBGPRINT(RT_DEBUG_OFF, ("11M"));
			else
				DBGPRINT(RT_DEBUG_OFF, ("unkonw"));

		} else if ( phy_mode == MODE_OFDM ) {
			if ( rate == TMI_TX_RATE_OFDM_6M )
				DBGPRINT(RT_DEBUG_OFF, ("6M"));
			else if ( rate == TMI_TX_RATE_OFDM_9M )
				DBGPRINT(RT_DEBUG_OFF, ("9M"));
			else if ( rate == TMI_TX_RATE_OFDM_12M )
				DBGPRINT(RT_DEBUG_OFF, ("12M"));
			else if ( rate == TMI_TX_RATE_OFDM_18M )
				DBGPRINT(RT_DEBUG_OFF, ("18M"));
			else if ( rate == TMI_TX_RATE_OFDM_24M )
				DBGPRINT(RT_DEBUG_OFF, ("24M"));
			else if ( rate == TMI_TX_RATE_OFDM_36M )
				DBGPRINT(RT_DEBUG_OFF, ("36M"));
			else if ( rate == TMI_TX_RATE_OFDM_48M )
				DBGPRINT(RT_DEBUG_OFF, ("48M"));
			else if ( rate == TMI_TX_RATE_OFDM_54M )
				DBGPRINT(RT_DEBUG_OFF, ("54M"));
			else
				DBGPRINT(RT_DEBUG_OFF, ("unkonw"));
		} else {
				DBGPRINT(RT_DEBUG_OFF, ("MCS%d", rate));
		}

		DBGPRINT(RT_DEBUG_OFF, (" 0x%x\n", rate_info[idx]));

	}

	DBGPRINT(RT_DEBUG_OFF, ("\tResp_RCPI0/Resp_RCPI1/Resp_RCPI2=0x%x/0x%x/0x%x\n",
			dw_13->field.resp_rcpi_0, dw_13->field.resp_rcpi_1, dw_13->field.resp_rcpi_2));

	DBGPRINT(RT_DEBUG_OFF, ("\t1CC(Noise)/2CC(Noise)/3CC(Noise)/CE_RMSD/CC_Sel/Ant_Sel=0x%x/0x%x/0x%x/0x%x/%d/%d\n",
			dw_14->field.sts_1_ch_cap_noise, dw_14->field.sts_2_ch_cap_noise, dw_14->field.sts_3_ch_cap_noise,
			dw_14->field.ce_rmsd, dw_14->field.cc_noise_sel, dw_14->field.ant_sel));

	DBGPRINT(RT_DEBUG_OFF, ("\tBA Info: TID/BA_En/BAWinSizeIdx(Range)\n"));
	for (idx = 0; idx <=7; idx++)
	{
		ba_en = (dw_15->field.ba_en & (1 << idx)) ? 1: 0;
		ba_size_idx = (dw_15->field.ba_win_size_tid & (0x7 << (idx * 3))) >> ((idx * 3));
		if (ba_size_idx < 8)
			ba_size = ba_range[ba_size_idx];
		DBGPRINT(RT_DEBUG_OFF, ("\t\t%d/%d/%d(%d)\n", idx, ba_en, ba_size_idx,  ba_size));
	}
}


VOID dump_wtbl_3_info(RTMP_ADAPTER *pAd, union wtbl_3_struc *tb)
{
	//hex_dump("WTBL Segment 3 Raw Data", (UCHAR *)tb, sizeof(union wtbl_3_struc));
	//DBGPRINT(RT_DEBUG_OFF, ("WTBL Segment 3 Fields:\n"));
}


VOID dump_wtbl_4_info(RTMP_ADAPTER *pAd, struct wtbl_4_struc *tb)
{
	//hex_dump("WTBL Segment 4 Raw Data", (UCHAR *)tb, sizeof(struct wtbl_4_struc));
	//DBGPRINT(RT_DEBUG_OFF, ("WTBL Segment 4 Fields:\n"));
}


VOID dump_wtbl_base_info(RTMP_ADAPTER *pAd)
{
	INT idx;

	for (idx = 0; idx < 4; idx++)
	{
		DBGPRINT(RT_DEBUG_OFF, ("\tWTBL Segment %d info:\n", idx+1));
		DBGPRINT(RT_DEBUG_OFF, ("\t\tMemBaseAddr/FID:0x%x/%d\n",
					pAd->mac_ctrl.wtbl_base_addr[idx],
					pAd->mac_ctrl.wtbl_base_fid[idx]));
		DBGPRINT(RT_DEBUG_OFF, ("\t\tEntrySize/Cnt:%d/%d\n",
					pAd->mac_ctrl.wtbl_entry_size[idx],
					pAd->mac_ctrl.wtbl_entry_cnt[idx]));
	}
}


VOID dump_wtbl_info(RTMP_ADAPTER *pAd, UINT wtbl_idx)
{
	INT idx, start_idx, end_idx, tok;
	UINT32 addr, val[16];
	struct wtbl_1_struc wtbl_1;
	struct wtbl_2_struc wtbl_2;
	union wtbl_3_struc wtbl_3;
	struct wtbl_4_struc wtbl_4;

	DBGPRINT(RT_DEBUG_OFF, ("Dump WTBL info of WLAN_IDX:%d\n", wtbl_idx));

	if (wtbl_idx == RESERVED_WCID) {
		start_idx = 0;
		end_idx = (MT_WTBL_SIZE - 1);
	} else if (wtbl_idx < MT_WTBL_SIZE) {
		start_idx = end_idx = wtbl_idx;
	} else {
		DBGPRINT(RT_DEBUG_ERROR, ("%s():Invalid WTBL index(%d)!\n",
					__FUNCTION__, wtbl_idx));
		return;
	}

	for (idx = start_idx; idx <= end_idx; idx++)
	{
		/* Read WTBL 1 */
		NdisZeroMemory((UCHAR *)&wtbl_1, sizeof(struct wtbl_1_struc));
		addr = pAd->mac_ctrl.wtbl_base_addr[0] + idx * pAd->mac_ctrl.wtbl_entry_size[0];
		DBGPRINT(RT_DEBUG_OFF, ("WTBL Segment 1 HW Addr:0x%x\n", addr));
		RTMP_IO_READ32(pAd, addr, &wtbl_1.wtbl_1_d0.word);
		RTMP_IO_READ32(pAd, addr + 4, &wtbl_1.wtbl_1_d1.word);
		RTMP_IO_READ32(pAd, addr + 8, &wtbl_1.wtbl_1_d2.word);
		RTMP_IO_READ32(pAd, addr + 12, &wtbl_1.wtbl_1_d3.word);
		RTMP_IO_READ32(pAd, addr + 16, &wtbl_1.wtbl_1_d4.word);
		dump_wtbl_1_info(pAd, &wtbl_1);

		/* Read WTBL 2 */
		NdisZeroMemory((UCHAR *)&wtbl_2, sizeof(struct wtbl_2_struc));
		addr = pAd->mac_ctrl.wtbl_base_addr[1] + idx * pAd->mac_ctrl.wtbl_entry_size[1];
		DBGPRINT(RT_DEBUG_OFF, ("WTBL Segment 2 HW Addr:0x%x\n", addr));
		for (tok = 0; tok < sizeof(struct wtbl_2_struc) / 4; tok++) {
			RTMP_IO_READ32(pAd, addr + tok * 4, &val[tok]);
		}
		dump_wtbl_2_info(pAd, (struct wtbl_2_struc *)&val[0]);

		/* Read WTBL 3 */
		NdisZeroMemory((UCHAR *)&wtbl_3, sizeof(union wtbl_3_struc));
		addr = pAd->mac_ctrl.wtbl_base_addr[2] + idx * pAd->mac_ctrl.wtbl_entry_size[2];
		DBGPRINT(RT_DEBUG_OFF, ("WTBL Segment 3 HW Addr:0x%x\n", addr));
		for (tok = 0; tok < sizeof(union wtbl_3_struc) / 4; tok++) {
			RTMP_IO_READ32(pAd, addr + tok * 4, &val[tok]);
		}
		dump_wtbl_3_info(pAd, (union wtbl_3_struc *)&val[0]);
		//RTMP_IO_READ32(pAd, addr, wtbl_3.);
		//dump_wtbl_3_info(pAd, &wtbl_3);

		/* Read WTBL 4 */
		NdisZeroMemory((UCHAR *)&wtbl_4, sizeof(struct wtbl_4_struc));
		addr = pAd->mac_ctrl.wtbl_base_addr[3] + idx * pAd->mac_ctrl.wtbl_entry_size[3];
		DBGPRINT(RT_DEBUG_OFF, ("WTBL Segment 4 HW Addr:0x%x\n", addr));
		RTMP_IO_READ32(pAd, addr, &wtbl_4.ac0.word[0]);
		RTMP_IO_READ32(pAd, addr+4, &wtbl_4.ac0.word[1]);
		RTMP_IO_READ32(pAd, addr + 8, &wtbl_4.ac1.word[0]);
		RTMP_IO_READ32(pAd, addr + 12, &wtbl_4.ac1.word[1]);
		RTMP_IO_READ32(pAd, addr + 16, &wtbl_4.ac2.word[0]);
		RTMP_IO_READ32(pAd, addr + 20, &wtbl_4.ac2.word[1]);
		RTMP_IO_READ32(pAd, addr + 24, &wtbl_4.ac3.word[0]);
		RTMP_IO_READ32(pAd, addr + 28, &wtbl_4.ac3.word[1]);
		dump_wtbl_4_info(pAd, &wtbl_4);
	}
}


VOID dump_wtbl_entry(RTMP_ADAPTER *pAd, struct wtbl_entry *ent)
{
	INT idx;

	DBGPRINT(RT_DEBUG_OFF, ("Dump WTBL SW Entry[%d] Cache info\n", ent->wtbl_idx));
	for (idx = 0; idx < 4; idx++)
	{
		DBGPRINT(RT_DEBUG_OFF, ("\tWTBL %d info:\n", idx+1));
		DBGPRINT(RT_DEBUG_OFF, ("\t\tAddr=0x%x, FID=%d, EID=%d\n",
					ent->wtbl_addr[idx], ent->wtbl_fid[idx], ent->wtbl_eid[idx]));
		switch (idx) {
			case 0:
				dump_wtbl_1_info(pAd, &ent->wtbl_1);
				break;
			case 1:
				dump_wtbl_2_info(pAd, &ent->wtbl_2);
				break;
			case 2:
				dump_wtbl_3_info(pAd, &ent->wtbl_3);
				break;
			case 3:
				dump_wtbl_4_info(pAd, &ent->wtbl_4);
				break;
			default:
				break;
		}
	}
}
#endif /* DBG */


INT mt_wtbl_get_entry234(RTMP_ADAPTER *pAd, UCHAR widx, struct wtbl_entry *ent)
{
	struct rtmp_mac_ctrl *wtbl_ctrl;
	UCHAR wtbl_idx, ecnt_per_page;
	UINT page_offset, element_offset, idx;

	wtbl_ctrl = &pAd->mac_ctrl;
	if (wtbl_ctrl->wtbl_entry_cnt[0] > 0)
		wtbl_idx = (widx < wtbl_ctrl->wtbl_entry_cnt[0] ? widx : wtbl_ctrl->wtbl_entry_cnt[0] - 1);
	else {
		DBGPRINT(RT_DEBUG_ERROR, ("%s():PSE not init yet!\n", __FUNCTION__));
		return FALSE;
	}

	ent->wtbl_idx = wtbl_idx;

	for (idx = 0; idx < 4; idx++) {
		if (idx == 0) {
			/* WTBL 1 */
			ent->wtbl_addr[idx] = wtbl_ctrl->wtbl_base_addr[0] +
								wtbl_idx * wtbl_ctrl->wtbl_entry_size[0];
			ent->wtbl_fid[idx] = 0;
			ent->wtbl_eid[idx] = 0;
		}
		else
		{
			/* WTBL 2/3/4 */
			ecnt_per_page = wtbl_ctrl->page_size / wtbl_ctrl->wtbl_entry_size[idx];
			page_offset = wtbl_idx / ecnt_per_page;
			element_offset = wtbl_idx % ecnt_per_page;
			ent->wtbl_fid[idx] = wtbl_ctrl->wtbl_base_fid[idx] + page_offset;
			if (idx == 2)
				ent->wtbl_eid[idx] = element_offset * 2;
			else
				ent->wtbl_eid[idx] = element_offset;
			ent->wtbl_addr[idx] = wtbl_ctrl->wtbl_base_addr[idx] +
							page_offset * wtbl_ctrl->page_size +
							element_offset * wtbl_ctrl->wtbl_entry_size[idx];
		}
	}

	return TRUE;
}


INT mt_wtbl_init(RTMP_ADAPTER *pAd)
{
	UINT32 page_cnt, ecnt_per_page, offset;
	UCHAR cnt = 0;
	struct wtbl_entry tb_entry;
	union WTBL_1_DW3 *dw3 = (union WTBL_1_DW3 *)&tb_entry.wtbl_1.wtbl_1_d3.word;
	union WTBL_1_DW4 *dw4 = (union WTBL_1_DW4 *)&tb_entry.wtbl_1.wtbl_1_d4.word;

	pAd->mac_ctrl.page_size = MT_PSE_PAGE_SIZE; // size in bytes of each PSE page

	pAd->mac_ctrl.wtbl_base_addr[0] = WTBL_WTBL1DW0; // Start address of WTBL2 in host view
	pAd->mac_ctrl.wtbl_entry_size[0] = sizeof(struct wtbl_1_struc);
	pAd->mac_ctrl.wtbl_entry_cnt[0] = MT_WTBL_SIZE;
	pAd->mac_ctrl.wtbl_base_fid[0] = 0;

	pAd->mac_ctrl.wtbl_base_addr[1] = MT_PCI_REMAP_ADDR_1; // Start address of WTBL2 in host view
	pAd->mac_ctrl.wtbl_entry_size[1] = sizeof(struct wtbl_2_struc);
	pAd->mac_ctrl.wtbl_entry_cnt[1] = MT_WTBL_SIZE;
	pAd->mac_ctrl.wtbl_base_fid[1] = 0;
	ecnt_per_page = pAd->mac_ctrl.page_size / sizeof(struct wtbl_2_struc);
	page_cnt = (pAd->mac_ctrl.wtbl_entry_cnt[1]  + ecnt_per_page - 1) / ecnt_per_page;
	offset = page_cnt * pAd->mac_ctrl.page_size;

	pAd->mac_ctrl.wtbl_base_fid[2] = pAd->mac_ctrl.wtbl_base_fid[1] + page_cnt;
	pAd->mac_ctrl.wtbl_base_addr[2] = pAd->mac_ctrl.wtbl_base_addr[1] + offset;
	pAd->mac_ctrl.wtbl_entry_size[2] = sizeof(union wtbl_3_struc);
	pAd->mac_ctrl.wtbl_entry_cnt[2] = MT_WTBL_SIZE;
	ecnt_per_page = pAd->mac_ctrl.page_size / sizeof(union wtbl_3_struc);
	page_cnt = (pAd->mac_ctrl.wtbl_entry_cnt[2]  + ecnt_per_page - 1) / ecnt_per_page;
	offset = page_cnt * pAd->mac_ctrl.page_size;

	pAd->mac_ctrl.wtbl_base_fid[3] = pAd->mac_ctrl.wtbl_base_fid[2] + page_cnt;
	pAd->mac_ctrl.wtbl_base_addr[3] = pAd->mac_ctrl.wtbl_base_addr[2] + offset;
	pAd->mac_ctrl.wtbl_entry_size[3] = sizeof(struct wtbl_4_struc);
	pAd->mac_ctrl.wtbl_entry_cnt[3] = MT_WTBL_SIZE;

	for (cnt = 0; cnt < MT_WTBL_SIZE; cnt++)
	{
		NdisZeroMemory(&tb_entry, sizeof(tb_entry));
		if (mt_wtbl_get_entry234(pAd, cnt, &tb_entry) == FALSE) {
			DBGPRINT(RT_DEBUG_ERROR, ("%s():Cannot found WTBL2/3/4 for WCID(%d)\n",
						__FUNCTION__, cnt));
			return FALSE;
		}

		dw3->field.wtbl2_fid = tb_entry.wtbl_fid[1];
		dw3->field.wtbl2_eid = tb_entry.wtbl_eid[1];
		dw3->field.wtbl4_fid = tb_entry.wtbl_fid[3];
		dw4->field.wtbl3_fid = tb_entry.wtbl_fid[2];
		dw4->field.wtbl3_eid = tb_entry.wtbl_eid[2];
		dw4->field.wtbl4_eid = tb_entry.wtbl_eid[3];

		RTMP_IO_WRITE32(pAd, tb_entry.wtbl_addr[0] + 12, dw3->word);
		RTMP_IO_WRITE32(pAd, tb_entry.wtbl_addr[0] + 16, dw4->word);

		tb_entry.wtbl_1.wtbl_1_d0.field.rc_a2 = 1;
		tb_entry.wtbl_1.wtbl_1_d0.field.rv = 1;
		RTMP_IO_WRITE32(pAd, tb_entry.wtbl_addr[0], tb_entry.wtbl_1.wtbl_1_d0.word);
	}
#ifdef DBG
	dump_wtbl_base_info(pAd);
#endif /* DBG */
	return TRUE;
}


INT mt_hw_tb_init(RTMP_ADAPTER *pAd, BOOLEAN bHardReset)
{
	// TODO: shiang-7603

	mt_wtbl_init(pAd);

	/* Create default entry for rx packets which A2 is not in our table */
	AsicUpdateRxWCIDTable(pAd, RESERVED_WCID, BROADCAST_ADDR);


#if defined(RLT_MAC) || defined(RTMP_MAC)
	/*
		ASIC will keep garbage value after boot
		Clear all shared key table when initial
		This routine can be ignored in radio-ON/OFF operation.
	*/
	if (bHardReset)
	{
		USHORT KeyIdx;
		UINT32 wcid_attr_base = 0, wcid_attr_size = 0, share_key_mode_base = 0;

#ifdef RLT_MAC
		if (pAd->chipCap.hif_type == HIF_RLT) {
			wcid_attr_base = RLT_MAC_WCID_ATTRIBUTE_BASE;
			wcid_attr_size = RLT_HW_WCID_ATTRI_SIZE;
			share_key_mode_base = RLT_SHARED_KEY_MODE_BASE;
		}
#endif /* RLT_MAC */
#ifdef RTMP_MAC
		if (pAd->chipCap.hif_type == HIF_RTMP) {
			wcid_attr_base = MAC_WCID_ATTRIBUTE_BASE;
			wcid_attr_size = HW_WCID_ATTRI_SIZE;
			share_key_mode_base = SHARED_KEY_MODE_BASE;
		}
#endif /* RTMP_MAC */

		for (KeyIdx = 0; KeyIdx < 4; KeyIdx++)
		{
			RTMP_IO_WRITE32(pAd, share_key_mode_base + 4*KeyIdx, 0);
		}

		/* Clear all pairwise key table when initial*/
		for (KeyIdx = 0; KeyIdx < 256; KeyIdx++)
		{
			RTMP_IO_WRITE32(pAd, wcid_attr_base + (KeyIdx * wcid_attr_size), 1);
		}
	}
#endif /* defined(RLT_MAC) || defined(RTMP_MAC) */

	return TRUE;
}


/*
	ASIC register initialization sets
*/
INT mt_mac_init(RTMP_ADAPTER *pAd)
{
	// TODO: shiang-7603
DBGPRINT(RT_DEBUG_OFF, ("%s()-->\n", __FUNCTION__));

	mt_mac_pse_init(pAd);

	/* re-set specific MAC registers for individual chip */
	if (pAd->chipOps.AsicMacInit != NULL)
		pAd->chipOps.AsicMacInit(pAd);

	/* auto-fall back settings */
	AsicAutoFallbackInit(pAd);

	AsicSetMacMaxLen(pAd);

DBGPRINT(RT_DEBUG_OFF, ("<--%s()\n", __FUNCTION__));
	return TRUE;
}

