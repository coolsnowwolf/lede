#!/bin/sh
# 6in4.sh - IPv6-in-IPv4 tunnel backend
# Copyright (c) 2010-2015 OpenWrt.org

[ -n "$INCLUDE_ONLY" ] || {
	. /lib/functions.sh
	. /lib/functions/network.sh
	. ../netifd-proto.sh
	init_proto "$@"
}

# Function taken from 6to4 package (6to4.sh), flipped returns
test_6in4_rfc1918()
{
	local oIFS="$IFS"; IFS="."; set -- $1; IFS="$oIFS"
	[ $1 -eq  10 ] && return 1
	[ $1 -eq 192 ] && [ $2 -eq 168 ] && return 1
	[ $1 -eq 172 ] && [ $2 -ge  16 ] && [ $2 -le  31 ] && return 1

	# RFC 6598
	[ $1 -eq 100 ] && [ $2 -ge  64 ] && [ $2 -le 127 ] && return 1

	return 0
}

proto_6in4_update() {
	sh -c '
		timeout=5

		(while [ $((timeout--)) -gt 0 ]; do
			sleep 1
			kill -0 $$ || exit 0
		done; kill -9 $$) 2>/dev/null &

		exec "$@"
	' "$1" "$@"
}

proto_6in4_add_prefix() {
	append "$3" "$1"
}

proto_6in4_setup() {
	local cfg="$1"
	local iface="$2"
	local link="6in4-$cfg"

	local mtu ttl tos ipaddr peeraddr ip6addr ip6prefix ip6prefixes tunlink tunnelid username password updatekey
	json_get_vars mtu ttl tos ipaddr peeraddr ip6addr tunlink tunnelid username password updatekey
	json_for_each_item proto_6in4_add_prefix ip6prefix ip6prefixes

	[ -z "$peeraddr" ] && {
		proto_notify_error "$cfg" "MISSING_ADDRESS"
		proto_block_restart "$cfg"
		return
	}

	( proto_add_host_dependency "$cfg" "$peeraddr" "$tunlink" )

	[ -z "$ipaddr" ] && {
		local wanif="$tunlink"
		if [ -z "$wanif" ] && ! network_find_wan wanif; then
			proto_notify_error "$cfg" "NO_WAN_LINK"
			return
		fi

		if ! network_get_ipaddr ipaddr "$wanif"; then
			proto_notify_error "$cfg" "NO_WAN_LINK"
			return
		fi
	}

	proto_init_update "$link" 1

	[ -n "$ip6addr" ] && {
		local local6="${ip6addr%%/*}"
		local mask6="${ip6addr##*/}"
		[ "$local6" = "$mask6" ] && mask6=
		proto_add_ipv6_address "$local6" "$mask6"
		proto_add_ipv6_route "::" 0 "" "" "" "$local6/$mask6"
	}

	for ip6prefix in $ip6prefixes; do
		proto_add_ipv6_prefix "$ip6prefix"
		proto_add_ipv6_route "::" 0 "" "" "" "$ip6prefix"
	done

	proto_add_tunnel
	json_add_string mode sit
	json_add_int mtu "${mtu:-1280}"
	json_add_int ttl "${ttl:-64}"
	[ -n "$tos" ] && json_add_string tos "$tos"
	json_add_string local "$ipaddr"
	json_add_string remote "$peeraddr"
	[ -n "$tunlink" ] && json_add_string link "$tunlink"
	proto_close_tunnel

	proto_send_update "$cfg"

	[ -n "$tunnelid" -a -n "$username" -a \( -n "$password" -o -n "$updatekey" \) ] && {
		[ -n "$updatekey" ] && password="$updatekey"

		local http="http"
		local urlget="uclient-fetch"
		local urlget_opts="-qO-"
		local ca_path="${SSL_CERT_DIR:-/etc/ssl/certs}"

		[ -f /lib/libustream-ssl.so ] && http=https
		[ "$http" = "https" -a -z "$(find $ca_path -name "*.0" 2>/dev/null)" ] && {
			urlget_opts="$urlget_opts --no-check-certificate"
		}

		local url="$http://ipv4.tunnelbroker.net/nic/update?hostname=$tunnelid"
		
		test_6in4_rfc1918 "$ipaddr" && {
			local url="${url}&myip=${ipaddr}"
		}

		local try=0
		local max=3

		(
			set -o pipefail
			while [ $((++try)) -le $max ]; do
				if proto_6in4_update $urlget $urlget_opts --user="$username" --password="$password" "$url" 2>&1 | \
					sed -e 's,^Killed$,timeout,' -e "s,^,update $try/$max: ," | \
					logger -t "$link";
				then
					logger -t "$link" "updated"
					return 0
				fi
				sleep 5
			done
			logger -t "$link" "update failed"
		)
	}
}

proto_6in4_teardown() {
	local cfg="$1"
}

proto_6in4_init_config() {
	no_device=1
	available=1

	proto_config_add_string "ipaddr"
	proto_config_add_string "ip6addr"
	proto_config_add_array "ip6prefix"
	proto_config_add_string "peeraddr"
	proto_config_add_string "tunlink"
	proto_config_add_string "tunnelid"
	proto_config_add_string "username"
	proto_config_add_string "password"
	proto_config_add_string "updatekey"
	proto_config_add_int "mtu"
	proto_config_add_int "ttl"
	proto_config_add_string "tos"
}

[ -n "$INCLUDE_ONLY" ] || {
	add_protocol 6in4
}
