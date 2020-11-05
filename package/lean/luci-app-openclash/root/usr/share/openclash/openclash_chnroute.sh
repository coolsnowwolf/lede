#!/bin/sh
. /usr/share/openclash/openclash_ps.sh

   status=$(unify_ps_status "openclash_chnroute.sh")
   [ "$status" -gt 3 ] && exit 0

   START_LOG="/tmp/openclash_start.log"
   LOGTIME=$(date "+%Y-%m-%d %H:%M:%S")
   LOG_FILE="/tmp/openclash.log"
   china_ip_route=$(uci get openclash.config.china_ip_route 2>/dev/null)

   echo "开始下载大陆IP白名单..." >$START_LOG
   if pidof clash >/dev/null; then
      curl -sL --connect-timeout 10 --retry 2 https://raw.githubusercontent.com/DivineEngine/Profiles/master/Clash/RuleSet/Extra/ChinaIP.yaml -o /tmp/ChinaIP.yaml >/dev/null 2>&1
   fi
   if [ "$?" -ne "0" ] || ! pidof clash >/dev/null; then
      curl -sL --connect-timeout 10 --retry 2 https://cdn.jsdelivr.net/gh/DivineEngine/Profiles@master/Clash/RuleSet/Extra/ChinaIP.yaml -o /tmp/ChinaIP.yaml >/dev/null 2>&1
   fi
   if [ "$?" -eq "0" ] && [ -s "/tmp/ChinaIP.yaml" ]; then
      echo "大陆IP白名单下载成功，检查版本是否更新..." >$START_LOG
      cmp -s /tmp/ChinaIP.yaml /etc/openclash/rule_provider/ChinaIP.yaml
         if [ "$?" -ne "0" ]; then
            echo "大陆IP白名单有更新，开始替换旧版本..." >$START_LOG
            mv /tmp/ChinaIP.yaml /etc/openclash/rule_provider/ChinaIP.yaml >/dev/null 2>&1
            echo "删除下载缓存..." >$START_LOG
            rm -rf /tmp/ChinaIP.yaml >/dev/null 2>&1
            rm -rf /usr/share/openclash/res/china_ip_route.ipset >/dev/null 2>&1
            [ "$china_ip_route" -eq 1 ] && [ "$(unify_ps_prevent)" -eq 0 ] && /etc/init.d/openclash restart
            echo "大陆IP白名单更新成功！" >$START_LOG
            echo "${LOGTIME} Chnroute Lists Update Successful" >>$LOG_FILE
            sleep 5
         else
            echo "大陆IP白名单没有更新，停止继续操作..." >$START_LOG
            echo "${LOGTIME} Updated Chnroute Lists No Change, Do Nothing" >>$LOG_FILE
            rm -rf /tmp/ChinaIP.yaml >/dev/null 2>&1
            sleep 5
         fi
   else
      echo "大陆IP白名单下载失败，请检查网络或稍后再试！" >$START_LOG
      rm -rf /tmp/ChinaIP.yaml >/dev/null 2>&1
      echo "${LOGTIME} Chnroute Lists Update Error" >>$LOG_FILE
      sleep 10
   fi
   echo "" >$START_LOG