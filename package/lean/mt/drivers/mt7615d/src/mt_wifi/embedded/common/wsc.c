/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2006, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of Ralink Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	wsc.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Paul Lin	06-08-08		Initial
	Snowpin Lee 06-09-12        Do modifications and Add APIs for AP
	Snowpin Lee 07-04-19        Do modifications and Add APIs for STA
	Snowpin Lee 07-05-17        Do modifications and Add APIs for AP Client
*/

#include    "rt_config.h"

#ifdef WSC_INCLUDED
#ifdef VENDOR_FEATURE7_SUPPORT
#ifdef WSC_LED_SUPPORT
#include "rt_led.h"
#endif /* WSC_LED_SUPPORT */
#include "arris_wps_gpio_handler.h"
#endif
#include    "wsc_tlv.h"
#include "hdev/hdev.h"
/*#ifdef LINUX */
/*#include <net/iw_handler.h> */
/*#endif*/
#ifdef INTELP6_SUPPORT
#ifdef MULTI_INF_SUPPORT
#define FIRST_AP_PROFILE_PATH	"/tmp/mt76xx_24.dat"		/* "/tmp/mt76xx_24.dat" */
#define SECOND_AP_PROFILE_PATH	"/tmp/mt76xx_5.dat"		/* "/tmp/mt76xx_5.dat" */
#endif
#endif

#define WSC_UPNP_MSG_TIMEOUT            (150 * OS_HZ)
#define RTMP_WSC_NLMSG_SIGNATURE_LEN    8
#define MAX_WEPKEYNAME_LEN              20
#define MAX_WEPKEYTYPE_LEN              20

#ifndef PF_NOFREEZE
#define PF_NOFREEZE  0
#endif

char WSC_MSG_SIGNATURE[] = {"RAWSCMSG"};

UINT8 WPS_DH_G_VALUE[1] = {0x02};
UINT8 WPS_DH_P_VALUE[192] = {
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xC9, 0x0F, 0xDA, 0xA2, 0x21, 0x68, 0xC2, 0x34,
	0xC4, 0xC6, 0x62, 0x8B, 0x80, 0xDC, 0x1C, 0xD1,
	0x29, 0x02, 0x4E, 0x08, 0x8A, 0x67, 0xCC, 0x74,
	0x02, 0x0B, 0xBE, 0xA6, 0x3B, 0x13, 0x9B, 0x22,
	0x51, 0x4A, 0x08, 0x79, 0x8E, 0x34, 0x04, 0xDD,
	0xEF, 0x95, 0x19, 0xB3, 0xCD, 0x3A, 0x43, 0x1B,
	0x30, 0x2B, 0x0A, 0x6D, 0xF2, 0x5F, 0x14, 0x37,
	0x4F, 0xE1, 0x35, 0x6D, 0x6D, 0x51, 0xC2, 0x45,
	0xE4, 0x85, 0xB5, 0x76, 0x62, 0x5E, 0x7E, 0xC6,
	0xF4, 0x4C, 0x42, 0xE9, 0xA6, 0x37, 0xED, 0x6B,
	0x0B, 0xFF, 0x5C, 0xB6, 0xF4, 0x06, 0xB7, 0xED,
	0xEE, 0x38, 0x6B, 0xFB, 0x5A, 0x89, 0x9F, 0xA5,
	0xAE, 0x9F, 0x24, 0x11, 0x7C, 0x4B, 0x1F, 0xE6,
	0x49, 0x28, 0x66, 0x51, 0xEC, 0xE4, 0x5B, 0x3D,
	0xC2, 0x00, 0x7C, 0xB8, 0xA1, 0x63, 0xBF, 0x05,
	0x98, 0xDA, 0x48, 0x36, 0x1C, 0x55, 0xD3, 0x9A,
	0x69, 0x16, 0x3F, 0xA8, 0xFD, 0x24, 0xCF, 0x5F,
	0x83, 0x65, 0x5D, 0x23, 0xDC, 0xA3, 0xAD, 0x96,
	0x1C, 0x62, 0xF3, 0x56, 0x20, 0x85, 0x52, 0xBB,
	0x9E, 0xD5, 0x29, 0x07, 0x70, 0x96, 0x96, 0x6D,
	0x67, 0x0C, 0x35, 0x4E, 0x4A, 0xBC, 0x98, 0x04,
	0xF1, 0x74, 0x6C, 0x08, 0xCA, 0x23, 0x73, 0x27,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
};

/* General used field */
UCHAR	STA_Wsc_Pri_Dev_Type[8] = {0x00, 0x01, 0x00, 0x50, 0xf2, 0x04, 0x00, 0x01};

#ifdef CON_WPS
UUID_BSSID_CH_INFO      TmpInfo_2G;
UUID_BSSID_CH_INFO      TmpInfo_5G;
#endif /*CON_WPS Dung_Ru*/

#ifdef CONFIG_AP_SUPPORT
UCHAR	AP_Wsc_Pri_Dev_Type[8] = {0x00, 0x06, 0x00, 0x50, 0xf2, 0x04, 0x00, 0x01};

#ifdef APCLI_SUPPORT

VOID WscApCliLinkDown(
	IN  PRTMP_ADAPTER pAd,
	IN  PWSC_CTRL pWscControl);
#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

static BOOLEAN WscCheckNonce(
	IN  PRTMP_ADAPTER pAdapter,
	IN  MLME_QUEUE_ELEM * Elem,
	IN  BOOLEAN bFlag,
	IN  PWSC_CTRL pWscControl);


static VOID WscGetConfigErrFromNack(
	IN  PRTMP_ADAPTER pAdapter,
	IN  MLME_QUEUE_ELEM * pElem,
	OUT USHORT * pConfigError);

static INT WscSetAuthMode(
	IN  PRTMP_ADAPTER pAd,
	IN  UCHAR CurOpMode,
	IN  UCHAR apidx,
	IN  RTMP_STRING * arg);

static INT WscSetEncrypType(
	IN  PRTMP_ADAPTER pAd,
	IN  UCHAR CurOpMode,
	IN  UCHAR apidx,
	IN  RTMP_STRING * arg);

static VOID WscSendNACK(
	IN  PRTMP_ADAPTER pAdapter,
	IN  MAC_TABLE_ENTRY * pEntry,
	IN  PWSC_CTRL pWscControl);

static INT wsc_write_dat_file_thread(IN ULONG data);


#ifdef CONFIG_AP_SUPPORT
static UINT32 WscGetAuthMode(
	IN  USHORT authFlag);

static UINT32 WscGetWepStatus(
	IN  USHORT encryFlag);
#endif /* CONFIG_AP_SUPPORT */

#ifdef CON_WPS
VOID ConWpsApCliMonitorTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)FunctionContext;
	PRTMP_ADAPTER pOpposAd, pActionAd;
	UINT actionBandIdx = 0;
#ifdef MULTI_INF_SUPPORT
	UINT nowBandIdx = multi_inf_get_idx(pAd);
	UINT opsBandIdx = !nowBandIdx;
#else
	UINT nowBandIdx = 0;
#endif /* MULTI_INF_SUPPORT */
	UCHAR index = BSS0;

	pOpposAd = NULL;
#ifdef MULTI_INF_SUPPORT
	pOpposAd = (PRTMP_ADAPTER)adapt_list[opsBandIdx];
	nowBandIdx = multi_inf_get_idx(pAd);
#else
	pOpposAd = NULL;
#endif /* MULTI_INF_SUPPORT */
	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_OFF, ("%s: Interface %s\n", __func__, pAd->net_dev->name));
#ifdef MULTI_INF_SUPPORT

	if (pOpposAd) {
		if (pOpposAd->ApCfg.ConWpsApCliStatus == TRUE) {
			/* 2G/5G Band WPS Ready */
			if (pAd->ApCfg.ConWpsApCliMode == CON_WPS_APCLI_BAND_2G)
				actionBandIdx = 0;
			else
				actionBandIdx = 1;
		} else
			actionBandIdx = nowBandIdx;

		WscStop(pAd, TRUE, &pAd->ApCfg.ApCliTab[BSS0].wdev.WscControl);
	} else
#endif /* MULTI_INF_SUPPORT */
	{
		actionBandIdx = nowBandIdx;

		/* Cancel the other band if onGoing */
		if (pAd->ApCfg.ConWpsApCliMode == CON_WPS_APCLI_BAND_2G)
			WscStop(pAd, TRUE, &pAd->ApCfg.ApCliTab[BSS1].wdev.WscControl);
		else if (pAd->ApCfg.ConWpsApCliMode == CON_WPS_APCLI_BAND_5G)
			WscStop(pAd, TRUE, &pAd->ApCfg.ApCliTab[BSS0].wdev.WscControl);
	}

#ifdef MULTI_INF_SUPPORT
	pActionAd = (PRTMP_ADAPTER)adapt_list[actionBandIdx];
#else
	pActionAd = pAd;
#endif /* MULTI_INF_SUPPORT */
	{
		for (index = 0; index < MAX_APCLI_NUM; index++)
			RTEnqueueInternalCmd(pActionAd, CMDTHREAD_APCLI_IF_DOWN, (VOID *)&index, sizeof(UCHAR));
	}
	pActionAd->ApCfg.ApCliTab[BSS0].Enable = TRUE;

	for (index = 0; index < MAX_APCLI_NUM; index++)
		pAd->ApCfg.ApCliTab[index].wdev.WscControl.conWscStatus = CON_WPS_STATUS_DISABLED;

	if (pOpposAd)
		pOpposAd->ApCfg.ConWpsApCliStatus = FALSE;

	pAd->ApCfg.ConWpsApCliStatus = FALSE;
}

DECLARE_TIMER_FUNCTION(ConWpsApCliMonitorTimeout);
BUILD_TIMER_FUNCTION(ConWpsApCliMonitorTimeout);
#endif /* CON_WPS */


/*
*	Standard UUID generation procedure. The UUID format generated by this function is base on UUID std. version 1.
*	It's a 16 bytes, one-time global unique number. and can show in string format like this:
*			550e8400-e29b-41d4-a716-446655440000
*	The format of uuid is:
*		uuid                        = <time_low> "-"
*					      <time_mid> "-"
*					      <time_high_and_version> "-"
*					      <clock_seq_high_and_reserved>
*		                          <clock_seq_low> "-"
*					      <node>
*		time_low                    = 4*<hex_octet>
*		time_mid                    = 2*<hex_octet>
*		time_high_and_version       = 2*<hex_octet>
*		clock_seq_high_and_reserved = <hex_octet>
*		clock_seq_low               = <hex_octet>
*		node                        = 6*<hex_octet>
*		hex_octet                   = <hex_digit> <hex_digit>
*		hex_digit                   = "0"|"1"|"2"|"3"|"4"|"5"|"6"|"7"|"8"|"9"
*					      |"a"|"b"|"c"|"d"|"e"|"f"
*					      |"A"|"B"|"C"|"D"|"E"|"F"
*	Note:
*		Actually, to IOT with JumpStart, we fix the first 10 bytes of UUID string!!!!
*/
INT WscGenerateUUID(
	RTMP_ADAPTER * pAd,
	UCHAR *uuidHexStr,
	UCHAR *uuidAscStr,
	int apIdx,
	BOOLEAN	bUseCurrentTime,
	BOOLEAN from_apcli)
{
	WSC_UUID_T uuid_t;
	unsigned long long uuid_time;
	int i;
	UINT16 clkSeq;
	char uuidTmpStr[UUID_LEN_STR + 2];
#ifdef MULTI_INF_SUPPORT
#ifdef CON_WPS_AP_SAME_UUID
	PRTMP_ADAPTER pOpposAd;
	PWSC_CTRL pWscControl;

	/* We Assume the 5G init phase after 2.4Ghz */
	if (from_apcli == FALSE) {
		for (i = 0; i < MAX_NUM_OF_INF; i++) {
			if ((pAd != adapt_list[i]) && (adapt_list[i] != NULL)) {
				pOpposAd = (PRTMP_ADAPTER) adapt_list[i];
				if (pOpposAd->ApCfg.MBSSID[apIdx].wdev.if_up_down_state) {
					pWscControl = &pOpposAd->ApCfg.MBSSID[apIdx].wdev.WscControl;
					NdisCopyMemory(uuidHexStr, &pWscControl->Wsc_Uuid_E[0], UUID_LEN_HEX);
					NdisCopyMemory(uuidAscStr, &pWscControl->Wsc_Uuid_Str[0], UUID_LEN_STR);
					goto show;
				}
			}
		}
	}

#endif /* CON_WPS_AP_SAME_UUID */
#endif /* MULTI_INF_SUPPORT */
#ifdef RTMP_RBUS_SUPPORT
	/* for fixed UUID -  YYHuang 07/10/09 */
#define FIXED_UUID
#endif /* RTMP_RBUS_SUPPORT */

	/* Get the current time. */
	if (bUseCurrentTime) {
		ULONG Now;

		NdisGetSystemUpTime(&Now);
		uuid_time = Now;
	} else
		uuid_time = 2860; /*xtime.tv_sec;	// Well, we fix this to make JumpStart  happy! */

	uuid_time *= 10000000;
	uuid_time += 0x01b21dd213814000LL;
#ifdef RTMP_RBUS_SUPPORT
#ifdef FIXED_UUID

	if (IS_RBUS_INF(pAd))
		uuid_time  = 0x2880288028802880LL;

#endif
#endif /* RTMP_RBUS_SUPPORT */
	uuid_t.timeLow = (UINT32)uuid_time & 0xFFFFFFFF;
	uuid_t.timeMid = (UINT16)((uuid_time >> 32) & 0xFFFF);
	uuid_t.timeHi_Version = (UINT16)((uuid_time >> 48) & 0x0FFF);
	uuid_t.timeHi_Version |= (1 << 12);
	/* Get the clock sequence. */
	clkSeq = (UINT16)(0x0601/*jiffies*/ & 0xFFFF);		/* Again, we fix this to make JumpStart happy! */
#ifdef RTMP_RBUS_SUPPORT
#ifdef FIXED_UUID

	if (IS_RBUS_INF(pAd))
		clkSeq = (UINT16)0x2880;

#endif
#endif /* RTMP_RBUS_SUPPORT */
	uuid_t.clockSeqLow = clkSeq & 0xFF;
	uuid_t.clockSeqHi_Var = (clkSeq & 0x3F00) >> 8;
	uuid_t.clockSeqHi_Var |= 0x80;
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		/* copy the Mac address as the value of node */
#ifdef APCLI_SUPPORT
		if (from_apcli)
			NdisMoveMemory(&uuid_t.node[0], &pAd->ApCfg.ApCliTab[apIdx].wdev.if_addr[0], sizeof(uuid_t.node));
		else
#endif /* APCLI_SUPPORT */
		{
			NdisMoveMemory(&uuid_t.node[0], &pAd->ApCfg.MBSSID[apIdx].wdev.if_addr[0], sizeof(uuid_t.node));
		}
	}
#endif /* CONFIG_AP_SUPPORT */
	/* Create the UUID ASCII string. */
	memset(uuidTmpStr, 0, sizeof(uuidTmpStr));
	snprintf(uuidTmpStr, sizeof(uuidTmpStr), "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
			 (unsigned int)uuid_t.timeLow, uuid_t.timeMid, uuid_t.timeHi_Version, uuid_t.clockSeqHi_Var, uuid_t.clockSeqLow,
			 uuid_t.node[0], uuid_t.node[1], uuid_t.node[2], uuid_t.node[3], uuid_t.node[4], uuid_t.node[5]);

	if (strlen(uuidTmpStr) > UUID_LEN_STR)
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("ERROR:UUID String size too large!\n"));

	strncpy((RTMP_STRING *)uuidAscStr, uuidTmpStr, UUID_LEN_STR);
	/* Create the UUID Hex format number */
	uuid_t.timeLow = cpu2be32(uuid_t.timeLow);
	NdisMoveMemory(&uuidHexStr[0], &uuid_t.timeLow, 4);
	uuid_t.timeMid = cpu2be16(uuid_t.timeMid);
	NdisMoveMemory(&uuidHexStr[4], &uuid_t.timeMid, 2);
	uuid_t.timeHi_Version = cpu2be16(uuid_t.timeHi_Version);
	NdisMoveMemory(&uuidHexStr[6], &uuid_t.timeHi_Version, 2);
	NdisMoveMemory(&uuidHexStr[8], &uuid_t.clockSeqHi_Var, 1);
	NdisMoveMemory(&uuidHexStr[9], &uuid_t.clockSeqLow, 1);
	NdisMoveMemory(&uuidHexStr[10], &uuid_t.node[0], 6);
#ifdef MULTI_INF_SUPPORT
#ifdef CON_WPS_AP_SAME_UUID
show:
#endif /* CON_WPS_AP_SAME_UUID */
#endif /* MULTI_INF_SUPPORT */
	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("The UUID Hex string is:"));

	for (i = 0; i < 16; i++)
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("%02x", (uuidHexStr[i] & 0xff)));

	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("\n"));
	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("The UUID ASCII string is:%s!\n", uuidAscStr));
	return 0;
}

VOID	WscInitCommonTimers(
	IN  PRTMP_ADAPTER pAdapter,
	IN  PWSC_CTRL pWscControl)
{
	WSC_TIMER_INIT(pAdapter, pWscControl, &pWscControl->EapolTimer, pWscControl->EapolTimerRunning, WscEAPOLTimeOutAction);
	WSC_TIMER_INIT(pAdapter, pWscControl, &pWscControl->Wsc2MinsTimer, pWscControl->Wsc2MinsTimerRunning, Wsc2MinsTimeOutAction);
	WSC_TIMER_INIT(pAdapter, pWscControl, &pWscControl->WscUPnPNodeInfo.UPnPMsgTimer, pWscControl->WscUPnPNodeInfo.bUPnPMsgTimerRunning, WscUPnPMsgTimeOutAction);
	pWscControl->WscUPnPNodeInfo.bUPnPMsgTimerPending = FALSE;
	WSC_TIMER_INIT(pAdapter, pWscControl, &pWscControl->M2DTimer, pWscControl->bM2DTimerRunning, WscM2DTimeOutAction);
#ifdef WSC_LED_SUPPORT
	WSC_TIMER_INIT(pAdapter, pWscControl, &pWscControl->WscLEDTimer, pWscControl->WscLEDTimerRunning, WscLEDTimer);
	WSC_TIMER_INIT(pAdapter, pWscControl, &pWscControl->WscSkipTurnOffLEDTimer, pWscControl->WscSkipTurnOffLEDTimerRunning, WscSkipTurnOffLEDTimer);
#endif /* WSC_LED_SUPPORT */
}

VOID	WscInitClientTimers(
	IN  PRTMP_ADAPTER pAdapter,
	IN  PWSC_CTRL pWScControl)
{
	WSC_TIMER_INIT(pAdapter, pWScControl, &pWScControl->WscPBCTimer, pWScControl->WscPBCTimerRunning, WscPBCTimeOutAction);
#ifdef WSC_STA_SUPPORT
	WSC_TIMER_INIT(pAdapter, pWScControl, &pWScControl->WscPINTimer, pWScControl->WscPINTimerRunning, WscPINTimeOutAction);
#endif
	WSC_TIMER_INIT(pAdapter, pWScControl, &pWScControl->WscScanTimer, pWScControl->WscScanTimerRunning, WscScanTimeOutAction);
	WSC_TIMER_INIT(pAdapter, pWScControl, &pWScControl->WscProfileRetryTimer, pWScControl->WscProfileRetryTimerRunning, WscProfileRetryTimeout);  /* add by johnli, fix WPS test plan 5.1.1 */
#ifdef CON_WPS
	WSC_TIMER_INIT(pAdapter, pWScControl, &pWScControl->ConWscApcliScanDoneCheckTimer, pWScControl->ConWscApcliScanDoneCheckTimerRunning, WscScanDoneCheckTimeOutAction);
#endif /*CON_WPS*/
}

/*
*	==========================================================================
*	Description:
*		wps state machine init, including state transition and timer init
*	Parameters:
*		S - pointer to the association state machine
*	==========================================================================
*/
VOID    WscStateMachineInit(
	IN  PRTMP_ADAPTER pAd,
	IN  STATE_MACHINE * S,
	OUT STATE_MACHINE_FUNC Trans[])
{
	PWSC_CTRL pWScControl;

	StateMachineInit(S,	(STATE_MACHINE_FUNC *)Trans, MAX_WSC_STATE, MAX_WSC_MSG, (STATE_MACHINE_FUNC)Drop, WSC_IDLE, WSC_MACHINE_BASE);
	StateMachineSetAction(S, WSC_IDLE, WSC_EAPOL_START_MSG, (STATE_MACHINE_FUNC)WscEAPOLStartAction);
	StateMachineSetAction(S, WSC_IDLE, WSC_EAPOL_PACKET_MSG, (STATE_MACHINE_FUNC)WscEAPAction);
	StateMachineSetAction(S, WSC_IDLE, WSC_EAPOL_UPNP_MSG, (STATE_MACHINE_FUNC)WscEAPAction);
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		UCHAR apidx;

		for (apidx = 0; apidx < MAX_MBSSID_NUM(pAd); apidx++) {
			pWScControl = &pAd->ApCfg.MBSSID[apidx].wdev.WscControl;
			pWScControl->pAd = pAd;
			pWScControl->wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
			pWScControl->EntryIfIdx = (MIN_NET_DEVICE_FOR_MBSSID | apidx);
			WscInitCommonTimers(pAd, pWScControl);
			pWScControl->WscUpdatePortCfgTimerRunning = FALSE;
			WSC_TIMER_INIT(pAd, pWScControl, &pWScControl->WscUpdatePortCfgTimer, pWScControl->WscUpdatePortCfgTimerRunning, WscUpdatePortCfgTimeout);
#ifdef WSC_V2_SUPPORT
			WSC_TIMER_INIT(pAd, pWScControl, &pWScControl->WscSetupLockTimer, pWScControl->WscSetupLockTimerRunning, WscSetupLockTimeout);
#endif /* WSC_V2_SUPPORT */
		}

#ifdef APCLI_SUPPORT

		for (apidx = 0; apidx < MAX_APCLI_NUM; apidx++) {
			pWScControl = &pAd->ApCfg.ApCliTab[apidx].wdev.WscControl;
			pWScControl->pAd = pAd;
			pWScControl->wdev = &pAd->ApCfg.ApCliTab[apidx].wdev;
			pWScControl->EntryIfIdx = (MIN_NET_DEVICE_FOR_APCLI | apidx);
			WscInitCommonTimers(pAd, pWScControl);
			WscInitClientTimers(pAd, pWScControl);
		}

#endif /* APCLI_SUPPORT */
#ifdef CON_WPS
		RTMPInitTimer(pAd, &pAd->ApCfg.ConWpsApCliBandMonitorTimer, GET_TIMER_FUNCTION(ConWpsApCliMonitorTimeout), pAd, FALSE);
		pAd->ApCfg.ConWpsMonitorTimerRunning = FALSE;
#endif /* CON_WPS */
	}
#endif /* CONFIG_AP_SUPPORT */
}

void WscM2DTimeOutAction(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	/* For each state, we didn't care about the retry issue, we just send control message
	*	to notify the UPnP deamon that some error happened in STATE MACHINE.
	*/
	PWSC_CTRL pWscControl = (PWSC_CTRL)FunctionContext;
	PRTMP_ADAPTER pAd = NULL;
	WSC_UPNP_NODE_INFO *pWscNodeInfo;
#ifdef CONFIG_AP_SUPPORT
	MAC_TABLE_ENTRY *pEntry = NULL;
#endif /* CONFIG_AP_SUPPORT */
	BOOLEAN Cancelled;
	UCHAR CurOpMode = 0xFF;

	if (!pWscControl) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR,
				 ("%s: pWscControl is NULL!!\n", __func__));
		return;
	}

	pAd = (PRTMP_ADAPTER)pWscControl->pAd;

	if (!pAd) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR,
				 ("%s: pAd is NULL!!\n", __func__));
		return;
	}

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	CurOpMode = AP_MODE;
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_AP_SUPPORT

	if (CurOpMode == AP_MODE)
		pEntry = MacTableLookup(pAd, pWscControl->EntryAddr);

#endif /* CONFIG_AP_SUPPORT */
	pWscNodeInfo = &pWscControl->WscUPnPNodeInfo;

	if (!pWscNodeInfo) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR,
				 ("%s: pWscNodeInfo is NULL!!\n", __func__));
		return;
	}

	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("UPnP StateMachine TimeOut(State=%d!)\n", pWscControl->WscState));

	if (
#ifdef CONFIG_AP_SUPPORT
		(((pEntry == NULL) || (pWscNodeInfo->registrarID != 0)) &&  (CurOpMode == AP_MODE)) ||
#endif /* CONFIG_AP_SUPPORT */
		(0)) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("%s():pEntry maybe gone or already received M2 Packet!\n", __func__));
		goto done;
	}

	if (pWscControl->M2DACKBalance != 0) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("%s(): waiting for M2DACK balance, extend the time!\n", __func__));
		/* Waiting for M2DACK balance. */
		RTMPModTimer(&pWscControl->M2DTimer, WSC_EAP_ID_TIME_OUT);
		pWscControl->M2DACKBalance = 0;
		goto done;
	} else {
		RTMPCancelTimer(&pWscControl->EapolTimer, &Cancelled);
		pWscControl->EapolTimerRunning = FALSE;
#ifdef CONFIG_AP_SUPPORT

		if (CurOpMode == AP_MODE) {
			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("%s(): send EAP-Fail to wireless Station!\n", __func__));
			/* Send EAPFail to Wireless Station and reset the status of Wsc. */
			WscSendEapFail(pAd, pWscControl, TRUE);

			/*pEntry->bWscCapable = FALSE; */
			if (pEntry != NULL)
				pEntry->Receive_EapolStart_EapRspId = 0;
		}

#endif /* CONFIG_AP_SUPPORT */
		pWscControl->EapMsgRunning = FALSE;
		pWscControl->WscState = WSC_STATE_OFF;
	}

done:
	pWscControl->bM2DTimerRunning = FALSE;
	pWscControl->M2DACKBalance = 0;
	pWscNodeInfo->registrarID = 0;
}


VOID WscUPnPMsgTimeOutAction(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	PWSC_CTRL pWscControl = (PWSC_CTRL)FunctionContext;
	PRTMP_ADAPTER pAd;
	WSC_UPNP_NODE_INFO *pWscNodeInfo;

	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("-----> WscUPnPMsgTimeOutAction\n"));

	/*It shouldn't happened! */
	if (!pWscControl)
		return;

	pAd = (PRTMP_ADAPTER)pWscControl->pAd;

	if (!pAd) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR,
				 ("%s: pAd is NULL!!\n", __func__));
		return;
	}

	pWscNodeInfo = &pWscControl->WscUPnPNodeInfo;

	if (!pWscNodeInfo) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR,
				 ("%s: pWscNodeInfo is NULL!!\n", __func__));
		return;
	}

	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("UPnP StateMachine TimeOut(State=%d!)\n", pWscControl->WscState));

	if (pWscNodeInfo->bUPnPMsgTimerPending) {
#define WSC_UPNP_TIMER_PENDIND_WAIT	2000
		RTMPModTimer(&pWscNodeInfo->UPnPMsgTimer, WSC_UPNP_TIMER_PENDIND_WAIT);
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("UPnPMsgTimer Pending......\n"));
	} else {
		int dataLen;
		UCHAR *pWscData;

		os_alloc_mem(NULL, (UCHAR **)&pWscData, WSC_MAX_DATA_LEN);

		if (pWscData != NULL) {
			memset(pWscData, 0, WSC_MAX_DATA_LEN);
			dataLen = BuildMessageNACK(pAd, pWscControl, pWscData);
			WscSendUPnPMessage(pAd, (pWscControl->EntryIfIdx & 0x0F),
							   WSC_OPCODE_UPNP_DATA, WSC_UPNP_DATA_SUB_NORMAL,
							   pWscData, dataLen, 0, 0, &pAd->CurrentAddress[0], AP_MODE);
			os_free_mem(pWscData);
		}

		pWscNodeInfo->bUPnPInProgress = FALSE;
		pWscNodeInfo->bUPnPMsgTimerPending = FALSE;
		pWscNodeInfo->bUPnPMsgTimerRunning = FALSE;
		pWscControl->WscState = WSC_STATE_OFF;
		pWscControl->WscStatus = STATUS_WSC_FAIL;
		RTMPSendWirelessEvent(pAd, IW_WSC_STATUS_FAIL, NULL, (pWscControl->EntryIfIdx & 0x0F), 0);
	}

	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("<----- WscUPnPMsgTimeOutAction\n"));
}

/*
*	==========================================================================
*	Description:
*		This function processes EapolStart packets from wps stations
*		or enqueued by self.
*
*	Return:
*		None
*	==========================================================================
*/
VOID WscEAPOLStartAction(
	IN  PRTMP_ADAPTER pAd,
	IN  MLME_QUEUE_ELEM * Elem)
{
	MAC_TABLE_ENTRY *pEntry;
	PWSC_CTRL pWpsCtrl = NULL;
	PHEADER_802_11 pHeader;
	PWSC_PEER_ENTRY pWscPeer = NULL;
	UCHAR CurOpMode = 0xFF;
	struct wifi_dev *wdev;

	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("-----> WscEAPOLStartAction\n"));
	wdev = Elem->wdev;
	ASSERT(wdev);
	pHeader = (PHEADER_802_11)Elem->Msg;
	wdev->wdev_ops->mac_entry_lookup(pAd, pHeader->Addr2, wdev, &pEntry);
	pWpsCtrl = &wdev->WscControl;

	/* Cannot find this wps station in MacTable of WPS AP. */
	if (pEntry == NULL) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("pEntry is NULL.\n"));
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("<----- WscEAPOLStartAction\n"));
		return;
	}

	if (WDEV_WSC_AP(wdev))
		CurOpMode = AP_MODE;
	else
		CurOpMode = STA_MODE;

#ifdef CONFIG_AP_SUPPORT
#ifdef CON_WPS
	{
		PWSC_CTRL pApCliWpsCtrl = NULL;

		if (CurOpMode == AP_MODE) {
			if (pEntry->func_tb_idx < MAX_APCLI_NUM)
				pApCliWpsCtrl = &pAd->ApCfg.ApCliTab[pEntry->func_tb_idx].wdev.WscControl;
			else
				pApCliWpsCtrl = &pAd->ApCfg.ApCliTab[BSS0].wdev.WscControl;

			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_OFF,
				("CON_WPS: Stop the ApCli WPS, state [%d]\n", pApCliWpsCtrl->WscState));
		}

		if (pApCliWpsCtrl && (pApCliWpsCtrl->conWscStatus != CON_WPS_STATUS_DISABLED) &&
			(pApCliWpsCtrl->WscState != WSC_STATE_OFF)) {
			WscStop(pAd, TRUE, pApCliWpsCtrl);
			pApCliWpsCtrl->WscConfMode = WSC_DISABLE;
			/* APCLI: For stop the other side of the band with WSC SM */
			WscConWpsStop(pAd, TRUE, pApCliWpsCtrl);
		}
	}
#endif /* CON_WPS */
#endif /* CONFIG_AP_SUPPORT */
	RTMP_SEM_LOCK(&pWpsCtrl->WscPeerListSemLock);
	WscInsertPeerEntryByMAC(&pWpsCtrl->WscPeerList, pEntry->Addr);
	RTMP_SEM_UNLOCK(&pWpsCtrl->WscPeerListSemLock);
	WscMaintainPeerList(pAd, pWpsCtrl);

	/*
	*	Check this STA is first one or not
	*/
	if (pWpsCtrl->WscPeerList.size != 0) {
		pWscPeer = (PWSC_PEER_ENTRY)pWpsCtrl->WscPeerList.pHead;

		if (NdisEqualMemory(pEntry->Addr, pWscPeer->mac_addr, MAC_ADDR_LEN) == FALSE) {
			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("This is not first WSC peer, ignore this EAPOL_Start!\n"));
			hex_dump("pEntry->Addr", pEntry->Addr, MAC_ADDR_LEN);
#ifdef CONFIG_AP_SUPPORT

			if (CurOpMode == AP_MODE)
				WscApShowPeerList(pAd, NULL);

#endif /* CONFIG_AP_SUPPORT */
			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("<----- WscEAPOLStartAction\n"));
			return;
		}
	}

	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("WscState = %d\n", pWpsCtrl->WscState));

	if ((pEntry->Receive_EapolStart_EapRspId == 0) ||
		(pWpsCtrl->WscState <= WSC_STATE_WAIT_REQ_ID)) {
		/* Receive the first EapolStart packet of this wps station. */
		pEntry->Receive_EapolStart_EapRspId |= WSC_ENTRY_GET_EAPOL_START;
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("WscEAPOLStartAction - receive EAPOL-Start from %02x:%02x:%02x:%02x:%02x:%02x\n",
				 PRINT_MAC(pEntry->Addr)));
		/* EapolStart packet is sent by station means this station wants to do wps process with AP. */
		pWpsCtrl->EapMsgRunning = TRUE;
		/* Update EntryAddr again */
		NdisMoveMemory(pWpsCtrl->EntryAddr, pEntry->Addr, MAC_ADDR_LEN);

		if (pEntry->bWscCapable == FALSE)
			pEntry->bWscCapable = TRUE;

		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("WscEAPOLStartAction(ra%d) - send EAP-Req(Id) to %02x:%02x:%02x:%02x:%02x:%02x\n",
				 pEntry->func_tb_idx, PRINT_MAC(pEntry->Addr)));
		/* Send EAP-Request/Id to station */
		WscSendEapReqId(pAd, pEntry, CurOpMode);
#if defined(CONFIG_MAP_SUPPORT)
		if (IS_MAP_TURNKEY_ENABLE(pAd))
			wapp_send_wsc_eapol_start_notification(pAd, wdev);
#endif
		if (!pWpsCtrl->EapolTimerRunning) {
			pWpsCtrl->EapolTimerRunning = TRUE;
			/* Set WPS_EAP Messages timeout function. */
			RTMPSetTimer(&pWpsCtrl->EapolTimer, WSC_EAP_ID_TIME_OUT);
		}
	} else
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("Ignore EAPOL-Start.\n"));

	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("<----- WscEAPOLStartAction\n"));
}

#ifdef APCLI_SUPPORT
#ifdef CON_WPS
static VOID WscConWPSChecking(
	IN  PRTMP_ADAPTER pAd,
	IN  PRTMP_ADAPTER pOpposAd,
	IN  UCHAR if_idx,
	IN  PWSC_CTRL pWscControl)
{
#ifdef MULTI_INF_SUPPORT
	if (pOpposAd) {
		if (pOpposAd->ApCfg.ConWpsApCliStatus == TRUE) {
			PRTMP_ADAPTER pActionAd;
			BOOLEAN Cancelled;

			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_OFF,
				("%s another Band WPS ready\n", pAd->net_dev->name));
			/* another Band WPS ready */
			if (pOpposAd->ApCfg.ConWpsMonitorTimerRunning) {
				RTMPCancelTimer(&pOpposAd->ApCfg.ConWpsApCliBandMonitorTimer, &Cancelled);
				pOpposAd->ApCfg.ConWpsMonitorTimerRunning = FALSE;

				/* BandSelection */
				if (pAd->ApCfg.ConWpsApCliMode == CON_WPS_APCLI_BAND_2G)
					pActionAd = (PRTMP_ADAPTER)adapt_list[0];
				else
					pActionAd = (PRTMP_ADAPTER)adapt_list[1];

				pActionAd->ApCfg.ApCliTab[if_idx].Enable = FALSE;
				ApCliIfDown(pActionAd);
				pActionAd->ApCfg.ApCliTab[if_idx].Enable = TRUE;
			}
		} else {
			/* This interface will be enabled in ConWpsApCliBandMonitorTimer */
			pAd->ApCfg.ApCliTab[if_idx].Enable = FALSE;
			ApCliIfDown(pAd);
			RTMPSetTimer(&pAd->ApCfg.ConWpsApCliBandMonitorTimer, CON_WPS_APCLIENT_MONITOR_TIME);
			pAd->ApCfg.ConWpsMonitorTimerRunning = TRUE;
		}
		pWscControl->conWscStatus = CON_WPS_STATUS_DISABLED;
	} else
#endif /* MULTI_INF_SUPPORT */
	{
		if (pAd->ApCfg.ConWpsApCliMode == CON_WPS_APCLI_BAND_2G)
			pAd->ApCfg.ApCliTab[1].Enable = FALSE;
		else
			pAd->ApCfg.ApCliTab[0].Enable = FALSE;

		ApCliIfDown(pAd);
		RTMPSetTimer(&pAd->ApCfg.ConWpsApCliBandMonitorTimer, CON_WPS_APCLIENT_MONITOR_TIME);
		pAd->ApCfg.ConWpsMonitorTimerRunning = TRUE;
	}
}
#endif /* CON_WPS */
#endif /* APCLI_SUPPORT */


/*
*	==========================================================================
*	Description:
*		This is state machine function when receiving EAP packets
*		which is WPS Registration Protocol.
*
*		There are two roles at our AP, as an
*		1. Enrollee
*		2. Internal Registrar
*		3. Proxy
*
*		There are two roles at our Station, as an
*		1. Enrollee
*		2. External Registrar
*
*		Running Scenarios:
*		-----------------------------------------------------------------
*		1a. Adding an AP as an Enrollee to a station as an External Registrar (EAP)
*			[External Registrar]<----EAP--->[Enrollee_AP]
*		-----------------------------------------------------------------
*		2a. Adding a station as an Enrollee to an AP with built-in Registrar (EAP)
*			[Registrar_AP]<----EAP--->[Enrollee_STA]
*		-----------------------------------------------------------------
*		3a. Adding an Enrollee with External Registrar (UPnP/EAP)
*			[External Registrar]<----UPnP--->[Proxy_AP]<---EAP--->[Enrollee_STA]
*		-----------------------------------------------------------------
*
*	Return:
*		None
*	==========================================================================
*/
VOID WscEAPAction(
	IN  PRTMP_ADAPTER pAdapter,
	IN  MLME_QUEUE_ELEM * Elem)
{
	UCHAR MsgType;
	BOOLEAN bUPnPMsg, Cancelled;
	MAC_TABLE_ENTRY	*pEntry = NULL;
	UCHAR MacAddr[MAC_ADDR_LEN] = {0};
#ifdef CONFIG_AP_SUPPORT
	UCHAR apidx = MAIN_MBSSID;
	UCHAR current_band = 0;
	UCHAR bss_index = 0;
#endif /* CONFIG_AP_SUPPORT */
	PWSC_CTRL pWscControl = NULL;
	PWSC_UPNP_NODE_INFO pWscUPnPNodeInfo = NULL;
	UCHAR CurOpMode = 0xFF;
	struct wifi_dev *wdev = NULL;
	struct wifi_dev_ops *ops = NULL;

	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE,
		("-----> WscEAPAction\n"));
	/* The first 6 bytes in Elem->Msg is the MAC address of wps peer. */
	memmove(MacAddr, Elem->Msg, MAC_ADDR_LEN);
	memmove(Elem->Msg, Elem->Msg + 6, Elem->MsgLen);
#ifdef DBG
	hex_dump("(WscEAPAction)Elem->MsgLen", Elem->Msg, Elem->MsgLen);
#endif /* DBG */
	MsgType = WscRxMsgType(pAdapter, Elem);
	bUPnPMsg = Elem->MsgType == WSC_EAPOL_UPNP_MSG ? TRUE : FALSE;
	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE,
		("WscEAPAction: Addr: %02x:%02x:%02x:%02x:%02x:%02x, MsgType: 0x%02X, bUPnPMsg: %s\n",
		 PRINT_MAC(MacAddr), MsgType, bUPnPMsg ? "TRUE" : "FALSE"));
	wdev = Elem->wdev;
	ops = wdev->wdev_ops;
	pWscControl = &wdev->WscControl;

	if (!bUPnPMsg)
		ops->mac_entry_lookup(pAdapter, MacAddr, wdev, &pEntry);

	if (WDEV_WSC_AP(wdev))
		CurOpMode = AP_MODE;
	else {
		CurOpMode = STA_MODE;
	}

#ifdef CONFIG_AP_SUPPORT
	if (CurOpMode == AP_MODE) {
		if (!bUPnPMsg) {
			if (pEntry) {
				if (IS_ENTRY_CLIENT(pEntry) && pEntry->func_tb_idx >= pAdapter->ApCfg.BssidNum) {
					MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE,
						("WscEAPAction: Unknow apidex(=%d).\n", pEntry->func_tb_idx));
					MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE,
						("<----- WscEAPAction\n"));
					return;
				}
				apidx = pEntry->func_tb_idx;
				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE,
					("WscEAPAction: apidex=%d.\n", pEntry->func_tb_idx));
			} else {
				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE,
					("WscEAPAction: pEntry is NULL.\n"));
				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE,
					("<----- WscEAPAction\n"));
				return;
			}
		}
	}
#endif /* CONFIG_AP_SUPPORT */
	pWscUPnPNodeInfo = &pWscControl->WscUPnPNodeInfo;
	pWscUPnPNodeInfo->bUPnPMsgTimerPending = TRUE;

	if (pEntry && IS_ENTRY_CLIENT(pEntry)) {
		if ((MsgType == WSC_MSG_EAP_REG_RSP_ID)
			|| (MsgType == WSC_MSG_EAP_ENR_RSP_ID)) {
			if ((pEntry->Receive_EapolStart_EapRspId & WSC_ENTRY_GET_EAP_RSP_ID)
				&& (pWscControl->WscState > WSC_STATE_WAIT_M1)) {
				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE,
					("WscEAPAction: Already receive EAP_RSP(Identitry) from this STA, ignore it.\n"));
				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE,
					("<----- WscEAPAction\n"));
				return;
			}
			pEntry->Receive_EapolStart_EapRspId |= WSC_ENTRY_GET_EAP_RSP_ID;
		}
	}

	pWscControl->EapolTimerPending = TRUE;
#ifdef WSC_V2_SUPPORT

	if (MsgType == WSC_MSG_EAP_FRAG_ACK) {
		WscSendEapFragData(pAdapter, pWscControl, pEntry);
		return;
	}
#endif /* WSC_V2_SUPPORT */
	if (MsgType == WSC_MSG_EAP_REG_RSP_ID) {
		/* Receive EAP-Response/Id from external registrar, so the role of AP is enrollee. */
		if (((pWscControl->WscConfMode & WSC_ENROLLEE) != 0) ||
			(((pWscControl->WscConfMode & WSC_PROXY) != 0) && bUPnPMsg)) {
			pWscControl->WscActionMode = WSC_ENROLLEE;
			pWscControl->WscUseUPnP = bUPnPMsg ? 1 : 0;
			MsgType = WSC_MSG_EAP_RSP_ID;
			WscEapEnrolleeAction(pAdapter, Elem, WSC_MSG_EAP_RSP_ID, pEntry, pWscControl);
		}
	} else if (MsgType == WSC_MSG_EAP_ENR_RSP_ID) {
		/* Receive EAP-Response/Id from wps enrollee station, so the role of AP is Registrar or Proxy. */
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("WscEAPAction: Rx Identity\n"));
		pWscControl->WscActionMode = WSC_REGISTRAR;

		if (bUPnPMsg) {
			/* Receive enrollee identity from UPnP */
		} else {
#ifdef CONFIG_AP_SUPPORT
			/* Receive enrollee identity from EAP */
			if ((pWscControl->WscMode == WSC_PBC_MODE)
			   ) {
				/*
				*	Some WPS PBC Station select AP from UI directly; doesn't do PBC scan.
				*	Need to check DPID from STA again here.
				*/
				current_band = HcGetBandByChannel(pAdapter, Elem->Channel);

				WscPBC_DPID_FromSTA(pAdapter, pWscControl->EntryAddr, current_band);
				WscPBCSessionOverlapCheck(pAdapter, current_band);

				if ((pAdapter->CommonCfg.WscStaPbcProbeInfo.WscPBCStaProbeCount[current_band] == 1) &&
					!NdisEqualMemory(pAdapter->CommonCfg.WscStaPbcProbeInfo.StaMacAddr[current_band][0], &ZERO_MAC_ADDR[0], MAC_ADDR_LEN) &&
					(NdisEqualMemory(pAdapter->CommonCfg.WscStaPbcProbeInfo.StaMacAddr[current_band][0], &pWscControl->EntryAddr[0], 6) == FALSE)) {
					for (bss_index = 0; bss_index < pAdapter->ApCfg.BssidNum; bss_index++) {
						if ((current_band == HcGetBandByWdev(&pAdapter->ApCfg.MBSSID[bss_index].wdev)) &&
								pAdapter->ApCfg.MBSSID[bss_index].wdev.WscControl.WscConfMode != WSC_DISABLE &&
								pAdapter->ApCfg.MBSSID[bss_index].wdev.WscControl.bWscTrigger == TRUE) {
							MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
									("%s(): found pAd->ApCfg.MBSSID[%d] WPS on\n",
									__func__, bss_index));
							break;
						}
					}

					/*bss in current band has triggered wps pbc, so check Peer DPID*/
					if (bss_index < pAdapter->ApCfg.BssidNum) {
						MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
							("%s(): pAd->ApCfg.MBSSID[%d] WPS on, PBC Overlap detected\n",
							__func__, bss_index));
						pAdapter->CommonCfg.WscPBCOverlap = TRUE;
					} else {
						MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
							("%s(): pAd->ApCfg.MBSSID[%d] WPS off, PBC Overlap is invalid\n",
							__func__, bss_index));
						pAdapter->CommonCfg.WscPBCOverlap = FALSE;
					}
				}
				if (pAdapter->CommonCfg.WscPBCOverlap) {
					hex_dump("EntryAddr", pWscControl->EntryAddr, 6);
					hex_dump("StaMacAddr0", pAdapter->CommonCfg.WscStaPbcProbeInfo.StaMacAddr[current_band][0], 6);
					hex_dump("StaMacAddr1", pAdapter->CommonCfg.WscStaPbcProbeInfo.StaMacAddr[current_band][1], 6);
					hex_dump("StaMacAddr2", pAdapter->CommonCfg.WscStaPbcProbeInfo.StaMacAddr[current_band][2], 6);
					hex_dump("StaMacAddr3", pAdapter->CommonCfg.WscStaPbcProbeInfo.StaMacAddr[current_band][3], 6);
				}
			}
			if ((pWscControl->WscMode == WSC_PBC_MODE) &&
				(pAdapter->CommonCfg.WscPBCOverlap == TRUE)) {
				/* PBC session overlap */
				pWscControl->WscStatus = STATUS_WSC_PBC_SESSION_OVERLAP;
					RTMPSendWirelessEvent(pAdapter, IW_WSC_PBC_SESSION_OVERLAP, NULL, (pWscControl->EntryIfIdx & 0x0F), 0);

				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("WscEAPAction: PBC Session Overlap!\n"));
#if defined(VENDOR_FEATURE6_SUPPORT) || defined(VENDOR_FEATURE7_SUPPORT)
				WscSendUPnPMessage(pAdapter, (pWscControl->EntryIfIdx & 0x0F), WSC_OPCODE_UPNP_CTRL, WSC_UPNP_DATA_SUB_PBC_OVERLAP, &pAdapter->ApCfg.MBSSID[pWscControl->EntryIfIdx & 0x0F].wdev.bssid[0], MAC_ADDR_LEN, 0, 0, &pAdapter->ApCfg.MBSSID[pWscControl->EntryIfIdx & 0x0F].wdev.bssid[0], AP_MODE);
#endif /* VENDOR_FEATURE6_SUPPORT */
			} else
#endif /* CONFIG_AP_SUPPORT */
			if ((pWscControl->WscConfMode & WSC_PROXY_REGISTRAR) != 0) {
				/* Notify UPnP daemon before send Eap-Req(wsc-start) */
				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("%s: pEntry->Addr=%02x:%02x:%02x:%02x:%02x:%02x\n",
						 __func__, PRINT_MAC(pEntry->Addr)));
#ifdef CONFIG_AP_SUPPORT
				if (CurOpMode == AP_MODE) {
					if (pEntry)
						WscSendUPnPConfReqMsg(pAdapter, (pWscControl->EntryIfIdx & 0x0F),
											  (PUCHAR)pAdapter->ApCfg.MBSSID[pEntry->func_tb_idx].Ssid, pEntry->Addr, 2, 0, CurOpMode);

					/* Reset the UPnP timer and status. */
					if (pWscControl->bM2DTimerRunning == TRUE) {
						RTMPCancelTimer(&pWscControl->M2DTimer, &Cancelled);
						pWscControl->bM2DTimerRunning = FALSE;
					}

					pWscControl->WscUPnPNodeInfo.registrarID = 0;
					pWscControl->M2DACKBalance = 0;
				}
#endif /* CONFIG_AP_SUPPORT */
				pWscControl->EapMsgRunning = TRUE;
				/* Change the state to next one */
				pWscControl->WscState = WSC_STATE_WAIT_M1;

				/* send EAP WSC_START */
				if (pEntry && IS_ENTRY_CLIENT(pEntry)) {
					pWscControl->bWscLastOne = TRUE;
					if (CurOpMode == AP_MODE)
						WscSendMessage(pAdapter, WSC_OPCODE_START, NULL, 0, pWscControl, AP_MODE, EAP_CODE_REQ);
				}
			}
		}
	} else if (MsgType == WSC_MSG_EAP_REQ_ID) {
		/* Receive EAP_Req/Identity from WPS AP or WCN */
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("Receive EAP_Req/Identity from WPS AP or WCN\n"));

		if (bUPnPMsg && (pWscControl->WscConfMode == WSC_ENROLLEE)) {
			pWscControl->WscActionMode = WSC_ENROLLEE;
			pWscControl->WscUseUPnP = 1;
			WscEapEnrolleeAction(pAdapter, Elem, WSC_MSG_EAP_REQ_START, pEntry, pWscControl);
		} else {
			/* Receive EAP_Req/Identity from WPS AP */
			if (pEntry != NULL)
				WscSendEapRspId(pAdapter, pEntry, pWscControl);
		}

		if (!bUPnPMsg) {
			if ((pWscControl->WscState < WSC_STATE_WAIT_M1) ||
				(pWscControl->WscState > WSC_STATE_WAIT_ACK)) {
				if (pWscControl->WscConfMode == WSC_REGISTRAR)
					pWscControl->WscState = WSC_STATE_WAIT_M1;
				else
					pWscControl->WscState = WSC_STATE_WAIT_WSC_START;
			}
		}
	} else if (MsgType == WSC_MSG_EAP_REQ_START) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("Receive EAP_Req(Wsc_Start) from WPS AP\n"));

		/* Receive EAP_Req(Wsc_Start) from WPS AP */
		if (pWscControl->WscConfMode == WSC_ENROLLEE) {
			pWscControl->WscActionMode = WSC_ENROLLEE;
			pWscControl->WscUseUPnP = bUPnPMsg ? 1 : 0;
			WscEapEnrolleeAction(pAdapter, Elem, WSC_MSG_EAP_REQ_START, pEntry, pWscControl);

			if (!pWscControl->EapolTimerRunning) {
				pWscControl->EapolTimerRunning = TRUE;
				RTMPSetTimer(&pWscControl->EapolTimer, WSC_EAP_ID_TIME_OUT);
			}
		} else
			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("Ignore EAP_Req(Wsc_Start) from WPS AP\n"));
	} else if (MsgType == WSC_MSG_EAP_FAIL) {
		/* Receive EAP_Fail from WPS AP */
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("Receive EAP_Fail from WPS AP\n"));

		if (pWscControl->WscState >= WSC_STATE_WAIT_EAPFAIL) {
			pWscControl->WscState = WSC_STATE_OFF;
#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT

			if ((CurOpMode == STA_MODE) &&
				(wdev->wdev_type == WDEV_TYPE_APCLI)) {
				UCHAR if_idx = (pWscControl->EntryIfIdx & 0x0F);

			pWscControl->WscConfMode = WSC_DISABLE;


#ifdef CON_WPS
			if (pWscControl->conWscStatus != CON_WPS_STATUS_DISABLED) {
				if  (pAdapter->ApCfg.ConWpsApCliMode == CON_WPS_APCLI_BAND_AUTO) {
					WscConWpsStop(pAdapter, TRUE, pWscControl);
					pWscControl->conWscStatus = CON_WPS_STATUS_DISABLED;
				} else {
					PRTMP_ADAPTER pOpposAd = NULL;
#ifdef MULTI_INF_SUPPORT
					UINT nowBandIdx, opposBandIdx;

					nowBandIdx = multi_inf_get_idx(pAdapter);
					opposBandIdx = !nowBandIdx;
					pOpposAd = (PRTMP_ADAPTER)adapt_list[opposBandIdx];
#endif /* MULTI_INF_SUPPORT */
					pAdapter->ApCfg.ConWpsApCliStatus = TRUE;
					WscConWPSChecking(pAdapter, pOpposAd, if_idx, pWscControl);
				}
			}

#endif /* CON_WPS */
				/* Bring apcli interface down first */
				if (pEntry && IS_ENTRY_APCLI(pEntry) && pAdapter->ApCfg.ApCliTab[if_idx].Enable == TRUE
#ifdef CON_WPS
					&& (pAdapter->ApCfg.ConWpsApCliMode == CON_WPS_APCLI_BAND_AUTO)
#endif /* CON_WPS */
				   ) {
					pAdapter->ApCfg.ApCliTab[if_idx].Enable = FALSE;
					ApCliIfDown(pAdapter);
#if  defined(CONFIG_MAP_SUPPORT)
					if (!IS_MAP_TURNKEY_ENABLE(pAdapter))
#endif
					pAdapter->ApCfg.ApCliTab[if_idx].Enable = TRUE;
				}
			}
#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
		} else if (pWscControl->WscState == WSC_STATE_RX_M2D) {
			/* Wait M2; */
		} else if ((pWscControl->WscState <= WSC_STATE_WAIT_REQ_ID) &&
				   (pWscControl->WscState != WSC_STATE_FAIL)) {
			/* Ignore. D-Link DIR-628 AP sometimes would send EAP_Fail to station after Link UP first then send EAP_Req/Identity. */
		} else {
			pWscControl->WscStatus = STATUS_WSC_FAIL;
			RTMPSendWirelessEvent(pAdapter, IW_WSC_STATUS_FAIL, NULL, (pWscControl->EntryIfIdx & 0x0F), 0);
			pWscControl->WscConfMode = WSC_DISABLE;
			/* Change the state to next one */
			pWscControl->WscState = WSC_STATE_OFF;
		}
	} else if (MsgType == WSC_MSG_M1) {
		UINT32 rv = 0;

		/*
		*	If Buffalo WPS STA doesn't receive M2D from AP, Buffalo WPS STA will stop to do WPS.
		*	Therefore we need to receive M1 and send M2D without trigger.
		*/
		if ((pWscControl->WscConfMode & WSC_REGISTRAR) != 0) {
			pWscControl->WscActionMode = WSC_REGISTRAR;

			/* If Message is from EAP, but UPnP Registrar is in progress now, ignore EAP_M1 */
			if (!bUPnPMsg && pWscControl->WscUPnPNodeInfo.bUPnPInProgress) {
				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE,
					("UPnP Registrar is working now, ignore EAP M1.\n"));
				goto out;
			} else {
				if (pEntry)
					WscEapRegistrarAction(pAdapter, Elem, MsgType, pEntry, pWscControl);
			}

			rv = 1;
		}
#ifdef CONFIG_AP_SUPPORT
		if (((pWscControl->WscConfMode & WSC_PROXY) != 0) && (!bUPnPMsg) && (CurOpMode == AP_MODE)) {
			if ((pWscControl->bWscTrigger
				)
				&& (pWscControl->WscState >= WSC_STATE_WAIT_M3))
				;
			else {
					pWscControl->WscActionMode = WSC_PROXY;
					WscEapApProxyAction(pAdapter, Elem, MsgType, pEntry, pWscControl);
			}
		} else if ((!pWscControl->bWscTrigger) && ((pWscControl->WscConfMode & WSC_PROXY) == 0) && (pAdapter->OpMode == OPMODE_AP)) {
			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("WscTrigger is FALSE, ignore EAP M1.\n"));
			goto out;
		}
#endif /* CONFIG_AP_SUPPORT */
	} else if (MsgType == WSC_MSG_M3 ||
		MsgType == WSC_MSG_M5 ||
		MsgType == WSC_MSG_M7 ||
		MsgType == WSC_MSG_WSC_DONE) {
		BOOLEAN bNonceMatch = WscCheckNonce(pAdapter, Elem, TRUE, pWscControl);

		if (((pWscControl->WscConfMode & WSC_REGISTRAR) != 0) &&
			(pWscControl->bWscTrigger
			) &&
			bNonceMatch) {
			/* If Message is from EAP, but UPnP Registrar is in progress now, ignore EAP Messages */
			if (!bUPnPMsg && pWscControl->WscUPnPNodeInfo.bUPnPInProgress) {
				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("UPnP Registrar is working now, ignore EAP Messages.\n"));
				goto out;
			} else {
				pWscControl->WscActionMode = WSC_REGISTRAR;
				if (pEntry)
					WscEapRegistrarAction(pAdapter, Elem, MsgType, pEntry, pWscControl);
			}
		}
#ifdef CONFIG_AP_SUPPORT
		else if (((pWscControl->WscConfMode & WSC_PROXY) != 0) && (!bUPnPMsg) && (CurOpMode == AP_MODE)) {
			pWscControl->WscActionMode = WSC_PROXY;
			WscEapApProxyAction(pAdapter, Elem, MsgType, pEntry, pWscControl);
		}
#endif /* CONFIG_AP_SUPPORT */
	} else if (MsgType == WSC_MSG_M2 ||
		MsgType == WSC_MSG_M2D ||
		MsgType == WSC_MSG_M4 ||
		MsgType == WSC_MSG_M6 ||
		MsgType == WSC_MSG_M8) {
		BOOLEAN bNonceMatch = WscCheckNonce(pAdapter, Elem, FALSE, pWscControl);
		BOOLEAN bGoWPS = FALSE;

		if ((CurOpMode == AP_MODE) ||
		((CurOpMode == STA_MODE) &&
			(pWscControl->bWscTrigger
		 )))
		bGoWPS = TRUE;


#ifdef CONFIG_AP_SUPPORT
#ifdef WSC_V2_SUPPORT
		if ((CurOpMode == AP_MODE) &&
			((pWscControl->WscV2Info.bWpsEnable == FALSE) && (pWscControl->WscV2Info.bEnableWpsV2)))
			bGoWPS = FALSE;

#endif /* WSC_V2_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
		if (((pWscControl->WscConfMode & WSC_ENROLLEE) != 0) &&
			bGoWPS &&
			bNonceMatch) {
			pWscControl->WscActionMode = WSC_ENROLLEE;
			pWscControl->WscUseUPnP = bUPnPMsg ? 1 : 0;

			if (MsgType == WSC_MSG_M2) {
				BOOLEAN	bReadOwnPIN = TRUE;
#ifdef CONFIG_AP_SUPPORT
				/* WPS Enrollee AP only supports PIN without trigger */
				if (CurOpMode == AP_MODE) {
					if (pWscControl->bWscTrigger == FALSE) {
						pWscControl->WscMode = 1;
						WscGetConfWithoutTrigger(pAdapter, pWscControl, FALSE);
					} else if (!(pWscControl->EntryIfIdx & MIN_NET_DEVICE_FOR_APCLI)) {
						WscBuildBeaconIE(pAdapter,
									pWscControl->WscConfStatus,
									TRUE,
									pWscControl->WscMode,
									pWscControl->WscConfigMethods,
									(pWscControl->EntryIfIdx & 0x0F),
									NULL,
									0,
									AP_MODE);
						WscBuildProbeRespIE(pAdapter,
									WSC_MSGTYPE_AP_WLAN_MGR,
									pWscControl->WscConfStatus,
									TRUE,
									pWscControl->WscMode,
									pWscControl->WscConfigMethods,
									pWscControl->EntryIfIdx,
									NULL,
									0,
									AP_MODE);
						wdev = (struct wifi_dev *)pWscControl->wdev;
						UpdateBeaconHandler(
							pAdapter,
							wdev,
							BCN_UPDATE_IE_CHG);
					}
				}
#endif /* CONFIG_AP_SUPPORT */
				if (bReadOwnPIN) {
					pWscControl->WscPinCodeLen = pWscControl->WscEnrolleePinCodeLen;
					WscGetRegDataPIN(pAdapter, pWscControl->WscEnrolleePinCode, pWscControl);
					MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("(%d) WscEnrolleePinCode: %08u\n", bReadOwnPIN, pWscControl->WscEnrolleePinCode));
				} else
					MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("WscPinCode: %08u\n", pWscControl->WscPinCode));
			}
			/* If Message is from EAP, but UPnP Registrar is in progress now, ignore EAP Messages */
			if (!bUPnPMsg && pWscControl->WscUPnPNodeInfo.bUPnPInProgress) {
				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("UPnP Registrar is working now, ignore EAP Messages.\n"));
				goto out;
			} else
				WscEapEnrolleeAction(pAdapter, Elem, MsgType, pEntry, pWscControl);
		}
#ifdef CONFIG_AP_SUPPORT
		else if (((pWscControl->WscConfMode & WSC_PROXY) != 0) && (bUPnPMsg) && (CurOpMode == AP_MODE)) {
			pWscControl->WscActionMode = WSC_PROXY;
			WscEapApProxyAction(pAdapter, Elem, MsgType, pEntry, pWscControl);
		}
#endif /* CONFIG_AP_SUPPORT */
	} else if (MsgType == WSC_MSG_WSC_ACK) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("WscState: %d\n", pWscControl->WscState));
		if (((pWscControl->WscConfMode & WSC_REGISTRAR) != 0) &&
			pWscControl->WscState <= WSC_STATE_SENT_M2D) {
			if (WscCheckNonce(pAdapter, Elem, TRUE, pWscControl)) {
				if (pWscControl->M2DACKBalance > 0)
					pWscControl->M2DACKBalance--;

				pWscControl->WscState = WSC_STATE_INIT;
				pWscControl->EapMsgRunning = FALSE;
			}
		} else {
			if (((pWscControl->WscConfMode & WSC_ENROLLEE) != 0) &&
				WscCheckNonce(pAdapter, Elem, FALSE, pWscControl)) {
				pWscControl->WscActionMode = WSC_ENROLLEE;
				pWscControl->WscUseUPnP = bUPnPMsg ? 1 : 0;
				WscEapEnrolleeAction(pAdapter, Elem, MsgType, pEntry, pWscControl);
			}

#ifdef CONFIG_AP_SUPPORT
			else if (((pWscControl->WscConfMode & WSC_PROXY) != 0) && (CurOpMode == AP_MODE)) {
				pWscControl->WscActionMode = WSC_PROXY;
				WscEapApProxyAction(pAdapter, Elem, MsgType, pEntry, pWscControl);
			}

#endif /* CONFIG_AP_SUPPORT */
		}
	} else if (MsgType == WSC_MSG_WSC_NACK) {
		BOOLEAN bReSetWscIE = FALSE;

		if (bUPnPMsg) {
			if ((pWscControl->WscState == WSC_STATE_WAIT_M8) &&
				(pWscControl->WscConfStatus == WSC_SCSTATE_CONFIGURED)) {
				/* Some external sta will send NACK when AP is configured. */
				/* bWscTrigger should be set FALSE, otherwise Proxy will send NACK to enrollee. */
				pWscControl->bWscTrigger = FALSE;
				pWscControl->WscStatus = STATUS_WSC_CONFIGURED;
				bReSetWscIE = TRUE;
			}
			{
				int dataLen;
				UCHAR *pWscData;
				BOOLEAN bUPnPStatus = FALSE;

				os_alloc_mem(NULL, (UCHAR **)&pWscData, WSC_MAX_DATA_LEN);

				if (pWscData != NULL) {
					memset(pWscData, 0, WSC_MAX_DATA_LEN);
					dataLen = BuildMessageNACK(pAdapter, pWscControl, pWscData);
					bUPnPStatus = WscSendUPnPMessage(pAdapter, (pWscControl->EntryIfIdx & 0x0F), WSC_OPCODE_UPNP_DATA,
													 WSC_UPNP_DATA_SUB_NORMAL, pWscData, dataLen,
													 Elem->TimeStamp.u.LowPart, Elem->TimeStamp.u.HighPart,
													 &pAdapter->CurrentAddress[0], CurOpMode);
					os_free_mem(pWscData);

					if (bUPnPStatus == FALSE)
						WscUPnPErrHandle(pAdapter, pWscControl, Elem->TimeStamp.u.LowPart);
				}

				if (pWscUPnPNodeInfo->bUPnPMsgTimerRunning == TRUE) {
					RTMPCancelTimer(&pWscUPnPNodeInfo->UPnPMsgTimer, &Cancelled);
					pWscUPnPNodeInfo->bUPnPMsgTimerRunning = FALSE;
				}

				pWscUPnPNodeInfo->bUPnPInProgress = FALSE;
				RTMPSendWirelessEvent(pAdapter, IW_WSC_STATUS_FAIL, NULL, (pWscControl->EntryIfIdx & 0x0F), 0);
			}
		}
		if (!bUPnPMsg &&
			(WscCheckNonce(pAdapter, Elem, FALSE, pWscControl) || WscCheckNonce(pAdapter, Elem, TRUE, pWscControl))) {
			USHORT config_error = 0;

			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("Receive NACK from WPS client.\n"));
			WscGetConfigErrFromNack(pAdapter, Elem, &config_error);

			/*
			*If a PIN authentication or communication error occurs,
			*the Registrar MUST warn the user and MUST NOT automatically reuse the PIN.
			*Furthermore, if the Registrar detects this situation and prompts the user for a new PIN from the Enrollee device,
			*it MUST NOT accept the same PIN again without warning the user of a potential attack.
			*/
			if ((pWscControl->WscState >= WSC_STATE_WAIT_M5) && (config_error != WSC_ERROR_SETUP_LOCKED)) {
				pWscControl->WscRejectSamePinFromEnrollee = TRUE;
				WscDelListEntryByMAC(&pWscControl->WscPeerList, pEntry->Addr);

				if (pWscControl->WscState < WSC_STATE_WAIT_M8) {
					pWscControl->WscStatus = STATUS_WSC_FAIL;
					RTMPSendWirelessEvent(pAdapter, IW_WSC_STATUS_FAIL, NULL, (pWscControl->EntryIfIdx & 0x0F), 0);
					bReSetWscIE = TRUE;
				}
			}
#ifdef CONFIG_AP_SUPPORT
			if ((pWscControl->WscState == WSC_STATE_OFF)
				&& (CurOpMode == AP_MODE)
				&& (pWscControl->RegData.SelfInfo.ConfigError != WSC_ERROR_NO_ERROR))
				bReSetWscIE = TRUE;
#endif /* CONFIG_AP_SUPPORT */
			if ((pWscControl->WscState == WSC_STATE_WAIT_M8) &&
				(pWscControl->WscConfStatus == WSC_SCSTATE_CONFIGURED)) {
				/* Some external sta will send NACK when AP is configured. */
				/* bWscTrigger should be set FALSE, otherwise Proxy will send NACK to enrollee. */
				pWscControl->bWscTrigger = FALSE;
				bReSetWscIE = TRUE;
				pWscControl->WscStatus = STATUS_WSC_CONFIGURED;
				pWscControl->WscRejectSamePinFromEnrollee = FALSE;
				RTMPSendWirelessEvent(pAdapter, IW_WSC_STATUS_SUCCESS, NULL, (pWscControl->EntryIfIdx & 0x0F), 0);
			}
#ifdef CONFIG_AP_SUPPORT
			else if ((CurOpMode == AP_MODE) &&
					 (pWscControl->WscState == WSC_STATE_WAIT_DONE) &&
					 (pWscControl->WscConfStatus == WSC_SCSTATE_CONFIGURED) &&
					 (IS_CIPHER_WEP(pAdapter->ApCfg.MBSSID[apidx].wdev.SecConfig.PairwiseCipher))) {
				bReSetWscIE = TRUE;
				pWscControl->WscStatus = STATUS_WSC_FAIL;
			}
			if ((CurOpMode == AP_MODE) && bReSetWscIE && (!pWscControl->WscPinCode)) {
				UCHAR apidx = pWscControl->EntryIfIdx & 0x0F;
				struct wifi_dev *wdev = &pAdapter->ApCfg.MBSSID[apidx].wdev;

				WscBuildBeaconIE(pAdapter, pWscControl->WscConfStatus, FALSE, 0, 0, (pWscControl->EntryIfIdx & 0x0F), NULL, 0, CurOpMode);
				WscBuildProbeRespIE(pAdapter, WSC_MSGTYPE_AP_WLAN_MGR, pWscControl->WscConfStatus, FALSE, 0, 0, pWscControl->EntryIfIdx, NULL, 0, CurOpMode);
				UpdateBeaconHandler(
						pAdapter,
						wdev,
						BCN_UPDATE_IE_CHG);

				if (pWscControl->Wsc2MinsTimerRunning) {
					RTMPCancelTimer(&pWscControl->Wsc2MinsTimer, &Cancelled);
					pWscControl->Wsc2MinsTimerRunning = FALSE;
				}

				if (pWscControl->bWscTrigger)
					pWscControl->bWscTrigger = FALSE;
			}
#endif /* CONFIG_AP_SUPPORT */
			if ((CurOpMode == AP_MODE)
				|| ((ADHOC_ON(pAdapter)) && (pWscControl->WscConfMode == WSC_REGISTRAR))
			   ) {
				WscSendEapFail(pAdapter, pWscControl, TRUE);
				pWscControl->WscState = WSC_STATE_FAIL;
			}

			if (!pWscControl->WscPinCode) {
				RTMPCancelTimer(&pWscControl->EapolTimer, &Cancelled);
				pWscControl->EapolTimerRunning = FALSE;
				pWscControl->RegData.ReComputePke = 1;
			}
		}
	} else {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("Unsupported Msg Type (%02X)\n", MsgType));
		pWscControl->WscStatus = STATUS_WSC_FAIL;
		pWscControl->RegData.SelfInfo.ConfigError = WSC_ERROR_NO_ERROR;
		WscSendNACK(pAdapter, pEntry, pWscControl);
		RTMPSendWirelessEvent(pAdapter, IW_WSC_STATUS_FAIL, NULL, BSS0, 0);
		goto out;
	}

	if (bUPnPMsg) {
		/* Messages from UPnP */
		if (pWscUPnPNodeInfo->bUPnPMsgTimerRunning)
			RTMPModTimer(&pWscUPnPNodeInfo->UPnPMsgTimer, WSC_UPNP_MSG_TIME_OUT);
	} else {
		if ((pWscControl->EapMsgRunning == TRUE) &&
			(!RTMP_TEST_FLAG(pAdapter, fRTMP_ADAPTER_HALT_IN_PROGRESS |
							 fRTMP_ADAPTER_NIC_NOT_EXIST))) {
			/* Messages from EAP */
			RTMPModTimer(&pWscControl->EapolTimer, WSC_EAP_MSG_TIME_OUT);
			pWscControl->EapolTimerRunning = TRUE;
		}
	}

	if (bUPnPMsg && pWscControl->EapolTimerRunning) {
#ifdef CONFIG_AP_SUPPORT

		if ((pWscControl->WscActionMode == WSC_PROXY) && (CurOpMode == AP_MODE))
			RTMPModTimer(&pWscControl->EapolTimer, WSC_EAP_MSG_TIME_OUT);
		else
#endif /* CONFIG_AP_SUPPORT */
		{
			RTMPCancelTimer(&pWscControl->EapolTimer, &Cancelled);
			pWscControl->EapolTimerRunning = FALSE;
		}
	}

out:

	if (bUPnPMsg)
		pWscUPnPNodeInfo->bUPnPMsgTimerPending = FALSE;

	pWscControl->EapolTimerPending = FALSE;
	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("<----- WscEAPAction\n"));
}

#ifdef CONFIG_MAP_SUPPORT
void wsc_send_config_event_to_wapp(IN  PRTMP_ADAPTER pAdapter,
		IN PWSC_CTRL pWscControl, IN WSC_PROFILE * pWscProfile,
		IN MAC_TABLE_ENTRY * pEntry)
{
	UCHAR *msg;
	struct wifi_dev *wdev;
	struct wapp_event *event;
	int TotalLen = 0, i = 0;
	APCLI_STRUCT *pApCliTab;
	UCHAR CurApIdx = (pWscControl->EntryIfIdx & 0x0F);
	PWSC_CREDENTIAL pCredential;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					("%s:SEND Event to WAPP for WSC profile currAPIndex %d\n", __func__, CurApIdx));

	if (CurApIdx >= MAX_APCLI_NUM)
		return;

	pApCliTab = &pAdapter->ApCfg.ApCliTab[CurApIdx];
	TotalLen = sizeof(struct wapp_event);

	os_alloc_mem(NULL, (PUCHAR *)&msg, TotalLen);
	if (msg == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s:failed to allocated memory\n", __func__));
		return;
	}
	NdisZeroMemory(msg, TotalLen);
	event = (struct wapp_event *)msg;
	event->event_id = WAPP_MAP_WSC_CONFIG;
	wdev = &pApCliTab->wdev;
	event->ifindex = RtmpOsGetNetIfIndex(wdev->if_dev);
	for (i = 0; i < pWscProfile->ProfileCnt; i++)
	{
		pCredential = &pWscProfile->Profile[i];
		pCredential->DevPeerRole = pEntry->DevPeerRole;
	}

	RtmpOSWrielessEventSend(wdev->if_dev, RT_WLAN_EVENT_CUSTOM,
			OID_WAPP_EVENT, NULL, (PUCHAR)event, TotalLen);
	os_free_mem((PUCHAR)msg);
}
#endif


/*
*	============================================================================
*	Enrollee			Enrollee			Enrollee
*	============================================================================
*/
VOID WscEapEnrolleeAction(
	IN  PRTMP_ADAPTER pAdapter,
	IN  MLME_QUEUE_ELEM * Elem,
	IN  UCHAR MsgType,
	IN  MAC_TABLE_ENTRY *pEntry,
	IN  PWSC_CTRL pWscControl)
{
	INT DataLen = 0, rv = 0, DH_Len = 0;
	UCHAR OpCode, bssIdx;
	PUCHAR WscData = NULL;
	BOOLEAN bUPnPMsg, bUPnPStatus = FALSE, Cancelled;
	WSC_UPNP_NODE_INFO *pWscUPnPInfo = &pWscControl->WscUPnPNodeInfo;
	UINT MaxWscDataLen = WSC_MAX_DATA_LEN;
	UCHAR CurOpMode = 0xFF;
#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
	UCHAR ifIndex = Elem->Priv;
#endif
#endif
	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE,
		("WscEapEnrolleeAction Enter!\n"));
	bUPnPMsg = Elem->MsgType == WSC_EAPOL_UPNP_MSG ? TRUE : FALSE;
	OpCode = bUPnPMsg ? WSC_OPCODE_UPNP_MASK : 0;
	bssIdx = 0;
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAdapter)
	CurOpMode = AP_MODE;
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_AP_SUPPORT

	if (CurOpMode == AP_MODE) {
		/* Early check. */
		if ((pWscControl->WscActionMode != WSC_ENROLLEE) ||
			(pWscControl->WscUseUPnP && pEntry) ||
			((pWscControl->WscUseUPnP == 0) && (!pEntry))) {
			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE,
				("EarlyCheckFailed: pWscControl->WscActionMode=%d, Configured=%d, WscUseUPnP=%d, pEntry=%p!\n",
				 pWscControl->WscActionMode, pWscControl->WscConfStatus, pWscControl->WscUseUPnP, pEntry));
			goto Fail;
		}

		bssIdx = (pWscControl->EntryIfIdx & 0x0F);
	}

#endif /* CONFIG_AP_SUPPORT */
	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE,
		("MsgType=0x%x, WscState=%d, bUPnPMsg=%d!\n", MsgType, pWscControl->WscState, bUPnPMsg));

	if (bUPnPMsg) {
#ifdef CONFIG_AP_SUPPORT

		if ((MsgType == WSC_MSG_EAP_RSP_ID) && (CurOpMode == AP_MODE)) {
			/* let it pass */
		} else
#endif /* CONFIG_AP_SUPPORT */
		if (MsgType == WSC_MSG_M2 && pWscUPnPInfo->bUPnPInProgress == FALSE) {
#ifdef CONFIG_AP_SUPPORT
			if (CurOpMode == AP_MODE) {
				MAC_TABLE_ENTRY *tempEntry;

				tempEntry = MacTableLookup(pAdapter, &pWscControl->EntryAddr[0]);
				if (tempEntry) {
					if ((tempEntry->Receive_EapolStart_EapRspId & WSC_ENTRY_GET_EAP_RSP_ID) == WSC_ENTRY_GET_EAP_RSP_ID)
						goto Done;
				}
					/* else cannot find the pEntry, so we need to handle this msg. */
			}
#endif /* CONFIG_AP_SUPPORT */
			pWscUPnPInfo->bUPnPInProgress = TRUE;
			/* Set the WscState as "WSC_STATE_WAIT_RESP_ID" because UPnP start from this state. */
			/* pWscControl->WscState = WSC_STATE_WAIT_RESP_ID; */
			RTMPSetTimer(&pWscUPnPInfo->UPnPMsgTimer, WSC_UPNP_MSG_TIME_OUT);
			pWscUPnPInfo->bUPnPMsgTimerRunning = TRUE;
		} else {
			/* For other messages, we must make sure pWscUPnPInfo->bUPnPInProgress== TRUE */
			if (pWscUPnPInfo->bUPnPInProgress == FALSE)
				goto Done;
		}
	}

#ifdef WSC_V2_SUPPORT
	MaxWscDataLen = MaxWscDataLen + (UINT)pWscControl->WscV2Info.ExtraTlv.TlvLen;
#endif /* WSC_V2_SUPPORT */
	os_alloc_mem(NULL, (UCHAR **)&WscData, MaxWscDataLen);

	if (WscData == NULL) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE,
			("WscData Allocate failed!\n"));
		goto Fail;
	}

	NdisZeroMemory(WscData, MaxWscDataLen);

	switch (MsgType) {
	case WSC_MSG_EAP_RSP_ID:
	case WSC_MSG_EAP_REQ_START:
		if (MsgType == WSC_MSG_EAP_RSP_ID)
			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE,
			("WscEapEnrolleeAction : Rx Identity(ReComputePke=%d)\n", pWscControl->RegData.ReComputePke));
		if (MsgType == WSC_MSG_EAP_REQ_START)
			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE,
			("WscEapEnrolleeAction : Rx Wsc_Start(ReComputePke=%d)\n", pWscControl->RegData.ReComputePke));

		{
#ifdef CONFIG_AP_SUPPORT
			/*
			*	We don't need to consider P2P case.
			*/
			IF_DEV_CONFIG_OPMODE_ON_AP(pAdapter) {
				if ((pWscControl->bWscAutoTriggerDisable == TRUE) &&
					(pWscControl->bWscTrigger == FALSE)) {
					if (bUPnPMsg == TRUE) {
						MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE,
							("%s(%d): WscAutoTrigger is disabled.\n", __func__, __LINE__));
						WscUPnPErrHandle(pAdapter, pWscControl, Elem->TimeStamp.u.LowPart);
						os_free_mem(WscData);
						return;
					} else if (pEntry && IS_ENTRY_CLIENT(pEntry)) {
						MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE,
							("%s(%d): WscAutoTrigger is disabled! Send EapFail to STA.\n", __func__, __LINE__));
						WscSendEapFail(pAdapter, pWscControl, TRUE);
						os_free_mem(WscData);
						return;
					}
				}
			}
#endif /* CONFIG_AP_SUPPORT */

			if (pWscControl->RegData.ReComputePke == 1) {
				INT idx;

				DH_Len = sizeof(pWscControl->RegData.Pke);

				/* Enrollee 192 random bytes for DH key generation */
				for (idx = 0; idx < 192; idx++)
					pWscControl->RegData.EnrolleeRandom[idx] = RandomByte(pAdapter);

				NdisZeroMemory(pWscControl->RegData.Pke, sizeof(pWscControl->RegData.Pke));
				RT_DH_PublicKey_Generate(
					WPS_DH_G_VALUE, sizeof(WPS_DH_G_VALUE),
					WPS_DH_P_VALUE, sizeof(WPS_DH_P_VALUE),
					pWscControl->RegData.EnrolleeRandom, sizeof(pWscControl->RegData.EnrolleeRandom),
					pWscControl->RegData.Pke, (UINT *) &DH_Len);

				/* Need to prefix zero padding */
				if ((DH_Len != sizeof(pWscControl->RegData.Pke)) &&
					(DH_Len < sizeof(pWscControl->RegData.Pke))) {
					UCHAR TempKey[192];
					INT DiffCnt;

					DiffCnt = sizeof(pWscControl->RegData.Pke) - DH_Len;
					NdisFillMemory(&TempKey, DiffCnt, 0);
					NdisCopyMemory(&TempKey[DiffCnt], pWscControl->RegData.Pke, DH_Len);
					NdisCopyMemory(pWscControl->RegData.Pke, TempKey, sizeof(TempKey));
					DH_Len += DiffCnt;
					MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: Do zero padding!\n", __func__));
				}

				pWscControl->RegData.ReComputePke = 0;
			}
		}

		OpCode |= WSC_OPCODE_MSG;
		DataLen = BuildMessageM1(pAdapter, pWscControl, WscData);

		if (!bUPnPMsg) {
			pWscControl->EapMsgRunning = TRUE;
			pWscControl->WscStatus = STATUS_WSC_EAP_M1_SENT;
		} else
			/* Sometime out-of-band registrars (ex: Vista) get M1 for collecting information of device. */
			pWscControl->WscStatus = STATUS_WSC_IDLE;

		/* Change the state to next one */
		if (pWscControl->WscState < WSC_STATE_SENT_M1)
			pWscControl->WscState = WSC_STATE_SENT_M1;

		RTMPSendWirelessEvent(pAdapter, IW_WSC_SEND_M1, NULL, (pWscControl->EntryIfIdx & 0x0F), 0);
		break;

	case WSC_MSG_M2:
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("WscEapEnrolleeAction : Rx M2\n"));

		/* Receive M2, if we are at WSC_STATE_WAIT_M2 start, process it immediately */
		if (pWscControl->WscState == WSC_STATE_SENT_M1 ||
			pWscControl->WscState == WSC_STATE_RX_M2D) {
			/* Process M2 */
			pWscControl->WscStatus = STATUS_WSC_EAP_M2_RECEIVED;
			NdisMoveMemory(pWscControl->RegData.PeerInfo.MacAddr, pWscControl->EntryAddr, 6);
			rv = ProcessMessageM2(pAdapter, pWscControl, Elem->Msg, Elem->MsgLen, (pWscControl->EntryIfIdx & 0x0F), &pWscControl->RegData);
			if (rv)
				goto Fail;
			else {
#ifdef CONFIG_AP_SUPPORT
#ifdef WSC_V2_SUPPORT

				if ((CurOpMode == AP_MODE) && pWscControl->bSetupLock) {
					rv = WSC_ERROR_SETUP_LOCKED;
					goto Fail;
				}

#endif /* WSC_V2_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
				{
					OpCode |= WSC_OPCODE_MSG;
					DataLen = BuildMessageM3(pAdapter, pWscControl, WscData);
					pWscControl->WscStatus = STATUS_WSC_EAP_M3_SENT;
					/* Change the state to next one */
					pWscControl->WscState = WSC_STATE_WAIT_M4;
					RTMPSendWirelessEvent(pAdapter, IW_WSC_SEND_M3, NULL, (pWscControl->EntryIfIdx & 0x0F), 0);
				}
			}
		}

		break;

	case WSC_MSG_M2D:
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("WscEapEnrolleeAction : Rx M2D\n"));

		/* Receive M2D, if we are at WSC_STATE_WAIT_M2 start, process it immediately */
		if (pWscControl->WscState == WSC_STATE_SENT_M1 ||
			pWscControl->WscState == WSC_STATE_RX_M2D) {
			BOOLEAN bReplyNack = FALSE;

			rv = ProcessMessageM2D(pAdapter, Elem->Msg, Elem->MsgLen, &pWscControl->RegData);
			if (rv)
				goto Fail;

			pWscControl->WscStatus = STATUS_WSC_EAP_M2D_RECEIVED;

			if (CurOpMode == AP_MODE) {
				bReplyNack = TRUE;
#ifdef APCLI_SUPPORT

				if (pEntry && !IS_ENTRY_APCLI(pEntry))
					bReplyNack = TRUE;
				else
					bReplyNack = FALSE;

#endif
			}

			if (bReplyNack) {
				/* For VISTA SP1 internal registrar test */
				OpCode |= WSC_OPCODE_NACK;
				DataLen = BuildMessageNACK(pAdapter, pWscControl, WscData);
				RTMPSendWirelessEvent(pAdapter, IW_WSC_SEND_NACK, NULL, (pWscControl->EntryIfIdx & 0x0F), 0);
			} else {
				/* When external registrar is Marvell station, */
				/* wps station sends NACK may confuse or reset Marvell wps state machine. */
				OpCode |= WSC_OPCODE_ACK;
				DataLen = BuildMessageACK(pAdapter, pWscControl, WscData);
				RTMPSendWirelessEvent(pAdapter, IW_WSC_SEND_ACK, NULL, (pWscControl->EntryIfIdx & 0x0F), 0);
			}

			/* Change the state to next one */
			pWscControl->WscState = WSC_STATE_RX_M2D;
		}

		break;

	case WSC_MSG_M4:
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("WscEapEnrolleeAction : Rx M4\n"));

		/* Receive M4, if we are at WSC_STATE_WAIT_M4 start, process it immediately */
		if (pWscControl->WscState == WSC_STATE_WAIT_M4) {
			/* Process M4 */
			pWscControl->WscStatus = STATUS_WSC_EAP_M4_RECEIVED;
			rv = ProcessMessageM4(pAdapter, pWscControl, Elem->Msg, Elem->MsgLen, &pWscControl->RegData);
			if ((rv)) {
#ifdef CONFIG_AP_SUPPORT
#ifdef WSC_V2_SUPPORT

				if (CurOpMode == AP_MODE)
					WscCheckPinAttackCount(pAdapter, pWscControl);

#endif /* WSC_V2_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
				goto Fail;
			} else {
				OpCode |= WSC_OPCODE_MSG;
				DataLen = BuildMessageM5(pAdapter, pWscControl, WscData);
				pWscControl->WscStatus = STATUS_WSC_EAP_M5_SENT;
				/* Change the state to next one */
				pWscControl->WscState = WSC_STATE_WAIT_M6;
				RTMPSendWirelessEvent(pAdapter, IW_WSC_SEND_M5, NULL, (pWscControl->EntryIfIdx & 0x0F), 0);
			}
		}

		break;

	case WSC_MSG_M6:
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("WscEapEnrolleeAction : Rx M6\n"));

		/* Receive M6, if we are at WSC_STATE_WAIT_M6 start, process it immediately */
		if (pWscControl->WscState == WSC_STATE_WAIT_M6) {
			/* Process M6 */
			pWscControl->WscStatus = STATUS_WSC_EAP_M6_RECEIVED;
			rv = ProcessMessageM6(pAdapter, pWscControl, Elem->Msg, Elem->MsgLen, &pWscControl->RegData);
			if (rv) {
#ifdef CONFIG_AP_SUPPORT
#ifdef WSC_V2_SUPPORT
				if (CurOpMode == AP_MODE)
					WscCheckPinAttackCount(pAdapter, pWscControl);
#endif /* WSC_V2_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
				goto Fail;
			} else {
				OpCode |= WSC_OPCODE_MSG;
				DataLen = BuildMessageM7(pAdapter, pWscControl, WscData);
				pWscControl->WscStatus = STATUS_WSC_EAP_M7_SENT;
				/* Change the state to next one */
				pWscControl->WscState = WSC_STATE_WAIT_M8;
				RTMPSendWirelessEvent(pAdapter, IW_WSC_SEND_M7, NULL, (pWscControl->EntryIfIdx & 0x0F), 0);

				/*
				*	Complete WPS with this STA. Delete it from WscPeerList for others STA to do WSC with AP
				*/
				if (pEntry) {
					RTMP_SEM_LOCK(&pWscControl->WscPeerListSemLock);
					WscDelListEntryByMAC(&pWscControl->WscPeerList, pEntry->Addr);
					RTMP_SEM_UNLOCK(&pWscControl->WscPeerListSemLock);
				}
			}
		}
		break;

	case WSC_MSG_M8:
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("WscEapEnrolleeAction : Rx M8\n"));

		/* Receive M8, if we are at WSC_STATE_WAIT_M6 start, process it immediately */
		if (pWscControl->WscState == WSC_STATE_WAIT_M8) {
			/* Process M8 */
			pWscControl->WscStatus = STATUS_WSC_EAP_M8_RECEIVED;
			rv = ProcessMessageM8(pAdapter, Elem->Msg, Elem->MsgLen, pWscControl);
			if (rv)
				goto Fail;
			else {
				OpCode |= WSC_OPCODE_DONE;
				DataLen = BuildMessageDONE(pAdapter, pWscControl, WscData);
#ifdef CONFIG_AP_SUPPORT

				if (CurOpMode == AP_MODE) {
					/* Change the state to next one */
#ifdef APCLI_SUPPORT
					/* Ap Client only supports Inband(EAP)-Enrollee. */
					if (!bUPnPMsg && pEntry && IS_ENTRY_APCLI(pEntry))
						pWscControl->WscState = WSC_STATE_WAIT_EAPFAIL;
					else
#endif /* APCLI_SUPPORT */
						pWscControl->WscState = WSC_STATE_WAIT_ACK;

				}

#endif /* CONFIG_AP_SUPPORT */

#if defined(VENDOR_FEATURE6_SUPPORT) || defined(VENDOR_FEATURE7_SUPPORT)
				WscSendUPnPMessage(pAdapter, (pWscControl->EntryIfIdx & 0x0F), WSC_OPCODE_UPNP_CTRL, WSC_UPNP_DATA_SUB_M8, &pAdapter->ApCfg.MBSSID[pWscControl->EntryIfIdx].wdev.bssid[0], MAC_ADDR_LEN, 0, 0, &pAdapter->ApCfg.MBSSID[pWscControl->EntryIfIdx & 0x0F].wdev.bssid[0], AP_MODE);
#endif /* VENDOR_FEATURE6_SUPPORT */

				RTMPSendWirelessEvent(pAdapter, IW_WSC_SEND_DONE, NULL, (pWscControl->EntryIfIdx & 0x0F), 0);
#ifdef VENDOR_FEATURE7_SUPPORT
				{
					/* Send the configuration made by external registrar */
					UCHAR newApConfig[136] = {0};
					if (WMODE_CAP_5G(pAdapter->ApCfg.MBSSID[pWscControl->EntryIfIdx & 0x0F].wdev.PhyMode)) {
						sprintf((char *) newApConfig,
						"bss=%d&ssid=%s&auth=%d&enc=%d&key=%s&upnp=%d",
						(pWscControl->EntryIfIdx & 0x0F) + WIFI_50_RADIO,
						pWscControl->WscProfile.Profile[0].SSID.Ssid,
						pWscControl->WscProfile.Profile[0].AuthType,
						/* 1: open, 2: wpa-psk, 4: shared, 8:wpa, 0x10: wpa2, 0x20: wpa2-psk */
						pWscControl->WscProfile.Profile[0].EncrType,
						/* 1: none, 2: wep, 4: tkip, 8: aes */
						pWscControl->WscProfile.Profile[0].Key,
						pWscControl->WscProfile.Profile[0].bFromUPnP);
						/* TRUE: This credential is from external UPnP registrar */
					} else {
						sprintf((char *) newApConfig,
						"bss=%d&ssid=%s&auth=%d&enc=%d&key=%s&upnp=%d",
						(pWscControl->EntryIfIdx & 0x0F) + WIFI_24_RADIO,
						pWscControl->WscProfile.Profile[0].SSID.Ssid,
						pWscControl->WscProfile.Profile[0].AuthType,
						/* 1: open, 2: wpa-psk, 4: shared, 8:wpa, 0x10: wpa2, 0x20: wpa2-psk */
						pWscControl->WscProfile.Profile[0].EncrType,
						/* 1: none, 2: wep, 4: tkip, 8: aes */
						pWscControl->WscProfile.Profile[0].Key,
						pWscControl->WscProfile.Profile[0].bFromUPnP);
						/* TRUE: This credential is from external UPnP registrar */
					}
					/* RTMPSendWirelessEvent(pAdapter, IW_WSC_SEND_DONE,
					* newApConfig, (pWscControl->EntryIfIdx & 0x0F), 0);
					*/
					ARRISMOD_CALL(arris_event_send_hook, ATOM_HOST, WLAN_EVENT,
					WPS_AP_CONFIGURED, newApConfig, strlen(newApConfig));
				}
#endif
			}
		}

		break;
#ifdef CONFIG_AP_SUPPORT

	case WSC_MSG_WSC_ACK:
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("WscEapEnrolleeAction : Rx ACK\n"));

		/* Receive ACK */
		if (pWscControl->WscState == WSC_STATE_WAIT_ACK) {
			/* Process ACK */
			pWscControl->WscStatus = STATUS_WSC_EAP_RAP_RSP_ACK;
			/* Send out EAP-Fail */
			WscSendEapFail(pAdapter, pWscControl, FALSE);
			pWscControl->WscState = WSC_STATE_CONFIGURED;
			pWscControl->WscStatus = STATUS_WSC_CONFIGURED;
		}

		break;
#endif /* CONFIG_AP_SUPPORT */

	default:
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("WscEapEnrolleeAction : Unsupported Msg Type\n"));
		break;
	}

	if (bUPnPMsg) {
		if ((MsgType == WSC_MSG_M8) && (pWscControl->WscState == WSC_STATE_WAIT_ACK)) {
			pWscControl->EapMsgRunning = FALSE;
			pWscControl->WscState = WSC_STATE_CONFIGURED;
			pWscControl->WscStatus = STATUS_WSC_CONFIGURED;

			if (pWscUPnPInfo->bUPnPMsgTimerRunning == TRUE) {
				RTMPCancelTimer(&pWscUPnPInfo->UPnPMsgTimer, &Cancelled);
				pWscUPnPInfo->bUPnPMsgTimerRunning = FALSE;
			}

			pWscUPnPInfo->bUPnPInProgress = FALSE;
			pWscUPnPInfo->registrarID = 0;
		}
	} else {
		if (((MsgType == WSC_MSG_WSC_ACK) && (pWscControl->WscState == WSC_STATE_CONFIGURED)) ||
			((MsgType == WSC_MSG_M8) && (pWscControl->WscState == WSC_STATE_WAIT_ACK))) {
			RTMPCancelTimer(&pWscControl->EapolTimer, &Cancelled);
			pWscControl->EapolTimerRunning = FALSE;
			pWscControl->EapMsgRunning = FALSE;
			/*NdisZeroMemory(pWscControl->EntryAddr, MAC_ADDR_LEN); */
		}
	}

	if (OpCode > WSC_OPCODE_UPNP_MASK)
		bUPnPStatus = WscSendUPnPMessage(pAdapter, (pWscControl->EntryIfIdx & 0x0F), WSC_OPCODE_UPNP_DATA,
										 WSC_UPNP_DATA_SUB_NORMAL, WscData, DataLen,
										 Elem->TimeStamp.u.LowPart, Elem->TimeStamp.u.HighPart,
										 &pAdapter->CurrentAddress[0], CurOpMode);
	else if (OpCode > 0 && OpCode < WSC_OPCODE_UPNP_MASK) {
		if (pWscControl->WscState != WSC_STATE_CONFIGURED) {
#ifdef WSC_V2_SUPPORT
			pWscControl->WscTxBufLen = 0;
			pWscControl->pWscCurBufIdx = NULL;
			pWscControl->bWscLastOne = TRUE;

			if (pWscControl->bWscFragment && (DataLen > pWscControl->WscFragSize)) {
				ASSERT(DataLen < MAX_MGMT_PKT_LEN);
				NdisMoveMemory(pWscControl->pWscTxBuf, WscData, DataLen);
				pWscControl->WscTxBufLen = DataLen;
				NdisZeroMemory(WscData, DataLen);
				pWscControl->bWscLastOne = FALSE;
				pWscControl->bWscFirstOne = TRUE;
				NdisMoveMemory(WscData, pWscControl->pWscTxBuf, pWscControl->WscFragSize);
				DataLen = pWscControl->WscFragSize;
				pWscControl->WscTxBufLen -= pWscControl->WscFragSize;
				pWscControl->pWscCurBufIdx = (pWscControl->pWscTxBuf + pWscControl->WscFragSize);
			}

#endif /* WSC_V2_SUPPORT */
#ifdef CONFIG_AP_SUPPORT

			if (CurOpMode == AP_MODE) {
				if (pEntry && IS_ENTRY_APCLI(pEntry))
					WscSendMessage(pAdapter, OpCode, WscData, DataLen, pWscControl, AP_CLIENT_MODE, EAP_CODE_RSP);
				else
					WscSendMessage(pAdapter, OpCode, WscData, DataLen, pWscControl, AP_MODE, EAP_CODE_REQ);
			}

#endif /* CONFIG_AP_SUPPORT */
		}
	} else
		bUPnPStatus = TRUE;

Fail:
	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("WscEapEnrolleeAction : rv = %d\n", rv));

	if (rv) {
#ifdef CONFIG_AP_SUPPORT
#ifdef WSC_V2_SUPPORT

		if ((CurOpMode == AP_MODE) && pWscControl->bSetupLock)
			rv = WSC_ERROR_SETUP_LOCKED;

#endif /* WSC_V2_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

		if (rv <= WSC_ERROR_DEV_PWD_AUTH_FAIL)
			pWscControl->RegData.SelfInfo.ConfigError = rv;
		else if ((rv == WSC_ERROR_HASH_FAIL) || (rv == WSC_ERROR_HMAC_FAIL))
			pWscControl->RegData.SelfInfo.ConfigError = WSC_ERROR_DECRYPT_CRC_FAIL;

		switch (rv) {
		case WSC_ERROR_DEV_PWD_AUTH_FAIL:
			pWscControl->WscStatus = STATUS_WSC_ERROR_DEV_PWD_AUTH_FAIL;
			break;

		default:
			pWscControl->WscStatus = STATUS_WSC_FAIL;
			break;
		}

			RTMPSendWirelessEvent(pAdapter, IW_WSC_STATUS_FAIL, NULL, (pWscControl->EntryIfIdx & 0x0F), 0);
		if (bUPnPMsg) {
			if (pWscUPnPInfo->bUPnPMsgTimerRunning == TRUE) {
				RTMPCancelTimer(&pWscUPnPInfo->UPnPMsgTimer, &Cancelled);
				pWscUPnPInfo->bUPnPMsgTimerRunning = FALSE;
			}

			pWscUPnPInfo->bUPnPInProgress = FALSE;
		} else
			WscSendNACK(pAdapter, pEntry, pWscControl);

#ifdef CONFIG_AP_SUPPORT

		if (CurOpMode == AP_MODE) {
				pWscControl->WscState = WSC_STATE_OFF;
		}

#ifdef APCLI_SUPPORT
		if (pEntry && IS_ENTRY_APCLI(pEntry)) {
			/* Clear MAC table entry so that it should try again for WPS connection */
			MlmeEnqueue(pAdapter, APCLI_CTRL_STATE_MACHINE,
					APCLI_CTRL_PEER_DISCONNECT_REQ, 0, NULL, ifIndex);
		}
#endif

#endif /* CONFIG_AP_SUPPORT */
		/*NdisZeroMemory(pWscControl->EntryAddr, MAC_ADDR_LEN); */
		/*pWscControl->WscMode = 1; */
		bUPnPStatus = FALSE;
	}

Done:

	if (WscData)
		os_free_mem(WscData);

	if (bUPnPMsg && (bUPnPStatus == FALSE))
		WscUPnPErrHandle(pAdapter, pWscControl, Elem->TimeStamp.u.LowPart);

	rv = 0;
#ifdef CONFIG_AP_SUPPORT

	if  (CurOpMode == AP_MODE) {
		if (((bUPnPMsg || (pEntry && IS_ENTRY_CLIENT(pEntry)))
			 && (pWscControl->WscState == WSC_STATE_CONFIGURED || pWscControl->WscState == WSC_STATE_WAIT_ACK))
#ifdef APCLI_SUPPORT
			|| ((!bUPnPMsg && pEntry && IS_ENTRY_APCLI(pEntry)) && (pWscControl->WscState == WSC_STATE_WAIT_EAPFAIL || pWscControl->WscState == WSC_STATE_CONFIGURED))
#endif /* APCLI_SUPPORT */
		   )
			rv = 1;
	}

#endif /* CONFIG_AP_SUPPORT */

	if (rv == 1) {
#ifdef WSC_LED_SUPPORT
		UCHAR WPSLEDStatus;
#endif /* WSC_LED_SUPPORT */
		pWscControl->bWscTrigger = FALSE;
		pWscControl->RegData.ReComputePke = 1;
		RTMPCancelTimer(&pWscControl->EapolTimer, &Cancelled);

		if (pWscControl->Wsc2MinsTimerRunning) {
			pWscControl->Wsc2MinsTimerRunning = FALSE;
			RTMPCancelTimer(&pWscControl->Wsc2MinsTimer, &Cancelled);
		}


		if ((pWscControl->WscConfStatus == WSC_SCSTATE_UNCONFIGURED)
#ifdef CONFIG_AP_SUPPORT
			|| (pWscControl->bWCNTest == TRUE)
#ifdef WSC_V2_SUPPORT
			|| (pWscControl->WscV2Info.bEnableWpsV2 && ((CurOpMode == AP_MODE) && !pWscControl->bSetupLock))
#endif /* WSC_V2_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
		   ) {
			pWscControl->WscStatus = STATUS_WSC_CONFIGURED;
			pWscControl->WscConfStatus = WSC_SCSTATE_CONFIGURED;
			pWscControl->WscMode = 1;
			RTMPSendWirelessEvent(pAdapter, IW_WSC_STATUS_SUCCESS, NULL, (pWscControl->EntryIfIdx & 0x0F), 0);
#ifdef CONFIG_AP_SUPPORT

			if (CurOpMode == AP_MODE) {
				pWscControl->RegData.SelfInfo.ScState = pWscControl->WscConfStatus;
#ifdef APCLI_SUPPORT

				if (!bUPnPMsg && pEntry && IS_ENTRY_APCLI(pEntry)) {
					WscWriteConfToApCliCfg(pAdapter, pWscControl, &pWscControl->WscProfile.Profile[0], TRUE);
#ifdef CONFIG_MAP_SUPPORT
					wsc_send_config_event_to_wapp(pAdapter, pWscControl,
						&pWscControl->WscProfile, pEntry);
#endif
					RtmpOsTaskWakeUp(&(pAdapter->wscTask));
				} else
#endif /* APCLI_SUPPORT */
				{
					RTMPSetTimer(&pWscControl->WscUpdatePortCfgTimer, 1000);
					pWscControl->WscUpdatePortCfgTimerRunning = TRUE;
				}

				if (bUPnPMsg || (pEntry && IS_ENTRY_CLIENT(pEntry))) {
					UCHAR apidx = pWscControl->EntryIfIdx & 0x0F;
					struct wifi_dev *wdev = &pAdapter->ApCfg.MBSSID[apidx].wdev;

					WscBuildBeaconIE(pAdapter, WSC_SCSTATE_CONFIGURED, FALSE, 0, 0, (pWscControl->EntryIfIdx & 0x0F), NULL, 0, CurOpMode);
					WscBuildProbeRespIE(pAdapter, WSC_MSGTYPE_AP_WLAN_MGR, WSC_SCSTATE_CONFIGURED, FALSE, 0, 0, (pWscControl->EntryIfIdx & 0x0F), NULL, 0, CurOpMode);
					UpdateBeaconHandler(
						pAdapter,
						wdev,
						BCN_UPDATE_IE_CHG);
				}
			}

#endif /* CONFIG_AP_SUPPORT */
		}

#ifdef WSC_LED_SUPPORT
		/* The protocol is finished. */
		WPSLEDStatus = LED_WPS_SUCCESS;
		RTMPSetLED(pAdapter, WPSLEDStatus);
#endif /* WSC_LED_SUPPORT */
	}
}

#ifdef CONFIG_AP_SUPPORT
/*
*	============================================================================
*	Proxy			Proxy			Proxy
*	============================================================================
*/
VOID WscEapApProxyAction(
	IN  PRTMP_ADAPTER pAdapter,
	IN  MLME_QUEUE_ELEM * Elem,
	IN  UCHAR MsgType,
	IN  MAC_TABLE_ENTRY *pEntry,
	IN  PWSC_CTRL pWscControl)
{
	PUCHAR  WscData = NULL;
	BOOLEAN sendToUPnP = FALSE, bUPnPStatus = FALSE, Cancelled;
	int reqID = 0;
	WSC_UPNP_NODE_INFO *pWscUPnPInfo = &pWscControl->WscUPnPNodeInfo;
	UINT MaxWscDataLen = WSC_MAX_DATA_LEN;

	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("WscEapApProxyAction Enter!\n"));

	if (Elem->MsgType == WSC_EAPOL_UPNP_MSG) {
		reqID = Elem->TimeStamp.u.LowPart;

		if (reqID > 0)
			sendToUPnP = TRUE;
	}

	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("WscEapApProxyAction():pEntry=%p, ElemMsgType=%ld, MsgType=%d!\n", pEntry, Elem->MsgType, MsgType));

	if ((pWscControl->WscActionMode != WSC_PROXY) ||
		((Elem->MsgType == WSC_EAPOL_PACKET_MSG) && (pEntry == NULL))) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("EarlyCheckFailed: gWscActionMode=%d, pEntry=%p!\n", pWscControl->WscActionMode, pEntry));
		goto Fail;
	}

#ifdef WSC_V2_SUPPORT
	MaxWscDataLen = MaxWscDataLen + (UINT)pWscControl->WscV2Info.ExtraTlv.TlvLen;
#endif /* WSC_V2_SUPPORT */
	os_alloc_mem(NULL, (UCHAR **)&WscData, MaxWscDataLen);

	if (WscData == NULL) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("WscData Allocate failed!\n"));
		goto Fail;
	}

	NdisZeroMemory(WscData, MaxWscDataLen);

	/* Base on state doing the Msg, State change diagram */
	if (Elem->MsgType == WSC_EAPOL_UPNP_MSG) {
		/* WSC message send from UPnP. */
		switch (MsgType) {
		case WSC_MSG_M2:
		case WSC_MSG_M4:
		case WSC_MSG_M6:
		case WSC_MSG_M8:
			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("WscEapApProxyAction: Rx WscMsg(%d) from UPnP, eventID=0x%x!\n", MsgType, reqID));
			WscSendMessage(pAdapter, WSC_OPCODE_MSG, Elem->Msg, Elem->MsgLen, pWscControl, AP_MODE, EAP_CODE_REQ);

			/*Notify the UPnP daemon which remote registar is negotiating with enrollee. */
			if (MsgType == WSC_MSG_M2) {
				pWscUPnPInfo->registrarID = Elem->TimeStamp.u.HighPart;
				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("%s():registrarID=0x%x!\n", __func__, pWscUPnPInfo->registrarID));
				bUPnPStatus = WscSendUPnPMessage(pAdapter, (pWscControl->EntryIfIdx & 0x0F),
												 WSC_OPCODE_UPNP_MGMT, WSC_UPNP_MGMT_SUB_REG_SELECT,
												 (PUCHAR)(&pWscUPnPInfo->registrarID), sizeof(UINT), 0, 0, NULL, AP_MODE);

				/*Reset the UPnP timer and status. */
				if (pWscControl->bM2DTimerRunning == TRUE) {
					RTMPCancelTimer(&pWscControl->M2DTimer, &Cancelled);
					pWscControl->bM2DTimerRunning = FALSE;
				}

				pWscControl->M2DACKBalance = 0;
				pWscUPnPInfo->registrarID = 0;
			}

			if (MsgType == WSC_MSG_M8) {
				UCHAR apidx = pWscControl->EntryIfIdx & 0x0F;
				struct wifi_dev *wdev = &pAdapter->ApCfg.MBSSID[apidx].wdev;

				WscBuildBeaconIE(pAdapter, WSC_SCSTATE_CONFIGURED, FALSE, 0, 0, (pWscControl->EntryIfIdx & 0x0F), NULL, 0, AP_MODE);
				WscBuildProbeRespIE(pAdapter, WSC_MSGTYPE_AP_WLAN_MGR, WSC_SCSTATE_CONFIGURED, FALSE, 0, 0, pWscControl->EntryIfIdx, NULL, 0, AP_MODE);
				UpdateBeaconHandler(
					pAdapter,
					wdev,
					BCN_UPDATE_IE_CHG);
			}

			break;

		case WSC_MSG_M2D:
			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("WscEapApProxyAction: Rx WscMsg M2D(%d) from UPnP, eventID=0x%x!\n", MsgType, reqID));

			/*If it's send by UPnP Action, response ok directly to remote UPnP Control Point! */
			if (reqID > 0)
				bUPnPStatus = WscSendUPnPMessage(pAdapter, (pWscControl->EntryIfIdx & 0x0F),
												 WSC_OPCODE_UPNP_DATA, WSC_UPNP_DATA_SUB_ACK,
												 0, 0, reqID, 0, NULL, AP_MODE);

			/*Send M2D to wireless station. */
			WscSendMessage(pAdapter, WSC_OPCODE_MSG, Elem->Msg, Elem->MsgLen, pWscControl, AP_MODE, EAP_CODE_REQ);
			pWscControl->M2DACKBalance++;

			if ((pWscUPnPInfo->registrarID == 0) && (pWscControl->bM2DTimerRunning == FALSE)) {
				/* Add M2D timer used to trigger the EAPFail Packet! */
				RTMPSetTimer(&pWscControl->M2DTimer, WSC_UPNP_M2D_TIME_OUT);
				pWscControl->bM2DTimerRunning = TRUE;
			}

			break;

		case WSC_MSG_WSC_NACK:
		default:
			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("Recv WscMsg(%d) from UPnP, request EventID=%d! drop it!\n", MsgType, reqID));
			break;
		}
	} else {
		/*WSC msg send from EAP. */
		switch (MsgType) {
		case WSC_MSG_M1:
		case WSC_MSG_M3:
		case WSC_MSG_M5:
		case WSC_MSG_M7:
			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("WscEapApProxyAction: Rx WscMsg(%d) from EAP\n", MsgType));

			/*This msg send to event-based external registrar */
			if (MsgType == WSC_MSG_M1) {
				bUPnPStatus = WscSendUPnPMessage(pAdapter, (pWscControl->EntryIfIdx & 0x0F),
												 WSC_OPCODE_UPNP_DATA, WSC_UPNP_DATA_SUB_TO_ALL,
												 Elem->Msg, Elem->MsgLen, 0, 0, &pWscControl->EntryAddr[0], AP_MODE);
				pWscControl->WscState = WSC_STATE_SENT_M1;
			} else
				bUPnPStatus = WscSendUPnPMessage(pAdapter, (pWscControl->EntryIfIdx & 0x0F),
												 WSC_OPCODE_UPNP_DATA, WSC_UPNP_DATA_SUB_TO_ALL,
												 Elem->Msg, Elem->MsgLen, 0, pWscUPnPInfo->registrarID,
												 &pWscControl->EntryAddr[0], AP_MODE);

			break;

		case WSC_MSG_WSC_ACK:
			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("WscEapApProxyAction: Rx WSC_ACK from EAP\n"));

			/* The M2D must appeared before the ACK, so we just need sub it when (pWscUPnPInfo->M2DACKBalance > 0) */
			if (pWscControl->M2DACKBalance > 0)
				pWscControl->M2DACKBalance--;

			break;

		case WSC_MSG_WSC_DONE:
			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("WscEapApProxyAction: Rx WSC_DONE from EAP\n"));
			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("WscEapApProxyAction: send WSC_DONE to UPnP Registrar!\n"));
			/*Send msg to event-based external registrar */
			bUPnPStatus = WscSendUPnPMessage(pAdapter, (pWscControl->EntryIfIdx & 0x0F),
											 WSC_OPCODE_UPNP_DATA, WSC_UPNP_DATA_SUB_TO_ONE,
											 Elem->Msg, Elem->MsgLen, 0,
											 pWscUPnPInfo->registrarID, &pWscControl->EntryAddr[0], AP_MODE);
			/*Send EAPFail to wireless station to finish the whole process. */
			WscSendEapFail(pAdapter, pWscControl, FALSE);
			RTMPCancelTimer(&pWscControl->EapolTimer, &Cancelled);
			pWscControl->EapolTimerRunning = FALSE;
			pEntry->bWscCapable = FALSE;
			pWscControl->EapMsgRunning = FALSE;
			NdisZeroMemory(pWscControl->EntryAddr, MAC_ADDR_LEN);

			if (pWscControl->Wsc2MinsTimerRunning) {
				pWscControl->Wsc2MinsTimerRunning = FALSE;
				RTMPCancelTimer(&pWscControl->Wsc2MinsTimer, &Cancelled);
			}

			break;

		default:
			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("Recv WSC Msg(%d) from EAP , it's impossible, drop it!\n", MsgType));
			break;
		}
	}

Fail:

	if (WscData)
		os_free_mem(WscData);

	if (sendToUPnP && (bUPnPStatus == FALSE)) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("Need to send UPnP but bUPnPStatus is false!MsgType=%d, regID=0x%x!\n", MsgType, reqID));
		WscUPnPErrHandle(pAdapter, pWscControl, reqID);
	}
}
#endif /* CONFIG_AP_SUPPORT */

/*
*	============================================================================
*	Registrar			Registrar			Registrar
*	============================================================================
*/
VOID WscEapRegistrarAction(
	IN  PRTMP_ADAPTER pAdapter,
	IN  MLME_QUEUE_ELEM * Elem,
	IN  UCHAR MsgType,
	IN  MAC_TABLE_ENTRY *pEntry,
	IN  PWSC_CTRL pWscControl)
{
	INT DataLen = 0, rv = 0;
	UCHAR OpCode = 0;
	UCHAR *WscData = NULL;
	BOOLEAN bUPnPMsg, bUPnPStatus = FALSE, Cancelled;
	WSC_UPNP_NODE_INFO *pWscUPnPInfo = &pWscControl->WscUPnPNodeInfo;
	UINT MaxWscDataLen = WSC_MAX_DATA_LEN;
	UCHAR CurOpMode = 0xFF;
	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("WscEapRegistrarAction Enter!\n"));
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAdapter)
	CurOpMode = AP_MODE;
#endif /* CONFIG_AP_SUPPORT */
	bUPnPMsg = Elem->MsgType == WSC_EAPOL_UPNP_MSG ? TRUE : FALSE;

	if (bUPnPMsg) {
		if (MsgType == WSC_MSG_M1) {
			/* It's a M1 message, we may need to initialize our state machine. */
			if ((pWscControl->WscActionMode == WSC_REGISTRAR)
				&& (pWscControl->EntryIfIdx == WSC_INIT_ENTRY_APIDX)
				&& (pWscControl->WscState < WSC_STATE_WAIT_M1)
				&& (pWscUPnPInfo->bUPnPInProgress == FALSE)) {
				pWscUPnPInfo->bUPnPInProgress = TRUE;
				/*Set the WscState as "WSC_STATE_WAIT_RESP_ID" because UPnP start from this state. */
				pWscControl->WscState = WSC_STATE_WAIT_M1;
				RTMPSetTimer(&pWscUPnPInfo->UPnPMsgTimer, WSC_UPNP_MSG_TIME_OUT);
				pWscUPnPInfo->bUPnPMsgTimerRunning = TRUE;
			}
		}

		OpCode = WSC_OPCODE_UPNP_MASK;
	} else {
		if (pWscControl->EapolTimerRunning)
			pWscControl->EapolTimerRunning = FALSE;
	}

#ifdef WSC_V2_SUPPORT
	MaxWscDataLen = MaxWscDataLen + (UINT)pWscControl->WscV2Info.ExtraTlv.TlvLen;
#endif /* WSC_V2_SUPPORT */
	os_alloc_mem(NULL, (UCHAR **)&WscData, MaxWscDataLen);

	if (WscData == NULL) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("WscData Allocate failed!\n"));
		goto Fail;
	}

	NdisZeroMemory(WscData, MaxWscDataLen);

	/* Base on state doing the Msg, State change diagram */
	switch (MsgType) {
	case WSC_MSG_M1:
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("WscEapRegistrarAction : Rx M1\n"));
		/* Receive M1, if we are at WSC_STATE_WAIT_M1 start, process it immediately */
		pWscControl->WscStatus = STATUS_WSC_EAP_M1_RECEIVED;

		if (pWscControl->WscState == WSC_STATE_WAIT_M1) {
			OpCode |= WSC_OPCODE_MSG;
			/* Process M1 */
			rv = ProcessMessageM1(pAdapter, pWscControl, Elem->Msg, Elem->MsgLen, &pWscControl->RegData);
			if (rv)
				goto Fail;
			else {
				BOOLEAN	bSendM2D = TRUE;
#ifdef CONFIG_MAP_SUPPORT
				if (IS_MAP_ENABLE(pAdapter))
					pEntry->DevPeerRole = pWscControl->RegData.PeerInfo.map_DevPeerRole;
#endif /* CONFIG_MAP_SUPPORT */

				if (pWscControl->bWscTrigger && (!pWscControl->bWscAutoTigeer)) {
					if (((pWscControl->WscMode == WSC_PBC_MODE) || (pWscControl->WscMode == WSC_SMPBC_MODE))
						|| (pWscControl->WscMode == WSC_PIN_MODE && pWscControl->WscPinCode != 0))
						bSendM2D = FALSE;
				}


				if (bSendM2D) {
					DataLen = BuildMessageM2D(pAdapter, pWscControl, WscData);
					pWscControl->WscState = WSC_STATE_SENT_M2D;
					pWscControl->M2DACKBalance++;

					if (pWscControl->bM2DTimerRunning == FALSE) {
						/* Add M2D timer used to trigger the EAPFail Packet! */
						RTMPSetTimer(&pWscControl->M2DTimer, WSC_UPNP_M2D_TIME_OUT);
						pWscControl->bM2DTimerRunning = TRUE;
					}
				} else {
					pWscControl->WscStatus = STATUS_WSC_EAP_M2_SENT;
					DataLen = BuildMessageM2(pAdapter, pWscControl, WscData);
					/* Change the state to next one */
						pWscControl->WscState = WSC_STATE_WAIT_M3;

					RTMPSendWirelessEvent(pAdapter, IW_WSC_SEND_M2, NULL, (pWscControl->EntryIfIdx & 0x0F), 0);
				}
			}
		}
		break;
	case WSC_MSG_M3:
		/* Receive M3 */
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("WscEapRegistrarAction : Rx M3\n"));

		if (pWscControl->WscState == WSC_STATE_WAIT_M3) {
			pWscControl->WscStatus = STATUS_WSC_EAP_M3_RECEIVED;
			rv = ProcessMessageM3(pAdapter, Elem->Msg, Elem->MsgLen, &pWscControl->RegData);
			if (rv)
				goto Fail;
			else {
				OpCode |= WSC_OPCODE_MSG;
				DataLen = BuildMessageM4(pAdapter, pWscControl, WscData);
				pWscControl->WscStatus = STATUS_WSC_EAP_M4_SENT;
				/* Change the state to next one */
				pWscControl->WscState = WSC_STATE_WAIT_M5;
				RTMPSendWirelessEvent(pAdapter, IW_WSC_SEND_M4, NULL, (pWscControl->EntryIfIdx & 0x0F), 0);
			}
		}

		break;

	case WSC_MSG_M5:
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("WscEapRegistrarAction : Rx M5\n"));

		if (pWscControl->WscState == WSC_STATE_WAIT_M5) {
			pWscControl->WscStatus = STATUS_WSC_EAP_M5_RECEIVED;
			rv = ProcessMessageM5(pAdapter, pWscControl, Elem->Msg, Elem->MsgLen, &pWscControl->RegData);
			if (rv)
				goto Fail;
			else {
				OpCode |= WSC_OPCODE_MSG;
				DataLen = BuildMessageM6(pAdapter, pWscControl, WscData);
				pWscControl->WscStatus = STATUS_WSC_EAP_M6_SENT;
				/* Change the state to next one */
				pWscControl->WscState = WSC_STATE_WAIT_M7;
				RTMPSendWirelessEvent(pAdapter, IW_WSC_SEND_M6, NULL, (pWscControl->EntryIfIdx & 0x0F), 0);
			}
		}

		break;

	case WSC_MSG_M7:
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("WscEapRegistrarAction : Rx M7\n"));

		if (pWscControl->WscState == WSC_STATE_WAIT_M7) {
			pWscControl->WscStatus = STATUS_WSC_EAP_M7_RECEIVED;
			rv = ProcessMessageM7(pAdapter, pWscControl, Elem->Msg, Elem->MsgLen, &pWscControl->RegData);
			if (rv)
				goto Fail;
			else {
				if ((CurOpMode == AP_MODE)
					) {
					OpCode |= WSC_OPCODE_MSG;
					DataLen = BuildMessageM8(pAdapter, pWscControl, WscData);
					pWscControl->WscStatus = STATUS_WSC_EAP_M8_SENT;
					/* Change the state to next one */
					pWscControl->WscState = WSC_STATE_WAIT_DONE;
#if defined(VENDOR_FEATURE6_SUPPORT) || defined(VENDOR_FEATURE7_SUPPORT)
						/*Registrar case:*/
						{
							/*
							*	If APUT is Registra ,
							*	send event to wscd / miniupnpd , to let it query crendential and sync to system's flash & GUI.
							*/
							WscSendUPnPMessage(pAdapter, (pWscControl->EntryIfIdx & 0x0F), WSC_OPCODE_UPNP_CTRL, WSC_UPNP_DATA_SUB_WSC_DONE, &pAdapter->ApCfg.MBSSID[pWscControl->EntryIfIdx & 0x0F].wdev.bssid[0], MAC_ADDR_LEN, 0, 0, &pAdapter->ApCfg.MBSSID[pWscControl->EntryIfIdx & 0x0F].wdev.bssid[0], AP_MODE);
						}
#endif /* VENDOR_FEATURE6_SUPPORT */
#if defined(CONFIG_MAP_SUPPORT)
					if (IS_MAP_TURNKEY_ENABLE(pAdapter))
							wapp_send_wsc_eapol_complete_notif (
								pAdapter,
								pWscControl->wdev);
#endif
					RTMPSendWirelessEvent(pAdapter, IW_WSC_SEND_M8, NULL, (pWscControl->EntryIfIdx & 0x0F), 0);
#ifdef CONFIG_AP_SUPPORT
#ifdef WSC_V2_SUPPORT
					if (pWscControl->WscV2Info.bEnableWpsV2 && (CurOpMode == AP_MODE))
						WscAddEntryToAclList(pAdapter, pEntry->func_tb_idx, pEntry->Addr);
#endif /* WSC_V2_SUPPORT */

					/*
					*	1. Complete WPS with this STA. Delete it from WscPeerList for others STA to do WSC with AP
					*	2. Some WPS STA will send dis-assoc close to WSC_DONE
					*	   then AP will miss WSC_DONE from STA; hence we need to call WscDelListEntryByMAC here.
					*/
					if (pEntry && (CurOpMode == AP_MODE)) {
						RTMP_SEM_LOCK(&pWscControl->WscPeerListSemLock);
						WscDelListEntryByMAC(&pWscControl->WscPeerList, pEntry->Addr);
						RTMP_SEM_UNLOCK(&pWscControl->WscPeerListSemLock);
					}

#endif /* CONFIG_AP_SUPPORT */
				}

			}
		}

		break;

	case WSC_MSG_WSC_DONE:
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("WscEapRegistrarAction : Rx DONE\n"));

		if (pWscControl->WscState == WSC_STATE_WAIT_DONE) {
#ifdef CONFIG_AP_SUPPORT

			if (CurOpMode == AP_MODE) {
				pWscControl->WscStatus = STATUS_WSC_EAP_RAP_RSP_DONE_SENT;
				/* Send EAP-Fail */
				WscSendEapFail(pAdapter, pWscControl, FALSE);
				pWscControl->WscStatus = STATUS_WSC_CONFIGURED;
#ifdef CON_WPS

				/* AP: stop the other side of band */
				if (pWscControl->conWscStatus != CON_WPS_STATUS_DISABLED) {
					WscConWpsStop(pAdapter, FALSE, pWscControl);
					pWscControl->conWscStatus = CON_WPS_STATUS_DISABLED;
				}

#endif /* CON_WPS */
			}

#endif /* CONFIG_AP_SUPPORT */
			pWscControl->WscState = WSC_STATE_CONFIGURED;
			RTMPSendWirelessEvent(pAdapter, IW_WSC_STATUS_SUCCESS, NULL, (pWscControl->EntryIfIdx & 0x0F), 0);
#ifdef VENDOR_FEATURE7_SUPPORT
				{
					UCHAR wsc_done_event_msg[128] = {0};

					if (WMODE_CAP_5G(pAdapter->ApCfg.MBSSID[pWscControl->EntryIfIdx & 0x0F].wdev.PhyMode))
						sprintf((char *)wsc_done_event_msg, "WSC Done for Bss(%d)",
							(pWscControl->EntryIfIdx & 0x0F) + WIFI_50_RADIO);
					else
						sprintf((char *)wsc_done_event_msg, "WSC Done for Bss(%d)",
							(pWscControl->EntryIfIdx & 0x0F) + WIFI_24_RADIO);

					ARRISMOD_CALL(arris_event_send_hook, ATOM_HOST, WLAN_EVENT, WPS_AP_CONFIGURED,
						wsc_done_event_msg, strlen(wsc_done_event_msg));
				}
#endif
			pWscControl->EapMsgRunning = FALSE;
		}

		break;

	default:
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("WscEapRegistrarAction : Unsupported Msg Type\n"));

		if (WscData)
			os_free_mem(WscData);

		return;
	}

	if (OpCode > WSC_OPCODE_UPNP_MASK)
		bUPnPStatus = WscSendUPnPMessage(pAdapter, (pWscControl->EntryIfIdx & 0x0F),
										 WSC_OPCODE_UPNP_DATA, WSC_UPNP_DATA_SUB_NORMAL,
										 WscData, DataLen,
										 Elem->TimeStamp.u.LowPart, Elem->TimeStamp.u.HighPart, &pWscControl->EntryAddr[0], CurOpMode);
	else if (OpCode > 0 && OpCode < WSC_OPCODE_UPNP_MASK) {
#ifdef WSC_V2_SUPPORT
		pWscControl->WscTxBufLen = 0;
		pWscControl->pWscCurBufIdx = NULL;
		pWscControl->bWscLastOne = TRUE;

		if (pWscControl->bWscFragment && (DataLen > pWscControl->WscFragSize)) {
			ASSERT(DataLen < MAX_MGMT_PKT_LEN);
			NdisMoveMemory(pWscControl->pWscTxBuf, WscData, DataLen);
			pWscControl->WscTxBufLen = DataLen;
			NdisZeroMemory(WscData, DataLen);
			pWscControl->bWscLastOne = FALSE;
			pWscControl->bWscFirstOne = TRUE;
			NdisMoveMemory(WscData, pWscControl->pWscTxBuf, pWscControl->WscFragSize);
			DataLen = pWscControl->WscFragSize;
			pWscControl->WscTxBufLen -= pWscControl->WscFragSize;
			pWscControl->pWscCurBufIdx = (pWscControl->pWscTxBuf + pWscControl->WscFragSize);
		}

#endif /* WSC_V2_SUPPORT */
#ifdef CONFIG_AP_SUPPORT

		if (CurOpMode == AP_MODE) {
			if (pWscControl->WscState != WSC_STATE_CONFIGURED)
				WscSendMessage(pAdapter, OpCode, WscData, DataLen, pWscControl, AP_MODE, EAP_CODE_REQ);
		}

#endif /* CONFIG_AP_SUPPORT */
	} else
		bUPnPStatus = TRUE;

	if (bUPnPMsg) {
		if (pWscControl->WscState == WSC_STATE_SENT_M2D) {
			/*After M2D, reset the status of State Machine. */
			pWscControl->WscState = WSC_STATE_WAIT_UPNP_START;
			pWscUPnPInfo->bUPnPInProgress = FALSE;
		}
	}

Fail:
	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("WscEapRegistrarAction : rv = %d\n", rv));

	if (rv) {
		if (rv <= WSC_ERROR_DEV_PWD_AUTH_FAIL) {
			pWscControl->RegData.SelfInfo.ConfigError = rv;
		} else if ((rv == WSC_ERROR_HASH_FAIL) || (rv == WSC_ERROR_HMAC_FAIL))
			pWscControl->RegData.SelfInfo.ConfigError = WSC_ERROR_DECRYPT_CRC_FAIL;

		switch (rv) {
		case WSC_ERROR_HASH_FAIL:
			pWscControl->WscStatus = STATUS_WSC_ERROR_HASH_FAIL;
			break;

		case WSC_ERROR_HMAC_FAIL:
			pWscControl->WscStatus = STATUS_WSC_ERROR_HMAC_FAIL;
			break;

		default:
			pWscControl->WscStatus = STATUS_WSC_FAIL;
			break;
		}

			RTMPSendWirelessEvent(pAdapter, IW_WSC_STATUS_FAIL, NULL, (pWscControl->EntryIfIdx & 0x0F), 0);

		if (bUPnPMsg) {
			if (pWscUPnPInfo->bUPnPMsgTimerRunning == TRUE) {
				RTMPCancelTimer(&pWscUPnPInfo->UPnPMsgTimer, &Cancelled);
				pWscUPnPInfo->bUPnPMsgTimerRunning = FALSE;
			}

			pWscUPnPInfo->bUPnPInProgress = FALSE;
		} else {
			DataLen = BuildMessageNACK(pAdapter, pWscControl, WscData);
#ifdef CONFIG_AP_SUPPORT

			if (CurOpMode == AP_MODE) {
				WscSendMessage(pAdapter, WSC_OPCODE_NACK, WscData, DataLen, pWscControl, AP_MODE, EAP_CODE_REQ);
				pEntry->bWscCapable = FALSE;
			}

#endif /* CONFIG_AP_SUPPORT */
			RTMPCancelTimer(&pWscControl->EapolTimer, &Cancelled);
			pWscControl->EapolTimerRunning = FALSE;
		}

		/*
		*   If a PIN authentication or communication error occurs after sending message M6,
		*   the Registrar MUST warn the user and MUST NOT automatically reuse the PIN.
		*   Furthermore, if the Registrar detects this situation and prompts the user for a new PIN from the Enrollee device,
		*   it MUST NOT accept the same PIN again without warning the user of a potential attack.
		*/
		if (pWscControl->WscState >= WSC_STATE_WAIT_M7) {
			pWscControl->WscRejectSamePinFromEnrollee = TRUE;
			pWscControl->WscPinCode = 0;
		}

		pWscControl->WscState = WSC_STATE_OFF;
		pWscControl->WscStatus = STATUS_WSC_IDLE;
		/*NdisZeroMemory(pWscControl->EntryAddr, MAC_ADDR_LEN); */
		/*pWscControl->WscMode = 1; */
		bUPnPStatus = FALSE;
	}

	if (WscData)
		os_free_mem(WscData);

	if (bUPnPMsg && (bUPnPStatus == FALSE))
		WscUPnPErrHandle(pAdapter, pWscControl, Elem->TimeStamp.u.LowPart);

	if (pWscControl->WscState == WSC_STATE_CONFIGURED) {
#ifdef WSC_LED_SUPPORT
		UCHAR WPSLEDStatus;
#endif /* WSC_LED_SUPPORT */
		pWscControl->bWscTrigger = FALSE;

/* WPS_BandSteering Support */
#ifdef BAND_STEERING
	if (pAdapter->ApCfg.BandSteering) {

		int apidx = pWscControl->EntryIfIdx & 0x0F;
		struct wifi_dev *wdev = NULL;

		if (apidx < HW_BEACON_MAX_NUM)
			wdev = &pAdapter->ApCfg.MBSSID[apidx].wdev;

		if (wdev) {
			PBND_STRG_CLI_TABLE table = Get_BndStrgTable(pAdapter, wdev->func_idx);

			if (table && table->bEnabled) {
				NdisAcquireSpinLock(&table->WpsWhiteListLock);
				ClearWpsWhiteList(&table->WpsWhiteList);
				NdisReleaseSpinLock(&table->WpsWhiteListLock);
				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("%s:channel %u wps whitelist cleared, size : %d\n",
				 __func__, table->Channel, table->WpsWhiteList.size));
			}
		}
	}
#endif



		if (pWscControl->Wsc2MinsTimerRunning) {
			pWscControl->Wsc2MinsTimerRunning = FALSE;
			RTMPCancelTimer(&pWscControl->Wsc2MinsTimer, &Cancelled);
		}

		if (bUPnPMsg) {
			if (pWscUPnPInfo->bUPnPMsgTimerRunning == TRUE) {
				RTMPCancelTimer(&pWscUPnPInfo->UPnPMsgTimer, &Cancelled);
				pWscUPnPInfo->bUPnPMsgTimerRunning = FALSE;
			}

			pWscUPnPInfo->bUPnPInProgress = FALSE;
			pWscUPnPInfo->registrarID = 0;
		}

#ifdef CONFIG_AP_SUPPORT
		else {
			if (CurOpMode == AP_MODE) {
				UCHAR apidx = pEntry->func_tb_idx;
				struct wifi_dev *wdev = &pAdapter->ApCfg.MBSSID[apidx].wdev;

				WscBuildBeaconIE(pAdapter, WSC_SCSTATE_CONFIGURED, FALSE, 0, 0, pEntry->func_tb_idx, NULL, 0, CurOpMode);
				WscBuildProbeRespIE(pAdapter, WSC_MSGTYPE_AP_WLAN_MGR, WSC_SCSTATE_CONFIGURED, FALSE, 0, 0, pWscControl->EntryIfIdx, NULL, 0, CurOpMode);
				UpdateBeaconHandler(
					pAdapter,
					wdev,
					BCN_UPDATE_IE_CHG);
			}
		}

		NdisZeroMemory(&pAdapter->CommonCfg.WscStaPbcProbeInfo, sizeof(WSC_STA_PBC_PROBE_INFO));
#endif /* CONFIG_AP_SUPPORT */

		if (
			(
				(pWscControl->WscConfStatus == WSC_SCSTATE_UNCONFIGURED) &&
				((CurOpMode == AP_MODE) || (ADHOC_ON(pAdapter)))
			)
		) {
			pWscControl->WscConfStatus = WSC_SCSTATE_CONFIGURED;
#ifdef CONFIG_AP_SUPPORT

			if (CurOpMode == AP_MODE) {
				{
					/*
					*	Use ApplyProfileIdx to inform WscUpdatePortCfgTimer AP acts registrar.
					*/
					pWscControl->WscProfile.ApplyProfileIdx |= 0x8000;
					RTMPSetTimer(&pWscControl->WscUpdatePortCfgTimer, 1000);
					pWscControl->WscUpdatePortCfgTimerRunning = TRUE;
				}
			}

#endif /* CONFIG_AP_SUPPORT */
		}

#ifdef WSC_LED_SUPPORT
		/* The protocol is finished. */
		WPSLEDStatus = LED_WPS_SUCCESS;
		RTMPSetLED(pAdapter, WPSLEDStatus);
#endif /* WSC_LED_SUPPORT */
		{
			pWscControl->WscPinCode = 0;
			pWscControl->WscMode = 1;
		}

		RTMPCancelTimer(&pWscControl->EapolTimer, &Cancelled);
		pWscControl->EapolTimerRunning = FALSE;
		return;
	}
}

VOID WscTimeOutProcess(
	IN  PRTMP_ADAPTER pAd,
	IN  PMAC_TABLE_ENTRY pEntry,
	IN  INT nWscState,
	IN  PWSC_CTRL pWscControl)
{
	INT WscMode;
	UCHAR CurOpMode = 0xFF;
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	CurOpMode = AP_MODE;
#endif /* CONFIG_AP_SUPPORT */

	if (nWscState == WSC_STATE_WAIT_ACK)
		pWscControl->WscState = WSC_STATE_CONFIGURED;
	else if (nWscState == WSC_STATE_WAIT_RESP_ID)
		pWscControl->WscState = WSC_STATE_OFF;
	else if (nWscState == WSC_STATE_RX_M2D) {
#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
		/* At this stage APCLI WSC connection is not failed */
		/* Till WSC timer is over, it is not failed so setting */
		/* WscState fail will disable the WSC conf mode in WscEAPAction */
		if (CurOpMode == AP_MODE)
			if (pEntry && !(IS_ENTRY_APCLI(pEntry)))
#endif
#endif
				pWscControl->WscState = WSC_STATE_FAIL;
#ifdef CONFIG_AP_SUPPORT

		if (CurOpMode == AP_MODE) {
			if (pEntry && IS_ENTRY_CLIENT(pEntry))
				WscSendEapFail(pAd, pWscControl, TRUE);

#ifdef APCLI_SUPPORT

			if (pEntry && IS_ENTRY_APCLI(pEntry))
				WscApCliLinkDown(pAd, pWscControl);

#endif /* APCLI_SUPPORT */
		}

#endif /* CONFIG_AP_SUPPORT */
		pWscControl->EapolTimerRunning = FALSE;
		pWscControl->WscRetryCount = 0;
		return;
	} else if (nWscState == WSC_STATE_WAIT_EAPFAIL) {
		pWscControl->WscState = WSC_STATE_OFF;
		pWscControl->WscStatus = STATUS_WSC_CONFIGURED;
		pWscControl->WscConfMode = WSC_DISABLE;
	} else {
#ifdef CONFIG_AP_SUPPORT

		if ((pWscControl->WscActionMode == WSC_PROXY) && (pAd->OpMode == OPMODE_AP))
			pWscControl->WscState = WSC_STATE_OFF;
		else
#endif /* CONFIG_AP_SUPPORT */
			pWscControl->WscState = WSC_STATE_FAIL;
	}

	if (nWscState == WSC_STATE_WAIT_M8)
		pWscControl->bWscTrigger = FALSE;

	pWscControl->WscRetryCount = 0;
	NdisZeroMemory(pWscControl->EntryAddr, MAC_ADDR_LEN);
	pWscControl->EapolTimerRunning = FALSE;

	if (pWscControl->WscMode == 1)
		WscMode = DEV_PASS_ID_PIN;
	else
		WscMode = DEV_PASS_ID_PBC;

#ifdef CONFIG_AP_SUPPORT

	if (CurOpMode == AP_MODE) {
		if ((pWscControl->WscConfStatus == WSC_SCSTATE_UNCONFIGURED) &&
			((nWscState == WSC_STATE_WAIT_DONE) || (nWscState == WSC_STATE_WAIT_ACK))) {
			pWscControl->bWscTrigger = FALSE;
			pWscControl->WscConfStatus = WSC_SCSTATE_CONFIGURED;
			WscBuildBeaconIE(pAd, pWscControl->WscConfStatus, FALSE, WscMode, pWscControl->WscConfigMethods, (pWscControl->EntryIfIdx & 0x0F), NULL, 0, CurOpMode);
			WscBuildProbeRespIE(pAd, WSC_MSGTYPE_AP_WLAN_MGR, pWscControl->WscConfStatus, FALSE, WscMode, pWscControl->WscConfigMethods, pWscControl->EntryIfIdx, NULL, 0, CurOpMode);
			pAd->WriteWscCfgToDatFile = pWscControl->EntryIfIdx;
			WscWriteConfToPortCfg(pAd,
								  pWscControl,
								  &pWscControl->WscProfile.Profile[0],
								  FALSE);
			{
				UCHAR apidx = pWscControl->EntryIfIdx & 0x0F;
				/*
				*	Using CMD thread to prevent in-band command failed.
				*	@20150710
				*	Need to add RfIC to do ApStop/ApStart.
				*	@20160321
				*/
				RTEnqueueInternalCmd(pAd, CMDTHREAD_AP_RESTART, (VOID *)&apidx, sizeof(UCHAR));
			}

			RtmpOsTaskWakeUp(&(pAd->wscTask));
		} else {
			if (pEntry && IS_ENTRY_CLIENT(pEntry)) {
				pEntry->bWscCapable = FALSE;
				WscSendEapFail(pAd, pWscControl, TRUE);
			}

			WscBuildBeaconIE(pAd, pWscControl->WscConfStatus, FALSE, WscMode, pWscControl->WscConfigMethods, (pWscControl->EntryIfIdx & 0x0F), NULL, 0, CurOpMode);
			WscBuildProbeRespIE(pAd, WSC_MSGTYPE_AP_WLAN_MGR, pWscControl->WscConfStatus, FALSE, WscMode, pWscControl->WscConfigMethods, pWscControl->EntryIfIdx, NULL, 0, CurOpMode);
		}

#ifdef APCLI_SUPPORT

		if (pEntry && IS_ENTRY_APCLI(pEntry))
			WscApCliLinkDown(pAd, pWscControl);

#endif /* APCLI_SUPPORT */
	}

#endif /* CONFIG_AP_SUPPORT */
	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("WscTimeOutProcess\n"));
}

VOID WscEAPOLTimeOutAction(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	PUCHAR WscData = NULL;
	PMAC_TABLE_ENTRY pEntry = NULL;
	PWSC_CTRL pWscControl = NULL;
	PRTMP_ADAPTER pAd = NULL;
	UINT MaxWscDataLen = WSC_MAX_DATA_LEN;
	UCHAR CurOpMode = 0xFF;
	struct wifi_dev *wdev;
	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("-----> WscEAPOLTimeOutAction\n"));
	ASSERT(FunctionContext);

	if (FunctionContext == 0)
		return;

	pWscControl = (PWSC_CTRL)FunctionContext;
	pAd = (PRTMP_ADAPTER)pWscControl->pAd;
	ASSERT(pAd);
	wdev = (struct wifi_dev *)pWscControl->wdev;
	ASSERT(wdev);
	wdev->wdev_ops->mac_entry_lookup(pAd, pWscControl->EntryAddr, wdev, &pEntry);

	if (WDEV_WSC_AP(wdev))
		CurOpMode = AP_MODE;
	else {
		CurOpMode = STA_MODE;
	}

	if ((CurOpMode == AP_MODE) || ADHOC_ON(pAd)) {
		if (pEntry == NULL) {
#ifdef CONFIG_AP_SUPPORT

			/*
			*	Some WPS Client will send dis-assoc close to WSC_DONE.
			*	If AP misses WSC_DONE, WPS Client still sends dis-assoc to AP.
			*	AP driver needs to check wsc_state here for considering WPS process with this client is completed.
			*/
			if ((CurOpMode == AP_MODE) &&
				((pWscControl->WscState == WSC_STATE_WAIT_DONE) || (pWscControl->WscState == WSC_STATE_WAIT_ACK))) {
				pWscControl->WscStatus = STATUS_WSC_CONFIGURED;
				pWscControl->bWscTrigger = FALSE;
				pWscControl->RegData.ReComputePke = 1;

				if (pWscControl->Wsc2MinsTimerRunning) {
					BOOLEAN Cancelled;

					pWscControl->Wsc2MinsTimerRunning = FALSE;
					RTMPCancelTimer(&pWscControl->Wsc2MinsTimer, &Cancelled);
				}

				WscTimeOutProcess(pAd, NULL, pWscControl->WscState, pWscControl);
			}

#endif /* CONFIG_AP_SUPPORT */
			pWscControl->EapolTimerRunning = FALSE;
			NdisZeroMemory(pWscControl->EntryAddr, MAC_ADDR_LEN);
			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("sta is left.\n"));
			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("<----- WscEAPOLTimeOutAction\n"));
			return;
		}
	}

	if (!pWscControl->EapolTimerRunning) {
		pWscControl->WscRetryCount = 0;
		goto out;
	}

	if (pWscControl->EapolTimerPending) {
		RTMPModTimer(&pWscControl->EapolTimer, WSC_EAP_MSG_TIME_OUT);
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("EapolTimer Pending......\n"));
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("<----- WscEAPOLTimeOutAction\n"));
		return;
	}

#ifdef WSC_V2_SUPPORT
	MaxWscDataLen = MaxWscDataLen + (UINT)pWscControl->WscV2Info.ExtraTlv.TlvLen;
#endif /* WSC_V2_SUPPORT */
	os_alloc_mem(NULL, (UCHAR **)&WscData, MaxWscDataLen);

	if (WscData != NULL)
		NdisZeroMemory(WscData, WSC_MAX_DATA_LEN);

#ifdef CONFIG_AP_SUPPORT

	if (CurOpMode == AP_MODE) {
		if (pEntry && IS_ENTRY_CLIENT(pEntry) && (pWscControl->WscState <= WSC_STATE_CONFIGURED) && (pWscControl->WscActionMode != WSC_PROXY)) {
			/* A timer in the AP should cause to be disconnected after 5 seconds if a */
			/* valid EAP-Rsp/Identity indicating WPS is not received. */
			/* << from WPS EAPoL and RSN handling.doc >> */
			WscTimeOutProcess(pWscControl->pAd, pEntry, WSC_STATE_WAIT_RESP_ID, pWscControl);
			/* If do disassocation here, it will affect connection of non-WPS clients. */
			goto out;
		}
	}

#endif /* CONFIG_AP_SUPPORT */
	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("WscState = %d\n", pWscControl->WscState));

	switch (pWscControl->WscState) {
	case WSC_STATE_WAIT_REQ_ID:

		/* For IWSC case, keep sending EAPOL_START until 2 mins timeout */
		if ((pWscControl->WscRetryCount >= 2)
		   )
			WscTimeOutProcess(pWscControl->pAd, pEntry, WSC_STATE_WAIT_REQ_ID, pWscControl);
		else {
			pWscControl->WscRetryCount++;
				WscSendEapolStart(pAd, pEntry->Addr, CurOpMode, wdev);

			RTMPModTimer(&pWscControl->EapolTimer, WSC_EAP_MSG_TIME_OUT);
		}

		break;

	case WSC_STATE_WAIT_WSC_START:
		if (pWscControl->WscRetryCount >= 2)
			WscTimeOutProcess(pWscControl->pAd, pEntry, WSC_STATE_WAIT_WSC_START, pWscControl);
		else {
			pWscControl->WscRetryCount++;
			RTMPModTimer(&pWscControl->EapolTimer, WSC_EAP_MSG_TIME_OUT);
		}

		break;

	case WSC_STATE_WAIT_M1:
		if (pWscControl->WscRetryCount >= 2)
			WscTimeOutProcess(pWscControl->pAd, pEntry, WSC_STATE_WAIT_M1, pWscControl);
		else {
#ifdef CONFIG_AP_SUPPORT

			if (CurOpMode == AP_MODE)
				WscSendMessage(pWscControl->pAd, WSC_OPCODE_START, NULL, 0, pWscControl, AP_MODE, EAP_CODE_REQ);

#endif /* CONFIG_AP_SUPPORT */
			pWscControl->WscRetryCount++;
			RTMPModTimer(&pWscControl->EapolTimer, WSC_EAP_MSG_TIME_OUT);
		}
		break;

	case WSC_STATE_SENT_M1:
		if (pWscControl->WscRetryCount >= 2)
			WscTimeOutProcess(pWscControl->pAd, pEntry, WSC_STATE_WAIT_M2, pWscControl);
		else {
			if (pWscControl->WscActionMode == WSC_ENROLLEE) {
#ifdef CONFIG_AP_SUPPORT
				if (CurOpMode == AP_MODE) {
					if (IS_ENTRY_CLIENT(pEntry))
						WscSendMessage(pWscControl->pAd,
						WSC_OPCODE_MSG,
						pWscControl->RegData.LastTx.Data,
						pWscControl->RegData.LastTx.Length, pWscControl,
						AP_MODE, EAP_CODE_REQ);
					else if (IS_ENTRY_APCLI(pEntry))
						WscSendMessage(pWscControl->pAd,
						WSC_OPCODE_MSG,
						pWscControl->RegData.LastTx.Data,
						pWscControl->RegData.LastTx.Length,
						pWscControl,
						AP_CLIENT_MODE, EAP_CODE_RSP);
				}
#endif /* CONFIG_AP_SUPPORT */
			}

			pWscControl->WscRetryCount++;
			RTMPModTimer(&pWscControl->EapolTimer, WSC_EAP_MSG_TIME_OUT);
		}

		break;

	case WSC_STATE_RX_M2D:
		if (pWscControl->WscRetryCount >= 2)
			WscTimeOutProcess(pWscControl->pAd, pEntry, WSC_STATE_RX_M2D, pWscControl);
		else {
			pWscControl->WscRetryCount++;
			RTMPModTimer(&pWscControl->EapolTimer, WSC_EAP_MSG_TIME_OUT);
		}

		break;

	case WSC_STATE_WAIT_PIN:
		if (pWscControl->WscRetryCount >= 2)
			WscTimeOutProcess(pWscControl->pAd, pEntry, WSC_STATE_WAIT_PIN, pWscControl);
		else {
			pWscControl->WscRetryCount++;
			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("No PIN CODE, cannot send M2 out!\n"));
			RTMPModTimer(&pWscControl->EapolTimer, WSC_EAP_MSG_TIME_OUT);
		}

		break;

	case WSC_STATE_WAIT_M3:
		if (pWscControl->WscRetryCount >= 2)
			WscTimeOutProcess(pWscControl->pAd, pEntry, WSC_STATE_WAIT_M3, pWscControl);
		else {
			if (pWscControl->WscActionMode == WSC_REGISTRAR) {
#ifdef CONFIG_AP_SUPPORT
				if (CurOpMode == AP_MODE)
					WscSendMessage(pWscControl->pAd,
					WSC_OPCODE_MSG,
					pWscControl->RegData.LastTx.Data,
					pWscControl->RegData.LastTx.Length,
					pWscControl,
					AP_MODE, EAP_CODE_REQ);
#endif /* CONFIG_AP_SUPPORT */
			}
			pWscControl->WscRetryCount++;
			RTMPModTimer(&pWscControl->EapolTimer, WSC_EAP_MSG_TIME_OUT);
		}

		break;

	case WSC_STATE_WAIT_M4:
		if (pWscControl->WscRetryCount >= 2)
			WscTimeOutProcess(pWscControl->pAd, pEntry, WSC_STATE_WAIT_M4, pWscControl);
		else {
			if (pWscControl->WscActionMode == WSC_ENROLLEE) {
#ifdef CONFIG_AP_SUPPORT
				if (CurOpMode == AP_MODE) {
					if (IS_ENTRY_CLIENT(pEntry))
						WscSendMessage(pWscControl->pAd,
						WSC_OPCODE_MSG,
						pWscControl->RegData.LastTx.Data,
						pWscControl->RegData.LastTx.Length,
						pWscControl,
						AP_MODE,
						EAP_CODE_REQ);
					else if (IS_ENTRY_APCLI(pEntry))
						WscSendMessage(pWscControl->pAd,
						WSC_OPCODE_MSG,
						pWscControl->RegData.LastTx.Data,
						pWscControl->RegData.LastTx.Length,
						pWscControl,
						AP_CLIENT_MODE,
						EAP_CODE_RSP);
				}
#endif /* CONFIG_AP_SUPPORT */
			}
			pWscControl->WscRetryCount++;
			RTMPModTimer(&pWscControl->EapolTimer, WSC_EAP_MSG_TIME_OUT);
		}

		break;

	case WSC_STATE_WAIT_M5:
		if (pWscControl->WscRetryCount >= 2)
			WscTimeOutProcess(pWscControl->pAd, pEntry, WSC_STATE_WAIT_M5, pWscControl);
		else {
			if (pWscControl->WscActionMode == WSC_REGISTRAR) {
#ifdef CONFIG_AP_SUPPORT
				if (CurOpMode == AP_MODE)
					WscSendMessage(pWscControl->pAd,
					WSC_OPCODE_MSG,
					pWscControl->RegData.LastTx.Data,
					pWscControl->RegData.LastTx.Length,
					pWscControl,
					AP_MODE, EAP_CODE_REQ);
#endif /* CONFIG_AP_SUPPORT */
			}
			pWscControl->WscRetryCount++;
			RTMPModTimer(&pWscControl->EapolTimer, WSC_EAP_MSG_TIME_OUT);
		}

		break;

	case WSC_STATE_WAIT_M6:
		if (pWscControl->WscRetryCount >= 2)
			WscTimeOutProcess(pWscControl->pAd, pEntry, WSC_STATE_WAIT_M6, pWscControl);
		else {
			if (pWscControl->WscActionMode == WSC_ENROLLEE) {
#ifdef CONFIG_AP_SUPPORT
				if (CurOpMode == AP_MODE) {
					if (IS_ENTRY_CLIENT(pEntry))
						WscSendMessage(pWscControl->pAd,
						WSC_OPCODE_MSG,
						pWscControl->RegData.LastTx.Data,
						pWscControl->RegData.LastTx.Length,
						pWscControl,
						AP_MODE, EAP_CODE_REQ);
					else if (IS_ENTRY_APCLI(pEntry))
						WscSendMessage(pWscControl->pAd,
						WSC_OPCODE_MSG,
						pWscControl->RegData.LastTx.Data,
						pWscControl->RegData.LastTx.Length,
						pWscControl,
						AP_CLIENT_MODE, EAP_CODE_RSP);
				}
#endif /* CONFIG_AP_SUPPORT */
			}
			pWscControl->WscRetryCount++;
			RTMPModTimer(&pWscControl->EapolTimer, WSC_EAP_MSG_TIME_OUT);
		}
		break;

	case WSC_STATE_WAIT_M7:
		if (pWscControl->WscRetryCount >= 2)
			WscTimeOutProcess(pWscControl->pAd, pEntry, WSC_STATE_WAIT_M7, pWscControl);
		else {
			if (pWscControl->WscActionMode == WSC_REGISTRAR) {
#ifdef CONFIG_AP_SUPPORT
				if (CurOpMode == AP_MODE)
					WscSendMessage(pWscControl->pAd,
					WSC_OPCODE_MSG,
					pWscControl->RegData.LastTx.Data,
					pWscControl->RegData.LastTx.Length,
					pWscControl,
					AP_MODE,
					EAP_CODE_REQ);

#endif /* CONFIG_AP_SUPPORT */
			}
			pWscControl->WscRetryCount++;
			RTMPModTimer(&pWscControl->EapolTimer, WSC_EAP_MSG_TIME_OUT);
		}
		break;

	case WSC_STATE_WAIT_M8:
		if (pWscControl->WscRetryCount >= 2)
			WscTimeOutProcess(pWscControl->pAd, pEntry, WSC_STATE_WAIT_M8, pWscControl);
		else {
			if (pWscControl->WscActionMode == WSC_ENROLLEE) {
#ifdef CONFIG_AP_SUPPORT
				if (CurOpMode == AP_MODE) {
					if (IS_ENTRY_CLIENT(pEntry))
						WscSendMessage(pWscControl->pAd,
						WSC_OPCODE_MSG,
						pWscControl->RegData.LastTx.Data,
						pWscControl->RegData.LastTx.Length,
						pWscControl,
						AP_MODE,
						EAP_CODE_REQ);
					else if (IS_ENTRY_APCLI(pEntry))
						WscSendMessage(pWscControl->pAd,
						WSC_OPCODE_MSG,
						pWscControl->RegData.LastTx.Data,
						pWscControl->RegData.LastTx.Length,
						pWscControl,
						AP_CLIENT_MODE,
						EAP_CODE_RSP);
				}
#endif /* CONFIG_AP_SUPPORT */
			}
			pWscControl->WscRetryCount++;
			RTMPModTimer(&pWscControl->EapolTimer, WSC_EAP_MSG_TIME_OUT);
		}
		break;

	case WSC_STATE_WAIT_DONE:
		if (pWscControl->WscRetryCount >= 2)
			WscTimeOutProcess(pWscControl->pAd, pEntry, WSC_STATE_WAIT_DONE, pWscControl);
		else {
			if (pWscControl->WscActionMode == WSC_REGISTRAR) {
#ifdef CONFIG_AP_SUPPORT
				if (CurOpMode == AP_MODE)
					WscSendMessage(pWscControl->pAd,
					WSC_OPCODE_MSG,
					pWscControl->RegData.LastTx.Data,
					pWscControl->RegData.LastTx.Length,
					pWscControl,
					AP_MODE,
					EAP_CODE_REQ);
#endif /* CONFIG_AP_SUPPORT */
			}
			pWscControl->WscRetryCount++;
			RTMPModTimer(&pWscControl->EapolTimer, WSC_EAP_MSG_TIME_OUT);
		}

		break;
#ifdef CONFIG_AP_SUPPORT

	/* Only AP_Enrollee needs to wait EAP_ACK */
	case WSC_STATE_WAIT_ACK:
		WscTimeOutProcess(pWscControl->pAd,
			pEntry, WSC_STATE_WAIT_ACK, pWscControl);
		break;
#endif /* CONFIG_AP_SUPPORT */

	case WSC_STATE_WAIT_EAPFAIL:
		/* Wait 2 seconds */
		if (pWscControl->WscRetryCount >= 1)
			WscTimeOutProcess(pWscControl->pAd, pEntry, WSC_STATE_WAIT_EAPFAIL, pWscControl);
		else {
			RTMPModTimer(&pWscControl->EapolTimer, WSC_EAP_EAP_FAIL_TIME_OUT);
			pWscControl->WscRetryCount++;
		}
		break;

	default:
		break;
	}
out:
	if (WscData)
		os_free_mem(WscData);
	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE,
		("<----- WscEAPOLTimeOutAction\n"));
}

VOID Wsc2MinsTimeOutAction(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	PWSC_CTRL pWscControl = (PWSC_CTRL)FunctionContext;
	PRTMP_ADAPTER pAd = NULL;
#ifdef CONFIG_AP_SUPPORT
	INT IsAPConfigured = 0;
#endif /* CONFIG_AP_SUPPORT */
	BOOLEAN Cancelled;
	UCHAR CurOpMode = 0xFF;
	struct wifi_dev *wdev;


	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("-----> Wsc2MinsTimeOutAction\n"));

	if (pWscControl != NULL) {
		wdev = (struct wifi_dev *)pWscControl->wdev;
		ASSERT(wdev);
		pAd =  (PRTMP_ADAPTER)pWscControl->pAd;

		if (pAd == NULL) {
			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("pAd is NULL!\n"));
			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("<----- Wsc2MinsTimeOutAction\n"));
			return;
		}

#ifdef CONFIG_AP_SUPPORT
#ifdef CON_WPS

		if (pWscControl->conWscStatus != CON_WPS_STATUS_DISABLED) {
			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_OFF, ("CON_WPS: Reset the status to default.\n"));
			pWscControl->conWscStatus = CON_WPS_STATUS_DISABLED;
		}

#endif /* CON_WPS */
		if (WDEV_WSC_AP(wdev))
			CurOpMode = AP_MODE;
		else
			CurOpMode = STA_MODE;
#endif /* CONFIG_AP_SUPPORT */
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("Wsc2MinsTimerRunning is %s\n",
				 pWscControl->Wsc2MinsTimerRunning ? "TRUE, reset WscState to WSC_STATE_OFF" : "FALSE"));

		/* Enable CLI interface */
#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
		if ((CurOpMode == STA_MODE) &&
			(wdev->wdev_type == WDEV_TYPE_APCLI)) {
			UCHAR apcli_idx = (pWscControl->EntryIfIdx & 0x0F);

			if (pAd->ApCfg.ApCliTab[apcli_idx].ApCliInit != FALSE)
				pAd->ApCfg.ApCliTab[apcli_idx].Enable = TRUE;
		}
#endif
#endif
#ifdef WSC_LED_SUPPORT
		/* 120 seconds WPS walk time expiration. */
		pWscControl->bWPSWalkTimeExpiration = TRUE;
#endif /* WSC_LED_SUPPORT */

		if (pWscControl->Wsc2MinsTimerRunning) {
			pWscControl->bWscTrigger = FALSE;
			if (pWscControl->EapolTimerRunning) {
				pWscControl->EapolTimerRunning = FALSE;
				RTMPCancelTimer(&pWscControl->EapolTimer, &Cancelled);
			}
#ifdef CONFIG_AP_SUPPORT

			if (CurOpMode == AP_MODE) {
				IsAPConfigured = pWscControl->WscConfStatus;

				if ((pWscControl->EntryIfIdx & 0x0F) < pAd->ApCfg.BssidNum) {
					UCHAR apidx = pWscControl->EntryIfIdx & 0x0F;
					struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
/* WPS_BandSteering Support */
#ifdef BAND_STEERING
					if (pAd->ApCfg.BandSteering) {
						PBND_STRG_CLI_TABLE table = Get_BndStrgTable(pAd, wdev->func_idx);

						if (table && table->bEnabled) {
							NdisAcquireSpinLock(&table->WpsWhiteListLock);
							ClearWpsWhiteList(&table->WpsWhiteList);
							NdisReleaseSpinLock(&table->WpsWhiteListLock);
							MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("%s:channel %u wps whitelist cleared, size : %d\n",
									__func__, table->Channel, table->WpsWhiteList.size));
						}
					}
#endif
					WscBuildBeaconIE(pWscControl->pAd, IsAPConfigured, FALSE, 0, 0, (pWscControl->EntryIfIdx & 0x0F), NULL, 0, CurOpMode);
					WscBuildProbeRespIE(pWscControl->pAd, WSC_MSGTYPE_AP_WLAN_MGR, IsAPConfigured, FALSE, 0, 0, pWscControl->EntryIfIdx, NULL, 0, CurOpMode);
					UpdateBeaconHandler(
						pWscControl->pAd,
						wdev,
						BCN_UPDATE_IE_CHG);
				}

				if ((pWscControl->WscConfMode & WSC_PROXY) == 0) {
					/* Proxy mechanism is disabled */
					pWscControl->WscState = WSC_STATE_OFF;
				}
			}

#endif /* CONFIG_AP_SUPPORT */
			pWscControl->WscMode = 1;
			pWscControl->WscRetryCount = 0;
			pWscControl->Wsc2MinsTimerRunning = FALSE;
			pWscControl->WscSelReg = 0;
			pWscControl->WscStatus = STATUS_WSC_IDLE;
			RTMPSendWirelessEvent(pAd, IW_WSC_2MINS_TIMEOUT, NULL, (pWscControl->EntryIfIdx & 0x0F), 0);

#if defined(VENDOR_FEATURE6_SUPPORT) || defined(VENDOR_FEATURE7_SUPPORT)
#ifdef CONFIG_AP_SUPPORT
			WscSendUPnPMessage(pAd, (pWscControl->EntryIfIdx & 0x0F), WSC_OPCODE_UPNP_CTRL, WSC_UPNP_DATA_SUB_WSC_TIMEOUT, &pAd->ApCfg.MBSSID[pWscControl->EntryIfIdx & 0x0F].wdev.bssid[0], MAC_ADDR_LEN, 0, 0, &pAd->ApCfg.MBSSID[pWscControl->EntryIfIdx & 0x0F].wdev.bssid[0], AP_MODE);
#endif /* CONFIG_AP_SUPPORT */
#endif /* VENDOR_FEATURE6_SUPPORT */
			if (pWscControl->WscScanTimerRunning) {
				pWscControl->WscScanTimerRunning = FALSE;
				RTMPCancelTimer(&pWscControl->WscScanTimer, &Cancelled);
			}

			if (pWscControl->WscPBCTimerRunning) {
				pWscControl->WscPBCTimerRunning = FALSE;
				RTMPCancelTimer(&pWscControl->WscPBCTimer, &Cancelled);
			}

		}

#ifdef WSC_LED_SUPPORT

		/* if link is up, there shall be nothing wrong */
		/* perhaps we will set another flag to do it */

#ifdef VENDOR_FEATURE7_SUPPORT
	if ((pWscControl->WscState == WSC_STATE_OFF) &&
	(pWscControl->WscStatus == STATUS_WSC_CONFIGURED))
#else
		if ((STA_STATUS_TEST_FLAG(pStaCfg, fSTA_STATUS_MEDIA_STATE_CONNECTED)) &&
			(pWscControl->WscState == WSC_STATE_OFF) &&
			(pWscControl->WscStatus == STATUS_WSC_CONFIGURED))
#endif
			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("WscConnectTimeout --> Connection OK\n"));
		else {
			UCHAR WPSLEDStatus;

			pWscControl->WscStatus = STATUS_WSC_FAIL;
			pWscControl->WscState = WSC_STATE_OFF;

			/* WPS LED mode 7, 8, 11 or 12. */
			if ((LED_MODE(pAd) == WPS_LED_MODE_7) ||
				(LED_MODE(pAd) == WPS_LED_MODE_8) ||
				(LED_MODE(pAd) == WPS_LED_MODE_11) ||
				(LED_MODE(pAd) == WPS_LED_MODE_12)) {
				pWscControl->bSkipWPSTurnOffLED = FALSE;
				/* Turn off the WPS LED modoe due to the maximum WPS processing time is expired (120 seconds). */
				WPSLEDStatus = LED_WPS_TURN_LED_OFF;
				RTMPSetLED(pAd, WPSLEDStatus);
			} else if ((LED_MODE(pAd) == WPS_LED_MODE_9) /* WPS LED mode 9. */
					  ) {
				if (pWscControl->WscMode == WSC_PIN_MODE) { /* PIN method. */
					/* The NIC using PIN method fails to finish the WPS handshaking within 120 seconds. */
					WPSLEDStatus = LED_WPS_ERROR;
					RTMPSetLED(pAd, WPSLEDStatus);
					/* Turn off the WPS LED after 15 seconds. */
					RTMPSetTimer(&pWscControl->WscLEDTimer, WSC_WPS_FAIL_LED_PATTERN_TIMEOUT);
					/* The Ralink UI would make RT_OID_DISCONNECT_REQUEST request while it receive STATUS_WSC_EAP_FAILED. */
					/* Allow the NIC to turn off the WPS LED after WSC_WPS_SKIP_TURN_OFF_LED_TIMEOUT seconds. */
					pWscControl->bSkipWPSTurnOffLED = TRUE;
					RTMPSetTimer(&pWscControl->WscSkipTurnOffLEDTimer, WSC_WPS_SKIP_TURN_OFF_LED_TIMEOUT);
					MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("%s: The NIC using PIN method fails to finish the WPS handshaking within 120 seconds.\n", __func__));
				} else if (pWscControl->WscMode == WSC_PBC_MODE) { /* PBC method. */
					switch (pWscControl->WscLastWarningLEDMode) { /* Based on last WPS warning LED mode. */
					case 0:
					case LED_WPS_ERROR:
					case LED_WPS_SESSION_OVERLAP_DETECTED:
						/* Failed to find any partner. */
						WPSLEDStatus = LED_WPS_ERROR;
						RTMPSetLED(pAd, WPSLEDStatus);
						/* Turn off the WPS LED after 15 seconds. */
						RTMPSetTimer(&pWscControl->WscLEDTimer, WSC_WPS_FAIL_LED_PATTERN_TIMEOUT);
						/* The Ralink UI would make RT_OID_DISCONNECT_REQUEST request while it receive STATUS_WSC_EAP_FAILED. */
						/* Allow the NIC to turn off the WPS LED after WSC_WPS_SKIP_TURN_OFF_LED_TIMEOUT seconds. */
						pWscControl->bSkipWPSTurnOffLED = TRUE;
						RTMPSetTimer(&pWscControl->WscSkipTurnOffLEDTimer, WSC_WPS_SKIP_TURN_OFF_LED_TIMEOUT);
						MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("%s: Last WPS LED status is LED_WPS_ERROR.\n", __func__));
						break;

					default:
						/* do nothing. */
						break;
					}
				} else {
					/* do nothing. */
				}
			} else {
				/* do nothing. */
			}

			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("WscConnectTimeout --> Fail to connect\n"));
		}

#endif /* WSC_LED_SUPPORT */
	}

	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("<----- Wsc2MinsTimeOutAction\n"));
}

/*
*	========================================================================
*
*	Routine Description:
*		Classify EAP message type for enrolee
*
*	Arguments:
*		pAd         - NIC Adapter pointer
*		Elem		- The EAP packet
*
*	Return Value:
*		Received EAP message type
*
*	IRQL = DISPATCH_LEVEL
*
*	Note:
*
*	========================================================================
*/
UCHAR WscRxMsgType(
	IN  PRTMP_ADAPTER pAdapter,
	IN  PMLME_QUEUE_ELEM pElem)
{
	USHORT Length;
	PUCHAR pData;
	USHORT WscType, WscLen;
	RTMP_STRING id_data[] = {"hello"};
	RTMP_STRING fail_data[] = {"EAP_FAIL"};
	RTMP_STRING wsc_start[] = {"WSC_START"};
#ifdef WSC_V2_SUPPORT
	RTMP_STRING wsc_frag_ack[] = "WSC_FRAG_ACK";
#endif /* WSC_V2_SUPPORT */
	RTMP_STRING regIdentity[] = {"WFA-SimpleConfig-Registrar"};
	RTMP_STRING enrIdentity[] = {"WFA-SimpleConfig-Enrollee"};

	if (pElem->Msg[0] == 'W' && pElem->Msg[1] == 'F' && pElem->Msg[2] == 'A') {
		/* Eap-Rsp(Identity) */
		if (memcmp(regIdentity, pElem->Msg, strlen(regIdentity)) == 0)
			return  WSC_MSG_EAP_REG_RSP_ID;
		else if (memcmp(enrIdentity, pElem->Msg, strlen(enrIdentity)) == 0)
			return  WSC_MSG_EAP_ENR_RSP_ID;
	} else if (NdisEqualMemory(id_data, pElem->Msg, pElem->MsgLen)) {
		/* Eap-Req/Identity(hello) */
		return  WSC_MSG_EAP_REQ_ID;
	} else if (NdisEqualMemory(fail_data, pElem->Msg, pElem->MsgLen)) {
		/* Eap-Fail */
		return  WSC_MSG_EAP_FAIL;
	} else if (NdisEqualMemory(wsc_start, pElem->Msg, pElem->MsgLen)) {
		/* Eap-Req(Wsc_Start) */
		return WSC_MSG_EAP_REQ_START;
	}

#ifdef WSC_V2_SUPPORT
	else if (NdisEqualMemory(wsc_frag_ack, pElem->Msg, pElem->MsgLen)) {
		/* WSC FRAG ACK */
		return WSC_MSG_EAP_FRAG_ACK;
	}

#endif /* WSC_V2_SUPPORT */
	else {
		/* Eap-Esp(Messages) */
		pData = pElem->Msg;
		Length = (USHORT)pElem->MsgLen;
		/* the first TLV item in EAP Messages must be WSC_IE_VERSION */
		NdisMoveMemory(&WscType, pData, 2);

		if (ntohs(WscType) != WSC_ID_VERSION)
			goto out;

		/* Not Wsc Start, We have to look for WSC_IE_MSG_TYPE to classify M2 ~ M8, the remain size must large than 4 */
		while (Length > 4) {
			/* arm-cpu has packet alignment issue, it's better to use memcpy to retrieve data */
			NdisMoveMemory(&WscType, pData, 2);
			NdisMoveMemory(&WscLen,  pData + 2, 2);
			WscLen = ntohs(WscLen);
			if (ntohs(WscType) == WSC_ID_MSG_TYPE)
				return *(pData + 4);	/* Found the message type */
			pData  += (WscLen + 4);
			Length -= (WscLen + 4);
		}
	}
out:
	return  WSC_MSG_UNKNOWN;
}

/*
*	========================================================================
*
*	Routine Description:
*		Classify WSC message type
*
*	Arguments:
*		EAPType		Value of EAP message type
*		MsgType		Internal Message definition for MLME state machine
*
*	Return Value:
*		TRUE		Found appropriate message type
*		FALSE		No appropriate message type
*
*	Note:
*		All these constants are defined in wsc.h
*		For supplicant, there is only EAPOL Key message avaliable
*
*	========================================================================
*/
BOOLEAN	WscMsgTypeSubst(
	IN  UCHAR EAPType,
	IN  UCHAR EAPCode,
	OUT INT *MsgType)
{
	switch (EAPType) {
	case EAPPacket:
		*MsgType = WSC_EAPOL_PACKET_MSG;
		break;

	case EAPOLStart:
		*MsgType = WSC_EAPOL_START_MSG;
		break;

	default:
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("WscMsgTypeSubst : unsupported EAP Type(%d);\n", EAPType));
		return FALSE;
	}

	return TRUE;
}

VOID WscInitRegistrarPair(RTMP_ADAPTER *pAd, WSC_CTRL *pWscControl, UCHAR apidx)
{
	UCHAR CurOpMode = 0xff;
	struct wifi_dev *wdev = NULL;

	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("-----> WscInitRegistrarPair\n"));
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	CurOpMode = AP_MODE;
#endif /* CONFIG_AP_SUPPORT */
	pWscControl->WscActionMode = 0;
	/* 1. Version */
	/*pWscControl->RegData.SelfInfo.Version = WSC_VERSION; */
	/* 2. UUID Enrollee, last 6 bytes use MAC */
	NdisMoveMemory(&pWscControl->RegData.SelfInfo.Uuid[0], &pWscControl->Wsc_Uuid_E[0], UUID_LEN_HEX);
	/* 3. MAC address */
#ifdef CONFIG_AP_SUPPORT

	if (CurOpMode == AP_MODE) {
		if (apidx >= HW_BEACON_MAX_NUM) {
			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR,
					 ("%s: apidx >= HW_BEACON_MAX_NUM!\n", __func__));
			apidx = 0;
		}

		if (apidx < HW_BEACON_MAX_NUM) {
			NdisMoveMemory(pWscControl->RegData.SelfInfo.MacAddr, pAd->ApCfg.MBSSID[apidx].wdev.bssid, 6);
			wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
		}
	}

#endif /* CONFIG_AP_SUPPORT */
	/* 4. Device Name */
#ifdef CONFIG_AP_SUPPORT

	if (CurOpMode == AP_MODE) {
		if (!RTMP_TEST_FLAG(pWscControl, 0x04)) {
#ifdef VENDOR_FEATURE7_SUPPORT
			if (pWscControl->WscConfStatus == WSC_SCSTATE_UNCONFIGURED)
				NdisMoveMemory(&pWscControl->RegData.SelfInfo.DeviceName,
				AP_WSC_DEVICE_NAME_OOB, sizeof(AP_WSC_DEVICE_NAME_OOB));
				/* ARRIS MODIFY */
			else if (WMODE_CAP_5G(wdev->PhyMode))
				NdisMoveMemory(&pWscControl->RegData.SelfInfo.DeviceName,
				AP_WSC_DEVICE_NAME_5G, sizeof(AP_WSC_DEVICE_NAME_5G));
				/* ARRIS MODIFY */
			else if (WMODE_CAP_2G(wdev->PhyMode))
				NdisMoveMemory(&pWscControl->RegData.SelfInfo.DeviceName,
				AP_WSC_DEVICE_NAME_2G, sizeof(AP_WSC_DEVICE_NAME_2G));
				/* ARRIS MODIFY */
#else
			NdisZeroMemory(&pWscControl->RegData.SelfInfo.DeviceName[0],
						   sizeof(pWscControl->RegData.SelfInfo.DeviceName));
			snprintf(&pWscControl->RegData.SelfInfo.DeviceName[0],
				sizeof(pWscControl->RegData.SelfInfo.DeviceName), "%s_%d", AP_WSC_DEVICE_NAME, apidx);
#endif
		}
	}

#endif /* CONFIG_AP_SUPPORT */

	/* 5. Manufacture woody */
	if (!RTMP_TEST_FLAG(pWscControl, 0x01))
		NdisMoveMemory(&pWscControl->RegData.SelfInfo.Manufacturer,
					   WSC_MANUFACTURE, sizeof(WSC_MANUFACTURE));

	/* 6. Model Name */
#ifdef CONFIG_AP_SUPPORT

	if (CurOpMode == AP_MODE) {
		if (!RTMP_TEST_FLAG(pWscControl, 0x02))
			NdisMoveMemory(&pWscControl->RegData.SelfInfo.ModelName, AP_WSC_MODEL_NAME, sizeof(AP_WSC_MODEL_NAME));
	}

#endif /* CONFIG_AP_SUPPORT */

	/* 7. Model Number */

	if (!RTMP_TEST_FLAG(pWscControl, 0x08))
		NdisMoveMemory(&pWscControl->RegData.SelfInfo.ModelNumber, WSC_MODEL_NUMBER, sizeof(WSC_MODEL_NUMBER));

	/* 8. Serial Number */
	if (!RTMP_TEST_FLAG(pWscControl, 0x10))
		NdisMoveMemory(&pWscControl->RegData.SelfInfo.SerialNumber, WSC_MODEL_SERIAL, sizeof(WSC_MODEL_SERIAL));

	/* 9. Authentication Type Flags */
	/* Open(=1), WPAPSK(=2),Shared(=4), WPA2PSK(=20),WPA(=8),WPA2(=10) */
	/* (0x01 | 0x02 | 0x04 | 0x20 | 0x08 | 0x10) = 0x3F */
	/* WCN vista logo will check this flags. */
#ifdef WSC_V2_SUPPORT

	if (pWscControl->WscV2Info.bEnableWpsV2)
		/*
		*	AuthTypeFlags only needs to include Open and WPA2PSK in WSC 2.0.
		*/
		pWscControl->RegData.SelfInfo.AuthTypeFlags = cpu2be16(0x0021);
	else
#endif /* WSC_V2_SUPPORT */
		pWscControl->RegData.SelfInfo.AuthTypeFlags = cpu2be16(0x003F);

	/* 10. Encryption Type Flags */
	/* None(=1), WEP(=2), TKIP(=4), AES(=8) */
	/* (0x01 | 0x02 | 0x04 | 0x08) = 0x0F */
#ifdef WSC_V2_SUPPORT

	if (pWscControl->WscV2Info.bEnableWpsV2)
		/*
		*	EncrTypeFlags only needs to include None and AES in WSC 2.0.
		*/
		pWscControl->RegData.SelfInfo.EncrTypeFlags = cpu2be16(0x0009);
	else
#endif /* WSC_V2_SUPPORT */
		pWscControl->RegData.SelfInfo.EncrTypeFlags  = cpu2be16(0x000F);

	/* 11. Connection Type Flag */
		pWscControl->RegData.SelfInfo.ConnTypeFlags = 0x01;					/* ESS */

	/* 12. Associate state */
	pWscControl->RegData.SelfInfo.AssocState = cpu2be16(0x0000);		/* Not associated */
	/* 13. Configure Error */
	pWscControl->RegData.SelfInfo.ConfigError = cpu2be16(0x0000);		/* No error */
	/* 14. OS Version */
	pWscControl->RegData.SelfInfo.OsVersion = cpu2be32(0x80000000);		/* first bit must be 1 */
	/* 15. RF Band */
	/* Some WPS AP would check RfBand value in M1, ex. D-Link DIR-628 */
	pWscControl->RegData.SelfInfo.RfBand = 0x00;

	if (WMODE_CAP_5G(wdev->PhyMode))
		pWscControl->RegData.SelfInfo.RfBand |= WSC_RFBAND_50GHZ;			/* 5.0G */

	if (WMODE_CAP_2G(wdev->PhyMode))
		pWscControl->RegData.SelfInfo.RfBand |= WSC_RFBAND_24GHZ;			/* 2.4G */

	/* 16. Config Method */
	pWscControl->RegData.SelfInfo.ConfigMethods = cpu2be16(pWscControl->WscConfigMethods);
	/*pWscControl->RegData.EnrolleeInfo.ConfigMethods = cpu2be16(WSC_CONFIG_METHODS);		// Label, Display, PBC */
	/*pWscControl->RegData.EnrolleeInfo.ConfigMethods = cpu2be16(0x0084);		// Label, Display, PBC */
	/* 17. Simple Config State */
#ifdef CONFIG_AP_SUPPORT

	if (CurOpMode == AP_MODE)
		pWscControl->RegData.SelfInfo.ScState = pWscControl->WscConfStatus;

#endif /* CONFIG_AP_SUPPORT */

	/* 18. Device Password ID */
	if (pWscControl->WscMode == WSC_PBC_MODE)
		pWscControl->RegData.SelfInfo.DevPwdId = cpu2be16(DEV_PASS_ID_PBC); /* PBC */
	else {
		/* Let PIN be default DPID.  */
			pWscControl->RegData.SelfInfo.DevPwdId = cpu2be16(DEV_PASS_ID_PIN);		/* PIN mode */
	}

	/* 19. SSID */
#ifdef CONFIG_AP_SUPPORT

	if (CurOpMode == AP_MODE) {
		if (apidx < HW_BEACON_MAX_NUM)
			NdisMoveMemory(pWscControl->RegData.SelfInfo.Ssid, pAd->ApCfg.MBSSID[apidx].Ssid, pAd->ApCfg.MBSSID[apidx].SsidLen);
	}

#endif /* CONFIG_AP_SUPPORT */
	/* 20. Primary Device Type */
#ifdef CONFIG_AP_SUPPORT

	if (CurOpMode == AP_MODE) {
			if (pWscControl->EntryIfIdx & MIN_NET_DEVICE_FOR_APCLI)
				NdisMoveMemory(&pWscControl->RegData.SelfInfo.PriDeviceType, &STA_Wsc_Pri_Dev_Type[0], 8);
			else
				NdisMoveMemory(&pWscControl->RegData.SelfInfo.PriDeviceType, &AP_Wsc_Pri_Dev_Type[0], 8);
	}

#endif /* CONFIG_AP_SUPPORT */
	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("<----- WscInitRegistrarPair\n"));
}


VOID WscSendEapReqId(
	IN  PRTMP_ADAPTER pAd,
	IN  PMAC_TABLE_ENTRY pEntry,
	IN  UCHAR CurOpMode)
{
	UCHAR Header802_3[14];
	USHORT Length;
	IEEE8021X_FRAME Ieee_8021x;
	EAP_FRAME EapFrame;
	UCHAR *pOutBuffer = NULL;
	ULONG FrameLen = 0;
	UCHAR Data[] = "hello";
	UCHAR Id;
	PWSC_CTRL pWpsCtrl = NULL;

	NdisZeroMemory(Header802_3, sizeof(UCHAR) * 14);
	/* 1. Send EAP-Rsp Id */
	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("-----> WscSendEapReqId\n"));
#ifdef CONFIG_AP_SUPPORT
	if (CurOpMode == AP_MODE) {
		if (pEntry->func_tb_idx < HW_BEACON_MAX_NUM) {
			pWpsCtrl = &pEntry->wdev->WscControl;
			MAKE_802_3_HEADER(Header802_3,
							  &pEntry->Addr[0],
							  &pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.bssid[0],
							  EAPOL);
		}
	}
#endif /* CONFIG_AP_SUPPORT */

	if (pWpsCtrl == NULL) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, ("pWpsCtrl == NULL!\n"));
		return;
	}

	/* Length, -1 NULL pointer of string */
	Length = sizeof(EAP_FRAME) + sizeof(Data) - 1;
	/* Zero 802.1x body */
	NdisZeroMemory(&Ieee_8021x, sizeof(Ieee_8021x));
	Ieee_8021x.Version = EAPOL_VER;
	Ieee_8021x.Type    = EAPPacket;
	Ieee_8021x.Length  = cpu2be16(Length);
	/* Zero EAP frame */
	NdisZeroMemory(&EapFrame, sizeof(EapFrame));
	/* RFC 3748 Ch 4.1: recommended to initalize Identifier with a random number */
	Id = RandomByte(pAd);

	if (Id == pWpsCtrl->lastId)
		Id += 1;

	EapFrame.Code   = EAP_CODE_REQ;
	EapFrame.Id     = Id;
	EapFrame.Length = cpu2be16(Length);
	EapFrame.Type   = EAP_TYPE_ID;
	pWpsCtrl->lastId = Id;
	/* Out buffer for transmitting EAP-Req(Identity) */
	os_alloc_mem(NULL, (UCHAR **)&pOutBuffer, MAX_LEN_OF_MLME_BUFFER);

	if (pOutBuffer == NULL)
		return;

	FrameLen = 0;
	/* Make	 Transmitting frame */
	MakeOutgoingFrame(pOutBuffer, &FrameLen,
					  sizeof(IEEE8021X_FRAME), &Ieee_8021x,
					  sizeof(EapFrame), &EapFrame,
					  (sizeof(Data) - 1), Data,
					  END_OF_ARGS);
	/* Copy frame to Tx ring */
	RTMPToWirelessSta(pAd, pEntry, Header802_3, sizeof(Header802_3), (PUCHAR)pOutBuffer, FrameLen, TRUE);
	pWpsCtrl->WscRetryCount = 0;

	if (pOutBuffer)
		os_free_mem(pOutBuffer);

	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("<----- WscSendEapReqId\n"));
}

/*
*	========================================================================
*
*	Routine Description:
*		Send EAPoL-Start packet to AP.
*
*	Arguments:
*		pAd         - NIC Adapter pointer
*
*	Return Value:
*		None
*
*	IRQL = DISPATCH_LEVEL
*
*	Note:
*		Actions after link up
*		1. Change the correct parameters
*		2. Send EAPOL - START
*
*	========================================================================
*/
VOID WscSendEapolStart(
	IN  PRTMP_ADAPTER pAdapter,
	IN  PUCHAR pBssid,
	IN  UCHAR CurOpMode,
	IN  VOID *wdev_obj)
{
	IEEE8021X_FRAME Packet;
	UCHAR Header802_3[14];
	MAC_TABLE_ENTRY *pEntry;
	UCHAR if_idx = 0;
	struct wifi_dev *wdev;
	WSC_CTRL *wsc_ctrl;

	ASSERT(wdev_obj);
	if (!wdev_obj) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR,
				 ("%s: wdev_obj is null.\n",
				  __func__));
		return;
	}
	wdev = (struct wifi_dev *)wdev_obj;
	wdev->wdev_ops->mac_entry_lookup(pAdapter, pBssid, wdev, &pEntry);

	if (pEntry == NULL) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE,
				 ("%s: cannot find this entry(%02x:%02x:%02x:%02x:%02x:%02x)\n",
				  __func__,
				  PRINT_MAC(pBssid)));
		return;
	}

	wsc_ctrl = &wdev->WscControl;

	if (wsc_ctrl->WscState >= WSC_STATE_WAIT_WSC_START)
		return;

	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("-----> WscSendEapolStart\n"));
	if_idx = pEntry->func_tb_idx;
	NdisZeroMemory(Header802_3, sizeof(UCHAR) * 14);
	/* 1. Change the authentication to open and encryption to none if necessary. */
	/* init 802.3 header and Fill Packet */
	MAKE_802_3_HEADER(Header802_3,
					  pBssid,
					  &wdev->if_addr[0],
					  EAPOL);
	/* Zero message 2 body */
	NdisZeroMemory(&Packet, sizeof(Packet));
	Packet.Version = EAPOL_VER;
	Packet.Type    = EAPOLStart;
	Packet.Length  = cpu2be16(0);

	if (pEntry)
		RTMPToWirelessSta(pAdapter, pEntry, Header802_3, sizeof(Header802_3), (PUCHAR)&Packet, 4, TRUE);

	/* Update WSC status */
	wsc_ctrl->WscStatus = STATUS_WSC_EAPOL_START_SENT;
	wsc_ctrl->WscState = WSC_STATE_WAIT_REQ_ID;

	if (!wsc_ctrl->EapolTimerRunning) {
		wsc_ctrl->EapolTimerRunning = TRUE;
		RTMPSetTimer(&wsc_ctrl->EapolTimer, WSC_EAPOL_START_TIME_OUT);
	}

	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("<----- WscSendEapolStart\n"));
}


VOID WscSendEapRspId(
	IN PRTMP_ADAPTER pAdapter,
	IN PMAC_TABLE_ENTRY pEntry,
	IN PWSC_CTRL pWscControl)
{
	UCHAR Header802_3[14];
	USHORT Length = 0;
	IEEE8021X_FRAME Ieee_8021x;
	EAP_FRAME EapFrame;
	UCHAR *pOutBuffer = NULL;
	ULONG FrameLen = 0;
	UCHAR regIdentity[] = "WFA-SimpleConfig-Registrar-1-0";
	UCHAR enrIdentity[] = "WFA-SimpleConfig-Enrollee-1-0";
	UCHAR CurOpMode = 0xff;
	struct wifi_dev *wdev;

	wdev = (struct wifi_dev *)pWscControl->wdev;
	NdisZeroMemory(Header802_3, sizeof(UCHAR) * 14);
	/* 1. Send EAP-Rsp Id */
	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("-----> WscSendEapRspId\n"));

	if (WDEV_WSC_AP(wdev))
		CurOpMode = AP_MODE;
	else
		CurOpMode = STA_MODE;

	MAKE_802_3_HEADER(Header802_3,
					  &pEntry->Addr[0],
					  &wdev->if_addr[0],
					  EAPOL);

	/* Length, -1 NULL pointer of string */
	if (pWscControl->WscConfMode == WSC_ENROLLEE)
		Length = sizeof(EAP_FRAME) + sizeof(enrIdentity) - 1;
	else if (pWscControl->WscConfMode == WSC_REGISTRAR)
		Length = sizeof(EAP_FRAME) + sizeof(regIdentity) - 1;

	/* Zero 802.1x body */
	NdisZeroMemory(&Ieee_8021x, sizeof(Ieee_8021x));
	Ieee_8021x.Version = EAPOL_VER;
	Ieee_8021x.Type    = EAPPacket;
	Ieee_8021x.Length  = cpu2be16(Length);
	/* Zero EAP frame */
	NdisZeroMemory(&EapFrame, sizeof(EapFrame));
	EapFrame.Code   = EAP_CODE_RSP;
	EapFrame.Id     = pWscControl->lastId;
	EapFrame.Length = cpu2be16(Length);
	EapFrame.Type   = EAP_TYPE_ID;
	/* Out buffer for transmitting EAP-Req(Identity) */
	os_alloc_mem(NULL, (UCHAR **)&pOutBuffer, MAX_LEN_OF_MLME_BUFFER);

	if (pOutBuffer == NULL)
		return;

	FrameLen = 0;

	if (pWscControl->WscConfMode == WSC_REGISTRAR) {
		/* Make tx frame */
		MakeOutgoingFrame(pOutBuffer, &FrameLen,
						  sizeof(IEEE8021X_FRAME), &Ieee_8021x,
						  sizeof(EapFrame), &EapFrame,
						  (sizeof(regIdentity) - 1), regIdentity,
						  END_OF_ARGS);
	} else if (pWscControl->WscConfMode == WSC_ENROLLEE) {
		/* Make	 Transmitting frame */
		MakeOutgoingFrame(pOutBuffer, &FrameLen,
						  sizeof(IEEE8021X_FRAME), &Ieee_8021x,
						  sizeof(EapFrame), &EapFrame,
						  (sizeof(enrIdentity) - 1), enrIdentity,
						  END_OF_ARGS);
	} else {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE,
			("WscConfMode(%d) is not WSC_REGISTRAR nor WSC_ENROLLEE.\n", pWscControl->WscConfMode));
		goto out;
	}

	/* Copy frame to Tx ring */
	RTMPToWirelessSta((PRTMP_ADAPTER)pWscControl->pAd, pEntry,
		Header802_3, LENGTH_802_3, (PUCHAR)pOutBuffer, FrameLen, TRUE);
	pWscControl->WscRetryCount = 0;

	if (!pWscControl->EapolTimerRunning) {
		pWscControl->EapolTimerRunning = TRUE;
		RTMPSetTimer(&pWscControl->EapolTimer, WSC_EAP_ID_TIME_OUT);
	}

out:
	if (pOutBuffer)
		os_free_mem(pOutBuffer);

	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE,
		("<----- WscSendEapRspId\n"));
}

VOID WscUPnPErrHandle(
	IN  PRTMP_ADAPTER pAd,
	IN  PWSC_CTRL pWscControl,
	IN  UINT eventID)
{
	int dataLen;
	UCHAR *pWscData;
	UCHAR CurOpMode;
	struct wifi_dev *wdev;

	wdev = (struct wifi_dev *)pWscControl->wdev;
	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("Into WscUPnPErrHandle, send WSC_OPCODE_UPNP_CTRL with eventID=0x%x!\n", eventID));

	if (WDEV_WSC_AP(wdev))
		CurOpMode = AP_MODE;
	else
		CurOpMode = STA_MODE;

	os_alloc_mem(NULL, (UCHAR **)&pWscData, WSC_MAX_DATA_LEN);

	if (pWscData != NULL) {
		NdisZeroMemory(pWscData, WSC_MAX_DATA_LEN);
		dataLen = BuildMessageNACK(pAd, pWscControl, pWscData);
		WscSendUPnPMessage(pAd, (pWscControl->EntryIfIdx & 0x0F),
						   WSC_OPCODE_UPNP_DATA, WSC_UPNP_DATA_SUB_NORMAL,
						   pWscData, dataLen, eventID, 0, NULL, CurOpMode);
		os_free_mem(pWscData);
	} else {
		WscSendUPnPMessage(pAd, (pWscControl->EntryIfIdx & 0x0F),
						   WSC_OPCODE_UPNP_CTRL, 0, NULL, 0, eventID, 0, NULL, CurOpMode);
	}
}

/*
*	Format of iwcustom msg WSC clientJoin message:
*		1. SSID which station want to probe(32 bytes):
*			<SSID string>
*			*If the length if SSID string is small than 32 bytes, fill 0x0 for remaining bytes.
*		2. sender MAC address(6 bytes):
*		3. Status:
*			Set as 1 means change APStatus as 1.
*			Set as 2 means change STAStatus as 1.
*			Set as 3 means trigger msg.
*
*			32         6        1
*		+----------+--------+------+
*		|SSIDString| SrcMAC |Status|
*/
int WscSendUPnPConfReqMsg(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR apIdx,
	IN PUCHAR ssidStr,
	IN PUCHAR macAddr,
	IN INT Status,
	IN UINT eventID,
	IN UCHAR CurOpMode)
{
	#define WSC_JOIN_MSG_LEN (MAX_LEN_OF_SSID + MAC_ADDR_LEN + 1)

	UCHAR pData[WSC_JOIN_MSG_LEN] = {0};
	strncpy((RTMP_STRING *) pData, (RTMP_STRING *)ssidStr, MAX_LEN_OF_SSID);
	NdisMoveMemory(&pData[MAX_LEN_OF_SSID], macAddr, MAC_ADDR_LEN);
	pData[MAX_LEN_OF_SSID+MAC_ADDR_LEN] = Status;
	WscSendUPnPMessage(pAd, apIdx, WSC_OPCODE_UPNP_MGMT, WSC_UPNP_MGMT_SUB_CONFIG_REQ,
							&pData[0], WSC_JOIN_MSG_LEN, eventID, 0, NULL, CurOpMode);
	return 0;
}


/*
*	NETLINK tunnel msg format send to WSCUPnP handler in user space:
*	1. Signature of following string(Not include the quote, 8 bytes)
*			"RAWSCMSG"
*	2. eID: eventID (4 bytes)
*			the ID of this message(4 bytes)
*	3. aID: ackID (4 bytes)
*			means that which event ID this mesage was response to.
*	4. TL:  Message Total Length (4 bytes)
*			Total length of this message.
*	5. F:   Flag (2 bytes)
*			used to notify some specific character of this msg segment.
*				Bit 1: fragment
*					set as 1 if netlink layer have more segment of this Msg need to send.
*				Bit 2~15: reserve, should set as 0 now.
*	5. SL:  Segment Length(2 bytes)
*			msg actual length in this segment, The SL may not equal the "TL" field if "F" ==1
*	6. devMac: device mac address(6 bytes)
*			Indicate the netdevice which this msg belong. For the wscd in user space will
*			depends this address dispatch the msg to correct UPnP Device instance to handle it.
*	7. "WSC_MSG" info:
*
*		 8                 4       4       4      2    2        6      variable length(MAXIMUM=232)
*	+------------+----+----+----+--+--+------+------------------------+
*	|  Signature       |eID  |aID  | TL   | F | SL|devMac| WSC_MSG                          |
*
*/
int WscSendUPnPMessage(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR devIfIdx,
	IN USHORT msgType,
	IN USHORT msgSubType,
	IN PUCHAR pData,
	IN INT dataLen,
	IN UINT eventID,
	IN UINT toIPAddr,
	IN PUCHAR pMACAddr,
	IN UCHAR CurOpMode)
{
	/*	union iwreq_data wrqu; */
	RTMP_WSC_NLMSG_HDR *pNLMsgHdr;
	RTMP_WSC_MSG_HDR *pWscMsgHdr;
	UCHAR hdrBuf[42]; /*RTMP_WSC_NLMSG_HDR_LEN + RTMP_WSC_MSG_HDR_LEN */
	int totalLen, leftLen, copyLen;
	PUCHAR pBuf = NULL, pBufPtr = NULL, pPos = NULL;
	PUCHAR	pDevAddr = NULL;
#ifdef CONFIG_AP_SUPPORT
	UCHAR	bssIdx = devIfIdx;
#endif /* CONFIG_AP_SUPPORT */
	ULONG Now;

	if (!pMACAddr)
		return FALSE;

	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("-----> WscSendUPnPMessage\n"));

	if ((msgType & WSC_OPCODE_UPNP_MASK) != WSC_OPCODE_UPNP_MASK)
		return FALSE;

#ifdef CONFIG_AP_SUPPORT

	if (CurOpMode == AP_MODE) {
#ifdef APCLI_SUPPORT

		if (devIfIdx & MIN_NET_DEVICE_FOR_APCLI) {
			bssIdx &= (~MIN_NET_DEVICE_FOR_APCLI);

			if (bssIdx >= MAX_APCLI_NUM)
				return FALSE;

			if (bssIdx < MAX_APCLI_NUM)
				pDevAddr = &pAd->ApCfg.ApCliTab[bssIdx].wdev.if_addr[0];
		} else
#endif /* APCLI_SUPPORT */
			pDevAddr = &pAd->ApCfg.MBSSID[bssIdx].wdev.bssid[0];
	}

#endif /* CONFIG_AP_SUPPORT */

	if (pDevAddr == NULL) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, ("pDevAddr == NULL!\n"));
		return FALSE;
	}

	/*Prepare the NLMsg header */
	memset(hdrBuf, 0, sizeof(hdrBuf));
	pNLMsgHdr = (RTMP_WSC_NLMSG_HDR *)hdrBuf;
	memcpy(pNLMsgHdr, WSC_MSG_SIGNATURE, RTMP_WSC_NLMSG_SIGNATURE_LEN);
	NdisGetSystemUpTime(&Now);
	pNLMsgHdr->envID = Now;
	pNLMsgHdr->ackID = eventID;
	pNLMsgHdr->msgLen = dataLen + RTMP_WSC_MSG_HDR_LEN;
	/*
	*	In order to support multiple wscd, we need this new field to notify
	*	the wscd which interface this msg send from.
	*/
	NdisMoveMemory(&pNLMsgHdr->devAddr[0],  pDevAddr, MAC_ADDR_LEN);
	/*Prepare the WscMsg header */
	pWscMsgHdr = (RTMP_WSC_MSG_HDR *)(hdrBuf + sizeof(RTMP_WSC_NLMSG_HDR));

	switch (msgType) {
	case WSC_OPCODE_UPNP_DATA:
		pWscMsgHdr->msgType = WSC_OPCODE_UPNP_DATA;
		break;

	case WSC_OPCODE_UPNP_MGMT:
		pWscMsgHdr->msgType = WSC_OPCODE_UPNP_MGMT;
		break;

	case WSC_OPCODE_UPNP_CTRL:
		pWscMsgHdr->msgType = WSC_OPCODE_UPNP_CTRL;
		break;

	default:
		return FALSE;
	}

	pWscMsgHdr->msgSubType = msgSubType;
	pWscMsgHdr->ipAddr = toIPAddr;
	pWscMsgHdr->msgLen = dataLen;

	if ((pWscMsgHdr->msgType == WSC_OPCODE_UPNP_DATA) &&
		(eventID == 0) && (NdisEqualMemory(pMACAddr, ZERO_MAC_ADDR, MAC_ADDR_LEN) == FALSE)) {
		pWscMsgHdr->msgSubType |= WSC_UPNP_DATA_SUB_INCLUDE_MAC;
		pNLMsgHdr->msgLen += MAC_ADDR_LEN;
		pWscMsgHdr->msgLen += MAC_ADDR_LEN;
	}

	/*Allocate memory and copy the msg. */
	totalLen = leftLen = pNLMsgHdr->msgLen;
	pPos = pData;
	os_alloc_mem(NULL, (UCHAR **)&pBuf, IWEVCUSTOM_MSG_MAX_LEN);

	if (pBuf != NULL) {
		int firstSeg = 1;

		while (leftLen) {
			/*Prepare the payload */
			memset(pBuf, 0, IWEVCUSTOM_MSG_MAX_LEN);
			pNLMsgHdr->segLen = (leftLen > IWEVCUSTOM_PAYLOD_MAX_LEN ? IWEVCUSTOM_PAYLOD_MAX_LEN : leftLen);
			leftLen -= pNLMsgHdr->segLen;
			pNLMsgHdr->flags = (leftLen > 0 ? 1 : 0);
			memcpy(pBuf, pNLMsgHdr, RTMP_WSC_NLMSG_HDR_LEN);
			pBufPtr = &pBuf[RTMP_WSC_NLMSG_HDR_LEN];

			if (firstSeg) {
				memcpy(pBufPtr, pWscMsgHdr, RTMP_WSC_MSG_HDR_LEN);
				pBufPtr += RTMP_WSC_MSG_HDR_LEN;
				copyLen = (pNLMsgHdr->segLen - RTMP_WSC_MSG_HDR_LEN);

				if ((pWscMsgHdr->msgSubType & WSC_UPNP_DATA_SUB_INCLUDE_MAC) == WSC_UPNP_DATA_SUB_INCLUDE_MAC) {
					NdisMoveMemory(pBufPtr, pMACAddr, MAC_ADDR_LEN);
					pBufPtr += MAC_ADDR_LEN;
					copyLen -= MAC_ADDR_LEN;
				}

				NdisMoveMemory(pBufPtr, pPos, copyLen);
				pPos += copyLen;
				firstSeg = 0;
			} else {
				NdisMoveMemory(pBufPtr, pPos, pNLMsgHdr->segLen);
				pPos += pNLMsgHdr->segLen;
			}

			/*Send WSC Msg to wscd, msg length = pNLMsgHdr->segLen + sizeof(RTMP_WSC_NLMSG_HDR) */
			RtmpOSWrielessEventSend(pAd->net_dev, RT_WLAN_EVENT_CUSTOM, RT_WSC_UPNP_EVENT_FLAG, NULL, pBuf, pNLMsgHdr->segLen + sizeof(RTMP_WSC_NLMSG_HDR));
		}

		os_free_mem(pBuf);
	}

	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("<----- WscSendUPnPMessage\n"));
	return TRUE;
}


VOID WscSendMessage(
	IN  PRTMP_ADAPTER pAdapter,
	IN  UCHAR OpCode,
	IN  PUCHAR pData,
	IN  INT Len,
	IN  PWSC_CTRL pWscControl,
	IN  UCHAR OpMode,
	IN  UCHAR EapType)
{
	/* Inb-EAP Message */
	UCHAR Header802_3[14];
	USHORT Length, MsgLen;
	IEEE8021X_FRAME Ieee_8021x;
	EAP_FRAME EapFrame;
	WSC_FRAME WscFrame;
	UCHAR *pOutBuffer = NULL;
	ULONG FrameLen = 0;
	MAC_TABLE_ENTRY *pEntry;
#ifdef CONFIG_AP_SUPPORT
	UCHAR bssIdx = (pWscControl->EntryIfIdx & 0x0F);
#endif /* CONFIG_AP_SUPPORT */
	UCHAR CurOpMode = 0xFF;

	if ((Len <= 0) && (OpCode != WSC_OPCODE_START) && (OpCode != WSC_OPCODE_FRAG_ACK))
		return;

	/* Send message */
	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("-----> WscSendMessage\n"));
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAdapter)
	CurOpMode = AP_MODE;
#endif /* CONFIG_AP_SUPPORT */
	NdisZeroMemory(Header802_3, sizeof(UCHAR) * 14);
#ifdef CONFIG_AP_SUPPORT

	if (CurOpMode == AP_MODE) {
		if (OpMode == AP_MODE) {
			if (bssIdx < HW_BEACON_MAX_NUM)
				MAKE_802_3_HEADER(Header802_3, &pWscControl->EntryAddr[0], &pAdapter->ApCfg.MBSSID[bssIdx].wdev.bssid[0], EAPOL);
		}

#ifdef APCLI_SUPPORT
		else if (OpMode == AP_CLIENT_MODE) {
			if (bssIdx < MAX_APCLI_NUM)
				MAKE_802_3_HEADER(Header802_3, &pWscControl->EntryAddr[0], &pAdapter->ApCfg.ApCliTab[bssIdx].wdev.if_addr[0], EAPOL);
		}

#endif /* APCLI_SUPPORT */
	}

#endif /* CONFIG_AP_SUPPORT */
	/* Length = EAP + WSC_Frame + Payload */
	Length = sizeof(EAP_FRAME) + sizeof(WSC_FRAME) + Len;

	if (pWscControl->bWscFragment && (pWscControl->bWscFirstOne)) {
		Length += 2;
		MsgLen = pWscControl->WscTxBufLen + Len;
		MsgLen = htons(MsgLen);
	}

	/* Zero 802.1x body */
	NdisZeroMemory(&Ieee_8021x, sizeof(Ieee_8021x));
	Ieee_8021x.Version = EAPOL_VER;
	Ieee_8021x.Type    = EAPPacket;
	Ieee_8021x.Length  = cpu2be16(Length);
	/* Zero EAP frame */
	NdisZeroMemory(&EapFrame, sizeof(EapFrame));

	if (EapType == EAP_CODE_REQ) {
		EapFrame.Code   = EAP_CODE_REQ;
		EapFrame.Id     = ++(pWscControl->lastId);
	} else {
		EapFrame.Code   = EAP_CODE_RSP;
		EapFrame.Id     = pWscControl->lastId; /* same as eap_req id */
	}

	EapFrame.Length = cpu2be16(Length);
	EapFrame.Type   = EAP_TYPE_WSC;
	/* Zero WSC Frame */
	NdisZeroMemory(&WscFrame, sizeof(WscFrame));
	WscFrame.SMI[0] = 0x00;
	WscFrame.SMI[1] = 0x37;
	WscFrame.SMI[2] = 0x2A;
	WscFrame.VendorType = cpu2be32(WSC_VENDOR_TYPE);
	WscFrame.OpCode = OpCode;
	WscFrame.Flags  = 0x00;

	if (pWscControl->bWscFragment && (pWscControl->bWscLastOne == FALSE))
		WscFrame.Flags  |= WSC_MSG_FLAG_MF;

	if (pWscControl->bWscFragment && (pWscControl->bWscFirstOne))
		WscFrame.Flags  |= WSC_MSG_FLAG_LF;

	/* Out buffer for transmitting message */
	os_alloc_mem(NULL, (UCHAR **)&pOutBuffer, MAX_LEN_OF_MLME_BUFFER);

	if (pOutBuffer == NULL)
		return;

	FrameLen = 0;

	/* Make	 Transmitting frame */
	if (pData && (Len > 0)) {
		if (pWscControl->bWscFragment && (pWscControl->bWscFirstOne)) {
			UCHAR	LF_Len = 2;
			ULONG	TmpLen = 0;

			pWscControl->bWscFirstOne = FALSE;
			MakeOutgoingFrame(pOutBuffer, &TmpLen,
							  sizeof(IEEE8021X_FRAME), &Ieee_8021x,
							  sizeof(EapFrame), &EapFrame,
							  sizeof(WscFrame), &WscFrame,
							  LF_Len, &MsgLen,
							  END_OF_ARGS);
			FrameLen += TmpLen;
			MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpLen,
							  Len, pData,
							  END_OF_ARGS);
			FrameLen += TmpLen;
		} else {
			MakeOutgoingFrame(pOutBuffer, &FrameLen,
							  sizeof(IEEE8021X_FRAME), &Ieee_8021x,
							  sizeof(EapFrame), &EapFrame,
							  sizeof(WscFrame), &WscFrame,
							  Len, pData,
							  END_OF_ARGS);
		}
	} else
		MakeOutgoingFrame(pOutBuffer, &FrameLen,
						  sizeof(IEEE8021X_FRAME), &Ieee_8021x,
						  sizeof(EapFrame), &EapFrame,
						  sizeof(WscFrame), &WscFrame,
						  END_OF_ARGS);

	/* Copy frame to Tx ring */
#ifdef CONFIG_AP_SUPPORT
	pEntry = MacTableLookup(pAdapter, &pWscControl->EntryAddr[0]);
#endif

	if (pEntry)
		RTMPToWirelessSta(pAdapter, pEntry, Header802_3, sizeof(Header802_3), (PUCHAR)pOutBuffer, FrameLen, TRUE);
	else
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_WARN, ("pEntry is NULL\n"));

	if (pOutBuffer)
		os_free_mem(pOutBuffer);

	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("<----- WscSendMessage\n"));
}

VOID WscBuildBeaconIE(
	IN  PRTMP_ADAPTER pAd,
	IN  UCHAR b_configured,
	IN  BOOLEAN b_selRegistrar,
	IN  USHORT devPwdId,
	IN  USHORT selRegCfgMethods,
	IN  UCHAR apidx,
	IN  UCHAR *pAuthorizedMACs,
	IN  UCHAR AuthorizedMACsLen,
	IN  UCHAR CurOpMode)
{
	WSC_IE_HEADER ieHdr;
	UCHAR *Data = NULL;
	PUCHAR pData;
	INT Len = 0, templen = 0;
	USHORT tempVal = 0;
	PWSC_CTRL pWpsCtrl = NULL;
	PWSC_REG_DATA pReg = NULL;
	struct wifi_dev *wdev = NULL;
	UCHAR inf_idx;
	/* allocate memory */
	os_alloc_mem(NULL, (UCHAR **)&Data, 256);

	if (Data == NULL) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, ("%s: Allocate memory fail!!!\n", __func__));
		return;
	}

	inf_idx = apidx & 0x0F;
#ifdef CONFIG_AP_SUPPORT

	if (CurOpMode == AP_MODE) {
		if (inf_idx < HW_BEACON_MAX_NUM)
			wdev = &pAd->ApCfg.MBSSID[inf_idx].wdev;
	}

#endif /* CONFIG_AP_SUPPORT */

	if (wdev)
		pWpsCtrl = &wdev->WscControl;

	if (!pWpsCtrl || !wdev) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR,
				 ("%s: pWpsCtrl or wdev is NULL!!!\n", __func__));
		os_free_mem(Data);
		return;
	}

	pReg = &pWpsCtrl->RegData;
	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("-----> WscBuildBeaconIE\n"));
	/* WSC IE HEader */
	ieHdr.elemId = 221;
	ieHdr.length = 4;
	ieHdr.oui[0] = 0x00;
	ieHdr.oui[1] = 0x50;
	ieHdr.oui[2] = 0xF2;
		ieHdr.oui[3] = 0x04;

	pData = (PUCHAR) &Data[0];
	Len = 0;
	/* 1. Version */
	templen = AppendWSCTLV(WSC_ID_VERSION, pData, &pReg->SelfInfo.Version, 0);
	pData += templen;
	Len   += templen;
	/* 2. Simple Config State */
	templen = AppendWSCTLV(WSC_ID_SC_STATE, pData, (UINT8 *)&b_configured, 0);
	pData += templen;
	Len   += templen;
#ifdef CONFIG_AP_SUPPORT
#ifdef WSC_V2_SUPPORT

	if ((CurOpMode == AP_MODE) && pWpsCtrl->bSetupLock) {
		/* AP Setup Lock */
		templen = AppendWSCTLV(WSC_ID_AP_SETUP_LOCKED, pData, (UINT8 *)&pWpsCtrl->bSetupLock, 0);
		pData += templen;
		Len   += templen;
	}

#endif /* WSC_V2_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

	if (b_selRegistrar) {
		/* 3.Selected Registrar */
		templen = AppendWSCTLV(WSC_ID_SEL_REGISTRAR, pData, (UINT8 *)&b_selRegistrar, 0);
		pData += templen;
		Len   += templen;
		/*4. Device Password ID */
		tempVal = htons(devPwdId);
		templen = AppendWSCTLV(WSC_ID_DEVICE_PWD_ID, pData, (UINT8 *)&tempVal, 0);
		pData += templen;
		Len   += templen;
		/* 5. Selected Registrar Config Methods */
		tempVal = selRegCfgMethods;
		tempVal = htons(tempVal);
		templen = AppendWSCTLV(WSC_ID_SEL_REG_CFG_METHODS, pData, (UINT8 *)&tempVal, 0);
		pData += templen;
		Len   += templen;
	}

	/* 6. UUID last 6 bytes use MAC */
	templen = AppendWSCTLV(WSC_ID_UUID_E, pData, &pWpsCtrl->Wsc_Uuid_E[0], 0);
	pData += templen;
	Len   += templen;

	/* 7. RF Bands */
#ifdef CONFIG_AP_SUPPORT
	if (CurOpMode == AP_MODE) {
		if (wdev && WMODE_CAP_5G(wdev->PhyMode))
			tempVal = 2;
		else
			tempVal = 1;
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef RT_BIG_ENDIAN
	tempVal = SWAP16(tempVal);
#endif /* RT_BIG_ENDIAN */
	templen = AppendWSCTLV(WSC_ID_RF_BAND, pData, (UINT8 *)&tempVal, 0);
	pData += templen;
	Len   += templen;
#ifdef WSC_V2_SUPPORT

	if (pWpsCtrl->WscV2Info.bEnableWpsV2) {
		PWSC_TLV pWscTLV = &pWpsCtrl->WscV2Info.ExtraTlv;

		WscGenV2Msg(pWpsCtrl,
					b_selRegistrar,
					pAuthorizedMACs,
					AuthorizedMACsLen,
					&pData,
					&Len);

		/* Extra attribute that is not defined in WSC Sepc. */
		if (pWscTLV->pTlvData && pWscTLV->TlvLen) {
			templen = AppendWSCTLV(pWscTLV->TlvTag, pData, (UINT8 *)pWscTLV->pTlvData, pWscTLV->TlvLen);
			pData += templen;
			Len   += templen;
		}
	}

#endif /* WSC_V2_SUPPORT */
	ieHdr.length = ieHdr.length + Len;
	NdisCopyMemory(wdev->WscIEBeacon.Value, &ieHdr, sizeof(WSC_IE_HEADER));
	NdisCopyMemory(wdev->WscIEBeacon.Value + sizeof(WSC_IE_HEADER), Data, Len);
	wdev->WscIEBeacon.ValueLen = sizeof(WSC_IE_HEADER) + Len;

	if (Data != NULL)
		os_free_mem(Data);

	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("<----- WscBuildBeaconIE\n"));
}

VOID WscBuildProbeRespIE(
	IN  PRTMP_ADAPTER	pAd,
	IN  UCHAR respType,
	IN  UCHAR scState,
	IN  BOOLEAN b_selRegistrar,
	IN  USHORT devPwdId,
	IN  USHORT selRegCfgMethods,
	IN  UCHAR apidx,
	IN  UCHAR *pAuthorizedMACs,
	IN  INT AuthorizedMACsLen,
	IN  UCHAR CurOpMode)
{
	WSC_IE_HEADER ieHdr;
	UCHAR *Data = NULL;
	PUCHAR pData;
	INT Len = 0, templen = 0;
	USHORT tempVal = 0;
	PWSC_CTRL pWpsCtrl = NULL;
	PWSC_REG_DATA pReg = NULL;
	struct wifi_dev *wdev = NULL;
	UCHAR inf_idx;

	/* allocate memory */
	os_alloc_mem(NULL, (UCHAR **)&Data, 512);

	if (Data == NULL) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR,
			("%s: Allocate memory fail!!!\n", __func__));
		return;
	}

	inf_idx = apidx & 0x0F;
#ifdef CONFIG_AP_SUPPORT

	if (CurOpMode == AP_MODE)
		wdev = &pAd->ApCfg.MBSSID[inf_idx].wdev;

#endif /* CONFIG_AP_SUPPORT */

	if (!wdev) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR,
				 ("%s: Wdev is NULL!!!\n", __func__));
		os_free_mem(Data);
		return;
	}

	pWpsCtrl = &wdev->WscControl;
	pReg = &pWpsCtrl->RegData;
	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE,
			 ("-----> %s:: apidx = %x\n", __func__, apidx));
	/* WSC IE Header */
	ieHdr.elemId = 221;
	ieHdr.length = 4;
	ieHdr.oui[0] = 0x00;
	ieHdr.oui[1] = 0x50;
	ieHdr.oui[2] = 0xF2;
		ieHdr.oui[3] = 0x04;

	pData = (PUCHAR) &Data[0];
	Len = 0;
	/* 1. Version */
	templen = AppendWSCTLV(WSC_ID_VERSION, pData, &pReg->SelfInfo.Version, 0);
	pData += templen;
	Len   += templen;
	/* 2. Simple Config State */
	templen = AppendWSCTLV(WSC_ID_SC_STATE, pData, (UINT8 *)&scState, 0);
	pData += templen;
	Len   += templen;
#ifdef CONFIG_AP_SUPPORT
#ifdef WSC_V2_SUPPORT

	if ((CurOpMode == AP_MODE) && pWpsCtrl->bSetupLock) {
		/* AP Setup Lock */
		templen = AppendWSCTLV(WSC_ID_AP_SETUP_LOCKED, pData, (UINT8 *)&pWpsCtrl->bSetupLock, 0);
		pData += templen;
		Len   += templen;
	}

#endif /* WSC_V2_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

	if (b_selRegistrar) {
		/* 3. Selected Registrar */
		templen = AppendWSCTLV(WSC_ID_SEL_REGISTRAR, pData, (UINT8 *)&b_selRegistrar, 0);
		pData += templen;
		Len   += templen;
		/* 4. Device Password ID */
		tempVal = htons(devPwdId);
		templen = AppendWSCTLV(WSC_ID_DEVICE_PWD_ID, pData, (UINT8 *)&tempVal, 0);
		pData += templen;
		Len   += templen;
		/* 5. Selected Registrar Config Methods */
		tempVal = htons(selRegCfgMethods);
		templen = AppendWSCTLV(WSC_ID_SEL_REG_CFG_METHODS, pData, (UINT8 *)&tempVal, 0);
		pData += templen;
		Len   += templen;
	}

	/* 6. Response Type WSC_ID_RESP_TYPE */
	templen = AppendWSCTLV(WSC_ID_RESP_TYPE, pData, (UINT8 *)&respType, 0);
	pData += templen;
	Len   += templen;
	/* 7. UUID last 6 bytes use MAC */
	templen = AppendWSCTLV(WSC_ID_UUID_E, pData, &pWpsCtrl->Wsc_Uuid_E[0], 0);
	pData += templen;
	Len   += templen;
	/* 8. Manufacturer */
	NdisZeroMemory(pData, 64 + 4);
	templen = AppendWSCTLV(WSC_ID_MANUFACTURER, pData,  pReg->SelfInfo.Manufacturer, strlen((RTMP_STRING *) pReg->SelfInfo.Manufacturer));
	pData += templen;
	Len   += templen;
	/* 9. Model Name */
	NdisZeroMemory(pData, 32 + 4);
	templen = AppendWSCTLV(WSC_ID_MODEL_NAME, pData, pReg->SelfInfo.ModelName, strlen((RTMP_STRING *) pReg->SelfInfo.ModelName));
	pData += templen;
	Len   += templen;
	/* 10. Model Number */
	NdisZeroMemory(pData, 32 + 4);
	templen = AppendWSCTLV(WSC_ID_MODEL_NUMBER, pData, pReg->SelfInfo.ModelNumber, strlen((RTMP_STRING *) pReg->SelfInfo.ModelNumber));
	pData += templen;
	Len   += templen;
	/* 11. Serial Number */
	NdisZeroMemory(pData, 32 + 4);
	templen = AppendWSCTLV(WSC_ID_SERIAL_NUM, pData, pReg->SelfInfo.SerialNumber, strlen((RTMP_STRING *) pReg->SelfInfo.SerialNumber));
	pData += templen;
	Len   += templen;
	/* 12. Primary Device Type */
	templen = AppendWSCTLV(WSC_ID_PRIM_DEV_TYPE, pData, pReg->SelfInfo.PriDeviceType, 0);
	pData += templen;
	Len   += templen;
	/* 13. Device Name */
	NdisZeroMemory(pData, 32 + 4);
	templen = AppendWSCTLV(WSC_ID_DEVICE_NAME, pData, pReg->SelfInfo.DeviceName, strlen((RTMP_STRING *) pReg->SelfInfo.DeviceName));
	pData += templen;
	Len   += templen;
	/* 14. Config Methods */
	/*tempVal = htons(0x008a); */
	/*tempVal = htons(0x0084); */
	{
		/*
		*	WSC 1.0 WCN logo testing will check the value of config method in probe response and M1.
		*	Config method shall be identical in probe response and M1.
		*/
#ifdef WSC_V2_SUPPORT
		if (pWpsCtrl->WscV2Info.bEnableWpsV2)
			tempVal = pWpsCtrl->WscConfigMethods & 0xF97F;
		else
#endif /* WSC_V2_SUPPORT */
			tempVal = pWpsCtrl->WscConfigMethods & 0x00FF;
	}

	tempVal = htons(tempVal);
	templen = AppendWSCTLV(WSC_ID_CONFIG_METHODS, pData, (UINT8 *)&tempVal, 0);
	pData += templen;
	Len   += templen;

	/* 15. RF Bands */
#ifdef CONFIG_AP_SUPPORT
	if (CurOpMode == AP_MODE) {
		if (WMODE_CAP_5G(wdev->PhyMode))
			tempVal = 2;
		else
			tempVal = 1;
	}
#endif /* CONFIG_AP_SUPPORT */

#ifdef RT_BIG_ENDIAN
	tempVal = SWAP16(tempVal);
#endif /* RT_BIG_ENDIAN */
	templen = AppendWSCTLV(WSC_ID_RF_BAND, pData, (UINT8 *)&tempVal, 0);
	pData += templen;
	Len   += templen;
#ifdef WSC_V2_SUPPORT

	if (pWpsCtrl->WscV2Info.bEnableWpsV2) {
		PWSC_TLV pWscTLV = &pWpsCtrl->WscV2Info.ExtraTlv;

		WscGenV2Msg(pWpsCtrl,
					b_selRegistrar,
					pAuthorizedMACs,
					AuthorizedMACsLen,
					&pData,
					&Len);

		/* Extra attribute that is not defined in WSC Sepc. */
		if (pWscTLV->pTlvData && pWscTLV->TlvLen) {
			templen = AppendWSCTLV(pWscTLV->TlvTag, pData, (UINT8 *)pWscTLV->pTlvData, pWscTLV->TlvLen);
			pData += templen;
			Len   += templen;
		}
	}

#endif /* WSC_V2_SUPPORT */

	if (Len > 251)
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("Len is overflow!\n"));

	ieHdr.length = ieHdr.length + Len;
	NdisCopyMemory(wdev->WscIEProbeResp.Value, &ieHdr, sizeof(WSC_IE_HEADER));
	NdisCopyMemory(wdev->WscIEProbeResp.Value + sizeof(WSC_IE_HEADER), Data, Len);
	wdev->WscIEProbeResp.ValueLen = sizeof(WSC_IE_HEADER) + Len;

	if (Data != NULL)
		os_free_mem(Data);

	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE,
			 ("<----- %s\n", __func__));
}

/*
*	========================================================================
*
*	Routine Description:
*		Ap send EAP-Fail to station
*
*	Arguments:
*		pAd    - NIC Adapter pointer
*		Id			- ID between EAP-Req and EAP-Rsp pair
*		pEntry		- The Station Entry information
*
*	Return Value:
*		None
*
*	========================================================================
*/
VOID WscSendEapFail(
	IN  PRTMP_ADAPTER pAd,
	IN  PWSC_CTRL pWscControl,
	IN  BOOLEAN bSendDeAuth)
{
	UCHAR Header802_3[14];
	USHORT Length;
	IEEE8021X_FRAME Ieee_8021x;
	EAP_FRAME EapFrame;
	UCHAR *pOutBuffer = NULL;
	ULONG FrameLen = 0;
#ifdef CONFIG_AP_SUPPORT
	UCHAR apidx = (pWscControl->EntryIfIdx & 0x0F);
#endif /* CONFIG_AP_SUPPORT */
	MAC_TABLE_ENTRY *pEntry;
	UCHAR CurOpMode = 0xFF;

	NdisZeroMemory(Header802_3, sizeof(UCHAR) * 14);
	/* 1. Send EAP-Rsp Id */
	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE,
		("-----> WscSendEapFail\n"));
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	CurOpMode = AP_MODE;
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_AP_SUPPORT

	if (CurOpMode == AP_MODE) {
		MAKE_802_3_HEADER(Header802_3,
						  &pWscControl->EntryAddr[0],
						  &pAd->ApCfg.MBSSID[apidx].wdev.bssid[0],
						  EAPOL);
	}

#endif /* CONFIG_AP_SUPPORT */
	/* Length, -1 type size, Eap-Fail doesn't need Type item */
	Length = sizeof(EAP_FRAME) - sizeof(UCHAR);
	/* Zero 802.1x body */
	NdisZeroMemory(&Ieee_8021x, sizeof(Ieee_8021x));
	Ieee_8021x.Version = EAPOL_VER;
	Ieee_8021x.Type    = EAPPacket;
	Ieee_8021x.Length  = cpu2be16(Length);
	/* Zero EAP frame */
	NdisZeroMemory(&EapFrame, sizeof(EapFrame));
	EapFrame.Code   = EAP_CODE_FAIL;
	EapFrame.Id     = pWscControl->lastId;
	EapFrame.Length = cpu2be16(Length);
	/* Out buffer for transmitting EAP-Req(Identity) */
	os_alloc_mem(NULL, (UCHAR **)&pOutBuffer, MAX_LEN_OF_MLME_BUFFER);

	if (pOutBuffer == NULL)
		return;

	FrameLen = 0;
	/* Make	 Transmitting frame */
	MakeOutgoingFrame(pOutBuffer, &FrameLen,
					  sizeof(IEEE8021X_FRAME), &Ieee_8021x,
					  sizeof(EapFrame) - 1, &EapFrame, END_OF_ARGS);
#ifdef CONFIG_AP_SUPPORT
	pEntry = MacTableLookup(pAd, &pWscControl->EntryAddr[0]);
#endif
	/* Copy frame to Tx ring */
	RTMPToWirelessSta(pAd, pEntry, Header802_3, sizeof(Header802_3), (PUCHAR)pOutBuffer, FrameLen, TRUE);

	if (pOutBuffer)
		os_free_mem(pOutBuffer);

#ifdef CONFIG_AP_SUPPORT

	if (pEntry && bSendDeAuth && (CurOpMode == AP_MODE))
		MlmeDeAuthAction(pAd, pEntry, REASON_DEAUTH_STA_LEAVING, TRUE);

	if (pEntry == NULL) {
		/*
		*	If STA dis-connect un-normally, reset EntryAddr here.
		*/
		NdisZeroMemory(pWscControl->EntryAddr, MAC_ADDR_LEN);
	}

#endif /* CONFIG_AP_SUPPORT */
	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("<----- WscSendEapFail\n"));
}

#ifdef CONFIG_AP_SUPPORT
VOID WscBuildAssocRespIE(
	IN  PRTMP_ADAPTER pAd,
	IN  UCHAR ApIdx,
	IN  UCHAR Reason,
	OUT PUCHAR pOutBuf,
	OUT PUCHAR pIeLen)
{
	WSC_IE_HEADER ieHdr;
	UCHAR *Data = NULL;
	PUCHAR pData;
	INT Len = 0, templen = 0;
	UINT8 tempVal = 0;
	PWSC_REG_DATA pReg;
	struct wifi_dev *wdev;
	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("-----> WscBuildAssocRespIE\n"));
	/* allocate memory */
	os_alloc_mem(NULL, (UCHAR **)&Data, 512);

	if (Data == NULL) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, ("%s: Allocate memory fail!!!\n", __func__));
		return;
	}

	wdev = &pAd->ApCfg.MBSSID[ApIdx].wdev;
	pReg = &wdev->WscControl.RegData;
	Data[0] = 0;
	/* WSC IE Header */
	ieHdr.elemId = 221;
	ieHdr.length = 4;
	ieHdr.oui[0] = 0x00;
	ieHdr.oui[1] = 0x50;
	ieHdr.oui[2] = 0xF2;
	ieHdr.oui[3] = 0x04;
	pData = (PUCHAR) &Data[0];
	Len = 0;
	/* Version */
	templen = AppendWSCTLV(WSC_ID_VERSION, pData, &pReg->SelfInfo.Version, 0);
	pData += templen;
	Len   += templen;
	/* Request Type */
	tempVal = WSC_MSGTYPE_AP_WLAN_MGR;
	templen = AppendWSCTLV(WSC_ID_RESP_TYPE, pData, (UINT8 *)&tempVal, 0);
	pData += templen;
	Len   += templen;
#ifdef WSC_V2_SUPPORT

	if (wdev->WscControl.WscV2Info.bEnableWpsV2) {
		WscGenV2Msg(&wdev->WscControl,
					FALSE,
					NULL,
					0,
					&pData,
					&Len);
	}

#endif /* WSC_V2_SUPPORT */
	ieHdr.length = ieHdr.length + Len;
	NdisMoveMemory(pOutBuf, &ieHdr, sizeof(WSC_IE_HEADER));
	NdisMoveMemory(pOutBuf + sizeof(WSC_IE_HEADER), Data, Len);
	*pIeLen = sizeof(WSC_IE_HEADER) + Len;

	if (Data != NULL)
		os_free_mem(Data);

	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("<----- WscBuildAssocRespIE\n"));
}


VOID WscSelectedRegistrar(
	IN  PRTMP_ADAPTER pAd,
	IN  PUCHAR pReginfo,
	IN  UINT Length,
	IN  UCHAR apidx)
{
	PUCHAR pData;
	INT IsAPConfigured;
	UCHAR wsc_version, wsc_sel_reg = 0;
	USHORT wsc_dev_pass_id = 0, wsc_sel_reg_conf_mthd = 0;
	USHORT WscType, WscLen;
	PUCHAR pAuthorizedMACs = NULL;
	UCHAR AuthorizedMACsLen = 0;
	PWSC_CTRL pWscCtrl;
	struct wifi_dev *wdev;

	pData = (PUCHAR)pReginfo;

	if (Length < 4) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("WscSelectedRegistrar --> Unknown IE\n"));
		return;
	}

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	pWscCtrl = &wdev->WscControl;
	hex_dump("WscSelectedRegistrar - Reginfo", pReginfo, Length);

	while (Length > 4) {
		/* arm-cpu has packet alignment issue, it's better to use memcpy to retrieve data */
		NdisMoveMemory(&WscType, pData, 2);
		NdisMoveMemory(&WscLen,  pData + 2, 2);
		WscLen = ntohs(WscLen);
		pData  += 4;
		Length -= 4;

		switch (ntohs(WscType)) {
		case WSC_ID_VERSION:
			wsc_version = *pData;
			break;

		case WSC_ID_SEL_REGISTRAR:
			wsc_sel_reg = *pData;
			break;

		case WSC_ID_DEVICE_PWD_ID:
			NdisMoveMemory(&wsc_dev_pass_id, pData, sizeof(USHORT));
			wsc_dev_pass_id = be2cpu16(wsc_dev_pass_id);
			break;

		case WSC_ID_SEL_REG_CFG_METHODS:
			NdisMoveMemory(&wsc_sel_reg_conf_mthd, pData, sizeof(USHORT));
			wsc_sel_reg_conf_mthd = be2cpu16(wsc_sel_reg_conf_mthd);
			break;

		case WSC_ID_VENDOR_EXT:
#ifdef WSC_V2_SUPPORT
			if (pWscCtrl->WscV2Info.bEnableWpsV2 && (WscLen > 0)) {
				/*
				*	Find WFA_EXT_ID_AUTHORIZEDMACS
				*/
				if (!pAuthorizedMACs)
					os_alloc_mem(NULL, &pAuthorizedMACs, WscLen);

				if (!pAuthorizedMACs) {
					MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE,
							 ("allocate pAuthorizedMACs memory fail!!\n"));
					return;
				}

				NdisZeroMemory(pAuthorizedMACs, WscLen);
				WscParseV2SubItem(WFA_EXT_ID_AUTHORIZEDMACS, pData, WscLen, pAuthorizedMACs, &AuthorizedMACsLen);
			}

#endif /* WSC_V2_SUPPORT */
			break;

		default:
			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("WscSelectedRegistrar --> Unknown IE 0x%04x\n", WscType));
			break;
		}

		/* Offset to net WSC Ie */
		pData  += WscLen;
		Length -= WscLen;
	}

	IsAPConfigured = pWscCtrl->WscConfStatus;

	if (wsc_sel_reg == 0x01) {
		pWscCtrl->WscSelReg = 1;
		WscBuildBeaconIE(pAd,
			WSC_SCSTATE_CONFIGURED,
			TRUE,
			wsc_dev_pass_id,
			wsc_sel_reg_conf_mthd,
			apidx,
			pAuthorizedMACs,
			AuthorizedMACsLen,
			AP_MODE);
		WscBuildProbeRespIE(pAd,
			WSC_MSGTYPE_AP_WLAN_MGR,
			WSC_SCSTATE_CONFIGURED,
			TRUE,
			wsc_dev_pass_id,
			wsc_sel_reg_conf_mthd,
			pWscCtrl->EntryIfIdx,
			pAuthorizedMACs,
			AuthorizedMACsLen,
			AP_MODE);
#ifdef WSC_V2_SUPPORT
		hex_dump("WscSelectedRegistrar - AuthorizedMACs::",
			pAuthorizedMACs, AuthorizedMACsLen);

		if ((AuthorizedMACsLen == 6) &&
			(NdisEqualMemory(pAuthorizedMACs, BROADCAST_ADDR, MAC_ADDR_LEN) == FALSE) &&
			(NdisEqualMemory(pAuthorizedMACs, ZERO_MAC_ADDR, MAC_ADDR_LEN) == FALSE) &&
			(pWscCtrl->WscState <= WSC_STATE_WAIT_M3)) {
			PWSC_PEER_ENTRY	pWscPeer = NULL;

			NdisMoveMemory(pWscCtrl->EntryAddr, pAuthorizedMACs, MAC_ADDR_LEN);
			RTMP_SEM_LOCK(&pWscCtrl->WscPeerListSemLock);
			WscClearPeerList(&pWscCtrl->WscPeerList);
			os_alloc_mem(pAd, (UCHAR **)&pWscPeer, sizeof(WSC_PEER_ENTRY));

			if (pWscPeer) {
				NdisZeroMemory(pWscPeer, sizeof(WSC_PEER_ENTRY));
				NdisMoveMemory(pWscPeer->mac_addr, pAuthorizedMACs, MAC_ADDR_LEN);
				NdisGetSystemUpTime(&pWscPeer->receive_time);
				insertTailList(&pWscCtrl->WscPeerList,
							   (RT_LIST_ENTRY *)pWscPeer);
				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE,
					("WscSelectedRegistrar --> Add this MAC to WscPeerList\n"));
			}

			ASSERT(pWscPeer != NULL);
			RTMP_SEM_UNLOCK(&pWscCtrl->WscPeerListSemLock);
		}

#endif /* WSC_V2_SUPPORT */
	} else {
		pWscCtrl->WscSelReg = 0;
		WscBuildBeaconIE(pAd,
			WSC_SCSTATE_CONFIGURED,
			FALSE, 0, 0, apidx, NULL, 0, AP_MODE);
		WscBuildProbeRespIE(pAd,
			WSC_MSGTYPE_AP_WLAN_MGR,
			WSC_SCSTATE_CONFIGURED,
			FALSE, 0, 0, pWscCtrl->EntryIfIdx, NULL, 0, AP_MODE);
	}

	UpdateBeaconHandler(
		pAd,
		wdev,
		BCN_UPDATE_IE_CHG);
#ifdef WSC_V2_SUPPORT
	if (pAuthorizedMACs)
		os_free_mem(pAuthorizedMACs);
#endif /* WSC_V2_SUPPORT */
}
#endif /* CONFIG_AP_SUPPORT */

#if defined(CONFIG_STA_SUPPORT) || defined(APCLI_SUPPORT)
/*
*	========================================================================
*
*	Routine Description:
*		Make WSC IE for the ProbeReq frame
*
*	Arguments:
*		pAd    - NIC Adapter pointer
*		pOutBuf		- all of WSC IE field
*		pIeLen		- length
*
*	Return Value:
*		None
*
*	IRQL = DISPATCH_LEVEL
*
*	Note:
*		None
*
*	========================================================================
*/
VOID WscBuildProbeReqIE(
	IN  RTMP_ADAPTER *pAd,
	IN  VOID *wdev_obj,
	OUT PUCHAR pOutBuf,
	OUT PUCHAR pIeLen)
{
	WSC_IE_HEADER ieHdr;
	UCHAR *OutMsgBuf = NULL; /* buffer to create message contents */
	INT Len = 0, templen = 0;
	PUCHAR pData;
	USHORT tempVal = 0;
	struct wifi_dev *wdev;
	PWSC_REG_DATA	pReg;
	WSC_CTRL *wsc_ctrl;

	wdev = (struct wifi_dev *)wdev_obj;
	wsc_ctrl = &wdev->WscControl;
	pReg = &wsc_ctrl->RegData;
	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, ("-----> WscBuildProbeReqIE\n"));
	/* allocate memory */
	os_alloc_mem(NULL, (UCHAR **)&OutMsgBuf, 512);

	if (OutMsgBuf == NULL) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, ("%s: Allocate memory fail!!!\n", __func__));
		return;
	}

	/* WSC IE Header */
	ieHdr.elemId = 221;
	ieHdr.length = 4;
	ieHdr.oui[0] = 0x00;
	ieHdr.oui[1] = 0x50;
	ieHdr.oui[2] = 0xF2;
		ieHdr.oui[3] = 0x04;

	pData = (PUCHAR) &OutMsgBuf[0];
	Len = 0;
	/* 1. Version */
	templen = AppendWSCTLV(WSC_ID_VERSION, pData, &pReg->SelfInfo.Version, 0);
	pData += templen;
	Len   += templen;

	/* 2. Request Type */
	if (wsc_ctrl->WscConfMode == WSC_REGISTRAR)
		tempVal = WSC_MSGTYPE_REGISTRAR;
	else if (wsc_ctrl->WscConfMode == WSC_ENROLLEE)
		tempVal = WSC_MSGTYPE_ENROLLEE_OPEN_8021X;
	else
		tempVal = WSC_MSGTYPE_ENROLLEE_INFO_ONLY;

	templen = AppendWSCTLV(WSC_ID_REQ_TYPE, pData, (UINT8 *)&tempVal, 0);
	pData += templen;
	Len   += templen;
	/* 3. Config method */
#ifdef WSC_V2_SUPPORT

	if (wsc_ctrl->WscV2Info.bEnableWpsV2)
		tempVal = wsc_ctrl->WscConfigMethods;
	else
#endif /* WSC_V2_SUPPORT */
	{
		tempVal = (wsc_ctrl->WscConfigMethods & 0x00FF);
	}

	tempVal = cpu2be16(tempVal);
	templen = AppendWSCTLV(WSC_ID_CONFIG_METHODS, pData, (UINT8 *)&tempVal, 0);
	pData += templen;
	Len   += templen;
	/* 4. UUID */
	templen = AppendWSCTLV(WSC_ID_UUID_E, pData, pReg->SelfInfo.Uuid, 0);
	pData += templen;
	Len   += templen;
	/* 5. Primary device type */
	templen = AppendWSCTLV(WSC_ID_PRIM_DEV_TYPE, pData, pReg->SelfInfo.PriDeviceType, 0);
	pData += templen;
	Len   += templen;
	/* 6. RF band, shall change based on current channel */
	templen = AppendWSCTLV(WSC_ID_RF_BAND, pData, &pReg->SelfInfo.RfBand, 0);
	pData += templen;
	Len   += templen;
	/* 7. Associate state */
	tempVal = pReg->SelfInfo.AssocState;
	templen = AppendWSCTLV(WSC_ID_ASSOC_STATE, pData, (UINT8 *)&tempVal, 0);
	pData += templen;
	Len   += templen;
	/* 8. Config error */
	tempVal = pReg->SelfInfo.ConfigError;
	templen = AppendWSCTLV(WSC_ID_CONFIG_ERROR, pData, (UINT8 *)&tempVal, 0);
	pData += templen;
	Len   += templen;
	/* 9. Device password ID */
	tempVal = pReg->SelfInfo.DevPwdId;
	templen = AppendWSCTLV(WSC_ID_DEVICE_PWD_ID, pData, (UINT8 *)&tempVal, 0);
	pData += templen;
	Len   += templen;
#ifdef WSC_V2_SUPPORT

	if (wsc_ctrl->WscV2Info.bEnableWpsV2) {
		/* 10. Manufacturer */
		NdisZeroMemory(pData, 64 + 4);
		templen = AppendWSCTLV(WSC_ID_MANUFACTURER, pData,  pReg->SelfInfo.Manufacturer, strlen((RTMP_STRING *) pReg->SelfInfo.Manufacturer));
		pData += templen;
		Len   += templen;
		/* 11. Model Name */
		NdisZeroMemory(pData, 32 + 4);
		templen = AppendWSCTLV(WSC_ID_MODEL_NAME, pData, pReg->SelfInfo.ModelName, strlen((RTMP_STRING *) pReg->SelfInfo.ModelName));
		pData += templen;
		Len   += templen;
		/* 12. Model Number */
		NdisZeroMemory(pData, 32 + 4);
		templen = AppendWSCTLV(WSC_ID_MODEL_NUMBER, pData, pReg->SelfInfo.ModelNumber, strlen((RTMP_STRING *) pReg->SelfInfo.ModelNumber));
		pData += templen;
		Len   += templen;
		/* 13. Device Name */
		NdisZeroMemory(pData, 32 + 4);
		templen = AppendWSCTLV(WSC_ID_DEVICE_NAME, pData, pReg->SelfInfo.DeviceName, strlen((RTMP_STRING *) pReg->SelfInfo.DeviceName));
		pData += templen;
		Len   += templen;
		/* Version2 */
		WscGenV2Msg(wsc_ctrl,
					FALSE,
					NULL,
					0,
					&pData,
					&Len);
	}

#endif /* WSC_V2_SUPPORT */
	ieHdr.length = ieHdr.length + Len;
	RTMPMoveMemory(pOutBuf, &ieHdr, sizeof(WSC_IE_HEADER));
	RTMPMoveMemory(pOutBuf + sizeof(WSC_IE_HEADER), OutMsgBuf, Len);
	*pIeLen = sizeof(WSC_IE_HEADER) + Len;

	if (OutMsgBuf != NULL)
		os_free_mem(OutMsgBuf);

	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, ("<----- WscBuildProbeReqIE\n"));
}


/*
*	========================================================================
*
*	Routine Description:
*		Make WSC IE for the AssocReq frame
*
*	Arguments:
*		pAd    - NIC Adapter pointer
*		pOutBuf		- all of WSC IE field
*		pIeLen		- length
*
*	Return Value:
*		None
*
*	IRQL = DISPATCH_LEVEL
*
*	Note:
*		None
*
*	========================================================================
*/
VOID WscBuildAssocReqIE(
	IN  PWSC_CTRL pWscControl,
	OUT PUCHAR pOutBuf,
	OUT PUCHAR pIeLen)
{
	WSC_IE_HEADER ieHdr;
	UCHAR *Data = NULL;
	PUCHAR pData;
	INT Len = 0, templen = 0;
	UINT8 tempVal = 0;
	PWSC_REG_DATA pReg = (PWSC_REG_DATA) &pWscControl->RegData;

	if (pWscControl == NULL) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, ("WscBuildAssocReqIE: pWscControl is NULL\n"));
		return;
	}

	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("-----> WscBuildAssocReqIE\n"));
	/* allocate memory */
	os_alloc_mem(NULL, (UCHAR **)&Data, 512);

	if (Data == NULL) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, ("%s: Allocate memory fail!!!\n", __func__));
		return;
	}

	/* WSC IE Header */
	ieHdr.elemId = 221;
	ieHdr.length = 4;
	ieHdr.oui[0] = 0x00;
	ieHdr.oui[1] = 0x50;
	ieHdr.oui[2] = 0xF2;
	ieHdr.oui[3] = 0x04;
	pData = (PUCHAR) &Data[0];
	Len = 0;
	/* Version */
	templen = AppendWSCTLV(WSC_ID_VERSION, pData, &pReg->SelfInfo.Version, 0);
	pData += templen;
	Len   += templen;

	/* Request Type */
	if (pWscControl->WscConfMode == WSC_ENROLLEE)
		tempVal = WSC_MSGTYPE_ENROLLEE_INFO_ONLY;
	else
		tempVal = WSC_MSGTYPE_REGISTRAR;

	templen = AppendWSCTLV(WSC_ID_REQ_TYPE, pData, (UINT8 *)&tempVal, 0);
	pData += templen;
	Len   += templen;
#ifdef WSC_V2_SUPPORT

	if (pWscControl->WscV2Info.bEnableWpsV2) {
		/* Version2 */
		WscGenV2Msg(pWscControl,
					FALSE,
					NULL,
					0,
					&pData,
					&Len);
	}

#endif /* WSC_V2_SUPPORT */
	ieHdr.length = ieHdr.length + Len;
	RTMPMoveMemory(pOutBuf, &ieHdr, sizeof(WSC_IE_HEADER));
	RTMPMoveMemory(pOutBuf + sizeof(WSC_IE_HEADER), Data, Len);
	*pIeLen = sizeof(WSC_IE_HEADER) + Len;

	if (Data != NULL)
		os_free_mem(Data);

	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("<----- WscBuildAssocReqIE\n"));
}
#endif /* defined(CONFIG_STA_SUPPORT) || defined (APCLI_SUPPORT) */



VOID WscProfileRetryTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	RTMP_ADAPTER *pAdapter = NULL;
	PWSC_CTRL pWscControl = (PWSC_CTRL)FunctionContext;
	BOOLEAN bReConnect = TRUE;
	UCHAR CurOpMode = 0xFF;
	UCHAR if_idx = 0;

	if (pWscControl == NULL)
		return;

	pAdapter = pWscControl->pAd;

	if (pAdapter != NULL) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("WscProfileRetryTimeout:: WSC profile retry timeout index: %d\n", pWscControl->WscProfile.ApplyProfileIdx));
		if_idx = (pWscControl->EntryIfIdx & 0x0F);
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAdapter)
		CurOpMode = AP_MODE;
#ifdef APCLI_SUPPORT

		if ((CurOpMode == AP_MODE)
			 && (pAdapter->ApCfg.ApCliTab[if_idx].CtrlCurrState == APCLI_CTRL_CONNECTED)
			 && (pAdapter->ApCfg.ApCliTab[if_idx].SsidLen != 0)) {
			INT i;

			for (i = 0; VALID_UCAST_ENTRY_WCID(pAdapter, i); i++) {
				PMAC_TABLE_ENTRY pEntry = &pAdapter->MacTab.Content[i];
				STA_TR_ENTRY *tr_entry = &pAdapter->MacTab.tr_entry[i];

				if (IS_ENTRY_APCLI(pEntry) &&
					 (pEntry->Sst == SST_ASSOC) &&
					 (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED))
					bReConnect = FALSE;
			}
		}

#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
		if (bReConnect) {
			if (pWscControl->WscProfile.ApplyProfileIdx < pWscControl->WscProfile.ProfileCnt - 1)
				pWscControl->WscProfile.ApplyProfileIdx++;
			else
				pWscControl->WscProfile.ApplyProfileIdx = 0;

#ifdef APCLI_SUPPORT

			if (CurOpMode == AP_MODE) {
				WscWriteConfToApCliCfg(pAdapter,
									   pWscControl,
									   &pWscControl->WscProfile.Profile[pWscControl->WscProfile.ApplyProfileIdx],
									   TRUE);
				{
					RTEnqueueInternalCmd(pAdapter, CMDTHREAD_APCLI_IF_DOWN, (VOID *)&if_idx, sizeof(UCHAR));
				}
			}

#endif /* APCLI_SUPPORT */
			pAdapter->WriteWscCfgToDatFile = (pWscControl->EntryIfIdx & 0x0F);
			/*#ifdef KTHREAD_SUPPORT */
			/*			WAKE_UP(&(pAdapter->wscTask)); */
			/*#else */
			/*			RTMP_SEM_EVENT_UP(&(pAdapter->wscTask.taskSema)); */
			/*#endif */
			RtmpOsTaskWakeUp(&(pAdapter->wscTask));
			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("WscProfileRetryTimeout:: WSC profile retry index: %d\n", pWscControl->WscProfile.ApplyProfileIdx));
		}

	}
}

VOID WscPBCTimeOutAction(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	PWSC_CTRL pWscControl = (PWSC_CTRL)FunctionContext;
	RTMP_ADAPTER *pAd = NULL;
	BOOLEAN Cancelled;

	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_OFF, ("-----> WscPBCTimeOutAction\n"));

	if (pWscControl != NULL)
		pAd = pWscControl->pAd;

#ifdef CON_WPS
#ifdef MULTI_INF_SUPPORT
	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_OFF, ("[IfaceIdx = %d] WscPBCTimeOutAction !!!\n", multi_inf_get_idx(pAd)));
#endif /* MULTI_INF_SUPPORT */
#endif  /*CON_WPS*/

	if (pAd != NULL) {
		if (pWscControl->WscPBCTimerRunning) {
			pWscControl->WscPBCTimerRunning = FALSE;
			RTMPCancelTimer(&pWscControl->WscPBCTimer, &Cancelled);
		}

		WscPBCExec(pAd, FALSE, pWscControl);
		/* call Mlme handler to execute it */
		RTMP_MLME_HANDLER(pAd);
	}

	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_OFF, ("<----- WscPBCTimeOutAction\n"));
}

#ifdef WSC_STA_SUPPORT
VOID WscPINTimeOutAction(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	PWSC_CTRL pWscControl = (PWSC_CTRL)FunctionContext;
	RTMP_ADAPTER *pAd = NULL;
	BOOLEAN Cancelled;

	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_OFF, ("-----> WscPINTimeOutAction\n"));

	if (pWscControl != NULL)
		pAd = pWscControl->pAd;

	if (pAd != NULL) {
		if (pWscControl->WscPINTimerRunning) {
			pWscControl->WscPINTimerRunning = FALSE;
			RTMPCancelTimer(&pWscControl->WscPINTimer, &Cancelled);
		}

		WscPINExec(pAd, FALSE, pWscControl);
		/* call Mlme handler to execute it */
		RTMP_MLME_HANDLER(pAd);
	}

	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_OFF, ("<----- WscPINTimeOutAction\n"));
}
#endif

/*
*	========================================================================
*
*	Routine Description:
*		Exec scan after scan timer expiration
*
*	Arguments:
*		FunctionContext		NIC Adapter pointer
*
*	Return Value:
*		None
*
*	IRQL = DISPATCH_LEVEL
*
*	Note:
*
*	========================================================================
*/
VOID WscScanTimeOutAction(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	RTMP_ADAPTER *pAd = NULL;
	PWSC_CTRL pWscControl = (PWSC_CTRL)FunctionContext;

	if (pWscControl == NULL)
		return;

	pAd = pWscControl->pAd;

	if (pAd != NULL) {
		/* call to execute the scan actions */
		if (TEST_FLAG_CONN_IN_PROG(pAd->ConInPrgress)) {
			/* Reconfigure the scan after 1 sec as currently connection is going */
			RTMPSetTimer(&pWscControl->WscScanTimer, 1000);
			pWscControl->WscScanTimerRunning = TRUE;
			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR,
				("Connection in progress so skipping scan else it can move AP to offchannel\n"));
			return;
		}
		WscScanExec(pAd, pWscControl);

		/* register 10 second timer for PBC or PIN connection execution */
		if (pWscControl->WscMode == WSC_PBC_MODE) {
			/* Prevent infinite loop if conncet time out didn't stop the repeat scan */
			if (pWscControl->WscState != WSC_STATE_OFF) {
#ifdef CON_WPS /*Move to scan complete to trigger //Dung_Ru*/

				if (pWscControl->conWscStatus == CON_WPS_STATUS_DISABLED)
#endif /* CON_WPS */
				{
#ifdef APCLI_SUPPORT
					if (pWscControl->WscApCliScanMode != TRIGGER_PARTIAL_SCAN)
#endif /* APCLI_SUPPORT */
					{
#if defined(CONFIG_MAP_SUPPORT)
						if (!IS_MAP_TURNKEY_ENABLE(pAd))
#endif
						{
							RTMPSetTimer(&pWscControl->WscPBCTimer, 10000);
							pWscControl->WscPBCTimerRunning = TRUE;
						}
					}
				}
			}
		} else if (pWscControl->WscMode == WSC_PIN_MODE) {
			/* Prevent infinite loop if conncet time out didn't stop the repeat scan */
#ifdef WSC_STA_SUPPORT
			if (pWscControl->WscState != WSC_STATE_OFF) {
				RTMPSetTimer(&pWscControl->WscPINTimer, 10000);
				pWscControl->WscPINTimerRunning = TRUE;
			}

#endif
		}

		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_OFF, ("!!! WscScanTimeOutAction !!!\n"));
		/* call Mlme handler to execute it */
		RTMP_MLME_HANDLER(pAd);
	}
}


#ifdef CON_WPS
VOID WscScanDoneCheckTimeOutAction(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	PWSC_CTRL pWscControl = (PWSC_CTRL)FunctionContext;
	RTMP_ADAPTER *pAd = NULL;
	BOOLEAN Cancelled;
	UCHAR if_idx = 0;
	struct wifi_dev *wdev;

	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_OFF, ("-----> WscScanDoneCheckTimeOutAction\n"));

	if (pWscControl != NULL) {
		pAd = pWscControl->pAd;
		if_idx = (pWscControl->EntryIfIdx & 0x0F);
	}

#ifdef MULTI_INF_SUPPORT
	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_OFF, ("[IfaceIdx = %d] WscScanDoneCheckTimeOutAction !!!\n", multi_inf_get_idx(pAd)));
#endif /* MULTI_INF_SUPPORT */

	if (pAd != NULL) {
		pWscControl->ConWscApcliScanDoneCheckTimerRunning = FALSE;
		RTMPCancelTimer(&pWscControl->ConWscApcliScanDoneCheckTimer, &Cancelled);

		/* call Mlme handler to execute it */
		if (if_idx < MAX_APCLI_NUM) {
			wdev = &(pAd->ApCfg.ApCliTab[if_idx].wdev);
			MlmeEnqueue(pAd, AP_SYNC_STATE_MACHINE, APMT2_MLME_SCAN_COMPLETE, 0, NULL, if_idx);
			RTMP_MLME_HANDLER(pAd);
		} else {
			APMlmeScanCompleteAction(pAd, NULL);
			RTMP_MLME_HANDLER(pAd);
		}
	}

	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_OFF, ("<----- WscScanDoneCheckTimeOutAction\n"));
}
#endif /*CON_WPS*/

BOOLEAN ValidateChecksum(
	IN UINT PIN)
{
	UINT accum = 0;

	accum += 3 * ((PIN / 10000000) % 10);
	accum += 1 * ((PIN / 1000000) % 10);
	accum += 3 * ((PIN / 100000) % 10);
	accum += 1 * ((PIN / 10000) % 10);
	accum += 3 * ((PIN / 1000) % 10);
	accum += 1 * ((PIN / 100) % 10);
	accum += 3 * ((PIN / 10) % 10);
	accum += 1 * ((PIN / 1) % 10);
	return (0 == (accum % 10));
} /* ValidateChecksum */

/*
*	Generate 4-digit random number, ex:1234
*/
UINT WscRandomGen4digitPinCode(
	IN  PRTMP_ADAPTER   pAd)
{
	UINT iPin;

	iPin = RandomByte2(pAd) * 256 * 256 + RandomByte2(pAd) * 256 + RandomByte2(pAd);
	iPin = iPin % 10000;
	return iPin;
}

UINT WscRandomGeneratePinCode(
	IN  PRTMP_ADAPTER pAd,
	IN  UCHAR apidx)
{
	UINT iPin;
	UINT checksum;

	iPin = RandomByte(pAd) * 256 * 256 + RandomByte(pAd) * 256 + RandomByte(pAd);
	iPin = iPin % 10000000;
	checksum = ComputeChecksum(iPin);
	iPin = iPin * 10 + checksum;
	return iPin;
}


#ifdef CONFIG_AP_SUPPORT
VOID  WscInformFromWPA(
	IN  PMAC_TABLE_ENTRY pEntry)
{
	/* WPA_STATE_MACHINE informs this Entry is already WPA_802_1X_PORT_SECURED. */
	RTMP_ADAPTER *pAd = (PRTMP_ADAPTER)pEntry->pAd;
	BOOLEAN Cancelled;
	struct wifi_dev *wdev;

	if (pEntry->func_tb_idx >= pAd->ApCfg.BssidNum)
		return;

	wdev = pEntry->wdev;
	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("-----> WscInformFromWPA\n"));

	if (MAC_ADDR_EQUAL(pEntry->Addr, wdev->WscControl.EntryAddr)) {
		NdisZeroMemory(wdev->WscControl.EntryAddr, MAC_ADDR_LEN);
		RTMPCancelTimer(&wdev->WscControl.EapolTimer, &Cancelled);
		wdev->WscControl.EapolTimerRunning = FALSE;
		pEntry->bWscCapable = FALSE;
		wdev->WscControl.WscState = WSC_STATE_CONFIGURED;
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("Reset EntryIfIdx to %d\n", WSC_INIT_ENTRY_APIDX));
	}

	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("<----- WscInformFromWPA\n"));
}
#endif /* CONFIG_AP_SUPPORT */

VOID WscStop(
	IN  PRTMP_ADAPTER pAd,
#ifdef CONFIG_AP_SUPPORT
	IN  BOOLEAN bFromApCli,
#endif /* CONFIG_AP_SUPPORT */
	IN  PWSC_CTRL pWscControl)
{
	PWSC_UPNP_NODE_INFO pWscUPnPInfo;
	BOOLEAN Cancelled;
#ifdef WSC_LED_SUPPORT
	UCHAR WPSLEDStatus;
#endif /* WSC_LED_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
	MAC_TABLE_ENTRY  *pEntry;
	/* UCHAR apidx = (pWscControl->EntryIfIdx & 0x0F); */
#endif /* CONFIG_AP_SUPPORT */
	UCHAR	CurOpMode = 0xff;
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	CurOpMode = AP_MODE;
#endif /* CONFIG_AP_SUPPORT */
	pWscUPnPInfo = &pWscControl->WscUPnPNodeInfo;

	if (pWscUPnPInfo->bUPnPMsgTimerRunning == TRUE) {
		pWscUPnPInfo->bUPnPMsgTimerRunning = FALSE;
		RTMPCancelTimer(&pWscUPnPInfo->UPnPMsgTimer, &Cancelled);
		pWscUPnPInfo->bUPnPMsgTimerPending = FALSE;
	}

	if (pWscControl->bM2DTimerRunning) {
		pWscControl->bM2DTimerRunning = FALSE;
		RTMPCancelTimer(&pWscControl->M2DTimer, &Cancelled);
	}

	pWscUPnPInfo->bUPnPInProgress = FALSE;
	pWscControl->M2DACKBalance = 0;
	pWscUPnPInfo->registrarID = 0;

	if (pWscControl->Wsc2MinsTimerRunning) {
		pWscControl->Wsc2MinsTimerRunning = FALSE;
		RTMPCancelTimer(&pWscControl->Wsc2MinsTimer, &Cancelled);
	}

	if (pWscControl->WscUpdatePortCfgTimerRunning) {
		pWscControl->WscUpdatePortCfgTimerRunning = FALSE;
		RTMPCancelTimer(&pWscControl->WscUpdatePortCfgTimer, &Cancelled);
	}
	if (pWscControl->EapolTimerRunning) {
		pWscControl->EapolTimerRunning = FALSE;
		RTMPCancelTimer(&pWscControl->EapolTimer, &Cancelled);
	}
#ifdef CONFIG_AP_SUPPORT

	if ((pWscControl->EntryIfIdx & 0x0F) < pAd->ApCfg.BssidNum) {
		pEntry = MacTableLookup2(pAd, pWscControl->EntryAddr, (struct wifi_dev *)pWscControl->wdev);

		if (CurOpMode == AP_MODE) {
			if (pEntry && !bFromApCli)
				pEntry->bWscCapable = FALSE;
		}
	}

#endif /* CONFIG_AP_SUPPORT */
	NdisZeroMemory(pWscControl->EntryAddr, MAC_ADDR_LEN);
	pWscControl->WscSelReg = 0;

	if ((pWscControl->WscStatus == STATUS_WSC_CONFIGURED) ||
		 (pWscControl->WscStatus == STATUS_WSC_FAIL) ||
		 (pWscControl->WscStatus == STATUS_WSC_PBC_TOO_MANY_AP))
		;
	else
		pWscControl->WscStatus = STATUS_WSC_NOTUSED;

	pWscControl->WscState = WSC_STATE_OFF;
	pWscControl->lastId = 1;
	pWscControl->EapMsgRunning = FALSE;
	pWscControl->EapolTimerPending = FALSE;
	pWscControl->bWscTrigger = FALSE;

/* WPS_BandSteering Support */
#ifdef CONFIG_AP_SUPPORT
#ifdef BAND_STEERING
	/* WPS: clear WPS WHITELIST in case of AP Wsc Stop */
	if (!bFromApCli && (pAd->ApCfg.BandSteering)) {

		struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[pWscControl->EntryIfIdx & 0x0F].wdev;
		PBND_STRG_CLI_TABLE table = Get_BndStrgTable(pAd, wdev->func_idx);

		if (table && table->bEnabled) {
			NdisAcquireSpinLock(&table->WpsWhiteListLock);
			ClearWpsWhiteList(&table->WpsWhiteList);
			NdisReleaseSpinLock(&table->WpsWhiteListLock);
			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("%s:channel %u wps whitelist cleared, size : %d\n",
			 __func__, table->Channel, table->WpsWhiteList.size));
		}
	}
#endif
#endif /* CONFIG_AP_SUPPORT */

	if (pWscControl->WscScanTimerRunning) {
		pWscControl->WscScanTimerRunning = FALSE;
		RTMPCancelTimer(&pWscControl->WscScanTimer, &Cancelled);
	}

	if (pWscControl->WscPBCTimerRunning) {
		pWscControl->WscPBCTimerRunning = FALSE;
		RTMPCancelTimer(&pWscControl->WscPBCTimer, &Cancelled);
	}

#ifdef CON_WPS

	if (bFromApCli && (pWscControl->EntryIfIdx & 0x0F) < MAX_APCLI_NUM) {
		pAd->ApCfg.ApCliTab[(pWscControl->EntryIfIdx & 0x0F)].ConWpsApCliModeScanDoneStatus = CON_WPS_APCLI_SCANDONE_STATUS_NOTRIGGER;

		if (pWscControl->ConWscApcliScanDoneCheckTimerRunning) {
			pWscControl->ConWscApcliScanDoneCheckTimerRunning = FALSE;
			RTMPCancelTimer(&pWscControl->ConWscApcliScanDoneCheckTimer, &Cancelled);
		}
	}  else {
		if (pWscControl->ConWscApcliScanDoneCheckTimerRunning) {
			pWscControl->ConWscApcliScanDoneCheckTimerRunning = FALSE;
			RTMPCancelTimer(&pWscControl->ConWscApcliScanDoneCheckTimer, &Cancelled);
		}
	}

#endif /*CON_WPS*/
#ifdef WSC_LED_SUPPORT

	if (pWscControl->WscLEDTimerRunning) {
		pWscControl->WscLEDTimerRunning = FALSE;
		RTMPCancelTimer(&pWscControl->WscLEDTimer, &Cancelled);
	}

	if (pWscControl->WscSkipTurnOffLEDTimerRunning) {
		pWscControl->WscSkipTurnOffLEDTimerRunning = FALSE;
		RTMPCancelTimer(&pWscControl->WscSkipTurnOffLEDTimer, &Cancelled);
	}

	/* Reset the WPS walk time. */
	pWscControl->bWPSWalkTimeExpiration = FALSE;
	WPSLEDStatus = LED_WPS_TURN_LED_OFF;
	RTMPSetLED(pAd, WPSLEDStatus);
#endif /* WSC_LED_SUPPORT */
}

#ifdef CON_WPS
VOID WscConWpsStop(
	IN  PRTMP_ADAPTER pAd,
	IN  BOOLEAN bFromApCli,
	IN  PWSC_CTRL pWscControl)
{
#ifdef MULTI_INF_SUPPORT
	/* Single Driver ctrl the WSC SM between the two pAd */
	PRTMP_ADAPTER pOpposAd;
	PWSC_CTRL pWpsCtrl = NULL;
	INT IsAPConfigured = 0;
	UCHAR pAdListInfo[2] = {0, 0};
	INT myBandIdx = 0, opsBandIdx = 0;
	INT nowBandIdx = multi_inf_get_idx(pAd);
	UCHAR loop;

	if (pWscControl->conWscStatus == CON_WPS_STATUS_DISABLED)
		return;

	/* Update the Global pAd List Band Info */
	pOpposAd = (PRTMP_ADAPTER)adapt_list[0];
	pAdListInfo[0] = 0;/* RFIC_IS_5G_BAND(pOpposAd); */
	pOpposAd = (PRTMP_ADAPTER)adapt_list[1];
	pAdListInfo[1] = 1;/* RFIC_IS_5G_BAND(pOpposAd); */
	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_OFF, ("%s pAdListInfo is5G---> [%d, %d]\n",
			 __func__, pAdListInfo[0], pAdListInfo[1]));

	/* which band from function in */
	if (nowBandIdx == pAdListInfo[0]) {
		myBandIdx = 0;
		opsBandIdx = 1;
	} else if (nowBandIdx == pAdListInfo[1]) {
		myBandIdx = 1;
		opsBandIdx = 0;
	} else {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, ("%s pAdListInfo is5G---> [%d, %d] Fail\n",
				 __func__, pAdListInfo[0], pAdListInfo[1]));
	}

	pOpposAd = (PRTMP_ADAPTER) adapt_list[opsBandIdx];

	if (bFromApCli) {
		if (pOpposAd) {
			for (loop = 0; loop < pOpposAd->ApCfg.ApCliNum; loop++) {
				pWpsCtrl = &pOpposAd->ApCfg.ApCliTab[loop].wdev.WscControl;
				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_OFF, ("CON_WPS: Stop the Band[%d] %s ApCli WPS, state [%d]\n",
						 opsBandIdx, pWpsCtrl->IfName, pWpsCtrl->WscState));

				if (pWpsCtrl->WscState != WSC_STATE_OFF) {
					WscStop(pOpposAd, TRUE, pWpsCtrl);
					pWpsCtrl->WscConfMode = WSC_DISABLE;
				}
			}
		}

		for (loop = 0; loop < pAd->ApCfg.ApCliNum; loop++) {
			if ((pWscControl->EntryIfIdx & 0x0F) == loop)
				continue;

			pWpsCtrl = &pAd->ApCfg.ApCliTab[loop].wdev.WscControl;

			if (pWpsCtrl->WscState != WSC_STATE_OFF) {
				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_OFF, ("CON_WPS: Stop the current pAd apcli [%d]\n",
						 loop));
				WscStop(pAd, TRUE, pWpsCtrl);
				pWpsCtrl->WscConfMode = WSC_DISABLE;
			}
		}
	} else {
		if (pOpposAd != NULL) {
			for (loop = 0; loop < HW_BEACON_MAX_NUM; loop++) {
				struct wifi_dev *wdev = &pOpposAd->ApCfg.MBSSID[loop].wdev;

				pWpsCtrl = &wdev->WscControl;
				IsAPConfigured = pWpsCtrl->WscConfStatus;

				if ((wdev) &&
					(pWpsCtrl->WscConfMode != WSC_DISABLE) &&
					(pWpsCtrl->bWscTrigger == TRUE)) {
					MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_OFF, ("CON_WPS[%d]: Stop the %s AP Wsc Machine\n", opsBandIdx, pOpposAd->net_dev->name));
					WscBuildBeaconIE(pOpposAd, IsAPConfigured, FALSE, 0, 0, loop, NULL, 0, AP_MODE);
					WscBuildProbeRespIE(pOpposAd, WSC_MSGTYPE_AP_WLAN_MGR, IsAPConfigured, FALSE, 0, 0,
										loop, NULL, 0, AP_MODE);
					UpdateBeaconHandler(pOpposAd, wdev, BCN_UPDATE_IE_CHG);
					WscStop(pOpposAd, FALSE, pWpsCtrl);
				}
			}
		} else {
			for (loop = 0; loop < HW_BEACON_MAX_NUM; loop++) {
				struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[loop].wdev;

				pWpsCtrl = &wdev->WscControl;
				IsAPConfigured = pWpsCtrl->WscConfStatus;

				if ((wdev) &&
					(pWpsCtrl->WscConfMode != WSC_DISABLE) &&
					(pWpsCtrl->bWscTrigger == TRUE)) {
					MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_OFF, ("CON_WPS: Stop the %s AP Wsc Machine\n", wdev->if_dev->name));
					WscBuildBeaconIE(pAd, IsAPConfigured, FALSE, 0, 0, loop, NULL, 0, AP_MODE);
					WscBuildProbeRespIE(pAd, WSC_MSGTYPE_AP_WLAN_MGR, IsAPConfigured, FALSE, 0, 0,
										loop, NULL, 0, AP_MODE);
					UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_IE_CHG);
					WscStop(pAd, FALSE, pWpsCtrl);
				}
			}
		}
	}

#endif /* MULTI_INF_SUPPORT */
	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_OFF, ("<----- WscConWpsStop\n"));
}
#endif /* CON_WPS */


VOID WscInit(
	IN  PRTMP_ADAPTER pAd,
	IN  BOOLEAN bFromApCli,
	IN  UCHAR BssIndex)
{
	PWSC_CTRL pWscControl = NULL;
	UCHAR CurOpMode = AP_MODE;
	struct wifi_dev *wdev = NULL;
#ifdef CONFIG_AP_SUPPORT
	INT IsAPConfigured;

	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
#ifdef APCLI_SUPPORT

		if (bFromApCli) {
			if ((BssIndex & 0x0F) < MAX_APCLI_NUM)
				wdev = &pAd->ApCfg.ApCliTab[BssIndex & 0x0F].wdev;
		} else
#endif /* APCLI_SUPPORT */
			wdev = &pAd->ApCfg.MBSSID[BssIndex & 0x0F].wdev;
	}
#endif /* CONFIG_AP_SUPPORT */

	if (wdev == NULL)
		return;

	pWscControl = &wdev->WscControl;

	if (pWscControl->WscEnrolleePinCode == 0) {
		if (pWscControl->WscEnrollee4digitPinCode) {
			pWscControl->WscEnrolleePinCodeLen = 4;
			pWscControl->WscEnrolleePinCode = WscRandomGen4digitPinCode(pAd);
		} else {
			pWscControl->WscEnrolleePinCode = GenerateWpsPinCode(pAd, bFromApCli, BssIndex);
			pWscControl->WscEnrolleePinCodeLen = 8;
		}
	}

	pWscControl->RegData.SelfInfo.Version = WSC_VERSION;
#ifdef WSC_V2_SUPPORT
	pWscControl->RegData.SelfInfo.Version2 = WSC_V2_VERSION;
#endif /* WSC_V2_SUPPORT */
	pWscControl->bWscLastOne = FALSE;
	pWscControl->bWscFirstOne = FALSE;
	pWscControl->WscStatus = STATUS_WSC_IDLE;
#ifdef CONFIG_AP_SUPPORT

	if (((CurOpMode == AP_MODE) &&
		 (pWscControl->WscConfMode == WSC_DISABLE))
#ifdef WSC_V2_SUPPORT
		|| ((pWscControl->WscV2Info.bWpsEnable == FALSE) && pWscControl->WscV2Info.bEnableWpsV2)
#endif /* WSC_V2_SUPPORT */
	   ) {
		if (CurOpMode == AP_MODE) {
#ifdef APCLI_SUPPORT

			if (!bFromApCli)
#endif /* APCLI_SUPPORT */
			{
				wdev->WscIEBeacon.ValueLen = 0;
				wdev->WscIEProbeResp.ValueLen = 0;
			}

#ifdef APCLI_SUPPORT
			else
				WscInitRegistrarPair(pAd, pWscControl, BssIndex & 0x0F);

#endif /* APCLI_SUPPORT */
		}
	} else
#endif /* CONFIG_AP_SUPPORT */
	{
		WscInitRegistrarPair(pAd, pWscControl, BssIndex & 0x0F);
#ifdef CONFIG_AP_SUPPORT

		if (CurOpMode == AP_MODE) {
#ifdef APCLI_SUPPORT

			if (!bFromApCli)
#endif /* APCLI_SUPPORT */
			{
				UCHAR apidx = pWscControl->EntryIfIdx & 0x0F;
				struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[apidx].wdev;

				IsAPConfigured = pWscControl->WscConfStatus;
				WscBuildBeaconIE(pAd, IsAPConfigured, FALSE, 0, 0, (BssIndex & 0x0F), NULL, 0, AP_MODE);
				WscBuildProbeRespIE(pAd, WSC_MSGTYPE_AP_WLAN_MGR, IsAPConfigured, FALSE, 0, 0, BssIndex, NULL, 0, AP_MODE);
				UpdateBeaconHandler(
					pAd,
					wdev,
					BCN_UPDATE_IE_CHG);
			}
		}

#endif /* CONFIG_AP_SUPPORT */
	}

}

USHORT WscGetAuthType(
	IN UINT32 authType)
{
	if (IS_AKM_OPEN(authType))
		return WSC_AUTHTYPE_OPEN;
	else if (IS_AKM_SHARED(authType))
		return WSC_AUTHTYPE_SHARED;
	else if (IS_AKM_WPANONE(authType))
		return WSC_AUTHTYPE_WPANONE;
	else if (IS_AKM_WPA1(authType) && IS_AKM_WPA2(authType))
		return WSC_AUTHTYPE_WPA | WSC_AUTHTYPE_WPA2;
	else if (IS_AKM_WPA1PSK(authType) && IS_AKM_WPA2PSK(authType))
		return WSC_AUTHTYPE_WPAPSK | WSC_AUTHTYPE_WPA2PSK;
	else if (IS_AKM_WPA1(authType))
		return WSC_AUTHTYPE_WPA;
	else if (IS_AKM_WPA1PSK(authType))
		return WSC_AUTHTYPE_WPAPSK;
	else if (IS_AKM_WPA2(authType))
		return WSC_AUTHTYPE_WPA2;
	else if (IS_AKM_WPA2PSK(authType))
		return WSC_AUTHTYPE_WPA2PSK;
	else
		return WSC_AUTHTYPE_OPEN;
}

USHORT WscGetEncryType(
	IN UINT32 encryType)
{
	if (IS_CIPHER_NONE(encryType))
		return WSC_ENCRTYPE_NONE;
	else if (IS_CIPHER_WEP(encryType))
		return WSC_ENCRTYPE_WEP;
	else if (IS_CIPHER_TKIP(encryType) && IS_CIPHER_CCMP128(encryType))
		return WSC_ENCRTYPE_AES | WSC_ENCRTYPE_TKIP;
	else if (IS_CIPHER_TKIP(encryType))
		return WSC_ENCRTYPE_TKIP;
	else if (IS_CIPHER_CCMP128(encryType))
		return WSC_ENCRTYPE_AES;
	else
		return WSC_ENCRTYPE_AES;
}

RTMP_STRING *WscGetAuthTypeStr(
	IN  USHORT authFlag)
{
	switch (authFlag) {
	case WSC_AUTHTYPE_OPEN:
		return "OPEN";

	case WSC_AUTHTYPE_WPAPSK:
		return "WPAPSK";

	case WSC_AUTHTYPE_SHARED:
		return "SHARED";

	case WSC_AUTHTYPE_WPANONE:
		return "WPANONE";

	case WSC_AUTHTYPE_WPA:
		return "WPA";

	case WSC_AUTHTYPE_WPA2:
		return "WPA2";

	default:
	case (WSC_AUTHTYPE_WPAPSK | WSC_AUTHTYPE_WPA2PSK):
		return "WPAPSKWPA2PSK";

	case WSC_AUTHTYPE_WPA2PSK:
		return "WPA2PSK";

	case (WSC_AUTHTYPE_OPEN | WSC_AUTHTYPE_SHARED):
		return "WEPAUTO";
	}
}

RTMP_STRING *WscGetEncryTypeStr(
	IN  USHORT encryFlag)
{
	switch (encryFlag) {
	case WSC_ENCRTYPE_NONE:
		return "NONE";

	case WSC_ENCRTYPE_WEP:
		return "WEP";

	case WSC_ENCRTYPE_TKIP:
		return "TKIP";

	default:
	case (WSC_ENCRTYPE_TKIP | WSC_ENCRTYPE_AES):
		return "TKIPAES";

	case WSC_ENCRTYPE_AES:
		return "AES";
	}
}

#ifdef CONFIG_AP_SUPPORT
static UINT32 WscGetAuthMode(
	IN  USHORT authFlag)
{
	UINT32 AKMMap = 0;

	switch (authFlag) {
	case WSC_AUTHTYPE_OPEN:
		SET_AKM_OPEN(AKMMap);
		break;

	case WSC_AUTHTYPE_WPAPSK:
		SET_AKM_WPA1PSK(AKMMap);
		break;

	case WSC_AUTHTYPE_SHARED:
		SET_AKM_SHARED(AKMMap);
		break;

	case WSC_AUTHTYPE_WPANONE:
		SET_AKM_WPANONE(AKMMap);
		break;

	case WSC_AUTHTYPE_WPA:
		SET_AKM_WPA1(AKMMap);
		break;

	case WSC_AUTHTYPE_WPA2:
		SET_AKM_WPA2(AKMMap);
		break;

	case (WSC_AUTHTYPE_WPAPSK | WSC_AUTHTYPE_WPA2PSK):
		SET_AKM_WPA1PSK(AKMMap);
		SET_AKM_WPA2PSK(AKMMap);
		break;

	case WSC_AUTHTYPE_WPA2PSK:
		SET_AKM_WPA2PSK(AKMMap);
		break;
	}

	return AKMMap;
}

static UINT32 WscGetWepStatus(
	IN  USHORT encryFlag)
{
	UINT32 EncryType = 0;

	switch (encryFlag) {
	case WSC_ENCRTYPE_NONE:
		SET_CIPHER_NONE(EncryType);
		break;

	case WSC_ENCRTYPE_WEP:
		SET_CIPHER_WEP(EncryType);
		break;

	case WSC_ENCRTYPE_TKIP:
		SET_CIPHER_TKIP(EncryType);
		break;

	case (WSC_ENCRTYPE_TKIP | WSC_ENCRTYPE_AES):
		SET_CIPHER_TKIP(EncryType);
		SET_CIPHER_CCMP128(EncryType);
		break;

	case WSC_ENCRTYPE_AES:
		SET_CIPHER_CCMP128(EncryType);
		break;
	}

	return EncryType;
}
#endif /* CONFIG_AP_SUPPORT */

static VOID WscSetWepCipher(
	IN  struct _SECURITY_CONFIG *pSecConfig,
	IN  PWSC_CREDENTIAL pCredential,
	IN  UCHAR WepKeyId,
	IN  USHORT WepKeyLen)
{
	if (WepKeyLen == 5 || WepKeyLen == 13) {
		pSecConfig->WepKey[WepKeyId].KeyLen = (UCHAR)WepKeyLen;
		os_move_mem(pSecConfig->WepKey[WepKeyId].Key, pCredential->Key, WepKeyLen);
		CLEAR_CIPHER(pSecConfig->PairwiseCipher);

		if (WepKeyLen == 5)
			SET_CIPHER_WEP40(pSecConfig->PairwiseCipher);
		else
			SET_CIPHER_WEP104(pSecConfig->PairwiseCipher);
	} else {
		pSecConfig->WepKey[WepKeyId].KeyLen = (UCHAR)(WepKeyLen / 2);
		AtoH((RTMP_STRING *) pCredential->Key, pSecConfig->WepKey[WepKeyId].Key, WepKeyLen / 2);

		if (WepKeyLen == 10)
			SET_CIPHER_WEP40(pSecConfig->PairwiseCipher);
		else
			SET_CIPHER_WEP104(pSecConfig->PairwiseCipher);
	}
}

void WscWriteConfToPortCfg(
	IN  PRTMP_ADAPTER pAd,
	IN  PWSC_CTRL pWscControl,
	IN  PWSC_CREDENTIAL pCredential,
	IN  BOOLEAN bEnrollee)
{
	UCHAR CurApIdx = MAIN_MBSSID;
	UCHAR CurOpMode = AP_MODE;
	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("-----> WscWriteConfToPortCfg\n"));
#ifdef CONFIG_AP_SUPPORT

	if (CurOpMode == AP_MODE)
		CurApIdx = (pWscControl->EntryIfIdx & 0x0F);

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_AP_SUPPORT

	if (bEnrollee || (CurOpMode == AP_MODE)) {
		if (CurOpMode == AP_MODE) {
			NdisZeroMemory(pAd->ApCfg.MBSSID[CurApIdx].Ssid, MAX_LEN_OF_SSID);
			NdisMoveMemory(pAd->ApCfg.MBSSID[CurApIdx].Ssid, pCredential->SSID.Ssid, pCredential->SSID.SsidLength);
			pAd->ApCfg.MBSSID[CurApIdx].SsidLen = pCredential->SSID.SsidLength;
		}

#endif /* CONFIG_AP_SUPPORT */
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("ra%d - AuthType: %u, EncrType: %u\n", CurApIdx, pCredential->AuthType, pCredential->EncrType));

		if (pCredential->AuthType & (WSC_AUTHTYPE_WPAPSK | WSC_AUTHTYPE_WPA2PSK | WSC_AUTHTYPE_WPANONE)) {
			if (!(pCredential->EncrType & (WSC_ENCRTYPE_TKIP | WSC_ENCRTYPE_AES))) {
				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("AuthType is WPAPSK or WPA2PAK.\n"
						 "Get illegal EncrType(%d) from External Registrar, set EncrType to TKIP\n",
						 pCredential->EncrType));
				pCredential->EncrType = WSC_ENCRTYPE_TKIP;
			}

#ifdef CONFIG_AP_SUPPORT
#ifdef WSC_V2_SUPPORT

			if ((CurOpMode == AP_MODE) && (pWscControl->WscV2Info.bEnableWpsV2)) {
				if (pCredential->AuthType == WSC_AUTHTYPE_WPAPSK)
					pCredential->AuthType = (WSC_AUTHTYPE_WPAPSK | WSC_AUTHTYPE_WPA2PSK);

				if (pCredential->EncrType == WSC_ENCRTYPE_TKIP)
					pCredential->EncrType = (WSC_ENCRTYPE_TKIP | WSC_ENCRTYPE_AES);
			}

#endif /* WSC_V2_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
		}

		WscSetAuthMode(pAd, CurOpMode, CurApIdx, WscGetAuthTypeStr(pCredential->AuthType));
		WscSetEncrypType(pAd, CurOpMode, CurApIdx, WscGetEncryTypeStr(pCredential->EncrType));

		if (pCredential->EncrType != WSC_ENCRTYPE_NONE) {
			if (pCredential->EncrType & (WSC_ENCRTYPE_TKIP | WSC_ENCRTYPE_AES)) {
				struct wifi_dev *p_wdev = NULL;
#ifdef CONFIG_AP_SUPPORT

				if (CurOpMode == AP_MODE) {
					p_wdev = &pAd->ApCfg.MBSSID[CurApIdx].wdev;
					p_wdev->SecConfig.PairwiseKeyId = 1;
				}

#endif /* CONFIG_AP_SUPPORT */

				if (pCredential->KeyLength >= 8 && pCredential->KeyLength <= 64) {
					RTMP_STRING PassphraseStr[65] = {0};

					pWscControl->WpaPskLen = pCredential->KeyLength;
					RTMPZeroMemory(pWscControl->WpaPsk, 64);
					RTMPMoveMemory(pWscControl->WpaPsk, pCredential->Key, pWscControl->WpaPskLen);
					RTMPMoveMemory(PassphraseStr, pCredential->Key, pWscControl->WpaPskLen);
					RTMPZeroMemory(p_wdev->SecConfig.PSK, LEN_PSK + 1);
					RTMPMoveMemory(p_wdev->SecConfig.PSK, pCredential->Key, pWscControl->WpaPskLen);
					MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("WpaPskLen = %d\n", pWscControl->WpaPskLen));
				} else {
					pWscControl->WpaPskLen = 0;
					MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("WPAPSK: Invalid Key Length (%d)\n", pCredential->KeyLength));
				}
			} else if (pCredential->EncrType == WSC_ENCRTYPE_WEP) { /* Only for WPS 2.0 */
				UCHAR   WepKeyId = 0;
				USHORT  WepKeyLen = pCredential->KeyLength;

				if ((pCredential->KeyIndex >= 1) && (pCredential->KeyIndex <= 4)) {
					struct _SECURITY_CONFIG *pSecConfig = NULL;

					WepKeyId = (pCredential->KeyIndex - 1); /* KeyIndex = 1 ~ 4 */
#ifdef CONFIG_AP_SUPPORT

					if (CurOpMode == AP_MODE)
						pSecConfig = &pAd->ApCfg.MBSSID[CurApIdx].wdev.SecConfig;

#endif /* CONFIG_AP_SUPPORT */
					pSecConfig->PairwiseKeyId = WepKeyId;

					/* 5 or 13 ASCII characters */
					/* 10 or 26 Hex characters */
					if (WepKeyLen == 5 || WepKeyLen == 13 || WepKeyLen == 10 || WepKeyLen == 26)
						WscSetWepCipher(pSecConfig, pCredential, WepKeyId, WepKeyLen);
					else
						MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE,
						("WEP: Invalid Key Length (%d)\n", pCredential->KeyLength));
				} else {
					MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE,
						("Unsupport default key index (%d)\n", WepKeyId));
#ifdef CONFIG_AP_SUPPORT
					if (CurOpMode == AP_MODE)
						pAd->ApCfg.MBSSID[CurApIdx].wdev.SecConfig.PairwiseKeyId = 0;
#endif /* CONFIG_AP_SUPPORT */
				}
			}
		}

#ifdef CONFIG_AP_SUPPORT
	}
#endif /* CONFIG_AP_SUPPORT */
	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE,
	("<----- ra%d - WscWriteConfToPortCfg\n", CurApIdx));
}


VOID	WscWriteSsidToDatFile(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING *pTempStr,
	IN  BOOLEAN bNewFormat,
	IN  UCHAR CurOpMode)
{
#ifdef CONFIG_AP_SUPPORT
	UCHAR	apidx;
#endif /* CONFIG_AP_SUPPORT */
	INT offset = 0;

	if (bNewFormat == FALSE) {
		NdisZeroMemory(pTempStr, 512);
#ifdef CONFIG_AP_SUPPORT

		if (CurOpMode == AP_MODE) {
			for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++) {
				if (apidx == 0) {
					NdisMoveMemory(pTempStr, "SSID=", strlen("SSID="));
					offset = strlen(pTempStr);
				} else {
					offset = strlen(pTempStr);
					NdisMoveMemory(pTempStr + offset, ";", 1);
					offset += 1;
				}

				NdisMoveMemory(pTempStr + offset, pAd->ApCfg.MBSSID[apidx].Ssid, pAd->ApCfg.MBSSID[apidx].SsidLen);
			}
		}

#endif /* CONFIG_AP_SUPPORT */
	}

#ifdef CONFIG_AP_SUPPORT
	else {
		RTMP_STRING item_str[10] = {0};

		for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++) {
			snprintf(item_str, sizeof(item_str), "SSID%d", (apidx + 1));

			if (rtstrstr(pTempStr, item_str)) {
				NdisZeroMemory(pTempStr, 512);
				NdisMoveMemory(pTempStr, item_str, strlen(item_str));
				offset = strlen(pTempStr);
				NdisMoveMemory(pTempStr + offset, "=", 1);
				offset += 1;
				NdisMoveMemory(pTempStr + offset, pAd->ApCfg.MBSSID[apidx].Ssid, pAd->ApCfg.MBSSID[apidx].SsidLen);
			}

			NdisZeroMemory(item_str, 10);
		}
	}

#endif /* CONFIG_AP_SUPPORT */
}


VOID WscWriteWpaPskToDatFile(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING *pTempStr,
	IN  BOOLEAN bNewFormat)
{
#ifdef CONFIG_AP_SUPPORT
	UCHAR apidx;
#endif /* CONFIG_AP_SUPPORT */
	PWSC_CTRL pWscControl;
	INT offset = 0;

	if (bNewFormat == FALSE) {
		NdisZeroMemory(pTempStr, 512);
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++) {
				pWscControl = &pAd->ApCfg.MBSSID[apidx].wdev.WscControl;

				if (apidx == 0) {
					NdisMoveMemory(pTempStr, "WPAPSK=", strlen("WPAPSK="));
					offset = strlen(pTempStr);
				} else {
					offset = strlen(pTempStr);
					NdisMoveMemory(pTempStr + offset, ";", 1);
					offset += 1;
				}

				NdisMoveMemory(pTempStr + offset, pWscControl->WpaPsk, pWscControl->WpaPskLen);
			}
		}
#endif /* CONFIG_AP_SUPPORT */
	}

#ifdef CONFIG_AP_SUPPORT
	else {
		RTMP_STRING item_str[10] = {0};

		for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++) {
			snprintf(item_str, sizeof(item_str), "WPAPSK%d", (apidx + 1));

			if (rtstrstr(pTempStr, item_str)) {
				pWscControl = &pAd->ApCfg.MBSSID[apidx].wdev.WscControl;
				NdisZeroMemory(pTempStr, 512);
				NdisMoveMemory(pTempStr, item_str, strlen(item_str));
				offset = strlen(pTempStr);
				NdisMoveMemory(pTempStr + offset, "=", 1);
				offset += 1;
				NdisMoveMemory(pTempStr + offset, pWscControl->WpaPsk, pWscControl->WpaPskLen);
			}

			NdisZeroMemory(item_str, 10);
		}
	}

#endif /* CONFIG_AP_SUPPORT */
}

static BOOLEAN WscCheckNonce(
	IN  PRTMP_ADAPTER pAdapter,
	IN  MLME_QUEUE_ELEM * pElem,
	IN  BOOLEAN bFlag,
	IN  PWSC_CTRL pWscControl)
{
	USHORT Length;
	PUCHAR pData;
	USHORT WscType, WscLen, WscId;

	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("-----> WscCheckNonce\n"));

	if (bFlag) {
		/* check Registrar Nonce */
		WscId = WSC_ID_REGISTRAR_NONCE;
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("check Registrar Nonce\n"));
	} else {
		/* check Enrollee Nonce */
		WscId = WSC_ID_ENROLLEE_NONCE;
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("check Enrollee Nonce\n"));
	}

	pData = pElem->Msg;
	Length = pElem->MsgLen;

	/* We have to look for WSC_IE_MSG_TYPE to classify M2 ~ M8, the remain size must large than 4 */
	while (Length > 4) {
		WSC_IE	TLV_Recv;
		char ZeroNonce[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

		memcpy((UINT8 *)&TLV_Recv, pData, 4);
		WscType = be2cpu16(TLV_Recv.Type);
		WscLen  = be2cpu16(TLV_Recv.Length);
		pData  += 4;
		Length -= 4;

		if (WscType == WscId) {
			if (RTMPCompareMemory(pWscControl->RegData.SelfNonce, pData, 16) == 0) {
				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("Nonce match!!\n"));
				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("<----- WscCheckNonce\n"));
				return TRUE;
			} else if (NdisEqualMemory(pData, ZeroNonce, 16)) {
				/* Intel external registrar will send WSC_NACK with enrollee nonce */
				/* "10 1A 00 10 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00" */
				/* when AP is configured and user selects not to configure AP. */
				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("Zero Enrollee Nonce!!\n"));
				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("<----- WscCheckNonce\n"));
				return TRUE;
			}
		}

		/* Offset to net WSC Ie */
		pData  += WscLen;
		Length -= WscLen;
	}

	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("Nonce mismatch!!\n"));
	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("<----- WscCheckNonce\n"));
	return FALSE;
}

VOID WscGetRegDataPIN(
	IN  PRTMP_ADAPTER pAdapter,
	IN  UINT PinCode,
	IN  PWSC_CTRL pWscControl)
{
	UCHAR tempPIN[9] = {0};

	if ((pWscControl->WscMode == WSC_PBC_MODE) ||
		(pWscControl->WscMode == WSC_SMPBC_MODE))
		pWscControl->WscPinCode = 0;
	else
		pWscControl->WscPinCode = PinCode;

	memset(pWscControl->RegData.PIN, 0, 8);

	if (pWscControl->WscPinCode == 0) {
		snprintf((RTMP_STRING *) tempPIN, sizeof(tempPIN), "00000000");
		memcpy(pWscControl->RegData.PIN, tempPIN, 8);
		pWscControl->RegData.PinCodeLen = 8;
	} else {
		if (pWscControl->WscPinCodeLen == 4) {
			UCHAR	temp4PIN[5] = {0};

			snprintf((RTMP_STRING *) temp4PIN, sizeof(temp4PIN), "%04u", pWscControl->WscPinCode);
			memcpy(pWscControl->RegData.PIN, temp4PIN, 4);
			pWscControl->RegData.PinCodeLen = 4;
		} else {
			snprintf((RTMP_STRING *) tempPIN, sizeof(tempPIN), "%08u", pWscControl->WscPinCode);
			memcpy(pWscControl->RegData.PIN, tempPIN, 8);
			pWscControl->RegData.PinCodeLen = 8;
		}
	}

	hex_dump("WscGetRegDataPIN - PIN", pWscControl->RegData.PIN, 8);
}


static VOID WscGetConfigErrFromNack(
	IN RTMP_ADAPTER *pAdapter,
	IN MLME_QUEUE_ELEM * pElem,
	OUT USHORT *pConfigError)
{
	USHORT Length = 0;
	PUCHAR pData;
	USHORT WscType, WscLen, ConfigError = 0;

	pData = pElem->Msg;
	Length = pElem->MsgLen;

	while (Length > 4) {
		WSC_IE	TLV_Recv;

		memcpy((UINT8 *)&TLV_Recv, pData, 4);
		WscType = be2cpu16(TLV_Recv.Type);
		WscLen  = be2cpu16(TLV_Recv.Length);
		pData  += 4;
		Length -= 4;

		if (WscType == WSC_ID_CONFIG_ERROR) {
			NdisMoveMemory(&ConfigError, pData, sizeof(USHORT));
			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("WSC_ID_CONFIG_ERROR: %d\n", ntohs(ConfigError)));
			*pConfigError = ntohs(ConfigError);
			return;
		}

		/* Offset to net WSC Ie */
		pData  += WscLen;
		Length -= WscLen;
	}

	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("WSC_ID_CONFIG_ERROR is missing\n"));
}


static INT WscSetAuthMode(
	IN  PRTMP_ADAPTER pAd,
	IN  UCHAR CurOpMode,
	IN  UCHAR apidx,
	IN  RTMP_STRING *arg)
{
#ifdef CONFIG_AP_SUPPORT

	if (CurOpMode == AP_MODE) {
		UINT32	i;
		struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
		struct _SECURITY_CONFIG *pSecConfig = NULL;

		pSecConfig = &wdev->SecConfig;
		SetWdevAuthMode(pSecConfig, arg);

		for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
			if (IS_ENTRY_CLIENT(&pAd->MacTab.Content[i]))
				pAd->MacTab.tr_entry[i].PortSecured  = WPA_802_1X_PORT_NOT_SECURED;
		}

		pAd->ApCfg.MBSSID[apidx].wdev.PortSecured = WPA_802_1X_PORT_NOT_SECURED;
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("IF(ra%d) %s::(AuthMode=0x%x)\n", apidx, __func__, wdev->SecConfig.AKMMap));
	}

#endif /* CONFIG_AP_SUPPORT */
	return TRUE;
}

static INT WscSetEncrypType(
	IN  PRTMP_ADAPTER pAd,
	IN  UCHAR CurOpMode,
	IN  UCHAR apidx,
	IN  RTMP_STRING *arg)
{
#ifdef CONFIG_AP_SUPPORT

	if (CurOpMode == AP_MODE) {
		struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
		struct _SECURITY_CONFIG *pSecConfig = NULL;

		pSecConfig = &wdev->SecConfig;
		SetWdevEncrypMode(pSecConfig, arg);
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("IF(ra%d) %s::(EncrypType=0x%x)\n", apidx, __func__, wdev->SecConfig.PairwiseCipher));
	}

#endif /* CONFIG_AP_SUPPORT */
	return TRUE;
}


/*
*	========================================================================
*
*	Routine Description:
*		Push PBC from HW/SW Buttton
*
*	Arguments:
*		pAd    - NIC Adapter pointer
*
*	Return Value:
*		None
*
*	IRQL = DISPATCH_LEVEL
*
*	Note:
*
*	========================================================================
*/

VOID  WscPushPBCAction(
	IN  PRTMP_ADAPTER pAd,
	IN  PWSC_CTRL pWscControl)
{
	BOOLEAN Cancelled;

	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("-----> WscPushPBCAction\n"));

	/* 0. PBC mode, disregard the SSID information, we have to get the current AP list */
	/*    and check the beacon for Push buttoned AP. */
	/* 1. Cancel old timer to prevent use push continuously */
	if (pWscControl->Wsc2MinsTimerRunning) {
		pWscControl->Wsc2MinsTimerRunning = FALSE;
		RTMPCancelTimer(&pWscControl->Wsc2MinsTimer, &Cancelled);
	}

	if (pWscControl->WscScanTimerRunning) {
		pWscControl->WscScanTimerRunning = FALSE;
		RTMPCancelTimer(&pWscControl->WscScanTimer, &Cancelled);
	}

	if (pWscControl->WscPBCTimerRunning) {
		pWscControl->WscPBCTimerRunning = FALSE;
		RTMPCancelTimer(&pWscControl->WscPBCTimer, &Cancelled);
	}

	/* Set WSC state to WSC_STATE_INIT */
	pWscControl->WscState = WSC_STATE_START;
	pWscControl->WscStatus = STATUS_WSC_SCAN_AP;
	/* Init Registrar pair structures */
	WscInitRegistrarPair(pAd, pWscControl, BSS0);
	/* For PBC, the PIN is all '0' */
	WscGetRegDataPIN(pAd, pWscControl->WscPinCode, pWscControl);
	/* 2. Set 2 min timout routine */
	RTMPSetTimer(&pWscControl->Wsc2MinsTimer, WSC_TWO_MINS_TIME_OUT);
	pWscControl->Wsc2MinsTimerRunning = TRUE;
	pWscControl->bWscTrigger = TRUE;	/* start work */
	/* 3. Call WscScan subroutine */
	WscScanExec(pAd, pWscControl);

	/* 4. Set 10 second timer to invoke PBC connection actions. */
#ifdef CON_WPS /*Move to scan complete to trigger //Dung_Ru*/

	if (pWscControl->conWscStatus == CON_WPS_STATUS_DISABLED)
#endif /* CON_WPS */
	{
#ifdef APCLI_SUPPORT
		if (pWscControl->WscApCliScanMode != TRIGGER_PARTIAL_SCAN)
#endif /* APCLI_SUPPORT */
		{
#if defined(CONFIG_MAP_SUPPORT)
			if (!IS_MAP_TURNKEY_ENABLE(pAd))
#endif
			{
				RTMPSetTimer(&pWscControl->WscPBCTimer, 10000);
				pWscControl->WscPBCTimerRunning = TRUE;
			}
		}
	}
	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("<----- WscPushPBCAction\n"));
}


#ifdef WSC_STA_SUPPORT

/*
*	========================================================================
*
*	Routine Description:
*		PIN from SW Buttton
*
*	Arguments:
*		pAd    - NIC Adapter pointer
*
*	Return Value:
*		None
*
*	IRQL = DISPATCH_LEVEL
*
*	Note:
*
*	========================================================================
*/

VOID WscPINAction(
	IN  PRTMP_ADAPTER pAd,
	IN  PWSC_CTRL pWscControl)
{
	BOOLEAN Cancelled;

	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("-----> WscPINAction\n"));

	/* 0. PIN mode, disregard the SSID information, we have to get the current AP list */
	/*    and check the beacon for WSC PIN AP. */

	/* 1. Cancel old timer to prevent use push continuously */
	if (pWscControl->Wsc2MinsTimerRunning) {
		pWscControl->Wsc2MinsTimerRunning = FALSE;
		RTMPCancelTimer(&pWscControl->Wsc2MinsTimer, &Cancelled);
	}

	if (pWscControl->WscScanTimerRunning) {
		pWscControl->WscScanTimerRunning = FALSE;
		RTMPCancelTimer(&pWscControl->WscScanTimer, &Cancelled);
	}

	if (pWscControl->WscPINTimerRunning) {
		pWscControl->WscPINTimerRunning = FALSE;
		RTMPCancelTimer(&pWscControl->WscPINTimer, &Cancelled);
	}

	/* Set WSC state to WSC_STATE_INIT */
	pWscControl->WscState = WSC_STATE_START;
	pWscControl->WscStatus = STATUS_WSC_SCAN_AP;
	/* Init Registrar pair structures */
	WscInitRegistrarPair(pAd, pWscControl, BSS0);
	/* For PBC, the PIN is all '0' */
	/* WscGetRegDataPIN(pAd, pWscControl->WscPinCode, pWscControl); */
	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("Enrollee_pin_code-----> %u\n", pWscControl->WscEnrolleePinCode));
	/* 2. Set 2 min timout routine */
	RTMPSetTimer(&pWscControl->Wsc2MinsTimer, WSC_TWO_MINS_TIME_OUT);
	pWscControl->Wsc2MinsTimerRunning = TRUE;
	pWscControl->bWscTrigger = TRUE;	/* start work */
	/* 3. Call WscScan subroutine */
	WscScanExec(pAd, pWscControl);
	/* 4. Set 10 second timer to invoke PIN connection actions. */
	RTMPSetTimer(&pWscControl->WscPINTimer, 10000);
	pWscControl->WscPINTimerRunning = TRUE;
	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("<----- WscPINAction\n"));
}

#endif

/*
*	========================================================================
*
*	Routine Description:
*		Doing an active scan with empty SSID, the scanened list will
*		be processed in PBCexec or PINexec routines
*
*	Arguments:
*		pAd         - NIC Adapter pointer
*
*	Return Value:
*		None
*
*	IRQL = DISPATCH_LEVEL
*
*	Note:
*
*	========================================================================
*/
VOID WscScanExec(
	IN  PRTMP_ADAPTER pAd,
	IN  PWSC_CTRL pWscControl)
{
#ifdef WSC_LED_SUPPORT
	UCHAR WPSLEDStatus;
#endif /* WSC_LED_SUPPORT */
#ifdef APCLI_SUPPORT
	UCHAR if_idx = (pWscControl->EntryIfIdx & 0x0F);
#endif /*APCLI_SUPPORT*/

	/* Prevent infinite loop if conncet time out didn't stop the repeat scan */
	if ((pWscControl->WscStatus == STATUS_WSC_FAIL) ||
		(pWscControl->WscState == WSC_STATE_OFF))
		return;

	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_OFF, ("!!! WscScanExec !!!\n"));
	pWscControl->WscStatus = STATUS_WSC_SCAN_AP;
#ifdef WSC_LED_SUPPORT
	/* The protocol is connecting to a partner. */
	WPSLEDStatus = LED_WPS_IN_PROCESS;
	RTMPSetLED(pAd, WPSLEDStatus);
#endif /* WSC_LED_SUPPORT */
#ifdef APCLI_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		if (if_idx < MAX_APCLI_NUM) {
#ifdef AP_SCAN_SUPPORT
			if (pWscControl->WscApCliScanMode == TRIGGER_PARTIAL_SCAN) {
				if ((!pAd->ScanCtrl.PartialScan.bScanning) &&
					(pAd->ScanCtrl.PartialScan.LastScanChannel == 0)) {
					pAd->ScanCtrl.PartialScan.pwdev = &pAd->ApCfg.ApCliTab[if_idx].wdev;
					pAd->ScanCtrl.PartialScan.bScanning = TRUE;
				}
			}
#endif /* AP_SCAN_SUPPORT */
#ifdef CON_WPS
			pAd->ApCfg.ApCliTab[if_idx].ConWpsApCliModeScanDoneStatus = CON_WPS_APCLI_SCANDONE_STATUS_ONGOING;
#endif /*CON_WPS*/
			ApSiteSurvey_by_wdev(pAd, NULL, SCAN_WSC_ACTIVE, FALSE, &pAd->ApCfg.ApCliTab[if_idx].wdev);
		}
	}
#endif /* APCLI_SUPPORT */
}

/*
*	========================================================================
*
*	Routine Description:
*		Doing PBC conenction verification, it will check current BSS list
*		and find the correct number of PBC AP. If only 1 exists, it will
*		start to make connection. Otherwise, it will set a scan timer
*		to perform another scan for next PBC connection execution.
*
*	Arguments:
*		pAd         - NIC Adapter pointer
*
*	Return Value:
*		None
*
*	IRQL = DISPATCH_LEVEL
*
*	Note:
*
*	========================================================================
*/
BOOLEAN	WscPBCExec(
	IN  PRTMP_ADAPTER pAd,
	IN  BOOLEAN bFromM2,
	IN  PWSC_CTRL pWscControl)
{
#ifdef WSC_LED_SUPPORT
	UCHAR WPSLEDStatus;
#endif /* WSC_LED_SUPPORT */
	struct wifi_dev *wdev;
	UCHAR CurOpMode = AP_MODE;

	if (pWscControl == NULL) {
		return FALSE;
	}

	wdev = (struct wifi_dev *)pWscControl->wdev;
	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_OFF,
			 ("-----> WscPBCExec (CurOpMode=%d) !!!\n", CurOpMode));
	/* 1. Search the qualified SSID from current SSID list */
	WscPBCBssTableSort(pAd, pWscControl);

	/* 2. Check the qualified AP for connection, if more than 1 AP avaliable, report error. */
	if (pWscControl->WscPBCBssCount != 1) {
		/* Set WSC state to WSC_FAIL */
		pWscControl->WscState = WSC_STATE_FAIL;

		if (pWscControl->WscPBCBssCount == 0) {
			pWscControl->WscStatus = STATUS_WSC_PBC_NO_AP;
#ifdef WSC_LED_SUPPORT
			/* Failed to find any partner. */
			WPSLEDStatus = LED_WPS_ERROR;
			RTMPSetLED(pAd, WPSLEDStatus);
#ifdef CONFIG_WIFI_LED_SUPPORT

			if (LED_MODE(pAd) == WPS_LED_MODE_SHARE)
				RTMPSetTimer(&pWscControl->WscLEDTimer, WSC_WPS_FAIL_WIFI_LED_TIMEOUT);

#endif /* CONFIG_WIFI_LED_SUPPORT */
#endif /* WSC_LED_SUPPORT */
			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_OFF, ("WscPBCExec --> AP list is %d, wait for next time\n",
					 pWscControl->WscPBCBssCount));
			{
				/* 2.1. Set 1 second timer to invoke another scan */
				RTMPSetTimer(&pWscControl->WscScanTimer, 1000);
				pWscControl->WscScanTimerRunning = TRUE;
			}
		} else {
			pWscControl->WscStatus = STATUS_WSC_PBC_TOO_MANY_AP;
				RTMPSendWirelessEvent(pAd, IW_WSC_PBC_SESSION_OVERLAP, NULL, BSS0, 0);

#ifdef WSC_LED_SUPPORT

			if (LED_MODE(pAd) == WPS_LED_MODE_9) { /* WPS LED mode 9. */
				/* In case of the WPS LED mode 9, the UI would abort the connection attempt by making the RT_OID_802_11_WSC_SET_WPS_STATE_MACHINE_TERMINATION request. */
				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("%s: Skip the WPS session overlap detected LED indication.\n", __func__));
			} else { /* Other LED mode. */
				/* Session overlap detected. */
				WPSLEDStatus = LED_WPS_SESSION_OVERLAP_DETECTED;
				RTMPSetLED(pAd, WPSLEDStatus);
			}

#endif /* WSC_LED_SUPPORT */
			/*
			*	20101210 - According to the response from WFA:
			*	The station shall not continue scanning waiting for only one registrar to appear
			*/
			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("WscPBCExec --> AP list is %d, stop WPS process!\n",
					 pWscControl->WscPBCBssCount));
			WscStop(pAd,
#ifdef CONFIG_AP_SUPPORT
					FALSE,
#endif /* CONFIG_AP_SUPPORT */
					pWscControl);
			pWscControl->WscConfMode = WSC_DISABLE;
		}

		/* 2.2 We have to quit for now */
		return FALSE;
	}

	if (bFromM2)
		return TRUE;

	pWscControl->WscState = WSC_STATE_START;
	pWscControl->WscStatus = STATUS_WSC_START_ASSOC;
#ifdef WSC_LED_SUPPORT
	/* The protocol is connecting to a partner. */
	WPSLEDStatus = LED_WPS_IN_PROCESS;
	RTMPSetLED(pAd, WPSLEDStatus);
#endif /* WSC_LED_SUPPORT */
#ifdef APCLI_SUPPORT

	if (CurOpMode == AP_MODE) {
		UCHAR apcli_idx = (pWscControl->EntryIfIdx & 0x0F);
		NdisMoveMemory(pWscControl->RegData.SelfInfo.MacAddr,
					   wdev->if_addr,
					   MAC_ADDR_LEN);
#ifdef RTMP_TIMER_TASK_SUPPORT
		rtmp_set_channel(pAd, wdev, pAd->ApCfg.ApCliTab[apcli_idx].MlmeAux.Channel);
#else
		RTEnqueueInternalCmd(pAd, CMDTHREAD_APCLI_PBC_TIMEOUT, (VOID *)&pAd->ApCfg.ApCliTab[apcli_idx], sizeof(APCLI_STRUCT));
#endif
		/* bring apcli interface down first */
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE,
				 ("!!! WscPBCExec --> Goto CMDTHREAD_APCLI_IF_DOWN!, pAd->ApCfg.ApCliTab[%d].Enable=%d  !!!\n",
				  apcli_idx,
				  pAd->ApCfg.ApCliTab[apcli_idx].Enable));
		{
			/* at this point is disable, need to turn on it! */
			pAd->ApCfg.ApCliTab[apcli_idx].Enable = TRUE;
			RTEnqueueInternalCmd(pAd, CMDTHREAD_APCLI_IF_DOWN, (VOID *)&apcli_idx, sizeof(UCHAR));
		}
	}

#endif /* APCLI_SUPPORT */
	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_OFF, ("<----- WscPBCExec !!!\n"));
	return TRUE;
}

#ifdef WSC_STA_SUPPORT
/*
*	========================================================================
*
*	Routine Description:
*		Doing PIN conenction verification, it will check current BSS list
*		and find the correct number of PIN AP. If only 1 exists, it will
*		start to make connection. Otherwise, it will set a scan timer
*		to perform another scan for next PIN connection execution.
*
*	Arguments:
*		pAd         - NIC Adapter pointer
*
*	Return Value:
*		None
*
*	IRQL = DISPATCH_LEVEL
*
*	Note:
*
*	========================================================================
*/
BOOLEAN	WscPINExec(
	IN  PRTMP_ADAPTER pAd,
	IN  BOOLEAN bFromM2,
	IN  PWSC_CTRL pWscControl)
{
#ifdef WSC_LED_SUPPORT
	UCHAR WPSLEDStatus;
#endif /* WSC_LED_SUPPORT */

	if (pWscControl == NULL)
		return FALSE;

	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_OFF, ("-----> WscPINExec !!!\n"));
	/* 1. Search the qualified SSID from current SSID list */
	WscPINBssTableSort(pAd, pWscControl);

	/* 2. Check the qualified AP for connection, if more than 1 AP with same mac addr avaliable, report error. */
	if (pWscControl->WscPINBssCount != 1) {
		/* Set WSC state to WSC_FAIL */
		pWscControl->WscState = WSC_STATE_FAIL;

		if (pWscControl->WscPINBssCount == 0) {
			pWscControl->WscStatus = STATUS_WSC_PBC_NO_AP;	/*No error code regarding PIN, so use PBC error code*/
#ifdef WSC_LED_SUPPORT
			/* Failed to find any partner. */
			WPSLEDStatus = LED_WPS_ERROR;
			RTMPSetLED(pAd, WPSLEDStatus);
#ifdef CONFIG_WIFI_LED_SUPPORT

			if (LED_MODE(pAd) == WPS_LED_MODE_SHARE)
				RTMPSetTimer(&pWscControl->WscLEDTimer, WSC_WPS_FAIL_WIFI_LED_TIMEOUT);

#endif /* CONFIG_WIFI_LED_SUPPORT */
#endif /* WSC_LED_SUPPORT */
			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_OFF, ("WscPINExec --> AP list is %d, wait for next time\n",
					 pWscControl->WscPINBssCount));
			{
				/* 2.1. Set 1 second timer to invoke another scan */
				RTMPSetTimer(&pWscControl->WscScanTimer, 1000);
				pWscControl->WscScanTimerRunning = TRUE;
			}
		} else {
			/* should never hit this case, as in PIN, bssid is preset */
			pWscControl->WscStatus = STATUS_WSC_PBC_TOO_MANY_AP;
				RTMPSendWirelessEvent(pAd, IW_WSC_PBC_SESSION_OVERLAP, NULL, BSS0, 0);

#ifdef WSC_LED_SUPPORT

			if (LED_MODE(pAd) == WPS_LED_MODE_9) { /* WPS LED mode 9. */
				/* In case of the WPS LED mode 9, the UI would abort the connection attempt by making the RT_OID_802_11_WSC_SET_WPS_STATE_MACHINE_TERMINATION request. */
				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("%s: Skip the WPS session overlap detected LED indication.\n", __func__));
			} else { /* Other LED mode. */
				/* Session overlap detected. */
				WPSLEDStatus = LED_WPS_SESSION_OVERLAP_DETECTED;
				RTMPSetLED(pAd, WPSLEDStatus);
			}

#endif /* WSC_LED_SUPPORT */
			/*
			*	20101210 - According to the response from WFA:
			*	The station shall not continue scanning waiting for only one registrar to appear
			*/
			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("WscPINExec --> AP list is %d, stop WPS process!\n",
					 pWscControl->WscPINBssCount));
			WscStop(pAd,
#ifdef CONFIG_AP_SUPPORT
					FALSE,
#endif /* CONFIG_AP_SUPPORT */
					pWscControl);
			pWscControl->WscConfMode = WSC_DISABLE;
		}

		/* 2.2 We have to quit for now */
		return FALSE;
	}

	if (bFromM2) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_OFF, ("bfromM2\n"));
		return TRUE;
	}

#ifdef WSC_LED_SUPPORT
	/* The protocol is connecting to a partner. */
	WPSLEDStatus = LED_WPS_IN_PROCESS;
	RTMPSetLED(pAd, WPSLEDStatus);
#endif /* WSC_LED_SUPPORT */
	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_OFF, ("<----- WscPINExec !!!\n"));
	return TRUE;
}
#endif

static VOID WscCheckIsSameAP(
	IN  UUID_BSSID_CH_INFO	*ApUuidBssid,
	IN  UUID_BSSID_CH_INFO	*TmpInfo,
	IN  UCHAR *zeros16,
	OUT BOOLEAN *bSameAP)
{
	if (RTMPCompareMemory(&ApUuidBssid->Uuid[0], &TmpInfo->Uuid[0], 16) == 0) {
		if (RTMPCompareMemory(&TmpInfo->Uuid[0], zeros16, 16) != 0) {
			/*
			*	Same UUID, indicate concurrent AP
			*	We can indicate 1 AP only.
			*/
			*bSameAP = TRUE;
		} else if (RTMPCompareMemory(&TmpInfo->Uuid[0], zeros16, 16) == 0) {
			if (ApUuidBssid->Band != TmpInfo->Band) {
				if (RTMPCompareMemory(&ApUuidBssid->Bssid[0], &TmpInfo->Bssid[0], 5) == 0) {
					/*
					*	Zero UUID at different band, and first 5bytes of two BSSIDs are the same.
					*	Indicate concurrent AP, we can indicate 1 AP only.
					*/
					*bSameAP = TRUE;
				}
			}
		}
	} else if ((RTMPCompareMemory(&TmpInfo->Uuid[0], zeros16, 16) == 0) ||
			   (RTMPCompareMemory(&ApUuidBssid->Uuid[0], zeros16, 16) == 0)) {
		if ((RTMPCompareMemory(&ApUuidBssid->Bssid[0], &TmpInfo->Bssid[0], 5) == 0) &&
			(ApUuidBssid->Band != TmpInfo->Band)) {
			INT tmpDiff = (INT)(ApUuidBssid->Bssid[5] - TmpInfo->Bssid[5]);

			/*
			*	Zero UUID and Non-zero UUID at different band, and two BSSIDs are very close.
			*	Indicate concurrent AP, we can indicate 1 AP only.
			*/
			if ((tmpDiff <= 4) ||
				(tmpDiff >= -4)) {
				*bSameAP = TRUE;
			}
		}
	}
}

BOOLEAN WscBssWpsIESearchForPBC(
	RTMP_ADAPTER *pAd,
	WSC_CTRL *pWscControl,
	BSS_ENTRY *pInBss,
	UUID_BSSID_CH_INFO ApUuidBssid[],
	INT VarIeLen,
	PUCHAR pVar)
{
	INT j = 0, Len = 0, idx = 0;
	BOOLEAN bFound, bSameAP, bSelReg;
	PUCHAR pData = NULL;
	PBEACON_EID_STRUCT pEid;
	USHORT DevicePasswordID;
	PWSC_IE pWscIE;
	UUID_BSSID_CH_INFO TmpInfo;
	UCHAR zeros16[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	BOOLEAN ret = FALSE;
#ifdef CON_WPS
	UCHAR apcli_idx = 0;
	struct wifi_dev *Reqwdev = NULL;
	UCHAR dev_band = 0;
#endif /*CON_WPS Dung_Ru*/

	pData   = pVar;
	bFound  = FALSE;
	bSameAP = FALSE;
	bSelReg = FALSE;
	Len = VarIeLen;
	NdisZeroMemory(&TmpInfo, sizeof(UUID_BSSID_CH_INFO));
#ifdef CON_WPS
	if (pWscControl->EntryIfIdx >= MIN_NET_DEVICE_FOR_APCLI) {
		apcli_idx = (pWscControl->EntryIfIdx & 0x0F);
		Reqwdev = &(pAd->ApCfg.ApCliTab[apcli_idx].wdev);

		if (Reqwdev)
			dev_band = HcGetBandByWdev(Reqwdev);
	}
#endif
	while ((Len > 0) && (bFound == FALSE)) {
		pEid = (PBEACON_EID_STRUCT) pData;

		/* No match, skip the Eid and move forward, IE_WFA_WSC = 0xdd */
		if (pEid->Eid != IE_WFA_WSC) {
			/* Set the offset and look for next IE */
			pData += (pEid->Len + 2);
			Len   -= (pEid->Len + 2);
			continue;
		} else {
			/* Found IE with 0xdd */
			/* check for WSC OUI -- 00 50 f2 04 */
			if ((NdisEqualMemory(pEid->Octet, WPS_OUI, 4) == FALSE)
			   ) {
				/* Set the offset and look for next IE */
				pData += (pEid->Len + 2);
				Len   -= (pEid->Len + 2);
				continue;
			}
		}

		/* 3. Found	AP with WSC IE in beacons, skip 6 bytes = 1 + 1 + 4 */
		pData += 6;
		Len   -= 6;

		/* 4. Start to look the PBC type within WSC VarIE */
		while (Len > 0) {
			/* Check for WSC IEs */
			pWscIE = (PWSC_IE) pData;

			if (be2cpu16(pWscIE->Type) == WSC_ID_SEL_REGISTRAR) {
				hex_dump("SelReg:", pData, 5);
				bSelReg = pWscIE->Data[0];
				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("bSelReg = %d\n", bSelReg));
			}


			/* Check for device password ID, PBC = 0x0004, SMPBC = 0x0006 */
			if (be2cpu16(pWscIE->Type) == WSC_ID_DEVICE_PWD_ID) {
				/* Found device password ID */
#ifdef WINBOND
				/*The Winbond's platform will fail to retrive 2-bytes data, if use the original */
				/*be2cpu16<-- */
				DevicePasswordID = WINBON_GET16((PUCHAR)&pWscIE->Data[0]);
#else
				DevicePasswordID = be2cpu16(get_unaligned((USHORT *)&pWscIE->Data[0]));
				/*DevicePasswordID = be2cpu16(*((USHORT *) &pWscIE->Data[0])); */
#endif /* WINBOND */
				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("WscPBCBssTableSort : DevicePasswordID = 0x%04x\n", DevicePasswordID));

				if (((pWscControl->WscMode == WSC_PBC_MODE) && (DevicePasswordID == DEV_PASS_ID_PBC)) ||
					((pWscControl->WscMode == WSC_SMPBC_MODE) && (DevicePasswordID == DEV_PASS_ID_SMPBC))) {
					/* Found matching PBC AP in current list, add it into table and add the count */
					bFound = TRUE;
					MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("DPID=PBC Found -->\n"));
					MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("#  Bssid %02x:%02x:%02x:%02x:%02x:%02x\n",
							 pInBss->Bssid[0], pInBss->Bssid[1], pInBss->Bssid[2], pInBss->Bssid[3], pInBss->Bssid[4], pInBss->Bssid[5]));

					if (pInBss->Channel > 14)
						TmpInfo.Band = WSC_RFBAND_50GHZ;
					else
						TmpInfo.Band = WSC_RFBAND_24GHZ;

					RTMPMoveMemory(&TmpInfo.Bssid[0], &pInBss->Bssid[0], MAC_ADDR_LEN);
					TmpInfo.Channel = pInBss->Channel;
					RTMPZeroMemory(&TmpInfo.Ssid[0], MAX_LEN_OF_SSID);
					RTMPMoveMemory(&TmpInfo.Ssid[0], &pInBss->Ssid[0], pInBss->SsidLen);
					TmpInfo.SsidLen = pInBss->SsidLen;
				}
			}

			/* UUID_E is optional for beacons, but mandatory for probe-request */
			if (be2cpu16(pWscIE->Type) == WSC_ID_UUID_E) {
				/* Avoid error UUID-E storage from PIN mode */
				RTMPMoveMemory(&TmpInfo.Uuid[0], (UCHAR *)(pData + 4), 16);
			}

			/* Set the offset and look for PBC information */
			/* Since Type and Length are both short type, we need to offset 4, not 2 */
			pData += (be2cpu16(pWscIE->Length) + 4);
			Len   -= (be2cpu16(pWscIE->Length) + 4);
		}
#ifdef CON_WPS
#ifdef CONFIG_MAP_SUPPORT
		if (Reqwdev && IS_MAP_TURNKEY_ENABLE(pAd)
			&& (MAPRcGetBandIdxByChannelCheck(pAd->hdev_ctrl, pInBss->Channel) != HcGetBandByWdev(Reqwdev))) {
			if (bFound == TRUE)
				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR,
				("FIND AP TmpInfo.Band =%d ssid=%s but skip(band=%d wdev band=%d)\n",
				TmpInfo.Band, TmpInfo.Ssid,
				MAPRcGetBandIdxByChannelCheck(pAd->hdev_ctrl, pInBss->Channel),
				HcGetBandByWdev(Reqwdev)));
			bFound = FALSE;
		}
#endif
		if (pAd->ApCfg.ConWpsApCliDisabled == FALSE) {
			if ((pAd->ApCfg.ApCliTab[apcli_idx].wdev.WscControl.conWscStatus & CON_WPS_STATUS_APCLI_RUNNING)
				&& (pAd->ApCfg.ApCliTab[apcli_idx ^ 1].wdev.WscControl.conWscStatus & CON_WPS_STATUS_APCLI_RUNNING)
				&& (pAd->ApCfg.ApCliTab[apcli_idx].ConWpsApCliModeScanDoneStatus != CON_WPS_APCLI_SCANDONE_STATUS_ONGOING
					&& pAd->ApCfg.ApCliTab[apcli_idx ^ 1].ConWpsApCliModeScanDoneStatus != CON_WPS_APCLI_SCANDONE_STATUS_ONGOING)) {
				if (Reqwdev && MAPRcGetBandIdxByChannelCheck(pAd->hdev_ctrl, pInBss->Channel) != HcGetBandByWdev(Reqwdev)) {
					if (bFound == TRUE)
						MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE,
						("FIND AP TmpInfo.Band =%d ssid=%s but skip(band=%d wdev band=%d)\n",
						TmpInfo.Band, TmpInfo.Ssid,
						MAPRcGetBandIdxByChannelCheck(pAd->hdev_ctrl, pInBss->Channel),
						HcGetBandByWdev(Reqwdev)));
					bFound = FALSE;
				}
			}
		}
#endif /*CON_WPS*/
		if ((bFound == TRUE) && (bSelReg == TRUE)) {
			if (pWscControl->WscPBCBssCount == 8)
				break;

			if (pWscControl->WscPBCBssCount > 0) {
				for (j = 0; j < pWscControl->WscPBCBssCount; j++) {
					WscCheckIsSameAP(&ApUuidBssid[j], &TmpInfo, zeros16, &bSameAP);
					if (bSameAP)
						break;
				}
			}

			if (bSameAP) {
				if ((pWscControl->WpsApBand == PREFERRED_WPS_AP_PHY_TYPE_2DOT4_G_FIRST) &&
					(TmpInfo.Band == WSC_RFBAND_24GHZ) &&
					(ApUuidBssid[j].Band != TmpInfo.Band)) {
					RTMPMoveMemory(&(ApUuidBssid[j].Bssid[0]), &TmpInfo.Bssid[0], MAC_ADDR_LEN);
					RTMPZeroMemory(&(ApUuidBssid[j].Ssid[0]), MAX_LEN_OF_SSID);
					RTMPMoveMemory(&(ApUuidBssid[j].Ssid[0]), &TmpInfo.Ssid[0], TmpInfo.SsidLen);
					ApUuidBssid[j].SsidLen = TmpInfo.SsidLen;
					ApUuidBssid[j].Channel = TmpInfo.Channel;
				} else if ((pWscControl->WpsApBand == PREFERRED_WPS_AP_PHY_TYPE_5_G_FIRST) &&
						   (TmpInfo.Band == WSC_RFBAND_50GHZ) &&
						   (ApUuidBssid[j].Band != TmpInfo.Band)) {
					RTMPMoveMemory(&(ApUuidBssid[j].Bssid[0]), &TmpInfo.Bssid[0], MAC_ADDR_LEN);
					RTMPZeroMemory(&(ApUuidBssid[j].Ssid[0]), MAX_LEN_OF_SSID);
					RTMPMoveMemory(&(ApUuidBssid[j].Ssid[0]), &TmpInfo.Ssid[0], TmpInfo.SsidLen);
					ApUuidBssid[j].SsidLen = TmpInfo.SsidLen;
					ApUuidBssid[j].Channel = TmpInfo.Channel;
				}
			}

			if (bSameAP == FALSE) {
				UCHAR index = pWscControl->WscPBCBssCount;
				/* Store UUID */
				RTMPMoveMemory(&(ApUuidBssid[index].Uuid[0]), &TmpInfo.Uuid[0], 16);
				RTMPMoveMemory(&(ApUuidBssid[index].Bssid[0]), &pInBss->Bssid[0], MAC_ADDR_LEN);
				RTMPZeroMemory(&(ApUuidBssid[index].Ssid[0]), MAX_LEN_OF_SSID);
				RTMPMoveMemory(&(ApUuidBssid[index].Ssid[0]), &pInBss->Ssid[0], pInBss->SsidLen);
				ApUuidBssid[index].SsidLen = pInBss->SsidLen;
				ApUuidBssid[index].Channel = pInBss->Channel;

				if (ApUuidBssid[index].Channel > 14)
					ApUuidBssid[index].Band = WSC_RFBAND_50GHZ;
				else
					ApUuidBssid[index].Band = WSC_RFBAND_24GHZ;

				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, ("UUID-E= "));

				for (idx = 0; idx < 16; idx++)
					MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, ("%02x  ", ApUuidBssid[index].Uuid[idx]));

				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, (" SSID: %s, CH: %d ", ApUuidBssid[index].Ssid, ApUuidBssid[index].Channel));

				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, ("\n"));
				pWscControl->WscPBCBssCount++;
			}
		}
	}
	ret = (bFound && bSelReg);
	return ret;
}

#ifdef WSC_STA_SUPPORT
BOOLEAN WscBssWpsIESearchForPIN(
	RTMP_ADAPTER *pAd,
	WSC_CTRL *pWscControl,
	BSS_ENTRY *pInBss,
	UUID_BSSID_CH_INFO ApUuidBssid[],
	INT VarIeLen,
	PUCHAR pVar)
{
	INT j = 0, Len = 0, idx = 0;
	BOOLEAN bFound, bSameAP, bSelReg, bSelWpsPIN1;
	PUCHAR pData = NULL;
	PBEACON_EID_STRUCT pEid;
	USHORT DevicePasswordID;
	PWSC_IE pWscIE;
	UUID_BSSID_CH_INFO TmpInfo;
	UCHAR zeros16[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	BOOLEAN ret = FALSE;
	pData   = pVar;
	bFound  = FALSE;
	bSameAP = FALSE;
	bSelReg = FALSE;
	bSelWpsPIN1 = FALSE;
	Len = VarIeLen;
	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("%s\n", __func__));
	NdisZeroMemory(&TmpInfo, sizeof(UUID_BSSID_CH_INFO));

	while ((Len > 0) && (bFound == FALSE)) {
		pEid = (PBEACON_EID_STRUCT) pData;

		/* No match, skip the Eid and move forward, IE_WFA_WSC = 0xdd */
		if (pEid->Eid != IE_WFA_WSC) {
			/* Set the offset and look for next IE */
			pData += (pEid->Len + 2);
			Len   -= (pEid->Len + 2);
			continue;
		} else {
			/* Found IE with 0xdd */
			/* check for WSC OUI -- 00 50 f2 04 */
			if ((NdisEqualMemory(pEid->Octet, WPS_OUI, 4) == FALSE)
			   ) {
				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("%s: wpsIE AP not found\n", __func__));
				/* Set the offset and look for next IE */
				pData += (pEid->Len + 2);
				Len   -= (pEid->Len + 2);
				continue;
			}
		}

		/* 3. Found	AP with WSC IE in beacons, skip 6 bytes = 1 + 1 + 4 */
		pData += 6;
		Len   -= 6;

		if (pWscControl->ScanCountToincludeWPSPin1 > 3)
			bSelWpsPIN1 = TRUE;

		/* 4. Start to look the PIN type within WSC VarIE */
		while (Len > 0) {
			/* Check for WSC IEs */
			pWscIE = (PWSC_IE) pData;

			if (be2cpu16(pWscIE->Type) == WSC_ID_SEL_REGISTRAR) {
				hex_dump("SelReg:", pData, 5);
				bSelReg = pWscIE->Data[0];
				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("bSelReg = %d\n", bSelReg));
			}


			/* Check for device password ID, PIN = 0x0000 */
			if (be2cpu16(pWscIE->Type) == WSC_ID_DEVICE_PWD_ID) {
				/* Found device password ID */
#ifdef WINBOND
				/*The Winbond's platform will fail to retrive 2-bytes data, if use the original */
				/*be2cpu16<-- */
				DevicePasswordID = WINBON_GET16((PUCHAR)&pWscIE->Data[0]);
#else
				DevicePasswordID = be2cpu16(get_unaligned((USHORT *)&pWscIE->Data[0]));
				/*DevicePasswordID = be2cpu16(*((USHORT *) &pWscIE->Data[0])); */
#endif /* WINBOND */
				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("WscPINBssTableSort : DevicePasswordID = 0x%04x\n", DevicePasswordID));

				if ((pWscControl->WscMode == WSC_PIN_MODE) && (DevicePasswordID == DEV_PASS_ID_PIN)) {
					/* Found matching PIN AP in current list, add it into table and add the count */
					bFound = TRUE;
					MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("DPID=PIN Found -->\n"));
					MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("#  Bssid %02x:%02x:%02x:%02x:%02x:%02x\n",
							 pInBss->Bssid[0], pInBss->Bssid[1], pInBss->Bssid[2], pInBss->Bssid[3], pInBss->Bssid[4], pInBss->Bssid[5]));

					if (pInBss->Channel > 14)
						TmpInfo.Band = WSC_RFBAND_50GHZ;
					else
						TmpInfo.Band = WSC_RFBAND_24GHZ;

					RTMPMoveMemory(&TmpInfo.Bssid[0], &pInBss->Bssid[0], MAC_ADDR_LEN);
					TmpInfo.Channel = pInBss->Channel;
					RTMPZeroMemory(&TmpInfo.Ssid[0], MAX_LEN_OF_SSID);
					RTMPMoveMemory(&TmpInfo.Ssid[0], &pInBss->Ssid[0], pInBss->SsidLen);
					TmpInfo.SsidLen = pInBss->SsidLen;
				}
			}

			/* UUID_E is optional for beacons, but mandatory for probe-request */
			if (be2cpu16(pWscIE->Type) == WSC_ID_UUID_E) {
				/* Avoid error UUID-E storage from PIN mode */
				RTMPMoveMemory(&TmpInfo.Uuid[0], (UCHAR *)(pData + 4), 16);
			}

			/* Set the offset and look for PIN information */
			/* Since Type and Length are both short type, we need to offset 4, not 2 */
			pData += (be2cpu16(pWscIE->Length) + 4);
			Len   -= (be2cpu16(pWscIE->Length) + 4);
		}


		if ((bFound == TRUE) && (bSelReg == TRUE)) {
			if (pWscControl->WscPINBssCount == 8)
				break;

			if (pWscControl->WscPINBssCount > 0) {
				for (j = 0; j < pWscControl->WscPINBssCount; j++) {
					WscCheckIsSameAP(&ApUuidBssid[j], &TmpInfo, zeros16, &bSameAP);
					if (bSameAP)
						break;
				}
			}

			if (bSameAP) {
				if ((pWscControl->WpsApBand == PREFERRED_WPS_AP_PHY_TYPE_2DOT4_G_FIRST) &&
					(TmpInfo.Band == WSC_RFBAND_24GHZ) &&
					(ApUuidBssid[j].Band != TmpInfo.Band)) {
					RTMPMoveMemory(&(ApUuidBssid[j].Bssid[0]), &TmpInfo.Bssid[0], MAC_ADDR_LEN);
					RTMPZeroMemory(&(ApUuidBssid[j].Ssid[0]), MAX_LEN_OF_SSID);
					RTMPMoveMemory(&(ApUuidBssid[j].Ssid[0]), &TmpInfo.Ssid[0], TmpInfo.SsidLen);
					ApUuidBssid[j].SsidLen = TmpInfo.SsidLen;
					ApUuidBssid[j].Channel = TmpInfo.Channel;
				} else if ((pWscControl->WpsApBand == PREFERRED_WPS_AP_PHY_TYPE_5_G_FIRST) &&
						   (TmpInfo.Band == WSC_RFBAND_50GHZ) &&
						   (ApUuidBssid[j].Band != TmpInfo.Band)) {
					RTMPMoveMemory(&(ApUuidBssid[j].Bssid[0]), &TmpInfo.Bssid[0], MAC_ADDR_LEN);
					RTMPZeroMemory(&(ApUuidBssid[j].Ssid[0]), MAX_LEN_OF_SSID);
					RTMPMoveMemory(&(ApUuidBssid[j].Ssid[0]), &TmpInfo.Ssid[0], TmpInfo.SsidLen);
					ApUuidBssid[j].SsidLen = TmpInfo.SsidLen;
					ApUuidBssid[j].Channel = TmpInfo.Channel;
				}
			}

			if (bSameAP == FALSE) {
				UCHAR index = pWscControl->WscPINBssCount;
				/* Store UUID */
				RTMPMoveMemory(&(ApUuidBssid[index].Uuid[0]), &TmpInfo.Uuid[0], 16);
				RTMPMoveMemory(&(ApUuidBssid[index].Bssid[0]), &pInBss->Bssid[0], MAC_ADDR_LEN);
				RTMPZeroMemory(&(ApUuidBssid[index].Ssid[0]), MAX_LEN_OF_SSID);
				RTMPMoveMemory(&(ApUuidBssid[index].Ssid[0]), &pInBss->Ssid[0], pInBss->SsidLen);
				ApUuidBssid[index].SsidLen = pInBss->SsidLen;
				ApUuidBssid[index].Channel = pInBss->Channel;

				if (ApUuidBssid[index].Channel > 14)
					ApUuidBssid[index].Band = WSC_RFBAND_50GHZ;
				else
					ApUuidBssid[index].Band = WSC_RFBAND_24GHZ;

				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, ("UUID-E= "));

				for (idx = 0; idx < 16; idx++)
					MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, ("%02x  ", ApUuidBssid[index].Uuid[idx]));

				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, ("\n"));
				pWscControl->WscPINBssCount++;
			}
		} else if (bSelWpsPIN1 == TRUE) {
			UCHAR index = pWscControl->WscPINBssCount;
			/* Store UUID */
			RTMPMoveMemory(&(ApUuidBssid[index].Uuid[0]), &TmpInfo.Uuid[0], 16);
			RTMPMoveMemory(&(ApUuidBssid[index].Bssid[0]), &pInBss->Bssid[0], MAC_ADDR_LEN);
			RTMPZeroMemory(&(ApUuidBssid[index].Ssid[0]), MAX_LEN_OF_SSID);
			RTMPMoveMemory(&(ApUuidBssid[index].Ssid[0]), &pInBss->Ssid[0], pInBss->SsidLen);
			ApUuidBssid[index].SsidLen = pInBss->SsidLen;
			ApUuidBssid[index].Channel = pInBss->Channel;

			if (ApUuidBssid[index].Channel > 14)
				ApUuidBssid[index].Band = WSC_RFBAND_50GHZ;
			else
				ApUuidBssid[index].Band = WSC_RFBAND_24GHZ;

			pWscControl->WscPINBssCount++;
		}
	}
	ret = ((bFound && bSelReg) || (bSelWpsPIN1));
	return ret;
}
#endif

/*
*	========================================================================
*
*	Routine Description:
*		Find WSC PBC activated AP list
*
*	Arguments:
*		pAd         - NIC Adapter pointer
*		OutTab		- Qualified AP BSS table
*
*	Return Value:
*		None
*
*	IRQL = DISPATCH_LEVEL
*
*	Note:
*		All these constants are defined in wsc.h
*
*	========================================================================
*/
VOID WscPBCBssTableSort(
	IN RTMP_ADAPTER *pAd,
	IN WSC_CTRL *pWscControl)
{
	INT i;
	BSS_ENTRY *pInBss;
	UUID_BSSID_CH_INFO *ApUuidBssid = NULL;
	BOOLEAN rv = FALSE;
	UCHAR wdevBand = 0;
	struct wifi_dev *wdev;
#if defined(CONFIG_STA_SUPPORT) || defined(APCLI_SUPPORT)
	UCHAR CurOpMode = AP_MODE;
#endif /* CONFIG_STA_SUPPORT || APCLI_SUPPORT */
#ifdef APCLI_SUPPORT
	PAPCLI_STRUCT apcli;
	UCHAR apcli_idx = 0;
#endif /* APCLI_SUPPORT */
#ifdef APCLI_SUPPORT
	if (CurOpMode == AP_MODE)
		apcli_idx = (pWscControl->EntryIfIdx & 0x0F);
	apcli = &pAd->ApCfg.ApCliTab[apcli_idx];
#endif /* APCLI_SUPPORT */
	wdev = (struct wifi_dev *)pWscControl->wdev;
#ifdef APCLI_SUPPORT
	/**
	 * wdev could be null in concurrent WPS case for opposite interface
	 * get wdev from apcli interface in that case.
	 */
	if (!wdev)
		wdev = &(pAd->ApCfg.ApCliTab[apcli_idx].wdev);
#endif /* APCLI_SUPPORT */
	wdevBand = HcGetBandByWdev(wdev);
	/* allocate memory */
	os_alloc_mem(NULL, (UCHAR **)&ApUuidBssid, sizeof(UUID_BSSID_CH_INFO) * 8);

	if (ApUuidBssid == NULL) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, ("%s: Allocate memory fail!!!\n", __func__));
		return;
	}

	NdisZeroMemory(&ApUuidBssid[0], sizeof(UUID_BSSID_CH_INFO));
	pWscControl->WscPBCBssCount = 0;

	for (i = 0; i < pAd->ScanTab.BssNr; i++) {
		/* BSS entry for VarIE processing */
		pInBss  = (BSS_ENTRY *) &pAd->ScanTab.BssEntry[i];

		/* 1. Check VarIE length */
		if (pInBss->VarIELen == 0)
			continue;

#ifndef APCLI_SUPPORT
		if (wdevBand != MAPRcGetBandIdxByChannelCheck(pAd->hdev_ctrl, pInBss->Channel))
			continue;
#endif

		/* 2. Search for WSC IE - 0xdd xx 00 50 f2 04 */
		rv = WscBssWpsIESearchForPBC(pAd,
									 pWscControl,
									 pInBss,
									 ApUuidBssid,
									 pInBss->VarIELen,
									 pInBss->VarIEs);

		if (rv == FALSE) {
			WscBssWpsIESearchForPBC(pAd,
									pWscControl,
									pInBss,
									ApUuidBssid,
									pInBss->VarIeFromProbeRspLen,
									pInBss->pVarIeFromProbRsp);
		}
	}

	if (pWscControl->WscPBCBssCount == 1) {
		RTMPZeroMemory(&pWscControl->WscSsid, sizeof(NDIS_802_11_SSID));
		RTMPMoveMemory(pWscControl->WscSsid.Ssid, ApUuidBssid[0].Ssid, ApUuidBssid[0].SsidLen);
		pWscControl->WscSsid.SsidLength = ApUuidBssid[0].SsidLen;
		RTMPZeroMemory(pWscControl->WscBssid, MAC_ADDR_LEN);
		RTMPMoveMemory(pWscControl->WscBssid, ApUuidBssid[0].Bssid, MAC_ADDR_LEN);
		RTMPMoveMemory(pWscControl->WscPeerUuid, ApUuidBssid[0].Uuid, sizeof(ApUuidBssid[0].Uuid));
#ifdef APCLI_SUPPORT
		if (CurOpMode == AP_MODE) {
			apcli->MlmeAux.Channel = ApUuidBssid[0].Channel;
			COPY_MAC_ADDR(apcli->CfgApCliBssid, pWscControl->WscBssid);
		}
		printk("will connect %s (%02X:%02X:%02X:%02X:%02X:%02X) on %d\n", pWscControl->WscSsid.Ssid, PRINT_MAC(pWscControl->WscBssid), ApUuidBssid[0].Channel);

#endif /* APCLI_SUPPORT */
	}

	if (ApUuidBssid != NULL)
		os_free_mem(ApUuidBssid);

		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_OFF, ("WscPBCBssTableSort : Total %d PBC Registrar Found\n", pWscControl->WscPBCBssCount));
}

#ifdef WSC_STA_SUPPORT
/*
*	========================================================================
*
*	Routine Description:
*		Find WSC PIN activated AP list
*
*	Arguments:
*		pAd         - NIC Adapter pointer
*		OutTab		- Qualified AP BSS table
*
*	Return Value:
*		None
*
*	IRQL = DISPATCH_LEVEL
*
*	Note:
*		All these constants are defined in wsc.h
*
*	========================================================================
*/
VOID WscPINBssTableSort(
	IN RTMP_ADAPTER *pAd,
	IN WSC_CTRL *pWscControl)
{
	INT i;
	BSS_ENTRY *pInBss;
	UUID_BSSID_CH_INFO *ApUuidBssid = NULL;
	BOOLEAN rv = FALSE;
	struct wifi_dev *wdev;

	wdev = (struct wifi_dev *)pWscControl->wdev;

	if (pWscControl == NULL)
		return;

	pWscControl->ScanCountToincludeWPSPin1++;
	/* allocate memory */
	os_alloc_mem(NULL, (UCHAR **)&ApUuidBssid, sizeof(UUID_BSSID_CH_INFO) * 8);

	if (ApUuidBssid == NULL) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, ("%s: Allocate memory fail!!!\n", __func__));
		return;
	}

	NdisZeroMemory(&ApUuidBssid[0], sizeof(UUID_BSSID_CH_INFO));
	pWscControl->WscPINBssCount = 0;
	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("%s: scan result AP Count:%u\n", __func__, pAd->ScanTab.BssNr));

	for (i = 0; i < pAd->ScanTab.BssNr; i++) {
		/* BSS entry for VarIE processing */
		pInBss  = (BSS_ENTRY *) &pAd->ScanTab.BssEntry[i];

		/*0. compare UI supplied AP mac addr with scan list entry, if not same continue */
		if (RTMPCompareMemory(pWscControl->WscBssid, pInBss->Bssid, 6))
			continue;

		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("#  wsc Bssid %02x:%02x:%02x:%02x:%02x:%02x\n",
				 pWscControl->WscBssid[0], pWscControl->WscBssid[1], pWscControl->WscBssid[2], pWscControl->WscBssid[3], pWscControl->WscBssid[4], pWscControl->WscBssid[5]));
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("# pIn Bssid %02x:%02x:%02x:%02x:%02x:%02x\n",
				 pInBss->Bssid[0], pInBss->Bssid[1], pInBss->Bssid[2], pInBss->Bssid[3], pInBss->Bssid[4], pInBss->Bssid[5]));

		/* 1. Check VarIE length */
		if (pInBss->VarIELen == 0)
			continue;

		/* 2. Search for WSC IE - 0xdd xx 00 50 f2 04 */
		rv = WscBssWpsIESearchForPIN(pAd,
									 pWscControl,
									 pInBss,
									 ApUuidBssid,
									 pInBss->VarIELen,
									 pInBss->VarIEs);

		if (rv == FALSE) {
			WscBssWpsIESearchForPIN(pAd,
									pWscControl,
									pInBss,
									ApUuidBssid,
									pInBss->VarIeFromProbeRspLen,
									pInBss->pVarIeFromProbRsp);
		}
	}

	if (pWscControl->WscPINBssCount == 1) {
		RTMPZeroMemory(&pWscControl->WscSsid, sizeof(NDIS_802_11_SSID));
		RTMPMoveMemory(pWscControl->WscSsid.Ssid, ApUuidBssid[0].Ssid, ApUuidBssid[0].SsidLen);
		pWscControl->WscSsid.SsidLength = ApUuidBssid[0].SsidLen;
		RTMPZeroMemory(pWscControl->WscBssid, MAC_ADDR_LEN);
		RTMPMoveMemory(pWscControl->WscBssid, ApUuidBssid[0].Bssid, MAC_ADDR_LEN);
	}

	if (ApUuidBssid != NULL)
		os_free_mem(ApUuidBssid);

		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_OFF, ("WscPINBssTableSort : Total %d PIN Registrar Found\n", pWscControl->WscPINBssCount));
}
#endif

VOID WscGenRandomKey(
	IN  PRTMP_ADAPTER pAd,
	IN  PWSC_CTRL pWscControl,
	INOUT PUCHAR pKey,
	INOUT PUSHORT pKeyLen)
{
	UCHAR tempRandomByte = 0;
	UCHAR idx = 0;
	UCHAR keylen = 0;
	UCHAR retry = 0;

	NdisZeroMemory(pKey, 64);

	/*
	*	Hex Key 64 digital
	*/
	if (pWscControl->WscKeyASCII == 0) {
		UCHAR	tmpStrB[3];

		for (idx = 0; idx < 32; idx++) {
			NdisZeroMemory(&tmpStrB[0], sizeof(tmpStrB));
			tempRandomByte = RandomByte(pAd);
			snprintf((RTMP_STRING *) &tmpStrB[0], 3, "%02x", tempRandomByte);
			NdisMoveMemory(pKey + (idx * 2), &tmpStrB[0], 2);
		}

		*pKeyLen = 64;
	} else {
		/*
		*	ASCII Key, random length
		*/
		if (pWscControl->WscKeyASCII == 1) {
			do {
				keylen = RandomByte(pAd);
				keylen = keylen % 64;

				if (retry++ > 20)
					keylen = 8;
			} while (keylen < 8);
		} else
			keylen = pWscControl->WscKeyASCII;

		/*
		*	Generate printable ASCII (decimal 33 to 126)
		*/
		for (idx = 0; idx < keylen; idx++) {
			tempRandomByte = RandomByte(pAd) % 94 + 33;
			*(pKey + idx) = tempRandomByte;
		}

		*pKeyLen = keylen;
	}
}

VOID WscCreateProfileFromCfg(
	IN  PRTMP_ADAPTER pAd,
	IN  UCHAR OpMode,
	IN  PWSC_CTRL pWscControl,
	OUT PWSC_PROFILE pWscProfile)
{
#ifdef CONFIG_AP_SUPPORT
	UCHAR apidx = (pWscControl->EntryIfIdx & 0x0F);
#endif /* CONFIG_AP_SUPPORT */
	USHORT authType = 0, encyType = 0;
	UCHAR WepKeyId = 0;
	PWSC_CREDENTIAL pCredential = NULL;
	UCHAR CurOpMode = AP_MODE;

#ifdef CONFIG_AP_SUPPORT

	if (CurOpMode == AP_MODE) {
		if ((OpMode & 0x0F) == AP_MODE) {
			/*
			*	AP needs to choose the STA's authType and encyType in two cases.
			*	1. AP is unconfigurated (authType and encyType will be updated to mixed mode by WscWriteConfToPortCfg() )
			*	2. AP's authType is mixed mode, we should choose the suitable authType and encyType to STA
			*	STA's authType and encyType depend on WscSecurityMode flag
			*/
			if (((pWscControl->WscConfStatus == WSC_SCSTATE_UNCONFIGURED) ||
				 (IS_AKM_FT_WPAPSKWPA2PSK_Entry(&pAd->ApCfg.MBSSID[apidx].wdev))) &&
				(OpMode & REGISTRAR_ACTION)) {
				switch (pAd->ApCfg.MBSSID[apidx].wdev.WscSecurityMode) {
				case WPAPSKTKIP:
					authType = WSC_AUTHTYPE_WPAPSK;
					encyType = WSC_ENCRTYPE_TKIP;
					break;

				case WPAPSKAES:
					authType = WSC_AUTHTYPE_WPAPSK;
					encyType = WSC_ENCRTYPE_AES;
					break;

				case WPA2PSKTKIP:
					authType = WSC_AUTHTYPE_WPA2PSK;
					encyType = WSC_ENCRTYPE_TKIP;
					break;

				case WPA2PSKAES:
					authType = WSC_AUTHTYPE_WPA2PSK;
					encyType = WSC_ENCRTYPE_AES;
					break;

				default:
					authType = (WSC_AUTHTYPE_WPAPSK | WSC_AUTHTYPE_WPA2PSK);
					encyType = (WSC_ENCRTYPE_TKIP | WSC_ENCRTYPE_AES);
					break;
				}

				if (pWscControl->WscConfStatus == WSC_SCSTATE_CONFIGURED) {
					/*
					*	Although AuthMode is mixed mode, cipher maybe not mixed mode.
					*	We need to correct cipher here.
					*/
					if (IS_CIPHER_TKIP_Entry(&pAd->ApCfg.MBSSID[apidx].wdev))
						encyType = WSC_ENCRTYPE_TKIP;

					if (IS_CIPHER_AES_Entry(&pAd->ApCfg.MBSSID[apidx].wdev))
						encyType |= WSC_ENCRTYPE_AES;	/*mixmode case, bitmap*/
				}
			} else {
				authType = WscGetAuthType(pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.AKMMap);
				encyType = WscGetEncryType(pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.PairwiseCipher);
			}

			WepKeyId = pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.GroupKeyId;
		}

#ifdef APCLI_SUPPORT
		else if (OpMode == AP_CLIENT_MODE) {
			apidx = apidx & 0x0F;

			if (apidx < MAX_APCLI_NUM) {
				authType = WscGetAuthType(pAd->ApCfg.ApCliTab[apidx].wdev.SecConfig.AKMMap);
				encyType = WscGetEncryType(pAd->ApCfg.ApCliTab[apidx].wdev.SecConfig.PairwiseCipher);
				WepKeyId = pAd->ApCfg.ApCliTab[apidx].wdev.SecConfig.GroupKeyId;
			}
		}

#endif /* APCLI_SUPPORT */
	}

#endif /* CONFIG_AP_SUPPORT */
	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("-----> WscGetDefaultProfileForM8\n"));
	pCredential = &pWscProfile->Profile[0]; /*Only support one credential now. 20070515 */
	NdisZeroMemory(pCredential, sizeof(WSC_CREDENTIAL));
	pWscProfile->ProfileCnt = 1;
	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("%s:: pWscControl->WscConfStatus  = %d, OpMode = %d\n",
			 __func__, pWscControl->WscConfStatus, OpMode));

	/* NewKey, NewKeyIndex for M8 */
	if ((pWscControl->WscConfStatus == WSC_SCSTATE_UNCONFIGURED) &&
		((((OpMode & 0x0F) == AP_MODE)
		 ) && (OpMode & REGISTRAR_ACTION))) {
		pCredential->KeyIndex = 1;

		if ((OpMode & 0x0F) == STA_MODE) {
			{
				WscGenRandomKey(pAd, pWscControl, pCredential->Key, &pCredential->KeyLength);
				authType = WSC_AUTHTYPE_WPA2PSK;
				encyType = WSC_ENCRTYPE_AES;
			}
		} else
			WscGenRandomKey(pAd, pWscControl, pCredential->Key, &pCredential->KeyLength);
	} else {
		struct _SECURITY_CONFIG *pSecConfig = NULL;

		pCredential->KeyIndex = 1;
		pCredential->KeyLength = 0;
		NdisZeroMemory(pCredential->Key, 64);

		switch (encyType) {
		case WSC_ENCRTYPE_NONE:
			break;

		case WSC_ENCRTYPE_WEP:
#ifdef CONFIG_AP_SUPPORT

			if (CurOpMode == AP_MODE)
				pSecConfig = &pAd->ApCfg.MBSSID[apidx].wdev.SecConfig;

#ifdef APCLI_SUPPORT

			if (OpMode == AP_CLIENT_MODE) {
				if (apidx < MAX_APCLI_NUM)
					pSecConfig = &pAd->ApCfg.ApCliTab[apidx].wdev.SecConfig;
			}

#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
			pCredential->KeyIndex = (WepKeyId + 1);

			if (pSecConfig && pSecConfig->WepKey[WepKeyId].KeyLen) {
				INT i;

				for (i = 0; i < pSecConfig->WepKey[WepKeyId].KeyLen; i++)
					snprintf((RTMP_STRING *) pCredential->Key, 64, "%s%02x", pCredential->Key, pSecConfig->WepKey[WepKeyId].Key[i]);

				pCredential->KeyLength = pSecConfig->WepKey[WepKeyId].KeyLen * 2;
			}

			break;

		case WSC_ENCRTYPE_TKIP:
		case WSC_ENCRTYPE_AES:
		case (WSC_ENCRTYPE_AES | WSC_ENCRTYPE_TKIP):
			pCredential->KeyLength = pWscControl->WpaPskLen;
			memcpy(pCredential->Key,
				   pWscControl->WpaPsk,
				   pWscControl->WpaPskLen);
			break;
		}
	}

	pCredential->AuthType = authType;
	pCredential->EncrType = encyType;
#ifdef CONFIG_AP_SUPPORT

	if (CurOpMode == AP_MODE) {
		if ((OpMode & 0x0F) == AP_MODE) {
			NdisMoveMemory(pCredential->MacAddr, pAd->ApCfg.MBSSID[apidx].wdev.bssid, 6);

			if ((pWscControl->WscConfStatus == WSC_SCSTATE_UNCONFIGURED) &&
				(pWscControl->WscDefaultSsid.SsidLength > 0) &&
				(pWscControl->WscDefaultSsid.SsidLength < 33)) {
				NdisMoveMemory(pCredential->SSID.Ssid, pWscControl->WscDefaultSsid.Ssid, pWscControl->WscDefaultSsid.SsidLength);
				pCredential->SSID.SsidLength = pWscControl->WscDefaultSsid.SsidLength;
			} else {
				NdisMoveMemory(pCredential->SSID.Ssid, pAd->ApCfg.MBSSID[apidx].Ssid, pAd->ApCfg.MBSSID[apidx].SsidLen);
				pCredential->SSID.SsidLength = pAd->ApCfg.MBSSID[apidx].SsidLen;
			}
		}

#ifdef APCLI_SUPPORT
		else if (OpMode == AP_CLIENT_MODE) {
			if (apidx < MAX_APCLI_NUM) {
				NdisMoveMemory(pCredential->MacAddr, APCLI_ROOT_BSSID_GET(pAd, pAd->ApCfg.ApCliTab[apidx].MacTabWCID), 6);
				NdisMoveMemory(pCredential->SSID.Ssid, pAd->ApCfg.ApCliTab[apidx].Ssid, pAd->ApCfg.ApCliTab[apidx].SsidLen);
				pCredential->SSID.SsidLength = pAd->ApCfg.ApCliTab[apidx].SsidLen;
			}
		}

#endif /* APCLI_SUPPORT */
	}

#endif /* CONFIG_AP_SUPPORT */
#ifdef WSC_V2_SUPPORT

	if (pWscControl->WscV2Info.bEnableWpsV2 && (OpMode & REGISTRAR_ACTION))
		NdisMoveMemory(pCredential->MacAddr, pWscControl->EntryAddr, 6);

#endif /* WSC_V2_SUPPORT */
	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("<----- WscCreateProfileFromCfg\n"));
}

#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
void    WscWriteConfToApCliCfg(
	IN  PRTMP_ADAPTER pAd,
	IN  PWSC_CTRL pWscControl,
	IN  PWSC_CREDENTIAL pCredential,
	IN  BOOLEAN bEnrollee)
{
	UCHAR CurApIdx = (pWscControl->EntryIfIdx & 0x0F);
	APCLI_STRUCT *pApCliTab;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	INT old_if_type = pObj->ioctl_if_type;
	struct _SECURITY_CONFIG *pOldSecConfig;

	pOldSecConfig = pObj->pSecConfig;

	if (CurApIdx >= MAX_APCLI_NUM)
		return;

	pObj->ioctl_if_type = INT_APCLI;
	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("-----> WscWriteConfToApCliCfg (apcli%d)\n", CurApIdx));
	{
		pApCliTab = &pAd->ApCfg.ApCliTab[CurApIdx];
		NdisZeroMemory(pApCliTab->Ssid, MAX_LEN_OF_SSID);
		NdisMoveMemory(pApCliTab->Ssid, pCredential->SSID.Ssid, pCredential->SSID.SsidLength);
		pApCliTab->SsidLen = pCredential->SSID.SsidLength;
		NdisZeroMemory(pApCliTab->CfgSsid, MAX_LEN_OF_SSID);
		NdisMoveMemory(pApCliTab->CfgSsid, pCredential->SSID.Ssid, pCredential->SSID.SsidLength);
		pApCliTab->CfgSsidLen = pCredential->SSID.SsidLength;
		NdisZeroMemory(pApCliTab->CfgApCliBssid, MAC_ADDR_LEN);
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("AuthType: %d, EncrType: %d\n", pCredential->AuthType, pCredential->EncrType));

		if ((pCredential->AuthType == WSC_AUTHTYPE_WPAPSK) ||
			(pCredential->AuthType == WSC_AUTHTYPE_WPA2PSK)) {
			if ((pCredential->EncrType != WSC_ENCRTYPE_TKIP) && (pCredential->EncrType != WSC_ENCRTYPE_AES)) {
				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("AuthType is WPAPSK or WPA2PAK.\n"
						 "Get illegal EncrType(%d) from External Registrar, set EncrType to TKIP\n",
						 pCredential->EncrType));
				pCredential->EncrType = WSC_ENCRTYPE_TKIP;
			}
		}

		pObj->pSecConfig = &pApCliTab->wdev.SecConfig;
		Set_SecAuthMode_Proc(pAd, WscGetAuthTypeStr(pCredential->AuthType));
		Set_SecEncrypType_Proc(pAd, WscGetEncryTypeStr(pCredential->EncrType));

		if (pCredential->EncrType != WSC_ENCRTYPE_NONE) {
			if (pCredential->EncrType & (WSC_ENCRTYPE_TKIP | WSC_ENCRTYPE_AES)) {
				pApCliTab->wdev.SecConfig.PairwiseKeyId = 0;

				if (pCredential->KeyLength >= 8 && pCredential->KeyLength <= 64) {
					pWscControl->WpaPskLen = (INT) pCredential->KeyLength;
					NdisZeroMemory(pWscControl->WpaPsk, 64);
					NdisMoveMemory(pWscControl->WpaPsk, pCredential->Key, pWscControl->WpaPskLen);
					os_zero_mem(pApCliTab->wdev.SecConfig.PSK, LEN_PSK + 1);
					os_move_mem(pApCliTab->wdev.SecConfig.PSK, pCredential->Key, pCredential->KeyLength);
					MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("WpaPskLen = %d\n", pWscControl->WpaPskLen));
				} else {
					pWscControl->WpaPskLen = 0;
					MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("WPAPSK: Invalid Key Length (%d)\n", pCredential->KeyLength));
				}
			} else if (pCredential->EncrType == WSC_ENCRTYPE_WEP) {
				CHAR   WepKeyId = 0;
				USHORT  WepKeyLen = pCredential->KeyLength;

				WepKeyId = (pCredential->KeyIndex - 1); /* KeyIndex = 1 ~ 4 */

				if ((WepKeyId >= 0) && (WepKeyId <= 3)) {
					struct _SECURITY_CONFIG *pSecConfig = NULL;

					pSecConfig = &pApCliTab->wdev.SecConfig;
					pSecConfig->PairwiseKeyId = WepKeyId;

					/* 5 or 13 ASCII characters */
					/* 10 or 26 Hex characters */
					if (WepKeyLen == 5 || WepKeyLen == 13 || WepKeyLen == 10 || WepKeyLen == 26)
						WscSetWepCipher(pSecConfig, pCredential, WepKeyId, WepKeyLen);
					else
						MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE,
						("WEP: Invalid Key Length (%d)\n", pCredential->KeyLength));
				} else {
					MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE,
						("Unsupport default key index (%d), use key Index 1.\n", WepKeyId));
					pApCliTab->wdev.SecConfig.PairwiseKeyId = 0;
				}
			}
		}
	}

	if (pWscControl->WscProfile.ProfileCnt > 1) {
		pWscControl->WscProfileRetryTimerRunning = TRUE;
		RTMPSetTimer(&pWscControl->WscProfileRetryTimer, WSC_PROFILE_RETRY_TIME_OUT);
	}

	pObj->ioctl_if_type = old_if_type;
	pObj->pSecConfig = pOldSecConfig;
	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("<----- WscWriteConfToApCliCfg\n"));
}

VOID WscApCliLinkDownById(
	IN  PRTMP_ADAPTER pAd,
	IN  UCHAR apidx)
{
	BOOLEAN apcliEn;
	PWSC_CTRL pWscControl;

	if (apidx >= MAX_APCLI_NUM)
		return;

	apcliEn = pAd->ApCfg.ApCliTab[apidx].Enable;
	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("-----> WscApCliLinkDownById, apidx=%u, apcliEn=%d\n", apidx, apcliEn));
	pWscControl = &pAd->ApCfg.ApCliTab[apidx].wdev.WscControl;
	NdisMoveMemory(pWscControl->RegData.SelfInfo.MacAddr,
				   pAd->ApCfg.ApCliTab[apidx].wdev.if_addr,
				   6);

	/* bring apcli interface down first */
	if (apcliEn == TRUE) {
		if (apidx < MAX_APCLI_NUM) {
			pAd->ApCfg.ApCliTab[apidx].Enable = FALSE;
			ApCliIfDown(pAd);
		}
	}

	pAd->ApCfg.ApCliTab[apidx].Enable = apcliEn;
	pWscControl->WscStatus = STATUS_WSC_LINK_UP;
	pWscControl->bWscTrigger = TRUE;
}

VOID WscApCliLinkDown(
	IN  PRTMP_ADAPTER pAd,
	IN  PWSC_CTRL pWscControl)
{
	UCHAR apidx = (pWscControl->EntryIfIdx & 0x0F);

	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("-----> WscApCliLinkDown, apidx=%u\n", apidx));

	if (apidx >= MAX_APCLI_NUM)
		return;

	RTEnqueueInternalCmd(pAd, CMDTHREAD_WSC_APCLI_LINK_DOWN, (VOID *)&apidx, sizeof(UCHAR));
}

#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

VOID WpsSmProcess(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem)
{
	INT HeaderLen = LENGTH_802_11;
	PHEADER_802_11 pHeader;
	PMAC_TABLE_ENTRY pEntry = NULL;
	INT apidx = MAIN_MBSSID;
	PWSC_CTRL pWpsCtrl = NULL;
	struct wifi_dev *wdev;
	struct wifi_dev_ops *ops;

	wdev = Elem->wdev;
	ASSERT(wdev);
	ops = wdev->wdev_ops;
	pHeader = (PHEADER_802_11)Elem->Msg;

#ifdef A4_CONN
	if (pHeader->FC.FrDs == 1 && pHeader->FC.ToDs == 1 && Elem->MsgType != WSC_EAPOL_UPNP_MSG)
		HeaderLen = LENGTH_802_11_WITH_ADDR4;
#endif

	HeaderLen += LENGTH_802_1_H + sizeof(IEEE8021X_FRAME) + sizeof(EAP_FRAME);

	if (Elem->MsgType == WSC_EAPOL_PACKET_MSG) {
		ops->mac_entry_lookup(pAd, pHeader->Addr2, wdev, &pEntry);

		if (pEntry)
			apidx = pEntry->func_tb_idx;
		else {
			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR,
					 ("%s:: cannot find this entry(%02x:%02x:%02x:%02x:%02x:%02x)\n",
					  __func__, PRINT_MAC(pHeader->Addr2)));
			return;
		}
	}

	pWpsCtrl = &wdev->WscControl;

	if ((Elem->MsgType == WSC_EAPOL_UPNP_MSG) && (Elem->MsgLen > HeaderLen)) {
		/*The WSC msg from UPnP daemon */
		PUCHAR		pData;
		UCHAR		MacAddr[MAC_ADDR_LEN] = {0};
		/* Skip the (802.11 + 802.1h + 802.1x + EAP) header */
		pData = (PUCHAR) &Elem->Msg[HeaderLen];
		Elem->MsgLen -= HeaderLen;
		/* The Addr1 of UPnP-Msg used to indicate the MAC address of the AP interface. Now always be ra0. */
		NdisMoveMemory(MacAddr, pHeader->Addr1, MAC_ADDR_LEN);
		NdisMoveMemory(Elem->Msg, MacAddr, MAC_ADDR_LEN);
		NdisMoveMemory(Elem->Msg + 6, pData, Elem->MsgLen);
		StateMachinePerformAction(pAd, &pAd->Mlme.WscMachine, Elem, pAd->Mlme.WscMachine.CurrState);
	} else if (Elem->MsgType == WSC_EAPOL_START_MSG)
		StateMachinePerformAction(pAd, &pAd->Mlme.WscMachine, Elem, pAd->Mlme.WscMachine.CurrState);
	else if (pEntry && (Elem->MsgType == WSC_EAPOL_PACKET_MSG)) {
		/* WSC_STATE_MACHINE can service only one station at one time */
		RTMP_STRING *pData;
		PEAP_FRAME  pEapFrame;
		/* Skip the EAP LLC header */
		pData = (RTMP_STRING *) &Elem->Msg[LENGTH_802_11 + LENGTH_802_1_H];
#ifdef A4_CONN
		if (pHeader->FC.FrDs == 1 && pHeader->FC.ToDs == 1)
			pData = (RTMP_STRING *) &Elem->Msg[LENGTH_802_11_WITH_ADDR4 + LENGTH_802_1_H];
#endif
		pEapFrame = (PEAP_FRAME)(pData + sizeof(IEEE8021X_FRAME));
		pData += sizeof(IEEE8021X_FRAME) + sizeof(EAP_FRAME);
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, ("%s::  EAPOL Packet.  Code = %d.    Type = %d\n",
				 __func__, pEapFrame->Code, pEapFrame->Type));

		if (pEapFrame->Code == EAP_CODE_FAIL) {
			/* EAP-Fail */
			RTMP_STRING fail_data[] = "EAP_FAIL";

			NdisMoveMemory(Elem->Msg, pHeader->Addr2, MAC_ADDR_LEN);
			NdisMoveMemory(Elem->Msg + MAC_ADDR_LEN, fail_data, strlen(fail_data));
			Elem->MsgLen = strlen(fail_data);
			StateMachinePerformAction(pAd, &pAd->Mlme.WscMachine, Elem, pAd->Mlme.WscMachine.CurrState);
			return;
		} else if ((pEapFrame->Code == EAP_CODE_REQ) && (pEapFrame->Type == EAP_TYPE_ID)) {
			/* EAP-Req (Identity) */
			RTMP_STRING id_data[] = "hello";

			pWpsCtrl->lastId = pEapFrame->Id;
			NdisMoveMemory(Elem->Msg, pHeader->Addr2, MAC_ADDR_LEN);
			NdisMoveMemory(Elem->Msg + MAC_ADDR_LEN, id_data, strlen(id_data));
			Elem->MsgLen = strlen(id_data);
			StateMachinePerformAction(pAd, &pAd->Mlme.WscMachine, Elem, pAd->Mlme.WscMachine.CurrState);
			return;
		} else if ((pEapFrame->Code == EAP_CODE_REQ) && (pEapFrame->Type == EAP_TYPE_WSC)) {
			/* EAP-Req (Messages) */
			if (Elem->MsgLen <= HeaderLen) {
				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, ("Elem->MsgLen(%ld) <= HeaderLen(%d) !!\n", Elem->MsgLen, HeaderLen));
				return;
			}

			pWpsCtrl->lastId = pEapFrame->Id;
			Elem->MsgLen -= HeaderLen;

			if (WscCheckWSCHeader((PUCHAR)pData)) {
				PWSC_FRAME			pWsc = (PWSC_FRAME) pData;

				NdisMoveMemory(Elem->Msg, pHeader->Addr2, MAC_ADDR_LEN);

				if (pWsc->OpCode == WSC_OPCODE_FRAG_ACK) {
					/*
					*	Send rest WSC frag data
					*/
					RTMP_STRING wsc_frag_ack[] = "WSC_FRAG_ACK";

					NdisMoveMemory(Elem->Msg + MAC_ADDR_LEN, wsc_frag_ack, strlen(wsc_frag_ack));
					Elem->MsgLen = strlen(wsc_frag_ack);
					StateMachinePerformAction(pAd, &pAd->Mlme.WscMachine, Elem, pAd->Mlme.WscMachine.CurrState);
				} else if (pWsc->OpCode == WSC_OPCODE_START) {
					RTMP_STRING wsc_start[] = "WSC_START";

					NdisMoveMemory(Elem->Msg + MAC_ADDR_LEN, wsc_start, strlen(wsc_start));
					Elem->MsgLen = strlen(wsc_start);
					StateMachinePerformAction(pAd, &pAd->Mlme.WscMachine, Elem, pAd->Mlme.WscMachine.CurrState);
				} else {
					if (pWsc->Flags & WSC_MSG_FLAG_LF) {
						pData += (sizeof(WSC_FRAME) + 2);
						Elem->MsgLen -= (sizeof(WSC_FRAME) + 2);
					} else {
						pData += sizeof(WSC_FRAME);
						Elem->MsgLen -= sizeof(WSC_FRAME);
					}

					if ((pWpsCtrl->WscRxBufLen + Elem->MsgLen) < (MAX_MGMT_PKT_LEN - 6)) {
						NdisMoveMemory((pWpsCtrl->pWscRxBuf + pWpsCtrl->WscRxBufLen), pData, Elem->MsgLen);
						pWpsCtrl->WscRxBufLen += Elem->MsgLen;
					}

#ifdef WSC_V2_SUPPORT

					if (pWsc->Flags & WSC_MSG_FLAG_MF)
						WscSendEapFragAck(pAd, pWpsCtrl, pEntry);
					else
#endif /* WSC_V2_SUPPORT */
					{
						NdisMoveMemory(Elem->Msg + MAC_ADDR_LEN, pWpsCtrl->pWscRxBuf, pWpsCtrl->WscRxBufLen);
						Elem->MsgLen = pWpsCtrl->WscRxBufLen;
						StateMachinePerformAction(pAd, &pAd->Mlme.WscMachine, Elem, pAd->Mlme.WscMachine.CurrState);
						pWpsCtrl->WscRxBufLen = 0;
						NdisZeroMemory(pWpsCtrl->pWscRxBuf, MAX_MGMT_PKT_LEN);
					}
				}

				return;
			}
			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("ERROR: WscCheckWSCHeader() return FALSE!\n"));
			return;
		}

		if (Elem->MsgLen <= HeaderLen) {
			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, ("Elem->MsgLen(%ld) <= HeaderLen(%d) !!\n", Elem->MsgLen, HeaderLen));
			return;
		}

		Elem->MsgLen -= HeaderLen;
		NdisMoveMemory(Elem->Msg, pHeader->Addr2, MAC_ADDR_LEN);

		if (IS_ENTRY_CLIENT(pEntry) &&
			(pEapFrame->Code == EAP_CODE_RSP) &&
			(pEapFrame->Type == EAP_TYPE_ID)) {
			BOOLEAN Cancelled;

			if (strstr(pData, "SimpleConfig")) {
				/* EAP-Rsp (Identity) */
				NdisMoveMemory(Elem->Msg + 6, pData, Elem->MsgLen);
				StateMachinePerformAction(pAd, &pAd->Mlme.WscMachine, Elem, pAd->Mlme.WscMachine.CurrState);
				return;
			}
			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("RTMPCancelTimer EapolTimer!!\n"));
			NdisZeroMemory(pWpsCtrl->EntryAddr, MAC_ADDR_LEN);
			pWpsCtrl->EapolTimerRunning = FALSE;
			RTMPCancelTimer(&pWpsCtrl->EapolTimer, &Cancelled);
			return;
		}
		if (WscCheckWSCHeader((PUCHAR) pData)) {
			/* EAP-Rsp (Messages) */
			PWSC_FRAME			pWsc = (PWSC_FRAME) pData;

			if (pWsc->OpCode == WSC_OPCODE_FRAG_ACK) {
				/*
				*	Send rest frag data
				*/
				RTMP_STRING wsc_frag_ack[] = "WSC_FRAG_ACK";

				NdisMoveMemory(Elem->Msg + MAC_ADDR_LEN, wsc_frag_ack, strlen(wsc_frag_ack));
				Elem->MsgLen = strlen(wsc_frag_ack);
				StateMachinePerformAction(pAd, &pAd->Mlme.WscMachine, Elem, pAd->Mlme.WscMachine.CurrState);
			} else {
				if (pWsc->Flags & WSC_MSG_FLAG_LF) {
					pData += (sizeof(WSC_FRAME) + 2);
					Elem->MsgLen -= (sizeof(WSC_FRAME) + 2);
				} else {
					pData += sizeof(WSC_FRAME);
					Elem->MsgLen -= sizeof(WSC_FRAME);
				}

				if ((pWpsCtrl->WscRxBufLen + Elem->MsgLen) < (MAX_MGMT_PKT_LEN - 6)) {
					NdisMoveMemory((pWpsCtrl->pWscRxBuf + pWpsCtrl->WscRxBufLen), pData, Elem->MsgLen);
					pWpsCtrl->WscRxBufLen += Elem->MsgLen;
				}

#ifdef WSC_V2_SUPPORT

				if (pWsc->Flags & WSC_MSG_FLAG_MF)
					WscSendEapFragAck(pAd, pWpsCtrl, pEntry);
				else
#endif /* WSC_V2_SUPPORT */
				{
					/* NdisMoveMemory(Elem->Msg+6, pData, Elem->MsgLen); */
					NdisMoveMemory(Elem->Msg + 6, pWpsCtrl->pWscRxBuf, pWpsCtrl->WscRxBufLen);
					Elem->MsgLen = pWpsCtrl->WscRxBufLen;
					StateMachinePerformAction(pAd, &pAd->Mlme.WscMachine, Elem, pAd->Mlme.WscMachine.CurrState);
					pWpsCtrl->WscRxBufLen = 0;
					NdisZeroMemory(pWpsCtrl->pWscRxBuf, MAX_MGMT_PKT_LEN);
				}
			}

			return;
		}
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("ERROR: WscCheckWSCHeader() return FALSE!\n"));
		return;
	}
	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_WARN, ("Unknow Message Type (=%lu)\n", Elem->MsgType));
}

#ifdef CONFIG_AP_SUPPORT

#define WSC_SINGLE_TRIGGER_APPNAME  "unknown"

#ifdef SDK_GOAHEAD_HTTPD
#undef WSC_SINGLE_TRIGGER_APPNAME
#define WSC_SINGLE_TRIGGER_APPNAME  "goahead"
#endif /* SDK_GOAHEAD_HTTPD */

#ifdef SDK_USER_LIGHTY
#undef WSC_SINGLE_TRIGGER_APPNAME
#define WSC_SINGLE_TRIGGER_APPNAME  "nvram_daemon"
#endif /* SDK_USER_LIGHTY */

INT WscGetConfWithoutTrigger(
	IN  PRTMP_ADAPTER pAd,
	IN  PWSC_CTRL pWscControl,
	IN  BOOLEAN bFromUPnP)
{
	INT WscMode;
	INT IsAPConfigured;
	PWSC_UPNP_NODE_INFO pWscUPnPNodeInfo;
	UCHAR apIdx;
	struct wifi_dev *wdev = NULL;

	/* TODO: Is it possible ApCli call this function?? */
	apIdx = (pWscControl->EntryIfIdx & 0x0F);
	if (apIdx < HW_BEACON_MAX_NUM)
		wdev = &pAd->ApCfg.MBSSID[apIdx].wdev;
#ifdef LINUX
#ifdef WSC_SINGLE_TRIGGER
	if (wdev != NULL) {
		if (pAd->dev_idx == 0)
			RtmpOSWrielessEventSend(wdev->if_dev, RT_WLAN_EVENT_CUSTOM, SIGXFSZ, NULL, NULL, 0); /* ra0 */
		else
			RtmpOSWrielessEventSend(wdev->if_dev, RT_WLAN_EVENT_CUSTOM, SIGWINCH, NULL, NULL, 0); /* rai0 */
	}
#endif /* WSC_SINGLE_TRIGGER */
#endif /* LINUX */
	IsAPConfigured = pWscControl->WscConfStatus;
	pWscUPnPNodeInfo = &pWscControl->WscUPnPNodeInfo;

	if (pWscControl->WscConfMode == WSC_DISABLE) {
		pWscControl->bWscTrigger = FALSE;
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("WscGetConfForUpnp:: WPS is disabled.\n"));
		return FALSE;
	}

	if (bFromUPnP)
		WscStop(pAd, FALSE, pWscControl);

	if (pWscControl->WscMode == 1)
		WscMode = DEV_PASS_ID_PIN;
	else
		WscMode = DEV_PASS_ID_PBC;

	WscBuildBeaconIE(pAd, IsAPConfigured, TRUE, WscMode, pWscControl->WscConfigMethods, (pWscControl->EntryIfIdx & 0x0F), NULL, 0, AP_MODE);
	WscBuildProbeRespIE(pAd, WSC_MSGTYPE_AP_WLAN_MGR, IsAPConfigured, TRUE, WscMode, pWscControl->WscConfigMethods, pWscControl->EntryIfIdx, NULL, 0, AP_MODE);
	if (wdev != NULL)
		UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_IE_CHG);
	/* 2mins time-out timer */
	RTMPSetTimer(&pWscControl->Wsc2MinsTimer, WSC_TWO_MINS_TIME_OUT);
	pWscControl->Wsc2MinsTimerRunning = TRUE;
	pWscControl->WscStatus = STATUS_WSC_LINK_UP;

	if (bFromUPnP)
		WscSendUPnPConfReqMsg(pAd, apIdx, (PUCHAR)pAd->ApCfg.MBSSID[apIdx].Ssid,
							  pAd->ApCfg.MBSSID[apIdx].wdev.bssid, 3, 0, AP_MODE);

	pWscControl->bWscTrigger = TRUE;
	pWscControl->bWscAutoTigeer = TRUE;
	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("%s:: trigger WSC state machine\n", __func__));
	return TRUE;
}
#endif /* CONFIG_AP_SUPPORT */

static VOID WscSendNACK(
	IN  PRTMP_ADAPTER pAdapter,
	IN  MAC_TABLE_ENTRY *pEntry,
	IN  PWSC_CTRL pWscControl)
{
	INT DataLen = 0;
	PUCHAR  pWscData = NULL;
	BOOLEAN Cancelled;
	UCHAR CurOpMode = AP_MODE;
	os_alloc_mem(NULL, (UCHAR **)&pWscData, WSC_MAX_DATA_LEN);

	if (pWscData == NULL) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("WscSendNACK:: WscData Allocate failed!\n"));
		return;
	}

	NdisZeroMemory(pWscData, WSC_MAX_DATA_LEN);
	DataLen = BuildMessageNACK(pAdapter, pWscControl, pWscData);
#ifdef CONFIG_AP_SUPPORT

	if (CurOpMode == AP_MODE) {
		if (pEntry &&
			(IS_ENTRY_APCLI(pEntry)
			)
		   )
			WscSendMessage(pAdapter, WSC_OPCODE_NACK, pWscData, DataLen, pWscControl, AP_CLIENT_MODE, EAP_CODE_RSP);
		else
			WscSendMessage(pAdapter, WSC_OPCODE_NACK, pWscData, DataLen, pWscControl, AP_MODE, EAP_CODE_REQ);
	}

#endif /* CONFIG_AP_SUPPORT */
	RTMPCancelTimer(&pWscControl->EapolTimer, &Cancelled);
	pWscControl->EapolTimerRunning = FALSE;
	pWscControl->RegData.ReComputePke = 1;

	if (pWscData)
		os_free_mem(pWscData);
}


VOID WscCheckWpsIeFromWpsAP(
	IN  PRTMP_ADAPTER pAd,
	IN  PEID_STRUCT pEid,
	OUT PUSHORT pDPIDFromAP)
{
	PUCHAR pData;
	SHORT Len = 0;
	PWSC_IE pWscIE;
	USHORT DevicePasswordID;

	if (NdisEqualMemory(pEid->Octet, WPS_OUI, 4)
	   ) {
		pData = (PUCHAR) pEid->Octet + 4;
		Len = (SHORT)(pEid->Len - 4);

		while (Len > 0) {
			WSC_IE	WscIE;

			NdisMoveMemory(&WscIE, pData, sizeof(WSC_IE));
			/* Check for WSC IEs */
			pWscIE = &WscIE;

			/* Check for device password ID, PIN = 0x0000, PBC = 0x0004 */
			if (pDPIDFromAP && be2cpu16(pWscIE->Type) == WSC_ID_DEVICE_PWD_ID) {
				/* Found device password ID */
				NdisMoveMemory(&DevicePasswordID, pData + 4, sizeof(DevicePasswordID));
				DevicePasswordID = be2cpu16(DevicePasswordID);
				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, ("WscCheckWpsIeFromWpsAP : DevicePasswordID = 0x%04x\n", DevicePasswordID));

				if (DevicePasswordID == DEV_PASS_ID_PIN) {
					/* PIN */
					*pDPIDFromAP = DEV_PASS_ID_PIN;
				} else if (DevicePasswordID == DEV_PASS_ID_PBC) {
					/* PBC */
					*pDPIDFromAP = DEV_PASS_ID_PBC;
				}
			}

			/* Set the offset and look for PBC information */
			/* Since Type and Length are both short type, we need to offset 4, not 2 */
			pData += (be2cpu16(pWscIE->Length) + 4);
			Len   -= (be2cpu16(pWscIE->Length) + 4);
		}
	}
}


VOID WscPBCSessionOverlapCheck(
	IN  PRTMP_ADAPTER pAd,
	IN	UCHAR current_band)
{
	ULONG now;
	PWSC_STA_PBC_PROBE_INFO	pWscStaPbcProbeInfo = &pAd->CommonCfg.WscStaPbcProbeInfo;
#ifdef CONFIG_AP_SUPPORT
	UCHAR bss_index = 0;
#endif

	pAd->CommonCfg.WscPBCOverlap = FALSE;

	if (pWscStaPbcProbeInfo->WscPBCStaProbeCount[current_band] > 1) {
		UCHAR  i;

		for (i = 0; i < MAX_PBC_STA_TABLE_SIZE; i++) {
			NdisGetSystemUpTime(&now);

			if (pWscStaPbcProbeInfo->Valid[current_band][i] &&
				RTMP_TIME_AFTER(now, pWscStaPbcProbeInfo->ReciveTime[current_band][i] + 120 * OS_HZ)) {
				NdisZeroMemory(&(pWscStaPbcProbeInfo->StaMacAddr[current_band][i][0]), MAC_ADDR_LEN);
				pWscStaPbcProbeInfo->ReciveTime[current_band][i] = 0;
				pWscStaPbcProbeInfo->Valid[current_band][i] = FALSE;
				pWscStaPbcProbeInfo->WscPBCStaProbeCount[current_band]--;
			}
		}

		if (pWscStaPbcProbeInfo->WscPBCStaProbeCount[current_band] > 1) {
#ifdef CONFIG_AP_SUPPORT
			/* For the current band , check whether any bss has triggered wps, if yes,
			*   do WscCheckPeerDPID to add WscPBCStaProbeCount which is ued to
			*   check if any wps pbc overlap existed; if not, do nothing to avoid false
			*   alarm of wps pbc overlap
			*/
			for (bss_index = 0; bss_index < pAd->ApCfg.BssidNum; bss_index++) {
				if ((current_band == HcGetBandByWdev(&pAd->ApCfg.MBSSID[bss_index].wdev)) &&
						pAd->ApCfg.MBSSID[bss_index].wdev.WscControl.WscConfMode != WSC_DISABLE &&
						pAd->ApCfg.MBSSID[bss_index].wdev.WscControl.bWscTrigger == TRUE) {
					MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
								("%s(): found pAd->ApCfg.MBSSID[%d] WPS on\n",
								__func__, bss_index));
					break;
				}
			}
#ifdef DBDC_MODE
			/*bss in current band has triggered wps pbc, so check Peer DPID*/
			if (bss_index < pAd->ApCfg.BssidNum) {
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("%s(): pAd->ApCfg.MBSSID[%d] WPS on, PBC Overlap detected\n",
					__func__, bss_index));
				pAd->CommonCfg.WscPBCOverlap = TRUE;
			} else {
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("%s(): pAd->ApCfg.MBSSID[%d] WPS off, PBC Overlap is invalid\n",
					__func__, bss_index));
				pAd->CommonCfg.WscPBCOverlap = FALSE;
			}
#else
			pAd->CommonCfg.WscPBCOverlap = TRUE;
#endif
#endif /* CONFIG_AP_SUPPORT */
		}
	}

	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("WscPBCSessionOverlapCheck : WscPBCStaProbeCount[%d] = %d\n",
			 current_band, pWscStaPbcProbeInfo->WscPBCStaProbeCount[current_band]));
}

VOID WscPBC_DPID_FromSTA(
	IN  PRTMP_ADAPTER pAd,
	IN  PUCHAR pMacAddr,
	IN	UCHAR current_band)
{
	INT Index = 0;
	UCHAR tab_idx;
	BOOLEAN bAddEntry = FALSE;
	ULONG now;
	PWSC_STA_PBC_PROBE_INFO	pWscStaPbcProbeInfo = &pAd->CommonCfg.WscStaPbcProbeInfo;

	NdisGetSystemUpTime(&now);
	if (pWscStaPbcProbeInfo->WscPBCStaProbeCount[current_band] == 0)
		bAddEntry = TRUE;
	else {
		for (tab_idx = 0; tab_idx < MAX_PBC_STA_TABLE_SIZE; tab_idx++) {
			if (NdisEqualMemory(pMacAddr, pWscStaPbcProbeInfo->StaMacAddr[current_band][tab_idx], MAC_ADDR_LEN)) {
				pWscStaPbcProbeInfo->ReciveTime[current_band][tab_idx] = now;
				return;
			}
		}
		for (tab_idx = 0; tab_idx < MAX_PBC_STA_TABLE_SIZE; tab_idx++) {
			if (RTMP_TIME_AFTER(now, pWscStaPbcProbeInfo->ReciveTime[current_band][tab_idx] + 120 * OS_HZ) ||
				NdisEqualMemory(pWscStaPbcProbeInfo->StaMacAddr[current_band][tab_idx], &ZERO_MAC_ADDR[0], MAC_ADDR_LEN)) {
				if (pWscStaPbcProbeInfo->Valid[current_band][tab_idx] == FALSE) {
					Index = tab_idx;
					bAddEntry = TRUE;
					break;
				}
				pWscStaPbcProbeInfo->ReciveTime[current_band][tab_idx] = now;
				NdisMoveMemory(pWscStaPbcProbeInfo->StaMacAddr[current_band][tab_idx], pMacAddr, MAC_ADDR_LEN);
				return;
			}
		}
	}
	if (bAddEntry) {
		pWscStaPbcProbeInfo->WscPBCStaProbeCount[current_band]++;
		pWscStaPbcProbeInfo->ReciveTime[current_band][Index] = now;
		pWscStaPbcProbeInfo->Valid[current_band][Index] = TRUE;
		NdisMoveMemory(pWscStaPbcProbeInfo->StaMacAddr[current_band][Index], pMacAddr, MAC_ADDR_LEN);
	}

	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("%s(): STA_MAC = %02x:%02x:%02x:%02x:%02x:%02x\n",
			 __func__, PRINT_MAC(pMacAddr)));
	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("%s(): WscPBCStaProbeCount[%d] = %d\n",
			 __func__, current_band, pWscStaPbcProbeInfo->WscPBCStaProbeCount[current_band]));
}


static VOID WscWriteAuthToDAT(
	IN  RTMP_ADAPTER *pAd,
	IN  UCHAR CurOpMode,
	IN  RTMP_STRING *pTempStr)
{
#ifdef CONFIG_AP_SUPPORT
	if (CurOpMode == AP_MODE) {
		INT index;

		for (index = 0; index < pAd->ApCfg.BssidNum; index++) {
			if (pAd->ApCfg.MBSSID[index].SsidLen) {
				if (index == 0)
					snprintf(pTempStr, 512, "%s%s", pTempStr, GetAuthModeStr(pAd->ApCfg.MBSSID[index].wdev.SecConfig.AKMMap));
				else
					snprintf(pTempStr, 512, "%s;%s", pTempStr, GetAuthModeStr(pAd->ApCfg.MBSSID[index].wdev.SecConfig.AKMMap));
			}
		}
	}
#endif /* CONFIG_AP_SUPPORT */
}

static VOID WscWriteEncrToDAT(
	IN  RTMP_ADAPTER *pAd,
	IN  UCHAR CurOpMode,
	IN  RTMP_STRING *pTempStr)
{
#ifdef CONFIG_AP_SUPPORT
	if (CurOpMode == AP_MODE) {
		INT index;

		for (index = 0; index < pAd->ApCfg.BssidNum; index++) {
			if (index == 0)
				snprintf(pTempStr, 512, "%s%s", pTempStr,
				GetEncryModeStr(pAd->ApCfg.MBSSID[index].wdev.SecConfig.PairwiseCipher));
			else
				snprintf(pTempStr, 512, "%s;%s", pTempStr,
				GetEncryModeStr(pAd->ApCfg.MBSSID[index].wdev.SecConfig.PairwiseCipher));
		}
	}
#endif /* CONFIG_AP_SUPPORT */
}

static VOID WscWriteWscConfModeToDAT(
	IN  RTMP_ADAPTER *pAd,
	IN  UCHAR CurOpMode,
	IN  RTMP_STRING *pTempStr)
{
	WSC_CTRL *wsc_ctrl;

#ifdef CONFIG_AP_SUPPORT
	if (CurOpMode == AP_MODE) {
		INT index;

		for (index = 0; index < pAd->ApCfg.BssidNum; index++) {
			wsc_ctrl = &pAd->ApCfg.MBSSID[index].wdev.WscControl;

			if (index == 0)
				snprintf(pTempStr, 512, "%s%d", pTempStr, wsc_ctrl->WscConfMode);
			else
				snprintf(pTempStr, 512, "%s;%d", pTempStr, wsc_ctrl->WscConfMode);
		}
	}
#endif /* CONFIG_AP_SUPPORT */
}

static VOID WscWriteWscConfStatusToDAT(
	IN  RTMP_ADAPTER *pAd,
	IN  UCHAR CurOpMode,
	IN  RTMP_STRING *pTempStr)
{
	WSC_CTRL *wsc_ctrl;

#ifdef CONFIG_AP_SUPPORT
	if (CurOpMode == AP_MODE) {
		INT index;

		for (index = 0; index < pAd->ApCfg.BssidNum; index++) {
			wsc_ctrl = &pAd->ApCfg.MBSSID[index].wdev.WscControl;

			if (index == 0)
				snprintf(pTempStr, 512, "%s%d", pTempStr, wsc_ctrl->WscConfStatus);
			else
				snprintf(pTempStr, 512, "%s;%d", pTempStr, wsc_ctrl->WscConfStatus);
		}
	}
#endif /* CONFIG_AP_SUPPORT */
}

static VOID WscWriteDefaultKeyIdToDAT(
	IN  RTMP_ADAPTER *pAd,
	IN  UCHAR CurOpMode,
	IN  RTMP_STRING *pTempStr)
{
#ifdef CONFIG_AP_SUPPORT
	if (CurOpMode == AP_MODE) {
		INT index;

		for (index = 0; index < pAd->ApCfg.BssidNum; index++) {
			if (index == 0)
				snprintf(pTempStr, 512, "%s%d", pTempStr, pAd->ApCfg.MBSSID[index].wdev.SecConfig.PairwiseKeyId + 1);
			else
				snprintf(pTempStr, 512, "%s;%d", pTempStr, pAd->ApCfg.MBSSID[index].wdev.SecConfig.PairwiseKeyId + 1);
		}
	}
#endif /* CONFIG_AP_SUPPORT */
}

#define WSC_PRINT_WEP_KEY(__idx, __key_len, __key, __out_buf) \
{ \
	for (__idx = 0; __idx < __key_len; (__idx)++) \
		snprintf(__out_buf, 512, "%s%02x", __out_buf, __key[__idx]); \
}

static BOOLEAN WscWriteWEPKeyToDAT(
	IN  RTMP_ADAPTER *pAd,
	IN  UCHAR CurOpMode,
	IN  RTMP_STRING *WepKeyFormatName,
	IN  RTMP_STRING *WepKeyName,
	IN  RTMP_STRING *pTempStr)
{
	WSC_CREDENTIAL *pCredentail;
	INT tempStrLen = 0;

#ifdef CONFIG_AP_SUPPORT
	if (CurOpMode == AP_MODE) {
		UCHAR apidx = (pAd->WriteWscCfgToDatFile & 0x0F);

		if ((strncmp(pTempStr, WepKeyFormatName, strlen(WepKeyFormatName)) == 0)) {
			pCredentail = &pAd->ApCfg.MBSSID[apidx].wdev.WscControl.WscProfile.Profile[0];

			if (IS_CIPHER_WEP_Entry(&pAd->ApCfg.MBSSID[apidx].wdev)) {
				UCHAR idx = 0, KeyType[4] = {0};
				RTMP_STRING *ptr2, *temp_ptr;

				ptr2 = rtstrstr(pTempStr, "=");

				if (!ptr2)
					return FALSE;

				temp_ptr = pTempStr;
				pTempStr = ptr2 + 1;
				KeyType[0] = (UCHAR)(*pTempStr - 0x30);

				for (idx = 1; idx < 4; idx++) {
					ptr2 = rtstrstr(pTempStr, ";");

					if (ptr2 == NULL)
						break;

					pTempStr = ptr2 + 1;

					if ((*pTempStr == '0') || (*pTempStr == '1'))
						KeyType[idx] = (UCHAR)(*pTempStr - 0x30);
				}

				pTempStr = temp_ptr;
				NdisZeroMemory(pTempStr, 512);
				NdisMoveMemory(pTempStr, WepKeyFormatName, strlen(WepKeyFormatName));

				for (idx = 0; idx < pAd->ApCfg.BssidNum; idx++) {
					if (idx == apidx)
						snprintf(pTempStr, 512, "%s0", pTempStr);
					else
						snprintf(pTempStr, 512, "%s%d", pTempStr, KeyType[idx]);

					if (apidx < (pAd->ApCfg.BssidNum - 1))
						snprintf(pTempStr, 512, "%s;", pTempStr);
				}
			}
		} else if ((strncmp(pTempStr, WepKeyName, strlen(WepKeyName)) == 0)) {
			pCredentail = &pAd->ApCfg.MBSSID[apidx].wdev.WscControl.WscProfile.Profile[0];

			if (IS_CIPHER_WEP_Entry(&pAd->ApCfg.MBSSID[apidx].wdev)) {
				NdisZeroMemory(pTempStr, 512);
				NdisMoveMemory(pTempStr, WepKeyName, strlen(WepKeyName));
				tempStrLen = strlen(pTempStr);

				if (pCredentail->KeyLength) {
					if ((pCredentail->KeyLength == 5) ||
						(pCredentail->KeyLength == 13)) {
						int jjj = 0;

						WSC_PRINT_WEP_KEY(jjj, pCredentail->KeyLength, pCredentail->Key, pTempStr);
					} else if ((pCredentail->KeyLength == 10) ||
							   (pCredentail->KeyLength == 26))
						NdisMoveMemory(pTempStr + tempStrLen, pCredentail->Key, pCredentail->KeyLength);
				}
			}
		}
	}
#endif /* CONFIG_AP_SUPPORT */
	return TRUE;
}

VOID WscWriteConfToDatFile(RTMP_ADAPTER *pAd, UCHAR CurOpMode)
{
	char *cfgData = 0;
	RTMP_STRING *fileName = NULL;
	RTMP_OS_FD file_r, file_w;
	RTMP_OS_FS_INFO osFSInfo;
	LONG rv, fileLen = 0;
	char *offset = 0;
	RTMP_STRING *pTempStr = NULL;
#ifdef CONFIG_AP_SUPPORT
	UCHAR apidx = (pAd->WriteWscCfgToDatFile & 0x0F);
#endif /* CONFIG_AP_SUPPORT */
	RTMP_STRING WepKeyName[MAX_WEPKEYNAME_LEN] = {0};
	RTMP_STRING WepKeyFormatName[MAX_WEPKEYNAME_LEN] = {0};

	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE,
		("-----> WscWriteConfToDatFile(CurOpMode = %d)\n", CurOpMode));
#ifdef CONFIG_AP_SUPPORT

	if (CurOpMode == AP_MODE) {
		if (apidx > pAd->ApCfg.BssidNum) {
			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE,
				("<----- WscWriteConfToDatFile (wrong apidx = %d)\n", apidx));
			return;
		}

		fileName = get_dev_l2profile(pAd);

		snprintf((RTMP_STRING *) WepKeyName, sizeof(WepKeyName), "Key%dStr%d=", pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.PairwiseKeyId + 1, apidx + 1);
		snprintf((RTMP_STRING *) WepKeyFormatName, sizeof(WepKeyFormatName), "Key%dType=", pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.PairwiseKeyId + 1);
	}

#endif /* CONFIG_AP_SUPPORT */
	RtmpOSFSInfoChange(&osFSInfo, TRUE);
	file_r = RtmpOSFileOpen(fileName, O_RDONLY, 0);

	if (IS_FILE_OPEN_ERR(file_r)) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE,
			("-->1) %s: Error opening file %s\n", __func__, fileName));
		return;
	}
	{
		char tempStr[64] = {0};

		while ((rv = RtmpOSFileRead(file_r, tempStr, 64)) > 0)
			fileLen += rv;

		os_alloc_mem(NULL, (UCHAR **)&cfgData, fileLen);

		if (cfgData == NULL) {
			RtmpOSFileClose(file_r);
			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE,
				("CfgData mem alloc fail. (fileLen = %ld)\n", fileLen));
			goto out;
		}

		NdisZeroMemory(cfgData, fileLen);
		RtmpOSFileSeek(file_r, 0);
		rv = RtmpOSFileRead(file_r, (RTMP_STRING *)cfgData, fileLen);
		RtmpOSFileClose(file_r);

		if (rv != fileLen) {
			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE,
				("CfgData mem alloc fail, fileLen = %ld\n", fileLen));
			goto ReadErr;
		}
	}

	file_w = RtmpOSFileOpen(fileName, O_WRONLY | O_TRUNC, 0);

	if (IS_FILE_OPEN_ERR(file_w))
		goto WriteFileOpenErr;
	else {
		offset = (PCHAR) rtstrstr((RTMP_STRING *) cfgData, "Default\n");
		offset += strlen("Default\n");
		RtmpOSFileWrite(file_w, (RTMP_STRING *)cfgData, (int)(offset - cfgData));
		os_alloc_mem(NULL, (UCHAR **)&pTempStr, 512);

		if (!pTempStr) {
			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE,
				("pTempStr mem alloc fail. (512)\n"));
			RtmpOSFileClose(file_w);
			goto WriteErr;
		}

		for (;;) {
			int i = 0;
			RTMP_STRING *ptr;
			BOOLEAN	bNewFormat = TRUE;

			NdisZeroMemory(pTempStr, 512);

			if ((size_t)(offset - cfgData) < fileLen) {
				ptr = (RTMP_STRING *) offset;

				while (*ptr && *ptr != '\n')
					pTempStr[i++] = *ptr++;

				pTempStr[i] = 0x00;
				offset += strlen(pTempStr) + 1;

				if ((strncmp(pTempStr, "SSID=", strlen("SSID=")) == 0) ||
					strncmp(pTempStr, "SSID1=", strlen("SSID1=")) == 0 ||
					strncmp(pTempStr, "SSID2=", strlen("SSID2=")) == 0 ||
					strncmp(pTempStr, "SSID3=", strlen("SSID3=")) == 0 ||
					strncmp(pTempStr, "SSID4=", strlen("SSID4=")) == 0
				   ) {
					if (rtstrstr(pTempStr, "SSID="))
						bNewFormat = FALSE;

					WscWriteSsidToDatFile(pAd, pTempStr, bNewFormat, CurOpMode);
				}

				else if (strncmp(pTempStr, "AuthMode=", strlen("AuthMode=")) == 0) {
					NdisZeroMemory(pTempStr, 512);
					snprintf(pTempStr, 512, "AuthMode=");
					WscWriteAuthToDAT(pAd, CurOpMode, pTempStr);
				} else if (strncmp(pTempStr, "EncrypType=", strlen("EncrypType=")) == 0) {
					NdisZeroMemory(pTempStr, 512);
					snprintf(pTempStr, 512, "EncrypType=");
					WscWriteEncrToDAT(pAd, CurOpMode, pTempStr);
				} else if ((strncmp(pTempStr, "WPAPSK=", strlen("WPAPSK=")) == 0) ||
						   (strncmp(pTempStr, "WPAPSK1=", strlen("WPAPSK1=")) == 0) ||
						   (strncmp(pTempStr, "WPAPSK2=", strlen("WPAPSK2=")) == 0) ||
						   (strncmp(pTempStr, "WPAPSK3=", strlen("WPAPSK3=")) == 0) ||
						   (strncmp(pTempStr, "WPAPSK4=", strlen("WPAPSK4=")) == 0)) {
					bNewFormat = TRUE;

					if (strstr(pTempStr, "WPAPSK="))
						bNewFormat = FALSE;

					WscWriteWpaPskToDatFile(pAd, pTempStr, bNewFormat);
				} else if (strncmp(pTempStr, "WscConfMode=", strlen("WscConfMode=")) == 0) {
					snprintf(pTempStr, 512, "WscConfMode=");
					WscWriteWscConfModeToDAT(pAd, CurOpMode, pTempStr);
				} else if (strncmp(pTempStr, "WscConfStatus=", strlen("WscConfStatus=")) == 0) {
					snprintf(pTempStr, 512, "WscConfStatus=");
					WscWriteWscConfStatusToDAT(pAd, CurOpMode, pTempStr);
				} else if (strncmp(pTempStr, "DefaultKeyID=", strlen("DefaultKeyID=")) == 0) {
					NdisZeroMemory(pTempStr, 512);
					snprintf(pTempStr, 512, "DefaultKeyID=");
					WscWriteDefaultKeyIdToDAT(pAd, CurOpMode, pTempStr);
				} else {
					if (WscWriteWEPKeyToDAT(pAd, CurOpMode, WepKeyFormatName, WepKeyName, pTempStr) == FALSE)
						goto WriteErr;
				}
				RtmpOSFileWrite(file_w, pTempStr, strlen(pTempStr));
				RtmpOSFileWrite(file_w, "\n", 1);
			} else
				break;
		}

		RtmpOSFileClose(file_w);
	}

WriteErr:

	if (pTempStr)
		os_free_mem(pTempStr);

ReadErr:
WriteFileOpenErr:

	if (cfgData)
		os_free_mem(cfgData);

out:
	RtmpOSFSInfoChange(&osFSInfo, FALSE);
	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("<----- WscWriteConfToDatFile\n"));
	return;
	}

#ifdef CONFIG_AP_SUPPORT
static VOID WscWriteWepKeyToAR9File(
	IN  RTMP_ADAPTER *pAd,
	IN  RTMP_STRING *pTempStr,
	IN  RTMP_STRING *pDatStr,
	OUT INT *datoffset)
{
	INT index;
	INT apidx;
	INT offset;
	INT tempStrLen;
	WSC_CREDENTIAL *pCredentail = NULL;
	RTMP_STRING WepKeyName[MAX_WEPKEYNAME_LEN] = {0};
	RTMP_STRING WepKeyFormatName[MAX_WEPKEYTYPE_LEN] = {0};

	for (index = 1; index <= 4; index++) {
		snprintf(WepKeyFormatName, sizeof(WepKeyFormatName), "Key%dType=", index);
		{
			NdisZeroMemory(pTempStr, 512);
			offset = 0;
			NdisMoveMemory(pTempStr, WepKeyFormatName, strlen(WepKeyFormatName));

			for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++) {
				if (IS_CIPHER_WEP(pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev.SecConfig.PairwiseCipher)) {
					pCredentail = &pAd->ApCfg.MBSSID[apidx].wdev.WscControl.WscProfile.Profile[0];

					if ((pCredentail->KeyLength == 5) ||
						(pCredentail->KeyLength == 13))
						snprintf(pTempStr, 512, "%s1", pTempStr); /* ASCII */
					else
						snprintf(pTempStr, 512, "%s0", pTempStr); /* Hex */
				}

				if (apidx < (pAd->ApCfg.BssidNum - 1))
					snprintf(pTempStr, 512, "%s;", pTempStr);
			}

			snprintf(pTempStr, 512, "%s\n", pTempStr);
			offset = strlen(pTempStr);
			NdisMoveMemory(pDatStr + (*datoffset), pTempStr, offset);
			(*datoffset) += offset;
		}
		snprintf(WepKeyName, sizeof(WepKeyName), "Key%dStr=", index);
		/*if (rtstrstr(pTempStr, WepKeyName)) */
		{
			NdisZeroMemory(pTempStr, 512);
			offset = 0;
			NdisMoveMemory(pTempStr, WepKeyName, strlen(WepKeyName));
			tempStrLen = strlen(pTempStr);

			for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++) {
				pCredentail = &pAd->ApCfg.MBSSID[apidx].wdev.WscControl.WscProfile.Profile[0];

				if (pCredentail->KeyLength) {
					NdisMoveMemory(pTempStr + tempStrLen, pCredentail->Key, pCredentail->KeyLength);
					tempStrLen = strlen(pTempStr);
				}

				if (apidx < (pAd->ApCfg.BssidNum - 1))
					NdisMoveMemory(pTempStr + tempStrLen, ";", 1);

				tempStrLen += 1;
			}

			snprintf(pTempStr, 512, "%s\n", pTempStr);
			offset = strlen(pTempStr);
			NdisMoveMemory(pDatStr + (*datoffset), pTempStr, offset);
			(*datoffset) += offset;
		}

		for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++) {
			snprintf(WepKeyName, sizeof(WepKeyName), "Key%dStr%d=", index, (apidx + 1));

			if (IS_CIPHER_WEP(pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.PairwiseCipher)) {
				NdisZeroMemory(pTempStr, 512);
				NdisMoveMemory(pTempStr, WepKeyName, strlen(WepKeyName));
				tempStrLen = strlen(pTempStr);
				pCredentail = &pAd->ApCfg.MBSSID[apidx].wdev.WscControl.WscProfile.Profile[0];
				NdisMoveMemory(pTempStr + tempStrLen, pCredentail->Key, pCredentail->KeyLength);
				NdisMoveMemory(pTempStr + tempStrLen + pCredentail->KeyLength, "\n", 1);
			}

			offset = tempStrLen + pCredentail->KeyLength + 1;
			NdisMoveMemory(pDatStr + (*datoffset), pTempStr, offset);
			(*datoffset) += offset;
		}
	}
}

void WscWriteConfToAR9File(
	IN  PRTMP_ADAPTER pAd,
	IN  UCHAR CurOpMode)
{
	RTMP_STRING *fileName = NULL;
	RTMP_OS_FD file_w;
	RTMP_OS_FS_INFO osFSInfo;
	INT offset = 0;
	INT datoffset = 0;
	RTMP_STRING *pTempStr = 0;
	RTMP_STRING *pDatStr = 0;
#ifdef CONFIG_AP_SUPPORT
	INT index = 0;
	UCHAR apidx = MAIN_MBSSID;
#endif /* CONFIG_AP_SUPPORT */
	PWSC_CTRL pWscControl = NULL;
	RTMP_STRING item_str[10] = {0};

	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("-----> WscWriteConfToAR9File\n"));
	RtmpOSFSInfoChange(&osFSInfo, TRUE);
	file_w = RtmpOSFileOpen(fileName, O_WRONLY | O_CREAT, 0);

	if (IS_FILE_OPEN_ERR(file_w))
		goto WriteFileOpenErr;
	else {
		os_alloc_mem(NULL, (UCHAR **)&pTempStr, 512);

		if (!pTempStr) {
			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("pTempStr mem alloc fail. (512)\n"));
			RtmpOSFileClose(file_w);
			goto WriteErr;
		}

		os_alloc_mem(NULL, (UCHAR **)&pDatStr, 4096);

		if (!pDatStr) {
			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("pDatStr mem alloc fail. (4096)\n"));
			RtmpOSFileClose(file_w);
			goto WriteErr;
		}

		NdisZeroMemory(pTempStr, 512);
		NdisZeroMemory(pDatStr, 4096);
		NdisZeroMemory(item_str, 10);

		for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++) {
			snprintf(item_str, sizeof(item_str), "SSID%d", (apidx + 1));
			{
				NdisMoveMemory(pTempStr, item_str, strlen(item_str));
				offset = strlen(pTempStr);
				NdisMoveMemory(pTempStr + offset, "=", 1);
				offset += 1;
				NdisMoveMemory(pTempStr + offset, pAd->ApCfg.MBSSID[apidx].Ssid, pAd->ApCfg.MBSSID[apidx].SsidLen);
				offset += pAd->ApCfg.MBSSID[apidx].SsidLen;
				NdisMoveMemory(pTempStr + offset, "\n", 1);
				offset += 1;
			}
			NdisZeroMemory(item_str, 10);
		}
		NdisMoveMemory(pDatStr, pTempStr, offset);
		datoffset += offset;

		offset = 0;
		NdisZeroMemory(pTempStr, 512);
		snprintf(pTempStr, 512, "AuthMode=");
		WscWriteAuthToDAT(pAd, CurOpMode, pTempStr);
		offset = strlen(pTempStr);
		NdisMoveMemory(pDatStr + datoffset, pTempStr, offset);
		datoffset += offset;

		offset = 0;
		NdisZeroMemory(pTempStr, 512);
		snprintf(pTempStr, 512, "EncrypType=");
		WscWriteEncrToDAT(pAd, CurOpMode, pTempStr);
		offset = strlen(pTempStr);
		NdisMoveMemory(pDatStr + datoffset, pTempStr, offset);
		datoffset += offset;

		offset = 0;
		NdisZeroMemory(pTempStr, 512);
		NdisZeroMemory(item_str, 10);

		for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++) {
			snprintf(item_str, sizeof(item_str), "WPAPSK%d", (apidx + 1));
			/*if (rtstrstr(pTempStr, item_str)) */
			{
				pWscControl = &pAd->ApCfg.MBSSID[apidx].wdev.WscControl;
				NdisMoveMemory(pTempStr, item_str, strlen(item_str));
				offset = strlen(pTempStr);
				NdisMoveMemory(pTempStr + offset, "=", 1);
				offset += 1;
				NdisMoveMemory(pTempStr + offset, pWscControl->WpaPsk, pWscControl->WpaPskLen);
				offset += pWscControl->WpaPskLen;
				NdisMoveMemory(pTempStr + offset, "\n", 1);
				offset += 1;
			}
			NdisZeroMemory(item_str, 10);
		}

		NdisMoveMemory(pDatStr + datoffset, pTempStr, offset);
		datoffset += offset;

		offset = 0;
		NdisZeroMemory(pTempStr, 512);
		snprintf(pTempStr, 512, "WscConfMode=");
#ifdef CONFIG_AP_SUPPORT

		for (index = 0; index < pAd->ApCfg.BssidNum; index++) {
			pWscControl = &pAd->ApCfg.MBSSID[index].wdev.WscControl;

			if (index == 0)
				snprintf(pTempStr, 512, "%s%d", pTempStr, pWscControl->WscConfMode);
			else
				snprintf(pTempStr, 512, "%s;%d", pTempStr, pWscControl->WscConfMode);
		}

		snprintf(pTempStr, 512, "%s\n", pTempStr);
		offset = strlen(pTempStr);
		NdisMoveMemory(pDatStr + datoffset, pTempStr, offset);
		datoffset += offset;
#endif /* CONFIG_AP_SUPPORT */

		offset = 0;
		NdisZeroMemory(pTempStr, 512);
		snprintf(pTempStr, 512, "WscConfStatus=");
#ifdef CONFIG_AP_SUPPORT

		for (index = 0; index < pAd->ApCfg.BssidNum; index++) {
			pWscControl = &pAd->ApCfg.MBSSID[index].wdev.WscControl;

			if (index == 0)
				snprintf(pTempStr, 512, "%s%d", pTempStr, pWscControl->WscConfStatus);
			else
				snprintf(pTempStr, 512, "%s;%d", pTempStr, pWscControl->WscConfStatus);
		}

		snprintf(pTempStr, 512, "%s\n", pTempStr);
		offset = strlen(pTempStr);
		NdisMoveMemory(pDatStr + datoffset, pTempStr, offset);
		datoffset += offset;
#endif /* CONFIG_AP_SUPPORT */

		offset = 0;
		NdisZeroMemory(pTempStr, 512);
		snprintf(pTempStr, 512, "DefaultKeyID=");
#ifdef CONFIG_AP_SUPPORT

		if (CurOpMode == AP_MODE) {
			for (index = 0; index < pAd->ApCfg.BssidNum; index++) {
				pWscControl = &pAd->ApCfg.MBSSID[index].wdev.WscControl;

				if (index == 0)
					snprintf(pTempStr, 512, "%s%d", pTempStr, pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.PairwiseKeyId + 1);
				else
					snprintf(pTempStr, 512, "%s;%d", pTempStr, pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.PairwiseKeyId + 1);
			}

			snprintf(pTempStr, 512, "%s\n", pTempStr);
			offset = strlen(pTempStr);
			NdisMoveMemory(pDatStr + datoffset, pTempStr, offset);
			datoffset += offset;
		}
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
		if (CurOpMode == AP_MODE)
			WscWriteWepKeyToAR9File(pAd, pTempStr, pDatStr, &datoffset);
#endif /* CONFIG_AP_SUPPORT */
		RtmpOSFileWrite(file_w, pDatStr, datoffset);
		RtmpOSFileClose(file_w);
	}

WriteErr:
	if (pTempStr)
		os_free_mem(pTempStr);

	if (pDatStr)
		os_free_mem(pDatStr);

WriteFileOpenErr:
	RtmpOSFSInfoChange(&osFSInfo, FALSE);
	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("<----- WscWriteConfToAR9File\n"));
}
#endif/*CONFIG_AP_SUPPORT*/

static INT wsc_write_dat_file_thread(
	IN ULONG Context)
{
	RTMP_OS_TASK *pTask;
	RTMP_ADAPTER *pAd;
	int	Status = 0;

	pTask = (RTMP_OS_TASK *)Context;
	pAd = (PRTMP_ADAPTER)RTMP_OS_TASK_DATA_GET(pTask);

	if (pAd == NULL)
		return 0;

	RtmpOSTaskCustomize(pTask);

	while (pTask && !RTMP_OS_TASK_IS_KILLED(pTask)) {
		RtmpusecDelay(2000);

		if (RtmpOSTaskWait(pAd, pTask, &Status) == FALSE) {
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);
			break;
		}

		if (Status != 0)
			break;


		if (pAd->pWscElme && (pAd->pWscElme->MsgLen != 0)) {
			MLME_QUEUE_ELEM	*pElme;

			os_alloc_mem(pAd, (UCHAR **)&pElme, sizeof(MLME_QUEUE_ELEM));

			if (pElme) {
				NdisZeroMemory(pElme, sizeof(MLME_QUEUE_ELEM));
				RTMP_SEM_LOCK(&pAd->WscElmeLock);
				NdisMoveMemory(pElme, pAd->pWscElme, sizeof(MLME_QUEUE_ELEM));
				pAd->pWscElme->MsgLen = 0;
				NdisZeroMemory(pAd->pWscElme->Msg, MAX_MGMT_PKT_LEN);
				RTMP_SEM_UNLOCK(&pAd->WscElmeLock);
				WpsSmProcess(pAd, pElme);
				os_free_mem(pElme);
			}
		}

		if (pAd->WriteWscCfgToDatFile != 0xFF) {
			UCHAR	CurOpMode = AP_MODE;
#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
			CurOpMode = AP_MODE;
#endif /* CONFIG_AP_SUPPORT */
			WscWriteConfToDatFile(pAd, CurOpMode);
			pAd->WriteWscCfgToDatFile = 0xFF;
		}
	}

	if (pTask)
		RtmpOSTaskNotifyToExit(pTask);

	return 0;
}


/*
* This kernel thread init in the probe fucntion, so we should kill it when do remove module.
*/
BOOLEAN WscThreadExit(RTMP_ADAPTER *pAd)
{
	INT ret;
	BOOLEAN Cancelled;
	/*
	*	This kernel thread init in the probe fucntion, so kill it when do remove module.
	*/
	ret = RtmpOSTaskKill(&pAd->wscTask);

	if (ret == NDIS_STATUS_FAILURE)
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, ("kill wsc task failed!\n"));

	if (pAd->pHmacData) {
		os_free_mem(pAd->pHmacData);
		pAd->pHmacData = NULL;
	}

	if (pAd->pWscElme) {
		os_free_mem(pAd->pWscElme);
		pAd->pWscElme = NULL;
	}

	NdisFreeSpinLock(&pAd->WscElmeLock);
#ifdef CONFIG_AP_SUPPORT

	if ((pAd->OpMode == OPMODE_AP)
	   ) {
		INT ap_idx;
		UCHAR MaxBssidNum = MAX_MBSSID_NUM(pAd);

		for (ap_idx = 0; ap_idx < MaxBssidNum; ap_idx++) {
			PWSC_CTRL	pWpsCtrl = &pAd->ApCfg.MBSSID[ap_idx].wdev.WscControl;

			WscStop(pAd, FALSE, pWpsCtrl);
			RTMPReleaseTimer(&pWpsCtrl->EapolTimer, &Cancelled);
			RTMPReleaseTimer(&pWpsCtrl->Wsc2MinsTimer, &Cancelled);
			RTMPReleaseTimer(&pWpsCtrl->WscUPnPNodeInfo.UPnPMsgTimer, &Cancelled);
			RTMPReleaseTimer(&pWpsCtrl->M2DTimer, &Cancelled);
#ifdef WSC_LED_SUPPORT
			RTMPReleaseTimer(&pWpsCtrl->WscLEDTimer, &Cancelled);
			RTMPReleaseTimer(&pWpsCtrl->WscSkipTurnOffLEDTimer, &Cancelled);
#endif
			RTMPReleaseTimer(&pWpsCtrl->WscUpdatePortCfgTimer, &Cancelled);
#ifdef WSC_V2_SUPPORT
			RTMPReleaseTimer(&pWpsCtrl->WscSetupLockTimer, &Cancelled);
#endif
			pWpsCtrl->WscRxBufLen = 0;

			if (pWpsCtrl->pWscRxBuf) {
				os_free_mem(pWpsCtrl->pWscRxBuf);
				pWpsCtrl->pWscRxBuf = NULL;
			}

			pWpsCtrl->WscTxBufLen = 0;

			if (pWpsCtrl->pWscTxBuf) {
				os_free_mem(pWpsCtrl->pWscTxBuf);
				pWpsCtrl->pWscTxBuf = NULL;
			}

#ifdef WSC_V2_SUPPORT

			if (pWpsCtrl->WscSetupLockTimerRunning) {
				BOOLEAN Cancelled;

				pWpsCtrl->WscSetupLockTimerRunning = FALSE;
				RTMPCancelTimer(&pWpsCtrl->WscSetupLockTimer, &Cancelled);
			}

			if (pWpsCtrl->WscV2Info.ExtraTlv.pTlvData) {
				os_free_mem(pWpsCtrl->WscV2Info.ExtraTlv.pTlvData);
				pWpsCtrl->WscV2Info.ExtraTlv.pTlvData = NULL;
			}

#endif /* WSC_V2_SUPPORT */
			WscClearPeerList(&pWpsCtrl->WscPeerList);
			NdisFreeSpinLock(&pWpsCtrl->WscPeerListSemLock);
		}

#ifdef APCLI_SUPPORT
		{
			INT index;

			for (index = 0; index < MAX_APCLI_NUM; index++) {
				PWSC_CTRL       pWpsCtrl = &pAd->ApCfg.ApCliTab[index].wdev.WscControl;

				WscStop(pAd, TRUE, pWpsCtrl);
				RTMPReleaseTimer(&pWpsCtrl->EapolTimer, &Cancelled);
				RTMPReleaseTimer(&pWpsCtrl->Wsc2MinsTimer, &Cancelled);
				RTMPReleaseTimer(&pWpsCtrl->WscUPnPNodeInfo.UPnPMsgTimer, &Cancelled);
				RTMPReleaseTimer(&pWpsCtrl->M2DTimer, &Cancelled);
#ifdef WSC_LED_SUPPORT
				RTMPReleaseTimer(&pWpsCtrl->WscLEDTimer, &Cancelled);
				RTMPReleaseTimer(&pWpsCtrl->WscSkipTurnOffLEDTimer, &Cancelled);
#endif
				RTMPReleaseTimer(&pWpsCtrl->WscPBCTimer, &Cancelled);
				RTMPReleaseTimer(&pWpsCtrl->WscScanTimer, &Cancelled);
				RTMPReleaseTimer(&pWpsCtrl->WscProfileRetryTimer, &Cancelled);
#ifdef CON_WPS
				RTMPReleaseTimer(&pWpsCtrl->ConWscApcliScanDoneCheckTimer, &Cancelled);
#endif /* CON_WPS */
				pWpsCtrl->WscTxBufLen = 0;

				if (pWpsCtrl->pWscTxBuf) {
					os_free_mem(pWpsCtrl->pWscTxBuf);
					pWpsCtrl->pWscTxBuf = NULL;
				}

				pWpsCtrl->WscRxBufLen = 0;

				if (pWpsCtrl->pWscRxBuf) {
					os_free_mem(pWpsCtrl->pWscRxBuf);
					pWpsCtrl->pWscRxBuf = NULL;
				}

				if (pWpsCtrl->pWscRxBuf)
					os_free_mem(pWpsCtrl->pWscRxBuf);

				WscClearPeerList(&pWpsCtrl->WscPeerList);
				NdisFreeSpinLock(&pWpsCtrl->WscPeerListSemLock);
			}
		}
#endif /* APCLI_SUPPORT */
#ifdef CON_WPS
		RTMPReleaseTimer(&pAd->ApCfg.ConWpsApCliBandMonitorTimer, &Cancelled);
#endif /* CON_WPS */
	}

#endif /* CONFIG_AP_SUPPORT */
	/* WSC hardware push button function 0811 */
	WSC_HDR_BTN_Stop(pAd);
	return TRUE;
}


/*
  * This kernel thread init in the probe function.
  */
NDIS_STATUS WscThreadInit(RTMP_ADAPTER *pAd)
{
	NDIS_STATUS status = NDIS_STATUS_FAILURE;
	RTMP_OS_TASK *pTask;

	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("-->WscThreadInit()\n"));
	pTask = &pAd->wscTask;
	RTMP_OS_TASK_INIT(pTask, "RtmpWscTask", pAd);
	status = RtmpOSTaskAttach(pTask, wsc_write_dat_file_thread, (ULONG)&pAd->wscTask);

	if (status == NDIS_STATUS_SUCCESS) {
		os_alloc_mem(NULL, &pAd->pHmacData, sizeof(CHAR) * (2048));

		if (pAd->pHmacData == NULL) {
			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, ("Wsc HmacData memory alloc failed!\n"));
			status = FALSE;
		}

		NdisAllocateSpinLock(pAd, &pAd->WscElmeLock);
		os_alloc_mem(NULL, (UCHAR **)&pAd->pWscElme, sizeof(MLME_QUEUE_ELEM));
	}

	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("<--WscThreadInit(), status=%d!\n", status));
	return status;
}


/* WSC hardware push button function 0811 */
/*
*========================================================================
*Routine Description:
*	Initialize the PUSH PUTTION Check Module.
*
*Arguments:
*	ad_p			- WLAN control block pointer
*
*Return Value:
*	None
*
*Note:
*========================================================================
*/
VOID WSC_HDR_BTN_Init(
	IN  PRTMP_ADAPTER pAd)
{
	pAd->CommonCfg.WscHdrPshBtnCheckCount = 0;
} /* End of WSC_HDR_BTN_Init */


/*
*========================================================================
*Routine Description:
*	Stop the PUSH PUTTION Check Module.
*
*Arguments:
*	ad_p			- WLAN control block pointer
*
*Return Value:
*	None
*
*Note:
*========================================================================
*/
VOID WSC_HDR_BTN_Stop(
	IN  PRTMP_ADAPTER pAd)
{
	pAd->CommonCfg.WscHdrPshBtnCheckCount = 0;
} /* End of WSC_HDR_BTN_Stop */


/*
*========================================================================
*Routine Description:
*	Start the PUSH PUTTION Check thread.
*
*Arguments:
*	*Context		- WLAN control block pointer
*
*Return Value:
*	0			- terminate the thread successfully
*
*Note:
*========================================================================
*/
VOID WSC_HDR_BTN_CheckHandler(
	IN  PRTMP_ADAPTER pAd)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	BOOLEAN flg_pressed = 0;

#ifdef VENDOR_FEATURE7_SUPPORT
	/* only need to check first SSID's WscConfMode as all BSSID use same setting */
	if (pAd->ApCfg.MBSSID[0].WscControl.WscConfMode == WSC_DISABLE)
		return;
	flg_pressed = arris_wps_gpio_check_wps_button();
#else
	WSC_HDR_BTN_MR_PRESS_FLG_GET(pAd, flg_pressed);
#endif

	if (flg_pressed) {
		/* the button is pressed */
		if (pAd->CommonCfg.WscHdrPshBtnCheckCount == WSC_HDR_BTN_CONT_TIMES) {
			/* we only handle once until the button is released */
			pAd->CommonCfg.WscHdrPshBtnCheckCount = 0;
			/* execute WSC PBC function */
#ifdef VENDOR_FEATURE7_SUPPORT
			if (WMODE_CAP_5G(pAd->ApCfg.MBSSID[0].wdev.PhyMode))
				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR,
				("wsc> execute WSC PBC for 5GHz radio\n"));
			else
				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR,
				("wsc> execute WSC PBC for 2.4GHz radio\n"));
#else
			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, ("wsc> execute WSC PBC...\n"));
#endif
#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
				pObj->ioctl_if = 0;
				Set_AP_WscMode_Proc(pAd, (PUCHAR)"2"); /* 2: PBC */
				Set_AP_WscGetConf_Proc(pAd, (PUCHAR)"1"); /* 1: Trigger */
			}
#endif /* CONFIG_AP_SUPPORT */
			return;
		}

		pAd->CommonCfg.WscHdrPshBtnCheckCount++;
	} else {
		/* the button is released */
		pAd->CommonCfg.WscHdrPshBtnCheckCount = 0;
	}
}

#ifdef WSC_LED_SUPPORT
/* */
/* Support WPS LED mode (mode 7, mode 8 and mode 9). */
/* Ref: User Feedback (page 80, WPS specification 1.0) */
/* */
BOOLEAN WscSupportWPSLEDMode(
	IN PRTMP_ADAPTER pAd)
{
	if ((LED_MODE(pAd) == WPS_LED_MODE_7) ||
		(LED_MODE(pAd) == WPS_LED_MODE_8) ||
		(LED_MODE(pAd) == WPS_LED_MODE_9) ||
		(LED_MODE(pAd) == WPS_LED_MODE_11) ||
		(LED_MODE(pAd) == WPS_LED_MODE_12)
#ifdef CONFIG_WIFI_LED_SUPPORT
		|| (LED_MODE(pAd) == WPS_LED_MODE_SHARE)
#endif /* CONFIG_WIFI_LED_SUPPORT */
	   ) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("%s: Support WPS LED mode (The WPS LED mode = %d).\n",
				 __func__, LED_MODE(pAd)));
		return TRUE; /* Support WPS LED mode. */
	}
	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("%s: Not support WPS LED mode (The WPS LED mode = %d).\n",
			 __func__, LED_MODE(pAd)));
	return FALSE; /* Not support WPS LED mode. */
}

BOOLEAN WscSupportWPSLEDMode10(
	IN PRTMP_ADAPTER pAd)
{
	UCHAR led_mode;

	led_mode = LED_MODE(pAd);
	if (led_mode == WPS_LED_MODE_10) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("%s: Support WPS LED mode (The WPS LED mode = %d).\n",
				 __func__, led_mode));
		return TRUE; /*Support WPS LED mode 10. */
	}
	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("%s: Not support WPS LED mode (The WPS LED mode = %d).\n",
			 __func__, led_mode));
	return FALSE; /* Not support WPS LED mode 10. */
}

/* */
/* Whether the WPS AP has security setting or not. */
/* Note that this function is valid only after the WPS handshaking. */
/* */
BOOLEAN WscAPHasSecuritySetting(
	IN PRTMP_ADAPTER pAdapter,
	IN PWSC_CTRL pWscControl)
{
	BOOLEAN bAPHasSecuritySetting = FALSE;
	UCHAR	currentIdx = MAIN_MBSSID;
#ifdef CONFIG_AP_SUPPORT
	currentIdx = (pWscControl->EntryIfIdx & 0x0F);
#endif /* CONFIG_AP_SUPPORT */

	switch (pWscControl->WscProfile.Profile[currentIdx].EncrType) {
	case WSC_ENCRTYPE_NONE: {
		bAPHasSecuritySetting = FALSE;
		break;
	}

	case WSC_ENCRTYPE_WEP:
	case WSC_ENCRTYPE_TKIP:
	case (WSC_ENCRTYPE_TKIP | WSC_ENCRTYPE_AES):
	case WSC_ENCRTYPE_AES: {
		bAPHasSecuritySetting = TRUE;
		break;
	}

	default: {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("%s: Incorrect encryption types (%d)\n",
				 __func__, pWscControl->WscProfile.Profile[currentIdx].EncrType));
		ASSERT(FALSE);
		break;
	}
	}

	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("%s: WSC Entryption Type = %d\n",
			 __func__, pWscControl->WscProfile.Profile[currentIdx].EncrType));
	return bAPHasSecuritySetting;
}


/* */
/* After the NIC connects with a WPS AP or not, */
/* the WscLEDTimer timer controls the LED behavior according to LED mode. */
/* */
VOID WscLEDTimer(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	PWSC_CTRL pWscControl = (PWSC_CTRL)FunctionContext;
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pWscControl->pAd;
	UCHAR WPSLEDStatus = 0;

	/* WPS LED mode 7, 8, 11 and 12. */
	if ((LED_MODE(pAd) == WPS_LED_MODE_7) ||
		(LED_MODE(pAd) == WPS_LED_MODE_8) ||
		(LED_MODE(pAd) == WPS_LED_MODE_11) ||
		(LED_MODE(pAd) == WPS_LED_MODE_12)) {
		WPSLEDStatus = LED_WPS_TURN_LED_OFF;
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("%s: Turn off the WPS successful LED pattern.\n", __func__));
	} else if ((LED_MODE(pAd) == WPS_LED_MODE_9) /* WPS LED mode 9. */
#ifdef CONFIG_WIFI_LED_SUPPORT
			   || (LED_MODE(pAd) == WPS_LED_MODE_SHARE)
#endif /* CONFIG_WIFI_LED_SUPPORT */
			  ) {
		switch (pWscControl->WscLEDMode) { /* Last WPS LED state. */
		/* Turn off the blue LED after 300 seconds. */
		case LED_WPS_SUCCESS:
			WPSLEDStatus = LED_WPS_TURN_LED_OFF;
			/* Turn on/off the WPS success LED according to AP's encryption algorithm after one second. */
			RTMPSetTimer(&pWscControl->WscLEDTimer, WSC_WPS_TURN_OFF_LED_TIMEOUT);
			pWscControl->WscLEDTimerRunning = TRUE;
			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("%s: LED_WPS_SUCCESS => LED_WPS_TURN_LED_OFF\n", __func__));
			break;

		/* After turn off the blue LED for one second. */
		/* AP uses an encryption algorithm: */
		/* a) YES: Turn on the blue LED. */
		/* b) NO: Turn off the blue LED. */
		case LED_WPS_TURN_LED_OFF:
			if ((pWscControl->WscState == WSC_STATE_OFF) &&
				(pWscControl->WscStatus == STATUS_WSC_CONFIGURED)) {
				if (WscAPHasSecuritySetting(pAd, pWscControl) == TRUE) { /* The NIC connects with an AP using an encryption algorithm. */
					/* Turn WPS success LED. */
					WPSLEDStatus = LED_WPS_TURN_ON_BLUE_LED;
					MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("%s: LED_WPS_TURN_LED_OFF => LED_WPS_TURN_ON_BLUE_LED\n", __func__));
				} else { /* The NIC connects with an AP using OPEN-NONE. */
					/* Turn off the WPS LED. */
					WPSLEDStatus = LED_WPS_TURN_LED_OFF;
					MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("%s: LED_WPS_TURN_LED_OFF => LED_WPS_TURN_LED_OFF\n", __func__));
				}
			}

			break;

		/* Turn off the amber LED after 15 seconds. */
		case LED_WPS_ERROR:
			WPSLEDStatus = LED_WPS_TURN_LED_OFF; /* Turn off the WPS LED. */
			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("%s: LED_WPS_ERROR/LED_WPS_SESSION_OVERLAP_DETECTED => LED_WPS_TURN_LED_OFF\n", __func__));
			break;

		/* Turn off the amber LED after ~3 seconds. */
		case LED_WPS_SESSION_OVERLAP_DETECTED:
			WPSLEDStatus = LED_WPS_TURN_LED_OFF; /* Turn off the WPS LED. */
			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("%s: LED_WPS_SESSION_OVERLAP_DETECTED => LED_WPS_TURN_LED_OFF\n", __func__));
			break;

		default:
			/* do nothing. */
			break;
		}

		if (WPSLEDStatus)
			RTMPSetLED(pAd, WPSLEDStatus);
	} else {
		/* do nothing. */
	}
}


VOID WscSkipTurnOffLEDTimer(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	PWSC_CTRL pWscControl = (PWSC_CTRL)FunctionContext;
	/* Allow the NIC to turn off the WPS LED again. */
	pWscControl->bSkipWPSTurnOffLED = FALSE;
	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("%s: Allow the NIC to turn off the WPS LED again.\n", __func__));
}

#endif /* WSC_LED_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
VOID WscUpdatePortCfgTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	WSC_CTRL *pWscControl = (WSC_CTRL *)FunctionContext;
	RTMP_ADAPTER *pAd = NULL;
	BOOLEAN	 bEnrollee = TRUE;
	WSC_CREDENTIAL *pCredential = NULL;
	BSS_STRUCT *pMbss = NULL;

	if (pWscControl == NULL)
		return;

	pCredential = (PWSC_CREDENTIAL) &pWscControl->WscProfile.Profile[0];
	pAd = (PRTMP_ADAPTER)pWscControl->pAd;

	if (pAd == NULL)
		return;

	pMbss = &pAd->ApCfg.MBSSID[pWscControl->EntryIfIdx & 0x0F];

	if (WscGetAuthMode(pCredential->AuthType) == pMbss->wdev.SecConfig.AKMMap &&
		WscGetWepStatus(pCredential->EncrType) == pMbss->wdev.SecConfig.PairwiseCipher &&
		NdisEqualMemory(pMbss->Ssid, pCredential->SSID.Ssid, pCredential->SSID.SsidLength) &&
		NdisEqualMemory(pWscControl->WpaPsk, pCredential->Key, pCredential->KeyLength))
		return;

	if (pWscControl->WscProfile.ApplyProfileIdx & 0x8000)
		bEnrollee = FALSE;

	WscWriteConfToPortCfg(pAd,
						  pWscControl,
						  &pWscControl->WscProfile.Profile[0],
						  bEnrollee);
	pWscControl->WscProfile.ApplyProfileIdx &= 0x7FFF;
	{
		NDIS_STATUS enq_rv = 0;
		UCHAR apidx = pWscControl->EntryIfIdx & 0x0F;

		pAd->WriteWscCfgToDatFile = apidx;
		/*
		*	Using CMD thread to prevent in-band command failed.
		*	@20150710
		*/
		enq_rv = RTEnqueueInternalCmd(pAd, CMDTHREAD_AP_RESTART, (VOID *)&apidx, sizeof(UCHAR));
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("%s: en-queu CMDTHREAD_AP_RESTART - enq_rv = 0x%x\n", __func__, enq_rv));
	}

	RtmpOsTaskWakeUp(&(pAd->wscTask));
}
#endif /* CONFIG_AP_SUPPORT */

VOID WscCheckPeerDPID(
	IN  PRTMP_ADAPTER pAd,
	IN  PFRAME_802_11 Fr,
	IN  PUCHAR eid_data,
	IN  INT eid_len,
	IN	UCHAR current_band)
{
	WSC_IE *pWscIE;
	PUCHAR pData = NULL;
	INT Len = 0;
	USHORT DevicePasswordID;
	PWSC_CTRL pWscCtrl = NULL;

	pData = eid_data + 4;
	Len = eid_len - 4;

#ifdef CONFIG_AP_SUPPORT
	if ((pAd->OpMode == OPMODE_AP)
	   ) {
		UCHAR	ap_idx = 0;

		for (ap_idx = 0; ap_idx < pAd->ApCfg.BssidNum; ap_idx++) {
			if (NdisEqualMemory(Fr->Hdr.Addr1, pAd->ApCfg.MBSSID[ap_idx].wdev.bssid, MAC_ADDR_LEN)) {
				pWscCtrl = &pAd->ApCfg.MBSSID[ap_idx].wdev.WscControl;
				break;
			}
		}
	}
#endif /* CONFIG_AP_SUPPORT */

	while (Len > 0) {
		WSC_IE	WscIE;

		NdisMoveMemory(&WscIE, pData, sizeof(WSC_IE));
		/* Check for WSC IEs*/
		pWscIE = &WscIE;

		/* Check for device password ID, PBC = 0x0004*/
		if (be2cpu16(pWscIE->Type) == WSC_ID_DEVICE_PWD_ID) {
			/* Found device password ID*/
			NdisMoveMemory(&DevicePasswordID, pData + 4, sizeof(DevicePasswordID));
			DevicePasswordID = be2cpu16(DevicePasswordID);

			if (DevicePasswordID == DEV_PASS_ID_PBC) {	/* Check for PBC value*/
				WscPBC_DPID_FromSTA(pAd, Fr->Hdr.Addr2, current_band);
				hex_dump("PBC STA:", Fr->Hdr.Addr2, MAC_ADDR_LEN);
				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("\n"));
			} else if (DevicePasswordID == DEV_PASS_ID_PIN) {
				/*
				*	WSC 2.0 STA will send probe request with WPS IE anyway.
				*	Do NOT add this STA to WscPeerList after AP is triggered to do PBC.
				*/
				if (pWscCtrl &&
					(!pWscCtrl->bWscTrigger || (pWscCtrl->WscMode != WSC_PBC_MODE))) {
					RTMP_SEM_LOCK(&pWscCtrl->WscPeerListSemLock);
					WscInsertPeerEntryByMAC(&pWscCtrl->WscPeerList, Fr->Hdr.Addr2);
					RTMP_SEM_UNLOCK(&pWscCtrl->WscPeerListSemLock);
				}
			}

			else {
				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("%s : DevicePasswordID = 0x%04x\n",
						 __func__, DevicePasswordID));
			}

			break;
		}

		/* Set the offset and look for PBC information*/
		/* Since Type and Length are both short type, we need to offset 4, not 2*/
		pData += (be2cpu16(pWscIE->Length) + 4);
		Len   -= (be2cpu16(pWscIE->Length) + 4);
	}
}

VOID WscClearPeerList(
	IN  PLIST_HEADER pWscEnList)
{
	RT_LIST_ENTRY *pEntry = NULL;

	pEntry = pWscEnList->pHead;

	while (pEntry != NULL) {
		removeHeadList(pWscEnList);
		os_free_mem(pEntry);
		pEntry = pWscEnList->pHead;
	}
}

PWSC_PEER_ENTRY	WscFindPeerEntry(
	IN  PLIST_HEADER pWscEnList,
	IN  PUCHAR pMacAddr)
{
	PWSC_PEER_ENTRY	pPeerEntry = NULL;
	RT_LIST_ENTRY *pListEntry = NULL;

	pListEntry = pWscEnList->pHead;
	pPeerEntry = (PWSC_PEER_ENTRY)pListEntry;

	while (pPeerEntry != NULL) {
		if (NdisEqualMemory(pPeerEntry->mac_addr, pMacAddr, MAC_ADDR_LEN))
			return pPeerEntry;

		pListEntry = pListEntry->pNext;
		pPeerEntry = (PWSC_PEER_ENTRY)pListEntry;
	}

	return NULL;
}

VOID WscInsertPeerEntryByMAC(
	IN  PLIST_HEADER pWscEnList,
	IN  PUCHAR pMacAddr)
{
	PWSC_PEER_ENTRY pWscPeer = NULL;

	pWscPeer = WscFindPeerEntry(pWscEnList, pMacAddr);

	if (pWscPeer)
		NdisGetSystemUpTime(&pWscPeer->receive_time);
	else {
		os_alloc_mem(NULL, (UCHAR **)&pWscPeer, sizeof(WSC_PEER_ENTRY));

		if (pWscPeer) {
			NdisZeroMemory(pWscPeer, sizeof(WSC_PEER_ENTRY));
			pWscPeer->pNext = NULL;
			NdisMoveMemory(pWscPeer->mac_addr, pMacAddr, MAC_ADDR_LEN);
			NdisGetSystemUpTime(&pWscPeer->receive_time);
			insertTailList(pWscEnList, (RT_LIST_ENTRY *)pWscPeer);
		}

		ASSERT(pWscPeer != NULL);
	}
}

#ifdef CONFIG_AP_SUPPORT
INT WscApShowPeerList(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR ApIdx = 0;
	PWSC_CTRL pWscControl = NULL;
	PWSC_PEER_ENTRY	pPeerEntry = NULL;
	RT_LIST_ENTRY *pListEntry = NULL;
	PLIST_HEADER pWscEnList = NULL;

	for (ApIdx = 0; ApIdx < pAd->ApCfg.BssidNum; ApIdx++) {
		pWscControl = &pAd->ApCfg.MBSSID[ApIdx].wdev.WscControl;
		pWscEnList = &pWscControl->WscPeerList;

		if (pWscEnList->size != 0) {
			WscMaintainPeerList(pAd, pWscControl);
			RTMP_SEM_LOCK(&pWscControl->WscPeerListSemLock);
			pListEntry = pWscEnList->pHead;
			pPeerEntry = (PWSC_PEER_ENTRY)pListEntry;

			while (pPeerEntry != NULL) {
				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_OFF,
					("MAC:%02x:%02x:%02x:%02x:%02x:%02x\tReveive Time:%lu\n",
					   pPeerEntry->mac_addr[0],
					   pPeerEntry->mac_addr[1],
					   pPeerEntry->mac_addr[2],
					   pPeerEntry->mac_addr[3],
					   pPeerEntry->mac_addr[4],
					   pPeerEntry->mac_addr[5],
					   pPeerEntry->receive_time));
				pListEntry = pListEntry->pNext;
				pPeerEntry = (PWSC_PEER_ENTRY)pListEntry;
			}

			RTMP_SEM_UNLOCK(&pWscControl->WscPeerListSemLock);
		}

		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_OFF, ("\n"));
	}

	return TRUE;
}

INT WscApShowPin(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR inf_idx = pObj->ioctl_if;
	BOOLEAN bFromApCli = FALSE;
	PWSC_CTRL pWscControl;

#ifdef APCLI_SUPPORT
	if (pObj->ioctl_if_type == INT_APCLI) {
		UCHAR idx_2;

		bFromApCli = TRUE;
		pWscControl = &pAd->ApCfg.ApCliTab[inf_idx].wdev.WscControl;
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_OFF,
			("IF(apcli%d) WPS Information:\n", inf_idx));

		if (pWscControl->WscEnrolleePinCodeLen == 8)
			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_OFF,
			("Enrollee PinCode(ApCli%d)        %08u\n",
				   inf_idx,
				   pWscControl->WscEnrolleePinCode));
		else
			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_OFF,
			("Enrollee PinCode(ApCli%d)        %04u\n",
				   inf_idx,
				   pWscControl->WscEnrolleePinCode));

		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_OFF,
			("Ap Client WPS Profile Count     = %d\n", pWscControl->WscProfile.ProfileCnt));

		for (idx_2 = 0; idx_2 < pWscControl->WscProfile.ProfileCnt; idx_2++) {
			PWSC_CREDENTIAL pCredential = &pWscControl->WscProfile.Profile[idx_2];

			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_OFF,
				("Profile[%d]:\n", idx_2));
			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_OFF,
				("SSID                            = %s\n", pCredential->SSID.Ssid));
			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_OFF,
				("AuthType                        = %s\n", WscGetAuthTypeStr(pCredential->AuthType)));
			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_OFF,
				("EncrypType                      = %s\n", WscGetEncryTypeStr(pCredential->EncrType)));
			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_OFF,
				("KeyIndex                        = %d\n", pCredential->KeyIndex));

			if (pCredential->KeyLength != 0)
				MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_OFF,
				("Key                             = %s\n", pCredential->Key));
		}
	} else
#endif /* APCLI_SUPPORT */
	{
		bFromApCli = FALSE;
		pWscControl = &pAd->ApCfg.MBSSID[inf_idx].wdev.WscControl;
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE,
			("IF(ra%d) WPS Information:\n", inf_idx));
#ifdef BB_SOC
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE,
			("WPS Wsc2MinsTimerRunning(ra%d)        = %d\n",
			   inf_idx, pAd->ApCfg.MBSSID[inf_idx].wdev.WscControl.Wsc2MinsTimerRunning));
#else
		/* display pin code */
		if (pAd->ApCfg.MBSSID[inf_idx].wdev.WscControl.WscEnrolleePinCodeLen == 8)
			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE,
				("Enrollee PinCode(ra%d)           %08u\n",
				   inf_idx, pAd->ApCfg.MBSSID[inf_idx].wdev.WscControl.WscEnrolleePinCode));
		else
			MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE,
				("Enrollee PinCode(ra%d)           %04u\n",
				inf_idx, pAd->ApCfg.MBSSID[inf_idx].wdev.WscControl.WscEnrolleePinCode));
#endif
	}

	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE, ("\n"));
	return TRUE;
}
#endif /* CONFIG_AP_SUPPORT */


VOID WscMaintainPeerList(
	IN  PRTMP_ADAPTER pAd,
	IN  PWSC_CTRL pWpsCtrl)
{
	PWSC_PEER_ENTRY	pPeerEntry = NULL;
	RT_LIST_ENTRY *pListEntry = NULL, *pTempListEntry = NULL;
	PLIST_HEADER pWscEnList = NULL;
	ULONG now_time = 0;

	RTMP_SEM_LOCK(&pWpsCtrl->WscPeerListSemLock);
	pWscEnList = &pWpsCtrl->WscPeerList;
	NdisGetSystemUpTime(&now_time);
	pListEntry = pWscEnList->pHead;
	pPeerEntry = (PWSC_PEER_ENTRY)pListEntry;

	while (pPeerEntry != NULL) {
		if (RTMP_TIME_AFTER(now_time, pPeerEntry->receive_time + (30 * OS_HZ))) {
			pTempListEntry = pListEntry->pNext;
			delEntryList(pWscEnList, pListEntry);
			os_free_mem(pPeerEntry);
			pListEntry = pTempListEntry;
		} else
			pListEntry = pListEntry->pNext;

		pPeerEntry = (PWSC_PEER_ENTRY)pListEntry;
	}

	RTMP_SEM_UNLOCK(&pWpsCtrl->WscPeerListSemLock);
}

VOID WscDelListEntryByMAC(
	IN  PLIST_HEADER pWscEnList,
	IN  PUCHAR pMacAddr)
{
	RT_LIST_ENTRY *pListEntry = NULL;

	pListEntry = (RT_LIST_ENTRY *)WscFindPeerEntry(pWscEnList, pMacAddr);

	if (pListEntry) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_TRACE,
			("WscDelListEntryByMAC : pMacAddr = %02X:%02X:%02X:%02X:%02X:%02X\n", PRINT_MAC(pMacAddr)));
		delEntryList(pWscEnList, pListEntry);
		os_free_mem(pListEntry);
	}
}

VOID	WscAssignEntryMAC(
	IN  PRTMP_ADAPTER pAd,
	IN  PWSC_CTRL pWpsCtrl)
{
	PWSC_PEER_ENTRY pPeerEntry = NULL;

	WscMaintainPeerList(pAd, pWpsCtrl);
	RTMP_SEM_LOCK(&pWpsCtrl->WscPeerListSemLock);
	pPeerEntry = (PWSC_PEER_ENTRY)pWpsCtrl->WscPeerList.pHead;
	NdisZeroMemory(pWpsCtrl->EntryAddr, MAC_ADDR_LEN);

	if (pPeerEntry)
		NdisMoveMemory(pWpsCtrl->EntryAddr, pPeerEntry->mac_addr, MAC_ADDR_LEN);

	RTMP_SEM_UNLOCK(&pWpsCtrl->WscPeerListSemLock);
}


/*
*	Get WSC IE data from WSC Peer by Tag.
*/
BOOLEAN WscGetDataFromPeerByTag(
	IN  PRTMP_ADAPTER pAd,
	IN  PUCHAR pIeData,
	IN  INT IeDataLen,
	IN  USHORT WscTag,
	OUT PUCHAR pWscBuf,
	OUT PUSHORT pWscBufLen)
{
	PUCHAR pData = pIeData;
	INT Len = 0;
	USHORT DataLen = 0;
	PWSC_IE pWscIE;

	Len = IeDataLen;

	while (Len > 0) {
		WSC_IE	WscIE;

		NdisMoveMemory(&WscIE, pData, sizeof(WSC_IE));
		/* Check for WSC IEs */
		pWscIE = &WscIE;

		if (be2cpu16(pWscIE->Type) == WscTag) {
			DataLen = be2cpu16(pWscIE->Length);

			if (pWscBufLen)
				*pWscBufLen = DataLen;

			NdisMoveMemory(pWscBuf, pData + 4, DataLen);
			return TRUE;
		}

		/* Set the offset and look for next WSC Tag information */
		/* Since Type and Length are both short type, we need to offset 4, not 2 */
		pData += (be2cpu16(pWscIE->Length) + 4);
		Len   -= (be2cpu16(pWscIE->Length) + 4);
	}

	return FALSE;
}

VOID WscUUIDInit(
	IN  PRTMP_ADAPTER pAd,
	IN  INT inf_idx,
	IN  UCHAR from_apcli)
{
#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT

	if (from_apcli) {
		PWSC_CTRL pWpsCtrl = &pAd->ApCfg.ApCliTab[inf_idx].wdev.WscControl;
#ifdef WSC_V2_SUPPORT
		PWSC_V2_INFO	pWscV2Info;
#endif /* WSC_V2_SUPPORT */

		NdisZeroMemory(pWpsCtrl->EntryAddr, MAC_ADDR_LEN);
#ifdef WSC_V2_SUPPORT
		pWpsCtrl->WscConfigMethods = 0x238C;
#else /* WSC_V2_SUPPORT */
		pWpsCtrl->WscConfigMethods = 0x018C;
#endif /* !WSC_V2_SUPPORT */
		WscGenerateUUID(pAd, &pWpsCtrl->Wsc_Uuid_E[0],
						&pWpsCtrl->Wsc_Uuid_Str[0], 0, FALSE, from_apcli);
		pWpsCtrl->bWscFragment = FALSE;
		pWpsCtrl->WscFragSize = 128;
		pWpsCtrl->WscRxBufLen = 0;

		if (pWpsCtrl->pWscRxBuf) {
			os_free_mem(pWpsCtrl->pWscRxBuf);
			pWpsCtrl->pWscRxBuf = NULL;
		}

		os_alloc_mem(pAd, &pWpsCtrl->pWscRxBuf, MAX_MGMT_PKT_LEN);

		if (pWpsCtrl->pWscRxBuf)
			NdisZeroMemory(pWpsCtrl->pWscRxBuf, MAX_MGMT_PKT_LEN);

		pWpsCtrl->WscTxBufLen = 0;

		if (pWpsCtrl->pWscTxBuf) {
			os_free_mem(pWpsCtrl->pWscTxBuf);
			pWpsCtrl->pWscTxBuf = NULL;
		}

		os_alloc_mem(pAd, &pWpsCtrl->pWscTxBuf, MAX_MGMT_PKT_LEN);

		if (pWpsCtrl->pWscTxBuf)
			NdisZeroMemory(pWpsCtrl->pWscTxBuf, MAX_MGMT_PKT_LEN);

		initList(&pWpsCtrl->WscPeerList);
		NdisAllocateSpinLock(pAd, &pWpsCtrl->WscPeerListSemLock);
		pWpsCtrl->PinAttackCount = 0;
		pWpsCtrl->bSetupLock = FALSE;
#ifdef WSC_V2_SUPPORT
		pWscV2Info = &pWpsCtrl->WscV2Info;
		pWscV2Info->bWpsEnable = TRUE;
		pWscV2Info->ExtraTlv.TlvLen = 0;
		pWscV2Info->ExtraTlv.TlvTag = 0;
		pWscV2Info->ExtraTlv.pTlvData = NULL;
		pWscV2Info->ExtraTlv.TlvType = TLV_ASCII;
		pWscV2Info->bEnableWpsV2 = TRUE;
#endif /* WSC_V2_SUPPORT */
		WscInit(pAd, TRUE, inf_idx);
	} else
#endif /* APCLI_SUPPORT */
	{
#ifdef HOSTAPD_SUPPORT

		if (pAd->ApCfg.MBSSID[inf_idx].Hostapd == Hostapd_EXT)
			MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[ra%d] WPS is control by hostapd now.\n", inf_idx));
		else
#endif /*HOSTAPD_SUPPORT*/
		{
			PWSC_CTRL pWscControl;
			UCHAR zeros16[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

			pWscControl = &pAd->ApCfg.MBSSID[inf_idx].wdev.WscControl;
			MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Generate UUID for apidx(%d)\n", inf_idx));

			if (NdisEqualMemory(&pWscControl->Wsc_Uuid_E[0], zeros16, UUID_LEN_HEX))
				WscGenerateUUID(pAd, &pWscControl->Wsc_Uuid_E[0], &pWscControl->Wsc_Uuid_Str[0], inf_idx, FALSE, from_apcli);

			WscInit(pAd, FALSE, inf_idx);
		}
	}

#endif /* CONFIG_AP_SUPPORT */
}

#endif /* WSC_INCLUDED */
