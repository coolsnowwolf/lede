/****************************************************************************
 * Ralink Tech Inc.
 * Taiwan, R.O.C.
 *
 * (c) Copyright 2010, Ralink Technology, Inc.
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

	All related TDLS UAPSD functions.

***************************************************************************/


#ifdef UAPSD_SUPPORT

#define MODULE_TDLS_UAPSD
#include "rt_config.h"

/* receive a traffic indication frame */
static VOID TDLS_UAPSD_PeerTrafficIndAction(
    IN	PRTMP_ADAPTER				pAd,
    IN	MLME_QUEUE_ELEM				*pElem);
/* receive a traffic response frame */
static VOID TDLS_UAPSD_PeerTrafficRspAction(
	IN	PRTMP_ADAPTER				pAd,
    IN	MLME_QUEUE_ELEM				*pElem);

/* build the traffic indication frame */
static ULONG TDLS_UAPSD_TrafficIndBuild(
	IN	PRTMP_ADAPTER				pAd,
	IN	UCHAR						*pPeerMac,
	OUT UCHAR						*pFrameBuf,
	OUT UCHAR						*pHeader802_3);
/* build the traffic indication frame payload */
static VOID TDLS_UAPSD_TrafficIndPayloadBuild(
	IN	PRTMP_ADAPTER				pAd,
	OUT PUCHAR						pFrameBuf,
	OUT PULONG						pFrameLen,
	IN	PRT_802_11_TDLS				pTDLS);

/* send a traffic indication frame */
static NDIS_STATUS TDLS_UAPSD_TrafficIndSend(
	IN	PRTMP_ADAPTER				pAd,
	IN	UCHAR						*pPeerMac);
/* send a traffic response frame */
static NDIS_STATUS TDLS_UAPSD_TrafficRspSend(
	IN	PRTMP_ADAPTER				pAd,
	IN	UCHAR						*pPeerMac,
	IN	UCHAR						PeerToken);
/* build the traffic response frame body */
static VOID TDLS_UAPSD_TrafficRspBuild(
	IN	PRTMP_ADAPTER				pAd,
	OUT PUCHAR						pFrameBuf,
	OUT PULONG						pFrameLen,
	IN	PRT_802_11_TDLS				pTDLS,
	IN	UCHAR						PeerToken);

/* get argument number value */
static UINT32 TDLS_UAPSD_CmdUtilNumGet(
	IN	CHAR						**ppArgv);
/* get argument MAC value */
static VOID TDLS_UAPSD_CmdUtilMacGet(
	IN	CHAR						**ppArgv,
	IN	UCHAR						*pDevMac);

/* simulate to send a TDLS Setup request to a peer */
static VOID TDLS_UAPSD_CmdSimSetupReqSend(
	IN	PRTMP_ADAPTER				pAd,
	IN	INT32						Argc,
	IN	CHAR						*pArgv);

/* simulate to receive a TDLS Traffic response from a peer */
static VOID TDLS_UAPSD_CmdSimTrafficRspRcv(
	IN	PRTMP_ADAPTER				pAd,
	IN	INT32						Argc,
	IN	CHAR						*pArgv);

/* simulate to receive a TDLS Traffic indication from a peer */
static VOID TDLS_UAPSD_CmdSimTrafficIndRcv(
	IN	PRTMP_ADAPTER				pAd,
	IN	INT32						Argc,
	IN	CHAR						*pArgv);

/* simulate to change the power save of a peer */
static VOID TDLS_UAPSD_CmdSimPeerPowerSaveChg(
	IN	PRTMP_ADAPTER				pAd,
	IN	INT32						Argc,
	IN	CHAR						*pArgv);

/* simulate to change our power save */
static VOID TDLS_UAPSD_CmdSimSelfPowerSaveChg(
	IN	PRTMP_ADAPTER				pAd,
	IN	INT32						Argc,
	IN	CHAR						*pArgv);

/* simulate to send a TDLS Traffic response to a peer */
static VOID TDLS_UAPSD_CmdSimTrafficRspSnd(
	IN	PRTMP_ADAPTER				pAd,
	IN	INT32						Argc,
	IN	CHAR						*pArgv);




/* public functions */
/*
========================================================================
Routine Description:
	Initialze TDLS UAPSD function.

Arguments:
	pAd				- WLAN control block pointer
	pFSM			- TDLS Finite State Machine

Return Value:
	TRUE			- init ok
	FALSE			- init fail

Note:
	Peer U-APSD Sleep STA is default feature in spec.
	Peer U-APSD Buffer STA is optional feature in spec.
========================================================================
*/
BOOLEAN TDLS_UAPSDP_Init(
	IN	PRTMP_ADAPTER				pAd,
    IN	STATE_MACHINE				*pFSM)
{
	/* init FSM */
    StateMachineSetAction(pFSM, TDLS_IDLE, MT2_PEER_TDLS_TRAFFIC_IND,
						(STATE_MACHINE_FUNC)TDLS_UAPSD_PeerTrafficIndAction);
    StateMachineSetAction(pFSM, TDLS_IDLE, MT2_PEER_TDLS_TRAFFIC_RSP,
						(STATE_MACHINE_FUNC)TDLS_UAPSD_PeerTrafficRspAction);

	/* init lock */
	NdisAllocateSpinLock(pAd, &pAd->StaCfg.TdlsInfo.TDLSUapsdLock);

	DBGPRINT(RT_DEBUG_TRACE, ("tdls uapsd> initialization ok!\n"));
	return TRUE;
}


/*
========================================================================
Routine Description:
	Release TDLS UAPSD function.

Arguments:
	pAd				- WLAN control block pointer

Return Value:
	TRUE			- release ok
	FALSE			- release fail

Note:
========================================================================
*/
BOOLEAN TDLS_UAPSDP_Release(
	IN	PRTMP_ADAPTER				pAd)
{
	/* free lock */
	NdisFreeSpinLock(&pAd->StaCfg.TdlsInfo.TDLSUapsdLock);

	return TRUE;
}


/*
========================================================================
Routine Description:
	Send a traffic indication frame.

Arguments:
	pAd				- WLAN control block pointer
	pPeerMac		- the peer MAC

Return Value:
	NDIS_STATUS_SUCCESS
	NDIS_STATUS_FAILURE

Note:
	11.2.1.14.1 Peer U-APSD Behavior at the PU buffer STA

	A PU buffer STA shall transmit a unicast TDLS Peer Traffic
	Indication frame to a PU sleep STA, through the AP, if and only if
	all of the following conditions are met:

	-- A frame with an RA corresponding to a PU sleep STA was placed
		into a buffer at the PU buffer STA;
	-- The buffer into which the frame was placed contained no other
		frames with the same RA; and
	-- One or more periods of dot11TDLSPeerUAPSDIndicationWindow
		beacon intervals have expired after the last service period.
========================================================================
*/
NDIS_STATUS TDLS_UAPSDP_TrafficIndSend(
	IN	PRTMP_ADAPTER				pAd,
	IN	UCHAR						*pPeerMac)
{
	NDIS_STATUS	NStatus;


	TDLS_SEMLOCK(pAd);
	NStatus = TDLS_UAPSD_TrafficIndSend(pAd, pPeerMac);
	TDLS_SEMUNLOCK(pAd);

	return NStatus;
}


