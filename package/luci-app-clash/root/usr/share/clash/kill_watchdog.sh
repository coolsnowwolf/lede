#!/bin/sh 

enable=$(uci get clash.config.enable 2>/dev/null)
if [ "${enable}" -eq 0 ];then

if [ -f  /tmp/watchlist ];then
rm -rf /tmp/watchlist
fi
line=$(ps | grep -n 'clash-watchdog.sh'|awk -F ':' '{print $2}' |awk -F ' ' '{print $1}'>/tmp/watchlist)
line_no=$(grep -n '' /tmp/watchlist|awk -F ':' '{print $1}')
num=$(grep -c '' /tmp/watchlist| awk '{print $1}')
nums=1
while [[ $nums -le $num ]]
do
	kill -9 $(sed -n "$nums"p /tmp/watchlist| awk '{print $1}') >/dev/null 2>&1 
	nums=$(( $nums + 1))
done
fi 
  
