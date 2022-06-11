#!/bin/sh /etc/rc.common

START=97
boot() {
	. /lib/functions.sh

	case $(board_name) in
		linksys,panamera)
			# clear partialboots
			nvram set partialboots=0 && nvram commit
			;;
	esac
}
