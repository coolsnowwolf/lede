#!/bin/bash
. /lib/functions.sh
. /usr/share/openclash/ruby.sh
. /usr/share/openclash/log.sh

set_lock() {
   exec 875>"/tmp/lock/openclash_proxies_get.lock" 2>/dev/null
   flock -x 875 2>/dev/null
}

del_lock() {
   flock -u 875 2>/dev/null
   rm -rf "/tmp/lock/openclash_proxies_get.lock"
}

sub_info_get()
{
   local section="$1" name
   config_get "name" "$section" "name" ""
   
   if [ -z "$name" ] || [ "$name" != "${CONFIG_NAME%%.*}" ]; then
      return
   else
      sub_cfg=true
   fi
}

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

#判断订阅配置
config_load "openclash"
config_foreach sub_info_get "config_subscribe"

#提取节点部分
proxy_hash=$(ruby_read "$CONFIG_FILE" ".select {|x| 'proxies' == x or 'proxy-providers' == x}")

CFG_FILE="/etc/config/openclash"
match_servers="/tmp/match_servers.list"
match_provider="/tmp/match_provider.list"
servers_update=$(uci get openclash.config.servers_update 2>/dev/null)
servers_if_update=$(uci get openclash.config.servers_if_update 2>/dev/null)

#proxy
num=$(ruby_read_hash "$proxy_hash" "['proxies'].count")
count=0

#provider
provider_num=$(ruby_read_hash "$proxy_hash" "['proxy-providers'].count")
provider_count=0

#group
group_hash=$(ruby_read "$CONFIG_FILE" ".select {|x| 'proxy-groups' == x}")

if [ -z "$num" ] && [ -z "$provider_num" ]; then
   LOG_OUT "Error: Unable To Parse Config File, Please Check And Try Again!"
   sleep 3
   del_lock
   exit 0
fi

cfg_new_servers_groups_check()
{
   config_group_exist=$(( $config_group_exist + 1 ))
   
   if [ -z "$1" ]; then
      return
   fi
   
   config_foreach cfg_group_name "groups" "$1"
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

   if [ "$name" = "$2" ]; then
      config_group_exists=$(( $config_group_exists + 1 ))
   fi
}

#判断当前配置文件策略组信息是否包含指定策略组
config_group_exist=0
config_group_exists=0
config_load "openclash"
config_list_foreach "config" "new_servers_group" cfg_new_servers_groups_check

if [ "$config_group_exists" -eq "$config_group_exist" ] && [ "$config_group_exist" -ne 0 ]; then
   config_group_exist=1
else
   config_group_exist=0
fi

