/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of Ralink Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	cmm_ez.c

	Abstract:
	Easy Setup APIs

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
*/
#include "rt_config.h"
#include "easy_setup/ez_mod_hooks.h"

extern ez_driver_ops_t ez_driver_ops_7603;
int EzDebugLevel = DBG_LVL_ERROR;

unsigned long ez_build_beacon_ie(
	void *wdev_obj,
	unsigned char *frame_buf)

{
	struct wifi_dev *wdev = wdev_obj;
	if (wdev->ez_driver_params.ezdev)
	{
		return ez_build_beacon_ie_hook((ez_dev_t *)wdev->ez_driver_params.ezdev, frame_buf);
	} else {
		ASSERT(FAIL);
		return 0;
	} 
}

unsigned long ez_build_probe_request_ie(
	void *wdev_obj,
	unsigned char *frame_buf)
{	
	struct wifi_dev *wdev = wdev_obj;
	if (wdev->ez_driver_params.ezdev)
		return ez_build_probe_request_ie_hook((ez_dev_t *)wdev->ez_driver_params.ezdev, frame_buf);
	
	ASSERT(FAIL);
	return 0;

}

unsigned long ez_build_probe_response_ie(
	void *wdev_obj,
	unsigned char *frame_buf)
{	
	struct wifi_dev *wdev = wdev_obj;
	if (wdev->ez_driver_params.ezdev)
		return ez_build_probe_response_ie_hook((ez_dev_t *)wdev->ez_driver_params.ezdev, frame_buf);
	
	ASSERT(FAIL);
	return 0;
	

}

unsigned long ez_build_auth_request_ie(
	void *wdev_obj,
	unsigned char *peer_addr,
	unsigned char *frame_buf)
{
	struct wifi_dev *wdev = wdev_obj;
	if (wdev->ez_driver_params.ezdev)
		return ez_build_auth_request_ie_hook((ez_dev_t *)wdev->ez_driver_params.ezdev,
			peer_addr, frame_buf);
	return 0;

}

unsigned long ez_build_auth_response_ie(
	void *wdev_obj,
	unsigned char *peer_addr,
	unsigned char *frame_buf)
{
	struct wifi_dev *wdev = wdev_obj;
	if (wdev->ez_driver_params.ezdev)
		return ez_build_auth_response_ie_hook((ez_dev_t *)wdev->ez_driver_params.ezdev,
			peer_addr, frame_buf);
	
	ASSERT(FAIL);
	return 0;
	

}

unsigned long ez_build_assoc_request_ie(
	void *ad_obj,
	void *wdev_obj,
	unsigned char *peer_addr,
	unsigned char *frame_buf,
	unsigned int frame_buf_len)
{
	struct wifi_dev *wdev = wdev_obj;
	if (wdev->ez_driver_params.ezdev)
		return ez_build_assoc_request_ie_hook((ez_dev_t *)wdev->ez_driver_params.ezdev,
			peer_addr, frame_buf, frame_buf_len);

	
	ASSERT(FAIL);
	return 0;
	
}

unsigned long ez_build_assoc_response_ie(
	void *wdev_obj,
	unsigned char *peer_addr,
	unsigned char *ap_gtk,
	unsigned int ap_gtk_len,
	unsigned char *frame_buf)
{
	struct wifi_dev *wdev = wdev_obj;
	if (wdev->ez_driver_params.ezdev)
		return ez_build_assoc_response_ie_hook((ez_dev_t *)wdev->ez_driver_params.ezdev,
		peer_addr, ap_gtk, ap_gtk_len, frame_buf);
	return 0;
}


unsigned char ez_process_probe_request(
	void *ad_obj,
	void *wdev_obj,
	unsigned char *peer_addr,
	void *msg,
	unsigned long msg_len)
{
	struct wifi_dev *wdev = wdev_obj;
	if (wdev->ez_driver_params.ezdev)
		return 	ez_process_probe_request_hook((ez_dev_t *)wdev->ez_driver_params.ezdev,
			peer_addr, msg, msg_len);
	
	ASSERT(FAIL);
	return 0;
}

void ez_process_beacon_probe_response(
	void *wdev_obj,
	void *msg,
	unsigned long msg_len)
{
	struct wifi_dev *wdev = wdev_obj;
	if (wdev->ez_driver_params.ezdev) {
		ez_process_beacon_probe_response_hook((ez_dev_t *)wdev->ez_driver_params.ezdev,
			msg, msg_len);
	} else {
		ASSERT(FAIL);
	}
}

unsigned char ez_process_auth_request(
	void *ad_obj,
	void *wdev_obj,
	void *auth_info_obj,
	void *msg,
	unsigned long msg_len)
{
	struct wifi_dev *wdev = wdev_obj;
	if (wdev->ez_driver_params.ezdev)
		return ez_process_auth_request_hook((ez_dev_t *)wdev->ez_driver_params.ezdev,
			auth_info_obj, msg, msg_len);
	
	ASSERT(FAIL);
	return 0;
}

void ez_process_auth_response(
	void *ad_obj,
	void *wdev_obj,
	unsigned char *peer_addr,
	unsigned long *current_state,
	void *msg,
	unsigned long msg_len)
{
	struct wifi_dev *wdev = wdev_obj;
	PRTMP_ADAPTER ad= ad_obj;
	APCLI_CTRL_MSG_STRUCT ApCliCtrlMsg; 
	if (wdev->ez_driver_params.ezdev) {
		ApCliCtrlMsg.Status = ez_process_auth_response_hook((ez_dev_t *)wdev->ez_driver_params.ezdev,
			peer_addr, msg, msg_len);
		*current_state = APCLI_AUTH_REQ_IDLE;
		MlmeEnqueue(ad, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_AUTH_RSP,
			sizeof(APCLI_CTRL_MSG_STRUCT), &ApCliCtrlMsg, wdev->func_idx);
	} else {		
		ASSERT(FAIL);
	}
}

unsigned short ez_process_assoc_request(
	void *wdev_obj,
	void *entry_obj,
	unsigned char isReassoc,
	void *msg,
	unsigned long msg_len)
{
	struct wifi_dev *wdev = wdev_obj;
	MAC_TABLE_ENTRY *entry = entry_obj;
	if (wdev->ez_driver_params.ezdev)
		return ez_process_assoc_request_hook((ez_dev_t *)wdev->ez_driver_params.ezdev,
			entry->Addr,&entry->easy_setup_mic_valid, isReassoc, msg, msg_len);

	ASSERT(FAIL);	
	return 0;

}


unsigned short ez_process_assoc_response(
	void *wdev_obj,
	unsigned char *peer_addr,
	void *msg,
	unsigned long msg_len)
{
	struct wifi_dev *wdev = wdev_obj;
	if (wdev->ez_driver_params.ezdev)
		return ez_process_assoc_response_hook((ez_dev_t *)wdev->ez_driver_params.ezdev,
			peer_addr, msg, msg_len);

	ASSERT(FAIL);	
	return 0;
	
}

INT Show_EasySetupInfo_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE  pObj;
    UCHAR	    apidx;
	struct wifi_dev *wdev;
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	apidx = pObj->ioctl_if;
#ifdef APCLI_SUPPORT
    if (pObj->ioctl_if_type == INT_APCLI)
    {
		wdev = &pAd->ApCfg.ApCliTab[apidx].wdev;
    }
    else
#endif /* APCLI_SUPPORT */
	{
		wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	}
	if (wdev->ez_driver_params.ezdev)
		ez_show_information_hook((ez_dev_t *)wdev->ez_driver_params.ezdev);
	else 
		return FALSE;
	return TRUE;
}




/*
    ==========================================================================
    Description:
        For Debug information
        Change DebugLevel
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT Set_EasySetup_Debug_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG dbg;
	POS_COOKIE  pObj;
	int apidx;
	struct wifi_dev *wdev;
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	
	apidx = pObj->ioctl_if;
	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;

	dbg = simple_strtol(arg, 0, 10);

	if( dbg <= DBG_LVL_MAX)
	{
		if (wdev->ez_driver_params.ez_ad)
		{
			EZ_ADAPTER *ez_ad = wdev->ez_driver_params.ez_ad;
			ez_ad->debug = dbg;
			EzDebugLevel = dbg;
			MTWF_PRINT("%s(): (EzDebugLevel = %d)\n", __FUNCTION__, ez_ad->debug);
		} else {
			MTWF_PRINT("%s(): Not EZ Interface\n", __FUNCTION__);			
			return FALSE;
		}
	}

	return TRUE;
}


INT Set_EasySetup_RoamTime_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 roam_time;
	POS_COOKIE	pObj;
	int apidx;
	struct wifi_dev *wdev;
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	
	apidx = pObj->ioctl_if;
	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	roam_time = simple_strtol(arg, 0, 10);
	if (roam_time == 0)
	{
		return FALSE;
	}

	if (wdev->ez_driver_params.ez_ad)
	{
		EZ_ADAPTER *ez_ad = wdev->ez_driver_params.ez_ad;
		ez_ad->ez_roam_time = roam_time;			
	} else {
		MTWF_PRINT("%s(): Not EZ Interface\n", __FUNCTION__);			
		return FALSE;
	}
	return TRUE;

}

INT Set_EasySetup_Delay_Disconnect_Count_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	unsigned char delay_disconnect_count;
	POS_COOKIE	pObj;
	int apidx;
	struct wifi_dev *wdev;
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	
	apidx = pObj->ioctl_if;
	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	delay_disconnect_count = simple_strtol(arg, 0, 10);

	if (wdev->ez_driver_params.ez_ad)
	{
		EZ_ADAPTER *ez_ad = wdev->ez_driver_params.ez_ad;
		ez_ad->ez_delay_disconnect_count = delay_disconnect_count;			
		MTWF_PRINT("%s(): (ez_delay_disconnect_count = %d)\n", __FUNCTION__, delay_disconnect_count);
	} else {
		MTWF_PRINT("%s(): Not EZ Interface\n", __FUNCTION__);			
		return FALSE;
	}
	return TRUE;

}

INT Set_EasySetup_Wait_For_Info_Transfer_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 wait_for_info_transfer;
	POS_COOKIE	pObj;
	int apidx;
	struct wifi_dev *wdev;
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	
	apidx = pObj->ioctl_if;
	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	wait_for_info_transfer = simple_strtol(arg, 0, 10);

	if (wdev->ez_driver_params.ez_ad)
	{
		EZ_ADAPTER *ez_ad = wdev->ez_driver_params.ez_ad;
		ez_ad->ez_wait_for_info_transfer = wait_for_info_transfer;			
		MTWF_PRINT("%s(): (ez_wait_for_info_transfer = %d)\n", __FUNCTION__, wait_for_info_transfer);
	} else {
		MTWF_PRINT("%s(): Not EZ Interface\n", __FUNCTION__);			
		return FALSE;
	}
	return TRUE;

}

INT Set_EasySetup_WDL_Missing_Time_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 wdl_missing_time;
	POS_COOKIE	pObj;
	int apidx;
	struct wifi_dev *wdev;
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	
	apidx = pObj->ioctl_if;
	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	wdl_missing_time = simple_strtol(arg, 0, 10);

	if (wdev->ez_driver_params.ez_ad)
	{
		EZ_ADAPTER *ez_ad = wdev->ez_driver_params.ez_ad;
		ez_ad->ez_wdl_missing_time = wdl_missing_time;			
		MTWF_PRINT("%s(): (ez_wdl_missing_time = %d)\n", __FUNCTION__, wdl_missing_time);
	} else {
		MTWF_PRINT("%s(): Not EZ Interface\n", __FUNCTION__);			
		return FALSE;
	}
	return TRUE;

}

INT Set_EasySetup_Force_Connect_Bssid_Time_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 force_connect_bssid_time;
	POS_COOKIE	pObj;
	int apidx;
	struct wifi_dev *wdev;
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	
	apidx = pObj->ioctl_if;
	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	force_connect_bssid_time = simple_strtol(arg, 0, 10);

	if (wdev->ez_driver_params.ez_ad)
	{
		EZ_ADAPTER *ez_ad = wdev->ez_driver_params.ez_ad;
		ez_ad->ez_force_connect_bssid_time = force_connect_bssid_time;			
		MTWF_PRINT("%s(): (ez_wdl_missing_time = %d)\n", __FUNCTION__, force_connect_bssid_time);
	} else {
		MTWF_PRINT("%s(): Not EZ Interface\n", __FUNCTION__);			
		return FALSE;
	}
	return TRUE;

}

INT Set_EasySetup_Peer_Entry_Age_Out_time_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 peer_entry_age_out_time;
	POS_COOKIE	pObj;
	int apidx;
	struct wifi_dev *wdev;
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	
	apidx = pObj->ioctl_if;
	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	peer_entry_age_out_time = simple_strtol(arg, 0, 10);

	if (wdev->ez_driver_params.ez_ad)
	{
		EZ_ADAPTER *ez_ad = wdev->ez_driver_params.ez_ad;
		ez_ad->ez_peer_entry_age_out_time = peer_entry_age_out_time;			
		MTWF_PRINT("%s(): (ez_peer_entry_age_out_time = %d)\n", __FUNCTION__, peer_entry_age_out_time);
	} else {
		MTWF_PRINT("%s(): Not EZ Interface\n", __FUNCTION__);			
		return FALSE;
	}
	return TRUE;

}

INT Set_EasySetup_Scan_Same_Channel_Time_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 scan_same_channel_time;
	POS_COOKIE	pObj;
	int apidx;
	struct wifi_dev *wdev;
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	
	apidx = pObj->ioctl_if;
	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	scan_same_channel_time = simple_strtol(arg, 0, 10);

	if (wdev->ez_driver_params.ez_ad)
	{
		EZ_ADAPTER *ez_ad = wdev->ez_driver_params.ez_ad;
		ez_ad->ez_scan_same_channel_time = scan_same_channel_time;			
		MTWF_PRINT("%s(): (ez_scan_same_channel_time = %d)\n", __FUNCTION__, scan_same_channel_time);
	} else {
		MTWF_PRINT("%s(): Not EZ Interface\n", __FUNCTION__);			
		return FALSE;
	}
	return TRUE;

}

INT Set_EasySetup_Partial_Scan_Time_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 partial_scan_time;
	POS_COOKIE	pObj;
	int apidx;
	struct wifi_dev *wdev;
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	
	apidx = pObj->ioctl_if;
	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	partial_scan_time = simple_strtol(arg, 0, 10);
	
	if (wdev->ez_driver_params.ez_ad)
	{
		EZ_ADAPTER *ez_ad = wdev->ez_driver_params.ez_ad;
		ez_ad->ez_partial_scan_time = partial_scan_time;			
		MTWF_PRINT("%s(): (ez_partial_scan_time = %d)\n", __FUNCTION__, partial_scan_time);
	} else {
		MTWF_PRINT("%s(): Not EZ Interface\n", __FUNCTION__);			
		return FALSE;
	}
	return TRUE;

}

INT ez_send_broadcast_deauth_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg) 
{
	POS_COOKIE	pObj;
	int apidx;
	struct wifi_dev *wdev;
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	
	apidx = pObj->ioctl_if;
	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;

	if (wdev->ez_driver_params.ezdev)
	{
		ez_send_broadcast_deauth_proc_hook((ez_dev_t *)wdev->ez_driver_params.ezdev);
	} else {
		MTWF_PRINT("%s(): Not EZ Interface\n", __FUNCTION__);			
		return FALSE;
	}
	return TRUE;

}

INT Set_EasySetup_Start_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING * arg)
{
	POS_COOKIE  pObj;
	UCHAR	    apidx;
	struct wifi_dev *wdev;
	BOOLEAN ap_mode = FALSE;

	INT ez_enable = simple_strtol(arg, 0, 10);

	if (ez_enable != 0 && ez_enable != 1) {
		EZ_DEBUG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s() invalid ez_enable(%d), valid ez_enable is 0 or 1.\n", __func__, ez_enable));
		return FALSE;
	}

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	apidx = pObj->ioctl_if;

#ifdef APCLI_SUPPORT
	if (pObj->ioctl_if_type == INT_APCLI) {
		if (apidx >= MAX_APCLI_NUM)
			return FALSE;

		wdev = &pAd->ApCfg.ApCliTab[apidx].wdev;
		ap_mode = FALSE;
	} else
#endif /* APCLI_SUPPORT */
	{
		if (apidx >= pAd->ApCfg.BssidNum)
			return FALSE;

		wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
		ap_mode = TRUE;
	}

	if (!IS_CONF_EZ_SETUP_ENABLED(wdev)) {
		EZ_DEBUG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s:easy-setup haven't configured by user yet, can't do this operation.\n", __func__));
		return TRUE;
	}

	if (wdev) {
		if (ez_enable)
			ez_start(wdev, ap_mode);
		else
			ez_stop(wdev);

		EZ_DEBUG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s() set ezstart=%d.\n", __func__, ez_enable));
	}
	return TRUE;
}

INT Set_EasySetup_GroupID_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	POS_COOKIE  pObj;
    UCHAR	    apidx;
	struct wifi_dev *wdev;
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	apidx = pObj->ioctl_if;
#ifdef APCLI_SUPPORT
    if (pObj->ioctl_if_type == INT_APCLI)
    {
		wdev = &pAd->ApCfg.ApCliTab[apidx].wdev;
		EZ_DEBUG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
			("IF(apcli%d) %s:: This command is from apcli interface now.\n", 
			apidx, __FUNCTION__));
    }
    else
#endif /* APCLI_SUPPORT */
	{
		wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
		EZ_DEBUG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
			("IF(ra%d) %s:: This command is from ra interface now.\n",
			apidx, __FUNCTION__));
	}
	if (wdev->ez_driver_params.ezdev)
	{
		ez_set_ezgroup_id_hook((ez_dev_t *)wdev->ez_driver_params.ezdev, arg, strlen(arg), apidx);
		return ez_set_group_id_hook((ez_dev_t *)wdev->ez_driver_params.ezdev, arg, strlen(arg), apidx);
	}
	return FALSE;
}


INT Set_EasySetup_GenGroupID_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	POS_COOKIE  pObj;
    UCHAR	    apidx;
	struct wifi_dev *wdev;
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	apidx = pObj->ioctl_if;
#ifdef APCLI_SUPPORT
    if (pObj->ioctl_if_type == INT_APCLI)
    {
		wdev = &pAd->ApCfg.ApCliTab[apidx].wdev;
		EZ_DEBUG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
			("IF(apcli%d) %s:: This command is from apcli interface now.\n", 
			apidx, __FUNCTION__));
    }
    else
#endif /* APCLI_SUPPORT */
	{
		wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
		EZ_DEBUG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
			("IF(ra%d) %s:: This command is from ra interface now.\n",
			apidx, __FUNCTION__));
	}

	if (wdev->ez_driver_params.ezdev)
	{
		ez_set_gen_group_id_hook((ez_dev_t *)wdev->ez_driver_params.ezdev, arg, strlen(arg), apidx);
		return ez_set_gen_group_id_hook((ez_dev_t *)wdev->ez_driver_params.ezdev, arg, strlen(arg), apidx);
	}
	return FALSE;
}


INT Set_EasySetup_RssiThreshold_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	POS_COOKIE	pObj;
	UCHAR		apidx;
	struct wifi_dev *wdev;
	INT rssi_threshold;
		
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	apidx = pObj->ioctl_if;

	rssi_threshold = simple_strtol(arg, 0, 10);
	
#ifdef APCLI_SUPPORT
	if (pObj->ioctl_if_type == INT_APCLI)
	{
		wdev = &pAd->ApCfg.ApCliTab[apidx].wdev;
		EZ_DEBUG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
			("IF(apcli%d) %s:: This command is from apcli interface now.\n", 
			apidx, __FUNCTION__));
	}
	else
#endif /* APCLI_SUPPORT */
	{
		wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
		EZ_DEBUG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
			("IF(ra%d) %s:: This command is from ra interface now.\n",
			apidx, __FUNCTION__));
	}

	if (wdev->ez_driver_params.ezdev) {
		//ez_set_rssi_threshold_hook((ez_dev_t *)wdev->ez_driver_params.ezdev, (-1)*rssi_threshold);
		ez_dev_t *ezdev = wdev->ez_driver_params.ezdev;
		ezdev->ez_security.rssi_threshold = -1*rssi_threshold;
		EZ_DEBUG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
				("%s:: - wdev->rssi_threshold = %d.\n", 
					__FUNCTION__, ezdev->ez_security.rssi_threshold));

	} else {
		return FALSE;
	}

	return TRUE;
}

INT Set_EasySetup_MaxScanDelay(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	POS_COOKIE  pObj;
    UCHAR	    apidx;
	//struct _ez_security *ez_security;
	UINT32 max_scan_delay;
	struct wifi_dev *wdev;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	apidx = pObj->ioctl_if;
	max_scan_delay = simple_strtol(arg, 0, 10);
#ifdef APCLI_SUPPORT
		if (pObj->ioctl_if_type == INT_APCLI)
		{
			wdev = &pAd->ApCfg.ApCliTab[apidx].wdev;
			EZ_DEBUG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
				("IF(apcli%d) %s:: This command is from apcli interface now.\n", 
				apidx, __FUNCTION__));
		}
		else
#endif /* APCLI_SUPPORT */
		{
			wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
			EZ_DEBUG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
				("IF(ra%d) %s:: This command is from ra interface now.\n",
				apidx, __FUNCTION__));
		}


	if (wdev->ez_driver_params.ezdev) {
		EZ_ADAPTER *ez_ad;
		ez_ad = wdev->ez_driver_params.ez_ad;
		ez_ad->max_scan_delay = max_scan_delay;
		//ez_set_max_scan_delay_hook((ez_dev_t *)wdev->ez_driver_params.ezdev, max_scan_delay);
	} else {
		return FALSE;
	}
	return TRUE;
}

INT set_EasySetup_Api_Mode(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	UCHAR ez_api_mode;
	UCHAR 		*macptr;
	POS_COOKIE  pObj;
    UCHAR	    apidx;
	struct wifi_dev *wdev;
	macptr = arg;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	apidx = pObj->ioctl_if;
	ez_api_mode = simple_strtol(arg, 0, 10);
#ifdef APCLI_SUPPORT
	if (pObj->ioctl_if_type == INT_APCLI)
	{
		wdev = &pAd->ApCfg.ApCliTab[apidx].wdev;
		EZ_DEBUG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
				("IF(apcli%d) %s:: This command is from apcli interface now.\n", 
				apidx, __FUNCTION__));
	}
	else
#endif /* APCLI_SUPPORT */
	{
		wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
		EZ_DEBUG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
			("IF(ra%d) %s:: This command is from ra interface now.\n",
			apidx, __FUNCTION__));
	}

	if (ez_api_mode > 2)
	{
		EZ_DEBUG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
					("%s:: POSSIBLE VALUES 0/1/2, ez_api_mode = %d\n", __FUNCTION__, ez_api_mode));
		return FALSE;
	}

	
	if (wdev->ez_driver_params.ez_ad) {
		EZ_ADAPTER *ez_ad = wdev->ez_driver_params.ez_ad;
		ez_ad->ez_api_mode = ez_api_mode;
		wdev->ez_driver_params.ez_api_mode = ez_api_mode;
	} else {
		return FALSE;
	}
	
	return TRUE;
}


INT set_EasySetup_MergeGroup_proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	POS_COOKIE  pObj;
    UCHAR	    apidx;
	UCHAR 		*macptr;
	int 		j;
	UCHAR 		macAddress[6];
	UCHAR 		broadcast_address_string[] = "FF:FF:FF:FF:FF:FF";
	struct wifi_dev *wdev;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	apidx = pObj->ioctl_if;


