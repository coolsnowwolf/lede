/****************************************************************************
 * Ralink Tech Inc.
 * Taiwan, R.O.C.
 *
 * (c) Copyright 2002, Ralink Technology, Inc.
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

	Define all structures, data types that rtmp.h needed in this file. Don't
	put any sturctures and functions definition which refer to RTMP_ADAPTER
	Here.


***************************************************************************/

#ifdef DOT11R_FT_SUPPORT


#ifndef __FT_CMM_H__
#define __FT_CMM_H__

#include "dot11r_ft.h"
#include "dot11i_wpa.h"


/* ========================================================================== */
/* Definition */
#define IS_FT_RSN_STA(_pEntry)		\
	((_pEntry) && IS_ENTRY_CLIENT(_pEntry) && (_pEntry->MdIeInfo.Len != 0) && (_pEntry->RSNIE_Len > 0))

#define FT_SET_MDID(__D, __S) \
		NdisMoveMemory((PUCHAR)(__D), (PUCHAR)(__S), FT_MDID_LEN)

#define FT_MDID_EQU(__D, __S) (memcmp((__D), (__S), FT_MDID_LEN) == 0)

#ifdef CONFIG_STA_SUPPORT
#define FT_OTD_RESPONSE		1
#define FT_OTA_RESPONSE		2

/* */
/* STA's FT OTD State machine: states, events, total function # */
/* */
#define FT_OTD_IDLE						0
#define FT_OTD_WAIT_SEQ2				1
#define FT_OTD_WAIT_SEQ4				2
#define FT_OTD_MAX_STATE				3

#define FT_OTD_MACHINE_BASE		        0
#define FT_OTD_MT2_MLME_REQ			    0
#define FT_OTD_MT2_PEER_EVEN			1
#define FT_OTD_MT2_TIMEOUT				2
#define FT_OTD_MAX_MSG				    3

#define	FT_OTD_FUNC_SIZE				(FT_OTD_MAX_STATE * FT_OTD_MAX_MSG)

/* */
/* STA's FT OTA AUTHENTICATION state machine: states, evvents, total function # */
/* */
#define FT_OTA_AUTH_REQ_IDLE			0
#define FT_OTA_AUTH_WAIT_RESP			1
#define FT_OTA_AUTH_WAIT_ACK			2
#define FT_OTA_MAX_AUTH_STATE			3

#define FT_OTA_AUTH_MACHINE_BASE		0
#define FT_OTA_MT2_MLME_AUTH_REQ		0
#define FT_OTA_MT2_PEER_AUTH_EVEN		1
#define FT_OTA_MT2_AUTH_TIMEOUT			2
#define FT_OTA_MAX_AUTH_MSG				3

#define FT_OTA_AUTH_FUNC_SIZE			(FT_OTA_MAX_AUTH_STATE * FT_OTA_MAX_AUTH_MSG)
#endif /* CONFIG_STA_SUPPORT */

/* ----- General ----- */
#define FT_KDP_FUNC_SOCK_COMM			/* used socket to communicate with driver */
#define FT_KDP_FUNC_R0KH_IP_RECORD		/* used to keep IP of any R0KH */
/*#define FT_KDP_FUNC_INFO_BROADCAST	broadcast my AP information periodically */
#define FT_KDP_TEST

#define FT_KDP_RALINK_PASSPHRASE		"Ralink"
#define FT_KDP_KEY_DEFAULT				"12345678"
#define FT_KDP_KEY_ENCRYPTION_EXTEND	8 /* 8B for AES encryption extend size */
#define FT_DEFAULT_MDID					"RT"

#define FT_REASSOC_DEADLINE				20

#define FT_KDP_INFO_BC_PERIOD_TIME		1000 /* 1s */

#define IAPP_ETH_PRO					0xEEEE

/* key information */
#define FT_IP_ADDRESS_SIZE				4
#define FT_NONCE_SIZE					8

#define FT_KDP_WPA_NAME_MAX_SIZE		16
#define FT_KDP_R0KHID_MAX_SIZE			48
#define FT_KDP_R1KHID_MAX_SIZE			6
#define FT_KDP_S1KHID_MAX_SIZE			6
#define FT_KDP_PMKR1_MAX_SIZE			32 /* 256-bit key */

#define FT_R1KH_ENTRY_TABLE_SIZE		64
#define FT_R1KH_ENTRY_HASH_TABLE_SIZE	FT_R1KH_ENTRY_TABLE_SIZE

/* ----- FT KDP ----- */
#define FT_KDP_EVENT_MAX				10 /* queue max 10 events */

/* ----- FT RIC ----- */
#define MAX_RICIES_LEN					255


/* ========================================================================== */
/* FT user configuration. */

/* ethernet header */
typedef struct _FT_DRV_ETH_HEADER {

	UCHAR	DA[ETH_ALEN];
	UCHAR	SA[ETH_ALEN];
	UINT16	Len;

} GNU_PACKED FT_DRV_ETH_HEADER;

