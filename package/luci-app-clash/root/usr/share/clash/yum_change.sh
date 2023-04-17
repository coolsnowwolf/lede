#!/bin/bash /etc/rc.common
. /lib/functions.sh 

		lang=$(uci get luci.main.lang 2>/dev/null)
		REAL_LOG="/usr/share/clash/clash_real.txt"
		if [ $lang == "en" ] || [ $lang == "auto" ];then
				echo "Setting DNS" >$REAL_LOG   
		elif [ $lang == "zh_cn" ];then
				 echo "设定DNS" >$REAL_LOG
		fi

		mode=$(uci get clash.config.mode 2>/dev/null)
		p_mode=$(uci get clash.config.p_mode 2>/dev/null)
		da_password=$(uci get clash.config.dash_pass 2>/dev/null)
		redir_port=$(uci get clash.config.redir_port 2>/dev/null)
		http_port=$(uci get clash.config.http_port 2>/dev/null)
		socks_port=$(uci get clash.config.socks_port 2>/dev/null)
		dash_port=$(uci get clash.config.dash_port 2>/dev/null)
		bind_addr=$(uci get clash.config.bind_addr 2>/dev/null)
		allow_lan=$(uci get clash.config.allow_lan 2>/dev/null)
		log_level=$(uci get clash.config.level 2>/dev/null)
		CONFIG_START="/tmp/dns.yaml"
		enhanced_mode=$(uci get clash.config.enhanced_mode 2>/dev/null)
		mixed_port=$(uci get clash.config.mixed_port 2>/dev/null)
		enable_ipv6=$(uci get clash.config.enable_ipv6 2>/dev/null)
		
		core=$(uci get clash.config.core 2>/dev/null)
		interf_name=$(uci get clash.config.interf_name 2>/dev/null)
		tun_mode=$(uci get clash.config.tun_mode 2>/dev/null)
		stack=$(uci get clash.config.stack 2>/dev/null)
		listen_port=$(uci get clash.config.listen_port 2>/dev/null)	
		TEMP_FILE="/tmp/clashdns.yaml"
		interf=$(uci get clash.config.interf 2>/dev/null)
		CONFIG_YAML="/etc/clash/config.yaml"

		rm -rf $TEMP_FILE 2>/dev/null
		
		echo " ">>/tmp/dns.yaml 2>/dev/null
		sed -i "1i\#****CLASH-CONFIG-START****#" $CONFIG_START 2>/dev/null
		sed -i "2i\port: ${http_port}" $CONFIG_START 2>/dev/null
		sed -i "/port: ${http_port}/a\socks-port: ${socks_port}" $CONFIG_START 2>/dev/null 
		sed -i "/socks-port: ${socks_port}/a\redir-port: ${redir_port}" $CONFIG_START 2>/dev/null 
		sed -i "/redir-port: ${redir_port}/a\mixed-port: ${mixed_port}" $CONFIG_START 2>/dev/null 
		sed -i "/mixed-port: ${mixed_port}/a\ipv6: ${enable_ipv6}" $CONFIG_START 2>/dev/null
		sed -i "/ipv6: ${enable_ipv6}/a\allow-lan: ${allow_lan}" $CONFIG_START 2>/dev/null
		if [ $allow_lan == "true" ];  then
		sed -i "/allow-lan: ${allow_lan}/a\bind-address: \"${bind_addr}\"" $CONFIG_START 2>/dev/null 
		sed -i "/bind-address: \"${bind_addr}\"/a\mode: ${p_mode}" $CONFIG_START 2>/dev/null
		sed -i "/mode: ${p_mode}/a\log-level: ${log_level}" $CONFIG_START 2>/dev/null 
		sed -i "/log-level: ${log_level}/a\external-controller: 0.0.0.0:${dash_port}" $CONFIG_START 2>/dev/null 
		sed -i "/external-controller: 0.0.0.0:${dash_port}/a\secret: \"${da_password}\"" $CONFIG_START 2>/dev/null 
		sed -i "/secret: \"${da_password}\"/a\external-ui: \"./dashboard\"" $CONFIG_START 2>/dev/null 
		sed -i -e "\$a " $CONFIG_START 2>/dev/null
		else
		sed -i "/allow-lan: ${allow_lan}/a\mode: Rule" $CONFIG_START 2>/dev/null
		sed -i "/mode: Rule/a\log-level: ${log_level}" $CONFIG_START 2>/dev/null 
		sed -i "/log-level: ${log_level}/a\external-controller: 0.0.0.0:${dash_port}" $CONFIG_START 2>/dev/null 
		sed -i "/external-controller: 0.0.0.0:${dash_port}/a\secret: \"${da_password}\"" $CONFIG_START 2>/dev/null 
		sed -i "/secret: \"${da_password}\"/a\external-ui: \"./dashboard\"" $CONFIG_START 2>/dev/null 
		sed -i -e "\$a " $CONFIG_START 2>/dev/null
		fi