#ifdef APCLI_SUPPORT
	if (pObj->ioctl_if_type == INT_APCLI)
	{
		wdev = &pAd->ApCfg.ApCliTab[apidx].wdev;
		EZ_DEBUG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
				("IF(apcli%d) %s:: This command is from apcli interface now.\n", 
				apidx, __FUNCTION__));
	}
	else
#endif /* APCLI_SUPPORT */
	{
		wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
		EZ_DEBUG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
			("IF(ra%d) %s:: This command is from ra interface now.\n",
			apidx, __FUNCTION__));
	}

	macptr = arg;
	if (strlen(macptr) == 0)
	{
		macptr = broadcast_address_string;
	}
	if(strlen(macptr) != 17)  /*Mac address acceptable format 01:02:03:04:05:06 length 17*/
	{
		EZ_DEBUG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
			("Incorrect Peer Address Format %s, expected is XX:XX:XX:XX:XX:XX.\n", 
			macptr));
		return FALSE;
	}
	if(strcmp(macptr,"00:00:00:00:00:00") == 0)
	{
		EZ_DEBUG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
			("Incorrect Peer Address Format %s, expected is XX:XX:XX:XX:XX:XX. All zeros not allowed\n", 
			macptr));
		return FALSE;
	}
	for (j=0; j<MAC_ADDR_LEN; j++)
	{
		AtoH(macptr, &macAddress[j], 1);
		if (macptr[2] != ':' && j < 5)
			{
				EZ_DEBUG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
				("Incorrect Peer Address Format %s, expected is XX:XX:XX:XX:XX:XX.\n", 
					macptr));
				return FALSE;
			}
		macptr=macptr+3;
	}
	EZ_DEBUG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_OFF,("%x:%x:%x:%x:%x:%x\n", macAddress[0],macAddress[1],macAddress[2], macAddress[3],macAddress[4],macAddress[5]));

	if (wdev->ez_driver_params.ezdev)
		return ez_merge_group_hook((ez_dev_t *)wdev->ez_driver_params.ezdev, macAddress);
	else 
		return FALSE;
}


INT Set_EasySetup_ForceSsid_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	POS_COOKIE pObj;
	UCHAR apidx;
	INT success = FALSE;
	struct wifi_dev *wdev;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	if (pObj->ioctl_if_type != INT_APCLI)
		return FALSE;

	apidx = pObj->ioctl_if;

#ifdef APCLI_SUPPORT
	if (pObj->ioctl_if_type == INT_APCLI)
	{
		wdev = &pAd->ApCfg.ApCliTab[apidx].wdev;
		EZ_DEBUG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
				("IF(apcli%d) %s:: This command is from apcli interface now.\n", 
				apidx, __FUNCTION__));
	}
	else
#endif /* APCLI_SUPPORT */
	{
		wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
		EZ_DEBUG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
			("IF(ra%d) %s:: This command is from ra interface now.\n",
			apidx, __FUNCTION__));
	}

	if (wdev->ez_driver_params.ezdev) {
		if(strlen(arg) <= MAX_LEN_OF_SSID)
		{
			ez_dev_t *ezdev = wdev->ez_driver_params.ezdev;
			//ez_apcli_force_ssid_hook((ez_dev_t *)wdev->ez_driver_params.ezdev,(PUCHAR)arg,strlen(arg));
			{
				NdisZeroMemory(ezdev->ez_security.ez_apcli_force_ssid,MAX_LEN_OF_SSID);
				NdisCopyMemory(ezdev->ez_security.ez_apcli_force_ssid,(PUCHAR)arg,strlen(arg));
				ezdev->ez_security.ez_apcli_force_ssid_len = strlen(arg);
			}
		}
		else
		success = FALSE;
	} else {
		success = FALSE;
	}
	return success;
}


INT Set_EasySetup_ForceBssid_Proc(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING *arg)
{
	INT i;
	RTMP_STRING *value;
	UCHAR apidx;
	POS_COOKIE pObj;
	struct wifi_dev *wdev;
	unsigned char forced_bssid[6];

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	if (pObj->ioctl_if_type != INT_APCLI)
		return FALSE;

	apidx = pObj->ioctl_if;

#ifdef APCLI_SUPPORT
	if (pObj->ioctl_if_type == INT_APCLI)
	{
		wdev = &pAd->ApCfg.ApCliTab[apidx].wdev;
	}
	else
#endif /* APCLI_SUPPORT */
	{
		wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	}


	if(strlen(arg) == 17)  /* Mac address acceptable format 01:02:03:04:05:06 length 17 */
	{
		for (i=0, value = rstrtok(arg,":"); value; value = rstrtok(NULL,":"), i++)
		{
			if((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value+1))) )
				return FALSE;  /* Invalid */

			AtoH(value, &forced_bssid[i], 1);
		}

		if(i != 6)
			return FALSE;  /* Invalid */
	}

	if (wdev->ez_driver_params.ezdev){
		ez_dev_t *ezdev = wdev->ez_driver_params.ezdev;
		NdisCopyMemory(ezdev->ez_security.ez_apcli_force_bssid,
			forced_bssid,MAC_ADDR_LEN);
		//ez_set_force_bssid_hook((ez_dev_t *)wdev->ez_driver_params.ezdev, forced_bssid);
	}
	return TRUE;
}

INT Set_EasySetup_BWPushConfig(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{

	UINT8 same_bw_push;
	//PRTMP_ADAPTER adOthBand = NULL;
	UCHAR apidx;
	POS_COOKIE pObj;
	struct wifi_dev *wdev;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	if (pObj->ioctl_if_type != INT_APCLI)
		return FALSE;

	apidx = pObj->ioctl_if;

#ifdef APCLI_SUPPORT
	if (pObj->ioctl_if_type == INT_APCLI)
	{
		wdev = &pAd->ApCfg.ApCliTab[apidx].wdev;
	}
	else
#endif /* APCLI_SUPPORT */
	{
		wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	}

	same_bw_push = simple_strtol(arg, 0, 10);

	if (wdev->ez_driver_params.ez_ad) {
		EZ_ADAPTER *ez_ad = wdev->ez_driver_params.ez_ad;
		ez_ad->push_bw_config = same_bw_push;
		//ez_set_push_bw_hook((ez_dev_t *)wdev->ez_driver_params.ezdev, same_bw_push);
		return TRUE;
	} else {
		return FALSE;
	}

	return TRUE;
}

void ez_handle_action_txstatus(void *ad_obj, unsigned int wcid)
{
	MAC_TABLE_ENTRY	*pEntry;
	RTMP_ADAPTER *pAd = ad_obj;

	pEntry = &pAd->MacTab.Content[wcid];
	EZ_DEBUG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
                ("%s(): before calling deauth  for Dev = %02x-%02x-%02x-%02x-%02x-%02x\n", __FUNCTION__, 
                pEntry->Addr[0],pEntry->Addr[1],pEntry->Addr[2],pEntry->Addr[3],pEntry->Addr[4],pEntry->Addr[5]));

	if (pEntry->wdev->ez_driver_params.ezdev)
		ez_handle_action_txstatus_hook((ez_dev_t *)pEntry->wdev->ez_driver_params.ezdev, pEntry->Addr);
	
}



//! Leverage form MP.1.0 CL 170364
INT Set_EasySetup_ssid_psk_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	char *value;
	char ssid[MAX_LEN_OF_SSID * 3 + 3];
	char psk[(LEN_PSK * 3) + 3];
	char EncrypType[32];
	char AuthMode[32];
	char ssid1[MAX_LEN_OF_SSID], ssid2[MAX_LEN_OF_SSID], ssid3[MAX_LEN_OF_SSID];
	char psk1[LEN_PSK + 1], psk2[LEN_PSK + 1], psk3[LEN_PSK + 1];
	char pmk1[LEN_PMK], pmk2[LEN_PMK], pmk3[LEN_PMK];
	char EncrypType1[32], EncrypType2[32];
	char AuthMode1[32], AuthMode2[32];
	int i;
	struct wifi_dev *wdev;
	
	wdev = &pAd->ApCfg.MBSSID[0].wdev;

	memset(ssid, '\0', sizeof(ssid));
	NdisZeroMemory(psk, sizeof(psk));
	memset(EncrypType, '\0', sizeof(EncrypType));
	memset(AuthMode, '\0', sizeof(AuthMode));	

	memset(ssid1, '\0', MAX_LEN_OF_SSID);
	memset(ssid2, '\0', MAX_LEN_OF_SSID);
	memset(ssid3, '\0', MAX_LEN_OF_SSID);

	memset(psk1, '\0',LEN_PSK);
	memset(psk2, '\0', LEN_PSK);
	memset(psk3, '\0', LEN_PSK);

	memset(EncrypType1, '\0', sizeof(EncrypType1));
	memset(EncrypType2, '\0', sizeof(EncrypType2));

	memset(AuthMode1, '\0', sizeof(AuthMode1));
	memset(AuthMode2, '\0', sizeof(AuthMode2));
					
	for (i=0, value = rstrtok(arg,";"); value; value = rstrtok(NULL,";"), i++)
	{
		if (i == 0)
		{
			//NdisZeroMemory(ssid,MAX_LEN_OF_SSID);
			memset(ssid, '\0', MAX_LEN_OF_SSID);
			NdisCopyMemory(ssid,value,strlen(value));
			ssid[strlen(value)] = '\0';
		} 
		else if(i==1) 
		{
			NdisZeroMemory(psk,LEN_PSK + 1);
			NdisCopyMemory(psk, value,strlen(value));
			psk[strlen(value)] = '\0';
		}
		else if(i == 2) 
		{
			//NdisZeroMemory(EncrypType, 15);
			NdisCopyMemory(EncrypType, value,strlen(value));
			EncrypType[strlen(value)] = '\0';
		}
		else if(i == 3) 
		{
			//NdisZeroMemory(AuthMode, 27);
			NdisCopyMemory(AuthMode, value,strlen(value));
			AuthMode[strlen(value)] = '\0';
		}
		
		if (i > 3)
		{
			break;
		}
	}
//	ssid_len = strlen(ssid);
	EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
	("New SSID = %s\n SSID_LEN = %d\n New PSK = %s\n", ssid, (int)strlen(ssid),psk));

	for (i=0, value = rstrtok(ssid,":"); value; value = rstrtok(NULL,":"), i++)
	{
		if (i == 0)
		{
			memset(ssid1, '\0', MAX_LEN_OF_SSID);
			NdisCopyMemory(ssid1,value,strlen(value));
		} else if (i == 1){
			memset(ssid2, '\0', MAX_LEN_OF_SSID);
			NdisCopyMemory(ssid2,value,strlen(value));
		} else if (i == 2){
			memset(ssid3, '\0', MAX_LEN_OF_SSID);
			NdisCopyMemory(ssid3,value,strlen(value));
		}
	}
	
	for (i=0, value = rstrtok(psk,":"); value; value = rstrtok(NULL,":"), i++)
	{
		if (i == 0)
		{
			NdisZeroMemory(psk1,LEN_PSK + 1);
			NdisCopyMemory(psk1, value,strlen(value));
			psk1[strlen(value)] = '\0';
		} else if (i == 1){
			NdisZeroMemory(psk2,LEN_PSK + 1);
			NdisCopyMemory(psk2, value,strlen(value));
			psk2[strlen(value)] = '\0';
		} else if (i == 2){
			NdisZeroMemory(psk3,LEN_PSK + 1);
			NdisCopyMemory(psk3, value,strlen(value));
			psk3[strlen(value)] = '\0';
		}
	}

	for (i=0, value = rstrtok(EncrypType,":"); value; value = rstrtok(NULL,":"), i++)
	{
		if (i == 0)
		{
			//memset(EncrypType1, '\0', 7);
			NdisCopyMemory(EncrypType1,value,strlen(value));
		} else if (i == 1){
			//memset(EncrypType2, '\0', 7);
			NdisCopyMemory(EncrypType2,value,strlen(value));
		} 
	}

	for (i=0, value = rstrtok(AuthMode,":"); value; value = rstrtok(NULL,":"), i++)
	{
		if (i == 0)
		{
			//memset(AuthMode1, '\0', 7);
			NdisCopyMemory(AuthMode1,value,strlen(value));
		} else if (i == 1){
			//memset(AuthMode2, '\0', 7);
			NdisCopyMemory(AuthMode2,value,strlen(value));
		} 
	}

	//SetWPAPSKKey(pAd, psk, strlen(psk), ssid, strlen(ssid), pmk);
	RT_CfgSetWPAPSKKey(pAd, psk1, strlen(psk1), ssid1, strlen(ssid1), pmk1);
	RT_CfgSetWPAPSKKey(pAd, psk2, strlen(psk2), ssid2, strlen(ssid2), pmk2);	
	RT_CfgSetWPAPSKKey(pAd, psk3, strlen(psk3), ssid3, strlen(ssid3), pmk3);			

	EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("New SSID1 = %s\n SSID_LEN1 = %d\n New PSK1 = %s\n", ssid1, (int)strlen(ssid1),psk1));
	EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("New SSID2 = %s\n SSID_LEN2 = %d\n New PSK2 = %s\n", ssid2, (int)strlen(ssid2),psk2));	
	EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("New SSID3 = %s\n SSID_LEN3 = %d\n New PSK3 = %s\n", ssid3, (int)strlen(ssid3),psk3));	
	EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("New Encr1 = %s\n encr_LEN1 = %d\n New encr2 = %s\n encr_LEN2 = %d\n", EncrypType1, (int)strlen(EncrypType1), EncrypType2, (int)strlen(EncrypType2)));	
	EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("New Encr1 = %s\n encr_LEN1 = %d\n New encr2 = %s\n encr_LEN2 = %d\n", AuthMode1, (int)strlen(AuthMode1), AuthMode2, (int)strlen(AuthMode2)));


	if (wdev->ez_driver_params.ezdev)
	{	
		ez_update_ssid_psk_msg_t update_ssid_psk_msg;
		
		NdisZeroMemory(&update_ssid_psk_msg, sizeof(update_ssid_psk_msg));

		NdisCopyMemory(update_ssid_psk_msg.ssid1,ssid1,strlen(ssid1));
		NdisCopyMemory(update_ssid_psk_msg.ssid2,ssid2,strlen(ssid2));
		NdisCopyMemory(update_ssid_psk_msg.ssid3,ssid3,strlen(ssid3));

		NdisCopyMemory(update_ssid_psk_msg.psk1,psk1,strlen(psk1));
		NdisCopyMemory(update_ssid_psk_msg.psk2,psk2,strlen(psk2));
		NdisCopyMemory(update_ssid_psk_msg.psk3,psk3,strlen(psk3));


		NdisCopyMemory(update_ssid_psk_msg.pmk1,pmk1,sizeof(pmk1));
		NdisCopyMemory(update_ssid_psk_msg.pmk2,pmk2,sizeof(pmk2));
		NdisCopyMemory(update_ssid_psk_msg.pmk3,pmk3,sizeof(pmk3));

		NdisCopyMemory(update_ssid_psk_msg.EncrypType1,EncrypType1,strlen(EncrypType1));
		NdisCopyMemory(update_ssid_psk_msg.EncrypType2,EncrypType2,strlen(EncrypType2));
		
		NdisCopyMemory(update_ssid_psk_msg.AuthMode1,AuthMode1,strlen(AuthMode1));
		NdisCopyMemory(update_ssid_psk_msg.AuthMode2,AuthMode2,strlen(AuthMode2));
		
		EzMlmeEnqueue(pAd, EZ_STATE_MACHINE, EZ_UPDATE_SSID_PSK_REQ, sizeof(update_ssid_psk_msg), &update_ssid_psk_msg, (ULONG)wdev);
	} else {
		return FALSE;
	}


	return TRUE;
}


//! Leverage form MP.1.0 CL 170364
INT Set_EasySetup_conf_ssid_psk_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	char *value;
	char ssid[3*MAX_LEN_OF_SSID + 3];
	char psk[3*LEN_PSK + 3];
	char EncrypType[32];
	char AuthMode[32];
	unsigned char ssid_len, psk_len, EncrypType_len, AuthMode_len, len;
	web_conf_info_t conf_info;
	int i;

	EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("arg %s\n", arg));
	for (i=0, value = rstrtok(arg,";"); value; value = rstrtok(NULL,";"), i++)
	{
		if (i == 0)
		{
			memset(ssid, '\0', 3*MAX_LEN_OF_SSID + 3);
			NdisCopyMemory(ssid,value,strlen(value));
			ssid[strlen(value)] = '\0';
		} 
		else if(i == 1) 
		{
			NdisZeroMemory(psk, 3*LEN_PSK + 3);
			NdisCopyMemory(psk, value,strlen(value));
			psk[strlen(value)] = '\0';
		}
		else if(i == 2) 
		{
			NdisZeroMemory(EncrypType, sizeof(EncrypType));
			NdisCopyMemory(EncrypType, value,strlen(value));
			EncrypType[strlen(value)] = '\0';
		}
		else if(i == 3) 
		{
			NdisZeroMemory(AuthMode, sizeof(AuthMode));
			NdisCopyMemory(AuthMode, value,strlen(value));
			AuthMode[strlen(value)] = '\0';
		}
		
		if (i > 3)
		{
			break;
		}
	}

	EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ssid %s\n", ssid));
	EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("psk %s\n", psk));
	EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("EncrypType %s\n", EncrypType));	
	EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("AuthMode %s\n", AuthMode));


	ssid_len = strlen(ssid);
	psk_len = strlen(psk);
	EncrypType_len = strlen(EncrypType);
	AuthMode_len = strlen(AuthMode);
	conf_info.data_len = 1 + ssid_len + 1 + psk_len + 1 + EncrypType_len + 1 + AuthMode_len;
	len = 0;
	memcpy(&conf_info.data[0] + len, &ssid_len, 1);
	len += 1;
	memcpy(&conf_info.data[0] + len, ssid, ssid_len);
	len += ssid_len;
	memcpy(&conf_info.data[0] + len , &psk_len, 1);
	len += 1;
	memcpy(&conf_info.data[0] + len, psk, psk_len);
	len += psk_len;
	memcpy(&conf_info.data[0] + len , &EncrypType_len, 1);
	len += 1;
	memcpy(&conf_info.data[0] + len , EncrypType, EncrypType_len);	
	len += EncrypType_len;
	memcpy(&conf_info.data[0] + len , &AuthMode_len, 1);
	len += 1;
	memcpy(&conf_info.data[0] + len , AuthMode, AuthMode_len);

	EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("conf_info.data_len %d\n", conf_info.data_len));
	EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("conf_info.data %s\n", conf_info.data));
	{
#ifdef SYSTEM_LOG_SUPPORT
		//RTMPSendWirelessEvent(pAd, OID_WH_EZ_MAN_CONF_EVENT, NULL, wdev->wdev_idx, 0);
#else /* SYSTEM_LOG_SUPPORT */
		RtmpOSWrielessEventSend(pAd->net_dev, RT_WLAN_EVENT_CUSTOM, OID_WH_EZ_MAN_CONF_EVENT,
								NULL, (void *)&conf_info, sizeof(web_conf_info_t));
#endif /* !SYSTEM_LOG_SUPPORT */

	}
	return TRUE;
}	


void ez_apcli_link_down(
	void *ad_obj,
	void *apcli_obj,
	unsigned char if_idx)
{
	RTMP_ADAPTER *ad;
	APCLI_STRUCT *apcli_entry;
	unsigned char wcid;
	struct wifi_dev *ap_wdev;

	ad = (RTMP_ADAPTER *)ad_obj;
	apcli_entry = (APCLI_STRUCT *)apcli_obj;
	wcid = apcli_entry->MacTabWCID;
	ap_wdev = &ad->ApCfg.MBSSID[if_idx].wdev;
	if(ap_wdev)
	{
		if (IS_EZ_SETUP_ENABLED(&apcli_entry->wdev)
			&& (wcid < MAX_LEN_OF_MAC_TABLE)) {
			STA_TR_ENTRY *tr_entry;
			tr_entry = &ad->MacTab.tr_entry[wcid];
			if (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED) {

				/*
					Clear easy setup flag and CfgBssid for roaming.
				*/
				EZ_DEBUG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
					("(%s) ApCli interface[%d] Clear easy setup flag and CfgBssid for roaming.\n", 
					__FUNCTION__, if_idx));
				apcli_entry->MlmeAux.support_easy_setup = FALSE;
				NdisZeroMemory(&apcli_entry->CfgApCliBssid[0], MAC_ADDR_LEN);	
				
				if(apcli_entry->wdev.ez_driver_params.ezdev)
					ez_apcli_link_down_hook((ez_dev_t *)apcli_entry->wdev.ez_driver_params.ezdev,apcli_entry->Disconnect_Sub_Reason);
				
#ifdef SYSTEM_LOG_SUPPORT
				RTMPSendWirelessEvent(ad, IW_WH_EZ_MY_APCLI_DISCONNECTED, NULL, ap_wdev->wdev_idx, 0);
#else /* SYSTEM_LOG_SUPPORT */
				RtmpOSWrielessEventSend(ap_wdev->if_dev, RT_WLAN_EVENT_CUSTOM, IW_WH_EZ_MY_APCLI_DISCONNECTED,
							    NULL, NULL, 0);
#endif /* !SYSTEM_LOG_SUPPORT */
			}
		}
#ifdef IF_UP_DOWN
		apcli_entry->wdev.ez_driver_params.ez_apcli_peer_ap_config_cnt = 0;
#endif
	}
}


BOOLEAN ez_update_connection_permission(
	void *ad_obj,
	struct wifi_dev *wdev,
	enum EZ_CONN_ACTION action)
{
	if (wdev->ez_driver_params.ezdev)
	{
		return ez_update_connection_permission_hook((ez_dev_t *)wdev->ez_driver_params.ezdev, action);	
	}

	return TRUE;
}


BOOLEAN ez_is_connection_allowed(struct wifi_dev *wdev)
{
	if (wdev->ez_driver_params.ezdev){
		ez_dev_t *ezdev = wdev->ez_driver_params.ezdev;
#ifdef EZ_API_SUPPORT 
		if (ezdev->ez_security.ez_api_mode == CONNECTION_OFFLOAD)
			return TRUE;
#endif

#ifdef IF_UP_DOWN
		if (!ez_all_intf_up(ezdev->ez_ad))
			return FALSE;
#endif
		return ezdev->ez_security.ez_is_connection_allowed;
	}else 
		return TRUE;

}

BOOLEAN ez_probe_rsp_join_action(void *ad_obj, void *wdev_obj, OUT BCN_IE_LIST *ie_list, unsigned long  Bssidx)
{	
	struct wifi_dev *wdev;
	wdev = (struct wifi_dev *)wdev_obj;

		if (wdev->ez_driver_params.ezdev)
			return ez_probe_rsp_join_action_hook((ez_dev_t *)wdev->ez_driver_params.ezdev, ie_list->vendor_ie.beacon_info.network_weight);
		else 
			return TRUE;
}

void ez_update_connection(void *ad_obj, void *wdev_obj)
{
	struct wifi_dev *wdev;
	wdev = (struct wifi_dev *)wdev_obj;	

	if (wdev->ez_driver_params.ezdev)
	{
		ez_update_connection_hook((ez_dev_t *)wdev->ez_driver_params.ezdev);
	}
}


