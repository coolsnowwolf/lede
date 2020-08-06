#!/bin/bash
. /lib/functions.sh

status=$(ps|grep -c /usr/share/openclash/openclash.sh)
[ "$status" -gt 3 ] && exit 0

START_LOG="/tmp/openclash_start.log"
LOGTIME=$(date "+%Y-%m-%d %H:%M:%S")
LOG_FILE="/tmp/openclash.log"
CFG_FILE="/tmp/config.yaml"
CRON_FILE="/etc/crontabs/root"
CONFIG_PATH=$(uci get openclash.config.config_path 2>/dev/null)
servers_update=$(uci get openclash.config.servers_update 2>/dev/null)
dns_port=$(uci get openclash.config.dns_port 2>/dev/null)
enable_redirect_dns=$(uci get openclash.config.enable_redirect_dns 2>/dev/null)
disable_masq_cache=$(uci get openclash.config.disable_masq_cache 2>/dev/null)
if_restart=0

urlencode() {
    local data
    if [ "$#" -eq 1 ]; then
       data=$(curl -s -o /dev/null -w %{url_effective} --get --data-urlencode "$1" "")
       if [ ! -z "$data" ]; then
           echo "${data##/?}"
       fi
    fi
}

kill_watchdog() {
   watchdog_pids=$(ps |grep openclash_watchdog.sh |grep -v grep |awk '{print $1}' 2>/dev/null)
      for watchdog_pid in $watchdog_pids; do
         kill -9 "$watchdog_pid" >/dev/null 2>&1
      done
}

config_download()
{
if [ "$URL_TYPE" == "v2rayn" ]; then
   subscribe_url=$(urlencode "$subscribe_url")
   curl -sL --connect-timeout 10 --retry 2 https://tgbot.lbyczf.com/v2rayn2clash?url="$subscribe_url" -o "$CFG_FILE" >/dev/null 2>&1
elif [ "$URL_TYPE" == "surge" ]; then
   subscribe_url=$(urlencode "$subscribe_url")
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
	if [ "$servers_update" -eq 1 ] || [ ! -z "$keyword" ] || [ ! -z "$ex_keyword" ]; then
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
  rm -rf /tmp/Proxy_Group 2>/dev/null
}

config_su_check()
{
   echo "配置文件下载成功，检查是否有更新..." >$START_LOG
   if [ -f "$CONFIG_FILE" ]; then
      cmp -s "$BACKPACK_FILE" "$CFG_FILE"
         if [ "$?" -ne 0 ]; then
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
   /usr/share/openclash/yml_field_name_ch.sh "$CFG_FILE"
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
      if [ "$enable_redirect_dns" -ne 0 ]; then
         uci del dhcp.@dnsmasq[-1].server >/dev/null 2>&1
         uci add_list dhcp.@dnsmasq[0].server=127.0.0.1#"$dns_port" >/dev/null 2>&1
         uci delete dhcp.@dnsmasq[0].resolvfile >/dev/null 2>&1
         uci set dhcp.@dnsmasq[0].noresolv=1 >/dev/null 2>&1
         [ "$disable_masq_cache" -eq 1 ] && {
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
      
      kill_watchdog

      uci del_list dhcp.@dnsmasq[0].server=127.0.0.1#"$dns_port" >/dev/null 2>&1
      uci set dhcp.@dnsmasq[0].resolvfile=/tmp/resolv.conf.auto >/dev/null 2>&1
      uci set dhcp.@dnsmasq[0].noresolv=0 >/dev/null 2>&1
      uci delete dhcp.@dnsmasq[0].cachesize >/dev/null 2>&1
      uci commit dhcp
      /etc/init.d/dnsmasq restart >/dev/null 2>&1
      sleep 3

      config_download
      
      if [ "$?" -eq 0 ] && [ -s "$CFG_FILE" ]; then
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
   config_get "ex_keyword" "$section" "ex_keyword" ""

   if [ "$enabled" -eq 0 ]; then
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

   if [ "$?" -eq 0 ] && [ -s "$CFG_FILE" ]; then
   	  config_encode
   	  if [ -n "$(grep "^ \{0,\}proxy-groups:" "$CFG_FILE")" ]; then
         if [ -n "$(grep "^ \{0,\}Proxy:" "$CFG_FILE" 2>/dev/null)" ] || [ -n "$(grep "^ \{0,\}proxy-providers:" "$CFG_FILE" 2>/dev/null)" ]; then
            if [ -n "$(grep "^ \{0,\}rules:" "$CFG_FILE" 2>/dev/null)" ] || [ -n "$(grep "^ \{0,\}script:" "$CFG_FILE" 2>/dev/null)" ]; then
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
   else
      config_download_direct
   fi
}

#分别获取订阅信息进行处理
config_load "openclash"
config_foreach sub_info_get "config_subscribe"
uci delete openclash.config.config_update_path >/dev/null 2>&1
uci commit openclash

if [ "$if_restart" -eq 1 ]; then
   /etc/init.d/openclash restart >/dev/null 2>&1
else
   sed -i '/openclash.sh/d' $CRON_FILE 2>/dev/null
   [ "$(uci get openclash.config.auto_update 2>/dev/null)" -eq 1 ] && [ "$(uci get openclash.config.config_auto_update_mode 2>/dev/null)" -ne 1 ] && echo "0 $(uci get openclash.config.auto_update_time 2>/dev/null) * * $(uci get openclash.config.config_update_week_time 2>/dev/null) /usr/share/openclash/openclash.sh" >> $CRON_FILE
   /etc/init.d/cron restart
fi
