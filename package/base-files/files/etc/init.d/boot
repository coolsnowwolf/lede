#!/bin/sh /etc/rc.common
# Copyright (C) 2006-2011 OpenWrt.org

START=10
STOP=90

uci_apply_defaults() {
	. /lib/functions/system.sh

	cd /etc/uci-defaults || return 0
	files="$(ls)"
	[ -z "$files" ] && return 0
	mkdir -p /tmp/.uci
	for file in $files; do
		( . "./$(basename $file)" ) && rm -f "$file"
	done
	uci commit
}

boot() {
	[ -f /proc/mounts ] || /sbin/mount_root
	[ -f /proc/jffs2_bbc ] && echo "S" > /proc/jffs2_bbc

	mkdir -p /var/lock
	chmod 1777 /var/lock
	mkdir -p /var/log
	mkdir -p /var/run
	mkdir -p /var/state
	mkdir -p /var/tmp
	mkdir -p /tmp/.uci
	chmod 0700 /tmp/.uci
	touch /var/log/wtmp
	touch /var/log/lastlog
	mkdir -p /tmp/resolv.conf.d
	touch /tmp/resolv.conf.d/resolv.conf.auto
	ln -sf /tmp/resolv.conf.d/resolv.conf.auto /tmp/resolv.conf
	grep -q debugfs /proc/filesystems && /bin/mount -o noatime -t debugfs debugfs /sys/kernel/debug
	grep -q bpf /proc/filesystems && /bin/mount -o nosuid,nodev,noexec,noatime,mode=0700 -t bpf bpffs /sys/fs/bpf
	grep -q pstore /proc/filesystems && /bin/mount -o noatime -t pstore pstore /sys/fs/pstore
	[ "$FAILSAFE" = "true" ] && touch /tmp/.failsafe

	/sbin/kmodloader

	[ ! -f /etc/config/wireless ] && {
		# compat for bcm47xx and mvebu
		sleep 1
	}

	/bin/config_generate
	uci_apply_defaults
	sync
	
	# temporary hack until configd exists
	/sbin/reload_config
}
