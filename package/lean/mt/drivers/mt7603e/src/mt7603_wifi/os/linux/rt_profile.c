/****************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ****************************************************************************

    Module Name:
	rt_profile.c
 
    Abstract:
 
    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
 */
 
#include "rt_config.h"

#if defined(BB_SOC) && defined(BB_RA_HWNAT_WIFI)
#include <linux/foe_hook.h>
#endif

#if defined (CONFIG_RA_HW_NAT)  || defined (CONFIG_RA_HW_NAT_MODULE)
#include "../../../../../net/nat/hw_nat/ra_nat.h"
#include "../../../../../net/nat/hw_nat/frame_engine.h"
#endif


#if defined(ANDROID_SUPPORT) || defined(RT_CFG80211_SUPPORT)
#define SECOND_INF_MAIN_DEV_NAME	"wlani"
#define SECOND_INF_MBSSID_DEV_NAME	"wlani"
#else
#define SECOND_INF_MAIN_DEV_NAME	"rai"
#define SECOND_INF_MBSSID_DEV_NAME	"rai"
#endif /*#if defined(ANDROID_SUPPORT) || defined(RT_CFG80211_SUPPORT)*/

#define SECOND_INF_WDS_DEV_NAME		"wdsi"
#define SECOND_INF_APCLI_DEV_NAME	"apclii"
#define SECOND_INF_MESH_DEV_NAME		"meshi"
#define SECOND_INF_P2P_DEV_NAME		"p2pi"
#define SECOND_INF_MONITOR_DEV_NAME		"moni"


#define xdef_to_str(s)   def_to_str(s) 
#define def_to_str(s)    #s

#if defined(CONFIG_SUPPORT_OPENWRT)
#define FIRST_EEPROM_FILE_PATH	"/etc/wireless/mt7603e/mt7603.eeprom.dat"
#define FIRST_AP_PROFILE_PATH		"/etc/wireless/mt7603e/mt7603.dat"
#else
#define FIRST_EEPROM_FILE_PATH	"/etc_ro/Wireless/RT2860/"
#define FIRST_AP_PROFILE_PATH		"/etc/Wireless/RT2860/RT2860AP.dat"
#endif
#define FIRST_CHIP_ID	xdef_to_str(CONFIG_RT_FIRST_CARD)

#if defined(CONFIG_SUPPORT_OPENWRT)
#define SECOND_EEPROM_FILE_PATH	"/etc/wireless/mt7603e/mt7603.eeprom.dat"
#define SECOND_AP_PROFILE_PATH		"/etc/wireless/mt7603e/mt7603.dat"
#else
#define SECOND_EEPROM_FILE_PATH	"/etc_ro/Wireless/iNIC/"
#define SECOND_AP_PROFILE_PATH	"/etc/Wireless/iNIC/iNIC_ap.dat"
#endif
#define SECOND_CHIP_ID	xdef_to_str(CONFIG_RT_SECOND_CARD)


#define MAX_L1PROFILE_INDEX	10
#define MAX_NUM_OF_INF               2

#ifndef CONFIG_RT_FIRST_IF_RF_OFFSET
#define CONFIG_RT_FIRST_IF_RF_OFFSET DEFAULT_RF_OFFSET
#endif

#ifndef CONFIG_RT_SECOND_IF_RF_OFFSET
#define CONFIG_RT_SECOND_IF_RF_OFFSET DEFAULT_RF_OFFSET
#endif

struct dev_type_name_map_t {
	INT type;
	RTMP_STRING prefix[IFNAMSIZ];
};

struct l1profile_info_t {
	RTMP_STRING profile_index[L1PROFILE_INDEX_LEN];
	RTMP_STRING profile_path[L2PROFILE_PATH_LEN];
	eeprom_flash_info ee_info;
	struct dev_type_name_map_t dev_name_map[MAX_INT_TYPES+1];
	RTMP_STRING single_sku_path[L2PROFILE_PATH_LEN];
};

struct l1profile_attribute_t {
	RTMP_STRING name[L1PROFILE_ATTRNAME_LEN];
	UINT32	extra;
	INT	(*handler)(RTMP_ADAPTER *pAd, UINT32 extra, RTMP_STRING *value);
};


static struct l1profile_info_t l1profile[MAX_NUM_OF_INF] = {
{{0}, FIRST_AP_PROFILE_PATH,  {CONFIG_RT_FIRST_IF_RF_OFFSET, EEPROM_SIZE},
									   {{INT_MAIN, INF_MAIN_DEV_NAME},
									   {INT_MBSSID, INF_MBSSID_DEV_NAME},
									   {INT_WDS, INF_WDS_DEV_NAME},
									   {INT_APCLI,	INF_APCLI_DEV_NAME},
									   {INT_MESH, INF_MESH_DEV_NAME},
									   {INT_P2P, INF_P2P_DEV_NAME},
									   {INT_MONITOR, INF_MONITOR_DEV_NAME},
									   {0} }
#ifdef SINGLE_SKU_V2
									   , {SINGLE_SKU_TABLE_FILE_NAME}
#endif /* SINGLE_SKU_V2 */
										},

{{1}, SECOND_AP_PROFILE_PATH,   {CONFIG_RT_SECOND_IF_RF_OFFSET, EEPROM_SIZE},
									   {{INT_MAIN,	SECOND_INF_MAIN_DEV_NAME},
									   {INT_MBSSID,	SECOND_INF_MBSSID_DEV_NAME},
									   {INT_WDS, SECOND_INF_WDS_DEV_NAME},
									   {INT_APCLI, SECOND_INF_APCLI_DEV_NAME},
									   {INT_MESH, SECOND_INF_MESH_DEV_NAME},
									   {INT_P2P, SECOND_INF_P2P_DEV_NAME},
									   {INT_MONITOR, SECOND_INF_MONITOR_DEV_NAME},
									   {0} }
#ifdef SINGLE_SKU_V2
									   , {SINGLE_SKU_TABLE_FILE_NAME}
#endif /* SINGLE_SKU_V2 */
										}
};


struct dev_id_name_map{
	INT chip_id;
	RTMP_STRING *chip_name;
};

static const struct dev_id_name_map id_name_list[]=
{
	{7610, "7610, 7610e 7610u"},

};


static NDIS_STATUS l1set_profile_path(RTMP_ADAPTER *pAd, UINT32 extra, RTMP_STRING *value)
{
	INT retVal = NDIS_STATUS_SUCCESS;
	RTMP_STRING *target = l1profile[get_dev_config_idx(pAd)].profile_path;

	if (strcmp(target, value)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s() profile update from %s to %s\n",
			__func__, target, value));
		strncpy(target, value, L2PROFILE_PATH_LEN - 1);
	} else
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s() profile remain %s\n", __func__, target));

	return retVal;
}

static NDIS_STATUS l1set_eeprom_bin(RTMP_ADAPTER *pAd, UINT32 extra, RTMP_STRING *value)
{
	INT retVal = NDIS_STATUS_SUCCESS;
	RTMP_STRING *target = l1profile[get_dev_config_idx(pAd)].ee_info.bin_name;
	UINT8 str_len;

	str_len = strlen(value);
	if (strcmp(target, value) && (str_len < L1PROFILE_ATTRNAME_LEN)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("eeprom binary update from %s to %s\n", target, value));
		strcpy(target, value);
		*(target+str_len) = '\0';
	} else
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("eeprom binary remain %s\n", target));

	return retVal;
}


static NDIS_STATUS l1set_eeprom_offset(RTMP_ADAPTER *pAd, UINT32 extra, RTMP_STRING *value)
{
	INT retVal = NDIS_STATUS_SUCCESS;
	eeprom_flash_info *target = (eeprom_flash_info *)(&(l1profile[get_dev_config_idx(pAd)].ee_info));
	UINT32	int_value = 0;

	int_value = simple_strtol(value, NULL, 0);

	if (target->offset != int_value) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s() eeprom offset update from 0x%x to 0x%x\n", __func__, target->offset, int_value));
		target->offset = int_value;
	} else
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s() eeprom offset remain 0x%x\n", __func__, target->offset));

	return retVal;
}


static NDIS_STATUS l1set_eeprom_size(RTMP_ADAPTER *pAd, UINT32 extra, RTMP_STRING *value)
{
	UINT32 int_value;
	INT retVal = NDIS_STATUS_SUCCESS;
	eeprom_flash_info *target = (eeprom_flash_info *)(&(l1profile[get_dev_config_idx(pAd)].ee_info));

	int_value = simple_strtol(value, NULL, 0);

	if (target->size != int_value) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s() eeprom size update from 0x%x to 0x%x\n", __func__, target->size, int_value));
		target->size = int_value;
	} else
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("%s() eeprom size remain 0x%x\n", __func__, target->size));

	return retVal;
}


#ifdef SINGLE_SKU_V2
static NDIS_STATUS l1set_single_sku_path(RTMP_ADAPTER *pAd, UINT32 extra, RTMP_STRING *value)
{
	INT retVal = NDIS_STATUS_SUCCESS;
	RTMP_STRING *target = l1profile[get_dev_config_idx(pAd)].single_sku_path;

	if (strcmp(target, value)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s() sku path update from %s to %s\n", __func__, target, value));
		strncpy(target, value, L2PROFILE_PATH_LEN - 1);
	} else
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s() profile remain %s\n", __func__, target));

	return retVal;
}
#endif /*SINGLE_SKU_V2*/

static NDIS_STATUS l1set_ifname(RTMP_ADAPTER *pAd, UINT32 extra, RTMP_STRING *value)
{
	INT retVal = NDIS_STATUS_SUCCESS;
	RTMP_STRING *target = NULL;
	RTMP_STRING realValue[IFNAMSIZ] = {0};
	INT len = strlen(value);
	
	target = get_dev_name_prefix(pAd, extra);
	if (target == NULL) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s(): get_dev_name_prefix is NULL\n", __func__));
		return NDIS_STATUS_FAILURE;
	}

	strncpy(realValue, value, IFNAMSIZ);
	  if (extra == INT_MAIN) {
		realValue[len - 1] = '\0';
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s() ifname rename from %s to %s\n", __func__, value, realValue));
	  }

	if (strcmp(target, realValue)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s() ifname update from %s to %s\n", __func__, target, realValue));

		strncpy(target, realValue, IFNAMSIZ);
	} else
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s() ifname remain %s\n", __func__, target));

	return retVal;
}

