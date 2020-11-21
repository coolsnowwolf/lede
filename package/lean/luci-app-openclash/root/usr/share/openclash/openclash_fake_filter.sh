#!/bin/bash
. /lib/functions.sh

START_LOG="/tmp/openclash_start.log"
CUSTOM_FILE="/etc/openclash/custom/openclash_custom_fake_filter.list"
FAKE_FILTER_FILE="/tmp/openclash_fake_filter.list"
SER_FAKE_FILTER_FILE="/tmp/openclash_servers_fake_filter.conf"

echo "正在设置Fake-IP黑名单..." >$START_LOG

rm -rf "$FAKE_FILTER_FILE" 2>/dev/null
if [ -s "$CUSTOM_FILE" ]; then
   cat "$CUSTOM_FILE" |while read -r line || [[ -n ${line} ]];
   do
      if [ -z "$(echo $line |grep '^ \{0,\}#' 2>/dev/null)" ]; then
         echo "  - '$line'" >> "$FAKE_FILTER_FILE"
      else
         continue
	    fi
   done 2>/dev/null
   if [ -s "$FAKE_FILTER_FILE" ]; then
      sed -i '1i\fake-ip-filter:' "$FAKE_FILTER_FILE"
   else
      rm -rf "$FAKE_FILTER_FILE" 2>/dev/null
   fi
fi

cfg_server_address()
{
	 local section="$1"
   config_get "server" "$section" "server" ""
   
   IFIP=$(echo $server |grep -E "^[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}$" 2>/dev/null)
   if [ -z "$IFIP" ] && [ ! -z "$server" ] && [ -z "$(grep "/$server/" "$SER_FAKE_FILTER_FILE" 2>/dev/null)" ]; then
      echo "server=/$server/$custom_domain_dns_server" >> "$SER_FAKE_FILTER_FILE"
   else
      return
   fi
}

#Fake下正确检测节点延迟及获取真实地址

rm -rf "$SER_FAKE_FILTER_FILE" 2>/dev/null
custom_domain_dns_server=$(uci get openclash.config.custom_domain_dns_server 2>/dev/null)
   [ -z "$custom_domain_dns_server" ] && {
	   custom_domain_dns_server="114.114.114.114"
	 }
config_load "openclash"
config_foreach cfg_server_address "servers"