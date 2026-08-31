'use strict';

import { append_value, log } from 'wifi.common';
import * as fs from 'fs';

export function parse_encryption(config, dev_config, phy_features) {
	if (!config.encryption)
		return;

	let encryption = split(config.encryption, '+', 2);

	config.wpa = 0;
	for (let k, v in { 'wpa2*': 2, 'wpa3*': 2, '*psk2*': 2, 'psk3*': 2, 'sae*': 2,
			'owe*': 2, 'dpp': 2, 'wpa*mixed*': 3, '*psk*mixed*': 3, 'wpa*': 1, '*psk*': 1, })
		if (wildcard(config.encryption, k)) {
			config.wpa = v;
			break;
		}

	config.auth_type = encryption[0] ?? 'none';

	/*
	 * GCMP-256 and the SAE-EXT-KEY (SAE-GDH) AKM are only mandatory for
	 * EHT/MLO and break interoperability with many clients, so only default
	 * them on where they are both required and safe to offer: on Compatibility
	 * mode (sae-compat) BSSes that run an EHT htmode, which carry them in a
	 * separate RSN Override element that legacy clients ignore. They stay off
	 * for WPA3-Personal (sae) and Transition (sae-mixed) mode and on non-EHT
	 * BSSes. Explicit gcmp256 and sae_ext_key options override this per BSS.
	 */
	let compat = (config.auth_type == 'sae-compat');
	config.gcmp256 ??= compat && wildcard(dev_config?.htmode ?? '', 'EHT*');
	config.sae_ext_key ??= compat && wildcard(dev_config?.htmode ?? '', 'EHT*');

	switch(config.auth_type) {
	case 'owe':
		config.auth_type = 'owe';
		break;

	case 'dpp':
		config.auth_type = 'dpp';
		break;

	case 'wpa3-192':
		config.auth_type = 'eap192';
		config.wpa_pairwise = 'GCMP-256';
		break;

	case 'wpa3-mixed':
		config.auth_type = 'eap-eap2';
		break;

	case 'wpa3':
		config.auth_type = 'eap2';
		break;

	case 'psk':
	case 'psk2':
	case 'psk-mixed':
		config.auth_type = 'psk';
		break;

	case 'sae':
	case 'psk3':
		config.auth_type = 'sae';
		break;

	case 'psk3-mixed':
	case 'sae-mixed':
		config.auth_type = 'psk-sae';
		break;

	case 'sae-compat':
		config.auth_type = 'psk-sae-compat';
		config.wpa_pairwise = 'CCMP';
		if (dev_config.band != '6g')
			config.rsn_override_pairwise = 'CCMP';
		if (config.gcmp256 && phy_features?.cipher_gcmp256)
			config.rsn_override_pairwise_2 = 'GCMP-256';
		break;

	case 'wpa':
	case 'wpa2':
	case 'wpa-mixed':
		config.auth_type = 'eap';
		break;
	}

	switch(encryption[1]){
	case 'tkip+aes':
	case 'tkip+ccmp':
	case 'aes+tkip':
	case 'ccmp+tkip':
		config.wpa_pairwise = 'CCMP TKIP';
		break;

	case 'ccmp256':
		config.wpa_pairwise = 'CCMP-256';
		break;

	case 'aes':
	case 'ccmp':
		config.wpa_pairwise = 'CCMP';
		break;

	case 'tkip':
		config.wpa_pairwise = 'TKIP';
		break;

	case 'gcmp256':
		config.wpa_pairwise = 'GCMP-256';
		break;

	case 'gcmp':
		config.wpa_pairwise = 'GCMP';
		break;
	}

	if (!config.wpa)
		config.wpa_pairwise ??= null;
	else if (dev_config.band == '60g')
		config.wpa_pairwise ??= 'GCMP';
	else if (config.gcmp256 && phy_features?.cipher_gcmp256)
		config.wpa_pairwise ??= 'GCMP-256 CCMP';
	else
		config.wpa_pairwise ??= 'CCMP';
};

