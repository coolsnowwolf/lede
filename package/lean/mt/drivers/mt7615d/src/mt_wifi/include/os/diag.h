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
#include "rtmp_type.h"
#include "rtmp_os.h"
#include "rtmp.h"

typedef enum _ENUM_DIAG_CONN_ERROR_CODE{
	DIAG_CONN_FRAME_LOST = 0,
	DIAG_CONN_CAP_ERROR,
	DIAG_CONN_AUTH_FAIL,
	DIAG_CONN_ACL_BLK,
	DIAG_CONN_STA_LIM,
	DIAG_CONN_DEAUTH,
	DIAG_CONN_BAND_STE,
	DIAG_CONN_ERROR_MAX,
	DIAG_CONN_DEAUTH_COM
}ENUM_DIAG_CONN_ERROR_CODE;


void DiagConnError(PRTMP_ADAPTER pAd, UCHAR apidx, UCHAR* addr, 
	ENUM_DIAG_CONN_ERROR_CODE Code, UINT32 Reason);
void DiagConnErrorWrite(PRTMP_ADAPTER pAd);
void DiagAddPid(OS_TASK *pTask);
void DiagDelPid(OS_TASK *pTask);
void DiagGetProcessInfo(PRTMP_ADAPTER	pAdapter, RTMP_IOCTL_INPUT_STRUCT	*wrq);
void DiagMiniportMMRequest(PRTMP_ADAPTER pAd, UCHAR *pData, UINT Length);
void DiagBcnTx(RTMP_ADAPTER *pAd, BSS_STRUCT *pMbss, UCHAR *pBeaconFrame,ULONG FrameLen );
void DiagLogFileWrite(PRTMP_ADAPTER pAd);
void DiagDevRxMgmtFrm(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk);
void DiagApMlmeOneSecProc(PRTMP_ADAPTER pAd);
void DiagCtrlAlloc(PRTMP_ADAPTER pAd);
void DiagCtrlFree(PRTMP_ADAPTER pAd);
BOOLEAN DiagProcInit(PRTMP_ADAPTER pAd);
BOOLEAN DiagProcExit(PRTMP_ADAPTER pAd);

#endif
#endif /* #ifndef _DIAG_H_ */

