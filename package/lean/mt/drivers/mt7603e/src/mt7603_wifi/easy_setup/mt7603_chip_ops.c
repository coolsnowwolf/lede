/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of Ralink Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	cmm_ez.c

	Abstract:
	Easy Setup APIs

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
*/

#ifdef WH_EZ_SETUP
#include "rt_config.h"

//ez_chipops_t ez_chipops;
UCHAR	IPV4TYPE[] = {0x08, 0x00};

//int EzDebugLevel = DBG_LVL_ERROR;

//----------------------------------------------------------------------------------------------------------
//											Arvind code
//-----------------------------------------------------------------------------------------------------------

void ez_hex_dump(char *str, unsigned char *pSrcBufVA, unsigned int SrcBufLen)
{
#ifdef DBG
		unsigned char *pt;
		int x;
		if (EzDebugLevel < DBG_LVL_TRACE)
			return;
		pt = pSrcBufVA;
		EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,("%s: %p, len = %d\n", str, pSrcBufVA, SrcBufLen));
		for (x = 0; x < SrcBufLen; x++) {
			if (x % 16 == 0)
				EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,("0x%04x : ", x));
			EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,("%02x ", ((unsigned char)pt[x])));
			if (x % 16 == 15)
				EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,("\n"));
		}
		EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,("\n"));
#endif /* DBG */
}

VOID UpdateBeaconHandler(		
	void *ad_obj,        
	struct wifi_dev *wdev,        
	UCHAR BCN_UPDATE_REASON)
{	
	RTMP_ADAPTER *pAd;	
	pAd = (RTMP_ADAPTER *)ad_obj;

#ifdef WH_EZ_SETUP
#ifdef DUAL_CHIP	
	if(!IS_SINGLE_CHIP_DBDC(pAd)) 
	{			
		if ((wdev !=NULL) && wdev->enable_easy_setup)				
			RTMP_SEM_LOCK(&pAd->ez_beacon_update_lock);		
	}
#endif
#endif	

	APMakeAllBssBeacon(pAd);	
	APUpdateAllBeaconFrame(pAd);

#ifdef WH_EZ_SETUP
#ifdef DUAL_CHIP		
	if(!IS_SINGLE_CHIP_DBDC(pAd)) 
	{			
		if ((wdev !=NULL) && wdev->enable_easy_setup)				
			RTMP_SEM_UNLOCK(&pAd->ez_beacon_update_lock);		
	}	
#endif
#endif	
}

void ez_install_pairwise_key_mt7603(
	RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	char *peer_mac,
	unsigned char *pmk,
	unsigned char *ptk,
	unsigned char authenticator)
{

	//struct _ez_peer_security_info *ez_peer = (struct _ez_peer_security_info *)peer;
	//ez_dev_t *ez_dev = (ez_dev_t *)ezdev;	
	//RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)ez_dev->ad;
	//struct wifi_dev *wdev = (struct wifi_dev *)ez_dev->wdev;	
	MAC_TABLE_ENTRY *entry;
	STA_TR_ENTRY *tr_entry;
	
	entry = MacTableLookup(pAd, peer_mac);
	tr_entry = &pAd->MacTab.tr_entry[entry->tr_tb_idx];

	SET_AUTHMODE_WPA2PSK(entry->AuthMode);
	SET_CIPHER_CCMP128(entry->WepStatus);
	SET_CIPHER_CCMP128(entry->GroupKeyWepStatus);
	//entry->SecConfig.PairwiseKeyId = 1; //Arvind don't need always use 0
	
	if (wdev && (wdev->wdev_type == WDEV_TYPE_AP)) {
		BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[wdev->func_idx];
		NdisZeroMemory(&pMbss->PMK[0], LEN_PMK);
		NdisCopyMemory(&pMbss->PMK[0], pmk, LEN_PMK);
			
	} else if (entry->wdev && entry->wdev->wdev_type == WDEV_TYPE_STA){
		APCLI_STRUCT *apcli_entry = &pAd->ApCfg.ApCliTab[wdev->func_idx];
		NdisZeroMemory(&apcli_entry->PMK[0], LEN_PMK);
		NdisCopyMemory(&apcli_entry->PMK[0], pmk, LEN_PMK);	
	}
	NdisZeroMemory(&entry->PTK[0], LEN_PTK);
	NdisCopyMemory(&entry->PTK[0], ptk, LEN_PTK);
	/* Set key material to Asic */
	WPAInstallPairwiseKey(pAd, entry->func_tb_idx, entry, authenticator);
	
	/* Update status and set Port as Secured */
	entry->WpaState = AS_PTKINITDONE;
	entry->GTKState = REKEY_ESTABLISHED;
	entry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
	tr_entry->PortSecured = WPA_802_1X_PORT_SECURED;

	//WifiSysUpdatePortSecur(pAd, entry);
}


unsigned char ez_RandomByte_mt7603(RTMP_ADAPTER *pAd)
{
	return RandomByte(pAd);
}

void ez_DH_PublicKey_Generate_mt7603(
	UINT8 GValue[], 
	UINT GValueLength,
	UINT8 PValue[],
	UINT PValueLength,
	UINT8 PrivateKey[],
	UINT PrivateKeyLength,
	UINT8 PublicKey[],
	UINT *PublicKeyLength)
{

	DH_PublicKey_Generate(GValue, GValueLength, PValue, PValueLength,
							PrivateKey, PrivateKeyLength, PublicKey, PublicKeyLength);
}

void ez_RT_DH_SecretKey_Generate_mt7603(
	UINT8 PublicKey[], 
	UINT PublicKeyLength, 
	UINT8 PValue[], 
	UINT PValueLength, 
	UINT8 PrivateKey[],
	UINT PrivateKeyLength, 
	UINT8 SecretKey[], 
	UINT *SecretKeyLength)
{

	RT_DH_SecretKey_Generate(PublicKey, PublicKeyLength, PValue, PValueLength, 
				PrivateKey, PrivateKeyLength, SecretKey, SecretKeyLength);
}

void ez_RT_SHA256_mt7603(
	IN  const UINT8 Message[], 
    IN  UINT MessageLen, 
    OUT UINT8 DigestMessage[])
{
	RT_SHA256(Message, MessageLen, DigestMessage);
}


VOID ez_WpaDerivePTK_mt7603(
	RTMP_ADAPTER *pAd, 
	UCHAR *PMK, 
	UCHAR *ANonce, 
	UCHAR *AA, 
	UCHAR *SNonce, 
	UCHAR *SA, 
	UCHAR *output, 
	UINT len)
{	
	WpaDerivePTK(pAd, PMK, ANonce, AA, SNonce, SA, output, len);
}

INT ez_AES_Key_Unwrap_mt7603(
	UINT8 CipherText[],
	UINT CipherTextLength, 
	UINT8 Key[],
	UINT KeyLength,
	UINT8 PlainText[],
	UINT *PlainTextLength)
{

	return AES_Key_Unwrap(CipherText, CipherTextLength, Key, KeyLength, PlainText,PlainTextLength);

}


