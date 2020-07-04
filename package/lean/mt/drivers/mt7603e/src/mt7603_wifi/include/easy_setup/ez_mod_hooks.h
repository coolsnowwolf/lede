#ifndef __EZ_MOD_HOOKS__
#define __EZ_MOD_HOOKS__
#include "shared_structs.h"

extern EZ_ADAPTER * ez_init_hook(void *driver_ad, void *wdev, unsigned char ap_mode);
extern void *ez_get_adapter_hook(void);

extern unsigned long ez_build_beacon_ie_hook(
	ez_dev_t *ezdev,
	unsigned char *frame_buf);

extern unsigned long ez_build_probe_request_ie_hook(
	ez_dev_t *ezdev,
	unsigned char *frame_buf);


extern unsigned long ez_build_probe_response_ie_hook(
	ez_dev_t *ezdev,
	unsigned char *frame_buf);

extern unsigned long ez_build_auth_request_ie_hook(
	ez_dev_t *ezdev,
	unsigned char *peer_addr,
	unsigned char *frame_buf);

extern unsigned long ez_build_auth_response_ie_hook(
	ez_dev_t *ezdev,
	unsigned char *peer_addr,
	unsigned char *frame_buf);

extern unsigned long ez_build_assoc_request_ie_hook(
	ez_dev_t *ezdev,
	unsigned char *peer_addr,
	unsigned char *frame_buf,
	unsigned int frame_buf_len);

extern unsigned long ez_build_assoc_response_ie_hook(
	ez_dev_t *ezdev,
	unsigned char *peer_addr,
	unsigned char *ap_gtk,
	unsigned int ap_gtk_len,
	unsigned char *frame_buf);

extern unsigned char ez_process_probe_request_hook(
	ez_dev_t *ezdev,
	unsigned char *peer_addr,
	void *msg,
	unsigned long msg_len);

extern void ez_process_beacon_probe_response_hook(
	ez_dev_t *ezdev,
	void *msg,
	unsigned long msg_len);

extern unsigned char ez_process_auth_request_hook(
	ez_dev_t *ezdev,
	void *auth_info_obj,
	void *msg,
	unsigned long msg_len);


extern USHORT ez_process_auth_response_hook(
	ez_dev_t *ezdev,
	unsigned char *peer_addr,
	void *msg,
	unsigned long msg_len);

extern unsigned short ez_process_assoc_request_hook(
	ez_dev_t *ezdev,
	unsigned char *peer_addr,
	UCHAR *easy_setup_mic_valid,
	unsigned char isReassoc,
	void *msg,
	unsigned long msg_len);

extern unsigned short ez_process_assoc_response_hook(
	ez_dev_t *ezdev,
	unsigned char *peer_addr,
	void *msg,
	unsigned long msg_len);

extern void ez_show_information_hook(
	ez_dev_t *ezdev);


extern INT ez_send_broadcast_deauth_proc_hook
	(ez_dev_t *ezdev);

extern unsigned char ez_set_ezgroup_id_hook(
	ez_dev_t *ezdev,
	unsigned char *ez_group_id,
	unsigned int ez_group_id_len,
	unsigned char inf_idx);

extern unsigned char ez_set_group_id_hook(
	ez_dev_t *ezdev,
	unsigned char *group_id,
	unsigned int group_id_len,
	unsigned char inf_idx);

extern unsigned char ez_set_gen_group_id_hook(
	ez_dev_t *ezdev,
	unsigned char *gen_group_id,
	unsigned int gen_group_id_len,
	unsigned char inf_idx);

extern void ez_set_rssi_threshold_hook(
	ez_dev_t *ezdev,
	char rssi_threshold);

extern void ez_set_max_scan_delay_hook(
	ez_dev_t *ezdev,
	UINT32 max_scan_delay);


extern void ez_set_api_mode_hook(
	ez_dev_t *ezdev,
	char ez_api_mode);

extern INT ez_merge_group_hook(
	ez_dev_t *ezdev, 
	UCHAR *macAddress);


extern void ez_apcli_force_ssid_hook(
	ez_dev_t *ezdev,
	unsigned char *ssid, 
	unsigned char ssid_len);

