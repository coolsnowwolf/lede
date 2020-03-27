#!/bin/sh /etc/rc.common
START=50

NAME=cpufreq

uci_get_by_type() {
	local ret=$(uci get $NAME.@$1[0].$2 2>/dev/null)
	echo ${ret:=$3}
}

start()
{
	config_load cpufreq
	local governor=$(uci_get_by_type settings governor ondemand)
	local minifreq=$(uci_get_by_type settings minifreq 48000)
	local maxfreq=$(uci_get_by_type settings maxfreq 716000)
	local upthreshold=$(uci_get_by_type settings upthreshold 50)
	local factor=$(uci_get_by_type settings factor 10)

	echo $governor > /sys/devices/system/cpu/cpufreq/policy0/scaling_governor
	echo $minifreq > /sys/devices/system/cpu/cpufreq/policy0/scaling_min_freq
	echo $maxfreq > /sys/devices/system/cpu/cpufreq/policy0/scaling_max_freq
	if [ "$governor" == "ondemand" ]; then
		echo $upthreshold > /sys/devices/system/cpu/cpufreq/ondemand/up_threshold
		echo $factor > /sys/devices/system/cpu/cpufreq/ondemand/sampling_down_factor
	fi
}
