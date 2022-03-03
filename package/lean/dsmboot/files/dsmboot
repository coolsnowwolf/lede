#!/bin/sh

uci -q batch <<-EOF >/dev/null
	set dhcp.@dnsmasq[0].enable_tftp='1'
	set dhcp.@dnsmasq[0].dhcp_boot='pxelinux.0'
	set dhcp.@dnsmasq[0].tftp_root='/root'
	commit dhcp
EOF

exit 0