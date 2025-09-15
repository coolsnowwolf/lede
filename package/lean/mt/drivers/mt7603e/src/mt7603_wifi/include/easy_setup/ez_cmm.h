#ifndef __EZ_CMM_H__
#define __EZ_CMM_H__


#define EZ_PMK_LEN                    32

#define EZ_MAX_STA_NUM 8

#define EZ_AES_KEY_ENCRYPTION_EXTEND	8 /* 8B for AES encryption extend size */

#define EZ_RAW_KEY_LEN                192

#define EZ_NONCE_LEN                  32

#define EZ_DH_KEY_LEN                 32

#define MAX_EZ_BANDS 						2
#define MAX_NON_EZ_BANDS 						2

#define MAX_EZ_PEERS_PER_BAND		8
#define EZDEV_NUM_MAX							6

#define EZ_PTK_LEN                    80



typedef struct ez_chipops_s{
	unsigned char (*RandomByte)(void * ezdev);
	void (*GenRandom)(void * ezdev, UCHAR *macAddr, UCHAR *random);
	void (*DH_PublicKey_Generate) (void * ezdev, UINT8 *GValue, UINT GValueLength,UINT8 *PValue,UINT PValueLength,
		UINT8 *PrivateKey,UINT PrivateKeyLength,UINT8 *PublicKey,UINT *PublicKeyLength);
	void (*RT_DH_SecretKey_Generate) (void * ezdev, UINT8 PublicKey[], UINT PublicKeyLength, UINT8 PValue[], UINT PValueLength, 
			UINT8 PrivateKey[],UINT PrivateKeyLength, UINT8 SecretKey[], UINT *SecretKeyLength);
	void (*RT_SHA256)(void *ezdev, const UINT8 Message[], UINT MessageLen, UINT8 DigestMessage[]);
	VOID (*WpaDerivePTK)(void * ezdev, UCHAR *PMK, UCHAR *ANonce, UCHAR *AA, UCHAR *SNonce, UCHAR *SA, UCHAR *output, UINT len);
	INT (*AES_Key_Unwrap)(void * ezdev, UINT8 CipherText[],UINT CipherTextLength, UINT8 Key[],UINT KeyLength,UINT8 PlainText[],UINT *PlainTextLength);
	void (*ez_install_pairwise_key)(void * ezdev, char *peer_mac, unsigned char *pmk, unsigned char *ptk, unsigned char authenticator);
	void (*ez_apcli_install_group_key)(void * ezdev, char *peer_mac, char *peer_gtk, unsigned char ptk_len);
	int (*wlan_config_get_ht_bw)(void 
*ezdev);
	int (*wlan_config_get_vht_bw)(void 
*ezdev);
	int (*wlan_operate_get_ht_bw)(void 
*ezdev);
	int (*wlan_operate_get_vht_bw)(void 
*ezdev);
	int (*wlan_config_get_ext_cha)(void 
*ezdev);
	int (*wlan_operate_get_ext_cha)(void 
*ezdev);
	int (*get_cli_aid)(void * ezdev, char * peer_mac);
	void (*ez_cancel_timer)(void * ezdev, void * timer_struct);
	void (* ez_set_timer)(void * ezdev, void * timer_struct, unsigned long time);
	int (* ez_is_timer_running)(void * ezdev, void * timer_struct);


	int (* get_apcli_enable)(void *ezdev);
	int (* ApScanRunning)(void *ezdev);

	void (*ez_send_unicast_deauth)(void *ezdev, char *peer_mac);
	void (*ez_restore_channel_config)(void *ezdev);
	void (*UpdateBeaconHandler)(void *ezdev, int reason);
	void (*ez_initiate_new_scan)(void *ezdev);
	void (*ez_update_security_setting)(void *ezdev, unsigned char *pmk);
	void (*ez_update_ap_wsc_profile)(void *ezdev);
	void (*APScanCnclAction)(void *ezdev);
	void (*ez_send_loop_detect_pkt)(void *ezdev, unsigned char *pOtherCliMac);
	

	INT (*ez_update_ap)(void *ezdev, void *updated_configs, int gid_different, int band_switched);
	INT (*ez_update_cli)(void *ezdev, void *updated_configs, int gid_different, int band_switched);
	void (*ez_update_ap_peer_record)(void *ezdev, BOOLEAN band_switched, unsigned char *peer_mac);
	void (*ez_update_cli_peer_record)(void *ezdev, BOOLEAN band_switched, unsigned char *peer_mac);
	void (*	MiniportMMRequest)(void *ezdev, char *out_buf,int frame_len);
	void (*NdisGetSystemUpTime)(void * ezdev, ULONG *time);
	INT (*AES_Key_Wrap )(void * ezdev, UINT8 PlainText[],UINT  PlainTextLength,UINT8 Key[],UINT  KeyLength,UINT8 CipherText[],UINT *CipherTextLength);
	INT (*RtmpOSWrielessEventSendExt)(void* ezdev,UINT32 eventType,INT flags,PUCHAR pSrcMac,PUCHAR pData,UINT32 dataLen);
	void (*ez_send_broadcast_deauth)(void *ezdev);
	void (*MgtMacHeaderInit)(void *ezdev, HEADER_802_11_EZ *pHdr80211,UCHAR SubType,UCHAR ToDs,UCHAR *pDA,UCHAR *pSA,UCHAR *pBssid);
	void (*apcli_stop_auto_connect)(void *ezdev, BOOLEAN enable);
	void (*timer_init)(void *ezdev, void* timer, void *callback);
	void (*set_ap_ssid_null)(void *ezdev);
	void (*ez_set_entry_apcli)(void *ezdev, UCHAR *mac_addr, BOOLEAN is_apcli);
	void *(*ez_get_pentry)(void *ezdev, UCHAR *mac_addr);
	void (*ez_mark_entry_duplicate)(void *ezdev, UCHAR *mac_addr);
	void (*ez_restore_cli_config)(void *ezdev);
	void (*ScanTableInit)(void *ezdev);
	void (*RT_HMAC_SHA1)(void * ezdev, UINT8 Key[], UINT KeyLen, UINT8 Message[], UINT MessageLen, UINT8 MAC[], UINT MACLen);
	BOOLEAN (*is_mlme_running)(void *ezdev);
	void (*ApSiteSurvey)(void *ezdev, int scan_one_channel);
	void (*ez_ApSiteSurvey_by_wdev)(void * ad_obj,void * pSsid,UCHAR ScanType,BOOLEAN ChannelSel,void *wdev_obj);
	void (*ez_BssTableSsidSort)(void * ad_obj,void *wdev_obj,EZ_BSS_TABLE *OutTab,
		CHAR Ssid[],UCHAR SsidLen);
	void (*ez_get_scan_table)(void * ad_obj,EZ_BSS_TABLE *ez_scan_tab);
	void (*ez_add_entry_in_apcli_tab)(void * ad_obj, void* wdev_obj, ULONG bss_idx);
	void (*ez_ApCliBssTabInit)(void * ad_obj, void* wdev_obj);
	BOOLEAN (*ez_update_cli_conn)(void * ad_obj, void * ezdev, EZ_BSS_ENTRY *bss_entry);
	void (*ez_update_partial_scan)(void * ez_ad,void * wdev_obj);
	void (*ez_rtmp_set_channel)(void * ad_obj, void * wdev_obj, UINT8 channel);

}ez_chipops_t;


