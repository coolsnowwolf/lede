'use strict';

import * as libuci from 'uci';
import { md5 } from 'digest';
import * as fs from 'fs';

import { append, append_raw, append_value, append_vars, append_list, append_string_vars, comment, push_config, set_default, touch_file } from 'wifi.common';
import * as netifd from 'wifi.netifd';
import * as iface from 'wifi.iface';

function iface_setup(config, phy, num_global_macaddr, macaddr_base) {
	switch(config.fixup) {
	case 'owe':
		config.ignore_broadcast_ssid = true;
		config.ssid = config.ssid + 'OWE';
		break;

	case 'owe-transition':
		let ifname = config.ifname;
		config.ifname = config.owe_transition_ifname;
		config.owe_transition_ifname = ifname;
		config.owe_transition_ssid = config.ssid + 'OWE';
		config.encryption = 'none';
		config.ignore_broadcast_ssid = false;
		/* the transition BSS needs its own address, not the main BSS's */
		config.macaddr = null;
		delete config.default_macaddr;
		delete config.random_macaddr;
		iface.prepare(config, phy, num_global_macaddr, macaddr_base);
		break;
	}

	comment('Setup interface: ' + config.ifname);

	config.bridge = config.network_bridge;
	config.snoop_iface = config.network_ifname;
	if (!config.wds)
		config.wds_bridge = null;
	else
		config.wds_sta = true;

	if (!config.idx)
		append('interface', config.ifname);
	else
		append('bss', config.ifname);

	if (config.multicast_to_unicast || config.proxy_arp)
		config.ap_isolate = 1;

	if (config.proxy_arp)
		set_default(config, 'na_mcast_to_ucast', true);

	append('bssid', config.macaddr);
	config.ssid2 = config.ssid;
	config.wmm_enabled = 1;
	append_string_vars(config, [ 'ssid2' ]);

	/* vendor_elements is a single concatenated hex blob, not one per line */
	if (type(config.vendor_elements) == 'array')
		config.vendor_elements = join('', config.vendor_elements);

	append_vars(config, [
		'ctrl_interface', 'ap_isolate', 'max_num_sta', 'ap_max_inactivity', 'airtime_bss_weight',
		'airtime_bss_limit', 'airtime_sta_weight', 'bss_load_update_period', 'chan_util_avg_period',
		'disassoc_low_ack', 'skip_inactivity_poll', 'ignore_broadcast_ssid', 'uapsd_advertisement_enabled',
		'utf8_ssid', 'multi_ap', 'multi_ap_vlanid', 'multi_ap_profile', 'tdls_prohibit', 'bridge',
		'wds_sta', 'wds_bridge', 'snoop_iface', 'vendor_elements', 'nas_identifier', 'radius_acct_interim_interval',
		'ocv', 'spp_amsdu', 'multicast_to_unicast', 'preamble', 'proxy_arp', 'per_sta_vif', 'mbo',
		'bss_transition', 'wnm_sleep_mode', 'wnm_sleep_mode_no_keys', 'qos_map_set', 'max_listen_int',
		'dtim_period', 'wmm_enabled', 'start_disabled', 'na_mcast_to_ucast', 'no_probe_resp_if_max_sta',
	]);
}

function iface_authentication_server(config) {
	for (let server in config.auth_server_addr) {
		append('auth_server_addr', server);
		append_vars(config, [ 'auth_server_port', 'auth_server_shared_secret' ]);
	}

	append_list(config, [ 'radius_auth_req_attr' ]);
}

function iface_accounting_server(config) {
	for (let server in config.acct_server_addr) {
		append('acct_server_addr', server);
		append_vars(config, [ 'acct_server_port', 'acct_server_shared_secret' ]);
	}

	append_list(config, [ 'radius_acct_req_attr' ]);
}

