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
	rtmp_data.c

	Abstract:
	Data path subroutines

	Revision History:
	Who 		When			What
	--------	----------		----------------------------------------------
*/
#include "rt_config.h"





VOID STARxEAPOLFrameIndicate(
	IN PRTMP_ADAPTER pAd,
	IN MAC_TABLE_ENTRY *pEntry,
	IN RX_BLK *pRxBlk,
	IN UCHAR FromWhichBSSID)
{
	UCHAR *pTmpBuf;


#ifdef WPA_SUPPLICANT_SUPPORT
	if (pAd->StaCfg.wpa_supplicant_info.WpaSupplicantUP) {
		/* All EAPoL frames have to pass to upper layer (ex. WPA_SUPPLICANT daemon) */
		/* TBD : process fragmented EAPol frames */
		{
			/* In 802.1x mode, if the received frame is EAP-SUCCESS packet, turn on the PortSecured variable */
			if ((pAd->StaCfg.wdev.IEEE8021X == TRUE) &&
			    (pAd->StaCfg.wdev.WepStatus == Ndis802_11WEPEnabled) &&
			    (EAP_CODE_SUCCESS ==
			     WpaCheckEapCode(pAd, pRxBlk->pData,
					     pRxBlk->DataSize,
					     LENGTH_802_1_H))) {
				PUCHAR Key;
				UCHAR CipherAlg;
				int idx = 0;

				DBGPRINT_RAW(RT_DEBUG_TRACE, ("Receive EAP-SUCCESS Packet\n"));
				STA_PORT_SECURED(pAd);

				if (pAd->StaCfg.wpa_supplicant_info.IEEE8021x_required_keys == FALSE) {
					idx = pAd->StaCfg.wpa_supplicant_info.DesireSharedKeyId;
					CipherAlg = pAd->StaCfg.wpa_supplicant_info.DesireSharedKey[idx].CipherAlg;
					Key = pAd->StaCfg.wpa_supplicant_info.DesireSharedKey[idx].Key;

					if (pAd->StaCfg.wpa_supplicant_info.DesireSharedKey[idx].KeyLen > 0) {
						/* Set key material and cipherAlg to Asic */
						RTMP_ASIC_SHARED_KEY_TABLE(pAd,
									   BSS0,
									   idx,
									   &pAd->StaCfg.wpa_supplicant_info.DesireSharedKey[idx]);

						/* STA doesn't need to set WCID attribute for group key */

						/* Assign pairwise key info */
						RTMP_SET_WCID_SEC_INFO(pAd,
								       BSS0,
								       idx,
								       CipherAlg,
								       BSSID_WCID,
								       SHAREDKEYTABLE);

						RTMP_IndicateMediaState(pAd,
									NdisMediaStateConnected);
						pAd->ExtraInfo = GENERAL_LINK_UP;

						/* For Preventing ShardKey Table is cleared by remove key procedure. */
						pAd->SharedKey[BSS0][idx].CipherAlg = CipherAlg;
						pAd->SharedKey[BSS0][idx].KeyLen =
						    pAd->StaCfg.wpa_supplicant_info.DesireSharedKey[idx].KeyLen;
						NdisMoveMemory(pAd->SharedKey[BSS0][idx].Key,
							       pAd->StaCfg.wpa_supplicant_info.DesireSharedKey[idx].Key,
							       pAd->StaCfg.wpa_supplicant_info.DesireSharedKey[idx].KeyLen);
					}
				}
			}
#ifdef WSC_STA_SUPPORT
			else {
				/* report EAP packets to MLME to check this packet is WPS packet or not */
				if (pAd->StaCfg.WscControl.WscState >= WSC_STATE_LINK_UP) {
					pTmpBuf = pRxBlk->pData - LENGTH_802_11;
					NdisMoveMemory(pTmpBuf, pRxBlk->pHeader, LENGTH_802_11);

					REPORT_MGMT_FRAME_TO_MLME(pAd,
								  pRxBlk->wcid,
								  pTmpBuf,
								  pRxBlk->DataSize +LENGTH_802_11,
								  pRxBlk->rssi[0],
								  pRxBlk->rssi[1],
								  pRxBlk->rssi[2],
								  0,
								  OPMODE_STA);
					DBGPRINT_RAW(RT_DEBUG_TRACE,
						     ("!!! report EAPOL DATA to MLME (len=%d) !!!\n",
						      pRxBlk->DataSize));
				}
			}
#endif /* WSC_STA_SUPPORT */

			Indicate_Legacy_Packet(pAd, pRxBlk, FromWhichBSSID);
			return;
		}
	} else
#endif /* WPA_SUPPLICANT_SUPPORT */
	{
		/*
		   Special DATA frame that has to pass to MLME
		   1. Cisco Aironet frames for CCX2. We need pass it to MLME for special process
		   2. EAPOL handshaking frames when driver supplicant enabled, pass to MLME for special process
		 */
		{
			pTmpBuf = pRxBlk->pData - LENGTH_802_11;
			NdisMoveMemory(pTmpBuf, pRxBlk->pHeader, LENGTH_802_11);

			REPORT_MGMT_FRAME_TO_MLME(pAd, pRxBlk->wcid,
						  pTmpBuf,
						  pRxBlk->DataSize +
						  LENGTH_802_11, pRxBlk->rssi[0],
						  pRxBlk->rssi[1], pRxBlk->rssi[2],
						  0,
						  OPMODE_STA);
			DBGPRINT_RAW(RT_DEBUG_TRACE,
				     ("!!! report EAPOL DATA to MLME (len=%d) !!!\n",
				      pRxBlk->DataSize));
		}
	}

	RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
	return;
}


VOID STARxDataFrameAnnounce(
	IN PRTMP_ADAPTER pAd,
	IN MAC_TABLE_ENTRY *pEntry,
	IN RX_BLK *pRxBlk,
	IN UCHAR FromWhichBSSID)
{

	/* non-EAP frame */
	if (!RTMPCheckWPAframe
	    (pAd, pEntry, pRxBlk->pData, pRxBlk->DataSize, FromWhichBSSID)) {
		/* before LINK UP, all DATA frames are rejected */
		if (!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED)) {
			RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket,
					    NDIS_STATUS_FAILURE);
			return;
		}
#ifdef DOT11Z_TDLS_SUPPORT
		if (TDLS_CheckTDLSframe(pAd, pRxBlk->pData, pRxBlk->DataSize)) {
			UCHAR *pTmpBuf;

			pTmpBuf = pRxBlk->pData - LENGTH_802_11;
			NdisMoveMemory(pTmpBuf, pRxBlk->pHeader, LENGTH_802_11);

			REPORT_MGMT_FRAME_TO_MLME(pAd, pRxBlk->wcid,
						  pTmpBuf,
						  pRxBlk->DataSize +
						  LENGTH_802_11, pRxBlk->rssi[0],
						  pRxBlk->rssi[1], pRxBlk->rssi[2],
						  0,
						  OPMODE_STA);
			DBGPRINT_RAW(RT_DEBUG_TRACE,
				     ("!!! report TDLS Action DATA to MLME (len=%d) !!!\n",
				      pRxBlk->DataSize));

			RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket,
					    NDIS_STATUS_FAILURE);
			return;
		}
#endif /* DOT11Z_TDLS_SUPPORT */



#ifdef WAPI_SUPPORT
		/* report to upper layer if the received frame is WAI frame */
		if (RTMPCheckWAIframe(pRxBlk->pData, pRxBlk->DataSize)) {
			Indicate_Legacy_Packet(pAd, pRxBlk, FromWhichBSSID);
			return;
		}
#endif /* WAPI_SUPPORT */

		{
			struct wifi_dev *wdev = &pAd->StaCfg.wdev;
			/* drop all non-EAP DATA frame before */
			/* this client's Port-Access-Control is secured */
			if (pRxBlk->pHeader->FC.Wep) {
				/* unsupported cipher suite */
				if (wdev->WepStatus == Ndis802_11EncryptionDisabled) {
					/* release packet */
					RELEASE_NDIS_PACKET(pAd,
							    pRxBlk->pRxPacket,
							    NDIS_STATUS_FAILURE);
					return;
				}
			} else {
				/* encryption in-use but receive a non-EAPOL clear text frame, drop it */
				if ((wdev->WepStatus != Ndis802_11EncryptionDisabled)
				    && (wdev->PortSecured == WPA_802_1X_PORT_NOT_SECURED)) {
					/* release packet */
					RELEASE_NDIS_PACKET(pAd,
							    pRxBlk->pRxPacket,
							    NDIS_STATUS_FAILURE);
					return;
				}
			}
		}
		RX_BLK_CLEAR_FLAG(pRxBlk, fRX_EAP);


		if (!RX_BLK_TEST_FLAG(pRxBlk, fRX_ARALINK)) {
			/* Normal legacy, AMPDU or AMSDU */
			CmmRxnonRalinkFrameIndicate(pAd, pRxBlk,
						    FromWhichBSSID);

		} else {
			/* ARALINK */
			CmmRxRalinkFrameIndicate(pAd, pEntry, pRxBlk,
						 FromWhichBSSID);
		}
#ifdef QOS_DLS_SUPPORT
		RX_BLK_CLEAR_FLAG(pRxBlk, fRX_DLS);
#endif /* QOS_DLS_SUPPORT */
	} else {
		RX_BLK_SET_FLAG(pRxBlk, fRX_EAP);
#ifdef DOT11_N_SUPPORT
		if (RX_BLK_TEST_FLAG(pRxBlk, fRX_AMPDU)
		    && (pAd->CommonCfg.bDisableReordering == 0)) {
			Indicate_AMPDU_Packet(pAd, pRxBlk, FromWhichBSSID);
		} else
#endif /* DOT11_N_SUPPORT */
		{
			/* Determin the destination of the EAP frame */
			/*  to WPA state machine or upper layer */
			STARxEAPOLFrameIndicate(pAd, pEntry, pRxBlk, FromWhichBSSID);
		}
	}
}


#ifdef HDR_TRANS_SUPPORT 
VOID STARxDataFrameAnnounce_Hdr_Trns(
	IN PRTMP_ADAPTER pAd,
	IN MAC_TABLE_ENTRY *pEntry,
	IN RX_BLK *pRxBlk,
	IN UCHAR FromWhichBSSID)
{

	/* non-EAP frame */
	if (!RTMPCheckWPAframe_Hdr_Trns
	    (pAd, pEntry, pRxBlk->pTransData, pRxBlk->TransDataSize, FromWhichBSSID)) {
		/* before LINK UP, all DATA frames are rejected */
		if (!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED)) {
			RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket,
					    NDIS_STATUS_FAILURE);
			return;
		}
#ifdef DOT11Z_TDLS_SUPPORT
		// TODO: check!!!
		if (TDLS_CheckTDLSframe(pAd, pRxBlk->pData, pRxBlk->DataSize)) {
			RXWI_STRUC *pRxWI = pRxBlk->pRxWI;
			UCHAR *pTmpBuf;

			pTmpBuf = pRxBlk->pData - LENGTH_802_11;
			NdisMoveMemory(pTmpBuf, pRxBlk->pHeader, LENGTH_802_11);

			REPORT_MGMT_FRAME_TO_MLME(pAd, pRxBlk->wcid,
						  pTmpBuf,
						  pRxBlk->DataSize +
						  LENGTH_802_11, pRxBlk->rssi[0],
						  pRxBlk->rssi[1], pRxBlk->rssi[2],
						  0,
						  OPMODE_STA);
			DBGPRINT_RAW(RT_DEBUG_TRACE,
				     ("!!! report TDLS Action DATA to MLME (len=%d) !!!\n",
				      pRxBlk->DataSize));

			RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket,
					    NDIS_STATUS_FAILURE);
			return;
		}
#endif /* DOT11Z_TDLS_SUPPORT */

#ifdef WAPI_SUPPORT
		// TODO: Check!
		/* report to upper layer if the received frame is WAI frame */
		if (RTMPCheckWAIframe(pRxBlk->pData, pRxBlk->DataSize)) {
			Indicate_Legacy_Packet(pAd, pRxBlk, FromWhichBSSID);
			return;
		}
#endif /* WAPI_SUPPORT */

		{
			/* drop all non-EAP DATA frame before */
			/* this client's Port-Access-Control is secured */
			if (pRxBlk->pHeader->FC.Wep) {
				/* unsupported cipher suite */
				if (pAd->StaCfg.wdev.WepStatus == Ndis802_11EncryptionDisabled) {
					/* release packet */
					RELEASE_NDIS_PACKET(pAd,
							    pRxBlk->pRxPacket,
							    NDIS_STATUS_FAILURE);
					return;
				}
			} else {
				/* encryption in-use but receive a non-EAPOL clear text frame, drop it */
				if ((pAd->StaCfg.wdev.WepStatus != Ndis802_11EncryptionDisabled)
				    && (pAd->StaCfg.wdev.PortSecured == WPA_802_1X_PORT_NOT_SECURED)) {
					/* release packet */
					RELEASE_NDIS_PACKET(pAd,
							    pRxBlk->pRxPacket,
							    NDIS_STATUS_FAILURE);
					return;
				}
			}
		}
		RX_BLK_CLEAR_FLAG(pRxBlk, fRX_EAP);


		if (!RX_BLK_TEST_FLAG(pRxBlk, fRX_ARALINK)) {
			/* Normal legacy, AMPDU or AMSDU */
			CmmRxnonRalinkFrameIndicate_Hdr_Trns(pAd, pRxBlk,
						    FromWhichBSSID);

		} else {
			/* ARALINK */
			CmmRxRalinkFrameIndicate(pAd, pEntry, pRxBlk,
						 FromWhichBSSID);
		}
#ifdef QOS_DLS_SUPPORT
		RX_BLK_CLEAR_FLAG(pRxBlk, fRX_DLS);
#endif /* QOS_DLS_SUPPORT */
	} else {
		RX_BLK_SET_FLAG(pRxBlk, fRX_EAP);
#ifdef DOT11_N_SUPPORT
		if (RX_BLK_TEST_FLAG(pRxBlk, fRX_AMPDU)
		    && (pAd->CommonCfg.bDisableReordering == 0)) {
			Indicate_AMPDU_Packet(pAd, pRxBlk, FromWhichBSSID);
		} else
#endif /* DOT11_N_SUPPORT */
		{
			/* Determin the destination of the EAP frame */
			/*  to WPA state machine or upper layer */
			STARxEAPOLFrameIndicate(pAd, pEntry, pRxBlk,
						FromWhichBSSID);
		}
	}
}
#endif /* HDR_TRANS_SUPPORT */


/* For TKIP frame, calculate the MIC value	*/
BOOLEAN STACheckTkipMICValue(
	IN PRTMP_ADAPTER pAd,
	IN MAC_TABLE_ENTRY *pEntry,
	IN RX_BLK * pRxBlk)
{
	PHEADER_802_11 pHeader = pRxBlk->pHeader;
	UCHAR *pData = pRxBlk->pData;
	USHORT DataSize = pRxBlk->DataSize;
	UCHAR UserPriority = pRxBlk->UserPriority;
	PCIPHER_KEY pWpaKey;
	UCHAR *pDA, *pSA;

	pWpaKey = &pAd->SharedKey[BSS0][pRxBlk->key_idx];

	pDA = pHeader->Addr1;
	if (RX_BLK_TEST_FLAG(pRxBlk, fRX_INFRA)) {
		pSA = pHeader->Addr3;
	} else {
		pSA = pHeader->Addr2;
	}

	if (RTMPTkipCompareMICValue(pAd,
				    pData,
				    pDA,
				    pSA,
				    pWpaKey->RxMic,
				    UserPriority, DataSize) == FALSE) {
		DBGPRINT_RAW(RT_DEBUG_ERROR, ("Rx MIC Value error 2\n"));

#ifdef WPA_SUPPLICANT_SUPPORT
		if (pAd->StaCfg.wpa_supplicant_info.WpaSupplicantUP) {
			WpaSendMicFailureToWpaSupplicant(pAd->net_dev, pHeader->Addr2,
							 (pWpaKey->Type == PAIRWISEKEY) ? TRUE : FALSE,
							 (INT) pRxBlk->key_idx, NULL);
		} else
#endif /* WPA_SUPPLICANT_SUPPORT */
		{
			RTMPReportMicError(pAd, pWpaKey);
		}

		/* release packet */
		RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket,
				    NDIS_STATUS_FAILURE);
		return FALSE;
	}

	return TRUE;
}


/*
 All Rx routines use RX_BLK structure to hande rx events
 It is very important to build pRxBlk attributes
  1. pHeader pointer to 802.11 Header
  2. pData pointer to payload including LLC (just skip Header)
  3. set payload size including LLC to DataSize
  4. set some flags with RX_BLK_SET_FLAG()
*/
VOID STAHandleRxDataFrame(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
	RXINFO_STRUC *pRxInfo = pRxBlk->pRxInfo;
	RXWI_STRUC *pRxWI = pRxBlk->pRxWI;
	HEADER_802_11 *pHeader = pRxBlk->pHeader;
	PNDIS_PACKET pRxPacket = pRxBlk->pRxPacket;
	BOOLEAN bFragment = FALSE;
	MAC_TABLE_ENTRY *pEntry = NULL;
	UCHAR FromWhichBSSID = BSS0;
	UCHAR UserPriority = 0;

//+++Add by shiang for debug
//---Add by shiangf for debug

	if ((pHeader->FC.FrDs == 1) && (pHeader->FC.ToDs == 1)) {
#ifdef CLIENT_WDS
			if ((pRxBlk->wcid < MAX_LEN_OF_MAC_TABLE)
			    && IS_ENTRY_CLIENT(&pAd->MacTab.Content[pRxBlk->wcid])) {
			RX_BLK_SET_FLAG(pRxBlk, fRX_WDS);
			pEntry = &pAd->MacTab.Content[pRxBlk->wcid];
		} else
#endif /* CLIENT_WDS */
		{
			RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
			return;
		}
	}
	else
	{

#ifdef QOS_DLS_SUPPORT
		if (RTMPRcvFrameDLSCheck(pAd, pHeader, pRxBlk->MPDUtotalByteCnt, pRxInfo))
			return;
#endif /* QOS_DLS_SUPPORT */

		/* Drop not my BSS frames */
		if (pRxBlk->wcid < MAX_LEN_OF_MAC_TABLE)
			pEntry = &pAd->MacTab.Content[pRxBlk->wcid];
		
		if (pRxInfo->MyBss == 0) {
#if defined(P2P_SUPPORT) || defined(RT_CFG80211_P2P_SUPPORT)
			/* When the p2p-IF up, the STA own address would be set as my_bssid address.
			   If receiving an "encrypted" broadcast packet(its WEP bit as 1) and doesn't match my BSSID, 
			   Asic pass to driver with "Decrypted" marked as 0 in pRxInfo. 		
			   The condition is below,
			   1. p2p IF is ON,
			   2. the addr2 of the received packet is STA's BSSID,
			   3. broadcast packet,
			   4. from DS packet,
			   5. Asic pass this packet to driver with "pRxInfo->Decrypted=0"
			 */
			if (
#ifdef RT_CFG80211_P2P_SUPPORT
			       TRUE /* The dummy device always present for CFG80211 application*/
#else
                    (P2P_INF_ON(pAd)) 
#endif /* RT_CFG80211_P2P_SUPPORT */
                     && (MAC_ADDR_EQUAL(pAd->CommonCfg.Bssid, pHeader->Addr2)) 
			   		 && (pRxInfo->Bcast || pRxInfo->Mcast) 
                     && (pHeader->FC.FrDs == 1) 
                     && (pHeader->FC.ToDs == 0) 
			         && (pRxInfo->Decrypted == 0))
			{
				/* set this m-cast frame is my-bss. */
				pRxInfo->MyBss = 1;
			}
			else
#endif /* P2P_SUPPORT || RT_CFG80211_P2P_SUPPORT*/
			{
				RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
				return;
			}
		}


		pAd->RalinkCounters.RxCountSinceLastNULL++;
#ifdef UAPSD_SUPPORT
		if (pAd->StaCfg.UapsdInfo.bAPSDCapable
		    && pAd->CommonCfg.APEdcaParm.bAPSDCapable
		    && (pHeader->FC.SubType & 0x08))
		{
			UCHAR *pData;
			DBGPRINT(RT_DEBUG_INFO, ("bAPSDCapable\n"));

			/* Qos bit 4 */
			pData = (PUCHAR) pHeader + LENGTH_802_11;
			if ((*pData >> 4) & 0x01)
			{
#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
				/* ccv EOSP frame so the peer can sleep */
				if (pEntry != NULL)
				{
					RTMP_PS_VIRTUAL_SLEEP(pEntry);
				}

				if (pAd->StaCfg.FlgPsmCanNotSleep == TRUE)
				{
					DBGPRINT(RT_DEBUG_TRACE, ("tdls uapsd> Rcv EOSP frame but we can not sleep!\n"));
				}
				else
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */
 				{
					DBGPRINT(RT_DEBUG_INFO,
						("RxDone- Rcv EOSP frame, driver may fall into sleep\n"));
					pAd->CommonCfg.bInServicePeriod = FALSE;

					/* Force driver to fall into sleep mode when rcv EOSP frame */
					if (!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE))
					{
#ifdef RTMP_MAC_PCI
						USHORT TbttNumToNextWakeUp;
						USHORT NextDtim = pAd->StaCfg.DtimPeriod;
						ULONG Now;

						NdisGetSystemUpTime(&Now);
						NextDtim -= (USHORT) (Now - pAd->StaCfg.LastBeaconRxTime) / pAd->CommonCfg.BeaconPeriod;

						TbttNumToNextWakeUp = pAd->StaCfg.DefaultListenCount;
						if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_RECEIVE_DTIM) && (TbttNumToNextWakeUp > NextDtim))
							TbttNumToNextWakeUp = NextDtim;

						RTMP_SET_PSM_BIT(pAd, PWR_SAVE);
						/* if WMM-APSD is failed, try to disable following line */
						AsicSleepThenAutoWakeup(pAd, TbttNumToNextWakeUp);
#endif /* RTMP_MAC_PCI */

					}
				}
			}

			if ((pHeader->FC.MoreData)
			    && (pAd->CommonCfg.bInServicePeriod)) {
				DBGPRINT(RT_DEBUG_TRACE,
					 ("Sending another trigger frame when More Data bit is set to 1\n"));
			}
		}
#endif /* UAPSD_SUPPORT */

#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
		/* 1: PWR_SAVE, 0: PWR_ACTIVE */
		if (pEntry != NULL)
		{
			UCHAR OldPwrMgmt;
			FRAME_CONTROL *pFmeCtrl = &pHeader->FC;

			OldPwrMgmt = RtmpPsIndicate(pAd, pHeader->Addr2,
									pRxBlk->wcid, pFmeCtrl->PwrMgmt);

#ifdef UAPSD_SUPPORT
			RTMP_PS_VIRTUAL_TIMEOUT_RESET(pEntry);

			if (pFmeCtrl->PwrMgmt)
			{
				if ((CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_APSD_CAPABLE)) &&
					(pFmeCtrl->SubType & 0x08))
				{
					/*
						In IEEE802.11e, 11.2.1.4 Power management with APSD,
						If there is no unscheduled SP in progress, the unscheduled SP begins
						when the QAP receives a trigger frame from a non-AP QSTA, which is a
						QoS data or QoS Null frame associated with an AC the STA has
						configured to be trigger-enabled.

						In WMM v1.1, A QoS Data or QoS Null frame that indicates transition
						to/from Power Save Mode is not considered to be a Trigger Frame and
						the AP shall not respond with a QoS Null frame.
					*/
					/* Trigger frame must be QoS data or QoS Null frame */
					UCHAR  OldUP;

					if ((*(pRxBlk->pData+LENGTH_802_11) & 0x10) == 0)
					{
						/* this is not a EOSP frame */
						OldUP = (*(pRxBlk->pData+LENGTH_802_11) & 0x07);
						if (OldPwrMgmt == PWR_SAVE)
						{
							//hex_dump("trigger frame", pRxBlk->pData, 26);
							UAPSD_TriggerFrameHandle(pAd, pEntry, OldUP);
						}
					}
					else
					{
						DBGPRINT(RT_DEBUG_TRACE,
							("This is a EOSP frame, not a trigger frame!\n"));
					}
				}
			}
