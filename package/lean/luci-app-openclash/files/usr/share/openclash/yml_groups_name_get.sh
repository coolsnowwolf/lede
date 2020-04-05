#!/bin/sh

CFG_FILE=$(uci get openclash.config.config_path 2>/dev/null)
UPDATE_CONFIG_FILE=$(uci get openclash.config.config_update_path 2>/dev/null)

if [ ! -z "$UPDATE_CONFIG_FILE" ]; then
   CFG_FILE="$UPDATE_CONFIG_FILE"
fi

if [ -z "$CFG_FILE" ]; then
	CFG_FILE="/etc/openclash/config/$(ls -lt /etc/openclash/config/ | grep -E '.yaml|.yml' | head -n 1 |awk '{print $9}')"
fi

if [ -f "$CFG_FILE" ]; then
   #检查关键字避免后续操作出错
   #proxies
   [ -z "$(grep "^Proxy:" "$CFG_FILE")" ] && {
      sed -i "s/^ \{1,\}Proxy:/c\Proxy:/" "$CFG_FILE" 2>/dev/null
   }
   [ -z "$(grep "^Proxy:" "$CFG_FILE")" ] && {
      sed -i "s/^proxies:/Proxy:/" "$CFG_FILE" 2>/dev/null
   }

	 #proxy-providers
	 [ -z "$(grep "^proxy-provider:" "$CFG_FILE")" ] && {
      sed -i "s/^ \{1,\}proxy-provider:/proxy-provider:/" "$CFG_FILE" 2>/dev/null
   }
   [ -z "$(grep "^proxy-provider:" "$CFG_FILE")" ] && {
      sed -i "/^ \{0,\}proxy-providers:/c\proxy-provider:" "$CFG_FILE" 2>/dev/null
   }
   #proxy-groups
   [ -z "$(grep "^Proxy Group:" "$CFG_FILE")" ] && {
      sed -i "s/^ \{0,\}\'Proxy Group\':/Proxy Group:/" "$CFG_FILE" 2>/dev/null
      sed -i 's/^ \{0,\}\"Proxy Group\":/Proxy Group:/' "$CFG_FILE" 2>/dev/null
      sed -i "s/^ \{1,\}Proxy Group:/Proxy Group:/" "$CFG_FILE" 2>/dev/null
   }
   [ -z "$(grep "^Proxy Group:" "$CFG_FILE")" ] && {
      sed -i "s/^ \{0,\}proxy-groups:/Proxy Group:/" "$CFG_FILE" 2>/dev/null
   }
   
   #rules
   [ -z "$(grep "^Rule:" "$CFG_FILE")" ] && {
      sed -i "s/^ \{1,\}Rule:/Rule:/" "$CFG_FILE" 2>/dev/null
   }
   [ -z "$(grep "^Rule:" "$CFG_FILE")" ] && {
      sed -i "/^ \{0,\}rules:/c\Rule:" "$CFG_FILE" 2>/dev/null
   }
   
   #dns
   [ -z "$(grep "^dns:" "$CFG_FILE")" ] && {
      sed -i "s/^ \{1,\}dns:/dns:/" "$CFG_FILE" 2>/dev/null
   }
   
#判断各个区位置
   group_len=$(sed -n '/^ \{0,\}Proxy Group:/=' "$CONFIG_FILE" 2>/dev/null)
   provider_len=$(sed -n '/^ \{0,\}proxy-provider:/=' "$CONFIG_FILE" 2>/dev/null)
   if [ "$provider_len" -ge "$group_len" ]; then
       awk '/Proxy Group:/,/proxy-provider:/{print}' "$CFG_FILE" 2>/dev/null |sed "s/\'//g" 2>/dev/null |sed 's/\"//g' 2>/dev/null |sed 's/\t/ /g' 2>/dev/null |grep name: |awk -F 'name:' '{print $2}' |sed 's/,.*//' |sed 's/^ \{0,\}//' 2>/dev/null |sed 's/ \{0,\}$//' 2>/dev/null |sed 's/ \{0,\}\}\{0,\}$//g' 2>/dev/null >/tmp/Proxy_Group 2>&1
   else
       awk '/Proxy Group:/,/Rule:/{print}' "$CFG_FILE" 2>/dev/null |sed "s/\'//g" 2>/dev/null |sed 's/\"//g' 2>/dev/null |sed 's/\t/ /g' 2>/dev/null |grep name: |awk -F 'name:' '{print $2}' |sed 's/,.*//' |sed 's/^ \{0,\}//' 2>/dev/null |sed 's/ \{0,\}$//' 2>/dev/null |sed 's/ \{0,\}\}\{0,\}$//g' 2>/dev/null >/tmp/Proxy_Group 2>&1
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