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
	fwdl.c
*/

#ifdef COMPOS_WIN
#include "MtConfig.h"
#if defined(EVENT_TRACING)
#include "fwdl.tmh"
#endif
#elif defined(COMPOS_TESTMODE_WIN)
#include "config.h"
#else
#include "rt_config.h"
#endif

INT NICLoadRomPatch(RTMP_ADAPTER *ad)
{
	int ret = NDIS_STATUS_SUCCESS;

	ret = mt_load_patch(ad);

	return ret;
}


INT NICLoadFirmware(RTMP_ADAPTER *ad)
{
	int ret = NDIS_STATUS_SUCCESS;

	ret = mt_load_fw(ad);

	return ret;
}

VOID NICRestartFirmware(RTMP_ADAPTER *ad)
{
	int ret = NDIS_STATUS_SUCCESS;

	ret = mt_restart_fw(ad);
}


INT FwdlHookInit(RTMP_ADAPTER *pAd)
{
	int ret = NDIS_STATUS_SUCCESS;

	ret = mt_fwdl_hook_init(pAd);

	return ret;

}
