/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2013, Ralink Technology, Inc.
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

#define RTMP_MODULE_OS

#ifdef RT_CFG80211_SUPPORT

#include "rt_config.h"


VOID CFG80211_Announce802_3Packet(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk, UCHAR FromWhichBSSID)
{
#ifdef CONFIG_AP_SUPPORT

	if (RX_BLK_TEST_FLAG(pRxBlk, fRX_STA))
		announce_or_forward_802_3_pkt(pAd, pRxBlk->pRxPacket, wdev_search_by_idx(pAd, FromWhichBSSID), OPMODE_AP);
	else
#endif /* CONFIG_AP_SUPPORT */
	{
		announce_or_forward_802_3_pkt(pAd, pRxBlk->pRxPacket, wdev_search_by_idx(pAd, FromWhichBSSID), OPMODE_STA);
	}
}

BOOLEAN CFG80211_CheckActionFrameType(
	IN RTMP_ADAPTER *pAd,
	IN PUCHAR preStr,
	IN PUCHAR pData,
	IN UINT32 length)
{
	BOOLEAN isP2pFrame = FALSE;
	struct ieee80211_mgmt *mgmt;

	mgmt = (struct ieee80211_mgmt *)pData;

	if (ieee80211_is_mgmt(mgmt->frame_control)) {
		if (ieee80211_is_probe_resp(mgmt->frame_control)) {
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("CFG80211_PKT: %s ProbeRsp Frame %d\n", preStr, pAd->LatchRfRegs.Channel));

			if (!mgmt->u.probe_resp.timestamp) {
				struct timeval tv;

				do_gettimeofday(&tv);
				mgmt->u.probe_resp.timestamp = ((UINT64) tv.tv_sec * 1000000) + tv.tv_usec;
			}
		} else if (ieee80211_is_disassoc(mgmt->frame_control))
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("CFG80211_PKT: %s DISASSOC Frame\n", preStr));
		else if (ieee80211_is_deauth(mgmt->frame_control))
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("CFG80211_PKT: %s Deauth Frame on %d\n", preStr, pAd->LatchRfRegs.Channel));
		else if (ieee80211_is_action(mgmt->frame_control)) {
			PP2P_PUBLIC_FRAME pFrame = (PP2P_PUBLIC_FRAME)pData;

			if ((pFrame->p80211Header.FC.SubType == SUBTYPE_ACTION) &&
				(pFrame->Category == CATEGORY_PUBLIC) &&
				(pFrame->Action == ACTION_WIFI_DIRECT)) {
				isP2pFrame = TRUE;

				switch (pFrame->Subtype) {
				case GO_NEGOCIATION_REQ:
					MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("CFG80211_PKT: %s GO_NEGOCIACTION_REQ %d\n",
							 preStr, pAd->LatchRfRegs.Channel));
					break;

				case GO_NEGOCIATION_RSP:
					MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("CFG80211_PKT: %s GO_NEGOCIACTION_RSP %d\n",
							 preStr, pAd->LatchRfRegs.Channel));
					break;

				case GO_NEGOCIATION_CONFIRM:
					MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("CFG80211_PKT: %s GO_NEGOCIACTION_CONFIRM %d\n",
							 preStr,  pAd->LatchRfRegs.Channel));
					break;

				case P2P_PROVISION_REQ:
					MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("CFG80211_PKT: %s P2P_PROVISION_REQ %d\n",
							 preStr, pAd->LatchRfRegs.Channel));
					break;

				case P2P_PROVISION_RSP:
					MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("CFG80211_PKT: %s P2P_PROVISION_RSP %d\n",
							 preStr, pAd->LatchRfRegs.Channel));
					break;

				case P2P_INVITE_REQ:
					MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("CFG80211_PKT: %s P2P_INVITE_REQ %d\n",
							 preStr, pAd->LatchRfRegs.Channel));
					break;

				case P2P_INVITE_RSP:
					MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("CFG80211_PKT: %s P2P_INVITE_RSP %d\n",
							 preStr, pAd->LatchRfRegs.Channel));
					break;

				case P2P_DEV_DIS_REQ:
					MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("CFG80211_PKT: %s P2P_DEV_DIS_REQ %d\n",
							 preStr, pAd->LatchRfRegs.Channel));
					break;

				case P2P_DEV_DIS_RSP:
					MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("CFG80211_PKT: %s P2P_DEV_DIS_RSP %d\n",
							 preStr, pAd->LatchRfRegs.Channel));
					break;
				}
			} else if ((pFrame->p80211Header.FC.SubType == SUBTYPE_ACTION) &&
					   (pFrame->Category == CATEGORY_PUBLIC) &&
					   ((pFrame->Action == ACTION_GAS_INITIAL_REQ)	 ||
						(pFrame->Action == ACTION_GAS_INITIAL_RSP)	 ||
						(pFrame->Action == ACTION_GAS_COMEBACK_REQ) ||
						(pFrame->Action == ACTION_GAS_COMEBACK_RSP)))
				isP2pFrame = TRUE;

			else
				MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("CFG80211_PKT: %s ACTION Frame with Channel%d\n", preStr, pAd->LatchRfRegs.Channel));
		} else
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("CFG80211_PKT: %s UNKOWN MGMT FRAME TYPE\n", preStr));
	} else
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("CFG80211_PKT: %s UNKOWN FRAME TYPE\n", preStr));

	return isP2pFrame;
}


