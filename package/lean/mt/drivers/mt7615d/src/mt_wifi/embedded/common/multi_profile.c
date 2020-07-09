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
	multi_profile.c
*/

#ifdef MULTI_PROFILE
#include "rt_config.h"

/*Local definition*/
#define FIRST_AP_2G_PROFILE_PATH	"/etc/Wireless/RT2860/RT2860_2G.dat"
#define FIRST_AP_5G_PROFILE_PATH	"/etc/Wireless/RT2860/RT2860_5G.dat"
#define FIRST_AP_MERGE_PROFILE_PATH ""
#if defined(BB_SOC) && !defined(MULTI_INF_SUPPORT)
#define FIRST_AP_5G_DEVNAME "rai0"
#define FIRST_MBSSID_5G_DEVNAME "rai"
#define FIRST_APCLI_5G_DEVNAME "apclii"
#else
#define FIRST_AP_5G_DEVNAME "rax0"
#define FIRST_MBSSID_5G_DEVNAME "rax"
#define FIRST_APCLI_5G_DEVNAME "apclix"
#endif
#define SECOND_AP_2G_PROFILE_PATH	"/etc/Wireless/iNIC/iNIC_ap_2G.dat"
#define SECOND_AP_5G_PROFILE_PATH	"/etc/Wireless/iNIC/iNIC_ap_5G.dat"
#define SECOND_AP_MERGE_PROFILE_PATH ""
#if defined(BB_SOC) && !defined(MULTI_INF_SUPPORT)
#define SECOND_AP_5G_DEVNAME "ra0"
#define SECOND_MBSSID_5G_DEVNAME "ra"
#define SECOND_APCLI_5G_DEVNAME "apcli"
#else
#define SECOND_AP_5G_DEVNAME "ray0"
#define SECOND_MBSSID_5G_DEVNAME "ray"
#define SECOND_APCLI_5G_DEVNAME "apcliiy"
#endif
#define THIRD_AP_2G_PROFILE_PATH	"/etc/Wireless/WIFI3/RT2870AP_2G.dat"
#define THIRD_AP_5G_PROFILE_PATH	"/etc/Wireless/WIFI3/RT2870AP_5G.dat"
#define THIRD_AP_MERGE_PROFILE_PATH ""
#define THIRD_AP_5G_DEVNAME "raz0"
#define THIRD_MBSSID_5G_DEVNAME "raz"
#define THIRD_APCLI_5G_DEVNAME "apcliez"


#define TEMP_STR_SIZE 128
/* A:B:C:D -> 4 (Channel group A, B, C, D) + 3 ':' */
#define LEN_BITMAP_CHGRP 7

struct mpf_data {
	UCHAR enable;
	UCHAR specific_dname;
	UCHAR pf1_num;
	UCHAR pf2_num;
	UCHAR total_num;
};

struct mpf_table {
	UCHAR profile_2g[L2PROFILE_PATH_LEN];
	UCHAR profile_5g[L2PROFILE_PATH_LEN];
	UCHAR merge[L2PROFILE_PATH_LEN];
	struct dev_type_name_map_t dev_name_map[MAX_INT_TYPES+1];
};

static struct mpf_table mtb[] = {
	{ FIRST_AP_2G_PROFILE_PATH, FIRST_AP_5G_PROFILE_PATH, FIRST_AP_MERGE_PROFILE_PATH,
		{{INT_MAIN, FIRST_AP_5G_DEVNAME}, {INT_MBSSID, FIRST_MBSSID_5G_DEVNAME},
			{INT_WDS, FIRST_AP_5G_DEVNAME}, {INT_APCLI, FIRST_APCLI_5G_DEVNAME},
			{INT_MESH, FIRST_AP_5G_DEVNAME}, {INT_P2P, FIRST_AP_5G_DEVNAME},
			{INT_MONITOR, FIRST_AP_5G_DEVNAME}, {INT_MSTA, FIRST_AP_5G_DEVNAME}, {0} } },
	{ SECOND_AP_2G_PROFILE_PATH, SECOND_AP_5G_PROFILE_PATH, SECOND_AP_MERGE_PROFILE_PATH,
		{{INT_MAIN, SECOND_AP_5G_DEVNAME}, {INT_MBSSID, SECOND_MBSSID_5G_DEVNAME},
			{INT_WDS, SECOND_AP_5G_DEVNAME}, {INT_APCLI, SECOND_APCLI_5G_DEVNAME},
			{INT_MESH, SECOND_AP_5G_DEVNAME}, {INT_P2P, SECOND_AP_5G_DEVNAME},
			{INT_MONITOR, SECOND_AP_5G_DEVNAME}, {INT_MSTA, SECOND_AP_5G_DEVNAME}, {0} } },
	{ THIRD_AP_2G_PROFILE_PATH, THIRD_AP_5G_PROFILE_PATH, THIRD_AP_MERGE_PROFILE_PATH,
		{{INT_MAIN, THIRD_AP_5G_DEVNAME}, {INT_MBSSID, THIRD_MBSSID_5G_DEVNAME},
			{INT_WDS, THIRD_AP_5G_DEVNAME}, {INT_APCLI, THIRD_APCLI_5G_DEVNAME},
			{INT_MESH, THIRD_AP_5G_DEVNAME}, {INT_P2P, THIRD_AP_5G_DEVNAME},
			{INT_MONITOR, THIRD_AP_5G_DEVNAME}, {INT_MSTA, THIRD_AP_5G_DEVNAME}, {0} } },
};

#ifdef CONFIG_AP_SUPPORT
#ifdef MBSS_SUPPORT
static INT multi_profile_merge_wsc(
	struct mpf_data *data,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final);
#endif /*MBSS_SUPPORT*/
#endif /*CONFIG_AP_SUPPORT*/


static UCHAR *get_dbdcdev_name_prefix(RTMP_ADAPTER *pAd, INT dev_type)
{
	struct dev_type_name_map_t *map;
	INT type_idx = 0, dev_idx = get_dev_config_idx(pAd);

	if (dev_idx < 0 || dev_idx >= MAX_NUM_OF_INF) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): invalid dev_idx(%d)!\n",
				 __func__, dev_idx));
		return NULL;
	}

	do {
		map = &(mtb[dev_idx].dev_name_map[type_idx]);

		if (map->type == dev_type) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("%s(): dev_idx = %d, dev_name_prefix=%s\n", __func__, dev_idx, map->prefix));
			return map->prefix;
		}

		type_idx++;
	} while (mtb[dev_idx].dev_name_map[type_idx].type != 0);

	return NULL;
}


NDIS_STATUS update_mtb_value(struct _RTMP_ADAPTER *pAd, UCHAR profile_id, UINT_32 extra, RTMP_STRING *value)
{
	INT retVal = NDIS_STATUS_SUCCESS;
	INT dev_idx = get_dev_config_idx(pAd);

	switch (profile_id) {
	case MTB_2G_PROFILE:
		if (strcmp(value, mtb[dev_idx].profile_2g)) {
			strncpy(mtb[dev_idx].profile_2g, value, L2PROFILE_PATH_LEN - 1);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					("mtb[%d].profile_2g updated as %s!\n", dev_idx, mtb[dev_idx].profile_2g));
		} else {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					("mtb[%d].profile_2g remain %s!\n", dev_idx, mtb[dev_idx].profile_2g));
		}
		break;
	case MTB_5G_PROFILE:
		if (strcmp(value, mtb[dev_idx].profile_5g)) {
			strncpy(mtb[dev_idx].profile_5g, value, L2PROFILE_PATH_LEN - 1);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					("mtb[%d].profile_5g updated as %s!\n", dev_idx, mtb[dev_idx].profile_5g));
		} else {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					("mtb[%d].profile_2g remain %s!\n", dev_idx, mtb[dev_idx].profile_2g));
		}
		break;
	case MTB_DEV_PREFIX:
		{
			RTMP_STRING *prefix = get_dbdcdev_name_prefix(pAd, extra);
			if (prefix == NULL) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s: MTB_DEV_PREFIX, get_dbdcdev_name_prefix is NULL\n", __func__));
				return NDIS_STATUS_FAILURE;
			}

			if (strcmp(prefix, value)) {
				strncpy(prefix, value, IFNAMSIZ);
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						("mtb[%d].prefix updated as %s!\n", dev_idx, prefix));
			} else {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						("mtb[%d].prefix remain %s!\n", dev_idx, prefix));
			}
		}
		break;
	default:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Uknown profile_id(%d)\n", profile_id));
		break;
	}

	return retVal;
}

static UINT32 count_depth(UCHAR *path)
{
	UINT32 length = 0;
	UCHAR *slash = strstr(path, "/");

	if (slash) {
		length = slash - path;
		length += (count_depth(slash+1)+1);
	}

	return length;
}

/*Local function body*/
/*
*
*/
static UCHAR *multi_profile_fname_get(struct _RTMP_ADAPTER *pAd, UCHAR profile_id)
{
	UCHAR *src = NULL;
	INT card_idx = 0;
#if defined(CONFIG_RT_FIRST_CARD) || defined(CONFIG_RT_SECOND_CARD) || defined(CONFIG_RT_THIRD_CARD)
	card_idx = get_dev_config_idx(pAd);
#endif /* CONFIG_RT_FIRST_CARD || CONFIG_RT_SECOND_CARD */

	if (profile_id == MTB_MERGE_PROFILE) {
		src = mtb[card_idx].merge;

		if (strlen(src) == 0) {
			strncat(src, mtb[card_idx].profile_2g, count_depth(mtb[card_idx].profile_2g));
			snprintf(src, L2PROFILE_PATH_LEN, "%sDBDC_card%d.dat", src, card_idx);
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("Open file \"%s\" to store DBDC cfg! (%d)\n",
			 src, count_depth(mtb[card_idx].profile_2g)));
	} else
		src = (profile_id == MTB_2G_PROFILE) ? mtb[card_idx].profile_2g : mtb[card_idx].profile_5g;

	return src;
}

/*
* open & read profile
*/
static INT multi_profile_read(CHAR *fname, CHAR *buf)
{
	INT retval = NDIS_STATUS_FAILURE;
	RTMP_OS_FD_EXT srcf;

	if (!fname)
		return retval;

	srcf = os_file_open(fname, O_RDONLY, 0);

	if (srcf.Status) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Open file \"%s\" failed!\n", fname));
		return retval;
	}

	if (buf) {
		os_zero_mem(buf, MAX_INI_BUFFER_SIZE);
		retval = os_file_read(srcf, buf, MAX_INI_BUFFER_SIZE - 1);

		if (retval > 0)
			retval = NDIS_STATUS_SUCCESS;
		else
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Read file \"%s\" failed(errCode=%d)!\n", fname, retval));
	} else
		retval = NDIS_STATUS_FAILURE;

	if (os_file_close(srcf) != 0) {
		retval = NDIS_STATUS_FAILURE;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Close file \"%s\" failed(errCode=%d)!\n", fname, retval));
	}

	return retval;
}

/*
* write merge profile for check
*/
static INT multi_profile_write(CHAR *fname, CHAR *buf)
{
	INT retval = NDIS_STATUS_FAILURE;
	RTMP_OS_FD_EXT srcf;

	if (!fname)
		return retval;

	srcf = os_file_open(fname, O_WRONLY | O_CREAT, 0);

	if (srcf.Status) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Open file \"%s\" failed!\n", fname));
		return retval;
	}

	if (buf) {
		retval = os_file_write(srcf, buf, strlen(buf));

		if (retval > 0)
			retval = NDIS_STATUS_SUCCESS;
		else
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Write file \"%s\" failed(errCode=%d)!\n", fname, retval));
	} else
		retval = NDIS_STATUS_FAILURE;

	if (os_file_close(srcf) != 0) {
		retval = NDIS_STATUS_FAILURE;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Close file \"%s\" failed(errCode=%d)!\n", fname, retval));
	}

	return retval;
}

