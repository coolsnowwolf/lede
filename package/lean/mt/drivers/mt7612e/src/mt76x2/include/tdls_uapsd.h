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

	All related TDLS UAPSD definitions & function prototype.

***************************************************************************/


/* sub function enable/disable */
#define TDLS_UAPSD_DEBUG				/* debug use */
#define TDLS_UAPSD_SLEEP_MODE_CHECK		/* check peer ps mode after link built */


/* extern MACRO & function */
#ifndef MODULE_TDLS_UAPSD

#define TDLS_UAPSD_EXTERN									extern


#else

#define TDLS_UAPSD_EXTERN

/* check if TDLS UAPSD function is disabled */
#define TDLS_UAPSD_IS_DISABLED(__pAd)										\
	(__pAd->CommonCfg.ExtCapIE.TdlsUAPSD == 0)

/* assign a new dialog token */
#define TDLS_UAPSD_DIALOG_GET(__pAd, __Token)								\
	__pAd->StaCfg.TdlsInfo.TdlsDialogToken ++;										\
	if (__pAd->StaCfg.TdlsInfo.TdlsDialogToken == 0)									\
		__pAd->StaCfg.TdlsInfo.TdlsDialogToken ++;									\
	__Token = __pAd->StaCfg.TdlsInfo.TdlsDialogToken;

/* get self MAC address */
#define TDLS_UAPSD_MY_MAC_GET(__pAd)										\
	__pAd->CurrentAddress

/* get self BSSID */
#define TDLS_UAPSD_MY_BSSID_GET(__pAd)										\
	__pAd->CommonCfg.Bssid

/* get self WMM capability information */
#define TDLS_UAPSD_CAP_INFO_GET(__pAd)										\
	__pAd->StaActive.CapabilityInfo

/* get tx rate for QoS null frame */
#define TDLS_UAPSD_QOS_NULL_RATE_GET(__pAd)									\
	__pAd->CommonCfg.TxRate

/* check if we are in sleep mode */
#define TDLS_UAPSD_ARE_WE_IN_PS(__pAd)										\
	(__pAd->StaCfg.Psm == PWR_SAVE)

/* check if we are in active mode */
#define TDLS_UAPSD_ARE_WE_IN_ACTIVE(__pAd)									\
	(__pAd->StaCfg.Psm != PWR_SAVE)

/* check if the peer is in sleep mode */
#define TDLS_UAPSD_ARE_PEER_IN_PS(__pEntry)									\
	(__pEntry->PsMode == PWR_SAVE)

/* check if the peer is in active mode */
#define TDLS_UAPSD_ARE_PEER_IN_ACTIVE(__pEntry)								\
	(__pEntry->PsMode != PWR_SAVE)

/* get support rate element information */
#define TDLS_UAPSD_SUP_RATE_GET(__pAd, __RateLen, __pRate)					\
	__RateLen = __pAd->CommonCfg.SupRateLen;								\
	NdisMoveMemory(__pRate, __pAd->CommonCfg.SupRate, __RateLen);

/* get extended rate element information */
#define TDLS_UAPSD_EXT_RATE_GET(__pAd, __RateLen, __pRate)					\
	__RateLen = __pAd->CommonCfg.ExtRateLen;								\
	NdisMoveMemory(__pRate, __pAd->CommonCfg.ExtRate, __RateLen);

/* get extended capability element information */
#define TDLS_UAPSD_EXT_CAP_IE_GET(__pAd, __pIE)								\
	NdisMoveMemory(__pIE, &__pAd->CommonCfg.ExtCapIE, sizeof(EXT_CAP_INFO_ELEMENT));

/* get TDLS entry */
#define TDLS_UAPSD_ENTRY_GET(__pAd, __LinkId)								\
	&(__pAd)->StaCfg.TdlsInfo.TDLSEntry[__LinkId]

/* send a TDLS action frame to the peer through AP */
#define TDLS_UAPSD_PKT_SEND_THROUGH_AP(__pAd, __pHeader8023, __pFme, __FmeLen) \
	RTMPToWirelessSta(__pAd, &__pAd->MacTab.Content[BSSID_WCID], 			\
					__pHeader8023, LENGTH_802_3, __pFme, (UINT)__FmeLen, FALSE);