static NDIS_STATUS is_dup_key(RTMP_STRING *key)
{
	INT retVal = NDIS_STATUS_SUCCESS;
	INT dev_idx = 0;
	RTMP_STRING *profile_index = NULL;

	for (dev_idx = 0; dev_idx < MAX_NUM_OF_INF; dev_idx++) {
		profile_index = l1profile[dev_idx].profile_index;

		if ((strlen(profile_index) > 0) && (strcmp(profile_index, key) == 0)) {
			retVal = NDIS_STATUS_FAILURE;
			dev_idx = MAX_NUM_OF_INF;	/* tend to leave loop */
		}
	}

	return retVal;
}

static NDIS_STATUS match_index_by_chipname(IN RTMP_STRING *l1profile_data,
					   IN RTMP_ADAPTER *pAd,
					   IN RTMP_STRING *chipName)
{
	INT retVal = NDIS_STATUS_FAILURE;
	INT if_idx = 0;
	RTMP_STRING	key[10] = {0};
	RTMP_STRING *tmpbuf = NULL;

	os_alloc_mem(NULL, (UCHAR **)&tmpbuf, MAX_PARAM_BUFFER_SIZE);

	while (if_idx < MAX_L1PROFILE_INDEX) {
		sprintf(key, "INDEX%d", if_idx);
		if (RTMPGetKeyParameter(key, tmpbuf, MAX_PARAM_BUFFER_SIZE, l1profile_data, TRUE)) {
			if (strncmp(tmpbuf, chipName, strlen(chipName)) == 0) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						("%s() %s found as %s\n", __func__, chipName, key));

				if (is_dup_key(key)) {	/* There might be not only single entry for one chip */
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						("%s() %s for %s occupied, next\n", __func__, key, chipName));
				} else {
					strncpy(l1profile[get_dev_config_idx(pAd)].profile_index,
						key, L1PROFILE_INDEX_LEN - 1);
					retVal = NDIS_STATUS_SUCCESS;
					if_idx = MAX_L1PROFILE_INDEX;	/* found, intend to leave */
				}
			} else {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						("%s() %s mismatch with %s as %s\n", __func__, chipName, tmpbuf, key));
			}

			if_idx++;
		} else {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s() %s not found, dismissed.\n", __func__, key));
			if_idx = MAX_L1PROFILE_INDEX;	/* hit maximum available index, intend to leave */
		}
	}

	os_free_mem(NULL, tmpbuf);

	return retVal;
}


static NDIS_STATUS l1get_profile_index(IN RTMP_STRING *l1profile_data, IN RTMP_ADAPTER *pAd)
{
	INT retVal = NDIS_STATUS_SUCCESS;
	INT dev_idx = get_dev_config_idx(pAd);
	RTMP_STRING chipName[10] = {0};
	RTMP_STRING *tmpbuf = NULL;

	sprintf(chipName, "MT%x", pAd->ChipID);
	os_alloc_mem(NULL, (UCHAR **)&tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (match_index_by_chipname(l1profile_data, pAd, chipName) == NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s() [%d]%s found by chip\n", __func__, dev_idx, chipName));
	} else {
		retVal = NDIS_STATUS_FAILURE;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s() [%d]%s not found, keep default\n", __func__, dev_idx, chipName));
	}

	os_free_mem(NULL, tmpbuf);

	return retVal;
}


static struct l1profile_attribute_t l1profile_attributes[] = {
	{ {"profile_path"},		0,				l1set_profile_path},
	{ {"EEPROM_name"},		0,				l1set_eeprom_bin},
	{ {"EEPROM_offset"},	0,				l1set_eeprom_offset},
	{ {"EEPROM_size"},		0,				l1set_eeprom_size},
	{ {"main_ifname"},		INT_MAIN,		l1set_ifname},
#ifdef CONFIG_AP_SUPPORT
#ifdef MBSS_SUPPORT
	{ {"ext_ifname"},		INT_MBSSID,		l1set_ifname},
#endif	/* MBSS_SUPPORT */
#ifdef WDS_SUPPORT
	{ {"wds_ifname"},		INT_WDS,		l1set_ifname},
#endif	/* WDS_SUPPORT */
#endif	/* CONFIG_AP_SUPPORT */
	{ {"apcli_ifname"},		INT_APCLI,		l1set_ifname},
#ifdef CONFIG_SNIFFER_SUPPORT
	{ {"monitor_ifname"},	INT_MONITOR,	l1set_ifname},
#endif	/* monitor_ifname */

#ifdef SINGLE_SKU_V2
	{ {"single_sku_path"},		0,			l1set_single_sku_path},
#endif /* SINGLE_SKU_V2 */
};


#ifdef SINGLE_SKU_V2
UCHAR *get_single_sku_path(RTMP_ADAPTER *pAd)
{
	UCHAR *src = NULL;

	src = l1profile[get_dev_config_idx(pAd)].single_sku_path;
	return src;
}

#endif /*SINGLE_SKU_V2*/

RTMP_STRING *get_dev_eeprom_binary(VOID *pvAd)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pvAd;
	return l1profile[get_dev_config_idx(pAd)].ee_info.bin_name;
}


UINT32 get_dev_eeprom_offset(VOID *pvAd)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pvAd;

	return l1profile[get_dev_config_idx(pAd)].ee_info.offset;
}


UINT32 get_dev_eeprom_size(VOID *pvAd)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pvAd;

	return l1profile[get_dev_config_idx(pAd)].ee_info.size;
}


UCHAR *get_dev_name_prefix(RTMP_ADAPTER *pAd, INT dev_type)
{
	struct dev_type_name_map_t *map;
	INT type_idx = 0, dev_idx = get_dev_config_idx(pAd);

	if (dev_idx < 0 || dev_idx >= MAX_NUM_OF_INF) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s():  invalid dev_idx(%d)!\n",
				 __func__, dev_idx));
		return NULL;
	}

	do {
		map = &(l1profile[dev_idx].dev_name_map[type_idx]);

		if (map->type == dev_type) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("%s(): dev_idx = %d, dev_name_prefix=%s\n",  __func__, dev_idx, map->prefix));
			return map->prefix;
		}

		type_idx++;
	} while (l1profile[dev_idx].dev_name_map[type_idx].type != 0);

	return NULL;
}


UCHAR *get_dev_profile(RTMP_ADAPTER *pAd)
{
	UCHAR *src = NULL;
	{	
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
#if defined(CONFIG_RT_FIRST_CARD) || defined(CONFIG_RT_SECOND_CARD)
			if (get_dev_config_idx(pAd) < 2)
				src = l1profile[get_dev_config_idx(pAd)].profile_path;
			else
#endif /* CONFIG_RT_SECOND_CARD */
			{
				src = AP_PROFILE_PATH;
			}
		}
#endif /* CONFIG_AP_SUPPORT */

	}

#ifdef MULTIPLE_CARD_SUPPORT
	src = (RTMP_STRING *)pAd->MC_FileName;
#endif /* MULTIPLE_CARD_SUPPORT */
	if (src != NULL)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s() <--  dev profile name :%s \n", __func__, src));
	else
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s() <--  dev profile name  NULL \n", __func__));
	return src;
}


INT ShowL1profile(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s() ===== L1 profile settings =====\n", __func__));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s = %s\n", "profile_path",      l1profile[get_dev_config_idx(pAd)].profile_path));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s = %x\n", "EEPROM_offset", l1profile[get_dev_config_idx(pAd)].ee_info.offset));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s = %x\n", "EEPROM_size",   l1profile[get_dev_config_idx(pAd)].ee_info.size));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s = %s\n", "main_ifname",    l1profile[get_dev_config_idx(pAd)].dev_name_map[0].prefix));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s = %s\n", "ext_ifname",      l1profile[get_dev_config_idx(pAd)].dev_name_map[1].prefix));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s = %s\n", "wds_ifname",     l1profile[get_dev_config_idx(pAd)].dev_name_map[2].prefix));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s = %s\n", "apcli_ifname",    l1profile[get_dev_config_idx(pAd)].dev_name_map[3].prefix));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s = %s\n", "mesh_ifname",   l1profile[get_dev_config_idx(pAd)].dev_name_map[4].prefix));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s = %s\n", "p2p_ifname",     l1profile[get_dev_config_idx(pAd)].dev_name_map[5].prefix));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s = %s\n", "monitor_ifname", l1profile[get_dev_config_idx(pAd)].dev_name_map[6].prefix));
#ifdef SINGLE_SKU_V2
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
	("%s = %s\n", "single_sku_path", l1profile[get_dev_config_idx(pAd)].single_sku_path));
#else
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s = NULL \n", "single_sku_path"));
#endif /* SINGLE_SKU_V2 */
	return TRUE;
}

