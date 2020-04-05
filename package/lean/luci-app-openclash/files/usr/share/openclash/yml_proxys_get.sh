#!/bin/bash /etc/rc.common
status=$(ps|grep -c /usr/share/openclash/yml_proxys_get.sh)
[ "$status" -gt "3" ] && exit 0

START_LOG="/tmp/openclash_start.log"
CONFIG_FILE=$(uci get openclash.config.config_path 2>/dev/null)
SERVER_RELAY="/tmp/relay_server"
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

#判断各个区位置
proxy_len=$(sed -n '/^ \{0,\}Proxy:/=' "$CONFIG_FILE" 2>/dev/null)
group_len=$(sed -n '/^ \{0,\}Proxy Group:/=' "$CONFIG_FILE" 2>/dev/null)
provider_len=$(sed -n '/^ \{0,\}proxy-provider:/=' "$CONFIG_FILE" 2>/dev/null)

if [ "$provider_len" -ge "$proxy_len" ] && [ "$provider_len" -le "$group_len" ]; then
   awk '/^ {0,}Proxy:/,/^ {0,}proxy-provider:/{print}' "$CONFIG_FILE" 2>/dev/null |sed 's/\"//g' 2>/dev/null |sed "s/\'//g" 2>/dev/null |sed 's/\t/ /g' 2>/dev/null >/tmp/yaml_proxy.yaml 2>&1
   awk '/^ {0,}proxy-provider:/,/^ {0,}Proxy Group:/{print}' "$CONFIG_FILE" 2>/dev/null |sed 's/\"//g' 2>/dev/null |sed "s/\'//g" 2>/dev/null |sed 's/\t/ /g' 2>/dev/null >/tmp/yaml_provider.yaml 2>&1
elif [ "$provider_len" -le "$proxy_len" ]; then
   awk '/^ {0,}Proxy:/,/^ {0,}Proxy Group:/{print}' "$CONFIG_FILE" 2>/dev/null |sed 's/\"//g' 2>/dev/null |sed "s/\'//g" 2>/dev/null |sed 's/\t/ /g' 2>/dev/null >/tmp/yaml_proxy.yaml 2>&1
   awk '/^ {0,}proxy-provider:/,/^ {0,}Proxy:/{print}' "$CONFIG_FILE" 2>/dev/null |sed 's/\"//g' 2>/dev/null |sed "s/\'//g" 2>/dev/null |sed 's/\t/ /g' 2>/dev/null >/tmp/yaml_provider.yaml 2>&1
elif [ "$provider_len" -ge "$group_len" ]; then
	 awk '/^ {0,}Proxy:/,/^ {0,}Proxy Group:/{print}' "$CONFIG_FILE" 2>/dev/null |sed 's/\"//g' 2>/dev/null |sed "s/\'//g" 2>/dev/null |sed 's/\t/ /g' 2>/dev/null >/tmp/yaml_proxy.yaml 2>&1
   awk '/^ {0,}proxy-provider:/,/^ {0,}Rule:/{print}' "$CONFIG_FILE" 2>/dev/null |sed 's/\"//g' 2>/dev/null |sed "s/\'//g" 2>/dev/null |sed 's/\t/ /g' 2>/dev/null >/tmp/yaml_provider.yaml 2>&1
elif [ "$provider_len" -le "$group_len" ]; then
   awk '/^ {0,}proxy-provider:/,/^ {0,}Proxy Group:/{print}' "$CONFIG_FILE" 2>/dev/null |sed 's/\"//g' 2>/dev/null |sed "s/\'//g" 2>/dev/null |sed 's/\t/ /g' 2>/dev/null >/tmp/yaml_provider.yaml 2>&1
else
   awk '/^ {0,}Proxy:/,/^ {0,}Proxy Group:/{print}' "$CONFIG_FILE" 2>/dev/null |sed 's/\"//g' 2>/dev/null |sed "s/\'//g" 2>/dev/null |sed 's/\t/ /g' 2>/dev/null >/tmp/yaml_proxy.yaml 2>&1
