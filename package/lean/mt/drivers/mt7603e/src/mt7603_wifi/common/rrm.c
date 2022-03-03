/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
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
 ***************************************************************************/

/****************************************************************************
	Abstract:

***************************************************************************/

#ifdef DOT11K_RRM_SUPPORT

#include "rt_config.h"

/*
static CHAR ZeroSsid[32] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
*/


static VOID RRM_QuietOffsetTimeout(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3);

static VOID RRM_QuietTimeout(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3);

DECLARE_TIMER_FUNCTION(RRM_QuietOffsetTimeout);
DECLARE_TIMER_FUNCTION(RRM_QuietTimeout);

BUILD_TIMER_FUNCTION(RRM_QuietOffsetTimeout);
BUILD_TIMER_FUNCTION(RRM_QuietTimeout);



static VOID RRM_QuietOffsetTimeout(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3)
{
	INT idx;
	PRRM_CONFIG pRrmCfg = NULL;
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)FunctionContext;
	PRALINK_TIMER_STRUCT pTimer = (PRALINK_TIMER_STRUCT) SystemSpecific3;

	for (idx = 0; idx < pAd->ApCfg.BssidNum; idx++)
	{
		pRrmCfg = &pAd->ApCfg.MBSSID[idx].RrmCfg;
		if (&pRrmCfg->QuietCB.QuietOffsetTimer == pTimer)
			break;
	}

	/* start Quiet . */
	if (idx == pAd->ApCfg.BssidNum)
		return;

	pRrmCfg = &pAd->ApCfg.MBSSID[idx].RrmCfg;
	pRrmCfg->QuietCB.QuietState = RRM_QUIET_SILENT;
	RTMPSetTimer(&pRrmCfg->QuietCB.QuietTimer,
		pRrmCfg->QuietCB.QuietDuration);

	return;
}

static VOID RRM_QuietTimeout(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3)
{
	INT idx;
	PRRM_CONFIG pRrmCfg = NULL;
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)FunctionContext;
	PRALINK_TIMER_STRUCT pTimer = (PRALINK_TIMER_STRUCT) SystemSpecific3;

	for (idx = 0; idx < pAd->ApCfg.BssidNum; idx++)
	{
		pRrmCfg = &pAd->ApCfg.MBSSID[idx].RrmCfg;
		if (&pRrmCfg->QuietCB.QuietTimer == pTimer)
			break;
	}

	/* stop Quiet . */
	if (idx == pAd->ApCfg.BssidNum)
		return;

	pRrmCfg = &pAd->ApCfg.MBSSID[idx].RrmCfg;
	pRrmCfg->QuietCB.QuietState = RRM_QUIET_IDLE;

	return;
}

void RRM_ReadParametersFromFile(
	IN PRTMP_ADAPTER pAd,
	RTMP_STRING *tmpbuf,
	RTMP_STRING *buffer)
{
	INT loop;
	RTMP_STRING *macptr;

	/* RRMEnable */
	if (RTMPGetKeyParameter("RRMEnable", tmpbuf, 255, buffer, TRUE))
	{
		for (loop=0, macptr = rstrtok(tmpbuf,";");
				(macptr && loop < MAX_MBSSID_NUM(pAd));
					macptr = rstrtok(NULL,";"), loop++)
		{
			LONG Enable;
			Enable = simple_strtol(macptr, 0, 10);
			pAd->ApCfg.MBSSID[loop].RrmCfg.bDot11kRRMEnable =
				(Enable > 0) ? TRUE : FALSE;
			DBGPRINT(RT_DEBUG_TRACE, ("%s::(bDot11kRRMEnable[%d]=%d)\n",
				__FUNCTION__, loop,
				pAd->ApCfg.MBSSID[loop].RrmCfg.bDot11kRRMEnable));
		}
	}
	else 
	{
		for (loop = 0; loop < MAX_MBSSID_NUM(pAd); loop++)
			pAd->ApCfg.MBSSID[loop].RrmCfg.bDot11kRRMEnable = FALSE;
	}

	/* Regulatory Class */
	if (RTMPGetKeyParameter("RegulatoryClass", tmpbuf, 255, buffer, TRUE))
	{	
		LONG Value;
		for (loop=0, macptr = rstrtok(tmpbuf,";");
				(macptr && loop < MAX_NUM_OF_REGULATORY_CLASS);
					macptr = rstrtok(NULL,";"), loop++)
		{
			Value = simple_strtol(macptr, 0, 10);
			pAd->CommonCfg.RegulatoryClass[loop] = Value;
		}
		
	}
		if (RTMPGetKeyParameter("RegDomain", tmpbuf, 255, buffer, TRUE)) {
			if (!strncmp(tmpbuf, "Global", 6))
				pAd->reg_domain = REG_GLOBAL;
			else
				pAd->reg_domain = REG_LOCAL;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("pAd->reg_domain = %u\n",
			pAd->reg_domain));
		}

	return;
}

UINT16 bcn_rand_int;
INT Set_BeaconReq_RandInt_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	bcn_rand_int = (UINT) simple_strtol(arg, 0, 10);
	return TRUE;
}

INT Set_Dot11kRRM_Enable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR ifIndex = pObj->ioctl_if;
	ULONG Value;

	if (ifIndex >= pAd->ApCfg.BssidNum)
	{
		DBGPRINT(RT_DEBUG_OFF, ("Unknow If index (%d)", ifIndex));
		return -1;
	}

	Value = (UINT) simple_strtol(arg, 0, 10);
	pAd->ApCfg.MBSSID[ifIndex].RrmCfg.bDot11kRRMEnable =
		(BOOLEAN)(Value) == 0 ? FALSE : TRUE;

	if(pAd->ApCfg.MBSSID[ifIndex].RrmCfg.bDot11kRRMEnable)
		pAd->ApCfg.MBSSID[ifIndex].CapabilityInfo |= RRM_CAP_BIT;
	else
		pAd->ApCfg.MBSSID[ifIndex].CapabilityInfo &= ~RRM_CAP_BIT;

	return 1;
}

INT Set_BeaconReq_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT Loop;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR ifIndex = pObj->ioctl_if;
	UINT Aid = 1;
	UINT ArgIdx;
	RTMP_STRING *thisChar;

	RRM_MLME_BCN_REQ_INFO BcnReq;
	UINT8 default_request_ie[MAX_NUM_OF_REQ_IE] = {
							0,		/* SSID */
							1, /* Support Rate*/

							50, /* Extended Support Rate*/

							45, /* HT IE*/
							61, /* HT ADD*/

							127, /* Ext Cap*/

							191, /* VHT 1*/
							192, /* VHT 2*/
							195, /* VHT 3*/

							48,		/* RSN IE */
							70,		/* RRM Capabilities. */
							54,		/* Mobility Domain. */
							221
						};	/* Vendor Specific. */

	ArgIdx = 0;
	NdisZeroMemory(&BcnReq, sizeof(RRM_MLME_BCN_REQ_INFO));
	BcnReq.report_detail = 1; /*set default report detail to 1*/
	/*set default request ie*/
	BcnReq.request_ie_num = MAX_NUM_OF_REQ_IE;
	NdisCopyMemory(BcnReq.request_ie, default_request_ie, MAX_NUM_OF_REQ_IE);

	while ((thisChar = strsep((char **)&arg, "!")) != NULL)
	{
		switch(ArgIdx)
		{
			case 0:/* Aid or Peer's MAC Addr */
				if (strlen(thisChar) == 17) {
					UCHAR mac_addr[MAC_ADDR_LEN];
					CHAR *token;
					MAC_TABLE_ENTRY *pEntry = NULL;
					INT i = 0;

					token = rstrtok(thisChar, ":");
					while (token != NULL) {
						AtoH(token, (char *) &mac_addr[i], 1);
						i++;
						if (i >= MAC_ADDR_LEN)
							break;
						token = rstrtok(NULL, ":");
				}
					pEntry = MacTableLookup(pAd, mac_addr);
					if (pEntry != NULL)
						Aid = pEntry->wcid;
				} else {
					Aid = (UINT8) simple_strtol(thisChar, 0, 16);
				}
				if (!VALID_WCID(Aid))
				{
					DBGPRINT(RT_DEBUG_ERROR, ("%s: unknow sta of Aid(%d)\n", __FUNCTION__, Aid));
					return TRUE;
				}
				break;

			case 1: /* Meausre Duration. */
				BcnReq.MeasureDuration = (UINT8) simple_strtol(thisChar, 0, 10);
				break;

			case 2: /* Regulator Class */
				BcnReq.RegulatoryClass = (UINT8) simple_strtol(thisChar, 0, 10);
				break;

			case 3: /* BSSID */
				if(strlen(thisChar) != 17)
				{
					DBGPRINT(RT_DEBUG_ERROR,
						("%s: invalid value BSSID.\n", 	__FUNCTION__));
					return TRUE;
				}

				if(strlen(thisChar) == 17)  /*Mac address acceptable format 01:02:03:04:05:06 length 17 */
				{
					RTMP_STRING *value;
					for (Loop=0, value = rstrtok(thisChar,":"); value; value = rstrtok(NULL,":"))
					{
						if((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value+1))) ) 
							return FALSE;  /*Invalid */

						AtoH(value, &BcnReq.Bssid[Loop++], 1);
					}

					if(Loop != 6)
						return TRUE;
				}
				break;

			case 4: /* SSID */
				BcnReq.pSsid = (PUINT8)thisChar;
				BcnReq.SsidLen = strlen(thisChar);
				break;

			case 5: /* measure channel */
				BcnReq.MeasureCh = (UINT8) simple_strtol(thisChar, 0, 10);
				break;

			case 6: /* measure mode. */
				BcnReq.MeasureMode = (UINT8) simple_strtol(thisChar, 0, 10);
								break;
			case 7: /* regulatory class. */
				{
					RTMP_STRING *RegClassString;
					int RegClassIdx;

					RegClassIdx = 0;
					while ((RegClassString = strsep((char **)&thisChar, "+")) != NULL)
					{
						BcnReq.ChRepRegulatoryClass[RegClassIdx] =
							(UINT8) simple_strtol(RegClassString, 0, 10);
						RegClassIdx++;
					}
				}
				break;
			case 8:{/* Channel Report  List. */
				RTMP_STRING *ChIdString;
				int ChId;

				ChId = 0;

				while ((ChIdString = strsep((char **)&thisChar, "#")) != NULL) {
					BcnReq.ChRepList[ChId] = (UINT8) simple_strtol(ChIdString, 0, 10);
					ChId++;
				}
			}
			break;
			case 9:/* report detail*/
				BcnReq.report_detail = (UINT8) simple_strtol(thisChar, 0, 10);
				break;
			case 10: {/* request ie */
				RTMP_STRING *req_ie_str;

				BcnReq.request_ie_num = 0;

				while ((req_ie_str = strsep((char **)&thisChar, "#")) != NULL) {
					if (req_ie_str[0] == '\0')
						break;

					BcnReq.request_ie[BcnReq.request_ie_num] =
						(UINT8) simple_strtol(req_ie_str, 0, 10);
					BcnReq.request_ie_num++;
				}

				if (BcnReq.request_ie_num == 0) {
					/*if request ie list is empty, add SSID ie to it*/
					BcnReq.request_ie_num = 1;
					BcnReq.request_ie[0] = 0;
				}
			}
			break;
		}
		ArgIdx++;
	}	

	if (ArgIdx < 7 || ArgIdx > 11)
	{
		DBGPRINT(RT_DEBUG_ERROR,
			("%s: invalid args (%d).\n", __FUNCTION__, ArgIdx));
		DBGPRINT(RT_DEBUG_ERROR,
			("eg: iwpriv ra0 set BcnReq=<Aid>-<Duration>-<RegulatoryClass>-<BSSID>-<SSID>-<MeasureCh>-<MeasureMode>-<ChRegClass>\n"));
		DBGPRINT(RT_DEBUG_ERROR,
				 ("eg: iwpriv ra0 set BcnReq=1!50!12!FF:FF:FF:FF:FF:FF!WiFi1!255!1!32+1!1#6#36#48\n"));
		return TRUE;
	}


	BcnReq.BcnReqCapFlag.field.ReportCondition = TRUE;
	/* Add MeasureMode Sanity check of peer STA */
	if (VALID_UCAST_ENTRY_WCID(Aid)) {
		PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[Aid];

		if (!pEntry) {
			DBGPRINT(RT_DEBUG_ERROR, ("%s: pEntry is  NULL!\n", __func__));
			return FALSE;
		}

		switch (BcnReq.MeasureMode) {
		case RRM_BCN_REQ_MODE_PASSIVE:
			if (!IS_RRM_BEACON_PASSIVE_MEASURE(pEntry)) {
				DBGPRINT(RT_DEBUG_ERROR,
					("%s: invalid Measure Mode. %d, Peer STA Support(PASS:%d, ACT:%d, TAB:%d)!\n",
					__func__, BcnReq.MeasureMode, IS_RRM_BEACON_PASSIVE_MEASURE(pEntry),
					IS_RRM_BEACON_MEASURE(pEntry), IS_RRM_BEACON_TABLE_MEASURE(pEntry)));
					/* return TRUE; */
			}
			break;

		case RRM_BCN_REQ_MODE_ACTIVE:
			if (!IS_RRM_BEACON_MEASURE(pEntry)) {
				DBGPRINT(RT_DEBUG_ERROR,
					("%s: invalid Measure Mode. %d, Peer STA Support(PASS:%d, ACT:%d, TAB:%d)!\n",
					__func__, BcnReq.MeasureMode, IS_RRM_BEACON_PASSIVE_MEASURE(pEntry),
					IS_RRM_BEACON_MEASURE(pEntry), IS_RRM_BEACON_TABLE_MEASURE(pEntry)));
				/* return TRUE; */
			}
			break;

		case RRM_BCN_REQ_MODE_BCNTAB:
			if (!IS_RRM_BEACON_TABLE_MEASURE(pEntry)) {
				DBGPRINT(RT_DEBUG_ERROR,
				("%s: invalid Measure Mode. %d, Peer STA Support(PASS:%d, ACT:%d, TAB:%d)!\n",
				__func__, BcnReq.MeasureMode, IS_RRM_BEACON_PASSIVE_MEASURE(pEntry),
				IS_RRM_BEACON_MEASURE(pEntry), IS_RRM_BEACON_TABLE_MEASURE(pEntry)));
					/* return TRUE; */
			}
			break;

		default:
			{
			DBGPRINT(RT_DEBUG_ERROR,
				("%s: invalid Measure Mode. %d\n",	__func__, BcnReq.MeasureMode));
				/* return TRUE; */
			}
				break;
		}
	}
	if (BcnReq.MeasureCh == 255)
		BcnReq.BcnReqCapFlag.field.ChannelRep = TRUE;
	else
		BcnReq.BcnReqCapFlag.field.ChannelRep = FALSE;
	BcnReq.RandInt = bcn_rand_int;
	RRM_EnqueueBcnReq(pAd, Aid, ifIndex, &BcnReq);

	return TRUE;
}

