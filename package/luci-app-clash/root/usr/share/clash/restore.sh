#!/bin/sh /etc/rc.common

if [ -f /usr/share/clashbackup/history ];then

HISTORY_PATH="/usr/share/clashbackup/history"
SECRET=$(uci get clash.config.dash_pass 2>/dev/null)
LAN_IP=$(uci get network.lan.ipaddr 2>/dev/null |awk -F '/' '{print $1}' 2>/dev/null)
PORT=$(uci get clash.config.dash_port 2>/dev/null)
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
      GORUP_NAME=$(urlencode "$(echo $line |awk -F '#*#' '{print $1}')")
      NOW_NAME=$(echo $line |awk -F '#*#' '{print $3}')
      curl -H "Authorization: Bearer ${SECRET}" -H "Content-Type:application/json" -X PUT -d '{"name":"'"$NOW_NAME"'"}' http://"$LAN_IP":"$PORT"/proxies/"$GORUP_NAME" >/dev/null 2>&1
    fi
done >/dev/null 2>&1 
curl -m 5 --retry 2 -H "Authorization: Bearer ${SECRET}" -H "Content-Type:application/json" -X DELETE http://"$LAN_IP":"$PORT"/connections >/dev/null 2>&1   
fi 