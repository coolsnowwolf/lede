#!/bin/sh
. /lib/functions.sh
. /usr/share/openclash/ruby.sh
. /usr/share/openclash/log.sh

LOGTIME=$(echo $(date "+%Y-%m-%d %H:%M:%S"))
LOG_FILE="/tmp/openclash.log"

yml_other_set()
{
   ruby -ryaml -E UTF-8 -e "
   begin
   Value = YAML.load_file('$4');
   rescue Exception => e
   puts '${LOGTIME} Error: Load File Error,【' + e.message + '】'
   end
   begin
   if $3 == 1 then
      if Value.has_key?('rules') and not Value['rules'].to_a.empty? then
         if File::exist?('/etc/openclash/custom/openclash_custom_rules.list') then
            Value_1 = YAML.load_file('/etc/openclash/custom/openclash_custom_rules.list')
            if Value_1 != false then
               Value_2 = Value_1.reverse!
               Value_2.each{|x| Value['rules'].insert(0,x)}
               Value['rules']=Value['rules'].uniq
            end
         end
         if File::exist?('/etc/openclash/custom/openclash_custom_rules_2.list') then
            Value_3 = YAML.load_file('/etc/openclash/custom/openclash_custom_rules_2.list')
            if Value_3 != false then
               ruby_add_index = Value['rules'].index(Value['rules'].grep(/(GEOIP|MATCH|FINAL)/).first)
               ruby_add_index ||= -1
               Value_4 = Value_3.reverse!
               Value_4.each{|x| Value['rules'].insert(ruby_add_index,x)}
               Value['rules']=Value['rules'].uniq
            end
         end
      else
         if File::exist?('/etc/openclash/custom/openclash_custom_rules.list') then
            Value_1 = YAML.load_file('/etc/openclash/custom/openclash_custom_rules.list')
            if Value_1 != false then
               Value['rules']=Value_1
               Value['rules']=Value['rules'].uniq
            end
         end
         if File::exist?('/etc/openclash/custom/openclash_custom_rules_2.list') then
            Value_2 = YAML.load_file('/etc/openclash/custom/openclash_custom_rules_2.list')
            if Value_2 != false then
               if Value['rules'].to_a.empty? then
                  Value['rules']=Value_2
               else
                  ruby_add_index = Value['rules'].index(Value['rules'].grep(/(GEOIP|MATCH|FINAL)/).first)
                  ruby_add_index ||= -1
                  Value_3 = Value_2.reverse!
                  Value_3.each{|x| Value['rules'].insert(ruby_add_index,x)}
               end
               Value['rules']=Value['rules'].uniq
            end
         end
      end
   end;
   rescue Exception => e
   puts '${LOGTIME} Error: Set Custom Rules Error,【' + e.message + '】'
   end
   begin
   if $5 == 1 then
      if Value.has_key?('rules') and not Value['rules'].to_a.empty? then
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
         'DOMAIN-SUFFIX,icetorrent.org,DIRECT',
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
         'DOMAIN-KEYWORD,announce,DIRECT',
         'DOMAIN-KEYWORD,torrent,DIRECT'
         )
         begin
         match_group=Value['rules'].grep(/(MATCH|FINAL)/)[0]
         if not match_group.empty? and not match_group.nil? then
            common_port_group=match_group.split(',')[2] or common_port_group=match_group.split(',')[1]
            if not common_port_group.empty? and not common_port_group.nil? then
               ruby_add_index = Value['rules'].index(Value['rules'].grep(/(MATCH|FINAL)/).first)
               ruby_add_index ||= -1
               Value['rules']=Value['rules'].to_a.insert(ruby_add_index,
               'DST-PORT,80,' + common_port_group,
               'DST-PORT,443,' + common_port_group,
               'DST-PORT,22,' + common_port_group
               )
            end
         end
         rescue Exception => e
         puts '${LOGTIME} Error: Set BT/P2P DIRECT Rules Error,【' + e.message + '】'
         end
         Value['rules'].to_a.collect!{|x|x.to_s.gsub(/(^MATCH.*|^FINAL.*)/, 'MATCH,DIRECT')}
      else
         puts '${LOGTIME} Warning: Because of No Rules Field, Stop Setting BT/P2P DIRECT Rules!'
      end;
   end;
   rescue Exception => e
   puts '${LOGTIME} Error: Set BT/P2P DIRECT Rules Error,【' + e.message + '】'
   end
   begin
   if Value.has_key?('rules') and not Value['rules'].to_a.empty? then
      if Value['rules'].to_a.grep(/(?=.*198.18)(?=.*REJECT)/).empty? then
         ruby_add_index = Value['rules'].index(Value['rules'].grep(/(GEOIP|MATCH|FINAL)/).first)
         ruby_add_index ||= -1
         Value['rules']=Value['rules'].to_a.insert(ruby_add_index,'IP-CIDR,198.18.0.1/16,REJECT,no-resolve')
      end
   else
      Value['rules']=%w(IP-CIDR,198.18.0.1/16,REJECT,no-resolve)
   end;
   rescue Exception => e
   puts '${LOGTIME} Error: Set 198.18.0.1/16 REJECT Rule Error,【' + e.message + '】'
   ensure
   File.open('$4','w') {|f| YAML.dump(Value, f)}
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
      LOG_OUT "Warrning: Multiple Other-Rules-Configurations Enabled, Ignore..."
      return
   fi
   
   config_get "rule_name" "$section" "rule_name" ""
   config_get "GlobalTV" "$section" "GlobalTV" ""
   config_get "AsianTV" "$section" "AsianTV" ""
   config_get "Proxy" "$section" "Proxy" ""
   config_get "Youtube" "$section" "Youtube" ""
   config_get "Bilibili" "$section" "Bilibili" ""
   config_get "Bahamut" "$section" "Bahamut" ""
   config_get "HBO" "$section" "HBO" ""
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
   config_get "Microsoft" "$section" "Microsoft" ""
   config_get "PayPal" "$section" "PayPal" ""
   config_get "Domestic" "$section" "Domestic" ""
   config_get "Others" "$section" "Others" ""
}

