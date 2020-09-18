/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2009, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of Ralink Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	hw_init.h

	Abstract:

	Revision History:
	Who			When	    What
	--------	----------  ----------------------------------------------
	Name		Date	    Modification logs
*/

#ifndef __HW_INIT_H__
#define __HW_INIT_H__

struct _RTMP_ADAPTER;

typedef struct {
	USHORT Dummy;/* empty struct will cause build error in testmode win */
} HIF_INFO_T;

/*HW related init*/
INT32 WfTopInit(struct _RTMP_ADAPTER *pAd);
INT32 WfHifInit(struct _RTMP_ADAPTER *pAd);
INT32 WfMcuInit(struct _RTMP_ADAPTER *pAd);
INT32 WfMacInit(struct _RTMP_ADAPTER *pAd);
INT32 WfEPROMInit(struct _RTMP_ADAPTER *pAd);
INT32 WfPhyInit(struct _RTMP_ADAPTER *pAd);

/*SW related init*/
INT32 WfSysPreInit(struct _RTMP_ADAPTER *pAd);
INT32 WfSysPosExit(struct _RTMP_ADAPTER *pAd);
INT32 WfSysCfgInit(struct _RTMP_ADAPTER *pAd);
INT32 WfSysCfgExit(struct _RTMP_ADAPTER *pAd);

/*OS dependence function*/
INT32 WfHifSysInit(struct _RTMP_ADAPTER *pAd, HIF_INFO_T *pHifInfo);
INT32 WfHifSysExit(struct _RTMP_ADAPTER *pAd);
INT32 WfMcuSysInit(struct _RTMP_ADAPTER *pAd);
INT32 WfMcuSysExit(struct _RTMP_ADAPTER *pAd);
INT32 WfEPROMSysInit(struct _RTMP_ADAPTER *pAd);
INT32 WfEPROMSysExit(struct _RTMP_ADAPTER *pAd);

/*Global*/
INT32 WfInit(struct _RTMP_ADAPTER *pAd);

#endif
