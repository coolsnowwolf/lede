#!/bin/sh
PATH="/usr/sbin:/usr/bin:/sbin:/bin"
echo "1">/var/run/onsetnlbw
interval=$(uci get nlbwmon.@nlbwmon[0].refresh_interval)
uci set nlbwmon.@nlbwmon[0].refresh_interval="2s"
/etc/init.d/nlbwmon reload
while true
do
	sleep 12
	watchdog=$(cat /var/run/onsetnlbw)
	if [ "$watchdog"x == "0"x ]; then
		uci set nlbwmon.@nlbwmon[0].refresh_interval=$interval
		uci commit nlbwmon
		/etc/init.d/nlbwmon reload
		rm -f /var/run/onsetnlbw
		exit 0
	else
		echo "0">/var/run/onsetnlbw
	fi
done