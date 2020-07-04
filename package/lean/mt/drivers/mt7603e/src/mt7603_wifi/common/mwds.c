/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 5F., No.36, Taiyuan St., Jhubei City,
 * Hsinchu County 302,
 * Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2009, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************


	Module Name:
	mwds.c

	Abstract:
	This is MWDS feature used to process those 4-addr of connected APClient or STA.

	Revision History:
	Who			When			What
	---------	----------	----------------------------------------------
 */
#ifdef MWDS
#include "rt_config.h"


VOID MWDSAPPeerEnable(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pEntry
)
{
	BOOLEAN mwds_enable = FALSE;
	BOOLEAN Ret = FALSE;

	if (pEntry->bSupportMWDS && pEntry->wdev && pEntry->wdev->bSupportMWDS)
		mwds_enable = TRUE;


	if (mwds_enable)
		Ret = a4_ap_peer_enable(pAd, pEntry, A4_TYPE_MWDS);

	if (Ret == FALSE)
		MWDSAPPeerDisable(pAd, pEntry);
}

VOID MWDSAPPeerDisable(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pEntry
)
{
	BOOLEAN Ret;

	Ret = a4_ap_peer_disable(pAd, pEntry, A4_TYPE_MWDS);
	if (Ret)
		pEntry->bSupportMWDS = FALSE;
}

#ifdef APCLI_SUPPORT
VOID MWDSAPCliPeerEnable(
	IN PRTMP_ADAPTER pAd,
	IN PAPCLI_STRUCT pApCliEntry,
	IN PMAC_TABLE_ENTRY pEntry
)
{
	BOOLEAN mwds_enable = FALSE;
	BOOLEAN Ret = FALSE;

	if (pApCliEntry->MlmeAux.bSupportMWDS && pApCliEntry->wdev.bSupportMWDS)
		mwds_enable = TRUE;


	if (mwds_enable)
		Ret = a4_apcli_peer_enable(pAd, pApCliEntry, pEntry, A4_TYPE_MWDS);

	if (Ret == FALSE)
		MWDSAPCliPeerDisable(pAd, pApCliEntry, pEntry);
}

VOID MWDSAPCliPeerDisable(
	IN PRTMP_ADAPTER pAd,
	IN PAPCLI_STRUCT pApCliEntry,
	IN PMAC_TABLE_ENTRY pEntry
)
{
	a4_apcli_peer_disable(pAd, pApCliEntry, pEntry, A4_TYPE_MWDS);
}
#endif /* APCLI_SUPPORT */

INT MWDSEnable(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR ifIndex,
	IN BOOLEAN isAP,
	IN BOOLEAN isDevOpen
)
{
	struct wifi_dev *wdev = NULL;

	if (isAP) {
		if (ifIndex < HW_BEACON_MAX_NUM) {
			wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;

			if (!wdev->bSupportMWDS) {
				wdev->bSupportMWDS = TRUE;
				a4_interface_init(pAd, ifIndex, TRUE, A4_TYPE_MWDS);
			}
		}
	}

#ifdef APCLI_SUPPORT
	else {
		if (ifIndex < MAX_APCLI_NUM) {
			wdev = &pAd->ApCfg.ApCliTab[ifIndex].wdev;

			if (!wdev->bSupportMWDS) {
				wdev->bSupportMWDS = TRUE;
				a4_interface_init(pAd, ifIndex, FALSE, A4_TYPE_MWDS);
			}
		}
	}

#endif /* APCLI_SUPPORT */

	return TRUE;
}

INT MWDSDisable(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR ifIndex,
	IN BOOLEAN isAP,
	IN BOOLEAN isDevClose
)
{
	struct wifi_dev *wdev = NULL;

	if (isAP) {
		if (ifIndex < HW_BEACON_MAX_NUM) {
			wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;

			if (wdev && wdev->bSupportMWDS) {
				wdev->bSupportMWDS = FALSE;
				a4_interface_deinit(pAd, ifIndex, TRUE, A4_TYPE_MWDS);
			}
		}
	}

#ifdef APCLI_SUPPORT
	else {
		if (ifIndex < MAX_APCLI_NUM) {
			wdev = &pAd->ApCfg.ApCliTab[ifIndex].wdev;

			if (wdev && wdev->bSupportMWDS) {
				wdev->bSupportMWDS = FALSE;
				a4_interface_deinit(pAd, ifIndex, FALSE, A4_TYPE_MWDS);
			}
		}
	}

#endif /* APCLI_SUPPORT */

	return TRUE;
}