fi

CFG_FILE="/etc/config/openclash"
server_file="/tmp/yaml_proxy.yaml"
provider_file="/tmp/yaml_provider.yaml"
single_server="/tmp/servers.yaml"
single_provider="/tmp/provider.yaml"
single_provider_gen="/tmp/provider_gen.yaml"
single_provider_che="/tmp/provider_che.yaml"
match_servers="/tmp/match_servers.list"
match_provider="/tmp/match_provider.list"
group_num=$(grep -c "name:" /tmp/yaml_group.yaml 2>/dev/null)
servers_update=$(uci get openclash.config.servers_update 2>/dev/null)
servers_if_update=$(uci get openclash.config.servers_if_update 2>/dev/null)
new_servers_group=$(uci get openclash.config.new_servers_group 2>/dev/null)

#proxy
sed -i "s/\'//g" $server_file 2>/dev/null
sed -i 's/\"//g' $server_file 2>/dev/null
line=$(sed -n '/name:/=' $server_file 2>/dev/null)
num=$(grep -c "name:" $server_file 2>/dev/null)
count=1

#provider
sed -i "s/\'//g" $provider_file 2>/dev/null
sed -i 's/\"//g' $provider_file 2>/dev/null
sed -i '/^ *$/d' $provider_file 2>/dev/null
sed -i '/^ \{0,\}#/d' $provider_file 2>/dev/null
sed -i 's/\t/ /g' $provider_file 2>/dev/null
provider_line=$(awk '{print $0"#*#"FNR}' $provider_file 2>/dev/null |grep -v '^ \{0,\}proxy-provider:\|^ \{0,\}Proxy:\|^ \{0,\}Proxy Group:\|^ \{0,\}Rule:\|^ \{0,\}type:\|^ \{0,\}path:\|^ \{0,\}url:\|^ \{0,\}interval:\|^ \{0,\}health-check:\|^ \{0,\}enable:' |awk -F '#*#' '{print $3}')
provider_num=$(grep -c "^ \{0,\}type:" $provider_file 2>/dev/null)
provider_count=1

cfg_get()
{
	echo "$(grep "$1" "$2" 2>/dev/null |awk -v tag=$1 'BEGIN{FS=tag} {print $2}' 2>/dev/null |sed 's/,.*//' 2>/dev/null |sed 's/\}.*//' 2>/dev/null |sed 's/^ \{0,\}//g' 2>/dev/null |sed 's/ \{0,\}$//g' 2>/dev/null)"
}

cfg_get_dynamic()
{
	echo "$(grep "^ \{0,\}$1" "$2" 2>/dev/null |grep -v "^ \{0,\}- name:"  |grep -v "^ \{0,\}- keep-alive" |awk -v tag=$1 'BEGIN{FS=tag} {print $2}' 2>/dev/null |sed 's/,.*//' 2>/dev/null |sed 's/\}.*//' 2>/dev/null |sed 's/^ \{0,\}//g' 2>/dev/null |sed 's/ \{0,\}$//g' 2>/dev/null)"
}

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

[ "$servers_update" -eq "1" ] && {
echo "" >"$match_provider"
provider_nums=0
config_load "openclash"
config_foreach yml_provider_name_get "proxy-provider"
}