#ifdef APCLI_SUPPORT
void ez_apcli_install_group_key_mt7603(
	RTMP_ADAPTER *ad,
	char *peer_gtk,
	unsigned char gtk_len,	
	MAC_TABLE_ENTRY *entry)
{
	/* Set Group key material, TxMic and RxMic for AP-Client*/
	APCliInstallSharedKey(
		ad, 
		peer_gtk, 
		gtk_len, 
		1, 
		entry);
}
#endif /* APCLI_SUPPORT */

INT ez_wlan_config_get_ht_bw_mt7603(RTMP_ADAPTER *pAd)
{
	return pAd->CommonCfg.RegTransmitSetting.field.BW;
}

#ifdef DOT11_VHT_AC
INT ez_wlan_config_get_vht_bw_mt7603(RTMP_ADAPTER *pAd)
{
	return pAd->CommonCfg.vht_bw;
}

#endif

INT ez_wlan_operate_get_ht_bw_mt7603(RTMP_ADAPTER *pAd)
{
	return pAd->CommonCfg.AddHTInfo.AddHtInfo.RecomWidth;
}

INT ez_wlan_operate_get_ext_cha_mt7603(RTMP_ADAPTER *pAd)
{
	return pAd->CommonCfg.AddHTInfo.AddHtInfo.ExtChanOffset;
}

INT ez_wlan_config_get_ext_cha_mt7603(RTMP_ADAPTER *pAd)
{
	return pAd->CommonCfg.RegTransmitSetting.field.EXTCHA;
}

int ez_get_cli_aid_mt7603(RTMP_ADAPTER *pAd, char *peer_mac)
{
	MAC_TABLE_ENTRY *pEntry;
	pEntry = MacTableLookup(pAd, peer_mac);

	if(pEntry)
		return pEntry->Aid;
	else {		
		printk("%s Entry not found\n", __FUNCTION__);
		ASSERT(FALSE);
		return -1;
	}

}

void ez_cancel_timer_mt7603(
	RTMP_ADAPTER *pAd, 
	void * timer_struct)
{
	ez_timer_t *timer = (ez_timer_t *)timer_struct;
	
	EZ_CANCEL_TIMER(&timer->ez_timer, timer->ez_timer_running);
	return ;
}

void ez_set_timer_mt7603(
	RTMP_ADAPTER *pAd, 
	void * timer_struct, 
	unsigned long time)
{
	ez_timer_t *timer = (ez_timer_t *)timer_struct;
	
	RTMPSetTimer(&timer->ez_timer, time);
	return ;
}

int ez_is_timer_running_mt7603(
	RTMP_ADAPTER *pAd, 
	void * timer_struct)
{

	//ez_is_timer_running(void * ezdev, void * timer_struct)
	return 0;
}

#ifdef APCLI_SUPPORT
int get_apcli_enable_mt7603(
	RTMP_ADAPTER *pAd, 
	int ifIndex)
{
	if(pAd->ApCfg.ApCliTab[ifIndex].Enable)
		return 1;
	else
		return 0;
}
#endif

int ez_ApScanRunning_mt7603(RTMP_ADAPTER *pAd)
{
	if(ApScanRunning(pAd))
		return 1;
	else
		return 0;

}

BOOLEAN EzMlmeEnqueue(
	IN RTMP_ADAPTER *pAd,
	IN ULONG Machine,
	IN ULONG MsgType,
	IN ULONG MsgLen,
	IN VOID *Msg,
	IN ULONG Priv)
{
	INT Tail;
	MLME_QUEUE	*Queue = (MLME_QUEUE *)&pAd->Mlme.Queue;

	/* Do nothing if the driver is starting halt state.*/
	/* This might happen when timer already been fired before cancel timer with mlmehalt*/
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
		return FALSE;

	/* First check the size, it MUST not exceed the mlme queue size*/
	if (MsgLen > MGMT_DMA_BUFFER_SIZE)
	{
		EZ_DEBUG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("MlmeEnqueue: msg too large, size = %ld \n", MsgLen));
		return FALSE;
	}

	if (MlmeQueueFull(Queue, 1))
	{
		return FALSE;
	}

	NdisAcquireSpinLock(&(Queue->Lock));
	Tail = Queue->Tail;
	/*
		Double check for safety in multi-thread system.
	*/
	if (Queue->Entry[Tail].Occupied)
	{
		NdisReleaseSpinLock(&(Queue->Lock));
		return FALSE;
	}
	Queue->Tail++;
	Queue->Num++;
	if (Queue->Tail == MAX_LEN_OF_MLME_QUEUE)
		Queue->Tail = 0;

	Queue->Entry[Tail].Occupied = TRUE;
	Queue->Entry[Tail].Machine = Machine;
	Queue->Entry[Tail].MsgType = MsgType;
	Queue->Entry[Tail].MsgLen = MsgLen;
	Queue->Entry[Tail].Priv = Priv;

	if (Msg != NULL)
	{
		Queue->Entry[Tail].Wcid = ((MLME_QUEUE_ELEM *)Msg)->Wcid;
		NdisMoveMemory(Queue->Entry[Tail].Msg, Msg, MsgLen);
	}

	NdisReleaseSpinLock(&(Queue->Lock));

	return TRUE;
}

void ez_send_unicast_deauth_ap(void *ad_obj, UCHAR *peer_addr)
{
	MLME_DEAUTH_REQ_STRUCT  *pInfo = NULL;
	MLME_QUEUE_ELEM *Elem;
	MAC_TABLE_ENTRY *pEntry = NULL;
	RTMP_ADAPTER *pAd = ad_obj;
	//struct _ez_peer_security_info *ez_peer;
	BOOLEAN ez_peer = FALSE;
	os_alloc_mem(pAd, (UCHAR **)&Elem, sizeof(MLME_QUEUE_ELEM));
	if(Elem == NULL)
	{
		EZ_DEBUG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Set::OID_802_11_DEAUTHENTICATION, Failed!!\n"));
		return;
	}
	if (Elem)
	{
		pInfo = (MLME_DEAUTH_REQ_STRUCT *) Elem->Msg;
		pInfo->Reason = REASON_NO_LONGER_VALID;
		NdisCopyMemory(pInfo->Addr, peer_addr, MAC_ADDR_LEN);
		if ((pEntry = MacTableLookup(pAd, peer_addr)) != NULL)
		{
			Elem->Wcid = pEntry->wcid;
			EZ_DEBUG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("WCID = %d\n;", Elem->Wcid));
			ez_hex_dump("address :", pEntry->Addr, 6);
			ez_peer = is_ez_peer(pEntry->wdev,pEntry->Addr);

			if (ez_peer && !ez_get_peer_delete_in_differred_context(pEntry->wdev,pEntry->Addr)){
				APMlmeDeauthReqAction(pAd, Elem);
			} else {
			
				EzMlmeEnqueue(pAd, AP_AUTH_STATE_MACHINE, APMT2_MLME_DEAUTH_REQ,
					sizeof(MLME_DEAUTH_REQ_STRUCT), Elem, 0);
			}
			EZ_DEBUG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("EZ Set::OID_802_11_DEAUTHENTICATION (Reason=%d)\n", pInfo->Reason));
		}
		os_free_mem(pAd, Elem);
	}
}

