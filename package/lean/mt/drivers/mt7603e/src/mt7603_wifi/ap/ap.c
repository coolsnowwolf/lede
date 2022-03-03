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
    soft_ap.c

    Abstract:
    Access Point specific routines and MAC table maintenance routines

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    John Chang  08-04-2003    created for 11g soft-AP

 */

#include "rt_config.h"
#include "ap.h"


#ifdef SW_ATF_SUPPORT
#define WcidTxThr	43
#define goodNodeMinDeqThr 433
#define goodNodeMaxDeqThr 520
#define badNodeMinDeqThr 43
#define badNodeMinEnqThr 3
#define badNodeMaxEnqThr 4
#define FalseCCAThr 200
#endif

#ifdef MULTI_CLIENT_SUPPORT
#define MULTI_CLIENT_NUM 5
UINT is_multiclient_mode_on(RTMP_ADAPTER *pAd)
{
	UINT res = 0;
	//INT test = 1; //lk added for test
	if (pAd->bManualMultiClientOn == 99)
		return res;

	if (pAd->MultiClientOnMode == 0) 
	{
		/* check by MacTable Size */
		if (pAd->MacTab.Size >= MULTI_CLIENT_NUM)
			res = 1; 
	}
#ifdef TRAFFIC_BASED_TXOP
	else if (pAd->MultiClientOnMode == 1)
	{
		/* check by pEntry's Traffic */
		if (pAd->StaTxopAbledCnt >= MULTI_CLIENT_NUM)
			res = 1;
	}
#endif

	return ( res || pAd->bManualMultiClientOn); 
}
#endif

#ifdef INTERFERENCE_RA_SUPPORT
UINT is_interference_mode_on(RTMP_ADAPTER *pAd)
{
       UINT res = 0;
       
       if (pAd->CommonCfg.Interfra >= 1)
               res = 1;

       return res;
}
#endif
     
char const *pEventText[EVENT_MAX_EVENT_TYPE] = {
	"restart access point",
	"successfully associated",
	"has disassociated",
	"has been aged-out and disassociated" ,
	"active countermeasures",
	"has disassociated with invalid PSK password"};


UCHAR get_apidx_by_addr(RTMP_ADAPTER *pAd, UCHAR *addr)
{
	UCHAR apidx;

	for (apidx=0; apidx<pAd->ApCfg.BssidNum; apidx++)
	{
		if (RTMPEqualMemory(addr, pAd->ApCfg.MBSSID[apidx].wdev.bssid, MAC_ADDR_LEN))
			break;
	}

	return apidx;
}


// TODO: shiang-usw, need to revise this to asic specific functions!
INT set_wdev_if_addr(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, INT opmode)
{
	INT idx = wdev->func_idx;

	if (opmode == OPMODE_AP)
	{
		COPY_MAC_ADDR(wdev->if_addr, pAd->CurrentAddress);
//+++Add by Carter for MT7603
#if defined(RTMP_MAC) || defined(RLT_MAC)
		if (pAd->chipCap.hif_type == HIF_RTMP || pAd->chipCap.hif_type == HIF_RLT)
		{
			if (pAd->chipCap.MBSSIDMode >= MBSSID_MODE1)
			{
				UCHAR MacMask = 0;

				if ((pAd->ApCfg.BssidNum + MAX_APCLI_NUM + MAX_MESH_NUM) <= 2)
					MacMask = 0xFE;
				else if ((pAd->ApCfg.BssidNum + MAX_APCLI_NUM + MAX_MESH_NUM) <= 4)
					MacMask = 0xFC;
				else if ((pAd->ApCfg.BssidNum + MAX_APCLI_NUM + MAX_MESH_NUM) <= 8)
					MacMask = 0xF8;
				else if ((pAd->ApCfg.BssidNum + MAX_APCLI_NUM + MAX_MESH_NUM) <= 16)
					MacMask = 0xF0;

				if (idx > 0)
				{
					wdev->if_addr[0] |= 0x2;
					if (pAd->chipCap.MBSSIDMode == MBSSID_MODE1)
					{
						/*
							Refer to HW definition -
							Bit1 of MAC address Byte0 is local administration bit
							and should be set to 1 in extended multiple BSSIDs'
							Bit3~ of MAC address Byte0 is extended multiple BSSID index.
						 */
#ifdef ENHANCE_NEW_MBSSID_MODE
						wdev->if_addr[0] &= ((MacMask << 2) + 3);
#endif /* ENHANCE_NEW_MBSSID_MODE */
						wdev->if_addr[0] += ((wdev->func_idx - 1) << 2);
					}
#ifdef ENHANCE_NEW_MBSSID_MODE
					else
					{
						wdev->if_addr[pAd->chipCap.MBSSIDMode - 1] &= (MacMask);
						wdev->if_addr[pAd->chipCap.MBSSIDMode - 1] += (idx - 1);
					}
#endif /* ENHANCE_NEW_MBSSID_MODE */
				}
			}
			else
				wdev->if_addr[5] += idx;
		}
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */

#ifdef MT_MAC
		if (pAd->chipCap.hif_type == HIF_MT)
		{
			//TODO: Carter, Apcli interface and MESH interface shall use HWBSSID1 or HWBSSID2???
			UINT32 Value;
			UCHAR MacMask = 0;

			//TODO: shall we make choosing which byte to be selectable???
			Value = 0x00000000;
			RTMP_IO_READ32(pAd, LPON_BTEIR, &Value);//read BTEIR bit[31:29] for determine to choose which byte to extend BSSID mac address.
						Value = Value | ((pAd->chipCap.MBSSIDMode -2) << 29);
						RTMP_IO_WRITE32(pAd, LPON_BTEIR, Value);

			Value = 0x00000000;
			RTMP_IO_READ32(pAd, RMAC_RMACDR, &Value);
			Value = Value & 0xfcffffff;/* clear bit[25:24] */

			if (pAd->ApCfg.BssidNum <= 2) {
				Value &= ~RMACDR_MBSSID_MASK;
				Value |= RMACDR_MBSSID(0x0);
				MacMask = 0xef;
			}
			else if (pAd->ApCfg.BssidNum <= 4) {
				Value &= ~RMACDR_MBSSID_MASK;
				Value |= RMACDR_MBSSID(0x1);
				MacMask = 0xcf;
			}
			else if (pAd->ApCfg.BssidNum <= 8) {
				Value &= ~RMACDR_MBSSID_MASK;
				Value |= RMACDR_MBSSID(0x2);
				MacMask = 0x8f;
			}
			else if (pAd->ApCfg.BssidNum <= 16) {
				Value &= ~RMACDR_MBSSID_MASK;
				Value |= RMACDR_MBSSID(0x3);
				MacMask = 0x0f;
			}
			else {
				Value &= ~RMACDR_MBSSID_MASK;
				Value |= RMACDR_MBSSID(0x3);
				MacMask = 0x0f;
			}

			RTMP_IO_WRITE32(pAd, RMAC_RMACDR, Value);

			if (idx > 0)
			{
				/* MT7603, bit1 in byte0 shall always be b'1 for Multiple BSSID */
				wdev->if_addr[0] |= 0x2;

				switch ((pAd->chipCap.MBSSIDMode -2)) {				
					case 0x1: /* choose bit[23:20]*/
						//mapping to MBSSID_MODE3
						wdev->if_addr[2] = wdev->if_addr[2] & MacMask;//clear high 4 bits,
						wdev->if_addr[2] = (wdev->if_addr[2] | (idx << 4));
						break;
					case 0x2: /* choose bit[31:28]*/
						//mapping to MBSSID_MODE4
						wdev->if_addr[3] = wdev->if_addr[3] & MacMask;//clear high 4 bits,
						wdev->if_addr[3] = (wdev->if_addr[3] | (idx << 4));
						break;
					case 0x3: /* choose bit[39:36]*/
						//mapping to MBSSID_MODE5
						wdev->if_addr[4] = wdev->if_addr[4] & MacMask;//clear high 4 bits,
						wdev->if_addr[4] = (wdev->if_addr[4] | (idx << 4));
						break;
					case 0x4: /* choose bit [47:44]*/
						//mapping to MBSSID_MODE6
						wdev->if_addr[5] = wdev->if_addr[5] & MacMask;//clear high 4 bits,
						wdev->if_addr[5] = (wdev->if_addr[5] | (idx << 4));
						break;
					default: /* choose bit[15:12]*/
						//mapping to MBSSID_MODE2
						wdev->if_addr[1] = wdev->if_addr[1] & MacMask;//clear high 4 bits,
						wdev->if_addr[1] = (wdev->if_addr[1] | (idx << 4));
						break;
				}
			}
		}
#endif /* MT_MAC */
//---Add by Carter for MT7603
	}

	return 0;
}


/*
	==========================================================================
	Description:
		Initialize AP specific data especially the NDIS packet pool that's
		used for wireless client bridging.
	==========================================================================
 */
NDIS_STATUS APInitialize(RTMP_ADAPTER *pAd)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	INT i;

	DBGPRINT(RT_DEBUG_TRACE, ("---> APInitialize\n"));

	/* Init Group key update timer, and countermeasures timer */
	for (i = 0; i < MAX_MBSSID_NUM(pAd); i++)
		RTMPInitTimer(pAd, &pAd->ApCfg.MBSSID[i].REKEYTimer, GET_TIMER_FUNCTION(GREKEYPeriodicExec), pAd,  TRUE);

	RTMPInitTimer(pAd, &pAd->ApCfg.CounterMeasureTimer, GET_TIMER_FUNCTION(CMTimerExec), pAd, FALSE);
#ifndef BCN_OFFLOAD_SUPPORT
#endif

#ifdef IDS_SUPPORT
	/* Init intrusion detection timer */
	RTMPInitTimer(pAd, &pAd->ApCfg.IDSTimer, GET_TIMER_FUNCTION(RTMPIdsPeriodicExec), pAd, FALSE);
	pAd->ApCfg.IDSTimerRunning = FALSE;
#endif /* IDS_SUPPORT */

#ifdef WAPI_SUPPORT
	/* Init WAPI rekey timer */
	RTMPInitWapiRekeyTimerAction(pAd, NULL);
#endif /* WAPI_SUPPORT */

#ifdef IGMP_SNOOP_SUPPORT
	MulticastFilterTableInit(pAd, &pAd->pMulticastFilterTable);
#endif /* IGMP_SNOOP_SUPPORT */

#ifdef DOT11V_WNM_SUPPORT
	initList(&pAd->DMSEntryList);
#endif /* DOT11V_WNM_SUPPORT */

#ifdef DOT11K_RRM_SUPPORT
	RRM_CfgInit(pAd);
#endif /* DOT11K_RRM_SUPPORT */

	DBGPRINT(RT_DEBUG_TRACE, ("<--- APInitialize\n"));
	return Status;
}


/*
	==========================================================================
	Description:
		Shutdown AP and free AP specific resources
	==========================================================================
 */
VOID APShutdown(RTMP_ADAPTER *pAd)
{
	DBGPRINT(RT_DEBUG_TRACE, ("---> APShutdown\n"));

		MlmeRadioOff(pAd);

#ifdef RTMP_MAC_PCI
		APStop(pAd);
#endif /* RTMP_MAC_PCI */

#ifdef IGMP_SNOOP_SUPPORT
	MultiCastFilterTableReset(&pAd->pMulticastFilterTable);
#endif /* IGMP_SNOOP_SUPPORT */

#ifdef DOT11V_WNM_SUPPORT
	DMSTable_Release(pAd);
#endif /* DOT11V_WNM_SUPPORT */

	NdisFreeSpinLock(&pAd->MacTabLock);

#ifdef WDS_SUPPORT
	NdisFreeSpinLock(&pAd->WdsTabLock);
#endif /* WDS_SUPPORT */

	DBGPRINT(RT_DEBUG_TRACE, ("<--- APShutdown\n"));
}


#ifdef DOT11W_PMF_SUPPORT
static INT ap_pmf_init(RTMP_ADAPTER *pAd, BSS_STRUCT *pMbss, INT idx)
{
	struct wifi_dev *wdev = &pMbss->wdev;

	/*
	   IEEE 802.11W/P.10 -
	   A STA that has associated with Management Frame Protection enabled
	   shall not use pairwise cipher suite selectors WEP-40, WEP-104,
	   TKIP, or "Use Group cipher suite".

	   IEEE 802.11W/P.3 -
	   IEEE Std 802.11 provides one security protocol, CCMP, for protection
	   of unicast Robust Management frames.
	 */
	pMbss->PmfCfg.MFPC = FALSE;
	pMbss->PmfCfg.MFPR = FALSE;
	pMbss->PmfCfg.PMFSHA256 = FALSE;

#ifdef DOT11_SAE_SUPPORT
	if (wdev->AuthMode == Ndis802_11AuthModeWPA3PSK || wdev->AuthMode == Ndis802_11AuthModeWPA2PSKWPA3PSK) {
	/* In WPA3 spec, When a WPA3-Personal only BSS is configured,
	* Protected Management Frame (PMF) shall be set to required (MFPR=1)
	* When WPA2-Personal and WPA3-Personal are configured on the same BSS (mixed mode),
	* Protected Management Frame (PMF) shall be set to capable (MFPC = 1, MFPR = 0)
	*/
		pMbss->PmfCfg.MFPC = TRUE;
		pMbss->PmfCfg.MFPR = (wdev->AuthMode == Ndis802_11AuthModeWPA2PSKWPA3PSK) ? FALSE : TRUE;
	}
#endif
#ifdef CONFIG_OWE_SUPPORT
	if (wdev->AuthMode == Ndis802_11AuthModeOWE) {
		pMbss->PmfCfg.MFPC = TRUE;
		pMbss->PmfCfg.MFPR = TRUE;
	}
#endif
#if defined(DOT11_SAE_SUPPORT) || defined(CONFIG_OWE_SUPPORT)
	if (pMbss->PmfCfg.Desired_MFPR != pMbss->PmfCfg.MFPR &&
		(wdev->AuthMode == Ndis802_11AuthModeWPA3PSK ||
		wdev->AuthMode == Ndis802_11AuthModeWPA2PSKWPA3PSK ||
		wdev->AuthMode == Ndis802_11AuthModeOWE)) {
		DBGPRINT(RT_DEBUG_ERROR, ("[PMF]%s:: PmfCfg Desired MFPR error\n", __func__));
	}
#endif
#if defined(DOT11_SAE_SUPPORT) || defined(CONFIG_OWE_SUPPORT)
	if (pMbss->PmfCfg.Desired_MFPC != pMbss->PmfCfg.MFPC &&
		(wdev->AuthMode == Ndis802_11AuthModeWPA3PSK ||
		wdev->AuthMode == Ndis802_11AuthModeWPA2PSKWPA3PSK ||
		wdev->AuthMode == Ndis802_11AuthModeOWE)) {
		DBGPRINT(RT_DEBUG_ERROR, ("[PMF]%s:: PmfCfg Desired MFPC error\n", __func__));
	}
#endif
	if (((((wdev->AuthMode == Ndis802_11AuthModeWPA2) ||
		(wdev->AuthMode == Ndis802_11AuthModeWPA2PSK)) &&
		(pMbss->PmfCfg.Desired_MFPC))
	#if defined(DOT11_SAE_SUPPORT) || defined(CONFIG_OWE_SUPPORT)
	|| (((wdev->AuthMode == Ndis802_11AuthModeWPA3PSK) ||
		(wdev->AuthMode == Ndis802_11AuthModeWPA2PSKWPA3PSK) ||
		wdev->AuthMode == Ndis802_11AuthModeOWE))
	#endif
	)
	&& (wdev->WepStatus == Ndis802_11AESEnable))
	{
		pMbss->PmfCfg.MFPC = TRUE;

#if defined(DOT11_SAE_SUPPORT) || defined(CONFIG_OWE_SUPPORT)
		if (wdev->AuthMode != Ndis802_11AuthModeWPA3PSK &&
			wdev->AuthMode != Ndis802_11AuthModeWPA2PSKWPA3PSK &&
			wdev->AuthMode != Ndis802_11AuthModeOWE)
#endif
		pMbss->PmfCfg.MFPR = pMbss->PmfCfg.Desired_MFPR;

		/* IGTK default key index as 4 */
		pMbss->PmfCfg.IGTK_KeyIdx = 4;

		/* Derive IGTK */
		PMF_DeriveIGTK(pAd, &pMbss->PmfCfg.IGTK[0][0]);

#ifdef MT_MAC
		if ((pAd->chipCap.hif_type == HIF_MT) && (pAd->chipCap.FlgPMFEncrtptMode == PMF_ENCRYPT_MODE_2))
		{
			CIPHER_KEY		CipherKey;
			USHORT Wcid;

			GET_PMF_GroupKey_WCID(pAd, Wcid, pMbss->mbss_idx);
			CipherKey.KeyLen = 16;
			memcpy(CipherKey.Key, &pMbss->PmfCfg.IGTK[0][0], CipherKey.KeyLen);
			CipherKey.CipherAlg = CIPHER_BIP;
			CmdProcAddRemoveKey(pAd, 0, pMbss->mbss_idx, 0, Wcid, SHAREDKEYTABLE, &CipherKey, BROADCAST_ADDR);
		}
#endif
                    if ((pMbss->PmfCfg.Desired_PMFSHA256) || (pMbss->PmfCfg.MFPR))
                            pMbss->PmfCfg.PMFSHA256 = TRUE;

	} else if (pMbss->PmfCfg.Desired_MFPC) {
                    DBGPRINT(RT_DEBUG_ERROR, ("[PMF]%s:: Security is not WPA2/WPA2PSK AES\n", __FUNCTION__));
	}

            DBGPRINT(RT_DEBUG_ERROR, ("[PMF]%s:: apidx=%d, MFPC=%d, MFPR=%d, SHA256=%d\n",
						__FUNCTION__, idx, pMbss->PmfCfg.MFPC,
						pMbss->PmfCfg.MFPR, pMbss->PmfCfg.PMFSHA256));

			return TRUE;
}
#endif /* DOT11W_PMF_SUPPORT */



INT ap_security_init(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, INT idx)
{
#ifdef DOT11W_PMF_SUPPORT
		ap_pmf_init(pAd, &pAd->ApCfg.MBSSID[idx], idx);
#endif /* DOT11W_PMF_SUPPORT */

	/* decide the mixed WPA cipher combination */
	if (wdev->WepStatus == Ndis802_11TKIPAESMix)
	{
		switch ((UCHAR)wdev->AuthMode)
		{
			/* WPA mode */
			case Ndis802_11AuthModeWPA:
			case Ndis802_11AuthModeWPAPSK:
				wdev->WpaMixPairCipher = WPA_TKIPAES_WPA2_NONE;
				break;

			/* WPA2 mode */
			case Ndis802_11AuthModeWPA2:
			case Ndis802_11AuthModeWPA2PSK:
				wdev->WpaMixPairCipher = WPA_NONE_WPA2_TKIPAES;
				break;

			/* WPA and WPA2 both mode */
			case Ndis802_11AuthModeWPA1WPA2:
			case Ndis802_11AuthModeWPA1PSKWPA2PSK:

				/* In WPA-WPA2 and TKIP-AES mixed mode, it shall use the maximum */
				/* cipher capability unless users assign the desired setting. */
				if (wdev->WpaMixPairCipher == MIX_CIPHER_NOTUSE ||
					wdev->WpaMixPairCipher == WPA_TKIPAES_WPA2_NONE ||
					wdev->WpaMixPairCipher == WPA_NONE_WPA2_TKIPAES)
					wdev->WpaMixPairCipher = WPA_TKIPAES_WPA2_TKIPAES;
				break;
		}

	}
	else
		wdev->WpaMixPairCipher = MIX_CIPHER_NOTUSE;

	if (wdev->WepStatus == Ndis802_11Encryption2Enabled ||
		wdev->WepStatus == Ndis802_11Encryption3Enabled ||
		wdev->WepStatus == Ndis802_11Encryption4Enabled)
		{
			RT_CfgSetWPAPSKKey(pAd, pAd->ApCfg.MBSSID[idx].WPAKeyString,
				strlen(pAd->ApCfg.MBSSID[idx].WPAKeyString),
				(PUCHAR)pAd->ApCfg.MBSSID[idx].Ssid,
				pAd->ApCfg.MBSSID[idx].SsidLen,
				pAd->ApCfg.MBSSID[idx].PMK);
		}
	/* Generate the corresponding RSNIE */
	RTMPMakeRSNIE(pAd, wdev->AuthMode, wdev->WepStatus, idx);

		return TRUE;
}

