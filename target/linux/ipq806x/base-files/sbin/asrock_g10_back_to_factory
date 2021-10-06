#!/bin/sh

. /lib/upgrade/asrock.sh

case $(board_name) in
asrock,g10)
	asrock_bootconfig_mangle "factory"
	if [ $? -eq 0 ]; then
		reboot
	fi
	;;
*)
	echo "Unsupported hardware."
	;;
esac
