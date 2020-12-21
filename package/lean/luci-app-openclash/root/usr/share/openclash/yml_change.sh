#!/bin/sh
. /usr/share/openclash/ruby.sh

LOG_FILE="/tmp/openclash.log"
START_LOG="/tmp/openclash_start.log"
LOGTIME=$(date "+%Y-%m-%d %H:%M:%S")

if [ "$14" != "1" ]; then
   controller_address="0.0.0.0"
   bind_address="*"
else
   controller_address=$11
   bind_address=$11
fi

if [ -n "$(ruby_read "$7" "['tun']")" ]; then
   if [ -n "$(ruby_read "$7" "['tun']['device-url']")" ]; then
      if [ "$15" -eq 1 ] || [ "$15" -eq 3 ]; then
         uci set openclash.config.config_reload=0
      fi
   else
      uci set openclash.config.config_reload=0
   fi
else
   if [ -n "$15" ]; then
      uci set openclash.config.config_reload=0
   fi
fi

if [ -z "$15" ]; then
   en_mode_tun=0
else
   en_mode_tun=$15
fi

if [ -z "$16" ]; then
   stack_type=system
else
   stack_type=$16
fi

if [ "$(ruby_read "$7" "['external-controller']")" != "$controller_address:$5" ]; then
   uci set openclash.config.config_reload=0
fi
    
if [ "$(ruby_read "$7" "['secret']")" != "$4" ]; then
   uci set openclash.config.config_reload=0
fi
uci commit openclash

if [ "$2" = "fake-ip" ]; then
   if [ ! -f "/tmp/openclash_fake_filter.list" ] || [ ! -z "$(grep "config servers" /etc/config/openclash 2>/dev/null)" ]; then
      /usr/share/openclash/openclash_fake_filter.sh
   fi
   if [ -s "/tmp/openclash_servers_fake_filter.conf" ]; then
      mkdir -p /tmp/dnsmasq.d
      ln -s /tmp/openclash_servers_fake_filter.conf /tmp/dnsmasq.d/dnsmasq_openclash.conf
   fi
fi

ruby -ryaml -E UTF-8 -e "
begin
   Value = YAML.load_file('$7');
rescue Exception => e
puts '${LOGTIME} Load File Error: ' + e.message
end
begin
Value['redir-port']=$6;
Value['port']=$9;
Value['socks-port']=$10;
Value['mixed-port']=$19;
Value['mode']='$13';
Value['log-level']='$12';
Value['allow-lan']=true;
Value['external-controller']='$controller_address:$5';
Value['secret']='$4';
Value['bind-address']='$bind_address';
Value['external-ui']='/usr/share/openclash/dashboard';
if not Value.key?('dns') then
   Value_1={'dns'=>{'enable'=>true}}
   Value['dns']=Value_1['dns']
else
   Value['dns']['enable']=true
end;
if $8 == 1 then
   Value['dns']['ipv6']=true
   Value['ipv6']=true
else
   Value['dns']['ipv6']=false
   Value['ipv6']=false
end;
Value['dns']['enhanced-mode']='$2';
if '$2' == 'fake-ip' then
   Value['dns']['fake-ip-range']='198.18.0.1/16'
else
   Value['dns'].delete('fake-ip-range')
end;
if $8 != 1 then
   Value['dns']['listen']='127.0.0.1:$17'
else
   Value['dns']['listen']='0.0.0.0:$17'
end;
Value_2={'tun'=>{'enable'=>true}};
if $en_mode_tun == 1 or $en_mode_tun == 3 then
   Value['tun']=Value_2['tun']
   Value['tun']['stack']='$stack_type'
   Value_2={'dns-hijack'=>['tcp://8.8.8.8:53','tcp://8.8.4.4:53']}
   Value['tun'].merge!(Value_2)
elsif $en_mode_tun == 2
   Value['tun']=Value_2['tun']
   Value['tun']['device-url']='dev://clash0'
   Value['tun']['dns-listen']='0.0.0.0:53'
elsif $en_mode_tun == 0
   if Value.key?('tun') then
      Value['tun'].clear
   end
end;
rescue Exception => e
puts '${LOGTIME} Set General Error: ' + e.message
end
begin
#添加自定义Hosts设置
if '$2' == 'redir-host' then
   if File::exist?('/etc/openclash/custom/openclash_custom_hosts.list') then
      Value_3 = YAML.load_file('/etc/openclash/custom/openclash_custom_hosts.list')
      if Value_3 != false then
         Value['dns']['use-hosts']=true
         if Value.has_key?('hosts') and not Value['hosts'].to_a.empty? then
            Value['hosts'].merge!(Value_3)
            Value['hosts'].uniq
         else
            Value['hosts']=Value_3
         end
      end
   end
end;
rescue Exception => e
puts '${LOGTIME} Set Hosts Rules Error: ' + e.message
end
begin
#fake-ip-filter
if '$2' == 'fake-ip' then
   if File::exist?('/tmp/openclash_fake_filter.list') then
     Value_4 = YAML.load_file('/tmp/openclash_fake_filter.list')
     if Value_4 != false then
        if Value['dns'].has_key?('fake-ip-filter') and not Value['dns']['fake-ip-filter'].to_a.empty? then
           Value_5 = Value_4['fake-ip-filter'].reverse!
           Value_5.each{|x| Value['dns']['fake-ip-filter'].insert(-1,x)}
           Value['dns']['fake-ip-filter']=Value['dns']['fake-ip-filter'].uniq
        else
           Value['dns']['fake-ip-filter']=Value_4['fake-ip-filter']
        end
     end
  end
end;
rescue Exception => e
puts '${LOGTIME} Set Fake IP Filter Error: ' + e.message
ensure
File.open('$7','w') {|f| YAML.dump(Value, f)}
end" 2>/dev/null >> $LOG_FILE