#endif /* UAPSD_SUPPORT */
		}
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */

		/* Drop NULL, CF-ACK(no data), CF-POLL(no data), and CF-ACK+CF-POLL(no data) data frame */
		if ((pHeader->FC.SubType & 0x04)) /* bit 2 : no DATA */
		{
			RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
			return;
		}

		if (pAd->StaCfg.BssType == BSS_INFRA) {
			/* Infrastructure mode, check address 2 for BSSID */
			if (1
#ifdef QOS_DLS_SUPPORT
			    && (!pAd->CommonCfg.bDLSCapable)
#endif /* QOS_DLS_SUPPORT */
#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
			    && (!IS_TDLS_SUPPORT(pAd))
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */
			    ) {
				if (!RTMPEqualMemory(&pHeader->Addr2, &pAd->MlmeAux.Bssid, 6))
				{
					/* Receive frame not my BSSID */
					RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
					return;
				}
			}
		}
		else
		{	/* Ad-Hoc mode or Not associated */

			/* Ad-Hoc mode, check address 3 for BSSID */
			if (!RTMPEqualMemory(&pHeader->Addr3, &pAd->CommonCfg.Bssid, 6)) {
				/* Receive frame not my BSSID */
				RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
				return;
			}
		}

		/*/ find pEntry */
		if (pRxBlk->wcid < MAX_LEN_OF_MAC_TABLE) {
			pEntry = &pAd->MacTab.Content[pRxBlk->wcid];

		} else {
			/* IOT issue with Marvell test bed AP
			    Marvell AP ResetToOOB and do wps. 
			    Because of AP send EAP Request too fast and without retransmit.
			    STA not yet add BSSID to WCID search table.
			    So, the EAP Request is dropped.
			    The patch lookup pEntry from MacTable.
			*/
			pEntry = MacTableLookup(pAd, &pHeader->Addr2[0]);
			if ( pEntry == NULL )
			{
				RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
				return;
			}
		}

		/* infra or ad-hoc */
		if (pAd->StaCfg.BssType == BSS_INFRA) {
			RX_BLK_SET_FLAG(pRxBlk, fRX_INFRA);
#if defined(DOT11Z_TDLS_SUPPORT) || defined(QOS_DLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
			if ((pHeader->FC.FrDs == 0) && (pHeader->FC.ToDs == 0))
				RX_BLK_SET_FLAG(pRxBlk, fRX_DLS);
			else
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */
			{
				ASSERT(pRxBlk->wcid == BSSID_WCID);
				if (pRxBlk->wcid != BSSID_WCID)
				{
					printk("[%d] 1: %02x:%02x:%02x:%02x:%02x:%02x, 2: %02x:%02x:%02x:%02x:%02x:%02x, 3:%02x:%02x:%02x:%02x:%02x:%02x", 
					pRxBlk->wcid, PRINT_MAC(pHeader->Addr1), PRINT_MAC(pHeader->Addr2), PRINT_MAC(pHeader->Addr3));
				}
			}
		}

#ifndef WFA_VHT_PF
		// TODO: shiang@PF#2, is this atheros protection still necessary here???
		/* check Atheros Client */
		if ((pEntry->bIAmBadAtheros == FALSE) && (pRxInfo->AMPDU == 1)
		    && (pHeader->FC.Retry)) {
			pEntry->bIAmBadAtheros = TRUE;
			pAd->CommonCfg.IOTestParm.bLastAtheros = TRUE;
			if (!STA_AES_ON(pAd))
				RTMP_UPDATE_PROTECT(pAd, 8 , ALLN_SETPROTECT, TRUE, FALSE);
		}
#endif /* WFA_VHT_PF */
	}


	pRxBlk->pData = (UCHAR *) pHeader;

	/*
	   update RxBlk->pData, DataSize
	   802.11 Header, QOS, HTC, Hw Padding
	 */
	/* 1. skip 802.11 HEADER */
#ifdef CLIENT_WDS
	if (RX_BLK_TEST_FLAG(pRxBlk, fRX_WDS)) {
		pRxBlk->pData += LENGTH_802_11_WITH_ADDR4;
		pRxBlk->DataSize -= LENGTH_802_11_WITH_ADDR4;
	} else
#endif /* CLIENT_WDS */
	{
		pRxBlk->pData += LENGTH_802_11;
		pRxBlk->DataSize -= LENGTH_802_11;
	}

	/* 2. QOS */
	if (pHeader->FC.SubType & 0x08) {
		RX_BLK_SET_FLAG(pRxBlk, fRX_QOS);
		UserPriority = *(pRxBlk->pData) & 0x0f;
		/* bit 7 in QoS Control field signals the HT A-MSDU format */
		if ((*pRxBlk->pData) & 0x80) {
			RX_BLK_SET_FLAG(pRxBlk, fRX_AMSDU);
		}

		/* skip QOS contorl field */
		pRxBlk->pData += 2;
		pRxBlk->DataSize -= 2;
	}
	pRxBlk->UserPriority = UserPriority;

	/* check if need to resend PS Poll when received packet with MoreData = 1 */
#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
	if (pRxInfo->U2M)
	{
		/* only for unicast packet */
		/* for TDLS power save, More Data bit is not used */
		if (((pEntry != NULL) && (!IS_ENTRY_TDLS(pEntry))) || (pEntry == NULL))
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */
		{
			if ((RtmpPktPmBitCheck(pAd) == TRUE) && (pHeader->FC.MoreData == 1))
			{
				if ((((UserPriority == 0) || (UserPriority == 3)) && pAd->CommonCfg.bAPSDAC_BE == 0) ||
		    			(((UserPriority == 1) || (UserPriority == 2)) && pAd->CommonCfg.bAPSDAC_BK == 0) ||
					(((UserPriority == 4) || (UserPriority == 5)) && pAd->CommonCfg.bAPSDAC_VI == 0) ||
					(((UserPriority == 6) || (UserPriority == 7)) && pAd->CommonCfg.bAPSDAC_VO == 0)) 
				{
					/* non-UAPSD delivery-enabled AC */
					RTMP_PS_POLL_ENQUEUE(pAd);
				}
			}
		}
#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
	}
#endif /* DOT11Z_TDLS_SUPPORT */

	/* 3. Order bit: A-Ralink or HTC+ */
	if (pHeader->FC.Order) {
#ifdef AGGREGATION_SUPPORT
		if ((pRxBlk->rx_rate.field.MODE <= MODE_OFDM)
		    && (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_AGGREGATION_INUSED)))
		{
			RX_BLK_SET_FLAG(pRxBlk, fRX_ARALINK);
		} else
#endif /* AGGREGATION_SUPPORT */
		{
#ifdef DOT11_N_SUPPORT
			RX_BLK_SET_FLAG(pRxBlk, fRX_HTC);
			/* skip HTC contorl field */
			pRxBlk->pData += 4;
			pRxBlk->DataSize -= 4;
#endif /* DOT11_N_SUPPORT */
		}
	}

	/* 4. skip HW padding */
	if (pRxInfo->L2PAD) {
		/* just move pData pointer, because DataSize excluding HW padding */
		RX_BLK_SET_FLAG(pRxBlk, fRX_PAD);
		pRxBlk->pData += 2;
	}
#ifdef DOT11_N_SUPPORT
	if (pRxInfo->BA) {
		RX_BLK_SET_FLAG(pRxBlk, fRX_AMPDU);
	}
#endif /* DOT11_N_SUPPORT */

#if defined(SOFT_ENCRYPT) || defined(ADHOC_WPA2PSK_SUPPORT)
	/* Use software to decrypt the encrypted frame if necessary.
	   If a received "encrypted" unicast packet(its WEP bit as 1) 
	   and it's passed to driver with "Decrypted" marked as 0 in pRxInfo. */
	if ((pHeader->FC.Wep == 1) && (pRxInfo->Decrypted == 0)) {
		PCIPHER_KEY pSwKey = RTMPSwCipherKeySelection(pAd,
						       			pRxBlk->pData, pRxBlk,
						       			pEntry);

		/* Cipher key table selection */
		if (!pSwKey) {
			DBGPRINT(RT_DEBUG_TRACE, ("No vaild cipher key for SW decryption!!!\n"));
			RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
			return;
		}

		/* Decryption by Software */
		if (RTMPSoftDecryptionAction(pAd,
					     (PUCHAR) pHeader,
					     UserPriority,
					     pSwKey,
					     pRxBlk->pData,
					     &(pRxBlk->DataSize)) !=
		    NDIS_STATUS_SUCCESS) {
			RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
			return;
		}
		/* Record the Decrypted bit as 1 */
		pRxInfo->Decrypted = 1;
	}
#endif /* SOFT_ENCRYPT || ADHOC_WPA2PSK_SUPPORT */

#ifdef DOT11Z_TDLS_SUPPORT
#ifdef TDLS_AUTOLINK_SUPPORT
	if (pAd->StaCfg.TdlsInfo.TdlsAutoLink)
	{
		if (!RX_BLK_TEST_FLAG(pRxBlk, fRX_DLS))
			TDLS_AutoSetupByRcvFrame(pAd, pHeader);
	}
#endif /* TDLS_AUTOLINK_SUPPORT */
#endif /* DOT11Z_TDLS_SUPPORT */

	/* Case I  Process Broadcast & Multicast data frame */
	if (pRxInfo->Bcast || pRxInfo->Mcast) {
#ifdef STATS_COUNT_SUPPORT
		INC_COUNTER64(pAd->WlanCounters.MulticastReceivedFrameCount);
#endif /* STATS_COUNT_SUPPORT */

		/* Drop Mcast/Bcast frame with fragment bit on */
		if (pHeader->FC.MoreFrag) {
			RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
			return;
		}

		/* Filter out Bcast frame which AP relayed for us */
		if (pHeader->FC.FrDs
		    && MAC_ADDR_EQUAL(pHeader->Addr3, pAd->CurrentAddress)) {
			RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
			return;
		}

		if (ADHOC_ON(pAd)) {
			MAC_TABLE_ENTRY *pAdhocEntry = NULL;
			pAdhocEntry = MacTableLookup(pAd, pHeader->Addr2);
			if (pAdhocEntry)
				Update_Rssi_Sample(pAd, &pAdhocEntry->RssiSample, pRxWI);
		}

#ifdef DOT11Z_TDLS_SUPPORT
#ifdef WFD_SUPPORT
		if ((pAd->StaCfg.WfdCfg.bWfdEnable) &&
			pRxInfo->Bcast &&
			TDLS_CheckTDLSframe(pAd, pRxBlk->pData, pRxBlk->DataSize))
		{
			UCHAR *pTmpBuf = pRxBlk->pData - LENGTH_802_11;

			NdisMoveMemory(pTmpBuf, pRxBlk->pHeader, LENGTH_802_11);
			REPORT_MGMT_FRAME_TO_MLME(pAd, pRxBlk->wcid,
						  pTmpBuf,
						  pRxBlk->DataSize +
						  LENGTH_802_11,
						  pRxBlk->rssi[0],
						  pRxBlk->rssi[1],
						  pRxBlk->rssi[2],
						  0,
						  OPMODE_STA);
			DBGPRINT_RAW(RT_DEBUG_TRACE,
					 ("!!! report TDLS Action DATA to MLME (len=%d) !!!\n",
					  pRxBlk->DataSize));

			RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
			return;
		}
#endif /* WFD_SUPPORT */
#endif /* DOT11Z_TDLS_SUPPORT */

		Indicate_Legacy_Packet(pAd, pRxBlk, FromWhichBSSID);
		return;
	}
	else if (pRxInfo->U2M)
	{
		pAd->LastRxRate = (ULONG)(pRxBlk->rx_rate.word);

#if defined(DOT11Z_TDLS_SUPPORT) || defined(QOS_DLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
		if (RX_BLK_TEST_FLAG(pRxBlk, fRX_DLS)) {
			MAC_TABLE_ENTRY *pDlsEntry = &pAd->MacTab.Content[pRxBlk->wcid];

			if (pDlsEntry && (pRxBlk->wcid < MAX_LEN_OF_MAC_TABLE)) {
				Update_Rssi_Sample(pAd, &pDlsEntry->RssiSample, pRxWI);
				NdisAcquireSpinLock(&pAd->MacTabLock);
				pDlsEntry->NoDataIdleCount = 0;
				NdisReleaseSpinLock(&pAd->MacTabLock);
			}
		}
		else
#endif
		if (INFRA_ON(pAd)) {
			MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[BSSID_WCID];
			if (pEntry)
				Update_Rssi_Sample(pAd, &pEntry->RssiSample, pRxWI);
		}
		else if (ADHOC_ON(pAd)) {
			MAC_TABLE_ENTRY *pAdhocEntry = NULL;
			pAdhocEntry = MacTableLookup(pAd, pHeader->Addr2);
			if (pAdhocEntry)
				Update_Rssi_Sample(pAd, &pAdhocEntry->RssiSample, pRxWI);
		}

		Update_Rssi_Sample(pAd, &pAd->StaCfg.RssiSample, pRxWI);

#ifdef DBG_DIAGNOSE
		if (pAd->DiagStruct.inited) {
			struct dbg_diag_info *diag_info;
			diag_info = &pAd->DiagStruct.diag_info[pAd->DiagStruct.ArrayCurIdx];
			diag_info->RxDataCnt++;
#ifdef DBG_RX_MCS
			if (pRxBlk->rx_rate.field.MODE == MODE_HTMIX ||
				pRxBlk->rx_rate.field.MODE == MODE_HTGREENFIELD) {
				if (pRxBlk->rx_rate.field.MCS < MAX_MCS_SET)
					diag_info->RxMcsCnt_HT[pRxBlk->rx_rate.field.MCS]++;
			}
#ifdef DOT11_VHT_AC
			if (pRxBlk->rx_rate.field.MODE == MODE_VHT) {
				INT mcs_idx = ((pRxBlk->rx_rate.field.MCS >> 4) * 10) + 
								(pRxBlk->rx_rate.field.MCS & 0xf);
				if (mcs_idx < MAX_VHT_MCS_SET)
					diag_info->RxMcsCnt_VHT[mcs_idx]++;
			}
#endif /* DOT11_VHT_AC */
#endif /* DBG_RX_MCS */
		}
#endif /* DBG_DIAGNOSE */

		pAd->StaCfg.LastSNR0 = (UCHAR) (pRxBlk->snr[0]);
		pAd->StaCfg.LastSNR1 = (UCHAR) (pRxBlk->snr[1]);
#ifdef DOT11N_SS3_SUPPORT
		if (pAd->CommonCfg.RxStream == 3)
			pAd->StaCfg.LastSNR2 = (UCHAR) (pRxBlk->snr[2]);
#endif /* DOT11N_SS3_SUPPORT */

		pAd->RalinkCounters.OneSecRxOkDataCnt++;

		if (pEntry != NULL)
		{
			pEntry->LastRxRate = pAd->LastRxRate;
#ifdef TXBF_SUPPORT
			//if (pRxWI->ShortGI)
			if (pRxBlk->rx_rate.field.ShortGI)
				pEntry->OneSecRxSGICount++;
			else
				pEntry->OneSecRxLGICount++;
#endif /* TXBF_SUPPORT */

			pEntry->freqOffset = (CHAR)(pRxBlk->freq_offset);
			pEntry->freqOffsetValid = TRUE;

		}

#ifdef PRE_ANT_SWITCH
#endif /* PRE_ANT_SWITCH */

//Freddy
//#ifdef RTMP_MAC_USB
		/* there's packet sent to me, keep awake for 1200ms */
		if (pAd->CountDowntoPsm < 12)
			pAd->CountDowntoPsm = 12;
//#endif /* RTMP_MAC_USB */

		if (!((pHeader->Frag == 0) && (pHeader->FC.MoreFrag == 0))) {
			/* re-assemble the fragmented packets */
			/* return complete frame (pRxPacket) or NULL */
			bFragment = TRUE;
			pRxPacket = RTMPDeFragmentDataFrame(pAd, pRxBlk);
		}

		if (pRxPacket) {
			pEntry = &pAd->MacTab.Content[pRxBlk->wcid];

			/* process complete frame */
			if (bFragment && (pRxInfo->Decrypted)
			    && (pEntry->WepStatus == Ndis802_11TKIPEnable)) {
				/* Minus MIC length */
				pRxBlk->DataSize -= 8;

				/* For TKIP frame, calculate the MIC value */
				if (STACheckTkipMICValue(pAd, pEntry, pRxBlk) == FALSE) {
					return;
				}
			}

			STARxDataFrameAnnounce(pAd, pEntry, pRxBlk, FromWhichBSSID);
			return;
		} else {
			/*
			   just return because RTMPDeFragmentDataFrame() will release rx packet, 
			   if packet is fragmented
			 */
			return;
		}
	}
#ifdef XLINK_SUPPORT
	else if (pAd->StaCfg.PSPXlink) {
		Indicate_Legacy_Packet(pAd, pRxBlk, FromWhichBSSID);
		return;
	}
#endif /* XLINK_SUPPORT */

	RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);

	return;
}


#ifdef HDR_TRANS_SUPPORT
VOID STAHandleRxDataFrame_Hdr_Trns(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
	RXINFO_STRUC *pRxInfo = pRxBlk->pRxInfo;
	RXWI_STRUC *pRxWI = pRxBlk->pRxWI;
	HEADER_802_11 *pHeader = pRxBlk->pHeader;
	PNDIS_PACKET pRxPacket = pRxBlk->pRxPacket;
	BOOLEAN bFragment = FALSE;
	MAC_TABLE_ENTRY *pEntry = NULL;
	UCHAR FromWhichBSSID = BSS0;
	UCHAR UserPriority = 0;
	UCHAR *pData;

//+++Add by shiang for debug
//---Add by shiangf for debug

	if ((pHeader->FC.FrDs == 1) && (pHeader->FC.ToDs == 1)) {
#ifdef CLIENT_WDS
			if ((pRxBlk->wcid < MAX_LEN_OF_MAC_TABLE)
			    && IS_ENTRY_CLIENT(&pAd->MacTab.Content[pRxBlk->wcid])) {
			RX_BLK_SET_FLAG(pRxBlk, fRX_WDS);
		} else
#endif /* CLIENT_WDS */
		{
			RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
			return;
		}
	}
	else
	{
#ifdef DOT11V_WNM_SUPPORT
		RTMP_SET_WNM_DMS(pRxPacket, 0);
		IS_WNM_DMS(pAd, pRxPacket, pHeader);
#endif /* DOT11V_WNM_SUPPORT */

#ifdef QOS_DLS_SUPPORT
		if (RTMPRcvFrameDLSCheck(pAd, pHeader, pRxBlk->MPDUtotalByteCnt, pRxBlk->pRxInfo))
			return;
#endif /* QOS_DLS_SUPPORT */

		/* Drop not my BSS frames */
		if (pRxInfo->MyBss == 0) {
			{
				RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
				return;
			}
		}


		pAd->RalinkCounters.RxCountSinceLastNULL++;
		if (pAd->StaCfg.UapsdInfo.bAPSDCapable
		    && pAd->CommonCfg.APEdcaParm.bAPSDCapable
		    && (pHeader->FC.SubType & 0x08))
		{
			DBGPRINT(RT_DEBUG_INFO, ("bAPSDCapable\n"));

			/* Qos bit 4 */
			pData = (PUCHAR) pHeader + LENGTH_802_11;
			if ((*pData >> 4) & 0x01)
			{
				DBGPRINT(RT_DEBUG_INFO,
					 ("RxDone- Rcv EOSP frame, driver may fall into sleep\n"));
				pAd->CommonCfg.bInServicePeriod = FALSE;

				/* Force driver to fall into sleep mode when rcv EOSP frame */
				if (!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE))
				{
#ifdef RTMP_MAC_PCI
					USHORT TbttNumToNextWakeUp;
					USHORT NextDtim = pAd->StaCfg.DtimPeriod;
					ULONG Now;

					NdisGetSystemUpTime(&Now);
					NextDtim -= (USHORT) (Now - pAd->StaCfg.LastBeaconRxTime) / pAd->CommonCfg.BeaconPeriod;

					TbttNumToNextWakeUp = pAd->StaCfg.DefaultListenCount;
					if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_RECEIVE_DTIM) && (TbttNumToNextWakeUp > NextDtim))
						TbttNumToNextWakeUp = NextDtim;

					RTMP_SET_PSM_BIT(pAd, PWR_SAVE);
					/* if WMM-APSD is failed, try to disable following line */
					AsicSleepThenAutoWakeup(pAd, TbttNumToNextWakeUp);
#endif /* RTMP_MAC_PCI */

				}
			}

			if ((pHeader->FC.MoreData)
			    && (pAd->CommonCfg.bInServicePeriod)) {
				DBGPRINT(RT_DEBUG_TRACE,
					 ("Sending another trigger frame when More Data bit is set to 1\n"));
			}
		}

		/* Drop NULL, CF-ACK(no data), CF-POLL(no data), and CF-ACK+CF-POLL(no data) data frame */
		if ((pHeader->FC.SubType & 0x04)) /* bit 2 : no DATA */
		{
			RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
			return;
		}

		if (pAd->StaCfg.BssType == BSS_INFRA) {
			/* Infrastructure mode, check address 2 for BSSID */
			if (1
#ifdef QOS_DLS_SUPPORT
			    && (!pAd->CommonCfg.bDLSCapable)
#endif /* QOS_DLS_SUPPORT */
#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
			    && (!IS_TDLS_SUPPORT(pAd))
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */
			    ) {
				if (!RTMPEqualMemory(&pHeader->Addr2, &pAd->MlmeAux.Bssid, 6))
				{
					/* Receive frame not my BSSID */
					RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
					return;
				}
			}
		}
		else
		{	/* Ad-Hoc mode or Not associated */

			/* Ad-Hoc mode, check address 3 for BSSID */
			if (!RTMPEqualMemory(&pHeader->Addr3, &pAd->CommonCfg.Bssid, 6)) {
				/* Receive frame not my BSSID */
				RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
				return;
			}
		}

		/*/ find pEntry */
		if (pRxBlk->wcid < MAX_LEN_OF_MAC_TABLE) {
			pEntry = &pAd->MacTab.Content[pRxBlk->wcid];

		} else {
			RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
			return;
		}

		/* infra or ad-hoc */
		if (pAd->StaCfg.BssType == BSS_INFRA) {
			RX_BLK_SET_FLAG(pRxBlk, fRX_INFRA);
#if defined(DOT11Z_TDLS_SUPPORT) || defined(QOS_DLS_SUPPORT)
			if ((pHeader->FC.FrDs == 0) && (pHeader->FC.ToDs == 0))
				RX_BLK_SET_FLAG(pRxBlk, fRX_DLS);
			else
#endif
				ASSERT(pRxBlk->wcid == BSSID_WCID);
		}

#ifndef WFA_VHT_PF
		// TODO: shiang@PF#2, is this atheros protection still necessary here???
		/* check Atheros Client */
		if ((pEntry->bIAmBadAtheros == FALSE) && (pRxInfo->AMPDU == 1)
		    && (pHeader->FC.Retry)) {
			pEntry->bIAmBadAtheros = TRUE;
			pAd->CommonCfg.IOTestParm.bLastAtheros = TRUE;
			if (!STA_AES_ON(pAd))
				RTMP_UPDATE_PROTECT(pAd, 8 , ALLN_SETPROTECT, TRUE, FALSE);
		}
#endif /* WFA_VHT_PF */
	}