/*
========================================================================
Routine Description:
	Check if ASIC can go to sleep mode.

Arguments:
	pAd				- WLAN control block pointer

Return Value:
	None

Note:
	Check all TDLS entries and return TRUE if all SPs are closed.
========================================================================
*/
BOOLEAN TDLS_UAPSDP_AsicCanSleep(
	IN	PRTMP_ADAPTER				pAd)
{
	RT_802_11_TDLS *pTDLS;
	UINT32 IdEntry;
	BOOLEAN FlgAllSpClosed = TRUE;


	TDLS_SEMLOCK(pAd);

	/* check if all SPs are closed */
	for(IdEntry=0; IdEntry<MAX_NUM_OF_TDLS_ENTRY; IdEntry++)
	{
		pTDLS = (PRT_802_11_TDLS)&pAd->StaCfg.TdlsInfo.TDLSEntry[IdEntry];

		if ((pTDLS->Valid == TRUE) &&
			(pTDLS->Status == TDLS_MODE_CONNECTED))
		{
			UINT32 Wcid = pTDLS->MacTabMatchWCID;
			PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[Wcid];


			if (!IS_ENTRY_TDLS(pEntry))
				continue;

			/*
				Two cases we can not sleep:
				1. One of any SP is not ended.
				2. A traffic indication is sent and no response is received.
			*/
			if ((pEntry->bAPSDFlagSPStart != 0) ||
				(pTDLS->FlgIsWaitingUapsdTraRsp == TRUE))
			{
				DBGPRINT(RT_DEBUG_TRACE,
						("tdls uapsd> SP not close or Ind sent (%d %d)!\n",
						pEntry->bAPSDFlagSPStart,
						pTDLS->FlgIsWaitingUapsdTraRsp));
				hex_dump("pEntry=", pEntry->Addr, 6);
				FlgAllSpClosed = FALSE;
				break;
			}
		}
	}

	TDLS_SEMUNLOCK(pAd);

	return FlgAllSpClosed;
}


/*
========================================================================
Routine Description:
	Check if ASIC can go to sleep mode.

Arguments:
	pAd				- WLAN control block pointer
	PsmOld			- Current power save mode
	PsmNew			- New power save mode

Return Value:
	None

Note:
========================================================================
*/
VOID TDLS_UAPSDP_PsmModeChange(
	IN	PRTMP_ADAPTER				pAd,
	IN	USHORT						PsmOld,
	IN	USHORT						PsmNew)
{
	MAC_TABLE_ENTRY	*pMacEntry;
	RT_802_11_TDLS *pTDLS;
	UINT32 IdTdls;
	struct wifi_dev *wdev =&pAd->StaCfg.wdev;

	if (PsmOld == PsmNew)
		return; /* no inform needs */

	/* sanity check */
    /* WPA 802.1x secured port control */
    if (((wdev->AuthMode == Ndis802_11AuthModeWPA) || 
         (wdev->AuthMode == Ndis802_11AuthModeWPAPSK) ||
         (wdev->AuthMode == Ndis802_11AuthModeWPA2) || 
         (wdev->AuthMode == Ndis802_11AuthModeWPA2PSK)
#ifdef WPA_SUPPLICANT_SUPPORT
		|| (wdev->IEEE8021X == TRUE)		
#endif 		
#ifdef WAPI_SUPPORT
		  || (wdev->AuthMode == Ndis802_11AuthModeWAICERT)
		  || (wdev->AuthMode == Ndis802_11AuthModeWAIPSK)
#endif /* WAPI_SUPPORT */
        ) &&
       (wdev->PortSecured == WPA_802_1X_PORT_NOT_SECURED)) 
	{
		return; /* port not yet secure */
	}

	DBGPRINT(RT_DEBUG_TRACE, ("tdls uapsd> our PSM mode change!\n"));

	/* indicate the peer */
	TDLS_SEMLOCK(pAd);

	for(IdTdls=0; IdTdls<MAX_NUM_OF_TDLS_ENTRY; IdTdls++)
	{
		pTDLS = (RT_802_11_TDLS *) &pAd->StaCfg.TdlsInfo.TDLSEntry[IdTdls];

		if ((pTDLS->Valid) && (pTDLS->Status == TDLS_MODE_CONNECTED))
		{
			/* get MAC Entry */
			pMacEntry = MacTableLookup(pAd, pTDLS->MacAddr);
			if (pMacEntry == NULL)
				continue; /* check next one */

			/* check if the peer is in ACTIVE mode */
			if (TDLS_UAPSD_ARE_PEER_IN_ACTIVE(pMacEntry))
			{
				/*
					pMacEntry->RssiSample.LastRssi0 is used to check if
					we have ever received any packet from the peer.
				*/
				/* send a null frame to the peer directly */
				DBGPRINT(RT_DEBUG_TRACE, ("tdls uapsd> send a NULL frame!\n"));

				RtmpEnqueueNullFrame(pAd, pMacEntry->Addr,
									pAd->CommonCfg.TxRate, pMacEntry->Aid,
									pMacEntry->apidx, TRUE, FALSE, 0);
				continue;
			}

			/*
				Send traffic indication frame to the peer when the peer
				is in power-save mode.
			*/
			TDLS_UAPSD_TrafficIndSend(pAd, pMacEntry->Addr);
		}
	}

	TDLS_SEMUNLOCK(pAd);
}




/* private functions */
/*
========================================================================
Routine Description:
	Display the UAPSD information for a peer.

Arguments:
	pAd				- WLAN control block pointer
	Argc			- the number of input parameters
	*pArgv			- input parameters

Return Value:
	None

Note:
	1. Command Format:
		iwpriv ra0 set tdls=01_[PEER MAC]
========================================================================
*/
static VOID TDLS_UAPSD_CmdPeerInfoDisplay(
	IN	PRTMP_ADAPTER				pAd,
	IN	INT32						Argc,
	IN	CHAR						*pArgv)
{
	MAC_TABLE_ENTRY	*pMacEntry;
	UCHAR PeerMac[6];
	UINT32 IdAcNum;


	/* get MAC address */
	TDLS_UAPSD_CmdUtilMacGet(&pArgv, PeerMac);

	/* get pEntry */
	pMacEntry = MacTableLookup(pAd, PeerMac);

	if (pMacEntry == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("tdls_cmd> ERROR! No such peer!\n"));
		return;
	}

	/* display UAPSD information */
	if (TDLS_UAPSD_ARE_PEER_IN_PS(pMacEntry))
		printk("\n    EDCA AC UAPSD information: (POWER SAVE)\n");
	else
		printk("\n    EDCA AC UAPSD information: (ACTIVE)\n");
	/* End of if */

	if (pMacEntry->MaxSPLength != 0)
	{
		printk("    Max SP Length: %d (%d frames)\n",
				pMacEntry->MaxSPLength, pMacEntry->MaxSPLength<<1);
	}
	else
		printk("    Max SP Length: 0 (all frames)\n");
	/* End of if */

	printk("    UAPSD/AC   AC0    AC1    AC2    AC3");
	printk("\n    Tr/De      ");

	for(IdAcNum=0; IdAcNum<4; IdAcNum++)
	{
		printk("%d/%d    ",
				pMacEntry->bAPSDCapablePerAC[IdAcNum],
				pMacEntry->bAPSDDeliverEnabledPerAC[IdAcNum]);
	} /* End of for */

	printk("\n");
}


