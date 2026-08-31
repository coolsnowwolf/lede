'use strict';

import * as nl80211 from 'nl80211';
import * as libubus from 'ubus';
import { readfile, stat } from "fs";

let wifi_devices = json(readfile('/usr/share/wifi_devices.json'));
let countries = json(readfile('/usr/share/iso3166.json'));
let board_data = json(readfile('/etc/board.json'));

let ubus = libubus.connect();

export let phys;
let interfaces;
let wireless_status;

function find_phy(wiphy) {
	for (let k,  phy in phys)
		if (phy && phy.wiphy == wiphy)
			return phy;
	return null;
}

function get_survey(iface) {
	let channels = nl80211.request(nl80211.const.NL80211_CMD_GET_SURVEY, nl80211.const.NLM_F_DUMP, { dev: iface.ifname });
	for (let channel in channels)
		if (channel.survey_info?.frequency == iface.wiphy_freq)
			return channel.survey_info;
	return null;
}

function get_noise(iface) {
	return iface.survey?.noise ?? -100;
}

function get_country(iface) {
	let reg = nl80211.request(nl80211.const.NL80211_CMD_GET_REG, 0, { dev: iface.ifname });

	return reg.reg_alpha2 ?? '';
}

function get_max_power(iface) {
	let phy = find_phy(iface.wiphy);

	for (let k, band in phy.wiphy_bands)
		if (band)
			for (let freq in band.freqs)
				if (freq.freq == iface.wiphy_freq)
					return freq.max_tx_power;;
	return 0;
}

function get_hardware_id(iface) {
	let hw = {
		type: 'nl80211',
		id: 'Generic MAC80211',
		power_offset: 0,
		channel_offset: 0,
	};

	let path = `/sys/class/ieee80211/phy${iface.wiphy}/device/`;
	if (stat(path + 'vendor')) {
		let data = [];
		for (let lookup in [ 'vendor', 'device', 'subsystem_vendor', 'subsystem_device' ])
			push(data, trim(readfile(path + lookup), '\n'));
		
		for (let device in wifi_devices.pci) {
			let match = 0;
			for (let i = 0; i < 4; i++)
				if (lc(data[i]) == lc(device[i]))
					match++;
			if (match == 4) {
				hw.type = `${data[0]}:${data[1]} ${data[2]}:${data[3]}`;
				hw.power_offset = device[4];
				hw.channel_offset = device[5];
				hw.id = `${device[6]} ${device[7]}`;
			}
		}
	}

	let compatible = trim(readfile(`/sys/class/net/${iface.ifname}/device/of_node/compatible`), '\n');
	if (compatible && wifi_devices.compatible[compatible]) {
		hw.id = wifi_devices.compatible[compatible][0] + ' ' + wifi_devices.compatible[compatible][1];
		hw.compatible = compatible;
		hw.type = 'embedded';
	}

	return hw;
}

const iftypes = [
	'Unknown', 'Ad-Hoc', 'Client', 'Master', 'Master (VLAN)',
	'WDS', 'Monitor', 'Mesh Point', 'P2P Client', 'P2P Go',
];

export let ifaces = {};

export function update() {
	phys = nl80211.request(nl80211.const.NL80211_CMD_GET_WIPHY, nl80211.const.NLM_F_DUMP, { split_wiphy_dump: true });
	interfaces = nl80211.request(nl80211.const.NL80211_CMD_GET_INTERFACE, nl80211.const.NLM_F_DUMP);
	wireless_status = ubus.call('network.wireless', 'status');

	ifaces = {};
	for (let k, v in interfaces) {
		let iface = ifaces[v.ifname] = v;

		iface.mode = iftypes[iface.iftype] ?? 'unknown',
		iface.survey = get_survey(iface);
		iface.noise = get_noise(iface);
		iface.country = get_country(iface);
		iface.max_power = get_max_power(iface);
		iface.assoclist = nl80211.request(nl80211.const.NL80211_CMD_GET_STATION, nl80211.const.NLM_F_DUMP, { dev: v.ifname }) ?? [];
		iface.hardware = get_hardware_id(iface);

		iface.bss_info = ubus.call('hostapd', 'bss_info', { iface: v.ifname });
		if (!iface.bss_info)
			iface.bss_info = ubus.call('wpa_supplicant', 'bss_info', { iface: v.ifname });
	}

	for (let radio, data in wireless_status)
		for (let k, v in data.interfaces) {
			if (!v.ifname || !ifaces[v.ifname])
				continue;

			ifaces[v.ifname].ssid = v.config.ssid || v.config.mesh_id;
			ifaces[v.ifname].radio = data.config;

			let bss_info = ifaces[v.ifname].bss_info;
			let owe_transition_ifname = bss_info?.owe_transition_ifname;

			if (v.config.owe_transition && ifaces[owe_transition_ifname]) {
				ifaces[v.ifname].owe_transition_ifname = owe_transition_ifname;
				ifaces[owe_transition_ifname].ssid = v.config.ssid;
				ifaces[owe_transition_ifname].radio = data.config;
				ifaces[owe_transition_ifname].owe_transition_ifname = v.ifname
			}
		}
};

