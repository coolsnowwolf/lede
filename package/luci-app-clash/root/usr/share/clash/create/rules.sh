#!/bin/sh /etc/rc.common
. /lib/functions.sh
rules_set()
{

   CFG_FILE="/etc/config/clash"
   local section="$1"
   config_get "type" "$section" "type" ""
   config_get "rulename" "$section" "rulename" ""
   config_get "rulenamee" "$section" "rulenamee" ""

   if [ -z "$type" ]; then
      return
   fi
   
   
   if [ -z "$rulename" ]; then
      uci set clash."$section".rulename="$rulenamee"
      uci commit clash
   fi
	  

}

start(){
   config_load "clash"
   config_foreach rules_set "rules"
}

start
