import * as nl80211 from "nl80211";
import * as rtnl from "rtnl";
import { readfile, glob, basename, readlink, open } from "fs";

const iftypes = {
	ap: nl80211.const.NL80211_IFTYPE_AP,
	mesh: nl80211.const.NL80211_IFTYPE_MESH_POINT,
	sta: nl80211.const.NL80211_IFTYPE_STATION,
	adhoc: nl80211.const.NL80211_IFTYPE_ADHOC,
	monitor: nl80211.const.NL80211_IFTYPE_MONITOR,
};

const mesh_params = {
	mesh_retry_timeout: "retry_timeout",
	mesh_confirm_timeout: "confirm_timeout",
	mesh_holding_timeout: "holding_timeout",
	mesh_max_peer_links: "max_peer_links",
	mesh_max_retries: "max_retries",
	mesh_ttl: "ttl",
	mesh_element_ttl: "element_ttl",
	mesh_auto_open_plinks: "auto_open_plinks",
	mesh_hwmp_max_preq_retries: "hwmp_max_preq_retries",
	mesh_path_refresh_time: "path_refresh_time",
	mesh_min_discovery_timeout: "min_discovery_timeout",
	mesh_hwmp_active_path_timeout: "hwmp_active_path_timeout",
	mesh_hwmp_preq_min_interval: "hwmp_preq_min_interval",
	mesh_hwmp_net_diameter_traversal_time: "hwmp_net_diam_trvs_time",
	mesh_hwmp_rootmode: "hwmp_rootmode",
	mesh_hwmp_rann_interval: "hwmp_rann_interval",
	mesh_gate_announcements: "gate_announcements",
	mesh_sync_offset_max_neighor: "sync_offset_max_neighbor",
	mesh_rssi_threshold: "rssi_threshold",
	mesh_hwmp_active_path_to_root_timeout: "hwmp_path_to_root_timeout",
	mesh_hwmp_root_interval: "hwmp_root_interval",
	mesh_hwmp_confirmation_interval: "hwmp_confirmation_interval",
	mesh_awake_window: "awake_window",
	mesh_plink_timeout: "plink_timeout",
	mesh_fwding: "forwarding",
	mesh_power_mode: "power_mode",
	mesh_nolearn: "nolearn"
};

function wdev_remove(name)
{
	nl80211.request(nl80211.const.NL80211_CMD_DEL_INTERFACE, 0, { dev: name });
}

