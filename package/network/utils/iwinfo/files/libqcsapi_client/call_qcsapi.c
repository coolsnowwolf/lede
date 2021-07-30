/*SH0
*******************************************************************************
**                                                                           **
**         Copyright (c) 2009 - 2012 Quantenna Communications, Inc.          **
**                                                                           **
**  File        : call_qcsapi.c                                              **
**  Description :                                                            **
**                                                                           **
*******************************************************************************
**                                                                           **
**  Redistribution and use in source and binary forms, with or without       **
**  modification, are permitted provided that the following conditions       **
**  are met:                                                                 **
**  1. Redistributions of source code must retain the above copyright        **
**     notice, this list of conditions and the following disclaimer.         **
**  2. Redistributions in binary form must reproduce the above copyright     **
**     notice, this list of conditions and the following disclaimer in the   **
**     documentation and/or other materials provided with the distribution.  **
**  3. The name of the author may not be used to endorse or promote products **
**     derived from this software without specific prior written permission. **
**                                                                           **
**  Alternatively, this software may be distributed under the terms of the   **
**  GNU General Public License ("GPL") version 2, or (at your option) any    **
**  later version as published by the Free Software Foundation.              **
**                                                                           **
**  In the case this software is distributed under the GPL license,          **
**  you should have received a copy of the GNU General Public License        **
**  along with this software; if not, write to the Free Software             **
**  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA  **
**                                                                           **
**  THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR       **
**  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES**
**  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  **
**  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,         **
**  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT **
**  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,**
**  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY    **
**  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT      **
**  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF **
**  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.        **
**                                                                           **
*******************************************************************************
EH0*/

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <errno.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <net80211/ieee80211_qos.h>
#include <net80211/ieee80211_dfs_reentry.h>
#include <net80211/ieee80211_ioctl.h>

#include <qtn/lhost_muc_comm.h>
#include <qtn/qtn_vlan.h>

#include "qcsapi.h"
#include "qcsapi_driver.h"
#include "call_qcsapi.h"
#include "qcsapi_sem.h"
#include "qcsapi_util.h"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof (a) / sizeof ((a)[0]))
#endif

#ifndef max
#define max(a, b) ((a) < (b) ? (b) : (a))
#endif

#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef IS_MULTIPLE_BITS_SET
#define IS_MULTIPLE_BITS_SET(_x)	(((unsigned)(_x)) & (((unsigned)(_x)) - 1))
#endif

#define printf	Do_not_use_printf
#define fprintf	Do_not_use_fprintf

#define IP_ADDR_STR_LEN 16
#define BEACON_INTERVAL_WARNING_LOWER_LIMIT	24
#define BEACON_INTERVAL_WARNING_UPPER_LIMIT	100

static const struct
{
	qcsapi_entry_point	 e_entry_point;
	const char		*api_name;
} qcsapi_entry_name[] =
{
	{ e_qcsapi_errno_get_message,		"get_error_message" },
	{ e_qcsapi_store_ipaddr,		"store_ipaddr" },
	{ e_qcsapi_interface_enable,		"enable_interface" },
	{ e_qcsapi_interface_get_BSSID,		"interface_BSSID" },
	{ e_qcsapi_interface_get_mac_addr,	"get_mac_addr" },
	{ e_qcsapi_interface_get_mac_addr,	"get_macaddr" },
	{ e_qcsapi_interface_set_mac_addr,	"set_mac_addr" },
	{ e_qcsapi_interface_set_mac_addr,	"set_macaddr" },
	{ e_qcsapi_interface_get_counter,	"get_counter" },
	{ e_qcsapi_interface_get_counter64,	"get_counter64" },
	{ e_qcsapi_pm_get_counter,		"get_pm_counter" },
	{ e_qcsapi_pm_get_elapsed_time,		"get_pm_elapsed_time" },
	{ e_qcsapi_flash_image_update,		"flash_image_update" },
	{ e_qcsapi_firmware_get_version,	"get_firmware_version" },
	{ e_qcsapi_system_get_time_since_start,	"get_time_since_start" },
	{ e_qcsapi_get_system_status,		"get_sys_status" },
	{ e_qcsapi_get_random_seed,		"get_random_seed" },
	{ e_qcsapi_set_random_seed,		"set_random_seed" },
	{ e_qcsapi_led_get,			"get_LED" },
	{ e_qcsapi_led_set,			"set_LED" },
	{ e_qcsapi_led_pwm_enable,		"set_LED_PWM" },
	{ e_qcsapi_led_brightness,		"set_LED_brightness" },
	{ e_qcsapi_gpio_get_config,		"get_GPIO_config" },
	{ e_qcsapi_gpio_set_config,		"set_GPIO_config" },
	{ e_qcsapi_gpio_monitor_reset_device,	"monitor_reset_device" },
	{ e_qcsapi_gpio_enable_wps_push_button,	"enable_wps_push_button" },
	{ e_qcsapi_file_path_get_config,	"get_file_path" },
	{ e_qcsapi_file_path_set_config,	"set_file_path" },
	{ e_qcsapi_wifi_set_wifi_macaddr,	"set_wifi_mac_addr" },
	{ e_qcsapi_wifi_set_wifi_macaddr,	"set_wifi_macaddr" },
	{ e_qcsapi_wifi_create_restricted_bss,	"wifi_create_restricted_bss"},
	{ e_qcsapi_wifi_create_bss,		"wifi_create_bss"},
	{ e_qcsapi_wifi_remove_bss,		"wifi_remove_bss"},
	{ e_qcsapi_wifi_get_primary_interface,	"get_primary_interface"},
	{ e_qcsapi_wifi_get_interface_by_index,	"get_interface_by_index"},
	{ e_qcsapi_wifi_get_mode,		"get_mode" },
	{ e_qcsapi_wifi_set_mode,		"set_mode" },
	{ e_qcsapi_wifi_get_phy_mode,		"get_phy_mode" },
	{ e_qcsapi_wifi_set_phy_mode,		"set_phy_mode" },
	{ e_qcsapi_wifi_reload_in_mode,		"reload_in_mode" },
	{ e_qcsapi_wifi_rfenable,		"rfenable" },
	{ e_qcsapi_service_control,             "service_control" },
	{ e_qcsapi_wfa_cert,			"wfa_cert" },
	{ e_qcsapi_wifi_rfstatus,		"rfstatus" },
	{ e_qcsapi_wifi_startprod,		"startprod" },
	{ e_qcsapi_wifi_get_bw,			"get_bw" },
	{ e_qcsapi_wifi_set_bw,			"set_bw" },
	{ e_qcsapi_wifi_get_BSSID,		"get_BSSID" },
	{ e_qcsapi_wifi_get_config_BSSID,	"get_config_BSSID" },
	{ e_qcsapi_wifi_ssid_get_bssid,		"get_ssid_bssid" },
	{ e_qcsapi_wifi_ssid_set_bssid,		"set_ssid_bssid" },
	{ e_qcsapi_wifi_get_SSID,		"get_SSID" },
	{ e_qcsapi_wifi_set_SSID,		"set_SSID" },
	{ e_qcsapi_wifi_get_channel,		"get_channel" },
	{ e_qcsapi_wifi_set_channel,		"set_channel" },
	{ e_qcsapi_wifi_get_auto_channel,	"get_auto_channel" },
	{ e_qcsapi_wifi_set_auto_channel,	"set_auto_channel" },
	{ e_qcsapi_wifi_get_standard,		"get_standard" },
	{ e_qcsapi_wifi_get_standard,		"get_802.11" },
	{ e_qcsapi_wifi_get_dtim,		"get_dtim" },
	{ e_qcsapi_wifi_set_dtim,		"set_dtim" },
	{ e_qcsapi_wifi_get_assoc_limit,	"get_dev_assoc_limit" },
	{ e_qcsapi_wifi_set_assoc_limit,	"set_dev_assoc_limit" },
	{ e_qcsapi_wifi_get_bss_assoc_limit,	"get_bss_assoc_limit" },
	{ e_qcsapi_wifi_set_bss_assoc_limit,	"set_bss_assoc_limit" },
	{ e_qcsapi_interface_get_status,	"get_status" },
	{ e_qcsapi_interface_set_ip4,		"set_ip" },
	{ e_qcsapi_interface_get_ip4,		"get_ip" },
	{ e_qcsapi_wifi_get_list_channels,	"get_list_of_channels" },
	{ e_qcsapi_wifi_get_list_channels,	"get_channel_list" },
	{ e_qcsapi_wifi_get_mode_switch,	"get_mode_switch" },
	{ e_qcsapi_wifi_get_mode_switch,	"get_wifi_mode_switch" },
	{ e_qcsapi_wifi_get_noise,		"get_noise" },
	{ e_qcsapi_wifi_get_rssi_by_chain,	"get_rssi_by_chain" },
	{ e_qcsapi_wifi_get_avg_snr,		"get_avg_snr" },
	{ e_qcsapi_wifi_get_option,		"get_option" },
	{ e_qcsapi_wifi_set_option,		"set_option" },
	{ e_qcsapi_wifi_get_rates,		"get_rates" },
	{ e_qcsapi_wifi_set_rates,		"set_rates" },
	{ e_qcsapi_wifi_get_max_bitrate,	"get_max_bitrate" },
	{ e_qcsapi_wifi_set_max_bitrate,	"set_max_bitrate" },
	{ e_qcsapi_wifi_get_beacon_type,	"get_beacon_type" },
	{ e_qcsapi_wifi_get_beacon_type,	"get_beacon" },
	{ e_qcsapi_wifi_set_beacon_type,	"set_beacon_type" },
	{ e_qcsapi_wifi_set_beacon_type,	"set_beacon" },
	{ e_qcsapi_wifi_get_beacon_interval,		"get_beacon_interval" },
	{ e_qcsapi_wifi_set_beacon_interval,		"set_beacon_interval" },
	{ e_qcsapi_wifi_get_list_regulatory_regions,
						"get_regulatory_regions" },
	{ e_qcsapi_wifi_get_list_regulatory_regions,
						"get_list_regulatory_regions" },
	{ e_qcsapi_wifi_get_regulatory_tx_power,
						"get_regulatory_tx_power" },
	{ e_qcsapi_wifi_get_configured_tx_power,
						"get_configured_tx_power" },
	{ e_qcsapi_wifi_set_regulatory_channel, "set_regulatory_channel" },
	{ e_qcsapi_wifi_set_regulatory_region,	"set_regulatory_region" },
	{ e_qcsapi_wifi_get_regulatory_region,	"get_regulatory_region" },
	{ e_qcsapi_wifi_overwrite_country_code,	"overwrite_country_code" },
	{ e_qcsapi_wifi_get_list_regulatory_channels,
						"get_list_regulatory_channels" },
	{ e_qcsapi_wifi_get_list_regulatory_bands,
						"get_list_regulatory_bands" },
	{ e_qcsapi_wifi_get_regulatory_db_version,
						"get_regulatory_db_version" },
	{ e_qcsapi_wifi_set_regulatory_tx_power_cap,
						"apply_regulatory_cap" },
	{ e_qcsapi_wifi_restore_regulatory_tx_power,
						"restore_regulatory_tx_power"},
	{ e_qcsapi_wifi_set_chan_pri_inactive,  "set_chan_pri_inactive" },
	{ e_qcsapi_wifi_set_chan_disabled,	"set_chan_disabled" },
	{ e_qcsapi_wifi_get_chan_disabled,	"get_chan_disabled" },

	{ e_qcsapi_wifi_get_tx_power,		"get_tx_power" },
	{ e_qcsapi_wifi_set_tx_power,		"set_tx_power" },
	{ e_qcsapi_wifi_get_tx_power_ext,	"get_tx_power_ext" },
	{ e_qcsapi_wifi_set_tx_power_ext,	"set_tx_power_ext" },
	{ e_qcsapi_wifi_get_chan_power_table,	"get_chan_power_table" },
	{ e_qcsapi_wifi_set_chan_power_table,	"set_chan_power_table" },
	{ e_qcsapi_wifi_get_bw_power,		"get_bw_power" },
	{ e_qcsapi_wifi_set_bw_power,		"set_bw_power" },
	{ e_qcsapi_wifi_get_bf_power,		"get_bf_power" },
	{ e_qcsapi_wifi_set_bf_power,		"set_bf_power" },
	{ e_qcsapi_wifi_get_power_selection,	"get_power_selection" },
	{ e_qcsapi_wifi_set_power_selection,	"set_power_selection" },
	{ e_qcsapi_wifi_get_carrier_interference,		"get_carrier_db" },
	{ e_qcsapi_wifi_get_congestion_idx,		"get_congest_idx" },
	{ e_qcsapi_wifi_get_supported_tx_power_levels, "get_supported_tx_power" },
	{ e_qcsapi_wifi_get_current_tx_power_level, "get_current_tx_power" },
	{ e_qcsapi_wifi_set_power_constraint, "set_power_constraint"},
	{ e_qcsapi_wifi_get_power_constraint, "get_power_constraint"},
	{ e_qcsapi_wifi_set_tpc_interval, "set_tpc_query_interval"},
	{ e_qcsapi_wifi_get_tpc_interval, "get_tpc_query_interval"},
	{ e_qcsapi_wifi_get_assoc_records,	"get_assoc_records" },
	{ e_qcsapi_wifi_get_list_DFS_channels,	"get_list_DFS_channels" },
	{ e_qcsapi_wifi_is_channel_DFS,		"is_channel_DFS" },
	{ e_qcsapi_wifi_get_DFS_alt_channel,	"get_DFS_alt_channel" },
	{ e_qcsapi_wifi_set_DFS_alt_channel,	"set_DFS_alt_channel" },
	{ e_qcsapi_wifi_set_DFS_reentry,	"start_dfsreentry"},
	{ e_qcsapi_wifi_get_scs_cce_channels,	"get_scs_cce_channels" },
	{ e_qcsapi_wifi_get_dfs_cce_channels,	"get_dfs_cce_channels" },
	{ e_qcsapi_wifi_get_csw_records,	"get_csw_records" },
	{ e_qcsapi_wifi_get_radar_status,	"get_radar_status" },
	{ e_qcsapi_wifi_get_WEP_encryption_level,
						"get_WEP_encryption_level" },
	{ e_qcsapi_wifi_get_WPA_encryption_modes, "get_WPA_encryption_modes" },
	{ e_qcsapi_wifi_set_WPA_encryption_modes, "set_WPA_encryption_modes" },
	{ e_qcsapi_wifi_get_WPA_authentication_mode, "get_WPA_authentication_mode" },
	{ e_qcsapi_wifi_set_WPA_authentication_mode, "set_WPA_authentication_mode" },

	{ e_qcsapi_wifi_get_interworking, "get_interworking" },
	{ e_qcsapi_wifi_set_interworking, "set_interworking" },
	{ e_qcsapi_wifi_get_80211u_params, "get_80211u_params" },
	{ e_qcsapi_wifi_set_80211u_params, "set_80211u_params" },
	{ e_qcsapi_security_get_nai_realms, "get_nai_realms" },
	{ e_qcsapi_security_add_nai_realm, "add_nai_realm" },
	{ e_qcsapi_security_del_nai_realm, "del_nai_realm" },
	{ e_qcsapi_security_add_roaming_consortium, "add_roaming_consortium" },
	{ e_qcsapi_security_del_roaming_consortium, "del_roaming_consortium" },
	{ e_qcsapi_security_get_roaming_consortium, "get_roaming_consortium" },
	{ e_qcsapi_security_get_venue_name, "get_venue_name" },
	{ e_qcsapi_security_add_venue_name, "add_venue_name" },
	{ e_qcsapi_security_del_venue_name, "del_venue_name" },
	{ e_qcsapi_security_get_oper_friendly_name, "get_oper_friendly_name" },
	{ e_qcsapi_security_add_oper_friendly_name, "add_oper_friendly_name" },
	{ e_qcsapi_security_del_oper_friendly_name, "del_oper_friendly_name" },
	{ e_qcsapi_security_get_hs20_conn_capab, "get_hs20_conn_capab" },
	{ e_qcsapi_security_add_hs20_conn_capab, "add_hs20_conn_capab" },
	{ e_qcsapi_security_del_hs20_conn_capab, "del_hs20_conn_capab" },

	{ e_qcsapi_wifi_get_hs20_status, "get_hs20_status" },
	{ e_qcsapi_wifi_set_hs20_status, "set_hs20_status" },
	{ e_qcsapi_wifi_get_hs20_params, "get_hs20_params" },
	{ e_qcsapi_wifi_set_hs20_params, "set_hs20_params" },

	{ e_qcsapi_remove_11u_param, "remove_11u_param" },
	{ e_qcsapi_remove_hs20_param, "remove_hs20_param" },

	{ e_qcsapi_wifi_set_proxy_arp, "set_proxy_arp" },
	{ e_qcsapi_wifi_get_proxy_arp, "get_proxy_arp" },
	{ e_qcsapi_wifi_get_l2_ext_filter, "get_l2_ext_filter" },
	{ e_qcsapi_wifi_set_l2_ext_filter, "set_l2_ext_filter" },

	{ e_qcsapi_wifi_get_IEEE11i_encryption_modes, "get_IEEE11i_encryption_modes" },
	{ e_qcsapi_wifi_set_IEEE11i_encryption_modes, "set_IEEE11i_encryption_modes" },
	{ e_qcsapi_wifi_get_IEEE11i_authentication_mode, "get_IEEE11i_authentication_mode" },
	{ e_qcsapi_wifi_set_IEEE11i_authentication_mode, "set_IEEE11i_authentication_mode" },
	{ e_qcsapi_wifi_get_michael_errcnt, "get_michael_errcnt" },
	{ e_qcsapi_wifi_get_pre_shared_key,	"get_pre_shared_key" },
	{ e_qcsapi_wifi_set_pre_shared_key,	"set_pre_shared_key" },
	{ e_qcsapi_wifi_add_radius_auth_server_cfg,	"add_radius_auth_server_cfg" },
	{ e_qcsapi_wifi_del_radius_auth_server_cfg,	"del_radius_auth_server_cfg" },
	{ e_qcsapi_wifi_get_radius_auth_server_cfg,	"get_radius_auth_server_cfg" },
	{ e_qcsapi_wifi_set_own_ip_addr,	"set_own_ip_addr" },
	{ e_qcsapi_wifi_get_psk_auth_failures,	"get_psk_auth_failures" },
	{ e_qcsapi_wifi_get_pre_shared_key,	"get_PSK" },
	{ e_qcsapi_wifi_set_pre_shared_key,	"set_PSK" },
	{ e_qcsapi_wifi_get_key_passphrase,	"get_passphrase" },
	{ e_qcsapi_wifi_get_key_passphrase,	"get_key_passphrase" },
	{ e_qcsapi_wifi_set_key_passphrase,	"set_passphrase" },
	{ e_qcsapi_wifi_set_key_passphrase,	"set_key_passphrase" },
	{ e_qcsapi_wifi_get_group_key_interval, "get_group_key_interval" },
        { e_qcsapi_wifi_set_group_key_interval, "set_group_key_interval" },
	{ e_qcsapi_wifi_get_pmf,	"get_pmf" },
	{ e_qcsapi_wifi_set_pmf,	"set_pmf" },
	{ e_qcsapi_wifi_get_count_associations,	"get_count_assoc" },
	{ e_qcsapi_wifi_get_count_associations,	"get_count_associations" },
	{ e_qcsapi_wifi_get_count_associations,	"get_association_count" },
	{ e_qcsapi_wifi_get_associated_device_mac_addr,	"get_associated_device_mac_addr" },
	{ e_qcsapi_wifi_get_associated_device_mac_addr,	"get_station_mac_addr" },
	{ e_qcsapi_wifi_get_associated_device_ip_addr,	"get_associated_device_ip_addr" },
	{ e_qcsapi_wifi_get_associated_device_ip_addr,	"get_station_ip_addr" },
	{ e_qcsapi_wifi_get_link_quality,	"get_link_quality" },
	{ e_qcsapi_wifi_get_rssi_per_association, "get_rssi" },
	{ e_qcsapi_wifi_get_hw_noise_per_association, "get_hw_noise" },
	{ e_qcsapi_wifi_get_rssi_in_dbm_per_association, "get_rssi_dbm" },
	{ e_qcsapi_wifi_get_snr_per_association, "get_snr" },
	{ e_qcsapi_wifi_get_rx_bytes_per_association, "get_rx_bytes" },
	{ e_qcsapi_wifi_get_rx_bytes_per_association, "get_assoc_rx_bytes" },
	{ e_qcsapi_wifi_get_tx_bytes_per_association, "get_tx_bytes" },
	{ e_qcsapi_wifi_get_tx_bytes_per_association, "get_assoc_tx_bytes" },
	{ e_qcsapi_wifi_get_rx_packets_per_association, "get_rx_packets" },
	{ e_qcsapi_wifi_get_rx_packets_per_association, "get_assoc_rx_packets" },
	{ e_qcsapi_wifi_get_tx_packets_per_association, "get_tx_packets" },
	{ e_qcsapi_wifi_get_tx_packets_per_association, "get_assoc_tx_packets" },
	{ e_qcsapi_wifi_get_tx_err_packets_per_association,
						"get_tx_err_packets" },
	{ e_qcsapi_wifi_get_tx_err_packets_per_association,
						"get_assoc_tx_err_packets" },
	{ e_qcsapi_wifi_get_bw_per_association, "get_assoc_bw" },
	{ e_qcsapi_wifi_get_tx_phy_rate_per_association, "get_tx_phy_rate" },
	{ e_qcsapi_wifi_get_rx_phy_rate_per_association, "get_rx_phy_rate" },
	{ e_qcsapi_wifi_get_tx_mcs_per_association, "get_tx_mcs" },
	{ e_qcsapi_wifi_get_rx_mcs_per_association, "get_rx_mcs" },
	{ e_qcsapi_wifi_get_achievable_tx_phy_rate_per_association,
						"get_achievable_tx_phy_rate" },
	{ e_qcsapi_wifi_get_achievable_rx_phy_rate_per_association,
						"get_achievable_rx_phy_rate" },
	{ e_qcsapi_wifi_get_auth_enc_per_association, "get_auth_enc_per_assoc" },
	{ e_qcsapi_wifi_get_tput_caps,	"get_tput_caps" },
	{ e_qcsapi_wifi_get_connection_mode,	"get_connection_mode" },
	{ e_qcsapi_wifi_get_vendor_per_association, "get_vendor" },
	{ e_qcsapi_wifi_get_max_mimo,	"get_max_mimo" },

	{ e_qcsapi_wifi_get_node_counter,	"get_node_counter" },
	{ e_qcsapi_wifi_get_node_param,		"get_node_param" },
	{ e_qcsapi_wifi_get_node_stats,		"get_node_stats" },

	{ e_qcsapi_wifi_get_max_queued,		"get_max_queued" },

	{ e_qcsapi_wifi_disassociate,	"disassociate" },
	{ e_qcsapi_wifi_disassociate_sta,	"disassociate_sta" },
	{ e_qcsapi_wifi_reassociate,	"reassociate" },

	{ e_qcsapi_wifi_associate,	"associate" },

	{ e_qcsapi_wifi_get_mac_address_filtering, "get_macaddr_filter" },
	{ e_qcsapi_wifi_set_mac_address_filtering, "set_macaddr_filter" },
	{ e_qcsapi_wifi_is_mac_address_authorized, "is_mac_addr_authorized" },
	{ e_qcsapi_wifi_is_mac_address_authorized, "is_macaddr_authorized" },
	{ e_qcsapi_wifi_get_authorized_mac_addresses, "get_authorized_mac_addr" },
	{ e_qcsapi_wifi_get_authorized_mac_addresses, "get_authorized_macaddr" },
	{ e_qcsapi_wifi_get_denied_mac_addresses, "get_blocked_mac_addr" },
	{ e_qcsapi_wifi_get_denied_mac_addresses, "get_blocked_macaddr" },
	{ e_qcsapi_wifi_get_denied_mac_addresses, "get_denied_mac_addr" },
	{ e_qcsapi_wifi_get_denied_mac_addresses, "get_denied_macaddr" },
	{ e_qcsapi_wifi_authorize_mac_address,	"authorize_mac_addr" },
	{ e_qcsapi_wifi_authorize_mac_address,	"authorize_macaddr" },
	{ e_qcsapi_wifi_deny_mac_address,	"block_macaddr" },
	{ e_qcsapi_wifi_deny_mac_address,	"block_mac_addr" },
	{ e_qcsapi_wifi_deny_mac_address,	"deny_macaddr" },
	{ e_qcsapi_wifi_deny_mac_address,	"deny_mac_addr" },
	{ e_qcsapi_wifi_remove_mac_address,	"remove_mac_addr" },
	{ e_qcsapi_wifi_remove_mac_address,	"remove_macaddr" },
	{ e_qcsapi_wifi_clear_mac_address_filters,	"clear_mac_filters" },
	{ e_qcsapi_wifi_set_mac_address_reserve,	"set_macaddr_reserve" },
	{ e_qcsapi_wifi_get_mac_address_reserve,	"get_macaddr_reserve" },
	{ e_qcsapi_wifi_clear_mac_address_reserve,	"clear_macaddr_reserve" },

	{ e_qcsapi_wifi_backoff_fail_max,	"backoff_fail_max" },
	{ e_qcsapi_wifi_backoff_timeout,	"backoff_timeout" },
	{ e_qcsapi_wifi_get_wpa_status,		"get_wpa_status" },
	{ e_qcsapi_wifi_get_auth_state,		"get_auth_state" },
	{ e_qcsapi_wifi_get_disconn_info,	"get_disconn_info" },
	{ e_qcsapi_wifi_reset_disconn_info,	"reset_disconn_info" },
	{ e_qcsapi_wifi_get_pairing_id,		"get_pairing_id"},
	{ e_qcsapi_wifi_set_pairing_id,		"set_pairing_id"},
	{ e_qcsapi_wifi_get_pairing_enable,	"get_pairing_enable"},
	{ e_qcsapi_wifi_set_pairing_enable,	"set_pairing_enable"},

	{ e_qcsapi_wifi_set_txqos_sched_tbl,	"set_txqos_sched_tbl" },
	{ e_qcsapi_wifi_get_txqos_sched_tbl,	"get_txqos_sched_tbl" },

	{ e_qcsapi_wps_registrar_report_button_press, "registrar_report_button_press" },
	{ e_qcsapi_wps_registrar_report_button_press, "registrar_report_pbc" },
	{ e_qcsapi_wps_registrar_report_pin,	"registrar_report_pin" },
	{ e_qcsapi_wps_registrar_get_pp_devname, "registrar_get_pp_devname" },
	{ e_qcsapi_wps_registrar_set_pp_devname, "registrar_set_pp_devname" },
	{ e_qcsapi_wps_enrollee_report_button_press, "enrollee_report_button_press" },
	{ e_qcsapi_wps_enrollee_report_button_press, "enrollee_report_pbc" },
	{ e_qcsapi_wps_enrollee_report_pin,	"enrollee_report_pin" },
	{ e_qcsapi_wps_enrollee_generate_pin,	"enrollee_generate_pin" },
	{ e_qcsapi_wps_get_ap_pin,		"get_wps_ap_pin" },
	{ e_qcsapi_wps_set_ap_pin,		"set_wps_ap_pin" },
	{ e_qcsapi_wps_save_ap_pin,		"save_wps_ap_pin" },
	{ e_qcsapi_wps_enable_ap_pin,		"enable_wps_ap_pin" },
	{ e_qcsapi_wps_get_sta_pin,	"get_wps_sta_pin" },
	{ e_qcsapi_wps_get_state,		"get_wps_state" },
	{ e_qcsapi_wps_get_configured_state,	"get_wps_configured_state" },
	{ e_qcsapi_wps_set_configured_state,	"set_wps_configured_state" },
	{ e_qcsapi_wps_get_runtime_state,	"get_wps_runtime_state" },
	{ e_qcsapi_wps_get_allow_pbc_overlap_status,		"get_allow_pbc_overlap_status" },
	{ e_qcsapi_wps_allow_pbc_overlap,		"allow_pbc_overlap" },
	{ e_qcsapi_wps_get_param,		"get_wps_param" },
	{ e_qcsapi_wps_set_param,		"set_wps_param" },
	{ e_qcsapi_wps_set_access_control,	"set_wps_access_control" },
	{ e_qcsapi_wps_get_access_control,	"get_wps_access_control" },
	{ e_qcsapi_non_wps_set_pp_enable,	"set_non_wps_pp_enable" },
	{ e_qcsapi_non_wps_get_pp_enable,	"get_non_wps_pp_enable" },
	{ e_qcsapi_wps_cancel,			"wps_cancel" },
	{ e_qcsapi_wps_set_pbc_in_srcm,		"set_wps_pbc_in_srcm" },
	{ e_qcsapi_wps_get_pbc_in_srcm,		"get_wps_pbc_in_srcm" },
	{ e_qcsapi_wps_timeout,			"wps_set_timeout" },
	{ e_qcsapi_wps_on_hidden_ssid,		"wps_on_hidden_ssid" },
	{ e_qcsapi_wps_on_hidden_ssid_status,	"wps_on_hidden_ssid_status" },
	{ e_qcsapi_wps_upnp_enable,		"wps_upnp_enable" },
	{ e_qcsapi_wps_upnp_status,		"wps_upnp_status" },
	{ e_qcsapi_wps_registrar_set_dfl_pbc_bss, "registrar_set_default_pbc_bss"},
	{ e_qcsapi_wps_registrar_get_dfl_pbc_bss, "registrar_get_default_pbc_bss"},

	{ e_qcsapi_wifi_set_dwell_times,	"set_dwell_times" },
	{ e_qcsapi_wifi_get_dwell_times,	"get_dwell_times" },
	{ e_qcsapi_wifi_set_bgscan_dwell_times,	"set_bgscan_dwell_times" },
	{ e_qcsapi_wifi_get_bgscan_dwell_times,	"get_bgscan_dwell_times" },
	{ e_qcsapi_wifi_start_scan,		"start_scan" },
	{ e_qcsapi_wifi_cancel_scan,		"cancel_scan" },
	{ e_qcsapi_wifi_get_scan_status,	"get_scanstatus" },
	{ e_qcsapi_wifi_get_cac_status,		"get_cacstatus" },
	{ e_qcsapi_wifi_wait_scan_completes,	"wait_scan_completes" },
	{ e_qcsapi_wifi_set_scan_chk_inv,	"set_scan_chk_inv" },
	{ e_qcsapi_wifi_get_scan_chk_inv,	"get_scan_chk_inv" },

        { e_qcsapi_SSID_create_SSID,		"SSID_create_SSID" },
        { e_qcsapi_SSID_create_SSID,		"create_SSID" },
	{ e_qcsapi_SSID_remove_SSID,		"remove_SSID" },
        { e_qcsapi_SSID_verify_SSID,		"SSID_verify_SSID" },
        { e_qcsapi_SSID_verify_SSID,		"verify_SSID" },
        { e_qcsapi_SSID_rename_SSID,		"SSID_rename_SSID" },
        { e_qcsapi_SSID_rename_SSID,		"rename_SSID" },
        { e_qcsapi_SSID_get_SSID_list,		"get_SSID_list" },
        { e_qcsapi_SSID_get_protocol,		"get_SSID_proto" },
        { e_qcsapi_SSID_get_protocol,		"SSID_get_proto" },
        { e_qcsapi_SSID_set_protocol,		"set_SSID_proto" },
        { e_qcsapi_SSID_set_protocol,		"SSID_set_proto" },
        { e_qcsapi_SSID_get_encryption_modes,	"SSID_get_encryption_modes" },
        { e_qcsapi_SSID_set_encryption_modes,	"SSID_set_encryption_modes" },
        { e_qcsapi_SSID_get_group_encryption,	"SSID_get_group_encryption" },
        { e_qcsapi_SSID_set_group_encryption,	"SSID_set_group_encryption" },
        { e_qcsapi_SSID_get_authentication_mode, "SSID_get_authentication_mode" },
        { e_qcsapi_SSID_set_authentication_mode, "SSID_set_authentication_mode" },
        { e_qcsapi_SSID_get_pre_shared_key,	"SSID_get_pre_shared_key" },
        { e_qcsapi_SSID_set_pre_shared_key,	"SSID_set_pre_shared_key" },
        { e_qcsapi_SSID_get_key_passphrase,	"SSID_get_key_passphrase" },
        { e_qcsapi_SSID_get_key_passphrase,	"SSID_get_passphrase" },
        { e_qcsapi_SSID_set_key_passphrase,	"SSID_set_key_passphrase" },
        { e_qcsapi_SSID_set_key_passphrase,	"SSID_set_passphrase" },
        { e_qcsapi_SSID_get_pmf,		"SSID_get_pmf" },
        { e_qcsapi_SSID_set_pmf,		"SSID_set_pmf" },
        { e_qcsapi_SSID_get_wps_SSID,		"SSID_get_WPS_SSID" },
        { e_qcsapi_wifi_vlan_config,		"vlan_config" },
	{ e_qcsapi_wifi_show_vlan_config,	"show_vlan_config" },
        { e_qcsapi_enable_vlan_pass_through,	"enable_vlan_pass_through" },

        { e_qcsapi_wifi_start_cca,		"start_cca" },
        { e_qcsapi_wifi_disable_wps,		"disable_wps" },
        { e_qcsapi_wifi_get_results_AP_scan,	"get_results_AP_scan" },
	{ e_qcsapi_wifi_get_count_APs_scanned,	"get_count_APs_scanned" },
	{ e_qcsapi_wifi_get_properties_AP,	"get_properties_AP" },

	{e_qcsapi_wifi_get_time_associated_per_association, "get_time_associated" },

	{ e_qcsapi_wifi_wds_add_peer,		"wds_add_peer"},
	{ e_qcsapi_wifi_wds_remove_peer,	"wds_remove_peer"},
	{ e_qcsapi_wifi_wds_get_peer_address,	"wds_get_peer_address"},
	{ e_qcsapi_wifi_wds_set_psk,		"wds_set_psk"},
	{ e_qcsapi_wifi_wds_set_mode,		"wds_set_mode"},
	{ e_qcsapi_wifi_wds_get_mode,		"wds_get_mode"},

	{ e_qcsapi_wifi_qos_get_param,		"get_qos_param" },
	{ e_qcsapi_wifi_qos_set_param,		"set_qos_param" },

	{ e_qcsapi_wifi_get_wmm_ac_map,		"get_wmm_ac_map" },
	{ e_qcsapi_wifi_set_wmm_ac_map,		"set_wmm_ac_map" },

	{ e_qcsapi_wifi_get_dscp_8021p_map,	"get_dscp_8021p_map" },
	{ e_qcsapi_wifi_set_dscp_8021p_map,	"set_dscp_8021p_map" },
	{ e_qcsapi_wifi_get_dscp_ac_map,	"get_dscp_ac_map" },
	{ e_qcsapi_wifi_set_dscp_ac_map,	"set_dscp_ac_map" },

        { e_qcsapi_wifi_get_priority,		"get_priority" },
        { e_qcsapi_wifi_set_priority,		"set_priority" },
        { e_qcsapi_wifi_get_airfair,		"get_airfair" },
        { e_qcsapi_wifi_set_airfair,		"set_airfair" },

	{ e_qcsapi_config_get_parameter,	"get_config_param"},
	{ e_qcsapi_config_get_parameter,	"get_persistent_param"},
	{ e_qcsapi_config_update_parameter,	"update_config_param"},
	{ e_qcsapi_config_update_parameter,	"update_persistent_param"},
	{ e_qcsapi_bootcfg_get_parameter,	"get_bootcfg_param"},
	{ e_qcsapi_bootcfg_update_parameter,	"update_bootcfg_param"},
	{ e_qcsapi_bootcfg_commit,		"commit_bootcfg"},
	{ e_qcsapi_wifi_get_mcs_rate,		"get_mcs_rate" },
	{ e_qcsapi_wifi_set_mcs_rate,		"set_mcs_rate" },
	{ e_qcsapi_config_get_ssid_parameter,		"get_persistent_ssid_param"},
	{ e_qcsapi_config_update_ssid_parameter,	"update_persistent_ssid_param"},

	{ e_qcsapi_wifi_enable_scs,			"enable_scs" },
	{ e_qcsapi_wifi_scs_switch_channel,		"scs_switch_chan" },
	{ e_qcsapi_wifi_set_scs_verbose,		"set_scs_verbose" },
	{ e_qcsapi_wifi_get_scs_status,			"get_scs_status" },
	{ e_qcsapi_wifi_set_scs_smpl_enable,		"set_scs_smpl_enable" },
	{ e_qcsapi_wifi_set_scs_smpl_dwell_time,	"set_scs_smpl_dwell_time" },
	{ e_qcsapi_wifi_set_scs_smpl_intv,		"set_scs_smpl_intv" },
	{ e_qcsapi_wifi_set_scs_intf_detect_intv,	"set_scs_intf_detect_intv" },
	{ e_qcsapi_wifi_set_scs_thrshld,		"set_scs_thrshld" },
	{ e_qcsapi_wifi_set_scs_report_only,		"set_scs_report_only" },
	{ e_qcsapi_wifi_get_scs_report_stat,		"get_scs_report" },
	{ e_qcsapi_wifi_set_scs_cca_intf_smth_fctr,	"set_scs_cca_intf_smth_fctr" },
	{ e_qcsapi_wifi_set_scs_chan_mtrc_mrgn,		"set_scs_chan_mtrc_mrgn" },
	{ e_qcsapi_wifi_get_scs_dfs_reentry_request,	"get_scs_dfs_reentry_request" },
	{ e_qcsapi_wifi_get_scs_cca_intf,		"get_scs_cca_intf" },
	{ e_qcsapi_wifi_get_scs_param,			"get_scs_params" },
	{ e_qcsapi_wifi_set_scs_stats,			"set_scs_stats" },

	{ e_qcsapi_wifi_start_ocac,			"start_ocac" },
	{ e_qcsapi_wifi_stop_ocac,			"stop_ocac" },
	{ e_qcsapi_wifi_get_ocac_status,		"get_ocac_status" },
	{ e_qcsapi_wifi_set_ocac_threshold,		"set_ocac_thrshld" },
	{ e_qcsapi_wifi_set_ocac_dwell_time,		"set_ocac_dwell_time" },
	{ e_qcsapi_wifi_set_ocac_duration,		"set_ocac_duration" },
	{ e_qcsapi_wifi_set_ocac_cac_time,		"set_ocac_cac_time" },
	{ e_qcsapi_wifi_set_ocac_report_only,		"set_ocac_report_only" },

	{ e_qcsapi_wifi_start_dfs_s_radio,		"start_dfs_s_radio" },
	{ e_qcsapi_wifi_stop_dfs_s_radio,		"stop_dfs_s_radio" },
	{ e_qcsapi_wifi_get_dfs_s_radio_status,		"get_dfs_s_radio_status" },
	{ e_qcsapi_wifi_get_dfs_s_radio_availability,	"get_dfs_s_radio_availability" },
	{ e_qcsapi_wifi_set_dfs_s_radio_threshold,	"set_dfs_s_radio_thrshld" },
	{ e_qcsapi_wifi_set_dfs_s_radio_dwell_time,	"set_dfs_s_radio_dwell_time" },
	{ e_qcsapi_wifi_set_dfs_s_radio_duration,	"set_dfs_s_radio_duration" },
	{ e_qcsapi_wifi_set_dfs_s_radio_cac_time,	"set_dfs_s_radio_cac_time" },
	{ e_qcsapi_wifi_set_dfs_s_radio_report_only,	"set_dfs_s_radio_report_only" },
	{ e_qcsapi_wifi_set_dfs_s_radio_wea_duration,	"set_dfs_s_radio_wea_duration" },
	{ e_qcsapi_wifi_set_dfs_s_radio_wea_cac_time,	"set_dfs_s_radio_wea_cac_time" },

	{ e_qcsapi_wifi_set_vendor_fix,			"set_vendor_fix" },
	{ e_qcsapi_wifi_get_rts_threshold,		"get_rts_threshold" },
	{ e_qcsapi_wifi_set_rts_threshold,		"set_rts_threshold" },
	{ e_qcsapi_set_soc_macaddr,				"set_soc_macaddr" },

	{ e_qcsapi_get_interface_stats,			"get_interface_stats" },
	{ e_qcsapi_get_phy_stats,			"get_phy_stats" },
	{ e_qcsapi_wifi_set_ap_isolate,			"set_ap_isolate" },
	{ e_qcsapi_wifi_get_ap_isolate,			"get_ap_isolate" },
	{ e_qcsapi_power_save,				"pm" },
	{ e_qcsapi_qpm_level,				"qpm_level" },
	{ e_qcsapi_reset_all_stats,			"reset_all_stats" },
	{ e_qcsapi_eth_phy_power_off,			"eth_phy_power_off" },
	{ e_qcsapi_aspm_l1,				"set_aspm_l1"},
	{ e_qcsapi_l1,					"set_l1"},
	{ e_qcsapi_telnet_enable,			"enable_telnet" },
	{ e_qcsapi_restore_default_config,		"restore_default_config" },
	{ e_qcsapi_run_script,				"run_script" },
	{ e_qcsapi_qtm,					"qtm" },
	{ e_qcsapi_test_traffic,			"test_traffic" },
	{ e_qcsapi_get_temperature,			"get_temperature" },
	{ e_qcsapi_set_accept_oui_filter,		"set_accept_oui_filter" },
	{ e_qcsapi_get_accept_oui_filter,		"get_accept_oui_filter" },

	{ e_qcsapi_get_swfeat_list,			"get_swfeat_list" },

	{ e_qcsapi_wifi_set_vht,			"set_vht" },
	{ e_qcsapi_wifi_get_vht,			"get_vht" },

	{ e_qcsapi_calcmd_set_test_mode,		"set_test_mode" },
	{ e_qcsapi_calcmd_show_test_packet,		"show_test_packet" },
	{ e_qcsapi_calcmd_send_test_packet,		"send_test_packet" },
	{ e_qcsapi_calcmd_stop_test_packet,		"stop_test_packet" },
	{ e_qcsapi_calcmd_send_dc_cw_signal,		"send_dc_cw_signal" },
	{ e_qcsapi_calcmd_stop_dc_cw_signal,		"stop_dc_cw_signal" },
	{ e_qcsapi_calcmd_get_test_mode_antenna_sel,	"get_test_mode_antenna_sel" },
	{ e_qcsapi_calcmd_get_test_mode_mcs,		"get_test_mode_mcs" },
	{ e_qcsapi_calcmd_get_test_mode_bw,		"get_test_mode_bw" },
	{ e_qcsapi_calcmd_get_tx_power,			"get_test_mode_tx_power" },
	{ e_qcsapi_calcmd_set_tx_power,			"set_test_mode_tx_power" },
	{ e_qcsapi_calcmd_get_test_mode_rssi,		"get_test_mode_rssi" },
	{ e_qcsapi_calcmd_set_mac_filter,		"calcmd_set_mac_filter" },
	{ e_qcsapi_calcmd_get_antenna_count,		"get_test_mode_antenna_count" },
	{ e_qcsapi_calcmd_clear_counter,		"calcmd_clear_counter" },
	{ e_qcsapi_calcmd_get_info,                     "get_info" },
	{ e_qcsapi_wifi_disable_dfs_channels,		"disable_dfs_channels" },

	{ e_qcsapi_br_vlan_promisc,			"enable_vlan_promisc" },
	{ e_qcsapi_add_ipff,				"add_ipff" },
	{ e_qcsapi_del_ipff,				"del_ipff" },
	{ e_qcsapi_get_ipff,				"get_ipff" },
	{ e_qcsapi_get_carrier_id,			"get_carrier_id" },
	{ e_qcsapi_set_carrier_id,			"set_carrier_id" },
	{ e_qcsapi_get_spinor_jedecid,			"get_spinor_jedecid" },
	{ e_qcsapi_get_custom_value,			"get_custom_value" },


	{ e_qcsapi_wifi_enable_tdls,			"enable_tdls" },
	{ e_qcsapi_wifi_enable_tdls_over_qhop,		"enable_tdls_over_qhop" },
	{ e_qcsapi_wifi_get_tdls_status,		"get_tdls_status" },
	{ e_qcsapi_wifi_set_tdls_params,		"set_tdls_params" },
	{ e_qcsapi_wifi_get_tdls_params,		"get_tdls_params" },
	{ e_qcsapi_wifi_tdls_operate,			"tdls_operate" },

	{ e_qcsapi_wifi_get_mlme_stats_per_mac,				"get_mlme_stats_per_mac" },
	{ e_qcsapi_wifi_get_mlme_stats_per_association,		"get_mlme_stats_per_association" },
	{ e_qcsapi_wifi_get_mlme_stats_macs_list,			"get_mlme_stats_macs_list" },

	{ e_qcsapi_get_nss_cap,				"get_nss_cap"},
	{ e_qcsapi_set_nss_cap,				"set_nss_cap"},

	{ e_qcsapi_get_security_defer_mode,		"get_security_defer_mode"},
	{ e_qcsapi_set_security_defer_mode,		"set_security_defer_mode"},
	{ e_qcsapi_apply_security_config,		"apply_security_config"},

	{ e_qcsapi_get_board_parameter,			"get_board_parameter" },
	{ e_qcsapi_wifi_set_intra_bss_isolate,		"set_intra_bss_isolate" },
	{ e_qcsapi_wifi_get_intra_bss_isolate,		"get_intra_bss_isolate" },
	{ e_qcsapi_wifi_set_bss_isolate,		"set_bss_isolate" },
	{ e_qcsapi_wifi_get_bss_isolate,		"get_bss_isolate" },

	{ e_qcsapi_wowlan_host_state,			"wowlan_host_state" },
	{ e_qcsapi_wowlan_match_type,			"wowlan_match_type" },
	{ e_qcsapi_wowlan_L2_type,			"wowlan_L2_type" },
	{ e_qcsapi_wowlan_udp_port,			"wowlan_udp_port" },
	{ e_qcsapi_wowlan_pattern,			"wowlan_pattern" },
	{ e_qcsapi_wowlan_get_host_state,		"wowlan_get_host_state" },
	{ e_qcsapi_wowlan_get_match_type,		"wowlan_get_match_type" },
	{ e_qcsapi_wowlan_get_L2_type,			"wowlan_get_L2_type" },
	{ e_qcsapi_wowlan_get_udp_port,			"wowlan_get_udp_port" },
	{ e_qcsapi_wowlan_get_pattern,			"wowlan_get_pattern" },

	{ e_qcsapi_wifi_set_extender_params,		"set_extender_params" },
	{ e_qcsapi_wifi_get_extender_status,		"get_extender_status" },

	{ e_qcsapi_wifi_enable_bgscan,			"enable_bgscan" },
	{ e_qcsapi_wifi_get_bgscan_status,		"get_bgscan_status" },

	{ e_qcsapi_get_uboot_info,			"get_uboot_info"},
	{ e_qcsapi_wifi_get_disassoc_reason,		"disassoc_reason"},

	{ e_qcsapi_is_startprod_done,			"is_startprod_done"},

	{ e_qcsapi_get_bb_param,			"get_bb_param" },
	{ e_qcsapi_set_bb_param,			"set_bb_param" },
	{ e_qcsapi_wifi_get_tx_amsdu,			"get_tx_amsdu" },
	{ e_qcsapi_wifi_set_tx_amsdu,			"set_tx_amsdu" },

	{ e_qcsapi_wifi_set_scan_buf_max_size,		"set_scan_buf_max_size" },
	{ e_qcsapi_wifi_get_scan_buf_max_size,		"get_scan_buf_max_size" },
	{ e_qcsapi_wifi_set_scan_table_max_len,		"set_scan_table_max_len" },
	{ e_qcsapi_wifi_get_scan_table_max_len,		"get_scan_table_max_len" },

	{ e_qcsapi_wifi_set_enable_mu,			"set_enable_mu" },
	{ e_qcsapi_wifi_get_enable_mu,			"get_enable_mu" },
	{ e_qcsapi_wifi_set_mu_use_precode,		"set_mu_use_precode" },
	{ e_qcsapi_wifi_get_mu_use_precode,		"get_mu_use_precode" },
	{ e_qcsapi_wifi_set_mu_use_eq,			"set_mu_use_eq" },
	{ e_qcsapi_wifi_get_mu_use_eq,			"get_mu_use_eq" },
	{ e_qcsapi_wifi_get_mu_groups,			"get_mu_groups" },
	{ e_qcsapi_set_emac_switch,			"set_emac_switch" },
	{ e_qcsapi_get_emac_switch,			"get_emac_switch" },
	{ e_qcsapi_eth_dscp_map,			"eth_dscp_map" },

	{ e_qcsapi_send_file,				"send_file" },
	{ e_qcsapi_wifi_verify_repeater_mode,		"verify_repeater_mode" },
	{ e_qcsapi_wifi_set_ap_interface_name,		"set_ap_interface_name" },
	{ e_qcsapi_wifi_get_ap_interface_name,		"get_ap_interface_name" },

	{ e_qcsapi_set_optim_stats,			"set_optim_stats" },

	{ e_qcsapi_set_sys_time,			"set_sys_time" },
	{ e_qcsapi_get_sys_time,			"get_sys_time" },
	{ e_qcsapi_get_eth_info,			"get_eth_info" },
	{ e_qcsapi_wifi_block_bss,			"block_bss" },

	{ e_qcsapi_nosuch_api, NULL }
};

static const struct
{
	qcsapi_counter_type	 counter_type;
	const char		*counter_name;
} qcsapi_counter_name[] =
{
	{ qcsapi_total_bytes_sent,		"tx_bytes" },
	{ qcsapi_total_bytes_received,		"rx_bytes" },
	{ qcsapi_total_packets_sent,		"tx_packets" },
	{ qcsapi_total_packets_received,	"rx_packets" },
	{ qcsapi_discard_packets_sent,		"tx_discard" },
	{ qcsapi_discard_packets_received,	"rx_discard" },
	{ qcsapi_error_packets_sent,		"tx_errors" },
	{ qcsapi_error_packets_received,	"rx_errors" },
	{ qcsapi_vlan_frames_received,		"rx_vlan_pkts" },
	{ qcsapi_fragment_frames_received,	"rx_fragment_pkts" },
	{ qcsapi_nosuch_counter,	 NULL }
};

static const struct
{
	qcsapi_option_type	 option_type;
	const char		*option_name;
} qcsapi_option_name[] =
{
	{ qcsapi_channel_refresh,	"channel_refresh" },
	{ qcsapi_DFS,			"DFS" },
	{ qcsapi_wmm,			"WiFi_MultiMedia" },
	{ qcsapi_wmm,			"WMM" },
	{ qcsapi_beacon_advertise,	"beacon_advertise" },
	{ qcsapi_beacon_advertise,	"beacon" },
	{ qcsapi_wifi_radio,		"radio" },
	{ qcsapi_autorate_fallback,	"autorate_fallback" },
	{ qcsapi_autorate_fallback,	"autorate" },
	{ qcsapi_security,		"security" },
	{ qcsapi_SSID_broadcast,	"broadcast_SSID" },
	{ qcsapi_SSID_broadcast,	"SSID_broadcast" },
	{ qcsapi_short_GI,		"shortGI" },
	{ qcsapi_short_GI,		"short_GI" },
	{ qcsapi_802_11h,		"802_11h" },
	{ qcsapi_tpc_query,		"tpc_query" },
	{ qcsapi_dfs_fast_channel_switch, "dfs_fast_switch" },
	{ qcsapi_dfs_avoid_dfs_scan,	"avoid_dfs_scan" },
	{ qcsapi_uapsd,			"uapsd" },
	{ qcsapi_sta_dfs,		"sta_dfs" },
	{ qcsapi_specific_scan,		"specific_scan" },
	{ qcsapi_GI_probing,		"GI_probing" },
	{ qcsapi_GI_fixed,		"GI_fixed" },
	{ qcsapi_stbc,			"stbc" },
	{ qcsapi_beamforming,		"beamforming" },
	{ qcsapi_nosuch_option,		 NULL }
};

static const struct
{
	qcsapi_board_parameter_type	board_param;
	const char			*board_param_name;
} qcsapi_board_parameter_name[] =
{
	{ qcsapi_hw_revision,		"hw_revision" },
	{ qcsapi_hw_id,			"hw_id" },
	{ qcsapi_hw_desc,		"hw_desc" },
	{ qcsapi_rf_chipid,		"rf_chipid" },
	{ qcsapi_bond_opt,              "bond_opt" },
	{ qcsapi_vht,                   "vht_status" },
	{ qcsapi_bandwidth,             "bw_supported" },
	{ qcsapi_spatial_stream,        "spatial_stream" },
	{ qcsapi_interface_types,	"interface_types" },
	{ qcsapi_nosuch_parameter,      NULL }
};

static const struct
{
	qcsapi_rate_type	 rate_type;
	const char		*rate_name;
} qcsapi_rate_types_name[] =
{
	{ qcsapi_basic_rates,		"basic_rates" },
	{ qcsapi_basic_rates,		"basic" },
	{ qcsapi_operational_rates,	"operational_rates" },
	{ qcsapi_operational_rates,	"operational" },
	{ qcsapi_possible_rates,	"possible_rates" },
	{ qcsapi_possible_rates,	"possible" },
	{ qcsapi_nosuch_rate,		 NULL }
};

static const struct {
	qcsapi_mimo_type std_type;
	const char *std_name;
} qcsapi_wifi_std_name[] = {
	{qcsapi_mimo_ht, "ht"},
	{qcsapi_mimo_vht, "vht"},
	{qcsapi_nosuch_standard, NULL}
};

static const struct
{
	qcsapi_flash_partiton_type	 partition_type;
	const char			*partition_name;
} qcsapi_partition_name[] =
{
	{ qcsapi_live_image,		"live" },
	{ qcsapi_safety_image,		"safety" },
	{ qcsapi_nosuch_partition,	 NULL }
};

static const struct
{
	int		 qos_queue_type;
	const char	*qos_queue_name;
} qcsapi_qos_queue_table[] =
{
	{ WME_AC_BE,	"BE" },
	{ WME_AC_BK,	"BK" },
	{ WME_AC_VI,	"VI" },
	{ WME_AC_VO,	"VO" },
	{ WME_AC_BE,	"besteffort" },
	{ WME_AC_BK,	"background" },
	{ WME_AC_VI,	"video" },
	{ WME_AC_VO,	"voice" }
};

static const struct
{
	const char	*fix_name;
	unsigned	fix_idx;
} qcsapi_vendor_fix_table[] =
{
	{ "brcm_dhcp",	VENDOR_FIX_IDX_BRCM_DHCP},
	{ "brcm_igmp",	VENDOR_FIX_IDX_BRCM_IGMP},
};

static const struct
{
	int		 qos_param_type;
	const char	*qos_param_name;
} qcsapi_qos_param_table[] =
{
	{ IEEE80211_WMMPARAMS_CWMIN,	"cwmin" },
	{ IEEE80211_WMMPARAMS_CWMAX,	"cwmax" },
	{ IEEE80211_WMMPARAMS_AIFS,	"aifs" },
	{ IEEE80211_WMMPARAMS_TXOPLIMIT, "tx_op" },
	{ IEEE80211_WMMPARAMS_TXOPLIMIT, "txoplimit" },
	{ IEEE80211_WMMPARAMS_ACM, "acm" },
	{ IEEE80211_WMMPARAMS_NOACKPOLICY, "noackpolicy" }
};

static const struct {
	qcsapi_per_assoc_param	 pa_param;
	char			*pa_name;
} qcsapi_pa_param_table[] = {
	{QCSAPI_LINK_QUALITY,	"link_quality"},
	{QCSAPI_RSSI_DBM,	"rssi_dbm"},
	{QCSAPI_BANDWIDTH,	"bw"},
	{QCSAPI_SNR,		"snr"},
	{QCSAPI_TX_PHY_RATE,	"tx_phy_rate"},
	{QCSAPI_RX_PHY_RATE,	"rx_phy_rate"},
	{QCSAPI_STAD_CCA,	"stand_cca_req"},
	{QCSAPI_RSSI,		"rssi"},
	{QCSAPI_PHY_NOISE,	"hw_noise"},
	{QCSAPI_SOC_MAC_ADDR,	"soc_macaddr"},
	{QCSAPI_SOC_IP_ADDR,	"soc_ipaddr"},
	{QCSAPI_NODE_MEAS_BASIC,"basic"},
	{QCSAPI_NODE_MEAS_CCA,	"cca"},
	{QCSAPI_NODE_MEAS_RPI,	"rpi"},
	{QCSAPI_NODE_MEAS_CHAN_LOAD, "channel_load"},
	{QCSAPI_NODE_MEAS_NOISE_HIS, "noise_histogram"},
	{QCSAPI_NODE_MEAS_BEACON, "beacon"},
	{QCSAPI_NODE_MEAS_FRAME, "frame"},
	{QCSAPI_NODE_MEAS_TRAN_STREAM_CAT, "tran_stream_cat"},
	{QCSAPI_NODE_MEAS_MULTICAST_DIAG, "multicast_diag"},
	{QCSAPI_NODE_TPC_REP,	"tpc_report"},
	{QCSAPI_NODE_LINK_MEASURE, "link_measure"},
	{QCSAPI_NODE_NEIGHBOR_REP, "neighbor_report"},
};

char *support_script_table[] = {
		"stop_test_packet",
		"send_test_packet",
		"set_test_mode",
		"set_tx_pow",
		"send_cw_signal",
		"stop_cw_signal",
		"send_cw_signal_4chain",
		"show_test_packet",
		"transmit_file",
		"remote_command"
};

static const struct{
	qcsapi_system_status bit_id;
	char *description;
} qcsapi_sys_status_table[] =
{
	{qcsapi_sys_status_ethernet, "Ethernet interface"},
	{qcsapi_sys_status_pcie_ep, "PCIE EP driver"},
	{qcsapi_sys_status_pcie_rc, "PCIE RC driver"},
	{qcsapi_sys_status_wifi, "WiFi driver"},
	{qcsapi_sys_status_rpcd, "Rpcd server"},
	{qcsapi_sys_status_cal_mode, "Calstate mode"},
	{qcsapi_sys_status_completed, "System boot up completely"},
};

static const struct{
	const char		*name;
	enum qscs_cfg_param_e	index;
} qcsapi_scs_param_names_table[] =
{
	{"scs_smpl_dwell_time",			SCS_SMPL_DWELL_TIME},
	{"scs_sample_intv",			SCS_SAMPLE_INTV},
	{"scs_thrshld_smpl_pktnum",		SCS_THRSHLD_SMPL_PKTNUM},
	{"scs_thrshld_smpl_airtime",		SCS_THRSHLD_SMPL_AIRTIME},
	{"scs_thrshld_atten_inc",		SCS_THRSHLD_ATTEN_INC},
	{"scs_thrshld_dfs_reentry",		SCS_THRSHLD_DFS_REENTRY},
	{"scs_thrshld_dfs_reentry_minrate",	SCS_THRSHLD_DFS_REENTRY_MINRATE},
	{"scs_thrshld_dfs_reentry_intf",	SCS_THRSHLD_DFS_REENTRY_INTF},
	{"scs_thrshld_loaded",			SCS_THRSHLD_LOADED},
	{"scs_thrshld_aging_nor",		SCS_THRSHLD_AGING_NOR},
	{"scs_thrshld_aging_dfsreent",		SCS_THRSHLD_AGING_DFSREENT},
	{"scs_enable",				SCS_ENABLE},
	{"scs_debug_enable",			SCS_DEBUG_ENABLE},
	{"scs_smpl_enable",			SCS_SMPL_ENABLE},
	{"scs_report_only",			SCS_REPORT_ONLY},
	{"scs_cca_idle_thrshld",		SCS_CCA_IDLE_THRSHLD},
	{"scs_cca_intf_hi_thrshld",		SCS_CCA_INTF_HI_THRSHLD},
	{"scs_cca_intf_lo_thrshld",		SCS_CCA_INTF_LO_THRSHLD},
	{"scs_cca_intf_ratio",			SCS_CCA_INTF_RATIO},
	{"scs_cca_intf_dfs_margin",		SCS_CCA_INTF_DFS_MARGIN},
	{"scs_pmbl_err_thrshld",		SCS_PMBL_ERR_THRSHLD},
	{"scs_cca_sample_dur",			SCS_CCA_SAMPLE_DUR},
	{"scs_cca_intf_smth_fctr",		SCS_CCA_INTF_SMTH_NOXP},
	{"scs_cca_intf_smth_fctr",		SCS_CCA_INTF_SMTH_XPED},
	{"scs_rssi_smth_fctr",			SCS_RSSI_SMTH_UP},
	{"scs_rssi_smth_fctr",			SCS_RSSI_SMTH_DOWN},
	{"scs_chan_mtrc_mrgn",			SCS_CHAN_MTRC_MRGN},
	{"scs_atten_adjust",			SCS_ATTEN_ADJUST},
	{"scs_pmbl_err_smth_fctr",		SCS_PMBL_ERR_SMTH_FCTR},
	{"scs_pmbl_err_range",			SCS_PMBL_ERR_RANGE},
	{"scs_pmbl_err_mapped_intf_range",	SCS_PMBL_ERR_MAPPED_INTF_RANGE},
	{"scs_sp_wf",				SCS_SP_WF},
	{"scs_lp_wf",				SCS_LP_WF},
	{"scs_pmp_rpt_cca_smth_fctr",		SCS_PMP_RPT_CCA_SMTH_FCTR},
	{"scs_pmp_rx_time_smth_fctr",		SCS_PMP_RX_TIME_SMTH_FCTR},
	{"scs_pmp_tx_time_smth_fctr",		SCS_PMP_TX_TIME_SMTH_FCTR},
	{"scs_pmp_stats_stable_percent",	SCS_PMP_STATS_STABLE_PERCENT},
	{"scs_pmp_stats_stable_range",		SCS_PMP_STATS_STABLE_RANGE},
	{"scs_pmp_stats_clear_interval",	SCS_PMP_STATS_CLEAR_INTERVAL},
	{"scs_as_rx_time_smth_fctr",		SCS_AS_RX_TIME_SMTH_FCTR},
	{"scs_as_tx_time_smth_fctr",		SCS_AS_TX_TIME_SMTH_FCTR},
	{"scs_cca_idle_smth_fctr",		SCS_CCA_IDLE_SMTH_FCTR},
	{"scs_tx_time_compensation",		SCS_TX_TIME_COMPENSTATION_START},
	{"scs_rx_time_compensation",		SCS_RX_TIME_COMPENSTATION_START},
	{"scs_tdls_time_compensation",		SCS_TDLS_TIME_COMPENSTATION_START}
};

static const struct
{
	qcsapi_extender_type param_type;
	const char *param_name;
} qcsapi_extender_param_table[] =
{
	{qcsapi_extender_role,	"role"},
	{qcsapi_extender_mbs_best_rssi,	"mbs_best_rssi"},
	{qcsapi_extender_rbs_best_rssi,	"rbs_best_rssi"},
	{qcsapi_extender_mbs_wgt,	"mbs_wgt"},
	{qcsapi_extender_rbs_wgt,	"rbs_wgt"},
	{qcsapi_extender_roaming,	"roaming"},
	{qcsapi_extender_bgscan_interval,	"bgscan_interval"},
	{qcsapi_extender_verbose,	"verbose"},
	{qcsapi_extender_nosuch_param,	NULL},
};

static const struct
{
	qcsapi_eth_info_result result_type;
	const char *result_label;
	const char *result_bit_set;
	const char *result_bit_unset;
} qcsapi_eth_info_result_table[] =
{
	{qcsapi_eth_info_connected,	"Connected",		"yes",		"no"},
	{qcsapi_eth_info_speed_unknown,	"Speed",		"unknown",	NULL},
	{qcsapi_eth_info_speed_10M,	"Speed",		"10Mb/s",	NULL},
	{qcsapi_eth_info_speed_100M,	"Speed",		"100Mb/s",	NULL},
	{qcsapi_eth_info_speed_1000M,	"Speed",		"1000Mb/s",	NULL},
	{qcsapi_eth_info_speed_10000M,	"Speed",		"10000Mb/s",	NULL},
	{qcsapi_eth_info_duplex_full,	"Duplex",		"full",		"half"},
	{qcsapi_eth_info_autoneg_on,	"Auto-negotiation",	NULL,		"disabled"},
	{qcsapi_eth_info_autoneg_success,"Auto-negotiation",	"completed",	"failed"},
};

static const struct
{
	qcsapi_eth_info_type type;
	qcsapi_eth_info_type_mask mask;
} qcsapi_eth_info_type_mask_table[] =
{
	{qcsapi_eth_info_link,		qcsapi_eth_info_link_mask},
	{qcsapi_eth_info_speed,		qcsapi_eth_info_speed_mask},
	{qcsapi_eth_info_duplex,	qcsapi_eth_info_duplex_mask},
	{qcsapi_eth_info_autoneg,	qcsapi_eth_info_autoneg_mask},
	{qcsapi_eth_info_all,		qcsapi_eth_info_all_mask},
};

static const struct
{
        int	reason_code;
        const char              *reason_string;
} qcsapi_disassoc_reason_list[] =
{
	{  0, "No disassoc reason reported" },
	{  1, "Unspecified reason" },
	{  2, "Previous authentication no longer valid" },
	{  3, "Deauthenticated because sending STA is leaving (or has left) IBSS or ESS" },
	{  4, "Disassociated due to inactivity" },
	{  5, "Disassociated because AP is unable to handle all currently associated STAs" },
	{  6, "Class 2 frame received from nonauthenticated STA" },
	{  7, "Class 3 frame received from nonassociated STA" },
	{  8, "Disassociated because sending STA is leaving (or has left) BSS" },
	{  9, "STA requesting (re)association is not authenticated with responding STA" },
	{ 10, "Disassociated because the information in the Power Capability element is unacceptable" },
	{ 11, "Disassociated because the information in the Supported Channels element is unacceptable" },
	{ 12, "Reserved" },
	{ 13, "Invalid information element, i.e., an information element defined in this standard for which the content does not meet the specifications in Clause 7" },
	{ 14, "Message integrity code (MIC) failure" },
	{ 15, "4-Way Handshake timeout" },
	{ 16, "Group Key Handshake timeout" },
	{ 17, "Information element in 4-Way Handshake different from (Re)Association Request/Probe Response/Beacon frame" },
	{ 18, "Invalid group cipher" },
	{ 19, "Invalid pairwise cipher" },
	{ 20, "Invalid AKMP" },
	{ 21, "Unsupported RSN information element version" },
	{ 22, "Invalid RSN information element capabilities" },
	{ 23, "IEEE 802.1X authentication failed" },
	{ 24, "Cipher suite rejected because of the security policy" },
	{ 25, "TDLS direct-link teardown due to TDLS peer STA unreachable via the TDLS direct link" },
	{ 26, "TDLS direct-link teardown for unspecified reason" },
	{ 27, "Disassociated because session terminated by SSP request" },
	{ 28, "Disassociated because of lack of SSP roaming agreement" },
	{ 29, "Requested service rejected because of SSP cipher suite or AKM requirement " },
	{ 30, "Requested service not authorized in this location" },
	{ 31, "TS deleted because QoS AP lacks sufficient bandwidth for this QoS STA due to a change in BSS service characteristics or operational mode" },
	{ 32, "Disassociated for unspecified, QoS-related reason" },
	{ 33, "Disassociated because QoS AP lacks sufficient bandwidth for this QoS STA" },
	{ 34, "Disassociated because excessive number of frames need to be acknowledged, but are not acknowledged due to AP transmissions and/or poor channel conditions" },
	{ 35, "Disassociated because STA is transmitting outside the limits of its TXOPs" },
	{ 36, "Requested from peer STA as the STA is leaving the BSS (or resetting)" },
	{ 37, "Requested from peer STA as it does not want to use the mechanism" },
	{ 38, "Requested from peer STA as the STA received frames using the mechanism for which a setup is required" },
	{ 39, "Requested from peer STA due to timeout" },
	{ 45, "Peer STA does not support the requested cipher suite" },
	{ 46, "Disassociated because authorized access limit reached" },
	{ 47, "Disassociated due to external service requirements" },
	{ 48, "Invalid FT Action frame count" },
	{ 49, "Invalid pairwise master key identifier (PMKI)" },
	{ 50, "Invalid MDE" },
	{ 51, "Invalid FTE" },
	{ 52, "SME cancels the mesh peering instance with the reason other than reaching the maximum number of peer mesh STAs" },
	{ 53, "The mesh STA has reached the supported maximum number of peer mesh STAs" },
	{ 54, "The received information violates the Mesh Configuration policy configured in the mesh STA profile" },
	{ 55, "The mesh STA has received a Mesh Peering Close message requesting to close the mesh peering" },
	{ 56, "The mesh STA has re-sent dot11MeshMaxRetries Mesh Peering Open messages, without receiving a Mesh Peering Confirm message" },
	{ 57, "The confirmTimer for the mesh peering instance times out" },
	{ 58, "The mesh STA fails to unwrap the GTK or the values in the wrapped contents do not match" },
	{ 59, "The mesh STA receives inconsistent information about the mesh parameters between Mesh Peering Management frames" },
	{ 60, "The mesh STA fails the authenticated mesh peering exchange because due to failure in selecting either the pairwise ciphersuite or group ciphersuite" },
	{ 61, "The mesh STA does not have proxy information for this external destination" },
	{ 62, "The mesh STA does not have forwarding information for this destination" },
	{ 63, "The mesh STA determines that the link to the next hop of an active path in its forwarding information is no longer usable" },
	{ 64, "The Deauthentication frame was sent because the MAC address of the STA already exists in the mesh BSS. See 11.3.3 (Additional mechanisms for an AP collocated with a mesh STA)" },
	{ 65, "The mesh STA performs channel switch to meet regulatory requirements" },
	{ 66, "The mesh STA performs channel switch with unspecified reason" },
};

static const struct
{
	qcsapi_tdls_type param_type;
	const char *param_name;
} qcsapi_tdls_param_table[] =
{
	{qcsapi_tdls_over_qhop_enabled,	"tdls_over_qhop"},
	{qcsapi_tdls_indication_window,	"indication_window"},
	{qcsapi_tdls_chan_switch_mode, "chan_switch_mode"},
	{qcsapi_tdls_chan_switch_off_chan, "chan_switch_off_chan"},
	{qcsapi_tdls_chan_switch_off_chan_bw, "chan_switch_off_chan_bw"},
	{qcsapi_tdls_link_timeout_time,	"link_timeout_time"},
	{qcsapi_tdls_verbose, "verbose"},
	{qcsapi_tdls_discovery_interval, "disc_interval"},
	{qcsapi_tdls_node_life_cycle, "node_life_cycle"},
	{qcsapi_tdls_mode, "mode"},
	{qcsapi_tdls_min_rssi, "min_valid_rssi"},
	{qcsapi_tdls_link_weight, "link_weight"},
	{qcsapi_tdls_rate_weight, "phy_rate_weight"},
	{qcsapi_tdls_training_pkt_cnt, "training_pkt_cnt"},
	{qcsapi_tdls_switch_ints, "link_switch_ints"},
	{qcsapi_tdls_path_select_pps_thrshld, "path_sel_pps_thrshld"},
	{qcsapi_tdls_path_select_rate_thrshld, "path_sel_rate_thrshld"},
};

static const struct
{
	qcsapi_tdls_oper oper;
	const char *oper_name;
} qcsapi_tdls_oper_table[] =
{
	{qcsapi_tdls_oper_discover, "discover"},
	{qcsapi_tdls_oper_setup, "setup"},
	{qcsapi_tdls_oper_teardown, "teardown"},
	{qcsapi_tdls_oper_switch_chan, "switch_chan"},
};

static const char *qcsapi_auth_algo_list[] = {
	"OPEN",
	"SHARED",
};

static const char *qcsapi_auth_keyproto_list[] = {
	"NONE",
	"WPA",
	"WPA2",
};

static const char *qcsapi_auth_keymgmt_list[] = {
	"NONE",
	"WPA-EAP",
	"WPA-PSK",
	"WEP",
};

static const char *qcsapi_auth_cipher_list[] = {
	"WEP",
	"TKIP",
	"OCB",
	"CCMP",
	"CMAC",
	"CKIP",
};

static const char *qcsapi_wifi_modes_strings[] = WLAN_WIFI_MODES_STRINGS;


static const char*
qcsapi_csw_reason_list[] = {
	[IEEE80211_CSW_REASON_UNKNOWN] = "UNKNOWN",
	[IEEE80211_CSW_REASON_SCS] = "SCS",
	[IEEE80211_CSW_REASON_DFS] = "DFS",
	[IEEE80211_CSW_REASON_MANUAL] = "MANUAL",
	[IEEE80211_CSW_REASON_CONFIG] = "CONFIG",
	[IEEE80211_CSW_REASON_SCAN] = "SCAN",
	[IEEE80211_CSW_REASON_OCAC] = "OCAC",
	[IEEE80211_CSW_REASON_CSA] = "CSA",
	[IEEE80211_CSW_REASON_TDLS_CS] = "TDLS",
};

static int		verbose_flag = 0;
static unsigned int	call_count = 1;
static unsigned int	delay_time = 0;

static unsigned int	internal_flags = 0;

static unsigned int	call_qcsapi_init_count = 1;

enum
{
	m_force_NULL_address = 0x01
};

char *qcsapi_80211u_params[] = {
		"internet",
		"access_network_type",
		"network_auth_type",
		"hessid",
		"domain_name",
		"ipaddr_type_availability",
		"anqp_3gpp_cell_net",
		"venue_group",
		"venue_type",
		"gas_comeback_delay",
		NULL
};

char *qcsapi_hs20_params[] = {
		"hs20_wan_metrics",
		"disable_dgaf",
		"hs20_operating_class",
};

/* returns 1 if successful; 0 if failure */

static int
name_to_entry_point_enum( char *lookup_name, qcsapi_entry_point *p_entry_point )
{
	int		retval = 1;
	int		found_entry = 0, proposed_enum = (int) e_qcsapi_nosuch_api;
	unsigned int	iter;
  /*
   * Silently skip over "qscapi_" ...
   */
	if (strncasecmp( lookup_name, "qscapi_", 7 ) == 0)
	  lookup_name += 7;

	for (iter = 0; qcsapi_entry_name[ iter ].api_name != NULL && found_entry == 0; iter++)
	{
		if (strcasecmp( qcsapi_entry_name[ iter ].api_name, lookup_name ) == 0)
		{
			found_entry = 1;
			*p_entry_point = qcsapi_entry_name[ iter ].e_entry_point;
		}
	}

	if (found_entry == 0)
	{
		*p_entry_point = proposed_enum;
		retval = 0;
	}

	return( retval );
}

/* Guaranteed to return a valid string address */

static const char *
entry_point_enum_to_name( qcsapi_entry_point e_qcsapi_entry_point )
{
	const char	*retaddr = "No such QCSAPI";
	int		 found_entry = 0;
	unsigned int	 iter;

	for (iter = 0; qcsapi_entry_name[ iter ].api_name != NULL && found_entry == 0; iter++)
	{
		if (qcsapi_entry_name[ iter ].e_entry_point == e_qcsapi_entry_point)
		{
			found_entry = 1;
			retaddr = qcsapi_entry_name[ iter ].api_name;
		}
	}

	return( retaddr );
}

static void
list_entry_point_names(qcsapi_output *print)
{
	unsigned int	 iter;

	print_out( print, "API entry point names (more than one name can refer to the same entry point):\n" );

	for (iter = 0; qcsapi_entry_name[ iter ].api_name != NULL; iter++)
	{
		print_out( print, "\t%s\n", qcsapi_entry_name[ iter ].api_name );
	}
}

static void
grep_entry_point_names(qcsapi_output *print, const char *reg)
{
	unsigned int	 iter;

	print_out( print, "API entry point names (more than one name can refer to the same entry point):\n" );

	for (iter = 0; qcsapi_entry_name[ iter ].api_name != NULL; iter++)
	{
		if ( strstr(qcsapi_entry_name[ iter ].api_name, reg) )
			print_out( print, "\t%s\n", qcsapi_entry_name[ iter ].api_name );
	}
}

/* returns 1 if successful; 0 if failure */

static int
name_to_counter_enum( char *lookup_name, qcsapi_counter_type *p_counter_type )
{
	int		retval = 0;
	int		found_entry = 0;
	unsigned int	iter;

	for (iter = 0; qcsapi_counter_name[ iter ].counter_name != NULL && found_entry == 0; iter++)
	{
		if (strcasecmp( qcsapi_counter_name[ iter ].counter_name, lookup_name ) == 0)
		{
			found_entry = 1;
			*p_counter_type = qcsapi_counter_name[ iter ].counter_type;
		}
	}

	if (found_entry)
	  retval = 1;

	return( retval );
}

/* Guaranteed to return a valid string address */

static const char *
counter_enum_to_name( qcsapi_counter_type the_counter_type )
{
	const char	*retaddr = "No such QCSAPI counter";
	int		 found_entry = 0;
	unsigned int	 iter;

	for (iter = 0; qcsapi_counter_name[ iter ].counter_name != NULL && found_entry == 0; iter++)
	{
		if (qcsapi_counter_name[ iter ].counter_type  == the_counter_type)
		{
			found_entry = 1;
			retaddr = qcsapi_counter_name[ iter ].counter_name;
		}
	}

	return( retaddr );
}

static void
list_counter_names(qcsapi_output *print)
{
	unsigned int	 iter;

	print_out( print, "API counters:\n" );

	for (iter = 0; qcsapi_counter_name[ iter ].counter_name != NULL; iter++) {
		print_out( print, "\t%s\n", qcsapi_counter_name[ iter ].counter_name );
	}
}

static void
list_per_node_param_names(qcsapi_output *print)
{
	unsigned int	 iter;

	print_out(print, "Per-node parameters:\n");

	for (iter = 0; iter < ARRAY_SIZE(qcsapi_pa_param_table); iter++) {
		print_out(print, "\t%s\n", qcsapi_pa_param_table[ iter ].pa_name);
	}
}

/* returns 1 if successful; 0 if failure */

static int
name_to_option_enum( char *lookup_name, qcsapi_option_type *p_option )
{
	int		retval = 0;
	int		found_entry = 0;
	unsigned int	iter;

	for (iter = 0; qcsapi_option_name[ iter ].option_name != NULL && found_entry == 0; iter++)
	{
		if (strcasecmp( qcsapi_option_name[ iter ].option_name, lookup_name ) == 0)
		{
			found_entry = 1;
			*p_option = qcsapi_option_name[ iter ].option_type;
		}
	}

	if (found_entry)
	  retval = 1;

	return( retval );
}

/* Guaranteed to return a valid string address */

static const char *
option_enum_to_name( qcsapi_option_type the_option_type )
{
	const char	*retaddr = "No such QCSAPI option";
	int		 found_entry = 0;
	unsigned int	 iter;

	for (iter = 0; qcsapi_option_name[ iter ].option_name != NULL && found_entry == 0; iter++)
	{
		if (qcsapi_option_name[ iter ].option_type == the_option_type)
		{
			found_entry = 1;
			retaddr = qcsapi_option_name[ iter ].option_name;
		}
	}

	return( retaddr );
}

static void
list_option_names(qcsapi_output *print)
{
	unsigned int	 iter;

	print_out( print, "API options (more than one name can refer to the same option):\n" );

	for (iter = 0; qcsapi_option_name[ iter ].option_name != NULL; iter++)
	{
		print_out( print, "\t%s\n", qcsapi_option_name[ iter ].option_name );
	}
}

/* returns 1 if successful; 0 if failure */
static int
name_to_board_parameter_enum( char *lookup_name, qcsapi_board_parameter_type *p_boardparam )
{
	int		retval = 0;
	int		found_entry = 0;
	unsigned int	iter;

	for (iter = 0;
	     qcsapi_board_parameter_name[ iter ].board_param_name != NULL && (found_entry == 0);
	     iter++)
	{
		if (strcasecmp( qcsapi_board_parameter_name[ iter ].board_param_name, lookup_name ) == 0)
		{
			found_entry = 1;
			*p_boardparam = qcsapi_board_parameter_name[ iter ].board_param;
		}
	}

	if (found_entry)
	  retval = 1;

	return( retval );
}

static const char *
board_paramter_enum_to_name( qcsapi_board_parameter_type the_board_param )
{
	const char	*retaddr = "No such QCSAPI option";
	int		 found_entry = 0;
	unsigned int	 iter;

	for (iter = 0;
	     qcsapi_board_parameter_name[ iter ].board_param_name != NULL && found_entry == 0;
	     iter++)
	{
		if (qcsapi_board_parameter_name[ iter ].board_param == the_board_param)
		{
			found_entry = 1;
			retaddr = qcsapi_board_parameter_name[ iter ].board_param_name;
		}
	}

	return( retaddr );
}

static void
list_board_parameter_names( qcsapi_output *print )
{
	unsigned int	 iter;

	for (iter = 0; qcsapi_board_parameter_name[ iter ].board_param_name != NULL; iter++)
	{
		print_out( print, "\t%s\n", qcsapi_board_parameter_name[ iter ].board_param_name );
	}
}

/* returns 1 if successful; 0 if failure */

static int
name_to_rates_enum( char *lookup_name, qcsapi_rate_type *p_rates )
{
	int		retval = 0;
	int		found_entry = 0;
	unsigned int	iter;

	for (iter = 0; qcsapi_rate_types_name[ iter ].rate_name != NULL && found_entry == 0; iter++)
	{
		if (strcasecmp( qcsapi_rate_types_name[ iter ].rate_name, lookup_name ) == 0)
		{
			found_entry = 1;
			*p_rates = qcsapi_rate_types_name[ iter ].rate_type;
		}
	}

	if (found_entry)
	  retval = 1;

	return( retval );
}

/* Guaranteed to return a valid string address */

static const char *
rates_enum_to_name( qcsapi_rate_type the_option_type )
{
	const char	*retaddr = "No such type of rates";
	int		 found_entry = 0;
	unsigned int	 iter;

	for (iter = 0; qcsapi_rate_types_name[ iter ].rate_name != NULL && found_entry == 0; iter++)
	{
		if (qcsapi_rate_types_name[ iter ].rate_type == the_option_type)
		{
			found_entry = 1;
			retaddr = qcsapi_rate_types_name[ iter ].rate_name;
		}
	}

	return( retaddr );
}

static int
name_to_wifi_std_enum(const char *lookup_name, qcsapi_mimo_type *p_modulation)
{
	unsigned int iter = 0;
	unsigned int found_entry = 0;

	while (qcsapi_wifi_std_name[iter].std_name && !found_entry) {
		if (!strcasecmp(qcsapi_wifi_std_name[iter].std_name, lookup_name)) {
			*p_modulation = qcsapi_wifi_std_name[iter].std_type;
			found_entry = 1;
		}
		++iter;
	}

	return found_entry;
}

static const char*
wifi_std_enum_to_name(const qcsapi_mimo_type lookup_type)
{
	unsigned int iter = 0;
	const char *ret_name = "No such type of standard";

	while (qcsapi_wifi_std_name[iter].std_name) {
		if (qcsapi_wifi_std_name[iter].std_type == lookup_type) {
			ret_name = qcsapi_wifi_std_name[iter].std_name;
			break;
		}
		++iter;
	}

	return ret_name;
}


/* returns 1 if successful; 0 if failure */
static int
name_to_partition_type( char *lookup_name, qcsapi_flash_partiton_type *p_partition_type )
{
	int		retval = 0;
	unsigned int	iter;

	for (iter = 0; qcsapi_partition_name[ iter ].partition_name != NULL && retval == 0; iter++)
	{
		if (strcasecmp( qcsapi_partition_name[ iter ].partition_name, lookup_name ) == 0)
		{
			retval = 1;
			*p_partition_type = qcsapi_partition_name[ iter ].partition_type;
		}
	}

	return( retval );
}

static int name_to_qos_queue_type(char *lookup_name, int *p_qos_queue_type)
{
	int		retval = 0;
	unsigned int	iter;

	for (iter = 0; iter < ARRAY_SIZE(qcsapi_qos_queue_table); iter++) {
		if (strcasecmp(qcsapi_qos_queue_table[iter].qos_queue_name, lookup_name) == 0) {
			*p_qos_queue_type = qcsapi_qos_queue_table[iter].qos_queue_type;
			retval = 1;
			break;
		}
	}

	return retval;
}

static int name_to_qos_param_type(char *lookup_name, int *p_qos_param_type)
{
	int		retval = 0;
	unsigned int	iter;

	for (iter = 0; iter < ARRAY_SIZE(qcsapi_qos_param_table); iter++) {
		if (strcasecmp(qcsapi_qos_param_table[iter].qos_param_name, lookup_name) == 0) {
			*p_qos_param_type = qcsapi_qos_param_table[iter].qos_param_type;
			retval = 1;
			break;
		}
	}

	return retval;
}

static int name_to_vendor_fix_idx(char *lookup_name, int *p_vendor_fix_idx)
{
	int		retval = 0;
	unsigned int	iter;

	for (iter = 0; iter < ARRAY_SIZE(qcsapi_vendor_fix_table); iter++) {
		if (strcasecmp(qcsapi_vendor_fix_table[iter].fix_name, lookup_name) == 0) {
			*p_vendor_fix_idx = qcsapi_vendor_fix_table[iter].fix_idx;
			retval = 1;
			break;
		}
	}

	return retval;
}

static int name_to_per_assoc_parameter(const char *param_name,
	 			       qcsapi_per_assoc_param *p_per_assoc_param)
{
	unsigned int	iter;

	for (iter = 0; iter < ARRAY_SIZE(qcsapi_pa_param_table); iter++) {
		if (strcasecmp(qcsapi_pa_param_table[iter].pa_name, param_name) == 0) {
			*p_per_assoc_param = qcsapi_pa_param_table[iter].pa_param;
			return 1;
		}
	}

	return 0;
}

static int parse_local_remote_flag(qcsapi_output *print, const char *local_remote_str, int *p_local_remote_flag)
{
	int	local_remote_flag = QCSAPI_LOCAL_NODE;

	if (isdigit(local_remote_str[0])) {
		local_remote_flag = atoi(local_remote_str);
	} else if (strcasecmp(local_remote_str, "remote") == 0) {
		local_remote_flag = QCSAPI_REMOTE_NODE;
	} else if (strcasecmp(local_remote_str, "local") == 0) {
		local_remote_flag = QCSAPI_LOCAL_NODE;
	} else {
		print_err(print, "Invalid value %s for local/remote flag\n", local_remote_str);
		return -1;
	}

	*p_local_remote_flag = local_remote_flag;
	return 0;
}

static int name_to_tdls_param_enum(char *lookup_name, qcsapi_tdls_type *p_tdls_type)
{
	int retval = 0;
	int found_entry = 0;
	unsigned int iter;

	for (iter = 0; qcsapi_tdls_param_table[iter].param_name != NULL && found_entry == 0; iter++) {
		if (strcasecmp(qcsapi_tdls_param_table[iter].param_name, lookup_name) == 0) {
			found_entry = 1;
			*p_tdls_type = qcsapi_tdls_param_table[iter].param_type;
			break;
		}
	}

	if (found_entry)
		retval = 1;

	return (retval);
}

static int name_to_tdls_oper_enum(char *lookup_name, qcsapi_tdls_oper *p_tdls_oper)
{
	int retval = 0;
	int found_entry = 0;
	unsigned int iter;
	unsigned int table_size = 0;

	table_size = TABLE_SIZE(qcsapi_tdls_oper_table);

	for (iter = 0; iter < table_size; iter++) {
		if (strcasecmp(qcsapi_tdls_oper_table[iter].oper_name, lookup_name) == 0) {
			found_entry = 1;
			*p_tdls_oper = qcsapi_tdls_oper_table[iter].oper;
			break;
		}
	}

	if (found_entry)
		retval = 1;

	return (retval);
}

static int name_to_extender_param_enum(char *lookup_name, qcsapi_extender_type *p_extender_type)
{
	unsigned int iter;

	for (iter = 0; qcsapi_extender_param_table[iter].param_name != NULL; iter++) {
		if (strcasecmp(qcsapi_extender_param_table[iter].param_name,
				lookup_name) == 0) {
			*p_extender_type = qcsapi_extender_param_table[iter].param_type;
			return 1;
		}
	}

	return 0;
}

static int
parse_generic_parameter_name(qcsapi_output *print, char *generic_parameter_name, qcsapi_generic_parameter *p_generic_parameter )
{
	int			retval = 1;
	qcsapi_unsigned_int	tmpuval = 0;
	qcsapi_tdls_type *p_tdls_type = NULL;
	qcsapi_tdls_oper *p_tdls_oper = NULL;
	qcsapi_extender_type *p_extender_type = NULL;

	switch( p_generic_parameter->generic_parameter_type )
	{
	  case e_qcsapi_option:
		retval = name_to_option_enum( generic_parameter_name, &(p_generic_parameter->parameter_type.option) );
		if (retval == 0)
		  print_err( print, "Invalid QCSAPI option %s\n", generic_parameter_name );
		break;

	  case e_qcsapi_counter:
		retval = name_to_counter_enum( generic_parameter_name, &(p_generic_parameter->parameter_type.counter) );
		if (retval == 0)
		  print_err( print, "Invalid QCSAPI counter %s\n", generic_parameter_name );
		break;

	  case e_qcsapi_rates:
		retval = name_to_rates_enum( generic_parameter_name, &(p_generic_parameter->parameter_type.typeof_rates) );
		if (retval == 0)
		  print_err( print, "Invalid QCSAPI type of rates %s\n", generic_parameter_name );
		break;

	  case e_qcsapi_modulation:
		retval = name_to_wifi_std_enum(generic_parameter_name,
						&p_generic_parameter->parameter_type.modulation);
		if (!retval)
			print_err(print, "Invalid QCSAPI MIMO modulation %s\n",
					generic_parameter_name);
		break;

	  case e_qcsapi_index:
	  case e_qcsapi_LED:
		if (!isdigit(generic_parameter_name[0])) {
			if (e_qcsapi_option == e_qcsapi_LED) {
				print_err(print, "LED must be a numeric value\n");
			} else {
				print_err(print, "Node index must be a numeric value\n");
			}
			retval = 0;
		}

		tmpuval = (qcsapi_unsigned_int) atoi( generic_parameter_name );
		if (p_generic_parameter->generic_parameter_type == e_qcsapi_LED && tmpuval > QCSAPI_MAX_LED)
		{
			print_err( print, "Invalid QSCAPI LED %u\n", tmpuval );
			retval = 0;
		}
		else
		  p_generic_parameter->index = (qcsapi_unsigned_int) atoi( generic_parameter_name );
		break;

	  case e_qcsapi_select_SSID:
	  case e_qcsapi_SSID_index:
	  /*
	   * APIs with generic parameter type of e_qcsapi_SSID_index expect both an SSID and an index.
	   * Get the SSID now.  Get the index in the individual call_qcsapi routines.
	   */
		strncpy(
			&(p_generic_parameter->parameter_type.the_SSID[ 0 ]),
			 generic_parameter_name,
			 sizeof( p_generic_parameter->parameter_type.the_SSID ) - 1
		);
		p_generic_parameter->parameter_type.the_SSID[ sizeof( p_generic_parameter->parameter_type.the_SSID ) - 1 ] = '\0';
		break;

	  case e_qcsapi_file_path_config:
		if (strcasecmp( "security", generic_parameter_name ) != 0)
		{
			print_err( print, "Invalid QCSAPI file path configuration %s\n", generic_parameter_name );
			retval = 0;
		}
		else
		  p_generic_parameter->index = (qcsapi_unsigned_int) qcsapi_security_configuration_path;
		break;

	  case e_qcsapi_tdls_params:
		p_tdls_type = &(p_generic_parameter->parameter_type.type_of_tdls);
		retval = name_to_tdls_param_enum(generic_parameter_name, p_tdls_type);
		if (retval == 0)
			print_err(print, "Invalid QCSAPI tdls param %s\n", generic_parameter_name);
		break;
	  case e_qcsapi_tdls_oper:
		p_tdls_oper = &(p_generic_parameter->parameter_type.tdls_oper);
		retval = name_to_tdls_oper_enum(generic_parameter_name, p_tdls_oper);
		if (retval == 0)
			print_err(print, "Invalid QCSAPI tdls oper %s\n", generic_parameter_name);
		break;


	  case e_qcsapi_board_parameter:
		retval = name_to_board_parameter_enum(generic_parameter_name,
				&(p_generic_parameter->parameter_type.board_param) );
		if (retval == 0)
		  print_err( print, "Invalid QCSAPI option %s\n", generic_parameter_name );
		break;

	  case e_qcsapi_extender_params:
		p_extender_type = &(p_generic_parameter->parameter_type.type_of_extender);
		retval = name_to_extender_param_enum(generic_parameter_name,
			p_extender_type);
		if (retval == 0)
			print_err(print, "Invalid QCSAPI extender param %s\n",
				generic_parameter_name);
		break;

	  case e_qcsapi_none:
	  default:
		print_err( print, "Programming error in parse generic parameter name:\n" );
		if (p_generic_parameter->generic_parameter_type == e_qcsapi_none)
		{
			print_err( print, "Called with generic parameter type of none.\n" );
		}
		else
		{
			print_err( print, "Called with unknown parameter type %d.\n",
					p_generic_parameter->generic_parameter_type );
		}
		retval = 0;
		break;
	}

	return( retval );
}

static const char *
wifi_mode_to_string(qcsapi_output *print, qcsapi_wifi_mode current_wifi_mode )
{
	const char	*retaddr = "Unknown WIFI mode";

	switch (current_wifi_mode)
	{
	  case qcsapi_mode_not_defined:
		retaddr = "WIFI mode not defined";
		break;

	  case qcsapi_access_point:
		retaddr = "Access point";
		break;

	  case qcsapi_station:
		retaddr = "Station";
		break;

	  case qcsapi_nosuch_mode:
	  default:
		print_out( print, "Unknown WIFI mode\n" );
		break;
	}

	return( retaddr );
}

static qcsapi_wifi_mode
string_to_wifi_mode(const char* str)
{
	if (strcasecmp(str, "ap") == 0) {
		return qcsapi_access_point;
	} else if (strcasecmp(str, "access_point") == 0) {
		return qcsapi_access_point;
	} else if (strcasecmp(str, "access point") == 0) {
		return qcsapi_access_point;
	} else if (strcasecmp(str, "sta") == 0) {
		return qcsapi_station;
	} else if (strcasecmp(str, "station") == 0) {
		return qcsapi_station;
	} else if (strcasecmp(str, "repeater") == 0) {
		return qcsapi_repeater;
	} else {
		return qcsapi_nosuch_mode;
	}
}

static int string_to_list(qcsapi_output *print, void *input_str, uint8_t *output_list, unsigned int *number)
{
	uint8_t list_number = 0;
	char *pcur = NULL, *pend = NULL;
	char buffer[256] = {0};
	char *input_end;
	int single_len = 0;

	if (!input_str || !output_list || !number)
		return -EINVAL;

	input_end = input_str + strnlen(input_str, 1024);
	pcur = input_str;
	do {
		pend = strchr(pcur, ',');
		if (pend) {
			single_len = pend - pcur;
			strncpy(buffer, pcur, single_len);
			buffer[single_len] = 0;
			pend++;
			output_list[list_number++] = atoi(buffer);
			pcur = pend;
		} else if (pcur) {
			output_list[list_number++] = atoi(pcur);
		}
	} while (pend && pend < input_end);

	*number = list_number;

	return 0;
}

static int
dump_generic_parameter_name(qcsapi_output *print, qcsapi_generic_parameter *p_generic_parameter )
{
	int	retval = 1;

	switch( p_generic_parameter->generic_parameter_type )
	{
	  case e_qcsapi_option:
		print_out( print, "%s", option_enum_to_name( p_generic_parameter->parameter_type.option ) );
		break;

	  case e_qcsapi_counter:
		print_out( print, "%s", counter_enum_to_name( p_generic_parameter->parameter_type.counter ) );
		break;

	  case e_qcsapi_rates:
		print_out( print, "%s", rates_enum_to_name( p_generic_parameter->parameter_type.typeof_rates ) );
		break;

	  case e_qcsapi_modulation:
		print_out(print, "%s", wifi_std_enum_to_name(
						p_generic_parameter->parameter_type.modulation));
		break;

	  case e_qcsapi_index:
	  case e_qcsapi_LED:
		print_out( print, "%u", p_generic_parameter->index );
		break;

	  case e_qcsapi_file_path_config:
		print_out( print, "security" );
		break;

	  case e_qcsapi_select_SSID:
	  case e_qcsapi_SSID_index:
		print_out( print, "%s", &(p_generic_parameter->parameter_type.the_SSID[ 0 ]) );
		break;

	  case e_qcsapi_board_parameter:
		print_out( print, "%s", board_paramter_enum_to_name( p_generic_parameter->parameter_type.board_param ) );
		break;

	  case e_qcsapi_none:
	  default:
		print_out( print, "Programming error in dump generic parameter name:\n" );
		if (p_generic_parameter->generic_parameter_type == e_qcsapi_none)
		{
			print_out( print, "Called with generic parameter type of none.\n" );
		}
		else
		{
			print_out( print, "Called with unknown parameter type %d.\n", p_generic_parameter->generic_parameter_type );
		}
		retval = 0;
		break;
	}

	return( retval );
}

static void
dump_mac_addr(qcsapi_output *print, qcsapi_mac_addr mac_addr )
{
	  print_out( print, "%02X:%02X:%02X:%02X:%02X:%02X\n",
		mac_addr[ 0 ], mac_addr[ 1 ], mac_addr[ 2 ],
		mac_addr[ 3 ], mac_addr[ 4 ], mac_addr[ 5 ]
	  );
}

static void dump_data_array(qcsapi_output *print, uint8_t *data, int size, int order, char delimiter)
{
	int i;

	if (data == NULL)
		return;

	i = 0;
	if (order == 10) {
		do {
			print_out(print, "%d%c", data[i], delimiter);
			i++;
		} while (i < (size - 1));
		print_out(print, "%d", data[i]);
	} else {
		do {
			print_out(print, "0x%x%c", data[i], delimiter);
			i++;
		} while (i < (size - 1));
		print_out(print, "0x%x", data[i]);
	}

	print_out(print, "\n");
}

static void
dump_scs_param(qcsapi_output *print, qcsapi_scs_param_rpt *p_rpt)
{
#define MAX_SCS_PARAM_DESC 35
	int j, loop;
	uint32_t str_len = 0;
	const char *name;
	uint32_t index;

	for (j = 0; j < TABLE_SIZE(qcsapi_scs_param_names_table); j++) {
		name = qcsapi_scs_param_names_table[j].name;
		index = qcsapi_scs_param_names_table[j].index;

		str_len = min(strlen(name), strlen("scs_tdls_time_compensation"));
		if (!strncmp(name, "scs_tx_time_compensation", str_len) ||
				!strncmp(name, "scs_rx_time_compensation", str_len) ||
				 !strncmp(name, "scs_tdls_time_compensation", str_len)) {
			print_out(print, "%-*s ", MAX_SCS_PARAM_DESC, name);
			loop = SCS_MAX_TXTIME_COMP_INDEX;
			do {
				print_out(print, "%u ", p_rpt[index++].scs_cfg_param);
				loop--;
			} while (loop);
			print_out(print, "\n");
		} else {
			if (p_rpt[index].scs_signed_param_flag == 0) {
				print_out(print, "%-*s %u\n", MAX_SCS_PARAM_DESC, name, p_rpt[index].scs_cfg_param);
			}
			else if (p_rpt[index].scs_signed_param_flag == 1) {
				print_out(print, "%-*s %d\n", MAX_SCS_PARAM_DESC, name, p_rpt[index].scs_cfg_param);
			}
			else {
				print_out(print, "invalid param flag!\n");
			}
		}
	}
}
static void
report_qcsapi_error( const call_qcsapi_bundle *p_calling_bundle, const int qcsapi_errorval )
{
	char	error_msg[ 128 ] = { '\0' };
	qcsapi_output *print = p_calling_bundle->caller_output;

	qcsapi_errno_get_message( qcsapi_errorval, &error_msg[ 0 ], sizeof( error_msg ) );
	print_out( print, "QCS API error %d: %s\n", 0 - qcsapi_errorval, &error_msg[ 0 ] );
}

static void
print_start_scan_usage(qcsapi_output* print)
{
	print_out(print, "start_scan usage:\n"
			"call_qcsapi start_scan wifi0 <algorithm> <select_channels> <control_flag>\n"
			"algorithm should be : reentry, clearest(default), no_pick, background\n"
			"select_channels should be : dfs, non_dfs, all(default)\n"
			"control_flag should be: flush, active, fast, normal, slow\n");
}

static void
print_cancel_scan_usage(qcsapi_output* print)
{
	print_out(print, "cancel_scan usage:\ncall_qcsapi cancel_scan wifi0 [force]\n");
}

static int safe_atou32(char *str, uint32_t *p, qcsapi_output *print, uint32_t min, uint32_t max)
{
	uint32_t v;

	if (qcsapi_verify_numeric(str) < 0
			|| qcsapi_str_to_uint32(str, &v) < 0) {
		print_err(print, "Invalid parameter %s - must be an unsigned integer\n", str);
		return 0;
	}

	if (v < min || v > max) {
		print_err(print, "Invalid parameter %s - value must be between %u and %u\n", str, min, max);
		return 0;
	}

	*p = v;

	return 1;
}

static int safe_atou16(char *str, uint16_t *p, qcsapi_output *print, uint16_t min, uint16_t max)
{
	uint32_t v;

	if (safe_atou32(str, &v, print, min, max)) {
		*p = (uint16_t)v;
		return 1;
	}

	return 0;
}

static const char *
csw_reason_to_string(uint32_t reason_id)
{
	COMPILE_TIME_ASSERT(ARRAY_SIZE(qcsapi_csw_reason_list) == IEEE80211_CSW_REASON_MAX);

	if (reason_id < ARRAY_SIZE(qcsapi_csw_reason_list))
		return qcsapi_csw_reason_list[reason_id];

	return qcsapi_csw_reason_list[IEEE80211_CSW_REASON_UNKNOWN];
}

/* interface programs to call individual QCSAPIs */

static int
call_qcsapi_errno_get_message( const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	      statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1)
	{
		print_err( print, "Not enough parameters in call qcsapi get error message\n" );
		print_err( print, "Usage: call_qcsapi get_error_message <returned error value> <size of message buffer>\n" );
		print_err( print, "Returned error value should be less than 0\n" );
		statval = 1;
	}
	else
	{
		int		 qcsapi_retval;
		int		 qcsapi_errorval = atoi( argv[ 0 ] );
		char		*error_str = NULL;
		unsigned int	 message_size = 80;
		int		 ok_to_proceed = 1;

		if (argc >= 2 && strcmp( argv[ 1 ], "NULL" ) != 0)
		{
			message_size = (unsigned int) atoi( argv[ 1 ] );
		}

		error_str = malloc( message_size );

		if (error_str == NULL)
		{
			print_err( print, "Failed to allocate %u chars\n", message_size );
			ok_to_proceed = 0;
			statval = 1;
		}

		if (ok_to_proceed)
		{
			qcsapi_retval = qcsapi_errno_get_message( qcsapi_errorval, error_str, message_size );

			if (qcsapi_retval >= 0)
			{
				if (verbose_flag >= 0)
				{
					print_out( print, "%s\n", error_str );
				}
			}
			else
			{
				report_qcsapi_error( p_calling_bundle, qcsapi_retval );
				statval = 1;
			}

			if (error_str != NULL)
				free( error_str );
		}
	}

	return( statval );
}

static int
call_qcsapi_store_ipaddr(const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int qcsapi_retval;
	qcsapi_output *print = p_calling_bundle->caller_output;
	uint32_t ipaddr;
	uint32_t netmask;
	int netmask_len;
	char *slash;
	char *usage = "Usage: call_qcsapi store_ipaddr <ip_address>[/<netmask>]\n";

	if (argc != 1) {
		print_out(print, usage);
		return -EINVAL;
	}

	slash = strstr(argv[0], "/");
	if (slash == NULL) {
		netmask = htonl(0xFFFFFF00);
	} else {
		*slash = '\0';
		netmask_len = atoi(slash + 1);
		if (netmask_len < 1 || netmask_len > 32) {
			print_err(print, "invalid network mask %s\n", slash + 1);
			return -EINVAL;
		}
		netmask = htonl(~((1 << (32 - netmask_len)) - 1));
	}

	if (inet_pton(AF_INET, argv[0], &ipaddr) != 1) {
		print_err(print, "invalid IPv4 address %s\n", argv[0]);
		return -EINVAL;
	}
	if (ipaddr == 0) {
		print_err(print, "invalid IPv4 address %s\n", argv[0]);
		return -EINVAL;
	}

	qcsapi_retval = qcsapi_store_ipaddr(ipaddr, netmask);

	if (qcsapi_retval < 0) {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		return 1;
	}

	if (verbose_flag >= 0) {
		print_out(print, "complete\n");
	}

	return 0;
}

static int
call_qcsapi_interface_enable( const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	int			 qcsapi_retval;
	const char		*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output		*print = p_calling_bundle->caller_output;

	if (argc < 1)
	{
		print_err( print, "Not enough parameters in call qcsapi enable interface\n" );
		print_err( print, "Usage: call_qcsapi enable_interface <WiFi interface> <0 | 1>\n" );
		statval = 1;
	}
	else
	{
		int	enable_flag = atoi( argv[ 0 ] );
	  /*
	   * This program is a model for all programs that call a QCSAPI.
	   * If the verbose flag is less than 0, do not report nominal (non-error) results.
	   *
	   * Like this, one can test for aging (sockets, files not closed) without
	   * seemingly endless output of "complete", etc.
	   *
	   * And if you want to see that output, just avoid enabling quiet mode.
	   *
	   * Errors though are ALWAYS reported (else how can you see if the aging test failed?)
	   * And keep trying the test; we may want to ensure a test case that is expected to
	   * cause an error does not itself have aging problems.
	   */
		qcsapi_retval = qcsapi_interface_enable( the_interface, enable_flag );
		if (qcsapi_retval >= 0)
		{
			if (verbose_flag >= 0)
			{
				print_out( print, "complete\n" );
			}
		}
		else
		{
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}
	}

	return( statval );
}

static int
call_qcsapi_interface_get_BSSID( const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_mac_addr		 the_mac_addr;
	int			 qcsapi_retval;
	const char		*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output		*print = p_calling_bundle->caller_output;

	if (argc > 0 && strcmp( argv[ 0 ], "NULL" ) == 0)
	  qcsapi_retval = qcsapi_interface_get_BSSID( the_interface, NULL );
	else
	  qcsapi_retval = qcsapi_interface_get_BSSID( the_interface, the_mac_addr );
	if (qcsapi_retval >= 0)
	{
		if (verbose_flag >= 0)
		{
			dump_mac_addr(print, the_mac_addr );
		}
	}
	else
	{
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_interface_get_mac_addr( const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_mac_addr		 the_mac_addr;
	int			 qcsapi_retval;
	const char		*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output		*print = p_calling_bundle->caller_output;

	if (argc > 0 && strcmp( argv[ 0 ], "NULL" ) == 0)
	  qcsapi_retval = qcsapi_interface_get_mac_addr( the_interface, NULL );
	else
	  qcsapi_retval = qcsapi_interface_get_mac_addr( the_interface, the_mac_addr );
	if (qcsapi_retval >= 0)
	{
		if (verbose_flag >= 0)
		{
			dump_mac_addr(print, the_mac_addr );
		}
	}
	else
	{
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_interface_set_mac_addr( const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_output		*print = p_calling_bundle->caller_output;

	if (argc < 1)
	{
		print_err( print, "Not enough parameters in call qcsapi interface set mac address, count is %d\n", argc );
		statval = 1;
	}
	else
	{
		int			qcsapi_retval;
		qcsapi_mac_addr	the_mac_addr;
		const char		*the_interface = p_calling_bundle->caller_interface;

		if (strcmp( argv[0], "NULL" ) == 0)
			qcsapi_retval = qcsapi_interface_set_mac_addr( the_interface, NULL );
		else
		{
			int ival = parse_mac_addr( argv[0], the_mac_addr );
			if (ival >= 0)
				qcsapi_retval = qcsapi_interface_set_mac_addr( the_interface, the_mac_addr );
			else {
				print_out( print, "Error parsing MAC address %s\n", argv[0]);
				statval = 1;
			}

			if (ival >= 0)
			{
				if (qcsapi_retval >= 0)
				{
					if (verbose_flag >= 0)
					{
						print_out( print, "complete\n" );
					}
				}
				else
				{
					report_qcsapi_error( p_calling_bundle, qcsapi_retval );
					statval = 1;
				}
			}
		}
	}

	return( statval );
}

static int
call_qcsapi_interface_get_counter( const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_unsigned_int	 counter_value;
	qcsapi_unsigned_int	*p_counter_value = NULL;
	int			 qcsapi_retval;
	const char		*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output		*print = p_calling_bundle->caller_output;
	qcsapi_counter_type	 the_counter_type = p_calling_bundle->caller_generic_parameter.parameter_type.counter;

	if (argc < 1 || strcmp( argv[ 0 ], "NULL" ) != 0)
	  p_counter_value = &counter_value;

	qcsapi_retval = qcsapi_interface_get_counter( the_interface, the_counter_type, p_counter_value );
	if (qcsapi_retval >= 0)
	{
		if (verbose_flag >= 0)
		{
			print_out( print, "%u\n", (unsigned int) counter_value );
		}
	}
	else
	{
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_interface_get_counter64( const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	uint64_t counter_value;
	uint64_t *p_counter_value = NULL;
	int qcsapi_retval;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;
	qcsapi_counter_type the_counter_type =
			p_calling_bundle->caller_generic_parameter.parameter_type.counter;

	if (argc < 1 || strcmp(argv[0], "NULL") != 0)
		p_counter_value = &counter_value;

	qcsapi_retval = qcsapi_interface_get_counter64(the_interface, the_counter_type,
			p_counter_value);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0)
			print_out(print, "%llu\n", counter_value);
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_pm_get_counter( const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int			 statval = 0;
	qcsapi_unsigned_int	 counter_value;
	qcsapi_unsigned_int	*p_counter_value = NULL;
	int			 qcsapi_retval;
	const char		*the_interface = p_calling_bundle->caller_interface;
	qcsapi_counter_type	 the_counter_type = p_calling_bundle->caller_generic_parameter.parameter_type.counter;
	qcsapi_output		*print = p_calling_bundle->caller_output;
	const char		*the_pm_interval = NULL;

	if (argc < 1) {
		print_err(print, "Usage: call_qcsapi pm_get_counter <WiFi interface> <counter> <PM interval>\n");
		return 1;
	}

	if (strcmp(argv[0], "NULL") != 0) {
		the_pm_interval = argv[0];
	}

	if (argc < 2 || (strcmp(argv[1], "NULL") != 0)) {
		p_counter_value = &counter_value;
	}

	qcsapi_retval = qcsapi_pm_get_counter(the_interface, the_counter_type, the_pm_interval, p_counter_value);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out(print, "%u\n", (unsigned int) counter_value);
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_pm_get_elapsed_time( const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int			 statval = 0;
	int			 qcsapi_retval;
	const char		*the_pm_interval = NULL;
	qcsapi_unsigned_int	 elapsed_time;
	qcsapi_unsigned_int	*p_elapsed_time = NULL;
	qcsapi_output		*print = p_calling_bundle->caller_output;

	if (argc < 1) {
		print_err(print, "Usage: call_qcsapi pm_get_elapsed_time <PM interval>\n");
		return 1;
	}

	if (strcmp(argv[0], "NULL") != 0) {
		the_pm_interval = argv[0];
	}

	if (argc < 2 || (strcmp(argv[1], "NULL") != 0)) {
		p_elapsed_time = &elapsed_time;
	}

	qcsapi_retval = qcsapi_pm_get_elapsed_time(the_pm_interval, p_elapsed_time);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out(print, "%u\n", (unsigned int) elapsed_time);
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}


static int
call_qcsapi_flash_image_update( const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	int				 qcsapi_retval;
	qcsapi_flash_partiton_type	 partition_type = qcsapi_nosuch_partition;
	const char			*image_file_path = NULL;
	qcsapi_output			*print = p_calling_bundle->caller_output;

	if (argc < 2) {
		print_err( print, "Not enough parameters in call qcsapi flash image update, count is %d\n", argc );
		print_err( print, "Usage: call_qcsapi flash_image_update <image file path> <live | safety>\n" );
		statval = 1;
	} else {
		if (strcmp( argv[ 0 ], "NULL" ) != 0) {
			image_file_path = argv[ 0 ];
		}

		if (name_to_partition_type( argv[ 1 ], &partition_type ) == 0) {
			print_err( print, "Unrecognized flash memory partition type %s\n", argv[ 1 ] );
			statval = 1;
		} else {
			qcsapi_retval = qcsapi_flash_image_update( image_file_path, partition_type );
			if (qcsapi_retval >= 0) {
				if (verbose_flag >= 0) {
					print_out( print, "complete\n" );
				}
			} else {
				report_qcsapi_error( p_calling_bundle, qcsapi_retval );
				statval = 1;
			}
		}
	}

	return( statval );
}

#define GET_FIRMWARE_VERSION_MAX_LEN	40

static int
call_qcsapi_firmware_get_version( const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int			 statval = 0;
	int			 qcsapi_retval;
	char			 firmware_version[ GET_FIRMWARE_VERSION_MAX_LEN ];
	char			*p_firmware_version = &firmware_version[ 0 ];
	qcsapi_unsigned_int	 version_size = GET_FIRMWARE_VERSION_MAX_LEN;
	qcsapi_output		*print = p_calling_bundle->caller_output;

	if (argc > 0) {
		if (strcmp( argv[ 0 ], "NULL" ) == 0 ) {
			p_firmware_version = NULL;
		}
		else if (isdigit( argv[ 0 ][ 0 ] )) {
			version_size = atoi( argv[ 0 ] );

			if (version_size > GET_FIRMWARE_VERSION_MAX_LEN) {
				version_size = GET_FIRMWARE_VERSION_MAX_LEN;
			}
		}
	}

	qcsapi_retval = qcsapi_firmware_get_version( p_firmware_version, version_size );

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "%s\n", p_firmware_version );
		}
	}
	else {
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_system_get_time_since_start(const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int			 statval = 0;
	int			 qcsapi_retval;
	qcsapi_unsigned_int	 time_since_startup;
	qcsapi_unsigned_int	*p_time_since_startup = &time_since_startup;
	qcsapi_output		*print = p_calling_bundle->caller_output;

	if (argc > 0 && strcmp(argv[0], "NULL") == 0) {
		p_time_since_startup = NULL;
	}

	qcsapi_retval = qcsapi_system_get_time_since_start(p_time_since_startup);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "%u\n", time_since_startup);
		}
	}
	else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_get_system_status(const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval;
	qcsapi_unsigned_int status;
	qcsapi_output *print = p_calling_bundle->caller_output;

	qcsapi_retval = qcsapi_get_system_status(&status);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out(print, "%X\n", status);
			int id;
			for (id = 0; id < TABLE_SIZE(qcsapi_sys_status_table); id++) {
				print_out(print, "bit %-2d - %s\n", qcsapi_sys_status_table[id].bit_id,
						qcsapi_sys_status_table[id].description);
			}
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_get_random_seed(const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval;
	qcsapi_output *print = p_calling_bundle->caller_output;
	struct qcsapi_data_512bytes *random_buf;
	int i;

	random_buf = malloc(sizeof(*random_buf));

	if (!random_buf) {
		print_err(print, "Failed to allocate %u bytes\n", sizeof(*random_buf));
		return 1;
	}

	qcsapi_retval = qcsapi_get_random_seed(random_buf);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			for (i = 0; i < sizeof(random_buf->data); i++) {
				print_out(print, "%c", random_buf->data[i]);
			}
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	free(random_buf);

	return statval;
}

static int
call_qcsapi_set_random_seed(const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval;
	qcsapi_output *print = p_calling_bundle->caller_output;
	struct qcsapi_data_512bytes *random_buf;
	qcsapi_unsigned_int entropy = 0;

	if (argc < 2) {
		print_err(print, "Usage: call_qcsapi set_random_seed <random_string> <entropy>\n");
		return 1;
	}

	entropy = atoi(argv[1]);

	random_buf = malloc(sizeof(*random_buf));

	if (!random_buf) {
		print_err(print, "Failed to allocate %u bytes\n", sizeof(*random_buf));
		return 1;
	}

	memset(random_buf, 0, sizeof(*random_buf));
	memcpy((void *)random_buf->data, (void *)argv[0],
			min(sizeof(random_buf->data), strlen(argv[0])));

	qcsapi_retval = qcsapi_set_random_seed(random_buf, entropy);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out(print, "complete\n");
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	free(random_buf);

	return statval;
}

static int
call_qcsapi_led_get( const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int		statval = 0;
	int		qcsapi_retval;
	uint8_t		the_led = (uint8_t) (p_calling_bundle->caller_generic_parameter.index);
	uint8_t		led_value, *p_led_value = NULL;
	qcsapi_output	*print = p_calling_bundle->caller_output;

	if (argc < 1 || strcmp( argv[ 0 ], "NULL" ) != 0)
	  p_led_value = &led_value;

	qcsapi_retval = qcsapi_led_get( the_led, p_led_value );
	if (qcsapi_retval >= 0)
	{
		if (verbose_flag >= 0)
		{
			print_out( print, "%u\n", led_value );
		}
	}
	else
	{
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_led_set( const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1)
	{
		print_err( print, "Not enough parameters in call qcsapi LED set, count is %d\n", argc );
		statval = 1;
	}
	else
	{
		int		qcsapi_retval;
		uint8_t		the_led = (uint8_t) (p_calling_bundle->caller_generic_parameter.index);
		uint8_t		new_value = (uint8_t) atoi( argv[ 0 ] );

		qcsapi_retval = qcsapi_led_set( the_led, new_value );
		if (qcsapi_retval >= 0)
		{
			if (verbose_flag >= 0)
			{
				print_out( print, "complete\n" );
			}
		}
		else
		{
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}
	}

	return( statval );
}

static int
call_qcsapi_led_pwm_enable( const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;
	int qcsapi_retval = 0;
	uint8_t led_ident = (uint8_t) (p_calling_bundle->caller_generic_parameter.index);
	qcsapi_unsigned_int onoff = 0;
	qcsapi_unsigned_int high_count = 0;
	qcsapi_unsigned_int low_count = 0;

	if (argc < 1)
		goto usage;
	if (sscanf(argv[0], "%u", &onoff) != 1)
		goto usage;
	if (onoff != 0 && argc < 3)
		goto usage;
	if (onoff != 0) {
		if (sscanf(argv[1], "%u", &high_count) != 1)
			goto usage;
		if (sscanf(argv[2], "%u", &low_count) != 1)
			goto usage;
	}

	qcsapi_retval = qcsapi_led_pwm_enable(led_ident, (uint8_t)onoff, high_count, low_count);
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0)
			print_out( print, "complete\n" );
	} else {
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return (statval);

usage:
	print_err(print, "Usage: call_qcsapi set_LED_PWM <led_ident> (1|0) <high_count> <low_count>\n");
	statval = 1;

	return (statval);
}

static int
call_qcsapi_led_brightness( const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;
	int qcsapi_retval = 0;
	uint8_t led_ident = (uint8_t) (p_calling_bundle->caller_generic_parameter.index);
	qcsapi_unsigned_int level = 0;

	if (argc < 1)
		goto usage;
	if (sscanf(argv[0], "%u", &level) != 1)
		goto usage;

	qcsapi_retval = qcsapi_led_brightness(led_ident, level);
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0)
			print_out( print, "complete\n" );
	} else {
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return (statval);

usage:
	print_err(print, "Usage: call_qcsapi set_LED_brightness <led_ident> <level>\n");
	statval = 1;

	return (statval);
}

static int
call_qcsapi_gpio_get_config( const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	int			qcsapi_retval;
	uint8_t			the_gpio = (uint8_t) (p_calling_bundle->caller_generic_parameter.index);
	qcsapi_gpio_config	gpio_config, *p_gpio_config = NULL;
	qcsapi_output		*print = p_calling_bundle->caller_output;

	if (argc < 1 || strcmp( argv[ 0 ], "NULL" ) != 0)
	  p_gpio_config = &gpio_config;

	qcsapi_retval = qcsapi_gpio_get_config( the_gpio, p_gpio_config );
	if (qcsapi_retval >= 0)
	{
		if (verbose_flag >= 0)
		{
			print_out( print, "%u\n", gpio_config );
		}
	}
	else
	{
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_gpio_set_config( const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1)
	{
		print_err( print, "Not enough parameters in call qcsapi GPIO set config, count is %d\n", argc );
		statval = 1;
	}
	else
	{
		int			qcsapi_retval;
		uint8_t			the_gpio = (uint8_t) (p_calling_bundle->caller_generic_parameter.index);
		qcsapi_gpio_config	new_value = (qcsapi_gpio_config) atoi( argv[ 0 ] );

		qcsapi_retval = qcsapi_gpio_set_config( the_gpio, new_value );
		if (qcsapi_retval >= 0)
		{
			if (verbose_flag >= 0)
			{
				print_out( print, "complete\n" );
			}
		}
		else
		{
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}
	}

	return( statval );
}

static int
call_qcsapi_gpio_enable_wps_push_button( const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1)
	{
		print_err( print, "Not enough parameters in call qcsapi GPIO enable wps push button, count is %d\n", argc );
		statval = 1;
	}
	else
	{
		int		qcsapi_retval;
		uint8_t		use_interrupt_flag = 0;
		uint8_t		wps_push_button = (uint8_t) (p_calling_bundle->caller_generic_parameter.index);
		uint8_t		active_logic = (uint8_t) atoi( argv[ 0 ] );

		if (argc > 1 && strcasecmp( argv[ 1 ], "intr" ) == 0)
		  use_interrupt_flag = 1;

		qcsapi_retval = qcsapi_gpio_enable_wps_push_button( wps_push_button, active_logic, use_interrupt_flag );
		if (qcsapi_retval >= 0)
		{
			if (verbose_flag >= 0)
			{
				print_out( print, "complete\n" );
			}
		}
		else
		{
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}
	}

	return( statval );
}

static int
call_qcsapi_file_path_get_config( const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	int			qcsapi_retval;
	qcsapi_file_path_config	the_file_path_config =
			(qcsapi_file_path_config) (p_calling_bundle->caller_generic_parameter.index);
	char			file_path[ 80 ], *p_file_path = NULL;
	qcsapi_output		*print = p_calling_bundle->caller_output;

	if (argc < 1 || strcmp( argv[ 0 ], "NULL" ) != 0)
	  p_file_path = &file_path[ 0 ];

	qcsapi_retval = qcsapi_file_path_get_config( the_file_path_config, p_file_path, sizeof( file_path ) );
	if (qcsapi_retval >= 0)
	{
		if (verbose_flag >= 0)
		{
			print_out( print, "%s\n", &file_path[ 0 ] );
		}
	}
	else
	{
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_file_path_set_config( const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1)
	{
		print_err( print, "Not enough parameters in call qcsapi file path set config, count is %d\n", argc );
		statval = 1;
	}
	else
	{
		int			qcsapi_retval;
		qcsapi_file_path_config	the_file_path_config =
				(qcsapi_file_path_config) (p_calling_bundle->caller_generic_parameter.index);

		qcsapi_retval = qcsapi_file_path_set_config( the_file_path_config, argv[ 0 ] );
		if (qcsapi_retval >= 0)
		{
			if (verbose_flag >= 0)
			{
				print_out( print, "complete\n" );
			}
		}
		else
		{
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}
	}

	return( statval );
}

static int
call_qcsapi_wifi_set_wifi_macaddr(const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1)
	{
		print_err( print, "Not enough parameters in call qcsapi file path set config, count is %d\n", argc );
		statval = 1;
	}
	else
	{
		qcsapi_mac_addr new_mac_addr;
		int		qcsapi_retval;
		int		ival = 0;

		if (strcmp( "NULL", argv[ 0 ] ) == 0)
		  qcsapi_retval = qcsapi_wifi_set_wifi_macaddr( NULL );
		else
		{
			ival = parse_mac_addr( argv[ 0 ], new_mac_addr );
			if (ival >= 0)
			  qcsapi_retval = qcsapi_wifi_set_wifi_macaddr( new_mac_addr );
			else
			{
				print_out( print, "Error parsing MAC address %s\n", argv[ 0 ] );
				statval = 1;
			}
		}

		if (ival >= 0)
		{
			if (qcsapi_retval >= 0)
			{
				if (verbose_flag >= 0)
				{
					print_out( print, "complete\n" );
				}
			}
			else
			{
				report_qcsapi_error( p_calling_bundle, qcsapi_retval );
				statval = 1;
			}
		}
	}

	return( statval );
}

static int
call_qcsapi_wifi_create_restricted_bss(const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval = 0;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;
	qcsapi_mac_addr mac_addr = {0};

	if (argc == 1) {
		qcsapi_retval = parse_mac_addr( argv[ 0 ], mac_addr );
		if (qcsapi_retval < 0) {
			print_out( print, "Error parsing MAC address %s\n", argv[ 0 ] );
			statval = 1;
		}
	}

	if (qcsapi_retval >= 0) {
		qcsapi_retval = qcsapi_wifi_create_restricted_bss(the_interface, mac_addr);
	}

	if (qcsapi_retval >= 0)
	{
		if (verbose_flag >= 0)
		{
			print_out( print, "complete\n");
		}
	}
	else
	{
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_create_bss(const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval = 0;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;
	qcsapi_mac_addr mac_addr = {0};

	if (argc == 1) {
		qcsapi_retval = parse_mac_addr( argv[ 0 ], mac_addr );
		if (qcsapi_retval < 0) {
			print_out( print, "Error parsing MAC address %s\n", argv[ 0 ] );
			statval = 1;
		}
	}

	if (qcsapi_retval >= 0) {
		qcsapi_retval = qcsapi_wifi_create_bss(the_interface, mac_addr);
	}

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "complete\n");
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_remove_bss(const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval = 0;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;

	qcsapi_retval = qcsapi_wifi_remove_bss(the_interface);
	if (qcsapi_retval >= 0)
	{
		if (verbose_flag >= 0)
		{
			print_out( print, "complete\n");
		}
	}
	else
	{
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_get_primary_interface(const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval = 0;
	char ifname[IFNAMSIZ];
	qcsapi_output *print = p_calling_bundle->caller_output;

	memset(ifname, 0, IFNAMSIZ);
	qcsapi_retval = qcsapi_get_primary_interface(ifname, IFNAMSIZ - 1);
	if (qcsapi_retval >= 0)
	{
		if (verbose_flag >= 0)
		{
			print_out( print, "%s\n", ifname);
		}
	}
	else
	{
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_get_interface_by_index(const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval = 0;
	char ifname[IFNAMSIZ];
	qcsapi_unsigned_int if_index = p_calling_bundle->caller_generic_parameter.index;
	qcsapi_output *print = p_calling_bundle->caller_output;

	memset(ifname, 0, IFNAMSIZ);
	qcsapi_retval = qcsapi_get_interface_by_index(if_index, ifname, IFNAMSIZ - 1);

	if (qcsapi_retval >= 0)
	{
		if (verbose_flag >= 0)
		{
			print_out( print, "%s\n", ifname);
		}
	}
	else
	{
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_get_mode( const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_wifi_mode	 current_wifi_mode, *p_wifi_mode = NULL;
	int			 qcsapi_retval;
	const char		*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output		*print = p_calling_bundle->caller_output;

	if (argc < 1 || strcmp( argv[ 0 ], "NULL" ) != 0)
	  p_wifi_mode = &current_wifi_mode;
	qcsapi_retval = qcsapi_wifi_get_mode( the_interface, p_wifi_mode );
	if (qcsapi_retval >= 0)
	{
		if (verbose_flag >= 1)
		{
			print_out( print, "%d (%s)\n", (int) current_wifi_mode,
					wifi_mode_to_string(print, current_wifi_mode ) );
		}
		else if (verbose_flag >= 0)
		{
			print_out( print, "%s\n",
					wifi_mode_to_string(print, current_wifi_mode ) );
		}
	  /*
	   * Else display nothing in quiet mode (verbose flag < 0)
	   */
	}
	else
	{
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_set_mode( const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1)
	{
		print_err( print, "Not enough parameters in call qcsapi WiFi set mode, count is %d\n", argc );
		statval = 1;
	}
	else
	{
		int		 qcsapi_retval;
		const char	*the_interface = p_calling_bundle->caller_interface;
		qcsapi_wifi_mode new_wifi_mode;

		new_wifi_mode = string_to_wifi_mode(argv[0]);

		if (new_wifi_mode == qcsapi_nosuch_mode) {
			print_err( print, "Unrecognized WiFi mode %s\n", argv[ 0 ] );
			statval = 1;
			return( statval );
		}

		qcsapi_retval = qcsapi_wifi_set_mode( the_interface, new_wifi_mode );
		if (qcsapi_retval >= 0)
		{
			if (verbose_flag >= 0)
			{
				print_out( print, "complete\n" );
			}
		}
		else
		{
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}
	}

	return( statval );
}

static int
call_qcsapi_wifi_get_phy_mode( const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int				statval = 0;
	int				qcsapi_retval;
	const char		*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output	*print = p_calling_bundle->caller_output;
	string_64		phy_mode;

	if (argc > 0 && (strcmp(argv[ 0 ], "NULL") == 0))
	{
		qcsapi_retval = -EFAULT;
	}
	else
	{
		memset(phy_mode, 0 , sizeof(phy_mode));
		qcsapi_retval = qcsapi_wifi_get_phy_mode( the_interface, phy_mode );
	}

	if (qcsapi_retval >= 0)
	{
		if (verbose_flag >= 0)
			print_out( print, "%s\n", phy_mode );
	}
	else
	{
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_set_phy_mode( const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int     statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1)
	{
		print_err( print, "Not enough parameters in call qcsapi WiFi set phy mode, count is %d\n", argc );
		statval = 1;
	}
	else
	{
		int			qcsapi_retval;
		const char	*the_interface = p_calling_bundle->caller_interface;
		const char	*mode = argv[0];

		qcsapi_retval = qcsapi_wifi_set_phy_mode( the_interface, mode );

		if (qcsapi_retval >= 0)
		{
			if (verbose_flag >= 0)
			{
				print_out( print, "complete\n" );
			}
		}
		else
		{
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}
	}

	return( statval );
}

static int
call_qcsapi_wifi_reload_in_mode( const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1) {
		print_err( print, "Not enough parameters in call qcsapi WiFi reload in mode, count is %d\n", argc );
		statval = 1;
	} else {
		int		qcsapi_retval;
		const char	*the_interface = p_calling_bundle->caller_interface;
		qcsapi_wifi_mode new_wifi_mode;

		new_wifi_mode = string_to_wifi_mode(argv[0]);

		if (new_wifi_mode == qcsapi_nosuch_mode) {
			print_err( print, "Unrecognized WiFi mode %s\n", argv[ 0 ] );
			statval = 1;
			return( statval );
		}

		qcsapi_retval = qcsapi_wifi_reload_in_mode( the_interface, new_wifi_mode );
		if (qcsapi_retval >= 0) {
			if (verbose_flag >= 0) {
				print_out( print, "complete\n" );
			}
		} else {
			if (new_wifi_mode == qcsapi_repeater && qcsapi_retval == -EOPNOTSUPP) {
				print_out(print, "MBSS is not supported in repeater mode - "
						"remove MBSS or repeater config\n");
			}
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}
	}

	return( statval );
}

static int
call_qcsapi_wifi_rfenable(const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;
	int len, i;

	if (argc < 1) {
		print_err(print, "Not enough parameters in call qcsapi rfenable, count is %d\n", argc);
		statval = 1;
	} else {
		int qcsapi_retval;

		len = strlen(argv[0]);
		for (i = 0; i < len; i++){
			if (isdigit(argv[0][i]) == 0){
				print_err(print, "Numerical parameter is required\n");
				statval = 1;
				return( statval );
			}
		}

		qcsapi_unsigned_int onoff = !!atoi(argv[0]);
		qcsapi_retval = qcsapi_wifi_rfenable(onoff);
		if (qcsapi_retval >= 0) {
			if (verbose_flag >= 0) {
				print_out(print, "complete\n");
			}
		} else {
			report_qcsapi_error(p_calling_bundle, qcsapi_retval);
			statval = 1;
		}
	}

	return statval;
}

static int
call_qcsapi_wifi_rfstatus(const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int	statval = 0;
	qcsapi_unsigned_int	rfstatus, *p_rfstatus = NULL;
	int			qcsapi_retval;
	qcsapi_output	*print = p_calling_bundle->caller_output;

	if (argc < 1 || strcmp( argv[ 0 ], "NULL" ) != 0)
		p_rfstatus = &rfstatus;

	qcsapi_retval = qcsapi_wifi_rfstatus( p_rfstatus );
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "%s\n", rfstatus ? "On" : "Off" );
		}
	} else {
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wifi_startprod(const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int		statval = 0;
	int		qcsapi_retval;
	qcsapi_output	*print = p_calling_bundle->caller_output;

	qcsapi_retval = qcsapi_wifi_startprod();

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out(print, "complete\n");
		}
	} else {
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wifi_get_bw( const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_unsigned_int	 current_bw, *p_bw = NULL;
	int			 qcsapi_retval;
	const char		*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output		*print = p_calling_bundle->caller_output;

	if (argc < 1 || strcmp( argv[ 0 ], "NULL" ) != 0)
	  p_bw = &current_bw;
	qcsapi_retval = qcsapi_wifi_get_bw( the_interface, p_bw );
	if (qcsapi_retval >= 0)
	{
		if (verbose_flag >= 0)
		{
			print_out( print, "%u\n", current_bw );
		}
	  /*
	   * Else display nothing in quiet mode (verbose flag < 0)
	   */
	}
	else
	{
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_set_bw( const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1)
	{
		print_err( print, "Not enough parameters in call qcsapi WiFi set bw, count is %d\n", argc );
		print_err( print, "Usage: call_qcsapi set_bw <WiFi interface> <40 | 20>\n" );
		statval = 1;
	}
	else
	{
		qcsapi_unsigned_int	 current_bw = (qcsapi_unsigned_int) atoi( argv[ 0 ] );
		int			 qcsapi_retval;
		const char		*the_interface = p_calling_bundle->caller_interface;

		qcsapi_retval = qcsapi_wifi_set_bw( the_interface, current_bw );
		if (qcsapi_retval >= 0)
		{
			if (verbose_flag >= 0)
			{
				print_out( print, "complete\n" );
			}
		  /*
		   * Else display nothing in quiet mode (verbose flag < 0)
		   */
		}
		else
		{
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}
	}

	return( statval );
}

static int
call_qcsapi_wifi_get_BSSID( const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_mac_addr	 the_mac_addr;
	int		 qcsapi_retval;
	const char	*the_interface = p_calling_bundle->caller_interface;

	if (argc > 0 && strcmp( argv[ 0 ], "NULL" ) == 0)
	  qcsapi_retval = qcsapi_wifi_get_BSSID( the_interface, NULL );
	else
	  qcsapi_retval = qcsapi_wifi_get_BSSID( the_interface, the_mac_addr );

	if (qcsapi_retval >= 0)
	{
		if (verbose_flag >= 0)
		{
			dump_mac_addr(p_calling_bundle->caller_output, the_mac_addr );
		}
	}
	else
	{
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_get_config_BSSID( const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_mac_addr	 the_mac_addr;
	int		 qcsapi_retval;
	const char	*the_interface = p_calling_bundle->caller_interface;

	if (argc > 0 && strcmp(argv[0], "NULL" ) == 0) {
		qcsapi_retval = qcsapi_wifi_get_config_BSSID( the_interface, NULL );
	} else {
		qcsapi_retval = qcsapi_wifi_get_config_BSSID( the_interface, the_mac_addr );
	}

	if (qcsapi_retval >= 0)
	{
		if (verbose_flag >= 0)
		{
			dump_mac_addr(p_calling_bundle->caller_output, the_mac_addr );
		}
	}
	else
	{
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_ssid_get_bssid(const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int		statval = 0;
	qcsapi_mac_addr	the_mac_addr;
	int		qcsapi_retval = 0;
	const char	*the_interface = p_calling_bundle->caller_interface;
	const char	*SSID = p_calling_bundle->caller_generic_parameter.parameter_type.the_SSID;

	qcsapi_retval = qcsapi_wifi_ssid_get_bssid(the_interface, SSID, the_mac_addr);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			dump_mac_addr(p_calling_bundle->caller_output, the_mac_addr);
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_ssid_set_bssid(const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int		statval = 0;
	qcsapi_mac_addr	the_mac_addr;
	int		qcsapi_retval = 0;
	int		ival = 0;
	const char	*the_interface = p_calling_bundle->caller_interface;
	const char	*SSID = p_calling_bundle->caller_generic_parameter.parameter_type.the_SSID;
	qcsapi_output	*print = p_calling_bundle->caller_output;

	ival = parse_mac_addr(argv[0], the_mac_addr);

	if (ival >= 0) {
		qcsapi_retval = qcsapi_wifi_ssid_set_bssid(the_interface, SSID, the_mac_addr);

		if (qcsapi_retval >= 0) {
			if (verbose_flag >= 0) {
				print_out( print, "complete\n");
			}
		} else {
			report_qcsapi_error(p_calling_bundle, qcsapi_retval);
			statval = 1;
		}

	} else {
		print_out( print, "Error parsing MAC address %s\n", argv[0]);
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_get_SSID( const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int statval = 0;
	qcsapi_SSID current_SSID;
	int qcsapi_retval;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;

	memset(current_SSID, 0, sizeof(current_SSID));
	if (argc > 0 && strcmp( argv[ 0 ], "NULL" ) == 0)
	  qcsapi_retval = qcsapi_wifi_get_SSID( the_interface, NULL );
	else
	  qcsapi_retval = qcsapi_wifi_get_SSID( the_interface, current_SSID );

	if (qcsapi_retval >= 0)
	{
		if (verbose_flag >= 0)
		{
			print_out( print, "%s\n", current_SSID );
		}
	}
	else
	{
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_set_SSID( const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	int		 qcsapi_retval;
	const char	*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output	*print = p_calling_bundle->caller_output;

	if (argc < 1)
	{
		print_err( print, "Not enough parameters in call qcsapi WiFi set SSID, count is %d\n", argc );
		statval = 1;
	}
	else
	{
		char	*new_SSID = argv[ 0 ];
	  /*
	   * For set SSID, require the Force NULL address flag to be set, so NULL can be used as an SSID.
	   */
		if (((internal_flags & m_force_NULL_address) == m_force_NULL_address) &&
		    (strcmp( argv[ 0 ], "NULL" ) == 0))
		  new_SSID = NULL;

		qcsapi_retval = qcsapi_wifi_set_SSID( the_interface, new_SSID );
		if (qcsapi_retval >= 0)
		{
			if (verbose_flag >= 0)
			{
				print_out( print, "complete\n" );
			}
		}
		else
		{
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}
	}

	return( statval );
}

static int
call_qcsapi_wifi_get_channel( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int			 statval = 0;
	qcsapi_unsigned_int	 channel_value, *p_channel_value = NULL;
	int			 qcsapi_retval;
	const char		*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output		*print = p_calling_bundle->caller_output;

	if (argc < 1 || strcmp( argv[ 0 ], "NULL" ) != 0)
	  p_channel_value = &channel_value;
	qcsapi_retval = qcsapi_wifi_get_channel( the_interface, p_channel_value );
	if (qcsapi_retval >= 0)
	{
		if (verbose_flag >= 0)
		{
			print_out( print, "%d\n", channel_value );
		}
	}
	else
	{
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_set_channel( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1)
	{
		print_err( print, "Not enough parameters in call qcsapi WiFi set channel, count is %d\n", argc );
		statval = 1;
	}
	else
	{
		qcsapi_unsigned_int	 channel_value = atoi( argv[ 0 ] );
		int			 qcsapi_retval;
		const char		*the_interface = p_calling_bundle->caller_interface;

		qcsapi_retval = qcsapi_wifi_set_channel( the_interface, channel_value );
		if (qcsapi_retval >= 0)
		{
			if (verbose_flag >= 0)
			{
				print_out( print, "complete\n" );
			}
		}
		else
		{
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}
	}

	return( statval );
}

static int
call_qcsapi_wifi_get_auto_channel( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int		 statval = 0;
	int		 qcsapi_retval;
	qcsapi_output	*print = p_calling_bundle->caller_output;
	const char	*the_interface = p_calling_bundle->caller_interface;
	char		 channel_value_str[QCSAPI_MAX_PARAMETER_VALUE_LEN] = {0};
	qcsapi_unsigned_int current_channel;

	qcsapi_retval = qcsapi_config_get_parameter(the_interface,
						    "channel",
						    channel_value_str,
						    sizeof(channel_value_str));

	if (qcsapi_retval >= 0)
	{
		sscanf(channel_value_str, "%u", &current_channel);

		if (verbose_flag >= 0) {
			print_out( print, "%s\n", current_channel==0 ? "enabled" : "disabled" );
		}
	} else {
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_set_auto_channel( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int		 statval = 0;
	qcsapi_output	*print = p_calling_bundle->caller_output;
	qcsapi_unsigned_int current_channel;
	int		 qcsapi_retval;
	const char	*the_interface = p_calling_bundle->caller_interface;
	char		 channel_value_str[QCSAPI_MAX_PARAMETER_VALUE_LEN] = {0};
	char		*param = argv[0];

	if (argc < 1) {
		print_err( print, "Not enough parameters in call qcsapi WiFi set auto channel,"
				" count is %d\n", argc );
		statval = 1;
		return( statval );
	}

	qcsapi_retval = qcsapi_config_get_parameter(the_interface,
						    "channel",
						    channel_value_str,
						    sizeof(channel_value_str));

	if (qcsapi_retval >= 0) {
		sscanf( channel_value_str, "%u", &current_channel );
	}

	if (qcsapi_retval >= 0 && strncmp( param, "enable", strlen(param) ) == 0) {
		if (current_channel > 0) {

			qcsapi_retval = qcsapi_config_update_parameter( the_interface, "channel", "0" );
			if (qcsapi_retval >= 0) {
				qcsapi_retval = qcsapi_wifi_set_channel( the_interface, 0 );
			}
		}
	} else if (qcsapi_retval >= 0 && strncmp( param, "disable", strlen(param) ) == 0) {
		if (current_channel == 0) {

			qcsapi_retval = qcsapi_wifi_get_channel( the_interface, &current_channel );
			if (qcsapi_retval >= 0) {
				sprintf( channel_value_str, "%u", current_channel );
				qcsapi_retval = qcsapi_config_update_parameter( the_interface,
										"channel",
										channel_value_str );
			}
		}
	} else if (qcsapi_retval >= 0) {
		qcsapi_retval = -qcsapi_parameter_not_found;
	}

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "complete\n" );
		}
	} else {
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}


static int
call_qcsapi_wifi_set_chan_pri_inactive( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1)
	{
		print_err( print, "Not enough parameters, count is %d\n", argc );
		statval = 1;
	}
	else
	{
		qcsapi_unsigned_int	 channel_value = atoi( argv[ 0 ] );
		int			 qcsapi_retval;
		const char		*the_interface = p_calling_bundle->caller_interface;
		qcsapi_unsigned_int	inactive = 1;

		if (argc == 2) {
			inactive = atoi( argv[1] );
		}

		qcsapi_retval = qcsapi_wifi_set_chan_pri_inactive( the_interface, channel_value, inactive );
		if (qcsapi_retval >= 0)
		{
			if (verbose_flag >= 0)
			{
				print_out( print, "complete\n" );
			}
		}
		else
		{
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}
	}

	return( statval );
}

static int
call_qcsapi_wifi_set_chan_disabled( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int		statval = 0;
	int		qcsapi_retval;
	qcsapi_output	*print = p_calling_bundle->caller_output;
	struct qcsapi_data_256bytes chan_list;
	const char	*the_interface = p_calling_bundle->caller_interface;
	uint32_t	listlen = 0;
	uint8_t		control_flag = 0;

	if (argc != 2) {
		print_err(print,
			"Usage: call_qcsapi set_chan_disabled <WiFi interface> "
				"<channel list> <enable/disable>\n");
		return 1;
	}

	if (!isdigit(*argv[1])) {
		print_err(print,
			"Unrecognized %s; Supported channel control: 0: disable 1:enable\n",
			argv[1]);
		return 1;
	} else {
		control_flag = atoi(argv[1]);
	}

	memset(&chan_list, 0, sizeof(chan_list));
	statval = string_to_list(print, argv[0], chan_list.data, &listlen);
	if (statval < 0)
		return statval;

	qcsapi_retval = qcsapi_wifi_chan_control(the_interface, &chan_list, listlen, control_flag);
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out(print, "complete\n");
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return( statval );
}
static void
dump_disabled_chanlist(qcsapi_output *print, uint8_t *data, uint8_t cnt)
{
	int loop;

	if (cnt > 0) {
		print_out(print, "%d", data[0]);
		for (loop = 1; loop < cnt; loop++) {
			print_out(print, ",%d", data[loop]);
		}
		print_out(print, "\n");
	}
}

static int
call_qcsapi_wifi_get_chan_disabled( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	int			 qcsapi_retval;
	const char		*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output		*print = p_calling_bundle->caller_output;
	struct qcsapi_data_256bytes chan_list;
	uint8_t cnt = 0;

	qcsapi_retval = qcsapi_wifi_get_chan_disabled(the_interface, &chan_list, &cnt);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			dump_disabled_chanlist(print, chan_list.data, cnt);
		}
	} else {
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_get_standard( const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	char		 ieee_standard[ 16 ], *p_standard = NULL;
	int		 qcsapi_retval;
	const char	*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output	*print = p_calling_bundle->caller_output;

	if (argc < 1 || strcmp( argv[ 0 ], "NULL" ) != 0)
	  p_standard = &ieee_standard[ 0 ];
	qcsapi_retval = qcsapi_wifi_get_IEEE_802_11_standard( the_interface, p_standard );

	if (qcsapi_retval >= 0)
	{
		if (verbose_flag >= 0)
		{
			print_out( print, "%s\n", &ieee_standard[ 0 ] );
		}
	}
	else
	{
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_get_dtim(call_qcsapi_bundle * p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval;
	qcsapi_unsigned_int dtim;
	qcsapi_unsigned_int *p_dtim = NULL;
	const char *interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1 || strcmp(argv[0], "NULL") != 0)
		p_dtim = &dtim;

	qcsapi_retval = qcsapi_wifi_get_dtim(interface, p_dtim);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out(print, "%d\n", dtim);
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wifi_set_dtim(call_qcsapi_bundle * p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1) {
		print_err(print, "Not enough parameters in call qcsapi WiFi set dtim, count is %d\n", argc);
		statval = 1;
	} else {
		qcsapi_unsigned_int dtim = atoi(argv[0]);
		int qcsapi_retval;
		const char *interface = p_calling_bundle->caller_interface;

		qcsapi_retval = qcsapi_wifi_set_dtim(interface, dtim);
		if (qcsapi_retval >= 0) {
			if (verbose_flag >= 0) {
				print_out(print, "complete\n");
			}
		} else {
			report_qcsapi_error(p_calling_bundle, qcsapi_retval);
			statval = 1;
		}
	}

	return statval;
}

static int
call_qcsapi_wifi_get_assoc_limit(call_qcsapi_bundle * p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval;
	qcsapi_unsigned_int assoc_limit;
	qcsapi_unsigned_int *p_assoc_limit = NULL;
	const char *interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1 || strcmp(argv[0], "NULL") != 0)
		p_assoc_limit = &assoc_limit;

	qcsapi_retval = qcsapi_wifi_get_assoc_limit(interface, p_assoc_limit);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out(print, "%d\n", assoc_limit);
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wifi_set_assoc_limit(call_qcsapi_bundle * p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1) {
		print_err(print, "Not enough parameters in call qcsapi WiFi set assoc_limit, count is %d\n", argc);
		statval = 1;
	} else {
		qcsapi_unsigned_int assoc_limit = atoi(argv[0]);
		int qcsapi_retval;
		const char *interface = p_calling_bundle->caller_interface;
		int i;

		for (i = 0; argv[0][i] != 0; i++) {
			if (isdigit(argv[0][i]) == 0) {
				print_err(print, "Invalid parameter:%s, should be integer\n", argv[0]);
				return 1;
			}
		}

		qcsapi_retval = qcsapi_wifi_set_assoc_limit(interface, assoc_limit);
		if (qcsapi_retval >= 0) {
			if (verbose_flag >= 0) {
				print_out(print, "complete\n");
			}
		} else {
			report_qcsapi_error(p_calling_bundle, qcsapi_retval);
			statval = 1;
		}
	}

	return statval;
}

static int
call_qcsapi_wifi_get_bss_assoc_limit(call_qcsapi_bundle * p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval;
	qcsapi_unsigned_int assoc_limit;
	qcsapi_unsigned_int *p_assoc_limit = NULL;
	const char *interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1 || strcmp(argv[0], "NULL") != 0)
		p_assoc_limit = &assoc_limit;

	qcsapi_retval = qcsapi_wifi_get_bss_assoc_limit(interface, p_assoc_limit);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out(print, "assoc_limit %d, priority %d\n",
				0xFF & assoc_limit, 0xFF & (assoc_limit >> 8));
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wifi_set_bss_assoc_limit(call_qcsapi_bundle * p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 2) {
		print_err(print, "Not enough parameters in call qcsapi WiFi set assoc_limit, count is %d\n", argc);
		statval = 1;
	} else {
		qcsapi_unsigned_int limit;
		qcsapi_unsigned_int priority;
		qcsapi_unsigned_int assoc_limit;
		int qcsapi_retval;
		const char *interface = p_calling_bundle->caller_interface;

		if (qcsapi_str_to_uint32(argv[0], &limit)) {
			print_err(print, "Invalid parameter %s - must be an unsigned integer\n",
					argv[0]);
			return 1;
		}

		if (qcsapi_str_to_uint32(argv[1], &priority)) {
			print_err(print, "Invalid parameter %s - must be an unsigned integer\n",
					argv[1]);
			return 1;
		}

		assoc_limit = limit | (priority << 8);

		qcsapi_retval = qcsapi_wifi_set_bss_assoc_limit(interface, assoc_limit);
		if (qcsapi_retval >= 0) {
			if (verbose_flag >= 0) {
				print_out(print, "complete\n");
			}
		} else {
			report_qcsapi_error(p_calling_bundle, qcsapi_retval);
			statval = 1;
		}
	}

	return statval;
}

static int
call_qcsapi_interface_get_status( const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	char		 interface_status[ 16 ], *p_status = NULL;
	int		 qcsapi_retval;
	const char	*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output	*print = p_calling_bundle->caller_output;

	if (argc < 1 || strcmp( argv[ 0 ], "NULL" ) != 0)
	  p_status = &interface_status[ 0 ];
	qcsapi_retval = qcsapi_interface_get_status( the_interface, p_status );

	if (qcsapi_retval >= 0)
	{
		print_out( print, "%s\n", &interface_status[ 0 ] );
	}
	else
	{
		if (verbose_flag >= 0)
		{
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		}

		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_interface_set_ip4( const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int     statval = 0;
	uint32_t        if_param_val;
	uint32_t        if_param_val_ne;
	int              qcsapi_retval;
	char            *if_param = NULL;
	const char      *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output   *print = p_calling_bundle->caller_output;
	if (argc < 2) {
		print_err( print, "Not enough parameters in call qcsapi set_ip\n" );
		print_err( print,
			"Usage: call_qcsapi set_ip <interface> <ipaddr | netmask> <ip_val | netmask_val> \n"
			);
		statval = 1;
	} else {
		if (strcmp(argv[0], "NULL") != 0)
			if_param = argv[ 0 ];

		if (inet_pton(AF_INET, argv[1], &if_param_val) != 1) {
			print_err(print, "invalid IPv4 argument %s\n", argv[1]);
			return -EINVAL;
		}
		if_param_val_ne = htonl(if_param_val);

		qcsapi_retval = qcsapi_interface_set_ip4(the_interface, if_param, if_param_val_ne);

		if (qcsapi_retval >= 0)
		{
			print_out(print, "complete\n");
		}
		else
		{
			if (verbose_flag >= 0)
			{
				report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			}
			statval = 1;
		}
	}

	return( statval );
}

static int
call_qcsapi_interface_get_ip4( const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int     statval = 0;
	string_64       if_param_val;
	char            *p_if_param_val = &if_param_val[ 0 ];
	int              qcsapi_retval;
	char		*if_param = NULL;
	const char      *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output   *print = p_calling_bundle->caller_output;

	if (argc < 2) {
		if_param = argv[0];
	}

	qcsapi_retval = qcsapi_interface_get_ip4(the_interface, if_param, p_if_param_val);

	if (qcsapi_retval >= 0)
	{
		print_out(print, "%s\n", p_if_param_val);
	}
	else
	{
		if (verbose_flag >= 0)
		{
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		}
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_get_list_channels( const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	char			*p_list_channels = NULL;
	int			 qcsapi_retval;
	const char		*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output		*print = p_calling_bundle->caller_output;
	static string_1024	 the_list_channels;
/*
 * Prefer a non-reentrant program to allocating 1025 bytes on the stack.
 */
	if (argc < 1 || strcmp( argv[ 0 ], "NULL" ) != 0)
	  p_list_channels = &the_list_channels[ 0 ];
	qcsapi_retval = qcsapi_wifi_get_list_channels( the_interface, p_list_channels );

	if (qcsapi_retval >= 0)
	{
		if (verbose_flag >= 0)
		{
			print_out( print, "%s\n", &the_list_channels[ 0 ] );
		}
	}
	else
	{
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_get_mode_switch( const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	uint8_t			 wifi_mode, *p_wifi_mode = NULL;
	int			 qcsapi_retval;
	qcsapi_output		*print = p_calling_bundle->caller_output;

	if (argc < 1 || strcmp( argv[ 0 ], "NULL" ) != 0)
	  p_wifi_mode = &wifi_mode;
	qcsapi_retval = qcsapi_wifi_get_mode_switch( p_wifi_mode );

	if (qcsapi_retval >= 0)
	{
		if (verbose_flag >= 0)
		{
			print_out( print, "%x\n", wifi_mode );
		}
	}
	else
	{
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_get_option( const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	int			 wifi_option, *p_wifi_option = NULL;
	int			 qcsapi_retval;
	const char		*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output		*print = p_calling_bundle->caller_output;
	qcsapi_option_type	 the_option = p_calling_bundle->caller_generic_parameter.parameter_type.option;

	if (argc < 1 || strcmp( argv[ 0 ], "NULL" ) != 0)
	  p_wifi_option = &wifi_option;
	qcsapi_retval = qcsapi_wifi_get_option( the_interface, the_option, p_wifi_option );

	if (qcsapi_retval >= 0)
	{
		if (verbose_flag >= 0)
		{
			if (wifi_option == 0)
			  print_out( print, "FALSE\n" );
			else
			  print_out( print, "TRUE\n" );
		}
	}
	else
	{
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

# define BUF_MAX_LEN	40

static int
call_qcsapi_get_board_parameter(const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int                             statval = 0;
	int                             qcsapi_retval = 0;
	qcsapi_output                   *print = p_calling_bundle->caller_output;
	qcsapi_board_parameter_type     the_boardparam = p_calling_bundle->caller_generic_parameter.parameter_type.board_param;
	string_64                       p_buffer;

	if (argc > 0 && (strcmp(argv[ 0 ], "NULL") == 0))
	{
		qcsapi_retval = -EFAULT;
	}
	else
	{
		memset(p_buffer, 0, sizeof(p_buffer));
		qcsapi_retval = qcsapi_get_board_parameter(the_boardparam, p_buffer);
	}

	if (qcsapi_retval >= 0)
	{
		if (verbose_flag >= 0)
		{
			print_out(print, "%s\n", p_buffer);
		}
	}
	else
	{
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return(statval);
}

static int
call_qcsapi_wifi_get_noise( const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	int		 qcsapi_retval;
	int		 current_noise, *p_noise = NULL;
	const char	*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output	*print = p_calling_bundle->caller_output;

	if (argc < 1 || strcmp( argv[ 0 ], "NULL" ) != 0) {
		p_noise = &current_noise;
	}

	qcsapi_retval = qcsapi_wifi_get_noise( the_interface, p_noise );
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "%d\n", current_noise );
		}
	} else {
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_get_rssi_by_chain( const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1) {
		print_err( print, "Not enough parameters in call qcsapi get RSSI by chain\n" );
		print_err( print, "Usage: call_qcsapi get_rssi_by_chain <WiFi interface> <RF chain>\n" );
		statval = 1;
	} else {
		int		 qcsapi_retval;
		int		 current_rssi = 0, *p_rssi = NULL;
		const char	*the_interface = p_calling_bundle->caller_interface;
		int		 rf_chain = atoi( argv[ 0 ] );

		if (argc < 2 || strcmp( argv[ 1 ], "NULL" ) != 0) {
			p_rssi = &current_rssi;
		}

		if (rf_chain == 0 && (qcsapi_verify_numeric(argv[0]) < 0)) {
			print_err( print, "Invalid argument %s - must be an integer\n", argv[ 0 ] );
			return 1;
		}

		qcsapi_retval = qcsapi_wifi_get_rssi_by_chain( the_interface, rf_chain, p_rssi );
		if (qcsapi_retval >= 0) {
			if (verbose_flag >= 0) {
				print_out( print, "%d\n", current_rssi );
			}
		} else {
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}
	}

	return( statval );
}

static int
call_qcsapi_wifi_get_avg_snr( const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int		 statval = 0;
	int		 qcsapi_retval;
	int		 current_snr, *p_snr = NULL;
	const char	*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output	*print = p_calling_bundle->caller_output;

	if (argc < 1 || strcmp( argv[ 0 ], "NULL" ) != 0) {
		p_snr = &current_snr;
	}

	qcsapi_retval = qcsapi_wifi_get_avg_snr( the_interface, p_snr );
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "%d\n", current_snr );
		}
	} else {
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_set_option( const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	int			 wifi_option;
	int			 qcsapi_retval;
	const char		*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output		*print = p_calling_bundle->caller_output;
	qcsapi_option_type	 the_option = p_calling_bundle->caller_generic_parameter.parameter_type.option;

	if (argc < 1)
	{
		print_err( print, "Not enough parameters in call qcsapi WiFi set option, count is %d\n", argc );
		statval = 1;
	}
	else
	{
		if ((strcasecmp(argv[0], "TRUE") == 0) || (strcasecmp(argv[0], "YES") == 0) ||
				(strcmp(argv[0], "1") == 0))
			wifi_option = 1;
		else if ((strcasecmp(argv[0], "FALSE") == 0) || (strcasecmp(argv[0], "NO") == 0) ||
				(strcmp(argv[0], "0") == 0))
			wifi_option = 0;
		else {
			print_err( print, "Invalid input arguments\n" );
			return 1;
		}

		qcsapi_retval = qcsapi_wifi_set_option( the_interface, the_option, wifi_option );
		if (qcsapi_retval >= 0)
		{
			if (verbose_flag >= 0)
			{
				print_out( print, "complete\n" );
			}
		}
		else
		{
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}
	}

	return( statval );
}

static int
call_qcsapi_wifi_get_rates( const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	char			*p_rates = NULL;
	int			 qcsapi_retval;
	const char		*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output		*print = p_calling_bundle->caller_output;
	qcsapi_rate_type	 the_rates_type = p_calling_bundle->caller_generic_parameter.parameter_type.typeof_rates;
	static string_1024	 the_rates;
/*
 * Prefer a non-reentrant program to allocating 1025 bytes on the stack.
 */
	if (argc < 1 || strcmp( argv[ 0 ], "NULL" ) != 0) {
		p_rates = &the_rates[ 0 ];
	}

	qcsapi_retval = qcsapi_wifi_get_rates( the_interface, the_rates_type, p_rates );

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "%s\n", the_rates );
		}
	} else {
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return (statval);
}

/**
 * validate_rates return 1 on success and 0 on failure
 */
static int
validate_rates(char *input_rate[], int num_rates)
{
        int rates[] = {2,4,11,12,18,22,24,36,48,72,96,108};
        int found = 0, i, j, rate;

        for (i = 0; i < num_rates; i++) {
                rate = atoi(input_rate[i]);
                found = 0;
                for (j = 0; j < ARRAY_SIZE(rates); j++) {
                        if (rate == rates[j]) {
                                found = 1;
                                break;
                        }

                }

                if (!found) {
			break;
		}
        }
        return found;
}

static int
call_qcsapi_wifi_set_rates( const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	int			 qcsapi_retval;
	const char		*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output		*print = p_calling_bundle->caller_output;
	qcsapi_rate_type	 the_rates_type = p_calling_bundle->caller_generic_parameter.parameter_type.typeof_rates;

	if (argc < 1) {
		print_err( print, "Not enough parameters in call qcsapi WiFi set rates, count is %d\n", argc );
		statval = 1;
	} else {
		char	*p_rates = argv[ 0 ];

		if (!validate_rates(argv, argc)) {
			print_err (print, "Invalid input rates, valid rates are 2,4,11,12,18,22,24,36,48,72,96,108 in 500Kbps units\n");
			return 1;
		}

		qcsapi_retval = qcsapi_wifi_set_rates( the_interface, the_rates_type, p_rates, argc);

		if (qcsapi_retval >= 0) {
			if (verbose_flag >= 0) {
				print_out( print, "complete\n" );
			}
		} else {
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}
	}

	return (statval);
}

static int
call_qcsapi_wifi_get_max_bitrate( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int			statval = 0;
	int			qcsapi_retval;
	const char		*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output		*print = p_calling_bundle->caller_output;

	char max_bitrate_str[QCSAPI_MAX_BITRATE_STR_MIN_LEN + 1] = {0};

	qcsapi_retval = qcsapi_get_max_bitrate(the_interface, max_bitrate_str, QCSAPI_MAX_BITRATE_STR_MIN_LEN);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "%s\n", &max_bitrate_str[ 0 ] );
		}
	} else {
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wifi_set_max_bitrate( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int			statval = 0;
	int			qcsapi_retval;
	const char		*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output		*print = p_calling_bundle->caller_output;

	if (argc < 1) {
		print_err( print, "Not enough parameters in call qcsapi wifi set max bitrate, count is %d\n", argc );
		statval = 1;
	}

	qcsapi_retval = qcsapi_set_max_bitrate(the_interface, argv[0]);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "complete\n");
		}
	} else {
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wifi_get_beacon_type( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	char		 beacon_type[ 16 ], *p_beacon = NULL;
	int		 qcsapi_retval;
	const char	*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output	*print = p_calling_bundle->caller_output;

	if (argc < 1 || strcmp( argv[ 0 ], "NULL" ) != 0)
	  p_beacon = &beacon_type[ 0 ];
	qcsapi_retval = qcsapi_wifi_get_beacon_type( the_interface, p_beacon );

	if (qcsapi_retval >= 0)
	{
		if (verbose_flag >= 0)
		{
			print_out( print, "%s\n", &beacon_type[ 0 ] );
		}
	}
	else
	{
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_set_beacon_type( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	int		 qcsapi_retval;
	const char	*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output	*print = p_calling_bundle->caller_output;

	if (argc < 1)
	{
		print_err( print, "Not enough parameters in call qcsapi WiFi set beacon, count is %d\n", argc );
		statval = 1;
	}
	else
	{
		char		 *p_beacon = argv[ 0 ];

	  /* Beacon type will not be NULL ... */

		if (strcmp( argv[ 0 ], "NULL" ) == 0)
		  p_beacon = NULL;
		qcsapi_retval = qcsapi_wifi_set_beacon_type( the_interface, p_beacon );

		if (qcsapi_retval >= 0)
		{
			if (verbose_flag >= 0)
			{
				print_out( print, "complete\n" );
			}
		}
		else
		{
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}
	}

	return( statval );
}

static int
call_qcsapi_wifi_get_beacon_interval( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_unsigned_int	bintval_value, *p_bintval_value = NULL;
	int	qcsapi_retval;
	const char *the_interface = p_calling_bundle->caller_interface;

	if (argc < 1 || strcmp( argv[ 0 ], "NULL" ) != 0)
	  p_bintval_value = &bintval_value;

	qcsapi_output	*print = p_calling_bundle->caller_output;

	qcsapi_retval = qcsapi_wifi_get_beacon_interval(the_interface,p_bintval_value);

	if( qcsapi_retval>=0 ){
		print_out( print,"%d\n",bintval_value );
	}else
	{
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_set_beacon_interval( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	int	qcsapi_retval;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;
	qcsapi_unsigned_int new_bintval = atoi( argv[ 0 ] );

	if ((new_bintval > BEACON_INTERVAL_WARNING_LOWER_LIMIT) && (new_bintval < BEACON_INTERVAL_WARNING_UPPER_LIMIT)) {
		print_out(print,"Warning, beacon interval less than 100ms may cause network performance degradation\n");
	}

	qcsapi_retval = qcsapi_wifi_set_beacon_interval(the_interface,new_bintval);

	if(qcsapi_retval>=0){
		print_out( print,"complete\n" );
	}else
	{
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return ( statval );
}

static int
call_qcsapi_wifi_get_list_regulatory_regions( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	string_256	supported_regions;
	int		qcsapi_retval;
	qcsapi_output	*print = p_calling_bundle->caller_output;

	if (argc < 1 || strcmp( argv[ 0 ], "NULL" ) != 0) {
		qcsapi_retval = qcsapi_regulatory_get_list_regulatory_regions(supported_regions);

		if (qcsapi_retval == -qcsapi_region_database_not_found) {
			qcsapi_retval = qcsapi_wifi_get_list_regulatory_regions(supported_regions);
		}

	} else {

		qcsapi_retval = qcsapi_regulatory_get_list_regulatory_regions(NULL);

		if (qcsapi_retval == -qcsapi_region_database_not_found) {
			qcsapi_retval = qcsapi_wifi_get_list_regulatory_regions(NULL);
		}
	}

	if (qcsapi_retval >= 0)
	{
		if (verbose_flag >= 0)
		{
			print_out( print, "%s\n", supported_regions );
		}
	}
	else
	{
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_get_regulatory_tx_power( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 2)
	{
		print_err( print, "Not enough parameters in call qcsapi get regulatory tx_power\n" );
		print_err( print, "Usage: call_qcsapi get_regulatory_tx_power <WiFi interface> <channel> <regulatory region>\n" );
		statval = 1;
	}
	else
	{
		int			 qcsapi_retval;
		const char		*the_interface = p_calling_bundle->caller_interface;
		qcsapi_unsigned_int	 the_channel = (qcsapi_unsigned_int) atoi( argv[ 0 ] );
		const char		*regulatory_region = NULL;
		int			*p_tx_power = NULL, tx_power = 0;

		if (strcmp( argv[ 1 ], "NULL" ) != 0)
		  regulatory_region = argv[ 1 ];

		if (argc < 3 || strcmp( argv[ 2 ], "NULL" ) != 0)
		  p_tx_power = &tx_power;


		qcsapi_retval = qcsapi_regulatory_get_regulatory_tx_power(
			the_interface,
			the_channel,
			regulatory_region,
			p_tx_power
		);

		if (qcsapi_retval == -qcsapi_region_database_not_found) {

			qcsapi_retval = qcsapi_wifi_get_regulatory_tx_power(
				the_interface,
				the_channel,
				regulatory_region,
				p_tx_power
			);
		}

		if (qcsapi_retval >= 0)
		{
			if (verbose_flag >= 0)
			{
				print_out( print, "%d\n", tx_power );
			}
		}
		else
		{
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}
	}

	return( statval );
}

static int
call_qcsapi_wifi_get_configured_tx_power(call_qcsapi_bundle *p_calling_bundle,
		int argc, char *argv[])
{
	int statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;
	const char *iface = p_calling_bundle->caller_interface;
	qcsapi_unsigned_int channel;
	const char *region;
	qcsapi_unsigned_int the_bw = 0;
	qcsapi_unsigned_int bf_on;
	qcsapi_unsigned_int number_ss;
	int retval;
	int tx_power = 0;

	const char *msg_usage_mandatory_params =
			"Not enough parameters in call qcsapi get_configured_tx_power\n"
			"Usage: call_qcsapi get_configured_tx_power"
			" <WiFi interface> <channel> <regulatory region>";

	if (argc < 2) {
		print_err(print, "%s [bandwidth] [bf_on] [number_ss]\n",
				msg_usage_mandatory_params);

		statval = 1;
		goto finish;
	}

	channel = (qcsapi_unsigned_int) atoi(argv[0]);
	region = argv[1];

	if (argc < 3) {
		retval = qcsapi_wifi_get_bw(iface, &the_bw);

		/* Call to get the BW might fail if the interface is wrong */
		if (retval < 0) {
			if ((retval == -ENODEV) || (retval == -EOPNOTSUPP)) {
				print_out(print, "Interface %s does not exist"
						"or not a Wireless Extension interface\n",
						iface);
			} else
				report_qcsapi_error(p_calling_bundle, retval);

			statval = 1;
			goto finish;
		}
	} else
		the_bw = (qcsapi_unsigned_int) atoi(argv[2]);

	if (argc < 4) {
		/* additional parameters are not specified: beamforming off, one spatial stream */
		bf_on = 0;
		number_ss = 1;
	} else if (argc >= 5) {
		bf_on = atoi(argv[3]);
		number_ss = atoi(argv[4]);
	} else {
		/* beamforming and spatial stream must be specified */
		print_err(print, "%s <bandwidth> <bf_on> <number_ss>\n",
				msg_usage_mandatory_params);

		statval = 1;
		goto finish;
	}

	retval = qcsapi_regulatory_get_configured_tx_power_ext(
			iface,
			channel,
			region,
			the_bw,
			bf_on,
			number_ss,
			&tx_power);

	if (retval == -qcsapi_region_database_not_found) {
		retval = qcsapi_wifi_get_configured_tx_power(
				iface,
				channel,
				region,
				the_bw,
				&tx_power);
	}

	if (retval >= 0) {
		if (verbose_flag >= 0)
			print_out(print, "%d\n", tx_power);
	} else {
		report_qcsapi_error(p_calling_bundle, retval);
		statval = 1;
	}

finish:

	return statval;
}

static int
call_qcsapi_wifi_set_regulatory_channel( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 2)
	{
		print_err( print, "Not enough parameters in call qcsapi set regulatory channel\n" );
		print_err( print,
	   "Usage: call_qcsapi set_regulatory_channel <WiFi interface> <channel> <regulatory region> <TX power offset>\n"
		);
		statval = 1;
	}
	else
	{
		int			 qcsapi_retval;
		const char		*the_interface = p_calling_bundle->caller_interface;
		qcsapi_unsigned_int	 the_channel = (qcsapi_unsigned_int) atoi( argv[ 0 ] );
		const char		*regulatory_region = NULL;
		qcsapi_unsigned_int	 tx_power_offset = 0;

		if (argc >= 3)
		  tx_power_offset = (qcsapi_unsigned_int) atoi( argv[ 2 ] );

		if (strcmp( argv[ 1 ], "NULL" ) != 0)
		  regulatory_region = argv[ 1 ];

		qcsapi_retval = qcsapi_regulatory_set_regulatory_channel(
				the_interface,
				the_channel,
				regulatory_region,
				tx_power_offset
			);

		if (qcsapi_retval == -qcsapi_region_database_not_found) {

			qcsapi_retval = qcsapi_wifi_set_regulatory_channel(
				the_interface,
				the_channel,
				regulatory_region,
				tx_power_offset
			);
		}



		if (qcsapi_retval >= 0)
		{
			if (verbose_flag >= 0)
			{
				print_out( print, "complete\n" );
			}
		}
		else
		{
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}
	}

	return( statval );
}

static int
call_qcsapi_wifi_set_regulatory_region( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1)
	{
		print_err( print, "Not enough parameters in call qcsapi set regulatory region\n" );
		print_err( print,
	   "Usage: call_qcsapi set_regulatory_region <WiFi interface> <regulatory region>\n"
		);
		statval = 1;
	}
	else
	{
		int		 qcsapi_retval;
		const char	*the_interface = p_calling_bundle->caller_interface;
		const char	*regulatory_region = NULL;

		if (strcmp( argv[ 0 ], "NULL" ) != 0)
		  regulatory_region = argv[ 0 ];

		qcsapi_retval = qcsapi_regulatory_set_regulatory_region(
			the_interface,
			regulatory_region
		);

		if (qcsapi_retval == -qcsapi_region_database_not_found) {
			qcsapi_retval = qcsapi_wifi_set_regulatory_region(
				the_interface,
				regulatory_region
			);
		}

		if (qcsapi_retval >= 0)
		{
			if (verbose_flag >= 0)
			{
				print_out( print, "complete\n" );
			}
		}
		else
		{
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}
	}

	return( statval );
}

static int
call_qcsapi_wifi_restore_regulatory_tx_power( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;
	int		 qcsapi_retval;
	const char	*the_interface = p_calling_bundle->caller_interface;

	qcsapi_retval = qcsapi_regulatory_restore_regulatory_tx_power(the_interface);

	if (qcsapi_retval >= 0)
	{
		if (verbose_flag >= 0)
		{
			print_out( print, "complete\n" );
		}
	}
	else
	{
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_get_regulatory_region( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;

	int		 qcsapi_retval;
	const char	*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output	*print = p_calling_bundle->caller_output;
	char		 regulatory_region[6];
	char		*p_regulatory_region = NULL;

	if (argc < 1 || strcmp( argv[ 0 ], "NULL" ) != 0) {
		p_regulatory_region = &regulatory_region[ 0 ];
	}

	qcsapi_retval = qcsapi_wifi_get_regulatory_region( the_interface, p_regulatory_region );

	if (qcsapi_retval >= 0) {
		print_out( print, "%s\n", p_regulatory_region );
	}
	else {
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_overwrite_country_code( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1)
	{
		print_err( print, "Not enough parameters in call qcsapi overwrite coutnry code\n" );
		print_err( print,
	   "Usage: call_qcsapi overwrite_country_code <WiFi interface> <curr_country_name> <new_country_name>\n"
		);
		statval = 1;
	}
	else
	{
		int		 qcsapi_retval;
		const char	*the_interface = p_calling_bundle->caller_interface;
		const char	*curr_country_name = NULL;
		const char	*new_country_name = NULL;

		if (strcmp( argv[ 0 ], "NULL" ) != 0)
			curr_country_name = argv[0];
		if (strcmp( argv[ 1 ], "NULL" ) != 0)
			new_country_name = argv[1];

		qcsapi_retval = qcsapi_regulatory_overwrite_country_code(
			the_interface,
			curr_country_name,
			new_country_name
		);

		if (qcsapi_retval >= 0)
		{
			if (verbose_flag >= 0)
				print_out( print, "complete\n" );
		} else if (qcsapi_retval == -qcsapi_configuration_error) {
			print_err( print, "Error: can't overwrite country code for provision board\n" );
			statval = 1;
		} else if (qcsapi_retval == -qcsapi_region_not_supported) {
			print_err( print, "Error: current region is not %s\n", curr_country_name);
			statval = 1;
		} else {
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}
	}

	return( statval );
}


static int
call_qcsapi_wifi_get_list_regulatory_channels( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1)
	{
		print_err( print, "Not enough parameters in call qcsapi get list regulatory channels\n" );
		print_err( print, "Usage: call_qcsapi get_list_regulatory_channels <regulatory region>\n" );
		statval = 1;
	}
	else
	{
		int			 qcsapi_retval = 0;
		char			*p_list_channels = NULL;
		const char		*regulatory_region = NULL;
		qcsapi_unsigned_int	 the_bw = 0;
/*
 * Prefer a non-reentrant program to allocating 1025 bytes on the stack.
 */
		static string_1024	 the_list_channels;

		if (strcmp( argv[ 0 ], "NULL" ) != 0)
		  regulatory_region = argv[ 0 ];

		if (argc < 2)
		{
			qcsapi_retval = qcsapi_wifi_get_bw( "wifi0", &the_bw );
		} else {
			the_bw = atoi(argv[1]);
		}

		if (argc < 3 || strcmp( argv[ 2 ], "NULL" ) != 0) {
			p_list_channels = &the_list_channels[ 0 ];
		}


		if (qcsapi_retval >= 0) {
			qcsapi_retval = qcsapi_regulatory_get_list_regulatory_channels( regulatory_region, the_bw, p_list_channels );
		}

		if (qcsapi_retval == -qcsapi_region_database_not_found) {
			qcsapi_retval = qcsapi_wifi_get_list_regulatory_channels(regulatory_region, the_bw, p_list_channels);
		}

		if (qcsapi_retval >= 0)
		{
			if (verbose_flag >= 0)
			{
				print_out( print, "%s\n", the_list_channels );
			}
		}
		else
		{
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}
	}

	return( statval );
}

static int
call_qcsapi_wifi_get_list_regulatory_bands( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1) {
		print_err( print, "Not enough parameters in call qcsapi get list regulatory channels\n" );
		print_err( print, "Usage: call_qcsapi get_list_regulatory_channels <regulatory region>\n" );
		statval = 1;

	} else {
		int qcsapi_retval;
		char *p_list_bands = NULL;
		const char *regulatory_region = NULL;

		/* Prefer a non-reentrant program to allocating 1025 bytes on the stack. */
		static string_128 the_list_bands;

		if (strcmp(argv[ 0 ], "NULL") != 0) {
			regulatory_region = argv[0];
		}

		if (argc < 3 || strcmp( argv[2], "NULL") != 0) {
			p_list_bands = &the_list_bands[0];
		}

		qcsapi_retval = qcsapi_regulatory_get_list_regulatory_bands(regulatory_region, p_list_bands);

		if (qcsapi_retval >= 0) {

			if (verbose_flag >= 0) {
				print_out( print, "%s\n", the_list_bands );
			}
		} else {
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}
	}

	return( statval );
}

static int
call_qcsapi_wifi_get_regulatory_db_version( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;
	int	qcsapi_retval;
	int	version = 0;
	int	index = 0;
	int	retval = 0;
	char	ch='v';
	int	*p_qcsapi_retval = &qcsapi_retval;
	const char *format[2] = { "%c%d", "0%c%x" };

	if (argc > 0) {
		index = atoi(argv[0]);
		ch='x';
	}

	if (verbose_flag >= 0)
		print_out(print, "Regulatory db version: ");

	do {
		*p_qcsapi_retval = qcsapi_regulatory_get_db_version(&version, index++);
		if (qcsapi_retval == -1 || retval < 0)
			break;

		print_out(print, format[argc > 0], ch, version);

		ch = '.';
		p_qcsapi_retval = &retval;
	} while (argc == 0 && qcsapi_retval >= 0);

	if (qcsapi_retval == -1) {
		print_out(print, "database not available");
	}

	print_out(print, "\n");

	if (qcsapi_retval < 0)
		statval = 1;

	return statval;
}

static int
call_qcsapi_wifi_set_regulatory_tx_power_cap( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1)
	{
		print_err( print, "Not enough parameters, count is %d\n", argc );
		statval = 1;
	}
	else
	{
		qcsapi_unsigned_int	 capped = atoi( argv[ 0 ] );
		int			 qcsapi_retval;

		qcsapi_retval = qcsapi_regulatory_apply_tx_power_cap( capped );
		if (qcsapi_retval >= 0)
		{
			if (verbose_flag >= 0)
			{
				print_out( print, "complete\n" );
			}
		}
		else
		{
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}
	}

	return( statval );
}

static int
call_qcsapi_wifi_get_tx_power( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1) {
		print_err( print, "Not enough parameters in call qcsapi get TX power\n" );
		print_err( print, "Usage: call_qcsapi get_tx_power <interface> <channel>\n" );
		statval = 1;
	}
	else {
		int			 qcsapi_retval;
		const char		*the_interface = p_calling_bundle->caller_interface;
		qcsapi_unsigned_int	 the_channel = atoi( argv[ 0 ] );
		int			 the_tx_power = 0;
		int			*p_tx_power = NULL;

		if (argc < 2 || strcmp( argv[ 1 ], "NULL" ) != 0) {
			p_tx_power = &the_tx_power;
		}

		qcsapi_retval = qcsapi_wifi_get_tx_power( the_interface, the_channel, p_tx_power );
		if (qcsapi_retval >= 0) {
			if (verbose_flag >= 0) {
				print_out( print, "%d\n", the_tx_power );
			}
		} else {
			report_qcsapi_error(p_calling_bundle, qcsapi_retval);
			statval = 1;
		}
	}

	return( statval );
}

static int
call_qcsapi_wifi_set_tx_power(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;
	int	qcsapi_retval;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_unsigned_int channel;
	int tx_power = 0;

	if (argc < 2) {
		print_err(print, "Not enough parameters in call qcsapi set_tx_power\n");
		return 1;
	}

	channel = atoi(argv[0]);
	tx_power = atoi(argv[1]);

	qcsapi_retval = qcsapi_wifi_set_tx_power(the_interface, channel, tx_power);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0)
			print_out(print, "complete\n");
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wifi_get_bw_power( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1) {
		print_err( print, "Not enough parameters in call qcsapi get_bw_power\n" );
		print_err( print, "Usage: call_qcsapi get_bw_power <interface> <channel>\n" );
		statval = 1;
	}
	else {
		int			qcsapi_retval;
		const char		*the_interface = p_calling_bundle->caller_interface;
		qcsapi_unsigned_int	the_channel = atoi( argv[ 0 ] );
		int			power_20M = 0;
		int			power_40M = 0;
		int			power_80M = 0;

		qcsapi_retval = qcsapi_wifi_get_bw_power( the_interface, the_channel,
				&power_20M, &power_40M, &power_80M );
		if (qcsapi_retval >= 0) {
			if (verbose_flag >= 0) {
				print_out( print, " pwr_20M  pwr_40M  pwr_80M\n %7d  %7d  %7d\n",
						power_20M, power_40M, power_80M );
			}
		} else {
			report_qcsapi_error(p_calling_bundle, qcsapi_retval);
			statval = 1;
		}
	}

	return( statval );
}

static int
call_qcsapi_wifi_set_bw_power(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;
	int	qcsapi_retval;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_unsigned_int channel;
	int power_20M = 0;
	int power_40M = 0;
	int power_80M = 0;

	if (argc < 2) {
		print_err(print, "Not enough parameters in call qcsapi set_bw_power\n");
		print_err( print, "Usage: call_qcsapi set_bw_power <interface> <channel>"
				" <power_20M> <power_40M> <power_80M>\n" );
		return 1;
	}

	channel = atoi(argv[0]);
	power_20M = atoi(argv[1]);
	if (argc >= 3) {
		power_40M = atoi(argv[2]);
		if (argc >= 4) {
			power_80M = atoi(argv[3]);
		}
	}

	qcsapi_retval = qcsapi_wifi_set_bw_power(the_interface, channel,
			power_20M, power_40M, power_80M);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0)
			print_out(print, "complete\n");
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wifi_get_bf_power( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 2) {
		print_err( print, "Not enough parameters in call qcsapi get_bf_power\n" );
		print_err( print, "Usage: call_qcsapi get_bf_power <interface> <channel> <number_ss>\n" );
		statval = 1;
	}
	else {
		int qcsapi_retval;
		const char *the_interface = p_calling_bundle->caller_interface;
		qcsapi_unsigned_int the_channel = atoi(argv[0]);
		int number_ss = atoi(argv[1]);
		int power_20M = 0;
		int power_40M = 0;
		int power_80M = 0;

		qcsapi_retval = qcsapi_wifi_get_bf_power( the_interface, the_channel,
				number_ss, &power_20M, &power_40M, &power_80M );
		if (qcsapi_retval >= 0) {
			if (verbose_flag >= 0) {
				print_out( print, " pwr_20M  pwr_40M  pwr_80M\n %7d  %7d  %7d\n",
						power_20M, power_40M, power_80M );
			}
		} else {
			report_qcsapi_error(p_calling_bundle, qcsapi_retval);
			statval = 1;
		}
	}

	return( statval );
}

static int
call_qcsapi_wifi_set_bf_power(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;
	int qcsapi_retval;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_unsigned_int channel;
	int power_20M = 0;
	int power_40M = 0;
	int power_80M = 0;
	int number_ss = 0;

	if (argc < 3) {
		print_err(print, "Not enough parameters in call qcsapi set_bf_power\n");
		print_err( print, "Usage: call_qcsapi set_bf_power <interface> <channel>"
				" <number_ss> <power_20M> <power_40M> <power_80M>\n" );
		return 1;
	}

	channel = atoi(argv[0]);
	number_ss = atoi(argv[1]);
	power_20M = atoi(argv[2]);
	if (argc >= 4) {
		power_40M = atoi(argv[3]);
		if (argc >= 5) {
			power_80M = atoi(argv[4]);
		}
	}

	qcsapi_retval = qcsapi_wifi_set_bf_power(the_interface, channel,
			number_ss, power_20M, power_40M, power_80M);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0)
			print_out(print, "complete\n");
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wifi_get_tx_power_ext( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 3) {
		print_err( print, "Not enough parameters in call_qcsapi get_tx_power_ext\n" );
		print_err( print, "Usage: call_qcsapi get_tx_power_ext <interface> <channel> <bf_on> <number_ss>\n" );
		statval = 1;
	} else {
		int qcsapi_retval;
		const char *the_interface = p_calling_bundle->caller_interface;
		qcsapi_unsigned_int the_channel = atoi(argv[0]);
		int bf_on = !!atoi(argv[1]);
		int number_ss = atoi(argv[2]);
		int power_20M = 0;
		int power_40M = 0;
		int power_80M = 0;

		qcsapi_retval = qcsapi_wifi_get_tx_power_ext( the_interface, the_channel,
				bf_on, number_ss, &power_20M, &power_40M, &power_80M );
		if (qcsapi_retval >= 0) {
			if (verbose_flag >= 0) {
				print_out( print, " pwr_20M  pwr_40M  pwr_80M\n %7d  %7d  %7d\n",
						power_20M, power_40M, power_80M );
			}
		} else {
			report_qcsapi_error(p_calling_bundle, qcsapi_retval);
			statval = 1;
		}
	}

	return( statval );
}

static int
call_qcsapi_wifi_set_tx_power_ext(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;
	int qcsapi_retval;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_unsigned_int channel;
	int power_20M = 0;
	int power_40M = 0;
	int power_80M = 0;
	int bf_on = 0;
	int number_ss = 0;

	if (argc < 4) {
		print_err(print, "Not enough parameters in call_qcsapi set_tx_power_ext\n");
		print_err( print, "Usage: call_qcsapi set_tx_power_ext <interface> <channel>"
				" <bf_on> <number_ss> <power_20M> <power_40M> <power_80M>\n" );
		return 1;
	}

	channel = atoi(argv[0]);
	bf_on = !!atoi(argv[1]);
	number_ss = atoi(argv[2]);
	power_20M = atoi(argv[3]);
	if (argc >= 5) {
		power_40M = atoi(argv[4]);
		if (argc >= 6) {
			power_80M = atoi(argv[5]);
		}
	}

	qcsapi_retval = qcsapi_wifi_set_tx_power_ext(the_interface, channel,
			bf_on, number_ss, power_20M, power_40M, power_80M);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0)
			print_out(print, "complete\n");
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wifi_get_chan_power_table( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1) {
		print_err( print, "Not enough parameters in call_qcsapi get_chan_power_table\n" );
		print_err( print, "Usage: call_qcsapi get_chan_power_table <interface> <channel>\n" );
		statval = 1;
	} else {
		int qcsapi_retval;
		int index;
		qcsapi_channel_power_table power_table;
		const char *the_interface = p_calling_bundle->caller_interface;

		power_table.channel = atoi(argv[0]);
		qcsapi_retval = qcsapi_wifi_get_chan_power_table(the_interface, &power_table);
		if (qcsapi_retval >= 0) {
			if (verbose_flag >= 0) {
				print_out( print, "Channel   80M      40M      20M\n");
				for (index = 0; index < QCSAPI_POWER_TOTAL; index++) {
					print_out( print,"%5d    %4d     %4d     %4d\n",
							power_table.channel,
							power_table.power_80M[index],
							power_table.power_40M[index],
							power_table.power_20M[index]);
				}
			}
		} else {
			report_qcsapi_error(p_calling_bundle, qcsapi_retval);
			statval = 1;
		}
	}

	return( statval );
}

static int
call_qcsapi_wifi_set_chan_power_table(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	qcsapi_output *print = p_calling_bundle->caller_output;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_channel_power_table power_table;
	int statval = 0;
	int qcsapi_retval;
	uint8_t channel;
	int8_t max_power;
	int8_t backoff;
	uint32_t backoff_20m = 0;
	uint32_t backoff_40m = 0;
	uint32_t backoff_80m = 0;
	char *endptr;
	int i;
	int offset;

	if (argc < 5) {
		print_err(print, "Not enough parameters in call_qcsapi set_chan_power_table\n");
		print_err( print, "Usage: call_qcsapi set_chan_power_table <interface> <channel>"
				" <max_power> <backoff_20M> <backoff_40M> <backoff_80M>\n" );
		print_err( print, "backoff_20M/40M/80M is a 32bits unsigned value, and every 4bits "
				"indicate the backoff from the max_power for a bf/ss case.\n"
				"The least significant 4 bits are for bfoff 1ss, and "
				"the most significant 4 bits are for bfon 4ss, and so forth.\n"
				"For example, max_power 23 and backoff_20M 0x54324321 means:\n"
				"  the power for 20Mhz bfoff 1ss: 23 - 1 = 22dBm\n"
				"  the power for 20Mhz bfoff 2ss: 23 - 2 = 21dBm\n"
				"  the power for 20Mhz bfoff 3ss: 23 - 3 = 20dBm\n"
				"  the power for 20Mhz bfoff 4ss: 23 - 4 = 19dBm\n"
				"  the power for 20Mhz bfon  1ss: 23 - 2 = 21dBm\n"
				"  the power for 20Mhz bfon  2ss: 23 - 3 = 20dBm\n"
				"  the power for 20Mhz bfon  3ss: 23 - 4 = 19dBm\n"
				"  the power for 20Mhz bfon  4ss: 23 - 5 = 18dBm\n");
		return 1;
	}

	channel = atoi(argv[0]);
	max_power = atoi(argv[1]);
	backoff_20m = strtoul(argv[2], &endptr, 0);
	backoff_40m = strtoul(argv[3], &endptr, 0);
	backoff_80m = strtoul(argv[4], &endptr, 0);

	power_table.channel = channel;

	if (max_power <= 0) {
		print_err(print, "Invalid max_power %d\n", max_power);
		return 1;
	}

	for (i = 0, offset = 0; i < QCSAPI_POWER_TOTAL; i++, offset += 4) {
		backoff = (backoff_20m >> offset) & 0xf;
		if (max_power <= backoff) {
			print_err(print, "Invalid backoff_20m, too large backoff"
					" for power index %d, backoff %d\n", i, backoff);
			return 1;
		}
		power_table.power_20M[i] = max_power - backoff;

		backoff = (backoff_40m >> offset) & 0xf;
		if (max_power <= backoff) {
			print_err(print, "Invalid backoff_40m, too large backoff"
					" for power index %d, backoff %d\n", i, backoff);
			return 1;
		}
		power_table.power_40M[i] = max_power - backoff;

		backoff = (backoff_80m >> offset) & 0xf;
		if (max_power <= backoff) {
			print_err(print, "Invalid backoff_80m, too large backoff"
					" for power index %d, backoff %d\n", i, backoff);
			return 1;
		}
		power_table.power_80M[i] = max_power - backoff;
	}

	qcsapi_retval = qcsapi_wifi_set_chan_power_table(the_interface, &power_table);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0)
			print_out(print, "complete\n");
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wifi_get_power_selection( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int			 statval = 0;
	qcsapi_unsigned_int	 power_selection;
	int			 qcsapi_retval;
	qcsapi_output		*print = p_calling_bundle->caller_output;

	qcsapi_retval = qcsapi_wifi_get_power_selection( &power_selection );
	if (qcsapi_retval >= 0)
	{
		if (verbose_flag >= 0)
		{
			print_out( print, "%d\n", power_selection );
		}
	}
	else
	{
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_set_power_selection( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc != 1)
	{
		print_err( print, "Incorrect parameters in call qcsapi set power selection\n");
		print_err( print, "Usage: call_qcsapi set_power_selection <0/1/2/3>\n" );
		statval = 1;
	}
	else
	{
		qcsapi_unsigned_int	 power_selection = atoi( argv[ 0 ] );
		int			 qcsapi_retval;

		qcsapi_retval = qcsapi_wifi_set_power_selection( power_selection );
		if (qcsapi_retval >= 0)
		{
			if (verbose_flag >= 0)
			{
				print_out( print, "complete\n" );
			}
		}
		else
		{
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}
	}

	return( statval );
}

static int
call_qcsapi_wifi_get_carrier_interference(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;
	int	qcsapi_retval;
	const char *the_interface = p_calling_bundle->caller_interface;
	int ci = 0;

	qcsapi_retval = qcsapi_wifi_get_carrier_interference(the_interface, &ci);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0)
			print_out(print, "%ddb\n", ci);
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wifi_get_congestion_idx(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;
	int	qcsapi_retval;
	const char *the_interface = p_calling_bundle->caller_interface;
	int ci;

	qcsapi_retval = qcsapi_wifi_get_congestion_index(the_interface, &ci);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0)
			print_out(print, "%d\n", ci);
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wifi_get_supported_tx_power_levels( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int		statval = 0;
	qcsapi_output	*print = p_calling_bundle->caller_output;
	int		qcsapi_retval;
	const char	*the_interface = p_calling_bundle->caller_interface;
	string_128	power_available = "";
	char		*p_power_available = &power_available[0];

	if (argc > 0 && strcmp(argv[ 0 ], "NULL") == 0) {
		p_power_available = NULL;
	}

	qcsapi_retval = qcsapi_wifi_get_supported_tx_power_levels(the_interface, p_power_available);
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out(print, "%s\n", p_power_available);
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wifi_get_current_tx_power_level( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int			statval = 0;
	qcsapi_output		*print = p_calling_bundle->caller_output;
	int			qcsapi_retval;
	const char		*the_interface = p_calling_bundle->caller_interface;
	qcsapi_unsigned_int	current_percentage = 0, *p_current_percentage = &current_percentage;

	if (argc > 0 && strcmp(argv[ 0 ], "NULL") == 0) {
		p_current_percentage = NULL;
	}

	qcsapi_retval = qcsapi_wifi_get_current_tx_power_level(the_interface, p_current_percentage);
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out(print, "%d\n", (int) current_percentage);
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wifi_set_power_constraint(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1)
	{
		print_err(print, "Not enough parameters in call qcsapi WiFi set power constraint, count is %d\n", argc);
		statval = 1;
	} else {
		int temp = atoi(argv[0]);
		qcsapi_unsigned_int pwr_constraint = (qcsapi_unsigned_int)temp;

		if (temp < 0) {
			qcsapi_retval = -EINVAL;
		} else {
			qcsapi_retval = qcsapi_wifi_set_power_constraint(the_interface, pwr_constraint);
		}

		if (qcsapi_retval >= 0) {
			print_out(print, "complete\n");
		} else {
			report_qcsapi_error(p_calling_bundle, qcsapi_retval);
			statval = 1;
		}
	}

	return statval;
}

static int
call_qcsapi_wifi_get_power_constraint(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	qcsapi_unsigned_int pwr_constraint, *p_pwr_constraint = NULL;
	int qcsapi_retval;
	const char *the_interface = p_calling_bundle->caller_interface;

	if (argc < 1 || strcmp(argv[0], "NULL") != 0)
		p_pwr_constraint = &pwr_constraint;

	qcsapi_output *print = p_calling_bundle->caller_output;

	qcsapi_retval = qcsapi_wifi_get_power_constraint(the_interface, p_pwr_constraint);

	if (qcsapi_retval >= 0) {
		print_out(print, "%d\n", pwr_constraint);
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wifi_set_tpc_interval(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1)
	{
		print_err(print, "Not enough parameters in call qcsapi WiFi set tpc interval, count is %d\n", argc);
		statval = 1;
	} else {
		int temp = atoi(argv[0]);

		if (temp <= 0) {
			qcsapi_retval = -EINVAL;
		} else {
			qcsapi_retval = qcsapi_wifi_set_tpc_interval(the_interface, temp);
		}

		if (qcsapi_retval >= 0) {
			print_out(print, "complete\n");
		} else {
			report_qcsapi_error(p_calling_bundle, qcsapi_retval);
			statval = 1;
		}
	}

	return statval;
}

static int
call_qcsapi_wifi_get_tpc_interval(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	qcsapi_unsigned_int tpc_interval, *p_tpc_interval = NULL;
	int qcsapi_retval;
	const char *the_interface = p_calling_bundle->caller_interface;

	if (argc < 1 || strcmp(argv[0], "NULL") != 0)
		p_tpc_interval = &tpc_interval;

	qcsapi_output *print = p_calling_bundle->caller_output;

	qcsapi_retval = qcsapi_wifi_get_tpc_interval(the_interface, p_tpc_interval);

	if (qcsapi_retval >= 0) {
		print_out(print, "%d\n", tpc_interval);
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wifi_set_scan_chk_inv(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1)
	{
		print_err(print, "call_qcsapi set_scan_chk_inv wifi0 <scan_chk_inv>\n", argc);
		statval = 1;
	} else {
		int temp = atoi(argv[0]);

		if (temp <= 0 || temp > (24 * 60 * 60)) {
			print_err(print, "value should be limited from 1 second to 24 hours\n");
			qcsapi_retval = -EINVAL;
		} else {
			qcsapi_retval = qcsapi_wifi_set_scan_chk_inv(the_interface, temp);
		}

		if (qcsapi_retval >= 0) {
			print_out(print, "complete\n");
		} else {
			report_qcsapi_error(p_calling_bundle, qcsapi_retval);
			statval = 1;
		}
	}

	return statval;
}

static int
call_qcsapi_wifi_get_scan_chk_inv(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	int scan_chk_inv, *p = NULL;
	int qcsapi_retval;
	const char *the_interface = p_calling_bundle->caller_interface;

	if (argc < 1 || strcmp(argv[0], "NULL") != 0)
		p = &scan_chk_inv;

	qcsapi_output *print = p_calling_bundle->caller_output;

	qcsapi_retval = qcsapi_wifi_get_scan_chk_inv(the_interface, p);

	if (qcsapi_retval >= 0) {
		print_out(print, "%d\n", scan_chk_inv);
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}


static void
local_display_assoc_records(qcsapi_output *print, const struct qcsapi_assoc_records *p_assoc_records)
{
	int	iter;

	for (iter = 0; iter < QCSAPI_ASSOC_MAX_RECORDS; iter++) {
		if (p_assoc_records->timestamp[iter] <= 0) {
			return;
		}

		char	 mac_addr_string[ 24 ];

		snprintf( &mac_addr_string[ 0 ], sizeof(mac_addr_string), MACFILTERINGMACFMT,
			  p_assoc_records->addr[iter][0],
			  p_assoc_records->addr[iter][1],
			  p_assoc_records->addr[iter][2],
			  p_assoc_records->addr[iter][3],
			  p_assoc_records->addr[iter][4],
			  p_assoc_records->addr[iter][5]
		);

		print_out(print, "%s: %d\n", &mac_addr_string[0], (int) p_assoc_records->timestamp[iter]);
	}
}

static int
call_qcsapi_wifi_get_assoc_records(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int				statval = 0;
	qcsapi_output			*print = p_calling_bundle->caller_output;
	int				qcsapi_retval;
	const char			*the_interface = p_calling_bundle->caller_interface;
	int				reset_flag = 0;
	struct qcsapi_assoc_records	assoc_records;
	struct qcsapi_assoc_records	*p_assoc_records = &assoc_records;

	if (argc > 0) {
		if (!isdigit(argv[0][0])) {
			print_err(print, "get_assoc_records: reset flag must be a numeric value\n");
			return 1;
		}

		reset_flag = atoi(argv[0]);
	}

	if (argc > 1 && strcmp(argv[1], "NULL") == 0) {
		p_assoc_records = NULL;
	}

	qcsapi_retval = qcsapi_wifi_get_assoc_records(the_interface, reset_flag, p_assoc_records);
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			local_display_assoc_records(print, &assoc_records);
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wifi_get_list_DFS_channels( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 2)
	{
		print_err( print, "Not enough parameters in call qcsapi get list DFS channels\n" );
		print_err( print, "Usage: call_qcsapi get_list_DFS_channels <regulatory region> <0 | 1> <bandwidth>\n" );
		statval = 1;
	}
	else
	{
		int			 qcsapi_retval;
		char			*p_list_channels = NULL;
		const char		*regulatory_region = NULL;
		int			 DFS_flag = 0;
		qcsapi_unsigned_int	 the_bw = 0;
/*
 * Prefer a non-reentrant program to allocating 1025 bytes on the stack.
 */
		static string_1024	 the_list_channels;

		if (strcmp( argv[ 0 ], "NULL" ) != 0)
		  regulatory_region = argv[ 0 ];

		DFS_flag = atoi( argv[ 1 ] );

		if (argc < 3)
		{
			qcsapi_retval = qcsapi_wifi_get_bw( "wifi0", &the_bw );
			if (qcsapi_retval < 0)
			  the_bw = 40;
		}
		else
		  the_bw = atoi( argv[ 2 ] );

		if (argc < 4 || strcmp( argv[ 3 ], "NULL" ) != 0)
		  p_list_channels = &the_list_channels[ 0 ];

		qcsapi_retval = qcsapi_regulatory_get_list_DFS_channels( regulatory_region, DFS_flag, the_bw, p_list_channels );

		if (qcsapi_retval == -qcsapi_region_database_not_found) {
			qcsapi_retval = qcsapi_wifi_get_list_DFS_channels( regulatory_region, DFS_flag, the_bw, p_list_channels );
		}

		if (qcsapi_retval >= 0)
		{
			if (verbose_flag >= 0)
			{
				print_out( print, "%s\n", the_list_channels );
			}
		}
		else
		{
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}
	}

	return( statval );
}

static int
call_qcsapi_wifi_is_channel_DFS( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 2)
	{
		print_err( print, "Not enough parameters in call qcsapi is channel DFS\n" );
		print_err( print, "Usage: call_qcsapi is_channel_DFS <regulatory region> <channel>\n" );
		statval = 1;
	}
	else
	{
		int			 qcsapi_retval;
		const char		*regulatory_region = NULL;
		int			 DFS_flag = 0;
		int			*p_DFS_flag = NULL;
		qcsapi_unsigned_int	 the_channel = (qcsapi_unsigned_int) atoi( argv[ 1 ] );

		if (strcmp( argv[ 0 ], "NULL" ) != 0)
		  regulatory_region = argv[ 0 ];

		if (argc < 3 || strcmp( argv[ 2 ], "NULL" ) != 0)
		  p_DFS_flag = &DFS_flag;

		qcsapi_retval = qcsapi_regulatory_is_channel_DFS( regulatory_region, the_channel, p_DFS_flag );

		if (qcsapi_retval == -qcsapi_region_database_not_found) {

			qcsapi_retval = qcsapi_wifi_is_channel_DFS( regulatory_region, the_channel, p_DFS_flag );
		}

		if (qcsapi_retval >= 0)
		{
			if (verbose_flag >= 0)
			{
				print_out( print, "%d\n", DFS_flag );
			}
		}
		else
		{
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}
	}

	return( statval );
}

static int
call_qcsapi_wifi_get_DFS_alt_channel( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int			 statval = 0;
	qcsapi_unsigned_int	 channel_value, *p_channel_value = NULL;
	int			 qcsapi_retval;
	const char		*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output		*print = p_calling_bundle->caller_output;

	if (argc < 1 || strcmp( argv[ 0 ], "NULL" ) != 0)
	  p_channel_value = &channel_value;
	qcsapi_retval = qcsapi_wifi_get_DFS_alt_channel( the_interface, p_channel_value );
	if (qcsapi_retval >= 0)
	{
		if (verbose_flag >= 0)
		{
			print_out( print, "%d\n", channel_value );
		}
	}
	else
	{
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_set_DFS_alt_channel( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1)
	{
		print_err( print, "Not enough parameters in call qcsapi WiFi set DFS alt channel, count is %d\n", argc );
		statval = 1;
	}
	else
	{
		qcsapi_unsigned_int	 dfs_alt_chan = atoi( argv[ 0 ] );
		int			 qcsapi_retval;
		const char		*the_interface = p_calling_bundle->caller_interface;

		qcsapi_retval = qcsapi_wifi_set_DFS_alt_channel( the_interface, dfs_alt_chan );
		if (qcsapi_retval >= 0)
		{
			if (verbose_flag >= 0)
			{
				print_out( print, "complete\n" );
			}
		}
		else
		{
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}
	}

	return( statval );
}

static int
call_qcsapi_wifi_set_dfs_reentry( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	int qcsapi_retval;
	qcsapi_output *print = p_calling_bundle->caller_output;
	const char	*the_interface = p_calling_bundle->caller_interface;

	qcsapi_retval = qcsapi_wifi_start_dfs_reentry(the_interface);
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "complete\n");
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_get_scs_cce_channels( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int			statval = 0;
	int			qcsapi_retval;
	const char		*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output		*print = p_calling_bundle->caller_output;
	qcsapi_unsigned_int	prev_chan = 0;
	qcsapi_unsigned_int	cur_chan = 0;
	qcsapi_unsigned_int	*p_prev_chan = &prev_chan;
	qcsapi_unsigned_int	*p_cur_chan = &cur_chan;

	if (argc >= 2) {
		if (strcmp(argv[1], "NULL") == 0) {
			p_cur_chan = NULL;
		}
	}

	if (argc >= 1) {
		if (strcmp(argv[0], "NULL") == 0) {
			p_prev_chan = NULL;
		}
	}

	qcsapi_retval = qcsapi_wifi_get_scs_cce_channels(the_interface, p_prev_chan, p_cur_chan);
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "%d %d\n", (int) prev_chan, (int) cur_chan);
		}
	} else {
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wifi_get_dfs_cce_channels( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int			statval = 0;
	int			qcsapi_retval;
	const char		*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output		*print = p_calling_bundle->caller_output;
	qcsapi_unsigned_int	prev_chan = 0;
	qcsapi_unsigned_int	cur_chan = 0;
	qcsapi_unsigned_int	*p_prev_chan = &prev_chan;
	qcsapi_unsigned_int	*p_cur_chan = &cur_chan;

	if (argc >= 2) {
		if (strcmp(argv[1], "NULL") == 0) {
			p_cur_chan = NULL;
		}
	}

	if (argc >= 1) {
		if (strcmp(argv[0], "NULL") == 0) {
			p_prev_chan = NULL;
		}
	}

	qcsapi_retval = qcsapi_wifi_get_dfs_cce_channels(the_interface, p_prev_chan, p_cur_chan);
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "%d %d\n", (int) prev_chan, (int) cur_chan);
		}
	} else {
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wifi_get_csw_records( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int statval = 0;
	int qcsapi_retval;
	int reset=0;
	int i;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;
	qcsapi_csw_record records;

	if (argc >= 1) {
		if (strcmp(argv[0], "1") == 0) {
			reset = 1;
		}
	}

	qcsapi_retval = qcsapi_wifi_get_csw_records(the_interface, reset, &records);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out(print, "channel switch history record count : %d\n", records.cnt);
			int index = records.index;
			int indextmp = 0;
			for (i = 0; i < records.cnt; i++){
				indextmp = (index + QCSAPI_CSW_MAX_RECORDS - i) % QCSAPI_CSW_MAX_RECORDS;
				print_out(print, "time=%u channel=%u reason=%s\n",
						records.timestamp[indextmp],
						records.channel[indextmp],
						csw_reason_to_string(records.reason[indextmp]));
			}

			if (reset) {
				print_out(print, "clear records complete\n");
			}
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval );
		statval = 1;
	}
	return statval;
}

static int
call_qcsapi_wifi_get_radar_status( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int statval = 0;
	int qcsapi_retval = 0;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;
	qcsapi_radar_status rdstatus;

	if (argc < 1) {
		print_err(print, "Not enough parameters\n");
		statval = 1;
	} else {
		memset(&rdstatus, 0, sizeof(rdstatus));
		rdstatus.channel = atoi(argv[0]);
		qcsapi_retval = qcsapi_wifi_get_radar_status(the_interface, &rdstatus);

		if(qcsapi_retval >= 0) {
			print_out(print, "channel %d:\nradar_status=%d\nradar_count=%d\n", rdstatus.channel, rdstatus.flags, rdstatus.ic_radardetected);
		} else {
			report_qcsapi_error(p_calling_bundle, qcsapi_retval);
			statval = 1;
		}
	}

	return statval;
}

static int
call_qcsapi_wifi_get_WEP_encryption_level( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	string_64	 WEP_encryption_level;
	int		 qcsapi_retval;
	const char	*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output	*print = p_calling_bundle->caller_output;

	if (argc > 0 && strcmp( argv[ 0 ], "NULL" ) == 0)
	  qcsapi_retval = qcsapi_wifi_get_WEP_encryption_level( the_interface, NULL );
	else
	  qcsapi_retval = qcsapi_wifi_get_WEP_encryption_level( the_interface, WEP_encryption_level );

	if (qcsapi_retval >= 0)
	{
		if (verbose_flag >= 0)
		{
			print_out( print, "%s\n", WEP_encryption_level );
		}
	}
	else
	{
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_get_WPA_encryption_modes( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	char		 encryption_modes[ 36 ], *p_encryption_modes = NULL;
	int		 qcsapi_retval;
	const char	*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output	*print = p_calling_bundle->caller_output;

	if (argc < 1 || strcmp( argv[ 0 ], "NULL" ) != 0)
	  p_encryption_modes = &encryption_modes[ 0 ];
	qcsapi_retval = qcsapi_wifi_get_WPA_encryption_modes( the_interface, p_encryption_modes );

	if (qcsapi_retval >= 0)
	{
		if (verbose_flag >= 0)
		{
			print_out( print, "%s\n", &encryption_modes[ 0 ] );
		}
	}
	else
	{
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_set_WPA_encryption_modes( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1)
	{
		print_err( print, "Not enough parameters in call qcsapi WiFi set encryption mode, count is %d\n", argc );
		statval = 1;
	}
	else
	{
		int		 qcsapi_retval;
		const char	*the_interface = p_calling_bundle->caller_interface;
		char		*p_encryption_modes = argv[ 0 ];

	  /* Encryption modes will not be NULL ... */

		if (strcmp( argv[ 0 ], "NULL" ) == 0)
		  p_encryption_modes = NULL;
		qcsapi_retval = qcsapi_wifi_set_WPA_encryption_modes( the_interface, p_encryption_modes );

		if (qcsapi_retval >= 0)
		{
			if (verbose_flag >= 0)
			{
				print_out( print, "complete\n" );
			}
		}
		else
		{
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}
	}

	return( statval );
}

static int
call_qcsapi_wifi_get_WPA_authentication_mode( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	char		 authentication_mode[ 36 ], *p_authentication_mode = NULL;
	int		 qcsapi_retval;
	const char	*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output	*print = p_calling_bundle->caller_output;

	if (argc < 1 || strcmp( argv[ 0 ], "NULL" ) != 0)
	  p_authentication_mode = &authentication_mode[ 0 ];
	qcsapi_retval = qcsapi_wifi_get_WPA_authentication_mode( the_interface, p_authentication_mode );

	if (qcsapi_retval >= 0)
	{
		if (verbose_flag >= 0)
		{
			print_out( print, "%s\n", &authentication_mode[ 0 ] );
		}
	}
	else
	{
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_set_WPA_authentication_mode( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1)
	{
		print_err( print, "Not enough parameters in call qcsapi WiFi set authentication mode, count is %d\n", argc );
		statval = 1;
	}
	else
	{
		int		 qcsapi_retval;
		const char	*the_interface = p_calling_bundle->caller_interface;
		char		*p_authentication_mode = argv[ 0 ];

	  /* Authentication mode will not be NULL ... */

		if (strcmp( argv[ 0 ], "NULL" ) == 0)
		  p_authentication_mode = NULL;
		qcsapi_retval = qcsapi_wifi_set_WPA_authentication_mode( the_interface, p_authentication_mode );

		if (qcsapi_retval >= 0)
		{
			if (verbose_flag >= 0)
			{
				print_out( print, "complete\n" );
			}
		}
		else
		{
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}
	}

	return( statval );
}

static int
call_qcsapi_wifi_get_interworking( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
        int             statval = 0;
        char            interworking[2],*p_interworking = NULL;
        int             qcsapi_retval;
        const char      *the_interface = p_calling_bundle->caller_interface;
        qcsapi_output   *print = p_calling_bundle->caller_output;

        if (argc < 1 || strcmp( argv[ 0 ], "NULL" ) != 0)
                p_interworking = &interworking[0];

        qcsapi_retval = qcsapi_wifi_get_interworking( the_interface, p_interworking );

        if (qcsapi_retval >= 0) {
                if (verbose_flag >= 0) {
                        print_out( print, "%s\n", &interworking );
                }
        } else {
                report_qcsapi_error( p_calling_bundle, qcsapi_retval );
                statval = 1;
        }

        return( statval );
}

static int
call_qcsapi_wifi_set_interworking( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1) {
		print_err( print, "Not enough parameters in call qcsapi WiFi set interworking, count is %d\n", argc );
		statval = 1;
	} else {
		int		 qcsapi_retval;
		const char	*the_interface = p_calling_bundle->caller_interface;
		char		*p_interworking = argv[ 0 ];

		if (strcmp( argv[ 0 ], "NULL" ) == 0)
			p_interworking = NULL;

		qcsapi_retval = qcsapi_wifi_set_interworking( the_interface, p_interworking );

		if (qcsapi_retval >= 0) {
			if (verbose_flag >= 0) {
				print_out( print, "complete\n" );
			}
		} else {
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}
	}

	return( statval );
}

static int
is_80211u_param( char *lookup_name )
{
	int retval = 1;
	unsigned int iter;

	for (iter = 0; qcsapi_80211u_params[iter] != NULL; iter++) {
		if (strcmp(qcsapi_80211u_params[iter], lookup_name) == 0) {
			retval = 0;
			break;
		}
	}

	return retval;
}

static int
call_qcsapi_wifi_get_80211u_params( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int		statval = 0;
	string_256	value;
	char		*p_buffer = NULL;
	int		qcsapi_retval;
	const char	*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output	*print = p_calling_bundle->caller_output;

	if (argc < 1) {
		print_out( print, "Usage : call_qcsapi get_80211u_params "
					"<interface> <80211u_param>");
		return 1;
	}

	if (is_80211u_param( argv[0] )) {
		print_out( print, "\n %s is not 80211u parameter",argv[0] );
		return 1;
	}

	if (strcmp( argv[ 0 ], "NULL" ) != 0)
		p_buffer = &value[ 0 ];

	qcsapi_retval = qcsapi_wifi_get_80211u_params( the_interface, argv[0], p_buffer );

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "%s\n", value );
		}
	} else {
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_set_80211u_params( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int		statval = 0;

	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 2) {
		print_err( print, "Not enough parameters in call qcsapi WiFi "
					"set_80211u_params, count is %d\n", argc );
		print_err(print, "Usage: call_qcsapi set_80211u_params "
					"<interface> <param> <value>\n");
		statval = 1;
	} else {
		int		qcsapi_retval;
		const char	*the_interface = p_calling_bundle->caller_interface;
		char		*p_11u_param = argv[ 0 ];

		if (strcmp( argv[ 0 ], "NULL" ) == 0)
			p_11u_param = NULL;

		if (is_80211u_param( argv[0] )) {
			print_err( print, "%s is not a valid 802.11u parameter\n",argv[0]);
			statval = 1;
		} else {
			if (!strcmp(argv[0], "ipaddr_type_availability")) {
				if (argc < 3) {
					print_err( print, "%s expects 2 arguments\n", argv[0]);
					return 1;
				}
			}

			qcsapi_retval = qcsapi_wifi_set_80211u_params( the_interface, p_11u_param,
									argv[1], argv[2] );

			if (qcsapi_retval >= 0) {
				if (verbose_flag >= 0) {
					print_out( print, "complete\n" );
				}
			} else {
				report_qcsapi_error( p_calling_bundle, qcsapi_retval );
				statval = 1;
			}
		}
	}

	return( statval );
}

static int
call_qcsapi_security_get_nai_realms( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int             statval = 0;
	string_4096	nai_value;
	char            *p_buffer = &nai_value[0];
	int             qcsapi_retval;
	const char      *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output   *print = p_calling_bundle->caller_output;

	qcsapi_retval = qcsapi_security_get_nai_realms( the_interface,  p_buffer );

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "%s\n", p_buffer );
		}
	} else {
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	 return( statval );
}

static int
call_qcsapi_security_add_nai_realm( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int		statval = 0;

	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 3) {
		print_err( print, "Not enough parameters in call qcsapi WiFi add_nai_realm,"
				"count is %d\n", argc );
		statval = 1;
	} else {
		int		 qcsapi_retval = 0;
		const char	*the_interface = p_calling_bundle->caller_interface;
		int		encoding;
		char		*p_nai_realm = argv[1];
		char		*p_eap_method = argv[2];

		if (*argv[0] < '0' ||  *argv[0] > '1' || strlen(argv[0]) > 1) {
			print_err( print, "invalid encoding\n");
			return ( statval );
		}

		encoding = atoi(argv[0]);

		if (strcmp( argv[ 1 ], "NULL" ) == 0)
			p_nai_realm = NULL;

		if (strcmp( argv[ 2 ], "NULL" ) == 0)
			p_eap_method = NULL;

		qcsapi_retval = qcsapi_security_add_nai_realm( the_interface,
								encoding,
								p_nai_realm,
								p_eap_method );

		if (qcsapi_retval >= 0) {
			if (verbose_flag >= 0) {
				print_out( print, "complete\n" );
			}
		} else {
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}
	}

	return( statval );
}

static int
call_qcsapi_security_del_nai_realm( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int		statval = 0;

	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1) {
		print_err( print, "Not enough parameters in call qcsapi WiFi del_nai_realm,"
					"count is %d\n", argc );
		statval = 1;
	} else {
		int		 qcsapi_retval;
		const char	*the_interface = p_calling_bundle->caller_interface;
		char		*p_nai_realm = argv[0];

		if (strcmp( argv[ 0 ], "NULL" ) == 0)
			p_nai_realm = NULL;

		qcsapi_retval = qcsapi_security_del_nai_realm( the_interface, p_nai_realm );

		if (qcsapi_retval >= 0) {
			if (verbose_flag >= 0) {
				print_out( print, "complete\n" );
			}
		} else {
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}
	}

	return( statval );
}

static int
call_qcsapi_security_get_roaming_consortium( call_qcsapi_bundle *p_calling_bundle,
					     int argc, char *argv[] )
{
	int             statval = 0;
	string_1024	roaming_value;
	char            *p_buffer = &roaming_value[ 0 ];
	int             qcsapi_retval;
	const char      *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output   *print = p_calling_bundle->caller_output;

	qcsapi_retval = qcsapi_security_get_roaming_consortium( the_interface,  p_buffer );

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "%s\n", p_buffer );
		}
	} else {
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	 return( statval );
}

static int
call_qcsapi_security_add_roaming_consortium( call_qcsapi_bundle *p_calling_bundle,
					     int argc, char *argv[] )
{
	int		statval = 0;

	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1) {
		print_err( print, "Not enough parameters in call qcsapi WiFi "
				   "add_roaming_consortium count is %d\n", argc );
		statval = 1;
	} else {
		int		 qcsapi_retval;
		const char	*the_interface = p_calling_bundle->caller_interface;
		char		*p_value = argv[0];

		if (strcmp( argv[ 0 ], "NULL" ) == 0)
			p_value = NULL;

		qcsapi_retval = qcsapi_security_add_roaming_consortium( the_interface, p_value );

		if (qcsapi_retval >= 0) {
			if (verbose_flag >= 0) {
				print_out( print, "complete\n" );
			}
		} else {
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}
	}

	return( statval );
}


static int
call_qcsapi_security_del_roaming_consortium( call_qcsapi_bundle *p_calling_bundle,
					     int argc, char *argv[] )
{
	int		statval = 0;

	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1) {
		print_err( print, "Not enough parameters in call qcsapi WiFi "
				   "del_roaming_consortium count is %d\n", argc );
		statval = 1;
	} else {
		int		 qcsapi_retval;
		const char	*the_interface = p_calling_bundle->caller_interface;
		char		*p_value = argv[0];

		if (strcmp( argv[ 0 ], "NULL" ) == 0)
			p_value = NULL;

		qcsapi_retval = qcsapi_security_del_roaming_consortium( the_interface, p_value );

		if (qcsapi_retval >= 0) {
			if (verbose_flag >= 0) {
				print_out( print, "complete\n" );
			}
		} else {
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}
	}

	return( statval );
}

static int
call_qcsapi_security_get_venue_name( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int		statval = 0;
	string_4096	venue_name;
	char            *p_venue_name = &venue_name[0];
	int		qcsapi_retval;
	const char	*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output	*print = p_calling_bundle->caller_output;

	qcsapi_retval = qcsapi_security_get_venue_name( the_interface, p_venue_name );

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "%s\n",venue_name);
		}
	} else {
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_security_add_venue_name( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 2) {
		print_err( print, "Not enough parameters in call qcsapi WiFi "
				  "add_venue_name, count is %d\n", argc);
		statval = 1;
	} else {
		int		 qcsapi_retval;
		const char	*the_interface = p_calling_bundle->caller_interface;
		char		*p_lang_code = argv[0];
		char		*p_venue_name = argv[1];

		if (strcmp( argv[0], "NULL" ) == 0)
			p_lang_code = NULL;

		if (strcmp( argv[1], "NULL" ) == 0)
			p_venue_name = NULL;

		qcsapi_retval = qcsapi_security_add_venue_name( the_interface, p_lang_code, p_venue_name );

		if (qcsapi_retval >= 0) {
			if (verbose_flag >= 0) {
				print_out( print, "complete\n" );
			}
		} else {
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}
	}

	return( statval );
}

static int
call_qcsapi_security_del_venue_name( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 2) {
		print_err( print, "Not enough parameters in call qcsapi WiFi "
				  "del_venue_name, count is %d\n", argc);
		statval = 1;
	} else {
		int		 qcsapi_retval;
		const char	*the_interface = p_calling_bundle->caller_interface;
		char		*p_lang_code = argv[0];
		char		*p_venue_name = argv[1];

		if (strcmp( argv[0], "NULL" ) == 0)
			p_lang_code = NULL;

		if (strcmp( argv[1], "NULL" ) == 0)
			p_venue_name = NULL;

		qcsapi_retval = qcsapi_security_del_venue_name( the_interface, p_lang_code, p_venue_name );

		if (qcsapi_retval >= 0) {
			if (verbose_flag >= 0) {
				print_out( print, "complete\n" );
			}
		} else {
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}
	}

	return( statval );
}

static int
call_qcsapi_security_get_oper_friendly_name( call_qcsapi_bundle *p_calling_bundle,
					     int argc, char *argv[] )
{
	int             statval = 0;
	string_4096	value;
	char            *p_value = &value[0];
	int             qcsapi_retval;
	const char      *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output   *print = p_calling_bundle->caller_output;

	qcsapi_retval = qcsapi_security_get_oper_friendly_name( the_interface, p_value );

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "%s\n", value);
		}
	} else {
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_security_add_oper_friendly_name( call_qcsapi_bundle *p_calling_bundle,
					     int argc, char *argv[] )
{
	int		statval = 0;

	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 2) {
		print_err( print, "Not enough parameters in call qcsapi WiFi "
				   "add_oper_friendly_name count is %d\n", argc);
		statval = 1;
	} else {
		int		 qcsapi_retval;
		const char	*the_interface = p_calling_bundle->caller_interface;
		char		*p_lang_code = argv[0];
		char		*p_oper_friendly_name = argv[1];

		if (strcmp( argv[0], "NULL" ) == 0)
			p_lang_code = NULL;

		if (strcmp( argv[1], "NULL" ) == 0)
			p_oper_friendly_name = NULL;

		qcsapi_retval = qcsapi_security_add_oper_friendly_name( the_interface,
									p_lang_code,
									p_oper_friendly_name );

		if (qcsapi_retval >= 0) {
			if (verbose_flag >= 0) {
				print_out( print, "complete\n" );
			}
		} else {
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}
	}

	return( statval );
}

static int
call_qcsapi_security_del_oper_friendly_name( call_qcsapi_bundle *p_calling_bundle,
					     int argc, char *argv[] )
{
	int		statval = 0;

	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 2) {
		print_err( print, "Not enough parameters in call qcsapi WiFi "
				  "del_oper_friendly_name count is %d\n", argc);
		statval = 1;
	} else {
		int		 qcsapi_retval;
		const char	*the_interface = p_calling_bundle->caller_interface;
		char		*p_lang_code = argv[0];
		char		*p_oper_friendly_name = argv[1];

		if (strcmp( argv[0], "NULL" ) == 0)
			p_lang_code = NULL;

		if (strcmp( argv[1], "NULL" ) == 0)
			p_oper_friendly_name = NULL;

		qcsapi_retval = qcsapi_security_del_oper_friendly_name( the_interface,
									p_lang_code,
									p_oper_friendly_name );

		if (qcsapi_retval >= 0) {
			if (verbose_flag >= 0) {
				print_out( print, "complete\n" );
			}
		} else {
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}
	}

	return( statval );
}


static int
call_qcsapi_security_get_hs20_conn_capab( call_qcsapi_bundle *p_calling_bundle,
					     int argc, char *argv[] )
{
	int		statval = 0;
	string_4096	value;
	char            *p_value = &value[0];
	int             qcsapi_retval;
	const char      *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output   *print = p_calling_bundle->caller_output;

	qcsapi_retval = qcsapi_security_get_hs20_conn_capab( the_interface, p_value );

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "%s\n", value);
		}
	} else {
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_security_add_hs20_conn_capab( call_qcsapi_bundle *p_calling_bundle,
					     int argc, char *argv[] )
{
	int			statval = 0;
	qcsapi_output		*print	= p_calling_bundle->caller_output;

	if (argc < 3) {
		print_err( print, "Not enough parameters in call qcsapi WiFi "
				   "add_hs20_conn_capab count is %d\n", argc);
		statval = 1;
	} else {
		int		 qcsapi_retval;
		const char	*the_interface = p_calling_bundle->caller_interface;
		char		*p_ip_proto = argv[0];
		char		*p_port_num = argv[1];
		char		*p_status = argv[2];

		if (strcmp( argv[0], "NULL" ) == 0)
			p_ip_proto = NULL;

		if (strcmp( argv[1], "NULL" ) == 0)
			p_port_num = NULL;

		if (strcmp( argv[2], "NULL" ) == 0)
			p_status = NULL;

		qcsapi_retval = qcsapi_security_add_hs20_conn_capab( the_interface,
									p_ip_proto,
									p_port_num,
									p_status );

		if (qcsapi_retval >= 0) {
			if (verbose_flag >= 0) {
				print_out( print, "complete\n" );
			}
		} else {
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}
	}

	return( statval );
}

static int
call_qcsapi_security_del_hs20_conn_capab( call_qcsapi_bundle *p_calling_bundle,
					     int argc, char *argv[] )
{
	int		statval = 0;
	qcsapi_output	*print	= p_calling_bundle->caller_output;

	if (argc < 3) {
		print_err( print, "Not enough parameters in call qcsapi WiFi "
				   "del_hs20_conn_capab count is %d\n", argc);
		statval = 1;
	} else {
		int		 qcsapi_retval;
		const char	*the_interface = p_calling_bundle->caller_interface;
		char		*p_ip_proto = argv[0];
		char		*p_port_num = argv[1];
		char		*p_status = argv[2];

		if (strcmp( argv[0], "NULL" ) == 0)
			p_ip_proto = NULL;

		if (strcmp( argv[1], "NULL" ) == 0)
			p_port_num = NULL;

		if (strcmp( argv[2], "NULL" ) == 0)
			p_status = NULL;

		qcsapi_retval = qcsapi_security_del_hs20_conn_capab( the_interface,
									p_ip_proto,
									p_port_num,
									p_status );

		if (qcsapi_retval >= 0) {
			if (verbose_flag >= 0) {
				print_out( print, "complete\n" );
			}
		} else {
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}
	}

	return ( statval );
}

static int
call_qcsapi_wifi_get_hs20_status( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int		statval = 0;
	char		hs20[2];
	char		*p_hs20 = NULL;
	int		qcsapi_retval;
	const char	*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output	*print = p_calling_bundle->caller_output;

	if (argc < 1 || strcmp( argv[ 0 ], "NULL" ) != 0)
		p_hs20 = &hs20[0];
	qcsapi_retval = qcsapi_wifi_get_hs20_status( the_interface, p_hs20 );

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "%s\n", p_hs20 );
		}
	} else {
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_set_hs20_status( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1) {
		print_err( print, "Not enough parameters in call qcsapi WiFi set hotspot, count is %d\n", argc );
		statval = 1;
	} else {
		int		 qcsapi_retval;
		const char	*the_interface = p_calling_bundle->caller_interface;
		char		*p_hs20 = argv[ 0 ];

		if (strcmp( argv[ 0 ], "NULL" ) == 0)
			p_hs20 = NULL;

		qcsapi_retval = qcsapi_wifi_set_hs20_status( the_interface, p_hs20 );

		if (qcsapi_retval >= 0) {
			if (verbose_flag >= 0) {
				print_out( print, "complete\n" );
			}
		} else {
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}
	}

	return( statval );
}

static int
call_qcsapi_wifi_set_proxy_arp( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1) {
		print_err(print, "Not enough parameters in qcsapi set_proxy_arp, count is %d\n", argc);
		statval = 1;
	} else {
		int		 qcsapi_retval;
		const char	*the_interface = p_calling_bundle->caller_interface;
		char		*proxy_arp = argv[ 0 ];

		if (atoi(argv[0]) != 0 && atoi(argv[0]) != 1) {
			print_err( print, "Invalid input for set_proxy_arp use 0 or 1\n");
			return 1;
		}

		qcsapi_retval = qcsapi_wifi_set_proxy_arp( the_interface, proxy_arp );

		if (qcsapi_retval >= 0) {
			if (verbose_flag >= 0) {
				print_out( print, "complete\n" );
			}
		} else {
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}
	}

	return( statval );
}

static int
call_qcsapi_wifi_get_proxy_arp( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int		statval = 0;
	char		proxy_arp[2];
	char		*p_proxy_arp = NULL;
	int		qcsapi_retval;
	const char	*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output	*print = p_calling_bundle->caller_output;

	if (argc > 0) {
		qcsapi_retval = -EFAULT;
	} else {
		p_proxy_arp = &proxy_arp[0];

		qcsapi_retval = qcsapi_wifi_get_proxy_arp( the_interface, p_proxy_arp );
	}

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "%s\n", p_proxy_arp );
		}
	} else {
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_get_l2_ext_filter( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int		statval = 0;
	int		qcsapi_retval;
	const char	*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output	*print = p_calling_bundle->caller_output;
	string_32       value;

	if (argc < 1) {
		print_err(print, "Not enough parameters in qcsapi get_l2_ext_filter, count is %d\n", argc);
		statval = 1;
	} else {
		char	*p_value = value;
		char	*p_param = argv[0];

		if (strcmp(p_param, "NULL") == 0)
			p_param = NULL;

		qcsapi_retval = qcsapi_wifi_get_l2_ext_filter( the_interface, p_param, p_value );

		if (qcsapi_retval >= 0) {
			if (verbose_flag >= 0) {
				print_out( print, "%s\n", p_value );
			}
		} else {
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}
	}

	return( statval );
}


static int
call_qcsapi_wifi_set_l2_ext_filter( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 2) {
		print_err(print, "Not enough parameters in qcsapi set_l2_ext_filter, count is %d\n", argc);
		statval = 1;
	} else {
		int		 qcsapi_retval;
		const char	*the_interface = p_calling_bundle->caller_interface;
		char            *p_param = argv[0];
		char            *p_value = argv[1];

		if (strcmp( argv[ 0 ], "NULL" ) == 0)
			p_param = NULL;

		if (strcmp( argv[ 1 ], "NULL" ) == 0)
			p_value = NULL;

		qcsapi_retval = qcsapi_wifi_set_l2_ext_filter( the_interface, p_param, p_value );

		if (qcsapi_retval >= 0) {
			if (verbose_flag >= 0) {
				print_out( print, "complete\n" );
			}
		} else {
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}
	}

	return( statval );
}

static int
check_hs20_param( char *lookup_name )
{
        int retval = 1;
        unsigned int iter;

	int hs20_param_count = TABLE_SIZE( qcsapi_hs20_params );

        for (iter = 0; iter < hs20_param_count; iter++) {
                if (strcmp(qcsapi_hs20_params[iter], lookup_name) == 0) {
                        retval = 0;
                        break;
                }
        }
        return retval;
}


static int
call_qcsapi_wifi_get_hs20_params( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int		statval = 0;
	string_64	value;
	char		*p_value = NULL;
	int		 qcsapi_retval;
	const char	*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output	*print = p_calling_bundle->caller_output;

	if (argc != 1) {
		print_out(print, "\n call_qcsapi get_hs20_params <interface>"
				" <hs20_param>\n");
		return 1;
	}

	p_value = &value[ 0 ];

	if (check_hs20_param( argv[0] )) {
		print_out( print, "\n %s is not hs20 parameter\n", argv[0]);
		return 1;
	}

	qcsapi_retval = qcsapi_wifi_get_hs20_params( the_interface, argv[0], p_value );

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "%s\n", p_value );
		}
	} else {
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_set_hs20_params( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 2) {
		print_err( print, "Not enough parameters in call qcsapi WiFi set_hs20_params, count is %d\n", argc );
		statval = 1;
	} else {
		int		qcsapi_retval;
		const char	*the_interface = p_calling_bundle->caller_interface;

		if (check_hs20_param( argv[0] )) {
			print_out( print, "\n %s is not hs20 parameter\n", argv[0]);
			return 1;
		}

		 if (!strcmp(argv[0], "hs20_wan_metrics")) {
			if (argc != 7) {
				print_out(print, "\n call_qcsapi set_hs20_params <interface>"
						" hs20_wan_metrics <WAN_info> <uplink_speed> "
						"<downlink_speed> <uplink_load> "
						"<downlink_load> <LMD>\n");
				return 1;
			}
		}

		if (!strcmp(argv[0], "disable_dgaf")) {
			if (argc != 2) {
				print_out(print, "\n call_qcsapi set_hs20_params "
						"<interface> disable_dgaf <0:disable 1:enable>\n");
				return 1;
			}
		}

		qcsapi_retval = qcsapi_wifi_set_hs20_params( the_interface, argv[0],
					argv[1], argv[2], argv[3], argv[4], argv[5], argv[6] );

		if (qcsapi_retval >= 0) {
			if (verbose_flag >= 0) {
				print_out( print, "complete\n" );
			}
		} else {
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}
	}

	return( statval );
}

static int
call_qcsapi_remove_11u_param( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	int	qcsapi_retval;

	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1) {
		print_err(print, "Not enough parameters in call qcsapi remove 11u_param, count is %d\n", argc);
		statval = 1;
	}

	if (is_80211u_param( argv[0] )) {
		print_out( print, "%s is not 80211u parameter\n",argv[0]);
		statval = 1;
	} else {
		char *param = argv[0];

		qcsapi_retval = qcsapi_remove_11u_param( the_interface, param );

                if (qcsapi_retval >= 0) {
                        if (verbose_flag >= 0) {
                                print_out(print, "complete\n");
                        }
                } else {
                        report_qcsapi_error( p_calling_bundle, qcsapi_retval );
                        statval = 1;
                }
	}

	return ( statval );
}

static int
call_qcsapi_remove_hs20_param( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
        int	qcsapi_retval;

	const char *the_interface = p_calling_bundle->caller_interface;
        qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1) {
                print_err(print, "Not enough parameters in call qcsapi remove hs20_param, count is %d\n", argc);
                statval = 1;
        }

	if (check_hs20_param( argv[0] )) {
		print_out( print, "%s is not hs20 parameter\n",argv[0]);
		statval = 1;
	} else {
		char *param = argv[0];

                qcsapi_retval = qcsapi_remove_hs20_param( the_interface, param );

                if (qcsapi_retval >= 0) {
                        if (verbose_flag >= 0) {
                                print_out(print, "complete\n");
                        }
                } else {
                        report_qcsapi_error( p_calling_bundle, qcsapi_retval );
                        statval = 1;
                }
	}

	return ( statval );

}

static int
call_qcsapi_wifi_get_IEEE11i_encryption_modes( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int		 statval = 0;
	char		 encryption_modes[ 36 ], *p_encryption_modes = NULL;
	int		 qcsapi_retval = 0;
	const char	*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output	*print = p_calling_bundle->caller_output;

	if (argc < 1 || strcmp( argv[ 0 ], "NULL" ) != 0)
		p_encryption_modes = &encryption_modes[ 0 ];

	qcsapi_retval = qcsapi_wifi_get_IEEE11i_encryption_modes( the_interface, p_encryption_modes );

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "%s\n", &encryption_modes[ 0 ] );
		}
	} else {
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_set_IEEE11i_encryption_modes( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int		 statval = 0;
	int		 qcsapi_retval;
	const char	*the_interface = p_calling_bundle->caller_interface;
	char		*p_encryption_mode = argv[ 0 ];
	qcsapi_output	*print = p_calling_bundle->caller_output;

	if (argc < 1) {
		print_err( print, "Not enough parameters in call qcsapi WiFi set authentication mode, count is %d\n", argc );
		statval = 1;
	} else {
		if (strcmp( argv[ 0 ], "NULL" ) == 0)
			p_encryption_mode = NULL;

		qcsapi_retval = qcsapi_wifi_set_IEEE11i_encryption_modes( the_interface, p_encryption_mode );

		if (qcsapi_retval >= 0) {
			if (verbose_flag >= 0) {
				print_out( print, "complete\n" );
			}
		} else {
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}
	}

	return( statval );
}

static int
call_qcsapi_wifi_get_IEEE11i_authentication_mode( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int		 statval = 0;
	char		 authentication_mode[ 36 ], *p_authentication_mode = NULL;
	int		 qcsapi_retval = 0;
	const char	*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output	*print = p_calling_bundle->caller_output;

	if (argc < 1 || strcmp( argv[ 0 ], "NULL" ) != 0)
		p_authentication_mode = &authentication_mode[ 0 ];

	qcsapi_retval = qcsapi_wifi_get_IEEE11i_authentication_mode( the_interface, p_authentication_mode );

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "%s\n", &authentication_mode[ 0 ] );
		}
	} else {
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_set_IEEE11i_authentication_mode( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int		 statval = 0;
	int		 qcsapi_retval;
	const char	*the_interface = p_calling_bundle->caller_interface;
	char		*p_authentication_mode = argv[ 0 ];
	qcsapi_output	*print = p_calling_bundle->caller_output;

	if (argc < 1) {
		print_err( print, "Not enough parameters in call qcsapi WiFi set authentication mode, count is %d\n", argc );
		statval = 1;
	} else {
		if (strcmp( argv[ 0 ], "NULL" ) == 0)
			p_authentication_mode = NULL;

		qcsapi_retval = qcsapi_wifi_set_IEEE11i_authentication_mode( the_interface, p_authentication_mode );

		if (qcsapi_retval >= 0) {
			if (verbose_flag >= 0) {
				print_out( print, "complete\n" );
			}
		} else {
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}
	}

	return( statval );
}

static int
call_qcsapi_wifi_get_michael_errcnt( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;
	int	qcsapi_retval;
	const char *the_interface = p_calling_bundle->caller_interface;
	uint32_t errcnt;

	qcsapi_retval = qcsapi_wifi_get_michael_errcnt(the_interface, &errcnt);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0)
			print_out(print, "%u\n", errcnt);
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wifi_get_pre_shared_key( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	char			 pre_shared_key[ 68 ], *p_pre_shared_key = NULL;
	int			 qcsapi_retval;
	const char		*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output		*print = p_calling_bundle->caller_output;
	qcsapi_unsigned_int	 the_index = p_calling_bundle->caller_generic_parameter.index;

	if (argc < 1 || strcmp( argv[ 0 ], "NULL" ) != 0)
	  p_pre_shared_key = &pre_shared_key[ 0 ];
	qcsapi_retval = qcsapi_wifi_get_pre_shared_key( the_interface, the_index, p_pre_shared_key );

	if (qcsapi_retval >= 0)
	{
		if (verbose_flag >= 0)
		{
			print_out( print, "%s\n", &pre_shared_key[ 0 ] );
		}
	}
	else
	{
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_set_pre_shared_key( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1)
	{
		print_err( print, "Not enough parameters in call qcsapi WiFi set pre-shared key, count is %d\n", argc );
		statval = 1;
	}
	else
	{
		int			 qcsapi_retval;
		const char		*the_interface = p_calling_bundle->caller_interface;
		char			*p_pre_shared_key = argv[ 0 ];
		qcsapi_unsigned_int	 the_index = p_calling_bundle->caller_generic_parameter.index;

	  /* PSK will not be NULL ... */

		if (strcmp( argv[ 0 ], "NULL" ) == 0)
		  p_pre_shared_key = NULL;
		qcsapi_retval = qcsapi_wifi_set_pre_shared_key( the_interface, the_index, p_pre_shared_key );

		if (qcsapi_retval >= 0)
		{
			if (verbose_flag >= 0)
			{
				print_out( print, "complete\n" );
			}
		}
		else
		{
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}
	}

	return( statval );
}

static int
call_qcsapi_wifi_get_psk_auth_failures(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;
	int qcsapi_retval;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_unsigned_int psk_auth_failure_cnt = 0;

	qcsapi_retval = qcsapi_wifi_get_psk_auth_failures(the_interface, &psk_auth_failure_cnt);
	if (qcsapi_retval >= 0)
	{
		if (verbose_flag >= 0)
			print_out(print, "%u\n", psk_auth_failure_cnt);
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wifi_get_key_passphrase( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	char			 passphrase[ 68 ], *p_passphrase = NULL;
	int			 qcsapi_retval;
	const char		*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output		*print = p_calling_bundle->caller_output;
	qcsapi_unsigned_int	 the_index = p_calling_bundle->caller_generic_parameter.index;

	if (argc < 1 || strcmp( argv[ 0 ], "NULL" ) != 0)
	  p_passphrase = &passphrase[ 0 ];
	qcsapi_retval = qcsapi_wifi_get_key_passphrase( the_interface, the_index, p_passphrase );

	if (qcsapi_retval >= 0)
	{
		if (verbose_flag >= 0)
		{
			print_out( print, "%s\n", &passphrase[ 0 ] );
		}
	}
	else
	{
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_set_key_passphrase( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1)
	{
		print_err( print, "Not enough parameters in call qcsapi WiFi set passphrase, count is %d\n", argc );
		statval = 1;
	}
	else
	{
		int			 qcsapi_retval;
		const char		*the_interface = p_calling_bundle->caller_interface;
		qcsapi_unsigned_int	 the_index = p_calling_bundle->caller_generic_parameter.index;
		char			*p_passphrase = argv[ 0 ];

	  /* No, you cannot has a passphrase of NULL.  Too bad !! */

		if (strcmp( argv[ 0 ], "NULL" ) == 0)
		  p_passphrase = NULL;
		qcsapi_retval = qcsapi_wifi_set_key_passphrase( the_interface, the_index, p_passphrase );

		if (qcsapi_retval >= 0)
		{
			if (verbose_flag >= 0)
			{
				print_out( print, "complete\n" );
			}
		}
		else
		{
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}
	}

	return( statval );
}

static int
call_qcsapi_wifi_get_group_key_interval( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
        int                     statval = 0;
        string_16               group_key_interval;
        char                    *p_group_key_interval = NULL;
        int                     qcsapi_retval;
        const char              *the_interface = p_calling_bundle->caller_interface;
        qcsapi_output           *print = p_calling_bundle->caller_output;

        if (argc < 1 || strcmp( argv[ 0 ], "NULL" ) != 0)
                p_group_key_interval = &group_key_interval[ 0 ];
        qcsapi_retval = qcsapi_wifi_get_group_key_interval( the_interface, p_group_key_interval );

        if (qcsapi_retval >= 0)
        {
                if (verbose_flag >= 0)
                {
                        print_out( print, "%s\n", &group_key_interval[ 0 ] );
                }
        }
	else
        {
                report_qcsapi_error( p_calling_bundle, qcsapi_retval );
                statval = 1;
        }

        return( statval );
}

static int
call_qcsapi_wifi_set_group_key_interval( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
        int     statval = 0;
        qcsapi_output *print = p_calling_bundle->caller_output;

        if (argc < 1)
        {
                print_err( print, "Not enough parameters in call qcsapi set group key interval, count is %d\n", argc);
                print_err( print, "Usage: call_qcsapi set_group_key_interval <WiFi interface> <group_key_interval>\n");
                statval = 1;
        }
	else
        {
                int                     qcsapi_retval;
                const char              *the_interface = p_calling_bundle->caller_interface;
                char                    *p_group_key_interval = argv[ 0 ];

                if (strcmp( argv[ 0 ], "NULL" ) == 0)
                        p_group_key_interval = NULL;
                qcsapi_retval = qcsapi_wifi_set_group_key_interval( the_interface, p_group_key_interval
 );

                if (qcsapi_retval >= 0)
                {
                        if (verbose_flag >= 0)
                        {
                                print_out( print, "complete\n" );
                        }
                }
                else
                {
                        report_qcsapi_error( p_calling_bundle, qcsapi_retval );
                        statval = 1;
                }
        }

	return( statval );
}

static int
call_qcsapi_wifi_get_pmf( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	int pmf_cap, *p_pmf_cap = NULL;
	int			 qcsapi_retval;
	const char		*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output		*print = p_calling_bundle->caller_output;

	if (argc < 1 || strcmp( argv[ 0 ], "NULL" ) != 0)
		p_pmf_cap = &pmf_cap;

	qcsapi_retval = qcsapi_wifi_get_pmf( the_interface, p_pmf_cap);

	if (qcsapi_retval >= 0)
	{
		if (verbose_flag >= 0)
		{
			print_out( print, "%d\n", pmf_cap );
		}
	}
	else
	{
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_set_pmf( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1)
	{
		print_err( print, "Not enough parameters in call qcsapi WiFi set pmf, count is %d\n", argc );
		statval = 1;
	}
	else
	{
		int			 qcsapi_retval;
		const char		*the_interface = p_calling_bundle->caller_interface;
		qcsapi_unsigned_int	 pmf_cap = atoi( argv[ 0 ] );

		qcsapi_retval = qcsapi_wifi_set_pmf( the_interface, pmf_cap );

		if (qcsapi_retval >= 0)
		{
			if (verbose_flag >= 0)
			{
				print_out( print, "complete\n" );
			}
		}
		else
		{
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}
	}

	return( statval );
}



static int
call_qcsapi_wifi_get_pairing_id( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	char			 pairing_id[ 33 ];
	int			 qcsapi_retval;
	const char		*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output		*print = p_calling_bundle->caller_output;

	qcsapi_retval = qcsapi_wifi_get_pairing_id( the_interface, pairing_id );

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "%s\n", pairing_id );
		}
	} else {
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_set_pairing_id( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1) {
		print_err( print, "Not enough parameters in call qcsapi WiFi set pairing ID, count is %d\n", argc );
		statval = 1;
	} else {
		int			 qcsapi_retval;
		const char		*the_interface = p_calling_bundle->caller_interface;
		char			*pairing_id = argv[ 0 ];

		qcsapi_retval = qcsapi_wifi_set_pairing_id( the_interface, pairing_id );

		if (qcsapi_retval >= 0) {
			if (verbose_flag >= 0) {
				print_out( print, "complete\n" );
			}
		} else {
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}
	}

	return( statval );
}

static int
call_qcsapi_wifi_get_pairing_enable( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	char			 pairing_enable[ 2 ];
	int			 qcsapi_retval;
	const char		*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output		*print = p_calling_bundle->caller_output;

	qcsapi_retval = qcsapi_wifi_get_pairing_enable( the_interface, pairing_enable );

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "%s\n", pairing_enable );
		}
	} else {
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_set_pairing_enable( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1) {
		print_err( print, "Not enough parameters in call qcsapi WiFi set pairing enalbe flag, count is %d\n", argc );
		statval = 1;
	} else {
		int			 qcsapi_retval;
		const char		*the_interface = p_calling_bundle->caller_interface;
		char			*pairing_enable = argv[ 0 ];

		qcsapi_retval = qcsapi_wifi_set_pairing_enable( the_interface, pairing_enable );

		if (qcsapi_retval >= 0) {
			if (verbose_flag >= 0) {
				print_out( print, "complete\n" );
			}
		} else {
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}
	}

	return( statval );
}

static int
call_qcsapi_wifi_set_txqos_sched_tbl( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1)
	{
		print_err( print, "Not enough parameters in call qcsapi set txqos sched table\n" );
		print_err( print,
	   "Usage: call_qcsapi set_txqos_sched_tbl <WiFi interface> [1|2]\n"
		);
		statval = 1;
	}
	else
	{
		const char	*the_interface = p_calling_bundle->caller_interface;
		int              index;
		string_64	cmd;

		index = atoi(argv[0]);
		sprintf(cmd, "iwpriv %s set_txqos_sched %d\n", the_interface, index);
		statval = system(cmd);

		if (statval == 0) {
			if (verbose_flag >= 0) {
				print_out( print, "complete\n" );
			}
		}
	}

	return( statval );
}

static int
call_qcsapi_wifi_get_txqos_sched_tbl( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc != 0)
	{
		print_err( print, "No need to give parameters for this command\n" );
		print_err( print, "Usage: call_qcsapi get_txqos_sched_tbl <WiFi interface>\n"
		);
		statval = 1;
	}
	else
	{
		const char	*the_interface = p_calling_bundle->caller_interface;
		string_64	cmd;

		sprintf(cmd, "iwpriv %s get_txqos_sched\n", the_interface);
		statval = system(cmd);

		if (statval == 0) {
			if (verbose_flag >= 0) {
				print_out( print, "complete\n" );
			}
		}
	}

	return( statval );
}

static int
call_qcsapi_eth_phy_power_off( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_output	*print = p_calling_bundle->caller_output;
	const char	*the_interface = p_calling_bundle->caller_interface;

	if (argc < 1) {
		print_err( print, "Not enough parameters for call_qcsapi eth_phy_power_off %d\n", argc );
		print_err( print, "Format: call_qcsapi eth_phy_power_off ifname on_off\n" );
		print_err( print, "ifname: interface name of the ethernet; on_off : 1 - off, 0 - on \n" );
		statval = 1;
	} else {
		int			 qcsapi_retval;
		int			on_off = atoi(argv[ 0 ]);

		qcsapi_retval = qcsapi_eth_phy_power_control( !!on_off, the_interface );

		if (qcsapi_retval >= 0) {
			if (verbose_flag >= 0) {
				print_out( print, "complete\n" );
			}
		} else {
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}
	}

	return( statval );
}

static int
call_qcsapi_set_aspm_l1( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int statval = 0;
	qcsapi_output	*print = p_calling_bundle->caller_output;

	if (argc < 1) {
		print_err( print, "Not enough parameters for the call_qcsapi set_aspm_l1 %d\n", argc );
		print_err( print, "Format: call_qcsapi set_aspm_l1 enable/disable [latency] \n" );
		print_err( print, "1 - enable, 0 - disable; latency(0~6) \n" );
		statval = 1;
	} else {
		int		qcsapi_retval;
		int		enable = atoi(argv[ 0 ]);
		int		latency = 0;

		if (enable && argc == 1) {
			print_err( print, "please enter latency value \n" );
			statval = 1;
			goto end;
		}

		if (enable)
			latency = atoi(argv[ 1 ]);

		qcsapi_retval = qcsapi_set_aspm_l1(enable, latency);

		if (qcsapi_retval >= 0) {
			if (verbose_flag >= 0) {
				print_out( print, "complete\n" );
			}
		} else {
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}

	}
end:
	return ( statval );
}

static int
call_qcsapi_set_l1( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1) {
		print_err( print, "Not enough parameters for the call_qcsapi set_l1 %d\n", argc );
		print_err( print, "Format: call_qcsapi set_l1 enter/exit \n" );
		print_err( print, "1 - enter, 0 - exit \n" );
		goto call_qcsapi_set_l1_error;
	}
	int qcsapi_retval;
	int enter = atoi(argv[ 0 ]);

	if (enter != 0 && enter != 1) {
		print_err( print, "parameter (%d) is not supported \n" );
		goto call_qcsapi_set_l1_error;
	}

	qcsapi_retval = qcsapi_set_l1(enter);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "complete\n" );
		}
	} else {
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		goto call_qcsapi_set_l1_error;
	}


	return ( statval );

 call_qcsapi_set_l1_error:
	statval = 1;
	return ( statval );
}

static int
call_qcsapi_wifi_get_mac_address_filtering( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_mac_address_filtering	 current_mac_address_filtering, *p_current_mac_address_filtering = NULL;
	int				 qcsapi_retval;
	const char			*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output			*print = p_calling_bundle->caller_output;

	if (argc < 1 || strcmp( argv[ 0 ], "NULL" ) != 0)
	  p_current_mac_address_filtering = &current_mac_address_filtering;
	qcsapi_retval = qcsapi_wifi_get_mac_address_filtering( the_interface, p_current_mac_address_filtering );
	if (qcsapi_retval >= 0)
	{
		if (verbose_flag >= 0)
		{
			print_out( print, "%d\n", (int) current_mac_address_filtering );
		}
	}
	else
	{
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_set_mac_address_filtering( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1)
	{
		print_err( print,
	   "Not enough parameters in call qcsapi WiFi set MAC address filtering, count is %d\n", argc
		);
		statval = 1;
	}
	else
	{
		int				 qcsapi_retval;
		const char			*the_interface = p_calling_bundle->caller_interface;
		qcsapi_mac_address_filtering	 current_mac_address_filtering =
					(qcsapi_mac_address_filtering) atoi( argv[ 0 ] );

		qcsapi_retval = qcsapi_wifi_set_mac_address_filtering( the_interface, current_mac_address_filtering );

		if (qcsapi_retval >= 0)
		{
			if (verbose_flag >= 0)
			{
				print_out( print, "complete\n" );
			}
		}
		else
		{
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}
	}

	return( statval );
}

static int
call_qcsapi_wifi_is_mac_address_authorized(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1)
	{
		print_err( print,
	   "Not enough parameters in call qcsapi WiFi is MAC address authorized, count is %d\n", argc
		);
		statval = 1;
	}
	else
	{
		const char	*the_interface = p_calling_bundle->caller_interface;
		qcsapi_mac_addr  the_mac_addr;
		int		 qcsapi_retval;
		int		 ival = 0, is_authorized = -1;

		if (strcmp( "NULL", argv[ 0 ] ) == 0)
		  qcsapi_retval = qcsapi_wifi_is_mac_address_authorized( the_interface, NULL, &is_authorized );
		else
		{
			ival = parse_mac_addr( argv[ 0 ], the_mac_addr );
			if (ival >= 0)
			  qcsapi_retval = qcsapi_wifi_is_mac_address_authorized(
				the_interface, the_mac_addr, &is_authorized
			  );
			else
			{
				print_out( print, "Error parsing MAC address %s\n", argv[ 0 ] );
				statval = 1;
			}
		}

		if (ival >= 0)
		{
			if (qcsapi_retval >= 0)
			{
				if (verbose_flag >= 0)
				{
					print_out( print, "%d\n", is_authorized );
				}
			}
			else
			{
				report_qcsapi_error( p_calling_bundle, qcsapi_retval );
				statval = 1;
			}
		}
	}

	return( statval );
}

static int
call_qcsapi_wifi_get_authorized_mac_addresses(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	int		 qcsapi_retval;
	const char	*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output	*print = p_calling_bundle->caller_output;
	char		*authorized_mac_addresses = NULL;
	unsigned int	 sizeof_authorized_mac_addresses = 126;
	int		 ok_to_proceed = 1;			/* tracks malloc failures */

	if (argc > 0)
	{
		if (strcmp( "NULL", argv[ 0 ] ) != 0)
		{
			sizeof_authorized_mac_addresses = (unsigned int) atoi( argv[ 0 ] );
		}
		else
		  sizeof_authorized_mac_addresses = 0;
	}

	if (sizeof_authorized_mac_addresses > 0)
	{
		authorized_mac_addresses = malloc( sizeof_authorized_mac_addresses );
		if (authorized_mac_addresses == NULL)
		{
			print_err( print, "Failed to allocate %u chars\n", sizeof_authorized_mac_addresses );
			ok_to_proceed = 0;
			statval = 1;
		}
	}

	if (ok_to_proceed)
	{
		qcsapi_retval = qcsapi_wifi_get_authorized_mac_addresses(
			the_interface, authorized_mac_addresses, sizeof_authorized_mac_addresses
		);

		if (qcsapi_retval >= 0)
		{
			if (verbose_flag >= 0)
			{
				print_out( print, "%s\n", authorized_mac_addresses );
			}
		}
		else
		{
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}

		if (authorized_mac_addresses != NULL)
			free( authorized_mac_addresses );
	}

	return( statval );
}

static int
call_qcsapi_wifi_get_denied_mac_addresses( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	int		 qcsapi_retval;
	const char	*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output	*print = p_calling_bundle->caller_output;
	char		*denied_mac_addresses = NULL;
	unsigned int	 sizeof_denied_mac_addresses = 126;
	int		 ok_to_proceed = 1;			/* tracks malloc failures */

	if (argc > 0)
	{
		if (strcmp( "NULL", argv[ 0 ] ) != 0)
		{
			sizeof_denied_mac_addresses = (unsigned int) atoi( argv[ 0 ] );
		}
		else
		  sizeof_denied_mac_addresses = 0;
	}

	if (sizeof_denied_mac_addresses > 0)
	{
		denied_mac_addresses = malloc( sizeof_denied_mac_addresses );
		if (denied_mac_addresses == NULL)
		{
			print_err( print, "Failed to allocate %u chars\n", sizeof_denied_mac_addresses );
			ok_to_proceed = 0;
			statval = 1;
		}
	}

	if (ok_to_proceed)
	{
		qcsapi_retval = qcsapi_wifi_get_denied_mac_addresses(
			the_interface, denied_mac_addresses, sizeof_denied_mac_addresses
		);

		if (qcsapi_retval >= 0)
		{
			if (verbose_flag >= 0)
			{
				print_out( print, "%s\n", denied_mac_addresses );
			}
		}
		else
		{
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}

		if (denied_mac_addresses != NULL)
			free( denied_mac_addresses );
	}

	return( statval );
}

static int
call_qcsapi_wifi_authorize_mac_address( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1)
	{
		print_err( print,
	   "Not enough parameters in call qcsapi WiFi authorize MAC address,count is %d\n", argc
		);
		statval = 1;
	}
	else
	{
		const char	*the_interface = p_calling_bundle->caller_interface;
		qcsapi_mac_addr  the_mac_addr;
		int		 qcsapi_retval;
		int		 ival = 0;

		if (strcmp( "NULL", argv[ 0 ] ) == 0)
		  qcsapi_retval = qcsapi_wifi_authorize_mac_address( the_interface, NULL );
		else
		{
			ival = parse_mac_addr( argv[ 0 ], the_mac_addr );
			if (ival >= 0)
			  qcsapi_retval = qcsapi_wifi_authorize_mac_address(
				the_interface, the_mac_addr
			  );
			else
			{
				print_out( print, "Error parsing MAC address %s\n", argv[ 0 ] );
				statval = 1;
			}
		}

		if (ival >= 0)
		{
			if (qcsapi_retval >= 0)
			{
				if (verbose_flag >= 0)
				{
					print_out( print, "complete\n" );
				}
			}
			else
			{
				report_qcsapi_error( p_calling_bundle, qcsapi_retval );
				statval = 1;
			}
		}
	}

	return( statval );
}

static int
call_qcsapi_wifi_deny_mac_address( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1)
	{
		print_err( print,
	   "Not enough parameters in call qcsapi WiFi deny MAC address, count is %d\n", argc
		);
		statval = 1;
	}
	else
	{
		const char	*the_interface = p_calling_bundle->caller_interface;
		qcsapi_mac_addr  the_mac_addr;
		int		 qcsapi_retval;
		int		 ival = 0;

		if (strcmp( "NULL", argv[ 0 ] ) == 0)
		  qcsapi_retval = qcsapi_wifi_deny_mac_address( the_interface, NULL );
		else
		{
			ival = parse_mac_addr( argv[ 0 ], the_mac_addr );
			if (ival >= 0)
			  qcsapi_retval = qcsapi_wifi_deny_mac_address(
				the_interface, the_mac_addr
			  );
			else
			{
				print_out( print, "Error parsing MAC address %s\n", argv[ 0 ] );
				statval = 1;
			}
		}

		if (ival >= 0)
		{
			if (qcsapi_retval >= 0)
			{
				if (verbose_flag >= 0)
				{
					print_out( print, "complete\n" );
				}
			}
			else
			{
				report_qcsapi_error( p_calling_bundle, qcsapi_retval );
				statval = 1;
			}
		}
	}

	return( statval );
}


static int
call_qcsapi_wifi_remove_mac_address( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1)
	{
		print_err( print,
	   "Not enough parameters in call qcsapi WiFi remove MAC address, count is %d\n", argc
		);
		statval = 1;
	}
	else
	{
		const char	*the_interface = p_calling_bundle->caller_interface;
		qcsapi_mac_addr  the_mac_addr;
		int		 qcsapi_retval;
		int		 ival = 0;

		if (strcmp( "NULL", argv[ 0 ] ) == 0)
		  qcsapi_retval = qcsapi_wifi_remove_mac_address( the_interface, NULL );
		else
		{
			ival = parse_mac_addr( argv[ 0 ], the_mac_addr );
			if (ival >= 0)
			  qcsapi_retval = qcsapi_wifi_remove_mac_address(
				the_interface, the_mac_addr
			  );
			else
			{
				print_out( print, "Error parsing MAC address %s\n", argv[ 0 ] );
				statval = 1;
			}
		}

		if (ival >= 0)
		{
			if (qcsapi_retval >= 0)
			{
				if (verbose_flag >= 0)
				{
					print_out( print, "complete\n" );
				}
			}
			else
			{
				report_qcsapi_error( p_calling_bundle, qcsapi_retval );
				statval = 1;
			}
		}
	}

	return( statval );
}

static int
call_qcsapi_wifi_clear_mac_address_filters( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	int				 qcsapi_retval;
	const char			*the_interface = p_calling_bundle->caller_interface;

	qcsapi_retval = qcsapi_wifi_clear_mac_address_filters( the_interface );

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "complete\n" );
		}
	} else {
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_set_mac_address_reserve(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	qcsapi_output *print = p_calling_bundle->caller_output;
	const char *the_interface = p_calling_bundle->caller_interface;

	int qcsapi_retval;

	if (argc < 1) {
		print_err(print,
			"Not enough parameters in call qcsapi WiFi reserve MAC address, count is %d\n",
				argc);
		return 1;
	} else if (argc == 1) {
		qcsapi_retval = qcsapi_wifi_set_mac_address_reserve(the_interface, argv[0], "");
	} else {
		qcsapi_retval = qcsapi_wifi_set_mac_address_reserve(the_interface, argv[0], argv[1]);
	}

	if (qcsapi_retval < 0) {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		return 1;
	}

	if (verbose_flag >= 0)
		print_out(print, "complete\n");

	return 0;
}

static int
call_qcsapi_wifi_get_mac_address_reserve(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	qcsapi_output *print = p_calling_bundle->caller_output;
	const char *the_interface = p_calling_bundle->caller_interface;
	string_256 buf;
	int qcsapi_retval;

	qcsapi_retval = qcsapi_wifi_get_mac_address_reserve(the_interface, buf);
	if (qcsapi_retval < 0) {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		return 1;
	}

	print_out(print, "%s", buf);

	return 0;
}

static int
call_qcsapi_wifi_clear_mac_address_reserve(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	qcsapi_output *print = p_calling_bundle->caller_output;
	const char *the_interface = p_calling_bundle->caller_interface;

	int qcsapi_retval;

	qcsapi_retval = qcsapi_wifi_clear_mac_address_reserve(the_interface);

	if (qcsapi_retval < 0) {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		return 1;
	}

	if (verbose_flag >= 0)
		print_out(print, "complete\n");

	return 0;
}

static int
call_qcsapi_wifi_backoff_fail_max( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1)
	{
		print_err( print,
	   "Not enough parameters in call qcsapi backoff fail max, count is %d\n", argc
		);
		statval = 1;
	}
	else
	{
		const char	*the_interface = p_calling_bundle->caller_interface;
		int		 qcsapi_retval;
		int		 backoff_fail_max = atoi( argv[ 0 ] );

		qcsapi_retval = qcsapi_wifi_backoff_fail_max( the_interface, backoff_fail_max );

		if (qcsapi_retval >= 0)
		{
			if (verbose_flag >= 0)
			{
				print_out( print, "complete\n" );
			}
		}
		else
		{
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}
	}

	return( statval );
}

static int
call_qcsapi_wifi_backoff_timeout( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1)
	{
		print_err( print,
	   "Not enough parameters in call qcsapi backoff timeout, count is %d\n", argc
		);
		statval = 1;
	}
	else
	{
		const char	*the_interface = p_calling_bundle->caller_interface;
		int		 qcsapi_retval;
		int		 backoff_timeout = atoi( argv[ 0 ] );

		qcsapi_retval = qcsapi_wifi_backoff_timeout( the_interface, backoff_timeout );

		if (qcsapi_retval >= 0)
		{
			if (verbose_flag >= 0)
			{
				print_out( print, "complete\n" );
			}
		}
		else
		{
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}
	}

	return( statval );
}

static int
call_qcsapi_wps_registrar_report_button_press( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	const char	*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output	*print = p_calling_bundle->caller_output;
	int		 qcsapi_retval = qcsapi_wps_registrar_report_button_press( the_interface );

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "complete\n" );
		}
	} else {
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wps_registrar_report_pin( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1) {
		print_err( print, "registrar report pin: required WPS PIN not present\n" );
		statval = 1;
	}
	else {
		const char	*the_interface = p_calling_bundle->caller_interface;
		const char	*p_wps_pin = NULL;
		int		 qcsapi_retval;

		if (strcmp( argv[ 0 ], "NULL" ) != 0) {
			p_wps_pin = argv[ 0 ];
		}

		qcsapi_retval = qcsapi_wps_registrar_report_pin( the_interface, p_wps_pin );

		if (qcsapi_retval >= 0) {
			if (verbose_flag >= 0) {
				print_out( print, "complete\n" );
			}
		} else {
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}
	}

	return( statval );
}

static int
call_qcsapi_wps_registrar_get_pp_devname(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int		 statval = 0;
	int		 qcsapi_retval;
	const char	*the_interface = p_calling_bundle->caller_interface;
	string_128	 pp_devname = "";
	char		*p_pp_devname = NULL;
	qcsapi_output	*print = p_calling_bundle->caller_output;
	int		 blacklist = 0;

	if (argc == 1 && strcmp(argv[0], "blacklist") == 0) {
		blacklist = 1;
	}
	if (argc >= 1 && strcmp(argv[0], "NULL") != 0) {
		p_pp_devname = &pp_devname[0];
	}

	qcsapi_retval = qcsapi_wps_registrar_get_pp_devname(the_interface, blacklist, p_pp_devname);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out(print, "%s\n", p_pp_devname);
		}
	} else {
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wps_registrar_set_pp_devname(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int		 statval = 0;
	int		 qcsapi_retval;
	const char	*the_interface = p_calling_bundle->caller_interface;
	char		*p_pp_devname = NULL;
	qcsapi_output	*print = p_calling_bundle->caller_output;
	uint32_t	wps_pp_status;
	int		update_blacklist = 0;

	if (argc == 1) {
		p_pp_devname = strcmp(argv[0], "NULL") == 0 ? NULL : argv[0];
	} else if (argc == 2 && strcmp(argv[0], "blacklist") == 0) {
		update_blacklist = 1;
		p_pp_devname = strcmp(argv[1], "NULL") == 0 ? NULL : argv[1];
	} else {
		print_err(print, "WPS Registrar Set PP Devname: \n"
				"setting white-list: call_qcsapi registrar_set_pp_devname <device name list>\n"
				"setting black-list: call_qcsapi registrar_set_pp_devname blacklist <device name list>\n");
		return 0;
	}

	qcsapi_retval = qcsapi_wps_get_access_control( the_interface, &wps_pp_status );
	if (qcsapi_retval >= 0) {
		if (wps_pp_status == 0) {
			print_err(print, "enable WPS Pairing Protection before setting device name list\n");
			return 1;
		}
	}

	if (qcsapi_retval >= 0)
		qcsapi_retval = qcsapi_wps_registrar_set_pp_devname(the_interface, update_blacklist, p_pp_devname);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out(print, "complete\n");
		}
	} else {
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return statval;
}


static int
call_qcsapi_wps_enrollee_report_button_press( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	int		 ival = 0;
	qcsapi_mac_addr	 local_bssid = { 0, 0, 0, 0, 0, 0 };
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc > 0) {
		/*
		 * Interpret BSSID parameter of "any" as direction to pass BSSID of all zeros to the API -
		 * so the WPS process will associate with any registrar.
		 */
		if (strcasecmp( argv[ 0 ], "any" ) != 0) {
			ival = parse_mac_addr( argv[ 0 ], local_bssid );

			if (ival < 0) {
				print_out( print, "Error parsing MAC address %s\n", argv[ 0 ] );
				statval = 1;
			}
		}
	}

	if (ival >= 0) {
		const char	*the_interface = p_calling_bundle->caller_interface;
		int		 qcsapi_retval = qcsapi_wps_enrollee_report_button_press(the_interface,
											 local_bssid);
		if (qcsapi_retval >= 0) {
			if (verbose_flag >= 0) {
				print_out( print, "complete\n" );
			}
		} else {
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}
	}

	return( statval );
}

static int
call_qcsapi_wps_enrollee_report_pin( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1) {
		print_err( print, "enrollee report pin: required WPS PIN not present\n" );
		statval = 1;
	} else {
		int		 qcsapi_retval = 0;
		const char	*the_interface = p_calling_bundle->caller_interface;
		qcsapi_mac_addr	 local_bssid = { 0, 0, 0, 0, 0, 0 };
		const char	*p_wps_pin = NULL;
		int		 ival = 0;
		int		 pin_argv_index = 0;

		if (argc > 1) {
			if (strcasecmp( argv[ 0 ], "any" ) != 0) {
				ival = parse_mac_addr( argv[ 0 ], local_bssid );
			}

			if (ival < 0) {
				print_out( print, "Error parsing MAC address %s\n", argv[ 0 ] );
				statval = 1;
			} else {
				pin_argv_index = 1;
			}
		}

		if (ival >= 0) {
			if (strcmp( argv[ pin_argv_index ], "NULL" ) != 0) {
				p_wps_pin = argv[ pin_argv_index ];
			}

			qcsapi_retval = qcsapi_wps_enrollee_report_pin( the_interface,
									local_bssid,
									p_wps_pin );
			if (qcsapi_retval >= 0) {
				if (verbose_flag >= 0) {
					print_out( print, "complete\n" );
				}
			} else {
				report_qcsapi_error( p_calling_bundle, qcsapi_retval );
				statval = 1;
			}
		}
	}

	return( statval );
}

static int
call_qcsapi_wps_enrollee_generate_pin( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	int		 ival = 0;
	qcsapi_mac_addr	 local_bssid = { 0, 0, 0, 0, 0, 0 };
	char		 generated_pin[ QCSAPI_WPS_MAX_PIN_LEN + 1 ];
	char		*p_generated_pin = NULL;
	qcsapi_output	*print = p_calling_bundle->caller_output;

	if (argc > 0) {
		if (argc < 2 || strcmp( argv[ 1 ], "NULL" ) != 0) {
			p_generated_pin = &generated_pin[ 0 ];
		}
		/*
		 * Interpret BSSID parameter of "any" as direction to pass BSSID of all zeros to the API -
		 * so the WPS process will associate with any registrar.
		 */
		if (strcasecmp( argv[ 0 ], "any" ) != 0) {
			ival = parse_mac_addr( argv[ 0 ], local_bssid );

			if (ival < 0) {
				print_out( print, "Error parsing MAC address %s\n", argv[ 0 ] );
				statval = 1;
			}
		}
	} else {
		p_generated_pin = &generated_pin[ 0 ];
	}

	if (ival >= 0) {
		const char	*the_interface = p_calling_bundle->caller_interface;
		int		 qcsapi_retval = qcsapi_wps_enrollee_generate_pin(the_interface,
										  local_bssid,
										  p_generated_pin);
		if (qcsapi_retval >= 0) {
			if (verbose_flag >= 0) {
				print_out( print, "%s\n", &generated_pin[0 ] );
			}
		} else {
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}
	}

	return( statval );
}

static int
call_qcsapi_wps_get_ap_pin(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval;
	const char *iface = p_calling_bundle->caller_interface;
	char generated_pin[QCSAPI_WPS_MAX_PIN_LEN + 1];
	qcsapi_output *print = p_calling_bundle->caller_output;
	int force_regenerate = 0;

	if (argc == 1) {
		force_regenerate = atoi(argv[0]);
	} else if (argc > 1) {
		print_err(print, "Too many arguments for wps_get_ap_pin\n");
		return 1;
	}

	if (force_regenerate != 0 && force_regenerate != 1) {
		print_err(print,
			  "Invalid parameter for force regenerate option: \"%s\" - must be 0 or 1",
			  argv[0]);
		return 1;
	}

	qcsapi_retval = qcsapi_wps_get_ap_pin(iface, generated_pin, force_regenerate);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out(print, "%s\n", generated_pin);
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static void local_set_wps_ap_pin_usage(qcsapi_output *print, int out)
{
	if (!out) {
		print_out(print, "usage: call_qscapi set_wps_ap_pin <AP PIN>\n"
				"AP PIN: 8bit or 4 bit digits\n");
	} else {
		print_err(print, "usage: call_qscapi set_wps_ap_pin <AP PIN>\n"
				"AP PIN: 8bit or 4 bit digits\n");
	}
}

static int
call_qcsapi_wps_set_ap_pin(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval;
	const char *iface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;
	char wps_pin[2 * QCSAPI_WPS_MAX_PIN_LEN] = {0};

	if (argc <= 0) {
		local_set_wps_ap_pin_usage(print, 1);
		return 1;
	}

	strncpy(wps_pin, argv[0], sizeof(wps_pin));

	qcsapi_retval = qcsapi_wps_set_ap_pin(iface, wps_pin);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out(print, "complete\n");
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wps_save_ap_pin(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval;
	const char *iface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc != 0) {
		print_err(print, "usage: call_qscapi save_wps_ap_pin\n");
		return 1;
	}

	qcsapi_retval = qcsapi_wps_save_ap_pin(iface);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out(print, "complete\n");
		}
	} else {
		if (qcsapi_retval == -qcsapi_parameter_not_found)
			print_err(print, "no ap PIN exists, set or generate one\n");
		else
			report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wps_enable_ap_pin(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval;
	const char *iface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;
	int enable;

	if (argc != 1) {
		print_err(print, "usage: call_qscapi enable_wps_ap_pin [1 | 0]\n");
		return 1;
	}

	enable = atoi(argv[0]);
	if (strlen(argv[0]) > 1 || !isdigit(*argv[0]) || (enable != 0 && enable != 1)) {
		print_err(print, "usage: call_qscapi enable_wps_ap_pin [1 | 0]\n");
		return 1;
	}

	qcsapi_retval = qcsapi_wps_enable_ap_pin(iface, enable);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out(print, "complete\n");
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wps_generate_random_pin(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval;
	const char *iface = p_calling_bundle->caller_interface;
	char generated_pin[QCSAPI_WPS_MAX_PIN_LEN + 1];
	qcsapi_output *print = p_calling_bundle->caller_output;

	qcsapi_retval = qcsapi_wps_get_sta_pin(iface, generated_pin);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out(print, "%s\n", generated_pin);
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

#define  WPS_GET_STATE_MAX_LEN	128

static int
call_qcsapi_wps_get_state( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	const char		*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output		*print = p_calling_bundle->caller_output;
	int			 qcsapi_retval;
	qcsapi_unsigned_int	 message_len = WPS_GET_STATE_MAX_LEN;
	char			 wps_state[ WPS_GET_STATE_MAX_LEN ] = "";
	char			*p_wps_state = &wps_state[ 0 ];

	if (argc > 0) {
		if (strcmp( argv[ 0 ], "NULL" ) == 0 ) {
			p_wps_state = NULL;
		}
		else if (isdigit( argv[ 0 ][ 0 ] )) {
			message_len = atoi( argv[ 0 ] );

			if (message_len > WPS_GET_STATE_MAX_LEN) {
				message_len = WPS_GET_STATE_MAX_LEN;
			}
		}
	}

	qcsapi_retval = qcsapi_wps_get_state( the_interface, p_wps_state, message_len );

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "%s\n", p_wps_state );
		}
	}
	else {
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

#define  WPA_GET_STATUS_MAX_LEN	32
#define  MAC_ADDR_STR_LEN		17
static int
call_qcsapi_wifi_get_wpa_status( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int			statval = 0;
	const char		*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output		*print = p_calling_bundle->caller_output;
	int			 qcsapi_retval;
	qcsapi_unsigned_int	 message_len = WPA_GET_STATUS_MAX_LEN;
	char			 wpa_status[ WPA_GET_STATUS_MAX_LEN ] = "";
	char			*p_wpa_status = &wpa_status[ 0 ];
	char			mac_addr[MAC_ADDR_STR_LEN + 1] = {0};

	if (argc > 0) {
		if (argc == 2) {
			if (isdigit( argv[ 1 ][ 0 ] )) {
				message_len = atoi( argv[ 1 ] );

				if (message_len > WPA_GET_STATUS_MAX_LEN) {
					message_len = WPA_GET_STATUS_MAX_LEN;
				}
			}
		}

		if (strnlen( argv[ 0 ], MAC_ADDR_STR_LEN + 1 ) == MAC_ADDR_STR_LEN ) {
			strcpy( mac_addr, argv[ 0 ] );
		} else {
			print_out( print, "mac address input error \n");
			return( statval );
		}
	}

	qcsapi_retval = qcsapi_wifi_get_wpa_status( the_interface, p_wpa_status, mac_addr, message_len );

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "%s\n", p_wpa_status );
		}
	}
	else {
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_get_auth_state( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int			statval = 0;
	const char		*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output		*print = p_calling_bundle->caller_output;
	int			 qcsapi_retval;
	char			 mac_addr[MAC_ADDR_STR_LEN + 1] = {0};
	int			 auth_state = 0;

	if (argc > 0) {
		if (strnlen( argv[ 0 ], (MAC_ADDR_STR_LEN + 1) ) == MAC_ADDR_STR_LEN ) {
			strcpy( mac_addr, argv[ 0 ] );
		} else {
			print_out( print, "Mac address input is invalid!\n" );
			return( statval );
		}
	} else {
		print_out( print, "Mac address should be input!\n" );
		return( statval );
	}

	qcsapi_retval = qcsapi_wifi_get_auth_state( the_interface, mac_addr, &auth_state );

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "%d\n", auth_state );
		}
	} else {
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_get_disconn_info(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int	statval = 0;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;
	int	qcsapi_retval;
	qcsapi_disconn_info info;

	memset(&info, 0, sizeof(info));
	qcsapi_retval = qcsapi_wifi_get_disconn_info(the_interface, &info);

	if (qcsapi_retval >= 0) {
		print_out( print, "association\t%d\n"
				"disconnect\t%d\n"
				"sequence\t%d\n"
				"uptime\t%d\n", info.asso_sta_count, info.disconn_count, info.sequence,
				info.up_time);
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wifi_reset_disconn_info(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int	statval = 0;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;
	int	qcsapi_retval;
	qcsapi_disconn_info info;

	memset(&info, 0, sizeof(info));
	info.resetflag = 1;
	qcsapi_retval = qcsapi_wifi_get_disconn_info(the_interface, &info);

	if (qcsapi_retval >= 0) {
		print_out( print, "Reset complete\n");
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wps_get_configured_state(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;
	int qcsapi_retval;
	qcsapi_unsigned_int message_len = WPS_GET_STATE_MAX_LEN;
	char wps_state[WPS_GET_STATE_MAX_LEN] = "";
	char *p_wps_state = &wps_state[0];

	if (argc > 0) {
		if (strcmp(argv[0], "NULL") == 0) {
			p_wps_state = NULL;
		} else if (isdigit(argv[0][0])) {
			message_len = atoi(argv[0]);
			if (message_len > WPS_GET_STATE_MAX_LEN) {
				message_len = WPS_GET_STATE_MAX_LEN;
			}
		}
	}

	qcsapi_retval = qcsapi_wps_get_configured_state(the_interface, p_wps_state, message_len);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "%s\n", p_wps_state);
		}
	}
	else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wps_get_runtime_state(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;
	int qcsapi_retval;
	qcsapi_unsigned_int message_len = WPS_GET_STATE_MAX_LEN;
	char wps_state[WPS_GET_STATE_MAX_LEN] = "";
	char *p_wps_state = &wps_state[0];

	if (argc > 0) {
		if (strcmp(argv[0], "NULL") == 0) {
			p_wps_state = NULL;
		} else if (isdigit(argv[0][0])) {
			message_len = atoi(argv[0]);
			if (message_len > WPS_GET_STATE_MAX_LEN) {
				message_len = WPS_GET_STATE_MAX_LEN;
			}
		}
	}

	qcsapi_retval = qcsapi_wps_get_runtime_state(the_interface, p_wps_state, message_len);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "%s\n", p_wps_state);
		}
	}
	else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wps_allow_pbc_overlap(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int	statval = 0;
	int		qcsapi_retval;
	const char	*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output	*print = p_calling_bundle->caller_output;
	int	allow = !!atoi(argv[0]);

	qcsapi_retval = qcsapi_wps_allow_pbc_overlap(the_interface, allow);
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "complete\n");
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return( statval );
}


static int
call_qcsapi_wps_get_allow_pbc_overlap_status(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval;
	int status = -1;
	const char *iface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;

	qcsapi_retval = qcsapi_wps_get_allow_pbc_overlap_status(iface, &status);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out(print, "%d\n", status);
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}


#define WPS_GET_CFG_MAX_LEN 100

static int
call_qcsapi_wps_get_param(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	const char *the_interface = p_calling_bundle->caller_interface;
	int qcsapi_retval;
	qcsapi_unsigned_int message_len = WPS_GET_CFG_MAX_LEN;
	qcsapi_wps_param_type wps_cfg_str_id;
	qcsapi_output *print = p_calling_bundle->caller_output;
	char wps_cfg_str[WPS_GET_CFG_MAX_LEN] = "";

	if (argc > 0) {
		if (strcmp(argv[0], "uuid") == 0) {
			wps_cfg_str_id = qcsapi_wps_uuid;
		} else if(strcmp(argv[0], "os_version") == 0){
			wps_cfg_str_id = qcsapi_wps_os_version;
		} else if(strcmp(argv[0], "device_name") == 0){
			wps_cfg_str_id = qcsapi_wps_device_name;
		} else if(strcmp(argv[0], "config_methods") == 0){
			wps_cfg_str_id = qcsapi_wps_config_methods;
		} else if(strcmp(argv[0], "ap_setup_locked") == 0){
			wps_cfg_str_id = qcsapi_wps_ap_setup_locked;
		} else if(strcmp(argv[0], "last_config_error") == 0){
			wps_cfg_str_id = qcsapi_wps_last_config_error;
		} else if(strcmp(argv[0], "registrar_number") == 0){
			wps_cfg_str_id = qcsapi_wps_registrar_number;
		}else if(strcmp(argv[0], "registrar_established") == 0){
			wps_cfg_str_id = qcsapi_wps_registrar_established;
		}else if (strcmp(argv[0], "force_broadcast_uuid") == 0) {
			wps_cfg_str_id = qcsapi_wps_force_broadcast_uuid;
		}else if (strcmp(argv[0], "ap_pin_fail_method") == 0) {
			wps_cfg_str_id = qcsapi_wps_ap_pin_fail_method;
		}else if (strcmp(argv[0], "auto_lockdown_max_retry") == 0) {
			wps_cfg_str_id = qcsapi_wps_auto_lockdown_max_retry;
		}else if (strcmp(argv[0], "auto_lockdown_fail_num") == 0) {
			wps_cfg_str_id = qcsapi_wps_auto_lockdown_fail_num;
		}else if (strcmp(argv[0], "wps_vendor_spec") == 0) {
			wps_cfg_str_id = qcsapi_wps_vendor_spec;
		}else if (strcmp(argv[0], "last_wps_client") == 0) {
			wps_cfg_str_id = qcsapi_wps_last_successful_client;
		}else if (strcmp(argv[0], "last_wps_client_devname") == 0) {
			wps_cfg_str_id = qcsapi_wps_last_successful_client_devname;
		} else if (strcmp(argv[0], "serial_number") == 0) {
			wps_cfg_str_id = qcsapi_wps_serial_number;
		} else if (strcmp(argv[0], "manufacturer") == 0) {
			wps_cfg_str_id = qcsapi_wps_manufacturer;
		} else if (strcmp(argv[0], "model_name") == 0) {
			wps_cfg_str_id = qcsapi_wps_model_name;
		} else if (strcmp(argv[0], "model_number") == 0) {
			wps_cfg_str_id = qcsapi_wps_model_number;
		} else if (strcmp(argv[0], "pbc_in_m1") == 0) {
			wps_cfg_str_id = qcsapi_wps_pbc_in_m1;
		} else {
			print_err(print, "wps cfg string ID input error! \n");
			return 1;
		}
	}else{
		print_err(print, "please input wps cfg string ID\n");
		return 1;
	}

	qcsapi_retval = qcsapi_wps_get_param(the_interface, wps_cfg_str_id, wps_cfg_str, message_len);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out(print, "%s\n", wps_cfg_str);
		}
	}
	else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wps_set_param(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	const char *the_interface = p_calling_bundle->caller_interface;
	int qcsapi_retval;
	qcsapi_wps_param_type wps_cfg_str_id;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc >= 2) {
		if (strcmp(argv[0], "ap_pin") == 0) {
			wps_cfg_str_id = qcsapi_wps_ap_pin;
		} else if (strcmp(argv[0], "config_methods") == 0) {
			wps_cfg_str_id = qcsapi_wps_config_methods;
		} else if (strcmp(argv[0], "setup_lock") == 0) {
			wps_cfg_str_id = qcsapi_wps_ap_setup_locked;
		} else if (strcmp(argv[0], "ap_setup_locked") == 0) {
			wps_cfg_str_id = qcsapi_wps_ap_setup_locked;
		} else if (strcmp(argv[0], "uuid") == 0) {
			wps_cfg_str_id = qcsapi_wps_uuid;
		} else if (strcmp(argv[0], "force_broadcast_uuid") == 0) {
			wps_cfg_str_id = qcsapi_wps_force_broadcast_uuid;
		} else if(strcmp(argv[0], "device_name") == 0){
			wps_cfg_str_id = qcsapi_wps_device_name;
		} else if (strcmp(argv[0], "ap_pin_fail_method") == 0) {
			wps_cfg_str_id = qcsapi_wps_ap_pin_fail_method;
		} else if (strcmp(argv[0], "auto_lockdown_max_retry") == 0) {
			wps_cfg_str_id = qcsapi_wps_auto_lockdown_max_retry;
		} else if (strcmp(argv[0], "wps_vendor_spec") == 0) {
			wps_cfg_str_id = qcsapi_wps_vendor_spec;
		} else if (strcmp(argv[0], "serial_number") == 0) {
			wps_cfg_str_id = qcsapi_wps_serial_number;
		} else if (strcmp(argv[0], "manufacturer") == 0) {
			wps_cfg_str_id = qcsapi_wps_manufacturer;
		} else if (strcmp(argv[0], "model_name") == 0) {
			wps_cfg_str_id = qcsapi_wps_model_name;
		} else if (strcmp(argv[0], "model_number") == 0) {
			wps_cfg_str_id = qcsapi_wps_model_number;
		} else if (strcmp(argv[0], "pbc_in_m1") == 0) {
			wps_cfg_str_id = qcsapi_wps_pbc_in_m1;
		} else {
			print_err(print, "WPS param type string input error or not supported!\n");
			return statval;
		}
	} else {
		print_err(print, "Input WPS param type string and param value!\n");
		return statval;
	}

	qcsapi_retval = qcsapi_wps_set_param(the_interface, wps_cfg_str_id, argv[1]);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out(print, "complete\n");
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wps_set_configured_state(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	const char *interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;
	uint8_t new_value;
	int qcsapi_retval;

	if (argc < 1) {
		print_err( print, "New WPS state argument required");
		return 1;
	}

	new_value = (uint8_t) atoi(argv[0]);

	qcsapi_retval = qcsapi_wps_set_configured_state(interface, new_value);
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "complete\n");
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		return 1;
	}

	return 0;
}

static int
call_qcsapi_wifi_set_dwell_times( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	const char	*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output	*print = p_calling_bundle->caller_output;
	int		qcsapi_retval;
	unsigned int	max_dwell_time_active_chan;
	unsigned int	min_dwell_time_active_chan;
	unsigned int	max_dwell_time_passive_chan;
	unsigned int	min_dwell_time_passive_chan;
	int		statval = 0;

	if (argc < 4) {
		print_err( print, "STA Set Dwell Times requires 4 dwell times\n" );
		return(1);
	}

	max_dwell_time_active_chan = (unsigned int) atoi(argv[0]);
	min_dwell_time_active_chan = (unsigned int) atoi(argv[1]);
	max_dwell_time_passive_chan = (unsigned int) atoi(argv[2]);
	min_dwell_time_passive_chan = (unsigned int) atoi(argv[3]);

	qcsapi_retval = qcsapi_wifi_set_dwell_times(the_interface,
							max_dwell_time_active_chan,
							min_dwell_time_active_chan,
							max_dwell_time_passive_chan,
							min_dwell_time_passive_chan);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "complete\n" );
		}
	} else {
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_get_dwell_times( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	const char	*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output	*print = p_calling_bundle->caller_output;
	int		qcsapi_retval;
	unsigned int	max_dwell_time_active_chan;
	unsigned int	min_dwell_time_active_chan;
	unsigned int	max_dwell_time_passive_chan;
	unsigned int	min_dwell_time_passive_chan;
	int		statval = 0;

	qcsapi_retval = qcsapi_wifi_get_dwell_times(the_interface,
							&max_dwell_time_active_chan,
							&min_dwell_time_active_chan,
							&max_dwell_time_passive_chan,
							&min_dwell_time_passive_chan);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "%d %d %d %d\n",
				 max_dwell_time_active_chan,
				 min_dwell_time_active_chan,
				 max_dwell_time_passive_chan,
				 min_dwell_time_passive_chan);
		}
	} else {
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_set_bgscan_dwell_times( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	const char	*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output	*print = p_calling_bundle->caller_output;
	int		qcsapi_retval;
	unsigned int	dwell_time_active_chan;
	unsigned int	dwell_time_passive_chan;
	int		statval = 0;

	if (argc < 2) {
		print_err( print, "STA Set BGScan Dwell Times requires 2 dwell times\n" );
		return(1);
	}

	dwell_time_active_chan = (unsigned int) atoi(argv[0]);
	dwell_time_passive_chan = (unsigned int) atoi(argv[1]);

	qcsapi_retval = qcsapi_wifi_set_bgscan_dwell_times(the_interface,
			dwell_time_active_chan,	dwell_time_passive_chan);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "complete\n" );
		}
	} else {
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_get_bgscan_dwell_times( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	const char	*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output	*print = p_calling_bundle->caller_output;
	int		qcsapi_retval;
	unsigned int	dwell_time_active_chan;
	unsigned int	dwell_time_passive_chan;
	int		statval = 0;

	qcsapi_retval = qcsapi_wifi_get_bgscan_dwell_times(the_interface,
			&dwell_time_active_chan, &dwell_time_passive_chan);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "%d %d\n",
				 dwell_time_active_chan, dwell_time_passive_chan);
		}
	} else {
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_get_count_associations( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_unsigned_int	 association_count, *p_association_count = NULL;
	int			 qcsapi_retval;
	const char		*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output		*print = p_calling_bundle->caller_output;

	if (argc < 1 || strcmp( argv[ 0 ], "NULL" ) != 0)
	  p_association_count = &association_count;
	qcsapi_retval = qcsapi_wifi_get_count_associations( the_interface, p_association_count );
	if (qcsapi_retval >= 0)
	{
		if (verbose_flag >= 0)
		{
			print_out( print, "%u\n", (unsigned int) association_count );
		}
	}
	else
	{
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_get_associated_device_mac_addr( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_mac_addr		 the_mac_addr;
	int			 qcsapi_retval;
	const char		*the_interface = p_calling_bundle->caller_interface;
	qcsapi_unsigned_int	 device_index = p_calling_bundle->caller_generic_parameter.index;

	if (argc > 0 && strcmp( argv[ 0 ], "NULL" ) == 0)
	  qcsapi_retval = qcsapi_wifi_get_associated_device_mac_addr( the_interface, device_index, NULL );
	else
	  qcsapi_retval = qcsapi_wifi_get_associated_device_mac_addr( the_interface, device_index, the_mac_addr );
	if (qcsapi_retval >= 0)
	{
		if (verbose_flag >= 0)
		{
			dump_mac_addr(p_calling_bundle->caller_output, the_mac_addr );
		}
	}
	else
	{
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_get_associated_device_ip_addr(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int			 statval = 0;
	unsigned int		 ip_addr = 0;
	char			 ip_str[IP_ADDR_STR_LEN + 1];
	int			 qcsapi_retval;
	const char		*the_interface = p_calling_bundle->caller_interface;
	qcsapi_unsigned_int	 device_index = p_calling_bundle->caller_generic_parameter.index;
	qcsapi_output		*print = p_calling_bundle->caller_output;

	if (argc > 0 && strcmp(argv[0], "NULL") == 0)
		qcsapi_retval = qcsapi_wifi_get_associated_device_ip_addr(the_interface, device_index, NULL);
	else
		qcsapi_retval = qcsapi_wifi_get_associated_device_ip_addr(the_interface, device_index, &ip_addr);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			inet_ntop(AF_INET, &ip_addr, ip_str, IP_ADDR_STR_LEN);
			print_out(print, "%s\n", ip_str);
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return(statval);
}

static int
call_qcsapi_wifi_get_link_quality( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_unsigned_int	 link_quality, *p_link_quality = NULL;
	int			 qcsapi_retval;
	const char		*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output		*print = p_calling_bundle->caller_output;
	qcsapi_unsigned_int	 association_index = p_calling_bundle->caller_generic_parameter.index;

	if (argc < 1 || strcmp( argv[ 0 ], "NULL" ) != 0)
	  p_link_quality = &link_quality;
	qcsapi_retval = qcsapi_wifi_get_link_quality( the_interface, association_index, p_link_quality );
	if (qcsapi_retval >= 0)
	{
		if (verbose_flag >= 0)
		{
			print_out( print, "%u\n", link_quality );
		}
	}
	else
	{
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_get_rssi_per_association( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_unsigned_int	 rssi, *p_rssi = NULL;
	int			 qcsapi_retval;
	const char		*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output		*print = p_calling_bundle->caller_output;
	qcsapi_unsigned_int	 association_index = p_calling_bundle->caller_generic_parameter.index;

	if (argc < 1 || strcmp( argv[ 0 ], "NULL" ) != 0)
	  p_rssi = &rssi;
	qcsapi_retval = qcsapi_wifi_get_rssi_per_association( the_interface, association_index, p_rssi );
	if (qcsapi_retval >= 0)
	{
		if (verbose_flag >= 0)
		{
			print_out( print, "%u\n", rssi );
		}
	}
	else
	{
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_get_rssi_in_dbm_per_association( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int			 statval = 0;
	int			 rssi, *p_rssi = NULL;
	int			 qcsapi_retval;
	const char		*the_interface = p_calling_bundle->caller_interface;
	qcsapi_unsigned_int	 association_index = p_calling_bundle->caller_generic_parameter.index;
	qcsapi_output		*print = p_calling_bundle->caller_output;

	if (argc < 1 || strcmp( argv[ 0 ], "NULL" ) != 0) {
		p_rssi = &rssi;
	}

	qcsapi_retval = qcsapi_wifi_get_rssi_in_dbm_per_association( the_interface, association_index, p_rssi );
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "%d\n", rssi );
		}
	} else {
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_get_snr_per_association( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int			 statval = 0;
	int			 snr, *p_snr = NULL;
	int			 qcsapi_retval;
	const char		*the_interface = p_calling_bundle->caller_interface;
	qcsapi_unsigned_int	 association_index = p_calling_bundle->caller_generic_parameter.index;
	qcsapi_output		*print = p_calling_bundle->caller_output;

	if (argc < 1 || strcmp( argv[ 0 ], "NULL" ) != 0)
	  p_snr = &snr;
	qcsapi_retval = qcsapi_wifi_get_snr_per_association( the_interface, association_index, p_snr );
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "%d\n", snr );
		}
	} else {
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_get_hw_noise_per_association( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	int	 hw_noise;
	int			 qcsapi_retval;
	const char		*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output		*print = p_calling_bundle->caller_output;
	qcsapi_unsigned_int	 association_index = p_calling_bundle->caller_generic_parameter.index;

	qcsapi_retval = qcsapi_wifi_get_hw_noise_per_association( the_interface, association_index, &hw_noise );
	if (qcsapi_retval >= 0)
	{
		if (verbose_flag >= 0)
		{
			print_out( print, "%d.%d\n", hw_noise/10, abs(hw_noise%10) );
		}
	}
	else
	{
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_get_rx_bytes_per_association( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	u_int64_t		 rx_bytes, *p_rx_bytes = NULL;
	int			 qcsapi_retval;
	const char		*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output		*print = p_calling_bundle->caller_output;
	qcsapi_unsigned_int	 association_index = p_calling_bundle->caller_generic_parameter.index;

	if (argc < 1 || strcmp( argv[ 0 ], "NULL" ) != 0)
	  p_rx_bytes = &rx_bytes;
	qcsapi_retval = qcsapi_wifi_get_rx_bytes_per_association( the_interface, association_index, p_rx_bytes );
	if (qcsapi_retval >= 0)
	{
		if (verbose_flag >= 0)
		{
			print_out( print, "%llu\n", rx_bytes );
		}
	}
	else
	{
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_get_tx_bytes_per_association( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	u_int64_t		 tx_bytes, *p_tx_bytes = NULL;
	int			 qcsapi_retval;
	const char		*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output		*print = p_calling_bundle->caller_output;
	qcsapi_unsigned_int	 association_index = p_calling_bundle->caller_generic_parameter.index;

	if (argc < 1 || strcmp( argv[ 0 ], "NULL" ) != 0)
	  p_tx_bytes = &tx_bytes;
	qcsapi_retval = qcsapi_wifi_get_tx_bytes_per_association( the_interface, association_index, p_tx_bytes );
	if (qcsapi_retval >= 0)
	{
		if (verbose_flag >= 0)
		{
			print_out( print, "%llu\n", tx_bytes );
		}
	}
	else
	{
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_get_rx_packets_per_association( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_unsigned_int	rx_packets, *p_rx_packets = NULL;
	int			qcsapi_retval;
	const char		*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output		*print = p_calling_bundle->caller_output;
	qcsapi_unsigned_int	association_index = p_calling_bundle->caller_generic_parameter.index;

	if (argc < 1 || strcmp( argv[ 0 ], "NULL" ) != 0)
	  p_rx_packets = &rx_packets;

	qcsapi_retval = qcsapi_wifi_get_rx_packets_per_association( the_interface, association_index, p_rx_packets );
	if (qcsapi_retval >= 0)	{
		if (verbose_flag >= 0) {
			print_out(print, "%u\n", rx_packets);
		}
	} else {
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_get_tx_packets_per_association( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int			statval = 0;
	qcsapi_unsigned_int	tx_packets, *p_tx_packets = NULL;
	int			qcsapi_retval;
	const char		*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output		*print = p_calling_bundle->caller_output;
	qcsapi_unsigned_int	association_index = p_calling_bundle->caller_generic_parameter.index;

	if (argc < 1 || strcmp( argv[ 0 ], "NULL" ) != 0)
	  p_tx_packets = &tx_packets;
	qcsapi_retval = qcsapi_wifi_get_tx_packets_per_association( the_interface, association_index, p_tx_packets );
	if (qcsapi_retval >= 0)	{
		if (verbose_flag >= 0) {
			print_out(print, "%u\n", tx_packets);
		}
	} else {
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_get_tx_err_packets_per_association( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int			statval = 0;
	qcsapi_output		*print = p_calling_bundle->caller_output;
	qcsapi_unsigned_int	tx_err_packets, *p_tx_err_packets = NULL;
	int			qcsapi_retval;
	const char		*the_interface = p_calling_bundle->caller_interface;
	qcsapi_unsigned_int	association_index = p_calling_bundle->caller_generic_parameter.index;

	if (argc < 1 || strcmp( argv[ 0 ], "NULL" ) != 0)
		p_tx_err_packets = &tx_err_packets;

	qcsapi_retval = qcsapi_wifi_get_tx_err_packets_per_association( the_interface, association_index, p_tx_err_packets );
	if (qcsapi_retval >= 0)	{
		if (verbose_flag >= 0) {
			print_out(print, "%u\n", tx_err_packets);
		}
	} else {
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_get_bw_per_association( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int			 statval = 0;
	int			 qcsapi_retval;
	const char		*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output		*print = p_calling_bundle->caller_output;
	qcsapi_unsigned_int	 association_index = p_calling_bundle->caller_generic_parameter.index;
	qcsapi_unsigned_int	 bw, *p_bw = NULL;

	if (argc < 1 || strcmp( argv[ 0 ], "NULL" ) != 0)
	  p_bw = &bw;
	qcsapi_retval = qcsapi_wifi_get_bw_per_association( the_interface, association_index, p_bw );
	if (qcsapi_retval >= 0)
	{
		if (verbose_flag >= 0)
		{
			print_out( print, "%u\n", bw );
		}
	}
	else
	{
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_get_tx_phy_rate_per_association( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int			 statval = 0;
	qcsapi_unsigned_int	 tx_rate, *p_tx_rate = NULL;
	qcsapi_output		*print = p_calling_bundle->caller_output;
	int		 	 qcsapi_retval;
	const char		*the_interface = p_calling_bundle->caller_interface;
	qcsapi_unsigned_int	 association_index = p_calling_bundle->caller_generic_parameter.index;

	if (argc < 1 || strcmp( argv[ 0 ], "NULL" ) != 0)
	  p_tx_rate = &tx_rate;
	qcsapi_retval = qcsapi_wifi_get_tx_phy_rate_per_association( the_interface, association_index, p_tx_rate );
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out(print, "%u\n", tx_rate);
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_get_rx_phy_rate_per_association( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int			 statval = 0;
	qcsapi_unsigned_int	 rx_rate, *p_rx_rate = NULL;
	qcsapi_output		*print = p_calling_bundle->caller_output;
	int		 	 qcsapi_retval;
	const char		*the_interface = p_calling_bundle->caller_interface;
	qcsapi_unsigned_int	 association_index = p_calling_bundle->caller_generic_parameter.index;

	if (argc < 1 || strcmp( argv[ 0 ], "NULL" ) != 0)
	  p_rx_rate = &rx_rate;
	qcsapi_retval = qcsapi_wifi_get_rx_phy_rate_per_association( the_interface, association_index, p_rx_rate );
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out(print, "%u\n", rx_rate);
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_get_tx_mcs_per_association( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int statval = 0;
	qcsapi_unsigned_int tx_mcs;
	qcsapi_output *print = p_calling_bundle->caller_output;
	int qcsapi_retval;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_unsigned_int association_index = p_calling_bundle->caller_generic_parameter.index;

	qcsapi_retval = qcsapi_wifi_get_tx_mcs_per_association(the_interface,
			association_index, &tx_mcs);
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0)
			print_out(print, "%u\n", tx_mcs);
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wifi_get_rx_mcs_per_association(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	qcsapi_unsigned_int rx_mcs;
	qcsapi_output *print = p_calling_bundle->caller_output;
	int qcsapi_retval;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_unsigned_int association_index = p_calling_bundle->caller_generic_parameter.index;

	qcsapi_retval = qcsapi_wifi_get_rx_mcs_per_association(the_interface,
			association_index, &rx_mcs);
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0)
			print_out(print, "%u\n", rx_mcs);
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wifi_get_achievable_tx_phy_rate_per_association( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int			 statval = 0;
	qcsapi_output		*print = p_calling_bundle->caller_output;
	qcsapi_unsigned_int	 achievable_tx_rate, *p_achievable_tx_rate = NULL;
	int			 qcsapi_retval;
	const char		*the_interface = p_calling_bundle->caller_interface;
	qcsapi_unsigned_int	 association_index = p_calling_bundle->caller_generic_parameter.index;

	if (argc < 1 || strcmp( argv[ 0 ], "NULL" ) != 0)
		p_achievable_tx_rate = &achievable_tx_rate;

	qcsapi_retval = qcsapi_wifi_get_achievable_tx_phy_rate_per_association( the_interface, association_index, p_achievable_tx_rate );
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out(print, "%u\n", achievable_tx_rate);
		}
	}
	else
	{
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_get_achievable_rx_phy_rate_per_association( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int			 statval = 0;
	qcsapi_output		*print = p_calling_bundle->caller_output;
	qcsapi_unsigned_int	 achievable_rx_rate, *p_achievable_rx_rate = NULL;
	int	 		 qcsapi_retval;
	const char		*the_interface = p_calling_bundle->caller_interface;
	qcsapi_unsigned_int	 association_index = p_calling_bundle->caller_generic_parameter.index;

	if (argc < 1 || strcmp( argv[ 0 ], "NULL" ) != 0)
		p_achievable_rx_rate = &achievable_rx_rate;

	qcsapi_retval = qcsapi_wifi_get_achievable_rx_phy_rate_per_association( the_interface, association_index, p_achievable_rx_rate );
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out(print, "%u\n", achievable_rx_rate);
		}
	} else {
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_get_auth_enc_per_association( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int			 statval = 0;
	qcsapi_output		*print = p_calling_bundle->caller_output;
	int			 qcsapi_retval;
	const char		*the_interface = p_calling_bundle->caller_interface;
	qcsapi_unsigned_int	 association_index = p_calling_bundle->caller_generic_parameter.index;
	qcsapi_unsigned_int	 auth_enc;
	uint8_t			*casted_ptr = (uint8_t*)&auth_enc;

	qcsapi_retval = qcsapi_wifi_get_auth_enc_per_association( the_interface, association_index, &auth_enc );
	if (qcsapi_retval >= 0) {
		if (casted_ptr[IEEE80211_AUTHDESCR_ALGO_POS] >= ARRAY_SIZE(qcsapi_auth_algo_list) ||
		    casted_ptr[IEEE80211_AUTHDESCR_KEYMGMT_POS] >= ARRAY_SIZE(qcsapi_auth_keymgmt_list) ||
		    casted_ptr[IEEE80211_AUTHDESCR_KEYPROTO_POS] >=  ARRAY_SIZE(qcsapi_auth_keyproto_list) ||
		    casted_ptr[IEEE80211_AUTHDESCR_CIPHER_POS] >= ARRAY_SIZE(qcsapi_auth_cipher_list)) {

			print_err(print, "error:unknown auth enc value \"%08X\"\n", auth_enc);
			return 1;
		}

		if (verbose_flag >= 0) {
			if (casted_ptr[IEEE80211_AUTHDESCR_KEYPROTO_POS]) {
				print_out(print, "%s/%s with %s\n",
					  qcsapi_auth_keyproto_list[casted_ptr[IEEE80211_AUTHDESCR_KEYPROTO_POS]],
					  qcsapi_auth_keymgmt_list[casted_ptr[IEEE80211_AUTHDESCR_KEYMGMT_POS]],
					  qcsapi_auth_cipher_list[casted_ptr[IEEE80211_AUTHDESCR_CIPHER_POS]]);
			} else {
				print_out(print, "%s/%s\n",
					  qcsapi_auth_algo_list[casted_ptr[IEEE80211_AUTHDESCR_ALGO_POS]],
					  qcsapi_auth_keymgmt_list[casted_ptr[IEEE80211_AUTHDESCR_KEYMGMT_POS]]);
			}
		}
	} else {
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wifi_get_vendor_per_association( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int			 statval = 0;
	qcsapi_output		*print = p_calling_bundle->caller_output;
	int			 qcsapi_retval;
	const char		*the_interface = p_calling_bundle->caller_interface;
	qcsapi_unsigned_int	 association_index = p_calling_bundle->caller_generic_parameter.index;
	qcsapi_unsigned_int	 vendor;

	qcsapi_retval = qcsapi_wifi_get_vendor_per_association(the_interface, association_index, &vendor);
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			switch (vendor) {
			case PEER_VENDOR_QTN:
				print_out(print, "quantenna\n");
				break;
			case PEER_VENDOR_BRCM:
				print_out(print, "broadcom\n");
				break;
			case PEER_VENDOR_ATH:
				print_out(print, "atheros\n");
				break;
			case PEER_VENDOR_RLNK:
				print_out(print, "ralink\n");
				break;
			case PEER_VENDOR_RTK:
				print_out(print, "realtek\n");
				break;
			case PEER_VENDOR_INTEL:
				print_out(print, "intel\n");
				break;
			default:
				print_out(print, "unknown\n");
				break;
			}
		}
	} else {
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}
	return( statval );
}

static int
call_qcsapi_wifi_get_max_mimo( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int			 statval = 0;
	qcsapi_output		*print = p_calling_bundle->caller_output;
	int			 qcsapi_retval;
	const char		*the_interface = p_calling_bundle->caller_interface;
	qcsapi_unsigned_int	 association_index = p_calling_bundle->caller_generic_parameter.index;
	string_16		max_mimo;

	qcsapi_retval = qcsapi_wifi_get_max_mimo(the_interface, association_index, max_mimo);
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out(print, "%s\n", max_mimo);
		}
	} else {
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}
	return statval;
}

static int
call_qcsapi_wifi_get_tput_caps(call_qcsapi_bundle *p_calling_bundle,
		int argc, char *argv[])
{
	int statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_unsigned_int association_index = p_calling_bundle->caller_generic_parameter.index;
	int qcsapi_retval;
	struct ieee8011req_sta_tput_caps tput_caps;
	struct ieee80211_ie_vhtcap *ie_vhtcap;
	struct ieee80211_ie_htcap *ie_htcap;

	qcsapi_retval = qcsapi_wifi_get_tput_caps(the_interface, association_index, &tput_caps);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			switch (tput_caps.mode) {
			case IEEE80211_WIFI_MODE_AC:
				print_out(print, "Mode: VHT\n");
				ie_vhtcap = (struct ieee80211_ie_vhtcap*)tput_caps.vhtcap_ie;

				print_out(print, "VHT Capabilities Info: ");
				dump_data_array(print, ie_vhtcap->vht_cap,
						sizeof(ie_vhtcap->vht_cap), 16, ' ');

				print_out(print, "Supported VHT MCS & NSS Set: ");
				dump_data_array(print, ie_vhtcap->vht_mcs_nss_set,
						sizeof(ie_vhtcap->vht_mcs_nss_set), 16, ' ');
				/* Fall through */
			case IEEE80211_WIFI_MODE_NA:
				/* Fall through */
			case IEEE80211_WIFI_MODE_NG:
				if (tput_caps.mode != IEEE80211_WIFI_MODE_AC) {
					print_out(print, "Mode: HT\n");
				}
				ie_htcap = (struct ieee80211_ie_htcap*)tput_caps.htcap_ie;

				print_out(print, "HT Capabilities Info: ");
				dump_data_array(print, ie_htcap->hc_cap,
						sizeof(ie_htcap->hc_cap), 16, ' ');

				print_out(print, "A-MPDU Parameters: %02X\n", ie_htcap->hc_ampdu);

				print_out(print, "Supported MCS Set: ");
				dump_data_array(print, ie_htcap->hc_mcsset,
						sizeof(ie_htcap->hc_mcsset), 16, ' ');

				print_out(print, "HT Extended Capabilities: ");
				dump_data_array(print, ie_htcap->hc_extcap,
						sizeof(ie_htcap->hc_extcap), 16, ' ');

				print_out(print, "Transmit Beamforming Capabilities: ");
				dump_data_array(print, ie_htcap->hc_txbf,
						sizeof(ie_htcap->hc_txbf), 16, ' ');

				print_out(print, "ASEL Capabilities: %02X\n", ie_htcap->hc_antenna);
				break;
			default:
				print_out(print, "Mode: non HT\n");
				break;
			}
		}
	} else {
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}
	return statval;
}

static int
call_qcsapi_wifi_get_connection_mode(call_qcsapi_bundle *p_calling_bundle,
				     int argc, char *argv[])
{
	int statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_unsigned_int association_index = p_calling_bundle->caller_generic_parameter.index;
	int qcsapi_retval;
	qcsapi_unsigned_int connection_mode;

	qcsapi_retval = qcsapi_wifi_get_connection_mode(the_interface,
							association_index,
							&connection_mode);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			if (connection_mode >= IEEE80211_WIFI_MODE_MAX) {
				connection_mode = IEEE80211_WIFI_MODE_NONE;
			}
			print_out(print, "%s\n", qcsapi_wifi_modes_strings[connection_mode]);
		}
	} else {
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}
	return statval;
}

static int
call_qcsapi_wifi_get_node_counter(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;
	qcsapi_unsigned_int node_index = p_calling_bundle->caller_generic_parameter.index;
	qcsapi_counter_type counter_type = QCSAPI_NOSUCH_COUNTER;
	int local_remote_flag = QCSAPI_LOCAL_NODE;
	uint64_t counter_value = 0;
	uint64_t *p_counter_value = &counter_value;

	if (argc < 1) {
		print_err(print, "Get Counter Per Association: type of counter required\n");
		return 1;
	}

	if (name_to_counter_enum(argv[0], &counter_type ) == 0) {
		print_err(print, "No such counter type %s\n", argv[0]);
		return 1;
	}

	if (argc > 1) {
		if (parse_local_remote_flag(print, argv[1], &local_remote_flag) < 0) {
			return 1;
		}
	}

	if (argc > 2 && strcmp(argv[2], "NULL" ) == 0) {
		p_counter_value = NULL;
	}

	qcsapi_retval = qcsapi_wifi_get_node_counter(the_interface,
						     node_index,
						     counter_type,
						     local_remote_flag,
						     p_counter_value);
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out(print, "%llu\n", counter_value);
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int parse_measure_request_param(qcsapi_measure_request_param *param,
		qcsapi_output *print, qcsapi_per_assoc_param type, int argc, char *argv[])
{
	int i;
	int ret;
	int qualified;
	int pre_len;
	int bad_format = 0;
	int mac[6];

	ret = 0;
	qualified = 0;
	switch (type) {
	case QCSAPI_NODE_MEAS_BASIC:
	{
		for (i = 0; i < argc; i++) {
			if ((strncmp(argv[i], "ch=", (pre_len = strlen("ch="))) == 0) &&
					(strlen(argv[i]) > pre_len)) {
				param->basic.channel = atoi(argv[i] + pre_len);
			} else if ((strncmp(argv[i], "off=", (pre_len = strlen("off="))) == 0) &&
					(strlen(argv[i]) > pre_len)) {
				param->basic.offset = atoi(argv[i] + pre_len);
			} else if ((strncmp(argv[i], "du=", (pre_len = strlen("du="))) == 0) &&
					(strlen(argv[i]) > pre_len)) {
				param->basic.duration = atoi(argv[i] + pre_len);
				qualified++;
			} else {
				print_err(print, "error:unknown parameter \"%s\"\n", argv[i]);
				bad_format = 1;
				break;
			}
		}

		if (!qualified || bad_format) {
			print_out(print, "basic measurement param:\n"
					"<du=duration> [ch=channel] "
					"[off=offset to start measuremnt]\n");
			ret = 1;
		}
		break;
	}
	case QCSAPI_NODE_MEAS_CCA:
	{
		for (i = 0; i < argc; i++) {
			if ((strncmp(argv[i], "ch=", (pre_len = strlen("ch="))) == 0) &&
					(strlen(argv[i]) > pre_len)) {
				param->cca.channel = atoi(argv[i] + pre_len);
			} else if ((strncmp(argv[i], "off=", (pre_len = strlen("off="))) == 0) &&
					(strlen(argv[i]) > pre_len)) {
				param->cca.offset = atoi(argv[i] + pre_len);
			} else if ((strncmp(argv[i], "du=", (pre_len = strlen("du="))) == 0) &&
					(strlen(argv[i]) > pre_len)) {
				param->cca.duration = atoi(argv[i] + pre_len);
				qualified++;
			} else {
				print_err(print, "error:unknown parameter \"%s\"\n", argv[i]);
				bad_format = 1;
				break;
			}
		}

		if (!qualified || bad_format) {
			print_out(print, "cca measurement param:\n"
					"<du=duration> [ch=channel] "
					"[off=offset to start measuremnt]\n");
			ret = 1;
		}
		break;
	}
	case QCSAPI_NODE_MEAS_RPI:
	{
		for (i = 0; i < argc; i++) {
			if ((strncmp(argv[i], "ch=", (pre_len = strlen("ch="))) == 0) &&
					(strlen(argv[i]) > pre_len)) {
				param->rpi.channel = atoi(argv[i] + pre_len);
			} else if ((strncmp(argv[i], "off=", (pre_len = strlen("off="))) == 0) &&
					(strlen(argv[i]) > pre_len)) {
				param->rpi.offset = atoi(argv[i] + pre_len);
			} else if ((strncmp(argv[i], "du=", (pre_len = strlen("du="))) == 0) &&
					(strlen(argv[i]) > pre_len)) {
				param->rpi.duration = atoi(argv[i] + pre_len);
				qualified++;
			} else {
				print_err(print, "error:unknown parameter \"%s\"\n", argv[i]);
				bad_format = 1;
			}
		}

		if (!qualified || bad_format) {
			print_out(print, "rpi measurement param:\n"
					"<du=duration> [ch=channel] "
					"[off=offset to start measuremnt]\n");
			ret = 1;
		}
		break;
	}
	case QCSAPI_NODE_MEAS_CHAN_LOAD:
	{
		for (i = 0; i < argc; i++) {
			if ((strncmp(argv[i], "ch=", (pre_len = strlen("ch="))) == 0) &&
					(strlen(argv[i]) > pre_len)) {
				param->chan_load.channel = atoi(argv[i] + pre_len);
			} else if ((strncmp(argv[i], "op=", (pre_len = strlen("op="))) == 0) &&
					(strlen(argv[i]) > pre_len)) {
				param->chan_load.op_class = atoi(argv[i] + pre_len);
			} else if ((strncmp(argv[i], "du=", (pre_len = strlen("du="))) == 0) &&
					(strlen(argv[i]) > pre_len)) {
				param->chan_load.duration = atoi(argv[i] + pre_len);
				qualified++;
			} else {
				print_err(print, "error:unknown parameter \"%s\"\n", argv[i]);
				bad_format = 1;
				break;
			}
		}

		if (!qualified || bad_format) {
			print_out(print, "channel load measurement param:\n"
					"<du=duration> [ch=channel] "
					"[op=operating class]\n");
			ret = 1;
		}
		break;
	}
	case QCSAPI_NODE_MEAS_NOISE_HIS:
	{
		for (i = 0; i < argc; i++) {
			if ((strncmp(argv[i], "ch=", (pre_len = strlen("ch="))) == 0) &&
					(strlen(argv[i]) > pre_len)) {
				param->noise_his.channel = atoi(argv[i] + pre_len);
			} else if ((strncmp(argv[i], "op=", (pre_len = strlen("op="))) == 0) &&
					(strlen(argv[i]) > pre_len)) {
				param->noise_his.op_class = atoi(argv[i] + pre_len);
			} else if ((strncmp(argv[i], "du=", (pre_len = strlen("du="))) == 0) &&
					(strlen(argv[i]) > pre_len)) {
				param->noise_his.duration = atoi(argv[i] + pre_len);
				qualified++;
			} else {
				print_err(print, "error:unknown parameter \"%s\"\n", argv[i]);
				bad_format = 1;
				break;
			}
		}

		if (!qualified || bad_format) {
			print_out(print, "noise histogram measurement param:\n"
					"<du=duration> [ch=channel] "
					"[op=operating class]\n");
			ret = 1;
		}
		break;
	}
	case QCSAPI_NODE_MEAS_BEACON:
	{
		for (i = 0; i < argc; i++) {
			if ((strncmp(argv[i], "ch=", (pre_len = strlen("ch="))) == 0) &&
					(strlen(argv[i]) > pre_len)) {
				param->beacon.channel = atoi(argv[i] + pre_len);
			} else if ((strncmp(argv[i], "op=", (pre_len = strlen("op="))) == 0) &&
					(strlen(argv[i]) > pre_len)) {
				param->beacon.op_class = atoi(argv[i] + pre_len);
			} else if ((strncmp(argv[i], "du=", (pre_len = strlen("du="))) == 0) &&
					(strlen(argv[i]) > pre_len)) {
				param->beacon.duration = atoi(argv[i] + pre_len);
				qualified++;
			} else if ((strncmp(argv[i], "mode=", (pre_len = strlen("mode="))) == 0) &&
					(strlen(argv[i]) > pre_len)) {
				param->beacon.mode = atoi(argv[i] + pre_len);
			} else {
				bad_format = 1;
				print_err(print, "error:unknown parameter \"%s\"\n", argv[i]);
				break;
			}
		}

		if (!qualified || bad_format) {
			print_out(print, "beacon measurement param:\n"
					"<du=duration> [ch=channel] "
					"[mode=beacon measurement mode][op=operating class]\n");
			ret = 1;
		}
		break;
	}
	case QCSAPI_NODE_MEAS_FRAME:
	{
		int cnt;

		for (i = 0; i < argc; i++) {
			if ((strncmp(argv[i], "ch=", (pre_len = strlen("ch="))) == 0) &&
					(strlen(argv[i]) > pre_len)) {
				param->frame.channel = atoi(argv[i] + pre_len);
			} else if ((strncmp(argv[i], "op=", (pre_len = strlen("op="))) == 0) &&
					(strlen(argv[i]) > pre_len)) {
				param->frame.op_class = atoi(argv[i] + pre_len);
			} else if ((strncmp(argv[i], "du=", (pre_len = strlen("du="))) == 0) &&
					(strlen(argv[i]) > pre_len)) {
				param->frame.duration = atoi(argv[i] + pre_len);
				qualified++;
			} else if ((strncmp(argv[i], "type=", (pre_len = strlen("type="))) == 0) &&
					(strlen(argv[i]) > pre_len)) {
				param->frame.type = atoi(argv[i] + pre_len);
				qualified++;
			} else if ((strncmp(argv[i], "mac=", (pre_len = strlen("mac="))) == 0) &&
					(strlen(argv[i]) > pre_len)) {
				if (sscanf(argv[i] + pre_len, "%x:%x:%x:%x:%x:%x", &mac[0],
							&mac[1],
							&mac[2],
							&mac[3],
							&mac[4],
							&mac[5]) != 6){
					bad_format = 1;
					break;
				}
				for (cnt = 0; cnt < 6; cnt++)
					param->frame.mac_address[cnt] = (uint8_t)mac[cnt];
			} else {
				bad_format = 1;
				print_err(print, "error:unknown parameter \"%s\"\n", argv[i]);
				break;
			}
		}

		if ((qualified < 2) || bad_format) {
			print_out(print, "frame measurement param:\n"
					"<du=duration>\n"
					"<type=measurement frame type, only 1 supported currently>\n"
					"[ch=channel] [op=operating class] [mac=specified mac address]\n");
			ret = 1;
		}
		break;
	}
	case QCSAPI_NODE_MEAS_TRAN_STREAM_CAT:
	{
		int cnt;

		for (i = 0; i < argc; i++) {
			if ((strncmp(argv[i], "tid=", (pre_len = strlen("tid="))) == 0) &&
					(strlen(argv[i]) > pre_len)) {
				param->tran_stream_cat.tid = atoi(argv[i] + pre_len);
				qualified++;
			} else if ((strncmp(argv[i], "bin0=", (pre_len = strlen("bin0="))) == 0) &&
					(strlen(argv[i]) > pre_len)) {
				param->tran_stream_cat.bin0 = atoi(argv[i] + pre_len);
			} else if ((strncmp(argv[i], "du=", (pre_len = strlen("du="))) == 0) &&
					(strlen(argv[i]) > pre_len)) {
				param->tran_stream_cat.duration = atoi(argv[i] + pre_len);
				qualified++;
			} else if ((strncmp(argv[i], "peer_sta=", (pre_len = strlen("peer_sta="))) == 0) &&
					(strlen(argv[i]) > pre_len)) {
				if (sscanf(argv[i] + pre_len, "%x:%x:%x:%x:%x:%x", &mac[0],
							&mac[1],
							&mac[2],
							&mac[3],
							&mac[4],
							&mac[5]) != 6) {
					bad_format = 1;
					break;
				}
				for (cnt = 0; cnt < 6; cnt++)
					param->tran_stream_cat.peer_sta[cnt] = (uint8_t)mac[cnt];
			} else {
				bad_format = 1;
				print_err(print, "error:unknown parameter \"%s\"\n", argv[i]);
				break;
			}
		}

		if ((qualified < 2) || bad_format) {
			print_out(print, "transmit stream category measurement param:\n"
					"<du=duration>\n"
					"<tid=traffic id>\n"
					"[peer_sta=peer station mac address] [bin0=bin0 range]\n");
			ret = 1;
		}
		break;
	}
	case QCSAPI_NODE_MEAS_MULTICAST_DIAG:
	{
		int cnt;

		for (i = 0; i < argc; i++) {
			if ((strncmp(argv[i], "du=", (pre_len = strlen("du="))) == 0) &&
					(strlen(argv[i]) > pre_len)) {
				param->multicast_diag.duration = atoi(argv[i] + pre_len);
				qualified++;
			} else if ((strncmp(argv[i], "group_mac=", (pre_len = strlen("group_mac="))) == 0) &&
					(strlen(argv[i]) > pre_len)) {
				if (sscanf(argv[i] + pre_len, "%x:%x:%x:%x:%x:%x", &mac[0],
							&mac[1],
							&mac[2],
							&mac[3],
							&mac[4],
							&mac[5]) != 6) {
					bad_format = 1;
					break;
				}
				for (cnt = 0; cnt < 6; cnt++)
					param->multicast_diag.group_mac[cnt] = (uint8_t)mac[cnt];
				qualified++;
			} else {
				bad_format = 1;
				print_err(print, "error:unknown parameter \"%s\"\n", argv[i]);
				break;
			}
		}

		if ((qualified < 2) || bad_format) {
			print_out(print, "multicast diagnostic measurement param:\n"
					"<du=duration>\n"
					"<group_mac=group mac address>\n");
			ret = 1;
		}
		break;
	}
	default:
		break;
	}

	return ret;
}

static int
call_qcsapi_wifi_get_node_param(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int			 statval = 0;
	int			 qcsapi_retval;
	const char		*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output		*print = p_calling_bundle->caller_output;
	qcsapi_unsigned_int	 node_index = p_calling_bundle->caller_generic_parameter.index;
	qcsapi_per_assoc_param	 param_type = QCSAPI_NO_SUCH_PER_ASSOC_PARAM;
	int			 local_remote_flag = QCSAPI_LOCAL_NODE;
	string_128 input_param_str;
	qcsapi_measure_request_param *request_param;
	qcsapi_measure_report_result report_result;
	int *p_param_value;

	if (argc < 1) {
		print_err(print, "Get Parameter Per Association: type of parameter required\n");
		return 1;
	}

	if (name_to_per_assoc_parameter(argv[0], &param_type) == 0) {
		print_err(print, "No such parameter type %s\n", argv[0]);
		return 1;
	}

	if (argc > 1) {
		if (parse_local_remote_flag(print, argv[1], &local_remote_flag) < 0) {
			return 1;
		}
	}

	request_param = (qcsapi_measure_request_param *)input_param_str;
	if (argc >= 2) {
		argc -= 2;
		argv += 2;
		memset(request_param, 0, sizeof(*request_param));
		if (parse_measure_request_param(request_param, print, param_type, argc, argv)) {
			return 1;
		}
	}

	memset(&report_result, 0, sizeof(report_result));
	qcsapi_retval = qcsapi_wifi_get_node_param(the_interface,
						   node_index,
						   param_type,
						   local_remote_flag,
						   input_param_str,
						   &report_result);
	p_param_value = report_result.common;
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			switch (param_type) {
			case QCSAPI_SOC_MAC_ADDR:
			{
				qcsapi_mac_addr		 the_mac_addr;
				memcpy(the_mac_addr, p_param_value, sizeof(qcsapi_mac_addr));
				dump_mac_addr(print, the_mac_addr );
				break;
			}
			case QCSAPI_SOC_IP_ADDR:
				print_out(print, "%d.%d.%d.%d\n",
						((char *)p_param_value)[0], ((char *)p_param_value)[1],
						((char *)p_param_value)[2], ((char *)p_param_value)[3]);
				break;
			case QCSAPI_NODE_MEAS_RPI:
				dump_data_array(print, report_result.rpi, 8, 10, ' ');
				break;
			case QCSAPI_NODE_TPC_REP:
				print_out(print, "link margin = %d db\n", report_result.tpc.link_margin);
				print_out(print, "transmit power = %d dbm\n", report_result.tpc.tx_power);
				break;
			case QCSAPI_NODE_MEAS_NOISE_HIS:
			{
				int i;

				print_out(print, "anntenna id = %d\n", report_result.noise_histogram.antenna_id);
				print_out(print, "anpi = %d\n", (0 - report_result.noise_histogram.anpi));
				for (i = 0; i < 11; i++)
					print_out(print, "ipi%d:%d\n", i, report_result.noise_histogram.ipi[i]);
				break;
			}
			case QCSAPI_NODE_MEAS_BEACON:
			{
				qcsapi_mac_addr the_mac_addr;

				print_out(print, "report frame info = %x\n", report_result.beacon.rep_frame_info);
				print_out(print, "rcpi = %d\n", report_result.beacon.rcpi);
				print_out(print, "rsni = %d\n", report_result.beacon.rsni);
				print_out(print, "mac address:");
				memcpy(the_mac_addr, report_result.beacon.bssid, sizeof(qcsapi_mac_addr));
				dump_mac_addr(print, the_mac_addr );
				print_out(print, "antenna id = %d\n", report_result.beacon.antenna_id);
				print_out(print, "parent_tsf = %d\n", report_result.beacon.parent_tsf);
				break;
			}
			case QCSAPI_NODE_MEAS_FRAME:
			{
				qcsapi_mac_addr the_mac_addr;

				if (report_result.frame.sub_ele_report == 0) {
					print_out(print, "no measurement result\n");
				} else {
					print_out(print, "TA address:");
					memcpy(the_mac_addr, report_result.frame.ta, sizeof(qcsapi_mac_addr));
					dump_mac_addr(print, the_mac_addr );
					print_out(print, "BSSID:");
					memcpy(the_mac_addr, report_result.frame.bssid, sizeof(qcsapi_mac_addr));
					dump_mac_addr(print, the_mac_addr );
					print_out(print, "phy_type = %d\n", report_result.frame.phy_type);
					print_out(print, "average RCPI = %d\n", report_result.frame.avg_rcpi);
					print_out(print, "last RSNI = %d\n", report_result.frame.last_rsni);
					print_out(print, "last RCPI = %d\n", report_result.frame.last_rcpi);
					print_out(print, "antenna id = %d\n", report_result.frame.antenna_id);
					print_out(print, "Frame count = %d\n", report_result.frame.frame_count);
				}
				break;
			}
			case QCSAPI_NODE_MEAS_TRAN_STREAM_CAT:
			{
				int i;

				print_out(print, "reason = %d\n", report_result.tran_stream_cat.reason);
				print_out(print, "transmitted MSDU count = %d\n", report_result.tran_stream_cat.tran_msdu_cnt);
				print_out(print, "MSDU Discarded Count = %d\n", report_result.tran_stream_cat.msdu_discard_cnt);
				print_out(print, "MSDU Failed Count = %d\n", report_result.tran_stream_cat.msdu_fail_cnt);
				print_out(print, "MSDU Multiple retry Count = %d\n", report_result.tran_stream_cat.msdu_mul_retry_cnt);
				print_out(print, "MSDU Qos CF-Polls Lost Count = %d\n", report_result.tran_stream_cat.qos_lost_cnt);
				print_out(print, "Average Queue Delay = %d\n", report_result.tran_stream_cat.avg_queue_delay);
				print_out(print, "Average Transmit Delay = %d\n", report_result.tran_stream_cat.avg_tran_delay);
				print_out(print, "Bin0 range = %d\n", report_result.tran_stream_cat.bin0_range);
				for (i = 0; i < 6; i++)
					print_out(print, "Bin%d = %d\n", i, report_result.tran_stream_cat.bins[i]);
				break;
			}
			case QCSAPI_NODE_MEAS_MULTICAST_DIAG:
				print_out(print, "reason = %d\n", report_result.multicast_diag.reason);
				print_out(print, "Multicast Received MSDU Count = %d\n", report_result.multicast_diag.mul_rec_msdu_cnt);
				print_out(print, "First Sequence Number = %d\n", report_result.multicast_diag.first_seq_num);
				print_out(print, "Last Sequence Number = %d\n", report_result.multicast_diag.last_seq_num);
				print_out(print, "Multicast Rate = %d\n", report_result.multicast_diag.mul_rate);
				break;
			case QCSAPI_NODE_LINK_MEASURE:
				print_out(print, "transmit power = %d\n", report_result.link_measure.tpc_report.tx_power);
				print_out(print, "link margin = %d\n", report_result.link_measure.tpc_report.link_margin);
				print_out(print, "receive antenna id = %d\n", report_result.link_measure.recv_antenna_id);
				print_out(print, "transmit antenna id = %d\n", report_result.link_measure.tran_antenna_id);
				print_out(print, "RCPI = %d\n", report_result.link_measure.rcpi);
				print_out(print, "RSNI = %d\n", report_result.link_measure.rsni);
				break;
			case QCSAPI_NODE_NEIGHBOR_REP:
			{
				uint8_t i;
				qcsapi_mac_addr the_mac_addr;

				if (report_result.neighbor_report.item_num == 0) {
					print_out(print, "no neighbor report\n");
				} else {
					for (i = 0; i < report_result.neighbor_report.item_num; i++) {
						print_out(print, "bssid=");
						memcpy(the_mac_addr, report_result.neighbor_report.items[i].bssid, sizeof(qcsapi_mac_addr));
						dump_mac_addr(print, the_mac_addr);
						print_out(print, "BSSID Info = 0x%x\n", report_result.neighbor_report.items[i].bssid_info);
						print_out(print, "operating class = %d\n", report_result.neighbor_report.items[i].operating_class);
						print_out(print, "channel = %d\n", report_result.neighbor_report.items[i].channel);
						print_out(print, "phy_type = %d\n", report_result.neighbor_report.items[i].phy_type);
					}
				}
			}
				break;
			default:
				print_out(print, "%d\n", *p_param_value);
				break;
			}
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wifi_get_node_stats(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int				 statval = 0;
	int				 qcsapi_retval;
	const char			*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output			*print = p_calling_bundle->caller_output;
	qcsapi_unsigned_int		 node_index = p_calling_bundle->caller_generic_parameter.index;
	int				 local_remote_flag = QCSAPI_LOCAL_NODE;
	struct qcsapi_node_stats	 node_stats, *p_node_stats = &node_stats;

	memset(&node_stats, 0, sizeof(node_stats));

	if (argc > 0) {
		if (parse_local_remote_flag(print, argv[0], &local_remote_flag) < 0) {
			return 1;
		}
	}

	if (argc > 1 && strcmp(argv[1], "NULL" ) == 0) {
		p_node_stats = NULL;
	}

	qcsapi_retval = qcsapi_wifi_get_node_stats(the_interface,
						   node_index,
						   local_remote_flag,
						   p_node_stats);
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			if (node_stats.snr < 0) {
				node_stats.snr = (node_stats.snr - QCSAPI_RSSI_OR_SNR_NZERO_CORRECT_VALUE) / QCSAPI_RSSI_OR_SNR_FACTOR;
			} else {
				node_stats.snr = (node_stats.snr + QCSAPI_RSSI_OR_SNR_NZERO_CORRECT_VALUE) / QCSAPI_RSSI_OR_SNR_FACTOR;
			}
			node_stats.snr = (0 - node_stats.snr);

			if (node_stats.rssi < 0) {
				node_stats.rssi = 0;
			} else {
				node_stats.rssi = (qcsapi_unsigned_int)(node_stats.rssi +
							QCSAPI_RSSI_OR_SNR_NZERO_CORRECT_VALUE) / QCSAPI_RSSI_OR_SNR_FACTOR;
			}

			print_out(print, "tx_bytes:\t%llu\n"
				       "tx_pkts:\t%lu\n"
				       "tx_discard:\t%lu\n"
				       "tx_err:\t\t%lu\n"
				       "tx_unicast:\t%lu\n"
				       "tx_multicast:\t%lu\n"
				       "tx_broadcast:\t%lu\n"
				       "tx_phy_rate:\t%lu\n"
				       "rx_bytes:\t%llu\n"
				       "rx_pkts:\t%lu\n"
				       "rx_discard:\t%lu\n"
				       "rx_err:\t\t%lu\n"
				       "rx_unicast:\t%lu\n"
				       "rx_multicast:\t%lu\n"
				       "rx_broadcast:\t%lu\n"
				       "rx_unknown:\t%lu\n"
				       "rx_phy_rate:\t%lu\n"
				       "mac_addr:\t%.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n"
				       "hw_noise:\t%d.%d\n"
				       "snr:\t\t%d\n"
				       "rssi:\t\t%d\n"
				       "bw:\t\t%d\n"
					,
					node_stats.tx_bytes,
					node_stats.tx_pkts,
					node_stats.tx_discard,
					node_stats.tx_err,
					node_stats.tx_unicast,
					node_stats.tx_multicast,
					node_stats.tx_broadcast,
					node_stats.tx_phy_rate,
					node_stats.rx_bytes,
					node_stats.rx_pkts,
					node_stats.rx_discard,
					node_stats.rx_err,
					node_stats.rx_unicast,
					node_stats.rx_multicast,
					node_stats.rx_broadcast,
					node_stats.rx_unknown,
					node_stats.rx_phy_rate,
					node_stats.mac_addr[0],
					node_stats.mac_addr[1],
					node_stats.mac_addr[2],
					node_stats.mac_addr[3],
					node_stats.mac_addr[4],
					node_stats.mac_addr[5],
					(node_stats.hw_noise/10),
					abs(node_stats.hw_noise%10),
					node_stats.snr,
					node_stats.rssi,
					node_stats.bw
				);
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wifi_get_max_queued(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int		 statval = 0;
	int		 qcsapi_retval;
	const char	*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output	*print = p_calling_bundle->caller_output;
	uint32_t	 node_index = p_calling_bundle->caller_generic_parameter.index;
	int		 local_remote_flag = QCSAPI_LOCAL_NODE;
	int		 reset_flag = 0;
	uint32_t	 max_queued, *p_max_queued = &max_queued;

	if (argc > 0) {
		if (parse_local_remote_flag(print, argv[0], &local_remote_flag) < 0) {
			return 1;
		}
	}

	if (argc > 1) {
		if (!isdigit(*argv[1])) {
			print_err(print, "Invalid format for reset flag\n", argv[1]);
			return 1;
		} else {
			reset_flag = atoi(argv[1]);
		}
	}

	if (argc > 2 && strcmp(argv[2], "NULL") == 0) {
		p_max_queued = NULL;
	}

	qcsapi_retval = qcsapi_wifi_get_max_queued(the_interface,
						   node_index,
						   local_remote_flag,
						   reset_flag,
						   p_max_queued);
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out(print, "%d\n", max_queued);
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wifi_associate(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int		statval = 0;
	int		qcsapi_retval;
	const char	*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output	*print = p_calling_bundle->caller_output;

	if (argc < 1) {
		print_err( print, "Not enough parameters in call qcsapi associate,"
				 " count is %d\n", argc );
		statval = 1;
	} else {
		char *join_ssid = argv[0];

		qcsapi_retval = qcsapi_wifi_associate(the_interface, join_ssid);
		if (qcsapi_retval >= 0) {
			if (verbose_flag >= 0) {
				print_out( print, "complete\n");
			}
		} else {
			report_qcsapi_error(p_calling_bundle, qcsapi_retval);
			statval = 1;
		}
	}

	return( statval );
}

static int
call_qcsapi_wifi_disassociate(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int	statval = 0;
	int		qcsapi_retval;
	const char	*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output	*print = p_calling_bundle->caller_output;

	qcsapi_retval = qcsapi_wifi_disassociate(the_interface);
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "complete\n");
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_disassociate_sta(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int                  statval = 1;
	int                  qcsapi_retval;
	const char*          the_interface = p_calling_bundle->caller_interface;
	qcsapi_output*       print = p_calling_bundle->caller_output;
	qcsapi_mac_addr      mac_addr = {0};

	if (argc < 1) {
		print_err( print, "MAC address required to be passed as a parameter\n");
	} else {
		qcsapi_retval = parse_mac_addr( argv[ 0 ], mac_addr );

		if (qcsapi_retval >= 0) {
			qcsapi_retval = qcsapi_wifi_disassociate_sta(the_interface, mac_addr);
			if (qcsapi_retval >= 0) {
				statval = 0;

				if (verbose_flag >= 0) {
					print_out( print, "complete\n");
				}
			} else {
				report_qcsapi_error(p_calling_bundle, qcsapi_retval);
			}
		} else {
			print_out( print, "Error parsing MAC address %s\n", argv[ 0 ] );
		}
	}

	return statval;
}

static int
call_qcsapi_wifi_reassociate(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval;
	const char* the_interface = p_calling_bundle->caller_interface;
	qcsapi_output* print = p_calling_bundle->caller_output;

	qcsapi_retval = qcsapi_wifi_reassociate(the_interface);
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out(print, "complete\n");
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_SSID_create_SSID( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	int		 qcsapi_retval;
	const char	*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output	*print = p_calling_bundle->caller_output;

	if (argc < 1)
	{
		print_err( print, "Not enough parameters in call qcsapi SSID create SSID, count is %d\n", argc );
		statval = 1;
	}
	else
	{
		char	*new_SSID = argv[ 0 ];
	  /*
	   * For create SSID, require the Force NULL address flag to be set, so NULL can be used as an SSID.
	   */
		if (((internal_flags & m_force_NULL_address) == m_force_NULL_address) &&
		    (strcmp( argv[ 0 ], "NULL" ) == 0))
		  new_SSID = NULL;

		qcsapi_retval = qcsapi_SSID_create_SSID( the_interface, new_SSID );
		if (qcsapi_retval >= 0)
		{
			if (verbose_flag >= 0)
			{
				print_out( print, "complete\n" );
			}
		}
		else
		{
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}
	}

	return( statval );
}

static int
call_qcsapi_SSID_remove_SSID(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1) {
		print_err(print, "Not enough parameters in call qcsapi SSID remove SSID, count is %d\n", argc);
		statval = 1;
	} else {
		char *del_SSID = argv[0];
		/*
		* For remove SSID, require the Force NULL address flag to be set, so NULL can be used as an SSID.
		*/
		if (((internal_flags & m_force_NULL_address) == m_force_NULL_address) &&
			(strcmp( argv[0], "NULL" ) == 0))
		  del_SSID = NULL;

		qcsapi_retval = qcsapi_SSID_remove_SSID(the_interface, del_SSID);
		if (qcsapi_retval >= 0) {
			if (verbose_flag >= 0) {
				print_out(print, "complete\n");
			}
		} else {
			report_qcsapi_error(p_calling_bundle, qcsapi_retval);
			statval = 1;
		}
	}

	return (statval);
}

static int
call_qcsapi_SSID_verify_SSID( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	int		 qcsapi_retval;
	const char	*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output	*print = p_calling_bundle->caller_output;

	if (argc < 1)
	{
		print_err( print, "Not enough parameters in call qcsapi SSID verify SSID, count is %d\n", argc );
		statval = 1;
	}
	else
	{
		char	*existing_SSID = argv[ 0 ];
	  /*
	   * For verify SSID, require the Force NULL address flag to be set, so NULL can be used as an SSID.
	   */
		if (((internal_flags & m_force_NULL_address) == m_force_NULL_address) &&
		    (strcmp( argv[ 0 ], "NULL" ) == 0))
		  existing_SSID = NULL;

		qcsapi_retval = qcsapi_SSID_verify_SSID( the_interface, existing_SSID );
		if (qcsapi_retval >= 0)
		{
			if (verbose_flag >= 0)
			{
				print_out( print, "complete\n" );
			}
		}
		else
		{
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}
	}

	return( statval );
}

static int
call_qcsapi_SSID_rename_SSID( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1)
	{
		print_err( print, "Not enough parameters in call qcsapi SSID rename SSID, count is %d\n", argc );
		statval = 1;
	}
	else
	{
		int		 qcsapi_retval;
		char		*new_SSID = argv[ 0 ];
		const char	*the_interface = p_calling_bundle->caller_interface;
		char		*existing_SSID = p_calling_bundle->caller_generic_parameter.parameter_type.the_SSID;
	  /*
	   * For rename SSID, require the Force NULL address flag to be set, so NULL can be used as an SSID.
	   */
		if (((internal_flags & m_force_NULL_address) == m_force_NULL_address) &&
		    (strcmp( existing_SSID, "NULL" ) == 0))
		  existing_SSID = NULL;
		if (((internal_flags & m_force_NULL_address) == m_force_NULL_address) &&
		    (strcmp( new_SSID, "NULL" ) == 0))
		  new_SSID = NULL;

		qcsapi_retval = qcsapi_SSID_rename_SSID( the_interface, existing_SSID, new_SSID );
		if (qcsapi_retval >= 0)
		{
			if (verbose_flag >= 0)
			{
				print_out( print, "complete\n" );
			}
		}
		else
		{
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}
	}

	return( statval );
}

#define DEFAULT_SSID_LIST_SIZE	2
#define MAX_SSID_LIST_SIZE	10

static int
call_qcsapi_SSID_get_SSID_list( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

  /*
   *  array_SSIDs has the space that receives the SSIDs from the API.
   *  Let this get as large as required, without affecting the integrity of the stack.
   */
	static qcsapi_SSID	 array_ssids[MAX_SSID_LIST_SIZE];

	int			 qcsapi_retval;
	unsigned int		 iter;
	const char		*the_interface = p_calling_bundle->caller_interface;
	qcsapi_unsigned_int	 sizeof_list = DEFAULT_SSID_LIST_SIZE;
	char			*list_ssids[MAX_SSID_LIST_SIZE + 1];

	if (argc > 0) {
		if (!isdigit(*argv[0])) {
			print_err(print,
				 "SSID Get List of (configured) SSIDs: size of list must be a numeric value\n");
			return 1;
		}

		sizeof_list = atoi(argv[0]);

		if (sizeof_list > MAX_SSID_LIST_SIZE) {
			print_err(print,
				 "SSID Get List of (configured) SSIDs: cannot exceed max list size of %d\n",
				  MAX_SSID_LIST_SIZE);
			return 1;
		}
	}

	for (iter = 0; iter < sizeof_list; iter++) {
		list_ssids[iter] = array_ssids[iter];
		*(list_ssids[iter]) = '\0';
	}

	qcsapi_retval = qcsapi_SSID_get_SSID_list(the_interface, sizeof_list, &list_ssids[0]);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			for (iter = 0; iter < sizeof_list; iter++) {
				if ((list_ssids[iter] == NULL) || strlen(list_ssids[iter]) < 1) {
					break;
				}

				print_out(print, "%s\n", list_ssids[iter]);
			}
		}
	} else {
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_SSID_get_protocol( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	string_16	 SSID_proto;
	char		*p_SSID_proto = NULL;
	int		 qcsapi_retval;
	const char	*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output	*print = p_calling_bundle->caller_output;
	char		*p_SSID = p_calling_bundle->caller_generic_parameter.parameter_type.the_SSID;

	if (((internal_flags & m_force_NULL_address) == m_force_NULL_address) &&
	    (strcmp( p_SSID, "NULL" ) == 0))
	  p_SSID = NULL;
	if (argc < 1 || strcmp( argv[ 0 ], "NULL" ) != 0)
	  p_SSID_proto = &SSID_proto[ 0 ];
	qcsapi_retval = qcsapi_SSID_get_protocol( the_interface, p_SSID, p_SSID_proto );

	if (qcsapi_retval >= 0)
	{
		if (verbose_flag >= 0)
		{
			print_out( print, "%s\n", &SSID_proto[ 0 ] );
		}
	}
	else
	{
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_SSID_get_encryption_modes( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	char		 encryption_modes[ 36 ], *p_encryption_modes = NULL;
	int		 qcsapi_retval;
	const char	*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output	*print = p_calling_bundle->caller_output;
	char		*p_SSID = p_calling_bundle->caller_generic_parameter.parameter_type.the_SSID;

	if (((internal_flags & m_force_NULL_address) == m_force_NULL_address) &&
	    (strcmp( p_SSID, "NULL" ) == 0))
	  p_SSID = NULL;
	if (argc < 1 || strcmp( argv[ 0 ], "NULL" ) != 0)
	  p_encryption_modes = &encryption_modes[ 0 ];
	qcsapi_retval = qcsapi_SSID_get_encryption_modes( the_interface, p_SSID, p_encryption_modes );

	if (qcsapi_retval >= 0)
	{
		if (verbose_flag >= 0)
		{
			print_out( print, "%s\n", &encryption_modes[ 0 ] );
		}
	}
	else
	{
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_SSID_get_group_encryption( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	char		 group_encryption[ 36 ], *p_group_encryption = NULL;
	int		 qcsapi_retval;
	const char	*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output	*print = p_calling_bundle->caller_output;
	char		*p_SSID = p_calling_bundle->caller_generic_parameter.parameter_type.the_SSID;

	if (((internal_flags & m_force_NULL_address) == m_force_NULL_address) &&
	    (strcmp( p_SSID, "NULL" ) == 0))
	  p_SSID = NULL;
	if (argc < 1 || strcmp( argv[ 0 ], "NULL" ) != 0)
	  p_group_encryption = &group_encryption[ 0 ];
	qcsapi_retval = qcsapi_SSID_get_group_encryption( the_interface, p_SSID, p_group_encryption );

	if (qcsapi_retval >= 0)
	{
		if (verbose_flag >= 0)
		{
			print_out( print, "%s\n", &group_encryption[ 0 ] );
		}
	}
	else
	{
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_SSID_get_authentication_mode( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	char		 authentication_mode[ 36 ], *p_authentication_mode = NULL;
	int		 qcsapi_retval;
	const char	*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output	*print = p_calling_bundle->caller_output;
	char		*p_SSID = p_calling_bundle->caller_generic_parameter.parameter_type.the_SSID;

	if (((internal_flags & m_force_NULL_address) == m_force_NULL_address) &&
	    (strcmp( p_SSID, "NULL" ) == 0))
	  p_SSID = NULL;
	if (argc < 1 || strcmp( argv[ 0 ], "NULL" ) != 0)
	  p_authentication_mode = &authentication_mode[ 0 ];
	qcsapi_retval = qcsapi_SSID_get_authentication_mode( the_interface, p_SSID, p_authentication_mode );

	if (qcsapi_retval >= 0)
	{
		if (verbose_flag >= 0)
		{
			print_out( print, "%s\n", &authentication_mode[ 0 ] );
		}
	}
	else
	{
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_SSID_set_protocol( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1)
	{
		print_err( print, "Not enough parameters in call qcsapi SSID set protocol, count is %d\n", argc );
		statval = 1;
	}
	else
	{
		int		 qcsapi_retval;
		const char	*the_interface = p_calling_bundle->caller_interface;
		char		*p_SSID = p_calling_bundle->caller_generic_parameter.parameter_type.the_SSID;
		char		*p_SSID_proto = argv[ 0 ];

		if (((internal_flags & m_force_NULL_address) == m_force_NULL_address) &&
		    (strcmp( p_SSID, "NULL" ) == 0))
		  p_SSID = NULL;

	    /* SSID protocol will not be NULL ... */

		if (strcmp( argv[ 0 ], "NULL" ) == 0)
		  p_SSID_proto = NULL;
		qcsapi_retval = qcsapi_SSID_set_protocol( the_interface, p_SSID, p_SSID_proto );

		if (qcsapi_retval >= 0)
		{
			if (verbose_flag >= 0)
			{
				print_out( print, "complete\n" );
			}
		}
		else
		{
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}
	}

	return( statval );
}

static int
call_qcsapi_SSID_set_encryption_modes( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1)
	{
		print_err( print, "Not enough parameters in call qcsapi SSID set encryption modes, count is %d\n", argc );
		statval = 1;
	}
	else
	{
		int		 qcsapi_retval;
		const char	*the_interface = p_calling_bundle->caller_interface;
		char		*p_SSID = p_calling_bundle->caller_generic_parameter.parameter_type.the_SSID;
		char		*p_encryption_modes = argv[ 0 ];

		if (((internal_flags & m_force_NULL_address) == m_force_NULL_address) &&
		    (strcmp( p_SSID, "NULL" ) == 0))
		  p_SSID = NULL;

	  /* Encryption modes will not be NULL ... */

		if (strcmp( argv[ 0 ], "NULL" ) == 0)
		  p_encryption_modes = NULL;
		qcsapi_retval = qcsapi_SSID_set_encryption_modes( the_interface, p_SSID, p_encryption_modes );

		if (qcsapi_retval >= 0)
		{
			if (verbose_flag >= 0)
			{
				print_out( print, "complete\n" );
			}
		}
		else
		{
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}
	}

	return( statval );
}

static int
call_qcsapi_SSID_set_group_encryption( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1)
	{
		print_err( print, "Not enough parameters in call qcsapi SSID set group encryption\n" );
		print_err( print, "Usage: call_qcsapi SSID_set_group_encryption <WiFi interface> <SSID> <\"TKIP\"|\"CCMP\">\n" );
		statval = 1;
	}
	else
	{
		int		 qcsapi_retval;
		const char	*the_interface = p_calling_bundle->caller_interface;
		char		*p_SSID = p_calling_bundle->caller_generic_parameter.parameter_type.the_SSID;
		char		*p_group_encryption = argv[ 0 ];

		if (((internal_flags & m_force_NULL_address) == m_force_NULL_address) &&
		    (strcmp( p_SSID, "NULL" ) == 0))
		  p_SSID = NULL;

	  /* Group encryption will not be NULL ... */

		if (strcmp( argv[ 0 ], "NULL" ) == 0)
		  p_group_encryption = NULL;
		qcsapi_retval = qcsapi_SSID_set_group_encryption( the_interface, p_SSID, p_group_encryption );

		if (qcsapi_retval >= 0)
		{
			if (verbose_flag >= 0)
			{
				print_out( print, "complete\n" );
			}
		}
		else
		{
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}
	}

	return( statval );
}

static int
call_qcsapi_SSID_set_authentication_mode( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1)
	{
		print_err( print, "Not enough parameters in call qcsapi SSID set authentication mode, count is %d\n", argc );
		statval = 1;
	}
	else
	{
		int		 qcsapi_retval;
		const char	*the_interface = p_calling_bundle->caller_interface;
		char		*p_SSID = p_calling_bundle->caller_generic_parameter.parameter_type.the_SSID;
		char		*p_authentication_mode = argv[ 0 ];

		if (((internal_flags & m_force_NULL_address) == m_force_NULL_address) &&
		    (strcmp( p_SSID, "NULL" ) == 0))
		  p_SSID = NULL;

	  /* Authentication mode will not be NULL ... */

		if (strcmp( argv[ 0 ], "NULL" ) == 0)
		  p_authentication_mode = NULL;
		qcsapi_retval = qcsapi_SSID_set_authentication_mode( the_interface, p_SSID, p_authentication_mode );

		if (qcsapi_retval >= 0)
		{
			if (verbose_flag >= 0)
			{
				print_out( print, "complete\n" );
			}
		}
		else
		{
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}
	}

	return( statval );
}

static int
call_qcsapi_SSID_get_pre_shared_key( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

  /*
   * Argument list needs to have the index.
   */
	if (argc < 1)
	{
		print_err( print, "Not enough parameters in call qcsapi SSID get key passphrase, count is %d\n", argc );
		statval = 1;
	}
	else
	{
		char			 pre_shared_key[ 68 ], *p_pre_shared_key = NULL;
		int			 qcsapi_retval;
		const char		*the_interface = p_calling_bundle->caller_interface;
		char			*p_SSID = p_calling_bundle->caller_generic_parameter.parameter_type.the_SSID;
		qcsapi_unsigned_int	 the_index = (qcsapi_unsigned_int) atoi( argv[ 0 ] );

		if (((internal_flags & m_force_NULL_address) == m_force_NULL_address) &&
		    (strcmp( p_SSID, "NULL" ) == 0))
		  p_SSID = NULL;

		if (argc < 2 || strcmp( argv[ 1 ], "NULL" ) != 0)
		  p_pre_shared_key = &pre_shared_key[ 0 ];
		qcsapi_retval = qcsapi_SSID_get_pre_shared_key( the_interface, p_SSID, the_index, p_pre_shared_key );

		if (qcsapi_retval >= 0)
		{
			if (verbose_flag >= 0)
			{
				print_out( print, "%s\n", &pre_shared_key[ 0 ] );
			}
		}
		else
		{
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}
	}

	return( statval );
}

static int
call_qcsapi_SSID_get_key_passphrase( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

  /*
   * Argument list needs to have the index.
   */
	if (argc < 1)
	{
		print_err( print, "Not enough parameters in call qcsapi SSID get key passphrase, count is %d\n", argc );
		statval = 1;
	}
	else
	{
		char			 passphrase[ 68 ], *p_passphrase = NULL;
		int			 qcsapi_retval;
		const char		*the_interface = p_calling_bundle->caller_interface;
		char			*p_SSID = p_calling_bundle->caller_generic_parameter.parameter_type.the_SSID;
		qcsapi_unsigned_int	 the_index = (qcsapi_unsigned_int) atoi( argv[ 0 ] );

		if (((internal_flags & m_force_NULL_address) == m_force_NULL_address) &&
		    (strcmp( p_SSID, "NULL" ) == 0))
		  p_SSID = NULL;

		if (argc < 2 || strcmp( argv[ 1 ], "NULL" ) != 0)
		  p_passphrase = &passphrase[ 0 ];
		qcsapi_retval = qcsapi_SSID_get_key_passphrase( the_interface, p_SSID, the_index, p_passphrase );

		if (qcsapi_retval >= 0)
		{
			if (verbose_flag >= 0)
			{
				print_out( print, "%s\n", &passphrase[ 0 ] );
			}
		}
		else
		{
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}
	}

	return( statval );
}

static int
call_qcsapi_SSID_set_pre_shared_key( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

  /*
   * Argument list needs to have both the index and the PSK.
   */
	if (argc < 2)
	{
		print_err( print, "Not enough parameters in call qcsapi SSID set key passphrase, count is %d\n", argc );
		statval = 1;
	}
	else
	{
		int			 qcsapi_retval;
		const char		*the_interface = p_calling_bundle->caller_interface;
		char			*p_SSID = p_calling_bundle->caller_generic_parameter.parameter_type.the_SSID;
		qcsapi_unsigned_int	 the_index = (qcsapi_unsigned_int) atoi( argv[ 0 ] );
		char			*p_PSK = argv[ 1 ];

		if (((internal_flags & m_force_NULL_address) == m_force_NULL_address) &&
		    (strcmp( p_SSID, "NULL" ) == 0))
		  p_SSID = NULL;

	  /* PSK will not be NULL.  */

		if (strcmp( argv[ 1 ], "NULL" ) == 0)
		  p_PSK = NULL;
		qcsapi_retval = qcsapi_SSID_set_pre_shared_key( the_interface, p_SSID, the_index, p_PSK );

		if (qcsapi_retval >= 0)
		{
			if (verbose_flag >= 0)
			{
				print_out( print, "complete\n" );
			}
		}
		else
		{
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}
	}

	return( statval );
}

static int
call_qcsapi_wifi_add_radius_auth_server_cfg(call_qcsapi_bundle *p_calling_bundle,
					    int argc, char *argv[])
{
	int statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 3) {
		print_err( print, "Not enough parameters in call qcsapi add radius auth server cfg,"
				  " count is %d\n", argc);
		print_err( print, "Usage: call_qcsapi add_radius_auth_server_cfg "
				  "<WiFi interface> <ipaddr> <port> <shared-key>\n");
		statval = 1;
	} else {
		int qcsapi_retval = 0;
		const char *the_interface = p_calling_bundle->caller_interface;
		char *p_radius_auth_server_ipaddr = argv[0];
		char *p_radius_auth_server_port = argv[1];
		char *p_radius_auth_server_sh_key = argv[2];

		qcsapi_retval = qcsapi_wifi_add_radius_auth_server_cfg(the_interface,
							p_radius_auth_server_ipaddr,
							p_radius_auth_server_port,
							p_radius_auth_server_sh_key);

		if (qcsapi_retval >= 0) {
			if (verbose_flag >= 0) {
				print_out( print, "complete\n" );
			}
		} else {
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}
	}

	return( statval );
}

static int
call_qcsapi_wifi_del_radius_auth_server_cfg(call_qcsapi_bundle *p_calling_bundle,
					    int argc, char *argv[])
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 2) {
		print_err( print, "Not enough parameters in call qcsapi add radius auth server cfg,"
				  " count is %d\n", argc);
		print_err( print, "Usage: call_qcsapi add_radius_auth_server_cfg "
				  "<WiFi interface> <ipaddr> <port> \n");
		statval = 1;
	} else {
		int qcsapi_retval = 0;
		const char *the_interface = p_calling_bundle->caller_interface;
		char *p_radius_auth_server_ipaddr = argv[0];
		char *p_radius_auth_server_port = argv[1];

		qcsapi_retval = qcsapi_wifi_del_radius_auth_server_cfg(the_interface,
							p_radius_auth_server_ipaddr,
							p_radius_auth_server_port);

		if (qcsapi_retval >= 0) {
			if (verbose_flag >= 0) {
				print_out( print, "complete\n" );
			}
		} else {
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}
	}

	return( statval );
}

static int
call_qcsapi_wifi_get_radius_auth_server_cfg(call_qcsapi_bundle *p_calling_bundle,
					    int argc, char *argv[])
{
	int	statval = 0;
	int	qcsapi_retval = 0;
	qcsapi_output   *print = p_calling_bundle->caller_output;
	const char	*the_interface = p_calling_bundle->caller_interface;
	string_1024	radius_auth_server_cfg;
	char *p_radius_auth_server_cfg = &radius_auth_server_cfg[0];

	qcsapi_retval = qcsapi_wifi_get_radius_auth_server_cfg(the_interface,
						p_radius_auth_server_cfg);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "%s\n", p_radius_auth_server_cfg);
		}
	} else {
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_set_own_ip_addr( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1)
	{
		print_err( print, "Not enough parameters in call qcsapi set own ip addr, count is %d\n", argc);
		print_err( print, "Usage: call_qcsapi set_own_ip_addr <WiFi interface> <ipaddr>\n");
		statval = 1;
	}
	else
	{
		int			qcsapi_retval;
		const char		*the_interface = p_calling_bundle->caller_interface;
		char			*p_own_ip_addr = argv[ 0 ];

		if (strcmp( argv[ 0 ], "NULL" ) == 0)
			p_own_ip_addr = NULL;
		qcsapi_retval = qcsapi_wifi_set_own_ip_addr( the_interface, p_own_ip_addr );

		if (qcsapi_retval >= 0)
		{
			if (verbose_flag >= 0)
			{
				print_out( print, "complete\n" );
			}
		}
		else
		{
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}
	}

	return( statval );
}

static int
call_qcsapi_SSID_set_key_passphrase( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

  /*
   * Argument list needs to have both the index and the passphrase.
   */
	if (argc < 2)
	{
		print_err( print, "Not enough parameters in call qcsapi SSID set key passphrase, count is %d\n", argc );
		statval = 1;
	}
	else
	{
		int			 qcsapi_retval;
		const char		*the_interface = p_calling_bundle->caller_interface;
		char			*p_SSID = p_calling_bundle->caller_generic_parameter.parameter_type.the_SSID;
		qcsapi_unsigned_int	 the_index = (qcsapi_unsigned_int) atoi( argv[ 0 ] );
		char			*p_passphrase = argv[ 1 ];

		if (((internal_flags & m_force_NULL_address) == m_force_NULL_address) &&
		    (strcmp( p_SSID, "NULL" ) == 0))
		  p_SSID = NULL;

	  /* Passphrase of NULL is not valid.  */

		if (strcmp( argv[ 1 ], "NULL" ) == 0)
		  p_passphrase = NULL;
		qcsapi_retval = qcsapi_SSID_set_key_passphrase( the_interface, p_SSID, the_index, p_passphrase );

		if (qcsapi_retval >= 0)
		{
			if (verbose_flag >= 0)
			{
				print_out( print, "complete\n" );
			}
		}
		else
		{
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}
	}

	return( statval );
}

static int
call_qcsapi_SSID_get_pmf( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	int pmf_cap, *p_pmf_cap = NULL;
	int		 qcsapi_retval;
	const char	*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output	*print = p_calling_bundle->caller_output;
	char		*p_SSID = p_calling_bundle->caller_generic_parameter.parameter_type.the_SSID;

	if (((internal_flags & m_force_NULL_address) == m_force_NULL_address) &&
	    (strcmp( p_SSID, "NULL" ) == 0))
		p_SSID = NULL;

	if (argc < 1 || strcmp( argv[ 0 ], "NULL" ) != 0)
		p_pmf_cap = &pmf_cap;

	qcsapi_retval = qcsapi_SSID_get_pmf( the_interface, p_SSID, p_pmf_cap);

	if (qcsapi_retval >= 0)
	{
		if (verbose_flag >= 0)
		{
			print_out( print, "%d\n", pmf_cap );
		}
	}
	else
	{
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_SSID_set_pmf( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;
        int qcsapi_retval;
        const char *the_interface = p_calling_bundle->caller_interface;
        char *p_SSID = p_calling_bundle->caller_generic_parameter.parameter_type.the_SSID;

	if (argc < 1)
	{
		print_err( print, "Not enough parameters in call qcsapi SSID set pmf mode, count is %d\n", argc );
		statval = 1;
	}
	else
	{
		qcsapi_unsigned_int	 pmf_cap = atoi( argv[ 0 ] );

		if (((internal_flags & m_force_NULL_address) == m_force_NULL_address) &&
		    (strcmp( p_SSID, "NULL" ) == 0))
		  p_SSID = NULL;

		qcsapi_retval = qcsapi_SSID_set_pmf( the_interface, p_SSID, pmf_cap );

		if (qcsapi_retval >= 0)
		{
			if (verbose_flag >= 0)
			{
				print_out( print, "complete\n" );
			}
		}
		else
		{
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}
	}

	return( statval );
}

static int
call_qcsapi_SSID_get_wps_SSID( const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_SSID	 the_wps_SSID = "";
	int		 qcsapi_retval;
	const char	*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output	*print = p_calling_bundle->caller_output;

	if (argc > 0 && strcmp( argv[ 0 ], "NULL" ) == 0)
	  qcsapi_retval = qcsapi_SSID_get_wps_SSID( the_interface, NULL );
	else
	  qcsapi_retval = qcsapi_SSID_get_wps_SSID( the_interface, the_wps_SSID );

	if (qcsapi_retval >= 0)
	{
		if (verbose_flag >= 0)
		{
			print_out( print, "%s\n", the_wps_SSID );
		}
	}
	else
	{
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_vlan_config(const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval;
	qcsapi_vlan_cmd cmd;
	uint32_t vlanid;
	uint32_t flags = 0;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 2) {
		print_err(print, "Not enough parameters in call qcsapi vlan_conf\n");
		statval = 1;
		goto usage;
	} else {

		if (!strcasecmp(argv[0], "bind")) {
			cmd = e_qcsapi_vlan_bind;
			vlanid = atoi(argv[1]);
			if (argc == 3) {
				flags = atoi(argv[2]);
			}
		} else if (!strcasecmp(argv[0], "unbind")) {
			cmd = e_qcsapi_vlan_unbind;
			vlanid = atoi(argv[1]);
		} else if (!strcasecmp(argv[0], "passthru")) {
			cmd = e_qcsapi_vlan_passthru;
			if (strcmp(argv[1], "all") == 0)
				vlanid = QVLAN_VID_ALL;
			else
				vlanid = atoi(argv[1]);
		} else if (!strcasecmp(argv[0], "unpassthru")) {
			cmd = e_qcsapi_vlan_unpassthru;
			if (strcmp(argv[1], "all") == 0)
				vlanid = QVLAN_VID_ALL;
			else
				vlanid = atoi(argv[1]);
		} else if (!strcasecmp(argv[0], "dynamic")) {
			vlanid = atoi(argv[1]);
			if (!vlanid)
				cmd = e_qcsapi_vlan_undynamic;
			else
				cmd = e_qcsapi_vlan_dynamic;
		} else if (!strcasecmp(argv[0], "enable")) {
			cmd = e_qcsapi_vlan_enable;
			vlanid = QVLAN_VID_ALL;
		} else if (!strcasecmp(argv[0], "disable")) {
			cmd = e_qcsapi_vlan_disable;
			vlanid = QVLAN_VID_ALL;
		} else {
			statval = 1;
			goto usage;
		}

		qcsapi_retval = qcsapi_wifi_vlan_config(the_interface, cmd, vlanid, flags);

		if (qcsapi_retval >= 0) {
			if (verbose_flag >= 0) {
				print_out(print, "complete\n");
			}
		} else {
			report_qcsapi_error(p_calling_bundle, qcsapi_retval);
			statval = 1;
		}
	}

	return statval;

usage:
	print_err(print, "Usage: call_qcsapi vlan_config <WiFi interface>");
	print_err(print, "              <enable | disable | bind | unbind | passthru | unpassthru | dynamic>");
	print_err(print, "              <vlanID | all>\n");

	return statval;
}

static void
call_qcsapi_wifi_print_vlan_config(const call_qcsapi_bundle *p_calling_bundle, const char *ifname, string_1024 str)
{
	qcsapi_output *print = p_calling_bundle->caller_output;
	struct qtn_vlan_config *vcfg = (struct qtn_vlan_config *)str;
	uint16_t vmode;
	uint16_t vid;
	uint16_t i;

	vmode = qtn_vlancfg_reform(vcfg);
	if (vmode == QVLAN_MODE_PTHRU) {
		print_out(print, "%s VLAN(s):", QVLAN_MODE_STR_PTHRU);
		for (i = 0; i < QVLAN_VID_MAX; i++) {
			if (vcfg->vlan_bitmap[i / 7] & (1 << (i % 7)))
				print_out(print, "%u,", i);
		}
		print_out(print, "\n");
	} else if (vmode == QVLAN_MODE_MBSS) {
		vid = (vcfg->vlan_cfg & QVLAN_MASK_VID);
		print_out(print, "%s, bound to VLAN %u\n", QVLAN_MODE_STR_BIND, vid);
	} else if (vmode == QVLAN_MODE_DYNAMIC) {
		print_out(print, "%s\n", QVLAN_MODE_STR_DYNAMIC);
	} else {
		print_out(print, "VLAN disabled\n");
	}
}

static int
call_qcsapi_wifi_show_vlan_config(const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int qcsapi_retval = 0;
	struct qtn_vlan_config *vcfg;
	const char *ifname = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;

	COMPILE_TIME_ASSERT(sizeof(string_1024) > sizeof(struct qtn_vlan_config));

	if (argc > 0) {
		print_err(print, "Too many parameters for show_vlan_config command\n");
		qcsapi_retval = 1;
	} else {
		vcfg = (struct qtn_vlan_config *)malloc(sizeof(struct qtn_vlan_config));
		if (!vcfg) {
			print_err(print, "Not enough memory to execute the API\n");
			return -1;
		}

		memset(vcfg, 0, sizeof(*vcfg));
		qcsapi_retval = qcsapi_wifi_show_vlan_config(ifname, (char *)vcfg);
		if (qcsapi_retval < 0) {
			report_qcsapi_error(p_calling_bundle, qcsapi_retval);
			qcsapi_retval = 1;
		} else {
			call_qcsapi_wifi_print_vlan_config(p_calling_bundle, ifname, (char *)vcfg);
			qcsapi_retval = 0;
		}
		free(vcfg);
	}

	return qcsapi_retval;
}

static int
call_qcsapi_enable_wlan_pass_through(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval;
	const char *ifname = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;
	int enabled = !!atoi(argv[0]);

	qcsapi_retval = qcsapi_enable_vlan_pass_through(ifname, enabled);
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "complete\n");
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_enable_vlan_promisc(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval;
	qcsapi_output *print = p_calling_bundle->caller_output;
	int enabled = !!atoi(argv[0]);

	qcsapi_retval = qcsapi_wifi_set_vlan_promisc(enabled);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "complete\n");
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_set_ipff(call_qcsapi_bundle *p_calling_bundle, int add, int argc, char *argv[])
{
	int qcsapi_retval;
	qcsapi_output *print = p_calling_bundle->caller_output;
	uint32_t ipaddr;
	uint32_t ipaddr_ne;
	char *usage = "Usage: call_qcsapi [add_ipff | del_ipff ] <ip_address>\n";

	/* FIXME subnets and IPv6 are not yet supported */

	if (argc != 1) {
		print_out(print, usage);
		return -EINVAL;
	}

	if (inet_pton(AF_INET, argv[0], &ipaddr_ne) != 1) {
		print_err(print, "invalid IPv4 address %s\n", argv[0]);
		return -EINVAL;
	}
	ipaddr = ntohl(ipaddr_ne);

	if (!IN_MULTICAST(ipaddr)) {
		print_err(print, "invalid multicast IPv4 address " NIPQUAD_FMT "\n",
			NIPQUAD(ipaddr_ne));
		return -EINVAL;
	}

	if (add) {
		qcsapi_retval = qcsapi_wifi_add_ipff(ipaddr);
	} else {
		qcsapi_retval = qcsapi_wifi_del_ipff(ipaddr);
	}

	if (qcsapi_retval < 0) {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		return 1;
	}

	if (verbose_flag >= 0) {
		print_out(print, "complete\n");
	}

	return 0;
}

static int
call_qcsapi_get_ipff(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	qcsapi_output *print = p_calling_bundle->caller_output;
#define QCSAPI_IPFF_GET_MAX	256
	char buf[IP_ADDR_STR_LEN * QCSAPI_IPFF_GET_MAX];

	qcsapi_wifi_get_ipff(buf, sizeof(buf));

	print_out(print, "%s", buf);

	if (verbose_flag >= 0) {
		print_out( print, "complete\n");
	}

	return 0;
}

static int
call_qcsapi_wifi_get_rts_threshold(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int qcsapi_retval;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;
	qcsapi_unsigned_int rts_threshold;

	qcsapi_retval = qcsapi_wifi_get_rts_threshold(the_interface, &rts_threshold);
	if (qcsapi_retval >= 0) {
		print_out(print, "%d\n", rts_threshold);
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		return 1;
	}

	return qcsapi_retval;
}

static int
call_qcsapi_wifi_set_rts_threshold(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int qcsapi_retval;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;
	qcsapi_unsigned_int rts_threshold;
	int32_t arg;

	if (sscanf(argv[0], "%d", &arg) != 1) {
		print_err(print, "Error parsing '%s'\n", argv[0]);
		return 1;
	}

	if (arg < IEEE80211_RTS_MIN) {
		print_err(print, "Value should be non negative\n");
		return 1;
	}

	rts_threshold = arg;

	qcsapi_retval = qcsapi_wifi_set_rts_threshold(the_interface, rts_threshold);
	if (qcsapi_retval >= 0) {
		print_out(print, "complete\n");
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		return 1;
	}

	return qcsapi_retval;
}

static int
call_qcsapi_wifi_disable_wps(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int	statval = 0;
	int		qcsapi_retval;
	const char	*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output	*print = p_calling_bundle->caller_output;
	int		disable_wps = atoi(argv[0]);

	qcsapi_retval = qcsapi_wifi_disable_wps(the_interface, disable_wps);
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "complete\n");
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_start_cca(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int qcsapi_retval;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;
	int channel;
	int duration;

	if (argc < 2) {
		print_err( print, "Format: start_cca <chan-num(36)> <msec-duration(40)> \n");
		return(1);
	}

	channel = atoi(argv[0]);
	duration = atoi(argv[1]);

	qcsapi_retval = qcsapi_wifi_start_cca(the_interface, channel, duration);
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "Complete.\n");
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		return 1;
	}

	return 0;
}

static int
call_qcsapi_wifi_start_scan(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int		 statval = 0;
	int		 qcsapi_retval;
	const char	*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output	*print = p_calling_bundle->caller_output;
	int pick_flags = 0;

	if (argc > 0) {
		while (argc > 0) {
			if (!strcasecmp("reentry", argv[0])) {
				pick_flags |= IEEE80211_PICK_REENTRY;
			} else if (!strcasecmp("clearest", argv[0])) {
				pick_flags |= IEEE80211_PICK_CLEAREST;
			} else if (!strcasecmp("no_pick", argv[0])) {
				pick_flags |= IEEE80211_PICK_NOPICK;
			} else if (!strcasecmp("background", argv[0])) {
				pick_flags |= IEEE80211_PICK_NOPICK_BG;
			} else if (!strcasecmp("dfs", argv[0])) {
				pick_flags |= IEEE80211_PICK_DFS;
			} else if (!strcasecmp("non_dfs", argv[0])) {
				pick_flags |= IEEE80211_PICK_NONDFS;
			} else if (!strcasecmp("all", argv[0])) {
				pick_flags |= IEEE80211_PICK_ALL;
			} else if (!strcasecmp("flush", argv[0])) {
				pick_flags |= IEEE80211_PICK_SCAN_FLUSH;
			} else if (!strcasecmp("active", argv[0])) {
				pick_flags |= IEEE80211_PICK_BG_ACTIVE;
			} else if (!strcasecmp("fast", argv[0])) {
				pick_flags |= IEEE80211_PICK_BG_PASSIVE_FAST;
			} else if (!strcasecmp("normal", argv[0])) {
				pick_flags |= IEEE80211_PICK_BG_PASSIVE_NORMAL;
			} else if (!strcasecmp("slow", argv[0])) {
				pick_flags |= IEEE80211_PICK_BG_PASSIVE_SLOW;
			} else {
				goto err_ret;
			}
			argc--;
			argv++;
		}

		if (pick_flags & IEEE80211_PICK_ALGORITHM_MASK) {
			uint32_t algorithm = pick_flags & IEEE80211_PICK_ALGORITHM_MASK;
			uint32_t chan_set = pick_flags & IEEE80211_PICK_DOMIAN_MASK;

			if (IS_MULTIPLE_BITS_SET(algorithm)) {
				print_out(print, "Only one pick algorithm can be specified\n");
				goto err_ret;
			}
			if (chan_set) {
				if (IS_MULTIPLE_BITS_SET(chan_set)) {
					print_out(print, "Only one channel set can be specified\n");
					goto err_ret;
				}
			} else {
				pick_flags |= IEEE80211_PICK_ALL;
			}
		} else {
			print_out(print, "pick algorithm was not specified\n");
			goto err_ret;
		}

		if (pick_flags & IEEE80211_PICK_NOPICK_BG) {
			uint32_t dfs_mode = pick_flags & IEEE80211_PICK_BG_MODE_MASK;

			if (IS_MULTIPLE_BITS_SET(dfs_mode)) {
				print_out(print, "Please specify only one DFS scan mode "
						"from active, (passive)fast, normal and slow\n");
				goto err_ret;
			}
		}
	}

	qcsapi_retval = qcsapi_wifi_start_scan_ext(the_interface, pick_flags);
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "complete\n");
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return( statval );

err_ret:
	print_start_scan_usage(print);
	return 1;
}

static int
call_qcsapi_wifi_cancel_scan(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;
	int force = 0;
	int qcsapi_retval;

	if (argc == 1) {
		if (!strcasecmp("force", argv[0])) {
			force = 1;
		} else {
			print_out(print, "Unknown parameter: %s\n", argv[0]);
			print_cancel_scan_usage(print);
			return 1;
		}
	} else if (argc != 0) {
		print_cancel_scan_usage(print);
		return 1;
	}

	qcsapi_retval = qcsapi_wifi_cancel_scan(the_interface, force);
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "complete\n");
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		return 1;
	}

	return 0;
}

static int
call_qcsapi_wifi_get_scan_status(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;
	int scanstatus = -1;

	qcsapi_retval = qcsapi_wifi_get_scan_status(the_interface, &scanstatus);
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out(print, "%d\n", scanstatus);
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wifi_get_cac_status(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;
	int cacstatus = -1;

	qcsapi_retval = qcsapi_wifi_get_cac_status(the_interface, &cacstatus);
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out(print, "%d\n", cacstatus);
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wifi_wait_scan_completes(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int		 statval = 0;
	int		 qcsapi_retval;
	const char	*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output	*print = p_calling_bundle->caller_output;
	time_t		 timeout;

	if (argc < 1) {
		print_err( print, "Wait Scan Completes requires a timeout\n" );
		return(1);
	}

	timeout = (time_t) atoi( argv[ 0 ] );

	qcsapi_retval = qcsapi_wifi_wait_scan_completes(the_interface, timeout);
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "complete\n");
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_get_results_AP_scan( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int			 statval = 0;
	qcsapi_unsigned_int	 count_APs_scanned, *p_count_APs_scanned = NULL;
	int			 qcsapi_retval;
	const char		*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output		*print = p_calling_bundle->caller_output;

	if (argc < 1 || strcmp( argv[ 0 ], "NULL" ) != 0)
	  p_count_APs_scanned = &count_APs_scanned;
	qcsapi_retval = qcsapi_wifi_get_results_AP_scan( the_interface, p_count_APs_scanned );
	if (qcsapi_retval >= 0)
	{
		if (verbose_flag >= 0)
		{
		  /*
		   * Unlike most APIs that return a value by reference, this API permits
		   * that reference address to be 0.
		   *
		   * Primary purpose of this API is to get the results of the last AP scan.
		   */
			if (p_count_APs_scanned != NULL)
			  print_out( print, "%d\n", (int) count_APs_scanned );
			else
			  print_out( print, "complete\n" );
		}
	}
	else
	{
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_get_count_APs_scanned( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_unsigned_int	 count_APs_scanned, *p_count_APs_scanned = NULL;
	int			 qcsapi_retval;
	const char		*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output		*print = p_calling_bundle->caller_output;

	if (argc < 1 || strcmp( argv[ 0 ], "NULL" ) != 0)
	  p_count_APs_scanned = &count_APs_scanned;
	qcsapi_retval = qcsapi_wifi_get_count_APs_scanned( the_interface, p_count_APs_scanned );
	if (qcsapi_retval >= 0)
	{
		if (verbose_flag >= 0)
		{
			print_out( print, "%d\n", (int) count_APs_scanned );
		}
	}
	else
	{
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_get_properties_AP( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_ap_properties	 ap_properties, *p_ap_properties = NULL;
	int			 qcsapi_retval;
	const char		*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output		*print = p_calling_bundle->caller_output;
	qcsapi_unsigned_int	 ap_index = p_calling_bundle->caller_generic_parameter.index;

	if (argc < 1 || strcmp( argv[ 0 ], "NULL" ) != 0)
	  p_ap_properties = &ap_properties;
	qcsapi_retval = qcsapi_wifi_get_properties_AP( the_interface, ap_index, p_ap_properties );
	if (qcsapi_retval >= 0)
	{
		if (verbose_flag >= 0)
		{
			char	 mac_addr_string[ 24 ];

			snprintf( &mac_addr_string[ 0 ], sizeof(mac_addr_string), MACFILTERINGMACFMT,
				  p_ap_properties->ap_mac_addr[ 0 ],
				  p_ap_properties->ap_mac_addr[ 1 ],
				  p_ap_properties->ap_mac_addr[ 2 ],
				  p_ap_properties->ap_mac_addr[ 3 ],
				  p_ap_properties->ap_mac_addr[ 4 ],
				  p_ap_properties->ap_mac_addr[ 5 ]
			);

			print_out( print, "\"%s\" %s %d %d %x %d %d %d %d %d\n",
				 p_ap_properties->ap_name_SSID,
				 &mac_addr_string[ 0 ],
				 p_ap_properties->ap_channel,
				 p_ap_properties->ap_RSSI,
				 p_ap_properties->ap_flags,
				 p_ap_properties->ap_protocol,
				 p_ap_properties->ap_authentication_mode,
				 p_ap_properties->ap_encryption_modes,
				 p_ap_properties->ap_qhop_role,
				 p_ap_properties->ap_wps
			);
		}
	}
	else
	{
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_get_mcs_rate( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int statval = 0;
	char mcs_rate[16];
	char *p_mcs_rate = NULL;
	int qcsapi_retval;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;

	p_mcs_rate = &mcs_rate[0];
	qcsapi_retval = qcsapi_wifi_get_mcs_rate(the_interface, p_mcs_rate);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0)
			print_out(print, "%s\n", &mcs_rate[0]);
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wifi_set_mcs_rate( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1)
	{
		print_err( print, "Not enough parameters in call qcsapi WiFi set MCS rate, count is %d\n", argc );
		statval = 1;
	}
	else
	{
		int		 qcsapi_retval;
		const char	*the_interface = p_calling_bundle->caller_interface;
		char		*p_mcs_rate = argv[ 0 ];

	  /* MCS rate will not be NULL ... */

		if (strcmp( argv[ 0 ], "NULL" ) == 0)
		  p_mcs_rate = NULL;
		qcsapi_retval = qcsapi_wifi_set_mcs_rate( the_interface, p_mcs_rate );

		if (qcsapi_retval >= 0)
		{
			if (verbose_flag >= 0)
			{
				print_out( print, "complete\n" );
			}
		}
		else
		{
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}
	}

	return( statval );
}

/******************************************************************************
DESCRIPTION: This API returns the time that station has associated with AP.

*******************************************************************************/
static int
call_qcsapi_wifi_get_time_associated_per_association( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	int		 qcsapi_retval = 0;
	qcsapi_unsigned_int	time_associated = 0;
	qcsapi_unsigned_int	*p_time_associated = NULL;
	const char		*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output		*print = p_calling_bundle->caller_output;
	qcsapi_unsigned_int	 association_index = p_calling_bundle->caller_generic_parameter.index;

	if (argc < 1 || strcmp( argv[ 0 ], "NULL" ) != 0)
	  p_time_associated = &time_associated;

	qcsapi_retval = qcsapi_wifi_get_time_associated_per_association(the_interface, association_index, p_time_associated);

	if (qcsapi_retval >= 0)
	{
		if (verbose_flag >= 0)
		{
			print_out( print, "%d\n", time_associated);
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_wds_add_peer(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval = 0;
	qcsapi_mac_addr  the_mac_addr;
	int ival = 0;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;
	qcsapi_unsigned_int encryption = 0;

	if (argc < 1) {
		print_err( print, "Not enough parameters in call qcsapi WiFi wds add peer, count is %d\n", argc);
		statval = 1;
	} else {
		ival = parse_mac_addr(argv[ 0 ], the_mac_addr);
		if ((argc > 1) && (strcasecmp(argv[1], "encrypt") == 0))
			encryption = 1;

		if (ival >= 0) {
			qcsapi_retval = qcsapi_wds_add_peer_encrypt(the_interface, the_mac_addr, encryption);

			if (qcsapi_retval >= 0) {
				if (verbose_flag >= 0) {
					print_out( print, "complete\n");
				}
			} else {
				report_qcsapi_error(p_calling_bundle, qcsapi_retval);
				statval = 1;
			}

		} else {
			print_out( print, "Error parsing MAC address %s\n", argv[ 0 ]);
			statval = 1;
		}
	}

	return statval;
}

static int
call_qcsapi_wifi_wds_remove_peer(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval = 0;
	qcsapi_mac_addr  the_mac_addr;
	int ival = 0;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1) {
		print_err( print,
	   "Not enough parameters in call qcsapi WiFi wds remove peer, count is %d\n", argc );
		statval = 1;
	} else {
		ival = parse_mac_addr(argv[ 0 ], the_mac_addr);

		if (ival >= 0) {
			qcsapi_retval = qcsapi_wds_remove_peer(the_interface, the_mac_addr);

			if (qcsapi_retval >= 0) {
				if (verbose_flag >= 0) {
					print_out( print, "complete\n");
				}
			} else {
				report_qcsapi_error(p_calling_bundle, qcsapi_retval);
				statval = 1;
			}

		} else {
			print_out( print, "Error parsing MAC address %s\n", argv[0]);
			statval = 1;
		}
	}

	return statval;
}

static int
call_qcsapi_wifi_wds_get_peer_address(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval = 0;
	qcsapi_mac_addr peer_address;
	qcsapi_unsigned_int index = 0;
	char temp_peer_address_str[20];
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1) {
		print_err( print,
	   "Not enough parameters in call qcsapi WiFi get peer address, count is %d\n", argc );
		statval = 1;
	} else {
		index = (qcsapi_unsigned_int) atoi(argv[0]);
		qcsapi_retval = qcsapi_wds_get_peer_address(the_interface, index, peer_address);

		if (qcsapi_retval >= 0) {
			if (verbose_flag >= 0) {
				snprintf(&temp_peer_address_str[0], sizeof(temp_peer_address_str),
					  MACFILTERINGMACFMT,
					  peer_address[0],
					  peer_address[1],
					  peer_address[2],
					  peer_address[3],
					  peer_address[4],
					  peer_address[5]);
				print_out( print, "%s\n", temp_peer_address_str);
			}
		} else {
			report_qcsapi_error(p_calling_bundle, qcsapi_retval);
			statval = 1;
		}
	}

	return statval;
}

static int
call_qcsapi_wifi_wds_set_psk(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval = 0;
	qcsapi_mac_addr peer_address;
	char *p_pre_shared_key = NULL;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;
	int ival = 0;

	if (argc < 2) {
		print_err( print,
	   "Not enough parameters in call qcsapi WiFi wds set psk, count is %d\n", argc );
		statval = 1;
	} else {
		ival = parse_mac_addr(argv[0], peer_address);

		if (ival >= 0) {
			p_pre_shared_key = argv[1];
			if (strcmp(p_pre_shared_key, "NULL") == 0) {
				p_pre_shared_key = NULL;
			}
			qcsapi_retval = qcsapi_wds_set_psk(the_interface, peer_address, p_pre_shared_key);

			if (qcsapi_retval >= 0) {
				if (verbose_flag >= 0) {
					print_out( print, "complete\n");
				}
			} else {
				report_qcsapi_error(p_calling_bundle, qcsapi_retval);
				statval = 1;
			}
		} else {
			print_out( print, "Error parsing MAC address %s\n", argv[ 0 ]);
			statval = 1;
		}
	}

	return statval;
}

static int
call_qcsapi_wifi_wds_set_mode(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval = 0;
	qcsapi_mac_addr peer_address;
	int rbs_mode;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;
	int ival = 0;

	if (argc < 2) {
		print_err( print, "Not enough parameters in call qcsapi WiFi wds set "
				"mode, count is %d\n", argc );
		statval = 1;
	} else {
		ival = parse_mac_addr(argv[0], peer_address);

		if (ival >= 0) {
			if (strcasecmp(argv[1], "rbs") == 0) {
				rbs_mode = 1;
			} else if (strcasecmp(argv[1], "mbs") == 0) {
				rbs_mode = 0;
			} else {
				print_out(print, "Error parsing WDS mode %s\n", argv[1]);
				return 1;
			}

			qcsapi_retval = qcsapi_wds_set_mode(the_interface, peer_address, rbs_mode);

			if (qcsapi_retval >= 0) {
				if (verbose_flag >= 0) {
					print_out( print, "complete\n");
				}
			} else {
				report_qcsapi_error(p_calling_bundle, qcsapi_retval);
				statval = 1;
			}
		} else {
			print_out( print, "Error parsing MAC address %s\n", argv[ 0 ]);
			statval = 1;
		}
	}

	return statval;
}

static int
call_qcsapi_wifi_wds_get_mode(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval = 0;
	int rbs_mode;
	qcsapi_unsigned_int index = 0;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;
	const char *mode_str[] = {"mbs", "rbs", "none"};

	if (argc < 1) {
		print_err( print, "Not enough parameters in call qcsapi WiFi get "
			"peer address, count is %d\n", argc );
		statval = 1;
	} else {
		index = (qcsapi_unsigned_int) atoi(argv[0]);
		qcsapi_retval = qcsapi_wds_get_mode(the_interface, index, &rbs_mode);

		if (qcsapi_retval >= 0) {
			if (verbose_flag >= 0) {
				print_out( print, "wds %s\n", mode_str[rbs_mode]);
			}
		} else {
			report_qcsapi_error(p_calling_bundle, qcsapi_retval);
			statval = 1;
		}
	}

	return statval;
}

static int
call_qcsapi_wifi_qos_get_param(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int		statval = 0;
	int		qcsapi_retval = 0;
	const char	*the_interface = p_calling_bundle->caller_interface;
	int		the_queue = -1;
	int		the_param = -1;
	int		ap_bss_flag = 0;
	int		qos_param_value;
	int		i;
	qcsapi_output	*print = p_calling_bundle->caller_output;

	if (argc < 2) {
		print_err(print, "Usage: call_qcsapi qos_get_param <WiFi interface> "
				"<QoS queue> <QoS param> [AP / BSS flag]\n");
		return 1;
	}

	if (isdigit(*argv[0])) {
		the_queue = atoi(argv[0]);
	} else if (name_to_qos_queue_type(argv[0], &the_queue) == 0) {
		print_err(print, "Unrecognized QoS queue %s\n", argv[0]);
		if (verbose_flag >= 0) {
			print_out( print, "Supported QOS queue ID and name:\n" );
			for (i = 0; i < ARRAY_SIZE(qcsapi_qos_queue_table); i++)
				print_out( print, "%d: %s\n",
					qcsapi_qos_queue_table[i].qos_queue_type,
					qcsapi_qos_queue_table[i].qos_queue_name );
		}
		return 1;
	}

	if (isdigit(*argv[1])) {
		the_param = atoi(argv[1]);
	} else if (name_to_qos_param_type(argv[1], &the_param) == 0) {
		print_err(print, "Unrecognized QoS param %s\n", argv[1]);
		if (verbose_flag >= 0) {
			print_out( print, "Supported QOS param ID and name:\n" );
			for (i = 0; i < ARRAY_SIZE(qcsapi_qos_param_table); i++)
				print_out( print, "%d: %s\n",
					qcsapi_qos_param_table[i].qos_param_type,
					qcsapi_qos_param_table[i].qos_param_name );
		}
		return 1;
	}

	if (argc > 2) {
		ap_bss_flag = atoi(argv[2]);
	}

	qcsapi_retval = qcsapi_wifi_qos_get_param(the_interface,
						  the_queue,
						  the_param,
						  ap_bss_flag,
						  &qos_param_value);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out(print, "%d\n", qos_param_value);
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wifi_qos_set_param(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int		statval = 0;
	int		qcsapi_retval = 0;
	const char	*the_interface = p_calling_bundle->caller_interface;
	int		the_queue = -1;
	int		the_param = -1;
	int		ap_bss_flag = 0;
	int		param_value = -1;
	int		i;
	qcsapi_output	*print = p_calling_bundle->caller_output;

	if (argc < 3) {
		print_err(print, "Usage: call_qcsapi qos_get_param <WiFi interface> "
				"<QoS queue> <QoS param> <value> [AP / BSS flag]\n");
		return 1;
	}

	if (isdigit(*argv[0])) {
		the_queue = atoi(argv[0]);
	} else if (name_to_qos_queue_type(argv[0], &the_queue) == 0) {
		print_err(print, "Unrecognized QoS queue %s\n", argv[0]);
		if (verbose_flag >= 0) {
			print_out( print, "Supported QOS queue ID and name:\n" );
			for (i = 0; i < ARRAY_SIZE(qcsapi_qos_queue_table); i++)
				print_out( print, "%d: %s\n",
					qcsapi_qos_queue_table[i].qos_queue_type,
					qcsapi_qos_queue_table[i].qos_queue_name );
		}
		return 1;
	}

	if (isdigit(*argv[1])) {
		the_param = atoi(argv[1]);
	} else if (name_to_qos_param_type(argv[1], &the_param) == 0) {
		print_err(print, "Unrecognized QoS param %s\n", argv[1]);
		if (verbose_flag >= 0) {
			print_out( print, "Supported QOS param ID and name:\n" );
			for (i = 0; i < ARRAY_SIZE(qcsapi_qos_param_table); i++)
				print_out( print, "%d: %s\n",
					qcsapi_qos_param_table[i].qos_param_type,
					qcsapi_qos_param_table[i].qos_param_name );
		}
		return 1;
	}

	if (isdigit(*argv[2])) {
		param_value = atoi(argv[2]);
	} else {
		print_err(print, "Unrecognized QoS param's value %s\n", argv[2]);
		return 1;
	}

	if (argc > 3)
		ap_bss_flag = atoi(argv[3]);

	qcsapi_retval = qcsapi_wifi_qos_set_param(the_interface,
						  the_queue,
						  the_param,
						  ap_bss_flag,
						  param_value);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out(print, "complete\n");
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wifi_get_wmm_ac_map(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval = 0;
	const char	*the_interface = p_calling_bundle->caller_interface;
	string_64 ac_map; /* Must be a string for the RPC generation Perl script */
	qcsapi_output	*print = p_calling_bundle->caller_output;

	assert(sizeof(ac_map) >= QCSAPI_WIFI_AC_MAP_SIZE);

	memset(ac_map, 0, sizeof(ac_map));
	qcsapi_retval = qcsapi_wifi_get_wmm_ac_map(the_interface, ac_map);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out(print, "%s\n", ac_map);
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wifi_set_wmm_ac_map(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int		statval = 0;
	int		qcsapi_retval = 0;
	const char	*the_interface = p_calling_bundle->caller_interface;
	int		user_prio = -1;
	int		ac_index = -1;
	qcsapi_output	*print = p_calling_bundle->caller_output;

	if (argc < 2) {
		print_err(print, "Usage: call_qcsapi set_wmm_ac_map <WiFi interface> "
				"<user priority> <AC index>\n");
		return 1;
	}

	if (isdigit(*argv[0])) {
		user_prio = atoi(argv[0]);
	} else {
		print_err(print, "Unrecognized user priority %s,"
					"Supported user priority range: 0~7\n", argv[0]);
		return 1;
	}

	if (isdigit(*argv[1])) {
		ac_index = atoi(argv[1]);
	} else {
		print_err(print, "Unrecognized AC index %s, "
			"Supported AC index range: 0(AC_BE), 1(AC_BK), 2(AC_VI), 3(AC_VO)\n", argv[1]);
		return 1;
	}

	qcsapi_retval = qcsapi_wifi_set_wmm_ac_map(the_interface,
						  user_prio,
						  ac_index);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out(print, "complete\n");
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wifi_get_dscp_8021p_map(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int	i;
	int	statval = 0;
	int	qcsapi_retval = 0;
	string_64 dot1p_mapping; /* Must be a string for the RPC generation Perl script */
	const char	*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output	*print = p_calling_bundle->caller_output;

	assert(sizeof(dot1p_mapping) >= IP_DSCP_NUM);

	memset(dot1p_mapping, 0, sizeof(dot1p_mapping));
	qcsapi_retval = qcsapi_wifi_get_dscp_8021p_map(the_interface, (char *)dot1p_mapping);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out(print, "IP DSCP/802.1p UP:\n");
			for (i = 0; i < IP_DSCP_NUM; i++) {
				print_out(print, "%2d/%d ", i, dot1p_mapping[i]);
				if ((i+1)%IEEE8021P_PRIORITY_NUM == 0)
					print_out(print, "\n");
			}
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wifi_set_dscp_8021p_map(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int	statval = 0;
	int	qcsapi_retval = 0;
	uint8_t	dot1p_up = 0;
	const char	*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output	*print = p_calling_bundle->caller_output;

	if (argc < 2) {
		print_err(print, "Usage: call_qcsapi set_dscp_8021p_map <WiFi interface> "
				"<IP DSCP list> <802.1p UP>\n");
		return 1;
	}


	if (isdigit(*argv[1])) {
		dot1p_up  = atoi(argv[1]);
	} else {
		print_err(print, "Unrecognized 802.1p UP %s, "
					"Supported 802.1p UP range: 0-7\n", argv[1]);
		return 1;
	}

	qcsapi_retval = qcsapi_wifi_set_dscp_8021p_map(the_interface,
						argv[0],
						dot1p_up);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out(print, "complete\n");
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wifi_get_dscp_ac_map(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
#define QCSAPI_BINARY_CONVERT_MASK	0x20
	int	i;
	int	statval = 0;
	int	qcsapi_retval = 0;
	struct qcsapi_data_64bytes ac_mapping;
	const char	*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output	*print = p_calling_bundle->caller_output;
	const char *acstr[] = {"AC_BE", "AC_BK", "AC_VI", "AC_VO"};

	assert(sizeof(ac_mapping) >= IP_DSCP_NUM);

	memset(&ac_mapping, 0, sizeof(ac_mapping));
	qcsapi_retval = qcsapi_wifi_get_dscp_ac_map(the_interface, &ac_mapping);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out(print, "DSCP            AC\n");
			for (i = 0; i < IP_DSCP_NUM; i++) {
				uint8_t mask = QCSAPI_BINARY_CONVERT_MASK;
				/* Print DSCP in binary format */
				while (mask) {
					 print_out(print, "%d", i & mask ? 1 : 0);
					 mask >>= 1;
				}
				print_out(print, "(0x%02x)    %s\n", i, acstr[(uint8_t)ac_mapping.data[i]]);
			}
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

/*
 * Convert given formatted dscp string into digital value
 * Two types of formatted dscp string are acceptable
 * eg,
 * TYPE I  -- 3,4,5,25,38
 * TYPE II -- 3-25
*/
static int call_qcsapi_convert_ipdscp_digital(const char *dscpstr, uint8_t *array, uint8_t *number)
{
	uint8_t ip_dscp_number = 0;
	char *pcur;
	char *p;
	char buffer[256] = {0};

	if (dscpstr == NULL || array == NULL || number == NULL)
		return -EINVAL;

	strncpy(buffer, dscpstr, (sizeof(buffer) - 1));
	pcur = buffer;
	do {
		p = strchr(pcur,'-');
		if (p) {
			uint8_t dscpstart;
			uint8_t dscpend;
			int i;

			*p = '\0';
			p++;
			if (!isdigit(*pcur) || !isdigit(*p))
				return -EINVAL;
			dscpstart = atoi(pcur);
			dscpend = atoi(p);

			if ((dscpstart > dscpend) || (dscpstart >= IP_DSCP_NUM)
				|| (dscpend >= IP_DSCP_NUM))
				return -EINVAL;
			ip_dscp_number = dscpend - dscpstart;
			for (i = 0; i <= ip_dscp_number; i++)
				array[i] =  dscpstart + i;
			break;
		} else {
			if (ip_dscp_number > (IP_DSCP_NUM - 1))
				return -EINVAL;

			p = strchr(pcur,',');
			if (p) {
				*p = '\0';
				p++;
				array[ip_dscp_number] = atoi(pcur);
				if (array[ip_dscp_number] >= IP_DSCP_NUM)
					return -EINVAL;
				pcur = p;
				ip_dscp_number++;
			} else {
				array[ip_dscp_number] = atoi(pcur);
				if (array[ip_dscp_number] >= IP_DSCP_NUM)
					return -EINVAL;
			}
		}
	} while (p);
	*number = ip_dscp_number + 1;

	return 0;
}

static int
call_qcsapi_wifi_set_dscp_ac_map(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int	statval = 0;
	int	qcsapi_retval = 0;
	uint8_t	listlen = 0;
	uint8_t	ac = 0;
	const char	*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output	*print = p_calling_bundle->caller_output;
	struct qcsapi_data_64bytes ip_dscp_value;

	if (argc != 2) {
		print_err(print,
			"Usage: call_qcsapi set_dscp_ac_map <WiFi interface> "
				"<IP DSCP list> <ac>\n");
		return 1;
	}

	if (!isdigit(*argv[1])) {
		print_err(print,
			"Unrecognized AC value %s; Supported AC range: 0-3\n",
			argv[1]);
		return 1;
	} else {
		ac = atoi(argv[1]);
	}

	memset(&ip_dscp_value, 0, sizeof(ip_dscp_value));
	statval = call_qcsapi_convert_ipdscp_digital(argv[0], ip_dscp_value.data, &listlen);
	if (statval < 0)
		return statval;

	qcsapi_retval = qcsapi_wifi_set_dscp_ac_map(the_interface,
						&ip_dscp_value, listlen, ac);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out(print, "complete\n");
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wifi_get_priority(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int	statval = 0;
	int	qcsapi_retval = 0;
	uint8_t	priority;
	const char	*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output	*print = p_calling_bundle->caller_output;

	qcsapi_retval = qcsapi_wifi_get_priority(the_interface, &priority);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out(print, "%u\n", priority);
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static void
call_qcsapi_wifi_set_priority_usage(qcsapi_output *print)
{
	print_err(print, "Usage: call_qcsapi set_priority <WiFi interface> <priority>\n");
	print_err(print, "Priority is an integer from 0 to %u.\n", QTN_VAP_PRIORITY_NUM - 1);
}

static int
call_qcsapi_wifi_set_priority(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int	statval = 0;
	int	qcsapi_retval = 0;
	uint8_t	priority = 0;
	const char	*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output	*print = p_calling_bundle->caller_output;

	if (argc != 1) {
		call_qcsapi_wifi_set_priority_usage(print);
		return 1;
	}

	if (isdigit(*argv[0])) {
		priority  = atoi(argv[0]);
		if (priority >= QTN_VAP_PRIORITY_NUM) {
			call_qcsapi_wifi_set_priority_usage(print);
			return 1;
		}
	} else {
		call_qcsapi_wifi_set_priority_usage(print);
		return 1;
	}

	qcsapi_retval = qcsapi_wifi_set_priority(the_interface,
						priority);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out(print, "complete\n");
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wifi_get_airfair(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int	statval = 0;
	int	qcsapi_retval = 0;
	uint8_t	airfair;
	const char	*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output	*print = p_calling_bundle->caller_output;

	qcsapi_retval = qcsapi_wifi_get_airfair(the_interface, &airfair);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out(print, "%u\n", airfair);
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static void
call_qcsapi_wifi_set_airfair_usage(qcsapi_output *print)
{
	print_err(print, "Usage: call_qcsapi set_airfair <WiFi interface> <status>\n");
	print_err(print, "Status is either 0(disabled) or 1(enabled).\n");
}

static int
call_qcsapi_wifi_set_airfair(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int	statval = 0;
	int	qcsapi_retval = 0;
	uint8_t	airfair = 0;
	const char	*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output	*print = p_calling_bundle->caller_output;

	if (argc != 1) {
		call_qcsapi_wifi_set_airfair_usage(print);
		return 1;
	}

	if (isdigit(*argv[0])) {
		airfair  = atoi(argv[0]);
		if (airfair > 1) {
			call_qcsapi_wifi_set_airfair_usage(print);
			return 1;
		}
	} else {
		call_qcsapi_wifi_set_airfair_usage(print);
		return 1;
	}

	qcsapi_retval = qcsapi_wifi_set_airfair(the_interface,
						airfair);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out(print, "complete\n");
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_config_get_parameter(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1) {
		print_err( print, "Usage: call_qcsapi get_persistent_param <WiFi interface> <parameter name>\n");
		statval = 1;
	} else {
		int		qcsapi_retval = 0;
		const char	*the_interface = p_calling_bundle->caller_interface;
		char		*parameter_name = argv[ 0 ];
		char		parameter_value_buffer[QCSAPI_MAX_PARAMETER_VALUE_LEN] = "";
		char		*parameter_value = &parameter_value_buffer[0];
		size_t		parameter_value_size = QCSAPI_MAX_PARAMETER_VALUE_LEN;

		if (strcmp(parameter_name, "NULL") == 0) {
			parameter_name = NULL;
		}

		if (argc > 1) {
			if (strcmp(argv[1], "NULL") == 0) {
				parameter_value = NULL;
			} else {
				parameter_value_size = (size_t) atoi(argv[1]);
			}
		}

		qcsapi_retval = qcsapi_config_get_parameter(the_interface,
							    parameter_name,
							    parameter_value,
							    parameter_value_size);

		if (qcsapi_retval >= 0) {
			if (verbose_flag >= 0) {
				print_out(print, "%s\n", &parameter_value_buffer[0]);
			}
		} else {
			report_qcsapi_error(p_calling_bundle, qcsapi_retval);
			statval = 1;
		}
	}

	return statval;
}

static int
call_qcsapi_config_update_parameter(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 2) {
		print_err( print,
	   "Not enough parameters in call qcsapi update persistent parameter, count is %d\n", argc);
		print_err( print, "Usage: call_qcsapi update_persistent_param <WiFi interface> <parameter name> <value>\n");
		statval = 1;
	} else {
		int		qcsapi_retval = 0;
		const char	*the_interface = p_calling_bundle->caller_interface;
		char		*parameter_name = argv[ 0 ];
		char		*parameter_value = argv[ 1 ];

		if (strcmp(parameter_name, "NULL") == 0) {
			parameter_name = NULL;
		}

		if (strcmp(parameter_value, "NULL") == 0) {
			parameter_value = NULL;
		}

		qcsapi_retval = qcsapi_config_update_parameter(the_interface, parameter_name, parameter_value);

		if (qcsapi_retval >= 0) {
			if (verbose_flag >= 0) {
				print_out( print, "complete\n");
			}
		} else {
			report_qcsapi_error(p_calling_bundle, qcsapi_retval);
			statval = 1;
		}
	}

	return statval;
}

static int
call_qcsapi_config_get_ssid_parameter(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1) {
		print_err( print, "Usage: call_qcsapi get_persistent_ssid_param <WiFi interface> <parameter name>\n");
		statval = 1;
	} else {
		int		qcsapi_retval = 0;
		const char	*the_interface = p_calling_bundle->caller_interface;
		char		*parameter_name = argv[ 0 ];
		char		parameter_value_buffer[QCSAPI_MAX_PARAMETER_VALUE_LEN] = "";
		char		*parameter_value = &parameter_value_buffer[0];
		size_t		parameter_value_size = QCSAPI_MAX_PARAMETER_VALUE_LEN;

		if (strcmp(parameter_name, "NULL") == 0) {
			parameter_name = NULL;
		}

		if (argc > 1) {
			if (strcmp(argv[1], "NULL") == 0) {
				parameter_value = NULL;
			} else {
				parameter_value_size = (size_t) atoi(argv[1]);
			}
		}

		qcsapi_retval = qcsapi_config_get_ssid_parameter(the_interface,
							    parameter_name,
							    parameter_value,
							    parameter_value_size);

		if (qcsapi_retval >= 0) {
			if (verbose_flag >= 0) {
				print_out(print, "%s\n", &parameter_value_buffer[0]);
			}
		} else {
			report_qcsapi_error(p_calling_bundle, qcsapi_retval);
			statval = 1;
		}
	}

	return statval;
}

static int
call_qcsapi_config_update_ssid_parameter(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 2) {
		print_err( print,
				"Not enough parameters in call_qcsapi update_persistent_ssid_parameter\n");
		print_err( print,
				"Usage: call_qcsapi update_persistent_ssid_param <WiFi interface> <parameter name> <value>\n");
		statval = 1;
	} else {
		int		qcsapi_retval = 0;
		const char	*the_interface = p_calling_bundle->caller_interface;
		char		*parameter_name = argv[ 0 ];
		char		*parameter_value = argv[ 1 ];

		if (strcmp(parameter_name, "NULL") == 0) {
			parameter_name = NULL;
		}

		if (strcmp(parameter_value, "NULL") == 0) {
			parameter_value = NULL;
		}

		qcsapi_retval = qcsapi_config_update_ssid_parameter(the_interface, parameter_name, parameter_value);

		if (qcsapi_retval >= 0) {
			if (verbose_flag >= 0) {
				print_out( print, "complete\n");
			}
		} else {
			report_qcsapi_error(p_calling_bundle, qcsapi_retval);
			statval = 1;
		}
	}

	return statval;
}

static int
call_qcsapi_bootcfg_get_parameter(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1) {
		print_err( print,
	   "Not enough parameters in call qcsapi get bootcfg parameter, count is %d\n", argc);
		print_err( print, "Usage: call_qcsapi get_bootcfg_param <parameter name>\n");
		statval = 1;
	} else {
		int		qcsapi_retval = 0;
		char		*parameter_name = argv[ 0 ];
		char		parameter_value[QCSAPI_MAX_PARAMETER_VALUE_LEN + 1] = {'\0'};
		char		*param_value_addr = &parameter_value[0];
		size_t		parameter_len = QCSAPI_MAX_PARAMETER_VALUE_LEN + 1;

		if (strcmp(parameter_name, "NULL") == 0) {
			parameter_name = NULL;
		}

		if (argc > 1 && strcmp(argv[1], "NULL") == 0) {
			param_value_addr = NULL;
		}

		if (argc > 2 && isdigit(argv[2][0])) {
			parameter_len = atoi(argv[2]);
		}

		qcsapi_retval = qcsapi_bootcfg_get_parameter(parameter_name,
							     param_value_addr,
							     parameter_len);

		if (qcsapi_retval >= 0) {
			if (verbose_flag >= 0) {
				print_out( print, "%s\n", param_value_addr);
			}
		} else {
			report_qcsapi_error(p_calling_bundle, qcsapi_retval);
			statval = 1;
		}
	}

	return statval;
}

static int
call_qcsapi_bootcfg_update_parameter(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 2) {
		print_err( print,
	   "Not enough parameters in call qcsapi update bootcfg parameter, count is %d\n", argc);
		print_err( print, "Usage: call_qcsapi update_bootcfg_param <parameter name> <value>\n");
		statval = 1;
	} else {
		int		qcsapi_retval = 0;
		char		*parameter_name = argv[0];
		char		*param_value_addr = argv[1];

		if (strcmp(parameter_name, "NULL") == 0) {
			parameter_name = NULL;
		}

		if (strcmp(param_value_addr, "NULL") == 0) {
			param_value_addr = NULL;
		}

		qcsapi_retval = qcsapi_bootcfg_update_parameter(parameter_name, param_value_addr);

		if (qcsapi_retval >= 0) {
			if (verbose_flag >= 0) {
				print_out( print, "complete\n");
			}
		} else {
			report_qcsapi_error(p_calling_bundle, qcsapi_retval);
			statval = 1;
		}
	}

	return statval;
}

static int
call_qcsapi_bootcfg_commit(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;
	int qcsapi_retval = 0;

	qcsapi_retval = qcsapi_bootcfg_commit();

	if (qcsapi_retval >= 0) {
		print_out(print, "complete\n");
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_service_control(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int     statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 2) {
		print_err( print,
				"Not enough parameters in call qcsapi service_control, count is %d\n", argc);
		print_err( print, "Usage: call_qcsapi service_control <service name> <action>\n");
		statval = 1;
	} else {
		int     qcsapi_retval = 0;
		char *name = argv[0];
		char *action = argv[1];
		qcsapi_service_name serv_name;
		qcsapi_service_action serv_action;

		if (strcmp(argv[0], "NULL") == 0) {
			name = NULL;
		} else if (strcmp(argv[0], "telnet") == 0) {
			name = "inetd";
		}
		if (strcmp(argv[1], "NULL") == 0) {
			action = NULL;
		}

		qcsapi_retval = qcsapi_get_service_name_enum(name, &serv_name);
		if (qcsapi_retval  >= 0) {
			qcsapi_retval  = qcsapi_get_service_action_enum(action, &serv_action);
		}

		if (qcsapi_retval  >= 0) {
			qcsapi_retval = qcsapi_service_control(serv_name, serv_action);
		}

		if (qcsapi_retval >= 0) {
			if (verbose_flag >= 0) {
				print_out( print, "complete\n");
			}
		} else {
			report_qcsapi_error(p_calling_bundle, qcsapi_retval);
			statval = 1;
		}
	}
	return statval;
}

static int
call_qcsapi_wfa_cert(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;
	uint16_t enable = 1;

	if (argc > 0) {
		if (safe_atou16(argv[0], &enable, print, 0, 1) == 0)
			return 1;
	}

	qcsapi_retval = qcsapi_wfa_cert_mode_enable(!!enable);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "complete\n");
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wifi_scs_enable(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval = 0;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;
	uint16_t enable = 1;

	if (argc > 0) {
		if (0 == safe_atou16(argv[0], &enable, print, 0, 1))
			return 1;
	}

	qcsapi_retval = qcsapi_wifi_scs_enable(the_interface, enable);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "complete\n");
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wifi_scs_switch_channel(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval = 0;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;

	qcsapi_retval = qcsapi_wifi_scs_switch_channel(the_interface);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "complete\n");
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wifi_set_scs_verbose(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval = 0;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;
	uint16_t enable = 1;

	if (argc > 0) {
		enable = (uint16_t) atoi(argv[0]);
	}

	qcsapi_retval = qcsapi_wifi_set_scs_verbose(the_interface, enable);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "complete\n");
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wifi_get_scs_status(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval = 0;
	qcsapi_unsigned_int status = 0;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;

	qcsapi_retval = qcsapi_wifi_get_scs_status(the_interface, &status);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			if (status == 1)
				print_out( print, "Enabled (%d)\n", status);
			else if (status == 0)
				print_out( print, "Disabled (%d)\n", status);
			else
				print_out( print, "Unknown (%d)\n", status);
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wifi_set_scs_smpl_enable(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval = 0;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;
	uint16_t enable = 1;

	if (argc > 0) {
		enable = (uint16_t) atoi(argv[0]);
	}

	qcsapi_retval = qcsapi_wifi_set_scs_smpl_enable(the_interface, enable);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out(print, "complete\n");
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wifi_set_scs_smpl_dwell_time(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval = 0;
	uint16_t sample_time = 0;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1) {
		print_out( print, "%s: programming error, expected at least 1 additional parameter\n", __func__);
		return(1);
	}

	if (safe_atou16(argv[0], &sample_time, print,
			IEEE80211_SCS_SMPL_DWELL_TIME_MIN, IEEE80211_SCS_SMPL_DWELL_TIME_MAX) == 0) {
		return 1;
	}

	qcsapi_retval = qcsapi_wifi_set_scs_smpl_dwell_time(the_interface, sample_time);
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "complete\n");
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wifi_set_scs_sample_intv(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval = 0;
	uint16_t sample_intv = 0;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (safe_atou16(argv[0], &sample_intv, print,
			IEEE80211_SCS_SMPL_INTV_MIN, IEEE80211_SCS_SMPL_INTV_MAX) == 0) {
		return 1;
	}

	qcsapi_retval = qcsapi_wifi_set_scs_sample_intv(the_interface, sample_intv);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "complete\n");
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wifi_set_scs_intf_detect_intv(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval = 0;
	uint16_t intv = 0;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (safe_atou16(argv[0], &intv, print,
			IEEE80211_SCS_CCA_DUR_MIN, IEEE80211_SCS_CCA_DUR_MAX) == 0) {
		return 1;
	}

	qcsapi_retval = qcsapi_wifi_set_scs_intf_detect_intv(the_interface, intv);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "complete\n");
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wifi_set_scs_thrshld(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 2) {
		print_err( print,
		       "Not enough parameters in call_qcsapi set_scs_thrshld, count is %d\n",
			argc);
		print_err( print,
			"Usage: call_qcsapi set_scs_thrshld <Wifi interface> <threshold parameter> <threshold value>\n");
		statval = 1;
	} else {
		int qcsapi_retval = 0;
		const char *the_interface = p_calling_bundle->caller_interface;
		char *thrshld_param_name = argv[0];
		uint16_t thrshld_value;

		if (safe_atou16(argv[1], &thrshld_value, print,
				0, 0xFFFF) == 0) {
			return 1;
		}

		qcsapi_retval = qcsapi_wifi_set_scs_thrshld(the_interface, thrshld_param_name, thrshld_value);

		if (qcsapi_retval >= 0) {
			if (verbose_flag >= 0) {
				print_out( print, "complete\n");
			}
		} else {
			report_qcsapi_error(p_calling_bundle, qcsapi_retval);
			statval = 1;
		}
	}

	return statval;
}

static int
call_qcsapi_wifi_set_scs_report_only(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{

	int statval = 0;
	int qcsapi_retval = 0;
	uint16_t report_value = 0;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;

	report_value = atoi(argv[0]);

	qcsapi_retval = qcsapi_wifi_set_scs_report_only(the_interface, report_value);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "complete\n");
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wifi_get_scs_report(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;
	int i;

	if (strcmp(argv[0], "current") == 0) {
		struct qcsapi_scs_currchan_rpt rpt;
		qcsapi_retval = qcsapi_wifi_get_scs_currchan_report(the_interface, &rpt);
		if (qcsapi_retval < 0) {
			report_qcsapi_error(p_calling_bundle, qcsapi_retval);
			statval = 1;
		} else {
			print_out(print, "SCS: current channel %d, cca_try=%u, cca_idle=%u cca_busy=%u cca_intf=%u"
					" cca_tx=%u tx_ms=%u rx_ms=%u pmbl_cnt=%u\n",
					rpt.chan,
					rpt.cca_try,
					rpt.cca_idle,
					rpt.cca_busy,
					rpt.cca_intf,
					rpt.cca_tx,
					rpt.tx_ms,
					rpt.rx_ms,
					rpt.pmbl);
		}
	} else if (strcmp(argv[0], "all") == 0) {
		struct qcsapi_scs_ranking_rpt rpt;
		qcsapi_retval = qcsapi_wifi_get_scs_stat_report(the_interface, &rpt);
		if (qcsapi_retval < 0) {
			report_qcsapi_error(p_calling_bundle, qcsapi_retval);
			statval = 1;
		} else {
			print_out(print, "SCS ranking report: chan number = %u\n", rpt.num);
			print_out(print, "chan dfs txpower cca_intf     metric    pmbl_ap   pmbl_sta"
					 "   age duration times\n");
			for (i = 0; i < rpt.num; i++) {
				print_out(print, "%4d %3d %7d %8u %10d %10d %10d %5u %8u %5u\n",
					rpt.chan[i],
					rpt.dfs[i],
					rpt.txpwr[i],
					rpt.cca_intf[i],
					rpt.metric[i],
					rpt.pmbl_ap[i],
					rpt.pmbl_sta[i],
					rpt.metric_age[i],
					rpt.duration[i],
					rpt.times[i]);
			}
		}
	} else if (strcmp(argv[0], "autochan") == 0) {
		struct qcsapi_autochan_rpt rpt;
		qcsapi_retval = qcsapi_wifi_get_autochan_report(the_interface, &rpt);
		if (qcsapi_retval < 0) {
			report_qcsapi_error(p_calling_bundle, qcsapi_retval);
			statval = 1;
		} else {
			print_out(print, "AP: initial auto channel ranking table: chan number = %u\n", rpt.num);
			print_out(print, "chan dfs txpower  numbeacon        cci        aci     metric\n");
			for (i = 0; i < rpt.num; i++) {
				print_out(print, "%4d %3d %7d %10u %10d %10d %10d\n",
					rpt.chan[i],
					rpt.dfs[i],
					rpt.txpwr[i],
					rpt.numbeacons[i],
					rpt.cci[i],
					rpt.aci[i],
					rpt.metric[i]);
			}
		}
	} else if (strcmp(argv[0], "score") == 0) {
		struct qcsapi_scs_score_rpt rpt;
		qcsapi_retval = qcsapi_wifi_get_scs_score_report(the_interface, &rpt);
		if (qcsapi_retval < 0) {
			report_qcsapi_error(p_calling_bundle, qcsapi_retval);
			statval = 1;
		} else {
			print_out(print, "SCS score report: channel number = %u\n", rpt.num);
			print_out(print, "channel  score\n");
			for (i = 0; i < rpt.num; i++) {
				print_out(print, "%4d  %5d\n", rpt.chan[i], rpt.score[i]);
			}
		}
	} else {
		print_err(print, "Invalid parameter:%s\nOptional choice:current all autochan score\n", argv[0]);
		return 1;
	}



	return statval;
}

static int
call_qcsapi_wifi_set_scs_cca_intf_smth_fctr(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{

	int statval = 0;
	int qcsapi_retval = 0;
	uint8_t fctr_noxp = 0;
	uint8_t fctr_xped = 0;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 2) {
		print_err(print,
			"Not enough parameters in call_qcsapi set_scs_cca_intf_smth_fctr, count is %d\n",
			argc);
		print_err(print,
			"Usage: call_qcsapi set_scs_cca_intf_smth_fctr <Wifi interface> "
			"<factor for never used channel> <factor for used channel>\n" );
		statval = 1;
	} else {
		fctr_noxp = atoi(argv[0]);
		fctr_xped = atoi(argv[1]);

		qcsapi_retval = qcsapi_wifi_set_scs_cca_intf_smth_fctr(the_interface, fctr_noxp, fctr_xped);

		if (qcsapi_retval >= 0) {
			if (verbose_flag >= 0) {
				print_out( print, "complete\n");
			}
		} else {
			report_qcsapi_error(p_calling_bundle, qcsapi_retval);
			statval = 1;
		}
	}

	return statval;
}

static int
call_qcsapi_wifi_set_scs_stats(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval = 0;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;
	uint16_t start = 1;

	if (argc > 0) {
		start = (uint16_t) atoi(argv[0]);
	}

	qcsapi_retval = qcsapi_wifi_set_scs_stats(the_interface, start);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out(print, "complete\n");
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wifi_set_vendor_fix(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int		statval = 0;
	int		qcsapi_retval = 0;
	const char	*the_interface = p_calling_bundle->caller_interface;
	int		fix_param = -1;
	int		param_value = -1;
	int		i;
	qcsapi_output	*print = p_calling_bundle->caller_output;

	if (argc < 2) {
		print_err(print, "Usage: call_qcsapi set_vendor_fix <WiFi interface> "
				"<fix-param> <value>\n");
		return 1;
	}

	if (name_to_vendor_fix_idx(argv[0], &fix_param) == 0) {
		print_err(print, "Unrecognized vendor fix param %s\n", argv[0]);
		if (verbose_flag >= 0) {
			print_out( print, "Supported vendor fix param:\n" );
			for (i = 0; i < ARRAY_SIZE(qcsapi_vendor_fix_table); i++)
				print_out( print, "%s\n",
						qcsapi_vendor_fix_table[i].fix_name);
		}
		return 1;
	}

	if (isdigit(*argv[1])) {
		param_value = atoi(argv[1]);
	} else {
		print_err(print, "Unrecognized vendor fix's value %s\n", argv[1]);
		return 1;
	}

	qcsapi_retval = qcsapi_wifi_set_vendor_fix(the_interface,
						  fix_param,
						  param_value);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out(print, "complete\n");
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wifi_set_scs_chan_mtrc_mrgn(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{

	int statval = 0;
	int qcsapi_retval = 0;
	uint8_t value = 0;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1) {
		print_err(print,
			"Not enough parameters in call_qcsapi set_scs_chan_mtrc_mrgn, count is %d\n",
			argc);
		print_err(print,
			"Usage: call_qcsapi set_scs_chan_mtrc_mrgn <Wifi interface> "
			"<channel metric margin>\n" );
		statval = 1;
	} else {
		value = atoi(argv[0]);

		qcsapi_retval = qcsapi_wifi_set_scs_chan_mtrc_mrgn(the_interface, value);

		if (qcsapi_retval >= 0) {
			if (verbose_flag >= 0) {
				print_out( print, "complete\n");
			}
		} else {
			report_qcsapi_error(p_calling_bundle, qcsapi_retval);
			statval = 1;
		}
	}

	return statval;
}

static int
call_qcsapi_wifi_get_scs_dfs_reentry_request(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval = 0;
	qcsapi_unsigned_int status = 0;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;

	qcsapi_retval = qcsapi_wifi_get_scs_dfs_reentry_request(the_interface, &status);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out(print, "%d\n", status);
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wifi_get_scs_cca_intf( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1) {
		print_err( print, "Not enough parameters in call qcsapi to get scs cca interference\n" );
		print_err( print, "Usage: call_qcsapi get_scs_cca_intf <interface> <channel>\n" );
		statval = 1;
	}
	else {
		int			 qcsapi_retval;
		const char		*the_interface = p_calling_bundle->caller_interface;
		qcsapi_unsigned_int	 the_channel = atoi( argv[ 0 ] );
		int			 cca_intf = 0;

		qcsapi_retval = qcsapi_wifi_get_scs_cca_intf( the_interface, the_channel, &cca_intf );
		if (qcsapi_retval >= 0) {
			if (verbose_flag >= 0) {
				print_out( print, "%d\n", cca_intf );
			}
		} else {
			report_qcsapi_error(p_calling_bundle, qcsapi_retval);
			statval = 1;
		}
	}

	return( statval );
}

static int
call_qcsapi_wifi_get_scs_param(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval = 0;
	int len = 0;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;
	qcsapi_scs_param_rpt *p_rpt;

	len = sizeof(*p_rpt)*SCS_PARAM_MAX;
	p_rpt = (qcsapi_scs_param_rpt *)malloc(len);
	if (p_rpt == NULL) {
		print_err( print, "malloc failed - %s\n", __func__);
		return 1;
	}

	memset(p_rpt, 0, len);
	qcsapi_retval = qcsapi_wifi_get_scs_param_report(the_interface, p_rpt, SCS_PARAM_MAX);
	if (qcsapi_retval < 0) {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	} else {
		dump_scs_param(print, p_rpt);
	}

	free(p_rpt);
	p_rpt = NULL;
	return statval;
}

static int
call_qcsapi_wifi_start_ocac(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;
	uint16_t channel_value = 0;

	if (argc < 1) {
		print_out(print, "Usage:\n"
				"  call_qcsapi start_ocac wifi0 { auto | <DFS channel> }\n");
		return 1;
	}

	/* parameter parse */
	if (!strcasecmp("auto", argv[0])) {
		channel_value = 0;
	} else {
		if (safe_atou16(argv[0], &channel_value, print, 0, 0xFFFF) == 0) {
			return 1;
		}
	}

	qcsapi_retval = qcsapi_wifi_start_ocac(the_interface, channel_value);
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "complete\n");
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_stop_ocac(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;

	qcsapi_retval = qcsapi_wifi_stop_ocac(the_interface);
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "complete\n");
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_get_ocac_status(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval = 0;
	qcsapi_unsigned_int status = 0;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;

	qcsapi_retval = qcsapi_wifi_get_ocac_status(the_interface, &status);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			if (status == 1)
				print_out( print, "Enabled\n");
			else if (status == 0)
				print_out( print, "Disabled\n");
			else
				print_out( print, "Unknown (%u)\n", status);
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wifi_set_ocac_dwell_time(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;
	uint16_t dwell_time = 0;

	if (argc < 1) {
		print_out(print, "Usage:\n"
				"  call_qcsapi set_ocac_dwell_time wifi0 <msecs>\n");
		return 1;
	}

	if (safe_atou16(argv[0], &dwell_time, print, 0, 0xFFFF) == 0) {
		return 1;
	}

	qcsapi_retval = qcsapi_wifi_set_ocac_dwell_time(the_interface, dwell_time);
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "complete\n");
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_set_ocac_duration(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;
	uint16_t duration = 0;

	if (argc < 1) {
		print_out(print, "Usage:\n"
				"  call_qcsapi set_ocac_duration wifi0 <seconds>\n");
		return 1;
	}

	if (safe_atou16(argv[0], &duration, print, 0, 0xFFFF) == 0) {
		return 1;
	}

	qcsapi_retval = qcsapi_wifi_set_ocac_duration(the_interface, duration);
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "complete\n");
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_set_ocac_cac_time(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;
	uint16_t cac_time = 0;

	if (argc < 1) {
		print_out(print, "Usage:\n"
				"  call_qcsapi set_ocac_cac_time wifi0 <seconds>\n");
		return 1;
	}

	if (safe_atou16(argv[0], &cac_time, print, 0, 0xFFFF) == 0) {
		return 1;
	}

	qcsapi_retval = qcsapi_wifi_set_ocac_cac_time(the_interface, cac_time);
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "complete\n");
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_set_ocac_report_only(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;
	uint16_t value = 0;

	if (argc < 1) {
		print_out(print, "Usage:\n"
				"  call_qcsapi set_ocac_report_only wifi0 { 1 | 0 }\n");
		return 1;
	}

	if (safe_atou16(argv[0], &value, print, 0, 0xFFFF) == 0) {
		return 1;
	}

	qcsapi_retval = qcsapi_wifi_set_ocac_report_only(the_interface, value);
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "complete\n");
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_set_ocac_threshold(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 2) {
		print_err( print,
			"Usage: call_qcsapi set_ocac_thrshld <WiFi interface> <threshold parameter> <threshold value>\n");
		statval = 1;
	} else {
		int qcsapi_retval = 0;
		const char *the_interface = p_calling_bundle->caller_interface;
		char *thrshld_param_name = argv[0];
		uint16_t thrshld_value;

		if (safe_atou16(argv[1], &thrshld_value, print,	0, 0xFFFF) == 0) {
			return 1;
		}

		qcsapi_retval = qcsapi_wifi_set_ocac_thrshld(the_interface, thrshld_param_name, thrshld_value);

		if (qcsapi_retval >= 0) {
			if (verbose_flag >= 0) {
				print_out( print, "complete\n");
			}
		} else {
			report_qcsapi_error(p_calling_bundle, qcsapi_retval);
			statval = 1;
		}
	}

	return statval;
}

static int
call_qcsapi_wifi_start_dfs_s_radio(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;
	uint16_t channel_value = 0;

	if (argc < 1) {
		print_out(print, "Usage:\n"
				"  call_qcsapi start_dfs_s_radio wifi0 { auto | <DFS channel> }\n");
		return 1;
	}

	/* parameter parse */
	if (!strcasecmp("auto", argv[0])) {
		channel_value = 0;
	} else {
		if (safe_atou16(argv[0], &channel_value, print, 0, 0xFFFF) == 0) {
			return 1;
		}
	}

	qcsapi_retval = qcsapi_wifi_start_dfs_s_radio(the_interface, channel_value);
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "complete\n");
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_stop_dfs_s_radio(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;

	qcsapi_retval = qcsapi_wifi_stop_dfs_s_radio(the_interface);
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "complete\n");
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_get_dfs_s_radio_status(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval = 0;
	qcsapi_unsigned_int status = 0;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;

	qcsapi_retval = qcsapi_wifi_get_dfs_s_radio_status(the_interface, &status);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			if (status == 1)
				print_out( print, "Enabled\n");
			else if (status == 0)
				print_out( print, "Disabled\n");
			else
				print_out( print, "Unknown (%u)\n", status);
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wifi_get_dfs_s_radio_availability(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int		statval = 0;
	int		qcsapi_retval;
	const char	*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output	*print = p_calling_bundle->caller_output;
	qcsapi_unsigned_int	available = 0;

	qcsapi_retval = qcsapi_wifi_get_dfs_s_radio_availability(the_interface, &available);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			if (available == 1)
				print_out( print, "Available\n");
			else
				print_out( print, "Unavailable\n");
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_set_dfs_s_radio_dwell_time(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;
	uint16_t dwell_time = 0;

	if (argc < 1) {
		print_out(print, "Usage:\n"
				"  call_qcsapi set_dfs_s_radio_dwell_time wifi0 <msecs>\n");
		return 1;
	}

	if (safe_atou16(argv[0], &dwell_time, print, 0, 0xFFFF) == 0) {
		return 1;
	}

	qcsapi_retval = qcsapi_wifi_set_dfs_s_radio_dwell_time(the_interface, dwell_time);
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "complete\n");
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_set_dfs_s_radio_duration(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;
	uint16_t duration = 0;

	if (argc < 1) {
		print_out(print, "Usage:\n"
				"  call_qcsapi set_dfs_s_radio_duration wifi0 <seconds>\n");
		return 1;
	}

	if (safe_atou16(argv[0], &duration, print, 0, 0xFFFF) == 0) {
		return 1;
	}

	qcsapi_retval = qcsapi_wifi_set_dfs_s_radio_duration(the_interface, duration);
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "complete\n");
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_set_dfs_s_radio_cac_time(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;
	uint16_t cac_time = 0;

	if (argc < 1) {
		print_out(print, "Usage:\n"
				"  call_qcsapi set_dfs_s_radio_cac_time wifi0 <seconds>\n");
		return 1;
	}

	if (safe_atou16(argv[0], &cac_time, print, 0, 0xFFFF) == 0) {
		return 1;
	}

	qcsapi_retval = qcsapi_wifi_set_dfs_s_radio_cac_time(the_interface, cac_time);
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "complete\n");
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_set_dfs_s_radio_report_only(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;
	uint16_t value = 0;

	if (argc < 1) {
		print_out(print, "Usage:\n"
				"  call_qcsapi set_dfs_s_radio_report_only wifi0 { 1 | 0 }\n");
		return 1;
	}

	if (safe_atou16(argv[0], &value, print, 0, 0xFFFF) == 0) {
		return 1;
	}

	qcsapi_retval = qcsapi_wifi_set_dfs_s_radio_report_only(the_interface, value);
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "complete\n");
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_set_dfs_s_radio_wea_duration(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;
	uint32_t duration = 0;

	if (argc < 1) {
		print_out(print, "Usage:\n"
				"  call_qcsapi set_dfs_s_radio_wea_duration wifi0 <seconds>\n");
		return 1;
	}

	if (safe_atou32(argv[0], &duration, print, 0, 0xFFFFFFFF) == 0) {
		return 1;
	}

	qcsapi_retval = qcsapi_wifi_set_dfs_s_radio_wea_duration(the_interface, duration);
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "complete\n");
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_set_dfs_s_radio_wea_cac_time(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;
	uint32_t cac_time = 0;

	if (argc < 1) {
		print_out(print, "Usage:\n"
				"  call_qcsapi set_dfs_s_radio_wea_cac_time wifi0 <seconds>\n");
		return 1;
	}

	if (safe_atou32(argv[0], &cac_time, print, 0, 0xFFFFFFFF) == 0) {
		return 1;
	}

	qcsapi_retval = qcsapi_wifi_set_dfs_s_radio_wea_cac_time(the_interface, cac_time);
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "complete\n");
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_set_dfs_s_radio_threshold(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 2) {
		print_err( print,
			"Usage: call_qcsapi set_dfs_s_radio_thrshld <WiFi interface> <threshold parameter> <threshold value>\n");
		statval = 1;
	} else {
		int qcsapi_retval = 0;
		const char *the_interface = p_calling_bundle->caller_interface;
		char *thrshld_param_name = argv[0];
		uint16_t thrshld_value;

		if (safe_atou16(argv[1], &thrshld_value, print,	0, 0xFFFF) == 0) {
			return 1;
		}

		qcsapi_retval = qcsapi_wifi_set_dfs_s_radio_thrshld(the_interface, thrshld_param_name, thrshld_value);

		if (qcsapi_retval >= 0) {
			if (verbose_flag >= 0) {
				print_out( print, "complete\n");
			}
		} else {
			report_qcsapi_error(p_calling_bundle, qcsapi_retval);
			statval = 1;
		}
	}

	return statval;
}

static int
call_qcsapi_wifi_set_ap_isolate(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval;
	int current_ap_isolate_status;
	qcsapi_output *print = p_calling_bundle->caller_output;
	const char *the_interface = p_calling_bundle->caller_interface;
	char *endptr;

	if (argc < 1) {
		print_err( print,
				"Parameter count incorrect. Should be 3, is %d\n", argc
		);
		statval = 1;
	} else {
		endptr = NULL;
		current_ap_isolate_status = strtol(argv[ 0 ], &endptr, 10);
		if (!endptr || (*endptr != 0) || ((endptr - 1) != argv[0]) ) {
			print_err( print, "Invalid isolation settting. Should be 0 or 1\n");
			statval = 1;
		} else {
			qcsapi_retval = qcsapi_wifi_set_ap_isolate(the_interface, current_ap_isolate_status);

			if (qcsapi_retval >= 0) {
				if (verbose_flag >= 0) {
					print_out(print, "complete\n");
				}
			} else {
				report_qcsapi_error(p_calling_bundle, qcsapi_retval);
				statval = 1;
			}
		}
	}

	return( statval );
}

static int
call_qcsapi_wifi_get_ap_isolate(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval;
	int current_ap_isolate_status = (int)qcsapi_ap_isolate_disabled;
	int *p_current_ap_isolate_status = NULL;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1 || strcmp( argv[ 0 ], "NULL" ) != 0) {
		p_current_ap_isolate_status = &current_ap_isolate_status;
	}

	qcsapi_retval = qcsapi_wifi_get_ap_isolate(the_interface, p_current_ap_isolate_status);
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out(print, "%d\n", current_ap_isolate_status);
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_get_interface_stats(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int	statval = 0;
	int			 qcsapi_retval;
	const char		*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output		*print = p_calling_bundle->caller_output;
	qcsapi_interface_stats	stats;

	qcsapi_retval = qcsapi_get_interface_stats( the_interface, &stats );
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			if (sizeof(long) == 8) {
				print_out(print,	"tx_bytes:\t%llu\n"
						"tx_pkts:\t%u\n"
						"tx_discard:\t%u\n"
						"tx_err:\t\t%u\n"
						"tx_unicast:\t%u\n"
						"tx_multicast:\t%u\n"
						"tx_broadcast:\t%u\n"
						"rx_bytes:\t%llu\n"
						"rx_pkts:\t%u\n"
						"rx_discard:\t%u\n"
						"rx_err:\t\t%u\n"
						"rx_unicast:\t%u\n"
						"rx_multicast:\t%u\n"
						"rx_broadcast:\t%u\n"
						"rx_unknown:\t%u\n",
						stats.tx_bytes,
						stats.tx_pkts,
						stats.tx_discard,
						stats.tx_err,
						stats.tx_unicast,
						stats.tx_multicast,
						stats.tx_broadcast,
						stats.rx_bytes,
						stats.rx_pkts,
						stats.rx_discard,
						stats.rx_err,
						stats.rx_unicast,
						stats.rx_multicast,
						stats.rx_broadcast,
						stats.rx_unknown);
			} else {
				print_out(print,	"tx_bytes:\t%llu\n"
						"tx_pkts:\t%lu\n"
						"tx_discard:\t%lu\n"
						"tx_err:\t\t%lu\n"
						"tx_unicast:\t%lu\n"
						"tx_multicast:\t%lu\n"
						"tx_broadcast:\t%lu\n"
						"rx_bytes:\t%llu\n"
						"rx_pkts:\t%lu\n"
						"rx_discard:\t%lu\n"
						"rx_err:\t\t%lu\n"
						"rx_unicast:\t%lu\n"
						"rx_multicast:\t%lu\n"
						"rx_broadcast:\t%lu\n"
						"rx_unknown:\t%lu\n",
						stats.tx_bytes,
						stats.tx_pkts,
						stats.tx_discard,
						stats.tx_err,
						stats.tx_unicast,
						stats.tx_multicast,
						stats.tx_broadcast,
						stats.rx_bytes,
						stats.rx_pkts,
						stats.rx_discard,
						stats.rx_err,
						stats.rx_unicast,
						stats.rx_multicast,
						stats.rx_broadcast,
						stats.rx_unknown);

			}
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_get_phy_stats(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int	statval = 0;
	int	iter;
	int			 qcsapi_retval;
	const char		*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output		*print = p_calling_bundle->caller_output;
	qcsapi_phy_stats	stats;

	qcsapi_retval = qcsapi_get_phy_stats( the_interface, &stats );
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out(print, "tstamp=\t\t%u\n"
					 "assoc=\t\t%u\n"
					 "channel=\t%u\n"
					 "attenuation=\t%u\n"
					 "cca_total=\t%u\n"
					 "cca_tx=\t\t%u\n"
					 "cca_rx=\t\t%u\n"
					 "cca_int=\t%u\n"
					 "cca_idle\t%u\n"
					 "rx_pkts=\t%u\n"
					 "rx_gain=\t%u\n"
					 "rx_cnt_crc=\t%u\n"
					 "rx_noise=\t%5.1f\n"
					 "tx_pkts=\t%u\n"
					 "tx_defers=\t%d\n"
					 "tx_touts=\t%u\n"
					 "tx_retries=\t%u\n"
					 "cnt_sp_fail=\t%u\n"
					 "cnt_lp_fail=\t%u\n"
					 "last_rx_mcs=\t%d\n"
					 "last_tx_mcs=\t%d\n",
				  stats.tstamp,
				  stats.assoc,
				  stats.channel,
				  stats.atten,
				  stats.cca_total,
				  stats.cca_tx,
				  stats.cca_rx,
				  stats.cca_int,
				  stats.cca_idle,
				  stats.rx_pkts,
				  stats.rx_gain,
				  stats.rx_cnt_crc,
				  stats.rx_noise,
				  stats.tx_pkts,
				  stats.tx_defers,
				  stats.tx_touts,
				  stats.tx_retries,
				  stats.cnt_sp_fail,
				  stats.cnt_lp_fail,
				  stats.last_rx_mcs,
				  stats.last_tx_mcs);
			print_out(print, "last_evm=\t%5.1f\n", stats.last_evm);
			for (iter = 0; iter < QCSAPI_QDRV_NUM_RF_STREAMS; iter++) {
				print_out(print, "last_evm_%d=\t%5.1f\n", iter, stats.last_evm_array[iter]);
			}

			print_out(print, "last_rcpi=\t%5.1f\n", stats.last_rcpi);

			print_out(print, "last_rssi=\t%5.1f\n", stats.last_rssi);
			for (iter = 0; iter < QCSAPI_QDRV_NUM_RF_STREAMS; iter++) {
				print_out(print, "last_rssi_%d=\t%5.1f\n", iter, stats.last_rssi_array[iter]);
			}
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_telnet_enable(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;
	qcsapi_unsigned_int on_off;

	if (argc < 1) {
		print_err( print, "Usage: call_qcsapi enable_telnet <value>\n");
		print_err( print, "Usage: value: 0 - disable; 1 - enable\n");
		statval = 1;
	} else {
		int		qcsapi_retval = 0;
		char		*parameter_value = argv[0];

		if (strcmp(parameter_value, "1") && strcmp(parameter_value, "0")) {
			print_err( print, "Usage: call_qcsapi enable_telnet <value>\n");
			print_err( print, "Usage: value: 0 - disable; 1 - enable\n");
			return 1;
		}

		on_off = (qcsapi_unsigned_int)atoi(parameter_value);
		qcsapi_retval = qcsapi_telnet_enable(on_off);

		if (qcsapi_retval == 0) {
			if (verbose_flag >= 0) {
				print_out( print, "complete\n");
			}
		} else {
			report_qcsapi_error(p_calling_bundle, qcsapi_retval);
			statval = 1;
		}
	}

	return statval;
}

static int
call_qcsapi_wps_set_access_control(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int		statval = 0;
	const char	*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output	*print = p_calling_bundle->caller_output;
	int		qcsapi_retval = 0;
	uint32_t	pp_enable;
	char		wps_state[32];

	if (argc < 1) {
		print_err( print, "Usage: call_qcsapi set_wps_access_control <value>\n" );
		print_err( print, "Usage: value: 0 - disable; 1 - enable\n" );
		statval = 1;
	} else {
		char	*parameter_value = argv[0];

		if (!strcmp(parameter_value, "1")) {
			pp_enable = 1;
		} else if (!strcmp(parameter_value, "0")) {
			pp_enable = 0;
		} else {
			print_err( print, "Usage: call_qcsapi set_wps_access_control <value>\n" );
			print_err( print, "Usage: value: 0 - disable; 1 - enable\n" );
			return 1;
		}

		qcsapi_retval = qcsapi_wps_get_configured_state(the_interface, wps_state, sizeof(wps_state));
		if (qcsapi_retval >= 0) {
			if (strncmp(wps_state, "configured", sizeof(wps_state)) != 0) {
				print_err(print, "enable WPS feature before setup WPS Access control\n");
				return 1;
			}
		}

		if (qcsapi_retval >= 0)
			qcsapi_retval = qcsapi_wps_set_access_control( the_interface, pp_enable );
	}

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "complete\n" );
		}
	} else {
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wps_get_access_control(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int		statval = 0;
	const char	*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output	*print = p_calling_bundle->caller_output;
	int		qcsapi_retval = 0;
	uint32_t	pp_enable;

	if (argc > 0) {
		print_err( print, "Usage: call_qcsapi get_wps_access\n" );
		print_err( print, "Usage: This command is used to get pair protection state \n" );
		statval = 1;
	} else {
		qcsapi_retval = qcsapi_wps_get_access_control( the_interface, &pp_enable );

		if (qcsapi_retval >= 0) {
			if (verbose_flag >= 0) {
				print_out( print, "%s\n", (pp_enable ? "1":"0") );
			}
		} else {
			report_qcsapi_error(p_calling_bundle, qcsapi_retval);
			statval = 1;
		}
	}

	return statval;
}

static int
call_qcsapi_non_wps_set_pp_enable(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int		statval = 0;
	const char	*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output	*print = p_calling_bundle->caller_output;
	int		qcsapi_retval = 0;
	uint32_t	pp_enable;

	if (argc < 1) {
		print_err( print, "Usage: call_qcsapi set_non_wps_pp_enable <value>\n" );
		print_err( print, "Usage: value: 0 - disable; 1 - enable\n" );
		statval = 1;
	} else {
		char	*parameter_value = argv[0];

		if (!strcmp(parameter_value, "1")) {
			pp_enable = 1;
		} else if (!strcmp(parameter_value, "0")) {
			pp_enable = 0;
		} else {
			print_err( print, "Usage: call_qcsapi set_non_wps_pp_enable <value>\n" );
			print_err( print, "Usage: value: 0 - disable; 1 - enable\n" );
			return 1;
		}

		qcsapi_retval = qcsapi_non_wps_set_pp_enable( the_interface, pp_enable );
	}

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "complete\n" );
		}
	} else {
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_non_wps_get_pp_enable(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int		statval = 0;
	const char	*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output	*print = p_calling_bundle->caller_output;
	int		qcsapi_retval = 0;
	uint32_t	pp_enable;

	if (argc > 0) {
		print_err( print, "Usage: call_qcsapi get_non_wps_pp_enable\n" );
		print_err( print, "Usage: This command is used to get non_wps pair protection state \n" );
		statval = 1;
	} else {
		qcsapi_retval = qcsapi_non_wps_get_pp_enable( the_interface, &pp_enable );

		if (qcsapi_retval >= 0) {
			if (verbose_flag >= 0) {
				print_out( print, "%s\n", (pp_enable ? "1":"0") );
			}
		} else {
			report_qcsapi_error(p_calling_bundle, qcsapi_retval);
			statval = 1;
		}
	}

	return statval;
}

static int
call_qcsapi_wps_cancel(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int		statval = 0;
	const char	*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output	*print = p_calling_bundle->caller_output;
	int		qcsapi_retval = 0;

	if (argc > 0) {
		print_err( print, "Usage: call_qcsapi wps_cancel <WiFi interface>\n" );
		statval = 1;
	} else {
		qcsapi_retval = qcsapi_wps_cancel(the_interface);

		if (qcsapi_retval >= 0) {
			if (verbose_flag >= 0) {
				print_out( print, "complete\n");
			}
		} else {
			report_qcsapi_error(p_calling_bundle, qcsapi_retval);
			statval = 1;
		}
	}

	return statval;
}

static int
call_qcsapi_wps_set_pbc_in_srcm(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output	*print = p_calling_bundle->caller_output;
	int qcsapi_retval = 0;
	uint16_t enabled = 0;

	if (argv[0] != NULL && safe_atou16(argv[0], &enabled, print, 0, 1))
		qcsapi_retval = qcsapi_wps_set_pbc_in_srcm(the_interface, enabled);
	else
		return 1;

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0)
			print_out(print, "complete\n");
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wps_get_pbc_in_srcm(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output	*print = p_calling_bundle->caller_output;
	int qcsapi_retval = 0;
	qcsapi_unsigned_int enabled = 0;

	qcsapi_retval = qcsapi_wps_get_pbc_in_srcm(the_interface, &enabled);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0)
			print_out(print, "%d\n", enabled);
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int call_qcsapi_wps_set_timeout(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int		statval = 0;
	const char	*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output	*print = p_calling_bundle->caller_output;
	int		qcsapi_retval = 0;
	int		timeout_val = 0;

	if (argc < 0) {
		print_out(print, "Usage: call_qcsapi wps_timeout <WiFi Interface> <timeout value>\n");
		statval = 1;
	} else {
		timeout_val = atoi(argv[0]);
		if (timeout_val < 120 || timeout_val > 600) {
			print_out(print, "Error: timeout should be limited from 120s to 600s\n");
			statval = 1;
		} else {
			qcsapi_retval = qcsapi_wps_set_timeout(the_interface, timeout_val);

			if (qcsapi_retval >= 0) {
				if (verbose_flag >= 0) {
					print_out( print, "complete\n");
				}
			} else {
				report_qcsapi_error(p_calling_bundle, qcsapi_retval);
				statval = 1;
			}
		}
	}

	return statval;
}

static int call_qcsapi_wps_on_hidden_ssid(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int		statval = 0;
	const char	*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output	*print = p_calling_bundle->caller_output;
	int		qcsapi_retval = 0;
	int		option = -1;

	if (argc < 0) {
		print_out(print, "Usage: call_qcsapi wps_on_hidden_ssid <WiFi Interface> <0 | 1>\n");
		statval = 1;
	} else {
		option = atoi(argv[0]);
		if ((strlen(argv[0]) != 1) || (!isdigit(*argv[0])) || ((option != 0) && (option != 1))) {
			print_out(print, "Usage: call_qcsapi wps_on_hidden_ssid <WiFi Interface> <0 | 1>\n");
			statval = 1;
		} else {
			qcsapi_retval = qcsapi_wps_on_hidden_ssid(the_interface, option);

			if (qcsapi_retval >= 0) {
				if (verbose_flag >= 0) {
					print_out( print, "complete\n");
				}
			} else {
				report_qcsapi_error(p_calling_bundle, qcsapi_retval);
				statval = 1;
			}
		}
	}

	return statval;
}

static int call_qcsapi_wps_on_hidden_ssid_status(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int		statval = 0;
	const char	*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output	*print = p_calling_bundle->caller_output;
	int		qcsapi_retval = 0;
	char		state[64];

	if (argc > 0) {
		print_out(print, "Usage: call_qcsapi wps_on_hidden_ssid_status <WiFi Interface>\n");
		statval = 1;
	} else {
		qcsapi_retval = qcsapi_wps_on_hidden_ssid_status(the_interface, state, sizeof(state));

		if (qcsapi_retval >= 0) {
			if (verbose_flag >= 0) {
				print_out( print, "%s\n", state);
			}
		} else {
			report_qcsapi_error(p_calling_bundle, qcsapi_retval);
			statval = 1;
		}
	}

	return statval;
}

static int call_qcsapi_wps_upnp_enable(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int		statval = 0;
	const char	*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output	*print = p_calling_bundle->caller_output;
	int		qcsapi_retval = 0;
	int		option = -1;

	if (argc < 0) {
		print_out(print, "Usage: call_qcsapi wps_upnp_enable <WiFi Interface> <0 | 1>\n");
		statval = 1;
	} else {
		option = atoi(argv[0]);
		if ((strlen(argv[0]) != 1) || (!isdigit(*argv[0])) || ((option != 0) && (option != 1))) {
			print_out(print, "Usage: call_qcsapi wps_upnp_enable <WiFi Interface> <0 | 1>\n");
			statval = 1;
		} else {
			qcsapi_retval = qcsapi_wps_upnp_enable(the_interface, option);

			if (qcsapi_retval >= 0) {
				if (verbose_flag >= 0) {
					print_out( print, "complete\n");
				}
			} else {
				report_qcsapi_error(p_calling_bundle, qcsapi_retval);
				statval = 1;
			}
		}
	}

	return statval;
}

static int call_qcsapi_wps_upnp_status(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int		statval = 0;
	const char	*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output	*print = p_calling_bundle->caller_output;
	int		qcsapi_retval = 0;
	char		reply_buffer[16];

	if (argc > 0) {
		print_out(print, "Usage: call_qcsapi wps_upnp_status <WiFi Interface>\n");
		statval = 1;
	} else {
		memset(reply_buffer, 0, sizeof(reply_buffer));
		qcsapi_retval = qcsapi_wps_upnp_status(the_interface, reply_buffer, sizeof(reply_buffer));

		if (qcsapi_retval >= 0) {
			print_out(print, "%s\n", reply_buffer);
			if (verbose_flag >= 0) {
				print_out( print, "complete\n");
			}
		} else {
			report_qcsapi_error(p_calling_bundle, qcsapi_retval);
			statval = 1;
		}
	}

	return statval;
}

static int  call_qcsapi_wps_registrar_set_dfl_pbc_bss(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int		statval = 0;
	const char	*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output	*print = p_calling_bundle->caller_output;
	int		qcsapi_retval = 0;

	if (argc > 0) {
		print_out(print, "Usage: call_qcsapi registrar_set_default_pbc_bss <WiFi Interface>\n");
		statval = 1;
	} else {
		qcsapi_retval = qcsapi_registrar_set_default_pbc_bss(the_interface);

		if (qcsapi_retval >= 0) {
			if (verbose_flag >= 0) {
				print_out( print, "complete\n");
			}
		} else {
			report_qcsapi_error(p_calling_bundle, qcsapi_retval);
			statval = 1;
		}
	}

	return statval;
}

static int  call_qcsapi_wps_registrar_get_dfl_pbc_bss(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int		statval = 0;
	qcsapi_output	*print = p_calling_bundle->caller_output;
	int		qcsapi_retval = 0;
	char		reply_buffer[16];

	if (argc > 0) {
		print_out(print, "Usage: call_qcsapi registrar_get_default_pbc_bss\n");
		statval = 1;
	} else {
		memset(reply_buffer, 0, sizeof(reply_buffer));
		qcsapi_retval = qcsapi_registrar_get_default_pbc_bss(reply_buffer, sizeof(reply_buffer));

		if (qcsapi_retval >= 0) {
			if (verbose_flag >= 0) {
				print_out(print, "%s\n", reply_buffer);
			}
		} else {
			report_qcsapi_error(p_calling_bundle, qcsapi_retval);
			statval = 1;
		}
	}

	return statval;
}

static int
call_qcsapi_reset_all_counters(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int		statval = 0;
	const char	*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output	*print = p_calling_bundle->caller_output;
	qcsapi_unsigned_int node_index = p_calling_bundle->caller_generic_parameter.index;
	int local_remote_flag = QCSAPI_LOCAL_NODE;
	int qcsapi_retval = 0;

	if (argc > 0) {
		if (parse_local_remote_flag(print, argv[0], &local_remote_flag) < 0) {
			return 1;
		}
	}

	qcsapi_retval = qcsapi_reset_all_counters(the_interface, node_index, local_remote_flag);

	if (qcsapi_retval == 0) {
		if (verbose_flag >= 0) {
			print_out( print, "complete\n");
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_test_traffic(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int		statval = 0;
	const char	*the_interface = p_calling_bundle->caller_interface;
	qcsapi_output	*print = p_calling_bundle->caller_output;
	int		qcsapi_retval = 0;
	uint32_t period = 0;

	if (argc < 1 || argc > 2) {
		statval = 1;
	} else {
		if ((argc == 2) && (!strcasecmp("start", argv[0]))) {
			sscanf(argv[1], "%u", &period);
			if (period < 10) {
				statval = 1;
				print_err( print, "<period> MUST >= 10 milliseconds for \"start\"\n");
				goto out;
			}
		} else if ((argc == 1) && (!strcasecmp("stop", argv[0]))) {
			period = 0;
		} else {
			statval = 1;
			goto out;
		}

		qcsapi_retval = qcsapi_wifi_test_traffic( the_interface, period );

		if (qcsapi_retval >= 0) {
			if (verbose_flag >= 0) {
				print_out( print, "complete\n" );
			}
		} else {
			report_qcsapi_error( p_calling_bundle, qcsapi_retval );
			statval = 1;
		}
	}

out:
	if (statval != 0 && qcsapi_retval >= 0) {
		print_err( print, "Usage: call_qcsapi test_traffic <WiFi interface> <start|stop> <period (unit:ms)>\n" );
		print_err( print, "Usage: This command is used to start or stop the test traffic\n" );
	}
	return statval;
}

static int
call_qcsapi_pm_get_set_mode(call_qcsapi_bundle *call, int argc, char *argv[])
{
	qcsapi_output *print = call->caller_output;
	int level;
	int rc = 0;

	if (argc == 1) {
		const char *arg = argv[0];
		if (strcmp(arg, "off") == 0) {
			level = QCSAPI_PM_MODE_DISABLE;
		} else if (strcmp(arg, "on") == 0 || strcmp(arg, "auto") == 0) {
			level = QCSAPI_PM_MODE_AUTO;
		} else if (strcmp(arg, "suspend") == 0) {
			level = QCSAPI_PM_MODE_SUSPEND;
		} else {
			print_err(print, "%s: could not parse '%s'\n", __FUNCTION__, arg);
			rc = -EINVAL;
			goto out;
		}

		rc = qcsapi_pm_set_mode(level);
	} else if (argc == 0) {
		rc = qcsapi_pm_get_mode(&level);
		if (rc < 0 || verbose_flag < 0) {
			goto out;
		}

		switch (level) {
		case QCSAPI_PM_MODE_DISABLE:
			print_out(print, "off\n");
			break;
		case QCSAPI_PM_MODE_SUSPEND:
			print_out(print, "suspend\n");
			break;
		default:
			print_out(print, "auto\n");
			break;
		}
	} else {
		rc = -EINVAL;
	}

out:
	if (rc < 0) {
		report_qcsapi_error(call, rc);
	}

	return rc;
}

static int
call_qcsapi_qpm_get_level(call_qcsapi_bundle *call, int argc, char *argv[])
{
        qcsapi_output *print = call->caller_output;
        int qpm_level;
        int rc = 0;

        if (argc == 0) {
                rc = qcsapi_get_qpm_level(&qpm_level);
                if (rc < 0 || verbose_flag < 0) {
                        goto out;
                }
                print_out(print, "%d\n", qpm_level);

        } else {
                rc = -EINVAL;
        }

out:
        if (rc < 0) {
                report_qcsapi_error(call, rc);
        }

        return rc;
}

static int
call_qcsapi_restore_default_config(call_qcsapi_bundle *call, int argc, char *argv[])
{
	int flag = 0;
	char *argp;
	int rc;

	while (argc > 0) {
		argp = argv[argc - 1];
		if (strcmp(argp, "1") == 0 ||
				strcmp(argp, "ip") == 0) {
			flag |= QCSAPI_RESTORE_FG_IP;
		} else if (strcmp(argp, "noreboot") == 0) {
			flag |= QCSAPI_RESTORE_FG_NOREBOOT;
		} else if (strcmp(argp, "ap") == 0) {
			flag |= QCSAPI_RESTORE_FG_AP;
		} else if (strcmp(argp, "sta") == 0) {
			flag |= QCSAPI_RESTORE_FG_STA;
		}
		argc--;
	}

	rc = qcsapi_restore_default_config(flag);

	if (rc < 0)
		report_qcsapi_error(call, rc);

	return rc;
}

static int g_is_qtm = 0;

typedef int(*call_qcsapi_vsp_fn)(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[]);

struct call_qcsapi_fnmap {
	const char *name;
	call_qcsapi_vsp_fn func;
};

static call_qcsapi_vsp_fn call_qcsapi_fnmap_find(const char *name, const struct call_qcsapi_fnmap *map, size_t map_size)
{
	int i;
	for (i = 0; i < map_size; i++) {

		/* skip whitelist */
		if (g_is_qtm && strcmp(map[i].name, "wl") == 0)
			continue;

		if (strcmp(map[i].name, name) == 0) {
			return map[i].func;
		}
	}

	return NULL;
}

static const struct qvsp_cfg_param qvsp_cfg_params[] = QVSP_CFG_PARAMS;

static uint32_t
qvsp_cfg_param_get(const char *name)
{
	int i;

	for (i = 0; i < QVSP_CFG_MAX; i++) {
		if (strcmp(name, qvsp_cfg_params[i].name) == 0) {
			return i;
		}
	}
	return QVSP_CFG_MAX;
}

static uint32_t qvsp_cfg_name_len = 0;
static uint32_t qvsp_cfg_units_len = 0;
static uint32_t qvsp_rule_name_len = 0;
static uint32_t qvsp_rule_units_len = 0;
static int qvsp_show_cfg_initialised = 0;

static const char *qvsp_inactive_reason[] = QVSP_INACTIVE_REASON;

/*
 * Getting VSP version: whether it is VSP (v1 for Ruby) or QTM (Quantenna Traffic Management, v2 for Topaz)
 */
static int
qvsp_is_qtm()
{
	struct qcsapi_data_512bytes *stats;
	struct qvsp_stats *p_stats;
	int rc;

	stats = malloc(sizeof(*stats));
	if (stats == NULL)
		return -ENOMEM;
	p_stats = (struct qvsp_stats *)stats->data;
	rc = qcsapi_qtm_get_stats("wifi0", stats);
	free(stats);
	if (rc < 0) {
		return rc;
	}

	return p_stats->is_qtm;
}

static int
call_qcsapi_vsp_is_active(call_qcsapi_bundle *call)
{
	qcsapi_output *print = call->caller_output;
	unsigned long inactive_flags = 0;
	int rc;
	int i;
	int first = 1;

	rc = qcsapi_qtm_get_inactive_flags(call->caller_interface, &inactive_flags);
	if (rc || inactive_flags) {
		if (rc == 0) {
			print_out(print, "QTM is inactive - reason:");

			for ( i = 0; i < ARRAY_SIZE(qvsp_inactive_reason); i++) {
				if ((inactive_flags & 0x1) && qvsp_inactive_reason[i]) {
					if (!first) {
						print_out(print, ", %s", qvsp_inactive_reason[i]);
					} else {
						print_out(print, " %s", qvsp_inactive_reason[i]);
						first = 0;
					}
				}
				inactive_flags >>= 1;
			}
			print_out(print, "\n");
		}
		return -EPERM;
	}

	return rc;
}

static int
call_qcsapi_vsp_is_enabled(call_qcsapi_bundle *call)
{
	qcsapi_output *print = call->caller_output;
	unsigned int enabled;
	int rc;

	rc = qcsapi_qtm_get_config(call->caller_interface, QVSP_CFG_ENABLED, &enabled);
	if (rc || (enabled == 0)) {
		print_out(print, "QTM is not enabled\n");
		return -EPERM;
	}

	return rc;
}

static void
call_qcsapi_vsp_cfg_paramlist(qcsapi_output *print)
{
	int i;
	const struct qvsp_cfg_param *param;
	int buflen = qvsp_cfg_name_len + qvsp_cfg_units_len + 5;
	char buf[buflen];

	print_out(print, "Parameters\n");


	for (i = 0; i < QVSP_CFG_MAX; i++) {
		param = &qvsp_cfg_params[i];
		snprintf(buf, buflen, "%s <%s>",
				param->name, param->units);
		print_out(print, "    %-*s   - %s [%u to %u]\n",
				buflen,
				buf,
				param->desc,
				param->min_val,
				param->max_val);
	}
}

static void
call_qcsapi_vsp_get_usage(qcsapi_output *print)
{
	print_out(print, "Usage\n"
			"    <qcsapi> qtm <if> get <param>\n\n");

	call_qcsapi_vsp_cfg_paramlist(print);
}

static void
call_qcsapi_vsp_set_usage(qcsapi_output *print)
{
	print_out(print, "Usage\n"
		"    <qcsapi> qtm <if> set <param> <val>\n\n");

	call_qcsapi_vsp_cfg_paramlist(print);
}

static int
call_qcsapi_vsp_get(call_qcsapi_bundle *call, int argc, char *argv[])
{
	qcsapi_output *print = call->caller_output;
	unsigned int index;
	unsigned int value;
	int ret;

	if (argc != 1) {
		call_qcsapi_vsp_get_usage(print);
		return -EINVAL;
	}

	index = qvsp_cfg_param_get(argv[0]);
	if (index >= QVSP_CFG_MAX) {
		call_qcsapi_vsp_get_usage(print);
		return -EINVAL;
	}

	ret = qcsapi_qtm_get_config(call->caller_interface, index, &value);
	if (ret) {
		if (ret == -EINVAL) {
			call_qcsapi_vsp_set_usage(print);
		} else if (ret == -qcsapi_not_supported) {
			print_err(print, "QTM is not supported\n");
		} else {
			print_err(print, "QTM get command failed\n");
		}
		return ret;
	}

	print_out(print, "%u\n", value);

	return 0;
}

static int
call_qcsapi_vsp_set(call_qcsapi_bundle *call, int argc, char *argv[])
{
	qcsapi_output *print = call->caller_output;
	unsigned int index;
	unsigned int value;
	int ret;

	if (argc != 2) {
		call_qcsapi_vsp_set_usage(print);
		return -EINVAL;
	}

	index = qvsp_cfg_param_get(argv[0]);
	ret = sscanf(argv[1], "%u", &value);

	if (index >= QVSP_CFG_MAX) {
		print_err(print, "Invalid argument: '%s'\n", argv[0]);
		call_qcsapi_vsp_set_usage(print);
		return -EINVAL;
	} else if (ret != 1) {
		print_err(print, "Error parsing argument '%s'\n", argv[1]);
		return -EINVAL;
	}

	ret = qcsapi_qtm_set_config(call->caller_interface, index, value);
	if (ret != 0) {
		if (ret == -EINVAL) {
			call_qcsapi_vsp_set_usage(print);
		} else if (ret == -qcsapi_not_supported) {
			print_err(print, "QTM is not supported\n");
		} else {
			print_err(print, "QTM set command failed\n");
		}
		return -EINVAL;
	}

	return 0;
}

const struct qvsp_rule_param qvsp_rule_params[] = QVSP_RULE_PARAMS;

static uint32_t
qvsp_rule_param_get(const char *name)
{
	int i;

	for (i = 0; i < QVSP_RULE_PARAM_MAX; i++) {
		if (strcmp(name, qvsp_rule_params[i].name) == 0) {
			return i;
		}
	}
	return QVSP_RULE_PARAM_MAX;
}

const static char *qvsp_rule_order_desc[] = QVSP_RULE_ORDER_DESCS;
const static char *qvsp_rule_order_desc_abbr[] = QVSP_RULE_ORDER_DESCS_ABBR;
const static char *qvsp_rule_dir_desc[] = QVSP_RULE_DIR_DESCS;
const static char *qvsp_if_desc[] = QVSP_IF_DESCS;
const static char *qvsp_strm_throt_desc_abbr[] = QVSP_STRM_THROT_DESCS_ABBR;

static void
call_qcsapi_vsp_rule_usage(qcsapi_output *print)
{
	const struct qvsp_rule_param *rule_param;
	int i;
	int j;
	int buflen = qvsp_rule_name_len + qvsp_rule_units_len + 6;
	char buf[buflen];

	print_out(print, "Usage\n"
			"    <qcsapi> qtm <if> rule add <param> <val> [<param> <val> ...]\n"
			"                                  - set a stream matching rule\n"
			"    <qcsapi> qtm <if> rule del    - delete all stream matching rules\n"
			"    <qcsapi> qtm <if> rule del [<rule_num>]\n"
			"                                  - delete a stream matching rule\n"
			"\n"
			"Parameters\n");

	for (i = 0; i < QVSP_RULE_PARAM_MAX; i++) {
		rule_param = &qvsp_rule_params[i];
		snprintf(buf, buflen, "    %s <%s>",
				rule_param->name, rule_param->units);
		print_out(print, "%-*s   - %s [%u to %u]\n",
				buflen,
				buf,
				rule_param->desc,
				rule_param->min_val,
				rule_param->max_val);
		switch (i) {
		case QVSP_RULE_PARAM_DIR:
			print_out(print, "%-*s       %u = Any\n", buflen, "", QVSP_RULE_DIR_ANY);
			print_out(print, "%-*s       %u = Tx\n", buflen, "", QVSP_RULE_DIR_TX);
			print_out(print, "%-*s       %u = Rx\n", buflen, "", QVSP_RULE_DIR_RX);
			break;
		case QVSP_RULE_PARAM_VAPPRI:
			print_out(print, "%-*s       0x01 = VAP Priority 0\n", buflen, "");
			print_out(print, "%-*s       0x02 = VAP Priority 1\n", buflen, "");
			print_out(print, "%-*s       0x04 = VAP Priority 2\n", buflen, "");
			print_out(print, "%-*s       0x08 = VAP Priority 3\n", buflen, "");
			break;
		case QVSP_RULE_PARAM_AC:
			print_out(print, "%-*s       0x01 = Best Effort (0)\n", buflen, "");
			print_out(print, "%-*s       0x02 = Background (1)\n", buflen, "");
			print_out(print, "%-*s       0x04 = Video (2)\n", buflen, "");
			print_out(print, "%-*s       0x08 = Voice (3)\n", buflen, "");
			break;
		case QVSP_RULE_PARAM_ORDER:
			for (j = 0; j < QVSP_RULE_ORDER_MAX; j++) {
				print_out(print, "%-*s       %u - %s\n",
						buflen,
						"",
						j,
						qvsp_rule_order_desc[j]);
			}
			break;
		default:
			break;
		}
	}
}

static int call_qcsapi_vsp_rule_parse(qcsapi_output *print,
		int argc, char **argv, struct qvsp_rule_flds *rule_fields)
{
	const struct qvsp_rule_param *rule_param;
	uint32_t rule_param_num;
	uint32_t val;
	int i;
	int ret;

	/* Must be field/value pairs */
	if (argc & 0x1) {
		call_qcsapi_vsp_rule_usage(print);
		return -EINVAL;
	}

	memset(rule_fields, 0, sizeof(*rule_fields));
	/* fields that are not 0 by default */
	rule_fields->param[QVSP_RULE_PARAM_THROT_POLICY] = QVSP_STRM_THROT_ADPT;
	if (!g_is_qtm)
		rule_fields->param[QVSP_RULE_PARAM_DEMOTE] = 1;

	for (i = 0; i < argc; i = i + 2) {
		ret = sscanf(argv[i + 1], "%u", &val);
		if (ret != 1) {
			print_err(print, "QTM: error parsing number: '%s'\n", argv[i + 1]);
			return -EINVAL;
		}

		rule_param_num = qvsp_rule_param_get(argv[i]);
		if (rule_param_num == QVSP_RULE_PARAM_MAX) {
			print_err(print, "QTM: invalid rule - %s\n", argv[i]);
			return -EINVAL;
		}

		rule_param = &qvsp_rule_params[rule_param_num];

		if ((val < rule_param->min_val) || (val > rule_param->max_val)) {
			print_err(print, "QTM: value for %s must be between %u and %u\n",
					argv[i], rule_param->min_val, rule_param->max_val);
			return -EINVAL;
		}

		if ((rule_param_num == QVSP_RULE_PARAM_PROTOCOL) &&
				(val != IPPROTO_UDP) && (val != IPPROTO_TCP)) {
			print_err(print, "QTM: protocol must be %u (TCP) or %u (UDP)\n",
					IPPROTO_TCP, IPPROTO_UDP);
			return -EINVAL;
		}

		rule_fields->param[rule_param_num] = val;
	}

	return 0;
}


static int
call_qcsapi_vsp_rule_add(call_qcsapi_bundle *call, int argc, char *argv[])
{
	qcsapi_output *print = call->caller_output;
	struct qvsp_rule_flds rule_fields;
	struct qcsapi_data_128bytes rule_flds_buf;
	int rc;

	if (argc == 0) {
		call_qcsapi_vsp_rule_usage(print);
		return -EINVAL;
	}

	rc = call_qcsapi_vsp_rule_parse(print, argc, argv, &rule_fields);
	if (rc) {
		return rc;
	}

	memcpy(rule_flds_buf.data, &rule_fields, sizeof(rule_flds_buf.data));
	return qcsapi_qtm_add_rule(call->caller_interface, &rule_flds_buf);
}

static int
call_qcsapi_vsp_rule_del(call_qcsapi_bundle *call, int argc, char *argv[])
{
	qcsapi_output *print = call->caller_output;
	struct qvsp_rule_flds rule_fields;
	struct qcsapi_data_128bytes rule_flds_buf;
	int rc;

	if (argc >= 2 && argc % 2 == 0) {
		rc = call_qcsapi_vsp_rule_parse(print, argc, argv, &rule_fields);
		if (rc) {
			return rc;
		}

		memcpy(rule_flds_buf.data, &rule_fields, sizeof(rule_flds_buf.data));
		return qcsapi_qtm_del_rule(call->caller_interface, &rule_flds_buf);
	} else if (argc == 1) {
		unsigned int index;
		rc = sscanf(argv[0], "%u", &index);
		if (rc != 1) {
			print_err(print, "Error parsing argument '%s'\n", argv[0]);
			return -EINVAL;
		}

		return qcsapi_qtm_del_rule_index(call->caller_interface, index);
	} else if (argc == 0) {
		return qcsapi_qtm_del_rule_index(call->caller_interface, ~0);
	} else {
		call_qcsapi_vsp_rule_usage(print);
		return -EINVAL;
	}
}

static const char *
call_qcsapi_vsp_dir_desc(enum qvsp_rule_dir_e dir)
{
	if (dir < ARRAY_SIZE(qvsp_rule_dir_desc)) {
		return qvsp_rule_dir_desc[dir];
	}
	return "invalid";
}

static int
call_qcsapi_vsp_rule_getlist(call_qcsapi_bundle *call, int argc, char *argv[])
{
	qcsapi_output *print = call->caller_output;
	const static unsigned int MAX_RULES = 64;
	const struct qvsp_rule_param *rule_param;
	struct qcsapi_data_3Kbytes *rules_buf;
	struct qvsp_rule_flds *rules;
	struct qvsp_rule_flds default_rule;
	int n;
	int i;
	int j;
	int rc = 0;

	rules_buf = malloc(sizeof(struct qcsapi_data_3Kbytes));
	if (rules_buf == NULL)
		return -ENOMEM;
	rules = (struct qvsp_rule_flds *)rules_buf->data;

	memset(&default_rule, 0, sizeof(default_rule));
	n = qcsapi_qtm_get_rule(call->caller_interface, rules_buf, MAX_RULES);
	if (n < 0) {
		rc = n;
		goto out;
	}

	print_out(print, "Rules\n");
	print_out(print, "    Rule ");
	for (i = 0; i < QVSP_RULE_PARAM_MAX; i++) {
		rule_param = &qvsp_rule_params[i];
		print_out(print, "%-8s ", rule_param->name);
	}
	print_out(print, "\n");

	if (n == 0) {
		print_out(print, "    no rules configured\n");
	}
	for (j = 0; j < n; j++) {
		const struct qvsp_rule_flds *rf = &rules[j];

		if (memcmp(&rules[j], &default_rule, sizeof(default_rule)) == 0) {
			print_out(print, "    dflt ");
		} else {
			print_out(print, "    %-4d ", j + 1);
		}
		for (i = 0; i < QVSP_RULE_PARAM_MAX; i++) {
			switch (i) {
			case QVSP_RULE_PARAM_DIR:
				print_out(print, "%-8s ",
					call_qcsapi_vsp_dir_desc(rf->param[i]));
				break;
			case QVSP_RULE_PARAM_VAPPRI:
			case QVSP_RULE_PARAM_AC:
				print_out(print, "0x%-6x ", rf->param[i]);
				break;
			case QVSP_RULE_PARAM_ORDER:
				print_out(print, "%u - %-4s ", rf->param[i],
						qvsp_rule_order_desc_abbr[rf->param[i]]);
				break;
			case QVSP_RULE_PARAM_THROT_POLICY:
				print_out(print, "%u - %-8s ", rf->param[i],
						qvsp_strm_throt_desc_abbr[rf->param[i]]);
				break;
			default:
				print_out(print, "%-8u ", rf->param[i]);
				break;
			}
		}
		print_out(print, "\n");
	}

out:
	if (rules_buf != NULL)
		free(rules_buf);
	return rc;
}

static int
call_qcsapi_vsp_rule(call_qcsapi_bundle *call, int argc, char *argv[])
{
	qcsapi_output *print = call->caller_output;
	call_qcsapi_vsp_fn func;

	static const struct call_qcsapi_fnmap mux[] = {
		{ "add",	call_qcsapi_vsp_rule_add },
		{ "del",	call_qcsapi_vsp_rule_del },
		{ "getlist",	call_qcsapi_vsp_rule_getlist },
	};

	if (argc < 1) {
		call_qcsapi_vsp_rule_usage(print);
		return -EINVAL;
	}

	func = call_qcsapi_fnmap_find(argv[0], mux, ARRAY_SIZE(mux));
	if (func == NULL) {
		call_qcsapi_vsp_rule_usage(print);
		return -EINVAL;
	} else {
		return func(call, argc - 1, argv + 1);
	}
}

#define QCSAPI_CIDR_STRLEN	(4)
#define QCSAPI_PORT_STRLEN	(6)

static void
call_qcsapi_vsp_wl_usage(qcsapi_output *print)
{
	print_out(print, "Usage\n"
			"    <qcsapi> qtm <if> wl add <saddr>[/<netmask>] <sport> <daddr>[/<netmask>] <dport>\n"
			"                               - add a whitelist entry\n"
			"    <qcsapi> qtm <if> wl del <saddr>[/<netmask>] <sport> <daddr>[/<netmask>] <dport>\n"
			"                               - delete a whitelist entry\n"
			"    <qcsapi> qtm <if> wl del\n"
			"                               - delete all whitelist entries\n"
			"\n"
			"Parameters\n"
			"  IPv4:\n"
			"    <saddr>                    - IP source address (0 for any)\n"
			"    <daddr>                    - IP destination address (0 for any)\n"
			"    <netmask>                  - IP netmask (1-32, default is 32)\n"
			"  IPv6:\n"
			"    <saddr>                    - IP source address (::0 for any)\n"
			"    <daddr>                    - IP destination address (::0 for any)\n"
			"    <netmask>                  - IP netmask (1-128, default is 128)\n"
			"  IPv4 or IPv6:\n"
			"    <sport>                    - IP source port (0 for any)\n"
			"    <dport>                    - IP destination port (0 for any)\n"
		 );
}

static int
call_qcsapi_parse_ip_cidr(call_qcsapi_bundle *call, const char *addr,
		__be32 *ipv4, struct in6_addr *ipv6, uint8_t *cidr)
{
	qcsapi_output *print = call->caller_output;
	int rc;
	int ipv;
	int max_cidr;
	char ipscan[128];

	rc = sscanf(addr, "%[^/]/%hhu", ipscan, cidr);

	if (strcmp(ipscan, "0") == 0) {
		*ipv4 = 0;
		ipv = 4;
	} else if (inet_pton(AF_INET, ipscan, ipv4) == 1) {
		ipv = 4;
	} else if (inet_pton(AF_INET6, ipscan, ipv6) == 1) {
		ipv = 6;
	} else {
		print_err(print, "Invalid value parsing ip[/mask] '%s'\n", addr);
		return -EINVAL;
	}

	if (ipv == 4) {
		max_cidr = sizeof(*ipv4) * NBBY;
	} else {
		max_cidr = sizeof(*ipv6) * NBBY;
	}

	if (rc == 2) {
		if (*cidr > max_cidr) {
			print_err(print, "Invalid CIDR (%u) for IPv%d address %s\n",
					*cidr, ipv, ipscan);
			return -EINVAL;
		}
	} else {
		*cidr = max_cidr;
	}

	return ipv;
}

static int
call_qcsapi_vsp_wl_parse_wlf(call_qcsapi_bundle *call, struct qvsp_wl_flds *wl_fields,
		const char *saddr, const char *daddr)
{
	qcsapi_output *print = call->caller_output;
	int rc;
	int ipv;

	rc = call_qcsapi_parse_ip_cidr(call, saddr,
			&wl_fields->hflds.ipv4.saddr,
			&wl_fields->hflds.ipv6.saddr,
			&wl_fields->s_cidr_bits);
	if (rc < 0) {
		return -EINVAL;
	}
	ipv = rc;

	rc = call_qcsapi_parse_ip_cidr(call, daddr,
			&wl_fields->hflds.ipv4.daddr,
			&wl_fields->hflds.ipv6.daddr,
			&wl_fields->d_cidr_bits);
	if (rc < 0) {
		return -EINVAL;
	} else if (rc != ipv) {
		print_err(print, "IP addresses are not both IPv4 or IPv6\n");
		return -EINVAL;
	}

	wl_fields->ip_version = ipv;

	return ipv;
}

static int
call_qcsapi_vsp_wl_parse(call_qcsapi_bundle *call, int argc, char *argv[], struct qvsp_wl_flds *wl_fields)
{
	qcsapi_output *print = call->caller_output;
	uint16_t sport;
	uint16_t dport;
	int ipv;

	if (argc != 4) {
		call_qcsapi_vsp_wl_usage(print);
		return -EINVAL;
	}

	ipv = call_qcsapi_vsp_wl_parse_wlf(call, wl_fields, argv[0], argv[2]);
	if (ipv < 0) {
		return -EINVAL;
	}

	if (sscanf(argv[1], "%hu", &sport) != 1) {
		print_err(print, "Error parsing source port '%s'\n", argv[1]);
		return -EINVAL;
	}
	if (sscanf(argv[3], "%hu", &dport) != 1) {
		print_err(print, "Error parsing destination port '%s'\n", argv[3]);
		return -EINVAL;
	}

	if (ipv == 4) {
		wl_fields->hflds.ipv4.sport = htons(sport);
		wl_fields->hflds.ipv4.dport = htons(dport);
	} else {
		wl_fields->hflds.ipv6.sport = htons(sport);
		wl_fields->hflds.ipv6.dport = htons(dport);
	}

	return 0;
}

static int
call_qcsapi_vsp_wl_add(call_qcsapi_bundle *call, int argc, char *argv[])
{
	struct qvsp_wl_flds wl;
	int rc;

	if (g_is_qtm)
		return 0;

	rc = call_qcsapi_vsp_wl_parse(call, argc, argv, &wl);
	if (rc) {
		return rc;
	}

#ifdef CALL_QCSAPI_QTM_UNSUPPORTED
	return qcsapi_vsp_add_wl(call->caller_interface, &wl);
#else
	return 0;
#endif
}

static int
call_qcsapi_vsp_wl_del(call_qcsapi_bundle *call, int argc, char *argv[])
{
	qcsapi_output *print = call->caller_output;
	struct qvsp_wl_flds wl;
	int rc;

	if (g_is_qtm)
		return 0;

	if (argc == 4) {
		rc = call_qcsapi_vsp_wl_parse(call, argc, argv, &wl);
		if (rc) {
			return rc;
		}

#ifdef CALL_QCSAPI_QTM_UNSUPPORTED
		return qcsapi_vsp_del_wl(call->caller_interface, &wl);
#else
		return 0;
#endif
	} else if (argc == 1) {
		unsigned int index;
		rc = sscanf(argv[0], "%u", &index);
		if (rc != 1) {
			print_err(print, "Error parsing argument '%s'\n", argv[0]);
			return -EINVAL;
		}

#ifdef CALL_QCSAPI_QTM_UNSUPPORTED
		return qcsapi_vsp_del_wl_index(call->caller_interface, index);
#else
		return 0;
#endif
	} else if (argc == 0) {
#ifdef CALL_QCSAPI_QTM_UNSUPPORTED
		return qcsapi_vsp_del_wl_index(call->caller_interface, ~0);
#else
		return 0;
#endif
	} else {
		call_qcsapi_vsp_wl_usage(print);
		return -EINVAL;
	}
}

static void call_qcsapi_vsp_wl_fmt(const struct qvsp_wl_flds *wlf, char *saddr, char *daddr, char *sport, char *dport)
{
	const struct in6_addr zero = IN6ADDR_ANY_INIT;

	strcpy(saddr, QVSP_CFG_SHOW_ANYSTR);
	strcpy(daddr, QVSP_CFG_SHOW_ANYSTR);
	strcpy(sport, QVSP_CFG_SHOW_ANYSTR);
	strcpy(dport, QVSP_CFG_SHOW_ANYSTR);

	if (wlf->ip_version == 4) {
		const struct qvsp_hash_flds_ipv4 *ip = &wlf->hflds.ipv4;

		if (ip->saddr) {
			sprintf(saddr, NIPQUAD_FMT "/%u",
					NIPQUAD(ip->saddr),
					wlf->s_cidr_bits);
		}
		if (ip->daddr) {
			sprintf(daddr, NIPQUAD_FMT "/%u",
					NIPQUAD(ip->daddr),
					wlf->d_cidr_bits);
		}
		if (ip->sport) {
			sprintf(sport, "%u", ntohs(ip->sport));
		}
		if (ip->dport) {
			sprintf(dport, "%u", ntohs(ip->dport));
		}
	} else {
		const struct qvsp_hash_flds_ipv6 *ip = &wlf->hflds.ipv6;
		char cidr[QCSAPI_CIDR_STRLEN];

		if (memcmp(&ip->saddr, &zero, sizeof(ip->saddr))) {
			inet_ntop(AF_INET6, &ip->saddr,
					saddr, INET6_ADDRSTRLEN);
			sprintf(cidr, "/%u", wlf->s_cidr_bits);
			strcat(saddr, cidr);
		}
		if (memcmp(&ip->daddr, &zero, sizeof(ip->daddr))) {
			inet_ntop(AF_INET6, &ip->daddr,
					daddr, INET6_ADDRSTRLEN);
			sprintf(cidr, "/%u", wlf->d_cidr_bits);
			strcat(daddr, cidr);
		}
		if (ip->sport) {
			sprintf(sport, "%u", ntohs(ip->sport));
		}
		if (ip->dport) {
			sprintf(dport, "%u", ntohs(ip->dport));
		}
	}
}

static int
call_qcsapi_vsp_wl_getlist(call_qcsapi_bundle *call, int argc, char *argv[])
{
	qcsapi_output *print = call->caller_output;
	const unsigned int MAX_WL = 64;
	const char *whitelist_fmt = "    %-*s %-*s %-*s %-*s\n";
	char saddr[INET6_ADDRSTRLEN + 1];
	char daddr[INET6_ADDRSTRLEN + 1];
	char sport[QCSAPI_PORT_STRLEN + 1];
	char dport[QCSAPI_PORT_STRLEN + 1];
	int max_saddr_strlen = NIPQUAD_LEN;
	int max_daddr_strlen = NIPQUAD_LEN;
	struct qvsp_wl_flds *wl;
	int n;
	int i;
	int rc = 0;

	if (g_is_qtm)
		return 0;

	wl = malloc(sizeof(*wl) * MAX_WL);
	if (!wl) {
		return -ENOMEM;
	}

#ifdef CALL_QCSAPI_QTM_UNSUPPORTED
	n = qcsapi_vsp_get_wl(call->caller_interface, wl, MAX_WL);
#else
	n = 0;
#endif
	if (n < 0) {
		rc = n;
	} else {
		/* find max string length for ip addresses */
		for (i = 0; i < n; i++) {
			int saddr_strlen;
			int daddr_strlen;
			call_qcsapi_vsp_wl_fmt(&wl[i], saddr, daddr, sport, dport);
			saddr_strlen = strlen(saddr);
			daddr_strlen = strlen(daddr);
			max_saddr_strlen = max(max_saddr_strlen, saddr_strlen);
			max_daddr_strlen = max(max_daddr_strlen, daddr_strlen);
		}

		print_out(print, "Whitelist\n");
		print_out(print, whitelist_fmt,
				max_saddr_strlen, "Source IP", QCSAPI_PORT_STRLEN, "SPort",
				max_daddr_strlen, "Dest IP", QCSAPI_PORT_STRLEN, "DPort");
		if (n == 0) {
			print_out(print, "    no whitelist entries\n");
		}

		for (i = 0; i < n; i++) {
			call_qcsapi_vsp_wl_fmt(&wl[i], saddr, daddr, sport, dport);
			print_out(print, whitelist_fmt,
					max_saddr_strlen, saddr, QCSAPI_PORT_STRLEN, sport,
					max_daddr_strlen, daddr, QCSAPI_PORT_STRLEN, dport);
		}
	}

	free(wl);

	return rc;
}

static int
call_qcsapi_vsp_wl(call_qcsapi_bundle *call, int argc, char *argv[])
{
	qcsapi_output *print = call->caller_output;
	call_qcsapi_vsp_fn func;

	static const struct call_qcsapi_fnmap mux[] = {
		{ "add",	call_qcsapi_vsp_wl_add },
		{ "del",	call_qcsapi_vsp_wl_del },
		{ "getlist",	call_qcsapi_vsp_wl_getlist },
	};

	if (argc < 1) {
		call_qcsapi_vsp_wl_usage(print);
		return -EINVAL;
	}

	func = call_qcsapi_fnmap_find(argv[0], mux, ARRAY_SIZE(mux));
	if (func == NULL) {
		call_qcsapi_vsp_rule_usage(print);
		return -EINVAL;
	} else {
		return func(call, argc - 1, argv + 1);
	}
}

static void
call_qcsapi_vsp_show_usage(qcsapi_output *print)
{
	print_out(print, "Usage\n"
		"    <qcsapi> qtm <if> show          - show current state and high throughput streams\n"
		"    <qcsapi> qtm <if> show all      - show current state and all streams\n"
		"    <qcsapi> qtm <if> show stats    - show stream and packet statistics\n"
		"    <qcsapi> qtm <if> show config   - show config parameters, rules, and whitelist\n");
}

static int
call_qcsapi_vsp_show_config(call_qcsapi_bundle *call, int argc, char *argv[])
{
	qcsapi_wifi_mode wifi_mode = qcsapi_nosuch_mode;
	qcsapi_output *print = call->caller_output;
	struct qcsapi_data_1Kbytes *cfg_buf;
	unsigned int *cfg;
	int rc;
	int i;

	cfg_buf = (struct qcsapi_data_1Kbytes *)malloc(sizeof(struct qcsapi_data_1Kbytes));
	if (cfg_buf == NULL) {
		return -ENOMEM;
	}
	cfg = (unsigned int *)cfg_buf->data;

	rc = qcsapi_qtm_get_config_all(call->caller_interface, cfg_buf, QVSP_CFG_MAX);
	if (rc) {
		free(cfg_buf);
		return rc;
	}

	print_out(print, "Parameters\n");
	for (i = 0; i < QVSP_CFG_MAX; i++) {
		if (cfg[i] == QCSAPI_QTM_CFG_INVALID)
			continue;
		print_out(print, "    %-*s  %-8u\n",
				qvsp_cfg_name_len,
				qvsp_cfg_params[i].name,
				cfg[i]);
	}
	free(cfg_buf);

	qcsapi_wifi_get_mode(call->caller_interface, &wifi_mode);
	if (wifi_mode == qcsapi_station) {
		return 0;
	}

	print_out(print, "\n");
	call_qcsapi_vsp_rule_getlist(call, 0, NULL);

	if (!g_is_qtm) {
		print_out(print, "\n");
		call_qcsapi_vsp_wl_getlist(call, 0, NULL);
	}
	return 0;
}

static const char *
call_qcsapi_qvsp_state_desc(enum qvsp_strm_state_e strm_state)
{
	switch (strm_state) {
	case QVSP_STRM_STATE_NONE:
		return "none";
	case QVSP_STRM_STATE_DISABLED:
		return "dis";
	case QVSP_STRM_STATE_LOW_TPUT:
		return "low";
	case QVSP_STRM_STATE_PRE_ENABLED:
		return "pre";
	case QVSP_STRM_STATE_ENABLED:
		return "ena";
	case QVSP_STRM_STATE_DELETED:
		return "del";
	case QVSP_STRM_STATE_MAX:
		break;
	}

	return "invalid";
}

static void call_qcsapi_vsp_strm_fmt(const struct qvsp_strm_info *strm, char *saddr, char *daddr, char *sport, char *dport)
{
	if (strm->ip_version == 4) {
		inet_ntop(AF_INET, &strm->hash_flds.ipv4.saddr, saddr, INET_ADDRSTRLEN);
		inet_ntop(AF_INET, &strm->hash_flds.ipv4.daddr, daddr, INET_ADDRSTRLEN);
		sprintf(sport, "%hu", ntohs(strm->hash_flds.ipv4.sport));
		sprintf(dport, "%hu", ntohs(strm->hash_flds.ipv4.dport));
	} else {
		inet_ntop(AF_INET6, &strm->hash_flds.ipv6.saddr, saddr, INET6_ADDRSTRLEN);
		inet_ntop(AF_INET6, &strm->hash_flds.ipv6.daddr, daddr, INET6_ADDRSTRLEN);
		sprintf(sport, "%hu", ntohs(strm->hash_flds.ipv6.sport));
		sprintf(dport, "%hu", ntohs(strm->hash_flds.ipv6.dport));
	}
}

static int
call_qcsapi_vsp_show_strms(call_qcsapi_bundle *call, int show_all)
{
	qcsapi_output *print = call->caller_output;
	unsigned int state[(sizeof(struct qcsapi_data_128bytes) / sizeof(unsigned int))];
	int rc;
	int i;
	struct qcsapi_data_4Kbytes *strms_buf = NULL;
	unsigned int strm_max = sizeof(*strms_buf) / sizeof(struct qvsp_strm_info);
	struct qvsp_strm_info *strms;
	qcsapi_wifi_mode wifi_mode = qcsapi_nosuch_mode;
	char *ip_proto;
	char saddr[INET6_ADDRSTRLEN + 1];
	char daddr[INET6_ADDRSTRLEN + 1];
	char sport[QCSAPI_PORT_STRLEN + 1];
	char dport[QCSAPI_PORT_STRLEN + 1];
	int max_saddr_strlen = NIPQUAD_LEN;
	int max_daddr_strlen = NIPQUAD_LEN;
	char node_idx_buf[5];
	char *sta_str;
	uint32_t bytes;
	uint32_t pkts;

	strms_buf = (struct qcsapi_data_4Kbytes *)malloc(sizeof(struct qcsapi_data_4Kbytes));
	if (strms_buf == NULL)
		return -ENOMEM;
	strms = (struct qvsp_strm_info *)strms_buf->data;

	qcsapi_wifi_get_mode(call->caller_interface, &wifi_mode);

	rc = qcsapi_qtm_get_state_all(call->caller_interface, (void *)&state, QVSP_STATE_READ_MAX);
	if (rc < 0) {
		goto out;
	}

	rc = qcsapi_qtm_get_strm(call->caller_interface, strms_buf, strm_max, show_all);
	if (rc < 0) {
		goto out;
	}

	print_out(print, "Free airtime:        %u\n", state[QVSP_STATE_FAT]);
	print_out(print, "Interference:        %u\n", state[QVSP_STATE_FAT_INTF]);
	print_out(print, "Streams:\n");
	print_out(print, "    Total:           %u\n", state[QVSP_STATE_STRM_TOT]);
	print_out(print, "    QTM peers:       %u\n", state[QVSP_STATE_STRM_QTN]);
	print_out(print, "    Enabled:         %u\n", state[QVSP_STATE_STRM_ENA]);
	print_out(print, "    Disabled:        %u\n", state[QVSP_STATE_STRM_DIS]);
	print_out(print, "    Demoted:         %u\n", state[QVSP_STATE_STRM_DMT]);

	/* find max length of IP addresses */
	for (i = 0; i < rc; i++) {
		const struct qvsp_strm_info *strm = &strms[i];
		int saddr_strlen;
		int daddr_strlen;

		if (show_all == 0 && strm->strm_state == QVSP_STRM_STATE_LOW_TPUT) {
			continue;
		}

		call_qcsapi_vsp_strm_fmt(strm, saddr, daddr, sport, dport);
		saddr_strlen = strlen(saddr);
		daddr_strlen = strlen(daddr);
		max_saddr_strlen = max(max_saddr_strlen, saddr_strlen);
		max_daddr_strlen = max(max_daddr_strlen, daddr_strlen);
	}

	print_out(print, "Hash Dir Sta ");
	if (g_is_qtm) {
		print_out(print, "%-17s ", "Station");
		print_out(print, "NdIdx TID VapPri AC ");
	} else {
		print_out(print, "%-*s %-*s %-*s %-*s ",
			max_saddr_strlen, "Source IP", QCSAPI_PORT_STRLEN, "SPort",
			max_daddr_strlen, "Dest IP", QCSAPI_PORT_STRLEN, "DPort");
		print_out(print, "NdIdx Prot AC ");
	}
	print_out(print, "pps    TotPps Kbps    MaxKbps Rate NdCst Cost  Max   Age Status Dmt ThrKbps\n");

	for (i = 0; i < rc; i++) {
		const struct qvsp_strm_info *strm = &strms[i];

		if (show_all == 0 && strm->strm_state == QVSP_STRM_STATE_LOW_TPUT) {
			continue;
		}

		call_qcsapi_vsp_strm_fmt(strm, saddr, daddr, sport, dport);

		switch (strm->ip_proto) {
		case IPPROTO_TCP:
			ip_proto = "TCP";
			break;
		case IPPROTO_UDP:
			ip_proto = "UDP";
			break;
		default:
			ip_proto = "?";
			break;
		}

		if (wifi_mode == qcsapi_station) {
			sta_str = "-";
		} else if (strm->is_3rdpt_udp_us) {
			sta_str = QVSP_3RDPT_STR;
		} else {
			sta_str = strm->disable_remote ? "y" : "n";
		}

		/* pkts and bytes in the air */
		if (strm->is_3rdpt_udp_us) {
			pkts = strm->prev_stats.pkts;
			bytes = strm->prev_stats.bytes;
		} else {
			pkts = strm->prev_stats.pkts_sent;
			bytes = strm->prev_stats.bytes_sent;
		}

		print_out(print, "%03u  %-3s %-3s ",
				strm->hash,
				call_qcsapi_vsp_dir_desc(strm->dir),
				sta_str);

		if (g_is_qtm) {
			print_out(print, MACSTR" ", MAC2STR(strm->node_mac));
		} else {
			print_out(print, "%-*s %-6s %-*s %-6s ",
				max_saddr_strlen, saddr, sport,
				max_daddr_strlen, daddr, dport);
		}

		switch (strm->hairpin_type) {
		case QVSP_HAIRPIN_NONE:
			snprintf(node_idx_buf, sizeof(node_idx_buf) - 1, "%u",
				strm->node_idx);
			break;
		case QVSP_HAIRPIN_UCAST:
			snprintf(node_idx_buf, sizeof(node_idx_buf) - 1, "%u-%u",
				strm->node_idx, strm->hairpin_id);
			break;
		case QVSP_HAIRPIN_MCAST:
			snprintf(node_idx_buf, sizeof(node_idx_buf) - 1, "%u-%u",
				strm->hairpin_id, strm->node_idx);
		}
		print_out(print, "%-5s ", node_idx_buf);

		if (g_is_qtm)
			print_out(print, "%-3u %-6u ", strm->tid, (uint32_t)strm->vap_pri);
		else
			print_out(print, "%-4s ", ip_proto);
		print_out(print, "%u  %-6u %-6u %-7u %-7u %-4u %-5u %-5u %-5u %-3lu %-6s %u-%u [%s]%-7u\n",
				strm->ac_in,
				pkts, strm->prev_stats.pkts,
				qvsp_b2kbit(bytes),
				qvsp_b2kbit(strm->bytes_max),
				(strm->ni_inv_phy_rate == 0) ?
					0 : qvsp_inv2phy(strm->ni_inv_phy_rate),
				strm->ni_cost,
				strm->cost_current,
				strm->cost_max,
				strm->last_ref_secs,
				call_qcsapi_qvsp_state_desc(strm->strm_state),
				strm->demote_rule,
				strm->demote_state,
				qvsp_strm_throt_desc_abbr[strm->throt_policy],
				strm->throt_rate);
	}

	if (rc >= strm_max) {
		print_out(print, ".... [list truncated]\n");
	}

	rc = 0;
out:
	if (strms_buf != NULL)
		free(strms_buf);
	return rc;
}

static int
call_qcsapi_vsp_show_all(call_qcsapi_bundle *call, int argc, char *argv[])
{
	return call_qcsapi_vsp_show_strms(call, 1);
}

static const char *
call_qcsapi_vsp_if_desc(enum qvsp_if_e vsp_if)
{
	if (vsp_if < ARRAY_SIZE(qvsp_if_desc)) {
		return qvsp_if_desc[vsp_if];
	}
	return "invalid";
}

static int
call_qcsapi_vsp_show_stats(call_qcsapi_bundle *call, int argc, char *argv[])
{
	qcsapi_output *print = call->caller_output;
	struct qvsp_stats stats;
	enum qvsp_if_e vsp_if;
	int rc;

	rc = qcsapi_qtm_get_stats(call->caller_interface, (void *)&stats);
	if (rc < 0) {
		return rc;
	}

	print_out(print, "Free airtime\n");
	print_out(print, "  Oversubscription:         %u\n", stats.fat_over);
	print_out(print, "  Streams disabled:         %u\n", stats.fat_chk_disable);
	print_out(print, "  Undersubscription:        %u\n", stats.fat_under);
	print_out(print, "  Stream re-enabled:        %u\n", stats.fat_chk_reenable);
	print_out(print, "Streams\n");
	print_out(print, "  Enabled:                  %u\n", stats.strm_enable);
	print_out(print, "  Disabled:                 %u\n", stats.strm_disable);
	print_out(print, "  Re-enabled:               %u\n", stats.strm_reenable);
	print_out(print, "\n");
	print_out(print, "  Interface   Added  Not Fnd\n");
	for (vsp_if = 0; vsp_if < QVSP_IF_MAX; vsp_if++) {
		print_out(print, "  %-8s %8u %8u\n",
			call_qcsapi_vsp_if_desc(vsp_if),
			stats.stats_if[vsp_if].strm_add,
			stats.stats_if[vsp_if].strm_none);
	}
	print_out(print, "Packets\n");
	print_out(print, "  Interface   Checked        UDP        TCP      Other     "
			"Ignore       Sent  Throttled   Disabled Frag Found  Not Found    Demoted\n");
	for (vsp_if = 0; vsp_if < QVSP_IF_MAX; vsp_if++) {
		print_out(print,
			"  %-8s %10u %10u %10u %10u %10u %10u %10u %10u %10u %10u %10u\n",
			call_qcsapi_vsp_if_desc(vsp_if),
			stats.stats_if[vsp_if].pkt_chk,
			stats.stats_if[vsp_if].pkt_udp,
			stats.stats_if[vsp_if].pkt_tcp,
			stats.stats_if[vsp_if].pkt_other,
			stats.stats_if[vsp_if].pkt_ignore,
			stats.stats_if[vsp_if].pkt_sent,
			stats.stats_if[vsp_if].pkt_drop_throttle,
			stats.stats_if[vsp_if].pkt_drop_disabled,
			stats.stats_if[vsp_if].pkt_frag_found,
			stats.stats_if[vsp_if].pkt_frag_not_found,
			stats.stats_if[vsp_if].pkt_demoted);
	}

	return 0;
}

static int
call_qcsapi_vsp_show(call_qcsapi_bundle *call, int argc, char *argv[])
{
	qcsapi_output *print = call->caller_output;
	call_qcsapi_vsp_fn func;
	int rc;

	static const struct call_qcsapi_fnmap mux[] = {
		{ "all",	call_qcsapi_vsp_show_all },
		{ "config",	call_qcsapi_vsp_show_config },
		{ "stats",	call_qcsapi_vsp_show_stats },
	};

	rc = call_qcsapi_vsp_is_enabled(call);
	if (rc < 0) {
		return rc;
	}

	if ((argv[0] == NULL) || (strcmp(argv[0], "config") != 0)) {
		rc = call_qcsapi_vsp_is_active(call);
		if (rc < 0) {
			return rc;
		}
	}

	if (argc == 0) {
		return call_qcsapi_vsp_show_strms(call, 0);
	}

	func = call_qcsapi_fnmap_find(argv[0], mux, ARRAY_SIZE(mux));
	if (func == NULL) {
		call_qcsapi_vsp_show_usage(print);
		return -EINVAL;
	} else {
		return func(call, argc - 1, argv + 1);
	}
}

static void
call_qcsapi_vsp_test_usage(qcsapi_output *print)
{
	print_out(print, "Usage:\n"
			"    <qcsapi> qtm <if> test fat <fat>\n");
}


static int
call_qcsapi_vsp_reset(call_qcsapi_bundle *call, int argc, char *argv[])
{
	int rc;

	rc = call_qcsapi_vsp_is_enabled(call);
	if (rc < 0) {
		return rc;
	}

	return qcsapi_qtm_set_state(call->caller_interface, QVSP_STATE_RESET, 0);
}

static int
call_qcsapi_vsp_test_fat(call_qcsapi_bundle *call, int argc, char *argv[])
{
	qcsapi_output *print = call->caller_output;
	unsigned int val;
	int rc;

	if (argc != 1) {
		goto err;
	}

	rc = sscanf(argv[0], "%u", &val);
	if (rc != 1) {
		print_err(print, "QTM: error parsing '%s'\n", argv[0]);
		goto err;
	}

	rc = call_qcsapi_vsp_is_enabled(call);
	if (rc < 0) {
		return rc;
	}

	return qcsapi_qtm_set_state(call->caller_interface, QVSP_STATE_TEST_FAT, val);

err:
	call_qcsapi_vsp_test_usage(print);
	return -EINVAL;
}

static int
call_qcsapi_vsp_test(call_qcsapi_bundle *call, int argc, char *argv[])
{
	qcsapi_output *print = call->caller_output;
	call_qcsapi_vsp_fn func;
	int rc;

	static const struct call_qcsapi_fnmap mux[] = {
		{ "fat",	call_qcsapi_vsp_test_fat },
	};

	if (argc < 1) {
		call_qcsapi_vsp_test_usage(print);
		return -EINVAL;
	}

	rc = call_qcsapi_vsp_is_enabled(call);
	if (rc < 0) {
		return rc;
	}

	func = call_qcsapi_fnmap_find(argv[0], mux, ARRAY_SIZE(mux));
	if (func == NULL) {
		call_qcsapi_vsp_test_usage(print);
		return -EINVAL;
	} else {
		return func(call, argc - 1, argv + 1);
	}
}

static void call_qcsapi_vsp_init(void)
{
	int i;

	if (!qvsp_show_cfg_initialised) {
		qvsp_show_cfg_initialised = 1;

		for (i = 0; i < QVSP_CFG_MAX; i++) {
			if (strlen(qvsp_cfg_params[i].name) > qvsp_cfg_name_len) {
				qvsp_cfg_name_len = strlen(qvsp_cfg_params[i].name);
			}
			if (strlen(qvsp_cfg_params[i].units) > qvsp_cfg_units_len) {
				qvsp_cfg_units_len = strlen(qvsp_cfg_params[i].units);
			}
		}

		for (i = 0; i < QVSP_RULE_PARAM_MAX; i++) {
			if (strlen(qvsp_rule_params[i].name) > qvsp_rule_name_len) {
				qvsp_rule_name_len = strlen(qvsp_rule_params[i].name);
			}
			if (strlen(qvsp_rule_params[i].units) > qvsp_rule_units_len) {
				qvsp_rule_units_len = strlen(qvsp_rule_params[i].units);
			}
		}
	}
}

static void
call_qcsapi_vsp_usage(qcsapi_output *print)
{
	if (!g_is_qtm) {
		print_out(print, "Usage:\n"
			"    <qcsapi> qtm <if> show [config | all]\n"
			"    <qcsapi> qtm <if> reset\n"
			"    <qcsapi> qtm <if> set <param> <val>\n"
			"    <qcsapi> qtm <if> get <param>\n"
			"    <qcsapi> qtm <if> rule [add | del] <arg> [<arg> ...]\n"
			"    <qcsapi> qtm <if> wl [add | del] <saddr> <daddr> <sport> <dport>\n"
			"    <qcsapi> qtm <if> test <args>\n");
	} else {
		print_out(print, "Usage:\n"
			"    <qcsapi> qtm <if> show [config | all]\n"
			"    <qcsapi> qtm <if> reset\n"
			"    <qcsapi> qtm <if> set <param> <val>\n"
			"    <qcsapi> qtm <if> get <param>\n"
			"    <qcsapi> qtm <if> rule [add | del] <arg> [<arg> ...]\n"
			"    <qcsapi> qtm <if> test <args>\n");
	}
}

static int
call_qcsapi_vsp(call_qcsapi_bundle *call, int argc, char *argv[])
{
	qcsapi_wifi_mode wifi_mode = qcsapi_nosuch_mode;
	qcsapi_output *print = call->caller_output;
	call_qcsapi_vsp_fn func;
	int rc;
	int statval = 0;

	static const struct call_qcsapi_fnmap mux[] = {
		{ "set",	call_qcsapi_vsp_set },
		{ "get",	call_qcsapi_vsp_get },
		{ "rule",	call_qcsapi_vsp_rule },
		{ "wl",		call_qcsapi_vsp_wl },
		{ "show",	call_qcsapi_vsp_show },
		{ "reset",	call_qcsapi_vsp_reset },
		{ "test",	call_qcsapi_vsp_test },
	};

	call_qcsapi_vsp_init();

	if (argc < 1) {
		call_qcsapi_vsp_usage(print);
		return -EINVAL;
	}

	rc = qvsp_is_qtm();
	if (rc < 0)
		return rc;
	g_is_qtm = rc;

	func = call_qcsapi_fnmap_find(argv[0], mux, ARRAY_SIZE(mux));
	if (func == NULL) {
		call_qcsapi_vsp_usage(print);
		return -EINVAL;
	}

	qcsapi_wifi_get_mode(call->caller_interface, &wifi_mode);
	if (wifi_mode == qcsapi_station) {
		if (func == call_qcsapi_vsp_set ||
			func == call_qcsapi_vsp_rule
			|| func == call_qcsapi_vsp_wl
			) {

			print_err(print, "QTM: %s command cannot be used on stations\n",
				argv[0]);
			return -EINVAL;
		}
	}

	rc = func(call, argc - 1, argv + 1);

	if (rc < 0) {
		report_qcsapi_error( call, rc );
		statval = 1;
	}

	return statval;
}

static int
verify_script_name(const char *script)
{
	int index;
	int number = TABLE_SIZE( support_script_table );
	for (index = 0; index < number; index++) {
		if (strcmp(script, support_script_table[index]) == 0)
			return 0;
	}

	return -1;
}

static int
call_qcsapi_run_script(call_qcsapi_bundle *call, int argc, char *argv[])
{
	int	statval = 0;
	int i = 0;
	char *scriptname = NULL;
	char param[QCSAPI_CMD_BUFSIZE], *param_p;
	int len = 0;
	int space = sizeof(param) - 1;
	int qcsapi_retval;
	qcsapi_output *print = call->caller_output;
	param_p = param;

	if (argc == 0) {
		print_err(print, "Not enough parameters\n");
		return 1;
	}

	scriptname = argv[0];
	statval = verify_script_name(scriptname);
	if (statval < 0) {
		print_err(print, "Script %s is not supported\n", scriptname);
		return 1;
	}

	for (i = 1; i < argc; i++) {
		if (strlen(argv[i]) + 1 < space) {
			len = sprintf(param_p , "%s ", argv[i]);
			param_p += len;
			space -= len;
		} else {
			print_err(print, "Parameter string is too long\n");
			return 1;
		}
	}

	*param_p = '\0';
	qcsapi_retval = qcsapi_wifi_run_script(scriptname, param);
	if (qcsapi_retval < 0) {
		report_qcsapi_error(call, qcsapi_retval);
		statval = 1;
	}

#ifdef BUILDIN_TARGET_BOARD
{
	char strbuf[QCSAPI_MSG_BUFSIZE] = {0};
	FILE *log;
	/* output the script message */
	log = fopen(QCSAPI_SCRIPT_LOG, "r");
	if (log != NULL) {
		while (fgets(strbuf, sizeof(strbuf), log))
			print_out(print, "%s", strbuf);
		fclose(log);
	} else {
		print_err(print, "Failed to open file %s\n", QCSAPI_SCRIPT_LOG);
		return 1;
	}
}
#endif

	return statval;
}

#define QCSAPI_TEMP_INVALID     (-274 * QDRV_TEMPSENS_COEFF10)

static int
call_qcsapi_get_temperature(call_qcsapi_bundle *call, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval;
	int temp_rfic_external = QCSAPI_TEMP_INVALID;
	int temp_rfic_internal = QCSAPI_TEMP_INVALID;
	int temp_bbic_internal = QCSAPI_TEMP_INVALID;

	qcsapi_output *print = call->caller_output;

	qcsapi_retval = qcsapi_get_temperature_info(&temp_rfic_external, &temp_rfic_internal,
			&temp_bbic_internal);

	if (qcsapi_retval >= 0) {
		if (temp_rfic_external != QCSAPI_TEMP_INVALID) {
			print_out(print, "temperature_rfic_external = %3.1f\n",
				  (float)temp_rfic_external / QDRV_TEMPSENS_COEFF);
		}
		if (temp_rfic_internal != QCSAPI_TEMP_INVALID) {
			print_out(print, "temperature_rfic_internal = %3.1f\n",
				  (float)temp_rfic_internal / QDRV_TEMPSENS_COEFF10);
		}
		if (temp_bbic_internal != QCSAPI_TEMP_INVALID) {
			print_out(print, "temperature_bbic_internal = %3.1f\n",
				  (float)temp_bbic_internal / QDRV_TEMPSENS_COEFF10);
		}
	} else {
		report_qcsapi_error(call, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_set_accept_oui_filter(call_qcsapi_bundle *call, int argc, char *argv[])
{
	int	statval = 0;
	qcsapi_output *print = call->caller_output;

	if (argc < 2)
	{
		print_err(print, "Not enough parameters\n");
		statval = 1;
	}
	else
	{
		const char *the_interface = call->caller_interface;
		qcsapi_mac_addr the_mac_addr;
		qcsapi_mac_addr oui = {0};
		int qcsapi_retval;
		int ival = 0;
		int action;

		action = atoi(argv[1]);
		ival = parse_mac_addr(argv[0], the_mac_addr);

		if (ival >= 0) {
			memcpy(oui, the_mac_addr, 3);
			qcsapi_retval = qcsapi_wifi_set_accept_oui_filter(the_interface, oui, action);
			if (qcsapi_retval >= 0) {
				if (verbose_flag >= 0)
					print_out(print, "complete\n");

			} else {
				report_qcsapi_error(call, qcsapi_retval);
				statval = 1;
			}

		} else {
			print_out(print, "Error parsing MAC address %s\n", argv[0]);
			statval = 1;
		}
	}

	return statval;
}

static int
call_qcsapi_get_accept_oui_filter(call_qcsapi_bundle *call, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval;
	const char *the_interface = call->caller_interface;
	qcsapi_output *print = call->caller_output;
	char *oui_list = NULL;
	unsigned int sizeof_oui_list = 126;

	if (argc > 0) {
		sizeof_oui_list = (unsigned int)atoi(argv[0]);
	}

	if (sizeof_oui_list > 0) {
		oui_list = malloc(sizeof_oui_list);
		if (oui_list == NULL) {
			print_err( print, "Failed to allocate %u chars\n", sizeof_oui_list);
			return 1;
		}
	}

	qcsapi_retval = qcsapi_wifi_get_accept_oui_filter(the_interface, oui_list, sizeof_oui_list);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0)
			print_out(print, "%s\n", oui_list);
	} else {
		report_qcsapi_error(call, qcsapi_retval);
		statval = 1;
	}

	if (oui_list != NULL)
		free(oui_list);

	return statval;
}

static int
call_qcsapi_wifi_set_vht(call_qcsapi_bundle *call, int argc, char *argv[])
{
	int rc = 0;
	qcsapi_output *print = call->caller_output;

	if (argc < 1) {
		print_err( print, "Not enough parameters in call qcsapi wifi set_vht, count is %d\n", argc );
		print_err( print, "Usage: call_qcsapi set_vht <WiFi interface> <0 | 1>\n" );
		rc = 1;
	} else {
		qcsapi_unsigned_int	vht_status = (qcsapi_unsigned_int) atoi( argv[0] );
		const char		*the_interface = call->caller_interface;
		int			qcsapi_retval;

		qcsapi_retval = qcsapi_wifi_set_vht( the_interface, vht_status );
		if (qcsapi_retval >= 0) {
			if (verbose_flag >= 0) {
				print_out( print, "complete\n");
			}
		} else {
			report_qcsapi_error( call, qcsapi_retval );
			rc = 1;
		}
	}
	return rc;
}

static int
call_qcsapi_get_swfeat_list(call_qcsapi_bundle *call, int argc, char *argv[])
{
	int qcsapi_retval;
	qcsapi_output *print = call->caller_output;
	string_4096 buf;

	qcsapi_retval = qcsapi_get_swfeat_list(buf);
	if (qcsapi_retval < 0) {
		report_qcsapi_error(call, qcsapi_retval);
		return 1;
	}

	print_out(print, "%s\n", buf);

	return 0;
}

/*
 * Pass-in epoch time (UTC secs) to convert to readable date string
 */
static void
local_qcsapi_timestr(char *const buf, const size_t bufsize, const uint32_t utc_time_secs)
{
	const time_t epoch_seconds = utc_time_secs;
	struct tm tm_parsed;

	gmtime_r(&epoch_seconds, &tm_parsed);

	strftime(buf, bufsize, "%d %B %Y %H:%M:%S", &tm_parsed);
}

static char *uboot_type_to_str(char type)
{
	char *ptr;

	switch (type - '0') {
	case UBOOT_INFO_LARGE:
		ptr = "Large";
		break;
	case UBOOT_INFO_MINI:
		ptr = "Mini";
		break;
	case UBOOT_INFO_TINY:
		ptr = "Tiny";
		break;
	default:
		ptr = "Unknown";
	}

	return ptr;
}

/*
 * Primary userspace call_qcsapi handler to get u-boot information
 */
static int
call_qcsapi_get_uboot_info(call_qcsapi_bundle *call, int argc, char *argv[])
{
	qcsapi_output *print = call->caller_output;
	struct early_flash_config ef_config;
	string_32 version_str;
	string_32 built_str = {0};
	uint32_t u_boot_time;
	int qcsapi_retval;
	int uboot_info;

	if (argc < 1) {
		print_err(print, "Not enough parameters in call_qcsapi get_uboot_info, "
					"count is %d\n", argc);
		print_err(print, "Usage: call_qcsapi get_uboot_info <info> : "
					"0 - ver, 1 - built, 2 - type, 3 - all\n");
		return -1;
	}

	qcsapi_retval = qcsapi_get_uboot_info(version_str, &ef_config);
	if (qcsapi_retval) {
		print_err(print, "Call to qcsapi_get_uboot_info failed qcsapi_retval=%d\n",
				qcsapi_retval);
		return -1;
	}

	errno = 0;
	u_boot_time = strtol((char *)ef_config.built_time_utc_sec, NULL, 10);
	if (errno) {
		print_err(print, "strtol(%s) failed, errno=-%d\n",
				(char *)ef_config.built_time_utc_sec, errno);
		return -errno;
	}

	/* Convert UTC seconds to readable date string */
	local_qcsapi_timestr(built_str, sizeof(built_str) - 1, u_boot_time);

	uboot_info = atoi(argv[0]);
	switch (uboot_info) {
	case UBOOT_INFO_VER:
		print_out(print, "Version: %s\n", version_str);
		break;
	case UBOOT_INFO_BUILT:
		print_out(print, "Built: %s\n", built_str);
		break;
	default:
	case UBOOT_INFO_TYPE:
	case UBOOT_INFO_ALL:
		if (uboot_info == UBOOT_INFO_ALL) {
			print_out(print, "Version: %s\nBuilt  : %s\n", version_str, built_str);
		}
		print_out(print, "Type   : U-boot (%s)\n",
				uboot_type_to_str(ef_config.uboot_type));
		break;
	}
	return 0;
}

static int
call_qcsapi_wifi_get_vht(call_qcsapi_bundle *call, int argc, char *argv[])
{
	int rc = 0;
	int qcsapi_retval;

	qcsapi_unsigned_int	vht_status;
	qcsapi_output *print = call->caller_output;
	const char *the_interface = call->caller_interface;

	qcsapi_retval = qcsapi_wifi_get_vht( the_interface, &vht_status);
	if (qcsapi_retval >= 0) {
		print_out(print, "%d\n", vht_status);
	} else {
		report_qcsapi_error(call, qcsapi_retval);
		rc = 1;
	}

	return rc;
}

static int
call_qcsapi_calcmd_set_test_mode(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int qcsapi_retval;
	qcsapi_output *print = p_calling_bundle->caller_output;
	int channel;
	int antenna ;
	int mcs;
	int bw;
	int pkt_size;
	int eleven_n;
	int primary_chan;
	if (argc < 7) {
		print_err( print, "Format: set_test_mode calcmd <channel> <antenna> <mcs> <bw> <packet size> <11n> <bf>\n");
		print_err( print, "Example: set_test_mode calcmd 36 127 7 40 40 1 1\n");
		return(1);
	}

	channel = atoi(argv[0]);
	antenna = atoi(argv[1]);
	mcs = atoi(argv[2]);
	bw = atoi(argv[3]);
	pkt_size = atoi(argv[4]);
	eleven_n = atoi(argv[5]);
	primary_chan = atoi(argv[6]);

	qcsapi_retval = qcsapi_calcmd_set_test_mode(channel, antenna, mcs, bw, pkt_size, eleven_n, primary_chan);
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "Complete.\n");
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		return 1;
	}

	return 0;
}


static int
call_qcsapi_calcmd_show_test_packet(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int qcsapi_retval;
	uint32_t txnum;
	uint32_t rxnum;
	uint32_t crc;

	qcsapi_output *print = p_calling_bundle->caller_output;

	qcsapi_retval = qcsapi_calcmd_show_test_packet(&txnum, &rxnum, &crc);
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out(print, "tx_pkts# = \t%d\nrx_pkts# = \t%d\nCRC_err# = \t%d\n", txnum, rxnum, crc);
			print_out(print, "Complete.\n");
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		return 1;
	}

	return 0;
}


static int
call_qcsapi_calcmd_send_test_packet(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int qcsapi_retval;
	int packet_num;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1) {
		print_err( print, "Format: send_test_packet calcmd <pkt_num>\n");
		print_err( print, "Example: send_test_packet calcmd 0\n");
		return(1);
	}

	packet_num = atoi(argv[0]);

	qcsapi_retval = qcsapi_calcmd_send_test_packet(packet_num);
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out(print, "Complete.\n");
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		return 1;
	}

	return qcsapi_retval;
}

static int
call_qcsapi_calcmd_stop_test_packet(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int qcsapi_retval;
	qcsapi_output *print = p_calling_bundle->caller_output;

	qcsapi_retval = qcsapi_calcmd_stop_test_packet();
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out(print, "Complete.\n");
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		return 1;
	}

	return qcsapi_retval;
}

static int
call_qcsapi_calcmd_send_dc_cw_signal(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int qcsapi_retval;
	qcsapi_unsigned_int channel;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1) {
		print_err( print, "Format: send_dc_cw_signal calcmd <channel>\n");
		print_err( print, "Example: send_dc_cw_signal calcmd 36\n");
		return(1);
	}

	channel = atoi(argv[0]);
	qcsapi_retval = qcsapi_calcmd_send_dc_cw_signal(channel);
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out(print, "Complete.\n");
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		return 1;
	}

	return qcsapi_retval;
}

static int
call_qcsapi_calcmd_stop_dc_cw_signal(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int qcsapi_retval;
	qcsapi_output *print = p_calling_bundle->caller_output;

	qcsapi_retval = qcsapi_calcmd_stop_dc_cw_signal();
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out(print, "Complete.\n");
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		return 1;
	}

	return qcsapi_retval;
}

static int
call_qcsapi_calcmd_get_test_mode_antenna_sel(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int qcsapi_retval;
	qcsapi_unsigned_int antenna;
	qcsapi_output *print = p_calling_bundle->caller_output;

	qcsapi_retval = qcsapi_calcmd_get_test_mode_antenna_sel(&antenna);
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out(print, "%d\n", antenna);
			print_out(print, "Complete.\n");
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		return 1;
	}

	return qcsapi_retval;
}

static int
call_qcsapi_calcmd_get_test_mode_mcs(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int qcsapi_retval;
	qcsapi_unsigned_int mcs;
	qcsapi_output *print = p_calling_bundle->caller_output;

	qcsapi_retval = qcsapi_calcmd_get_test_mode_mcs(&mcs);
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out(print, "%d\n", mcs);
			print_out(print, "Complete.\n");
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		return 1;
	}

	return qcsapi_retval;
}

static int
call_qcsapi_calcmd_get_test_mode_bw(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int qcsapi_retval;
	qcsapi_unsigned_int bw;
	qcsapi_output *print = p_calling_bundle->caller_output;

	qcsapi_retval = qcsapi_calcmd_get_test_mode_bw(&bw);
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out(print, "%d\n", bw);
			print_out(print, "Complete.\n");
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		return 1;
	}

	return qcsapi_retval;
}

static int
call_qcsapi_calcmd_get_tx_power(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int qcsapi_retval;
	qcsapi_calcmd_tx_power_rsp tx_power;
	qcsapi_output *print = p_calling_bundle->caller_output;

	qcsapi_retval = qcsapi_calcmd_get_tx_power(&tx_power);
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out(print, "%d.%ddBm %d.%ddBm %d.%ddBm %d.%ddBm\n",
				tx_power.value[0]>>2,(tx_power.value[0]&3)*25,
				tx_power.value[1]>>2,(tx_power.value[1]&3)*25,
				tx_power.value[2]>>2,(tx_power.value[2]&3)*25,
				tx_power.value[3]>>2,(tx_power.value[3]&3)*25);
			print_out(print, "Complete.\n");
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		return 1;
	}

	return qcsapi_retval;
}

static int
call_qcsapi_calcmd_set_tx_power(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int qcsapi_retval;
	qcsapi_unsigned_int tx_power;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1) {
		print_err( print, "Format: set_test_mode_tx_power calcmd <tx_power>\n");
		print_err( print, "Example: set_test_mode_tx_power calcmd 19\n");
		return(1);
	}

	tx_power = atoi(argv[0]);
	qcsapi_retval = qcsapi_calcmd_set_tx_power(tx_power);
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out(print, "Complete.\n");
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		return 1;
	}

	return qcsapi_retval;
}


static int
call_qcsapi_calcmd_get_test_mode_rssi(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int qcsapi_retval;
	qcsapi_calcmd_rssi_rsp rssi;
	qcsapi_output *print = p_calling_bundle->caller_output;

	qcsapi_retval = qcsapi_calcmd_get_test_mode_rssi(&rssi);
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out(print, "%d.%d %d.%d %d.%d %d.%d\n",
				rssi.value[0]/10, rssi.value[0]%10,
				rssi.value[1]/10, rssi.value[1]%10,
				rssi.value[2]/10, rssi.value[2]%10,
				rssi.value[3]/10, rssi.value[3]%10);
			print_out(print, "Complete.\n");
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		return 1;
	}

	return qcsapi_retval;
}

static int
call_qcsapi_calcmd_set_mac_filter(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int qcsapi_retval = 0;
	int sec_enable;
	int q_num;
	qcsapi_mac_addr mac_addr;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc != 3) {
		print_out(print, "Parameter input error! \n");
		print_out(print, "Format:\n");
		print_out(print, "call_qcsapi set_mac_filter wifi0 #q_num #sec_enable #mac_addr \n");
		print_out(print, "Example: call_qcsapi set_mac_filter wifi0 1 2 00:11:22:33:44:55\n");

		return qcsapi_retval;
	}

	q_num = atoi(argv[0]);
	sec_enable = atoi(argv[1]);
	qcsapi_retval = parse_mac_addr(argv[2], mac_addr);

	if (qcsapi_retval >= 0) {
		qcsapi_retval = qcsapi_calcmd_set_mac_filter(q_num, sec_enable, mac_addr);
	}

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out(print, "Complete.\n");
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		qcsapi_retval = 1;
	}

	return qcsapi_retval;
}

static int
call_qcsapi_calcmd_get_antenna_count(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int qcsapi_retval;
	qcsapi_unsigned_int antenna_count;
	qcsapi_output *print = p_calling_bundle->caller_output;

	qcsapi_retval = qcsapi_calcmd_get_antenna_count(&antenna_count);
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out(print, "%d\n", antenna_count);
			print_out(print, "Complete.\n");
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		return 1;
	}

	return qcsapi_retval;
}

static int
call_qcsapi_calcmd_clear_counter(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int qcsapi_retval;
	qcsapi_output *print = p_calling_bundle->caller_output;

	qcsapi_retval = qcsapi_calcmd_clear_counter();
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out(print, "Complete.\n");
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		return 1;
	}

	return qcsapi_retval;
}

static int
call_qcsapi_calcmd_get_info(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int qcsapi_retval;
	qcsapi_output *print = p_calling_bundle->caller_output;
	string_1024 output_info;

	qcsapi_retval = qcsapi_calcmd_get_info(output_info);
	if (qcsapi_retval >= 0) {
		print_out(print, "%s", output_info);
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		return 1;
	}

	return qcsapi_retval;
}


int
call_qcsapi_disable_dfs_channels(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int qcsapi_retval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;
	const char *the_interface = p_calling_bundle->caller_interface;
	int new_channel = 0;

	if (argc < 1) {
		print_err(print, "usage:\ncall_qcsapi disable_dfs_channels <0|1> [new channel]\n");
		return 1;
	} else if (argc > 1) {
		new_channel = atoi(argv[1]);
	}

	qcsapi_retval = qcsapi_wifi_disable_dfs_channels(the_interface, atoi(argv[0]), new_channel);
	if (qcsapi_retval >= 0)
	{
		if (verbose_flag >= 0)
		{
			print_out( print, "complete\n" );
		}
	}
	else
	{
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
	}

	return qcsapi_retval;
}

static int
call_qcsapi_wifi_set_soc_macaddr(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int	statval = 0;
	qcsapi_mac_addr new_mac_addr;
	int		qcsapi_retval = 0;;
	int		ival = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;
	const char	*the_interface = p_calling_bundle->caller_interface;

	if (argc < 1)
	{
		print_err( print, "Not enough parameters in call qcsapi file path, count is %d\n", argc );
		statval = 1;
	}
	else
	{
		if (strcmp( "NULL", argv[ 0 ] ) == 0)
		{
			print_out( print, "Mac addr is NULL \n");
			statval = 1;
		}
		else
		{
			ival = parse_mac_addr( argv[ 0 ], new_mac_addr );
			if (ival >= 0)
			  qcsapi_retval = qcsapi_set_soc_mac_addr( the_interface, new_mac_addr );
			else
			{
				print_out( print, "Error parsing MAC address %s\n", argv[ 0 ] );
				statval = 1;
			}
		}

		if (ival >= 0)
		{
			if (qcsapi_retval >= 0)
			{
				if (verbose_flag >= 0)
				{
					print_out( print, "complete\n" );
				}
			}
			else
			{
				report_qcsapi_error( p_calling_bundle, qcsapi_retval );
				statval = 1;
			}
		}
	}

	return statval;

}

static int
call_qcsapi_wifi_enable_tdls(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int	statval = 0;
	int	qcsapi_retval = 0;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;
	uint32_t enable_tdls = 1;

	if (argc > 0) {
		/*type conversion and parameter value check*/
		if (0 == safe_atou32(argv[0], &enable_tdls, print, 0, 1))
			return 1;
	}

	qcsapi_retval = qcsapi_wifi_enable_tdls(the_interface, enable_tdls);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0)
			print_out(print, "complete\n");
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wifi_enable_tdls_over_qhop(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int	statval = 0;
	int	qcsapi_retval = 0;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;
	uint32_t tdls_over_qhop_en = 0;

	if (argc > 0) {
		/*type conversion and parameter value check*/
		if (0 == safe_atou32(argv[0], &tdls_over_qhop_en, print, 0, 1))
			return 1;
	}

	qcsapi_retval = qcsapi_wifi_enable_tdls_over_qhop(the_interface, tdls_over_qhop_en);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0)
			print_out(print, "complete\n");
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int call_qcsapi_wifi_get_tdls_status(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	uint32_t tdls_status = 0;
	int32_t tdls_mode = 0;
	int32_t tdls_over_qhop_en = 0;
	int qcsapi_retval;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;
	qcsapi_tdls_type type = qcsapi_tdls_nosuch_param;

	qcsapi_retval = qcsapi_wifi_get_tdls_status(the_interface, &tdls_status);
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			if (tdls_status == 0)
				print_out(print, "tdls function: disabled\n");
			else
				print_out(print, "tdls function: enabled\n");
		}

		if(tdls_status != 0) {
			type = qcsapi_tdls_over_qhop_enabled;
			qcsapi_retval = qcsapi_wifi_get_tdls_params(the_interface, type, &tdls_over_qhop_en);

			if (qcsapi_retval >= 0) {
				if (verbose_flag >= 0)
					print_out(print, "tdls over qhop: %s\n", tdls_over_qhop_en ? "enabled" : "disabled");
			} else {
				report_qcsapi_error(p_calling_bundle, qcsapi_retval);
				statval = 1;
			}

			if (qcsapi_retval >= 0) {
				type = qcsapi_tdls_mode;
				qcsapi_retval = qcsapi_wifi_get_tdls_params(the_interface, type, &tdls_mode);

				if (qcsapi_retval >= 0) {
					if (verbose_flag >= 0)
						print_out(print, "tdls mode: %s\n", tdls_mode ? "forced" : "auto");
				} else {
					report_qcsapi_error(p_calling_bundle, qcsapi_retval);
					statval = 1;
				}
			}
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return (statval);
}

static int call_qcsapi_wifi_set_tdls_params(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval = 0;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;
	qcsapi_tdls_type type = p_calling_bundle->caller_generic_parameter.parameter_type.type_of_tdls;
	int value = 0;

	if (argc < 1) {
		print_err(print, "Not enough parameters, count is %d\n", argc);
		return 1;
	}

	value = atoi(argv[0]);

	qcsapi_retval = qcsapi_wifi_set_tdls_params(the_interface, type, value);
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0)
			print_out(print, "complete\n");
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int call_qcsapi_wifi_get_tdls_params(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	uint32_t tdls_status = 0;
	int qcsapi_retval;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;
	qcsapi_tdls_type type = 0;
	int value = 0;
	unsigned int iter;
	uint32_t param_num = 0;

	qcsapi_retval = qcsapi_wifi_get_tdls_status(the_interface, &tdls_status);
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0)
			print_out(print, "tdls function: %s\n", tdls_status ? "enabled" : "disabled");
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
		goto out;
	}

	param_num = TABLE_SIZE(qcsapi_tdls_param_table);
	for (iter = 0; iter < param_num; iter++) {
		type = qcsapi_tdls_param_table[iter].param_type;
		qcsapi_retval = qcsapi_wifi_get_tdls_params(the_interface, type, &value);

		if (qcsapi_retval >= 0) {
			if (verbose_flag >= 0) {
				if (type == qcsapi_tdls_over_qhop_enabled)
					print_out(print, "tdls over qhop: %s\n", value ? "enabled" : "disabled");
				else if (type == qcsapi_tdls_mode)
					print_out(print, "tdls mode: %s\n", value ? "forced" : "auto");
				else if((type >= qcsapi_tdls_min_rssi) && (type <= qcsapi_tdls_path_select_rate_thrshld))
					print_out(print, "\t%s: %d\n", qcsapi_tdls_param_table[iter].param_name, value);
				else
					print_out(print, "%s: %d\n", qcsapi_tdls_param_table[iter].param_name, value);
			}
		} else {
			report_qcsapi_error(p_calling_bundle, qcsapi_retval);
			statval = 1;
			goto out;
		}
	}
out:
	return (statval);
}

static int
call_qcsapi_get_carrier_id(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval = 0;
	qcsapi_unsigned_int carrier_id = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	qcsapi_retval = qcsapi_get_carrier_id(&carrier_id);

	if (qcsapi_retval >= 0) {
		print_out( print, "%d\n", carrier_id);
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_set_carrier_id(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;
	uint32_t carrier_id;
	uint32_t update_uboot = 0;

	if (argc < 1)
	{
		print_err( print, "Not enough parameters in call qcsapi set_carrier_id\n" );
		print_err( print, "Usage:  call_qcsapi set_carrier_id <carrier ID> <update uboot flag>\n");
		print_err( print, "        The second parameter is optional\n");
		print_err( print, "Example: call_qcsapi set_carrier_id 1\n");
		return 1;
	}

	if (isdigit(*argv[0])) {
		carrier_id = atoi(argv[0]);
	} else {
		print_err(print, "Unrecognized carrier id value %s\n", argv[0]);
		return 1;
	}

	/*
	 * The second parameter is optional and it indicates whether it is needed to update uboot.
	 * By default no update about uboot env. If the setting carrier ID is needed to write back into uboot
	 * this parameter is needed and should be set to 1.
	 */
	if (argc > 1) {
		if (isdigit(*argv[1])) {
			update_uboot = atoi(argv[1]);
		} else {
			print_err(print, "Unrecognized uboot update flag %s\n", argv[1]);
			return 1;
		}
	}

	qcsapi_retval = qcsapi_set_carrier_id(carrier_id, update_uboot);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0)
			print_out( print, "complete\n");
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wifi_get_spinor_jedecid( const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int statval = 0;
	unsigned int jedecid;
	int qcsapi_retval;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;

	qcsapi_retval = qcsapi_wifi_get_spinor_jedecid( the_interface, &jedecid );

	if (qcsapi_retval >= 0)
	{
		if (verbose_flag >= 0)
		{
			print_out( print, "0x%08x\n", jedecid );
		}
	}
	else
	{
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_get_custom_value( const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int statval = 0;
	int qcsapi_retval;
	qcsapi_output *print = p_calling_bundle->caller_output;
	char *key;
	char value[QCSAPI_CUSTOM_VALUE_MAX_LEN] = {'\0'};

	if (argc != 1) {
		print_err(print, "Usage: call_qcsapi get_custom_value <key>\n");
		return 1;
	}

	key = argv[0];
	qcsapi_retval = qcsapi_get_custom_value(key, value);

	if (qcsapi_retval >= 0) {
		print_out(print, "%s\n", value);
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wifi_get_mlme_stats_per_mac(const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval = 0;
	qcsapi_mac_addr the_mac_addr;
	qcsapi_mlme_stats stats;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc >= 1 && strcmp(argv[0], "NULL") != 0) {
		if (parse_mac_addr(argv[0], the_mac_addr) < 0) {
			print_out(print, "Error parsing MAC address %s\n", argv[0]);
			return 1;
		}
	} else {
		memset(the_mac_addr, 0x00, sizeof(the_mac_addr));
	}

	qcsapi_retval = qcsapi_wifi_get_mlme_stats_per_mac(the_mac_addr, &stats);

	if (qcsapi_retval >= 0) {
		print_out(print,
				  "auth:\t\t%u\n"
				  "auth_fails:\t%u\n"
				  "assoc:\t\t%u\n"
				  "assoc_fails:\t%u\n"
				  "deauth:\t\t%u\n"
				  "diassoc:\t%u\n",
				  stats.auth,
				  stats.auth_fails,
				  stats.assoc,
				  stats.assoc_fails,
				  stats.deauth,
				  stats.diassoc);
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}
static int
call_qcsapi_wifi_get_mlme_stats_per_association(const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval = 0;
	qcsapi_mlme_stats stats;
	qcsapi_output *print = p_calling_bundle->caller_output;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_unsigned_int association_index = p_calling_bundle->caller_generic_parameter.index;

	qcsapi_retval = qcsapi_wifi_get_mlme_stats_per_association(the_interface, association_index, &stats);

	if (qcsapi_retval >= 0) {
		print_out(print,
				  "auth:\t\t%u\n"
				  "auth_fails:\t%u\n"
				  "assoc:\t\t%u\n"
				  "assoc_fails:\t%u\n"
				  "deauth:\t\t%u\n"
				  "diassoc:\t%u\n",
				  stats.auth,
				  stats.auth_fails,
				  stats.assoc,
				  stats.assoc_fails,
				  stats.deauth,
				  stats.diassoc);
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wifi_get_mlme_stats_macs_list(const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;
	qcsapi_mlme_stats_macs mac_list;
	qcsapi_mac_addr terminator_addr;
	int i;

	memset(&terminator_addr, 0xFF, sizeof(terminator_addr));

	qcsapi_retval = qcsapi_wifi_get_mlme_stats_macs_list(&mac_list);

	if (qcsapi_retval >= 0) {
		for (i = 0;i < QCSAPI_MLME_STATS_MAX_MACS; ++i) {
			if (memcmp(mac_list.addr[i], terminator_addr, sizeof(qcsapi_mac_addr)) == 0) {
				break;
			}
			dump_mac_addr(print, mac_list.addr[i]);
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wifi_get_nss_cap(const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	qcsapi_output *print = p_calling_bundle->caller_output;
	qcsapi_mimo_type modulation;
	int qcsapi_retval;
	unsigned int nss;

	modulation = p_calling_bundle->caller_generic_parameter.parameter_type.modulation;
	qcsapi_retval = qcsapi_wifi_get_nss_cap(p_calling_bundle->caller_interface,
						modulation, &nss);

	if (qcsapi_retval >= 0) {
		print_out(print, "%u\n", nss);
		if (verbose_flag >= 0) {
			print_out(print, "complete\n");
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		return 1;
	}

	return 0;
}

static int
call_qcsapi_wifi_set_nss_cap(const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	qcsapi_output *const print = p_calling_bundle->caller_output;
	qcsapi_mimo_type modulation;
	int retval = 0;

	modulation = p_calling_bundle->caller_generic_parameter.parameter_type.modulation;

	if (argc != 1) {
		print_err(print, "Usage: call_qcsapi set_nss_cap "
					"<WiFi interface> {ht|vht} <nss>\n");
		retval = 1;
	} else {
		qcsapi_unsigned_int nss = (qcsapi_unsigned_int)atoi(argv[0]);
		int qcsapi_retval;

		qcsapi_retval = qcsapi_wifi_set_nss_cap(p_calling_bundle->caller_interface,
							modulation, nss);

		if (qcsapi_retval >= 0) {
			if (verbose_flag >= 0) {
				print_out(print, "complete\n");
			}
		} else {
			report_qcsapi_error(p_calling_bundle, qcsapi_retval);
			retval = 1;
		}
	}

	return retval;
}

static int
call_qcsapi_wifi_get_security_defer_mode(const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	qcsapi_output *print = p_calling_bundle->caller_output;
	int qcsapi_retval;
	int defer;

	qcsapi_retval = qcsapi_wifi_get_security_defer_mode(p_calling_bundle->caller_interface, &defer);

	if (qcsapi_retval >= 0) {
		print_out(print, "%d\n", defer);
		if (verbose_flag >= 0) {
			print_out(print, "complete\n");
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		return 1;
	}

	return 0;
}

static int
call_qcsapi_wifi_set_security_defer_mode(const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	qcsapi_output *const print = p_calling_bundle->caller_output;
	int retval = 0;

	if (argc != 1) {
		print_err(print, "Usage: call_qcsapi set_defer "
					"wifi0 {0|1}\n");
		retval = 1;
	} else {
		int defer = (qcsapi_unsigned_int)atoi(argv[0]);
		int qcsapi_retval;

		qcsapi_retval = qcsapi_wifi_set_security_defer_mode(p_calling_bundle->caller_interface, defer);

		if (qcsapi_retval >= 0) {
			if (verbose_flag >= 0) {
				print_out(print, "complete\n");
			}
		} else {
			report_qcsapi_error(p_calling_bundle, qcsapi_retval);
			retval = 1;
		}
	}

	return retval;
}

static int
call_qcsapi_wifi_apply_security_config(const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	qcsapi_output *const print = p_calling_bundle->caller_output;
	int retval = 0;

	if (argc != 0) {
		print_err(print, "Usage: call_qcsapi apply_security_config "
					"<WiFi interface>\n");
		retval = 1;
	} else {
		int qcsapi_retval = 0;

		qcsapi_retval = qcsapi_wifi_apply_security_config(p_calling_bundle->caller_interface);
		if (qcsapi_retval >= 0) {
			if (verbose_flag >= 0) {
				print_out(print, "complete\n");
			}
		} else {
			report_qcsapi_error(p_calling_bundle, qcsapi_retval);
			retval = 1;
		}
	}

	return retval;
}

static int
call_qcsapi_wifi_set_intra_bss_isolate(call_qcsapi_bundle *p_calling_bundle,
		int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval = 0;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;
	qcsapi_unsigned_int enable;

	if (argc < 1) {
		print_err(print, "Not enough parameters, count is %d\n", argc);
		return 1;
	}

	enable = (qcsapi_unsigned_int)atoi(argv[0]);
	if (enable > 1) {
		print_err(print, "bad parameter %s\n", argv[0]);
		return 1;
	}

	qcsapi_retval = qcsapi_wifi_set_intra_bss_isolate(the_interface, enable);
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0)
			print_out(print, "complete\n");
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wifi_get_intra_bss_isolate(call_qcsapi_bundle *p_calling_bundle,
		int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval = 0;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;
	qcsapi_unsigned_int enable;

	qcsapi_retval = qcsapi_wifi_get_intra_bss_isolate(the_interface, &enable);
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0)
			print_out(print, "%u\n", enable);
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wifi_set_bss_isolate(call_qcsapi_bundle *p_calling_bundle,
		int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval = 0;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;
	qcsapi_unsigned_int enable;

	if (argc < 1) {
		print_err(print, "Not enough parameters, count is %d\n", argc);
		return 1;
	}

	enable = (qcsapi_unsigned_int)atoi(argv[0]);
	if (enable > 1) {
		print_err(print, "bad parameter %s\n", argv[0]);
		return 1;
	}

	qcsapi_retval = qcsapi_wifi_set_bss_isolate(the_interface, enable);
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0)
			print_out(print, "complete\n");
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wifi_get_bss_isolate(call_qcsapi_bundle *p_calling_bundle,
		int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval = 0;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;
	qcsapi_unsigned_int enable;

	qcsapi_retval = qcsapi_wifi_get_bss_isolate(the_interface, &enable);
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0)
			print_out(print, "%u\n", enable);
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wifi_host_state_set(const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	uint16_t host_state;
	int qcsapi_retval;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1) {
		print_err(print, "not enough params\n");
		print_err(print, "Usage: call_qcsapi wowlan_host_state "
					"<WiFi interface> {0|1}\n");
		return 1;
	}

	if (isdigit(*argv[0])) {
		host_state = atoi(argv[0]);
	} else {
		return 1;
	}
	qcsapi_retval = qcsapi_set_host_state(the_interface, host_state);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out(print, "success\n");
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_host_state_get(call_qcsapi_bundle *p_calling_bundle,
		int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval = 0;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;
	uint16_t host_state;
	qcsapi_unsigned_int host_state_len = sizeof(host_state);

	qcsapi_retval = qcsapi_wifi_wowlan_get_host_state(the_interface, &host_state, &host_state_len);
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0)
			print_out(print, "%u\n", host_state);
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wifi_wowlan_match_type_set(const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	uint16_t wowlan_match;
	int qcsapi_retval;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1) {
		print_err(print, "not enough params\n");
		print_err(print, "Usage: call_qcsapi wowlan_match_type "
					"<WiFi interface> <protocol> "
					"protocol should be 0, 1(L2) or 2(L3) "
					"0 means match standard magic L2 type(0x0842) or L3 UDP destination port(7 or 9)\n");
		return 1;
	}

	if (isdigit(*argv[0])) {
		wowlan_match = atoi(argv[0]);
	} else {
		return 1;
	}
	qcsapi_retval = qcsapi_wowlan_set_match_type(the_interface, wowlan_match);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out(print, "success\n");
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_wowlan_match_type_get(call_qcsapi_bundle *p_calling_bundle,
		int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval = 0;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;
	uint16_t match_type;
	qcsapi_unsigned_int len = sizeof(match_type);

	qcsapi_retval = qcsapi_wifi_wowlan_get_match_type(the_interface, &match_type, &len);
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0)
			print_out(print, "%u\n", match_type);
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}
static int
call_qcsapi_wifi_wowlan_L2_type_set(const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	uint16_t ether_type;
	int qcsapi_retval;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1) {
		print_err(print, "not enough params\n");
		print_err(print, "Usage: call_qcsapi wowlan_L2_type "
					"<WiFi interface> <Ether type>\n");
		return 1;
	}

	if (isdigit(*argv[0])) {
		ether_type = atoi(argv[0]);
	} else {
		return 1;
	}
	qcsapi_retval = qcsapi_wowlan_set_L2_type(the_interface, ether_type);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out(print, "success\n");
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_wowlan_L2_type_get(call_qcsapi_bundle *p_calling_bundle,
		int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval = 0;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;
	uint16_t l2_type;
	qcsapi_unsigned_int len = sizeof(l2_type);

	qcsapi_retval = qcsapi_wifi_wowlan_get_l2_type(the_interface, &l2_type, &len);
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0)
			print_out(print, "%u\n", l2_type);
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}
static int
call_qcsapi_wifi_wowlan_udp_port_set(const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	uint16_t udp_port;
	int qcsapi_retval;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1) {
		print_err(print, "not enough params\n");
		print_err(print, "Usage: call_qcsapi wowlan_udp_port "
					"<WiFi interface> <udp port>\n");
		return 1;
	}

	if (isdigit(*argv[0])) {
		udp_port = atoi(argv[0]);
	} else {
		return 1;
	}
	qcsapi_retval = qcsapi_wowlan_set_udp_port(the_interface, udp_port);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out(print, "success\n");
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return( statval );
}

static int
call_qcsapi_wifi_wowlan_udp_port_get(call_qcsapi_bundle *p_calling_bundle,
		int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval = 0;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;
	uint16_t udp_port;
	qcsapi_unsigned_int len = sizeof(udp_port);

	qcsapi_retval = qcsapi_wifi_wowlan_get_udp_port(the_interface, &udp_port, &len);
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0)
			print_out(print, "%u\n", udp_port);
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}
#define MAX_USER_DEFINED_MAGIC	256
void str_to_hex(uint8_t *pbDest, const char *pbSrc, int nLen)
{
	char h1,h2;
	uint8_t s1,s2;
	int i;

	for (i = 0; i < nLen; i++)
	{
		h1 = pbSrc[2*i];
		h2 = pbSrc[2*i+1];

		s1 = toupper(h1) - 0x30;
		if (s1 > 9)
		s1 -= 7;

		s2 = toupper(h2) - 0x30;
		if (s2 > 9)
			s2 -= 7;

		pbDest[i] = s1*16 + s2;
	}
}

int get_pattern_string(const char *arg, uint8_t *pattern)
{
	int loop = 0;
	int num = 0;
	int pattern_len = strnlen(arg, MAX_USER_DEFINED_MAGIC<<1);

	while (loop < pattern_len) {
		if (isxdigit(arg[loop]) && isxdigit(arg[loop+1])) {
			str_to_hex(&pattern[num], &arg[loop], 1);
			num++;
			loop += 2;
		} else {
			loop++;
		}
	}
	return num;
}

static int
call_qcsapi_wifi_wowlan_pattern_set(const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;
	uint8_t pattern[MAX_USER_DEFINED_MAGIC];
	struct qcsapi_data_256bytes pattern_data;
	uint32_t input_string_len;
	uint32_t actual_string_len;

	if (argc < 1) {
		print_err(print, "not enough params\n");
		print_err(print, "Usage: call_qcsapi wowlan_pattern "
					"<WiFi interface> <pattern> "
					"pattern should be aabb0a0b and 256 bytes in total length\n");
		return 1;
	}

	memset(pattern, 0, MAX_USER_DEFINED_MAGIC);
	if ((input_string_len = strnlen(argv[0], (MAX_USER_DEFINED_MAGIC<<1)+1)) > (MAX_USER_DEFINED_MAGIC<<1)) {
		print_err(print, "pattern should be 256 bytes in total length\n");
		return 1;
	}

	actual_string_len = get_pattern_string(argv[0], pattern);
	if (actual_string_len != (input_string_len>>1)) {
		print_err(print, "there are unrecognized chars\n");
		return 1;
	}

	memset(&pattern_data, 0, sizeof(pattern_data));
	memcpy(pattern_data.data, pattern, actual_string_len);
	qcsapi_retval = qcsapi_wowlan_set_magic_pattern(the_interface, &pattern_data, actual_string_len);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "success\n");
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}
	return( statval );
}

static void
dump_magic_pattern(qcsapi_output *print, struct qcsapi_data_256bytes *magic_pattern, qcsapi_unsigned_int pattern_len)
{
	int i;

	for (i = 0; i < pattern_len; i++) {
		print_out(print, "%02X", magic_pattern->data[i]);
	}
	print_out(print, "\n");
}

static int
call_qcsapi_wifi_wowlan_pattern_get(call_qcsapi_bundle *p_calling_bundle,
		int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval = 0;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;
	struct qcsapi_data_256bytes magic_pattern;
	qcsapi_unsigned_int pattern_len = sizeof(magic_pattern);

	memset(&magic_pattern, 0, sizeof(magic_pattern));
	qcsapi_retval = qcsapi_wifi_wowlan_get_magic_pattern(the_interface, &magic_pattern, &pattern_len);
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0)
			dump_magic_pattern(print, &magic_pattern, pattern_len);
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wifi_tdls_operate(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;
	qcsapi_tdls_oper operate = p_calling_bundle->caller_generic_parameter.parameter_type.tdls_oper;
	int statval = 0;
	int qcsapi_retval = 0;
	int cs_interval = 0;

	if (operate == qcsapi_tdls_oper_switch_chan) {
		if (argc < 2) {
			print_err(print, "Not enough parameters, count is %d\n", argc);
			return 1;
		}
		cs_interval = atoi(argv[1]);
	} else {
		if (argc < 1) {
			print_err(print, "Not enough parameters, count is %d\n", argc);
			return 1;
		}
	}

	qcsapi_retval = qcsapi_wifi_tdls_operate(the_interface, operate, argv[0], cs_interval);
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0)
			print_out(print, "complete\n");
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int call_qcsapi_wifi_set_extender_params(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval = 0;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;
	qcsapi_extender_type type = p_calling_bundle->caller_generic_parameter.parameter_type.type_of_extender;
	int value = 0;

	if (argc < 1) {
		print_err(print, "Not enough parameters\n");
		statval = 1;
		goto out;
	}

	switch (type) {
	case qcsapi_extender_role:
		if (strcasecmp(argv[0], "mbs") == 0) {
			value = IEEE80211_EXTENDER_ROLE_MBS;
		} else if (strcasecmp(argv[0], "rbs") == 0) {
			value = IEEE80211_EXTENDER_ROLE_RBS;
		} else if (strcasecmp(argv[0], "none") == 0) {
			value = IEEE80211_EXTENDER_ROLE_NONE;
		} else {
			print_err(print, "invalid role [%s]\n", argv[0]);
			statval = 1;
			goto out;
		}
		break;
	case qcsapi_extender_mbs_best_rssi:
	case qcsapi_extender_rbs_best_rssi:
	case qcsapi_extender_mbs_wgt:
	case qcsapi_extender_rbs_wgt:
	case qcsapi_extender_verbose:
	case qcsapi_extender_roaming:
	case qcsapi_extender_bgscan_interval:
		if (sscanf(argv[0], "%d", &value) != 1) {
			print_err(print, "Error parsing '%s'\n", argv[0]);
			return 1;
		}
		break;
	default:
		statval = 1;
		goto out;
		break;
	}

	qcsapi_retval = qcsapi_wifi_set_extender_params(the_interface, type, value);
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0)
			print_out(print, "complete\n");
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}
out:
	return statval;
}

static int
call_qcsapi_wifi_get_bgscan_status(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval = 0;
	int enable = 0;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;

	qcsapi_retval = qcsapi_wifi_get_bgscan_status(the_interface, &enable);

	if (qcsapi_retval >= 0) {
		print_out( print, "Bgscan enable: %d\n", enable);
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wifi_enable_bgscan(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval = 0;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;
	int enable = 0;

	if (argc < 1) {
		print_err(print, "Not enough parameters, count is %d\n", argc);
		return 1;
	}

	if (isdigit(*argv[0])) {
		enable = atoi(argv[0]);
	} else {
		print_err(print, "Unrecognized parameter value %s\n", argv[0]);
		return 1;
	}

	qcsapi_retval = qcsapi_wifi_enable_bgscan(the_interface, enable);
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0)
			print_out(print, "complete\n");
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static void
print_extender_params(qcsapi_extender_type type, int value, qcsapi_output *print,
	int iter)
{
	char *role = NULL;
	if (type == qcsapi_extender_role) {
		switch(value) {
		case IEEE80211_EXTENDER_ROLE_NONE:
			role = "NONE";
			break;
		case IEEE80211_EXTENDER_ROLE_MBS:
			role = "MBS";
			break;
		case IEEE80211_EXTENDER_ROLE_RBS:
			role = "RBS";
			break;
		default:
			break;
		}
		print_out(print, "%s: %s\n",
			qcsapi_extender_param_table[iter].param_name, role);
	} else {
		print_out(print, "%s: %d\n",
			qcsapi_extender_param_table[iter].param_name, value);
	}
}

static void
print_eth_info(qcsapi_eth_info_type type, qcsapi_eth_info_result value, qcsapi_output *print)
{
	int iter;
	int mask = 0;

	for (iter = 0; iter < ARRAY_SIZE(qcsapi_eth_info_type_mask_table); iter++) {
		if (qcsapi_eth_info_type_mask_table[iter].type == type) {
			mask = qcsapi_eth_info_type_mask_table[iter].mask;
			break;
		}
	}

	for (iter = 0; iter < ARRAY_SIZE(qcsapi_eth_info_result_table); iter++) {
		if (!(mask & 1 << iter))
			continue;
		if (value & qcsapi_eth_info_result_table[iter].result_type) {
			if (qcsapi_eth_info_result_table[iter].result_bit_set) {
				print_out(print, "%s: %s\n",
					qcsapi_eth_info_result_table[iter].result_label,
					qcsapi_eth_info_result_table[iter].result_bit_set);
			}
		} else {
			if (qcsapi_eth_info_result_table[iter].result_bit_unset) {
				print_out(print, "%s: %s\n",
					qcsapi_eth_info_result_table[iter].result_label,
					qcsapi_eth_info_result_table[iter].result_bit_unset);
			}
		}
	}
}

static int
call_qcsapi_wifi_get_tx_amsdu(call_qcsapi_bundle *p_calling_bundle,
		int argc, char *argv[])
{
	int statval = 0;
	int enable, qcsapi_retval;
	const char *wifi = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;

	qcsapi_retval = qcsapi_wifi_get_tx_amsdu(wifi, &enable);

	if (qcsapi_retval >= 0) {
		print_out(print, "%d\n", enable);
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wifi_set_tx_amsdu(call_qcsapi_bundle *p_calling_bundle,
		int argc, char *argv[])
{
	int statval = 0;
	int enable, qcsapi_retval;
	const char *wifi = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 1) {
		print_err(print, "Usage: call_qcsapi set_tx_amsdu "
				"<WiFi interface> { 0 | 1 }\n");
		return 1;
	}

	enable = atoi(argv[0]);
	if ((enable != 0) && (enable != 1)) {
		print_err(print, "bad parameter %s\n", argv[0]);
		return 1;
	}

	qcsapi_retval = qcsapi_wifi_set_tx_amsdu(wifi, enable);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0)
			print_out(print, "success\n");
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wifi_get_extender_status(call_qcsapi_bundle *p_calling_bundle,
	int argc, char *argv[])
{
	int qcsapi_retval;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;
	qcsapi_extender_type type = 0;
	int value = 0;
	unsigned int iter;

	for (iter = 0; iter < ARRAY_SIZE(qcsapi_extender_param_table); iter++) {
		type = qcsapi_extender_param_table[iter].param_type;
		if (type == qcsapi_extender_nosuch_param)
			continue;
		qcsapi_retval = qcsapi_wifi_get_extender_params(the_interface,
			type, &value);

		if (qcsapi_retval >= 0) {
			if (verbose_flag >= 0) {
				print_extender_params(type, value, print, iter);
			}
		} else {
			report_qcsapi_error(p_calling_bundle, qcsapi_retval);
			return 1;
		}
	}

	return 0;
}

static int
call_qcsapi_is_startprod_done(call_qcsapi_bundle *p_calling_bundle,
	int argc, char *argv[])
{
    int qcsapi_retval=0;
    int status=0;

    qcsapi_output *print = p_calling_bundle->caller_output;

    qcsapi_retval = qcsapi_is_startprod_done(&status);

    if (qcsapi_retval < 0) {
        report_qcsapi_error(p_calling_bundle, qcsapi_retval);
        return 1;
    }

    if (verbose_flag >= 0) {
        print_out(print, "%d\n",status);
    }

    return 0;
}

static int
call_qcsapi_wifi_get_disassoc_reason(call_qcsapi_bundle *call, int argc, char *argv[])
{
        int rc = 0;
        int qcsapi_retval;

        qcsapi_unsigned_int     disassoc_reason;
        qcsapi_output *print = call->caller_output;
        const char *the_interface = call->caller_interface;

        qcsapi_retval = qcsapi_wifi_get_disassoc_reason( the_interface, &disassoc_reason);
        if (qcsapi_retval >= 0) {
		if (disassoc_reason <= ARRAY_SIZE(qcsapi_disassoc_reason_list)) {
			print_out(print,"Disassoc Reason Code - %d: %s\n", disassoc_reason, qcsapi_disassoc_reason_list[disassoc_reason].reason_string);
		} else {
			print_out(print,"Reserved Code [%d]", disassoc_reason);
		}
        } else {
                report_qcsapi_error(call, qcsapi_retval);
                rc = 1;
        }

        return rc;
}

static int
call_qcsapi_wifi_get_bb_param( const call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
        int statval = 0;
        int qcsapi_retval;
        qcsapi_output *print = p_calling_bundle->caller_output;
	const char              *the_interface = p_calling_bundle->caller_interface;
	qcsapi_unsigned_int      bb_param;

        qcsapi_retval = qcsapi_wifi_get_bb_param(the_interface, &bb_param);

        if (qcsapi_retval >= 0) {
                print_out(print, "%d\n", bb_param);
        } else {
                report_qcsapi_error(p_calling_bundle, qcsapi_retval);
                statval = 1;
        }

        return statval;
}

static int
call_qcsapi_wifi_set_bb_param(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{

        int     statval = 0;
        qcsapi_output *print = p_calling_bundle->caller_output;

        if (argc < 1)
        {
                print_err( print, "Not enough parameters in call qcsapi WiFi bb_param, count is %d\n", argc );
                statval = 1;
        }
        else
        {
                qcsapi_unsigned_int      bb_param = atoi( argv[ 0 ] );
                int                      qcsapi_retval;
                const char              *the_interface = p_calling_bundle->caller_interface;

                qcsapi_retval = qcsapi_wifi_set_bb_param(the_interface, bb_param);
                if (qcsapi_retval >= 0)
                {
                        if (bb_param >= 0)
                        {
                                print_out( print, "complete\n" );
                        }
                }
                else
                {
                        report_qcsapi_error(p_calling_bundle, qcsapi_retval );
                        statval = 1;
                }
        }

        return( statval );
}

static int
call_qcsapi_wifi_set_scan_buf_max_size(call_qcsapi_bundle *p_calling_bundle,
		int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval = 0;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;
	qcsapi_unsigned_int max_buf_size;

	if (argc < 1) {
		print_err(print, "Not enough parameters, count is %d\n", argc);
		return 1;
	}

	max_buf_size = (qcsapi_unsigned_int)atoi(argv[0]);

	qcsapi_retval = qcsapi_wifi_set_scan_buf_max_size(the_interface, max_buf_size);
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0)
			print_out(print, "complete\n");
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wifi_get_scan_buf_max_size(call_qcsapi_bundle *p_calling_bundle,
		int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval = 0;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;
	qcsapi_unsigned_int max_buf_size;

	qcsapi_retval = qcsapi_wifi_get_scan_buf_max_size(the_interface, &max_buf_size);
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0)
			print_out(print, "%u\n", max_buf_size);
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wifi_set_scan_table_max_len(call_qcsapi_bundle *p_calling_bundle,
		int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval = 0;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;
	qcsapi_unsigned_int max_table_len;

	if (argc < 1) {
		print_err(print, "Not enough parameters, count is %d\n", argc);
		return 1;
	}

	max_table_len = (qcsapi_unsigned_int)atoi(argv[0]);
	qcsapi_retval = qcsapi_wifi_set_scan_table_max_len(the_interface, max_table_len);
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0)
			print_out(print, "complete\n");
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wifi_get_scan_table_max_len(call_qcsapi_bundle *p_calling_bundle,
		int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval = 0;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;
	qcsapi_unsigned_int max_table_len;

	qcsapi_retval = qcsapi_wifi_get_scan_table_max_len(the_interface, &max_table_len);
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0)
			print_out(print, "%u\n", max_table_len);
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wifi_set_enable_mu(call_qcsapi_bundle *p_calling_bundle,
		int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval = 0;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;
	qcsapi_unsigned_int mu_enable;

	if (argc < 1 || strcmp(argv[0], "NULL") == 0) {
		print_err(print, "Not enough parameters, count is %d\n", argc);
		return 1;
	}

	mu_enable = (qcsapi_unsigned_int)atoi(argv[0]);
	qcsapi_retval = qcsapi_wifi_set_enable_mu(the_interface, mu_enable);

	if (qcsapi_retval < 0) {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	} else if (verbose_flag >= 0) {
		print_out(print, "complete\n");
	}

	return statval;
}

static int
call_qcsapi_wifi_get_enable_mu(call_qcsapi_bundle *p_calling_bundle,
		int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval = 0;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;
	qcsapi_unsigned_int mu_enable;

	qcsapi_retval = qcsapi_wifi_get_enable_mu(the_interface, &mu_enable);

	if (qcsapi_retval < 0) {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	} else if (verbose_flag >= 0) {
		print_out(print, "%u\n", mu_enable);
	}

	return statval;
}

static int
call_qcsapi_wifi_set_mu_use_precode(call_qcsapi_bundle *p_calling_bundle,
		int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval = 0;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;
	qcsapi_unsigned_int prec_enable;
	qcsapi_unsigned_int grp;

	if (argc < 2 || strcmp(argv[1], "NULL") == 0 || strcmp(argv[0], "NULL") == 0) {
		print_err(print, "Not enough parameters, count is %d\n", argc);
		return 1;
	}

	grp = (qcsapi_unsigned_int)atoi(argv[0]);
	prec_enable = (qcsapi_unsigned_int)atoi(argv[1]);

	qcsapi_retval = qcsapi_wifi_set_mu_use_precode(the_interface, grp, prec_enable);

	if (qcsapi_retval < 0) {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	} else if (verbose_flag >= 0) {
		print_out(print, "complete\n");
	}

	return statval;
}

static int
call_qcsapi_wifi_get_mu_use_precode(call_qcsapi_bundle *p_calling_bundle,
		int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval = 0;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;
	qcsapi_unsigned_int prec_enable;
	qcsapi_unsigned_int grp;

	if (argc < 1 || strcmp(argv[0], "NULL") == 0) {
		print_err(print, "Not enough parameters, count is %d\n", argc);
		return 1;
	}

	grp = (qcsapi_unsigned_int)atoi(argv[0]);
	qcsapi_retval = qcsapi_wifi_get_mu_use_precode(the_interface, grp, &prec_enable);

	if (qcsapi_retval < 0) {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	} else if (verbose_flag >= 0) {
		print_out(print, "%u\n", prec_enable);
	}

	return statval;
}

static int
call_qcsapi_wifi_set_mu_use_eq(call_qcsapi_bundle *p_calling_bundle,
		int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval = 0;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;
	qcsapi_unsigned_int eq_enable;

	if (argc < 1 || strcmp(argv[0], "NULL") == 0) {
		print_err(print, "Not enough parameters, count is %d\n", argc);
		return 1;
	}

	eq_enable = (qcsapi_unsigned_int)atoi(argv[0]);
	qcsapi_retval = qcsapi_wifi_set_mu_use_eq(the_interface, eq_enable);

	if (qcsapi_retval < 0) {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	} else if (verbose_flag >= 0) {
		print_out(print, "complete\n");
	}

	return statval;
}

static int
call_qcsapi_wifi_get_mu_use_eq(call_qcsapi_bundle *p_calling_bundle,
		int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval = 0;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;
	qcsapi_unsigned_int eq_enable;

	qcsapi_retval = qcsapi_wifi_get_mu_use_eq(the_interface, &eq_enable);

	if (qcsapi_retval < 0) {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	} else if (verbose_flag >= 0) {
		print_out(print, "%u\n", eq_enable);
	}

	return statval;
}

static int
call_qcsapi_wifi_get_mu_groups(call_qcsapi_bundle *p_calling_bundle,
		int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval = 0;
	const char *the_interface = p_calling_bundle->caller_interface;
	qcsapi_output *print = p_calling_bundle->caller_output;
	char buf[1024];

	qcsapi_retval = qcsapi_wifi_get_mu_groups(the_interface, &buf[0], sizeof(buf));

	if (qcsapi_retval < 0) {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	} else if (verbose_flag >= 0) {
		print_out(print, "%s", buf);
	}

	return statval;
}

static int
call_qcsapi_wifi_set_optim_stats(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
        int     statval = 0;
        qcsapi_output *print = p_calling_bundle->caller_output;
	char *str_ptr;

        if (argc < 1)
        {
                print_err( print, "Not enough parameters in call qcsapi WiFi set_optim_stats, count is %d\n", argc );
                statval = 1;
        }
        else
        {
                qcsapi_unsigned_int      rx_optim_stats = strtol(argv[ 0 ], &str_ptr, 10);
                int                      qcsapi_retval;
                const char              *the_interface = p_calling_bundle->caller_interface;

                qcsapi_retval = qcsapi_wifi_set_optim_stats(the_interface, rx_optim_stats);
                if (qcsapi_retval >= 0)
                {
			print_out( print, "complete\n" );
                }
                else
                {
                        report_qcsapi_error(p_calling_bundle, qcsapi_retval );
                        statval = 1;
                }
        }

        return( statval );
}

static int call_qcsapi_send_file(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval;
	const char *image_file_path = NULL;
	int image_flags = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc != 1 && argc != 2) {
		print_err(print, "Usage: call_qcsapi send_file <image file path> <flags>\n");
		statval = 1;
	} else {
		if (strcmp(argv[0], "NULL") != 0) {
			image_file_path = argv[0];

			qcsapi_retval = qcsapi_send_file(image_file_path, image_flags);
			if (qcsapi_retval < 0) {
				report_qcsapi_error(p_calling_bundle, qcsapi_retval);
				statval = 1;
			}
		} else {
			statval = 1;
		}
	}

	return statval;
}

static int
call_qcsapi_dscp_fill(call_qcsapi_bundle *p_calling_bundle,
		int argc, char *argv[])
{
	int statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 3) {
		statval = 1;
		print_err(print, "Usage: call_qcsapi dscp <fill> <emac0|emac1> <value>\n");
	}
	else {
		int qcsapi_retval;
		const char *eth_type = argv[1];
		const char *value = argv[2];

		if (strcmp(eth_type, "NULL") == 0) {
			eth_type = NULL;
		}
		if (strcmp(value, "NULL") == 0) {
			value = NULL;
		}

		qcsapi_retval = qcsapi_eth_dscp_map(qcsapi_eth_dscp_fill,
							eth_type,
							NULL,
							value,
							NULL,
							0);

		if (qcsapi_retval >= 0) {
			if (verbose_flag >= 0) {
				print_out(print, "complete\n" );
			}
		} else {
			report_qcsapi_error(p_calling_bundle, qcsapi_retval);
			statval = 1;
		}
	}

	return statval;
}

static int
call_qcsapi_dscp_poke(call_qcsapi_bundle *p_calling_bundle,
		int argc, char *argv[])
{
	int statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 4) {
		print_err(print, "Usage: call_qcsapi dscp <poke> <emac0|emac1> <level> <value>\n");
		statval = 1;
	}
	else {
		int qcsapi_retval;
		const char *eth_type = argv[1];
		const char *level = argv[2];
		const char *value = argv[3];

		if (strcmp(eth_type, "NULL") == 0) {
			eth_type = NULL;
		}
		if (strcmp(level, "NULL") == 0) {
			level = NULL;
		}
		if (strcmp(value, "NULL") == 0) {
			value = NULL;
		}

		qcsapi_retval = qcsapi_eth_dscp_map(qcsapi_eth_dscp_poke,
							eth_type,
							level,
							value,
							NULL,
							0);

		if (qcsapi_retval >= 0) {
			if (verbose_flag >= 0) {
				print_out(print, "complete\n");
			}
		} else {
			report_qcsapi_error(p_calling_bundle, qcsapi_retval);
			statval = 1;
		}
	}

	return statval;
}

static int
call_qcsapi_dscp_dump(call_qcsapi_bundle *p_calling_bundle,
		int argc, char *argv[])
{
	int statval = 0;
	int qcsapi_retval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;
	char buf[2048] = {0};
	char *eth_type = argv[1];

	if (strcmp(eth_type, "NULL") == 0) {
		eth_type = NULL;
	}

	qcsapi_retval = qcsapi_eth_dscp_map(qcsapi_eth_dscp_dump,
						eth_type,
						NULL,
						NULL,
						&buf[0],
						sizeof(buf));

	if (qcsapi_retval < 0) {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	} else if (verbose_flag >= 0) {
		print_out(print, "%s", buf);
	}

	return statval;
}

static int
call_qcsapi_get_emac_switch(call_qcsapi_bundle *p_calling_bundle,
		int argc, char *argv[])
{
	int statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;
	int qcsapi_retval = 0;
	char buf[2048] = {0};

	qcsapi_retval = qcsapi_get_emac_switch(buf);

	if (qcsapi_retval < 0) {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	} else if (verbose_flag >= 0) {
		print_out(print, "%s\n", buf);
	}

	return statval;
}

static int
call_qcsapi_set_emac_switch(call_qcsapi_bundle *p_calling_bundle,
		int argc, char *argv[])
{
	int statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;
	int qcsapi_retval = 0;
	qcsapi_unsigned_int value;

	value = (qcsapi_unsigned_int)atoi(argv[0]);
	if (value == 0) {
		qcsapi_retval = qcsapi_set_emac_switch(qcsapi_emac_switch_enable);
	} else {
		qcsapi_retval = qcsapi_set_emac_switch(qcsapi_emac_switch_disable);
	}

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out(print, "complete\n");
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_eth_dscp_map(call_qcsapi_bundle *p_calling_bundle,
		int argc, char *argv[])
{
	int statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc < 2) {
		print_err(print, "Usage: call_qcsapi dscp <fill|poke|dump>"
					" <emac0|emac1> [level] [value]\n");
		statval = 1;
	} else {
		char *param = argv[0];

		if (strcmp(param, "fill") == 0) {
			statval = call_qcsapi_dscp_fill(p_calling_bundle, argc, argv);
		} else if (strcmp(param, "poke") == 0) {
			statval = call_qcsapi_dscp_poke(p_calling_bundle, argc, argv);
		} else if (strcmp(param, "dump") == 0) {
			statval = call_qcsapi_dscp_dump(p_calling_bundle, argc, argv);
		} else {
			print_err(print, "Usage: call_qcsapi dscp <fill|poke|dump>"
						" <emac0|emac1> [level] [value]\n");
			statval = 1;
		}
	}

	return statval;
}

static int
call_qcsapi_get_eth_info(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	qcsapi_output		*print = p_calling_bundle->caller_output;
	qcsapi_eth_info_type	eth_info_type = qcsapi_eth_nosuch_type;
	qcsapi_eth_info_result	eth_info_result = qcsapi_eth_info_unknown;
	int			qcsapi_retval;
	const char		*the_interface = p_calling_bundle->caller_interface;

	if (argc != 0 && argc != 1) {
		print_err(print, "Usage: call_qcsapi get_eth_info <ifname> "
					"{ link | speed | duplex | autoneg }\n");
		return 1;
	}

	if (argc == 0) {
		for (eth_info_type = qcsapi_eth_info_start;
				eth_info_type < qcsapi_eth_info_all;
				eth_info_type++) {
			qcsapi_retval = qcsapi_get_eth_info(the_interface, eth_info_type);
			if (qcsapi_retval >= 0) {
				eth_info_result |= (qcsapi_eth_info_result)qcsapi_retval;
			} else {
				report_qcsapi_error(p_calling_bundle, qcsapi_retval);
				return 1;
			}
		}
		print_eth_info(eth_info_type, eth_info_result, print);
		return 0;
	}

	if (!strcmp("link", argv[0])) {
		eth_info_type = qcsapi_eth_info_link;
	} else if (!strcmp("speed", argv[0])) {
		eth_info_type = qcsapi_eth_info_speed;
	} else if (!strcmp("duplex", argv[0])) {
		eth_info_type = qcsapi_eth_info_duplex;
	} else if (!strcmp("autoneg", argv[0])) {
		eth_info_type = qcsapi_eth_info_autoneg;
	} else {
		print_out(print, "Invalid option\n");
		return 1;
	}

	qcsapi_retval = qcsapi_get_eth_info(the_interface, eth_info_type);
	if (qcsapi_retval >= 0) {
		print_eth_info(eth_info_type, qcsapi_retval, print);
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		return 1;
	}

	return 0;
}

static int
call_qcsapi_set_sys_time(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval;
	qcsapi_output *print = p_calling_bundle->caller_output;
	unsigned long sec;

	if (argc != 1) {
		print_err(print, "Usage: call_qcsapi set_sys_time <seconds since epoch>\n");
		return 1;
	}

	if (qcsapi_verify_numeric(argv[0]) < 0) {
		print_err(print, "Invalid value for seconds since epoch\n");
		return 1;
	}

	sec = strtoul(argv[0], NULL, 10);
	if (sec == 0 || sec >= UINT32_MAX) {
		print_err(print, "Invalid value for seconds since epoch\n");
		return 1;
	}

	statval = qcsapi_wifi_set_sys_time((uint32_t)sec);
	if (statval >= 0 && verbose_flag >= 0)
		print_out(print, "complete\n");
	else
		report_qcsapi_error(p_calling_bundle, statval);

	return statval;
}

static int
call_qcsapi_get_sys_time(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;
	uint32_t sec;

	if (argc != 0) {
		print_err(print, "Usage: call_qcsapi get_sys_time\n");
		return 1;
	}

	statval = qcsapi_wifi_get_sys_time(&sec);
	if (statval == 0) {
		print_out(print, "%u\n", sec);
	} else {
		report_qcsapi_error(p_calling_bundle, statval);
	}

	return statval;
}

static int
call_qcsapi_wifi_block_bss(call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[])
{
	int	statval = 0;
	int	qcsapi_retval;
	qcsapi_output *print = p_calling_bundle->caller_output;
	qcsapi_unsigned_int	 flag;
	const char		*the_interface = p_calling_bundle->caller_interface;

	if (argc < 1) {
		print_err( print, "Not enough parameters in call_qcsapi block_bss\n" );
		print_err( print, "Usage:  call_qcsapi block_bss <wifix> <0/1> \n");
		return 1;
	}

	if (isdigit(*argv[0])) {
		flag = atoi(argv[0]);
	} else {
		print_err(print, "Unrecognized %s\n", argv[0]);
		return 1;
	}

	qcsapi_retval = qcsapi_wifi_block_bss(the_interface, flag);
	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "complete\n" );
		}
	} else {
		report_qcsapi_error( p_calling_bundle, qcsapi_retval );
		statval = 1;
	}

	return (statval);
}

static int
call_qcsapi_wifi_set_ap_interface_name(call_qcsapi_bundle *p_calling_bundle,
			int argc, char *argv[])
{
	int qcsapi_retval;
	int statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc != 1) {
		print_out(print, "Usage: call_qcsapi "
					"set_ap_interface_name <interface name>\n");
		return -EINVAL;
	}

	qcsapi_retval = qcsapi_wifi_set_ap_interface_name(argv[0]);

	if (qcsapi_retval >= 0) {
		if (verbose_flag >= 0) {
			print_out( print, "complete\n");
		}
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wifi_get_ap_interface_name(call_qcsapi_bundle *p_calling_bundle,
			int argc, char *argv[])
{
	int qcsapi_retval = 0;
	int statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;
	char ifname[IFNAMSIZ] = {0};

	if (argc > 0) {
		print_out(print, "Usage: call_qcsapi "
					"get_ap_interface_name\n");
		return -EINVAL;
	}

	qcsapi_retval = qcsapi_wifi_get_ap_interface_name(ifname);
	if(qcsapi_retval >= 0) {
                print_out(print, "%s\n", ifname);
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

static int
call_qcsapi_wifi_verify_repeater_mode(call_qcsapi_bundle *p_calling_bundle,
			int argc, char *argv[])
{
	int qcsapi_retval = 0;
	int statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

	if (argc > 0) {
		print_out(print, "Usage: call_qcsapi "
					"verify_repeater_mode\n");
		return -EINVAL;
	}

	qcsapi_retval = qcsapi_wifi_verify_repeater_mode();
	if(qcsapi_retval >= 0) {
                print_out(print, "%d\n", qcsapi_retval);
	} else {
		report_qcsapi_error(p_calling_bundle, qcsapi_retval);
		statval = 1;
	}

	return statval;
}

/* end of programs to call individual QCSAPIs */

static int
call_particular_qcsapi( call_qcsapi_bundle *p_calling_bundle, int argc, char *argv[] )
{
	int	statval = 0;
	qcsapi_output *print = p_calling_bundle->caller_output;

  /*
   * Interface programs that SET a parameter require the
   * current list of arguments to get additional parameters
   */
	switch (p_calling_bundle->caller_qcsapi)
	{
	  case e_qcsapi_errno_get_message:
		statval = call_qcsapi_errno_get_message( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_store_ipaddr:
		statval = call_qcsapi_store_ipaddr( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_interface_enable:
		statval = call_qcsapi_interface_enable( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_interface_get_BSSID:
		statval = call_qcsapi_interface_get_BSSID( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_interface_get_mac_addr:
		statval = call_qcsapi_interface_get_mac_addr( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_interface_set_mac_addr:
		statval = call_qcsapi_interface_set_mac_addr( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_interface_get_counter:
		statval = call_qcsapi_interface_get_counter( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_interface_get_counter64:
		statval = call_qcsapi_interface_get_counter64( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_flash_image_update:
		statval = call_qcsapi_flash_image_update( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_firmware_get_version:
		statval = call_qcsapi_firmware_get_version( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_system_get_time_since_start:
		statval = call_qcsapi_system_get_time_since_start( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_get_system_status:
		statval = call_qcsapi_get_system_status( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_get_random_seed:
		statval = call_qcsapi_get_random_seed( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_set_random_seed:
		statval = call_qcsapi_set_random_seed( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_led_get:
		statval = call_qcsapi_led_get( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_led_set:
		statval = call_qcsapi_led_set( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_led_pwm_enable:
		statval = call_qcsapi_led_pwm_enable( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_led_brightness:
		statval = call_qcsapi_led_brightness( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_gpio_get_config:
		statval = call_qcsapi_gpio_get_config( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_gpio_set_config:
		statval = call_qcsapi_gpio_set_config( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_gpio_enable_wps_push_button:
		statval = call_qcsapi_gpio_enable_wps_push_button( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_file_path_get_config:
		statval = call_qcsapi_file_path_get_config( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_file_path_set_config:
		statval = call_qcsapi_file_path_set_config( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_set_wifi_macaddr:
		statval = call_qcsapi_wifi_set_wifi_macaddr( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_create_restricted_bss:
		statval = call_qcsapi_wifi_create_restricted_bss(p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_create_bss:
		statval = call_qcsapi_wifi_create_bss(p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_remove_bss:
		statval = call_qcsapi_wifi_remove_bss(p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_primary_interface:
		statval = call_qcsapi_wifi_get_primary_interface(p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_interface_by_index:
		statval = call_qcsapi_wifi_get_interface_by_index(p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_mode:
		statval = call_qcsapi_wifi_get_mode( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_set_mode:
		statval = call_qcsapi_wifi_set_mode( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_phy_mode:
		statval = call_qcsapi_wifi_get_phy_mode( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_set_phy_mode:
		statval = call_qcsapi_wifi_set_phy_mode( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_reload_in_mode:
		statval = call_qcsapi_wifi_reload_in_mode( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_rfenable:
		statval = call_qcsapi_wifi_rfenable( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_rfstatus:
		statval = call_qcsapi_wifi_rfstatus( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_startprod:
		statval = call_qcsapi_wifi_startprod( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_bw:
		statval = call_qcsapi_wifi_get_bw( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_set_bw:
		statval = call_qcsapi_wifi_set_bw( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_noise:
		statval = call_qcsapi_wifi_get_noise( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_rssi_by_chain:
		statval = call_qcsapi_wifi_get_rssi_by_chain( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_avg_snr:
		statval = call_qcsapi_wifi_get_avg_snr( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_BSSID:
		statval = call_qcsapi_wifi_get_BSSID( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_config_BSSID:
		statval = call_qcsapi_wifi_get_config_BSSID( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_ssid_get_bssid:
		statval = call_qcsapi_wifi_ssid_get_bssid( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_ssid_set_bssid:
		statval = call_qcsapi_wifi_ssid_set_bssid( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_SSID:
		statval = call_qcsapi_wifi_get_SSID( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_set_SSID:
		statval = call_qcsapi_wifi_set_SSID( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_channel:
		statval = call_qcsapi_wifi_get_channel( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_set_channel:
		statval = call_qcsapi_wifi_set_channel( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_auto_channel:
		statval = call_qcsapi_wifi_get_auto_channel( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_set_auto_channel:
		statval = call_qcsapi_wifi_set_auto_channel( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_standard:
		statval = call_qcsapi_wifi_get_standard( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_dtim:
		statval = call_qcsapi_wifi_get_dtim( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_set_dtim:
		statval = call_qcsapi_wifi_set_dtim( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_assoc_limit:
		statval = call_qcsapi_wifi_get_assoc_limit( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_set_assoc_limit:
		statval = call_qcsapi_wifi_set_assoc_limit( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_bss_assoc_limit:
		statval = call_qcsapi_wifi_get_bss_assoc_limit( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_set_bss_assoc_limit:
		statval = call_qcsapi_wifi_set_bss_assoc_limit( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_interface_get_status:
		statval = call_qcsapi_interface_get_status( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_interface_get_ip4:
		statval = call_qcsapi_interface_get_ip4( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_pm_get_counter:
		statval = call_qcsapi_pm_get_counter( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_pm_get_elapsed_time:
		statval = call_qcsapi_pm_get_elapsed_time( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_interface_set_ip4:
		statval = call_qcsapi_interface_set_ip4( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_list_channels:
		statval = call_qcsapi_wifi_get_list_channels( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_mode_switch:
		statval = call_qcsapi_wifi_get_mode_switch( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_option:
		statval = call_qcsapi_wifi_get_option( p_calling_bundle, argc, argv );
		break;

	 case e_qcsapi_get_board_parameter:
		statval = call_qcsapi_get_board_parameter( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_set_option:
		statval = call_qcsapi_wifi_set_option( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_rates:
		statval = call_qcsapi_wifi_get_rates( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_set_rates:
		statval = call_qcsapi_wifi_set_rates( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_max_bitrate:
		statval = call_qcsapi_wifi_get_max_bitrate( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_set_max_bitrate:
		statval = call_qcsapi_wifi_set_max_bitrate( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_beacon_type:
		statval = call_qcsapi_wifi_get_beacon_type( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_set_beacon_type:
		statval = call_qcsapi_wifi_set_beacon_type( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_beacon_interval:
		statval = call_qcsapi_wifi_get_beacon_interval(  p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_set_beacon_interval:
		statval = call_qcsapi_wifi_set_beacon_interval(  p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_list_regulatory_regions:
		statval = call_qcsapi_wifi_get_list_regulatory_regions( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_regulatory_tx_power:
		statval = call_qcsapi_wifi_get_regulatory_tx_power( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_configured_tx_power:
		statval = call_qcsapi_wifi_get_configured_tx_power( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_set_regulatory_channel:
		statval = call_qcsapi_wifi_set_regulatory_channel( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_set_regulatory_region:
		statval = call_qcsapi_wifi_set_regulatory_region( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_restore_regulatory_tx_power:
		statval = call_qcsapi_wifi_restore_regulatory_tx_power( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_regulatory_region:
		statval = call_qcsapi_wifi_get_regulatory_region( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_overwrite_country_code:
		statval = call_qcsapi_wifi_overwrite_country_code( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_list_regulatory_channels:
		statval = call_qcsapi_wifi_get_list_regulatory_channels( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_list_regulatory_bands:
		statval = call_qcsapi_wifi_get_list_regulatory_bands( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_regulatory_db_version:
		statval = call_qcsapi_wifi_get_regulatory_db_version( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_set_regulatory_tx_power_cap:
		statval = call_qcsapi_wifi_set_regulatory_tx_power_cap( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_set_chan_pri_inactive:
		statval = call_qcsapi_wifi_set_chan_pri_inactive( p_calling_bundle, argc, argv );
		break;
	  case e_qcsapi_wifi_set_chan_disabled:
		statval = call_qcsapi_wifi_set_chan_disabled( p_calling_bundle, argc, argv );
		break;
	  case e_qcsapi_wifi_get_chan_disabled:
		statval = call_qcsapi_wifi_get_chan_disabled( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_tx_power:
		statval = call_qcsapi_wifi_get_tx_power( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_set_tx_power:
		statval = call_qcsapi_wifi_set_tx_power( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_tx_power_ext:
		statval = call_qcsapi_wifi_get_tx_power_ext( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_set_tx_power_ext:
		statval = call_qcsapi_wifi_set_tx_power_ext( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_chan_power_table:
		statval = call_qcsapi_wifi_get_chan_power_table( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_set_chan_power_table:
		statval = call_qcsapi_wifi_set_chan_power_table( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_bw_power:
		statval = call_qcsapi_wifi_get_bw_power( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_set_bw_power:
		statval = call_qcsapi_wifi_set_bw_power( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_bf_power:
		statval = call_qcsapi_wifi_get_bf_power( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_set_bf_power:
		statval = call_qcsapi_wifi_set_bf_power( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_power_selection:
		statval = call_qcsapi_wifi_get_power_selection( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_set_power_selection:
		statval = call_qcsapi_wifi_set_power_selection( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_carrier_interference:
		statval = call_qcsapi_wifi_get_carrier_interference( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_congestion_idx:
		statval = call_qcsapi_wifi_get_congestion_idx( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_supported_tx_power_levels:
		statval = call_qcsapi_wifi_get_supported_tx_power_levels( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_current_tx_power_level:
		statval = call_qcsapi_wifi_get_current_tx_power_level( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_set_power_constraint:
		statval = call_qcsapi_wifi_set_power_constraint( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_power_constraint:
		statval = call_qcsapi_wifi_get_power_constraint( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_set_tpc_interval:
		statval = call_qcsapi_wifi_set_tpc_interval( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_tpc_interval:
		statval = call_qcsapi_wifi_get_tpc_interval( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_assoc_records:
		statval = call_qcsapi_wifi_get_assoc_records(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_wifi_get_list_DFS_channels:
		statval = call_qcsapi_wifi_get_list_DFS_channels( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_is_channel_DFS:
		statval = call_qcsapi_wifi_is_channel_DFS( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_DFS_alt_channel:
		statval = call_qcsapi_wifi_get_DFS_alt_channel( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_set_DFS_alt_channel:
		statval = call_qcsapi_wifi_set_DFS_alt_channel( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_set_DFS_reentry:
		statval = call_qcsapi_wifi_set_dfs_reentry( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_scs_cce_channels:
		statval = call_qcsapi_wifi_get_scs_cce_channels( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_dfs_cce_channels:
		statval = call_qcsapi_wifi_get_dfs_cce_channels( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_csw_records:
		statval = call_qcsapi_wifi_get_csw_records( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_radar_status:
		statval = call_qcsapi_wifi_get_radar_status( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_WEP_encryption_level:
		statval = call_qcsapi_wifi_get_WEP_encryption_level( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_WPA_encryption_modes:
		statval = call_qcsapi_wifi_get_WPA_encryption_modes( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_set_WPA_encryption_modes:
		statval = call_qcsapi_wifi_set_WPA_encryption_modes( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_WPA_authentication_mode:
		statval = call_qcsapi_wifi_get_WPA_authentication_mode( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_set_WPA_authentication_mode:
		statval = call_qcsapi_wifi_set_WPA_authentication_mode( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_interworking:
		statval = call_qcsapi_wifi_get_interworking( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_set_interworking:
		statval = call_qcsapi_wifi_set_interworking( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_80211u_params:
		statval = call_qcsapi_wifi_get_80211u_params( p_calling_bundle, argc, argv );
		 break;

	  case e_qcsapi_wifi_set_80211u_params:
		statval = call_qcsapi_wifi_set_80211u_params( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_security_get_nai_realms:
		statval = call_qcsapi_security_get_nai_realms( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_security_add_nai_realm:
		statval = call_qcsapi_security_add_nai_realm( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_security_del_nai_realm:
		statval = call_qcsapi_security_del_nai_realm( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_security_add_roaming_consortium:
		statval = call_qcsapi_security_add_roaming_consortium( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_security_del_roaming_consortium:
		statval = call_qcsapi_security_del_roaming_consortium( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_security_get_roaming_consortium:
		statval = call_qcsapi_security_get_roaming_consortium( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_security_get_venue_name:
		statval = call_qcsapi_security_get_venue_name( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_security_add_venue_name:
		statval = call_qcsapi_security_add_venue_name( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_security_del_venue_name:
		statval = call_qcsapi_security_del_venue_name( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_security_get_oper_friendly_name:
		statval = call_qcsapi_security_get_oper_friendly_name( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_security_add_oper_friendly_name:
		statval = call_qcsapi_security_add_oper_friendly_name( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_security_del_oper_friendly_name:
		statval = call_qcsapi_security_del_oper_friendly_name( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_security_get_hs20_conn_capab:
		statval = call_qcsapi_security_get_hs20_conn_capab( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_security_add_hs20_conn_capab:
		statval = call_qcsapi_security_add_hs20_conn_capab( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_security_del_hs20_conn_capab:
		statval = call_qcsapi_security_del_hs20_conn_capab( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_hs20_status:
		statval = call_qcsapi_wifi_get_hs20_status( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_set_hs20_status:
		statval = call_qcsapi_wifi_set_hs20_status( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_hs20_params:
		statval = call_qcsapi_wifi_get_hs20_params( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_set_hs20_params:
		statval = call_qcsapi_wifi_set_hs20_params( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_remove_11u_param:
		statval = call_qcsapi_remove_11u_param( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_remove_hs20_param:
		statval = call_qcsapi_remove_hs20_param( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_set_proxy_arp:
		statval = call_qcsapi_wifi_set_proxy_arp( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_proxy_arp:
		statval = call_qcsapi_wifi_get_proxy_arp( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_l2_ext_filter:
		statval = call_qcsapi_wifi_get_l2_ext_filter( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_set_l2_ext_filter:
		statval = call_qcsapi_wifi_set_l2_ext_filter( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_IEEE11i_encryption_modes:
		statval = call_qcsapi_wifi_get_IEEE11i_encryption_modes( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_set_IEEE11i_encryption_modes:
		statval = call_qcsapi_wifi_set_IEEE11i_encryption_modes( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_IEEE11i_authentication_mode:
		statval = call_qcsapi_wifi_get_IEEE11i_authentication_mode( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_set_IEEE11i_authentication_mode:
		statval = call_qcsapi_wifi_set_IEEE11i_authentication_mode( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_michael_errcnt:
		statval = call_qcsapi_wifi_get_michael_errcnt( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_pre_shared_key:
		statval = call_qcsapi_wifi_get_pre_shared_key( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_set_pre_shared_key:
		statval = call_qcsapi_wifi_set_pre_shared_key( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_psk_auth_failures:
		statval = call_qcsapi_wifi_get_psk_auth_failures( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_key_passphrase:
		statval = call_qcsapi_wifi_get_key_passphrase( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_set_key_passphrase:
		statval = call_qcsapi_wifi_set_key_passphrase( p_calling_bundle, argc, argv );
		break;

          case e_qcsapi_wifi_get_group_key_interval:
                statval = call_qcsapi_wifi_get_group_key_interval( p_calling_bundle, argc, argv );
                break;

	  case e_qcsapi_wifi_set_group_key_interval:
                statval = call_qcsapi_wifi_set_group_key_interval( p_calling_bundle, argc, argv );
                break;

	  case e_qcsapi_wifi_get_pmf:
		statval = call_qcsapi_wifi_get_pmf( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_set_pmf:
		statval = call_qcsapi_wifi_set_pmf( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_SSID_get_wps_SSID:
		statval = call_qcsapi_SSID_get_wps_SSID( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_vlan_config:
		statval = call_qcsapi_wifi_vlan_config( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_show_vlan_config:
		statval = call_qcsapi_wifi_show_vlan_config( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_enable_vlan_pass_through:
		statval = call_qcsapi_enable_wlan_pass_through( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_br_vlan_promisc:
		statval = call_qcsapi_enable_vlan_promisc( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_add_ipff:
		statval = call_qcsapi_set_ipff( p_calling_bundle, 1, argc, argv );
		break;

	  case e_qcsapi_del_ipff:
		statval = call_qcsapi_set_ipff( p_calling_bundle, 0, argc, argv );
		break;

	  case e_qcsapi_get_ipff:
		statval = call_qcsapi_get_ipff( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_rts_threshold:
		statval = call_qcsapi_wifi_get_rts_threshold( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_set_rts_threshold:
		statval = call_qcsapi_wifi_set_rts_threshold( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_mac_address_filtering:
		statval = call_qcsapi_wifi_get_mac_address_filtering( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_set_mac_address_filtering:
		statval = call_qcsapi_wifi_set_mac_address_filtering( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_is_mac_address_authorized:
		statval = call_qcsapi_wifi_is_mac_address_authorized( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_authorized_mac_addresses:
		statval = call_qcsapi_wifi_get_authorized_mac_addresses( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_denied_mac_addresses:
		statval = call_qcsapi_wifi_get_denied_mac_addresses( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_authorize_mac_address:
		statval = call_qcsapi_wifi_authorize_mac_address( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_deny_mac_address:
		statval = call_qcsapi_wifi_deny_mac_address( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_remove_mac_address:
		statval = call_qcsapi_wifi_remove_mac_address( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_clear_mac_address_filters:
		statval = call_qcsapi_wifi_clear_mac_address_filters( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_set_mac_address_reserve:
		statval = call_qcsapi_wifi_set_mac_address_reserve( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_mac_address_reserve:
		statval = call_qcsapi_wifi_get_mac_address_reserve( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_clear_mac_address_reserve:
		statval = call_qcsapi_wifi_clear_mac_address_reserve( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_backoff_fail_max:
		statval = call_qcsapi_wifi_backoff_fail_max( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_backoff_timeout:
		statval = call_qcsapi_wifi_backoff_timeout( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wps_registrar_report_button_press:
		statval = call_qcsapi_wps_registrar_report_button_press( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wps_registrar_report_pin:
		statval = call_qcsapi_wps_registrar_report_pin( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wps_registrar_get_pp_devname:
		statval = call_qcsapi_wps_registrar_get_pp_devname( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wps_registrar_set_pp_devname:
		statval = call_qcsapi_wps_registrar_set_pp_devname( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wps_enrollee_report_button_press:
		statval = call_qcsapi_wps_enrollee_report_button_press( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wps_enrollee_report_pin:
		statval = call_qcsapi_wps_enrollee_report_pin( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wps_enrollee_generate_pin:
		statval = call_qcsapi_wps_enrollee_generate_pin( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wps_get_sta_pin:
		statval = call_qcsapi_wps_generate_random_pin( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wps_get_ap_pin:
		statval = call_qcsapi_wps_get_ap_pin( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wps_set_ap_pin:
		statval = call_qcsapi_wps_set_ap_pin( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wps_save_ap_pin:
		statval = call_qcsapi_wps_save_ap_pin( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wps_enable_ap_pin:
		statval = call_qcsapi_wps_enable_ap_pin( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wps_get_state:
		statval = call_qcsapi_wps_get_state( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wps_get_configured_state:
		statval = call_qcsapi_wps_get_configured_state( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wps_set_configured_state:
		statval = call_qcsapi_wps_set_configured_state( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wps_get_runtime_state:
		statval = call_qcsapi_wps_get_runtime_state( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wps_allow_pbc_overlap:
		statval = call_qcsapi_wps_allow_pbc_overlap( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wps_get_allow_pbc_overlap_status:
		statval = call_qcsapi_wps_get_allow_pbc_overlap_status( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wps_get_param:
		statval = call_qcsapi_wps_get_param( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wps_set_param:
		statval = call_qcsapi_wps_set_param( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wps_set_access_control:
		statval = call_qcsapi_wps_set_access_control( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wps_get_access_control:
		statval = call_qcsapi_wps_get_access_control( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_non_wps_set_pp_enable:
		statval = call_qcsapi_non_wps_set_pp_enable( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_non_wps_get_pp_enable:
		statval = call_qcsapi_non_wps_get_pp_enable( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wps_cancel:
		statval = call_qcsapi_wps_cancel(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_wps_set_pbc_in_srcm:
		statval = call_qcsapi_wps_set_pbc_in_srcm(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_wps_get_pbc_in_srcm:
		statval = call_qcsapi_wps_get_pbc_in_srcm(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_wps_timeout:
		statval = call_qcsapi_wps_set_timeout(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_wps_on_hidden_ssid:
		statval = call_qcsapi_wps_on_hidden_ssid(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_wps_on_hidden_ssid_status:
		statval = call_qcsapi_wps_on_hidden_ssid_status(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_wps_upnp_enable:
		statval = call_qcsapi_wps_upnp_enable(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_wps_upnp_status:
		statval = call_qcsapi_wps_upnp_status(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_wps_registrar_set_dfl_pbc_bss:
		statval = call_qcsapi_wps_registrar_set_dfl_pbc_bss( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wps_registrar_get_dfl_pbc_bss:
		statval = call_qcsapi_wps_registrar_get_dfl_pbc_bss( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_wpa_status:
		statval = call_qcsapi_wifi_get_wpa_status( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_auth_state:
		statval = call_qcsapi_wifi_get_auth_state( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_disconn_info:
		statval = call_qcsapi_wifi_get_disconn_info( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_reset_disconn_info:
		statval = call_qcsapi_wifi_reset_disconn_info( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_set_dwell_times:
		statval = call_qcsapi_wifi_set_dwell_times( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_dwell_times:
		statval = call_qcsapi_wifi_get_dwell_times( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_set_bgscan_dwell_times:
		statval = call_qcsapi_wifi_set_bgscan_dwell_times( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_bgscan_dwell_times:
		statval = call_qcsapi_wifi_get_bgscan_dwell_times( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_count_associations:
		statval = call_qcsapi_wifi_get_count_associations( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_associated_device_mac_addr:
		statval = call_qcsapi_wifi_get_associated_device_mac_addr( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_associated_device_ip_addr:
		statval = call_qcsapi_wifi_get_associated_device_ip_addr(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_wifi_get_link_quality:
		statval = call_qcsapi_wifi_get_link_quality( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_rssi_per_association:
		statval = call_qcsapi_wifi_get_rssi_per_association( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_rssi_in_dbm_per_association:
		statval = call_qcsapi_wifi_get_rssi_in_dbm_per_association( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_snr_per_association:
		statval = call_qcsapi_wifi_get_snr_per_association( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_hw_noise_per_association:
		statval = call_qcsapi_wifi_get_hw_noise_per_association( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_rx_bytes_per_association:
		statval = call_qcsapi_wifi_get_rx_bytes_per_association( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_tx_bytes_per_association:
		statval = call_qcsapi_wifi_get_tx_bytes_per_association( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_rx_packets_per_association:
		statval = call_qcsapi_wifi_get_rx_packets_per_association( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_tx_packets_per_association:
		statval = call_qcsapi_wifi_get_tx_packets_per_association( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_tx_err_packets_per_association:
		statval = call_qcsapi_wifi_get_tx_err_packets_per_association( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_bw_per_association:
		statval = call_qcsapi_wifi_get_bw_per_association( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_tx_phy_rate_per_association:
		call_qcsapi_wifi_get_tx_phy_rate_per_association(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_wifi_get_rx_phy_rate_per_association:
		call_qcsapi_wifi_get_rx_phy_rate_per_association(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_wifi_get_tx_mcs_per_association:
		call_qcsapi_wifi_get_tx_mcs_per_association(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_wifi_get_rx_mcs_per_association:
		call_qcsapi_wifi_get_rx_mcs_per_association(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_wifi_get_achievable_tx_phy_rate_per_association:
		call_qcsapi_wifi_get_achievable_tx_phy_rate_per_association( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_achievable_rx_phy_rate_per_association:
		call_qcsapi_wifi_get_achievable_rx_phy_rate_per_association( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_auth_enc_per_association:
		call_qcsapi_wifi_get_auth_enc_per_association( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_tput_caps:
		call_qcsapi_wifi_get_tput_caps(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_wifi_get_connection_mode:
		call_qcsapi_wifi_get_connection_mode(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_wifi_get_vendor_per_association:
		call_qcsapi_wifi_get_vendor_per_association( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_max_mimo:
		call_qcsapi_wifi_get_max_mimo( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_node_counter:
		statval = call_qcsapi_wifi_get_node_counter(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_wifi_get_node_param:
		statval = call_qcsapi_wifi_get_node_param(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_wifi_get_node_stats:
		statval = call_qcsapi_wifi_get_node_stats(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_wifi_get_max_queued:
		statval = call_qcsapi_wifi_get_max_queued(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_wifi_disassociate:
		statval = call_qcsapi_wifi_disassociate(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_wifi_disassociate_sta:
		statval = call_qcsapi_wifi_disassociate_sta(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_wifi_reassociate:
		statval = call_qcsapi_wifi_reassociate(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_wifi_associate:
		statval = call_qcsapi_wifi_associate(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_SSID_create_SSID:
		statval = call_qcsapi_SSID_create_SSID( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_SSID_remove_SSID:
		statval = call_qcsapi_SSID_remove_SSID( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_SSID_verify_SSID:
		statval = call_qcsapi_SSID_verify_SSID( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_SSID_rename_SSID:
		statval = call_qcsapi_SSID_rename_SSID( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_SSID_get_SSID_list:
		statval = call_qcsapi_SSID_get_SSID_list( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_SSID_get_protocol:
		statval = call_qcsapi_SSID_get_protocol( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_SSID_get_encryption_modes:
		statval = call_qcsapi_SSID_get_encryption_modes( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_SSID_get_group_encryption:
		statval = call_qcsapi_SSID_get_group_encryption( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_SSID_get_authentication_mode:
		statval = call_qcsapi_SSID_get_authentication_mode( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_SSID_set_protocol:
		statval = call_qcsapi_SSID_set_protocol( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_SSID_set_encryption_modes:
		statval = call_qcsapi_SSID_set_encryption_modes( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_SSID_set_group_encryption:
		statval = call_qcsapi_SSID_set_group_encryption( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_SSID_set_authentication_mode:
		statval = call_qcsapi_SSID_set_authentication_mode( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_SSID_get_pre_shared_key:
		statval = call_qcsapi_SSID_get_pre_shared_key( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_SSID_set_pre_shared_key:
		statval = call_qcsapi_SSID_set_pre_shared_key( p_calling_bundle, argc, argv );
		break;

	case e_qcsapi_wifi_add_radius_auth_server_cfg:
		statval = call_qcsapi_wifi_add_radius_auth_server_cfg( p_calling_bundle, argc, argv );
		break;

	case e_qcsapi_wifi_del_radius_auth_server_cfg:
		statval = call_qcsapi_wifi_del_radius_auth_server_cfg( p_calling_bundle, argc, argv );
		break;

	case e_qcsapi_wifi_get_radius_auth_server_cfg:
		statval = call_qcsapi_wifi_get_radius_auth_server_cfg( p_calling_bundle, argc, argv );
		break;

	case e_qcsapi_wifi_set_own_ip_addr:
		statval = call_qcsapi_wifi_set_own_ip_addr( p_calling_bundle, argc, argv );
		break;

	case e_qcsapi_SSID_get_key_passphrase:
		statval = call_qcsapi_SSID_get_key_passphrase( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_SSID_set_key_passphrase:
		statval = call_qcsapi_SSID_set_key_passphrase( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_SSID_get_pmf:
		statval = call_qcsapi_SSID_get_pmf( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_SSID_set_pmf:
		statval = call_qcsapi_SSID_set_pmf( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_start_scan:
		statval = call_qcsapi_wifi_start_scan(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_wifi_cancel_scan:
		statval = call_qcsapi_wifi_cancel_scan(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_wifi_get_scan_status:
		statval = call_qcsapi_wifi_get_scan_status(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_wifi_get_cac_status:
		statval = call_qcsapi_wifi_get_cac_status(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_wifi_wait_scan_completes:
		statval = call_qcsapi_wifi_wait_scan_completes(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_wifi_set_scan_chk_inv:
		statval = call_qcsapi_wifi_set_scan_chk_inv(p_calling_bundle, argc, argv);

		break;

	  case e_qcsapi_wifi_get_scan_chk_inv:
		statval = call_qcsapi_wifi_get_scan_chk_inv(p_calling_bundle, argc, argv);

		break;

	  case e_qcsapi_wifi_start_cca:
		statval = call_qcsapi_wifi_start_cca(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_wifi_disable_wps:
		statval = call_qcsapi_wifi_disable_wps(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_wifi_get_results_AP_scan:
		statval = call_qcsapi_wifi_get_results_AP_scan( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_count_APs_scanned:
		statval = call_qcsapi_wifi_get_count_APs_scanned( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_properties_AP:
		statval = call_qcsapi_wifi_get_properties_AP( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_mcs_rate:
		statval = call_qcsapi_wifi_get_mcs_rate( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_set_mcs_rate:
		statval = call_qcsapi_wifi_set_mcs_rate( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_time_associated_per_association:
		statval = call_qcsapi_wifi_get_time_associated_per_association( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_wds_add_peer:
		statval = call_qcsapi_wifi_wds_add_peer( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_wds_remove_peer:
		statval = call_qcsapi_wifi_wds_remove_peer( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_wds_get_peer_address:
		statval = call_qcsapi_wifi_wds_get_peer_address( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_wds_set_psk:
		statval = call_qcsapi_wifi_wds_set_psk( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_wds_set_mode:
		statval = call_qcsapi_wifi_wds_set_mode( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_wds_get_mode:
		statval = call_qcsapi_wifi_wds_get_mode( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_qos_get_param:
		statval = call_qcsapi_wifi_qos_get_param( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_qos_set_param:
		statval = call_qcsapi_wifi_qos_set_param( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_wmm_ac_map:
		statval = call_qcsapi_wifi_get_wmm_ac_map( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_set_wmm_ac_map:
		statval = call_qcsapi_wifi_set_wmm_ac_map( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_dscp_8021p_map:
		statval = call_qcsapi_wifi_get_dscp_8021p_map( p_calling_bundle, argc, argv );
		break;
	  case e_qcsapi_wifi_set_dscp_8021p_map:
		statval = call_qcsapi_wifi_set_dscp_8021p_map( p_calling_bundle, argc, argv );
		break;
	  case e_qcsapi_wifi_get_dscp_ac_map:
		statval = call_qcsapi_wifi_get_dscp_ac_map( p_calling_bundle, argc, argv );
		break;
	  case e_qcsapi_wifi_set_dscp_ac_map:
		statval = call_qcsapi_wifi_set_dscp_ac_map( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_priority:
		statval = call_qcsapi_wifi_get_priority( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_set_priority:
		statval = call_qcsapi_wifi_set_priority( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_airfair:
		statval = call_qcsapi_wifi_get_airfair( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_set_airfair:
		statval = call_qcsapi_wifi_set_airfair( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_config_get_parameter:
		statval = call_qcsapi_config_get_parameter( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_config_update_parameter:
		statval = call_qcsapi_config_update_parameter( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_config_get_ssid_parameter:
		statval = call_qcsapi_config_get_ssid_parameter( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_config_update_ssid_parameter:
		statval = call_qcsapi_config_update_ssid_parameter( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_service_control:
		statval = call_qcsapi_service_control(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_wfa_cert:
		statval = call_qcsapi_wfa_cert(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_wifi_enable_scs:
		statval = call_qcsapi_wifi_scs_enable(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_wifi_scs_switch_channel:
		statval = call_qcsapi_wifi_scs_switch_channel(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_wifi_set_scs_verbose:
		statval = call_qcsapi_wifi_set_scs_verbose(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_wifi_get_scs_status:
		statval = call_qcsapi_wifi_get_scs_status(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_wifi_set_scs_smpl_enable:
		statval = call_qcsapi_wifi_set_scs_smpl_enable(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_wifi_set_scs_smpl_dwell_time:
		statval = call_qcsapi_wifi_set_scs_smpl_dwell_time(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_wifi_set_scs_smpl_intv:
		statval = call_qcsapi_wifi_set_scs_sample_intv(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_wifi_set_scs_intf_detect_intv:
		statval = call_qcsapi_wifi_set_scs_intf_detect_intv(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_wifi_set_scs_thrshld:
		statval = call_qcsapi_wifi_set_scs_thrshld(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_wifi_set_scs_report_only:
		statval = call_qcsapi_wifi_set_scs_report_only(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_wifi_get_scs_report_stat:
		statval = call_qcsapi_wifi_get_scs_report(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_wifi_set_scs_cca_intf_smth_fctr:
		statval = call_qcsapi_wifi_set_scs_cca_intf_smth_fctr(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_wifi_set_scs_chan_mtrc_mrgn:
		statval = call_qcsapi_wifi_set_scs_chan_mtrc_mrgn(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_wifi_get_scs_dfs_reentry_request:
		statval = call_qcsapi_wifi_get_scs_dfs_reentry_request(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_wifi_get_scs_cca_intf:
		statval = call_qcsapi_wifi_get_scs_cca_intf( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_scs_param:
		statval = call_qcsapi_wifi_get_scs_param(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_wifi_set_scs_stats:
		statval = call_qcsapi_wifi_set_scs_stats(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_wifi_start_ocac:
		statval = call_qcsapi_wifi_start_ocac(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_wifi_stop_ocac:
		statval = call_qcsapi_wifi_stop_ocac(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_wifi_get_ocac_status:
		statval = call_qcsapi_wifi_get_ocac_status(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_wifi_set_ocac_threshold:
		statval = call_qcsapi_wifi_set_ocac_threshold(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_wifi_set_ocac_dwell_time:
		statval = call_qcsapi_wifi_set_ocac_dwell_time(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_wifi_set_ocac_duration:
		statval = call_qcsapi_wifi_set_ocac_duration(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_wifi_set_ocac_cac_time:
		statval = call_qcsapi_wifi_set_ocac_cac_time(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_wifi_set_ocac_report_only:
		statval = call_qcsapi_wifi_set_ocac_report_only(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_wifi_start_dfs_s_radio:
		statval = call_qcsapi_wifi_start_dfs_s_radio(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_wifi_stop_dfs_s_radio:
		statval = call_qcsapi_wifi_stop_dfs_s_radio(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_wifi_get_dfs_s_radio_status:
		statval = call_qcsapi_wifi_get_dfs_s_radio_status(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_wifi_get_dfs_s_radio_availability:
		statval = call_qcsapi_wifi_get_dfs_s_radio_availability(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_wifi_set_dfs_s_radio_threshold:
		statval = call_qcsapi_wifi_set_dfs_s_radio_threshold(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_wifi_set_dfs_s_radio_dwell_time:
		statval = call_qcsapi_wifi_set_dfs_s_radio_dwell_time(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_wifi_set_dfs_s_radio_duration:
		statval = call_qcsapi_wifi_set_dfs_s_radio_duration(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_wifi_set_dfs_s_radio_cac_time:
		statval = call_qcsapi_wifi_set_dfs_s_radio_cac_time(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_wifi_set_dfs_s_radio_report_only:
		statval = call_qcsapi_wifi_set_dfs_s_radio_report_only(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_wifi_set_dfs_s_radio_wea_duration:
		statval = call_qcsapi_wifi_set_dfs_s_radio_wea_duration(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_wifi_set_dfs_s_radio_wea_cac_time:
		statval = call_qcsapi_wifi_set_dfs_s_radio_wea_cac_time(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_wifi_set_vendor_fix:
		statval = call_qcsapi_wifi_set_vendor_fix( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_set_ap_isolate:
		statval = call_qcsapi_wifi_set_ap_isolate(p_calling_bundle, argc, argv);
		break;
	  case e_qcsapi_wifi_get_ap_isolate:
		statval = call_qcsapi_wifi_get_ap_isolate(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_power_save:
		statval = call_qcsapi_pm_get_set_mode(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_qpm_level:
		statval = call_qcsapi_qpm_get_level(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_get_interface_stats:
		statval = call_qcsapi_get_interface_stats( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_get_phy_stats:
		statval = call_qcsapi_get_phy_stats( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_bootcfg_get_parameter:
		statval = call_qcsapi_bootcfg_get_parameter( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_bootcfg_update_parameter:
		statval = call_qcsapi_bootcfg_update_parameter( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_bootcfg_commit:
		statval = call_qcsapi_bootcfg_commit( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_telnet_enable:
		statval = call_qcsapi_telnet_enable( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_restore_default_config:
		statval = call_qcsapi_restore_default_config( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_reset_all_stats:
		statval = call_qcsapi_reset_all_counters( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_run_script:
		statval = call_qcsapi_run_script( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_qtm:
		statval = call_qcsapi_vsp( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_pairing_id:
		statval = call_qcsapi_wifi_get_pairing_id( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_set_pairing_id:
		statval = call_qcsapi_wifi_set_pairing_id( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_pairing_enable:
		statval = call_qcsapi_wifi_get_pairing_enable( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_set_pairing_enable:
		statval = call_qcsapi_wifi_set_pairing_enable( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_set_txqos_sched_tbl:
		statval = call_qcsapi_wifi_set_txqos_sched_tbl( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_wifi_get_txqos_sched_tbl:
		statval = call_qcsapi_wifi_get_txqos_sched_tbl( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_eth_phy_power_off:
		statval = call_qcsapi_eth_phy_power_off( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_aspm_l1:
		statval = call_qcsapi_set_aspm_l1( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_l1:
		statval = call_qcsapi_set_l1( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_test_traffic:
		statval = call_qcsapi_test_traffic( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_get_temperature:
		statval = call_qcsapi_get_temperature( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_set_accept_oui_filter:
		statval = call_qcsapi_set_accept_oui_filter( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_get_accept_oui_filter:
		statval = call_qcsapi_get_accept_oui_filter( p_calling_bundle, argc, argv );
		break;

	  case e_qcsapi_get_swfeat_list:
		statval = call_qcsapi_get_swfeat_list( p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_wifi_set_vht:
		statval = call_qcsapi_wifi_set_vht( p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_wifi_get_vht:
		statval = call_qcsapi_wifi_get_vht( p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_calcmd_set_test_mode:
		statval = call_qcsapi_calcmd_set_test_mode(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_calcmd_show_test_packet:
		statval = call_qcsapi_calcmd_show_test_packet(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_calcmd_send_test_packet:
		statval = call_qcsapi_calcmd_send_test_packet(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_calcmd_stop_test_packet:
		statval = call_qcsapi_calcmd_stop_test_packet(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_calcmd_send_dc_cw_signal:
		statval = call_qcsapi_calcmd_send_dc_cw_signal(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_calcmd_stop_dc_cw_signal:
		statval = call_qcsapi_calcmd_stop_dc_cw_signal(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_calcmd_get_test_mode_antenna_sel:
		statval = call_qcsapi_calcmd_get_test_mode_antenna_sel(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_calcmd_get_test_mode_mcs:
		statval = call_qcsapi_calcmd_get_test_mode_mcs(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_calcmd_get_test_mode_bw:
		statval = call_qcsapi_calcmd_get_test_mode_bw(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_calcmd_get_tx_power:
		statval = call_qcsapi_calcmd_get_tx_power(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_calcmd_set_tx_power:
		statval = call_qcsapi_calcmd_set_tx_power(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_calcmd_get_test_mode_rssi:
		statval = call_qcsapi_calcmd_get_test_mode_rssi(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_calcmd_set_mac_filter:
		statval = call_qcsapi_calcmd_set_mac_filter(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_calcmd_get_antenna_count:
		statval = call_qcsapi_calcmd_get_antenna_count(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_calcmd_clear_counter:
		statval = call_qcsapi_calcmd_clear_counter(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_calcmd_get_info:
		statval = call_qcsapi_calcmd_get_info(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_set_soc_macaddr:
		statval = call_qcsapi_wifi_set_soc_macaddr(p_calling_bundle, argc, argv);
		break;

	  case e_qcsapi_wifi_enable_tdls:
		statval = call_qcsapi_wifi_enable_tdls(p_calling_bundle, argc, argv);
		break;
	  case e_qcsapi_wifi_enable_tdls_over_qhop:
		statval = call_qcsapi_wifi_enable_tdls_over_qhop(p_calling_bundle, argc, argv);
		break;
	case e_qcsapi_wifi_disable_dfs_channels:
		statval = call_qcsapi_disable_dfs_channels(p_calling_bundle, argc, argv);
		break;
	  case e_qcsapi_wifi_get_tdls_status:
		statval = call_qcsapi_wifi_get_tdls_status(p_calling_bundle, argc, argv);
		break;
	  case e_qcsapi_wifi_set_tdls_params:
		statval = call_qcsapi_wifi_set_tdls_params(p_calling_bundle, argc, argv);
		break;
	  case e_qcsapi_wifi_get_tdls_params:
		statval = call_qcsapi_wifi_get_tdls_params(p_calling_bundle, argc, argv);
		break;
	  case e_qcsapi_get_carrier_id:
		  statval = call_qcsapi_get_carrier_id( p_calling_bundle, argc, argv );
		break;
	  case e_qcsapi_set_carrier_id:
		  statval = call_qcsapi_set_carrier_id( p_calling_bundle, argc, argv );
		break;
	  case e_qcsapi_get_spinor_jedecid:
		statval = call_qcsapi_wifi_get_spinor_jedecid(p_calling_bundle, argc, argv);
		break;
	  case e_qcsapi_get_custom_value:
		statval = call_qcsapi_wifi_get_custom_value(p_calling_bundle, argc, argv);
		break;
	  case e_qcsapi_wifi_tdls_operate:
                statval = call_qcsapi_wifi_tdls_operate(p_calling_bundle, argc, argv);
                break;
	  case e_qcsapi_wifi_get_mlme_stats_per_mac:
		statval = call_qcsapi_wifi_get_mlme_stats_per_mac(p_calling_bundle, argc, argv);
		break;
	  case e_qcsapi_wifi_get_mlme_stats_per_association:
		statval = call_qcsapi_wifi_get_mlme_stats_per_association(p_calling_bundle, argc, argv);
		break;
	  case e_qcsapi_wifi_get_mlme_stats_macs_list:
		statval = call_qcsapi_wifi_get_mlme_stats_macs_list(p_calling_bundle, argc, argv);
		break;
	  case e_qcsapi_get_nss_cap:
		statval = call_qcsapi_wifi_get_nss_cap(p_calling_bundle, argc, argv);
		break;
	  case e_qcsapi_set_nss_cap:
		statval = call_qcsapi_wifi_set_nss_cap(p_calling_bundle, argc, argv);
		break;
	  case e_qcsapi_get_security_defer_mode:
		statval = call_qcsapi_wifi_get_security_defer_mode(p_calling_bundle, argc, argv);
		break;
	  case e_qcsapi_set_security_defer_mode:
		statval = call_qcsapi_wifi_set_security_defer_mode(p_calling_bundle, argc, argv);
		break;
	  case e_qcsapi_apply_security_config:
		statval = call_qcsapi_wifi_apply_security_config(p_calling_bundle, argc, argv);
		break;
	  case e_qcsapi_wifi_set_intra_bss_isolate:
		statval = call_qcsapi_wifi_set_intra_bss_isolate(p_calling_bundle, argc, argv);
		break;
	  case e_qcsapi_wifi_get_intra_bss_isolate:
		statval = call_qcsapi_wifi_get_intra_bss_isolate(p_calling_bundle, argc, argv);
		break;
	  case e_qcsapi_wifi_set_bss_isolate:
		statval = call_qcsapi_wifi_set_bss_isolate(p_calling_bundle, argc, argv);
		break;
	  case e_qcsapi_wifi_get_bss_isolate:
		statval = call_qcsapi_wifi_get_bss_isolate(p_calling_bundle, argc, argv);
		break;
	  case e_qcsapi_wowlan_host_state:
		statval = call_qcsapi_wifi_host_state_set(p_calling_bundle, argc, argv);
		break;
	  case e_qcsapi_wowlan_match_type:
		statval = call_qcsapi_wifi_wowlan_match_type_set(p_calling_bundle, argc, argv);
		break;
	  case e_qcsapi_wowlan_L2_type:
		statval = call_qcsapi_wifi_wowlan_L2_type_set(p_calling_bundle, argc, argv);
		break;
	  case e_qcsapi_wowlan_udp_port:
		statval = call_qcsapi_wifi_wowlan_udp_port_set(p_calling_bundle, argc, argv);
		break;
	  case e_qcsapi_wowlan_pattern:
		statval = call_qcsapi_wifi_wowlan_pattern_set(p_calling_bundle, argc, argv);
		break;
	  case e_qcsapi_wowlan_get_host_state:
		statval = call_qcsapi_wifi_host_state_get(p_calling_bundle, argc, argv);
		break;
	  case e_qcsapi_wowlan_get_match_type:
		statval = call_qcsapi_wifi_wowlan_match_type_get(p_calling_bundle, argc, argv);
		break;
	  case e_qcsapi_wowlan_get_L2_type:
		statval = call_qcsapi_wifi_wowlan_L2_type_get(p_calling_bundle, argc, argv);
		break;
	  case e_qcsapi_wowlan_get_udp_port:
		statval = call_qcsapi_wifi_wowlan_udp_port_get(p_calling_bundle, argc, argv);
		break;
	  case e_qcsapi_wowlan_get_pattern:
		statval = call_qcsapi_wifi_wowlan_pattern_get(p_calling_bundle, argc, argv);
		break;
	  case e_qcsapi_wifi_set_extender_params:
		statval = call_qcsapi_wifi_set_extender_params(p_calling_bundle,
			argc, argv);
		break;
	  case e_qcsapi_wifi_get_extender_status:
		statval = call_qcsapi_wifi_get_extender_status(p_calling_bundle,
			argc, argv);
		break;
	  case e_qcsapi_wifi_enable_bgscan:
		statval = call_qcsapi_wifi_enable_bgscan(p_calling_bundle,
			argc, argv);
		break;
	  case e_qcsapi_wifi_get_bgscan_status:
		statval = call_qcsapi_wifi_get_bgscan_status(p_calling_bundle,
			argc, argv);
		break;
	  case e_qcsapi_get_uboot_info:
		statval = call_qcsapi_get_uboot_info(p_calling_bundle, argc, argv);
		break;
	  case e_qcsapi_wifi_get_disassoc_reason:
		statval = call_qcsapi_wifi_get_disassoc_reason(p_calling_bundle, argc, argv);
		break;
	  case e_qcsapi_is_startprod_done:
		statval = call_qcsapi_is_startprod_done(p_calling_bundle, argc, argv);
		break;
	  case e_qcsapi_get_bb_param:
                statval = call_qcsapi_wifi_get_bb_param(p_calling_bundle, argc, argv);
                break;
	  case e_qcsapi_set_bb_param:
                statval = call_qcsapi_wifi_set_bb_param(p_calling_bundle, argc, argv);
		break;
	  case e_qcsapi_wifi_get_tx_amsdu:
		statval = call_qcsapi_wifi_get_tx_amsdu(p_calling_bundle, argc, argv);
		break;
	  case e_qcsapi_wifi_set_tx_amsdu:
		statval = call_qcsapi_wifi_set_tx_amsdu(p_calling_bundle, argc, argv);
		break;
	  case e_qcsapi_wifi_set_scan_buf_max_size:
		statval = call_qcsapi_wifi_set_scan_buf_max_size(p_calling_bundle, argc, argv);
		break;
	  case e_qcsapi_wifi_get_scan_buf_max_size:
		statval = call_qcsapi_wifi_get_scan_buf_max_size(p_calling_bundle, argc, argv);
		break;
	  case e_qcsapi_wifi_set_scan_table_max_len:
		statval = call_qcsapi_wifi_set_scan_table_max_len(p_calling_bundle, argc, argv);
		break;
	  case e_qcsapi_wifi_get_scan_table_max_len:
		statval = call_qcsapi_wifi_get_scan_table_max_len(p_calling_bundle, argc, argv);
		break;
	  case e_qcsapi_wifi_set_enable_mu:
		statval = call_qcsapi_wifi_set_enable_mu(p_calling_bundle, argc, argv);
		break;
	  case e_qcsapi_wifi_get_enable_mu:
		statval = call_qcsapi_wifi_get_enable_mu(p_calling_bundle, argc, argv);
		break;
	  case e_qcsapi_wifi_set_mu_use_precode:
		statval = call_qcsapi_wifi_set_mu_use_precode(p_calling_bundle, argc, argv);
		break;
	  case e_qcsapi_wifi_get_mu_use_precode:
		statval = call_qcsapi_wifi_get_mu_use_precode(p_calling_bundle, argc, argv);
		break;
	  case e_qcsapi_wifi_set_mu_use_eq:
		statval = call_qcsapi_wifi_set_mu_use_eq(p_calling_bundle, argc, argv);
		break;
	  case e_qcsapi_wifi_get_mu_use_eq:
		statval = call_qcsapi_wifi_get_mu_use_eq(p_calling_bundle, argc, argv);
		break;
	  case e_qcsapi_wifi_get_mu_groups:
		statval = call_qcsapi_wifi_get_mu_groups(p_calling_bundle, argc, argv);
		break;
	  case e_qcsapi_send_file:
		statval = call_qcsapi_send_file(p_calling_bundle, argc, argv);
		break;
	  case e_qcsapi_get_emac_switch:
		statval = call_qcsapi_get_emac_switch(p_calling_bundle, argc, argv);
		break;
	  case e_qcsapi_set_emac_switch:
		statval = call_qcsapi_set_emac_switch(p_calling_bundle, argc, argv);
		break;
	  case e_qcsapi_eth_dscp_map:
		statval = call_qcsapi_eth_dscp_map(p_calling_bundle, argc, argv);
		break;
	  case e_qcsapi_set_optim_stats:
                statval = call_qcsapi_wifi_set_optim_stats(p_calling_bundle, argc, argv);
		break;
	  case e_qcsapi_set_sys_time:
		statval = call_qcsapi_set_sys_time(p_calling_bundle, argc, argv);
		break;
	  case e_qcsapi_get_sys_time:
		statval = call_qcsapi_get_sys_time(p_calling_bundle, argc, argv);
		break;
	  case e_qcsapi_get_eth_info:
                statval = call_qcsapi_get_eth_info(p_calling_bundle, argc, argv);
		break;
	  case e_qcsapi_wifi_block_bss:
		statval = call_qcsapi_wifi_block_bss(p_calling_bundle, argc, argv);
		break;
	  case e_qcsapi_wifi_verify_repeater_mode:
		statval = call_qcsapi_wifi_verify_repeater_mode(p_calling_bundle, argc, argv);
		break;
	  case e_qcsapi_wifi_set_ap_interface_name:
		statval = call_qcsapi_wifi_set_ap_interface_name(p_calling_bundle, argc, argv);
		break;
	  case e_qcsapi_wifi_get_ap_interface_name:
		statval = call_qcsapi_wifi_get_ap_interface_name(p_calling_bundle, argc, argv);
		break;
	  default:
		print_out( print, "no interface program (yet) for QCS API enum %d\n", p_calling_bundle->caller_qcsapi );
	}

	return( statval );
}

static int
call_qcsapi(qcsapi_output *print, int argc, char *argv[] )
{
	qcsapi_entry_point		 e_the_entry_point = e_qcsapi_nosuch_api;
	int				 ok_to_continue = 1;
	int				 expected_argc = 1;
	call_qcsapi_bundle		 calling_bundle;
	const struct qcsapi_entry	*qcsapi_table_entry = NULL;
	int				 statval = 0;

	calling_bundle.caller_output = print;

  /*
   * Argument count (argc) required to be at least 1, the name of the QCS API to be called.
   */
	if (argc < 1)
	{
		print_out( print, "programming error in call_qcsapi, argc = %d\n", argc );
		ok_to_continue = 0;
	}

	if (ok_to_continue)
	{
		if (name_to_entry_point_enum( argv[ 0 ], &e_the_entry_point ) == 0)
		{
			print_out( print, "QCSAPI entry point %s not found\n", argv[ 0 ] );
			ok_to_continue = 0;
		}
	}
  /*
   * Selected QCSAPIs are NOT supported by call_qcsapi.
   */
	if (ok_to_continue)
	{
		if (e_the_entry_point == e_qcsapi_gpio_monitor_reset_device)
		{
			print_out( print, "GPIO monitor reset device cannot be accessed from call_qcsapi\n" );
			ok_to_continue = 0;
		}
	}

	if (ok_to_continue)
	{
		qcsapi_table_entry = entry_point_enum_to_table_entry( e_the_entry_point );

		if (qcsapi_table_entry == NULL)
		{
			print_out( print, "programming error in call_qcsapi, no entry for enum %d\n", (int) e_the_entry_point );
			ok_to_continue = 0;
		}
		else
		{
		  /*
		   * Originally all APIs expected an interface name.  Now a few APIs apply to the entire system,
		   * and thus do not require an interface name.  These new APIs are identified as get system value
		   * and set system value.  Older APIs are identified as get and set APIs.  They require an
		   * interface, which now needs to be accounted for here.  And set system value APIs will require
		   * an additional parameter, the new system-wide value.
		   *
		   * APIs that expect an additional parameter (counters, rates, etc.) require an additional parameter
		   * APIs that expect an SSID AND an index (SSID get passphrase) require yet another parameter
		   * APIs that SET a value require yet another parameter
		   *
		   * No interdependencies.
		   */
			if (qcsapi_table_entry->e_typeof_api == e_qcsapi_get_api ||
			    qcsapi_table_entry->e_typeof_api == e_qcsapi_set_api)
			  expected_argc++;						// account for the interface
			if (qcsapi_table_entry->e_generic_param_type != e_qcsapi_none)
			  expected_argc++;
			if (qcsapi_table_entry->e_generic_param_type == e_qcsapi_SSID_index)
			  expected_argc++;
			if (qcsapi_table_entry->e_typeof_api == e_qcsapi_set_api ||
			    qcsapi_table_entry->e_typeof_api == e_qcsapi_set_system_value)
			  expected_argc++;
			if (qcsapi_table_entry->e_typeof_api == e_qcsapi_set_api_without_parameter)
				expected_argc++;

			if (expected_argc > argc)
			{
				print_out( print,
			   "Too few command line parameters in call_qcsapi, expected %d, found %d\n", expected_argc, argc
				);
				ok_to_continue = 0;
			}
		}

		if (ok_to_continue)
		{
		  /* Eliminate the QCS API name from the argument list. */

			argc--;
			argv++;

		  /* Begin filling in the calling bundle ... */

			calling_bundle.caller_qcsapi = e_the_entry_point;

			if (qcsapi_table_entry->e_typeof_api == e_qcsapi_get_api ||
			    qcsapi_table_entry->e_typeof_api == e_qcsapi_set_api ||
				qcsapi_table_entry->e_typeof_api == e_qcsapi_set_api_without_parameter)
			{
				calling_bundle.caller_interface = argv[ 0 ];
				argc--;
				argv++;
			}
			else
			  calling_bundle.caller_interface = NULL;

			calling_bundle.caller_generic_parameter.generic_parameter_type = qcsapi_table_entry->e_generic_param_type;
		}
	}

	if (ok_to_continue)
	{
		if (calling_bundle.caller_generic_parameter.generic_parameter_type != e_qcsapi_none)
		{
		  /* Again we checked previously that enough arguments were present ... */

			if (parse_generic_parameter_name(print, argv[ 0 ], &(calling_bundle.caller_generic_parameter)) == 0)
			  ok_to_continue = 0;
			else
			{
			  /* And remove the parameter name from the argument list. */

			argc--;
			argv++;
			}
		}
	}

	if (ok_to_continue)
	{
		unsigned int	iter;

		if (verbose_flag > 0)
		{
			print_out( print, "call QCSAPI: %s", entry_point_enum_to_name( calling_bundle.caller_qcsapi ) );

			if (qcsapi_table_entry->e_typeof_api == e_qcsapi_get_api ||
			    qcsapi_table_entry->e_typeof_api == e_qcsapi_set_api ||
				qcsapi_table_entry->e_typeof_api == e_qcsapi_set_api_without_parameter)
			{
				print_out( print, " %s", calling_bundle.caller_interface );
			}

			if (calling_bundle.caller_generic_parameter.generic_parameter_type != e_qcsapi_none)
			{
				print_out( print, " " );
				dump_generic_parameter_name(print, &(calling_bundle.caller_generic_parameter) );
			}

			if (argc > 0)
			{
				for (iter = 0; iter < argc; iter++)
				  print_out( print, " %s", argv[ iter ] );
			}

			print_out( print, "\n" );
		}

		if (call_qcsapi_init_count > 0)
		{
			if (call_qcsapi_init_count == 1)
			  qcsapi_init();
			else
			{
				for (iter = 0; iter < call_qcsapi_init_count; iter++)
				  qcsapi_init();
			}
		}

		if (call_count < 2) {
			statval = call_particular_qcsapi( &calling_bundle, argc, argv );
		} else {
			for (iter = 0; iter < call_count - 1; iter++) {
				call_particular_qcsapi( &calling_bundle, argc, argv );
				if (delay_time > 0) {
					sleep( delay_time );
				}
			}

			call_particular_qcsapi( &calling_bundle, argc, argv );
		}
	}

	return( statval );
}

static int
process_options(qcsapi_output *print, int argc, char **argv)
{
	int	local_index = 0;

	while (local_index < argc && *(argv[ local_index ]) == '-')
	{
		char		*option_arg = argv[ local_index ];
		unsigned int	 length_option = strlen( option_arg );

		if (length_option > 1)
		{
			char	option_letter = option_arg[ 1 ];

			if (option_letter == 'v')
			{
				unsigned int	index_2 = 1;

				while (option_arg[ index_2 ] == 'v')
				{
					verbose_flag++;
					index_2++;
				}
			}
			else if (option_letter == 'q')
			{
				unsigned int	index_2 = 1;

				while (option_arg[ index_2 ] == 'q')
				{
					verbose_flag--;
					index_2++;
				}
			}
		  /*
		   * Process all options that require a numeric (integer) value here.
		   */
			else if (option_letter == 'n' || option_letter == 'd' || option_letter == 'i')
			{
				char	*local_addr = NULL;

				if (length_option > 2)
				{
					local_addr = option_arg + 2;
				}
				else
				{
					if (local_index + 1 >= argc)
					{
						print_err( print, "Missing numeric value for %c option\n", option_letter );
					}
					else
					{
						local_index++;
						local_addr = argv[ local_index ];
					}

				}

				if (local_addr != NULL)
				{
					int	min_value = 1;
					int	local_value = atoi( local_addr );
				  /*
				   * Most options require a numeric value to be greater than 0.  'i' is an exception.
				   */
					if (option_letter == 'i')
					  min_value = 0;

					if (local_value < min_value)
					{
						print_err( print,
							"Invalid numeric value %d for %c option\n",
							local_value, option_letter);
						return -EINVAL;
					}
					else
					{
						if (option_letter == 'n')
						  call_count = (unsigned int) local_value;
						else if (option_letter == 'i')
						  call_qcsapi_init_count = (unsigned int) local_value;
						else
						  delay_time = (unsigned int) local_value;
					}
				}
			  /*
			   * Error causing local_addr to be NULL has already been reported.
			   */
			}
			else if (option_letter == 'h')
			{
				if (local_index + 1 >= argc)
				{
					list_entry_point_names(print);
				}
				else
				{
					char	*local_addr = NULL;

					local_index++;
					local_addr = argv[ local_index ];

					if (strcasecmp( local_addr, "options" ) == 0)
					  list_option_names(print);
					else if (strcasecmp( local_addr, "entry_points" ) == 0)
					  list_entry_point_names(print);
					else if (strcasecmp( local_addr, "counters" ) == 0)
					  list_counter_names(print);
					else if (strcasecmp( local_addr, "per_node_params" ) == 0)
					  list_per_node_param_names(print);
					else if (strcasecmp( local_addr, "board_parameters" ) == 0)
					  list_board_parameter_names(print);
					else {
						print_err(print, "Unrecognized help option %s\n", local_addr );
						print_err(print, "Choose from 'entry_points', 'counters', 'options', 'per_node_params', or 'board_parameters'\n");
					}
				}

				return -EINVAL;
			}
			else if (option_letter == 'g')
			{
				char *reg;

				if (local_index + 1 >= argc)
				{
					return -EINVAL;
				}

				reg = argv[ ++local_index ];

				grep_entry_point_names(print, reg);

				return -EINVAL;
			}
			else if (option_letter == 'f')
			{
				if (local_index + 1 >= argc)
				{
					print_err( print, "Missing numeric value for %c option\n", option_letter );
				}
				else
				{
					char	*local_addr = NULL;

					local_index++;
					local_addr = argv[ local_index ];

					if (strcmp( "force_NULL", local_addr ) == 0)
					{
						internal_flags |= m_force_NULL_address;
					}
					else
					{
						print_err( print, "Unrecognized parameter %s for %c option\n",
								local_addr, option_letter);
					}
				}
			}
			else if (option_letter == 'u')
			{
				qcsapi_sem_disable();
			}
			else
			{
				print_out( print, "unrecognized option '%c'\n", option_letter );
			}
		}
	  /*
	   * Control would take the non-existent else clause if the argument were just "-".
	   */
		local_index++;
	}

	if( verbose_flag > 1)
	{
		print_out( print, "Verbose flag: %d, call count: %u\n", verbose_flag, call_count );
	}

	return( local_index );
}

static void
call_qscapi_help(qcsapi_output *print)
{
	print_out( print, "Usage:\n" );
	print_out( print, "    To get a parameter value: call_qcsapi <QCS API> <interface>\n" );
	print_out( print, "                              call_qcsapi <QCS API> <interface> <type of parameter>\n" );
	print_out( print, "    To set a parameter: call_qcsapi <QCS API> <interface> <parameter value>\n" );
	print_out( print, "                        call_qcsapi <QCS API> <interface> <type of parameter> <parameter value>\n" );
}

int
qcsapi_main(qcsapi_output *print, int argc, char **argv)
{
	int ival;
	int exitval = 0;

	if (argc <= 1) {
		call_qscapi_help(print);
		return -EINVAL;
	}

	argc--;
	argv++;

	ival = process_options(print, argc, argv);
	if (ival < 0) {
		exitval = ival;
	} else {
		argc = argc - ival;
		argv += ival;

		exitval = call_qcsapi(print, argc, argv);
	}

	return exitval;
}

