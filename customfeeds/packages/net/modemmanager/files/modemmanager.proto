#!/bin/sh
# Copyright (C) 2016-2019 Aleksander Morgado <aleksander@aleksander.es>

[ -x /usr/bin/mmcli ] || exit 0
[ -x /usr/sbin/pppd ] || exit 0

[ -n "$INCLUDE_ONLY" ] || {
	. /lib/functions.sh
	. ../netifd-proto.sh
	. ./ppp.sh
	init_proto "$@"
}

cdr2mask ()
{
	# Number of args to shift, 255..255, first non-255 byte, zeroes
	set -- $(( 5 - ($1 / 8) )) 255 255 255 255 $(( (255 << (8 - ($1 % 8))) & 255 )) 0 0 0
	if [ "$1" -gt 1 ]
	then
		shift "$1"
	else
		shift
	fi
	echo "${1-0}"."${2-0}"."${3-0}"."${4-0}"
}

# This method expects as first argument a list of key-value pairs, as returned by mmcli --output-keyvalue
# The second argument must be exactly the name of the field to read
#
# Sample output:
#     $ mmcli -m 0 -K
#     modem.dbus-path                                 : /org/freedesktop/ModemManager1/Modem/0
#     modem.generic.device-identifier                 : ed6eff2e3e0f90463da1c2a755b2acacd1335752
#     modem.generic.manufacturer                      : Dell Inc.
#     modem.generic.model                             : DW5821e Snapdragon X20 LTE
#     modem.generic.revision                          : T77W968.F1.0.0.4.0.GC.009\n026
#     modem.generic.carrier-configuration             : GCF
#     modem.generic.carrier-configuration-revision    : 08E00009
#     modem.generic.hardware-revision                 : DW5821e Snapdragon X20 LTE
#     ....
modemmanager_get_field() {
	local list=$1
	local field=$2
	local value=""

	[ -z "${list}" ] || [ -z "${field}" ] && return

	# there is always at least a whitespace after each key, and we use that as part of the
	# key matching we do (e.g. to avoid getting 'modem.generic.state-failed-reason' as a result
	# when grepping for 'modem.generic.state'.
	line=$(echo "${list}" | grep "${field} ")
	value=$(echo ${line#*:})

	# not found?
	[ -n "${value}" ] || return 2

	# only print value if set
	[ "${value}" != "--" ] && echo "${value}"
	return 0
}

# build a comma-separated list of values from the list
modemmanager_get_multivalue_field() {
	local list=$1
	local field=$2
	local value=""
	local length idx item

	[ -z "${list}" ] || [ -z "${field}" ] && return

	length=$(modemmanager_get_field "${list}" "${field}.length")
	[ -n "${length}" ] || return 0
	[ "$length" -ge 1 ] || return 0

	idx=1
	while [ $idx -le "$length" ]; do
		item=$(modemmanager_get_field "${list}" "${field}.value\[$idx\]")
		[ -n "${item}" ] && [ "${item}" != "--" ] && {
			[ -n "${value}" ] && value="${value}, "
			value="${value}${item}"
		}
		idx=$((idx + 1))
	done

	# nothing built?
	[ -n "${value}" ] || return 2

	# only print value if set
	echo "${value}"
	return 0
}

modemmanager_cleanup_connection() {
	local modemstatus="$1"

	local bearercount idx bearerpath

	bearercount=$(modemmanager_get_field "${modemstatus}" "modem.generic.bearers.length")

	# do nothing if no bearers reported
	[ -n "${bearercount}" ] && [ "$bearercount" -ge 1 ] && {
		# explicitly disconnect just in case
		mmcli --modem="${device}" --simple-disconnect >/dev/null 2>&1
		# and remove all bearer objects, if any found
		idx=1
		while [ $idx -le "$bearercount" ]; do
			bearerpath=$(modemmanager_get_field "${modemstatus}" "modem.generic.bearers.value\[$idx\]")
			mmcli --modem "${device}" --delete-bearer="${bearerpath}" >/dev/null 2>&1
			idx=$((idx + 1))
		done
	}
}

modemmanager_connected_method_ppp_ipv4() {
	local interface="$1"
	local ttyname="$2"
	local username="$3"
	local password="$4"
	local allowedauth="$5"

	# all auth types are allowed unless a user given list is given
	local authopts
	local pap=1
	local chap=1
	local mschap=1
	local mschapv2=1
	local eap=1

	[ -n "$allowedauth" ] && {
		pap=0 chap=0 mschap=0 mschapv2=0 eap=0
		for auth in $allowedauth; do
			case $auth in
				"pap") pap=1 ;;
				"chap") chap=1 ;;
				"mschap") mschap=1 ;;
				"mschapv2") mschapv2=1 ;;
				"eap") eap=1 ;;
				*) ;;
			esac
		done
	}

	[ $pap -eq 1 ] || append authopts "refuse-pap"
	[ $chap -eq 1 ] || append authopts "refuse-chap"
	[ $mschap -eq 1 ] || append authopts "refuse-mschap"
	[ $mschapv2 -eq 1 ] || append authopts "refuse-mschap-v2"
	[ $eap -eq 1 ] || append authopts "refuse-eap"

	proto_run_command "${interface}" /usr/sbin/pppd \
		"${ttyname}" \
		115200 \
		nodetach \
		noaccomp \
		nobsdcomp \
		nopcomp \
		novj \
		noauth \
		$authopts \
		${username:+ user $username} \
		${password:+ password $password} \
		lcp-echo-failure 5 \
		lcp-echo-interval 15 \
		lock \
		crtscts \
		nodefaultroute \
		usepeerdns \
		ipparam "${interface}" \
		ip-up-script /lib/netifd/ppp-up \
		ip-down-script /lib/netifd/ppp-down
}

