#!/bin/sh

[ -x /usr/bin/sstpc ] || exit 0

[ -n "$INCLUDE_ONLY" ] || {
	. /lib/functions.sh
	. ../netifd-proto.sh
	init_proto "$@"
}

proto_sstp_init_config() {
	proto_config_add_string "server"
	proto_config_add_string "username"
	proto_config_add_string "password"
	proto_config_add_string "pppd_options"
	proto_config_add_string "sstp_options"
	proto_config_add_int "log_level"
	proto_config_add_int "mtu"
	proto_config_add_boolean "ipv6"
	proto_config_add_boolean "defaultroute"
	proto_config_add_boolean "peerdns"
	available=1
	no_device=1
}

proto_sstp_setup() {
	local config="$1"; shift
	local iface="$2"
	local ifname="sstp-$config"

	local ip serv_addr server ipv6 defaultroute peerdns
	json_get_var server server && {
		for ip in $(resolveip -t 5 "$server"); do
			( proto_add_host_dependency "$config" "$ip" )
			serv_addr=1
		done
	}
	[ -n "$serv_addr" ] || {
		echo "Could not resolve server address"
		sleep 5
		proto_setup_failed "$config"
		exit 1
	}

	json_get_vars username password pppd_options sstp_options log_level ipv6 defaultroute peerdns
	if [ "$ipv6" = 1 ]; then
		ipv6=1
	else
		ipv6=""
	fi
	if [ "$defaultroute" = 0 ]; then
		defaultroute=""
	else
		defaultroute=1
	fi

	if [ "$peerdns" = 0 ]; then
		peerdns=""
	else
		peerdns=1
	fi

	[ -n "$mtu" ] || json_get_var mtu mtu
	[ -n "$log_level" ] || log_level=0

	local load
	for module in slhc ppp_generic ppp_async ppp_mppe ip_gre gre pptp; do
		grep -q "^$module " /proc/modules && continue
		/sbin/insmod $module 2>&- >&-
		load=1
	done
	[ "$load" = "1" ] && sleep 1

	proto_init_update "$ifname" 1
	proto_send_update "$config"

	proto_run_command "$config" sstpc \
		--cert-warn \
		--password $password \
		--user $username \
		--log-level $log_level \
		--save-server-route \
		--ipparam $config \
		$sstp_options \
		$server \
		ifname $ifname \
		require-mschap-v2 \
		${ipv6:++ipv6} \
		refuse-pap \
		noauth \
		${defaultroute:+replacedefaultroute defaultroute} \
		${peerdns:+usepeerdns} \
		ip-up-script /lib/netifd/ppp-up \
		ipv6-up-script /lib/netifd/ppp-up \
		ip-down-script /lib/netifd/ppp-down \
		ipv6-down-script /lib/netifd/ppp-down \
		${mtu:+mtu $mtu mru $mtu} \
		$pppd_options
		
	# WORKAROUND: Workaround to properly register the sstp interface (As seeen in: https://forum.archive.openwrt.org/viewtopic.php?id=58007)
	# WORKAROUND: Start
	sleep 10
	proto_init_update "$ifname" 1
	proto_send_update "$config"
	# WORKAROUND: End
	
	# if use pppoe and sstp at same time , firewall need reload .
	# but don't konw why
	/etc/init.d/firewall reload 2>&- >&-
}

proto_sstp_teardown() {
	local interface="$1"

	case "$ERROR" in
		11|19)
			proto_notify_error "$interface" AUTH_FAILED
			proto_block_restart "$interface"
		;;
		2)
			proto_notify_error "$interface" INVALID_OPTIONS
			proto_block_restart "$interface"
		;;
	esac
	proto_kill_command "$interface"
}

[ -n "$INCLUDE_ONLY" ] || {
	add_protocol sstp
}
