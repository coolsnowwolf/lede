/*
  * Copyright (c) 2016 MediaTek Inc.  All rights reserved.
  *
  * This software is available to you under a choice of one of two
  * licenses.  You may choose to be licensed under the terms of the GNU
  * General Public License (GPL) Version 2, available from the file
  * COPYING in the main directory of this source tree, or the
  * BSD license below:
  *
  *     Redistribution and use in source and binary forms, with or
  *     without modification, are permitted provided that the following
  *     conditions are met:
  *
  *      - Redistributions of source code must retain the above
  *        copyright notice, this list of conditions and the following
  *        disclaimer.
  *
  *      - Redistributions in binary form must reproduce the above
  *        copyright notice, this list of conditions and the following
  *        disclaimer in the documentation and/or other materials
  *        provided with the distribution.
  *
  * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
  * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
  * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
  * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
  * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  * SOFTWARE.
  */

#ifndef _DIAG_H_
#define _DIAG_H_

#ifdef WIFI_DIAG

#include "rtmp_comm.h"
#include "rtmp_def.h"
#include "fpga/fpga_ctl.h"
#include "rtmp_timer.h"
#include "mlme.h"
#include "band_steering_def.h"
#include "rtmp_type.h"
#include "rtmp.h"



typedef struct _DIAG_FRAME_INFO {
	UCHAR isTX;
	/* for RX: pRxBlk->wcid=>MAC table entry => apidx => MBSSID[apidx] => ssid
	*   for TX: (1) pHeader->Addr1 => MAC table entry (exist) => apidx => MBSSID[apidx] => ssid
	*		  (2) if MAC table entry not exist, traverse all MBSSID entry
	*/
	UCHAR *ssid;
	UCHAR ssid_len;
	UCHAR band; /* 0:2G, 1:5G, get from pAd->LatchRfRegs.Channel */
	UCHAR *pData; /* include 80211 header, refer to pRxBlk (RX)/ pData (TX) */
	UINT32 dataLen; /* packet length */
} DIAG_FRAME_INFO;

typedef enum _ENUM_DIAG_CONN_ERROR_CODE {
	DIAG_CONN_FRAME_LOST = 0,
	DIAG_CONN_CAP_ERROR,
	DIAG_CONN_AUTH_FAIL,
	DIAG_CONN_ACL_BLK,
	DIAG_CONN_STA_LIM,
	DIAG_CONN_DEAUTH,
	DIAG_CONN_BAND_STE,
	DIAG_CONN_ERROR_MAX
} ENUM_DIAG_CONN_ERROR_CODE;

void DiagGetProcessInfo(PRTMP_ADAPTER	pAdapter, RTMP_IOCTL_INPUT_STRUCT	*wrq);
void DiagApMlmeOneSecProc(PRTMP_ADAPTER pAd);
void DiagAddPid(INT pid, PUCHAR name);
void DiagDelPid(INT32 pid);
void DiagFrameCache(DIAG_FRAME_INFO *info);
void DiagMiniportMMRequest(PRTMP_ADAPTER pAd, UCHAR *pData, UINT Length);
void DiagDevRxMgmtFrm(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk);
#ifdef CONFIG_SNIFFER_SUPPORT
void DiagDevRxCntlFrm(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk);
#endif
void DiagBcnTx(RTMP_ADAPTER *pAd, BSS_STRUCT *pMbss, UCHAR *pBeaconFrame, ULONG FrameLen);
void DiagConnError(PRTMP_ADAPTER pAd, UCHAR apidx, UCHAR *addr,
	ENUM_DIAG_CONN_ERROR_CODE Code, UINT32 Reason);
void DiagLogFileWrite(void);
void DiagAssocErrorFileWrite(void);
BOOLEAN DiagProcInit(PRTMP_ADAPTER pAd);
BOOLEAN DiagProcExit(PRTMP_ADAPTER pAd);
BOOLEAN DiagProcMemAllocate(void);
BOOLEAN DiagProcMemFree(void);
#endif
#endif /* #ifndef _DIAG_H_ */

