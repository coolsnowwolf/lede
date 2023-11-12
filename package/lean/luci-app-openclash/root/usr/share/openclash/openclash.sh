#!/bin/bash
. /lib/functions.sh
. /usr/share/openclash/ruby.sh
. /usr/share/openclash/openclash_ps.sh
. /usr/share/openclash/log.sh

set_lock() {
   exec 889>"/tmp/lock/openclash_subs.lock" 2>/dev/null
   flock -x 889 2>/dev/null
}

del_lock() {
   flock -u 889 2>/dev/null
   rm -rf "/tmp/lock/openclash_subs.lock"
}

LOGTIME=$(echo $(date "+%Y-%m-%d %H:%M:%S"))
LOG_FILE="/tmp/openclash.log"
CFG_FILE="/tmp/yaml_sub_tmp_config.yaml"
CRON_FILE="/etc/crontabs/root"
CONFIG_PATH=$(uci -q get openclash.config.config_path)
servers_update=$(uci -q get openclash.config.servers_update)
dns_port=$(uci -q get openclash.config.dns_port)
enable_redirect_dns=$(uci -q get openclash.config.enable_redirect_dns)
disable_masq_cache=$(uci -q get openclash.config.disable_masq_cache)
default_resolvfile=$(uci -q get openclash.config.default_resolvfile)
en_mode=$(uci -q get openclash.config.en_mode)
china_ip_route=$(uci -q get openclash.config.china_ip_route)
disable_udp_quic=$(uci -q get openclash.config.disable_udp_quic)
ipv6_enable=$(uci -q get openclash.config.ipv6_enable)
router_self_proxy=$(uci -q get openclash.config.router_self_proxy || echo 1)
DNSPORT=$(uci -q get dhcp.@dnsmasq[0].port)
DNSMASQ_CONF_DIR=$(uci -q get dhcp.@dnsmasq[0].confdir || echo '/tmp/dnsmasq.d')
DNSMASQ_CONF_DIR=${DNSMASQ_CONF_DIR%*/}
custom_china_domain_dns_server=$(uci -q get openclash.config.custom_china_domain_dns_server || echo "114.114.114.114")
FW4=$(command -v fw4)
CLASH="/etc/openclash/clash"
CLASH_CONFIG="/tmp"


if [ -z "$DNSPORT" ]; then
   DNSPORT=$(netstat -nlp |grep -E '127.0.0.1:.*dnsmasq' |awk -F '127.0.0.1:' '{print $2}' |awk '{print $1}' |head -1 || echo 53)
fi
restart=0
only_download=0
set_lock

urlencode() {
   if [ "$#" -eq 1 ]; then
      echo "$(/usr/share/openclash/openclash_urlencode.lua "$1")"
   fi
}

kill_watchdog() {
   watchdog_pids=$(unify_ps_pids "openclash_watchdog.sh")
   for watchdog_pid in $watchdog_pids; do
      kill -9 "$watchdog_pid" >/dev/null 2>&1
   done
   
   streaming_unlock_pids=$(unify_ps_pids "openclash_streaming_unlock.lua")
   for streaming_unlock_pid in $streaming_unlock_pids; do
      kill -9 "$streaming_unlock_pid" >/dev/null 2>&1
   done >/dev/null 2>&1
}

config_test()
{
   if [ -f "$CLASH" ]; then
      LOG_OUT "Config File Download Successful, Test If There is Any Errors..."
      test_info=$(nohup $CLASH -t -d $CLASH_CONFIG -f "$CFG_FILE")
      local IFS=$'\n'
      for i in $test_info; do
         if [ -n "$(echo "$i" |grep "configuration file")" ]; then
            local info=$(echo "$i" |sed "s# ${CFG_FILE} #【${CONFIG_FILE}】#g")
            LOG_OUT "$info"
         else
            echo "$i" >> "$LOG_FILE"
         fi
      done
      if [ -n "$(echo "$test_info" |grep "test failed")" ]; then
         return 1
      fi
   else
      return 0
   fi
}

config_download()
{
if [ -n "$subscribe_url_param" ]; then
   if [ -n "$c_address" ]; then
      curl -SsL --connect-timeout 30 -m 60 --speed-time 30 --speed-limit 1 --retry 2 -H 'User-Agent: Clash' "$c_address""$subscribe_url_param" -o "$CFG_FILE" 2>&1 | awk -v time="$(date "+%Y-%m-%d %H:%M:%S")" -v file="$CFG_FILE" '{print time "【" file "】Download Failed:【"$0"】"}' >> "$LOG_FILE"
   else
      curl -SsL --connect-timeout 30 -m 60 --speed-time 30 --speed-limit 1 --retry 2 -H 'User-Agent: Clash' https://api.dler.io/sub"$subscribe_url_param" -o "$CFG_FILE" 2>&1 | awk -v time="$(date "+%Y-%m-%d %H:%M:%S")" -v file="$CFG_FILE" '{print time "【" file "】Download Failed:【"$0"】"}' >> "$LOG_FILE"
      if [ "$?" -ne 0 ]; then
         curl -SsL --connect-timeout 30 -m 60 --speed-time 30 --speed-limit 1 --retry 2 -H 'User-Agent: Clash' https://subconverter.herokuapp.com/sub"$subscribe_url_param" -o "$CFG_FILE" 2>&1 | awk -v time="$(date "+%Y-%m-%d %H:%M:%S")" -v file="$CFG_FILE" '{print time "【" file "】Download Failed:【"$0"】"}' >> "$LOG_FILE"
      fi
   fi
else
   curl -SsL --connect-timeout 30 -m 60 --speed-time 30 --speed-limit 1 --retry 2 -H 'User-Agent: Clash' "$subscribe_url" -o "$CFG_FILE" 2>&1 | awk -v time="$(date "+%Y-%m-%d %H:%M:%S")" -v file="$CFG_FILE" '{print time "【" file "】Download Failed:【"$0"】"}' >> "$LOG_FILE"
fi
}

