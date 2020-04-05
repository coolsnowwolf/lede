#!/bin/sh /etc/rc.common

cfg_groups_set()
{

   CFG_FILE="/etc/config/openclash"
   local section="$1"
   config_get "name" "$section" "name" ""
   config_get "old_name_cfg" "$section" "old_name_cfg" ""
   config_get "old_name" "$section" "old_name" ""

   if [ -z "$name" ]; then
      return
   fi
   
   if [ -z "$old_name_cfg" ]; then
      uci set openclash."$section".old_name_cfg="$name"
      uci commit openclash
   fi
   
   if [ -z "$old_name" ]; then
      uci set openclash."$section".old_name="$name"
      uci commit openclash
   fi
   
   #名字变化时处理配置文件
   if [ "$name" != "$old_name_cfg" ] && [ ! -z "$old_name_cfg" ]; then
   	  sed -i "s/old_name_cfg \'${old_name_cfg}\'/old_name_cfg \'${name}\'/g" $CFG_FILE 2>/dev/null
      sed -i "s/groups \'${old_name_cfg}/groups \'${name}/g" $CFG_FILE 2>/dev/null
      sed -i "s/other_group \'${old_name_cfg}/other_group \'${name}/g" $CFG_FILE 2>/dev/null
      sed -i "s/new_servers_group \'${old_name_cfg}/new_servers_group \'${name}/g" $CFG_FILE 2>/dev/null
      sed -i "s/relay_groups \'${old_name_cfg}/relay_groups \'${name}/g" $CFG_FILE 2>/dev/null
      config_load "openclash"
   fi

}

start(){
status=$(ps|grep -c /usr/share/openclash/yml_groups_name_ch.sh)
[ "$status" -gt "3" ] && exit 0

   config_load "openclash"
   config_foreach cfg_groups_set "groups"
}
