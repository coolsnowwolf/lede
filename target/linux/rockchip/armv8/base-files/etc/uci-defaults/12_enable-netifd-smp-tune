#!/bin/sh
uci -q batch <<-EOF >/dev/null
	set network.globals.packet_steering=1
	commit network
EOF

exit 0
