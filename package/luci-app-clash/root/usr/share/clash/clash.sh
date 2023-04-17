#!/bin/bash /etc/rc.common
   
clash_url=$(uci get clash.config.clash_url 2>/dev/null)
ssr_url=$(uci get clash.config.ssr_url 2>/dev/null)
v2_url=$(uci get clash.config.v2_url 2>/dev/null)

config_name=$(uci get clash.config.config_name 2>/dev/null) 
subtype=$(uci get clash.config.subcri 2>/dev/null) 
REAL_LOG="/usr/share/clash/clash_real.txt" 
lang=$(uci get luci.main.lang 2>/dev/null)
CONFIG_YAML="/usr/share/clash/config/sub/${config_name}.yaml" 
 
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


if [ ! -f "/usr/share/clashbackup/confit_list.conf" ];then 
   touch /usr/share/clashbackup/confit_list.conf
fi


check_name=$(grep -F "${config_name}.yaml" "/usr/share/clashbackup/confit_list.conf")


if [ ! -z $check_name ];then
   
	if [ $lang == "en" ] || [ $lang == "auto" ];then
				echo "Config with same name exist, please rename and download again" >$REAL_LOG
	elif [ $lang == "zh_cn" ];then
				echo "已存在同名配置，请重命名名配置重新下载" >$REAL_LOG
	fi
	sleep 5
	echo "Clash for OpenWRT" >$REAL_LOG
	exit 0	

   
else

	if [ $lang == "en" ] || [ $lang == "auto" ];then
				echo "Downloading Configuration..." >$REAL_LOG
	elif [ $lang == "zh_cn" ];then
				echo "开始下载配置" >$REAL_LOG
	fi
	sleep 1
		
	if [ "$subtype" = "clash" ];then
	wget -c4 --no-check-certificate --user-agent="Clash/OpenWRT" "$clash_url" -O 2>&1 >1 $CONFIG_YAML
	if [ "$?" -eq "0" ]; then
	echo "${config_name}.yaml#$clash_url#$subtype" >>/usr/share/clashbackup/confit_list.conf
	fi
    fi
	
	if [ "$subtype" = "ssr2clash" ];then
	wget -c4 --no-check-certificate --user-agent="Clash/OpenWRT" "https://gfwsb.114514.best/sub?target=clashr&url=$ssr_url" -O 2>&1 >1 $CONFIG_YAML
	if [ "$?" -eq "0" ]; then
	echo "${config_name}.yaml#$ssr_url#$subtype" >>/usr/share/clashbackup/confit_list.conf
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
		sleep 1
		
	fi
    fi

	if [ "$subtype" = "v2clash" ];then
	wget -c4 --no-check-certificate --user-agent="Clash/OpenWRT" "https://tgbot.lbyczf.com/v2rayn2clash?url=$v2_url" -O 2>&1 >1 $CONFIG_YAML
	if [ "$?" -eq "0" ]; then
	echo "${config_name}.yaml#$v2_url#$subtype" >>/usr/share/clashbackup/confit_list.conf
	fi
    fi	
	
	if [ $lang == "en" ] || [ $lang == "auto" ];then
		echo "Downloading Configuration Completed" >$REAL_LOG
		sleep 2
		echo "Clash for OpenWRT" >$REAL_LOG
	elif [ $lang == "zh_cn" ];then
		echo "下载配置完成" >$REAL_LOG
		sleep 2
		echo "Clash for OpenWRT" >$REAL_LOG
	fi
   
fi   
