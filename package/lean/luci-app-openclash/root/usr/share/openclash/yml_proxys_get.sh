#!/bin/bash
. /lib/functions.sh
. /usr/share/openclash/openclash_ps.sh
. /usr/share/openclash/ruby.sh

status=$(unify_ps_status "yml_proxys_get.sh")
[ "$status" -gt "3" ] && exit 0

START_LOG="/tmp/openclash_start.log"
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

#提取节点部分
proxy_hash=$(ruby_read "YAML.load_file('$CONFIG_FILE')" ".select {|x| 'proxies' == x or 'proxy-providers' == x}")

CFG_FILE="/etc/config/openclash"
match_servers="/tmp/match_servers.list"
match_provider="/tmp/match_provider.list"
servers_update=$(uci get openclash.config.servers_update 2>/dev/null)
servers_if_update=$(uci get openclash.config.servers_if_update 2>/dev/null)
new_servers_group=$(uci get openclash.config.new_servers_group 2>/dev/null)

#proxy
num=$(ruby_read "$proxy_hash" "['proxies'].count")
count=0

#provider
provider_num=$(ruby_read "$proxy_hash" "['proxy-providers'].count")
provider_count=0

#group
group_hash=$(ruby_read "YAML.load_file('$CONFIG_FILE')" ".select {|x| 'proxy-groups' == x}")
group_count=$(ruby_read "$group_hash" "['proxy-groups'].count")

if [ -z "$num" ] && [ -z "$provider_num" ]; then
   echo "配置文件校验失败，请检查配置文件后重试！" >$START_LOG
   echo "${LOGTIME} Error: Unable To Parse Config File, Please Check And Try Again!" >> $LOG_FILE
   sleep 3
   exit 0
fi

cfg_new_servers_groups_check()
{
   if [ -z "$1" ]; then
      return
   fi
   
   [ "$1" = "$2" ] && {
	    config_group_exist=$(( $config_group_exist + 1 ))
	 }
}

cfg_group_name()
{
   local section="$1"
   config_get "name" "$section" "name" ""
   config_get "config" "$section" "config" ""

   if [ -z "$config" ]; then
      return
   fi
   
   if [ "$config" != "$CONFIG_NAME" ] && [ "$config" != "all" ]; then
      return
   fi

   if [ -z "$name" ]; then
	    return
   fi

   config_list_foreach "config" "new_servers_group" cfg_new_servers_groups_check "$name"
   config_group_exists=$(( $config_group_exists + 1 ))
}

#判断当前配置文件策略组信息是否包含指定策略组
config_group_exist=0
config_group_exists=0
config_load "openclash"
config_foreach cfg_group_name "groups"

if [ "$config_group_exists" -eq "$config_group_exist" ]; then
   config_group_exist=1
else
   config_group_exist=0
fi

echo "开始更新【$CONFIG_NAME】的代理集配置..." >$START_LOG

yml_provider_name_get()
{
   local section="$1"
   config_get "name" "$section" "name" ""
   [ ! -z "$name" ] && {
      echo "$provider_nums.$name" >>"$match_provider"
   }
   provider_nums=$(( $provider_nums + 1 ))
}

cfg_new_provider_groups_get()
{
	 if [ -z "$1" ]; then
      return
   fi
   
   ${uci_add}groups="${1}"
}

[ "$servers_update" -eq 1 ] && {
echo "" >"$match_provider"
provider_nums=0
config_load "openclash"
config_foreach yml_provider_name_get "proxy-provider"
}

