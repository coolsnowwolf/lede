#!/usr/bin/env ucode
'use strict';
import { vlist_new, is_equal, wdev_set_mesh_params, wdev_remove, wdev_set_up, phy_open } from "/usr/share/hostap/common.uc";
import { readfile, writefile, basename, readlink, glob } from "fs";
let libubus = require("ubus");

let keep_devices = {};
let phy_name = shift(ARGV);
let command = shift(ARGV);
let phy, phydev;

function iface_stop(wdev)
{
	if (keep_devices[wdev.ifname])
		return;

	wdev_remove(wdev.ifname);
}

function iface_start(wdev)
{
	let ifname = wdev.ifname;

	if (readfile(`/sys/class/net/${ifname}/ifindex`)) {
		wdev_set_up(ifname, false);
		wdev_remove(ifname);
	}
	let wdev_config = {};
	for (let key in wdev)
		wdev_config[key] = wdev[key];
	if (!wdev_config.macaddr && wdev.mode != "monitor")
		wdev_config.macaddr = phydev.macaddr_next();
	phydev.wdev_add(ifname, wdev_config);
	wdev_set_up(ifname, true);
	let htmode = wdev.htmode || "NOHT";
	if (wdev.freq)
		system(`iw dev ${ifname} set freq ${wdev.freq} ${htmode}`);
	if (wdev.mode == "adhoc") {
		let cmd = ["iw", "dev", ifname, "ibss", "join", wdev.ssid, wdev.freq, htmode, "fixed-freq" ];
		if (wdev.bssid)
			push(cmd, wdev.bssid);
		for (let key in [ "beacon-interval", "basic-rates", "mcast-rate", "keys" ])
			if (wdev[key])
				push(cmd, key, wdev[key]);
		system(cmd);
	} else if (wdev.mode == "mesh") {
		let cmd = [ "iw", "dev", ifname, "mesh", "join", wdev.ssid ];
		if (wdev.freq) {
			push(cmd, "freq", wdev.freq);
			if (htmode && htmode != "NOHT")
				push(cmd, htmode);
		}
		for (let key in [ "basic-rates", "mcast-rate", "beacon-interval" ])
			if (wdev[key])
				push(cmd, key, wdev[key]);
		system(cmd);

		wdev_set_mesh_params(ifname, wdev);
	}
}

function iface_cb(new_if, old_if)
{
	if (old_if && new_if && is_equal(old_if, new_if))
		return;

	if (old_if)
		iface_stop(old_if);
	if (new_if)
		iface_start(new_if);
}

function drop_inactive(config)
{
	for (let key in config) {
		if (!readfile(`/sys/class/net/${key}/ifindex`))
			delete config[key];
	}
}

function add_ifname(config)
{
	for (let key in config)
		config[key].ifname = key;
}

function delete_ifname(config)
{
	for (let key in config)
		delete config[key].ifname;
}

function usage()
{
	warn(`Usage: ${basename(sourcepath())} <phy> <command> [<arguments>]

Commands:
	set_config <config> [<device]...] - set phy configuration
	get_macaddr <id>		  - get phy MAC address for vif index <id>
`);
	exit(1);
}

const commands = {
	set_config: function(args) {
		let statefile = `/var/run/wdev-${phy_name}.json`;

		let new_config = shift(args);
		for (let dev in ARGV)
			keep_devices[dev] = true;

		if (!new_config)
			usage();

		new_config = json(new_config);
		if (!new_config) {
			warn("Invalid configuration\n");
			exit(1);
		}

		let old_config = readfile(statefile);
		if (old_config)
			old_config = json(old_config);

		let config = vlist_new(iface_cb);
		if (type(old_config) == "object")
			config.data = old_config;

		add_ifname(config.data);
		drop_inactive(config.data);

		let ubus = libubus.connect();
		let data = ubus.call("hostapd", "config_get_macaddr_list", { phy: phydev.phy });
		let macaddr_list = [];
		if (type(data) == "object" && data.macaddr)
			macaddr_list = data.macaddr;
		ubus.disconnect();
		phydev.macaddr_init(macaddr_list);

		add_ifname(new_config);
		config.update(new_config);

		drop_inactive(config.data);
		delete_ifname(config.data);
		writefile(statefile, sprintf("%J", config.data));
	},
	get_macaddr: function(args) {
		let data = {};

		for (let arg in args) {
			arg = split(arg, "=", 2);
			data[arg[0]] = arg[1];
		}

		let macaddr = phydev.macaddr_generate(data);
		if (!macaddr) {
			warn(`Could not get MAC address for phy ${phy_name}\n`);
			exit(1);
		}

		print(macaddr + "\n");
	},
};

if (!phy_name || !command | !commands[command])
	usage();

let phy_split = split(phy_name, ":");
phydev = phy_open(phy_split[0], phy_split[1]);
phy = phydev.phy;
if (!phydev) {
	warn(`PHY ${phy_name} does not exist\n`);
	exit(1);
}

commands[command](ARGV);