void ez_send_unicast_deauth_apcli(void *ad_obj, USHORT idx)
{
	MLME_QUEUE_ELEM *Elem;
	//struct _ez_peer_security_info *ez_peer = NULL;
	BOOLEAN ez_peer = FALSE;
	struct wifi_dev *wdev;
	PRTMP_ADAPTER ad = ad_obj;

	wdev = &ad->ApCfg.ApCliTab[idx].wdev;

	os_alloc_mem(ad_obj, (UCHAR **)&Elem, sizeof(MLME_QUEUE_ELEM));
	if(Elem == NULL)
	{
		EZ_DEBUG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Set::OID_802_11_DEAUTHENTICATION, Failed!!\n"));
		return;
	}

	ez_peer = is_ez_peer(wdev, wdev->bssid);

	if (ez_peer && !ez_get_peer_delete_in_differred_context(wdev, wdev->bssid)){
		Elem->Priv = idx | IMM_DISCONNECT;
		ApCliCtrlDeAuthAction(ad_obj, Elem);
	} else {
		MlmeEnqueue(ad, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_DISCONNECT_REQ, 0, NULL, idx);
	}

	os_free_mem(ad, Elem);
}

void ez_send_unicast_deauth_mt7603(void *ad_obj, UCHAR *peer_addr)
{
	MAC_TABLE_ENTRY *pEntry = NULL;

	pEntry = MacTableLookup(ad_obj,peer_addr);
	
	RtmpOsMsDelay(50);

	if (pEntry && pEntry->wdev->wdev_type == WDEV_TYPE_AP)
	{
		ez_send_unicast_deauth_ap(ad_obj, peer_addr);
	} else if (pEntry && pEntry->wdev->wdev_type == WDEV_TYPE_STA) {
		ez_send_unicast_deauth_apcli(ad_obj, pEntry->wdev->func_idx);
	}
}

void ez_UpdateBeaconHandler_mt7603(
	RTMP_ADAPTER *ez_ad, 
	struct wifi_dev *wdev, 
	UCHAR reason)
{
	UpdateBeaconHandler(ez_ad, wdev, reason);
	return;
}

void ez_update_security_setting_mt7603(
	RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	unsigned char *pmk)
{
	int i;
	unsigned char *ptr;

	EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
			("------> %s()\n", __FUNCTION__));

	if(wdev->wdev_type == WDEV_TYPE_AP)
	{
		BSS_STRUCT *pMbss;
		pMbss = &pAd->ApCfg.MBSSID[wdev->func_idx];
		
		NdisCopyMemory(&pMbss->PMK[0], pmk, LEN_PMK);
		hex_dump("pMbss_pmk", &pMbss->PMK[0], LEN_PMK);
		ptr = &pMbss->PSK[0];
		NdisZeroMemory(ptr, sizeof(pMbss->PSK));
		EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, 
				("%s() pMbss->PSK: %s (%d)\n", 
				__FUNCTION__, pMbss->PSK, (int)strlen(pMbss->PSK)));	
		
	}
	else if(wdev->wdev_type == WDEV_TYPE_STA)
	{
		APCLI_STRUCT *pApCliEntry;
		pApCliEntry = &pAd->ApCfg.ApCliTab[wdev->func_idx];

		NdisCopyMemory(&pApCliEntry->PMK[0], pmk, LEN_PMK);
		hex_dump("pApCliEntry_pmk", &pApCliEntry->PMK[0], LEN_PMK);
		ptr = &pApCliEntry->PSK[0];
		NdisZeroMemory(ptr, sizeof(pApCliEntry->PSK));
		EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, 
				("%s() pApCliEntry.PSK: %s (%d)\n", 
				__FUNCTION__, pApCliEntry->PSK, (int)strlen(pApCliEntry->PSK)));	
			
	}

	for (i = 0; i < LEN_PMK; i++) {
		snprintf(ptr, (LEN_PSK+1), "%s%02x", ptr, pmk[i]);
	}
	
	EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
			("<------ %s()\n", __FUNCTION__));
}

#ifdef WSC_AP_SUPPORT
void ez_updage_ap_wsc_profile_mt7603(
	RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	unsigned char if_idx)
{
	WSC_CTRL *wsc_ctrl;
	BSS_STRUCT *pMbss;

	pMbss = &pAd->ApCfg.MBSSID[if_idx];
	wsc_ctrl = &pAd->ApCfg.MBSSID[if_idx].WscControl;
	NdisZeroMemory(&wsc_ctrl->WpaPsk[0], LEN_PSK);
	wsc_ctrl->WpaPskLen = strlen(pMbss->PSK);
	NdisCopyMemory(&wsc_ctrl->WpaPsk[0], pMbss->PSK, wsc_ctrl->WpaPskLen);
#ifdef APCLI_SUPPORT
	if ((wdev->wdev_type == WDEV_TYPE_STA)
		&& (pAd->ApCfg.MBSSID[if_idx].wdev.enable_easy_setup)) {
		APCLI_STRUCT *pApCliEntry;
		pApCliEntry = &pAd->ApCfg.ApCliTab[if_idx];

		NdisZeroMemory(pMbss->PSK, sizeof(pMbss->PSK));
		NdisCopyMemory(pMbss->PSK, pApCliEntry->PSK, sizeof(pMbss->PSK));

	}
#endif /* APCLI_SUPPORT */
	wsc_ctrl->WscConfStatus = WSC_SCSTATE_CONFIGURED;
}
#endif /* WSC_AP_SUPPORT */

void ez_MiniportMMRequest_mt7603(
	RTMP_ADAPTER *pAd, 
	UCHAR QueIdx, 
	UCHAR *out_buf,
	UINT frame_len)
{
	MiniportMMRequest(pAd, QueIdx, out_buf, frame_len);
	return;
}

void ez_NdisGetSystemUpTime_mt7603(ULONG *time)
{
	NdisGetSystemUpTime(time);
	return;
}

INT ez_AES_Key_Wrap_mt7603(
	UINT8 PlainText[],
	UINT  PlainTextLength,
	UINT8 Key[],
	UINT  KeyLength,
	UINT8 CipherText[],
	UINT *CipherTextLength)
{
	return AES_Key_Wrap(PlainText, PlainTextLength, Key, KeyLength, CipherText, CipherTextLength);

}

INT ez_RtmpOSWrielessEventSendExt_mt7603(
	PNET_DEV pNetDev,
	UINT32 eventType,
	INT flags,
	PUCHAR pSrcMac,
	PUCHAR pData,
	UINT32 dataLen)
{
	return RtmpOSWrielessEventSendExt(pNetDev, eventType, flags, pSrcMac, pData, dataLen, 0);

}

