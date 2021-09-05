#!/bin/sh
[ -f "$1" ] && china_ip=$1
ipset -! flush china 2>/dev/null
ipset -! -R <<-EOF || exit 1
	create china hash:net
	$(cat ${china_ip:=/etc/ssrplus/china_ssr.txt} | sed -e "s/^/add china /")
EOF