function iface_auth_type(config, band) {
	if (config.auth_type in [ 'sae', 'owe', 'eap2', 'eap192', 'dpp' ])
		config.ieee80211w = 2;

	if (config.auth_type in [ 'psk-sae', 'eap-eap2' ])
		set_default(config, 'ieee80211w', 1);

	if (config.auth_type == 'psk-sae-compat') {
		if (band == '6g') {
			set_default(config, 'ieee80211w', 2);
		} else {
			set_default(config, 'ieee80211w', 0);
			config.rsn_override_mfp = 2;
			config.rsn_override_omit_rsnxe = 1;
		}
		if (config.rsn_override_pairwise_2)
			config.rsn_override_mfp_2 = 2;
	}

	if (config.auth_type == 'owe') {
		set_default(config, 'owe_groups', '19 20 21');
		set_default(config, 'owe_ptk_workaround', 1);
	}

	if (config.auth_type in [ 'sae', 'psk-sae', 'psk-sae-compat' ]) {
		config.sae_require_mfp = 1;
		set_default(config, 'sae_groups', '19 20 21');
		if (!config.ppsk) {
			if (band == '6g')
				set_default(config, 'sae_pwe', 1);
			else
				set_default(config, 'sae_pwe', 2);
		}
	}

	if (config.own_ip_addr)
		config.dynamic_own_ip_addr = null;

	if (!config.wpa)
		config.wpa_disable_eapol_key_retries = null;

	switch(config.auth_type) {
	case 'none':
	case 'owe':
		config.wps_possible = 1;
		config.wps_state = 1;

		append_string_vars(config, [ 'owe_transition_ssid' ]);
		append_vars(config, [
			'owe_transition_bssid', 'owe_transition_ifname',
		]);
		break;

	case 'dpp':
		append_vars(config, [
			'dpp_connector', 'dpp_csign', 'dpp_netaccesskey',
		]);
		break;

	case 'psk':
	case 'psk2':
	case 'sae':
	case 'psk-sae':
	case 'psk-sae-compat':
		config.vlan_possible = 1;
		config.wps_possible = 1;

		if (config.ppsk) {
			iface_authentication_server(config);
			config.macaddr_acl = 2;
			config.wpa_psk_radius = 2;
		} else if (length(config.key) == 64) {
			config.wpa_psk = config.key;
		} else if (length(config.key) >= 8 && length(config.key) <= 63) {
			config.wpa_passphrase = config.key;
		} else if (config.key) {
			 netifd.setup_failed('INVALID_WPA_PSK');
		}

		if (config.auth_type in [ 'psk', 'psk-sae', 'psk-sae-compat' ] && band != '6g') {
			set_default(config, 'wpa_psk_file', `/var/run/hostapd-${config.ifname}.psk`);
			touch_file(config.wpa_psk_file);
		}

		if (config.auth_type in [ 'sae', 'psk-sae', 'psk-sae-compat' ]) {
			set_default(config, 'sae_password_file', `/var/run/hostapd-${config.ifname}.sae`);
			touch_file(config.sae_password_file);
		}
		break;

	case 'eap':
	case 'eap2':
	case 'eap-eap2':
	case 'eap192':
		config.vlan_possible = 1;

		if (config.fils) {
			set_default(config, 'erp_domain', config.mobility_domain);
			set_default(config, 'erp_domain', substr(md5(config.ssid + '\n'), 0, 8));
			set_default(config, 'fils_realm', config.erp_domain);
			set_default(config, 'erp_send_reauth_start', 1);
			set_default(config, 'fils_cache_id', substr(md5(config.fils_realm + '\n'), 0, 4));
		}

		if (!config.eap_server) {
			iface_authentication_server(config);
			iface_accounting_server(config);
		}

		if (config.radius_das_client && config.radius_das_secret) {
			set_default(config, 'radius_das_port', 3799);
			config.radius_das_client = config.radius_das_client + ' ' + config.radius_das_secret;
		}

		set_default(config, 'eapol_version', config.wpa & 1);
		if (!config.eapol_version)
			config.eapol_version = null;
		append('eapol_key_index_workaround', '1');
		append('ieee8021x', '1');

		break;
	}

	append_vars(config, [
		'sae_require_mfp', 'sae_password_file', 'sae_pwe', 'sae_groups', 'sae_track_password', 'time_advertisement', 'time_zone',
		'wpa_group_rekey', 'wpa_ptk_rekey', 'wpa_gmk_rekey', 'wpa_strict_rekey',
		'macaddr_acl', 'wpa_psk_radius', 'wpa_psk', 'wpa_passphrase', 'wpa_psk_file',
		'eapol_version', 'dynamic_vlan', 'radius_request_cui', 'eap_reauth_period',
		'radius_das_client', 'radius_das_port', 'owe_groups', 'owe_ptk_workaround', 'own_ip_addr', 'dynamic_own_ip_addr',
		'wpa_disable_eapol_key_retries', 'auth_algs', 'wpa', 'wpa_pairwise',
		'erp_domain', 'fils_realm', 'erp_send_reauth_start', 'fils_cache_id'
	]);

	if (config.dpp && config.auth_type != 'dpp')
		append_vars(config, [
			'dpp_connector', 'dpp_csign', 'dpp_netaccesskey',
		]);
}