/* send a TDLS action frame to the peer without AP's help */
#define TDLS_UAPSD_PKT_SEND_TO_PEER(__pAd, __pHeader8023, __pFme, __FmeLen, __pTDLS) \
	RTMPToWirelessSta(__pAd, &__pAd->MacTab.Content[__pTDLS->MacTabMatchWCID], \
					__pHeader8023, LENGTH_802_3, __pFme, (UINT)__FmeLen, FALSE);

/* resource protection */
#define TDLS_SEMLOCK(__pAd)
#define TDLS_SEMUNLOCK(__pAd)

/* connection sanity check */
#define TDLS_UAPSD_IS_CONN_NOT_BUILT(__pTDLS)								\
	(((__pTDLS)->Valid == 0) || ((__pTDLS)->Status < TDLS_MODE_CONNECTED))

/* link ID sanity check */
#define TDLS_UAPSD_IS_LINK_INVALID(__LinkId)								\
	(((__LinkId) == -1) || ((__LinkId) >= MAX_NUM_OF_TDLS_ENTRY))

/* rebuild a TDLS link */
#define TDLS_UAPSD_REBUILD_LINK(__pAd, __pPeerMac)

#define TDLS_UAPSD_ELM_PU_BUFFER_STATUS						106
#define TDLS_UAPSD_ELM_LEN_PU_BUFFER_STATUS					3

#ifdef RT_BIG_ENDIAN
typedef struct __TDLS_UAPSD_PU_BUFFER_STATUS_ELM {

	UINT8 AC_BK: 1; /* bit0: AC_BK traffic available field */
	UINT8 AC_BE: 1; /* bit1: AC_BE traffic available field */
	UINT8 AC_VI: 1; /* bit2: AC_VI traffic available field */
	UINT8 AC_VO: 1; /* bit3: AC_VO traffic available field */

	UINT8 Reserved: 4;

} TDLS_UAPSD_PU_BUFFER_STATUS_ELM;
#else /* RT_BIG_ENDIAN */

typedef struct __TDLS_UAPSD_PU_BUFFER_STATUS_ELM {

	UINT8 Reserved: 4;

	UINT8 AC_VO: 1; /* bit3: AC_VO traffic available field */
	UINT8 AC_VI: 1; /* bit2: AC_VI traffic available field */
	UINT8 AC_BE: 1; /* bit1: AC_BE traffic available field */
	UINT8 AC_BK: 1; /* bit0: AC_BK traffic available field */

} TDLS_UAPSD_PU_BUFFER_STATUS_ELM;
#endif /* RT_BIG_ENDIAN */

#endif /* MODULE_WMM_UAPSD */


#ifdef UAPSD_SUPPORT
#define TDLS_UAPSD_ENTRY_INIT(__pTDLS)										\
	__pTDLS->FlgIsWaitingUapsdTraRsp = FALSE;
#else /* UAPSD_SUPPORT */

#define TDLS_UAPSD_ENTRY_INIT(__pTDLS)
#endif /* UAPSD_SUPPORT */




/* Public function list */
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
TDLS_UAPSD_EXTERN BOOLEAN TDLS_UAPSDP_Init(
	IN	PRTMP_ADAPTER				pAd,
    IN	STATE_MACHINE				*pFSM);

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
TDLS_UAPSD_EXTERN BOOLEAN TDLS_UAPSDP_Release(
	IN	PRTMP_ADAPTER				pAd);

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
========================================================================
*/
TDLS_UAPSD_EXTERN NDIS_STATUS TDLS_UAPSDP_TrafficIndSend(
	IN	PRTMP_ADAPTER				pAd,
	IN	UCHAR						*pPeerMac);

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
TDLS_UAPSD_EXTERN BOOLEAN TDLS_UAPSDP_AsicCanSleep(
	IN	PRTMP_ADAPTER				pAd);

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
	IN	USHORT						PsmNew);

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
TDLS_UAPSD_EXTERN INT TDLS_Ioctl(
	IN	PRTMP_ADAPTER				pAd,
	IN	PSTRING						pArgvIn);

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
TDLS_UAPSD_EXTERN INT Set_TdlsUapsdProc(
	IN	PRTMP_ADAPTER				pAd,
	IN	PSTRING						pArgvIn);

/* End of ap_uapsd.h */