/*
* replace function
*/
INT multi_profile_replace(CHAR *cha, CHAR *rep, CHAR *value)
{
	CHAR *token = NULL;

	while ((token = strstr(value, cha)) != NULL)
		strncpy(token, rep, strlen(rep));

	return NDIS_STATUS_SUCCESS;
}

/*
* Separate
*/
static INT multi_profile_merge_separate(
	UCHAR *parm,
	UCHAR *buf1,
	UCHAR *buf2,
	UCHAR *final)
{
	CHAR tmpbuf[TEMP_STR_SIZE] = "";
	CHAR tmpbuf2[TEMP_STR_SIZE] = "";
	CHAR value[TEMP_STR_SIZE] = "";

	if (!buf1 || !buf2)
		return NDIS_STATUS_FAILURE;

	if (RTMPGetKeyParameter(parm, tmpbuf, TEMP_STR_SIZE, buf1, TRUE) != TRUE)
		return NDIS_STATUS_SUCCESS;

	if (RTMPGetKeyParameter(parm, tmpbuf2, TEMP_STR_SIZE, buf2, TRUE) != TRUE)
		return NDIS_STATUS_SUCCESS;

	snprintf(value, sizeof(value), "%s;%s", tmpbuf, tmpbuf2);
	RTMPSetKeyParameter(parm, value, TEMP_STR_SIZE, final, TRUE);
	return NDIS_STATUS_SUCCESS;
}

/*
*perband
*/
static INT multi_profile_merge_perband(
	struct mpf_data *data,
	UCHAR *parm,
	UCHAR *buf1,
	UCHAR *buf2,
	UCHAR *final)
{
	CHAR tmpbuf1[TEMP_STR_SIZE] = "";
	CHAR tmpbuf2[TEMP_STR_SIZE] = "";
	CHAR value[TEMP_STR_SIZE] = "";
	UCHAR i, j;
	CHAR *tmpbuf;
	CHAR *macptr;

	if (!buf1 || !buf2)
		return NDIS_STATUS_FAILURE;

	if (RTMPGetKeyParameter(parm, tmpbuf1, TEMP_STR_SIZE, buf1, TRUE) != TRUE)
		return NDIS_STATUS_SUCCESS;

	if (RTMPGetKeyParameter(parm, tmpbuf2, TEMP_STR_SIZE, buf2, TRUE) != TRUE)
		return NDIS_STATUS_SUCCESS;

	os_zero_mem(value, sizeof(value));

	/*check number of perband parameter mode*/
	for (i = 0, macptr = rstrtok(tmpbuf1, ";"); macptr; macptr = rstrtok(NULL, ";"), i++)
		/*do nothing*/

	for (j = 0, macptr = rstrtok(tmpbuf2, ";"); macptr; macptr = rstrtok(NULL, ";"), j++)
		/*do nothing*/

	if (i > 1 || j > 1) {
		multi_profile_merge_separate(parm, buf1, buf2, final);
	} else {
		for (i = 0; i < data->total_num; i++) {
			tmpbuf = (i < data->pf1_num) ? tmpbuf1 : tmpbuf2;
			snprintf(value, sizeof(value), "%s%s;", value, tmpbuf);
		}
		RTMPSetKeyParameter(parm, value, TEMP_STR_SIZE, final, TRUE);
	}

	return NDIS_STATUS_SUCCESS;
}

#ifdef CONFIG_AP_SUPPORT
#ifdef MBSS_SUPPORT
/*
* MACAddress(Idx)
*/
static INT multi_profile_merge_mac_address(
	struct mpf_data *mpf,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	CHAR tmpbuf[25] = "";
	CHAR tok_str[25] = "";
	UCHAR i = 0;
	UCHAR j = 0;

	/* set file parameter to portcfg*/
	if (RTMPGetKeyParameter("MacAddress", tmpbuf, 25, buf2, TRUE)) {
		snprintf(tok_str, sizeof(tok_str), "MacAddress%d", mpf->pf1_num);
		RTMPAddKeyParameter(tok_str, tmpbuf, 25, final);
	}

	for (i = 1; i <= mpf->pf2_num ; i++) {
		snprintf(tok_str, sizeof(tok_str), "MacAddress%d", i);

		if (RTMPGetKeyParameter(tok_str, tmpbuf, 25, buf2, TRUE)) {
			j = i + mpf->pf1_num;
			snprintf(tok_str, sizeof(tok_str), "MacAddress%d", j);
			RTMPAddKeyParameter(tok_str, tmpbuf, 25, final);
		}
	}

	return NDIS_STATUS_SUCCESS;
}

/*
* For increase from 1
*/
static INT multi_profile_merge_increase(
	struct mpf_data *mpf,
	UCHAR start_idx,
	CHAR *parm,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	CHAR tmpbuf[TEMP_STR_SIZE] = "";
	CHAR tok_str[25] = "";
	UCHAR i = 0;
	UCHAR j = 0;
	UCHAR k = 0;

	/* set file parameter to portcfg*/
	for (i = 0; i < mpf->pf2_num; i++) {
		k = start_idx + i;
		snprintf(tok_str, sizeof(tok_str), "%s%d", parm, k);

		if (RTMPGetKeyParameter(tok_str, tmpbuf, TEMP_STR_SIZE, buf2, FALSE)) {
			j = k + mpf->pf1_num;
			snprintf(tok_str, sizeof(tok_str), "%s%d", parm, j);
			RTMPSetKeyParameter(tok_str, tmpbuf, TEMP_STR_SIZE, final, TRUE);
		}
	}

	return NDIS_STATUS_SUCCESS;
}

/*
* Key%dType  & Key%dStr%
*/
static INT multi_profile_merge_keytype(
	struct mpf_data *mpf,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	CHAR tmpbuf[TEMP_STR_SIZE] = "";
	CHAR tok_str[25] = "";
	UCHAR i = 0;
	UCHAR j = 0;
	UCHAR k = 0;

	for (k = 1; k <= 4; k++) {
		snprintf(tok_str, sizeof(tok_str), "Key%dType", k);
		multi_profile_merge_separate(tok_str, buf1, buf2, final);
	}

	/* set file parameter to keytype*/
	for (i = 1; i <= mpf->pf2_num; i++) {
		j = i + mpf->pf1_num;

		for (k = 1; k <= 4; k++) {
			snprintf(tok_str, sizeof(tok_str), "Key%dStr%d", k, i);

			if (RTMPGetKeyParameter(tok_str, tmpbuf, TEMP_STR_SIZE, buf2, TRUE)) {
				snprintf(tok_str, sizeof(tok_str), "Key%dStr%d", k, j);
				RTMPSetKeyParameter(tok_str, tmpbuf, TEMP_STR_SIZE, final, TRUE);
			}
		}
	}

	return NDIS_STATUS_SUCCESS;
}

/*
* Security
*/
static INT multi_profile_merge_security(
	struct mpf_data *mpf,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	/*IEEE8021X*/
	multi_profile_merge_separate("IEEE8021X", buf1, buf2, final);
	/*PreAuth*/
	multi_profile_merge_separate("PreAuth", buf1, buf2, final);
	/*AuthMode*/
	multi_profile_merge_separate("AuthMode", buf1, buf2, final);
	/*EncrypType*/
	multi_profile_merge_separate("EncrypType", buf1, buf2, final);
	/*RekeyMethod*/
	multi_profile_merge_separate("RekeyMethod", buf1, buf2, final);
	/*RekeyInterval*/
	multi_profile_merge_separate("RekeyInterval", buf1, buf2, final);
	/*PMKCachePeriod*/
	multi_profile_merge_separate("PMKCachePeriod", buf1, buf2, final);
	/*WPAPSK*/
	multi_profile_merge_increase(mpf, 1, "WPAPSK", buf1, buf2, final);
	/*DefaultKeyID*/
	multi_profile_merge_separate("DefaultKeyID", buf1, buf2, final);
	/*KeyType & KeyStr*/
	multi_profile_merge_keytype(mpf, buf1, buf2, final);
	/*AccessPolicy */
	multi_profile_merge_increase(mpf, 0, "AccessPolicy", buf1, buf2, final);
	/*AccessControlList*/
	multi_profile_merge_increase(mpf, 0, "AccessControlList", buf1, buf2, final);
	/*RADIUS_Server*/
	multi_profile_merge_separate("RADIUS_Server", buf1, buf2, final);
	/*RADIUS_Port*/
	multi_profile_merge_separate("RADIUS_Port", buf1, buf2, final);
	/*RADIUS_Key*/
	multi_profile_merge_separate("RADIUS_Key", buf1, buf2, final);
	/*RADIUS Key%d*/
	multi_profile_merge_increase(mpf, 1, "RADIUS_Key", buf1, buf2, final);
	/*EAPifname*/
	multi_profile_merge_separate("EAPifname", buf1, buf2, final);
	/*PreAuthifname*/
	multi_profile_merge_separate("PreAuthifname", buf1, buf2, final);
	/*PMFMFPC*/
	multi_profile_merge_separate("PMFMFPC", buf1, buf2, final);
	/*PMFMFPR*/
	multi_profile_merge_separate("PMFMFPR", buf1, buf2, final);
	/*PMFSHA256*/
	multi_profile_merge_separate("PMFSHA256", buf1, buf2, final);
	return NDIS_STATUS_SUCCESS;
}

/*
*
*/
/* TODO: related to multi_profile_merge_apedca */
static INT multi_profile_merge_default_edca(
	struct mpf_data *data,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	UCHAR i;
	CHAR aifs[32] = "";
	CHAR cwmin[32] = "";
	CHAR cwmax[32] = "";
	CHAR txop[32] = "";
	CHAR acm[32] = "";
	CHAR *buf = NULL;
	CHAR value[256] = "";
	UCHAR idx;
	CHAR tok_str[25] = "";

	for (i = 0; i < 2; i++) {
		buf = (i == 0) ? buf1 : buf2;
		os_zero_mem(value, sizeof(value));

		/*APAifsn*/
		if (RTMPGetKeyParameter("APAifsn", aifs, sizeof(aifs), buf, FALSE) != TRUE)
			return NDIS_STATUS_FAILURE;

		multi_profile_replace(";", ",", aifs);

		/*APCwmin*/
		if (RTMPGetKeyParameter("APCwmin", cwmin, sizeof(cwmin), buf, FALSE) != TRUE)
			return NDIS_STATUS_FAILURE;

		multi_profile_replace(";", ",", cwmin);

		/*APCwmax*/
		if (RTMPGetKeyParameter("APCwmax", cwmax, sizeof(cwmax), buf, FALSE) != TRUE)
			return NDIS_STATUS_FAILURE;

		multi_profile_replace(";", ",", cwmax);

		/*APTxop*/
		if (RTMPGetKeyParameter("APTxop", txop, sizeof(txop), buf, FALSE) != TRUE)
			return NDIS_STATUS_FAILURE;

		multi_profile_replace(";", ",", txop);

		/*APACM*/
		if (RTMPGetKeyParameter("APACM", acm, sizeof(acm), buf, FALSE) != TRUE)
			return NDIS_STATUS_FAILURE;

		multi_profile_replace(";", ",", acm);
		/*merge*/
		snprintf(value, sizeof(value), "1;%s;%s;%s;%s;%s", aifs, cwmin, cwmax, txop, acm);
		/*set*/
		snprintf(tok_str, sizeof(tok_str), "APEdca%d", i);
		RTMPSetKeyParameter(tok_str, value, sizeof(value), final, TRUE);
	}

	os_zero_mem(value, sizeof(value));

	for (i = 0; i < data->total_num; i++) {
		idx = (i < data->pf1_num) ? 0 : 1;
		snprintf(value, sizeof(value), "%s%d;", value, idx);
	}

	RTMPSetKeyParameter("EdcaIdx", value, sizeof(value), final, TRUE);
	return NDIS_STATUS_SUCCESS;
}