LOG_OUT "Start Getting【$CONFIG_NAME】Proxy-providers Setting..."

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
   provider_name=$(ruby_read_hash "$proxy_hash" "['proxy-providers'].keys[$provider_count]")
   #type
   provider_type=$(ruby_read_hash "$proxy_hash" "['proxy-providers'].values[$provider_count]['type']")

   if [ -z "$provider_name" ] || [ -z "$provider_type" ]; then
      let provider_count++
      continue
   fi
   
   LOG_OUT "Start Getting【$CONFIG_NAME - $provider_name】Proxy-provider Setting..."

   #代理集存在时获取代理集编号
   provider_nums=$(grep -Fw "$provider_name" "$match_provider" 2>/dev/null|awk -F '.' '{print $1}')
   if [ "$servers_update" -eq 1 ] && [ -n "$provider_nums" ]; then
      sed -i "/^${provider_nums}\./c\#match#" "$match_provider" 2>/dev/null
      uci_set="uci -q set openclash.@proxy-provider["$provider_nums"]."
      ${uci_set}manual="0"
      ${uci_set}type="$provider_type"
   else
   #代理集不存在时添加新代理集
      name=openclash
      uci_name_tmp=$(uci add $name proxy-provider)
      uci_set="uci -q set $name.$uci_name_tmp."
      uci_add="uci -q add_list $name.$uci_name_tmp."
   
      if [ "$config_group_exist" -eq 0 ] && [ "$servers_if_update" = "1" ] && [ "$servers_update" -eq 1 ]; then
         ${uci_set}enabled="0"
      else
         ${uci_set}enabled="1"
      fi
      if [ "$servers_if_update" = "1" ] || "$sub_cfg"; then
         ${uci_set}manual="0"
      else
         ${uci_set}manual="1"
      fi
      ${uci_set}config="$CONFIG_NAME"
      ${uci_set}name="$provider_name"
      ${uci_set}type="$provider_type"
   fi
   ruby -ryaml -E UTF-8 -e "
   begin
   Value = $proxy_hash;
   Thread.new{
   #path
   if Value['proxy-providers'].values[$provider_count].key?('path') then
      if '$provider_type' == 'http' then
         provider_path = '${uci_set}path=./proxy_provider/${provider_name}.yaml'
      else
         provider_path = '${uci_set}path=\"' + Value['proxy-providers'].values[$provider_count]['path'].to_s + '\"'
      end
      system(provider_path)
   end
   }.join;
   
   Thread.new{
   #gen_url
   if Value['proxy-providers'].values[$provider_count].key?('url') then
      provider_gen_url = '${uci_set}provider_url=\"' + Value['proxy-providers'].values[$provider_count]['url'].to_s + '\"'
      system(provider_gen_url)
   end
   }.join;
   
   Thread.new{
   #gen_interval
   if Value['proxy-providers'].values[$provider_count].key?('interval') then
      provider_gen_interval = '${uci_set}provider_interval=' + Value['proxy-providers'].values[$provider_count]['interval'].to_s
      system(provider_gen_interval)
   end
   }.join;
   
   Thread.new{
   #che_enable
   if Value['proxy-providers'].values[$provider_count].key?('health-check') then
      if Value['proxy-providers'].values[$provider_count]['health-check'].key?('enable') then
         provider_che_enable = '${uci_set}health_check=' + Value['proxy-providers'].values[$provider_count]['health-check']['enable'].to_s
         system(provider_che_enable)
      end
   end
   }.join;
   
   Thread.new{
   #che_url
   if Value['proxy-providers'].values[$provider_count].key?('health-check') then
      if Value['proxy-providers'].values[$provider_count]['health-check'].key?('url') then
         provider_che_url = '${uci_set}health_check_url=\"' + Value['proxy-providers'].values[$provider_count]['health-check']['url'].to_s + '\"'
         system(provider_che_url)
      end
   end
   }.join;
   
   Thread.new{
   #che_interval
   if Value['proxy-providers'].values[$provider_count].key?('health-check') then
      if Value['proxy-providers'].values[$provider_count]['health-check'].key?('interval') then
         provider_che_interval = '${uci_set}health_check_interval=' + Value['proxy-providers'].values[$provider_count]['health-check']['interval'].to_s
         system(provider_che_interval)
      end
   end
   }.join;
      
   rescue Exception => e
   puts '${LOGTIME} Error: Resolve Proxy-provider Error,【${CONFIG_NAME} - ${provider_name}: ' + e.message + '】'
   end
   " 2>/dev/null >> $LOG_FILE &
      
   if [ "$servers_update" != 1 ] || [ -z "$provider_nums" ]; then
      #加入策略组
      if [ "$servers_if_update" == 1 ] && [ "$config_group_exist" == 1 ]; then
         #新代理集且设置默认策略组时加入指定策略组
         config_load "openclash"
         config_list_foreach "config" "new_servers_group" cfg_new_provider_groups_get
      else
         ruby -ryaml -E UTF-8 -e "
         Thread.new{
         begin
            Value = $group_hash
            Value['proxy-groups'].each{
            |x|
            if x.key?('use') then
               if x['use'].include?('$provider_name') then
                  uci = '${uci_add}groups=\"' + x['name'] + '\"'
                  system(uci)
               end
            end
            }
         rescue Exception => e
         puts '${LOGTIME} Error: Resolve Proxy-provider Error,【${CONFIG_NAME} - ${provider_name}: ' + e.message + '】'
         end
         }.join;
         " 2>/dev/null >> $LOG_FILE &
      fi
   fi
   let provider_count++
