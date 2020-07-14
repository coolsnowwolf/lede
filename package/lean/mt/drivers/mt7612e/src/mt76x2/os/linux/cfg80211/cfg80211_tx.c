/****************************************************************************
 * Ralink Tech Inc.
 * Taiwan, R.O.C.
 *
 * (c) Copyright 2013, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************/

/****************************************************************************
 
	Abstract:

	All related CFG80211 P2P function body.

	History:

***************************************************************************/

#define RTMP_MODULE_OS

#ifdef RT_CFG80211_SUPPORT

#include "rt_config.h"

VOID CFG80211_SwitchTxChannel(RTMP_ADAPTER *pAd, ULONG Data)
{
	//UCHAR lock_channel = CFG80211_getCenCh(pAd, Data);
	UCHAR lock_channel = Data;
	if (pAd->LatchRfRegs.Channel != lock_channel)
	{
		AsicSwitchChannel(pAd, lock_channel, FALSE);
		AsicLockChannel(pAd, lock_channel);
		
		DBGPRINT(RT_DEBUG_INFO, ("Off-Channel Send Packet: From(%d)-To(%d)\n", 
									pAd->LatchRfRegs.Channel, lock_channel));
	}
	else
		DBGPRINT(RT_DEBUG_INFO, ("Off-Channel Channel Equal: %d\n", pAd->LatchRfRegs.Channel));

}

