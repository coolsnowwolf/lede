#!/bin/sh
#
# Copyright (C) 2018 OpenWrt.org
# Copyright (C) 2018 rosysong@rosinson.com
#

. /lib/functions.sh

FLAG_DEV_TYPE=
FLAG_DEV_MOPT=
FLAG_HAS_SECT=

samba_dev_filter() { # <devname> <[path,/dev/]>
	case $1 in
	${2}mtdblock*|\
	${2}ubi*)
		FLAG_DEV_TYPE="mtd"
		;;
	${2}loop*|\
	${2}mmcblk*|\
	${2}sd*|\
	${2}hd*|\
	${2}md*|\
	${2}nvme*|\
	${2}vd*|\
	${2}xvd*)
		FLAG_DEV_TYPE="not-mtd"
		;;
	*)
		[ -b ${2}${1} ] && FLAG_DEV_TYPE="not-mtd"
		[ -b /dev/mapper/$1 ] && FLAG_DEV_TYPE="not-mtd"
		;;
	esac
}

samba_cfg_lookup() { # <section> <name>
	config_get name $1 name
	[ "$name" = "$2" ] || return
	FLAG_HAS_SECT=y
}

samba_cfg_delete() { # <section> <name>
	config_get name $1 name
	[ "$name" = "$2" ] || return
	uci -q delete samba.$1
}

samba_find_mount_point() { # <devname>
	# search mount point in /proc/mounts
	while read l; do
		local d=$(echo $l | awk '/^\/dev/ {print $1}')
		[ "$d" = "/dev/$1" ] || continue

		FLAG_DEV_MOPT=$(echo $l | awk '/^\/dev/ {print $2}')
		break
	done < /proc/mounts
}

_samba_add_section() { # <devname> <mount point>
	uci -q batch <<-EOF
		add samba sambashare
		set samba.@sambashare[-1].browseable='yes'
		set samba.@sambashare[-1].name='$1'
		set samba.@sambashare[-1].path='$2'
		set samba.@sambashare[-1].users='root'
		set samba.@sambashare[-1].read_only='no'
		set samba.@sambashare[-1].guest_ok='yes'
		set samba.@sambashare[-1].create_mask='0755'
		set samba.@sambashare[-1].dir_mask='0755'
	EOF
}

samba_add_section() { # <devname> [<mount point>]
	FLAG_HAS_SECT=
	FLAG_DEV_MOPT=

	config_foreach samba_cfg_lookup sambashare $1
	[ -z "$FLAG_HAS_SECT" ] || return

	samba_find_mount_point $1
	[ -n "$FLAG_DEV_MOPT" ] || return

	[ -n "$2" -a "$2" = "$FLAG_DEV_MOPT" ] || \
		_samba_add_section $1 $FLAG_DEV_MOPT
}

samba_delete_section() { # <devname>
	config_foreach samba_cfg_delete sambashare $1
}
