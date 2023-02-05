#!/bin/sh

uci -q batch <<-EOF >/dev/null
	delete ucitrack.@pcimodem[-1]
	add ucitrack pcimodem
	set ucitrack.@pcimodem[-1].init=pcimodem
	commit ucitrack
EOF

rm -f /tmp/luci-indexcache
exit 0
