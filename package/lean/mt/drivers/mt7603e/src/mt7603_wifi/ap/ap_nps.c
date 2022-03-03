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

	tim_info = &pMbss->tim_buf;
	if (tim_info->TimBufIdx >= HW_BEACON_MAX_NUM)
		return result;

	if (tim_info->bTimSntReq == TRUE)
	{
		result = TRUE;
	}

	return result;
}

#ifdef MT_MAC
VOID RT28xx_UpdateTimToAsic(
	IN RTMP_ADAPTER *pAd,
	IN INT apidx,
	IN ULONG FrameLen)
{
	TIM_BUF_STRUC *tim_buf = NULL;
	UCHAR *buf;
	INT len;
	PNDIS_PACKET *pkt = NULL;

	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		tim_buf = &pAd->ApCfg.MBSSID[apidx].tim_buf;
	}

	if (!tim_buf) {
		DBGPRINT(RT_DEBUG_ERROR, ("%s(): bcn_buf is NULL!\n", __FUNCTION__));
		return;
	}

	pkt = tim_buf->TimPkt;
	if (pkt) {
		buf = (UCHAR *)GET_OS_PKT_DATAPTR(pkt);
		len = FrameLen + pAd->chipCap.tx_hw_hdr_len;
		SET_OS_PKT_LEN(pkt, len);

		/* Now do hardware-depened kick out.*/
		RTMP_SEM_LOCK(&pAd->BcnRingLock);
		HAL_KickOutMgmtTx(pAd, Q_IDX_BCN, pkt, buf, len);
		RTMP_SEM_UNLOCK(&pAd->BcnRingLock);
	} else {
		DBGPRINT(RT_DEBUG_ERROR, ("%s(): TimPkt is NULL!\n", __FUNCTION__));
	}
}

#endif /* MT_MAC */

VOID write_tmac_info_tim(RTMP_ADAPTER *pAd, INT apidx, UCHAR *tmac_buf, HTTRANSMIT_SETTING *BeaconTransmit, ULONG frmLen)
{
	MAC_TX_INFO mac_info;

	NdisZeroMemory((UCHAR *)&mac_info, sizeof(mac_info));

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
	mac_info.q_idx = Q_IDX_BCN;
	mac_info.hdr_len = 24;
	mac_info.bss_idx = apidx;
	mac_info.SpeEn = 1;
	mac_info.Preamble = LONG_PREAMBLE;
	write_tmac_info(pAd, tmac_buf, &mac_info, BeaconTransmit);
}

