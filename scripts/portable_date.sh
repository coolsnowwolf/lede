#!/bin/sh

case $(uname) in
	NetBSD|OpenBSD|DragonFly|FreeBSD|Darwin)
		date -j -f "%Y-%m-%d %H:%M:%S %z" "$1" "$2" 2>/dev/null
		;;
	*)
		date -d "$1" "$2"
esac

exit $?