/* R0KH information record */
typedef struct _FT_KDP_R0KH_INFO {

	struct _FT_KDP_R0KH_INFO *pNext;

	UCHAR	R0KHID[FT_KDP_R0KHID_MAX_SIZE];
	UCHAR	MAC[ETH_ALEN];
	UINT32	IP;

	ULONG	TimeUpdate;

} FT_KDP_R0KH_INFO;

/* FT KDP Control Block */
typedef struct _FT_KDP_CTRL_BLOCK {

	/* used to record all known R0KH information in the DS */
#ifdef FT_KDP_FUNC_R0KH_IP_RECORD
#define FT_KDP_R0KH_INFO_MAX_SIZE	256
	FT_KDP_R0KH_INFO *R0KH_InfoHead;
	FT_KDP_R0KH_INFO *R0KH_InfoTail;
	UINT32 R0KH_Size;
#endif /* FT_KDP_FUNC_R0KH_IP_RECORD */

#ifndef FT_KDP_FUNC_SOCK_COMM
	/* if the signal context is got from FT KDP daemon */
	struct _LIST_HEADR EventList;
#endif /* FT_KDP_SOCK_COMM */

#ifdef FT_KDP_FUNC_INFO_BROADCAST
	RALINK_TIMER_STRUCT TimerReport;
#endif /* FT_KDP_FUNC_INFO_BROADCAST */

	/* used to encrypt/decrypt the DS packet */
	UCHAR CryptKey[100];
} FT_KDP_CTRL_BLOCK;

/* FT RIC Control Block */
#ifdef CONFIG_STA_SUPPORT
typedef struct _FT_RIC_STATUS {

	UCHAR AP_MAC[ETH_ALEN];

	BOOLEAN FlgHasBaResource; /* TRUE: exist BA resource request */
	BOOLEAN FlgIsBaAccepted; /* TRUE: BLOCK ACK resource is accepted */

	UCHAR TspecNumberOfAccepted;
	UCHAR TspecNumberOfRequested;

	UINT32 TspecMediumTimeTotalCur; /* 32-us */
	UINT32 TspecMediumTimeTotalNew; /* 32-us */

	UCHAR UnknownRDIE; /* number of RDIE with wrong Identifier or resource */

} FT_RIC_STATUS;

typedef struct _FT_RIC_STATUS_TSPEC {

	BOOLEAN FlgIsValid; /* TRUE: valid */
	BOOLEAN FlgIsAccepted; /* TRUE: accepted */
	UCHAR RDIE_Identifier;
	UINT16 MedimumTime; /* 32-us, maybe 0 for dnlink */

} FT_RIC_STATUS_TSPEC;

typedef struct _FT_RIC_AP {

	struct _FT_RIC_AP *pNext;

	/*
		11r D9.0, 11A.11.3.2 AP procedures

		The RIC Response shall comprise one RDIE for each RDIE in the RIC
		Request. The RDIEs shall be in the same order as in the request and
		the RDIE Identifier in each RDIE shall be the value of the RDIE
		Identifier in the corresponding RDIE in the request.
	*/

	BOOLEAN FlgHasBaResource; /* TRUE: exist BA resource request */
	UCHAR RDIE_IdentifierBA;

	UCHAR AP_MAC[ETH_ALEN];

} FT_RIC_AP;

typedef struct _FT_RIC_CTRL_BLOCK {

	/* resources for different APs */
	LIST_HEADER ResourceList;

	UCHAR RDIE_Identifier;

} FT_RIC_CTRL_BLOCK;
#endif /* CONFIG_STA_SUPPORT */

typedef struct _FT_CAP_CFG
{
	UINT8 FtOverDs:1; /* 1: Enable FT over the DS. */
	UINT8 RsrReqCap:1; /* 1: Enable FT resource request. */
	UINT8 Dot11rFtEnable:1; /* 1: FT enable , 0: FT disable. */
	UINT8 :5;	/* reserve. */
} FT_CAP_CFG, *PFT_CAP_CFG;

typedef struct GNU_PACKED _FT_CFG
{
	UINT8 FtMdId[FT_MDID_LEN]; /* Mobility domain ID of Fast Bss. */
	UINT32 AssocDeadLine; /* 100ms */
	UINT8 FtR0khId[FT_ROKH_ID_LEN + 1]; /* Lenght of ROKHID can be 1 to 48 chars. */
	UINT8 FtR0khIdLen;
	FT_CAP_CFG FtCapFlag; /* FT capability Configuration. */
} FT_CFG, *PFT_CFG;

typedef struct __FT_R1KH_ENTRY
{
	struct __FT_R1KH_ENTRY *pNext;
	UINT32 KeyLifeTime;
	UINT32 RassocDeadline;
	NDIS_802_11_AUTHENTICATION_MODE AuthMode;
	UINT8 PmkR0Name[16];
	UINT8 PmkR1Name[16];
	UINT8 PmkR1Key[32];
	UINT8 PairwisChipher[4];
	UINT8 AkmSuite[4];
	UINT8 R0khId[FT_ROKH_ID_LEN + 1];
	UINT8 R0khIdLen;
	UINT8 StaMac[6];
} FT_R1HK_ENTRY, *PFT_R1HK_ENTRY;