void ez_handle_pairmsg4(void *ad_obj, IN MAC_TABLE_ENTRY *pEntry)
{
	RTMP_ADAPTER *pAd = ad_obj;
	struct wifi_dev *wdev = pEntry->wdev;
	ez_dev_t *ezdev;
	APCLI_STRUCT *apcli_entry = &pAd->ApCfg.ApCliTab[pEntry->wdev->func_idx];
	

	apcli_entry->MlmeAux.attempted_candidate_index = EZ_INDEX_NOT_FOUND;

	if (wdev->ez_driver_params.ezdev)
	{
		ezdev = wdev->ez_driver_params.ezdev;

		NdisCopyMemory(ezdev->ez_security.this_band_info.shared_info.ssid,apcli_entry->Ssid,apcli_entry->SsidLen);
		NdisCopyMemory(ezdev->ez_security.this_band_info.pmk, apcli_entry->PMK,PMK_LEN);
	
		ezdev->ez_security.this_band_info.shared_info.ssid_len = apcli_entry->SsidLen;
		wdev->ez_driver_params.ez_wps_reconnect = FALSE;

		printk("%s, %d\n", __FUNCTION__, __LINE__);
		ez_handle_pairmsg4_hook((ez_dev_t *)wdev->ez_driver_params.ezdev, pEntry->Addr);
	}
	

}



void ez_roam(struct wifi_dev *wdev,BSS_ENTRY *pBssEntry)
{
	if (wdev->ez_driver_params.ezdev)
	{
		ez_roam_hook((ez_dev_t *)wdev->ez_driver_params.ezdev, pBssEntry->support_easy_setup, &pBssEntry->beacon_info,
			pBssEntry->Bssid, pBssEntry->Channel);
	}

}


ULONG
BssTableSearchWithBssId(
    IN BSS_TABLE *Tab,
    IN PUCHAR     Bssid,
    IN UCHAR      Channel)
{
    UCHAR i;
    UINT BssNr = Tab->BssNr;
    
    for (i = 0; i < BssNr; i++)
    {
        if ((i < MAX_LEN_OF_BSS_TABLE) &&
            MAC_ADDR_EQUAL(&(Tab->BssEntry[i].Bssid), Bssid))
        {
            return i;
        }
    }
    return (ULONG)BSS_NOT_FOUND;
}

BSS_ENTRY *ez_find_roam_candidate(void * ad_obj, unsigned char *bssid, struct wifi_dev *wdev)
{
	PRTMP_ADAPTER pAd = ad_obj;
	ULONG bss_idx;
//	BSS_ENTRY * pBssEntry = NULL;
	//if (IS_SINGLE_CHIP_DBDC(pAd))
	{
		bss_idx = BssTableSearchWithBssId(&pAd->ScanTab,bssid,0);
	}
	if (bss_idx == BSS_NOT_FOUND)
	{
		EZ_DEBUG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("ez_find_roam_candidate: Roam entry not found\n"));
		return NULL;
	}
	else
	{
		return &pAd->ScanTab.BssEntry[bss_idx];
	}

}



INT Set_EasySetup_RoamBssid_Proc(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING *arg)
{
	INT i;
	
	RTMP_STRING *value;
	UCHAR ifIndex;
	POS_COOKIE pObj;
	BSS_ENTRY *pBssEntry = NULL;
	struct wifi_dev *wdev;
	UCHAR roam_bssid[6];

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	
	if (pObj->ioctl_if_type != INT_APCLI)
		return FALSE;

	ifIndex = pObj->ioctl_if;

	wdev = &pAd->ApCfg.ApCliTab[ifIndex].wdev;
	if(strlen(arg) == 17)  /* Mac address acceptable format 01:02:03:04:05:06 length 17 */
	{
		for (i=0, value = rstrtok(arg,":"); value; value = rstrtok(NULL,":"), i++)
		{
			if((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value+1))) )
				return FALSE;  /* Invalid */

			AtoH(value, &roam_bssid[i], 1);
		}

		if(i != 6)
			return FALSE;  /* Invalid */
	}


	/*if connection not already ongoing, then initiate a connection.*/
	pBssEntry = ez_find_roam_candidate(pAd,roam_bssid,&pAd->ApCfg.ApCliTab[ifIndex].wdev );
	if (pBssEntry!= NULL){
//! Levarage from MP1.0 CL#170063
			if (wdev->ez_driver_params.ezdev) {
				if (ez_set_roam_bssid_hook((ez_dev_t *)wdev->ez_driver_params.ezdev, roam_bssid))
					EzMlmeEnqueue(pAd, EZ_STATE_MACHINE, EZ_ROAM_REQ, 0, NULL, (ULONG)&pAd->ApCfg.ApCliTab[ifIndex].wdev);
				return TRUE;
			}
	}
	else {
		if (wdev->ez_driver_params.ezdev)
			ez_reset_roam_bssid_hook((ez_dev_t *)wdev->ez_driver_params.ezdev);
		
		return FALSE;
	}
	return FALSE;
}


void ez_prepare_security_key(
	void *wdev_obj,
	unsigned char *peer_addr,
	unsigned char authenticator)
{
	struct wifi_dev *wdev = wdev_obj;
	wdev = (struct wifi_dev *)wdev_obj;


	if (wdev->ez_driver_params.ezdev)
	{
		ez_prepare_security_key_hook((ez_dev_t *)wdev->ez_driver_params.ezdev,peer_addr,authenticator);
	} else {
		ASSERT(FALSE);
	}
}

BOOLEAN ez_is_channel_same(struct wifi_dev *wdev)
{
	PRTMP_ADAPTER pAd = wdev->sys_handle;
	struct wifi_dev *apcli_wdev = &pAd->ApCfg.ApCliTab[wdev->func_idx].wdev;
	channel_info_t *channel_info;
	ez_dev_t *ezdev = wdev->ez_driver_params.ezdev;

	if (wdev->ez_driver_params.ezdev){
		channel_info = 	&ezdev->ez_security.this_band_info.shared_info.channel_info;
	} else {
		return FALSE;
	}	
	
	if ((pAd->CommonCfg.Channel == channel_info->channel) &&
#ifdef EZ_PUSH_BW_SUPPORT
	    (ezdev->ez_security.ap_did_fallback) &&
	     (ez_driver_ops_wlan_config_get_ht_bw_mt7603(wdev->ez_driver_params.ezdev) == channel_info->ht_bw && ez_driver_ops_wlan_config_get_ht_bw_mt7603(apcli_wdev->ez_driver_params.ezdev) == channel_info->ht_bw) &&
	     (ez_driver_ops_wlan_config_get_vht_bw_mt7603(wdev->ez_driver_params.ezdev) == channel_info->vht_bw && ez_driver_ops_wlan_config_get_vht_bw_mt7603(apcli_wdev) == channel_info->vht_bw) &&
#endif
	    (ez_driver_ops_wlan_config_get_ext_cha_mt7603(wdev->ez_driver_params.ezdev) == channel_info->extcha && ez_driver_ops_wlan_config_get_ext_cha_mt7603(apcli_wdev->ez_driver_params.ezdev) == channel_info->extcha)
	){
#if (defined(DOT11_N_SUPPORT) && defined(DOT11N_DRAFT3))
		if(ezdev->ez_security.ap_did_fallback){
			if(ezdev->ez_security.fallback_channel == channel_info->channel){
				EZ_DEBUG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("\nez_is_channel_same: Restoring ap to fallback mode\n"));
				ez_driver_ops_wlan_operate_set_ht_bw_mt7603(wdev->ez_driver_params.ezdev, HT_BW_20);
				ez_driver_ops_wlan_operate_set_ext_cha_mt7603(wdev->ez_driver_params.ezdev, EXTCHA_NONE);
			}
		}
#endif
		return TRUE;

	}
	else
		return FALSE;
}

void ez_process_action_frame(
	void *ad_obj,
	void *elem_obj)
{
	
	RTMP_ADAPTER *ad = (RTMP_ADAPTER *)ad_obj;
	MLME_QUEUE_ELEM *elem;
	MAC_TABLE_ENTRY *entry;
	struct wifi_dev *wdev;
	elem = (MLME_QUEUE_ELEM *)elem_obj;
	entry = &ad->MacTab.Content[elem->Wcid];
	wdev = entry->wdev;

	if (wdev->ez_driver_params.ezdev)
		ez_process_action_frame_hook((ez_dev_t *)wdev->ez_driver_params.ezdev,
		entry->Addr,elem->Msg,elem->MsgLen);
}


unsigned char ez_set_driver_group_id(
	ez_driver_params_t *ez_driver_param,
	unsigned char *ez_group_id,
	unsigned int ez_group_id_len,
	unsigned char inf_idx)
{
	int i;
	if (ez_driver_param->ez_group_id_len != 0) {
		os_free_mem(NULL, ez_driver_param->ez_group_id);
		ez_driver_param->ez_group_id_len = 0;
	}
	ez_driver_param->ez_group_id_len = ez_group_id_len;
	os_alloc_mem(NULL, &ez_driver_param->ez_group_id, ez_group_id_len);
	if (ez_driver_param->ez_group_id) {
		NdisZeroMemory(ez_driver_param->ez_group_id, ez_group_id_len);
		NdisCopyMemory(ez_driver_param->ez_group_id, ez_group_id, ez_group_id_len);

		ez_driver_param->ez_group_id_len = ez_group_id_len;
		ez_driver_param->group_id_len = ez_group_id_len;
		ez_driver_param->group_id = ez_driver_param->ez_group_id;
		
		EZ_DEBUG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
			("IF(ra%d) %s :: group id \n", 
			inf_idx, __FUNCTION__));
		for (i = 0; i < ez_driver_param->ez_group_id_len; i++) {
			EZ_DEBUG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
			("%02x ", ez_driver_param->ez_group_id[i]));
		}
		EZ_DEBUG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
			("\n"));
		return TRUE;
	}
	else {
		EZ_DEBUG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
			("IF(ra%d) %s::Memory alloc fails\n\n", 
			inf_idx, __FUNCTION__));
		ez_driver_param->ez_group_id_len = 0;
		return FALSE;
	}
}


unsigned char ez_set_gen_driver_group_id(
	ez_driver_params_t *ez_driver_param,
	unsigned char *gen_group_id,
	unsigned int gen_group_id_len,
	unsigned char inf_idx)
{
	int i;
	UCHAR hash_data[LEN_PMK];

	if (ez_driver_param->gen_group_id_len != 0) 
	{
		os_free_mem(NULL, ez_driver_param->gen_group_id);
		ez_driver_param->gen_group_id_len = 0;
	}

	NdisZeroMemory(&hash_data[0], LEN_PMK);
	RT_SHA256(gen_group_id, gen_group_id_len, &hash_data[0]);
	ez_set_driver_group_id(ez_driver_param, &hash_data[0], LEN_PMK, inf_idx);

	ez_driver_param->gen_group_id_len = gen_group_id_len;
	os_alloc_mem(NULL, &ez_driver_param->gen_group_id, gen_group_id_len);

	if (ez_driver_param->gen_group_id)
	{
		NdisZeroMemory(ez_driver_param->gen_group_id, ez_driver_param->gen_group_id_len);
		NdisCopyMemory(ez_driver_param->gen_group_id, gen_group_id, ez_driver_param->gen_group_id_len);
		MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
			("IF(ra%d) %s :: group id \n", 
			inf_idx, __FUNCTION__));
		for (i = 0; i < ez_driver_param->gen_group_id_len; i++) {
			MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
			("%02x ", ez_driver_param->gen_group_id[i]));
		}
		MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
			("\n"));
		return TRUE;
	}
	else 
	{
		MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
			("IF(ra%d) %s::Memory alloc fails\n\n", 
			inf_idx, __FUNCTION__));
		ez_driver_param->gen_group_id_len = 0;
		return FALSE;
	}
}

unsigned char ez_set_driver_open_group_id(
	ez_driver_params_t *ez_driver_param,
	unsigned char *open_group_id,
	unsigned int open_group_id_len,
	unsigned char inf_idx)
{
	int i;
	if (ez_driver_param->open_group_id_len != 0) {
		NdisZeroMemory(ez_driver_param->open_group_id,OPEN_GROUP_MAX_LEN);
		ez_driver_param->open_group_id_len = 0;
	}
	if(open_group_id_len > OPEN_GROUP_MAX_LEN)
		return FALSE;
	ez_driver_param->open_group_id_len = open_group_id_len;
	//EZ_MEM_ALLOC(NULL, &ez_security->group_id, ez_security->group_id_len);
		NdisZeroMemory(ez_driver_param->open_group_id, OPEN_GROUP_MAX_LEN);
		NdisCopyMemory(ez_driver_param->open_group_id, open_group_id, ez_driver_param->open_group_id_len);
		EZ_DEBUG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
			("IF(ra%d) %s :: open_group id \n", 
			inf_idx, __FUNCTION__));
		for (i = 0; i < ez_driver_param->open_group_id_len; i++) {
			EZ_DEBUG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
			("%02x ", ez_driver_param->open_group_id[i]));
		}
		EZ_DEBUG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
			("\n"));
		return TRUE;
}


void convert_pmk_string_to_hex(char *sys_pmk_string, char *sys_pmk)
{
	int ret;
	unsigned char nibble;
	
	for (ret = 0; ret < 64; ret++)
	{
		nibble = sys_pmk_string[ret];
		if ((nibble <= '9'))
		{
			nibble = nibble - '0';
		} 
		else if (nibble < 'a') 
		{
			nibble = nibble - 'A' + 10;
		} else {
			nibble = nibble - 'a' + 10;			
		}
		if (ret % 2)
		{
			sys_pmk[ret/2] |= nibble; 
		}
		else 
		{
			sys_pmk[ret/2] = nibble << 4;
		}
	}
	
}

void ez_read_parms_from_file(
	void *ad_obj,
	char *tmpbuf,
	char *buffer)
{
	int i;
	RTMP_STRING *macptr;
	RTMP_ADAPTER *ad;

	ad = (RTMP_ADAPTER *)ad_obj;
	if(RTMPGetKeyParameter("EzEnable", tmpbuf, 10, buffer, TRUE))
	{
		for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
		{
			INT ez_enable = simple_strtol(macptr, 0, 10);

			if (i >= ad->ApCfg.BssidNum)
				break;

			if (ez_enable == 0)
			{
				ad->ApCfg.MBSSID[i].wdev.ez_driver_params.enable_easy_setup = FALSE;
			}
			else
			{
				ad->ApCfg.MBSSID[i].wdev.ez_driver_params.enable_easy_setup = TRUE;
			}

			EZ_DEBUG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(ra%d) EzEnable=%d\n", 
				i, ad->ApCfg.MBSSID[i].wdev.ez_driver_params.enable_easy_setup));
		}
	}
	if(RTMPGetKeyParameter("EtherTrafficBand", tmpbuf, 10, buffer, TRUE))
	{
		INT default_group_data_band = 0;
	
		macptr = rstrtok(tmpbuf,";");
		if(macptr)
		default_group_data_band = simple_strtol(macptr, 0, 10);
		
	
		if (default_group_data_band == 0)
		{
			ad->ApCfg.MBSSID[MAIN_MBSSID].wdev.ez_driver_params.default_group_data_band = EZ_DROP_GROUP_DATA_BAND24G;	
		}
		else
		{
			EZ_DEBUG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("===============DROP BAND 5G\n"));
			ad->ApCfg.MBSSID[MAIN_MBSSID].wdev.ez_driver_params.default_group_data_band = EZ_DROP_GROUP_DATA_BAND5G;
		}
	}

	if(RTMPGetKeyParameter("EzGroupID", tmpbuf, 256, buffer, TRUE))
	{
		for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
		{
			ez_driver_params_t *ez_driver_param;

			if (i >= ad->ApCfg.BssidNum)
				break;
			ez_driver_param = &ad->ApCfg.MBSSID[i].wdev.ez_driver_params;
		
			ez_set_driver_group_id(ez_driver_param, macptr, strlen(macptr), i);
		}
	}
	if(RTMPGetKeyParameter("EzGenGroupID", tmpbuf, 256, buffer, TRUE))
	{
		for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
		{
			ez_driver_params_t *ez_driver_param;

			if (i >= ad->ApCfg.BssidNum)
				break;

			ez_driver_param = &ad->ApCfg.MBSSID[i].wdev.ez_driver_params;
			ez_set_gen_driver_group_id(ez_driver_param, macptr, strlen(macptr), i);
		}
	}
	if(RTMPGetKeyParameter("EzOpenGroupID", tmpbuf, 256, buffer, TRUE))
	{
		for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
		{
			ez_driver_params_t *ez_driver_param;

			if (i >= ad->ApCfg.BssidNum)
				break;
			ez_driver_param = &ad->ApCfg.MBSSID[i].wdev.ez_driver_params;
			ez_set_driver_open_group_id(ez_driver_param, macptr, strlen(macptr), i);
		}
	}
#ifdef EZ_API_SUPPORT
	if(RTMPGetKeyParameter("EzApiMode", tmpbuf, 256, buffer, TRUE))
	{
		//for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
		{
			
			INT ez_api_mode = 0;
			EZ_ADAPTER *ez_ad = ad->ez_ad;
			macptr = rstrtok(tmpbuf,";");
			if(macptr)
			ez_api_mode = simple_strtol(macptr, 0, 10);

			ez_ad->ez_api_mode = ez_api_mode;
			//break;
		}
	}
#endif
#ifdef EZ_PUSH_BW_SUPPORT
	if(RTMPGetKeyParameter("EzPushBw", tmpbuf, 10, buffer, TRUE))
	{
		INT ez_push_bw = 0;
		EZ_ADAPTER *ez_ad = ad->ez_ad;

		macptr = rstrtok(tmpbuf,";");
		if(macptr)
		ez_push_bw = simple_strtol(macptr, 0, 10);

		if (ez_push_bw == 0)
		{
			ez_ad->push_bw_config = FALSE;
		}
		else
		{
			ez_ad->push_bw_config = TRUE;
		}

		EZ_DEBUG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("EzPushBw=%d\n", 
			ez_ad->push_bw_config));
	}
#endif
#ifdef APCLI_SUPPORT
	if(RTMPGetKeyParameter("ApCliEzEnable", tmpbuf, 10, buffer, TRUE))
	{
		for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
		{
			INT ez_enable = simple_strtol(macptr, 0, 10);

			if (i >= MAX_APCLI_NUM)
				break;

			if (ez_enable == 0)
			{
				ad->ApCfg.ApCliTab[i].wdev.ez_driver_params.enable_easy_setup = FALSE;
			}
			else
			{
				ad->ApCfg.ApCliTab[i].wdev.ez_driver_params.enable_easy_setup = TRUE;
			}

			EZ_DEBUG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(apcli%d) ApCliEzEnable=%d\n", 
				i, ad->ApCfg.ApCliTab[i].wdev.ez_driver_params.enable_easy_setup));
		}
	}

	if(RTMPGetKeyParameter("ApCliEzGroupID", tmpbuf, 256, buffer, TRUE))
	{
		for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
		{
			ez_driver_params_t *ez_driver_param;

			if (i >= MAX_APCLI_NUM)
				break;

			ez_driver_param = &ad->ApCfg.ApCliTab[i].wdev.ez_driver_params;
			ez_set_driver_group_id(ez_driver_param, macptr, strlen(macptr), i);
			//ez_set_group_id(ez_security, macptr, strlen(macptr), i);
		}
	}
	if(RTMPGetKeyParameter("ApCliEzGenGroupID", tmpbuf, 256, buffer, TRUE))
	{
		for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
		{
			ez_driver_params_t *ez_driver_param;
			
			if (i >= MAX_APCLI_NUM)
				break;

			ez_driver_param = &ad->ApCfg.ApCliTab[i].wdev.ez_driver_params;

			ez_set_gen_driver_group_id(ez_driver_param, macptr, strlen(macptr), i);
		}
	}
	
	if(RTMPGetKeyParameter("ApCliEzOpenGroupID", tmpbuf, 256, buffer, TRUE))
	{
		for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
		{
			ez_driver_params_t *ez_driver_param;

			if (i >= MAX_APCLI_NUM)
				break;

			ez_driver_param = &ad->ApCfg.ApCliTab[i].wdev.ez_driver_params;
			ez_set_driver_open_group_id(ez_driver_param, macptr, strlen(macptr), i);
		}
	}
	
	if(RTMPGetKeyParameter("ApCliEzRssiThreshold", tmpbuf, 10, buffer, TRUE))
	{
		for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
		{
			INT rssi_threshold = simple_strtol(macptr, 0, 10);

			if (i >= MAX_APCLI_NUM)
				break;

			ad->ApCfg.ApCliTab[i].wdev.ez_driver_params.rssi_threshold = (CHAR)(-1)*rssi_threshold;

			EZ_DEBUG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(apcli%d) rssi_threshold=%d\n", 
				i, ad->ApCfg.ApCliTab[i].wdev.ez_driver_params.rssi_threshold));
		}
	}
#endif /* APCLI_SUPPORT */

	if(RTMPGetKeyParameter("EzDefaultSsid", tmpbuf, 256, buffer, TRUE))
	{
		for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
		{

			ad->ApCfg.MBSSID[i].wdev.ez_driver_params.default_ssid_len = strlen(macptr);
			NdisZeroMemory(ad->ApCfg.MBSSID[i].wdev.ez_driver_params.default_ssid,MAX_LEN_OF_SSID);
			NdisCopyMemory(ad->ApCfg.MBSSID[i].wdev.ez_driver_params.default_ssid,macptr, strlen(macptr));

			NdisZeroMemory(ad->ApCfg.ApCliTab[i].wdev.ez_driver_params.default_ssid,MAX_LEN_OF_SSID);
			ad->ApCfg.ApCliTab[i].wdev.ez_driver_params.default_ssid_len = strlen(macptr);
			NdisCopyMemory(ad->ApCfg.ApCliTab[i].wdev.ez_driver_params.default_ssid,macptr, strlen(macptr));

		}
	}
	if(RTMPGetKeyParameter("EzDefaultPmk", tmpbuf, 256, buffer, TRUE))
	{
		for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
		{
			convert_pmk_string_to_hex(macptr, ad->ApCfg.MBSSID[i].wdev.ez_driver_params.default_pmk);
			NdisCopyMemory(ad->ApCfg.ApCliTab[i].wdev.ez_driver_params.default_pmk,ad->ApCfg.MBSSID[i].wdev.ez_driver_params.default_pmk,PMK_LEN);
		}
	}

	if(RTMPGetKeyParameter("EzDefaultPmkValid", tmpbuf, 256, buffer, TRUE))
	{
		for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
		{

			ad->ApCfg.MBSSID[i].wdev.ez_driver_params.default_pmk_valid = simple_strtol(macptr,0, 10);
			ad->ApCfg.ApCliTab[i].wdev.ez_driver_params.default_pmk_valid = simple_strtol(macptr, 0 , 10);
//! Levarage from MP1.0 CL 170210

		}
	}
//! Levarage from MP1.0 CL 170210
#ifdef EZ_REGROUP_SUPPORT	// add for multiprofile too?
	if(RTMPGetKeyParameter("RegroupSupport", tmpbuf, 10, buffer, TRUE))
	{
		for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
		{
			INT regrp_supp = simple_strtol(macptr, 0, 10);

			if (i >= ad->ApCfg.BssidNum)
				break;

			if (regrp_supp == 0)
			{
				ad->ApCfg.MBSSID[i].wdev.ez_driver_params.en_regrp_supp = FALSE;
				ad->ApCfg.ApCliTab[i].wdev.ez_driver_params.en_regrp_supp = FALSE;
			}
			else
			{
				ad->ApCfg.MBSSID[i].wdev.ez_driver_params.en_regrp_supp = TRUE;
				ad->ApCfg.ApCliTab[i].wdev.ez_driver_params.en_regrp_supp = TRUE;
			}

			EZ_DEBUG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(ra%d) RegroupSupport=%d\n", 
				i, ad->ApCfg.MBSSID[i].wdev.ez_driver_params.en_regrp_supp));
		}

	}
