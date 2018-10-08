#!/bin/sh

rm -f /tmp/dnsmasq.adblock

wget-ssl --no-check-certificate -O- https://easylist-downloads.adblockplus.org/easylistchina+easylist.txt | grep ^\|\|[^\*]*\^$ | sed -e 's:||:address\=\/:' -e 's:\^:/0\.0\.0\.0:' > /tmp/dnsmasq.adblock
if [ -s "/tmp/dnsmasq.adblock" ];then
	sed -i '/youku.com/d' /tmp/dnsmasq.adblock
	if ( ! cmp -s /tmp/dnsmasq.adblock /usr/share/adbyby/dnsmasq.adblock );then
		mv /tmp/dnsmasq.adblock /usr/share/adbyby/dnsmasq.adblock	
	fi	
fi

sh /usr/share/adbyby/adupdate.sh




