'use strict';

import { readfile, realpath, lsdir } from "fs";
import * as nl80211 from "nl80211";

function phy_filename(phy, name) {
	return `/sys/class/ieee80211/${phy}/${name}`;
}

function phy_file(phy, name) {
	return trim(readfile(phy_filename(phy, name)));
}

function phy_index(phy) {
	return +phy_file(phy, "index");
}

function phy_path_match(phy, path) {
	let phy_path = realpath(phy_filename(phy, "device"));
	return substr(phy_path, -length(path)) == path;
}

function phy_paths(path) {
	return type(path) == "array" ? path : [ path ];
}

function phy_path_match_any(phy, paths) {
	for (let path in phy_paths(paths)) {
		if (!path)
			continue;
		if (phy_path_match(phy, split(path, "+")[0]))
			return true;
	}

	return false;
}

function __find_phy_by_path(phys, paths) {
	if (!paths)
		return null;

	for (let path in phy_paths(paths)) {
		if (!path)
			continue;
		path = split(path, "+");
		let match = filter(phys, (phy) => phy_path_match(phy, path[0]));
		match = sort(match, (a, b) => phy_index(a) - phy_index(b));
		match = match[+path[1]];
		if (match)
			return match;
	}

	return null;
}

function find_phy_by_macaddr(phys, macaddr) {
	macaddr = lc(macaddr);
	return filter(phys, (phy) => phy_file(phy, "macaddress") == macaddr)[0];
}

function rename_phy_by_name(phys, name, rename) {
	let data = json(readfile("/etc/board.json")).wlan;
	if (!data)
		return;

	data = data[name];
	if (!data)
		return;

	let prev_name = __find_phy_by_path(phys, data.path);
	if (!prev_name)
		return;

	if (!rename)
		return true;

	let idx = phy_index(prev_name);
	nl80211.request(nl80211.const.NL80211_CMD_SET_WIPHY, 0, {
		wiphy: idx,
		wiphy_name: name
	});

	let prev_idx = index(phys, prev_name);
	if (prev_idx >= 0)
		phys[prev_idx] = name;

	return true;
}

function find_phy_by_path(phys, path) {
	let name = __find_phy_by_path(phys, path);
	if (!name)
		return;

	let data = json(readfile("/etc/board.json")).wlan;
	if (!data || data[name])
		return name;

	for (let cur_name, cur_data in data) {
		if (!phy_path_match_any(name, cur_data.path))
			continue;

		let idx = phy_index(name);
		nl80211.request(nl80211.const.NL80211_CMD_SET_WIPHY, 0, {
			wiphy: idx,
			wiphy_name: cur_name
		});

		return cur_name;
	}

	return name;
}

function find_phy_by_name(phys, name, rename) {
	if (index(phys, name) >= 0)
		return name;

	if (!rename_phy_by_name(phys, name, rename))
		return;

	if (!rename)
		return name;

	return index(phys, name) < 0 ? null : name;
}

export function find_phy(config, rename) {
	let phys = lsdir("/sys/class/ieee80211");

	return find_phy_by_path(phys, config.path) ??
	       find_phy_by_macaddr(phys, config.macaddr) ??
	       find_phy_by_name(phys, config.phy, rename);
};
