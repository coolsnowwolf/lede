#!/bin/sh

CFG_FILE=$(uci get openclash.config.config_path 2>/dev/null)
UPDATE_CONFIG_FILE=$(uci get openclash.config.config_update_path 2>/dev/null)
GROUP_FILE="/tmp/yaml_group_cache.yaml"

if [ ! -z "$UPDATE_CONFIG_FILE" ]; then
   CFG_FILE="$UPDATE_CONFIG_FILE"
fi

if [ -z "$CFG_FILE" ]; then
	CFG_FILE="/etc/openclash/config/$(ls -lt /etc/openclash/config/ | grep -E '.yaml|.yml' | head -n 1 |awk '{print $9}')"
fi

if [ -f "$CFG_FILE" ]; then
   #检查关键字避免后续操作出错
	 /usr/share/openclash/yml_field_name_ch.sh "$CFG_FILE"
   
   #取出group部分
   group_len=$(sed -n '/^proxy-groups:/=' "$CFG_FILE" 2>/dev/null)
   if [ -n "$group_len" ]; then
   	  /usr/share/openclash/yml_field_cut.sh "$group_len" "$GROUP_FILE" "$CFG_FILE"
   	  rm -rf /tmp/yaml_general 2>/dev/null
   fi 2>/dev/null
   
   cat "$GROUP_FILE" |sed "s/\'//g" 2>/dev/null |sed 's/\"//g' 2>/dev/null |sed 's/\t/ /g' 2>/dev/null |grep name: |awk -F 'name:' '{print $2}' |sed 's/,.*//' |sed 's/^ \{0,\}//' 2>/dev/null |sed 's/ \{0,\}$//' 2>/dev/null |sed 's/ \{0,\}\}\{0,\}$//g' 2>/dev/null >/tmp/Proxy_Group 2>&1
   
   rm -rf "$GROUP_FILE" 2>/dev/null

   if [ "$?" -eq "0" ]; then
      echo 'DIRECT' >>/tmp/Proxy_Group
      echo 'REJECT' >>/tmp/Proxy_Group
   else
      echo '读取错误，配置文件异常！' >/tmp/Proxy_Group
   fi
else
   echo '读取错误，配置文件异常！' >/tmp/Proxy_Group
fi