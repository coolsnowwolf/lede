. /lib/functions.sh

load_wifi() {
	local kernel_version=$(uname -r)
	[ -e "/lib/modules/$kernel_version/mt7603e.ko" ] && modprobe mt7603e
	[ -e "/lib/modules/$kernel_version/mt76x2_ap.ko" ] && modprobe mt76x2_ap
}

boot_hook_add preinit_main load_wifi
