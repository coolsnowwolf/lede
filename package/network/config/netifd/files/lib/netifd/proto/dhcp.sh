#!/bin/sh

[ -x /sbin/udhcpc ] || exit 0

. /lib/functions.sh
. /lib/functions/network.sh
. ../netifd-proto.sh
. /lib/config/uci.sh
init_proto "$@"

proto_dhcp_init_config() {
	renew_handler=1
	restart_handler=1

	proto_config_add_string 'ipaddr:ipaddr'
	proto_config_add_string 'hostname:hostname'
	proto_config_add_string clientid
	proto_config_add_string sendclientid
	proto_config_add_string vendorid
	proto_config_add_boolean 'broadcast:bool'
	proto_config_add_boolean 'norelease:bool'
	proto_config_add_string 'reqopts:list(string)'
	proto_config_add_boolean 'defaultreqopts:bool'
	proto_config_add_string iface6rd
	proto_config_add_array 'sendopts:list(string)'
	proto_config_add_boolean delegate
	proto_config_add_string zone6rd
	proto_config_add_string zone
	proto_config_add_string mtu6rd
	proto_config_add_string customroutes
	proto_config_add_boolean classlessroute
	proto_config_add_int timeout
	proto_config_add_int retry
	proto_config_add_int tryagain
}

proto_dhcp_add_sendopts() {
	[ -n "$1" ] && append "$3" "-x $1"
}

proto_dhcp_get_default_clientid() {
	[ -z "$1" ] && return

	local iface="$1"
	local duid
	local iaid

	duid="$(uci_get network @globals[0] dhcp_default_duid)"
	[ -n "$duid" ] && {
		duid="$(hexdump_2hex "$duid")"
		[ -z "$duid" ] && logger -p warn -t dhcp "$iface: ignoring invalid dhcp_default_duid value"
	}
	[ -z "$duid" ] && return
	network_generate_iface_iaid iaid "$iface"
	printf "ff%s%s" "$iaid" "$duid"
}

proto_dhcp_setup() {
	local config="$1"
	local iface="$2"

	local ipaddr hostname clientid sendclientid vendorid broadcast norelease reqopts defaultreqopts iface6rd sendopts delegate zone6rd zone mtu6rd customroutes classlessroute timeout retry tryagain
	json_get_vars ipaddr hostname clientid sendclientid vendorid broadcast norelease reqopts defaultreqopts iface6rd delegate zone6rd zone mtu6rd customroutes classlessroute timeout retry tryagain

	local opt dhcpopts
	for opt in $reqopts; do
		append dhcpopts "-O $opt"
	done

	json_for_each_item proto_dhcp_add_sendopts sendopts dhcpopts

	[ -z "$hostname" ] && hostname="$(cat /proc/sys/kernel/hostname)"
	[ "$hostname" = "*" ] && hostname=

	[ "$defaultreqopts" = 0 ] && defaultreqopts="-o" || defaultreqopts=
	[ "$broadcast" = 1 ] && broadcast="-B" || broadcast=
	[ "$norelease" = 1 ] && norelease="" || norelease="-R"
	case "$sendclientid" in
		global)
			clientid="$(proto_dhcp_get_default_clientid "$iface")"
			;;
		hardware)
			clientid=''
			;;
		none)
			clientid='-C'
			;;
		auto|\
		*)
			[ -n "$clientid" ] && {
				clientid="$(hexdump_2hex "$clientid")"
				[ -z "$clientid" ] && {
					logger -p warn -t dhcp "$iface: ignoring invalid clientid value"
				}
			}
			[ -z "$clientid" ] && clientid="$(proto_dhcp_get_default_clientid "$iface")"
			;;
	esac
	[ -n "${clientid##-C}" ] && clientid="-x 0x3d:$clientid"
	[ -n "$vendorid" ] && append dhcpopts "-x 0x3c:$(echo -n "$vendorid" | hexdump -ve '1/1 "%02x"')"
	[ -n "$iface6rd" ] && proto_export "IFACE6RD=$iface6rd"
	[ "$iface6rd" != 0 -a -f /lib/netifd/proto/6rd.sh ] && append dhcpopts "-O 212"
	[ -n "$zone6rd" ] && proto_export "ZONE6RD=$zone6rd"
	[ -n "$zone" ] && proto_export "ZONE=$zone"
	[ -n "$mtu6rd" ] && proto_export "MTU6RD=$mtu6rd"
	[ -n "$customroutes" ] && proto_export "CUSTOMROUTES=$customroutes"
	[ "$delegate" = "0" ] && proto_export "IFACE6RD_DELEGATE=0"
	# Request classless route option (see RFC 3442) by default
	[ "$classlessroute" = "0" ] || append dhcpopts "-O 121"

	# Avoid sending duplicate Option 60 values
	local emptyvendorid
	case "$dhcpopts" in
		*"-x 0"[xX]*"3"[cC]":"* |\
		*"-x 60:"* |\
		*"-x vendor:"*)
			emptyvendorid=1
			;;
	esac

	proto_export "INTERFACE=$config"
	proto_run_command "$config" udhcpc \
		-p /var/run/udhcpc-$iface.pid \
		-s /lib/netifd/dhcp.script \
		-f -t "${retry:-0}" -i "$iface" \
		${timeout:+-T "$timeout"} \
		${tryagain:+-A "$tryagain"} \
		${ipaddr:+-r ${ipaddr/\/*/}} \
		${hostname:+-x "hostname:$hostname"} \
		${emptyvendorid:+-V ""} \
		$clientid $defaultreqopts $broadcast $norelease $dhcpopts
}

proto_dhcp_renew() {
	local interface="$1"
	# SIGUSR1 forces udhcpc to renew its lease
	local sigusr1="$(kill -l SIGUSR1)"
	[ -n "$sigusr1" ] && proto_kill_command "$interface" $sigusr1
}

proto_dhcp_restart() {
	local interface="$1"
	# SIGHUP asks a patched udhcpc to release the current lease and
	# immediately re-enter INIT_SELECTING so a fresh DHCPDISCOVER goes
	# out. Requires the matching busybox udhcpc patch.
	local sighup="$(kill -l SIGHUP)"
	[ -n "$sighup" ] && proto_kill_command "$interface" $sighup
}

proto_dhcp_teardown() {
	local interface="$1"
	proto_kill_command "$interface"
}

add_protocol dhcp