#ifndef WH_EZ_SETUP
static 
#endif
INT ap_key_tb_init(RTMP_ADAPTER *pAd)
{
	BSS_STRUCT *pMbss;
	struct wifi_dev *wdev;
	USHORT Wcid;
	INT idx, i;

	/*
		Initialize security variable per entry,
		1. 	pairwise key table, re-set all WCID entry as NO-security mode.
		2.	access control port status
	*/
	/* Init Security variables */
	for (idx = 0; idx < pAd->ApCfg.BssidNum; idx++)
	{
		pMbss = &pAd->ApCfg.MBSSID[idx];
		wdev = &pAd->ApCfg.MBSSID[idx].wdev;
		Wcid = 0;

		wdev->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
		if (IS_WPA_CAPABILITY(wdev->AuthMode))
			wdev->DefaultKeyId = 1;

		/* Get a specific WCID to record this MBSS key attribute */
		GET_GroupKey_WCID(pAd, Wcid, idx);

		/* When WEP, TKIP or AES is enabled, set group key info to Asic */
		if (wdev->WepStatus == Ndis802_11WEPEnabled)
		{
			UCHAR CipherAlg, key_idx;

			for (key_idx=0; key_idx < SHARE_KEY_NUM; key_idx++)
			{
				CipherAlg = pAd->SharedKey[idx][key_idx].CipherAlg;

				if (pAd->SharedKey[idx][key_idx].KeyLen > 0)
				{
					/* Set key material to Asic */
					AsicAddSharedKeyEntry(pAd, idx, key_idx, &pAd->SharedKey[idx][key_idx]);

					if (key_idx == wdev->DefaultKeyId)
					{
						/* Generate 3-bytes IV randomly for software encryption using */
						for(i = 0; i < LEN_WEP_TSC; i++)
							pAd->SharedKey[idx][key_idx].TxTsc[i] = RandomByte(pAd);

						/* Update WCID attribute table and IVEIV table */
						RTMPSetWcidSecurityInfo(pAd,
										idx,
										key_idx,
										CipherAlg,
										Wcid,
										SHAREDKEYTABLE);

#ifdef MT_MAC
						if (pAd->chipCap.hif_type == HIF_MT)
							CmdProcAddRemoveKey(pAd, 0, idx, key_idx, Wcid, SHAREDKEYTABLE, &pAd->SharedKey[idx][key_idx], BROADCAST_ADDR);
#endif
					}
				}
			}
		}
		else if ((wdev->WepStatus == Ndis802_11TKIPEnable) ||
				 (wdev->WepStatus == Ndis802_11AESEnable) ||
				 (wdev->WepStatus == Ndis802_11TKIPAESMix))
		{
			/* Generate GMK and GNonce randomly per MBSS */
			GenRandom(pAd, wdev->bssid, pMbss->GMK);
			GenRandom(pAd, wdev->bssid, pMbss->GNonce);

			/* Derive GTK per BSSID */
			WpaDeriveGTK(pMbss->GMK,
						(UCHAR*)pMbss->GNonce,
						wdev->bssid,
						pMbss->GTK,
						LEN_TKIP_GTK);

			/* Install Shared key */
			WPAInstallSharedKey(pAd,
								wdev->GroupKeyWepStatus,
								idx,
								wdev->DefaultKeyId,
								Wcid,
								TRUE,
								pMbss->GTK,
								LEN_TKIP_GTK);

		}
#ifdef WAPI_SUPPORT
		else if (pMbss->wdev.WepStatus == Ndis802_11EncryptionSMS4Enabled)
		{
			INT	cnt;

			/* Initial the related variables */
			pMbss->wdev.DefaultKeyId = 0;
			NdisMoveMemory(pMbss->key_announce_flag, AE_BCAST_PN, LEN_WAPI_TSC);
			if (IS_HW_WAPI_SUPPORT(pAd))
				pMbss->sw_wpi_encrypt = FALSE;
			else
				pMbss->sw_wpi_encrypt = TRUE;

			/* Generate NMK randomly */
			for (cnt = 0; cnt < LEN_WAPI_NMK; cnt++)
				pMbss->NMK[cnt] = RandomByte(pAd);

			/* Count GTK for this BSSID */
			RTMPDeriveWapiGTK(pMbss->NMK, pMbss->GTK);

			/* Install Shared key */
			WAPIInstallSharedKey(pAd,
								 wdev->GroupKeyWepStatus,
								 idx,
								 wdev->DefaultKeyId,
								 Wcid,
								 pMbss->GTK);

		}
#endif /* WAPI_SUPPORT */

#ifdef DOT1X_SUPPORT
		/* Send singal to daemon to indicate driver had restarted */
		if ((wdev->AuthMode == Ndis802_11AuthModeWPA) || (wdev->AuthMode == Ndis802_11AuthModeWPA2)
        		|| (wdev->AuthMode == Ndis802_11AuthModeWPA1WPA2) || (wdev->IEEE8021X == TRUE))
		{
			;/*bDot1xReload = TRUE; */
    		}
#endif /* DOT1X_SUPPORT */

		DBGPRINT(RT_DEBUG_TRACE, ("### BSS(%d) AuthMode(%d)=%s, WepStatus(%d)=%s, AccessControlList.Policy=%ld\n",
					idx, wdev->AuthMode, GetAuthMode(wdev->AuthMode),
					wdev->WepStatus, GetEncryptType(wdev->WepStatus),
					pMbss->AccessControlList.Policy));
	}


	return TRUE;
}

INT ap_key_tb_single_init(RTMP_ADAPTER *pAd, BSS_STRUCT *pDstMbss)
{
	BSS_STRUCT *pMbss;
	struct wifi_dev *wdev;
	USHORT Wcid;
	INT idx, i;

/*
*	Initialize security variable per entry,
*	1.	pairwise key table, re-set all WCID entry as NO-security mode.
*	2.	access control port status
*/
	/* Init Security variables */

	for (idx = 0; idx < pAd->ApCfg.BssidNum; idx++) {
		pMbss = &pAd->ApCfg.MBSSID[idx];
		wdev = &pAd->ApCfg.MBSSID[idx].wdev;
		Wcid = 0;

		if (pMbss == pDstMbss) {
			wdev->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
			if (IS_WPA_CAPABILITY(wdev->AuthMode))
				wdev->DefaultKeyId = 1;

			/* Get a specific WCID to record this MBSS key attribute */
			GET_GroupKey_WCID(pAd, Wcid, idx);

			/* When WEP, TKIP or AES is enabled, set group key info to Asic */
			if (wdev->WepStatus == Ndis802_11WEPEnabled) {
				UCHAR CipherAlg, key_idx;

				for (key_idx = 0; key_idx < SHARE_KEY_NUM; key_idx++) {
					CipherAlg = pAd->SharedKey[idx][key_idx].CipherAlg;

					if (pAd->SharedKey[idx][key_idx].KeyLen > 0) {
						/* Set key material to Asic */
						AsicAddSharedKeyEntry(pAd, idx, key_idx,
								&pAd->SharedKey[idx][key_idx]);

					if (key_idx == wdev->DefaultKeyId) {
						/* Generate 3-bytes IV randomly for software encryption using */
							for (i = 0; i < LEN_WEP_TSC; i++)
								pAd->SharedKey[idx][key_idx].TxTsc[i] = RandomByte(pAd);

							/* Update WCID attribute table and IVEIV table */
							RTMPSetWcidSecurityInfo(pAd,
												idx,
												key_idx,
												CipherAlg,
												Wcid,
												SHAREDKEYTABLE);
#ifdef MT_MAC
							if (pAd->chipCap.hif_type == HIF_MT)
								CmdProcAddRemoveKey(pAd, 0, idx, key_idx, Wcid,
											SHAREDKEYTABLE,
											&pAd->SharedKey[idx][key_idx],
											BROADCAST_ADDR);
#endif
						}
					}
				}
			} else if ((wdev->WepStatus == Ndis802_11TKIPEnable) ||
						(wdev->WepStatus == Ndis802_11AESEnable) ||
						(wdev->WepStatus == Ndis802_11TKIPAESMix)) {
					/* Generate GMK and GNonce randomly per MBSS */
					GenRandom(pAd, wdev->bssid, pMbss->GMK);
					GenRandom(pAd, wdev->bssid, pMbss->GNonce);

					/* Derive GTK per BSSID */
					WpaDeriveGTK(pMbss->GMK,
								(UCHAR *)pMbss->GNonce,
								wdev->bssid,
								pMbss->GTK,
								LEN_TKIP_GTK);

					/* Install Shared key */
					WPAInstallSharedKey(pAd,
										wdev->GroupKeyWepStatus,
										idx,
										wdev->DefaultKeyId,
										Wcid,
										TRUE,
										pMbss->GTK,
										LEN_TKIP_GTK);

			}
#ifdef WAPI_SUPPORT
			else if (pMbss->wdev.WepStatus == Ndis802_11EncryptionSMS4Enabled) {
				INT cnt;

				/* Initial the related variables */
				pMbss->wdev.DefaultKeyId = 0;
				NdisMoveMemory(pMbss->key_announce_flag, AE_BCAST_PN, LEN_WAPI_TSC);
				if (IS_HW_WAPI_SUPPORT(pAd))
					pMbss->sw_wpi_encrypt = FALSE;
				else
					pMbss->sw_wpi_encrypt = TRUE;

				/* Generate NMK randomly */
				for (cnt = 0; cnt < LEN_WAPI_NMK; cnt++)
					pMbss->NMK[cnt] = RandomByte(pAd);

				/* Count GTK for this BSSID */
				RTMPDeriveWapiGTK(pMbss->NMK, pMbss->GTK);

				/* Install Shared key */
				WAPIInstallSharedKey(pAd,
									 wdev->GroupKeyWepStatus,
									 idx,
									 wdev->DefaultKeyId,
									 Wcid,
									 pMbss->GTK);

				}
#endif /* WAPI_SUPPORT */

#ifdef DOT1X_SUPPORT
				/* Send signal to daemon to indicate driver had restarted */
				if ((wdev->AuthMode == Ndis802_11AuthModeWPA) ||
					(wdev->AuthMode == Ndis802_11AuthModeWPA2) ||
					(wdev->AuthMode == Ndis802_11AuthModeWPA1WPA2) ||
					(wdev->IEEE8021X == TRUE)) {
					;/*bDot1xReload = TRUE; */
					}
#endif /* DOT1X_SUPPORT */

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("### BSS(%d) AuthMode(%d)=%s, WepStatus(%d)=%s, AccessControlList.Policy=%ld\n",
				idx, wdev->AuthMode, GetAuthMode(wdev->AuthMode),
				wdev->WepStatus, GetEncryptType(wdev->WepStatus),
				pMbss->AccessControlList.Policy));
		}

		}

	return TRUE;
}


static INT ap_hw_tb_init(RTMP_ADAPTER *pAd)
{
	INT i;
#ifdef A4_CONN
	APCLI_STRUCT *pApCliEntry = NULL;
#endif
	MAC_TABLE_ENTRY *pEntry = NULL;

	DBGPRINT(RT_DEBUG_TRACE, ("%s():Reset WCID Table\n", __FUNCTION__));

	for (i=0; i<MAX_LEN_OF_MAC_TABLE; i++)
	{
		pEntry = &pAd->MacTab.Content[i];
#ifdef A4_CONN
		if (IS_ENTRY_APCLI(pEntry)) {
			pApCliEntry = &pAd->ApCfg.ApCliTab[pEntry->func_tb_idx];
			if (IS_APCLI_A4(pApCliEntry) && pEntry->Sst == SST_ASSOC
				&& pAd->MacTab.tr_entry[i].PortSecured == WPA_802_1X_PORT_SECURED)
				continue;
		}
		if (pEntry->wcid == APCLI_MCAST_WCID)
			continue;
#endif
		AsicDelWcidTab(pAd, pEntry->wcid);
		pAd->MacTab.tr_entry[i].PortSecured  = WPA_802_1X_PORT_NOT_SECURED;
		AsicRemovePairwiseKeyEntry(pAd, (UCHAR)i);
	}

	return TRUE;
}

/*Nobody uses it currently*/
INT ap_phy_rrm_init(RTMP_ADAPTER *pAd)
{

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
		AsicSetTxStream(pAd, pAd->Antenna.field.TxPath);
	else
#endif /* MT_MAC */
	ASIC_RLT_SET_TX_STREAM(pAd, OPMODE_AP, TRUE);

	AsicSetRxStream(pAd, pAd->Antenna.field.RxPath);

	pAd->CommonCfg.CentralChannel = pAd->CommonCfg.Channel;

	// TODO: shiang-usw, get from MT7620_MT7610 Single driver, check this!!
	N_ChannelCheck(pAd);//correct central channel offset

	AsicBBPAdjust(pAd);
#ifdef DOT11_VHT_AC
	if (pAd->CommonCfg.BBPCurrentBW == BW_80)
		pAd->hw_cfg.cent_ch = pAd->CommonCfg.vht_cent_ch;
	else
#endif /* DOT11_VHT_AC */
		pAd->hw_cfg.cent_ch = pAd->CommonCfg.CentralChannel;

	AsicSwitchChannel(pAd, pAd->hw_cfg.cent_ch, FALSE);
	AsicLockChannel(pAd, pAd->hw_cfg.cent_ch);

#ifdef DOT11_VHT_AC
//+++Add by shiang for debug
	DBGPRINT(RT_DEBUG_OFF, ("%s(): AP Set CentralFreq at %d(Prim=%d, HT-CentCh=%d, VHT-CentCh=%d, BBP_BW=%d)\n",
						__FUNCTION__, pAd->hw_cfg.cent_ch, pAd->CommonCfg.Channel,
						pAd->CommonCfg.CentralChannel, pAd->CommonCfg.vht_cent_ch,
						pAd->CommonCfg.BBPCurrentBW));
//---Add by shiang for debug
#endif /* DOT11_VHT_AC */

	return TRUE;
}
INT ap_mlme_set_capability(RTMP_ADAPTER *pAd, BSS_STRUCT *pMbss)
{
	struct wifi_dev *wdev = &pMbss->wdev;
	BOOLEAN SpectrumMgmt = FALSE;

#ifdef A_BAND_SUPPORT
	/* Decide the Capability information field */
	/* In IEEE Std 802.1h-2003, the spectrum management bit is enabled in the 5 GHz band */
	if ((pAd->CommonCfg.Channel > 14) && pAd->CommonCfg.bIEEE80211H == TRUE)
		SpectrumMgmt = TRUE;
#endif /* A_BAND_SUPPORT */

	pMbss->CapabilityInfo = CAP_GENERATE(1,
										0,
										(wdev->WepStatus != Ndis802_11EncryptionDisabled),
										(pAd->CommonCfg.TxPreamble == Rt802_11PreambleLong ? 0 : 1),
										pAd->CommonCfg.bUseShortSlotTime,
										SpectrumMgmt);

#ifdef DOT11K_RRM_SUPPORT
	if (pMbss->RrmCfg.bDot11kRRMEnable == TRUE)
		pMbss->CapabilityInfo |= RRM_CAP_BIT;
#endif /* DOT11K_RRM_SUPPORT */

	if (pMbss->wdev.bWmmCapable == TRUE)
	{
		/*
			In WMM spec v1.1, A WMM-only AP or STA does not set the "QoS"
			bit in the capability field of association, beacon and probe
			management frames.
		*/
/*			pMbss->CapabilityInfo |= 0x0200; */
	}

#ifdef UAPSD_SUPPORT
	if (pMbss->wdev.UapsdInfo.bAPSDCapable == TRUE)
	{
		/*
			QAPs set the APSD subfield to 1 within the Capability
			Information field when the MIB attribute
			dot11APSDOptionImplemented is true and set it to 0 otherwise.
			STAs always set this subfield to 0.
		*/
		pMbss->CapabilityInfo |= 0x0800;
	}
#endif /* UAPSD_SUPPORT */

	return TRUE;
}


INT ap_func_init(RTMP_ADAPTER *pAd)
{

#ifdef MAT_SUPPORT
	MATEngineInit(pAd);
#endif /* MAT_SUPPORT */

#ifdef CLIENT_WDS
	CliWds_ProxyTabInit(pAd);
#endif /* CLIENT_WDS */

	return TRUE;
}


static void update_edca_param(RTMP_ADAPTER *pAd)
{
    //TODO: 
    return ;
}

/*
	==========================================================================
	Description:
		Start AP service. If any vital AP parameter is changed, a STOP-START
		sequence is required to disassociate all STAs.

	IRQL = DISPATCH_LEVEL.(from SetInformationHandler)
	IRQL = PASSIVE_LEVEL. (from InitializeHandler)

	Note:
		Can't call NdisMIndicateStatus on this routine.

		RT61 is a serialized driver on Win2KXP and is a deserialized on Win9X
		Serialized callers of NdisMIndicateStatus must run at IRQL = DISPATCH_LEVEL.

	==========================================================================
 */
VOID APStartUp(RTMP_ADAPTER *pAd)
{
#if defined(INF_AMAZON_SE) || defined(RTMP_MAC_USB)
	UINT32 i;
#endif /* defined(INF_AMAZON_SE) || defined(RTMP_MAC_USB) */
	UCHAR idx;
	UCHAR phy_mode = pAd->CommonCfg.cfg_wmode;
	BOOLEAN bWmmCapable = FALSE;
	EDCA_PARM *edca_param;

	DBGPRINT(RT_DEBUG_TRACE, ("===> APStartUp\n"));

#ifdef INF_AMAZON_SE
	for (i=0;i<NUM_OF_TX_RING;i++)
		pAd->BulkOutDataSizeLimit[i]=24576;
#endif /* INF_AMAZON_SE */

	AsicDisableSync(pAd);
	for (idx = 0; idx < pAd->ApCfg.BssidNum; idx++)
	{
		BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[idx];
		struct wifi_dev *wdev = &pMbss->wdev;
		UCHAR tr_tb_idx = MAX_LEN_OF_MAC_TABLE + idx;

		pMbss->mbss_idx = idx;
#ifdef AIRPLAY_SUPPORT
		if (AIRPLAY_ON(pAd))
			pMbss->bcn_buf.bBcnSntReq = TRUE;
		else
			pMbss->bcn_buf.bBcnSntReq = FALSE;
#endif /* AIRPLAY_SUPPORT */
		pMbss->bcn_buf.bcn_state = BCN_TX_IDLE;
		
#ifdef BCN_OFFLOAD_SUPPORT
		pMbss->updateEventIsTriggered = FALSE;
#endif /* BCN_OFFLOAD_SUPPORT */

		if ((pMbss->SsidLen <= 0) || (pMbss->SsidLen > MAX_LEN_OF_SSID))
		{
			NdisMoveMemory(pMbss->Ssid, "HT_AP", 5);
			pMbss->Ssid[5] = '0' + idx;
			pMbss->SsidLen = 6;
		}

		/*
			re-copy the MAC to virtual interface to avoid these MAC = all zero,
			when re-open the ra0,
		   	i.e. ifconfig ra0 down, ifconfig ra0 up, ifconfig ra0 down, ifconfig up...
		*/
		wdev->func_idx = idx;
		wdev->tr_tb_idx = tr_tb_idx;
//+++Add by Carter for MT7603
		wdev->func_dev = (VOID *)pMbss;//FIXME: dirty code for prevent ra1 up crash.
//---Add by Carter for MT7603
		set_wdev_if_addr(pAd, wdev, OPMODE_AP);
		if (wdev->if_dev)
		{
			NdisMoveMemory(RTMP_OS_NETDEV_GET_PHYADDR(wdev->if_dev),
								wdev->if_addr, MAC_ADDR_LEN);
		}
		COPY_MAC_ADDR(wdev->bssid, wdev->if_addr);

		wdev_init(pAd, wdev, WDEV_TYPE_AP);
		if (idx == 0)
			mgmt_tb_set_mcast_entry(pAd, MCAST_WCID);

		tr_tb_set_mcast_entry(pAd, tr_tb_idx, wdev);
		COPY_MAC_ADDR(pAd->CommonCfg.Bssid, pAd->CurrentAddress);

		ap_security_init(pAd, wdev, idx);
#ifdef STA_FORCE_ROAM_SUPPORT
				DBGPRINT(RT_DEBUG_OFF, 
					("\n[Force Roam] => Force Roam Support = %d\n",pAd->en_force_roam_supp));
				DBGPRINT(RT_DEBUG_OFF, 
					("[Force Roam] => StaLowRssiThr=%d dBm low_sta_renotify=%d sec StaAgeTime=%d sec\n",pAd->sta_low_rssi, pAd->low_sta_renotify,pAd->sta_age_time));	
				DBGPRINT(RT_DEBUG_OFF, 
					("[Force Roam] => MntrAgeTime=%d sec mntr_min_pkt_count=%d mntr_min_time=%d sec mntr_avg_rssi_pkt_count=%d\n",
					pAd->mntr_age_time, pAd->mntr_min_pkt_count,pAd->mntr_min_time, pAd->mntr_avg_rssi_pkt_count));
				DBGPRINT(RT_DEBUG_OFF, 
					("[Force Roam] => AclAgeTime=%d sec AclHoldTime=%d sec\n",pAd->acl_age_time, pAd->acl_hold_time));
#endif

#ifdef MWDS
		 if (wdev->bDefaultMwdsStatus == TRUE)
			MWDSEnable(pAd, wdev->func_idx, TRUE, TRUE);
#endif

#if defined(MAP_SUPPORT) && defined(A4_CONN)
		if (IS_MAP_ENABLE(pAd))
			map_a4_init(pAd, pMbss->mbss_idx, TRUE);
#endif
#if defined(WAPP_SUPPORT)
		wapp_init(pAd, pMbss);
#endif

#ifdef WH_EZ_SETUP
			if (IS_CONF_EZ_SETUP_ENABLED(wdev))
				ez_start(wdev, TRUE);
			else 
			{		
//				ez_allocate_or_update_non_ez_band(wdev);
			}
#endif /* WH_EZ_SETUP */


		ap_mlme_set_capability(pAd, pMbss);

#ifdef WSC_V2_SUPPORT
		if (pMbss->WscControl.WscV2Info.bEnableWpsV2)
		{
			/* WPS V2 doesn't support WEP and WPA/WPAPSK-TKIP. */
			if ((wdev->WepStatus == Ndis802_11WEPEnabled) ||
				(wdev->WepStatus == Ndis802_11TKIPEnable) ||
				(pMbss->bHideSsid))
				WscOnOff(pAd, idx, TRUE);
			else
				WscOnOff(pAd, idx, FALSE);
		}
#endif /* WSC_V2_SUPPORT */

		/* If any BSS is WMM Capable, we need to config HW CRs */
		if (pMbss->wdev.bWmmCapable)
       	 	bWmmCapable = TRUE;
#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)
#if (MT7615_MT7603_COMBO_FORWARDING == 1)
	{
		if (wf_fwd_check_device_hook)
			wf_fwd_check_device_hook(wdev->if_dev, INT_MBSSID, pMbss->mbss_idx, pAd->CommonCfg.Channel, 1);
	}
#endif /* MT7615_MT7603_COMBO_FORWARDING */
#endif
#ifdef DOT11U_INTERWORKING
		pMbss->GASCtrl.b11U_enable = 1;
#endif/* DOT11U_INTERWORKING */
#if defined(MAP_SUPPORT) && defined(WAPP_SUPPORT)
		if (wdev->if_dev && pAd->net_dev)
			wapp_send_bss_state_change(pAd, wdev, WAPP_BSS_START);
#endif
	}

#ifdef DOT11_N_SUPPORT
	if (phy_mode != pAd->CommonCfg.PhyMode)
		RTMPSetPhyMode(pAd, phy_mode);

	SetCommonHT(pAd);
#endif /* DOT11_N_SUPPORT */

	if (WMODE_CAP_N(pAd->CommonCfg.PhyMode) || bWmmCapable)
	{
		/* EDCA parameters used for AP's own transmission */
		if (pAd->CommonCfg.APEdcaParm.bValid == FALSE)
			set_default_ap_edca_param(pAd);

		/* EDCA parameters to be annouced in outgoing BEACON, used by WMM STA */
		if (pAd->ApCfg.BssEdcaParm.bValid == FALSE)
			set_default_sta_edca_param(pAd);

		edca_param = &pAd->CommonCfg.APEdcaParm;
	}
	else
		edca_param = NULL;
	AsicSetEdcaParm(pAd, edca_param);


#ifdef DOT11_N_SUPPORT
	if (!WMODE_CAP_N(pAd->CommonCfg.PhyMode))
		pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth = BW_20; /* Patch UI */

    if (pAd->chipCap.hif_type == HIF_MT)
    {
		AsicSetRDG(pAd, pAd->CommonCfg.bRdg);
		AsicWtblSetRDG(pAd, pAd->CommonCfg.bRdg);

		if (pAd->CommonCfg.bRdg)
			AsicUpdateTxOP(pAd, WMM_PARAM_AC_1, 0x80);
		else
			AsicUpdateTxOP(pAd, WMM_PARAM_AC_1, 0);
    }

	AsicSetRalinkBurstMode(pAd, pAd->CommonCfg.bRalinkBurstMode);

	/*update edca depend on CommCfg*/
	update_edca_param(pAd); // Currently do nothing inside...

#ifdef PIGGYBACK_SUPPORT
	RTMPSetPiggyBack(pAd, pAd->CommonCfg.bPiggyBackCapable);
#endif /* PIGGYBACK_SUPPORT */
#endif /* DOT11_N_SUPPORT */

	AsicSetBssid(pAd, pAd->CurrentAddress, 0x0);

	ap_hw_tb_init(pAd);

#if defined(RTMP_MAC) || defined(RLT_MAC)
#ifdef FIFO_EXT_SUPPORT
	if ((pAd->chipCap.hif_type == HIF_RTMP) || (pAd->chipCap.hif_type == HIF_RLT))
		AsicFifoExtSet(pAd);
#endif /* FIFO_EXT_SUPPORT */
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */

	ap_phy_rrm_init(pAd);

	/* Clear BG-Protection flag */
	OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED);