INT Set_LinkMeasureReq_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR ifIndex = pObj->ioctl_if;
	UINT Aid = 1;

	Aid = simple_strtol(arg, 0, 10);
	RRM_EnqueueLinkMeasureReq(pAd, Aid, ifIndex);

	return TRUE;
}

INT Set_TxStreamMeasureReq_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR ifIndex = pObj->ioctl_if;
	UINT Aid = 1;
	UINT ArgIdx;
	RTMP_STRING *thisChar;

	RRM_MLME_TRANSMIT_REQ_INFO TransmitReq;
	PMAC_TABLE_ENTRY pMacEntry;

	ArgIdx = 0;
	NdisZeroMemory(&TransmitReq, sizeof(RRM_MLME_TRANSMIT_REQ_INFO));

	while ((thisChar = strsep((char **)&arg, "-")) != NULL)
	{
		switch(ArgIdx)
		{
			case 0:	/* Aid. */
				Aid = (UINT8) simple_strtol(thisChar, 0, 10);
				if (!VALID_WCID(Aid))
				{
					DBGPRINT(RT_DEBUG_ERROR, ("%s: unknow sta of Aid(%d)\n", __FUNCTION__, Aid));
					return TRUE;
				}
				break;

			case 1: /* DurationMandotory. */
				TransmitReq.bDurationMandatory =
					((UINT16)simple_strtol(thisChar, 0, 10) > 0 ? TRUE : FALSE);
				break;

			case 2: /* Measure Duration */
				TransmitReq.MeasureDuration = (UINT16)simple_strtol(thisChar, 0, 10);
				break;

			case 3: /* TID */
				TransmitReq.Tid = (UINT8) simple_strtol(thisChar, 0, 10);
				break;

			case 4: /* Bin 0 Range */
				TransmitReq.BinRange = (UINT8) simple_strtol(thisChar, 0, 10);
				break;

			case 5: /* Averange Condition */
				TransmitReq.ArvCondition =
					((UINT8) simple_strtol(thisChar, 0, 10)) > 0 ? 1 : 0;
				break;

			case 6: /* Consecutive Condition */
				TransmitReq.ConsecutiveCondition =
					((UINT8) simple_strtol(thisChar, 0, 10)) > 0 ? 1 : 0;
				break;

			case 7: /* Delay Condition */
				TransmitReq.DelayCondition =
					((UINT8) simple_strtol(thisChar, 0, 10)) > 0 ? 1 : 0;
				break;

			case 8: /* Averange Error Threshold */
				TransmitReq.AvrErrorThreshold =
					(UINT8) simple_strtol(thisChar, 0, 10);
				break;

			case 9: /* Consecutive Error Threshold */
				TransmitReq.ConsecutiveErrorThreshold =
					(UINT8) simple_strtol(thisChar, 0, 10);
				break;

			case 10: /* Delay Threshold */
				TransmitReq.DelayThreshold =
					(UINT8) simple_strtol(thisChar, 0, 10);
				break;

			case 11: /* Measure counter  */
				TransmitReq.MeasureCnt =
					(UINT8) simple_strtol(thisChar, 0, 10);

				break;

			case 12: /* Trigger time out */
				TransmitReq.TriggerTimeout =
					(UINT8) simple_strtol(thisChar, 0, 10);
				break;
			
		}
		ArgIdx++;
	}	

	if ((ArgIdx != 13) && (ArgIdx != 5))
	{
		DBGPRINT(RT_DEBUG_ERROR,
			("%s: invalid args (%d).\n", __FUNCTION__, ArgIdx));
		DBGPRINT(RT_DEBUG_ERROR,
			("eg: iwpriv ra0 set txreq=<Aid>-<DurationMandortory>-<Duration>-<TID>-<BinRange>[-<AvrCond>-<ConsecutiveCond>-<DealyCond>-<AvrErrorThreshold>-<ConsecutiveErrorThreshold>-<DelayThreshold>-<MeasureCnt>-<TriggerTimeout>]\n"));
		return TRUE;
	}

	if (ArgIdx == 5)
		TransmitReq.bTriggerReport = 0;
	else
		TransmitReq.bTriggerReport = 1;

	pMacEntry = &pAd->MacTab.Content[Aid];
	DBGPRINT(RT_DEBUG_ERROR, ("%s::Aid=%d, PeerMac=%02x:%02x:%02x:%02x:%02x:%02x\n",
		__FUNCTION__, Aid,	pMacEntry->Addr[0], pMacEntry->Addr[1],
		pMacEntry->Addr[2], pMacEntry->Addr[3], pMacEntry->Addr[4], pMacEntry->Addr[5]));

	DBGPRINT(RT_DEBUG_ERROR, ("Duration=%d, Tid=%d, Bin 0 Range=%d\n",
		TransmitReq.MeasureDuration, TransmitReq.Tid, TransmitReq.BinRange));

	DBGPRINT(RT_DEBUG_ERROR, ("ArvCondition=%d, ConsecutiveCondition=%d, DelayCondition=%d\n",
		TransmitReq.ArvCondition, TransmitReq.ConsecutiveCondition, TransmitReq.DelayCondition));

	DBGPRINT(RT_DEBUG_ERROR, ("AvrErrorThreshold=%d, ConsecutiveErrorThreshold=%d\n",
		TransmitReq.AvrErrorThreshold, TransmitReq.ConsecutiveErrorThreshold));

	DBGPRINT(RT_DEBUG_ERROR, ("DelayThreshold=%d\n", TransmitReq.DelayThreshold));

	DBGPRINT(RT_DEBUG_ERROR, ("MeasureCnt=%d, TriggerTimeout=%d\n",
		TransmitReq.MeasureCnt, TransmitReq.TriggerTimeout));

	RRM_EnqueueTxStreamMeasureReq(pAd, Aid, ifIndex, &TransmitReq);

	return TRUE;
}

INT Set_RRM_Selftest_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT Cmd = 1;

	PMAC_TABLE_ENTRY pEntry;
	UCHAR StaAddr[MAC_ADDR_LEN] = {0x00,0x0c,0x43,0x00,0x00,0x00};
	struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[0/*pEntry->func_tb_idx*/].wdev;

	Cmd = simple_strtol(arg, 0, 10);

	switch(Cmd)
	{
		case 1: /* insert a STA for RRM Beacon Report request testing. */
			pEntry = MacTableInsertEntry(pAd, StaAddr, wdev, ENTRY_CLIENT, OPMODE_AP, TRUE);
			pEntry->Sst = SST_ASSOC;
			pEntry->CapabilityInfo |= RRM_CAP_BIT;
			pEntry->RrmEnCap.field.BeaconActiveMeasureCap = 1;
			break;
	}
	return TRUE;
}

INT RRM_InfoDisplay_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT loop;

	for (loop = 0; loop < pAd->ApCfg.BssidNum; loop++)
	{
		DBGPRINT(RT_DEBUG_OFF, ("%d: bDot11kRRMEnable=%d\n",
			loop, pAd->ApCfg.MBSSID[loop].RrmCfg.bDot11kRRMEnable));
	}

	DBGPRINT(RT_DEBUG_OFF, ("Country Code=%s\n",
		pAd->CommonCfg.CountryCode));

	DBGPRINT(RT_DEBUG_OFF, ("Power Constraint=%d\n",
		pAd->CommonCfg.PwrConstraint));

	DBGPRINT(RT_DEBUG_OFF, ("Regulator Class="));
	for (loop = 0; loop < MAX_NUM_OF_REGULATORY_CLASS; loop++)
	{
		if (pAd->CommonCfg.RegulatoryClass[loop] == 0)
			break;

		DBGPRINT(RT_DEBUG_OFF, ("%d ",
			pAd->CommonCfg.RegulatoryClass[loop]));
	}
	DBGPRINT(RT_DEBUG_OFF, ("\n"));	

	DBGPRINT(RT_DEBUG_OFF, ("Regulator TxPowerPercentage=%ld\n",
		pAd->CommonCfg.TxPowerPercentage));
	return TRUE;
}

VOID RRM_CfgInit(
	IN PRTMP_ADAPTER pAd)
{
	INT loop;
	PRRM_CONFIG pRrmCfg;

	for (loop = 0; loop < MAX_MBSSID_NUM(pAd); loop++)
	{
		pRrmCfg = &pAd->ApCfg.MBSSID[loop].RrmCfg;
		pRrmCfg->QuietCB.QuietPeriod = RRM_DEFAULT_QUIET_PERIOD;
		pRrmCfg->QuietCB.QuietDuration = RRM_DEFAULT_QUIET_DURATION;
		pRrmCfg->QuietCB.QuietOffset = RRM_DEFAULT_QUIET_OFFSET;

		RTMPInitTimer(pAd, &pRrmCfg->QuietCB.QuietOffsetTimer, GET_TIMER_FUNCTION(RRM_QuietOffsetTimeout), pAd, FALSE);
		RTMPInitTimer(pAd, &pRrmCfg->QuietCB.QuietTimer, GET_TIMER_FUNCTION(RRM_QuietTimeout), pAd, FALSE);

		pRrmCfg->QuietCB.QuietState = RRM_QUIET_IDLE;
		pRrmCfg->QuietCB.CurAid = 1;
		pRrmCfg->bDot11kRRMNeighborRepTSFEnable = FALSE;
	}

	return;
}

VOID RRM_QuietUpdata(
	IN PRTMP_ADAPTER pAd)
{
	INT loop;
	PRRM_CONFIG pRrmCfg;

	BSS_STRUCT *mbss;
	struct wifi_dev *wdev;
	for (loop = 0; loop < pAd->ApCfg.BssidNum; loop++)
	{
		mbss = &pAd->ApCfg.MBSSID[loop];
		wdev = &mbss->wdev;
	
		if ((wdev->if_dev == NULL) || ((wdev->if_dev != NULL) &&
			!(RTMP_OS_NETDEV_STATE_RUNNING(wdev->if_dev))))
		{
			/* the interface is down */
			continue;
		}


		pRrmCfg = &pAd->ApCfg.MBSSID[loop].RrmCfg;
		if (pRrmCfg->bDot11kRRMEnable != TRUE)
			continue;

		if (pRrmCfg->QuietCB.QuietCnt == pRrmCfg->QuietCB.QuietPeriod)
		{
			/* issue Bcn Report Request to STAs supported RRM-Bcn Report. */
			/* Voice-enterpise doesn't require it in testing-event3 so remove it. */
			/*RRM_BcnReortQuery(pAd, loop, pRrmCfg); */
		}

		if ((pRrmCfg->QuietCB.QuietCnt == 0)
			&& (pRrmCfg->QuietCB.QuietState == RRM_QUIET_IDLE))
		{
			if (pRrmCfg->QuietCB.QuietOffset != 0)
			{
				/* Start QuietOffsetTimer. */
				RTMPSetTimer(&pRrmCfg->QuietCB.QuietOffsetTimer,
					pRrmCfg->QuietCB.QuietOffset);
			}
			else
			{
				/* Start Quiet Timer. */
				pRrmCfg->QuietCB.QuietState = RRM_QUIET_SILENT;

				RTMPSetTimer(&pRrmCfg->QuietCB.QuietTimer,
					pRrmCfg->QuietCB.QuietDuration);
			}
		}

		RRM_QUIET_CNT_DEC(pRrmCfg->QuietCB.QuietCnt, pRrmCfg->QuietCB.QuietPeriod);
	}
	return;
}