#ifdef CONFIG_AP_SUPPORT
BOOLEAN CFG80211_SyncPacketWmmIe(RTMP_ADAPTER *pAd, VOID *pData, ULONG dataLen)
{
	const UINT WFA_OUI = 0x0050F2;
	const UCHAR WMM_OUI_TYPE = 0x2;
	UCHAR *wmm_ie = NULL;

	return;//????
	//hex_dump("probe_rsp_in:", pData, dataLen);
	wmm_ie = cfg80211_find_vendor_ie(WFA_OUI, WMM_OUI_TYPE, pData, dataLen);

	if (wmm_ie != NULL)
        {
#ifdef UAPSD_SUPPORT
                if (pAd->ApCfg.MBSSID[0].UapsdInfo.bAPSDCapable == TRUE)
                {
                        wmm_ie[8] |= 0x80;
                }
#endif /* UAPSD_SUPPORT */


                UINT i = QID_AC_BE;
                /* WMM: sync from driver's EDCA paramter */
                for (i = QID_AC_BE; i <= QID_AC_VO; i++)
                {

                        wmm_ie[10+ (i*4)] = (i << 5) +                                     /* b5-6 is ACI */
                                            ((UCHAR)pAd->ApCfg.BssEdcaParm.bACM[i] << 4) + /* b4 is ACM */
                                            (pAd->ApCfg.BssEdcaParm.Aifsn[i] & 0x0f);      /* b0-3 is AIFSN */

                        wmm_ie[11+ (i*4)] = (pAd->ApCfg.BssEdcaParm.Cwmax[i] << 4) +       /* b5-8 is CWMAX */
                                            (pAd->ApCfg.BssEdcaParm.Cwmin[i] & 0x0f);      /* b0-3 is CWMIN */
                        wmm_ie[12+ (i*4)] = (UCHAR)(pAd->ApCfg.BssEdcaParm.Txop[i] & 0xff);/* low byte of TXOP */
                        wmm_ie[13+ (i*4)] = (UCHAR)(pAd->ApCfg.BssEdcaParm.Txop[i] >> 8);  /* high byte of TXOP */
                }

		return TRUE;
        }

	return FALSE;	
}
VOID CFG80211_ParseBeaconIE(RTMP_ADAPTER *pAd, MULTISSID_STRUCT *pMbss, struct wifi_dev *wdev,UCHAR *wpa_ie,UCHAR *rsn_ie)
{
	PEID_STRUCT 		 pEid;
	PUCHAR				pTmp;
	NDIS_802_11_ENCRYPTION_STATUS	TmpCipher;
	NDIS_802_11_ENCRYPTION_STATUS	PairCipher;		/* Unicast cipher 1, this one has more secured cipher suite */
	NDIS_802_11_ENCRYPTION_STATUS	PairCipherAux;	/* Unicast cipher 2 if AP announce two unicast cipher suite */
	NDIS_802_11_ENCRYPTION_STATUS	GroupCipher;	/* Group cipher */
	PAKM_SUITE_STRUCT				pAKM;
	USHORT							Count;
	BOOLEAN bWPA = FALSE;
	BOOLEAN bWPA2 = FALSE;
	BOOLEAN bMix = FALSE;

		/* Security */
	PairCipher	 = Ndis802_11WEPDisabled;
	PairCipherAux = Ndis802_11WEPDisabled;
	
	if ((wpa_ie == NULL) && (rsn_ie == NULL)) //open case
	{
		DBGPRINT(RT_DEBUG_TRACE,("%s:: Open/None case\n", __FUNCTION__));
		wdev->AuthMode = Ndis802_11AuthModeOpen;
		wdev->WepStatus = Ndis802_11WEPDisabled;
		wdev->WpaMixPairCipher = MIX_CIPHER_NOTUSE;
	}
	
	 if ((wpa_ie != NULL)) //wpapsk/tkipaes case
	{
		pEid = wpa_ie;
		pTmp = pEid;
		if (NdisEqualMemory(pEid->Octet, WPA_OUI, 4))
		{
			wdev->AuthMode = Ndis802_11AuthModeOpen;
			DBGPRINT(RT_DEBUG_TRACE,("%s:: WPA case\n", __FUNCTION__));
			bWPA = TRUE;
			pTmp   += 11;
				switch (*pTmp)
				{
					case 1:
						DBGPRINT(RT_DEBUG_TRACE,("Group Ndis802_11GroupWEP40Enabled\n"));
						wdev->GroupKeyWepStatus  = Ndis802_11GroupWEP40Enabled;
						break;
					case 5:
						DBGPRINT(RT_DEBUG_TRACE,("Group Ndis802_11GroupWEP104Enabled\n"));
						wdev->GroupKeyWepStatus  = Ndis802_11GroupWEP104Enabled;
						break;
					case 2:
						DBGPRINT(RT_DEBUG_TRACE,("Group Ndis802_11TKIPEnable\n"));
						wdev->GroupKeyWepStatus  = Ndis802_11TKIPEnable;
						break;
					case 4:
						DBGPRINT(RT_DEBUG_TRACE,(" Group Ndis802_11AESEnable\n"));
						wdev->GroupKeyWepStatus  = Ndis802_11AESEnable;
						break;
					default:
						break;
				}
				/* number of unicast suite*/
				pTmp   += 1;

				/* skip all unicast cipher suites*/
				/*Count = *(PUSHORT) pTmp;				*/
				Count = (pTmp[1]<<8) + pTmp[0];
				pTmp   += sizeof(USHORT);

				/* Parsing all unicast cipher suite*/
				while (Count > 0)
				{
					/* Skip OUI*/
					pTmp += 3;
					TmpCipher = Ndis802_11WEPDisabled;
					switch (*pTmp)
					{
						case 1:
						case 5: /* Although WEP is not allowed in WPA related auth mode, we parse it anyway*/
							TmpCipher = Ndis802_11WEPEnabled;
							break;
						case 2:
							TmpCipher = Ndis802_11TKIPEnable;
							break;
						case 4:
							TmpCipher = Ndis802_11AESEnable;
							break;
						default:
							break;
					}
					if (TmpCipher > PairCipher)
					{
						/* Move the lower cipher suite to PairCipherAux*/
						PairCipherAux = PairCipher;
						PairCipher	= TmpCipher;
					}
					else
					{
						PairCipherAux = TmpCipher;
					}
					pTmp++;
					Count--;
				}
				switch (*pTmp)
				{
					case 1:
						/* Set AP support WPA-enterprise mode*/
							wdev->AuthMode = Ndis802_11AuthModeWPA;
						break;
					case 2:
						/* Set AP support WPA-PSK mode*/
							wdev->AuthMode = Ndis802_11AuthModeWPAPSK;
						break;
					default:
						break;
				}
				pTmp   += 1;

					DBGPRINT(RT_DEBUG_TRACE,("AuthMode = %s\n",GetAuthMode(wdev->AuthMode)));
					if (wdev->GroupKeyWepStatus == PairCipher)
					{
						wdev->WpaMixPairCipher = MIX_CIPHER_NOTUSE;
						pMbss->wdev.WepStatus=wdev->GroupKeyWepStatus;
					}
					else
					{
						DBGPRINT(RT_DEBUG_TRACE,("WPA Mix TKIPAES\n"));

						bMix = TRUE;
					}
				pMbss->RSNIE_Len[0] = wpa_ie[1];
				NdisMoveMemory(pMbss->RSN_IE[0], wpa_ie+2, wpa_ie[1]);//copy rsn ie			
		}
		else {
			DBGPRINT(RT_DEBUG_TRACE,("%s:: Open/None case\n", __FUNCTION__));
			wdev->AuthMode = Ndis802_11AuthModeOpen;		
		}	
	}
	if ((rsn_ie != NULL))
	{
		PRSN_IE_HEADER_STRUCT			pRsnHeader;
		PCIPHER_SUITE_STRUCT			pCipher;

		pEid = rsn_ie;
		pTmp = pEid;
		pRsnHeader = (PRSN_IE_HEADER_STRUCT) pTmp;
				
				/* 0. Version must be 1*/
		if (le2cpu16(pRsnHeader->Version) == 1)
		{
			pTmp   += sizeof(RSN_IE_HEADER_STRUCT);

			/* 1. Check group cipher*/
			pCipher = (PCIPHER_SUITE_STRUCT) pTmp;		

			if (NdisEqualMemory(pTmp, RSN_OUI, 3))
			{	
				DBGPRINT(RT_DEBUG_TRACE,("%s:: WPA2 case\n", __FUNCTION__));
				bWPA2 = TRUE;
				wdev->AuthMode = Ndis802_11AuthModeOpen;
					switch (pCipher->Type)
					{
						case 1:
							DBGPRINT(RT_DEBUG_TRACE,("Ndis802_11GroupWEP40Enabled\n"));
							wdev->GroupKeyWepStatus = Ndis802_11GroupWEP40Enabled;
							break;
						case 5:
							DBGPRINT(RT_DEBUG_TRACE,("Ndis802_11GroupWEP104Enabled\n"));
							wdev->GroupKeyWepStatus = Ndis802_11GroupWEP104Enabled;
							break;
						case 2:
							DBGPRINT(RT_DEBUG_TRACE,("Ndis802_11TKIPEnable\n"));
							wdev->GroupKeyWepStatus = Ndis802_11TKIPEnable;
							break;
						case 4:
							DBGPRINT(RT_DEBUG_TRACE,("Ndis802_11AESEnable\n"));
							wdev->GroupKeyWepStatus = Ndis802_11AESEnable;
							break;
						default:
							break;
					}

					/* set to correct offset for next parsing*/
					pTmp   += sizeof(CIPHER_SUITE_STRUCT);

					/* 2. Get pairwise cipher counts*/
					/*Count = *(PUSHORT) pTmp;*/
					Count = (pTmp[1]<<8) + pTmp[0];
					pTmp   += sizeof(USHORT);			

					/* 3. Get pairwise cipher*/
					/* Parsing all unicast cipher suite*/
					while (Count > 0)
					{
						/* Skip OUI*/
						pCipher = (PCIPHER_SUITE_STRUCT) pTmp;
						TmpCipher = Ndis802_11WEPDisabled;
						switch (pCipher->Type)
						{
							case 1:
							case 5: /* Although WEP is not allowed in WPA related auth mode, we parse it anyway*/
								TmpCipher = Ndis802_11WEPEnabled;
								break;
							case 2:
								TmpCipher = Ndis802_11TKIPEnable;
								break;
							case 4:
								TmpCipher = Ndis802_11AESEnable;
								break;
							default:
								break;
						}

						//pMbss->wdev.WepStatus = TmpCipher;
						if (TmpCipher > PairCipher)
						{
							/* Move the lower cipher suite to PairCipherAux*/
							PairCipherAux = PairCipher;
							PairCipher	 = TmpCipher;
						}
						else
						{
							PairCipherAux = TmpCipher;
						}
						pTmp += sizeof(CIPHER_SUITE_STRUCT);
						Count--;
					}

					/* 4. get AKM suite counts*/
					/*Count	= *(PUSHORT) pTmp;*/
					Count = (pTmp[1]<<8) + pTmp[0];
					pTmp   += sizeof(USHORT);

					/* 5. Get AKM ciphers*/
					/* Parsing all AKM ciphers*/
					while (Count > 0)
					{
						pAKM = (PAKM_SUITE_STRUCT) pTmp;
						if (!RTMPEqualMemory(pTmp, RSN_OUI, 3))
							break;

						switch (pAKM->Type)
						{
							case 0:
									wdev->AuthMode = Ndis802_11AuthModeWPANone;
								break;                                                        
							case 1:
								/* Set AP support WPA-enterprise mode*/
									wdev->AuthMode = Ndis802_11AuthModeWPA2;
								break;
							case 2:                                                      
								/* Set AP support WPA-PSK mode*/
									wdev->AuthMode = Ndis802_11AuthModeWPA2PSK;
								break;
							default:
									wdev->AuthMode = Ndis802_11AuthModeMax;
								break;
						}
						pTmp   += sizeof(AKM_SUITE_STRUCT);
						Count--;
					}		
					DBGPRINT(RT_DEBUG_TRACE,("AuthMode = %s\n",GetAuthMode(wdev->AuthMode)));
					if (wdev->GroupKeyWepStatus == PairCipher)
					{
						wdev->WpaMixPairCipher = MIX_CIPHER_NOTUSE;
						pMbss->wdev.WepStatus=wdev->GroupKeyWepStatus;
					}
					else
					{
						DBGPRINT(RT_DEBUG_TRACE,("WPA2 Mix TKIPAES\n"));
						bMix= TRUE;
					}
					pMbss->RSNIE_Len[0] = rsn_ie[1];
					NdisMoveMemory(pMbss->RSN_IE[0], rsn_ie+2, rsn_ie[1]);//copy rsn ie			
			}
			else {
				DBGPRINT(RT_DEBUG_TRACE,("%s:: Open/None case\n", __FUNCTION__));
				wdev->AuthMode = Ndis802_11AuthModeOpen;			
			}
		}
	}


		if (bWPA2 && bWPA)
		{
			if (bMix)
			{
				DBGPRINT(RT_DEBUG_TRACE,("WPAPSK/WPA2PSK Mix TKIPAES\n"));
				wdev->WpaMixPairCipher = WPA_TKIPAES_WPA2_TKIPAES;
				wdev->WepStatus = Ndis802_11TKIPAESMix;
			}
		} else if (bWPA2) {
			if (bMix)
			{
				DBGPRINT(RT_DEBUG_TRACE,("WPA2PSK Mix TKIPAES\n"));
				wdev->WpaMixPairCipher = WPA_NONE_WPA2_TKIPAES;
				wdev->WepStatus = Ndis802_11TKIPAESMix;
			}		
		} else if (bWPA) {
			if (bMix)
			{
				DBGPRINT(RT_DEBUG_TRACE,("WPAPSK Mix TKIPAES\n"));
				wdev->WpaMixPairCipher = WPA_TKIPAES_WPA2_NONE;
				wdev->WepStatus = Ndis802_11TKIPAESMix;
			}
		}	
	}

