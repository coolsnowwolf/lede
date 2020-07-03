/*
 ***************************************************************************
 * Mediatek Tech Inc.
 * No.1, Dusing 1st Rd.,
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2018, Mediatek Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of Ralink Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	misc_app.h

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs

	Carter Chen	2018-Oct-11		misc applications definitions.
*/
#ifndef _MISC_APP_H_
#define _MISC_APP_H_

#include "rtmp_type.h"
#include "rtmp_def.h"
#include "rtmp.h"

enum wifi_cap_type {
	WIFI_CAP_CHIP = 0,
	WIFI_CAP_SEC,
	WIFI_CAP_FEATURE,
	WIFI_CAP_MAX, /*further capabilities, please add before WIFI_CAP_MAX*/
};

enum wifi_chip_cap_type {
	WIFI_CHIP_HT_MODE_SUP = 0,
	WIFI_CHIP_VHT_MODE_SUP,
	WIFI_CHIP_HE_MODE_SUP,
	WIFI_CHIP_DBDC_MODE_SUP,
	WIFI_CHIP_CH_SUP,
	WIFI_CHIP_ADV_SEC_MODE_SUP,
	WIFI_CHIP_BF_SUP,
	WIFI_CHIP_MU_MIMO_SUP,
	WIFI_CHIP_MBSS_NUM,
	WIFI_CHIP_TX_STR_NUM,
	WIFI_CHIP_RX_STR_NUM,
	WIFI_CHIP_CAP_MAX, /*further capabilities, please add before WIFI_CHIP_CAP_MAX*/
};

enum wifi_sec_cap_type {
	WIFI_SEC_WEP = 0,
	WIFI_SEC_WAPI,
	WIFI_SEC_PMF,
	WIFI_SEC_WPA1,
	WIFI_SEC_WPA1PSK,
	WIFI_SEC_WPA2,
	WIFI_SEC_WPA2PSK,
	WIFI_SEC_WPA3,
	WIFI_SEC_WPA3PSK,
	WIFI_SEC_WPA3_SUITE_B_192,
	WIFI_SEC_WPA3_ENHANCE_OPEN,
	WIFI_SEC_CAP_MAX, /*further capabilities, please add before WIFI_SEC_CAP_MAX*/
};

enum wifi_feature_cap_type {
	WIFI_FEATURE_AP = 0,
	WIFI_FEATURE_STA,
	WIFI_FEATURE_REPT,
	WIFI_FEATURE_WDS,
	WIFI_FEATURE_WPS,
	WIFI_FEATURE_DFS,
	WIFI_FEATURE_VOW,
	WIFI_FEATURE_PASSPOINT,
	WIFI_FEATURE_11R,
	WIFI_FEATURE_11K,
	WIFI_FEATURE_11V,
	WIFI_FEATURE_MBO,
	WIFI_FEATURE_MAP,
	WIFI_FEATURE_MAX, /*further capabilities, please add before WIFI_FEATURE_MAX*/
};

enum wifi_cap_operation_status {
	WIFI_CAP_STATUS_OK = 0,
	WIFI_CAP_STATUS_FAIL,
	WIFI_CAP_STATUS_INVALID_PARAMETER,
};

enum wifi_cap_operation_status wifi_sup_list_register(struct _RTMP_ADAPTER *ad,
						      enum wifi_cap_type cap_type);

enum wifi_cap_operation_status wifi_sup_list_unregister(struct _RTMP_ADAPTER *ad,
							enum wifi_cap_type cap_type,
							int sub_type);

INT show_wifi_cap_list(struct _RTMP_ADAPTER *ad, char *arg);

#endif /* _MISC_APP_H_ */


