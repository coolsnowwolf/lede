#!/bin/bash /etc/rc.common
status=$(ps|grep -c /usr/share/openclash/openclash.sh)
[ "$status" -gt "3" ] && exit 0

START_LOG="/tmp/openclash_start.log"
LOGTIME=$(date "+%Y-%m-%d %H:%M:%S")
LOG_FILE="/tmp/openclash.log"
CFG_FILE="/tmp/config.yaml"
CONFIG_PATH=$(uci get openclash.config.config_path 2>/dev/null)
servers_update=$(uci get openclash.config.servers_update 2>/dev/null)
dns_port=$(uci get openclash.config.dns_port 2>/dev/null)
enable_redirect_dns=$(uci get openclash.config.enable_redirect_dns 2>/dev/null)
disable_masq_cache=$(uci get openclash.config.disable_masq_cache 2>/dev/null)
if_restart=0

config_download()
{
if [ "$URL_TYPE" == "v2rayn" ]; then
   subscribe_url=`echo $subscribe_url |sed 's/{/%7B/g;s/}/%7D/g;s/:/%3A/g;s/\"/%22/g;s/,/%2C/g;s/?/%3F/g;s/=/%3D/g;s/&/%26/g;s/\//%2F/g'`
   curl -sL --connect-timeout 10 --retry 2 https://tgbot.lbyczf.com/v2rayn2clash?url="$subscribe_url" -o "$CFG_FILE" >/dev/null 2>&1
elif [ "$URL_TYPE" == "surge" ]; then
   subscribe_url=`echo $subscribe_url |sed 's/{/%7B/g;s/}/%7D/g;s/:/%3A/g;s/\"/%22/g;s/,/%2C/g;s/?/%3F/g;s/=/%3D/g;s/&/%26/g;s/\//%2F/g'`
   curl -sL --connect-timeout 10 --retry 2 https://tgbot.lbyczf.com/surge2clash?url="$subscribe_url" -o "$CFG_FILE" >/dev/null 2>&1
else
   curl -sL --connect-timeout 10 --retry 2 --user-agent "clash" "$subscribe_url" -o "$CFG_FILE" >/dev/null 2>&1
fi
}

config_cus_up()
{
	if [ -z "$CONFIG_PATH" ]; then
	    CONFIG_PATH="/etc/openclash/config/$(ls -lt /etc/openclash/config/ | grep -E '.yaml|.yml' | head -n 1 |awk '{print $9}')"
	    uci set openclash.config.config_path="$CONFIG_PATH"
      uci commit openclash
	fi
	if [ "$servers_update" -eq "1" ] || [ ! -z "$keyword" ]; then
	   echo "配置文件【$name】替换成功，开始挑选节点..." >$START_LOG
	   uci set openclash.config.config_update_path="/etc/openclash/config/$name.yaml"
	   uci set openclash.config.servers_if_update=1
	   uci commit openclash
	   /usr/share/openclash/yml_groups_get.sh
	   uci set openclash.config.servers_if_update=1
	   uci commit openclash
	   /usr/share/openclash/yml_groups_set.sh
	   if [ "$CONFIG_FILE" == "$CONFIG_PATH" ]; then
	      if_restart=1
	   fi
	   echo "${LOGTIME} Config 【$name】 Update Successful" >>$LOG_FILE
	   echo "配置文件【$name】更新成功！" >$START_LOG
	   sleep 3
	   echo "" >$START_LOG
	elif [ "$CONFIG_FILE" == "$CONFIG_PATH" ]; then
     echo "${LOGTIME} Config 【$name】 Update Successful" >>$LOG_FILE
     echo "配置文件【$name】更新成功！" >$START_LOG
     sleep 3
     if_restart=1
  else
     echo "配置文件【$name】更新成功！" >$START_LOG
     echo "${LOGTIME} Config 【$name】 Update Successful" >>$LOG_FILE
     sleep 3
     echo "" >$START_LOG
  fi
}

