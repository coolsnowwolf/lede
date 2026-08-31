#!/usr/bin/env ucode
'use strict';

import * as fs from 'fs';

const WG = '/usr/bin/wg';

function wg_exists() {
	return fs.access(WG, fs.F_OK);
}

function ensure_key_is_generated(cursor, section_name) {
	let private_key = cursor.get('network', section_name, 'private_key');

	if (!private_key || private_key == 'generate') {
		let proc = fs.popen(`${WG} genkey`);
		if (!proc)
			return null;

		let generated_key = rtrim(proc.read('all'));
		proc.close();

		if (generated_key) {
			cursor.set('network', section_name, 'private_key', generated_key);
			cursor.commit('network');
			return generated_key;
		}
	}

	return private_key;
}

function to_array(val) {
	return type(val) == 'array' ? val : split(trim(val), /\s+/);
}

function parse_address(addr) {
	if (index(addr, ':') >= 0) {
		if (index(addr, '/') >= 0) {
			let parts = split(addr, '/');
			return { family: 6, address: parts[0], mask: int(parts[1]) };
		}
		return { family: 6, address: addr, mask: 128 };
	}

	if (index(addr, '/') >= 0) {
		let parts = split(addr, '/');
		return { family: 4, address: parts[0], mask: int(parts[1]) };
	}

	return { family: 4, address: addr, mask: 32 };
}

function load_peers(cursor, iface) {
	let peers = [];
	let peer_type = sprintf('wireguard_%s', iface);

	cursor.foreach('network', peer_type, (peer_section) => {
		let disabled = peer_section.disabled;
		if (disabled == '1')
			return;

		let route_allowed_ips = peer_section.route_allowed_ips;
		let peer_key = peer_section.public_key;
		let peer_eph = peer_section.endpoint_host;
		let peer_port = peer_section.endpoint_port ?? '51820';
		let peer_a_ips = peer_section.allowed_ips;
		let peer_p_ka = peer_section.persistent_keepalive;
		let peer_psk = peer_section.preshared_key;

		if (!peer_key)
			return;

		let peer_data = {
			public_key: peer_key,
			preshared_key: peer_psk,
			endpoint_host: peer_eph,
			endpoint_port: peer_port,
			allowed_ips: peer_a_ips,
			persistent_keepalive: peer_p_ka,
			route_allowed_ips: route_allowed_ips == '1'
		};

		push(peers, peer_data);
	});

	return peers;
}

function proto_setup(proto) {
	if (!wg_exists()) {
		warn('WireGuard tools not found at ', WG, '\n');
		proto.setup_failed();
		return;
	}

	let iface = proto.iface;
	let config = proto.config;

	system(sprintf('ip link add dev %s type wireguard 2>/dev/null || true', iface));

	if (config.mtu)
		system(sprintf('ip link set mtu %d dev %s', int(config.mtu), iface));

	let wg_config = '[Interface]\n';
	wg_config += sprintf('PrivateKey=%s\n', config.private_key);

	if (config.listen_port)
		wg_config += sprintf('ListenPort=%d\n', int(config.listen_port));

	if (config.fwmark)
		wg_config += sprintf('FwMark=%s\n', config.fwmark);

	let metric = int(config.metric);

	let ipv4_routes = [];
	let ipv6_routes = [];

	for (let peer in config.peers) {
		wg_config += '\n[Peer]\n';
		wg_config += sprintf('PublicKey=%s\n', peer.public_key);

		if (peer.preshared_key)
			wg_config += sprintf('PresharedKey=%s\n', peer.preshared_key);

		if (peer.endpoint_host) {
			let eph = peer.endpoint_host;
			if (index(eph, ':') >= 0 && substr(eph, 0, 1) != '[')
				eph = sprintf('[%s]', eph);
			wg_config += sprintf('Endpoint=%s:%s\n', eph, peer.endpoint_port);
		}

		if (peer.allowed_ips) {
			let allowed_list = to_array(peer.allowed_ips);
			wg_config += sprintf('AllowedIPs=%s\n', join(', ', allowed_list));

			if (peer.route_allowed_ips) {
				for (let allowed_ip in allowed_list) {
					let addr_info = parse_address(allowed_ip);
					let route = { target: addr_info.address, netmask: '' + addr_info.mask };
					if (metric)
						route.metric = metric;
					if (addr_info.family == 6)
						push(ipv6_routes, route);
					else
						push(ipv4_routes, route);
				}
			}
		}

		if (peer.persistent_keepalive)
			wg_config += sprintf('PersistentKeepalive=%s\n', peer.persistent_keepalive);
	}

	let wg_proc = fs.popen(sprintf('%s syncconf %s /dev/stdin', WG, iface), 'w');
	if (!wg_proc) {
		warn('Failed to run wg syncconf for ', iface, '\n');
		proto.setup_failed();
		return;
	}

	wg_proc.write(wg_config);
	let wg_result = wg_proc.close();

	if (wg_result != 0) {
		warn('wg syncconf failed for ', iface, '\n');
		proto.setup_failed();
		return;
	}

	system(sprintf('ip link set up dev %s', iface));

	let ipv4_addrs = [];
	let ipv6_addrs = [];

	if (config.addresses) {
		let addr_list = to_array(config.addresses);
		for (let address in addr_list) {
			let addr_info = parse_address(address);
			let addr = { ipaddr: addr_info.address, mask: '' + addr_info.mask };
			if (addr_info.family == 6)
				push(ipv6_addrs, addr);
			else
				push(ipv4_addrs, addr);
		}
	}

	let link_data = {
		ifname: iface
	};

	if (length(ipv4_addrs) > 0)
		link_data.ipaddr = ipv4_addrs;

	if (length(ipv6_addrs) > 0)
		link_data.ip6addr = ipv6_addrs;

	if (length(ipv4_routes) > 0)
		link_data.routes = ipv4_routes;

	if (length(ipv6_routes) > 0)
		link_data.routes6 = ipv6_routes;

	if (config.ip6prefix) {
		let prefix_list = to_array(config.ip6prefix);
		if (length(prefix_list) > 0)
			link_data.ip6prefix = prefix_list;
	}

	if (config.nohostroute != '1') {
		let endpoints_proc = fs.popen(sprintf('%s show %s endpoints', WG, iface));
		if (endpoints_proc) {
			let endpoints_data = endpoints_proc.read('all');
			endpoints_proc.close();

			let endpoint_lines = split(endpoints_data, '\n');
			for (let line in endpoint_lines) {
				if (!line)
					continue;

				let parts = split(rtrim(line), '\t');
				if (length(parts) < 2)
					continue;

				let endpoint = parts[1];
				let addr_match = match(endpoint, regexp('\\[?([0-9.:a-f]+)\\]?:([0-9]+)'));
				if (addr_match && length(addr_match) > 1)
					proto.add_host_dependency(addr_match[1], config.tunlink);
			}
		}
	}

	proto.update_link(true, link_data);
}

function proto_teardown(proto) {
	let iface = proto.iface;
	system(sprintf('ip link del dev %s 2>/dev/null', iface));
	proto.update_link(false);
}

function proto_renew(proto) {
	proto_setup(proto);
}

netifd.add_proto({
	available: true,
	no_proto_task: true,
	'renew-handler': true,
	name: 'wireguard',

	config: function(ctx) {
		return {
			...ctx.data,
			private_key: ensure_key_is_generated(ctx.uci, ctx.section),
			peers: load_peers(ctx.uci, ctx.section)
		};
	},

	setup: proto_setup,
	teardown: proto_teardown,
	renew: proto_renew
});