#ifdef RLT_MAC
#ifdef CONFIG_RX_CSO_SUPPORT
	if (RTMP_TEST_MORE_FLAG(pAd, fRTMP_ADAPTER_RX_CSO_SUPPORT))
	{
		RXFCE_INFO *pRxFceInfo = pRxBlk->pRxFceInfo;

		if ( pRxFceInfo->l3l4_done )
		{


/*
			if ( (pRxFceInfo->ip_err) || (pRxFceInfo->tcp_err)
				|| (pRxFceInfo->udp_err) ) {
				RTMP_SET_TCP_CHKSUM_FAIL(pRxPacket, TRUE);
			}
*/
			// Linux always do IP header chksum
			if (  (pRxFceInfo->tcp_err) || (pRxFceInfo->udp_err) ) {
				RTMP_SET_TCP_CHKSUM_FAIL(pRxPacket, TRUE);
			}
		}
	}
#endif /* CONFIG_RX_CSO_SUPPORT */
#endif /* RLT_MAC */

	pData = (UCHAR *) pHeader;

	/*
	   update RxBlk->pData, DataSize
	   802.11 Header, QOS, HTC, Hw Padding
	 */
	/* 1. skip 802.11 HEADER */
#ifdef CLIENT_WDS
	if (RX_BLK_TEST_FLAG(pRxBlk, fRX_WDS)) {
		pData += LENGTH_802_11_WITH_ADDR4;
	} else
#endif /* CLIENT_WDS */
	{
		pData += LENGTH_802_11;
	}

	/* 2. QOS */
	if (pHeader->FC.SubType & 0x08) {
		RX_BLK_SET_FLAG(pRxBlk, fRX_QOS);
		UserPriority = *(pData) & 0x0f;
		/* bit 7 in QoS Control field signals the HT A-MSDU format */
		if ((*pData) & 0x80) {
			RX_BLK_SET_FLAG(pRxBlk, fRX_AMSDU);
		}

		/* skip QOS contorl field */
		pData += 2;
	}
	pRxBlk->UserPriority = UserPriority;

	/* check if need to resend PS Poll when received packet with MoreData = 1 */
	if ((pAd->StaCfg.Psm == PWR_SAVE) && (pHeader->FC.MoreData == 1)) {
		if ((((UserPriority == 0) || (UserPriority == 3)) &&
		     pAd->CommonCfg.bAPSDAC_BE == 0) ||
		    (((UserPriority == 1) || (UserPriority == 2)) &&
		     pAd->CommonCfg.bAPSDAC_BK == 0) ||
		    (((UserPriority == 4) || (UserPriority == 5)) &&
		     pAd->CommonCfg.bAPSDAC_VI == 0) ||
		    (((UserPriority == 6) || (UserPriority == 7)) &&
		     pAd->CommonCfg.bAPSDAC_VO == 0)) {
			/* non-UAPSD delivery-enabled AC */
			RTMP_PS_POLL_ENQUEUE(pAd);
		}
	}

	/* 3. Order bit: A-Ralink or HTC+ */
	if (pHeader->FC.Order) {
#ifdef AGGREGATION_SUPPORT
		if ((pRxBlk->rx_rate.field.MODE <= MODE_OFDM)
		    && (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_AGGREGATION_INUSED)))
		{
			RX_BLK_SET_FLAG(pRxBlk, fRX_ARALINK);
		} else
#endif /* AGGREGATION_SUPPORT */
		{
#ifdef DOT11_N_SUPPORT
			RX_BLK_SET_FLAG(pRxBlk, fRX_HTC);
			/* skip HTC contorl field */
			pData += 4;
#endif /* DOT11_N_SUPPORT */
		}
	}

	/* 4. skip HW padding */
	if (pRxInfo->L2PAD) {
		/* just move pData pointer, because DataSize excluding HW padding */
		RX_BLK_SET_FLAG(pRxBlk, fRX_PAD);
		pData += 2;
	}
#ifdef DOT11_N_SUPPORT
	if (pRxInfo->BA) {
		RX_BLK_SET_FLAG(pRxBlk, fRX_AMPDU);
	}
#endif /* DOT11_N_SUPPORT */

#if defined(SOFT_ENCRYPT) || defined(ADHOC_WPA2PSK_SUPPORT)
	/* Use software to decrypt the encrypted frame if necessary.
	   If a received "encrypted" unicast packet(its WEP bit as 1) 
	   and it's passed to driver with "Decrypted" marked as 0 in pRxInfo. */
	if ((pHeader->FC.Wep == 1) && (pRxInfo->Decrypted == 0)) {
		PCIPHER_KEY pSwKey = RTMPSwCipherKeySelection(pAd,
						       pRxBlk->pData, pRxBlk,
						       pEntry);

		/* Cipher key table selection */
		if (!pSwKey) {
			DBGPRINT(RT_DEBUG_TRACE, ("No vaild cipher key for SW decryption!!!\n"));
			RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
			return;
		}

		/* Decryption by Software */
		if (RTMPSoftDecryptionAction(pAd,
					     (PUCHAR) pHeader,
					     UserPriority,
					     pSwKey,
					     pRxBlk->pTransData + 14,
					     &(pRxBlk->TransDataSize)) !=
		    NDIS_STATUS_SUCCESS) {
			RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
			return;
		}
		/* Record the Decrypted bit as 1 */
		pRxInfo->Decrypted = 1;
	}
#endif /* SOFT_ENCRYPT || ADHOC_WPA2PSK_SUPPORT */

	/* Case I  Process Broadcast & Multicast data frame */
	if (pRxInfo->Bcast || pRxInfo->Mcast) {
#ifdef STATS_COUNT_SUPPORT
		INC_COUNTER64(pAd->WlanCounters.MulticastReceivedFrameCount);
#endif /* STATS_COUNT_SUPPORT */

		/* Drop Mcast/Bcast frame with fragment bit on */
		if (pHeader->FC.MoreFrag) {
			RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
			return;
		}

		/* Filter out Bcast frame which AP relayed for us */
		if (pHeader->FC.FrDs
		    && MAC_ADDR_EQUAL(pHeader->Addr3, pAd->CurrentAddress)) {
			RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
			return;
		}

		if (ADHOC_ON(pAd)) {
			MAC_TABLE_ENTRY *pAdhocEntry = NULL;
			pAdhocEntry = MacTableLookup(pAd, pHeader->Addr2);
			if (pAdhocEntry)
				Update_Rssi_Sample(pAd, &pAdhocEntry->RssiSample, pRxWI);
		}

		Indicate_Legacy_Packet_Hdr_Trns(pAd, pRxBlk, FromWhichBSSID);
		return;
	}
	else if (pRxInfo->U2M)
	{
		pAd->LastRxRate = (ULONG)(pRxBlk->rx_rate.word);

#if defined(DOT11Z_TDLS_SUPPORT) || defined(QOS_DLS_SUPPORT)
		if (RX_BLK_TEST_FLAG(pRxBlk, fRX_DLS)) {
			MAC_TABLE_ENTRY *pDlsEntry = &pAd->MacTab.Content[pRxBlk->wcid];

			if (pDlsEntry && (pRxBlk->wcid < MAX_LEN_OF_MAC_TABLE)) {
				Update_Rssi_Sample(pAd, &pDlsEntry->RssiSample, pRxWI);
				NdisAcquireSpinLock(&pAd->MacTabLock);
				pDlsEntry->NoDataIdleCount = 0;
				NdisReleaseSpinLock(&pAd->MacTabLock);
			}
		}
		else
#endif
		if (ADHOC_ON(pAd)) {
			MAC_TABLE_ENTRY *pAdhocEntry = NULL;
			pAdhocEntry = MacTableLookup(pAd, pHeader->Addr2);
			if (pAdhocEntry)
				Update_Rssi_Sample(pAd, &pAdhocEntry->RssiSample, pRxWI);
		}

		Update_Rssi_Sample(pAd, &pAd->StaCfg.RssiSample, pRxWI);

		pAd->StaCfg.LastSNR0 = (UCHAR) (pRxBlk->snr[0]);
		pAd->StaCfg.LastSNR1 = (UCHAR) (pRxBlk->snr[1]);
#ifdef DOT11N_SS3_SUPPORT
		if (pAd->CommonCfg.RxStream == 3)
			pAd->StaCfg.LastSNR2 = (UCHAR) (pRxBlk->snr[2]);
#endif /* DOT11N_SS3_SUPPORT */

		pAd->RalinkCounters.OneSecRxOkDataCnt++;

		if (pEntry != NULL)
		{
			pEntry->LastRxRate = pAd->LastRxRate;
#ifdef TXBF_SUPPORT
			if (pRxWI->ShortGI)
				pEntry->OneSecRxSGICount++;
			else
				pEntry->OneSecRxLGICount++;
#endif /* TXBF_SUPPORT */

			pEntry->freqOffset = (CHAR)(pRxBlk->freq_offset);
			pEntry->freqOffsetValid = TRUE;

		}

#ifdef PRE_ANT_SWITCH
#endif /* PRE_ANT_SWITCH */

		/* there's packet sent to me, keep awake for 1200ms */
		if (pAd->CountDowntoPsm < 12)
			pAd->CountDowntoPsm = 12;

		if (!((pHeader->Frag == 0) && (pHeader->FC.MoreFrag == 0))) {
			/* re-assemble the fragmented packets */
			/* return complete frame (pRxPacket) or NULL */
			bFragment = TRUE;
			pRxPacket = RTMPDeFragmentDataFrame(pAd, pRxBlk);
		}

		if (pRxPacket) {
			pEntry = &pAd->MacTab.Content[pRxBlk->wcid];

			/* process complete frame */
			if (bFragment && (pRxInfo->Decrypted)
			    && (pEntry->WepStatus == Ndis802_11TKIPEnable)) {
				/* Minus MIC length */
				pRxBlk->DataSize -= 8;

				/* For TKIP frame, calculate the MIC value */
				if (STACheckTkipMICValue(pAd, pEntry, pRxBlk) == FALSE) {
					return;
				}
			}

			STARxDataFrameAnnounce_Hdr_Trns(pAd, pEntry, pRxBlk, FromWhichBSSID);
			return;
		} else {
			/*
			   just return because RTMPDeFragmentDataFrame() will release rx packet, 
			   if packet is fragmented
			 */
			return;
		}
	}
#ifdef XLINK_SUPPORT
	else if (pAd->StaCfg.PSPXlink) {
		Indicate_Legacy_Packet(pAd, pRxBlk, FromWhichBSSID);
		return;
	}
#endif /* XLINK_SUPPORT */

	RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);

	return;
}
#endif /* HDR_TRANS_SUPPORT */


/*
	========================================================================

	Routine Description:
	Arguments:
		pAd 	Pointer to our adapter

	IRQL = DISPATCH_LEVEL
	
	========================================================================
*/
VOID RTMPHandleTwakeupInterrupt(
	IN PRTMP_ADAPTER pAd)
{
	AsicForceWakeup(pAd, FALSE);
}


/*
========================================================================
Routine Description:
	This routine is used to do packet parsing and classification for Tx packet 
	to STA device, and it will en-queue packets to our TxSwQ depends on AC 
	class.
	
Arguments:
	pAd    		Pointer to our adapter
	pPacket 	Pointer to send packet

Return Value:
	NDIS_STATUS_SUCCESS		If succes to queue the packet into TxSwQ.
	NDIS_STATUS_FAILURE			If failed to do en-queue.

Note:
	You only can put OS-indepened & STA related code in here.
========================================================================
*/
INT STASendPacket(RTMP_ADAPTER *pAd, PNDIS_PACKET pPacket)
{
	PACKET_INFO PacketInfo;
	UCHAR *pSrcBufVA;
	UINT SrcBufLen, AllowFragSize;
	UCHAR NumberOfFrag;
	UCHAR QueIdx;
	UCHAR UserPriority;
	UCHAR Wcid;
	unsigned long IrqFlags;
	MAC_TABLE_ENTRY *pMacEntry = NULL;
	struct wifi_dev *wdev;


	RTMP_QueryPacketInfo(pPacket, &PacketInfo, &pSrcBufVA, &SrcBufLen);
	if ((pSrcBufVA == NULL) || (SrcBufLen <= 14))
	{
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
		DBGPRINT(RT_DEBUG_ERROR, ("%s():pkt error(%p, %d)\n",
					__FUNCTION__, pSrcBufVA, SrcBufLen));
		return NDIS_STATUS_FAILURE;
	}

	Wcid = RTMP_GET_PACKET_WCID(pPacket);
	/* In HT rate adhoc mode, A-MPDU is often used. So need to lookup BA Table and MAC Entry. */
	/* Note multicast packets in adhoc also use BSSID_WCID index. */
	{
		if (pAd->StaCfg.BssType == BSS_INFRA) {
#if defined(QOS_DLS_SUPPORT) || defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
			if (VALID_WCID(Wcid) &&
			    (IS_ENTRY_DLS(&pAd->MacTab.Content[Wcid]) ||
			     IS_ENTRY_TDLS(&pAd->MacTab.Content[Wcid]))) {
			    DBGPRINT(RT_DEBUG_TRACE, ("assigning TDLS entry \n", __FUNCTION__));
				pMacEntry = &pAd->MacTab.Content[Wcid];
			} else
#endif /*defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)*/
			{
				pMacEntry = &pAd->MacTab.Content[BSSID_WCID];
				RTMP_SET_PACKET_WCID(pPacket, BSSID_WCID);
			}
		} else if (ADHOC_ON(pAd)) {
			if (*pSrcBufVA & 0x01) {
				RTMP_SET_PACKET_WCID(pPacket, MCAST_WCID);
				pMacEntry = &pAd->MacTab.Content[MCAST_WCID];
			} else {
#ifdef XLINK_SUPPORT
				if (pAd->StaCfg.PSPXlink) {
					pMacEntry = &pAd->MacTab.Content[MCAST_WCID];
					pMacEntry->Aid = MCAST_WCID;
				} else
#endif /* XLINK_SUPPORT */
					pMacEntry = MacTableLookup(pAd, pSrcBufVA);

				if (pMacEntry)
					RTMP_SET_PACKET_WCID(pPacket, pMacEntry->wcid);
#ifdef ETH_CONVERT_SUPPORT
				else {
					RTMP_SET_PACKET_WCID(pPacket, MCAST_WCID);
				}
#endif /* ETH_CONVERT_SUPPORT */
			}
		}
	}

	if (!pMacEntry) {
		DBGPRINT(RT_DEBUG_ERROR,
			 ("%s():No such Addr(%2x:%2x:%2x:%2x:%2x:%2x) in MacTab\n",
			 __FUNCTION__, PRINT_MAC(pSrcBufVA)));
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
		return NDIS_STATUS_FAILURE;
	}

	if (ADHOC_ON(pAd)
	    ) {
		RTMP_SET_PACKET_WCID(pPacket, (UCHAR) pMacEntry->wcid);
	}

	wdev = &pAd->StaCfg.wdev;

	/* Check the Ethernet Frame type of this packet, and set the RTMP_SET_PACKET_SPECIFIC flags. */
	/*              Here we set the PACKET_SPECIFIC flags(LLC, VLAN, DHCP/ARP, EAPOL). */
	UserPriority = 0;
	QueIdx = QID_AC_BE;
	RTMPCheckEtherType(pAd, pPacket, pMacEntry, wdev, &UserPriority, &QueIdx);

#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
#ifdef UAPSD_SUPPORT
	/* the code must behind RTMPCheckEtherType() to get QueIdx */
	if ((pMacEntry != NULL) &&
		(pMacEntry->PsMode == PWR_SAVE) &&
		(pMacEntry->FlgPsModeIsWakeForAWhile == FALSE) &&
		(pMacEntry->bAPSDDeliverEnabledPerAC[QueIdx] == 0))
	{
		/* the packet will be sent to AP, not the peer directly */
		pMacEntry = &pAd->MacTab.Content[BSSID_WCID];

		/* for AP entry, pEntry->bAPSDDeliverEnabledPerAC[QueIdx] always is 0 */
		RTMP_SET_PACKET_WCID(pPacket, BSSID_WCID);
	}
#endif /* UAPSD_SUPPORT */
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */

#ifdef WAPI_SUPPORT
	/* Check the status of the controlled port and filter the outgoing frame for WAPI */
	if (((wdev->AuthMode == Ndis802_11AuthModeWAICERT) ||
	     (wdev->AuthMode == Ndis802_11AuthModeWAIPSK))
	    && (wdev->PortSecured == WPA_802_1X_PORT_NOT_SECURED)
	    && (RTMP_GET_PACKET_WAI(pPacket) == FALSE)
	    ) {
		DBGPRINT(RT_DEBUG_TRACE,
			 ("%s():Drop packet before port secured!\n", __FUNCTION__));
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);

		return NDIS_STATUS_FAILURE;
	}
#endif /* WAPI_SUPPORT */

	/* WPA 802.1x secured port control - drop all non-802.1x frame before port secured */
	if (((wdev->AuthMode == Ndis802_11AuthModeWPA) ||
	     (wdev->AuthMode == Ndis802_11AuthModeWPAPSK) ||
	     (wdev->AuthMode == Ndis802_11AuthModeWPA2) ||
	     (wdev->AuthMode == Ndis802_11AuthModeWPA2PSK)
#ifdef WPA_SUPPLICANT_SUPPORT
	     || (pAd->StaCfg.wdev.IEEE8021X == TRUE)
#endif /* WPA_SUPPLICANT_SUPPORT */
	    )
	    && ((wdev->PortSecured == WPA_802_1X_PORT_NOT_SECURED)
		|| (pAd->StaCfg.MicErrCnt >= 2))
	    && (RTMP_GET_PACKET_EAPOL(pPacket) == FALSE)
	    ) {
		DBGPRINT(RT_DEBUG_TRACE,
			 ("%s():Drop packet before port secured!\n", __FUNCTION__));
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);

		return NDIS_STATUS_FAILURE;
	}
#ifdef WSC_STA_SUPPORT
	if ((pAd->StaCfg.WscControl.WscConfMode != WSC_DISABLE) &&
	    (pAd->StaCfg.WscControl.bWscTrigger == TRUE) &&
	    (RTMP_GET_PACKET_EAPOL(pPacket) == FALSE)) {
		DBGPRINT(RT_DEBUG_TRACE,
			 ("%s():Drop packet before WPS process completed!\n", __FUNCTION__));
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);

		return NDIS_STATUS_FAILURE;
	}
#endif /* WSC_STA_SUPPORT */

	/* 
		STEP 1. Decide number of fragments required to deliver this MSDU.
			The estimation here is not very accurate because difficult to
			take encryption overhead into consideration here. The result
			"NumberOfFrag" is then just used to pre-check if enough free
			TXD are available to hold this MSDU.
	*/
	if (*pSrcBufVA & 0x01)	/* fragmentation not allowed on multicast & broadcast */
		NumberOfFrag = 1;
	else if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_AGGREGATION_INUSED))
		NumberOfFrag = 1;	/* Aggregation overwhelms fragmentation */
	else if (CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_AMSDU_INUSED))
		NumberOfFrag = 1;	/* Aggregation overwhelms fragmentation */
#ifdef DOT11_N_SUPPORT
	else if ((wdev->HTPhyMode.field.MODE == MODE_HTMIX)
		 || (wdev->HTPhyMode.field.MODE == MODE_HTGREENFIELD))
		NumberOfFrag = 1;	/* MIMO RATE overwhelms fragmentation */
#endif /* DOT11_N_SUPPORT */
	else
	{
		/*
			The calculated "NumberOfFrag" is a rough estimation because of various
			encryption/encapsulation overhead not taken into consideration. This number is just
			used to make sure enough free TXD are available before fragmentation takes place.
			In case the actual required number of fragments of an NDIS packet
			excceeds "NumberOfFrag"caculated here and not enough free TXD available, the
			last fragment (i.e. last MPDU) will be dropped in RTMPHardTransmit() due to out of
			resource, and the NDIS packet will be indicated NDIS_STATUS_FAILURE. This should
			rarely happen and the penalty is just like a TX RETRY fail. Affordable.
		*/
		UINT32 Size;

		AllowFragSize = (pAd->CommonCfg.FragmentThreshold) - LENGTH_802_11 - LENGTH_CRC;
		Size = PacketInfo.TotalPacketLength - LENGTH_802_3 + LENGTH_802_1_H;
		NumberOfFrag = (Size / AllowFragSize) + 1;
		/* To get accurate number of fragmentation, Minus 1 if the size just match to allowable fragment size */
		if ((Size % AllowFragSize) == 0) {
			NumberOfFrag--;
		}
	}
	/* Save fragment number to Ndis packet reserved field */
	RTMP_SET_PACKET_FRAGMENTS(pPacket, NumberOfFrag);

{
	BOOLEAN RTSRequired;

	/*
		STEP 2. Check the requirement of RTS; decide packet TX rate
		If multiple fragment required, RTS is required only for the first fragment
		if the fragment size large than RTS threshold
	 */
	if (NumberOfFrag > 1)
		RTSRequired = (pAd->CommonCfg.FragmentThreshold > pAd->CommonCfg.RtsThreshold) ? 1 : 0;
	else
		RTSRequired = (PacketInfo.TotalPacketLength > pAd->CommonCfg.RtsThreshold) ? 1 : 0;
	RTMP_SET_PACKET_RTS(pPacket, RTSRequired);
}

	RTMP_SET_PACKET_UP(pPacket, UserPriority);

