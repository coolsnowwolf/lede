#!/bin/sh 

enable=$(uci get clash.config.enable 2>/dev/null)
if [ "${enable}" -eq 1 ];then
	if ! pidof clash>/dev/null; then
		/etc/init.d/clash restart 2>&1 &
	fi
fi





