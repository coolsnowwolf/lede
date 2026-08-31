#
# Copyright (C) 2014-2016 OpenWrt.org
# Copyright (C) 2016 LEDE-Project.org
#

RAMFS_COPY_BIN='fw_printenv fw_setenv mkfs.f2fs fdisk'
RAMFS_COPY_DATA='/etc/fw_env.config /var/lock/fw_printenv.lock'
REQUIRE_IMAGE_METADATA=1

# U-Boot loads the first boot.scr it finds, so keep the active slot's script in
# sync even when the full upgrade is written to the inactive slot.
methode_update_active_bootscript() {
	local active_part="$1"
	local updated_part="$2"
	local active_dev
	local updated_dev
	local ret=0

	[ "$active_part" -a "$updated_part" ] || return 0

	active_dev="$(find_mmc_part "$active_part" "$CI_ROOTDEV")"
	updated_dev="$(find_mmc_part "$updated_part" "$CI_ROOTDEV")"
	[ "$active_dev" -a "$updated_dev" ] || return 1

	mkdir -p /tmp/bootpart-new
	mount -o ro "$updated_dev" /tmp/bootpart-new || return 1

	cp /tmp/bootpart-new/boot.scr /tmp/boot.scr
	ret=$?
	umount /tmp/bootpart-new
	[ $ret -eq 0 ] || return 1

	mkdir -p /tmp/bootpart-active
	mount "$active_dev" /tmp/bootpart-active || return 1

	cp /tmp/boot.scr /tmp/bootpart-active/boot.scr || ret=1
	sync

	umount /tmp/bootpart-active

	return $ret
}

platform_check_image() {
	case "$(board_name)" in
	cznic,turris-mox|\
	glinet,gl-mv1000|\
	globalscale,espressobin|\
	globalscale,espressobin-emmc|\
	globalscale,espressobin-ultra|\
	globalscale,espressobin-v7|\
	globalscale,espressobin-v7-emmc)
		legacy_sdcard_check_image "$1"
		;;
	*)
		return 0
		;;
	esac
}

platform_do_upgrade() {
	case "$(board_name)" in
	cznic,turris-mox|\
	glinet,gl-mv1000|\
	globalscale,espressobin|\
	globalscale,espressobin-emmc|\
	globalscale,espressobin-ultra|\
	globalscale,espressobin-v7|\
	globalscale,espressobin-v7-emmc)
		legacy_sdcard_do_upgrade "$1"
		;;
	methode,udpu|\
	methode,edpu)
		local active_kernpart

		[ "$(rootfs_type)" = "tmpfs" ] && {
			local firmware_active="$(fw_printenv -n bootactive)"
			case "$firmware_active" in
			1)
				active_kernpart="kernel_1"
				CI_KERNPART="kernel_2"
				CI_ROOTPART="rootfs_2"
				fw_setenv bootactive 2
				;;
			2)
				active_kernpart="kernel_2"
				CI_KERNPART="kernel_1"
				CI_ROOTPART="rootfs_1"
				fw_setenv bootactive 1
				;;
			esac
		}

		local root="$(cmdline_get_var root)"
		case "$root" in
		/dev/mmcblk*p2)
			active_kernpart="kernel_1"
			CI_KERNPART="kernel_2"
			CI_ROOTPART="rootfs_2"
			fw_setenv bootactive 2
			;;
		/dev/mmcblk*p4)
			active_kernpart="kernel_2"
			CI_KERNPART="kernel_1"
			CI_ROOTPART="rootfs_1"
			fw_setenv bootactive 1
			;;
		esac

		emmc_do_upgrade "$1"

		methode_update_active_bootscript "$active_kernpart" "$CI_KERNPART" || \
			echo "Failed to update active boot script"
		;;
	*)
		default_do_upgrade "$1"
		;;
	esac
}
platform_copy_config() {
	case "$(board_name)" in
	cznic,turris-mox|\
	glinet,gl-mv1000|\
	globalscale,espressobin|\
	globalscale,espressobin-emmc|\
	globalscale,espressobin-ultra|\
	globalscale,espressobin-v7|\
	globalscale,espressobin-v7-emmc)
		legacy_sdcard_copy_config
		;;
	methode,udpu|\
	methode,edpu)
		emmc_copy_config
		;;
	esac
}
