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
	oid.h

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs

	Carter Chen	2018-Oct-11		move oid_related data structure to here.
*/
#ifndef _OID_STRUCT_H_
#define _OID_STRUCT_H_

/*--------OID_QUERY_FEATURE_SUP_LIST start-----------*/

/*beginning of chip capability supporting list*/
#define WIFI_CAP_HT_MODE_BITS		0
#define WIFI_CAP_HT_MODE_MASK		0x00000001

#define WIFI_CAP_VHT_MODE_BITS		1
#define WIFI_CAP_VHT_MODE_MASK		0x00000002

#define WIFI_CAP_HE_MODE_BITS		2
#define WIFI_CAP_HE_MODE_MASK		0x00000004

#define WIFI_CAP_DBDC_SUP_MODE_BITS	3
#define WIFI_CAP_DBDC_SUP_MODE_MASK	0x00000018
#define WIFI_CAP_DBDC_SUP_MODE_NONE	0
#define WIFI_CAP_DBDC_SUP_MODE_2G5G	1 /*normal dbdc supporting*/
#define WIFI_CAP_DBDC_SUP_MODE_5G5G	2 /*for instance, 7615A*/

#define WIFI_CAP_CH_SUP_MODE_BITS	5
#define WIFI_CAP_CH_SUP_MODE_MASK	0x00000060
#define WIFI_CAP_CH_SUP_MODE_2G		0
#define WIFI_CAP_CH_SUP_MODE_5G		1 /*5G only supporting*/
#define WIFI_CAP_CH_SUP_MODE_2G5G	2

#define WIFI_CAP_ADV_SEC_SUP_BITS	7 /*CCMP-256/GCMP-128/GCMP-256*/
#define WIFI_CAP_ADV_SEC_SUP_MASK	0x00000080

#define WIFI_CAP_BF_SUP_BITS		8
#define WIFI_CAP_BF_SUP_MASK		0x00000100

#define WIFI_CAP_MU_MIMO_SUP_BITS	9
#define WIFI_CAP_MU_MIMO_SUP_MASK	0x00000200

#define WIFI_CAP_MBSS_NUM_BITS		10
#define WIFI_CAP_MBSS_NUM_MASK		0x00007C00

#define WIFI_CAP_TX_STR_NUM_BITS	15
#define WIFI_CAP_TX_STR_NUM_MASK	0x00078000

#define WIFI_CAP_RX_STR_NUM_BITS	19
#define WIFI_CAP_RX_STR_NUM_MASK	0x00780000
/*end of chip capability supporting list*/

/*beginning of security supporting list*/
#define WIFI_SEC_WEP_SUP_BITS		0
#define WIFI_SEC_WEP_SUP_MASK		0x00000001

#define WIFI_SEC_WAPI_SUP_BITS		1
#define WIFI_SEC_WAPI_SUP_MASK		0x00000002

#define WIFI_SEC_PMF_SUP_BITS		2
#define WIFI_SEC_PMF_SUP_MASK		0x00000004

#define WIFI_SEC_WPA1_SUP_BITS		3
#define WIFI_SEC_WPA1_SUP_MASK		0x00000008

#define WIFI_SEC_WPA1PSK_SUP_BITS	4
#define WIFI_SEC_WPA1PSK_SUP_MASK	0x00000010

#define WIFI_SEC_WPA2_SUP_BITS		5
#define WIFI_SEC_WPA2_SUP_MASK		0x00000020

#define WIFI_SEC_WPA2PSK_SUP_BITS	6
#define WIFI_SEC_WPA2PSK_SUP_MASK	0x00000040

#define WIFI_SEC_WPA3_SUP_BITS		7
#define WIFI_SEC_WPA3_SUP_MASK		0x00000080

#define WIFI_SEC_WPA3PSK_SUP_BITS	8
#define WIFI_SEC_WPA3PSK_SUP_MASK	0x00000100

#define WIFI_SEC_WPA3SUITEB192_SUP_BITS	9
#define WIFI_SEC_WPA3SUITEB192_SUP_MASK	0x00000200

#define WIFI_SEC_ENHANCE_OPEN_SUP_BITS	10
#define WIFI_SEC_ENHANCE_OPEN_SUP_MASK	0x00000400
/*end of security supporting list*/

/*beginning of wifi feature supporting list*/
#define WIFI_FEATURE_AP_SUP_BITS	0
#define WIFI_FEATURE_AP_SUP_MASK	0x00000001

#define WIFI_FEATURE_STA_SUP_BITS	1
#define WIFI_FEATURE_STA_SUP_MASK	0x00000002

#define WIFI_FEATURE_REPT_SUP_BITS	2
#define WIFI_FEATURE_REPT_SUP_MASK	0x00000004

#define WIFI_FEATURE_WDS_SUP_BITS	3
#define WIFI_FEATURE_WDS_SUP_MASK	0x00000008

#define WIFI_FEATURE_WPS_SUP_BITS	4
#define WIFI_FEATURE_WPS_SUP_MASK	0x00000010

#define WIFI_FEATURE_DFS_SUP_BITS	5
#define WIFI_FEATURE_DFS_SUP_MASK	0x00000020

#define WIFI_FEATURE_VOW_SUP_BITS	6
#define WIFI_FEATURE_VOW_SUP_MASK	0x00000040

#define WIFI_FEATURE_PASSPOINT_SUP_BITS	7
#define WIFI_FEATURE_PASSPOINT_SUP_MASK	0x00000080

#define WIFI_FEATURE_11R_SUP_BITS	8
#define WIFI_FEATURE_11R_SUP_MASK	0x00000100

#define WIFI_FEATURE_11K_SUP_BITS	9
#define WIFI_FEATURE_11K_SUP_MASK	0x00000200

#define WIFI_FEATURE_11V_SUP_BITS	10
#define WIFI_FEATURE_11V_SUP_MASK	0x00000400

#define WIFI_FEATURE_MBO_SUP_BITS	11
#define WIFI_FEATURE_MBO_SUP_MASK	0x00000800

#define WIFI_FEATURE_MAP_SUP_BITS	12
#define WIFI_FEATURE_MAP_SUP_MASK	0x00001000
/*end of wifi feature supporting list*/

struct wifi_feature_support_list_query {
	UINT32	chip_cap_list;
	UINT32	sec_cap_list;
	UINT32	adv_feature_cap_list;
	UINT32	reserved_cap[5];/*future extension.*/
};

/*--------OID_QUERY_FEATURE_SUP_LIST end-----------*/

#endif /* _OID_STRUCT_H_ */