done 2>/dev/null


#删除订阅中已不存在的代理集
if [ "$servers_if_update" = "1" ]; then
     LOG_OUT "Deleting【$CONFIG_NAME】Proxy-providers That no Longer Exists in Subscription"
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

cfg_new_servers_groups_get()
{
	 if [ -z "$1" ]; then
      return
   fi

   ${uci_add}groups="${1}"
}
	   
LOG_OUT "Start Getting【$CONFIG_NAME】Proxies Setting..."

[ "$servers_update" -eq 1 ] && {
echo "" >"$match_servers"
server_num=0
config_load "openclash"
config_foreach yml_servers_name_get "servers"
}

while [ "$count" -lt "$num" ]
do
   #name
   server_name=$(ruby_read_hash "$proxy_hash" "['proxies'][$count]['name']")

   if [ -z "$server_name" ]; then
      let count++
      continue
   fi
   
#节点存在时获取节点编号
   server_num=$(grep -Fw "$server_name" "$match_servers" 2>/dev/null|awk -F '.' '{print $1}')
   if [ "$servers_update" -eq 1 ] && [ -n "$server_num" ]; then
      sed -i "/^${server_num}\./c\#match#" "$match_servers" 2>/dev/null
   fi
   
   #type
   server_type=$(ruby_read_hash "$proxy_hash" "['proxies'][$count]['type']")

   LOG_OUT "Start Getting【$CONFIG_NAME - $server_type - $server_name】Proxy Setting..."

   if [ "$servers_update" -eq 1 ] && [ ! -z "$server_num" ]; then
#更新已有节点
      uci_set="uci -q set openclash.@servers["$server_num"]."
      uci_add="uci -q add_list $name.$uci_name_tmp."
      uci_del="uci -q del_list $name.$uci_name_tmp."

      ${uci_set}manual="0"
      ${uci_set}name="$server_name"
      ${uci_set}type="$server_type"
   else
