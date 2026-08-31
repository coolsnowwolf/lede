#!/usr/bin/env ucode

import * as libubus from "ubus";

let script_path = ARGV[0];
let proto_name = ARGV[1];
let action = ARGV[2];
let iface_name = ARGV[3];
let config_json = ARGV[4];
let device = ARGV[5];

let config;
try {
	let blob = json(config_json);
	let inner = blob?._ucode_config;
	config = inner ? json(inner) : blob;
} catch (e) {
	warn(`Failed to parse config JSON: ${e}\n${e.stacktrace[0].context}\n`);
	exit(1);
}

let ubus = libubus.connect();
if (!ubus) {
	warn(`Failed to connect to ubus\n`);
	exit(1);
}

let notify_path = `network.interface.${iface_name}`;

function proto_notify(data)
{
	return ubus.call(notify_path, "notify_proto", data);
}

let proto = {
	iface: iface_name,
	proto: proto_name,
	config,
	device,

	notify: proto_notify,

	update_link: function(up, data) {
		let msg = { action: 0, "link-up": up, ...(data ?? {}) };
		return proto_notify(msg);
	},

	run_command: function(argv, env) {
		let msg = { action: 1, command: argv };
		if (env)
			msg.env = env;
		return proto_notify(msg);
	},

	kill_command: function(signal) {
		return proto_notify({ action: 2, signal: signal ?? 15 });
	},

	error: function(errors) {
		return proto_notify({ action: 3, error: errors });
	},

	block_restart: function() {
		return proto_notify({ action: 4 });
	},

	set_available: function(available) {
		return proto_notify({ action: 5, available });
	},

	add_host_dependency: function(host, ifname) {
		let msg = { action: 6 };
		if (host)
			msg.host = host;
		if (ifname)
			msg.ifname = ifname;
		return proto_notify(msg);
	},

	setup_failed: function() {
		return proto_notify({ action: 7 });
	},
};

let handlers = {};

let netifd_stub = {
	add_proto: function(handler) {
		if (handler?.name)
			handlers[handler.name] = handler;
	},
};

try {
	include(script_path, { netifd: netifd_stub });
} catch (e) {
	warn(`Failed to load proto handler script '${script_path}': ${e}\n${e.stacktrace[0].context}\n`);
	exit(1);
}

let handler = handlers[proto_name];
if (!handler) {
	warn(`No handler found for protocol '${proto_name}'\n`);
	exit(1);
}

if (!handler[action]) {
	warn(`Handler '${proto_name}' has no '${action}' function\n`);
	exit(1);
}

handler[action](proto);
