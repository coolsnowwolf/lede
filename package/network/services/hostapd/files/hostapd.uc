let libubus = require("ubus");
import { open, readfile } from "fs";
import { wdev_create, wdev_remove, is_equal, vlist_new, phy_is_fullmac, phy_open } from "common";

let ubus = libubus.connect();

hostapd.data.config = {};

hostapd.data.file_fields = {
	vlan_file: true,
	wpa_psk_file: true,
	accept_mac_file: true,
	deny_mac_file: true,
	eap_user_file: true,
	ca_cert: true,
	server_cert: true,
	server_cert2: true,
	private_key: true,
	private_key2: true,
	dh_file: true,
	eap_sim_db: true,
};

function iface_remove(cfg)
{
	if (!cfg || !cfg.bss || !cfg.bss[0] || !cfg.bss[0].ifname)
		return;

	for (let bss in cfg.bss)
		wdev_remove(bss.ifname);
}

function iface_gen_config(phy, config, start_disabled)
{
	let str = `data:
${join("\n", config.radio.data)}
channel=${config.radio.channel}
`;

	for (let i = 0; i < length(config.bss); i++) {
		let bss = config.bss[i];
		let type = i > 0 ? "bss" : "interface";
		let nasid = bss.nasid ?? replace(bss.bssid, ":", "");

		str += `
${type}=${bss.ifname}
bssid=${bss.bssid}
${join("\n", bss.data)}
nas_identifier=${nasid}
`;
		if (start_disabled)
			str += `
start_disabled=1
`;
	}

	return str;
}

function iface_freq_info(iface, config, params)
{
	let freq = params.frequency;
	if (!freq)
		return null;

	let sec_offset = params.sec_chan_offset;
	if (sec_offset != -1 && sec_offset != 1)
		sec_offset = 0;

	let width = 0;
	for (let line in config.radio.data) {
		if (!sec_offset && match(line, /^ht_capab=.*HT40/)) {
			sec_offset = null; // auto-detect
			continue;
		}

		let val = match(line, /^(vht_oper_chwidth|he_oper_chwidth)=(\d+)/);
		if (!val)
			continue;

		val = int(val[2]);
		if (val > width)
			width = val;
	}

	if (freq < 4000)
		width = 0;

	return hostapd.freq_info(freq, sec_offset, width);
}

function iface_add(phy, config, phy_status)
{
	let config_inline = iface_gen_config(phy, config, !!phy_status);

	let bss = config.bss[0];
	let ret = hostapd.add_iface(`bss_config=${phy}:${config_inline}`);
	if (ret < 0)
		return false;

	if (!phy_status)
		return true;

	let iface = hostapd.interfaces[phy];
	if (!iface)
		return false;

	let freq_info = iface_freq_info(iface, config, phy_status);

	return iface.start(freq_info) >= 0;
}

function iface_config_macaddr_list(config)
{
	let macaddr_list = {};
	for (let i = 0; i < length(config.bss); i++) {
		let bss = config.bss[i];
		if (!bss.default_macaddr)
			macaddr_list[bss.bssid] = i;
	}

	return macaddr_list;
}

function iface_update_supplicant_macaddr(phy, config)
{
	let macaddr_list = [];
	for (let i = 0; i < length(config.bss); i++)
		push(macaddr_list, config.bss[i].bssid);
	ubus.call("wpa_supplicant", "phy_set_macaddr_list", { phy: phy, macaddr: macaddr_list });
}

function iface_restart(phydev, config, old_config)
{
	let phy = phydev.name;

	hostapd.remove_iface(phy);
	iface_remove(old_config);
	iface_remove(config);

	if (!config.bss || !config.bss[0]) {
		hostapd.printf(`No bss for phy ${phy}`);
		return;
	}

	phydev.macaddr_init(iface_config_macaddr_list(config));
	for (let i = 0; i < length(config.bss); i++) {
		let bss = config.bss[i];
		if (bss.default_macaddr)
			bss.bssid = phydev.macaddr_next();
	}

	iface_update_supplicant_macaddr(phy, config);

	let bss = config.bss[0];
	let err = wdev_create(phy, bss.ifname, { mode: "ap" });
	if (err)
		hostapd.printf(`Failed to create ${bss.ifname} on phy ${phy}: ${err}`);

	let ubus = hostapd.data.ubus;
	let phy_status = ubus.call("wpa_supplicant", "phy_status", { phy: phy });
	if (phy_status && phy_status.state == "COMPLETED") {
		if (iface_add(phy, config, phy_status))
			return;

		hostapd.printf(`Failed to bring up phy ${phy} ifname=${bss.ifname} with supplicant provided frequency`);
	}

	ubus.call("wpa_supplicant", "phy_set_state", { phy: phy, stop: true });
	if (!iface_add(phy, config))
		hostapd.printf(`hostapd.add_iface failed for phy ${phy} ifname=${bss.ifname}`);
	ubus.call("wpa_supplicant", "phy_set_state", { phy: phy, stop: false });
}