NDIS_STATUS load_dev_l1profile(RTMP_ADAPTER *pAd)
{
	RTMP_STRING *buffer = NULL;
	RTMP_OS_FD srcf;
	INT retval = NDIS_STATUS_SUCCESS;
	ULONG buf_size = MAX_INI_BUFFER_SIZE;
	RTMP_OS_FS_INFO		osFSInfo;
#ifdef HOSTAPD_SUPPORT
	int i;
#endif /*HOSTAPD_SUPPORT */

	DBGPRINT(RT_DEBUG_OFF, ("%s()-->\n", __func__));
	os_alloc_mem(pAd, (UCHAR **)&buffer, buf_size);
	if (!buffer) {
		DBGPRINT(DBG_LVL_OFF, ("%s() <--   alloc memory fail!\n", __func__));
		return NDIS_STATUS_FAILURE;
	}

	NdisZeroMemory(buffer, buf_size);

	RtmpOSFSInfoChange(&osFSInfo, TRUE);
	srcf = RtmpOSFileOpen(L1_PROFILE_PATH, O_RDONLY, 0);

	if (IS_FILE_OPEN_ERR(srcf)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s() Open file \"%s\" failed, try embedded default!\n", __func__, L1_PROFILE_PATH));
		goto err_out1;
	} else {

		retval = RtmpOSFileRead(srcf, buffer, buf_size - 1);

		if (retval == 0) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s() Read file \"%s\" fail! use default setting (%d)\n", __func__,
				L1_PROFILE_PATH, retval));
			goto err_out1;
		} else {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("%s() Read file \"%s\"(%s) succeed!\n", __func__, L1_PROFILE_PATH, buffer));

			if  (RtmpOSFileClose(srcf) != 0) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s() Close file \"%s\" failed(errCode=%d)!\n", __func__,
					L1_PROFILE_PATH, retval));
				goto err_out1;
			} else {

				RTMP_STRING *tmpbuf = NULL;
				RTMP_STRING key[30] = {'\0'};
				UINT32	attr_index = 0;
				INT dev_idx = get_dev_config_idx(pAd);
				RTMP_STRING *profile_index = l1profile[dev_idx].profile_index;
				struct l1profile_attribute_t *l1attr = NULL;

				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					("%s() Close file \"%s\" succeed!\n", __func__, L1_PROFILE_PATH));

				/* Do not fetch INDEX%d each time loading l1profile */
				if ((strlen(profile_index) == 0) && (l1get_profile_index(buffer, pAd)
					!= NDIS_STATUS_SUCCESS))
					goto err_out1;

				os_alloc_mem(NULL, (UCHAR **)&tmpbuf, MAX_PARAM_BUFFER_SIZE);

				if (tmpbuf == NULL)
					goto err_out1;

				if (RTMPGetKeyParameter(profile_index, tmpbuf, MAX_PARAM_BUFFER_SIZE,
					buffer, TRUE)) {
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						("%s() Chip=%s\n", __func__, tmpbuf));

					for (attr_index = 0; attr_index < ARRAY_SIZE(l1profile_attributes);
					       attr_index++) {
						l1attr = &l1profile_attributes[attr_index];
						snprintf(key, sizeof(key), "%s_%s", profile_index, l1attr->name);

						if (RTMPGetKeyParameter(key, tmpbuf, MAX_PARAM_BUFFER_SIZE,
							buffer, TRUE)) {
							MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
								("%s() %s=%s\n", __func__, l1attr->name, tmpbuf));

							if (l1attr->handler)
								l1attr->handler(pAd, l1attr->extra, tmpbuf);
							else
								MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
								("unknown handler for %s, ignored!\n", l1attr->name));
						} else {
							MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
								("%s() %s not found\n", __func__, l1attr->name));
						}
					}

					retval = NDIS_STATUS_SUCCESS;
				} else {
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						("%s() %s not found!!\n", __func__, profile_index));
					retval = NDIS_STATUS_FAILURE;
				}
				if (tmpbuf)
					os_free_mem(NULL, tmpbuf);
				if (buffer)
					os_free_mem(NULL, buffer);
				return retval;
			}
		}
	}

err_out1:
	RtmpOSFSInfoChange(&osFSInfo, FALSE);
	retval = NDIS_STATUS_FAILURE;
	if (buffer)
		os_free_mem(NULL, buffer);
	return retval;
}


INT get_dev_config_idx(RTMP_ADAPTER *pAd)
{

	INT idx = 0;
#if defined(CONFIG_RT_FIRST_CARD) && defined(CONFIG_RT_SECOND_CARD)
	INT first_card = 0, second_card = 0;

	A2Hex(first_card, FIRST_CHIP_ID);
	A2Hex(second_card, SECOND_CHIP_ID);
	DBGPRINT(RT_DEBUG_TRACE, ("chip_id1=0x%x, chip_id2=0x%x, pAd->MACVersion=0x%x\n",
		first_card, second_card, pAd->MACVersion));

	if (IS_RT8592(pAd))
		idx = 0;
	else if (IS_RT5392(pAd) || IS_MT76x0(pAd) || IS_MT76x2(pAd))
		idx = 1;

#if defined(CONFIG_RT_FIRST_IF_MT7615E)
	/* MT7615A (ra0) + MT7603(rai0) combination */
	if (IS_MT7603E(pAd))
		idx = 1;
#endif

#endif /* defined(CONFIG_RT_FIRST_CARD) && defined(CONFIG_RT_SECOND_CARD) */

	pAd->dev_idx = idx;
	return idx;
}


void RTMPPreReadParametersHook(RTMP_ADAPTER *pAd)
{


    return;
}

NDIS_STATUS	RTMPReadParametersHook(RTMP_ADAPTER *pAd)
{
	RTMP_STRING *src = NULL;
	RTMP_OS_FD srcf;
	RTMP_OS_FS_INFO osFSInfo;
	INT retval = NDIS_STATUS_FAILURE;
	ULONG buf_size = MAX_INI_BUFFER_SIZE, fsize;
	RTMP_STRING *buffer = NULL;

#ifdef HOSTAPD_SUPPORT
	int i;
#endif /*HOSTAPD_SUPPORT */

	src = get_dev_profile(pAd);
	if (src && *src)
	{
		RtmpOSFSInfoChange(&osFSInfo, TRUE);
		srcf = RtmpOSFileOpen(src, O_RDONLY, 0);
		if (IS_FILE_OPEN_ERR(srcf)) 
		{
			DBGPRINT(RT_DEBUG_ERROR, ("Open file \"%s\" failed!\n", src));
		}
		else 
		{
#ifndef OS_ABL_SUPPORT
			// TODO: need to roll back when convert into OSABL code
				 fsize = (ULONG)file_inode(srcf)->i_size;
				if (buf_size < (fsize + 1))
					buf_size = fsize + 1;
#endif /* OS_ABL_SUPPORT */
			os_alloc_mem(pAd, (UCHAR **)&buffer, buf_size);
			if (buffer) {
				memset(buffer, 0x00, buf_size);
				retval =RtmpOSFileRead(srcf, buffer, buf_size - 1);
				if (retval > 0)
				{
					RTMPSetProfileParameters(pAd, buffer);
					retval = NDIS_STATUS_SUCCESS;
				}
				else
					DBGPRINT(RT_DEBUG_ERROR, ("Read file \"%s\" failed(errCode=%d)!\n", src, retval));
				os_free_mem(NULL, buffer);
			} else 
				retval = NDIS_STATUS_FAILURE;

			if (RtmpOSFileClose(srcf) != 0)
			{
				retval = NDIS_STATUS_FAILURE;
				DBGPRINT(RT_DEBUG_ERROR, ("Close file \"%s\" failed(errCode=%d)!\n", src, retval));
			}
		}
		
		RtmpOSFSInfoChange(&osFSInfo, FALSE);
	}

#ifdef HOSTAPD_SUPPORT
	for (i = 0; i < pAd->ApCfg.BssidNum; i++)
	{
		pAd->ApCfg.MBSSID[i].Hostapd = Hostapd_Diable;
		DBGPRINT(RT_DEBUG_TRACE, ("Reset ra%d hostapd support=FLASE", i));
	}
#endif /*HOSTAPD_SUPPORT */

#ifdef SINGLE_SKU_V2
	RTMPSetSingleSKUParameters(pAd);
#endif /* SINGLE_SKU_V2 */

	return (retval);

}


void RTMP_IndicateMediaState(
	IN	PRTMP_ADAPTER		pAd,
	IN  NDIS_MEDIA_STATE	media_state)
{	
	pAd->IndicateMediaState = media_state;

#ifdef SYSTEM_LOG_SUPPORT
		if (pAd->IndicateMediaState == NdisMediaStateConnected)
		{
			RTMPSendWirelessEvent(pAd, IW_STA_LINKUP_EVENT_FLAG, pAd->MacTab.Content[BSSID_WCID].Addr, BSS0, 0);
		}
		else
		{							
			RTMPSendWirelessEvent(pAd, IW_STA_LINKDOWN_EVENT_FLAG, pAd->MacTab.Content[BSSID_WCID].Addr, BSS0, 0); 		
		}	
#endif /* SYSTEM_LOG_SUPPORT */
}