config_cus_up()
{
	if [ -z "$CONFIG_PATH" ]; then
      for file_name in /etc/openclash/config/*
      do
         if [ -f "$file_name" ]; then
            CONFIG_PATH=$file_name
            break
         fi
      done
      uci -q set openclash.config.config_path="$CONFIG_PATH"
      uci commit openclash
	fi
	if [ -z "$subscribe_url_param" ]; then
	   if [ -n "$key_match_param" ] || [ -n "$key_ex_match_param" ]; then
	      LOG_OUT "Config File【$name】is Replaced Successfully, Start Picking Nodes..."	      
	      ruby -ryaml -rYAML -I "/usr/share/openclash" -E UTF-8 -e "
	      begin
	         Value = YAML.load_file('$CONFIG_FILE');
	         if Value.has_key?('proxies') and not Value['proxies'].to_a.empty? then
	            Value['proxies'].reverse.each{
	            |x|
	            if not '$key_match_param'.empty? then
	               if not /$key_match_param/i =~ x['name'] then
	                  Value['proxies'].delete(x)
	                  Value['proxy-groups'].each{
	                     |g|
	                     g['proxies'].reverse.each{
	                        |p|
	                        if p == x['name'] then
	                           g['proxies'].delete(p)
	                        end
	                     }
	                  }
	               end
	            end;
	            if not '$key_ex_match_param'.empty? then
	               if /$key_ex_match_param/i =~ x['name'] then
	                  if Value['proxies'].include?(x) then
	                     Value['proxies'].delete(x)
	                     Value['proxy-groups'].each{
	                        |g|
	                        g['proxies'].reverse.each{
	                           |p|
	                           if p == x['name'] then
	                              g['proxies'].delete(p)
	                           end
	                        }
	                     }
	                  end
	               end
	            end;
	            }
	         end;
	      rescue Exception => e
	         puts '${LOGTIME} Error: Filter Proxies Failed,【' + e.message + '】'
	      ensure
	         File.open('$CONFIG_FILE','w') {|f| YAML.dump(Value, f)};
	      end" 2>/dev/null >> $LOG_FILE
	   fi
   fi
   if [ "$servers_update" -eq 1 ]; then
      LOG_OUT "Config File【$name】is Replaced Successfully, Start to Reserving..."
      uci -q set openclash.config.config_update_path="/etc/openclash/config/$name.yaml"
      uci -q set openclash.config.servers_if_update=1
      uci commit openclash
      /usr/share/openclash/yml_groups_get.sh
      uci -q set openclash.config.servers_if_update=1
      uci commit openclash
      /usr/share/openclash/yml_groups_set.sh
      if [ "$CONFIG_FILE" == "$CONFIG_PATH" ]; then
         restart=1
      fi
      LOG_OUT "Config File【$name】Update Successful!"
      SLOG_CLEAN
   elif [ "$CONFIG_FILE" == "$CONFIG_PATH" ]; then
      LOG_OUT "Config File【$name】Update Successful!"
      restart=1
   else
      LOG_OUT "Config File【$name】Update Successful!"
      SLOG_CLEAN
   fi
   
   rm -rf /tmp/Proxy_Group 2>/dev/null
}

config_su_check()
{
   LOG_OUT "Config File Test Successful, Check If There is Any Update..."
   sed -i 's/!<str> /!!str /g' "$CFG_FILE" >/dev/null 2>&1
   if [ -f "$CONFIG_FILE" ]; then
      cmp -s "$BACKPACK_FILE" "$CFG_FILE"
      if [ "$?" -ne 0 ]; then
         LOG_OUT "Config File【$name】Are Updates, Start Replacing..."
         cp "$CFG_FILE" "$BACKPACK_FILE"
         #保留规则部分
         if [ "$servers_update" -eq 1 ] && [ "$only_download" -eq 0 ]; then
   	        ruby -ryaml -rYAML -I "/usr/share/openclash" -E UTF-8 -e "
               Value = YAML.load_file('$CONFIG_FILE');
               Value_1 = YAML.load_file('$CFG_FILE');
               if Value.key?('rules') or Value.key?('script') or Value.key?('rule-providers') then
                  if Value.key?('rules') then
                     Value_1['rules'] = Value['rules']
                  end;
                  if Value.key?('script') then
                     Value_1['script'] = Value['script']
                  end;
                  if Value.key?('rule-providers') then
                     Value_1['rule-providers'] = Value['rule-providers']
                  end;
                  File.open('$CFG_FILE','w') {|f| YAML.dump(Value_1, f)};
               end;
            " 2>/dev/null
         fi
         mv "$CFG_FILE" "$CONFIG_FILE" 2>/dev/null
         if [ "$only_download" -eq 0 ]; then
            config_cus_up
         else
            LOG_OUT "Config File【$name】Update Successful!"
            SLOG_CLEAN
         fi
      else
         LOG_OUT "Config File【$name】No Change, Do Nothing!"
         rm -rf "$CFG_FILE"
         SLOG_CLEAN
      fi
   else
      LOG_OUT "Config File【$name】Download Successful, Start To Create..."
      mv "$CFG_FILE" "$CONFIG_FILE" 2>/dev/null
      cp "$CONFIG_FILE" "$BACKPACK_FILE"
      if [ "$only_download" -eq 0 ]; then
         config_cus_up
      else
         LOG_OUT "Config File【$name】Update Successful!"
         SLOG_CLEAN
      fi
   fi
}

config_error()
{
   LOG_OUT "Error:【$name】Update Error, Please Try Again Later..."
   rm -rf "$CFG_FILE" 2>/dev/null
   SLOG_CLEAN
}

change_dns()
{
   if pidof clash >/dev/null; then
      if [ "$enable_redirect_dns"  = "1" ]; then
         uci -q del dhcp.@dnsmasq[-1].server
         uci -q add_list dhcp.@dnsmasq[0].server=127.0.0.1#"$dns_port"
         uci -q delete dhcp.@dnsmasq[0].resolvfile
         uci -q set dhcp.@dnsmasq[0].noresolv=1
         [ "$disable_masq_cache" -eq 1 ] && {
            uci -q set dhcp.@dnsmasq[0].cachesize=0
         }
         uci commit dhcp
      fi

      if [ -z "$(echo "$en_mode" |grep "redir-host")" ] && [ "$china_ip_route" -eq 1 ] && [ "$enable_redirect_dns" != "2" ]; then
         cat "/etc/openclash/accelerated-domains.china.conf" |awk -v dns="${custom_china_domain_dns_server}" -F '/' '!/^$/&&!/^#/{print $1"/"$2"/"dns}' >${DNSMASQ_CONF_DIR}/dnsmasq_accelerated-domains.china.conf 2>/dev/null
         for i in `awk '!/^$/&&!/^#/&&!/(^([1-9]|1[0-9]|1[1-9]{2}|2[0-4][0-9]|25[0-5])\.)(([0-9]{1,2}|1[1-9]{2}|2[0-4][0-9]|25[0-5])\.){2}([1-9]|[1-9][0-9]|1[0-9]{2}|2[0-5][0-9]|25[0-4])((\/[0-9][0-9])?)$/{printf("%s\n",$0)}' /etc/openclash/custom/openclash_custom_chnroute_pass.list`
         do
            sed -i "/server=\/${i}\//d" ${DNSMASQ_CONF_DIR}/dnsmasq_accelerated-domains.china.conf 2>/dev/null
         done 2>/dev/null
      fi

      /etc/init.d/dnsmasq restart >/dev/null 2>&1

      if [ -n "$FW4" ]; then
         for nft in "nat_output" "mangle_output"; do
            local handles=$(nft -a list chain inet fw4 ${nft} |grep -E "openclash|OpenClash" |grep -v "OpenClash DNS Hijack" |awk -F '# handle ' '{print$2}')
            for handle in $handles; do
               nft delete rule inet fw4 ${nft} handle ${handle}
            done
         done >/dev/null 2>&1
         echo "$nat_output_rules" |while read line
         do
            if [ -n "$(echo "$line" |grep "OpenClash DNS Hijack")" ]; then
               continue
            fi
            nft add rule inet fw4 nat_output ${line}
         done >/dev/null 2>&1
         echo "$mangle_output_rules" |while read line
         do
            if [ -n "$(echo "$line" |grep "OpenClash DNS Hijack")" ]; then
               continue
            fi
            nft add rule inet fw4 mangle_output ${line}
         done >/dev/null 2>&1

         if [ "$enable_redirect_dns" = "2" ]; then
            if [ "$router_self_proxy" = 1 ]; then
               nft add rule inet fw4 nat_output position 0 tcp dport 53 ip daddr {127.0.0.1} meta skuid != 65534 counter redirect to "$dns_port" comment \"OpenClash DNS Hijack\" 2>/dev/null
               nft add rule inet fw4 nat_output position 0 udp dport 53 ip daddr {127.0.0.1} meta skuid != 65534 counter redirect to "$dns_port" comment \"OpenClash DNS Hijack\" 2>/dev/null
            fi
            if [ "$ipv6_enable" -eq 1 ]; then
               if [ "$router_self_proxy" = 1 ]; then
                  nft add rule inet fw4 nat_output position 0 meta nfproto {ipv6} tcp dport 53 ip daddr {::/0} meta skuid != 65534 counter redirect to "$dns_port" comment \"OpenClash DNS Hijack\" 2>/dev/null
                  nft add rule inet fw4 nat_output position 0 meta nfproto {ipv6} udp dport 53 ip daddr {::/0} meta skuid != 65534 counter redirect to "$dns_port" comment \"OpenClash DNS Hijack\" 2>/dev/null
               fi
            fi
         fi
         if [ -z "$(echo "$en_mode" |grep "redir-host")" ] && [ "$china_ip_route" -eq 1 ] && [ "$enable_redirect_dns" != "2" ]; then
            LOG_OUT "Tip: Bypass the China IP May Cause the Dnsmasq Load For a Long Time After Restart in FAKE-IP Mode, Hijack the DNS to Core Untill the Dnsmasq Works Well..."
            nft insert rule inet fw4 dstnat position 0 tcp dport 53 counter redirect to "$dns_port" comment \"OpenClash DNS Hijack\" 2>/dev/null
            nft insert rule inet fw4 dstnat position 0 udp dport 53 counter redirect to "$dns_port" comment \"OpenClash DNS Hijack\" 2>/dev/null
            nft 'add chain inet fw4 nat_output { type nat hook output priority -1; }' 2>/dev/null
            nft add rule inet fw4 nat_output position 0 tcp dport 53 meta skuid != 65534 counter redirect to "$dns_port" comment \"OpenClash DNS Hijack\" 2>/dev/null
            nft add rule inet fw4 nat_output position 0 udp dport 53 meta skuid != 65534 counter redirect to "$dns_port" comment \"OpenClash DNS Hijack\" 2>/dev/null
            nft add rule inet fw4 nat_output position 0 tcp dport 12353 meta skuid != 65534 counter redirect to "$DNSPORT" comment \"OpenClash DNS Hijack\" 2>/dev/null
            nft add rule inet fw4 nat_output position 0 udp dport 12353 meta skuid != 65534 counter redirect to "$DNSPORT" comment \"OpenClash DNS Hijack\" 2>/dev/null
            if [ "$ipv6_enable" -eq 1 ]; then
               nft insert rule inet fw4 dstnat position 0 meta nfproto {ipv6} tcp dport 53 counter redirect to "$dns_port" comment \"OpenClash DNS Hijack\" 2>/dev/null
               nft insert rule inet fw4 dstnat position 0 meta nfproto {ipv6} udp dport 53 counter redirect to "$dns_port" comment \"OpenClash DNS Hijack\" 2>/dev/null
               nft 'add chain inet fw4 nat_output { type nat hook output priority -1; }' 2>/dev/null
               nft add rule inet fw4 nat_output position 0 meta nfproto {ipv6} tcp dport 53 meta skuid != 65534 counter redirect to "$dns_port" comment \"OpenClash DNS Hijack\" 2>/dev/null
               nft add rule inet fw4 nat_output position 0 meta nfproto {ipv6} udp dport 53 meta skuid != 65534 counter redirect to "$dns_port" comment \"OpenClash DNS Hijack\" 2>/dev/null
               nft add rule inet fw4 nat_output position 0 meta nfproto {ipv6} tcp dport 12353 meta skuid != 65534 counter redirect to "$DNSPORT" comment \"OpenClash DNS Hijack\" 2>/dev/null
               nft add rule inet fw4 nat_output position 0 meta nfproto {ipv6} udp dport 12353 meta skuid != 65534 counter redirect to "$DNSPORT" comment \"OpenClash DNS Hijack\" 2>/dev/null
            fi
         fi
      else
         iptables -t nat -D OUTPUT -j openclash_output >/dev/null 2>&1
         iptables -t mangle -D OUTPUT -j openclash_output >/dev/null 2>&1
         ip6tables -t mangle -D OUTPUT -j openclash_output >/dev/null 2>&1
         iptables -t nat -A OUTPUT -j openclash_output >/dev/null 2>&1
         iptables -t mangle -A OUTPUT -j openclash_output >/dev/null 2>&1
         ip6tables -t mangle -A OUTPUT -j openclash_output >/dev/null 2>&1
         if [ "$enable_redirect_dns" = "2" ]; then
            if [ "$router_self_proxy" = 1 ]; then
               iptables -t nat -I OUTPUT -p udp --dport 53 -d 127.0.0.1 -m owner ! --uid-owner 65534 -j REDIRECT --to-ports "$dns_port" -m comment --comment "OpenClash DNS Hijack" 2>/dev/null
               iptables -t nat -I OUTPUT -p tcp --dport 53 -d 127.0.0.1 -m owner ! --uid-owner 65534 -j REDIRECT --to-ports "$dns_port" -m comment --comment "OpenClash DNS Hijack" 2>/dev/null
            fi
            if [ "$ipv6_enable" -eq 1 ]; then
               if [ "$router_self_proxy" = 1 ]; then
                  ip6tables -t nat -I OUTPUT -p udp --dport 53 -d ::/0 -m owner ! --uid-owner 65534 -j REDIRECT --to-ports "$dns_port" -m comment --comment "OpenClash DNS Hijack" 2>/dev/null
                  ip6tables -t nat -I OUTPUT -p tcp --dport 53 -d ::/0 -m owner ! --uid-owner 65534 -j REDIRECT --to-ports "$dns_port" -m comment --comment "OpenClash DNS Hijack" 2>/dev/null
               fi
            fi
         fi
         if [ -z "$(echo "$en_mode" |grep "redir-host")" ] && [ "$china_ip_route" -eq 1 ] && [ "$enable_redirect_dns" != "2" ]; then
            LOG_OUT "Tip: Bypass the China IP May Cause the Dnsmasq Load For a Long Time After Restart in FAKE-IP Mode, Hijack the DNS to Core Untill the Dnsmasq Works Well..."
            iptables -t nat -I PREROUTING -p udp --dport 53 -j REDIRECT --to-ports "$dns_port" -m comment --comment "OpenClash DNS Hijack" 2>/dev/null
            iptables -t nat -I PREROUTING -p tcp --dport 53 -j REDIRECT --to-ports "$dns_port" -m comment --comment "OpenClash DNS Hijack" 2>/dev/null
            iptables -t nat -I OUTPUT -p udp --dport 53 -m owner ! --uid-owner 65534 -j REDIRECT --to-ports "$dns_port" -m comment --comment "OpenClash DNS Hijack" 2>/dev/null
            iptables -t nat -I OUTPUT -p tcp --dport 53 -m owner ! --uid-owner 65534 -j REDIRECT --to-ports "$dns_port" -m comment --comment "OpenClash DNS Hijack" 2>/dev/null
            iptables -t nat -I OUTPUT -p udp --dport 12353 -m owner ! --uid-owner 65534 -j REDIRECT --to-ports "$DNSPORT" -m comment --comment "OpenClash DNS Hijack" 2>/dev/null
            iptables -t nat -I OUTPUT -p tcp --dport 12353 -m owner ! --uid-owner 65534 -j REDIRECT --to-ports "$DNSPORT" -m comment --comment "OpenClash DNS Hijack" 2>/dev/null
            if [ "$ipv6_enable" -eq 1 ]; then
               ip6tables -t nat -I PREROUTING -p udp --dport 53 -j REDIRECT --to-ports "$dns_port" -m comment --comment "OpenClash DNS Hijack" 2>/dev/null
               ip6tables -t nat -I PREROUTING -p tcp --dport 53 -j REDIRECT --to-ports "$dns_port" -m comment --comment "OpenClash DNS Hijack" 2>/dev/null
               ip6tables -t nat -I OUTPUT -p udp --dport 53 -m owner ! --uid-owner 65534 -j REDIRECT --to-ports "$dns_port" -m comment --comment "OpenClash DNS Hijack" 2>/dev/null
               ip6tables -t nat -I OUTPUT -p tcp --dport 53 -m owner ! --uid-owner 65534 -j REDIRECT --to-ports "$dns_port" -m comment --comment "OpenClash DNS Hijack" 2>/dev/null
               ip6tables -t nat -I OUTPUT -p udp --dport 12353 -m owner ! --uid-owner 65534 -j REDIRECT --to-ports "$DNSPORT" -m comment --comment "OpenClash DNS Hijack" 2>/dev/null
               ip6tables -t nat -I OUTPUT -p tcp --dport 12353 -m owner ! --uid-owner 65534 -j REDIRECT --to-ports "$DNSPORT" -m comment --comment "OpenClash DNS Hijack" 2>/dev/null
            fi
         fi
      fi
      [ "$(unify_ps_status "openclash_watchdog.sh")" -eq 0 ] && [ "$(unify_ps_prevent)" -eq 0 ] && nohup /usr/share/openclash/openclash_watchdog.sh &
   fi
}

field_name_check()
{
   #检查field名称（不兼容旧写法）
   ruby -ryaml -rYAML -I "/usr/share/openclash" -E UTF-8 -e "
      Value = YAML.load_file('$CFG_FILE');
      if Value.key?('Proxy') or Value.key?('Proxy Group') or Value.key?('Rule') or Value.key?('rule-provider') then
         if Value.key?('Proxy') then
            Value['proxies'] = Value['Proxy']
            Value.delete('Proxy')
            puts '${LOGTIME} Warning: Proxy is no longer used. Auto replaced by proxies'
         end
         if Value.key?('Proxy Group') then
            Value['proxy-groups'] = Value['Proxy Group']
            Value.delete('Proxy Group')
            puts '${LOGTIME} Warning: Proxy Group is no longer used. Auto replaced by proxy-groups'
         end
         if Value.key?('Rule') then
            Value['rules'] = Value['Rule']
            Value.delete('Rule')
            puts '${LOGTIME} Warning: Rule is no longer used. Auto replaced by rules'
         end
         if Value.key?('rule-provider') then
            Value['rule-providers'] = Value['rule-provider']
            Value.delete('rule-provider')
             puts '${LOGTIME} Warning: rule-provider is no longer used. Auto replaced by rule-providers'
         end;
         File.open('$CFG_FILE','w') {|f| YAML.dump(Value, f)};
      end;
   " 2>/dev/null >> $LOG_FILE
}

config_download_direct()
{
   if pidof clash >/dev/null && [ "$router_self_proxy" = 1 ]; then
      kill_watchdog
      if [ "$enable_redirect_dns" -eq 1 ]; then
         uci -q del_list dhcp.@dnsmasq[0].server=127.0.0.1#"$dns_port"
         if [ -n "$default_resolvfile" ]; then
            uci -q set dhcp.@dnsmasq[0].resolvfile="$default_resolvfile"
         elif [ -s "/tmp/resolv.conf.d/resolv.conf.auto" ] && [ -n "$(grep "nameserver" /tmp/resolv.conf.d/resolv.conf.auto)" ]; then
            uci -q set dhcp.@dnsmasq[0].resolvfile=/tmp/resolv.conf.d/resolv.conf.auto
         elif [ -s "/tmp/resolv.conf.auto" ] && [ -n "$(grep "nameserver" /tmp/resolv.conf.auto)" ]; then
            uci -q set dhcp.@dnsmasq[0].resolvfile=/tmp/resolv.conf.auto
         else
            rm -rf /tmp/resolv.conf.auto 2>/dev/null
            touch /tmp/resolv.conf.auto 2>/dev/null
            cat >> "/tmp/resolv.conf.auto" <<-EOF
# Interface lan
nameserver 114.114.114.114
nameserver 119.29.29.29
EOF
            uci -q set dhcp.@dnsmasq[0].resolvfile=/tmp/resolv.conf.auto
         fi
         uci -q set dhcp.@dnsmasq[0].noresolv=0
         uci -q delete dhcp.@dnsmasq[0].cachesize
         uci commit dhcp
         rm -rf ${DNSMASQ_CONF_DIR}/dnsmasq_accelerated-domains.china.conf >/dev/null 2>&1
         /etc/init.d/dnsmasq restart >/dev/null 2>&1
      fi
      if [ -n "$FW4" ]; then
         nat_output_rules=$(nft -a list chain inet fw4 nat_output |grep -E "openclash|OpenClash" |awk -F '# handle ' '{print$1}' |sed 's/^[ \t]*//g')
         mangle_output_rules=$(nft -a list chain inet fw4 mangle_output |grep -E "openclash|OpenClash" |awk -F '# handle ' '{print$1}' |sed 's/^[ \t]*//g')
         for nft in "nat_output" "mangle_output"; do
            local handles=$(nft -a list chain inet fw4 ${nft} |grep -E "openclash|OpenClash" |awk -F '# handle ' '{print$2}')
            for handle in $handles; do
               nft delete rule inet fw4 ${nft} handle ${handle}
            done
         done >/dev/null 2>&1
      else
         iptables -t nat -D OUTPUT -j openclash_output >/dev/null 2>&1
         iptables -t mangle -D OUTPUT -j openclash_output >/dev/null 2>&1
         ip6tables -t mangle -D OUTPUT -j openclash_output >/dev/null 2>&1
         for ipt in "iptables -nvL OUTPUT -t nat" "iptables -nvL OUTPUT -t mangle" "ip6tables -nvL OUTPUT -t mangle" "ip6tables -nvL OUTPUT -t nat"; do
            for comment in "OpenClash DNS Hijack"; do
               local lines=$($ipt |sed 1,2d |sed -n "/${comment}/=" 2>/dev/null |sort -rn)
               if [ -n "$lines" ]; then
                  for line in $lines; do
                     $(echo "$ipt" |awk -v OFS=" " '{print $1,$4,$5}' |sed 's/[ ]*$//g') -D $(echo "$ipt" |awk '{print $3}') $line
                  done
               fi
            done
         done >/dev/null 2>&1
      fi
      
      sleep 3

      config_download
      
      if [ "${PIPESTATUS[0]}" -eq 0 ] && [ -s "$CFG_FILE" ]; then
         #prevent ruby unexpected error
         sed -i -E 's/protocol-param: ([^,'"'"'"''}( *#)\n\r]+)/protocol-param: "\1"/g' "$CFG_FILE" 2>/dev/null
         sed -i '/^ \{0,\}enhanced-mode:/d' "$CFG_FILE" >/dev/null 2>&1
         config_test
         if [ $? -ne 0 ]; then
            LOG_OUT "Error: Config File Tested Faild, Please Check The Log Infos!"
            change_dns
            config_error
            return
         fi
         ruby -ryaml -rYAML -I "/usr/share/openclash" -E UTF-8 -e "
         begin
         YAML.load_file('$CFG_FILE');
         rescue Exception => e
         puts '${LOGTIME} Error: Unable To Parse Config File,【' + e.message + '】'
         system 'rm -rf ${CFG_FILE} 2>/dev/null'
         end
         " 2>/dev/null >> $LOG_FILE
         if [ $? -ne 0 ]; then
            LOG_OUT "Error: Ruby Works Abnormally, Please Check The Ruby Library Depends!"
            only_download=1
            change_dns
            config_su_check
         elif [ ! -f "$CFG_FILE" ]; then
            LOG_OUT "Config File Format Validation Failed..."
            change_dns
            config_error
         elif ! "$(ruby_read "$CFG_FILE" ".key?('proxies')")" && ! "$(ruby_read "$CFG_FILE" ".key?('proxy-providers')")" ; then
            field_name_check
            if ! "$(ruby_read "$CFG_FILE" ".key?('proxies')")" && ! "$(ruby_read "$CFG_FILE" ".key?('proxy-providers')")" ; then
               LOG_OUT "Error: Updated Config【$name】Has No Proxy Field, Update Exit..."
               change_dns
               config_error
            else
               change_dns
               config_su_check
            fi
         else
            change_dns
            config_su_check
         fi
      else
         change_dns
         config_error
      fi
   else
      config_error
   fi
}

