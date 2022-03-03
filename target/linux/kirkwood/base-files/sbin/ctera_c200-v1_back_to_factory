#!/bin/sh

. /lib/functions.sh

case $(board_name) in
ctera,c200-v1)
	part=$(find_mtd_part "active_bank")

	if [ -n "$part" ]; then
		active_bank="$(strings $part | grep bank)"

		if [ $active_bank = "bank1" ]; then
			echo "bank2" > /tmp/change_bank
		else
			echo "bank1" > /tmp/change_bank
		fi

		mtd write /tmp/change_bank active_bank
		reboot
	else
		echo "active_bank partition missed!"
		return 1
	fi
	;;
*)
	echo "Unsupported hardware."
	;;
esac