typedef struct __non_ez_band_psk_info_tag{
	unsigned char encrypted_psk[LEN_PSK + EZ_AES_KEY_ENCRYPTION_EXTEND];
}NON_EZ_BAND_PSK_INFO_TAG;


//! Levarage from MP.1.0 CL #170037
typedef struct __non_man_info{
	unsigned char ssid[MAX_LEN_OF_SSID];
	unsigned char ssid_len;	
	unsigned char psk[LEN_PSK];
//! Leverage form MP.1.0 CL 170364
	unsigned char encryptype[32];
	unsigned char authmode[32];

#ifdef DOT11R_FT_SUPPORT
		UINT8 FtMdId[FT_MDID_LEN]; // share MDID info so that all devices use same MDID, irrespective of FT enabled or not.
#else
		UINT8 rsvd[2];
#endif
	
} NON_MAN_INFO;


//! Leverage form MP.1.0 CL 170037
typedef struct __non_man_info_tag{
	unsigned char ssid[MAX_LEN_OF_SSID];
	unsigned char ssid_len;	
	unsigned char encrypted_psk[LEN_PSK + EZ_AES_KEY_ENCRYPTION_EXTEND];
//! Leverage form MP.1.0 CL 170364
	unsigned char encryptype[32];
	unsigned char authmode[32];

#ifdef DOT11R_FT_SUPPORT
	UINT8 FtMdId[FT_MDID_LEN]; // share MDID info so that all devices use same MDID, irrespective of FT enabled or not.
#else
	UINT8 rsvd[2];
#endif

} NON_MAN_INFO_TAG;


