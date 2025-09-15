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
     ap_sanity.c
     
     Abstract:
     Handle association related requests either from WSTA or from local MLME
     
     Revision History:
     Who         When          What
     --------    ----------    ----------------------------------------------
     John Chang  08-14-2003    created for 11g soft-AP
     John Chang  12-30-2004    merge with STA driver for RT2600
*/

#include "rt_config.h"

extern UCHAR	CISCO_OUI[];

extern UCHAR	WPA_OUI[];
extern UCHAR	RSN_OUI[];
extern UCHAR	WME_INFO_ELEM[];
extern UCHAR	WME_PARM_ELEM[];
extern UCHAR	RALINK_OUI[];
extern UCHAR	MTK_OUI[];

extern UCHAR 	BROADCOM_OUI[]; 
extern UCHAR    WPS_OUI[];

/* 
    ==========================================================================
    Description:
        MLME message sanity check
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
 */

BOOLEAN PeerAssocReqCmmSanity(
	RTMP_ADAPTER *pAd, 
	BOOLEAN isReassoc,
	VOID *Msg,
	INT MsgLen,
	IE_LISTS *ie_lists)
{
    PFRAME_802_11	Fr = (PFRAME_802_11)Msg;
    PEID_STRUCT eid_ptr;
    UCHAR Sanity = 0;
    UCHAR WPA1_OUI[4] = { 0x00, 0x50, 0xF2, 0x01 };
    UCHAR WPA2_OUI[3] = { 0x00, 0x0F, 0xAC };
#ifdef CONFIG_HOTSPOT_R2
	UCHAR HS2_OSEN_OUI[4] = { 0x50, 0x6f, 0x9a, 0x12 };
	UCHAR HS2OUIBYTE[4] = {0x50, 0x6f, 0x9a, 0x10};
#endif    

    MAC_TABLE_ENTRY *pEntry = (MAC_TABLE_ENTRY *)NULL;
#ifdef DOT11R_FT_SUPPORT
	PFT_INFO pFtInfo = &ie_lists->FtInfo;
#endif /* DOT11R_FT_SUPPORT */
#ifdef DOT11K_RRM_SUPPORT
	RRM_EN_CAP_IE *pRrmEnCap = &ie_lists->RrmEnCap;
#endif /* DOT11K_RRM_SUPPORT */
	HT_CAPABILITY_IE *pHtCapability = &ie_lists->HTCapability;


	pEntry = MacTableLookup(pAd, &Fr->Hdr.Addr2[0]);
	if (pEntry == NULL)
		return FALSE;

	COPY_MAC_ADDR(&ie_lists->Addr2[0], &Fr->Hdr.Addr2[0]);
	
	NdisMoveMemory(&ie_lists->CapabilityInfo, &Fr->Octet[0], 2);
	NdisMoveMemory(&ie_lists->ListenInterval, &Fr->Octet[2], 2);

	if (isReassoc) 
	{
		NdisMoveMemory(&ie_lists->ApAddr[0], &Fr->Octet[4], 6);
		eid_ptr = (PEID_STRUCT) &Fr->Octet[10];
	}
	else
	{
		eid_ptr = (PEID_STRUCT) &Fr->Octet[4];
	}


    /* get variable fields from payload and advance the pointer */
    while (((UCHAR *)eid_ptr + eid_ptr->Len + 1) < ((UCHAR *)Fr + MsgLen))
    {
        switch(eid_ptr->Eid)
        {
            case IE_SSID:
			if (((Sanity&0x1) == 1))
				break;

                if ((eid_ptr->Len <= MAX_LEN_OF_SSID))
                {
                    Sanity |= 0x01;
                    NdisMoveMemory(&ie_lists->Ssid[0], eid_ptr->Octet, eid_ptr->Len);
                    ie_lists->SsidLen = eid_ptr->Len;
                    DBGPRINT(RT_DEBUG_TRACE, ("PeerAssocReqSanity - SsidLen = %d  \n", ie_lists->SsidLen));
                }
                else
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("PeerAssocReqSanity - wrong IE_SSID\n"));
                    return FALSE;
                }
                break;

            case IE_SUPP_RATES:
                if ((eid_ptr->Len <= MAX_LEN_OF_SUPPORTED_RATES) &&
					(eid_ptr->Len > 0))
                {
                    Sanity |= 0x02;
                    NdisMoveMemory(&ie_lists->SupportedRates[0], eid_ptr->Octet, eid_ptr->Len);

                    DBGPRINT(RT_DEBUG_TRACE,
						("PeerAssocReqSanity - IE_SUPP_RATES., Len=%d. "
						"Rates[0]=%x\n", eid_ptr->Len, ie_lists->SupportedRates[0]));
                    DBGPRINT(RT_DEBUG_TRACE,
						("Rates[1]=%x %x %x %x %x %x %x\n",
						ie_lists->SupportedRates[1], ie_lists->SupportedRates[2],
						ie_lists->SupportedRates[3], ie_lists->SupportedRates[4],
						ie_lists->SupportedRates[5], ie_lists->SupportedRates[6],
						ie_lists->SupportedRates[7]));

                    ie_lists->SupportedRatesLen = eid_ptr->Len;
                }
                else
                {
					UCHAR RateDefault[8] = \
							{ 0x82, 0x84, 0x8b, 0x96, 0x12, 0x24, 0x48, 0x6c };

                	/* HT rate not ready yet. return true temporarily. rt2860c */
                    /*DBGPRINT(RT_DEBUG_TRACE, ("PeerAssocReqSanity - wrong IE_SUPP_RATES\n")); */
                    Sanity |= 0x02;
                    ie_lists->SupportedRatesLen = 8;
					NdisMoveMemory(&ie_lists->SupportedRates[0], RateDefault, 8);

                    DBGPRINT(RT_DEBUG_TRACE,
						("PeerAssocReqSanity - wrong IE_SUPP_RATES., Len=%d\n",
						eid_ptr->Len));
                }
                break;

            case IE_EXT_SUPP_RATES:
                if (eid_ptr->Len + ie_lists->SupportedRatesLen <= MAX_LEN_OF_SUPPORTED_RATES)
                {
                    NdisMoveMemory(&ie_lists->SupportedRates[ie_lists->SupportedRatesLen], eid_ptr->Octet,
									eid_ptr->Len);
                    ie_lists->SupportedRatesLen += eid_ptr->Len;
                }
                else
                {
                    NdisMoveMemory(&ie_lists->SupportedRates[ie_lists->SupportedRatesLen], eid_ptr->Octet,
									MAX_LEN_OF_SUPPORTED_RATES - (ie_lists->SupportedRatesLen));
                    ie_lists->SupportedRatesLen = MAX_LEN_OF_SUPPORTED_RATES;
                }
                break;
                
            case IE_HT_CAP:
			if (eid_ptr->Len >= sizeof(HT_CAPABILITY_IE))
			{
				NdisMoveMemory(pHtCapability, eid_ptr->Octet, SIZE_HT_CAP_IE);

				*(USHORT *)(&pHtCapability->HtCapInfo) = cpu2le16(*(USHORT *)(&pHtCapability->HtCapInfo));

#ifdef UNALIGNMENT_SUPPORT
				{
					EXT_HT_CAP_INFO extHtCapInfo;

					NdisMoveMemory((PUCHAR)(&extHtCapInfo), (PUCHAR)(&pHtCapability->ExtHtCapInfo), sizeof(EXT_HT_CAP_INFO));
					*(USHORT *)(&extHtCapInfo) = cpu2le16(*(USHORT *)(&extHtCapInfo));
					NdisMoveMemory((PUCHAR)(&pHtCapability->ExtHtCapInfo), (PUCHAR)(&extHtCapInfo), sizeof(EXT_HT_CAP_INFO));		
				}
#else				
				*(USHORT *)(&pHtCapability->ExtHtCapInfo) = cpu2le16(*(USHORT *)(&pHtCapability->ExtHtCapInfo));
#endif /* UNALIGNMENT_SUPPORT */

				ie_lists->ht_cap_len = SIZE_HT_CAP_IE;
				Sanity |= 0x10;
				DBGPRINT(RT_DEBUG_WARN, ("PeerAssocReqSanity - IE_HT_CAP\n"));
			}
			else
			{
				DBGPRINT(RT_DEBUG_WARN, ("PeerAssocReqSanity - wrong IE_HT_CAP.eid_ptr->Len = %d\n", eid_ptr->Len));
			}
				
		break;
		case IE_EXT_CAPABILITY:
			if (eid_ptr->Len)
			{
				INT ext_len = eid_ptr->Len;

				ext_len = ext_len > sizeof(EXT_CAP_INFO_ELEMENT) ? sizeof(EXT_CAP_INFO_ELEMENT) : ext_len;
				NdisMoveMemory(&ie_lists->ExtCapInfo, eid_ptr->Octet, ext_len);
				DBGPRINT(RT_DEBUG_WARN, ("PeerAssocReqSanity - IE_EXT_CAPABILITY!\n"));
			}

			break;

            case IE_WPA:    /* same as IE_VENDOR_SPECIFIC */
            case IE_WPA2:
#ifdef CONFIG_HOTSPOT_R2
				if (NdisEqualMemory(eid_ptr->Octet, HS2OUIBYTE, sizeof(HS2OUIBYTE)) && (eid_ptr->Len >= 5))
				{
					//UCHAR tmp2 = 0x12;
					UCHAR *hs2_config = (UCHAR *)&eid_ptr->Octet[4];
					UCHAR ppomo_exist = ((*hs2_config) >> 1) & 0x01;
					UCHAR hs2_version = ((*hs2_config) >> 4) & 0x0f;
					//UCHAR *tmp3 = (UCHAR *)&pEntry->hs_info.ppsmo_id;
					//UCHAR tmp[2] = {0x12,0x34};

					pEntry->hs_info.version = hs2_version;
					pEntry->hs_info.ppsmo_exist = ppomo_exist;	
					if (pEntry->hs_info.ppsmo_exist)
					{
						NdisMoveMemory(&pEntry->hs_info.ppsmo_id, &eid_ptr->Octet[5], 2);
						//NdisMoveMemory(tmp3, tmp, 2);
					}				
					printk("Assoc HS2 STA:version:%d,ppomo exist:%d, value:0x%x\n", pEntry->hs_info.version, pEntry->hs_info.ppsmo_exist, pEntry->hs_info.ppsmo_id);
					break;
				}
#endif /* CONFIG_HOTSPOT_R2 */
#ifdef DOT11R_FT_SUPPORT
#endif /* DOT11R_FT_SUPPORT */


				if (NdisEqualMemory(eid_ptr->Octet, WPS_OUI, 4))
				{
#ifdef WSC_AP_SUPPORT
#ifdef WSC_V2_SUPPORT
					if ((pAd->ApCfg.MBSSID[pEntry->apidx].WscControl.WscV2Info.bWpsEnable) ||
						(pAd->ApCfg.MBSSID[pEntry->apidx].WscControl.WscV2Info.bEnableWpsV2 == FALSE))
#endif /* WSC_V2_SUPPORT */
						ie_lists->bWscCapable = TRUE;
#endif /* WSC_AP_SUPPORT */
					break;
				}
#ifdef SMART_MESH					
				if ((eid_ptr->Len >= NTGR_OUI_LEN) && NdisEqualMemory(eid_ptr->Octet, NETGEAR_OUI, NTGR_OUI_LEN))
				{
					if(pEntry)
					{
						if(eid_ptr->Len > NTGR_OUI_LEN)
						{
							PSMART_MESH_CFG pSmartMeshCfg = NULL;
							pSmartMeshCfg = &pAd->ApCfg.MBSSID[pEntry->apidx].SmartMeshCfg;
							if ((eid_ptr->Octet[3] & 0x02))
							{
								pEntry->bSupportSmartMesh = TRUE;
								if(pSmartMeshCfg->bSupportSmartMesh)
									pEntry->bEnableSmartMesh = TRUE;
								DBGPRINT(RT_DEBUG_ERROR, ("Peer supports SMART MESH\n"));
							}
							else
								pEntry->bEnableSmartMesh = FALSE;
							
							if (((pSmartMeshCfg->HiFiFlagMask != 0) && (pSmartMeshCfg->HiFiFlagValue != 0)) &&
								((eid_ptr->Octet[3] & pSmartMeshCfg->HiFiFlagMask) == pSmartMeshCfg->HiFiFlagValue))
							{
								pEntry->bHyperFiPeer = TRUE;
								DBGPRINT(RT_DEBUG_ERROR, ("Peer is Hyper-Fi device\n"));
							}
							else
								pEntry->bHyperFiPeer = FALSE;
#ifdef MWDS
							if ((eid_ptr->Octet[3] & 0x01))
							{
								pEntry->bSupportMWDS = TRUE;
								if(pAd->ApCfg.MBSSID[pEntry->apidx].bSupportMWDS)
									pEntry->bEnableMWDS = TRUE;
								DBGPRINT(RT_DEBUG_ERROR, ("Peer supports MWDS\n"));
							}
							else
								pEntry->bEnableMWDS = FALSE;
#endif /* MWDS */
#ifdef WSC_AP_SUPPORT
#ifdef SMART_MESH_HIDDEN_WPS
                            if ((eid_ptr->Octet[3] & 0x04))
							{
								pEntry->bSupportHiddenWPS = TRUE;
								DBGPRINT(RT_DEBUG_ERROR, ("Peer supports HiddenWPS\n"));
							}
							else
								pEntry->bSupportHiddenWPS = FALSE;

                            if((eid_ptr->Len - NTGR_OUI_LEN) >= NTGR_CUSTOM_IE_MAX_LEN)
                                pEntry->bRunningHiddenWPS = (eid_ptr->Octet[5] & HIDDEN_WPS_STATE_RUNNING) ? TRUE : FALSE;
#endif /* SMART_MESH_HIDDEN_WPS */
#endif /* WSC_AP_SUPPORT */
						}
					}
					else
						DBGPRINT(RT_DEBUG_ERROR, ("%s():pEntry is NULL\n",__func__));
					break;
				}				
#endif /* SMART_MESH */

				/* Handle Atheros and Broadcom draft 11n STAs */
				if (NdisEqualMemory(eid_ptr->Octet, BROADCOM_OUI, 3))
				{
					switch (eid_ptr->Octet[3])
					{
						case 0x33: 
							if ((eid_ptr->Len-4) == sizeof(HT_CAPABILITY_IE))
							{
								NdisMoveMemory(pHtCapability, &eid_ptr->Octet[4], SIZE_HT_CAP_IE);

								*(USHORT *)(&pHtCapability->HtCapInfo) = cpu2le16(*(USHORT *)(&pHtCapability->HtCapInfo));
#ifdef UNALIGNMENT_SUPPORT
								{
									EXT_HT_CAP_INFO extHtCapInfo;

									NdisMoveMemory((PUCHAR)(&extHtCapInfo), (PUCHAR)(&pHtCapability->ExtHtCapInfo), sizeof(EXT_HT_CAP_INFO));
									*(USHORT *)(&extHtCapInfo) = cpu2le16(*(USHORT *)(&extHtCapInfo));
									NdisMoveMemory((PUCHAR)(&pHtCapability->ExtHtCapInfo), (PUCHAR)(&extHtCapInfo), sizeof(EXT_HT_CAP_INFO));		
								}
#else				
								*(USHORT *)(&pHtCapability->ExtHtCapInfo) = cpu2le16(*(USHORT *)(&pHtCapability->ExtHtCapInfo));
#endif /* UNALIGNMENT_SUPPORT */

								ie_lists->ht_cap_len = SIZE_HT_CAP_IE;
							}
							break;
						
						default:
							/* ignore other cases */
							break;
					}
				}

                if (NdisEqualMemory(eid_ptr->Octet, RALINK_OUI, 3) && (eid_ptr->Len == 7))
                {
			if (eid_ptr->Octet[3] != 0)
		       	ie_lists->RalinkIe = eid_ptr->Octet[3];
        		else
        			ie_lists->RalinkIe = 0xf0000000; /* Set to non-zero value (can't set bit0-2) to represent this is Ralink Chip. So at linkup, we will set ralinkchip flag. */
                    break;
                }
                
                if (NdisEqualMemory(eid_ptr->Octet, MTK_OUI, 3) && (eid_ptr->Len == 7))
                {
			if (eid_ptr->Octet[3] != 0)
		       	ie_lists->MediatekIe= eid_ptr->Octet[3];
                    break;
                }
                
                /* WMM_IE */
                if (NdisEqualMemory(eid_ptr->Octet, WME_INFO_ELEM, 6) && (eid_ptr->Len == 7))
                {
                    ie_lists->bWmmCapable = TRUE;

#ifdef UAPSD_SUPPORT
			if (pEntry)
			{
				UAPSD_AssocParse(pAd,
							pEntry, (UINT8 *)&eid_ptr->Octet[6],
							pAd->ApCfg.MBSSID[pEntry->apidx].UapsdInfo.bAPSDCapable);
			}
#endif /* UAPSD_SUPPORT */

                    break;
                }

                if (pAd->ApCfg.MBSSID[pEntry->apidx].wdev.AuthMode < Ndis802_11AuthModeWPA)
                    break;
                
                /* 	If this IE did not begins with 00:0x50:0xf2:0x01,  
                	it would be proprietary. So we ignore it. */
                if (!NdisEqualMemory(eid_ptr->Octet, WPA1_OUI, sizeof(WPA1_OUI))
                    && !NdisEqualMemory(&eid_ptr->Octet[2], WPA2_OUI, sizeof(WPA2_OUI)))
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("Not RSN IE, maybe WMM IE!!!\n"));
#ifdef CONFIG_HOTSPOT_R2
					if (!NdisEqualMemory(eid_ptr->Octet, HS2_OSEN_OUI, sizeof(HS2_OSEN_OUI)))
					{
						unsigned char *tmp = (unsigned char *)eid_ptr->Octet;
		
						DBGPRINT(RT_DEBUG_OFF, ("!!!!!!not found OSEN IE,%x:%x:%x:%x\n", *tmp, *(tmp+1), *(tmp+2), *(tmp+3)));
						CLIENT_STATUS_CLEAR_FLAG(pEntry, fCLIENT_STATUS_OSEN_CAPABLE);
                    break;                          
                }
					else
					{
						CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_OSEN_CAPABLE);
						DBGPRINT(RT_DEBUG_OFF, ("!!!!!!found OSEN IE\n"));
					}
