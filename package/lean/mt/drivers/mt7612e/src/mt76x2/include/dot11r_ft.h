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
    dot11r_ft.h
 
    Abstract:
	Defined status code, IE and frame structures that FT (802.11rD9.0) needed.
 
    Revision History:
    Who        When          What
    ---------  ----------    ----------------------------------------------
    Fonchi Wu  12-02-2008    created for 11r soft-AP
 */


#ifndef __DOT11R_FT_H
#define __DOT11R_FT_H

#include "rtmp_type.h"


#ifdef DOT11R_FT_SUPPORT
/*
	Fast BSS transition auth algorithm in 802.11rD9.0 specification.
	All other algorithms are defined in  rtmp_def.h
*/
#define FT_MDID_LEN					2
#define FT_ROKH_ID_LEN				48
#define FT_R1KH_ID_LEN				6
#endif /* DOT11R_FT_SUPPORT */

#if defined(DOT11R_FT_SUPPORT) || defined(DOT11Z_TDLS_SUPPORT)
#define FT_MIC_LEN					16
#define FT_NONCE_LEN				32			
#endif

#ifdef DOT11R_FT_SUPPORT
#define FT_CATEGORY_BSS_TRANSITION	6
#define FT_ACTION_BT_REQ			1
#define FT_ACTION_BT_RSP			2
#define FT_ACTION_BT_CONFIRM		3
#define FT_ACTION_BT_ACK			4

/* Status codes defined in 802.11rD9.0 specification. */
#define FT_STATUS_CODE_RESERVED				27
#define FT_STATUS_CODE_R0KH_UNREACHABLE		28
#define FT_STATUS_CODE_INVALID_FME_COUNT	52
#define FT_STATUS_CODE_INVALID_PMKID		53
#define FT_STATUS_CODE_INVALID_MDIE			54
#define FT_STATUS_CODE_INVALID_FTIE			55
#endif /* DOT11R_FT_SUPPORT */

#if defined(DOT11R_FT_SUPPORT) || defined(DOT11Z_TDLS_SUPPORT)
/* Information element ID defined in 802.11rD9.0 specification. */
#define IE_FT_MDIE				54
#define IE_FT_FTIE				55
#define IE_FT_TIMEOUT_INTERVAL	56
#define IE_FT_RIC_DATA			57
#define IE_FT_RIC_DESCRIPTOR	75


/* RIC Type */
#define FT_RIC_TYPE_BA			1

/* AKM SUITE */
#define FT_AKM_SUITE_1X		3
#define FT_AKM_SUITE_PSK	4
#endif

#ifdef DOT11R_FT_SUPPORT
/*
** MDIE: Mobile Domain IE.
*/
typedef union GNU_PACKED _FT_CAP_AND_POLICY
{
	struct GNU_PACKED
	{
#ifdef RT_BIG_ENDIAN
	UINT8 :6;
	UINT8 RsrReqCap:1;
	UINT8 FtOverDs:1;
#else
	UINT8 FtOverDs:1;
	UINT8 RsrReqCap:1;
	UINT8 :6;
#endif
	} field;
	UINT8 word;
} FT_CAP_AND_POLICY, *PFT_CAP_AND_POLICY;

typedef struct GNU_PACKED _FT_MDIE
{
	UINT8 MdId[FT_MDID_LEN];
	FT_CAP_AND_POLICY FtCapPlc;
} FT_MDIE, *PFT_MDIE;
#endif /* DOT11R_FT_SUPPORT */

#if defined(DOT11R_FT_SUPPORT) || defined(DOT11Z_TDLS_SUPPORT)
typedef union GNU_PACKED _FT_MIC_CTR_FIELD
{
	/*
		IECnt: contains the number of IEs
		that are included int eht MIC calculation.
	*/
	struct GNU_PACKED
	{
#ifdef RT_BIG_ENDIAN
	UINT16 IECnt:8;
	UINT16 :8;
#else
	UINT16 :8;
	UINT16 IECnt:8;
#endif
	} field;
	UINT16 word;
} FT_MIC_CTR_FIELD, *PFT_MIC_CTR_FIELD;

