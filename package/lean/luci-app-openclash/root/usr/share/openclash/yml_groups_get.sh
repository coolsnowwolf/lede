#!/bin/bash
. /lib/functions.sh
. /usr/share/openclash/log.sh

set_lock() {
   exec 876>"/tmp/lock/openclash_groups_get.lock" 2>/dev/null
   flock -x 876 2>/dev/null
}

del_lock() {
   flock -u 876 2>/dev/null
   rm -rf "/tmp/lock/openclash_groups_get.lock"
}

ruby_read_hash()
{
   RUBY_YAML_PARSE="Thread.new{Value = $1; puts Value$2}.join"
   ruby -ryaml -rYAML -I "/usr/share/openclash" -E UTF-8 -e "$RUBY_YAML_PARSE" 2>/dev/null
}

ruby_read()
{
   RUBY_YAML_PARSE="Thread.new{Value = YAML.load_file('$1'); puts Value$2}.join"
   ruby -ryaml -rYAML -I "/usr/share/openclash" -E UTF-8 -e "$RUBY_YAML_PARSE" 2>/dev/null
}

CFG_FILE="/etc/config/openclash"
other_group_file="/tmp/yaml_other_group.yaml"
servers_update=$(uci -q get openclash.config.servers_update)
servers_if_update=$(uci -q get openclash.config.servers_if_update)
CONFIG_FILE=$(uci -q get openclash.config.config_path)
CONFIG_NAME=$(echo "$CONFIG_FILE" |awk -F '/' '{print $5}' 2>/dev/null)
UPDATE_CONFIG_FILE=$(uci -q get openclash.config.config_update_path)
UPDATE_CONFIG_NAME=$(echo "$UPDATE_CONFIG_FILE" |awk -F '/' '{print $5}' 2>/dev/null)
LOGTIME=$(echo $(date "+%Y-%m-%d %H:%M:%S"))
LOG_FILE="/tmp/openclash.log"
set_lock

if [ ! -z "$UPDATE_CONFIG_FILE" ]; then
   CONFIG_FILE="$UPDATE_CONFIG_FILE"
   CONFIG_NAME="$UPDATE_CONFIG_NAME"
fi

