#!/bin/bash /etc/rc.common
. /lib/functions.sh  

config_name=$(uci get clash.config.config_update_name 2>/dev/null)
CONFIG_YAML="/usr/share/clash/config/sub/${config_name}" 
url=$(grep -F "${config_name}" "/usr/share/clashbackup/confit_list.conf" | awk -F '#' '{print $2}')
lang=$(uci get luci.main.lang 2>/dev/null)
REAL_LOG="/usr/share/clash/clash_real.txt"
c_type=$(uci get clash.config.config_type 2>/dev/null)
path=$(uci get clash.config.use_config 2>/dev/null)
type=$(grep -F "${config_name}" "/usr/share/clashbackup/confit_list.conf" | awk -F '#' '{print $3}') 

if [ $type == "clash" ] && [ ! -z $url ];then

	if [ $lang == "en" ] || [ $lang == "auto" ];then
				echo "Updating Configuration..." >$REAL_LOG
	elif [ $lang == "zh_cn" ];then
				echo "开始更新配置" >$REAL_LOG
	fi
	wget --no-check-certificate --user-agent="Clash/OpenWRT" $url -O 2>&1 >1 $CONFIG_YAML
	
	if [ "$?" -eq "0" ]; then
		if [ $lang == "en" ] || [ $lang == "auto" ];then
			echo "Updating Configuration Completed" >$REAL_LOG

		elif [ $lang == "zh_cn" ];then
			echo "更新配置完成" >$REAL_LOG

		fi
		sleep 1
		echo "Clash for OpenWRT" >$REAL_LOG
		sleep 2
		use_config=$(uci get clash.config.use_config 2>/dev/null)

		if [ "$c_type" -eq 1 ] && [ "$CONFIG_YAML" = "$use_config" ];then 
		if pidof clash >/dev/null; then
				/etc/init.d/clash restart 2>/dev/null
		fi
		fi		
			
	fi
fi