cat $CONFIG_START >> $TEMP_FILE 2>/dev/null
if [ "$interf" -eq 1 ] && [ ! -z "$interf_name" ] ;then
cat >> "/tmp/interf_name.yaml" <<-EOF
interface-name: ${interf_name} 
EOF

cat /tmp/interf_name.yaml >> $TEMP_FILE 2>/dev/null
sed -i -e "\$a " $TEMP_FILE 2>/dev/null

fi

authentication_set()
{
   local section="$1"
   config_get "username" "$section" "username" ""
   config_get "password" "$section" "password" ""
   config_get_bool "enabled" "$section" "enabled" "1"

    if [ "$enabled" = "0" ]; then
      return
    fi

	echo "   - \"$username:$password\"" >>/tmp/authentication.yaml

	   
}
   config_load "clash"
   config_foreach authentication_set "authentication"
   
if [ -f /tmp/authentication.yaml ];then
sed -i "1i\authentication:" /tmp/authentication.yaml 
fi

cat /tmp/authentication.yaml >> $TEMP_FILE 2>/dev/null
sed -i -e "\$a " $TEMP_FILE 2>/dev/null



if [ "${tun_mode}" -eq 1 ];then

if [ "${core}" -eq 4 ] || [ "${core}" -eq 3 ];then

cat >> "/tmp/tun.yaml" <<-EOF
tun:
  enable: true 
  stack: ${stack}  
EOF


if [ $core -eq 3 ];then

cat >> "/tmp/tun.yaml" <<-EOF
  device-url: dev://utun
  dns-listen: 0.0.0.0:${listen_port}   
EOF
fi

cat /tmp/tun.yaml >> $TEMP_FILE 2>/dev/null




dnshijack_set()
{
   local section="$1"
   config_get "type" "$section" "type" ""
   config_get "port" "$section" "port" "" 
   config_get "ip" "$section" "ip" ""   
   config_get_bool "enabled" "$section" "enabled" "1"

    if [ "$enabled" = "0" ]; then
      return
    fi


	if [ "$type" == "none" ] && [ ! -z "$port" ]; then
			echo "   - $ip:$port">>/tmp/dnshijack.yaml
	elif [ "$type" == "none" ] && [ -z "$port" ]; then
			echo "   - $ip">>/tmp/dnshijack.yaml
	elif [ -z "$port" ]; then
			echo "   - $type$ip">>/tmp/dnshijack.yaml
	else
			echo "   - $type$ip:$port">>/tmp/dnshijack.yaml
	fi
	
	
}
   config_load "clash"
   config_foreach dnshijack_set "dnshijack"
   
if [ -f /tmp/dnshijack.yaml ];then
sed -i "1i\  dns-hijack:" /tmp/dnshijack.yaml 
fi

cat /tmp/dnshijack.yaml >> $TEMP_FILE 2>/dev/null
sed -i -e "\$a " $TEMP_FILE 2>/dev/null		
		
fi
fi

hosts_set()
{
   local section="$1"
   config_get "address" "$section" "address" ""
   config_get "ip" "$section" "ip" ""
   config_get_bool "enabled" "$section" "enabled" "1"

    if [ "$enabled" = "0" ]; then
      return
    fi

	echo "  '$address': '$ip'" >>/tmp/hosts.yaml

	   
}
if [ "$enhanced_mode" == "redir-host" ];then
   config_load "clash"
   config_foreach hosts_set "hosts"
fi
   
if [ -f /tmp/hosts.yaml ];then
sed -i "1i\hosts:" /tmp/hosts.yaml 
fi
cat /tmp/hosts.yaml >> $TEMP_FILE 2>/dev/null
sed -i -e "\$a " $TEMP_FILE 2>/dev/null

sleep 1

enable_dns=$(uci get clash.config.enable_dns 2>/dev/null) 

if [ "$enable_dns" -eq 1 ];then


cat >> "/tmp/enable_dns.yaml" <<-EOF
dns:
  enable: true
  listen: 0.0.0.0:${listen_port}   
EOF


if [ "$enable_ipv6" == "true" ];then

cat >> "/tmp/enable_dns.yaml" <<-EOF
  ipv6: true
EOF

fi

cat /tmp/enable_dns.yaml >> $TEMP_FILE 2>/dev/null
	
default_nameserver=$(uci get clash.config.default_nameserver 2>/dev/null)		
for list in $default_nameserver; do 
echo "   - $list">>/tmp/default_nameserver.yaml
done

if [ -f /tmp/default_nameserver.yaml ];then
sed -i "1i\  default-nameserver:" /tmp/default_nameserver.yaml
fi


	
cat >> "/tmp/default_nameserver.yaml" <<-EOF
  enhanced-mode: $enhanced_mode
