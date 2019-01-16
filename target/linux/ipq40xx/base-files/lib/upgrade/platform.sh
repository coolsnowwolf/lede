PART_NAME=firmware
REQUIRE_IMAGE_METADATA=1

RAMFS_COPY_BIN='fw_printenv fw_setenv'
RAMFS_COPY_DATA='/etc/fw_env.config /var/lock/fw_printenv.lock'

ubi_kill_if_exist() {
	local ubidev="$( nand_find_ubi "$CI_UBIPART" )"
	local c_ubivol="$( nand_find_volume $ubidev $1 )"
	umount -f /dev/$c_ubivol 2>/dev/null
	[ "$c_ubivol" ] && ubirmvol /dev/$ubidev -N $1 || true
	echo "Partition $1 removed."
}

# idea from @981213
# Tar sysupgrade for ASUS RT-AC82U/RT-AC58U
# An ubi repartition is required due to the strange partition table created by Asus.
# We create all the factory partitions to make sure that the U-boot tftp recovery still works.
# The reserved kernel partition size should be enough to put the factory image in.
asus_nand_upgrade_tar() {
	local kpart_size="$1"
	local tar_file="$2"

	local board_dir=$(tar tf $tar_file | grep -m 1 '^sysupgrade-.*/$')
	board_dir=${board_dir%/}

	local kernel_length=`(tar xf $tar_file ${board_dir}/kernel -O | wc -c) 2> /dev/null`
	local rootfs_length=`(tar xf $tar_file ${board_dir}/root -O | wc -c) 2> /dev/null`

	local mtdnum="$( find_mtd_index "$CI_UBIPART" )"
	if [ ! "$mtdnum" ]; then
		echo "cannot find ubi mtd partition $CI_UBIPART"
		return 1
	fi

	local ubidev="$( nand_find_ubi "$CI_UBIPART" )"
	if [ ! "$ubidev" ]; then
		ubiattach -m "$mtdnum"
		sync
		ubidev="$( nand_find_ubi "$CI_UBIPART" )"
	fi

	if [ ! "$ubidev" ]; then
		echo "cannot find ubi device $CI_UBIPART"
		return 1
	fi

	local root_ubivol="$( nand_find_volume $ubidev rootfs )"
	# remove ubiblock device of rootfs
	local root_ubiblk="ubiblock${root_ubivol:3}"
	if [ "$root_ubivol" -a -e "/dev/$root_ubiblk" ]; then
		echo "removing $root_ubiblk"
		if ! ubiblock -r /dev/$root_ubivol; then
			echo "cannot remove $root_ubiblk"
			return 1;
		fi
	fi

	ubi_kill_if_exist rootfs_data
	ubi_kill_if_exist rootfs
	ubi_kill_if_exist jffs2
	ubi_kill_if_exist linux2
	ubi_kill_if_exist linux

	ubimkvol /dev/$ubidev -N linux -s $kpart_size
	ubimkvol /dev/$ubidev -N linux2 -s $kpart_size
	ubimkvol /dev/$ubidev -N jffs2 -s 2539520
	ubimkvol /dev/$ubidev -N rootfs -s $rootfs_length
	ubimkvol /dev/$ubidev -N rootfs_data -m

	local kern_ubivol="$(nand_find_volume $ubidev $CI_KERNPART)"
	echo "Kernel at $kern_ubivol.Writing..."
	tar xf $tar_file ${board_dir}/kernel -O | \
		ubiupdatevol /dev/$kern_ubivol -s $kernel_length -
	echo "Done."

	local root_ubivol="$(nand_find_volume $ubidev rootfs)"
	echo "Rootfs at $root_ubivol.Writing..."
	tar xf $tar_file ${board_dir}/root -O | \
		ubiupdatevol /dev/$root_ubivol -s $rootfs_length -
	echo "Done."

	nand_do_upgrade_success
}