#获取代理集信息
while [ "$provider_count" -lt "$provider_num" ]
do
   #name
   provider_name=$(ruby_read "$proxy_hash" "['proxy-providers'].keys[$provider_count]")
   #type
   provider_type=$(ruby_read "$proxy_hash" "['proxy-providers'].values[$provider_count]['type']")
   #path
   provider_path=$(ruby_read "$proxy_hash" "['proxy-providers'].values[$provider_count]['path']")
   #gen_url
   provider_gen_url=$(ruby_read "$proxy_hash" "['proxy-providers'].values[$provider_count]['url']")
   #gen_interval
   provider_gen_interval=$(ruby_read "$proxy_hash" "['proxy-providers'].values[$provider_count]['interval']")
   #che_enable
   provider_che_enable=$(ruby_read "$proxy_hash" "['proxy-providers'].values[$provider_count]['health-check']['enable']")
   #che_url
   provider_che_url=$(ruby_read "$proxy_hash" "['proxy-providers'].values[$provider_count]['health-check']['url']")
   #che_interval
   provider_che_interval=$(ruby_read "$proxy_hash" "['proxy-providers'].values[$provider_count]['health-check']['interval']")
   
   if [ -z "$provider_name" ] || [ -z "$provider_type" ]; then
      let provider_count++
      continue
   fi
   
   echo "正在读取【$CONFIG_NAME】-【$provider_name】代理集配置..." >$START_LOG
   
   #代理集存在时获取代理集编号
   provider_nums=$(grep -Fw "$provider_name" "$match_provider" 2>/dev/null|awk -F '.' '{print $1}')
   if [ "$servers_update" -eq 1 ] && [ -n "$provider_nums" ]; then
      sed -i "/^${provider_nums}\./c\#match#" "$match_provider" 2>/dev/null
      uci_set="uci -q set openclash.@proxy-provider["$provider_nums"]."
      ${uci_set}manual="0"
      ${uci_set}name="$provider_name"
      ${uci_set}type="$provider_type"
      if [ "$provider_type" = "http" ]; then
         ${uci_set}path="./proxy_provider/$provider_name.yaml"
      elif [ "$provider_type" = "file" ]; then
         ${uci_set}path="$provider_path"
      fi
      ${uci_set}provider_url="$provider_gen_url"
      ${uci_set}provider_interval="$provider_gen_interval"
      ${uci_set}health_check="$provider_che_enable"
      ${uci_set}health_check_url="$provider_che_url"
      ${uci_set}health_check_interval="$provider_che_interval"
   else
   #代理集不存在时添加新代理集
      name=openclash
      uci_name_tmp=$(uci add $name proxy-provider)
      uci_set="uci -q set $name.$uci_name_tmp."
      uci_add="uci -q add_list $name.$uci_name_tmp."
   
      if [ -z "$new_servers_group" ] && [ "$servers_if_update" = "1" ] && [ "$servers_update" -eq 1 ]; then
         ${uci_set}enabled="0"
      else
         ${uci_set}enabled="1"
      fi
      if [ "$servers_if_update" = "1" ]; then
         ${uci_set}manual="0"
      else
         ${uci_set}manual="1"
      fi
      ${uci_set}config="$CONFIG_NAME"
      ${uci_set}name="$provider_name"
      ${uci_set}type="$provider_type"
      if [ "$provider_type" = "http" ]; then
         ${uci_set}path="./proxy_provider/$provider_name.yaml"
      elif [ "$provider_type" = "file" ]; then
         ${uci_set}path="$provider_path"
      fi
      ${uci_set}provider_url="$provider_gen_url"
      ${uci_set}provider_interval="$provider_gen_interval"
      ${uci_set}health_check="$provider_che_enable"
      ${uci_set}health_check_url="$provider_che_url"
      ${uci_set}health_check_interval="$provider_che_interval"


#加入策略组
      if [ "$servers_if_update" = "1" ] && [ ! -z "$new_servers_group" ] && [ "$config_group_exist" -eq 1 ]; then
#新代理集且设置默认策略组时加入指定策略组
         config_load "openclash"
         config_list_foreach "config" "new_servers_group" cfg_new_provider_groups_get
      else
         for ((i=0;i<$group_count;i++))
         do
            if "$(ruby_read "$group_hash" "['proxy-groups'][$i]['use'].include?('$provider_name')")"; then
               ${uci_add}groups="$(ruby_read "$group_hash" "['proxy-groups'][$i]['name']")"
            fi
	       done 2>/dev/null
	    fi
   fi
   uci commit openclash
   let provider_count++
done 2>/dev/null


