#!/bin/sh
. /usr/share/openclash/openclash_ps.sh
. /lib/functions.sh
. /usr/share/openclash/ruby.sh
. /usr/share/openclash/log.sh

   set_lock() {
      exec 877>"/tmp/lock/openclash_rule.lock" 2>/dev/null
      flock -x 877 2>/dev/null
   }

   del_lock() {
      flock -u 877 2>/dev/null
      rm -rf "/tmp/lock/openclash_rule.lock"
   }

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
      LOG_OUT "Warrning: Multiple Other-Rules-Configurations Enabled, Ignore..."
      return
   fi
   
   config_get "rule_name" "$section" "rule_name" ""
   
   LOG_OUT "Start Downloading Third Party Rules in Use..."
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
      LOG_OUT "Download Successful, Start Preprocessing Rule File..."
      ruby -ryaml -E UTF-8 -e "
      begin
      YAML.load_file('/tmp/rules.yaml');
      rescue Exception => e
      puts '${LOGTIME} Error: Unable To Parse Updated Rules File,【${rule_name}:' + e.message + '】'
      system 'rm -rf /tmp/rules.yaml 2>/dev/null'
      end
      " 2>/dev/null >> $LOG_FILE
      if [ $? -ne 0 ]; then
         LOG_OUT "Error: Ruby Works Abnormally, Please Check The Ruby Library Depends!"
         rm -rf /tmp/rules.yaml >/dev/null 2>&1
         sleep 3
         SLOG_CLEAN
         del_lock
         exit 0
      elif [ ! -f "/tmp/rules.yaml" ]; then
         LOG_OUT "Error:【$rule_name】Rule File Format Validation Failed, Please Try Again Later..."
         rm -rf /tmp/rules.yaml >/dev/null 2>&1
         sleep 3
         SLOG_CLEAN
         del_lock
         exit 0
      elif ! "$(ruby_read "/tmp/rules.yaml" ".key?('rules')")" ; then
         LOG_OUT "Error: Updated Others Rules【$rule_name】Has No Rules Field, Update Exit..."
         rm -rf /tmp/rules.yaml >/dev/null 2>&1
         sleep 3
         SLOG_CLEAN
         del_lock
         exit 0
      #校验是否含有新策略组
      elif ! "$(ruby -ryaml -E UTF-8 -e "
         Value = YAML.load_file('/usr/share/openclash/res/${rule_name}.yaml');
         Value_1 = YAML.load_file('/tmp/rules.yaml');
         OLD_GROUP = Value['rules'].collect{|x| x.split(',')[2] or x.split(',')[1]}.uniq;
         NEW_GROUP = Value_1['rules'].collect{|x| x.split(',')[2] or x.split(',')[1]}.uniq;
         puts (OLD_GROUP | NEW_GROUP).eql?(OLD_GROUP)
         ")" ; then
         LOG_OUT "Error: Updated Others Rules【$rule_name】Has Incompatible Proxy-Group, Update Exit, Please Wait For OpenClash Update To Adapt..."
         rm -rf /tmp/rules.yaml >/dev/null 2>&1
         sleep 3
         SLOG_CLEAN
         del_lock
         exit 0
      fi
      
      #取出规则部分
      ruby_read "/tmp/rules.yaml" ".select {|x| 'rule-providers' == x or 'script' == x or 'rules' == x }.to_yaml" > "$OTHER_RULE_FILE"
      #合并
      cat "$OTHER_RULE_FILE" > "/tmp/rules.yaml" 2>/dev/null
      rm -rf /tmp/other_rule* 2>/dev/null
      
      LOG_OUT "Check The Downloaded Rule File For Updates..."
      cmp -s /usr/share/openclash/res/"$rule_name".yaml /tmp/rules.yaml
      if [ "$?" -ne "0" ]; then
         LOG_OUT "Detected that The Downloaded Rule File Has Been Updated, Starting To Replace..."
         mv /tmp/rules.yaml /usr/share/openclash/res/"$rule_name".yaml >/dev/null 2>&1
         LOG_OUT "Other Rules【$rule_name】Update Successful!"
         ifrestart=1
      else
         LOG_OUT "Updated Other Rules【$rule_name】No Change, Do Nothing!"
         sleep 5
      fi
   else
      LOG_OUT "Other Rules【$rule_name】Update Error, Please Try Again Later..."
      sleep 5
   fi
   }
   
   LOGTIME=$(echo $(date "+%Y-%m-%d %H:%M:%S"))
   LOG_FILE="/tmp/openclash.log"
   RUlE_SOURCE=$(uci get openclash.config.rule_source 2>/dev/null)
   set_lock
   
   if [ "$RUlE_SOURCE" = "0" ]; then
      LOG_OUT "Other Rules Not Enable, Update Stop!"
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
        LOG_OUT "Get Other Rules Settings Faild, Update Stop!"
        sleep 5
      fi
      if [ "$ifrestart" -eq 1 ] && [ "$(unify_ps_prevent)" -eq 0 ]; then
         /etc/init.d/openclash restart >/dev/null 2>&1 &
      fi
   fi
   rm -rf /tmp/rules.yaml >/dev/null 2>&1
   SLOG_CLEAN
   del_lock