if [ -z "$CONFIG_FILE" ]; then
   for file_name in /etc/openclash/config/*
   do
      if [ -f "$file_name" ]; then
         CONFIG_FILE=$file_name
         CONFIG_NAME=$(echo "$CONFIG_FILE" |awk -F '/' '{print $5}' 2>/dev/null)
         break
      fi
   done
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
	uci -q commit openclash
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
   group_num=$(grep "^config groups$" "$CFG_FILE" |wc -l)
   for ((i=$group_num;i>=0;i--))
	 do
	    if [ "$(uci -q get openclash.@groups["$i"].config)" = "$CONFIG_NAME" ] || [ "$(uci -q get openclash.@groups["$i"].config)" = "all" ]; then
	       uci -q delete openclash.@groups["$i"]
	       uci -q commit openclash
	    fi
	 done
#删除启用的节点
   server_num=$(grep "^config servers$" "$CFG_FILE" |wc -l)
   for ((i=$server_num;i>=0;i--))
	 do
	    if [ "$(uci -q get openclash.@servers["$i"].config)" = "$CONFIG_NAME" ] || [ "$(uci -q get openclash.@servers["$i"].config)" = "all" ]; then
	    	 if [ "$(uci -q get openclash.@servers["$i"].enabled)" = "1" ] && [ "$(uci -q get openclash.@servers["$i"].manual)" = "0" ]; then
	          uci -q delete openclash.@servers["$i"]
	          uci -q commit openclash
	       fi
	    fi
	 done
#删除启用的代理集
   provider_num=$(grep "^config proxy-provider$" "$CFG_FILE" 2>/dev/null |wc -l)
   for ((i=$provider_num;i>=0;i--))
	 do
	    if [ "$(uci -q get openclash.@proxy-provider["$i"].config)" = "$CONFIG_NAME" ] || [ "$(uci -q get openclash.@proxy-provider["$i"].config)" = "all" ]; then
	       if [ "$(uci -q get openclash.@proxy-provider["$i"].enabled)" = "1" ] && [ "$(uci -q get openclash.@proxy-provider["$i"].manual)" = "0" ]; then
	          uci -q delete openclash.@proxy-provider["$i"]
	          uci -q commit openclash
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
fi

count=0
match_group_file="/tmp/Proxy_Group"
#提取策略组部分
group_hash=$(ruby_read "$CONFIG_FILE" ".select {|x| 'proxy-groups' == x}")
num=$(ruby_read_hash "$group_hash" "['proxy-groups'].count")

if [ -z "$num" ]; then
   LOG_OUT "Error: Unable To Parse Config File, Please Check And Try Again!"
   del_lock
   exit 0
fi

cfg_delete

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
   uci_name_tmp=$(uci -q add $name groups)
   uci_set="uci -q set $name.$uci_name_tmp."
   uci_add="uci -q add_list $name.$uci_name_tmp."
   
   ${uci_set}enabled="1"
   ${uci_set}config="$CONFIG_NAME"
   ${uci_set}name="$group_name"
   ${uci_set}old_name="$group_name"
   ${uci_set}old_name_cfg="$group_name"
   ${uci_set}type="$group_type"

   ruby -ryaml -rYAML -I "/usr/share/openclash" -E UTF-8 -e "
   begin
   Value = ${group_hash};
   Thread.new{
   #strategy
   if Value['proxy-groups'][$count].key?('strategy') then
      group_strategy = '${uci_set}strategy=' + Value['proxy-groups'][$count]['strategy'].to_s
      system(group_strategy)
   end;
   }.join;
   
   Thread.new{
   #disable-udp
   if Value['proxy-groups'][$count].key?('disable-udp') then
      group_disable_udp = '${uci_set}disable_udp=' + Value['proxy-groups'][$count]['disable-udp'].to_s
      system(group_disable_udp)
   end;
   }.join;
   
   Thread.new{
   if Value['proxy-groups'][$count]['type'] == 'url-test' or Value['proxy-groups'][$count]['type'] == 'fallback' or Value['proxy-groups'][$count]['type'] == 'load-balance' then
      #test_url
      if Value['proxy-groups'][$count].key?('url') then
         group_test_url = '${uci_set}test_url=\"' + Value['proxy-groups'][$count]['url'].to_s + '\"'
         system(group_test_url)
      end;

      #test_interval
      if Value['proxy-groups'][$count].key?('interval') then
         group_test_interval = '${uci_set}test_interval=' + Value['proxy-groups'][$count]['interval'].to_s
         system(group_test_interval)
      end;

      #test_tolerance
      if Value['proxy-groups'][$count]['type'] == 'url-test' then
         if Value['proxy-groups'][$count].key?('tolerance') then
            group_test_tolerance = '${uci_set}tolerance=' + Value['proxy-groups'][$count]['tolerance'].to_s
            system(group_test_tolerance)
         end;
      end;
   end;
   }.join;
   
   Thread.new{
   #Policy Filter
   if Value['proxy-groups'][$count].key?('filter') then
      policy_filter = '${uci_set}policy_filter=' + Value['proxy-groups'][$count]['filter'].to_s
      system(policy_filter)
   end
   }.join;
   
   Thread.new{
   #interface-name
   if Value['proxy-groups'][$count].key?('interface-name') then
      interface_name = '${uci_set}interface_name=' + Value['proxy-groups'][$count]['interface-name'].to_s
      system(interface_name)
   end
   }.join;
   
   Thread.new{
   #routing-mark
   if Value['proxy-groups'][$count].key?('routing-mark') then
      routing_mark = '${uci_set}routing_mark=' + Value['proxy-groups'][$count]['routing-mark'].to_s
      system(routing_mark)
   end
   }.join;
   
   Thread.new{
   #other_group
   Value_1 = File.readlines('/tmp/Proxy_Group').map!{|x| x.strip}; 
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
   puts '${LOGTIME} Error: Resolve Groups Failed,【${CONFIG_NAME} - ${group_type} - ${group_name}: ' + e.message + '】'
   end
   " 2>/dev/null >> $LOG_FILE &
   
   let count++
done

wait
uci -q commit openclash
/usr/share/openclash/yml_proxys_get.sh
del_lock