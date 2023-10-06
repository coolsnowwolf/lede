import * as nl80211 from "nl80211";
import * as rtnl from "rtnl";
import { readfile, glob, basename, readlink } from "fs";

const iftypes = {
	ap: nl80211.const.NL80211_IFTYPE_AP,
	mesh: nl80211.const.NL80211_IFTYPE_MESH_POINT,
	sta: nl80211.const.NL80211_IFTYPE_STATION,
	adhoc: nl80211.const.NL80211_IFTYPE_ADHOC,
	monitor: nl80211.const.NL80211_IFTYPE_MONITOR,
};

function wdev_remove(name)
{
	nl80211.request(nl80211.const.NL80211_CMD_DEL_INTERFACE, 0, { dev: name });
}

function __phy_is_fullmac(phyidx)
{
	let data = nl80211.request(nl80211.const.NL80211_CMD_GET_WIPHY, 0, { wiphy: phyidx });

	return !data.software_iftypes.ap_vlan;
}

function phy_is_fullmac(phy)
{
	let phyidx = int(trim(readfile(`/sys/class/ieee80211/${phy}/index`)));

	return __phy_is_fullmac(phyidx);
}

function find_reusable_wdev(phyidx)
{
	if (!__phy_is_fullmac(phyidx))
		return null;

	let data = nl80211.request(
		nl80211.const.NL80211_CMD_GET_INTERFACE,
		nl80211.const.NLM_F_DUMP,
		{ wiphy: phyidx });
	for (let res in data)
		if (trim(readfile(`/sys/class/net/${res.ifname}/operstate`)) == "down")
			return res.ifname;
	return null;
}

function wdev_create(phy, name, data)
{
	let phyidx = int(readfile(`/sys/class/ieee80211/${phy}/index`));

	wdev_remove(name);

	if (!iftypes[data.mode])
		return `Invalid mode: ${data.mode}`;

	let req = {
		wiphy: phyidx,
		ifname: name,
		iftype: iftypes[data.mode],
	};

	if (data["4addr"])
		req["4addr"] = data["4addr"];
	if (data.macaddr)
		req.mac = data.macaddr;

	nl80211.error();

	let reuse_ifname = find_reusable_wdev(phyidx);
	if (reuse_ifname &&
	    (reuse_ifname == name ||
	     rtnl.request(rtnl.const.RTM_SETLINK, 0, { dev: reuse_ifname, ifname: name}) != false))
		nl80211.request(
			nl80211.const.NL80211_CMD_SET_INTERFACE, 0, {
				wiphy: phyidx,
				dev: name,
				iftype: iftypes[data.mode],
			});
	else
		nl80211.request(
			nl80211.const.NL80211_CMD_NEW_INTERFACE,
			nl80211.const.NLM_F_CREATE,
			req);

	let error = nl80211.error();
	if (error)
		return error;

	if (data.powersave != null) {
		nl80211.request(nl80211.const.NL80211_CMD_SET_POWER_SAVE, 0,
			{ dev: name, ps_state: data.powersave ? 1 : 0});
	}

	return null;
}

function phy_sysfs_file(phy, name)
{
	return trim(readfile(`/sys/class/ieee80211/${phy}/${name}`));
}

function macaddr_split(str)
{
	return map(split(str, ":"), (val) => hex(val));
}

function macaddr_join(addr)
{
	return join(":", map(addr, (val) => sprintf("%02x", val)));
}

function wdev_macaddr(wdev)
{
	return trim(readfile(`/sys/class/net/${wdev}/address`));
}

