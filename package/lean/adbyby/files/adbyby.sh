#!/bin/sh
PROG_PATH=/usr/share/adbyby
err=0
until [ $err -ge 5 ]; do
	if [ -n "$(pgrep $PROG_PATH/adbyby)" ]; then
		iptables-save | grep ADBYBY >/dev/null || \
		/etc/init.d/adbyby add_rule
		sleep 10
		err=0
	else
		$PROG_PATH/adbyby --no-daemon &>/dev/null &
		sleep 1
		err=$((err+1))
	fi
done
/etc/init.d/adbyby del_rule