#删除订阅中已不存在的代理集
if [ "$servers_if_update" = "1" ]; then
     echo "删除【$CONFIG_NAME】订阅中已不存在的代理集..." >$START_LOG
     sed -i '/#match#/d' "$match_provider" 2>/dev/null
     cat $match_provider 2>/dev/null|awk -F '.' '{print $1}' |sort -rn |while read line
     do
        if [ -z "$line" ]; then
           continue
        fi
        if [ "$(uci get openclash.@proxy-provider["$line"].manual)" = "0" ] && [ "$(uci get openclash.@proxy-provider["$line"].config)" = "$CONFIG_NAME" ]; then
           uci delete openclash.@proxy-provider["$line"] 2>/dev/null
        fi
     done
fi


yml_servers_name_get()
{
	 local section="$1"
   config_get "name" "$section" "name" ""
   [ ! -z "$name" ] && {
      echo "$server_num.$name" >>"$match_servers"
   }
   server_num=$(( $server_num + 1 ))
}

server_key_get()
{
   local section="$1"
   config_get_bool "enabled" "$section" "enabled" "1"

   if [ "$enabled" = "0" ]; then
      return
   fi
   
   config_get "name" "$section" "name" ""
   config_get "keyword" "$section" "keyword" ""
   config_get "ex_keyword" "$section" "ex_keyword" ""
   
   if [ -z "$name" ]; then
      name="config"
   fi
   
   if [ ! -z "$keyword" ] && [ "$name.yaml" == "$CONFIG_NAME" ]; then
      config_keyword="$keyword"
      key_section="$1"
   fi
   
   if [ ! -z "$ex_keyword" ] && [ "$name.yaml" == "$CONFIG_NAME" ]; then
      config_ex_keyword="$ex_keyword"
      key_section="$1"
   fi

}

server_key_match()
{

	if [ "$match" = "true" ] || [ ! -z "$(echo "$1" |grep "^ \{0,\}$")" ] || [ ! -z "$(echo "$1" |grep "^\t\{0,\}$")" ]; then
	   return
	fi
	
	if [ ! -z "$(echo "$1" |grep "&")" ]; then
	   key_word=$(echo "$1" |sed 's/&/ /g')
	   match=0
	   matchs=0
	   for k in $key_word
	   do
	      if [ -z "$k" ]; then
	         continue
	      fi
	      
	      if [ ! -z "$(echo "$2" |grep -i "$k")" ]; then
	         match=$(( $match + 1 ))
	      fi
	      matchs=$(( $matchs + 1 ))
	   done
	   if [ "$match" = "$matchs" ]; then
	   	  match="true"
	   else
	      match="false"
	   fi
	else
	   if [ ! -z "$(echo "$2" |grep -i "$1")" ]; then
	      match="true"
	   fi
	fi
}

server_key_exmatch()
{

	if [ "$match" = "false" ] || [ ! -z "$(echo "$1" |grep "^ \{0,\}$")" ] || [ ! -z "$(echo "$1" |grep "^\t\{0,\}$")" ]; then
	   return
	fi
	
	if [ ! -z "$(echo "$1" |grep "&")" ]; then
	   key_word=$(echo "$1" |sed 's/&/ /g')
	   match=0
	   matchs=0
	   for k in $key_word
	   do
	      if [ -z "$k" ]; then
	         continue
	      fi
	      
	      if [ ! -z "$(echo "$2" |grep -i "$k")" ]; then
	         match=$(( $match + 1 ))
	      fi
	      matchs=$(( $matchs + 1 ))
	   done
	   if [ "$match" = "$matchs" ]; then
	   	  match="false"
	   else
	      match="true"
	   fi
	else
	   if [ ! -z "$(echo "$2" |grep -i "$1")" ]; then
	      match="false"
	   fi
	fi
}

cfg_new_servers_groups_get()
{
	 if [ -z "$1" ]; then
      return
   fi
   
   ${uci_add}groups="${1}"
}
	   
echo "开始更新【$CONFIG_NAME】的服务器节点配置..." >$START_LOG

[ "$servers_update" -eq 1 ] && {
echo "" >"$match_servers"
server_num=0
config_load "openclash"
config_foreach yml_servers_name_get "servers"
}

