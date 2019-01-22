#!/bin/sh

/etc/dnsforwarder/gen-gfwlist.sh > /tmp/ol-gfw.txt

if [ -s "/tmp/ol-gfw.txt" ];then
	sort -u /etc/dnsforwarder/base-gfwlist.txt /tmp/ol-gfw.txt > /tmp/china-banned
	if ( ! cmp -s /tmp/china-banned /etc/dnsforwarder/china-banned );then
		if [ -s "/tmp/china-banned" ];then
			mv /tmp/china-banned /etc/dnsforwarder/china-banned
			echo "Update GFW-List Done!"
		fi
	else
		echo "GFW-List No Change!"
	fi
fi

rm -f /tmp/gfwlist.txt
rm -f /tmp/ol-gfw.txt

