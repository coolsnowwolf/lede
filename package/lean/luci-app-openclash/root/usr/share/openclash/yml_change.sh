#!/bin/sh

    #删除旧hosts配置
    hostlen=$(sed -n '/hosts:/=' "$7" 2>/dev/null)
    dnslen=$(sed -n '/dns:/=' "$7" 2>/dev/null)
    dnsheadlen=$(expr "$dnslen" - 1)
    if [ ! -z "$hostlen" ] && [ "$hostlen" -gt "$dnslen" ]; then
       sed -i '/^ \{0,\}hosts:/,$d' "$7" 2>/dev/null
	  elif [ ! -z "$hostlen" ]; then
       sed -i '/##Custom HOSTS##/,/##Custom HOSTS END##/d' "$7" 2>/dev/null
       if [ -z "$(awk '/^ {0,}hosts:/,/^dns:/{print}' "$7" 2>/dev/null |awk -F ':' '{print $2}' 2>/dev/null)" ]; then
          sed -i "/${hostlen}p/,/${dnsheadlen}p/d" "$7" 2>/dev/null
          sed -i '/^ \{0,\}hosts:/d' "$7" 2>/dev/null
       fi
	  fi
	   
    if [ -z "$(grep "^  enhanced-mode: $2" "$7")" ]; then
       if [ ! -z "$(grep "^ \{0,\}enhanced-mode:" "$7")" ]; then
          sed -i "/^ \{0,\}enhanced-mode:/c\  enhanced-mode: ${2}" "$7"
       else
          sed -i "/^dns:/a\  enhanced-mode: ${2}" "$7"
       fi
    fi
    
    if [ "$2" = "fake-ip" ]; then
       if [ -z "$(grep "^ \{0,\}fake-ip-range: 198.18.0.1/16" "$7")" ]; then
          if [ ! -z "$(grep "^ \{0,\}fake-ip-range:" "$7")" ]; then
             sed -i "/^ \{0,\}fake-ip-range:/c\  fake-ip-range: 198.18.0.1/16" "$7"
          else
             sed -i "/^ \{0,\}enhanced-mode:/a\  fake-ip-range: 198.18.0.1/16" "$7"
          fi
       fi
    else
       sed -i '/^ \{0,\}fake-ip-range:/d' "$7" 2>/dev/null
    fi
    
    sed -i '/##Custom DNS##/d' "$7" 2>/dev/null


    if [ -z "$(grep "^redir-port: $6" "$7")" ]; then
       if [ ! -z "$(grep "^ \{0,\}redir-port:" "$7")" ]; then
          sed -i "/^ \{0,\}redir-port:/c\redir-port: ${6}" "$7"
       else
          sed -i "/^dns:/i\redir-port: ${6}" "$7"
       fi
    fi
    
    if [ -z "$(grep "^port: $9" "$7")" ]; then
       if [ ! -z "$(grep "^ \{0,\}port:" "$7")" ]; then
          sed -i "/^ \{0,\}port:/c\port: ${9}" "$7"
       else
          sed -i "/^dns:/i\port: ${9}" "$7"
       fi
    fi
    
    if [ -z "$(grep "^socks-port: $10" "$7")" ]; then
       if [ ! -z "$(grep "^ \{0,\}socks-port:" "$7")" ]; then
          sed -i "/^ \{0,\}socks-port:/c\socks-port: ${10}" "$7"
       else
          sed -i "/^dns:/i\socks-port: ${10}" "$7"
       fi
    fi
    
    if [ -z "$(grep "^mode: $13" "$7")" ]; then
       if [ ! -z "$(grep "^ \{0,\}mode:" "$7")" ]; then
          sed -i "/^ \{0,\}mode:/c\mode: ${13}" "$7"
       else
          sed -i "/^dns:/i\mode: ${13}" "$7"
       fi
    fi
    
    if [ -z "$(grep "^log-level: $12" "$7")" ]; then
       if [ ! -z "$(grep "^ \{0,\}log-level:" "$7")" ]; then
          sed -i "/^ \{0,\}log-level:/c\log-level: ${12}" "$7"
       else
          sed -i "/^dns:/i\log-level: ${12}" "$7"
       fi
    fi
    
    if [ "$14" -ne 1 ]; then
       controller_address="0.0.0.0"
       bind_address="*"
    else
       controller_address=$11
       bind_address=$11
    fi
    
    if [ -z "$(grep "^external-controller: $controller_address:$5" "$7")" ]; then
       if [ ! -z "$(grep "^ \{0,\}external-controller:" "$7")" ]; then
          sed -i "/^ \{0,\}external-controller:/c\external-controller: ${controller_address}:${5}" "$7"
       else
          sed -i "/^dns:/i\external-controller: ${controller_address}:${5}" "$7"
       fi
       uci set openclash.config.config_reload=0
    fi
    
    if [ -z "$(grep "^secret: \"$4\"" "$7")" ]; then
       if [ ! -z "$(grep "^ \{0,\}secret:" "$7")" ]; then
          sed -i "/^ \{0,\}secret:/c\secret: \"${4}\"" "$7"
       else
          sed -i "/^dns:/i\secret: \"${4}\"" "$7"
       fi
       uci set openclash.config.config_reload=0
    fi
    
    if [ -z "$(grep "^ \{0,\}device-url:" "$7")" ] && [ "$15" -eq 2 ]; then
       uci set openclash.config.config_reload=0
    elif [ -z "$(grep "^ \{0,\}tun:" "$7")" ] && [ -n "$15" ]; then
       uci set openclash.config.config_reload=0
    elif [ -n "$(grep "^ \{0,\}tun:" "$7")" ] && [ -z "$15" ]; then
       uci set openclash.config.config_reload=0
    elif [ -n "$(grep "^ \{0,\}device-url:" "$7")" ] && [ "$15" -eq 1 ]; then
       uci set openclash.config.config_reload=0
    elif [ -n "$(grep "^ \{0,\}device-url:" "$7")" ] && [ "$15" -eq 3 ]; then
       uci set openclash.config.config_reload=0
    fi
    
    uci commit openclash

    if [ -z "$(grep "^   enable: true" "$7")" ]; then
       if [ ! -z "$(grep "^ \{0,\}enable:" "$7")" ]; then
          sed -i "/^ \{0,\}enable:/c\  enable: true" "$7"
       else
          sed -i "/^dns:/a\  enable: true" "$7"
       fi
    fi
    
    if [ -z "$(grep "^allow-lan: true" "$7")" ]; then
       if [ ! -z "$(grep "^ \{0,\}allow-lan:" "$7")" ]; then
          sed -i "/^ \{0,\}allow-lan:/c\allow-lan: true" "$7"
       else
          sed -i "/^dns:/i\allow-lan: true" "$7"
       fi
    fi
    
    sed -i '/bind-address:/d' "$7" 2>/dev/null
    sed -i "/^allow-lan:/a\bind-address: \"${bind_address}\"" "$7"
    
    if [ -z "$(grep '^external-ui: "/usr/share/openclash/dashboard"' "$7")" ]; then
       if [ ! -z "$(grep "^ \{0,\}external-ui:" "$7")" ]; then
          sed -i '/^ \{0,\}external-ui:/c\external-ui: "/usr/share/openclash/dashboard"' "$7"
       else
          sed -i '/^dns:/i\external-ui: "/usr/share/openclash/dashboard"' "$7"
       fi
    fi

    if [ "$8" -eq 1 ]; then
       sed -i '/^ \{0,\}ipv6:/d' "$7" 2>/dev/null
       sed -i "/^ \{0,\}enable: true/a\  ipv6: true" "$7"
       sed -i "/^ \{0,\}mode:/i\ipv6: true" "$7"
    else
       sed -i '/^ \{0,\}ipv6:/d' "$7" 2>/dev/null
       sed -i "/^ \{0,\}enable: true/a\  ipv6: false" "$7"
       sed -i "/^ \{0,\}mode:/i\ipv6: false" "$7"
    fi