if [ "$2" != "0" ]; then
   /usr/share/openclash/yml_groups_name_get.sh
   if [ $? -ne 0 ]; then
      LOG_OUT "Error: Unable To Parse Config File, Please Check And Try Again!"
      exit 0
   fi
   config_load "openclash"
   config_foreach yml_other_rules_get "other_rules" "$6"
   if [ -z "$rule_name" ]; then
      yml_other_set "$1" "$2" "$3" "$4" "$5"
      exit 0
   #判断策略组是否存在
   elif [ "$rule_name" = "ConnersHua_return" ]; then
	    if [ -z "$(grep -F "$Proxy" /tmp/Proxy_Group)" ]\
	 || [ -z "$(grep -F "$Others" /tmp/Proxy_Group)" ];then
         LOG_OUT "Warning: Because of The Different Porxy-Group's Name, Stop Setting The Other Rules!"
         yml_other_set "$1" "$2" "$3" "$4" "$5"
         exit 0
	    fi
   elif [ "$rule_name" = "ConnersHua" ]; then
       if [ -z "$(grep "$GlobalTV" /tmp/Proxy_Group)" ]\
	 || [ -z "$(grep -F "$AsianTV" /tmp/Proxy_Group)" ]\
	 || [ -z "$(grep -F "$Proxy" /tmp/Proxy_Group)" ]\
	 || [ -z "$(grep -F "$Others" /tmp/Proxy_Group)" ]\
	 || [ -z "$(grep -F "$Domestic" /tmp/Proxy_Group)" ]; then
         LOG_OUT "Warning: Because of The Different Porxy-Group's Name, Stop Setting The Other Rules!"
         yml_other_set "$1" "$2" "$3" "$4" "$5"
         exit 0
       fi
   elif [ "$rule_name" = "lhie1" ]; then
       if [ -z "$(grep -F "$GlobalTV" /tmp/Proxy_Group)" ]\
	 || [ -z "$(grep -F "$AsianTV" /tmp/Proxy_Group)" ]\
	 || [ -z "$(grep -F "$Proxy" /tmp/Proxy_Group)" ]\
	 || [ -z "$(grep -F "$Youtube" /tmp/Proxy_Group)" ]\
	 || [ -z "$(grep -F "$Bilibili" /tmp/Proxy_Group)" ]\
	 || [ -z "$(grep -F "$Bahamut" /tmp/Proxy_Group)" ]\
	 || [ -z "$(grep -F "$HBO" /tmp/Proxy_Group)" ]\
	 || [ -z "$(grep -F "$Pornhub" /tmp/Proxy_Group)" ]\
	 || [ -z "$(grep -F "$Apple" /tmp/Proxy_Group)" ]\
	 || [ -z "$(grep -F "$Scholar" /tmp/Proxy_Group)" ]\
	 || [ -z "$(grep -F "$Netflix" /tmp/Proxy_Group)" ]\
	 || [ -z "$(grep -F "$Disney" /tmp/Proxy_Group)" ]\
	 || [ -z "$(grep -F "$Spotify" /tmp/Proxy_Group)" ]\
	 || [ -z "$(grep -F "$Steam" /tmp/Proxy_Group)" ]\
	 || [ -z "$(grep -F "$AdBlock" /tmp/Proxy_Group)" ]\
	 || [ -z "$(grep -F "$Speedtest" /tmp/Proxy_Group)" ]\
   || [ -z "$(grep -F "$Telegram" /tmp/Proxy_Group)" ]\
   || [ -z "$(grep -F "$Microsoft" /tmp/Proxy_Group)" ]\
   || [ -z "$(grep -F "$PayPal" /tmp/Proxy_Group)" ]\
	 || [ -z "$(grep -F "$Others" /tmp/Proxy_Group)" ]\
	 || [ -z "$(grep -F "$Domestic" /tmp/Proxy_Group)" ]; then
         LOG_OUT "Warning: Because of The Different Porxy-Group's Name, Stop Setting The Other Rules!"
         yml_other_set "$1" "$2" "$3" "$4" "$5"
         exit 0
       fi
   fi
   if [ -z "$Proxy" ]; then
      LOG_OUT "Error: Missing Porxy-Group's Name, Stop Setting The Other Rules!"
      yml_other_set "$1" "$2" "$3" "$4" "$5"
      exit 0
   else
       #删除原有的部分，防止冲突
       if [ -n "$(ruby_read "$4" "['script']")" ]; then
          ruby_edit "$4" ".delete('script')"
       fi
       if [ -n "$(ruby_read "$4" "['rules']")" ]; then
          ruby_edit "$4" ".delete('rules')"
       fi
       if [ "$rule_name" = "lhie1" ]; then
       	    ruby -ryaml -E UTF-8 -e "
       	    begin
       	    Value = YAML.load_file('$4');
       	    Value_1 = YAML.load_file('/usr/share/openclash/res/lhie1.yaml');
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
       	    x.to_s.gsub(/,Bilibili,AsianTV$/, ',Bilibili,$Bilibili#d')
       	    .gsub(/,Bahamut,GlobalTV$/, ',Bahamut,$Bahamut#d')
       	    .gsub(/,HBO,GlobalTV$/, ',HBO,$HBO#d')
       	    .gsub(/,Pornhub,GlobalTV$/, ',Pornhub,$Pornhub#d')
       	    .gsub(/,GlobalTV$/, ',$GlobalTV#d')
       	    .gsub(/,AsianTV$/, ',$AsianTV#d')
       	    .gsub(/,Proxy$/, ',$Proxy#d')
       	    .gsub(/,YouTube$/, ',$Youtube#d')
       	    .gsub(/,Apple$/, ',$Apple#d')
       	    .gsub(/,Scholar$/, ',$Scholar#d')
       	    .gsub(/,Netflix$/, ',$Netflix#d')
       	    .gsub(/,Disney$/, ',$Disney#d')
       	    .gsub(/,Spotify$/, ',$Spotify#d')
       	    .gsub(/,Steam$/, ',$Steam#d')
       	    .gsub(/,AdBlock$/, ',$AdBlock#d')
       	    .gsub(/,Speedtest$/, ',$Speedtest#d')
       	    .gsub(/,Telegram$/, ',$Telegram#d')
       	    .gsub(/,Microsoft$/, ',$Microsoft#d')
       	    .to_s.gsub(/,PayPal$/, ',$PayPal#d')
       	    .gsub(/,Domestic$/, ',$Domestic#d')
       	    .gsub(/,Others$/, ',$Others#d')
       	    .gsub(/#d/, '')
       	    };
       	    Value['script']['code'].to_s.gsub!(/\"Bilibili\": \"AsianTV\"/,'\"Bilibili\": \"$Bilibili#d\"')
       	    .gsub!(/\"Bahamut\": \"GlobalTV\"/,'\"Bahamut\": \"$Bahamut#d\"')
       	    .gsub!(/\"HBO\": \"GlobalTV\"/,'\"HBO\": \"$HBO#d\"')
       	    .gsub!(/\"Pornhub\": \"GlobalTV\"/,'\"Pornhub\": \"$Pornhub#d\"')
       	    .gsub!(/: \"GlobalTV\"/,': \"$GlobalTV#d\"')
       	    .gsub!(/: \"AsianTV\"/,': \"$AsianTV#d\"')
       	    .gsub!(/: \"Proxy\"/,': \"$Proxy#d\"')
       	    .gsub!(/: \"YouTube\"/,': \"$Youtube#d\"')
       	    .gsub!(/: \"Apple\"/,': \"$Apple#d\"')
       	    .gsub!(/: \"Scholar\"/,': \"$Scholar#d\"')
       	    .gsub!(/: \"Netflix\"/,': \"$Netflix#d\"')
       	    .gsub!(/: \"Disney\"/,': \"$Disney#d\"')
       	    .gsub!(/: \"Spotify\"/,': \"$Spotify#d\"')
       	    .gsub!(/: \"Steam\"/,': \"$Steam#d\"')
       	    .gsub!(/: \"AdBlock\"/,': \"$AdBlock#d\"')
       	    .gsub!(/: \"Speedtest\"/,': \"$Speedtest#d\"')
       	    .gsub!(/: \"Telegram\"/,': \"$Telegram#d\"')
       	    .gsub!(/: \"Microsoft\"/,': \"$Microsoft#d\"')
       	    .gsub!(/: \"PayPal\"/,': \"$PayPal#d\"')
       	    .gsub!(/: \"Domestic\"/,': \"$Domestic#d\"')
       	    .gsub!(/return \"Domestic\"$/, 'return \"$Domestic#d\"')
       	    .gsub!(/return \"Others\"$/, 'return \"$Others#d\"')
       	    .gsub!(/#d/, '');
       	    File.open('$4','w') {|f| YAML.dump(Value, f)};
       	    rescue Exception => e
       	    puts '${LOGTIME} Error: Set lhie1 Rules Error,【' + e.message + '】'
       	    end" 2>/dev/null >> $LOG_FILE
       elif [ "$rule_name" = "ConnersHua" ]; then
            ruby -ryaml -E UTF-8 -e "
            begin
       	    Value = YAML.load_file('$4');
            Value_1 = YAML.load_file('/usr/share/openclash/res/ConnersHua.yaml');
       	    if Value_1.has_key?('rule-providers') and not Value_1['rule-providers'].to_a.empty? then
       	       if Value.has_key?('rule-providers') and not Value['rule-providers'].to_a.empty? then
                  Value['rule-providers'].merge!(Value_1['rule-providers'])
       	       else
                  Value['rule-providers']=Value_1['rule-providers']
       	       end
       	    end;
       	    Value['rules']=Value_1['rules'];
       	    Value['rules'].to_a.collect!{|x|
       	    x.to_s.gsub(/,Streaming$/, ',$GlobalTV#d')
       	    .gsub(/,StreamingSE$/, ',$AsianTV#d')
       	    .gsub(/(,PROXY$|,IP-Blackhole$)/, ',$Proxy#d')
       	    .gsub(/,China,DIRECT$/, ',China,$Domestic#d')
       	    .gsub(/,ChinaIP,DIRECT$/, ',ChinaIP,$Domestic#d')
       	    .gsub(/,CN,DIRECT$/, ',CN,$Domestic#d')
       	    .gsub(/,MATCH$/, ',$Others#d')
       	    .gsub(/#d/, '')
       	    };
       	    File.open('$4','w') {|f| YAML.dump(Value, f)};
       	    rescue Exception => e
       	    puts '${LOGTIME} Error: Set ConnersHua Rules Error,【' + e.message + '】'
       	    end" 2>/dev/null >> $LOG_FILE
       else
            ruby -ryaml -E UTF-8 -e "
            begin
       	    Value = YAML.load_file('$4');
       	    Value_1 = YAML.load_file('/usr/share/openclash/res/ConnersHua_return.yaml');
       	    Value['rules']=Value_1['rules'];
       	    Value['rules'].to_a.collect!{|x|
       	    x.to_s.gsub(/,PROXY$/, ',$Proxy#d')
       	    .gsub(/MATCH,DIRECT$/, 'MATCH,$Others#d')
       	    .gsub(/#d/, '')
       	    };
       	    File.open('$4','w') {|f| YAML.dump(Value, f)};
       	    rescue Exception => e
       	    puts '${LOGTIME} Error: Set ConnersHua Return Rules Error,【' + e.message + '】'
       	    end" 2>/dev/null >> $LOG_FILE
       fi
   fi
fi

yml_other_set "$1" "$2" "$3" "$4" "$5"
