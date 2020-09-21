#!/bin/sh

YML_FILE="$1"

   #proxy-providers
	 [ -z "$(grep "^proxy-providers:" "$YML_FILE")" ] && {
      sed -i "s/^ \{1,\}proxy-providers:/proxy-providers:/g" "$YML_FILE" 2>/dev/null
   }
   [ -z "$(grep "^proxy-providers:" "$YML_FILE")" ] && {
      sed -i "s/^ \{0,\}proxy-provider:/proxy-providers:/g" "$YML_FILE" 2>/dev/null
   }
   #proxy-groups
   [ -z "$(grep "^proxy-groups:" "$YML_FILE")" ] && {
      sed -i "s/^ \{0,\}\'Proxy Group\':/proxy-groups:/g" "$YML_FILE" 2>/dev/null
      sed -i 's/^ \{0,\}\"Proxy Group\":/proxy-groups:/g' "$YML_FILE" 2>/dev/null
      sed -i "s/^ \{1,\}proxy-groups:/proxy-groups:/g" "$YML_FILE" 2>/dev/null
   }
   [ -z "$(grep "^proxy-groups:" "$YML_FILE")" ] && {
      sed -i "s/^ \{0,\}Proxy Group:/proxy-groups:/g" "$YML_FILE" 2>/dev/null
   }
   
   #proxies
   [ -z "$(grep "^Proxy:" "$YML_FILE")" ] && {
      sed -i "s/^proxies:/Proxy:/g" "$YML_FILE" 2>/dev/null
   }
   [ -z "$(grep "^Proxy:" "$YML_FILE")" ] && {
   	  group_len=$(sed -n '/^Proxy Group:/=' "$YML_FILE" 2>/dev/null)
   	  proxies_len=$(sed -n '/proxies:/=' "$YML_FILE" 2>/dev/null |sed -n 1p)
      if [ "$proxies_len" -lt "$group_len" ]; then
         sed -i "${proxies_len}s/ \{0,\}proxies:/Proxy:/" "$YML_FILE" 2>/dev/null
      fi 2>/dev/null
   }
   
   #rules
   [ -z "$(grep "^rules:" "$YML_FILE")" ] && {
      sed -i "s/^ \{1,\}rules:/rules:/g" "$YML_FILE" 2>/dev/null
   }
   [ -z "$(grep "^rules:" "$YML_FILE")" ] && {
      sed -i "s/^ \{0,\}Rule:/rules:/g" "$YML_FILE" 2>/dev/null
   }
   
   #rule-providers:
   [ -z "$(grep "^rule-providers:" "$YML_FILE")" ] && {
      sed -i "s/^ \{1,\}rule-providers:/rule-providers:/g" "$YML_FILE" 2>/dev/null
   }
   
   #script:
   [ -z "$(grep "^script:" "$YML_FILE")" ] && {
      sed -i "s/^ \{1,\}script:/script:/g" "$YML_FILE" 2>/dev/null
   }
