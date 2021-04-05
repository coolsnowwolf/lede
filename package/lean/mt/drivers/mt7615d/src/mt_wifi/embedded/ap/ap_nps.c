/****************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ****************************************************************************

    Module Name:
    ap_nps.c

    Abstract:
    IEEE 802.11v NPS SoftAP related function

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    Carter Chen  08-23-2013    created for 7603 AP
 */
#ifdef MT_MAC

#include "rt_config.h"

extern VOID write_tmac_info_tim(RTMP_ADAPTER *pAd, INT apidx, UCHAR *tmac_buf, HTTRANSMIT_SETTING *BeaconTransmit, ULONG frmLen);

BOOLEAN TimTransmitRequired(RTMP_ADAPTER *pAd, INT apidx, BSS_STRUCT *pMbss)
{
	BOOLEAN result = FALSE;
	TIM_BUF_STRUC *tim_info;

	tim_info = &pMbss->wdev.bcn_buf.tim_buf;

	if (tim_info->TimBufIdx >= HW_BEACON_MAX_NUM)
		return result;

	if (tim_info->bTimSntReq == TRUE)
		result = TRUE;

	return result;
}

#ifdef MT_MAC
VOID MtUpdateTimToAsic(
	IN RTMP_ADAPTER *pAd,
	IN INT apidx,
	IN ULONG FrameLen)
{
	TIM_BUF_STRUC *tim_buf = NULL;
	UCHAR *buf;
	INT len;
	PNDIS_PACKET *pkt = NULL;
	UINT32 WdevIdx;
	struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		tim_buf = &wdev->bcn_buf.tim_buf;
	}

	if (!tim_buf) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): bcn_buf is NULL!\n", __func__));
		return;
	}

	WdevIdx = wdev->wdev_idx;
	pkt = tim_buf->TimPkt;

	if (pkt) {
		UCHAR qIdx;
		buf = (UCHAR *)GET_OS_PKT_DATAPTR(pkt);
		len = FrameLen + cap->tx_hw_hdr_len;
		SET_OS_PKT_LEN(pkt, len);
		RTMP_SET_PACKET_WDEV(pkt, WdevIdx);
		qIdx = HcGetBcnQueueIdx(pAd, wdev);
		RTMP_SET_PACKET_TYPE(pkt, TX_MGMT);
		send_mlme_pkt(pAd, pkt, wdev, qIdx, FALSE);
		return;
	} else
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): TimPkt is NULL!\n", __func__));
}
#endif /* MT_MAC */

VOID write_tmac_info_tim(RTMP_ADAPTER *pAd, INT apidx, UCHAR *tmac_buf, HTTRANSMIT_SETTING *BeaconTransmit, ULONG frmLen)
{
	MAC_TX_INFO mac_info;
	struct wifi_dev *wdev;

	NdisZeroMemory((UCHAR *)&mac_info, sizeof(mac_info));
#ifdef MT_MAC
	mac_info.Type = FC_TYPE_MGMT;
	mac_info.SubType = SUBTYPE_ACTION;
#endif
	mac_info.FRAG = FALSE;
	mac_info.CFACK = FALSE;
	mac_info.InsTimestamp = FALSE;
	mac_info.AMPDU = FALSE;
	mac_info.BM = 1;
	mac_info.Ack = FALSE;
	mac_info.NSeq = TRUE;
	mac_info.BASize = 0;
	mac_info.WCID = 0;
	mac_info.Length = frmLen;
	mac_info.PID = PID_MGMT;
	mac_info.TID = 0;
	mac_info.TxRate = 0;
	mac_info.Txopmode = IFS_HTTXOP;
	mac_info.hdr_len = 24;
	mac_info.bss_idx = apidx;
	mac_info.SpeEn = 1;
	mac_info.Preamble = LONG_PREAMBLE;
	mac_info.IsAutoRate = FALSE;
	wdev = wdev_search_by_address(pAd, tmac_buf + sizeof(TMAC_TXD_L) + 10);

	if (!wdev)
		return;

	mac_info.q_idx = HcGetBcnQueueIdx(pAd, wdev);
#ifdef MT_MAC
	mac_info.TxSPriv = wdev->func_idx;
	mac_info.OmacIdx = wdev->OmacIdx;

	if (wdev->bcn_buf.BcnUpdateMethod == BCN_GEN_BY_FW)
		mac_info.IsOffloadPkt = TRUE;
	else
		mac_info.IsOffloadPkt = FALSE;

	mac_info.Preamble = LONG_PREAMBLE;
	NdisZeroMemory(tmac_buf, sizeof(TMAC_TXD_L));
#endif
	pAd->archOps.write_tmac_info_fixed_rate(pAd, tmac_buf, &mac_info, BeaconTransmit);
}

