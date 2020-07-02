#!/bin/sh

[ "${ACTION}" = "released" ] || exit 0

if [ "$SEEN" -ge 5 ]
then
	echo "REBOOT" > /dev/console
	sync
	reboot
fi

return 0
