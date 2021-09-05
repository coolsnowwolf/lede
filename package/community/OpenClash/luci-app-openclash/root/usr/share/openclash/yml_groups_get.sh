#!/bin/bash
. /lib/functions.sh
. /usr/share/openclash/ruby.sh
. /usr/share/openclash/log.sh

set_lock() {
   exec 876>"/tmp/lock/openclash_groups_get.lock" 2>/dev/null
   flock -x 876 2>/dev/null
}

del_lock() {
   flock -u 876 2>/dev/null
   rm -rf "/tmp/lock/openclash_groups_get.lock"
}
   

CFG_FILE="/etc/config/openclash"
other_group_file="/tmp/yaml_other_group.yaml"
servers_update=$(uci get openclash.config.servers_update 2>/dev/null)
servers_if_update=$(uci get openclash.config.servers_if_update 2>/dev/null)
CONFIG_FILE=$(uci get openclash.config.config_path 2>/dev/null)
CONFIG_NAME=$(echo "$CONFIG_FILE" |awk -F '/' '{print $5}' 2>/dev/null)
UPDATE_CONFIG_FILE=$(uci get openclash.config.config_update_path 2>/dev/null)
UPDATE_CONFIG_NAME=$(echo "$UPDATE_CONFIG_FILE" |awk -F '/' '{print $5}' 2>/dev/null)
LOGTIME=$(echo $(date "+%Y-%m-%d %H:%M:%S"))
LOG_FILE="/tmp/openclash.log"
set_lock

if [ ! -z "$UPDATE_CONFIG_FILE" ]; then
   CONFIG_FILE="$UPDATE_CONFIG_FILE"
   CONFIG_NAME="$UPDATE_CONFIG_NAME"
fi

if [ -z "$CONFIG_FILE" ]; then
	CONFIG_FILE="/etc/openclash/config/$(ls -lt /etc/openclash/config/ | grep -E '.yaml|.yml' | head -n 1 |awk '{print $9}')"
	CONFIG_NAME=$(echo "$CONFIG_FILE" |awk -F '/' '{print $5}' 2>/dev/null)
fi

if [ -z "$CONFIG_NAME" ]; then
   CONFIG_FILE="/etc/openclash/config/config.yaml"
   CONFIG_NAME="config.yaml"
fi

BACKUP_FILE="/etc/openclash/backup/$(echo "$CONFIG_FILE" |awk -F '/' '{print $5}' 2>/dev/null)"

if [ ! -s "$CONFIG_FILE" ] && [ ! -s "$BACKUP_FILE" ]; then
   del_lock
   exit 0
elif [ ! -s "$CONFIG_FILE" ] && [ -s "$BACKUP_FILE" ]; then
   mv "$BACKUP_FILE" "$CONFIG_FILE"
fi

LOG_OUT "Start Getting【$CONFIG_NAME】Groups Setting..."

/usr/share/openclash/yml_groups_name_get.sh
if [ $? -ne 0 ]; then
	LOG_OUT "Read Error, Config File【$CONFIG_NAME】Abnormal!"
	uci commit openclash
	sleep 5
	SLOG_CLEAN
	del_lock
	exit 0
fi

#判断当前配置文件是否有策略组信息
cfg_group_name()
{
   local section="$1"
   config_get "config" "$section" "config" ""

   if [ -z "$config" ]; then
      return
   fi

   [ "$config" = "$CONFIG_NAME" ] && {
      config_group_exist=1
   }
}

#删除不必要的配置
cfg_delete()
{
   LOG_OUT "Deleting Old Configuration..."
#删除策略组
   group_num=$(grep "config groups" "$CFG_FILE" |wc -l)
   for ((i=$group_num;i>=0;i--))
	 do
	    if [ "$(uci get openclash.@groups["$i"].config 2>/dev/null)" = "$CONFIG_NAME" ] || [ "$(uci get openclash.@groups["$i"].config 2>/dev/null)" = "all" ]; then
	       uci delete openclash.@groups["$i"] 2>/dev/null
	       uci commit openclash
	    fi
	 done
#删除启用的节点
   server_num=$(grep "config servers" "$CFG_FILE" |wc -l)
   for ((i=$server_num;i>=0;i--))
	 do
	    if [ "$(uci get openclash.@servers["$i"].config 2>/dev/null)" = "$CONFIG_NAME" ] || [ "$(uci get openclash.@servers["$i"].config 2>/dev/null)" = "all" ]; then
	    	 if [ "$(uci get openclash.@servers["$i"].enabled 2>/dev/null)" = "1" ]; then
	          uci delete openclash.@servers["$i"] 2>/dev/null
	          uci commit openclash
	       fi
	    fi
	 done
#删除启用的代理集
   provider_num=$(grep "config proxy-provider" "$CFG_FILE" 2>/dev/null |wc -l)
   for ((i=$provider_num;i>=0;i--))
	 do
	    if [ "$(uci get openclash.@proxy-provider["$i"].config 2>/dev/null)" = "$CONFIG_NAME" ] || [ "$(uci get openclash.@proxy-provider["$i"].config 2>/dev/null)" = "all" ]; then
	       if [ "$(uci get openclash.@proxy-provider["$i"].enabled)" = "1" ]; then
	          uci delete openclash.@proxy-provider["$i"] 2>/dev/null
	          uci commit openclash
	       fi
	    fi
	 done
}

