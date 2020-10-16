#!/bin/sh
. /lib/functions.sh

field_cut()
{
   local i lines end_len
   if [ "$4" != "dns" ]; then
      proxy_len=$(sed -n '/^Proxy:/=' "$3" 2>/dev/null)
      provider_len=$(sed -n '/^proxy-providers:/=' "$3" 2>/dev/null)
      group_len=$(sed -n '/^proxy-groups:/=' "$3" 2>/dev/null)
      rule_len=$(sed -n '/^rules:/=' "$3" 2>/dev/null)
      rule_provider_len=$(sed -n '/^rule-providers:/=' "$3" 2>/dev/null)
      script_len=$(sed -n '/^script:/=' "$3" 2>/dev/null)
      nameserver_len=$(sed -n '/^ \{0,\}nameserver:/=' "$3" 2>/dev/null)
      if [ -z "$nameserver_len" ]; then
         fallback_len=$(sed -n '/^ \{0,\}fallback:/=' "$3" 2>/dev/null)
         lines="$general_len $fallback_len $proxy_len $provider_len $group_len $rule_len $rule_provider_len $script_len"
      else
         lines="$general_len $nameserver_len $proxy_len $provider_len $group_len $rule_len $rule_provider_len $script_len"
      fi
   else
      fallback_filter_len=$(sed -n '/^ \{0,\}fallback-filter:/=' "$3" 2>/dev/null)
      cfw_bypass_len=$(sed -n '/^ \{0,\}cfw-bypass:/=' "$3" 2>/dev/null)
      cfw_latency_timeout_len=$(sed -n '/^ \{0,\}cfw-latency-timeout:/=' "$3" 2>/dev/null)
      nameserver_len=$(sed -n '/^ \{0,\}nameserver:/=' "$3" 2>/dev/null)
      if [ -z "$nameserver_len" ]; then
         fallback_len=$(sed -n '/^ \{0,\}fallback:/=' "$3" 2>/dev/null)
         lines="$fallback_len $fallback_filter_len $cfw_bypass_len $cfw_latency_timeout_len"
      else
         lines="$nameserver_len $fallback_filter_len $cfw_bypass_len $cfw_latency_timeout_len"
      fi
      
   fi
   
   for i in $lines; do
      if [ -z "$1" ]; then
         break
      fi
      
      if [ "$1" -ge "$i" ]; then
         continue
      fi
	    
      if [ "$end_len" -gt "$i" ] || [ -z "$end_len" ]; then
	       end_len="$i"
      fi
   done 2>/dev/null
   
   if [ -n "$1" ] && [ -z "$end_len" ]; then
      end_len=$(sed -n '$=' "$3")
   elif [ -n "$end_len" ]; then
      end_len=$(expr "$end_len" - 1)
   fi
      
   if [ "$4" = "yaml_get" ]; then
      sed -n "${1},${end_len}p" "$3" |sed 's/\"//g' 2>/dev/null |sed "s/\'//g" 2>/dev/null |sed 's/\t/ /g' 2>/dev/null > "$2" 2>/dev/null
   elif [ "$4" = "dns" ]; then
   	  sed -n "${1},${end_len}p" "$3" > "$2" 2>/dev/null
   	  sed -i "${1},${end_len}d" "$3" 2>/dev/null
   else
      sed -n "${1},${end_len}p" "$3" > "$2" 2>/dev/null
   fi

}

if [ ! -f "/tmp/yaml_general" ]; then
   #识别general部分
   space_num=$(grep "^ \{0,\}socks-port:" "$3" 2>/dev/null |awk -F ':' '{print $1}' |grep -c " ")
   if [ -z "$space_num" ]; then
      space_num=$(grep "^ \{0,\}allow-lan:" "$3" 2>/dev/null |awk -F ':' '{print $1}' |grep -c " ")
   fi
   cat "/usr/share/openclash/res/default.yaml" |awk '1;/dns:/{exit}' 2>/dev/null |sed '/^ \{0,\}#\|^ \{0,\}-\|^ \{0,\}$/d' 2>/dev/null |awk -F ':' '{print $1}' |while read -r line
   do
      if [ -z "$line" ]; then
         continue
      fi
      
      line_len=$(sed -n "/^ \{0,${space_num}\}${line}:/=" "$3" 2>/dev/null)
      
      if [ -z "$line_len" ] || [ "$(echo $line_len |awk -F ' ' '{print NF}' 2>/dev/null)" -ge 2 ]; then
         continue
      fi
      
      if [ -z "$general_len" ]; then
         general_len=$line_len
         echo $general_len >/tmp/yaml_general
         continue
      fi
      
      if [ "$general_len" -gt "$line_len" ]; then
         general_len=$line_len
         echo $general_len >/tmp/yaml_general
      fi
   done 2>/dev/null
fi

   general_len=$(cat /tmp/yaml_general 2>/dev/null)
   if [ "$1" = "general" ]; then
      field_cut "$general_len" "$2" "$3"
   else
      field_cut "$1" "$2" "$3" "$4"
   fi
   
   dns_hijack_len=$(sed -n '/dns-hijack:/=' "$2" 2>/dev/null)
   if [ -n "$dns_hijack_len" ]; then
      dns_hijack_end_len=$dns_hijack_len
      while ( [ -n "$(echo "$hijack_line" |grep "^ \{0,\}-")" ] || [ -n "$(echo "$hijack_line" |grep "^ \{0,\}$")" ] || [ -z "$hijack_line" ] )
      do
    	   dns_hijack_end_len=$(expr "$dns_hijack_end_len" + 1)
    	   hijack_line=$(sed -n "${dns_hijack_end_len}p" "$2")
      done 2>/dev/null
      dns_hijack_end_len=$(expr "$dns_hijack_end_len" - 1)
      sed -i "${dns_hijack_len},${dns_hijack_end_len}d" "$2" 2>/dev/null
   fi
   sed -i '/^ \{0,\}tun:/,/^ \{0,\}enable:/d' "$2" 2>/dev/null
   sed -i '/^ \{0,\}dns-hijack:/d' "$2" 2>/dev/null
   sed -i '/^ \{0,\}macOS-auto-route:/d' "$2" 2>/dev/null
   sed -i '/^ \{0,\}macOS-auto-detect-interface:/d' "$2" 2>/dev/null
   sed -i '/^ \{0,\}stack:/d' "$2" 2>/dev/null
   sed -i '/^ \{0,\}device-url:/d' "$2" 2>/dev/null
   sed -i '/^ \{0,\}dns-listen:/d' "$2" 2>/dev/null