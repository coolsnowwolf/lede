#!/bin/sh
. /lib/functions.sh
. /usr/share/openclash/ruby.sh
. /usr/share/openclash/log.sh

LOGTIME=$(echo $(date "+%Y-%m-%d %H:%M:%S"))
LOG_FILE="/tmp/openclash.log"
RULE_PROVIDER_FILE="/tmp/yaml_rule_provider.yaml"
GAME_RULE_FILE="/tmp/yaml_game_rule.yaml"
github_address_mod=$(uci -q get openclash.config.github_address_mod || echo 0)
urltest_address_mod=$(uci -q get openclash.config.urltest_address_mod || echo 0)
tolerance=$(uci -q get openclash.config.tolerance || echo 0)
urltest_interval_mod=$(uci -q get openclash.config.urltest_interval_mod || echo 0)
CONFIG_NAME="$5"

#处理自定义规则集
yml_set_custom_rule_provider()
{
   local section="$1"
   local enabled name config type behavior path url interval group position
   config_get_bool "enabled" "$section" "enabled" "1"
   config_get "name" "$section" "name" ""
   config_get "config" "$section" "config" ""
   config_get "type" "$section" "type" ""
   config_get "behavior" "$section" "behavior" ""
   config_get "path" "$section" "path" ""
   config_get "url" "$section" "url" ""
   config_get "interval" "$section" "interval" ""
   config_get "group" "$section" "group" ""
   config_get "position" "$section" "position" ""
   config_get "format" "$section" "format" ""

   if [ "$enabled" = "0" ]; then
      return
   fi

   if [ -n "$(grep "$url" "$RULE_PROVIDER_FILE" 2>/dev/null)" ] && [ -n "$url" ]; then
      return
   fi
   
   if [ -n "$config" ] && [ "$config" != "$CONFIG_NAME" ] && [ "$config" != "all" ]; then
      return
   fi
   
   if [ -z "$name" ] || [ -z "$type" ] || [ -z "$behavior" ]; then
      return
   fi

   if [ "$type" = "http" ] && [ -z "$url" ]; then
      return
   fi

   if [ "$path" != "./rule_provider/$name.yaml" ] && [ "$type" = "http" ]; then
      path="./rule_provider/$name.yaml"
   elif [ -z "$path" ]; then
      return
   fi
  
   if [ -n "$(grep "$path" "$RULE_PROVIDER_FILE" 2>/dev/null)" ]; then
      return
   fi

   if [ -z "$interval" ] && [ "$type" = "http" ]; then
      interval=86400
   fi

cat >> "$RULE_PROVIDER_FILE" <<-EOF
  $name:
    type: $type
    behavior: $behavior
    path: $path
EOF
    if [ -n "$format" ]; then
cat >> "$RULE_PROVIDER_FILE" <<-EOF
    format: $format
EOF
    fi
    if [ "$type" = "http" ]; then
cat >> "$RULE_PROVIDER_FILE" <<-EOF
    url: $url
    interval: $interval
EOF
    fi

   yml_rule_set_add "$name" "$group" "$position"
}

yml_rule_set_add()
{
   if [ -z "$3" ]; then
      return
   fi

   if [ "$3" == "1" ]; then
      if [ -z "$(grep "^ \{0,\}rules:$" /tmp/yaml_rule_set_bottom_custom.yaml 2>/dev/null)" ]; then
         echo "rules:" > "/tmp/yaml_rule_set_bottom_custom.yaml"
      fi
      echo "- RULE-SET,${1},${2}" >> "/tmp/yaml_rule_set_bottom_custom.yaml"
   else
      if [ -z "$(grep "^ \{0,\}rules:$" /tmp/yaml_rule_set_top_custom.yaml 2>/dev/null)" ]; then
         echo "rules:" > "/tmp/yaml_rule_set_top_custom.yaml"
      fi
      echo "- RULE-SET,${1},${2}" >> "/tmp/yaml_rule_set_top_custom.yaml"
   fi
}

yml_gen_rule_provider_file()
{
   if [ -z "$1" ]; then
      return
   fi
   
   RULE_PROVIDER_FILE_NAME=$(grep "^$1," /usr/share/openclash/res/rule_providers.list |awk -F ',' '{print $6}' 2>/dev/null)
   if [ -z "$RULE_PROVIDER_FILE_NAME" ]; then
      RULE_PROVIDER_FILE_NAME=$(grep "^$1," /usr/share/openclash/res/rule_providers.list |awk -F ',' '{print $5}' 2>/dev/null)
   fi
   RULE_PROVIDER_FILE_BEHAVIOR=$(grep ",$RULE_PROVIDER_FILE_NAME$" /usr/share/openclash/res/rule_providers.list |awk -F ',' '{print $3}' 2>/dev/null)
   RULE_PROVIDER_FILE_PATH="/etc/openclash/rule_provider/$RULE_PROVIDER_FILE_NAME"
   RULE_PROVIDER_FILE_URL_PATH="$(grep ",$RULE_PROVIDER_FILE_NAME$" /usr/share/openclash/res/rule_providers.list |awk -F ',' '{print $4$5}' 2>/dev/null)"
   if [ "$github_address_mod" -eq 0 ]; then
      RULE_PROVIDER_FILE_URL="https://raw.githubusercontent.com/${RULE_PROVIDER_FILE_URL_PATH}"
   else
      if [ "$github_address_mod" == "https://cdn.jsdelivr.net/" ] || [ "$github_address_mod" == "https://fastly.jsdelivr.net/" ] || [ "$github_address_mod" == "https://testingcf.jsdelivr.net/" ]; then
         RULE_PROVIDER_FILE_URL="${github_address_mod}gh/"$(echo "$RULE_PROVIDER_FILE_URL_PATH" |awk -F '/master' '{print $1}' 2>/dev/null)"@master"$(echo "$RULE_PROVIDER_FILE_URL_PATH" |awk -F 'master' '{print $2}')""
      elif [ "$github_address_mod" == "https://raw.fastgit.org/" ]; then
         RULE_PROVIDER_FILE_URL="https://raw.fastgit.org/"$(echo "$RULE_PROVIDER_FILE_URL_PATH" |awk -F '/master' '{print $1}' 2>/dev/null)"/master"$(echo "$RULE_PROVIDER_FILE_URL_PATH" |awk -F 'master' '{print $2}')""
      else
         RULE_PROVIDER_FILE_URL="${github_address_mod}https://raw.githubusercontent.com/${RULE_PROVIDER_FILE_URL_PATH}"
      fi
      
   fi
   if [ -n "$(grep "$RULE_PROVIDER_FILE_URL" $RULE_PROVIDER_FILE 2>/dev/null)" ]; then
      return
   fi

cat >> "$RULE_PROVIDER_FILE" <<-EOF
  $1:
    type: http
    behavior: $RULE_PROVIDER_FILE_BEHAVIOR
    path: $RULE_PROVIDER_FILE_PATH
    url: $RULE_PROVIDER_FILE_URL
EOF
   if [ -z "$3" ]; then
cat >> "$RULE_PROVIDER_FILE" <<-EOF
    interval: 86400
EOF
   else
cat >> "$RULE_PROVIDER_FILE" <<-EOF
    interval: $3
EOF
   fi
   yml_rule_set_add "$1" "$2" "$4"
}

