#!/bin/sh

get_uptime() {
	local uptime=$(cat /proc/uptime)
	echo "${uptime%%.*}"
}
