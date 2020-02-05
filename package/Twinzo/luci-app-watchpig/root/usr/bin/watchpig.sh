#!/bin/sh
#
# Copyright (C) 2010 segal.di.ubi.pt
#
# This is free software, licensed under the GNU General Public License v2.
#

mode="$1"

# Fix potential typo in mode (backward compatibility).
[ "$mode" = "allways" ] && mode="allways"
[ "$mode" = "Ranmac" ] && mode="Ranmac"
[ "$mode" = "Noping" ] && mode="Noping"
[ "$mode" = "ping3" ] && mode="ping3"
[ "$mode" = "ping2" ] && mode="ping2"
[ "$mode" = "ping1" ] && mode="ping1"

lang()
{
	language=$(uci get luci.main.lang 2>/dev/null)
	language_zh_cn=$(uci get luci.languages.zh_cn 2>/dev/null)
    ([ "$language" == "zh_cn" ]||([ "$language" == "auto" ]&&[ -n "language_zh_cn" ]))&&echo 1
}

shutdown_now() {
	local forcedelay="$1"

	reboot &

	[ "$forcedelay" -ge 1 ] && {
		sleep "$forcedelay"

		echo b > /proc/sysrq-trigger # Will immediately reboot the system without syncing or unmounting your disks.
	}
}

stochastic(){
	echo `dd if=/dev/urandom bs=1 count=32 2>/dev/null | md5sum | cut -b 0-12 | sed 's/\(..\)/\1:/g; s/.$//'`
}

random_mac()
{	
	local stom=$("stochastic"); local interface=$(uci get watchpig.@mac[0].interface)
	uci set network.$interface.macaddr="$stom" 2>/dev/null
	uci commit 2>/dev/null
}

reconnect(){
	local forcedelay="$1"
	[ "$forcedelay" -ge 1 ] && {
		sleep "$forcedelay"
	}
	/etc/init.d/network reload&
}

watchpig_allways() {
	local period="$1"; local forcedelay="$2";local custom="${3:-}"
	local realperiod="$1";

	sleep "$period" && shutdown_now "$forcedelay" && eval "$custom"	
}

watchpig_ranmac() {
	local period="$1"; local forcedelay="$2";local custom="${3:-}"

	sleep "$period" && random_mac && reconnect "$forcedelay" && eval "$custom"
}

watchpig_reconnect() {
	local period="$1"; local forcedelay="$2";local custom="${3:-}"

	sleep "$period" && reconnect "$forcedelay" && eval "$custom"
}

watchpig_ping_behaviour() {
	local action="$1"
	[ "$action" = "ping1" ] && sleep "$period" && shutdown_now "$forcedelay"
	[ "$action" = "ping2" ] && sleep "$period" && reconnect "$forcedelay"
	[ "$action" = "ping1" ] && sleep "$period" && random_mac && reconnect "$forcedelay"
}

watchpig_ping() {
	local period="$1"; local forcedelay="$2"; local pinghosts="$3"; local pingperiod="$4"
	local custom="$5";local behave="$6"
	time_now="$(cat /proc/uptime)"
	time_now="${time_now%%.*}"
	time_lastcheck="$time_now"
	time_lastcheck_withinternet="$time_now"

	while true
	do
		# account for the time ping took to return. With a ping time of 5s, ping might take more than that, so it is important to avoid even more delay.
		time_now="$(cat /proc/uptime)"
		time_now="${time_now%%.*}"
		time_diff="$((time_now-time_lastcheck))"

		[ "$time_diff" -lt "$pingperiod" ] && {
			sleep_time="$((pingperiod-time_diff))"
			sleep "$sleep_time"
		}

		time_now="$(cat /proc/uptime)"
		time_now="${time_now%%.*}"
		time_lastcheck="$time_now"

		for host in $pinghosts
		do
			if ping -c 1 "$host" &> /dev/null
			then
				time_lastcheck_withinternet="$time_now"
			else
				time_diff="$((time_now-time_lastcheck_withinternet))"
				[ $("lang") -ne 1 ] && logger -p daemon.info -t "watchpig[$$]" "no internet connectivity for $time_diff seconds. Reseting when reaching $realperiod"
				[ $("lang") -eq 1 ] && logger -p daemon.info -t "watchpig[$$]" "网络中断达$time_diff秒。达到$realperiod时重置"
				[ $("lang") -eq 1 ] && logger -p daemon.info -t "watchpig[$$]" "别担心，这可能只是普通的延迟"
			fi
		done

		time_diff="$((time_now-time_lastcheck_withinternet))"
		[ "$time_diff" -ge "$period" ] && watchpig_ping_behaviour "$behave" && eval "$custom"

	done
}

	if [ "$mode" = "allways" ]
	then
		watchpig_allways "$2" "$3" "$4"
	elif [ "$mode" = "Ranmac" ]; then
		watchpig_ranmac "$2" "$3" "$4"
	elif [ "$mode" = "Noping" ]; then
		watchpig_reconnect "$2" "$3" "$4"
	elif [ "$mode" = "ping1" ]; then
		watchpig_ping "$2" "$3" "$4" "$5" "$6" "ping1" 
	elif [ "$mode" = "ping2" ]; then
		watchpig_ping "$2" "$3" "$4" "$5" "$6" "ping2"
	elif [ "$mode" = "ping3" ]; then
		watchpig_ping "$2" "$3" "$4" "$5" "$6" "ping3"
	else
		random_mac && /etc/init.d/network reload
	fi
