#!/bin/sh

logfile_path() (
	configfile=$(uci -q get mosdns.config.configfile)
	if [ "$configfile" = "/etc/mosdns/config.yaml" ]; then
		uci -q get mosdns.config.logfile
	else
		[ ! -f /etc/mosdns/config_custom.yaml ] && exit 1
		cat /etc/mosdns/config_custom.yaml | grep -A 4 log | grep file | awk -F ":" '{print $2}' | sed 's/\"//g;s/ //g'
	fi
)

interface_dns() (
	peerdns=$(uci -q get network.wan.peerdns)
	proto=$(uci -q get network.wan.proto)
	if [ "$peerdns" = 0 ] || [ "$proto" = "static" ]; then
		uci -q get network.wan.dns
	else
		interface_status=$(ubus call network.interface.wan status)
		echo $interface_status | jsonfilter -e "@['dns-server'][0]"
		echo $interface_status | jsonfilter -e "@['dns-server'][1]"
	fi
	[ $? -ne 0 ] && echo "119.29.29.29"
)

ad_block() (
	adblock=$(uci -q get mosdns.config.adblock)
	if [ "$adblock" -eq 1 ]; then
		ad_source=$(uci -q get mosdns.config.ad_source)
		if [ "$ad_source" = "geosite.dat" ]; then
			echo "provider:geosite:category-ads-all"
		else
			echo "provider:adlist"
		fi
	else
		echo "full:disable-category-ads-all.null"
	fi
)

adlist_update() (
	ad_source=$(uci -q get mosdns.config.ad_source)
	[ "$ad_source" = "geosite.dat" ] && exit 0
	AD_TMPDIR=$(mktemp -d) || exit 1
	if [[ "$ad_source" =~ "^https://raw.githubusercontent.com" ]]; then
		google_status=$(curl -I -4 -m 3 -o /dev/null -s -w %{http_code} http://www.google.com/generate_204)
		[ "$google_status" -ne "204" ] && mirror="https://ghproxy.com/"
	fi
	echo -e "\e[1;32mDownloading $mirror$ad_source\e[0m"
	curl --connect-timeout 60 -m 90 --ipv4 -fSLo "$AD_TMPDIR/adlist.txt" "$mirror$ad_source"
	if [ $? -ne 0 ]; then
		rm -rf "$AD_TMPDIR"
		exit 1
	else
		\cp "$AD_TMPDIR/adlist.txt" /etc/mosdns/rule/adlist.txt
		rm -rf "$AD_TMPDIR"
	fi
)

geodat_update() (
	geodat_download() (
		google_status=$(curl -I -4 -m 3 -o /dev/null -s -w %{http_code} http://www.google.com/generate_204)
		[ "$google_status" -ne "204" ] && mirror="https://ghproxy.com/"
		echo -e "\e[1;32mDownloading "$mirror"https://github.com/Loyalsoldier/v2ray-rules-dat/releases/latest/download/$1\e[0m"
		curl --connect-timeout 60 -m 900 --ipv4 -fSLo "$TMPDIR/$1" ""$mirror"https://github.com/Loyalsoldier/v2ray-rules-dat/releases/latest/download/$1"
	)
	TMPDIR=$(mktemp -d) || exit 1
	geodat_download geoip.dat && geodat_download geosite.dat
	if [ $? -ne 0 ]; then
		rm -rf "$TMPDIR"
		exit 1
	fi
	cp -f "$TMPDIR"/* /usr/share/v2ray
	rm -rf "$TMPDIR"
)

if [ "$1" == "dns" ]; then
	interface_dns
elif [ "$1" == "ad" ]; then
	ad_block
elif [ "$1" == "geodata" ]; then
	geodat_update && adlist_update
elif [ "$1" == "logfile" ]; then
	logfile_path
elif [ "$1" == "adlist_update" ]; then
	adlist_update
fi
