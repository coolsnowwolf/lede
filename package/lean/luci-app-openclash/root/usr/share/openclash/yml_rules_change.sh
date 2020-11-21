#!/bin/sh
. /lib/functions.sh
. /usr/share/openclash/ruby.sh

/usr/share/openclash/yml_groups_name_get.sh

yml_other_set()
{
   ruby -ryaml -E UTF-8 -e "Value = $4;
   if $3 == 1 then
      if Value.has_key?('rules') and not Value['rules'].to_a.empty? then
         if File::exist?('/etc/openclash/custom/openclash_custom_hosts.list') then
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
         if File::exist?('/etc/openclash/custom/openclash_custom_hosts.list') then
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
   if $7 == 1 and Value.has_key?('rules') then
      ruby_add_index = Value['rules'].index(Value['rules'].grep(/(GEOIP|MATCH|FINAL)/).first)
      ruby_add_index ||= -1
      Value['rules']=Value['rules'].to_a.insert(ruby_add_index,
      'DOMAIN-KEYWORD,tracker,DIRECT',
      'DOMAIN-KEYWORD,announce.php?passkey=,DIRECT',
      'DOMAIN-KEYWORD,torrent,DIRECT',
      'DOMAIN-KEYWORD,peer_id=,DIRECT',
      'DOMAIN-KEYWORD,info_hash,DIRECT',
      'DOMAIN-KEYWORD,get_peers,DIRECT',
      'DOMAIN-KEYWORD,find_node,DIRECT',
      'DOMAIN-KEYWORD,BitTorrent,DIRECT',
      'DOMAIN-KEYWORD,announce_peer,DIRECT'
      )
      Value['rules'].to_a.collect!{|x|x.to_s.gsub(/(^MATCH.*|^FINAL.*)/, 'MATCH,DIRECT')}
   end;
   if Value.has_key?('rules') and Value['rules'].to_a.grep(/(?=.*198.18)(?=.*REJECT)/).empty? then
      ruby_add_index = Value['rules'].index(Value['rules'].grep(/(GEOIP|MATCH|FINAL)/).first)
      ruby_add_index ||= -1
      Value['rules']=Value['rules'].to_a.insert(ruby_add_index,'IP-CIDR,198.18.0.1/16,REJECT,no-resolve')
   end;
   File.open('$8','w') {|f| YAML.dump(Value, f)}
   " 2>/dev/null || ruby -ryaml -E UTF-8 -e "Value = $4; File.open('$8','w') {|f| YAML.dump(Value, f)}"
}

