#!/bin/sh

log_max_size=100
log_file="/tmp/unblockmusic.log"
log_size=0

/usr/share/UnblockNeteaseMusic/getmusicip.sh
sleep 29s

while true
do
  icount=`busybox ps -w | grep UnblockNeteaseMusic | grep -v grep | grep -v logcheck.sh`
	if [ -z "$icount" ]; then
      /usr/share/UnblockNeteaseMusic/getmusicip.sh
      /etc/init.d/unblockmusic restart 
  fi
	log_size=$(expr $(ls -l $log_file | awk '{print $5}') / 1024)
	[ $log_size -ge $log_max_size ] && echo "$(date -R) # Start UnblockNeteaseMusic" >/tmp/unblockmusic.log
	sleep 29s
done