#ifdef EZ_NETWORK_MERGE_SUPPORT
VOID ez_APMlmeBroadcastDeauthReqAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	MLME_BROADCAST_DEAUTH_REQ_STRUCT	*pInfo;
    HEADER_802_11			Hdr;
    PUCHAR					pOutBuffer = NULL;
    NDIS_STATUS				NStatus;
    ULONG					FrameLen = 0;
    MAC_TABLE_ENTRY			*pEntry;
	UCHAR					apidx = 0;
    struct wifi_dev         *wdev;
	int wcid, startWcid;
	startWcid = 1;

	EZ_DEBUG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s\n", __FUNCTION__));

	pInfo = (PMLME_BROADCAST_DEAUTH_REQ_STRUCT)Elem->Msg;

	if (!MAC_ADDR_EQUAL(pInfo->Addr,BROADCAST_ADDR))
	{
		return;
	}

	wdev = pInfo->wdev;
	apidx = wdev->func_idx;

	for (wcid = startWcid; wcid < MAX_LEN_OF_MAC_TABLE; wcid++)
	{
		pEntry = &pAd->MacTab.Content[wcid];
		if ((pEntry->wdev != wdev))
		{
			continue;
		}
		if (wdev->enable_easy_setup && is_ez_peer(pEntry->wdev, pEntry->Addr))
		{
			continue;
		}

		RTMPSendWirelessEvent(pAd, IW_DEAUTH_EVENT_FLAG, pEntry->Addr, 0, 0);
		ApLogEvent(pAd, pInfo->Addr, EVENT_DISASSOCIATED);
		ez_hex_dump("DeleteEntryAddr",pEntry->Addr,6);
		
		MacTableDeleteEntry(pAd, wcid, pEntry->Addr);

#ifdef WH_EVENT_NOTIFIER
		{
			EventHdlr pEventHdlrHook = NULL;
			pEventHdlrHook = GetEventNotiferHook(WHC_DRVEVNT_STA_LEAVE);
			if(pEventHdlrHook && wdev)
				pEventHdlrHook(pAd, wdev, pEntry->Addr, Elem->Channel);
		}
#endif /* WH_EVENT_NOTIFIER */
	}

	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
	if (NStatus != NDIS_STATUS_SUCCESS)
		return;

	EZ_DEBUG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("AUTH - Send DE-AUTH req to %02x:%02x:%02x:%02x:%02x:%02x\n",
			PRINT_MAC(pInfo->Addr)));
	MgtMacHeaderInit(pAd, &Hdr, SUBTYPE_DEAUTH, 0, pInfo->Addr,
					pAd->ApCfg.MBSSID[apidx].wdev.if_addr,
					pAd->ApCfg.MBSSID[apidx].wdev.bssid);
	MakeOutgoingFrame(pOutBuffer,				&FrameLen,
					  sizeof(HEADER_802_11),	&Hdr,
					  2,						&pInfo->Reason,
					  END_OF_ARGS);

	MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
	MlmeFreeMemory(NULL, pOutBuffer);
}
#endif

void ez_send_broadcast_deauth_mt7603(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	MLME_BROADCAST_DEAUTH_REQ_STRUCT  *pInfo = NULL;
	MLME_QUEUE_ELEM *Elem;

	os_alloc_mem(pAd, (UCHAR **)&Elem, sizeof(MLME_QUEUE_ELEM));

	if(Elem == NULL)
	{
		EZ_DEBUG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Set::OID_802_11_DEAUTHENTICATION, Failed!!\n"));
		return;
	}
	
	if (Elem)
	{
		pInfo = (MLME_BROADCAST_DEAUTH_REQ_STRUCT *) Elem->Msg;
		pInfo->wdev = wdev;
		Elem->Wcid = WCID_ALL;
		pInfo->Reason = MLME_EZ_DISCONNECT_NON_EZ;
		NdisCopyMemory(pInfo->Addr, BROADCAST_ADDR, MAC_ADDR_LEN);
		APMlmeDeauthReqAction(pAd, Elem);
		os_free_mem(pAd, Elem);
	}
}

void ez_timer_init_mt7603(RTMP_ADAPTER *pAd, void* timer, void *callback)
{
	//EZ_TIMER_INIT(pAd, void* timer, void *callback);

}
void ez_set_ap_ssid_null_mt7603(
	RTMP_ADAPTER *pAd, 
	INT apidx)
{
	BSS_STRUCT *mbss;
	
	mbss = &pAd->ApCfg.MBSSID[apidx];
	NdisZeroMemory(mbss->Ssid, MAX_LEN_OF_SSID);
	mbss->SsidLen = 0;
	return ;
}
void ez_set_entry_apcli_mt7603(
	RTMP_ADAPTER *pAd, 
	UCHAR *mac_addr, 
	BOOLEAN is_apcli)
{
	MAC_TABLE_ENTRY *pEntry;
	pEntry = MacTableLookup(pAd, mac_addr);

	if(pEntry && is_apcli)
		SET_ENTRY_APCLI(pEntry);
	else
		printk("%s Entry not found\n", __FUNCTION__);
	
	return;	
}
void* ez_get_pentry_mt7603(
	RTMP_ADAPTER *pAd, 
	UCHAR *mac_addr)
{
	MAC_TABLE_ENTRY *pEntry;
	pEntry = MacTableLookup(pAd, mac_addr);

	return pEntry;	
}
void ez_mark_entry_duplicate_mt7603(RTMP_ADAPTER *pAd, UCHAR *mac_addr)
{
	MAC_TABLE_ENTRY *pEntry;
	pEntry = MacTableLookup(pAd, mac_addr);

#ifdef WH_EZ_SETUP
#ifdef EZ_DUAL_BAND_SUPPORT

	if(pEntry)
		pEntry->link_duplicate = TRUE;
	else
		printk("%s Entry not found !! \n", __FUNCTION__);

#endif
#endif /* WH_EZ_SETUP */
	return ;
}

void ez_ScanTableInit_mt7603(RTMP_ADAPTER *pAd)
{
	BssTableInit(&pAd->ScanTab);
	return ;
}

//-------------------------------------------------
//		raghav
//--------------------------------------------------

void ez_wlan_operate_set_ht_bw_mt7603(RTMP_ADAPTER *pAd,UINT8 ht_bw)
{
	pAd->CommonCfg.AddHTInfo.AddHtInfo.RecomWidth = ht_bw;
}

void ez_wlan_operate_set_ext_cha_mt7603(RTMP_ADAPTER *pAd,UINT8 ext_cha)
{
	pAd->CommonCfg.AddHTInfo.AddHtInfo.ExtChanOffset = ext_cha;
}

INT	rtmp_set_channel_mt7603(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR Channel)
{
#ifdef CONFIG_AP_SUPPORT
	INT32 i;
#endif /* CONFIG_AP_SUPPORT */
	INT32 Success = TRUE;
	UCHAR RFChannel;

	pAd->CommonCfg.Channel = Channel;

#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
#ifdef APCLI_AUTO_CONNECT_SUPPORT
			if (pAd->ApCfg.ApCliAutoConnectChannelSwitching == FALSE)
				pAd->ApCfg.ApCliAutoConnectChannelSwitching = TRUE;
#endif /* APCLI_AUTO_CONNECT_SUPPORT */
		}
