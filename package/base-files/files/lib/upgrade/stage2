#!/bin/sh

. /lib/functions.sh
. /lib/functions/system.sh

export IMAGE="$1"
COMMAND="$2"

export INTERACTIVE=0
export VERBOSE=1
export CONFFILES=/tmp/sysupgrade.conffiles

RAMFS_COPY_BIN=		# extra programs for temporary ramfs root
RAMFS_COPY_DATA=	# extra data files

include /lib/upgrade


supivot() { # <new_root> <old_root>
	/bin/mount | grep "on $1 type" 2>&- 1>&- || /bin/mount -o bind $1 $1
	mkdir -p $1$2 $1/proc $1/sys $1/dev $1/tmp $1/overlay && \
	/bin/mount -o noatime,move /proc $1/proc && \
	pivot_root $1 $1$2 || {
		/bin/umount -l $1 $1
		return 1
	}

	/bin/mount -o noatime,move $2/sys /sys
	/bin/mount -o noatime,move $2/dev /dev
	/bin/mount -o noatime,move $2/tmp /tmp
	/bin/mount -o noatime,move $2/overlay /overlay 2>&-
	return 0
}

switch_to_ramfs() {
	RAMFS_COPY_LOSETUP="$(command -v /usr/sbin/losetup)"
	RAMFS_COPY_LVM="$(command -v lvm)"

	for binary in \
		/bin/busybox /bin/ash /bin/sh /bin/mount /bin/umount	\
		pivot_root mount_root reboot sync kill sleep		\
		md5sum hexdump cat zcat dd tar gzip			\
		ls basename find cp mv rm mkdir rmdir mknod touch chmod \
		'[' printf wc grep awk sed cut sort tail		\
		mtd partx losetup mkfs.ext4 nandwrite flash_erase	\
		ubiupdatevol ubiattach ubiblock ubiformat		\
		ubidetach ubirsvol ubirmvol ubimkvol			\
		snapshot snapshot_tool date logger			\
		/usr/sbin/fw_printenv /usr/bin/fwtool			\
		$RAMFS_COPY_LOSETUP $RAMFS_COPY_LVM			\
		$RAMFS_COPY_BIN
	do
		local file="$(command -v "$binary" 2>/dev/null)"
		[ -n "$file" ] && install_bin "$file"
	done
	install_file /etc/resolv.conf /lib/*.sh /lib/functions/*.sh	\
		/lib/upgrade/*.sh /lib/upgrade/do_stage2 		\
		/usr/share/libubox/jshn.sh /usr/sbin/fw_setenv		\
		/etc/fw_env.config $RAMFS_COPY_DATA

	mkdir -p $RAM_ROOT/var/lock

	[ -L "/lib64" ] && ln -s /lib $RAM_ROOT/lib64

	supivot $RAM_ROOT /mnt || {
		v "Failed to switch over to ramfs. Please reboot."
		exit 1
	}

	/bin/mount -o remount,ro /mnt
	/bin/umount -l /mnt

	grep -e "^/dev/dm-.*" -e "^/dev/loop.*" /proc/mounts | while read bdev mp _r; do
		umount $mp
	done

	[ "$RAMFS_COPY_LOSETUP" ] && losetup -D
	[ "$RAMFS_COPY_LVM" ] && {
		mkdir -p /tmp/lvm/cache
		$RAMFS_COPY_LVM vgchange -aln --ignorelockingfailure
	}

	grep /overlay /proc/mounts > /dev/null && {
		/bin/mount -o noatime,remount,ro /overlay
		/bin/umount -l /overlay
	}
}

kill_remaining() { # [ <signal> [ <loop> ] ]
	local loop_limit=10

	local sig="${1:-TERM}"
	local loop="${2:-0}"
	local run=true
	local stat
	local proc_ppid=$(cut -d' ' -f4  /proc/$$/stat)

	v "Sending $sig to remaining processes ..."

	while $run; do
		run=false
		for stat in /proc/[0-9]*/stat; do
			[ -f "$stat" ] || continue

			local pid name state ppid rest
			read pid rest < $stat
			name="${rest#\(}" ; rest="${name##*\) }" ; name="${name%\)*}"
			set -- $rest ; state="$1" ; ppid="$2"

			# Skip PID1, our parent, ourself and our children
			[ $pid -ne 1 -a $pid -ne $proc_ppid -a $pid -ne $$ -a $ppid -ne $$ ] || continue

			[ -f "/proc/$pid/cmdline" ] || continue

			local cmdline
			read cmdline < /proc/$pid/cmdline

			# Skip kernel threads
			[ -n "$cmdline" ] || continue

			v "Sending signal $sig to $name ($pid)"
			kill -$sig $pid 2>/dev/null

			[ $loop -eq 1 ] && sleep 2 && run=true
		done

		let loop_limit--
		[ $loop_limit -eq 0 ] && {
			v "Failed to kill all processes."
			exit 1
		}
	done
}

indicate_upgrade

while read -r a b c; do
	case "$a" in
		MemT*) mem="$b" ;; esac
done < /proc/meminfo

[ "$mem" -gt 32768 ] && \
	skip_services="dnsmasq log network"
for service in /etc/init.d/*; do
	service=${service##*/}

	case " $skip_services " in
		*" $service "*) continue ;; esac

	ubus call service delete '{ "name": "'"$service"'" }' 2>/dev/null
done

killall -9 telnetd 2>/dev/null
killall -9 dropbear 2>/dev/null
killall -9 ash 2>/dev/null

kill_remaining TERM
sleep 4
kill_remaining KILL 1

sleep 6

echo 3 > /proc/sys/vm/drop_caches

if [ -n "$IMAGE" ] && type 'platform_pre_upgrade' >/dev/null 2>/dev/null; then
	platform_pre_upgrade "$IMAGE"
fi

if [ -n "$(rootfs_type)" ]; then
	v "Switching to ramdisk..."
	switch_to_ramfs
fi

# Exec new shell from ramfs
exec /bin/busybox ash -c "$COMMAND"