#ifdef CFG_TDLS_SUPPORT
BOOLEAN CFG80211_HandleTdlsDiscoverRespFrame(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk, UCHAR OpMode)
{
#ifdef MT_MAC
#else
	RXWI_STRUC *pRxWI = pRxBlk->pRxWI;
	PNET_DEV pNetDev = NULL;
#endif /* MT_MAC */
	PHEADER_802_11 pHeader = pRxBlk->pHeader;
	UINT32 freq;
	/* PNDIS_PACKET pRxPacket = pRxBlk->pRxPacket; */
	PP2P_PUBLIC_FRAME pFrame = (PP2P_PUBLIC_FRAME)pHeader;
	UINT32 MPDUtotalByteCnt = 0;
#ifdef MT_MAC
	MPDUtotalByteCnt = pRxBlk->MPDUtotalByteCnt;
#else
	MPDUtotalByteCnt = pRxWI->RXWI_N.MPDUtotalByteCnt;
#endif /* MT_MAC */

	if ((pFrame->p80211Header.FC.SubType == SUBTYPE_ACTION) &&
		(pFrame->Category == CATEGORY_PUBLIC) &&
		(pFrame->Action == ACTION_TDLS_DISCOVERY_RSP)) {
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s : GOT ACTION_TDLS_DISCOVERY_RSP ACTION: 0x%x\n", __func__, pFrame->Action));
		MAP_CHANNEL_ID_TO_KHZ(pAd->LatchRfRegs.Channel, freq);
		freq /= 1000;
		CFG80211OS_RxMgmt(pAd->net_dev, freq, (PUCHAR)pHeader, MPDUtotalByteCnt);
		return TRUE;
	}

	/* report TDLS DiscoverRespFrame to supplicant , but it does nothing , events.c line 2432 */
	return FALSE;
}
#endif /* CFG_TDLS_SUPPORT */


BOOLEAN CFG80211_HandleP2pMgmtFrame(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk, UCHAR OpMode)
{
#ifndef MT_MAC
	RXWI_STRUC *pRxWI = pRxBlk->pRxWI;
#endif /* !MT_MAC */
	HEADER_802_11 Header;
	PHEADER_802_11 pHeader = &Header;
	PCFG80211_CTRL pCfg80211_ctrl = &pAd->cfg80211_ctrl;
	UINT32 freq;
	UINT32 MPDUtotalByteCnt = 0;

	NdisZeroMemory(&Header, sizeof(HEADER_802_11));
	NdisCopyMemory((UCHAR *) &(pHeader->FC), pRxBlk->FC, sizeof(FRAME_CONTROL));
	NdisCopyMemory(pHeader->Addr1, pRxBlk->Addr1, MAC_ADDR_LEN);
	NdisCopyMemory(pHeader->Addr2, pRxBlk->Addr2, MAC_ADDR_LEN);
	NdisCopyMemory(pHeader->Addr3, pRxBlk->Addr3, MAC_ADDR_LEN);
	pHeader->Duration = pRxBlk->Duration;
#ifdef MT_MAC
	MPDUtotalByteCnt = pRxBlk->MPDUtotalByteCnt;
#else
	MPDUtotalByteCnt = pRxWI->RXWI_N.MPDUtotalByteCnt;
#endif /* MT_MAC */

	if ((pHeader->FC.SubType == SUBTYPE_PROBE_REQ) ||
		((pHeader->FC.SubType == SUBTYPE_ACTION) &&
		 CFG80211_CheckActionFrameType(pAd, "RX", (PUCHAR)pHeader, MPDUtotalByteCnt))) {
		MAP_CHANNEL_ID_TO_KHZ(pAd->LatchRfRegs.Channel, freq);
		freq /= 1000;

		if (((pHeader->FC.SubType == SUBTYPE_PROBE_REQ) &&
			 (pCfg80211_ctrl->cfg80211MainDev.Cfg80211RegisterProbeReqFrame == TRUE)) ||
			((pHeader->FC.SubType == SUBTYPE_ACTION)  /*&& ( pAd->Cfg80211RegisterActionFrame == TRUE)*/)) {
			MTWF_LOG(DBG_CAT_P2P, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("MAIN STA RtmpOsCFG80211RxMgmt OK!! TYPE = %d, freq = %d, %02x:%02x:%02x:%02x:%02x:%02x\n",
					 pHeader->FC.SubType, freq, PRINT_MAC(pHeader->Addr2)));
			CFG80211OS_RxMgmt(CFG80211_GetEventDevice(pAd), freq, pRxBlk->pData, MPDUtotalByteCnt);

			if (OpMode == OPMODE_AP)
				return TRUE;
		}
	}

	return FALSE;
}


#endif /* RT_CFG80211_SUPPORT */