function format_channel(freq) {
	if (freq < 1000)
		return 0;
	if (freq == 2484)
		return 14;
	if (freq == 5935)
		return 2;
	if (freq < 2484)
		return (freq - 2407) / 5;
	if (freq >= 4910 && freq <= 4980)
		return (freq - 4000) / 5;
	if (freq < 5950)
		return (freq - 5000) / 5;
	if (freq <= 45000)
		return (freq - 5950) / 5;
	if (freq >= 58320 && freq <= 70200)
		return (freq - 56160) / 2160;

	return 'unknown';
}

function format_band(freq) {
	if (freq == 5935)
		return '6';
	if (freq < 2484)
		return '2.4';
	if (freq < 5950)
		return '5';
	if (freq <= 45000)
		return '6';

	return '60';
}

function format_frequency(freq) {
	return freq ? sprintf('%.03f', freq / 1000.0) : 'unknown';
}

function format_rate(rate) {
	return rate ? sprintf('%.01f', rate / 10.0) : 'unknown';
}

function format_expected_throughput(rate) {
	return rate ? sprintf('%.01f', rate / 1000.0) : 'unknown';
}

function format_mgmt_key(key) {
	switch(+key) {
	case 1:
		return '802.1x';

	case 2:
		return 'WPA PSK';

	case 3:
		return 'FT 802.1x';

	case 4:
		return 'FT PSK';

	case 5:
	case 11: // deprecated 802.1x-suiteB-SHA256
		return '802.1x-SHA256';

	case 6:
		return 'WPA PSK-SHA256';

	case 8: // SAE with SHA256
	case 24: // SAE using group-dependent hash
		return 'SAE';

	case 9: // FT SAE with SHA256
	case 25: // FT SAE using group-dependent hash
		return 'FT SAE';

	case 12:
		return '802.1x-192bit';

	case 13:
		return 'FT 802.1x-SHA384';

	case 14:
		return 'FILS-SHA256';

	case 15:
		return 'FILS-SHA384';

	case 16:
		return 'FT FILS-SHA256';

	case 17:
		return 'FT FILS-SHA384';

	case 18:
		return 'OWE';

	case 19:
		return 'FT PSK-SHA384';

	case 20:
		return 'WPA PSK-SHA384';

	}

	return null;
}

function assoc_flags(data) {
	const assoc_mhz = {
		width_40: 40,
		width_80: 80,
		width_80p80: '80+80',
		width_160: 160,
		width_320: 320,
		width_10: 10,
		width_5: 5
	};

	let mhz = 'unknown';
	for (let k, v in assoc_mhz)
		if (data[k])
			mhz = v; 

	const assoc_flags = {
		mcs: {
			mcs: 'MCS',
		},
		vht_mcs: {
			vht_mcs: 'VHT-MCS',
			vht_nss: 'VHT-NSS',
		},
		he_mcs: {
			he_mcs: 'HE-MCS',
			he_nss: 'HE-NSS',
			he_gi: 'HE-GI',
			he_dcm: 'HE-DCM',
		},
		eht_mcs: {
			eht_mcs: 'EHT-MCS',
			eht_nss: 'EHT-NSS',
			eht_gi: 'EHT-GI',
		},
	};

	let flags = [];
	for (let k, v in assoc_flags) {
		if (!data[k])
			continue;

		let first = 0;
		for (let name, flag in v) {
			if (data[name] == null)
				continue;
			push(flags, `${flag} ${data[name]}`);
			if (!first++)
				push(flags, `${mhz}MHz`);
		}
	}

	return flags;
}