/*
========================================================================
Routine Description:
	Display our UAPSD information.

Arguments:
	pAd				- WLAN control block pointer
	Argc			- the number of input parameters
	*pArgv			- input parameters

Return Value:
	None

Note:
	1. Command Format:
		iwpriv ra0 set tdls=02

	2. 11.2.1.14 Peer U-APSD
		A STA that configured Peer U-APSD at a TDLS peer STA enters power save
		mode on a TDLS direct link after the successful transmission to the
		TDLS peer STA over the direct link of an acknowledged MPDU with the
		Power Management field set to one.
========================================================================
*/
static VOID TDLS_UAPSD_CmdSelfInfoDisplay(
	IN	PRTMP_ADAPTER				pAd,
	IN	INT32						Argc,
	IN	CHAR						*pArgv)
{
	if (TDLS_UAPSD_ARE_WE_IN_PS(pAd))
		printk("\n    EDCA AC UAPSD information: (POWER SAVE)\n");
	else
		printk("\n    EDCA AC UAPSD information: (ACTIVE)\n");
	/* End of if */

	if (pAd->CommonCfg.MaxSPLength != 0)
	{
		printk("    Max SP Length: %d (%d frames)\n",
				pAd->CommonCfg.MaxSPLength, pAd->CommonCfg.MaxSPLength<<1);
	}
	else
		printk("    Max SP Length: 0 (all frames)\n");
	/* End of if */

	printk("      AP UAPSD/AC   AC0    AC1    AC2    AC3");

	printk("\n         Tr/De      %d/%d    %d/%d    %d/%d    %d/%d\n\n",
			pAd->CommonCfg.bACMAPSDTr[0],
			pAd->CommonCfg.bAPSDAC_BE,
			pAd->CommonCfg.bACMAPSDTr[1],
			pAd->CommonCfg.bAPSDAC_BK,
			pAd->CommonCfg.bACMAPSDTr[2],
			pAd->CommonCfg.bAPSDAC_VI,
			pAd->CommonCfg.bACMAPSDTr[3],
			pAd->CommonCfg.bAPSDAC_VO);

	printk("    TDLS UAPSD/AC   AC0    AC1    AC2    AC3");

	printk("\n         Tr/De      %d/%d    %d/%d    %d/%d    %d/%d\n",
			pAd->CommonCfg.TDLS_bAPSDAC_BE,
			pAd->CommonCfg.TDLS_bAPSDAC_BE,
			pAd->CommonCfg.TDLS_bAPSDAC_BK,
			pAd->CommonCfg.TDLS_bAPSDAC_BK,
			pAd->CommonCfg.TDLS_bAPSDAC_VI,
			pAd->CommonCfg.TDLS_bAPSDAC_VI,
			pAd->CommonCfg.TDLS_bAPSDAC_VO,
			pAd->CommonCfg.TDLS_bAPSDAC_VO);

	printk("\n");
}


/*
========================================================================
Routine Description:
	Configure our UAPSD information.

Arguments:
	pAd				- WLAN control block pointer
	Argc			- the number of input parameters
	*pArgv			- input parameters

Return Value:
	None

Note:
	1. Command Format:
		iwpriv ra0 set tdls=03_1_1_1_1_0

		[UAPSD for BE] [UAPSD for BK] [UAPSD for VI] [UAPSD for VO]
		[Max SP length]
========================================================================
*/
static VOID TDLS_UAPSD_CmdConfigure(
	IN	PRTMP_ADAPTER				pAd,
	IN	INT32						Argc,
	IN	CHAR						*pArgv)
{
	pAd->CommonCfg.TDLS_bAPSDAC_BE = TDLS_UAPSD_CmdUtilNumGet(&pArgv);
	pAd->CommonCfg.TDLS_bAPSDAC_BK = TDLS_UAPSD_CmdUtilNumGet(&pArgv);
	pAd->CommonCfg.TDLS_bAPSDAC_VI = TDLS_UAPSD_CmdUtilNumGet(&pArgv);
	pAd->CommonCfg.TDLS_bAPSDAC_VO = TDLS_UAPSD_CmdUtilNumGet(&pArgv);
	pAd->CommonCfg.TDLS_MaxSPLength = TDLS_UAPSD_CmdUtilNumGet(&pArgv);

	DBGPRINT(RT_DEBUG_TRACE,
			("tdls_cmd> configure our UAPSD to %d %d %d %d %d\n",
			pAd->CommonCfg.TDLS_bAPSDAC_BE,
			pAd->CommonCfg.TDLS_bAPSDAC_BK,
			pAd->CommonCfg.TDLS_bAPSDAC_VI,
			pAd->CommonCfg.TDLS_bAPSDAC_VO,
			pAd->CommonCfg.TDLS_MaxSPLength));
}




#define TDLS_UAPSD_PEER_SHOW			01 /* tdls uapsd peer display */
#define TDLS_UAPSD_SELF_SHOW			02 /* tdls uapsd self display */
#define TDLS_UAPSD_CONFIG				03 /* tdls uapsd configure */

#ifdef TDLS_UAPSD_DEBUG
#define TDLS_UAPSD_SIM_SETUP_REQ		51 /* tdls setup request */
#define TDLS_UAPSD_SIM_TRAFFIC_RSP_RCV	52 /* tdls traffic rsp receive */
#define TDLS_UAPSD_SIM_TRAFFIC_IND_RCV	53 /* tdls traffic ind receive */
#define TDLS_UAPSD_SIM_PEER_PS			54 /* tdls peer ps mode change */
#define TDLS_UAPSD_SIM_PS				55 /* tdls ps mode change */
#define TDLS_UAPSD_SIM_TRAFFIC_RSP_SND	56 /* tdls traffic rsp send */
#define TDLS_UAPSD_SIM_BEACON_LOST		57 /* tdls beacon lost */
#endif /* TDLS_UAPSD_DEBUG */

/*
========================================================================
Routine Description:
	Test command.

Arguments:
	pAd				- WLAN control block pointer
	pArgvIn			- the data flow information

Return Value:
	0				- OK
	others			- FAIL
========================================================================
*/
INT TDLS_Ioctl(
	IN	PRTMP_ADAPTER				pAd,
	IN	PSTRING						pArgvIn)
{
	CHAR BufCmd[3] = { 0, 0, 0 };
	CHAR *pArgv, *pParam;
	UINT32 Command;
	INT32 Argc;


	/* init */
	pArgv = (CHAR *)pArgvIn;

	/* get command type */
	/* command format is iwpriv ra0 set tdls=[cmd id]_[arg1]_......_[argn] */
	NdisCopyMemory(BufCmd, pArgv, 2);
	Command = simple_strtol((PSTRING)BufCmd, 0, 10);
	pArgv += 2; /* skip command field */

	/* get Argc number */
	Argc = 0;
	pParam = pArgv;

	while(1)
	{
		if (*pParam == '_')
			Argc ++;
		/* End of if */

		if ((*pParam == 0x00) || (Argc > 20))
			break;
		/* End of if */

		pParam++;
	} /* End of while */

	pArgv++; /* skip _ points to arg1 */

	/* handle the command */
	switch(Command)
	{
		case TDLS_UAPSD_PEER_SHOW: /* display peer uapsd info */
			DBGPRINT(RT_DEBUG_TRACE,
					("tdls_cmd> display peer UAPSD information\n"));
			TDLS_UAPSD_CmdPeerInfoDisplay(pAd, Argc, pArgv);
			break;

		case TDLS_UAPSD_SELF_SHOW: /* display self uapsd info */
			DBGPRINT(RT_DEBUG_TRACE,
					("tdls_cmd> display self UAPSD information\n"));
			TDLS_UAPSD_CmdSelfInfoDisplay(pAd, Argc, pArgv);
			break;

		case TDLS_UAPSD_CONFIG: /* tdls uapsd configure */
			DBGPRINT(RT_DEBUG_TRACE,
					("tdls_cmd> configure our UAPSD\n"));
			TDLS_UAPSD_CmdConfigure(pAd, Argc, pArgv);
			break;

#ifdef TDLS_UAPSD_DEBUG
		case TDLS_UAPSD_SIM_SETUP_REQ: /* tdls setup request */
			DBGPRINT(RT_DEBUG_TRACE,
					("tdls_cmd> simulate to send a setup request\n"));
			TDLS_UAPSD_CmdSimSetupReqSend(pAd, Argc, pArgv);
			break;

		case TDLS_UAPSD_SIM_TRAFFIC_RSP_RCV: /* tdls traffic response */
			DBGPRINT(RT_DEBUG_TRACE,
					("tdls_cmd> simulate to receive a traffic response\n"));
			TDLS_UAPSD_CmdSimTrafficRspRcv(pAd, Argc, pArgv);
			break;

		case TDLS_UAPSD_SIM_TRAFFIC_IND_RCV: /* tdls traffic indication */
			DBGPRINT(RT_DEBUG_TRACE,
					("tdls_cmd> simulate to receive a traffic indication\n"));
			TDLS_UAPSD_CmdSimTrafficIndRcv(pAd, Argc, pArgv);
			break;

		case TDLS_UAPSD_SIM_PEER_PS: /* tdls peer power save */
			DBGPRINT(RT_DEBUG_TRACE,
					("tdls_cmd> simulate to change the peer power save\n"));
			TDLS_UAPSD_CmdSimPeerPowerSaveChg(pAd, Argc, pArgv);
			break;

		case TDLS_UAPSD_SIM_PS: /* tdls power save */
			DBGPRINT(RT_DEBUG_TRACE,
					("tdls_cmd> simulate to change our power save\n"));
			TDLS_UAPSD_CmdSimSelfPowerSaveChg(pAd, Argc, pArgv);
			break;

		case TDLS_UAPSD_SIM_TRAFFIC_RSP_SND: /* tdls traffic response */
			DBGPRINT(RT_DEBUG_TRACE,
					("tdls_cmd> simulate to send traffic response\n"));
			TDLS_UAPSD_CmdSimTrafficRspSnd(pAd, Argc, pArgv);
			break;

		case TDLS_UAPSD_SIM_BEACON_LOST: /* tdls beacon lost */
			DBGPRINT(RT_DEBUG_TRACE,
					("tdls_cmd> simulate the beacon lost case\n"));
			pAd->Mlme.ChannelQuality = 0;

			if (pAd->StaCfg.bAutoConnectByBssid)
			   pAd->StaCfg.bAutoConnectByBssid = FALSE;

			pAd->MlmeAux.CurrReqIsFromNdis = FALSE;

			/* Lost AP, send disconnect & link down event*/
			LinkDown(pAd, FALSE);
			break;
#endif /* TDLS_UAPSD_DEBUG */

		default: /* error command type */
			DBGPRINT(RT_DEBUG_ERROR, ("tdls_cmd> ERROR! No such command!\n"));
			return -EINVAL; /* input error */
	} /* End of switch */

	return 0; /* ok */
}


