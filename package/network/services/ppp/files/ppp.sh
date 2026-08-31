#!/bin/sh

[ -x /usr/sbin/pppd ] || exit 0

[ -n "$INCLUDE_ONLY" ] || {
	. /lib/functions.sh
	. /lib/functions/network.sh
	. ../netifd-proto.sh
	init_proto "$@"
}

ppp_select_ipaddr()
{
	local subnets=$1
	local res
	local res_mask

	for subnet in $subnets; do
		local addr="${subnet%%/*}"
		local mask="${subnet#*/}"

		if [ -n "$res_mask" -a "$mask" != 32 ]; then
			[ "$mask" -gt "$res_mask" ] || [ "$res_mask" = 32 ] && {
				res="$addr"
				res_mask="$mask"
			}
		elif [ -z "$res_mask" ]; then
			res="$addr"
			res_mask="$mask"
		fi
	done

	echo "$res"
}

ppp_exitcode_tostring()
{
	local errorcode=$1
	[ -n "$errorcode" ] || errorcode=5

	case "$errorcode" in
		0) echo "OK" ;;
		1) echo "FATAL_ERROR" ;;
		2) echo "OPTION_ERROR" ;;
		3) echo "NOT_ROOT" ;;
		4) echo "NO_KERNEL_SUPPORT" ;;
		5) echo "USER_REQUEST" ;;
		6) echo "LOCK_FAILED" ;;
		7) echo "OPEN_FAILED" ;;
		8) echo "CONNECT_FAILED" ;;
		9) echo "PTYCMD_FAILED" ;;
		10) echo "NEGOTIATION_FAILED" ;;
		11) echo "PEER_AUTH_FAILED" ;;
		12) echo "IDLE_TIMEOUT" ;;
		13) echo "CONNECT_TIME" ;;
		14) echo "CALLBACK" ;;
		15) echo "PEER_DEAD" ;;
		16) echo "HANGUP" ;;
		17) echo "LOOPBACK" ;;
		18) echo "INIT_FAILED" ;;
		19) echo "AUTH_TOPEER_FAILED" ;;
		20) echo "TRAFFIC_LIMIT" ;;
		21) echo "CNID_AUTH_FAILED";;
		*) echo "UNKNOWN_ERROR" ;;
	esac
}

ppp_generic_init_config() {
	proto_config_add_string username
	proto_config_add_string password
	proto_config_add_string keepalive
	proto_config_add_boolean keepalive_adaptive
	proto_config_add_int demand
	proto_config_add_string pppd_options
	proto_config_add_string 'connect:file'
	proto_config_add_string 'disconnect:file'
	[ -e /proc/sys/net/ipv6 ] && proto_config_add_string ipv6
	proto_config_add_boolean authfail
	proto_config_add_int mtu
	proto_config_add_string pppname
	proto_config_add_string unnumbered
	proto_config_add_string reqprefix
	proto_config_add_boolean persist
	proto_config_add_int maxfail
	proto_config_add_int holdoff
	proto_config_add_boolean sourcefilter
	proto_config_add_boolean delegate
	proto_config_add_boolean norelease
}

ppp_generic_setup() {
	local config="$1"; shift
	local localip

	json_get_vars ip6table demand keepalive keepalive_adaptive username password pppd_options pppname unnumbered reqprefix persist maxfail holdoff peerdns sourcefilter delegate norelease

	[ ! -e /proc/sys/net/ipv6 ] && ipv6=0 || json_get_var ipv6 ipv6

	if [ "$ipv6" = 0 ]; then
		ipv6=""
	elif [ -z "$ipv6" -o "$ipv6" = auto ]; then
		ipv6=1
		autoipv6=1
	fi

	if [ "$autoipv6" != 1 ]; then
		reqprefix=""
		norelease=""
	fi

	if [ "${demand:-0}" -gt 0 ]; then
		demand="precompiled-active-filter /etc/ppp/filter demand idle $demand"
	else
		demand=""
	fi
	if [ -n "$persist" ]; then
		[ "${persist}" -lt 1 ] && persist="nopersist" || persist="persist"
	fi
	if [ -z "$maxfail" ]; then
		[ "$persist" = "persist" ] && maxfail=0 || maxfail=1
	fi
	[ -n "$mtu" ] || json_get_var mtu mtu
	[ -n "$pppname" ] || pppname="${proto:-ppp}-$config"
	[ -n "$unnumbered" ] && {
		local subnets
		( proto_add_host_dependency "$config" "" "$unnumbered" )
		network_get_subnets subnets "$unnumbered"
		localip=$(ppp_select_ipaddr "$subnets")
		[ -n "$localip" ] || {
			proto_block_restart "$config"
			return
		}
	}

	[ -n "$keepalive" ] || keepalive="5 1"

	local lcp_failure="${keepalive%%[, ]*}"
	local lcp_interval="${keepalive##*[, ]}"
	local lcp_adaptive="lcp-echo-adaptive"
	[ "${lcp_failure:-0}" -lt 1 ] && lcp_failure=""
	[ "$lcp_interval" != "$keepalive" ] || lcp_interval=5
	[ "${keepalive_adaptive:-1}" -lt 1 ] && lcp_adaptive=""
	[ -n "$connect" ] || json_get_var connect connect
	[ -n "$disconnect" ] || json_get_var disconnect disconnect
	[ "$sourcefilter" = "0" ] || sourcefilter=""
	[ "$delegate" != "0" ] && delegate=""
	[ "$norelease" = "1" ] || norelease=""

	proto_run_command "$config" /usr/sbin/pppd \
		nodetach ipparam "$config" \
		ifname "$pppname" \
		${localip:+$localip:} \
		${lcp_failure:+lcp-echo-interval $lcp_interval lcp-echo-failure $lcp_failure $lcp_adaptive} \
		${ipv6:++ipv6} \
		${autoipv6:+set AUTOIPV6=1} \
		${reqprefix:+set REQPREFIX=$reqprefix} \
		${norelease:+set NORELEASE=1} \
		${ip6table:+set IP6TABLE=$ip6table} \
		${peerdns:+set PEERDNS=$peerdns} \
		${sourcefilter:+set NOSOURCEFILTER=1} \
		${delegate:+set DELEGATE=0} \
		nodefaultroute \
		usepeerdns \
		$demand $persist maxfail $maxfail \
		${holdoff:+holdoff "$holdoff"} \
		${username:+user "$username" password "$password"} \
		${connect:+connect "$connect"} \
		${disconnect:+disconnect "$disconnect"} \
		ip-up-script /lib/netifd/ppp-up \
		${ipv6:+ipv6-up-script /lib/netifd/ppp6-up} \
		ip-down-script /lib/netifd/ppp-down \
		${ipv6:+ipv6-down-script /lib/netifd/ppp-down} \
		${mtu:+mtu $mtu mru $mtu} \
		"$@" $pppd_options
}