#endif
#ifdef IF_UP_DOWN
	if (RTMPGetKeyParameter("EzIntfCnt", tmpbuf, 10, buffer, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			UINT intf_cnt_ap, intf_cnt_cli;
			RTMP_STRING *macptr1 = NULL;

			if (i >= ad->ApCfg.BssidNum)
				break;
			macptr1 = rstrtok(macptr, ":");
			intf_cnt_ap = simple_strtol(macptr1, 0, 10);
			macptr1 = rstrtok(macptr, ":");
			intf_cnt_cli = simple_strtol(macptr1, 0, 10);

			ad->ApCfg.MBSSID[i].wdev.ez_driver_params.ez_intf_count_config_ap = intf_cnt_ap;
			ad->ApCfg.MBSSID[i].wdev.ez_driver_params.ez_intf_count_config_cli = intf_cnt_cli;

			EZ_DEBUG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("I/F(ra%d) IntfCnt=%d:%d\n",
				i, ad->ApCfg.MBSSID[i].wdev.ez_driver_params.ez_intf_count_config_ap,
				ad->ApCfg.MBSSID[i].wdev.ez_driver_params.ez_intf_count_config_cli));
		}
	}
#endif
}

int ez_internet_msg_handle(void *ad_obj, RTMP_IOCTL_INPUT_STRUCT *wrq)
{
	RTMP_ADAPTER *pAd;
	int status;
	POS_COOKIE	pObj;
	int apidx;
	struct wifi_dev *wdev;
	int Status;
	p_internet_command_t p_internet_command;
	p_internet_command = NULL;
	status = NDIS_STATUS_SUCCESS;
	pAd = (RTMP_ADAPTER *)ad_obj;
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	apidx = pObj->ioctl_if;
	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	
	os_alloc_mem(NULL, (UCHAR **)&p_internet_command, wrq->u.data.length);
	
	if (p_internet_command == NULL)
	{
		EZ_DEBUG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_ERROR, ("!!!(%s) : no memory!!!\n", __FUNCTION__));
		return NDIS_STATUS_FAILURE;
	}
	
	copy_from_user(p_internet_command, wrq->u.data.pointer, wrq->u.data.length);
	
	EZ_DEBUG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("Net_status =%d,\n",
		p_internet_command->Net_status));
	Status = ez_internet_msghandle_hook((ez_dev_t *)wdev->ez_driver_params.ezdev, p_internet_command);
	os_free_mem(pAd, p_internet_command);
	if(Status != NDIS_STATUS_SUCCESS)
	{
		Status = -NDIS_STATUS_FAILURE;
	}
	return Status;
}

INT Custom_DataHandle(void *ad_obj, RTMP_IOCTL_INPUT_STRUCT *wrq)
{
	RTMP_ADAPTER *pAd;
	p_ez_custom_data_cmd_t p_custom_data; 

	pAd = (RTMP_ADAPTER *)ad_obj;
	p_custom_data = NULL;
	
	os_alloc_mem(NULL, (UCHAR **)&p_custom_data, wrq->u.data.length);
	if (p_custom_data == NULL)
	{
		MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("!!!(%s) : no memory!!!\n", __FUNCTION__));
		return NDIS_STATUS_FAILURE;
	}

	memset(p_custom_data, '\0', wrq->u.data.length);

	copy_from_user(p_custom_data, wrq->u.data.pointer, wrq->u.data.length);
	
	ez_custom_data_handle_hook((EZ_ADAPTER *)pAd->ez_ad, p_custom_data, (unsigned char )wrq->u.data.length);

	os_free_mem(pAd, p_custom_data);

	return NDIS_STATUS_SUCCESS;
}


#ifdef EZ_REGROUP_SUPPORT

regrp_ap_info_struct * ez_find_regrp_ap_by_bssid(regrp_ap_info_struct *p_ap_info_list, UINT8 * bssid)
{
	int i;
	for(i=0; i< EZ_MAX_DEVICE_SUPPORT; i++)
	{
		if(p_ap_info_list[i].valid && MAC_ADDR_EQUAL(p_ap_info_list[i].bssid,bssid))
			return &p_ap_info_list[i];
	}
	return NULL;
}

void ez_delete_regrp_old_ap(regrp_ap_info_struct *p_ap_info_list)
{
	int i;
	ULONG now;
	NdisGetSystemUpTime(&now);
	for(i=0; i< EZ_MAX_DEVICE_SUPPORT; i++)
	{
		if(p_ap_info_list[i].valid && RTMP_TIME_AFTER(now,p_ap_info_list[i].last_rx_time + 5*OS_HZ))
		{
			printk("%s: %02x:%02x:%02x:%02x:%02x:%02x\n",__func__, PRINT_MAC(p_ap_info_list[i].bssid));
	//		hex_dump("RegrpAPDel: Ageout",p_ap_info_list[i].bssid,MAC_ADDR_LEN);
			NdisZeroMemory(&p_ap_info_list[i],sizeof(regrp_ap_info_struct));
		//	p_ap_info_list[i].valid = 1;
		}
	}
}

regrp_ap_info_struct * ez_add_regrp_ap(regrp_ap_info_struct *p_ap_info_list)
{
	int i;

	ez_delete_regrp_old_ap(p_ap_info_list);

	for(i=0; i< EZ_MAX_DEVICE_SUPPORT; i++)
	{
		if(!p_ap_info_list[i].valid)
		{
			NdisZeroMemory(&p_ap_info_list[i],sizeof(regrp_ap_info_struct));
			p_ap_info_list[i].valid = 1;
			return &p_ap_info_list[i];
		}
	}
	return NULL;
}

extern regrp_ap_info_struct regrp_ap_info[2][EZ_MAX_DEVICE_SUPPORT];

void ez_print_cand_list(p_regrp_ap_info_struct p_ap_info)
{
	int i;
	for (i = 0; (i < EZ_MAX_DEVICE_SUPPORT) ; i++) 
	{
		if(p_ap_info[i].valid == 1)
		{
			EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
				("[%d] %02x:%02x:%02x:%02x:%02x:%02x RSSI=%d: %d EZ=%d InternetStatus= 0x%x\n", i, 
				PRINT_MAC(p_ap_info[i].bssid),
				p_ap_info[i].avg_rssi,p_ap_info[i].last_rssi, !p_ap_info[i].Non_MAN,p_ap_info[i].internet_status));
		}
	}
}
int ez_get_candidate_list(struct wifi_dev *wdev, pregrp_cmd_cand_list peer_list, UINT8 *data_len)
{
	UCHAR i=0;
//	BSS_ENTRY *bss_entry = NULL;
	pntw_info pcand_entry = NULL;
	//RTMP_ADAPTER *pAd = (RTMP_ADAPTER *) wdev->sys_handle;
	//PAPCLI_STRUCT pApCliEntry = NULL;
	UINT8 band;

	if(wdev->channel <= 14)
		band =0;
	else
		band = 1;


	//pApCliEntry = &pAd->ApCfg.ApCliTab[wdev->func_idx];

	peer_list->cand_count = 0;
	pcand_entry = (pntw_info) &peer_list->list[0];
	printk("<<===>%s : %d>>\n",__FUNCTION__,band);
	// following logic referred from ez_apcli_search_best_ap_configured(), with required checks kept
	ez_delete_regrp_old_ap(regrp_ap_info[0]);
	ez_delete_regrp_old_ap(regrp_ap_info[1]);
	ez_print_cand_list(regrp_ap_info[band]);

	for (i = 0; (i < EZ_MAX_DEVICE_SUPPORT) ; i++) 
	{
		p_regrp_ap_info_struct p_ap_info = &regrp_ap_info[band][i];
		if(p_ap_info->valid)
		{
			// fill the candidate info
			//memcpy(pcand_entry->ssid,bss_entry->Ssid,bss_entry->SsidLen);
			NdisCopyMemory(pcand_entry->bssid,p_ap_info->bssid, MAC_ADDR_LEN);
			pcand_entry->Non_MAN = p_ap_info->Non_MAN;
			pcand_entry->rssi = p_ap_info->avg_rssi;		
			if(!pcand_entry->Non_MAN){
				NdisCopyMemory(pcand_entry->nw_wt,p_ap_info->nw_wt, NETWORK_WEIGHT_LEN);
				NdisCopyMemory(&pcand_entry->node_number, &p_ap_info->node_number,sizeof(EZ_NODE_NUMBER));
			}
			else
			{
				NdisZeroMemory(pcand_entry->nw_wt,NETWORK_WEIGHT_LEN);
				NdisZeroMemory(&pcand_entry->node_number,sizeof(EZ_NODE_NUMBER));
			}
			
			peer_list->cand_count++;
			pcand_entry++;

		}
	}

	EZ_DEBUG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("Number of Candidates = %d\n", peer_list->cand_count));
	NdisZeroMemory(&regrp_ap_info[band][0], sizeof(regrp_ap_info_struct)*EZ_MAX_DEVICE_SUPPORT);
#ifdef DBG
//	printk("Raghav0\n");
	pcand_entry = (pntw_info) &peer_list->list[0];
//	printk("Raghav1\n");
	for (i=0; i<peer_list->cand_count; i++) {
		EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
			("[%d] %02x:%02x:%02x:%02x:%02x:%02x RSSI=%d EZ=%d InternetStatus= 0x%x\n", i, 
			pcand_entry[i].bssid[0],
			pcand_entry[i].bssid[1],
			pcand_entry[i].bssid[2],
			pcand_entry[i].bssid[3],
			pcand_entry[i].bssid[4],
			pcand_entry[i].bssid[5],
			pcand_entry[i].rssi, !pcand_entry[i].Non_MAN,pcand_entry[i].internet_status));
	}
	
	//EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
	//	("%s(line.%d) - apcli_entry->CfgSsid-%s, apcli_entry->Ssid-%s\n", 
	//	__FUNCTION__, __LINE__,apcli_entry->CfgSsid, apcli_entry->Ssid));
#endif

	// skip rssi sorting to save time, as requestor can do it if required
	//BssTableSortByRssi(out_table, FALSE);
	*data_len = peer_list->cand_count * sizeof(ntw_info);

	return NDIS_STATUS_SUCCESS;
}

int ez_handle_regroup_query_cmd(RTMP_ADAPTER *pAd, RTMP_IOCTL_INPUT_STRUCT *wrq)
{
    POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
    int Status = NDIS_STATUS_SUCCESS;
	p_regrp_command_t p_regrp_cmd;
    struct wifi_dev *wdev;
    UCHAR ifIndex;
    BOOLEAN apcliEn=FALSE;
	EZ_ADAPTER *ez_ad = pAd->ez_ad; 
	printk("===>%s\n",__FUNCTION__);

    pObj = (POS_COOKIE) pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;

	if(wrq->u.data.length < sizeof(regrp_command_t))
		return FALSE;

	p_regrp_cmd = (p_regrp_command_t)(wrq->u.data.pointer);

	switch(p_regrp_cmd->command_id)
	{
		case OID_REGROUP_CMD_REGRP_SUPP:
		{
			pcmd_regrp_supp pbuf = (pcmd_regrp_supp)p_regrp_cmd;
			printk("===>%s OID_REGROUP_CMD_REGRP_SUPP\n",__FUNCTION__);

#ifdef APCLI_SUPPORT
			if (pObj->ioctl_if_type == INT_APCLI) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
					("IF(apcli%d) %s:: OID_REGROUP_CMD_CONNECTED_PEER_LIST", 
					ifIndex, __FUNCTION__));

				wdev = &pAd->ApCfg.ApCliTab[ifIndex].wdev;
				//apcliEn = pAd->ApCfg.ApCliTab[ifIndex].Enable;
				//if (!apcliEn){
				//	wrq->u.data.length = sizeof(regrp_command_t);
				//	return NDIS_STATUS_SUCCESS;
				//}
		
			}
			else 
#endif /* APCLI_SUPPORT */
			{
				wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
					("IF(ra%d) %s:: ", 
					ifIndex, __FUNCTION__));
			}

			if(IS_REGRP_SUPP(wdev)){
				pbuf->regrp_supp = TRUE;
			}
			else
				pbuf->regrp_supp = FALSE;

			if(IS_EZ_SETUP_ENABLED(wdev)){
				pbuf->ez_supp = TRUE;


				pbuf->configured = TRUE;
			}
			else
				pbuf->ez_supp = FALSE;

			// always take channel from ap context, though apcli wdev should have same channel
			pbuf->channel = pAd->ApCfg.MBSSID[ifIndex].wdev.channel;

			break;
		}			

		case OID_REGROUP_CMD_CAND_LIST:
			printk("===>%s OID_REGROUP_CMD_CAND_LIST\n",__FUNCTION__);

			if (pObj->ioctl_if_type == INT_APCLI) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
					("IF(apcli%d) %s:: OID_REGROUP_CMD_ROAM_AP_LIST", 
					ifIndex, __FUNCTION__));

				wdev = &pAd->ApCfg.ApCliTab[ifIndex].wdev;
				apcliEn = pAd->ApCfg.ApCliTab[ifIndex].Enable;
				if (!apcliEn || (!IS_REGRP_SUPP(wdev) || !IS_EZ_SETUP_ENABLED(wdev)) ){
					wrq->u.data.length = sizeof(regrp_command_t);
					return NDIS_STATUS_FAILURE;	// proper??
				}

				Status = ez_get_candidate_list(wdev,(pregrp_cmd_cand_list)p_regrp_cmd,&p_regrp_cmd->command_len);

				wrq->u.data.length = sizeof(regrp_cmd_hdr) + p_regrp_cmd->command_len;
				Status = copy_to_user(wrq->u.data.pointer, p_regrp_cmd, wrq->u.data.length);
			}
			else
				Status = -EOPNOTSUPP; 
			break;
		
		case OID_REGROUP_QUERY_INTERFACE_DETAILS:
		{
			vr_ap_info_struct ap_info;
			printk("===>%s OID_REGROUP_QUERY_INTERFACE_DETAILS\n",__FUNCTION__);

			NdisZeroMemory(&ap_info,sizeof(ap_info));
			ap_info.ssid_len = pAd->ApCfg.MBSSID[ifIndex].SsidLen;
			NdisCopyMemory(ap_info.ssid,pAd->ApCfg.MBSSID[ifIndex].Ssid,pAd->ApCfg.MBSSID[ifIndex].SsidLen);
			COPY_MAC_ADDR(ap_info.mac_addr, pAd->ApCfg.MBSSID[ifIndex].wdev.bssid);
			NdisCopyMemory(ap_info.intf_prefix, get_dev_name_prefix(pAd, INT_MAIN), strlen(get_dev_name_prefix(pAd, INT_MAIN)));
			ap_info.wdev_id = ifIndex;
			Status = copy_to_user(wrq->u.data.pointer, &ap_info, wrq->u.data.length);
			break;
		}
		case OID_REGROUP_QUERY_NODE_NUMBER_WT:
		{
			node_num_wt node_wt;
			
			printk("===>%s OID_REGROUP_QUERY_NODE_NUMBER_WT\n",__FUNCTION__);
			NdisCopyMemory(&node_wt.node_number, &ez_ad->device_info.ez_node_number,sizeof(EZ_NODE_NUMBER));	
			NdisCopyMemory(node_wt.network_wt, ez_ad->device_info.network_weight,NETWORK_WEIGHT_LEN);
			
			Status = copy_to_user(wrq->u.data.pointer, &node_wt, sizeof(node_wt));
			break;
		}
		default:
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
				("Query::unknown IOCTL's subcmd = 0x%08x, IFidx=%d\n", p_regrp_cmd->command_id, ifIndex));
			Status = -EOPNOTSUPP;
			break;	
	}	

	return Status;

}

int regrp_set_mode(struct wifi_dev *wdev, UINT8 req_mode){
    int Status = NDIS_STATUS_SUCCESS;
	UINT8 *pcurr_mode = &wdev->ez_driver_params.regrp_mode;

	RTMP_SEM_LOCK(&wdev->ez_driver_params.regrp_mode_lock);

	printk("regrp_set_mode => Current Stae: %d, requested State : %d", *pcurr_mode, req_mode);

	if(*pcurr_mode == req_mode){
		printk("regrp_set_mode: Same Mode request\n");
		goto exit_func;
	}

	switch(req_mode){	// check handling required for requested mode, based on possible current mode

		case REGRP_MODE_BLOCKED:
			{
				if(*pcurr_mode == NON_REGRP_MODE){	// regroup actviity started on device
	//				startRegrpModeTimer(wdev);	// should happen only once
				}
				else { // unblocked mdoe to blocked mode		i.e. regp finished for this interface
		//			stopRegrpUnblockModeTimer(wdev);
					// do other cleanup like candiate list cleanup ?? if apcli / ap check
					// ensure conenct activity gets stopped?
				}
				*pcurr_mode = req_mode;
				//cancel the scan timer if running
				if(wdev->ez_driver_params.ez_scan_timer.ez_timer_running== TRUE)
				{
					EZ_CANCEL_TIMER(&wdev->ez_driver_params.ez_scan_timer.ez_timer,
						wdev->ez_driver_params.ez_scan_timer.ez_timer_running);
					//RTMPCancelTimer(&wdev->ez_security.ez_scan_timer,&Cancelled);
				}
			}
			break;
		case REGRP_MODE_UNBLOCKED:
			if(*pcurr_mode == NON_REGRP_MODE){	// regroup actviity started on device
				printk(" Abnormal transition requested \n");
				Status = NDIS_STATUS_FAILURE; // proper?
				goto exit_func;
			}
			else{ // blocked to unblocked mode	regrp started for this interface
				// init context required for conenct attempt ...	if ap/apcli check

				// check conenct state too?
				
				if(ez_is_roaming_ongoing_hook(wdev->sys_handle)){
					printk(" Reject as roaming ongoing \n");
					Status = NDIS_STATUS_FAILURE; // proper?
					goto exit_func;
				}

			}
			*pcurr_mode = req_mode;

			break;

		case NON_REGRP_MODE:
			if(*pcurr_mode == REGRP_MODE_BLOCKED){	// regroup actviity started on device
				//stopRegrpModeTimer(wdev);
				// other cleanup??
			}
			else{ // unblocked mdoe to blocked mode
			//	stopRegrpUnblockModeTimer(wdev);
				// do other cleanup like candiate list cleanup ??	if apcli/ap check
				// ensure conenct activity gets stopped?

				//stopRegrpModeTimer(wdev);
				// other cleanup

			}
			*pcurr_mode = req_mode;

			break;
		default:
			printk("regrp_set_mod: Unknown mode requested\n");
			Status = NDIS_STATUS_FAILURE; // proper?
			break;
	}

exit_func:
	
	RTMP_SEM_UNLOCK(&wdev->ez_driver_params.regrp_mode_lock);

	return Status;	
}


int ez_handle_regroup_set_cmd(RTMP_ADAPTER *pAd, RTMP_IOCTL_INPUT_STRUCT *wrq)
{
    POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
    int Status = NDIS_STATUS_SUCCESS;
	p_regrp_command_t p_regrp_cmd;
    struct wifi_dev *wdev;
    UCHAR ifIndex;
   // BOOLEAN apcliEn=FALSE;

//    BOOLEAN apcliEn=FALSE;
	printk("===>%s \n",__FUNCTION__);

    pObj = (POS_COOKIE) pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;

	if(wrq->u.data.length < sizeof(regrp_command_t))
		return FALSE;

	p_regrp_cmd = (p_regrp_command_t)(wrq->u.data.pointer);

	switch(p_regrp_cmd->command_id)
	{
		case OID_REGROUP_CMD_EN_REGRP_MODE:
		{
				
			pcmd_regrp_mode pbuf = (pcmd_regrp_mode)p_regrp_cmd;
			printk("===>%s OID_REGROUP_CMD_EN_REGRP_MODE\n",__FUNCTION__);

#ifdef APCLI_SUPPORT
			if (pObj->ioctl_if_type == INT_APCLI) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
					("IF(apcli%d) %s:: OID_REGROUP_CMD_CONNECTED_PEER_LIST", 
					ifIndex, __FUNCTION__));

				wdev = &pAd->ApCfg.ApCliTab[ifIndex].wdev;
				//apcliEn = pAd->ApCfg.ApCliTab[ifIndex].Enable;
				//if (!apcliEn){
				//	wrq->u.data.length = sizeof(regrp_command_t);
				//	return NDIS_STATUS_SUCCESS;
				//}
		
			}
			else 
#endif /* APCLI_SUPPORT */
			{
				wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
					("IF(ra%d) %s:: ", 
					ifIndex, __FUNCTION__));
			}

			Status = regrp_set_mode(wdev,pbuf->mode);

			break;
		}			
		default:
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
				("Set::unknown IOCTL's subcmd = 0x%08x, IFidx=%d\n", p_regrp_cmd->command_id, ifIndex));
			Status = -EOPNOTSUPP;
			break;	
	}	

	return Status;

}

#endif




int ez_parse_set_command(void *ad_obj, RTMP_IOCTL_INPUT_STRUCT *wrq, IN int cmd)
{
    RTMP_ADAPTER *pAd;
	struct wifi_dev *wdev;
	UCHAR inf_idx;
	int Status = NDIS_STATUS_SUCCESS;
	POS_COOKIE pObj;
	pAd = (RTMP_ADAPTER *)ad_obj;
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	inf_idx = pObj->ioctl_if;
#ifdef APCLI_SUPPORT
	if (pObj->ioctl_if_type == INT_APCLI) {
		wdev = &pAd->ApCfg.ApCliTab[inf_idx].wdev;
		if (!pAd->ApCfg.ApCliTab[inf_idx].Enable)
			return FALSE;

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
			("IF(apcli%d) %s:: ", 
			inf_idx, __FUNCTION__));
	}
	else 
#endif /* APCLI_SUPPORT */
	{
		wdev = &pAd->ApCfg.MBSSID[inf_idx].wdev;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
			("IF(ra%d) %s:: ", 
			inf_idx, __FUNCTION__));
	}

	switch(cmd & 0x7FFF)
	{
		
		case OID_WH_EZ_ENABLE:
			inf_idx = pObj->ioctl_if;
#ifdef APCLI_SUPPORT
			if (pObj->ioctl_if_type == INT_APCLI) {
				wdev = &pAd->ApCfg.ApCliTab[inf_idx].wdev;
				if (!pAd->ApCfg.ApCliTab[inf_idx].Enable)
					return FALSE;
	
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
					("IF(apcli%d) %s:: ", 
					inf_idx, __FUNCTION__));
			}
			else 
#endif /* APCLI_SUPPORT */
			{
				wdev = &pAd->ApCfg.MBSSID[inf_idx].wdev;
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
					("IF(ra%d) %s:: ", 
					inf_idx, __FUNCTION__));
			}
		
			if (wrq->u.data.length != sizeof(UCHAR))
				Status = -EINVAL;
			else
			{
				Status = copy_from_user(&wdev->ez_driver_params.enable_easy_setup, wrq->u.data.pointer, wrq->u.data.length);
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:: enable_easy_setup = %d.\n", 
					__FUNCTION__, wdev->ez_driver_params.enable_easy_setup));
			}
			break;
		
			case OID_WH_EZ_GROUP_ID:
				inf_idx = pObj->ioctl_if;