function iface_ppsk(config) {
	if (!(config.auth_type in [ 'none', 'owe', 'psk', 'sae', 'psk-sae', 'psk-sae-compat', 'wep' ]) || !config.auth_server_addr)
		return;

	iface_authentication_server(config);
	append('macaddr_acl', '2');
}

function iface_wps(config) {
	push_config(config, 'config_methods', 'wps_pushbutton', 'push_button');
	push_config(config, 'config_methods', 'wps_label', 'label');

	if (config.multi_ap == 1)
		config.wps_possible = false;

	if (config.wps_possible && length(config.config_methods)) {
		config.eap_server = 1;
		set_default(config, 'wps_state', 2);

		if (config.ext_registrar && config.network_bridge)
			set_default(config, 'upnp_iface', config.network_bridge);

		if (config.multi_ap && config.multi_ap_backhaul_ssid) {
			append_string_vars(config, [ 'multi_ap_backhaul_ssid' ]);
			if (length(config.multi_ap_backhaul_key) == 64)
				append('multi_ap_backhaul_wpa_psk', config.multi_ap_backhaul_key);
			else if (length(config.multi_ap_backhaul_key) > 8)
				append('multi_ap_backhaul_wpa_passphrase', config.multi_ap_backhaul_key);
			else
				netifd.setup_failed('INVALID_WPA_PSK');
		}

		append_vars(config, [
			'wps_state', 'device_type', 'device_name', 'config_methods', 'wps_independent', 'eap_server',
			'ap_pin', 'ap_setup_locked', 'upnp_iface', 'uuid'
		]);
	}
}

function iface_rrm(config) {
	set_default(config, 'rrm_neighbor_report', config.ieee80211k);
	set_default(config, 'rrm_beacon_report', config.ieee80211k);

	append_vars(config, [
		'rrm_neighbor_report', 'rrm_beacon_report', 'rnr', 'ftm_responder',
	]);
}

function iface_ftm(config, phy_features) {
	if (!phy_features.ftm_responder || !config.ftm_responder)
		return;

	append_vars(config, [
		'ftm_responder', 'lci', 'civic'
	]);
}

function iface_macfilter(config) {
	let path = `/var/run/hostapd-${config.ifname}.maclist`;

	switch(config.macfilter) {
	case 'allow':
		append('accept_mac_file', path);
		append('macaddr_acl', 1);
		config.vlan_possible = 1;
		break;

	case 'deny':
		append('deny_mac_file', path);
		append('macaddr_acl', 0);
		break;

	default:
		return;
	}

	let file = fs.open(path, 'w');
	if (!file) {
		warn(`Failed to open ${path}`);
		return;
	}

	if (config.maclist)
		file.write(join('\n', config.maclist));

	let macfile = fs.readfile(config.macfile);
	if (macfile)
		file.write(macfile);
	file.close();
}

function iface_vlan(interface, config, vlans) {
	let path = `/var/run/hostapd-${config.ifname}.vlan`;

	let file = fs.open(path, 'w');
	for (let k, vlan in vlans)
		if (vlan.config.name && vlan.config.vid) {
			let ifname = `${config.ifname}-${vlan.config.name}`;
			file.write(`${vlan.config.vid} ${ifname}\n`);
			netifd.set_vlan(interface, ifname, k);
		}
	file.close();

	set_default(config, 'vlan_file', path);
	append_vars(config, [ 'vlan_file' ]);

	if (!config.vlan_possible || !config.dynamic_vlan)
		return;

	set_default(config, 'vlan_no_bridge', !config.vlan_bridge);

	append_vars(config, [
		'dynamic_vlan', 'vlan_naming', 'vlan_bridge', 'vlan_no_bridge',
		'vlan_tagged_interface'
	]);
}

function iface_wpa_stations(config, stas) {
	let path = `/var/run/hostapd-${config.ifname}.psk`;

	let file = fs.open(path, 'w');
	for (let k, sta in stas)
		if (sta.config.mac && sta.config.key) {
			for (let mac in sta.config.mac) {
				let station = `${mac} ${sta.config.key}\n`;
				if (sta.config.vid)
					station = `vlanid=${sta.config.vid} ` + station;
				file.write(station);
			}
		}
	file.close();

	set_default(config, 'wpa_psk_file', path);
}

