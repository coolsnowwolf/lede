#!/bin/sh

. /lib/functions.sh

include /lib/upgrade

v "Performing system upgrade..."
if type 'platform_do_upgrade' >/dev/null 2>/dev/null; then
	platform_do_upgrade "$IMAGE"
else
	default_do_upgrade "$IMAGE"
fi

if [ -n "$UPGRADE_BACKUP" ] && type 'platform_copy_config' >/dev/null 2>/dev/null; then
	platform_copy_config
fi

v "Upgrade completed"
sleep 1

v "Rebooting system..."
umount -a
reboot -f
sleep 5
echo b 2>/dev/null >/proc/sysrq-trigger