VOID RRM_PeerNeighborReqAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem) 
{
	PFRAME_802_11 pFr = (PFRAME_802_11)Elem->Msg;
	PUCHAR pFramePtr = pFr->Octet;
	PMAC_TABLE_ENTRY pEntry;
	UINT8 DialogToken;
	PCHAR pSsid = NULL;
	CHAR ssidbuf[MAX_LEN_OF_SSID+1];
	
	UINT8 SsidLen = 0;
	PMEASURE_REQ_ENTRY pMeasure_Entry = NULL;
	PNET_DEV NetDev = NULL;
	UINT32 FrameLen = 0;
	UINT8 NRReqToken = 0, APIndex = 0;

	DBGPRINT(RT_DEBUG_TRACE, ("%s::\n", __FUNCTION__));

	pEntry = MacTableLookup(pAd, pFr->Hdr.Addr2);
	if (!pEntry || (pEntry->Sst != SST_ASSOC))
		return;

	/*whether the up-layer or driver handle the request*/
	if (pAd->ApCfg.HandleNRReqbyUplayer) {
		/*send event to the up-layer */
		for (APIndex = 0; APIndex < MAX_MBSSID_NUM(pAd); APIndex++) {
			if (MAC_ADDR_EQUAL(pFr->Hdr.Addr3, pAd->ApCfg.MBSSID[APIndex].wdev.bssid)) {
				NetDev = pAd->ApCfg.MBSSID[APIndex].wdev.if_dev;
				break;
			}
		}
		if (NetDev == NULL) {
			MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("no valid NetDev! BSSID=%02x:%02x:%02x:%02x:%02x:%02x\n",
				PRINT_MAC(pFr->Hdr.Addr3)));
			return;
		}
		/* skip Category and action code. */
		pFramePtr += 2;
		NdisMoveMemory(&NRReqToken, pFramePtr, 1);
		pMeasure_Entry = MeasureReqInsert(pAd, NRReqToken);
		RTMP_SEM_LOCK(&pAd->CommonCfg.MeasureReqTabLock);
		pMeasure_Entry->skip_time_check = TRUE;
		pMeasure_Entry->CurrentState = WAIT_NR_RSP;
		pMeasure_Entry->Priv = pAd;
		pMeasure_Entry->ControlIndex = APIndex;
		memcpy(pMeasure_Entry->StaMac, pEntry->Addr, MAC_ADDR_LEN);
		RTMPInitTimer(pAd, &pMeasure_Entry->WaitNRRspTimer,
				GET_TIMER_FUNCTION(WaitNRRspTimeout), pMeasure_Entry, FALSE);
		RTMP_SEM_UNLOCK(&pAd->CommonCfg.MeasureReqTabLock);

		FrameLen = Elem->MsgLen - sizeof(HEADER_802_11) - 2;
		wext_send_nr_req_event(NetDev, pFr->Hdr.Addr2, pFramePtr, FrameLen);

		RTMPSetTimer(&pMeasure_Entry->WaitNRRspTimer, NR_RSP_TIMEOUT_VALUE);
		} else {
			if (RRM_PeerNeighborReqSanity(pAd, Elem->Msg, Elem->MsgLen, &DialogToken, &pSsid, &SsidLen)) {
				DBGPRINT(RT_DEBUG_TRACE, ("DialogToken=%x\n", DialogToken));
				snprintf(ssidbuf, sizeof(ssidbuf), "%s", pSsid);
				ssidbuf[SsidLen] = '\0';

				DBGPRINT(RT_DEBUG_TRACE, ("pSsid=%s\n", /*pSsid*/ssidbuf));
				DBGPRINT(RT_DEBUG_TRACE, ("SsidLen=%d\n", SsidLen));
				RRM_EnqueueNeighborRep(pAd, pEntry, DialogToken, pSsid, SsidLen);
			}
		}
	return;
}

VOID RRM_BeaconReportHandler(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev,
	IN PRRM_BEACON_REP_INFO pBcnRepInfo,
	IN LONG Length,
	IN PMEASURE_REQ_ENTRY pDialogEntry
#if defined(CUSTOMER_DCC_FEATURE) || defined(NEIGHBORING_AP_STAT)
			,
			IN UCHAR	*Snr
#endif
)
{
	CHAR Rssi;
	USHORT LenVIE = 0;
	NDIS_802_11_VARIABLE_IEs *pVIE = NULL;
	UCHAR VarIE[MAX_VIE_LEN];
	ULONG Idx = BSS_NOT_FOUND;
	LONG RemainLen = Length;
	PRRM_BEACON_REP_INFO pBcnRep;
	PUINT8 ptr;
	RRM_BEACON_REP_INFO_FIELD BcnReqInfoField;
	UINT32 Ptsf;
	BCN_IE_LIST *ie_list = NULL;
	PNET_DEV NetDev = NULL;
	BOOLEAN bFrameBody = FALSE;

	os_alloc_mem(NULL, (UCHAR **)&ie_list, sizeof(BCN_IE_LIST));
	if (ie_list == NULL) {
		DBGPRINT(RT_DEBUG_ERROR, ("%s(): Alloc ie_list failed!\n", __FUNCTION__));
		return;
	}
	NdisZeroMemory(ie_list, sizeof(BCN_IE_LIST));

	ptr = (PUINT8)pBcnRepInfo;

	pBcnRep = (PRRM_BEACON_REP_INFO)ptr;
#ifdef RT_BIG_ENDIAN
	pBcnRep->ActualMeasureStartTime = le2cpu64(pBcnRep->ActualMeasureStartTime);
	pBcnRep->MeasureDuration = le2cpu16(pBcnRep->MeasureDuration);
	pBcnRep->ParentTSF = le2cpu32(pBcnRep->ParentTSF);
#endif
	/* send Beacon Response to up-layer */
	NetDev = pAd->ApCfg.MBSSID[pDialogEntry->ControlIndex].wdev.if_dev;
	wext_send_bcn_rsp_event(NetDev, pDialogEntry->StaMac,
		ptr, sizeof(*pBcnRepInfo), pDialogEntry->DialogToken);

	Ptsf = pBcnRep->ParentTSF;
	BcnReqInfoField.word = pBcnRep->RepFrameInfo; 

	DBGPRINT(RT_DEBUG_TRACE, ("%s:: ReqClass=%d, Channel=%d\n",
		__FUNCTION__, pBcnRep->RegulatoryClass, pBcnRep->ChNumber));

	DBGPRINT(RT_DEBUG_TRACE, ("Bssid=%02x:%02x:%02x:%02x:%02x:%02x\n",
						PRINT_MAC(pBcnRep->Bssid)));

	Rssi = pBcnRep->RCPI + pAd->BbpRssiToDbmDelta;

	RemainLen -= sizeof(RRM_BEACON_REP_INFO);
	ptr += sizeof(RRM_BEACON_REP_INFO);

	/* check option sub element IE. */
	while (RemainLen > 0)
	{
		PRRM_SUBFRAME_INFO pRrmSubFrame;
		pRrmSubFrame = (PRRM_SUBFRAME_INFO)ptr;

		switch(pRrmSubFrame->SubId)
		{
			case 1:
				if (BcnReqInfoField.field.ReportFrameType == 0)
				{
					/* Init Variable IE structure */
					pVIE = (PNDIS_802_11_VARIABLE_IEs) VarIE;
					pVIE->Length = 0;

					bFrameBody = PeerBeaconAndProbeRspSanity(pAd,
									pRrmSubFrame->Oct,
									pRrmSubFrame->Length,
									pBcnRep->ChNumber,
									ie_list,
									&LenVIE,
									pVIE,
									FALSE,
									TRUE);
				}
				break;

			case 221:
				DBGPRINT(RT_DEBUG_TRACE, ("%s:: SubIe: ID=%x, Len=%d\n",
					__FUNCTION__, pRrmSubFrame->SubId, pRrmSubFrame->Length));
				break;
		}

		RemainLen -= (pRrmSubFrame->Length + 2);
		ptr += (pRrmSubFrame->Length + 2);

		/* avoid infinite loop. */
		if (pRrmSubFrame->Length == 0)
			break;
	}

	if (NdisEqualMemory(pBcnRep->Bssid, ie_list->Bssid, MAC_ADDR_LEN) == FALSE) {
		DBGPRINT(RT_DEBUG_WARN, ("%s():BcnReq->BSSID not equal ie_list->Bssid!\n", __FUNCTION__));
		if (NdisEqualMemory(ie_list->Bssid, ZERO_MAC_ADDR, MAC_ADDR_LEN)) {
			COPY_MAC_ADDR(&ie_list->Addr2[0], pBcnRep->Bssid);
			COPY_MAC_ADDR(&ie_list->Bssid[0], pBcnRep->Bssid);
		}
	}

	if (ie_list->Channel == 0)
		ie_list->Channel = pBcnRep->ChNumber;

#ifdef AP_SCAN_SUPPORT
	if (bFrameBody)
	{

		ie_list->FromBcnReport = TRUE;

		Idx = BssTableSetEntry(pAd, &pAd->ScanTab, ie_list, Rssi, LenVIE, pVIE
#if defined(CUSTOMER_DCC_FEATURE) || defined(NEIGHBORING_AP_STAT)
									,
									Snr[0],
									Snr[1]
#endif
			);
		if (Idx != BSS_NOT_FOUND) {
			BSS_ENTRY *pBssEntry = &pAd->ScanTab.BssEntry[Idx];

			NdisMoveMemory(pBssEntry->PTSF, (PUCHAR)&Ptsf, 4);
			pBssEntry->RegulatoryClass = pBcnRep->RegulatoryClass;
			pBssEntry->CondensedPhyType = BcnReqInfoField.field.CondensePhyType;
			pBssEntry->RSNI = pBcnRep->RSNI;
#ifdef MBO_SUPPORT
			/* update this entry to daemon */
			MboIndicateOneNRtoDaemonByBssEntry(pAd, wdev, pBssEntry);
#endif
			DBGPRINT(RT_DEBUG_TRACE,
				("%s():RegulatoryClass=%d, CondensedPhyType=%d, ",
				__func__, pBssEntry->RegulatoryClass, pBssEntry->CondensedPhyType));
			DBGPRINT(RT_DEBUG_TRACE, ("BSSID=%02x:%02x:%02x:%02x:%02x:%02x, RSNI=%d\n",
				PRINT_MAC(pBssEntry->Bssid), pBssEntry->RSNI));
		}
	}
#endif /* AP_SCAN_SUPPORT */
	if (ie_list != NULL)
		os_free_mem(NULL, ie_list);

	return;
}

VOID RRM_PeerMeasureRepAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem) 
{
	PFRAME_802_11 pFr = (PFRAME_802_11)Elem->Msg;
	PUCHAR pFramePtr = pFr->Octet;
	ULONG MsgLen = Elem->MsgLen;
	PMEASURE_REQ_ENTRY pDialogEntry = NULL;
	PMAC_TABLE_ENTRY pEntry;
	UINT8 DialogToken;
	BOOLEAN Cancelled;
	PNET_DEV NetDev = NULL;
#if defined(CUSTOMER_DCC_FEATURE) || defined(NEIGHBORING_AP_STAT)
	UCHAR Snr[2];

	Snr[0] = Elem->Snr0;
	Snr[1] = Elem->Snr1;

	Snr[0] = ConvertToSnr(pAd, Snr[0]);
	Snr[1] = ConvertToSnr(pAd, Snr[1]);
#endif

	DBGPRINT(RT_DEBUG_TRACE, ("%s::\n", __FUNCTION__));

	/* skip Category and action code. */
	pFramePtr += 2;
	MsgLen -= 2;

	/* get DialogToken. */
	NdisMoveMemory(&DialogToken, pFramePtr, 1);
	pFramePtr += 1;
	MsgLen -= 1;

	/*
		Not a autonomous measure report (non zero DialogToken).
		check the dialog token field.
		drop it if the dialog token doesn't match.
	*/
	if (DialogToken != 0) {
		DBGPRINT(RT_DEBUG_TRACE,
				("DialogToken is non-zere, check DialogToken!\n"));

		pDialogEntry = MeasureReqLookUp(pAd, DialogToken);
		if (!pDialogEntry) {
			MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("DialogToken invalid(%d)!", DialogToken));
			return;
		}
		RTMPCancelTimer(&pDialogEntry->WaitBCNRepTimer, &Cancelled);
		RTMPReleaseTimer(&pDialogEntry->WaitBCNRepTimer, &Cancelled);
	} else {
		if (pFramePtr[2] != 0) {
			DBGPRINT(RT_DEBUG_TRACE,
				("DialogToken is zere, MeasurementToken is non-zero, check MeasurementToken!\n"));

			pDialogEntry = MeasureReqLookUp(pAd, pFramePtr[2]);
			if (!pDialogEntry) {
				DBGPRINT(RT_DEBUG_ERROR,
					("MeasurementToken invalid(%d)!", pFramePtr[2]));
				return;
			}
			RTMPCancelTimer(&pDialogEntry->WaitBCNRepTimer, &Cancelled);
			RTMPReleaseTimer(&pDialogEntry->WaitBCNRepTimer, &Cancelled);
		} else {
			DBGPRINT(RT_DEBUG_ERROR,
				("Both DialogToken and MeasurementToken are zero, drop this packet!\n"));
			return;
		}
	}

	do
	{
		PEID_STRUCT eid_ptr;
		MEASURE_REPORT_MODE ReportMode;
		UINT8 ReportType;
		PRRM_BEACON_REP_INFO pMeasureRep;

		/* Is the STA associated. Dorp the Measure report if it's not. */
		pEntry = MacTableLookup(pAd, pFr->Hdr.Addr2);
		if (!pEntry || (pEntry->Sst != SST_ASSOC) || !pEntry->wdev)
			break;


		eid_ptr = (PEID_STRUCT)pFramePtr;
		while (((UCHAR*)eid_ptr + eid_ptr->Len + 1) < ((PUCHAR)pFramePtr + MsgLen))
		{
			switch (eid_ptr->Eid) {
			case IE_MEASUREMENT_REPORT:
				{
					LONG BcnRepLen = (LONG)eid_ptr->Len - 3;

					NdisMoveMemory(&ReportMode, eid_ptr->Octet + 1, 1);
					NdisMoveMemory(&ReportType, eid_ptr->Octet + 2, 1);
					pMeasureRep = (PVOID)(eid_ptr->Octet + 3);
					if (BcnRepLen) {
						if (ReportType == RRM_MEASURE_SUBTYPE_BEACON)
							RRM_BeaconReportHandler(pAd, pEntry->wdev, pMeasureRep,
							BcnRepLen, pDialogEntry
#if defined(CUSTOMER_DCC_FEATURE) || defined(NEIGHBORING_AP_STAT)
							, Snr
#endif
							);
#if defined(WAPP_SUPPORT)
						wapp_send_bcn_report(pAd, pEntry, eid_ptr->Octet + 3, BcnRepLen);
#endif
					} else {
						if (ReportMode.field.Refused) {
							DBGPRINT(RT_DEBUG_TRACE,
								("STA refuse to do mesurement with type(%d)",
								ReportType));
						} else if (ReportMode.field.Incapable) {
							DBGPRINT(RT_DEBUG_TRACE,
								("STA do not support mesurement with type(%d)",
								ReportType));
						} else if (ReportMode.field.Late) {
							DBGPRINT(RT_DEBUG_TRACE,
								("the request to do mesurement with type(%d) is too late",
								ReportType));
						} else {
							DBGPRINT(RT_DEBUG_TRACE,
								("STA has no beacon report(%02x:%02x:%02x:%02x:%02x:%02x) info to report",
								PRINT_MAC(pFr->Hdr.Addr2)));
						}
						/* send Beacon Response to up-layer */
						NetDev = pAd->ApCfg.MBSSID[pDialogEntry->ControlIndex].wdev.if_dev;
						wext_send_bcn_rsp_event(NetDev, pDialogEntry->StaMac,
							(PUCHAR)eid_ptr->Octet, 3, DialogToken);
					}
				}
				break;

			default:
				break;
			}
			eid_ptr = (PEID_STRUCT)((UCHAR*)eid_ptr + 2 + eid_ptr->Len);
		}
#if defined(WAPP_SUPPORT)
		wapp_send_bcn_report_complete(pAd, pEntry);
#endif
	} while(FALSE);
	MeasureReqDelete(pAd, pDialogEntry->DialogToken);
}


