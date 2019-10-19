#!/bin/bash

log_max_size="4" #使用KB计算
log_file="/tmp/unblockmusic.log"

while true
do
	(( log_size = "$(ls -l "${log_file}" | awk -F ' ' '{print $5}')" / "1024" ))
	(( "${log_size}" >= "${log_max_size}" )) && echo "" > /tmp/unblockmusic.log
	sleep 10m
done