while [ "$count" -lt "$num" ]
do
   #name
   server_name=$(ruby_read "$proxy_hash" "['proxies'][$count]['name']")
   
   if [ -z "$server_name" ]; then
      let count++
      continue
   fi
   
   config_load "openclash"
   config_foreach server_key_get "config_subscribe"
   
   #匹配关键字订阅节点
   if [ "$servers_if_update" = "1" ]; then
      if [ -n "$config_keyword" ] || [ --n "$config_ex_keyword" ]; then
         if [ -n "$config_keyword" ] && [ -z "$config_ex_keyword" ]; then
            match="false"
            config_list_foreach "$key_section" "keyword" server_key_match "$server_name"
         elif [ -z "$config_keyword" ] && [ ! -z "$config_ex_keyword" ]; then
         	  match="true"
            config_list_foreach "$key_section" "ex_keyword" server_key_exmatch "$server_name"
         elif [ ! -z "$config_keyword" ] && [ ! -z "$config_ex_keyword" ]; then
            match="false"
            config_list_foreach "$key_section" "keyword" server_key_match "$server_name"
            config_list_foreach "$key_section" "ex_keyword" server_key_exmatch "$server_name"
         fi

         if [ "$match" = "false" ]; then
            echo "跳过【$server_name】服务器节点..." >$START_LOG
            let count++
            continue
         fi
      fi
   fi
   
#节点存在时获取节点编号
   server_num=$(grep -Fw "$server_name" "$match_servers" 2>/dev/null|awk -F '.' '{print $1}')
   if [ "$servers_update" -eq 1 ] && [ -n "$server_num" ]; then
      sed -i "/^${server_num}\./c\#match#" "$match_servers" 2>/dev/null
   fi
   
   #type
   server_type=$(ruby_read "$proxy_hash" "['proxies'][$count]['type']")
   #server
   server=$(ruby_read "$proxy_hash" "['proxies'][$count]['server']")
   #port
   port=$(ruby_read "$proxy_hash" "['proxies'][$count]['port']")
   #udp
   udp=$(ruby_read "$proxy_hash" "['proxies'][$count]['udp']")
   
   if [ "$server_type" = "ss" ]; then
      #cipher
      cipher=$(ruby_read "$proxy_hash" "['proxies'][$count]['cipher']")
      #password
      password=$(ruby_read "$proxy_hash" "['proxies'][$count]['password']")
      #plugin:
      plugin=$(ruby_read "$proxy_hash" "['proxies'][$count]['plugin']")
      #path:
      path=$(ruby_read "$proxy_hash" "['proxies'][$count]['plugin-opts']['path']")
      #mode:
      mode=$(ruby_read "$proxy_hash" "['proxies'][$count]['plugin-opts']['mode']")
      #host:
      host=$(ruby_read "$proxy_hash" "['proxies'][$count]['plugin-opts']['host']")
      #mux:
      mux=$(ruby_read "$proxy_hash" "['proxies'][$count]['plugin-opts']['mux']")
      #headers_custom:
      headers=$(ruby_read "$proxy_hash" "['proxies'][$count]['plugin-opts']['headers']['custom']")
      #obfs:
      obfs=$(ruby_read "$proxy_hash" "['proxies'][$count]['obfs']")
      #obfs-host:
      obfs_host=$(ruby_read "$proxy_hash" "['proxies'][$count]['obfs-host']")
      #tls:
      tls=$(ruby_read "$proxy_hash" "['proxies'][$count]['plugin-opts']['tls']")
      #skip-cert-verify:
      verify=$(ruby_read "$proxy_hash" "['proxies'][$count]['plugin-opts']['skip-cert-verify']")
   fi
   
   if [ "$server_type" = "ssr" ]; then
      #cipher
      cipher=$(ruby_read "$proxy_hash" "['proxies'][$count]['cipher']")
      #password
      password=$(ruby_read "$proxy_hash" "['proxies'][$count]['password']")
      #obfs:
      obfs=$(ruby_read "$proxy_hash" "['proxies'][$count]['obfs']")
      #protocol:
      protocol=$(ruby_read "$proxy_hash" "['proxies'][$count]['protocol']")
      #obfs-param:
      obfs_param=$(ruby_read "$proxy_hash" "['proxies'][$count]['obfs-param']")
      #protocol-param:
      protocol_param=$(ruby_read "$proxy_hash" "['proxies'][$count]['protocol-param']")
   fi
   
   if [ "$server_type" = "vmess" ]; then
      #uuid:
      uuid=$(ruby_read "$proxy_hash" "['proxies'][$count]['uuid']")
      #alterId:
      alterId=$(ruby_read "$proxy_hash" "['proxies'][$count]['alterId']")
      #cipher
      cipher=$(ruby_read "$proxy_hash" "['proxies'][$count]['cipher']")
      #servername
      servername=$(ruby_read "$proxy_hash" "['proxies'][$count]['servername']")
      #network:
      network=$(ruby_read "$proxy_hash" "['proxies'][$count]['network']")
      #ws-path:
      ws_path=$(ruby_read "$proxy_hash" "['proxies'][$count]['ws-path']")
      #Host:
      Host=$(ruby_read "$proxy_hash" "['proxies'][$count]['ws-headers']['Host']")
      #http_paths:
      http_paths=$(ruby_read "$proxy_hash" "['proxies'][$count]['http-opts']['path']")
      #tls:
      tls=$(ruby_read "$proxy_hash" "['proxies'][$count]['tls']")
      #skip-cert-verify:
      verify=$(ruby_read "$proxy_hash" "['proxies'][$count]['skip-cert-verify']")
      #keep-alive
      keep_alive=$(ruby_read "$proxy_hash" "['proxies'][$count]['http-opts']['headers']['Connection']")
   fi
   
   if [ "$server_type" = "socks5" ] || [ "$server_type" = "http" ]; then
      #username:
      username=$(ruby_read "$proxy_hash" "['proxies'][$count]['username']")
      #password
      password=$(ruby_read "$proxy_hash" "['proxies'][$count]['password']")
      #tls:
      tls=$(ruby_read "$proxy_hash" "['proxies'][$count]['tls']")
      #skip-cert-verify:
      verify=$(ruby_read "$proxy_hash" "['proxies'][$count]['skip-cert-verify']")
   fi
   
   if [ "$server_type" = "http" ]; then
      #sni:
      sni=$(ruby_read "$proxy_hash" "['proxies'][$count]['sni']")
   fi
   
   if [ "$server_type" = "snell" ]; then
      #psk:
      psk=$(ruby_read "$proxy_hash" "['proxies'][$count]['psk']")
      #mode:
      mode=$(ruby_read "$proxy_hash" "['proxies'][$count]['obfs-opts']['mode']")
      #host:
      host=$(ruby_read "$proxy_hash" "['proxies'][$count]['obfs-opts']['host']")
   fi
   
   if [ "$server_type" = "trojan" ]; then
      #password
      password=$(ruby_read "$proxy_hash" "['proxies'][$count]['password']")
      #sni:
      sni=$(ruby_read "$proxy_hash" "['proxies'][$count]['sni']")
      #alpn:
      alpns=$(ruby_read "$proxy_hash" "['proxies'][$count]['alpn']")
      #skip-cert-verify:
      verify=$(ruby_read "$proxy_hash" "['proxies'][$count]['skip-cert-verify']")
   fi

   echo "正在读取【$CONFIG_NAME】-【$server_type】-【$server_name】服务器节点配置..." > "$START_LOG"
   
   if [ "$servers_update" -eq 1 ] && [ ! -z "$server_num" ]; then