VOID APMakeBssTimFrame(RTMP_ADAPTER *pAd, INT apidx)
{
	BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[apidx];
	HEADER_802_11 TimHdr;
	LARGE_INTEGER FakeTimestamp;
	ULONG FrameLen = 0;
	UCHAR *pTimFrame, *ptr, *tmac_info;
	HTTRANSMIT_SETTING TimTransmit = {.word = 0};   /* MGMT frame PHY rate setting when operatin at HT rate. */
	//UINT8 TXWISize = pAd->chipCap.TXWISize;
	UINT8 tx_hw_hdr_len = pAd->chipCap.tx_hw_hdr_len;
	UCHAR Cat = 11;//Tim Category field
	UCHAR Act = 0;//Tim Action field
	UCHAR ChkBcn = 0;//Check Beacon field init from 0.
	//UCHAR *pTim;
	//UCHAR ID_1B, TimFirst, TimLast;
	//UINT  i;

	if(!TimTransmitRequired(pAd, apidx, pMbss))
		return;

	if (pMbss->tim_buf.TimPkt == NULL) {
		DBGPRINT(RT_DEBUG_ERROR, ("%s():Invalid TimPkt for MBSS[%d]\n",
					__func__, apidx));
		return;
	}

	tmac_info = (UCHAR *)GET_OS_PKT_DATAPTR(pMbss->tim_buf.TimPkt);
	pTimFrame = (UCHAR *)(tmac_info + tx_hw_hdr_len);

	ActHeaderInit(pAd,
			&TimHdr,
			BROADCAST_ADDR,
			pMbss->wdev.if_addr,
			pMbss->wdev.bssid);
	

	MakeOutgoingFrame(pTimFrame,			&FrameLen,
			sizeof(HEADER_802_11),		&TimHdr,
			1,				&Cat,
			1,				&Act,
			1,				&ChkBcn,
			TIMESTAMP_LEN,			&FakeTimestamp,
			END_OF_ARGS);

	TimTransmit.word = 0;

	pMbss->TimIELocationInTim = (UCHAR)FrameLen; 

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
	

// 	/* find the smallest AID (PS mode) */
// 	TimFirst = 0; /* record first TIM byte != 0x00 */
// 	TimLast = 0;  /* record last  TIM byte != 0x00 */
// 	pTim = pMbss->TimBitmaps;
// 
// 	for(ID_1B=0; ID_1B<WLAN_MAX_NUM_OF_TIM; ID_1B++)
// 	{
// 		/* get the TIM indicating PS packets for 8 stations */
// 		UCHAR tim_1B = pTim[ID_1B];
// 
// 		if (ID_1B == 0)
// 			tim_1B &= 0xfe; /* skip bit0 bc/mc */
// 
// 		if (tim_1B == 0)
// 			continue; /* find next 1B */
// 
// 		if (TimFirst == 0)
// 			TimFirst = ID_1B;
// 
// 		TimLast = ID_1B;
// 	}
// 
// 	/* fill TIM content to beacon buffer */
// 	if (TimFirst & 0x01)
// 		TimFirst --; /* find the even offset byte */
// 
// 	*(ptr + 1) = 3+(TimLast-TimFirst+1); /* TIM IE length */
// 	*(ptr + 4) = TimFirst;
// 
// 	for(i=TimFirst; i<=TimLast; i++)
// 		*(ptr + 5 + i - TimFirst) = pTim[i];
// 
// 	/* bit0 means backlogged mcast/bcast */
// 	/* per spec, this bit in TIM frame shall always 0. */
// 	//TODO: MTK proprietary mechanism.
// 	//if (pAd->ApCfg.DtimCount == 0)
// 		//*(ptr + 4) |= (pMbss->TimBitmaps[WLAN_CT_TIM_BCMC_OFFSET] & 0x01);
// 	*(ptr + 4) = 0;

	/* adjust TIM length according to the new TIM */
	FrameLen += 16;//(2 + *(ptr+1));

	/* When Beacon is use CCK to send, TIM shall use OFDM to send. and it's mandatory. */
	if (pAd->CommonCfg.Channel <= 14) {
		TimTransmit.field.MODE = MODE_OFDM;
		TimTransmit.field.MCS = MCS_RATE_6;
	}

	write_tmac_info_tim(pAd, apidx, tmac_info, &TimTransmit, FrameLen);
	
//	asic_write_bcn_buf(pAd, 
//			tmac_info, TXWISize,
//			pTimFrame, FrameLen,
//			pAd->BeaconOffset[pMbss->bcn_buf.BcnBufIdx]);

	RT28xx_UpdateTimToAsic(pAd, apidx, FrameLen);

//+++Add by shiang for debug
	DBGPRINT(RT_DEBUG_OFF, ("%s(): Dump the Beacon Packet of BSS%d!\n", __FUNCTION__, apidx));
	hex_dump("Initial BeaconBuf", tmac_info, FrameLen + tx_hw_hdr_len);
//---Add by shiang for debug
}

static UCHAR GetTimNum(RTMP_ADAPTER *pAd)
{
	int i;
	int NumTim;
	TIM_BUF_STRUC *tim_info;

	NumTim = 0;
	for (i=0; i<pAd->ApCfg.BssidNum; i++)
	{
		tim_info = &pAd->ApCfg.MBSSID[i].tim_buf;
		if (tim_info->bTimSntReq)
		{
			tim_info->TimBufIdx = NumTim;
			NumTim ++;
		}
	}

	return NumTim;
}

VOID APMakeAllTimFrame(RTMP_ADAPTER *pAd)
{
	INT i;
	//UCHAR NumOfTims;

	/* choose the Beacon number */
	/*NumOfTims =*/ GetTimNum(pAd);

	for(i=0; i<pAd->ApCfg.BssidNum; i++) {
		APMakeBssTimFrame(pAd, i);
//		APMakeBssTimFrame2(pAd, i);//Carter, for test tttt 2 pkt purpose
	}
}

INT wdev_tim_buf_init(RTMP_ADAPTER *pAd, TIM_BUF_STRUC *tim_info)
{
	//bcn_info->bBcnSntReq = FALSE;
	tim_info->TimBufIdx = HW_BEACON_MAX_NUM;

	if (!tim_info->TimPkt) {
		RTMPAllocateNdisPacket(pAd, &tim_info->TimPkt, NULL, 0, NULL, MAX_TIM_SIZE);
		//NdisAllocateSpinLock(pAd, &bcn_info->bcn_lock);
	} else {
		DBGPRINT(RT_DEBUG_OFF, ("%s():TimPkt is allocated!\n", __func__));

	}
//	if (!tim_info->TimPkt2) {
//		RTMPAllocateNdisPacket(pAd, &tim_info->TimPkt2, NULL, NULL, NULL, MAX_TIM_SIZE);
		//NdisAllocateSpinLock(pAd, &bcn_info->bcn_lock);
//	} else {
//		DBGPRINT(RT_DEBUG_OFF, ("%s():TimPkt2 is allocated!\n", __func__));
	//}
	tim_info->bTimSntReq = TRUE;

	return TRUE;
}


#endif /* MT_MAC */
