PART_NAME=firmware
REQUIRE_IMAGE_METADATA=1

RAMFS_COPY_BIN='fw_printenv fw_setenv'
RAMFS_COPY_DATA='/etc/fw_env.config /var/lock/fw_printenv.lock'

platform_check_image() {
	case "$(board_name)" in
	asus,rt-ac42u |\
	asus,rt-ac58u)
		local ubidev=$(nand_find_ubi $CI_UBIPART)
		local asus_root=$(nand_find_volume $ubidev jffs2)

		[ -n "$asus_root" ] || return 0

		cat << EOF
jffs2 partition is still present.
There's probably no space left
to install the filesystem.

You need to delete the jffs2 partition first:
# ubirmvol /dev/ubi0 --name=jffs2

Once this is done. Retry.
EOF
		return 1
		;;
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
		CI_KERNPART="linux"
		nand_do_upgrade "$1"
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
