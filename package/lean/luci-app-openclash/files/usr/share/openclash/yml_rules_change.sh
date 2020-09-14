#!/bin/bash

SCRIPT_FILE="/tmp/yaml_script.yaml"
RULE_PROVIDER_FILE="/tmp/yaml_rule_provider.yaml"
OTHER_RULE_PROVIDER_FILE="/tmp/other_rule_provider.yaml"
OTHER_RULE_FILE="/tmp/other_rule.yaml"
RULE_PROVIDER_BAK_FILE="/tmp/yaml_rule_provider_bak.yaml"
RULE_BAK_FILE="/tmp/yaml_rules_bak.yaml"
SCRIPT_BAK_FILE="/tmp/yaml_script_bak.yaml"
check_def=0

/usr/share/openclash/yml_groups_name_get.sh

yml_other_set()
{
      sed -i "s/'//g" "$4" 2>/dev/null
      sed -i '/^##Custom Rules##/,/^##Custom Rules End##/d' "$4" 2>/dev/null
      sed -i '/^##Custom Rules##/d' "$4" 2>/dev/null
      sed -i '/^##Custom Rules End##/d' "$4" 2>/dev/null
      sed -i '/^##Custom Rules 2##/,/^##Custom Rules 2 End##/d' "$4" 2>/dev/null
      sed -i '/^##Custom Rules 2##/d' "$4" 2>/dev/null
      sed -i '/^##Custom Rules 2 End##/d' "$4" 2>/dev/null
      sed -i '/- DOMAIN-KEYWORD,tracker,DIRECT/d' "$4" 2>/dev/null
      sed -i '/- DOMAIN-KEYWORD,announce.php?passkey=,DIRECT/d' "$4" 2>/dev/null
      sed -i '/- DOMAIN-KEYWORD,torrent,DIRECT/d' "$4" 2>/dev/null
      sed -i '/- DOMAIN-KEYWORD,peer_id=,DIRECT/d' "$4" 2>/dev/null
      sed -i '/- DOMAIN-KEYWORD,info_hash,DIRECT/d' "$4" 2>/dev/null
      sed -i '/- DOMAIN-KEYWORD,get_peers,DIRECT/d' "$4" 2>/dev/null
      sed -i '/- DOMAIN-KEYWORD,find_node,DIRECT/d' "$4" 2>/dev/null
      sed -i '/- DOMAIN-KEYWORD,BitTorrent,DIRECT/d' "$4" 2>/dev/null
      sed -i '/- DOMAIN-KEYWORD,announce_peer,DIRECT/d' "$4" 2>/dev/null
      
      if [ -z "$(grep '^ \{0,\}- IP-CIDR,198.18.0.1/16,REJECT,no-resolve' "$4")" ] && [ "$6" = "fake-ip" ]; then
         if [ ! -z "$(grep "^ \{0,\}- IP-CIDR,198.18.0.1/16" "$4")" ]; then
            sed -i "/^ \{0,\}- IP-CIDR,198.18.0.1\/16/c\- IP-CIDR,198.18.0.1\/16,REJECT,no-resolve" "$4" 2>/dev/null
         else
            sed -i '1,/^ \{0,\}- GEOIP/{/^ \{0,\}- GEOIP/s/^ \{0,\}- GEOIP/- IP-CIDR,198.18.0.1\/16,REJECT,no-resolve\n&/}' "$4" 2>/dev/null
            if [ -z "$(grep '^- IP-CIDR,198.18.0.1/16,REJECT,no-resolve' "$4")" ]; then
               sed -i '1,/^ \{0,\}- MATCH/{/^ \{0,\}- MATCH/s/^ \{0,\}- MATCH/- IP-CIDR,198.18.0.1\/16,REJECT,no-resolve\n&/}' "$4" 2>/dev/null
            fi
         fi
      fi

      if [ "$7" = 1 ]; then
         sed -i '1,/^ \{0,\}- GEOIP/{/^ \{0,\}- GEOIP/s/^ \{0,\}- GEOIP/- DOMAIN-KEYWORD,tracker,DIRECT\n&/}' "$4" 2>/dev/null
         if [ -z "$(grep '^- DOMAIN-KEYWORD,tracker,DIRECT' "$4")" ]; then
            sed -i '1,/^ \{0,\}- MATCH/{/^ \{0,\}- MATCH/s/^ \{0,\}- MATCH/- DOMAIN-KEYWORD,tracker,DIRECT\n&/}' "$4" 2>/dev/null
         fi
         if [ -z "$(grep '^- DOMAIN-KEYWORD,tracker,DIRECT' "$4")" ]; then
            echo "- DOMAIN-KEYWORD,tracker,DIRECT" >> "$4" 2>/dev/null
         fi
         sed -i "/- DOMAIN-KEYWORD,tracker,DIRECT/a\- DOMAIN-KEYWORD,announce.php?passkey=,DIRECT" "$4" 2>/dev/null
         sed -i "/- DOMAIN-KEYWORD,tracker,DIRECT/a\- DOMAIN-KEYWORD,torrent,DIRECT" "$4" 2>/dev/null
         sed -i "/- DOMAIN-KEYWORD,tracker,DIRECT/a\- DOMAIN-KEYWORD,peer_id=,DIRECT" "$4" 2>/dev/null
         sed -i "/- DOMAIN-KEYWORD,tracker,DIRECT/a\- DOMAIN-KEYWORD,info_hash,DIRECT" "$4" 2>/dev/null
         sed -i "/- DOMAIN-KEYWORD,tracker,DIRECT/a\- DOMAIN-KEYWORD,get_peers,DIRECT" "$4" 2>/dev/null
         sed -i "/- DOMAIN-KEYWORD,tracker,DIRECT/a\- DOMAIN-KEYWORD,find_node,DIRECT" "$4" 2>/dev/null
         sed -i "/- DOMAIN-KEYWORD,tracker,DIRECT/a\- DOMAIN-KEYWORD,BitTorrent,DIRECT" "$4" 2>/dev/null
         sed -i "/- DOMAIN-KEYWORD,tracker,DIRECT/a\- DOMAIN-KEYWORD,announce_peer,DIRECT" "$4" 2>/dev/null
         if [ -z "$(grep "###- MATCH," "$4")" ] && [ -z "$(grep "###- FINAL," "$4")" ]; then
            sed -i 's/- MATCH,/###&/' "$4" 2>/dev/null
            echo "- MATCH,DIRECT" >> "$4" 2>/dev/null
         fi
      else
         if [ ! -z "$(grep "###- MATCH," "$4")" ] || [ ! -z "$(grep "###- FINAL," "$4")" ]; then
            sed -i '/^- MATCH,DIRECT/d' "$4" 2>/dev/null
            sed -i "s/###- MATCH,/- MATCH,/" "$4" 2>/dev/null
         fi
      fi

      if [ "$3" = 1 ]; then
         sed -i '/^rules:/a\##Custom Rules End##' "$4" 2>/dev/null
         sed -i '/^rules:/a\##Custom Rules##' "$4" 2>/dev/null
         sed -i '/^##Custom Rules##/r/etc/openclash/custom/openclash_custom_rules.list' "$4" 2>/dev/null
         sed -i '/^ \{0,\}- MATCH,/i\##Custom Rules 2##' "$4" 2>/dev/null
         if [ -z "$(grep '^##Custom Rules 2##' "$4")" ]; then
            echo "##Custom Rules 2##" >> "$4" 2>/dev/null
         fi
         sed -i '/^##Custom Rules 2##/a\##Custom Rules 2 End##' "$4" 2>/dev/null
         sed -i '/^##Custom Rules 2##/r/etc/openclash/custom/openclash_custom_rules_2.list' "$4" 2>/dev/null
      fi
      
      if [ "$5" = 1 ] || [ "$3" = 1 ] || [ "$7" = 1 ] || [ -z "$(grep '- IP-CIDR,198.18.0.1/16,REJECT,no-resolve' "$4")" ]; then
         sed -i "s/^ \{0,\}-/-/" "$4" 2>/dev/null #修改参数空格
         sed -i "s/^\t\{0,\}-/-/" "$4" 2>/dev/null #修改参数tab
      fi
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
         yml_other_set "$1" "$2" "$3" "$4" "$5" "$6" "$7"
         exit 0
	    fi
   elif [ "$2" = "ConnersHua" ]; then
       if [ -z "$(grep "$GlobalTV" /tmp/Proxy_Group)" ]\
	 || [ -z "$(grep "$AsianTV" /tmp/Proxy_Group)" ]\
	 || [ -z "$(grep "$Proxy" /tmp/Proxy_Group)" ]\
	 || [ -z "$(grep "$Others" /tmp/Proxy_Group)" ]\
	 || [ -z "$(grep "$Domestic" /tmp/Proxy_Group)" ]; then
         echo "${1} Warning: Because of The Different Porxy-Group's Name, Stop Setting The Other Rules!" >>/tmp/openclash.log
         yml_other_set "$1" "$2" "$3" "$4" "$5" "$6" "$7"
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
         yml_other_set "$1" "$2" "$3" "$4" "$5" "$6" "$7"
         exit 0
       fi
   fi
   if [ "$Proxy" = "读取错误，配置文件异常！" ]; then
      echo "${1} Warning: Can not Get The Porxy-Group's Name, Stop Setting The Other Rules!" >>/tmp/openclash.log
      yml_other_set "$1" "$2" "$3" "$4" "$5" "$6" "$7"
      exit 0
   else
    rulesource=$(grep '##source:' "$4" |awk -F ':' '{print $2}')
    [ "$rulesource" != "$2" ] && {
       check_def=1
    	}
    
    [ "$check_def" -ne 1 ] && {
    	grep "^##updated$" /etc/openclash/"$2".yaml 1>/dev/null
    	[ "$?" -eq 0 ] && {
    	sed -i '/^##updated$/d' /etc/openclash/"$2".yaml
        check_def=1
        }
    }

    [ "$check_def" -ne 1 ] && {
    GlobalTV_YAML=$(grep '##GlobalTV:' "$4" |awk -F ':' '{print $2}')
    AsianTV_YAML=$(grep '##AsianTV:' "$4" |awk -F ':' '{print $2}')
    Proxy_YAML=$(grep '##Proxy:' "$4" |awk -F ':' '{print $2}')
    Youtube_YAML=$(grep '##Youtube:' "$4" |awk -F ':' '{print $2}')
    Apple_YAML=$(grep '##Apple:' "$4" |awk -F ':' '{print $2}')
    Netflix_YAML=$(grep '##Netflix:' "$4" |awk -F ':' '{print $2}')
    Spotify_YAML=$(grep '##Spotify:' "$4" |awk -F ':' '{print $2}')
    Steam_YAML=$(grep '##Steam:' "$4" |awk -F ':' '{print $2}')
    AdBlock_YAML=$(grep '##AdBlock:' "$4" |awk -F ':' '{print $2}')
    Others_YAML=$(grep '##Others:' "$4" |awk -F ':' '{print $2}')
    Domestic_YAML=$(grep '##Domestic:' "$4" |awk -F ':' '{print $2}')
    Speedtest_YAML=$(grep '##Speedtest:' "$4" |awk -F ':' '{print $2}')
    Telegram_YAML=$(grep '##Telegram:' "$4" |awk -F ':' '{print $2}')
    PayPal_YAML=$(grep '##PayPal:' "$4" |awk -F ':' '{print $2}')
    Microsoft_YAML=$(grep '##Microsoft:' "$4" |awk -F ':' '{print $2}')

    if [ "$2" = "ConnersHua_return" ]; then
	     if [ "$Proxy" != "$Proxy_YAML" ]\
	 || [ "$Others" != "$Others_YAML" ];then
         check_def=1
	     fi
    elif [ "$2" = "ConnersHua" ]; then
       if [ "$GlobalTV" != "$GlobalTV_YAML" ]\
	 || [ "$AsianTV" != "$AsianTV_YAML" ]\
	 || [ "$Proxy" != "$Proxy_YAML" ]\
	 || [ "$Others" != "$Others_YAML" ]\
	 || [ "$Domestic" != "$Domestic_YAML" ]; then
         check_def=1
       fi
    elif [ "$2" = "lhie1" ]; then
       if [ "$GlobalTV" != "$GlobalTV_YAML" ]\
	 || [ "$AsianTV" != "$AsianTV_YAML" ]\
	 || [ "$Proxy" != "$Proxy_YAML" ]\
	 || [ "$Youtube" != "$Youtube_YAML" ]\
	 || [ "$Apple" != "$Apple_YAML" ]\
	 || [ "$Netflix" != "$Netflix_YAML" ]\
	 || [ "$Spotify" != "$Spotify_YAML" ]\
	 || [ "$Steam" != "$Steam_YAML" ]\
	 || [ "$AdBlock" != "$AdBlock_YAML" ]\
	 || [ "$Speedtest" != "$Speedtest_YAML" ]\
   || [ "$Telegram" != "$Telegram_YAML" ]\
   || [ "$Microsoft" != "$Microsoft_YAML" ]\
   || [ "$PayPal" != "$PayPal_YAML" ]\
	 || [ "$Others" != "$Others_YAML" ]\
	 || [ "$Domestic" != "$Domestic_YAML" ]; then
         check_def=1
       fi
    fi
    }

    if [ "$check_def" -eq 1 ]; then
       sed -i '/^rules:/,$d' "$4" 2>/dev/null
       sed -i '/##Other-rule-providers##/,/##Other-rule-providers-end##/d' "$9" 2>/dev/null
       if [ "$2" = "lhie1" ]; then
       	    #删除原有的script部分，防止冲突
       	    rm -rf "$SCRIPT_FILE" 2>/dev/null
       	    cp /etc/openclash/lhie1.yaml "$OTHER_RULE_PROVIDER_FILE"
       	    sed -n '/^ \{0,\}rules:/,$p' "$OTHER_RULE_PROVIDER_FILE" > "$OTHER_RULE_FILE" 2>/dev/null
       	    sed -i '/^ \{0,\}rules:/,$d' "$OTHER_RULE_PROVIDER_FILE" 2>/dev/null
       	    sed -n '/^ \{0,\}script:/,$p' "$OTHER_RULE_PROVIDER_FILE" > "$SCRIPT_FILE" 2>/dev/null
       	    sed -i '/^ \{0,\}script:/,$d' "$OTHER_RULE_PROVIDER_FILE" 2>/dev/null
       	    sed -i "/^ \{0,\}script:/c\script:" "$SCRIPT_FILE" 2>/dev/null
       	    sed -i "/^ \{0,\}rules:/c\rules:" "$OTHER_RULE_FILE" 2>/dev/null
       	    sed -i "/^ \{0,\}rule-providers:/c\rule-providers:" "$OTHER_RULE_PROVIDER_FILE" 2>/dev/null
            echo "##Other-rule-providers-end##" >> "$OTHER_RULE_PROVIDER_FILE"
            if [ -z "$(sed -n '/^ \{0,\}rule-providers:/=' "$9" 2>/dev/null)" ]; then
               sed -i "s/,GlobalTV$/,${GlobalTV}#d/g" "$OTHER_RULE_FILE" 2>/dev/null
               sed -i "s/: \"GlobalTV\"/: \"${GlobalTV}#d\"/g" "$SCRIPT_FILE" 2>/dev/null
               sed -i "s/,GlobalTV,no-resolve$/,${GlobalTV},no-resolve#d/g" "$OTHER_RULE_FILE" 2>/dev/null
               sed -i "/rules:/a\##GlobalTV:${GlobalTV}" "$OTHER_RULE_FILE" 2>/dev/null
               sed -i "s/,AsianTV$/,${AsianTV}#d/g" "$OTHER_RULE_FILE" 2>/dev/null
               sed -i "s/: \"AsianTV\"/: \"${AsianTV}#d\"/g" "$SCRIPT_FILE" 2>/dev/null
               sed -i "s/,AsianTV,no-resolve$/,${AsianTV},no-resolve#d/g" "$OTHER_RULE_FILE" 2>/dev/null
               sed -i "/rules:/a\##AsianTV:${AsianTV}" "$OTHER_RULE_FILE" 2>/dev/null
               sed -i "s/,Proxy$/,${Proxy}#d/g" "$OTHER_RULE_FILE" 2>/dev/null
               sed -i "s/: \"Proxy\"/: \"${Proxy}#d\"/g" "$SCRIPT_FILE" 2>/dev/null
               sed -i "s/,Proxy,no-resolve$/,${Proxy},no-resolve#d/g" "$OTHER_RULE_FILE" 2>/dev/null
               sed -i "/rules:/a\##Proxy:${Proxy}" "$OTHER_RULE_FILE" 2>/dev/null
               sed -i "s/,YouTube$/,${Youtube}#d/g" "$OTHER_RULE_FILE" 2>/dev/null
               sed -i "s/: \"YouTube\"/: \"${Youtube}#d\"/g" "$SCRIPT_FILE" 2>/dev/null
               sed -i "s/,YouTube,no-resolve$/,${Youtube},no-resolve#d/g" "$OTHER_RULE_FILE" 2>/dev/null
               sed -i "/rules:/a\##Youtube:${Youtube}" "$OTHER_RULE_FILE" 2>/dev/null
               sed -i "s/,Apple$/,${Apple}#d/g" "$OTHER_RULE_FILE" 2>/dev/null
               sed -i "s/: \"Apple\"/: \"${Apple}#d\"/g" "$SCRIPT_FILE" 2>/dev/null
               sed -i "s/,Apple,no-resolve$/,${Apple},no-resolve#d/g" "$OTHER_RULE_FILE" 2>/dev/null
               sed -i "/rules:/a\##Apple:${Apple}" "$OTHER_RULE_FILE" 2>/dev/null
               sed -i "s/,Netflix$/,${Netflix}#d/g" "$OTHER_RULE_FILE" 2>/dev/null
               sed -i "s/: \"Netflix\"/: \"${Netflix}#d\"/g" "$SCRIPT_FILE" 2>/dev/null
               sed -i "s/,Netflix,no-resolve$/,${Netflix},no-resolve#d/g" "$OTHER_RULE_FILE" 2>/dev/null
               sed -i "/rules:/a\##Netflix:${Netflix}" "$OTHER_RULE_FILE" 2>/dev/null
               sed -i "s/,Spotify$/,${Spotify}#d/g" "$OTHER_RULE_FILE" 2>/dev/null
               sed -i "s/: \"Spotify\"/: \"${Spotify}#d\"/g" "$SCRIPT_FILE" 2>/dev/null
               sed -i "s/,Spotify,no-resolve$/,${Spotify},no-resolve#d/g" "$OTHER_RULE_FILE" 2>/dev/null
               sed -i "/rules:/a\##Spotify:${Spotify}" "$OTHER_RULE_FILE" 2>/dev/null
               sed -i "s/,Steam$/,${Steam}#d/g" "$OTHER_RULE_FILE" 2>/dev/null
               sed -i "s/: \"Steam\"/: \"${Steam}#d\"/g" "$SCRIPT_FILE" 2>/dev/null
               sed -i "s/,Steam,no-resolve$/,${Steam},no-resolve#d/g" "$OTHER_RULE_FILE" 2>/dev/null
               sed -i "/rules:/a\##Steam:${Steam}" "$OTHER_RULE_FILE" 2>/dev/null
               sed -i "s/,AdBlock$/,${AdBlock}#d/g" "$OTHER_RULE_FILE" 2>/dev/null
               sed -i "s/: \"AdBlock\"/: \"${AdBlock}#d\"/g" "$SCRIPT_FILE" 2>/dev/null
               sed -i "s/,AdBlock,no-resolve$/,${AdBlock},no-resolve#d/g" "$OTHER_RULE_FILE" 2>/dev/null
               sed -i "/rules:/a\##AdBlock:${AdBlock}" "$OTHER_RULE_FILE" 2>/dev/null
               sed -i "s/,Speedtest$/,${Speedtest}#d/g" "$OTHER_RULE_FILE" 2>/dev/null
               sed -i "s/: \"Speedtest\"/: \"${Speedtest}#d\"/g" "$SCRIPT_FILE" 2>/dev/null
               sed -i "s/,Speedtest$/,${Speedtest},no-resolve#d/g" "$OTHER_RULE_FILE" 2>/dev/null
               sed -i "/rules:/a\##Speedtest:${Speedtest}" "$OTHER_RULE_FILE" 2>/dev/null
               sed -i "s/,Telegram$/,${Telegram}#d/g" "$OTHER_RULE_FILE" 2>/dev/null
               sed -i "s/: \"Telegram\"/: \"${Telegram}#d\"/g" "$SCRIPT_FILE" 2>/dev/null
               sed -i "s/,Telegram$/,${Telegram},no-resolve#d/g" "$OTHER_RULE_FILE" 2>/dev/null
               sed -i "/rules:/a\##Telegram:${Telegram}" "$OTHER_RULE_FILE" 2>/dev/null
               sed -i "s/,Microsoft$/,${Microsoft}#d/g" "$OTHER_RULE_FILE" 2>/dev/null
               sed -i "s/: \"Microsoft\"/: \"${Microsoft}#d\"/g" "$SCRIPT_FILE" 2>/dev/null
               sed -i "s/,Microsoft$/,${Microsoft},no-resolve#d/g" "$OTHER_RULE_FILE" 2>/dev/null
               sed -i "/rules:/a\##Microsoft:${Microsoft}" "$OTHER_RULE_FILE" 2>/dev/null
               sed -i "s/,PayPal$/,${PayPal}#d/g" "$OTHER_RULE_FILE" 2>/dev/null
               sed -i "s/: \"PayPal\"/: \"${PayPal}#d\"/g" "$SCRIPT_FILE" 2>/dev/null
               sed -i "s/,PayPal$/,${PayPal},no-resolve#d/g" "$OTHER_RULE_FILE" 2>/dev/null
               sed -i "/rules:/a\##PayPal:${PayPal}" "$OTHER_RULE_FILE" 2>/dev/null
               sed -i "s/,Domestic$/,${Domestic}#d/g" "$OTHER_RULE_FILE" 2>/dev/null
               sed -i "s/: \"Domestic\"/: \"${Domestic}#d\"/g" "$SCRIPT_FILE" 2>/dev/null
               sed -i "s/return \"Domestic\"$/return \"${Domestic}#d\"/g" "$SCRIPT_FILE" 2>/dev/null
               sed -i "s/,Domestic$/,${Domestic},no-resolve#d/g" "$OTHER_RULE_FILE" 2>/dev/null
               sed -i "/rules:/a\##Domestic:${Domestic}" "$OTHER_RULE_FILE" 2>/dev/null
               sed -i "s/,Others$/,${Others}#d/g" "$OTHER_RULE_FILE" 2>/dev/null
               sed -i "s/: \"Others\"/: \"${Others}#d\"/g" "$SCRIPT_FILE" 2>/dev/null
               sed -i "s/return \"Others\"$/return \"${Others}#d\"/g" "$SCRIPT_FILE" 2>/dev/null
               sed -i "s/,Others$/,${Others},no-resolve#d/g" "$OTHER_RULE_FILE" 2>/dev/null
               sed -i "/rules:/a\##Others:${Others}" "$OTHER_RULE_FILE" 2>/dev/null
               sed -i "s/#d//g" "$OTHER_RULE_FILE" 2>/dev/null
               sed -i "s/#d//g" "$SCRIPT_FILE" 2>/dev/null
               sed -i "/^rule-providers:/c\rule-providers: ##Other-rule-providers##" "$OTHER_RULE_PROVIDER_FILE" 2>/dev/null
               cat "$OTHER_RULE_PROVIDER_FILE" >> "$9" 2>/dev/null
            else
            #处理缩进
               sed -i '/^ *$/d' "$9" 2>/dev/null
               sed -i 's/\t/ /g' "$9" 2>/dev/null
               sed -i '/^ \{0,\}#/d' "$9" 2>/dev/null
               sed -i 's/^ \{1,\}/  /g' "$9" 2>/dev/null
               sed -i 's/^ \{1,\}type:/    type:/g' "$9" 2>/dev/null
               sed -i 's/^ \{1,\}behavior:/    behavior:/g' "$9" 2>/dev/null
               sed -i 's/^ \{1,\}path:/    path:/g' "$9" 2>/dev/null
               sed -i 's/^ \{1,\}url:/    url:/g' "$9" 2>/dev/null
               sed -i 's/^ \{1,\}interval:/    interval:/g' "$9" 2>/dev/null
               sed -i 's/^ \{1,\}rule-providers:/rule-providers:/g' "$9" 2>/dev/null
               
               sed -i '/^ \{0,\}rule-providers:/a\##Other-rule-providers##' "$OTHER_RULE_PROVIDER_FILE" 2>/dev/null
               sed -i '/^ \{0,\}rule-providers:/d' "$OTHER_RULE_PROVIDER_FILE" 2>/dev/null
               sed -i '/rule-providers:/r/tmp/other_rule_provider.yaml' "$9" 2>/dev/null
            fi
       elif [ "$2" = "ConnersHua" ]; then
            cp /etc/openclash/ConnersHua.yaml "$OTHER_RULE_PROVIDER_FILE"
            sed -n '/^rules:/,$p' "$OTHER_RULE_PROVIDER_FILE" > "$OTHER_RULE_FILE" 2>/dev/null
            sed -i '/^rules:/,$d' "$OTHER_RULE_PROVIDER_FILE" 2>/dev/null
            sed -i "/^ \{0,\}rule-providers:/c\rule-providers:" "$OTHER_RULE_PROVIDER_FILE" 2>/dev/null
            echo "##Other-rule-providers-end##" >> "$OTHER_RULE_PROVIDER_FILE"
            if [ -z "$(sed -n '/^ \{0,\}rule-providers:/=' "$9" 2>/dev/null)" ]; then
               sed -i "s/,Streaming$/,${GlobalTV}#d/g" "$OTHER_RULE_FILE" 2>/dev/null
               sed -i "s/,Streaming,no-resolve$/,${GlobalTV},no-resolve#d/g" "$OTHER_RULE_FILE" 2>/dev/null
               sed -i "/rules:/a\##GlobalTV:${GlobalTV}" "$OTHER_RULE_FILE" 2>/dev/null
               sed -i "s/,StreamingSE$/,${AsianTV}#d/g" "$OTHER_RULE_FILE" 2>/dev/null
               sed -i "s/,StreamingSE,no-resolve$/,${AsianTV},no-resolve#d/g" "$OTHER_RULE_FILE" 2>/dev/null
               sed -i "/rules:/a\##AsianTV:${AsianTV}" "$OTHER_RULE_FILE" 2>/dev/null
               sed -i "s/,PROXY$/,${Proxy}#d/g" "$OTHER_RULE_FILE" 2>/dev/null
               sed -i "s/,PROXY,no-resolve$/,${Proxy},no-resolve#d/g" "$OTHER_RULE_FILE" 2>/dev/null
               sed -i "s/,IP-Blackhole$/,${Proxy}#d/g" "$OTHER_RULE_FILE" 2>/dev/null
               sed -i "s/,IP-Blackhole,no-resolve$/,${Proxy},no-resolve#d/g" "$OTHER_RULE_FILE" 2>/dev/null
               sed -i "/rules:/a\##Proxy:${Proxy}" "$OTHER_RULE_FILE" 2>/dev/null
               sed -i "s/,China,DIRECT$/,China,${Domestic}#d/g" "$OTHER_RULE_FILE" 2>/dev/null
               sed -i "s/,China,DIRECT,no-resolve$/,China,${Domestic},no-resolve#d/g" "$OTHER_RULE_FILE" 2>/dev/null
               sed -i "s/,ChinaIP,DIRECT$/,ChinaIP,${Domestic}#d/g" "$OTHER_RULE_FILE" 2>/dev/null
               sed -i "s/,ChinaIP,DIRECT,no-resolve$/,ChinaIP,${Domestic},no-resolve#d/g" "$OTHER_RULE_FILE" 2>/dev/null
               sed -i "s/,CN,DIRECT$/,CN,${Domestic}#d/g" "$OTHER_RULE_FILE" 2>/dev/null
               sed -i "s/,CN,DIRECT,no-resolve$/,CN,${Domestic},no-resolve#d/g" "$OTHER_RULE_FILE" 2>/dev/null
               sed -i "/rules:/a\##Domestic:${Domestic}" "$OTHER_RULE_FILE" 2>/dev/null
               sed -i "s/,MATCH$/,${Others}#d/g" "$OTHER_RULE_FILE" 2>/dev/null
               sed -i "s/,MATCH,no-resolve$/,${Others},no-resolve#d/g" "$OTHER_RULE_FILE" 2>/dev/null
               sed -i "/rules:/a\##Others:${Others}" "$OTHER_RULE_FILE" 2>/dev/null
               sed -i "s/#d//g" "$OTHER_RULE_FILE" 2>/dev/null
               sed -i "/^rule-providers:/c\rule-providers: ##Other-rule-providers##" "$OTHER_RULE_PROVIDER_FILE" 2>/dev/null
               cat "$OTHER_RULE_PROVIDER_FILE" >> "$9" 2>/dev/null
            else
            #处理缩进
               sed -i '/^ *$/d' "$9" 2>/dev/null
               sed -i 's/\t/ /g' "$9" 2>/dev/null
               sed -i '/^ \{0,\}#/d' "$9" 2>/dev/null
               sed -i 's/^ \{1,\}/  /g' "$9" 2>/dev/null
               sed -i 's/^ \{1,\}type:/    type:/g' "$9" 2>/dev/null
               sed -i 's/^ \{1,\}behavior:/    behavior:/g' "$9" 2>/dev/null
               sed -i 's/^ \{1,\}path:/    path:/g' "$9" 2>/dev/null
               sed -i 's/^ \{1,\}url:/    url:/g' "$9" 2>/dev/null
               sed -i 's/^ \{1,\}interval:/    interval:/g' "$9" 2>/dev/null
               sed -i 's/^ \{1,\}rule-providers:/rule-providers:/g' "$9" 2>/dev/null
               
               sed -i '/^ \{0,\}rule-providers:/a\##Other-rule-providers##' "$OTHER_RULE_PROVIDER_FILE" 2>/dev/null
               sed -i '/^ \{0,\}rule-providers:/d' "$OTHER_RULE_PROVIDER_FILE" 2>/dev/null
               sed -i '/rule-providers:/r/tmp/other_rule_provider.yaml' "$9" 2>/dev/null
            fi
       else
            cp /etc/openclash/ConnersHua_return.yaml "$OTHER_RULE_FILE"
            sed -i "s/,PROXY$/,${Proxy}#d/g" "$OTHER_RULE_FILE" 2>/dev/null
            sed -i "s/,PROXY,no-resolve$/,${Proxy},no-resolve#d/g" "$OTHER_RULE_FILE" 2>/dev/null
            sed -i "/rules:/a\##Proxy:${Proxy}" "$OTHER_RULE_FILE" 2>/dev/null
            sed -i "s/,DIRECT$/,${Others}#d/g" "$OTHER_RULE_FILE" 2>/dev/null
            sed -i "s/,DIRECT,no-resolve$/,${Others},no-resolve#d/g" "$OTHER_RULE_FILE" 2>/dev/null
            sed -i "/rules:/a\##Others:${Others}" "$OTHER_RULE_FILE" 2>/dev/null
            sed -i "s/#d//g" "$OTHER_RULE_FILE" 2>/dev/null
       fi
       cat "$OTHER_RULE_FILE" >> "$4" 2>/dev/null
       rm -rf /tmp/other_rule* 2>/dev/null
    fi
   fi
elif [ "$2" = 0 ]; then
   [ -f "$8" ] && {
      grep '##source:' "$4" 1>/dev/null
      if [ "$?" -eq "0" ]; then
         cp -f "$RULE_PROVIDER_BAK_FILE" "$RULE_PROVIDER_FILE" 2>/dev/null
         cp -f "SCRIPT_BAK_FILE" "$SCRIPT_FILE" 2>/dev/null
         cp -f "$RULE_BAK_FILE" "$4" 2>/dev/null
      fi
    	}
fi

yml_other_set "$1" "$2" "$3" "$4" "$5" "$6" "$7"