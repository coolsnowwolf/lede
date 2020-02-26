#!/bin/sh

log_max_size=100
log_file="/tmp/unblockmusic.log"
log_size=0

while true
do
	sleep 10m
	icount=`busybox ps -w | grep UnblockNeteaseMusic/app.js |grep -v grep`
	[ -z "$icount" ] && /etc/init.d/unblockmusic restart 
	log_size=$(expr $(ls -l $log_file | awk '{print $5}') / 1024)
	[ $log_size -ge $log_max_size ] && echo "$(date -R) # Start UnblockNeteaseMusic" >/tmp/unblockmusic.log
done