modemmanager_disconnected_method_ppp_ipv4() {
	local interface="$1"

	echo "running disconnection (ppp method)"

	[ -n "${ERROR}" ] && {
		local errorstring
		errorstring=$(ppp_exitcode_tostring "${ERROR}")
		case "$ERROR" in
			0)
				;;
			2)
				proto_notify_error "$interface" "$errorstring"
				proto_block_restart "$interface"
				;;
			*)
				proto_notify_error "$interface" "$errorstring"
				;;
		esac
	} || echo "pppd result code not given"

	proto_kill_command "$interface"
}

modemmanager_connected_method_dhcp_ipv4() {
	local interface="$1"
	local wwan="$2"
	local metric="$3"

	proto_init_update "${wwan}" 1
	proto_set_keep 1
	proto_send_update "${interface}"

	json_init
	json_add_string name "${interface}_4"
	json_add_string ifname "@${interface}"
	json_add_string proto "dhcp"
	proto_add_dynamic_defaults
	[ -n "$metric" ] && json_add_int metric "${metric}"
	json_close_object
	ubus call network add_dynamic "$(json_dump)"
}

modemmanager_connected_method_static_ipv4() {
	local interface="$1"
	local wwan="$2"
	local address="$3"
	local prefix="$4"
	local gateway="$5"
	local mtu="$6"
	local dns1="$7"
	local dns2="$8"
	local metric="$9"

	local mask=""

	[ -n "${address}" ] || {
		proto_notify_error "${interface}" ADDRESS_MISSING
		return
	}

	[ -n "${prefix}" ] || {
		proto_notify_error "${interface}" PREFIX_MISSING
		return
	}
	mask=$(cdr2mask "${prefix}")

	[ -n "${mtu}" ] && /sbin/ip link set dev "${wwan}" mtu "${mtu}"

	proto_init_update "${wwan}" 1
	proto_set_keep 1
	echo "adding IPv4 address ${address}, netmask ${mask}"
	proto_add_ipv4_address "${address}" "${mask}"
	[ -n "${gateway}" ] && {
		echo "adding default IPv4 route via ${gateway}"
		proto_add_ipv4_route "0.0.0.0" "0" "${gateway}" "${address}"
	}
	[ -n "${dns1}" ] && {
		echo "adding primary DNS at ${dns1}"
		proto_add_dns_server "${dns1}"
	}
	[ -n "${dns2}" ] && {
		echo "adding secondary DNS at ${dns2}"
		proto_add_dns_server "${dns2}"
	}
	[ -n "$metric" ] && json_add_int metric "${metric}"
	proto_send_update "${interface}"
}

