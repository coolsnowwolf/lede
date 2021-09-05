#!/bin/sh
. /lib/functions.sh
. /usr/share/openclash/openclash_ps.sh

set_lock() {
   exec 881>"/tmp/lock/openclash_history_get.lock" 2>/dev/null
   flock -x 881 2>/dev/null
}

del_lock() {
   flock -u 881 2>/dev/null
   rm -rf "/tmp/lock/openclash_history_get.lock"
}

close_all_conection() {
   SECRET=$(uci -q get openclash.config.dashboard_password)
   LAN_IP=$(uci -q get network.lan.ipaddr |awk -F '/' '{print $1}' 2>/dev/null || ip addr show 2>/dev/null | grep -w 'inet' | grep 'global' | grep 'brd' | grep -Eo 'inet [0-9\.]+' | awk '{print $2}' | head -n 1)
   PORT=$(uci -q get openclash.config.cn_port)
   curl -m 2 -H "Authorization: Bearer ${SECRET}" -H "Content-Type:application/json" -X DELETE http://"$LAN_IP":"$PORT"/connections >/dev/null 2>&1
}

if [ "$1" = "close_all_conection" ]; then
   close_all_conection
   exit 0
fi

CONFIG_FILE=$(unify_ps_cfgname)
CONFIG_NAME=$(echo "$CONFIG_FILE" |awk -F '/' '{print $4}' 2>/dev/null)
HISTORY_PATH="/etc/openclash/history/${CONFIG_NAME%.*}"
CACHE_PATH="/etc/openclash/.cache"
set_lock

if [ -z "$CONFIG_FILE" ] || [ ! -f "$CONFIG_FILE" ]; then
   CONFIG_FILE=$(uci get openclash.config.config_path 2>/dev/null)
   CONFIG_NAME=$(echo "$CONFIG_FILE" |awk -F '/' '{print $5}' 2>/dev/null)
   HISTORY_PATH="/etc/openclash/history/${CONFIG_NAME%.*}"
fi

if [ -n "$(pidof clash)" ] && [ -f "$CONFIG_FILE" ] && [ -f "$CACHE_PATH" ]; then
   cmp -s "$CACHE_PATH" "$HISTORY_PATH"
   if [ "$?" -ne "0" ]; then
      cp "$CACHE_PATH" "$HISTORY_PATH" 2>/dev/null
   fi
fi

del_lock