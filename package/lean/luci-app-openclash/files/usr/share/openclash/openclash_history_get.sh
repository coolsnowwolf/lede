#!/bin/sh

CURL_GROUP_CACHE="/tmp/openclash_history_gorup.json"
CURL_NOW_CACHE="/tmp/openclash_history_now.json"
CURL_CACHE="/tmp/openclash_history_curl.json"
HISTORY_PATH="/etc/openclash/history"
SECRET=$(uci get openclash.config.dashboard_password 2>/dev/null)
LAN_IP=$(uci get network.lan.ipaddr 2>/dev/null |awk -F '/' '{print $1}' 2>/dev/null)
PORT=$(uci get openclash.config.cn_port 2>/dev/null)

curl --connect-timeout 5 --retry 2 -w %{http_code}"\n" -H "Authorization: Bearer ${SECRET}" -H "Content-Type:application/json" -X GET http://"$LAN_IP":"$PORT"/proxies > "$CURL_CACHE" 2>/dev/null
if [ "$(sed -n '$p' "$CURL_CACHE")" -eq "200" ]; then
   cat "$CURL_CACHE" |jsonfilter -e '@["proxies"][@.type="Selector"]["name"]' > "$CURL_GROUP_CACHE" 2>/dev/null
   cat "$CURL_CACHE" |jsonfilter -e '@["proxies"][@.type="Selector"]["now"]' > "$CURL_NOW_CACHE" 2>/dev/null
   awk 'NR==FNR{a[i]=$0;i++}NR>FNR{print a[j]"#*#"$0;j++}' "$CURL_GROUP_CACHE" "$CURL_NOW_CACHE" > "$HISTORY_PATH" 2>/dev/null
fi
rm -rf /tmp/openclash_history_*  2>/dev/null