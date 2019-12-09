#!/bin/bash

log_max_size="100" #使用KB计算
log_file="/tmp/unblockmusic.log"

while true
do
  sleep 30s
  icount=`busybox ps -w | grep app.js |grep -v grep| wc -l`
  if [ $icount -ne 2 ] ;then 
    /etc/init.d/unblockmusic restart 
  fi
	(( log_size = "$(ls -l "${log_file}" | awk -F ' ' '{print $5}')" / "1024" ))
	(( "${log_size}" >= "${log_max_size}" )) && echo "" > /tmp/unblockmusic.log
	sleep 10m
done