/*
* apedca
*/
static INT multi_profile_merge_apedca(
	struct mpf_data *data,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	CHAR tmpbuf[256] = "";
	CHAR tmpbuf2[256] = "";
	UCHAR edca_idx = 0;
	CHAR *macptr;
	UCHAR i;
	UCHAR j;
	UCHAR edca_own[4] = { 0, 0, 0, 0 };
	CHAR tok_str[25] = "";

	/*for seach 2.4G band EdcaIdx*/
	if (RTMPGetKeyParameter("EdcaIdx", tmpbuf, 256, buf1, FALSE) != TRUE)
		/*default EDCA parameters*/
		return multi_profile_merge_default_edca(data, buf1, buf2, final);

	for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
		edca_idx = simple_strtol(macptr, 0, 10);

		if (edca_idx < 4)
			edca_own[edca_idx] = (0x10 | edca_idx);
	}

	/*for seach 5G band EdcaIdx*/
	if (RTMPGetKeyParameter("EdcaIdx", tmpbuf2, 256, buf2, FALSE) != TRUE)
		return NDIS_STATUS_SUCCESS;

	for (i = 0, macptr = rstrtok(tmpbuf2, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
		edca_idx = simple_strtol(macptr, 0, 10);

		if (edca_idx >= 4) {
			snprintf(tmpbuf, sizeof(tmpbuf), "%s;%d", tmpbuf, 0);
			continue;
		}

		for (j = 0; j < 4; j++) {
			if (edca_own[j] == 0)
				break;
		}

		if (j < 4)
			edca_own[j] = (0x20 | edca_idx);

		snprintf(tmpbuf, sizeof(tmpbuf), "%s;%d", tmpbuf, j);
	}

	RTMPSetKeyParameter("EdcaIdx", tmpbuf, 256, final, TRUE);

	/*merge  ApEdca%*/
	for (i = 0; i < 4; i++) {
		if (edca_own[i] & 0x10)
			macptr = buf1;
		else if (edca_own[i] & 0x20)
			macptr = buf2;
		else
			continue;

		j = (edca_own[i] & 0x3);
		snprintf(tok_str, sizeof(tok_str), "APEdca%d", j);

		if (RTMPGetKeyParameter(tok_str, tmpbuf, 256, macptr, TRUE)) {
			snprintf(tok_str, sizeof(tok_str), "APEdca%d", i);
			RTMPSetKeyParameter(tok_str, tmpbuf, 256, final, TRUE);
		}
	}

	return NDIS_STATUS_SUCCESS;
}

/*
* bssedca
*/
static INT multi_profile_merge_bssedca(
	struct mpf_data *data,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	UCHAR i;
	CHAR aifs[32] = "";
	CHAR cwmin[32] = "";
	CHAR cwmax[32] = "";
	CHAR txop[32] = "";
	CHAR acm[32] = "";
	CHAR *buf = NULL;
	CHAR value[256] = "";
	CHAR tok_str[25] = "";

	for (i = 0; i < data->total_num; i++) {
		buf = (i < data->pf1_num) ? buf1 : buf2;
		os_zero_mem(value, sizeof(value));

		/*BSSAifsn*/
		if (RTMPGetKeyParameter("BSSAifsn", aifs, sizeof(aifs), buf, FALSE) != TRUE)
			return NDIS_STATUS_FAILURE;

		multi_profile_replace(";", ",", aifs);

		/*BSSCwmin*/
		if (RTMPGetKeyParameter("BSSCwmin", cwmin, sizeof(cwmin), buf, FALSE) != TRUE)
			return NDIS_STATUS_FAILURE;

		multi_profile_replace(";", ",", cwmin);

		/*BSSCwmax*/
		if (RTMPGetKeyParameter("BSSCwmax", cwmax, sizeof(cwmax), buf, FALSE) != TRUE)
			return NDIS_STATUS_FAILURE;

		multi_profile_replace(";", ",", cwmax);

		/*BSSTxop*/
		if (RTMPGetKeyParameter("BSSTxop", txop, sizeof(txop), buf, FALSE) != TRUE)
			return NDIS_STATUS_FAILURE;

		multi_profile_replace(";", ",", txop);

		/*BSSACM*/
		if (RTMPGetKeyParameter("BSSACM", acm, sizeof(acm), buf, FALSE) != TRUE)
			return NDIS_STATUS_FAILURE;

		multi_profile_replace(";", ",", acm);
		/*merge*/
		snprintf(value, sizeof(value), "%s;%s;%s;%s;%s", aifs, cwmin, cwmax, txop, acm);
		/*set*/
		snprintf(tok_str, sizeof(tok_str), "BSSEdca%d", i);
		RTMPSetKeyParameter(tok_str, value, sizeof(value), final, TRUE);
	}

	return NDIS_STATUS_SUCCESS;
}

/*
* Channel
*/
static INT multi_profile_merge_channel(
	struct mpf_data *data,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	UCHAR ch1[32] = "";
	UCHAR ch2[32] = "";
	UCHAR value[256] = "";
	UCHAR i;
	UCHAR *ch;

	if (RTMPGetKeyParameter("Channel", ch1, sizeof(ch1), buf1, TRUE) != TRUE)
		return NDIS_STATUS_FAILURE;

	if (RTMPGetKeyParameter("Channel", ch2, sizeof(ch2), buf2, TRUE) != TRUE)
		return NDIS_STATUS_FAILURE;

	os_zero_mem(value, sizeof(value));

	for (i = 0; i < data->total_num; i++) {
		ch = (i < data->pf1_num) ? ch1 : ch2;
		snprintf(value, sizeof(value), "%s%s;", value, ch);
	}

	RTMPSetKeyParameter("Channel", value, sizeof(value), final, TRUE);
	return NDIS_STATUS_SUCCESS;
}

static INT multi_profile_merge_chgrp(
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	UCHAR chgrp1[TEMP_STR_SIZE] = "";
	UCHAR chgrp2[TEMP_STR_SIZE] = "";
	UCHAR value[TEMP_STR_SIZE] = "";

	if (RTMPGetKeyParameter("ChannelGrp", chgrp1, TEMP_STR_SIZE, buf1, TRUE) != TRUE)
		return NDIS_STATUS_FAILURE;
	if (strlen(chgrp1) < LEN_BITMAP_CHGRP) {
		snprintf(chgrp1, sizeof(chgrp1), "0:0:0:0");
	}

	if (RTMPGetKeyParameter("ChannelGrp", chgrp2, TEMP_STR_SIZE, buf2, TRUE) != TRUE)
		return NDIS_STATUS_FAILURE;
	if (strlen(chgrp2) < LEN_BITMAP_CHGRP) {
		snprintf(chgrp2, sizeof(chgrp2), "0:0:0:0");
	}

	os_zero_mem(value, sizeof(value));
	snprintf(value, sizeof(value), "%s;%s", chgrp1, chgrp2);

	RTMPSetKeyParameter("ChannelGrp", value, TEMP_STR_SIZE, final, TRUE);
	return NDIS_STATUS_SUCCESS;
}
/*
* WirelessMode
*/
static INT multi_profile_merge_wireless_mode(
	struct mpf_data *data,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	UCHAR wmode1[32] = "";
	UCHAR wmode2[32] = "";
	UCHAR value[256] = "";
	CHAR *macptr;
	UCHAR i;
	UCHAR *ch;

	if (RTMPGetKeyParameter("WirelessMode", wmode1, sizeof(wmode1), buf1, TRUE) != TRUE)
		return NDIS_STATUS_FAILURE;

	/*check number of wireless mode*/
	for (i = 0, macptr = rstrtok(wmode1, ";") ; macptr ; macptr = rstrtok(NULL, ";"), i++)
		/*do nothing*/

	if (i > 1) {

		multi_profile_merge_separate("WirelessMode", buf1, buf2, final);

	} else {

		if (RTMPGetKeyParameter("WirelessMode", wmode2, sizeof(wmode2), buf2, TRUE) != TRUE)
			return NDIS_STATUS_FAILURE;

		os_zero_mem(value, sizeof(value));
		for (i = 0 ; i < data->total_num ; i++) {

			ch = (i < data->pf1_num) ? wmode1 : wmode2;
			snprintf(value, sizeof(value), "%s%s;", value, ch);

		}

		RTMPSetKeyParameter("WirelessMode", value, sizeof(value), final, TRUE);
	}
	return NDIS_STATUS_SUCCESS;
}

/*
* VOW_BW_Ctrl
*/
static INT multi_profile_merge_vow_bw_ctrl(
	struct mpf_data *data,
	CHAR *parm,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	UCHAR group1[128] = "";
	UCHAR group2[128] = "";
	UCHAR value[128] = "";
	UCHAR i;
	UCHAR *ptok = NULL;

	if (RTMPGetKeyParameter(parm, group1, sizeof(group1), buf1, TRUE) != TRUE)
		return NDIS_STATUS_FAILURE;

	if (RTMPGetKeyParameter(parm, group2, sizeof(group2), buf2, TRUE) != TRUE)
		return NDIS_STATUS_FAILURE;

	os_zero_mem(value, sizeof(value));

	for (i = 0, ptok = rstrtok(group1, ";"); ptok; ptok = rstrtok(NULL, ";"), i++) {
		if (i >= data->pf1_num)
			break;

		snprintf(value, sizeof(value), "%s%s;", value, ptok);
	}

	for (i = 0, ptok = rstrtok(group2, ";"); ptok; ptok = rstrtok(NULL, ";"), i++) {
		if (i >= data->pf2_num)
			break;

		snprintf(value, sizeof(value), "%s%s;", value, ptok);
	}

	RTMPSetKeyParameter(parm, value, sizeof(value), final, TRUE);
	return NDIS_STATUS_SUCCESS;
}

/*
*
*/
static INT multi_profile_merge_ack_policy(
	struct mpf_data *data,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	UCHAR i;
	UCHAR idx;
	CHAR tmpbuf[32];
	CHAR tok_str[25] = "";

	/*read 2.4G profile*/
	if (RTMPGetKeyParameter("AckPolicy", tmpbuf, 32, buf1, TRUE)) {
		for (i = 0 ; i < data->pf1_num ; i++) {
			snprintf(tok_str, sizeof(tok_str), "APAckPolicy%d", i);
			RTMPSetKeyParameter(tok_str, tmpbuf, sizeof(tmpbuf), final, TRUE);
		}
	}

	/*read 5G profile*/
	if (RTMPGetKeyParameter("AckPolicy", tmpbuf, 32, buf2, TRUE)) {
		for (i = 0 ; i < data->pf2_num ; i++) {
			idx = i + data->pf1_num;
			snprintf(tok_str, sizeof(tok_str), "APAckPolicy%d", idx);
			RTMPSetKeyParameter(tok_str, tmpbuf, sizeof(tmpbuf), final, TRUE);
		}
	}

	return NDIS_STATUS_SUCCESS;
}