modemmanager_connected_method_dhcp_ipv6() {
	local interface="$1"
	local wwan="$2"
	local metric="$3"

	proto_init_update "${wwan}" 1
	proto_set_keep 1
	proto_send_update "${interface}"

	json_init
	json_add_string name "${interface}_6"
	json_add_string ifname "@${interface}"
	json_add_string proto "dhcpv6"
	proto_add_dynamic_defaults
	json_add_string extendprefix 1 # RFC 7278: Extend an IPv6 /64 Prefix to LAN
	[ -n "$metric" ] && json_add_int metric "${metric}"
	json_close_object
	ubus call network add_dynamic "$(json_dump)"
}

modemmanager_connected_method_static_ipv6() {
	local interface="$1"
	local wwan="$2"
	local address="$3"
	local prefix="$4"
	local gateway="$5"
	local mtu="$6"
	local dns1="$7"
	local dns2="$8"
	local metric="$9"

	[ -n "${address}" ] || {
		proto_notify_error "${interface}" ADDRESS_MISSING
		return
	}

	[ -n "${prefix}" ] || {
		proto_notify_error "${interface}" PREFIX_MISSING
		return
	}

	[ -n "${mtu}" ] && /sbin/ip link set dev "${wwan}" mtu "${mtu}"

	proto_init_update "${wwan}" 1
	proto_set_keep 1
	echo "adding IPv6 address ${address}, prefix ${prefix}"
	proto_add_ipv6_address "${address}" "128"
	proto_add_ipv6_prefix "${address}/${prefix}"
	[ -n "${gateway}" ] && {
		echo "adding default IPv6 route via ${gateway}"
		proto_add_ipv6_route "${gateway}" "128"
		proto_add_ipv6_route "::0" "0" "${gateway}" "" "" "${address}/${prefix}"
	}
	[ -n "${dns1}" ] && {
		echo "adding primary DNS at ${dns1}"
		proto_add_dns_server "${dns1}"
	}
	[ -n "${dns2}" ] && {
		echo "adding secondary DNS at ${dns2}"
		proto_add_dns_server "${dns2}"
	}
	[ -n "$metric" ] && json_add_int metric "${metric}"
	proto_send_update "${interface}"
}

modemmanager_disconnected_method_common() {
	local interface="$1"

	echo "running disconnection (common)"
	proto_notify_error "${interface}" MM_DISCONNECT_IN_PROGRESS

	proto_init_update "*" 0
	proto_send_update "${interface}"
}

proto_modemmanager_init_config() {
	available=1
	no_device=1
	proto_config_add_string	 device
	proto_config_add_string	 apn
	proto_config_add_string	 'allowedauth:list(string)'
	proto_config_add_string	 username
	proto_config_add_string	 password
	proto_config_add_string	 pincode
	proto_config_add_string	 iptype
	proto_config_add_int	 signalrate
	proto_config_add_boolean lowpower
	proto_config_add_defaults
}