export function wpa_key_mgmt(config, band) {
	if (!config.wpa)
		return;

	switch(config.auth_type) {
	case 'psk':
	case 'psk2':
		append_value(config, 'wpa_key_mgmt', 'WPA-PSK');
		if (config.wpa >= 2 && config.ieee80211r)
			append_value(config, 'wpa_key_mgmt', 'FT-PSK');
		if (config.ieee80211w)
			append_value(config, 'wpa_key_mgmt', 'WPA-PSK-SHA256');
		break;

	case 'eap':
		append_value(config, 'wpa_key_mgmt', 'WPA-EAP');
		if (config.wpa >= 2 && config.ieee80211r)
			append_value(config, 'wpa_key_mgmt', 'FT-EAP');
		if (config.ieee80211w)
			append_value(config, 'wpa_key_mgmt', 'WPA-EAP-SHA256');
		break;

	case 'eap192':
		append_value(config, 'wpa_key_mgmt', 'WPA-EAP-SUITE-B-192');
		if (config.ieee80211r)
			append_value(config, 'wpa_key_mgmt', 'FT-EAP-SHA384');
		break;

	case 'eap-eap2':
		append_value(config, 'wpa_key_mgmt', 'WPA-EAP-SHA256');
		if (config.ieee80211r)
			append_value(config, 'wpa_key_mgmt', 'FT-EAP');

		append_value(config, 'wpa_key_mgmt', 'WPA-EAP');
		break;

	case 'eap2':
		append_value(config, 'wpa_key_mgmt', 'WPA-EAP-SHA256');
		if (config.ieee80211r)
			append_value(config, 'wpa_key_mgmt', 'FT-EAP');
		break;

	case 'sae':
		append_value(config, 'wpa_key_mgmt', 'SAE');
		if (config.sae_ext_key)
			append_value(config, 'wpa_key_mgmt', 'SAE-EXT-KEY');
		if (config.ieee80211r) {
			append_value(config, 'wpa_key_mgmt', 'FT-SAE');
			if (config.sae_ext_key)
				append_value(config, 'wpa_key_mgmt', 'FT-SAE-EXT-KEY');
		}
		break;

	case 'psk-sae':
		append_value(config, 'wpa_key_mgmt', 'SAE');
		if (config.sae_ext_key)
			append_value(config, 'wpa_key_mgmt', 'SAE-EXT-KEY');
		if (config.ieee80211r) {
			append_value(config, 'wpa_key_mgmt', 'FT-SAE');
			if (config.sae_ext_key)
				append_value(config, 'wpa_key_mgmt', 'FT-SAE-EXT-KEY');
		}

		append_value(config, 'wpa_key_mgmt', 'WPA-PSK');
		if (config.ieee80211w)
			append_value(config, 'wpa_key_mgmt', 'WPA-PSK-SHA256');
		if (config.ieee80211r)
			append_value(config, 'wpa_key_mgmt', 'FT-PSK');
		break;

	case 'psk-sae-compat':
		if (band == '6g') {
			append_value(config, 'wpa_key_mgmt', 'SAE');
			if (config.ieee80211r)
				append_value(config, 'wpa_key_mgmt', 'FT-SAE');

			if (config.sae_ext_key) {
				append_value(config, 'rsn_override_key_mgmt_2', 'SAE-EXT-KEY');
				if (config.ieee80211r)
					append_value(config, 'rsn_override_key_mgmt_2', 'FT-SAE-EXT-KEY');
			}
		} else {
			append_value(config, 'wpa_key_mgmt', 'WPA-PSK');
			if (config.ieee80211w)
				append_value(config, 'wpa_key_mgmt', 'WPA-PSK-SHA256');
			if (config.ieee80211r)
				append_value(config, 'wpa_key_mgmt', 'FT-PSK');

			append_value(config, 'rsn_override_key_mgmt', 'SAE');
			if (config.ieee80211r)
				append_value(config, 'rsn_override_key_mgmt', 'FT-SAE');

			if (config.sae_ext_key) {
				append_value(config, 'rsn_override_key_mgmt_2', 'SAE-EXT-KEY');
				if (config.ieee80211r)
					append_value(config, 'rsn_override_key_mgmt_2', 'FT-SAE-EXT-KEY');
			}
		}
		break;

	case 'owe':
		append_value(config, 'wpa_key_mgmt', 'OWE');
		break;

	case 'dpp':
		append_value(config, 'wpa_key_mgmt', 'DPP');
		break;
	}

	if (config.dpp && config.auth_type != 'dpp')
		append_value(config, 'wpa_key_mgmt', 'DPP');

	if (config.fils) {
		switch(config.auth_type) {
		case 'eap192':
			append_value(config, 'wpa_key_mgmt', 'FILS-SHA384');
			if (config.ieee80211r)
				append_value(config, 'wpa_key_mgmt', 'FT-FILS-SHA384');
			break;

		case 'eap-eap2':
		case 'eap2':
		case 'eap':
			append_value(config, 'wpa_key_mgmt', 'FILS-SHA256');
			if (config.ieee80211r)
				append_value(config, 'wpa_key_mgmt', 'FT-FILS-SHA256');
			break;
		}
	}

	config.key_mgmt = config.wpa_key_mgmt;
};

function macaddr_random() {
	let f = fs.open("/dev/urandom", "r");
	let addr = f.read(6);

	addr = map(split(addr, ""), (v) => ord(v));
	addr[0] &= ~1;
	addr[0] |= 2;

	return join(":", map(addr, (v) => sprintf("%02x", v)));
}

let mac_idx = 0;
export function prepare(data, phy, num_global_macaddr, macaddr_base) {
	if (!data.macaddr) {
		let pipe = fs.popen(`ucode /usr/share/hostap/wdev.uc ${phy} get_macaddr id=${mac_idx} num_global=${num_global_macaddr} mbssid=${data.mbssid ?? 0} macaddr_base=${macaddr_base ?? ""}`);

		data.macaddr = trim(pipe.read("all"), '\n');
		pipe.close();

		data.default_macaddr = true;
		mac_idx++;
	} else if (data.macaddr == 'random') {
		data.macaddr = macaddr_random();
		data.random_macaddr = true;
	}

	log(`Preparing interface: ${data.ifname} with MAC: ${data.macaddr}`);
};