VOID APMakeBssTimFrame(RTMP_ADAPTER *pAd, INT apidx)
{
	BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[apidx];
	HEADER_802_11 TimHdr;
	LARGE_INTEGER FakeTimestamp;
	ULONG FrameLen = 0;
	UCHAR *pTimFrame, *ptr, *tmac_info;
	HTTRANSMIT_SETTING TimTransmit = {.word = 0};   /* MGMT frame PHY rate setting when operatin at HT rate. */
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 tx_hw_hdr_len = cap->tx_hw_hdr_len;
	UCHAR Cat = 11;/* Tim Category field */
	UCHAR Act = 0;/* Tim Action field */
	UCHAR ChkBcn = 0;/* Check Beacon field init from 0. */
	ULONG UpdatePos = 0;
	struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[apidx].wdev;

	if (!TimTransmitRequired(pAd, apidx, pMbss))
		return;

	if (wdev->bcn_buf.tim_buf.TimPkt == NULL) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():Invalid TimPkt for MBSS[%d]\n",
				 __func__, apidx));
		return;
	}

	tmac_info = (UCHAR *)GET_OS_PKT_DATAPTR(wdev->bcn_buf.tim_buf.TimPkt);
	pTimFrame = (UCHAR *)(tmac_info + tx_hw_hdr_len);
	ActHeaderInit(pAd,
				  &TimHdr,
				  BROADCAST_ADDR,
				  wdev->if_addr,
				  wdev->bssid);
	MakeOutgoingFrame(pTimFrame,			&FrameLen,
					  sizeof(HEADER_802_11),		&TimHdr,
					  1,				&Cat,
					  1,				&Act,
					  1,				&ChkBcn,
					  TIMESTAMP_LEN,			&FakeTimestamp,
					  END_OF_ARGS);
	TimTransmit.word = 0;
	wdev->bcn_buf.TimIELocationInTim = (UCHAR)FrameLen;
	UpdatePos = wdev->bcn_buf.TimIELocationInTim;
	/*
		step 2 - update TIM IE
		TODO: enlarge TIM bitmap to support up to 64 STAs
		TODO: re-measure if RT2600 TBTT interrupt happens faster than BEACON sent out time
	*/
	ptr = pTimFrame + (UCHAR)FrameLen;
	*ptr = IE_TIM;
	*(ptr+1) = 0x0e;
	*(ptr + 2) = pAd->ApCfg.DtimCount;
	*(ptr + 3) = pAd->ApCfg.DtimPeriod;
	*(ptr + 4) = 0xa0;
	*(ptr + 5) = 0xa0;
	*(ptr + 6) = 0xa0;
	*(ptr + 7) = 0xa0;
	*(ptr + 8) = 0xa0;
	*(ptr + 9) = 0xa0;
	*(ptr + 10) = 0xa0;
	*(ptr + 11) = 0xa0;
	*(ptr + 12) = 0xa0;
	*(ptr + 13) = 0xa0;
	*(ptr + 14) = 0xa0;
	*(ptr + 15) = 0xa0;
	/* adjust TIM length according to the new TIM */
	FrameLen += 16;/* (2 + *(ptr+1)); */

	/* When Beacon is use CCK to send, high rate TIM shall use OFDM to send. and it's mandatory. */
	if (wdev->channel <= 14) {
		TimTransmit.field.MODE = MODE_OFDM;
		TimTransmit.field.MCS = MCS_RATE_6;
	} else {
		TimTransmit.field.MODE = MODE_OFDM;
		TimTransmit.field.MCS = MCS_RATE_9;
	}

	write_tmac_info_tim(pAd, apidx, tmac_info, &TimTransmit, FrameLen);
#ifdef BCN_V2_SUPPORT	/* add bcn v2 support , 1.5k beacon support */
	AsicUpdateBeacon(pAd, wdev, FrameLen, PKT_V2_TIM);
#else
	AsicUpdateBeacon(pAd, wdev, FrameLen, PKT_TIM);
#endif
	if (0) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("%s(): Dump the TimFrame of BSS%d!\n",
				  __func__, apidx));
		hex_dump("Initial TimFrameBuf", tmac_info, FrameLen + tx_hw_hdr_len);
	}
}

UCHAR GetTimNum(RTMP_ADAPTER *pAd)
{
	int i;
	int NumTim;
	TIM_BUF_STRUC *tim_info;

	NumTim = 0;

	for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
		tim_info = &pAd->ApCfg.MBSSID[i].wdev.bcn_buf.tim_buf;

		if (tim_info->bTimSntReq) {
			tim_info->TimBufIdx = NumTim;
			NumTim++;
		}
	}

	return NumTim;
}

VOID APMakeAllTimFrame(RTMP_ADAPTER *pAd)
{
	INT i;

	for (i = 0; i < pAd->ApCfg.BssidNum; i++)
		APMakeBssTimFrame(pAd, i);
}

INT wdev_tim_buf_init(RTMP_ADAPTER *pAd, TIM_BUF_STRUC *tim_info)
{
	INT ret;

	/* tim_info->TimBufIdx = HW_BEACON_MAX_NUM; */
	if (!tim_info->TimPkt) {
		ret = RTMPAllocateNdisPacket(pAd, &tim_info->TimPkt, NULL, 0, NULL, MAX_TIM_SIZE);
		if (ret != NDIS_STATUS_SUCCESS)
			return FALSE;
	} else {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s():TimPkt is allocated!\n", __func__));
	}

	tim_info->bTimSntReq = TRUE;
	return TRUE;
}


#endif /* MT_MAC */