INT Set_Enable_MWDS_Proc(
	IN  PRTMP_ADAPTER pAd,
	IN  BOOLEAN Enable,
	IN  BOOLEAN isAP
)
{
	POS_COOKIE      pObj;
	UCHAR           ifIndex;
	pObj = (POS_COOKIE) pAd->OS_Cookie;

	if (isAP) {
		ifIndex = pObj->ioctl_if;
		pAd->ApCfg.MBSSID[ifIndex].wdev.bDefaultMwdsStatus = (Enable == 0) ? FALSE : TRUE;
	}

#ifdef APCLI_SUPPORT
	else {
		if (pObj->ioctl_if_type != INT_APCLI)
			return FALSE;

		ifIndex = pObj->ioctl_if;
		pAd->ApCfg.ApCliTab[ifIndex].wdev.bDefaultMwdsStatus = (Enable == 0) ? FALSE : TRUE;
	}

#endif /* APCLI_SUPPORT */

	if (Enable)
		MWDSEnable(pAd, ifIndex, isAP, FALSE);
	else
		MWDSDisable(pAd, ifIndex, isAP, FALSE);

	return TRUE;
}

INT Set_Ap_MWDS_Proc(
	IN  PRTMP_ADAPTER pAd,
	IN  PSTRING arg
)
{
	UCHAR Enable;
	Enable = simple_strtol(arg, 0, 10);
	return Set_Enable_MWDS_Proc(pAd, Enable, TRUE);
}

INT Set_ApCli_MWDS_Proc(
	IN  PRTMP_ADAPTER pAd,
	IN  PSTRING arg
)
{
	UCHAR Enable;
	Enable = simple_strtol(arg, 0, 10);
	return Set_Enable_MWDS_Proc(pAd, Enable, FALSE);
}


VOID rtmp_read_MWDS_from_file(
	IN  PRTMP_ADAPTER pAd,
	PSTRING tmpbuf,
	PSTRING buffer
)
{
	PSTRING tmpptr = NULL;
#ifdef CONFIG_AP_SUPPORT

	/* ApMWDS */
	if (RTMPGetKeyParameter("ApMWDS", tmpbuf, 256, buffer, TRUE)) {
		INT Value;
		UCHAR i = 0;

		for (i = 0, tmpptr = rstrtok(tmpbuf, ";"); tmpptr; tmpptr = rstrtok(NULL, ";"), i++) {
			if (i >= pAd->ApCfg.BssidNum)
				break;

			Value = (INT) simple_strtol(tmpptr, 0, 10);

			if (Value == 0)
				MWDSDisable(pAd, i, TRUE, FALSE);
			else
				MWDSEnable(pAd, i, TRUE, FALSE);

			pAd->ApCfg.MBSSID[i].wdev.bDefaultMwdsStatus = (Value == 0) ? FALSE : TRUE;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ApMWDS=%d\n", Value));
		}
	}

#endif /* CONFIG_AP_SUPPORT */
#ifdef APCLI_SUPPORT

	/* ApCliMWDS */
	if (RTMPGetKeyParameter("ApCliMWDS", tmpbuf, 256, buffer, TRUE)) {
		INT Value;
		UCHAR i = 0;

		for (i = 0, tmpptr = rstrtok(tmpbuf, ";"); tmpptr; tmpptr = rstrtok(NULL, ";"), i++) {
			if (i >= MAX_APCLI_NUM)
				break;

			Value = (INT) simple_strtol(tmpptr, 0, 10);

			if (Value == 0)
				MWDSDisable(pAd, i, FALSE, FALSE);
			else
				MWDSEnable(pAd, i, FALSE, FALSE);

			pAd->ApCfg.ApCliTab[i].wdev.bDefaultMwdsStatus = (Value == 0) ? FALSE : TRUE;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ApCliMWDS=%d\n", Value));
		}
	}

#endif /* APCLI_SUPPORT */
}

#endif /* MWDS */
