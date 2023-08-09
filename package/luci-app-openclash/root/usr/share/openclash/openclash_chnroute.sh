#!/bin/bash
. /usr/share/openclash/openclash_ps.sh
. /usr/share/openclash/log.sh

   FW4=$(command -v fw4)

   set_lock() {
      exec 879>"/tmp/lock/openclash_chn.lock" 2>/dev/null
      flock -x 879 2>/dev/null
   }

   del_lock() {
      flock -u 879 2>/dev/null
      rm -rf "/tmp/lock/openclash_chn.lock"
   }

   china_ip_route=$(uci -q get openclash.config.china_ip_route)
   china_ip6_route=$(uci -q get openclash.config.china_ip6_route)
   CHNR_CUSTOM_URL=$(uci -q get openclash.config.chnr_custom_url)
   CHNR6_CUSTOM_URL=$(uci -q get openclash.config.chnr6_custom_url)
   CNDOMAIN_CUSTOM_URL=$(uci -q get openclash.config.cndomain_custom_url)
   disable_udp_quic=$(uci -q get openclash.config.disable_udp_quic)
   small_flash_memory=$(uci -q get openclash.config.small_flash_memory)
   en_mode=$(uci -q get openclash.config.en_mode)
   LOG_FILE="/tmp/openclash.log"
   restart=0
   set_lock
   
   if [ "$small_flash_memory" != "1" ]; then
      chnr_path="/etc/openclash/china_ip_route.ipset"
      chnr6_path="/etc/openclash/china_ip6_route.ipset"
      cndomain_path="/etc/openclash/accelerated-domains.china.conf"
      mkdir -p /etc/openclash
   else
      chnr_path="/tmp/etc/openclash/china_ip_route.ipset"
      chnr6_path="/tmp/etc/openclash/china_ip6_route.ipset"
      cndomain_path="/tmp/etc/openclash/accelerated-domains.china.conf"
      mkdir -p /tmp/etc/openclash
   fi

   LOG_OUT "Start Downloading The Chnroute Cidr List..."
   if [ -z "$CHNR_CUSTOM_URL" ]; then
      if pidof clash >/dev/null; then
         curl -SsL --connect-timeout 30 -m 60 --speed-time 30 --speed-limit 1 --retry 2 https://ispip.clang.cn/all_cn.txt -o /tmp/china_ip_route.txt 2>&1 | awk -v time="$(date "+%Y-%m-%d %H:%M:%S")" -v file="/tmp/china_ip_route.txt" '{print time "【" file "】Download Failed:【"$0"】"}' >> "$LOG_FILE"
      fi
      if [ "${PIPESTATUS[0]}" != "0" ] || ! pidof clash >/dev/null; then
         curl -SsL --connect-timeout 30 -m 60 --speed-time 30 --speed-limit 1 --retry 2 https://ispip.clang.cn/all_cn_cidr.txt -o /tmp/china_ip_route.txt 2>&1 | awk -v time="$(date "+%Y-%m-%d %H:%M:%S")" -v file="/tmp/china_ip_route.txt" '{print time "【" file "】Download Failed:【"$0"】"}' >> "$LOG_FILE"
      fi
   else
      curl -SsL --connect-timeout 30 -m 60 --speed-time 30 --speed-limit 1 --retry 2 "$CHNR_CUSTOM_URL" -o /tmp/china_ip_route.txt 2>&1 | awk -v time="$(date "+%Y-%m-%d %H:%M:%S")" -v file="/tmp/china_ip_route.txt" '{print time "【" file "】Download Failed:【"$0"】"}' >> "$LOG_FILE"
   fi

   if [ "${PIPESTATUS[0]}" -eq 0 ] && [ -s "/tmp/china_ip_route.txt" ]; then
      LOG_OUT "Chnroute Cidr List Download Success, Check Updated..."
      #预处理
      if [ -n "$FW4" ]; then
         echo "define china_ip_route = {" >/tmp/china_ip_route.list
         awk '!/^$/&&!/^#/{printf("    %s,'" "'\n",$0)}' /tmp/china_ip_route.txt >>/tmp/china_ip_route.list
         echo "}" >>/tmp/china_ip_route.list
         echo "add set inet fw4 china_ip_route { type ipv4_addr; flags interval; auto-merge; }" >>/tmp/china_ip_route.list
         echo 'add element inet fw4 china_ip_route $china_ip_route' >>/tmp/china_ip_route.list
      else
         echo "create china_ip_route hash:net family inet hashsize 1024 maxelem 1000000" >/tmp/china_ip_route.list
         awk '!/^$/&&!/^#/{printf("add china_ip_route %s'" "'\n",$0)}' /tmp/china_ip_route.txt >>/tmp/china_ip_route.list
      fi
      cmp -s /tmp/china_ip_route.list "$chnr_path"
      if [ "$?" -ne "0" ]; then
         LOG_OUT "Chnroute Cidr List Has Been Updated, Starting To Replace The Old Version..."
         mv /tmp/china_ip_route.list "$chnr_path" >/dev/null 2>&1
         if [ "$china_ip_route" -eq 1 ] || [ "$disable_udp_quic" -eq 1 ]; then
            restart=1
         fi
         LOG_OUT "Chnroute Cidr List Update Successful!"
      else
         LOG_OUT "Updated Chnroute Cidr List No Change, Do Nothing..."
      fi
   else
      LOG_OUT "Chnroute Cidr List Update Error, Please Try Again Later..."
   fi
   
   #ipv6
   LOG_OUT "Start Downloading The Chnroute6 Cidr List..."
   if [ -z "$CHNR6_CUSTOM_URL" ]; then
      curl -SsL --connect-timeout 30 -m 60 --speed-time 30 --speed-limit 1 --retry 2 https://ispip.clang.cn/all_cn_ipv6.txt -o /tmp/china_ip6_route.txt 2>&1 | awk -v time="$(date "+%Y-%m-%d %H:%M:%S")" -v file="/tmp/china_ip6_route.txt" '{print time "【" file "】Download Failed:【"$0"】"}' >> "$LOG_FILE"
   else
      curl -SsL --connect-timeout 30 -m 60 --speed-time 30 --speed-limit 1 --retry 2 "$CHNR6_CUSTOM_URL" -o /tmp/china_ip6_route.txt 2>&1 | awk -v time="$(date "+%Y-%m-%d %H:%M:%S")" -v file="/tmp/china_ip6_route.txt" '{print time "【" file "】Download Failed:【"$0"】"}' >> "$LOG_FILE"
   fi
   if [ "${PIPESTATUS[0]}" -eq "0" ] && [ -s "/tmp/china_ip6_route.txt" ]; then
      LOG_OUT "Chnroute6 Cidr List Download Success, Check Updated..."
      #预处理
      if [ -n "$FW4" ]; then
         echo "define china_ip6_route = {" >/tmp/china_ip6_route.list
         awk '!/^$/&&!/^#/{printf("    %s,'" "'\n",$0)}' /tmp/china_ip6_route.txt >>/tmp/china_ip6_route.list
         echo "}" >>/tmp/china_ip6_route.list
         echo "add set inet fw4 china_ip6_route { type ipv6_addr; flags interval; auto-merge; }" >>/tmp/china_ip6_route.list
         echo 'add element inet fw4 china_ip6_route $china_ip6_route' >>/tmp/china_ip6_route.list
      else
         echo "create china_ip6_route hash:net family inet6 hashsize 1024 maxelem 1000000" >/tmp/china_ip6_route.list
         awk '!/^$/&&!/^#/{printf("add china_ip6_route %s'" "'\n",$0)}' /tmp/china_ip6_route.txt >>/tmp/china_ip6_route.list
      fi
      cmp -s /tmp/china_ip6_route.list "$chnr6_path"
      if [ "$?" -ne "0" ]; then
         LOG_OUT "Chnroute6 Cidr List Has Been Updated, Starting To Replace The Old Version..."
         mv /tmp/china_ip6_route.list "$chnr6_path" >/dev/null 2>&1
         if [ "$china_ip6_route" -eq 1 ] || [ "$disable_udp_quic" -eq 1 ]; then
            restart=1
         fi
         LOG_OUT "Chnroute6 Cidr List Update Successful!"
      else
         LOG_OUT "Updated Chnroute6 Cidr List No Change, Do Nothing..."
      fi
   else
      LOG_OUT "Chnroute6 Cidr List Update Error, Please Try Again Later..."
   fi

   #CN DOMAIN
   LOG_OUT "Start Downloading The CN Domains List..."
   if [ -n "$CNDOMAIN_CUSTOM_URL" ]; then
      curl -SsL --connect-timeout 30 -m 60 --speed-time 30 --speed-limit 1 --retry 2 "$CNDOMAIN_CUSTOM_URL" -o "/tmp/china_domains.list" 2>&1 | awk -v time="$(date "+%Y-%m-%d %H:%M:%S")" -v file="/tmp/china_domains.list" '{print time "【" file "】Download Failed:【"$0"】"}' >> "$LOG_FILE"
   else
      if [ "$github_address_mod" != "0" ]; then
         if [ "$github_address_mod" == "https://cdn.jsdelivr.net/" ] || [ "$github_address_mod" == "https://fastly.jsdelivr.net/" ] || [ "$github_address_mod" == "https://testingcf.jsdelivr.net/" ]; then
            curl -SsL --connect-timeout 30 -m 60 --speed-time 30 --speed-limit 1 --retry 2 "$github_address_mod"gh/felixonmars/dnsmasq-china-list@master/accelerated-domains.china.conf -o "/tmp/china_domains.list" 2>&1 | awk -v time="$(date "+%Y-%m-%d %H:%M:%S")" -v file="/tmp/china_domains.list" '{print time "【" file "】Download Failed:【"$0"】"}' >> "$LOG_FILE"
         elif [ "$github_address_mod" == "https://raw.fastgit.org/" ]; then
            curl -SsL --connect-timeout 30 -m 60 --speed-time 30 --speed-limit 1 --retry 2 https://raw.fastgit.org/felixonmars/dnsmasq-china-list/master/accelerated-domains.china.conf -o "/tmp/china_domains.list" 2>&1 | awk -v time="$(date "+%Y-%m-%d %H:%M:%S")" -v file="/tmp/china_domains.list" '{print time "【" file "】Download Failed:【"$0"】"}' >> "$LOG_FILE"
         else
            curl -SsL --connect-timeout 30 -m 60 --speed-time 30 --speed-limit 1 --retry 2 "$github_address_mod"https://raw.githubusercontent.com/felixonmars/dnsmasq-china-list/master/accelerated-domains.china.conf -o "/tmp/china_domains.list" 2>&1 | awk -v time="$(date "+%Y-%m-%d %H:%M:%S")" -v file="/tmp/china_domains.list" '{print time "【" file "】Download Failed:【"$0"】"}' >> "$LOG_FILE"
         fi
      else
         curl -SsL --connect-timeout 30 -m 60 --speed-time 30 --speed-limit 1 --retry 2 https://raw.githubusercontent.com/felixonmars/dnsmasq-china-list/master/accelerated-domains.china.conf -o "/tmp/china_domains.list" 2>&1 | awk -v time="$(date "+%Y-%m-%d %H:%M:%S")" -v file="/tmp/china_domains.list" '{print time "【" file "】Download Failed:【"$0"】"}' >> "$LOG_FILE"
      fi
   fi
   
   if [ "${PIPESTATUS[0]}" -eq "0" ] && [ -s "/tmp/china_domains.list" ] && [ -n "$(cat "/tmp/china_domains.list" |head -1 |grep "server=")" ]; then
      LOG_OUT "CN Domains List Download Success, Check Updated..."
      cmp -s /tmp/china_domains.list "$cndomain_path"
      if [ "$?" -ne "0" ]; then
         LOG_OUT "CN Domains List Has Been Updated, Starting To Replace The Old Version..."
         mv /tmp/china_domains.list "$cndomain_path" >/dev/null 2>&1
         if [ "$china_ip_route" -eq 1 ] && [ -z "$(echo "$en_mode" |grep "redir-host")" ]; then
            restart=1
         fi
         LOG_OUT "CN Domains List Update Successful!"
      else
         LOG_OUT "Updated CN Domains List No Change, Do Nothing..."
      fi
   else
      LOG_OUT "CN Domains List Update Error, Please Try Again Later..."
   fi

   if [ "$restart" -eq 1 ] && [ "$(unify_ps_prevent)" -eq 0 ] && [ "$(find /tmp/lock/ |grep -v "openclash.lock" |grep -c "openclash")" -le 1 ]; then
      /etc/init.d/openclash restart >/dev/null 2>&1 &
   elif [ "$restart" -eq 0 ] && [ "$(unify_ps_prevent)" -eq 0 ] && [ "$(find /tmp/lock/ |grep -v "openclash.lock" |grep -c "openclash")" -le 1 ] && [ "$(uci -q get openclash.config.restart)" -eq 1 ]; then
      /etc/init.d/openclash restart >/dev/null 2>&1 &
      uci -q set openclash.config.restart=0
      uci -q commit openclash
   elif [ "$restart" -eq 1 ] && [ "$(unify_ps_prevent)" -eq 0 ]; then
      uci -q set openclash.config.restart=1
      uci -q commit openclash
   fi
 
   rm -rf /tmp/china_ip*_route* >/dev/null 2>&1
   rm -rf /tmp/china_domains.list >/dev/null 2>&1
   SLOG_CLEAN
   del_lock