#更新已有节点
      uci_set="uci -q set openclash.@servers["$server_num"]."
      uci_add="uci -q add_list $name.$uci_name_tmp."
      uci_del="uci -q del_list $name.$uci_name_tmp."
      
      ${uci_set}manual="0"
      ${uci_set}type="$server_type"
      ${uci_set}server="$server"
      ${uci_set}port="$port"
      ${uci_set}udp="$udp"
      ${uci_set}tls="$tls"
      ${uci_set}skip_cert_verify="$verify"
      
      if [ "$server_type" = "ss" ]; then
      	 ${uci_set}cipher="$cipher"
      	 ${uci_set}password="$password"
         ${uci_set}obfs="$obfs"
         ${uci_set}host="$obfs_host"
         ${uci_set}path="$path"
         ${uci_set}mux="$mux"
         ${uci_set}custom="$headers"
         [ -z "$obfs" ] && ${uci_set}obfs="$mode"
         [ -z "$obfs" ] && [ -z "$mode" ] && ${uci_set}obfs="none"
      fi
      [ -z "$obfs_host" ] && ${uci_set}host="$host"
      
      if [ "$server_type" = "ssr" ]; then
      	 ${uci_set}cipher_ssr="$cipher"
      	 ${uci_set}password="$password"
         ${uci_set}obfs_ssr="$obfs"
         ${uci_set}obfs_param="$obfs_param"
         ${uci_set}protocol="$protocol"
         ${uci_set}protocol_param="$protocol_param"

      fi
      
      if [ "$server_type" = "snell" ]; then
      	 ${uci_set}obfs_snell="$mode"
         [ -z "$mode" ] && ${uci_set}obfs_snell="none"
         ${uci_set}psk="$psk"
      fi

      if [ "$server_type" = "vmess" ]; then
         ${uci_set}securitys="$cipher"
         ${uci_set}alterId="$alterId"
         ${uci_set}uuid="$uuid"
         ${uci_set}servername="$servername"
         if [ "$network" = "ws" ]; then
            ${uci_set}obfs_vmess="websocket"
            ${uci_set}path="$ws_path"
            ${uci_set}custom="$Host"
         elif [ "$network" = "http" ]; then
            ${uci_set}obfs_vmess="http"
            ${uci_del}http_path >/dev/null 2>&1
            for http_path in $http_paths; do
               ${uci_add}http_path="$http_path" >/dev/null 2>&1
            done
            if [ "$keep_alive" = "keep-alive" ]; then
               ${uci_set}keep_alive="true"
            else
               ${uci_set}keep_alive="false"
            fi
         else
            ${uci_set}obfs_vmess="none"
         fi
      fi
      
	    if [ "$server_type" = "socks5" ] || [ "$server_type" = "http" ]; then
         ${uci_set}auth_name="$username"
         ${uci_set}auth_pass="$password"
      else
         ${uci_set}password="$password"
	    fi
	    
      if [ "$server_type" = "http" ]; then
         ${uci_set}sni="$sni"
      fi
	   
	    if [ "$server_type" = "trojan" ]; then
         ${uci_set}sni="$sni"
         ${uci_del}alpn >/dev/null 2>&1
         for alpn in $alpns; do
            ${uci_add}alpn="$alpn" >/dev/null 2>&1
         done
	    fi
   else
