#!/bin/sh
# Copyright (C) 2006-2016 OpenWrt.org
# Copyright (C) 2010 Vertical Communications

[ -z "$PREINIT" ] && exec /sbin/init

export PATH="%PATH%"

. /lib/functions.sh
. /lib/functions/preinit.sh
. /lib/functions/system.sh

boot_hook_init preinit_essential
boot_hook_init preinit_main
boot_hook_init failsafe
boot_hook_init initramfs
boot_hook_init preinit_mount_root

for pi_source_file in /lib/preinit/*; do
	. $pi_source_file
done

boot_run_hook preinit_essential

pi_mount_skip_next=false
pi_jffs2_mount_success=false
pi_failsafe_net_message=false

boot_run_hook preinit_main