config_su_check()
{
   echo "配置文件下载成功，检查是否有更新..." >$START_LOG
   if [ -f "$CONFIG_FILE" ]; then
      cmp -s "$BACKPACK_FILE" "$CFG_FILE"
         if [ "$?" -ne "0" ]; then
            echo "配置文件【$name】有更新，开始替换..." >$START_LOG
            mv "$CFG_FILE" "$CONFIG_FILE" 2>/dev/null
            cp "$CONFIG_FILE" "$BACKPACK_FILE"
            config_cus_up
         else
            echo "配置文件【$name】没有任何更新，停止继续操作..." >$START_LOG
            rm -rf "$CFG_FILE"
            echo "${LOGTIME} Updated Config【$name】 No Change, Do Nothing" >>$LOG_FILE
            sleep 5
            echo "" >$START_LOG
         fi
   else
      echo "配置文件下载成功，本地没有配置文件，开始创建 ..." >$START_LOG
      mv "$CFG_FILE" "$CONFIG_FILE" 2>/dev/null
      cp "$CONFIG_FILE" "$BACKPACK_FILE"
      config_cus_up
   fi
}

config_encode()
{
	#proxies
   [ -z "$(grep "^Proxy:" "$CFG_FILE")" ] && {
      sed -i "/^ \{1,\}Proxy:/c\Proxy:" "$CFG_FILE" 2>/dev/null
   }
   [ -z "$(grep "^Proxy:" "$CFG_FILE")" ] && {
      sed -i "s/^proxies:/Proxy:/" "$CFG_FILE" 2>/dev/null
   }

	 #proxy-providers
	 [ -z "$(grep "^proxy-provider:" "$CFG_FILE")" ] && {
      sed -i "/^ \{1,\}proxy-provider:/c\proxy-provider:" "$CFG_FILE" 2>/dev/null
   }
   [ -z "$(grep "^proxy-provider:" "$CFG_FILE")" ] && {
      sed -i "/^ \{0,\}proxy-providers:/c\proxy-provider:" "$CFG_FILE" 2>/dev/null
   }
   #proxy-groups
   [ -z "$(grep "^Proxy Group:" "$CFG_FILE")" ] && {
      sed -i "/^ \{0,\}\'Proxy Group\':/c\Proxy Group:" "$CFG_FILE" 2>/dev/null
      sed -i '/^ \{0,\}\"Proxy Group\":/c\Proxy Group:' "$CFG_FILE" 2>/dev/null
      sed -i "/^ \{1,\}Proxy Group:/c\Proxy Group:" "$CFG_FILE" 2>/dev/null
   }
   [ -z "$(grep "^Proxy Group:" "$CFG_FILE")" ] && {
      sed -i "/^ \{0,\}proxy-groups:/c\Proxy Group:" "$CFG_FILE" 2>/dev/null
   }
   
   #rules
   [ -z "$(grep "^Rule:" "$CFG_FILE")" ] && {
      sed -i "/^ \{1,\}Rule:/c\Rule:" "$CFG_FILE" 2>/dev/null
   }
   [ -z "$(grep "^Rule:" "$CFG_FILE")" ] && {
      sed -i "/^ \{0,\}rules:/c\Rule:" "$CFG_FILE" 2>/dev/null
   }
}

config_error()
{
   echo "配置文件【$name】下载失败，请检查网络或稍后再试！" >$START_LOG
   echo "${LOGTIME} Config 【$name】Update Error" >>$LOG_FILE
   rm -rf "$CFG_FILE" 2>/dev/null
   sleep 5
   echo "" >$START_LOG
}

