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
small_flash_memory=$(uci -q get openclash.config.small_flash_memory)
HISTORY_PATH_OLD="/etc/openclash/history/${CONFIG_NAME%.*}"
HISTORY_PATH="/etc/openclash/history/${CONFIG_NAME%.*}.db"
core_version=$(uci -q get openclash.config.core_version || echo 0)
CACHE_PATH_OLD="/etc/openclash/.cache"
source "/etc/openwrt_release"

set_lock

if [ -z "$CONFIG_FILE" ] || [ ! -f "$CONFIG_FILE" ]; then
   CONFIG_FILE=$(uci get openclash.config.config_path 2>/dev/null)
   CONFIG_NAME=$(echo "$CONFIG_FILE" |awk -F '/' '{print $5}' 2>/dev/null)
   HISTORY_PATH_OLD="/etc/openclash/history/${CONFIG_NAME%.*}"
   HISTORY_PATH="/etc/openclash/history/${CONFIG_NAME%.*}.db"
fi

if [ -n "$(pidof clash)" ] && [ -f "$CONFIG_FILE" ]; then
   if [ "$small_flash_memory" == "1" ] || [ -n "$(echo $core_version |grep mips)" ] || [ -n "$(echo $DISTRIB_ARCH |grep mips)" ] || [ -n "$(opkg status libc 2>/dev/null |grep 'Architecture' |awk -F ': ' '{print $2}' |grep mips)" ]; then
   CACHE_PATH="/tmp/etc/openclash/cache.db"
      if [ -f "$CACHE_PATH" ]; then
         cmp -s "$CACHE_PATH" "$HISTORY_PATH"
         if [ "$?" -ne "0" ]; then
            cp "$CACHE_PATH" "$HISTORY_PATH" 2>/dev/null
         fi
      fi
   fi
   if [ -f "$CACHE_PATH_OLD" ]; then
      cmp -s "$CACHE_PATH_OLD" "$HISTORY_PATH_OLD"
      if [ "$?" -ne "0" ]; then
         cp "$CACHE_PATH_OLD" "$HISTORY_PATH_OLD" 2>/dev/null
      fi
   fi
fi

del_lock