function dbm2mw(dbm) {
	const LOG10_MAGIC = 1.25892541179;
	let res = 1.0;
	let ip = dbm / 10;
	let fp = dbm % 10;

	for (let k = 0; k < ip; k++)
		res *= 10;
	for (let k = 0; k < fp; k++)
		res *= LOG10_MAGIC;
	
	return int(res);
}

function dbm2quality(dbm) {
	let quality = dbm;

	if (quality < -110)
		quality = -110;
	else if (quality > -40)
		quality = -40;
	quality += 110;

	return quality;
}

function hwmodelist(name) {
	const mode = { 'HT*': 'n', 'VHT*': 'ac', 'HE*': 'ax', 'EHT*': 'be' };
	let iface = ifaces[name];
	let phy = board_data.wlan?.['phy' + iface.wiphy];
	if (!phy || !iface.radio?.band)
		return '';
	let htmodes = phy.info.bands[uc(iface.radio.band)].modes;
	let list = [];
	if (iface.radio.band == '2g' && 'NOHT' in htmodes)
		push(list, 'g/b');
	for (let k, v in mode)
		for (let htmode in htmodes)
			if (wildcard(htmode, k))
				push(list, v);

	return join('/', reverse(uniq(list)));
}

export function assoclist(dev) {
	if (!ifaces[dev])
		return {};

	let stations = ifaces[dev].assoclist;
	let ret = {};
	
	for (let station in stations) {
		let sta = {
			mac: uc(station.mac),
			signal: station.sta_info.signal_avg,
			noise: ifaces[dev].noise,
			snr: station.sta_info.signal_avg - ifaces[dev].noise,
			inactive_time: station.sta_info.inactive_time,
			rx: {
				bitrate: format_rate(station.sta_info.rx_bitrate?.bitrate ?? 0),
				bitrate_raw: station.sta_info.rx_bitrate?.bitrate ?? 0,
				packets: station.sta_info.rx_packets ?? 0,
				flags: assoc_flags(station.sta_info.rx_bitrate ?? {}),
			},
			tx: {
				bitrate: format_rate(station.sta_info.tx_bitrate?.bitrate ?? 0),
				bitrate_raw: station.sta_info.tx_bitrate?.bitrate ?? 0,
				packets: station.sta_info.tx_packets ?? 0,
				flags: assoc_flags(station.sta_info.tx_bitrate ?? {}),
			},
			expected_throughput: format_expected_throughput(station.sta_info.expected_throughput ?? 0),
			connected_time: station.sta_info.connected_time ?? 0,
		};
		ret[sta.mac] = sta;
	}

	return ret;
};

export function freqlist(name) {
	const freq_flags = {
		no_10mhz: 'NO_10MHZ',
		no_20mhz: 'NO_20MHZ',
		no_ht40_minus: 'NO_HT40-',
		no_ht40_plus: 'NO_HT40+',
		no_80mhz: 'NO_80MHZ',
		no_160mhz: 'NO_160MHZ',
		indoor_only: 'INDOOR_ONLY',
		no_ir: 'NO_IR',
		no_he: 'NO_HE',
		radar: 'RADAR_DETECTION',
	};

	let iface = ifaces[name];
	let phy = find_phy(iface.wiphy);
	let channels = [];

	for (let k, band in phy.wiphy_bands) {
		if (!band)
			continue;

		let band_name = format_band(band.freqs[0].freq);
		for (let freq in band.freqs) {
			if (freq.disabled)
				continue;

			let channel = {
				freq: format_frequency(freq.freq),
				band: band_name,
				channel: format_channel(freq.freq),
				flags: [],
				active: iface.wiphy_freq == freq.freq,
			};
	
			for (let k, v in freq_flags)
				if (freq[k])
					push(channel.flags, v);
			
			push(channels, channel);
		}
	}

	return channels;
};