/*
* Country Region
*/
static INT multi_profile_merge_country_region(
	struct mpf_data *data,
	CHAR *parm,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	INT Status = FALSE;
	CHAR value[TEMP_STR_SIZE] = "";
	CHAR tmpbuf[TEMP_STR_SIZE] = "";

#ifdef DEFAULT_5G_PROFILE
	Status = RTMPGetKeyParameter(parm, tmpbuf, TEMP_STR_SIZE, buf1, TRUE);
#else
	Status = RTMPGetKeyParameter(parm, tmpbuf, TEMP_STR_SIZE, buf2, TRUE);
#endif

	if (Status == TRUE) {
		snprintf(value, sizeof(value), "%s", tmpbuf);
		RTMPSetKeyParameter(parm, value, TEMP_STR_SIZE, final, TRUE);
	}

	return NDIS_STATUS_SUCCESS;
}

/*
* mbss related merge function
*/
static INT multi_profile_merge_mbss(
	struct mpf_data *data,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	/*merge MACAddress*/
	multi_profile_merge_mac_address(data, buf1, buf2, final);
	/*merge SSID*/
	multi_profile_merge_increase(data, 1, "SSID", buf1, buf2, final);
	/*merge FragThreshold*/
	multi_profile_merge_perband(data, "FragThreshold", buf1, buf2, final);
	/*merge DLSCapable*/
	multi_profile_merge_separate("DLSCapable", buf1, buf2, final);
	/* MAP config */
#ifdef CONFIG_MAP_SUPPORT
	multi_profile_merge_separate("MapEnable", buf1, buf2, final);
	multi_profile_merge_separate("MAP_Turnkey", buf1, buf2, final);
	multi_profile_merge_separate("MAP_Ext", buf1, buf2, final);
#endif
	/*merge WirelessMode*/
	multi_profile_merge_wireless_mode(data, buf1, buf2, final);
	/*merge Channel*/
	multi_profile_merge_channel(data, buf1, buf2, final);
	/*merge ChannelGrp*/
	multi_profile_merge_chgrp(buf1, buf2, final);
	/*merge AutoChannelSkipList*/
	multi_profile_merge_separate("AutoChannelSkipList", buf1, buf2, final);
	/*merge ACSCheckTime*/
	multi_profile_merge_separate("ACSCheckTime", buf1, buf2, final);
	/*merge security*/
	multi_profile_merge_security(data, buf1, buf2, final);
	/*merge WmmCapable*/
	multi_profile_merge_separate("WmmCapable", buf1, buf2, final);
	/*merge NoForwarding*/
	multi_profile_merge_separate("NoForwarding", buf1, buf2, final);
	/*merge StationKeepAlive*/
	multi_profile_merge_perband(data, "StationKeepAlive", buf1, buf2, final);
	/*merge HideSSID*/
	multi_profile_merge_separate("HideSSID", buf1, buf2, final);
	/*merge HT_EXTCHA*/
	multi_profile_merge_perband(data, "HT_EXTCHA", buf1, buf2, final);
	/*merge HT_TxStream*/
	multi_profile_merge_perband(data, "HT_TxStream", buf1, buf2, final);
	/*merge HT_RxStream*/
	multi_profile_merge_perband(data, "HT_RxStream", buf1, buf2, final);
	/*merge HT_MCS*/
	multi_profile_merge_separate("HT_MCS", buf1, buf2, final);
	/*merge HT_BW*/
	multi_profile_merge_perband(data, "HT_BW", buf1, buf2, final);
	/*merge HT_STBC*/
	multi_profile_merge_perband(data, "HT_STBC", buf1, buf2, final);
	/*merge HT_LDPC*/
	multi_profile_merge_perband(data, "HT_LDPC", buf1, buf2, final);
	/*merge HT_AMSDU*/
	multi_profile_merge_perband(data, "HT_AMSDU", buf1, buf2, final);
	/*merge VHT_STBC*/
	multi_profile_merge_separate("VHT_STBC", buf1, buf2, final);
	/*merge VHT_LDPC*/
	multi_profile_merge_separate("VHT_LDPC", buf1, buf2, final);
	/*merge MbssMaxStaNum*/
	multi_profile_merge_separate("MbssMaxStaNum", buf1, buf2, final);
	/*merge APSDCapable*/
	multi_profile_merge_separate("APSDCapable", buf1, buf2, final);
	/*merge DscpQosMapping*/
#ifdef DSCP_QOS_MAP_SUPPORT
	multi_profile_merge_separate("DscpQosMapEnable", buf1, buf2, final);
	multi_profile_merge_separate("DscpQosMap", buf1, buf2, final);
#endif
	/*merge APEdcaIdx*/
	multi_profile_merge_apedca(data, buf1, buf2, final);
	/*merge BSSEdcaIdx*/
	multi_profile_merge_bssedca(data, buf1, buf2, final);
	/*merge AckPolicy*/
	multi_profile_merge_ack_policy(data, buf1, buf2, final);
	/*merge CountryRegionABand*/
	multi_profile_merge_country_region(data, "CountryRegionABand", buf1, buf2, final);
	/*merge VOW BW_Ctrl related profile*/
	multi_profile_merge_vow_bw_ctrl(data, "VOW_Group_Min_Rate", buf1, buf2, final);
	multi_profile_merge_vow_bw_ctrl(data, "VOW_Group_Max_Rate", buf1, buf2, final);
	multi_profile_merge_vow_bw_ctrl(data, "VOW_Group_Min_Ratio", buf1, buf2, final);
	multi_profile_merge_vow_bw_ctrl(data, "VOW_Group_Max_Ratio", buf1, buf2, final);
	multi_profile_merge_vow_bw_ctrl(data, "VOW_Airtime_Ctrl_En", buf1, buf2, final);
	multi_profile_merge_vow_bw_ctrl(data, "VOW_Rate_Ctrl_En", buf1, buf2, final);
	multi_profile_merge_vow_bw_ctrl(data, "VOW_Group_Min_Rate_Bucket_Size", buf1, buf2, final);
	multi_profile_merge_vow_bw_ctrl(data, "VOW_Group_Max_Rate_Bucket_Size", buf1, buf2, final);
	multi_profile_merge_vow_bw_ctrl(data, "VOW_Group_Min_Airtime_Bucket_Size", buf1, buf2, final);
	multi_profile_merge_vow_bw_ctrl(data, "VOW_Group_Max_Airtime_Bucket_Size", buf1, buf2, final);
	multi_profile_merge_vow_bw_ctrl(data, "VOW_Group_Backlog", buf1, buf2, final);
	multi_profile_merge_vow_bw_ctrl(data, "VOW_Group_Max_Wait_Time", buf1, buf2, final);
	multi_profile_merge_vow_bw_ctrl(data, "VOW_Group_DWRR_Quantum", buf1, buf2, final);
	multi_profile_merge_wsc(data, buf1, buf2, final);
#ifdef TXBF_SUPPORT
	/*merge ETxBfEnCond*/
	multi_profile_merge_perband(data, "ETxBfEnCond", buf1, buf2, final);
	/*merge ITxBfEn*/
	multi_profile_merge_perband(data, "ITxBfEn", buf1, buf2, final);
#endif /* TXBF_SUPPORT */

	return NDIS_STATUS_SUCCESS;
}

/*
* wsc related merge function
*/
static INT multi_profile_merge_wsc(
	struct mpf_data *data,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
#if defined(WSC_V2_SUPPORT) || defined(WSC_AP_SUPPORT)
	UCHAR WscConMode[32] = "";
	UCHAR WscConMode2[32] = "";
	UCHAR value[256] = "";
	INT	i;
	CHAR *macptr;
#endif
#ifdef WSC_V2_SUPPORT
	/*merge WscV2Support*/
	{
		/*merge WscConfMode*/
		if (RTMPGetKeyParameter("WscV2Support", WscConMode, sizeof(WscConMode), buf1, TRUE) != TRUE)
			goto label_wsc_v2_done;

		for (i = 0, macptr = rstrtok(WscConMode, ";"); macptr; macptr = rstrtok(NULL, ";"), i++)
			;/*do nothing*/

		if (RTMPGetKeyParameter("WscV2Support", WscConMode, sizeof(WscConMode), buf1, TRUE) != TRUE)
			goto label_wsc_v2_done;

		if (data->pf1_num > i) {/* need to append default value */
			INT append_cnt = data->pf1_num - i;
			INT loop_cnt = 0;

			while (append_cnt) {
				snprintf(WscConMode, sizeof(WscConMode), "%s; ", WscConMode);
				append_cnt--;
				loop_cnt++;
			}
		} else if (data->pf1_num < i)
			goto label_wsc_v2_done;

		if (RTMPGetKeyParameter("WscV2Support", WscConMode2, sizeof(WscConMode2), buf2, TRUE) != TRUE)
			goto label_wsc_v2_done;

		for (i = 0, macptr = rstrtok(WscConMode2, ";"); macptr; macptr = rstrtok(NULL, ";"), i++)
			;/*do nothing*/

		if (RTMPGetKeyParameter("WscV2Support", WscConMode2, sizeof(WscConMode2), buf2, TRUE) != TRUE)
			goto label_wsc_v2_done;

		if (data->pf2_num > i) {/* need to append default value */
			INT append_cnt = data->pf2_num - i;
			INT loop_cnt = 0;

			while (append_cnt) {
				snprintf(WscConMode2, sizeof(WscConMode2), "%s; ", WscConMode2);
				append_cnt--;
				loop_cnt++;
			}
		} else if (data->pf2_num < i)
			goto label_wsc_v2_done;

		snprintf(value, sizeof(value), "%s;%s", WscConMode, WscConMode2);
		RTMPSetKeyParameter("WscV2Support", value, sizeof(value), final, TRUE);
	}
label_wsc_v2_done:
#endif /*WSC_V2_SUPPORT*/
#ifdef WSC_AP_SUPPORT
	{
		/*merge WscConfMode*/
		if (RTMPGetKeyParameter("WscConfMode", WscConMode, sizeof(WscConMode), buf1, TRUE) != TRUE)
			goto label_WscConfMode_done;

		for (i = 0, macptr = rstrtok(WscConMode, ";"); macptr; macptr = rstrtok(NULL, ";"), i++)
			;/*do nothing*/

		if (RTMPGetKeyParameter("WscConfMode", WscConMode, sizeof(WscConMode), buf1, TRUE) != TRUE)
			goto label_WscConfMode_done;

		if (data->pf1_num > i) {/* need to append default value */
			INT append_cnt = data->pf1_num - i;
			INT loop_cnt = 0;

			while (append_cnt) {
				snprintf(WscConMode, sizeof(WscConMode), "%s; ", WscConMode);
				append_cnt--;
				loop_cnt++;
			}
		} else if (data->pf1_num < i)
			goto label_WscConfMode_done;

		if (RTMPGetKeyParameter("WscConfMode", WscConMode2, sizeof(WscConMode2), buf2, TRUE) != TRUE)
			goto label_WscConfMode_done;

		for (i = 0, macptr = rstrtok(WscConMode2, ";"); macptr; macptr = rstrtok(NULL, ";"), i++)
			;/*do nothing*/

		if (RTMPGetKeyParameter("WscConfMode", WscConMode2, sizeof(WscConMode2), buf2, TRUE) != TRUE)
			goto label_WscConfMode_done;

		if (data->pf2_num > i) {/* need to append default value */
			INT append_cnt = data->pf2_num - i;
			INT loop_cnt = 0;

			while (append_cnt) {
				snprintf(WscConMode2, sizeof(WscConMode2), "%s; ", WscConMode2);
				append_cnt--;
				loop_cnt++;
			}
		} else if (data->pf2_num < i)
			goto label_WscConfMode_done;

		snprintf(value, sizeof(value), "%s;%s", WscConMode, WscConMode2);
		RTMPSetKeyParameter("WscConfMode", value, sizeof(value), final, TRUE);
label_WscConfMode_done:

		/*merge WscConfStatus*/
		if (RTMPGetKeyParameter("WscConfStatus", WscConMode, sizeof(WscConMode), buf1, TRUE) != TRUE)
			goto label_WscConfStatus_done;

		for (i = 0, macptr = rstrtok(WscConMode, ";"); macptr; macptr = rstrtok(NULL, ";"), i++)
			;/*do nothing*/

		if (RTMPGetKeyParameter("WscConfStatus", WscConMode, sizeof(WscConMode), buf1, TRUE) != TRUE)
			goto label_WscConfStatus_done;

		if (data->pf1_num > i) {/* need to append default value */
			INT append_cnt = data->pf1_num - i;
			INT loop_cnt = 0;

			while (append_cnt) {
				snprintf(WscConMode, sizeof(WscConMode), "%s; ", WscConMode);
				append_cnt--;
				loop_cnt++;
			}
		} else if (data->pf1_num < i)
			goto label_WscConfStatus_done;

		if (RTMPGetKeyParameter("WscConfStatus", WscConMode2, sizeof(WscConMode2), buf2, TRUE) != TRUE)
			goto label_WscConfStatus_done;

		for (i = 0, macptr = rstrtok(WscConMode2, ";"); macptr; macptr = rstrtok(NULL, ";"), i++)
			;/*do nothing*/

		if (RTMPGetKeyParameter("WscConfStatus", WscConMode2, sizeof(WscConMode2), buf2, TRUE) != TRUE)
			goto label_WscConfStatus_done;

		if (data->pf2_num > i) {/* need to append default value */
			INT append_cnt = data->pf2_num - i;
			INT loop_cnt = 0;

			while (append_cnt) {
				snprintf(WscConMode2, sizeof(WscConMode2), "%s; ", WscConMode2);
				append_cnt--;
				loop_cnt++;
			}
		} else if (data->pf2_num < i)
			goto label_WscConfStatus_done;

		snprintf(value, sizeof(value), "%s;%s", WscConMode, WscConMode2);
		RTMPSetKeyParameter("WscConfStatus", value, sizeof(value), final, TRUE);
	}
label_WscConfStatus_done:
#endif /*WSC_AP_SUPPORT*/
	return NDIS_STATUS_SUCCESS;
}
#endif /*MBSS_SUPPORT*/


