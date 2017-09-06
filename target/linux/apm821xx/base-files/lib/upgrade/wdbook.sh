. /lib/functions.sh

get_magic_at() {
	local file="$1"
	local pos="$2"
	get_image "$file" | dd bs=1 count=2 skip="$pos" 2>/dev/null | hexdump -v -n 2 -e '1/1 "%02x"'
}

mbl_do_platform_check() {
	local board="$1"
	local file="$2"
	local magic

	magic=$(get_magic_at "$file" 510)

	[ "$magic" != "55aa" ] && {
		echo "Failed to verify MBR boot signature."
		return 1
	}

	return 0;
}

mbl_do_upgrade() {
	sync
	get_image "$1" | dd of=/dev/sda bs=2M conv=fsync
	sleep 1
}

mbl_copy_config() {
	mkdir -p /boot
	[ -f /boot/uImage ] || mount -t ext4 -o rw,noatime /dev/sda1 /boot
	cp -af "$CONF_TAR" /boot/
	sync
	umount /boot
}
