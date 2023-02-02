#!/bin/sh

uci -q batch <<-EOF >/dev/null
	delete ucitrack.@gobinetmodem[-1]
	add ucitrack gobinetmodem
	set ucitrack.@gobinetmodem[-1].init=gobinetmodem
	commit ucitrack
EOF

rm -f /tmp/luci-indexcache
exit 0
