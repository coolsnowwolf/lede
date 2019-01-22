#!/bin/sh

. /lib/functions.sh

cpulimit_get(){
	echo cpulimit_get
	config_get enabled $1 enabled
	[ $enabled -gt 0 ] || return 1
	config_get limit $1 limit
	config_get exename $1 exename
	cpulimit -l $limit -e $exename &
	echo cpulimit -l $limit -e $exename
}



stop() {
killall -9 cpulimit
#	ps -a|grep cpulimit|while read line
#	do
#	  killall cpulimit>/dev/null 2>&1
#	done
}

case "$1" in
	"start")
	echo start
	killall -9 cpulimit
	config_load cpulimit
	config_foreach cpulimit_get list
	echo end
	exit 0	
;;
	"stop")echo stop; killall -9 cpulimit;;
esac