/*
========================================================================
Routine Description:
	Set our UAPSD.

Arguments:
	pAd				- WLAN control block pointer
	pArgvIn			- the data flow information

Return Value:
	0				- OK
	others			- FAIL
========================================================================
*/
INT Set_TdlsUapsdProc(
	IN	PRTMP_ADAPTER				pAd,
	IN	PSTRING						pArgvIn)
{
	return TDLS_Ioctl(pAd, pArgvIn);
}




/* private function */
/*
========================================================================
Routine Description:
	Build the traffic indication frame.

Arguments:
	pAd				- WLAN control block pointer
	pPeerMac		- the peer
	pFrameBuf		- frame
	pHeader802_3	- frame header

Return Value:
	Frame Length

Note:
========================================================================
*/
static ULONG TDLS_UAPSD_TrafficIndBuild(
	IN	PRTMP_ADAPTER				pAd,
	IN	UCHAR						*pPeerMac,
	OUT UCHAR						*pFrameBuf,
	OUT UCHAR						*pHeader802_3)
{
	RT_802_11_TDLS *pTDLS = NULL;
	UCHAR	TDLS_ETHERTYPE[] = {0x89, 0x0d};
	ULONG	FrameLen = 0;
	INT32	LinkId;
	BOOLEAN TimerCancelled;


	DBGPRINT(RT_DEBUG_TRACE, ("====> %s\n", __FUNCTION__));

	/* search TDLS entry */
	LinkId = TDLS_SearchLinkId(pAd, pPeerMac);
	if (TDLS_UAPSD_IS_LINK_INVALID(LinkId))
	{
		DBGPRINT(RT_DEBUG_TRACE,
				("%s: can not find the LinkId!\n", __FUNCTION__));
		TDLS_UAPSD_REBUILD_LINK(pAd, pPeerMac);
		goto LabelExit;
	}

	DBGPRINT(RT_DEBUG_TRACE, ("tdls uapsd> LinkId = %d\n", LinkId));

	pTDLS = TDLS_UAPSD_ENTRY_GET(pAd, LinkId);

	/* sanity check */
	if (TDLS_UAPSD_IS_CONN_NOT_BUILT(pTDLS))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("tdls uapsd> link is not yet built "
				"so we can not send a traffic ind to the peer!!!"));
		goto LabelExit;
	}

	if (pTDLS->FlgIsWaitingUapsdTraRsp == TRUE)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("tdls uapsd> traffic ind was sent before!\n"));
		goto LabelExit; /* has sent it */
	}

	pTDLS->FlgIsWaitingUapsdTraRsp = TRUE;

	/* init packet header */
	MAKE_802_3_HEADER(pHeader802_3, pTDLS->MacAddr,
						pAd->CurrentAddress, TDLS_ETHERTYPE);

	/* build the frame */
	TDLS_UAPSD_TrafficIndPayloadBuild(pAd, pFrameBuf, &FrameLen, pTDLS);

	hex_dump("TDLS UAPSD Peer Traffic Ind sending packet", pFrameBuf, FrameLen);
	/*
		11.2.1.14.1 Peer U-APSD Behavior at the PU buffer STA
		When no corresponding TDLS Peer Traffic Response frame has been
		received within dot11TDLSResponseTimeout after sending a TDLS Peer
		Traffic Indication frame, the STA shall tear down the direct link.

		The default value is 5 seconds.
	*/
	/* set traffic indication timer */
	RTMPCancelTimer(&pTDLS->Timer, &TimerCancelled);
	RTMPSetTimer(&pTDLS->Timer, TDLS_TIMEOUT);

	/* free resources */
LabelExit:
	return FrameLen;
}


/*
========================================================================
Routine Description:
	Build the traffic indication frame payload.

Arguments:
	pAd				- WLAN control block pointer
	pFrameBuf		- frame
	pFrameLen		- frame length
	pTDLS			- TDLS link entry

Return Value:
	None

Note:
========================================================================
*/
static VOID TDLS_UAPSD_TrafficIndPayloadBuild(
	IN	PRTMP_ADAPTER				pAd,
	OUT PUCHAR						pFrameBuf,
	OUT PULONG						pFrameLen,
	IN	PRT_802_11_TDLS				pTDLS)
{
	UCHAR RemoteFrameType = PROTO_NAME_TDLS;
	ULONG TempLen;
	UCHAR Token;			// Dialog token


	/* fill remote frame type */
	MakeOutgoingFrame(pFrameBuf,		&TempLen,
						1,				&RemoteFrameType,
						END_OF_ARGS);
	*pFrameLen = TempLen;

	/* fill action code */
	TDLS_InsertActField(pAd, (pFrameBuf + *pFrameLen), pFrameLen,
						CATEGORY_TDLS, TDLS_ACTION_CODE_PEER_TRAFFIC_INDICATION);

	/* fill Dialog Token */
	TDLS_UAPSD_DIALOG_GET(pAd, Token);
	TDLS_InsertDialogToken(pAd, (pFrameBuf + *pFrameLen), pFrameLen,
							Token);

	/* fill link identifier */
	if (pTDLS->bInitiator)
		TDLS_InsertLinkIdentifierIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen,
									pTDLS->MacAddr, pAd->CurrentAddress);
	else
		TDLS_InsertLinkIdentifierIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen,
									pAd->CurrentAddress, pTDLS->MacAddr);

	/* fill PU buffer status */
	TDLS_InsertPuBufferStatus(pAd, (pFrameBuf + *pFrameLen), pFrameLen,
							pTDLS->MacAddr);
}