extern void ez_set_force_bssid_hook(
	ez_dev_t *ezdev, 
	UCHAR *mac_addr);

extern void ez_set_push_bw_hook(ez_dev_t *ezdev, UINT8 same_bw_push);

extern void ez_handle_action_txstatus_hook(ez_dev_t *ezdev, UINT8 * Addr);

extern void set_ssid_psk_hook(ez_dev_t *ezdev, 
	char *ssid1, char *pmk1, char *psk1, 
	char *ssid2, char *pmk2, char *psk2, 
	char *ssid3, char *pmk3, char *psk3, 
	char *EncrypType1, char *EncrypType2, 
	char *AuthMode1, char *AuthMode2);


extern void ez_apcli_link_down_hook(ez_dev_t *ezdev,unsigned long Disconnect_Sub_Reason);

extern BOOLEAN ez_update_connection_permission_hook(
	ez_dev_t *ezdev, enum EZ_CONN_ACTION action);

extern BOOLEAN ez_is_connection_allowed_hook(ez_dev_t *ezdev);

extern BOOLEAN ez_probe_rsp_join_action_hook(ez_dev_t *ezdev, 
	char *network_weight);

extern void ez_update_connection_hook(ez_dev_t *ezdev);

//extern void ez_handle_pairmsg4_hook(ez_dev_t *ezdev);
void ez_handle_pairmsg4_hook(ez_dev_t *ezdev, UCHAR *peer_mac);

extern void ez_roam_hook(ez_dev_t *ezdev, 
	unsigned char bss_support_easy_setup,
	beacon_info_tag_t* bss_beacon_info,
	char* bss_bssid,
	UCHAR bss_channel);

extern BOOLEAN ez_set_roam_bssid_hook(ez_dev_t *ezdev, UCHAR *roam_bssid);

extern void ez_reset_roam_bssid_hook(ez_dev_t *ezdev);

extern BOOLEAN ez_get_push_bw_hook(ez_dev_t *ezdev);

extern channel_info_t * ez_get_channel_hook(ez_dev_t *ezdev);

extern BOOLEAN ez_did_ap_fallback_hook(ez_dev_t *ezdev);


extern BOOLEAN ez_ap_fallback_channel(ez_dev_t *ezdev);

extern void ez_prepare_security_key_hook(
	ez_dev_t *ezdev,
	unsigned char *peer_addr,
	unsigned char authenticator);

extern enum_group_merge_action_t is_group_merge_candidate_hook(unsigned int easy_setup_capability, 
	ez_dev_t *ezdev, 
	void *temp_bss_entry,
	UCHAR *Bssid);

extern void ez_process_action_frame_hook(
	ez_dev_t *ezdev,
	UCHAR *peer_mac,
	UCHAR *Msg,
	UINT msg_len);

extern BOOLEAN check_best_ap_rssi_threshold_hook(ez_dev_t *ezdev, char rssi);

extern void ez_set_ap_fallback_context_hook(ez_dev_t *ezdev, BOOLEAN fallback, unsigned char fallback_channel);

extern struct _ez_peer_security_info *ez_peer_table_search_by_addr_hook(
	ez_dev_t *ezdev,
	unsigned char *addr);

extern void ez_set_delete_peer_in_differed_context_hook(
	void * ezdev, 
	struct _ez_peer_security_info *ez_peer, 
	BOOLEAN set);

extern BOOLEAN ez_is_triband_hook(void);

extern void * ez_get_otherband_ad_hook(ez_dev_t *ezdev);

extern unsigned short ez_check_for_ez_enable_hook(
	void *msg,
	unsigned long msg_len
	);

extern void ez_update_ap_cap_hook(ez_dev_t *ezdev, BOOLEAN set, unsigned int capability);

extern void ez_set_delay_disconnect_count_hook(ez_dev_t *ezdev, unsigned char count);

extern void ez_acquire_lock_hook(EZ_ADAPTER *ez_ad, ez_dev_t *ezdev,unsigned char lock_id);

extern void ez_release_lock_hook(EZ_ADAPTER *ez_ad, ez_dev_t *ezdev,unsigned char lock_id);