typedef enum _enum_loop_chk_role{
    NONE,
	SOURCE,
	DEST
}enum_loop_chk_role;

struct _ez_roam_info
{
	unsigned char ez_apcli_roam_bssid[MAC_ADDR_LEN];
	unsigned char roam_channel;
	unsigned long timestamp;
};

typedef struct GNU_PACKED _loop_chk_info {
	enum_loop_chk_role loop_chk_role;
	UCHAR source_mac[MAC_ADDR_LEN];
}LOOP_CHK_INFO;


typedef struct GNU_PACKED channel_info_s{
	unsigned char channel;
#ifdef EZ_PUSH_BW_SUPPORT
	unsigned char ht_bw;
	unsigned char vht_bw;
#else
	unsigned char rsvd1;
	unsigned char rsvd2;
#endif
	unsigned char extcha;
}channel_info_t;


typedef struct ez_init_params_s{
	void * ad_obj;
	void * wdev_obj;
	char func_idx;
	enum EZDEV_TYPE ezdev_type;
	UCHAR mac_add[MAC_ADDR_LEN];
	UCHAR ssid[MAX_LEN_OF_SSID];
	unsigned char ssid_len;
	UCHAR pmk[PMK_LEN];
	
	unsigned int group_id_len;
	unsigned int ez_group_id_len;	//for localy maintain EzGroupID
	unsigned int gen_group_id_len;  //for localy maintain EzGenGroupID

	unsigned char *group_id;
	unsigned char *ez_group_id;		//for localy maintain EzGroupID
	unsigned char *gen_group_id;	//for localy maintain EzGenGroupID	

	unsigned int open_group_id_len;
	unsigned char open_group_id[OPEN_GROUP_MAX_LEN];

	void *chipops_lut;
	char channel;
	UINT32 os_hz;
	void *ez_scan_timer;
	void *ez_stop_scan_timer;
	void *ez_scan_pause_timer;
	void *ez_group_merge_timer;
	void *ez_loop_chk_timer;
	void *ez_connect_wait_timer;

	channel_info_t channel_info;
} ez_init_params_t;

typedef struct GNU_PACKED interface_info_tag_s
{
	unsigned char ssid[MAX_LEN_OF_SSID];
	unsigned char ssid_len;
	channel_info_t channel_info;
	unsigned char ap_mac_addr[MAC_ADDR_LEN];//! this band AP MAC
	unsigned char cli_mac_addr[MAC_ADDR_LEN];//! this band CLI MAC
	unsigned char link_duplicate;//! when seen in CLI context it meens that other band CLI is also connected to same repeater, if seen in AP context(ez_peer) it means that both CLIs of other repeater are connected to me
#ifdef DOT11R_FT_SUPPORT
	UINT8 FtMdId[FT_MDID_LEN]; // share MDID info so that all devices use same MDID, irrespective of FT enabled or not.
#else
	UINT8 rsvd[2];
#endif
} interface_info_tag_t;

typedef struct GNU_PACKED interface_info_s{
	interface_info_tag_t shared_info;
	unsigned char cli_peer_ap_mac[MAC_ADDR_LEN];//! mac address of AP to which my cli connects, will be mostly used in CLI wdev context
	BOOLEAN non_easy_connection;
	unsigned char interface_activated;
	unsigned char pmk[EZ_PMK_LEN];
}interface_info_t;


struct _ez_security {
	void *ad;
	void *ezdev;
#ifdef EZ_API_SUPPORT
	enum_ez_api_mode ez_api_mode;
#endif
	spinlock_t		ez_apcli_list_sem_lock;

	void * ez_scan_timer;
	void * ez_stop_scan_timer;
	void * ez_scan_pause_timer;
	void * ez_group_merge_timer;