#ifdef ETH_CONVERT_SUPPORT
	if (pAd->StaCfg.bFragFlag == TRUE) {
		static QUEUE_HEADER queue1[4], queue2[4];
		static BOOLEAN is_init = 0;
		static INT32 ID1[4] = { -1, -1, -1, -1 }, ID2[4] = {-1, -1, -1, -1};
		static INT32 ID1_FragSize[4] = { -1, -1, -1, -1 }, ID2_FragSize[4] = {-1, -1, -1, -1};
		static ULONG jiffies1[4], jiffies2[4];

		QUEUE_HEADER *pAC_Queue1, *pAC_Queue2;
		INT32 *pAC_ID1, *pAC_ID2, *pAC_ID1_FragSize, *pAC_ID2_FragSize;
		ULONG *pAC_Jiffies1, *pAC_Jiffies2;

		UINT32 i;
		typedef union ip_flags_frag_offset {
			struct {
#ifdef RT_BIG_ENDIAN
				USHORT flags_reserved:1;
				USHORT flags_may_frag:1;
				USHORT flags_more_frag:1;
				USHORT frag_offset:13;
#else
				USHORT frag_offset:13;
				USHORT flags_more_frag:1;
				USHORT flags_may_frag:1;
				USHORT flags_reserved:1;
#endif
			} field;
			USHORT word;
		} IP_FLAGS_FRAG_OFFSET;

		typedef struct ip_v4_hdr {
#ifdef RT_BIG_ENDIAN
			UCHAR version:4, ihl:4;
#else
			UCHAR ihl:4, version:4;
#endif
			UCHAR tos;
			USHORT tot_len;
			USHORT identifier;
		} IP_V4_HDR;

		IP_V4_HDR *pIpv4Hdr, Ipv4Hdr;
		IP_FLAGS_FRAG_OFFSET *pFlags_frag_offset, Flags_frag_offset;
		ULONG Now;

		if (is_init == 0) {
			for (i = 0; i < 4; i++) {
				InitializeQueueHeader(&queue1[i]);
				InitializeQueueHeader(&queue2[i]);
			}
			is_init = 1;
		}

		pAC_Queue1 = &queue1[QueIdx];
		pAC_Queue2 = &queue2[QueIdx];
		pAC_ID1 = &ID1[QueIdx];
		pAC_ID2 = &ID2[QueIdx];
		pAC_ID1_FragSize = &ID1_FragSize[QueIdx];
		pAC_ID2_FragSize = &ID2_FragSize[QueIdx];
		pAC_Jiffies1 = &jiffies1[QueIdx];
		pAC_Jiffies2 = &jiffies2[QueIdx];

		NdisGetSystemUpTime(&Now);
		if ((pAC_Queue1->Number != 0)
		    && (RTMP_TIME_AFTER(Now, (*pAC_Jiffies1) + (1000 * OS_HZ))))
		{
			PQUEUE_ENTRY pBackupPktEntry;
			PNDIS_PACKET pBackupPkt;

			while (1) {
				pBackupPktEntry = RemoveHeadQueue(pAC_Queue1);
				if (pBackupPktEntry == NULL)
					break;
				pBackupPkt = QUEUE_ENTRY_TO_PACKET(pBackupPktEntry);
				RELEASE_NDIS_PACKET(pAd, pBackupPkt, NDIS_STATUS_FAILURE);
			}

			*pAC_ID1 = -1;
			*pAC_ID1_FragSize = -1;
			*pAC_Jiffies1 = 0;
		}

		NdisGetSystemUpTime(&Now);
		if ((pAC_Queue2->Number != 0)
		    && (RTMP_TIME_AFTER(Now, (*pAC_Jiffies2) + (1000 * OS_HZ))))
		{
			PQUEUE_ENTRY pBackupPktEntry;
			PNDIS_PACKET pBackupPkt;

			while (1) {
				pBackupPktEntry = RemoveHeadQueue(pAC_Queue2);
				if (pBackupPktEntry == NULL)
					break;
				pBackupPkt = QUEUE_ENTRY_TO_PACKET(pBackupPktEntry);
				RELEASE_NDIS_PACKET(pAd, pBackupPkt, NDIS_STATUS_FAILURE);
			}

			*pAC_ID2 = -1;
			*pAC_ID2_FragSize = -1;
			*pAC_Jiffies2 = 0;
		}

		if (RTMP_GET_PACKET_IPV4(pPacket) &&
		    ((pAd->TxSwQueue[QueIdx].Number >= (pAd->TxSwQMaxLen >> 1)) ||
				((pAC_Queue1->Number) || (pAC_Queue2->Number)))
		    ) {
			pIpv4Hdr = (IP_V4_HDR *) (PacketInfo.pFirstBuffer + LENGTH_802_3);
			pFlags_frag_offset =
			    (IP_FLAGS_FRAG_OFFSET *) (PacketInfo.pFirstBuffer + LENGTH_802_3 + 6);
			Flags_frag_offset.word = ntohs(pFlags_frag_offset->word);
			Ipv4Hdr.identifier = ntohs(pIpv4Hdr->identifier);
			Ipv4Hdr.tot_len = ntohs(pIpv4Hdr->tot_len);
			Ipv4Hdr.ihl = pIpv4Hdr->ihl;

			/* check if 1st fragment */
			if ((Flags_frag_offset.field.flags_more_frag == 1) &&
			    (Flags_frag_offset.field.frag_offset == 0)) {
				NdisGetSystemUpTime(&Now);
				if ((*pAC_ID1) == -1) {
					*pAC_ID1 = Ipv4Hdr.identifier;
					*pAC_Jiffies1 = Now;
					*pAC_ID1_FragSize = Ipv4Hdr.tot_len - (Ipv4Hdr.ihl * 4);
					InsertTailQueue(pAC_Queue1, PACKET_TO_QUEUE_ENTRY(pPacket));
				} else if ((*pAC_ID2) == -1) {
					*pAC_ID2 = Ipv4Hdr.identifier;
					*pAC_Jiffies2 = Now;
					*pAC_ID2_FragSize = Ipv4Hdr.tot_len - (Ipv4Hdr.ihl * 4);
					InsertTailQueue(pAC_Queue2, PACKET_TO_QUEUE_ENTRY(pPacket));
				} else {
					QUEUE_HEADER *pTempqueue;
					PQUEUE_ENTRY pBackupPktEntry;
					PNDIS_PACKET pBackupPkt;

					/* free backup fragments */
					if ((*pAC_Jiffies1) > (*pAC_Jiffies2)) {
						pTempqueue = pAC_Queue2;
						*pAC_ID2 = Ipv4Hdr.identifier;
					} else {
						pTempqueue = pAC_Queue1;
						*pAC_ID1 = Ipv4Hdr.identifier;
					}

					if (pTempqueue->Number != 0) {
						while (1) {
							pBackupPktEntry = RemoveHeadQueue(pTempqueue);
							if (pBackupPktEntry == NULL)
								break;
							pBackupPkt = QUEUE_ENTRY_TO_PACKET(pBackupPktEntry);
							RELEASE_NDIS_PACKET(pAd, pBackupPkt, NDIS_STATUS_FAILURE);
						}
					}
				}
			} else {
				/* check if last fragment */
				if ((Ipv4Hdr.identifier == (*pAC_ID1))
				    || (Ipv4Hdr.identifier == (*pAC_ID2))) {
					QUEUE_HEADER *pTempqueue;
					PQUEUE_ENTRY pBackupPktEntry;
					PNDIS_PACKET pBackupPkt;

					if (Ipv4Hdr.identifier == (*pAC_ID1)) {
						InsertTailQueue(pAC_Queue1, PACKET_TO_QUEUE_ENTRY(pPacket));
						pTempqueue = pAC_Queue1;
					} else {
						InsertTailQueue(pAC_Queue2, PACKET_TO_QUEUE_ENTRY(pPacket));
						pTempqueue = pAC_Queue2;
					}

					/* the last fragment */
					if ((Flags_frag_offset.field.flags_more_frag == 0)
					    && (Flags_frag_offset.field.frag_offset != 0)) {
						UINT32 fragment_count = 0;
						BOOLEAN bDrop = FALSE;
						if (Ipv4Hdr.identifier == (*pAC_ID1)) {
							fragment_count = ((Flags_frag_offset.field.frag_offset * 8) / (*pAC_ID1_FragSize)) + 1;
							if (pTempqueue->Number != fragment_count)
								bDrop = TRUE;
							*pAC_ID1 = -1;
						}

						if (Ipv4Hdr.identifier == (*pAC_ID2)) {
							fragment_count = ((Flags_frag_offset.field.frag_offset * 8) / (*pAC_ID2_FragSize)) + 1;
							if (pTempqueue->Number != fragment_count)
								bDrop = TRUE;
							*pAC_ID2 = -1;
						}

						/* if number does not equal coreect fragment count or no enough space, */
						/* free backup fragments to SwTxQueue[] */
						if ((bDrop == TRUE) ||
						    (pAd->TxSwQueue[QueIdx].Number > (pAd->TxSwQMaxLen - pTempqueue->Number))) {
							while (1) {
								pBackupPktEntry = RemoveHeadQueue(pTempqueue);
								if (pBackupPktEntry == NULL)
									break;
								pBackupPkt = QUEUE_ENTRY_TO_PACKET(pBackupPktEntry);
								RELEASE_NDIS_PACKET(pAd, pBackupPkt, NDIS_STATUS_FAILURE);
							}
							return
							    NDIS_STATUS_SUCCESS;
						}

						/* move backup fragments to SwTxQueue[] */
						NdisAcquireSpinLock(&pAd->TxSwQueueLock[QueIdx]);
						while (1) {
							pBackupPktEntry = RemoveHeadQueue(pTempqueue);
							if (pBackupPktEntry == NULL)
								break;
							InsertTailQueueAc(pAd, pMacEntry, &pAd->TxSwQueue[QueIdx], pBackupPktEntry);
						}
						NdisReleaseSpinLock(&pAd->TxSwQueueLock[QueIdx]);
					}
				} else {

					/*
					   bypass none-fist fragmented packets (we should not drop this packet)
					   when 
					   1. (pAd->TxSwQueue[QueIdx].Number >= (pAd->TxSwQMaxLen[QueIdx] >> 1)
					   and 
					   2. two fragmented buffer are empty 
					 */
					if (((*pAC_ID1) == -1)
					    && ((*pAC_ID2) == -1)) {
						goto normal_drop;
					}

					if ((Flags_frag_offset.field.flags_more_frag != 0)
					    && (Flags_frag_offset.field.frag_offset != 0)) {
						NdisAcquireSpinLock(&pAd->TxSwQueueLock[QueIdx]);
						RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
						NdisReleaseSpinLock(&pAd->TxSwQueueLock[QueIdx]);
						return NDIS_STATUS_SUCCESS;
					} else {
						goto normal_drop;
					}
				}
			}
		} else
			goto normal_drop;
	}
	else
	{
	      normal_drop:
#endif /* ETH_CONVERT_SUPPORT */

#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
#ifdef UAPSD_SUPPORT
	if ((pMacEntry != NULL) &&
		(pMacEntry->PsMode == PWR_SAVE) &&
		(pMacEntry->FlgPsModeIsWakeForAWhile == FALSE) &&
		(pMacEntry->bAPSDDeliverEnabledPerAC[QueIdx] != 0))
	{
		/*
			only UAPSD of the peer for the queue is set;
			for non-UAPSD queue, we will send it to the AP.
		*/
		
		if (RtmpInsertPsQueue(pAd, pPacket, pMacEntry, QueIdx) != NDIS_STATUS_SUCCESS)
			return NDIS_STATUS_FAILURE;
	}
	else
		/* non-PS mode or send PS packet to AP */
#endif /* UAPSD_SUPPORT */
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */
	{
		/* Make sure SendTxWait queue resource won't be used by other threads */
		RTMP_IRQ_LOCK(&pAd->irq_lock, IrqFlags);
		if (pAd->TxSwQueue[QueIdx].Number >= pAd->TxSwQMaxLen) {
			RTMP_IRQ_UNLOCK(&pAd->irq_lock, IrqFlags);
#ifdef BLOCK_NET_IF
			StopNetIfQueue(pAd, QueIdx, pPacket);
#endif /* BLOCK_NET_IF */
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);

			return NDIS_STATUS_FAILURE;
		} else {
				InsertTailQueueAc(pAd, pMacEntry, &pAd->TxSwQueue[QueIdx], PACKET_TO_QUEUE_ENTRY(pPacket));
		}
		RTMP_IRQ_UNLOCK(&pAd->irq_lock, IrqFlags);
	}
#ifdef ETH_CONVERT_SUPPORT
	}
#endif /* ETH_CONVERT_SUPPORT */

#ifdef DOT11_N_SUPPORT
	RTMP_BASetup(pAd, pMacEntry, UserPriority);
#endif /* DOT11_N_SUPPORT */

	pAd->RalinkCounters.OneSecOsTxCount[QueIdx]++;	/* TODO: for debug only. to be removed */
	return NDIS_STATUS_SUCCESS;
}


/*
	========================================================================

	Routine Description:
		This subroutine will scan through releative ring descriptor to find
		out avaliable free ring descriptor and compare with request size.
		
	Arguments:
		pAd Pointer to our adapter
		QueIdx		Selected TX Ring
		
	Return Value:
		NDIS_STATUS_FAILURE 	Not enough free descriptor
		NDIS_STATUS_SUCCESS 	Enough free descriptor

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL
	
	Note:
	
	========================================================================
*/
#ifdef RTMP_MAC_PCI
NDIS_STATUS RTMPFreeTXDRequest(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR QueIdx,
	IN UCHAR NumberRequired,
	IN PUCHAR FreeNumberIs)
{
	ULONG FreeNumber = 0;
	NDIS_STATUS Status = NDIS_STATUS_FAILURE;

	switch (QueIdx) {
	case QID_AC_BK:
	case QID_AC_BE:
	case QID_AC_VI:
	case QID_AC_VO:
	case QID_HCCA:
		if (pAd->TxRing[QueIdx].TxSwFreeIdx >
		    pAd->TxRing[QueIdx].TxCpuIdx)
			FreeNumber =
			    pAd->TxRing[QueIdx].TxSwFreeIdx -
			    pAd->TxRing[QueIdx].TxCpuIdx - 1;
		else
			FreeNumber =
			    pAd->TxRing[QueIdx].TxSwFreeIdx + TX_RING_SIZE -
			    pAd->TxRing[QueIdx].TxCpuIdx - 1;

		if (FreeNumber >= NumberRequired)
			Status = NDIS_STATUS_SUCCESS;
		break;

	case QID_MGMT:
		if (pAd->MgmtRing.TxSwFreeIdx > pAd->MgmtRing.TxCpuIdx)
			FreeNumber =
			    pAd->MgmtRing.TxSwFreeIdx - pAd->MgmtRing.TxCpuIdx -
			    1;
		else
			FreeNumber =
			    pAd->MgmtRing.TxSwFreeIdx + MGMT_RING_SIZE -
			    pAd->MgmtRing.TxCpuIdx - 1;

		if (FreeNumber >= NumberRequired)
			Status = NDIS_STATUS_SUCCESS;
		break;

#ifdef CONFIG_ANDES_SUPPORT
	case QID_CTRL:
		if (pAd->CtrlRing.TxSwFreeIdx > pAd->CtrlRing.TxCpuIdx)
			FreeNumber =
			    pAd->CtrlRing.TxSwFreeIdx - pAd->CtrlRing.TxCpuIdx -
			    1;
		else
			FreeNumber =
			    pAd->CtrlRing.TxSwFreeIdx + MGMT_RING_SIZE -
			    pAd->CtrlRing.TxCpuIdx - 1;

		if (FreeNumber >= NumberRequired)
			Status = NDIS_STATUS_SUCCESS;
		break;
#endif /* CONFIG_ANDES_SUPPORT */

	default:
		DBGPRINT(RT_DEBUG_ERROR,
			 ("RTMPFreeTXDRequest::Invalid QueIdx(=%d)\n", QueIdx));
		break;
	}
	*FreeNumberIs = (UCHAR) FreeNumber;

	return (Status);
}
#endif /* RTMP_MAC_PCI */




VOID RTMPSendNullFrame(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR TxRate,
	IN BOOLEAN bQosNull,
	IN USHORT PwrMgmt)
{
	UCHAR NullFrame[48];
	ULONG Length;
	PHEADER_802_11 pHeader_802_11;
	struct wifi_dev *wdev = &pAd->StaCfg.wdev;
#ifdef RALINK_ATE
	if (ATE_ON(pAd))
	{
		return;
	}
#endif /* RALINK_ATE */

	/* WPA 802.1x secured port control */
	if (((wdev->AuthMode == Ndis802_11AuthModeWPA) ||
	     (wdev->AuthMode == Ndis802_11AuthModeWPAPSK) ||
	     (wdev->AuthMode == Ndis802_11AuthModeWPA2) ||
	     (wdev->AuthMode == Ndis802_11AuthModeWPA2PSK)
#ifdef WPA_SUPPLICANT_SUPPORT
	     || (pAd->StaCfg.wdev.IEEE8021X == TRUE)
#endif
#ifdef WAPI_SUPPORT
	     || (wdev->AuthMode == Ndis802_11AuthModeWAICERT)
	     || (wdev->AuthMode == Ndis802_11AuthModeWAIPSK)
#endif /* WAPI_SUPPORT */
	    ) && (wdev->PortSecured == WPA_802_1X_PORT_NOT_SECURED)) {
		return;
	}

	NdisZeroMemory(NullFrame, 48);
	Length = sizeof (HEADER_802_11);

	pHeader_802_11 = (PHEADER_802_11) NullFrame;

	pHeader_802_11->FC.Type = FC_TYPE_DATA;
	pHeader_802_11->FC.SubType = SUBTYPE_DATA_NULL;
	pHeader_802_11->FC.ToDs = 1;
	COPY_MAC_ADDR(pHeader_802_11->Addr1, pAd->CommonCfg.Bssid);
	COPY_MAC_ADDR(pHeader_802_11->Addr2, pAd->CurrentAddress);
	COPY_MAC_ADDR(pHeader_802_11->Addr3, pAd->CommonCfg.Bssid);

	if (pAd->CommonCfg.bAPSDForcePowerSave) {
		pHeader_802_11->FC.PwrMgmt = PWR_SAVE;
	} else {
		BOOLEAN FlgCanPmBitSet = TRUE;

#ifdef DOT11Z_TDLS_SUPPORT
		/* check TDLS condition */
		if (pAd->StaCfg.TdlsInfo.TdlsFlgIsKeepingActiveCountDown == TRUE)
			FlgCanPmBitSet = FALSE;
#endif /* DOT11Z_TDLS_SUPPORT */

		if (FlgCanPmBitSet == TRUE)
		pHeader_802_11->FC.PwrMgmt = PwrMgmt;
		else
			pHeader_802_11->FC.PwrMgmt = PWR_ACTIVE;
	}

	pHeader_802_11->Duration = pAd->CommonCfg.Dsifs + RTMPCalcDuration(pAd, TxRate, 14);

	/* sequence is increased in MlmeHardTx */
	pHeader_802_11->Sequence = pAd->Sequence;
	pAd->Sequence = (pAd->Sequence + 1) & MAXSEQ;	/* next sequence  */

	/* Prepare QosNull function frame */
	if (bQosNull) {
		pHeader_802_11->FC.SubType = SUBTYPE_QOS_NULL;

		/* copy QOS control bytes */
		NullFrame[Length] = 0;
		NullFrame[Length + 1] = 0;
		Length += 2;	/* if pad with 2 bytes for alignment, APSD will fail */
	}

	HAL_KickOutNullFrameTx(pAd, 0, NullFrame, Length);

}


/*
--------------------------------------------------------
FIND ENCRYPT KEY AND DECIDE CIPHER ALGORITHM
	Find the WPA key, either Group or Pairwise Key
	LEAP + TKIP also use WPA key.
--------------------------------------------------------
Decide WEP bit and cipher suite to be used. Same cipher suite should be used for whole fragment burst
	In Cisco CCX 2.0 Leap Authentication
	WepStatus is Ndis802_11WEPEnabled but the key will use PairwiseKey
	Instead of the SharedKey, SharedKey Length may be Zero.
*/
VOID STAFindCipherAlgorithm(RTMP_ADAPTER *pAd, TX_BLK *pTxBlk)
{
	NDIS_802_11_ENCRYPTION_STATUS Cipher;	/* To indicate cipher used for this packet */
	UCHAR CipherAlg = CIPHER_NONE;	/* cipher alogrithm */
	UCHAR KeyIdx = 0xff;
	PUCHAR pSrcBufVA;
	PCIPHER_KEY pKey = NULL;
	PMAC_TABLE_ENTRY pMacEntry;
	struct wifi_dev *wdev = &pAd->StaCfg.wdev;

	pSrcBufVA = GET_OS_PKT_DATAPTR(pTxBlk->pPacket);
	pMacEntry = pTxBlk->pMacEntry;

#ifdef WAPI_SUPPORT
	if (pMacEntry
		    && (pMacEntry->WepStatus == Ndis802_11EncryptionSMS4Enabled)) {
		if (RTMP_GET_PACKET_WAI(pTxBlk->pPacket)) {
			/* WAI negotiation packet is always clear. */
			CipherAlg = CIPHER_NONE;
			pKey = NULL;
		} else {
			KeyIdx = pMacEntry->usk_id;	/* USK ID */
			CipherAlg = pMacEntry->PairwiseKey.CipherAlg;
			if (CipherAlg == CIPHER_SMS4) {
				pKey = &pMacEntry->PairwiseKey;
#ifdef SOFT_ENCRYPT
				if (CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_SOFTWARE_ENCRYPT)) {
					TX_BLK_SET_FLAG(pTxBlk, fTX_bSwEncrypt);
					/* TSC increment pre encryption transmittion */
					inc_iv_byte(pKey->TxTsc, LEN_WAPI_TSC, 2);
				}
#endif /* SOFT_ENCRYPT */
			}
		}
	}
	else
#endif /* WAPI_SUPPORT */
	{
		/* Select Cipher */
		if ((*pSrcBufVA & 0x01) && (ADHOC_ON(pAd)))
			Cipher = pAd->StaCfg.GroupCipher;	/* Cipher for Multicast or Broadcast */
		else
			Cipher = pAd->StaCfg.PairCipher;	/* Cipher for Unicast */

		if (RTMP_GET_PACKET_EAPOL(pTxBlk->pPacket)) {
			ASSERT(pAd->SharedKey[BSS0][0].CipherAlg <= CIPHER_CKIP128);

			/* 4-way handshaking frame must be clear */
			if (!(TX_BLK_TEST_FLAG(pTxBlk, fTX_bClearEAPFrame)) &&
			    (pAd->SharedKey[BSS0][0].CipherAlg) &&
			    (pAd->SharedKey[BSS0][0].KeyLen)) {
				CipherAlg = pAd->SharedKey[BSS0][0].CipherAlg;
				KeyIdx = 0;
			}
		} else if (Cipher == Ndis802_11WEPEnabled) {
			KeyIdx = wdev->DefaultKeyId;
		} else if ((Cipher == Ndis802_11TKIPEnable) ||
			   (Cipher == Ndis802_11AESEnable)) {
			if ((*pSrcBufVA & 0x01) && (ADHOC_ON(pAd)))	/* multicast */
				KeyIdx = wdev->DefaultKeyId;
			else if (pAd->SharedKey[BSS0][0].KeyLen)
				KeyIdx = 0;
			else
				KeyIdx = wdev->DefaultKeyId;
		}

		if (KeyIdx == 0xff)
			CipherAlg = CIPHER_NONE;
#ifdef ADHOC_WPA2PSK_SUPPORT
		else if ((ADHOC_ON(pAd))
			 && (Cipher == Ndis802_11AESEnable)
			 && (wdev->AuthMode == Ndis802_11AuthModeWPA2PSK)
			 && (wdev->PortSecured == WPA_802_1X_PORT_SECURED)) {
			CipherAlg = CIPHER_AES;
			pKey = &pAd->SharedKey[BSS0][KeyIdx];
		}
#endif /* ADHOC_WPA2PSK_SUPPORT */
		else if ((Cipher == Ndis802_11EncryptionDisabled)
			 || (pAd->SharedKey[BSS0][KeyIdx].KeyLen == 0))
			CipherAlg = CIPHER_NONE;
#ifdef WPA_SUPPLICANT_SUPPORT
		else if (pAd->StaCfg.wpa_supplicant_info.WpaSupplicantUP &&
			 (Cipher == Ndis802_11WEPEnabled) &&
			 (wdev->IEEE8021X == TRUE) &&
			 (wdev->PortSecured == WPA_802_1X_PORT_NOT_SECURED))
			CipherAlg = CIPHER_NONE;
#endif /* WPA_SUPPLICANT_SUPPORT */
		else {
			CipherAlg = pAd->SharedKey[BSS0][KeyIdx].CipherAlg;
			pKey = &pAd->SharedKey[BSS0][KeyIdx];
		}
	}

	pTxBlk->CipherAlg = CipherAlg;
	pTxBlk->pKey = pKey;
	pTxBlk->KeyIdx = KeyIdx;
}


#ifdef HDR_TRANS_SUPPORT
VOID STABuildWifiInfo(RTMP_ADAPTER *pAd, TX_BLK *pTxBlk)
{
	PWIFI_INFO_STRUC pWI;
#ifdef QOS_DLS_SUPPORT
	BOOLEAN bDLSFrame = FALSE;
	INT DlsEntryIndex = 0;
#endif /* QOS_DLS_SUPPORT */
	UINT8 TXWISize = pAd->chipCap.TXWISize;

	pTxBlk->MpduHeaderLen = WIFI_INFO_SIZE;

	pWI =
	    (WIFI_INFO_STRUC *) & pTxBlk->HeaderBuf[TXINFO_SIZE + TXWISize];

	NdisZeroMemory(pWI, WIFI_INFO_SIZE);

	pWI->field.QoS = (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWMM)) ? 1 : 0;

#ifdef QOS_DLS_SUPPORT
	if (INFRA_ON(pAd)) {
		/* Check if the frame can be sent through DLS direct link interface */
		/* If packet can be sent through DLS, then force aggregation disable. (Hard to determine peer STA's capability) */
		DlsEntryIndex = RTMPCheckDLSFrame(pAd, pTxBlk->pSrcBufHeader);
		if (DlsEntryIndex >= 0)
			bDLSFrame = TRUE;
		else
			bDLSFrame = FALSE;
	}