server_key_match()
{
	 local key_match key_word
	 
   if [ -n "$(echo "$1" |grep "^ \{0,\}$")" ] || [ -n "$(echo "$1" |grep "^\t\{0,\}$")" ]; then
	    return
   fi
	 
   if [ -n "$(echo "$1" |grep "&")" ]; then
      key_word=$(echo "$1" |sed 's/&/ /g')
	    for k in $key_word
	    do
	       if [ -z "$k" ]; then
	          continue
	       fi
	       k="(?=.*$k)"
	       key_match="$key_match$k"
	    done
	    key_match="^($key_match).*"
   else
	    if [ -n "$1" ]; then
	       key_match="($1)"
	    fi
   fi
   
   if [ "$2" = "keyword" ]; then
      if [ -z "$key_match_param" ]; then
         key_match_param="$key_match"
      else
         key_match_param="$key_match_param|$key_match"
      fi
   elif [ "$2" = "ex_keyword" ]; then
   	  if [ -z "$key_ex_match_param" ]; then
         key_ex_match_param="$key_match"
      else
         key_ex_match_param="$key_ex_match_param|$key_match"
      fi
   fi
}

sub_info_get()
{
   local section="$1" subscribe_url template_path subscribe_url_param template_path_encode key_match_param key_ex_match_param c_address de_ex_keyword
   config_get_bool "enabled" "$section" "enabled" "1"
   config_get "name" "$section" "name" ""
   config_get "sub_convert" "$section" "sub_convert" ""
   config_get "address" "$section" "address" ""
   config_get "keyword" "$section" "keyword" ""
   config_get "ex_keyword" "$section" "ex_keyword" ""
   config_get "emoji" "$section" "emoji" ""
   config_get "udp" "$section" "udp" ""
   config_get "skip_cert_verify" "$section" "skip_cert_verify" ""
   config_get "sort" "$section" "sort" ""
   config_get "convert_address" "$section" "convert_address" ""
   config_get "template" "$section" "template" ""
   config_get "node_type" "$section" "node_type" ""
   config_get "rule_provider" "$section" "rule_provider" ""
   config_get "custom_template_url" "$section" "custom_template_url" ""
   config_get "de_ex_keyword" "$section" "de_ex_keyword" ""
   
   if [ "$enabled" -eq 0 ]; then
      return
   fi
   
   if [ -z "$address" ]; then
      return
   fi
   
   if [ "$udp" == "true" ]; then
      udp="&udp=true"
   else
      udp=""
   fi
   
   if [ "$rule_provider" == "true" ]; then
      rule_provider="&expand=false&classic=true"
   else
      rule_provider=""
   fi
   
   if [ -z "$name" ]; then
      name="config"
      CONFIG_FILE="/etc/openclash/config/config.yaml"
      BACKPACK_FILE="/etc/openclash/backup/config.yaml"
   else
      CONFIG_FILE="/etc/openclash/config/$name.yaml"
      BACKPACK_FILE="/etc/openclash/backup/$name.yaml"
   fi

   if [ -n "$2" ] && [ "$2" != "$CONFIG_FILE" ]; then
      return
   fi
   
   if [ ! -z "$keyword" ] || [ ! -z "$ex_keyword" ]; then
      config_list_foreach "$section" "keyword" server_key_match "keyword"
      config_list_foreach "$section" "ex_keyword" server_key_match "ex_keyword"
   fi
   
   if [ -n "$de_ex_keyword" ]; then
      for i in $de_ex_keyword;
      do
      	if [ -z "$key_ex_match_param" ]; then
      	   key_ex_match_param="($i)"
      	else
      	   key_ex_match_param="$key_ex_match_param|($i)"
        fi
      done
   fi
         
   if [ "$sub_convert" -eq 0 ]; then
      subscribe_url=$address
   elif [ "$sub_convert" -eq 1 ] && [ -n "$template" ]; then
      while read line
      do
      	subscribe_url=$([ -n "$subscribe_url" ] && echo "$subscribe_url|")$(urlencode "$line")
      done < <(echo "$address")
      if [ "$template" != "0" ]; then
         template_path=$(grep "^$template," /usr/share/openclash/res/sub_ini.list |awk -F ',' '{print $3}' 2>/dev/null)
      else
         template_path=$custom_template_url
      fi
      if [ -n "$template_path" ]; then
         template_path_encode=$(urlencode "$template_path")
         [ -n "$key_match_param" ] && key_match_param="(?i)$(urlencode "$key_match_param")"
         [ -n "$key_ex_match_param" ] && key_ex_match_param="(?i)$(urlencode "$key_ex_match_param")"
         subscribe_url_param="?target=clash&new_name=true&url=$subscribe_url&config=$template_path_encode&include=$key_match_param&exclude=$key_ex_match_param&emoji=$emoji&list=false&sort=$sort$udp&scv=$skip_cert_verify&append_type=$node_type&fdn=true$rule_provider"
         c_address="$convert_address"
      else
         subscribe_url=$address
      fi
   else
      subscribe_url=$address
   fi

   LOG_OUT "Start Updating Config File【$name】..."

   config_download

   if [ "${PIPESTATUS[0]}" -eq 0 ] && [ -s "$CFG_FILE" ]; then
      #prevent ruby unexpected error
      sed -i -E 's/protocol-param: ([^,'"'"'"''}( *#)\n\r]+)/protocol-param: "\1"/g' "$CFG_FILE" 2>/dev/null
      sed -i '/^ \{0,\}enhanced-mode:/d' "$CFG_FILE" >/dev/null 2>&1
      config_test
      if [ $? -ne 0 ]; then
         LOG_OUT "Error: Config File Tested Faild, Please Check The Log Infos!"
         config_download_direct
         return
      fi
      ruby -ryaml -rYAML -I "/usr/share/openclash" -E UTF-8 -e "
      begin
      YAML.load_file('$CFG_FILE');
      rescue Exception => e
      puts '${LOGTIME} Error: Unable To Parse Config File,【' + e.message + '】'
      system 'rm -rf ${CFG_FILE} 2>/dev/null'
      end
      " 2>/dev/null >> $LOG_FILE
      if [ $? -ne 0 ]; then
         LOG_OUT "Error: Ruby Works Abnormally, Please Check The Ruby Library Depends!"
         only_download=1
         config_su_check
      elif [ ! -f "$CFG_FILE" ]; then
         LOG_OUT "Config File Format Validation Failed, Trying To Download Without Agent..."
         config_download_direct
      elif ! "$(ruby_read "$CFG_FILE" ".key?('proxies')")" && ! "$(ruby_read "$CFG_FILE" ".key?('proxy-providers')")" ; then
         field_name_check
         if ! "$(ruby_read "$CFG_FILE" ".key?('proxies')")" && ! "$(ruby_read "$CFG_FILE" ".key?('proxy-providers')")" ; then
            LOG_OUT "Error: Updated Config【$name】Has No Proxy Field, Trying To Download Without Agent..."
            config_download_direct
         else
            config_su_check
         fi
      else
         config_su_check
      fi
   else
      LOG_OUT "Error: Config File【$name】Subscribed Failed, Trying to Download Without Agent..."
      config_download_direct
   fi
}

