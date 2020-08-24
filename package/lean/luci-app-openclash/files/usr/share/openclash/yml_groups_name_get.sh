#!/bin/sh
. /lib/functions.sh

CFG_FILE=$(uci get openclash.config.config_path 2>/dev/null)
UPDATE_CONFIG_FILE=$(uci get openclash.config.config_update_path 2>/dev/null)

get_nextlen()
{

	 if [ "$#" -eq 0 ]; then
	    return
	 fi

   for i in $@; do
      if [ -z "$group_len" ]; then
         break
      fi
      
	    if [ "$group_len" -ge "$i" ]; then
	       continue
	    fi
	    
	    if [ "$next_len" -gt "$i" ] || [ -z "$next_len" ]; then
	       next_len="$i"
	    fi
   done 2>/dev/null

}

if [ ! -z "$UPDATE_CONFIG_FILE" ]; then
   CFG_FILE="$UPDATE_CONFIG_FILE"
fi

if [ -z "$CFG_FILE" ]; then
	CFG_FILE="/etc/openclash/config/$(ls -lt /etc/openclash/config/ | grep -E '.yaml|.yml' | head -n 1 |awk '{print $9}')"
fi

if [ -f "$CFG_FILE" ]; then
   #检查关键字避免后续操作出错
	 /usr/share/openclash/yml_field_name_ch.sh "$CFG_FILE"
   
#判断各个区位置
   group_len=$(sed -n '/^proxy-groups:/=' "$CFG_FILE" 2>/dev/null)
   provider_len=$(sed -n '/proxy-providers:/=' "$CFG_FILE" 2>/dev/null)
   rule_provider_len=$(sed -n '/^rule-providers:/=' "$CFG_FILE" 2>/dev/null)
   script_len=$(sed -n '/^script:/=' "$CFG_FILE" 2>/dev/null)
   get_nextlen "$provider_len" "$rule_provider_len" "$script_len"
   
   if [ -n "$next_len" ]; then
       sed -n "${group_len},${next_len}p" "$CFG_FILE" 2>/dev/null |sed "s/\'//g" 2>/dev/null |sed 's/\"//g' 2>/dev/null |sed 's/\t/ /g' 2>/dev/null |grep name: |awk -F 'name:' '{print $2}' |sed 's/,.*//' |sed 's/^ \{0,\}//' 2>/dev/null |sed 's/ \{0,\}$//' 2>/dev/null |sed 's/ \{0,\}\}\{0,\}$//g' 2>/dev/null >/tmp/Proxy_Group 2>&1
   else
       awk '/proxy-groups:/,/rules:/{print}' "$CFG_FILE" 2>/dev/null |sed "s/\'//g" 2>/dev/null |sed 's/\"//g' 2>/dev/null |sed 's/\t/ /g' 2>/dev/null |grep name: |awk -F 'name:' '{print $2}' |sed 's/,.*//' |sed 's/^ \{0,\}//' 2>/dev/null |sed 's/ \{0,\}$//' 2>/dev/null |sed 's/ \{0,\}\}\{0,\}$//g' 2>/dev/null >/tmp/Proxy_Group 2>&1
   fi

   if [ "$?" -eq "0" ]; then
      echo 'DIRECT' >>/tmp/Proxy_Group
      echo 'REJECT' >>/tmp/Proxy_Group
   else
      echo '读取错误，配置文件异常！' >/tmp/Proxy_Group
   fi
else
   echo '读取错误，配置文件异常！' >/tmp/Proxy_Group
fi