#endif /* QOS_DLS_SUPPORT */

	if (pTxBlk->pMacEntry) {
		if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bForceNonQoS)) {
			pWI->field.Seq_Num =
			    pTxBlk->pMacEntry->NonQosDataSeq;
			pTxBlk->pMacEntry->NonQosDataSeq =
			    (pTxBlk->pMacEntry->NonQosDataSeq + 1) & MAXSEQ;
		} else {
			pWI->field.Seq_Num =
			    pTxBlk->pMacEntry->TxSeq[pTxBlk->UserPriority];
			pTxBlk->pMacEntry->TxSeq[pTxBlk->UserPriority] =
			    (pTxBlk->pMacEntry->TxSeq[pTxBlk->UserPriority] + 1) & MAXSEQ;
		}
	} else {
		pWI->field.Seq_Num = pAd->Sequence;
		pAd->Sequence = (pAd->Sequence + 1) & MAXSEQ;	/* next sequence  */
	}

	pWI->field.More_Data = TX_BLK_TEST_FLAG(pTxBlk, fTX_bMoreData);

	if (pAd->StaCfg.BssType == BSS_INFRA) {
#ifdef QOS_DLS_SUPPORT
		if (bDLSFrame) 
			pWI->field.Mode = 0;	/* IBSS */
		else
#endif /* QOS_DLS_SUPPORT */
#ifdef DOT11Z_TDLS_SUPPORT
		if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bTdlsEntry)) 
			pWI->field.Mode = 0; /* IBSS */
		else
#endif /* DOT11Z_TDLS_SUPPORT */
			pWI->field.Mode = 2; /* STA*/
	} else if (ADHOC_ON(pAd)) {
		pWI->field.Mode = 0; /* IBSS */
	}

	if (pTxBlk->CipherAlg != CIPHER_NONE)
		pWI->field.WEP = 1;

	if (pAd->CommonCfg.bAPSDForcePowerSave)
		pWI->field.PS = PWR_SAVE;
	else
		pWI->field.PS = (pAd->StaCfg.Psm == PWR_SAVE);
}


VOID STABuildCacheWifiInfo(
	IN RTMP_ADAPTER *pAd,
	IN TX_BLK *pTxBlk,
	IN UCHAR *pWiInfo)
{
	PWIFI_INFO_STRUC pWI;
	MAC_TABLE_ENTRY *pMacEntry;

	pWI = (PWIFI_INFO_STRUC)pWiInfo;
	pMacEntry = pTxBlk->pMacEntry;

	pTxBlk->MpduHeaderLen = WIFI_INFO_SIZE;

	/* More Bit */
	pWI->field.More_Data = TX_BLK_TEST_FLAG(pTxBlk, fTX_bMoreData);

	/* Sequence */
	pWI->field.Seq_Num = pMacEntry->TxSeq[pTxBlk->UserPriority];
	pMacEntry->TxSeq[pTxBlk->UserPriority] =
	    (pMacEntry->TxSeq[pTxBlk->UserPriority] + 1) & MAXSEQ;

	{
		/* Check if the frame can be sent through DLS direct link interface
		   If packet can be sent through DLS, then force aggregation disable. (Hard to determine peer STA's capability) */
#ifdef QOS_DLS_SUPPORT
		BOOLEAN bDLSFrame = FALSE;
		INT DlsEntryIndex = 0;

		DlsEntryIndex = RTMPCheckDLSFrame(pAd, pTxBlk->pSrcBufHeader);
		if (DlsEntryIndex >= 0)
			bDLSFrame = TRUE;
		else
			bDLSFrame = FALSE;
#endif /* QOS_DLS_SUPPORT */

		/* The addr3 of normal packet send from DS is Dest Mac address. */
#ifdef QOS_DLS_SUPPORT
		if (bDLSFrame) {
			pWI->field.Mode = 0;	/* IBSS */
		} else
#endif /* QOS_DLS_SUPPORT */
#ifdef DOT11Z_TDLS_SUPPORT
		if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bTdlsEntry)) {
			pWI->field.Mode = 0;	/* IBSS */
		} else
#endif /* DOT11Z_TDLS_SUPPORT */
		if (ADHOC_ON(pAd))
			pWI->field.Mode = 0;	/* IBSS */
		else {
			pWI->field.Mode = 2; /* STA*/
		}
	}

	/* 
	   -----------------------------------------------------------------
	   STEP 2. MAKE A COMMON 802.11 HEADER SHARED BY ENTIRE FRAGMENT BURST. Fill sequence later. 
	   -----------------------------------------------------------------
	 */
	if (pAd->CommonCfg.bAPSDForcePowerSave)
		pWI->field.PS = PWR_SAVE;
	else
		pWI->field.PS = (pAd->StaCfg.Psm == PWR_SAVE);
}
#endif /* HDR_TRANS_SUPPORT */


VOID STABuildCommon802_11Header(RTMP_ADAPTER *pAd, TX_BLK *pTxBlk)
{
	HEADER_802_11 *wifi_hdr;
#ifdef QOS_DLS_SUPPORT
	BOOLEAN bDLSFrame = FALSE;
	INT DlsEntryIndex = 0;
#endif /* QOS_DLS_SUPPORT */
	UINT8 TXWISize = pAd->chipCap.TXWISize;

	/* MAKE A COMMON 802.11 HEADER */

	/* normal wlan header size : 24 octets */
	pTxBlk->MpduHeaderLen = sizeof (HEADER_802_11);
	wifi_hdr = (HEADER_802_11 *)&pTxBlk->HeaderBuf[TXINFO_SIZE + TXWISize + TSO_SIZE];
	NdisZeroMemory(wifi_hdr, sizeof (HEADER_802_11));

	wifi_hdr->FC.FrDs = 0;
	wifi_hdr->FC.Type = FC_TYPE_DATA;
	wifi_hdr->FC.SubType = ((TX_BLK_TEST_FLAG(pTxBlk, fTX_bWMM)) ? SUBTYPE_QDATA : SUBTYPE_DATA);

#ifdef QOS_DLS_SUPPORT
	if (INFRA_ON(pAd)) {
		/* Check if the frame can be sent through DLS direct link interface */
		/* If packet can be sent through DLS, then force aggregation disable. (Hard to determine peer STA's capability) */
		DlsEntryIndex = RTMPCheckDLSFrame(pAd, pTxBlk->pSrcBufHeader);
		if (DlsEntryIndex >= 0)
			bDLSFrame = TRUE;
		else
			bDLSFrame = FALSE;
	}
#endif /* QOS_DLS_SUPPORT */

	if (pTxBlk->pMacEntry) {
		if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bForceNonQoS)) {
			wifi_hdr->Sequence = pTxBlk->pMacEntry->NonQosDataSeq;
			pTxBlk->pMacEntry->NonQosDataSeq = (pTxBlk->pMacEntry->NonQosDataSeq + 1) & MAXSEQ;
		} else {
			wifi_hdr->Sequence = pTxBlk->pMacEntry->TxSeq[pTxBlk->UserPriority];
			pTxBlk->pMacEntry->TxSeq[pTxBlk->UserPriority] = (pTxBlk->pMacEntry->TxSeq[pTxBlk->UserPriority] + 1) & MAXSEQ;
		}
	} else {
		wifi_hdr->Sequence = pAd->Sequence;
		pAd->Sequence = (pAd->Sequence + 1) & MAXSEQ;	/* next sequence  */
	}

	wifi_hdr->Frag = 0;

	wifi_hdr->FC.MoreData = TX_BLK_TEST_FLAG(pTxBlk, fTX_bMoreData);

	{
		if (pAd->StaCfg.BssType == BSS_INFRA) {
#ifdef QOS_DLS_SUPPORT
			if (bDLSFrame) {
				COPY_MAC_ADDR(wifi_hdr->Addr1, pTxBlk->pSrcBufHeader);
				COPY_MAC_ADDR(wifi_hdr->Addr2, pAd->CurrentAddress);
				COPY_MAC_ADDR(wifi_hdr->Addr3, pAd->CommonCfg.Bssid);
				wifi_hdr->FC.ToDs = 0;
			} else
#endif /* QOS_DLS_SUPPORT */
#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
			if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bTdlsEntry)) {
				COPY_MAC_ADDR(wifi_hdr->Addr1, pTxBlk->pSrcBufHeader);
				COPY_MAC_ADDR(wifi_hdr->Addr2, pAd->CurrentAddress);
				COPY_MAC_ADDR(wifi_hdr->Addr3, pAd->CommonCfg.Bssid);				
				wifi_hdr->FC.ToDs = 0;
				DBGPRINT(RT_DEBUG_TRACE, ("##### TDLS - set header ToDS 0 !!!!!!!!!\n"));
			} else
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */
			{
				COPY_MAC_ADDR(wifi_hdr->Addr1, pAd->CommonCfg.Bssid);
				COPY_MAC_ADDR(wifi_hdr->Addr2, pAd->CurrentAddress);
				COPY_MAC_ADDR(wifi_hdr->Addr3, pTxBlk->pSrcBufHeader);
				wifi_hdr->FC.ToDs = 1;
#ifdef CLIENT_WDS
				if (!MAC_ADDR_EQUAL
				    ((pTxBlk->pSrcBufHeader + MAC_ADDR_LEN),
				     pAd->CurrentAddress)) {
					wifi_hdr->FC.FrDs = 1;
					COPY_MAC_ADDR(&wifi_hdr->Octet[0], pTxBlk->pSrcBufHeader + MAC_ADDR_LEN);	/* ADDR4 = SA */
					pTxBlk->MpduHeaderLen += MAC_ADDR_LEN;
				}
#endif /* CLIENT_WDS */
			}
		}
		else if (ADHOC_ON(pAd))
		{
			COPY_MAC_ADDR(wifi_hdr->Addr1, pTxBlk->pSrcBufHeader);
#ifdef XLINK_SUPPORT
			if (pAd->StaCfg.PSPXlink)
				/* copy the SA of ether frames to address 2 of 802.11 frame */
				COPY_MAC_ADDR(wifi_hdr->Addr2, pTxBlk->pSrcBufHeader + MAC_ADDR_LEN);
			else
#endif /* XLINK_SUPPORT */
				COPY_MAC_ADDR(wifi_hdr->Addr2, pAd->CurrentAddress);
			COPY_MAC_ADDR(wifi_hdr->Addr3, pAd->CommonCfg.Bssid);
			wifi_hdr->FC.ToDs = 0;
		}
	}

	if (pTxBlk->CipherAlg != CIPHER_NONE)
		wifi_hdr->FC.Wep = 1;

	/*
	   -----------------------------------------------------------------
	   STEP 2. MAKE A COMMON 802.11 HEADER SHARED BY ENTIRE FRAGMENT BURST. Fill sequence later. 
	   -----------------------------------------------------------------
	 */
	if (pAd->CommonCfg.bAPSDForcePowerSave)
		wifi_hdr->FC.PwrMgmt = PWR_SAVE;
	else
		wifi_hdr->FC.PwrMgmt = (RtmpPktPmBitCheck(pAd) == TRUE);

#ifdef RT_CFG80211_P2P_SINGLE_DEVICE
		if (INFRA_ON(pAd))
		{
			if (CFG_P2PCLI_ON(pAd) && pAd->cfg80211_ctrl.bP2pCliPmEnable)
				wifi_hdr->FC.PwrMgmt = PWR_SAVE;
		}
#endif /* RT_CFG80211_P2P_SINGLE_DEVICE */
}


#ifdef DOT11_N_SUPPORT
VOID STABuildCache802_11Header(
	IN RTMP_ADAPTER *pAd,
	IN TX_BLK *pTxBlk,
	IN UCHAR *pHeader)
{
	MAC_TABLE_ENTRY *pMacEntry;
	PHEADER_802_11 pHeader80211;

	pHeader80211 = (PHEADER_802_11) pHeader;
	pMacEntry = pTxBlk->pMacEntry;

	/* Update the cached 802.11 HEADER */

	/* normal wlan header size : 24 octets */
	pTxBlk->MpduHeaderLen = sizeof (HEADER_802_11);

	/* More Bit */
	pHeader80211->FC.MoreData = TX_BLK_TEST_FLAG(pTxBlk, fTX_bMoreData);

	/* Sequence */
	pHeader80211->Sequence = pMacEntry->TxSeq[pTxBlk->UserPriority];
	pMacEntry->TxSeq[pTxBlk->UserPriority] =
	    (pMacEntry->TxSeq[pTxBlk->UserPriority] + 1) & MAXSEQ;

	{
		/* Check if the frame can be sent through DLS direct link interface
		   If packet can be sent through DLS, then force aggregation disable. (Hard to determine peer STA's capability) */
#ifdef QOS_DLS_SUPPORT
		BOOLEAN bDLSFrame = FALSE;
		INT DlsEntryIndex = 0;

		DlsEntryIndex = RTMPCheckDLSFrame(pAd, pTxBlk->pSrcBufHeader);
		if (DlsEntryIndex >= 0)
			bDLSFrame = TRUE;
		else
			bDLSFrame = FALSE;
#endif /* QOS_DLS_SUPPORT */

		/* The addr3 of normal packet send from DS is Dest Mac address. */
#ifdef QOS_DLS_SUPPORT
		if (bDLSFrame) {
			COPY_MAC_ADDR(pHeader80211->Addr1, pTxBlk->pSrcBufHeader);
			COPY_MAC_ADDR(pHeader80211->Addr3, pAd->CommonCfg.Bssid);
			pHeader80211->FC.ToDs = 0;
		} else
#endif /* QOS_DLS_SUPPORT */
#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
		if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bTdlsEntry)) {
			COPY_MAC_ADDR(pHeader80211->Addr1, pTxBlk->pSrcBufHeader);
			COPY_MAC_ADDR(pHeader80211->Addr3, pAd->CommonCfg.Bssid);
		} else
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */
		if (ADHOC_ON(pAd))
			COPY_MAC_ADDR(pHeader80211->Addr3,
				      pAd->CommonCfg.Bssid);
		else {
			COPY_MAC_ADDR(pHeader80211->Addr3,
				      pTxBlk->pSrcBufHeader);
#ifdef CLIENT_WDS
			if (!MAC_ADDR_EQUAL
			    ((pTxBlk->pSrcBufHeader + MAC_ADDR_LEN),
			     pAd->CurrentAddress)) {
				pHeader80211->FC.FrDs = 1;
				COPY_MAC_ADDR(&pHeader80211->Octet[0], pTxBlk->pSrcBufHeader + MAC_ADDR_LEN);	/* ADDR4 = SA */
				pTxBlk->MpduHeaderLen += MAC_ADDR_LEN;
			}
#endif /* CLIENT_WDS */
		}
	}

	/* 
	   -----------------------------------------------------------------
	   STEP 2. MAKE A COMMON 802.11 HEADER SHARED BY ENTIRE FRAGMENT BURST. Fill sequence later. 
	   -----------------------------------------------------------------
	 */
	if (pAd->CommonCfg.bAPSDForcePowerSave)
		pHeader80211->FC.PwrMgmt = PWR_SAVE;
	else
		pHeader80211->FC.PwrMgmt = (RtmpPktPmBitCheck(pAd) == TRUE);
}
#endif /* DOT11_N_SUPPORT */


static inline PUCHAR STA_Build_ARalink_Frame_Header(
	IN RTMP_ADAPTER *pAd,
	IN TX_BLK *pTxBlk)
{
	PUCHAR pHeaderBufPtr;
	HEADER_802_11 *pHeader_802_11;
	PNDIS_PACKET pNextPacket;
	UINT32 nextBufLen;
	PQUEUE_ENTRY pQEntry;
	UINT8 TXWISize = pAd->chipCap.TXWISize;

	STAFindCipherAlgorithm(pAd, pTxBlk);
	STABuildCommon802_11Header(pAd, pTxBlk);

	pHeaderBufPtr = &pTxBlk->HeaderBuf[TXINFO_SIZE + TXWISize];
	pHeader_802_11 = (HEADER_802_11 *) pHeaderBufPtr;

	/* steal "order" bit to mark "aggregation" */
	pHeader_802_11->FC.Order = 1;

	/* skip common header */
	pHeaderBufPtr += pTxBlk->MpduHeaderLen;

	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWMM)) {
		/* build QOS Control bytes */
		*pHeaderBufPtr = (pTxBlk->UserPriority & 0x0F);

#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
#ifdef UAPSD_SUPPORT
		UAPSD_MR_EOSP_SET(pHeaderBufPtr, pTxBlk);
#endif /* UAPSD_SUPPORT */
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */

		*(pHeaderBufPtr + 1) = 0;
		pHeaderBufPtr += 2;
		pTxBlk->MpduHeaderLen += 2;
	}

	/* padding at front of LLC header. LLC header should at 4-bytes aligment. */
	pTxBlk->HdrPadLen = (ULONG) pHeaderBufPtr;
	pHeaderBufPtr = (PUCHAR) ROUND_UP(pHeaderBufPtr, 4);
	pTxBlk->HdrPadLen = (ULONG) (pHeaderBufPtr - pTxBlk->HdrPadLen);

	/* For RA Aggregation, */
	/* put the 2nd MSDU length(extra 2-byte field) after QOS_CONTROL in little endian format */
	pQEntry = pTxBlk->TxPacketList.Head;
	pNextPacket = QUEUE_ENTRY_TO_PACKET(pQEntry);
	nextBufLen = GET_OS_PKT_LEN(pNextPacket);
	if (RTMP_GET_PACKET_VLAN(pNextPacket))
		nextBufLen -= LENGTH_802_1Q;

	*pHeaderBufPtr = (UCHAR) nextBufLen & 0xff;
	*(pHeaderBufPtr + 1) = (UCHAR) (nextBufLen >> 8);

	pHeaderBufPtr += 2;
	pTxBlk->MpduHeaderLen += 2;

	return pHeaderBufPtr;

}


#ifdef DOT11_N_SUPPORT
static inline PUCHAR STA_Build_AMSDU_Frame_Header(
	IN RTMP_ADAPTER *pAd,
	IN TX_BLK *pTxBlk)
{
	PUCHAR pHeaderBufPtr;
	HEADER_802_11 *pHeader_802_11;
	UINT8 TXWISize = pAd->chipCap.TXWISize;

	STAFindCipherAlgorithm(pAd, pTxBlk);
	STABuildCommon802_11Header(pAd, pTxBlk);

	pHeaderBufPtr = &pTxBlk->HeaderBuf[TXINFO_SIZE + TXWISize];
	pHeader_802_11 = (HEADER_802_11 *) pHeaderBufPtr;

	/* skip common header */
	pHeaderBufPtr += pTxBlk->MpduHeaderLen;

	/* build QOS Control bytes */
	*pHeaderBufPtr =
	    (pTxBlk->UserPriority & 0x0F) | (pAd->CommonCfg.
					     AckPolicy[pTxBlk->QueIdx] << 5);

#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
#ifdef UAPSD_SUPPORT
	UAPSD_MR_EOSP_SET(pHeaderBufPtr, pTxBlk);
#endif /* UAPSD_SUPPORT */
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */

	/* A-MSDU packet */
	*pHeaderBufPtr |= 0x80;

	*(pHeaderBufPtr + 1) = 0;
	pHeaderBufPtr += 2;
	pTxBlk->MpduHeaderLen += 2;

	/*
	   padding at front of LLC header
	   LLC header should locate at 4-octets aligment

	   @@@ MpduHeaderLen excluding padding @@@
	 */
	pTxBlk->HdrPadLen = (ULONG) pHeaderBufPtr;
	pHeaderBufPtr = (PUCHAR) ROUND_UP(pHeaderBufPtr, 4);
	pTxBlk->HdrPadLen = (ULONG) (pHeaderBufPtr - pTxBlk->HdrPadLen);

	return pHeaderBufPtr;

}


#ifdef TXBF_SUPPORT
VOID STA_AMPDU_Frame_Tx(
	IN PRTMP_ADAPTER pAd,
	IN TX_BLK *pTxBlk,
	IN CHAR TxSndgTypePerEntry)
#else
VOID STA_AMPDU_Frame_Tx(
	IN PRTMP_ADAPTER pAd,
	IN TX_BLK *pTxBlk)
#endif
{
	HEADER_802_11 *pHeader_802_11;
	PUCHAR pHeaderBufPtr;
	USHORT FreeNumber = 0;
	MAC_TABLE_ENTRY *pMacEntry;
	BOOLEAN bVLANPkt;
	PQUEUE_ENTRY pQEntry;
	BOOLEAN			bHTCPlus;
	UINT8 TXWISize = pAd->chipCap.TXWISize;

	
	ASSERT(pTxBlk);

	while (pTxBlk->TxPacketList.Head) {
		pQEntry = RemoveHeadQueue(&pTxBlk->TxPacketList);
		pTxBlk->pPacket = QUEUE_ENTRY_TO_PACKET(pQEntry);
		if (RTMP_FillTxBlkInfo(pAd, pTxBlk) != TRUE) {
			RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket,
					    NDIS_STATUS_FAILURE);
			continue;
		}

		bVLANPkt = (RTMP_GET_PACKET_VLAN(pTxBlk->pPacket) ? TRUE : FALSE);

		pMacEntry = pTxBlk->pMacEntry;
		if ((pMacEntry->isCached)
#ifdef TXBF_SUPPORT
			&& (TxSndgTypePerEntry == SNDG_TYPE_DISABLE)
#endif // TXBF_SUPPORT //
		)
		{
			/* NOTE: Please make sure the size of pMacEntry->CachedBuf[] is smaller than pTxBlk->HeaderBuf[]!!!! */
#ifndef VENDOR_FEATURE1_SUPPORT
			NdisMoveMemory((PUCHAR)
				       (&pTxBlk->HeaderBuf[TXINFO_SIZE]),
				       (PUCHAR) (&pMacEntry->CachedBuf[0]),
				       TXWISize + sizeof (HEADER_802_11));
#else
			pTxBlk->HeaderBuf = (UCHAR *) (pMacEntry->HeaderBuf);
#endif /* VENDOR_FEATURE1_SUPPORT */

			pHeaderBufPtr = (UCHAR *)(&pTxBlk->HeaderBuf[TXINFO_SIZE + TXWISize]);
			STABuildCache802_11Header(pAd, pTxBlk, pHeaderBufPtr);

#ifdef SOFT_ENCRYPT
			RTMPUpdateSwCacheCipherInfo(pAd, pTxBlk, pHeaderBufPtr);
#endif /* SOFT_ENCRYPT */
		} else {
			STAFindCipherAlgorithm(pAd, pTxBlk);
			STABuildCommon802_11Header(pAd, pTxBlk);

			pHeaderBufPtr = &pTxBlk->HeaderBuf[TXINFO_SIZE + TXWISize];
		}

#ifdef SOFT_ENCRYPT
		if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bSwEncrypt)) {
			/* Check if the original data has enough buffer 
			   to insert or append WPI related field. */
			if (RTMPExpandPacketForSwEncrypt(pAd, pTxBlk) == FALSE) {
				RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket,
						    NDIS_STATUS_FAILURE);
				continue;
			}
		}
#endif /* SOFT_ENCRYPT */

