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

//#ifndef __EZ_chip_ops_api_H__
//#define __EZ_chip_ops_api_H__

#ifdef WH_EZ_SETUP

#include "shared_structs.h"

unsigned char ez_driver_ops_RandomByte_mt7603(void *ezdev);

void ez_driver_ops_GenRandom_mt7603(
	void *ezdev,
	UCHAR *macAddr, 
	UCHAR *random);

void ez_driver_ops_DH_PublicKey_Generate_mt7603(
	void * ezdev,	
	UINT8 GValue[], 
	UINT GValueLength,
	UINT8 PValue[],
	UINT PValueLength,
	UINT8 PrivateKey[],
	UINT PrivateKeyLength,
	UINT8 PublicKey[],
	UINT *PublicKeyLength);

void ez_driver_ops_RT_DH_SecretKey_Generate_mt7603(
	void * ezdev,	
	UINT8 PublicKey[], 
	UINT PublicKeyLength, 
	UINT8 PValue[], 
	UINT PValueLength, 
	UINT8 PrivateKey[],
	UINT PrivateKeyLength, 
	UINT8 SecretKey[], 
	UINT *SecretKeyLength);

void ez_driver_ops_RT_SHA256_mt7603(
	void * ezdev,	
	IN  const UINT8 Message[], 
    IN  UINT MessageLen, 
    OUT UINT8 DigestMessage[]);

VOID ez_driver_ops_WpaDerivePTK_mt7603(
	void *ezdev,	
	UCHAR *PMK, 
	UCHAR *ANonce, 
	UCHAR *AA, 
	UCHAR *SNonce, 
	UCHAR *SA, 
	UCHAR *output, 
	UINT len);

INT ez_driver_ops_AES_Key_Unwrap_mt7603(
	void * ezdev,	
	UINT8 CipherText[],
	UINT CipherTextLength, 
	UINT8 Key[],
	UINT KeyLength,
	UINT8 PlainText[],
	UINT *PlainTextLength);

void ez_driver_ops_install_pairwise_key_mt7603(
	void *ezdev, 
	char *peer_mac,
	unsigned char *pmk, 	
	unsigned char *ptk, 
	unsigned char authenticator);

#ifdef APCLI_SUPPORT
void ez_driver_ops_apcli_install_group_key_mt7603(
	void * ezdev,	
	char *peer_mac,	
	char *peer_gtk,
	unsigned char gtk_len);
#endif

int ez_driver_ops_wlan_config_get_ht_bw_mt7603(void *ezdev);

int ez_driver_ops_wlan_config_get_vht_bw_mt7603(void *ezdev);

int ez_driver_ops_wlan_operate_get_ht_bw_mt7603(void *ezdev);

int ez_driver_ops_wlan_operate_get_vht_bw_mt7603(void *ezdev);

int ez_driver_ops_wlan_config_get_ext_cha_mt7603(void *ezdev);

int ez_driver_ops_wlan_operate_get_ext_cha_mt7603(void *ezdev);

int ez_driver_ops_get_cli_aid_mt7603(
	void *ezdev,
	char * peer_mac);

void ez_driver_ops_ez_cancel_timer_mt7603(
	void * ezdev,
	void * timer_struct);

void ez_driver_ops_ez_set_timer_mt7603(
	void * ezdev,
	void * timer_struct, 
	unsigned long time);

BOOLEAN ez_driver_ops_is_timer_running_mt7603(
	void *ezdev,
	void * timer_struct);

int ez_driver_ops_get_apcli_enable_mt7603(void *ezdev);

int ez_driver_ops_ApScanRunning_mt7603(void *ezdev);

void ez_driver_ops_send_unicast_deauth_mt7603(
	void *ezdev,
	char *peer_mac);

void ez_driver_ops_UpdateBeaconHandler_mt7603(
	void *ezdev, 
	int reason);

void ez_driver_ops_update_security_setting_mt7603(
	void *ezdev,
	unsigned char *pmk);

void ez_driver_ops_update_ap_wsc_profile_mt7603(void *ezdev);

