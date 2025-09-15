#!/bin/sh

uci -q batch <<-EOT
	delete firewall.mtkiappd
	set firewall.mtkiappd=include
	set firewall.mtkiappd.type=restore
	set firewall.mtkiappd.path=/usr/share/mtkiappd/firewall.include
	set firewall.mtkiappd.reload=1
	commit firewall
EOT

exit 0
