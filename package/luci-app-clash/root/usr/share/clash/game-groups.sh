#!/bin/sh /etc/rc.common
. /lib/functions.sh
cfg_groups_set()
{

   CFG_FILE="/etc/config/clash"
   local section="$1"
   config_get "name" "$section" "name" ""
   config_get "old_name_cfg" "$section" "old_name_cfg" ""
   config_get "old_name" "$section" "old_name" ""

   if [ -z "$name" ]; then
      return
   fi
   
    
   if [ "$name" != "$old_name_cfg" ]; then
      sed -i "s/\'${old_name_cfg}\'/\'${name}\'/g" $CFG_FILE 2>/dev/null
      sed -i "s/old_name \'${name}\'/old_name \'${old_name}\'/g" $CFG_FILE 2>/dev/null
      config_load "clash"
   fi

}

start(){
status=$(ps|grep -c /usr/share/clash/game-groups.sh)
[ "$status" -gt "3" ] && exit 0

   config_load "clash"
   config_foreach cfg_groups_set "conf_groups"
}
