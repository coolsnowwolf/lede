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
	case "$(board_name)" in
	zte,mf286d |\
	zte,mf289f)
		CI_UBIPART="rootfs"
		local mtdnum="$( find_mtd_index $CI_UBIPART )"
		[ ! "$mtdnum" ] && return 1
		ubiattach -m "$mtdnum" || true
		local ubidev="$( nand_find_ubi $CI_UBIPART )"
		local ubi_rootfs=$(nand_find_volume $ubidev ubi_rootfs)
		local ubi_rootfs_data=$(nand_find_volume $ubidev ubi_rootfs_data)

		[ -n "$ubi_rootfs" ] || [ -n "$ubi_rootfs_data" ] || return 0

		cat << EOF
ubi_rootfs partition is still present.

You need to delete the stock partition first:
# ubirmvol /dev/ubi0 -N ubi_rootfs
Please also delete ubi_rootfs_data, if exist:
# ubirmvol /dev/ubi0 -N ubi_rootfs_data

Once this is done. Retry.
EOF
		return 1
		;;
	esac
	return 0;
}

askey_do_upgrade() {
	local tar_file="$1"

	local board_dir=$(tar tf $tar_file | grep -m 1 '^sysupgrade-.*/$')
	board_dir=${board_dir%/}

	tar Oxf $tar_file ${board_dir}/root | mtd write - rootfs

	nand_do_upgrade "$1"
}

zyxel_do_upgrade() {
	local tar_file="$1"

	local board_dir=$(tar tf $tar_file | grep -m 1 '^sysupgrade-.*/$')
	board_dir=${board_dir%/}

	tar Oxf $tar_file ${board_dir}/kernel | mtd write - kernel

	if [ -n "$UPGRADE_BACKUP" ]; then
		tar Oxf $tar_file ${board_dir}/root | mtd -j "$UPGRADE_BACKUP" write - rootfs
	else
		tar Oxf $tar_file ${board_dir}/root | mtd write - rootfs
	fi
}

platform_do_upgrade_mikrotik_nand() {
	local fw_mtd=$(find_mtd_part kernel)
	fw_mtd="${fw_mtd/block/}"
	[ -n "$fw_mtd" ] || return

	local board_dir=$(tar tf "$1" | grep -m 1 '^sysupgrade-.*/$')
	board_dir=${board_dir%/}
	[ -n "$board_dir" ] || return

	local kernel_len=$(tar xf "$1" ${board_dir}/kernel -O | wc -c)
	[ -n "$kernel_len" ] || return

	tar xf "$1" ${board_dir}/kernel -O | ubiformat "$fw_mtd" -y -S $kernel_len -f -

	CI_KERNPART="none"
	nand_do_upgrade "$1"
}

platform_do_upgrade() {
	case "$(board_name)" in
	8dev,jalapeno |\
	aruba,ap-303 |\
	aruba,ap-303h |\
	aruba,ap-365 |\
	avm,fritzbox-7530 |\
	avm,fritzrepeater-1200 |\
	avm,fritzrepeater-3000 |\
	buffalo,wtr-m2133hp |\
	cilab,meshpoint-one |\
	edgecore,ecw5211 |\
	edgecore,oap100 |\
	engenius,eap2200 |\
	glinet,gl-ap1300 |\
	luma,wrtq-329acn |\
	mobipromo,cm520-79f |\
	netgear,wac510 |\
	p2w,r619ac-64m |\
	p2w,r619ac-128m |\
	qxwlan,e2600ac-c2)
		nand_do_upgrade "$1"
		;;
	glinet,gl-b2200)
		CI_KERNPART="0:HLOS"
		CI_ROOTPART="rootfs"
		CI_DATAPART="rootfs_data"
		emmc_do_upgrade "$1"
		;;
	alfa-network,ap120c-ac)
		part="$(awk -F 'ubi.mtd=' '{printf $2}' /proc/cmdline | sed -e 's/ .*$//')"
		if [ "$part" = "rootfs1" ]; then
			fw_setenv active 2 || exit 1
			CI_UBIPART="rootfs2"
		else
			fw_setenv active 1 || exit 1
			CI_UBIPART="rootfs1"
		fi
		nand_do_upgrade "$1"
		;;
	asus,map-ac2200)
		CI_KERNPART="linux"
		nand_do_upgrade "$1"
		;;
	asus,rt-ac42u |\
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
	cellc,rtl30vw)
		CI_UBIPART="ubifs"
		askey_do_upgrade "$1"
		;;
	compex,wpj419)
		nand_do_upgrade "$1"
		;;
	linksys,ea6350v3 |\
	linksys,ea8300 |\
	linksys,mr8300)
		platform_do_upgrade_linksys "$1"
		;;
	meraki,mr33)
		CI_KERNPART="part.safe"
		nand_do_upgrade "$1"
		;;
	mikrotik,cap-ac|\
	mikrotik,hap-ac2|\
	mikrotik,lhgg-60ad|\
	mikrotik,sxtsq-5-ac)
		[ "$(rootfs_type)" = "tmpfs" ] && mtd erase firmware
		default_do_upgrade "$1"
		;;
	mikrotik,hap-ac3)
		platform_do_upgrade_mikrotik_nand "$1"
		;;
	netgear,rbr50 |\
	netgear,rbs50 |\
	netgear,srr60 |\
	netgear,srs60)
		platform_do_upgrade_netgear_orbi_upgrade "$1"
		;;
	openmesh,a42 |\
	openmesh,a62 |\
	plasmacloud,pa1200 |\
	plasmacloud,pa2200)
		PART_NAME="inactive"
		platform_do_upgrade_dualboot_datachk "$1"
		;;
	teltonika,rutx10 |\
	zte,mf286d |\
	zte,mf289f)
		CI_UBIPART="rootfs"
		nand_do_upgrade "$1"
		;;
	zyxel,nbg6617)
		zyxel_do_upgrade "$1"
		;;
	*)
		default_do_upgrade "$1"
		;;
	esac
}

platform_copy_config() {
	case "$(board_name)" in
	glinet,gl-b2200)
		emmc_copy_config
		;;
	esac
	return 0;
}
