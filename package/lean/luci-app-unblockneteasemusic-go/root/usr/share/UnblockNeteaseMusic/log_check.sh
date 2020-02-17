#!/bin/bash
log_max_size="16" #使用KB计算
log_file="/tmp/unblockneteasemusic.log"

(( log_size = "$(ls -l "${log_file}" | awk -F ' ' '{print $5}')" / "1024" ))
(( "${log_size}" >= "${log_max_size}" )) && echo "" > "${log_file}"

[ "*$(uci get unblockneteasemusic.@unblockneteasemusic[0].daemon_enable)*" == "*1*" ] && { [ -z "$(ps |grep "UnblockNeteaseMusic" |grep -v "grep")" ] && /etc/init.d/unblockneteasemusic restart; }