EOF
cat /tmp/default_nameserver.yaml >> $TEMP_FILE 2>/dev/null


if [ "$enhanced_mode" == "fake-ip" ];then

fake_ip_range=$(uci get clash.config.fake_ip_range 2>/dev/null)
cat >> "/tmp/fake_ip_range.yaml" <<-EOF
  fake-ip-range: $fake_ip_range
EOF

cat /tmp/fake_ip_range.yaml >> $TEMP_FILE 2>/dev/null
fi

if [ "$enhanced_mode" == "fake-ip" ];then

fake_ip_filter=$(uci get clash.config.fake_ip_filter 2>/dev/null)		
for list in $fake_ip_filter; do 
echo "   - '$list'">>/tmp/fake_ip_filter.yaml
done

if [ -f /tmp/fake_ip_filter.yaml ];then
sed -i "1i\  fake-ip-filter:" /tmp/fake_ip_filter.yaml
fi

cat /tmp/fake_ip_filter.yaml >> $TEMP_FILE 2>/dev/null
fi
	
dnsservers_set()
{
   local section="$1"
   config_get "ser_address" "$section" "ser_address" ""
   config_get "protocol" "$section" "protocol" ""
   config_get "ser_type" "$section" "ser_type" ""
   config_get_bool "enabled" "$section" "enabled" "1"
   config_get "ser_port" "$section" "ser_port" ""

   if [ "$enabled" = "0" ]; then
      return
   fi
   
   if [ -z "$ser_type" ]; then
      return
   fi
   
   

	
    if [ "$ser_type" == "nameserver" ]; then
		if [ "$protocol" == "none" ] && [ ! -z "$ser_port" ]; then
				echo "   - $ser_address:$ser_port" >>/tmp/nameservers.yaml
		elif [ "$protocol" == "none" ] && [ -z "$ser_port" ]; then
				echo "   - $ser_address" >>/tmp/nameservers.yaml
		elif [ -z "$ser_port" ]; then
				echo "   - $protocol$ser_address" >>/tmp/nameservers.yaml
		else
				echo "   - $protocol$ser_address:$ser_port" >>/tmp/nameservers.yaml
		fi
    elif [ "$ser_type" == "fallback" ]; then
		if [ "$protocol" == "none" ] && [ ! -z "$ser_port" ]; then
				echo "   - $ser_address:$ser_port" >>/tmp/fallback.yaml
		elif [ "$protocol" == "none" ] && [ -z "$ser_port" ]; then
				echo "   - $ser_address" >>/tmp/fallback.yaml
		elif [ -z "$ser_port" ]; then
				echo "   - $protocol$ser_address" >>/tmp/fallback.yaml
		else
				echo "   - $protocol$ser_address:$ser_port" >>/tmp/fallback.yaml
		fi	  
   fi
	
}
   config_load "clash"
   config_foreach dnsservers_set "dnsservers"
   
if [ -f /tmp/nameservers.yaml ];then
sed -i "1i\  nameserver:" /tmp/nameservers.yaml 
fi
cat /tmp/nameservers.yaml >> $TEMP_FILE 2>/dev/null

if [ -f /tmp/fallback.yaml ];then
sed -i "1i\  fallback:" /tmp/fallback.yaml 

cat >> "/tmp/fallback.yaml" <<-EOF
  fallback-filter:
   geoip: true
   ipcidr:
    - 240.0.0.0/4
EOF

sed -i -e "\$a " /tmp/fallback.yaml 2>/dev/null
fi
cat /tmp/fallback.yaml >> $TEMP_FILE 2>/dev/null

fi