function array_to_obj(arr, key, start)
{
	let obj = {};

	start ??= 0;
	for (let i = start; i < length(arr); i++) {
		let cur = arr[i];
		obj[cur[key]] = cur;
	}

	return obj;
}

function find_array_idx(arr, key, val)
{
	for (let i = 0; i < length(arr); i++)
		if (arr[i][key] == val)
			return i;

	return -1;
}

function bss_reload_psk(bss, config, old_config)
{
	if (is_equal(old_config.hash.wpa_psk_file, config.hash.wpa_psk_file))
		return;

	old_config.hash.wpa_psk_file = config.hash.wpa_psk_file;
	if (!is_equal(old_config, config))
		return;

	let ret = bss.ctrl("RELOAD_WPA_PSK");
	ret ??= "failed";

	hostapd.printf(`Reload WPA PSK file for bss ${config.ifname}: ${ret}`);
}

function remove_file_fields(config)
{
	return filter(config, (line) => !hostapd.data.file_fields[split(line, "=")[0]]);
}

function bss_remove_file_fields(config)
{
	let new_cfg = {};

	for (let key in config)
		new_cfg[key] = config[key];
	new_cfg.data = remove_file_fields(new_cfg.data);
	new_cfg.hash = {};
	for (let key in config.hash)
		new_cfg.hash[key] = config.hash[key];
	delete new_cfg.hash.wpa_psk_file;
	delete new_cfg.hash.vlan_file;

	return new_cfg;
}

function bss_config_hash(config)
{
	return hostapd.sha1(remove_file_fields(config) + "");
}

function bss_find_existing(config, prev_config, prev_hash)
{
	let hash = bss_config_hash(config.data);

	for (let i = 0; i < length(prev_config.bss); i++) {
		if (!prev_hash[i] || hash != prev_hash[i])
			continue;

		prev_hash[i] = null;
		return i;
	}

	return -1;
}

function get_config_bss(config, idx)
{
	if (!config.bss[idx]) {
		hostapd.printf(`Invalid bss index ${idx}`);
		return null;
	}

	let ifname = config.bss[idx].ifname;
	if (!ifname)
		hostapd.printf(`Could not find bss ${config.bss[idx].ifname}`);

	return hostapd.bss[ifname];
}

