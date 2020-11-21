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
    dot11w_pmf.h
 
    Abstract:
	Defined status code, IE and frame structures that PMF (802.11w) needed.
 
    Revision History:
    Who        	 When          What
    ---------  	 ----------    ----------------------------------------------
    Albert Yang  4-02-2009     created for IEEE802.11W PMF
 */

#ifndef __DOT11W_PMF_H
#define __DOT11W_PMF_H

#ifdef DOT11W_PMF_SUPPORT

#include "rtmp_type.h"


#define LEN_PMF_MMIE			16
#define PMF_CIPHER_SUITE_LEN	4
#define LEN_PMF_BIP_AAD_HDR		20
#define LEN_PMF_BIP_MIC			8
#define LEN_PMF_IGTK_KDE		24

/* PMF Action filed value */
#define PMF_ACTION_QA_QUERY_REQ		0
#define PMF_ACTION_QA_QUERY_RESP	1

/* Information element ID defined in 802.11w specification. */
#define IE_PMF_MMIE				76

/* The definition in IEEE 802.11w - Table 7-32 Cipher suite selectors */
extern UCHAR		OUI_PMF_BIP_CIPHER[];

/* The definition in IEEE 802.11w - Table 7-34 AKM suite selectors */
extern UCHAR		OUI_PMF_8021X_AKM[];
extern UCHAR		OUI_PMF_PSK_AKM[];

/* The Key ID filed */
typedef union GNU_PACKED _PMF_IGTK_KEY_ID
{
	struct GNU_PACKED{
#ifdef RT_BIG_ENDIAN
	UINT16 :4;
	UINT16 KeyId:12;
#else
	UINT16 KeyId:12;
	UINT16 :4;
#endif
	} field;
	UINT16 word;
} PMF_IGTK_KEY_ID, *PPMF_IGTK_KEY_ID;

/* The SubIE of Fast BSS transition information element */
typedef struct GNU_PACKED _FT_IGTK_SUB_ELEMENT
{
	UINT8 KeyID[2];		/* indicates the value of the BIP key ID */
	UINT8 IPN[6];		/* indicates the receive sequence counter for the IGTK being installed */
	UINT8 KeyLen;		/* the length of IGTK in octets */
	UINT8 Key[24];		/* The length of the resulting AES-Keywrapped IGTK in the Key field */
} FT_IGTK_SUB_ELEMENT, *PFT_IGTK_SUB_ELEMENT;

/* Management MIC information element */
typedef struct GNU_PACKED _PMF_MMIE
{
	UINT8 KeyID[2];		/* identifies the IGTK used to compute the MIC */
	UINT8 IPN[6];		/* indicates the receive sequence counter for the IGTK being installed */	
	UINT8 MIC[LEN_PMF_BIP_MIC];		/* The length of the resulting AES-Keywrapped IGTK in the Key field */
} PMF_MMIE, *PPMF_MMIE;

/* IGTK KDE format */
typedef struct GNU_PACKED _PMF_IGTK_KDE
{
	UINT8 KeyID[2];		/* identifies the IGTK used to compute the MIC */
	UINT8 IPN[6];		/* indicates the receive sequence counter for the IGTK being installed */	
	UINT8 IGTK[0];		/* The length of the IGTK */
} PMF_IGTK_KDE, *PPMF_IGTK_KDE;


/* =====================
 * 	PMF SA Query Action 
 * ===================== */
typedef struct GNU_PACKED _PMF_SA_QUERY_ACTION
{	
	UCHAR	Category;	
	UCHAR	Action;

	/*  a 16-bit non-negative counter value */
	UINT8	TransactionID[2];
} PMF_SA_QUERY_ACTION, *PPMF_SA_QUERY_ACTION;

#endif /* DOT11W_PMF_SUPPORT */

#endif /* __DOT11W_PMF_H */

