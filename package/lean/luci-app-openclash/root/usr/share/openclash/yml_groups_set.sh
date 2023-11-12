#!/bin/bash
. /lib/functions.sh
. /usr/share/openclash/log.sh

set_lock() {
   exec 887>"/tmp/lock/openclash_groups_set.lock" 2>/dev/null
   flock -x 887 2>/dev/null
}

del_lock() {
   flock -u 887 2>/dev/null
   rm -rf "/tmp/lock/openclash_groups_set.lock"
}

set_lock
GROUP_FILE="/tmp/yaml_groups.yaml"
CFG_FILE="/etc/config/openclash"
servers_update=$(uci -q get openclash.config.servers_update)
CONFIG_FILE=$(uci -q get openclash.config.config_path)
CONFIG_NAME=$(echo "$CONFIG_FILE" |awk -F '/' '{print $5}' 2>/dev/null)
UPDATE_CONFIG_FILE=$(uci -q get openclash.config.config_update_path)
UPDATE_CONFIG_NAME=$(echo "$UPDATE_CONFIG_FILE" |awk -F '/' '{print $5}' 2>/dev/null)

if [ -n "$UPDATE_CONFIG_FILE" ]; then
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

set_groups()
{
   if [ -z "$1" ]; then
      return
   fi
   
   if [ "$add_for_this" -eq 1 ]; then
      return
   fi

   if [ "$1" = "all" ] || [[ "$3" =~ ${1} ]]; then
      set_group=1
      add_for_this=1
      echo "      - \"${2}\"" >>$GROUP_FILE
   fi

}