#TUN
    if [ "$15" -eq 1 ] || [ "$15" -eq 3 ]; then
       sed -i "/^dns:/i\tun:" "$7"
       sed -i "/^dns:/i\  enable: true" "$7"
       if [ -n "$16" ]; then
          sed -i "/^dns:/i\  stack: ${16}" "$7"
       else
          sed -i "/^dns:/i\  stack: system" "$7"
       fi
       sed -i "/^dns:/i\  dns-hijack:" "$7"
#       sed -i "/^dns:/i\  - 8.8.8.8:53" "$7"
       sed -i "/^dns:/i\  - tcp://8.8.8.8:53" "$7"
#       sed -i "/^dns:/i\  - 8.8.4.4:53" "$7"
       sed -i "/^dns:/i\  - tcp://8.8.4.4:53" "$7"
    elif [ "$15" -eq 2 ]; then
       sed -i "/^dns:/i\tun:" "$7"
       sed -i "/^dns:/i\  enable: true" "$7"
       sed -i "/^dns:/i\  device-url: dev://clash0" "$7"
       sed -i "/^dns:/i\  dns-listen: 0.0.0.0:53" "$7"
    fi

#添加自定义Hosts设置
	   
    if [ "$2" = "redir-host" ]; then
	     if [ -z "$(grep "^ \{0,\}hosts:" $7)" ]; then
	        sed -i '/^dns:/i\hosts:' "$7" 2>/dev/null
   	   else
	        if [ ! -z "$(grep "^ \{1,\}hosts:" $7)" ]; then
	           sed -i "/^ \{0,\}hosts:/c\hosts:" "$7"
	        fi
	     fi
	     if [ -z "$(grep "^ \{0,\}use-hosts:" $7)" ]; then
	        sed -i "/^dns:/a\  use-hosts: true" "$7"
   	   else
	        if [ ! -z "$(grep "^ \{0,\}use-hosts:" $7)" ]; then
	           sed -i "/^ \{0,\}use-hosts:/c\  use-hosts: true" "$7"
	        fi
	     fi
	     sed -i '/^hosts:/a\##Custom HOSTS END##' "$7" 2>/dev/null
	     sed -i '/^hosts:/a\##Custom HOSTS##' "$7" 2>/dev/null
	     sed -i '/##Custom HOSTS##/r/etc/openclash/custom/openclash_custom_hosts.list' "$7" 2>/dev/null
	     sed -i "/^hosts:/,/^dns:/ {s/^ \{0,\}'/  '/}" "$7" 2>/dev/null #修改参数空格
	  fi
	  
	  sed -i "s/^ \{0,\}- /  - /" "$7" 2>/dev/null
	  if [ ! -z "$(grep "^ \{0,\}default-nameserver:" "$7")" ]; then
       sed -i "/^ \{0,\}default-nameserver:/c\  default-nameserver:" "$7"
    fi
    
