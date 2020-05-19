#!/bin/sh
checkmd5(){
local nowmd5=$(md5sum /etc/hosts)
nowmd5=${nowmd5%% *}
local lastmd5=$(uci get AdGuardHome.AdGuardHome.hostsmd5 2>/dev/null)
if [ "$nowmd5" != "$lastmd5" ]; then
	uci set AdGuardHome.AdGuardHome.hostsmd5="$nowmd5"
	uci commit AdGuardHome
	[ "$1" == "noreload" ] || /etc/init.d/AdGuardHome reload
fi
}
[ "$1" == "del" ] && sed -i '/programaddstart/,/programaddend/d' /etc/hosts && checkmd5 "$2" && exit 0
/usr/bin/awk 'BEGIN{
while ((getline < "/tmp/dhcp.leases") > 0)
{
    a[$2]=$4;
}
while (("ip -6 neighbor show | grep -v fe80" | getline) > 0)
{
    if (a[$5]) {print $1" "a[$5] >"/tmp/tmphost"; }
}
print "#programaddend" >"/tmp/tmphost";
}'
grep programaddstart /etc/hosts >/dev/null 2>&1
if [ "$?" == "0" ]; then
	sed -i '/programaddstart/,/programaddend/c\#programaddstart' /etc/hosts
	sed -i '/programaddstart/'r/tmp/tmphost /etc/hosts
else
	echo "#programaddstart" >>/etc/hosts
	cat /tmp/tmphost >> /etc/hosts
fi
rm /tmp/tmphost
checkmd5 "$2"