#endif /* CONFIG_AP_SUPPORT */

	/* check if this channel is valid*/
	if (ChannelSanity(pAd, Channel) == TRUE)
	{
		Success = TRUE;
	}
	else
	{
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			Channel = FirstChannel(pAd);
			DBGPRINT(RT_DEBUG_WARN,("This channel is out of channel list, set as the first channel(%d) \n ", Channel));
		}
#endif /* CONFIG_AP_SUPPORT */

	}

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		if ((WMODE_CAP_5G(pAd->CommonCfg.PhyMode))
			&& (pAd->CommonCfg.bIEEE80211H == TRUE))
		{
			for (i = 0; i < pAd->ChannelListNum; i++)
			{
				if (pAd->ChannelList[i].Channel == Channel)
				{
					if (pAd->ChannelList[i].RemainingTimeForUse > 0)
					{
						DBGPRINT(RT_DEBUG_ERROR, ("ERROR: previous detection of a radar on this channel(Channel=%d)\n", Channel));
						Success = FALSE;
						break;
					}
					else
					{
						DBGPRINT(RT_DEBUG_INFO, ("RemainingTimeForUse %d ,Channel %d\n",
								pAd->ChannelList[i].RemainingTimeForUse, Channel));
					}
				}
			}
		}

		if (Success == TRUE)
		{
			if ((pAd->CommonCfg.Channel > 14 )
				&& (pAd->CommonCfg.bIEEE80211H == TRUE))
			{
				pAd->Dot11_H.org_ch = pAd->CommonCfg.Channel;
			}

			pAd->CommonCfg.Channel = Channel; //lk added

#ifdef DOT11_N_SUPPORT
			N_ChannelCheck(pAd);
			
			if (WMODE_CAP_N(pAd->CommonCfg.PhyMode) &&
					pAd->CommonCfg.RegTransmitSetting.field.BW == BW_40)
				RFChannel = N_SetCenCh(pAd, pAd->CommonCfg.Channel);
			else
#endif /* DOT11_N_SUPPORT */
				RFChannel = pAd->CommonCfg.Channel;

			DBGPRINT(RT_DEBUG_TRACE, ("%s(): CtrlChannel(%d), CentralChannel(%d) \n", 
							__FUNCTION__, pAd->CommonCfg.Channel,
							pAd->CommonCfg.CentralChannel));

			if ((pAd->CommonCfg.Channel > 14 )
				&& (pAd->CommonCfg.bIEEE80211H == TRUE)
#ifdef WH_EZ_SETUP
				&& (!((wdev->enable_easy_setup == TRUE) && (pAd->Dot11_H.RDMode == RD_NORMAL_MODE)))
#endif

			)
			{
				if (pAd->Dot11_H.RDMode == RD_SILENCE_MODE)
				{
					AsicSwitchChannel(pAd, RFChannel, FALSE);
#ifdef WH_EZ_SETUP					
#ifdef EZ_NETWORK_MERGE_SUPPORT
					//! base channel is different from target channel, do restart AP					
					if (!IS_EZ_SETUP_ENABLED(wdev) || wdev->do_not_restart_interfaces != 1)
					{
#endif					
#endif
						APStop(pAd);
						APStartUp(pAd);
#ifdef WH_EZ_SETUP							
#ifdef EZ_NETWORK_MERGE_SUPPORT
					} else {
							
						if(Channel > 14)
							ap_phy_rrm_init(pAd);
						else
							ap_phy_rrm_init(pAd);
					}
#endif
#endif					
					
				}
				else
				{
					NotifyChSwAnnToPeerAPs(pAd, ZERO_MAC_ADDR, pAd->CurrentAddress, 1, pAd->CommonCfg.Channel);
					pAd->Dot11_H.RDMode = RD_SWITCHING_MODE;
					pAd->Dot11_H.CSCount = 0;
					pAd->Dot11_H.new_channel = Channel;
				}
			}
			else
			{
#ifdef WH_EZ_SETUP					
#ifdef EZ_NETWORK_MERGE_SUPPORT				
				//! base channel is different from target channel, do not restart AP
				if (!IS_EZ_SETUP_ENABLED(wdev) || wdev->do_not_restart_interfaces != 1)
				{
#endif				
#endif
					AsicSwitchChannel(pAd, RFChannel, FALSE);
					APStop(pAd);
					APStartUp(pAd);
#ifdef WH_EZ_SETUP						
#ifdef EZ_NETWORK_MERGE_SUPPORT
				} else {
					if(Channel > 14)
						ap_phy_rrm_init(pAd);
					else
						ap_phy_rrm_init(pAd);
				}

#endif
#endif								
			}
		}
	}
#endif /* CONFIG_AP_SUPPORT */

	if (Success == TRUE)
		DBGPRINT(RT_DEBUG_TRACE, ("Set_Channel_Proc::(Channel=%d)\n", pAd->CommonCfg.Channel));

#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
#ifdef APCLI_AUTO_CONNECT_SUPPORT
			pAd->ApCfg.ApCliAutoConnectChannelSwitching = FALSE;
#endif /* APCLI_AUTO_CONNECT_SUPPORT */
		}
#endif /* CONFIG_AP_SUPPORT */
	return Success;
}

void ez_APScanCnclAction_mt7603(RTMP_ADAPTER *pAd)
{
	APScanCnclAction(pAd, NULL);
}