/*
** FTIE: Fast Transition IE.
*/
typedef struct GNU_PACKED _FT_FTIE
{
	FT_MIC_CTR_FIELD MICCtr;		/* 2 Octects. */
	UINT8 MIC[FT_MIC_LEN];			/* 16 Octects. */
	UINT8 ANonce[FT_NONCE_LEN];		/* 32 Octects. */
	UINT8 SNonce[FT_NONCE_LEN];		/* 32 Octects. */
	UINT8 Option[0];				/* 1:R1KHID, 2:GTK, 3:ROKHId, else:Res */ 
} FT_FTIE, *PFT_FTIE;
#endif

#ifdef DOT11R_FT_SUPPORT
/* Sub-element IDs */
typedef enum _FT_SUB_ELEMENT_ID
{
	FT_R1KH_ID = 1,
	FT_GTK,
	FT_R0KH_ID,
#ifdef DOT11W_PMF_SUPPORT
	FT_IGTK_ID,	
#endif /* DOT11W_PMF_SUPPORT */
	FT_RESERVED_ID
} FT_SUB_ELEMENT_ID, *PFT_SUB_ELEMENT_ID;

typedef struct GNU_PACKED _FT_OPTION_FIELD
{
	UINT8 SubElementId;
	UINT8 Len;
	UINT8 Oct[0];
} FT_OPTION_FIELD, *PFT_OPTION_FIELD;

/* Sub element frame of FTIE. */
typedef union GNU_PACKED _FT_GTK_KEY_INFO
{
	struct GNU_PACKED{
#ifdef RT_BIG_ENDIAN
	UINT16 :14;
	UINT16 KeyId:2;
#else
	UINT16 KeyId:2;
	UINT16 :14;
#endif
	} field;
	UINT16 word;
} FT_GTK_KEY_INFO, *PFT_GTK_KEY_INFO;

typedef struct GNU_PACKED _FT_GTK_SUB_ELEMENT
{
	FT_GTK_KEY_INFO KeyInfo;	/* 2 octects. */
	UINT8 KeyLen;
	UINT8 RSC[8];				/* Receive seq counter. */
	UINT8 Key[0];				/* 5 to 32 octects. */
} FT_GTK_SUB_ELEMENT, *PFT_GTK_SUB_ELEMENT;
#endif /* DOT11R_FT_SUPPORT */

#if defined(DOT11R_FT_SUPPORT) || defined(DOT11Z_TDLS_SUPPORT)
/*
** Timeout Interval IE.
*/
typedef enum _FT_TIMEOUT_INTERVAL_TYPE
{
	REASSOC_DEADLINE_INTERVAL = 1,	/* TUs */
	KEY_LIFETIME_INTERVAL,				/* seconds. */
#ifdef DOT11W_PMF_SUPPORT
	ASSOC_COMEBACK_TIME,				/* TUs */	
#endif /* DOT11W_PMF_SUPPORT */
	RESERVED_INTERVAL
} FT_TIMEOUT_INTERVAL_TYPE, *PFT_TIMEOUT_INTERVAL_TYPE;

typedef struct GNU_PACKED _FT_TIMEOUT_INTERVAL_IE
{
	UINT8 TimeoutIntervalType;
	UINT32 TimeoutIntervalValue;
} FT_TIMEOUT_INTERVAL_IE, *PFT_TIMEOUT_INTERVAL_IE;
#endif

#ifdef DOT11R_FT_SUPPORT
/* ----- Fast BSS Transition Action ----- */
#define FT_OVER_DS_CATEGORY				0x06

typedef struct _FT_OVER_DS_ACTION_REQ_CONFIRM {

	/* must be 6 */
	UCHAR	Category;

#define FT_OVER_DS_ACTION_REQ			0x01
#define FT_OVER_DS_ACTION_CON			0x03
	/* must be 1 or 3 */
	UCHAR	Action;

	/* the STA¡¦s MAC address */
	UCHAR	STA_Addr[ETH_ALEN];

	/* the BSSID value of the target AP */
	UCHAR	TargetAP_Addr[ETH_ALEN];

	/* REQ: RSN, Mobility Domain, or Fast BSS Transition element */
	/* CONFIRM: RSN, Mobility Domain, Fast BSS Transition, or RIC element */
	UCHAR	InfoElm[0];/*[1024 - 14]; */
} GNU_PACKED FT_OVER_DS_ACTION_REQ_FONFIRM;

