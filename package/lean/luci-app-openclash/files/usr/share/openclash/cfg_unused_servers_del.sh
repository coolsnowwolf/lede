#!/bin/sh
. /lib/functions.sh

cfg_unused_servers_del()
{

   local section="$1"
   config_get_bool "enabled" "$section" "enabled" "1"

   if [ "$enabled" = "1" ]; then
      return
   fi

   #删除未选中节点
   uci delete openclash."$section" 2>/dev/null
}

status=$(ps|grep -c /usr/share/openclash/cfg_unused_servers_del.sh)
[ "$status" -gt "3" ] && exit 0
   config_load "openclash"
   config_foreach cfg_unused_servers_del "servers"
   uci commit openclash

