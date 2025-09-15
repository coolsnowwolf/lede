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
	misc_app.c
*/

#include "rt_config.h"
#include "misc_app.h"

#ifndef _LINUX_BITOPS_H
#define BIT(n)                          ((UINT32) 1 << (n))
#endif /* BIT */

static enum wifi_cap_operation_status wifi_chip_cap_list_register(struct _RTMP_ADAPTER *ad)
{
	int dbdc_cap = WIFI_CAP_DBDC_SUP_MODE_NONE;
	int channel_cap = WIFI_CAP_CH_SUP_MODE_2G;
	struct wifi_feature_support_list_query *wifi_cap = &ad->wifi_cap_list;
	UINT32	*chip_cap_list = &wifi_cap->chip_cap_list;
	UINT32	local_cap = 0;
	struct _RTMP_CHIP_CAP *chip_cap = hc_get_chip_cap(ad->hdev_ctrl);

	if (chip_cap->phy_caps & fPHY_CAP_HT)
		local_cap |= BIT(WIFI_CAP_HT_MODE_BITS);

	if (chip_cap->phy_caps & fPHY_CAP_VHT)
		local_cap |= BIT(WIFI_CAP_VHT_MODE_BITS);

	if (chip_cap->phy_caps & fPHY_CAP_HE)
		local_cap |= BIT(WIFI_CAP_HE_MODE_BITS);

	if (chip_cap->asic_caps & fASIC_CAP_DBDC) {
		if (BOARD_IS_5G_ONLY(ad))
			dbdc_cap = WIFI_CAP_DBDC_SUP_MODE_5G5G;
		else
			dbdc_cap = WIFI_CAP_DBDC_SUP_MODE_2G5G;
	}
	local_cap |= (dbdc_cap << WIFI_CAP_DBDC_SUP_MODE_BITS);

	if (RFIC_IS_5G_BAND(ad))
		channel_cap = WIFI_CAP_CH_SUP_MODE_2G5G;
	else if (BOARD_IS_5G_ONLY(ad))
		channel_cap = WIFI_CAP_CH_SUP_MODE_5G;
	local_cap |= (channel_cap << WIFI_CAP_CH_SUP_MODE_BITS);

	if (chip_cap->asic_caps & fASIC_CAP_ADV_SECURITY)
		local_cap |= BIT(WIFI_CAP_ADV_SEC_SUP_BITS);

	if (chip_cap->phy_caps & fPHY_CAP_TXBF)
		local_cap |= BIT(WIFI_CAP_BF_SUP_BITS);

	if (chip_cap->phy_caps & fPHY_CAP_MUMIMO)
		local_cap |= BIT(WIFI_CAP_MU_MIMO_SUP_BITS);

#ifdef CONFIG_AP_SUPPORT
	local_cap |= (ad->ApCfg.BssidNum << WIFI_CAP_MBSS_NUM_BITS);
#endif

	local_cap |= (chip_cap->max_nss << WIFI_CAP_TX_STR_NUM_BITS);
	local_cap |= (chip_cap->max_nss << WIFI_CAP_RX_STR_NUM_BITS);

	os_move_mem(chip_cap_list, &local_cap, sizeof(UINT32));

	return WIFI_CAP_STATUS_OK;
}

static enum wifi_cap_operation_status wifi_sec_list_register(struct _RTMP_ADAPTER *ad)
{
	struct wifi_feature_support_list_query *wifi_cap = &ad->wifi_cap_list;
	UINT32	*sec_cap_list = &wifi_cap->sec_cap_list;
	UINT32	local_cap = 0;

	local_cap |= (BIT(WIFI_SEC_WEP_SUP_BITS) |
		      BIT(WIFI_SEC_WPA1PSK_SUP_BITS) |
		      BIT(WIFI_SEC_WPA2PSK_SUP_BITS));


#ifdef DOT11W_PMF_SUPPORT
	local_cap |= BIT(WIFI_SEC_PMF_SUP_BITS);
#endif

#ifdef DOT1X_SUPPORT
	local_cap |= (BIT(WIFI_SEC_WPA1_SUP_BITS) | BIT(WIFI_SEC_WPA2_SUP_BITS));
#endif

#if defined(DOT11_SAE_SUPPORT) && defined(DOT11W_PMF_SUPPORT)
	local_cap |= BIT(WIFI_SEC_WPA3PSK_SUP_BITS);
#if defined(DOT1X_SUPPORT)
	local_cap |= BIT(WIFI_SEC_WPA3_SUP_BITS);
#if defined(DOT11_SUITEB_SUPPORT)
	local_cap |= BIT(WIFI_SEC_WPA3SUITEB192_SUP_BITS);
#endif /*DOT11_SUITEB_SUPPORT*/
#endif /*DOT1X_SUPPORT*/
#endif /*DOT11_SAE_SUPPORT && DOT11W_PMF_SUPPORT*/

#ifdef CONFIG_OWE_SUPPORT
	local_cap |= BIT(WIFI_SEC_ENHANCE_OPEN_SUP_BITS);
#endif

	os_move_mem(sec_cap_list, &local_cap, sizeof(UINT32));

	return WIFI_CAP_STATUS_OK;
}

static enum wifi_cap_operation_status wifi_feature_list_register(struct _RTMP_ADAPTER *ad)
{
	return WIFI_CAP_STATUS_OK;
}

enum wifi_cap_operation_status wifi_sup_list_register(struct _RTMP_ADAPTER *ad,
						      enum wifi_cap_type cap_type)
{
	enum wifi_cap_operation_status ret = WIFI_CAP_STATUS_OK;

	switch (cap_type) {
	case WIFI_CAP_CHIP:
		ret = wifi_chip_cap_list_register(ad);
		break;
	case WIFI_CAP_SEC:
		ret = wifi_sec_list_register(ad);
		break;
	case WIFI_CAP_FEATURE:
		ret = wifi_feature_list_register(ad);
		break;
	default:
		ret = WIFI_CAP_STATUS_INVALID_PARAMETER;
	}

	return ret;
}

static void print_wifi_chip_cap(struct _RTMP_ADAPTER *ad)
{
	struct wifi_feature_support_list_query *wifi_cap = &ad->wifi_cap_list;
	UINT32	*chip_cap_list = &wifi_cap->chip_cap_list;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		("%s, chip_cap:0x%08x\n", __func__, *chip_cap_list));
}

static void print_wifi_sec_cap(struct _RTMP_ADAPTER *ad)
{
	struct wifi_feature_support_list_query *wifi_cap = &ad->wifi_cap_list;
	UINT32	*sec_cap_list = &wifi_cap->sec_cap_list;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		("%s, sec_cap:0x%08x\n", __func__, *sec_cap_list));
}

static void print_wifi_feature_cap(struct _RTMP_ADAPTER *ad)
{

}

INT show_wifi_cap_list(struct _RTMP_ADAPTER *ad, char *arg)
{
	print_wifi_chip_cap(ad);
	print_wifi_sec_cap(ad);
	print_wifi_feature_cap(ad);

	return TRUE;
}