#添加新节点
      name=openclash
      uci_name_tmp=$(uci add $name servers)

      uci_set="uci -q set $name.$uci_name_tmp."
      uci_add="uci -q add_list $name.$uci_name_tmp."

      if [ "$config_group_exist" -eq 0 ] && [ "$servers_if_update" = "1" ] && [ "$servers_update" -eq 1 ]; then
         ${uci_set}enabled="0"
      else
         ${uci_set}enabled="1"
      fi
      if [ "$servers_if_update" = "1" ] || "$sub_cfg"; then
         ${uci_set}manual="0"
      else
         ${uci_set}manual="1"
      fi
      ${uci_set}config="$CONFIG_NAME"
      ${uci_set}name="$server_name"
      ${uci_set}type="$server_type"
   fi

   ruby -ryaml -E UTF-8 -e "
   begin
   Value = $proxy_hash;
   Thread.new{
   #server
   if Value['proxies'][$count].key?('server') then
      server = '${uci_set}server=\"' + Value['proxies'][$count]['server'].to_s + '\"'
      system(server)
   end
   }.join;
   
   Thread.new{
   #port
   if Value['proxies'][$count].key?('port') then
      port = '${uci_set}port=' + Value['proxies'][$count]['port'].to_s
      system(port)
   end
   }.join;
   
   Thread.new{
   #udp
   if Value['proxies'][$count].key?('udp') then
      udp = '${uci_set}udp=' + Value['proxies'][$count]['udp'].to_s
      system(udp)
   end
   }.join;
   
   if '$server_type' == 'ss' then
      Thread.new{
      #cipher
      if Value['proxies'][$count].key?('cipher') then
         cipher = '${uci_set}cipher=' + Value['proxies'][$count]['cipher'].to_s
         system(cipher)
      end
      }.join
      
      Thread.new{
      #plugin-opts
      if Value['proxies'][$count].key?('plugin-opts') then
         #mode
         if Value['proxies'][$count]['plugin-opts'].key?('mode') then
            mode = '${uci_set}obfs=' + Value['proxies'][$count]['plugin-opts']['mode'].to_s
            system(mode)
         else
            mode = '${uci_set}obfs=none'
            system(mode)
         end
         #host:
         if Value['proxies'][$count]['plugin-opts'].key?('host') then
            host = '${uci_set}host=\"' + Value['proxies'][$count]['plugin-opts']['host'].to_s + '\"'
            system(host)
         end
         if Value['proxies'][$count]['plugin'].to_s == 'v2ray-plugin' then
            #path
            if Value['proxies'][$count]['plugin-opts'].key?('path') then
               path = '${uci_set}path=\"' + Value['proxies'][$count]['plugin-opts']['path'].to_s + '\"'
               system(path)
            end
            #mux
            if Value['proxies'][$count]['plugin-opts'].key?('mux') then
               mux = '${uci_set}mux=' + Value['proxies'][$count]['plugin-opts']['mux'].to_s
               system(mux)
            end
            #headers
            if Value['proxies'][$count]['plugin-opts'].key?('headers') then
               if Value['proxies'][$count]['plugin-opts']['headers'].key?('custom') then
                  custom = '${uci_set}custom=\"' + Value['proxies'][$count]['plugin-opts']['headers']['custom'].to_s + '\"'
                  system(custom)
               end
            end
            #tls
            if Value['proxies'][$count]['plugin-opts'].key?('tls') then
               tls = '${uci_set}tls=' + Value['proxies'][$count]['plugin-opts']['tls'].to_s
               system(tls)
            end
            #skip-cert-verify
            if Value['proxies'][$count]['plugin-opts'].key?('skip-cert-verify') then
               skip_cert_verify = '${uci_set}skip_cert_verify=' + Value['proxies'][$count]['plugin-opts']['skip-cert-verify'].to_s
               system(skip_cert_verify)
            end
         end
      end
      }.join
   end;
   if '$server_type' == 'ssr' then
      Thread.new{
      #cipher
      if Value['proxies'][$count].key?('cipher') then
         cipher = '${uci_set}cipher_ssr=' + Value['proxies'][$count]['cipher'].to_s
         system(cipher)
      end
      }.join
      
      Thread.new{
      #obfs
      if Value['proxies'][$count].key?('obfs') then
         obfs = '${uci_set}obfs_ssr=' + Value['proxies'][$count]['obfs'].to_s
         system(obfs)
      end
      }.join
      
      Thread.new{
      #protocol
      if Value['proxies'][$count].key?('protocol') then
         protocol = '${uci_set}protocol=' + Value['proxies'][$count]['protocol'].to_s
         system(protocol)
      end
      }.join
      
      Thread.new{
      #obfs-param
      if Value['proxies'][$count].key?('obfs-param') then
         obfs_param = '${uci_set}obfs_param=\"' + Value['proxies'][$count]['obfs-param'].to_s + '\"'
         system(obfs_param)
      end
      }.join
      
      Thread.new{
      #protocol-param
      if Value['proxies'][$count].key?('protocol-param') then
         protocol_param = '${uci_set}protocol_param=\"' + Value['proxies'][$count]['protocol-param'].to_s + '\"'
         system(protocol_param)
      end
      }.join
   end;
   if '$server_type' == 'vmess' then
      Thread.new{
      #uuid
      if Value['proxies'][$count].key?('uuid') then
         uuid = '${uci_set}uuid=' + Value['proxies'][$count]['uuid'].to_s
         system(uuid)
      end
      }.join
      
      Thread.new{
      #alterId
      if Value['proxies'][$count].key?('alterId') then
         alterId = '${uci_set}alterId=' + Value['proxies'][$count]['alterId'].to_s
         system(alterId)
      end
      }.join
      
      Thread.new{
      #cipher
      if Value['proxies'][$count].key?('cipher') then
         cipher = '${uci_set}securitys=' + Value['proxies'][$count]['cipher'].to_s
         system(cipher)
      end
      }.join
      
      Thread.new{
      #tls
      if Value['proxies'][$count].key?('tls') then
         tls = '${uci_set}tls=' + Value['proxies'][$count]['tls'].to_s
         system(tls)
      end
      }.join
      
      Thread.new{
      #skip-cert-verify
      if Value['proxies'][$count].key?('skip-cert-verify') then
         skip_cert_verify = '${uci_set}skip_cert_verify=' + Value['proxies'][$count]['skip-cert-verify'].to_s
         system(skip_cert_verify)
      end
      }.join
      
      Thread.new{
      #servername
      if Value['proxies'][$count].key?('servername') then
         servername = '${uci_set}servername=\"' + Value['proxies'][$count]['servername'].to_s + '\"'
         system(servername)
      end
      }.join
      
      Thread.new{
      #network:
      if Value['proxies'][$count].key?('network') then
         if Value['proxies'][$count]['network'].to_s == 'ws'
            system '${uci_set}obfs_vmess=websocket'
            #ws-path:
            if Value['proxies'][$count].key?('ws-path') then
               path = '${uci_set}path=\"' + Value['proxies'][$count]['ws-path'].to_s + '\"'
               system(path)
            end
            #Host:
            if Value['proxies'][$count].key?('ws-headers') then
               if Value['proxies'][$count]['ws-headers'].key?('Host') then
                  custom = '${uci_set}custom=\"' + Value['proxies'][$count]['ws-headers']['Host'].to_s + '\"'
                  system(custom)
               end
            end
            #ws-opts-path:
            if Value['proxies'][$count].key?('ws-opts') then
               if Value['proxies'][$count]['ws-opts'].key?('path') then
                  ws_opts_path = '${uci_set}ws_opts_path=\"' + Value['proxies'][$count]['ws-opts']['path'].to_s + '\"'
                  system(ws_opts_path)
               end
               #ws-opts-headers:
               if Value['proxies'][$count]['ws-opts'].key?('headers') then
                  ws_opts_headers = '${uci_set}ws_opts_headers=\"' + Value['proxies'][$count]['ws-opts']['headers'].to_s + '\"'
                  system(ws_opts_headers)
               end
               #max-early-data:
               if Value['proxies'][$count]['ws-opts'].key?('max-early-data') then
                  max_early_data = '${uci_set}max_early_data=\"' + Value['proxies'][$count]['ws-opts']['max-early-data'].to_s + '\"'
                  system(max_early_data)
               end
               #early-data-header-name:
               if Value['proxies'][$count]['ws-opts'].key?('early-data-header-name') then
                  early_data_header_name = '${uci_set}early_data_header_name=\"' + Value['proxies'][$count]['ws-opts']['early-data-header-name'].to_s + '\"'
                  system(early_data_header_name)
               end
            end
         elsif Value['proxies'][$count]['network'].to_s == 'http'
            system '${uci_set}obfs_vmess=http'
            if Value['proxies'][$count].key?('http-opts') then
               if Value['proxies'][$count]['http-opts'].key?('path') then
                  system '${uci_del}http_path >/dev/null 2>&1'
                  Value['proxies'][$count]['http-opts']['path'].each{
                  |x|
                  http_path = '${uci_add}http_path=\"' + x.to_s + '\"'
                  system(http_path)
                  }
               end
               if Value['proxies'][$count]['http-opts'].key?('headers') then
                  if Value['proxies'][$count]['http-opts']['headers'].key?('Connection') then
                     if Value['proxies'][$count]['http-opts']['headers']['Connection'].include?('keep-alive') then
                        keep_alive = '${uci_set}keep_alive=true'
                     else
                        keep_alive = '${uci_set}keep_alive=false'
                     end
                     system(keep_alive)
                  end
               end
            end
         elsif Value['proxies'][$count]['network'].to_s == 'h2'
            system '${uci_set}obfs_vmess=h2'
            if Value['proxies'][$count].key?('h2-opts') then
               if Value['proxies'][$count]['h2-opts'].key?('host') then
                  system '${uci_del}h2_host >/dev/null 2>&1'
                  Value['proxies'][$count]['h2-opts']['host'].each{
                  |x|
                  h2_host = '${uci_add}h2_host=\"' + x.to_s + '\"'
                  system(h2_host)
                  }
               end
               if Value['proxies'][$count]['h2-opts'].key?('path') then
                  h2_path = '${uci_set}h2_path=\"' + Value['proxies'][$count]['h2-opts']['path'].to_s + '\"'
                  system(h2_path)
               end
            end
         elsif Value['proxies'][$count]['network'].to_s == 'grpc'
            #grpc-service-name
            system '${uci_set}obfs_vmess=grpc'
            if Value['proxies'][$count].key?('grpc-opts') then
               if Value['proxies'][$count]['grpc-opts'].key?('grpc-service-name') then
                  grpc_service_name = '${uci_set}grpc_service_name=\"' + Value['proxies'][$count]['grpc-opts']['grpc-service-name'].to_s + '\"'
                  system(grpc_service_name)
               end
            end
         else
            system '${uci_set}obfs_vmess=none'
         end
      end
      }.join
   end;
   if '$server_type' == 'snell' then
      Thread.new{
      if Value['proxies'][$count].key?('obfs-opts') then
         if Value['proxies'][$count]['obfs-opts'].key?('mode') then
            mode = '${uci_set}obfs_snell=' + Value['proxies'][$count]['obfs-opts']['mode'].to_s
            system(mode)
         else
            system '${uci_set}obfs_snell=none'
         end
         if Value['proxies'][$count]['obfs-opts'].key?('host') then
            host = '${uci_set}host=\"' + Value['proxies'][$count]['obfs-opts']['host'].to_s + '\"'
            system(host)
         end
      end
      }.join
      
      Thread.new{
      if Value['proxies'][$count].key?('psk') then
         psk = '${uci_set}psk=' + Value['proxies'][$count]['psk'].to_s
         system(psk)
      end
      }.join
   end;
   if '$server_type' == 'socks5' or '$server_type' == 'http' then
      Thread.new{
      if Value['proxies'][$count].key?('username') then
         username = '${uci_set}auth_name=\"' + Value['proxies'][$count]['username'].to_s + '\"'
         system(username)
      end
      }.join
      
      Thread.new{
      if Value['proxies'][$count].key?('password') then
         password = '${uci_set}auth_pass=\"' + Value['proxies'][$count]['password'].to_s + '\"'
         system(password)
      end
      }.join
      
      Thread.new{
      #tls
      if Value['proxies'][$count].key?('tls') then
         tls = '${uci_set}tls=' + Value['proxies'][$count]['tls'].to_s
         system(tls)
      end
      }.join
      
      Thread.new{
      #skip-cert-verify
      if Value['proxies'][$count].key?('skip-cert-verify') then
         skip_cert_verify = '${uci_set}skip_cert_verify=' + Value['proxies'][$count]['skip-cert-verify'].to_s
         system(skip_cert_verify)
      end
      }.join
   else
      Thread.new{
      if Value['proxies'][$count].key?('password') then
         password = '${uci_set}password=\"' + Value['proxies'][$count]['password'].to_s + '\"'
         system(password)
      end
      }.join
	 end;
   if '$server_type' == 'http' or '$server_type' == 'trojan' then
      Thread.new{
      if Value['proxies'][$count].key?('sni') then
         sni = '${uci_set}sni=\"' + Value['proxies'][$count]['sni'].to_s + '\"'
         system(sni)
      end
      }.join
   end;
   if '$server_type' == 'trojan' then
      Thread.new{
      #alpn
      if Value['proxies'][$count].key?('alpn') then
      system '${uci_del}alpn >/dev/null 2>&1'
      Value['proxies'][$count]['alpn'].each{
      |x|
      alpn = '${uci_add}alpn=\"' + x.to_s + '\"'
      system(alpn)
      }
      end
      }.join
      
      Thread.new{
      #grpc-service-name
      if Value['proxies'][$count].key?('grpc-opts') then
         if Value['proxies'][$count]['grpc-opts'].key?('grpc-service-name') then
            grpc_service_name = '${uci_set}grpc_service_name=\"' + Value['proxies'][$count]['grpc-opts']['grpc-service-name'].to_s + '\"'
            system(grpc_service_name)
         end
      end
      }.join
      
      Thread.new{
      #skip-cert-verify
      if Value['proxies'][$count].key?('skip-cert-verify') then
         skip_cert_verify = '${uci_set}skip_cert_verify=' + Value['proxies'][$count]['skip-cert-verify'].to_s
         system(skip_cert_verify)
      end
      }.join
   end;
   
   rescue Exception => e
   puts '${LOGTIME} Error: Resolve Proxy Error,【${CONFIG_NAME} - ${server_type} - ${server_name}: ' + e.message + '】'
   end
   " 2>/dev/null >> $LOG_FILE &
   
   if [ "$servers_update" != 1 ] || [ -z "$server_num" ]; then
      #加入策略组
      if [ "$servers_if_update" = 1 ] && [ "$config_group_exist" = 1 ]; then
         #新代理且设置默认策略组时加入指定策略组
         config_load "openclash"
         config_list_foreach "config" "new_servers_group" cfg_new_servers_groups_get
      else
         ruby -ryaml -E UTF-8 -e "
         Thread.new{
         begin
            Value = $group_hash
            #proxy
            Value['proxy-groups'].each{
            |x|
            if x.key?('proxies') then
               if x['proxies'].include?('$server_name') then
                  uci_proxy = '${uci_add}groups=\"' + x['name'] + '\"'
                  system(uci_proxy)
               end
            end
            }
            #relay
            Value['proxy-groups'].each{
            |x|
            if x['type'] == 'relay' then
               if x['proxies'].include?('$server_name') then
                  uci_relay = '${uci_add}relay_groups=\"' + x['name'] + '#relay#' + x['proxies'].index('$server_name') + '\"'
                  system(uci_relay)
               end
            end
            }
         rescue Exception => e
         puts '${LOGTIME} Error: Resolve Proxy Error,【${CONFIG_NAME} - ${server_type} - ${server_name}: ' + e.message + '】'
         end
         }.join;
         " 2>/dev/null >> $LOG_FILE &
      fi
   fi
   let count++
done 2>/dev/null

#删除订阅中已不存在的节点
if [ "$servers_if_update" = "1" ]; then
     LOG_OUT "Deleting【$CONFIG_NAME】Proxies That no Longer Exists in Subscription"
     sed -i '/#match#/d' "$match_servers" 2>/dev/null
     cat $match_servers |awk -F '.' '{print $1}' |sort -rn |while read -r line
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
wait
uci commit openclash
/usr/share/openclash/cfg_servers_address_fake_filter.sh
LOG_OUT "Config File【$CONFIG_NAME】Read Successful!"
sleep 3
SLOG_CLEAN
rm -rf /tmp/match_servers.list 2>/dev/null
rm -rf /tmp/match_provider.list 2>/dev/null
rm -rf /tmp/yaml_other_group.yaml 2>/dev/null
del_lock