typedef struct _FT_OVER_DS_ACTION_RSP_ACK {

	/* must be 6 */
	UCHAR	Category;

#define FT_OVER_DS_ACTION_RSP			0x02
#define FT_OVER_DS_ACTION_ACK			0x04
	/* must be 2 or 4 */
	UCHAR	Action;

	/* the STA¡¦s MAC address */
	UCHAR	STA_Addr[ETH_ALEN];

	/* the BSSID value of the target AP */
	UCHAR	TargetAP_Addr[ETH_ALEN];

	/* FT_STATUS_CODE_R0KH_UNREACHABLE ~ FT_STATUS_CODE_INVALID_FTIE */
	UINT16	StatusCode;

	/* RSP: RSN, Mobility Domain, or Fast BSS Transition element */
	/* ACK: RSN, Mobility Domain, Fast BSS Transition, Timeout, or RIC element */
	UCHAR	InfoElm[0];/*[1024 - 16]; */
} GNU_PACKED FT_OVER_DS_ACTION_RSP_ACK;


/* ----- RIC(Resource Information Container) ----- */

/* element ID and element length */
#define FT_ELM_HDR_LEN					2

/* Resource Information Container Data information element */
typedef struct _FT_ELM_RIC_DATA_INFO {

#define FT_ELM_ID_RIC_DATA_INFO			57
	UCHAR	ElmID;

#define FT_ELM_LEN_RIC_DATA_INFO		4
	UCHAR	ElmLen;

	/*
		RDIE: RIC Data information Element
		An arbitrary 8-bit value, chosen by the resource requestor to
		uniquely identify the RDIE within the RIC.
	*/
	UCHAR	RDIE_Identifier;

	/*
		Resource Descriptor Count
		Number of alternative Resource Descriptors that follow this RDIE.
	*/
	UCHAR	RD_Count;

	/* the result of the request */
	/* FT_STATUS_CODE_R0KH_UNREACHABLE ~ FT_STATUS_CODE_INVALID_FTIE */
	UINT16	StatusCode;

} GNU_PACKED FT_ELM_RIC_DATA_INFO;

/* Resource Information Container descriptor information element */
#define FT_ELM_RIC_DESCP_INFO_LEN		(FT_ELM_HDR_LEN+7)

typedef struct GNU_PACKED _FT_RIC_DESCP_BLOCK_ACK {
	UINT16 BaParm; /* Block Ack Parameter Set */
	UINT16 TimeOutValue; /* Block Ack Timeout Value */ 
	UINT16 BaStartSeq; /* Block Ack Starting Sequence Control */
} FT_RIC_DESCP_BLOCK_ACK;

typedef struct GNU_PACKED _FT_ELM_RIC_DESCP_INFO {
#define FT_ELM_ID_RIC_DESCP_INFO						75
	UCHAR	ElmID;

#define FT_ELM_LEN_RIC_DESCP_INFO_RSC_BLOCK_ACK			7
	UCHAR	ElmLen;

#define FT_ELM_RIC_DESCP_INFO_RSC_TYPE_BLOCK_ACK		1
	UCHAR	ResourceType;

	/* negotiated resources */
	/*
		For FT_ELM_RIC_DESCP_INFO_RSC_TYPE_BLOCK_ACK:
			Block Ack Parameter Set (2B),
			Block Ack Timeout Value (2B), and
			Block Ack Starting Sequence Control (2B)
	*/
	UCHAR	Container[6];

} FT_ELM_RIC_DESCP_INFO;

typedef struct GNU_PACKED __FT_ACTION
{
	UINT8 Category;
	UINT8 Action;
	UINT8 StaMac[MAC_ADDR_LEN];
	UINT8 TargetApAddr[MAC_ADDR_LEN];
	UINT8 Oct[0];
} FT_ACTION, *PFT_ACTION;

typedef struct GNU_PACKED __FT_RRB
{
	UINT8 RemoteFrTyp;
	UINT8 FTPktType;
	UINT16 FTActLen;
	UINT8 APAdr[MAC_ADDR_LEN];
	UINT8 Oct[0];
} FT_RRB, *PFT_RRB;
#endif /* DOT11R_FT_SUPPORT */

#endif /* __DOT11R_FT_H */