# idea from @981213
# Factory image sysupgrade for ASUS RT-AC82U/RT-AC58U
# Delete all the partitions we created before, create "linux" partition and write factory image in.
# Skip the first 64bytes which is an uImage header to verify the firmware.
# The kernel partition size should be the original one.
asus_nand_upgrade_factory() {
	local kpart_size="$1"
	local fw_file="$2"

	local mtdnum="$( find_mtd_index "$CI_UBIPART" )"
	if [ ! "$mtdnum" ]; then
		echo "cannot find ubi mtd partition $CI_UBIPART"
		return 1
	fi

	local ubidev="$( nand_find_ubi "$CI_UBIPART" )"
	if [ ! "$ubidev" ]; then
		ubiattach -m "$mtdnum"
		sync
		ubidev="$( nand_find_ubi "$CI_UBIPART" )"
	fi

	if [ ! "$ubidev" ]; then
		echo "cannot find ubi device $CI_UBIPART"
		return 1
	fi

	local root_ubivol="$( nand_find_volume $ubidev rootfs )"
	# remove ubiblock device of rootfs
	local root_ubiblk="ubiblock${root_ubivol:3}"
	if [ "$root_ubivol" -a -e "/dev/$root_ubiblk" ]; then
		echo "removing $root_ubiblk"
		if ! ubiblock -r /dev/$root_ubivol; then
			echo "cannot remove $root_ubiblk"
			return 1;
		fi
	fi

	ubi_kill_if_exist rootfs_data
	ubi_kill_if_exist rootfs
	ubi_kill_if_exist jffs2
	ubi_kill_if_exist linux2
	ubi_kill_if_exist linux

	ubimkvol /dev/$ubidev -N linux -s $kpart_size

	local kern_ubivol="$(nand_find_volume $ubidev $CI_KERNPART)"
	echo "Asus linux at $kern_ubivol.Writing..."
	ubiupdatevol /dev/$kern_ubivol --skip=64 $fw_file
	echo "Done."

	umount -a
	reboot -f
}

platform_check_image() {
	return 0;
}

zyxel_do_upgrade() {
	local tar_file="$1"

	local board_dir=$(tar tf $tar_file | grep -m 1 '^sysupgrade-.*/$')
	board_dir=${board_dir%/}

	tar Oxf $tar_file ${board_dir}/kernel | mtd write - kernel

	if [ "$SAVE_CONFIG" -eq 1 ]; then
		tar Oxf $tar_file ${board_dir}/root | mtd -j "$CONF_TAR" write - rootfs
	else
		tar Oxf $tar_file ${board_dir}/root | mtd write - rootfs
	fi
}

platform_do_upgrade() {
	case "$(board_name)" in
	8dev,jalapeno)
		nand_do_upgrade "$ARGV"
		;;
	asus,rt-ac1300uhp |\
	asus,rt-acrh17|\
	linksys,ea6350v3 |\
	asus,rt-ac58u)
		local magic=$(get_magic_long "$1")
		CI_UBIPART="UBI_DEV"
		CI_KERNPART="linux"
		if [ "$magic" == "27051956" ]; then
			echo "Got Asus factory image."
			asus_nand_upgrade_factory 50409472 "$1"
		else
			asus_nand_upgrade_tar 20951040 "$1"
		fi
		;;
	openmesh,a42 |\
	openmesh,a62)
		PART_NAME="inactive"
		platform_do_upgrade_openmesh "$ARGV"
		;;
	meraki,mr33)
		CI_KERNPART="part.safe"
		nand_do_upgrade "$1"
		;;
	zyxel,nbg6617)
		zyxel_do_upgrade "$1"
		;;
	*)
		default_do_upgrade "$ARGV"
		;;
	esac
}

platform_nand_pre_upgrade() {
	case "$(board_name)" in
	linksys,ea6350v3)
		CI_UBIPART="UBI_DEV"
		CI_KERNPART="linux"
		;;
	meraki,mr33)
		CI_KERNPART="part.safe"
		;;
	esac
}
