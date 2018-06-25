get_magic_at() {
	local file="$1"
	local pos="$2"
	get_image "$file" | dd bs=1 count=2 skip="$pos" 2>/dev/null | hexdump -v -n 2 -e '1/1 "%02x"'
}

platform_check_image_sdcard() {
	local file="$1"
	local magic

	magic=$(get_magic_at "$file" 510)
	[ "$magic" != "55aa" ] && {
		echo "Failed to verify MBR boot signature."
		return 1
	}

	return 0;
}

platform_do_upgrade_sdcard() {
	local board=$(board_name)

	sync
	get_image "$1" | dd of=/dev/mmcblk0 bs=2M conv=fsync

	case "$board" in
	armada-385-turris-omnia)
		fw_setenv openwrt_bootargs 'earlyprintk console=ttyS0,115200 root=/dev/mmcblk0p2 rootfstype=auto rootwait'
		fw_setenv openwrt_mmcload 'setenv bootargs "$openwrt_bootargs cfg80211.freg=$regdomain"; fatload mmc 0 0x01000000 zImage; fatload mmc 0 0x02000000 armada-385-turris-omnia.dtb'
		fw_setenv factory_mmcload 'setenv bootargs "$bootargs cfg80211.freg=$regdomain"; btrload mmc 0 0x01000000 boot/zImage @; btrload mmc 0 0x02000000 boot/dtb @'
		fw_setenv mmcboot 'run openwrt_mmcload || run factory_mmcload; bootz 0x01000000 - 0x02000000'
		;;
	esac

	sleep 1
}

platform_copy_config_sdcard() {
	mkdir -p /boot
	[ -f /boot/kernel.img ] || mount -o rw,noatime /dev/mmcblk0p1 /boot
	cp -af "$CONF_TAR" /boot/
	sync
	umount /boot
}
