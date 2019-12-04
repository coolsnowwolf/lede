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
	Management Frame Protection Required is defined in IEEE802.11w

	Define all structures, data types that rtmp.h needed in this file. Don't
	put any sturctures and functions definition which refer to RTMP_ADAPTER
	Here.

***************************************************************************/


#ifdef DOT11W_PMF_SUPPORT

#ifndef __PMF_CMM_H__
#define __PMF_CMM_H__

#include "rtmp_type.h"
#include "security/dot11w_pmf.h"

#define	NORMAL_FRAME                    0
#define	ERROR_FRAME                     1
#define	NOT_ROBUST_GROUP_FRAME		2
#define	NOT_ROBUST_UNICAST_FRAME	3
#define	UNICAST_ROBUST_FRAME		4
#define	GROUP_ROBUST_FRAME		5

typedef enum _PMF_STATUS_NUM {
	PMF_STATUS_SUCCESS,
	PMF_POLICY_VIOLATION,
	PMF_UNICAST_ENCRYPT_FAILURE,
	PMF_ENCAP_BIP_FAILURE,
	PMF_UNICAST_DECRYPT_FAILURE,
	PMF_EXTRACT_BIP_FAILURE,
	PMF_STATUS_RESV
} PMF_STATUS_NUM;

/*
 * Management Frame Protection Required is defined in IEEE802.11w
 */
typedef struct GNU_PACKED _PMF_CFG {
	/*
	 * A STA sets this bit to 1 to advertise that protection of Robust
	 * Management Frames is enabled.
	 */
	BOOLEAN	MFPC;           /* This is actual active */
	BOOLEAN	Desired_MFPC;	/* This is user desired */

	/*
	 * A STA sets this bit to 1 to advertise that protection of
	 * Robust Management Frames is mandatory.
	 * If a STA sets this bit to 1, then that STA only allows RSNAs
	 * from STAs which provide Management Frame Protection.
	 */
	BOOLEAN	MFPR;           /* This is actual active */
	BOOLEAN	Desired_MFPR;	/* This is user desired */

	BOOLEAN	PMFSHA256;         /* This is actual active */
	BOOLEAN	Desired_PMFSHA256; /* This is user desired */

	/* Connect State */
	BOOLEAN	UsePMFConnect;

	UINT32 igtk_cipher;

	UINT8	IGTK_KeyIdx;			/* It shall be 4 or 5 */
	UCHAR	IGTK[2][LEN_MAX_IGTK];
	UCHAR	IPN[2][LEN_WPA_TSC];

	UCHAR PmfTxTsc[LEN_WPA_TSC];
	UCHAR PmfRxTsc[LEN_WPA_TSC];
	RALINK_TIMER_STRUCT SAQueryTimer;
	RALINK_TIMER_STRUCT SAQueryConfirmTimer;
	UCHAR SAQueryStatus;
	USHORT TransactionID;
} PMF_CFG, *PPMF_CFG;


#endif /* __PMF_CMM_H__ */
#endif /* DOT11W_PMF_SUPPORT */