int rrm_MsgHandle(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *wrq)
{
	p_rrm_command_t p_rrm_command = NULL;
	int status = NDIS_STATUS_FAILURE;

	os_alloc_mem(NULL, (UCHAR **)&p_rrm_command, wrq->u.data.length);
	if (p_rrm_command == NULL) {
		DBGPRINT(RT_DEBUG_TRACE, ("!!!(%s) : no memory!!!\n", __func__));
		return NDIS_STATUS_FAILURE;
	}

	copy_from_user(p_rrm_command, wrq->u.data.pointer, wrq->u.data.length);

	switch (p_rrm_command->command_id) {
	case OID_802_11_RRM_CMD_ENABLE:
		if (p_rrm_command->command_len == 0x1) {
			status = Set_Dot11kRRM_Enable(pAd, p_rrm_command->command_body[0]);
		} else {
			DBGPRINT(RT_DEBUG_TRACE, ("Unexpected command len (%d)", p_rrm_command->command_len));
			status = NDIS_STATUS_FAILURE;
		}
		break;

	case OID_802_11_RRM_CMD_CAP:
		if (p_rrm_command->command_len == 0x8) {
			status = set_rrm_capabilities(pAd, p_rrm_command->command_body);
		} else {
			DBGPRINT(RT_DEBUG_TRACE, ("Unexpected command len (%d)", p_rrm_command->command_len));
			status = NDIS_STATUS_FAILURE;
		}
		break;
	/*this cmd is used to compose beacon request frame by the up-layer raw data */
	case OID_802_11_RRM_CMD_SEND_BEACON_REQ:
			status = rrm_send_beacon_req(pAd,
				(p_bcn_req_data_t)p_rrm_command->command_body,
				p_rrm_command->command_len);
		break;
	/*this cmd is used to compose beacon request frame by the up-layer parameters*/
	case OID_802_11_RRM_CMD_SET_BEACON_REQ_PARAM:
			status = rrm_send_beacon_req_param(pAd,
				(p_bcn_req_info)p_rrm_command->command_body,
				p_rrm_command->command_len);
		break;
	/*this cmd is used to compose neighbor report frame by the up-layer raw data*/
	case OID_802_11_RRM_CMD_SEND_NEIGHBOR_REPORT:
			status = rrm_send_nr_rsp_ie(pAd,
				(p_nr_rsp_data_t)p_rrm_command->command_body,
				p_rrm_command->command_len);
		break;
	/*this cmd is used to compose neighbor report frame by the up-layer parameters*/
	case OID_802_11_RRM_CMD_SET_NEIGHBOR_REPORT_PARAM:
			status = rrm_send_nr_rsp_param(pAd,
						(p_rrm_nrrsp_info_custom_t)p_rrm_command->command_body,
							p_rrm_command->command_len);
		break;
	/*this cmd is used to config whether the driver or the up-layer handle the neighbor report request*/
	case OID_802_11_RRM_CMD_HANDLE_NEIGHBOR_REQUEST_BY_DAEMON:
			status = rrm_set_handle_nr_req_flag(pAd, p_rrm_command->command_body[0]);
		break;
		}
	os_free_mem(NULL, p_rrm_command);
	return status;
}


int Set_Dot11kRRM_Enable(RTMP_ADAPTER *pAd, UINT8 enable)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR ifIndex = pObj->ioctl_if;
	BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[ifIndex];

	if (ifIndex >= pAd->ApCfg.BssidNum) {
		DBGPRINT(RT_DEBUG_TRACE, ("Unknown If index (%d)", ifIndex));
		return NDIS_STATUS_FAILURE;
	}

	pMbss->RrmCfg.bDot11kRRMEnable = (BOOLEAN)(enable) == 0 ? FALSE : TRUE;

	if (pMbss->RrmCfg.bDot11kRRMEnable == TRUE)
		pMbss->CapabilityInfo |= RRM_CAP_BIT;
	else
		pMbss->CapabilityInfo &= ~RRM_CAP_BIT;

	APUpdateCapabilityAndErpIe(pAd);
	APUpdateBeaconFrame(pAd, ifIndex);
	return NDIS_STATUS_SUCCESS;
}

int set_rrm_capabilities(RTMP_ADAPTER *pAd, UINT8 *rrm_capabilities)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR ifIndex = pObj->ioctl_if;
	BSS_STRUCT *pMbss;
	PRRM_CONFIG pRrmCfg;

	if (ifIndex >= pAd->ApCfg.BssidNum) {
		DBGPRINT(RT_DEBUG_OFF, ("Unknown If index (%d)", ifIndex));
		return NDIS_STATUS_FAILURE;
	}

	pMbss = &pAd->ApCfg.MBSSID[ifIndex];
	pRrmCfg = &pMbss->RrmCfg;
	NdisCopyMemory((void *)&pRrmCfg->rrm_capabilities, (void *)rrm_capabilities, sizeof(RRM_EN_CAP_IE));

	pRrmCfg->rrm_capabilities.word &= pRrmCfg->max_rrm_capabilities.word;

	if (pMbss->RrmCfg.bDot11kRRMEnable == TRUE)
		APUpdateBeaconFrame(pAd, ifIndex);

	return NDIS_STATUS_SUCCESS;
}

int rrm_send_beacon_req(
	RTMP_ADAPTER *pAd,
	p_bcn_req_data_t p_bcn_req_data,
	UINT32 bcn_req_len)
{
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	PUCHAR pBuf = NULL;
	PBCN_EVENT_DATA Event;
	PMEASURE_REQ_ENTRY pEntry = NULL;
	BSS_STRUCT *pMbss = NULL;
	UINT32 EventLen = 0, Len = 0;
	NDIS_STATUS NStatus = NDIS_STATUS_SUCCESS;
	UCHAR IfIdx = pObj->ioctl_if;

	DBGPRINT(RT_DEBUG_TRACE,
		("%s\n", __func__));

	/* check the AP supports 11k or not */
	pMbss = &pAd->ApCfg.MBSSID[IfIdx];
	if (pMbss->RrmCfg.bDot11kRRMEnable == FALSE) {
		DBGPRINT(RT_DEBUG_ERROR,
			("%s() AP(%02x:%02x:%02x:%02x:%02x:%02x) not support rrm\n",
			__func__, PRINT_MAC(pMbss->wdev.bssid)));
		return NDIS_STATUS_FAILURE;
	}

	Len = sizeof(*p_bcn_req_data) + p_bcn_req_data->bcn_req_len;
	if (bcn_req_len != Len) {
		DBGPRINT(RT_DEBUG_ERROR,
			("%s, bcn req len check failed len1=%d, len2=%d\n",
			__func__, bcn_req_len, Len));
		return NDIS_STATUS_FAILURE;
	}

	/*send bcn req msg into mlme*/
	EventLen = sizeof(*Event) + bcn_req_len;
	os_alloc_mem(NULL, (UCHAR **)&pBuf, EventLen);
	if (!pBuf) {
		DBGPRINT(RT_DEBUG_ERROR,
			("%s, allocate event memory failed\n", __func__));
		return NDIS_STATUS_RESOURCES;
	}
	NdisZeroMemory(pBuf, EventLen);

	/*insert bcn req token into measure table*/
	if (p_bcn_req_data->dialog_token == 0) {
		DBGPRINT(RT_DEBUG_ERROR,
			("%s, invalid MesureReq Token(0)!\n",
			__func__));
		os_free_mem(NULL, pBuf);
		return NDIS_STATUS_FAILURE;
	}
	pEntry = MeasureReqInsert(pAd, p_bcn_req_data->dialog_token);
	if (!pEntry) {
		DBGPRINT(RT_DEBUG_ERROR,
			("%s, Fail to Insert MesureReq Token(%d)!\n",
			__func__, p_bcn_req_data->dialog_token));
		os_free_mem(NULL, pBuf);
		return NDIS_STATUS_FAILURE;
	}
	pEntry->skip_time_check = TRUE;
	pEntry->CurrentState = WAIT_BCN_REQ;
	pEntry->Priv = pAd;
	pEntry->ControlIndex = IfIdx;

	Event = (BCN_EVENT_DATA *)pBuf;
	Event->ControlIndex = IfIdx;
	Event->MeasureReqToken = p_bcn_req_data->dialog_token;
	RTMPMoveMemory(Event->stamac, p_bcn_req_data->peer_address, MAC_ADDR_LEN);
	Event->DataLen = bcn_req_len;
	NdisMoveMemory(Event->Data, p_bcn_req_data, bcn_req_len);

	if (MlmeEnqueue(pAd, BCN_STATE_MACHINE, BCN_REQ_RAW, EventLen, pBuf, 0)
		== FALSE) {
		NStatus = NDIS_STATUS_FAILURE;
		MeasureReqDelete(pAd, p_bcn_req_data->dialog_token);
	}

	/*free memory*/
	os_free_mem(NULL, pBuf);
	return NStatus;
}

INT rrm_send_beacon_req_param(
	IN PRTMP_ADAPTER pAd,
	IN p_bcn_req_info pBcnReq,
	IN UINT32 BcnReqLen)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	BSS_STRUCT *pMbss = NULL;
	PMEASURE_REQ_ENTRY pEntry = NULL;
	PBCN_EVENT_DATA Event = NULL;
	UINT32 Len = 0;
	NDIS_STATUS NStatus = NDIS_STATUS_SUCCESS;
	PUCHAR pBuf = NULL;
	UINT8 IfIdx = pObj->ioctl_if;
	UINT8 MeasureReqToken = 0;
	static UINT8 k;

	DBGPRINT(RT_DEBUG_TRACE,
		("%s\n", __func__));

	/* check the AP supports 11k or not */
	pMbss = &pAd->ApCfg.MBSSID[IfIdx];
	if (pMbss->RrmCfg.bDot11kRRMEnable == FALSE) {
		DBGPRINT(RT_DEBUG_ERROR,
			("%s() rrm off\n", __func__));
		return NDIS_STATUS_FAILURE;
	}

	if (BcnReqLen != sizeof(*pBcnReq)) {
		DBGPRINT(RT_DEBUG_ERROR, ("%s, bcn req len check failed len1=%d, len2=%d\n",
			__func__, BcnReqLen, sizeof(*pBcnReq)));
		return NDIS_STATUS_FAILURE;
	}

	/*send bcn req msg into mlme*/
	Len = sizeof(*Event) + BcnReqLen;
	os_alloc_mem(NULL, (UCHAR **)&pBuf, Len);
	if (!pBuf) {
		DBGPRINT(RT_DEBUG_ERROR,
			("%s, allocate event memory failed\n", __func__));
		return NDIS_STATUS_RESOURCES;
	}
	NdisZeroMemory(pBuf, Len);

	/*insert bcn req token into measure table*/
	k++;
	if (k == 0)
		k = 1;
	MeasureReqToken = k;
	pEntry = MeasureReqInsert(pAd, MeasureReqToken);
	if (!pEntry) {
		DBGPRINT(RT_DEBUG_ERROR,
			("%s, Fail to Insert MesureReq Token(%d)!\n",
			__func__, MeasureReqToken));
		os_free_mem(NULL, pBuf);
		return NDIS_STATUS_FAILURE;
	}
	pEntry->skip_time_check = TRUE;
	pEntry->CurrentState = WAIT_BCN_REQ;
	pEntry->Priv = pAd;
	pEntry->ControlIndex = IfIdx;
	COPY_MAC_ADDR(pEntry->StaMac, pBcnReq->peer_address);

	Event = (PBCN_EVENT_DATA)pBuf;
	Event->ControlIndex = IfIdx;
	Event->MeasureReqToken = MeasureReqToken;
	RTMPMoveMemory(Event->stamac, pBcnReq->peer_address, MAC_ADDR_LEN);
	Event->DataLen = BcnReqLen;
	NdisMoveMemory(Event->Data, pBcnReq, BcnReqLen);

	if (MlmeEnqueue(pAd, BCN_STATE_MACHINE, BCN_REQ, Len, pBuf, 0) == FALSE) {
		NStatus = NDIS_STATUS_FAILURE;
		MeasureReqDelete(pAd, MeasureReqToken);
	}

	/*free memory*/
	os_free_mem(NULL, pBuf);
	return NStatus;
}