if [ "$2" != 0 ]; then
   #判断策略组是否存在
   GlobalTV=$(uci get openclash.config.GlobalTV 2>/dev/null)
   AsianTV=$(uci get openclash.config.AsianTV 2>/dev/null)
   Proxy=$(uci get openclash.config.Proxy 2>/dev/null)
   Youtube=$(uci get openclash.config.Youtube 2>/dev/null)
   Apple=$(uci get openclash.config.Apple 2>/dev/null)
   Netflix=$(uci get openclash.config.Netflix 2>/dev/null)
   Spotify=$(uci get openclash.config.Spotify 2>/dev/null)
   Steam=$(uci get openclash.config.Steam 2>/dev/null)
   AdBlock=$(uci get openclash.config.AdBlock 2>/dev/null)
   Netease_Music=$(uci get openclash.config.Netease_Music 2>/dev/null)
   Speedtest=$(uci get openclash.config.Speedtest 2>/dev/null)
   Telegram=$(uci get openclash.config.Telegram 2>/dev/null)
   Microsoft=$(uci get openclash.config.Microsoft 2>/dev/null)
   PayPal=$(uci get openclash.config.PayPal 2>/dev/null)
   Domestic=$(uci get openclash.config.Domestic 2>/dev/null)
   Others=$(uci get openclash.config.Others 2>/dev/null)
   if [ "$2" = "ConnersHua_return" ]; then
	    if [ -z "$(grep "$Proxy" /tmp/Proxy_Group)" ]\
	 || [ -z "$(grep "$Others" /tmp/Proxy_Group)" ];then
         echo "${1} Warning: Because of The Different Porxy-Group's Name, Stop Setting The Other Rules!" >>/tmp/openclash.log
         yml_other_set "$1" "$2" "$3" "$4" "$5" "$6" "$7" "$10"
         exit 0
	    fi
   elif [ "$2" = "ConnersHua" ]; then
       if [ -z "$(grep "$GlobalTV" /tmp/Proxy_Group)" ]\
	 || [ -z "$(grep "$AsianTV" /tmp/Proxy_Group)" ]\
	 || [ -z "$(grep "$Proxy" /tmp/Proxy_Group)" ]\
	 || [ -z "$(grep "$Others" /tmp/Proxy_Group)" ]\
	 || [ -z "$(grep "$Domestic" /tmp/Proxy_Group)" ]; then
         echo "${1} Warning: Because of The Different Porxy-Group's Name, Stop Setting The Other Rules!" >>/tmp/openclash.log
         yml_other_set "$1" "$2" "$3" "$4" "$5" "$6" "$7" "$10"
         exit 0
       fi
   elif [ "$2" = "lhie1" ]; then
       if [ -z "$(grep "$GlobalTV" /tmp/Proxy_Group)" ]\
	 || [ -z "$(grep "$AsianTV" /tmp/Proxy_Group)" ]\
	 || [ -z "$(grep "$Proxy" /tmp/Proxy_Group)" ]\
	 || [ -z "$(grep "$Youtube" /tmp/Proxy_Group)" ]\
	 || [ -z "$(grep "$Apple" /tmp/Proxy_Group)" ]\
	 || [ -z "$(grep "$Netflix" /tmp/Proxy_Group)" ]\
	 || [ -z "$(grep "$Spotify" /tmp/Proxy_Group)" ]\
	 || [ -z "$(grep "$Steam" /tmp/Proxy_Group)" ]\
	 || [ -z "$(grep "$AdBlock" /tmp/Proxy_Group)" ]\
	 || [ -z "$(grep "$Speedtest" /tmp/Proxy_Group)" ]\
   || [ -z "$(grep "$Telegram" /tmp/Proxy_Group)" ]\
   || [ -z "$(grep "$Microsoft" /tmp/Proxy_Group)" ]\
   || [ -z "$(grep "$PayPal" /tmp/Proxy_Group)" ]\
	 || [ -z "$(grep "$Others" /tmp/Proxy_Group)" ]\
	 || [ -z "$(grep "$Domestic" /tmp/Proxy_Group)" ]; then
         echo "${1} Warning: Because of The Different Porxy-Group's Name, Stop Setting The Other Rules!" >>/tmp/openclash.log
         yml_other_set "$1" "$2" "$3" "$4" "$5" "$6" "$7" "$10"
         exit 0
       fi
   fi
   if [ "$Proxy" = "读取错误，配置文件异常！" ]; then
      echo "${1} Warning: Can not Get The Porxy-Group's Name, Stop Setting The Other Rules!" >>/tmp/openclash.log
      yml_other_set "$1" "$2" "$3" "$4" "$5" "$6" "$7" "$10"
      exit 0
   else
       #删除原有的部分，防止冲突
       CONFIG_HASH=$4
       if [ -n "$(ruby_read "$CONFIG_HASH" "['script']")" ]; then
          CONFIG_HASH=$(ruby_edit "$CONFIG_HASH" "['script'].clear")
       fi
       if [ -n "$(ruby_read "$CONFIG_HASH" "['rules']")" ]; then
          CONFIG_HASH=$(ruby_edit "$CONFIG_HASH" "['rules'].clear")
       fi
       if [ "$2" = "lhie1" ]; then
       	    CONFIG_HASH=$(ruby -ryaml -E UTF-8 -e "Value = $CONFIG_HASH;
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
       	    x.to_s.gsub(/,GlobalTV$/, ',$GlobalTV#d')
       	    .gsub(/,AsianTV$/, ',$AsianTV#d')
       	    .gsub(/,Proxy$/, ',$Proxy#d')
       	    .gsub(/,YouTube$/, ',$Youtube#d')
       	    .gsub(/,Apple$/, ',$Apple#d')
       	    .gsub(/,Netflix$/, ',$Netflix#d')
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
       	    Value['script']['code'].gsub!(/: \"GlobalTV\"/,': \"$GlobalTV#d\"')
       	    .gsub!(/: \"AsianTV\"/,': \"$AsianTV#d\"')
       	    .gsub!(/: \"Proxy\"/,': \"$Proxy#d\"')
       	    .gsub!(/: \"YouTube\"/,': \"$Youtube#d\"')
       	    .gsub!(/: \"Apple\"/,': \"$Apple#d\"')
       	    .gsub!(/: \"Netflix\"/,': \"$Netflix#d\"')
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
       	    puts Value" 2>/dev/null || echo "$CONFIG_HASH")
       elif [ "$2" = "ConnersHua" ]; then
            CONFIG_HASH=$(ruby -ryaml -E UTF-8 -e "Value = $CONFIG_HASH;
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
       	    puts Value" 2>/dev/null || echo "$CONFIG_HASH")
       else
            CONFIG_HASH=$(ruby -ryaml -E UTF-8 -e "Value = $CONFIG_HASH;
       	    Value_1 = YAML.load_file('/usr/share/openclash/res/ConnersHua_return.yaml');
       	    Value['rules']=Value_1['rules'];
       	    Value['rules'].to_a.collect!{|x|
       	    x.to_s.gsub(/,PROXY$/, ',$Proxy#d')
       	    .gsub(/MATCH,DIRECT$/, 'MATCH,$Others#d')
       	    .gsub(/#d/, '')
       	    };
       	    puts Value" 2>/dev/null || echo "$CONFIG_HASH")
       fi
   fi
   yml_other_set "$1" "$2" "$3" "$CONFIG_HASH" "$5" "$6" "$7" "$10"
else
   yml_other_set "$1" "$2" "$3" "$4" "$5" "$6" "$7" "$10"
fi

