#!/bin/sh

[ -n "$INCLUDE_ONLY" ] || {
	. /lib/functions.sh
	. ../netifd-proto.sh
	init_proto "$@"
}
#DBG=-v

proto_mbim_init_config() {
	available=1
	no_device=1
	proto_config_add_string "device:device"
	proto_config_add_string pdptype
	proto_config_add_string apn
	proto_config_add_string pincode
	proto_config_add_string delay
	proto_config_add_string auth
	proto_config_add_string username
	proto_config_add_string password
	proto_config_add_defaults
}

_proto_mbim_setup() {
	local interface="$1"
	local tid=2

	local device pdptype apn pincode delay auth username password $PROTO_DEFAULT_OPTIONS
	json_get_vars device pdptype apn pincode delay auth username password $PROTO_DEFAULT_OPTIONS

	[ -n "$ctl_device" ] && device=$ctl_device

	pdptype=$(echo "$pdptype" | awk '{print tolower($0)}')
	case "$pdptype" in
		ip )
			pdptype="ipv4"
			;;
		ipv4 | ipv6 | ipv4v6 | ipv4-and-ipv6 )
			;;
		* )
			pdptype="default"
			;;
	esac

	[ -n "$device" ] || {
		echo "mbim[$$]" "No control device specified"
		proto_notify_error "$interface" NO_DEVICE
		proto_set_available "$interface" 0
		return 1
	}
	[ -c "$device" ] || {
		echo "mbim[$$]" "The specified control device does not exist"
		proto_notify_error "$interface" NO_DEVICE
		proto_set_available "$interface" 0
		return 1
	}

	devname="$(basename "$device")"
	devpath="$(readlink -f /sys/class/usbmisc/$devname/device/)"
	ifname="$( ls "$devpath"/net )"

	[ -n "$ifname" ] || {
		echo "mbim[$$]" "Failed to find matching interface"
		proto_notify_error "$interface" NO_IFNAME
		proto_set_available "$interface" 0
		return 1
	}

	[ -n "$apn" ] || {
		echo "mbim[$$]" "No APN specified"
		proto_notify_error "$interface" NO_APN
		return 1
	}

	[ -n "$delay" ] && sleep "$delay"

	echo "mbim[$$]" "Reading capabilities"
	umbim $DBG -n -d $device caps || {
		echo "mbim[$$]" "Failed to read modem caps"
		proto_notify_error "$interface" PIN_FAILED
		return 1
	}
	tid=$((tid + 1))

	[ "$pincode" ] && {
		echo "mbim[$$]" "Sending pin"
		umbim $DBG -n -t $tid -d $device unlock "$pincode" || {
			echo "mbim[$$]" "Unable to verify PIN"
			proto_notify_error "$interface" PIN_FAILED
			proto_block_restart "$interface"
			return 1
		}
	}
	tid=$((tid + 1))

	echo "mbim[$$]" "Checking pin"
	local pinstate="/var/run/mbim.$$.pinstate"
	umbim $DBG -n -t $tid -d $device pinstate > "$pinstate" 2>&1 || {
		local pin
		pin=$(awk '$2=="pin:" {print $5}' "$pinstate")
		# we only need pin1 (the SIM pin) to connect
		[ "$pin" = "pin1" ] && {
			echo "mbim[$$]" "PIN required"
			proto_notify_error "$interface" PIN_FAILED
			proto_block_restart "$interface"
			return 1
		}
	}
	tid=$((tid + 1))

	echo "mbim[$$]" "Checking subscriber"
	umbim $DBG -n -t $tid -d $device subscriber || {
		echo "mbim[$$]" "Subscriber init failed"
		proto_notify_error "$interface" NO_SUBSCRIBER
		return 1
	}
	tid=$((tid + 1))

	echo "mbim[$$]" "Register with network"
	umbim $DBG -n -t $tid -d $device registration || {
		echo "mbim[$$]" "Subscriber registration failed"
		proto_notify_error "$interface" NO_REGISTRATION
		return 1
	}
	tid=$((tid + 1))

	echo "mbim[$$]" "Attach to network"
	umbim $DBG -n -t $tid -d $device attach || {
		echo "mbim[$$]" "Failed to attach to network"
		proto_notify_error "$interface" ATTACH_FAILED
		return 1
	}
	tid=$((tid + 1))

	echo "mbim[$$]" "Connect to network"
	while ! umbim $DBG -n -t $tid -d $device connect "$pdptype:$apn" "$auth" "$username" "$password"; do
		tid=$((tid + 1))
		sleep 1;
	done
	tid=$((tid + 1))

	echo "mbim[$$]" "Connected, obtain IP address and configure interface"
	local config="/var/run/mbim.$$.config"
	umbim $DBG -n -t $tid -d $device config > "$config" || {
		echo "mbim[$$]" "Failed to obtain IP address"
		proto_notify_error "$interface" CONFIG_FAILED
		return 1
	}
	tid=$((tid + 1))

	proto_init_update "$ifname" 1
	proto_add_data
	json_add_int tid $tid
	proto_close_data
	proto_send_update "$interface"

	local ip_4 ip_6
	ip_4=$(awk '$1=="ipv4address:" {print $2}' "$config")
	ip_6=$(awk '$1=="ipv6address:" {print $2}' "$config")
	[ -n "$ip_4" ] || [ -n "$ip_6" ] || {
		echo "mbim[$$]" "Failed to obtain IP addresses"
		proto_notify_error "$interface" CONFIG_FAILED
		return 1
	}

	proto_init_update "$ifname" 1
	proto_set_keep 1
	local ip mask gateway mtu dns dns_servers

	[ -n "$ip_4" ] && {
		echo "mbim[$$]" "Configure IPv4 on $ifname"
		ip=${ip_4%%/*}
		mask=${ip_4##*/}
		gateway=$(awk '$1=="ipv4gateway:" {print $2}' "$config")
		mtu=$(awk '$1=="ipv4mtu:" {print $2}' "$config")
		[ "$mtu" ] && ip link set "$ifname" mtu "$mtu"

		proto_add_ipv4_address "$ip" "$mask"
		[ "$defaultroute" = 0 ] || proto_add_ipv4_route 0.0.0.0 0 "$gateway" "$ip_4" "$metric"
		[ "$peerdns" = 0 ] || {
			dns_servers=$(awk '$1=="ipv4dnsserver:" {printf "%s ",$2}' "$config")
			for dns in $dns_servers; do
				proto_add_dns_server "$dns"
			done
		}

	}
	[ -n "$ip_6" ] && {
		echo "mbim[$$]" "Configure IPv6 on $ifname"
		ip=${ip_6%%/*}
		mask=${ip_6##*/}
		gateway=$(awk '$1=="ipv6gateway:" {print $2}' "$config")
		mtu=$(awk '$1=="ipv6mtu:" {print $2}' "$config")
		[ "$mtu" ] && ip -6 link set "$ifname" mtu "$mtu"

		proto_add_ipv6_address "$ip" "128"
		proto_add_ipv6_prefix "$ip_6"
		proto_add_ipv6_route "$gateway" "128"
		[ "$defaultroute" = 0 ] || proto_add_ipv6_route "::" 0 "$gateway" "$metric" "" "$ip_6"
		[ "$peerdns" = 0 ] || {
			dns_servers=$(awk '$1=="ipv6dnsserver:" {printf "%s ",$2}' "$config")
			for dns in $dns_servers; do
				proto_add_dns_server "$dns"
			done
		}
	}

	proto_send_update "$interface"
	echo "mbim[$$]" "Connection setup complete"
}

proto_mbim_setup() {
	local ret

	_proto_mbim_setup "$@"
	ret=$?

	rm -f "/var/run/mbim.$$."*

	[ "$ret" = 0 ] || {
		logger "mbim bringup failed, retry in 15s"
		sleep 15
	}

	return $ret
}

proto_mbim_teardown() {
	local interface="$1"

	local device tid
	json_get_vars device tid

	[ -n "$ctl_device" ] && device=$ctl_device

	echo "mbim[$$]" "Stopping network"
	[ -n "$tid" ] && umbim $DBG -t$tid -d "$device" disconnect

	proto_init_update "*" 0
	proto_send_update "$interface"
}

[ -n "$INCLUDE_ONLY" ] || add_protocol mbim