function iface_reload_config(phydev, config, old_config)
{
	let phy = phydev.name;

	if (!old_config || !is_equal(old_config.radio, config.radio))
		return false;

	if (is_equal(old_config.bss, config.bss))
		return true;

	if (!old_config.bss || !old_config.bss[0])
		return false;

	let iface = hostapd.interfaces[phy];
	if (!iface) {
		hostapd.printf(`Could not find previous interface ${iface_name}`);
		return false;
	}

	let iface_name = old_config.bss[0].ifname;
	let first_bss = hostapd.bss[iface_name];
	if (!first_bss) {
		hostapd.printf(`Could not find bss of previous interface ${iface_name}`);
		return false;
	}

	let macaddr_list = iface_config_macaddr_list(config);
	let bss_list = [];
	let bss_list_cfg = [];
	let prev_bss_hash = [];

	for (let bss in old_config.bss) {
		let hash = bss_config_hash(bss.data);
		push(prev_bss_hash, bss_config_hash(bss.data));
	}

	// Step 1: find (possibly renamed) interfaces with the same config
	// and store them in the new order (with gaps)
	for (let i = 0; i < length(config.bss); i++) {
		let prev;

		// For fullmac devices, the first interface needs to be preserved,
		// since it's treated as the master
		if (!i && phy_is_fullmac(phy)) {
			prev = 0;
			prev_bss_hash[0] = null;
		} else {
			prev = bss_find_existing(config.bss[i], old_config, prev_bss_hash);
		}
		if (prev < 0)
			continue;

		let cur_config = config.bss[i];
		let prev_config = old_config.bss[prev];

		let prev_bss = get_config_bss(old_config, prev);
		if (!prev_bss)
			return false;

		// try to preserve MAC address of this BSS by reassigning another
		// BSS if necessary
		if (cur_config.default_macaddr &&
		    !macaddr_list[prev_config.bssid]) {
			macaddr_list[prev_config.bssid] = i;
			cur_config.bssid = prev_config.bssid;
		}

		bss_list[i] = prev_bss;
		bss_list_cfg[i] = old_config.bss[prev];
	}

	if (config.mbssid && !bss_list_cfg[0]) {
		hostapd.printf("First BSS changed with MBSSID enabled");
		return false;
	}

	// Step 2: if none were found, rename and preserve the first one
	if (length(bss_list) == 0) {
		// can't change the bssid of the first bss
		if (config.bss[0].bssid != old_config.bss[0].bssid) {
			if (!config.bss[0].default_macaddr) {
				hostapd.printf(`BSSID of first interface changed: ${lc(old_config.bss[0].bssid)} -> ${lc(config.bss[0].bssid)}`);
				return false;
			}

			config.bss[0].bssid = old_config.bss[0].bssid;
		}

		let prev_bss = get_config_bss(old_config, 0);
		if (!prev_bss)
			return false;

		macaddr_list[config.bss[0].bssid] = 0;
		bss_list[0] = prev_bss;
		bss_list_cfg[0] = old_config.bss[0];
		prev_bss_hash[0] = null;
	}

	// Step 3: delete all unused old interfaces
	for (let i = 0; i < length(prev_bss_hash); i++) {
		if (!prev_bss_hash[i])
			continue;

		let prev_bss = get_config_bss(old_config, i);
		if (!prev_bss)
			return false;

		let ifname = old_config.bss[i].ifname;
		hostapd.printf(`Remove bss '${ifname}' on phy '${phy}'`);
		prev_bss.delete();
		wdev_remove(ifname);
	}

	// Step 4: rename preserved interfaces, use temporary name on duplicates
	let rename_list = [];
	for (let i = 0; i < length(bss_list); i++) {
		if (!bss_list[i])
			continue;

		let old_ifname = bss_list_cfg[i].ifname;
		let new_ifname = config.bss[i].ifname;
		if (old_ifname == new_ifname)
			continue;

		if (hostapd.bss[new_ifname]) {
			new_ifname = "tmp_" + substr(hostapd.sha1(new_ifname), 0, 8);
			push(rename_list, i);
		}

		hostapd.printf(`Rename bss ${old_ifname} to ${new_ifname}`);
		if (!bss_list[i].rename(new_ifname)) {
			hostapd.printf(`Failed to rename bss ${old_ifname} to ${new_ifname}`);
			return false;
		}

		bss_list_cfg[i].ifname = new_ifname;
	}

	// Step 5: rename interfaces with temporary names
	for (let i in rename_list) {
		let new_ifname = config.bss[i].ifname;
		if (!bss_list[i].rename(new_ifname)) {
			hostapd.printf(`Failed to rename bss to ${new_ifname}`);
			return false;
		}
		bss_list_cfg[i].ifname = new_ifname;
	}

	// Step 6: assign BSSID for newly created interfaces
	let macaddr_data = {
		num_global: config.num_global_macaddr ?? 1,
		mbssid: config.mbssid ?? 0,
	};
	macaddr_list = phydev.macaddr_init(macaddr_list, macaddr_data);
	for (let i = 0; i < length(config.bss); i++) {
		if (bss_list[i])
			continue;
		let bsscfg = config.bss[i];

		let mac_idx = macaddr_list[bsscfg.bssid];
		if (mac_idx < 0)
			macaddr_list[bsscfg.bssid] = i;
		if (mac_idx == i)
			continue;

		// statically assigned bssid of the new interface is in conflict
		// with the bssid of a reused interface. reassign the reused interface
		if (!bsscfg.default_macaddr) {
			// can't update bssid of the first BSS, need to restart
			if (!mac_idx < 0)
				return false;

			bsscfg = config.bss[mac_idx];
		}

		let addr = phydev.macaddr_next(i);
		if (!addr) {
			hostapd.printf(`Failed to generate mac address for phy ${phy}`);
			return false;
		}
		bsscfg.bssid = addr;
	}

	let config_inline = iface_gen_config(phy, config);

	// Step 7: fill in the gaps with new interfaces
	for (let i = 0; i < length(config.bss); i++) {
		let ifname = config.bss[i].ifname;
		let bss = bss_list[i];

		if (bss)
			continue;

		hostapd.printf(`Add bss ${ifname} on phy ${phy}`);
		bss_list[i] = iface.add_bss(config_inline, i);
		if (!bss_list[i]) {
			hostapd.printf(`Failed to add new bss ${ifname} on phy ${phy}`);
			return false;
		}
	}

	// Step 8: update interface bss order
	if (!iface.set_bss_order(bss_list)) {
		hostapd.printf(`Failed to update BSS order on phy '${phy}'`);
		return false;
	}

	// Step 9: update config
	for (let i = 0; i < length(config.bss); i++) {
		if (!bss_list_cfg[i])
			continue;

		let ifname = config.bss[i].ifname;
		let bss = bss_list[i];

		if (is_equal(config.bss[i], bss_list_cfg[i]))
			continue;

		if (is_equal(bss_remove_file_fields(config.bss[i]),
		             bss_remove_file_fields(bss_list_cfg[i]))) {
			hostapd.printf(`Update config data files for bss ${ifname}`);
			if (bss.set_config(config_inline, i, true) < 0) {
				hostapd.printf(`Could not update config data files for bss ${ifname}`);
				return false;
			} else {
				bss.ctrl("RELOAD_WPA_PSK");
				continue;
			}
		}

		bss_reload_psk(bss, config.bss[i], bss_list_cfg[i]);
		if (is_equal(config.bss[i], bss_list_cfg[i]))
			continue;

		hostapd.printf(`Reload config for bss '${config.bss[0].ifname}' on phy '${phy}'`);
		if (bss.set_config(config_inline, i) < 0) {
			hostapd.printf(`Failed to set config for bss ${ifname}`);
			return false;
		}
	}

	return true;
}

