#!/bin/sh

. /etc/openwrt_release

IEEE_PATH="/sys/class/ieee80211"
THERMAL_PATH="/sys/class/thermal"

case "$DISTRIB_TARGET" in
ipq40xx/*|ipq806x/*)
	wifi_temp="$(awk '{printf("%.1f°C ", $0 / 1000)}' "$IEEE_PATH"/phy*/device/hwmon/hwmon*/temp1_input 2>"/dev/null" | awk '$1=$1')"
	;;
mediatek/mt7622)
	wifi_temp="$(awk '{printf("%.1f°C ", $0 / 1000)}' "$IEEE_PATH"/wl*/hwmon*/temp1_input 2>"/dev/null" | awk '$1=$1')"
	;;
*)
	wifi_temp="$(awk '{printf("%.1f°C ", $0 / 1000)}' "$IEEE_PATH"/phy*/hwmon*/temp1_input 2>"/dev/null" | awk '$1=$1')"
	;;
esac

case "$DISTRIB_TARGET" in
ipq40xx/*)
	if [ -e "$IEEE_PATH/phy0/hwmon0/temp1_input" ]; then
		mt76_temp="$(awk -F ': ' '{print $2}' "$IEEE_PATH/phy0/hwmon0/temp1_input" 2>"/dev/null")°C"
	fi
	[ -z "$mt76_temp" ] || wifi_temp="${wifi_temp:+$wifi_temp }$mt76_temp"
	;;
*)
	cpu_temp="$(awk '{printf("%.1f°C", $0 / 1000)}' "$THERMAL_PATH/thermal_zone0/temp" 2>"/dev/null")"
	;;
esac

if [ -n "$cpu_temp" ] && [ -z "$wifi_temp" ]; then
	echo -n "CPU: $cpu_temp"
elif [ -z "$cpu_temp" ] && [ -n "$wifi_temp" ]; then
	echo -n "WiFi: $wifi_temp"
elif [ -n "$cpu_temp" ] && [ -n "$wifi_temp" ]; then
	echo -n "CPU: $cpu_temp, WiFi: $wifi_temp"
else
	echo -n "No temperature info"
fi