yml_get_rule_provider()
{
   local section="$1"
   local enabled group config interval position
   config_get_bool "enabled" "$section" "enabled" "1"
   config_get "group" "$section" "group" ""
   config_get "config" "$section" "config" ""
   config_get "interval" "$section" "interval" ""
   config_get "position" "$section" "position" ""

   if [ "$enabled" = "0" ]; then
      return
   fi
   
   if [ -n "$config" ] && [ "$config" != "$CONFIG_NAME" ] && [ "$config" != "all" ]; then
      return
   fi

   if [ -z "$group" ]; then
      return
   fi
   
   config_list_foreach "$section" "rule_name" yml_gen_rule_provider_file "$group" "$interval" "$position"
}

get_rule_file()
{
   if [ -z "$1" ]; then
      return
   fi
   
   GAME_RULE_FILE_NAME=$(grep "^$1," /usr/share/openclash/res/game_rules.list |awk -F ',' '{print $3}' 2>/dev/null)

   if [ -z "$GAME_RULE_FILE_NAME" ]; then
      GAME_RULE_FILE_NAME=$(grep "^$1," /usr/share/openclash/res/game_rules.list |awk -F ',' '{print $2}' 2>/dev/null)
   fi
   
   GAME_RULE_PATH="./game_rules/$GAME_RULE_FILE_NAME"

   yml_rule_set_add "$1" "$2" "1"

cat >> "$RULE_PROVIDER_FILE" <<-EOF
  $1:
    type: file
    behavior: ipcidr
    path: '${GAME_RULE_PATH}'
EOF
}

yml_game_rule_get()
{
   local section="$1"
   local enabled group config
   config_get_bool "enabled" "$section" "enabled" "1"
   config_get "group" "$section" "group" ""
   config_get "config" "$section" "config" ""

   if [ "$enabled" = "0" ]; then
      return
   fi
   
   if [ -n "$config" ] && [ "$config" != "$CONFIG_NAME" ] && [ "$config" != "all" ]; then
      return
   fi
   
   if [ -z "$group" ]; then
      return
   fi
   
   config_list_foreach "$section" "rule_name" get_rule_file "$group"
}