void ez_driver_ops_MiniportMMRequest_mt7603(
	void *ezdev,
	char *out_buf,
	int frame_len,
	BOOLEAN need_tx_status);

void ez_driver_ops_NdisGetSystemUpTime_mt7603(
	void *ezdev,
	ULONG *time);


INT ez_driver_ops_AES_Key_Wrap_mt7603(
	void *ezdev,
	UINT8 PlainText[],
	UINT  PlainTextLength,
	UINT8 Key[],
	UINT  KeyLength,
	UINT8 CipherText[],
	UINT *CipherTextLength);

INT ez_driver_ops_RtmpOSWrielessEventSendExt_mt7603(
	void *ezdev,
	UINT32 eventType,
	INT flags,
	PUCHAR pSrcMac,
	PUCHAR pData,
	UINT32 dataLen);

void ez_driver_ops_send_broadcast_deauth_mt7603(void *ezdev);

void ez_driver_ops_apcli_stop_auto_connect_mt7603(
	void *ezdev,
	BOOLEAN enable);

void ez_driver_ops_timer_init_mt7603(
	void *ezdev,
	void* timer, 
	void *callback);

void ez_driver_ops_set_ap_ssid_null_mt7603(void *ezdev);
void* ez_driver_ops_get_pentry_mt7603(
	void *ezdev,
	UCHAR *mac_addr);

void ez_driver_ops_mark_entry_duplicate_mt7603(
	void *ezdev,
	UCHAR *mac_addr);

void ez_driver_ops_restore_cli_config_mt7603(void *ezdev);

void ez_driver_ops_ScanTableInit_mt7603(void *ezdev);

void ez_driver_ops_RT_HMAC_SHA1_mt7615(
	void * ezdev, 
	UINT8 Key[], 
	UINT KeyLen, 
	UINT8 Message[], 
	UINT MessageLen, 
	UINT8 MAC[], 
	UINT MACLen);

void ez_driver_ops_wlan_config_set_ht_bw_mt7603(void *ezdev, UINT8 ht_bw);

void ez_driver_ops_wlan_config_set_ext_cha_mt7603(void *ezdev, UINT8 ext_cha);

//
//		raghav
//
void ez_driver_ops_wlan_operate_set_ht_bw_mt7603(
	void *ezdev,
	UINT8 ht_bw);

void ez_driver_ops_wlan_operate_set_ext_cha_mt7603(
	void *ezdev,
	UINT8 ext_cha);

void ez_driver_ops_rtmp_set_channel_mt7603(
	void *ad_obj, 
	void * wdev_obj, 
	UCHAR Channel);

void ez_driver_ops_APScanCnclAction_mt7603(void *ezdev);

void ez_driver_ops_send_loop_detect_pkt_mt7603(
	void *ezdev, 
	PUCHAR pOtherCliMac);

void ez_driver_ops_update_ap_peer_record_mt7603(
	void *ezdev, 
	BOOLEAN band_switched, 
	PUCHAR peer_addr);

void ez_driver_ops_update_cli_peer_record_mt7603(
	void *ezdev, 
	BOOLEAN band_switched, 
	PUCHAR peer_addr);

void ez_driver_ops_MgtMacHeaderInit_mt7603(
	void *ezdev, 
	HEADER_802_11_EZ *pHdr80211,
	UCHAR SubType,
	UCHAR ToDs,
	UCHAR *pDA,
	UCHAR *pSA,
	UCHAR *pBssid);

// prakhar :::

BOOLEAN ez_driver_ops_update_ap_mt7603(
	void *ezdev, 
	void *updated_configs);

BOOLEAN ez_driver_ops_update_cli_mt7603(
	void *ezdev, 
	void *updated_configs);

void ez_driver_ops_ApSiteSurvey_mt7603(
	void *ezdev,
	int scan_one_channel);

void ez_driver_ops_wlan_config_set_vht_bw_mt7603(void *ezdev, UINT8 vht_bw);

INT ez_driver_ops_SetCommonHtVht_mt7603(void *ezdev);

#endif
//#endif
