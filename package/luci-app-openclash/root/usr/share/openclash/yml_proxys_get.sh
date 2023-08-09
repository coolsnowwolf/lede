#!/bin/bash
. /lib/functions.sh
. /usr/share/openclash/log.sh

set_lock() {
   exec 875>"/tmp/lock/openclash_proxies_get.lock" 2>/dev/null
   flock -x 875 2>/dev/null
}

del_lock() {
   flock -u 875 2>/dev/null
   rm -rf "/tmp/lock/openclash_proxies_get.lock"
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

#提取节点部分
proxy_hash=$(ruby_read "$CONFIG_FILE" ".select {|x| 'proxies' == x or 'proxy-providers' == x}")

CFG_FILE="/etc/config/openclash"
match_servers="/tmp/match_servers.list"
match_provider="/tmp/match_provider.list"
servers_update=$(uci -q get openclash.config.servers_update)
servers_if_update=$(uci -q get openclash.config.servers_if_update)

#proxy
num=$(ruby_read_hash "$proxy_hash" "['proxies'].count")
count=0

#provider
provider_num=$(ruby_read_hash "$proxy_hash" "['proxy-providers'].count")
provider_count=0

#group
group_hash=$(ruby_read "$CONFIG_FILE" ".select {|x| 'proxy-groups' == x}")

if [ -z "$group_hash" ]; then
	LOG_OUT "Error: Unable To Parse Config File, Please Check And Try Again!"
  del_lock
  exit 0
fi
	
if [ -z "$num" ] && [ -z "$provider_num" ]; then
   LOG_OUT "Error: Unable To Parse Config File, Please Check And Try Again!"
   del_lock
   exit 0
fi

cfg_new_servers_groups_check()
{
   
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
      config_group_exist=$(( $config_group_exist + 1 ))
   fi
}

#判断当前配置文件策略组信息是否包含指定策略组
config_group_exist=0
if [ -z "$(uci -q get openclash.config.new_servers_group)" ]; then
   config_group_exist=2
elif [ "$(uci -q get openclash.config.new_servers_group)" = "all" ]; then
   config_group_exist=1
else
   config_load "openclash"
   config_list_foreach "config" "new_servers_group" cfg_new_servers_groups_check

   if [ "$config_group_exist" -ne 0 ]; then
      config_group_exist=1
   else
      config_group_exist=0
   fi
fi

LOG_OUT "Start Getting【$CONFIG_NAME】Proxy-providers Setting..."

yml_provider_name_get()
{
   local section="$1"
   config_get "name" "$section" "name" ""
   config_get "config" "$section" "config" ""
   if [ -n "$name" ] && [ "$config" = "$CONFIG_NAME" ]; then
      echo "$provider_nums.$name" >>"$match_provider"
   fi
   provider_nums=$(( $provider_nums + 1 ))
}

cfg_new_provider_groups_get()
{
	 if [ -z "$1" ]; then
      return
   fi
   
   ${uci_add}groups="${1}"
}

echo "" >"$match_provider"
provider_nums=0
config_load "openclash"
config_foreach yml_provider_name_get "proxy-provider"

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
   if [ -n "$provider_nums" ]; then
      sed -i "/^${provider_nums}\./c\#match#" "$match_provider" 2>/dev/null
      uci_set="uci -q set openclash.@proxy-provider["$provider_nums"]."
      uci_get="uci -q get openclash.@proxy-provider["$provider_nums"]."
      uci_add="uci -q add_list openclash.@proxy-provider["$provider_nums"]."
      uci_del="uci -q delete openclash.@proxy-provider["$provider_nums"]."
      if [ -z "${uci_get}manual" ]; then
         ${uci_set}manual="0"
      fi
      ${uci_set}type="$provider_type"
   else
   #代理集不存在时添加新代理集
      name=openclash
      uci_name_tmp=$(uci add $name proxy-provider)
      uci_set="uci -q set $name.$uci_name_tmp."
      uci_add="uci -q add_list $name.$uci_name_tmp."
      uci_del="uci -q delete $name.$uci_name_tmp."
   
      if [ "$config_group_exist" -eq 0 ] && [ "$servers_if_update" = "1" ] && [ "$servers_update" -eq 1 ]; then
         ${uci_set}enabled="0"
      else
         ${uci_set}enabled="1"
      fi
      ${uci_set}manual="0"
      ${uci_set}config="$CONFIG_NAME"
      ${uci_set}name="$provider_name"
      ${uci_set}type="$provider_type"
   fi
   ruby -ryaml -rYAML -I "/usr/share/openclash" -E UTF-8 -e "
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
   #filter
   if Value['proxy-providers'].values[$provider_count].key?('filter') then
      provider_gen_filter = '${uci_set}provider_filter=' + Value['proxy-providers'].values[$provider_count]['filter'].to_s
      system(provider_gen_filter)
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
   puts '${LOGTIME} Error: Resolve Proxy-providers Failed,【${CONFIG_NAME} - ${provider_name}: ' + e.message + '】'
   end
   " 2>/dev/null >> $LOG_FILE &
      
   #加入策略组
   if [ "$servers_if_update" = "1" ] && [ "$config_group_exist" = "1" ] && [ "$servers_update" = "1" ] && [ -z "$provider_nums" ]; then
      #新代理集且设置默认策略组时加入指定策略组
      config_load "openclash"
      config_list_foreach "config" "new_servers_group" cfg_new_provider_groups_get
   elif [ "$servers_if_update" != "1" ]; then
      ruby -ryaml -rYAML -I "/usr/share/openclash" -E UTF-8 -e "
      Thread.new{
      begin
         Value = ${group_hash};
         system '${uci_del}groups >/dev/null 2>&1';
         Value['proxy-groups'].each{
         |x|
         if x.key?('use') then
            x['use'].each{
            |y|
            if y == '$provider_name' then
               uci = '${uci_add}groups=\"' + x['name'] + '\"'
               system(uci)
               break
            end
            }
         end
         };
      rescue Exception => e
      puts '${LOGTIME} Error: Resolve Proxy-providers Failed,【${CONFIG_NAME} - ${provider_name}: ' + e.message + '】'
      end
      }.join;
      " 2>/dev/null >> $LOG_FILE &
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
   config_get "config" "$section" "config" ""
   if [ -n "$name" ] && [ "$config" = "$CONFIG_NAME" ]; then
      echo "$server_num.$name" >>"$match_servers"
   fi
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

echo "" >"$match_servers"
server_num=0
config_load "openclash"
config_foreach yml_servers_name_get "servers"

while [ "$count" -lt "$num" ]
do
   #name
   server_name=$(ruby_read_hash "$proxy_hash" "['proxies'][$count]['name']")

   if [ -z "$server_name" ]; then
      let count++
      continue
   fi
   
   #type
   server_type=$(ruby_read_hash "$proxy_hash" "['proxies'][$count]['type']")

   LOG_OUT "Start Getting【$CONFIG_NAME - $server_type - $server_name】Proxy Setting..."
   
#节点存在时获取节点编号
   server_num=$(grep -Fw "$server_name" "$match_servers" 2>/dev/null|awk -F '.' '{print $1}')
   if [ -n "$server_num" ]; then
#更新已有节点
      sed -i "/^${server_num}\./c\#match#" "$match_servers" 2>/dev/null
      uci_set="uci -q set openclash.@servers["$server_num"]."
      uci_get="uci -q get openclash.@servers["$server_num"]."
      uci_add="uci -q add_list openclash.@servers["$server_num"]."
      uci_del="uci -q delete openclash.@servers["$server_num"]."

      if [ -z "${uci_get}manual" ]; then
         ${uci_set}manual="0"
      fi
      ${uci_set}type="$server_type"
   else
#添加新节点
      name=openclash
      uci_name_tmp=$(uci add $name servers)
      uci_set="uci -q set $name.$uci_name_tmp."
      uci_add="uci -q add_list $name.$uci_name_tmp."
      uci_del="uci -q delete $name.$uci_name_tmp."

      if [ "$config_group_exist" -eq 0 ] && [ "$servers_if_update" = "1" ] && [ "$servers_update" -eq 1 ]; then
         ${uci_set}enabled="0"
      else
         ${uci_set}enabled="1"
      fi
      ${uci_set}manual="0"
      ${uci_set}config="$CONFIG_NAME"
      ${uci_set}name="$server_name"
      ${uci_set}type="$server_type"
   fi

   ruby -ryaml -rYAML -I "/usr/share/openclash" -E UTF-8 -e "
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
   
   Thread.new{
   #interface-name
   if Value['proxies'][$count].key?('interface-name') then
      interface_name = '${uci_set}interface_name=' + Value['proxies'][$count]['interface-name'].to_s
      system(interface_name)
   end
   }.join;
   
   Thread.new{
   #routing-mark
   if Value['proxies'][$count].key?('routing-mark') then
      routing_mark = '${uci_set}routing_mark=' + Value['proxies'][$count]['routing-mark'].to_s
      system(routing_mark)
   end
   }.join;

   Thread.new{
      #ip_version
      if Value['proxies'][$count].key?('ip-version') then
         ip_version = '${uci_set}ip_version=' + Value['proxies'][$count]['ip-version'].to_s
         system(ip_version)
      end
   }.join
   
   Thread.new{
      #TFO
      if Value['proxies'][$count].key?('tfo') then
         tfo = '${uci_set}tfo=' + Value['proxies'][$count]['tfo'].to_s
         system(tfo)
      end
   }.join

   Thread.new{
      #Multiplex
      if Value['proxies'][$count].key?('smux') then
         if Value['proxies'][$count]['smux'].key?('enabled') then
            smux = '${uci_set}multiplex=' + Value['proxies'][$count]['smux']['enabled'].to_s
            system(smux)
         end;
         #multiplex_protocol
         if Value['proxies'][$count]['smux'].key?('protocol') then
            multiplex_protocol = '${uci_set}multiplex_protocol=' + Value['proxies'][$count]['smux']['protocol'].to_s
            system(multiplex_protocol)
         end;
         #multiplex_max_connections
         if Value['proxies'][$count]['smux'].key?('max-connections') then
            multiplex_max_connections = '${uci_set}multiplex_max_connections=' + Value['proxies'][$count]['smux']['max-connections'].to_s
            system(multiplex_max_connections)
         end;
         #multiplex_min_streams
         if Value['proxies'][$count]['smux'].key?('min-streams') then
            multiplex_min_streams = '${uci_set}multiplex_min_streams=' + Value['proxies'][$count]['smux']['min-streams'].to_s
            system(multiplex_min_streams)
         end;
         #multiplex_max_streams
         if Value['proxies'][$count]['smux'].key?('max-streams') then
            multiplex_max_streams = '${uci_set}multiplex_max_streams=' + Value['proxies'][$count]['smux']['max-streams'].to_s
            system(multiplex_max_streams)
         end;
         #multiplex_padding
         if Value['proxies'][$count]['smux'].key?('padding') then
            multiplex_padding = '${uci_set}multiplex_padding=' + Value['proxies'][$count]['smux']['padding'].to_s
            system(multiplex_padding)
         end;
         #multiplex_statistic
         if Value['proxies'][$count]['smux'].key?('statistic') then
            multiplex_statistic = '${uci_set}multiplex_statistic=' + Value['proxies'][$count]['smux']['statistic'].to_s
            system(multiplex_statistic)
         end;
         #multiplex_only_tcp
         if Value['proxies'][$count]['smux'].key?('only-tcp') then
            multiplex_only_tcp = '${uci_set}multiplex_only_tcp=' + Value['proxies'][$count]['smux']['only-tcp'].to_s
            system(multiplex_only_tcp)
         end;
      end;
   }.join

   if '$server_type' == 'ss' then
      Thread.new{
      #cipher
      if Value['proxies'][$count].key?('cipher') then
         cipher = '${uci_set}cipher=' + Value['proxies'][$count]['cipher'].to_s
         system(cipher)
      end
      }.join

      Thread.new{
      #udp-over-tcp
      if Value['proxies'][$count].key?('udp-over-tcp') then
         udp_over_tcp = '${uci_set}udp_over_tcp=' + Value['proxies'][$count]['udp-over-tcp'].to_s
         system(udp_over_tcp)
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
         #fingerprint
         if Value['proxies'][$count]['plugin-opts'].key?('fingerprint') then
            fingerprint = '${uci_set}fingerprint=' + Value['proxies'][$count]['plugin-opts']['fingerprint'].to_s
            system(fingerprint)
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
         end;
         if Value['proxies'][$count]['plugin'].to_s == 'shadow-tls' then
            mode = '${uci_set}obfs=' + Value['proxies'][$count]['plugin'].to_s
            system(mode)
            #password
            if Value['proxies'][$count]['plugin-opts'].key?('password') then
               obfs_password = '${uci_set}obfs_password=\"' + Value['proxies'][$count]['plugin-opts']['password'].to_s + '\"'
               system(obfs_password)
            end
         end;
         if Value['proxies'][$count]['plugin'].to_s == 'restls' then
            mode = '${uci_set}obfs=' + Value['proxies'][$count]['plugin'].to_s
            system(mode)
            #password
            if Value['proxies'][$count]['plugin-opts'].key?('password') then
               obfs_password = '${uci_set}obfs_password=\"' + Value['proxies'][$count]['plugin-opts']['password'].to_s + '\"'
               system(obfs_password)
            end
            #version-hint
            if Value['proxies'][$count]['plugin-opts'].key?('version-hint') then
               obfs_version_hint = '${uci_set}obfs_version_hint=\"' + Value['proxies'][$count]['plugin-opts']['version-hint'].to_s + '\"'
               system(obfs_version_hint)
            end
            #restls-script
            if Value['proxies'][$count]['plugin-opts'].key?('restls-script') then
               obfs_restls_script = '${uci_set}obfs_restls_script=\"' + Value['proxies'][$count]['plugin-opts']['restls-script'].to_s + '\"'
               system(obfs_restls_script)
            end
         end;
      end
      }.join
   end;
   if '$server_type' == 'ssr' then
      Thread.new{
      #cipher
      if Value['proxies'][$count].key?('cipher') then
         if Value['proxies'][$count]['cipher'].to_s == 'none' then
            cipher = '${uci_set}cipher_ssr=dummy'
         else
            cipher = '${uci_set}cipher_ssr=' + Value['proxies'][$count]['cipher'].to_s
         end
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
      #xudp
      if Value['proxies'][$count].key?('xudp') then
         xudp = '${uci_set}xudp=' + Value['proxies'][$count]['xudp'].to_s
         system(xudp)
      end
      }.join;

      Thread.new{
      #packet_encoding
      if Value['proxies'][$count].key?('packet-encoding') then
         packet_encoding = '${uci_set}packet_encoding=' + Value['proxies'][$count]['packet-encoding'].to_s
         system(packet_encoding)
      end
      }.join;

      Thread.new{
      #GlobalPadding
      if Value['proxies'][$count].key?('global-padding') then
         global_padding = '${uci_set}global_padding=' + Value['proxies'][$count]['global-padding'].to_s
         system(global_padding)
      end
      }.join;

      Thread.new{
      #authenticated_length
      if Value['proxies'][$count].key?('authenticated-length') then
         authenticated_length = '${uci_set}authenticated_length=' + Value['proxies'][$count]['authenticated-length'].to_s
         system(authenticated_length)
      end
      }.join;
      
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
      #fingerprint
      if Value['proxies'][$count].key?('fingerprint') then
         fingerprint = '${uci_set}fingerprint=' + Value['proxies'][$count]['fingerprint'].to_s
         system(fingerprint)
      end
      }.join

      Thread.new{
      #client_fingerprint
      if Value['proxies'][$count].key?('client-fingerprint') then
         client_fingerprint = '${uci_set}client_fingerprint=' + Value['proxies'][$count]['client-fingerprint'].to_s
         system(client_fingerprint)
      end
      }.join
      
      Thread.new{
      #network:
      if Value['proxies'][$count].key?('network') then
         if Value['proxies'][$count]['network'].to_s == 'ws'
            system '${uci_set}obfs_vmess=websocket'
            #ws-path:
            if Value['proxies'][$count].key?('ws-path') then
               path = '${uci_set}ws_opts_path=\"' + Value['proxies'][$count]['ws-path'].to_s + '\"'
               system(path)
            end
            #Host:
            if Value['proxies'][$count].key?('ws-headers') then
               system '${uci_del}ws_opts_headers >/dev/null 2>&1'
               Value['proxies'][$count]['ws-headers'].keys.each{
               |v|
                  custom = '${uci_add}ws_opts_headers=\"' + v.to_s + ': '+ Value['proxies'][$count]['ws-headers'][v].to_s + '\"'
                  system(custom)
               }
            end
            #ws-opts-path:
            if Value['proxies'][$count].key?('ws-opts') then
               if Value['proxies'][$count]['ws-opts'].key?('path') then
                  ws_opts_path = '${uci_set}ws_opts_path=\"' + Value['proxies'][$count]['ws-opts']['path'].to_s + '\"'
                  system(ws_opts_path)
               end
               #ws-opts-headers:
               if Value['proxies'][$count]['ws-opts'].key?('headers') then
                  system '${uci_del}ws_opts_headers >/dev/null 2>&1'
                  Value['proxies'][$count]['ws-opts']['headers'].keys.each{
                  |v|
                     ws_opts_headers = '${uci_add}ws_opts_headers=\"' + v.to_s + ': '+ Value['proxies'][$count]['ws-opts']['headers'][v].to_s + '\"'
                     system(ws_opts_headers)
                  }
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

   #Tuic
   if '$server_type' == 'tuic' then
      Thread.new{
      #tc_ip
      if Value['proxies'][$count].key?('ip') then
         tc_ip = '${uci_set}tc_ip=' + Value['proxies'][$count]['ip'].to_s
         system(tc_ip)
      end
      }.join

      Thread.new{
      #tc_token
      if Value['proxies'][$count].key?('token') then
         tc_token = '${uci_set}tc_token=' + Value['proxies'][$count]['token'].to_s
         system(tc_token)
      end
      }.join

      Thread.new{
      #heartbeat_interval
      if Value['proxies'][$count].key?('heartbeat-interval') then
         heartbeat_interval = '${uci_set}heartbeat_interval=' + Value['proxies'][$count]['heartbeat-interval'].to_s
         system(heartbeat_interval)
      end
      }.join

      Thread.new{
      #tc_alpn
      if Value['proxies'][$count].key?('alpn') then
         system '${uci_del}tc_alpn >/dev/null 2>&1'
         Value['proxies'][$count]['alpn'].each{
         |x|
            tc_alpn = '${uci_add}tc_alpn=\"' + x.to_s + '\"'
            system(tc_alpn)
         }
      end;
      }.join

      Thread.new{
      #disable_sni
      if Value['proxies'][$count].key?('disable-sni') then
         disable_sni = '${uci_set}disable_sni=' + Value['proxies'][$count]['disable-sni'].to_s
         system(disable_sni)
      end
      }.join

      Thread.new{
      #reduce_rtt
      if Value['proxies'][$count].key?('reduce-rtt') then
         reduce_rtt = '${uci_set}reduce_rtt=' + Value['proxies'][$count]['reduce-rtt'].to_s
         system(reduce_rtt)
      end
      }.join

      Thread.new{
      #fast_open
      if Value['proxies'][$count].key?('fast-open') then
         fast_open = '${uci_set}fast_open=' + Value['proxies'][$count]['fast-open'].to_s
         system(fast_open)
      end
      }.join

      Thread.new{
      #request_timeout
      if Value['proxies'][$count].key?('request-timeout') then
         request_timeout = '${uci_set}request_timeout=' + Value['proxies'][$count]['request-timeout'].to_s
         system(request_timeout)
      end
      }.join

      Thread.new{
      #udp_relay_mode
      if Value['proxies'][$count].key?('udp-relay-mode') then
         udp_relay_mode = '${uci_set}udp_relay_mode=' + Value['proxies'][$count]['udp-relay-mode'].to_s
         system(udp_relay_mode)
      end
      }.join

      Thread.new{
      #congestion_controller
      if Value['proxies'][$count].key?('congestion-controller') then
         congestion_controller = '${uci_set}congestion_controller=' + Value['proxies'][$count]['congestion-controller'].to_s
         system(congestion_controller)
      end
      }.join

      Thread.new{
      #max_udp_relay_packet_size
      if Value['proxies'][$count].key?('max-udp-relay-packet-size') then
         max_udp_relay_packet_size = '${uci_set}max_udp_relay_packet_size=' + Value['proxies'][$count]['max-udp-relay-packet-size'].to_s
         system(max_udp_relay_packet_size)
      end
      }.join

      Thread.new{
      #max-open-streams
      if Value['proxies'][$count].key?('max-open-streams') then
         max_open_streams = '${uci_set}max_open_streams=' + Value['proxies'][$count]['max-open-streams'].to_s
         system(max_open_streams)
      end
      }.join
   end;

   #WireGuard
   if '$server_type' == 'wireguard' then
      Thread.new{
      #wg_ip
      if Value['proxies'][$count].key?('ip') then
         wg_ip = '${uci_set}wg_ip=' + Value['proxies'][$count]['ip'].to_s
         system(wg_ip)
      end
      }.join

      Thread.new{
      #wg_ipv6
      if Value['proxies'][$count].key?('ipv6') then
         wg_ipv6 = '${uci_set}wg_ipv6=' + Value['proxies'][$count]['ipv6'].to_s
         system(wg_ipv6)
      end
      }.join

      Thread.new{
      #private_key
      if Value['proxies'][$count].key?('private-key') then
         private_key = '${uci_set}private_key=' + Value['proxies'][$count]['private-key'].to_s
         system(private_key)
      end
      }.join

      Thread.new{
      #public_key
      if Value['proxies'][$count].key?('public-key') then
         public_key = '${uci_set}public_key=' + Value['proxies'][$count]['public-key'].to_s
         system(public_key)
      end
      }.join

      Thread.new{
      #preshared_key
      if Value['proxies'][$count].key?('preshared-key') then
         preshared_key = '${uci_set}preshared_key=' + Value['proxies'][$count]['preshared-key'].to_s
         system(preshared_key)
      end
      }.join

      Thread.new{
      #wg_mtu
      if Value['proxies'][$count].key?('mtu') then
         wg_mtu = '${uci_set}wg_mtu=' + Value['proxies'][$count]['mtu'].to_s
         system(wg_mtu)
      end
      }.join

      Thread.new{
      #wg_dns
      if Value['proxies'][$count].key?('dns') then
         system '${uci_del}wg_dns >/dev/null 2>&1'
         Value['proxies'][$count]['dns'].each{
         |x|
            wg_dns = '${uci_add}wg_dns=\"' + x.to_s + '\"'
            system(wg_dns)
         }
      end;
      }.join
   end;

   if '$server_type' == 'hysteria' then
      Thread.new{
      #hysteria_protocol
      if Value['proxies'][$count].key?('protocol') then
         hysteria_protocol = '${uci_set}hysteria_protocol=' + Value['proxies'][$count]['protocol'].to_s
         system(hysteria_protocol)
      end
      }.join

      Thread.new{
      #hysteria_up
      if Value['proxies'][$count].key?('up') then
         hysteria_up = '${uci_set}hysteria_up=' + Value['proxies'][$count]['up'].to_s
         system(hysteria_up)
      end
      }.join

      Thread.new{
      #hysteria_down
      if Value['proxies'][$count].key?('down') then
         hysteria_down = '${uci_set}hysteria_down=' + Value['proxies'][$count]['down'].to_s
         system(hysteria_down)
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
      #sni
      if Value['proxies'][$count].key?('sni') then
         sni = '${uci_set}sni=\"' + Value['proxies'][$count]['sni'].to_s + '\"'
         system(sni)
      end
      }.join

      Thread.new{
      #alpn
      if Value['proxies'][$count].key?('alpn') then
         system '${uci_del}hysteria_alpn >/dev/null 2>&1'
         if Value['proxies'][$count]['alpn'].class.to_s != 'Array' then
            alpn = '${uci_add}hysteria_alpn=\"' + Value['proxies'][$count]['alpn'].to_s + '\"'
            system(alpn)
         else
            Value['proxies'][$count]['alpn'].each{
            |x|
               alpn = '${uci_add}hysteria_alpn=\"' + x.to_s + '\"'
               system(alpn)
            }
         end
      end;
      }.join

      Thread.new{
      #recv_window_conn
      if Value['proxies'][$count].key?('recv-window-conn') then
         recv_window_conn = '${uci_set}recv_window_conn=' + Value['proxies'][$count]['recv-window-conn'].to_s
         system(recv_window_conn)
      end
      }.join

      Thread.new{
      #recv_window
      if Value['proxies'][$count].key?('recv-window') then
         recv_window = '${uci_set}recv_window=' + Value['proxies'][$count]['recv-window'].to_s
         system(recv_window)
      end
      }.join

      Thread.new{
      #hysteria_obfs
      if Value['proxies'][$count].key?('obfs') then
         hysteria_obfs = '${uci_set}hysteria_obfs=' + Value['proxies'][$count]['obfs'].to_s
         system(hysteria_obfs)
      end
      }.join

      Thread.new{
      #hysteria_auth
      if Value['proxies'][$count].key?('auth') then
         hysteria_auth = '${uci_set}hysteria_auth=' + Value['proxies'][$count]['auth'].to_s
         system(hysteria_auth)
      end
      }.join

      Thread.new{
      #hysteria_auth_str
      if Value['proxies'][$count].key?('auth-str') then
         hysteria_auth_str = '${uci_set}hysteria_auth_str=' + Value['proxies'][$count]['auth-str'].to_s
         system(hysteria_auth_str)
      end
      }.join

      Thread.new{
      #hysteria_ca
      if Value['proxies'][$count].key?('ca') then
         hysteria_ca = '${uci_set}hysteria_ca=' + Value['proxies'][$count]['ca'].to_s
         system(hysteria_ca)
      end
      }.join

      Thread.new{
      #hysteria_ca_str
      if Value['proxies'][$count].key?('ca-str') then
         hysteria_ca_str = '${uci_set}hysteria_ca_str=' + Value['proxies'][$count]['ca-str'].to_s
         system(hysteria_ca_str)
      end
      }.join

      Thread.new{
      #disable_mtu_discovery
      if Value['proxies'][$count].key?('disable-mtu-discovery') then
         disable_mtu_discovery = '${uci_set}disable_mtu_discovery=' + Value['proxies'][$count]['disable-mtu-discovery'].to_s
         system(disable_mtu_discovery)
      end
      }.join

      Thread.new{
      #fast_open
      if Value['proxies'][$count].key?('fast-open') then
         fast_open = '${uci_set}fast_open=' + Value['proxies'][$count]['fast-open'].to_s
         system(fast_open)
      end
      }.join

      Thread.new{
      #fingerprint
      if Value['proxies'][$count].key?('fingerprint') then
         fingerprint = '${uci_set}fingerprint=' + Value['proxies'][$count]['fingerprint'].to_s
         system(fingerprint)
      end
      }.join

      Thread.new{
      #ports
      if Value['proxies'][$count].key?('ports') then
         ports = '${uci_set}ports=' + Value['proxies'][$count]['ports'].to_s
         system(ports)
      end
      }.join

      Thread.new{
      #hop-interval
      if Value['proxies'][$count].key?('hop-interval') then
         hop_interval = '${uci_set}hop_interval=' + Value['proxies'][$count]['hop-interval'].to_s
         system(hop_interval)
      end
      }.join
   end;

   if '$server_type' == 'vless' then
      Thread.new{
      #uuid
      if Value['proxies'][$count].key?('uuid') then
         uuid = '${uci_set}uuid=' + Value['proxies'][$count]['uuid'].to_s
         system(uuid)
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
      #flow
      if Value['proxies'][$count].key?('flow') then
         flow = '${uci_set}vless_flow=\"' + Value['proxies'][$count]['flow'].to_s + '\"'
         system(flow)
      end
      }.join
      
      Thread.new{
      #network:
      if Value['proxies'][$count].key?('network') then
         if Value['proxies'][$count]['network'].to_s == 'ws'
            system '${uci_set}obfs_vless=ws'
            #ws-opts-path:
            if Value['proxies'][$count].key?('ws-opts') then
               if Value['proxies'][$count]['ws-opts'].key?('path') then
                  ws_opts_path = '${uci_set}ws_opts_path=\"' + Value['proxies'][$count]['ws-opts']['path'].to_s + '\"'
                  system(ws_opts_path)
               end
               #ws-opts-headers:
               if Value['proxies'][$count]['ws-opts'].key?('headers') then
                  system '${uci_del}ws_opts_headers >/dev/null 2>&1'
                  Value['proxies'][$count]['ws-opts']['headers'].keys.each{
                  |v|
                     ws_opts_headers = '${uci_add}ws_opts_headers=\"' + v.to_s + ': '+ Value['proxies'][$count]['ws-opts']['headers'][v].to_s + '\"'
                     system(ws_opts_headers)
                  }
               end
            end
         elsif Value['proxies'][$count]['network'].to_s == 'grpc'
            #grpc-service-name
            system '${uci_set}obfs_vless=grpc'
            if Value['proxies'][$count].key?('grpc-opts') then
               if Value['proxies'][$count]['grpc-opts'].key?('grpc-service-name') then
                  grpc_service_name = '${uci_set}grpc_service_name=\"' + Value['proxies'][$count]['grpc-opts']['grpc-service-name'].to_s + '\"'
                  system(grpc_service_name)
               end
            end
            if Value['proxies'][$count].key?('reality-opts') then
               if Value['proxies'][$count]['reality-opts'].key?('public-key') then
                  reality_public_key = '${uci_set}reality_public_key=\"' + Value['proxies'][$count]['reality-opts']['public-key'].to_s + '\"'
                  system(reality_public_key)
               end
               if Value['proxies'][$count]['reality-opts'].key?('short-id') then
                  reality_short_id = '${uci_set}reality_short_id=\"' + Value['proxies'][$count]['reality-opts']['short-id'].to_s + '\"'
                  system(reality_short_id)
               end
            end
         elsif Value['proxies'][$count]['network'].to_s == 'tcp'
            system '${uci_set}obfs_vless=tcp'
            if Value['proxies'][$count].key?('reality-opts') then
               if Value['proxies'][$count]['reality-opts'].key?('public-key') then
                  reality_public_key = '${uci_set}reality_public_key=\"' + Value['proxies'][$count]['reality-opts']['public-key'].to_s + '\"'
                  system(reality_public_key)
               end
               if Value['proxies'][$count]['reality-opts'].key?('short-id') then
                  reality_short_id = '${uci_set}reality_short_id=\"' + Value['proxies'][$count]['reality-opts']['short-id'].to_s + '\"'
                  system(reality_short_id)
               end
            end
         end
      end
      }.join

      Thread.new{
      #xudp
      if Value['proxies'][$count].key?('xudp') then
         xudp = '${uci_set}xudp=' + Value['proxies'][$count]['xudp'].to_s
         system(xudp)
      end
      }.join;

      Thread.new{
      #packet-addr
      if Value['proxies'][$count].key?('packet-addr') then
         packet_addr = '${uci_set}packet_addr=' + Value['proxies'][$count]['packet-addr'].to_s
         system(packet_addr)
      end
      }.join;

      Thread.new{
      #packet_encoding
      if Value['proxies'][$count].key?('packet-encoding') then
         packet_encoding = '${uci_set}packet_encoding=' + Value['proxies'][$count]['packet-encoding'].to_s
         system(packet_encoding)
      end
      }.join;

      Thread.new{
      #fingerprint
      if Value['proxies'][$count].key?('fingerprint') then
         fingerprint = '${uci_set}fingerprint=' + Value['proxies'][$count]['fingerprint'].to_s
         system(fingerprint)
      end
      }.join

      Thread.new{
      #client_fingerprint
      if Value['proxies'][$count].key?('client-fingerprint') then
         client_fingerprint = '${uci_set}client_fingerprint=' + Value['proxies'][$count]['client-fingerprint'].to_s
         system(client_fingerprint)
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
      
      Thread.new{
      if Value['proxies'][$count].key?('version') then
         snell_version = '${uci_set}snell_version=' + Value['proxies'][$count]['version'].to_s
         system(snell_version)
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

      Thread.new{
      #http-headers:
      if Value['proxies'][$count].key?('headers') then
         system '${uci_del}http_headers >/dev/null 2>&1'
         Value['proxies'][$count]['headers'].keys.each{
         |v|
            http_headers = '${uci_add}http_headers=\"' + v.to_s + ': '+ Value['proxies'][$count]['headers'][v].to_s + '\"'
            system(http_headers)
         }
      end
      }.join

      Thread.new{
      #fingerprint
      if Value['proxies'][$count].key?('fingerprint') then
         fingerprint = '${uci_set}fingerprint=' + Value['proxies'][$count]['fingerprint'].to_s
         system(fingerprint)
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
         system '${uci_set}obfs_trojan=grpc'
         if Value['proxies'][$count]['grpc-opts'].key?('grpc-service-name') then
            grpc_service_name = '${uci_set}grpc_service_name=\"' + Value['proxies'][$count]['grpc-opts']['grpc-service-name'].to_s + '\"'
            system(grpc_service_name)
         end
      end
      }.join
      
      Thread.new{
      if Value['proxies'][$count].key?('ws-opts') then
         system '${uci_set}obfs_trojan=ws'
         #trojan_ws_path
         if Value['proxies'][$count]['ws-opts'].key?('path') then
            trojan_ws_path = '${uci_set}trojan_ws_path=\"' + Value['proxies'][$count]['ws-opts']['path'].to_s + '\"'
            system(trojan_ws_path)
         end
         #trojan_ws_headers
         if Value['proxies'][$count]['ws-opts'].key?('headers') then
            system '${uci_del}trojan_ws_headers >/dev/null 2>&1'
            Value['proxies'][$count]['ws-opts']['headers'].keys.each{
            |v|
               trojan_ws_headers = '${uci_add}trojan_ws_headers=\"' + v.to_s + ': '+ Value['proxies'][$count]['ws-opts']['headers'][v].to_s + '\"'
               system(trojan_ws_headers)
            }
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

      Thread.new{
      #fingerprint
      if Value['proxies'][$count].key?('fingerprint') then
         fingerprint = '${uci_set}fingerprint=' + Value['proxies'][$count]['fingerprint'].to_s
         system(fingerprint)
      end
      }.join

      Thread.new{
      #client_fingerprint
      if Value['proxies'][$count].key?('client-fingerprint') then
         client_fingerprint = '${uci_set}client_fingerprint=' + Value['proxies'][$count]['client-fingerprint'].to_s
         system(client_fingerprint)
      end
      }.join
   end;
   
   rescue Exception => e
   puts '${LOGTIME} Error: Resolve Proxies Failed,【${CONFIG_NAME} - ${server_type} - ${server_name}: ' + e.message + '】'
   end
   " 2>/dev/null >> $LOG_FILE &
   

   #加入策略组
   if [ "$servers_if_update" = "1" ] && [ "$config_group_exist" = "1" ] && [ "$servers_update" = "1" ] && [ -z "$server_num" ]; then
      #新代理且设置默认策略组时加入指定策略组
      config_load "openclash"
      config_list_foreach "config" "new_servers_group" cfg_new_servers_groups_get
   elif [ "$servers_if_update" != "1" ]; then
      ruby -ryaml -rYAML -I "/usr/share/openclash" -E UTF-8 -e "
      Thread.new{
      begin
         Value = ${group_hash};
         #proxy
         system '${uci_del}groups >/dev/null 2>&1';
         Value['proxy-groups'].each{
         |x|
         if x.key?('proxies') then
            x['proxies'].each{
            |y|
            if y == '$server_name' then
               uci_proxy = '${uci_add}groups=\"' + x['name'] + '\"'
               system(uci_proxy)
               break
            end
            }
         end
         };;
         #relay
         system '${uci_del}relay_groups >/dev/null 2>&1';
         Value['proxy-groups'].each{
         |x|
         if x['type'] == 'relay' and x.key?('proxies') then
            x['proxies'].each{
            |y|
            if y == '$server_name' then
               uci_relay = '${uci_add}relay_groups=\"' + x['name'] + '#relay#' + x['proxies'].index('$server_name') + '\"'
               system(uci_relay)
               break
            end
            }
         end
         };
      rescue Exception => e
      puts '${LOGTIME} Error: Resolve Proxies Failed,【${CONFIG_NAME} - ${server_type} - ${server_name}: ' + e.message + '】'
      end
      }.join;
      " 2>/dev/null >> $LOG_FILE &
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
        if [ "$(uci -q get openclash.@servers["$line"].manual)" = "0" ] && [ "$(uci -q get openclash.@servers["$line"].config)" = "$CONFIG_NAME" ]; then
           uci -q delete openclash.@servers["$line"]
        fi
     done 2>/dev/null
fi

uci -q set openclash.config.servers_if_update=0
wait
uci -q commit openclash
LOG_OUT "Config File【$CONFIG_NAME】Read Successful!"
sleep 3
SLOG_CLEAN
rm -rf /tmp/match_servers.list 2>/dev/null
rm -rf /tmp/match_provider.list 2>/dev/null
rm -rf /tmp/yaml_other_group.yaml 2>/dev/null
del_lock