#fake-ip-filter
    sed -i '/##Custom fake-ip-filter##/,/##Custom fake-ip-filter END##/d' "$7" 2>/dev/null
	  if [ "$2" = "fake-ip" ]; then
      if [ ! -f "/etc/openclash/fake_filter.list" ] || [ ! -z "$(grep "config servers" /etc/config/openclash)" ]; then
         /usr/share/openclash/openclash_fake_filter.sh
      fi
      if [ -s "/etc/openclash/servers_fake_filter.conf" ]; then
         mkdir -p /tmp/dnsmasq.d
         ln -s /etc/openclash/servers_fake_filter.conf /tmp/dnsmasq.d/dnsmasq_openclash.conf
      fi
      if [ -s "/etc/openclash/fake_filter.list" ]; then
      	if [ ! -z "$(grep "^ \{0,\}fake-ip-filter:" "$7")" ]; then
      	   sed -i "/^ \{0,\}fake-ip-filter:/c\  fake-ip-filter:" "$7"
      	   sed -i '/^ \{0,\}fake-ip-filter:/r/etc/openclash/fake_filter.list' "$7" 2>/dev/null
      	else
      	   echo "  fake-ip-filter:" >> "$7"
      	   sed -i '/^ \{0,\}fake-ip-filter:/r/etc/openclash/fake_filter.list' "$7" 2>/dev/null
        fi
      fi
   fi