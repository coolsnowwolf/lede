KERNEL_IMG="openwrt-au1000-au1500-vmlinux.bin"
ROOTFS_IMG="openwrt-au1000-au1500-root.fs"

platform_check_image() {
	[ "$#" -gt 1 ] && return 1
	case "$(get_magic_word "$1")" in
		6f70)
			( get_image "$1" | tar -tf - $KERNEL_IMG >/dev/null && \
			  get_image "$1" | tar -tf - $ROOTFS_IMG >/dev/null) || {
				echo "Invalid image contents"
				return 1
			}
			return 0;;
		*)
			echo "Invalid image type"
			return 1;;
	esac
}

platform_do_upgrade() {
	sync
	local conf=""
	[ -f "$CONF_TAR" -a "$SAVE_CONFIG" -eq 1 ] && conf="-j $CONF_TAR"
	get_image "$1" | tar -Oxvf - $KERNEL_IMG | mtd write - "kernel"
	get_image "$1" | tar -Oxvf - $ROOTFS_IMG | mtd $conf write - "rootfs"
}