proto_modemmanager_setup() {
	local interface="$1"

	local modempath modemstatus bearercount bearerpath connectargs bearerstatus beareriface
	local bearermethod_ipv4 bearermethod_ipv6 auth cliauth
	local operatorname operatorid registration accesstech signalquality

	local device apn allowedauth username password pincode iptype metric signalrate

	local address prefix gateway mtu dns1 dns2

	json_get_vars device apn allowedauth username password pincode iptype metric signalrate

	# validate sysfs path given in config
	[ -n "${device}" ] || {
		echo "No device specified"
		proto_notify_error "${interface}" NO_DEVICE
		proto_set_available "${interface}" 0
		return 1
	}
	[ -e "${device}" ] || {
		echo "Device not found in sysfs"
		proto_set_available "${interface}" 0
		return 1
	}

	# validate that ModemManager is handling the modem at the sysfs path
	modemstatus=$(mmcli --modem="${device}" --output-keyvalue)
	modempath=$(modemmanager_get_field "${modemstatus}" "modem.dbus-path")
	[ -n "${modempath}" ] || {
		echo "Device not managed by ModemManager"
		proto_notify_error "${interface}" DEVICE_NOT_MANAGED
		proto_set_available "${interface}" 0
		return 1
	}
	echo "modem available at ${modempath}"

	# always cleanup before attempting a new connection, just in case
	modemmanager_cleanup_connection "${modemstatus}"

	# if allowedauth list given, build option string
	for auth in $allowedauth; do
		cliauth="${cliauth}${cliauth:+|}$auth"
	done

	# setup connect args; APN mandatory (even if it may be empty)
	echo "starting connection with apn '${apn}'..."
	proto_notify_error "${interface}" MM_CONNECT_IN_PROGRESS

	connectargs="apn=${apn}${iptype:+,ip-type=${iptype}}${cliauth:+,allowed-auth=${cliauth}}${username:+,user=${username}}${password:+,password=${password}}${pincode:+,pin=${pincode}}"
	mmcli --modem="${device}" --timeout 120 --simple-connect="${connectargs}" || {
		proto_notify_error "${interface}" MM_CONNECT_FAILED
		proto_block_restart "${interface}"
		return 1
	}

	# check if Signal refresh rate is set
	if [ -n "${signalrate}" ] && [ "${signalrate}" -eq "${signalrate}" ] 2>/dev/null; then
		echo "setting signal refresh rate to ${signalrate} seconds"
		mmcli --modem="${device}" --signal-setup="${signalrate}"
	else
		echo "signal refresh rate is not set"
	fi

	# log additional useful information
	modemstatus=$(mmcli --modem="${device}" --output-keyvalue)
	operatorname=$(modemmanager_get_field "${modemstatus}" "modem.3gpp.operator-name")
	[ -n "${operatorname}" ] && echo "network operator name: ${operatorname}"
	operatorid=$(modemmanager_get_field "${modemstatus}" "modem.3gpp.operator-code")
	[ -n "${operatorid}" ] && echo "network operator MCCMNC: ${operatorid}"
	registration=$(modemmanager_get_field "${modemstatus}" "modem.3gpp.registration-state")
	[ -n "${registration}" ] && echo "registration type: ${registration}"
	accesstech=$(modemmanager_get_multivalue_field "${modemstatus}" "modem.generic.access-technologies")
	[ -n "${accesstech}" ] && echo "access technology: ${accesstech}"
	signalquality=$(modemmanager_get_field "${modemstatus}" "modem.generic.signal-quality.value")
	[ -n "${signalquality}" ] && echo "signal quality: ${signalquality}%"

	# we won't like it if there are more than one bearers, as that would mean the
	# user manually created them, and that's unsupported by this proto
	bearercount=$(modemmanager_get_field "${modemstatus}" "modem.generic.bearers.length")
	[ -n "${bearercount}" ] && [ "$bearercount" -eq 1 ] || {
		proto_notify_error "${interface}" INVALID_BEARER_LIST
		return 1
	}

	# load connected bearer information
	bearerpath=$(modemmanager_get_field "${modemstatus}" "modem.generic.bearers.value\[1\]")
	bearerstatus=$(mmcli --bearer "${bearerpath}" --output-keyvalue)

	# load network interface and method information
	beareriface=$(modemmanager_get_field "${bearerstatus}" "bearer.status.interface")
	bearermethod_ipv4=$(modemmanager_get_field "${bearerstatus}" "bearer.ipv4-config.method")
	bearermethod_ipv6=$(modemmanager_get_field "${bearerstatus}" "bearer.ipv6-config.method")

	# setup IPv4
	[ -n "${bearermethod_ipv4}" ] && {
		echo "IPv4 connection setup required in interface ${interface}: ${bearermethod_ipv4}"
		case "${bearermethod_ipv4}" in
		"dhcp")
			modemmanager_connected_method_dhcp_ipv4 "${interface}" "${beareriface}" "${metric}"
			;;
		"static")
			address=$(modemmanager_get_field "${bearerstatus}" "bearer.ipv4-config.address")
			prefix=$(modemmanager_get_field "${bearerstatus}" "bearer.ipv4-config.prefix")
			gateway=$(modemmanager_get_field "${bearerstatus}" "bearer.ipv4-config.gateway")
			mtu=$(modemmanager_get_field "${bearerstatus}" "bearer.ipv4-config.mtu")
			dns1=$(modemmanager_get_field "${bearerstatus}" "bearer.ipv4-config.dns.value\[1\]")
			dns2=$(modemmanager_get_field "${bearerstatus}" "bearer.ipv4-config.dns.value\[2\]")
			modemmanager_connected_method_static_ipv4 "${interface}" "${beareriface}" "${address}" "${prefix}" "${gateway}" "${mtu}" "${dns1}" "${dns2}" "${metric}"
			;;
		"ppp")
			modemmanager_connected_method_ppp_ipv4 "${interface}" "${beareriface}" "${username}" "${password}" "${allowedauth}"
			;;
		*)
			proto_notify_error "${interface}" UNKNOWN_METHOD
			return 1
			;;
		esac
	}

	# setup IPv6
	# note: if using ipv4v6, both IPv4 and IPv6 settings will have the same MTU and metric values reported
	[ -n "${bearermethod_ipv6}" ] && {
		echo "IPv6 connection setup required in interface ${interface}: ${bearermethod_ipv6}"
		case "${bearermethod_ipv6}" in
		"dhcp")
			modemmanager_connected_method_dhcp_ipv6 "${interface}" "${beareriface}" "${metric}"
			;;
		"static")
			address=$(modemmanager_get_field "${bearerstatus}" "bearer.ipv6-config.address")
			prefix=$(modemmanager_get_field "${bearerstatus}" "bearer.ipv6-config.prefix")
			gateway=$(modemmanager_get_field "${bearerstatus}" "bearer.ipv6-config.gateway")
			mtu=$(modemmanager_get_field "${bearerstatus}" "bearer.ipv6-config.mtu")
			dns1=$(modemmanager_get_field "${bearerstatus}" "bearer.ipv6-config.dns.value\[1\]")
			dns2=$(modemmanager_get_field "${bearerstatus}" "bearer.ipv6-config.dns.value\[2\]")
			modemmanager_connected_method_static_ipv6 "${interface}" "${beareriface}" "${address}" "${prefix}" "${gateway}" "${mtu}" "${dns1}" "${dns2}" "${metric}"
			;;
		"ppp")
			proto_notify_error "${interface}" "unsupported method"
			return 1
			;;
		*)
			proto_notify_error "${interface}" UNKNOWN_METHOD
			return 1
			;;
		esac
	}

	return 0
}

