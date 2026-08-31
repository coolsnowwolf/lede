#!/usr/bin/ucode

'use strict';

import { set_default, log } from 'wifi.common';
import { validate, dump_options } from 'wifi.validate';
import * as supplicant from 'wifi.supplicant';
import * as hostapd from 'wifi.hostapd';
import * as netifd from 'wifi.netifd';
import * as iface from 'wifi.iface';
import { find_phy } from 'wifi.utils';
import * as nl80211 from 'nl80211';
import * as fs from 'fs';

global.radio = ARGV[2];

const mesh_param_list = [
	"mesh_retry_timeout", "mesh_confirm_timeout", "mesh_holding_timeout", "mesh_max_peer_links",
	"mesh_max_retries", "mesh_ttl", "mesh_element_ttl", "mesh_hwmp_max_preq_retries",
	"mesh_path_refresh_time", "mesh_min_discovery_timeout", "mesh_hwmp_active_path_timeout",
	"mesh_hwmp_preq_min_interval", "mesh_hwmp_net_diameter_traversal_time", "mesh_hwmp_rootmode",
	"mesh_hwmp_rann_interval", "mesh_gate_announcements", "mesh_sync_offset_max_neighor",
	"mesh_rssi_threshold", "mesh_hwmp_active_path_to_root_timeout", "mesh_hwmp_root_interval",
	"mesh_hwmp_confirmation_interval", "mesh_awake_window", "mesh_plink_timeout",
	"mesh_auto_open_plinks", "mesh_fwding", "mesh_nolearn", "mesh_power_mode"
];

function phy_suffix(radio, sep) {
	if (radio == null || radio < 0)
		return "";
	return sep + radio;
}

function reset_config(phy, radio) {
	let name = phy + phy_suffix(radio, ".");
	let prev_config = `/var/run/hostapd-${name}.conf`;

	global.ubus.call('hostapd', 'config_set', { phy, radio, config: '', prev_config });
	global.ubus.call('wpa_supplicant', 'config_set', { phy, radio, config: []});

	name = phy + phy_suffix(radio, ":");
	system(`ucode /usr/share/hostap/wdev.uc ${name} set_config '{}'`);
}

function get_channel_frequency(band, channel) {
	if (channel < 1)
		return null;

	switch (band) {
	case '2g':
		if (channel == 14)
			return 2484;
		return 2407 + channel * 5;
	case '5g':
		if (channel >= 182 && channel <= 196)
			return 4000 + channel * 5;
		return 5000 + channel * 5;
	case '6g':
		if (channel == 2)
			return 5935;
		return 5950 + channel * 5;
	case '60g':
		return 56160 + channel * 2160;
	}
}

function setup_phy(phy, config, data) {
	if (config.channel == "auto")
		config.channel = 0;
	config.channel = +config.channel;
	config.frequency = get_channel_frequency(config.band, config.channel);

	if (config.country_code) {
		log(`Setting country code to ${config.country_code}`);
		system(`iw reg set ${config.country_code}`);
	}

	set_default(config, 'rxantenna', 0xffffffff);
	set_default(config, 'txantenna', 0xffffffff);

	if (config.txantenna == 'all')
		config.txantenna = 0xffffffff;
	if (config.rxantenna == 'all')
		config.rxantenna = 0xffffffff;

	let antenna_changed = (config.txantenna != data?.txantenna || config.rxantenna != data?.rxantenna);

	if (antenna_changed)
		reset_config(phy, config.radio);

	netifd.set_data({
		phy,
		radio: config.radio,
		txantenna: config.txantenna,
		rxantenna: config.rxantenna
	});

	if (config.txpower)
		config.txpower = 'fixed ' + config.txpower + '00';
	else
		config.txpower = 'auto';

	log(`Configuring '${phy}' distance: ${config.distance}`);
	if (antenna_changed) {
		log(`Setting antenna for '${phy}' txantenna: ${config.txantenna}, rxantenna: ${config.rxantenna}`);
		system(`iw phy ${phy} set antenna ${config.txantenna} ${config.rxantenna}`);
	}
	system(`iw phy ${phy} set distance ${config.distance}`);
	system(`iw phy ${phy} set txpower ${config.txpower}`);

	if (config.frag)
		system(`iw phy ${phy} set frag ${config.frag}`);
	if (config.rts)
		system(`iw phy ${phy} set rts ${config.rts}`);
}

function iw_htmode(config) {
	let suffix = substr(config.htmode, 3);
	if (suffix == "40+" || suffix == "40-")
		return "HT" + suffix;

	switch (config.htmode ?? "NONE") {
	case "HT20":
	case "VHT20":
	case "HE20":
	case "EHT20":
		return "HT20";
	case "VHT80":
	case "HE80":
	case "EHT80":
	case "HE160":
	case "EHT160":
	case "EHT320":
		return "80MHZ";
	case "NONE":
	case "NOHT":
		return "NOHT";
	}

	if (substr(config.htmode, 2) == "40") {
		switch (config.band) {
		case "2g":
			if (+config.channel < 7)
				return "HT40+";
			else
				return "HT40-";
		default:
			return ((+config.channel / 4) % 2) ? "HT40+" : "HT40-";
		}
	}

	return null;
}

function config_add(config, name, val) {
	if (val != null)
		config[name] = val;
}

