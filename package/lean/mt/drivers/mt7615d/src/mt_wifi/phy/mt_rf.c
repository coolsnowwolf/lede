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
	mt_rf.c
*/
#ifdef COMPOS_WIN
#include "MtConfig.h"
#if defined(EVENT_TRACING)
#include "mt_rf.tmh"
#endif
#else
#include "rt_config.h"
#endif

INT32 MTShowPartialRF(RTMP_ADAPTER *pAd, UINT32 Start, UINT32 End)
{
#ifdef COMPOS_WIN
#else
	UINT32 RFIdx, Offset, Value;

#ifdef MT_MAC
	for (RFIdx = 0; RFIdx < pAd->Antenna.field.TxPath; RFIdx++) {
		for (Offset = Start; Offset <= End; Offset = Offset + 4) {
			MtCmdRFRegAccessRead(pAd, RFIdx, Offset, &Value);
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s():%d 0x%04x 0x%08x\n", __func__, RFIdx, Offset, Value));
		}
	}
#endif /* MT_MAC */
#endif
	return TRUE;
}


INT32 MTShowAllRF(RTMP_ADAPTER *pAd)
{
#ifdef COMPOS_WIN
#else
	UINT32 RFIdx, Offset, Value;

#ifdef MT_MAC
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	for (RFIdx = 0; RFIdx < pAd->Antenna.field.TxPath; RFIdx++) {
		for (Offset = 0; Offset <= (cap->MaxNumOfRfId * 4); Offset = Offset + 4) {
			MtCmdRFRegAccessRead(pAd, RFIdx, Offset, &Value);
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s():%d 0x%04x 0x%08x\n", __func__, RFIdx, Offset, Value));
		}
	}
#endif /* MT_MAC */
#endif
	return TRUE;
}