void tbtt_tasklet(unsigned long data)
{
#ifdef CONFIG_AP_SUPPORT
#ifdef WORKQUEUE_BH
	struct work_struct *work = (struct work_struct *)data;
	POS_COOKIE pObj = container_of(work, struct os_cookie, tbtt_task);
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pObj->pAd_va;
#else
		PRTMP_ADAPTER pAd = (RTMP_ADAPTER *)data;
#endif /* WORKQUEUE_BH */

#ifdef RTMP_MAC_PCI
	if (pAd->OpMode == OPMODE_AP)
	{
#ifdef AP_QLOAD_SUPPORT
		/* update channel utilization */
		QBSS_LoadUpdate(pAd, 0);
#endif /* AP_QLOAD_SUPPORT */

#ifdef DOT11K_RRM_SUPPORT
		RRM_QuietUpdata(pAd);
#endif /* DOT11K_RRM_SUPPORT */
	}
#endif /* RTMP_MAC_PCI */

	if (pAd->OpMode == OPMODE_AP)
	{
		/* step 7 - if DTIM, then move backlogged bcast/mcast frames from PSQ to TXQ whenever DtimCount==0 */
#ifdef RTMP_MAC_PCI
		/* 
			NOTE:
			This updated BEACON frame will be sent at "next" TBTT instead of at cureent TBTT. The reason is
			because ASIC already fetch the BEACON content down to TX FIFO before driver can make any
			modification. To compenstate this effect, the actual time to deilver PSQ frames will be
			at the time that we wrapping around DtimCount from 0 to DtimPeriod-1
		*/
		if (pAd->ApCfg.DtimCount == 0)
#endif /* RTMP_MAC_PCI */
		{	
#ifdef RTMP_MAC_PCI
			QUEUE_ENTRY *pEntry;	
			QUEUE_ENTRY *pTail;
			UINT count = 0;
			unsigned long IrqFlags = 0;			
#endif /* RTMP_MAC_PCI */
		
#ifndef MT_MAC
			BOOLEAN bPS = FALSE;
#endif
			
#ifdef MT_MAC
			UINT apidx = 0, deq_cnt = 0;
			
			if ((pAd->chipCap.hif_type == HIF_MT) && (pAd->MacTab.fAnyStationInPsm == TRUE))
			{
#ifdef USE_BMC
				#define MAX_BMCCNT 16
				int max_bss_cnt = 0;
				UINT mac_val = 0;

				/* BMC Flush */
				mac_val = 0x7fff0001;
                if (AsicSetBmcQCR(
				
                        pAd,
                        BMC_FLUSH,
                        CR_WRITE,
                        apidx,
                        &mac_val) == FALSE)
				
				{
					return;
				}
				
				if ((pAd->ApCfg.BssidNum == 0) || (pAd->ApCfg.BssidNum == 1))
				{
					max_bss_cnt = 0xf;
				}
				else
				{
					max_bss_cnt = MAX_BMCCNT / pAd->ApCfg.BssidNum;
				}
#endif
				for(apidx=0;apidx<pAd->ApCfg.BssidNum;apidx++)		
            	{
	                BSS_STRUCT *pMbss;
					UINT wcid = 0;
					STA_TR_ENTRY *tr_entry = NULL;
			
					pMbss = &pAd->ApCfg.MBSSID[apidx];
				
					wcid = pMbss->wdev.tr_tb_idx;
					tr_entry = &pAd->MacTab.tr_entry[wcid]; 
				
					if (tr_entry->tx_queue[QID_AC_BE].Head != NULL)
					{
#ifdef USE_BMC
						UINT bmc_cnt = 0;

                        if (AsicSetBmcQCR(pAd, BMC_CNT_UPDATE, CR_READ, apidx, &mac_val)
                            == FALSE)
                        {
                            return;
                        }
						if ((apidx >= 0) && (apidx <= 4))
						{
							if (apidx == 0)
								bmc_cnt = mac_val & 0xf;
							else 
								bmc_cnt = (mac_val >> (12+ (4*apidx))) & 0xf;
						}
						else if ((apidx >= 5) && (apidx <= 12))
						{
							bmc_cnt = (mac_val >> (4*(apidx-5))) & 0xf;
						}
						else if ((apidx >=13) && (apidx <= 15))
						{
							bmc_cnt = (mac_val >> (4*(apidx-13))) & 0xf;
						}

						
						if (bmc_cnt >= max_bss_cnt)
							deq_cnt = 0;
						else
							deq_cnt = max_bss_cnt - bmc_cnt;
						
						if (tr_entry->tx_queue[QID_AC_BE].Number <= deq_cnt)
#endif /* USE_BMC */
							deq_cnt = tr_entry->tx_queue[QID_AC_BE].Number;


#ifdef RTMP_MAC_PCI
						RTMP_IRQ_LOCK(&pAd->irq_lock, IrqFlags);
						pEntry = tr_entry->tx_queue[QID_AC_BE].Head;
						pTail = tr_entry->tx_queue[QID_AC_BE].Tail;

						/* Only last pkt no need to mark the more bit */
						if (pEntry!=NULL && (tr_entry->tx_queue[QID_AC_BE].Number >=2) && deq_cnt >=2) 
						{
							count = 1; //first pkt
							
							while((pEntry!=pTail) && (count<deq_cnt))
							{
								RTMP_SET_PACKET_MOREDATA(QUEUE_ENTRY_TO_PACKET(pEntry), TRUE);
								pEntry = pEntry->Next;
								count ++;
							}
						}
						RTMP_IRQ_UNLOCK(&pAd->irq_lock, IrqFlags);
#endif /* RTMP_MAC_PCI */


						RTMPDeQueuePacket(pAd, FALSE, QID_AC_BE, wcid, deq_cnt); 
			
						DBGPRINT(RT_DEBUG_INFO, ("%s: bss:%d, deq_cnt = %d\n", __FUNCTION__, apidx, deq_cnt));
					}
			
					if (WLAN_MR_TIM_BCMC_GET(apidx) == 0x01)
					{
						if  ( (tr_entry->tx_queue[QID_AC_BE].Head == NULL) && 
							(tr_entry->EntryType == ENTRY_CAT_MCAST))
						{
							WLAN_MR_TIM_BCMC_CLEAR(tr_entry->func_tb_idx);	/* clear MCAST/BCAST TIM bit */
							DBGPRINT(RT_DEBUG_INFO | DBG_FUNC_UAPSD, ("%s: clear MCAST/BCAST TIM bit \n", __FUNCTION__));
						}
					}
				}
#ifdef USE_BMC				
				/* BMC start */
                mac_val = 0x7fff0001;
                if (AsicSetBmcQCR(pAd, BMC_ENABLE, CR_WRITE, apidx, &mac_val)
                    == FALSE)
                {
                    return;
                }
#endif				
			}
#else /* MT_MAC */
			RTMP_IRQ_LOCK(&pAd->irq_lock, IrqFlags);
			while (pAd->MacTab.McastPsQueue.Head)
			{
				bPS = TRUE;
				if (pAd->TxSwQueue[QID_AC_BE].Number <= (pAd->TxSwQMaxLen + MAX_PACKETS_IN_MCAST_PS_QUEUE))
				{
					pEntry = RemoveHeadQueue(&pAd->MacTab.McastPsQueue);
					/*if(pAd->MacTab.McastPsQueue.Number) */
					if (count)
					{
						RTMP_SET_PACKET_MOREDATA(pEntry, TRUE);
						RTMP_SET_PACKET_TXTYPE(pEntry, TX_LEGACY_FRAME);
					}
					InsertHeadQueue(&pAd->TxSwQueue[QID_AC_BE], pEntry);
					count++;
				}
				else
				{
					break;
				}
			}
			RTMP_IRQ_UNLOCK(&pAd->irq_lock, IrqFlags);
			

			if (pAd->MacTab.McastPsQueue.Number == 0)
			{			
		                UINT bss_index;

                		/* clear MCAST/BCAST backlog bit for all BSS */
				for(bss_index=BSS0; bss_index<pAd->ApCfg.BssidNum; bss_index++)
					WLAN_MR_TIM_BCMC_CLEAR(bss_index);
			}
			pAd->MacTab.PsQIdleCount = 0;

			if (bPS == TRUE) 
			{
				// TODO: shiang-usw, modify the WCID_ALL to pMBss->tr_entry because we need to tx B/Mcast frame here!!
				RTMPDeQueuePacket(pAd, FALSE, NUM_OF_TX_RING, WCID_ALL, /*MAX_TX_IN_TBTT*/MAX_PACKETS_IN_MCAST_PS_QUEUE);
			}
#endif /* !MT_MAC */			
		}
	}
#endif /* CONFIG_AP_SUPPORT */
}

#ifdef INF_PPA_SUPPORT
static INT process_nbns_packet(
	IN PRTMP_ADAPTER 	pAd, 
	IN struct sk_buff 		*skb)
{
	UCHAR *data;
	USHORT *eth_type;

	data = (UCHAR *)eth_hdr(skb);
	if (data == 0)
	{
		data = (UCHAR *)skb->data;
		if (data == 0)
		{
			DBGPRINT(RT_DEBUG_ERROR, ("%s::Error\n", __FUNCTION__));
			return 1;
		}
	}
	
	eth_type = (USHORT *)&data[12];
	if (*eth_type == cpu_to_be16(ETH_P_IP))
	{
		INT ip_h_len;
		UCHAR *ip_h;
		UCHAR *udp_h;
		USHORT dport, host_dport;
	    
		ip_h = data + 14;
		ip_h_len = (ip_h[0] & 0x0f)*4;

		if (ip_h[9] == 0x11) /* UDP */
		{
			udp_h = ip_h + ip_h_len;
			memcpy(&dport, udp_h + 2, 2);
			host_dport = ntohs(dport);
			if ((host_dport == 67) || (host_dport == 68)) /* DHCP */
			{
				return 0;          
			}
		}
	}
    	else if ((data[12] == 0x88) && (data[13] == 0x8e)) /* EAPOL */
	{ 
		return 0;
    	}
	return 1;
}
#endif /* INF_PPA_SUPPORT */

#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)
struct net_device *rlt_dev_get_by_name(const char *name)
{
	struct net_device *pNetDev = NULL;

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
	pNetDev = dev_get_by_name(&init_net, name);
#else
	pNetDev = dev_get_by_name(name);
#endif
	if (pNetDev)
		dev_put(pNetDev);

	return pNetDev;
}