	spinlock_t	ez_scan_pause_timer_lock;
	signed char best_ap_rssi_threshold;
	unsigned int capability;
	unsigned int group_id_len;
	unsigned int ez_group_id_len;	//for localy maintain EzGroupID
	unsigned int gen_group_id_len;  //for localy maintain EzGenGroupID
	unsigned char *group_id;
	unsigned char *ez_group_id;		//for localy maintain EzGroupID
	unsigned char *gen_group_id;	//for localy maintain EzGenGroupID	
	unsigned char self_dh_random_seed[EZ_RAW_KEY_LEN]; /* do NOT change after configured */
	unsigned char self_pke[EZ_RAW_KEY_LEN];
	unsigned char self_pkr[EZ_RAW_KEY_LEN];
	unsigned char self_nonce[EZ_NONCE_LEN];
	unsigned char keep_finding_provider;
	unsigned char first_scan;
	unsigned char scan_one_channel;
	unsigned char client_count;
	unsigned char go_internet;
	unsigned char user_configured;
	signed char rssi_threshold;
#ifdef EZ_NETWORK_MERGE_SUPPORT

	unsigned char merge_peer_addr[MAC_ADDR_LEN];
#ifdef DISCONNECT_ON_CONFIG_UPDATE
	unsigned char force_connect_bssid[MAC_ADDR_LEN];
	unsigned long force_bssid_timestamp;
#endif
	unsigned char weight_update_going_on; //! flag will be set when force weight is pushed, normal action frame will not be processed in this case
	unsigned char do_not_restart_interfaces;//! flag will be set while calling rtmp_set_channel to avoid CLI down/up
	unsigned char delay_disconnect_count;//! to increase beacon miss duration
    interface_info_t this_band_info;//! ssid, pmk, mac address, and CLI peer MAC address, information of wdev to which wdev correspond
	interface_info_t other_band_info_backup;//! ssid, pmk, mac address, and CLI peer MAC address, information of wdev to which wdev correspond
	BOOLEAN ap_did_fallback;
	unsigned char fallback_channel;

#endif
#ifdef NEW_CONNECTION_ALGO
	
	BOOLEAN ez_apcli_immediate_connect;
	BOOLEAN ez_connection_permission_backup;
	BOOLEAN ez_is_connection_allowed;
	unsigned char ez_apcli_force_ssid[MAX_LEN_OF_SSID];
	unsigned char ez_apcli_force_ssid_len;
	unsigned char ez_apcli_force_bssid[MAC_ADDR_LEN];
	unsigned char ez_apcli_force_channel;
	
	unsigned int open_group_id_len;
	unsigned char open_group_id[OPEN_GROUP_MAX_LEN];
#ifdef EZ_ROAM_SUPPORT
	struct _ez_roam_info ez_roam_info;
	UCHAR ez_ap_roam_blocked_mac[MAC_ADDR_LEN];
#endif

#ifdef EZ_DUAL_BAND_SUPPORT
	BOOLEAN internal_force_connect_bssid;
	BOOLEAN internal_force_connect_bssid_timeout;
	unsigned long force_connect_bssid_time_stamp;
    LOOP_CHK_INFO loop_chk_info;
	void * ez_loop_chk_timer;
	unsigned char ez_loop_chk_timer_running;
	BOOLEAN first_loop_check;
	BOOLEAN dest_loop_detect;
#endif
	unsigned char ez_action_type;
	UINT32 ez_scan_delay;
	UINT32 ez_max_scan_delay;
	BOOLEAN ez_scan;
	BOOLEAN ez_scan_same_channel;
	ULONG	ez_scan_same_channel_timestamp;
#endif
	BOOLEAN ez_wps_reconnect;
	unsigned char ez_wps_bssid[6];
	ULONG	ez_wps_reconnect_timestamp;
	BOOLEAN disconnect_by_ssid_update;

	unsigned char default_ssid[MAX_LEN_OF_SSID];
	unsigned char default_ssid_len;
	unsigned char default_pmk[EZ_PMK_LEN];
	unsigned char default_pmk_valid;
	ULONG partial_scan_time_stamp;
	BOOLEAN bPartialScanRunning;
};

typedef struct ez_dev_s{
	void *ad;
	void *wdev;
	enum EZDEV_TYPE ezdev_type;
	char channel;
	char own_mac_addr[MAC_ADDR_LEN];
	char if_addr[MAC_ADDR_LEN];
	char bssid[MAC_ADDR_LEN];
	unsigned char ez_band_idx;
	struct _ez_security ez_security;
	char non_ez_connection;
	spinlock_t ez_peer_table_lock;
	unsigned int os_hz;
	ez_chipops_t *chipops;
} ez_dev_t;