export function info(name) {
	let order = [];
	for (let iface, data in ifaces) 
		push(order, iface);

	let list = [];
	for (let iface in sort(order)) {
		if (name && iface != name)
			continue;
		let data = ifaces[iface];
		let dev = {
			iface,
			ssid: data.ssid,
			mac: data.mac,
			mode: data.mode,
			channel: format_channel(data.wiphy_freq),
			freq: format_frequency(data.wiphy_freq),
			htmode: data.radio?.htmode,
			center_freq1: format_channel(data.center_freq1) || 'unknown',
			center_freq2: format_channel(data.center_freq2) || 'unknown',
			txpower: data.wiphy_tx_power_level / 100,
			noise: data.noise,
			signal: 0,
			bitrate: 0,
			encryption: 'unknown',
			hwmode: hwmodelist(iface),
			phy: 'phy' + data.wiphy,
			vaps: 'no',
			hw_type: data.hardware.type,
			hw_id: data.hardware.id,
			power_offset: data.hardware.power_offset || 'none',
			channel_offset: data.hardware.channel_offset || 'none',
		};

		let phy = find_phy(data.wiphy);
		for (let limit in phy.interface_combinations?.[0]?.limits)
			if (limit.types?.ap && limit.max > 1)
				dev.vaps = 'yes';

		if (data.bss_info) {
			if (data.bss_info.wpa_key_mgmt && data.bss_info.wpa_pairwise)
				dev.encryption = `${replace(data.bss_info.wpa_key_mgmt, ' ', ' / ')} (${data.bss_info.wpa_pairwise})`;
			else if (data.owe_transition_ifname)
				dev.encryption = 'none (OWE transition)';
			else
				dev.encryption = 'none';
		}

		let stations = assoclist(iface);
		for (let k, station in stations) {
			dev.signal += station.signal;
			dev.bitrate += station.tx.bitrate_raw;
		}
		dev.signal /= length(data.assoclist) || 1;
		dev.bitrate /= length(data.assoclist) || 1;
		dev.bitrate = format_rate(dev.bitrate);
		dev.quality = dbm2quality(dev.signal);

		if (data.owe_transition_ifname)
			dev.owe_transition_ifname = data.owe_transition_ifname;

		push(list, dev);
	}

	return list;
};

export function htmodelist(name) {
	let iface = ifaces[name];
	let phy = board_data.wlan?.['phy' + iface.wiphy];
	if (!phy || !iface.radio.band)
		return [];

	return filter(phy.info.bands[uc(iface.radio.band)].modes, (v) => v != 'NOHT');
};

export function txpowerlist(name) {
	let iface = ifaces[name];
	let max_power = iface.max_power / 100;
	let match = iface.wiphy_tx_power_level / 100;
	let list = [];

	for (let power = 0; power <= max_power; power++) {
		let txpower = {
			dbm: power,
			mw: dbm2mw(power),
			active: power == match,
		};
		push(list, txpower);
	}
	
	return list;
};

export function countrylist(dev) {
	let iface = ifaces[dev];

	let list = {
		active: iface.country,
		countries, 
	};

	return list;
};

function scan_extension(ext, cell) {
	const eht_chan_width = [ '20 MHz', '40 MHz', '80 MHz', '160 MHz', '320 MHz'];

	switch(ord(ext, 0)) {
	case 36:
		cell.he = {};

		if (cell.band == '6') {
			let offset = 7;

			if (ord(ext, 2) & 0x40)
				offset += 3;

			if (ord(ext, 2) & 0x80)
				offset += 1;

			cell.he.chan_width = eht_chan_width[ord(ext, offset + 1) & 0x3];
			cell.he.center_chan_1 = ord(ext, offset + 2);
			cell.he.center_chan_2 = ord(ext, offset + 3);
		} else if (cell.vht) {
			cell.he.chan_width = cell.vht.chan_width;
			cell.he.center_chan_1 = cell.vht.center_chan_1;
			cell.he.center_chan_2 = cell.vht.center_chan_2;
		} else if (cell.ht) {
			cell.he.chan_width = cell.ht.chan_width;
		}
		break;

	case 106:
		if (!(ord(ext, 1) & 0x1))
			break;

		cell.eht = {
			chan_width: eht_chan_width[ord(ext, 6) & 0x7],
			center_chan_1: ord(ext, 7),
			center_chan_2: ord(ext, 8),
		};
		break;
	}
};