VOID ApCliLinkCoverRxPolicy(
	IN PRTMP_ADAPTER pAd,
	IN PNDIS_PACKET pPacket,
	OUT BOOLEAN *DropPacket)
{
#ifdef MAC_REPEATER_SUPPORT
	VOID *opp_band_tbl = NULL;
	VOID *band_tbl = NULL;
	INVAILD_TRIGGER_MAC_ENTRY *pInvalidEntry = NULL;
	REPEATER_CLIENT_ENTRY *pOtherBandReptEntry = NULL;
#if (MT7615_MT7603_COMBO_FORWARDING == 1)
	REPEATER_CLIENT_ENTRY *pAnotherBandReptEntry = NULL;
	VOID *other_band_tbl = NULL;
#endif
	PNDIS_PACKET pRxPkt = pPacket;
	UCHAR *pPktHdr = NULL;
	UCHAR isLinkValid;

	pPktHdr = GET_OS_PKT_DATAPTR(pRxPkt);

	if (wf_fwd_feedback_map_table)
#if (MT7615_MT7603_COMBO_FORWARDING == 1)
				wf_fwd_feedback_map_table(pAd, &band_tbl, &opp_band_tbl,&other_band_tbl);
#else
				wf_fwd_feedback_map_table(pAd, &band_tbl, &opp_band_tbl);
#endif	

#if (MT7615_MT7603_COMBO_FORWARDING == 1)
	if ((opp_band_tbl == NULL) && (other_band_tbl == NULL))
#else	
	if (opp_band_tbl == NULL)
#endif
		return;

	if (IS_GROUP_MAC(pPktHdr)) {
		pInvalidEntry = RepeaterInvaildMacLookup(pAd, pPktHdr+6);
#if (MT7615_MT7603_COMBO_FORWARDING == 1)
	if (opp_band_tbl != NULL)
#endif
		pOtherBandReptEntry = RTMPLookupRepeaterCliEntry(opp_band_tbl, FALSE, pPktHdr+6, FALSE, &isLinkValid);
#if (MT7615_MT7603_COMBO_FORWARDING == 1)
		if (other_band_tbl != NULL)
			pAnotherBandReptEntry = RTMPLookupRepeaterCliEntry(other_band_tbl, FALSE, pPktHdr+6, FALSE, &isLinkValid);
		if ((pInvalidEntry != NULL) || (pOtherBandReptEntry != NULL) || (pAnotherBandReptEntry != NULL)) {
			DBGPRINT(RT_DEBUG_INFO, ("%s, recv broadcast from InvalidRept Entry, drop this packet\n", __func__));
#else
		if ((pInvalidEntry != NULL) || (pOtherBandReptEntry != NULL)) {
			DBGPRINT(RT_DEBUG_INFO, ("%s, recv broadcast from InvalidRept Entry, drop this packet\n", __func__));
#endif
			*DropPacket = TRUE;
		}
	}
#endif
}
#endif /* CONFIG_WIFI_PKT_FWD */

void announce_802_3_packet(
	IN VOID *pAdSrc,
	IN PNDIS_PACKET pPacket,
	IN UCHAR OpMode)
{
#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pAdSrc;
#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
	PNDIS_PACKET pRxPkt = pPacket;
#if( defined(WH_EZ_SETUP) && (defined (CONFIG_WIFI_PKT_FWD) || defined (CONFIG_WIFI_PKT_FWD_MODULE)))
	BOOLEAN bypass_rx_fwd = FALSE;
#endif

	//DBGPRINT(RT_DEBUG_OFF, ("=>%s(): OpMode=%d\n", __FUNCTION__, OpMode));
#ifdef DOT11V_WNM_SUPPORT
#endif /* DOT11V_WNM_SUPPORT */
	ASSERT(pPacket);
	MEM_DBG_PKT_FREE_INC(pPacket);

#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
#ifdef MAT_SUPPORT
		if (RTMP_MATPktRxNeedConvert(pAd, RtmpOsPktNetDevGet(pRxPkt)))
		{
#if defined (CONFIG_WIFI_PKT_FWD)
#if (MT7615_MT7603_COMBO_FORWARDING == 1)
			if ((wf_fwd_needed_hook != NULL) && (wf_fwd_needed_hook() == TRUE)) {
#endif
			BOOLEAN	 need_drop = FALSE;

			ApCliLinkCoverRxPolicy(pAd, pPacket, &need_drop);
			
			if (need_drop == TRUE) {
				RELEASE_NDIS_PACKET(pAd, pRxPkt, NDIS_STATUS_FAILURE);
				return;
#if (MT7615_MT7603_COMBO_FORWARDING == 1)
		}
#endif
			}
#endif /* CONFIG_WIFI_PKT_FWD */
			RTMP_MATEngineRxHandle(pAd, pRxPkt, 0);
		}
#endif /* MAT_SUPPORT */
	}
#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */


    /* Push up the protocol stack */
#ifdef CONFIG_AP_SUPPORT
#if defined(PLATFORM_BL2348) || defined(PLATFORM_BL23570)
	{
		extern int (*pToUpperLayerPktSent)(PNDIS_PACKET *pSkb);
		RtmpOsPktProtocolAssign(pRxPkt);
		pToUpperLayerPktSent(pRxPkt);
		return;
	}
#endif /* defined(PLATFORM_BL2348) || defined(PLATFORM_BL23570) */
#endif /* CONFIG_AP_SUPPORT */

#ifdef IKANOS_VX_1X0
	{
		IKANOS_DataFrameRx(pAd, pRxPkt);
		return;
	}
#endif /* IKANOS_VX_1X0 */

#ifdef INF_PPA_SUPPORT
	{
		if (ppa_hook_directpath_send_fn && (pAd->PPAEnable == TRUE)) 
		{
			INT retVal, ret = 0;
			UINT ppa_flags = 0;
			
			retVal = process_nbns_packet(pAd, pRxPkt);
			
			if (retVal > 0)
			{
				ret = ppa_hook_directpath_send_fn(pAd->g_if_id, pRxPkt, pRxPkt->len, ppa_flags);
				if (ret == 0)
				{
					pRxPkt = NULL;
					return;
				}
				RtmpOsPktRcvHandle(pRxPkt);
			}
			else if (retVal == 0)
			{
				RtmpOsPktProtocolAssign(pRxPkt);
				RtmpOsPktRcvHandle(pRxPkt);
			}
			else
			{
				dev_kfree_skb_any(pRxPkt);
				MEM_DBG_PKT_FREE_INC(pAd);
			}
		}	
		else
		{
			RtmpOsPktProtocolAssign(pRxPkt);
			RtmpOsPktRcvHandle(pRxPkt);
		}

		return;
	}
#endif /* INF_PPA_SUPPORT */

	{
#ifdef CONFIG_RT2880_BRIDGING_ONLY
		PACKET_CB_ASSIGN(pRxPkt, 22) = 0xa8;
#endif

#if defined(CONFIG_RA_CLASSIFIER)||defined(CONFIG_RA_CLASSIFIER_MODULE)
		if(ra_classifier_hook_rx!= NULL)
		{
			unsigned int flags;
			
			RTMP_IRQ_LOCK(&pAd->page_lock, flags);
			ra_classifier_hook_rx(pRxPkt, classifier_cur_cycle);
			RTMP_IRQ_UNLOCK(&pAd->page_lock, flags);
		}
#endif /* CONFIG_RA_CLASSIFIER */


	}

	
#ifdef CONFIG_AP_SUPPORT
#ifdef BG_FT_SUPPORT
		if (BG_FTPH_PacketFromApHandle(pRxPkt) == 0)
			return;
#endif /* BG_FT_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

		RtmpOsPktProtocolAssign(pRxPkt);
		
#if defined(BB_SOC) && defined(BB_RA_HWNAT_WIFI)
		if (ra_sw_nat_hook_set_magic)
			ra_sw_nat_hook_set_magic(pRxPkt, FOE_MAGIC_WLAN);
		
		if (ra_sw_nat_hook_rx != NULL) 
		{
			if (ra_sw_nat_hook_rx(pRxPkt) == 0)
				return;
		}
#endif		

#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)
#if (MT7615_MT7603_COMBO_FORWARDING == 1)
		if ((wf_fwd_needed_hook != NULL) && (wf_fwd_needed_hook() == TRUE)) 
#endif	
		{
			struct sk_buff *pOsRxPkt = RTPKT_TO_OSPKT(pRxPkt);
#if (MT7615_MT7603_COMBO_FORWARDING != 1)
			PNET_DEV rx_dev = RtmpOsPktNetDevGet(pRxPkt);

			/* all incoming packets should set CB to mark off which net device received and in which band */
			if ((rx_dev == rlt_dev_get_by_name("rai0")) || (rx_dev == rlt_dev_get_by_name("apclii0")) ||
				(rx_dev == rlt_dev_get_by_name("rai1")) || (rx_dev == rlt_dev_get_by_name("apclii1"))) {
				RTMP_SET_PACKET_BAND(pOsRxPkt, RTMP_PACKET_SPECIFIC_5G);

				if (NdisEqualMemory(pOsRxPkt->dev->name, "apcli", 5))
					RTMP_SET_PACKET_RECV_FROM(pOsRxPkt, RTMP_PACKET_RECV_FROM_5G_CLIENT);
				else
					RTMP_SET_PACKET_RECV_FROM(pOsRxPkt, RTMP_PACKET_RECV_FROM_5G_AP);
			} else {
				RTMP_SET_PACKET_BAND(pOsRxPkt, RTMP_PACKET_SPECIFIC_2G);
			
				if (NdisEqualMemory(pOsRxPkt->dev->name, "apcli", 5))
					RTMP_SET_PACKET_RECV_FROM(pOsRxPkt, RTMP_PACKET_RECV_FROM_2G_CLIENT);
				else
					RTMP_SET_PACKET_RECV_FROM(pOsRxPkt, RTMP_PACKET_RECV_FROM_2G_AP);
			}
#else
		if (RTMP_IS_PACKET_AP_APCLI(pOsRxPkt))
		{
#endif
			if (wf_fwd_rx_hook != NULL)
			{
				struct ethhdr *mh = eth_hdr(pRxPkt);
				int ret = 0;
				
				if ((mh->h_dest[0] & 0x1) == 0x1)
				{
#if (MT7615_MT7603_COMBO_FORWARDING == 1)
					if (RTMP_IS_PACKET_APCLI(pOsRxPkt))
#else
					if (NdisEqualMemory(pOsRxPkt->dev->name, "apcli", 5)) 
#endif
					{
#ifdef MAC_REPEATER_SUPPORT
						if ((pAd->ApCfg.bMACRepeaterEn == TRUE) &&
							(RTMPQueryLookupRepeaterCliEntryMT(pAd, mh->h_source) == TRUE)) {
							RELEASE_NDIS_PACKET(pAd, pRxPkt, NDIS_STATUS_FAILURE);
							return;
						}
						else
#endif /* MAC_REPEATER_SUPPORT */
						{
							VOID *opp_band_tbl = NULL;
							VOID *band_tbl = NULL;
#if (MT7615_MT7603_COMBO_FORWARDING == 1)
							VOID *other_band_tbl = NULL;	
#endif

							if (wf_fwd_feedback_map_table)
#if (MT7615_MT7603_COMBO_FORWARDING == 1)
								wf_fwd_feedback_map_table(pAd, &band_tbl, &opp_band_tbl,&other_band_tbl);
#else
								wf_fwd_feedback_map_table(pAd, &band_tbl, &opp_band_tbl);
#endif							
#if (MT7615_MT7603_COMBO_FORWARDING == 1)
							
							if (band_tbl != NULL)
							{
								if (MAC_ADDR_EQUAL(((UCHAR *)((REPEATER_ADAPTER_DATA_TABLE *)band_tbl)->Wdev_ifAddr), mh->h_source))
									{
									//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN, 
									//    ("announce_802_3_packet: drop rx pkt by wf_fwd_feedback_map_table band_tbl check of source addr against Wdev_ifAddr\n"));
										RELEASE_NDIS_PACKET(pAd, pRxPkt, NDIS_STATUS_FAILURE);
										return;
									}
							}
							if (opp_band_tbl != NULL)
							{
								if ((MAC_ADDR_EQUAL(((UCHAR *)((REPEATER_ADAPTER_DATA_TABLE *)opp_band_tbl)->Wdev_ifAddr), mh->h_source)))
									{
									//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN, 
									//    ("announce_802_3_packet: drop rx pkt by wf_fwd_feedback_map_table opp_band_tbl check of source addr against Wdev_ifAddr\n"));
										RELEASE_NDIS_PACKET(pAd, pRxPkt, NDIS_STATUS_FAILURE);
										return;
									}
							}
							if (other_band_tbl != NULL)
							{
								if ((MAC_ADDR_EQUAL(((UCHAR *)((REPEATER_ADAPTER_DATA_TABLE *)other_band_tbl)->Wdev_ifAddr), mh->h_source)))
									{
									//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN, 
									// ("announce_802_3_packet: drop rx pkt by wf_fwd_feedback_map_table other_band_tbl check of source addr against Wdev_ifAddr\n"));
										RELEASE_NDIS_PACKET(pAd, pRxPkt, NDIS_STATUS_FAILURE);
										return;
									}
							}
#else
							if ((opp_band_tbl != NULL) 
								&& MAC_ADDR_EQUAL(((UCHAR *)((REPEATER_ADAPTER_DATA_TABLE *)opp_band_tbl)->Wdev_ifAddr), mh->h_source)) {
								RELEASE_NDIS_PACKET(pAd, pRxPkt, NDIS_STATUS_FAILURE);
								return;
							}
#endif
						}
					}
				}
#ifdef WH_EZ_SETUP
				if (IS_EZ_SETUP_ENABLED(pAd->wdev_list[pAd->CurWdevIdx]))
				{
					struct wifi_dev *wdev = pAd->wdev_list[pAd->CurWdevIdx];
				//	interface_info_t other_band_config; 		
					if (ez_need_bypass_rx_fwd(wdev)
							/*wdev->wdev_type == WDEV_TYPE_APCLI && ez_get_other_band_info(pAd,wdev, &other_band_config)
							&& !wdev->ez_security.this_band_info.shared_info.link_duplicate 
							&& !MAC_ADDR_EQUAL(other_band_config.cli_peer_ap_mac ,ZERO_MAC_ADDR)*/)
						{
							bypass_rx_fwd = TRUE;
						} else {
							bypass_rx_fwd = FALSE;
						}
				} else
					{
						bypass_rx_fwd = FALSE;
					}
					if (!bypass_rx_fwd) {
#endif		
				
				ret = wf_fwd_rx_hook(pRxPkt);

				if (ret == 0) {
					return;
				} else if (ret == 2) {
					RELEASE_NDIS_PACKET(pAd, pRxPkt, NDIS_STATUS_FAILURE);
					return;
				}
#ifdef WH_EZ_SETUP		
				}
#endif			
#if (MT7615_MT7603_COMBO_FORWARDING == 1)
			}
#endif
			}
#if (MT7615_MT7603_COMBO_FORWARDING == 1)
		else
			DBGPRINT(RT_DEBUG_TRACE, 
			("No CB Packet RTMP_IS_PACKET_AP_APCLI(%d)\n", RTMP_IS_PACKET_AP_APCLI(pOsRxPkt)));
#endif			
			
		}
#endif /* CONFIG_WIFI_PKT_FWD */
		RtmpOsPktRcvHandle(pRxPkt);
}


