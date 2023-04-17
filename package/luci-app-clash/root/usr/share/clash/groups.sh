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
   
    
   if [ -z "$old_name_cfg" ]; then
      uci set clash."$section".old_name_cfg="$name"
      uci commit clash
   fi
   
   if [ -z "$old_name" ]; then
      uci set clash."$section".old_name="$name"
      uci commit clash
   fi
   
	  
   if [ "$name" != "$old_name_cfg" ] && [ ! -z "$old_name_cfg" ]; then
   	  sed -i "s/old_name_cfg \'${old_name_cfg}\'/old_name_cfg \'${name}\'/g" $CFG_FILE 2>/dev/null
      sed -i "s/groups \'${old_name_cfg}/groups \'${name}/g" $CFG_FILE 2>/dev/null
      sed -i "s/other_group \'${old_name_cfg}/other_group \'${name}/g" $CFG_FILE 2>/dev/null
      sed -i "s/new_servers_group \'${old_name_cfg}/new_servers_group \'${name}/g" $CFG_FILE 2>/dev/null
      sed -i "s/relay_groups \'${old_name_cfg}/relay_groups \'${name}/g" $CFG_FILE 2>/dev/null	  
      config_load "clash"
   fi

}

start(){
status=$(ps|grep -c /usr/share/clash/groups.sh)
[ "$status" -gt "3" ] && exit 0

   config_load "clash"
   config_foreach cfg_groups_set "groups"
}