export function scan(dev) {
	const rsn_cipher = [ 'NONE', 'WEP-40', 'TKIP', 'WRAP', 'CCMP', 'WEP-104', 'AES-OCB', 'CKIP', 'GCMP', 'GCMP-256', 'CCMP-256' ];
	const ht_chan_offset = [ 'no secondary', 'above', '[reserved]', 'below' ];
	const vht_chan_width = [ '20 or 40 MHz', '80 MHz', '80+80 MHz', '160 MHz' ];
	const ht_chan_width = [ '20 MHz', '40 MHz or higher' ];
	const SCAN_FLAG_AP = (1<<2);

	let params = {
		dev,
		scan_flags: SCAN_FLAG_AP,
		scan_ssids: [ '' ],
	};

	nl80211.request(nl80211.const.NL80211_CMD_TRIGGER_SCAN, 0, params);
	let err = nl80211.error();
	if (err) {
		warn("Unable to trigger scan on " + dev + ": " + err + "\n");
		return null;
	}

	let res = nl80211.waitfor([
		nl80211.const.NL80211_CMD_NEW_SCAN_RESULTS,
		nl80211.const.NL80211_CMD_SCAN_ABORTED
	], 5000);

	if (!res) {
		warn("Netlink error while awaiting scan results on " + dev + ": " + nl80211.error() + "\n");
		return null;
	} else if (res.cmd == nl80211.const.NL80211_CMD_SCAN_ABORTED) {
		warn("Scan aborted by kernel on " + dev + "\n");
		return null;
	}

	let scan = nl80211.request(nl80211.const.NL80211_CMD_GET_SCAN, nl80211.const.NLM_F_DUMP, { dev });

	let cells = [];
	for (let k, bss in scan) {
		bss = bss.bss;
		let cell = {
			bssid: uc(bss.bssid),
			frequency: format_frequency(bss.frequency),
			band: format_band(bss.frequency),
			channel: format_channel(bss.frequency),
			dbm: bss.signal_mbm / 100,

		};

		if (bss.capability & (1 << 1))
			cell.mode = 'Ad-Hoc';
		else if (bss.capability & (1 << 0))
			cell.mode = 'Master';
		else
			cell.mode = 'Mesh Point';

		cell.quality = dbm2quality(cell.dbm);

		for (let ie in bss.information_elements)
			switch(ie.type) {
			case 0:
			case 114:
				cell.ssid = ie.data;
				break;

			case 7:
				cell.country = substr(ie.data, 0, 2);
				break;

			case 48:
				cell.crypto = {
					group: rsn_cipher[ord(ie.data, 5)] ?? '',
					pair: [],
					key_mgmt: [],
				};

				let offset = 6;
				let count = ord(ie.data, offset);
				offset += 2;
				
				for (let i = 0; i < count; i++) {
					let key = rsn_cipher[ord(ie.data, offset + 3)];
					if (key)
						push(cell.crypto.pair, key);
					offset += 4;
				}
				
				count = ord(ie.data, offset);
				offset += 2;

				for (let i = 0; i < count; i++) {
					let key = format_mgmt_key(ord(ie.data, offset + 3));
					if (key)
						push(cell.crypto.key_mgmt, key);
					offset += 4;
				}
				break;

			case 61:
				cell.ht = {
					primary_channel: ord(ie.data, 0),
					secondary_chan_off: ht_chan_offset[ord(ie.data, 1) & 0x3],
					chan_width: ht_chan_width[(ord(ie.data, 1) & 0x4) >> 2],
				};
				break;

			case 192:
				cell.vht = {
					chan_width: vht_chan_width[ord(ie.data, 0)],
					center_chan_1: ord(ie.data, 1),
					center_chan_2: ord(ie.data, 2),
				};
				break;

			case 255:
				scan_extension(ie.data, cell);
				break;
			};

		

		push(cells, cell);
	}

	return cells;
};

update();