#ifdef CONFIG_SNIFFER_SUPPORT

INT Monitor_VirtualIF_Open(PNET_DEV dev_p)
{
	VOID *pAd;

	/* increase MODULE use count */
	RT_MOD_INC_USE_COUNT();

	pAd = RTMP_OS_NETDEV_GET_PRIV(dev_p);
	ASSERT(pAd);

	DBGPRINT(RT_DEBUG_ERROR, ("%s: ===> %s\n", __FUNCTION__, RTMP_OS_NETDEV_GET_DEVNAME(dev_p)));

	if (VIRTUAL_IF_UP(pAd) != 0)
		return -1;

	RTMP_COM_IoctlHandle(pAd, NULL, CMD_RTPRIV_IOCTL_SNIFF_OPEN, 0, dev_p, 0);

#ifdef CONFIG_RA_HW_NAT_WIFI_NEW_ARCH
	RT_MOD_HNAT_REG(dev_p);
#endif
	//Monitor_Open(pAd,dev_p);

	return 0;
}

INT Monitor_VirtualIF_Close(PNET_DEV dev_p)
{
	VOID *pAd;

	pAd = RTMP_OS_NETDEV_GET_PRIV(dev_p);
	ASSERT(pAd);

	DBGPRINT(RT_DEBUG_TRACE, ("%s: ===> %s\n", __FUNCTION__, RTMP_OS_NETDEV_GET_DEVNAME(dev_p)));

	RTMP_COM_IoctlHandle(pAd, NULL, CMD_RTPRIV_IOCTL_SNIFF_CLOSE, 0, dev_p, 0);
	//Monitor_Close(pAd,dev_p);
#ifdef CONFIG_RA_HW_NAT_WIFI_NEW_ARCH
	RT_MOD_HNAT_DEREG(dev_p);
#endif
	VIRTUAL_IF_DOWN(pAd);

	RT_MOD_DEC_USE_COUNT();

	return 0;
}


VOID RT28xx_Monitor_Init(VOID *pAd, PNET_DEV main_dev_p)
{
	RTMP_OS_NETDEV_OP_HOOK netDevOpHook;	

	/* init operation functions */
	NdisZeroMemory(&netDevOpHook, sizeof(RTMP_OS_NETDEV_OP_HOOK));
	netDevOpHook.open = Monitor_VirtualIF_Open;
	netDevOpHook.stop = Monitor_VirtualIF_Close;
	netDevOpHook.xmit = rt28xx_send_packets;
	netDevOpHook.ioctl = rt28xx_ioctl;
	DBGPRINT(RT_DEBUG_OFF, ("%s: %d !!!!####!!!!!!\n",__FUNCTION__, __LINE__)); 
	RTMP_COM_IoctlHandle(pAd, NULL, CMD_RTPRIV_IOCTL_SNIFF_INIT,	0, &netDevOpHook, 0);

}
VOID RT28xx_Monitor_Remove(VOID *pAd)
{

	RTMP_COM_IoctlHandle(pAd, NULL, CMD_RTPRIV_IOCTL_SNIFF_REMOVE, 0, NULL, 0);
	
}