#添加新节点
      name=openclash
      uci_name_tmp=$(uci add $name servers)

      uci_set="uci -q set $name.$uci_name_tmp."
      uci_add="uci -q add_list $name.$uci_name_tmp."

      if [ -z "$new_servers_group" ] && [ "$servers_if_update" = "1" ] && [ "$servers_update" -eq 1 ]; then
         ${uci_set}enabled="0"
      else
         ${uci_set}enabled="1"
      fi
      if [ "$servers_if_update" = "1" ]; then
         ${uci_set}manual="0"
      else
         ${uci_set}manual="1"
      fi
      ${uci_set}config="$CONFIG_NAME"
      ${uci_set}name="$server_name"
      ${uci_set}type="$server_type"
      ${uci_set}server="$server"
      ${uci_set}port="$port"
      ${uci_set}udp="$udp"
      ${uci_set}tls="$tls"
      ${uci_set}skip_cert_verify="$verify"
      
      if [ "$server_type" = "ss" ]; then
      	 ${uci_set}cipher="$cipher"
      	 ${uci_set}password="$password"
         ${uci_set}obfs="$obfs"
         ${uci_set}host="$obfs_host"
         ${uci_set}path="$path"
         ${uci_set}mux="$mux"
         ${uci_set}custom="$headers"
         [ -z "$obfs" ] && ${uci_set}obfs="$mode"
         [ -z "$obfs" ] && [ -z "$mode" ] && ${uci_set}obfs="none"
      fi
      [ -z "$obfs_host" ] && ${uci_set}host="$host"
      
      if [ "$server_type" = "ssr" ]; then
      	 ${uci_set}cipher_ssr="$cipher"
      	 ${uci_set}password="$password"
         ${uci_set}obfs_ssr="$obfs"
         ${uci_set}obfs_param="$obfs_param"
         ${uci_set}protocol="$protocol"
         ${uci_set}protocol_param="$protocol_param"

      fi
      
      if [ "$server_type" = "snell" ]; then
      	 ${uci_set}obfs_snell="$mode"
         [ -z "$mode" ] && ${uci_set}obfs_snell="none"
         ${uci_set}psk="$psk"
      fi

      if [ "$server_type" = "vmess" ]; then
         ${uci_set}securitys="$cipher"
         ${uci_set}alterId="$alterId"
         ${uci_set}uuid="$uuid"
         ${uci_set}servername="$servername"
         if [ "$network" = "ws" ]; then
            ${uci_set}obfs_vmess="websocket"
            ${uci_set}path="$ws_path"
            ${uci_set}custom="$Host"
         elif [ "$network" = "http" ]; then
            ${uci_set}obfs_vmess="http"
            ${uci_del}http_path >/dev/null 2>&1
            for http_path in $http_paths; do
               ${uci_add}http_path="$http_path" >/dev/null 2>&1
            done
            if [ "$keep_alive" = "keep-alive" ]; then
               ${uci_set}keep_alive="true"
            else
               ${uci_set}keep_alive="false"
            fi
         else
            ${uci_set}obfs_vmess="none"
         fi
      fi
      
	    if [ "$server_type" = "socks5" ] || [ "$server_type" = "http" ]; then
         ${uci_set}auth_name="$username"
         ${uci_set}auth_pass="$password"
      else
         ${uci_set}password="$password"
	    fi
	    
	    if [ "$server_type" = "http" ]; then
         ${uci_set}sni="$sni"
      fi
	   
	    if [ "$server_type" = "trojan" ]; then
         ${uci_set}sni="$sni"
         ${uci_del}alpn >/dev/null 2>&1
         for alpn in $alpns; do
            ${uci_add}alpn="$alpn" >/dev/null 2>&1
         done
	    fi

