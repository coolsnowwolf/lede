#!/bin/sh

. /lib/functions.sh
. ../netifd-proto.sh
init_proto "$@"

proto_4g_init_config() {
	no_device=1
        available=1
	
	proto_config_add_string "apn"
	proto_config_add_string "pincode"
	proto_config_add_string "username"
	proto_config_add_string "password"
	proto_config_add_string "metric"
}

proto_4g_setup() {
	local interface="$1"
	local ifname="$2"
	local usbid=`lsusb`
	local devicename deviceid
	local ec20 

	json_get_var apn apn
	json_get_var pincode pincode
	json_get_var username username
	json_get_var password password
	json_get_var metric metric

	ec20=`lsusb  | grep "2c7c:0125"``lsusb  | grep "05c6:9215"`

	ifname="eth1"                                     

	if [ "$ec20" != "" ]; then
		devicename=EC20
		if [ ! -c "/dev/qcqmi1" ]; then
			echo "no qcqmi1"
			sleep 15
			return 1
		fi
	fi

	if  [ ! -n "$apn" -o ! -n "$username" -o ! -n "$password" ]; then
		auth=""
	else
		auth=0
	fi

	case "$devicename" in
		EC20)
			quectel-CM \
				-s $apn $username $password $auth \
				-p $pincode &
			sleep 10
			echo "Starting DHCP"
        		proto_init_update "$ifname" 1
        		proto_send_update "$interface"

        		json_init
        		json_add_string name "${interface}_4"
        		json_add_string ifname "@$interface"
			json_add_int metric "$metric"
        		json_add_string proto "dhcp"
        		json_close_object
        		ubus call network add_dynamic "$(json_dump)"

        		json_init
        		json_add_string name "${interface}_6"
        		json_add_string ifname "@$interface"
			json_add_int metric "$metric"
        		json_add_string proto "dhcpv6"
        		json_close_object
        		ubus call network add_dynamic "$(json_dump)"

		;;
	esac
}

proto_4g_teardown() {
	local interface="$1"
	proto_kill_command "$interface"
}

add_protocol 4g