if [ $type == "ssr2clash" ] && [ ! -z $url ];then

	if [ $lang == "en" ] || [ $lang == "auto" ];then
				echo "Updating Configuration..." >$REAL_LOG
	elif [ $lang == "zh_cn" ];then
				echo "开始更新配置" >$REAL_LOG
	fi
	wget --no-check-certificate --user-agent="Clash/OpenWRT" "https://ssrsub2clashr.herokuapp.com/ssrsub2clash?sub=$url" -O 2>&1 >1 $CONFIG_YAML
	
	if [ "$?" -eq "0" ]; then
	
		CONFIG_YAMLL="/tmp/conf"
		da_password=$(uci get clash.config.dash_pass 2>/dev/null)
		redir_port=$(uci get clash.config.redir_port 2>/dev/null)
		http_port=$(uci get clash.config.http_port 2>/dev/null)
		socks_port=$(uci get clash.config.socks_port 2>/dev/null) 
		dash_port=$(uci get clash.config.dash_port 2>/dev/null)
		bind_addr=$(uci get clash.config.bind_addr 2>/dev/null)
		allow_lan=$(uci get clash.config.allow_lan 2>/dev/null)
		log_level=$(uci get clash.config.level 2>/dev/null)
		p_mode=$(uci get clash.config.p_mode 2>/dev/null)
		sed -i "/^Proxy:/i\#clash-openwrt" $CONFIG_YAML 2>/dev/null
		sed -i '1,/#clash-openwrt/d' $CONFIG_YAML 2>/dev/null
		
		cat /usr/share/clash/dns.yaml $CONFIG_YAML > $CONFIG_YAMLL 2>/dev/null
		mv $CONFIG_YAMLL $CONFIG_YAML 2>/dev/null
		
		sed -i "1i\#****CLASH-CONFIG-START****#" $CONFIG_YAML 2>/dev/null
		sed -i "2i\port: ${http_port}" $CONFIG_YAML 2>/dev/null
		sed -i "/port: ${http_port}/a\socks-port: ${socks_port}" $CONFIG_YAML 2>/dev/null 
		sed -i "/socks-port: ${socks_port}/a\redir-port: ${redir_port}" $CONFIG_YAML 2>/dev/null 
		sed -i "/redir-port: ${redir_port}/a\allow-lan: ${allow_lan}" $CONFIG_YAML 2>/dev/null 
		if [ $allow_lan == "true" ];  then
		sed -i "/allow-lan: ${allow_lan}/a\bind-address: \"${bind_addr}\"" $CONFIG_YAML 2>/dev/null 
		sed -i "/bind-address: \"${bind_addr}\"/a\mode: ${p_mode}" $CONFIG_YAML 2>/dev/null
		sed -i "/mode: ${p_mode}/a\log-level: ${log_level}" $CONFIG_YAML 2>/dev/null 
		sed -i "/log-level: ${log_level}/a\external-controller: 0.0.0.0:${dash_port}" $CONFIG_YAML 2>/dev/null 
		sed -i "/external-controller: 0.0.0.0:${dash_port}/a\secret: \"${da_password}\"" $CONFIG_YAML 2>/dev/null 
		sed -i "/secret: \"${da_password}\"/a\external-ui: \"/usr/share/clash/dashboard\"" $CONFIG_YAML 2>/dev/null 
		
		else
		sed -i "/allow-lan: ${allow_lan}/a\mode: Rule" $CONFIG_YAML 2>/dev/null
		sed -i "/mode: Rule/a\log-level: ${log_level}" $CONFIG_YAML 2>/dev/null 
		sed -i "/log-level: ${log_level}/a\external-controller: 0.0.0.0:${dash_port}" $CONFIG_YAML 2>/dev/null 
		sed -i "/external-controller: 0.0.0.0:${dash_port}/a\secret: \"${da_password}\"" $CONFIG_YAML 2>/dev/null 
		sed -i "/secret: \"${da_password}\"/a\external-ui: \"/usr/share/clash/dashboard\"" $CONFIG_YAML 2>/dev/null	
		fi	
	
		if [ $lang == "en" ] || [ $lang == "auto" ];then
			echo "Updating Configuration Completed" >$REAL_LOG

		elif [ $lang == "zh_cn" ];then
			echo "更新配置完成" >$REAL_LOG

		fi
		sleep 1
		echo "Clash for OpenWRT" >$REAL_LOG
		sleep 2
		use_config=$(uci get clash.config.use_config 2>/dev/null)

		if [ "$c_type" -eq 1 ] && [ "$CONFIG_YAML" = "$use_config" ];then 
		if pidof clash >/dev/null; then
				/etc/init.d/clash restart 2>/dev/null
		fi
		fi		
			
	fi
fi
if [ $type == "v2clash" ] && [ ! -z $url ];then

	if [ $lang == "en" ] || [ $lang == "auto" ];then
				echo "Updating Configuration..." >$REAL_LOG
	elif [ $lang == "zh_cn" ];then
				echo "开始更新配置" >$REAL_LOG
	fi
	wget --no-check-certificate --user-agent="Clash/OpenWRT" "https://tgbot.lbyczf.com/v2rayn2clash?url=$url" -O 2>&1 >1 $CONFIG_YAML
	
	if [ "$?" -eq "0" ]; then
		if [ $lang == "en" ] || [ $lang == "auto" ];then
			echo "Updating Configuration Completed" >$REAL_LOG

		elif [ $lang == "zh_cn" ];then
			echo "更新配置完成" >$REAL_LOG

		fi
		sleep 1
		echo "Clash for OpenWRT" >$REAL_LOG
		sleep 2
		use_config=$(uci get clash.config.use_config 2>/dev/null)

		if [ "$c_type" -eq 1 ] && [ "$CONFIG_YAML" = "$use_config" ];then 
		if pidof clash >/dev/null; then
				/etc/init.d/clash restart 2>/dev/null
		fi
		fi		
			
	fi
fi

