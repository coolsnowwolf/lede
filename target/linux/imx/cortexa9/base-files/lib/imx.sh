#
# Copyright (C) 2010-2013 OpenWrt.org
#

rootpartuuid() {
	local cmdline=$(cat /proc/cmdline)
	local bootpart=${cmdline##*root=}
	bootpart=${bootpart%% *}
	local uuid=${bootpart#PARTUUID=}
	echo ${uuid%-02}
}

bootdev_from_uuid() {
	blkid | grep "PTUUID=\"$(rootpartuuid)\"" | cut -d : -f1
}

bootpart_from_uuid() {
	blkid | grep $(rootpartuuid)-01 | cut -d : -f1
}

rootpart_from_uuid() {
	blkid | grep $(rootpartuuid)-02 | cut -d : -f1
}

apalis_mount_boot() {
	mkdir -p /boot
	[ -f /boot/uImage ] || {
		mount -o rw,noatime $(bootpart_from_uuid) /boot > /dev/null
	}
}