#else
					break;							
#endif          
                                             
                }
                
                if (/*(eid_ptr->Len <= MAX_LEN_OF_RSNIE) &&*/ (eid_ptr->Len >= MIN_LEN_OF_RSNIE))
                {
					hex_dump("Received RSNIE in Assoc-Req", (UCHAR *)eid_ptr, eid_ptr->Len + 2);
                    
					/* Copy whole RSNIE context */
                    NdisMoveMemory(&ie_lists->RSN_IE[0], eid_ptr, eid_ptr->Len + 2);
					ie_lists->RSNIE_Len =eid_ptr->Len + 2;

#ifdef DOT11R_FT_SUPPORT
					NdisMoveMemory(pFtInfo->RSN_IE, eid_ptr, eid_ptr->Len + 2);
					pFtInfo->RSNIE_Len = eid_ptr->Len + 2;
#endif /* DOT11R_FT_SUPPORT */					
                }
                else
                {
                    ie_lists->RSNIE_Len = 0;
                    DBGPRINT(RT_DEBUG_TRACE, ("PeerAssocReqSanity - missing IE_WPA(%d)\n",eid_ptr->Len));
                    return FALSE;
                }               
                break;

#ifdef WAPI_SUPPORT				
			case IE_WAPI:				
				if ((pAd->ApCfg.MBSSID[pEntry->apidx].wdev.AuthMode != Ndis802_11AuthModeWAICERT) &&
					(pAd->ApCfg.MBSSID[pEntry->apidx].wdev.AuthMode != Ndis802_11AuthModeWAIPSK))
                    break;				

				/* Sanity check the validity of WIE */
				/* Todo - AlbertY */
				
				/* Copy whole WAPI-IE context */
                NdisMoveMemory(&ie_lists->RSN_IE[0], eid_ptr, eid_ptr->Len + 2);
				ie_lists->RSNIE_Len =eid_ptr->Len + 2;		
				DBGPRINT(RT_DEBUG_TRACE, ("PeerAssocReqSanity - IE_WAPI(%d)\n",eid_ptr->Len));
				break;