for n in $provider_line
do
   [ "$provider_count" -eq 1 ] && {
      startLine="$n"
   }
   
   provider_count=$(expr "$provider_count" + 1)
   if [ "$provider_count" -gt "$provider_num" ]; then
      endLine=$(sed -n '$=' $provider_file)
   else
      endLine=$(expr $(echo "$provider_line" | sed -n "${provider_count}p") - 1)
   fi

   sed -n "${startLine},${endLine}p" $provider_file >$single_provider
   health_check_line=$(sed -n '/^ \{0,\}health-check:/=' $single_provider)
   sed -n "1,${health_check_line}p" $single_provider >$single_provider_gen
   sed -n "${health_check_line},\$p" $single_provider >$single_provider_che
   
   startLine=$(expr "$endLine" + 1)

   #name
   provider_name="$(sed -n "${n}p" $provider_file |awk -F ':' '{print $1}' |sed 's/^ \{0,\}//g' 2>/dev/null |sed 's/ \{0,\}$//g' 2>/dev/null)"
   
   #type
   provider_type="$(cfg_get "type:" "$single_provider_gen")"
   
   #path
   provider_path="$(cfg_get "path:" "$single_provider_gen")"
   
   #gen_url
   provider_gen_url="$(cfg_get "url:" "$single_provider_gen")"
   
   #gen_interval
   provider_gen_interval="$(cfg_get "interval:" "$single_provider_gen")"
   
   #che_enable
   provider_che_enable="$(cfg_get "enable:" "$single_provider_che")"
   
   #che_url
   provider_che_url="$(cfg_get "url:" "$single_provider_che")"
   
   #che_interval
   provider_che_interval="$(cfg_get "interval:" "$single_provider_che")"
   
   echo "正在读取【$CONFIG_NAME】-【$provider_name】代理集配置..." >$START_LOG
   
   #代理集存在时获取代理集编号
   provider_nums=$(grep -Fw "$provider_name" "$match_provider" |awk -F '.' '{print $1}')
   if [ "$servers_update" -eq "1" ] && [ ! -z "$provider_nums" ]; then
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
   
      if [ -z "$new_servers_group" ] && [ "$servers_if_update" = "1" ] && [ "$servers_update" -eq "1" ]; then
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
      if [ "$servers_if_update" = "1" ] && [ ! -z "$new_servers_group" ] && [ ! -z "$(grep "config groups" "$CFG_FILE")" ]; then
#新代理集且设置默认策略组时加入指定策略组
         config_load "openclash"
         config_list_foreach "config" "new_servers_group" cfg_new_provider_groups_get
      else
         for ((i=1;i<=$group_num;i++))
         do
            single_group="/tmp/group_$i.yaml"
            use_line=$(sed -n '/^ \{0,\}use:/=' $single_group)
            proxies_line=$(sed -n '/^ \{0,\}proxies:/=' $single_group)
            if [ "$use_line" -le "$proxies_line" ]; then
               if [ ! -z "$(sed -n "${use_line},${proxies_line}p" "$single_group" |grep -F "$provider_name")" ]; then
                  group_name=$(cfg_get "name:" "$single_group")
                  ${uci_add}groups="$group_name"
               fi
            elif [ "$use_line" -ge "$proxies_line" ]; then
               if [ ! -z "$(sed -n "${use_line},\$p" "$single_group" |grep -F "$provider_name")" ]; then
                  group_name=$(cfg_get "name:" "$single_group")
                  ${uci_add}groups="$group_name"
               fi
            elif [ ! -z "$use_line" ] && [ -z "$proxies_line" ]; then
         	     if [ ! -z "$(grep -F "$provider_name" $single_group)" ]; then
                  group_name=$(cfg_get "name:" "$single_group")
                  ${uci_add}groups="$group_name"
               fi
            fi 2>/dev/null
	       done
	    fi
   fi
   uci commit openclash
done

#删除订阅中已不存在的代理集
if [ "$servers_if_update" = "1" ]; then
     echo "删除【$CONFIG_NAME】订阅中已不存在的代理集..." >$START_LOG
     sed -i '/#match#/d' "$match_provider" 2>/dev/null
     cat $match_provider |awk -F '.' '{print $1}' |sort -rn |while read line
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

[ "$servers_update" -eq "1" ] && {
echo "" >"$match_servers"
server_num=0
config_load "openclash"
config_foreach yml_servers_name_get "servers"
}