/* Form and transmit Custom loop detect Pkt*/
 void ez_send_loop_detect_pkt_mt7603(
	IN	PRTMP_ADAPTER	pAd,
	IN  PMAC_TABLE_ENTRY pMacEntry,
	IN  PUCHAR          pOtherCliMac)
{
	UCHAR BROADCAST_ADDR[MAC_ADDR_LEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	UCHAR Header802_3[14]={0};
	UCHAR               CustomPayload[26]={0x45,0x00, // version(4bit), hdr lenght(4bit), tos
		                                   0x00,0x1A, // total ip datagram length
		                                   0x00,0x01, // identification
		                                   0x40,0x00, // flag & fragmentation
										   0x40,0xFD, // TTL, Protocol type
										   0x39,0xE7, // hdr checksum (considered 0 for calculation)
										   0x00,0x00, // Source IP
										   0x00,0x00, // Source IP
										   0xFF,0xFF, // Dest IP
										   0xFF,0xFF, // Dest IP
										   0x11,0x22, // Random Bytes
										   0x33,0x44, // Random bytes
										   0x55,0x66}; // Random bytes

	//Protocol Type (0x3D any host internal protocol,0x3F any local host network,0xFD & 0xFE for experimentation, 0xFF reserved)
	// 2 byte header checksum field, 	 calculated manually by formula and assuming checksum is zero

	//MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_OFF, ("-----> ez_send_loop_detect_pkt\n"));
	//EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,("ez_send_loop_detect_pkt---->\n"));

	NdisZeroMemory(Header802_3,sizeof(UCHAR)*14);

	MAKE_802_3_HEADER(Header802_3, BROADCAST_ADDR, &pMacEntry->wdev->if_addr[0], IPV4TYPE); // or snap type?

	// Using fixed payload due to checksum calculation required using one's complement
	NdisCopyMemory(&CustomPayload[20],pOtherCliMac,MAC_ADDR_LEN);
	//EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,("ez_send_loop_detect_pkt=> Add in payload Other CLi MAC: %02x-%02x-%02x-%02x-%02x-%02x\n",
	//	CustomPayload[20],CustomPayload[21],CustomPayload[22],CustomPayload[23],CustomPayload[24],CustomPayload[25]));

	// Copy frame to Tx ring

	// Pkt 1
	//EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,("ez_send_loop_detect_pkt: wdev_idx:0x%x => Send Loop Detection Pkt 1\n",pMacEntry->wdev->wdev_idx));
	RTMPToWirelessSta((PRTMP_ADAPTER)pAd, pMacEntry,
					 Header802_3, LENGTH_802_3, (PUCHAR)CustomPayload, 26, FALSE);
	OS_WAIT(5);

	// Pkt 2
	//EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,("ez_send_loop_detect_pkt: wdev_idx:0x%x => Send Loop Detection Pkt 2\n",pMacEntry->wdev->wdev_idx));
	RTMPToWirelessSta((PRTMP_ADAPTER)pAd, pMacEntry,
					 Header802_3, LENGTH_802_3, (PUCHAR)CustomPayload, 26, FALSE);
	OS_WAIT(5);

	// Pkt 3
	//EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,("ez_send_loop_detect_pkt: wdev_idx:0x%x => Send Loop Detection Pkt 3\n",pMacEntry->wdev->wdev_idx));
	RTMPToWirelessSta((PRTMP_ADAPTER)pAd, pMacEntry,
					 Header802_3, LENGTH_802_3, (PUCHAR)CustomPayload, 26, FALSE);

	//EZ_DEBUG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_OFF, ("<----- ez_send_loop_detect_pkt\n"));
}


#ifdef DOT11_N_SUPPORT
/*
	========================================================================
	Rakesh: based on ApCliCheckHt

	========================================================================
*/
static BOOLEAN ez_ApStaSetHt(
	IN struct wifi_dev *wdev,
	MAC_TABLE_ENTRY *pEntry)
{
	HT_CAPABILITY_IE *aux_ht_cap;
	UCHAR cfg_ht_bw;
	PRTMP_ADAPTER ad = (PRTMP_ADAPTER)wdev->sys_handle;

	cfg_ht_bw = ez_wlan_config_get_ht_bw_mt7603(ad);

	aux_ht_cap = &pEntry->HTCapability;
	/* choose smaller setting */

#ifdef CONFIG_MULTI_CHANNEL
	aux_ht_cap->HtCapInfo.ChannelWidth = cfg_ht_bw; //pAddHtInfo->AddHtInfo.RecomWidth;
#else /* CONFIG_MULTI_CHANNEL */
	aux_ht_cap->HtCapInfo.ChannelWidth = cfg_ht_bw; //pAddHtInfo->AddHtInfo.RecomWidth & cfg_ht_bw;
#endif /* !CONFIG_MULTI_CHANNEL */

	/* Fix throughput issue for some vendor AP with AES mode */
	if (cfg_ht_bw) // remove pAddHtInfo->AddHtInfo.RecomWidth & check
		aux_ht_cap->HtCapInfo.CCKmodein40 = ad->CommonCfg.HtCapability.HtCapInfo.CCKmodein40;
	else 
		aux_ht_cap->HtCapInfo.CCKmodein40 = 0;

	return TRUE;
}

/*
	========================================================================
	Rakesh: based on ApCliCheckHt

	========================================================================
*/
BOOLEAN ez_ApCliSetHt_mt7603(
	IN APCLI_STRUCT *pApCliEntry,
	MAC_TABLE_ENTRY *pEntry)
{
	HT_CAPABILITY_IE *aux_ht_cap;
	UCHAR cfg_ht_bw;
	PRTMP_ADAPTER ad = (PRTMP_ADAPTER)(pApCliEntry->wdev.sys_handle);

	cfg_ht_bw = ez_wlan_config_get_ht_bw_mt7603(ad);

	aux_ht_cap = &pApCliEntry->MlmeAux.HtCapability;
	/* choose smaller setting */

#ifdef CONFIG_MULTI_CHANNEL
	aux_ht_cap->HtCapInfo.ChannelWidth = cfg_ht_bw; //pAddHtInfo->AddHtInfo.RecomWidth;
#else /* CONFIG_MULTI_CHANNEL */
	aux_ht_cap->HtCapInfo.ChannelWidth = cfg_ht_bw; //pAddHtInfo->AddHtInfo.RecomWidth & cfg_ht_bw;
#endif /* !CONFIG_MULTI_CHANNEL */

	/* Fix throughput issue for some vendor AP with AES mode */
	if (cfg_ht_bw) // remove pAddHtInfo->AddHtInfo.RecomWidth & check
		aux_ht_cap->HtCapInfo.CCKmodein40 = ad->CommonCfg.HtCapability.HtCapInfo.CCKmodein40;
	else 
		aux_ht_cap->HtCapInfo.CCKmodein40 = 0;

	/* Record the RxMcs of AP */
	NdisMoveMemory(pApCliEntry->RxMcsSet, ad->CommonCfg.HtCapability.MCSSet, 16);

	NdisMoveMemory(&pEntry->HTCapability, &pApCliEntry->MlmeAux.HtCapability, sizeof(HT_CAPABILITY_IE));
	NdisMoveMemory(pEntry->HTCapability.MCSSet, pApCliEntry->RxMcsSet, 16);

	return TRUE;
}

#endif

void ez_update_cli_peer_record_mt7603(void * ad_obj, void * wdev_obj, BOOLEAN band_switched, PUCHAR peer_addr)
{
	struct wifi_dev* wdev = wdev_obj;
	struct wifi_dev *ap_wdev = NULL;
	APCLI_STRUCT *pApCliEntry;
	PRTMP_ADAPTER pAd = ad_obj;
	MAC_TABLE_ENTRY *pMacEntry= NULL;
#ifdef DOT11_N_SUPPORT
	UCHAR supported_bw;
#endif
#ifdef DOT11_VHT_AC
	unsigned char vht_bw;
#endif
	pAd = ((PRTMP_ADAPTER)wdev->sys_handle);
	EZ_DEBUG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_INFO,("%s >>>\n", __FUNCTION__));
	
	if (wdev->wdev_type == WDEV_TYPE_AP)
	{
		wdev = &pAd->ApCfg.ApCliTab[wdev->func_idx].wdev;
	}
	pApCliEntry = wdev->func_dev;
	




			pMacEntry = MacTableLookup(wdev->sys_handle,peer_addr);

			if(pMacEntry != NULL){

				// Rakesh: print current info pMacEntry->HTPhyMode.word	pMacEntry->MaxHTPhyMode.word	ClientStatusFlags
				// pApCliEntry->MlmeAux.HtCapability	pMacEntry->HTCapability	(pApCliEntry->MlmeAux.vht_cap), (pApCliEntry->MlmeAux.vht_op)
				// pMacEntry->vht_cap_ie
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,("%s(),MacEntry CURRENT MODE = %d, BW = %d\n", __FUNCTION__, 
					pMacEntry->MaxHTPhyMode.field.MODE, 
					pMacEntry->MaxHTPhyMode.field.BW));

				// Rakesh: following code taken from ApCliLinkUp

				ap_wdev = &(pAd->ApCfg.MBSSID[wdev->func_idx].wdev);

#ifdef DOT11_N_SUPPORT
				supported_bw = ez_wlan_config_get_ht_bw_mt7603(pAd);

				if(pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth != supported_bw){
					MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("**** CmnCfg HtCap chan width not in sync with current ht bw\n"));
					pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth = supported_bw;
				}
				
				ez_ApCliSetHt_mt7603(pApCliEntry,pMacEntry); // will update pEntry->HTCapability