function iface_sae_stations(config, stas) {
	let path = `/var/run/hostapd-${config.ifname}.sae`;

	let file = fs.open(path, 'w');
	for (let k, sta in stas)
		if (sta.config.mac && sta.config.key) {
			for (let mac in sta.config.mac) {
				if (mac == '00:00:00:00:00:00')
					mac = 'ff:ff:ff:ff:ff:ff';

				let station = `${sta.config.key}|mac=${mac}`;
				if (sta.config.vid)
					station = station + `|vlanid=${sta.config.vid}`;
				station = station + '\n';
				file.write(station);
			}
		}
	file.close();

	set_default(config, 'sae_password_file', path);
}

function iface_eap_server(config) {
	if (!config.eap_server)
		return;

	set_default(config, 'eap_server', true);
	set_default(config, 'eap_server_erp', true);

	append_vars(config, [
		'eap_server', 'eap_server_erp', 'eap_user_file', 'ca_cert', 'server_cert',
		'private_key', 'private_key_passwd', 'server_id',
	]);
}

function iface_roaming(config) {
	if (!config.ieee80211r || config.wpa < 2)
		return;

	set_default(config, 'mobility_domain', substr(md5(config.ssid + '\n'), 0, 4));
	set_default(config, 'ft_psk_generate_local', config.auth_type == 'psk');
	set_default(config, 'ft_iface', config.network_ifname);

	if (!config.ft_psk_generate_local) {
		if (!config.r0kh || !config.r1kh) {
			if (!config.auth_secret && !config.key)
				netifd.setup_failed('FT_KEY_CANT_BE_DERIVED');

			let ft_key = md5(`${config.mobility_domain}/${config.auth_secret ?? config.key}`);

			set_default(config, 'r0kh', [ 'ff:ff:ff:ff:ff:ff,*,' + ft_key ]);
			set_default(config, 'r1kh', [ '00:00:00:00:00:00,00:00:00:00:00:00,' + ft_key ]);
		}

		for (let name in [ 'r0kh', 'r1kh' ])
			for (let val in config[name])
				append(name, join(' ', split(val, ',', 3)));

		append_vars(config, [
			'r1_key_holder', 'r0_key_lifetime', 'pmk_r1_push'
		]);
	}

	append_vars(config, [
		'mobility_domain', 'ft_psk_generate_local', 'ft_over_ds', 'reassociation_deadline',
		'ft_iface'
	]);
}

function default_group_mgmt_cipher(config) {
	let p = ' ' + (config.wpa_pairwise ?? '') + ' ';

	if (wildcard(p, '* CCMP *') || wildcard(p, '* TKIP *'))
		return 'AES-128-CMAC';
	if (wildcard(p, '* CCMP-256 *'))
		return 'BIP-CMAC-256';
	if (wildcard(p, '* GCMP *'))
		return 'BIP-GMAC-128';
	if (wildcard(p, '* GCMP-256 *'))
		return 'BIP-GMAC-256';
	return 'AES-128-CMAC';
}

function iface_mfp(config) {
	let override_mfp = config.rsn_override_mfp || config.rsn_override_mfp_2;

	if ((!config.ieee80211w && !override_mfp) || config.wpa < 2) {
		append('ieee80211w', 0);
		return;
	}

	config.group_mgmt_cipher = config.ieee80211w_mgmt_cipher ?? default_group_mgmt_cipher(config);

	set_default(config, 'beacon_prot', 1);

	append_vars(config, [
		'ieee80211w', 'group_mgmt_cipher', 'beacon_prot',
		'assoc_sa_query_max_timeout', 'assoc_sa_query_retry_timeout'
	]);
}

function iface_transition_disable(config) {
	if (config.wpa < 2)
		return;

	let list = config.transition_disable;
	if (!list || !length(list))
		return;

	for (let s in list)
		if (s == 'off' || s == '0')
			return;

	let bits = 0;
	for (let s in list) {
		if (s == 'on' || s == '1') {
			bits = 0;
			switch (config.auth_type) {
			case 'sae':    bits = 0x01; break;
			case 'eap2':
			case 'eap192': bits = 0x04; break;
			case 'owe':    if (!config.owe_transition) bits = 0x08; break;
			}
			break;
		}
		switch (s) {
		case 'sae':    bits |= 0x01; break;
		case 'sae-pk': bits |= 0x02; break;
		case 'wpa3':   bits |= 0x04; break;
		case 'owe':    bits |= 0x08; break;
		}
	}

	if (bits)
		append('transition_disable', sprintf('0x%02x', bits));
}