for n in $line
do

   [ "$count" -eq 1 ] && {
      startLine="$n"
   }

   count=$(expr "$count" + 1)
   if [ "$count" -gt "$num" ]; then
      endLine=$(sed -n '$=' $server_file)
   else
      endLine=$(expr $(echo "$line" | sed -n "${count}p") - 1)
   fi

   sed -n "${startLine},${endLine}p" $server_file >$single_server
   startLine=$(expr "$endLine" + 1)
   
   #name
   server_name="$(cfg_get "name:" "$single_server")"

   config_load "openclash"
   config_foreach server_key_get "config_subscribe"
   
#匹配关键字订阅节点
   if [ "$servers_if_update" = "1" ]; then
      if [ ! -z "$config_keyword" ] || [ ! -z "$config_ex_keyword" ]; then
         if [ ! -z "$config_keyword" ] && [ -z "$config_ex_keyword" ]; then
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
            continue
         fi
      fi
   fi
   
#节点存在时获取节点编号
   server_num=$(grep -Fw "$server_name" "$match_servers" |awk -F '.' '{print $1}')
   if [ "$servers_update" -eq "1" ] && [ ! -z "$server_num" ]; then
      sed -i "/^${server_num}\./c\#match#" "$match_servers" 2>/dev/null
   fi
   
   #type
   server_type="$(cfg_get "type:" "$single_server")"
   #server
   server="$(cfg_get "server:" "$single_server")"
   #port
   port="$(cfg_get "port:" "$single_server")"
   #cipher
   cipher="$(cfg_get "cipher:" "$single_server")"
   #password
   password="$(cfg_get "password:" "$single_server")"
   #udp
   udp="$(cfg_get "udp:" "$single_server")"
   #plugin:
   plugin="$(cfg_get "plugin:" "$single_server")"
   #plugin-opts:
   plugin_opts="$(cfg_get "plugin-opts:" "$single_server")"
   #obfs:
   obfs="$(cfg_get "obfs:" "$single_server")"
   #psk:
   psk="$(cfg_get "psk:" "$single_server")"
   #obfs-host:
   obfs_host="$(cfg_get "obfs-host:" "$single_server")"
   #mode:
   mode="$(cfg_get "mode:" "$single_server")"
   #tls:
   tls="$(cfg_get "tls:" "$single_server")"
   #skip-cert-verify:
   verify="$(cfg_get "skip-cert-verify:" "$single_server")"
   #mux:
   mux="$(cfg_get "mux:" "$single_server")"
   #host:
   host="$(cfg_get "host:" "$single_server")"
   #Host:
   Host="$(cfg_get "Host:" "$single_server")"
   #path:
   path="$(cfg_get "path:" "$single_server")"
   #ws-path:
   ws_path="$(cfg_get "ws-path:" "$single_server")"
   #headers_custom:
   headers="$(cfg_get "custom:" "$single_server")"
   #uuid:
   uuid="$(cfg_get "uuid:" "$single_server")"
   #alterId:
   alterId="$(cfg_get "alterId:" "$single_server")"
   #network:
   network="$(cfg_get "network:" "$single_server")"
   #username:
   username="$(cfg_get "username:" "$single_server")"
   #sni:
   sni="$(cfg_get "sni:" "$single_server")"
   #alpn:
   alpns="$(cfg_get_dynamic "-" "$single_server")"
   #http_paths:
   http_paths="$(cfg_get_dynamic "-" "$single_server")"
   
   echo "正在读取【$CONFIG_NAME】-【$server_type】-【$server_name】服务器节点配置..." >$START_LOG
   
   if [ "$servers_update" -eq "1" ] && [ ! -z "$server_num" ]; then