void RRM_measurement_report_to_host(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	PFRAME_802_11 pFr = (PFRAME_802_11)Elem->Msg;
	PUCHAR pFramePtr = pFr->Octet;
	ULONG MsgLen = Elem->MsgLen;
	UINT8 DialogToken;
	p_rrm_event_t p_rrm_event;
	p_bcn_rsp_data_t p_bcn_rsp_data;

	MsgLen -= sizeof(HEADER_802_11);

	/* skip category and action code. */
	pFramePtr += 2;
	MsgLen -= 2;
	DialogToken = *pFramePtr;

	pFramePtr += 1;
	MsgLen -= 1;
	os_alloc_mem(NULL, (UCHAR **)&p_rrm_event, sizeof(rrm_event_t) + sizeof(bcn_rsp_data_t) + MsgLen);
	if (p_rrm_event == NULL) {
		DBGPRINT(RT_DEBUG_OFF, ("!!!(%s) : no memory!!!\n", __func__));
		return;
	}

	p_rrm_event->event_id = 1;
	p_rrm_event->event_len = sizeof(bcn_rsp_data_t) + MsgLen;
	p_bcn_rsp_data = (p_bcn_rsp_data_t)(&p_rrm_event->event_body[0]);
	p_bcn_rsp_data->dialog_token = DialogToken;
	p_bcn_rsp_data->ifindex = 1;
	p_bcn_rsp_data->bcn_rsp_len = MsgLen;
	COPY_MAC_ADDR(p_bcn_rsp_data->peer_address, pFr->Hdr.Addr2);
	memcpy(&p_bcn_rsp_data->bcn_rsp[0], pFramePtr, MsgLen);
	RtmpOSWrielessEventSend(
				pAd->net_dev,
				RT_WLAN_EVENT_CUSTOM,
				OID_802_11_RRM_EVENT,
				NULL,
				(UCHAR *) p_rrm_event,
				sizeof(rrm_event_t) + sizeof(bcn_rsp_data_t) + MsgLen);
	os_free_mem(NULL, p_rrm_event);
}


/*
*	this function is used to compose the neighbor report response frame with
*	parameters sent by the up-layer
*	return value: error reason or success
*/
int rrm_send_nr_rsp_param(RTMP_ADAPTER *pAd,
	p_rrm_nrrsp_info_custom_t p_nr_rsp_data, UINT32 nr_rsp_data_len)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	PNR_EVENT_DATA Event = NULL;
	BSS_STRUCT *pMbss = NULL;
	PUCHAR pBuf = NULL;
	UINT32 Len = 0;
	UCHAR ifIndex = pObj->ioctl_if;

	pMbss = &pAd->ApCfg.MBSSID[ifIndex];
	if (pMbss->RrmCfg.bDot11kRRMEnable == FALSE) {
		DBGPRINT(RT_DEBUG_ERROR,
			("%s() rrm off\n", __func__));
		return NDIS_STATUS_FAILURE;
	}

	Len = MAC_ADDR_LEN + 2 +
		p_nr_rsp_data->nrresp_info_count * sizeof(struct nr_info);
	if (nr_rsp_data_len != Len) {
		DBGPRINT(RT_DEBUG_ERROR,
			("%s() length check failed nr_rsp_data_len=%d, Len=%d\n",
			__func__, nr_rsp_data_len, Len));
		return NDIS_STATUS_FAILURE;
	}

	if (p_nr_rsp_data->nrresp_info_count > MAX_CANDIDATE_NUM) {
		DBGPRINT(RT_DEBUG_ERROR,
			("%s the num of candidate(%d) excceed %d\n", __func__,
			  p_nr_rsp_data->nrresp_info_count, MAX_CANDIDATE_NUM));
		return NDIS_STATUS_FAILURE;
	}

	/*send msg to mlme*/
	Len = sizeof(*Event) + nr_rsp_data_len;
	os_alloc_mem(NULL, (UCHAR **)&pBuf, Len);
	if (!pBuf) {
		DBGPRINT(RT_DEBUG_ERROR,
			("%s():allocate event memory failed\n", __func__));
		return NDIS_STATUS_RESOURCES;
	}
	NdisZeroMemory(pBuf, Len);
	Event = (PNR_EVENT_DATA)pBuf;
	Event->ControlIndex = ifIndex;
	Event->MeasureReqToken = p_nr_rsp_data->dialogtoken;
	NdisMoveMemory(Event->stamac, p_nr_rsp_data->peer_address,
		MAC_ADDR_LEN);
	Event->DataLen = nr_rsp_data_len;
	NdisMoveMemory(Event->Data, (PUCHAR)p_nr_rsp_data, nr_rsp_data_len);
	MlmeEnqueue(pAd, NEIGHBOR_STATE_MACHINE, NR_RSP_PARAM, Len, pBuf, 0);

	/*free the memory*/
	os_free_mem(NULL, pBuf);

	return NDIS_STATUS_SUCCESS;
}


/*this function is used to check the validity of customer parameters*/
int check_rrm_nrrsp_custom_params(
	RTMP_ADAPTER *pAd,
	p_rrm_nrrsp_info_custom_t p_nr_rsp_data,
	UINT32 nr_rsp_data_len)
{
	struct nr_info *pinfo = NULL;
	PMAC_TABLE_ENTRY pEntry = NULL;
	UINT32 Len = 0;
	UCHAR ZERO_MAC_ADDR[MAC_ADDR_LEN]  = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	RRM_EN_CAP_IE Zero_RrmEnCap;

	pEntry = MacTableLookup(pAd, p_nr_rsp_data->peer_address);
	if (!pEntry || !((Ndis802_11AuthModeAutoSwitch >=
			pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.AuthMode) ||
			((pEntry->WpaState == AS_PTKINITDONE) &&
			(pEntry->GTKState == REKEY_ESTABLISHED)))) {
		DBGPRINT(RT_DEBUG_ERROR,
			("%s() STA(%02x:%02x:%02x:%02x:%02x:%02x) not associates with AP!\n",
			__func__, PRINT_MAC(p_nr_rsp_data->peer_address)));
		goto error;
	}

	memset((PUCHAR)&Zero_RrmEnCap, 0, sizeof(Zero_RrmEnCap));
	if (!memcmp(&Zero_RrmEnCap, &pEntry->RrmEnCap,
			sizeof(RRM_EN_CAP_IE))) {
		DBGPRINT(RT_DEBUG_ERROR,
			("%s() STA(%02x:%02x:%02x:%02x:%02x:%02x) not support rrm\n",
			__func__, PRINT_MAC(p_nr_rsp_data->peer_address)));
		goto error;
	}

	if (p_nr_rsp_data->nrresp_info_count == 0) {
		DBGPRINT(RT_DEBUG_WARN,
			("%s() nr count equals to 0; neighbor report response is meaningless\n",
			__func__));
		return NDIS_STATUS_SUCCESS;
	}

	Len = p_nr_rsp_data->nrresp_info_count * sizeof(struct nr_info);
	pinfo = p_nr_rsp_data->nrresp_info;

/*
*	need check the validity of each neighbor report candidates
*	   check bssid field of each neighbor report candidates
*/
	while (Len > 0) {
		if (MAC_ADDR_EQUAL(pinfo->bssid, ZERO_MAC_ADDR)) {
			DBGPRINT(RT_DEBUG_ERROR,
				("%s() bssid check failed\n", __func__));
			goto error;
		}
		if (pinfo->channum == 0) {
			DBGPRINT(RT_DEBUG_ERROR,
				("%s() channel check failed\n", __func__));
			goto error;
		}

		Len -= sizeof(struct nr_info);
		pinfo++;
	}

	return NDIS_STATUS_SUCCESS;

error:
	return NDIS_STATUS_FAILURE;
}



void compose_rrm_nrrsp_ie(RTMP_ADAPTER *pAd,
	PUCHAR nr_rsp_ie, PUINT32 p_ie_len,
	struct nr_info *p_candidate_info, UINT8 cnt)
{
	UINT i = 0;
	struct nr_info *p_info = p_candidate_info;
	BOOLEAN nr_flag = FALSE;
	ULONG bss_index = BSS_NOT_FOUND;
	BSS_ENTRY *pBssEntry = NULL;
	RRM_NEIGHBOR_REP_INFO NeighborRepInfo;
	RRM_BSSID_INFO BssidInfo;

	for (i = 0; i < cnt; i++, p_info++) {
/*
*	first, check if need get info from scan table to compose
*	neighbor report response ie
*/
		if (!(p_info->phytype &&
			p_info->regclass &&
			p_info->capinfo &&
			p_info->is_ht &&
			p_info->is_vht &&
			p_info->mobility)) {
			nr_flag = TRUE;
		}
/*
*	if nr_flag equals to TRUE, it means that the up-layer need driver to
*	help with the build of neighbor report ie. so need find info to complete
*	the ie from scan table matched by bssid. if do not find info in
*	scan table, just skip this candidate
*/
		if (nr_flag) {
			bss_index =
				BssTableSearch(&pAd->ScanTab, p_info->bssid, p_info->channum);
			if (bss_index == BSS_NOT_FOUND) {
				DBGPRINT(RT_DEBUG_WARN,
				("%s() bss not found\n", __func__));
				continue;
			}
			pBssEntry = &pAd->ScanTab.BssEntry[bss_index];
			if (p_info->regclass == 0)
				p_info->regclass = pBssEntry->RegulatoryClass;
			if (p_info->capinfo == 0)
				p_info->capinfo = pBssEntry->CapabilityInfo;
			if (p_info->is_ht == 0)
				p_info->is_ht = (pBssEntry->HtCapabilityLen != 0)?1:0;
#ifdef DOT11_VHT_AC
			if (p_info->is_vht == 0)
				p_info->is_vht = (pBssEntry->vht_cap_len != 0)?1:0;
#endif /* DOT11_VHT_AC */
			if (p_info->is_ht == 0)
				p_info->is_ht = (pBssEntry->HtCapabilityLen != 0)?1:0;
			if (p_info->mobility == 0)
				p_info->mobility = (pBssEntry->bHasMDIE)?1:0;

			if (pBssEntry->Channel > 14) {
				if (pBssEntry->HtCapabilityLen != 0) {
#ifdef DOT11_VHT_AC
					if (pBssEntry->vht_cap_len != 0)
						pBssEntry->CondensedPhyType = 9;
					else
#endif /* DOT11_VHT_AC */
						pBssEntry->CondensedPhyType = 7;
				} else { /* OFDM case */
					pBssEntry->CondensedPhyType = 4;
				}
			} else {
				if (pBssEntry->HtCapabilityLen != 0) /* HT case */
					pBssEntry->CondensedPhyType = 7;
				else if (ERP_IS_NON_ERP_PRESENT(pBssEntry->Erp)) /* ERP case */
					pBssEntry->CondensedPhyType = 6;
				else if (pBssEntry->SupRateLen > 4) /* OFDM case (1,2,5.5,11 for CCK 4 Rates) */
					pBssEntry->CondensedPhyType = 4;
				/* no CCK's definition in spec. */
			}
			if (p_info->phytype == 0)
				p_info->phytype = pBssEntry->CondensedPhyType;
		}

		/*compose neighbor report ie buffer*/
		BssidInfo.word = 0;
		BssidInfo.field.APReachAble =
			(p_info->ap_reachability == 0)?3:p_info->ap_reachability;
		BssidInfo.field.Security = p_info->security;
		BssidInfo.field.KeyScope = p_info->key_scope;
		BssidInfo.field.SepctrumMng = (p_info->capinfo & (1 << 8))?1:0;
		BssidInfo.field.Qos = (p_info->capinfo & (1 << 9))?1:0;
		BssidInfo.field.APSD = (p_info->capinfo & (1 << 11))?1:0;
		BssidInfo.field.RRM = (p_info->capinfo & RRM_CAP_BIT)?1:0;
		BssidInfo.field.DelayBlockAck = (p_info->capinfo & (1 << 14))?1:0;
		BssidInfo.field.ImmediateBA = (p_info->capinfo & (1 << 15))?1:0;
		BssidInfo.field.MobilityDomain = p_info->mobility;
		BssidInfo.field.HT = p_info->is_ht;
#ifdef DOT11_VHT_AC
		BssidInfo.field.VHT = p_info->is_vht;
#endif /* DOT11_VHT_AC */
		COPY_MAC_ADDR(NeighborRepInfo.Bssid, p_info->bssid);
		NeighborRepInfo.BssidInfo = cpu2le32(BssidInfo.word);
		NeighborRepInfo.RegulatoryClass = p_info->regclass;
		NeighborRepInfo.ChNum = p_info->channum;
		NeighborRepInfo.PhyType = p_info->phytype;

		RRM_InsertNeighborRepIE_New(pAd, (nr_rsp_ie + *p_ie_len), (PULONG)p_ie_len,
				sizeof(RRM_NEIGHBOR_REP_INFO), &NeighborRepInfo);
	}

}



