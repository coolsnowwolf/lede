#!/bin/sh
NAME=shadowsocksr
switch_server=$1
uci_get_by_type() {
	local ret=$(uci get $NAME.@$1[0].$2 2>/dev/null)
	echo ${ret:=$3}
}
if [ -z "$switch_server" ]; then
	GLOBAL_SERVER=$(uci_get_by_type global global_server nil)
else
	GLOBAL_SERVER=$switch_server
fi
NETFLIX_SERVER=$(uci_get_by_type global netflix_server nil)
[ "$NETFLIX_SERVER" == "same" ] && NETFLIX_SERVER=$GLOBAL_SERVER
if [ "$NETFLIX_SERVER" != "nil" ]; then
	netflix() {
		if [ -f "/tmp/dnsmasq.ssr/gfw_list.conf" ]; then
			for line in $(cat /etc/ssr/netflix.list); do sed -i "/$line/d" /tmp/dnsmasq.ssr/gfw_list.conf; done
			for line in $(cat /etc/ssr/netflix.list); do sed -i "/$line/d" /tmp/dnsmasq.ssr/gfw_base.conf; done
		fi
		awk '!/^$/&&!/^#/{printf("ipset=/%s/'"netflix"'\n",$0)}' /etc/ssr/netflix.list >/tmp/dnsmasq.ssr/netflix_forward.conf
		awk '!/^$/&&!/^#/{printf("server=/%s/'"127.0.0.1#$1"'\n",$0)}' /etc/ssr/netflix.list >>/tmp/dnsmasq.ssr/netflix_forward.conf
	}
	if [ "$NETFLIX_SERVER" != "$GLOBAL_SERVER" ]; then
		netflix 5555
	else
		netflix 5335
	fi
else
	rm -f /tmp/dnsmasq.ssr/netflix_forward.conf
fi
for line in $(cat /etc/ssr/black.list); do sed -i "/$line/d" /tmp/dnsmasq.ssr/gfw_list.conf; done
for line in $(cat /etc/ssr/black.list); do sed -i "/$line/d" /tmp/dnsmasq.ssr/gfw_base.conf; done
for line in $(cat /etc/ssr/white.list); do sed -i "/$line/d" /tmp/dnsmasq.ssr/gfw_list.conf; done
for line in $(cat /etc/ssr/white.list); do sed -i "/$line/d" /tmp/dnsmasq.ssr/gfw_base.conf; done
for line in $(cat /etc/ssr/deny.list); do sed -i "/$line/d" /tmp/dnsmasq.ssr/gfw_list.conf; done
for line in $(cat /etc/ssr/deny.list); do sed -i "/$line/d" /tmp/dnsmasq.ssr/gfw_base.conf; done
awk '!/^$/&&!/^#/{printf("ipset=/%s/'"blacklist"'\n",$0)}' /etc/ssr/black.list >/tmp/dnsmasq.ssr/blacklist_forward.conf
awk '!/^$/&&!/^#/{printf("server=/%s/'"127.0.0.1#5335"'\n",$0)}' /etc/ssr/black.list >>/tmp/dnsmasq.ssr/blacklist_forward.conf
awk '!/^$/&&!/^#/{printf("ipset=/%s/'"whitelist"'\n",$0)}' /etc/ssr/white.list >/tmp/dnsmasq.ssr/whitelist_forward.conf
awk '!/^$/&&!/^#/{printf("address=/%s/''\n",$0)}' /etc/ssr/deny.list >/tmp/dnsmasq.ssr/denylist.conf
if [ "$(uci_get_by_type global adblock 0)" == "1" ]; then
	[ "$1" == "" ] && cp -f /etc/ssr/ad.conf /tmp/dnsmasq.ssr/
	if [ -f "/tmp/dnsmasq.ssr/ad.conf" ]; then
		for line in $(cat /etc/ssr/black.list); do sed -i "/$line/d" /tmp/dnsmasq.ssr/ad.conf; done
		for line in $(cat /etc/ssr/white.list); do sed -i "/$line/d" /tmp/dnsmasq.ssr/ad.conf; done
		for line in $(cat /etc/ssr/deny.list); do sed -i "/$line/d" /tmp/dnsmasq.ssr/ad.conf; done
		for line in $(cat /etc/ssr/netflix.list); do sed -i "/$line/d" /tmp/dnsmasq.ssr/ad.conf; done
	fi
fi
