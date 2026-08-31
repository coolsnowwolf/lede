#!/usr/bin/env ucode
'use strict';
import { readfile, writefile, realpath, glob, basename, unlink, open, rename } from "fs";
import { is_equal } from "/usr/share/hostap/common.uc";
let nl = require("nl80211");

let board_file = "/etc/board.json";
let prev_board_data = json(readfile(board_file));
let board_data = json(readfile(board_file));

function phy_idx(name) {
	return +rtrim(readfile(`/sys/class/ieee80211/${name}/index`));
}

function phy_path(name) {
	let devpath = realpath(`/sys/class/ieee80211/${name}/device`);

	devpath = replace(devpath, /^\/sys\/devices\//, "");
	if (match(devpath, /^platform\/.*\/pci/))
		devpath = replace(devpath, /^platform\//, "");
	let dev_phys = map(glob(`/sys/class/ieee80211/${name}/device/ieee80211/*`), basename);
	sort(dev_phys, (a, b) => phy_idx(a) - phy_idx(b));

	let ofs = index(dev_phys, name);
	if (ofs > 0)
		devpath += `+${ofs}`;

	return devpath;
}

function cleanup() {
	let wlan = board_data.wlan;

	for (let name in wlan)
		if (substr(name, 0, 3) == "phy")
			delete wlan[name];
		else
			delete wlan[name].info;
}

function wiphy_path_match(entry_path, path) {
	if (type(entry_path) == "array")
		return index(entry_path, path) >= 0;

	return entry_path == path;
}

function wiphy_get_entry(phy, path) {
	board_data.wlan ??= {};

	let wlan = board_data.wlan;
	for (let name in wlan)
		if (wiphy_path_match(wlan[name].path, path))
			return wlan[name];

	wlan[phy] = {
		path: path
	};

	return wlan[phy];
}

function freq_to_channel(freq) {
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
	return 0;
}

function freq_range_match(ranges, freq) {
	freq *= 1000;
	for (let range in ranges) {
		if (freq >= range[0] && freq <= range[1])
			return true;
	}
	return false;
}

function wiphy_detect() {
	let phys = nl.request(nl.const.NL80211_CMD_GET_WIPHY, nl.const.NLM_F_DUMP, { split_wiphy_dump: true });
	if (!phys)
		return;

	for (let phy in phys) {
		if (!phy)
			continue;

		let name = phy.wiphy_name;
		let path = phy_path(name);
		let info = {
			antenna_rx: phy.wiphy_antenna_avail_rx,
			antenna_tx: phy.wiphy_antenna_avail_tx,
			bands: {},
			radios: []
		};

		for (let radio in phy.radios) {
			// S1G is not supported yet
			radio.freq_ranges = filter(radio.freq_ranges,
				(range) => range.end > 2000000
			);

			if (!length(radio.freq_ranges))
				continue;

			push(info.radios, {
				index: radio.index,
				freq_ranges: map(radio.freq_ranges,
					(range) => [ range.start, range.end ]
				),
				bands: {}
			});
		}

		let bands = info.bands;
		for (let band in phy.wiphy_bands) {
			if (!band || !band.freqs)
				continue;
			let freq = band.freqs[0].freq;
			let band_info = {};
			let band_name;
			if (freq > 50000)
				band_name = "60G";
			else if (freq > 5900)
				band_name = "6G";
			else if (freq > 4000)
				band_name = "5G";
			else if (freq > 2000)
				band_name = "2G";
			else
				continue;
			bands[band_name] = band_info;
			if (band.ht_capa > 0)
				band_info.ht = true;
			if (band.vht_capa > 0)
				band_info.vht = true;
			let he_phy_cap = 0;
			let eht_phy_cap = 0;

			for (let ift in band.iftype_data) {
				if (!ift.he_cap_phy)
					continue;

				band_info.he = true;
				he_phy_cap |= ift.he_cap_phy[0];

				if (!ift.eht_cap_phy)
					continue;

				band_info.eht = true;
				eht_phy_cap |= ift.eht_cap_phy[0];
			}

			if (band_name != "2G" &&
			    (he_phy_cap & 0x18) || ((band.vht_capa >> 2) & 0x3))
				band_info.max_width = 160;
			else if (band_name != "2G" &&
			         (he_phy_cap & 4) || band.vht_capa > 0)
				band_info.max_width = 80;
			else if ((band.ht_capa & 0x2) || (he_phy_cap & 0x2))
				band_info.max_width = 40;
			else
				band_info.max_width = 20;

			let modes = band_info.modes = [ "NOHT" ];
			if (band_info.ht)
				push(modes, "HT20");
			if (band_info.vht)
				push(modes, "VHT20");
			if (band_info.he)
				push(modes, "HE20");
			if (band_info.eht)
				push(modes, "EHT20");
			if (band.ht_capa & 0x2) {
				push(modes, "HT40");
				if (band_info.vht)
					push(modes, "VHT40")
			}
			if (he_phy_cap & 2)
				push(modes, "HE40");

			if (eht_phy_cap && he_phy_cap & 2)
				push(modes, "EHT40");

			for (let radio in info.radios) {
				let freq_match = filter(band.freqs,
					(freq) => freq_range_match(radio.freq_ranges, freq.freq)
				);
				if (!length(freq_match))
					continue;

				let radio_band = {};
				radio.bands[band_name] = radio_band;

				freq_match = filter(freq_match,
					(freq) => !freq.disabled
				);

				let freq = freq_match[0];
				if (freq)
					radio_band.default_channel = freq_to_channel(freq.freq);
			}

			for (let freq in band.freqs) {
				if (freq.disabled)
					continue;
				let chan = freq_to_channel(freq.freq);
				if (!chan)
					continue;
				band_info.default_channel = chan;
				break;
			}

			if (band_name == "2G")
				continue;

			if (he_phy_cap & 4)
				push(modes, "HE40");
			if (eht_phy_cap && he_phy_cap & 4)
				push(modes, "EHT40");
			if (band_info.vht)
				push(modes, "VHT80");
			if (he_phy_cap & 4)
				push(modes, "HE80");
			if (eht_phy_cap && he_phy_cap & 4)
				push(modes, "EHT80");
			if ((band.vht_capa >> 2) & 0x3)
				push(modes, "VHT160");
			if (he_phy_cap & 0x18)
				push(modes, "HE160");
			if (eht_phy_cap && he_phy_cap & 0x18)
				push(modes, "EHT160");

			if (eht_phy_cap & 2)
				push(modes, "EHT320");
		}

		let entry = wiphy_get_entry(name, path);
		entry.info = info;
	}
}

cleanup();
wiphy_detect();
if (!is_equal(prev_board_data, board_data)) {
	let new_file = board_file + ".new";
	unlink(new_file);
	let f = open(new_file, "wx");
	if (!f)
		exit(1);
	f.write(sprintf("%.J\n", board_data));
	f.close();
	rename(new_file, board_file);
}