ppp_generic_teardown() {
	local interface="$1"
	local errorstring=$(ppp_exitcode_tostring $ERROR)

	case "$ERROR" in
		0)
		;;
		2)
			proto_notify_error "$interface" "$errorstring"
			proto_block_restart "$interface"
		;;
		11|19)
			json_get_var authfail authfail
			proto_notify_error "$interface" "$errorstring"
			if [ "${authfail:-0}" -gt 0 ]; then
				proto_block_restart "$interface"
			fi
		;;
		*)
			proto_notify_error "$interface" "$errorstring"
		;;
	esac

	proto_kill_command "$interface"
}

# PPP on serial device

proto_ppp_init_config() {
	proto_config_add_string "device"
	ppp_generic_init_config
	no_device=1
	available=1
	lasterror=1
}

proto_ppp_setup() {
	local config="$1"

	json_get_var device device
	ppp_generic_setup "$config" "$device"
}

proto_ppp_teardown() {
	ppp_generic_teardown "$@"
}

proto_pppoe_init_config() {
	ppp_generic_init_config
	proto_config_add_string "ac"
	proto_config_add_string "service"
	proto_config_add_string "ac_mac"
	proto_config_add_string "host_uniq"
	proto_config_add_int "padi_attempts"
	proto_config_add_int "padi_timeout"

	lasterror=1
}

proto_pppoe_setup() {
	local config="$1"
	local iface="$2"

	/sbin/modprobe -qa slhc ppp_generic pppox pppoe

	json_get_var mtu mtu
	mtu="${mtu:-1492}"

	json_get_var ac ac
	json_get_var service service
	json_get_var ac_mac ac_mac
	json_get_var host_uniq host_uniq
	json_get_var padi_attempts padi_attempts
	json_get_var padi_timeout padi_timeout

	ppp_generic_setup "$config" \
		plugin pppoe.so \
		${ac:+rp_pppoe_ac "$ac"} \
		${service:+rp_pppoe_service "$service"} \
		${ac_mac:+pppoe-mac "$ac_mac"} \
		${host_uniq:+host-uniq "$host_uniq"} \
		${padi_attempts:+pppoe-padi-attempts $padi_attempts} \
		${padi_timeout:+pppoe-padi-timeout $padi_timeout} \
		"nic-$iface"
}

proto_pppoe_teardown() {
	ppp_generic_teardown "$@"
}

proto_pppoa_init_config() {
	ppp_generic_init_config
	proto_config_add_int "atmdev"
	proto_config_add_int "vci"
	proto_config_add_int "vpi"
	proto_config_add_string "encaps"
	no_device=1
	available=1
	lasterror=1
}

proto_pppoa_setup() {
	local config="$1"
	local iface="$2"

	/sbin/modprobe -qa slhc ppp_generic pppox pppoatm

	json_get_vars atmdev vci vpi encaps

	case "$encaps" in
		1|vc) encaps="vc-encaps" ;;
		*) encaps="llc-encaps" ;;
	esac

	ppp_generic_setup "$config" \
		plugin pppoatm.so \
		${atmdev:+$atmdev.}${vpi:-8}.${vci:-35} \
		${encaps}
}

proto_pppoa_teardown() {
	ppp_generic_teardown "$@"
}

proto_pptp_init_config() {
	ppp_generic_init_config
	proto_config_add_string "server"
	proto_config_add_string "interface"
	available=1
	no_device=1
	lasterror=1
}

proto_pptp_setup() {
	local config="$1"
	local iface="$2"

	local ip serv_addr server interface
	json_get_vars interface server
	[ -n "$server" ] && {
		for ip in $(resolveip -t 5 "$server"); do
			( proto_add_host_dependency "$config" "$ip" $interface )
			serv_addr=1
		done
	}
	[ -n "$serv_addr" ] || {
		echo "Could not resolve server address"
		sleep 5
		proto_setup_failed "$config"
		exit 1
	}

	/sbin/modprobe -qa slhc ppp_generic ppp_async ppp_mppe ip_gre gre pptp
	sleep 1

	ppp_generic_setup "$config" \
		plugin pptp.so \
		pptp_server $server \
		file /etc/ppp/options.pptp
}

proto_pptp_teardown() {
	ppp_generic_teardown "$@"
}

[ -n "$INCLUDE_ONLY" ] || {
	add_protocol ppp
	[ -f /usr/lib/pppd/*/pppoe.so ] && add_protocol pppoe
	[ -f /usr/lib/pppd/*/pppoatm.so ] && add_protocol pppoa
	[ -f /usr/lib/pppd/*/pptp.so ] && add_protocol pptp
}
