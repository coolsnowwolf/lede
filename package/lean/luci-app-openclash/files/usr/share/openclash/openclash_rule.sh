#!/bin/sh
   status=$(ps|grep -c /usr/share/openclash/openclash_rule.sh)
   [ "$status" -gt 3 ] && exit 0
   
   START_LOG="/tmp/openclash_start.log"
   LOGTIME=$(date "+%Y-%m-%d %H:%M:%S")
   LOG_FILE="/tmp/openclash.log"
   echo "开始获取使用中的第三方规则名称..." >$START_LOG
   RUlE_SOURCE=$(uci get openclash.config.rule_source 2>/dev/null)
   HTTP_PORT=$(uci get openclash.config.http_port 2>/dev/null)
   PROXY_ADDR=$(uci get network.lan.ipaddr 2>/dev/null |awk -F '/' '{print $1}' 2>/dev/null)

   if [ -s "/tmp/openclash.auth" ]; then
	    PROXY_AUTH=$(cat /tmp/openclash.auth |awk -F '- ' '{print $2}' |sed -n '1p' 2>/dev/null)
   fi
   echo "开始下载使用中的第三方规则..." >$START_LOG
      if [ "$RUlE_SOURCE" = "lhie1" ]; then
      	 if pidof clash >/dev/null; then
            curl -sL --connect-timeout 10 --retry 2 -x http://$PROXY_ADDR:$HTTP_PORT -U "$PROXY_AUTH" https://raw.githubusercontent.com/lhie1/Rules/master/Clash/Rule.yaml -o /tmp/rules.yaml >/dev/null 2>&1
      	 else
            curl -sL --connect-timeout 10 --retry 2 https://raw.githubusercontent.com/lhie1/Rules/master/Clash/Rule.yaml -o /tmp/rules.yaml >/dev/null 2>&1
         fi
         sed -i '1i rules:' /tmp/rules.yaml
      elif [ "$RUlE_SOURCE" = "ConnersHua" ]; then
      	 if pidof clash >/dev/null; then
            curl -sL --connect-timeout 10 --retry 2 -x http://$PROXY_ADDR:$HTTP_PORT -U "$PROXY_AUTH" https://raw.githubusercontent.com/DivineEngine/Profiles/master/Clash/Global.yaml -o /tmp/rules.yaml >/dev/null 2>&1
      	 else
            curl -sL --connect-timeout 10 --retry 2 https://raw.githubusercontent.com/DivineEngine/Profiles/master/Clash/Global.yaml -o /tmp/rules.yaml >/dev/null 2>&1
         fi
         sed -i -n '/^rule-providers:/,$p' /tmp/rules.yaml 2>/dev/null
         sed -i "s/# - RULE-SET,ChinaIP,DIRECT/- RULE-SET,ChinaIP,DIRECT/g" /tmp/rules.yaml 2>/dev/null
         sed -i "s/- GEOIP,/#- GEOIP,/g" /tmp/rules.yaml 2>/dev/null
      elif [ "$RUlE_SOURCE" = "ConnersHua_return" ]; then
      	 if pidof clash >/dev/null; then
            curl -sL --connect-timeout 10 --retry 2 -x http://$PROXY_ADDR:$HTTP_PORT -U "$PROXY_AUTH" https://raw.githubusercontent.com/ConnersHua/Profiles/master/Clash/China.yaml -o /tmp/rules.yaml >/dev/null 2>&1
      	 else
            curl -sL --connect-timeout 10 --retry 2 https://raw.githubusercontent.com/ConnersHua/Profiles/master/Clash/China.yaml -o /tmp/rules.yaml >/dev/null 2>&1
         fi
         sed -i -n '/^rules:/,$p' /tmp/rules.yaml 2>/dev/null
      fi
   if [ "$?" -eq "0" ] && [ "$RUlE_SOURCE" != 0 ] && [ -s "/tmp/rules.yaml" ]; then
      echo "下载成功，开始预处理规则文件..." >$START_LOG
      sed -i "/^rules:/a\##source:${RUlE_SOURCE}" /tmp/rules.yaml >/dev/null 2>&1
      echo "检查下载的规则文件是否有更新..." >$START_LOG
      cmp -s /etc/openclash/"$RUlE_SOURCE".yaml /tmp/rules.yaml
      if [ "$?" -ne "0" ]; then
         echo "检测到下载的规则文件有更新，开始替换..." >$START_LOG
         mv /tmp/rules.yaml /etc/openclash/"$RUlE_SOURCE".yaml >/dev/null 2>&1
         sed -i '/^rules:/a\##updated' /etc/openclash/"$RUlE_SOURCE".yaml >/dev/null 2>&1
         echo "替换成功，重新加载 OpenClash 应用新规则..." >$START_LOG
         status=$(ps |grep -v openclash_watchdog |grep -c openclash.sh)
         while ( [ "$status" -gt 1 ] )
         do
            sleep 5
            status=$(ps |grep -v openclash_watchdog |grep -c openclash.sh)
         done
         /etc/init.d/openclash restart 2>/dev/null
         echo "${LOGTIME} Other Rules 【$RUlE_SOURCE】 Update Successful" >>$LOG_FILE
      else
         echo "检测到下载的规则文件没有更新，停止继续操作..." >$START_LOG
         rm -rf /tmp/rules.yaml >/dev/null 2>&1
         echo "${LOGTIME} Updated Other Rules 【$RUlE_SOURCE】 No Change, Do Nothing" >>$LOG_FILE
         sleep 10
         echo "" >$START_LOG
      fi
   elif [ "$RUlE_SOURCE" = 0 ]; then
      echo "未启用第三方规则，更新程序终止！" >$START_LOG
      rm -rf /tmp/rules.yaml >/dev/null 2>&1
      echo "${LOGTIME} Other Rules 【$RUlE_SOURCE】 Not Enable, Update Stop" >>$LOG_FILE
      sleep 10
      echo "" >$START_LOG
   else
      echo "第三方规则下载失败，请检查网络或稍后再试！" >$START_LOG
      rm -rf /tmp/rules.yaml >/dev/null 2>&1
      echo "${LOGTIME} Other Rules 【$RUlE_SOURCE】 Update Error" >>$LOG_FILE
      sleep 10
      echo "" >$START_LOG
   fi