rm -rf /tmp/tun.yaml /tmp/enable_dns.yaml /tmp/fallback.yaml /tmp/nameservers.yaml /tmp/fake_ip_filter.yaml /tmp/default_nameserver.yaml /tmp/hosts.yaml /tmp/authentication.yaml /tmp/dnshijack.yaml /tmp/fake_ip_range.yaml /tmp/dns.yaml /tmp/interf_name.yaml
		
	
		if [ "${enable_dns}" == "0" ];then
		
			if [ ! -z "$(grep "^dns:" "$CONFIG_YAML")" ]; then
				sed -i "/dns:/i\#clash-openwrt" $CONFIG_YAML 2>/dev/null
				sed -i "/#clash-openwrt/a\#=============" $CONFIG_YAML 2>/dev/null
				sed -i '1,/#clash-openwrt/d' $CONFIG_YAML 2>/dev/null
				
				sed -i '/#=============/ d' $CONFIG_YAML 2>/dev/null
			fi
			cat $CONFIG_YAML >> $TEMP_FILE 2>/dev/null
			mv $TEMP_FILE $CONFIG_YAML 2>/dev/null
			
		elif [ "${enable_dns}" == "1" ];then
		

			if [ ! -z "$(grep "^proxies:" "$CONFIG_YAML")" ]; then
			  sed -i "/^proxies:/i\#clash-openwrt" $CONFIG_YAML 2>/dev/null
			elif [ ! -z "$(grep "^proxy-providers:" "$CONFIG_YAML")" ]; then
			  sed -i "/proxy-providers:/i\#clash-openwrt" $CONFIG_YAML 2>/dev/null
			fi
		
			sed -i "/#clash-openwrt/a\#=============" $CONFIG_YAML 2>/dev/null
			sed -i "/#=============/a\ " $CONFIG_YAML 2>/dev/null
			sed -i '1,/#clash-openwrt/d' $CONFIG_YAML 2>/dev/null
			
			mv /etc/clash/config.yaml /etc/clash/dns.yaml
			cat $TEMP_FILE /etc/clash/dns.yaml > $CONFIG_YAML 2>/dev/null
			rm -rf /etc/clash/dns.yaml
			sed -i '/#=============/ d' $CONFIG_YAML 2>/dev/null
		fi
		rm -rf  $TEMP_FILE 2>/dev/null
		
add_address(){
	servers_get()
	{
	   local section="$1"
	   config_get "server" "$section" "server" ""
	   echo "$server" >>/tmp/server.conf
	}
	config_load clash
	config_foreach servers_get "servers"

	count=$(grep -c '' /tmp/server.conf)
	count_num=1
	while [[ $count_num -le $count ]]
	do
	line=$(sed -n "$count_num"p /tmp/server.conf)
	check_addr=$(grep -F "$line" "/usr/share/clash/server.list")
	if [ -z $check_addr ];then
	echo $line >>/usr/share/clashbackup/address.list
	fi	
	count_num=$(( $count_num + 1))	
	done

	sed -i "1i\#START" /usr/share/clashbackup/address.list 2>/dev/null
	sed -i -e "\$a#END" /usr/share/clashbackup/address.list 2>/dev/null
			
	cat /usr/share/clashbackup/address.list /usr/share/clash/server.list > /usr/share/clash/me.list
	rm -rf /usr/share/clash/server.list 
	mv /usr/share/clash/me.list /usr/share/clash/server.list
	chmod 755 /usr/share/clash/server.list
	rm -rf /tmp/server.conf /usr/share/clashbackup/address.list >/dev/null 2>&1
}


		#fake_ip=$(egrep '^ {0,}enhanced-mode' /etc/clash/config.yaml |grep enhanced-mode: |awk -F ': ' '{print $2}')
		fake_ip=$(uci get clash.config.enhanced_mode 2>/dev/null)

		if [ "${fake_ip}" == "fake-ip" ];then
		
		add_address >/dev/null 2>&1
		wait
		CUSTOM_FILE="/usr/share/clash/server.list"
		FAKE_FILTER_FILE="/usr/share/clash/fake_filter.list"
		num=$(grep -c '' /usr/share/clash/server.list 2>/dev/null)

		rm -rf "$FAKE_FILTER_FILE" 2>/dev/null

			if [ -s "$CUSTOM_FILE" ]; then

				count_num=1
				while [[ $count_num -le $num ]]
				do 
				line=$(sed -n "$count_num"p /usr/share/clash/server.list)
				if [ -z "$(echo $line |grep '^ \{0,\}#' 2>/dev/null)" ]; then	
					 echo "   - '$line'" >> "$FAKE_FILTER_FILE"
				fi
				count_num=$(( $count_num + 1))	
				done	  
		  
		 
			fi


			if [ $lang == "en" ] || [ $lang == "auto" ];then
				echo "Setting Up Fake-IP Filter" >$REAL_LOG 
			elif [ $lang == "zh_cn" ];then
				 echo "正在设置Fake-IP黑名单" >$REAL_LOG
			fi	

			sed -i '/fake-ip-filter:/d' "/etc/clash/config.yaml" 2>/dev/null
			if [ ! -z "$(egrep '^ {0,}fake-ip-range:' "/etc/clash/config.yaml")" ];then	
				sed -i '/fake-ip-range/a\  fake-ip-filter:' /etc/clash/config.yaml 2>/dev/null
				sed -i '/fake-ip-filter:/r/usr/share/clash/fake_filter.list' "/etc/clash/config.yaml" 2>/dev/null	
			elif [ ! -z "$(egrep '^ {0,}enhanced-mode:' "/etc/clash/config.yaml")" ];then
				sed -i '/^enhanced-mode:/a\  fake-ip-filter:' /etc/clash/config.yaml 2>/dev/null
				sed -i '/fake-ip-filter:/r/usr/share/clash/fake_filter.list' "/etc/clash/config.yaml" 2>/dev/null
			fi	
		fi	
	
