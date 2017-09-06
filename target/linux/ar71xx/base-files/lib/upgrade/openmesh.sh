# The U-Boot loader of the OpenMesh devices requires image sizes and
# checksums to be provided in the U-Boot environment.
# The OpenMesh devices come with 2 main partitions - while one is active
# sysupgrade will flash the other. The boot order is changed to boot the
# newly flashed partition. If the new partition can't be booted due to
# upgrade failures the previously used partition is loaded.

trim()
{
	echo $1
}

cfg_value_get()
{
	local cfg=$1 cfg_opt
	local section=$2 our_section=0
	local param=$3 our_param=

	for cfg_opt in $cfg
		do
			[ "$cfg_opt" = "[$section]" ] && our_section=1 && continue
			[ "$our_section" = "1" ] || continue

			our_param=$(echo ${cfg_opt%%=*})
			[ "$param" = "$our_param" ] && echo ${cfg_opt##*=} && break
		done
}

# make sure we got uboot-envtools and fw_env.config copied over to the ramfs
# create /var/lock for the lock "fw_setenv.lock" of fw_setenv
platform_add_ramfs_ubootenv()
{
	[ -e /usr/sbin/fw_printenv ] && install_bin /usr/sbin/fw_printenv /usr/sbin/fw_setenv
	[ -e /etc/fw_env.config ] && install_file /etc/fw_env.config
	mkdir -p $RAM_ROOT/var/lock
}
append sysupgrade_pre_upgrade platform_add_ramfs_ubootenv

platform_check_image_target_openmesh()
{
	img_board_target="$1"

	case "$img_board_target" in
		A60)
			[ "$board" = "a40" ] && return 0
			[ "$board" = "a60" ] && return 0
			echo "Invalid image board target ($img_board_target) for this platform: $board. Use the correct image for this platform"
			return 1
			;;
		OM2P)
			[ "$board" = "om2p" ] && return 0
			[ "$board" = "om2pv2" ] && return 0
			[ "$board" = "om2pv4" ] && return 0
			[ "$board" = "om2p-lc" ] && return 0
			[ "$board" = "om2p-hs" ] && return 0
			[ "$board" = "om2p-hsv2" ] && return 0
			[ "$board" = "om2p-hsv3" ] && return 0
			[ "$board" = "om2p-hsv4" ] && return 0
			echo "Invalid image board target ($img_board_target) for this platform: $board. Use the correct image for this platform"
			return 1
			;;
		OM5P)
			[ "$board" = "om5p" ] && return 0
			[ "$board" = "om5p-an" ] && return 0
			echo "Invalid image board target ($img_board_target) for this platform: $board. Use the correct image for this platform"
			return 1
			;;
		OM5PAC)
			[ "$board" = "om5p-ac" ] && return 0
			[ "$board" = "om5p-acv2" ] && return 0
			echo "Invalid image board target ($img_board_target) for this platform: $board. Use the correct image for this platform"
			return 1
			;;
		MR1750)
			[ "$board" = "mr1750" ] && return 0
			[ "$board" = "mr1750v2" ] && return 0
			echo "Invalid image board target ($img_board_target) for this platform: $board. Use the correct image for this platform"
			return 1
			;;
		MR600)
			[ "$board" = "mr600" ] && return 0
			[ "$board" = "mr600v2" ] && return 0
			echo "Invalid image board target ($img_board_target) for this platform: $board. Use the correct image for this platform"
			return 1
			;;
		MR900)
			[ "$board" = "mr900" ] && return 0
			[ "$board" = "mr900v2" ] && return 0
			echo "Invalid image board target ($img_board_target) for this platform: $board. Use the correct image for this platform"
			return 1
			;;
		*)
			echo "Invalid board target ($img_board_target). Use the correct image for this platform"
			return 1
			;;
	esac
}

platform_check_image_openmesh()
{
	local img_magic=$1
	local img_path=$2
	local fw_printenv=/usr/sbin/fw_printenv
	local img_board_target= img_num_files= i=0
	local cfg_name= kernel_name= rootfs_name=

	case "$img_magic" in
		# Combined Extended Image v1
		43453031)
			img_board_target=$(trim $(dd if="$img_path" bs=4 skip=1 count=8 2>/dev/null))
			img_num_files=$(trim $(dd if="$img_path" bs=2 skip=18 count=1 2>/dev/null))
			;;
		*)
			echo "Invalid image ($img_magic). Use combined extended images on this platform"
			return 1
			;;
	esac

	platform_check_image_target_openmesh "$img_board_target" || return 1

	[ $img_num_files -lt 3 ] && {
		echo "Invalid number of embedded images ($img_num_files). Use the correct image for this platform"
		return 1
	}

	cfg_name=$(trim $(dd if="$img_path" bs=2 skip=19 count=16 2>/dev/null))

	[ "$cfg_name" != "fwupgrade.cfg" ] && {
		echo "Invalid embedded config file ($cfg_name). Use the correct image for this platform"
		return 1
	}

	kernel_name=$(trim $(dd if="$img_path" bs=2 skip=55 count=16 2>/dev/null))

	[ "$kernel_name" != "kernel" ] && {
		echo "Invalid embedded kernel file ($kernel_name). Use the correct image for this platform"
		return 1
	}

	rootfs_name=$(trim $(dd if="$img_path" bs=2 skip=91 count=16 2>/dev/null))

	[ "$rootfs_name" != "rootfs" ] && {
		echo "Invalid embedded kernel file ($rootfs_name). Use the correct image for this platform"
		return 1
	}

	[ ! -x "$fw_printenv" ] && {
		echo "Please install uboot-envtools!"
		return 1
	}

	[ ! -r "/etc/fw_env.config" ] && {
		echo "/etc/fw_env.config is missing"
		return 1
	}

	return 0
}