yml_rule_group_get()
{
   local section="$1"
   local enabled group config
   config_get_bool "enabled" "$section" "enabled" "1"
   config_get "group" "$section" "group" ""
   config_get "config" "$section" "config" ""

   if [ "$enabled" = "0" ]; then
      return
   fi
   
   if [ -n "$config" ] && [ "$config" != "$CONFIG_NAME" ] && [ "$config" != "all" ]; then
      return
   fi
   
   if [ -z "$group" ] || [ "$group" = "DIRECT" ] || [ "$group" = "REJECT" ]; then
      return
   fi

   group_check=$(ruby -ryaml -rYAML -I "/usr/share/openclash" -E UTF-8 -e "
   begin
      Thread.new{
         Value = YAML.load_file('$2');
         Value['proxy-groups'].each{
            |x|
            if x['name'] == '$group' then
               if (x.key?('use') and not x['use'].to_a.empty?) or (x.key?('proxies') and not x['proxies'].to_a.empty?) then
                  puts 'return'
               end;
            end;
         };
      }.join;
   rescue Exception => e
      puts 'return'
   end;" 2>/dev/null)

   if [ "$group_check" != "return" ]; then
      /usr/share/openclash/yml_groups_set.sh >/dev/null 2>&1 "$group"
   fi
}

yml_other_set()
{
   config_load "openclash"
   config_foreach yml_get_rule_provider "rule_provider_config"
   config_foreach yml_set_custom_rule_provider "rule_providers"
   config_foreach yml_game_rule_get "game_config"
   #添加缺失的节点与策略组
   config_foreach yml_rule_group_get "rule_provider_config" "$3"
   config_foreach yml_rule_group_get "rule_providers" "$3"
   config_foreach yml_rule_group_get "game_config" "$3"
   local fake_ip="$(echo "${12}" |awk -F '/' '{print $1}')"
   ruby -ryaml -rYAML -I "/usr/share/openclash" -E UTF-8 -e "
   begin
      Value = YAML.load_file('$3');
   rescue Exception => e
      puts '${LOGTIME} Error: Load File Failed,【' + e.message + '】';
   end;

   begin
   Thread.new{
      if $4 == 1 then
         Value['rules']=Value['rules'].to_a.insert(0,
         'DOMAIN-SUFFIX,awesome-hd.me,DIRECT',
         'DOMAIN-SUFFIX,broadcasthe.net,DIRECT',
         'DOMAIN-SUFFIX,chdbits.co,DIRECT',
         'DOMAIN-SUFFIX,classix-unlimited.co.uk,DIRECT',
         'DOMAIN-SUFFIX,empornium.me,DIRECT',
         'DOMAIN-SUFFIX,gazellegames.net,DIRECT',
         'DOMAIN-SUFFIX,hdchina.org,DIRECT',
         'DOMAIN-SUFFIX,hdsky.me,DIRECT',
         'DOMAIN-SUFFIX,icetorrent.org,DIRECT',
         'DOMAIN-SUFFIX,jpopsuki.eu,DIRECT',
         'DOMAIN-SUFFIX,keepfrds.com,DIRECT',
         'DOMAIN-SUFFIX,madsrevolution.net,DIRECT',
         'DOMAIN-SUFFIX,m-team.cc,DIRECT',
         'DOMAIN-SUFFIX,nanyangpt.com,DIRECT',
         'DOMAIN-SUFFIX,ncore.cc,DIRECT',
         'DOMAIN-SUFFIX,open.cd,DIRECT',
         'DOMAIN-SUFFIX,ourbits.club,DIRECT',
         'DOMAIN-SUFFIX,passthepopcorn.me,DIRECT',
         'DOMAIN-SUFFIX,privatehd.to,DIRECT',
         'DOMAIN-SUFFIX,redacted.ch,DIRECT',
         'DOMAIN-SUFFIX,springsunday.net,DIRECT',
         'DOMAIN-SUFFIX,tjupt.org,DIRECT',
         'DOMAIN-SUFFIX,totheglory.im,DIRECT',
         'DOMAIN-SUFFIX,smtp,DIRECT',
         'DOMAIN-KEYWORD,announce,DIRECT',
         'DOMAIN-KEYWORD,torrent,DIRECT',
         'DOMAIN-KEYWORD,tracker,DIRECT'
         );
         match_group=Value['rules'].grep(/(MATCH|FINAL)/)[0];
         if not match_group.nil? then
            common_port_group=match_group.split(',')[2] or common_port_group=match_group.split(',')[1];
            if not common_port_group.nil? then
               ruby_add_index = Value['rules'].index(Value['rules'].grep(/(MATCH|FINAL)/).first);
               ruby_add_index ||= -1;
               Value['rules']=Value['rules'].to_a.insert(ruby_add_index,
               'PROCESS-NAME,aria2c,DIRECT',
               'PROCESS-NAME,BitComet,DIRECT',
               'PROCESS-NAME,fdm,DIRECT',
               'PROCESS-NAME,NetTransport,DIRECT',
               'PROCESS-NAME,qbittorrent,DIRECT',
               'PROCESS-NAME,Thunder,DIRECT',
               'PROCESS-NAME,transmission-daemon,DIRECT',
               'PROCESS-NAME,transmission-qt,DIRECT',
               'PROCESS-NAME,uTorrent,DIRECT',
               'PROCESS-NAME,WebTorrent,DIRECT',
               'PROCESS-NAME,aria2c,DIRECT',
               'PROCESS-NAME,fdm,DIRECT',
               'PROCESS-NAME,Folx,DIRECT',
               'PROCESS-NAME,NetTransport,DIRECT',
               'PROCESS-NAME,qbittorrent,DIRECT',
               'PROCESS-NAME,Thunder,DIRECT',
               'PROCESS-NAME,Transmission,DIRECT',
               'PROCESS-NAME,transmission,DIRECT',
               'PROCESS-NAME,uTorrent,DIRECT',
               'PROCESS-NAME,WebTorrent,DIRECT',
               'PROCESS-NAME,WebTorrent Helper,DIRECT',
               'PROCESS-NAME,v2ray,DIRECT',
               'PROCESS-NAME,ss-local,DIRECT',
               'PROCESS-NAME,ssr-local,DIRECT',
               'PROCESS-NAME,ss-redir,DIRECT',
               'PROCESS-NAME,ssr-redir,DIRECT',
               'PROCESS-NAME,ss-server,DIRECT',
               'PROCESS-NAME,trojan-go,DIRECT',
               'PROCESS-NAME,xray,DIRECT',
               'PROCESS-NAME,hysteria,DIRECT',
               'PROCESS-NAME,UUBooster,DIRECT',
               'PROCESS-NAME,uugamebooster,DIRECT',
               'DST-PORT,80,' + common_port_group,
               'DST-PORT,443,' + common_port_group,
               'DST-PORT,22,' + common_port_group
               );
            end;
         end
         Value['rules'].to_a.collect!{|x|x.to_s.gsub(/(^MATCH.*|^FINAL.*)/, 'MATCH,DIRECT')};
      end;
   }.join;
   rescue Exception => e
      puts '${LOGTIME} Error: Set BT/P2P DIRECT Rules Failed,【' + e.message + '】';
   end;
   
   begin
   Thread.new{
      if $6 == 0 and ${11} != 2 and '${13}' == 'fake-ip' then
         if Value.has_key?('rules') and not Value['rules'].to_a.empty? then
            if Value['rules'].to_a.grep(/(?=.*SRC-IP-CIDR,'${fake_ip}')/).empty? then
               Value['rules']=Value['rules'].to_a.insert(0,'SRC-IP-CIDR,${12},DIRECT');
            end
            if Value['rules'].to_a.grep(/(?=.*SRC-IP-CIDR,'$7')/).empty? and not '$7'.empty? then
               Value['rules']=Value['rules'].to_a.insert(0,'SRC-IP-CIDR,$7/32,DIRECT');
            end;
         else
            Value['rules']=['SRC-IP-CIDR,${12},DIRECT','SRC-IP-CIDR,$7/32,DIRECT'];
         end;
      elsif Value.has_key?('rules') and not Value['rules'].to_a.empty? then
         Value['rules'].delete('SRC-IP-CIDR,${12},DIRECT');
         Value['rules'].delete('SRC-IP-CIDR,$7/32,DIRECT');
      end;
   }.join;
   rescue Exception => e
      puts '${LOGTIME} Error: Set Router Self Proxy Rule Failed,【' + e.message + '】';
   end

   #处理规则集
   begin
   Thread.new{
      if File::exist?('$RULE_PROVIDER_FILE') then
         Value_1 = YAML.load_file('$RULE_PROVIDER_FILE');
         if Value.has_key?('rule-providers') and not Value['rule-providers'].to_a.empty? then
            Value['rule-providers'].merge!(Value_1);
         else
            Value['rule-providers']=Value_1;
         end;
      end;
   }.join;
   rescue Exception => e
      puts '${LOGTIME} Error: Custom Rule Provider Merge Failed,【' + e.message + '】';
   end;
      
   begin
   Thread.new{
      if Value.has_key?('rules') and not Value['rules'].to_a.empty? then
         if File::exist?('/tmp/yaml_rule_set_bottom_custom.yaml') then
            if $4 != 1 then
               ruby_add_index = Value['rules'].index(Value['rules'].grep(/(GEOIP|MATCH|FINAL)/).first);
            else
               if Value['rules'].grep(/GEOIP/)[0].nil? or Value['rules'].grep(/GEOIP/)[0].empty? then
                  ruby_add_index = Value['rules'].index(Value['rules'].grep(/DST-PORT,80/).last);
                  ruby_add_index ||= Value['rules'].index(Value['rules'].grep(/(MATCH|FINAL)/).first);
               else
                  ruby_add_index = Value['rules'].index(Value['rules'].grep(/GEOIP/).first);
               end;
            end;
            ruby_add_index ||= -1;
            Value_1 = YAML.load_file('/tmp/yaml_rule_set_bottom_custom.yaml');
            if ruby_add_index != -1 then
               Value_1['rules'].uniq.reverse.each{|x| Value['rules'].insert(ruby_add_index,x)};
            else
               Value_1['rules'].uniq.each{|x| Value['rules'].insert(ruby_add_index,x)};
            end;
         end;
         if File::exist?('/tmp/yaml_rule_set_top_custom.yaml') then
            Value_1 = YAML.load_file('/tmp/yaml_rule_set_top_custom.yaml');
            if Value['rules'].to_a.grep(/(?=.*'${fake_ip}')(?=.*REJECT)/).empty? then
               Value_1['rules'].uniq.reverse.each{|x| Value['rules'].insert(0,x)};
            else
               ruby_add_index = Value['rules'].index(Value['rules'].grep(/(?=.*'${fake_ip}')(?=.*REJECT)/).first);
               Value_1['rules'].uniq.reverse.each{|x| Value['rules'].insert(ruby_add_index + 1,x)};
            end;
         end;
      else
         if File::exist?('/tmp/yaml_rule_set_top_custom.yaml') then
            Value['rules'] = YAML.load_file('/tmp/yaml_rule_set_top_custom.yaml')['rules'].uniq;
         end;
         if File::exist?('/tmp/yaml_rule_set_bottom_custom.yaml') then
            Value_1 = YAML.load_file('/tmp/yaml_rule_set_bottom_custom.yaml');
            if File::exist?('/tmp/yaml_rule_set_top_custom.yaml') then
               Value['rules'] = Value['rules'] | Value_1['rules'].uniq;
            else
               Value['rules'] = Value_1['rules'].uniq;
            end;
         end;
      end;
   }.join;
   rescue Exception => e
      puts '${LOGTIME} Error: Rule Set Add Failed,【' + e.message + '】';
   end;

   begin
   Thread.new{
      if File::exist?('/tmp/yaml_groups.yaml') or File::exist?('/tmp/yaml_servers.yaml') or File::exist?('/tmp/yaml_provider.yaml') then
         if File::exist?('/tmp/yaml_groups.yaml') then
            Value_1 = YAML.load_file('/tmp/yaml_groups.yaml');
            if Value.has_key?('proxy-groups') and not Value['proxy-groups'].to_a.empty? then
               Value['proxy-groups'] = Value['proxy-groups'] + Value_1;
               Value['proxy-groups'].uniq;
            else
               Value['proxy-groups'] = Value_1;
            end;
         end;
         if File::exist?('/tmp/yaml_servers.yaml') then
            Value_2 = YAML.load_file('/tmp/yaml_servers.yaml');
            if Value.has_key?('proxies') and not Value['proxies'].to_a.empty? then
               Value['proxies'] = Value['proxies'] + Value_2['proxies'];
               Value['proxies'].uniq;
            else
               Value['proxies']=Value_2['proxies'];
            end
         end;
         if File::exist?('/tmp/yaml_provider.yaml') then
            Value_3 = YAML.load_file('/tmp/yaml_provider.yaml');
            if Value.has_key?('proxy-providers') and not Value['proxy-providers'].to_a.empty? then
               Value['proxy-providers'].merge!(Value_3['proxy-providers']);
               Value['proxy-providers'].uniq;
            else
               Value['proxy-providers']=Value_3['proxy-providers'];
            end;
         end;
      end;
   }.join;
   rescue Exception => e
      puts '${LOGTIME} Error: Game Proxy Merge Failed,【' + e.message + '】';
   end;

   begin
   Thread.new{
   if $2 == 1 then
   #script
      if ${10} != 1 then
         for i in ['/etc/openclash/custom/openclash_custom_rules.list','/etc/openclash/custom/openclash_custom_rules_2.list'] do
            if File::exist?(i) then
               Value_1 = YAML.load_file(i);
               if Value_1 != false then
                  if Value_1.class.to_s == 'Hash' then
                     if Value_1['script'] and Value_1['script'].class.to_s != 'Array' then
                        if Value.key?('script') and not Value_1['script'].to_a.empty? then
                           if Value['script'].key?('code') and Value_1['script'].key?('code') then
                              if not Value['script']['code'].include?('def main(ctx, metadata):') then
                                 Value['script']['code'] = Value_1['script']['code'];
                              else
                                 if i == '/etc/openclash/custom/openclash_custom_rules.list' then
                                    if not Value_1['script']['code'].include?('def main(ctx, metadata):') then
                                       Value['script']['code'].gsub!('def main(ctx, metadata):', \"def main(ctx, metadata):\n\" + Value_1['script']['code']);
                                    else
                                       Value['script']['code'].gsub!('def main(ctx, metadata):', Value_1['script']['code']);
                                    end;
                                 else
                                    insert_index = Value['script']['code'].index(/ctx.geoip/);
                                    insert_index ||= Value['script']['code'].rindex(/return/);
                                    insert_index ||= -1;
                                    if insert_index != -1 then
                                       insert_index  = Value['script']['code'].rindex(\"\n\", insert_index) + 1;
                                    end
                                    if not Value_1['script']['code'].include?('def main(ctx, metadata):') then
                                       Value['script']['code'].insert(insert_index, Value_1['script']['code']);
                                    else
                                       Value['script']['code'].insert(insert_index, Value_1['script']['code'].gsub('def main(ctx, metadata):', ''));
                                    end;
                                 end;
                              end;
                           elsif Value_1['script'].key?('code') then
                              Value['script']['code'] = Value_1['script']['code'];
                           end;
                           if Value['script'].key?('shortcuts') and Value_1['script'].key?('shortcuts') and not Value_1['script']['shortcuts'].to_a.empty? then
                              Value['script']['shortcuts'].merge!(Value_1['script']['shortcuts']).uniq;
                           elsif Value_1['script'].key?('shortcuts') and not Value_1['script']['shortcuts'].to_a.empty? then
                              Value['script']['shortcuts'] = Value_1['script']['shortcuts'];
                           end;
                        else
                           Value['script'] = Value_1['script'];
                        end;
                     end;
                  end;
               end;
            end;
         end;
      end;
   #rules
      if Value.has_key?('rules') and not Value['rules'].to_a.empty? then
         if File::exist?('/etc/openclash/custom/openclash_custom_rules.list') then
            Value_1 = YAML.load_file('/etc/openclash/custom/openclash_custom_rules.list');
            if Value_1 != false then
               if Value_1.class.to_s == 'Hash' then
                  if not Value_1['rules'].to_a.empty? and Value_1['rules'].class.to_s == 'Array' then
                     Value_2 = Value_1['rules'].to_a.reverse!;
                  end;
               elsif Value_1.class.to_s == 'Array' then
                  Value_2 = Value_1.reverse!;
               end;
               if defined? Value_2 then
                  Value_2.each{|x|
                     if ${10} != 1 then
                        if x =~ /(^GEOSITE,|^AND,|^OR,|^NOT,|^IP-SUFFIX,|^SRC-IP-SUFFIX,|^IN-TYPE,|^SUB-RULE,|PORT,[0-9]+\/+|PORT,[0-9]+-+)/ or x.split(',')[-1] == 'tcp' or x.split(',')[-1] == 'udp' then
                           puts '${LOGTIME} Warning: Skip the Custom Rule that Core not Support【' + x + '】'
                           next
                        end;
                     end;
                     Value['rules'].insert(0,x);
                  };
                  Value['rules'] = Value['rules'].uniq;
               end;
            end;
         end;
         if File::exist?('/etc/openclash/custom/openclash_custom_rules_2.list') then
            Value_3 = YAML.load_file('/etc/openclash/custom/openclash_custom_rules_2.list');
            if Value_3 != false then
               if Value['rules'].grep(/GEOIP/)[0].nil? or Value['rules'].grep(/GEOIP/)[0].empty? then
                  ruby_add_index = Value['rules'].index(Value['rules'].grep(/DST-PORT,80/).last);
                  ruby_add_index ||= Value['rules'].index(Value['rules'].grep(/(MATCH|FINAL)/).first);
               else
                  ruby_add_index = Value['rules'].index(Value['rules'].grep(/GEOIP/).first);
               end;
               ruby_add_index ||= -1;
               if Value_3.class.to_s == 'Hash' then
                  if not Value_3['rules'].to_a.empty? and Value_3['rules'].class.to_s == 'Array' then
                     Value_4 = Value_3['rules'].to_a.reverse!;
                  end;
               elsif Value_3.class.to_s == 'Array' then
                  Value_4 = Value_3.reverse!;
               end;
               if defined? Value_4 then
                  if ruby_add_index == -1 then
                     Value_4 = Value_4.reverse!;
                  end;
                  Value_4.each{|x|
                     if ${10} != 1 then
                        if x =~ /(^GEOSITE,|^AND,|^OR,|^NOT,|^IP-SUFFIX,|^SRC-IP-SUFFIX,|^IN-TYPE,|^SUB-RULE,|PORT,[0-9]+\/+|PORT,[0-9]+-+)/ or x.split(',')[-1] == 'tcp' or x.split(',')[-1] == 'udp' then
                           puts '${LOGTIME} Warning: Skip the Custom Rule that Core not Support【' + x + '】'
                           next
                        end;
                     end;
                     Value['rules'].insert(ruby_add_index,x);
                  };
                  Value['rules'] = Value['rules'].uniq;
               end;
            end;
         end;
      else
         if File::exist?('/etc/openclash/custom/openclash_custom_rules.list') then
            Value_1 = YAML.load_file('/etc/openclash/custom/openclash_custom_rules.list');
            if Value_1 != false then
               if Value_1.class.to_s == 'Hash' then
                 if not Value_1['rules'].to_a.empty? and Value_1['rules'].class.to_s == 'Array' then
                     Value_1['rules'].each{|x|
                     if ${10} != 1 then
                        if x =~ /(^GEOSITE,|^AND,|^OR,|^NOT,|^IP-SUFFIX,|^SRC-IP-SUFFIX,|^IN-TYPE,|^SUB-RULE,|PORT,[0-9]+\/+|PORT,[0-9]+-+)/ or x.split(',')[-1] == 'tcp' or x.split(',')[-1] == 'udp' then
                           puts '${LOGTIME} Warning: Skip the Custom Rule that Core not Support【' + x + '】'
                           Value_1.delete(x);
                        end;
                     end;
                     };
                    Value['rules'] = Value_1['rules'];
                    Value['rules'] = Value['rules'].uniq;
                 end;
               elsif Value_1.class.to_s == 'Array' then
                  Value_1.each{|x|
                     if ${10} != 1 then
                        if x =~ /(^GEOSITE,|^AND,|^OR,|^NOT,|^IP-SUFFIX,|^SRC-IP-SUFFIX,|^IN-TYPE,|^SUB-RULE,|PORT,[0-9]+\/+|PORT,[0-9]+-+)/ or x.split(',')[-1] == 'tcp' or x.split(',')[-1] == 'udp' then
                           puts '${LOGTIME} Warning: Skip the Custom Rule that Core not Support【' + x + '】'
                           Value_1.delete(x);
                        end;
                     end;
                  };
                  Value['rules'] = Value_1;
                  Value['rules'] = Value['rules'].uniq;
               end;
            end;
         end;
         if File::exist?('/etc/openclash/custom/openclash_custom_rules_2.list') then
            Value_2 = YAML.load_file('/etc/openclash/custom/openclash_custom_rules_2.list');
            if Value_2 != false then
               if Value['rules'].to_a.empty? then
                  if Value_2.class.to_s == 'Hash' then
                     if not Value_2['rules'].to_a.empty? and Value_2['rules'].class.to_s == 'Array' then
                        Value_2['rules'].each{|x|
                           if ${10} != 1 then
                              if x =~ /(^GEOSITE,|^AND,|^OR,|^NOT,|^IP-SUFFIX,|^SRC-IP-SUFFIX,|^IN-TYPE,|^SUB-RULE,|PORT,[0-9]+\/+|PORT,[0-9]+-+)/ or x.split(',')[-1] == 'tcp' or x.split(',')[-1] == 'udp' then
                                 puts '${LOGTIME} Warning: Skip the Custom Rule that Core not Support【' + x + '】'
                                 Value_2.delete(x);
                              end;
                           end;
                        };
                        Value['rules'] = Value_2['rules'];
                        Value['rules'] = Value['rules'].uniq;
                     end;
                  elsif Value_2.class.to_s == 'Array' then
                     Value_2.each{|x|
                        if ${10} != 1 then
                           if x =~ /(^GEOSITE,|^AND,|^OR,|^NOT,|^IP-SUFFIX,|^SRC-IP-SUFFIX,|^IN-TYPE,|^SUB-RULE,|PORT,[0-9]+\/+|PORT,[0-9]+-+)/ or x.split(',')[-1] == 'tcp' or x.split(',')[-1] == 'udp' then
                              puts '${LOGTIME} Warning: Skip the Custom Rule that Core not Support【' + x + '】'
                              Value_2.delete(x);
                           end;
                        end;
                     };
                     Value['rules'] = Value_2;
                     Value['rules'] = Value['rules'].uniq;
                  end;
               else
                  if Value['rules'].grep(/GEOIP/)[0].nil? or Value['rules'].grep(/GEOIP/)[0].empty? then
                     ruby_add_index = Value['rules'].index(Value['rules'].grep(/DST-PORT,80/).last);
                     ruby_add_index ||= Value['rules'].index(Value['rules'].grep(/(MATCH|FINAL)/).first);
                  else
                     ruby_add_index = Value['rules'].index(Value['rules'].grep(/GEOIP/).first);
                  end;
                  ruby_add_index ||= -1;
                  if Value_2.class.to_s == 'Hash' then
                    if not Value_2['rules'].to_a.empty? and Value_2['rules'].class.to_s == 'Array' then
                       Value_3 = Value_2['rules'].to_a.reverse!;
                    end;
                  elsif Value_2.class.to_s == 'Array' then
                     Value_3 = Value_2.reverse!;
                  end;
                  if defined? Value_3 then
                     if ruby_add_index == -1 then
                        Value_3 = Value_3.reverse!;
                     end
                     Value_3.each{|x|
                        if ${10} != 1 then
                           if x =~ /(^GEOSITE,|^AND,|^OR,|^NOT,|^IP-SUFFIX,|^SRC-IP-SUFFIX,|^IN-TYPE,|^SUB-RULE,|PORT,[0-9]+\/+|PORT,[0-9]+-+)/ or x.split(',')[-1] == 'tcp' or x.split(',')[-1] == 'udp' then
                              puts '${LOGTIME} Warning: Skip the Custom Rule that Core not Support【' + x + '】'
                              next
                           end;
                        end;
                        Value['rules'].insert(ruby_add_index,x);
                     };
                     Value['rules'] = Value['rules'].uniq;
                  end;
               end;
            end;
         end;
      end;
   #SUB-RULE
      if Value.has_key?('sub-rules') and not Value['sub-rules'].to_a.empty? then
         if File::exist?('/etc/openclash/custom/openclash_custom_rules.list') then
            Value_1 = YAML.load_file('/etc/openclash/custom/openclash_custom_rules.list');
            if Value_1 != false then
               if Value_1.class.to_s == 'Hash' then
                  if not Value_1['sub-rules'].to_a.empty? and Value_1['sub-rules'].class.to_s == 'Hash' then
                     if ${10} == 1 then
                        Value['sub-rules'] = Value['sub-rules'].merge!(Value_1['sub-rules']);
                     else
                        puts '${LOGTIME} Warning: Skip the Custom Rule that Core not Support【sub-rules】'
                     end;
                  end;
               end;
            end;
         end;
         if File::exist?('/etc/openclash/custom/openclash_custom_rules_2.list') then
            Value_2 = YAML.load_file('/etc/openclash/custom/openclash_custom_rules_2.list');
            if Value_2 != false then
               if Value_2.class.to_s == 'Hash' then
                  if not Value_2['sub-rules'].to_a.empty? and Value_2['sub-rules'].class.to_s == 'Hash' then
                     if ${10} == 1 then
                        Value['sub-rules'] = Value['sub-rules'].merge!(Value_2['sub-rules']);
                     else
                        puts '${LOGTIME} Warning: Skip the Custom Rule that Core not Support【sub-rules】'
                     end;
                  end;
               end;
            end;
         end;
      else
         if File::exist?('/etc/openclash/custom/openclash_custom_rules.list') then
            Value_1 = YAML.load_file('/etc/openclash/custom/openclash_custom_rules.list');
            if Value_1 != false then
               if Value_1.class.to_s == 'Hash' then
                  if not Value_1['sub-rules'].to_a.empty? and Value_1['sub-rules'].class.to_s == 'Hash' then
                     if ${10} == 1 then
                        Value['sub-rules'] = Value_1['sub-rules'];
                     else
                        puts '${LOGTIME} Warning: Skip the Custom Rule that Core not Support【sub-rules】'
                     end;
                  end;
               end;
            end;
         end;
         if File::exist?('/etc/openclash/custom/openclash_custom_rules_2.list') then
            Value_2 = YAML.load_file('/etc/openclash/custom/openclash_custom_rules_2.list');
            if Value_2 != false then
               if Value_2.class.to_s == 'Hash' then
                  if not Value_2['sub-rules'].to_a.empty? and Value_2['sub-rules'].class.to_s == 'Hash' then
                     if ${10} == 1 then 
                        Value['sub-rules'] = Value_2['sub-rules'];
                     else
                        puts '${LOGTIME} Warning: Skip the Custom Rule that Core not Support【sub-rules】'
                     end;
                  end;
               end;
            end;
         end;
      end;
   end;
   }.join;
   rescue Exception => e
      puts '${LOGTIME} Error: Set Custom Rules Failed,【' + e.message + '】';
   end;

   #loop prevent
   begin
   Thread.new{
      if Value.has_key?('rules') and not Value['rules'].to_a.empty? then
         if Value['rules'].to_a.grep(/(?=.*'${fake_ip}')(?=.*REJECT)/).empty? then
            Value['rules']=Value['rules'].to_a.insert(0,'IP-CIDR,${12},REJECT,no-resolve');
         end;
         if Value['rules'].to_a.grep(/(?=.*DST-PORT,'$8',REJECT)/).empty? then
            Value['rules']=Value['rules'].to_a.insert(0,'DST-PORT,$8,REJECT');
         end;
         if Value['rules'].to_a.grep(/(?=.*DST-PORT,'$9',REJECT)/).empty? then
            Value['rules']=Value['rules'].to_a.insert(0,'DST-PORT,$9,REJECT');
         end;
      else
         Value['rules']=['IP-CIDR,${12},REJECT,no-resolve','DST-PORT,$8,REJECT','DST-PORT,$9,REJECT'];
      end;
   }.join;
   rescue Exception => e
      puts '${LOGTIME} Error: Set Loop Protect Rules Failed,【' + e.message + '】';
   end;

   #修改集路径
   begin
   Thread.new{
      provider = {'proxy-providers' => 'proxy_provider', 'rule-providers' => 'rule_provider'}
      provider.each{|i, p|
         if Value.key?(i) then
            Value[i].values.each{
            |x,v|
            if x['path'] and not x['path'] =~ /.\/#{p}\/*/ and not x['path'] =~ /.\/game_rules\/*/ then
               v=File.basename(x['path']);
               x['path']='./'+p+'/'+v;
            end;
            #CDN Replace
            if '$github_address_mod' != '0' then
               if '$github_address_mod' == 'https://cdn.jsdelivr.net/' or '$github_address_mod' == 'https://fastly.jsdelivr.net/' or '$github_address_mod' == 'https://testingcf.jsdelivr.net/'then
                  if x['url'] and x['url'] =~ /^https:\/\/raw.githubusercontent.com/ then
                     x['url'] = '$github_address_mod' + 'gh/' + x['url'].split('/')[3] + '/' + x['url'].split('/')[4] + '@' + x['url'].split(x['url'].split('/')[2] + '/' + x['url'].split('/')[3] + '/' + x['url'].split('/')[4] + '/')[1];
                  end;
               elsif '$github_address_mod' == 'https://raw.fastgit.org/' then
                  if x['url'] and x['url'] =~ /^https:\/\/raw.githubusercontent.com/ then
                     x['url'] = 'https://raw.fastgit.org/' + x['url'].split('/')[3] + '/' + x['url'].split('/')[4] + '/' + x['url'].split(x['url'].split('/')[2] + '/' + x['url'].split('/')[3] + '/' + x['url'].split('/')[4] + '/')[1];
                  end;
               else
                  if x['url'] and x['url'] =~ /^https:\/\/(raw.|gist.)(githubusercontent.com|github.com)/ then
                     x['url'] = '$github_address_mod' + x['url'];
                  end;
               end;
            end;
            };
         end;
      };
   }.join;
   rescue Exception => e
      puts '${LOGTIME} Error: Edit Provider Path Failed,【' + e.message + '】';
   end;

   #tolerance
   begin
   Thread.new{
      if '$tolerance' != '0' then
         Value['proxy-groups'].each{
            |x|
               if x['type'] == 'url-test' then
                  x['tolerance']=${tolerance};
               end
            };
      end;
   }.join;
   rescue Exception => e
      puts '${LOGTIME} Error: Edit URL-Test Group Tolerance Option Failed,【' + e.message + '】';
   end;

   #URL-Test interval
   begin
   Thread.new{
      if '$urltest_interval_mod' != '0' then
         if Value.key?('proxy-groups') then
            Value['proxy-groups'].each{
               |x|
               if x['type'] == 'url-test' or x['type'] == 'fallback' or x['type'] == 'load-balance' then
                  x['interval']=${urltest_interval_mod};
               end
            };
         end;
         if Value.key?('proxy-providers') then
            Value['proxy-providers'].values.each{
               |x|
               if x['health-check'] and x['health-check']['enable'] and x['health-check']['enable'] == 'true' then
                  x['health-check']['interval']=${urltest_interval_mod};
               end;
            };
         end;
      end;
   }.join;
   rescue Exception => e
      puts '${LOGTIME} Error: Edit URL-Test Interval Failed,【' + e.message + '】';
   end;

   #修改测速地址
   begin
   Thread.new{
      if '$urltest_address_mod' != '0' then
         if Value.key?('proxy-providers') then
            Value['proxy-providers'].values.each{
            |x|
            if x['health-check'] and x['health-check']['enable'] and x['health-check']['enable'] == 'true' then
               x['health-check']['url']='$urltest_address_mod';
            end;
            };
         end;
         if Value.key?('proxy-groups') then
            Value['proxy-groups'].each{
            |x|
            if x['type'] == 'url-test' or x['type'] == 'fallback' or x['type'] == 'load-balance' then
               x['url']='$urltest_address_mod';
            end;
            };
         end;
      end;
   }.join;
   rescue Exception => e
      puts '${LOGTIME} Error: Edit URL-Test URL Failed,【' + e.message + '】';
   ensure
      File.open('$3','w') {|f| YAML.dump(Value, f)};
   end" 2>/dev/null >> $LOG_FILE
}

yml_other_rules_get()
{
   local section="$1"
   local enabled config
   config_get_bool "enabled" "$section" "enabled" "1"
   config_get "config" "$section" "config" ""
   
   if [ "$enabled" = "0" ] || [ "$config" != "$2" ]; then
      return
   fi
   
   if [ -n "$rule_name" ]; then
      LOG_OUT "Warning: Multiple Other-Rules-Configurations Enabled, Ignore..."
      return
   fi
   
   config_get "rule_name" "$section" "rule_name" ""
   config_get "GlobalTV" "$section" "GlobalTV" ""
   config_get "AsianTV" "$section" "AsianTV" ""
   config_get "Proxy" "$section" "Proxy" ""
   config_get "Youtube" "$section" "Youtube" ""
   config_get "Bilibili" "$section" "Bilibili" ""
   config_get "Bahamut" "$section" "Bahamut" ""
   config_get "HBOMax" "$section" "HBOMax" "$GlobalTV"
   config_get "HBOGo" "$section" "HBOGo" "$GlobalTV"
   config_get "Pornhub" "$section" "Pornhub" ""
   config_get "Apple" "$section" "Apple" ""
   config_get "Scholar" "$section" "Scholar" ""
   config_get "Netflix" "$section" "Netflix" ""
   config_get "Disney" "$section" "Disney" ""
   config_get "Spotify" "$section" "Spotify" ""
   config_get "Steam" "$section" "Steam" ""
   config_get "AdBlock" "$section" "AdBlock" ""
   config_get "Netease_Music" "$section" "Netease_Music" ""
   config_get "Speedtest" "$section" "Speedtest" ""
   config_get "Telegram" "$section" "Telegram" ""
   config_get "Crypto" "$section" "Crypto" "$Proxy"
   config_get "Discord" "$section" "Discord" "$Proxy"
   config_get "Microsoft" "$section" "Microsoft" ""
   config_get "PayPal" "$section" "PayPal" ""
   config_get "Domestic" "$section" "Domestic" ""
   config_get "Others" "$section" "Others" ""
   config_get "GoogleFCM" "$section" "GoogleFCM" "DIRECT"
   config_get "Discovery" "$section" "Discovery" "$GlobalTV"
   config_get "DAZN" "$section" "DAZN" "$GlobalTV"
   config_get "ChatGPT" "$section" "ChatGPT" "$Proxy"
   config_get "AppleTV" "$section" "AppleTV" "$GlobalTV"
   config_get "miHoYo" "$section" "miHoYo" "$Domestic"
}

if [ "$1" != "0" ]; then
   /usr/share/openclash/yml_groups_name_get.sh
   if [ $? -ne 0 ]; then
      LOG_OUT "Error: Unable To Parse Config File, Please Check And Try Again!"
      exit 0
   fi
   config_load "openclash"
   config_foreach yml_other_rules_get "other_rules" "$5"
   if [ -z "$rule_name" ]; then
      yml_other_set "$1" "$2" "$3" "$4" "$5" "$6" "$7" "$8" "$9" "${10}" "${11}" "${12}" "${13}"
      exit 0
   #判断策略组是否存在
   elif [ "$rule_name" = "ConnersHua_return" ]; then
       if [ -z "$(grep -F "$Proxy" /tmp/Proxy_Group)" ]\
    || [ -z "$(grep -F "$Others" /tmp/Proxy_Group)" ];then
         LOG_OUT "Warning: Because of The Different Porxy-Group's Name, Stop Setting The Other Rules!"
         yml_other_set "$1" "$2" "$3" "$4" "$5" "$6" "$7" "$8" "$9" "${10}" "${11}" "${12}" "${13}"
         exit 0
       fi
   elif [ "$rule_name" = "ConnersHua" ]; then
       if [ -z "$(grep "$GlobalTV" /tmp/Proxy_Group)" ]\
    || [ -z "$(grep -F "$AsianTV" /tmp/Proxy_Group)" ]\
    || [ -z "$(grep -F "$Proxy" /tmp/Proxy_Group)" ]\
    || [ -z "$(grep -F "$Others" /tmp/Proxy_Group)" ]\
    || [ -z "$(grep -F "$Domestic" /tmp/Proxy_Group)" ]; then
         LOG_OUT "Warning: Because of The Different Porxy-Group's Name, Stop Setting The Other Rules!"
         yml_other_set "$1" "$2" "$3" "$4" "$5" "$6" "$7" "$8" "$9" "${10}"  "${11}" "${12}" "${13}"
         exit 0
       fi
   elif [ "$rule_name" = "lhie1" ]; then
       if [ -z "$(grep -F "$GlobalTV" /tmp/Proxy_Group)" ]\
    || [ -z "$(grep -F "$AsianTV" /tmp/Proxy_Group)" ]\
    || [ -z "$(grep -F "$Proxy" /tmp/Proxy_Group)" ]\
    || [ -z "$(grep -F "$Youtube" /tmp/Proxy_Group)" ]\
    || [ -z "$(grep -F "$Bilibili" /tmp/Proxy_Group)" ]\
    || [ -z "$(grep -F "$Bahamut" /tmp/Proxy_Group)" ]\
    || [ -z "$(grep -F "$HBOMax" /tmp/Proxy_Group)" ]\
    || [ -z "$(grep -F "$HBOGo" /tmp/Proxy_Group)" ]\
    || [ -z "$(grep -F "$Pornhub" /tmp/Proxy_Group)" ]\
    || [ -z "$(grep -F "$Apple" /tmp/Proxy_Group)" ]\
    || [ -z "$(grep -F "$AppleTV" /tmp/Proxy_Group)" ]\
    || [ -z "$(grep -F "$Scholar" /tmp/Proxy_Group)" ]\
    || [ -z "$(grep -F "$Netflix" /tmp/Proxy_Group)" ]\
    || [ -z "$(grep -F "$Disney" /tmp/Proxy_Group)" ]\
    || [ -z "$(grep -F "$Discovery" /tmp/Proxy_Group)" ]\
    || [ -z "$(grep -F "$DAZN" /tmp/Proxy_Group)" ]\
    || [ -z "$(grep -F "$ChatGPT" /tmp/Proxy_Group)" ]\
    || [ -z "$(grep -F "$Spotify" /tmp/Proxy_Group)" ]\
    || [ -z "$(grep -F "$Steam" /tmp/Proxy_Group)" ]\
    || [ -z "$(grep -F "$miHoYo" /tmp/Proxy_Group)" ]\
    || [ -z "$(grep -F "$AdBlock" /tmp/Proxy_Group)" ]\
    || [ -z "$(grep -F "$Speedtest" /tmp/Proxy_Group)" ]\
    || [ -z "$(grep -F "$Telegram" /tmp/Proxy_Group)" ]\
    || [ -z "$(grep -F "$Crypto" /tmp/Proxy_Group)" ]\
    || [ -z "$(grep -F "$Discord" /tmp/Proxy_Group)" ]\
    || [ -z "$(grep -F "$Microsoft" /tmp/Proxy_Group)" ]\
    || [ -z "$(grep -F "$PayPal" /tmp/Proxy_Group)" ]\
    || [ -z "$(grep -F "$Others" /tmp/Proxy_Group)" ]\
    || [ -z "$(grep -F "$GoogleFCM" /tmp/Proxy_Group)" ]\
    || [ -z "$(grep -F "$Domestic" /tmp/Proxy_Group)" ]; then
         LOG_OUT "Warning: Because of The Different Porxy-Group's Name, Stop Setting The Other Rules!"
         yml_other_set "$1" "$2" "$3" "$4" "$5" "$6" "$7" "$8" "$9" "${10}" "${11}" "${12}" "${13}"
         exit 0
       fi
   fi
   if [ -z "$Proxy" ]; then
      LOG_OUT "Error: Missing Porxy-Group's Name, Stop Setting The Other Rules!"
      yml_other_set "$1" "$2" "$3" "$4" "$5" "$6" "$7" "$8" "$9" "${10}" "${11}" "${12}" "${13}"
      exit 0
   else
      if [ "$rule_name" = "lhie1" ]; then
         ruby -ryaml -rYAML -I "/usr/share/openclash" -E UTF-8 -e "
         begin
            Value = YAML.load_file('$3');
            Value_1 = YAML.load_file('/usr/share/openclash/res/lhie1.yaml');
            if Value.has_key?('script') then
               Value.delete('script')
            end;
            if Value.has_key?('rules') then
               Value.delete('rules')
            end;
            if Value_1.has_key?('rule-providers') and not Value_1['rule-providers'].to_a.empty? then
               if Value.has_key?('rule-providers') and not Value['rule-providers'].to_a.empty? then
                  Value['rule-providers'].merge!(Value_1['rule-providers'])
               else
                  Value['rule-providers']=Value_1['rule-providers']
               end
            end;
            Value['script']=Value_1['script'];
            Value['rules']=Value_1['rules'];
            Value['rules'].to_a.collect!{|x|
            x.to_s.gsub(/,Bilibili,Asian TV$/, ',Bilibili,$Bilibili#delete_')
            .gsub(/,Bahamut,Global TV$/, ',Bahamut,$Bahamut#delete_')
            .gsub(/,HBO Max,Global TV$/, ',HBO Max,$HBOMax#delete_')
            .gsub(/,HBO Go,Global TV$/, ',HBO Go,$HBOGo#delete_')
            .gsub(/,Discovery Plus,Global TV$/, ',Discovery Plus,$Discovery#delete_')
            .gsub(/,DAZN,Global TV$/, ',DAZN,$DAZN#delete_')
            .gsub(/,Pornhub,Global TV$/, ',Pornhub,$Pornhub#delete_')
            .gsub(/,Global TV$/, ',$GlobalTV#delete_')
            .gsub(/,Asian TV$/, ',$AsianTV#delete_')
            .gsub(/,Proxy$/, ',$Proxy#delete_')
            .gsub(/,YouTube$/, ',$Youtube#delete_')
            .gsub(/,Apple$/, ',$Apple#delete_')
            .gsub(/,Apple TV$/, ',$AppleTV#delete_')
            .gsub(/,Scholar$/, ',$Scholar#delete_')
            .gsub(/,Netflix$/, ',$Netflix#delete_')
            .gsub(/,Disney$/, ',$Disney#delete_')
            .gsub(/,Spotify$/, ',$Spotify#delete_')
            .gsub(/,ChatGPT$/, ',$ChatGPT#delete_')
            .gsub(/,Steam$/, ',$Steam#delete_')
            .gsub(/,miHoYo$/, ',$miHoYo#delete_')
            .gsub(/,AdBlock$/, ',$AdBlock#delete_')
            .gsub(/,Speedtest$/, ',$Speedtest#delete_')
            .gsub(/,Telegram$/, ',$Telegram#delete_')
            .gsub(/,Crypto$/, ',$Crypto#delete_')
            .gsub(/,Discord$/, ',$Discord#delete_')
            .gsub(/,Microsoft$/, ',$Microsoft#delete_')
            .to_s.gsub(/,PayPal$/, ',$PayPal#delete_')
            .gsub(/,Domestic$/, ',$Domestic#delete_')
            .gsub(/,Others$/, ',$Others#delete_')
            .gsub(/,Google FCM$/, ',$GoogleFCM#delete_')
            .gsub(/#delete_/, '')
            };
            Value['script']['code'].to_s.gsub!(/\"Bilibili\": \"Asian TV\"/,'\"Bilibili\": \"$Bilibili#delete_\"')
            .gsub!(/\"Bahamut\": \"Global TV\"/,'\"Bahamut\": \"$Bahamut#delete_\"')
            .gsub!(/\"HBO Max\": \"Global TV\"/,'\"HBO Max\": \"$HBOMax#delete_\"')
            .gsub!(/\"HBO Go\": \"Global TV\"/,'\"HBO Go\": \"$HBOGo#delete_\"')
            .gsub!(/\"Discovery Plus\": \"Global TV\"/,'\"Discovery Plus\": \"$Discovery#delete_\"')
            .gsub!(/\"DAZN\": \"Global TV\"/,'\"DAZN\": \"$DAZN#delete_\"')
            .gsub!(/\"Pornhub\": \"Global TV\"/,'\"Pornhub\": \"$Pornhub#delete_\"')
            .gsub!(/: \"Global TV\"/,': \"$GlobalTV#delete_\"')
            .gsub!(/: \"Asian TV\"/,': \"$AsianTV#delete_\"')
            .gsub!(/: \"Proxy\"/,': \"$Proxy#delete_\"')
            .gsub!(/: \"YouTube\"/,': \"$Youtube#delete_\"')
            .gsub!(/: \"Apple\"/,': \"$Apple#delete_\"')
            .gsub!(/: \"Apple TV\"/,': \"$AppleTV#delete_\"')
            .gsub!(/: \"Scholar\"/,': \"$Scholar#delete_\"')
            .gsub!(/: \"Netflix\"/,': \"$Netflix#delete_\"')
            .gsub!(/: \"Disney\"/,': \"$Disney#delete_\"')
            .gsub!(/: \"Spotify\"/,': \"$Spotify#delete_\"')
            .gsub!(/: \"ChatGPT\"/,': \"$ChatGPT#delete_\"')
            .gsub!(/: \"Steam\"/,': \"$Steam#delete_\"')
            .gsub!(/: \"miHoYo\"/,': \"$miHoYo#delete_\"')
            .gsub!(/: \"AdBlock\"/,': \"$AdBlock#delete_\"')
            .gsub!(/: \"Speedtest\"/,': \"$Speedtest#delete_\"')
            .gsub!(/: \"Telegram\"/,': \"$Telegram#delete_\"')
            .gsub!(/: \"Crypto\"/,': \"$Crypto#delete_\"')
            .gsub!(/: \"Discord\"/,': \"$Discord#delete_\"')
            .gsub!(/: \"Microsoft\"/,': \"$Microsoft#delete_\"')
            .gsub!(/: \"PayPal\"/,': \"$PayPal#delete_\"')
            .gsub!(/: \"Domestic\"/,': \"$Domestic#delete_\"')
            .gsub!(/: \"Google FCM\"/,': \"$GoogleFCM#delete_\"')
            .gsub!(/return \"Domestic\"$/, 'return \"$Domestic#delete_\"')
            .gsub!(/return \"Others\"$/, 'return \"$Others#delete_\"')
            .gsub!(/#delete_/, '');
            File.open('$3','w') {|f| YAML.dump(Value, f)};
         rescue Exception => e
            puts '${LOGTIME} Error: Set lhie1 Rules Failed,【' + e.message + '】';
         end" 2>/dev/null >> $LOG_FILE
      elif [ "$rule_name" = "ConnersHua" ]; then
         ruby -ryaml -rYAML -I "/usr/share/openclash" -E UTF-8 -e "
         begin
            Value = YAML.load_file('$3');
            Value_1 = YAML.load_file('/usr/share/openclash/res/ConnersHua.yaml');
            if Value.has_key?('script') then
               Value.delete('script')
            end;
            if Value.has_key?('rules') then
               Value.delete('rules')
            end;
            if Value_1.has_key?('rule-providers') and not Value_1['rule-providers'].to_a.empty? then
               if Value.has_key?('rule-providers') and not Value['rule-providers'].to_a.empty? then
                  Value['rule-providers'].merge!(Value_1['rule-providers'])
               else
                  Value['rule-providers']=Value_1['rule-providers']
               end
            end;
            Value['rules']=Value_1['rules'];
            Value['rules'].to_a.collect!{|x|
            x.to_s.gsub(/,Streaming$/, ',$GlobalTV#delete_')
            .gsub(/,StreamingSE$/, ',$AsianTV#delete_')
            .gsub(/(,PROXY$|,IP-Blackhole$)/, ',$Proxy#delete_')
            .gsub(/,China,DIRECT$/, ',China,$Domestic#delete_')
            .gsub(/,ChinaIP,DIRECT$/, ',ChinaIP,$Domestic#delete_')
            .gsub(/,CN,DIRECT$/, ',CN,$Domestic#delete_')
            .gsub(/,MATCH$/, ',$Others#delete_')
            .gsub(/#delete_/, '')
            };
            File.open('$3','w') {|f| YAML.dump(Value, f)};
         rescue Exception => e
            puts '${LOGTIME} Error: Set ConnersHua Rules Failed,【' + e.message + '】';
         end" 2>/dev/null >> $LOG_FILE
      else
         ruby -ryaml -rYAML -I "/usr/share/openclash" -E UTF-8 -e "
         begin
            Value = YAML.load_file('$3');
            Value_1 = YAML.load_file('/usr/share/openclash/res/ConnersHua_return.yaml');
            if Value.has_key?('script') then
               Value.delete('script')
            end;
            if Value.has_key?('rules') then
               Value.delete('rules')
            end;
            Value['rules']=Value_1['rules'];
            Value['rules'].to_a.collect!{|x|
            x.to_s.gsub(/,PROXY$/, ',$Proxy#delete_')
            .gsub(/MATCH,DIRECT$/, 'MATCH,$Others#delete_')
            .gsub(/#delete_/, '')
            };
            File.open('$3','w') {|f| YAML.dump(Value, f)};
         rescue Exception => e
            puts '${LOGTIME} Error: Set ConnersHua Return Rules Failed,【' + e.message + '】';
         end" 2>/dev/null >> $LOG_FILE
       fi
   fi
fi

yml_other_set "$1" "$2" "$3" "$4" "$5" "$6" "$7" "$8" "$9" "${10}" "${11}" "${12}" "${13}"