#endif
#ifdef DOT11_VHT_AC
				vht_bw = ez_wlan_config_get_vht_bw_mt7603(wdev);

				if(pAd->CommonCfg.vht_bw != vht_bw){
					MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("**** CmnCfg VHt Cap chan width not in sync with current vht bw\n"));
					pAd->CommonCfg.vht_bw = vht_bw;
				}

				ez_ApCliSetVht(pApCliEntry,pMacEntry); // will update pEntry->vht_cap_ie
#endif

#ifdef DOT11_N_SUPPORT
				/* If this Entry supports 802.11n, upgrade to HT rate. */
				//if (pApCliEntry->MlmeAux.HtCapabilityLen != 0)		Rakesh: todo check peer capability before updating
				if (WMODE_CAP_N(pApCliEntry->wdev.PhyMode))  // Rakesh: code written asuming all device suport same PhyMode
				{
					// Rakesh refer ht_mode_adjust() for partial changes taken here as applicable

					if ( //(peer->HtCapInfo.ChannelWidth) && 			Rakesh: todo check peer capability before updating
						(ez_wlan_config_get_ht_bw_mt7603(pAd)) && (ez_wlan_operate_get_ht_bw_mt7603(pAd)) && (supported_bw == BW_40))
					{
						pMacEntry->MaxHTPhyMode.field.BW= BW_40;
					}
					else
					{	
						pMacEntry->MaxHTPhyMode.field.BW = BW_20;
						pAd->MacTab.fAnyStation20Only = TRUE;
					}
						
					MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,("%s(),MacEntry Updated MODE = %d, BW = %d\n", __FUNCTION__, 
						pMacEntry->MaxHTPhyMode.field.MODE, 
						pMacEntry->MaxHTPhyMode.field.BW));

					pMacEntry->HTPhyMode.word = pMacEntry->MaxHTPhyMode.word;
	
				}
				//else	Rakesh: Todo check AP capability
				//{
				//	pAd->MacTab.fAnyStationIsLegacy = TRUE;
				//	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("ApCliLinkUp - MaxSupRate=%d Mbps\n",
				//					  RateIdToMbps[pMacEntry->MaxSupportedRate]));
				//}
#endif /* DOT11_N_SUPPORT */
				
				
				pMacEntry->HTPhyMode.word = pMacEntry->MaxHTPhyMode.word;

				pApCliEntry->MlmeAux.SupRateLen = pAd->CommonCfg.SupRateLen;//ap_wdev->rate.SupRateLen;
				NdisMoveMemory(pApCliEntry->MlmeAux.SupRate, pAd->CommonCfg.SupRate, pApCliEntry->MlmeAux.SupRateLen); // Rakesh: RTMPCheckRates not required as same phy mode on all devices
				
				pApCliEntry->MlmeAux.ExtRateLen = pAd->CommonCfg.ExtRateLen;
				NdisMoveMemory(pApCliEntry->MlmeAux.ExtRate, pAd->CommonCfg.ExtRate, pApCliEntry->MlmeAux.ExtRateLen); // Rakesh: RTMPCheckRates not required as same phy mode on all devices
				
				RTMPSetSupportMCS(wdev->sys_handle,
								OPMODE_AP,
								pMacEntry,
								pApCliEntry->MlmeAux.SupRate,
								pApCliEntry->MlmeAux.SupRateLen,
								pApCliEntry->MlmeAux.ExtRate,
								pApCliEntry->MlmeAux.ExtRateLen,
#ifdef DOT11_VHT_AC
								pApCliEntry->MlmeAux.vht_cap_len,
								&pApCliEntry->MlmeAux.vht_cap,
#endif /* DOT11_VHT_AC */
								&pApCliEntry->MlmeAux.HtCapability,
								pApCliEntry->MlmeAux.HtCapabilityLen);
				
				//WifiSysApCliChBwUpdate(pAd,pApCliEntry,CliIdx, pMacEntry);
				
#ifdef MT_MAC
				if (pAd->chipCap.hif_type == HIF_MT) {

					if (wdev->bAutoTxRateSwitch == TRUE)
					{
						pMacEntry->bAutoTxRateSwitch = TRUE;
					}
					else
					{
						pMacEntry->HTPhyMode.field.MCS = wdev->HTPhyMode.field.MCS;
						pMacEntry->bAutoTxRateSwitch = FALSE;
	
						/* If the legacy mode is set, overwrite the transmit setting of this entry. */
						RTMPUpdateLegacyTxSetting((UCHAR)wdev->DesiredTransmitSetting.field.FixedTxMode, pMacEntry);
					}

					MlmeRAInit(pAd, pMacEntry);
	
				}
#endif
			}
			else{
				EZ_DEBUG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("ERROR !!! not found other peer mac entry\n"));
				ASSERT(FALSE);
			}

	EZ_DEBUG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_INFO,("%s <<<\n", __FUNCTION__));
}


void ez_update_ap_peer_record_mt7603(void * ad_obj, void * wdev_obj, BOOLEAN band_switched, PUCHAR peer_addr)
{
	struct wifi_dev* wdev = wdev_obj;
	PRTMP_ADAPTER pAd = ad_obj;
	MAC_TABLE_ENTRY *pEntry= NULL;
#ifdef DOT11_N_SUPPORT
	UCHAR supported_bw;
#endif
#ifdef DOT11_VHT_AC
	UCHAR vht_bw;
#endif
	//PRTMP_ADAPTER adOthBand = ez_get_otherband_ad(wdev);
	
	EZ_DEBUG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_INFO,("%s >>>\n",__FUNCTION__));

	if (wdev->wdev_type == WDEV_TYPE_STA)
	{
		wdev = &pAd->ApCfg.MBSSID[wdev->func_idx].wdev;
	}


	EZ_DEBUG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("%s, wdev->idx:%x ap_peer_mac = %02x:%02x:%02x:%02x:%02x:%02x\n", 
		__FUNCTION__,wdev->wdev_idx,PRINT_MAC(peer_addr)));

	pEntry = MacTableLookup(wdev->sys_handle,peer_addr);
	
	if(pEntry != NULL){

		// Rakesh: print current info pMacEntry->HTPhyMode.word	pMacEntry->MaxHTPhyMode.word	ClientStatusFlags
		// pApCliEntry->MlmeAux.HtCapability	pMacEntry->HTCapability	(pApCliEntry->MlmeAux.vht_cap), (pApCliEntry->MlmeAux.vht_op)
		// pMacEntry->vht_cap_ie
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,("%s(),MacEntry CURRENT MODE = %d, BW = %d\n", __FUNCTION__, 
			pEntry->MaxHTPhyMode.field.MODE, 
			pEntry->MaxHTPhyMode.field.BW));

		// Rakesh: following code taken from update_associated_mac_entry & ap_cmm_peer_assoc_req_action	

