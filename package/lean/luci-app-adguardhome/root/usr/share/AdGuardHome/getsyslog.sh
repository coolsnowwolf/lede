#!/bin/sh
PATH="/usr/sbin:/usr/bin:/sbin:/bin"
logread -e AdGuardHome > /tmp/AdGuardHometmp.log
logread -e AdGuardHome -f >> /tmp/AdGuardHometmp.log &
pid=$!
echo "1">/var/run/AdGuardHomesyslog
while true
do
	sleep 12
	watchdog=$(cat /var/run/AdGuardHomesyslog)
	if [ "$watchdog"x == "0"x ]; then
		kill $pid
		rm /tmp/AdGuardHometmp.log
		rm /var/run/AdGuardHomesyslog
		exit 0
	else
		echo "0">/var/run/AdGuardHomesyslog
	fi
done