#ifdef DOT11_N_SUPPORT
#ifdef GREENAP_SUPPORT
	if (pAd->ApCfg.bGreenAPEnable == TRUE)
	{
		RTMP_CHIP_ENABLE_AP_MIMOPS(pAd,TRUE);
		pAd->ApCfg.GreenAPLevel=GREENAP_WITHOUT_ANY_STAS_CONNECT;
	}
#endif /* GREENAP_SUPPORT */
#endif /* DOT11_N_SUPPORT */

	MlmeSetTxPreamble(pAd, (USHORT)pAd->CommonCfg.TxPreamble);
	for (idx = 0; idx < pAd->ApCfg.BssidNum; idx++)
	{
		MlmeUpdateTxRates(pAd, FALSE, idx);

#ifdef DYNAMIC_RX_RATE_ADJ
		UpdateSuppRateBitmap(pAd);
#endif /* DYNAMIC_RX_RATE_ADJ */

#ifdef DOT11_N_SUPPORT
		if (WMODE_CAP_N(pAd->CommonCfg.PhyMode))
			MlmeUpdateHtTxRates(pAd, idx);
#endif /* DOT11_N_SUPPORT */
	}

	/* Set the RadarDetect Mode as Normal, bc the APUpdateAllBeaconFram() will refer this parameter. */
	pAd->Dot11_H.RDMode = RD_NORMAL_MODE;

	/* Disable Protection first. */
	AsicUpdateProtect(pAd, 0, (ALLN_SETPROTECT|CCKSETPROTECT|OFDMSETPROTECT), TRUE, FALSE);

	APUpdateCapabilityAndErpIe(pAd);
#ifdef DOT11_N_SUPPORT
	APUpdateOperationMode(pAd);
#endif /* DOT11_N_SUPPORT */

#ifdef LED_CONTROL_SUPPORT
	RTMPSetLED(pAd, LED_LINK_UP);	
#endif /* LED_CONTROL_SUPPORT */

#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)
#if (MT7615_MT7603_COMBO_FORWARDING == 1)
	WifiFwdSet(pAd->CommonCfg.WfFwdDisabled);
#endif /* CONFIG_WIFI_PKT_FWD */
#endif /* CONFIG_WIFI_PKT_FWD */
	ap_key_tb_init(pAd);

	ApLogEvent(pAd, pAd->CurrentAddress, EVENT_RESET_ACCESS_POINT);
	pAd->Mlme.PeriodicRound = 0;
	pAd->Mlme.OneSecPeriodicRound = 0;
	pAd->MacTab.MsduLifeTime = 5; /* default 5 seconds */

	OPSTATUS_SET_FLAG(pAd, fOP_AP_STATUS_MEDIA_STATE_CONNECTED);

	RTMP_IndicateMediaState(pAd, NdisMediaStateConnected);


	/*
		NOTE!!!:
			All timer setting shall be set after following flag be cleared
				fRTMP_ADAPTER_HALT_IN_PROGRESS
	*/
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);

	RadarStateCheck(pAd);



	/* start sending BEACON out */
	APMakeAllBssBeacon(pAd);
	APUpdateAllBeaconFrame(pAd);

#ifdef DFS_SUPPORT
	if (IS_DOT11_H_RADAR_STATE(pAd, RD_SILENCE_MODE))
		NewRadarDetectionStart(pAd);
#endif /* DFS_SUPPORT */
#ifdef CARRIER_DETECTION_SUPPORT
	if (pAd->CommonCfg.CarrierDetect.Enable == TRUE)
		CarrierDetectionStart(pAd);
#endif /* CARRIER_DETECTION_SUPPORT */

	if (pAd->Dot11_H.RDMode == RD_NORMAL_MODE)
		AsicEnableBssSync(pAd, pAd->CommonCfg.BeaconPeriod);

	/* Pre-tbtt interrupt setting. */
	AsicSetPreTbtt(pAd, TRUE);

#ifdef WAPI_SUPPORT
	RTMPStartWapiRekeyTimerAction(pAd, NULL);
#endif /* WAPI_SUPPORT */

	/*
		Set group re-key timer if necessary.
		It must be processed after clear flag "fRTMP_ADAPTER_HALT_IN_PROGRESS"
	*/
	WPA_APSetGroupRekeyAction(pAd);

#ifdef WDS_SUPPORT
	/* Prepare WEP key */
	WdsPrepareWepKeyFromMainBss(pAd);

	/* Add wds key infomation to ASIC */
	AsicUpdateWdsRxWCIDTable(pAd);
#endif /* WDS_SUPPORT */

#ifdef IDS_SUPPORT
	/* Start IDS timer */
	if (pAd->ApCfg.IdsEnable)
	{
#ifdef SYSTEM_LOG_SUPPORT
		if (pAd->CommonCfg.bWirelessEvent == FALSE)
			DBGPRINT(RT_DEBUG_WARN, ("!!! WARNING !!! The WirelessEvent parameter doesn't be enabled \n"));
#endif /* SYSTEM_LOG_SUPPORT */

		RTMPIdsStart(pAd);
	}
#endif /* IDS_SUPPORT */



#ifdef DOT11R_FT_SUPPORT
	FT_Init(pAd);
#endif /* DOT11R_FT_SUPPORT */


#ifdef BAND_STEERING
	if (pAd->ApCfg.BandSteering) {
		PBND_STRG_CLI_TABLE table;
		BSS_STRUCT *pBMbss;

		BndStrg_Init(pAd);
		for (idx = 0; idx < MAX_MBSSID_NUM(pAd); idx++) {
			pBMbss = &pAd->ApCfg.MBSSID[idx];
			table = Get_BndStrgTable(pAd, idx);
			if (table) {
				/* Inform daemon interface ready */
				BndStrg_SetInfFlags(pAd, &pBMbss->wdev, table, TRUE);
			}
		}
	}
#endif /* BAND_STEERING */


#ifdef CONFIG_MAC_PCI
	RTMP_ASIC_INTERRUPT_ENABLE(pAd);
#endif

	DBGPRINT(RT_DEBUG_OFF, ("Main bssid = %02x:%02x:%02x:%02x:%02x:%02x\n",
						PRINT_MAC(pAd->ApCfg.MBSSID[BSS0].wdev.bssid)));

#ifdef SW_ATF_SUPPORT
	pAd->AtfParaSet.lastTimerCnt = 0;
	pAd->AtfParaSet.flagATF = FALSE;
	pAd->AtfParaSet.dropDelta = 2;
	pAd->AtfParaSet.flagOnce = FALSE;
	pAd->AtfParaSet.wcidTxThr = WcidTxThr;
	pAd->AtfParaSet.deq_goodNodeMaxThr = goodNodeMaxDeqThr;
	pAd->AtfParaSet.deq_goodNodeMinThr = goodNodeMinDeqThr;
	pAd->AtfParaSet.deq_badNodeMinThr = badNodeMinDeqThr;
	pAd->AtfParaSet.enq_badNodeMaxThr = badNodeMaxEnqThr;
	pAd->AtfParaSet.enq_badNodeMinThr = badNodeMinEnqThr;
	pAd->AtfParaSet.enq_badNodeCurrent = 3;
	pAd->AtfParaSet.atfFalseCCAThr = FalseCCAThr;
#endif

	DBGPRINT(RT_DEBUG_TRACE, ("<=== APStartUp\n"));
}


/*
	==========================================================================
	Description:
		disassociate all STAs and stop AP service.
	Note:
	==========================================================================
 */
VOID APStop(RTMP_ADAPTER *pAd)
{
	BOOLEAN Cancelled;
	INT idx;
	BSS_STRUCT *pMbss;
#ifdef WH_EZ_SETUP
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;	
	struct wifi_dev *wdev;
	wdev = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev;
#endif

	DBGPRINT(RT_DEBUG_TRACE, ("!!! APStop !!!\n"));

#ifdef CONFIG_MAC_PCI
	RTMP_ASIC_INTERRUPT_DISABLE(pAd);
#endif

#ifdef DFS_SUPPORT
		NewRadarDetectionStop(pAd);
#endif /* DFS_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
#ifdef CARRIER_DETECTION_SUPPORT
		if (pAd->CommonCfg.CarrierDetect.Enable == TRUE)
		{
			/* make sure CarrierDetect wont send CTS */
			CarrierDetectionStop(pAd);
		}
#endif /* CARRIER_DETECTION_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */


#ifdef WDS_SUPPORT
	WdsDown(pAd);
#endif /* WDS_SUPPORT */

#ifdef APCLI_SUPPORT
	ApCliIfDown(pAd);
#endif /* APCLI_SUPPORT */

	MacTableReset(pAd, 1);

	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);

	/* Disable pre-tbtt interrupt */
	AsicSetPreTbtt(pAd, FALSE);

	/* Disable piggyback */
	RTMPSetPiggyBack(pAd, FALSE);

   	AsicUpdateProtect(pAd, 0,  (ALLN_SETPROTECT|CCKSETPROTECT|OFDMSETPROTECT), TRUE, FALSE);

	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
	{
		AsicDisableSync(pAd);
		
#ifdef LED_CONTROL_SUPPORT
		/* Set LED */
		RTMPSetLED(pAd, LED_LINK_DOWN);
#endif /* LED_CONTROL_SUPPORT */
	}



	for (idx = 0; idx < MAX_MBSSID_NUM(pAd); idx++)
	{
		pMbss = &pAd->ApCfg.MBSSID[idx];
		if (pMbss->REKEYTimerRunning == TRUE)
		{
			RTMPCancelTimer(&pMbss->REKEYTimer, &Cancelled);
			pMbss->REKEYTimerRunning = FALSE;
		}

#ifdef MWDS
		MWDSDisable(pAd, idx, TRUE, TRUE);
#endif /* MWDS */
#if defined(MAP_SUPPORT) && defined(A4_CONN)
		if (IS_MAP_ENABLE(pAd))
			map_a4_deinit(pAd, pMbss->mbss_idx, TRUE);
#endif
#if defined(MAP_SUPPORT) && defined(WAPP_SUPPORT)
		if (pMbss->wdev.if_dev && pAd->net_dev)
			wapp_send_bss_state_change(pAd, &pMbss->wdev, WAPP_BSS_STOP);
#endif /*WAPP_SUPPORT*/
#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)
#if (MT7615_MT7603_COMBO_FORWARDING == 1)
		{
			if (wf_fwd_entry_delete_hook)
				wf_fwd_entry_delete_hook (pAd->net_dev, pMbss->wdev.if_dev, 0);

			if (wf_fwd_check_device_hook)
				wf_fwd_check_device_hook(pMbss->wdev.if_dev, INT_MBSSID, pMbss->mbss_idx, pMbss->wdev.channel, 0);
		}
#endif /* CONFIG_WIFI_PKT_FWD */
#endif
		pMbss->bcn_buf.bcn_state = BCN_TX_IDLE;
#ifdef WH_EZ_SETUP
		if(IS_CONF_EZ_SETUP_ENABLED(&pMbss->wdev))
			ez_stop(&pMbss->wdev);
#endif /* WH_EZ_SETUP */

#ifdef BAND_STEERING
		if(pAd->ApCfg.BandSteering)
		{
			PBND_STRG_CLI_TABLE table;
			table = Get_BndStrgTable(pAd, idx);
			if(table)
			{
				/* Inform daemon interface ready */
				BndStrg_SetInfFlags(pAd, &pMbss->wdev, table, FALSE);
			}
		}
#endif /* BAND_STEERING */

	}

	if (pAd->ApCfg.CMTimerRunning == TRUE)
	{
		RTMPCancelTimer(&pAd->ApCfg.CounterMeasureTimer, &Cancelled);
		pAd->ApCfg.CMTimerRunning = FALSE;
	}
	pAd->ApCfg.BANClass3Data = FALSE;

#ifdef WAPI_SUPPORT
	RTMPCancelWapiRekeyTimerAction(pAd, NULL);
#endif /* WAPI_SUPPORT */

	/* */
	/* Cancel the Timer, to make sure the timer was not queued. */
	/* */
	OPSTATUS_CLEAR_FLAG(pAd, fOP_AP_STATUS_MEDIA_STATE_CONNECTED);
	RTMP_IndicateMediaState(pAd, NdisMediaStateDisconnected);

#ifdef IDS_SUPPORT
	/* if necessary, cancel IDS timer */
	RTMPIdsStop(pAd);
#endif /* IDS_SUPPORT */

#ifdef DOT11R_FT_SUPPORT
	FT_Release(pAd);
#endif /* DOT11R_FT_SUPPORT */

#ifdef DOT11V_WNM_SUPPORT
	DMSTable_Release(pAd);
#endif /* DOT11V_WNM_SUPPORT */


}

VOID APStopBssOnly(RTMP_ADAPTER *pAd, BSS_STRUCT *pMbss)
{
	BOOLEAN Cancelled;
	struct wifi_dev *wdev_bss;
#ifdef WH_EZ_SETUP
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev;
#endif

	wdev_bss = &pMbss->wdev;

	if (!wdev_bss) {
		DBGPRINT(RT_DEBUG_ERROR, ("Error!!! wdev_map is null !!!\n"));
		return;
	}
	DBGPRINT(RT_DEBUG_TRACE, ("!!! APStop !!!\n"));

#ifdef CONFIG_MAC_PCI
	RTMP_ASIC_INTERRUPT_DISABLE(pAd);
#endif

#ifdef DFS_SUPPORT
		NewRadarDetectionStop(pAd);
#endif /* DFS_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
#ifdef CARRIER_DETECTION_SUPPORT
		if (pAd->CommonCfg.CarrierDetect.Enable == TRUE) {
			/* make sure CarrierDetect wont send CTS */
			CarrierDetectionStop(pAd);
		}
#endif /* CARRIER_DETECTION_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */


#ifdef WDS_SUPPORT
	WdsDown(pAd);
#endif /* WDS_SUPPORT */

#if defined(APCLI_SUPPORT) && defined(MAP_SUPPORT) && defined(A4_CONN)
	{
		for (idx = 0; idx < MAX_APCLI_NUM; idx++) {
			wdev_apcli = &pAd->ApCfg.ApCliTab[idx].wdev;

			/* WPS cli will disconnect and connect again */
			pWscControl = &pAd->ApCfg.ApCliTab[idx].WscControl;
			if (pWscControl->bWscTrigger == TRUE)
				continue;

			if (wdev_apcli->channel == wdev_bss->channel) {
				UINT8 enable = pAd->ApCfg.ApCliTab[idx].Enable;

				if (enable) {
					pAd->ApCfg.ApCliTab[idx].Enable = FALSE;
					ApCliIfDown(pAd);
					pAd->ApCfg.ApCliTab[idx].Enable = enable;
				}
			}
		}
	}
#endif /* APCLI_SUPPORT */

	MacTableResetWdev(pAd, wdev_bss);

	/*RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS); */

	/* Disable pre-tbtt interrupt */
	/*CMDHandler(pAd); */
	AsicSetPreTbtt(pAd, FALSE);

	/* Disable piggyback */
	RTMPSetPiggyBack(pAd, FALSE);

	AsicUpdateProtect(pAd, 0,  (ALLN_SETPROTECT|CCKSETPROTECT|OFDMSETPROTECT), TRUE, FALSE);

	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)) {
		AsicDisableSync(pAd);

#ifdef LED_CONTROL_SUPPORT
		/* Set LED */
		RTMPSetLED(pAd, LED_LINK_DOWN);
#endif /* LED_CONTROL_SUPPORT */
	}



	if (pMbss->REKEYTimerRunning == TRUE) {
		RTMPCancelTimer(&pMbss->REKEYTimer, &Cancelled);
		pMbss->REKEYTimerRunning = FALSE;
	}

#ifdef MWDS
	MWDSDisable(pAd, wdev_bss->func_idx, TRUE, TRUE);
#endif /* MWDS */
#if defined(MAP_SUPPORT) && defined(A4_CONN)
	if (IS_MAP_ENABLE(pAd))
		map_a4_deinit(pAd, pMbss->mbss_idx, TRUE);
#endif
#if defined(MAP_SUPPORT) && defined(WAPP_SUPPORT)
	if (wdev_bss->if_dev && pAd->net_dev)
		wapp_send_bss_state_change(pAd, wdev_bss, WAPP_BSS_STOP);
#endif /*WAPP_SUPPORT*/
#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)
#if (MT7615_MT7603_COMBO_FORWARDING == 1)
	{
		if (wf_fwd_entry_delete_hook)
			wf_fwd_entry_delete_hook(pAd->net_dev, pMbss->wdev.if_dev, 0);

		if (wf_fwd_check_device_hook)
			wf_fwd_check_device_hook(pMbss->wdev.if_dev, INT_MBSSID,
									pMbss->mbss_idx, pMbss->wdev.channel, 0);
	}
