/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology	5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering	the source code	is stricitly prohibited, unless	the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	pbf.h

	Abstract:
	Ralink Wireless Chip MAC related definition & structures

	Revision History:
	Who			When		  What
	--------	----------	  ----------------------------------------------
*/

#ifndef __PBF_H__
#define __PBF_H__


#ifdef RLT_MAC
#include "mac/mac_ral/nmac/ral_nmac_pbf.h"
#endif /* RLT_MAC */

#ifdef RTMP_MAC
#include "mac/mac_ral/omac/ral_omac_pbf.h"
#endif /* RTMP_MAC */


/* ================================================================================= */
/* Register format  for PBF                                                                                                                                                     */
/* ================================================================================= */


#define US_CYC_CNT      0x02a4
#ifdef RT_BIG_ENDIAN
typedef	union _US_CYC_CNT_STRUC {
	struct {
	    UINT32  rsv2:7;
	    UINT32  TestEn:1;
	    UINT32  TestSel:8;
	    UINT32  rsv1:7;
	    UINT32  MiscModeEn:1;
	    UINT32  UsCycCnt:8;
	} field;
	UINT32 word;
} US_CYC_CNT_STRUC;
#else
typedef	union _US_CYC_CNT_STRUC {
	struct {
		UINT32  UsCycCnt:8;
		UINT32  MiscModeEn:1;
		UINT32  rsv1:7;
		UINT32  TestSel:8;
		UINT32  TestEn:1;
		UINT32  rsv2:7;
	} field;
	UINT32 word;
} US_CYC_CNT_STRUC;
#endif


#define PBF_SYS_CTRL 	 0x0400


#define PBF_CTRL			0x0410
#define MCU_INT_STA		0x0414
#define MCU_INT_ENA	0x0418
#define TXRXQ_PCNT		0x0438
#define PBF_DBG			0x043c


#endif /* __PBF_H__ */