function __phy_is_fullmac(phyidx)
{
	let data = nl80211.request(nl80211.const.NL80211_CMD_GET_WIPHY, 0, { wiphy: phyidx });

	return !data.software_iftypes.monitor;
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

function wdev_set_radio_mask(name, mask)
{
	nl80211.request(nl80211.const.NL80211_CMD_SET_INTERFACE, 0, {
		dev: name,
		vif_radio_mask: mask
	});
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
	if (data.radio_mask > 0)
		req.vif_radio_mask = data.radio_mask;
	else if (data.radio != null && data.radio >= 0)
		req.vif_radio_mask = 1 << data.radio;

	nl80211.error();

	let reuse_ifname = find_reusable_wdev(phyidx);
	if (reuse_ifname &&
	    (reuse_ifname == name ||
	     rtnl.request(rtnl.const.RTM_SETLINK, 0, { dev: reuse_ifname, ifname: name}) != false)) {
		req.dev = req.ifname;
		delete req.ifname;
		nl80211.request(nl80211.const.NL80211_CMD_SET_INTERFACE, 0, req);
	} else {
		nl80211.request(
			nl80211.const.NL80211_CMD_NEW_INTERFACE,
			nl80211.const.NLM_F_CREATE,
			req);
	}

	let error = nl80211.error();
	if (error)
		return error;

	if (data.powersave != null) {
		nl80211.request(nl80211.const.NL80211_CMD_SET_POWER_SAVE, 0,
			{ dev: name, ps_state: data.powersave ? 1 : 0});
	}

	return null;
}

function wdev_set_mesh_params(name, data)
{
	let mesh_cfg = {};

	for (let key in mesh_params) {
		let val = data[key];
		if (val == null)
			continue;
		mesh_cfg[mesh_params[key]] = int(val);
	}

	if (!length(mesh_cfg))
		return null;

	nl80211.request(nl80211.const.NL80211_CMD_SET_MESH_CONFIG, 0,
		{ dev: name, mesh_params: mesh_cfg });

	return nl80211.error();
}

function wdev_set_up(name, up)
{
	rtnl.request(rtnl.const.RTM_SETLINK, 0, { dev: name, change: 1, flags: up ? 1 : 0 });
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
		let phy = this.phy;
		let radio_idx = this.radio;
		let idx = int(data.id ?? 0);
		let mbssid = int(data.mbssid ?? 0) > 0;
		let num_global = int(data.num_global ?? 1);
		let use_global = !mbssid && idx < num_global;

		let base_addr = phy_sysfs_file(phy, "macaddress");
		if (!base_addr)
			return null;

		let base_mask = phy_sysfs_file(phy, "address_mask");
		if (!base_mask)
			return null;

		if (base_mask == "00:00:00:00:00:00")
			base_mask = "ff:ff:ff:ff:ff:ff";

		if (data.macaddr_base)
			base_addr = data.macaddr_base;
		else if (base_mask == "ff:ff:ff:ff:ff:ff" &&
		    (radio_idx > 0 || idx >= num_global)) {
			let addrs = split(phy_sysfs_file(phy, "addresses"), "\n");

			if (radio_idx != null) {
				if (radio_idx && radio_idx < length(addrs))
					base_addr = addrs[radio_idx];
				else
					idx += radio_idx * 16;
			} else {
				if (idx < length(addrs))
					return addrs[idx];
			}
		}

		if (!idx && !mbssid)
			return base_addr;

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

	macaddr_next: function(val, reuse) {
		let data = this.macaddr_options ?? {};
		let list = this.macaddr_list;
		let addr;

		for (let i = 0; i < 32; i++) {
			data.id = i;

			let mac = this.macaddr_generate(data);
			if (!mac)
				break;

			if (mac == reuse) {
				addr = mac;
				break;
			}

			if (addr != null || list[mac] != null)
				continue;

			addr = mac;
			if (reuse == null)
				break;
		}

		if (addr == null)
			return null;

		list[addr] = val != null ? val : -1;

		return addr;
	},

	wdev_add: function(name, data) {
		return wdev_create(this.phy, name, {
			...data,
			radio: this.radio,
		});
	},

	for_each_wdev: function(cb) {
		let wdevs = nl80211.request(
			nl80211.const.NL80211_CMD_GET_INTERFACE,
			nl80211.const.NLM_F_DUMP,
			{ wiphy: this.idx }
		);

		let mac_wdev = {};
		for (let wdev in wdevs) {
			if (wdev.iftype == nl80211.const.NL80211_IFTYPE_AP_VLAN)
				continue;
			if (this.radio != null && wdev.vif_radio_mask != null &&
			    !(wdev.vif_radio_mask & (1 << this.radio)))
				continue;
			mac_wdev[wdev.mac] = wdev;
		}

		for (let wdev in wdevs) {
			if (!mac_wdev[wdev.mac])
				continue;

			cb(wdev.ifname);
		}
	}
};

function phy_open(phy, radio)
{
	let phyidx = readfile(`/sys/class/ieee80211/${phy}/index`);
	if (!phyidx)
		return null;

	let name = phy;
	if (radio === "" || radio < 0)
		radio = null;
	if (radio != null)
		name += "." + radio;

	return proto({
		phy, name, radio,
		idx: int(phyidx),
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

export { wdev_remove, wdev_create, wdev_set_mesh_params, wdev_set_radio_mask, wdev_set_up, is_equal, vlist_new, phy_is_fullmac, phy_open };