change_dns()
{
   if pidof clash >/dev/null; then
      if [ "$enable_redirect_dns" -ne "0" ]; then
         uci del dhcp.@dnsmasq[-1].server >/dev/null 2>&1
         uci add_list dhcp.@dnsmasq[0].server=127.0.0.1#"$dns_port" >/dev/null 2>&1
         uci delete dhcp.@dnsmasq[0].resolvfile >/dev/null 2>&1
         uci set dhcp.@dnsmasq[0].noresolv=1 >/dev/null 2>&1
         [ "$disable_masq_cache" -eq "1" ] && {
            uci set dhcp.@dnsmasq[0].cachesize=0 >/dev/null 2>&1
         }
         uci commit dhcp
         /etc/init.d/dnsmasq restart >/dev/null 2>&1
      fi
      nohup /usr/share/openclash/openclash_watchdog.sh &
   fi
}

config_download_direct()
{
   if pidof clash >/dev/null; then
      echo "配置文件【$name】下载失败，尝试不使用代理下载配置文件..." >$START_LOG
      
      watchdog_pids=$(ps |grep openclash_watchdog.sh |grep -v grep |awk '{print $1}' 2>/dev/null)
      for watchdog_pid in $watchdog_pids; do
         kill -9 "$watchdog_pid" >/dev/null 2>&1
      done

      uci del_list dhcp.@dnsmasq[0].server=127.0.0.1#"$dns_port" >/dev/null 2>&1
      uci set dhcp.@dnsmasq[0].resolvfile=/tmp/resolv.conf.auto >/dev/null 2>&1
      uci set dhcp.@dnsmasq[0].noresolv=0 >/dev/null 2>&1
      uci delete dhcp.@dnsmasq[0].cachesize >/dev/null 2>&1
      uci commit dhcp
      /etc/init.d/dnsmasq restart >/dev/null 2>&1
      sleep 3

      config_download
      
      if [ "$?" -eq "0" ] && [ -s "$CFG_FILE" ]; then
      	 change_dns
         config_su_check
      else
         change_dns
         config_error
      fi
   else
      config_error
   fi
}

sub_info_get()
{
   local section="$1"
   config_get_bool "enabled" "$section" "enabled" "1"
   config_get "name" "$section" "name" ""
   config_get "type" "$section" "type" ""
   config_get "address" "$section" "address" ""
   config_get "keyword" "$section" "keyword" ""

   if [ "$enabled" = "0" ]; then
      return
   fi
   
   if [ -z "$address" ]; then
      return
   else
      subscribe_url="$address"
   fi
	
   if [ -z "$name" ]; then
      name="config"
      CONFIG_FILE="/etc/openclash/config/config.yaml"
      BACKPACK_FILE="/etc/openclash/backup/config.yaml"
   else
      CONFIG_FILE="/etc/openclash/config/$name.yaml"
      BACKPACK_FILE="/etc/openclash/backup/$name.yaml"
   fi
   
   URL_TYPE="$type"
   
   echo "开始更新配置文件【$name】..." >$START_LOG

   config_download

   if [ "$?" -eq "0" ] && [ -s "$CFG_FILE" ]; then
   	  config_encode
   	  grep "^ \{0,\}Proxy Group:" "$CFG_FILE" >/dev/null 2>&1 && grep "^ \{0,\}Rule:" "$CFG_FILE" >/dev/null 2>&1
      if [ "$?" -eq "0" ]; then
         grep "^ \{0,\}Proxy:" "$CFG_FILE" >/dev/null 2>&1 || grep "^ \{0,\}proxy-provider:" "$CFG_FILE" >/dev/null 2>&1
         if [ "$?" -eq "0" ]; then
            config_su_check
         else
            config_download_direct
         fi
      else
         config_download_direct
      fi
   else
      config_download_direct
   fi
}

#分别获取订阅信息进行处理
config_load "openclash"
config_foreach sub_info_get "config_subscribe"
uci delete openclash.config.config_update_path >/dev/null 2>&1
uci commit openclash
[ "$if_restart" == "1" ] && /etc/init.d/openclash restart >/dev/null 2>&1
