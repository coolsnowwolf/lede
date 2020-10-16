#!/bin/sh
. /lib/functions.sh
. /usr/share/openclash/openclash_ps.sh

status=$(unify_ps_status "yml_groups_set.sh")
[ "$status" -gt "3" ] && exit 0

START_LOG="/tmp/openclash_start.log"
GROUP_FILE="/tmp/yaml_groups.yaml"
CONFIG_GROUP_FILE="/tmp/yaml_group.yaml"
CFG_FILE="/etc/config/openclash"
servers_update=$(uci get openclash.config.servers_update 2>/dev/null)
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

set_groups()
{
  if [ -z "$1" ]; then
     return
  fi

	if [ "$1" = "$3" ]; then
	   set_group=1
	   echo "      - \"${2}\"" >>$GROUP_FILE
	fi

}

set_relay_groups()
{
  if [ -z "$1" ]; then
     return
  fi
  
  if [ ! -z "$(echo "$1" |grep "#relay#")" ]; then
     server_relay_num=$(echo "$1" |awk -F '#relay#' '{print $2}')
     server_group_name=$(echo "$1" |awk -F '#relay#' '{print $1}')
  fi

  if [ ! -z "$server_relay_num" ] && [ "$server_group_name" = "$3" ]; then
     set_group=1
     echo "$server_relay_num #      - \"${2}\"" >>/tmp/relay_server
	fi
}

#加入节点
yml_servers_add()
{
	
	local section="$1"
	config_get_bool "enabled" "$section" "enabled" "1"
	config_get "config" "$section" "config" ""
	config_get "name" "$section" "name" ""
	config_get "relay_groups" "$section" "relay_groups" ""
	
	if [ ! -z "$config" ] && [ "$config" != "$CONFIG_NAME" ] && [ "$config" != "all" ]; then
      return
  fi
  
	if [ "$enabled" = "0" ]; then
      return
  else
     if [ -z "$4" ] && [ "$3" = "relay" ] && [ ! -z "$relay_groups" ]; then
	      config_list_foreach "$section" "relay_groups" set_relay_groups "$name" "$2"
	   elif [ -z "$4" ]; then
	      config_list_foreach "$section" "groups" set_groups "$name" "$2"
     fi
	   
	   if [ ! -z "$if_game_group" ] && [ -z "$(grep -F $name /tmp/yaml_proxy.yaml)" ]; then
	      /usr/share/openclash/yml_proxys_set.sh "$name" "proxy"
	   fi
	fi
	
}

#加入其它策略组
set_other_groups()
{
   if [ -z "$1" ]; then
      return
   fi
   set_group=1
   echo "      - ${1}" >>$GROUP_FILE

}

#加入代理集
set_proxy_provider()
{
	local section="$1"
	config_get_bool "enabled" "$section" "enabled" "1"
	config_get "config" "$section" "config" ""
	config_get "name" "$section" "name" ""
	
	if [ ! -z "$config" ] && [ "$config" != "$CONFIG_NAME" ] && [ "$config" != "all" ]; then
      return
  fi
  
	if [ "$enabled" = "0" ]; then
      return
  else
     if [ -z "$3" ]; then
	      config_list_foreach "$section" "groups" set_provider_groups "$name" "$2"
     fi
	   
	   if [ ! -z "$if_game_group" ] && [ -z "$(grep "^ \{0,\}$name" /tmp/yaml_proxy_provider.yaml)" ]; then
	      /usr/share/openclash/yml_proxys_set.sh "$name" "proxy-provider"
	   fi
	fi
}

set_provider_groups()
{
  if [ -z "$1" ]; then
     return
  fi

	if [ "$1" = "$3" ]; then
	   set_proxy_provider=1
	   echo "      - ${2}" >>$GROUP_FILE
	fi

}

#创建策略组
yml_groups_set()
{

   local section="$1"
   config_get "config" "$section" "config" ""
   config_get "type" "$section" "type" ""
   config_get "name" "$section" "name" ""
   config_get "old_name" "$section" "old_name" ""
   config_get "test_url" "$section" "test_url" ""
   config_get "test_interval" "$section" "test_interval" ""
   config_get "tolerance" "$section" "tolerance" ""
   
   if [ ! -z "$if_game_group" ] && [ "$if_game_group" != "$name" ]; then
      return
   fi

   if [ ! -z "$config" ] && [ "$config" != "$CONFIG_NAME" ] && [ "$config" != "all" ]; then
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
   
   echo "正在写入【$type】-【$name】策略组到配置文件【$CONFIG_NAME】..." >$START_LOG
   
   echo "  - name: $name" >>$GROUP_FILE
   echo "    type: $type" >>$GROUP_FILE
   group_name="$name"
   echo "    proxies: $group_name" >>$GROUP_FILE
   
   #名字变化时处理规则部分
   if [ "$name" != "$old_name" ] && [ ! -z "$old_name" ]; then
      sed -i "s/,${old_name}/,${name}#d/g" "$CONFIG_FILE" 2>/dev/null
      sed -i "s/:${old_name}$/:${name}#d/g" "$CONFIG_FILE" 2>/dev/null #修改第三方规则分组对应标签
      sed -i "s/old_name \'${old_name}/old_name \'${name}/g" "$CFG_FILE" 2>/dev/null
      config_load "openclash"
   fi
   
   set_group=0
   set_proxy_provider=0
   
   if [ "$type" = "select" ] || [ "$type" = "relay" ]; then
      config_list_foreach "$section" "other_group" set_other_groups #加入其他策略组
   else
      config_list_foreach "$section" "other_group_dr" set_other_groups #仅加入direct/reject其他策略组
   fi
   
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
   
   [ ! -z "$test_url" ] && {
   	  echo "    url: $test_url" >>$GROUP_FILE
   }
   [ ! -z "$test_interval" ] && {
      echo "    interval: \"$test_interval\"" >>$GROUP_FILE
   }
   [ ! -z "$tolerance" ] && {
      echo "    tolerance: \"$tolerance\"" >>$GROUP_FILE
   }
}

create_config=$(uci get openclash.config.create_config 2>/dev/null)
servers_if_update=$(uci get openclash.config.servers_if_update 2>/dev/null)
if_game_group="$1"
if [ "$create_config" = "0" ] || [ "$servers_if_update" = "1" ] || [ ! -z "$if_game_group" ]; then
   /usr/share/openclash/yml_groups_name_get.sh
   if [ ! -z "$(grep "读取错误" /tmp/Proxy_Group)"]; then
      echo "配置文件【$CONFIG_NAME】的信息读取失败，无法进行修改，请选择一键创建配置文件..." >$START_LOG
      uci commit openclash
      sleep 5
      echo "" >$START_LOG
      exit 0
   else
      if [ -z "$if_game_group" ]; then
         echo "开始写入配置文件【$CONFIG_NAME】的策略组信息..." >$START_LOG
         echo "proxy-groups:" >$GROUP_FILE
      else
         echo "开始加入游戏&规则集策略组【$if_game_group】的信息..." >$START_LOG
         rm -rf $GROUP_FILE
      fi
      config_load "openclash"
      config_foreach yml_groups_set "groups"
      sed -i "s/#d//g" "$CONFIG_FILE" 2>/dev/null
      rm -rf /tmp/relay_server.list 2>/dev/null
      echo "配置文件【$CONFIG_NAME】的策略组写入完成！" >$START_LOG
   fi
fi
if [ -z "$if_game_group" ]; then
   /usr/share/openclash/yml_proxys_set.sh
fi