#加入策略组
     if [ "$servers_if_update" = "1" ] && [ -n "$new_servers_group" ] && [ "$config_group_exist" -eq 1 ]; then
#新节点且设置默认策略组时加入指定策略组
        config_load "openclash"
        config_list_foreach "config" "new_servers_group" cfg_new_servers_groups_get
     else
        for ((i=0;i<$group_count;i++))
        do
           group_type=$(ruby_read "$group_hash" "['proxy-groups'][$i]['type']")
           proxies=$(ruby_read "$group_hash" "['proxy-groups'][$i]['proxies']")
           if "$(ruby_read "$group_hash" "['proxy-groups'][$i]['proxies'].include?('$server_name')")"; then
           	  group_name=$(ruby_read "$group_hash" "['proxy-groups'][$i]['name']")
              if [ "$group_type" = "relay" ]; then
                 s=1
                 for server_relay in $proxies; do
                    if [ "$server_relay" = "$server_name" ]; then
                       ${uci_add}groups="$group_name"
                       ${uci_add}relay_groups="$group_name#relay#$s"
                    else
                       let s++
                    fi
                 done 2>/dev/null
              else
                 ${uci_add}groups="$group_name"
              fi
           fi
	      done 2>/dev/null
     fi
   fi
   uci commit openclash
   let count++
done 2>/dev/null

#删除订阅中已不存在的节点
if [ "$servers_if_update" = "1" ]; then
     echo "删除【$CONFIG_NAME】订阅中已不存在的节点..." >$START_LOG
     sed -i '/#match#/d' "$match_servers" 2>/dev/null
     cat $match_servers |awk -F '.' '{print $1}' |sort -rn |while read line
     do
        if [ -z "$line" ]; then
           continue
        fi
        if [ "$(uci get openclash.@servers["$line"].manual 2>/dev/null)" = "0" ] && [ "$(uci get openclash.@servers["$line"].config 2>/dev/null)" = "$CONFIG_NAME" ]; then
           uci delete openclash.@servers["$line"] 2>/dev/null
        fi
     done 2>/dev/null
fi

uci set openclash.config.servers_if_update=0
uci commit openclash
/usr/share/openclash/cfg_servers_address_fake_filter.sh
echo "配置文件【$CONFIG_NAME】读取完成！" >$START_LOG
sleep 3
echo "" >$START_LOG
rm -rf /tmp/match_servers.list 2>/dev/null
rm -rf /tmp/match_provider.list 2>/dev/null
rm -rf /tmp/yaml_other_group.yaml 2>/dev/null
