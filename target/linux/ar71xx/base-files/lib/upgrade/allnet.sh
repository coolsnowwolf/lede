# The U-Boot loader of the some Allnet devices requires image sizes and
# checksums to be provided in the U-Boot environment.
# In case the check fails during boot, a failsafe-system is started to provide
# a minimal web-interface for flashing a new firmware.

# make sure we got uboot-envtools and fw_env.config copied over to the ramfs
# create /var/lock for the lock "fw_setenv.lock" of fw_setenv
platform_add_ramfs_ubootenv() {
	[ -e /usr/sbin/fw_printenv ] && install_bin /usr/sbin/fw_printenv /usr/sbin/fw_setenv
	[ -e /etc/fw_env.config ] && install_file /etc/fw_env.config
	mkdir -p $RAM_ROOT/var/lock
}
append sysupgrade_pre_upgrade platform_add_ramfs_ubootenv

# determine size of the main firmware partition
platform_get_firmware_size() {
	local dev size erasesize name
	while read dev size erasesize name; do
		name=${name#'"'}; name=${name%'"'}
		case "$name" in
			firmware)
				printf "%d" "0x$size"
				break
			;;
		esac
	done < /proc/mtd
}

# get the first 4 bytes (magic) of a given file starting at offset in hex format
get_magic_long_at() {
	dd if="$1" skip=$(( $CI_BLKSZ / 4 * $2 )) bs=4 count=1 2>/dev/null | hexdump -v -n 4 -e '1/1 "%02x"'
}

get_filesize() {
	wc -c "$1" | while read image_size _n ; do echo $image_size ; break; done
}

# scan through the update image pages until matching a magic
platform_get_offset() {
	offsetcount=0
	magiclong="x"
	if [ -n "$3" ]; then
		offsetcount=$3
	fi
	while magiclong=$( get_magic_long_at "$1" "$offsetcount" ) && [ -n "$magiclong" ]; do
		case "$magiclong" in
			"2705"*)
				# U-Boot image magic
				if [ "$2" = "uImage" ]; then
					echo $offsetcount
					return
				fi
			;;
			"68737173"|"73717368")
				# SquashFS
				if [ "$2" = "rootfs" ]; then
					echo $offsetcount
					return
				fi
			;;
			"deadc0de"|"19852003")
				# JFFS2 empty page
				if [ "$2" = "rootfs-data" ]; then
					echo $offsetcount
					return
				fi
			;;
		esac
		offsetcount=$(( $offsetcount + 1 ))
	done
}

platform_check_image_allnet() {
	local fw_printenv=/usr/sbin/fw_printenv
	[ ! -n "$fw_printenv" -o ! -x "$fw_printenv" ] && {
		echo "Please install uboot-envtools!"
		return 1
	}

	[ ! -r "/etc/fw_env.config" ] && {
		echo "/etc/fw_env.config is missing"
		return 1
	}

	local image_size=$( get_filesize "$1" )
	local firmware_size=$( platform_get_firmware_size )
	[ $image_size -ge $firmware_size ] &&
	{
		echo "upgrade image is too big (${image_size}b > ${firmware_size}b)"
	}

	local vmlinux_blockoffset=$( platform_get_offset "$1" uImage )
	[ -z $vmlinux_blockoffset ] && {
		echo "vmlinux-uImage not found"
		return 1
	}

	local rootfs_blockoffset=$( platform_get_offset "$1" rootfs "$vmlinux_blockoffset" )
	[ -z $rootfs_blockoffset ] && {
		echo "missing rootfs"
		return 1
	}

	local data_blockoffset=$( platform_get_offset "$1" rootfs-data "$rootfs_blockoffset" )
	[ -z $data_blockoffset ] && {
		echo "rootfs doesn't have JFFS2 end marker"
		return 1
	}

	return 0
}

platform_do_upgrade_allnet() {
	local firmware_base_addr=$( printf "%d" "$1" )
	local vmlinux_blockoffset=$( platform_get_offset "$2" uImage )
	if [ ! -n "$vmlinux_blockoffset" ]; then
		echo "can't determine uImage offset"
		return 1
	fi
	local rootfs_blockoffset=$( platform_get_offset "$2" rootfs $(( $vmlinux_blockoffset + 1 )) )
	local vmlinux_offset=$(( $vmlinux_blockoffset * $CI_BLKSZ ))
	local vmlinux_addr=$(( $firmware_base_addr + $vmlinux_offset ))
	local vmlinux_hexaddr=0x$( printf "%08x" "$vmlinux_addr" )
	if [ ! -n "$rootfs_blockoffset" ]; then
		echo "can't determine rootfs offset"
		return 1
	fi
	local rootfs_offset=$(( $rootfs_blockoffset * $CI_BLKSZ ))
	local rootfs_addr=$(( $firmware_base_addr + $rootfs_offset ))
	local rootfs_hexaddr=0x$( printf "%08x" "$rootfs_addr" )
	local vmlinux_blockcount=$(( $rootfs_blockoffset - $vmlinux_blockoffset ))
	local vmlinux_size=$(( $rootfs_offset - $vmlinux_offset ))
	local vmlinux_hexsize=0x$( printf "%08x" "$vmlinux_size" )
	local data_blockoffset=$( platform_get_offset "$2" rootfs-data $(( $rootfs_blockoffset + 1 )) )
	if [ ! -n "$data_blockoffset" ]; then
		echo "can't determine rootfs size"
		return 1
	fi
	local data_offset=$(( $data_blockoffset * $CI_BLKSZ ))
	local rootfs_blockcount=$(( $data_blockoffset - $rootfs_blockoffset ))
	local rootfs_size=$(( $data_offset - $rootfs_offset ))
	local rootfs_hexsize=0x$( printf "%08x" "$rootfs_size" )

	local rootfs_md5=$( dd if="$2" bs=$CI_BLKSZ skip=$rootfs_blockoffset count=$rootfs_blockcount 2>/dev/null | md5sum -); rootfs_md5="${rootfs_md5%% *}"
	local vmlinux_md5=$( dd if="$2" bs=$CI_BLKSZ skip=$vmlinux_blockoffset count=$vmlinux_blockcount 2>/dev/null | md5sum -); vmlinux_md5="${vmlinux_md5%% *}"
	# this needs a recent version of uboot-envtools!
	cat >/tmp/fw_env_upgrade <<EOF
vmlinux_start_addr $vmlinux_hexaddr
vmlinux_size $vmlinux_hexsize
vmlinux_checksum $vmlinux_md5
rootfs_start_addr $rootfs_hexaddr
rootfs_size $rootfs_hexsize
rootfs_checksum $rootfs_md5
bootcmd bootm $vmlinux_hexaddr
EOF
	fw_setenv -s /tmp/fw_env_upgrade || {
		echo "failed to update U-Boot environment"
		return 1
	}
	shift
	default_do_upgrade "$@"
}
