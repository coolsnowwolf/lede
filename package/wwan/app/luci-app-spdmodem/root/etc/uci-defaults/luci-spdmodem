#!/bin/sh

uci -q batch <<-EOF >/dev/null
	delete ucitrack.@spdmodem[-1]
	add ucitrack spdmodem
	set ucitrack.@spdmodem[-1].init=spdmodem
	commit ucitrack
EOF

rm -f /tmp/luci-indexcache
exit 0