/*
========================================================================
Routine Description:
	Send a traffic indication frame.

Arguments:
	pAd				- WLAN control block pointer
	pPeerMac		- the peer MAC

Return Value:
	NDIS_STATUS_SUCCESS
	NDIS_STATUS_FAILURE

Note:
	11.2.1.14.1 Peer U-APSD Behavior at the PU buffer STA

	A PU buffer STA shall transmit a unicast TDLS Peer Traffic
	Indication frame to a PU sleep STA, through the AP, if and only if
	all of the following conditions are met:

	-- A frame with an RA corresponding to a PU sleep STA was placed
		into a buffer at the PU buffer STA;
	-- The buffer into which the frame was placed contained no other
		frames with the same RA; and
	-- One or more periods of dot11TDLSPeerUAPSDIndicationWindow
		beacon intervals have expired after the last service period.
========================================================================
*/
static NDIS_STATUS TDLS_UAPSD_TrafficIndSend(
	IN	PRTMP_ADAPTER				pAd,
	IN	UCHAR						*pPeerMac)
{
	PUCHAR pOutBuffer = NULL;
	ULONG FrameLen = 0;
	UCHAR Header802_3[14];
	NDIS_STATUS	NStatus = NDIS_STATUS_FAILURE;


	DBGPRINT(RT_DEBUG_TRACE, ("====> %s\n", __FUNCTION__));

	/* allocate resources */
	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
	if (NStatus	!= NDIS_STATUS_SUCCESS)
		goto LabelExit;

	FrameLen = TDLS_UAPSD_TrafficIndBuild(pAd, pPeerMac,
										pOutBuffer, Header802_3);
	if (FrameLen <= 0)
		goto LabelExit;

	/*
		Keep ACTIVE and do not enter sleep mode until all EOSPs are sent
		and we will wake up our ASIC in STAHardTransmit() of
		TDLS_UAPSD_PKT_SEND_THROUGH_AP() if we are sleep.
	*/
	ASIC_PS_CAN_NOT_SLEEP(pAd);

	/* send the frame to the peer with AP's help */
	TDLS_UAPSD_PKT_SEND_THROUGH_AP(pAd, Header802_3, pOutBuffer, FrameLen);
/*	hex_dump("TDLS traffic indication send pack", pOutBuffer, FrameLen); */

	NStatus = NDIS_STATUS_SUCCESS;

	/* free resources */
LabelExit:
	if (pOutBuffer != NULL)
		MlmeFreeMemory(pAd, pOutBuffer);
	return NStatus;
}


/*
========================================================================
Routine Description:
	Send a traffic response frame.

Arguments:
	pAd				- WLAN control block pointer
	pTDLS			- the peer entry

Return Value:
	NDIS_STATUS_SUCCESS
	NDIS_STATUS_FAILURE

Note:
========================================================================
*/
static NDIS_STATUS TDLS_UAPSD_TrafficRspSend(
	IN	PRTMP_ADAPTER				pAd,
	IN	UCHAR						*pPeerMac,
	IN	UCHAR						PeerToken)
{
	MAC_TABLE_ENTRY	*pMacEntry; 
	RT_802_11_TDLS *pTDLS = NULL;
	UCHAR	TDLS_ETHERTYPE[] = {0x89, 0x0d};
	UCHAR	Header802_3[14];
	PUCHAR	pOutBuffer = NULL;
	ULONG	FrameLen = 0;
	ULONG	TempLen;
	INT32	LinkId;
	UCHAR	RemoteFrameType = PROTO_NAME_TDLS;
	NDIS_STATUS	NStatus = NDIS_STATUS_FAILURE;


	DBGPRINT(RT_DEBUG_TRACE, ("====> %s\n", __FUNCTION__));

	/* search TDLS entry */
	LinkId = TDLS_SearchLinkId(pAd, pPeerMac);
	if (TDLS_UAPSD_IS_LINK_INVALID(LinkId))
	{
		DBGPRINT(RT_DEBUG_TRACE,
				("%s: can not find the LinkId!\n", __FUNCTION__));
		TDLS_UAPSD_REBUILD_LINK(pAd, pPeerMac);
		goto LabelExit;
	}

	DBGPRINT(RT_DEBUG_TRACE, ("tdls uapsd> LinkId = %d\n", LinkId));

	pTDLS = TDLS_UAPSD_ENTRY_GET(pAd, LinkId);

	/* sanity check */
	if (TDLS_UAPSD_IS_CONN_NOT_BUILT(pTDLS))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("tdls uapsd> link is not yet built "
				"so we can not send a traffic ind to the peer!!!"));
		goto LabelExit;
	}

	/* init */
	MAKE_802_3_HEADER(Header802_3, pTDLS->MacAddr,
						pAd->CurrentAddress, TDLS_ETHERTYPE);

	/* allocate buffer for transmitting message */
	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
	if (NStatus	!= NDIS_STATUS_SUCCESS)
		goto LabelExit;

	/* build the frame */
	MakeOutgoingFrame(pOutBuffer,		&TempLen,
						1,				&RemoteFrameType,
						END_OF_ARGS);
	FrameLen = FrameLen + TempLen;

	TDLS_UAPSD_TrafficRspBuild(pAd, pOutBuffer, &FrameLen, pTDLS, PeerToken);

	hex_dump("TDLS UAPSD Peer Traffic Response sending packet", pOutBuffer, FrameLen);

	/* need to set the power save mode of the peer to ACTIVE */
	/* we will recover its mode after EOSP frame is received */
	pMacEntry = MacTableLookup(pAd, pTDLS->MacAddr);
	if (pMacEntry == NULL)
		goto LabelExit;

	/* peer can not sleep for a while */
		RTMP_PS_VIRTUAL_WAKEUP_PEER(pMacEntry);

	/* send the frame to the peer without AP's help */
	TDLS_UAPSD_PKT_SEND_TO_PEER(pAd, Header802_3, pOutBuffer, FrameLen, pTDLS);
/*	hex_dump("TDLS traffic response send pack", pOutBuffer, FrameLen); */

	NStatus = NDIS_STATUS_SUCCESS;

	/* free resources */
LabelExit:
	if (pOutBuffer != NULL)
		MlmeFreeMemory(pAd, pOutBuffer);
	return NStatus;
}


/*
========================================================================
Routine Description:
	Build the traffic response frame body.

Arguments:
	pAd				- WLAN control block pointer
	pFrameBuf		- frame
	pFrameLen		- frame length
	pTDLS			- TDLS link entry

Return Value:
	None

Note:
========================================================================
*/
static VOID TDLS_UAPSD_TrafficRspBuild(
	IN	PRTMP_ADAPTER				pAd,
	OUT PUCHAR						pFrameBuf,
	OUT PULONG						pFrameLen,
	IN	PRT_802_11_TDLS				pTDLS,
	IN	UCHAR						PeerToken)
{
	/* fill action code */
	TDLS_InsertActField(pAd, (pFrameBuf + *pFrameLen), pFrameLen,
						CATEGORY_TDLS, TDLS_ACTION_CODE_PEER_TRAFFIC_RESPONSE);

	/* fill Dialog Token */
	TDLS_InsertDialogToken(pAd, (pFrameBuf + *pFrameLen), pFrameLen,
							PeerToken);

	/* fill link identifier */
	if (pTDLS->bInitiator)
	{
		TDLS_InsertLinkIdentifierIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen,
									pTDLS->MacAddr, pAd->CurrentAddress);
	}
	else
	{
		TDLS_InsertLinkIdentifierIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen,
									pAd->CurrentAddress, pTDLS->MacAddr);
	}
}