#endif /* CONFIG_WIFI_PKT_FWD */
#endif
	pMbss->bcn_buf.bcn_state = BCN_TX_IDLE;
#ifdef WH_EZ_SETUP
	if (IS_CONF_EZ_SETUP_ENABLED(&pMbss->wdev))
		ez_stop(&pMbss->wdev);
#endif /* WH_EZ_SETUP */

#ifdef BAND_STEERING
	if (pAd->ApCfg.BandSteering) {
		PBND_STRG_CLI_TABLE table;

		table = Get_BndStrgTable(pAd, wdev_bss->func_idx);
		if (table) {
			/* Inform daemon interface ready */
			BndStrg_SetInfFlags(pAd, &pMbss->wdev, table, FALSE);
		}
	}
#endif /* BAND_STEERING */

	if (pAd->ApCfg.CMTimerRunning == TRUE) {
		RTMPCancelTimer(&pAd->ApCfg.CounterMeasureTimer, &Cancelled);
		pAd->ApCfg.CMTimerRunning = FALSE;
		pAd->ApCfg.BANClass3Data = FALSE;
	}

#ifdef WAPI_SUPPORT
	RTMPCancelWapiRekeyTimerAction(pAd, NULL);
#endif /* WAPI_SUPPORT */

	/* */
	/* Cancel the Timer, to make sure the timer was not queued. */
	/* */
	/* OPSTATUS_CLEAR_FLAG(pAd, fOP_AP_STATUS_MEDIA_STATE_CONNECTED); */
	RTMP_IndicateMediaState(pAd, NdisMediaStateDisconnected);

#ifdef IDS_SUPPORT
	/* if necessary, cancel IDS timer */
	RTMPIdsStop(pAd);
#endif /* IDS_SUPPORT */

#ifdef DOT11R_FT_SUPPORT
	FT_Release(pAd);
#endif /* DOT11R_FT_SUPPORT */

#ifdef DOT11V_WNM_SUPPORT
	DMSTable_Release(pAd);
#endif /* DOT11V_WNM_SUPPORT */


}

/*
	==========================================================================
	Description:
		This routine is used to clean up a specified power-saving queue. It's
		used whenever a wireless client is deleted.
	==========================================================================
 */
VOID APCleanupPsQueue(RTMP_ADAPTER *pAd, QUEUE_HEADER *pQueue)
{
	PQUEUE_ENTRY pEntry;
	PNDIS_PACKET pPacket;

	DBGPRINT(RT_DEBUG_TRACE, ("%s(): (0x%08lx)...\n", __FUNCTION__, (ULONG)pQueue));

	while (pQueue->Head)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s():%u...\n", __FUNCTION__, pQueue->Number));

		pEntry = RemoveHeadQueue(pQueue);
		/*pPacket = CONTAINING_RECORD(pEntry, NDIS_PACKET, MiniportReservedEx); */
		pPacket = QUEUE_ENTRY_TO_PACKET(pEntry);
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
	}
}

#ifdef APCLI_SUPPORT
#ifdef TRAFFIC_BASED_TXOP
static VOID CheckApEntryInTraffic(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry, STA_TR_ENTRY *tr_entry)
{
	UINT32 TxTotalByteCnt = 0;
	UINT32 RxTotalByteCnt = 0;

	if((IS_ENTRY_APCLI(pEntry) || IS_ENTRY_CLIENT(pEntry))
           && (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED))
        {
		TxTotalByteCnt = pEntry->OneSecTxBytes;
		RxTotalByteCnt = pEntry->OneSecRxBytes;
  	      DBGPRINT(RT_DEBUG_INFO,("WICD%d, %dM, TxBytes:%d,  RxBytes:%d\n", pEntry->wcid, (((TxTotalByteCnt + RxTotalByteCnt) << 3) >> 20), 
		  	TxTotalByteCnt, RxTotalByteCnt));

		if ((TxTotalByteCnt == 0) || (RxTotalByteCnt == 0))
		{
		}
		else if ((((TxTotalByteCnt + RxTotalByteCnt) << 3) >> 20) > pAd->CommonCfg.ManualTxopThreshold)
		{
			//printk("%dM, %d,  %d, %d\n", (((TxTotalByteCnt + RxTotalByteCnt) << 3) >> 20), TxTotalByteCnt, RxTotalByteCnt, (TxTotalByteCnt/RxTotalByteCnt));
			if (TxTotalByteCnt > RxTotalByteCnt)
		        {
				 if ((TxTotalByteCnt/RxTotalByteCnt) >= pAd->CommonCfg.ManualTxopUpBound)
				 {
				 	if (IS_ENTRY_CLIENT(pEntry))
                        			pAd->StaTxopAbledCnt++;
                			else
                        			pAd->ApClientTxopAbledCnt++;
				 }
			}
		}
	}
}
#endif /* TRAFFIC_BASED_TXOP */
#endif /* APCLI_SUPPORT */

#ifdef MULTI_CLIENT_SUPPORT
extern	UINT RtsRetryCnt;
#endif
#ifdef STA_FORCE_ROAM_SUPPORT
void load_froam_defaults(RTMP_ADAPTER *pAd)
{
	pAd->en_force_roam_supp = FROAM_SUPP_DEF;
	pAd->sta_low_rssi = (-1) * STA_LOW_RSSI;
	pAd->low_sta_renotify = LOW_RSSI_STA_RENOTIFY_TIME;
	pAd->sta_age_time = STALIST_AGEOUT_TIME;
	pAd->mntr_age_time = MNTRLIST_AGEOUT_TIME;
	pAd->mntr_min_pkt_count = MNTR_MIN_PKT_COUNT;
	pAd->mntr_min_time = MNTR_MIN_TIME;
	pAd->mntr_avg_rssi_pkt_count = AVG_RSSI_PKT_COUNT;
	pAd->sta_good_rssi = (-1) * STA_DETECT_RSSI;
	pAd->acl_age_time = ACLLIST_AGEOUT_TIME;
	pAd->acl_hold_time = ACLLIST_HOLD_TIME;	
	DBGPRINT(RT_DEBUG_TRACE, 
		("\n[Force Roam] => Force Roam Support = %d\n",pAd->en_force_roam_supp));
	DBGPRINT(RT_DEBUG_TRACE, 
		("[Force Roam] => StaLowRssiThr=%d dBm low_sta_renotify=%d sec StaAgeTime=%d sec\n",pAd->sta_low_rssi, pAd->low_sta_renotify,pAd->sta_age_time)); 	
	DBGPRINT(RT_DEBUG_TRACE, 
		("[Force Roam] => MntrAgeTime=%d sec mntr_min_pkt_count=%d mntr_min_time=%d sec mntr_avg_rssi_pkt_count=%d\n",
		pAd->mntr_age_time, pAd->mntr_min_pkt_count,pAd->mntr_min_time, pAd->mntr_avg_rssi_pkt_count));
	DBGPRINT(RT_DEBUG_TRACE, 
		("[Force Roam] => AclAgeTime=%d sec AclHoldTime=%d sec\n",pAd->acl_age_time, pAd->acl_hold_time));
}
#define MINIMUM_POWER_VALUE		       -127
static CHAR staMaxRssi(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, RSSI_SAMPLE *pRssi)
{
	CHAR Rssi = MINIMUM_POWER_VALUE;
	UINT32	rx_stream;
	rx_stream = pAd->Antenna.field.RxPath;
	if ((rx_stream == 1) && (pRssi->AvgRssi[0] < 0))
	{
		Rssi = pRssi->AvgRssi[0];
	}
	if ((rx_stream >= 2) && (pRssi->AvgRssi[1] < 0))
	{
		Rssi = max(pRssi->AvgRssi[0], pRssi->AvgRssi[1]);
	}
	if ((rx_stream >= 3) && (pRssi->AvgRssi[2] < 0))
	{
		Rssi = max(Rssi, pRssi->AvgRssi[2]);
	}
	return Rssi;
}
static void sta_rssi_check(void *ad_obj, void *pEntry)
{
	CHAR maxRssi = MINIMUM_POWER_VALUE;
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)ad_obj;
	MAC_TABLE_ENTRY *pMacEntry = (MAC_TABLE_ENTRY *)pEntry;

	// average value of rssi for all antenna is not being considered, as it depends on whether all antenna slots as usage
	// have got antenna connected.
	// Instead Max value would be used

	maxRssi = staMaxRssi(pAd, pMacEntry->wdev, &pMacEntry->RssiSample);

	printk("STA %02x-%02x-%02x-%02x-%02x-%02x maxRssi:%d !!\n",
		pMacEntry->Addr[0],pMacEntry->Addr[1],pMacEntry->Addr[2],pMacEntry->Addr[3],pMacEntry->Addr[4],pMacEntry->Addr[5],maxRssi);

	if(pMacEntry->low_rssi_notified)	// i.e rssi improved
	{
		if(maxRssi > pAd->sta_low_rssi ){
			froam_event_sta_good_rssi event_data;
			
			printk("issue FROAM_EVT_STA_RSSI_GOOD -> for STA %02x-%02x-%02x-%02x-%02x-%02x\n",
			    pMacEntry->Addr[0],pMacEntry->Addr[1],pMacEntry->Addr[2],pMacEntry->Addr[3],pMacEntry->Addr[4],pMacEntry->Addr[5]);
			
			memset(&event_data,0,sizeof(event_data));

			event_data.hdr.event_id = FROAM_EVT_STA_RSSI_GOOD;	
			event_data.hdr.event_len = sizeof(froam_event_sta_good_rssi) - sizeof(froam_event_hdr);

			//DBGPRINT(RT_DEBUG_TRACE,("FROAM_EVT_STA_RSSI_GOOD payload len:%d datLen: %d-> \n",
			//	event_data.hdr.event_len,sizeof(event_data)));
			
			memcpy(event_data.mac,pMacEntry->Addr,MAC_ADDR_LEN);
			
			RtmpOSWrielessEventSend(
						pAd->net_dev,
						RT_WLAN_EVENT_CUSTOM,
						OID_FROAM_EVENT,
						NULL,
						(UCHAR *) &event_data,
						sizeof(event_data));
			
			pMacEntry->low_rssi_notified = FALSE;
			pMacEntry->tick_sec = 0;
			//printk("Froam event sent <- \n");
		}
		else{
			pMacEntry->tick_sec++;
			if(pMacEntry->tick_sec >= pAd->low_sta_renotify){
				pMacEntry->tick_sec = 0;
				pMacEntry->low_rssi_notified = FALSE;
			}
		}
	}
	else if((maxRssi != MINIMUM_POWER_VALUE) && (maxRssi < pAd->sta_low_rssi ))	//pAd->ApCfg.EventNotifyCfg.StaRssiDetectThreshold))
	{
		froam_event_sta_low_rssi event_data;

		DBGPRINT(RT_DEBUG_TRACE,("issue FROAM_EVT_STA_RSSI_LOW -> for STA %02x-%02x-%02x-%02x-%02x-%02x\n",
		    pMacEntry->Addr[0],pMacEntry->Addr[1],pMacEntry->Addr[2],pMacEntry->Addr[3],pMacEntry->Addr[4],pMacEntry->Addr[5]));
		
		memset(&event_data,0,sizeof(event_data));

		event_data.hdr.event_id = FROAM_EVT_STA_RSSI_LOW;	
		event_data.hdr.event_len = sizeof(froam_event_sta_low_rssi) - sizeof(froam_event_hdr);

		//event_data.channel = pMacEntry->wdev->channel;
		event_data.channel = pAd->CommonCfg.Channel;
		memcpy(event_data.mac,pMacEntry->Addr,MAC_ADDR_LEN);

		RtmpOSWrielessEventSend(
					pAd->net_dev,
					RT_WLAN_EVENT_CUSTOM,
					OID_FROAM_EVENT,
					NULL,
					(UCHAR *) &event_data,
					sizeof(event_data));

		pMacEntry->low_rssi_notified = TRUE;
		pMacEntry->tick_sec = 0;

		//DBGPRINT(RT_DEBUG_TRACE,("Froam event sent <- \n"));
	}
}
#endif

/*
	==========================================================================
	Description:
		This routine is called by APMlmePeriodicExec() every second to check if
		1. any associated client in PSM. If yes, then TX MCAST/BCAST should be
		   out in DTIM only
		2. any client being idle for too long and should be aged-out from MAC table
		3. garbage collect PSQ
	==========================================================================
*/
VOID MacTableMaintenance(RTMP_ADAPTER *pAd)
{
	int i, startWcid;
#ifdef DOT11_N_SUPPORT
	ULONG MinimumAMPDUSize = pAd->CommonCfg.DesiredHtPhy.MaxRAmpduFactor; /*Default set minimum AMPDU Size to 2, i.e. 32K */
	BOOLEAN	bRdgActive;
	BOOLEAN bRalinkBurstMode;
#ifdef MT_MAC
	UCHAR nLegacySTA = 0;	/* number of legacy stations */
	UCHAR iLegacySTA[MAX_LEN_OF_TR_TABLE];	/* wtbl index of legacy stations */
#endif /* MT_MAC */
#endif /* DOT11_N_SUPPORT */
#ifdef RTMP_MAC_PCI
	ULONG IrqFlags = 0;
#endif /* RTMP_MAC_PCI */
	UINT fAnyStationPortSecured[HW_BEACON_MAX_NUM];
 	UINT bss_index;
	MAC_TABLE *pMacTable;
#if defined(PRE_ANT_SWITCH) || defined(CFO_TRACK)
	int lastClient=0;
#endif /* defined(PRE_ANT_SWITCH) || defined(CFO_TRACK) */
	CHAR avgRssi;
	BSS_STRUCT *pMbss;
#ifdef WFA_VHT_PF
	RSSI_SAMPLE *worst_rssi = NULL;
	int worst_rssi_sta_idx = 0;
#endif /* WFA_VHT_PF */
#ifdef MT_MAC
	BOOLEAN bPreAnyStationInPsm = FALSE;
#endif /* MT_MAC */
#ifdef EDCCA_RB
	int client_rb=0;
	int client_b=0;
#endif

	NdisZeroMemory(fAnyStationPortSecured, sizeof(fAnyStationPortSecured));

	pMacTable = &pAd->MacTab;

#ifdef MT_MAC
	bPreAnyStationInPsm = pMacTable->fAnyStationInPsm;
#endif /* MT_MAC */

	pMacTable->fAnyStationInPsm = FALSE;
	pMacTable->fAnyStationBadAtheros = FALSE;
	pMacTable->fAnyTxOPForceDisable = FALSE;
	pMacTable->fAllStationAsRalink = TRUE;
#ifdef DOT11_N_SUPPORT
	pMacTable->fAnyStationNonGF = FALSE;
	pMacTable->fAnyStation20Only = FALSE;
	pMacTable->fAnyStationIsLegacy = FALSE;
	pMacTable->fAnyStationMIMOPSDynamic = FALSE;
#ifdef GREENAP_SUPPORT
	/*Support Green AP */
	pMacTable->fAnyStationIsHT=FALSE;
#endif /* GREENAP_SUPPORT */

#ifdef DOT11N_DRAFT3
	pMacTable->fAnyStaFortyIntolerant = FALSE;
#endif /* DOT11N_DRAFT3 */
	pMacTable->fAllStationGainGoodMCS = TRUE;
#endif /* DOT11_N_SUPPORT */

#ifdef WAPI_SUPPORT
	pMacTable->fAnyWapiStation = FALSE;
#endif /* WAPI_SUPPORT */

	startWcid = 1;


#ifdef SMART_CARRIER_SENSE_SUPPORT
	pAd->SCSCtrl.SCSMinRssi = 0; /* (Reset)The minimum RSSI of STA */
#endif /* SMART_CARRIER_SENSE_SUPPORT */

#ifdef APCLI_SUPPORT
#ifdef TRAFFIC_BASED_TXOP
	pAd->StaTxopAbledCnt = 0;
    pAd->ApClientTxopAbledCnt = 0;
#endif /* TRAFFIC_BASED_TXOP */
#endif /* APCLI_SUPPORT */

	for (i = startWcid; i < MAX_LEN_OF_MAC_TABLE; i++)
	{
		MAC_TABLE_ENTRY *pEntry = &pMacTable->Content[i];
		STA_TR_ENTRY *tr_entry = &pMacTable->tr_entry[i];
		BOOLEAN bDisconnectSta = FALSE;
#ifdef APCLI_SUPPORT

#ifdef TRAFFIC_BASED_TXOP	
		CheckApEntryInTraffic(pAd, pEntry, tr_entry);
#endif /* TRAFFIC_BASED_TXOP */

		pEntry->AvgTxBytes = (pEntry->AvgTxBytes == 0) ? pEntry->OneSecTxBytes : \
								((pEntry->AvgTxBytes + pEntry->OneSecTxBytes) >> 1);
		pEntry->OneSecTxBytes = 0;
		pEntry->AvgRxBytes = (pEntry->AvgRxBytes == 0) ? pEntry->OneSecRxBytes : \
								((pEntry->AvgRxBytes + pEntry->OneSecRxBytes) >> 1);
		pEntry->OneSecRxBytes = 0;

		if(IS_ENTRY_APCLI(pEntry) && (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED))
		{
#ifdef MAC_REPEATER_SUPPORT
			if (pEntry->bReptCli)
			{
				pEntry->ReptCliIdleCount++;

				if ((pEntry->bReptEthCli) && (pEntry->ReptCliIdleCount >= MAC_TABLE_AGEOUT_TIME))
				{
					MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_DISCONNECT_REQ, 0, NULL,
									(64 + (MAX_EXT_MAC_ADDR_SIZE * pEntry->func_tb_idx) + pEntry->MatchReptCliIdx));
					RTMP_MLME_HANDLER(pAd);
					//RTMPRemoveRepeaterEntry(pAd, pEntry->func_tb_idx, pEntry->MatchReptCliIdx);
					continue;
				}
			}
#endif /* MAC_REPEATER_SUPPORT */


			if ((pAd->Mlme.OneSecPeriodicRound % 10) == 8)
			{
				/* use Null or QoS Null to detect the ACTIVE station*/
				BOOLEAN ApclibQosNull = FALSE;

				if (CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE))
					ApclibQosNull = TRUE;
#ifdef WH_EZ_SETUP
				if (IS_ADPTR_EZ_SETUP_ENABLED(pAd) && (ScanRunning(pAd) == FALSE)){
#endif
			       ApCliRTMPSendNullFrame(pAd,pEntry->CurrTxRate, ApclibQosNull, pEntry, PWR_ACTIVE);
#ifdef WH_EZ_SETUP
				}
#endif

				continue;
			}
		}
#endif /* APCLI_SUPPORT */

		if (!IS_ENTRY_CLIENT(pEntry))
			continue;

#if defined(MAP_SUPPORT) && defined(WAPP_SUPPORT)
		if ((pEntry->wdev) && (IS_MAP_ENABLE(pAd))) {
			UINT32 tx_tp = (pEntry->AvgTxBytes >> BYTES_PER_SEC_TO_MBPS);
			UINT32 rx_tp = (pEntry->AvgRxBytes >> BYTES_PER_SEC_TO_MBPS);
			ULONG data_rate = 0;
			UINT32 tp_ratio = 0;
			UINT8 bidir_traffc_mode = 0;

			getRate(pEntry->HTPhyMode, &data_rate);
			tp_ratio = ((tx_tp + rx_tp) * 100) / data_rate;

			if (tp_ratio > STA_TP_IDLE_THRESHOLD)
				bidir_traffc_mode = TRAFFIC_BIDIR_ACTIVE_MODE;
			else
				bidir_traffc_mode = TRAFFIC_BIDIR_IDLE_MODE;

			if ((pEntry->pre_traffic_mode == TRAFFIC_BIDIR_ACTIVE_MODE) &&
				(tp_ratio <= STA_TP_IDLE_THRESHOLD))
				wapp_send_cli_active_change(pAd, pEntry, INACTIVE);
			else if ((pEntry->pre_traffic_mode == TRAFFIC_BIDIR_IDLE_MODE) &&
				(tp_ratio > STA_TP_IDLE_THRESHOLD))
				wapp_send_cli_active_change(pAd, pEntry, ACTIVE);

			pEntry->pre_traffic_mode = bidir_traffc_mode;
		}
#endif

#ifdef MT_PS
		CheckSkipTX(pAd, pEntry);