VOID send_nr_resp_toair(
	IN PRTMP_ADAPTER    pAd,
	IN MLME_QUEUE_ELEM  *Elem)
{
	PNR_EVENT_DATA Event = (PNR_EVENT_DATA)Elem->Msg;
	PMEASURE_REQ_ENTRY pEntry = NULL;
	PMAC_TABLE_ENTRY pMacEntry = NULL;
	PUCHAR Buf = NULL;
	PRRM_FRAME RRMFrame = NULL;
	UINT32 FrameLen = 0, VarLen = Event->DataLen;
	BOOLEAN Cancelled;
	RRM_EN_CAP_IE Zero_RrmEnCap;

	/*check token*/
	pEntry = MeasureReqLookUp(pAd, Event->MeasureReqToken);
	if (pEntry == NULL && Event->MeasureReqToken != 0) {
		DBGPRINT(RT_DEBUG_ERROR,
			("%s() invalid token(%d)\n", __func__,
			Event->MeasureReqToken));
		return;
	}

	pMacEntry = MacTableLookup(pAd, Event->stamac);
	if (!pMacEntry || !((Ndis802_11AuthModeAutoSwitch >=
			pAd->ApCfg.MBSSID[pMacEntry->func_tb_idx].wdev.AuthMode) ||
			((pMacEntry->WpaState == AS_PTKINITDONE) &&
			(pMacEntry->GTKState == REKEY_ESTABLISHED)))) {
		DBGPRINT(RT_DEBUG_ERROR,
			("%s() STA(%02x:%02x:%02x:%02x:%02x:%02x not associates with AP!\n",
			__func__, PRINT_MAC(Event->stamac)));
		return;
	}

	memset((PUCHAR)&Zero_RrmEnCap, 0, sizeof(Zero_RrmEnCap));
	if (!memcmp((PUCHAR)&Zero_RrmEnCap, (PUCHAR)&pMacEntry->RrmEnCap,
			sizeof(RRM_EN_CAP_IE))) {
		DBGPRINT(RT_DEBUG_ERROR,
			("%s() STA(%02x:%02x:%02x:%02x:%02x:%02x) not support rrm\n",
			__func__, PRINT_MAC(Event->stamac)));
		return;
	}

	os_alloc_mem(NULL, (UCHAR **)&Buf, sizeof(*RRMFrame) + VarLen);
	if (!Buf) {
		DBGPRINT(RT_DEBUG_ERROR,
			("%s Not available memory for neighbor report frame\n",
			__func__));
		return;
	}

	NdisZeroMemory(Buf, sizeof(*RRMFrame) + VarLen);
	RRMFrame = (PRRM_FRAME)Buf;
	ActHeaderInit(pAd, &RRMFrame->Hdr, Event->stamac,
					pAd->ApCfg.MBSSID[Event->ControlIndex].wdev.bssid,
					pAd->ApCfg.MBSSID[Event->ControlIndex].wdev.bssid);
	FrameLen = sizeof(HEADER_802_11);
	RRMFrame->Category = CATEGORY_RM;
	FrameLen += 1;
	RRMFrame->u.NR_RSP.Action = RRM_NEIGHTBOR_RSP;
	FrameLen += 1;
	RRMFrame->u.NR_RSP.DialogToken = Event->MeasureReqToken;
	FrameLen += 1;
	NdisMoveMemory(RRMFrame->u.NR_RSP.Variable, Event->Data,
					Event->DataLen);
	FrameLen += Event->DataLen;

	/*receive neighbor report response;  cancel timer*/
	if (Event->MeasureReqToken) {
		RTMPCancelTimer(&pEntry->WaitNRRspTimer, &Cancelled);
		RTMPReleaseTimer(&pEntry->WaitNRRspTimer, &Cancelled);
	}
	MeasureReqDelete(pAd, Event->MeasureReqToken);
	MiniportMMRequest(pAd, (MGMT_USE_QUEUE_FLAG | QID_AC_BE), Buf, FrameLen);
	os_free_mem(NULL, Buf);

}



VOID send_nr_rsp_param_toair(
	IN PRTMP_ADAPTER    pAd,
	IN MLME_QUEUE_ELEM  *Elem)
{
	PNR_EVENT_DATA Event = (PNR_EVENT_DATA)Elem->Msg;
	p_rrm_nrrsp_info_custom_t p_nr_rsp_data = NULL;
	PMEASURE_REQ_ENTRY pEntry = NULL;
	PUCHAR pOutBuffer = NULL;
	ULONG FrameLen = 0;
	UINT32 nr_rsp_data_len = Event->DataLen;
	NDIS_STATUS NStatus = NDIS_STATUS_SUCCESS;
	BOOLEAN Cancelled;
	HEADER_802_11 ActHdr;

	p_nr_rsp_data = (p_rrm_nrrsp_info_custom_t)Event->Data;

	/*check token*/
	pEntry = MeasureReqLookUp(pAd, p_nr_rsp_data->dialogtoken);
	if (pEntry == NULL && p_nr_rsp_data->dialogtoken != 0) {
		DBGPRINT(RT_DEBUG_ERROR,
				("%s() invalid token(%d)\n", __func__,
				p_nr_rsp_data->dialogtoken));
		return;
	}
	/*check the other parameters*/
	NStatus = check_rrm_nrrsp_custom_params(pAd, p_nr_rsp_data, nr_rsp_data_len);
	if (NStatus != NDIS_STATUS_SUCCESS) {
		DBGPRINT(RT_DEBUG_ERROR,
			("%s() check customer params failed\n", __func__));
		return;
	}

	/*allocate a buffer prepare for Neighbor report IE*/
	NStatus = MlmeAllocateMemory(pAd, (PVOID)&pOutBuffer);
	if (NStatus != NDIS_STATUS_SUCCESS) {
		DBGPRINT(RT_DEBUG_ERROR,
			("%s() allocate memory for neighbor report frame failed\n",
			__func__));
		return;
	}
	/*compose 80211 header*/
	ActHeaderInit(pAd, &ActHdr, p_nr_rsp_data->peer_address,
					pAd->ApCfg.MBSSID[Event->ControlIndex].wdev.bssid,
					pAd->ApCfg.MBSSID[Event->ControlIndex].wdev.bssid);
	NdisMoveMemory(pOutBuffer, (PCHAR)&ActHdr, sizeof(ActHdr));
	FrameLen = sizeof(ActHdr);

	/*compose the  Neighbor report header
	*   category, action and token
	*/
	InsertActField(pAd, (pOutBuffer + FrameLen), &FrameLen,
		CATEGORY_RM, RRM_NEIGHTBOR_RSP);
	InsertDialogToken(pAd, (pOutBuffer + FrameLen),
		&FrameLen, p_nr_rsp_data->dialogtoken);

	hex_dump("nr rsp header", pOutBuffer, FrameLen);
	/*compose the  Neighbor report ie*/
	compose_rrm_nrrsp_ie(pAd, pOutBuffer, (PUINT32)&FrameLen,
		p_nr_rsp_data->nrresp_info,
		p_nr_rsp_data->nrresp_info_count);
	hex_dump("nr rsp payload", pOutBuffer,
						FrameLen);
	/*receive neighbor report response;
	*   cancel timer
	*/
	if (p_nr_rsp_data->dialogtoken) {
		RTMPCancelTimer(&pEntry->WaitNRRspTimer, &Cancelled);
		RTMPReleaseTimer(&pEntry->WaitNRRspTimer, &Cancelled);
	}
	MeasureReqDelete(pAd, p_nr_rsp_data->dialogtoken);
	MiniportMMRequest(pAd, (MGMT_USE_QUEUE_FLAG | QID_AC_BE), pOutBuffer, FrameLen);
	os_free_mem(NULL, pOutBuffer);

}



int rrm_send_nr_rsp_ie(RTMP_ADAPTER *pAd,
	p_nr_rsp_data_t p_nr_rsp_data,
	UINT32 nr_rsp_data_len)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	BSS_STRUCT *pMbss = NULL;
	PNR_EVENT_DATA Event = NULL;
	PUCHAR pBuf = NULL;
	UINT32 FrameLen = p_nr_rsp_data->nr_rsp_len;
	UINT32 Len = 0;
	UINT8 ifIndex = pObj->ioctl_if;


	pMbss = &pAd->ApCfg.MBSSID[ifIndex];
	if (pMbss->RrmCfg.bDot11kRRMEnable == FALSE) {
		DBGPRINT(RT_DEBUG_ERROR,
			("%s() rrm off\n", __func__));
		return NDIS_STATUS_FAILURE;
	}

	Len = sizeof(*p_nr_rsp_data) + FrameLen;
	if (nr_rsp_data_len != Len) {
		DBGPRINT(RT_DEBUG_ERROR,
			("%s() length check failed nr_rsp_data_len=%d, Len=%d\n",
			__func__, nr_rsp_data_len, Len));
		return NDIS_STATUS_FAILURE;
	}

	if (FrameLen > 1000) {
		DBGPRINT(RT_DEBUG_ERROR,
			("%s nr rsp ie len(%d) is too long", __func__, FrameLen));
		return NDIS_STATUS_FAILURE;
	}

	/*send a msg into mlme*/
	Len = sizeof(*Event) + FrameLen;
	os_alloc_mem(NULL, (UCHAR **)&pBuf, Len);
	if (!pBuf) {
		DBGPRINT(RT_DEBUG_ERROR,
			("%s() allocate event memory failed\n", __func__));
		return NDIS_STATUS_RESOURCES;
	}
	NdisZeroMemory(pBuf, Len);
	Event = (PNR_EVENT_DATA)pBuf;
	Event->ControlIndex = ifIndex;
	Event->MeasureReqToken = p_nr_rsp_data->dialog_token;
	NdisMoveMemory(Event->stamac, p_nr_rsp_data->peer_address,
		MAC_ADDR_LEN);
	Event->DataLen = FrameLen;
	NdisMoveMemory(Event->Data, p_nr_rsp_data->nr_rsp, FrameLen);
	MlmeEnqueue(pAd, NEIGHBOR_STATE_MACHINE, NR_RSP, Len, pBuf, 0);

	/*free the memory*/
	os_free_mem(NULL, pBuf);

	return NDIS_STATUS_SUCCESS;
}



/*this func is used to config whether the driver or
*  the up-layer handle the neighbor report request
 */
int rrm_set_handle_nr_req_flag(RTMP_ADAPTER *pAd, UINT8 by_daemon)
{
	pAd->ApCfg.HandleNRReqbyUplayer = (by_daemon == 0) ? 0:1;
	DBGPRINT(RT_DEBUG_TRACE,
			("%s, HandleNRReqbyUplayer(%d)!\n",
			__func__, pAd->ApCfg.HandleNRReqbyUplayer));

	return NDIS_STATUS_SUCCESS;
}



void wext_send_nr_req_event(PNET_DEV net_dev,
	const char *peer_mac_addr,
	const char *nr_req,
	UINT16 nr_req_len)
{
	struct nr_req_data *data = NULL;
	p_rrm_event_t event_data = NULL;
	UINT32 buflen = 0;
	UCHAR *buf;

	buflen = sizeof(*event_data) + sizeof(*data) + nr_req_len;
	os_alloc_mem(NULL, (UCHAR **)&buf, buflen);
	NdisZeroMemory(buf, buflen);

	event_data = (p_rrm_event_t)buf;
	event_data->event_id = OID_802_11_RRM_EVT_NEIGHBOR_REQUEST;
	event_data->event_len = sizeof(*data) + nr_req_len;

	data = (struct nr_req_data *)event_data->event_body;
	data->ifindex = RtmpOsGetNetIfIndex(net_dev);
	memcpy(data->peer_mac_addr, peer_mac_addr, 6);
	data->nr_req_len = nr_req_len;
	memcpy(data->nr_req, nr_req, nr_req_len);

	RtmpOSWrielessEventSend(net_dev, RT_WLAN_EVENT_CUSTOM,
					OID_802_11_RRM_EVENT, NULL, (PUCHAR)buf, buflen);

	os_free_mem(NULL, buf);
}



/*used to check if the up-layer does not send neighbor report response in time*/
VOID  NRRspTimeout(
	IN PRTMP_ADAPTER    pAd,
	IN MLME_QUEUE_ELEM  *Elem)
{
	PNR_EVENT_DATA Event = (PNR_EVENT_DATA)Elem->Msg;
	PMEASURE_REQ_ENTRY pEntry = NULL;
	UINT8 MeasureReqToken = Event->MeasureReqToken;
	BOOLEAN Cancelled;

	DBGPRINT(RT_DEBUG_TRACE,
		("%s StaMac(%02x:%02x:%02x:%02x:%02x:%02x), MeasureReqToken=%d\n",
		__func__, PRINT_MAC(Event->stamac), MeasureReqToken));

	pEntry = MeasureReqLookUp(pAd, MeasureReqToken);

	if (!pEntry) {
		DBGPRINT(RT_DEBUG_TRACE,
			("%s nr response entry not founded\n", __func__));
		return;
	}

	RTMPReleaseTimer(&pEntry->WaitNRRspTimer, &Cancelled);
	/*delete measure entry to avoid the full condition of measure table*/
	MeasureReqDelete(pAd, MeasureReqToken);
}



VOID WaitNRRspTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	PMEASURE_REQ_ENTRY pEntry = (PMEASURE_REQ_ENTRY)FunctionContext;
	PRTMP_ADAPTER pAd;
	UINT32 Len = 0;
	NR_EVENT_DATA Event;

	DBGPRINT(RT_DEBUG_TRACE, ("%s\n", __func__));

	if (!pEntry) {
		DBGPRINT(RT_DEBUG_ERROR,
			("%s: MeasureReq Entry doesn't exist\n", __func__));
		return;
	}

	pAd = pEntry->Priv;
	NdisZeroMemory(&Event, sizeof(Event));
	Event.ControlIndex = pEntry->ControlIndex;
	Event.MeasureReqToken = pEntry->DialogToken;
	memcpy(Event.stamac, pEntry->StaMac, MAC_ADDR_LEN);
	Event.DataLen = 0;
	Len = sizeof(Event);

	MlmeEnqueue(pAd, NEIGHBOR_STATE_MACHINE, NR_RSP_TIMEOUT, Len, &Event, 0);

}
BUILD_TIMER_FUNCTION(WaitNRRspTimeout);


