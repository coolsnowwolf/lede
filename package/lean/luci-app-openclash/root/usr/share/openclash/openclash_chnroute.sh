#!/bin/sh
. /usr/share/openclash/openclash_ps.sh

   status=$(unify_ps_status "openclash_chnroute.sh")
   [ "$status" -gt 3 ] && exit 0

   START_LOG="/tmp/openclash_start.log"
   LOGTIME=$(date "+%Y-%m-%d %H:%M:%S")
   LOG_FILE="/tmp/openclash.log"
   china_ip_route=$(uci get openclash.config.china_ip_route 2>/dev/null)
   CHNR_CUSTOM_URL=$(uci get openclash.config.chnr_custom_url 2>/dev/null)
   small_flash_memory=$(uci get openclash.config.small_flash_memory 2>/dev/null)
   
   if [ "$small_flash_memory" != "1" ]; then
   	  chnr_path="/etc/openclash/china_ip_route.ipset"
   	  mkdir -p /etc/openclash
   else
   	  chnr_path="/tmp/etc/openclash/china_ip_route.ipset"
   	  mkdir -p /tmp/etc/openclash
   fi

   echo "开始下载大陆IP白名单..." >$START_LOG
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
      echo "大陆IP白名单下载成功，检查版本是否更新..." >$START_LOG
      #预处理
      echo "create china_ip_route hash:net family inet hashsize 1024 maxelem 65536" >/tmp/china_ip_route.list
      awk '!/^$/&&!/^#/{printf("add china_ip_route %s'" "'\n",$0)}' /tmp/china_ip_route.txt >>/tmp/china_ip_route.list
      cmp -s /tmp/china_ip_route.list "$chnr_path"
      if [ "$?" -ne "0" ]; then
         echo "大陆IP白名单有更新，开始替换旧版本..." >$START_LOG
         mv /tmp/china_ip_route.list "$chnr_path" >/dev/null 2>&1
         [ "$china_ip_route" -eq 1 ] && [ "$(unify_ps_prevent)" -eq 0 ] && /etc/init.d/openclash restart >/dev/null 2>&1 &
         echo "大陆IP白名单更新成功！" >$START_LOG
         echo "${LOGTIME} Chnroute Lists Update Successful" >>$LOG_FILE
         sleep 5
      else
         echo "大陆IP白名单没有更新，停止继续操作..." >$START_LOG
         echo "${LOGTIME} Updated Chnroute Lists No Change, Do Nothing" >>$LOG_FILE
         sleep 5
      fi
   else
      echo "大陆IP白名单下载失败，请检查网络或稍后再试！" >$START_LOG
      echo "${LOGTIME} Chnroute Lists Update Error" >>$LOG_FILE
      sleep 5
   fi
   rm -rf /tmp/china_ip_route* >/dev/null 2>&1
   echo "" >$START_LOG