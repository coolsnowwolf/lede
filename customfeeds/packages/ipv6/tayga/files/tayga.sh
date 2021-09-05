# tayga.sh - NAT64 backend

find_tayga_wanif4() {
	local if=$(ip -4 r l e 0.0.0.0/0); if="${if#default* dev }"; if="${if%% *}"
	[ -n "$if" ] && grep -qs "^ *$if:" /proc/net/dev && echo "$if"
}

find_tayga_wanip4() {
	local ip=$(ip -4 a s dev "$1"); ip="${ip#*inet }"
	echo "${ip%%[^0-9.]*}"
}

find_tayga_wanif6() {
	local if=$(ip -6 r l e ::/0); if="${if#default* dev }"; if="${if%% *}"
	[ -n "$if" ] && grep -qs "^ *$if:" /proc/net/dev && echo "$if"
}

find_tayga_wanip6() {
	local ip=$(ip -6 a s dev "$1"); ip="${ip#*inet6 }"
	echo "${ip%%[^0-9A-Fa-f:]*}"
}

# Hook into scan_interfaces() to synthesize a .device option
# This is needed for /sbin/ifup to properly dispatch control
# to setup_interface_tayga() even if no .ifname is set in
# the configuration.
scan_tayga() {
	config_set "$1" device "tayga-$1"
}

coldplug_interface_tayga() {
	setup_interface_tayga "tayga-$1" "$1"
}

tayga_add_static_mappings() {
	local tmpconf="$1"

	(
		. /lib/functions.sh
		config_load firewall

		tayga_map_rule_add() {
			local cfg="$1"
			local tmpconf="$2"
			local ipv4_addr ipv6_addr
			config_get ipv4_addr "$cfg" ipv4_addr ""
			config_get ipv6_addr "$cfg" ipv6_addr ""
			[ -n "$ipv4_addr" ] && [ -n "$ipv6_addr" ] &&
				echo "map $ipv4_addr $ipv6_addr" >>$tmpconf
		}

		config_foreach tayga_map_rule_add nat64 "$tmpconf"
	)
}

setup_interface_tayga() {
	local iface="$1"
	local cfg="$2"
	local link="tayga-$cfg"

	local ipv4_addr ipv6_addr prefix dynamic_pool

	config_get ipv4_addr "$cfg" ipv4_addr
	config_get ipv6_addr "$cfg" ipv6_addr
	config_get prefix "$cfg" prefix
	config_get dynamic_pool "$cfg" dynamic_pool

	local args

	include /lib/network
	scan_interfaces

	local wanip4=$(uci_get network "$cfg" ipv4addr)
	local wanip6=$(uci_get network "$cfg" ipv6addr)

	local wanif4=$(find_tayga_wanif4)
	local wanif6=$(find_tayga_wanif6)

	[ -z "$wanip4" ] && {
		[ -n "$wanif4" ] && {
			wanip4=$(find_tayga_wanip4 "$wanif4")
			uci_set_state network "$cfg" wan4_device "$wanif4"
		}
	}

	[ -z "$wanip6" ] && {
		[ -n "$wanif6" ] && {
			wanip6=$(find_tayga_wanip6 "$wanif6")
			uci_set_state network "$cfg" wan6_device "$wanif6"
		}
	}

	[ -n "$wanip4" ] && [ -n "$wanip6" ] || {
		echo "Cannot determine local IPv4 and IPv6 addressed for tayga NAT64 $cfg - skipping"
		return 1
	}

	local tmpconf="/var/etc/tayga-$cfg.conf"
	args="-c $tmpconf"
	mkdir -p /var/etc
	mkdir -p /var/run/tayga/$cfg

	echo "tun-device $link" >$tmpconf
	echo "ipv4-addr $ipv4_addr" >>$tmpconf
	[ -n "$ipv6_addr" ] &&
		echo "ipv6-addr $ipv6_addr" >>$tmpconf
	[ -n "$prefix" ] &&
		echo "prefix $prefix" >>$tmpconf

	tayga_add_static_mappings "$tmpconf"

	[ -n "$dynamic_pool" ] &&
		echo "dynamic-pool $dynamic_pool" >>$tmpconf
	echo "data-dir /var/run/tayga/$cfg" >>$tmpconf

	# creating the tunnel below will trigger a net subsystem event
	# prevent it from touching or iface by disabling .auto here
	uci_set_state network "$cfg" ifname $link
	uci_set_state network "$cfg" auto 0

	# here we create TUN device and check configuration
	tayga $args --mktun || return 1

	ip link set "$link" up

	ip addr add "$wanip4" dev "$link"
	ip addr add "$wanip6" dev "$link"

	[ -n "$dynamic_pool" ] &&
		ip -4 route add "$dynamic_pool" dev "$link"
	[ -n "$prefix" ] &&
		ip -6 route add "$prefix" dev "$link"

	start-stop-daemon -S -x tayga -- $args -p /var/run/$link.pid

	env -i ACTION="ifup" DEVICE="$link" INTERFACE="$cfg" PROTO="tayga" \
		/sbin/hotplug-call iface
}

stop_interface_tayga() {
	local cfg="$1"
	local link="tayga-$cfg"

	env -i ACTION="ifdown" DEVICE="$link" INTERFACE="$cfg" PROTO="tayga" \
		/sbin/hotplug-call iface

	service_kill tayga "/var/run/$link.pid"

	ip link set "$link" down
	ip addr flush dev "$link"
	ip route flush dev "$link"
}