#endif /* MT_PS */
		if (pEntry->NoDataIdleCount == 0)
			pEntry->StationKeepAliveCount = 0;

		pEntry->NoDataIdleCount ++;
		// TODO: shiang-usw,  remove upper setting becasue we need to migrate to tr_entry!
		pAd->MacTab.tr_entry[pEntry->wcid].NoDataIdleCount = 0;

		pEntry->StaConnectTime ++;

		pMbss = &pAd->ApCfg.MBSSID[pEntry->func_tb_idx];

		/* 0. STA failed to complete association should be removed to save MAC table space. */
		if ((pEntry->Sst != SST_ASSOC) && (pEntry->NoDataIdleCount >= pEntry->AssocDeadLine))
		{
			DBGPRINT(RT_DEBUG_TRACE,
					("%02x:%02x:%02x:%02x:%02x:%02x fail to complete ASSOC in %lu sec\n",
					PRINT_MAC(pEntry->Addr), pEntry->AssocDeadLine));
#ifdef WSC_AP_SUPPORT
			if (NdisEqualMemory(pEntry->Addr, pMbss->WscControl.EntryAddr, MAC_ADDR_LEN))
				NdisZeroMemory(pMbss->WscControl.EntryAddr, MAC_ADDR_LEN);
#endif /* WSC_AP_SUPPORT */
#ifdef WH_EZ_SETUP
			if (IS_EZ_SETUP_ENABLED(pEntry->wdev))
			{
				struct _ez_peer_security_info * ez_peer = NULL;
				//BOOLEAN ez_peer = FALSE;
				ez_peer = ez_peer_table_search_by_addr(pEntry->wdev, pEntry->Addr);
				if (ez_peer)
				{
					ez_set_delete_peer_in_differed_context(pEntry->wdev, ez_peer, TRUE);
					ez_send_unicast_deauth(pAd,pEntry->Addr);
				} else {
					MacTableDeleteEntry(pAd, pEntry->wcid, pEntry->Addr);
				}
			}
			else {
#endif
				MacTableDeleteEntry(pAd, pEntry->wcid, pEntry->Addr);
#ifdef WH_EZ_SETUP
			}
#endif
			continue;
		}

		/*
			1. check if there's any associated STA in power-save mode. this affects outgoing
				MCAST/BCAST frames should be stored in PSQ till DtimCount=0
		*/
		if (pEntry->PsMode == PWR_SAVE) {
			pMacTable->fAnyStationInPsm = TRUE;
			if (pEntry->wdev && pEntry->wdev->wdev_type == WDEV_TYPE_AP) {
				pAd->MacTab.tr_entry[pEntry->wdev->tr_tb_idx].PsMode = PWR_SAVE;
				if (tr_entry->PsDeQWaitCnt)
				{
					tr_entry->PsDeQWaitCnt++;
					if (tr_entry->PsDeQWaitCnt > 2)
						tr_entry->PsDeQWaitCnt = 0;
				}
			}
		}

#ifdef DOT11_N_SUPPORT
		if (pEntry->MmpsMode == MMPS_DYNAMIC)
			pMacTable->fAnyStationMIMOPSDynamic = TRUE;

		if (pEntry->MaxHTPhyMode.field.BW == BW_20)
			pMacTable->fAnyStation20Only = TRUE;

		if (pEntry->MaxHTPhyMode.field.MODE != MODE_HTGREENFIELD)
			pMacTable->fAnyStationNonGF = TRUE;

		if ((pEntry->MaxHTPhyMode.field.MODE == MODE_OFDM) || (pEntry->MaxHTPhyMode.field.MODE == MODE_CCK))
		{
			pMacTable->fAnyStationIsLegacy = TRUE;
#ifdef MT_MAC
			iLegacySTA[nLegacySTA] = i;
			nLegacySTA ++;
#endif /* MT_MAC */
		}
#ifdef GREENAP_SUPPORT
		else
			pMacTable->fAnyStationIsHT=TRUE;
#endif /* GREENAP_SUPPORT */

#ifdef DOT11N_DRAFT3
		if (pEntry->bForty_Mhz_Intolerant)
			pMacTable->fAnyStaFortyIntolerant = TRUE;
#endif /* DOT11N_DRAFT3 */

		/* Get minimum AMPDU size from STA */
		if (MinimumAMPDUSize > pEntry->MaxRAmpduFactor)
			MinimumAMPDUSize = pEntry->MaxRAmpduFactor;
#endif /* DOT11_N_SUPPORT */

		if (pEntry->bIAmBadAtheros)
		{
			pMacTable->fAnyStationBadAtheros = TRUE;
#ifdef DOT11_N_SUPPORT
			if (pAd->CommonCfg.IOTestParm.bRTSLongProtOn == FALSE)
				AsicUpdateProtect(pAd, 8, ALLN_SETPROTECT, FALSE, pMacTable->fAnyStationNonGF);
#endif /* DOT11_N_SUPPORT */
		}

#ifdef MAC_REPEATER_SUPPORT
		{

		}
#endif /* MAC_REPEATER_SUPPORT */
		/* detect the station alive status */
		/* detect the station alive status */
#ifdef NEW_IXIA_METHOD
		if (force_connect == 1)
			pEntry->NoDataIdleCount = 0;
#endif
		if ((pMbss->StationKeepAliveTime > 0) &&
			(pEntry->NoDataIdleCount >= pMbss->StationKeepAliveTime))
		{
			/*
				If no any data success between ap and the station for
				StationKeepAliveTime, try to detect whether the station is
				still alive.

				Note: Just only keepalive station function, no disassociation
				function if too many no response.
			*/

			/*
				For example as below:

				1. Station in ACTIVE mode,

		        ......
		        sam> tx ok!
		        sam> count = 1!	 ==> 1 second after the Null Frame is acked
		        sam> count = 2!	 ==> 2 second after the Null Frame is acked
		        sam> count = 3!
		        sam> count = 4!
		        sam> count = 5!
		        sam> count = 6!
		        sam> count = 7!
		        sam> count = 8!
		        sam> count = 9!
		        sam> count = 10!
		        sam> count = 11!
		        sam> count = 12!
		        sam> count = 13!
		        sam> count = 14!
		        sam> count = 15! ==> 15 second after the Null Frame is acked
		        sam> tx ok!      ==> (KeepAlive Mechanism) send a Null Frame to
										detect the STA life status
		        sam> count = 1!  ==> 1 second after the Null Frame is acked
		        sam> count = 2!
		        sam> count = 3!
		        sam> count = 4!
		        ......

				If the station acknowledges the QoS Null Frame,
				the NoDataIdleCount will be reset to 0.


				2. Station in legacy PS mode,

				We will set TIM bit after 15 seconds, the station will send a
				PS-Poll frame and we will send a QoS Null frame to it.
				If the station acknowledges the QoS Null Frame, the
				NoDataIdleCount will be reset to 0.


				3. Station in legacy UAPSD mode,

				Currently we do not support the keep alive mechanism.
				So if your station is in UAPSD mode, the station will be
				kicked out after 300 seconds.

				Note: the rate of QoS Null frame can not be 1M of 2.4GHz or
				6M of 5GHz, or no any statistics count will occur.
			*/

			if (pEntry->StationKeepAliveCount++ == 0)
			{
					if (pEntry->PsMode == PWR_SAVE)
					{
						/* use TIM bit to detect the PS station */
						WLAN_MR_TIM_BIT_SET(pAd, pEntry->func_tb_idx, pEntry->Aid);
					}
					else
					{
						/* use Null or QoS Null to detect the ACTIVE station */
						BOOLEAN bQosNull = FALSE;

						if (CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE))
							bQosNull = TRUE;

						RtmpEnqueueNullFrame(pAd, pEntry->Addr, pEntry->CurrTxRate,
	    	                           						pEntry->Aid, pEntry->func_tb_idx, bQosNull, TRUE, 0);
					}
			}
			else
			{
				if (pEntry->StationKeepAliveCount >= pMbss->StationKeepAliveTime)
					pEntry->StationKeepAliveCount = 0;
			}
		}

		/* 2. delete those MAC entry that has been idle for a long time */
		if ((pEntry->TxSucCnt == 0) && (pEntry->NoDataIdleCount >= pEntry->StaIdleTimeout))
		{
			bDisconnectSta = TRUE;
			DBGPRINT(RT_DEBUG_WARN, ("ageout %02x:%02x:%02x:%02x:%02x:%02x after %d-sec silence\n",
					PRINT_MAC(pEntry->Addr), pEntry->StaIdleTimeout));
			ApLogEvent(pAd, pEntry->Addr, EVENT_AGED_OUT);
#ifdef WIFI_DIAG
			if (IS_ENTRY_CLIENT(pEntry))
				DiagConnError(pAd, pEntry->func_tb_idx, pEntry->Addr,
					DIAG_CONN_FRAME_LOST, REASON_AGING_TIME_OUT);
#endif
#ifdef CONN_FAIL_EVENT
			if (IS_ENTRY_CLIENT(pEntry))
				ApSendConnFailMsg(pAd,
					pAd->ApCfg.MBSSID[pEntry->func_tb_idx].Ssid,
					pAd->ApCfg.MBSSID[pEntry->func_tb_idx].SsidLen,
					pEntry->Addr,
					REASON_DEAUTH_STA_LEAVING);
#endif
		}
#ifdef FAST_DETECT_STA_OFF
		else if (pEntry->ConCounters.DisconnectFlag && Flag_fast_detect_sta_off == 1) {
			bDisconnectSta = TRUE;
			pEntry->ConCounters.DisconnectFlag = 0;
			DBGPRINT(RT_DEBUG_ERROR, ("STA-%02x:%02x:%02x:%02x:%02x:%02x is inactive!!!\n",
				PRINT_MAC(pEntry->Addr)));
#ifdef WIFI_DIAG
			if (IS_ENTRY_CLIENT(pEntry))
				DiagConnError(pAd, pEntry->func_tb_idx, pEntry->Addr,
					DIAG_CONN_FRAME_LOST, REASON_AGING_TIME_OUT);
#endif
#ifdef CONN_FAIL_EVENT
			if (IS_ENTRY_CLIENT(pEntry))
				ApSendConnFailMsg(pAd,
					pAd->ApCfg.MBSSID[pEntry->func_tb_idx].Ssid,
					pAd->ApCfg.MBSSID[pEntry->func_tb_idx].SsidLen,
					pEntry->Addr,
					REASON_DEAUTH_STA_LEAVING);
#endif
		}
#endif
		else if (pEntry->ContinueTxFailCnt >= pAd->ApCfg.EntryLifeCheck)
		{
			/*
				AP have no way to know that the PwrSaving STA is leaving or not.
				So do not disconnect for PwrSaving STA.
			*/
			if (pEntry->PsMode != PWR_SAVE)
			{
				bDisconnectSta = TRUE;
				DBGPRINT(RT_DEBUG_WARN, ("STA-%02x:%02x:%02x:%02x:%02x:%02x had left (%d %lu)\n",
					PRINT_MAC(pEntry->Addr),
					pEntry->ContinueTxFailCnt, pAd->ApCfg.EntryLifeCheck));
#ifdef WH_EVENT_NOTIFIER
				if(pEntry)
				{
					EventHdlr pEventHdlrHook = NULL;
					pEventHdlrHook = GetEventNotiferHook(WHC_DRVEVNT_STA_TIMEOUT);
					if(pEventHdlrHook && pEntry->wdev)
						pEventHdlrHook(pAd, pEntry);
				}
#endif /* WH_EVENT_NOTIFIER */
#ifdef WIFI_DIAG
				if (IS_ENTRY_CLIENT(pEntry))
					DiagConnError(pAd, pEntry->func_tb_idx, pEntry->Addr,
						DIAG_CONN_FRAME_LOST, REASON_CONTINUE_TX_FAIL);
#endif
#ifdef CONN_FAIL_EVENT
				if (IS_ENTRY_CLIENT(pEntry))
					ApSendConnFailMsg(pAd,
						pAd->ApCfg.MBSSID[pEntry->func_tb_idx].Ssid,
						pAd->ApCfg.MBSSID[pEntry->func_tb_idx].SsidLen,
						pEntry->Addr,
						REASON_DEAUTH_STA_LEAVING);
#endif
			}
		}

		if ((pMbss->RssiLowForStaKickOut != 0) &&
			  ( (avgRssi=RTMPAvgRssi(pAd, &pEntry->RssiSample)) < pMbss->RssiLowForStaKickOut))
		{
			bDisconnectSta = TRUE;
			DBGPRINT(RT_DEBUG_WARN, ("Disassoc STA %02x:%02x:%02x:%02x:%02x:%02x , RSSI Kickout Thres[%d]-[%d]\n",
					PRINT_MAC(pEntry->Addr), pMbss->RssiLowForStaKickOut,
					avgRssi));
#ifdef WIFI_DIAG
			if (IS_ENTRY_CLIENT(pEntry))
				DiagConnError(pAd, pEntry->func_tb_idx, pEntry->Addr,
					DIAG_CONN_DEAUTH, REASON_RSSI_TOO_LOW);
#endif
#ifdef CONN_FAIL_EVENT
			if (IS_ENTRY_CLIENT(pEntry))
				ApSendConnFailMsg(pAd,
					pAd->ApCfg.MBSSID[pEntry->func_tb_idx].Ssid,
					pAd->ApCfg.MBSSID[pEntry->func_tb_idx].SsidLen,
					pEntry->Addr,
					REASON_DEAUTH_STA_LEAVING);
#endif
		}

#ifdef SMART_CARRIER_SENSE_SUPPORT
		if (pAd->SCSCtrl.SCSEnable == SCS_ENABLE)
		{
			CHAR tmpRssi = RTMPMinRssi(pAd, pEntry->RssiSample.AvgRssi[0], pEntry->RssiSample.AvgRssi[1], pEntry->RssiSample.AvgRssi[2]);
			if (tmpRssi <	pAd->SCSCtrl.SCSMinRssi )
				pAd->SCSCtrl.SCSMinRssi = tmpRssi;
		}
#endif /* SMART_CARRIER_SENSE_SUPPORT */


#ifdef ALL_NET_EVENT
		{
			CHAR tmpRssi = RTMPAvgRssi(pAd, &pEntry->RssiSample);
			if (tmpRssi > -75)
			{
				wext_send_event(pEntry->wdev->if_dev,
					pEntry->Addr,
					pEntry->bssid,
					pAd->CommonCfg.Channel,
					tmpRssi,
					FBT_LINK_STRONG_NOTIFY);
			}
			else if (tmpRssi <= -75)
			{
				wext_send_event(pEntry->wdev->if_dev,
					pEntry->Addr,
					pEntry->bssid,
					pAd->CommonCfg.Channel,
					tmpRssi,
					FBT_LINK_WEAK_NOTIFY);				
			}
		}
#endif /* ALL_NET_EVENT */


		if (bDisconnectSta)
		{
#ifdef FAST_DETECT_STA_OFF
			UCHAR flush_wcid = pEntry->wcid;
#endif
			/* send wireless event - for ageout */
			RTMPSendWirelessEvent(pAd, IW_AGEOUT_EVENT_FLAG, pEntry->Addr, 0, 0);

			if (pEntry->Sst == SST_ASSOC)
			{
				PUCHAR pOutBuffer = NULL;
				NDIS_STATUS NStatus;
				ULONG FrameLen = 0;
				HEADER_802_11 DeAuthHdr;
				USHORT Reason;

				/*  send out a DISASSOC request frame */
				NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
				if (NStatus != NDIS_STATUS_SUCCESS)
				{
					DBGPRINT(RT_DEBUG_TRACE, (" MlmeAllocateMemory fail  ..\n"));
					/*NdisReleaseSpinLock(&pAd->MacTabLock); */
					continue;
				}

#ifdef FAST_DETECT_STA_OFF
				if (Flag_fast_detect_sta_off == 1)
					pEntry->detect_deauth = TRUE;
#endif

				Reason = REASON_DEAUTH_STA_LEAVING;
				DBGPRINT(RT_DEBUG_WARN, ("Send DEAUTH - Reason = %d frame  TO %x %x %x %x %x %x \n",
										Reason, PRINT_MAC(pEntry->Addr)));
				MgtMacHeaderInit(pAd, &DeAuthHdr, SUBTYPE_DEAUTH, 0, pEntry->Addr,
								pMbss->wdev.if_addr,
								pMbss->wdev.bssid);
				MakeOutgoingFrame(pOutBuffer, &FrameLen,
								sizeof(HEADER_802_11), &DeAuthHdr,
								2, &Reason,
								END_OF_ARGS);
				MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
				MlmeFreeMemory(pAd, pOutBuffer);

#ifdef MAC_REPEATER_SUPPORT
				if ((pAd->ApCfg.bMACRepeaterEn == TRUE) && IS_ENTRY_CLIENT(pEntry)
#ifdef A4_CONN
				&& (IS_ENTRY_A4(pEntry) == FALSE)
#endif /* A4_CONN */
				)
				{
					UCHAR apCliIdx, CliIdx, isLinkValid ;
					REPEATER_CLIENT_ENTRY *pReptEntry = NULL;

					pReptEntry = RTMPLookupRepeaterCliEntry(pAd, TRUE, pEntry->Addr, TRUE, &isLinkValid);
					if (pReptEntry && (pReptEntry->CliConnectState != 0))
					{
						apCliIdx = pReptEntry->MatchApCliIdx;
						CliIdx = pReptEntry->MatchLinkIdx;
						MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_DISCONNECT_REQ, 0, NULL,
										(64 + MAX_EXT_MAC_ADDR_SIZE*apCliIdx + CliIdx));
								RTMP_MLME_HANDLER(pAd);
								//RTMPRemoveRepeaterEntry(pAd, apCliIdx, CliIdx);
					}
				}
#endif /* MAC_REPEATER_SUPPORT */
			}

#ifdef WH_EZ_SETUP
			if (IS_EZ_SETUP_ENABLED(pEntry->wdev))
			{
				struct _ez_peer_security_info * ez_peer = NULL;
				ez_peer = ez_peer_table_search_by_addr(pEntry->wdev,pEntry->Addr);
				if (ez_peer)
				{
					ez_set_delete_peer_in_differed_context(pEntry->wdev, ez_peer, TRUE);
					ez_send_unicast_deauth(pAd,pEntry->Addr);	
				} else {
					MacTableDeleteEntry(pAd, pEntry->wcid, pEntry->Addr);
				}
			}
			else {
#endif
				MacTableDeleteEntry(pAd, pEntry->wcid, pEntry->Addr);
#ifdef FAST_DETECT_STA_OFF
				if (Flag_fast_detect_sta_off == 1)
					CmdFlushFrameByWlanIdx(pAd, flush_wcid);
#endif
#ifdef WH_EZ_SETUP
			}
#endif

			continue;
		}

#if defined(CONFIG_HOTSPOT_R2) || defined(CONFIG_DOT11V_WNM)
		if (pEntry->BTMDisassocCount == 1)
		{
			PUCHAR      pOutBuffer = NULL;
			NDIS_STATUS NStatus;
			ULONG       FrameLen = 0;
			HEADER_802_11 DisassocHdr;
			USHORT      Reason;

			/*  send out a DISASSOC request frame */
			NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
			if (NStatus != NDIS_STATUS_SUCCESS) 
			{
				DBGPRINT(RT_DEBUG_TRACE, (" MlmeAllocateMemory fail  ..\n"));
				/*NdisReleaseSpinLock(&pAd->MacTabLock); */
				continue;
			}

			Reason = REASON_DISASSOC_INACTIVE;
			DBGPRINT(RT_DEBUG_ERROR, ("BTM ASSOC - Send DISASSOC  Reason = %d frame  TO %x %x %x %x %x %x \n",Reason,pEntry->Addr[0],
				pEntry->Addr[1],pEntry->Addr[2],pEntry->Addr[3],pEntry->Addr[4],pEntry->Addr[5]));
			MgtMacHeaderInit(pAd, &DisassocHdr, SUBTYPE_DISASSOC, 0, pEntry->Addr, pMbss->wdev.if_addr, pMbss->wdev.bssid);
			MakeOutgoingFrame(pOutBuffer, &FrameLen, sizeof(HEADER_802_11), &DisassocHdr, 2, &Reason, END_OF_ARGS);
			MiniportMMRequest(pAd, (MGMT_USE_QUEUE_FLAG | QID_AC_BE), pOutBuffer, FrameLen);
			MlmeFreeMemory(pAd, pOutBuffer);
#ifdef WIFI_DIAG
			if (IS_ENTRY_CLIENT(pEntry))
				DiagConnError(pAd, pEntry->func_tb_idx, pEntry->Addr,
					DIAG_CONN_DEAUTH, Reason);
#endif
#ifdef CONN_FAIL_EVENT
			if (IS_ENTRY_CLIENT(pEntry))
				ApSendConnFailMsg(pAd,
					pAd->ApCfg.MBSSID[pEntry->func_tb_idx].Ssid,
					pAd->ApCfg.MBSSID[pEntry->func_tb_idx].SsidLen,
					pEntry->Addr,
					Reason);
#endif
			//JERRY
			if (!pEntry->IsKeep) {
#ifdef WH_EZ_SETUP
				if (IS_EZ_SETUP_ENABLED(pEntry->wdev))
				{
					void * ez_peer = NULL;
					ez_peer = ez_peer_table_search_by_addr(pEntry->wdev,pEntry->Addr);
					if (ez_peer)
					{
						ez_set_delete_peer_in_differed_context(pEntry->wdev, ez_peer, TRUE);
						ez_send_unicast_deauth(pAd,pEntry->Addr);
					} else {
							MacTableDeleteEntry(pAd, pEntry->wcid, pEntry->Addr);
					}
				}
				else {
#endif				
					MacTableDeleteEntry(pAd, pEntry->wcid, pEntry->Addr);
#ifdef WH_EZ_SETUP
				}
#endif
			}
			continue;
		}
		if (pEntry->BTMDisassocCount != 0)
			pEntry->BTMDisassocCount--;