enum NR_STATE NRPeerCurrentState(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	PNR_EVENT_DATA Event = (PNR_EVENT_DATA)Elem->Msg;
	PMEASURE_REQ_ENTRY pEntry = NULL;
	UINT8 MeasureReqToken = Event->MeasureReqToken;

	DBGPRINT(RT_DEBUG_TRACE,
			("%s, MeasureReqToken=%d!\n", __func__, MeasureReqToken));

	 pEntry = MeasureReqLookUp(pAd, MeasureReqToken);

	if (pEntry != NULL) {
		DBGPRINT(RT_DEBUG_TRACE,
			("%s, CurrentState=%d!\n", __func__, pEntry->CurrentState));
		return pEntry->CurrentState;
	 } else {
		 DBGPRINT(RT_DEBUG_ERROR,
			("%s, invalid MeasureReqToken!\n", __func__));

		return NR_UNKNOWN;
	 }
}

int check_rrm_BcnReq_custom_params(
	RTMP_ADAPTER *pAd,
	p_bcn_req_info p_beacon_req)
{
	PMAC_TABLE_ENTRY pEntry = NULL;
	UCHAR ZERO_OpClassList[OP_LEN] = {0};

	DBGPRINT(RT_DEBUG_TRACE,
		("%s()\n", __func__));

	/* the the peer is connected or not */
	pEntry = MacTableLookup(pAd, p_beacon_req->peer_address);
	if (!pEntry || !((Ndis802_11AuthModeAutoSwitch >=
			pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.AuthMode) ||
			((pEntry->WpaState == AS_PTKINITDONE) &&
			(pEntry->GTKState == REKEY_ESTABLISHED)))) {
		DBGPRINT(RT_DEBUG_ERROR,
			("%s() STA(%02x:%02x:%02x:%02x:%02x:%02x) not associates with AP!\n",
			__func__, PRINT_MAC(p_beacon_req->peer_address)));
		goto error;
	}

	/* check the peer supports 11k or not */
	if (!(pEntry->RrmEnCap.field.BeaconPassiveMeasureCap ||
		pEntry->RrmEnCap.field.BeaconActiveMeasureCap)) {
		DBGPRINT(RT_DEBUG_ERROR,
			("%s() STA(%02x:%02x:%02x:%02x:%02x:%02x) not support beacon report!\n",
			__func__, PRINT_MAC(p_beacon_req->peer_address)));
		goto error;
	}


	/* operating class; only mandatory when channel is set to 0*/
	if (p_beacon_req->channum == 0 && p_beacon_req->regclass == 0) {
		DBGPRINT(RT_DEBUG_ERROR,
			("%s() regclass cannot set to 0 when channel is set to 0\n",
			__func__));
		goto error;
	}

	if (p_beacon_req->channum == 255) {
		if (p_beacon_req->op_class_len == 0) {
			DBGPRINT(RT_DEBUG_ERROR,
					("%s() op class must be set when channel is set to 255!\n",
					__func__));
			goto error;
		}
		if (!RTMPCompareMemory(p_beacon_req->op_class_list,
				ZERO_OpClassList, OP_LEN)) {
			DBGPRINT(RT_DEBUG_ERROR,
				("%s() op class list is invalid!\n", __func__));
			goto error;
		}
	}

	return NDIS_STATUS_SUCCESS;

error:
	return NDIS_STATUS_FAILURE;
}



VOID set_rrm_BcnReq_optional_params(
	RTMP_ADAPTER *pAd,
	p_bcn_req_info p_beacon_req)
{
	PMAC_TABLE_ENTRY pEntry = NULL;
	UCHAR ZERO_MAC_ADDR[MAC_ADDR_LEN]  = {0};
	UCHAR BROADCAST_ADDR[MAC_ADDR_LEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	UCHAR DefaultRequest[13] = {0,	/* SSID */
							1,	 /* Support Rate*/
							45, /* HT IE*/
							48, /* RSN IE */
							50, /* Extended Support Rate*/
							54, /* Mobility Domain. */
							61, /* HT ADD*/
							70, /* RRM Capabilities. */
							127, /* Ext Cap*/
							191, /* VHT 1*/
							192, /* VHT 2*/
							195,  /* VHT 3*/
							221  /*vendor ie*/
						   };

	pEntry = MacTableLookup(pAd, p_beacon_req->peer_address);

	if (p_beacon_req->channum != 0 && p_beacon_req->channum != 255) {
		if (p_beacon_req->regclass == 0) {
			p_beacon_req->regclass =
				get_regulatory_class(pAd, p_beacon_req->channum);
			DBGPRINT(RT_DEBUG_TRACE,
				("%s() set optional regclass = %d\n",
				__func__, p_beacon_req->regclass));
		}
	}
	if (p_beacon_req->duration == 0) {
		p_beacon_req->duration = 20;
		DBGPRINT(RT_DEBUG_TRACE,
			("%s() set optional duration = %d\n",
			__func__, p_beacon_req->duration));
	}
	if (!RTMPCompareMemory(p_beacon_req->bssid, ZERO_MAC_ADDR, MAC_ADDR_LEN)) {
		RTMPMoveMemory(p_beacon_req->bssid, BROADCAST_ADDR, MAC_ADDR_LEN);
		DBGPRINT(RT_DEBUG_TRACE,
			("%s() set optional bssid = %02x:%02x:%02x:%02x:%02x:%02x\n",
			__func__, PRINT_MAC(p_beacon_req->bssid)));
	}
	if (p_beacon_req->mode == 0) {
		if (pEntry != NULL && pEntry->RrmEnCap.field.BeaconActiveMeasureCap)
			p_beacon_req->mode = 1;
		else
			p_beacon_req->mode = 0;
	} else {
		p_beacon_req->mode--;
	}
	DBGPRINT(RT_DEBUG_TRACE,
			("%s() set optional mode = %d!\n",
			__func__, p_beacon_req->mode));

	if (p_beacon_req->timeout == 0) {
		p_beacon_req->timeout = BCN_REP_TIMEOUT_VALUE;
		DBGPRINT(RT_DEBUG_TRACE,
			("%s() set optional timeout = %d!\n",
			__func__, p_beacon_req->timeout));
	}

	if (p_beacon_req->detail == 0) {
		p_beacon_req->detail = 1;
		DBGPRINT(RT_DEBUG_TRACE,
			("%s() set optional detail = %d\n",
			__func__, p_beacon_req->detail));
		p_beacon_req->request_len = 13;
		RTMPMoveMemory(p_beacon_req->request, DefaultRequest, 13);
	} else {
		p_beacon_req->detail--;
	}
}



void compose_rrm_BcnReq_ie(RTMP_ADAPTER *pAd,
	PUCHAR beacon_req_ie, PUINT32 beacon_req_ie_len,
	p_bcn_req_info p_beacon_req, UINT8 measure_req_token, UCHAR ifidx)
{
	PUCHAR pOutBuffer = beacon_req_ie;
	ULONG FrameLen = 0, HeaderLen = 0;
	UINT32 FramelenTmp = 0;
	UINT16 Repetitions = 0;
	UINT8 MeasureReqType = RRM_MEASURE_SUBTYPE_BEACON;
	UINT8 TotalLen = 0, idx = 0;
	MEASURE_REQ_MODE MeasureReqMode;
	RRM_BEACON_REQ_INFO BcnReq;

	DBGPRINT(RT_DEBUG_TRACE,
		("%s\n", __func__));
	/*
	*	Action header has a field to indicate total length of packet
	*	but the total length is unknown untial whole packet completd.
	*	So skip the action here and fill it late.
	*	1. skip Catgore (1 octect), Action(1 octect).
	*	2. skip dailog token (1 octect).
	*	3. skip Num Of Repetitions field (2 octects)
	*	3. skip MeasureReqIE (2 + sizeof(MEASURE_REQ_INFO)).
	*/
	FrameLen = 7 + sizeof(MEASURE_REQ_INFO);
	TotalLen = sizeof(MEASURE_REQ_INFO);
	/*Insert Beacon Req IE*/
	BcnReq.RegulatoryClass = p_beacon_req->regclass;
	BcnReq.ChNumber = p_beacon_req->channum;
	BcnReq.RandomInterval = cpu2le16(p_beacon_req->random_ivl);
	BcnReq.MeasureDuration = cpu2le16(p_beacon_req->duration);
	BcnReq.MeasureMode = p_beacon_req->mode;
	COPY_MAC_ADDR(BcnReq.Bssid, p_beacon_req->bssid);
	RRM_InsertBcnReqIE(pAd, (pOutBuffer+FrameLen),
		&FrameLen, (PUCHAR)&BcnReq);
	TotalLen += sizeof(RRM_BEACON_REQ_INFO);
	DBGPRINT(RT_DEBUG_TRACE,
			("%s RegulatoryClass=%d ChNumber=%d RandomInterval=%d ",
			__func__, BcnReq.RegulatoryClass, BcnReq.ChNumber,
			BcnReq.RandomInterval));
	DBGPRINT(RT_DEBUG_TRACE,
			("MeasureDuration=%d MeasureMode=%d ",
			BcnReq.MeasureDuration, BcnReq.MeasureMode));
	DBGPRINT(RT_DEBUG_TRACE,
			("Bssid(%02x:%02x:%02x:%02x:%02x:%02x\n",
			PRINT_MAC(BcnReq.Bssid)));


	/* inssert SSID sub field.
	*	Fix Voice Enterprise : Item V-E-4.3,
	*	case2 still need to include the SSID sub filed even SsidLen is 0
	*/
	RRM_InsertBcnReqSsidSubIE(pAd, (pOutBuffer+FrameLen),
		&FrameLen, (PUCHAR)p_beacon_req->req_ssid, p_beacon_req->req_ssid_len);
	TotalLen += (p_beacon_req->req_ssid_len + 2);

	/* inssert beacon reporting information sub field. */
	RRM_InsertBcnReqRepCndSubIE(pAd, (pOutBuffer+FrameLen),
		&FrameLen, p_beacon_req->rep_conditon, p_beacon_req->ref_value);
		TotalLen += (2 + 2);

	/* inssert reporting detail sub field. */
	RRM_InsertBcnReqRepDetailSubIE(pAd, (pOutBuffer+FrameLen),
			&FrameLen, p_beacon_req->detail);
	TotalLen += (2 + 1);

	if (p_beacon_req->detail == 1) {
		RRM_InsertRequestIE_NEW(pAd, (pOutBuffer+FrameLen),
			&FrameLen, p_beacon_req->request, p_beacon_req->request_len);
		TotalLen += (p_beacon_req->request_len + 2);
	}

	/* inssert channel report sub field. */
	if (p_beacon_req->channum == 255) {
		idx = 0;
		while (p_beacon_req->op_class_list[idx] != 0) {
			FramelenTmp = FrameLen;
			InsertChannelRepIE(pAd, (pOutBuffer+FrameLen), &FrameLen,
				(RTMP_STRING *)pAd->CommonCfg.CountryCode,
				p_beacon_req->op_class_list[idx], p_beacon_req->ch_list);
			TotalLen += (FrameLen - FramelenTmp);
			idx++;
		}
	}

	if (TotalLen > 250) {
		DBGPRINT(RT_DEBUG_ERROR,
			("%s() MeasureReq IE len > 255!\n", __func__));
	}

	/* Insert Action header here. */
	MeasureReqMode.word = 0;
	Repetitions = p_beacon_req->num_rpt;
	MakeMeasurementReqFrame(pAd, pOutBuffer, &HeaderLen,
		TotalLen, CATEGORY_RM, RRM_MEASURE_REQ, measure_req_token,
		MeasureReqMode.word, MeasureReqType, Repetitions);

	*beacon_req_ie_len = FrameLen;
}



void wext_send_bcn_rsp_event(PNET_DEV net_dev,
	PUCHAR peer_mac_addr,
	PUCHAR bcn_rsp,
	UINT32 bcn_rsp_len,
	UINT8 dia_token)
{
	p_bcn_rsp_data_t data = NULL;
	p_rrm_event_t event_data = NULL;
	UINT32 buflen = 0;
	UCHAR *buf;

	DBGPRINT(RT_DEBUG_TRACE,
				("%s\n", __func__));

	buflen = sizeof(*event_data) + sizeof(*data) + bcn_rsp_len;
	os_alloc_mem(NULL, (UCHAR **)&buf, buflen);
	NdisZeroMemory(buf, buflen);

	event_data = (p_rrm_event_t)buf;
	event_data->event_id = OID_802_11_RRM_EVT_BEACON_REPORT;
	DBGPRINT(RT_DEBUG_TRACE,
				("%s event_data->event_id = %d\n", __func__,
				event_data->event_id));
	event_data->event_len = sizeof(*data) + bcn_rsp_len;

	data = (p_bcn_rsp_data_t)event_data->event_body;
	data->dialog_token = dia_token;
	data->ifindex = RtmpOsGetNetIfIndex(net_dev);
	memcpy(data->peer_address, peer_mac_addr, 6);
	data->bcn_rsp_len = bcn_rsp_len;
	memcpy(data->bcn_rsp, bcn_rsp, bcn_rsp_len);

	hex_dump("data in Driver", buf, buflen);

	RtmpOSWrielessEventSend(net_dev, RT_WLAN_EVENT_CUSTOM,
					OID_802_11_RRM_EVENT, NULL, (PUCHAR)buf, buflen);

	os_free_mem(NULL, buf);

}


static VOID  BcnRepTimeout(
	IN PRTMP_ADAPTER    pAd,
	IN MLME_QUEUE_ELEM  *Elem)
{
	PNET_DEV NetDev = NULL;
	PMEASURE_REQ_ENTRY pEntry = NULL;
	PBCN_EVENT_DATA Event = (BCN_EVENT_DATA *)Elem->Msg;
	UINT8 MeasureReqToken = Event->MeasureReqToken;
	BOOLEAN Cancelled;

	DBGPRINT(RT_DEBUG_TRACE,
		("%s StaMac(%02x:%02x:%02x:%02x:%02x:%02x) MeasureReqToken=%d\n",
		__func__, PRINT_MAC(Event->stamac), MeasureReqToken));


	pEntry = MeasureReqLookUp(pAd, MeasureReqToken);
	if (!pEntry) {
		DBGPRINT(RT_DEBUG_ERROR,
			("%s Bcnreq entry not founded\n", __func__));
		return;
	}

	RTMPReleaseTimer(&pEntry->WaitBCNRepTimer, &Cancelled);
	MeasureReqDelete(pAd, MeasureReqToken);

	/* send Beacon Request timeout to up-layer */
	NetDev = pAd->ApCfg.MBSSID[pEntry->ControlIndex].wdev.if_dev;
	if (NetDev == NULL) {
		DBGPRINT(RT_DEBUG_ERROR,
			("no valid NetDev! IfIndex=%d\n", pEntry->ControlIndex));
		return;
	}

	wext_send_bcn_rsp_event(NetDev, Event->stamac, Event->Data,
		0, MeasureReqToken);
}



VOID WaitPeerBCNRepTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	PMEASURE_REQ_ENTRY pEntry = (PMEASURE_REQ_ENTRY)FunctionContext;
	PRTMP_ADAPTER pAd = NULL;
	UINT32 Len = 0;
	BCN_EVENT_DATA Event;

	DBGPRINT(RT_DEBUG_TRACE,
		("%s\n", __func__));

	pAd = pEntry->Priv;
	Len = sizeof(Event);
	NdisZeroMemory((PUCHAR)&Event, Len);
	Event.MeasureReqToken = pEntry->DialogToken;
	RTMPMoveMemory(Event.stamac, pEntry->StaMac, MAC_ADDR_LEN);
	Event.DataLen = 0;

	MlmeEnqueue(pAd, BCN_STATE_MACHINE, BCN_REP_TIMEOUT,
		Len, (PUCHAR)&Event, 0);
}
BUILD_TIMER_FUNCTION(WaitPeerBCNRepTimeout);