function config_add_mesh_params(config, data) {
	for (let param in mesh_param_list)
		config_add(config, param, data[param]);
}

function setup() {
	let data = json(ARGV[3]);

	data.phy = find_phy(data.config, true);
	if (!data.phy) {
		log('Bug: PHY is undefined for device');
		netifd.set_retry(false);
		return 1;
	}
	data.phy_suffix = phy_suffix(data.config.radio, ":");
	data.vif_phy_suffix = phy_suffix(data.config.radio, ".");
	data.ifname_prefix = data.config.ifname_prefix;
	if (!data.ifname_prefix)
		data.ifname_prefix = data.phy + data.vif_phy_suffix + "-";
	let active_ifnames = [];

	log('Starting');

	let config = data.config;

	if (!config.band) {
		switch (config.hwmode) {
		case 'a':
		case '11a':
			config.band = '5g';
			break;
		case 'ad':
		case '11ad':
			config.band = '60g';
			break;
		case 'b':
		case 'g':
		case '11b':
		case '11g':
		default:
			config.band = '2g';
			break;
		}
	}
	delete config.hwmode;

	validate('device', config);
	setup_phy(data.phy, data.config, data.data);

	let supplicant_mesh;
	let has_ap = false;
	let idx = {};
	let supplicant_data = [];
	let wdev_data = {};

	for (let k, v in data.interfaces) {
		let mode = v.config.mode;
		idx[mode] ??= 0;
		let mode_idx = idx[mode]++;

		if (!v.config.ifname)
			v.config.ifname = data.ifname_prefix + mode + mode_idx;
		push(active_ifnames, v.config.ifname);

		if (v.config.encryption == 'owe' && v.config.owe_transition) {
			mode_idx = idx[mode]++;
			v.config.owe_transition_ifname = data.ifname_prefix + mode + mode_idx;
			push(active_ifnames, v.config.ifname);
		}

		switch (mode) {
		case 'ap':
			has_ap = true;
			for (let _, sta in v.stas)
				validate('station', sta.config);
			// fallthrough
		case 'sta':
		case 'adhoc':
		case 'mesh':
			if (mode != "ap")
				data.config.noscan = true;
			validate('iface', v.config);
			iface.prepare(v.config, data.phy + data.phy_suffix, data.config.num_global_macaddr, data.config.macaddr_base);
			netifd.set_vif(k, v.config.ifname);
			break;
		}

		switch (mode) {
		case 'adhoc':
			if (config.frequency && !v.config.wpa)
				break;
			// fallthrough
		case 'mesh':
			supplicant_mesh ??= (fs.access('/usr/sbin/wpa_supplicant', 'x') && !system("/usr/sbin/wpa_supplicant -vmesh"));
			if (mode == "mesh" && !supplicant_mesh)
				break;
			// fallthrough
		case 'sta':
			data.ap_start_disabled = true;
			data.channel_follow = true;
			let config = supplicant.generate(supplicant_data, data, v);
			if (mode == "mesh")
				config_add_mesh_params(config, v.config);
			continue;
		case 'monitor':
			break;
		default:
			continue;
		}

		// fallback to wdev setup
		let config = {
			mode,
			ssid: v.config.ssid,
		};

		if (!v.config.default_macaddr)
			config.macaddr = v.config.macaddr;

		config_add(config, "freq", data.config.frequency);
		config_add(config, "htmode", iw_htmode(data.config));
		if (mode != "monitor") {
			let basic_rate_list = v.config.basic_rate ?? data.config.basic_rate;
			config_add(config, "basic-rates", supplicant.ratelist(basic_rate_list));
			config_add(config, "mcast-rate", supplicant.ratestr(v.config.mcast_rate));
			config_add(config, "beacon-interval", data.config.beacon_int);
			if (mode == "adhoc")
				config_add(config, "bssid", v.config.bssid);
			if (mode == "mesh") {
				config_add(config, "ssid", v.config.mesh_id);
				config_add_mesh_params(config, v.config);
			}
		}

		wdev_data[v.config.ifname] = config;
	}

	for (let ifname in active_ifnames) {
		if (!wdev_data[ifname])
			continue;

		let if_config = {
			[ifname]: wdev_data[ifname]
		};
		system(`ucode /usr/share/hostap/wdev.uc ${data.phy}${data.phy_suffix} set_config '${if_config}'`);
	}

	if (fs.access('/usr/sbin/wpa_supplicant', 'x'))
		supplicant.setup(supplicant_data, data);

	if (fs.access('/usr/sbin/hostapd', 'x'))
		hostapd.setup(data);

	if (length(supplicant_data) > 0)
		supplicant.start(data);

	netifd.set_up();

	return 0
}

function teardown() {
	let data = json(ARGV[3]);

	if (ARGV[2] == "#mlo")
		return 0;

	if (!data.data?.phy) {
		log('Bug: PHY is undefined for device');
		return 1;
	}

	log(`Tearing down ${data.data.phy}`);

	reset_config(data.data.phy, data.data.radio);

	return 0;
}

let ret = 1;

switch(ARGV[1]) {
case 'dump':
	ret = dump_options();
	break;

case 'setup':
	ret = setup();
	break;

case 'teardown':
	ret = teardown();
	break;
}

exit(ret);