typedef struct __FT_TAB
{
	/* KDP. */
	PVOID pFT_KDP_Ctrl_BK;
	NDIS_SPIN_LOCK FT_KdpLock;
	BOOLEAN FlgIsFtKdpInit;

	/* R1KH. */
	LIST_HEADER FT_R1khEntryTab[FT_R1KH_ENTRY_HASH_TABLE_SIZE];
	NDIS_SPIN_LOCK FT_R1khEntryTabLock;
	ULONG FT_R1khEntryTabSize;

	BOOLEAN FT_R1khEntryTabReady;
} FT_TAB, *PFT_TAB; 

typedef struct __FT_MDIE_INFO
{
	INT Len;
	UINT8 MdId[FT_MDID_LEN];
	FT_CAP_AND_POLICY FtCapPlc;

	PVOID pMdIe;
} FT_MDIE_INFO, *PFT_MDIE_INFO;

typedef struct __FT_FTIE_INFO
{
	INT Len;
	FT_MIC_CTR_FIELD MICCtr;
	UINT8 MIC[16];
	UINT8 ANonce[32];
	UINT8 SNonce[32];
	UINT8 R0khIdLen;
	UINT8 R0khId[FT_ROKH_ID_LEN + 1];
	UINT8 R1khIdLen;
	UINT8 R1khId[FT_R1KH_ID_LEN + 1];
	UINT8 GtkLen;
	UINT8 GtkSubIE[64];

	PVOID pFtIe;
} FT_FTIE_INFO, *PFT_FTIE_INFO;

typedef struct __FT_RIC_INFO
{
	INT Len;
	VOID *pRicInfo;
	UINT8 RicIEs[MAX_RICIES_LEN];
	UINT8 RicIEsLen;
} FT_RIC_INFO, *PFT_RIC_INFO;

typedef struct __FT_INFO
{
	FT_MDIE_INFO MdIeInfo;
	FT_FTIE_INFO FtIeInfo;
	FT_RIC_INFO RicInfo;
    UCHAR RSNIE_Len;
    UCHAR RSN_IE[MAX_LEN_OF_RSNIE];
} FT_INFO, *PFT_INFO;

typedef struct __FT_MIC_CONTENT
{
	PUINT8		rsnie_ptr;
	UINT		rsnie_len;
	PUINT8		mdie_ptr;
	UINT		mdie_len;
	PUINT8		ftie_ptr;
	UINT		ftie_len;
	PUINT8		ric_ptr;
	UINT		ric_len;
} FT_MIC_CONTENT, *PFT_MIC_CONTENT;


#ifdef CONFIG_STA_SUPPORT
#define FT_ACT_TIMEOUT                2000       /* unit: msec */

typedef struct _MLME_FT_OTA_AUTH_REQ_STRUCT {
    UCHAR		Addr[MAC_ADDR_LEN];
    USHORT		Alg;
    ULONG		Timeout;
	FT_MDIE		MdIe;
} MLME_FT_OTA_AUTH_REQ_STRUCT, *PMLME_FT_OTA_AUTH_REQ_STRUCT;

typedef struct _DOT11R_CMN_STRUC 
{	
	BOOLEAN			bFtSupport;			/* enable or disable Fast BSS Transition support */
	BOOLEAN     	bInMobilityDomain;	/* Set to TRUE if is in a Mobility Domain. Decide to do Fast BSS Transition or not. */
	BOOLEAN			bSupportResource;	/* Set to TRUE if user wants to do resource request. */
	UINT8     		FtRspSuccess;		/* 0: not ft, 1: ft_action response from OTD, 2: ft_auth response from OTA */
	UINT8			PMKR0Name[LEN_PMK_NAME];
	UINT8			PMKR0[LEN_PMK];
	UINT8 			R0khIdLen;
	UINT8 			R0khId[FT_ROKH_ID_LEN + 1];
	FT_MDIE_INFO	MdIeInfo;
	FT_FTIE_INFO	FtIeInfo;
} DOT11R_CMN_STRUC, *PDOT11R_CMN_STRUC;

typedef struct _MLME_FT_REQ_STRUCT {
	UCHAR		TargetAddr[MAC_ADDR_LEN];
	UCHAR		HaveRSN;
	UCHAR		SNonce[32];	
	UCHAR		R0khIdLen;
	UCHAR		R0khId[FT_ROKH_ID_LEN + 1];
	FT_MDIE		MdIe;
	ULONG		Timeout;
	ULONG		VIeLen;
	UCHAR		VIe[256];		/* Carry RSNIE */
} MLME_FT_REQ_STRUCT, *PMLME_FT_REQ_STRUCT;
#endif /* CONFIG_STA_SUPPORT */

#endif /* __FT_CMM_H__ */
#endif /* DOT11R_FT_SUPPORT */

/* End of ft_cmm.h */