#endif /* WAPI_SUPPORT */				

#ifdef DOT11R_FT_SUPPORT
			case IE_FT_MDIE:
				FT_FillMdIeInfo(eid_ptr, &pFtInfo->MdIeInfo);
				break;

			case IE_FT_FTIE:
				FT_FillFtIeInfo(eid_ptr, &pFtInfo->FtIeInfo);
				break;

			case IE_FT_RIC_DATA:
				/* record the pointer of first RDIE. */
				if (pFtInfo->RicInfo.pRicInfo == NULL)
				{
					pFtInfo->RicInfo.pRicInfo = &eid_ptr->Eid;
					pFtInfo->RicInfo.Len = ((UCHAR*)Fr + MsgLen) - (UCHAR*)eid_ptr + 1;
				}
			case IE_FT_RIC_DESCRIPTOR:
				if ((pFtInfo->RicInfo.RicIEsLen + eid_ptr->Len + 2) < MAX_RICIES_LEN)
				{
					NdisMoveMemory(&pFtInfo->RicInfo.RicIEs[pFtInfo->RicInfo.RicIEsLen],
									&eid_ptr->Eid, eid_ptr->Len + 2);
					pFtInfo->RicInfo.RicIEsLen += eid_ptr->Len + 2;
				}
				break;
