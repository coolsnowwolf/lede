#!/bin/sh
. /usr/share/openclash/openclash_ps.sh
. /lib/functions.sh
. /usr/share/openclash/ruby.sh

   status=$(unify_ps_status "openclash_rule.sh")
   [ "$status" -gt 3 ] && exit 0

   yml_other_rules_dl()
   {
   local section="$1"
   local enabled config
   config_get_bool "enabled" "$section" "enabled" "1"
   config_get "config" "$section" "config" ""
   
   if [ "$enabled" = "0" ] || [ "$config" != "$2" ]; then
      return
   fi
   
   if [ -n "$rule_name" ]; then
      echo "${LOGTIME} Warrning: Multiple Other-Rules-Configurations Enabled, Ignore..." >> $LOG_FILE
      return
   fi
   
   config_get "rule_name" "$section" "rule_name" ""
   
   echo "开始下载使用中的第三方规则..." >$START_LOG
   if [ "$rule_name" = "lhie1" ]; then
     if pidof clash >/dev/null; then
         curl -sL --connect-timeout 10 --retry 2 https://raw.githubusercontent.com/lhie1/Rules/master/Clash/Rule.yaml -o /tmp/rules.yaml >/dev/null 2>&1
     fi
     if [ "$?" -ne "0" ] || ! pidof clash >/dev/null; then
         curl -sL --connect-timeout 10 --retry 2 https://cdn.jsdelivr.net/gh/lhie1/Rules@master/Clash/Rule.yaml -o /tmp/rules.yaml >/dev/null 2>&1
      fi
      sed -i '1i rules:' /tmp/rules.yaml
   elif [ "$rule_name" = "ConnersHua" ]; then
      if pidof clash >/dev/null; then
         curl -sL --connect-timeout 10 --retry 2 https://raw.githubusercontent.com/DivineEngine/Profiles/master/Clash/Outbound.yaml -o /tmp/rules.yaml >/dev/null 2>&1
      fi
      if [ "$?" -ne "0" ] || ! pidof clash >/dev/null; then
         curl -sL --connect-timeout 10 --retry 2 https://cdn.jsdelivr.net/gh/DivineEngine/Profiles@master/Clash/Outbound.yaml -o /tmp/rules.yaml >/dev/null 2>&1
      fi
      sed -i "s/# - RULE-SET,ChinaIP,DIRECT/- RULE-SET,ChinaIP,DIRECT/g" /tmp/rules.yaml 2>/dev/null
      sed -i "s/- GEOIP,/#- GEOIP,/g" /tmp/rules.yaml 2>/dev/null
   elif [ "$rule_name" = "ConnersHua_return" ]; then
      if pidof clash >/dev/null; then
         curl -sL --connect-timeout 10 --retry 2 https://raw.githubusercontent.com/DivineEngine/Profiles/master/Clash/Inbound.yaml -o /tmp/rules.yaml >/dev/null 2>&1
      fi
      if [ "$?" -ne "0" ] || ! pidof clash >/dev/null; then
         curl -sL --connect-timeout 10 --retry 2 https://cdn.jsdelivr.net/gh/DivineEngine/Profiles@master/Clash/Inbound.yaml -o /tmp/rules.yaml >/dev/null 2>&1
      fi
   fi
   if [ "$?" -eq "0" ] && [ -s "/tmp/rules.yaml" ]; then
      echo "下载成功，开始预处理规则文件..." >$START_LOG
      ruby -ryaml -E UTF-8 -e "
      begin
      YAML.load_file('/tmp/rules.yaml');
      rescue Exception => e
      puts '${LOGTIME} Error: Unable To Parse Updated ${rule_name} Rules File ' + e.message
      system 'rm -rf /tmp/rules.yaml 2>/dev/null'
      end
      " 2>/dev/null >> $LOG_FILE
      if [ $? -ne 0 ]; then
         echo "${LOGTIME} Error: Ruby Works Abnormally, Please Check The Ruby Library Depends!" >> $LOG_FILE
         echo "Ruby依赖异常，无法校验配置文件，请确认ruby依赖工作正常后重试！" > $START_LOG
         rm -rf /tmp/rules.yaml >/dev/null 2>&1
         sleep 3
         exit 0
      elif [ ! -f "/tmp/rules.yaml" ]; then
         echo "$rule_name 规则文件格式校验失败，请稍后再试..." > $START_LOG
         rm -rf /tmp/rules.yaml >/dev/null 2>&1
         sleep 3
         exit 0
      elif ! "$(ruby_read "/tmp/rules.yaml" ".key?('rules')")" ; then
         echo "${LOGTIME} Error: Updated Others Rules 【$rule_name】 Has No Rules Field, Update Exit..." >> $LOG_FILE
         echo "$rule_name 规则文件规则部分校验失败，请稍后再试..." > $START_LOG
         rm -rf /tmp/rules.yaml >/dev/null 2>&1
         sleep 3
         exit 0
      fi
      #取出规则部分
      ruby_read "/tmp/rules.yaml" ".select {|x| 'rule-providers' == x or 'script' == x or 'rules' == x }.to_yaml" > "$OTHER_RULE_FILE"
      #合并
      cat "$OTHER_RULE_FILE" > "/tmp/rules.yaml" 2>/dev/null
      rm -rf /tmp/other_rule* 2>/dev/null
      
      echo "检查下载的规则文件是否有更新..." >$START_LOG
      cmp -s /usr/share/openclash/res/"$rule_name".yaml /tmp/rules.yaml
      if [ "$?" -ne "0" ]; then
         echo "检测到下载的规则文件有更新，开始替换..." >$START_LOG
         mv /tmp/rules.yaml /usr/share/openclash/res/"$rule_name".yaml >/dev/null 2>&1
         echo "${LOGTIME} Other Rules 【$rule_name】 Update Successful" >>$LOG_FILE
         ifrestart=1
      else
         echo "检测到下载的规则文件没有更新，停止继续操作..." >$START_LOG
         echo "${LOGTIME} Updated Other Rules 【$rule_name】 No Change, Do Nothing" >>$LOG_FILE
         sleep 5
      fi
   else
      echo "第三方规则下载失败，请检查网络或稍后再试！" >$START_LOG
      echo "${LOGTIME} Other Rules 【$rule_name】 Update Error" >>$LOG_FILE
      sleep 5
   fi
   }

   START_LOG="/tmp/openclash_start.log"
   LOGTIME=$(date "+%Y-%m-%d %H:%M:%S")
   LOG_FILE="/tmp/openclash.log"
   RUlE_SOURCE=$(uci get openclash.config.rule_source 2>/dev/null)
   
   if [ "$RUlE_SOURCE" = "0" ]; then
      echo "未启用第三方规则，更新程序终止！" >$START_LOG
      echo "${LOGTIME} Other Rules Not Enable, Update Stop" >>$LOG_FILE
      sleep 5
   else
      OTHER_RULE_FILE="/tmp/other_rule.yaml"
      CONFIG_FILE=$(uci get openclash.config.config_path 2>/dev/null)
      CONFIG_NAME=$(echo "$CONFIG_FILE" |awk -F '/' '{print $5}' 2>/dev/null)
      ifrestart=0
   
      if [ -z "$CONFIG_FILE" ]; then
         CONFIG_FILE="/etc/openclash/config/$(ls -lt /etc/openclash/config/ | grep -E '.yaml|.yml' | head -n 1 |awk '{print $9}')"
	       CONFIG_NAME=$(echo "$CONFIG_FILE" |awk -F '/' '{print $5}' 2>/dev/null)
      fi

      if [ -z "$CONFIG_NAME" ]; then
         CONFIG_FILE="/etc/openclash/config/config.yaml"
         CONFIG_NAME="config.yaml"
      fi
      
      config_load "openclash"
      config_foreach yml_other_rules_dl "other_rules" "$CONFIG_NAME"
      if [ -z "$rule_name" ]; then
      	echo "未找到第三方规则配置，更新程序终止！" >$START_LOG
        echo "${LOGTIME} Get Other Rules Settings Faild, Update Stop" >>$LOG_FILE
        sleep 5
      fi
      if [ "$ifrestart" -eq 1 ] && [ "$(unify_ps_prevent)" -eq 0 ]; then
         /etc/init.d/openclash restart >/dev/null 2>&1 &
      fi
   fi
   rm -rf /tmp/rules.yaml >/dev/null 2>&1
   echo "" >$START_LOG