#!/bin/sh
. /usr/share/openclash/openclash_ps.sh
. /usr/share/openclash/log.sh

   set_lock() {
      exec 879>"/tmp/lock/openclash_chn.lock" 2>/dev/null
      flock -x 879 2>/dev/null
   }

   del_lock() {
      flock -u 879 2>/dev/null
      rm -rf "/tmp/lock/openclash_chn.lock"
   }

   china_ip_route=$(uci get openclash.config.china_ip_route 2>/dev/null)
   china_ip6_route=$(uci get openclash.config.china_ip_route 2>/dev/null)
   CHNR_CUSTOM_URL=$(uci get openclash.config.chnr_custom_url 2>/dev/null)
   CHNR6_CUSTOM_URL=$(uci get openclash.config.chnr6_custom_url 2>/dev/null)
   small_flash_memory=$(uci get openclash.config.small_flash_memory 2>/dev/null)
   set_lock
   
   if [ "$small_flash_memory" != "1" ]; then
   	  chnr_path="/etc/openclash/china_ip_route.ipset"
   	  chnr6_path="/etc/openclash/china_ip6_route.ipset"
   	  mkdir -p /etc/openclash
   else
   	  chnr_path="/tmp/etc/openclash/china_ip_route.ipset"
   	  chnr6_path="/tmp/etc/openclash/china_ip6_route.ipset"
   	  mkdir -p /tmp/etc/openclash
   fi

   LOG_OUT "Start Downloading The Chnroute Cidr List..."
   if [ -z "$CHNR_CUSTOM_URL" ]; then
      if pidof clash >/dev/null; then
         curl -sL --connect-timeout 10 --retry 2 https://ispip.clang.cn/all_cn.txt -o /tmp/china_ip_route.txt >/dev/null 2>&1
      fi
      if [ "$?" -ne "0" ] || ! pidof clash >/dev/null; then
         curl -sL --connect-timeout 10 --retry 2 https://ispip.clang.cn/all_cn_cidr.txt -o /tmp/china_ip_route.txt >/dev/null 2>&1
      fi
   else
      curl -sL --connect-timeout 10 --retry 2 "$CHNR_CUSTOM_URL" -o /tmp/china_ip_route.txt >/dev/null 2>&1
   fi
   if [ "$?" -eq "0" ] && [ -s "/tmp/china_ip_route.txt" ]; then
      LOG_OUT "Chnroute Cidr List Download Success, Check Updated..."
      #预处理
      echo "create china_ip_route hash:net family inet hashsize 1024 maxelem 1000000" >/tmp/china_ip_route.list
      awk '!/^$/&&!/^#/{printf("add china_ip_route %s'" "'\n",$0)}' /tmp/china_ip_route.txt >>/tmp/china_ip_route.list
      cmp -s /tmp/china_ip_route.list "$chnr_path"
      if [ "$?" -ne "0" ]; then
         LOG_OUT "Chnroute Cidr List Has Been Updated, Starting To Replace The Old Version..."
         mv /tmp/china_ip_route.list "$chnr_path" >/dev/null 2>&1
         [ "$china_ip_route" -eq 1 ] && [ "$(unify_ps_prevent)" -eq 0 ] && /etc/init.d/openclash restart >/dev/null 2>&1 &
         LOG_OUT "Chnroute Cidr List Update Successful!"
         sleep 5
      else
         LOG_OUT "Updated Chnroute Cidr List No Change, Do Nothing..."
         sleep 5
      fi
   else
      LOG_OUT "Chnroute Cidr List Update Error, Please Try Again Later..."
      sleep 5
   fi
   
   #ipv6
   LOG_OUT "Start Downloading The Chnroute6 Cidr List..."
   if [ -z "$CHNR6_CUSTOM_URL" ]; then
      curl -sL --connect-timeout 10 --retry 2 https://ispip.clang.cn/all_cn_ipv6.txt -o /tmp/china_ip6_route.txt >/dev/null 2>&1
   else
      curl -sL --connect-timeout 10 --retry 2 "$CHNR6_CUSTOM_URL" -o /tmp/china_ip6_route.txt >/dev/null 2>&1
   fi
   if [ "$?" -eq "0" ] && [ -s "/tmp/china_ip6_route.txt" ]; then
      LOG_OUT "Chnroute6 Cidr List Download Success, Check Updated..."
      #预处理
      echo "create china_ip6_route hash:net family inet6 hashsize 1024 maxelem 1000000" >/tmp/china_ip6_route.list
      awk '!/^$/&&!/^#/{printf("add china_ip6_route %s'" "'\n",$0)}' /tmp/china_ip6_route.txt >>/tmp/china_ip6_route.list
      cmp -s /tmp/china_ip6_route.list "$chnr6_path"
      if [ "$?" -ne "0" ]; then
         LOG_OUT "Chnroute6 Cidr List Has Been Updated, Starting To Replace The Old Version..."
         mv /tmp/china_ip6_route.list "$chnr6_path" >/dev/null 2>&1
         [ "$china_ip6_route" -eq 1 ] && [ "$(unify_ps_prevent)" -eq 0 ] && /etc/init.d/openclash restart >/dev/null 2>&1 &
         LOG_OUT "Chnroute6 Cidr List Update Successful!"
         sleep 5
      else
         LOG_OUT "Updated Chnroute6 Cidr List No Change, Do Nothing..."
         sleep 5
      fi
   else
      LOG_OUT "Chnroute6 Cidr List Update Error, Please Try Again Later..."
      sleep 5
   fi
   rm -rf /tmp/china_ip*_route* >/dev/null 2>&1
   SLOG_CLEAN
   del_lock
