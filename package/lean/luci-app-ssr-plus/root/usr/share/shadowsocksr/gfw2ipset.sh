#!/bin/sh
NAME=shadowsocksr
uci_get_by_type() {
	local ret=$(uci get $NAME.@$1[0].$2 2>/dev/null)
	echo ${ret:=$3}
}
cp -a /etc/dnsmasq.ssr /tmp/
cp -a /etc/dnsmasq.oversea /tmp/
GLOBAL_SERVER=$(uci_get_by_type global global_server)
NETFLIX_SERVER=$(uci_get_by_type global netflix_server nil)
[ "$NETFLIX_SERVER" == "same" ] && NETFLIX_SERVER=$GLOBAL_SERVER
if [ "$NETFLIX_SERVER" != "nil" ]; then
	netflix() {
		for line in $(cat /etc/ssr/netflix.list); do sed -i "/$line/d" /tmp/dnsmasq.ssr/gfw_list.conf; done
		awk '!/^$/&&!/^#/{printf("ipset=/.%s/'"netflix"'\n",$0)}' /etc/ssr/netflix.list >/tmp/dnsmasq.ssr/netflix_forward.conf
		awk '!/^$/&&!/^#/{printf("server=/.%s/'"127.0.0.1#$1"'\n",$0)}' /etc/ssr/netflix.list >>/tmp/dnsmasq.ssr/netflix_forward.conf
	}
	if [ "$NETFLIX_SERVER" != "$GLOBAL_SERVER" ]; then
		netflix 5555
	else
		netflix 5335
	fi
else
	rm -f /tmp/dnsmasq.ssr/netflix_forward.conf
fi
if [ "$1" == "" ]; then
	awk '!/^$/&&!/^#/{printf("ipset=/.%s/'"blacklist"'\n",$0)}' /etc/ssr/black.list >/tmp/dnsmasq.ssr/blacklist_forward.conf
	awk '!/^$/&&!/^#/{printf("server=/.%s/'"127.0.0.1#5335"'\n",$0)}' /etc/ssr/black.list >>/tmp/dnsmasq.ssr/blacklist_forward.conf
	awk '!/^$/&&!/^#/{printf("ipset=/.%s/'"whitelist"'\n",$0)}' /etc/ssr/white.list >/tmp/dnsmasq.ssr/whitelist_forward.conf
	if [ "$(uci_get_by_type global adblock 0)" == "0" ]; then
		rm -f /tmp/dnsmasq.ssr/ad.conf
	fi
fi
/etc/init.d/dnsmasq restart >/dev/null 2>&1