#endif /* DOT11R_FT_SUPPORT */

#ifdef DOT11K_RRM_SUPPORT
			case IE_RRM_EN_CAP:
				{
					UINT64 value;
					NdisMoveMemory(&value, eid_ptr->Octet, sizeof(UINT64));
					pRrmEnCap->word = le2cpu64(value);
				}
				break;
#endif /* DOT11K_RRM_SUPPORT */

#ifdef DOT11_VHT_AC
		case IE_VHT_CAP:
			if (eid_ptr->Len == sizeof(VHT_CAP_IE))
			{
				NdisMoveMemory(&ie_lists->vht_cap, eid_ptr->Octet, sizeof(VHT_CAP_IE));
				ie_lists->vht_cap_len = eid_ptr->Len;
				DBGPRINT(RT_DEBUG_TRACE, ("%s():IE_VHT_CAP\n", __FUNCTION__));
			}
			else
			{
				DBGPRINT(RT_DEBUG_WARN, ("%s():wrong IE_VHT_CAP, eid->Len = %d\n",
							__FUNCTION__, eid_ptr->Len));
			}

		case IE_VHT_OP:
			if (eid_ptr->Len == sizeof(VHT_OP_IE))
			{
				NdisMoveMemory(&ie_lists->vht_op, eid_ptr->Octet, sizeof(VHT_OP_IE));
				ie_lists->vht_op_len = eid_ptr->Len;
				DBGPRINT(RT_DEBUG_TRACE, ("%s():IE_VHT_OP\n", __FUNCTION__));
			}

		case IE_OPERATING_MODE_NOTIFY:
			if (eid_ptr->Len == sizeof(OPERATING_MODE)) {
				ie_lists->operating_mode_len = sizeof(OPERATING_MODE);
				NdisMoveMemory(&ie_lists->operating_mode, &eid_ptr->Octet[0], sizeof(OPERATING_MODE));
				DBGPRINT(RT_DEBUG_TRACE, ("%s():IE_OPERATING_MODE_NOTIFY!\n", __FUNCTION__));
			}
			break;