#endif /* CONFIG_HOTSPOT_R2 */

		/* 3. garbage collect the ps_queue if the STA has being idle for a while */
		if ((pEntry->PsMode == PWR_SAVE) && (tr_entry->ps_state == APPS_RETRIEVE_DONE || tr_entry->ps_state == APPS_RETRIEVE_IDLE))
		{
			 if (tr_entry->enqCount > 0) 
			{
				tr_entry->PsQIdleCount++;
				if (tr_entry->PsQIdleCount > 2)
				{
					rtmp_tx_swq_exit(pAd, pEntry->wcid);
					tr_entry->PsQIdleCount = 0;
					WLAN_MR_TIM_BIT_CLEAR(pAd, pEntry->func_tb_idx, pEntry->Aid);
					DBGPRINT(RT_DEBUG_TRACE, ("%s():Clear WCID[%d] packets\n",__FUNCTION__, pEntry->wcid));
				}
			}
		}
		else
		{
			tr_entry->PsQIdleCount = 0;
		}

#ifdef UAPSD_SUPPORT
		UAPSD_QueueMaintenance(pAd, pEntry);
#endif /* UAPSD_SUPPORT */

		/* check if this STA is Ralink-chipset */
		if (!CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_RALINK_CHIPSET))
			pMacTable->fAllStationAsRalink = FALSE;

		/* Check if the port is secured */
		if (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)
			fAnyStationPortSecured[pEntry->func_tb_idx]++;
#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
		if ((pEntry->BSS2040CoexistenceMgmtSupport)
			&& (pAd->CommonCfg.Bss2040CoexistFlag & BSS_2040_COEXIST_INFO_NOTIFY)
			&& (pAd->CommonCfg.bBssCoexEnable == TRUE)
		)
		{
			SendNotifyBWActionFrame(pAd, pEntry->wcid, pEntry->func_tb_idx);
		}
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */
#ifdef WAPI_SUPPORT
		if (pEntry->WepStatus == Ndis802_11EncryptionSMS4Enabled)
			pMacTable->fAnyWapiStation = TRUE;
#endif /* WAPI_SUPPORT */

#if defined(PRE_ANT_SWITCH) || defined(CFO_TRACK)
		lastClient = i;
#endif /* defined(PRE_ANT_SWITCH) || defined(CFO_TRACK) */

		/* only apply burst when run in MCS0,1,8,9,16,17, not care about phymode */
		if ((pEntry->HTPhyMode.field.MCS != 32) &&
			((pEntry->HTPhyMode.field.MCS % 8 == 0) || (pEntry->HTPhyMode.field.MCS % 8 == 1)))
		{
			pMacTable->fAllStationGainGoodMCS = FALSE;
		}

#ifdef WFA_VHT_PF
		if (worst_rssi == NULL) {
			worst_rssi = &pEntry->RssiSample;
			worst_rssi_sta_idx = i;
		} else {
			if (worst_rssi->AvgRssi[0] > pEntry->RssiSample.AvgRssi[0]) {
				worst_rssi = &pEntry->RssiSample;
				worst_rssi_sta_idx = i;
			}
		}
#endif /* WFA_VHT_PF */
#ifdef STA_FORCE_ROAM_SUPPORT
			if (IS_ENTRY_CLIENT(pEntry) &&
				tr_entry && (!pEntry->is_peer_entry_apcli)
				&& (((PRTMP_ADAPTER)(pEntry->wdev->sys_handle))->en_force_roam_supp)
				&& (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED))
			{
				sta_rssi_check(pAd, pEntry);
			}
#endif

#ifdef WH_EVENT_NOTIFIER
        if(pAd->ApCfg.EventNotifyCfg.bStaRssiDetect)
        {
            avgRssi = RTMPAvgRssi(pAd, &pEntry->RssiSample);
            if(avgRssi < pAd->ApCfg.EventNotifyCfg.StaRssiDetectThreshold)
            {
                if(pEntry && IS_ENTRY_CLIENT(pEntry)
#ifdef A4_CONN
				&& !IS_ENTRY_A4(pEntry)
#endif /* A4_CONN */
                    && (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)
                    )
                {
                    EventHdlr pEventHdlrHook = NULL;
                    pEventHdlrHook = GetEventNotiferHook(WHC_DRVEVNT_STA_RSSI_TOO_LOW);
                    if(pEventHdlrHook && pEntry->wdev)
                        pEventHdlrHook(pAd, pEntry->wdev, pEntry->Addr, avgRssi);
                }
            }
        }
        
        if(pEntry && IS_ENTRY_CLIENT(pEntry)
#ifdef A4_CONN
		&& !IS_ENTRY_A4(pEntry)
#endif /* A4_CONN */
           && (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)
          )
       {
            EventHdlr pEventHdlrHook = NULL;
            struct EventNotifierCfg *pEventNotifierCfg = &pAd->ApCfg.EventNotifyCfg;
            if(pEventNotifierCfg->bStaStateTxDetect && (pEventNotifierCfg->StaTxPktDetectPeriod > 0))
            {
               pEventNotifierCfg->StaTxPktDetectRound++;
               if(((pEventNotifierCfg->StaTxPktDetectRound % pEventNotifierCfg->StaTxPktDetectPeriod) == 0))
               {
                   if((pEntry->tx_state.CurrentState == WHC_STA_STATE_ACTIVE) &&
                      (pEntry->tx_state.PacketCount < pEventNotifierCfg->StaStateTxThreshold))
                   {
                       pEntry->tx_state.CurrentState = WHC_STA_STATE_IDLE;
                       pEventHdlrHook = GetEventNotiferHook(WHC_DRVEVNT_STA_ACTIVITY_STATE);
                       if(pEventHdlrHook && pEntry->wdev)
                           pEventHdlrHook(pAd, pEntry, TRUE);
                   }
                   else if((pEntry->tx_state.CurrentState == WHC_STA_STATE_IDLE) &&
                           (pEntry->tx_state.PacketCount >= pEventNotifierCfg->StaStateTxThreshold))
                   {
                       
                       pEntry->tx_state.CurrentState = WHC_STA_STATE_ACTIVE;
                       pEventHdlrHook = GetEventNotiferHook(WHC_DRVEVNT_STA_ACTIVITY_STATE);
                       if(pEventHdlrHook && pEntry->wdev)
                           pEventHdlrHook(pAd, pEntry, TRUE);
                   }
                   pEventNotifierCfg->StaTxPktDetectRound = 0;
                   pEntry->tx_state.PacketCount = 0;
               }
            }

            if(pEventNotifierCfg->bStaStateRxDetect && (pEventNotifierCfg->StaRxPktDetectPeriod > 0))
            {
               pEventNotifierCfg->StaRxPktDetectRound++;
               if(((pEventNotifierCfg->StaRxPktDetectRound % pEventNotifierCfg->StaRxPktDetectPeriod) == 0))
               {
                   if((pEntry->rx_state.CurrentState == WHC_STA_STATE_ACTIVE) &&
                      (pEntry->rx_state.PacketCount < pEventNotifierCfg->StaStateRxThreshold))
                   {
                       pEntry->rx_state.CurrentState = WHC_STA_STATE_IDLE;
                       pEventHdlrHook = GetEventNotiferHook(WHC_DRVEVNT_STA_ACTIVITY_STATE);
                       if(pEventHdlrHook && pEntry->wdev)
                           pEventHdlrHook(pAd, pEntry, FALSE);
                   }
                   else if((pEntry->rx_state.CurrentState == WHC_STA_STATE_IDLE) &&
                           (pEntry->rx_state.PacketCount >= pEventNotifierCfg->StaStateRxThreshold))
                   {
                       pEntry->rx_state.CurrentState = WHC_STA_STATE_ACTIVE;
                       pEventHdlrHook = GetEventNotiferHook(WHC_DRVEVNT_STA_ACTIVITY_STATE);
                       if(pEventHdlrHook && pEntry->wdev)
                           pEventHdlrHook(pAd, pEntry, FALSE);
                   }
                   pEventNotifierCfg->StaRxPktDetectRound = 0;
                   pEntry->rx_state.PacketCount = 0;
               }
            }
       } 
#endif /* WH_EVENT_NOTIFIER */


#ifdef EDCCA_RB
		if (IS_ENTRY_CLIENT(pEntry) && (pEntry->Sst == SST_ASSOC))
		{
		    client_rb++;

		    if (pEntry->MaxHTPhyMode.field.MODE == MODE_CCK)
		        client_b++;
		}
#endif /* EDCCA_RB */

	}


#ifdef EDCCA_RB
	if ((client_rb == 1) &&
	    (client_b == 1) &&
	    (pAd->CommonCfg.PhyMode == WMODE_B))
	    RTMP_IO_WRITE32(pAd, CR_PHYMUX_11, 0xa0a10000); //0x1422C
	else
	    RTMP_IO_WRITE32(pAd, CR_PHYMUX_11, 0x00000000); //0x1422C
#endif

#ifdef MT_MAC
	/* If we check that any preview stations are in Psm and no stations are in Psm now. */
	/* AP will dequeue all buffer broadcast packets */

	if ((pAd->chipCap.hif_type == HIF_MT) && (pMacTable->fAnyStationInPsm == FALSE)) {
		UINT apidx = 0;
        for (apidx = 0; apidx<pAd->ApCfg.BssidNum; apidx++)
        {
            BSS_STRUCT *pMbss;
            UINT wcid = 0;
            STA_TR_ENTRY *tr_entry = NULL;

            pMbss = &pAd->ApCfg.MBSSID[apidx];

            wcid = pMbss->wdev.tr_tb_idx;
            tr_entry = &pAd->MacTab.tr_entry[wcid];

			if ((bPreAnyStationInPsm == TRUE) &&  (tr_entry->tx_queue[QID_AC_BE].Head != NULL)) {
					if (tr_entry->tx_queue[QID_AC_BE].Number > MAX_PACKETS_IN_MCAST_PS_QUEUE)
					RTMPDeQueuePacket(pAd, FALSE, NUM_OF_TX_RING, wcid, MAX_PACKETS_IN_MCAST_PS_QUEUE);
					else
						RTMPDeQueuePacket(pAd, FALSE, NUM_OF_TX_RING, wcid, tr_entry->tx_queue[QID_AC_BE].Number);
			}
		}
	}
#endif

#ifdef WFA_VHT_PF
	if (worst_rssi != NULL &&
		((pAd->Mlme.OneSecPeriodicRound % 10) == 5) &&
		(worst_rssi_sta_idx >= 1))
	{
		CHAR gain = 2;
		if (worst_rssi->AvgRssi[0] >= -40)
			gain = 1;
		else if (worst_rssi->AvgRssi[0] <= -50)
			gain = 2;
		rt85592_lna_gain_adjust(pAd, gain);
		DBGPRINT(RT_DEBUG_TRACE, ("%s():WorstRSSI for STA(%02x:%02x:%02x:%02x:%02x:%02x):%d,%d,%d, Set Gain as %s\n",
					__FUNCTION__,
					PRINT_MAC(pMacTable->Content[worst_rssi_sta_idx].Addr),
					worst_rssi->AvgRssi[0], worst_rssi->AvgRssi[1], worst_rssi->AvgRssi[2],
					(gain == 2 ? "Mid" : "Low")));
	}
#endif /* WFA_VHT_PF */

#ifdef PRE_ANT_SWITCH
#endif /* PRE_ANT_SWITCH */

#ifdef CFO_TRACK
#endif /* CFO_TRACK */

	/* Update the state of port per MBSS */
	for (bss_index = BSS0; bss_index < MAX_MBSSID_NUM(pAd); bss_index++)
	{
		struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[bss_index].wdev;
		if ((fAnyStationPortSecured[bss_index] > 0)
#ifdef CONFIG_FPGA_MODE
			|| (pAd->fpga_ctl.fpga_on & 0x1)
#endif /* CONFIG_FPGA_MODE */
		)
		{
			wdev->PortSecured = WPA_802_1X_PORT_SECURED;
			pAd->MacTab.tr_entry[wdev->tr_tb_idx].PortSecured = WPA_802_1X_PORT_SECURED;
		}
		else {
			wdev->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
			pAd->MacTab.tr_entry[wdev->tr_tb_idx].PortSecured = WPA_802_1X_PORT_NOT_SECURED;
		}
	}

#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
	if (pAd->CommonCfg.Bss2040CoexistFlag & BSS_2040_COEXIST_INFO_NOTIFY)
		pAd->CommonCfg.Bss2040CoexistFlag &= (~BSS_2040_COEXIST_INFO_NOTIFY);
#endif /* DOT11N_DRAFT3 */

	/* If all associated STAs are Ralink-chipset, AP shall enable RDG. */
	if (pAd->CommonCfg.bRdg && pMacTable->fAllStationAsRalink)
		bRdgActive = TRUE;
	else
		bRdgActive = FALSE;

	if (pAd->CommonCfg.bRalinkBurstMode && pMacTable->fAllStationGainGoodMCS)
		bRalinkBurstMode = TRUE;
	else
		bRalinkBurstMode = FALSE;

#ifdef GREENAP_SUPPORT
	if (WMODE_CAP_N(pAd->CommonCfg.PhyMode))
	{
		if(pAd->MacTab.fAnyStationIsHT == FALSE
			&& pAd->ApCfg.bGreenAPEnable == TRUE)
		{
			if (pAd->ApCfg.GreenAPLevel!=GREENAP_ONLY_11BG_STAS)
			{
				RTMP_CHIP_ENABLE_AP_MIMOPS(pAd,FALSE);
				pAd->ApCfg.GreenAPLevel=GREENAP_ONLY_11BG_STAS;
			}
		}
		else
		{
			if (pAd->ApCfg.GreenAPLevel!=GREENAP_11BGN_STAS)
			{
				RTMP_CHIP_DISABLE_AP_MIMOPS(pAd);
				pAd->ApCfg.GreenAPLevel=GREENAP_11BGN_STAS;
			}
		}
	}
#endif /* GREENAP_SUPPORT */

	if (pAd->MacTab.Size > 3) 
	{
#ifdef MULTI_CLIENT_SUPPORT
		AsicSetRTSTxCntLimit(pAd, TRUE, RtsRetryCnt);
#else
		AsicSetRTSTxCntLimit(pAd, TRUE, 0x7);	
#endif	
	} else {		
		AsicSetRTSTxCntLimit(pAd, TRUE, MT_RTS_RETRY);	
	}

	if (bRdgActive != RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RDG_ACTIVE))
    {
        AsicSetRDG(pAd, bRdgActive);
#ifdef MT_MAC
		if (pAd->chipCap.hif_type == HIF_MT)
        {
            AsicWtblSetRDG(pAd, bRdgActive);
        }
#endif /* MT_MAC */
    }

	if (bRalinkBurstMode != RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RALINK_BURST_MODE))
		AsicSetRalinkBurstMode(pAd, bRalinkBurstMode);

	if ((pMacTable->fAnyStationBadAtheros == FALSE) && (pAd->CommonCfg.IOTestParm.bRTSLongProtOn == TRUE))
	{
		AsicUpdateProtect(pAd, pAd->CommonCfg.AddHTInfo.AddHtInfo2.OperaionMode, ALLN_SETPROTECT, FALSE, pMacTable->fAnyStationNonGF);
	}

#ifdef MT_MAC
	/* Under EDCCA_OFF condition, for TGn 4.2.7 fail issue, 
		Tx PER rate is very high when running with Intel 6300 testbed STA,
		but low data rate has no any imprevement to get Tx PER lower.
		That should be caused by collision situation.

		Thus we enable RTS/CTS to let retrying happens on RTS frames.
		Data frame can keeps low Tx PER and can slow down the rate adaption.
		Only legacy STAs are focused because we don't expect any behavior of 11n STA is changed.
	*/
	if (nLegacySTA == 1)
	{
		AsicWtblSetRTS(pAd, iLegacySTA[0], FALSE);
	}
	else if (nLegacySTA >= 2)
	{
		UCHAR j;

		for (j = 0; j < nLegacySTA; j ++)
			AsicWtblSetRTS(pAd, iLegacySTA[j], TRUE);
	}
#endif /* MT_MAC */
	
#endif /* DOT11_N_SUPPORT */

#ifdef RTMP_MAC_PCI
	RTMP_IRQ_LOCK(&pAd->irq_lock, IrqFlags);
#endif /* RTMP_MAC_PCI */
	/*
		4.
		garbage collect pAd->MacTab.McastPsQueue if backlogged MCAST/BCAST frames
		stale in queue. Since MCAST/BCAST frames always been sent out whenever
		DtimCount==0, the only case to let them stale is surprise removal of the NIC,
		so that ASIC-based Tbcn interrupt stops and DtimCount dead.
	*/
	// TODO: shiang-usw. revise this becasue now we have per-BSS McastPsQueue!
	if (pMacTable->McastPsQueue.Head)
	{
		UINT bss_index;

		pMacTable->PsQIdleCount ++;
		if (pMacTable->PsQIdleCount > 1)
		{

			APCleanupPsQueue(pAd, &pMacTable->McastPsQueue);
			pMacTable->PsQIdleCount = 0;

			if (pAd->ApCfg.BssidNum > MAX_MBSSID_NUM(pAd))
				pAd->ApCfg.BssidNum = MAX_MBSSID_NUM(pAd);

			/* clear MCAST/BCAST backlog bit for all BSS */
			for(bss_index=BSS0; bss_index<pAd->ApCfg.BssidNum; bss_index++)
				WLAN_MR_TIM_BCMC_CLEAR(bss_index);
		}
	}
	else
		pMacTable->PsQIdleCount = 0;
#ifdef RTMP_MAC_PCI
	RTMP_IRQ_UNLOCK(&pAd->irq_lock, IrqFlags);
#endif /* RTMP_MAC_PCI */
}


UINT32 MacTableAssocStaNumGet(RTMP_ADAPTER *pAd)
{
	UINT32 num = 0;
	UINT32 i;

	for (i = 1; i < MAX_LEN_OF_MAC_TABLE; i++)
	{
		MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[i];

		if (!IS_ENTRY_CLIENT(pEntry))
			continue;

		if (pEntry->Sst == SST_ASSOC)
			num ++;
	}

	return num;
}


/*
	==========================================================================
	Description:
		Look up a STA MAC table. Return its Sst to decide if an incoming
		frame from this STA or an outgoing frame to this STA is permitted.
	Return:
	==========================================================================
*/
MAC_TABLE_ENTRY *APSsPsInquiry(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR *pAddr,
	OUT SST *Sst,
	OUT USHORT *Aid,
	OUT UCHAR *PsMode,
	OUT UCHAR *Rate)
{
	MAC_TABLE_ENTRY *pEntry = NULL;

	if (MAC_ADDR_IS_GROUP(pAddr)) /* mcast & broadcast address */
	{
		*Sst = SST_ASSOC;
		*Aid = MCAST_WCID;	/* Softap supports 1 BSSID and use WCID=0 as multicast Wcid index */
		*PsMode = PWR_ACTIVE;
		*Rate = pAd->CommonCfg.MlmeRate;
	}
	else /* unicast address */
	{
		pEntry = MacTableLookup(pAd, pAddr);
		if (pEntry)
		{
			*Sst = pEntry->Sst;
			*Aid = pEntry->Aid;
			*PsMode = pEntry->PsMode;
			if ((pEntry->AuthMode >= Ndis802_11AuthModeWPA) && (pEntry->GTKState != REKEY_ESTABLISHED))
				*Rate = pAd->CommonCfg.MlmeRate;
			else
				*Rate = pEntry->CurrTxRate;
		}
		else
		{
			*Sst = SST_NOT_AUTH;
			*Aid = MCAST_WCID;
			*PsMode = PWR_ACTIVE;
			*Rate = pAd->CommonCfg.MlmeRate;
		}
	}

	return pEntry;
}


#ifdef SYSTEM_LOG_SUPPORT
/*
	==========================================================================
	Description:
		This routine is called to log a specific event into the event table.
		The table is a QUERY-n-CLEAR array that stop at full.
	==========================================================================
 */
VOID ApLogEvent(RTMP_ADAPTER *pAd, UCHAR *pAddr, USHORT Event)
{
	if (pAd->EventTab.Num < MAX_NUM_OF_EVENT)
	{
		RT_802_11_EVENT_LOG *pLog = &pAd->EventTab.Log[pAd->EventTab.Num];
		RTMP_GetCurrentSystemTime(&pLog->SystemTime);
		COPY_MAC_ADDR(pLog->Addr, pAddr);
		pLog->Event = Event;
		DBGPRINT_RAW(RT_DEBUG_TRACE,("LOG#%ld %02x:%02x:%02x:%02x:%02x:%02x %s\n",
			pAd->EventTab.Num, pAddr[0], pAddr[1], pAddr[2],
			pAddr[3], pAddr[4], pAddr[5], pEventText[Event]));
		pAd->EventTab.Num += 1;
	}
}
#endif /* SYSTEM_LOG_SUPPORT */