#更新已有节点
      uci_set="uci -q set openclash.@servers["$server_num"]."
      uci_add="uci -q add_list $name.$uci_name_tmp."
      uci_del="uci -q del_list $name.$uci_name_tmp."
      
      ${uci_set}manual="0"
      ${uci_set}type="$server_type"
      ${uci_set}server="$server"
      ${uci_set}port="$port"
      if [ "$server_type" = "vmess" ]; then
         ${uci_set}securitys="$cipher"
      else
         ${uci_set}cipher="$cipher"
      fi
      ${uci_set}udp="$udp"
      ${uci_set}obfs="$obfs"
      ${uci_set}host="$obfs_host"
      [ -z "$mode" ] && [ "$server_type" = "snell" ] && ${uci_set}obfs_snell="$mode"
      [ -z "$obfs" ] && [ "$server_type" = "ss" ] && ${uci_set}obfs="$mode"
      [ -z "$obfs" ] && [ "$server_type" = "ss" ] && [ -z "$mode" ] && ${uci_set}obfs="none"
      [ -z "$mode" ] && [ "$server_type" = "snell" ] &&  ${uci_set}obfs_snell="none"
      [ -z "$mode" ] && [ "$network" = "ws" ] && [ "$server_type" = "vmess" ] && ${uci_set}obfs_vmess="websocket"
      [ -z "$mode" ] && [ "$network" = "http" ] && [ "$server_type" = "vmess" ] && ${uci_set}obfs_vmess="http"
      [ -z "$mode" ] && [ -z "$network" ] && [ "$server_type" = "vmess" ] && ${uci_set}obfs_vmess="none"
      [ -z "$obfs_host" ] && ${uci_set}host="$host"
      ${uci_set}psk="$psk"
      ${uci_set}tls="$tls"
      ${uci_set}skip_cert_verify="$verify"
      ${uci_set}path="$path"
      [ -z "$path" ] && [ "$network" = "ws" ] && ${uci_set}path="$ws_path"
      ${uci_set}mux="$mux"
      ${uci_set}custom="$headers"
      [ -z "$headers" ] && [ "$network" = "ws" ] && ${uci_set}custom="$Host"
    
	   if [ "$server_type" = "vmess" ]; then
       #v2ray
       ${uci_set}alterId="$alterId"
       ${uci_set}uuid="$uuid"
       ${uci_del}http_path >/dev/null 2>&1
       for http_path in $http_paths; do
          ${uci_add}http_path="$http_path" >/dev/null 2>&1
       done
       if [ ! -z "$(grep "^ \{0,\}- keep-alive" "$single_server")" ]; then
          ${uci_set}keep_alive="true"
       else
          ${uci_set}keep_alive="false"
       fi
	   fi
	
	   if [ "$server_type" = "socks5" ] || [ "$server_type" = "http" ]; then
        ${uci_set}auth_name="$username"
        ${uci_set}auth_pass="$password"
     else
        ${uci_set}password="$password"
	   fi
	   
	   if [ "$server_type" = "trojan" ]; then
       #trojan
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

      if [ -z "$new_servers_group" ] && [ "$servers_if_update" = "1" ] && [ "$servers_update" -eq "1" ]; then
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
      if [ "$server_type" = "vmess" ]; then
         ${uci_set}securitys="$cipher"
      else
         ${uci_set}cipher="$cipher"
      fi
      ${uci_set}udp="$udp"
      ${uci_set}obfs="$obfs"
      ${uci_set}host="$obfs_host"
      [ -z "$mode" ] && [ "$server_type" = "snell" ] && ${uci_set}obfs_snell="$mode"
      [ -z "$obfs" ] && [ "$server_type" = "ss" ] && ${uci_set}obfs="$mode"
      [ -z "$obfs" ] && [ "$server_type" = "ss" ] && [ -z "$mode" ] && ${uci_set}obfs="none"
      [ -z "$mode" ] && [ "$server_type" = "snell" ] &&  ${uci_set}obfs_snell="none"
      [ -z "$mode" ] && [ "$network" = "ws" ] && [ "$server_type" = "vmess" ] && ${uci_set}obfs_vmess="websocket"
      [ -z "$mode" ] && [ "$network" = "http" ] && [ "$server_type" = "vmess" ] && ${uci_set}obfs_vmess="http"
      [ -z "$mode" ] && [ -z "$network" ] && [ "$server_type" = "vmess" ] && ${uci_set}obfs_vmess="none"
      [ -z "$obfs_host" ] && ${uci_set}host="$host"
      ${uci_set}psk="$psk"
      ${uci_set}tls="$tls"
      ${uci_set}skip_cert_verify="$verify"
      ${uci_set}path="$path"
      [ -z "$path" ] && [ "$network" = "ws" ] && ${uci_set}path="$ws_path"
      ${uci_set}mux="$mux"
      ${uci_set}custom="$headers"
      [ -z "$headers" ] && [ "$network" = "ws" ] && ${uci_set}custom="$Host"
    
	   if [ "$server_type" = "vmess" ]; then
       #v2ray
       ${uci_set}alterId="$alterId"
       ${uci_set}uuid="$uuid"
       ${uci_del}http_path >/dev/null 2>&1
       for http_path in $http_paths; do
          ${uci_add}http_path="$http_path" >/dev/null 2>&1
       done
       if [ ! -z "$(grep "^ \{0,\}- keep-alive" "$single_server")" ]; then
          ${uci_set}keep_alive="true"
       else
          ${uci_set}keep_alive="false"
       fi
	   fi
	
	   if [ "$server_type" = "socks5" ] || [ "$server_type" = "http" ]; then
        ${uci_set}auth_name="$username"
        ${uci_set}auth_pass="$password"
     else
        ${uci_set}password="$password"
	   fi
	   
	   if [ "$server_type" = "trojan" ]; then
       #trojan
       ${uci_set}sni="$sni"
       for alpn in $alpns; do
        ${uci_add}alpn="$alpn" >/dev/null 2>&1
       done
	   fi

