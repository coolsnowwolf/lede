#!/bin/sh

. /lib/functions.sh
. ../netifd-proto.sh
init_proto "$@"

INCLUDE_ONLY=1

ctl_device=""
dat_device=""

proto_mbim_setup() { echo "wwan[$$] mbim proto is missing"; }
proto_qmi_setup() { echo "wwan[$$] qmi proto is missing"; }
proto_ncm_setup() { echo "wwan[$$] ncm proto is missing"; }
proto_3g_setup() { echo "wwan[$$] 3g proto is missing"; }
proto_directip_setup() { echo "wwan[$$] directip proto is missing"; }

[ -f ./mbim.sh ] && . ./mbim.sh
[ -f ./ncm.sh ] && . ./ncm.sh
[ -f ./qmi.sh ] && . ./qmi.sh
[ -f ./3g.sh ] && { . ./ppp.sh; . ./3g.sh; }
[ -f ./directip.sh ] && . ./directip.sh

proto_wwan_init_config() {
	available=1
	no_device=1

	proto_config_add_string apn
	proto_config_add_string auth
	proto_config_add_string username
	proto_config_add_string password
	proto_config_add_string pincode
	proto_config_add_string delay
	proto_config_add_string modes
	proto_config_add_string bus
}

proto_wwan_setup() {
	local driver usb devicename desc bus

	json_get_vars bus

	if [ -L "/sys/bus/usb/devices/${bus}" ]; then
		if [ -f "/sys/bus/usb/devices/${bus}/idVendor" ] \
			&& [ -f "/sys/bus/usb/devices/${bus}/idProduct" ]; then
			local vendor product
			vendor=$(cat /sys/bus/usb/devices/${bus}/idVendor)
			product=$(cat /sys/bus/usb/devices/${bus}/idProduct)
			[ -f /lib/network/wwan/$vendor:$product ] && {
				usb=/lib/network/wwan/$vendor:$product
				devicename=$bus
			}
		else
			echo "wwan[$$]" "Specified usb bus ${bus} was not found"
			proto_notify_error "$interface" NO_USB
			proto_block_restart "$interface"
			return 1
		fi
	else
		echo "wwan[$$]" "Searching for a valid wwan usb device..."
		for a in $(ls /sys/bus/usb/devices); do
			local vendor product
			[ -z "$usb" -a -f /sys/bus/usb/devices/$a/idVendor -a  -f /sys/bus/usb/devices/$a/idProduct ] || continue
			vendor=$(cat /sys/bus/usb/devices/$a/idVendor)
			product=$(cat /sys/bus/usb/devices/$a/idProduct)
			[ -f /lib/network/wwan/$vendor:$product ] && {
				usb=/lib/network/wwan/$vendor:$product
				devicename=$a
			}
		done
	fi

	echo "wwan[$$]" "Using wwan usb device on bus $devicename"

	[ -n "$usb" ] && {
		local old_cb control data

		json_set_namespace wwan old_cb
		json_init
		json_load "$(cat "$usb")"
		json_select
		json_get_vars desc control data
		json_set_namespace "$old_cb"

		[ -n "$control" -a -n "$data" ] && {
			ttys=$(ls -d /sys/bus/usb/devices/$devicename/${devicename}*/tty?* /sys/bus/usb/devices/$devicename/${devicename}*/tty/tty?* | sed "s/.*\///g" | tr "\n" " ")
			ctl_device=/dev/$(echo $ttys | cut -d" " -f $((control + 1)))
			dat_device=/dev/$(echo $ttys | cut -d" " -f $((data + 1)))
			driver=comgt
		}
	}

	[ -z "$ctl_device" ] && for net in $(ls /sys/class/net/ | grep -e wwan -e usb); do
		[ -z "$ctl_device" ] || continue
		[ -n "$bus" ] && {
			[ $(readlink /sys/class/net/$net | grep $bus) ] || continue
		}
		driver=$(grep DRIVER /sys/class/net/$net/device/uevent | cut -d= -f2)
		case "$driver" in
		qmi_wwan|cdc_mbim)
			ctl_device=/dev/$(ls /sys/class/net/$net/device/usbmisc)
			;;
		sierra_net|cdc_ether|*cdc_ncm)
			ctl_device=/dev/$(cd /sys/class/net/$net/; find ../../../ -name ttyUSB* |xargs -n1 basename | head -n1)
			;;
		*) continue;;
		esac
		echo "wwan[$$]" "Using proto:$proto device:$ctl_device iface:$net desc:$desc"
	done

	[ -n "$ctl_device" ] || {
		echo "wwan[$$]" "No valid device was found"
		proto_notify_error "$interface" NO_DEVICE
		proto_block_restart "$interface"
		return 1
	}

	uci_set_state network "$interface" driver "$driver"
	uci_set_state network "$interface" ctl_device "$ctl_device"
	uci_set_state network "$interface" dat_device "$dat_device"

	case $driver in
	qmi_wwan)		proto_qmi_setup $@ ;;
	cdc_mbim)		proto_mbim_setup $@ ;;
	sierra_net)		proto_directip_setup $@ ;;
	comgt)			proto_3g_setup $@ ;;
	cdc_ether|*cdc_ncm)	proto_ncm_setup $@ ;;
	esac
}

proto_wwan_teardown() {
	local interface=$1
	local driver=$(uci_get_state network "$interface" driver)
	ctl_device=$(uci_get_state network "$interface" ctl_device)
	dat_device=$(uci_get_state network "$interface" dat_device)

	case $driver in
	qmi_wwan)		proto_qmi_teardown $@ ;;
	cdc_mbim)		proto_mbim_teardown $@ ;;
	sierra_net)		proto_directip_teardown $@ ;;
	comgt)			proto_3g_teardown $@ ;;
	cdc_ether|*cdc_ncm)	proto_ncm_teardown $@ ;;
	esac
}

add_protocol wwan
