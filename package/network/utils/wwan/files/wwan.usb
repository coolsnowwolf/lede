[ "$ACTION" = add -a "$DEVTYPE" = usb_device ] || exit 0

vid=$(cat /sys$DEVPATH/idVendor)
pid=$(cat /sys$DEVPATH/idProduct)
[ -f "/lib/network/wwan/$vid:$pid" ] || exit 0

find_wwan_iface() {
	local cfg="$1"
	local proto
	config_get proto "$cfg" proto
	[ "$proto" = wwan ] || return 0
	proto_set_available "$cfg" 1
	ifup $cfg
	exit 0
}

config_load network
config_foreach find_wwan_iface interface