#endif /* DOT11_VHT_AC */

            default:
                break;
        }

        eid_ptr = (PEID_STRUCT)((UCHAR*)eid_ptr + 2 + eid_ptr->Len);        
    }

	if ((Sanity&0x3) != 0x03)	 
	{
		DBGPRINT(RT_DEBUG_WARN, ("%s(): - missing mandatory field\n", __FUNCTION__));
		return FALSE;
	}
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s() - success\n", __FUNCTION__));
		return TRUE;
	}
}




/* 
    ==========================================================================
    Description:
        MLME message sanity check
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
 */
BOOLEAN PeerDisassocReqSanity(
    IN PRTMP_ADAPTER pAd, 
    IN VOID *Msg, 
    IN ULONG MsgLen, 
    OUT PUCHAR pAddr1, 
    OUT PUCHAR pAddr2, 
    OUT	UINT16	*SeqNum,
    OUT USHORT *Reason) 
{
    PFRAME_802_11 Fr = (PFRAME_802_11)Msg;

    COPY_MAC_ADDR(pAddr1, &Fr->Hdr.Addr1);
    COPY_MAC_ADDR(pAddr2, &Fr->Hdr.Addr2);
	*SeqNum = Fr->Hdr.Sequence;
    NdisMoveMemory(Reason, &Fr->Octet[0], 2);

    return TRUE;
}


