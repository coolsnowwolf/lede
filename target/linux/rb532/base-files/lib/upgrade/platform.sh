REQUIRE_IMAGE_METADATA=1
RAMFS_COPY_BIN='nandwrite'
CI_KERNPART=none

platform_check_image() {
	[ -e /dev/ubi0 ] || {
		ubiattach -m 1
		sleep 1
	}
	return 0;
}

platform_do_upgrade() {
	mtd erase kernel
	tar xf "$1" "sysupgrade-$(board_name)/kernel" -O | nandwrite -o /dev/mtd0 -
	nand_do_upgrade "$1"
}