const phy_proto = {
	macaddr_init: function(used, options) {
		this.macaddr_options = options ?? {};
		this.macaddr_list = {};

		if (type(used) == "object")
			for (let addr in used)
				this.macaddr_list[addr] = used[addr];
		else
			for (let addr in used)
				this.macaddr_list[addr] = -1;

		this.for_each_wdev((wdev) => {
			let macaddr = wdev_macaddr(wdev);
			this.macaddr_list[macaddr] ??= -1;
		});

		return this.macaddr_list;
	},

	macaddr_generate: function(data) {
		let phy = this.name;
		let idx = int(data.id ?? 0);
		let mbssid = int(data.mbssid ?? 0) > 0;
		let num_global = int(data.num_global ?? 1);
		let use_global = !mbssid && idx < num_global;

		let base_addr = phy_sysfs_file(phy, "macaddress");
		if (!base_addr)
			return null;

		if (!idx && !mbssid)
			return base_addr;

		let base_mask = phy_sysfs_file(phy, "address_mask");
		if (!base_mask)
			return null;

		if (base_mask == "00:00:00:00:00:00" && idx >= num_global) {
			let addrs = split(phy_sysfs_file(phy, "addresses"), "\n");

			if (idx < length(addrs))
				return addrs[idx];

			base_mask = "ff:ff:ff:ff:ff:ff";
		}

		let addr = macaddr_split(base_addr);
		let mask = macaddr_split(base_mask);
		let type;

		if (mbssid)
			type = "b5";
		else if (use_global)
			type = "add";
		else if (mask[0] > 0)
			type = "b1";
		else if (mask[5] < 0xff)
			type = "b5";
		else
			type = "add";

		switch (type) {
		case "b1":
			if (!(addr[0] & 2))
				idx--;
			addr[0] |= 2;
			addr[0] ^= idx << 2;
			break;
		case "b5":
			if (mbssid)
				addr[0] |= 2;
			addr[5] ^= idx;
			break;
		default:
			for (let i = 5; i > 0; i--) {
				addr[i] += idx;
				if (addr[i] < 256)
					break;
				addr[i] %= 256;
			}
			break;
		}

		return macaddr_join(addr);
	},

	macaddr_next: function(val) {
		let data = this.macaddr_options ?? {};
		let list = this.macaddr_list;

		for (let i = 0; i < 32; i++) {
			data.id = i;

			let mac = this.macaddr_generate(data);
			if (!mac)
				return null;

			if (list[mac] != null)
				continue;

			list[mac] = val != null ? val : -1;
			return mac;
		}
	},

	for_each_wdev: function(cb) {
		let wdevs = glob(`/sys/class/ieee80211/${this.name}/device/net/*`);
		wdevs = map(wdevs, (arg) => basename(arg));
		for (let wdev in wdevs) {
			if (basename(readlink(`/sys/class/net/${wdev}/phy80211`)) != this.name)
				continue;

			cb(wdev);
		}
	}
};

function phy_open(phy)
{
	let phyidx = readfile(`/sys/class/ieee80211/${phy}/index`);
	if (!phyidx)
		return null;

	return proto({
		name: phy,
		idx: int(phyidx)
	}, phy_proto);
}

const vlist_proto = {
	update: function(values, arg) {
		let data = this.data;
		let cb = this.cb;
		let seq = { };
		let new_data = {};
		let old_data = {};

		this.data = new_data;

		if (type(values) == "object") {
			for (let key in values) {
				old_data[key] = data[key];
				new_data[key] = values[key];
				delete data[key];
			}
		} else {
			for (let val in values) {
				let cur_key = val[0];
				let cur_obj = val[1];

				old_data[cur_key] = data[cur_key];
				new_data[cur_key] = val[1];
				delete data[cur_key];
			}
		}

		for (let key in data) {
			cb(null, data[key], arg);
			delete data[key];
		}
		for (let key in new_data)
			cb(new_data[key], old_data[key], arg);
	}
};

function is_equal(val1, val2) {
	let t1 = type(val1);

	if (t1 != type(val2))
		return false;

	if (t1 == "array") {
		if (length(val1) != length(val2))
			return false;

		for (let i = 0; i < length(val1); i++)
			if (!is_equal(val1[i], val2[i]))
				return false;

		return true;
	} else if (t1 == "object") {
		for (let key in val1)
			if (!is_equal(val1[key], val2[key]))
				return false;
		for (let key in val2)
			if (val1[key] == null)
				return false;
		return true;
	} else {
		return val1 == val2;
	}
}

function vlist_new(cb) {
	return proto({
			cb: cb,
			data: {}
		}, vlist_proto);
}

export { wdev_remove, wdev_create, is_equal, vlist_new, phy_is_fullmac, phy_open };