platform_do_upgrade_openmesh()
{
	local img_path=$1 img_board_target=
	local kernel_start_addr= kernel_start_addr1= kernel_start_addr2=
	local kernel_size= kernel_md5=
	local rootfs_size= rootfs_checksize= rootfs_md5=
	local kernel_bsize= total_size=
	local data_offset=$((64 * 1024)) block_size= offset=
	local uboot_env_upgrade="/tmp/fw_env_upgrade"
	local cfg_size= kernel_size= rootfs_size=
	local append=""

	[ -f "$CONF_TAR" -a "$SAVE_CONFIG" -eq 1 ] && append="-j $CONF_TAR"

	cfg_size=$(dd if="$img_path" bs=2 skip=35 count=4 2>/dev/null)
	kernel_size=$(dd if="$img_path" bs=2 skip=71 count=4 2>/dev/null)
	rootfs_size=$(dd if="$img_path" bs=2 skip=107 count=4 2>/dev/null)

	img_board_target=$(trim $(dd if="$img_path" bs=4 skip=1 count=8 2>/dev/null))
	cfg_content=$(dd if="$img_path" bs=1 skip=$data_offset count=$(echo $((0x$cfg_size))) 2>/dev/null)

	case $img_board_target in
		OM2P)
			block_size=$((256 * 1024))
			total_size=7340032
			kernel_start_addr1=0x9f1c0000
			kernel_start_addr2=0x9f8c0000
			;;
		OM5P|OM5PAC|MR600|MR900|MR1750|A60)
			block_size=$((64 * 1024))
			total_size=7995392
			kernel_start_addr1=0x9f0b0000
			kernel_start_addr2=0x9f850000
			;;
	esac

	kernel_md5=$(cfg_value_get "$cfg_content" "vmlinux" "md5sum")
	rootfs_md5=$(cfg_value_get "$cfg_content" "rootfs" "md5sum")
	rootfs_checksize=$(cfg_value_get "$cfg_content" "rootfs" "checksize")

	if [ "$((0x$kernel_size % $block_size))" = "0" ]
		then
			kernel_bsize=$(echo $((0x$kernel_size)))
		else
			kernel_bsize=$((0x$kernel_size + ($block_size - (0x$kernel_size % $block_size))))
	fi

	mtd -q erase inactive

	offset=$(echo $(($data_offset + 0x$cfg_size + 0x$kernel_size)))
	dd if="$img_path" bs=1 skip=$offset count=$(echo $((0x$rootfs_size))) 2>&- | mtd -n -p $kernel_bsize $append write - "inactive"

	offset=$(echo $(($data_offset + 0x$cfg_size)))
	dd if="$img_path" bs=1 skip=$offset count=$(echo $((0x$kernel_size))) 2>&- | mtd -n write - "inactive"

	rm $uboot_env_upgrade 2>&-

	if [ "$(grep 'mtd3:.*inactive' /proc/mtd)" ]
		then
			printf "kernel_size_1 %u\n" $(($kernel_bsize / 1024)) >> $uboot_env_upgrade
			printf "rootfs_size_1 %u\n" $((($total_size - $kernel_bsize) / 1024)) >> $uboot_env_upgrade
			printf "bootseq 1,2\n" >> $uboot_env_upgrade
			kernel_start_addr=$kernel_start_addr1
		else
			printf "kernel_size_2 %u\n" $(($kernel_bsize / 1024)) >> $uboot_env_upgrade
			printf "rootfs_size_2 %u\n" $((($total_size - $kernel_bsize) / 1024)) >> $uboot_env_upgrade
			printf "bootseq 2,1\n" >> $uboot_env_upgrade
			kernel_start_addr=$kernel_start_addr2
	fi

	printf "vmlinux_start_addr %s\n" $kernel_start_addr >> $uboot_env_upgrade
	printf "vmlinux_size 0x%s\n" $kernel_size >> $uboot_env_upgrade
	printf "vmlinux_checksum %s\n" $kernel_md5 >> $uboot_env_upgrade
	printf "rootfs_start_addr 0x%x\n" $(($kernel_start_addr + $kernel_bsize)) >> $uboot_env_upgrade
	printf "rootfs_size %s\n" $rootfs_checksize >> $uboot_env_upgrade
	printf "rootfs_checksum %s\n" $rootfs_md5 >> $uboot_env_upgrade

	fw_setenv -s $uboot_env_upgrade || {
		echo "failed to update U-Boot environment"
		return 1
	}
}
