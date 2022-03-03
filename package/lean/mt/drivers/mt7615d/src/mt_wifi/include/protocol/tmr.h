/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
    tmr.h

    Abstract:
    802.11v-Timing Measurement,
    802.11mc-Fine Timing Measurement related function and state machine

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    Carter      2014-1120     create

*/

#ifndef _TMR_H_
#define _TMR_H_

#ifdef MT_MAC
#ifndef COMPOS_TESTMODE_WIN
#include "rtmp.h"
#endif

enum TMR_TYPE {
	TMR_DISABLE = 0,
	TMR_INITIATOR,
	TMR_RESPONDER
};

enum TMR_IDENRIRY {
	TMR_IR0_TX = 0,
	TMR_IR1_RX = 1
};

enum TMR_INITIATOR_SEND_PKT_STATE {
	SEND_IDLE = 0,
	SEND_OUT
};

VOID TmrReportParser(struct _RTMP_ADAPTER *pAd, TMR_FRM_STRUC *tmr,
	BOOLEAN fgFinalResult, UINT32 TOAECalibrationResult);

VOID MtSetTmrEnable(struct _RTMP_ADAPTER *pAd, UCHAR enable);
INT TmrCtrlInit(struct _RTMP_ADAPTER *pAd, UCHAR TmrType, UCHAR Ver);

VOID TmrCtrl(struct _RTMP_ADAPTER *pAd, UCHAR enable, UCHAR Ver);

#endif /* MT_MAC */
#endif /* _TMR_H_ */