typedef struct GNU_PACKED weight_defining_link_s {
	void *ezdev;
	ULONG time_stamp;
	ULONG ap_time_stamp;
	UCHAR peer_mac[MAC_ADDR_LEN];
	UCHAR peer_ap_mac[MAC_ADDR_LEN];
}weight_defining_link_t;


typedef struct GNU_PACKED device_info_s{

	unsigned char network_weight[NETWORK_WEIGHT_LEN];
	weight_defining_link_t weight_defining_link;
	EZ_NODE_NUMBER ez_node_number;
} device_info_t;




typedef struct __ez_triband_sec_config
{
	UINT32 PairwiseCipher;
	UINT32 GroupCipher;    
	UINT32 AKMMap;
	
} EZ_TRIBAND_SEC_CONFIG;


typedef struct __non_ez_band_info_tag{
	unsigned char ssid[MAX_LEN_OF_SSID];
	unsigned char ssid_len;	
	EZ_TRIBAND_SEC_CONFIG triband_sec;	
	unsigned char encrypted_pmk[EZ_PMK_LEN + EZ_AES_KEY_ENCRYPTION_EXTEND];
	//unsigned char encrypted_psk[LEN_PSK + EZ_AES_KEY_ENCRYPTION_EXTEND];
#ifdef DOT11R_FT_SUPPORT
	UINT8 FtMdId[FT_MDID_LEN]; // share MDID info so that all devices use same MDID, irrespective of FT enabled or not.
#else
	UINT8 rsvd[2];
#endif

}NON_EZ_BAND_INFO_TAG;

struct _ez_peer_security_info {
	void *ad;
	void *ezdev;
	unsigned char ez_band_idx;
	unsigned int capability;
	unsigned int group_id_len;
	unsigned int gtk_len;
	unsigned char *group_id;
	unsigned char *gtk;
	unsigned char mac_addr[MAC_ADDR_LEN];
	unsigned char peer_pke[EZ_RAW_KEY_LEN];
	unsigned char peer_nonce[EZ_NONCE_LEN];
	unsigned char dh_key[EZ_DH_KEY_LEN];
	unsigned char sw_key[EZ_PTK_LEN];
	//unsigned char pmk[EZ_PMK_LEN];
	unsigned char valid;

#if defined (NEW_CONNECTION_ALGO) || defined (EZ_NETWORK_MERGE_SUPPORT)
	unsigned char port_secured;
	unsigned int open_group_id_len;
	unsigned char open_group_id[OPEN_GROUP_MAX_LEN];

#endif

#ifdef EZ_NETWORK_MERGE_SUPPORT	
#ifdef EZ_DUAL_BAND_SUPPORT
	device_info_t device_info;// when device info is used in ez_peer, only node number is expected to give correct information, others are session variable which does not hold any significance after comparision
	interface_info_t this_band_info;
	interface_info_t other_band_info;
#else
	unsigned char ssid_len;
	unsigned char ssid[MAX_LEN_OF_SSID];
	unsigned char other_band_ssid_len;
	unsigned char other_band_ssid[MAX_LEN_OF_SSID];
	unsigned char other_band_pmk[PMK_LEN];
	unsigned char network_weight[NETWORK_WEIGHT_LEN];
	unsigned char ap_mac_addr[MAC_ADDR_LEN];
	unsigned char target_channel;
	unsigned char ht_bw;
	unsigned char vht_bw;
	unsigned char ext_cha_offset;
#endif	
#endif	

#ifdef NEW_CONNECTION_ALGO
	unsigned long creation_time;
	unsigned char ez_peer_table_index;
#endif
	BOOLEAN delete_in_differred_context;
	BOOLEAN ez_disconnect_due_roam;
	NON_EZ_BAND_INFO_TAG non_ez_band_info[MAX_NON_EZ_BANDS];
	NON_EZ_BAND_PSK_INFO_TAG non_ez_psk_info[MAX_NON_EZ_BANDS];
	//! Levarage from MP1.0 CL#170037
	NON_MAN_INFO_TAG non_man_info;
	ez_chipops_t *chipops;
};


