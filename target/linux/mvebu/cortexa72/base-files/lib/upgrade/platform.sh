#
# Copyright (C) 2014-2016 OpenWrt.org
# Copyright (C) 2016 LEDE-Project.org
#

RAMFS_COPY_BIN='fw_printenv fw_setenv'
RAMFS_COPY_DATA='/etc/fw_env.config /var/lock/fw_printenv.lock'
REQUIRE_IMAGE_METADATA=1

platform_check_image() {
	case "$(board_name)" in
	checkpoint,v-80|\
	checkpoint,v-81)
		local root="$(cmdline_get_var root)"
		case "$root" in
		PARTUUID=????????-02) # USB or SD
			legacy_sdcard_check_image "$1"
			;;
		""|\
		/dev/mmcblk1p3)       # initramfs or eMMC
			# check sysupgrade tar image
			local gz="$(identify_if_gzip "$1")"
			tar t${gz}f "$1" | grep -q "sysupgrade-.*/CONTROL" || {
				v "image is not a valid tar file for sysupgrade on eMMC!"
				return 1
			}
			# check availability of partitions
			local part_name part_dev
			for part_name in "kernel-1" "dtb-1" "rootfs-1"; do
				part_dev="$(find_mmc_part $part_name mmcblk1)"
				[ -n "$part_dev" ] || {
					v "eMMC partition \"$part_name\" not found!"
					return 1
				}
			done
			;;
		*)
			return 1
			;;
		esac
		;;
	globalscale,mochabin|\
	iei,puzzle-m901|\
	iei,puzzle-m902|\
	marvell,armada8040-mcbin-doubleshot|\
	marvell,armada8040-mcbin-singleshot|\
	marvell,armada8040-clearfog-gt-8k|\
	solidrun,clearfog-pro)
		legacy_sdcard_check_image "$1"
		;;
	*)
		return 0
		;;
	esac
}

platform_do_upgrade() {
	case "$(board_name)" in
	checkpoint,v-80|\
	checkpoint,v-81)
		local root="$(cmdline_get_var root)"
		case "$root" in
		PARTUUID=????????-02) # USB or SD
			legacy_sdcard_do_upgrade "$1"
			;;
		*)                    # initramfs or eMMC
			CI_ROOTDEV="mmcblk1"
			CI_KERNPART="kernel-1"
			CI_ROOTPART="rootfs-1"
			CI_DTBPART="dtb-1"
			emmc_do_upgrade "$1"
			;;
		esac
		;;
	iei,puzzle-m901|\
	iei,puzzle-m902)
		platform_do_upgrade_emmc "$1"
		;;
	globalscale,mochabin|\
	marvell,armada8040-mcbin-doubleshot|\
	marvell,armada8040-mcbin-singleshot|\
	marvell,armada8040-clearfog-gt-8k|\
	solidrun,clearfog-pro)
		legacy_sdcard_do_upgrade "$1"
		;;
	mikrotik,rb5009ug|\
	mikrotik,rb5009upr)
		nand_do_upgrade "$1"
		;;
	*)
		default_do_upgrade "$1"
		;;
	esac
}
platform_copy_config() {
	case "$(board_name)" in
	checkpoint,v-80|\
	checkpoint,v-81)
		local root="$(cmdline_get_var root)"
		case "$root" in
		PARTUUID=????????-02) # USB or SD
			legacy_sdcard_copy_config
			;;
		*)                    # initramfs or eMMC
			mkdir /tmp/new_root
			if ! mount -t ext4 /dev/mmcblk1p3 /tmp/new_root 2>/dev/null; then
				emmc_copy_config
			else
				v "new rootfs is ext4, saving configurations..."
				cp "$UPGRADE_BACKUP" "/tmp/new_root/$BACKUP_FILE"
				umount /tmp/new_root
			fi
			;;
		esac
		;;
	globalscale,mochabin|\
	iei,puzzle-m901|\
	iei,puzzle-m902|\
	marvell,armada8040-mcbin-doubleshot|\
	marvell,armada8040-mcbin-singleshot|\
	marvell,armada8040-clearfog-gt-8k|\
	solidrun,clearfog-pro)
		legacy_sdcard_copy_config
		;;
	esac
}
