#!/bin/sh /etc/rc.common
# Copyright (C) 2017 Rodolfo Giometti <giometti@enneenne.com>
#
# Based on Debian's script /etc/init.d/sysfsutils by
# Martin Pitt <mpitt@debian.org>

load_conffile() {
	FILE="$1"
	sed  's/#.*$//; /^[[:space:]]*$/d;
	  s/^[[:space:]]*\([^=[:space:]]*\)[[:space:]]*\([^=[:space:]]*\)[[:space:]]*=[[:space:]]*\(.*\)/\1 \2 \3/' \
	  $FILE | {
	while read f1 f2 f3; do
		if [ "$f1" = "mode" -a -n "$f2" -a -n "$f3" ]; then
			if [ -f "/sys/$f2" ] || [ -d "/sys/$f2" ]; then
				chmod "$f3" "/sys/$f2"
			else
				echo "unknown attribute $f2"
			fi
		elif [ "$f1" = "owner" -a -n "$f2" -a -n "$f3" ]; then
			if [ -f "/sys/$f2" ]; then
				chown "$f3" "/sys/$f2"
			else
				echo "unknown attribute $f2"
			fi
		elif [ "$f1" -a -n "$f2" -a -z "$f3" ]; then
			if [ -f "/sys/$f1" ]; then
				# Some fields need a terminating newline, others
				# need the terminating newline to be absent :-(
				echo -n "$f2" > "/sys/$f1" 2>/dev/null ||
					echo "$f2" > "/sys/$f1"
			else
				echo "unknown attribute $f1"
			fi
		else
			echo "syntax error in $CONFFILE: '$f1' '$f2' '$f3'"
			exit 1
		fi
	done
	}
}

START=11
start() {
	for file in /etc/sysfs.conf /etc/sysfs.d/*.conf; do
		[ -r "$file" ] || continue
		load_conffile "$file"
	done
}
