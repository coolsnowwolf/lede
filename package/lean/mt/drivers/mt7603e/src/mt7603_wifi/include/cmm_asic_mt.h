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
	cmm_asic_mt.h
*/

#ifndef __CMM_ASIC_MT_H__
#define __CMM_ASIC_MT_H__

VOID MTPciPollTxRxEmpty(struct _RTMP_ADAPTER *pAd);
VOID MTPciMlmeRadioOff(struct _RTMP_ADAPTER *pAd);
VOID MTPciMlmeRadioOn(struct _RTMP_ADAPTER *pAd);
VOID MTUsbPollTxRxEmpty(struct _RTMP_ADAPTER *pAd);
VOID MTUsbMlmeRadioOff(struct _RTMP_ADAPTER *pAd);
VOID MTUsbMlmeRadioOn(struct _RTMP_ADAPTER *pAd);
#endif