static VOID SendBcnReqToAir_SetParam(
	IN PRTMP_ADAPTER    pAd,
	IN MLME_QUEUE_ELEM  *Elem)
{
	BCN_EVENT_DATA *Event = (BCN_EVENT_DATA *)Elem->Msg;
	p_bcn_req_info pBcnReq = (p_bcn_req_info)Event->Data;
	PMEASURE_REQ_ENTRY pMeasureReqEntry = NULL;
	PUCHAR pOutBuffer = NULL;
	NDIS_STATUS NStatus = NDIS_STATUS_SUCCESS;
	UINT32 FrameLen = 0, TmpLen = 0;
	ULONG Timeout = 0;
	UINT8 MeasureReqToken = Event->MeasureReqToken;
	UINT8 IfIdx = Event->ControlIndex;
	HEADER_802_11 ActHdr;

	DBGPRINT(RT_DEBUG_TRACE,
		("%s()\n", __func__));

	/*check the parameters*/
	NStatus = check_rrm_BcnReq_custom_params(pAd, pBcnReq);
	if (NStatus != NDIS_STATUS_SUCCESS) {
		DBGPRINT(RT_DEBUG_ERROR,
			("%s() check customer params failed\n", __func__));
		MeasureReqDelete(pAd, MeasureReqToken);
		return;
	}

	/*set the optional paratmeters when customer don't set*/
	set_rrm_BcnReq_optional_params(pAd, pBcnReq);

	/*compose the Beacon Request frame*/
	NStatus = MlmeAllocateMemory(pAd, (PVOID)&pOutBuffer);
	if (NStatus != NDIS_STATUS_SUCCESS) {
		DBGPRINT(RT_DEBUG_ERROR,
			("%s, MEMORY ALLOC Failed!!\n", __func__));
		MeasureReqDelete(pAd, MeasureReqToken);
		return;
	}
	ActHeaderInit(pAd, &ActHdr, Event->stamac,
					pAd->ApCfg.MBSSID[IfIdx].wdev.bssid,
					pAd->ApCfg.MBSSID[IfIdx].wdev.bssid);
	memcpy(pOutBuffer, (PUCHAR)&ActHdr, sizeof(ActHdr));
	FrameLen = sizeof(ActHdr);

	compose_rrm_BcnReq_ie(pAd, (pOutBuffer+FrameLen), &TmpLen,
		pBcnReq, MeasureReqToken, IfIdx);
	FrameLen += TmpLen;

	hex_dump("bcn req frame", pOutBuffer, FrameLen);

	MiniportMMRequest(pAd, (MGMT_USE_QUEUE_FLAG | QID_AC_BE), pOutBuffer, FrameLen);

	pMeasureReqEntry = MeasureReqLookUp(pAd, MeasureReqToken);
	if (pMeasureReqEntry == NULL) {
		DBGPRINT(RT_DEBUG_ERROR,
			("%s() pMeasureReqEntry is NULL!\n", __func__));
		MlmeFreeMemory(pAd, pOutBuffer);
		return;
	}
	pMeasureReqEntry->CurrentState = WAIT_BCN_REP;
	Timeout = pBcnReq->timeout * 1000;
	DBGPRINT(RT_DEBUG_TRACE,
			("%s() Timeout=%lds!\n", __func__, Timeout/1000));
	RTMPInitTimer(pAd, &pMeasureReqEntry->WaitBCNRepTimer,
		GET_TIMER_FUNCTION(WaitPeerBCNRepTimeout), pMeasureReqEntry, FALSE);
	RTMPSetTimer(&pMeasureReqEntry->WaitBCNRepTimer, Timeout);

	MlmeFreeMemory(pAd, pOutBuffer);

}


static VOID SendBcnReqToAir_Raw(
	IN PRTMP_ADAPTER    pAd,
	IN MLME_QUEUE_ELEM  *Elem)
{
	PBCN_EVENT_DATA Event = (PBCN_EVENT_DATA)Elem->Msg;
	PMEASURE_REQ_ENTRY pMeasureReqEntry = NULL;
	PMAC_TABLE_ENTRY pEntry = NULL;
	p_bcn_req_data_t p_bcn_req_data_raw = (p_bcn_req_data_t)Event->Data;
	PUCHAR pbuf = NULL;
	ULONG Timeout = 0, FrameLen = 0;
	NDIS_STATUS NStatus = NDIS_STATUS_SUCCESS;
	UINT8 MeasureReqToken = Event->MeasureReqToken;
	UINT8 IfIdx = Event->ControlIndex;
	HEADER_802_11 ActHdr;

	DBGPRINT(RT_DEBUG_TRACE,
		("%s()\n", __func__));

	/* the the peer is connected or not */
	pEntry = MacTableLookup(pAd, p_bcn_req_data_raw->peer_address);
	if (!pEntry || !((Ndis802_11AuthModeAutoSwitch >=
			pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.AuthMode) ||
			((pEntry->WpaState == AS_PTKINITDONE) &&
			(pEntry->GTKState == REKEY_ESTABLISHED)))) {
		DBGPRINT(RT_DEBUG_ERROR,
			("%s() STA(%02x:%02x:%02x:%02x:%02x:%02x) not associates with AP!\n",
			__func__, PRINT_MAC(p_bcn_req_data_raw->peer_address)));
		MeasureReqDelete(pAd, MeasureReqToken);
		return;
	}

	/* check the peer supports 11k or not */
	if (!(pEntry->RrmEnCap.field.BeaconPassiveMeasureCap ||
		pEntry->RrmEnCap.field.BeaconActiveMeasureCap)) {
		DBGPRINT(RT_DEBUG_ERROR,
			("%s() STA(%02x:%02x:%02x:%02x:%02x:%02x) does not support beacon report!\n",
			__func__, PRINT_MAC(p_bcn_req_data_raw->peer_address)));
		MeasureReqDelete(pAd, MeasureReqToken);
		return;
	}

	/*compose the Beacon Request frame*/
	NStatus = MlmeAllocateMemory(pAd, (PVOID)&pbuf);
	if (NStatus != NDIS_STATUS_SUCCESS) {
		DBGPRINT(RT_DEBUG_TRACE,
			("%s() allocate memory failed\n", __func__));
		MeasureReqDelete(pAd, MeasureReqToken);
		return;
	}

	ActHeaderInit(pAd, &ActHdr, p_bcn_req_data_raw->peer_address,
					pAd->ApCfg.MBSSID[IfIdx].wdev.bssid,
					pAd->ApCfg.MBSSID[IfIdx].wdev.bssid);
	NdisMoveMemory(pbuf, (PCHAR)&ActHdr, sizeof(HEADER_802_11));
	FrameLen = sizeof(HEADER_802_11);

	InsertActField(pAd, (pbuf + FrameLen), &FrameLen,
		MT2_PEER_RM_CATE, RRM_MEASURE_REQ);
	InsertDialogToken(pAd, (pbuf + FrameLen), &FrameLen,
		p_bcn_req_data_raw->dialog_token);
	memcpy((pbuf + FrameLen), p_bcn_req_data_raw->bcn_req,
		p_bcn_req_data_raw->bcn_req_len);
	FrameLen += p_bcn_req_data_raw->bcn_req_len;

	MiniportMMRequest(pAd, (MGMT_USE_QUEUE_FLAG | QID_AC_BE),
		pbuf, FrameLen);

	pMeasureReqEntry = MeasureReqLookUp(pAd, MeasureReqToken);
	if (pMeasureReqEntry == NULL) {
		DBGPRINT(RT_DEBUG_ERROR,
			("%s() pMeasureReqEntry is NULL!\n", __func__));
		MlmeFreeMemory(pAd, pbuf);
		return;
	}
	pMeasureReqEntry->CurrentState = WAIT_BCN_REP;
	Timeout =  BCN_REP_TIMEOUT_VALUE;
	DBGPRINT(RT_DEBUG_TRACE,
			("%s() Timeout=%lds!\n", __func__, Timeout/1000));

	RTMPInitTimer(pAd, &pMeasureReqEntry->WaitBCNRepTimer,
		GET_TIMER_FUNCTION(WaitPeerBCNRepTimeout), pMeasureReqEntry, FALSE);
	RTMPSetTimer(&pMeasureReqEntry->WaitBCNRepTimer, Timeout);

	MlmeFreeMemory(pAd, pbuf);
	return;
}

enum BCN_STATE BCNPeerCurrentState(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	PBCN_EVENT_DATA Event = (PBCN_EVENT_DATA)Elem->Msg;
	PMEASURE_REQ_ENTRY pEntry = NULL;
	UINT8 MeasureReqToken = Event->MeasureReqToken;

	DBGPRINT(RT_DEBUG_TRACE,
			("%s, MeasureReqToken=%d!\n", __func__, MeasureReqToken));

	pEntry = MeasureReqLookUp(pAd, MeasureReqToken);
	if (pEntry != NULL) {
		DBGPRINT(RT_DEBUG_TRACE,
			  ("%s, CurrentState=%d!\n", __func__, pEntry->CurrentState));
		  return pEntry->CurrentState;
	} else {
		DBGPRINT(RT_DEBUG_TRACE,
			  ("%s, invalid MeasureReqToken!\n", __func__));
		return BCN_IDLE;
	}
}



VOID RRMBcnReqStateMachineInit(
			IN	PRTMP_ADAPTER pAd,
			IN	STATE_MACHINE * S,
			OUT STATE_MACHINE_FUNC	Trans[])
{
	DBGPRINT(RT_DEBUG_TRACE, ("%s\n", __func__));

	StateMachineInit(S, (STATE_MACHINE_FUNC *)Trans, MAX_BCN_STATE, MAX_BCN_MSG,
		(STATE_MACHINE_FUNC)Drop, BCN_IDLE, BCN_MACHINE_BASE);

#ifdef CONFIG_AP_SUPPORT
	StateMachineSetAction(S, WAIT_BCN_REQ, BCN_REQ, (STATE_MACHINE_FUNC)SendBcnReqToAir_SetParam);
	StateMachineSetAction(S, WAIT_BCN_REQ, BCN_REQ_RAW, (STATE_MACHINE_FUNC)SendBcnReqToAir_Raw);
	StateMachineSetAction(S, WAIT_BCN_REP, BCN_REP_TIMEOUT, (STATE_MACHINE_FUNC)BcnRepTimeout);
#endif /* CONFIG_AP_SUPPORT */
}


VOID NRStateMachineInit(
	IN	PRTMP_ADAPTER pAd,
	IN	STATE_MACHINE * S,
	OUT STATE_MACHINE_FUNC	Trans[])
{
	DBGPRINT(RT_DEBUG_TRACE, ("%s\n", __func__));

	StateMachineInit(S, (STATE_MACHINE_FUNC *)Trans, MAX_NR_STATE, MAX_NR_MSG,
		(STATE_MACHINE_FUNC)Drop, NR_UNKNOWN, NR_MACHINE_BASE);

#ifdef CONFIG_AP_SUPPORT
	StateMachineSetAction(S, WAIT_NR_RSP, NR_RSP, (STATE_MACHINE_FUNC)send_nr_resp_toair);
	StateMachineSetAction(S, WAIT_NR_RSP, NR_RSP_PARAM, (STATE_MACHINE_FUNC)send_nr_rsp_param_toair);
	StateMachineSetAction(S, WAIT_NR_RSP, NR_RSP_TIMEOUT, (STATE_MACHINE_FUNC)NRRspTimeout);
#endif /* CONFIG_AP_SUPPORT */
}


#endif /* DOT11K_RRM_SUPPORT */