#ifdef APCLI_SUPPORT
				if (pObj->ioctl_if_type == INT_APCLI) {
					wdev = &pAd->ApCfg.ApCliTab[inf_idx].wdev;
				if (!pAd->ApCfg.ApCliTab[inf_idx].Enable)
					return FALSE;
		
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
					 ("IF(apcli%d) %s:: ", 
					 inf_idx, __FUNCTION__));
				}
				else 
#endif /* APCLI_SUPPORT */
				{
					wdev = &pAd->ApCfg.MBSSID[inf_idx].wdev;
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
						("IF(ra%d) %s:: ", 
						inf_idx, __FUNCTION__));
				}
				if (wdev->ez_driver_params.ezdev) {
					ez_set_ezgroup_id_hook((ez_dev_t *)wdev->ez_driver_params.ezdev, wrq->u.data.pointer, wrq->u.data.length, inf_idx);
					ez_set_group_id_hook((ez_dev_t *)wdev->ez_driver_params.ezdev, wrq->u.data.pointer, wrq->u.data.length, inf_idx);
				} else {
					return FALSE;
				}
				break;
		
				case OID_WH_EZ_GEN_GROUP_ID:
					inf_idx = pObj->ioctl_if;
#ifdef APCLI_SUPPORT
					if (pObj->ioctl_if_type == INT_APCLI) {
						wdev = &pAd->ApCfg.ApCliTab[inf_idx].wdev;
						if (!pAd->ApCfg.ApCliTab[inf_idx].Enable)
							return FALSE;
		
						MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
							("IF(apcli%d) %s:: ", 
							inf_idx, __FUNCTION__));
					}
					else 
#endif /* APCLI_SUPPORT */
					{
						wdev = &pAd->ApCfg.MBSSID[inf_idx].wdev;
						MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
							("IF(ra%d) %s:: ", 
							inf_idx, __FUNCTION__));
					}
					if (wrq->u.data.length == 0) {
						Status = -EINVAL;
					}
					else {
						if (wdev->ez_driver_params.ezdev) {
							Status = ez_set_gen_group_id_hook((ez_dev_t *)wdev->ez_driver_params.ezdev, 
								wrq->u.data.pointer, wrq->u.data.length, inf_idx);
						} else {							
							Status = -EINVAL;
						}
					}
					break;
		
				case OID_WH_EZ_RSSI_THRESHOLD:
					inf_idx = pObj->ioctl_if;
#ifdef APCLI_SUPPORT
					if (pObj->ioctl_if_type == INT_APCLI) {
						wdev = &pAd->ApCfg.ApCliTab[inf_idx].wdev;
						if (!pAd->ApCfg.ApCliTab[inf_idx].Enable)
							return FALSE;
		
						MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
							("IF(apcli%d) %s:: ", 
							inf_idx, __FUNCTION__));
					}
					else 
#endif /* APCLI_SUPPORT */
					{
						wdev = &pAd->ApCfg.MBSSID[inf_idx].wdev;
						MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
							("IF(ra%d) %s:: ", 
							inf_idx, __FUNCTION__));
					}
					if (wrq->u.data.length != sizeof(CHAR))
						Status = -EINVAL;
					else
					{
						if (wdev->ez_driver_params.ezdev){
							ez_dev_t *ezdev = wdev->ez_driver_params.ezdev;
							char rssi_val;
							Status = copy_from_user(&rssi_val, wrq->u.data.pointer, wrq->u.data.length);
							ezdev->ez_security.rssi_threshold = -1 * rssi_val;
							MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:: rssi_threshold = %d.\n", 
								__FUNCTION__, wdev->ez_driver_params.enable_easy_setup));
							}
					}
					break;
					
				case OID_WH_EZ_INTERNET_COMMAND:
					if (wdev->ez_driver_params.ezdev){
						Status = ez_internet_msg_handle(ad_obj, wrq);
						
					}  else {
						Status = -NDIS_STATUS_FAILURE;
					} 
					break;

#ifdef EZ_PUSH_BW_SUPPORT
				case OID_WH_EZ_PUSH_BW:
				{
					//PRTMP_ADAPTER adOthBand = NULL;

					if (wrq->u.data.length != sizeof(CHAR))
						Status = -EINVAL;
					else
					{
						BOOLEAN push_bw_config;
						Status = copy_from_user(&push_bw_config, wrq->u.data.pointer, wrq->u.data.length);
						if (wdev->ez_driver_params.ez_ad) {
							((EZ_ADAPTER *)(wdev->ez_driver_params.ez_ad))->push_bw_config = push_bw_config;
							MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:: push_bw_config = %d.\n", 
							__FUNCTION__, ((EZ_ADAPTER *)(wdev->ez_driver_params.ez_ad))->push_bw_config));
						
						}
					}
				}
					break;
#endif						
				case OID_WH_EZ_CUSTOM_DATA_CMD:
				{		
					inf_idx = pObj->ioctl_if;

					Status = Custom_DataHandle(pAd, wrq);  
	
					if(Status != NDIS_STATUS_SUCCESS)
					{
						Status = -NDIS_STATUS_FAILURE;
					}
					
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						("OID_WH_EZ_CUSTOM_DATA_CMD Status : %d\n", Status));
				}				 
				break;
#ifdef EZ_REGROUP_SUPPORT
				case OID_WH_EZ_REGROUP_COMMAND:
				
					printk("===>%s OID_WH_EZ_REGROUP_COMMAND\n",__FUNCTION__);
					Status = ez_handle_regroup_set_cmd(pAd, wrq);
				
					break;
#endif

				default:
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set::unknown IOCTL's subcmd = 0x%08x\n", cmd));
					Status = -EOPNOTSUPP;
					break;	
	}
	return Status;
}

// Arvind : Todo
int ez_parse_query_command(void *ad_obj, RTMP_IOCTL_INPUT_STRUCT *wrq, IN int cmd)
{
	NDIS_802_11_AUTHENTICATION_MODE	AuthMode;
	NDIS_802_11_WEP_STATUS	WepStatus;
    RTMP_ADAPTER *pAd;
    struct wifi_dev *wdev;
    UCHAR ifIndex;
    POS_COOKIE  pObj;	
    UCHAR apidx;
    BOOLEAN apcliEn=FALSE;
    int Status = NDIS_STATUS_SUCCESS;
	struct ez_GUI_info GUI_info;
	struct wifi_dev *apcli_wdev;
	APCLI_STRUCT *apcli_entry;
    pAd = (RTMP_ADAPTER *)ad_obj;
    pObj = (POS_COOKIE) pAd->OS_Cookie;

    apidx = pObj->ioctl_if;	
	ifIndex = pObj->ioctl_if;

	AuthMode = Ndis802_11AuthModeOpen;
	WepStatus = Ndis802_11WEPDisabled;

	
    switch(cmd & 0x7FFF)
    {
	case OID_WH_EZ_ENABLE:
		ifIndex = pObj->ioctl_if;
#ifdef APCLI_SUPPORT
		if (pObj->ioctl_if_type == INT_APCLI) {
			wdev = &pAd->ApCfg.ApCliTab[ifIndex].wdev;
			apcliEn = pAd->ApCfg.ApCliTab[ifIndex].Enable;
			if (!apcliEn)
				return FALSE;
	
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
				("IF(apcli%d) %s:: ", 
				ifIndex, __FUNCTION__));
		}
		else 
#endif /* APCLI_SUPPORT */
		{
			wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
				("IF(ra%d) %s:: ", 
				ifIndex, __FUNCTION__));
		}
			wrq->u.data.length = sizeof(wdev->ez_driver_params.enable_easy_setup);
			Status = copy_to_user(wrq->u.data.pointer, &wdev->ez_driver_params.enable_easy_setup, wrq->u.data.length);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
					("Status = %d, enable_easy_setup = %d\n", 
					Status, wdev->ez_driver_params.enable_easy_setup));
			break;
	
	case OID_WH_EZ_GROUP_ID:
			ifIndex = pObj->ioctl_if;
#ifdef APCLI_SUPPORT
			if (pObj->ioctl_if_type == INT_APCLI) {
				wdev = &pAd->ApCfg.ApCliTab[ifIndex].wdev;
				apcliEn = pAd->ApCfg.ApCliTab[ifIndex].Enable;
				if (!apcliEn)
					return FALSE;
	
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
				("IF(apcli%d) %s:: ", 
				ifIndex, __FUNCTION__));
			}
			else 
#endif /* APCLI_SUPPORT */
			{
				wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
					("IF(ra%d) %s:: ", 
					ifIndex, __FUNCTION__));
			}
				wrq->u.data.length = ((ez_dev_t *)(wdev->ez_driver_params.ezdev))->ez_security.group_id_len;
				Status = copy_to_user(wrq->u.data.pointer, 
					((ez_dev_t *)(wdev->ez_driver_params.ezdev))->ez_security.group_id, 
					((ez_dev_t *)(wdev->ez_driver_params.ezdev))->ez_security.group_id_len);
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
					("Status = %d, OID_WH_EZ_GROUP_ID\n", 
					Status));
				break;
	
	case OID_WH_EZ_RSSI_THRESHOLD:
			ifIndex = pObj->ioctl_if;
#ifdef APCLI_SUPPORT
			if (pObj->ioctl_if_type == INT_APCLI) {
				wdev = &pAd->ApCfg.ApCliTab[ifIndex].wdev;
				apcliEn = pAd->ApCfg.ApCliTab[ifIndex].Enable;
				if (!apcliEn)
					return FALSE;
	
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
				("IF(apcli%d) %s:: ", 
				ifIndex, __FUNCTION__));
			}
			else 
#endif /* APCLI_SUPPORT */
			{
				wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
					("IF(ra%d) %s:: ", 
					ifIndex, __FUNCTION__));
			}
	
			wrq->u.data.length = sizeof(((ez_dev_t *)(wdev->ez_driver_params.ezdev))->ez_security.rssi_threshold);
			Status = copy_to_user(wrq->u.data.pointer, 
				&((ez_dev_t *)(wdev->ez_driver_params.ezdev))->ez_security.rssi_threshold, 
				wrq->u.data.length);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
				("Status = %d, rssi_threshold = %d\n", 
				Status, ((ez_dev_t *)(wdev->ez_driver_params.ezdev))->ez_security.rssi_threshold));
			break;

	case  OID_WH_EZ_GET_GUI_INFO:

			ifIndex = pObj->ioctl_if;
#ifdef APCLI_SUPPORT
			apcli_entry = &pAd->ApCfg.ApCliTab[ifIndex];
			apcli_wdev = &pAd->ApCfg.ApCliTab[ifIndex].wdev;
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("IF(apcli%d) %s:: ", ifIndex, __FUNCTION__));
#endif /* APCLI_SUPPORT */

			wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("IF(ra%d) %s:: ", ifIndex, __FUNCTION__));

			memset(&GUI_info, '\0', sizeof(GUI_info));
			NdisCopyMemory(&GUI_info.EzEnable, &wdev->ez_driver_params.enable_easy_setup, sizeof(wdev->ez_driver_params.enable_easy_setup));
			//NdisCopyMemory(&GUI_info.EzConfStatus, &ez_adapter.configured_status, sizeof(ez_adapter.configured_status));
			if (IS_EZ_SETUP_ENABLED(wdev))
			{
			NdisCopyMemory(GUI_info.EzGroupID, 
			((ez_dev_t *)(wdev->ez_driver_params.ezdev))->ez_security.ez_group_id, 
			((ez_dev_t *)(wdev->ez_driver_params.ezdev))->ez_security.ez_group_id_len);
			NdisCopyMemory(GUI_info.EzGenGroupId, 
				((ez_dev_t *)(wdev->ez_driver_params.ezdev))->ez_security.gen_group_id, 
				((ez_dev_t *)(wdev->ez_driver_params.ezdev))->ez_security.gen_group_id_len);
			NdisCopyMemory(GUI_info.EzOpenGroupID, 
				((ez_dev_t *)(wdev->ez_driver_params.ezdev))->ez_security.open_group_id, 
				((ez_dev_t *)(wdev->ez_driver_params.ezdev))->ez_security.open_group_id_len);
#ifdef APCLI_SUPPORT
			//NdisCopyMemory(&GUI_info.ApCliEzConfStatus, &ez_adapter.configured_status, sizeof(ez_adapter.configured_status));
			NdisCopyMemory(GUI_info.ApCliEzGroupID, 
				((ez_dev_t *)(apcli_wdev->ez_driver_params.ezdev))->ez_security.ez_group_id, 
				((ez_dev_t *)(apcli_wdev->ez_driver_params.ezdev))->ez_security.ez_group_id_len);
			NdisCopyMemory(GUI_info.ApCliEzGenGroupId, 
				((ez_dev_t *)(apcli_wdev->ez_driver_params.ezdev))->ez_security.gen_group_id, 
				((ez_dev_t *)(apcli_wdev->ez_driver_params.ezdev))->ez_security.gen_group_id_len);
			NdisCopyMemory(GUI_info.ApCliEzOpenGroupID, 
				((ez_dev_t *)(apcli_wdev->ez_driver_params.ezdev))->ez_security.open_group_id, 
				((ez_dev_t *)(apcli_wdev->ez_driver_params.ezdev))->ez_security.open_group_id_len);
			NdisCopyMemory(GUI_info.ApCliHideSSID, apcli_entry->CfgHideSsid, apcli_entry->CfgHideSsidLen);

			AuthMode = apcli_wdev->AuthMode;	
			if ((AuthMode >= Ndis802_11AuthModeOpen) && 
					(AuthMode <= Ndis802_11AuthModeWPA1PSKWPA2PSK))
				NdisCopyMemory(GUI_info.ApCliAuthMode, RTMPGetRalinkAuthModeStr(AuthMode), strlen(RTMPGetRalinkAuthModeStr(AuthMode)));
							
			WepStatus = apcli_wdev->WepStatus;
			if ((WepStatus >= Ndis802_11WEPEnabled) && 
					(WepStatus <= Ndis802_11Encryption4KeyAbsent))
				NdisCopyMemory(GUI_info.ApCliEncrypType, RTMPGetRalinkEncryModeStr(WepStatus), strlen(RTMPGetRalinkEncryModeStr(WepStatus)));

			NdisCopyMemory(GUI_info.ApCliWPAPSK, &pAd->ApCfg.ApCliTab[ifIndex].PSK, strlen(pAd->ApCfg.ApCliTab[ifIndex].PSK));

			NdisCopyMemory(GUI_info.ApCliSsid, apcli_entry->CfgSsid, apcli_entry->CfgSsidLen);
			NdisCopyMemory(&GUI_info.ApCliEnable, &pAd->ApCfg.ApCliTab[ifIndex].Enable, sizeof(pAd->ApCfg.ApCliTab[ifIndex].Enable));
			NdisCopyMemory(&GUI_info.ApCliEzEnable, &apcli_wdev->ez_driver_params.enable_easy_setup, sizeof(apcli_wdev->ez_driver_params.enable_easy_setup));
#endif
			}
			wrq->u.data.length = sizeof(GUI_info);
			Status = copy_to_user(wrq->u.data.pointer, &GUI_info, sizeof(GUI_info));

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Status = %d, OID_WH_EZ_GROUP_ID\n", Status));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("sizeof(GUI_info):%d\n", sizeof(GUI_info)));
			
		break;

		case  OID_WH_EZ_UPDATE_STA_INFO:
		{
			int sta_cnt;
			int i = 0;
			char buf[256];
			MAC_TABLE_ENTRY *pMacEntry = NULL;
			struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
			EZ_ADAPTER *ez_ad = wdev->ez_driver_params.ez_ad;

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("\n OID_WH_EZ_UPDATE_STA_INFO \n"));
			Status = copy_from_user(&buf[0], wrq->u.data.pointer, wrq->u.data.length);
				
			
			sta_cnt = wrq->u.data.length / 6;
			for (i = 0; i < sta_cnt; i ++)
			{
				if (ez_is_triband())
				{

					pMacEntry = MacTableLookup2(ez_ad->non_ez_band_info[0].pAd,&buf[i*MAC_ADDR_LEN],NULL);
					if(pMacEntry != NULL){
						MacTableDeleteEntry(ez_ad->non_ez_band_info[0].pAd,pMacEntry->wcid,pMacEntry->Addr);
						pMacEntry = NULL;
					}

					
					pMacEntry = MacTableLookup2(ez_ad->non_ez_band_info[1].pAd,&buf[i*MAC_ADDR_LEN],NULL);
					if(pMacEntry != NULL){
						MacTableDeleteEntry(ez_ad->non_ez_band_info[1].pAd,pMacEntry->wcid,pMacEntry->Addr);
						pMacEntry = NULL;
					}
				} else {
					pMacEntry = MacTableLookup2(ez_ad->ez_band_info[0].pAd,&buf[i*MAC_ADDR_LEN],NULL);
					if(pMacEntry != NULL){
						MacTableDeleteEntry(ez_ad->ez_band_info[0].pAd,pMacEntry->wcid,pMacEntry->Addr);
						pMacEntry = NULL;
					}
					

					if(ez_ad->ez_band_info[1].pAd)
					{
						pMacEntry = MacTableLookup2(ez_ad->ez_band_info[1].pAd,&buf[i*MAC_ADDR_LEN],NULL);
						if(pMacEntry != NULL){
							MacTableDeleteEntry(ez_ad->ez_band_info[1].pAd,pMacEntry->wcid,pMacEntry->Addr);
							pMacEntry = NULL;
						}				

					}
				}
			}
			RtmpOSWrielessEventSend(pAd->net_dev, RT_WLAN_EVENT_CUSTOM, OID_WH_EZ_UPDATE_STA_INFO,
				NULL, buf, wrq->u.data.length);
			
			break;
		}
#ifdef EZ_REGROUP_SUPPORT
		case OID_WH_EZ_REGROUP_COMMAND:
			printk("===>%s OID_WH_EZ_REGROUP_COMMAND\n",__FUNCTION__);
			Status = ez_handle_regroup_query_cmd(pAd, wrq);
			break;
#endif
			
		default:
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
				("Query::unknown IOCTL's subcmd = 0x%08x, apidx=%d\n", cmd, apidx));
			Status = -EOPNOTSUPP;
			break;	
    }
	
    return Status;
}

void ez_vendor_ie_parse(struct _vendor_ie_cap *vendor_ie, PEID_STRUCT info_elem)
{

	vendor_ie->support_easy_setup = TRUE;

	if (info_elem->Octet[EZ_TAG_OFFSET]
		== EZ_TAG_CAPABILITY_INFO) {
		NdisCopyMemory(&vendor_ie->ez_capability, 
				&info_elem->Octet[EZ_TAG_DATA_OFFSET], 
				EZ_CAPABILITY_LEN);
		vendor_ie->ez_capability = be2cpu32(vendor_ie->ez_capability);
	}
	if (info_elem->Octet[EZ_TAG_OFFSET]
	== EZ_TAG_BEACON_INFO) {
		NdisCopyMemory(&vendor_ie->beacon_info,&info_elem->Octet[EZ_TAG_DATA_OFFSET], info_elem->Octet[EZ_TAG_LEN_OFFSET]);
	}

	if (info_elem->Octet[EZ_TAG_OFFSET]
	== EZ_TAG_NON_EZ_BEACON) {
		vendor_ie->non_ez_beacon = TRUE;
	}

	if (info_elem->Octet[EZ_TAG_OFFSET]
		== EZ_TAG_OPEN_GROUP_ID) {
		NdisCopyMemory(vendor_ie->open_group_id, 
				&info_elem->Octet[EZ_TAG_DATA_OFFSET], 
				info_elem->Octet[EZ_TAG_LEN_OFFSET]);
		vendor_ie->open_group_id_len = info_elem->Octet[EZ_TAG_LEN_OFFSET];
	}

}

void ez_set_ap_fallback_context(struct wifi_dev *wdev, BOOLEAN fallback, unsigned char fallback_channel)
{
	if (wdev->ez_driver_params.ezdev){
		ez_dev_t *ezdev = wdev->ez_driver_params.ezdev;
		EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("\nez_set_ap_fallback_context: ezdev idx %x, type %x, fallback %x, chan %d\n",
			ezdev->ez_band_idx, ezdev->ezdev_type, fallback, fallback_channel));
		ezdev->ez_security.ap_did_fallback = fallback;
		ezdev->ez_security.fallback_channel = fallback_channel;

		//ez_set_ap_fallback_context_hook((ez_dev_t *)wdev->ez_driver_params.ezdev, fallback,  fallback_channel);
	}
}

void *ez_peer_table_search_by_addr(
	struct wifi_dev *wdev,
	unsigned char *addr)
{
	if (wdev->ez_driver_params.ezdev)
		return ez_peer_table_search_by_addr_hook((ez_dev_t *)wdev->ez_driver_params.ezdev, addr);
	return NULL;
}

void ez_set_delete_peer_in_differed_context(struct wifi_dev *wdev, void *ez_peer_obj, BOOLEAN set)
{
	struct _ez_peer_security_info *ez_peer = ez_peer_obj; 
	if (wdev->ez_driver_params.ezdev){
		if (set)
		{
			ez_peer->delete_in_differred_context = TRUE;
		} else {		
			ez_peer->delete_in_differred_context = FALSE;
		}
	}
}

#ifdef APCLI_AUTO_CONNECT_SUPPORT
BOOLEAN ez_probe_count_handle(PAPCLI_STRUCT pApCliEntry)
{
	BOOLEAN attempt_same_peer= FALSE;
    pApCliEntry->ProbeReqCnt = 0;
	return attempt_same_peer;
}
#endif

BOOLEAN ez_ApCliAutoConnectBWAdjust(
	IN void	*ad_obj,
	IN struct wifi_dev	*wdev,
	IN void		*bss_entry_obj)
{
	RTMP_ADAPTER *pAd = ad_obj;
	BSS_ENTRY		*bss_entry = bss_entry_obj;
    BOOLEAN bAdjust = FALSE;
	BOOLEAN bAdjust_by_channel = FALSE;
	BOOLEAN bAdjust_by_ht = FALSE;
	BOOLEAN bAdjust_by_vht = FALSE;
    UCHAR 	orig_op_ht_bw;
	UCHAR 	orig_op_vht_bw;
	UCHAR	orig_ext_cha;
	EZ_ADAPTER *ez_ad = wdev->ez_driver_params.ez_ad;

	if (pAd == NULL || wdev == NULL || bss_entry == NULL) {
		EZ_DEBUG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
            	          ("(%s)  Error! entry is NULL.\n", __FUNCTION__));
		return FALSE;
	}

	EZ_DEBUG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
        	          ("BW info of root AP (%s):\n",bss_entry->Ssid));
	orig_op_ht_bw = ez_driver_ops_wlan_config_get_ht_bw_mt7603(wdev->ez_driver_params.ezdev);
	orig_op_vht_bw = ez_driver_ops_wlan_config_get_vht_bw_mt7603(wdev->ez_driver_params.ezdev);
	orig_ext_cha = ez_driver_ops_wlan_config_get_ext_cha_mt7603(wdev->ez_driver_params.ezdev);

    if((pAd->CommonCfg.Channel != bss_entry->Channel)) {
		bAdjust = TRUE;
		bAdjust_by_channel = TRUE;
	}

