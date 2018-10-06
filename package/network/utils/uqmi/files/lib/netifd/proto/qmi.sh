#!/bin/sh

[ -n "$INCLUDE_ONLY" ] || {
	. /lib/functions.sh
	. ../netifd-proto.sh
	init_proto "$@"
}

proto_qmi_init_config() {
	available=1
	no_device=1
	proto_config_add_string "device:device"
	proto_config_add_string apn
	proto_config_add_string auth
	proto_config_add_string username
	proto_config_add_string password
	proto_config_add_string pincode
	proto_config_add_int delay
	proto_config_add_string modes
	proto_config_add_string pdptype
	proto_config_add_int profile
	proto_config_add_boolean dhcpv6
	proto_config_add_boolean autoconnect
	proto_config_add_int plmn
	proto_config_add_defaults
}

proto_qmi_setup() {
	local interface="$1"
	local dataformat connstat
	local device apn auth username password pincode delay modes pdptype profile dhcpv6 autoconnect plmn $PROTO_DEFAULT_OPTIONS
	local cid_4 pdh_4 cid_6 pdh_6
	local ip_6 ip_prefix_length gateway_6 dns1_6 dns2_6
	json_get_vars device apn auth username password pincode delay modes pdptype profile dhcpv6 autoconnect plmn $PROTO_DEFAULT_OPTIONS

	[ "$metric" = "" ] && metric="0"

	[ -n "$ctl_device" ] && device=$ctl_device

	[ -n "$device" ] || {
		echo "No control device specified"
		proto_notify_error "$interface" NO_DEVICE
		proto_set_available "$interface" 0
		return 1
	}

	[ -n "$delay" ] && sleep "$delay"

	device="$(readlink -f $device)"
	[ -c "$device" ] || {
		echo "The specified control device does not exist"
		proto_notify_error "$interface" NO_DEVICE
		proto_set_available "$interface" 0
		return 1
	}

	devname="$(basename "$device")"
	devpath="$(readlink -f /sys/class/usbmisc/$devname/device/)"
	ifname="$( ls "$devpath"/net )"
	[ -n "$ifname" ] || {
		echo "The interface could not be found."
		proto_notify_error "$interface" NO_IFACE
		proto_set_available "$interface" 0
		return 1
	}

	while uqmi -s -d "$device" --get-pin-status | grep '"UIM uninitialized"' > /dev/null; do
		[ -e "$device" ] || return 1
		sleep 1;
	done

	[ -n "$pincode" ] && {
		uqmi -s -d "$device" --verify-pin1 "$pincode" > /dev/null || uqmi -s -d "$device" --uim-verify-pin1 "$pincode" > /dev/null || {
			echo "Unable to verify PIN"
			proto_notify_error "$interface" PIN_FAILED
			proto_block_restart "$interface"
			return 1
		}
	}

	[ -n "$plmn" ] && {
		local mcc mnc
		if [ "$plmn" = 0 ]; then
			mcc=0
			mnc=0
			echo "Setting PLMN to auto"
		else
			mcc=${plmn:0:3}
			mnc=${plmn:3}
			echo "Setting PLMN to $plmn"
		fi
		uqmi -s -d "$device" --set-plmn --mcc "$mcc" --mnc "$mnc" || {
			echo "Unable to set PLMN"
			proto_notify_error "$interface" PLMN_FAILED
			proto_block_restart "$interface"
			return 1
		}
	}

	# Cleanup current state if any
	uqmi -s -d "$device" --stop-network 0xffffffff --autoconnect

	# Set IP format
	uqmi -s -d "$device" --set-data-format 802.3
	uqmi -s -d "$device" --wda-set-data-format 802.3
	dataformat="$(uqmi -s -d "$device" --wda-get-data-format)"

	if [ "$dataformat" = '"raw-ip"' ]; then

		[ -f /sys/class/net/$ifname/qmi/raw_ip ] || {
			echo "Device only supports raw-ip mode but is missing this required driver attribute: /sys/class/net/$ifname/qmi/raw_ip"
			return 1
		}

		echo "Device does not support 802.3 mode. Informing driver of raw-ip only for $ifname .."
		echo "Y" > /sys/class/net/$ifname/qmi/raw_ip
	fi

	uqmi -s -d "$device" --sync

	echo "Waiting for network registration"
	while uqmi -s -d "$device" --get-serving-system | grep '"searching"' > /dev/null; do
		[ -e "$device" ] || return 1
		sleep 5;
	done

	[ -n "$modes" ] && uqmi -s -d "$device" --set-network-modes "$modes"

	echo "Starting network $interface"

	pdptype=$(echo "$pdptype" | awk '{print tolower($0)}')
	[ "$pdptype" = "ip" -o "$pdptype" = "ipv6" -o "$pdptype" = "ipv4v6" ] || pdptype="ip"

	if [ "$pdptype" = "ip" ]; then
		[ -z "$autoconnect" ] && autoconnect=1
		[ "$autoconnect" = 0 ] && autoconnect=""
	else
		[ "$autoconnect" = 1 ] || autoconnect=""
	fi

	[ "$pdptype" = "ip" -o "$pdptype" = "ipv4v6" ] && {
		cid_4=$(uqmi -s -d "$device" --get-client-id wds)
		if ! [ "$cid_4" -eq "$cid_4" ] 2> /dev/null; then
			echo "Unable to obtain client ID"
			proto_notify_error "$interface" NO_CID
			return 1
		fi

		uqmi -s -d "$device" --set-client-id wds,"$cid_4" --set-ip-family ipv4 > /dev/null

		pdh_4=$(uqmi -s -d "$device" --set-client-id wds,"$cid_4" \
			--start-network \
			${apn:+--apn $apn} \
			${profile:+--profile $profile} \
			${auth:+--auth-type $auth} \
			${username:+--username $username} \
			${password:+--password $password} \
			${autoconnect:+--autoconnect})

        # pdh_4 is a numeric value on success
		if ! [ "$pdh_4" -eq "$pdh_4" ] 2> /dev/null; then
			echo "Unable to connect IPv4"
			uqmi -s -d "$device" --set-client-id wds,"$cid_4" --release-client-id wds
			proto_notify_error "$interface" CALL_FAILED
			return 1
		fi

        # Check data connection state
		connstat=$(uqmi -s -d "$device" --get-data-status)
                [ "$connstat" == '"connected"' ] || {
                        echo "No data link!"
                        uqmi -s -d "$device" --set-client-id wds,"$cid_4" --release-client-id wds
                        proto_notify_error "$interface" CALL_FAILED
                        return 1
                }
	}

	[ "$pdptype" = "ipv6" -o "$pdptype" = "ipv4v6" ] && {
		cid_6=$(uqmi -s -d "$device" --get-client-id wds)
		if ! [ "$cid_6" -eq "$cid_6" ] 2> /dev/null; then
			echo "Unable to obtain client ID"
			proto_notify_error "$interface" NO_CID
			return 1
		fi

		uqmi -s -d "$device" --set-client-id wds,"$cid_6" --set-ip-family ipv6 > /dev/null

		pdh_6=$(uqmi -s -d "$device" --set-client-id wds,"$cid_6" \
			--start-network \
			${apn:+--apn $apn} \
			${profile:+--profile $profile} \
			${auth:+--auth-type $auth} \
			${username:+--username $username} \
			${password:+--password $password} \
			${autoconnect:+--autoconnect})

        # pdh_6 is a numeric value on success
		if ! [ "$pdh_6" -eq "$pdh_6" ] 2> /dev/null; then
			echo "Unable to connect IPv6"
			uqmi -s -d "$device" --set-client-id wds,"$cid_6" --release-client-id wds
			proto_notify_error "$interface" CALL_FAILED
			return 1
		fi

        # Check data connection state
		connstat=$(uqmi -s -d "$device" --get-data-status)
                [ "$connstat" == '"connected"' ] || {
                        echo "No data link!"
                        uqmi -s -d "$device" --set-client-id wds,"$cid_6" --release-client-id wds
                        proto_notify_error "$interface" CALL_FAILED
                        return 1
                }
	}

	echo "Setting up $ifname"
	proto_init_update "$ifname" 1
	proto_set_keep 1
	proto_add_data
	[ -n "$pdh_4" ] && {
		json_add_string "cid_4" "$cid_4"
		json_add_string "pdh_4" "$pdh_4"
	}
	[ -n "$pdh_6" ] && {
		json_add_string "cid_6" "$cid_6"
		json_add_string "pdh_6" "$pdh_6"
	}
	proto_close_data
	proto_send_update "$interface"
	[ -n "$pdh_6" ] && {
		if [ -z "$dhcpv6" -o "$dhcpv6" = 0 ]; then
			json_load "$(uqmi -s -d $device --set-client-id wds,$cid_6 --get-current-settings)"
			json_select ipv6
			json_get_var ip_6 ip
			json_get_var gateway_6 gateway
			json_get_var dns1_6 dns1
			json_get_var dns2_6 dns2
			json_get_var ip_prefix_length ip-prefix-length

			proto_init_update "$ifname" 1
			proto_set_keep 1
			proto_add_ipv6_address "$ip_6" "128"
			proto_add_ipv6_prefix "${ip_6}/${ip_prefix_length}"
			proto_add_ipv6_route "$gateway_6" "128"
			[ "$defaultroute" = 0 ] || proto_add_ipv6_route "::0" 0 "$gateway_6" "" "" "${ip_6}/${ip_prefix_length}"
			[ "$peerdns" = 0 ] || {
				proto_add_dns_server "$dns1_6"
				proto_add_dns_server "$dns2_6"
			}
			proto_send_update "$interface"
		else
			json_init
			json_add_string name "${interface}_6"
			json_add_string ifname "@$interface"
			json_add_string proto "dhcpv6"
			proto_add_dynamic_defaults
			# RFC 7278: Extend an IPv6 /64 Prefix to LAN
			json_add_string extendprefix 1
			json_close_object
			ubus call network add_dynamic "$(json_dump)"
		fi
	}

	[ -n "$pdh_4" ] && {
		json_init
		json_add_string name "${interface}_4"
		json_add_string ifname "@$interface"
		json_add_string proto "dhcp"
		proto_add_dynamic_defaults
		json_close_object
		ubus call network add_dynamic "$(json_dump)"
	}
}

qmi_wds_stop() {
	local cid="$1"
	local pdh="$2"

	[ -n "$cid" ] || return

	uqmi -s -d "$device" --set-client-id wds,"$cid" \
		--stop-network 0xffffffff \
		--autoconnect > /dev/null 2>&1

	[ -n "$pdh" ] && {
		uqmi -s -d "$device" --set-client-id wds,"$cid" \
			--stop-network "$pdh" > /dev/null 2>&1
	}

	uqmi -s -d "$device" --set-client-id wds,"$cid" \
		--release-client-id wds > /dev/null 2>&1
}

proto_qmi_teardown() {
	local interface="$1"

	local device cid_4 pdh_4 cid_6 pdh_6
	json_get_vars device

	[ -n "$ctl_device" ] && device=$ctl_device

	echo "Stopping network $interface"

	json_load "$(ubus call network.interface.$interface status)"
	json_select data
	json_get_vars cid_4 pdh_4 cid_6 pdh_6

	qmi_wds_stop "$cid_4" "$pdh_4"
	qmi_wds_stop "$cid_6" "$pdh_6"

	proto_init_update "*" 0
	proto_send_update "$interface"
}

[ -n "$INCLUDE_ONLY" ] || {
	add_protocol qmi
}