function iface_set_config(phy, config)
{
	let old_config = hostapd.data.config[phy];

	hostapd.data.config[phy] = config;

	if (!config) {
		hostapd.remove_iface(phy);
		return iface_remove(old_config);
	}

	let phydev = phy_open(phy);
	if (!phydev) {
		hostapd.printf(`Failed to open phy ${phy}`);
		return false;
	}

	try {
		let ret = iface_reload_config(phydev, config, old_config);
		if (ret) {
			iface_update_supplicant_macaddr(phy, config);
			hostapd.printf(`Reloaded settings for phy ${phy}`);
			return 0;
		}
	} catch (e) {
			hostapd.printf(`Error reloading config: ${e}\n${e.stacktrace[0].context}`);
	}

	hostapd.printf(`Restart interface for phy ${phy}`);
	let ret = iface_restart(phydev, config, old_config);

	return ret;
}

function config_add_bss(config, name)
{
	let bss = {
		ifname: name,
		data: [],
		hash: {}
	};

	push(config.bss, bss);

	return bss;
}

function iface_load_config(filename)
{
	let f = open(filename, "r");
	if (!f)
		return null;

	let config = {
		radio: {
			data: []
		},
		bss: [],
		orig_file: filename,
	};

	let bss;
	let line;
	while ((line = trim(f.read("line"))) != null) {
		let val = split(line, "=", 2);
		if (!val[0])
			continue;

		if (val[0] == "interface") {
			bss = config_add_bss(config, val[1]);
			break;
		}

		if (val[0] == "channel") {
			config.radio.channel = val[1];
			continue;
		}

		if (val[0] == "#num_global_macaddr" ||
		    val[0] == "mbssid")
			config[val[0]] = int(val[1]);

		push(config.radio.data, line);
	}

	while ((line = trim(f.read("line"))) != null) {
		if (line == "#default_macaddr")
			bss.default_macaddr = true;

		let val = split(line, "=", 2);
		if (!val[0])
			continue;

		if (val[0] == "bssid") {
			bss.bssid = lc(val[1]);
			continue;
		}

		if (val[0] == "nas_identifier")
			bss.nasid = val[1];

		if (val[0] == "bss") {
			bss = config_add_bss(config, val[1]);
			continue;
		}

		if (hostapd.data.file_fields[val[0]])
			bss.hash[val[0]] = hostapd.sha1(readfile(val[1]));

		push(bss.data, line);
	}
	f.close();

	return config;
}

function ex_wrap(func) {
	return (req) => {
		try {
			let ret = func(req);
			return ret;
		} catch(e) {
			hostapd.printf(`Exception in ubus function: ${e}\n${e.stacktrace[0].context}`);
		}
		return libubus.STATUS_UNKNOWN_ERROR;
	};
}