#ifdef APCLI_SUPPORT
/*
* apcli related merge function
*/
static INT multi_profile_merge_apcli(
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	INT status = FALSE;
	CHAR tmpbuf[TEMP_STR_SIZE] = "";
	CHAR value[TEMP_STR_SIZE] = "";
#ifdef MAC_REPEATER_SUPPORT
	/*MACRepeaterEn, use profile 1*/
	status = RTMPGetKeyParameter("MACRepeaterEn", tmpbuf, TEMP_STR_SIZE, buf1, TRUE);

	if (status == TRUE) {
		snprintf(value, sizeof(value), "%s", tmpbuf);
		RTMPSetKeyParameter("MACRepeaterEn", value, TEMP_STR_SIZE, final, TRUE);
	}

	/*MACRepeaterOuiMode, use profile 1*/
	status = RTMPGetKeyParameter("MACRepeaterOuiMode", tmpbuf, TEMP_STR_SIZE, buf1, TRUE);

	if (status == TRUE) {
		snprintf(value, sizeof(value), "%s", tmpbuf);
		RTMPSetKeyParameter("MACRepeaterOuiMode", value, TEMP_STR_SIZE, final, TRUE);
	}

#endif
	/*merge ApCliEnable*/
	multi_profile_merge_separate("ApCliEnable", buf1, buf2, final);
	/*merge ApCliSsid*/
	/*multi_profile_merge_separate("ApCliSsid", buf1, buf2, final);*/
	/*read SSID with space allowed*/
	{
		if (RTMPGetKeyParameter("ApCliSsid", tmpbuf, TEMP_STR_SIZE, buf1, FALSE) == TRUE)
			snprintf(value, sizeof(value), "%s", tmpbuf);
		else
			snprintf(value, sizeof(value), "%s", "");
		if (RTMPGetKeyParameter("ApCliSsid", tmpbuf, TEMP_STR_SIZE, buf2, FALSE) == TRUE)
			snprintf(value, sizeof(value), "%s;%s", value, tmpbuf);
		RTMPSetKeyParameter("ApCliSsid", value, TEMP_STR_SIZE, final, FALSE);
	}
	/*merge ApCliWirelessMode*/
	multi_profile_merge_separate("ApCliWirelessMode", buf1, buf2, final);
	/*merge ApCliBssid*/
	multi_profile_merge_separate("ApCliBssid", buf1, buf2, final);
	/*merge ApCliAuthMode*/
	multi_profile_merge_separate("ApCliAuthMode", buf1, buf2, final);
	/*merge ApCliEncrypType*/
	multi_profile_merge_separate("ApCliEncrypType", buf1, buf2, final);
	{
		/*merge apcli0 ApCliWPAPSK*/
		status = RTMPGetKeyParameter("ApCliWPAPSK", tmpbuf, TEMP_STR_SIZE, buf1, TRUE);

		if (status == TRUE) {
			snprintf(value, sizeof(value), "%s", tmpbuf);
			RTMPSetKeyParameter("ApCliWPAPSK", value, TEMP_STR_SIZE, final, TRUE);
		}

		/*tansfer apcli1 ApCliWPAPSK to ApCliWPAPSK1*/
		status = RTMPGetKeyParameter("ApCliWPAPSK", tmpbuf, TEMP_STR_SIZE, buf2, TRUE);

		if (status == TRUE) {
			snprintf(value, sizeof(value), "%s", tmpbuf);
			RTMPSetKeyParameter("ApCliWPAPSK1", value, TEMP_STR_SIZE, final, TRUE);
		}
	}
	/*merge ApCliDefaultKeyID*/
	multi_profile_merge_separate("ApCliDefaultKeyID", buf1, buf2, final);
	/*merge ApCliKey1Type*/
	multi_profile_merge_separate("ApCliKey1Type", buf1, buf2, final);
	/*merge ApCliKey2Type*/
	multi_profile_merge_separate("ApCliKey2Type", buf1, buf2, final);
	/*merge ApCliKey3Type*/
	multi_profile_merge_separate("ApCliKey3Type", buf1, buf2, final);
	/*merge ApCliKey4Type*/
	multi_profile_merge_separate("ApCliKey4Type", buf1, buf2, final);
	{
		/*merge apcli0 ApCliKey1Str*/
		status = RTMPGetKeyParameter("ApCliKey1Str", tmpbuf, TEMP_STR_SIZE, buf1, TRUE);

		if (status == TRUE) {
			snprintf(value, sizeof(value), "%s", tmpbuf);
			RTMPSetKeyParameter("ApCliKey1Str", value, TEMP_STR_SIZE, final, TRUE);
		}

		/*tansfer apcli1 ApCliKey1Str to ApCliKey1Str1*/
		status = RTMPGetKeyParameter("ApCliKey1Str", tmpbuf, TEMP_STR_SIZE, buf2, TRUE);

		if (status == TRUE) {
			snprintf(value, sizeof(value), "%s", tmpbuf);
			RTMPSetKeyParameter("ApCliKey1Str1", value, TEMP_STR_SIZE, final, TRUE);
		}

		/*merge apcli0 ApCliKey2Str*/
		status = RTMPGetKeyParameter("ApCliKey2Str", tmpbuf, TEMP_STR_SIZE, buf1, TRUE);

		if (status == TRUE) {
			snprintf(value, sizeof(value), "%s", tmpbuf);
			RTMPSetKeyParameter("ApCliKey2Str", value, TEMP_STR_SIZE, final, TRUE);
		}

		/*tansfer apcli1 ApCliKey2Str to ApCliKey2Str1*/
		status = RTMPGetKeyParameter("ApCliKey2Str", tmpbuf, TEMP_STR_SIZE, buf2, TRUE);

		if (status == TRUE) {
			snprintf(value, sizeof(value), "%s", tmpbuf);
			RTMPSetKeyParameter("ApCliKey2Str1", value, TEMP_STR_SIZE, final, TRUE);
		}

		/*merge apcli0 ApCliKey3Str*/
		status = RTMPGetKeyParameter("ApCliKey3Str", tmpbuf, TEMP_STR_SIZE, buf1, TRUE);

		if (status == TRUE) {
			snprintf(value, sizeof(value), "%s", tmpbuf);
			RTMPSetKeyParameter("ApCliKey3Str", value, TEMP_STR_SIZE, final, TRUE);
		}

		/*tansfer apcli1 ApCliKey3Str to ApCliKey3Str1*/
		status = RTMPGetKeyParameter("ApCliKey3Str", tmpbuf, TEMP_STR_SIZE, buf2, TRUE);

		if (status == TRUE) {
			snprintf(value, sizeof(value), "%s", tmpbuf);
			RTMPSetKeyParameter("ApCliKey3Str1", value, TEMP_STR_SIZE, final, TRUE);
		}

		/*merge apcli0 ApCliKey4Str*/
		status = RTMPGetKeyParameter("ApCliKey4Str", tmpbuf, TEMP_STR_SIZE, buf1, TRUE);

		if (status == TRUE) {
			snprintf(value, sizeof(value), "%s", tmpbuf);
			RTMPSetKeyParameter("ApCliKey4Str", value, TEMP_STR_SIZE, final, TRUE);
		}

		/*tansfer apcli1 ApCliKey4Str to ApCliKey4Str1*/
		status = RTMPGetKeyParameter("ApCliKey4Str", tmpbuf, TEMP_STR_SIZE, buf2, TRUE);

		if (status == TRUE) {
			snprintf(value, sizeof(value), "%s", tmpbuf);
			RTMPSetKeyParameter("ApCliKey4Str1", value, TEMP_STR_SIZE, final, TRUE);
		}
	}
	/*merge ApCliTxMode*/
	multi_profile_merge_separate("ApCliTxMode", buf1, buf2, final);
	/*merge ApCliTxMcs*/
	multi_profile_merge_separate("ApCliTxMcs", buf1, buf2, final);
#ifdef WSC_AP_SUPPORT
	/*merge ApCli_Wsc4digitPinCode*/
	multi_profile_merge_separate("ApCli_Wsc4digitPinCode", buf1, buf2, final);
	/*merge ApCliWscScanMode*/
	multi_profile_merge_separate("ApCliWscScanMode", buf1, buf2, final);
#endif /*WSC_AP_SUPPORT*/
#ifdef UAPSD_SUPPORT
	/*merge ApCliAPSDCapable*/
	multi_profile_merge_separate("ApCliAPSDCapable", buf1, buf2, final);
#endif /*UAPSD_SUPPORT*/
	/*merge ApCliPMFMFPC*/
	multi_profile_merge_separate("ApCliPMFMFPC", buf1, buf2, final);
	/*merge ApCliPMFMFPR*/
	multi_profile_merge_separate("ApCliPMFMFPR", buf1, buf2, final);
	/*merge ApCliPMFSHA256*/
	multi_profile_merge_separate("ApCliPMFSHA256", buf1, buf2, final);
	return NDIS_STATUS_SUCCESS;
}
#endif /*APCLI_SUPPORT*/