#ifdef DOT11_N_SUPPORT
/*
	==========================================================================
	Description:
		Operationg mode is as defined at 802.11n for how proteciton in this BSS operates.
		Ap broadcast the operation mode at Additional HT Infroamtion Element Operating Mode fields.
		802.11n D1.0 might has bugs so this operating mode use  EWC MAC 1.24 definition first.

		Called when receiving my bssid beacon or beaconAtJoin to update protection mode.
		40MHz or 20MHz protection mode in HT 40/20 capabale BSS.
		As STA, this obeys the operation mode in ADDHT IE.
		As AP, update protection when setting ADDHT IE and after new STA joined.
	==========================================================================
*/
VOID APUpdateOperationMode(RTMP_ADAPTER *pAd)
{
	BOOLEAN bDisableBGProtect = FALSE, bNonGFExist = FALSE;

	pAd->CommonCfg.AddHTInfo.AddHtInfo2.OperaionMode = 0;
	if ((pAd->ApCfg.LastNoneHTOLBCDetectTime + (5 * OS_HZ)) > pAd->Mlme.Now32) /* non HT BSS exist within 5 sec */
	{
		pAd->CommonCfg.AddHTInfo.AddHtInfo2.OperaionMode = 1;
		bDisableBGProtect = FALSE;
		bNonGFExist = TRUE;
	}

   	/* If I am 40MHz BSS, and there exist HT-20MHz station. */
	/* Update to 2 when it's zero.  Because OperaionMode = 1 or 3 has more protection. */
	if ((pAd->CommonCfg.AddHTInfo.AddHtInfo2.OperaionMode == 0) &&
		(pAd->MacTab.fAnyStation20Only) &&
		(pAd->CommonCfg.DesiredHtPhy.ChannelWidth == 1))
	{
		pAd->CommonCfg.AddHTInfo.AddHtInfo2.OperaionMode = 2;
		bDisableBGProtect = TRUE;
	}

	if (pAd->MacTab.fAnyStationIsLegacy || pAd->MacTab.Size > 1)
	{
		pAd->CommonCfg.AddHTInfo.AddHtInfo2.OperaionMode = 3;
		bDisableBGProtect = TRUE;
	}

	if (bNonGFExist == FALSE)
		bNonGFExist = pAd->MacTab.fAnyStationNonGF;

	AsicUpdateProtect(pAd,
						pAd->CommonCfg.AddHTInfo.AddHtInfo2.OperaionMode,
						(ALLN_SETPROTECT),
						bDisableBGProtect,
						bNonGFExist);

	pAd->CommonCfg.AddHTInfo.AddHtInfo2.NonGfPresent = pAd->MacTab.fAnyStationNonGF;
}
#endif /* DOT11_N_SUPPORT */


/*
	==========================================================================
	Description:
        Check to see the exist of long preamble STA in associated list
    ==========================================================================
 */
BOOLEAN ApCheckLongPreambleSTA(RTMP_ADAPTER *pAd)
{
    UCHAR   i;

    for (i=0; i<MAX_LEN_OF_MAC_TABLE; i++)
    {
		PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[i];
		if (!IS_ENTRY_CLIENT(pEntry) || (pEntry->Sst != SST_ASSOC))
			continue;

        if (!CAP_IS_SHORT_PREAMBLE_ON(pEntry->CapabilityInfo))
        {
            return TRUE;
        }
    }

    return FALSE;
}


/*
	==========================================================================
	Description:
		Update ERP IE and CapabilityInfo based on STA association status.
		The result will be auto updated into the next outgoing BEACON in next
		TBTT interrupt service routine
	==========================================================================
 */
VOID APUpdateCapabilityAndErpIe(RTMP_ADAPTER *pAd)
{
	UCHAR  i, ErpIeContent = 0;
	BOOLEAN ShortSlotCapable = pAd->CommonCfg.bUseShortSlotTime;
	UCHAR	apidx;
	BOOLEAN	bUseBGProtection;
	BOOLEAN	LegacyBssExist;


	if (WMODE_EQUAL(pAd->CommonCfg.PhyMode, WMODE_B))
		return;

	for (i=1; i<MAX_LEN_OF_MAC_TABLE; i++)
	{
		MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[i];
		if (!IS_ENTRY_CLIENT(pEntry) || (pEntry->Sst != SST_ASSOC))
			continue;

		/* at least one 11b client associated, turn on ERP.NonERPPresent bit */
		/* almost all 11b client won't support "Short Slot" time, turn off for maximum compatibility */
		if (pEntry->MaxSupportedRate < RATE_FIRST_OFDM_RATE)
		{
			ShortSlotCapable = FALSE;
			ErpIeContent |= 0x01;
		}

		/* at least one client can't support short slot */
		if ((pEntry->CapabilityInfo & 0x0400) == 0)
			ShortSlotCapable = FALSE;
	}

	/* legacy BSS exist within 5 sec */
	if ((pAd->ApCfg.LastOLBCDetectTime + (5 * OS_HZ)) > pAd->Mlme.Now32)
		LegacyBssExist = TRUE;
	else
		LegacyBssExist = FALSE;

	/* decide ErpIR.UseProtection bit, depending on pAd->CommonCfg.UseBGProtection
		AUTO (0): UseProtection = 1 if any 11b STA associated
		ON (1): always USE protection
		OFF (2): always NOT USE protection
	*/
	if (pAd->CommonCfg.UseBGProtection == 0)
	{
		ErpIeContent = (ErpIeContent)? 0x03 : 0x00;
		/*if ((pAd->ApCfg.LastOLBCDetectTime + (5 * OS_HZ)) > pAd->Mlme.Now32) // legacy BSS exist within 5 sec */
		if (LegacyBssExist)
		{
			ErpIeContent |= 0x02;                                     /* set Use_Protection bit */
		}
	}
	else if (pAd->CommonCfg.UseBGProtection == 1)
		ErpIeContent |= 0x02;


	bUseBGProtection = (pAd->CommonCfg.UseBGProtection == 1) ||    /* always use */
						((pAd->CommonCfg.UseBGProtection == 0) && ERP_IS_USE_PROTECTION(ErpIeContent));

#ifdef A_BAND_SUPPORT
	/* always no BG protection in A-band. falsely happened when switching A/G band to a dual-band AP */
	if (pAd->CommonCfg.Channel > 14)
		bUseBGProtection = FALSE;
#endif /* A_BAND_SUPPORT */

	if (bUseBGProtection != OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED))
	{
		USHORT OperationMode = 0;
		BOOLEAN	bNonGFExist = 0;

#ifdef DOT11_N_SUPPORT
		OperationMode = pAd->CommonCfg.AddHTInfo.AddHtInfo2.OperaionMode;
		bNonGFExist = pAd->MacTab.fAnyStationNonGF;
#endif /* DOT11_N_SUPPORT */
		if (bUseBGProtection)
		{
			OPSTATUS_SET_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED);
			AsicUpdateProtect(pAd, OperationMode, (OFDMSETPROTECT), FALSE, bNonGFExist);
		}
		else
		{
			OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED);
			AsicUpdateProtect(pAd, OperationMode, (OFDMSETPROTECT), TRUE, bNonGFExist);
		}
	}

	/* Decide Barker Preamble bit of ERP IE */
	if ((pAd->CommonCfg.TxPreamble == Rt802_11PreambleLong) || (ApCheckLongPreambleSTA(pAd) == TRUE))
		pAd->ApCfg.ErpIeContent = (ErpIeContent | 0x04);
	else
		pAd->ApCfg.ErpIeContent = ErpIeContent;

#ifdef A_BAND_SUPPORT
	/* Force to use ShortSlotTime at A-band */
	if (pAd->CommonCfg.Channel > 14)
		ShortSlotCapable = TRUE;
#endif /* A_BAND_SUPPORT */

	/* deicide CapabilityInfo.ShortSlotTime bit */
    for (apidx=0; apidx<pAd->ApCfg.BssidNum; apidx++)
    {
		USHORT *pCapInfo = &(pAd->ApCfg.MBSSID[apidx].CapabilityInfo);

		/* In A-band, the ShortSlotTime bit should be ignored. */
		if (ShortSlotCapable
#ifdef A_BAND_SUPPORT
			&& (pAd->CommonCfg.Channel <= 14)
#endif /* A_BAND_SUPPORT */
			)
    		(*pCapInfo) |= 0x0400;
		else
    		(*pCapInfo) &= 0xfbff;


   		if (pAd->CommonCfg.TxPreamble == Rt802_11PreambleLong)
			(*pCapInfo) &= (~0x020);
		else
			(*pCapInfo) |= 0x020;

	}

	AsicSetSlotTime(pAd, ShortSlotCapable, pAd->CommonCfg.Channel);

}

#ifdef STA_FORCE_ROAM_SUPPORT
BOOLEAN ApCheckFroamAccessControlList(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR        pAddr,
	IN UCHAR         Apidx)
{
	BOOLEAN Result = TRUE;
	ULONG i;

	if (pAd->en_force_roam_supp) {
		Result = TRUE;
		for (i = 0; i < pAd->ApCfg.MBSSID[Apidx].FroamAccessControlList.Num; i++) {
			if (MAC_ADDR_EQUAL(pAddr, pAd->ApCfg.MBSSID[Apidx].FroamAccessControlList.Entry[i].Addr)) {
				Result = FALSE;
				break;
			}
		}
	}
	if (Result == FALSE) {
		DBGPRINT(RT_DEBUG_TRACE, ("%02x:%02x:%02x:%02x:%02x:%02x failed in ACL checking\n",
		pAddr[0], pAddr[1], pAddr[2], pAddr[3], pAddr[4], pAddr[5]));
	}
	return Result;
}
#endif

/*
	==========================================================================
	Description:
		Check if the specified STA pass the Access Control List checking.
		If fails to pass the checking, then no authentication nor association
		is allowed
	Return:
		MLME_SUCCESS - this STA passes ACL checking

	==========================================================================
*/
BOOLEAN ApCheckAccessControlList(RTMP_ADAPTER *pAd, UCHAR *pAddr, UCHAR Apidx)
{
	BOOLEAN Result = TRUE;
	ULONG i;
#ifdef ACL_BLK_COUNT_SUPPORT
		ULONG idx;
#endif
#ifdef ACL_V2_SUPPORT
	if((Result = ACL_V2_List_Check(pAd,pAddr,Apidx)) == FALSE)
	{
		goto done;
	}
#endif /* ACL_V2_SUPPORT */

#ifdef STA_FORCE_ROAM_SUPPORT
		if (pAd->en_force_roam_supp) {
			Result = TRUE;
			for (i = 0; i < pAd->ApCfg.MBSSID[Apidx].FroamAccessControlList.Num; i++) {
				if (MAC_ADDR_EQUAL(pAddr,
					pAd->ApCfg.MBSSID[Apidx].FroamAccessControlList.Entry[i].Addr)) {
					Result = FALSE;
					break;
				}
			}
		}
#endif
	if (Result == TRUE) {
		if (pAd->ApCfg.MBSSID[Apidx].AccessControlList.Policy == 0)       /* ACL is disabled */
			Result = TRUE;
		else
		{
			if (pAd->ApCfg.MBSSID[Apidx].AccessControlList.Policy == 1)   /* ACL is a positive list */
				Result = FALSE;
			else                                              /* ACL is a negative list */
				Result = TRUE;
			for (i = 0; i < pAd->ApCfg.MBSSID[Apidx].AccessControlList.Num; i++) {
				if (MAC_ADDR_EQUAL(pAddr, pAd->ApCfg.MBSSID[Apidx].AccessControlList.Entry[i].Addr)) {
					Result = !Result;
					break;
				}
			}
		}
	}
#ifdef ACL_V2_SUPPORT
done:
#endif /* ACL_V2_SUPPORT */
#ifdef ACL_BLK_COUNT_SUPPORT
	if (pAd->ApCfg.MBSSID[Apidx].AccessControlList.Policy != 2) {
		for (i = 0; i < pAd->ApCfg.MBSSID[Apidx].AccessControlList.Num; i++)
			(pAd->ApCfg.MBSSID[Apidx].AccessControlList.Entry[i].Reject_Count) = 0;
	}
#endif
    if (FALSE == Result)
    {
#ifdef ACL_BLK_COUNT_SUPPORT

		if (pAd->ApCfg.MBSSID[Apidx].AccessControlList.Policy == 2) {
			for (idx = 0; idx < pAd->ApCfg.MBSSID[Apidx].AccessControlList.Num; idx++) {
				if (MAC_ADDR_EQUAL(pAddr, pAd->ApCfg.MBSSID[Apidx].AccessControlList.Entry[idx].Addr)) {
					(pAd->ApCfg.MBSSID[Apidx].AccessControlList.Entry[idx].Reject_Count) += 1;
					break;
				}
			}
		}
#endif
		DBGPRINT(RT_DEBUG_TRACE, ("%02x:%02x:%02x:%02x:%02x:%02x failed in ACL checking\n",
				PRINT_MAC(pAddr)));
	}

    return Result;
}


/*
	==========================================================================
	Description:
		This routine update the current MAC table based on the current ACL.
		If ACL change causing an associated STA become un-authorized. This STA
		will be kicked out immediately.
	==========================================================================
*/
VOID ApUpdateAccessControlList(RTMP_ADAPTER *pAd, UCHAR Apidx)
{
	USHORT   AclIdx, MacIdx;
	BOOLEAN  Matched;
	PUCHAR      pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	ULONG       FrameLen = 0;
	HEADER_802_11 DisassocHdr;
	USHORT      Reason;
	MAC_TABLE_ENTRY *pEntry;
	BSS_STRUCT *pMbss;
	BOOLEAN drop;

	ASSERT(Apidx < MAX_MBSSID_NUM(pAd));
	if (Apidx >= MAX_MBSSID_NUM(pAd))
		return;
	DBGPRINT(RT_DEBUG_TRACE, ("ApUpdateAccessControlList : Apidx = %d\n", Apidx));

	/* ACL is disabled. Do nothing about the MAC table. */
	pMbss = &pAd->ApCfg.MBSSID[Apidx];
#ifdef STA_FORCE_ROAM_SUPPORT
	if (pMbss->AccessControlList.Policy == 0) {
		if (pAd->en_force_roam_supp == 0)
			return;
	}
#else
	if (pMbss->AccessControlList.Policy == 0)
		return;
#endif

	for (MacIdx=0; MacIdx < MAX_LEN_OF_MAC_TABLE; MacIdx++)
	{
		pEntry = &pAd->MacTab.Content[MacIdx];
		if (!IS_ENTRY_CLIENT(pEntry))
			continue;

		/* We only need to update associations related to ACL of MBSSID[Apidx]. */
		if (pEntry->func_tb_idx != Apidx)
			continue;

		drop = FALSE;
		Matched = FALSE;
#ifdef STA_FORCE_ROAM_SUPPORT
		if (pAd->en_force_roam_supp) {
			for (AclIdx = 0; AclIdx < pMbss->FroamAccessControlList.Num; AclIdx++) {
				if (MAC_ADDR_EQUAL(&pEntry->Addr[0],
					pMbss->FroamAccessControlList.Entry[AclIdx].Addr)) {
					Matched = TRUE;
					drop = TRUE;
					DBGPRINT(RT_DEBUG_TRACE, ("STA on negative ACL. remove it...\n"));
					break;
				}
			}
		}
		if (drop == FALSE)
#endif
		{
			Matched = FALSE;
			for (AclIdx = 0; AclIdx < pMbss->AccessControlList.Num; AclIdx++) {
				if (MAC_ADDR_EQUAL(&pEntry->Addr[0], pMbss->AccessControlList.Entry[AclIdx].Addr)) {
					Matched = TRUE;
					break;
				}
			}
			if ((Matched == FALSE) && (pMbss->AccessControlList.Policy == 1)) {
				drop = TRUE;
				DBGPRINT(RT_DEBUG_TRACE, ("STA not on positive ACL. remove it...\n"));
			} else if ((Matched == TRUE) && (pMbss->AccessControlList.Policy == 2)) {
				drop = TRUE;
				DBGPRINT(RT_DEBUG_TRACE, ("STA on negative ACL. remove it...\n"));
			}
		}
		if (drop == TRUE) {
			DBGPRINT(RT_DEBUG_TRACE, ("Apidx = %d\n", Apidx));
			DBGPRINT(RT_DEBUG_TRACE, ("pAd->ApCfg.MBSSID[%d].AccessControlList.Policy = %ld\n", Apidx,
				pMbss->AccessControlList.Policy));

			/* Before delete the entry from MacTable, send disassociation packet to client. */
			if (pEntry->Sst == SST_ASSOC)
			{
				/* send out a DISASSOC frame */
				NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
				if (NStatus != NDIS_STATUS_SUCCESS)
				{
					DBGPRINT(RT_DEBUG_TRACE, (" MlmeAllocateMemory fail  ..\n"));
					return;
				}

				Reason = REASON_DECLINED;
				DBGPRINT(RT_DEBUG_ERROR, ("ASSOC - Send DISASSOC  Reason = %d frame  TO %x %x %x %x %x %x \n",
							Reason, PRINT_MAC(pEntry->Addr)));
				MgtMacHeaderInit(pAd, &DisassocHdr, SUBTYPE_DISASSOC, 0,
									pEntry->Addr,
									pMbss->wdev.if_addr,
									pMbss->wdev.bssid);
				MakeOutgoingFrame(pOutBuffer, &FrameLen, sizeof(HEADER_802_11), &DisassocHdr, 2, &Reason, END_OF_ARGS);
				MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
				MlmeFreeMemory(pAd, pOutBuffer);

				RtmpusecDelay(5000);
			}
#ifdef WIFI_DIAG
			if (IS_ENTRY_CLIENT(pEntry))
				DiagConnError(pAd, pEntry->func_tb_idx, pEntry->Addr,
					DIAG_CONN_ACL_BLK, 0);
#endif
#ifdef CONN_FAIL_EVENT
			if (IS_ENTRY_CLIENT(pEntry))
				ApSendConnFailMsg(pAd,
					pAd->ApCfg.MBSSID[pEntry->func_tb_idx].Ssid,
					pAd->ApCfg.MBSSID[pEntry->func_tb_idx].SsidLen,
					pEntry->Addr,
					REASON_DECLINED);
#endif
			MacTableDeleteEntry(pAd, pEntry->wcid, pEntry->Addr);
		}
	}
}