#endif /* CONFIG_AP_SUPPORT */

static
PCFG80211_TX_PACKET CFG80211_TxMgmtFrameSearch(RTMP_ADAPTER *pAd, USHORT Sequence)
{
	PLIST_HEADER  pPacketList = &pAd->cfg80211_ctrl.cfg80211TxPacketList;
	PCFG80211_TX_PACKET pTxPkt = NULL;
	PLIST_ENTRY pListEntry = NULL;

	DBGPRINT(RT_DEBUG_ERROR, ("CFG_TX_STATUS: Search %d\n", Sequence));
	pListEntry = pPacketList->pHead;
	pTxPkt = (PCFG80211_TX_PACKET)pListEntry;

	while (pTxPkt != NULL)
	{
		if (pTxPkt->TxStatusSeq == Sequence)
		{
			DBGPRINT(RT_DEBUG_ERROR, ("CFG_TX_STATUS: got %d\n", Sequence));
			return pTxPkt;
		}	
		
		pListEntry = pListEntry->pNext;
		pTxPkt = (PCFG80211_TX_PACKET)pListEntry;
	}	

}

INT CFG80211_SendMgmtFrame(RTMP_ADAPTER *pAd, VOID *pData, ULONG Data)
{
	if (pData != NULL) 
	{
		{		
			PCFG80211_CTRL pCfg80211_ctrl = &pAd->cfg80211_ctrl;

			pCfg80211_ctrl->TxStatusInUsed = TRUE;
			pCfg80211_ctrl->TxStatusSeq = pAd->Sequence;

			if (pCfg80211_ctrl->pTxStatusBuf != NULL)
			{
				os_free_mem(NULL, pCfg80211_ctrl->pTxStatusBuf);
				pCfg80211_ctrl->pTxStatusBuf = NULL;
			}

			os_alloc_mem(NULL, (UCHAR **)&pCfg80211_ctrl->pTxStatusBuf, Data);
			if (pCfg80211_ctrl->pTxStatusBuf != NULL)
			{
				NdisCopyMemory(pCfg80211_ctrl->pTxStatusBuf, pData, Data);
				pCfg80211_ctrl->TxStatusBufLen = Data;
			}
			else
			{
				pCfg80211_ctrl->TxStatusBufLen = 0;
				DBGPRINT(RT_DEBUG_ERROR, ("CFG_TX_STATUS: MEM ALLOC ERROR\n"));
				return NDIS_STATUS_FAILURE;
			}
			CFG80211_CheckActionFrameType(pAd, "TX", pData, Data);

#ifdef CONFIG_AP_SUPPORT
			struct ieee80211_mgmt *mgmt;
        		mgmt = (struct ieee80211_mgmt *)pData;
        		if (ieee80211_is_probe_resp(mgmt->frame_control))
			{
				//BOOLEAN res;
				INT offset = sizeof(HEADER_802_11) + 12;
				CFG80211_SyncPacketWmmIe(pAd, pData + offset , Data - offset);
				//hex_dump("probe_rsp:", pData, Data);
			}
#endif /* CONFIG_AP_SUPPORT */

			MiniportMMRequest(pAd, 0, pData, Data);
		}
	}

}

VOID CFG80211_SendMgmtFrameDone(RTMP_ADAPTER *pAd, USHORT Sequence)
{
//RTMP_USB_SUPPORT/RTMP_PCI_SUPPORT
	PCFG80211_CTRL pCfg80211_ctrl = &pAd->cfg80211_ctrl;

	if (pCfg80211_ctrl->TxStatusInUsed && pCfg80211_ctrl->pTxStatusBuf 
		/*&& (pAd->TxStatusSeq == pHeader->Sequence)*/)
	{
		DBGPRINT(RT_DEBUG_INFO, ("CFG_TX_STATUS: REAL send %d\n", Sequence));
		
		CFG80211OS_TxStatus(CFG80211_GetEventDevice(pAd), 5678, 
							pCfg80211_ctrl->pTxStatusBuf, pCfg80211_ctrl->TxStatusBufLen, 
							TRUE);
		pCfg80211_ctrl->TxStatusSeq = 0;
		pCfg80211_ctrl->TxStatusInUsed = FALSE;
	} 


}

#endif /* RT_CFG80211_SUPPORT */


