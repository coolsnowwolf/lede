#!/bin/sh /etc/rc.common

START=90
STOP=89
USE_PROCD=1

service_triggers() {
	procd_add_reload_trigger "chilli"
}

config_cb() {
	chilli_conf="/var/run/chilli_${2}.conf"
	[ -e "$chilli_conf" ] && rm -f "$chilli_conf"
}

option_cb() {
	case "$1" in
		# ignored/internal settings
		disabled)
			;;
		# UCI settings
		network)
			. /lib/functions/network.sh
			local ifname
			network_get_device ifname "$2"
			echo "dhcpif=\"$ifname\"" >> "$chilli_conf"
			;;
		# boolean settings
		acctupdate|chillixml|coanoipcheck|debug|dhcpbroadcast|dhcpmacset|dhcpnotidle|\
		dhcpradius|dnsparanoia|domaindnslocal|eapolenable|fg|forgiving|framedservice|\
		ieee8021q|injectwispr|ipv6|ipv6only|layer3|locationcopycalled|\
		locationimmediateupdate|locationopt82|locationstopstart|macallowlocal|\
		macauth|macauthdeny|macreauth|mmapring|mschapv2|noarpentries|noc2c|nochallenge|\
		nodynip|noradallow|nosystemdns|nouamsuccess|nousergardendata|nowispr1|nowispr2|\
		only8021q|openidauth|papalwaysok|patricia|postauthproxyssl|proxymacaccept|\
		proxyonacct|radiusoriginalurl|radsec|redir|redirdnsreq|redirssl|redirurl|reload|\
		routeonetone|scalewin|seskeepalive|statusfilesave|strictdhcp|strictmacauth|\
		swapoctets|uamallowpost|uamanydns|uamanyip|uamauthedallowed|uamgardendata|\
		uamnatanyip|uamotherdata|uamuissl|usetap|vlanlocation|wpaguests)
			[ "$2" = "true" -o "$2" = "1" ] && echo "$1" >> "$chilli_conf"
			;;
		*)
			echo "$1=\"$2\"" >> "$chilli_conf"
			;;
	esac
}

start_chilli() {
	local cfg="$1"
	local base="/var/run/chilli_${cfg}"

	config_get_bool disabled "$1" 'disabled' 0
	[ $disabled = 1 ] && return

	procd_open_instance "$cfg"
	procd_set_param command /usr/sbin/chilli
	procd_set_param file "$chilli_conf"
	procd_append_param command \
		--fg \
		--conf "${base}.conf" \
		--pidfile "${base}.pid" \
		--cmdsocket "${base}.sock" \
		--unixipc "${base}.ipc"
	procd_set_param respawn
	procd_set_param stdout 1
	procd_set_param stderr 1
	procd_close_instance
}

start_service() {
	config_load chilli
	config_foreach start_chilli chilli
}

stop_service() {
	rm -f /var/run/chilli_*
}

reload_chilli() {
	local pid
	local cfg="$1"
	local base="/var/run/chilli_${cfg}"
	if [ -f "${base}.pid" ]; then
		pid="$(cat "${base}.pid")"
		[ -f "/var/run/chilli.${pid}.cfg.bin" ] && rm -f "/var/run/chilli.${pid}.cfg.bin"
		chilli_query -s "${base}.sock" reload
	fi
}

reload_service() {
	config_load chilli
	config_foreach reload_chilli chilli
}