#ifdef DOT11_N_SUPPORT
		supported_bw = ez_wlan_config_get_ht_bw_mt7603(pAd);

		if(((PRTMP_ADAPTER)wdev->sys_handle)->CommonCfg.HtCapability.HtCapInfo.ChannelWidth != supported_bw){
			EZ_DEBUG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("**** CmnCfg HtCap chan width not in sync with current ht bw\n"));
			((PRTMP_ADAPTER)wdev->sys_handle)->CommonCfg.HtCapability.HtCapInfo.ChannelWidth = supported_bw;
		}
		
		ez_ApStaSetHt(wdev,pEntry); // will update pEntry->HTCapability
#endif
#ifdef DOT11_VHT_AC
		vht_bw = ez_wlan_config_get_vht_bw_mt7603(wdev);

		if(((PRTMP_ADAPTER)wdev->sys_handle)->CommonCfg.vht_bw != vht_bw){
			EZ_DEBUG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("**** CmnCfg VHt Cap chan width not in sync with current vht bw\n"));
			((PRTMP_ADAPTER)wdev->sys_handle)->CommonCfg.vht_bw = vht_bw;
		}

		ez_ApStaSetVht(wdev,pEntry); // will update pEntry->vht_cap_ie
#endif
		RTMPSetSupportMCS(wdev->sys_handle,
						OPMODE_AP,
						pEntry,
						pAd->CommonCfg.SupRate, //ie_list->SupportedRates,	// Rakesh: use ap values as same phy mode on all devices, no need to use RTMPCheckRates as done by Cli
						pAd->CommonCfg.SupRateLen, //ie_list->SupportedRatesLen,
						NULL,  				//wdev->rate.ExtRate can be used
						0,				//wdev->rate.ExtRateLen can be used
#ifdef DOT11_VHT_AC
						sizeof(pEntry->vht_cap_ie),	 //use build_vht_cap_ie??
						&pEntry->vht_cap_ie,
#endif /* DOT11_VHT_AC */
						&pEntry->HTCapability,
						sizeof(pEntry->HTCapability));

#ifdef DOT11_N_SUPPORT

		/* If this Entry supports 802.11n, upgrade to HT rate. */
		if (//(ie_list->ht_cap_len != 0) &&			// Rakesh: todo check peer capability first before updating
			//(wdev->DesiredHtPhyInfo.bHtEnable) &&
			WMODE_CAP_N(wdev->PhyMode))	// Rakesh: current code assumes all devices on same PhyMode
		{
			// Rakesh refer ht_mode_adjust() for partial changes taken here as applicable
			
			if ( //(peer->HtCapInfo.ChannelWidth) &&			Rakesh: todo check peer capability before updating
				(ez_wlan_config_get_ht_bw_mt7603(pAd)) && (ez_wlan_operate_get_ht_bw_mt7603(pAd)) && (supported_bw == BW_40))
			{
				pEntry->MaxHTPhyMode.field.BW= BW_40;
			}
			else
			{	
				pEntry->MaxHTPhyMode.field.BW = BW_20;
				pAd->MacTab.fAnyStation20Only = TRUE;
			}
				
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,("%s(),MacEntry Updated MODE = %d, BW = %d\n", __FUNCTION__, 
				pEntry->MaxHTPhyMode.field.MODE, 
				pEntry->MaxHTPhyMode.field.BW));
			
		}
		//else	Rakesh: todo: check actual cability
		//{
		//	pAd->MacTab.fAnyStationIsLegacy = TRUE;
		//	NdisZeroMemory(&pEntry->HTCapability, sizeof(HT_CAPABILITY_IE));
//#ifdef DOT11_VHT_AC
		//	// TODO: shiang-usw, it's ugly and need to revise it
		//	NdisZeroMemory(&pEntry->vht_cap_ie, sizeof(VHT_CAP_IE));
		//	pEntry->SupportVHTMCS1SS = 0;
		//	pEntry->SupportVHTMCS2SS = 0;
		//	pEntry->SupportVHTMCS3SS = 0;
		//	pEntry->SupportVHTMCS4SS = 0;
		//	pEntry->SupportRateMode &= (~SUPPORT_VHT_MODE);
//#endif /* DOT11_VHT_AC */
		//}
#endif /* DOT11_N_SUPPORT */
	
		pEntry->HTPhyMode.word = pEntry->MaxHTPhyMode.word;

#ifdef MT_MAC
		if (((PRTMP_ADAPTER)wdev->sys_handle)->chipCap.hif_type == HIF_MT)
		{
			if (wdev->bAutoTxRateSwitch == TRUE)
			{
				pEntry->bAutoTxRateSwitch = TRUE;
			}
			else
			{
				pEntry->HTPhyMode.field.MCS = wdev->HTPhyMode.field.MCS;
				pEntry->bAutoTxRateSwitch = FALSE;
	
#ifdef WFA_VHT_PF
				if (WMODE_CAP_AC(wdev->PhyMode)) {
					pEntry->HTPhyMode.field.MCS = wdev->DesiredTransmitSetting.field.MCS +
												((wlan_config_get_tx_stream(wdev) - 1) << 4);
				}
#endif /* WFA_VHT_PF */

#ifdef DOT11_VHT_AC
				if (pEntry->HTPhyMode.field.MODE == MODE_VHT)
				{
					pEntry->HTPhyMode.field.MCS = wdev->DesiredTransmitSetting.field.MCS +
												((wlan_config_get_tx_stream(wdev) - 1) << 4);
				}
#endif
				/* If the legacy mode is set, overwrite the transmit setting of this entry. */
				RTMPUpdateLegacyTxSetting((UCHAR)wdev->DesiredTransmitSetting.field.FixedTxMode, pEntry);
			}
	
		}
#endif /* MT_MAC */

	//	WifiSysApPeerChBwUpdate(((PRTMP_ADAPTER)wdev->sys_handle),pEntry);//,ie_list); // Rakesh: todo pass peer capabilities

	}
	else{
		EZ_DEBUG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("ERROR !!! not found other peer mac entry\n"));
		ASSERT(FALSE);
	}

	EZ_DEBUG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_INFO,("%s <<<\n", __FUNCTION__));


}

//----------------------------------------------------------------------------------------------------------
//											chip ops
//-----------------------------------------------------------------------------------------------------------

#endif