typedef struct __ez_band_info{
	void *pAd;
	CHAR func_idx; 
	ez_dev_t ap_ezdev;
	ez_dev_t cli_ezdev;
	struct _ez_peer_security_info ez_peer_table[MAX_EZ_PEERS_PER_BAND];
	ez_chipops_t lut_chipops;
} EZ_BAND_INFO;


typedef struct non_ez_chipops_s{
	//unsigned char (*RandomByte)(void *);
	int (*RtmpOSWrielessEventSendExt)(	void *ad,int band_id,UINT32 eventType,INT flags,PUCHAR pSrcMac,PUCHAR pData,UINT32 dataLen);
	void (*ez_update_non_ez_ap)(void * ad_obj, void *non_ez_and_info_tag, void *non_ez_band_info, void *updated_configs);
	void (*HwCtrlWifiSysRestart)(void * ez_ad, INT band_id);
	void (*ez_send_broadcast_deauth)(void *ad, void *wdev);
	void (*ez_init_non_ez_ap)(void *ad, void *wdev);
}non_ez_chipops_t;


typedef struct __non_ez_band_info{
	void *ez_ad;
	void *pAd;
	CHAR func_idx; 
	void *non_ez_ap_wdev;
	void *non_ez_cli_wdev;
	non_ez_chipops_t lut_chipops;
	unsigned char ssid[MAX_LEN_OF_SSID];
	unsigned char ssid_len;	
	EZ_TRIBAND_SEC_CONFIG triband_sec;	
	BOOLEAN need_restart;
	unsigned char pmk[EZ_PMK_LEN];
	char channel;
	unsigned char psk[EZ_PMK_LEN];
#ifdef DOT11R_FT_SUPPORT
		UINT8 FtMdId[FT_MDID_LEN]; // share MDID info so that all devices use same MDID, irrespective of FT enabled or not.
#else
		UINT8 rsvd[2];
#endif
	
} NON_EZ_BAND_INFO;

typedef struct __ez_adapter{
	unsigned char band_count;
	EZ_BAND_INFO ez_band_info[MAX_EZ_BANDS];
	unsigned char non_ez_band_count;
	NON_EZ_BAND_INFO non_ez_band_info[MAX_NON_EZ_BANDS];
	unsigned int backhaul_channel;
	unsigned int front_end_channel;
	
#ifdef EZ_PUSH_BW_SUPPORT
	BOOLEAN push_bw_config;
#endif
#ifdef EZ_API_SUPPORT
		enum_ez_api_mode ez_api_mode;
#endif

	UINT8 ez_roam_time;
	unsigned char ez_delay_disconnect_count;
	UINT8 ez_wait_for_info_transfer;
	UINT8 ez_wdl_missing_time;
	UINT32 ez_force_connect_bssid_time;
	UINT8 ez_peer_entry_age_out_time;
	UINT8 ez_scan_same_channel_time;
	UINT32 ez_partial_scan_time;
	signed char best_ap_rssi_threshld[10];
	unsigned char best_ap_rssi_threshld_max;
	UINT32 max_scan_delay;
	
	void *ez_connect_wait_timer;
	unsigned char ez_connect_wait_timer_running;
	unsigned long ez_connect_wait_timer_value;
	unsigned long ez_connect_wait_timer_timestamp;
	unsigned char configured_status; /* 0x01 - un-configured, 0x02 - configured */
	device_info_t device_info;//! network weight, node number and weight defining link info, all interface should have same content in htis structure
	char Peer2p4mac[MAC_ADDR_LEN];
	//! Levarage from MP1.0 CL#170037
	unsigned char is_man_nonman;
	NON_MAN_INFO non_man_info;	
	
#ifdef DUAL_CHIP		
		spinlock_t ez_handle_disconnect_lock;
		spinlock_t ez_beacon_update_lock;
		spinlock_t ez_miniport_lock;
		spinlock_t ez_set_channel_lock;
		spinlock_t ez_set_peer_lock;
		spinlock_t ez_conn_perm_lock;
		spinlock_t ez_mlme_sync_lock;
#endif	

	ez_dev_t *ezdev_list[EZDEV_NUM_MAX];
	BOOLEAN SingleChip;
	int debug;
//! Levarage from MP1.0 CL 170210
//! repeater device flag removed from ez_adapter
} EZ_ADAPTER;



#endif
