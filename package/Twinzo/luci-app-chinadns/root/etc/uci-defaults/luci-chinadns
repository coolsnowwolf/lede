#!/bin/sh

uci -q batch <<-EOF >/dev/null
	delete ucitrack.@chinadns[-1]
	add ucitrack chinadns
	set ucitrack.@chinadns[-1].init=chinadns
	commit ucitrack
EOF

rm -f /tmp/luci-indexcache
exit 0