#分别获取订阅信息进行处理
config_load "openclash"
config_foreach sub_info_get "config_subscribe" "$1"
uci -q delete openclash.config.config_update_path
uci commit openclash

if [ "$restart" -eq 1 ] && [ "$(unify_ps_prevent)" -eq 0 ] && [ "$(find /tmp/lock/ |grep -v "openclash.lock" |grep -c "openclash")" -le 1 ]; then
   /etc/init.d/openclash restart >/dev/null 2>&1 &
elif [ "$restart" -eq 0 ] && [ "$(unify_ps_prevent)" -eq 0 ] && [ "$(find /tmp/lock/ |grep -v "openclash.lock" |grep -c "openclash")" -le 1 ] && [ "$(uci -q get openclash.config.restart)" -eq 1 ]; then
   /etc/init.d/openclash restart >/dev/null 2>&1 &
   uci -q set openclash.config.restart=0
   uci -q commit openclash
elif [ "$restart" -eq 1 ] && [ "$(unify_ps_prevent)" -eq 0 ] && [ "$(find /tmp/lock/ |grep -v "openclash.lock" |grep -c "openclash")" -gt 1 ]; then
   uci -q set openclash.config.restart=1
   uci -q commit openclash
else
   sed -i '/openclash.sh/d' $CRON_FILE 2>/dev/null
   [ "$(uci -q get openclash.config.auto_update)" -eq 1 ] && [ "$(uci -q get openclash.config.config_auto_update_mode)" -ne 1 ] && echo "0 $(uci -q get openclash.config.auto_update_time) * * $(uci -q get openclash.config.config_update_week_time) /usr/share/openclash/openclash.sh" >> $CRON_FILE
   /etc/init.d/cron restart
fi
del_lock
