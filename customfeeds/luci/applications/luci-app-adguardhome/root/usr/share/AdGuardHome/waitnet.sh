#!/bin/sh

PATH="/usr/sbin:/usr/bin:/sbin:/bin"
count=0
while :
do
	ping -c 1 -W 1 -q www.baidu.com 1>/dev/null 2>&1
	if [ "$?" == "0" ]; then
		/etc/init.d/AdGuardHome force_reload
		break
	fi
	ping -c 1 -W 1 -q 202.108.22.5 1>/dev/null 2>&1
	if [ "$?" == "0" ]; then
		/etc/init.d/AdGuardHome force_reload
		break
	fi
	sleep 5
	ping -c 1 -W 1 -q www.google.com 1>/dev/null 2>&1
	if [ "$?" == "0" ]; then
		/etc/init.d/AdGuardHome force_reload
		break
	fi
	ping -c 1 -W 1 -q 8.8.8.8 1>/dev/null 2>&1
	if [ "$?" == "0" ]; then
		/etc/init.d/AdGuardHome force_reload
		break
	fi
	sleep 5
	count=$((count+1))
	if [ $count -gt 18 ]; then
		/etc/init.d/AdGuardHome force_reload
		break
	fi
done
return 0