void STA_MonPktSend(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
	PNET_DEV pNetDev;
	PNDIS_PACKET pRxPacket;
	PHEADER_802_11 pHeader;
	USHORT DataSize;
	UINT32 MaxRssi;
	UINT32 timestamp = 0;
	CHAR RssiForRadiotap = 0;
	UCHAR L2PAD, PHYMODE, BW, ShortGI, MCS, LDPC, LDPC_EX_SYM, AMPDU, STBC, RSSI1;
	UCHAR BssMonitorFlag11n, Channel, CentralChannel = 0;
	UCHAR *pData, *pDevName;
	UCHAR sniffer_type = pAd->sniffer_ctl.sniffer_type;
	UCHAR sideband_index = 0;

	ASSERT(pRxBlk->pRxPacket);
	
	if (pRxBlk->DataSize < 10) {
		DBGPRINT(RT_DEBUG_ERROR, ("%s : Size is too small! (%d)\n", __FUNCTION__, pRxBlk->DataSize));
		goto err_free_sk_buff;
	}

	if (sniffer_type == RADIOTAP_TYPE) {
		if (pRxBlk->DataSize + sizeof(struct mtk_radiotap_header) > RX_BUFFER_AGGRESIZE) {
			DBGPRINT(RT_DEBUG_ERROR, ("%s : Size is too large! (%d)\n", __FUNCTION__, pRxBlk->DataSize + sizeof(struct mtk_radiotap_header)));
			goto err_free_sk_buff;
		}
	}

	if (sniffer_type == PRISM_TYPE) {
		if (pRxBlk->DataSize + sizeof(wlan_ng_prism2_header) > RX_BUFFER_AGGRESIZE) {
			DBGPRINT(RT_DEBUG_ERROR, ("%s : Size is too large! (%d)\n", __FUNCTION__, pRxBlk->DataSize + sizeof(wlan_ng_prism2_header)));
			goto err_free_sk_buff;
		}
	}

	MaxRssi = RTMPMaxRssi(pAd,
				ConvertToRssi(pAd, (struct raw_rssi_info *)(&pRxBlk->rx_signal.raw_rssi[0]), RSSI_IDX_0),
				ConvertToRssi(pAd, (struct raw_rssi_info *)(&pRxBlk->rx_signal.raw_rssi[0]), RSSI_IDX_1),
				ConvertToRssi(pAd, (struct raw_rssi_info *)(&pRxBlk->rx_signal.raw_rssi[0]), RSSI_IDX_2));

	if (sniffer_type == RADIOTAP_TYPE){
		RssiForRadiotap = RTMPMaxRssi(pAd,
				ConvertToRssi(pAd, (struct raw_rssi_info *)(&pRxBlk->rx_signal.raw_rssi[0]), RSSI_IDX_0),
				ConvertToRssi(pAd, (struct raw_rssi_info *)(&pRxBlk->rx_signal.raw_rssi[0]), RSSI_IDX_1),
				ConvertToRssi(pAd, (struct raw_rssi_info *)(&pRxBlk->rx_signal.raw_rssi[0]), RSSI_IDX_2));
	}

	pNetDev = pAd->monitor_ctrl.monitor_wdev.if_dev;  //send packet to mon0
	//pNetDev = get_netdev_from_bssid(pAd, BSS0);
	pRxPacket = pRxBlk->pRxPacket;
	pHeader = pRxBlk->pHeader;
	pData = pRxBlk->pData;
	DataSize = pRxBlk->DataSize;
	L2PAD = pRxBlk->pRxInfo->L2PAD;
	PHYMODE = pRxBlk->rx_rate.field.MODE;
	BW = pRxBlk->rx_rate.field.BW;

	ShortGI = pRxBlk->rx_rate.field.ShortGI;
	MCS = pRxBlk->rx_rate.field.MCS;
	LDPC = pRxBlk->rx_rate.field.ldpc;
	if (pAd->chipCap.hif_type == HIF_RLT)
		LDPC_EX_SYM = pRxBlk->ldpc_ex_sym;
	else
		LDPC_EX_SYM = 0;
		
	AMPDU = pRxBlk->pRxInfo->AMPDU;
	STBC = pRxBlk->rx_rate.field.STBC;
	RSSI1 = pRxBlk->rx_signal.raw_rssi[1];
	//if(pRxBlk->pRxWI->RXWI_N.bbp_rxinfo[12] != 0)
#ifdef RLT_MAC
	NdisCopyMemory(&timestamp,&pRxBlk->pRxWI->RXWI_N.bbp_rxinfo[12],4);
#endif
#ifdef MT_MAC
	timestamp = pRxBlk->TimeStamp;
#endif

	BssMonitorFlag11n = 0;
#ifdef MONITOR_FLAG_11N_SNIFFER_SUPPORT
	BssMonitorFlag11n = (pAd->StaCfg.BssMonitorFlag & MONITOR_FLAG_11N_SNIFFER);
#endif /* MONITOR_FLAG_11N_SNIFFER_SUPPORT */
	pDevName = (UCHAR *)RtmpOsGetNetDevName(pAd->net_dev);
	Channel = pAd->CommonCfg.Channel;

	if (BW == BW_20)
		CentralChannel = Channel;
	else if (BW == BW_40)
	CentralChannel = pAd->CommonCfg.CentralChannel;
#ifdef DOT11_VHT_AC
	else if (BW == BW_80)
		CentralChannel = pAd->CommonCfg.vht_cent_ch; 
#endif /* DOT11_VHT_AC */

#ifdef DOT11_VHT_AC
	if (BW == BW_80) {
		sideband_index = vht_prim_ch_idx(CentralChannel, Channel);
	}
#endif /* DOT11_VHT_AC */

#ifdef SNIFFER_MIB_CMD
{
	PSNIFFER_MIB_CTRL psniffer_mib_ctrl;
	SNIFFER_MAC_CTRL *pMACEntry;
	psniffer_mib_ctrl = &pAd->ApCfg.sniffer_mib_ctrl;
	BOOLEAN  check = FALSE;
	
	RTMP_SEM_LOCK(&psniffer_mib_ctrl->MAC_ListLock);

	if (psniffer_mib_ctrl->MAC_ListNum == 0) // no set mac to sniffer, means sniffer all
	{
		check = TRUE;
		RTMP_SEM_UNLOCK(&psniffer_mib_ctrl->MAC_ListLock);
		goto report_upper;
	}
	
	DlListForEach(pMACEntry, &psniffer_mib_ctrl->MAC_List, SNIFFER_MAC_CTRL, List)
	{
		//only conpare addr1, addr2 , no addr3
		if (NdisEqualMemory(pHeader->Addr1, pMACEntry->MACAddr, MAC_ADDR_LEN) 
			|| NdisEqualMemory(pHeader->Addr2, pMACEntry->MACAddr, MAC_ADDR_LEN)
			)
		{
#ifdef ALL_NET_EVENT
			PSNIFFER_MAC_NOTIFY_T pMACEntryNotify = &pMACEntry->rNotify;
			if(pMACEntryNotify->i4RxPacketConut >= SNIFFER_MAC_MAX_RX_PACKET_COUNT)
			{
				BOOLEAN fgCancelled;
				INT32 i4AvgRssi = pMACEntryNotify->i4RssiAccum/pMACEntryNotify->i4RxPacketConut;
				UCHAR bssid[MAC_ADDR_LEN] = {0}; //report zero bssid, to prevent too many check
				wext_send_event(pNetDev,pMACEntry->MACAddr,bssid,Channel,i4AvgRssi,FBT_LINK_STA_FOUND_NOTIFY);
				
				// Clear data and reset timer for next collection.
				NdisZeroMemory(pMACEntryNotify,sizeof(*pMACEntryNotify));
				RTMPCancelTimer(&pMACEntry->rNotifyTimer, &fgCancelled);
				RTMPSetTimer(&pMACEntry->rNotifyTimer, SNIFFER_MAC_TIMEOUT);
			}
			else
			{
				RSSI_SAMPLE rRssiSample;
				NdisZeroMemory(&rRssiSample,sizeof(rRssiSample));
				rRssiSample.AvgRssi[0] = ConvertToRssi(pAd, (struct raw_rssi_info *)(&pRxBlk->rx_signal.raw_rssi[0]), RSSI_IDX_0);
				rRssiSample.AvgRssi[1] = ConvertToRssi(pAd, (struct raw_rssi_info *)(&pRxBlk->rx_signal.raw_rssi[0]), RSSI_IDX_1);
				rRssiSample.AvgRssi[2] = ConvertToRssi(pAd, (struct raw_rssi_info *)(&pRxBlk->rx_signal.raw_rssi[0]), RSSI_IDX_2);
					
				pMACEntryNotify->pNetDev = pNetDev;
				pMACEntryNotify->u4Channel = Channel;
				pMACEntryNotify->i4RssiAccum += RTMPAvgRssi(pAd,&rRssiSample);
				pMACEntryNotify->i4RxPacketConut++;
			}
#endif /* ALL_NET_EVENT */
			check = TRUE;
		}
		//DBGPRINT(RT_DEBUG_ERROR, ("%s:: MAC [%d]=%02x:%02x:%02x:%02x:%02x:%02x\n", __FUNCTION__,i++, PRINT_MAC(pMACEntry->MACAddr)));
	}
	RTMP_SEM_UNLOCK(&psniffer_mib_ctrl->MAC_ListLock);

	report_upper:
		
	if (check == FALSE)
	{
		goto err_free_sk_buff;
	}
}
#endif /* SNIFFER_MIB_CMD */

	if (sniffer_type == RADIOTAP_TYPE) {
		send_radiotap_monitor_packets(pNetDev, pRxPacket, (void *)pHeader, pData, DataSize,
									  L2PAD, PHYMODE, BW, ShortGI, MCS, LDPC, LDPC_EX_SYM, 
									  AMPDU, STBC, RSSI1, pDevName, Channel, CentralChannel,
							 		  sideband_index, RssiForRadiotap,timestamp);
	}

	if (sniffer_type == PRISM_TYPE) {
		send_prism_monitor_packets(pNetDev, pRxPacket, (void *)pHeader, pData, DataSize,
						L2PAD, PHYMODE, BW, ShortGI, MCS, AMPDU, STBC, RSSI1,
						BssMonitorFlag11n, pDevName, Channel, CentralChannel,
						MaxRssi);
	}

	return;

err_free_sk_buff:
	RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
	return;
}
#endif /* CONFIG_SNIFFER_SUPPORT */


extern NDIS_SPIN_LOCK TimerSemLock;

VOID RTMPFreeAdapter(VOID *pAdSrc)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	POS_COOKIE os_cookie;
	int index;	

	os_cookie=(POS_COOKIE)pAd->OS_Cookie;
#ifdef MULTIPLE_CARD_SUPPORT
#ifdef RTMP_FLASH_SUPPORT
	if (pAd->eebuf && (pAd->eebuf != pAd->EEPROMImage))
	{
		os_free_mem(NULL, pAd->eebuf);
		pAd->eebuf = NULL;
	}
#endif /* RTMP_FLASH_SUPPORT */
#endif /* MULTIPLE_CARD_SUPPORT */
	NdisFreeSpinLock(&pAd->MgmtRingLock);
	
#ifdef RTMP_MAC_PCI
	for (index = 0; index < NUM_OF_RX_RING; index++)
		NdisFreeSpinLock(&pAd->RxRingLock[index]);

	NdisFreeSpinLock(&pAd->McuCmdLock);
#endif /* RTMP_MAC_PCI */

#if defined(RT3290) || defined(RLT_MAC)
	NdisFreeSpinLock(&pAd->WlanEnLock);
#endif /* defined(RT3290) || defined(RLT_MAC) */

	for (index =0 ; index < NUM_OF_TX_RING; index++)
	{
		NdisFreeSpinLock(&pAd->TxSwQueueLock[index]);
		NdisFreeSpinLock(&pAd->DeQueueLock[index]);
		pAd->DeQueueRunning[index] = FALSE;
	}
	
	NdisFreeSpinLock(&pAd->irq_lock);

#ifdef RTMP_MAC_PCI
	NdisFreeSpinLock(&pAd->LockInterrupt);
#ifdef CONFIG_ANDES_SUPPORT
	NdisFreeSpinLock(&pAd->CtrlRingLock);
#endif

#ifdef MT_MAC
	NdisFreeSpinLock(&pAd->BcnRingLock);
#endif /* MT_MAC */

	NdisFreeSpinLock(&pAd->tssi_lock);
#endif /* RTMP_MAC_PCI */


#ifdef UAPSD_SUPPORT
	NdisFreeSpinLock(&pAd->UAPSDEOSPLock); /* OS_ABL_SUPPORT */