#加入策略组
     if [ "$servers_if_update" = "1" ] && [ ! -z "$new_servers_group" ] && [ ! -z "$(grep "config groups" "$CFG_FILE")" ]; then
#新节点且设置默认策略组时加入指定策略组
        config_load "openclash"
        config_list_foreach "config" "new_servers_group" cfg_new_servers_groups_get
     else
	      for ((i=1;i<=$group_num;i++))
	      do
	         single_group="/tmp/group_$i.yaml"
	         group_type="$(cfg_get "type:" "$single_group")"
	         if [ ! -z "$(grep -F "$server_name" "$single_group")" ] && [ "$group_type" = "relay" ]; then
	         	  group_name=$(cfg_get "name:" "$single_group")
	            grep "^ \{0,\}-" "$single_group" 2>/dev/null |grep -v "^ \{0,\}- name:" 2>/dev/null > $SERVER_RELAY
	            s=1
	            cat $SERVER_RELAY |while read -r line
	            do
	               if [ ! -z "$(echo "$line" |grep -F "$server_name" 2>/dev/null)" ]; then
                    ${uci_add}groups="$group_name"
                    ${uci_add}relay_groups="$group_name#relay#$s"
                 else
                    s=$(expr "$s" + 1)
                 fi
              done
           elif [ ! -z "$(grep -F "$server_name" "$single_group")" ]; then
              group_name=$(cfg_get "name:" "$single_group")
              ${uci_add}groups="$group_name"
           fi
	      done
     fi
   fi
   uci commit openclash
done

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
     done
fi

uci set openclash.config.servers_if_update=0
uci commit openclash
/usr/share/openclash/cfg_servers_address_fake_block.sh
echo "配置文件【$CONFIG_NAME】读取完成！" >$START_LOG
sleep 3
echo "" >$START_LOG
rm -rf /tmp/servers.yaml 2>/dev/null
rm -rf /tmp/yaml_proxy.yaml 2>/dev/null
rm -rf /tmp/group_*.yaml 2>/dev/null
rm -rf /tmp/yaml_group.yaml 2>/dev/null
rm -rf /tmp/match_servers.list 2>/dev/null
rm -rf /tmp/yaml_provider.yaml 2>/dev/null
rm -rf /tmp/provider.yaml 2>/dev/null
rm -rf /tmp/provider_gen.yaml 2>/dev/null
rm -rf /tmp/provider_che.yaml 2>/dev/null
rm -rf /tmp/match_provider.list 2>/dev/null
rm -rf /tmp/relay_server 2>/dev/null
