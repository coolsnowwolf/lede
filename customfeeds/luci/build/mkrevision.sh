#!/bin/sh

TOPDIR="${0%mkrevision.sh}"

[ -d "$TOPDIR/../build" ] || {
	echo "Please execute as ./build/mkrevision.sh" >&2
	exit 1
}

(
	cd "$TOPDIR"
	if svn info >/dev/null 2>/dev/null; then
		revision="svn-r$(LC_ALL=C svn info | sed -ne 's/^Revision: //p')"
	elif git log -1 >/dev/null 2>/dev/null; then
		revision="svn-r$(LC_ALL=C git log -1 | sed -ne 's/.*git-svn-id: .*@\([0-9]\+\) .*/\1/p')"
		if [ "$revision" = "svn-r" ]; then
			set -- $(git log -1 --format="%ct %h")
			secs="$(($1 % 86400))"
			yday="$(date --utc --date="@$1" "+%y.%j")"
			revision="$(printf 'git-%s.%05d-%s' "$yday" "$secs" "$2")"
		fi
	else
		revision="unknown"
	fi

	echo "$revision"
)