let main_obj = {
	reload: {
		args: {
			phy: "",
		},
		call: ex_wrap(function(req) {
			let phy_list = req.args.phy ? [ req.args.phy ] : keys(hostapd.data.config);
			for (let phy_name in phy_list) {
				let phy = hostapd.data.config[phy_name];
				let config = iface_load_config(phy.orig_file);
				iface_set_config(phy_name, config);
			}

			return 0;
		})
	},
	apsta_state: {
		args: {
			phy: "",
			up: true,
			frequency: 0,
			sec_chan_offset: 0,
			csa: true,
			csa_count: 0,
		},
		call: ex_wrap(function(req) {
			if (req.args.up == null || !req.args.phy)
				return libubus.STATUS_INVALID_ARGUMENT;

			let phy = req.args.phy;
			let config = hostapd.data.config[phy];
			if (!config || !config.bss || !config.bss[0] || !config.bss[0].ifname)
				return 0;

			let iface = hostapd.interfaces[phy];
			if (!iface)
				return 0;

			if (!req.args.up) {
				iface.stop();
				return 0;
			}

			if (!req.args.frequency)
				return libubus.STATUS_INVALID_ARGUMENT;

			let freq_info = iface_freq_info(iface, config, req.args);
			if (!freq_info)
				return libubus.STATUS_UNKNOWN_ERROR;

			let ret;
			if (req.args.csa) {
				freq_info.csa_count = req.args.csa_count ?? 10;
				ret = iface.switch_channel(freq_info);
			} else {
				ret = iface.start(freq_info);
			}
			if (!ret)
				return libubus.STATUS_UNKNOWN_ERROR;

			return 0;
		})
	},
	config_get_macaddr_list: {
		args: {
			phy: ""
		},
		call: ex_wrap(function(req) {
			let phy = req.args.phy;
			if (!phy)
				return libubus.STATUS_INVALID_ARGUMENT;

			let ret = {
				macaddr: [],
			};

			let config = hostapd.data.config[phy];
			if (!config)
				return ret;

			ret.macaddr = map(config.bss, (bss) => bss.bssid);
			return ret;
		})
	},
	config_set: {
		args: {
			phy: "",
			config: "",
			prev_config: "",
		},
		call: ex_wrap(function(req) {
			let phy = req.args.phy;
			let file = req.args.config;
			let prev_file = req.args.prev_config;

			if (!phy)
				return libubus.STATUS_INVALID_ARGUMENT;

			if (prev_file && !hostapd.data.config[phy]) {
				let config = iface_load_config(prev_file);
				if (config)
					config.radio.data = [];
				hostapd.data.config[phy] = config;
			}

			let config = iface_load_config(file);

			hostapd.printf(`Set new config for phy ${phy}: ${file}`);
			iface_set_config(phy, config);

			return {
				pid: hostapd.getpid()
			};
		})
	},
	config_add: {
		args: {
			iface: "",
			config: "",
		},
		call: ex_wrap(function(req) {
			if (!req.args.iface || !req.args.config)
				return libubus.STATUS_INVALID_ARGUMENT;

			if (hostapd.add_iface(`bss_config=${req.args.iface}:${req.args.config}`) < 0)
				return libubus.STATUS_INVALID_ARGUMENT;

			return {
				pid: hostapd.getpid()
			};
		})
	},
	config_remove: {
		args: {
			iface: ""
		},
		call: ex_wrap(function(req) {
			if (!req.args.iface)
				return libubus.STATUS_INVALID_ARGUMENT;

			hostapd.remove_iface(req.args.iface);
			return 0;
		})
	},
};

hostapd.data.ubus = ubus;
hostapd.data.obj = ubus.publish("hostapd", main_obj);

function bss_event(type, name, data) {
	let ubus = hostapd.data.ubus;

	data ??= {};
	data.name = name;
	hostapd.data.obj.notify(`bss.${type}`, data, null, null, null, -1);
	ubus.call("service", "event", { type: `hostapd.${name}.${type}`, data: {} });
}

return {
	shutdown: function() {
		for (let phy in hostapd.data.config)
			iface_set_config(phy, null);
		hostapd.ubus.disconnect();
	},
	bss_add: function(name, obj) {
		bss_event("add", name);
	},
	bss_reload: function(name, obj, reconf) {
		bss_event("reload", name, { reconf: reconf != 0 });
	},
	bss_remove: function(name, obj) {
		bss_event("remove", name);
	}
};
