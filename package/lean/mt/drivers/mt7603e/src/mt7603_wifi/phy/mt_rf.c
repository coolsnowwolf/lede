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

#include "rt_config.h"
INT32 MTShowPartialRF(RTMP_ADAPTER *pAd, UINT32 Start, UINT32 End)
{
	UINT32 RFIdx, Offset, Value;

#ifdef MT_MAC
	for (RFIdx = 0; RFIdx < pAd->Antenna.field.TxPath; RFIdx++)
	{
		for (Offset = Start; Offset <= End; Offset = Offset + 4)
		{
			CmdRFRegAccessRead(pAd, RFIdx, Offset, &Value);
			DBGPRINT(RT_DEBUG_OFF, ("%s():%d 0x%04x 0x%08x\n", __FUNCTION__, RFIdx, Offset, Value));
		}
	}
#endif /* MT_MAC */

	return TRUE;
}


INT32 MTShowAllRF(RTMP_ADAPTER *pAd)
{
	UINT32 RFIdx, Offset, Value;

#ifdef MT_MAC
	for (RFIdx = 0; RFIdx < pAd->Antenna.field.TxPath; RFIdx++)
	{
		for (Offset = 0; Offset <= (pAd->chipCap.MaxNumOfRfId * 4); Offset = Offset + 4)
		{
			CmdRFRegAccessRead(pAd, RFIdx, Offset, &Value);
			DBGPRINT(RT_DEBUG_OFF, ("%s():%d 0x%04x 0x%08x\n", __FUNCTION__, RFIdx, Offset, Value));
		}
	}
#endif /* MT_MAC */

	return TRUE;
}