function iface_key_caching(config) {
	if (config.wpa < 2)
		return;

	if (config.network_bridge && config.rsn_preauth) {
		set_default(config, 'okc', true);
		config.rsn_preauth_interfaces = config.network_bridge;

		append_vars(config, [
			'rsn_preauth', 'rsn_preauth_interfaces'
		]);
	} else {
		set_default(config, 'okc', (config.auth_type in  [ 'sae', 'psk-sae', 'psk-sae-compat', 'owe' ]));
	}

	if (!config.okc && !config.fils)
		config.disable_pmksa_caching = 1;

	append_vars(config, [
		'okc', 'disable_pmksa_caching'
	]);
}

function iface_hs20(config) {
	if (!config.hs20)
		return;

	append_vars(config, [
		'hs20', 'disable_dgaf', 'anqp_domain_id', 'hs20_deauth_req_timeout',
		'hs20_wan_metrics', 'hs20_operating_class', 'hs20_t_c_filename', 'hs20_t_c_timestamp',
		'hs20_t_c_server_url'
	]);
	append_list(config, [ 'hs20_conn_capab' ]);
}

function iface_interworking(config) {
	if (!config.iw_enabled)
		return;

	config.interworking = true;

	if (config.domain_name)
		config.domain_name = join(',', config.domain_name);

	if (config.anqp_3gpp_cell_net)
		config.anqp_3gpp_cell_net = join(';', config.anqp_3gpp_cell_net);

	append_vars(config, [
		'interworking', 'internet', 'asra', 'uesa', 'access_network_type', 'hessid', 'venue_group',
		'venue_type', 'network_auth_type', 'gas_address3', 'roaming_consortium',
		'domain_name', 'anqp_3gpp_cell_net',
	]);
	append_list(config, [ 'anqp_elem', 'nai_realm', 'venue_name', 'venue_url' ]);
}

function iface_rates(config) {
	for (let key in [ 'supported_rates', 'basic_rates' ])
		append(key, map(config[key], x => x / 100))
}

export function generate(interface, data, config, vlans, stas, phy_features) {
	config.ctrl_interface = '/var/run/hostapd';

	config.start_disabled = data.ap_start_disabled;
	iface_setup(config, data.phy + data.phy_suffix, data.config.num_global_macaddr, data.config.macaddr_base);

	iface.parse_encryption(config, data.config, phy_features);
	if (data.config.band == '6g') {
		if (config.auth_type == 'psk-sae')
			config.auth_type = 'sae';
		if (config.auth_type == 'eap-eap2')
			config.auth_type = 'eap2';
	}

	if (config.auth_type in [ 'psk', 'psk-sae', 'psk-sae-compat' ] && data.config.band != '6g')
		iface_wpa_stations(config, stas);
	if (config.auth_type in [ 'sae', 'psk-sae', 'psk-sae-compat' ])
		iface_sae_stations(config, stas);

	iface_rates(data.config);

	iface_auth_type(config, data.config.band);

	iface_accounting_server(config);

	iface_ppsk(config);

	iface_wps(config);

	iface_rrm(config);

	iface_ftm(config, phy_features);

	iface_macfilter(config);

	iface_vlan(interface, config, vlans);

	iface_eap_server(config);

	iface_roaming(config);

	iface_mfp(config);

	iface_transition_disable(config);

	iface_key_caching(config);

	iface_hs20(config);

	iface_interworking(config);

	iface.wpa_key_mgmt(config, data.config.band);
	append_vars(config, [
		'wpa_key_mgmt',
	]);

	if (config.rsn_override_key_mgmt && config.rsn_override_pairwise && config.rsn_override_mfp) {
		append_vars(config, [
			'rsn_override_key_mgmt',
			'rsn_override_pairwise',
			'rsn_override_mfp'
		]);
	}

	if (config.rsn_override_key_mgmt_2 && config.rsn_override_pairwise_2 && config.rsn_override_mfp_2) {
		append_vars(config, [
			'rsn_override_key_mgmt_2',
			'rsn_override_pairwise_2',
			'rsn_override_mfp_2'
		]);
	}

	if (config.rsn_override_omit_rsnxe) {
		append_vars(config, ['rsn_override_omit_rsnxe']);
	}

	/* raw options */
	for (let raw in config.hostapd_bss_options)
		append_raw(raw);

	if (config.mlo) {
		append_raw('mld_ap=1');
		if (data.config.radio != null)
			append_raw('mld_link_id=' + data.config.radio);
	}

	if (config.default_macaddr)
		append_raw('#default_macaddr');
	else if (config.random_macaddr)
		append_raw('#random_macaddr');
};