#ifdef VENDOR_FEATURE1_SUPPORT
		if (pMacEntry->isCached
		    && (pMacEntry->Protocol ==
			RTMP_GET_PACKET_PROTOCOL(pTxBlk->pPacket))
#ifdef SOFT_ENCRYPT
		    && !TX_BLK_TEST_FLAG(pTxBlk, fTX_bSwEncrypt)
#endif /* SOFT_ENCRYPT */
#ifdef TXBF_SUPPORT
			&& (TxSndgTypePerEntry == SNDG_TYPE_DISABLE)
#endif // TXBF_SUPPORT //
			)
		{
			pHeader_802_11 = (HEADER_802_11 *) pHeaderBufPtr;

			/* skip common header */
			pHeaderBufPtr += pTxBlk->MpduHeaderLen;

			/* build QOS Control bytes */
			*pHeaderBufPtr = (pTxBlk->UserPriority & 0x0F);
#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
#ifdef UAPSD_SUPPORT
			UAPSD_MR_EOSP_SET(pHeaderBufPtr, pTxBlk);
#endif /* UAPSD_SUPPORT */
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */
			pTxBlk->MpduHeaderLen = pMacEntry->MpduHeaderLen;
			pHeaderBufPtr = ((UCHAR *) pHeader_802_11) + pTxBlk->MpduHeaderLen;

			pTxBlk->HdrPadLen = pMacEntry->HdrPadLen;

			/* skip 802.3 header */
			pTxBlk->pSrcBufData =
			    pTxBlk->pSrcBufHeader + LENGTH_802_3;
			pTxBlk->SrcBufLen -= LENGTH_802_3;

			/* skip vlan tag */
			if (RTMP_GET_PACKET_VLAN(pTxBlk->pPacket)) {
				pTxBlk->pSrcBufData += LENGTH_802_1Q;
				pTxBlk->SrcBufLen -= LENGTH_802_1Q;
			}
		}
		else
#endif /* VENDOR_FEATURE1_SUPPORT */
		{
			pHeader_802_11 = (HEADER_802_11 *) pHeaderBufPtr;

			/* skip common header */
			pHeaderBufPtr += pTxBlk->MpduHeaderLen;

			/*
			   build QOS Control bytes
			 */
			*pHeaderBufPtr = (pTxBlk->UserPriority & 0x0F);
#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
#ifdef UAPSD_SUPPORT
			UAPSD_MR_EOSP_SET(pHeaderBufPtr, pTxBlk);
#endif /* UAPSD_SUPPORT */
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */
			*(pHeaderBufPtr + 1) = 0;
			pHeaderBufPtr += 2;
			pTxBlk->MpduHeaderLen += 2;

			/*
			   build HTC+ 
			   HTC control field following QoS field
			 */
			bHTCPlus = FALSE;

			if ((pAd->CommonCfg.bRdg == TRUE)
			    && CLIENT_STATUS_TEST_FLAG(pTxBlk->pMacEntry, fCLIENT_STATUS_RDG_CAPABLE)
#ifdef TXBF_SUPPORT
				&& (TxSndgTypePerEntry != SNDG_TYPE_NDP)
#endif /* TXBF_SUPPORT */
			)
			{
				if (pMacEntry->isCached == FALSE)
				{
					/* mark HTC bit */
					pHeader_802_11->FC.Order = 1;

					NdisZeroMemory(pHeaderBufPtr, sizeof(HT_CONTROL));
					((PHT_CONTROL)pHeaderBufPtr)->RDG = 1;
				}
				
				bHTCPlus = TRUE;
			}

#ifdef TXBF_SUPPORT
			pTxBlk->TxSndgPkt = SNDG_TYPE_DISABLE;

			NdisAcquireSpinLock(&pMacEntry->TxSndgLock);	
			if (TxSndgTypePerEntry >= SNDG_TYPE_SOUNDING)
			{
				DBGPRINT(RT_DEBUG_TRACE, ("--Sounding in AMPDU: TxSndgType=%d, MCS=%d\n",
								TxSndgTypePerEntry,
								TxSndgTypePerEntry==SNDG_TYPE_NDP? pMacEntry->sndgMcs: pTxBlk->pTransmit->field.MCS));

				// Set HTC bit
				if (bHTCPlus == FALSE)
				{
					bHTCPlus = TRUE;
					NdisZeroMemory(pHeaderBufPtr, sizeof(HT_CONTROL));
				}
	
				if (TxSndgTypePerEntry == SNDG_TYPE_SOUNDING)
				{
					// Select compress if supported. Otherwise select noncompress
					if (pAd->CommonCfg.ETxBfNoncompress==0 &&
						(pMacEntry->HTCapability.TxBFCap.ExpComBF>0) )
						((PHT_CONTROL)pHeaderBufPtr)->CSISTEERING = 3;
					else
						((PHT_CONTROL)pHeaderBufPtr)->CSISTEERING = 2;

				}
				else if (TxSndgTypePerEntry == SNDG_TYPE_NDP)
				{
					// Select compress if supported. Otherwise select noncompress
					if (pAd->CommonCfg.ETxBfNoncompress==0 &&
						(pMacEntry->HTCapability.TxBFCap.ExpComBF>0) &&
						(pMacEntry->HTCapability.TxBFCap.ComSteerBFAntSup >= (pMacEntry->sndgMcs/8))
						)
						((PHT_CONTROL)pHeaderBufPtr)->CSISTEERING = 3;
					else
						((PHT_CONTROL)pHeaderBufPtr)->CSISTEERING = 2;

					// Set NDP Announcement
					((PHT_CONTROL)pHeaderBufPtr)->NDPAnnounce = 1;

					pTxBlk->TxNDPSndgBW = pMacEntry->sndgBW;
					pTxBlk->TxNDPSndgMcs = pMacEntry->sndgMcs;
				}
	
				pTxBlk->TxSndgPkt = TxSndgTypePerEntry;
				//pMacEntry->TxSndgType = SNDG_TYPE_DISABLE;
			}
			
			NdisReleaseSpinLock(&pMacEntry->TxSndgLock);
				
#ifdef MFB_SUPPORT
#if defined(MRQ_FORCE_TX)//have to replace this by the correct condition!!!
			pMacEntry->HTCapability.ExtHtCapInfo.MCSFeedback = MCSFBK_MRQ;
#endif
			if ((pMacEntry->HTCapability.ExtHtCapInfo.MCSFeedback >=MCSFBK_MRQ) &&
					(pTxBlk->TxSndgPkt == SNDG_TYPE_DISABLE))//because the signal format of sounding frmae may be different from normal data frame, which may result in different MFB
			{
				if (bHTCPlus == FALSE)
				{
					bHTCPlus = TRUE;
					NdisZeroMemory(pHeaderBufPtr, sizeof(HT_CONTROL));
				}
				MFB_PerPareMRQ(pAd, pHeaderBufPtr, pMacEntry);
			}

			if (pAd->CommonCfg.HtCapability.ExtHtCapInfo.MCSFeedback >=MCSFBK_MRQ && pMacEntry->toTxMfb == 1)
			{
				if (bHTCPlus == FALSE)
				{
					bHTCPlus = TRUE;
					NdisZeroMemory(pHeaderBufPtr, sizeof(HT_CONTROL));
				}
				MFB_PerPareMFB(pAd, pHeaderBufPtr, pMacEntry);// not complete yet!!!
				pMacEntry->toTxMfb = 0;
			}
#endif // MFB_SUPPORT //
#endif // TXBF_SUPPORT //

			if (bHTCPlus)
			{
				pHeader_802_11->FC.Order = 1;
				pHeaderBufPtr += 4;
				pTxBlk->MpduHeaderLen += 4;
			}

			/* pTxBlk->MpduHeaderLen = pHeaderBufPtr - pTxBlk->HeaderBuf - TXWI_SIZE - TXINFO_SIZE; */
			ASSERT(pTxBlk->MpduHeaderLen >= 24);

			/* skip 802.3 header */
			pTxBlk->pSrcBufData = pTxBlk->pSrcBufHeader + LENGTH_802_3;
			pTxBlk->SrcBufLen -= LENGTH_802_3;

			/* skip vlan tag */
			if (bVLANPkt) {
				pTxBlk->pSrcBufData += LENGTH_802_1Q;
				pTxBlk->SrcBufLen -= LENGTH_802_1Q;
			}

			/*
			   padding at front of LLC header
			   LLC header should locate at 4-octets aligment

			   @@@ MpduHeaderLen excluding padding @@@
			 */
			pTxBlk->HdrPadLen = (ULONG) pHeaderBufPtr;
			pHeaderBufPtr = (PUCHAR) ROUND_UP(pHeaderBufPtr, 4);
			pTxBlk->HdrPadLen = (ULONG) (pHeaderBufPtr - pTxBlk->HdrPadLen);

#ifdef VENDOR_FEATURE1_SUPPORT
			pMacEntry->HdrPadLen = pTxBlk->HdrPadLen;
#endif /* VENDOR_FEATURE1_SUPPORT */

#ifdef SOFT_ENCRYPT
			if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bSwEncrypt)) {
				UCHAR iv_offset = 0, ext_offset = 0;

				/*
				   if original Ethernet frame contains no LLC/SNAP, 
				   then an extra LLC/SNAP encap is required
				 */
				EXTRA_LLCSNAP_ENCAP_FROM_PKT_OFFSET(pTxBlk->pSrcBufData - 2,
								    pTxBlk->pExtraLlcSnapEncap);

				/* Insert LLC-SNAP encapsulation (8 octets) to MPDU data buffer */
				if (pTxBlk->pExtraLlcSnapEncap) {
					/* Reserve the front 8 bytes of data for LLC header */
					pTxBlk->pSrcBufData -= LENGTH_802_1_H;
					pTxBlk->SrcBufLen += LENGTH_802_1_H;

					NdisMoveMemory(pTxBlk->pSrcBufData,
						       pTxBlk->pExtraLlcSnapEncap, 6);
				}

				/* Construct and insert specific IV header to MPDU header */
				RTMPSoftConstructIVHdr(pTxBlk->CipherAlg,
						       pTxBlk->KeyIdx,
						       pTxBlk->pKey->TxTsc,
						       pHeaderBufPtr,
						       &iv_offset);
				pHeaderBufPtr += iv_offset;
				pTxBlk->MpduHeaderLen += iv_offset;

				/* Encrypt the MPDU data by software */
				RTMPSoftEncryptionAction(pAd,
							 pTxBlk->CipherAlg,
							 (PUCHAR)
							 pHeader_802_11,
							 pTxBlk->pSrcBufData,
							 pTxBlk->SrcBufLen,
							 pTxBlk->KeyIdx,
							 pTxBlk->pKey,
							 &ext_offset);
				pTxBlk->SrcBufLen += ext_offset;
				pTxBlk->TotalFrameLen += ext_offset;

			} else
#endif /* SOFT_ENCRYPT */
			{

				/*
				   Insert LLC-SNAP encapsulation - 8 octets
				 */
				EXTRA_LLCSNAP_ENCAP_FROM_PKT_OFFSET(pTxBlk->pSrcBufData - 2,
								    pTxBlk->pExtraLlcSnapEncap);
				if (pTxBlk->pExtraLlcSnapEncap) {
					NdisMoveMemory(pHeaderBufPtr,
						       pTxBlk->pExtraLlcSnapEncap, 6);
					pHeaderBufPtr += 6;
					/* get 2 octets (TypeofLen) */
					NdisMoveMemory(pHeaderBufPtr,
						       pTxBlk->pSrcBufData - 2,
						       2);
					pHeaderBufPtr += 2;
					pTxBlk->MpduHeaderLen += LENGTH_802_1_H;
				}

			}

#ifdef VENDOR_FEATURE1_SUPPORT
			pMacEntry->Protocol =
			    RTMP_GET_PACKET_PROTOCOL(pTxBlk->pPacket);
			pMacEntry->MpduHeaderLen = pTxBlk->MpduHeaderLen;
#endif /* VENDOR_FEATURE1_SUPPORT */
		}

		if ((pMacEntry->isCached)
#ifdef TXBF_SUPPORT
			&& (pTxBlk->TxSndgPkt == SNDG_TYPE_DISABLE)
#endif // TXBF_SUPPORT //
		)
		{
			RTMPWriteTxWI_Cache(pAd, (TXWI_STRUC *) (&pTxBlk->HeaderBuf[TXINFO_SIZE]), pTxBlk);
		} else {
			RTMPWriteTxWI_Data(pAd, (TXWI_STRUC *) (&pTxBlk->HeaderBuf[TXINFO_SIZE]), pTxBlk);

			NdisZeroMemory((PUCHAR) (&pMacEntry->CachedBuf[0]), sizeof (pMacEntry->CachedBuf));
			NdisMoveMemory((PUCHAR) (&pMacEntry->CachedBuf[0]), (PUCHAR) (&pTxBlk->HeaderBuf[TXINFO_SIZE]), (pHeaderBufPtr -(PUCHAR) (&pTxBlk->HeaderBuf[TXINFO_SIZE])));

#ifdef VENDOR_FEATURE1_SUPPORT
			/* use space to get performance enhancement */
			NdisZeroMemory((PUCHAR) (&pMacEntry->HeaderBuf[0]), sizeof (pMacEntry->HeaderBuf));
			NdisMoveMemory((PUCHAR) (&pMacEntry->HeaderBuf[0]),
				       (PUCHAR) (&pTxBlk->HeaderBuf[0]),
				       (pHeaderBufPtr - (PUCHAR) (&pTxBlk->HeaderBuf[0])));
#endif /* VENDOR_FEATURE1_SUPPORT */

			pMacEntry->isCached = TRUE;
		}

#ifdef TXBF_SUPPORT
		if (pTxBlk->TxSndgPkt != SNDG_TYPE_DISABLE)
			pMacEntry->isCached = FALSE;
#endif // TXBF_SUPPORT //

#ifdef STATS_COUNT_SUPPORT
		/* calculate Transmitted AMPDU count and ByteCount  */
		{
			pAd->RalinkCounters.TransmittedMPDUsInAMPDUCount.u.LowPart++;
			pAd->RalinkCounters.TransmittedOctetsInAMPDUCount.QuadPart += pTxBlk->SrcBufLen;
		}
#endif /* STATS_COUNT_SUPPORT */

		HAL_WriteTxResource(pAd, pTxBlk, TRUE, &FreeNumber);

#ifdef DBG_CTRL_SUPPORT
#ifdef INCLUDE_DEBUG_QUEUE
		if (pAd->CommonCfg.DebugFlags & DBF_DBQ_TXFRAME)
			dbQueueEnqueueTxFrame((UCHAR *)(&pTxBlk->HeaderBuf[TXINFO_SIZE]), (UCHAR *)pHeader_802_11);
#endif /* INCLUDE_DEBUG_QUEUE */
#endif /* DBG_CTRL_SUPPORT */

		/* Kick out Tx */
#ifdef PCIE_PS_SUPPORT
		if (!RTMP_TEST_PSFLAG(pAd, fRTMP_PS_DISABLE_TX))
#endif /* PCIE_PS_SUPPORT */
			HAL_KickOutTx(pAd, pTxBlk, pTxBlk->QueIdx);

		pAd->RalinkCounters.KickTxCount++;
		pAd->RalinkCounters.OneSecTxDoneCount++;
	}
}


#ifdef HDR_TRANS_SUPPORT
VOID STA_AMPDU_Frame_Tx_Hdr_Trns(
	IN PRTMP_ADAPTER pAd,
	IN TX_BLK *pTxBlk)
{
	HEADER_802_11 *pHeader_802_11;
	UCHAR *pWiBufPtr;
	USHORT FreeNumber = 0;
	MAC_TABLE_ENTRY *pMacEntry;
	BOOLEAN bVLANPkt;
	PQUEUE_ENTRY pQEntry;
	BOOLEAN			bHTCPlus;
	UINT8 TXWISize = pAd->chipCap.TXWISize;
	PWIFI_INFO_STRUC pWI;
	
	ASSERT(pTxBlk);

	while (pTxBlk->TxPacketList.Head) {
		pQEntry = RemoveHeadQueue(&pTxBlk->TxPacketList);
		pTxBlk->pPacket = QUEUE_ENTRY_TO_PACKET(pQEntry);
		if (RTMP_FillTxBlkInfo(pAd, pTxBlk) != TRUE) {
			RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket,
					    NDIS_STATUS_FAILURE);
			continue;
		}

		bVLANPkt = (RTMP_GET_PACKET_VLAN(pTxBlk->pPacket) ? TRUE : FALSE);

		pMacEntry = pTxBlk->pMacEntry;

		if ((pMacEntry->isCached))
		{
			/* NOTE: Please make sure the size of pMacEntry->CachedBuf[] is smaller than pTxBlk->HeaderBuf[]!!!! */
			NdisMoveMemory((PUCHAR)
				       (&pTxBlk->HeaderBuf[TXINFO_SIZE]),
				       (PUCHAR) (&pMacEntry->CachedBuf[0]),
				       TXWISize + WIFI_INFO_SIZE);

			pWiBufPtr = (PUCHAR) (&pTxBlk->HeaderBuf[TXINFO_SIZE + TXWISize]);

			STABuildCacheWifiInfo(pAd, pTxBlk, pWiBufPtr);

		} else {
			STAFindCipherAlgorithm(pAd, pTxBlk);

			STABuildWifiInfo(pAd, pTxBlk);

			pWiBufPtr = &pTxBlk->HeaderBuf[TXINFO_SIZE + TXWISize];
		}

		pWI = (PWIFI_INFO_STRUC)pWiBufPtr;

		pTxBlk->pSrcBufData = pTxBlk->pSrcBufHeader;

		if (bVLANPkt)
			pWI->field.VLAN = TRUE;
	
		pWI->field.TID = (pTxBlk->UserPriority & 0x0F);


		{
			/*
			   build HTC+ 
			   HTC control field following QoS field
			 */
			bHTCPlus = FALSE;

			if ((pAd->CommonCfg.bRdg == TRUE)
			    && CLIENT_STATUS_TEST_FLAG(pTxBlk->pMacEntry, fCLIENT_STATUS_RDG_CAPABLE)
			)
			{
				if (pMacEntry->isCached == FALSE)
				{
					/* mark HTC bit */
					pWI->field.RDG = 1;
				}
				
				bHTCPlus = TRUE;
			}

		}

		if ((pMacEntry->isCached))
		{
			RTMPWriteTxWI_Cache(pAd,
					    (TXWI_STRUC *) (&pTxBlk->HeaderBuf[TXINFO_SIZE]),
					    pTxBlk);
		} else {
			RTMPWriteTxWI_Data(pAd,
					   (TXWI_STRUC *) (&pTxBlk->HeaderBuf[TXINFO_SIZE]),
					   pTxBlk);

			NdisZeroMemory((PUCHAR) (&pMacEntry->CachedBuf[0]),
				       sizeof (pMacEntry->CachedBuf));
			NdisMoveMemory((PUCHAR) (&pMacEntry->CachedBuf[0]),
				       (PUCHAR) (&pTxBlk->HeaderBuf[TXINFO_SIZE]),
				       TXWISize + WIFI_INFO_SIZE);


			pMacEntry->isCached = TRUE;
		}

#ifdef STATS_COUNT_SUPPORT
		/* calculate Transmitted AMPDU count and ByteCount  */
		{
			pAd->RalinkCounters.TransmittedMPDUsInAMPDUCount.u.LowPart++;
			pAd->RalinkCounters.TransmittedOctetsInAMPDUCount.QuadPart += pTxBlk->SrcBufLen;
		}
#endif /* STATS_COUNT_SUPPORT */
		pTxBlk->NeedTrans = TRUE;
		HAL_WriteTxResource(pAd, pTxBlk, TRUE, &FreeNumber);

#ifdef DBG_CTRL_SUPPORT
#ifdef INCLUDE_DEBUG_QUEUE
		if (pAd->CommonCfg.DebugFlags & DBF_DBQ_TXFRAME)
			dbQueueEnqueueTxFrame((UCHAR *)(&pTxBlk->HeaderBuf[TXINFO_SIZE]), (UCHAR *)pHeader_802_11);
#endif /* INCLUDE_DEBUG_QUEUE */
#endif /* DBG_CTRL_SUPPORT */

		/* Kick out Tx */
#ifdef PCIE_PS_SUPPORT
		if (!RTMP_TEST_PSFLAG(pAd, fRTMP_PS_DISABLE_TX))
#endif /* PCIE_PS_SUPPORT */
			HAL_KickOutTx(pAd, pTxBlk, pTxBlk->QueIdx);

		pAd->RalinkCounters.KickTxCount++;
		pAd->RalinkCounters.OneSecTxDoneCount++;
	}

}
#endif /* HDR_TRANS_SUPPORT */


VOID STA_AMSDU_Frame_Tx(
	IN PRTMP_ADAPTER pAd,
	IN TX_BLK *pTxBlk)
{
	PUCHAR pHeaderBufPtr;
	USHORT FreeNumber = 0;
	USHORT subFramePayloadLen = 0;	/* AMSDU Subframe length without AMSDU-Header / Padding */
	USHORT totalMPDUSize = 0;
	UCHAR *subFrameHeader;
	UCHAR padding = 0;
	USHORT FirstTx = 0, LastTxIdx = 0;
	BOOLEAN bVLANPkt;
	int frameNum = 0;
	PQUEUE_ENTRY pQEntry;


	ASSERT(pTxBlk);

	ASSERT((pTxBlk->TxPacketList.Number > 1));

	while (pTxBlk->TxPacketList.Head) {
		pQEntry = RemoveHeadQueue(&pTxBlk->TxPacketList);
		pTxBlk->pPacket = QUEUE_ENTRY_TO_PACKET(pQEntry);
		if (RTMP_FillTxBlkInfo(pAd, pTxBlk) != TRUE) {
			RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket,
					    NDIS_STATUS_FAILURE);
			continue;
		}

		bVLANPkt =
		    (RTMP_GET_PACKET_VLAN(pTxBlk->pPacket) ? TRUE : FALSE);

		/* skip 802.3 header */
		pTxBlk->pSrcBufData = pTxBlk->pSrcBufHeader + LENGTH_802_3;
		pTxBlk->SrcBufLen -= LENGTH_802_3;

		/* skip vlan tag */
		if (bVLANPkt) {
			pTxBlk->pSrcBufData += LENGTH_802_1Q;
			pTxBlk->SrcBufLen -= LENGTH_802_1Q;
		}

		if (frameNum == 0) {
			pHeaderBufPtr = STA_Build_AMSDU_Frame_Header(pAd, pTxBlk);

			/* NOTE: TxWI->TxWIMPDUByteCnt will be updated after final frame was handled. */
			RTMPWriteTxWI_Data(pAd, (TXWI_STRUC *) (&pTxBlk->HeaderBuf[TXINFO_SIZE]), pTxBlk);
		} else {
			pHeaderBufPtr = &pTxBlk->HeaderBuf[0];
			padding = ROUND_UP(AMSDU_SUBHEAD_LEN + subFramePayloadLen, 4) - 
								(AMSDU_SUBHEAD_LEN + subFramePayloadLen);
			NdisZeroMemory(pHeaderBufPtr, padding + AMSDU_SUBHEAD_LEN);
			pHeaderBufPtr += padding;
			pTxBlk->MpduHeaderLen = padding;
		}

		/*
		   A-MSDU subframe
		   DA(6)+SA(6)+Length(2) + LLC/SNAP Encap
		 */
		subFrameHeader = pHeaderBufPtr;
		subFramePayloadLen = pTxBlk->SrcBufLen;

		NdisMoveMemory(subFrameHeader, pTxBlk->pSrcBufHeader, 12);

#ifdef ETH_CONVERT_SUPPORT
		if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bDonglePkt))
			NdisMoveMemory(&subFrameHeader[6], pAd->CurrentAddress,
				       6);
