/*
 ***************************************************************************
 * MediaTek Inc.
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

	Module Name:
	cmm_rvr_dbg.h
*/


#ifndef __CMM_RVR_DBG_H__
#define __CMM_RVR_DBG_H__

#include "rt_config.h"

#define VIEW_ERROR			-1
#define VIEW_BASICINFO		 1 /*bit0*/
#define VIEW_WCID			 2 /*bit1*/
#define VIEW_MACCOUNTER		 4 /*bit2*/
#define VIEW_PHYCOUNTER		 8 /*bit3*/
#define VIEW_NOISE			16 /*bit4*/
#define VIEW_CNNUMBER		32 /*bit5*/
#define VIEW_6				64 /*bit6*/
#define VIEW_OTHERS 	   128 /*bit7*/

#define Case_ERROR			-1
#define Case_SHOW			 0
#define Case_SET		     1

#define BF_MASK		0x0000FFFF
#define BF_OFSET			16


typedef struct _RvR_Debug_CTRL {
	UINT8 ucViewLevel;
	UINT8 ucWcid;
	UINT8 ucCNcnt;
	UINT32 uiiBFTxcnt;
	UINT32 uieBFTxcnt;
} RvR_Debug_CTRL, *PRvR_Debug_CTRL;

INT RTMPIoctlRvRDebug(RTMP_ADAPTER *pAd, RTMP_IOCTL_INPUT_STRUCT *pRDCmdStr);
VOID RTMPIoctlRvRDebug_Init(RTMP_ADAPTER *pAd);
INT rd_dashboard(RTMP_ADAPTER *pAd, RTMP_IOCTL_INPUT_STRUCT *wrq);
/*rd sub-command */
INT rd_view(RTMP_ADAPTER *pAd, RTMP_STRING *arg, RTMP_IOCTL_INPUT_STRUCT *wrq);
INT rd_view_plus(RTMP_ADAPTER *pAd, RTMP_STRING *arg, RTMP_IOCTL_INPUT_STRUCT *wrq);
INT rd_view_minus(RTMP_ADAPTER *pAd, RTMP_STRING *arg, RTMP_IOCTL_INPUT_STRUCT *wrq);
INT rd_wcid(RTMP_ADAPTER *pAd, RTMP_STRING *arg, RTMP_IOCTL_INPUT_STRUCT *wrq);
INT rd_reset(RTMP_ADAPTER *pAd, RTMP_STRING *arg, RTMP_IOCTL_INPUT_STRUCT *wrq);
INT rd_help(RTMP_ADAPTER *pAd, RTMP_STRING *arg, RTMP_IOCTL_INPUT_STRUCT *wrq);
/* print log */
INT printBasicinfo(RTMP_ADAPTER *pAd, RTMP_STRING *msg);
VOID printView(RTMP_ADAPTER *pAd, RTMP_STRING *msg);
INT printWcid (RTMP_ADAPTER *pAd, RTMP_STRING *msg);
INT printMacCounter (RTMP_ADAPTER *pAd, RTMP_STRING *msg);
INT printPhyCounter (RTMP_ADAPTER *pAd, RTMP_STRING *msg);
INT printNoise (RTMP_ADAPTER *pAd, RTMP_STRING *msg);
INT printOthers (RTMP_ADAPTER *pAd, RTMP_STRING *msg);
INT printCNNum(RTMP_ADAPTER *pAd, RTMP_STRING *msg);
/*Update */
INT updateBFTxCnt(RTMP_ADAPTER *pAd);
INT updateCNNum(RTMP_ADAPTER *pAd, BOOLEAN Is_Enable);
/*Set */
INT setRXV2(RTMP_ADAPTER *pAd, BOOLEAN Is_Enable);
INT setCNNum(RTMP_ADAPTER *pAd, BOOLEAN Is_Enable);

INT getViewLevelValue(RTMP_STRING *msg);
#endif /* __CMM_RVR_DBG_H__ */
