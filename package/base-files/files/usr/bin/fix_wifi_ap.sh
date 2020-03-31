#!/bin/sh

#
# Fix loss of AP mode fails 
#

TIMEOUT=30
SLEEP=3
try_times=0
DEVICENAME=phy0

logger -t fix_wifi_ap "enter fix_wifi_ap.sh"
if [ $(uci get wireless.default_radio0.mode | grep -c "ap") -eq 1 ]; then
    while [ $(iwinfo | grep -c "ESSID: unknown") -ge 1 ]; do
        let try_times=$try_times+1
        logger -t fix_wifi_ap "loop times: ${try_times}"
        if [ -f /var/run/hostapd-phy0.conf ]; then
            logger -t fix_wifi_ap "found hostapd-phy0.conf"
    	# same as /etc/hotplug.d/ieee80211/20-hostapd
    	if [ -f /var/run/hostapd-${DEVICENAME}.pid ]; then
                kill $(cat /var/run/hostapd-${DEVICENAME}.pid)
    	    rm -rf /var/run/hostapd-${DEVICENAME}.pid
            fi
            logger -t fix_wifi_ap "force kill hostapd"
            killall hostapd
            if [ -d /var/run/hostapd-${DEVICENAME} ]; then
                rm -rf /var/run/hostapd-${DEVICENAME}
            fi
            logger -t fix_wifi_ap "re-run hostapd"
            /usr/sbin/hostapd -s -n phy0 -P /var/run/hostapd-${DEVICENAME}.pid -B /var/run/hostapd-phy0.conf
        fi

        if [ $((try_times * SLEEP)) -ge $TIMEOUT ]; then
            break
        fi
        sleep $SLEEP
    done
else
    logger -t fix_wifi_ap "do nothing, wireless.default_radio0.mode is not ap"
fi

logger -t fix_wifi_ap "leave fix_wifi_ap.sh"

