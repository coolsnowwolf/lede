#ifndef __MWDS_H__
#define __MWDS_H__
/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 5F., No.36, Taiyuan St., Jhubei City,
 * Hsinchu County 302,
 * Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2009, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************


    Module Name:
	mwds.h
 
    Abstract:
    This is MWDS feature used to process those 4-addr of connected APClient or STA.
    
    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
 */
#ifdef MWDS
#include "rtmp_def.h"

#define MWDS_SUPPORT(B0)            ((B0)&0x80)

VOID MWDSAPPeerEnable(
	IN PRTMP_ADAPTER pAd,
	IN MAC_TABLE_ENTRY *pEntry);

VOID MWDSAPPeerDisable(
	IN PRTMP_ADAPTER pAd,
	IN MAC_TABLE_ENTRY *pEntry);

#ifdef APCLI_SUPPORT
VOID MWDSAPCliPeerEnable(
	IN PRTMP_ADAPTER pAd,
	IN PAPCLI_STRUCT pApCliEntry,
	IN MAC_TABLE_ENTRY *pEntry);

VOID MWDSAPCliPeerDisable(
	IN PRTMP_ADAPTER pAd,
	IN PAPCLI_STRUCT pApCliEntry,
	IN MAC_TABLE_ENTRY *pEntry);
#endif /* APCLI_SUPPORT */
INT MWDSEnable(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR ifIndex,
	IN BOOLEAN isAP,
	IN BOOLEAN isDevOpen);

INT MWDSDisable(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR ifIndex,
	IN BOOLEAN isAP,
	IN BOOLEAN isDevClose);

INT Set_Ap_MWDS_Proc(
	IN  PRTMP_ADAPTER pAd, 
	IN  PSTRING arg);

INT Set_ApCli_MWDS_Proc(
	IN  PRTMP_ADAPTER pAd, 
	IN  PSTRING arg);

VOID rtmp_read_MWDS_from_file(
	IN  PRTMP_ADAPTER pAd,
	PSTRING tmpbuf,
	PSTRING buffer);

#endif /* MWDS */
#endif /* __MWDS_H__*/