#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
/*
	Depends on the 802.11n Draft 4.0, Before the HT AP start a BSS, it should scan some specific channels to
collect information of existing BSSs, then depens on the collected channel information, adjust the primary channel
and secondary channel setting.

	For 5GHz,
		Rule 1: If the AP chooses to start a 20/40 MHz BSS in 5GHz and that occupies the same two channels
				as any existing 20/40 MHz BSSs, then the AP shall ensure that the primary channel of the
				new BSS is identical to the primary channel of the existing 20/40 MHz BSSs and that the
				secondary channel of the new 20/40 MHz BSS is identical to the secondary channel of the
				existing 20/40 MHz BSSs, unless the AP discoverr that on those two channels are existing
				20/40 MHz BSSs with different primary and secondary channels.
		Rule 2: If the AP chooses to start a 20/40MHz BSS in 5GHz, the selected secondary channel should
				correspond to a channel on which no beacons are detected during the overlapping BSS
				scan time performed by the AP, unless there are beacons detected on both the selected
				primary and secondary channels.
		Rule 3: An HT AP should not start a 20 MHz BSS in 5GHz on a channel that is the secondary channel
				of a 20/40 MHz BSS.
	For 2.4GHz,
		Rule 1: The AP shall not start a 20/40 MHz BSS in 2.4GHz if the value of the local variable "20/40
				Operation Permitted" is FALSE.

		20/40OperationPermitted =  (P == OPi for all values of i) AND
								(P == OTi for all values of i) AND
								(S == OSi for all values if i)
		where
			P 	is the operating or intended primary channel of the 20/40 MHz BSS
			S	is the operating or intended secondary channel of the 20/40 MHz BSS
			OPi  is member i of the set of channels that are members of the channel set C and that are the
				primary operating channel of at least one 20/40 MHz BSS that is detected within the AP's
				BSA during the previous X seconds
			OSi  is member i of the set of channels that are members of the channel set C and that are the
				secondary operating channel of at least one 20/40 MHz BSS that is detected within AP's
				BSA during the previous X seconds
			OTi  is member i of the set of channels that comparises all channels that are members of the
				channel set C that were listed once in the Channel List fields of 20/40 BSS Intolerant Channel
				Report elements receved during the previous X seconds and all channels that are members
				of the channel set C and that are the primary operating channel of at least one 20/40 MHz
				BSS that were detected within the AP's BSA during the previous X seconds.
			C	is the set of all channels that are allowed operating channels within the current operational
				regulatory domain and whose center frequency falls within the 40 MHz affected channel
				range given by following equation:
					                                                 Fp + Fs                  Fp + Fs
					40MHz affected channel range = [ ------  - 25MHz,  ------- + 25MHz ]
					                                                      2                          2
					Where
						Fp = the center frequency of channel P
						Fs = the center frequency of channel S

			"==" means that the values on either side of the "==" are to be tested for equaliy with a resulting
				 Boolean value.
			        =>When the value of OPi is the empty set, then the expression (P == OPi for all values of i)
			        	is defined to be TRUE
			        =>When the value of OTi is the empty set, then the expression (P == OTi for all values of i)
			        	is defined to be TRUE
			        =>When the value of OSi is the empty set, then the expression (S == OSi for all values of i)
			        	is defined to be TRUE
*/
INT GetBssCoexEffectedChRange(
	IN RTMP_ADAPTER *pAd,
	IN BSS_COEX_CH_RANGE *pCoexChRange)
{
	INT index, cntrCh = 0;

	memset(pCoexChRange, 0, sizeof(BSS_COEX_CH_RANGE));

	/* Build the effected channel list, if something wrong, return directly. */
#ifdef A_BAND_SUPPORT
	if (pAd->CommonCfg.Channel > 14)
	{	/* For 5GHz band */
		for (index = 0; index < pAd->ChannelListNum; index++)
		{
			if(pAd->ChannelList[index].Channel == pAd->CommonCfg.Channel)
				break;
		}

		if (index < pAd->ChannelListNum)
		{
			/* First get the primary channel */
			pCoexChRange->primaryCh = pAd->ChannelList[index].Channel;

			/* Now check about the secondary and central channel */
			if(pAd->CommonCfg.RegTransmitSetting.field.EXTCHA == EXTCHA_ABOVE)
			{
				pCoexChRange->effectChStart = pCoexChRange->primaryCh;
				pCoexChRange->effectChEnd = pCoexChRange->primaryCh + 4;
				pCoexChRange->secondaryCh = pCoexChRange->effectChEnd;
			}
			else
			{
				pCoexChRange->effectChStart = pCoexChRange->primaryCh -4;
				pCoexChRange->effectChEnd = pCoexChRange->primaryCh;
				pCoexChRange->secondaryCh = pCoexChRange->effectChStart;
			}

			DBGPRINT(RT_DEBUG_TRACE,("5.0GHz: Found CtrlCh idx(%d) from the ChList, ExtCh=%s, PriCh=[Idx:%d, CH:%d], SecCh=[Idx:%d, CH:%d], effected Ch=[CH:%d~CH:%d]!\n",
										index,
										((pAd->CommonCfg.RegTransmitSetting.field.EXTCHA == EXTCHA_ABOVE) ? "ABOVE" : "BELOW"),
										pCoexChRange->primaryCh, pAd->ChannelList[pCoexChRange->primaryCh].Channel,
										pCoexChRange->secondaryCh, pAd->ChannelList[pCoexChRange->secondaryCh].Channel,
										pAd->ChannelList[pCoexChRange->effectChStart].Channel,
										pAd->ChannelList[pCoexChRange->effectChEnd].Channel));
			return TRUE;
		}
		else
		{
			/* It should not happened! */
			DBGPRINT(RT_DEBUG_ERROR, ("5GHz: Cannot found the CtrlCh(%d) in ChList, something wrong?\n",
						pAd->CommonCfg.Channel));
		}
	}
	else
#endif /* A_BAND_SUPPORT */
	{	/* For 2.4GHz band */
		for (index = 0; index < pAd->ChannelListNum; index++)
		{
			if(pAd->ChannelList[index].Channel == pAd->CommonCfg.Channel)
				break;
		}

		if (index < pAd->ChannelListNum)
		{
			/* First get the primary channel */
			pCoexChRange->primaryCh = index;

			/* Now check about the secondary and central channel */
			if(pAd->CommonCfg.RegTransmitSetting.field.EXTCHA == EXTCHA_ABOVE)
			{
				if ((index + 4) < pAd->ChannelListNum)
				{
					cntrCh = index + 2;
					pCoexChRange->secondaryCh = index + 4;
				}
			}
			else
			{
				if ((index - 4) >=0)
				{
					cntrCh = index - 2;
					pCoexChRange->secondaryCh = index - 4;
				}
			}

			if (cntrCh)
			{
				pCoexChRange->effectChStart = (cntrCh - 5) > 0 ? (cntrCh - 5) : 0;
				pCoexChRange->effectChEnd= cntrCh + 5;
				DBGPRINT(RT_DEBUG_TRACE,("2.4GHz: Found CtrlCh idx(%d) from the ChList, ExtCh=%s, PriCh=[Idx:%d, CH:%d], SecCh=[Idx:%d, CH:%d], effected Ch=[CH:%d~CH:%d]!\n",
										index,
										((pAd->CommonCfg.RegTransmitSetting.field.EXTCHA == EXTCHA_ABOVE) ? "ABOVE" : "BELOW"),
										pCoexChRange->primaryCh, pAd->ChannelList[pCoexChRange->primaryCh].Channel,
										pCoexChRange->secondaryCh, pAd->ChannelList[pCoexChRange->secondaryCh].Channel,
										pAd->ChannelList[pCoexChRange->effectChStart].Channel,
										pAd->ChannelList[pCoexChRange->effectChEnd].Channel));
			}
			return TRUE;
		}

		/* It should not happened! */
		DBGPRINT(RT_DEBUG_ERROR, ("2.4GHz: Didn't found valid channel range, Ch index=%d, ChListNum=%d, CtrlCh=%d\n",
									index, pAd->ChannelListNum, pAd->CommonCfg.Channel));
	}

	return FALSE;
}


VOID APOverlappingBSSScan(RTMP_ADAPTER *pAd)
{
	BOOLEAN needFallBack = FALSE;
	UCHAR Channel = pAd->CommonCfg.Channel;
	INT chStartIdx, chEndIdx, index,curPriChIdx, curSecChIdx;
#ifdef WH_EZ_SETUP
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;	
	struct wifi_dev *wdev;
	wdev = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev;
#endif

	/* We just care BSS who operating in 40MHz N Mode. */
	if ((!WMODE_CAP_N(pAd->CommonCfg.PhyMode)) || 
		(pAd->CommonCfg.Channel > 14) ||
		(pAd->CommonCfg.ori_bw_before_2040_coex == BW_20))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("The pAd->PhyMode=%d, BW=%d, didn't need channel adjustment!\n",
				pAd->CommonCfg.PhyMode, pAd->CommonCfg.RegTransmitSetting.field.BW));
		return;
	}

	/* Build the effected channel list, if something wrong, return directly. */
#ifdef A_BAND_SUPPORT
	if (pAd->CommonCfg.Channel > 14)
	{	/* For 5GHz band */
		for (index = 0; index < pAd->ChannelListNum; index++)
		{
			if(pAd->ChannelList[index].Channel == pAd->CommonCfg.Channel)
				break;
		}

		if (index < pAd->ChannelListNum)
		{
			curPriChIdx = index;
			if(pAd->CommonCfg.RegTransmitSetting.field.EXTCHA == EXTCHA_ABOVE)
			{
				chStartIdx = index;
				chEndIdx = chStartIdx + 1;
				curSecChIdx = chEndIdx;
			}
			else
			{
				chStartIdx = index - 1;
				chEndIdx = index;
				curSecChIdx = chStartIdx;
			}
		}
		else
		{
			/* It should not happened! */
			DBGPRINT(RT_DEBUG_ERROR, ("5GHz: Cannot found the ControlChannel(%d) in ChannelList, something wrong?\n",
						pAd->CommonCfg.Channel));
			return;
		}
	}
	else
#endif /* A_BAND_SUPPORT */
	{	/* For 2.4GHz band */
		for (index = 0; index < pAd->ChannelListNum; index++)
		{
			if(pAd->ChannelList[index].Channel == pAd->CommonCfg.Channel)
				break;
		}

		if (index < pAd->ChannelListNum)
		{

			if(pAd->CommonCfg.RegTransmitSetting.field.EXTCHA == EXTCHA_ABOVE)
			{
				curPriChIdx = index;
				curSecChIdx = ((index + 4) < pAd->ChannelListNum) ? (index + 4) : (pAd->ChannelListNum - 1);

				chStartIdx = (curPriChIdx >= 3) ? (curPriChIdx - 3) : 0;
				chEndIdx = ((curSecChIdx + 3) < pAd->ChannelListNum) ? (curSecChIdx + 3) : (pAd->ChannelListNum - 1);
			}
			else
			{
				curPriChIdx = index;
				curSecChIdx = ((index - 4) >=0 ) ? (index - 4) : 0;
				chStartIdx =(curSecChIdx >= 3) ? (curSecChIdx - 3) : 0;
				chEndIdx =  ((curPriChIdx + 3) < pAd->ChannelListNum) ? (curPriChIdx + 3) : (pAd->ChannelListNum - 1);;
			}
		}
		else
		{
			/* It should not happened! */
			DBGPRINT(RT_DEBUG_ERROR, ("2.4GHz: Cannot found the Control Channel(%d) in ChannelList, something wrong?\n",
						pAd->CommonCfg.Channel));
			return;
		}
	}

{
	BSS_COEX_CH_RANGE  coexChRange;
	GetBssCoexEffectedChRange(pAd, &coexChRange);
}

	/* Before we do the scanning, clear the bEffectedChannel as zero for latter use. */
	for (index = 0; index < pAd->ChannelListNum; index++)
		pAd->ChannelList[index].bEffectedChannel = 0;

	pAd->CommonCfg.BssCoexApCnt = 0;

	/* If we are not ready for Tx/Rx Pakcet, enable it now for receiving Beacons. */
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP) == 0)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("Card still not enable Tx/Rx, enable it now!\n"));
#ifdef RTMP_MAC_PCI
		/* Enable Interrupt */
		rtmp_irq_init(pAd);

		RTMP_IRQ_ENABLE(pAd);
#endif /* RTMP_MAC_PCI */


		/* rtmp_rx_done_handle() API will check this flag to decide accept incoming packet or not. */
		/* Set the flag be ready to receive Beacon frame for autochannel select. */
		RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_START_UP);
	}

	RTMPEnableRxTx(pAd);

	DBGPRINT(RT_DEBUG_TRACE, ("Ready to do passive scanning for Channel[%d] to Channel[%d]!\n",
			pAd->ChannelList[chStartIdx].Channel, pAd->ChannelList[chEndIdx].Channel));

	/* Now start to do the passive scanning. */
	pAd->CommonCfg.bOverlapScanning = TRUE;
	for (index = chStartIdx; index<=chEndIdx; index++)
	{
		Channel = pAd->ChannelList[index].Channel;
		AsicSetChannel(pAd, Channel, BW_20,  EXTCHA_NONE, TRUE);

		DBGPRINT(RT_DEBUG_ERROR, ("SYNC - BBP R4 to 20MHz.l\n"));
		/*DBGPRINT(RT_DEBUG_TRACE, ("Passive scanning for Channel %d.....\n", Channel)); */
		OS_WAIT(300); /* wait for 200 ms at each channel. */
	}
	pAd->CommonCfg.bOverlapScanning = FALSE;

	/* After scan all relate channels, now check the scan result to find out if we need fallback to 20MHz. */
	for (index = chStartIdx; index <= chEndIdx; index++)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("Channel[Idx=%d, Ch=%d].bEffectedChannel=0x%x!\n",
					index, pAd->ChannelList[index].Channel, pAd->ChannelList[index].bEffectedChannel));
		if ((pAd->ChannelList[index].bEffectedChannel & (EFFECTED_CH_PRIMARY | EFFECTED_CH_LEGACY))  && (index != curPriChIdx) )
		{
			needFallBack = TRUE;
			DBGPRINT(RT_DEBUG_TRACE, ("needFallBack=TRUE due to OP/OT!\n"));
		}
		if ((pAd->ChannelList[index].bEffectedChannel & EFFECTED_CH_SECONDARY)  && (index != curSecChIdx))
		{
			needFallBack = TRUE;
			DBGPRINT(RT_DEBUG_TRACE, ("needFallBack=TRUE due to OS!\n"));
		}
	}

	/* If need fallback, now do it. */
	if ((needFallBack == TRUE)
		&& (pAd->CommonCfg.BssCoexApCnt > pAd->CommonCfg.BssCoexApCntThr)
	)
	{
#if (defined(WH_EZ_SETUP) && defined(EZ_NETWORK_MERGE_SUPPORT))
		if(IS_EZ_SETUP_ENABLED(wdev)){
			EZ_DEBUG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("\nAPOverlappingBSSScan: Fallback at bootup ****\n"));
			ez_set_ap_fallback_context(wdev,TRUE,pAd->CommonCfg.Channel);
		}
#endif /* WH_EZ_SETUP */
		pAd->CommonCfg.AddHTInfo.AddHtInfo.RecomWidth = 0;
		pAd->CommonCfg.AddHTInfo.AddHtInfo.ExtChanOffset = 0;
		pAd->CommonCfg.LastBSSCoexist2040.field.BSS20WidthReq = 1;
		pAd->CommonCfg.Bss2040CoexistFlag |= BSS_2040_COEXIST_INFO_SYNC;
#ifdef NEW_BW2040_COEXIST_SUPPORT
		pAd->CommonCfg.RegTransmitSetting.field.BW = 0;/* radio fallback */
#endif /* NEW_BW2040_COEXIST_SUPPORT */
		pAd->CommonCfg.Bss2040NeedFallBack = 1;
		pAd->CommonCfg.RegTransmitSetting.field.EXTCHA = 0;
	}

	/* Recover the bandwidth to support 20/40Mhz if the original setting does support that, and no need to fallback */
	if ((needFallBack == FALSE)
		&& (pAd->CommonCfg.ori_bw_before_2040_coex == BW_40)) {
		pAd->CommonCfg.AddHTInfo.AddHtInfo.RecomWidth = pAd->CommonCfg.ori_bw_before_2040_coex;
		pAd->CommonCfg.AddHTInfo.AddHtInfo.ExtChanOffset = pAd->CommonCfg.ori_ext_channel_before_2040_coex;
		pAd->CommonCfg.LastBSSCoexist2040.field.BSS20WidthReq = 0;
		pAd->CommonCfg.Bss2040CoexistFlag &= (~BSS_2040_COEXIST_INFO_SYNC);
		pAd->CommonCfg.Bss2040NeedFallBack = 0;
		pAd->CommonCfg.RegTransmitSetting.field.EXTCHA = pAd->CommonCfg.ori_ext_channel_before_2040_coex;
		DBGPRINT(RT_DEBUG_ERROR, ("rollback the bandwidth setting to support 20/40Mhz\n"));
	}

	return;
}
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */


#ifdef DOT1X_SUPPORT
/*
 ========================================================================
 Routine Description:
    Send Leyer 2 Frame to notify 802.1x daemon. This is a internal command

 Arguments:

 Return Value:
    TRUE - send successfully
    FAIL - send fail

 Note:
 ========================================================================
*/
BOOLEAN DOT1X_InternalCmdAction(
    IN  PRTMP_ADAPTER	pAd,
    IN  MAC_TABLE_ENTRY *pEntry,
    IN UINT8 cmd)
{
	// TODO: shiang-usw, fix me for pEntry->apidx to func_tb_idx
	INT				apidx = MAIN_MBSSID;
	UCHAR 			RalinkIe[9] = {221, 7, 0x00, 0x0c, 0x43, 0x00, 0x00, 0x00, 0x00};
	UCHAR			s_addr[MAC_ADDR_LEN];
	UCHAR			EAPOL_IE[] = {0x88, 0x8e};
	UINT8			frame_len = LENGTH_802_3 + sizeof(RalinkIe);
	UCHAR			FrameBuf[frame_len];
	UINT8			offset = 0;

	/* Init the frame buffer */
	NdisZeroMemory(FrameBuf, frame_len);

	if (pEntry)
	{
		apidx = pEntry->func_tb_idx;
		NdisMoveMemory(s_addr, pEntry->Addr, MAC_ADDR_LEN);
	}
	else
	{
		/* Fake a Source Address for transmission */
		NdisMoveMemory(s_addr, pAd->ApCfg.MBSSID[apidx].wdev.bssid, MAC_ADDR_LEN);
		s_addr[0] |= 0x80;
	}

	/* Assign internal command for Ralink dot1x daemon */
	RalinkIe[5] = cmd;

	/* Prepare the 802.3 header */
	MAKE_802_3_HEADER(FrameBuf,
					  pAd->ApCfg.MBSSID[apidx].wdev.bssid,
					  s_addr,
					  EAPOL_IE);
	offset += LENGTH_802_3;

	/* Prepare the specific header of internal command */
	NdisMoveMemory(&FrameBuf[offset], RalinkIe, sizeof(RalinkIe));

	/* Report to upper layer */
	if (RTMP_L2_FRAME_TX_ACTION(pAd, apidx, FrameBuf, frame_len) == FALSE)
		return FALSE;

	DBGPRINT(RT_DEBUG_TRACE, ("%s done. (cmd=%d)\n", __FUNCTION__, cmd));

	return TRUE;
}


/*
 ========================================================================
 Routine Description:
    Send Leyer 2 Frame to trigger 802.1x EAP state machine.

 Arguments:

 Return Value:
    TRUE - send successfully
    FAIL - send fail

 Note:
 ========================================================================
*/
BOOLEAN DOT1X_EapTriggerAction(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry)
{
	// TODO: shiang-usw, fix me for pEntry->apidx to func_tb_idx
	INT				apidx = MAIN_MBSSID;
	UCHAR 			eapol_start_1x_hdr[4] = {0x01, 0x01, 0x00, 0x00};
	UINT8			frame_len = LENGTH_802_3 + sizeof(eapol_start_1x_hdr);
	UCHAR			FrameBuf[frame_len+32];
	UINT8			offset = 0;

    if((pEntry->AuthMode == Ndis802_11AuthModeWPA) || (pEntry->AuthMode == Ndis802_11AuthModeWPA2) || (pAd->ApCfg.MBSSID[apidx].wdev.IEEE8021X == TRUE))
	{
		/* Init the frame buffer */
		NdisZeroMemory(FrameBuf, frame_len);

		/* Assign apidx */
		apidx = pEntry->func_tb_idx;

		/* Prepare the 802.3 header */
		MAKE_802_3_HEADER(FrameBuf, pAd->ApCfg.MBSSID[apidx].wdev.bssid, pEntry->Addr, EAPOL);
		offset += LENGTH_802_3;

		/* Prepare a fake eapol-start body */
		NdisMoveMemory(&FrameBuf[offset], eapol_start_1x_hdr, sizeof(eapol_start_1x_hdr));

#ifdef CONFIG_HOTSPOT_R2
		if (pEntry)
		{
        		BSS_STRUCT *pMbss = pEntry->pMbss;
			if ((pMbss->HotSpotCtrl.HotSpotEnable == 1) && (pMbss->wdev.AuthMode == Ndis802_11AuthModeWPA2) && (pEntry->hs_info.ppsmo_exist == 1))
			{
                		UCHAR HS2_Header[4] = {0x50,0x6f,0x9a,0x12};
				memcpy(&FrameBuf[offset+sizeof(eapol_start_1x_hdr)], HS2_Header, 4);
				memcpy(&FrameBuf[offset+sizeof(eapol_start_1x_hdr)+4], &pEntry->hs_info, sizeof(struct _sta_hs_info));
				frame_len += 4+sizeof(struct _sta_hs_info);
				printk("event eapol start, %x:%x:%x:%x\n", 
						FrameBuf[offset+sizeof(eapol_start_1x_hdr)+4],FrameBuf[offset+sizeof(eapol_start_1x_hdr)+5], 
						FrameBuf[offset+sizeof(eapol_start_1x_hdr)+6],FrameBuf[offset+sizeof(eapol_start_1x_hdr)+7]);
            		}
		}
#endif	
		/* Report to upper layer */
		if (RTMP_L2_FRAME_TX_ACTION(pAd, apidx, FrameBuf, frame_len) == FALSE)
			return FALSE;

		DBGPRINT(RT_DEBUG_TRACE, ("Notify 8021.x daemon to trigger EAP-SM for this sta(%02x:%02x:%02x:%02x:%02x:%02x)\n", PRINT_MAC(pEntry->Addr)));

	}

	return TRUE;
}

#endif /* DOT1X_SUPPORT */

#ifdef CONN_FAIL_EVENT
void ApSendConnFailMsg(
	PRTMP_ADAPTER pAd,
	CHAR *Ssid,
	UCHAR SsidLen,
	UCHAR *StaAddr,
	USHORT ReasonCode)
{
	struct CONN_FAIL_MSG msg;

	if (!pAd)
		return;

	memset(&msg, 0, sizeof(msg));

	if (SsidLen > 32)
		msg.SsidLen = 32;
	else
		msg.SsidLen = SsidLen;

	if (Ssid && (msg.SsidLen > 0))
		memcpy(msg.Ssid, Ssid, msg.SsidLen);

	if (StaAddr)
		memcpy(msg.StaAddr, StaAddr, 6);

	msg.ReasonCode = ReasonCode;

	RtmpOSWrielessEventSend(
		pAd->net_dev,
		RT_WLAN_EVENT_CUSTOM,
		OID_802_11_CONN_FAIL_MSG,
		NULL,
		(UCHAR *)&msg,
		sizeof(msg));

	DBGPRINT(RT_DEBUG_TRACE,
		("%s, SsidLen=%d, Ssid=%s, StaAddr=%02x:%02x:%02x:%02x:%02x:%02x, ReasonCode=%d\n",
		__func__, msg.SsidLen, msg.Ssid, PRINT_MAC(msg.StaAddr), msg.ReasonCode));
}
#endif