extern BOOLEAN ez_is_weight_same_hook(ez_dev_t *ezdev, UCHAR *peer_weight);

extern BOOLEAN ez_is_other_band_mlme_running_hook(ez_dev_t *ezdev);

extern BOOLEAN ez_handle_scan_channel_restore_hook(ez_dev_t *ezdev);

extern void ez_triband_insert_tlv_hook(void *ez_ad, 
	unsigned int tag_ID, UCHAR * buffer, ULONG* tag_len);

extern void ez_handle_peer_disconnection_hook(ez_dev_t *ezdev, unsigned char * mac_addr);

extern BOOLEAN ez_sta_rx_pkt_handle_hook(ez_dev_t *ezdev, UCHAR *pPayload, UINT MPDUtotalByteCnt);

extern BOOLEAN ez_apcli_rx_grp_pkt_drop_hook(ez_dev_t *ezdev,unsigned char * dest);

extern BOOLEAN ez_apcli_tx_grp_pkt_drop_hook(ez_dev_t *ezdev,struct sk_buff *pSkb);

extern void send_delay_disconnect_to_peers_hook(void *ap_ezdev);
extern BOOLEAN ez_internet_msghandle_hook(ez_dev_t *ezdev, p_internet_command_t p_internet_command);
extern void ez_custom_data_handle_hook(EZ_ADAPTER *ez_ad, p_ez_custom_data_cmd_t p_custom_data, int length);
extern BOOLEAN ez_is_roaming_ongoing_hook(EZ_ADAPTER *ez_ad);
extern void ez_peer_table_maintenance_hook(EZ_ADAPTER *ez_ad);
extern BOOLEAN ez_port_secured_hook(
	ez_dev_t *ezdev,
	UCHAR *peer_mac,
	unsigned char ap_mode);
extern void ez_ap_peer_beacon_action_hook(ez_dev_t *ezdev, unsigned char * mac_addr, int peer_capability);
extern int  ez_handle_send_packets_hook(ez_dev_t *ezdev, NDIS_PACKET *pPacket);

extern BOOLEAN ez_set_open_group_id_hook(
	ez_dev_t *ezdev,
	unsigned char *open_group_id,
	unsigned int open_group_id_len,
	unsigned char inf_idx);

extern void APTribandRestartNonEzReqAction_hook(EZ_ADAPTER *ez_ad);
extern BOOLEAN ez_ap_tx_grp_pkt_drop_to_ez_apcli_hook(ez_dev_t *ezdev, struct sk_buff *pSkb);

extern ez_dev_t * ez_start_hook(ez_init_params_t *ez_init_params);
extern void ez_exit_hook(void *driver_ad);
extern VOID ez_connection_alllow_all_hook(EZ_ADAPTER *ez_ad);
extern void ez_stop_hook(
	ez_dev_t * ezdev);

VOID ez_connection_allow_all_hook(EZ_ADAPTER *ez_ad, unsigned char default_pmk_valid);


extern VOID ez_scan_timeout_hook(ez_dev_t *ezdev);

//extern void ez_stop_scan_timeout_hook(ez_dev_t *ezdev);

extern void ez_group_merge_timeout_hook(ez_dev_t *ezdev);

extern VOID ez_loop_chk_timeout_hook(
	ez_dev_t *ezdev);

extern BOOLEAN ez_need_bypass_rx_fwd_hook(ez_dev_t *ezdev);
extern void increment_best_ap_rssi_threshold_hook(ez_dev_t *ezdev);

void ez_allocate_or_update_non_ez_band_hook(void *wdev, void *ad, UINT32 ezdev_type, CHAR func_idx, non_ez_driver_ops_t *driver_ops, UCHAR *channel);
extern void ez_initiate_new_scan_hook(EZ_ADAPTER *ez_ad);
#ifdef IF_UP_DOWN
BOOLEAN ez_check_valid_hook(void);
BOOLEAN ez_all_intf_up_hook(EZ_ADAPTER *ez_ad);
void ez_apcli_disconnect_both_intf_hook(ez_dev_t *ezdev, PUCHAR bssid);
#endif

#endif