#endif /* ETH_CONVERT_SUPPORT */

		pHeaderBufPtr += AMSDU_SUBHEAD_LEN;
		pTxBlk->MpduHeaderLen += AMSDU_SUBHEAD_LEN;

		/* Insert LLC-SNAP encapsulation - 8 octets */
		EXTRA_LLCSNAP_ENCAP_FROM_PKT_OFFSET(pTxBlk->pSrcBufData - 2,
						    pTxBlk->pExtraLlcSnapEncap);

		subFramePayloadLen = pTxBlk->SrcBufLen;

		if (pTxBlk->pExtraLlcSnapEncap) {
			NdisMoveMemory(pHeaderBufPtr,
				       pTxBlk->pExtraLlcSnapEncap, 6);
			pHeaderBufPtr += 6;
			/* get 2 octets (TypeofLen) */
			NdisMoveMemory(pHeaderBufPtr, pTxBlk->pSrcBufData - 2,
				       2);
			pHeaderBufPtr += 2;
			pTxBlk->MpduHeaderLen += LENGTH_802_1_H;
			subFramePayloadLen += LENGTH_802_1_H;
		}

		/* update subFrame Length field */
		subFrameHeader[12] = (subFramePayloadLen & 0xFF00) >> 8;
		subFrameHeader[13] = subFramePayloadLen & 0xFF;

		totalMPDUSize += pTxBlk->MpduHeaderLen + pTxBlk->SrcBufLen;

		if (frameNum == 0)
			FirstTx =
			    HAL_WriteMultiTxResource(pAd, pTxBlk, frameNum,
						     &FreeNumber);
		else
			LastTxIdx =
			    HAL_WriteMultiTxResource(pAd, pTxBlk, frameNum,
						     &FreeNumber);

#ifdef DBG_CTRL_SUPPORT
#ifdef INCLUDE_DEBUG_QUEUE
		if (pAd->CommonCfg.DebugFlags & DBF_DBQ_TXFRAME)
			dbQueueEnqueueTxFrame((UCHAR *)(&pTxBlk->HeaderBuf[TXINFO_SIZE]), NULL);
#endif /* INCLUDE_DEBUG_QUEUE */
#endif /* DBG_CTRL_SUPPORT */

		frameNum++;

		pAd->RalinkCounters.KickTxCount++;
		pAd->RalinkCounters.OneSecTxDoneCount++;

		/* calculate Transmitted AMSDU Count and ByteCount */
		{
			pAd->RalinkCounters.TransmittedAMSDUCount.u.LowPart++;
			pAd->RalinkCounters.TransmittedOctetsInAMSDU.QuadPart +=
			    totalMPDUSize;
		}

	}

	HAL_FinalWriteTxResource(pAd, pTxBlk, totalMPDUSize, FirstTx);
	HAL_LastTxIdx(pAd, pTxBlk->QueIdx, LastTxIdx);

	/* Kick out Tx */
#ifdef PCIE_PS_SUPPORT
	if (!RTMP_TEST_PSFLAG(pAd, fRTMP_PS_DISABLE_TX))
#endif /* PCIE_PS_SUPPORT */
		HAL_KickOutTx(pAd, pTxBlk, pTxBlk->QueIdx);
}
#endif /* DOT11_N_SUPPORT */


VOID STA_Legacy_Frame_Tx(RTMP_ADAPTER *pAd, TX_BLK *pTxBlk)
{
	HEADER_802_11 *wifi_hdr;
	UCHAR *pHeaderBufPtr;
	USHORT FreeNumber = 0;
	BOOLEAN bVLANPkt;
	PQUEUE_ENTRY pQEntry;
	UINT8 TXWISize = pAd->chipCap.TXWISize;

	ASSERT(pTxBlk);

	pQEntry = RemoveHeadQueue(&pTxBlk->TxPacketList);
	pTxBlk->pPacket = QUEUE_ENTRY_TO_PACKET(pQEntry);
	if (RTMP_FillTxBlkInfo(pAd, pTxBlk) != TRUE) {
		RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_FAILURE);
		return;
	}
#ifdef STATS_COUNT_SUPPORT
	if (pTxBlk->TxFrameType == TX_MCAST_FRAME) {
		INC_COUNTER64(pAd->WlanCounters.MulticastTransmittedFrameCount);
	}
#endif /* STATS_COUNT_SUPPORT */

	if (pTxBlk->TxRate < pAd->CommonCfg.MinTxRate)
		pTxBlk->TxRate = pAd->CommonCfg.MinTxRate;

	STAFindCipherAlgorithm(pAd, pTxBlk);
	STABuildCommon802_11Header(pAd, pTxBlk);

#ifdef SOFT_ENCRYPT
	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bSwEncrypt)) {
		/* Check if the original data has enough buffer 
		   to insert or append WPI related field. */
		if (RTMPExpandPacketForSwEncrypt(pAd, pTxBlk) == FALSE) {
			RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_FAILURE);
			return;
		}
	}
#endif /* SOFT_ENCRYPT */

	/* skip 802.3 header */
	pTxBlk->pSrcBufData = pTxBlk->pSrcBufHeader + LENGTH_802_3;
	pTxBlk->SrcBufLen -= LENGTH_802_3;

	/* skip vlan tag */
	bVLANPkt = (RTMP_GET_PACKET_VLAN(pTxBlk->pPacket) ? TRUE : FALSE);
	if (bVLANPkt) {
		pTxBlk->pSrcBufData += LENGTH_802_1Q;
		pTxBlk->SrcBufLen -= LENGTH_802_1Q;
	}

	pHeaderBufPtr = &pTxBlk->HeaderBuf[TXINFO_SIZE + TXWISize + TSO_SIZE];
	wifi_hdr = (HEADER_802_11 *) pHeaderBufPtr;

	/* skip common header */
	pHeaderBufPtr += pTxBlk->MpduHeaderLen;

	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWMM)) {
		/* build QOS Control bytes */
		*(pHeaderBufPtr) =
		    ((pTxBlk->UserPriority & 0x0F) | (pAd->CommonCfg.AckPolicy[pTxBlk->QueIdx] << 5));
#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
#ifdef UAPSD_SUPPORT
		UAPSD_MR_EOSP_SET(pHeaderBufPtr, pTxBlk);
#endif /* UAPSD_SUPPORT */
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */
		*(pHeaderBufPtr + 1) = 0;
		pHeaderBufPtr += 2;
		pTxBlk->MpduHeaderLen += 2;
	}

	/* The remaining content of MPDU header should locate at 4-octets aligment */
	pTxBlk->HdrPadLen = (ULONG) pHeaderBufPtr;
	pHeaderBufPtr = (PUCHAR) ROUND_UP(pHeaderBufPtr, 4);
	pTxBlk->HdrPadLen = (ULONG) (pHeaderBufPtr - pTxBlk->HdrPadLen);

#ifdef SOFT_ENCRYPT
	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bSwEncrypt)) {
		UCHAR iv_offset = 0, ext_offset = 0;

		/*
		   if original Ethernet frame contains no LLC/SNAP, 
		   then an extra LLC/SNAP encap is required 
		 */
		EXTRA_LLCSNAP_ENCAP_FROM_PKT_OFFSET(pTxBlk->pSrcBufData - 2,
						    pTxBlk->pExtraLlcSnapEncap);

		/* Insert LLC-SNAP encapsulation (8 octets) to MPDU data buffer */
		if (pTxBlk->pExtraLlcSnapEncap) {
			/* Reserve the front 8 bytes of data for LLC header */
			pTxBlk->pSrcBufData -= LENGTH_802_1_H;
			pTxBlk->SrcBufLen += LENGTH_802_1_H;

			NdisMoveMemory(pTxBlk->pSrcBufData,
				       pTxBlk->pExtraLlcSnapEncap, 6);
		}

		/* Construct and insert specific IV header to MPDU header */
		RTMPSoftConstructIVHdr(pTxBlk->CipherAlg,
				       pTxBlk->KeyIdx,
				       pTxBlk->pKey->TxTsc,
				       pHeaderBufPtr, &iv_offset);
		pHeaderBufPtr += iv_offset;
		pTxBlk->MpduHeaderLen += iv_offset;

		/* Encrypt the MPDU data by software */
		RTMPSoftEncryptionAction(pAd,
					 pTxBlk->CipherAlg,
					 (UCHAR *)wifi_hdr,
					 pTxBlk->pSrcBufData,
					 pTxBlk->SrcBufLen,
					 pTxBlk->KeyIdx,
					 pTxBlk->pKey, &ext_offset);
		pTxBlk->SrcBufLen += ext_offset;
		pTxBlk->TotalFrameLen += ext_offset;

	}
	else
#endif /* SOFT_ENCRYPT */
	{

		/*
		   Insert LLC-SNAP encapsulation - 8 octets

		   if original Ethernet frame contains no LLC/SNAP, 
		   then an extra LLC/SNAP encap is required 
		 */
		EXTRA_LLCSNAP_ENCAP_FROM_PKT_START(pTxBlk->pSrcBufHeader,
						   pTxBlk->pExtraLlcSnapEncap);
		if (pTxBlk->pExtraLlcSnapEncap) {
			UCHAR vlan_size;

			NdisMoveMemory(pHeaderBufPtr, pTxBlk->pExtraLlcSnapEncap, 6);
			pHeaderBufPtr += 6;
			/* skip vlan tag */
			vlan_size = (bVLANPkt) ? LENGTH_802_1Q : 0;
			/* get 2 octets (TypeofLen) */
			NdisMoveMemory(pHeaderBufPtr,
				       pTxBlk->pSrcBufHeader + 12 + vlan_size,
				       2);
			pHeaderBufPtr += 2;
			pTxBlk->MpduHeaderLen += LENGTH_802_1_H;
		}

	}

#ifdef ADHOC_WPA2PSK_SUPPORT
	if (ADHOC_ON(pAd)
	    && (pAd->StaCfg.wdev.AuthMode == Ndis802_11AuthModeWPA2PSK)
	    && (pAd->StaCfg.GroupCipher == Ndis802_11AESEnable)
	    && (!pTxBlk->pMacEntry)) {
		/* use Wcid as Hardware Key Index */
		GET_GroupKey_WCID(pAd, pTxBlk->Wcid, BSS0);
	}
#endif /* ADHOC_WPA2PSK_SUPPORT */

	/*
	   prepare for TXWI
	   use Wcid as Key Index
	 */

	RTMPWriteTxWI_Data(pAd, (TXWI_STRUC *)(&pTxBlk->HeaderBuf[TXINFO_SIZE]), pTxBlk);
	HAL_WriteTxResource(pAd, pTxBlk, TRUE, &FreeNumber);

#ifdef DBG_CTRL_SUPPORT
#ifdef INCLUDE_DEBUG_QUEUE
	if (pAd->CommonCfg.DebugFlags & DBF_DBQ_TXFRAME)
		dbQueueEnqueueTxFrame((UCHAR *)(&pTxBlk->HeaderBuf[TXINFO_SIZE]), (UCHAR *)wifi_hdr);
#endif /* INCLUDE_DEBUG_QUEUE */
#endif /* DBG_CTRL_SUPPORT */

	pAd->RalinkCounters.KickTxCount++;
	pAd->RalinkCounters.OneSecTxDoneCount++;

	/*
	   Kick out Tx
	 */
#ifdef PCIE_PS_SUPPORT
	if (!RTMP_TEST_PSFLAG(pAd, fRTMP_PS_DISABLE_TX))
#endif /* PCIE_PS_SUPPORT */
		HAL_KickOutTx(pAd, pTxBlk, pTxBlk->QueIdx);
}


#ifdef HDR_TRANS_SUPPORT
VOID STA_Legacy_Frame_Tx_Hdr_Trns(
	IN PRTMP_ADAPTER pAd,
	IN TX_BLK *pTxBlk)
{
	PUCHAR pHeaderBufPtr;
	USHORT FreeNumber = 0;
	BOOLEAN bVLANPkt;
	PQUEUE_ENTRY pQEntry;
	UINT8 TXWISize = pAd->chipCap.TXWISize;
	PWIFI_INFO_STRUC pWI;

	ASSERT(pTxBlk);

	//printk("STA_Legacy_Frame_Tx_Hdr_Trns\n");

	pQEntry = RemoveHeadQueue(&pTxBlk->TxPacketList);
	pTxBlk->pPacket = QUEUE_ENTRY_TO_PACKET(pQEntry);
	if (RTMP_FillTxBlkInfo(pAd, pTxBlk) != TRUE) {
		RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_FAILURE);
		return;
	}

#ifdef STATS_COUNT_SUPPORT
	if (pTxBlk->TxFrameType == TX_MCAST_FRAME) {
		INC_COUNTER64(pAd->WlanCounters.MulticastTransmittedFrameCount);
	}
#endif /* STATS_COUNT_SUPPORT */


	if (RTMP_GET_PACKET_RTS(pTxBlk->pPacket))
		TX_BLK_SET_FLAG(pTxBlk, fTX_bRtsRequired);
	else
		TX_BLK_CLEAR_FLAG(pTxBlk, fTX_bRtsRequired);

	bVLANPkt = (RTMP_GET_PACKET_VLAN(pTxBlk->pPacket) ? TRUE : FALSE);

	if (pTxBlk->TxRate < pAd->CommonCfg.MinTxRate)
		pTxBlk->TxRate = pAd->CommonCfg.MinTxRate;

	STAFindCipherAlgorithm(pAd, pTxBlk);
	STABuildWifiInfo(pAd, pTxBlk);

	pHeaderBufPtr = &pTxBlk->HeaderBuf[TXINFO_SIZE + TXWISize];

	pWI = (PWIFI_INFO_STRUC)pHeaderBufPtr;

	//hex_dump("wifi info:", pWI, sizeof(WIFI_INFO_STRUC));

	pTxBlk->pSrcBufData = pTxBlk->pSrcBufHeader;

	//hex_dump("pSrcBufData" , pTxBlk->pSrcBufData, pTxBlk->SrcBufLen);

	if(bVLANPkt)
		pWI->field.VLAN = TRUE;

	pWI->field.TID = (pTxBlk->UserPriority & 0x0F);


#ifdef ADHOC_WPA2PSK_SUPPORT
	if (ADHOC_ON(pAd)
	    && (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2PSK)
	    && (pAd->StaCfg.GroupCipher == Ndis802_11AESEnable)
	    && (!pTxBlk->pMacEntry)) {
		/* use Wcid as Hardware Key Index */
		GET_GroupKey_WCID(pAd, pTxBlk->Wcid, BSS0);
	}
#endif /* ADHOC_WPA2PSK_SUPPORT */

	/*
	   prepare for TXWI
	   use Wcid as Key Index
	 */

	RTMPWriteTxWI_Data(pAd, (TXWI_STRUC *) (&pTxBlk->HeaderBuf[TXINFO_SIZE]), pTxBlk);

	pTxBlk->NeedTrans = TRUE;
	HAL_WriteTxResource(pAd, pTxBlk, TRUE, &FreeNumber);

#ifdef DBG_CTRL_SUPPORT
#ifdef INCLUDE_DEBUG_QUEUE
	if (pAd->CommonCfg.DebugFlags & DBF_DBQ_TXFRAME)
		dbQueueEnqueueTxFrame((UCHAR *)(&pTxBlk->HeaderBuf[TXINFO_SIZE]), (UCHAR *)pHeader_802_11);
#endif /* INCLUDE_DEBUG_QUEUE */
#endif /* DBG_CTRL_SUPPORT */

	pAd->RalinkCounters.KickTxCount++;
	pAd->RalinkCounters.OneSecTxDoneCount++;

	/*
	   Kick out Tx
	 */
#ifdef PCIE_PS_SUPPORT
	if (!RTMP_TEST_PSFLAG(pAd, fRTMP_PS_DISABLE_TX))
#endif /* PCIE_PS_SUPPORT */
		HAL_KickOutTx(pAd, pTxBlk, pTxBlk->QueIdx);
}
#endif /* HDR_TRANS_SUPPORT */

VOID STA_ARalink_Frame_Tx(
	IN PRTMP_ADAPTER pAd,
	IN TX_BLK * pTxBlk)
{
	PUCHAR pHeaderBufPtr;
	USHORT freeCnt = 0;
	USHORT totalMPDUSize = 0;
	USHORT FirstTx, LastTxIdx;
	int frameNum = 0;
	BOOLEAN bVLANPkt;
	PQUEUE_ENTRY pQEntry;

	ASSERT(pTxBlk);

	ASSERT((pTxBlk->TxPacketList.Number == 2));

	FirstTx = LastTxIdx = 0;	/* Is it ok init they as 0? */
	while (pTxBlk->TxPacketList.Head) {
		pQEntry = RemoveHeadQueue(&pTxBlk->TxPacketList);
		pTxBlk->pPacket = QUEUE_ENTRY_TO_PACKET(pQEntry);

		if (RTMP_FillTxBlkInfo(pAd, pTxBlk) != TRUE) {
			RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket,
					    NDIS_STATUS_FAILURE);
			continue;
		}

		bVLANPkt =
		    (RTMP_GET_PACKET_VLAN(pTxBlk->pPacket) ? TRUE : FALSE);

		/* skip 802.3 header */
		pTxBlk->pSrcBufData = pTxBlk->pSrcBufHeader + LENGTH_802_3;
		pTxBlk->SrcBufLen -= LENGTH_802_3;

		/* skip vlan tag */
		if (bVLANPkt) {
			pTxBlk->pSrcBufData += LENGTH_802_1Q;
			pTxBlk->SrcBufLen -= LENGTH_802_1Q;
		}

		if (frameNum == 0) {	/* For first frame, we need to create the 802.11 header + padding(optional) + RA-AGG-LEN + SNAP Header */

			pHeaderBufPtr =
			    STA_Build_ARalink_Frame_Header(pAd, pTxBlk);

			/*
			   It's ok write the TxWI here, because the TxWI->TxWIMPDUByteCnt 
			   will be updated after final frame was handled.
			 */
			RTMPWriteTxWI_Data(pAd, (TXWI_STRUC *) (&pTxBlk->HeaderBuf[TXINFO_SIZE]), pTxBlk);


			/*
			   Insert LLC-SNAP encapsulation - 8 octets
			 */
			EXTRA_LLCSNAP_ENCAP_FROM_PKT_OFFSET(pTxBlk->pSrcBufData - 2,
							    pTxBlk->pExtraLlcSnapEncap);

			if (pTxBlk->pExtraLlcSnapEncap) {
				NdisMoveMemory(pHeaderBufPtr,
					       pTxBlk->pExtraLlcSnapEncap, 6);
				pHeaderBufPtr += 6;
				/* get 2 octets (TypeofLen) */
				NdisMoveMemory(pHeaderBufPtr, pTxBlk->pSrcBufData - 2, 2);
				pHeaderBufPtr += 2;
				pTxBlk->MpduHeaderLen += LENGTH_802_1_H;
			}
		} else {	/* For second aggregated frame, we need create the 802.3 header to headerBuf, because PCI will copy it to SDPtr0. */

			pHeaderBufPtr = &pTxBlk->HeaderBuf[0];
			pTxBlk->MpduHeaderLen = 0;

			/* 
			   A-Ralink sub-sequent frame header is the same as 802.3 header.
			   DA(6)+SA(6)+FrameType(2)
			 */
			NdisMoveMemory(pHeaderBufPtr, pTxBlk->pSrcBufHeader,
				       12);
			pHeaderBufPtr += 12;
			/* get 2 octets (TypeofLen) */
			NdisMoveMemory(pHeaderBufPtr, pTxBlk->pSrcBufData - 2,
				       2);
			pHeaderBufPtr += 2;
			pTxBlk->MpduHeaderLen = ARALINK_SUBHEAD_LEN;
		}

		totalMPDUSize += pTxBlk->MpduHeaderLen + pTxBlk->SrcBufLen;

		/* FreeNumber = GET_TXRING_FREENO(pAd, QueIdx); */
		if (frameNum == 0)
			FirstTx =
			    HAL_WriteMultiTxResource(pAd, pTxBlk, frameNum,
						     &freeCnt);
		else
			LastTxIdx =
			    HAL_WriteMultiTxResource(pAd, pTxBlk, frameNum,
						     &freeCnt);

#ifdef DBG_CTRL_SUPPORT
#ifdef INCLUDE_DEBUG_QUEUE
		if (pAd->CommonCfg.DebugFlags & DBF_DBQ_TXFRAME)
			dbQueueEnqueueTxFrame((UCHAR *)(&pTxBlk->HeaderBuf[TXINFO_SIZE]), NULL);
#endif /* INCLUDE_DEBUG_QUEUE */
#endif /* DBG_CTRL_SUPPORT */

		frameNum++;

		pAd->RalinkCounters.OneSecTxAggregationCount++;
		pAd->RalinkCounters.KickTxCount++;
		pAd->RalinkCounters.OneSecTxDoneCount++;
	}

	HAL_FinalWriteTxResource(pAd, pTxBlk, totalMPDUSize, FirstTx);
	HAL_LastTxIdx(pAd, pTxBlk->QueIdx, LastTxIdx);

	/*
	   Kick out Tx
	 */
#ifdef PCIE_PS_SUPPORT
	if (!RTMP_TEST_PSFLAG(pAd, fRTMP_PS_DISABLE_TX))
#endif /* PCIE_PS_SUPPORT */
		HAL_KickOutTx(pAd, pTxBlk, pTxBlk->QueIdx);

}


VOID STA_Fragment_Frame_Tx(
	IN RTMP_ADAPTER *pAd,
	IN TX_BLK *pTxBlk)
{
	HEADER_802_11 *pHeader_802_11;
	PUCHAR pHeaderBufPtr;
	USHORT freeCnt = 0;
	UCHAR fragNum = 0;
	PACKET_INFO PacketInfo;
	USHORT EncryptionOverhead = 0;
	UINT32 FreeMpduSize, SrcRemainingBytes;
	USHORT AckDuration;
	UINT NextMpduSize;
	BOOLEAN bVLANPkt;
	PQUEUE_ENTRY pQEntry;
	HTTRANSMIT_SETTING *pTransmit;
#ifdef SOFT_ENCRYPT
	PUCHAR tmp_ptr = NULL;
	UINT32 buf_offset = 0;
#endif /* SOFT_ENCRYPT */
	UINT8 TXWISize = pAd->chipCap.TXWISize;

	ASSERT(pTxBlk);

	pQEntry = RemoveHeadQueue(&pTxBlk->TxPacketList);
	pTxBlk->pPacket = QUEUE_ENTRY_TO_PACKET(pQEntry);
	if (RTMP_FillTxBlkInfo(pAd, pTxBlk) != TRUE) {
		RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_FAILURE);
		return;
	}

	ASSERT(TX_BLK_TEST_FLAG(pTxBlk, fTX_bAllowFrag));
	bVLANPkt = (RTMP_GET_PACKET_VLAN(pTxBlk->pPacket) ? TRUE : FALSE);

	STAFindCipherAlgorithm(pAd, pTxBlk);
	STABuildCommon802_11Header(pAd, pTxBlk);

#ifdef SOFT_ENCRYPT
	/*
	   Check if the original data has enough buffer 
	   to insert or append extended field.
	 */
	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bSwEncrypt)) {
		if (RTMPExpandPacketForSwEncrypt(pAd, pTxBlk) == FALSE) {
			RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket,
					    NDIS_STATUS_FAILURE);
			return;
		}
	}
#endif /* SOFT_ENCRYPT */

	if (pTxBlk->CipherAlg == CIPHER_TKIP) {
		pTxBlk->pPacket =
		    duplicate_pkt_with_TKIP_MIC(pAd, pTxBlk->pPacket);
		if (pTxBlk->pPacket == NULL)
			return;
		RTMP_QueryPacketInfo(pTxBlk->pPacket, &PacketInfo,
				     &pTxBlk->pSrcBufHeader,
				     &pTxBlk->SrcBufLen);
	}

	/* skip 802.3 header */
	pTxBlk->pSrcBufData = pTxBlk->pSrcBufHeader + LENGTH_802_3;
	pTxBlk->SrcBufLen -= LENGTH_802_3;

	/* skip vlan tag */
	if (bVLANPkt) {
		pTxBlk->pSrcBufData += LENGTH_802_1Q;
		pTxBlk->SrcBufLen -= LENGTH_802_1Q;
	}

	pHeaderBufPtr = &pTxBlk->HeaderBuf[TXINFO_SIZE + TXWISize];
	pHeader_802_11 = (HEADER_802_11 *) pHeaderBufPtr;

	/* skip common header */
	pHeaderBufPtr += pTxBlk->MpduHeaderLen;

	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWMM)) {
		/*
		   build QOS Control bytes
		 */
		*pHeaderBufPtr = (pTxBlk->UserPriority & 0x0F);

#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
#ifdef UAPSD_SUPPORT
		UAPSD_MR_EOSP_SET(pHeaderBufPtr, pTxBlk);
#endif /* UAPSD_SUPPORT */
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */

		*(pHeaderBufPtr + 1) = 0;
		pHeaderBufPtr += 2;
		pTxBlk->MpduHeaderLen += 2;
	}

	/*
	   padding at front of LLC header
	   LLC header should locate at 4-octets aligment
	 */
	pTxBlk->HdrPadLen = (ULONG) pHeaderBufPtr;
	pHeaderBufPtr = (PUCHAR) ROUND_UP(pHeaderBufPtr, 4);
	pTxBlk->HdrPadLen = (ULONG) (pHeaderBufPtr - pTxBlk->HdrPadLen);

