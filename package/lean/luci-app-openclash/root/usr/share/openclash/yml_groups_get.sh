#!/bin/bash
. /lib/functions.sh
. /usr/share/openclash/openclash_ps.sh
. /usr/share/openclash/ruby.sh

status=$(unify_ps_status "yml_groups_get.sh")
[ "$status" -gt "3" ] && exit 0

START_LOG="/tmp/openclash_start.log"
CFG_FILE="/etc/config/openclash"
other_group_file="/tmp/yaml_other_group.yaml"
servers_update=$(uci get openclash.config.servers_update 2>/dev/null)
servers_if_update=$(uci get openclash.config.servers_if_update 2>/dev/null)
CONFIG_FILE=$(uci get openclash.config.config_path 2>/dev/null)
CONFIG_NAME=$(echo "$CONFIG_FILE" |awk -F '/' '{print $5}' 2>/dev/null)
UPDATE_CONFIG_FILE=$(uci get openclash.config.config_update_path 2>/dev/null)
UPDATE_CONFIG_NAME=$(echo "$UPDATE_CONFIG_FILE" |awk -F '/' '{print $5}' 2>/dev/null)

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
   exit 0
elif [ ! -s "$CONFIG_FILE" ] && [ -s "$BACKUP_FILE" ]; then
   mv "$BACKUP_FILE" "$CONFIG_FILE"
fi

echo "开始更新【$CONFIG_NAME】的策略组配置..." >$START_LOG

/usr/share/openclash/yml_groups_name_get.sh
[ ! -z "$(grep "读取错误" /tmp/Proxy_Group)" ] && {
	echo "读取错误，配置文件【$CONFIG_NAME】异常！" >$START_LOG
	uci commit openclash
	sleep 5
	echo "" >$START_LOG
	exit 0
}

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
   echo "正在删除旧配置..." >$START_LOG
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
   exit 0
else
   cfg_delete
fi

count=0
match_group_file="/tmp/Proxy_Group"
#提取策略组部分
group_hash=$(ruby_read "YAML.load_file('$CONFIG_FILE')" ".select {|x| 'proxy-groups' == x}")
num=$(ruby_read "$group_hash" "['proxy-groups'].count")
if [ -z "$num" ]; then
   echo "配置文件校验失败，请检查配置文件后重试！" >$START_LOG
   echo "${LOGTIME} Error: Unable To Parse Config File, Please Check And Try Again!" >> $LOG_FILE
   sleep 3
   exit 0
fi

while [ "$count" -lt "$num" ]
do
	
   #type
   group_type=$(ruby_read "$group_hash" "['proxy-groups'][$count]['type']")
   #strategy
   group_strategy=$(ruby_read "$group_hash" "['proxy-groups'][$count]['strategy']")
   #name
   group_name=$(ruby_read "$group_hash" "['proxy-groups'][$count]['name']")
   #disable-udp
   group_disable_udp=$(ruby_read "$group_hash" "['proxy-groups'][$count]['disable-udp']")
   #test_url
   group_test_url=$(ruby_read "$group_hash" "['proxy-groups'][$count]['url']")
   #test_interval
   group_test_interval=$(ruby_read "$group_hash" "['proxy-groups'][$count]['interval']")
   #test_tolerance
   group_test_tolerance=$(ruby_read "$group_hash" "['proxy-groups'][$count]['tolerance']")
   
   if [ -z "$group_type" ] || [ -z "$group_name" ]; then
      let count++
      continue
   fi

   echo "正在读取【$CONFIG_NAME】-【$group_type】-【$group_name】策略组配置..." > $START_LOG
   
   name=openclash
   uci_name_tmp=$(uci add $name groups)
   uci_set="uci -q set $name.$uci_name_tmp."
   uci_add="uci -q add_list $name.$uci_name_tmp."

   ${uci_set}config="$CONFIG_NAME"
   ${uci_set}name="$group_name"
   ${uci_set}old_name="$group_name"
   ${uci_set}old_name_cfg="$group_name"
   ${uci_set}type="$group_type"
   ${uci_set}disable_udp="$group_disable_udp"
   ${uci_set}strategy="$group_strategy"
   ${uci_set}test_url="$group_test_url"
   ${uci_set}test_interval="$group_test_interval"
   ${uci_set}tolerance="$group_test_tolerance"
	 
	 #other_group
	 ruby_read "$group_hash" "['proxy-groups'][$count]['proxies']" > $other_group_file
	 
	 cat $other_group_file |while read -r line
   do
      if [ -z "$line" ]; then
         continue
      fi
      
      if [ "$group_type" != "select" ] && [ "$group_type" != "relay" ]; then
         if [ "$line" != "DIRECT" ] && [ "$line" != "REJECT" ]; then
            continue
         fi
      fi
      
      if [ -n "$(grep -F "$line" "$match_group_file")" ]; then
         if [ "$group_type" = "select" ] || [ "$group_type" = "relay" ]; then
            ${uci_add}other_group="$line"
         elif [ "$line" = "DIRECT" ] || [ "$line" = "REJECT" ]; then
            ${uci_add}other_group_dr="$line"
         fi
      fi
	 done
   let count++
done

uci commit openclash
/usr/share/openclash/yml_proxys_get.sh