config_load "openclash"
config_foreach cfg_group_name "groups"

if [ "$servers_if_update" -eq 1 ] && [ "$servers_update" -eq 1 ] && [ "$config_group_exist" -eq 1 ]; then
   /usr/share/openclash/yml_proxys_get.sh
   del_lock
   exit 0
else
   cfg_delete
fi

count=0
match_group_file="/tmp/Proxy_Group"
#提取策略组部分
group_hash=$(ruby_read "$CONFIG_FILE" ".select {|x| 'proxy-groups' == x}")
num=$(ruby_read_hash "$group_hash" "['proxy-groups'].count")

if [ -z "$num" ]; then
   LOG_OUT "Error: Unable To Parse Config File, Please Check And Try Again!"
   sleep 3
   del_lock
   exit 0
fi

while [ "$count" -lt "$num" ]
do

   #type
   group_type=$(ruby_read_hash "$group_hash" "['proxy-groups'][$count]['type']")
   #name
   group_name=$(ruby_read_hash "$group_hash" "['proxy-groups'][$count]['name']")

   if [ -z "$group_type" ] || [ -z "$group_name" ]; then
      let count++
      continue
   fi
   
   LOG_OUT "Start Getting【$CONFIG_NAME - $group_type - $group_name】Group Setting..."
   
   name=openclash
   uci_name_tmp=$(uci add $name groups)
   uci_set="uci -q set $name.$uci_name_tmp."
   uci_add="uci -q add_list $name.$uci_name_tmp."
   
   ${uci_set}config="$CONFIG_NAME"
   ${uci_set}name="$group_name"
   ${uci_set}old_name="$group_name"
   ${uci_set}old_name_cfg="$group_name"
   ${uci_set}type="$group_type"

   ruby -ryaml -E UTF-8 -e "
   begin
   Value = $group_hash;
   Thread.new{
   #strategy
   if Value['proxy-groups'][$count].key?('strategy') then
      group_strategy = '${uci_set}strategy=' + Value['proxy-groups'][$count]['strategy'].to_s
      system(group_strategy)
   end
   }.join;
   
   Thread.new{
   #disable-udp
   if Value['proxy-groups'][$count].key?('disable-udp') then
      group_disable_udp = '${uci_set}disable_udp=' + Value['proxy-groups'][$count]['disable-udp'].to_s
      system(group_disable_udp)
   end
   }.join;
   
   Thread.new{
   #test_url
   if Value['proxy-groups'][$count].key?('url') then
      group_test_url = '${uci_set}test_url=\"' + Value['proxy-groups'][$count]['url'].to_s + '\"'
      system(group_test_url)
   end
   }.join;
   
   Thread.new{
   #test_interval
   if Value['proxy-groups'][$count].key?('interval') then
      group_test_interval = '${uci_set}test_interval=' + Value['proxy-groups'][$count]['interval'].to_s
      system(group_test_interval)
   end
   }.join;
   
   Thread.new{
   #test_tolerance
   if Value['proxy-groups'][$count].key?('tolerance') then
      group_test_tolerance = '${uci_set}tolerance=' + Value['proxy-groups'][$count]['tolerance'].to_s
      system(group_test_tolerance)
   end
   }.join;
   
   Thread.new{
   #other_group
   Value_1=YAML.load_file('/tmp/Proxy_Group'); 
	 if Value['proxy-groups'][$count].key?('proxies') then 
	    Value['proxy-groups'][$count]['proxies'].each{
	    |x|
	       if Value_1.include?(x) then
	          uci = '${uci_add}other_group=\"' + x.to_s + '\"'
	          system(uci)
	       end
	    }
	 end
	 }.join;
   rescue Exception => e
   puts '${LOGTIME} Error: Resolve Proxy-group Error,【${CONFIG_NAME} - ${group_type} - ${group_name}: ' + e.message + '】'
   end
   " 2>/dev/null >> $LOG_FILE &
   
   let count++
done

wait
uci commit openclash
/usr/share/openclash/yml_proxys_get.sh
del_lock