#ifdef BAND_STEERING
static INT multi_profile_merge_bandsteering(
	struct mpf_data *data,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	/* multi_profile_merge_separate("BndStrgBssIdx",buf1,buf2,final); */
	/* return NDIS_STATUS_SUCCESS; */

	CHAR tmpbuf[TEMP_STR_SIZE] = "";
	CHAR tmpbuf2[TEMP_STR_SIZE] = "";
	CHAR value[TEMP_STR_SIZE] = "";
	RTMP_STRING *macptr = NULL;
	int i = 0;

	if (!buf1 || !buf2)
		return NDIS_STATUS_FAILURE;

	if (RTMPGetKeyParameter("BndStrgBssIdx", tmpbuf, TEMP_STR_SIZE, buf1, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			if (i == data->pf1_num)
				break;

			if (i == 0)
				snprintf((value + strlen(value)), sizeof(value), "%s", macptr);
			else
				snprintf((value + strlen(value)), sizeof(value), ";%s", macptr);
		}
		if (i < data->pf1_num) {
			for (; i < data->pf1_num; i++) {
				if (i == 0)
					snprintf((value + strlen(value)), sizeof(value), "%s", "1");
				else
					snprintf((value + strlen(value)), sizeof(value), ";%s", "0");
			}
		}
	} else{
		for (i = 0; i < data->pf1_num; i++) {
			if (i == 0)
				snprintf((value + strlen(value)), sizeof(value), "%s", "1");
			else
				snprintf((value + strlen(value)), sizeof(value), ";%s", "0");
		}
	}

	if (RTMPGetKeyParameter("BndStrgBssIdx", tmpbuf2, TEMP_STR_SIZE, buf2, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf2, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			if (i == data->pf2_num)
				break;
			snprintf((value + strlen(value)), sizeof(value), ";%s", macptr);
		}
		if (i < data->pf2_num) {
			for (; i < data->pf2_num; i++) {
				if (i == 0)
					snprintf((value + strlen(value)), sizeof(value), ";%s", "1");
				else
					snprintf((value + strlen(value)), sizeof(value), ";%s", "0");
			}
		}
	} else{
		for (i = 0; i < data->pf2_num; i++) {
			if (i == 0)
				snprintf((value + strlen(value)), sizeof(value), ";%s", "1");
			else
				snprintf((value + strlen(value)), sizeof(value), ";%s", "0");
		}
	}
	RTMPSetKeyParameter("BndStrgBssIdx", value, TEMP_STR_SIZE, final, TRUE);
	return NDIS_STATUS_SUCCESS;

}
#endif

/*
* BssidNum
*/
static INT multi_profile_merge_bssidnum(struct mpf_data *data, CHAR *buf1, CHAR *buf2, CHAR *final)
{
	CHAR tmpbuf[25] = "";
	UCHAR num1 = 0;
	UCHAR num2 = 0;
	UCHAR total;

	if (RTMPGetKeyParameter("BssidNum", tmpbuf, 25, buf1, TRUE))
		num1 = (UCHAR) simple_strtol(tmpbuf, 0, 10);

	if (RTMPGetKeyParameter("BssidNum", tmpbuf, 25, buf2, TRUE))
		num2 = (UCHAR) simple_strtol(tmpbuf, 0, 10);

	total = num1 + num2;
	snprintf(tmpbuf, sizeof(tmpbuf), "%d", total);
	RTMPSetKeyParameter("BssidNum", tmpbuf, 25, final, TRUE);
	/*assign bss number*/
	data->pf1_num = num1;
	data->pf2_num = num2;
	data->total_num = total;
	return NDIS_STATUS_SUCCESS;
}

static INT multi_profile_merge_edcca(CHAR *buf1, CHAR *buf2, CHAR *final)
{
	/*merge EDCCA*/
	multi_profile_merge_separate("EDCCAEnable", buf1, buf2, final);
	return NDIS_STATUS_SUCCESS;
}
#endif /*CONFIG_AP_SUPPORT*/

/*
 * protections: including HT_PROTECT / RTS_THRESHOLD
 */
static INT multi_profile_merge_protection(
	struct mpf_data *data,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	/*RTSPktThreshold*/
	multi_profile_merge_perband(data, "RTSPktThreshold", buf1, buf2, final);
	/*RTSThreshold*/
	multi_profile_merge_perband(data, "RTSThreshold", buf1, buf2, final);
	/*HT_PRORTECT*/
	multi_profile_merge_perband(data, "HT_PROTECT", buf1, buf2, final);
	return NDIS_STATUS_SUCCESS;
}

static INT multi_profile_merge_frag(
	struct mpf_data *data,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	/*Fragment Threshold*/
	multi_profile_merge_perband(data, "FragThreshold", buf1, buf2, final);
	return NDIS_STATUS_SUCCESS;
}

static INT multi_profile_merge_gi(
	struct mpf_data *data,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	/*HT_GI*/
	multi_profile_merge_perband(data, "HT_GI", buf1, buf2, final);
	/*VHT_SGI*/
	/*multi_profile_merge_separate(data, "VHT_SGI", buf1, buf2, final);*/
	return NDIS_STATUS_SUCCESS;
}

static INT multi_profile_merge_mpdu_density(
	struct mpf_data *data,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	/*HT_MpduDensity*/
	multi_profile_merge_perband(data, "HT_MpduDensity", buf1, buf2, final);
	return NDIS_STATUS_SUCCESS;
}

static INT multi_profile_merge_ht_mode(
	struct mpf_data *data,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	/*HT_OpMode*/
	multi_profile_merge_perband(data, "HT_OpMode", buf1, buf2, final);
	return NDIS_STATUS_SUCCESS;
}

/*
* merge 5G only related
*/
static INT multi_profile_merge_5g_only(
	struct mpf_data *data,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	CHAR tmpbuf[64] = "";
	CHAR *buf_mu = buf2;
	UCHAR len = sizeof(tmpbuf);
#ifndef DEFAULT_5G_PROFILE
	UCHAR tmpbuf_dbdc[25] = "";
	UCHAR dbdc_mode = 0;
#endif /* n DEFAULT_5G_PROFILE*/

	/*merge VHT_BW*/
	if (multi_profile_merge_perband(data, "VHT_BW", buf1, buf2, final) != NDIS_STATUS_SUCCESS)
		return NDIS_STATUS_FAILURE;

	/*merge VHT_SGI*/
	if (multi_profile_merge_perband(data, "VHT_SGI", buf1, buf2, final) != NDIS_STATUS_SUCCESS)
		return NDIS_STATUS_FAILURE;

	/*merge VHT_BW_SIGNAL*/
	if (multi_profile_merge_perband(data, "VHT_BW_SIGNAL", buf1, buf2, final) != NDIS_STATUS_SUCCESS)
		return NDIS_STATUS_FAILURE;

	/*merge VHT_Sec80_Channel*/
	if (multi_profile_merge_perband(data, "VHT_Sec80_Channel", buf1, buf2, final) != NDIS_STATUS_SUCCESS)
		return NDIS_STATUS_FAILURE;


#ifndef DEFAULT_5G_PROFILE
	/*check dbdc mode is enable*/
	if (RTMPGetKeyParameter("DBDC_MODE", tmpbuf_dbdc, 25, buf1, TRUE)) {
		dbdc_mode = (UCHAR) simple_strtol(tmpbuf_dbdc, 0, 10);

		if (dbdc_mode == ENUM_DBDC_5G5G)
			buf_mu = buf1; /* 5G+5G, use 1st profile's MUTxRxEnable */

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("[%s] DBDC_MODE=%d\n", __func__, dbdc_mode));
	}
#endif /* n DEFAULT_5G_PROFILE*/

	/*MUTxRxEnable*/
	if (RTMPGetKeyParameter("MUTxRxEnable", tmpbuf, len, buf_mu, TRUE) == TRUE)
		RTMPSetKeyParameter("MUTxRxEnable", tmpbuf, len, final, TRUE);

#ifdef DEFAULT_5G_PROFILE
	buf_mu = buf1;
#endif

	/*IEEE80211H*/
	if (RTMPGetKeyParameter("IEEE80211H", tmpbuf, len, buf_mu, TRUE) == TRUE)
		RTMPSetKeyParameter("IEEE80211H", tmpbuf, len, final, TRUE);

	/*DFS related params is 5G only, use profile 2*/
#ifdef MT_DFS_SUPPORT

	/*DfsEnable*/
	if (RTMPGetKeyParameter("DfsEnable", tmpbuf, len, buf_mu, TRUE) == TRUE)
		RTMPSetKeyParameter("DfsEnable", tmpbuf, len, final, TRUE);

#endif

	/*RDRegion*/
	if (RTMPGetKeyParameter("RDRegion", tmpbuf, len, buf_mu, TRUE) == TRUE)
		RTMPSetKeyParameter("RDRegion", tmpbuf, len, final, TRUE);

	return NDIS_STATUS_SUCCESS;
}

