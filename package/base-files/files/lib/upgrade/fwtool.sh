fwtool_check_image() {
	[ $# -gt 1 ] && return 1

	. /usr/share/libubox/jshn.sh

	if ! fwtool -q -i /tmp/sysupgrade.meta "$1"; then
		echo "Image metadata not found"
		[ "$REQUIRE_IMAGE_METADATA" = 1 -a "$FORCE" != 1 ] && {
			echo "Use sysupgrade -F to override this check when downgrading or flashing to vendor firmware"
		}
		[ "$REQUIRE_IMAGE_METADATA" = 1 ] && return 1
		return 0
	fi

	json_load "$(cat /tmp/sysupgrade.meta)" || {
		echo "Invalid image metadata"
		return 1
	}

	device="$(cat /tmp/sysinfo/board_name)"

	json_select supported_devices || return 1

	json_get_keys dev_keys
	for k in $dev_keys; do
		json_get_var dev "$k"
		[ "$dev" = "$device" ] && return 0
	done

	echo "Device $device not supported by this image"
	echo -n "Supported devices:"
	for k in $dev_keys; do
		json_get_var dev "$k"
		echo -n " $dev"
	done
	echo

	return 1
}