set_relay_groups()
{
   if [ -z "$1" ]; then
      return
   fi
   
   if [ "$add_for_this" -eq 1 ]; then
      return
   fi
   
   if [ -n "$(echo "$1" |grep "#relay#")" ]; then
      server_relay_num=$(echo "$1" |awk -F '#relay#' '{print $2}')
      server_group_name=$(echo "$1" |awk -F '#relay#' '{print $1}')
   fi

   if [ -n "$server_relay_num" ]; then
      if [[ "$3" =~ ${server_group_name} ]] || [ "$server_group_name" = "all" ]; then
         set_group=1
         add_for_this=1
         echo "$server_relay_num #      - \"${2}\"" >>/tmp/relay_server
      fi
   fi
}

#加入节点
yml_servers_add()
{
   
   local section="$1"
   add_for_this=0
   config_get_bool "enabled" "$section" "enabled" "1"
   config_get "config" "$section" "config" ""
   config_get "name" "$section" "name" ""
   config_get "relay_groups" "$section" "relay_groups" ""
   
   if [ -n "$config" ] && [ "$config" != "$CONFIG_NAME" ] && [ "$config" != "all" ]; then
      return
  fi
  
   if [ "$enabled" = "0" ]; then
      return
   else
      if [ -z "$4" ] && [ "$3" = "relay" ] && [ -n "$relay_groups" ]; then
         config_list_foreach "$section" "relay_groups" set_relay_groups "$name" "$2"
      elif [ -z "$4" ]; then
         config_list_foreach "$section" "groups" set_groups "$name" "$2"
      fi
         
      if [ -n "$if_game_group" ] && [ -z "$(ruby -ryaml -rYAML -I "/usr/share/openclash" -E UTF-8 -e "Value = YAML.load_file('$CONFIG_FILE'); Value['proxies'].each{|x| if x['name'].eql?('$name') then puts x['name'] end}" 2>/dev/null)" ]; then
         /usr/share/openclash/yml_proxys_set.sh "$name" "proxy"
      fi
   fi
   
}

add_other_group()
{
   local section="$1"
   local name enabled config
   config_get_bool "enabled" "$section" "enabled" "1"
   config_get "config" "$section" "config" ""
   config_get "name" "$section" "name" ""

   if [ "$enabled" = "0" ]; then
      return
   fi

   if [ -n "$config" ] && [ "$config" != "$CONFIG_NAME" ] && [ "$config" != "all" ]; then
      return
   fi
   
   if [ -z "$name" ]; then
      return
   fi

   if [ "$3" = "$name" ]; then
      return
   fi

   if [ "$2" = "all" ] || [[ "$name" =~ ${2} ]]; then
      set_group=1
      echo "      - ${name}" >>$GROUP_FILE
   fi
}

#加入其它策略组
set_other_groups()
{
   if [ -z "$1" ]; then
      return
   fi

   if [ "$1" = "DIRECT" ] || [ "$1" = "REJECT" ]; then
      set_group=1
      echo "      - ${1}" >>$GROUP_FILE
      return
   fi

   config_foreach add_other_group "groups" "$1" "$2" #比对策略组
}

#加入代理集
set_proxy_provider()
{
   local section="$1"
   add_for_this=0
   config_get_bool "enabled" "$section" "enabled" "1"
   config_get "config" "$section" "config" ""
   config_get "name" "$section" "name" ""
   
   if [ -n "$config" ] && [ "$config" != "$CONFIG_NAME" ] && [ "$config" != "all" ]; then
      return
  fi
  
   if [ "$enabled" = "0" ]; then
      return
   else
      if [ -z "$3" ]; then
         config_list_foreach "$section" "groups" set_provider_groups "$name" "$2"
      fi
      
      if [ -n "$if_game_group" ] && [ -z "$(ruby -ryaml -rYAML -I "/usr/share/openclash" -E UTF-8 -e "Value = YAML.load_file('$CONFIG_FILE'); Value['proxy-providers'].keys.each{|x| if x.eql?('$name') then puts x end}" 2>/dev/null)" ]; then
         /usr/share/openclash/yml_proxys_set.sh "$name" "proxy-provider"
      fi
   fi
}

set_provider_groups()
{
   if [ -z "$1" ]; then
      return
   fi
   
   if [ "$add_for_this" -eq 1 ]; then
      return
   fi

   if [[ "$3" =~ ${1} ]] || [ "$1" = "all" ]; then
      set_proxy_provider=1
      add_for_this=1
      echo "      - ${2}" >>$GROUP_FILE
   fi

}

#创建策略组
yml_groups_set()
{

   local section="$1"
   config_get_bool "enabled" "$section" "enabled" "1"
   config_get "config" "$section" "config" ""
   config_get "type" "$section" "type" ""
   config_get "name" "$section" "name" ""
   config_get "disable_udp" "$section" "disable_udp" ""
   config_get "strategy" "$section" "strategy" ""
   config_get "old_name" "$section" "old_name" ""
   config_get "test_url" "$section" "test_url" ""
   config_get "test_interval" "$section" "test_interval" ""
   config_get "tolerance" "$section" "tolerance" ""
   config_get "interface_name" "$section" "interface_name" ""
   config_get "routing_mark" "$section" "routing_mark" ""
   config_get "policy_filter" "$section" "policy_filter" ""

   if [ "$enabled" = "0" ]; then
      return
   fi
   
   if [ -n "$if_game_group" ] && [ "$if_game_group" != "$name" ]; then
      return
   fi

   if [ -n "$config" ] && [ "$config" != "$CONFIG_NAME" ] && [ "$config" != "all" ]; then
      return
   fi
   
   if [ -z "$type" ]; then
      return
   fi
   
   if [ -z "$name" ]; then
      return
   fi
   
   if [ -z "$test_url" ] || [ -z "$test_interval" ] && [ "$type" != "select" ] && [ "$type" != "relay" ]; then
      return
   fi
   
   #游戏策略组存在时判断节点是否存在
   if [ -n "$if_game_group" ] && [ -n "$(grep "^$if_game_group$" /tmp/Proxy_Group)" ]; then
      config_foreach yml_servers_add "servers" "$name" "$type" "check" #加入服务器节点
      config_foreach set_proxy_provider "proxy-provider" "$group_name" "check" #加入代理集
      return
   fi
   
   LOG_OUT "Start Writing【$CONFIG_NAME - $type - $name】Group To Config File..."
   
   echo "  - name: $name" >>$GROUP_FILE
   echo "    type: $type" >>$GROUP_FILE
   if [ "$type" = "load-balance" ]; then
      [ -n "$strategy" ] && {
           echo "    strategy: $strategy" >>$GROUP_FILE
      }
   fi
   [ -n "$disable_udp" ] && {
      echo "    disable-udp: $disable_udp" >>$GROUP_FILE
   }
   group_name="$name"
   echo "    proxies: $group_name" >>$GROUP_FILE
   
   #名字变化时处理规则部分
   if [ "$name" != "$old_name" ] && [ -n "$old_name" ]; then
      sed -i "s/,${old_name}/,${name}#delete_/g" "$CONFIG_FILE" 2>/dev/null
      sed -i "s/: \"${old_name}\"/: \"${name}#delete_\"/g" "$CONFIG_FILE" 2>/dev/null
      sed -i "s/return \"${old_name}\"$/return \"${name}#delete_\"/g" "$CONFIG_FILE" 2>/dev/null
      sed -i "s/old_name \'${old_name}\'/old_name \'${name}\'/g" "$CFG_FILE" 2>/dev/null
      config_load "openclash"
   fi
   
   set_group=0
   set_proxy_provider=0
   
   config_list_foreach "$section" "other_group" set_other_groups "$name" #加入其他策略组
   config_foreach yml_servers_add "servers" "$name" "$type" #加入服务器节点
   
   if [ "$type" = "relay" ] && [ -s "/tmp/relay_server" ]; then
      cat /tmp/relay_server |sort -k 1 |awk -F '#' '{print $2}' > /tmp/relay_server.list 2>/dev/null
      sed -i "/^ \{0,\}proxies: ${group_name}/r/tmp/relay_server.list" "$GROUP_FILE" 2>/dev/null
      rm -rf /tmp/relay_server 2>/dev/null
   fi

   echo "    use: $group_name" >>$GROUP_FILE
   
   config_foreach set_proxy_provider "proxy-provider" "$group_name" #加入代理集

   if [ "$set_group" -eq 1 ]; then
      sed -i "/^ \{0,\}proxies: ${group_name}/c\    proxies:" $GROUP_FILE
   else
      sed -i "/proxies: ${group_name}/d" $GROUP_FILE 2>/dev/null
   fi
   
   if [ "$set_proxy_provider" -eq 1 ]; then
      sed -i "/^ \{0,\}use: ${group_name}/c\    use:" $GROUP_FILE
   else
      sed -i "/use: ${group_name}/d" $GROUP_FILE 2>/dev/null
   fi
   
   [ -n "$test_url" ] && {
        echo "    url: $test_url" >>$GROUP_FILE
   }
   [ -n "$test_interval" ] && {
      echo "    interval: \"$test_interval\"" >>$GROUP_FILE
   }
   [ -n "$tolerance" ] && {
      echo "    tolerance: \"$tolerance\"" >>$GROUP_FILE
   }
   [ -n "$policy_filter" ] && {
      echo "    filter: \"$policy_filter\"" >>$GROUP_FILE
   }
   [ -n "$interface_name" ] && {
      echo "    interface-name: \"$interface_name\"" >>$GROUP_FILE
   }
   [ -n "$routing_mark" ] && {
      echo "    routing-mark: \"$routing_mark\"" >>$GROUP_FILE
   }
}

create_config=$(uci -q get openclash.config.create_config)
servers_if_update=$(uci -q get openclash.config.servers_if_update)
if_game_group="$1"
if [ "$create_config" = "0" ] || [ "$servers_if_update" = "1" ] || [ -n "$if_game_group" ]; then
   /usr/share/openclash/yml_groups_name_get.sh
   if [ $? -ne 0 ]; then
      LOG_OUT "Error: Config File【$CONFIG_NAME】Unable To Parse, Please Choose One-key Function To Create Config File..."
      uci -q commit openclash
      SLOG_CLEAN
      del_lock
      exit 0
   else
      if [ -z "$if_game_group" ]; then
         echo "proxy-groups:" >$GROUP_FILE
      fi
      config_load "openclash"
      config_foreach yml_groups_set "groups"
      sed -i "s/#delete_//g" "$CONFIG_FILE" 2>/dev/null
      rm -rf /tmp/relay_server.list 2>/dev/null
   fi
fi

del_lock
if [ -z "$if_game_group" ]; then
   /usr/share/openclash/yml_proxys_set.sh
fi