/*
========================================================================
Routine Description:
	Receive a traffic indication frame.

Arguments:
	pAd				- WLAN control block pointer
	pElem			- the frame information

Return Value:
	None

Note:
========================================================================
*/
static VOID TDLS_UAPSD_PeerTrafficIndAction(
    IN	PRTMP_ADAPTER				pAd,
    IN	MLME_QUEUE_ELEM				*pElem)
{
	UCHAR Token;
	UCHAR PeerAddr[6];
	UCHAR PeerAddr1[6];
	ULONG OffsetPuBuff;
	INT LinkId = 0xff;
	PRT_802_11_TDLS	pTDLS = NULL;
	PFRAME_802_11 pFrame = (PFRAME_802_11)pElem->Msg;

	DBGPRINT(RT_DEBUG_TRACE, ("tdls uapsd> ====> %s\n", __FUNCTION__));

	/* Not TDLS Capable, ignore it */
	if (!IS_TDLS_SUPPORT(pAd))
		return;

	/* Not BSS mode, ignore it */
	if (!INFRA_ON(pAd))
		return;

	hex_dump("TDLS UAPSD Peer Traffic Ind receive pack", pElem->Msg, pElem->MsgLen);

	/* sanity check */
	if (TDLS_UAPSD_ARE_WE_IN_ACTIVE(pAd))
		return; /* we are not in power-save mode */

	COPY_MAC_ADDR(PeerAddr, &pFrame->Hdr.Addr3);
	// Drop not within my TDLS Table that created before !
	LinkId = TDLS_SearchLinkId(pAd, PeerAddr);
	if (LinkId == -1 || LinkId == MAX_NUM_OF_TDLS_ENTRY)
	{
		DBGPRINT(RT_DEBUG_ERROR,("TDLS - TDLS_UAPSD_PeerTrafficIndAction() can not find the LinkId!\n"));
		return;
	}

	// Point to the current Link ID
	pTDLS = (PRT_802_11_TDLS)&pAd->StaCfg.TdlsInfo.TDLSEntry[LinkId];

	OffsetPuBuff = PeerTdlsBasicSanity(pAd,
									pElem->Msg,
									pElem->MsgLen,
									pTDLS->bInitiator,
									&Token,
									PeerAddr1);
	if (OffsetPuBuff <= 0)
		return;

/*	hex_dump("PeerAddr=", PeerAddr, 6); */
	DBGPRINT(RT_DEBUG_ERROR, ("tdls uapsd> PU Buffer Status = 0x%x\n",
			pElem->Msg[OffsetPuBuff+2])); /* 2: skip ID and length field */

	/* reply a response frame with UP = 5 */
	/* for TDLS UAPSD, all AC will be UAPSD mode */
	TDLS_UAPSD_TrafficRspSend(pAd, PeerAddr, Token);
}


/*
========================================================================
Routine Description:
	Receive a traffic response frame.

Arguments:
	pAd				- WLAN control block pointer
	pElem			- the frame information
Return Value:
	None

Note:
========================================================================
*/
static VOID TDLS_UAPSD_PeerTrafficRspAction(
	IN	PRTMP_ADAPTER				pAd,
    IN	MLME_QUEUE_ELEM				*pElem)
{
	UCHAR Token;
	UCHAR PeerAddr[6];
	UCHAR PeerAddr1[6];
	RT_802_11_TDLS *pTDLS;
	INT32 LinkId = 0xff;
	BOOLEAN TimerCancelled;
	PFRAME_802_11 pFrame = (PFRAME_802_11)pElem->Msg;

	DBGPRINT(RT_DEBUG_TRACE, ("tdls uapsd> ====> %s\n", __FUNCTION__));

	/* Not TDLS Capable, ignore it */
	if (!IS_TDLS_SUPPORT(pAd))
		return;
	
	/* Not BSS mode, ignore it */
	if (!INFRA_ON(pAd))
		return;

	hex_dump("TDLS UAPSD Peer Traffic Response receive pack", pElem->Msg, pElem->MsgLen);

	COPY_MAC_ADDR(PeerAddr, &pFrame->Hdr.Addr2);
	// Drop not within my TDLS Table that created before !
	LinkId = TDLS_SearchLinkId(pAd, PeerAddr);
	if (TDLS_UAPSD_IS_LINK_INVALID(LinkId))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: can not find the LinkId!\n", __FUNCTION__));
		return;
	}

	// Point to the current Link ID
	pTDLS = (PRT_802_11_TDLS)&pAd->StaCfg.TdlsInfo.TDLSEntry[LinkId];

	/* sanity check */
	PeerTdlsBasicSanity(pAd,
						pElem->Msg,
						pElem->MsgLen,
						pTDLS->bInitiator,
						&Token,
						PeerAddr1);

/*	hex_dump("PeerAddr=", PeerAddr, 6); */

	/* search TDLS entry */
	LinkId = TDLS_SearchLinkId(pAd, PeerAddr);
	if (TDLS_UAPSD_IS_LINK_INVALID(LinkId))
	{
		DBGPRINT(RT_DEBUG_TRACE,
				("%s: can not find the LinkId!\n", __FUNCTION__));
		TDLS_UAPSD_REBUILD_LINK(pAd, PeerAddr);
		return;
	}

	DBGPRINT(RT_DEBUG_TRACE, ("tdls uapsd> LinkId = %d\n", LinkId));

	/* cancel waiting flag to avoid tear down the link */
	pTDLS = TDLS_UAPSD_ENTRY_GET(pAd, LinkId);
	pTDLS->FlgIsWaitingUapsdTraRsp = FALSE;
	RTMPCancelTimer(&pTDLS->Timer, &TimerCancelled);

	/* check if we can sleep if we are sleep mode */
	RtmpAsicSleepHandle(pAd);
}


/*
========================================================================
Routine Description:
	Get argument number value.

Arguments:
	*pArgv			- input parameters

Return Value:
	decimal number

Note:
========================================================================
*/
static UINT32 TDLS_UAPSD_CmdUtilNumGet(
	IN	CHAR						**ppArgv)
{
	CHAR Buf[20], *pNum;
	UINT32 ID;


	pNum = (*ppArgv);

	for(ID=0; ID<sizeof(Buf)-1; ID++)
	{
		if ((*pNum == '_') || (*pNum == 0x00))
			break;
		/* End of if */

		pNum ++;
	} /* End of for */

	if (ID == sizeof(Buf)-1)
		return 0; /* argument length is too large */
	/* End of if */

	memcpy(Buf, (*ppArgv), ID);
	Buf[ID] = 0x00;

	*ppArgv += ID+1; /* skip _ */

	return simple_strtol((PSTRING)Buf, 0, 10);
} /* End of TDLS_UAPSD_CmdUtilNumGet */


/*
========================================================================
Routine Description:
	Get argument MAC value.

Arguments:
	**ppArgv			- input parameters
	*pDevMac			- MAC address

Return Value:
	None

Note:
========================================================================
*/
static VOID TDLS_UAPSD_CmdUtilMacGet(
	IN	CHAR						**ppArgv,
	IN	UCHAR						*pDevMac)
{
	CHAR Buf[3];
	CHAR *pMAC = (CHAR *)(*ppArgv);
	UINT32 ID;


	if ((pMAC[0] == '0') && (pMAC[1] == '_'))
	{
		*ppArgv = (&pMAC[2]);
		return;
	} /* End of if */

	NdisZeroMemory(pDevMac, 6);

	/* must exist 18 octets */
	for(ID=0; ID<18; ID+=2)
	{
		if ((pMAC[ID] == '_') || (pMAC[ID] == 0x00))
		{
			*ppArgv = (&pMAC[ID]+1);
			return;
		} /* End of if */
	} /* End of for */

	/* get mac */
	for(ID=0; ID<18; ID+=3)
	{
		Buf[0] = pMAC[0];
		Buf[1] = pMAC[1];
		Buf[2] = 0x00;

		AtoH(Buf, pDevMac, 1);
		pMAC += 3;
		pDevMac ++;
	} /* End of for */

	*ppArgv += 17+1; /* skip _ */
} /* End of TDLS_UAPSD_CmdUtilMacGet */


