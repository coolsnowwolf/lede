#!/bin/sh /etc/rc.common
# Copyright (C) 2013-2014 OpenWrt.org

START=00
STOP=90

RTC_DEV=/dev/rtc0
HWCLOCK=/sbin/hwclock

boot() {
	hwclock_load
	local maxtime="$(find_max_time)"
	local curtime="$(date +%s)"
	if [ $curtime -lt $maxtime ]; then
		date -s @$maxtime
		hwclock_save
	fi
}

start() {
	hwclock_load
}

stop() {
	hwclock_save
}

hwclock_load() {
	[ -e "$RTC_DEV" ] && [ -e "$HWCLOCK" ] && $HWCLOCK -s -u -f $RTC_DEV
}

hwclock_save(){
	[ -e "$RTC_DEV" ] && [ -e "$HWCLOCK" ] && $HWCLOCK -w -u -f $RTC_DEV && \
		logger -t sysfixtime "saved '$(date)' to $RTC_DEV"
}

find_max_time() {
	local file newest

	for file in $( find /etc -type f ) ; do
		[ -z "$newest" -o "$newest" -ot "$file" ] && newest=$file
	done
	[ "$newest" ] && date -r "$newest" +%s
}
