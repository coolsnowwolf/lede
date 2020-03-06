#!/bin/sh

log_max_size=100
log_file="/tmp/unblockmusic.log"
log_size=0

while true
do
  ipset -! -N music hash:ip
  sleep 10s
  wget-ssl -q -t 99 -T 10 http://httpdns.n.netease.com/httpdns/v2/d?domain=music.163.com,interface.music.163.com,interface3.music.163.com,apm.music.163.com,apm3.music.163.com,clientlog.music.163.com,clientlog3.music.163.com -O- | grep -Eo '[0-9]+?\.[0-9]+?\.[0-9]+?\.[0-9]+?' | sort | uniq | awk '{print "ipset -! add music "$1}' | sh
	icount=`busybox ps -w | grep UnblockNeteaseMusic | grep -v grep | grep -v logcheck.sh`
	[ -z "$icount" ] && /etc/init.d/unblockmusic restart 
	log_size=$(expr $(ls -l $log_file | awk '{print $5}') / 1024)
	[ $log_size -ge $log_max_size ] && echo "$(date -R) # Start UnblockNeteaseMusic" >/tmp/unblockmusic.log
	sleep 30s
done