#ifdef DEFAULT_5G_PROFILE
/*
* merge 2G only related
*/
static INT multi_profile_merge_2g_only(CHAR *buf1, CHAR *buf2, CHAR *final)
{
	CHAR tmpbuf[64] = "";
	UCHAR len = sizeof(tmpbuf);

	/*merge CountryRegion*/
	if (RTMPGetKeyParameter("CountryRegion", tmpbuf, len, buf2, TRUE) == TRUE)
		RTMPSetKeyParameter("CountryRegion", tmpbuf, len, final, TRUE);

	/*merge DisableOLBC*/
	if (RTMPGetKeyParameter("DisableOLBC", tmpbuf, len, buf2, TRUE) == TRUE)
		RTMPSetKeyParameter("DisableOLBC", tmpbuf, len, final, TRUE);

	/*merge G_BAND_256QAM*/
	if (RTMPGetKeyParameter("G_BAND_256QAM", tmpbuf, len, buf2, TRUE) == TRUE)
		RTMPSetKeyParameter("G_BAND_256QAM", tmpbuf, len, final, TRUE);

	/*merge HT_BSSCoexistence*/
	if (RTMPGetKeyParameter("HT_BSSCoexistence", tmpbuf, len, buf2, TRUE) == TRUE)
		RTMPSetKeyParameter("HT_BSSCoexistence", tmpbuf, len, final, TRUE);

	/*BGProtection*/
	if (RTMPGetKeyParameter("BGProtection", tmpbuf, len, buf2, TRUE) == TRUE)
		RTMPSetKeyParameter("BGProtection", tmpbuf, len, final, TRUE);

	/*TxPreamble*/
	if (RTMPGetKeyParameter("TxPreamble", tmpbuf, len, buf2, TRUE) == TRUE)
		RTMPSetKeyParameter("TxPreamble", tmpbuf, len, final, TRUE);


	return NDIS_STATUS_SUCCESS;
}
/*
* merge global setting only related
*/
static INT multi_profile_merge_global_setting_only(CHAR *buf1, CHAR *buf2, CHAR *final)
{
	CHAR tmpbuf[64] = "";
	UCHAR len = sizeof(tmpbuf);

	/*merge CountryCode*/
	if (RTMPGetKeyParameter("CountryCode", tmpbuf, len, buf2, TRUE) == TRUE)
		RTMPSetKeyParameter("CountryCode", tmpbuf, len, final, TRUE);

	/*merge NoForwardingBTNBSSID*/
	if (RTMPGetKeyParameter("NoForwardingBTNBSSID", tmpbuf, len, buf2, TRUE) == TRUE)
		RTMPSetKeyParameter("NoForwardingBTNBSSID", tmpbuf, len, final, TRUE);

	/*merge GreenAP*/
	if (RTMPGetKeyParameter("GreenAP", tmpbuf, len, buf2, TRUE) == TRUE)
		RTMPSetKeyParameter("GreenAP", tmpbuf, len, final, TRUE);

	/*merge PcieAspm*/
	if (RTMPGetKeyParameter("PcieAspm", tmpbuf, len, buf2, TRUE) == TRUE)
		RTMPSetKeyParameter("PcieAspm", tmpbuf, len, final, TRUE);

	/*DBDC_MODE*/
	if (RTMPGetKeyParameter("DBDC_MODE", tmpbuf, len, buf2, TRUE) == TRUE)
		RTMPSetKeyParameter("DBDC_MODE", tmpbuf, len, final, TRUE);

	if (RTMPGetKeyParameter("DBDC_MODE", tmpbuf, len, buf1, TRUE) == TRUE)
		RTMPSetKeyParameter("DBDC_MODE", tmpbuf, len, final, TRUE);

	/*IcapMode*/
	if (RTMPGetKeyParameter("IcapMode", tmpbuf, len, buf2, TRUE) == TRUE)
		RTMPSetKeyParameter("IcapMode", tmpbuf, len, final, TRUE);

	/*CarrierDetect*/
	if (RTMPGetKeyParameter("CarrierDetect", tmpbuf, len, buf2, TRUE) == TRUE)
		RTMPSetKeyParameter("CarrierDetect", tmpbuf, len, final, TRUE);

	/*DebugFlags*/
	if (RTMPGetKeyParameter("DebugFlags", tmpbuf, len, buf2, TRUE) == TRUE)
		RTMPSetKeyParameter("DebugFlags", tmpbuf, len, final, TRUE);

	/*E2pAccessMode*/
	if (RTMPGetKeyParameter("E2pAccessMode", tmpbuf, len, buf2, TRUE) == TRUE)
		RTMPSetKeyParameter("E2pAccessMode", tmpbuf, len, final, TRUE);

	/*EfuseBufferMode*/
	if (RTMPGetKeyParameter("EfuseBufferMode", tmpbuf, len, buf2, TRUE) == TRUE)
		RTMPSetKeyParameter("EfuseBufferMode", tmpbuf, len, final, TRUE);

	/*WCNTest*/
	if (RTMPGetKeyParameter("WCNTest", tmpbuf, len, buf2, TRUE) == TRUE)
		RTMPSetKeyParameter("WCNTest", tmpbuf, len, final, TRUE);

	/*AutoChannelSelect*/
	if (RTMPGetKeyParameter("AutoChannelSelect", tmpbuf, len, buf2, TRUE) == TRUE)
		RTMPSetKeyParameter("AutoChannelSelect", tmpbuf, len, final, TRUE);

	/*HT_RDG*/
	if (RTMPGetKeyParameter("HT_RDG", tmpbuf, len, buf2, TRUE) == TRUE)
		RTMPSetKeyParameter("HT_RDG", tmpbuf, len, final, TRUE);

	/*HT_BADecline*/
	if (RTMPGetKeyParameter("HT_BADecline", tmpbuf, len, buf2, TRUE) == TRUE)
		RTMPSetKeyParameter("HT_BADecline", tmpbuf, len, final, TRUE);

	/*HT_AutoBA*/
	if (RTMPGetKeyParameter("HT_AutoBA", tmpbuf, len, buf2, TRUE) == TRUE)
		RTMPSetKeyParameter("HT_AutoBA", tmpbuf, len, final, TRUE);

	/*HT_BAWinSize*/
	if (RTMPGetKeyParameter("HT_BAWinSize", tmpbuf, len, buf2, TRUE) == TRUE)
		RTMPSetKeyParameter("HT_BAWinSize", tmpbuf, len, final, TRUE);

	/*BeaconPeriod*/
	if (RTMPGetKeyParameter("BeaconPeriod", tmpbuf, len, buf2, TRUE) == TRUE)
		RTMPSetKeyParameter("BeaconPeriod", tmpbuf, len, final, TRUE);

	/*DtimPeriod*/
	if (RTMPGetKeyParameter("DtimPeriod", tmpbuf, len, buf2, TRUE) == TRUE)
		RTMPSetKeyParameter("DtimPeriod", tmpbuf, len, final, TRUE);

	/*TxBurst*/
	if (RTMPGetKeyParameter("TxBurst", tmpbuf, len, buf2, TRUE) == TRUE)
		RTMPSetKeyParameter("TxBurst", tmpbuf, len, final, TRUE);

	/*PktAggregate*/
	if (RTMPGetKeyParameter("PktAggregate", tmpbuf, len, buf2, TRUE) == TRUE)
		RTMPSetKeyParameter("PktAggregate", tmpbuf, len, final, TRUE);

	/*VOW_WATF_Enable*/
	if (RTMPGetKeyParameter("VOW_WATF_Enable", tmpbuf, len, buf2, TRUE) == TRUE)
		RTMPSetKeyParameter("VOW_WATF_Enable", tmpbuf, len, final, TRUE);

	/*VOW_WATF_MAC_LV1*/
	if (RTMPGetKeyParameter("VOW_WATF_MAC_LV1", tmpbuf, len, buf2, TRUE) == TRUE)
		RTMPSetKeyParameter("VOW_WATF_MAC_LV1", tmpbuf, len, final, TRUE);

	/*VOW_WATF_MAC_LV2*/
	if (RTMPGetKeyParameter("VOW_WATF_MAC_LV2", tmpbuf, len, buf2, TRUE) == TRUE)
		RTMPSetKeyParameter("VOW_WATF_MAC_LV2", tmpbuf, len, final, TRUE);

	/*VOW_WATF_MAC_LV3*/
	if (RTMPGetKeyParameter("VOW_WATF_MAC_LV3", tmpbuf, len, buf2, TRUE) == TRUE)
		RTMPSetKeyParameter("VOW_WATF_MAC_LV3", tmpbuf, len, final, TRUE);

	/*VOW_WATF_MAC_LV4*/
	if (RTMPGetKeyParameter("VOW_WATF_MAC_LV4", tmpbuf, len, buf2, TRUE) == TRUE)
		RTMPSetKeyParameter("VOW_WATF_MAC_LV4", tmpbuf, len, final, TRUE);

	/*VOW_Airtime_Fairness_En*/
	if (RTMPGetKeyParameter("VOW_Airtime_Fairness_En", tmpbuf, len, buf2, TRUE) == TRUE)
		RTMPSetKeyParameter("VOW_Airtime_Fairness_En", tmpbuf, len, final, TRUE);

	/*VOW_BW_Ctrl*/
	if (RTMPGetKeyParameter("VOW_BW_Ctrl", tmpbuf, len, buf2, TRUE) == TRUE)
		RTMPSetKeyParameter("VOW_BW_Ctrl", tmpbuf, len, final, TRUE);

	/* VOW_RX_En */
	if (RTMPGetKeyParameter("VOW_RX_En", tmpbuf, len, buf2, TRUE) == TRUE)
		RTMPSetKeyParameter("VOW_RX_En", tmpbuf, len, final, TRUE);

	/*RED_Enable*/
	if (RTMPGetKeyParameter("RED_Enable", tmpbuf, len, buf2, TRUE) == TRUE)
		RTMPSetKeyParameter("RED_Enable", tmpbuf, len, final, TRUE);

	return NDIS_STATUS_SUCCESS;
}
#endif

#ifdef IGMP_SNOOP_SUPPORT
/*
* merge igmp related
*/
static INT multi_profile_merge_igmp(CHAR *buf1, CHAR *buf2, CHAR *final)
{
	/*merge VHT_BW*/
	multi_profile_merge_separate("IgmpSnEnable", buf1, buf2, final);
	return NDIS_STATUS_SUCCESS;
}
#endif /* IGMP_SNOOP_SUPPORT */

/*
*
*/
static INT multi_profile_merge_dbdc_mode(CHAR *buf1, CHAR *buf2, CHAR *final)
{
#ifdef DEFAULT_5G_PROFILE
	UCHAR tmpbuf[25] = "";
	UCHAR dbdc_mode = 0;

	/*check dbdc mode is enable*/
	if (RTMPGetKeyParameter("DBDC_MODE", tmpbuf, 25, buf1, TRUE)) {
		dbdc_mode = (UCHAR) simple_strtol(tmpbuf, 0, 10);

		if (!dbdc_mode) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("DBDC_MODE is not enable! Not need to merge.\n"));
			goto buf2_check;
		}

		return NDIS_STATUS_SUCCESS;
	}

buf2_check:

	if (RTMPGetKeyParameter("DBDC_MODE", tmpbuf, 25, buf2, TRUE)) {
		dbdc_mode = (UCHAR) simple_strtol(tmpbuf, 0, 10);

		if (!dbdc_mode) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("DBDC_MODE is not enable! Not need to merge.\n"));
			return NDIS_STATUS_FAILURE;
		}
	}

	return NDIS_STATUS_SUCCESS;
#else
	UCHAR tmpbuf[25] = "";
	UCHAR dbdc_mode = 0;

	/*check dbdc mode is enable*/
	if (RTMPGetKeyParameter("DBDC_MODE", tmpbuf, 25, buf1, TRUE)) {
		dbdc_mode = (UCHAR) simple_strtol(tmpbuf, 0, 10);

		if (!dbdc_mode) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("DBDC_MODE is not enable! Not need to merge.\n"));
			return NDIS_STATUS_FAILURE;
		}
	}

	return NDIS_STATUS_SUCCESS;
#endif
}

/*
*  TXPOWER merge function for multiple profile mode
*/
static INT multi_profile_merge_txpwr(
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
#ifdef SINGLE_SKU_V2
	/*merge SKUenable*/
	multi_profile_merge_separate("SKUenable", buf1, buf2, final);
#endif /* SINGLE_SKU_V2 */
	/*merge PERCENTAGEenable*/
	multi_profile_merge_separate("PERCENTAGEenable", buf1, buf2, final);
	/*merge BFBACKOFFenable*/
	multi_profile_merge_separate("BFBACKOFFenable", buf1, buf2, final);
	/*merge TxPower*/
	multi_profile_merge_separate("TxPower", buf1, buf2, final);

#ifdef TX_POWER_CONTROL_SUPPORT
	/*merge Tx Power Boost Table*/
	multi_profile_merge_separate("PowerUpCckOfdm", buf1, buf2, final);
	multi_profile_merge_separate("PowerUpHT20", buf1, buf2, final);
	multi_profile_merge_separate("PowerUpHT40", buf1, buf2, final);
	multi_profile_merge_separate("PowerUpVHT20", buf1, buf2, final);
	multi_profile_merge_separate("PowerUpVHT40", buf1, buf2, final);
	multi_profile_merge_separate("PowerUpVHT80", buf1, buf2, final);
	multi_profile_merge_separate("PowerUpVHT160", buf1, buf2, final);
#endif /* TX_POWER_CONTROL_SUPPORT */

	return NDIS_STATUS_SUCCESS;
}

#ifndef MBSS_SUPPORT
#ifdef TXBF_SUPPORT
/*
*  TXBF merge function for multiple profile mode
*/
static INT multi_profile_merge_txbf(
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	/* merge ETxBfEnCond*/
	multi_profile_merge_separate("ETxBfEnCond", buf1, buf2, final);
	/* merge ITxBfEn */
	multi_profile_merge_separate("ITxBfEn", buf1, buf2, final);

	return NDIS_STATUS_SUCCESS;
}
#endif /* TXBF_SUPPORT */
#endif /* n MBSS_SUPPORT */



#ifdef CONFIG_AP_SUPPORT
#ifdef DOT11R_FT_SUPPORT
static INT multi_profile_merge_ft(
	struct mpf_data *data,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	/*merge FtMdId*/
	multi_profile_merge_increase(data, 1, "FtMdId", buf1, buf2, final);
	/*merge FtSupport */
	multi_profile_merge_separate("FtSupport", buf1, buf2, final);
	return NDIS_STATUS_SUCCESS;
}
#endif
#endif
#ifdef DOT11K_RRM_SUPPORT
static INT multi_profile_merge_rrm(
	struct mpf_data *data,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	/*merge FtSupport */
	multi_profile_merge_separate("RRMEnable", buf1, buf2, final);
	return NDIS_STATUS_SUCCESS;
}
#endif