/* 
    ==========================================================================
    Description:
        MLME message sanity check
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
 */
BOOLEAN PeerDeauthReqSanity(
    IN PRTMP_ADAPTER pAd, 
    IN VOID *Msg, 
    IN ULONG MsgLen, 
    OUT PUCHAR pAddr2, 
    OUT	UINT16	*SeqNum, 
    OUT USHORT *Reason) 
{
    PFRAME_802_11 Fr = (PFRAME_802_11)Msg;

    COPY_MAC_ADDR(pAddr2, &Fr->Hdr.Addr2);
	*SeqNum = Fr->Hdr.Sequence;
    NdisMoveMemory(Reason, &Fr->Octet[0], 2);

    return TRUE;
}


/* 
    ==========================================================================
    Description:
        MLME message sanity check
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
 */
BOOLEAN APPeerAuthSanity(
    IN PRTMP_ADAPTER pAd, 
    IN VOID *Msg, 
    IN ULONG MsgLen, 
    OUT PUCHAR pAddr1, 
    OUT PUCHAR pAddr2, 
    OUT USHORT *Alg, 
    OUT USHORT *Seq, 
    OUT USHORT *Status, 
    CHAR *ChlgText
#ifdef DOT11R_FT_SUPPORT
	,PFT_INFO pFtInfo
#endif /* DOT11R_FT_SUPPORT */
    ) 
{
    PFRAME_802_11 Fr = (PFRAME_802_11)Msg;

	COPY_MAC_ADDR(pAddr1,  &Fr->Hdr.Addr1);		/* BSSID */
    COPY_MAC_ADDR(pAddr2,  &Fr->Hdr.Addr2);		/* SA */
    NdisMoveMemory(Alg,    &Fr->Octet[0], 2);
    NdisMoveMemory(Seq,    &Fr->Octet[2], 2);
    NdisMoveMemory(Status, &Fr->Octet[4], 2);

    if (*Alg == AUTH_MODE_OPEN) 
    {
        if (*Seq == 1 || *Seq == 2) 
        {
            return TRUE;
        } 
        else 
        {
            DBGPRINT(RT_DEBUG_TRACE, ("APPeerAuthSanity fail - wrong Seg# (=%d)\n", *Seq));
            return FALSE;
        }
    } 
    else if (*Alg == AUTH_MODE_KEY) 
    {
        if (*Seq == 1 || *Seq == 4) 
        {
            return TRUE;
        } 
        else if (*Seq == 2 || *Seq == 3) 
        {
            NdisMoveMemory(ChlgText, &Fr->Octet[8], CIPHER_TEXT_LEN);
            return TRUE;
        } 
        else 
        {
            DBGPRINT(RT_DEBUG_TRACE, ("APPeerAuthSanity fail - wrong Seg# (=%d)\n", *Seq));
            return FALSE;
        }
    } 
#ifdef DOT11R_FT_SUPPORT
	else if (*Alg == AUTH_MODE_FT)
	{
		PEID_STRUCT eid_ptr;
		UCHAR *Ptr;
		UCHAR WPA2_OUI[3]={0x00,0x0F,0xAC};

		NdisZeroMemory(pFtInfo, sizeof(FT_INFO));

		Ptr = &Fr->Octet[6];
		eid_ptr = (PEID_STRUCT) Ptr;

	    /* get variable fields from payload and advance the pointer */
		while(((UCHAR*)eid_ptr + eid_ptr->Len + 1) < ((UCHAR*)Fr + MsgLen))
		{
			switch(eid_ptr->Eid)
			{
				case IE_FT_MDIE:
					FT_FillMdIeInfo(eid_ptr, &pFtInfo->MdIeInfo);
					break;

				case IE_FT_FTIE:
					FT_FillFtIeInfo(eid_ptr, &pFtInfo->FtIeInfo);
					break;

				case IE_FT_RIC_DATA:
					/* record the pointer of first RDIE. */
					if (pFtInfo->RicInfo.pRicInfo == NULL)
					{
						pFtInfo->RicInfo.pRicInfo = &eid_ptr->Eid;
						pFtInfo->RicInfo.Len = ((UCHAR*)Fr + MsgLen)
												- (UCHAR*)eid_ptr + 1;
					}

					if ((pFtInfo->RicInfo.RicIEsLen + eid_ptr->Len + 2) < MAX_RICIES_LEN)
					{
						NdisMoveMemory(&pFtInfo->RicInfo.RicIEs[pFtInfo->RicInfo.RicIEsLen],
										&eid_ptr->Eid, eid_ptr->Len + 2);
						pFtInfo->RicInfo.RicIEsLen += eid_ptr->Len + 2;
					}
					break;

					
				case IE_FT_RIC_DESCRIPTOR:
					if ((pFtInfo->RicInfo.RicIEsLen + eid_ptr->Len + 2) < MAX_RICIES_LEN)
					{
						NdisMoveMemory(&pFtInfo->RicInfo.RicIEs[pFtInfo->RicInfo.RicIEsLen],
										&eid_ptr->Eid, eid_ptr->Len + 2);
						pFtInfo->RicInfo.RicIEsLen += eid_ptr->Len + 2;
					}
					break;

				case IE_RSN:
					if (NdisEqualMemory(&eid_ptr->Octet[2], WPA2_OUI, sizeof(WPA2_OUI)))
					{
	                    NdisMoveMemory(pFtInfo->RSN_IE, eid_ptr, eid_ptr->Len + 2);
						pFtInfo->RSNIE_Len = eid_ptr->Len + 2;
					}
					break;

				default:
					break;
			}
	        eid_ptr = (PEID_STRUCT)((UCHAR*)eid_ptr + 2 + eid_ptr->Len);        
		}
	}
#endif /* DOT11R_FT_SUPPORT */
    else 
    {
        DBGPRINT(RT_DEBUG_TRACE, ("APPeerAuthSanity fail - wrong algorithm (=%d)\n", *Alg));
        return FALSE;
    }

	return TRUE;
}