#ifdef SOFT_ENCRYPT
	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bSwEncrypt)) {
		UCHAR iv_offset = 0;

		/* if original Ethernet frame contains no LLC/SNAP, */
		/* then an extra LLC/SNAP encap is required */
		EXTRA_LLCSNAP_ENCAP_FROM_PKT_OFFSET(pTxBlk->pSrcBufData - 2,
						    pTxBlk->pExtraLlcSnapEncap);

		/* Insert LLC-SNAP encapsulation (8 octets) to MPDU data buffer */
		if (pTxBlk->pExtraLlcSnapEncap) {
			/* Reserve the front 8 bytes of data for LLC header */
			pTxBlk->pSrcBufData -= LENGTH_802_1_H;
			pTxBlk->SrcBufLen += LENGTH_802_1_H;

			NdisMoveMemory(pTxBlk->pSrcBufData,
				       pTxBlk->pExtraLlcSnapEncap, 6);
		}

		/* Construct and insert specific IV header to MPDU header */
		RTMPSoftConstructIVHdr(pTxBlk->CipherAlg,
				       pTxBlk->KeyIdx,
				       pTxBlk->pKey->TxTsc,
				       pHeaderBufPtr, &iv_offset);
		pHeaderBufPtr += iv_offset;
		pTxBlk->MpduHeaderLen += iv_offset;

	} else
#endif /* SOFT_ENCRYPT */
	{


		/*
		   Insert LLC-SNAP encapsulation - 8 octets

		   if original Ethernet frame contains no LLC/SNAP, 
		   then an extra LLC/SNAP encap is required 
		 */
		EXTRA_LLCSNAP_ENCAP_FROM_PKT_START(pTxBlk->pSrcBufHeader,
						   pTxBlk->pExtraLlcSnapEncap);
		if (pTxBlk->pExtraLlcSnapEncap) {
			UCHAR vlan_size;

			NdisMoveMemory(pHeaderBufPtr,
				       pTxBlk->pExtraLlcSnapEncap, 6);
			pHeaderBufPtr += 6;
			/* skip vlan tag */
			vlan_size = (bVLANPkt) ? LENGTH_802_1Q : 0;
			/* get 2 octets (TypeofLen) */
			NdisMoveMemory(pHeaderBufPtr,
				       pTxBlk->pSrcBufHeader + 12 + vlan_size,
				       2);
			pHeaderBufPtr += 2;
			pTxBlk->MpduHeaderLen += LENGTH_802_1_H;
		}
	}

	/*
	   If TKIP is used and fragmentation is required. Driver has to
	   append TKIP MIC at tail of the scatter buffer
	   MAC ASIC will only perform IV/EIV/ICV insertion but no TKIP MIC
	 */
	if (pTxBlk->CipherAlg == CIPHER_TKIP) {
#ifdef ETH_CONVERT_SUPPORT
		/*
		   When enable dongle mode for EthernetConverter, we cannot directly calculate the
		   MIC value base on original 802.3 packet, we need use our MAC address as the
		   src MAC of 802.3 packet to calculate the MIC, so we use the "bDonglePkt" to
		   indicate if the function should calculate this packet base on origianl paylaod
		   or need to change the srcMAC as our MAC address.
		 */
		RTMPCalculateMICValue(pAd, pTxBlk->pPacket,
				      pTxBlk->pExtraLlcSnapEncap, pTxBlk->pKey,
				      (TX_BLK_TEST_FLAG
				       (pTxBlk, fTX_bDonglePkt)));
#else
		RTMPCalculateMICValue(pAd, pTxBlk->pPacket,
				      pTxBlk->pExtraLlcSnapEncap, pTxBlk->pKey,
				      0);
#endif /* ETH_CONVERT_SUPPORT */

		/*
		   NOTE: DON'T refer the skb->len directly after following copy. Becasue the length is not adjust
		   to correct lenght, refer to pTxBlk->SrcBufLen for the packet length in following progress. 
		 */
		NdisMoveMemory(pTxBlk->pSrcBufData + pTxBlk->SrcBufLen,
			       &pAd->PrivateInfo.Tx.MIC[0], 8);
		pTxBlk->SrcBufLen += 8;
		pTxBlk->TotalFrameLen += 8;
	}

	/*
	   calcuate the overhead bytes that encryption algorithm may add. This
	   affects the calculate of "duration" field
	 */
	if ((pTxBlk->CipherAlg == CIPHER_WEP64)
	    || (pTxBlk->CipherAlg == CIPHER_WEP128))
		EncryptionOverhead = 8;	/* WEP: IV[4] + ICV[4]; */
	else if (pTxBlk->CipherAlg == CIPHER_TKIP)
		EncryptionOverhead = 12;	/* TKIP: IV[4] + EIV[4] + ICV[4], MIC will be added to TotalPacketLength */
	else if (pTxBlk->CipherAlg == CIPHER_AES)
		EncryptionOverhead = 16;	/* AES: IV[4] + EIV[4] + MIC[8] */
#ifdef WAPI_SUPPORT
	else if (pTxBlk->CipherAlg == CIPHER_SMS4)
		EncryptionOverhead = 16;	/* SMS4: MIC[16] */
#endif /* WAPI_SUPPORT */
	else
		EncryptionOverhead = 0;

	pTransmit = pTxBlk->pTransmit;
	/* Decide the TX rate */
	if (pTransmit->field.MODE == MODE_CCK)
		pTxBlk->TxRate = pTransmit->field.MCS;
	else if (pTransmit->field.MODE == MODE_OFDM)
		pTxBlk->TxRate = pTransmit->field.MCS + RATE_FIRST_OFDM_RATE;
	else
		pTxBlk->TxRate = RATE_6_5;

	/* decide how much time an ACK/CTS frame will consume in the air */
	if (pTxBlk->TxRate <= RATE_LAST_OFDM_RATE)
		AckDuration =
		    RTMPCalcDuration(pAd,
				     pAd->CommonCfg.ExpectedACKRate[pTxBlk->TxRate],
				     14);
	else
		AckDuration = RTMPCalcDuration(pAd, RATE_6_5, 14);

	/* Init the total payload length of this frame. */
	SrcRemainingBytes = pTxBlk->SrcBufLen;

	pTxBlk->TotalFragNum = 0xff;

#ifdef SOFT_ENCRYPT
	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bSwEncrypt)) {
		/* store the outgoing frame for calculating MIC per fragmented frame */
		os_alloc_mem(pAd, (PUCHAR *) & tmp_ptr, pTxBlk->SrcBufLen);
		if (tmp_ptr == NULL) {
			DBGPRINT(RT_DEBUG_ERROR,
				 ("!!!%s : no memory for SW MIC calculation !!!\n",
				  __FUNCTION__));
			RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket,
					    NDIS_STATUS_FAILURE);
			return;
		}
		NdisMoveMemory(tmp_ptr, pTxBlk->pSrcBufData, pTxBlk->SrcBufLen);
	}
#endif /* SOFT_ENCRYPT */

	do {
		FreeMpduSize = pAd->CommonCfg.FragmentThreshold - LENGTH_CRC - pTxBlk->MpduHeaderLen;
		if (SrcRemainingBytes <= FreeMpduSize) {	/* this is the last or only fragment */

			pTxBlk->SrcBufLen = SrcRemainingBytes;

			pHeader_802_11->FC.MoreFrag = 0;
			pHeader_802_11->Duration =
			    pAd->CommonCfg.Dsifs + AckDuration;

			/* Indicate the lower layer that this's the last fragment. */
			pTxBlk->TotalFragNum = fragNum;
		} else {	/* more fragment is required */

			pTxBlk->SrcBufLen = FreeMpduSize;

			NextMpduSize =
			    min(((UINT) SrcRemainingBytes - pTxBlk->SrcBufLen),
				((UINT) pAd->CommonCfg.FragmentThreshold));
			pHeader_802_11->FC.MoreFrag = 1;
			pHeader_802_11->Duration =
			    (3 * pAd->CommonCfg.Dsifs) + (2 * AckDuration) +
			    RTMPCalcDuration(pAd, pTxBlk->TxRate,
					     NextMpduSize + EncryptionOverhead);
		}

		SrcRemainingBytes -= pTxBlk->SrcBufLen;

		if (fragNum == 0)
			pTxBlk->FrameGap = IFS_HTTXOP;
		else
			pTxBlk->FrameGap = IFS_SIFS;

#ifdef SOFT_ENCRYPT
		if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bSwEncrypt)) {
			UCHAR ext_offset = 0;

			NdisMoveMemory(pTxBlk->pSrcBufData,
				       tmp_ptr + buf_offset, pTxBlk->SrcBufLen);
			buf_offset += pTxBlk->SrcBufLen;

			/* Encrypt the MPDU data by software */
			RTMPSoftEncryptionAction(pAd,
						 pTxBlk->CipherAlg,
						 (PUCHAR) pHeader_802_11,
						 pTxBlk->pSrcBufData,
						 pTxBlk->SrcBufLen,
						 pTxBlk->KeyIdx,
						 pTxBlk->pKey, &ext_offset);
			pTxBlk->SrcBufLen += ext_offset;
			pTxBlk->TotalFrameLen += ext_offset;

		}
#endif /* SOFT_ENCRYPT */

		RTMPWriteTxWI_Data(pAd, (TXWI_STRUC *) (&pTxBlk->HeaderBuf[TXINFO_SIZE]), pTxBlk);
		HAL_WriteFragTxResource(pAd, pTxBlk, fragNum, &freeCnt);

#ifdef DBG_CTRL_SUPPORT
#ifdef INCLUDE_DEBUG_QUEUE
		if (pAd->CommonCfg.DebugFlags & DBF_DBQ_TXFRAME)
			dbQueueEnqueueTxFrame((UCHAR *)(&pTxBlk->HeaderBuf[TXINFO_SIZE]), (UCHAR *)pHeader_802_11);
#endif /* INCLUDE_DEBUG_QUEUE */
#endif /* DBG_CTRL_SUPPORT */

		pAd->RalinkCounters.KickTxCount++;
		pAd->RalinkCounters.OneSecTxDoneCount++;

		/* Update the frame number, remaining size of the NDIS packet payload. */
#ifdef SOFT_ENCRYPT
		if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bSwEncrypt)) {
#ifdef WAPI_SUPPORT
			if (pTxBlk->CipherAlg == CIPHER_SMS4) {
				/* incease WPI IV for next MPDU */
				inc_iv_byte(pTxBlk->pKey->TxTsc, LEN_WAPI_TSC,
					    2);
				/* Construct and insert WPI-SMS4 IV header to MPDU header */
				RTMPConstructWPIIVHdr(pTxBlk->KeyIdx,
						      pTxBlk->pKey->TxTsc,
						      pHeaderBufPtr -
						      (LEN_WPI_IV_HDR));
			} else
#endif /* WAPI_SUPPORT */
			if ((pTxBlk->CipherAlg == CIPHER_WEP64)
				    || (pTxBlk->CipherAlg == CIPHER_WEP128)) {
				inc_iv_byte(pTxBlk->pKey->TxTsc, LEN_WEP_TSC,
					    1);
				/* Construct and insert 4-bytes WEP IV header to MPDU header */
				RTMPConstructWEPIVHdr(pTxBlk->KeyIdx,
						      pTxBlk->pKey->TxTsc,
						      pHeaderBufPtr -
						      (LEN_WEP_IV_HDR));
			} else if (pTxBlk->CipherAlg == CIPHER_TKIP) ;
			else if (pTxBlk->CipherAlg == CIPHER_AES) {
				inc_iv_byte(pTxBlk->pKey->TxTsc, LEN_WPA_TSC,
					    1);
				/* Construct and insert 8-bytes CCMP header to MPDU header */
				RTMPConstructCCMPHdr(pTxBlk->KeyIdx,
						     pTxBlk->pKey->TxTsc,
						     pHeaderBufPtr -
						     (LEN_CCMP_HDR));
			}
		} else
#endif /* SOFT_ENCRYPT */
		{
			/* space for 802.11 header. */
			if (fragNum == 0 && pTxBlk->pExtraLlcSnapEncap)
				pTxBlk->MpduHeaderLen -= LENGTH_802_1_H;
		}

		fragNum++;
		/* SrcRemainingBytes -= pTxBlk->SrcBufLen; */
		pTxBlk->pSrcBufData += pTxBlk->SrcBufLen;

		pHeader_802_11->Frag++;	/* increase Frag # */

	} while (SrcRemainingBytes > 0);

#ifdef SOFT_ENCRYPT
	if (tmp_ptr != NULL)
		os_free_mem(pAd, tmp_ptr);
#endif /* SOFT_ENCRYPT */

	/*
	   Kick out Tx
	 */
#ifdef PCIE_PS_SUPPORT
	if (!RTMP_TEST_PSFLAG(pAd, fRTMP_PS_DISABLE_TX))
#endif /* PCIE_PS_SUPPORT */
		HAL_KickOutTx(pAd, pTxBlk, pTxBlk->QueIdx);
}


#define RELEASE_FRAMES_OF_TXBLK(_pAd, _pTxBlk, _pQEntry, _Status) 										\
		while(_pTxBlk->TxPacketList.Head)														\
		{																						\
			_pQEntry = RemoveHeadQueue(&_pTxBlk->TxPacketList);									\
			RELEASE_NDIS_PACKET(_pAd, QUEUE_ENTRY_TO_PACKET(_pQEntry), _Status);	\
		}


#ifdef VHT_TXBF_SUPPORT
VOID STA_NDPA_Frame_Tx(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pMacEntry)
{
	UCHAR *buf;
	VHT_NDPA_FRAME *vht_ndpa;
	struct wifi_dev *wdev;
	UINT frm_len, sta_cnt;
	SNDING_STA_INFO *sta_info;


	if (pMacEntry) 
	{
		wdev = pMacEntry->wdev;

		if (MlmeAllocateMemory(pAd, &buf) != NDIS_STATUS_SUCCESS)
			return;

		NdisZeroMemory(buf, MGMT_DMA_BUFFER_SIZE);

		vht_ndpa = (VHT_NDPA_FRAME *)buf;
		frm_len = sizeof(VHT_NDPA_FRAME);
		vht_ndpa->fc.Type = FC_TYPE_CNTL;
		vht_ndpa->fc.SubType = SUBTYPE_VHT_NDPA;
		COPY_MAC_ADDR(vht_ndpa->ra, pMacEntry->Addr);
		COPY_MAC_ADDR(vht_ndpa->ta, wdev->if_addr);

		/* Currnetly we only support 1 STA for a VHT DNPA */
		sta_info = vht_ndpa->sta_info;
		sta_info->aid12 = 0;
		sta_info->fb_type = SNDING_FB_SU;
		sta_info->nc_idx = 0;
		vht_ndpa->token.token_num = pMacEntry->snd_dialog_token;
		frm_len += sizeof(SNDING_STA_INFO);

		if (frm_len >= (MGMT_DMA_BUFFER_SIZE - sizeof(SNDING_STA_INFO))) {
			DBGPRINT(RT_DEBUG_ERROR, ("%s(): len(%d) too large!cnt=%d\n",
						__FUNCTION__, frm_len, sta_cnt));
		}

		if (pMacEntry->snd_dialog_token & 0xc0)
			pMacEntry->snd_dialog_token = 0;
		else
			pMacEntry->snd_dialog_token++;

		vht_ndpa->duration = 100;

		//DBGPRINT(RT_DEBUG_OFF, ("Send VHT NDPA Frame to STA(%02x:%02x:%02x:%02x:%02x:%02x)\n",
		//						PRINT_MAC(pMacEntry->Addr)));
		//hex_dump("VHT NDPA Frame", buf, frm_len);

		// NDPA's BW needs to sync with Tx BW
		pAd->CommonCfg.MlmeTransmit.field.BW = pMacEntry->HTPhyMode.field.BW;

		MiniportMMRequest(pAd, 0, buf, frm_len);
		MlmeFreeMemory(pAd, buf);
	}

	//pMacEntry->TxSndgType = SNDG_TYPE_DISABLE;
}
#endif /* VHT_TXBF_SUPPORT */


/*
	========================================================================

	Routine Description:
		Copy frame from waiting queue into relative ring buffer and set 
	appropriate ASIC register to kick hardware encryption before really
	sent out to air.
		
	Arguments:
		pAd 	Pointer to our adapter
		PNDIS_PACKET	Pointer to outgoing Ndis frame
		NumberOfFrag	Number of fragment required
		
	Return Value:
		None

	IRQL = DISPATCH_LEVEL
	
	Note:
	
	========================================================================
*/
#ifdef TXBF_SUPPORT
NDIS_STATUS STAHardTransmit(RTMP_ADAPTER *pAd, TX_BLK *pTxBlk, UCHAR QueIdx, UCHAR TxSndgTypePerEntry)
#else
NDIS_STATUS STAHardTransmit(RTMP_ADAPTER *pAd, TX_BLK *pTxBlk, UCHAR QueIdx)
#endif
{
	NDIS_PACKET *pPacket;
	PQUEUE_ENTRY pQEntry;

#ifdef HDR_TRANS_SUPPORT
		BOOLEAN bDoHdrTrans = TRUE;
#endif /* HDR_TRANS_SUPPORT */

	/*
	   ---------------------------------------------
	   STEP 0. DO SANITY CHECK AND SOME EARLY PREPARATION.
	   ---------------------------------------------        
	 */
	ASSERT(pTxBlk->TxPacketList.Number);
	if (pTxBlk->TxPacketList.Head == NULL) {
		DBGPRINT(RT_DEBUG_ERROR,
			 ("pTxBlk->TotalFrameNum == %ld!\n",
			  pTxBlk->TxPacketList.Number));
		return NDIS_STATUS_FAILURE;
	}

	pPacket = QUEUE_ENTRY_TO_PACKET(pTxBlk->TxPacketList.Head);

	/* there's packet to be sent, keep awake for 1200ms */
	if (pAd->CountDowntoPsm < 12)
		pAd->CountDowntoPsm = 12;

	/* ------------------------------------------------------------------
	   STEP 1. WAKE UP PHY
	   outgoing frame always wakeup PHY to prevent frame lost and 
	   turn off PSM bit to improve performance
	   ------------------------------------------------------------------
	   not to change PSM bit, just send this frame out?
	 */
	if ((pAd->StaCfg.Psm == PWR_SAVE)
	    && OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE)) {
		DBGPRINT_RAW(RT_DEBUG_INFO, ("AsicForceWakeup At HardTx\n"));
#ifdef RTMP_MAC_PCI
		AsicForceWakeup(pAd, TRUE);
#endif /* RTMP_MAC_PCI */
	}

	/* It should not change PSM bit, when APSD turn on. */
	if ((!
	     (pAd->StaCfg.UapsdInfo.bAPSDCapable
	      && pAd->CommonCfg.APEdcaParm.bAPSDCapable)
	     && (pAd->CommonCfg.bAPSDForcePowerSave == FALSE))
	    || (RTMP_GET_PACKET_EAPOL(pTxBlk->pPacket))
	    || (RTMP_GET_PACKET_WAI(pTxBlk->pPacket))) {
		if ((RtmpPktPmBitCheck(pAd) == TRUE) &&
		    (pAd->StaCfg.WindowsPowerMode ==
		     Ndis802_11PowerModeFast_PSP))
			RTMP_SET_PSM_BIT(pAd, PWR_ACTIVE);
	}

#ifdef HDR_TRANS_SUPPORT
#ifdef SOFT_ENCRYPT
	if ( TX_BLK_TEST_FLAG(pTxBlk, fTX_bSwEncrypt)) /* need LLC, not yet generated */
		bDoHdrTrans = FALSE;
	else
#endif /* SOFT_ENCRYPT */
#ifdef XLINK_SUPPORT
	if ( pAd->StaCfg.PSPXlink ) /* Use SA as Address 2 */
		bDoHdrTrans = FALSE;
	else
#endif /* XLINK_SUPPORT */
#ifdef CLIENT_WDS
	if (!MAC_ADDR_EQUAL		/* 4 MAC Addresses */
			((pTxBlk->pSrcBufHeader + MAC_ADDR_LEN),
			pAd->CurrentAddress)) 
		bDoHdrTrans = FALSE;
	else
#endif /* CLIENT_WDS */
	{
#ifdef TXBF_SUPPORT
		bDoHdrTrans = FALSE;
#endif // TXBF_SUPPORT //
	}
#endif /* HDR_TRANS_SUPPORT */

	switch (pTxBlk->TxFrameType) {
#ifdef DOT11_N_SUPPORT
	case TX_AMPDU_FRAME:
#ifdef HDR_TRANS_SUPPORT
		if (bDoHdrTrans)
			STA_AMPDU_Frame_Tx_Hdr_Trns(pAd, pTxBlk);
		else
#endif /* HDR_TRANS_SUPPORT */
#ifdef TXBF_SUPPORT
		STA_AMPDU_Frame_Tx(pAd, pTxBlk, TxSndgTypePerEntry);
#else
		STA_AMPDU_Frame_Tx(pAd, pTxBlk);
#endif

		break;
	case TX_AMSDU_FRAME:
		STA_AMSDU_Frame_Tx(pAd, pTxBlk);
		break;
#endif /* DOT11_N_SUPPORT */
	case TX_LEGACY_FRAME:
		{
#ifdef HDR_TRANS_SUPPORT
		if (bDoHdrTrans)
			STA_Legacy_Frame_Tx_Hdr_Trns(pAd, pTxBlk);
		else
#endif /* HDR_TRANS_SUPPORT */			
			STA_Legacy_Frame_Tx(pAd, pTxBlk);
		break;
		}
	case TX_MCAST_FRAME:
		STA_Legacy_Frame_Tx(pAd, pTxBlk);
		break;
	case TX_RALINK_FRAME:
		STA_ARalink_Frame_Tx(pAd, pTxBlk);
		break;
	case TX_FRAG_FRAME:
		STA_Fragment_Frame_Tx(pAd, pTxBlk);
		break;
	default:
		{
			/* It should not happened! */
			DBGPRINT(RT_DEBUG_ERROR,
				 ("Send a pacekt was not classified!! It should not happen!\n"));
			while (pTxBlk->TxPacketList.Number) {
				pQEntry =
				    RemoveHeadQueue(&pTxBlk->TxPacketList);
				pPacket = QUEUE_ENTRY_TO_PACKET(pQEntry);
				if (pPacket)
					RELEASE_NDIS_PACKET(pAd, pPacket,
							    NDIS_STATUS_FAILURE);
			}
		}
		break;
	}

	return (NDIS_STATUS_SUCCESS);

}


VOID Sta_Announce_or_Forward_802_3_Packet(
	IN RTMP_ADAPTER *pAd,
	IN PNDIS_PACKET pPacket,
	IN UCHAR FromWhichBSSID)
{
	if (TRUE
	) {
		announce_802_3_packet(pAd, pPacket, OPMODE_STA);
	} else {
		/* release packet */
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
	}
}