#ifdef DOT11_N_SUPPORT
    if (WMODE_CAP_N(wdev->PhyMode) && (bss_entry->AddHtInfoLen != 0))
    {
        ADD_HTINFO *add_ht_info = &bss_entry->AddHtInfo.AddHtInfo;
		UCHAR op_ht_bw = ez_driver_ops_wlan_config_get_ht_bw_mt7603(wdev->ez_driver_params.ezdev);
		UCHAR ext_cha = ez_driver_ops_wlan_config_get_ext_cha_mt7603(wdev->ez_driver_params.ezdev);
        EZ_DEBUG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
            	          ("(%s)op_ht_bw = %d, ext_cha = %d, recommended = %d\n", __FUNCTION__, op_ht_bw, ext_cha, add_ht_info->RecomWidth));
        if(!bAdjust && 
           ((ext_cha != add_ht_info->ExtChanOffset) ||
			(op_ht_bw != add_ht_info->RecomWidth))) {
			EZ_DEBUG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("Need to adjust bandwidth\n"));
			bAdjust = TRUE;
		}

        if (bAdjust)
        {
			switch (add_ht_info->RecomWidth) //peer side vht bw
			{
				case BW_20:
					if (op_ht_bw == BW_40) {
						EZ_DEBUG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("Bring it down to 20\n"));
#ifdef EZ_PUSH_BW_SUPPORT
						if( ez_ad->push_bw_config )
						{
							ez_driver_ops_wlan_config_set_ht_bw_mt7603(wdev->ez_driver_params.ezdev, add_ht_info->RecomWidth);
						}
#endif
						bAdjust_by_ht = TRUE;
					}
					break;
				case BW_40:
					if (op_ht_bw == BW_20) {
							EZ_DEBUG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("bring it up to 40 ans set EXTXHA\n"));
#ifdef EZ_PUSH_BW_SUPPORT
							if( ez_ad->push_bw_config )
							{
								ez_driver_ops_wlan_config_set_ht_bw_mt7603(wdev->ez_driver_params.ezdev, add_ht_info->RecomWidth);
							}
#endif
							ez_driver_ops_wlan_config_set_ext_cha_mt7603(wdev->ez_driver_params.ezdev, add_ht_info->ExtChanOffset);
							bAdjust_by_ht = TRUE;
					} else {
							EZ_DEBUG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("just set the EXTCHA\n"));
							ez_driver_ops_wlan_config_set_ext_cha_mt7603(wdev->ez_driver_params.ezdev, add_ht_info->ExtChanOffset);
							bAdjust_by_ht = TRUE;
					}
					break;
			}
        }
    }
#endif /* DOT11_N_SUPPORT */


#ifdef DOT11_VHT_AC
    if (WMODE_CAP_AC(wdev->PhyMode) && IS_CAP_BW160(pAd) && (bss_entry->vht_cap_len != 0) && (bss_entry->vht_op_len !=0))
    {
		VHT_OP_INFO *vht_op = &bss_entry->vht_op_ie.vht_op_info;
		UCHAR bw = VHT_BW_2040;
#ifdef DOT11_VHT_R2
        bw = check_vht_op_bw (vht_op);
#else
        bw = vht_op->ch_width;
        print_vht_op_info(vht_op);
#endif /* DOT11_VHT_R2 */

#ifdef EZ_PUSH_BW_SUPPORT
		if( ez_ad->push_bw_config )
		{
#ifdef DOT11_VHT_AC		
			wlan_config_set_vht_bw(wdev, bw);
#endif
		}
#endif

    }
#endif /* DOT11_VHT_AC */
	bAdjust = FALSE;
	if (bAdjust_by_channel == TRUE)
		bAdjust = TRUE;
	if (bAdjust_by_ht == TRUE)
		bAdjust = TRUE;
	if (bAdjust_by_vht == TRUE)
		bAdjust = TRUE;
	if (bAdjust) {
		EZ_DEBUG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s:Adjust (%d %d %d)\n\r",__func__,
			bAdjust_by_channel,bAdjust_by_ht,bAdjust_by_vht));
		EZ_DEBUG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s:HT BW:%d to %d. MAX(%d)\n\r",__func__,
			orig_op_ht_bw, ez_driver_ops_wlan_operate_get_ht_bw_mt7603(wdev->ez_driver_params.ezdev),ez_driver_ops_wlan_config_get_ht_bw_mt7603(wdev->ez_driver_params.ezdev)));
#ifdef 	DOT11_VHT_AC
		EZ_DEBUG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s:VHT BW:%d to %d. MAX(%d)\n\r",__func__,
				 orig_op_vht_bw,ez_driver_ops_wlan_operate_get_vht_bw_mt7603(wdev->ez_driver_params.ezdev),wlan_config_get_vht_bw(wdev)));
#endif
		EZ_DEBUG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s:EXT CH:%d to %d. CFG(%d)\n\r",__func__,
				 orig_ext_cha,ez_driver_ops_wlan_operate_get_ext_cha_mt7603(wdev->ez_driver_params.ezdev),ez_driver_ops_wlan_config_get_ext_cha_mt7603(wdev->ez_driver_params.ezdev)));
	}
	
	SetCommonHT(pAd);
	return bAdjust;
}

BOOLEAN ez_is_triband()
{
	return ez_is_triband_hook();
}

unsigned short ez_check_for_ez_enable(
	struct wifi_dev *wdev,
	void *msg,
	unsigned long msg_len
	)
{
	return ez_check_for_ez_enable_hook(msg, msg_len);
}

BOOLEAN ez_is_roam_blocked_mac(struct wifi_dev *wdev, UCHAR *mac_addr)
{
	if (wdev->ez_driver_params.ezdev){
		ez_dev_t *ezdev = wdev->ez_driver_params.ezdev;
		return MAC_ADDR_EQUAL(ezdev->ez_security.ez_ap_roam_blocked_mac
			, mac_addr);
	}
	return FALSE;
}
void ez_peer_table_maintenance(PRTMP_ADAPTER ad)
{
	ULONG now;
	int i;
	
	if (ad->ez_ad)
		ez_peer_table_maintenance_hook((EZ_ADAPTER *)ad->ez_ad);
	
	NdisGetSystemUpTime(&now);

	for (i = 0; i <  ad->ApCfg.BssidNum; i++)
	{
		struct wifi_dev * ez_wdev = &ad->ApCfg.ApCliTab[i].wdev;

		if (ez_wdev->ez_driver_params.ezdev 
				&& ez_wdev->ez_driver_params.ez_wps_reconnect 
				&& RTMP_TIME_AFTER(now, ez_wdev->ez_driver_params.ez_wps_reconnect_timestamp + 60*OS_HZ))
		{
			ez_wdev->ez_driver_params.ez_wps_reconnect = FALSE;
			NdisZeroMemory(ez_wdev->ez_driver_params.ez_wps_bssid,MAC_ADDR_LEN);
		}
	}
}

BOOLEAN ez_port_secured(
	void *ad_obj,
	void *entry_obj,
	unsigned char if_idx,
	unsigned char ap_mode)
{

	MAC_TABLE_ENTRY *entry;
	struct wifi_dev *wdev;
	entry = (MAC_TABLE_ENTRY *)entry_obj;
	wdev = entry->wdev;

	entry->easy_setup_enabled = TRUE;	

	if (wdev->wdev_type == WDEV_TYPE_STA)
	{
		APCLI_STRUCT *apcli_entry = wdev->func_dev;
		apcli_entry->MlmeAux.attempted_candidate_index = EZ_INDEX_NOT_FOUND;
	}

	if (wdev->ez_driver_params.ezdev)
		return ez_port_secured_hook((ez_dev_t *)wdev->ez_driver_params.ezdev, entry->Addr,ap_mode);
	return 0;
}

void increment_best_ap_rssi_threshold(struct wifi_dev *wdev)
{
	if (wdev->ez_driver_params.ezdev)
		increment_best_ap_rssi_threshold_hook(wdev->ez_driver_params.ezdev);
}

UCHAR ez_get_delay_disconnect_count(struct wifi_dev *wdev)
{
	if (wdev->ez_driver_params.ezdev)
	{
		ez_dev_t *ezdev = wdev->ez_driver_params.ezdev;
		return ezdev->ez_security.delay_disconnect_count;
	}
	return 0;
}
#ifdef IF_UP_DOWN
BOOLEAN ez_all_intf_up(void *ez_ad)
{
	return ez_all_intf_up_hook(ez_ad);
}

void ez_apcli_disconnect_both_intf(struct wifi_dev *wdev)
{
	ez_dev_t *ezdev = wdev->ez_driver_params.ezdev;

	ez_apcli_disconnect_both_intf_hook(ezdev, wdev->bssid);
}

BOOLEAN ez_apcli_check_beacon_params(struct wifi_dev *wdev, beacon_info_tag_t beacon_info)
{
	ez_dev_t *ezdev = wdev->ez_driver_params.ezdev;
	EZ_ADAPTER *ez_ad = NULL;
	struct _ez_peer_security_info *ez_peer = NULL;

	if (ezdev == NULL) {
		EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s(line.%d) - Ezdev NULL.\n",
					__func__, __LINE__));
		return FALSE;
	}
	ez_ad = ezdev->ez_ad;
	/*check the node number and network weight of beacon with the ones stored with us.*/

	ez_peer = ez_peer_table_search_by_addr(wdev, wdev->bssid);


	if (ez_peer == NULL || ez_ad == NULL) {
		/*Connected to third party AP*/
		return TRUE;
	}

	if (!NdisEqualMemory(ez_ad->device_info.network_weight, beacon_info.network_weight, NETWORK_WEIGHT_LEN)
		|| !((ez_peer->device_info.ez_node_number.path_len == beacon_info.node_number.path_len)
			&& NdisEqualMemory(ez_peer->device_info.ez_node_number.root_mac,
						beacon_info.node_number.root_mac,
						beacon_info.node_number.path_len))
	) {

		hex_dump("EzPeerWt", (PUCHAR)ez_ad->device_info.network_weight, NETWORK_WEIGHT_LEN);
		hex_dump("BcnWt", (PUCHAR)beacon_info.network_weight, NETWORK_WEIGHT_LEN);

		hex_dump("EzPeerNN", (PUCHAR)&ez_peer->device_info.ez_node_number, sizeof(EZ_NODE_NUMBER));
		hex_dump("BcnNN", (PUCHAR)&beacon_info.node_number, sizeof(EZ_NODE_NUMBER));
		return FALSE;
	}
	return TRUE;
}

#endif

void ez_set_delay_disconnect_count(struct wifi_dev *wdev, unsigned char count)
{
	if (wdev->ez_driver_params.ezdev)
	{
		ez_dev_t *ezdev = wdev->ez_driver_params.ezdev;
		ezdev->ez_security.delay_disconnect_count =  count;
	}
}

void ez_acquire_lock(PRTMP_ADAPTER pAd, struct wifi_dev *wdev,unsigned char lock_id)
{
	if (pAd)
	{
		ez_acquire_lock_hook(pAd->ez_ad, NULL, lock_id);	
	} else if (wdev)
	{
		ez_acquire_lock_hook(NULL, (ez_dev_t *)wdev->ez_driver_params.ezdev, lock_id);	
	}
}


void ez_release_lock(PRTMP_ADAPTER pAd, struct wifi_dev *wdev,unsigned char lock_id)
{
	if (pAd)
	{
		ez_release_lock_hook((EZ_ADAPTER *)pAd->ez_ad, NULL, lock_id);	
	} else if (wdev)
	{
		ez_release_lock_hook(NULL, (ez_dev_t *)wdev->ez_driver_params.ezdev, lock_id);	
	}
}

BOOLEAN ez_is_weight_same(struct wifi_dev *wdev, UCHAR * weight)
{
	if (wdev->ez_driver_params.ezdev)
	{
		return ez_is_weight_same_hook((ez_dev_t *)wdev->ez_driver_params.ezdev, weight);
	}
	return FALSE;
}

BOOLEAN ez_is_other_band_mlme_running(struct wifi_dev *wdev)
{
	if (!IS_CONF_EZ_SETUP_ENABLED(wdev))
		return FALSE;

	if (wdev->ez_driver_params.ezdev)
		return ez_is_other_band_mlme_running_hook((ez_dev_t *)wdev->ez_driver_params.ezdev);
	return FALSE;
}

BOOLEAN ez_handle_scan_channel_restore(struct wifi_dev *wdev)
{
	if (wdev) {
		if (wdev->ez_driver_params.scan_one_channel) {
			wdev->ez_driver_params.scan_one_channel = FALSE;
			return TRUE;
		}
	}
	return FALSE;
}

void ez_triband_insert_tlv(PRTMP_ADAPTER pAd, unsigned int tag_ID, UCHAR * buffer, ULONG* tag_len)
{
	if (pAd->ez_ad)
		ez_triband_insert_tlv_hook((EZ_ADAPTER *)pAd->ez_ad, tag_ID, buffer,tag_len);
}


void ez_ap_peer_beacon_action(struct wifi_dev *ez_wdev, unsigned char * mac_addr, int capability, CHAR *RealRssi, BCN_IE_LIST *ie_list)
{
	//RTMP_ADAPTER *pAd = ez_wdev->sys_handle;
	
	if (ez_wdev->ez_driver_params.ezdev) {
		ez_dev_t * ezdev = ez_wdev->ez_driver_params.ezdev;
		ez_ap_peer_beacon_action_hook(ezdev, mac_addr, capability);

		
#ifdef EZ_REGROUP_SUPPORT

		if(IS_REGRP_SUPP(ez_wdev) && ez_wdev->channel == ie_list->Channel)
		{
			UINT8 band;
			//UINT8 mac[6]= {0x20,0xaa,0x4b,0x6a,0x36,0x63};
			//printk("My SSID = %s\n", ezdev->ez_security.this_band_info.shared_info.ssid);
			if(ez_wdev->channel <= 14)
				band =0;
			else
				band = 1;
			if((ie_list->vendor_ie.support_easy_setup == 1 && 
				ie_list->vendor_ie.open_group_id_len == ezdev->ez_security.open_group_id_len
				&& NdisEqualMemory(ie_list->vendor_ie.open_group_id,ezdev->ez_security.open_group_id,ie_list->vendor_ie.open_group_id_len))
				||
				(ie_list->vendor_ie.support_easy_setup == 0 &&
				SSID_EQUAL(ezdev->ez_security.this_band_info.shared_info.ssid,
					ezdev->ez_security.this_band_info.shared_info.ssid_len, 
					ie_list->Ssid, ie_list->SsidLen))
				)
			{
				p_regrp_ap_info_struct ap_info = NULL;

				ap_info = ez_find_regrp_ap_by_bssid(regrp_ap_info[band],ie_list->Bssid);
				if(ap_info == NULL)
				{
					ap_info = ez_add_regrp_ap(regrp_ap_info[band]);
					printk("Added Entry: %02x:%02x:%02x:%02x:%02x:%02x, RSSI: %d\n",PRINT_MAC(ie_list->Bssid),*RealRssi);
				}
				if(ap_info != NULL)
				{
					ap_info->Non_MAN = !ie_list->vendor_ie.support_easy_setup;
					COPY_MAC_ADDR(ap_info->bssid,ie_list->Bssid);
					if(ap_info->Non_MAN == 0)
					{
						NdisCopyMemory(&ap_info->node_number,
							&ie_list->vendor_ie.beacon_info.node_number,
							sizeof(EZ_NODE_NUMBER));
						NdisCopyMemory(ap_info->nw_wt,
							ie_list->vendor_ie.beacon_info.network_weight,
							NETWORK_WEIGHT_LEN);
					}
					else
					{
						NdisZeroMemory(&ap_info->node_number,
							sizeof(EZ_NODE_NUMBER));
						NdisZeroMemory(ap_info->nw_wt,
							NETWORK_WEIGHT_LEN);
					}
					ap_info->last_rssi = *RealRssi;
					ap_info->rx_cnt++;
					ap_info->rssi_sum += *RealRssi;
					if(ap_info->rx_cnt <= 0)
					{
					   ap_info->rssi_sum = *RealRssi;
					   ap_info->rx_cnt = 1;
					}
					ap_info->avg_rssi = (CHAR)( (ap_info->rssi_sum)/(INT32)(ap_info->rx_cnt));
											NdisGetSystemUpTime(&ap_info->last_rx_time);
					
				
				}
			}
		}
#endif
	}
}

void ez_handle_peer_disconnection(struct wifi_dev *wdev, unsigned char * mac_addr)
{
	if (wdev->ez_driver_params.ezdev && !wdev->ez_driver_params.ez_wps_reconnect)
	{
		ez_handle_peer_disconnection_hook((ez_dev_t *)wdev->ez_driver_params.ezdev, mac_addr);
	}
}


int ez_handle_send_packets(struct wifi_dev *wdev, PNDIS_PACKET pPacket)
{

	//UCHAR *pDestAddr = GET_OS_PKT_DATAPTR(pPacket);
	int ret = 1;

	if (wdev->ez_driver_params.ezdev){
		ret =ez_handle_send_packets_hook((ez_dev_t *)wdev->ez_driver_params.ezdev, pPacket);
		if( ret == 0)		
			RELEASE_NDIS_PACKET(wdev->sys_handle, pPacket, NDIS_STATUS_FAILURE);
	}
	return ret;
}

BOOLEAN ez_sta_rx_pkt_handle(struct wifi_dev *wdev, RX_BLK *pRxBlk)
{
	if (wdev->ez_driver_params.ezdev)
		return ez_sta_rx_pkt_handle_hook((ez_dev_t *)wdev->ez_driver_params.ezdev,
			pRxBlk->pData, pRxBlk->MPDUtotalByteCnt);
	return 0;
}
BOOLEAN ez_apcli_rx_grp_pkt_drop(
	IN struct wifi_dev *wdev,
	IN struct sk_buff *pSkb)
{
	if (wdev->ez_driver_params.ezdev)
	{
		return ez_apcli_rx_grp_pkt_drop_hook((ez_dev_t *)wdev->ez_driver_params.ezdev,GET_OS_PKT_DATAPTR(pSkb));
	}
	ASSERT(FALSE);
	return FALSE;

}

BOOLEAN ez_apcli_tx_grp_pkt_drop(struct wifi_dev *wdev,struct sk_buff *pSkb)
{
	if (wdev->ez_driver_params.ezdev)
		return ez_apcli_tx_grp_pkt_drop_hook((ez_dev_t *)wdev->ez_driver_params.ezdev,pSkb);
	ASSERT(FALSE);
	return FALSE;
}



void ez_send_delay_disconnect_for_pbc(struct wifi_dev *wdev)
{
	PRTMP_ADAPTER ad = wdev->sys_handle;
	struct wifi_dev *ap_wdev = &ad->ApCfg.MBSSID[wdev->func_idx].wdev;

	if(ap_wdev->ez_driver_params.ezdev)
	{
		send_delay_disconnect_to_peers_hook((ez_dev_t *)ap_wdev->ez_driver_params.ezdev);
	} else {
		ASSERT(FALSE);
	}
}

//Arvind : To be handle



INT Set_EasySetup_Open_GenGroupID_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	POS_COOKIE  pObj;
    UCHAR	    apidx;
	UINT32 data_len;
	struct wifi_dev * wdev;
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	apidx = pObj->ioctl_if;

#ifdef APCLI_SUPPORT
    if (pObj->ioctl_if_type == INT_APCLI)
    {
		wdev = &pAd->ApCfg.ApCliTab[apidx].wdev;
		EZ_DEBUG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
			("IF(apcli%d) %s:: This command is from apcli interface now.\n", 
			apidx, __FUNCTION__));
    }
    else
#endif /* APCLI_SUPPORT */
	{
		wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
		EZ_DEBUG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
			("IF(ra%d) %s:: This command is from ra interface now.\n",
			apidx, __FUNCTION__));
	}

	data_len = strlen(arg);

	if (wdev->ez_driver_params.ezdev){
		return ez_set_open_group_id_hook((ez_dev_t*)wdev->ez_driver_params.ezdev, 
											arg, data_len, apidx);
	} else 
	{
		return FALSE;
	}
	return TRUE;
}

BOOLEAN ez_join_timeout_handle(void *ad_obj, unsigned char ifIndex)
{
	PRTMP_ADAPTER pAd = ad_obj;
	PAPCLI_STRUCT pApCliEntry;
	PULONG pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].CtrlCurrState;
	BSS_TABLE *aux_bss_table;

	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];	
	NdisZeroMemory(pApCliEntry->MlmeAux.Bssid, MAC_ADDR_LEN);
	NdisZeroMemory(pApCliEntry->MlmeAux.Ssid, MAX_LEN_OF_SSID);
	pApCliEntry->MlmeAux.SsidLen = 0;
	aux_bss_table = &pApCliEntry->MlmeAux.SsidBssTab;
    if((pApCliEntry->MlmeAux.attempted_candidate_index != EZ_INDEX_NOT_FOUND) &&
		(pApCliEntry->MlmeAux.attempted_candidate_index <= aux_bss_table->BssNr) ){
		aux_bss_table->BssEntry[pApCliEntry->MlmeAux.attempted_candidate_index].bConnectAttemptFailed = TRUE;
    }
	if (IS_EZ_SETUP_ENABLED(&pApCliEntry->wdev) && !ez_apcli_search_best_ap(pAd,pApCliEntry,ifIndex)){
		*pCurrState = APCLI_CTRL_DISCONNECTED;
		return FALSE;
			}
	return TRUE;
}

void ez_update_bss_entry(OUT BSS_ENTRY *pBss, IN BCN_IE_LIST *ie_list)
{
	NdisCopyMemory(&pBss->beacon_info,&ie_list->vendor_ie.beacon_info,sizeof(beacon_info_tag_t));
	NdisCopyMemory(pBss->open_group_id,ie_list->vendor_ie.open_group_id, ie_list->vendor_ie.open_group_id_len);
	pBss->open_group_id_len = ie_list->vendor_ie.open_group_id_len;
}



VOID APTribandUpdateChannelReqAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	printk("Not Implemented\n");
	return;	
}


VOID EzTribandRestartNonEzReqAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{

	if (pAd->ez_ad)
		APTribandRestartNonEzReqAction_hook((EZ_ADAPTER *)pAd->ez_ad);
}



//! Levarage from MP1.0 CL#170063
void EzRoamReqAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	
	struct wifi_dev *wdev = (struct wifi_dev *)Elem->Priv;
	BSS_ENTRY *pBssEntry = NULL;
	ez_dev_t *ezdev = wdev->ez_driver_params.ezdev;
	if (ezdev)
	{
	} else {
		return;
	}
	/*if connection not already ongoing, then initiate a connection.*/
	pBssEntry = ez_find_roam_candidate(pAd, 
		ezdev->ez_security.ez_roam_info.ez_apcli_roam_bssid, 
		wdev);
	if (pBssEntry!= NULL){
		ez_roam(wdev, pBssEntry);
		return;
	}
	else {
		MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("bss entry for this bssid not found!!\n"));
		ez_reset_roam_bssid_hook((ez_dev_t *)wdev->ez_driver_params.ezdev);

		return;
	}
}


void EzPeriodicExecAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	if (IS_ADPTR_EZ_SETUP_ENABLED(pAd))
		ez_peer_table_maintenance(pAd);
}


void EzUpdateSsidPskAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	ez_update_ssid_psk_msg_t *ez_update_ssid_psk_msg = (ez_update_ssid_psk_msg_t *)Elem->Msg;
	struct wifi_dev *wdev = (struct wifi_dev *)Elem->Priv;
	set_ssid_psk_hook((ez_dev_t *)wdev->ez_driver_params.ezdev, 
		ez_update_ssid_psk_msg->ssid1, ez_update_ssid_psk_msg->pmk1, ez_update_ssid_psk_msg->psk1, 
		ez_update_ssid_psk_msg->ssid2, ez_update_ssid_psk_msg->pmk2, ez_update_ssid_psk_msg->psk2, 
		ez_update_ssid_psk_msg->ssid3, ez_update_ssid_psk_msg->pmk3, ez_update_ssid_psk_msg->psk3, 
		ez_update_ssid_psk_msg->EncrypType1, ez_update_ssid_psk_msg->EncrypType2, 
		ez_update_ssid_psk_msg->AuthMode1, ez_update_ssid_psk_msg->AuthMode2);
}

VOID EzStateMachineInit(
	IN RTMP_ADAPTER *pAd,
	IN STATE_MACHINE *Sm,
	OUT STATE_MACHINE_FUNC Trans[])
{
	StateMachineInit(Sm, (STATE_MACHINE_FUNC *)Trans, EZ_MAX_STATE, EZ_MAX_MSG, (STATE_MACHINE_FUNC)Drop, EZ_IDLE, EZ_MACHINE_BASE);
	StateMachineSetAction(Sm, EZ_IDLE, EZ_ROAM_REQ, (STATE_MACHINE_FUNC)EzRoamReqAction);
	StateMachineSetAction(Sm, EZ_IDLE, EZ_TRIBAND_RESTART_NON_EZ_REQ, (STATE_MACHINE_FUNC)EzTribandRestartNonEzReqAction);
	StateMachineSetAction(Sm, EZ_IDLE, EZ_PERIODIC_EXEC_REQ, (STATE_MACHINE_FUNC)EzPeriodicExecAction);
	StateMachineSetAction(Sm, EZ_IDLE, EZ_UPDATE_SSID_PSK_REQ, (STATE_MACHINE_FUNC)EzUpdateSsidPskAction);

}

VOID ez_ApCliCtrlJoinFailAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	PAPCLI_STRUCT pApCliEntry;
	USHORT ifIndex = (USHORT)(Elem->Priv);
	PULONG pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].CtrlCurrState;
	BSS_TABLE *aux_bss_table;
	APCLI_MLME_JOIN_REQ_STRUCT JoinReq;
	EZ_DEBUG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("(%s)\n", __FUNCTION__));

	{
		struct wifi_dev *ap_wdev =  &pAd->ApCfg.MBSSID[ifIndex].wdev;
		ez_restore_channel_config(ap_wdev);
		NdisZeroMemory(pAd->ApCfg.ApCliTab[ifIndex].CfgSsid,MAX_LEN_OF_SSID);
		pAd->ApCfg.ApCliTab[ifIndex].CfgSsidLen = 0;
			pAd->ApCfg.ApCliTab[ifIndex].CfgSsidLen = pAd->ApCfg.MBSSID[ifIndex].SsidLen;
			NdisCopyMemory(pAd->ApCfg.ApCliTab[ifIndex].CfgSsid,pAd->ApCfg.MBSSID[ifIndex].Ssid,pAd->ApCfg.MBSSID[ifIndex].SsidLen);
		}
	if (ifIndex >= MAX_APCLI_NUM)
		return;
	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
	if (ApScanRunning(pAd) == TRUE)
	{
			ez_update_connection_permission(pAd,&pApCliEntry->wdev,EZ_ALLOW_ALL);
		*pCurrState = APCLI_CTRL_DISCONNECTED;
		return;
	}

#ifdef EZ_API_SUPPORT
		if (pApCliEntry->wdev.ez_driver_params.ez_api_mode == CONNECTION_OFFLOAD)
		{
			ez_dev_t * ezdev = pApCliEntry->wdev.ez_driver_params.ezdev;
			if(ezdev){
						NdisZeroMemory(ezdev->ez_security.ez_apcli_force_bssid,MAC_ADDR_LEN);
					}
#ifdef SYSTEM_LOG_SUPPORT
				RTMPSendWirelessEvent(pAd, IW_WH_EZ_MY_APCLI_DISCONNECTED, NULL, pApCliEntry->wdev.wdev_idx, 0);
#else /* SYSTEM_LOG_SUPPORT */
				RtmpOSWrielessEventSend(pApCliEntry->wdev.if_dev, RT_WLAN_EVENT_CUSTOM, IW_WH_EZ_MY_APCLI_DISCONNECTED,
								NULL, NULL, 0);
#endif /* !SYSTEM_LOG_SUPPORT */	
		}
#endif	
	
	NdisZeroMemory(pApCliEntry->MlmeAux.Bssid, MAC_ADDR_LEN);
	NdisZeroMemory(pApCliEntry->MlmeAux.Ssid, MAX_LEN_OF_SSID);
	pApCliEntry->MlmeAux.SsidLen = 0;
	aux_bss_table = &pApCliEntry->MlmeAux.SsidBssTab;
	if((pApCliEntry->MlmeAux.attempted_candidate_index != EZ_INDEX_NOT_FOUND) &&
			(pApCliEntry->MlmeAux.attempted_candidate_index <= aux_bss_table->BssNr) ){
		aux_bss_table->BssEntry[pApCliEntry->MlmeAux.attempted_candidate_index].bConnectAttemptFailed = TRUE;
	}
#ifdef APCLI_AUTO_CONNECT_SUPPORT
	pApCliEntry->ProbeReqCnt = 0;
#endif
	pApCliEntry->AuthReqCnt = 0;
	pApCliEntry->AssocReqCnt = 0;
	*pCurrState = APCLI_CTRL_PROBE;
	NdisZeroMemory(&JoinReq, sizeof(APCLI_MLME_JOIN_REQ_STRUCT));

#ifdef WSC_AP_SUPPORT
    if ((pAd->ApCfg.ApCliTab[ifIndex].WscControl.WscConfMode != WSC_DISABLE) &&
		(pAd->ApCfg.ApCliTab[ifIndex].WscControl.bWscTrigger == TRUE))
    {
        NdisZeroMemory(JoinReq.Ssid, MAX_LEN_OF_SSID);
        JoinReq.SsidLen = pAd->ApCfg.ApCliTab[ifIndex].WscControl.WscSsid.SsidLength;
		NdisMoveMemory(JoinReq.Ssid, pAd->ApCfg.ApCliTab[ifIndex].WscControl.WscSsid.Ssid, JoinReq.SsidLen);
    }
    else
#endif /* WSC_AP_SUPPORT */
	{
				if (IS_EZ_SETUP_ENABLED(&pApCliEntry->wdev) 
					&& !pApCliEntry->wdev.ez_driver_params.ez_wps_reconnect)
				{
					if (!ez_update_connection_permission(pAd,&pApCliEntry->wdev,EZ_DISALLOW_ALL_ALLOW_ME))
					{
						*pCurrState = APCLI_CTRL_DISCONNECTED;
						return;
					}
					if(!ez_apcli_search_best_ap(pAd,pApCliEntry,ifIndex))
					{
						*pCurrState = APCLI_CTRL_DISCONNECTED;
						if(pApCliEntry->wdev.ez_driver_params.ezdev){
							if (!ez_update_connection_permission_hook(pApCliEntry->wdev.ez_driver_params.ezdev
								,EZ_ALLOW_ALL))
							{
								ASSERT(FALSE);
							}
						}
						return;
					}
				}

		if (pApCliEntry->CfgSsidLen != 0)
		{
			JoinReq.SsidLen = pApCliEntry->CfgSsidLen;
			NdisMoveMemory(&(JoinReq.Ssid), pApCliEntry->CfgSsid, JoinReq.SsidLen);
		}
	}

	if (!MAC_ADDR_EQUAL(pApCliEntry->CfgApCliBssid, ZERO_MAC_ADDR))
	{
		COPY_MAC_ADDR(JoinReq.Bssid, pApCliEntry->CfgApCliBssid);
	}

	EZ_DEBUG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("(%s) Probe Ssid=%s, Bssid=%02x:%02x:%02x:%02x:%02x:%02x\n",
		__FUNCTION__, JoinReq.Ssid, JoinReq.Bssid[0], JoinReq.Bssid[1], JoinReq.Bssid[2],
		JoinReq.Bssid[3], JoinReq.Bssid[4], JoinReq.Bssid[5]));
	MlmeEnqueue(pAd, APCLI_SYNC_STATE_MACHINE, APCLI_MT2_MLME_PROBE_REQ,
		sizeof(APCLI_MLME_JOIN_REQ_STRUCT), &JoinReq, ifIndex);
	return;
}

BOOLEAN ascii2int(RTMP_STRING *in, UINT32 *out)
{
        UINT32 decimal_val, val;
        CHAR *p, asc_val;
        decimal_val = 0;
        p = (char *)in;
        while((*p) != 0)
        {
                val = 0;
                asc_val = *p;
                if ((asc_val >= '0') && (asc_val <= '9'))
                        val = asc_val - 48;
                else
                        return FALSE;
                decimal_val = (decimal_val * 10) + val;
                p++;
        }
        *out = decimal_val;
        return TRUE;
}

VOID RTMPIoctlGetEzScanTable(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq)
{
	RTMP_STRING *msg;
	INT 		i=0;
	INT			WaitCnt;
	INT 		Status=0;
	INT         max_len = LINE_LEN;
	RTMP_STRING *this_char;
	UINT32		bss_start_idx;
	BSS_ENTRY *pBss;
	UINT32 TotalLen, BufLen = IW_SCAN_MAX_DATA;

	this_char = wrq->u.data.pointer;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
		("%s(): Before check, this_char = %s\n" ,
		__FUNCTION__, this_char));
	if (ascii2int(this_char, &bss_start_idx) == FALSE)
		bss_start_idx = 0;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
		("%s(): After check, this_char = %s, out = %d\n" ,
		__FUNCTION__, this_char, bss_start_idx));

	TotalLen = sizeof(CHAR)*((MAX_LEN_OF_BSS_TABLE)*max_len) + 100;

		BufLen = IW_SCAN_MAX_DATA;

	os_alloc_mem(NULL, (PUCHAR *)&msg, TotalLen);

	if (msg == NULL)
	{
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
			("%s(): msg memory alloc fail.\n", 
			__FUNCTION__));
		return;
	}

	memset(msg, 0 , TotalLen);
	if (pAdapter->ScanTab.BssNr ==0) {
		sprintf(msg,"No BssInfo\n");
		wrq->u.data.length = strlen(msg);
		Status = copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
			("%s(): wrq->u.data.length = %d\n", 
			__FUNCTION__, wrq->u.data.length));
		os_free_mem(pAdapter, (PUCHAR)msg);
		return;
	}
	if (bss_start_idx > (pAdapter->ScanTab.BssNr-1)) {
		sprintf(msg,"BssInfo Idx(%d) is out of range(0~%d)\n", 
				bss_start_idx, (pAdapter->ScanTab.BssNr-1));
		wrq->u.data.length = strlen(msg);
		Status = copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
			("%s(): wrq->u.data.length = %d\n", __FUNCTION__, wrq->u.data.length));
		os_free_mem(pAdapter, (PUCHAR)msg);
		return;
	}
	sprintf(msg,"%s","\n");
	sprintf(msg+strlen(msg),"%s","\n");
	sprintf(msg+strlen(msg),"%-12s%-4s%-33s%-20s%-23s%-9s%-7s%-7s%-3s\n",
	    " Capability", "Ch", "SSID", "BSSID", "Security", "Siganl(%)", "W-Mode", " ExtCH"," NT");

	WaitCnt = 0;

	while ((ScanRunning(pAdapter) == TRUE) && (WaitCnt++ < 200))
		OS_WAIT(500);

	for(i=bss_start_idx; i<pAdapter->ScanTab.BssNr ;i++)
	{
		pBss = &pAdapter->ScanTab.BssEntry[i];

		if (pBss->support_easy_setup) {			
			if( pBss->Channel==0)
				break;

			if((strlen(msg)+100 ) >= BufLen)
				break;
			
			/*
				EZ Capability
			*/
			sprintf(msg+strlen(msg)," 0x%-9x", pBss->easy_setup_capability);
			
			RTMPCommSiteSurveyData(msg, pBss, TotalLen);
		}

	}
	wrq->u.data.length = strlen(msg);
	Status = copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length);

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
		("%s(): wrq->u.data.length = %d\n", __FUNCTION__, wrq->u.data.length));
	os_free_mem(pAdapter, (PUCHAR)msg);
}


INT Set_EasySetup_Best_Ap_RSSI_Threshold(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	char *value;
	int i;
	EZ_ADAPTER *ez_ad = pAd->ez_ad;
	if (ez_ad == NULL)
		return FALSE;
	for (i=0, value = rstrtok(arg,";"); value; value = rstrtok(NULL,";"), i++)
	{
		if (i == 0)
		{
			ez_ad->best_ap_rssi_threshld_max = simple_strtol(value, 0, 10);
		} else {
			ez_ad->best_ap_rssi_threshld[i - 1] = simple_strtol(value, 0, 10);
		}
		EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,("%d;", ez_ad->best_ap_rssi_threshld[i]));
	}
	EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,("\n"));
	return TRUE;
}

BOOLEAN ez_ap_tx_grp_pkt_drop_to_ez_apcli(struct wifi_dev *wdev,struct sk_buff *pSkb)
{
	if (wdev->ez_driver_params.ezdev)
		return ez_ap_tx_grp_pkt_drop_to_ez_apcli_hook(wdev->ez_driver_params.ezdev,pSkb);

	return FALSE;
}

void ez_init(
	void *ad_obj,
	void *wdev_obj,
	unsigned char ap_mode)
{
	PRTMP_ADAPTER ad = ad_obj;
	EZ_ADAPTER *ez_ad;
	struct wifi_dev *wdev = wdev_obj;
/*
	if (!IS_EZ_SETUP_ENABLED(wdev)) {
		EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s - easy setup function is disabled.(wdev_type=0x%x)\n",
			__func__, wdev->wdev_type));
		return;
	}
*/
	EZ_DEBUG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("ez_init(), %s\n", wdev->if_dev->name));

	ez_ad = ez_init_hook(ad_obj, wdev_obj, ap_mode);
	ad->ez_ad = ez_ad;
	ad->SingleChip = &ez_ad->SingleChip;
	wdev->ez_driver_params.ez_started = FALSE;

#ifdef EZ_REGROUP_SUPPORT
	NdisAllocateSpinLock(ad, &wdev->ez_driver_params.regrp_mode_lock);
	wdev->ez_driver_params.regrp_mode = NON_REGRP_MODE;
	EZ_DEBUG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("\n[REGROUP] => Regroup Support = %d\n",wdev->ez_driver_params.en_regrp_supp));
#endif
}

void ez_init_ap_security_settings(struct wifi_dev *wdev)
{

		BSS_STRUCT *ap_mbss = wdev->func_dev;
		if (wdev->ez_driver_params.default_pmk_valid){
				
			ap_mbss->SsidLen = wdev->ez_driver_params.default_ssid_len;
			NdisCopyMemory(ap_mbss->Ssid,wdev->ez_driver_params.default_ssid,wdev->ez_driver_params.default_ssid_len);
			NdisCopyMemory(ap_mbss->PMK, wdev->ez_driver_params.default_pmk, LEN_PMK);
			//NdisCopyMemory(ap_mbss->PMK,wdev->SecConfig.PMK,LEN_PMK);
		} else {
			RT_CfgSetWPAPSKKey(wdev->sys_handle, ap_mbss->PSK, strlen(ap_mbss->PSK), 
				ap_mbss->Ssid, ap_mbss->SsidLen, ap_mbss->PMK);
			//NdisCopyMemory(wdev->SecConfig.PMK,ap_mbss->PMK,LEN_PMK);
		}
		
		SET_AUTHMODE_WPA2PSK(wdev->AuthMode);
		SET_ENCRYTYPE_AES(wdev->WepStatus);
		SET_ENCRYTYPE_AES(wdev->GroupKeyWepStatus);
		//SetWdevEncrypMode(&wdev->SecConfig, "AES");
		//SetWdevAuthMode(&wdev->SecConfig, "WPA2PSK");
		ap_security_init(wdev->sys_handle, wdev, wdev->func_idx);
		ap_key_tb_init(wdev->sys_handle);
}

void ez_init_apcli_settings(struct wifi_dev *wdev)
{
	BSS_STRUCT *ap_mbss;
	PRTMP_ADAPTER ad = wdev->sys_handle;
	APCLI_STRUCT *apcli_entry;
	ap_mbss = &ad->ApCfg.MBSSID[wdev->func_idx];
	apcli_entry = &ad->ApCfg.ApCliTab[wdev->func_idx];
	apcli_entry->MlmeAux.SsidBssTab.BssNr = 0;
	apcli_entry->MlmeAux.attempted_candidate_index = EZ_INDEX_NOT_FOUND;
	apcli_entry->avoid_loop = TRUE;
	apcli_entry->stop_auto_connect = FALSE;
	apcli_entry->CfgSsidLen = ap_mbss->SsidLen;
	NdisCopyMemory(apcli_entry->PMK, ap_mbss->PMK, LEN_PMK);	
	NdisCopyMemory(apcli_entry->CfgSsid, ap_mbss->Ssid, ap_mbss->SsidLen);	
}

void ez_init_shared_params(ez_init_params_t *init_params, struct wifi_dev *wdev)
{
	PRTMP_ADAPTER ad = wdev->sys_handle;
	EZ_ADAPTER *ez_ad = ad->ez_ad;
	BSS_STRUCT *ap_mbss;
	ap_mbss = &ad->ApCfg.MBSSID[wdev->func_idx];
	init_params->ad_obj = wdev->sys_handle;
	init_params->wdev_obj = wdev;
	init_params->ezdev_type = wdev->wdev_type;

	COPY_MAC_ADDR(init_params->mac_add, wdev->if_addr);
	if (wdev->wdev_type == WDEV_TYPE_AP){
		NdisCopyMemory(init_params->ssid,
			((BSS_STRUCT *)(wdev->func_dev))->Ssid,
			((BSS_STRUCT *)(wdev->func_dev))->SsidLen);
		init_params->ssid_len = ((BSS_STRUCT *)(wdev->func_dev))->SsidLen;
		NdisCopyMemory(init_params->pmk, ((BSS_STRUCT *)(wdev->func_dev))->PMK,PMK_LEN);
	} else {
		NdisCopyMemory(init_params->ssid, 
			((APCLI_STRUCT *)(wdev->func_dev))->CfgSsid, 
			((APCLI_STRUCT *)(wdev->func_dev))->CfgSsidLen);			
		init_params->ssid_len = ((APCLI_STRUCT *)(wdev->func_dev))->CfgSsidLen;
		NdisCopyMemory(init_params->pmk, ((APCLI_STRUCT *)(wdev->func_dev))->PMK,PMK_LEN);
		init_params->ezdev_type = WDEV_TYPE_APCLI;
	}

	//NdisCopyMemory(init_params->pmk,wdev->SecConfig.PMK,PMK_LEN);

	init_params->group_id_len = wdev->ez_driver_params.group_id_len;
	init_params->ez_group_id_len = wdev->ez_driver_params.ez_group_id_len;	//for localy maintain EzGroupID
	init_params->gen_group_id_len = wdev->ez_driver_params.gen_group_id_len;  //for localy maintain EzGenGroupID

	init_params->group_id = wdev->ez_driver_params.group_id;
	init_params->ez_group_id= wdev->ez_driver_params.ez_group_id;	//for localy maintain EzGroupID
	init_params->gen_group_id= wdev->ez_driver_params.gen_group_id;  //for localy maintain EzGenGroupID

	NdisCopyMemory(init_params->open_group_id,
		wdev->ez_driver_params.open_group_id,
		wdev->ez_driver_params.open_group_id_len);

	init_params->open_group_id_len = wdev->ez_driver_params.open_group_id_len;
	wdev->ez_driver_params.ez_api_mode = ez_ad->ez_api_mode;
	init_params->driver_ops_lut = &ez_driver_ops_7603;
	init_params->func_idx = wdev->func_idx;
	init_params->channel = &ad->CommonCfg.Channel;//&wdev->channel;

	init_params->ez_scan_timer			= &wdev->ez_driver_params.ez_scan_timer;
	//init_params->ez_stop_scan_timer		= &wdev->ez_driver_params.ez_stop_scan_timer;
	init_params->ez_scan_pause_timer		= &wdev->ez_driver_params.ez_scan_pause_timer;
	init_params->ez_group_merge_timer	= &wdev->ez_driver_params.ez_group_merge_timer;
	init_params->ez_connect_wait_timer = &wdev->ez_driver_params.ez_connect_wait_timer;
	init_params->ez_loop_chk_timer		= &wdev->ez_driver_params.ez_loop_chk_timer;
	init_params->os_hz = OS_HZ;

	init_params->channel_info.ht_bw = ad->CommonCfg.RegTransmitSetting.field.BW;
	init_params->channel_info.vht_bw = 0;
	init_params->channel_info.channel = ad->CommonCfg.Channel; //wdev->channel;
	init_params->channel_info.extcha = ad->CommonCfg.RegTransmitSetting.field.EXTCHA;
	init_params->default_group_data_band = wdev->ez_driver_params.default_group_data_band;	
#ifdef IF_UP_DOWN
	init_params->ez_intf_count_config_ap = wdev->ez_driver_params.ez_intf_count_config_ap;
	init_params->ez_intf_count_config_cli = wdev->ez_driver_params.ez_intf_count_config_cli;
	init_params->non_ez_intf_count_config_ap = 0;
	init_params->non_ez_intf_count_config_cli = 0;
#endif
}

VOID ez_scan_timeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	ez_driver_params_t *ez_driver_params;

	if (FunctionContext) {
		ez_driver_params = (ez_driver_params_t *)FunctionContext;
		if (ez_driver_params->ez_scan_timer.ez_timer_running) {

			ez_driver_params->ez_scan_timer.ez_timer_running = FALSE;
			if(ez_driver_params->ezdev){
				ez_scan_timeout_hook(ez_driver_params->ezdev);
			}
		}
		else {
			EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
				("%s - Timer expired but timer running flag is FALSE. Do nothing here.\n", 
				__FUNCTION__));
		}
	}
	else {
		EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
			("%s - FunctionContext is null?? CHECK!!\n", __FUNCTION__));
	}
}

VOID ez_scan_pause_timeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	ez_driver_params_t *ez_driver_params;
	ez_dev_t *ezdev;
	RTMP_ADAPTER *ad;
	struct wifi_dev *wdev;
#ifdef APCLI_SUPPORT
	APCLI_STRUCT *apcli_entry;
#endif

	if (FunctionContext) {
		ez_driver_params = (ez_driver_params_t *)FunctionContext;
		ezdev = ez_driver_params->ezdev;
		if(ezdev){

			wdev = ezdev->wdev;
			ad = wdev->sys_handle;
			
			//RTMP_SEM_LOCK(&ez_sec_info->ez_scan_pause_timer_lock);
			ez_acquire_lock(NULL, wdev,SCAN_PAUSE_TIMER_LOCK);
			if (ez_driver_params->ez_scan_pause_timer.ez_timer_running) {
				ez_driver_params->ez_scan_pause_timer.ez_timer_running = FALSE;

#ifdef APCLI_SUPPORT
				/* Timeout for peer apcli to conenct to my ap is over.
				     Allow scan on apcli interface now.
				*/
				apcli_entry = &ad->ApCfg.ApCliTab[wdev->func_idx];
				apcli_entry->stop_auto_connect = FALSE;

	            // Let internal periodic apcli join logic handle when to resume scan

#endif /* APCLI_SUPPORT */			

			}
			else {
				EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
					("%s - Timer expired but timer running flag is FALSE. Do nothing here.\n", 
					__FUNCTION__));
			}

			ez_release_lock(NULL, wdev,SCAN_PAUSE_TIMER_LOCK);
			
		}
	}
	else {
		EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
			("%s - FunctionContext is null?? CHECK!!\n", __FUNCTION__));
	}
}