#ifdef CONFIG_DOT11V_WNM
static INT multi_profile_merge_wnm(
	struct mpf_data *data,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	/*merge WNMEnable */
	multi_profile_merge_separate("WNMEnable", buf1, buf2, final);
	return NDIS_STATUS_SUCCESS;
}
#endif


#ifdef DSCP_PRI_SUPPORT
INT multi_profile_merge_dscp_pri(
	struct mpf_data *data,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	INT8 i = 0;
	CHAR tok_str[TEMP_STR_SIZE] = "";
	CHAR tmpbuf[TEMP_STR_SIZE] = "";

	if (!buf2)
		return NDIS_STATUS_FAILURE;

	for (i = 0; i < data->pf2_num; i++) {
		snprintf(tok_str, sizeof(tok_str), "DscpPriMapBss%d", i);
		if (RTMPGetKeyParameter(tok_str, tmpbuf, TEMP_STR_SIZE, buf2, TRUE)) {
			snprintf(tok_str, sizeof(tok_str), "DscpPriMapBss%d", (data->pf1_num + i));
			RTMPSetKeyParameter(tok_str, tmpbuf, TEMP_STR_SIZE, final, TRUE);
		} else {
			snprintf(tok_str, sizeof(tok_str), "DscpPriMapBss%d", (data->pf1_num + i));
			RTMPSetKeyParameter(tok_str, "", TEMP_STR_SIZE, final, TRUE);
		}
	}

	return NDIS_STATUS_SUCCESS;
}
#endif

/*
* set second profile and merge it.
*/
static INT multi_profile_merge(
	struct _RTMP_ADAPTER *ad,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	INT retval = NDIS_STATUS_FAILURE;
	struct mpf_data *data = NULL;

	if (multi_profile_merge_dbdc_mode(buf1, buf2, final) != NDIS_STATUS_SUCCESS)
		return retval;

	/*create mpf_data*/
	os_alloc_mem(ad, (UCHAR **)&data, sizeof(struct mpf_data));

	if (!data)
		return retval;

	ad->multi_pf_ctrl = data;
	/*first copy buf1 to final*/
	os_move_mem(final, buf1, MAX_INI_BUFFER_SIZE);
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(ad) {
		/*merge BssidNum*/
		if (multi_profile_merge_bssidnum(data, buf1, buf2, final) != NDIS_STATUS_SUCCESS)
			return retval;

#ifdef MBSS_SUPPORT

		if (multi_profile_merge_mbss(data, buf1, buf2, final) != NDIS_STATUS_SUCCESS)
			return retval;

#endif /*MBSS_SUPPORT*/

		if (multi_profile_merge_edcca(buf1, buf2, final) != NDIS_STATUS_SUCCESS)
			return retval;

#ifdef APCLI_SUPPORT

		if (multi_profile_merge_apcli(buf1, buf2, final) != NDIS_STATUS_SUCCESS)
			return retval;

#endif /*APCLI_SUPPORT*/
#ifdef BAND_STEERING
		if (multi_profile_merge_bandsteering(data, buf1, buf2, final) != NDIS_STATUS_SUCCESS)
			return retval;
#endif /* BAND_STEERING */
	}
#endif /*CONFIG_AP_SUPPORT*/

	if (multi_profile_merge_gi(data, buf1, buf2, final) != NDIS_STATUS_SUCCESS)
		return retval;

	if (multi_profile_merge_protection(data, buf1, buf2, final) != NDIS_STATUS_SUCCESS)
		return retval;

	if (multi_profile_merge_frag(data, buf1, buf2, final) != NDIS_STATUS_SUCCESS)
		return retval;

	if (multi_profile_merge_mpdu_density(data, buf1, buf2, final) != NDIS_STATUS_SUCCESS)
		return retval;

	if (multi_profile_merge_ht_mode(data, buf1, buf2, final) != NDIS_STATUS_SUCCESS)
		return retval;

	if (multi_profile_merge_5g_only(data, buf1, buf2, final) != NDIS_STATUS_SUCCESS)
		return retval;
#ifdef DEFAULT_5G_PROFILE

	if (multi_profile_merge_2g_only(buf1, buf2, final) != NDIS_STATUS_SUCCESS)
		return retval;

	/* will remove global setting from 2G profile after UI 5G default is ready */
	if (multi_profile_merge_global_setting_only(buf1, buf2, final) != NDIS_STATUS_SUCCESS)
		return retval;

#endif
#ifdef IGMP_SNOOP_SUPPORT

	if (multi_profile_merge_igmp(buf1, buf2, final) != NDIS_STATUS_SUCCESS)
		return retval;

#endif /* IGMP_SNOOP_SUPPORT */

	if (multi_profile_merge_txpwr(buf1, buf2, final) != NDIS_STATUS_SUCCESS)
		return retval;

#ifndef MBSS_SUPPORT
#ifdef TXBF_SUPPORT
	if (multi_profile_merge_txbf(buf1, buf2, final) != NDIS_STATUS_SUCCESS)
		return retval;
#endif /* TXBF_SUPPORT */
#endif



#ifdef CONFIG_AP_SUPPORT
#ifdef DOT11R_FT_SUPPORT
	if (multi_profile_merge_ft(data, buf1, buf2, final) != NDIS_STATUS_SUCCESS)
		return retval;
#endif
#endif

#ifdef DOT11K_RRM_SUPPORT
	if (multi_profile_merge_rrm(data, buf1, buf2, final) != NDIS_STATUS_SUCCESS)
		return retval;
#endif

#ifdef CONFIG_DOT11V_WNM
	if (multi_profile_merge_wnm(data, buf1, buf2, final) != NDIS_STATUS_SUCCESS)
		return retval;
#endif


#ifdef DSCP_PRI_SUPPORT
	if (multi_profile_merge_dscp_pri(data, buf1, buf2, final) != NDIS_STATUS_SUCCESS)
		return retval;
#endif

	data->enable = TRUE;
	/*adjust specific device name*/
	data->specific_dname = TRUE;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, (
				 "multi-profile merge success, en:%d,pf1_num:%d,pf2_num:%d,total:%d\n",
				 data->enable,
				 data->pf1_num,
				 data->pf2_num,
				 data->total_num));
	return NDIS_STATUS_SUCCESS;
}


/*Global function body*/
INT multi_profile_check(struct _RTMP_ADAPTER *ad, CHAR *final)
{
	ULONG buf_size = MAX_INI_BUFFER_SIZE;
	CHAR *buf1 = NULL;
	CHAR *buf2 = NULL;
	INT retval = NDIS_STATUS_FAILURE;
	UCHAR *fname = NULL;
	/*open first profile file*/
	os_alloc_mem(ad, (UCHAR **)&buf1, buf_size);

	if (!buf1)
		goto end;

#ifdef DEFAULT_5G_PROFILE
	fname = multi_profile_fname_get(ad, MTB_5G_PROFILE);
#else
	fname = multi_profile_fname_get(ad, MTB_2G_PROFILE);
#endif

	if (multi_profile_read(fname, buf1) != NDIS_STATUS_SUCCESS)
		goto end1;

	/*open second profile file*/
	os_alloc_mem(ad, (UCHAR **)&buf2, buf_size);

	if (!buf2)
		goto end1;

#ifdef DEFAULT_5G_PROFILE
	fname = multi_profile_fname_get(ad, MTB_2G_PROFILE);
#else
	fname = multi_profile_fname_get(ad, MTB_5G_PROFILE);
#endif

	if (multi_profile_read(fname, buf2) != NDIS_STATUS_SUCCESS)
		goto end2;

	/*merge it*/
	if (multi_profile_merge(ad, buf1, buf2, final) != NDIS_STATUS_SUCCESS)
		goto end2;

	fname = multi_profile_fname_get(ad, MTB_MERGE_PROFILE);
	multi_profile_write(fname, final);
	retval = NDIS_STATUS_SUCCESS;
end2:
	os_free_mem(buf2);
end1:
	os_free_mem(buf1);
end:
	return retval;
}

/*
*
*/
INT	multi_profile_devname_req(struct _RTMP_ADAPTER *ad, UCHAR *final_name, UCHAR *ifidx)
{
	UCHAR *dev_name;
	struct mpf_data *data;

	if (!ad->multi_pf_ctrl)
		return NDIS_STATUS_SUCCESS;

	data = (struct mpf_data *) ad->multi_pf_ctrl;

	if (!data->enable || !data->specific_dname)
		return NDIS_STATUS_SUCCESS;

	if (*ifidx >= data->pf1_num) {
		if (*ifidx == data->pf1_num)
			dev_name = get_dbdcdev_name_prefix(ad, INT_MAIN);
		else
			dev_name = get_dbdcdev_name_prefix(ad, INT_MBSSID);

		snprintf(final_name, IFNAMSIZ, "%s", dev_name);
		*ifidx -= data->pf1_num;
	}

	return NDIS_STATUS_SUCCESS;
}

INT	multi_profile_apcli_devname_req(struct _RTMP_ADAPTER *ad, UCHAR *final_name, INT *ifidx)
{
	struct mpf_data *data;

	if (!ad->multi_pf_ctrl)
		return NDIS_STATUS_SUCCESS;

	data = (struct mpf_data *) ad->multi_pf_ctrl;

	if (!data->enable || !data->specific_dname)
		return NDIS_STATUS_SUCCESS;

	if (*ifidx == 1) {
		/* apcli1 is 2.4G, name is apclix0*/
		snprintf(final_name, IFNAMSIZ, "%s", get_dbdcdev_name_prefix(ad, INT_APCLI));
	}

	return NDIS_STATUS_SUCCESS;
}

#ifdef DSCP_QOS_MAP_SUPPORT
INT multi_profile_get_bss_num(struct _RTMP_ADAPTER *ad, UINT8 profile_num)
{
	struct mpf_data *data;

	if (!ad->multi_pf_ctrl)
		return 0;

	data = (struct mpf_data *) ad->multi_pf_ctrl;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
	("MultiProfile profile1 BssNum %d for profile2 BssNum %d \n",
			data->pf1_num, data->pf2_num));
	if (profile_num == 1)
		return data->pf1_num;
	else
		return data->pf2_num;
}
#endif
/*
*
*/
VOID multi_profile_exit(struct _RTMP_ADAPTER *ad)
{
	if (ad->multi_pf_ctrl)
		os_free_mem(ad->multi_pf_ctrl);

	ad->multi_pf_ctrl = NULL;
}

/*
*
*/
UCHAR is_multi_profile_enable(struct _RTMP_ADAPTER *ad)
{
	struct mpf_data *data;

	if (!ad->multi_pf_ctrl)
		return FALSE;

	data = (struct mpf_data *) ad->multi_pf_ctrl;
	return data->enable;
}

/*
*
*/
UCHAR multi_profile_get_pf1_num(struct _RTMP_ADAPTER *ad)
{
	struct mpf_data *data;

	if (!ad->multi_pf_ctrl)
		return 0;

	data = (struct mpf_data *) ad->multi_pf_ctrl;
	return data->pf1_num;
}

/*
*
*/
UCHAR multi_profile_get_pf2_num(struct _RTMP_ADAPTER *ad)
{
	struct mpf_data *data;

	if (!ad->multi_pf_ctrl)
		return 0;

	data = (struct mpf_data *) ad->multi_pf_ctrl;
	return data->pf2_num;
}

#endif /*MULTI_PROFILE*/
