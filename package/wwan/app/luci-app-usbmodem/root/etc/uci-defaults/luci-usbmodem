#!/bin/sh

uci -q batch <<-EOF >/dev/null
	delete ucitrack.@usbmodem[-1]
	add ucitrack usbmodem
	set ucitrack.@usbmodem[-1].init=usbmodem
	commit ucitrack
EOF

rm -f /tmp/luci-indexcache
exit 0
