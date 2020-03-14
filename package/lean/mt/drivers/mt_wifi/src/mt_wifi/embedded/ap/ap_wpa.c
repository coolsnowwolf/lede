/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
    wpa.c

    Abstract:

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
    Jan Lee     03-07-22        Initial
    Rory Chen   04-11-29        Add WPA2PSK
*/
#include "rt_config.h"

extern UCHAR	EAPOL[];

/*
    ==========================================================================
    Description:
	Port Access Control Inquiry function. Return entry's Privacy and Wpastate.
    Return:
	pEntry
    ==========================================================================
*/
MAC_TABLE_ENTRY *PACInquiry(RTMP_ADAPTER *pAd, UCHAR Wcid)
{
	MAC_TABLE_ENTRY *pEntry = NULL;

	if (VALID_UCAST_ENTRY_WCID(pAd, Wcid))
		pEntry = &(pAd->MacTab.Content[Wcid]);

	return pEntry;
}


/*
    ==========================================================================
    Description:
	Function to handle countermeasures active attack.  Init 60-sec timer if necessary.
    Return:
    ==========================================================================
*/
VOID HandleCounterMeasure(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry)
{
	INT i;
	BOOLEAN Cancelled;

	if (!pEntry)
		return;

	/* Todo by AlbertY - Not support currently in ApClient-link */
	if (IS_ENTRY_APCLI(pEntry) || IS_ENTRY_REPEATER(pEntry))
		return;

	/* if entry not set key done, ignore this RX MIC ERROR */
	if ((pEntry->SecConfig.Handshake.WpaState < AS_PTKINITDONE) || (pEntry->SecConfig.Handshake.GTKState != REKEY_ESTABLISHED))
		return;

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("HandleCounterMeasure ===>\n"));
	/* record which entry causes this MIC error, if this entry sends disauth/disassoc, AP doesn't need to log the CM */
	pEntry->CMTimerRunning = TRUE;
	pAd->ApCfg.MICFailureCounter++;
	/* send wireless event - for MIC error */
	RTMPSendWirelessEvent(pAd, IW_MIC_ERROR_EVENT_FLAG, pEntry->Addr, 0, 0);

	if (pAd->ApCfg.CMTimerRunning == TRUE) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Receive CM Attack Twice within 60 seconds ====>>>\n"));
		/* send wireless event - for counter measures */
		RTMPSendWirelessEvent(pAd, IW_COUNTER_MEASURES_EVENT_FLAG, pEntry->Addr, 0, 0);
		ApLogEvent(pAd, pEntry->Addr, EVENT_COUNTER_M);
		/* renew GTK */
		GenRandom(pAd, pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.bssid, pAd->ApCfg.MBSSID[pEntry->func_tb_idx].GNonce);
		/* Cancel CounterMeasure Timer */
		RTMPCancelTimer(&pAd->ApCfg.CounterMeasureTimer, &Cancelled);
		pAd->ApCfg.CMTimerRunning = FALSE;

		for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
			struct wifi_dev *wdev = pAd->MacTab.Content[i].wdev;
			/* happened twice within 60 sec,  AP SENDS disaccociate all associated STAs.  All STA's transition to State 2 */
			if ((IS_ENTRY_CLIENT(&pAd->MacTab.Content[i])) && wdev != NULL
			&& wdev->channel == pEntry->wdev->channel)
				MlmeDeAuthAction(pAd, &pAd->MacTab.Content[i], REASON_MIC_FAILURE, FALSE);
		}

		/*
			Further,  ban all Class 3 DATA transportation for a period 0f 60 sec
			disallow new association , too
		*/
		pAd->ApCfg.BANClass3Data = TRUE;
		/* check how many entry left...  should be zero */
		/*pAd->ApCfg.MBSSID[pEntry->func_tb_idx].GKeyDoneStations = pAd->MacTab.Size; */
		/*MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("GKeyDoneStations=%d\n", pAd->ApCfg.MBSSID[pEntry->func_tb_idx].GKeyDoneStations)); */
	}

	RTMPSetTimer(&pAd->ApCfg.CounterMeasureTimer, 60 * MLME_TASK_EXEC_INTV * MLME_TASK_EXEC_MULTIPLE);
	pAd->ApCfg.CMTimerRunning = TRUE;
	pAd->ApCfg.PrevaMICFailTime = pAd->ApCfg.aMICFailTime;
	RTMP_GetCurrentSystemTime(&pAd->ApCfg.aMICFailTime);
}


/*
    ==========================================================================
    Description:
	countermeasures active attack timer execution
    Return:
    ==========================================================================
*/
VOID CMTimerExec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	UINT            i, j = 0;
	PRTMP_ADAPTER   pAd = (PRTMP_ADAPTER)FunctionContext;

	pAd->ApCfg.BANClass3Data = FALSE;

	for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
		if (IS_ENTRY_CLIENT(&pAd->MacTab.Content[i])
			&& (pAd->MacTab.Content[i].CMTimerRunning == TRUE)) {
			pAd->MacTab.Content[i].CMTimerRunning = FALSE;
			j++;
		}
	}

	if (j > 1)
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Find more than one entry which generated MIC Fail ..\n"));

	pAd->ApCfg.CMTimerRunning = FALSE;
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("Counter measure timer expired, resume connection access.\n"));
}


#ifdef HOSTAPD_SUPPORT
/*for sending an event to notify hostapd about michael failure. */
VOID ieee80211_notify_michael_failure(
	IN	PRTMP_ADAPTER    pAd,
	IN	PHEADER_802_11   pHeader,
	IN	UINT            keyix,
	IN	INT              report)
{
	static const char *tag = "MLME-MICHAELMICFAILURE.indication";
	/*	struct net_device *dev = pAd->net_dev; */
	/*	union iwreq_data wrqu; */
	char buf[128];		/* XXX */

	/* TODO: needed parameters: count, keyid, key type, src address, TSC */
	if (report) { /*station reports a mic error to this ap. */
		snprintf(buf, sizeof(buf), "%s(keyid=%d %scast addr=%s)", tag,
				 keyix, "uni",
				 ether_sprintf(pHeader->Addr2));
	} else { /*ap itself receives a mic error. */
		snprintf(buf, sizeof(buf), "%s(keyid=%d %scast addr=%s)", tag,
				 keyix, IEEE80211_IS_MULTICAST(pHeader->Addr1) ?  "broad" : "uni",
				 ether_sprintf(pHeader->Addr2));
	}

	RtmpOSWrielessEventSend(pAd->net_dev, RT_WLAN_EVENT_CUSTOM, -1, NULL, NULL, 0);
	/*	NdisZeroMemory(&wrqu, sizeof(wrqu)); */
	/*	wrqu.data.length = strlen(buf); */
	/*	wireless_send_event(dev, RT_WLAN_EVENT_CUSTOM, &wrqu, buf); */
}


const CHAR *ether_sprintf(const UINT8 *mac)
{
	static char etherbuf[18];

	snprintf(etherbuf, sizeof(etherbuf), "%02x:%02x:%02x:%02x:%02x:%02x", PRINT_MAC(mac));
	return etherbuf;
}
#endif /* HOSTAPD_SUPPORT */


#ifdef APCLI_SUPPORT
#endif/*APCLI_SUPPORT*/