VOID ez_group_merge_timeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	ez_driver_params_t *ez_driver_params;
	ez_driver_params = (ez_driver_params_t *)FunctionContext;
	ez_driver_params->ez_group_merge_timer.ez_timer_running = FALSE;

	if(ez_driver_params->ezdev)
		ez_group_merge_timeout_hook(ez_driver_params->ezdev);

}


void ez_wait_for_connection_allow_timeout(
		IN PVOID SystemSpecific1,
		IN PVOID FunctionContext,
		IN PVOID SystemSpecific2,
		IN PVOID SystemSpecific3)
{
	ez_driver_params_t *ez_driver_params;

	EZ_DEBUG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE,(" %s -->\n", __FUNCTION__));

	if (FunctionContext) {
		ez_driver_params = (ez_driver_params_t *)FunctionContext;
		if (ez_driver_params->ez_connect_wait_timer.ez_timer_running) {
			ez_driver_params->ez_connect_wait_timer.ez_timer_running = FALSE;
			if(ez_driver_params->ezdev)
				ez_update_connection_permission_hook(ez_driver_params->ezdev,EZ_ALLOW_ALL_TIMEOUT);
		}
		else {
			EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
				("%s - Timer expired but timer running flag is FALSE. Do nothing here.\n", 
				__FUNCTION__));
		}
	}
	else {
		EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
			("%s - FunctionContext is null?? CHECK!!\n", __FUNCTION__));
	}
}

/* Loop Check timeout handler*/
VOID ez_loop_chk_timeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	ez_driver_params_t * ez_driver_params = NULL;
	if(FunctionContext){
		ez_driver_params = (ez_driver_params_t *)FunctionContext;
		
	    if(ez_driver_params->ez_loop_chk_timer.ez_timer_running){

			//EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,("ez_loop_chk_timeout : wdev_idx=0x%x, wdev_type=0x%x, func_idx=0x%x :clear loop chk context\n",
			//	wdev->wdev_idx,wdev->wdev_type,wdev->func_idx));

			ez_driver_params->ez_loop_chk_timer.ez_timer_running = FALSE;

			if(ez_driver_params->ezdev)
				ez_loop_chk_timeout_hook(ez_driver_params->ezdev);
			}
			else{
				EZ_DEBUG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("ERROR !!! ez_loop_chk_timeout: not found mac entry\n"));
			}
	}
	else{
		EZ_DEBUG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("ez_loop_chk_timeout: FunctionContext NULL !!!\n"));
	}

}

void ez_start(
	void *wdev_obj,
	unsigned char ap_mode)
{
	struct wifi_dev *wdev;
	//struct _ez_security *ez_security;
	ez_init_params_t init_params;
	PRTMP_ADAPTER ad;
	EZ_ADAPTER *ez_ad;
	
	wdev = (struct wifi_dev *)wdev_obj;
	ad = wdev->sys_handle;
	EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("------> %s(%s)\n", __func__, wdev->if_dev->name));

/*	if (!IS_EZ_SETUP_ENABLED(wdev)) {
		EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
			("%s - easy setup function is disabled.(wdev_type=0x%x)\n", 
			__FUNCTION__, wdev->wdev_type));
		return;
	}
*/

	NdisAcquireSpinLock(&ad->Mlme.TaskLock);
	if (wdev->ez_driver_params.ez_started == TRUE) {
		NdisReleaseSpinLock(&ad->Mlme.TaskLock);
		DBGPRINT(RT_DEBUG_ERROR, ("easy-setup already started, skip ez_start.\n"));
		return;
	}
	wdev->ez_driver_params.ez_started = TRUE;

	if (wdev->ez_driver_params.group_id_len == 0) {
		wdev->ez_driver_params.ez_started = FALSE;
		NdisReleaseSpinLock(&ad->Mlme.TaskLock);

		/*Do NOT start easy function if there is no group id information.*/
		EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s - No group id information. return and don't start easy set up.\n",
			__func__));
		return;
	}
	NdisReleaseSpinLock(&ad->Mlme.TaskLock);

	if (wdev->wdev_type == WDEV_TYPE_AP)
		ez_init_ap_security_settings(wdev);

	if (wdev->wdev_type == WDEV_TYPE_STA)
		ez_init_apcli_settings(wdev);

	EZ_TIMER_INIT(ad, &wdev->ez_driver_params, 
		&wdev->ez_driver_params.ez_scan_timer.ez_timer, 
		wdev->ez_driver_params.ez_scan_timer.ez_timer_running, 
		ez_scan_timeout);
	EZ_TIMER_INIT(ad, &wdev->ez_driver_params, 
		&wdev->ez_driver_params.ez_scan_pause_timer.ez_timer, 
		wdev->ez_driver_params.ez_scan_pause_timer.ez_timer_running, 
		ez_scan_pause_timeout);
#ifdef EZ_NETWORK_MERGE_SUPPORT
	EZ_TIMER_INIT(ad, &wdev->ez_driver_params, 
			&wdev->ez_driver_params.ez_group_merge_timer.ez_timer, 
			wdev->ez_driver_params.ez_group_merge_timer.ez_timer_running, 
			ez_group_merge_timeout);
#endif
#ifdef NEW_CONNECTION_ALGO
	EZ_TIMER_INIT(ad,&wdev->ez_driver_params,
	&wdev->ez_driver_params.ez_connect_wait_timer.ez_timer,
	wdev->ez_driver_params.ez_connect_wait_timer.ez_timer_running,
	ez_wait_for_connection_allow_timeout);
#endif
#ifdef EZ_DUAL_BAND_SUPPORT
	EZ_TIMER_INIT(ad, &wdev->ez_driver_params, 
		&wdev->ez_driver_params.ez_loop_chk_timer.ez_timer, 
		wdev->ez_driver_params.ez_loop_chk_timer.ez_timer_running, 
		ez_loop_chk_timeout);
#endif

#ifdef EZ_REGROUP_SUPPORT
	wdev->ez_driver_params.regrp_mode = NON_REGRP_MODE;
	EZ_DEBUG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
		("\n[REGROUP] => Regroup Support = %d\n",wdev->ez_driver_params.en_regrp_supp));
	wdev->ez_driver_params.ap_entry_count = 0;
	NdisZeroMemory(&wdev->ez_driver_params.ap_list[0], MAX_AP_CANDIDATES * sizeof(struct _drvr_cand_list));
#endif

	ez_init_shared_params(&init_params,wdev);
	wdev->ez_driver_params.ezdev = ez_start_hook(&init_params);
	
	wdev->ez_driver_params.ez_ad = ez_get_adapter_hook();
	
	printk("sizeof EZ_ADAPTER = %d\n", sizeof(EZ_ADAPTER));
	printk("sizeof ez_dev_t = %d\n", sizeof(ez_dev_t));


	ez_ad = wdev->ez_driver_params.ez_ad;
	if (ez_ad->sanity_check1 == &ez_ad->sanity_check1)
	{
		printk("content of sanity1 ==== %p\n", ez_ad->sanity_check1);
		printk("Address of sanity1 ==== %p\n", &ez_ad->sanity_check1);
		printk("1 passed\n");
	} else {
		printk("content of sanity1 ==== %p\n", ez_ad->sanity_check1);
		printk("Address of sanity1 ==== %p\n", &ez_ad->sanity_check1);
	}

	if (ez_ad->sanity_check == &ez_ad->sanity_check)
	{
		printk("content of sanity ==== %p\n", ez_ad->sanity_check);
		printk("Address of sanity ==== %p\n", &ez_ad->sanity_check);
		
		printk("DRIVER MOD structures are in sync\n");
	} else {
		printk("content of sanity ==== %p\n", ez_ad->sanity_check);
		printk("Address of sanity ==== %p\n", &ez_ad->sanity_check);
		
		printk("DRIVER MOD structures do not match\n");		

	}
#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)
	if ((wf_fwd_set_easy_setup_mode != NULL) && (wdev->ez_driver_params.ez_api_mode != CONNECTION_OFFLOAD)){
		EZ_DEBUG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("SET EASY SETUP MODE IN FWD MODULE\n"));
		wf_fwd_set_easy_setup_mode(TRUE); // Arvind Need to handle
	}
#endif

	if (ap_mode) {

		UpdateBeaconHandler(ad, wdev, IE_CHANGE);
	}

	EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("<------ %s()\n", __FUNCTION__));
}



void ez_stop(
	void *wdev_obj)
{
	struct wifi_dev *wdev;
	PRTMP_ADAPTER pAd;	
	BOOLEAN is_triband = FALSE;
	ez_timer_t *ptimer;

	wdev = (struct wifi_dev *)wdev_obj;
	pAd = wdev->sys_handle;
	is_triband = ez_is_triband();

	EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("-->%s(%s): wdev_type=0x%x\n", __func__, wdev->if_dev->name, wdev->wdev_type));

	while(1)
	{
		if (IS_DUAL_CHIP_DBDC(pAd) && !is_triband)
		{
			ez_acquire_lock(pAd, NULL, MLME_SYNC_LOCK);
		}
		NdisAcquireSpinLock(&pAd->Mlme.TaskLock);

		if (wdev->ez_driver_params.ez_started == FALSE) {
			DBGPRINT(RT_DEBUG_ERROR, ("easy-setup already stopped, skip ez_stop.\n"));
			NdisReleaseSpinLock(&pAd->Mlme.TaskLock);
			if (IS_DUAL_CHIP_DBDC(pAd) && !is_triband)
				ez_release_lock(pAd, NULL, MLME_SYNC_LOCK);
			return;
		}

		if (pAd->Mlme.bRunning == FALSE) {
			if (IS_DUAL_CHIP_DBDC(pAd) && !is_triband) {
				if (ez_is_other_band_mlme_running(&pAd->ApCfg.MBSSID[0].wdev)) {
					NdisReleaseSpinLock(&pAd->Mlme.TaskLock);
					ez_release_lock(pAd, NULL, MLME_SYNC_LOCK);
					RtmpusecDelay(5000);
					continue;
				}
			}

			wdev->ez_driver_params.ez_started = FALSE;
			if (wdev->ez_driver_params.ezdev) {
				ez_stop_hook(wdev->ez_driver_params.ezdev);
				wdev->ez_driver_params.ezdev = NULL;
			}

#if defined (CONFIG_WIFI_PKT_FWD)
			/*if (IS_EZ_SETUP_ENABLED(wdev) && (wf_fwd_set_easy_setup_mode != NULL)){*/
			if (wf_fwd_set_easy_setup_mode != NULL) {
				EZ_DEBUG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					("CLEAR EASY SETUP MODE IN FWD MODULE\n"));
				wf_fwd_set_easy_setup_mode(FALSE); /* Arvind Need to handle*/
			}
#endif	
#ifdef EZ_REGROUP_SUPPORT
			wdev->ez_driver_params.ap_entry_count = 0;
			NdisZeroMemory(&wdev->ez_driver_params.ap_list[0],
				MAX_AP_CANDIDATES * sizeof(struct _drvr_cand_list));
			wdev->ez_driver_params.regrp_mode = NON_REGRP_MODE;
			EZ_DEBUG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\n[REGROUP] => Regroup Support = %d\n", wdev->ez_driver_params.en_regrp_supp));
#endif

			ptimer = &wdev->ez_driver_params.ez_scan_timer;
			if (ptimer->ez_timer_running == TRUE)
				EZ_CANCEL_TIMER(&ptimer->ez_timer, ptimer->ez_timer_running);
			EZ_RELEASE_TIMER(&ptimer->ez_timer, ptimer->ez_timer_running);

			ptimer = &wdev->ez_driver_params.ez_scan_pause_timer;
			if (ptimer->ez_timer_running == TRUE)
				EZ_CANCEL_TIMER(&ptimer->ez_timer, ptimer->ez_timer_running);
			EZ_RELEASE_TIMER(&ptimer->ez_timer, ptimer->ez_timer_running);

#ifdef EZ_NETWORK_MERGE_SUPPORT	
			ptimer = &wdev->ez_driver_params.ez_group_merge_timer;
			if (ptimer->ez_timer_running == TRUE)
				EZ_CANCEL_TIMER(&ptimer->ez_timer, ptimer->ez_timer_running);
			EZ_RELEASE_TIMER(&ptimer->ez_timer, ptimer->ez_timer_running);
#endif

#ifdef NEW_CONNECTION_ALGO
			ptimer = &wdev->ez_driver_params.ez_connect_wait_timer;
			if (ptimer->ez_timer_running == TRUE)
				EZ_CANCEL_TIMER(&ptimer->ez_timer, ptimer->ez_timer_running);
			EZ_RELEASE_TIMER(&ptimer->ez_timer, ptimer->ez_timer_running);
#endif

#ifdef EZ_DUAL_BAND_SUPPORT
			ptimer = &wdev->ez_driver_params.ez_loop_chk_timer;
			if (ptimer->ez_timer_running == TRUE)
				EZ_CANCEL_TIMER(&ptimer->ez_timer, ptimer->ez_timer_running);
			EZ_RELEASE_TIMER(&ptimer->ez_timer, ptimer->ez_timer_running);
#endif

			NdisReleaseSpinLock(&pAd->Mlme.TaskLock);
			if (IS_DUAL_CHIP_DBDC(pAd) && !is_triband)
				ez_release_lock(pAd, NULL, MLME_SYNC_LOCK);
			break;
		}
		NdisReleaseSpinLock(&pAd->Mlme.TaskLock);
		if (IS_DUAL_CHIP_DBDC(pAd) && !is_triband)
			ez_release_lock(pAd, NULL, MLME_SYNC_LOCK);
		RtmpusecDelay(5000);
	}

	EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("<------ %s()\n", __FUNCTION__));
}

INT Set_ez_connection_allow_all(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING *arg)
{
	POS_COOKIE  pObj;
    UCHAR	    apidx;
	struct wifi_dev *wdev;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	apidx = pObj->ioctl_if;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	if (IS_EZ_SETUP_ENABLED(wdev) && pAd->ez_ad) {
		ez_connection_allow_all_hook(pAd->ez_ad, wdev->ez_driver_params.default_pmk_valid);
		return TRUE;
	}
	printk("Ez setup not activated on this interface\n");
	return FALSE;
}


void ez_restore_channel_config(struct wifi_dev *wdev)
{
	PRTMP_ADAPTER ad;
	struct wifi_dev *apcli_wdev;
	ez_dev_t *ezdev = wdev->ez_driver_params.ezdev;
	EZ_ADAPTER *ez_ad=NULL;
	if(ezdev)
		ez_ad  = ezdev->ez_ad;

	ad = wdev->sys_handle;
	apcli_wdev = &ad->ApCfg.ApCliTab[wdev->func_idx].wdev;
	EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,("%s\n", __FUNCTION__));

	// Rakesh: Disabling this as doing it here is not right in various scenarios.
	// Optimization can be done in future for caller to choose/inform whether to skip.
	if(ez_ad){
		if( 
#ifdef EZ_PUSH_BW_SUPPORT
			( !ez_ad->push_bw_config ) &&
#endif
		    (ez_is_channel_same(wdev) == TRUE) )
		{
			EZ_DEBUG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("ez_restore_channel_config: Channel already same\n")); 
			return;
		}

#ifdef EZ_PUSH_BW_SUPPORT
		if( ez_ad->push_bw_config )
		{
			if(ezdev->ez_security.this_band_info.shared_info.channel_info.ht_bw != 0xFF)
				ez_driver_ops_wlan_config_set_ht_bw_mt7603(ezdev, ezdev->ez_security.this_band_info.shared_info.channel_info.ht_bw);
#ifdef DOT11_VHT_AC
			if(ezdev->ez_security.this_band_info.shared_info.channel_info.vht_bw != 0xFF)
				wlan_config_set_vht_bw(wdev, ezdev->ez_security.this_band_info.shared_info.channel_info.vht_bw);
#endif		
		}
#endif

		ez_driver_ops_wlan_config_set_ext_cha_mt7603(ezdev, ezdev->ez_security.this_band_info.shared_info.channel_info.extcha);

#ifdef EZ_PUSH_BW_SUPPORT
		if( ez_ad->push_bw_config )
		{
			if(ezdev->ez_security.this_band_info.shared_info.channel_info.ht_bw != 0xFF)
				ez_driver_ops_wlan_config_set_ht_bw_mt7603(ezdev, ezdev->ez_security.this_band_info.shared_info.channel_info.ht_bw);
#ifdef DOT11_VHT_AC
			if(ezdev->ez_security.this_band_info.shared_info.channel_info.vht_bw != 0xFF)
				wlan_config_set_vht_bw(apcli_wdev, ezdev->ez_security.this_band_info.shared_info.channel_info.vht_bw); 
#endif		
		}
#endif

		ez_driver_ops_wlan_config_set_ext_cha_mt7603(ezdev, ezdev->ez_security.this_band_info.shared_info.channel_info.extcha);

#if (defined(DOT11_N_SUPPORT) && defined(DOT11N_DRAFT3))
		if(ezdev->ez_security.ap_did_fallback){
			if(ezdev->ez_security.fallback_channel == ezdev->ez_security.this_band_info.shared_info.channel_info.channel){
				EZ_DEBUG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("\nez_restore_channel_config: Restoring ap to fallback mode\n"));
				ez_driver_ops_wlan_operate_set_ht_bw_mt7603(wdev->ez_driver_params.ezdev, HT_BW_20);
				ez_driver_ops_wlan_operate_set_ext_cha_mt7603(wdev->ez_driver_params.ezdev, EXTCHA_NONE);
			}
		}
#endif

		wdev->ez_driver_params.do_not_restart_interfaces = 1;
		rtmp_set_channel(wdev->sys_handle, wdev, ezdev->ez_security.this_band_info.shared_info.channel_info.channel);
		wdev->ez_driver_params.do_not_restart_interfaces = 0;
	}
}


BOOLEAN ez_ap_scan_complete_handle(struct wifi_dev *wdev)
{
	ez_dev_t *ezdev = NULL;

	if (wdev)
		ezdev = wdev->ez_driver_params.ezdev;

	if(ezdev)
		return (wdev->ez_driver_params.scan_one_channel 
			|| ezdev->ez_security.internal_force_connect_bssid);
	else
		return FALSE;
}

BOOLEAN ez_need_bypass_rx_fwd(struct wifi_dev *wdev)
{
	ez_dev_t *ezdev = wdev->ez_driver_params.ezdev;
	if(ezdev)
		return ez_need_bypass_rx_fwd_hook(ezdev);
	else
		return FALSE;

}

//extern void ez_exit_hook(
//	ez_dev_t  *ezdev);

void ez_exit(
	void *wdev_obj)
{
	struct wifi_dev *wdev = wdev_obj;
	ez_exit_hook((RTMP_ADAPTER *)wdev->sys_handle);
#ifdef EZ_REGROUP_SUPPORT
	NdisFreeSpinLock(&wdev->ez_driver_params.regrp_mode_lock);
#endif
}

void ez_driver_ops_fill_out_table_mt7603(BSS_TABLE* Tab, EZ_BSS_TABLE *pEzBss);

BOOLEAN ez_apcli_search_best_ap_hook(
	void *ez_ad_obj,
	void *ezdev_obj,
	void *out_tab);

BOOLEAN ez_apcli_search_best_ap(
	void *ad_obj,
	void *apcli_entry_obj,
	unsigned char inf_idx)
{
	BOOLEAN ret;
	PRTMP_ADAPTER pAd = ad_obj;
	APCLI_STRUCT *apcli_entry = apcli_entry_obj;
	EZ_BSS_TABLE *out_tab = NULL;
	ez_dev_t *ezdev;
//	BSS_STRUCT *ap_mbss = &pAd->ApCfg.MBSSID[apcli_entry->wdev.func_idx];
	os_alloc_mem(pAd, (PUCHAR *)&out_tab, sizeof(EZ_BSS_TABLE));
	if(out_tab == NULL)
		return FALSE;

	ezdev = apcli_entry->wdev.ez_driver_params.ezdev;
	if(ezdev){
		if (apcli_entry->stop_auto_connect) {
			EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
						("%s(line.%d) - Provider search Paused.\n", 
						__FUNCTION__, __LINE__));
			os_free_mem(ad_obj,out_tab);
			return FALSE;
		}
#ifdef EZ_REGROUP_SUPPORT
		if(apcli_entry->wdev.ez_driver_params.regrp_mode == REGRP_MODE_BLOCKED){
			EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
						("%s(line.%d) - Provider search Blocked for Regroup on this wdev_idx : %d.\n", 
						__FUNCTION__, __LINE__, apcli_entry->wdev.wdev_idx));
			os_free_mem(ad_obj,out_tab);
			return FALSE;
		}
#endif
#ifdef IF_UP_DOWN
		if (pAd->ez_ad && !ez_all_intf_up(pAd->ez_ad)) {
			EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s(line.%d) - Provider search Blocked for All interfaces not up. wdev_idx : %d.\n",
				__func__, __LINE__, apcli_entry->wdev.wdev_idx));
			os_free_mem(ad_obj, out_tab);
			return FALSE;
		}
#endif
		ez_driver_ops_fill_out_table_mt7603(&apcli_entry->MlmeAux.SsidBssTab,out_tab);

		NdisZeroMemory(ezdev->CfgSsid, sizeof(ezdev->CfgSsid));
		NdisZeroMemory(apcli_entry->CfgSsid, sizeof(apcli_entry->CfgSsid));
		NdisCopyMemory(apcli_entry->CfgSsid, ezdev->ez_security.this_band_info.shared_info.ssid, ezdev->ez_security.this_band_info.shared_info.ssid_len);	
		apcli_entry->CfgSsidLen = ezdev->ez_security.this_band_info.shared_info.ssid_len;
		NdisCopyMemory(ezdev->CfgSsid, apcli_entry->CfgSsid,apcli_entry->CfgSsidLen);
		
		ezdev->CfgSsidLen = ezdev->ez_security.this_band_info.shared_info.ssid_len;
		printk("%s, ezdev->CfgSsid--->%s\n", __FUNCTION__, ezdev->CfgSsid);
		ret =  ez_apcli_search_best_ap_hook(pAd->ez_ad,apcli_entry->wdev.ez_driver_params.ezdev,out_tab);
		os_free_mem(ad_obj,out_tab);
		return ret;
	}
	else
		return FALSE;

}

void ez_send_unicast_deauth(void *ad_obj, UCHAR *peer_addr)
{
	MAC_TABLE_ENTRY *pEntry = NULL;
	struct wifi_dev *wdev;
	pEntry = MacTableLookup(ad_obj,peer_addr);
		
	wdev = pEntry->wdev;
	if(wdev->ez_driver_params.ezdev)
		ez_driver_ops_send_unicast_deauth_mt7603(wdev->ez_driver_params.ezdev,peer_addr);
}

void ez_allocate_or_update_non_ez_band(void *wdev_obj)
{
	struct wifi_dev *wdev = wdev_obj;

	ez_allocate_or_update_non_ez_band_hook(wdev, wdev->sys_handle, wdev->wdev_type, wdev->func_idx,NULL, &wdev->channel);
	
}
