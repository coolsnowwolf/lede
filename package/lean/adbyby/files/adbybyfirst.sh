#!/bin/sh
PROG_PATH=/usr/share/adbyby
if [ -z "$(dnsmasq --version | grep no-ipset)" ]; then
	[ $(uci get adbyby.@adbyby[-1].wan_mode) -ne 2 ] && \
	[ $(awk -F= '/^ipset/{print $2}' $PROG_PATH/adhook.ini) -eq 1 ] && \
	{
		sed -i 's/adbyby_list/adbyby_wan/' /tmp/adbyby_host.conf
		echo conf-file=/tmp/adbyby_host.conf >> /etc/dnsmasq.conf
		/etc/init.d/dnsmasq restart
	}
fi