proto_modemmanager_teardown() {
	local interface="$1"

	local modemstatus bearerpath errorstring
	local bearermethod_ipv4 bearermethod_ipv6

	local device lowpower iptype
	json_get_vars device lowpower iptype

	echo "stopping network"
	proto_notify_error "${interface}" MM_TEARDOWN_IN_PROGRESS

	# load connected bearer information, just the first one should be ok
	modemstatus=$(mmcli --modem="${device}" --output-keyvalue)
	bearerpath=$(modemmanager_get_field "${modemstatus}" "modem.generic.bearers.value\[1\]")
	[ -n "${bearerpath}" ] || {
		echo "couldn't load bearer path"
		return
	}

	# load bearer connection methods
	bearerstatus=$(mmcli --bearer "${bearerpath}" --output-keyvalue)
	bearermethod_ipv4=$(modemmanager_get_field "${bearerstatus}" "bearer.ipv4-config.method")
	[ -n "${bearermethod_ipv4}" ] &&
		echo "IPv4 connection teardown required in interface ${interface}: ${bearermethod_ipv4}"
	bearermethod_ipv6=$(modemmanager_get_field "${bearerstatus}" "bearer.ipv6-config.method")
	[ -n "${bearermethod_ipv6}" ] &&
		echo "IPv6 connection teardown required in interface ${interface}: ${bearermethod_ipv6}"

	# disconnection handling only requires special treatment in IPv4/PPP
	[ "${bearermethod_ipv4}" = "ppp" ] && modemmanager_disconnected_method_ppp_ipv4 "${interface}"
	modemmanager_disconnected_method_common "${interface}"

	# disconnect
	mmcli --modem="${device}" --simple-disconnect ||
		proto_notify_error "${interface}" DISCONNECT_FAILED

	# disable
	mmcli --modem="${device}" --disable
	proto_notify_error "${interface}" MM_MODEM_DISABLED

	# low power, only if requested
	[ "${lowpower:-0}" -lt 1 ] ||
		mmcli --modem="${device}" --set-power-state-low
}

[ -n "$INCLUDE_ONLY" ] || {
	add_protocol modemmanager
}
