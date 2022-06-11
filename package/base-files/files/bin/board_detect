#!/bin/sh

CFG=$1

[ -n "$CFG" ] || CFG=/etc/board.json

[ -d "/etc/board.d/" -a ! -s "$CFG" ] && {
	for a in $(ls /etc/board.d/*); do
		[ -s $a ] || continue;
		$(. $a)
	done
}

[ -s "$CFG" ] || return 1