/*
========================================================================
Routine Description:
	Simulate to send a TDLS Setup request to a peer.

Arguments:
	pAd				- WLAN control block pointer
	Argc			- the number of input parameters
	*pArgv			- input parameters

Return Value:
	None

Note:
	1. Command Format:
		iwpriv ra0 set tdls=51_[PEER MAC]

	11.21.4 TDLS direct-link establishment
	TDLS Setup Request frames, TDLS Setup Response frames, and TDLS Setup
	Confirm frames shall be transmitted through the AP and shall not be
	transmitted to a group address.
========================================================================
*/
static VOID TDLS_UAPSD_CmdSimSetupReqSend(
	IN	PRTMP_ADAPTER				pAd,
	IN	INT32						Argc,
	IN	CHAR						*pArgv)
{
	MLME_QUEUE_ELEM *pElem;
	RT_802_11_TDLS TDLS, *pTDLS = &TDLS;
	UCHAR	TDLS_ETHERTYPE[] = {0x89, 0x0d};
	UCHAR	Header802_3[14];
	PUCHAR	pOutBuffer = NULL;
	ULONG	FrameLen = 0;
	ULONG	TempLen;
	UCHAR	RemoteFrameType = PROTO_NAME_TDLS;
	NDIS_STATUS	NStatus = NDIS_STATUS_SUCCESS;
	UCHAR PeerMac[6];
	UINT32 IdTdls;


	/* get MAC address */
	TDLS_UAPSD_CmdUtilMacGet(&pArgv, PeerMac);

	/* allocate buffer for transmitting message */
	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
	if (NStatus	!= NDIS_STATUS_SUCCESS)	
		return;

	os_alloc_mem(NULL, (UCHAR **)&pElem, sizeof(MLME_QUEUE_ELEM));
	if (pElem == NULL)
	{
		MlmeFreeMemory(pAd, pOutBuffer);
		return;
	}

	/* init link entry */
	NdisZeroMemory(pTDLS, sizeof(RT_802_11_TDLS));
	pTDLS->TimeOut = 0;
	COPY_MAC_ADDR(pTDLS->MacAddr, PeerMac);
	pTDLS->Valid = 1;

	/* search a empty entry */
	for(IdTdls=0; IdTdls<MAX_NUM_OF_TDLS_ENTRY; IdTdls++)
	{
		if (!pAd->StaCfg.TdlsInfo.TDLSEntry[IdTdls].Valid)
		{
			NdisMoveMemory(&pAd->StaCfg.TdlsInfo.TDLSEntry[IdTdls],
							pTDLS, sizeof(RT_802_11_TDLS_UI));
			break;
		}
	}
	if (IdTdls == MAX_NUM_OF_TDLS_ENTRY)
	{
		MlmeFreeMemory(pAd, pOutBuffer);
		os_free_mem(NULL, pElem);
		return;
	}

	/* init request frame */
	MAKE_802_3_HEADER(Header802_3, pTDLS->MacAddr,
						pAd->CurrentAddress, TDLS_ETHERTYPE);

	MakeOutgoingFrame(pOutBuffer,		&TempLen,
						1,				&RemoteFrameType,
						END_OF_ARGS);

	FrameLen = FrameLen + TempLen;
	TDLS_BuildSetupRequest(pAd, pOutBuffer, &FrameLen,
							TDLS_UAPSD_ENTRY_GET(pAd, IdTdls));
	hex_dump("Request=", pOutBuffer, FrameLen);

	TDLS_UAPSD_PKT_SEND_THROUGH_AP(pAd, Header802_3, pOutBuffer, FrameLen);

	/* init response frame */
	FrameLen += LENGTH_802_11 + LENGTH_802_1_H;
	pElem->MsgLen = LENGTH_802_11 + LENGTH_802_1_H + FrameLen;

	/* copy payload type, category, action (3B) */
	memcpy(pElem->Msg + LENGTH_802_11 + LENGTH_802_1_H,
			pOutBuffer, 3);
	/* status code = 0x00 00 (2B) */
	*(USHORT *)(pElem->Msg + LENGTH_802_11 + LENGTH_802_1_H + 3) = 0x00;

	/* copy others */
	memcpy(pElem->Msg + LENGTH_802_11 + LENGTH_802_1_H + 3 + 2,
			pOutBuffer + 3, FrameLen - 3);

	/* handle response frame */
	TDLS_PeerSetupRspAction(pAd, pElem);

	/* free memory */
	MlmeFreeMemory(pAd, pOutBuffer);
	os_free_mem(NULL, pElem);
} /* End of TDLS_UAPSD_CmdSimSetupReqSend */


/*
========================================================================
Routine Description:
	Simulate to receive a TDLS Traffic response from a peer.

Arguments:
	pAd				- WLAN control block pointer
	Argc			- the number of input parameters
	*pArgv			- input parameters

Return Value:
	None

Note:
	1. Command Format:
		iwpriv ra0 set tdls=52_[PEER MAC]

	2. In the actual case, the traffic response frame will be handled in
		STAHandleRxDataFrame() because the traffic response frame is a
		DATA frame, not management action frame.
========================================================================
*/
static VOID TDLS_UAPSD_CmdSimTrafficRspRcv(
	IN	PRTMP_ADAPTER				pAd,
	IN	INT32						Argc,
	IN	CHAR						*pArgv)
{
	MAC_TABLE_ENTRY	*pMacEntry;
	UCHAR PeerMac[6];
	RT_802_11_TDLS *pTDLS = NULL;
	INT32 LinkId;


	/* get MAC address */
	TDLS_UAPSD_CmdUtilMacGet(&pArgv, PeerMac);

	/* get pEntry */
	pMacEntry = MacTableLookup(pAd, PeerMac);

	if (pMacEntry == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("tdls_cmd> ERROR! No such peer!\n"));
		return;
	}

	/* search TDLS entry */
	LinkId = TDLS_SearchLinkId(pAd, PeerMac);
	if (TDLS_UAPSD_IS_LINK_INVALID(LinkId))
	{
		DBGPRINT(RT_DEBUG_ERROR,
				("%s: can not find the LinkId!\n", __FUNCTION__));
		TDLS_UAPSD_REBUILD_LINK(pAd, PeerMac);
		return;
	}

	DBGPRINT(RT_DEBUG_ERROR, ("tdls uapsd> LinkId = %d\n", LinkId));

	/* cancel waiting flag to avoid tear down the link */
	pTDLS = TDLS_UAPSD_ENTRY_GET(pAd, LinkId);
	pTDLS->FlgIsWaitingUapsdTraRsp = FALSE;

	/* handle UAPSD SP */
	/*
		TDLS uses Ethertype 89-0d frames, as defined in Annex U.
		The TDLS payload contains a TDLS Action frame body as is specified
		in 7.4.11. The UP shall be AC_VI, unless otherwise specified.

		So these TDLS action frames are DATA frame, not management frame.
	*/
	UAPSD_TriggerFrameHandle(pAd, pMacEntry, 5);
}


