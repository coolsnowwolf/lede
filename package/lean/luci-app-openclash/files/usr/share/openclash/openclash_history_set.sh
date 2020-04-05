#!/bin/sh

HISTORY_PATH="/etc/openclash/history"
SECRET=$(uci get openclash.config.dashboard_password 2>/dev/null)
LAN_IP=$(uci get network.lan.ipaddr 2>/dev/null |awk -F '/' '{print $1}' 2>/dev/null)
PORT=$(uci get openclash.config.cn_port 2>/dev/null)

urlencode() {
    local data
    if [ "$#" != 1 ]; then
        return 1
    fi
    data=$(curl -s -o /dev/null -w %{url_effective} --get --data-urlencode "$1" "")
    if [ ! -z "$data" ]; then
        echo "${data##/?}"
    fi
    return 0
}

cat $HISTORY_PATH |while read line
do
   if [ -z "$(echo $line |grep "#*#")" ]; then
      continue
   else
      GROUP_NAME=$(urlencode "$(echo $line |awk -F '#*#' '{print $1}')")
      NOW_NAME=$(echo $line |awk -F '#*#' '{print $3}')
      curl -H "Authorization: Bearer ${SECRET}" -H "Content-Type:application/json" -X PUT -d '{"name":"'"$NOW_NAME"'"}' http://"$LAN_IP":"$PORT"/proxies/"$GROUP_NAME" >/dev/null 2>&1
   fi
done >/dev/null 2>&1