#endif /* UAPSD_SUPPORT */

#ifdef DOT11_N_SUPPORT
	NdisFreeSpinLock(&pAd->mpdu_blk_pool.lock);
#endif /* DOT11_N_SUPPORT */

	if (pAd->iw_stats)
	{
		os_free_mem(NULL, pAd->iw_stats);
		pAd->iw_stats = NULL;
	}
	if (pAd->stats)
	{
		os_free_mem(NULL, pAd->stats);
		pAd->stats = NULL;
	}

	NdisFreeSpinLock(&TimerSemLock);

#ifdef MT_MAC
#ifdef CONFIG_AP_SUPPORT
    if ((pAd->chipCap.hif_type == HIF_MT) && (pAd->OpMode == OPMODE_AP))
	{
        BSS_STRUCT *pMbss;
		pMbss = &pAd->ApCfg.MBSSID[MAIN_MBSSID];
        ASSERT(pMbss);
		if (pMbss) {
			wdev_bcn_buf_deinit(pAd, &pMbss->bcn_buf);
		} else {
			DBGPRINT(RT_DEBUG_ERROR, ("%s():func_dev is NULL!\n", __FUNCTION__));
			return;
		}
	}
#endif
#endif
#ifdef CONFIG_ATE
#endif /* CONFIG_ATE */

	RTMP_OS_FREE_TIMER(pAd);
	RTMP_OS_FREE_LOCK(pAd);
	RTMP_OS_FREE_TASKLET(pAd);
	RTMP_OS_FREE_TASK(pAd);
	RTMP_OS_FREE_SEM(pAd);
	RTMP_OS_FREE_ATOMIC(pAd);

	RtmpOsVfree(pAd); /* pci_free_consistent(os_cookie->pci_dev,sizeof(RTMP_ADAPTER),pAd,os_cookie->pAd_pa); */
	if (os_cookie)
		os_free_mem(NULL, os_cookie);
}




int RTMPSendPackets(
	IN NDIS_HANDLE dev_hnd,
	IN PPNDIS_PACKET pkt_list,
	IN UINT pkt_cnt,
	IN UINT32 pkt_total_len,
	IN RTMP_NET_ETH_CONVERT_DEV_SEARCH Func)
{
	struct wifi_dev *wdev = (struct wifi_dev *)dev_hnd;
	RTMP_ADAPTER *pAd;
	PNDIS_PACKET pPacket = pkt_list[0];

	ASSERT(wdev->sys_handle);
	pAd = (RTMP_ADAPTER *)wdev->sys_handle;

	if(pAd == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s(): pAd is NULL!\n", __FUNCTION__));
	    return 0;
	}
	
	INC_COUNTER64(pAd->WlanCounters.TransmitCountFrmOs);

	if (!pPacket)
		return 0;

#ifdef WSC_NFC_SUPPORT
        {
                struct sk_buff *pRxPkt = RTPKT_TO_OSPKT(pPacket);
                USHORT protocol = 0;
                protocol = ntohs(pRxPkt->protocol);
                if (protocol == 0x6605)
                {
                        NfcParseRspCommand(pAd, pRxPkt->data, pRxPkt->len);
                        RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
                        return 0;
                }
                else
                {
                        printk("%04x\n", protocol);
                }
        }
#endif /* WSC_NFC_SUPPORT */


	if (pkt_total_len < 14)
	{
		hex_dump("bad packet", GET_OS_PKT_DATAPTR(pPacket), pkt_total_len);
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
		return 0;
	}

#ifdef CONFIG_ATE
	// TODO: shiang-usw, can remove this?
	if (ATE_ON(pAd))
	{
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
		return 0;
	}
#endif /* CONFIG_ATE */


#ifdef CONFIG_5VT_ENHANCE
	RTMP_SET_PACKET_5VT(pPacket, 0);
	if (*(int*)(GET_OS_PKT_CB(pPacket)) == BRIDGE_TAG) {
		RTMP_SET_PACKET_5VT(pPacket, 1);
	}
#endif /* CONFIG_5VT_ENHANCE */

#ifdef WH_EZ_SETUP
	if(IS_EZ_SETUP_ENABLED(wdev)
#ifdef EZ_API_SUPPORT	
	 && (wdev->ez_driver_params.ez_api_mode != CONNECTION_OFFLOAD) 
#endif	 
	 && (wdev->wdev_type == WDEV_TYPE_STA) )
	{
		if (ez_handle_send_packets(wdev, pPacket) == 0)
			return 0;
	}
#endif

#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)
#if (MT7615_MT7603_COMBO_FORWARDING == 1)
	if ((wf_fwd_needed_hook != NULL) && (wf_fwd_needed_hook() == TRUE)) {
#endif
	if (wf_fwd_tx_hook != NULL)
	{
		if (wf_fwd_tx_hook(pPacket) == 1)
		{
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
			return 0;
		}
	}
#if (MT7615_MT7603_COMBO_FORWARDING == 1)
    }
#endif
#endif /* CONFIG_WIFI_PKT_FWD */

	return wdev_tx_pkts((NDIS_HANDLE)pAd, (PPNDIS_PACKET) &pPacket, 1, wdev);
}


#ifdef CONFIG_AP_SUPPORT
/*
========================================================================
Routine Description:
	Driver pre-Ioctl for AP.

Arguments:
	pAdSrc			- WLAN control block pointer
	pCB				- the IOCTL parameters

Return Value:
	NDIS_STATUS_SUCCESS	- IOCTL OK
	Otherwise			- IOCTL fail

Note:
========================================================================
*/
INT RTMP_AP_IoctlPrepare(RTMP_ADAPTER *pAd, VOID *pCB)
{
	RT_CMD_AP_IOCTL_CONFIG *pConfig = (RT_CMD_AP_IOCTL_CONFIG *)pCB;
	POS_COOKIE pObj;
	USHORT index;
	INT	Status = NDIS_STATUS_SUCCESS;
#ifdef CONFIG_APSTA_MIXED_SUPPORT
	INT cmd = 0xff;
#endif /* CONFIG_APSTA_MIXED_SUPPORT */


	pObj = (POS_COOKIE) pAd->OS_Cookie;
	//
	// Check if radio is OFF then return from here.
	//
	if(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
	{
		if (pConfig->pCmdData == NULL)
			return Status;

		return -ENETDOWN;
    }	

    /* determine this ioctl command is comming from which interface. */
    if (pConfig->priv_flags == INT_MAIN)
    {
		pObj->ioctl_if_type = INT_MAIN;
		pObj->ioctl_if = MAIN_MBSSID;
		
    }
    else if (pConfig->priv_flags == INT_MBSSID)
    {
		pObj->ioctl_if_type = INT_MBSSID;
/*    	if (!RTMPEqualMemory(net_dev->name, pAd->net_dev->name, 3))  // for multi-physical card, no MBSSID */
		if (strcmp(pConfig->name, RtmpOsGetNetDevName(pAd->net_dev)) != 0) /* sample */
    	{
	        for (index = 1; index < pAd->ApCfg.BssidNum; index++)
	    	{
	    	    if (pAd->ApCfg.MBSSID[index].wdev.if_dev == pConfig->net_dev)
	    	    {
	    	        pObj->ioctl_if = index;
	    	        break;
	    	    }
	    	}
	    	
	        /* Interface not found! */
	        if(index == pAd->ApCfg.BssidNum)
	            return -ENETDOWN;
            
	    }
	    else    /* ioctl command from I/F(ra0) */
	    {
    	    pObj->ioctl_if = MAIN_MBSSID;
	    }
        MBSS_MR_APIDX_SANITY_CHECK(pAd, pObj->ioctl_if);
    }
#ifdef WDS_SUPPORT
	else if (pConfig->priv_flags == INT_WDS)
	{
		pObj->ioctl_if_type = INT_WDS;
		for(index = 0; index < MAX_WDS_ENTRY; index++)
		{
			if (pAd->WdsTab.WdsEntry[index].wdev.if_dev == pConfig->net_dev)
			{
				pObj->ioctl_if = index;
				break;
			}
			
			if(index == MAX_WDS_ENTRY)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("%s(): can not find wds I/F\n", __FUNCTION__));
				return -ENETDOWN;
			}
		}
	}
#endif /* WDS_SUPPORT */
#ifdef APCLI_SUPPORT
	else if (pConfig->priv_flags == INT_APCLI)
	{
		pObj->ioctl_if_type = INT_APCLI;
		for (index = 0; index < MAX_APCLI_NUM; index++)
		{
			if (pAd->ApCfg.ApCliTab[index].wdev.if_dev == pConfig->net_dev)
			{
				pObj->ioctl_if = index;
				break;
			}

			if(index == MAX_APCLI_NUM)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("%s(): can not find Apcli I/F\n", __FUNCTION__));
				return -ENETDOWN;
			}
		}
		APCLI_MR_APIDX_SANITY_CHECK(pObj->ioctl_if);
	}
#endif /* APCLI_SUPPORT */
    else
    {
	/* DBGPRINT(RT_DEBUG_WARN, ("IOCTL is not supported in WDS interface\n")); */	
    	return -EOPNOTSUPP;
    }

	pConfig->apidx = pObj->ioctl_if;
	
	return Status;
}


VOID AP_E2PROM_IOCTL_PostCtrl(
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN	RTMP_STRING *msg)
{
	wrq->u.data.length = strlen(msg);
	if (copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s: copy_to_user() fail\n", __FUNCTION__));			
	}
}


VOID IAPP_L2_UpdatePostCtrl(RTMP_ADAPTER *pAd, UINT8 *mac_p, INT wdev_idx)
{
}
#endif /* CONFIG_AP_SUPPORT */


#ifdef WDS_SUPPORT
VOID AP_WDS_KeyNameMakeUp(
	IN	RTMP_STRING *pKey,
	IN	UINT32						KeyMaxSize,
	IN	INT							KeyId)
{
	snprintf(pKey, KeyMaxSize, "Wds%dKey", KeyId);
}
#endif /* WDS_SUPPORT */