/*
========================================================================
Routine Description:
	Simulate to receive a TDLS Traffic indication from a peer.

Arguments:
	pAd				- WLAN control block pointer
	Argc			- the number of input parameters
	*pArgv			- input parameters

Return Value:
	None

Note:
	1. Command Format:
		iwpriv ra0 set tdls=53_[PEER MAC]
========================================================================
*/
static VOID TDLS_UAPSD_CmdSimTrafficIndRcv(
	IN	PRTMP_ADAPTER				pAd,
	IN	INT32						Argc,
	IN	CHAR						*pArgv)
{
	UCHAR PeerMac[6];
	MLME_QUEUE_ELEM *pElem = NULL;
	PUCHAR pOutBuffer = NULL;
	ULONG FrameLen = 0;
	INT32 LinkId;
	RT_802_11_TDLS *pTDLS = NULL;
	NDIS_STATUS	NStatus = NDIS_STATUS_SUCCESS;
	UCHAR RemoteFrameType = PROTO_NAME_TDLS;
	ULONG TempLen;
	UCHAR Token;			// Dialog token


	/* get MAC address */
	TDLS_UAPSD_CmdUtilMacGet(&pArgv, PeerMac);

	/* allocate resources */
	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
	if (NStatus	!= NDIS_STATUS_SUCCESS)
		goto LabelExit;

	/* make up a virtual traffic indication frame */
	/* search TDLS entry */
	LinkId = TDLS_SearchLinkId(pAd, PeerMac);
	if (TDLS_UAPSD_IS_LINK_INVALID(LinkId))
	{
		DBGPRINT(RT_DEBUG_ERROR,
				("%s: can not find the LinkId!\n", __FUNCTION__));
		TDLS_UAPSD_REBUILD_LINK(pAd, PeerMac);
		goto LabelExit;
	}
	pTDLS = TDLS_UAPSD_ENTRY_GET(pAd, LinkId);

	/* build the frame */
	/* fill remote frame type */
	MakeOutgoingFrame(pOutBuffer,		&TempLen,
						1,				&RemoteFrameType,
						END_OF_ARGS);
	FrameLen = TempLen;

	/* fill action code */
	TDLS_InsertActField(pAd, (pOutBuffer + FrameLen), &FrameLen,
						CATEGORY_TDLS, TDLS_ACTION_CODE_SETUP_REQUEST);

	/* fill Dialog Token */
	TDLS_UAPSD_DIALOG_GET(pAd, Token);
	TDLS_InsertDialogToken(pAd, (pOutBuffer + FrameLen), &FrameLen,
							Token);

	/* fill link identifier */
	TDLS_InsertLinkIdentifierIE(pAd, (pOutBuffer + FrameLen), &FrameLen,
								pTDLS->MacAddr, pAd->CurrentAddress);

	/* fill PU buffer status */
	TDLS_InsertPuBufferStatus(pAd, (pOutBuffer + FrameLen), &FrameLen,
							pTDLS->MacAddr);

	if (FrameLen <= 0)
		goto LabelExit;

/*	hex_dump("TDLS traffic indication send pack", pOutBuffer, FrameLen); */

	/* allocate resources */
	os_alloc_mem(NULL, (UCHAR **)&pElem, sizeof(MLME_QUEUE_ELEM));
	if (pElem == NULL)
		goto LabelExit;

	/* copy the indication frame */
	FrameLen += LENGTH_802_11 + LENGTH_802_1_H;
	pElem->MsgLen = LENGTH_802_11 + LENGTH_802_1_H + FrameLen;

	/* copy payload */
	memcpy(pElem->Msg + LENGTH_802_11 + LENGTH_802_1_H,
			pOutBuffer, FrameLen);

	/* handle it */
	TDLS_UAPSD_PeerTrafficIndAction(pAd, pElem);

	/* free resources */
LabelExit:
	if (pElem != NULL)
		os_free_mem(NULL, pElem);
	if (pOutBuffer != NULL)
		MlmeFreeMemory(pAd, pOutBuffer);
}


/*
========================================================================
Routine Description:
	Simulate to change the power save of a peer.

Arguments:
	pAd				- WLAN control block pointer
	Argc			- the number of input parameters
	*pArgv			- input parameters

Return Value:
	None

Note:
	1. Command Format:
		iwpriv ra0 set tdls=54_[PEER MAC]_[0/1]
========================================================================
*/
static VOID TDLS_UAPSD_CmdSimPeerPowerSaveChg(
	IN	PRTMP_ADAPTER				pAd,
	IN	INT32						Argc,
	IN	CHAR						*pArgv)
{
	MAC_TABLE_ENTRY	*pMacEntry;
	UCHAR PeerMac[6];
	UINT32 PeerPsMode;


	/* get MAC address */
	TDLS_UAPSD_CmdUtilMacGet(&pArgv, PeerMac);

	/* get pEntry */
	pMacEntry = MacTableLookup(pAd, PeerMac);

	if (pMacEntry == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("tdls_cmd> ERROR! No such peer!\n"));
		return;
	}

	/* change mode */
	PeerPsMode = TDLS_UAPSD_CmdUtilNumGet(&pArgv);

	if (PeerPsMode != 0)
	{
		pMacEntry->PsMode = PWR_ACTIVE;
		DBGPRINT(RT_DEBUG_TRACE, ("tdls_cmd> Change to ACTIVE!\n"));
	}
	else
	{
		pMacEntry->PsMode = PWR_SAVE;
		DBGPRINT(RT_DEBUG_TRACE, ("tdls_cmd> Change to POWER SAVE!\n"));
	}
}


/*
========================================================================
Routine Description:
	Simulate to change our power save.

Arguments:
	pAd				- WLAN control block pointer
	Argc			- the number of input parameters
	*pArgv			- input parameters

Return Value:
	None

Note:
	1. Command Format:
		iwpriv ra0 set tdls=55_[0/1]
========================================================================
*/
static VOID TDLS_UAPSD_CmdSimSelfPowerSaveChg(
	IN	PRTMP_ADAPTER				pAd,
	IN	INT32						Argc,
	IN	CHAR						*pArgv)
{
	UINT32 PeerPsMode;


	PeerPsMode = TDLS_UAPSD_CmdUtilNumGet(&pArgv);

	if (PeerPsMode != 0)
	{
		pAd->StaCfg.Psm = PWR_ACTIVE;
		DBGPRINT(RT_DEBUG_TRACE, ("tdls_cmd> Change to ACTIVE!\n"));
	}
	else
	{
		pAd->StaCfg.Psm = PWR_SAVE;
		DBGPRINT(RT_DEBUG_TRACE, ("tdls_cmd> Change to POWER SAVE!\n"));
	}
}


/*
========================================================================
Routine Description:
	Simulate to send a TDLS Traffic response to a peer.

Arguments:
	pAd				- WLAN control block pointer
	Argc			- the number of input parameters
	*pArgv			- input parameters

Return Value:
	None

Note:
	1. Command Format:
		iwpriv ra0 set tdls=56_[PEER MAC]
========================================================================
*/
static VOID TDLS_UAPSD_CmdSimTrafficRspSnd(
	IN	PRTMP_ADAPTER				pAd,
	IN	INT32						Argc,
	IN	CHAR						*pArgv)
{
	MAC_TABLE_ENTRY	*pMacEntry;
	UCHAR PeerMac[6];
	UCHAR Token = 0;


	/* get MAC address */
	TDLS_UAPSD_CmdUtilMacGet(&pArgv, PeerMac);

	/* get pEntry */
	pMacEntry = MacTableLookup(pAd, PeerMac);

	if (pMacEntry == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("tdls_cmd> ERROR! No such peer!\n"));
		return;
	}

	/* send reponse */
	TDLS_UAPSD_TrafficRspSend(pAd, PeerMac, Token);
}

#endif /* UAPSD_SUPPORT */

/* End of tdls_uapsd.c */
