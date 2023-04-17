#!/bin/bash /etc/rc.common
. /lib/functions.sh


RULE_PROVIDER="/tmp/rule_provider.yaml"
GROUP_FILE="/tmp/groups.yaml"

SCRIPT="/usr/share/clash/create/script.yaml"
rule_providers=$(uci get clash.config.rule_providers 2>/dev/null)
CFG_FILE="/etc/config/clash"
config_name=$(uci get clash.config.name_tag 2>/dev/null)
lang=$(uci get luci.main.lang 2>/dev/null)
CONFIG_YAML="/usr/share/clash/config/custom/${config_name}.yaml"  
check_name=$(grep -F "${config_name}.yaml" "/usr/share/clashbackup/create_list.conf")
REAL_LOG="/usr/share/clash/clash_real.txt"
same_tag=$(uci get clash.config.same_tag 2>/dev/null)
rcount=$( grep -c "config ruleprovider" $CFG_FILE 2>/dev/null)
create=$(uci get clash.config.provider_config 2>/dev/null)
SERVER_FILE="/tmp/servers.yaml"
Proxy_Group="/tmp/Proxy_Group"

if [ "${create}" -eq 1 ];then

create(){

if  [ $config_name == "" ] || [ -z $config_name ];then

	if [ $lang == "en" ] || [ $lang == "auto" ];then
				echo "Tag Your Config" >$REAL_LOG
	elif [ $lang == "zh_cn" ];then
				echo "标记您的配置" >$REAL_LOG
	fi
	sleep 5
	echo "Clash for OpenWRT" >$REAL_LOG
	exit 0	
	
fi


if [ ! -z $check_name ] && [ "${same_tag}" -eq 0 ];then

	if [ $lang == "en" ] || [ $lang == "auto" ];then
				echo "Config with same name exist, please rename tag and create again" >$REAL_LOG
	elif [ $lang == "zh_cn" ];then
				echo "已存在同名配置，请重命名标记,重新创建配置" >$REAL_LOG
	fi
	sleep 5
	echo "Clash for OpenWRT" >$REAL_LOG
	exit 0	
fi

	

if [ -f $RULE_PROVIDER ];then
	rm -rf $RULE_PROVIDER 2>/dev/null
fi
	   
	   
rule_set()
{

   local section="$1"
   config_get "name" "$section" "name" ""
   config_get "type" "$section" "type" ""
   config_get "behavior" "$section" "behavior" ""
   config_get "path" "$section" "path" ""
   config_get "url" "$section" "url" ""
   config_get "interval" "$section" "interval" ""
   config_get_bool "enabled" "$section" "enabled" "1"

   if [ "$enabled" = "0" ]; then
      return
   fi

   if [ "$path" != "./ruleprovider/$name.yaml" ] && [ "$type" = "http" ]; then
      path="./ruleprovider/$name.yaml"
   elif [ -z "$path" ]; then
      return
   fi
   
cat >> "$RULE_PROVIDER" <<-EOF
  $name:
    type: $type
    behavior: $behavior
    path: $path	
EOF

	

	

	if [ "$type" == "http" ]; then
cat >> "$RULE_PROVIDER" <<-EOF
    url: $url
    interval: $interval
EOF
	fi
}

if [ $rcount -gt 0 ];then	
	 config_load clash
	 config_foreach rule_set "ruleprovider"
fi

if [ -f $RULE_PROVIDER ];then 
	sed -i "1i\   " $RULE_PROVIDER 2>/dev/null 
	sed -i "2i\rule-providers:" $RULE_PROVIDER 2>/dev/null
fi


PROVIDER_FILE="/tmp/yaml_provider.yaml"
pcount=$( grep -c "config proxyprovider" $CFG_FILE 2>/dev/null)

if [ -f $PROVIDER_FILE ];then
	rm -rf $PROVIDER_FILE 2>/dev/null
fi

yml_proxy_provider_set()
{
   local section="$1"

   config_get "type" "$section" "type" ""
   config_get "name" "$section" "name" ""
   config_get "path" "$section" "path" ""
   config_get "provider_url" "$section" "provider_url" ""
   config_get "provider_interval" "$section" "provider_interval" ""
   config_get "health_check" "$section" "health_check" ""
   config_get "health_check_url" "$section" "health_check_url" ""
   config_get "health_check_interval" "$section" "health_check_interval" ""
   config_get_bool "enabled" "$section" "enabled" "1"

   if [ "$enabled" = "0" ]; then
      return
   fi

   
   if [ "$path" != "./proxyprovider/$name.yaml" ] && [ "$type" = "http" ]; then
      path="./proxyprovider/$name.yaml"
   elif [ -z "$path" ]; then
      return
   fi
	  

   if [ -z "$type" ]; then
      return
   fi
   
   if [ -z "$name" ]; then
      return
   fi

   
   if [ -z "$health_check" ]; then
      return
   fi

   
   echo "$name" >> /tmp/Proxy_Provider
   
cat >> "$PROVIDER_FILE" <<-EOF
  $name:
    type: $type
    path: $path
EOF
   if [ ! -z "$provider_url" ]; then
cat >> "$PROVIDER_FILE" <<-EOF
    url: $provider_url
    interval: $provider_interval
EOF
   fi
cat >> "$PROVIDER_FILE" <<-EOF
    health-check:
      enable: $health_check
      url: $health_check_url
      interval: $health_check_interval
EOF

}


if [ $pcount -gt 0 ];then
	config_load "clash"
	config_foreach yml_proxy_provider_set "proxyprovider"
fi

if [ -f $PROVIDER_FILE ];then 
	sed -i "1i\   " $PROVIDER_FILE 2>/dev/null 
	sed -i "2i\proxy-providers:" $PROVIDER_FILE 2>/dev/null
	rm -rf /tmp/Proxy_Provider
fi


if [ -f $GROUP_FILE ];then
	rm -rf $GROUP_FILE 2>/dev/null
fi

set_groups()
{
  if [ -z "$1" ]; then
     return
  fi

	if [ "$1" = "$3" ]; then
	   set_group=1
	   echo "  - \"${2}\"" >>$GROUP_FILE
	fi

}


set_other_groups()
{
   set_group=1
   if [ "${1}" = "DIRECT" ]||[ "${1}" = "REJECT" ];then
   echo "    - ${1}" >>$GROUP_FILE 2>/dev/null 
   elif [ "${1}" = "ALL" ];then
   cat $Proxy_Group >> $GROUP_FILE 2>/dev/null
   else
   echo "    - \"${1}\"" >>$GROUP_FILE 2>/dev/null 
   fi

}

set_proxy_provider()
{
	local section="$1"
	config_get "name" "$section" "name" ""
	config_list_foreach "$section" "pgroups" set_provider_groups "$name" "$2"

}

set_provider_groups()
{
	if [ -z "$1" ]; then
		return
	fi

	if [ "$1" = "$3" ]; then
	   set_proxy_provider=1
	   echo "    - ${2}" >>$GROUP_FILE
	fi

}



scount=$( grep -c "config servers" $CFG_FILE 2>/dev/null)

servers_set()
{
   local section="$1"
   config_get "type" "$section" "type" ""
   config_get "name" "$section" "name" ""
   config_get "server" "$section" "server" ""
   config_get "port" "$section" "port" ""
   config_get "cipher" "$section" "cipher" ""
   config_get "password" "$section" "password" ""
   config_get "securitys" "$section" "securitys" ""
   config_get "udp" "$section" "udp" ""
   config_get "obfs" "$section" "obfs" ""
   config_get "obfs_vmess" "$section" "obfs_vmess" ""
   config_get "host" "$section" "host" ""
   config_get "custom" "$section" "custom" ""
   config_get "tls" "$section" "tls" ""
   config_get "tls_custom" "$section" "tls_custom" ""
   config_get "skip_cert_verify" "$section" "skip_cert_verify" ""
   config_get "path" "$section" "path" ""
   config_get "alterId" "$section" "alterId" ""
   config_get "uuid" "$section" "uuid" ""
   config_get "auth_name" "$section" "auth_name" ""
   config_get "auth_pass" "$section" "auth_pass" ""
   config_get "mux" "$section" "mux" ""
   config_get "protocol" "$section" "protocol" ""
   config_get "protocolparam" "$section" "protocolparam" ""
   config_get "obfsparam" "$section" "obfsparam" ""
   config_get "obfs_ssr" "$section" "obfs_ssr" ""
   config_get "cipher_ssr" "$section" "cipher_ssr" ""
   config_get "psk" "$section" "psk" ""
   config_get "obfs_snell" "$section" "obfs_snell" ""
   config_get "sni" "$section" "sni" ""
   config_get "alpn_h2" "$section" "alpn_h2" ""
   config_get "alpn_http" "$section" "alpn_http" ""
   config_get "http_path" "$section" "http_path" ""
   config_get "keep_alive" "$section" "keep_alive" ""
   config_get "servername" "$section" "servername" ""
   config_get_bool "enabled" "$section" "enabled" "1"

   if [ "$enabled" = "0" ]; then
      return
   fi

   
   if [ -z "$type" ]; then
      return
   fi
    
   if [ -z "$server" ]; then
      return
   fi

   if [ ! -z "$mux" ]; then
      muxx="mux: $mux"
   fi
   if [ "$obfs_snell" = "none" ]; then
      obfs_snell=""
   fi
   
   if [ -z "$name" ]; then
      name="Server"
   fi
   
   if [ -z "$port" ]; then
      return
   fi
   
   if [ ! -z "$udp" ] && [ "$obfs" = "none" ] && [ "$type" = "ss" ]; then
      udpp=", udp: $udp"
   fi

   if [ ! -z "$udp" ] && [ "$type" != "trojan" ] && [ "$type" != "ss" ]; then
      udpp=", udp: $udp"
   fi
   
   if [ "$obfs" != "none" ] && [ "$type" = "ss" ]; then
      if [ "$obfs" = "websocket" ]; then
         obfss="plugin: v2ray-plugin"
      else
         obfss="plugin: obfs"
      fi
   fi
   
   if [ "$obfs_vmess" = "websocket" ]; then
      obfs_vmess="network: ws"
   fi
   
   if [ "$obfs_vmess" = "http" ]; then
      obfs_vmess="network: http"
   fi
   
   if [ ! -z "$custom" ] && [ "$type" = "vmess" ]; then
      custom="Host: $custom"
   fi
   
   if [ ! -z "$path" ]; then
      if [ "$type" != "vmess" ]; then
         path="path: '$path'"
      elif [ "$obfs_vmess" = "network: ws" ]; then
         path="ws-path: $path"
      fi
   fi

   if [ ! -z "$auth_name" ] && [ ! -z "$auth_pass" ]; then
      auth_psk=", username: $auth_name, password: $auth_pass"
   fi
   
   
   if [ -z "$password" ]; then
   	 if [ "$type" = "ss" ] || [ "$type" = "trojan" ]; then
        return
     fi
   fi

   
   if [ "$type" = "ss" ] && [ "$obfs" = " " ]; then
      echo "- { name: \"$name\", type: $type, server: $server, port: $port, cipher: $cipher, password: "$password"$udpp }" >>$SERVER_FILE
   elif [ "$type" = "ss" ] && [ "$obfs" = "websocket" ] || [ "$obfs" = "tls" ] ||[ "$obfs" = "http" ]; then
cat >> "$SERVER_FILE" <<-EOF
- name: "$name"
  type: $type
  server: $server
  port: $port
  cipher: $cipher
  password: "$password"
EOF
  if [ ! -z "$udp" ]; then
cat >> "$SERVER_FILE" <<-EOF
  udp: $udp
EOF
  fi
  if [ ! -z "$obfss" ]; then
cat >> "$SERVER_FILE" <<-EOF
  $obfss
  plugin-opts:
    mode: $obfs
EOF
  fi
  
   if [ "$host" ]; then
cat >> "$SERVER_FILE" <<-EOF
    host: $host
EOF
  fi 
  
  if [ "$tls_custom" = "true" ] && [ "$type" = "ss" ]; then
cat >> "$SERVER_FILE" <<-EOF
    tls: true
EOF
  fi
   if [ "$skip_cert_verify" = "true" ] && [ "$type" = "ss" ]; then
cat >> "$SERVER_FILE" <<-EOF
    skip-cert-verify: true
EOF
  fi

    if [ ! -z "$path" ]; then
cat >> "$SERVER_FILE" <<-EOF
    $paths
EOF
  fi
  
      if [ "$mux" = "true" ]; then
cat >> "$SERVER_FILE" <<-EOF
    $muxx
EOF
  fi

  if [ ! -z "$custom" ]; then
cat >> "$SERVER_FILE" <<-EOF
    headers:
      custom: $custom
EOF
  fi
   fi
#trojan
if [ "$type" = "trojan" ]; then
cat >> "$SERVER_FILE" <<-EOF
- name: "$name"
  type: $type
  server: $server
  port: $port
  password: "$password"
EOF
if [ ! -z "$udp" ]; then
cat >> "$SERVER_FILE" <<-EOF
  udp: $udp
EOF
fi
if [ ! -z "$sni" ]; then
cat >> "$SERVER_FILE" <<-EOF
  sni: $sni
EOF
fi

if [ "$alpn_h2" == "1" ] || [ "$alpn_http" == "1" ]; then
cat >> "$SERVER_FILE" <<-EOF
  alpn:
EOF
if [ "$alpn_h2" == "1" ]; then
cat >> "$SERVER_FILE" <<-EOF
    - h2
EOF
fi
if [ "$alpn_http" == "1" ]; then
cat >> "$SERVER_FILE" <<-EOF
    - http/1.1
EOF
fi

fi

if [ "$skip_cert_verify" = "true" ] && [ "$type" = "trojan" ]; then
cat >> "$SERVER_FILE" <<-EOF
  skip-cert-verify: true
EOF
  fi
fi
   
#vmess
   if [ "$type" = "vmess" ]; then
cat >> "$SERVER_FILE" <<-EOF
- name: "$name"
  type: $type
  server: $server
  port: $port
  uuid: $uuid
  alterId: $alterId
  cipher: $securitys
EOF
      if [ ! -z "$udp" ]; then
cat >> "$SERVER_FILE" <<-EOF
  udp: $udp
EOF
      fi
      if [ "$tls" = "true" ]; then
cat >> "$SERVER_FILE" <<-EOF
  tls: $tls
EOF
      fi
      if [ "$skip_cert_verify" = "true" ]; then
cat >> "$SERVER_FILE" <<-EOF
  skip-cert-verify: $skip_cert_verify
EOF
      fi
      if [ ! -z "$servername" ] && [ "$tls" = "true" ]; then
cat >> "$SERVER_FILE" <<-EOF
  servername: $servername
EOF
fi	  
      if [ "$obfs_vmess" != "none" ]; then
cat >> "$SERVER_FILE" <<-EOF
  $obfs_vmess
EOF
         if [ ! -z "$path" ] && [ "$obfs_vmess" = "network: ws" ]; then
cat >> "$SERVER_FILE" <<-EOF
  $path
EOF
         fi
         if [ ! -z "$custom" ] && [ "$obfs_vmess" = "network: ws" ]; then
cat >> "$SERVER_FILE" <<-EOF
  ws-headers:
    $custom
EOF
         fi
         if [ ! -z "$http_path" ] && [ "$obfs_vmess" = "network: http" ]; then
cat >> "$SERVER_FILE" <<-EOF
  http-opts:
    method: "GET"
    path:
EOF
            config_list_foreach "$section" "http_path" set_http_path
         fi
         if [ "$keep_alive" = "true" ] && [ "$obfs_vmess" = "network: http" ]; then
cat >> "$SERVER_FILE" <<-EOF
    headers:
      Connection:
        - keep-alive
EOF
         fi
      fi
   fi

#socks5
   if [ "$type" = "socks5" ]; then
cat >> "$SERVER_FILE" <<-EOF
- name: "$name"
  type: $type
  server: $server
  port: $port
EOF
      if [ ! -z "$auth_name" ]; then
cat >> "$SERVER_FILE" <<-EOF
  username: $auth_name
EOF
      fi
      if [ ! -z "$auth_pass" ]; then
cat >> "$SERVER_FILE" <<-EOF
  password: $auth_pass
EOF
      fi
      if [ ! -z "$udp" ]; then
cat >> "$SERVER_FILE" <<-EOF
  udp: $udp
EOF
      fi
      if [ "$skip_cert_verify" = "true" ]; then
cat >> "$SERVER_FILE" <<-EOF
  skip-cert-verify: $skip_cert_verify
EOF
      fi
      if [ "$tls" = "true" ]; then
cat >> "$SERVER_FILE" <<-EOF
  tls: $tls
EOF
      fi
   fi

#http
   if [ "$type" = "http" ]; then
cat >> "$SERVER_FILE" <<-EOF
- name: "$name"
  type: $type
  server: $server
  port: $port
EOF
      if [ ! -z "$auth_name" ]; then
cat >> "$SERVER_FILE" <<-EOF
  username: $auth_name
EOF
      fi
      if [ ! -z "$auth_pass" ]; then
cat >> "$SERVER_FILE" <<-EOF
  password: $auth_pass
EOF
      fi
      if [ "$skip_cert_verify" = "true" ]; then
cat >> "$SERVER_FILE" <<-EOF
  skip-cert-verify: $skip_cert_verify
EOF
      fi
      if [ "$tls" = "true" ]; then
cat >> "$SERVER_FILE" <<-EOF
  tls: $tls
EOF
      fi
      if [ ! -z "$sni" ]; then
cat >> "$SERVER_FILE" <<-EOF
  sni: $sni
EOF
      fi	  
   fi


#snell
   if [ "$type" = "snell" ]; then
cat >> "$SERVER_FILE" <<-EOF
- name: "$name"
  type: $type
  server: $server
  port: $port
  psk: $psk
EOF
   if [ "$obfs_snell" != "none" ] && [ ! -z "$host" ]; then
cat >> "$SERVER_FILE" <<-EOF
  obfs-opts:
    mode: $obfs_snell
    host: $host
EOF
   fi
   fi

if [ "$type" = "ssr" ]; then
cat >> "$SERVER_FILE" <<-EOF
- name: "$name"
  type: $type
  server: $server
  port: $port
  cipher: $cipher_ssr
  password: "$password"
  protocol: "$protocol"
  protocol-param: "$protocolparam"
  obfs: "$obfs_ssr"
  obfs-param: "$obfsparam"
EOF
  if [ ! -z "$udp" ]; then
cat >> "$SERVER_FILE" <<-EOF
  udp: $udp
EOF
  fi
fi


}


if [ $scount -gt 0 ];then
config_load clash
config_foreach servers_set "servers"
fi

if [ ! -z "${scount}" ] || [ "${scount}" -ne 0 ];then

sed -i "1i\   " $SERVER_FILE 2>/dev/null 

sed -i "2i\proxies:" $SERVER_FILE 2>/dev/null 

egrep '^ {0,}-' $SERVER_FILE |grep name: |awk -F 'name: ' '{print $2}' |sed 's/,.*//' >$Proxy_Group 2>&1

sed -i "s/^ \{0,\}/    - /" $Proxy_Group 2>/dev/null 
fi


yml_servers_add()
{
	
	local section="$1"
	config_get "name" "$section" "name" ""
	config_list_foreach "$section" "pgroups" set_groups "$name" "$2"
	config_get "relay_groups" "$section" "relay_groups" ""
}

yml_groups_set()
{

   local section="$1"
   config_get "type" "$section" "type" ""
   config_get "name" "$section" "name" ""
   config_get "old_name" "$section" "old_name" ""
   config_get "test_url" "$section" "test_url" ""
   config_get "test_interval" "$section" "test_interval" ""
   config_get "other_group" "$section" "other_group" ""
   config_get_bool "enabled" "$section" "enabled" "1"

   if [ "$enabled" = "0" ]; then
      return
   fi


   if [ -z "$type" ]; then
      return
   fi
   
   if [ -z "$name" ]; then
      return
   fi
   
   echo "- name: $name" >>$GROUP_FILE 2>/dev/null 
   echo "  type: $type" >>$GROUP_FILE 2>/dev/null 
   group_name="$name"
   echo "  proxies: " >>$GROUP_FILE
   
   set_group=0
   set_proxy_provider=0 
   
   
  	
   config_list_foreach "$section" "other_group" set_other_groups

   if [ "$( grep -c "config proxyprovider" $CFG_FILE )" -gt 0 ];then  

		    echo "  use: $group_name" >>$GROUP_FILE	   
		    if [ "$type" != "relay" ]; then
				 config_foreach set_proxy_provider "proxyprovider" "$group_name" 
		    fi

		    if [ "$set_proxy_provider" -eq 1 ]; then
			  sed -i "/^ \{0,\}use: ${group_name}/c\  use:" $GROUP_FILE
		    else
			  sed -i "/use: ${group_name}/d" $GROUP_FILE 
		    fi
			

		    if [ "$set_group" -eq 1 ]; then
			  sed -i "/^ \{0,\}proxies: ${group_name}/c\  proxies:" $GROUP_FILE
		    else
			  sed -i "/proxies: ${group_name}/d" $GROUP_FILE 
		    fi	     
   fi      
 
   
    [ ! -z "$test_url" ] && {
		echo "  url: $test_url" >>$GROUP_FILE 2>/dev/null 
    }
    [ ! -z "$test_interval" ] && {
		echo "  interval: \"$test_interval\"" >>$GROUP_FILE 2>/dev/null 
    }
}

gcount=$( grep -c "config pgroups" $CFG_FILE 2>/dev/null)
if [ $gcount -gt 0 ];then
	config_load clash
	config_foreach yml_groups_set "pgroups"
fi


if [ -f $GROUP_FILE ]; then
	sed -i "1i\  " $GROUP_FILE 2>/dev/null 
	sed -i "2i\proxy-groups:" $GROUP_FILE 2>/dev/null 
fi


RULE_FILE="/tmp/rules.yaml"
rucount=$( grep -c "config rules" $CFG_FILE 2>/dev/null)

if [ -f $RULE_FILE ];then
	rm -rf $RULE_FILE 2>/dev/null
fi
	   
	   
add_rules()
{
	   local section="$1"
	   config_get "rulegroups" "$section" "rulegroups" ""
	   config_get "rulename" "$section" "rulename" ""
	   config_get "type" "$section" "type" ""
	   config_get "res" "$section" "res" ""
	   config_get "rulenamee" "$section" "rulenamee" ""
		config_get_bool "enabled" "$section" "enabled" "1"

	   if [ "$enabled" = "0" ]; then
		  return
	   fi
	   
	    if [ ! -z $rulename ];then
	      rulename=$rulename
		elif [ ! -z $rulenamee ];then
		  rulename=$rulenamee
		fi	
		  
	   if [ "${res}" -eq 1 ];then
		echo "- $type,$rulename,$rulegroups,no-resolve">>$RULE_FILE
	   elif [ "${type}" == "MATCH" ];then
	    echo "- $type,$rulegroups">>$RULE_FILE
	   else
		echo "- $type,$rulename,$rulegroups">>$RULE_FILE
	   fi
}

if [ $rucount -gt 0 ];then	
 config_load clash
 config_foreach add_rules "rules"
fi
 
if [ -f $RULE_FILE ];then 
	sed -i "1i\   " $RULE_FILE 2>/dev/null 
	sed -i "2i\rules:" $RULE_FILE 2>/dev/null
fi 

		lang=$(uci get luci.main.lang 2>/dev/null)
		REAL_LOG="/usr/share/clash/clash_real.txt"

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
		 
		mixed_port=$(uci get clash.config.mixed_port 2>/dev/null)
		enable_ipv6=$(uci get clash.config.enable_ipv6 2>/dev/null)
		
		core=$(uci get clash.config.core 2>/dev/null)
		interf_name=$(uci get clash.config.interf_name 2>/dev/null)
		tun_mode=$(uci get clash.config.tun_mode 2>/dev/null)
		stack=$(uci get clash.config.stack 2>/dev/null)
		listen_port=$(uci get clash.config.listen_port 2>/dev/null)	
		TEMP_FILE="/tmp/clashdns.yaml"
		interf=$(uci get clash.config.interf 2>/dev/null)
		enhanced_mode=$(uci get clash.config.enhanced_mode 2>/dev/null)

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



if [ $tun_mode -eq 1 ];then

cat >> "/tmp/tun.yaml" <<-EOF
tun:
  enable: true  
EOF


if [ $core -eq 4 ];then
cat >> "/tmp/tun.yaml" <<-EOF
  stack: ${stack}   
EOF
fi 

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


enable_dns=$(uci get clash.config.enable_dns 2>/dev/null) 



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



rm -rf /tmp/tun.yaml /tmp/enable_dns.yaml /tmp/fallback.yaml /tmp/nameservers.yaml /tmp/fake_ip_filter.yaml /tmp/default_nameserver.yaml /tmp/hosts.yaml /tmp/authentication.yaml /tmp/dnshijack.yaml /tmp/fake_ip_range.yaml /tmp/dns.yaml /tmp/interf_name.yaml
		
		


script=$(uci get clash.config.script 2>/dev/null)
ruleprovider=$(uci get clash.config.rulprp 2>/dev/null)
ppro=$(uci get clash.config.ppro 2>/dev/null)
rul=$(uci get clash.config.rul 2>/dev/null)
prox=$(uci get clash.config.prox 2>/dev/null)
CONFIG_YAML_RULE="/usr/share/clash/rule.yaml"
orul=$(uci get clash.config.orul 2>/dev/null)

if [ $prox -eq 1 ];then
if [ -f $SERVER_FILE ];then
cat $SERVER_FILE >> $TEMP_FILE  2>/dev/null
sed -i -e '$a\' $TEMP_FILE  2>/dev/null
fi
fi

if [ $ppro -eq 1 ];then
if [ -f $PROVIDER_FILE ];then 
cat $PROVIDER_FILE >> $TEMP_FILE 2>/dev/null
fi
fi

if [ -f $GROUP_FILE ];then
cat $GROUP_FILE >> $TEMP_FILE 2>/dev/null
fi


if [ $ruleprovider -eq 1 ];then
if [ -f $RULE_PROVIDER ];then
cat $RULE_PROVIDER >> $TEMP_FILE  2>/dev/null
sed -i -e '$a\' $TEMP_FILE  2>/dev/null
fi
fi

if [ $script -eq 1 ];then
if [ -f $SCRIPT ];then
cat $SCRIPT >> $TEMP_FILE  2>/dev/null
sed -i -e '$a\' $TEMP_FILE  2>/dev/null
fi
fi


if [ $rul -eq 1 ];then
if [ -f $RULE_FILE ];then
cat $RULE_FILE >> $TEMP_FILE 2>/dev/null
fi
fi

if [ $orul -eq 1 ];then
cat $TEMP_FILE $CONFIG_YAML_RULE > $CONFIG_YAML 2>/dev/null
fi

if [ $orul -eq 0 ] || [ -z $orul ];then
mv $TEMP_FILE  $CONFIG_YAML 2>/dev/null
fi

if [ -z $check_name ] && [ "${same_tag}" -eq 1 ];then
echo "${config_name}.yaml" >>/usr/share/clashbackup/create_list.conf
elif [ -z $check_name ] && [ "${same_tag}" -eq 0 ];then
echo "${config_name}.yaml" >>/usr/share/clashbackup/create_list.conf
fi

rm -rf $RULE_PROVIDER $PROVIDER_FILE $GROUP_FILE  $RULE_FILE $SERVER_FILE $Proxy_Group

if [ $lang == "en" ] || [ $lang == "auto" ];then
		echo "Completed Creating Custom Config " >$REAL_LOG 
		 sleep 1
			echo "Clash for OpenWRT" >$REAL_LOG
elif [ $lang == "zh_cn" ];then
    	echo "创建自定义配置完成." >$REAL_LOG
		sleep 1
		echo "Clash for OpenWRT" >$REAL_LOG
fi


use=$(uci get clash.config.use_config 2>/dev/null)
config_type=$(uci get clash.config.config_type 2>/dev/null)

if [  "$use" == "$CONFIG_YAML" ] && [ "$config_type" == "3" ];then
	if pidof clash >/dev/null; then
			/etc/init.d/clash restart 2>/dev/null
	fi
fi
}
create 2>/dev/null
fi



	